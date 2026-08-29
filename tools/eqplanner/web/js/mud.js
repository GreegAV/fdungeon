/*
 * mud.js -- the bits of FDungeon's rules the planner has to reproduce.
 * Every function here mirrors a specific place in the C sources; the comment
 * above it says where, so the two can be kept in step.
 */

const MUD = (() => {

  /* ---------------------------------------------------------------- colours */
  /* comm.c:colour() -> the ANSI codes in merc_def.h */
  const COLOURS = {
    x: null,                       // CLEAR
    r: '#c03a3a', g: '#3f9e4d', y: '#b8860b', b: '#4169c9',
    m: '#a349a3', c: '#2e9e9e', w: '#b6b6b6',
    D: '#6b6b6b', R: '#ff6b6b', G: '#66d97a', Y: '#f2d24b',
    B: '#6f9dff', M: '#e07be0', C: '#5ee0e0', W: '#f2f2f2',
  };

  const escapeHtml = (s) => s.replace(/[&<>"]/g, (c) =>
    ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));

  /* Renders the MUD's `{c` colour markup as HTML spans. */
  function colourize(text) {
    if (!text) return '';
    let out = '', open = false, i = 0;
    while (i < text.length) {
      const ch = text[i];
      if (ch === '{' && i + 1 < text.length) {
        const code = text[i + 1];
        i += 2;
        if (code === '{') { out += '{'; continue; }
        if (code === '/') { out += '<br>'; continue; }
        if (code === '`' || code === '*') { out += escapeHtml(code); continue; }
        if (open) { out += '</span>'; open = false; }
        const col = COLOURS[code];
        if (col) { out += `<span style="color:${col}">`; open = true; }
        continue;
      }
      out += escapeHtml(ch);
      i++;
    }
    if (open) out += '</span>';
    return out;
  }

  /* Same, but for the fixed-width body pictures: spaces must survive. */
  function colourizePicture(line) {
    return colourize(line).replace(/ /g, '&nbsp;');
  }

  function stripColours(text) {
    return (text || '').replace(/\{(.)/g, (m, c) => (c === '{' ? '{' : ''));
  }

  /* ------------------------------------------------------- grammatical case */
  /* comm.c:gram_newformat() -- "stem|end1|end2|..." picks one Russian case. */
  function gramCase(description, caseSet = 1) {
    const COPYTOBUF = 1, FINDNEED = 2, COPYEND = 3, FINDNEXT = 4;
    let phase = COPYTOBUF, counter = caseSet, out = '';
    for (const ch of description || '') {
      if (ch === ' ' || ch === '-') phase = COPYTOBUF;
      if (phase !== FINDNEED) counter = caseSet;
      if (ch === '|') {
        if (phase === COPYEND) phase = FINDNEXT;
        if (phase === COPYTOBUF) phase = FINDNEED;
        if (phase === FINDNEED) { counter--; if (counter <= 0) phase = COPYEND; }
      } else if (phase === COPYEND || phase === COPYTOBUF) {
        out += ch;
      }
    }
    return out;
  }

  /* ----------------------------------------------------------------- flags */
  /* db.c:flag_convert() -- A..Z = bits 0..25, a..z = bits 26..51 */
  function bit(letter) {
    const c = letter.charCodeAt(0);
    if (c >= 65 && c <= 90) return Math.pow(2, c - 65);
    return Math.pow(2, 26 + c - 97);
  }

  function hasFlag(value, flagBit) {
    // values stay well inside 2^52, so plain arithmetic is safe here
    return Math.floor(value / flagBit) % 2 === 1;
  }

  function flagNames(value, table) {
    return table.filter(([b]) => hasFlag(value, b)).map(([, n]) => n);
  }

  /* ------------------------------------------------------- wear locations */
  /* merc_def.h WEAR_* */
  const WEAR = {
    LIGHT: 0, FINGER_L: 1, FINGER_R: 2, NECK: 3, BODY: 5, HEAD: 6, LEGS: 7,
    FEET: 8, HANDS: 9, ARMS: 10, SHIELD: 11, ABOUT: 12, WAIST: 13,
    WRIST_L: 14, WRIST_R: 15, RHAND: 16, HOLD: 17, FLOAT: 18, LHAND: 19,
  };

  const ITEM_ARMOR = 9, ITEM_WEAPON = 5, ITEM_LIGHT = 1;

  /* handler.c:apply_ac() -- note there is no case for RHAND/LHAND/FINGER, so
     shields, held items and rings contribute no armour value in this codebase;
     everything they give comes through their `ac` applies instead. */
  const AC_MULT = {
    [WEAR.BODY]: 3, [WEAR.HEAD]: 2, [WEAR.LEGS]: 2, [WEAR.FEET]: 1,
    [WEAR.HANDS]: 1, [WEAR.ARMS]: 1, [WEAR.SHIELD]: 1, [WEAR.NECK]: 1,
    [WEAR.ABOUT]: 2, [WEAR.WAIST]: 1, [WEAR.WRIST_L]: 1, [WEAR.WRIST_R]: 1,
    [WEAR.HOLD]: 1,
  };

  function applyAc(obj, wearNum, acType) {
    if (obj.t !== 'armor') return 0;
    const mult = AC_MULT[wearNum] || 0;
    return mult * (obj.val[acType] || 0);
  }

  /* ------------------------------------------------------------- alignment */
  /* merc_def.h IS_GOOD / IS_EVIL / IS_NEUTRAL */
  const ALIGN_VALUE = { good: 1000, neutral: 0, evil: -1000 };

  /* handler.c:equip_char() refuses anti-<align> gear. */
  function alignAllows(obj, align, anti) {
    if (align === 'good' && hasFlag(obj.ef, anti.good)) return false;
    if (align === 'evil' && hasFlag(obj.ef, anti.evil)) return false;
    if (align === 'neutral' && hasFlag(obj.ef, anti.neutral)) return false;
    return true;
  }

  /* ----------------------------------------------------------------- saves */
  /* magic.c:calc_saves() */
  function calcSaves(savingThrow, level) {
    let saves = savingThrow;
    if (level <= 15) saves *= 3;
    else if (level >= 16 && level < 40) saves *= 2;
    else if (level >= 60 && level < 90) saves = Math.trunc(saves * 3 / 4);
    else if (level >= 90 && level < 100) saves = Math.trunc(saves * 2 / 3);
    else if (level >= 100 && level < 102) saves = Math.trunc(saves / 2);
    return 50 - saves;
  }

  /* ----------------------------------------------------------------- totals */
  /* handler.c:affect_modify() + equip_char() applied over a set of items. */
  const APPLY = {
    STR: 1, DEX: 2, INT: 3, WIS: 4, CON: 5, MANA: 12, HIT: 13, MOVE: 14,
    AC: 17, HITROLL: 18, DAMROLL: 19, SAVES: 20, SAVING_ROD: 21,
    SAVING_PETRI: 22, SAVING_BREATH: 23, SAVING_SPELL: 24, SPELL_AFFECT: 25,
  };

  function emptyTotals() {
    return {
      armor: [0, 0, 0, 0], hitroll: 0, damroll: 0, saving_throw: 0,
      hit: 0, mana: 0, move: 0,
      str: 0, dex: 0, int: 0, wis: 0, con: 0,
      weight: 0, cost: 0, pieces: 0,
    };
  }

  /* `equipped` maps a wear_num to an object record. */
  function computeTotals(equipped) {
    const t = emptyTotals();
    for (const [wearNumStr, obj] of Object.entries(equipped)) {
      if (!obj) continue;
      const wearNum = Number(wearNumStr);
      t.pieces++;
      t.weight += obj.w || 0;
      t.cost += obj.c || 0;

      // equip_char(): ch->armor[i] -= apply_ac(obj, iWear, i)
      for (let i = 0; i < 4; i++) t.armor[i] -= applyAc(obj, wearNum, i);

      for (const [loc, mod] of obj.af || []) {
        switch (loc) {
          case APPLY.STR: t.str += mod; break;
          case APPLY.DEX: t.dex += mod; break;
          case APPLY.INT: t.int += mod; break;
          case APPLY.WIS: t.wis += mod; break;
          case APPLY.CON: t.con += mod; break;
          case APPLY.MANA: t.mana += mod; break;
          case APPLY.HIT: t.hit += mod; break;
          case APPLY.MOVE: t.move += mod; break;
          case APPLY.AC: for (let i = 0; i < 4; i++) t.armor[i] += mod; break;
          case APPLY.HITROLL: t.hitroll += mod; break;
          case APPLY.DAMROLL: t.damroll += mod; break;
          case APPLY.SAVES:
          case APPLY.SAVING_ROD:
          case APPLY.SAVING_PETRI:
          case APPLY.SAVING_BREATH:
          case APPLY.SAVING_SPELL: t.saving_throw += mod; break;
          default: break;
        }
      }
    }
    return t;
  }

  /* ---------------------------------------------------------------- weapons */
  /* magic2.c:spell_identify() -- new_format weapons roll value1 d value2 */
  function weaponAverage(obj) {
    return Math.trunc((1 + obj.val[2]) * obj.val[1] / 2);
  }

  return {
    COLOURS, WEAR, APPLY, ALIGN_VALUE, ITEM_ARMOR, ITEM_WEAPON, ITEM_LIGHT,
    escapeHtml, colourize, colourizePicture, stripColours, gramCase,
    bit, hasFlag, flagNames, applyAc, alignAllows, calcSaves,
    computeTotals, weaponAverage, emptyTotals,
  };
})();
