#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Extracts equipment data out of the FDungeon area files and the ASCII body
pictures out of mud/const.c, and writes them as JSON for the web planner
(tools/eqplanner/web).

The .are reader mimics db.c:load_objects() / fread_* so that what the planner
shows is what the MUD would actually load.

Usage:  python3 tools/eqplanner/extract_data.py [--repo <path>] [--out <dir>]
"""

import argparse
import json
import os
import re
import sys

ENCODING = "cp1251"          # area files and const.c are CP1251
FALLBACK = "replace"

# ---------------------------------------------------------------------------
# constants mirrored from mud/merc_def.h
# ---------------------------------------------------------------------------

ITEM_TYPES = {
    1: "light", 2: "scroll", 3: "wand", 4: "staff", 5: "weapon",
    8: "treasure", 9: "armor", 10: "potion", 11: "clothing", 12: "furniture",
    13: "trash", 15: "container", 17: "drink", 18: "key", 19: "food",
    20: "money", 22: "boat", 23: "npc_corpse", 24: "pc_corpse",
    25: "fountain", 26: "pill", 27: "protect", 28: "map", 29: "portal",
    30: "warp_stone", 31: "room_key", 32: "gem", 33: "jewelry", 34: "jukebox",
    35: "enchanter", 36: "scuba", 37: "bonus",
}
ITEM_TYPE_BY_NAME = {
    "light": 1, "scroll": 2, "wand": 3, "staff": 4, "weapon": 5,
    "treasure": 8, "armor": 9, "potion": 10, "clothing": 11, "furniture": 12,
    "trash": 13, "container": 15, "drinkcontainer": 17, "drink": 17,
    "key": 18, "food": 19, "money": 20, "boat": 22, "npccorpse": 23,
    "pc corpse": 24, "fountain": 25, "pill": 26, "protect": 27, "map": 28,
    "portal": 29, "warpstone": 30, "roomkey": 31, "gem": 32, "jewelry": 33,
    "jukebox": 34, "enchanter": 35, "scuba": 36, "bonus": 37,
}

# item types whose value[] fields are not plain flags (load_objects switch)
TYPED_VALUES = {
    5: "weapon", 15: "container", 17: "drink", 25: "drink",
    3: "wand", 4: "wand", 10: "spells", 26: "spells", 2: "spells", 1: "light",
}

WEAPON_CLASS = ["exotic", "sword", "dagger", "spear", "mace", "axe",
                "flail", "whip", "polearm", "staff"]

# wear_flags bits, letter A..  (flag_convert: A=1<<0 ... Z=1<<25)
def bit(letter):
    if 'A' <= letter <= 'Z':
        return 1 << (ord(letter) - ord('A'))
    return 1 << (26 + ord(letter) - ord('a'))

WEAR_TAKE, WEAR_FINGER, WEAR_NECK_F, WEAR_BODY_F, WEAR_HEAD_F = (
    bit('A'), bit('B'), bit('C'), bit('D'), bit('E'))
WEAR_LEGS_F, WEAR_FEET_F, WEAR_HANDS_F, WEAR_ARMS_F, WEAR_SHIELD_F = (
    bit('F'), bit('G'), bit('H'), bit('I'), bit('J'))
WEAR_ABOUT_F, WEAR_WAIST_F, WEAR_WRIST_F, WIELD_F, HOLD_F = (
    bit('K'), bit('L'), bit('M'), bit('N'), bit('O'))
WEAR_FLOAT_F = bit('Q')

EXTRA_FLAG_NAMES = [
    ('A', "glow"), ('B', "hum"), ('C', "dark"), ('D', "lock"), ('E', "evil"),
    ('F', "invis"), ('G', "magic"), ('H', "nodrop"), ('I', "bless"),
    ('J', "antigood"), ('K', "antievil"), ('L', "antineutral"),
    ('M', "noremove"), ('N', "inventory"), ('O', "nopurge"),
    ('P', "rotdeath"), ('Q', "visdeath"), ('S', "nonmetal"),
    ('T', "nolocate"), ('U', "meltdrop"), ('V', "hadtimer"),
    ('W', "sellextract"), ('X', "noident"), ('Y', "burnproof"),
    ('Z', "nouncurse"),
]
ANTI_GOOD, ANTI_EVIL, ANTI_NEUTRAL = bit('J'), bit('K'), bit('L')

WEAR_FLAG_NAMES = [
    ('A', "take"), ('B', "finger"), ('C', "neck"), ('D', "body"),
    ('E', "head"), ('F', "legs"), ('G', "feet"), ('H', "hands"),
    ('I', "arms"), ('J', "shield"), ('K', "about"), ('L', "waist"),
    ('M', "wrist"), ('N', "wield"), ('O', "hold"), ('P', "nosac"),
    ('Q', "float"),
]

WEAPON_FLAG_NAMES = [
    ('A', "flaming"), ('B', "frost"), ('C', "vampiric"), ('D', "sharp"),
    ('E', "vorpal"), ('F', "twohands"), ('G', "shocking"), ('H', "poison"),
    ('I', "missile"), ('J', "return"), ('K', "round"), ('L', "vamp_mana"),
]

APPLY_NAMES = {
    0: "none", 1: "strength", 2: "dexterity", 3: "intelligence",
    4: "wisdom", 5: "constitution", 6: "sex", 7: "class", 8: "level",
    9: "age", 10: "height", 11: "weight", 12: "mana", 13: "hp",
    14: "moves", 15: "gold", 16: "experience", 17: "armor class",
    18: "hit roll", 19: "damage roll", 20: "saves", 21: "save vs rod",
    22: "save vs petrification", 23: "save vs breath", 24: "save vs spell",
    25: "spell affect",
}

CONDITION = {'P': 1000, 'G': 900, 'A': 750, 'W': 500,
             'D': 250, 'B': 100, 'R': 0}


# ---------------------------------------------------------------------------
# fread_* equivalents working over an in-memory string
# ---------------------------------------------------------------------------

class AreaReader:
    """Character-stream reader replicating db.c's fread_* helpers."""

    def __init__(self, text):
        self.s = text
        self.i = 0
        self.n = len(text)

    def eof(self):
        return self.i >= self.n

    def getc(self):
        if self.i >= self.n:
            return ''
        c = self.s[self.i]
        self.i += 1
        return c

    def ungetc(self):
        if self.i > 0:
            self.i -= 1

    def skip_space(self):
        while self.i < self.n and self.s[self.i].isspace():
            self.i += 1

    def fread_letter(self):
        self.skip_space()
        return self.getc()

    def fread_string(self):
        """Reads up to the next '~'. db.c also skips leading whitespace."""
        self.skip_space()
        start = self.i
        end = self.s.find('~', start)
        if end < 0:
            self.i = self.n
            return self.s[start:]
        self.i = end + 1
        return self.s[start:end].replace('\n\r', '\n').replace('\r', '')

    def fread_word(self):
        self.skip_space()
        c = self.getc()
        if c in ("'", '"'):
            quote = c
            start = self.i
            end = self.s.find(quote, start)
            if end < 0:
                self.i = self.n
                return self.s[start:]
            self.i = end + 1
            return self.s[start:end]
        start = self.i - 1
        while self.i < self.n and not self.s[self.i].isspace():
            self.i += 1
        return self.s[start:self.i]

    def fread_number(self):
        self.skip_space()
        c = self.getc()
        negative = False
        if c == '+':
            c = self.getc()
        elif c == '-':
            negative = True
            c = self.getc()
        number = 0
        while c.isdigit():
            number = number * 10 + int(c)
            c = self.getc()
        if c == '|':
            number += self.fread_number()
        elif c != ' ':
            self.ungetc()
        return -number if negative else number

    def fread_flag(self):
        self.skip_space()
        c = self.getc()
        negative = False
        if c == '-':
            negative = True
            c = self.getc()
        number = 0
        if not c.isdigit():
            while c and (('A' <= c <= 'Z') or ('a' <= c <= 'z')):
                number += bit(c)
                c = self.getc()
        while c.isdigit():
            number = number * 10 + int(c)
            c = self.getc()
        if c == '|':
            number += self.fread_flag()
        elif c != ' ':
            self.ungetc()
        return -number if negative else number


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def decode_flags(value, table):
    return [name for letter, name in table if value & bit(letter)]


