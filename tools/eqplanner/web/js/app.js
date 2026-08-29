/*
 * app.js -- FDungeon equipment planner UI.
 *
 * Left   : the ASCII body picture from mud/const.c (where_name_male/female),
 *          one line per wear slot, exactly as `equipment` prints it.
 * Centre : one dropdown per wear slot, filled with the items a character of
 *          the chosen level and alignment is allowed to wear there.
 * Right  : an identify panel modelled on magic2.c:spell_identify().
 * Bottom : the totals -- hitroll/damroll, armour class, saves.
 */

(() => {
  'use strict';

  const $ = (sel) => document.querySelector(sel);

  const state = {
    meta: null,
    objects: [],
    pictures: null,
    bySlot: new Map(),      // slot key -> objects that fit there
    equipped: {},           // slot key -> object record (or undefined)
    activeSlot: null,
    selected: null,         // object shown in the identify panel
    areasOn: new Set(),     // area files whose items may show up
  };

  /* merc_def.h: LEVEL_HERO -- above this an area is immortal-only ground.
     Security is the OLC edit level from #AREADATA; 9 is implementor-only and
     in practice marks the god/test areas whose gear is not part of normal
     play (apc, godcastl, imperror ...). */
  const LEVEL_HERO = 101;
  const IMPLEMENTOR_SECURITY = 9;
  const AREA_STORAGE_KEY = 'fdungeon.eqplanner.areas';

  const el = {
    level: $('#level'), align: $('#align'), klass: $('#class'), sex: $('#sex'),
    minlevel: $('#minlevel'), search: $('#search'),
    doll: $('#doll'), slots: $('#slots'), identify: $('#identify'),
    totals: $('#totals'), loading: $('#loading'), slotCount: $('#slotCount'),
    dollHint: $('#dollHint'), areaList: $('#areaList'),
    areaSearch: $('#areaSearch'), areaSummary: $('#areaSummary'),
    areaFilter: $('#areafilter'),
  };

  /* ------------------------------------------------------------------ i18n */

  const APPLY_UA = {
    1: 'сила', 2: 'спритність', 3: 'інтелект', 4: 'мудрість',
    5: 'статура', 6: 'стать', 7: 'клас', 8: 'рівень', 9: 'вік',
    10: 'зріст', 11: 'вага', 12: 'мана', 13: 'хіти', 14: 'мува',
    15: 'золото', 16: 'досвід', 17: 'armor class', 18: 'hitroll',
    19: 'damroll', 20: 'saves', 21: 'save vs rod',
    22: 'save vs petrification', 23: 'save vs breath', 24: 'save vs spell',
    25: 'spell affect',
  };

  const TYPE_UA = {
    light: 'джерело світла', scroll: 'сувій', wand: 'чарівна паличка',
    staff: 'посох', weapon: 'зброя', treasure: 'скарб', armor: 'броня',
    potion: 'зілля', clothing: 'одяг', furniture: 'меблі', trash: 'мотлох',
    container: 'контейнер', drink: 'посудина', key: 'ключ', food: 'їжа',
    money: 'гроші', boat: 'човен', fountain: 'фонтан', pill: 'пігулка',
    protect: 'оберіг', map: 'мапа', portal: 'портал',
    warp_stone: 'камінь переносу', room_key: 'ключ від кімнати',
    gem: 'самоцвіт', jewelry: 'прикраса', jukebox: 'музична скринька',
    enchanter: 'зачарувач', scuba: 'акваланг', bonus: 'бонус',
  };

  const WEAPON_CLASS_UA = {
    exotic: 'екзотична', sword: 'меч', dagger: 'кинджал', spear: 'спис',
    mace: 'булава', axe: 'сокира', flail: 'ціп', whip: 'батіг',
    polearm: 'древкова', staff: 'посох',
  };

  /* Extra flags worth shouting about in the identify pane. */
  const FLAG_UA = {
    glow: 'світиться', hum: 'гуде', dark: 'темний', evil: 'зле',
    invis: 'невидиме', magic: 'магічне', nodrop: 'не викинути',
    bless: 'благословенне', antigood: 'anti-good', antievil: 'anti-evil',
    antineutral: 'anti-neutral', noremove: 'не зняти',
    nopurge: 'nopurge', rotdeath: 'зникає після смерті',
    visdeath: 'проявляється після смерті', nonmetal: 'неметалеве',
    meltdrop: 'тане при падінні', sellextract: 'зникає при продажу',
    noident: 'не ідентифікується', burnproof: 'не горить',
    nouncurse: 'не знімається прокляття', nolocate: 'не шукається',
    take: 'можна взяти', nosac: 'не жертвується',
  };

  const DANGEROUS = new Set(['nodrop', 'noremove', 'rotdeath', 'meltdrop',
                             'sellextract', 'antigood', 'antievil',
                             'antineutral']);

  const ua = (map, key) => map[key] || key;

  /* -------------------------------------------------------------- loading */

  async function load() {
    const [objRes, picRes] = await Promise.all([
      fetch('data/objects.json'),
      fetch('data/pictures.json'),
    ]);
    if (!objRes.ok || !picRes.ok) {
      throw new Error('data/objects.json або data/pictures.json не знайдено. '
        + 'Спочатку запусти: python3 tools/eqplanner/extract_data.py');
    }
    const data = await objRes.json();
    state.meta = data.meta;
    state.objects = data.objects;
    state.pictures = await picRes.json();

    for (const slot of state.meta.slots) state.bySlot.set(slot.key, []);
    for (const o of state.objects) {
      for (const key of o.s) {
        const list = state.bySlot.get(key);
        if (list) list.push(o);
      }
    }
    // best first inside each slot: higher level items are the interesting ones
    for (const list of state.bySlot.values()) {
      list.sort((a, b) => b.l - a.l || a.n.localeCompare(b.n, 'uk'));
    }
  }

  /* ---------------------------------------------------------- area filter */

  /* Areas the planner leaves switched off until asked: those whose Credits
     band starts at hero level or above, and those an implementor alone may
     edit -- the god areas hold gear with five-digit hitrolls. */
  function isImmortalArea(area) {
    return (area.lo !== null && area.lo >= LEVEL_HERO)
      || area.security >= IMPLEMENTOR_SECURITY;
  }

  function defaultAreas() {
    return new Set(state.meta.areas
      .filter((a) => !isImmortalArea(a))
      .map((a) => a.file));
  }

  function loadAreaChoice() {
    try {
      const saved = JSON.parse(localStorage.getItem(AREA_STORAGE_KEY));
      if (Array.isArray(saved) && saved.length) {
        const known = new Set(state.meta.areas.map((a) => a.file));
        return new Set(saved.filter((f) => known.has(f)));
      }
    } catch { /* no storage, or stale JSON -- fall through */ }
    return defaultAreas();
  }

  function saveAreaChoice() {
    try {
      localStorage.setItem(AREA_STORAGE_KEY, JSON.stringify([...state.areasOn]));
    } catch { /* private mode: the choice just will not persist */ }
  }

  function renderAreaList() {
    const needle = el.areaSearch.value.trim().toLowerCase();
    const counts = new Map();
    for (const o of state.objects) {
      counts.set(o.a, (counts.get(o.a) || 0) + 1);
    }

    const frag = document.createDocumentFragment();
    for (const area of state.meta.areas) {
      if (needle
        && !area.file.toLowerCase().includes(needle)
        && !area.name.toLowerCase().includes(needle)) continue;

      const label = document.createElement('label');
      if (isImmortalArea(area)) label.classList.add('godly');

      const box = document.createElement('input');
      box.type = 'checkbox';
      box.value = area.file;
      box.checked = state.areasOn.has(area.file);

      const name = html('span', 'aname');
      name.textContent = area.name || area.file;
      name.title = `${area.file} — ${counts.get(area.file) || 0} предметів`;

      const band = html('span', 'aband');
      band.textContent = area.lo === null ? '—' : `${area.lo}-${area.hi}`;

      label.append(box, name, band);
      frag.appendChild(label);
    }
    el.areaList.replaceChildren(frag);
    el.areaSummary.textContent =
      `— увімкнено ${state.areasOn.size} з ${state.meta.areas.length}`;
  }

  /* ------------------------------------------------------------ filtering */

  function wearNumOf(slotKey) {
    const slot = state.meta.slots.find((s) => s.key === slotKey);
    return slot ? slot.wear_num : -1;
  }

  function candidates(slotKey) {
    const level = Number(el.level.value) || 1;
    const minLevel = Number(el.minlevel.value) || 0;
    const align = el.align.value;
    const needle = el.search.value.trim().toLowerCase();
    const anti = state.meta.anti;

    return (state.bySlot.get(slotKey) || []).filter((o) => {
      if (!state.areasOn.has(o.a)) return false;
      // act_obj.c:wear_obj() -- "ты должен достичь N уровня"
      if (o.l > level) return false;
      if (o.l < minLevel) return false;
      // handler.c:equip_char() -- anti-<align> gear is refused
      if (!MUD.alignAllows(o, align, anti)) return false;
      if (needle
        && !o.n.toLowerCase().includes(needle)
        && !(o.kw || '').toLowerCase().includes(needle)
        && String(o.v) !== needle) return false;
      return true;
    });
  }

  /* How good a piece is, used for sorting and for the autofill button.
     Weights follow what actually matters in ROM combat maths. */
  function score(obj, wearNum) {
    let s = 0;
    for (let i = 0; i < 4; i++) s += MUD.applyAc(obj, wearNum, i) * 0.5;
    for (const [loc, mod] of obj.af || []) {
      switch (loc) {
        case MUD.APPLY.HITROLL: s += mod * 4; break;
        case MUD.APPLY.DAMROLL: s += mod * 5; break;
        case MUD.APPLY.AC: s -= mod * 0.5; break;
        case MUD.APPLY.SAVES:
        case MUD.APPLY.SAVING_ROD:
        case MUD.APPLY.SAVING_PETRI:
        case MUD.APPLY.SAVING_BREATH:
        case MUD.APPLY.SAVING_SPELL: s -= mod * 2; break;
        case MUD.APPLY.HIT: s += mod * 0.2; break;
        case MUD.APPLY.MANA: s += mod * 0.15; break;
        case MUD.APPLY.MOVE: s += mod * 0.05; break;
        case MUD.APPLY.STR: case MUD.APPLY.DEX: case MUD.APPLY.CON:
        case MUD.APPLY.INT: case MUD.APPLY.WIS: s += mod * 3; break;
        default: break;
      }
    }
    if (obj.t === 'weapon') s += MUD.weaponAverage(obj) * 2;
    return s;
  }

  /* --------------------------------------------------------------- render */

  function render() {
    renderSlots();
    renderDoll();
    renderIdentify();
    renderTotals();
  }

  function renderSlots() {
    const frag = document.createDocumentFragment();
    let total = 0;

    state.meta.slots.forEach((slot, idx) => {
      const list = candidates(slot.key);
      total += list.length;
      const chosen = state.equipped[slot.key];

      const row = document.createElement('div');
      row.className = 'slot';
      row.dataset.slot = slot.key;
      row.dataset.idx = String(idx);
      if (state.activeSlot === slot.key) row.classList.add('active');
      if (chosen) row.classList.add('filled');

      const label = document.createElement('div');
      label.className = 'slot-label';
      label.textContent = slot.label;
      label.title = `${slot.label} (wear_num ${slot.wear_num})`;

      const select = document.createElement('select');
      select.dataset.slot = slot.key;

      const none = document.createElement('option');
      none.value = '';
      none.textContent = list.length
        ? `— порожньо (${list.length}) —`
        : '— немає варіантів —';
      select.appendChild(none);

      const wearNum = slot.wear_num;
      const sorted = list.slice().sort((a, b) =>
        b.l - a.l
        || score(b, wearNum) - score(a, wearNum)
        || a.n.localeCompare(b.n, 'uk'));
      for (const o of sorted) {
        const opt = document.createElement('option');
        opt.value = String(o.v);
        opt.textContent =
          `[${String(o.l).padStart(3)}] ${o.n} · ${o.a.replace(/\.are$/, '')}`;
        select.appendChild(opt);
      }

      // an item chosen before the filters narrowed still has to stay visible
      if (chosen && !sorted.some((o) => o.v === chosen.v)) {
        const opt = document.createElement('option');
        opt.value = String(chosen.v);
        opt.textContent = `[${String(chosen.l).padStart(3)}] ${chosen.n} (поза фільтром)`;
        select.appendChild(opt);
      }
      select.value = chosen ? String(chosen.v) : '';
      if (!chosen) select.classList.add('empty');

      const lvl = document.createElement('div');
      lvl.className = 'slot-lvl';
      if (chosen) {
        lvl.textContent = `ур.${chosen.l}`;
        if (chosen.l > Number(el.level.value)) lvl.classList.add('warn');
      }

      row.append(label, select, lvl);
      frag.appendChild(row);
    });

    el.slots.replaceChildren(frag);
    el.slotCount.textContent = `— ${total} доступних предметів`;
  }

  function renderDoll() {
    const klass = Number(el.klass.value) || 0;
    const pics = state.pictures[el.sex.value] || state.pictures.male;
    const picture = pics[klass] || pics[0];

    const frag = document.createDocumentFragment();
    state.meta.slots.forEach((slot, idx) => {
      const line = document.createElement('span');
      line.className = 'dl';
      line.dataset.slot = slot.key;
      line.title = slot.label;
      if (state.equipped[slot.key]) line.classList.add('filled');
      if (state.activeSlot === slot.key) line.classList.add('active');
      line.innerHTML = MUD.colourizePicture(picture[idx] || '') || '&nbsp;';
      frag.appendChild(line);
    });
    // the picture has more lines than the slot list uses; show the tail too
    for (let i = state.meta.slots.length; i < picture.length; i++) {
      if (!picture[i] || !picture[i].trim()) continue;
      const line = document.createElement('span');
      line.className = 'dl';
      line.innerHTML = MUD.colourizePicture(picture[i]);
      frag.appendChild(line);
    }
    el.doll.replaceChildren(frag);
    renderDollHint();
  }

  function renderDollHint() {
    const active = state.activeSlot
      ? state.meta.slots.find((s) => s.key === state.activeSlot)
      : null;
    el.dollHint.textContent = active
      ? `Рядок ${active.label} — ${state.equipped[active.key]
          ? state.equipped[active.key].n : 'порожньо'}`
      : 'Рядок силуету відповідає слоту — клікни, щоб перейти.';
  }

  function dl(pairs) {
    const d = document.createElement('dl');
    for (const [k, v] of pairs) {
      if (v === null || v === undefined || v === '') continue;
      const dt = document.createElement('dt');
      dt.textContent = k;
      const dd = document.createElement('dd');
      if (v instanceof Node) dd.appendChild(v);
      else dd.textContent = String(v);
      d.append(dt, dd);
    }
    return d;
  }

  function html(tag, cls, inner) {
    const n = document.createElement(tag);
    if (cls) n.className = cls;
    if (inner !== undefined) n.innerHTML = inner;
    return n;
  }

  /* Modelled on magic2.c:spell_identify(). */
  function renderIdentify() {
    const o = state.selected;
    if (!o) {
      el.identify.replaceChildren(
        html('p', 'muted', 'Обери предмет у будь-якому слоті.'));
      return;
    }

    const frag = document.createDocumentFragment();

    frag.appendChild(html('div', 'iname',
      MUD.colourize(MUD.gramCase(o.sr || o.n, 1))));

    frag.appendChild(dl([
      ['тип', ua(TYPE_UA, o.t)],
      ['рівень', o.l],
      ['вага', (o.w / 10).toFixed(1)],
      ['вартість', o.c.toLocaleString('uk')],
      ['keywords', o.kw || ''],
      ['vnum', o.v],
      ['область', `${o.a}${areaBand(o.a)}`],
    ]));

    // ---- type-specific block
    if (o.t === 'weapon') {
      const cls = state.meta.weapon_class[o.val[0]] || 'exotic';
      const avg = MUD.weaponAverage(o);
      frag.appendChild(html('h3', null, 'Зброя'));
      frag.appendChild(dl([
        ['клас', ua(WEAPON_CLASS_UA, cls)],
        ['пошкодження', `${o.val[1]}d${o.val[2]} (у середньому ${avg})`],
        ['тип атаки', o.atk || '—'],
      ]));
      const wflags = MUD.flagNames(o.val[4], state.meta.weapon_flag_names);
      if (wflags.length) frag.appendChild(tags(wflags));
    } else if (o.t === 'armor') {
      frag.appendChild(html('h3', null, 'Броня'));
      frag.appendChild(dl([
        ['pierce / bash / slash / magic',
          `${o.val[0]} / ${o.val[1]} / ${o.val[2]} / ${o.val[3]}`],
      ]));
    } else if (o.t === 'wand' || o.t === 'staff') {
      frag.appendChild(html('h3', null, 'Заряди'));
      frag.appendChild(dl([
        ['рівень заклять', o.val[0]],
        ['зарядів', `${o.val[2]} з ${o.val[1]}`],
      ]));
    }

    // ---- applies
    const applies = (o.af || []).filter(([, mod]) => mod !== 0);
    if (applies.length) {
      frag.appendChild(html('h3', null, 'Модифікатори'));
      const ul = document.createElement('ul');
      for (const [loc, mod] of applies) {
        const li = document.createElement('li');
        const sign = mod > 0 ? '+' : '';
        // for AC and saves a lower number is the better one
        const better = (loc === MUD.APPLY.AC || (loc >= 20 && loc <= 24))
          ? mod < 0 : mod > 0;
        li.innerHTML = `${MUD.escapeHtml(ua(APPLY_UA, loc))} `
          + `<span class="${better ? 'pos' : 'neg'}">${sign}${mod}</span>`;
        ul.appendChild(li);
      }
      frag.appendChild(ul);
    }

    // ---- flags
    const extra = MUD.flagNames(o.ef, state.meta.extra_flag_names);
    if (extra.length) {
      frag.appendChild(html('h3', null, 'Прапорці'));
      frag.appendChild(tags(extra));
    }
    const wear = MUD.flagNames(o.wf, state.meta.wear_flag_names);
    if (wear.length) {
      frag.appendChild(html('h3', null, 'Носиться як'));
      frag.appendChild(tags(wear));
    }

    // ---- what this piece contributes where it sits
    const slotKey = Object.keys(state.equipped)
      .find((k) => state.equipped[k] && state.equipped[k].v === o.v);
    if (slotKey) {
      const wearNum = wearNumOf(slotKey);
      const ac = [0, 1, 2, 3].map((i) => -MUD.applyAc(o, wearNum, i));
      if (ac.some((x) => x !== 0)) {
        frag.appendChild(html('h3', null, 'Внесок у AC на цьому слоті'));
        frag.appendChild(dl([['pierce / bash / slash / magic', ac.join(' / ')]]));
      } else if (o.t === 'armor') {
        frag.appendChild(html('h3', null, 'Внесок у AC на цьому слоті'));
        frag.appendChild(html('p', 'hint',
          'handler.c:apply_ac() не має гілки для цього слота — '
          + 'значення броні тут не рахується, працюють лише applies.'));
      }
    }

    el.identify.replaceChildren(frag);
  }

  function areaBand(file) {
    const area = state.meta.areas.find((a) => a.file === file);
    if (!area || area.lo === null) return '';
    return ` (${area.lo}-${area.hi})`;
  }

  function tags(names) {
    const box = document.createElement('div');
    for (const n of names) {
      const t = html('span', 'tag' + (DANGEROUS.has(n) ? ' danger' : ''));
      t.textContent = ua(FLAG_UA, n);
      t.title = n;
      box.appendChild(t);
    }
    return box;
  }

  function stat(label, value, small) {
    const s = html('div', 'stat' + (small ? ' small' : ''));
    const b = document.createElement('b');
    b.textContent = value;
    const sp = document.createElement('span');
    sp.textContent = label;
    s.append(b, sp);
    return s;
  }

  function renderTotals() {
    const level = Number(el.level.value) || 1;
    const equippedByWearNum = {};
    for (const [key, obj] of Object.entries(state.equipped)) {
      if (obj) equippedByWearNum[wearNumOf(key)] = obj;
    }
    const t = MUD.computeTotals(equippedByWearNum);

    const frag = document.createDocumentFragment();
    const sign = (n) => (n > 0 ? `+${n}` : String(n));

    frag.appendChild(stat('hitroll', sign(t.hitroll)));
    frag.appendChild(stat('damroll', sign(t.damroll)));
    frag.appendChild(html('div', 'sep'));

    frag.appendChild(stat('ac pierce', t.armor[0], true));
    frag.appendChild(stat('ac bash', t.armor[1], true));
    frag.appendChild(stat('ac slash', t.armor[2], true));
    frag.appendChild(stat('ac magic', t.armor[3], true));
    frag.appendChild(html('div', 'sep'));

    // do_score prints -1 * calc_saves(ch) with saving_throw in brackets
    const saves = -MUD.calcSaves(t.saving_throw, level);
    frag.appendChild(stat('saves', `${saves} (${sign(t.saving_throw)})`));
    frag.appendChild(html('div', 'sep'));

    frag.appendChild(stat('hp', sign(t.hit), true));
    frag.appendChild(stat('mana', sign(t.mana), true));
    frag.appendChild(stat('mv', sign(t.move), true));
    frag.appendChild(html('div', 'sep'));

    const stats = [['str', t.str], ['dex', t.dex], ['int', t.int],
                   ['wis', t.wis], ['con', t.con]].filter(([, v]) => v);
    for (const [k, v] of stats) frag.appendChild(stat(k, sign(v), true));
    if (stats.length) frag.appendChild(html('div', 'sep'));

    frag.appendChild(stat('предметів', `${t.pieces}/${state.meta.slots.length}`, true));
    frag.appendChild(stat('вага', (t.weight / 10).toFixed(1), true));
    frag.appendChild(stat('вартість', t.cost.toLocaleString('uk'), true));

    const warnings = collectWarnings();
    if (warnings.length) {
      const w = html('div', 'warnings');
      w.textContent = '⚠ ' + warnings.join('  •  ');
      frag.appendChild(w);
    }

    el.totals.replaceChildren(frag);
  }

  /* Rules from act_obj.c:wear_obj() that a planner can usefully flag. */
  function collectWarnings() {
    const out = [];
    const rh = state.equipped.rhand, lh = state.equipped.lhand;
    const twoHands = state.meta.weapon_flag_names
      .find(([, n]) => n === 'twohands');
    const isTwoHanded = (o) => o && o.t === 'weapon' && twoHands
      && MUD.hasFlag(o.val[4], twoHands[0]);

    if (isTwoHanded(rh) && lh) {
      out.push('дворучна зброя у правій руці — ліва має бути вільна');
    }
    if (isTwoHanded(lh)) {
      out.push('дворучну зброю не можна тримати у лівій руці');
    }
    const shieldBit = state.meta.wear_flag_names.find(([, n]) => n === 'shield');
    const isShield = (o) => o && shieldBit && MUD.hasFlag(o.wf, shieldBit[0]);
    if (isShield(rh) && isShield(lh)) {
      out.push('два щити одночасно носити не можна');
    }
    const wieldBit = state.meta.wear_flag_names.find(([, n]) => n === 'wield');
    const isWeapon = (o) => o && wieldBit && MUD.hasFlag(o.wf, wieldBit[0]);
    if (isWeapon(rh) && isWeapon(lh)) {
      const cls = state.meta.weapon_class[lh.val[0]];
      if (cls !== 'dagger') {
        out.push('друга зброя у лівій руці — лише кинджал (крім воїнів)');
      } else {
        out.push('друга зброя потребує вміння dual');
      }
    }
    const level = Number(el.level.value) || 1;
    for (const [key, o] of Object.entries(state.equipped)) {
      if (o && o.l > level) {
        const slot = state.meta.slots.find((s) => s.key === key);
        out.push(`${slot.label}: потрібен ${o.l} рівень`);
      }
    }
    return out;
  }

  /* ---------------------------------------------------------------- actions */

  function pick(slotKey, vnum) {
    if (!vnum) {
      delete state.equipped[slotKey];
      if (state.selected && !Object.values(state.equipped)
        .some((o) => o && o.v === state.selected.v)) state.selected = null;
    } else {
      const obj = state.objects.find((o) => o.v === Number(vnum));
      if (!obj) return;
      state.equipped[slotKey] = obj;
      state.selected = obj;
    }
    state.activeSlot = slotKey;
    syncUrl();
    render();
  }

  function autofill() {
    for (const slot of state.meta.slots) {
      const list = candidates(slot.key);
      if (!list.length) continue;
      let best = null, bestScore = -Infinity;
      for (const o of list) {
        const s = score(o, slot.wear_num);
        if (s > bestScore) { bestScore = s; best = o; }
      }
      if (best) state.equipped[slot.key] = best;
    }
    // do not leave an illegal two-handed + off-hand pair behind
    const twoHands = state.meta.weapon_flag_names.find(([, n]) => n === 'twohands');
    const rh = state.equipped.rhand;
    if (rh && rh.t === 'weapon' && twoHands && MUD.hasFlag(rh.val[4], twoHands[0])) {
      delete state.equipped.lhand;
    }
    // put something useful in the identify pane straight away
    const firstFilled = state.meta.slots.find((s) => state.equipped[s.key]);
    if (firstFilled) {
      state.selected = state.equipped.rhand || state.equipped[firstFilled.key];
      state.activeSlot = state.equipped.rhand ? 'rhand' : firstFilled.key;
    }
    syncUrl();
    render();
  }

  function clearAll() {
    state.equipped = {};
    state.selected = null;
    state.activeSlot = null;
    syncUrl();
    render();
  }

  /* --------------------------------------------------------- URL round-trip */

  function syncUrl() {
    const p = new URLSearchParams();
    p.set('lvl', el.level.value);
    p.set('align', el.align.value);
    p.set('class', el.klass.value);
    p.set('sex', el.sex.value);
    const eq = Object.entries(state.equipped)
      .filter(([, o]) => o)
      .map(([k, o]) => `${k}:${o.v}`)
      .join(',');
    if (eq) p.set('eq', eq);
    history.replaceState(null, '', `${location.pathname}?${p}`);
  }

  function restoreFromUrl() {
    const p = new URLSearchParams(location.search);
    if (p.has('lvl')) el.level.value = p.get('lvl');
    if (p.has('align')) el.align.value = p.get('align');
    if (p.has('class')) el.klass.value = p.get('class');
    if (p.has('sex')) el.sex.value = p.get('sex');
    for (const pair of (p.get('eq') || '').split(',')) {
      if (!pair) continue;
      const [key, vnum] = pair.split(':');
      const obj = state.objects.find((o) => o.v === Number(vnum));
      if (obj && state.bySlot.has(key)) state.equipped[key] = obj;
    }
  }

  /* ------------------------------------------------------------------ wire */

  function bind() {
    el.slots.addEventListener('change', (ev) => {
      const sel = ev.target.closest('select[data-slot]');
      if (sel) pick(sel.dataset.slot, sel.value);
    });
    el.slots.addEventListener('focusin', (ev) => {
      const row = ev.target.closest('.slot');
      if (!row) return;
      state.activeSlot = row.dataset.slot;
      const chosen = state.equipped[row.dataset.slot];
      if (chosen) state.selected = chosen;
      renderIdentify();
      markActive();
    });
    el.doll.addEventListener('click', (ev) => {
      const line = ev.target.closest('.dl[data-slot]');
      if (!line) return;
      const sel = el.slots.querySelector(
        `select[data-slot="${CSS.escape(line.dataset.slot)}"]`);
      if (sel) sel.focus();
    });

    for (const node of [el.level, el.align, el.minlevel, el.search]) {
      node.addEventListener('input', () => { syncUrl(); render(); });
    }
    for (const node of [el.klass, el.sex]) {
      node.addEventListener('change', () => { syncUrl(); renderDoll(); });
    }
    el.areaList.addEventListener('change', (ev) => {
      const box = ev.target.closest('input[type=checkbox]');
      if (!box) return;
      if (box.checked) state.areasOn.add(box.value);
      else state.areasOn.delete(box.value);
      saveAreaChoice();
      renderAreaList();
      render();
    });
    el.areaSearch.addEventListener('input', renderAreaList);
    el.areaFilter.addEventListener('click', (ev) => {
      const btn = ev.target.closest('button[data-area-action]');
      if (!btn) return;
      const action = btn.dataset.areaAction;
      if (action === 'all') {
        state.areasOn = new Set(state.meta.areas.map((a) => a.file));
      } else if (action === 'none') {
        state.areasOn = new Set();
      } else {
        state.areasOn = defaultAreas();
      }
      saveAreaChoice();
      renderAreaList();
      render();
    });
    $('#autofill').addEventListener('click', autofill);
    $('#clear').addEventListener('click', clearAll);
    $('#share').addEventListener('click', async (ev) => {
      syncUrl();
      try {
        await navigator.clipboard.writeText(location.href);
        ev.target.textContent = 'Скопійовано';
      } catch {
        ev.target.textContent = 'Скопіюй з адресного рядка';
      }
      setTimeout(() => { ev.target.textContent = 'Посилання'; }, 1600);
    });
  }

  function markActive() {
    for (const node of el.slots.querySelectorAll('.slot')) {
      node.classList.toggle('active', node.dataset.slot === state.activeSlot);
    }
    for (const node of el.doll.querySelectorAll('.dl')) {
      node.classList.toggle('active', node.dataset.slot === state.activeSlot);
    }
    renderDollHint();
  }

  /* ------------------------------------------------------------------- boot */

  load().then(() => {
    state.meta.classes.forEach((name, i) => {
      const opt = document.createElement('option');
      opt.value = String(i);
      opt.textContent = `${state.meta.classes_ru[i]} (${name})`;
      el.klass.appendChild(opt);
    });
    state.areasOn = loadAreaChoice();
    restoreFromUrl();
    bind();
    renderAreaList();
    render();
    el.loading.classList.add('hidden');
  }).catch((err) => {
    el.loading.classList.add('error');
    el.loading.textContent = String(err.message || err);
  });
})();