def gram_case(description, case_set=1):
    """Port of comm.c:gram_newformat() -- picks one grammatical case out of the
    'stem|end1|end2|...' short description format."""
    COPYTOBUF, FINDNEED, COPYEND, FINDNEXT = 1, 2, 3, 4
    phase = COPYTOBUF
    counter = case_set
    out = []
    for ch in description:
        if ch in (' ', '-'):
            phase = COPYTOBUF
        if phase != FINDNEED:
            counter = case_set
        if ch == '|':
            if phase == COPYEND:
                phase = FINDNEXT
            if phase == COPYTOBUF:
                phase = FINDNEED
            if phase == FINDNEED:
                counter -= 1
                if counter <= 0:
                    phase = COPYEND
        elif phase in (COPYEND, COPYTOBUF):
            out.append(ch)
    return ''.join(out)


COLOR_RE = re.compile(r'\{.')


def strip_colors(text):
    def repl(m):
        return '{' if m.group(0) == '{{' else ''
    return COLOR_RE.sub(repl, text)


# ---------------------------------------------------------------------------
# object parsing
# ---------------------------------------------------------------------------

def parse_objects(reader, area_name, warnings=None):
    """Replicates db.c:load_objects().

    The MUD aborts on a malformed entry; here a broken (usually truncated)
    object just ends the section and is reported through `warnings`, so one
    bad file cannot cost us the other 120 areas."""
    objects = []
    while True:
        mark = reader.i
        try:
            objects.append(_parse_one_object(reader, area_name))
        except _EndOfSection:
            break
        except Exception as exc:                      # noqa: BLE001
            if warnings is not None:
                warnings.append("%s: stopped after %d objects (%s)"
                                % (area_name, len(objects), exc))
            reader.i = mark
            break
    return objects


class _EndOfSection(Exception):
    pass


def _parse_one_object(reader, area_name):
    letter = reader.fread_letter()
    if letter != '#':
        raise ValueError("load_objects: '#' not found")
    vnum = reader.fread_number()
    if vnum == 0:
        raise _EndOfSection

    o = {"vnum": vnum, "area": area_name}
    o["name"] = reader.fread_string()
    short_raw = reader.fread_string()
    o["description"] = reader.fread_string()
    o["material"] = reader.fread_string()

    type_word = reader.fread_word().lower()
    item_type = ITEM_TYPE_BY_NAME.get(type_word, 0)
    o["item_type"] = item_type
    o["type_name"] = ITEM_TYPES.get(item_type, type_word or "unknown")
    o["extra_flags"] = reader.fread_flag()
    o["wear_flags"] = reader.fread_flag()

    v = [0, 0, 0, 0, 0]
    kind = TYPED_VALUES.get(item_type)
    if kind == "weapon":
        wname = reader.fread_word().lower()
        v[0] = WEAPON_CLASS.index(wname) if wname in WEAPON_CLASS else 0
        v[1] = reader.fread_number()
        v[2] = reader.fread_number()
        o["attack"] = reader.fread_word()
        v[3] = 0
        v[4] = reader.fread_flag()
    elif kind == "container":
        v[0] = reader.fread_number()
        v[1] = reader.fread_flag()
        v[2] = reader.fread_number()
        v[3] = reader.fread_number()
        v[4] = reader.fread_number()
    elif kind == "drink":
        v[0] = reader.fread_number()
        v[1] = reader.fread_number()
        o["liquid"] = reader.fread_word()
        v[3] = reader.fread_number()
        v[4] = reader.fread_number()
    elif kind == "wand":
        v[0] = reader.fread_number()
        v[1] = reader.fread_number()
        v[2] = reader.fread_number()
        o["spell"] = reader.fread_word()
        v[4] = reader.fread_number()
    elif kind == "spells":
        v[0] = reader.fread_number()
        o["spells"] = [reader.fread_word() for _ in range(4)]
    elif kind == "light":
        for k in range(5):
            v[k] = reader.fread_number()
        v[0] = v[1] = v[3] = v[4] = 0
    else:
        for k in range(5):
            v[k] = reader.fread_flag()
    o["value"] = v

    o["level"] = reader.fread_number()
    o["weight"] = reader.fread_number()
    o["cost"] = reader.fread_number()

    word = reader.fread_word()
    if re.fullmatch(r'[-+]?\d+', word):
        o["durability"] = int(word)
        cond_letter = reader.fread_letter()
    else:
        o["durability"] = 0
        cond_letter = word[0] if word else '?'
    o["condition"] = CONDITION.get(cond_letter, 200)

    affects, obj_flags, extra_descr = [], [], []
    while True:
        letter = reader.fread_letter()
        if letter == 'A':
            loc = reader.fread_number()
            mod = reader.fread_number()
            affects.append({"where": "object", "loc": loc, "mod": mod})
        elif letter == 'F':
            where_letter = reader.fread_letter()
            where = {'A': "affects", 'I': "immune",
                     'R': "resist", 'V': "vuln"}.get(where_letter, "affects")
            loc = reader.fread_number()
            mod = reader.fread_number()
            bits = reader.fread_flag()
            affects.append({"where": where, "loc": loc,
                            "mod": mod, "bits": bits})
            obj_flags.append(where)
        elif letter == 'E':
            kw = reader.fread_string()
            desc = reader.fread_string()
            extra_descr.append({"keyword": kw, "desc": desc})
        elif letter == 'T':
            o["ttype"] = reader.fread_flag()
            o["ttime"] = reader.fread_number()
        elif letter == '':
            break
        else:
            reader.ungetc()
            break

    o["affects"] = affects
    o["short_raw"] = short_raw
    o["short"] = strip_colors(gram_case(short_raw, 1)).strip()
    return o


CREDITS_RE = re.compile(r'<\s*(-?\d+)\s+(-?\d+)\s*>')

AREA_FLAG_NAMES = [
    ('A', "changed"), ('B', "added"), ('C', "loading"), ('D', "noquest"),
    ('E', "noreform"), ('F', "wizlock"), ('G', "law"), ('H', "savelock"),
]


def parse_areadata(reader, fname):
    """Replicates db.c:load_area() -- Name / Credits / Security / Flags."""
    area = {"file": fname, "name": fname, "credits": "",
            "lo": None, "hi": None, "security": 9, "flags": 0}
    while True:
        word = reader.fread_word()
        if not word:
            break
        key = word.lower()
        if key == "end":
            break
        if key == "name":
            area["name"] = strip_colors(reader.fread_string()).strip()
        elif key == "builders":
            area["builders"] = reader.fread_string().strip()
        elif key == "clan":
            reader.fread_string()
        elif key == "credits":
            credits = reader.fread_string()
            area["credits"] = strip_colors(credits).strip()
            m = CREDITS_RE.search(credits)
            if m:
                area["lo"], area["hi"] = int(m.group(1)), int(m.group(2))
        elif key == "vnums":
            area["min_vnum"] = reader.fread_number()
            area["max_vnum"] = reader.fread_number()
        elif key == "security":
            area["security"] = reader.fread_number()
        elif key == "flags":
            area["flags"] = reader.fread_number()
        elif key == "version":
            area["version"] = reader.fread_number()
        else:
            # unknown key: db.c does fread_to_eol()
            nl = reader.s.find('\n', reader.i)
            reader.i = len(reader.s) if nl < 0 else nl + 1
    return area


def parse_area_file(path, warnings=None):
    with open(path, 'rb') as fh:
        raw = fh.read()
    text = raw.decode(ENCODING, FALLBACK)
    area_name = os.path.basename(path)
    objects = []
    area = {"file": area_name, "name": area_name, "credits": "",
            "lo": None, "hi": None, "security": 9, "flags": 0}
    reader = AreaReader(text)
    while not reader.eof():
        letter = reader.fread_letter()
        if letter != '#':
            break
        word = reader.fread_word()
        if word.startswith('$'):
            break
        if word.upper() == "OBJECTS":
            objects.extend(parse_objects(reader, area_name, warnings))
        elif word.upper() == "AREADATA":
            try:
                area = parse_areadata(reader, area_name)
            except Exception as exc:                   # noqa: BLE001
                if warnings is not None:
                    warnings.append("%s: #AREADATA (%s)" % (area_name, exc))
            nxt = find_next_section(reader)
            if nxt is None:
                break
            reader.i = nxt
        else:
            # skip forward to the next top-level section marker
            nxt = find_next_section(reader)
            if nxt is None:
                break
            reader.i = nxt
    return objects, area


SECTION_RE = re.compile(
    r'^#(AREADATA|HELPS|MOBILES|OBJECTS|RESETS|ROOMS|SHOPS|SPECIALS|MOBPROGS|\$)',
    re.MULTILINE)


def find_next_section(reader):
    m = SECTION_RE.search(reader.s, reader.i)
    return m.start() if m else None


# ---------------------------------------------------------------------------
# slot model  (wear_l in mud/tables.c, see act_obj.c:wear_obj for hand slots)
# ---------------------------------------------------------------------------

SLOTS = [
    # key,        wear_num, ru label,        accepted wear_flags mask
    ("light",      0,  "Свет",              None),
    ("head",       6,  "Голова",            WEAR_HEAD_F),
    ("neck",       3,  "Шея",               WEAR_NECK_F),
    ("arms",      10,  "Руки",              WEAR_ARMS_F),
    ("about",     12,  "Тело (плащ)",       WEAR_ABOUT_F),
    ("body",       5,  "Торс",              WEAR_BODY_F),
    ("hands",      9,  "Кисти",             WEAR_HANDS_F),
    ("rhand",     16,  "Пр. рука",          WIELD_F | HOLD_F | WEAR_SHIELD_F),
    ("lhand",     19,  "Лев. рука",         WIELD_F | HOLD_F | WEAR_SHIELD_F),
    ("wrist_l",   14,  "Запястье Л",        WEAR_WRIST_F),
    ("wrist_r",   15,  "Запястье П",        WEAR_WRIST_F),
    ("finger_l",   1,  "Палец Л",           WEAR_FINGER),
    ("finger_r",   2,  "Палец П",           WEAR_FINGER),
    ("waist",     13,  "Пояс",              WEAR_WAIST_F),
    ("legs",       7,  "Ноги",              WEAR_LEGS_F),
    ("feet",       8,  "Ступни",            WEAR_FEET_F),
    ("float",     18,  "Парит рядом",       WEAR_FLOAT_F),
]


# act_obj.c:wear_obj() is a chain of early-returning branches, so an object
# lands in the FIRST group whose flag it carries -- a sword flagged
# `AFMNT` (take/legs/wrist/wield) is a weapon and nothing else, because the
# wield branch returns before the legs branch is ever reached.
WEAR_ORDER = [
    (["rhand", "lhand"], WEAR_SHIELD_F | WIELD_F | HOLD_F),
    (["finger_r", "finger_l"], WEAR_FINGER),
    (["neck"], WEAR_NECK_F),
    (["body"], WEAR_BODY_F),
    (["head"], WEAR_HEAD_F),
    (["legs"], WEAR_LEGS_F),
    (["feet"], WEAR_FEET_F),
    (["hands"], WEAR_HANDS_F),
    (["arms"], WEAR_ARMS_F),
    (["about"], WEAR_ABOUT_F),
    (["waist"], WEAR_WAIST_F),
    (["wrist_r", "wrist_l"], WEAR_WRIST_F),
    (["float"], WEAR_FLOAT_F),
]


def slots_for(obj):
    """Which UI slot(s) this object can occupy, following wear_obj()'s order."""
    # the ITEM_LIGHT branch comes before every flag test
    if obj["item_type"] == 1:
        return ["light"]
    wf = obj["wear_flags"]
    for keys, mask in WEAR_ORDER:
        if wf & mask:
            return list(keys)
    return []


# ---------------------------------------------------------------------------
# ASCII silhouettes out of mud/const.c
# ---------------------------------------------------------------------------

PIC_LINES = 20      # MAX_WEAR
CLASSES = ["mage", "cleric", "thief", "warrior"]
CLASS_RU = ["Маг", "Клирик", "Вор", "Воин"]

C_STRING_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')


def unescape_c(s):
    out, i = [], 0
    mapping = {'n': '\n', 'r': '\r', 't': '\t', '\\': '\\',
               '"': '"', "'": "'", '0': '\0'}
    while i < len(s):
        if s[i] == '\\' and i + 1 < len(s):
            out.append(mapping.get(s[i + 1], s[i + 1]))
            i += 2
        else:
            out.append(s[i])
            i += 1
    return ''.join(out)


def _scan_array_body(text, start):
    """Returns the body of a brace block starting at `start` (just past the
    opening brace), counting braces only outside of C string literals -- the
    pictures themselves contain '{' colour codes."""
    depth, i, n = 1, start, len(text)
    while i < n and depth:
        c = text[i]
        if c == '"':
            i += 1
            while i < n:
                if text[i] == '\\':
                    i += 2
                    continue
                if text[i] == '"':
                    break
                i += 1
            i += 1
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            i = text.find('\n', i)
            if i < 0:
                break
            continue
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
        i += 1
    return text[start:i - 1]


def _split_entries(body):
    """Yields each top-level `{ ... }` entry of an array body, ignoring braces
    that live inside string literals (the colour codes)."""
    i, n = 0, len(body)
    while i < n:
        c = body[i]
        if c == '"':
            i += 1
            while i < n:
                if body[i] == '\\':
                    i += 2
                    continue
                if body[i] == '"':
                    break
                i += 1
            i += 1
            continue
        if c == '{':
            entry = _scan_array_body(body, i + 1)
            yield entry
            i += 1 + len(entry) + 1
            continue
        i += 1


def parse_pictures(const_c_path):
    """Pulls where_name_male / where_name_female out of mud/const.c.

    Each entry is one class (mage, cleric, thief, warrior) and holds one line
    of the body picture per wear slot -- act_info.c prints picture[iWear] in
    front of the slot's name, which is exactly how the planner lines the
    silhouette up with the slot dropdowns."""
    with open(const_c_path, 'rb') as fh:
        text = fh.read().decode(ENCODING, FALLBACK)

    result = {}
    for var, sex in (("where_name_male", "male"),
                     ("where_name_female", "female")):
        m = re.search(r'\b' + var + r'\s*\[\s*\]\s*=\s*\{', text)
        if not m:
            raise ValueError("picture array %s not found" % var)
        body = _scan_array_body(text, m.end())
        pics = []
        for entry in _split_entries(body):
            lines = [unescape_c(x) for x in C_STRING_RE.findall(entry)]
            if not lines:
                continue
            # picture[] is char*[MAX_WEAR]; short initialisers leave NULLs
            lines = (lines + [""] * PIC_LINES)[:PIC_LINES]
            pics.append(lines)
        if len(pics) != len(CLASSES):
            raise ValueError("%s: got %d pictures, expected %d"
                             % (var, len(pics), len(CLASSES)))
        result[sex] = pics
    return result


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def read_area_list(areas_dir):
    """The MUD boots exactly the files listed in areas/area.lst."""
    lst = os.path.join(areas_dir, "area.lst")
    names = []
    if os.path.isfile(lst):
        with open(lst, "rb") as fh:
            for line in fh.read().decode(ENCODING, FALLBACK).splitlines():
                line = line.strip()
                if not line or line.startswith("$"):
                    continue
                if line.lower().endswith(".are"):
                    names.append(line)
    return names


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    repo_default = os.path.abspath(os.path.join(here, "..", ".."))

    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--repo", default=repo_default,
                    help="path to the fdungeon checkout")
    ap.add_argument("--out", default=os.path.join(here, "web", "data"),
                    help="directory to write the JSON files into")
    ap.add_argument("--all", action="store_true",
                    help="scan every areas/*.are instead of only the files "
                         "listed in areas/area.lst")
    ap.add_argument("--pretty", action="store_true",
                    help="pretty-print the JSON (bigger files)")
    args = ap.parse_args()

    areas_dir = os.path.join(args.repo, "areas")
    const_c = os.path.join(args.repo, "mud", "const.c")
    os.makedirs(args.out, exist_ok=True)

    on_disk = sorted(f for f in os.listdir(areas_dir) if f.endswith(".are"))
    if args.all:
        are_files = on_disk
    else:
        are_files = [f for f in read_area_list(areas_dir) if f in on_disk]
        if not are_files:
            are_files = on_disk

    all_objects, warnings, areas = [], [], {}
    for fname in are_files:
        path = os.path.join(areas_dir, fname)
        try:
            objs, area = parse_area_file(path, warnings)
            all_objects.extend(objs)
            areas[fname] = area
        except Exception as exc:                      # noqa: BLE001
            warnings.append("%s: %s" % (fname, exc))

    # keep only what can actually be worn / wielded, drop duplicate vnums
    wearable, seen = [], set()
    for o in all_objects:
        if o["vnum"] in seen:
            continue
        slots = slots_for(o)
        if not slots:
            continue
        seen.add(o["vnum"])
        rec = {
            "v": o["vnum"],
            "n": o["short"] or strip_colors(o["name"]).strip(),
            "sr": o["short_raw"],
            "kw": o["name"],
            "l": o["level"],
            "t": o["type_name"],
            "w": o["weight"],
            "c": o["cost"],
            "ef": o["extra_flags"],
            "wf": o["wear_flags"],
            "val": o["value"],
            "af": [[a["loc"], a["mod"]] for a in o["affects"]
                   if a["where"] == "object" and a["loc"] not in (0, 25)
                   and a["mod"] != 0],
            "fa": [[a["where"], a["loc"], a["mod"], a.get("bits", 0)]
                   for a in o["affects"] if a["where"] != "object"],
            "s": slots,
            "a": o["area"],
        }
        if "attack" in o:
            rec["atk"] = o["attack"]
        wearable.append(rec)

    wearable.sort(key=lambda r: (r["l"], r["v"]))

    used = {r["a"] for r in wearable}
    area_list = [dict(areas[f], flag_names=decode_flags(areas[f]["flags"],
                                                        AREA_FLAG_NAMES))
                 for f in are_files if f in areas and f in used]
    area_list.sort(key=lambda a: a["file"])

    meta = {
        "areas": area_list,
        "slots": [{"key": k, "wear_num": w, "label": lbl}
                  for k, w, lbl, _m in SLOTS],
        "classes": CLASSES,
        "classes_ru": CLASS_RU,
        "extra_flag_names": [[bit(l), n] for l, n in EXTRA_FLAG_NAMES],
        "wear_flag_names": [[bit(l), n] for l, n in WEAR_FLAG_NAMES],
        "weapon_flag_names": [[bit(l), n] for l, n in WEAPON_FLAG_NAMES],
        "weapon_class": WEAPON_CLASS,
        "apply_names": APPLY_NAMES,
        "anti": {"good": ANTI_GOOD, "evil": ANTI_EVIL,
                 "neutral": ANTI_NEUTRAL},
        "counts": {"areas": len(are_files), "objects": len(all_objects),
                   "wearable": len(wearable)},
    }

    dump = (lambda o: json.dumps(o, ensure_ascii=False, indent=1)) if args.pretty \
        else (lambda o: json.dumps(o, ensure_ascii=False, separators=(',', ':')))

    with open(os.path.join(args.out, "objects.json"), "w",
              encoding="utf-8") as fh:
        fh.write(dump({"meta": meta, "objects": wearable}))

    pictures = parse_pictures(const_c)
    with open(os.path.join(args.out, "pictures.json"), "w",
              encoding="utf-8") as fh:
        fh.write(dump(pictures))

    print("areas parsed : %d" % len(are_files))
    print("objects read : %d" % len(all_objects))
    print("wearable kept: %d" % len(wearable))
    print("pictures     : male=%d female=%d, %d lines each"
          % (len(pictures["male"]), len(pictures["female"]), PIC_LINES))
    for w in warnings:
        print("  ! %s" % w, file=sys.stderr)


if __name__ == "__main__":
    main()
