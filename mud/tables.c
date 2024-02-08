// Copyrights (C) 1998-2003, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

struct quenia_words quenia_table[] =
{
//   name        word  descr      long               start  counter
  { Q_RESTORE,    "", "restore",  "Magic Restore",   3,     0},
  { Q_GOODSPELL,  "", "enforce",  "God Spells",      3,     0},
  { Q_BADSPELL,   "", "curse",    "Devil Spells",    2,     0},
  { Q_POWER,      "", "power",    "Power word",     10,     0},
  { Q_BLESS,      "", "bless",    "Blessing",        5,     0},
  { Q_FIREPROOF,  "", "fireproof","Fireproof",       5,     0},
  { Q_PEACE,      "", "peace",    "Peace word",      3,     0},
  { Q_END,        "", "",         "END of table",  255,     0},
  { Q_END,        "", "BUG",      "",              255,     0},
  { Q_END,        "", "BUG",      "",              255,     0},
};

const struct clan_rank_type clan_ranks[7] =
{
 { "member" },
 { "junior" },
 { "senior" },
 { "{Wd{xeputy" },
 { "{Wsecond{x" },
 { "{WLeader{x" },
 { "{WFirst {x" }
};

// for position
const struct position_type position_table[] =
{
    {   "dead",                 "dead"  },
    {   "mortally wounded",     "mort"  },
    {   "incapacitated",        "incap" },
    {   "stunned",              "stun"  },
    {   "sleeping",             "sleep" },
    {   "resting",              "rest"  },
    {   "sitting",              "sit"   },
    {   "fighting",             "fight" },
    {   "standing",             "stand" },
    {   "",                     "bug"   }, 
    {   NULL,                   NULL    }
};

// for sex
const struct sex_type sex_table[] =
{
   {    "none"          },
   {    "male"          },
   {    "female"        },
   {    "either"        },
   {    NULL            }
};

// for sizes
const struct size_type size_table[] =
{ 
    {   "tiny"          },
    {   "small"         },
    {   "medium"        },
    {   "large"         },
    {   "huge",         },
    {   "giant"         },
    {   NULL            }
};

// various flag tables
const struct flag_type act_flags[] =
{
    {   "npc",                  A,      FALSE   },
    {   "sentinel",             B,      TRUE    },
    {   "scavenger",            C,      TRUE    },
    {   "keeper",               D,      TRUE    },
    {   "assasin_master",       E,      TRUE    },
    {   "aggressive",           F,      TRUE    },
    {   "stay_area",            G,      TRUE    },
    {   "wimpy",                H,      TRUE    },
    {   "pet",                  I,      TRUE    },
    {   "train",                J,      TRUE    },
    {   "practice",             K,      TRUE    },
    {   "account",              L,      TRUE    },
    {   "noquest",              M,      TRUE    },
    {   "nosteal",              N,      TRUE    },
    {   "undead",               O,      TRUE    },
    {   "referi",               P,      TRUE    },
    {   "cleric",               Q,      TRUE    },
    {   "mage",                 R,      TRUE    },
    {   "thief",                S,      TRUE    },
    {   "warrior",              T,      TRUE    },
    {   "noalign",              U,      TRUE    },
    {   "nopurge",              V,      TRUE    },
    {   "stopdoor",             W,      TRUE    },
    {   "clanenchanter",        X,      TRUE    },
    {   "forger",               Y,      TRUE    },
    {   "stay_sector",          Z,      TRUE    },
    {   "healer",               aa,     TRUE    },
    {   "gain",                 bb,     TRUE    },
    {   "update_always",        cc,     TRUE    },
    {   "changer",              dd,     TRUE    },
    {   "extract_corpse",       ee,     TRUE    },
    {   NULL,                   0,      FALSE   }
};

const struct flag_type mprog_flags[] =
{
  { "act",    TRIG_ACT,   TRUE },
  { "bribe",  TRIG_BRIBE, TRUE },
  { "death",  TRIG_DEATH, TRUE },
  { "entry",  TRIG_ENTRY, TRUE },
  { "fight",  TRIG_FIGHT, TRUE },
  { "give",   TRIG_GIVE,  TRUE },
  { "greet",  TRIG_GREET, TRUE },
  { "grall",  TRIG_GRALL, TRUE },
  { "kill",   TRIG_KILL,  TRUE },
  { "hpcnt",  TRIG_HPCNT, TRUE },
  { "random", TRIG_RANDOM,TRUE },
  { "speech", TRIG_SPEECH,TRUE },
  { "exit",   TRIG_EXIT,  TRUE },
  { "exall",  TRIG_EXALL, TRUE },
  { "delay",  TRIG_DELAY, TRUE },
  { "surr",   TRIG_SURR,  TRUE },
  {  NULL,    0,          0    }
};


const struct flag_type plr_flags[] =
{
    {   "npc",         A,  FALSE },
    {   "tipsy",       B,  TRUE  },
    {   "autoassist",  C,  FALSE },
    {   "nopost",      D,  FALSE },
    {   "autoloot",    E,  FALSE },
    {   "autosac",     F,  FALSE },
    {   "autogold",    G,  FALSE },
    {   "lastremort",  H,  FALSE },
    {   "plr_questor", M,  FALSE },
    {   "holylight",   N,  FALSE },
    {   "can_loot",    P,  FALSE },
    {   "nosummon",    Q,  FALSE },
    {   "nofollow",    R,  FALSE },
    {   "nomlove",     S,  FALSE },
    {   "colour",      T,  FALSE },
    {   "permit",      U,  TRUE  },
    {   "5remort",     V,  TRUE  },
    {   "log",         W,  FALSE },
    {   "deny",        X,  TRUE  },
    {   "freeze",      Y,  TRUE  },
    {   "wanted",      Z,  FALSE },
    {   "mustdrink",   aa, TRUE  },
    {   "raper",       bb, TRUE  },
    {   "army",        ee, TRUE  },
    {   "nocancel",    cc, TRUE  },
    {   "nosend",      dd, TRUE  },
    {   "sifilis",     hh, TRUE  },
    {   "can_fly",     ii, TRUE  },
    {   NULL,          0,  0     }
};

const struct flag_type affect_flags[] =
{
    {   "blind",                A,      TRUE    },
    {   "invisible",            B,      TRUE    },
    {   "detect_evil",          C,      TRUE    },
    {   "detect_invis",         D,      TRUE    },
    {   "detect_magic",         E,      TRUE    },
    {   "detect_hidden",        F,      TRUE    },
    {   "detect_good",          G,      TRUE    },
    {   "sanctuary",            H,      TRUE    },
    {   "faerie_fire",          I,      TRUE    },
    {   "infrared",             J,      TRUE    },
    {   "curse",                K,      TRUE    },
    {   "swim",                 L,      TRUE    },
    {   "poison",               M,      TRUE    },
    {   "protect_evil",         N,      TRUE    },
    {   "protect_good",         O,      TRUE    },
    {   "sneak",                P,      TRUE    },
    {   "hide",                 Q,      TRUE    },
    {   "nostalgia",            R,      TRUE    },
    {   "charm",                S,      TRUE    },
    {   "flying",               T,      TRUE    },
    {   "pass_door",            U,      TRUE    },
    {   "haste",                V,      TRUE    },
    {   "calm",                 W,      TRUE    },
    {   "plague",               X,      TRUE    },
    {   "shield",               Y,      TRUE    },
    {   "dark_vision",          Z,      TRUE    },
    {   "berserk",              aa,     TRUE    },
    {   "fireshield",           bb,     TRUE    },
    {   "regeneration",         cc,     TRUE    },
    {   "slow",                 dd,     TRUE    },
    {   "mist",                 ee,     TRUE    },
    {   "frenzy",               gg,     TRUE    },
    {   "gaseous_form",         hh,     TRUE    },
    {   "ensnare",              kk,     TRUE    },
    {   "shield",               mm,     TRUE    },
    {   "wbreath",              nn,     TRUE    },    
    {   NULL,                   0,      0       }
};

const struct flag_type off_flags[] =
{
    {   "area_attack",          A,      TRUE    },
    {   "backstab",             B,      TRUE    },
    {   "bash",                 C,      TRUE    },
    {   "berserk",              D,      TRUE    },
    {   "disarm",               E,      TRUE    },
    {   "dodge",                F,      TRUE    },
    {   "fade",                 G,      TRUE    },
    {   "fast",                 H,      TRUE    },
    {   "kick",                 I,      TRUE    },
    {   "dirt_kick",            J,      TRUE    },
    {   "parry",                K,      TRUE    },
    {   "rescue",               L,      TRUE    },
    {   "tail",                 M,      TRUE    },
    {   "trip",                 N,      TRUE    },
    {   "crush",                O,      TRUE    },
    {   "assist_all",           P,      TRUE    },
    {   "assist_align",         Q,      TRUE    },
    {   "assist_race",          R,      TRUE    },
    {   "assist_players",       S,      TRUE    },
    {   "assist_guard",         T,      TRUE    },
    {   "assist_vnum",          U,      TRUE    },
    {   "dwarves_accounter",    Y,      TRUE    },
    {   "assasin_accounter",    Z,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type imm_flags[] =
{
    {   "summon",               A,      TRUE    },
    {   "charm",                B,      TRUE    },
    {   "magic",                C,      TRUE    },
    {   "weapon",               D,      TRUE    },
    {   "bash",                 E,      TRUE    },
    {   "pierce",               F,      TRUE    },
    {   "slash",                G,      TRUE    },
    {   "fire",                 H,      TRUE    },
    {   "cold",                 I,      TRUE    },
    {   "lightning",            J,      TRUE    },
    {   "acid",                 K,      TRUE    },
    {   "poison",               L,      TRUE    },
    {   "negative",             M,      TRUE    },
    {   "holy",                 N,      TRUE    },
    {   "energy",               O,      TRUE    },
    {   "mental",               P,      TRUE    },
    {   "disease",              Q,      TRUE    },
    {   "drowning",             R,      TRUE    },
    {   "light",                S,      TRUE    },
    {   "sound",                T,      TRUE    },
    {   "wood",                 X,      TRUE    },
    {   "silver",               Y,      TRUE    },
    {   "iron",                 Z,      TRUE    },
    {   "all",                  aa,     TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type form_flags[] =
{
    {   "edible",               FORM_EDIBLE,            TRUE    },
    {   "poison",               FORM_POISON,            TRUE    },
    {   "magical",              FORM_MAGICAL,           TRUE    },
    {   "instant_decay",        FORM_INSTANT_DECAY,     TRUE    },
    {   "other",                FORM_OTHER,             TRUE    },
    {   "animal",               FORM_ANIMAL,            TRUE    },
    {   "sentient",             FORM_SENTIENT,          TRUE    },
    {   "undead",               FORM_UNDEAD,            TRUE    },
    {   "construct",            FORM_CONSTRUCT,         TRUE    },
    {   "mist",                 FORM_MIST,              TRUE    },
    {   "intangible",           FORM_INTANGIBLE,        TRUE    },
    {   "biped",                FORM_BIPED,             TRUE    },
    {   "centaur",              FORM_CENTAUR,           TRUE    },
    {   "insect",               FORM_INSECT,            TRUE    },
    {   "spider",               FORM_SPIDER,            TRUE    },
    {   "crustacean",           FORM_CRUSTACEAN,        TRUE    },
    {   "worm",                 FORM_WORM,              TRUE    },
    {   "blob",                 FORM_BLOB,              TRUE    },
    {   "mammal",               FORM_MAMMAL,            TRUE    },
    {   "bird",                 FORM_BIRD,              TRUE    },
    {   "reptile",              FORM_REPTILE,           TRUE    },
    {   "snake",                FORM_SNAKE,             TRUE    },
    {   "dragon",               FORM_DRAGON,            TRUE    },
    {   "amphibian",            FORM_AMPHIBIAN,         TRUE    },
    {   "fish",                 FORM_FISH ,             TRUE    },
    {   "cold_blood",           FORM_COLD_BLOOD,        TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type part_flags[] =
{
    {   "head",                 PART_HEAD,              TRUE    },
    {   "arms",                 PART_ARMS,              TRUE    },
    {   "legs",                 PART_LEGS,              TRUE    },
    {   "heart",                PART_HEART,             TRUE    },
    {   "brains",               PART_BRAINS,            TRUE    },
    {   "guts",                 PART_GUTS,              TRUE    },
    {   "hands",                PART_HANDS,             TRUE    },
    {   "feet",                 PART_FEET,              TRUE    },
    {   "fingers",              PART_FINGERS,           TRUE    },
    {   "ear",                  PART_EAR,               TRUE    },
    {   "eye",                  PART_EYE,               TRUE    },
    {   "long_tongue",          PART_LONG_TONGUE,       TRUE    },
    {   "eyestalks",            PART_EYESTALKS,         TRUE    },
    {   "tentacles",            PART_TENTACLES,         TRUE    },
    {   "fins",                 PART_FINS,              TRUE    },
    {   "wings",                PART_WINGS,             TRUE    },
    {   "tail",                 PART_TAIL,              TRUE    },
    {   "claws",                PART_CLAWS,             TRUE    },
    {   "fangs",                PART_FANGS,             TRUE    },
    {   "horns",                PART_HORNS,             TRUE    },
    {   "scales",               PART_SCALES,            TRUE    },
    {   "tusks",                PART_TUSKS,             TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type comm_flags[] =
{
  {   "quiet",                COMM_QUIET,             TRUE    },
  {   "noemote",              COMM_NOEMOTE,           TRUE    },
  {   "deaf",                 COMM_DEAF,              TRUE    },
  {   "brief",                COMM_BRIEF,             TRUE    },
  {   "nogscoc",              COMM_NOGSOC,            TRUE    },
  {   "prompt",               COMM_PROMPT,            TRUE    },
  {   "nodelete",             COMM_NODELETE,          TRUE    },
  {   "telnet_ga",            COMM_TELNET_GA,         TRUE    },
  {   "notell",               COMM_NOTELL,            TRUE    },
  {   "nochannels",           COMM_NOCHANNELS,        TRUE    },
  {   "russian",              COMM_RUSSIAN,           TRUE    },
  {   "prompt",               COMM_PROMPT,            TRUE    },
  {   "afk",                  COMM_AFK,               TRUE    },
  {   "coder",                COMM_CODER,             TRUE    },
  {   "willsave",             COMM_WILLSAVE,          TRUE    },
  {   "flush",                COMM_FLUSH,             TRUE    },
  {   NULL,                   0,                      0       }
};

const struct flag_type area_flags[] =
{
    {   "none",                 AREA_NONE,              FALSE   },
    {   "changed",              AREA_CHANGED,           TRUE    },
    {   "added",                AREA_ADDED,             TRUE    },
    {   "loading",              AREA_LOADING,           FALSE   },
    {   "noquest",              AREA_NOQUEST,           FALSE   },
    {   "noreform",             AREA_NOREFORM,          FALSE   },
    {   "wizlock",              AREA_WIZLOCK,           FALSE   },
    {   "savelock",             AREA_SAVELOCK,          FALSE   },
    {   NULL,                   0,                      0       }
};

const struct flag_type sex_flags[] =
{
    {   "male",                 SEX_MALE,               TRUE    },
    {   "female",               SEX_FEMALE,             TRUE    },
    {   "neutral",              SEX_NEUTRAL,            TRUE    },
    {   "random",               3,                      TRUE    },   /* ROM */
    {   "none",                 SEX_NEUTRAL,            TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type exit_flags[] =
{
  { "door",          EX_ISDOOR,      TRUE },
  { "closed",        EX_CLOSED,      TRUE },
  { "locked",        EX_LOCKED,      TRUE },
  { "pickproof",     EX_PICKPROOF,   TRUE },
  { "nopass",        EX_NOPASS,      TRUE },
  { "easy",          EX_EASY,        TRUE },
  { "hard",          EX_HARD,        TRUE },
  { "infuriating",   EX_INFURIATING, TRUE },
  { "noclose",       EX_NOCLOSE,     TRUE },
  { "nolock",        EX_NOLOCK,      TRUE },
  { "dwarvesguild",  EX_DWARVESGUILD,TRUE },
  { NULL,            0,              0    }
};

const struct flag_type door_resets[] =
{
  { "open and unlocked",    0, TRUE  },
  { "closed and unlocked",  1, TRUE  },
  { "closed and locked",    2, TRUE  },
  { NULL,                   0, 0     }
};

const struct flag_type room_flags[] =
{
  {   "dark",           ROOM_DARK,           TRUE  },
  {   "nomagic",        ROOM_NO_MAGIC,       TRUE  },
  {   "no_mob",         ROOM_NO_MOB,         TRUE  },
  {   "indoors",        ROOM_INDOORS,        TRUE  },
  {   "hard",           ROOM_HARD,           TRUE  },
  {   "nomorph",        ROOM_NOMORPH,        TRUE  },
  {   "private",        ROOM_PRIVATE,        TRUE  },
  {   "deleted",        ROOM_DELETED,        TRUE  },
  {   "safe",           ROOM_SAFE,           TRUE  },
  {   "solitary",       ROOM_SOLITARY,       TRUE  },
  {   "pet_shop",       ROOM_PET_SHOP,       TRUE  },
  {   "no_recall",      ROOM_NO_RECALL,      TRUE  },
  {   "nosuicide",      ROOM_NOSUICIDE,      TRUE  },
  {   "gods_only",      ROOM_GODS_ONLY,      TRUE  },
  {   "heroes_only",    ROOM_HEROES_ONLY,    TRUE  },
  {   "newbies_only",   ROOM_NEWBIES_ONLY,   TRUE  },
  {   "law",            ROOM_LAW,            TRUE  },
  {   "nowhere",        ROOM_NOWHERE,        TRUE  },
  {   "arena",          ROOM_ARENA,          TRUE  },
  {   "noflee",         ROOM_NOFLEE,         TRUE  },
  {   "blacksmith",     ROOM_BLACKSMITH,     TRUE  },
  {   "dwarvesguild",   ROOM_DWARVES_GUILD,  TRUE  },
  {   "drent",          ROOM_DWARVES_RENT,   TRUE  },
  {   "mag_only",       ROOM_MAG_ONLY,       TRUE  },
  {   "war_only",       ROOM_WAR_ONLY,       TRUE  },
  {   "thi_only",       ROOM_THI_ONLY,       TRUE  },
  {   "cle_only",       ROOM_CLE_ONLY,       TRUE  },
  {   "marry",          ROOM_MARRY,          TRUE  },
  {   "allvisible",     ROOM_ALL_VIS,        TRUE  },
  {   "rforge",         ROOM_RFORGE,         TRUE  },
  {   "relder",         ROOM_ELDER,          TRUE  },
  {   NULL,             0,                   0     }
};

const struct flag_type sector_flags[] =
{
  {   "inside",       SECT_INSIDE,        TRUE  },
  {   "city",         SECT_CITY,          TRUE  },
  {   "field",        SECT_FIELD,         TRUE  },
  {   "forest",       SECT_FOREST,        TRUE  },
  {   "hills",        SECT_HILLS,         TRUE  },
  {   "mountain",     SECT_MOUNTAIN,      TRUE  },
  {   "swim",         SECT_WATER_SWIM,    TRUE  },
  {   "noswim",       SECT_WATER_NOSWIM,  TRUE  },
  {   "unused",       SECT_UNUSED,        TRUE  },
  {   "air",          SECT_AIR,           TRUE  },
  {   "desert",       SECT_DESERT,        TRUE  },
  {   "rock_mountain",SECT_ROCK_MOUNTAIN, TRUE  },
  {   "snow_mountain",SECT_SNOW_MOUNTAIN, TRUE  },
  {   "enter",        SECT_ENTER,         TRUE  },
  {   "road",         SECT_ROAD,          TRUE  },
  {   "swamp",        SECT_SWAMP,         TRUE  },
  {   "jungle",       SECT_JUNGLE,        TRUE  },
  {   "ruins",        SECT_RUINS,         TRUE  },
  {   "underwater",   SECT_UWATER,        TRUE  },
  {   NULL,           0,                  0     }
};

const struct flag_type type_flags[] =
{
    {   "light",                ITEM_LIGHT,             TRUE    },
    {   "scroll",               ITEM_SCROLL,            TRUE    },
    {   "wand",                 ITEM_WAND,              TRUE    },
    {   "staff",                ITEM_STAFF,             TRUE    },
    {   "weapon",               ITEM_WEAPON,            TRUE    },
    {   "treasure",             ITEM_TREASURE,          TRUE    },
    {   "armor",                ITEM_ARMOR,             TRUE    },
    {   "potion",               ITEM_POTION,            TRUE    },
    {   "furniture",            ITEM_FURNITURE,         TRUE    },
    {   "trash",                ITEM_TRASH,             TRUE    },
    {   "container",            ITEM_CONTAINER,         TRUE    },
    {   "drinkcontainer",       ITEM_DRINK_CON,         TRUE    },
    {   "key",                  ITEM_KEY,               TRUE    },
    {   "food",                 ITEM_FOOD,              TRUE    },
    {   "money",                ITEM_MONEY,             TRUE    },
    {   "boat",                 ITEM_BOAT,              TRUE    },
    {   "npccorpse",            ITEM_CORPSE_NPC,        TRUE    },
    {   "pc corpse",            ITEM_CORPSE_PC,         FALSE   },
    {   "fountain",             ITEM_FOUNTAIN,          TRUE    },
    {   "pill",                 ITEM_PILL,              TRUE    },
    {   "protect",              ITEM_PROTECT,           TRUE    },
    {   "map",                  ITEM_MAP,               TRUE    },
    {   "portal",               ITEM_PORTAL,            TRUE    },
    {   "warpstone",            ITEM_WARP_STONE,        TRUE    },
    {   "roomkey",              ITEM_ROOM_KEY,          TRUE    },
    {   "gem",                  ITEM_GEM,               TRUE    },
    {   "jewelry",              ITEM_JEWELRY,           TRUE    },
    {   "jukebox",              ITEM_JUKEBOX,           TRUE    },
    {   "enchanter",            ITEM_ENCHANT,           TRUE    },
    {   "scuba",                ITEM_SCUBA,             TRUE    },
    {   "bonus",                ITEM_BONUS,             TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type extra_flags[] =
{
    {   "glow",                 ITEM_GLOW,              TRUE    },
    {   "hum",                  ITEM_HUM,               TRUE    },
    {   "dark",                 ITEM_DARK,              TRUE    },
    {   "lock",                 ITEM_LOCK,              TRUE    },
    {   "evil",                 ITEM_EVIL,              TRUE    },
    {   "invis",                ITEM_INVIS,             TRUE    },
    {   "magic",                ITEM_MAGIC,             TRUE    },
    {   "nodrop",               ITEM_NODROP,            TRUE    },
    {   "bless",                ITEM_BLESS,             TRUE    },
    {   "antigood",             ITEM_ANTI_GOOD,         TRUE    },
    {   "antievil",             ITEM_ANTI_EVIL,         TRUE    },
    {   "antineutral",          ITEM_ANTI_NEUTRAL,      TRUE    },
    {   "noremove",             ITEM_NOREMOVE,          TRUE    },
    {   "inventory",            ITEM_INVENTORY,         TRUE    },
    {   "nopurge",              ITEM_NOPURGE,           TRUE    },
    {   "rotdeath",             ITEM_ROT_DEATH,         TRUE    },
    {   "visdeath",             ITEM_VIS_DEATH,         TRUE    },
    {   "nonmetal",             ITEM_NONMETAL,          TRUE    },
    {   "meltdrop",             ITEM_MELT_DROP,         TRUE    },
    {   "hadtimer",             ITEM_HAD_TIMER,         TRUE    },
    {   "sellextract",          ITEM_SELL_EXTRACT,      TRUE    },
    {   "noident",              ITEM_NO_IDENT,          TRUE    },
    {   "burnproof",            ITEM_BURN_PROOF,        TRUE    },
    {   "nouncurse",            ITEM_NOUNCURSE,         TRUE    },
    {   "no_locate",            ITEM_NOLOCATE,          TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type wear_flags[] =
{
    {   "take",                 ITEM_TAKE,              TRUE    },
    {   "finger",               ITEM_WEAR_FINGER,       TRUE    },
    {   "neck",                 ITEM_WEAR_NECK,         TRUE    },
    {   "body",                 ITEM_WEAR_BODY,         TRUE    },
    {   "head",                 ITEM_WEAR_HEAD,         TRUE    },
    {   "legs",                 ITEM_WEAR_LEGS,         TRUE    },
    {   "feet",                 ITEM_WEAR_FEET,         TRUE    },
    {   "hands",                ITEM_WEAR_HANDS,        TRUE    },
    {   "arms",                 ITEM_WEAR_ARMS,         TRUE    },
    {   "shield",               ITEM_WEAR_SHIELD,       TRUE    },
    {   "about",                ITEM_WEAR_ABOUT,        TRUE    },
    {   "waist",                ITEM_WEAR_WAIST,        TRUE    },
    {   "wrist",                ITEM_WEAR_WRIST,        TRUE    },
    {   "wield",                ITEM_WIELD,             TRUE    },
    {   "hold",                 ITEM_HOLD,              TRUE    },
    {   "nosac",                ITEM_NO_SAC,            TRUE    },
    {   "wearfloat",            ITEM_WEAR_FLOAT,        TRUE    },
/*  {   "twohands",             ITEM_TWO_HANDS,         TRUE    }, */
    {   NULL,                   0,                      0       }
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {   "none",                 APPLY_NONE,             TRUE    },
    {   "strength",             APPLY_STR,              TRUE    },
    {   "dexterity",            APPLY_DEX,              TRUE    },
    {   "intelligence",         APPLY_INT,              TRUE    },
    {   "wisdom",               APPLY_WIS,              TRUE    },
    {   "constitution",         APPLY_CON,              TRUE    },
    {   "sex",                  APPLY_SEX,              TRUE    },
    {   "class",                APPLY_CLASS,            TRUE    },
    {   "level",                APPLY_LEVEL,            TRUE    },
    {   "age",                  APPLY_AGE,              TRUE    },
    {   "height",               APPLY_HEIGHT,           TRUE    },
    {   "weight",               APPLY_WEIGHT,           TRUE    },
    {   "mana",                 APPLY_MANA,             TRUE    },
    {   "hp",                   APPLY_HIT,              TRUE    },
    {   "move",                 APPLY_MOVE,             TRUE    },
    {   "gold",                 APPLY_GOLD,             TRUE    },
    {   "experience",           APPLY_EXP,              TRUE    },
    {   "ac",                   APPLY_AC,               TRUE    },
    {   "hitroll",              APPLY_HITROLL,          TRUE    },
    {   "damroll",              APPLY_DAMROLL,          TRUE    },
    {   "saves",                APPLY_SAVES,            TRUE    },
    {   "savingpara",           APPLY_SAVING_PARA,      TRUE    },
    {   "savingrod",            APPLY_SAVING_ROD,       TRUE    },
    {   "savingpetri",          APPLY_SAVING_PETRI,     TRUE    },
    {   "savingbreath",         APPLY_SAVING_BREATH,    TRUE    },
    {   "savingspell",          APPLY_SAVING_SPELL,     TRUE    },
    {   "spellaffect",          APPLY_SPELL_AFFECT,     TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type wear_loc_strings[]=
{
  { "� ���������",     WEAR_NONE,    TRUE },
  { "��������",        WEAR_LIGHT,   TRUE },
  { "�� �. �����",     WEAR_FINGER_L,TRUE },
  { "�� �. �����",     WEAR_FINGER_R,TRUE },
  { "�� ���",          WEAR_NECK,    TRUE },
  { "�� ����",         WEAR_BODY,    TRUE },
  { "�� ������",       WEAR_HEAD,    TRUE },
  { "�� �����",        WEAR_LEGS,    TRUE },
  { "�� ����",         WEAR_FEET,    TRUE },
  { "�� �����",        WEAR_HANDS,   TRUE },
  { "�� ����",         WEAR_ARMS,    TRUE },
  { "��� ���",         WEAR_SHIELD,  TRUE },
  { "�������� �� ����",WEAR_ABOUT,   TRUE },
  { "�� �����",        WEAR_WAIST,   TRUE },
  { "�� �. ��������",  WEAR_WRIST_L, TRUE },
  { "�� �. ��������",  WEAR_WRIST_R, TRUE },
  { "����������",      WEAR_RHAND,   TRUE },
  { "������ � ����",   WEAR_HOLD,    TRUE },
  { "������ ������",   WEAR_FLOAT,   TRUE },
  { NULL,              0,            0    }
};

const struct flag_type wear_loc_flags[] =
{
    {   "none",         WEAR_NONE,      TRUE    },
    {   "light",        WEAR_LIGHT,     TRUE    },
    {   "lfinger",      WEAR_FINGER_L,  TRUE    },
    {   "rfinger",      WEAR_FINGER_R,  TRUE    },
    {   "neck",         WEAR_NECK,      TRUE    },
    {   "body",         WEAR_BODY,      TRUE    },
    {   "head",         WEAR_HEAD,      TRUE    },
    {   "legs",         WEAR_LEGS,      TRUE    },
    {   "feet",         WEAR_FEET,      TRUE    },
    {   "hands",        WEAR_HANDS,     TRUE    },
    {   "arms",         WEAR_ARMS,      TRUE    },
    {   "shield",       WEAR_SHIELD,    TRUE    },
    {   "about",        WEAR_ABOUT,     TRUE    },
    {   "waist",        WEAR_WAIST,     TRUE    },
    {   "lwrist",       WEAR_WRIST_L,   TRUE    },
    {   "rwrist",       WEAR_WRIST_R,   TRUE    },
    {   "rhand",        WEAR_RHAND,     TRUE    },
    {   "lhand",        WEAR_LHAND,     TRUE    },
    {   "hold",         WEAR_HOLD,      TRUE    },
    {   "floating",     WEAR_FLOAT,     TRUE    },
    {   NULL,           0,              0       }
};

const   struct flag_type weapon_flags[]=
{
    {   "hit",          0,              TRUE    },  /*  0 */
    {   "slice",        1,              TRUE    },      
    {   "stab",         2,              TRUE    },
    {   "slash",        3,              TRUE    },
    {   "whip",         4,              TRUE    },
    {   "claw",         5,              TRUE    },  /*  5 */
    {   "blast",        6,              TRUE    },
    {   "pound",        7,              TRUE    },
    {   "crush",        8,              TRUE    },
    {   "grep",         9,              TRUE    },
    {   "bite",         10,             TRUE    },  /* 10 */
    {   "pierce",       11,             TRUE    },
    {   "suction",      12,             TRUE    },
    {   "beating",      13,             TRUE    },
    {   "digestion",    14,             TRUE    },
    {   "charge",       15,             TRUE    },  /* 15 */
    {   "slap",         16,             TRUE    },
    {   "punch",        17,             TRUE    },
    {   "wrath",        18,             TRUE    },
    {   "magic",        19,             TRUE    },
    {   "divinepower",  20,             TRUE    },  /* 20 */
    {   "cleave",       21,             TRUE    },
    {   "scratch",      22,             TRUE    },
    {   "peckpierce",   23,             TRUE    },
    {   "peckbash",     24,             TRUE    },
    {   "chop",         25,             TRUE    },  /* 25 */
    {   "sting",        26,             TRUE    },
    {   "smash",        27,             TRUE    },
    {   "shockingbite", 28,             TRUE    },
    {   "flamingbite",  29,             TRUE    },
    {   "freezingbite", 30,             TRUE    },  /* 30 */
    {   "acidicbite",   31,             TRUE    },
    {   "chomp",        32,             TRUE    },
    {   "lifedrain",    33,             TRUE    },
    {   "thrust",       34,             TRUE    },
    {   "slime",        35,             TRUE    },  /* 35 */
    {   "shock",        36,             TRUE    },
    {   "thwack",       37,             TRUE    },
    {   "flame",        38,             TRUE    },
    {   "chill",        39,             TRUE    },
    {   "shoot",        43,             TRUE    },
    {   NULL,           0,              0       }
};
const struct flag_type container_flags[] =
{
    {   "closeable",            1,              TRUE    },
    {   "pickproof",            2,              TRUE    },
    {   "closed",               4,              TRUE    },
    {   "locked",               8,              TRUE    },
    {   "puton",                16,             TRUE    },
    {   NULL,                   0,              0       }
};

const struct flag_type trap_flags[] =
{
    {   "alarm",            RAF_ALARM,          TRUE    },
    {   "remfly",           RAF_REMFLY,         TRUE    },
    {   "reminvis",         RAF_REMINVIS,       TRUE    },
    {   "fear",             RAF_FEAR,           TRUE    },
    {   "blind",            RAF_BLIND,          TRUE    },
    {   "curse",            RAF_CURSE,          TRUE    },
    {   "stun",             RAF_STUN,           TRUE    },
    {   "target_any",       RAF_TARGET_ANY,     TRUE    },
    {   "target_npc",       RAF_TARGET_NPC,     TRUE    },
    {   "target_pc",        RAF_TARGET_PC,      TRUE    },
    {   "target_flying",    RAF_TARGET_FLY,     TRUE    },
    {   "target_walking",   RAF_TARGET_WALK,    TRUE    },
    {   NULL,               0,                  0       }
};


/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/
const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   NULL,              0,                    0       },
};

const struct flag_type weapon_class[] =
{
    {   "exotic",        0,                    TRUE    },
    {   "sword",         1,                    TRUE    },
    {   "dagger",        2,                    TRUE    },
    {   "spear",         3,                    TRUE    },
    {   "mace",          4,                    TRUE    },
    {   "axe",           5,                    TRUE    },
    {   "flail",         6,                    TRUE    },
    {   "whip",          7,                    TRUE    },
    {   "polearm",       8,                    TRUE    },
    {   "staff",         9,                    TRUE    },
    {   NULL,            0,                    0       }
};

const struct flag_type weapon_type2[] =
{
    {   "none",          0,                     TRUE   },
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",      WEAPON_TWO_HANDS,     TRUE    },
    {   "shocking",      WEAPON_SHOCKING,      TRUE    },
    {   "poison",        WEAPON_POISON,        TRUE    },
    {   "missile",       WEAPON_MISSILE,       TRUE    },
    {   "return",        WEAPON_RETURN,        TRUE    },
    {   "round",         WEAPON_ROUND,         TRUE    },
    {   "vamp_mana",     WEAPON_VAMP_MANA,     TRUE    },
    {   NULL,              0,                   0       }
};

const struct flag_type res_flags[] =
{
    {   "summon",        RES_SUMMON,           TRUE    },
    {   "charm",         RES_CHARM,            TRUE    },
    {   "magic",         RES_MAGIC,            TRUE    },
    {   "weapon",        RES_WEAPON,           TRUE    },
    {   "bash",          RES_BASH,             TRUE    },
    {   "pierce",        RES_PIERCE,           TRUE    },
    {   "slash",         RES_SLASH,            TRUE    },
    {   "fire",          RES_FIRE,             TRUE    },
    {   "cold",          RES_COLD,             TRUE    },
    {   "lightning",     RES_LIGHTNING,        TRUE    },
    {   "acid",          RES_ACID,             TRUE    },
    {   "poison",        RES_POISON,           TRUE    },
    {   "negative",      RES_NEGATIVE,         TRUE    },
    {   "holy",          RES_HOLY,             TRUE    },
    {   "energy",        RES_ENERGY,           TRUE    },
    {   "mental",        RES_MENTAL,           TRUE    },
    {   "disease",       RES_DISEASE,          TRUE    },
    {   "drowning",      RES_DROWNING,         TRUE    },
    {   "light",         RES_LIGHT,            TRUE    },
    {   "sound",         RES_SOUND,            TRUE    },
    {   "wood",          RES_WOOD,             TRUE    },
    {   "silver",        RES_SILVER,           TRUE    },
    {   "iron",          RES_IRON,             TRUE    },
    {   NULL,            0,                    0    }
};

const struct flag_type vuln_flags[] =
{
    {   "summon",        VULN_SUMMON,          TRUE    },
    {   "charm",         VULN_CHARM,           TRUE    },
    {   "magic",         VULN_MAGIC,           TRUE    },
    {   "weapon",        VULN_WEAPON,          TRUE    },
    {   "bash",          VULN_BASH,            TRUE    },
    {   "pierce",        VULN_PIERCE,          TRUE    },
    {   "slash",         VULN_SLASH,           TRUE    },
    {   "fire",          VULN_FIRE,            TRUE    },
    {   "cold",          VULN_COLD,            TRUE    },
    {   "lightning",     VULN_LIGHTNING,       TRUE    },
    {   "acid",          VULN_ACID,            TRUE    },
    {   "poison",        VULN_POISON,          TRUE    },
    {   "negative",      VULN_NEGATIVE,        TRUE    },
    {   "holy",          VULN_HOLY,            TRUE    },
    {   "energy",        VULN_ENERGY,          TRUE    },
    {   "mental",        VULN_MENTAL,          TRUE    },
    {   "disease",       VULN_DISEASE,         TRUE    },
    {   "drowning",      VULN_DROWNING,        TRUE    },
    {   "light",         VULN_LIGHT,           TRUE    },
    {   "sound",         VULN_SOUND,           TRUE    },
    {   "wood",          VULN_WOOD,            TRUE    },
    {   "silver",        VULN_SILVER,          TRUE    },
    {   "iron",          VULN_IRON,            TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0     }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",    GATE_NORMAL_EXIT,     TRUE },
    {   "no_curse",       GATE_NOCURSE,         TRUE },
    {   "go_with",        GATE_GOWITH,          TRUE },
    {   "buggy",          GATE_BUGGY,           TRUE },
    {   "random",         GATE_RANDOM,          TRUE },
    {   NULL,             0,                    0    }
};

const struct flag_type furniture_flags[]=
{
    {   "none",           0,                    TRUE    },
    {   "stand_at",       STAND_AT,             TRUE    },
    {   "stand_on",       STAND_ON,             TRUE    },
    {   "stand_in",       STAND_IN,             TRUE    },
    {   "sit_at",         SIT_AT,               TRUE    },
    {   "sit_on",         SIT_ON,               TRUE    },
    {   "sit_in",         SIT_IN,               TRUE    },
    {   "rest_at",        REST_AT,              TRUE    },
    {   "rest_on",        REST_ON,              TRUE    },
    {   "rest_in",        REST_IN,              TRUE    },
    {   "sleep_at",       SLEEP_AT,             TRUE    },
    {   "sleep_on",       SLEEP_ON,             TRUE    },
    {   "sleep_in",       SLEEP_IN,             TRUE    },
    {   "put_at",         PUT_AT,               TRUE    },
    {   "put_on",         PUT_ON,               TRUE    },
    {   "put_in",         PUT_IN,               TRUE    },
    {   "put_inside",     PUT_INSIDE,           TRUE    },
    {   NULL,             0,                    0       }
};

const struct mitem_type mitem[]=
{
 { "none",           "��� �������",                     ENCH_NONE           },
 { "bless",          "permanent {Cbless{x",             ENCH_BLESS          },
 { "remvamp",        "remove {Dvampire {xflag",         ENCH_REMVAMP        },
 { "nodrop",         "add nodrop flag",                 ENCH_NODROP         },
 { "noremove",       "add noremove flag",               ENCH_NOREMOVE       },
 { "burnproof",      "permanent {Mfireproof {xflag",    ENCH_BURNPROOF      },
 { "reminvis",       "removes {Dinvisible {xflag",      ENCH_REMINVIS       },
 { "nolocate",       "add permanent {DNolocate {xflag", ENCH_NOLOCATE       },
 { "nouncurse",      "add permanent {CNOUNCURSE {xflag",ENCH_NOUNCURSE      },
 { "remantigood",    "removes {CAnti-Good {xflag",      ENCH_REMANTIGOOD    },
 { "remantievil",    "removes {RAnti-Evil {xflag",      ENCH_REMANTIEVIL    },
 { "remantoneutral", "removes {WAnti-Neutral {xflag",   ENCH_REMANTINEUTRAL },
 { "addvampiric",    "add {Dvampiric {xflag to weapon", ENCH_ADDVAMP        },
 { "addflaming",     "add {MFlaming {x to weapon",      ENCH_ADDFLAMING     },
 { "remtwohand",     "removes two-hand from weapon",    ENCH_REMTWOHAND     },
 { "setweight",      "set special weight to object",    ENCH_SETWEIGHT      },
 { "addhumming",     "add Humming flag",                ENCH_HUM            },
 { "addsharp",       "add Sharp flag",                  ENCH_SHARP          }
};

const struct wear_ntype wear_l[]=
{
 { "{C����    |{x",   WEAR_LIGHT},
 { "{c������  |{x",   WEAR_HEAD},
 { "{c���     |{x",   WEAR_NECK},
 { "{c�����   |{x",   WEAR_ARMS},
 { "{c����    |{x",   12},
 { "{c������  |{x",   WEAR_BODY},
 { "{c��������|{x",   WEAR_HANDS},
 { "{w�.����  {c|{x", WEAR_RHAND},
 { "{w�.����  {c|{x", WEAR_LHAND},
 { "{c��������|{x",   WEAR_WRIST_L},
 { "{c��������|{x",   WEAR_WRIST_R},
 { "{c�����   |{x",   WEAR_FINGER_L},
 { "{c�����   |{x",   WEAR_FINGER_R},
 { "{c����    |{x",   WEAR_WAIST},  
 { "{c����    |{x",   WEAR_LEGS},  
 { "{c������  |{x",   WEAR_FEET},  
 { "{c������  |{x",   WEAR_FLOAT}
};

const struct rspec_type rspec_table[] =
{
 { "����������� ������                                 ", SPEC_FLY          }, 
 { "����������� ��������                               ", SPEC_SNEAK        },
 { "����������� ��������������� ���� � 2 ���� �������  ", SPEC_MANAREGEN    },
 { "����������� �������� ������� ���� ������ (kick)    ", SPEC_DKICK        },
 { "����� CRUSH - ����, �������� �����                 ", SPEC_CRUSH        },
 { "����� TAIL  - ����������� ���� �������             ", SPEC_TAIL         },
 { "����������� �������:{/    ����� ���� �����{/    ������ ����� �������{/    ����������� ���������� �� ������� �����{/    ����� �� �������� ��� ������������� vampiric   ", SPEC_VAMPIRE      },
 { "����������� ���������� ��� ���                     ", SPEC_NOEAT        },
 { "����������� ���������� ��� �����                   ", SPEC_NODRINK      },
 { "����������� ������� ��������� ������ � ����� ����  ", SPEC_TWOHAND      },
 { "����� Blacksmith - ������� �������                 ", SPEC_BLACKSMITH   },
 { "������� �������������� ��������                    ", SPEC_REGENERATION },
 { "��������� �������� ���������� energy drain         ", SPEC_ENERGY       },
 { "����������� ���������� ����������� �������������   ", SPEC_DODGE        },
 { "����������� ����������� ���������                  ", SPEC_INVIS        },
 { "����������� ���������                              ", SPEC_HIDE         },
 { "+5% level ��� charm � sleep, ������������ � bash   ", SPEC_PSY          },
 { "�������� ������ regeneration (����� ������ �������)", SPEC_REGENSP      },
 { "����������� ������� ������ �������� �����          ", SPEC_PASSFLEE     },
 { "����������� ������������ � �����                   ", SPEC_MIST         },
 { "����������� ����� ����, ����� ����������           ", SPEC_HOWL         },
 { "����������� ����������� ��������� �� 1 ������      ", SPEC_TRAIN        },
 { "������������ � bash � crush                        ", SPEC_RESBASH      },
 { "����������� ��������� ��� ����� ��� ���������      ", SPEC_UWATER       },
 { "�� ������ ���� ��� ������                          ", SPEC_NOLOSTEXP    },
 { "������� ��������� �� ����                          ", SPEC_WATERWALK    },
 { "����� ������ ����������������� ���������           ", SPEC_RDEATH       },
 { "Damage �� Backstab �������� �� 7%                  ", SPEC_BACKSTAB     },
 { "������������ �����                                 ", SPEC_IGNOREALIGN  },
 { NULL, 0 }
};


const struct material_type material_table[]=
{
//{ name               ,real_name       ,d_dam  ,hard ,metal,res,vul},
 { "none"              ,"���"             ,1    ,80   ,FALSE,0  ,0 ,0,0 },
 { "unknown"           ,"���"             ,10   ,100  ,FALSE,0  ,0 ,0,0 },
 { "������"            ,"������"          ,100  ,70   ,FALSE,0  ,0 ,0,0 },
 { "glass"             ,"������"          ,100  ,70   ,FALSE,0  ,0 ,0,0 },
 { "����"              ,"����"            ,30   ,30   ,FALSE,0  ,0 ,0,0 },
 { "�������"           ,"�������"         ,150  ,30   ,FALSE,0  ,0 ,0,0 },
 { "�������"           ,"�������"         ,150  ,30   ,FALSE,0  ,0 ,0,0 },
 { "���"               ,"���"             ,40   ,40   ,FALSE,0  ,0 ,0,0 },
 { "fur"               ,"���"             ,40   ,40   ,FALSE,0  ,0 ,0,0 },
 { "����"              ,"����"            ,120  ,50   ,FALSE,0  ,0 ,0,0 },
 { "leather"           ,"����"            ,120  ,50   ,FALSE,0  ,0 ,0,0 },
 { "������"            ,"������"          ,550  ,100  ,FALSE,0  ,0 ,0,0 },
 { "stone"             ,"������"          ,550  ,100  ,FALSE,0  ,0 ,0,0 },
 { "������ ��������"   ,"������ ��������" ,610  ,120  ,FALSE,0  ,0 ,0,0 },
 { "������ ��������"   ,"������ ��������" ,610  ,120  ,FALSE,0  ,0 ,0,0 },
 { "�����"             ,"�����"           ,80   ,60   ,FALSE,0  ,0 ,0,0 },
 { "yew"               ,"���"             ,475  ,90   ,FALSE,RES_WOOD ,VULN_WOOD ,15,15},
 { "���"               ,"���"             ,475  ,90   ,FALSE,RES_WOOD ,VULN_WOOD ,15,15},
 { "wool"              ,"������"          ,50   ,60   ,FALSE,0  ,0 ,0,0 },
 { "wooden"            ,"������"          ,450  ,90   ,FALSE,RES_WOOD ,VULN_WOOD ,25,25},
 { "wood"              ,"������"          ,450  ,90   ,FALSE,RES_WOOD ,VULN_WOOD ,25,25},
 { "wire"              ,"���������"       ,475  ,80   ,FALSE,0  ,0  ,0,0},
 { "wax"               ,"����"            ,30   ,80   ,FALSE,0  ,0  ,0,0},
 { "����"              ,"����"            ,30   ,80   ,FALSE,0  ,0  ,0,0},
 { "water"             ,"����"            ,350  ,70   ,FALSE,0  ,0  ,0,0},
 { "velvet"            ,"�������"         ,80   ,80   ,FALSE,0  ,0  ,0,0},
 { "�������"           ,"�������"         ,80   ,80   ,FALSE,0  ,0  ,0,0},
 { "vellum"            ,"���������"       ,110  ,40   ,FALSE,0  ,0  ,0,0},
 { "������ ���������"  ,"���������"       ,110  ,40   ,FALSE,0  ,0  ,0,0},
 { "stone"             ,"������"          ,550  ,100  ,FALSE,0  ,0 ,0,0 },
 { "Steel"             ,"�����"           ,700  ,110  ,TRUE ,0  ,0  ,0,0},                         
 { "steel"             ,"�����"           ,700  ,110  ,TRUE ,0  ,0  ,0,0},                         
 { "Sorrow"            ,"������"          ,410  ,100  ,TRUE ,0  ,0  ,0,0},                         
 { "sorrow"            ,"������"          ,410  ,100  ,TRUE ,0  ,0  ,0,0},                         
 { "softwood"          ,"������ ������"   ,320  ,70   ,FALSE,RES_WOOD ,VULN_WOOD ,15,15},          
 { "soft leather"      ,"������ ����"     ,100  ,50   ,FALSE,0  ,0  ,0,0},                         
 { "snakeskin"         ,"������� ����"    ,200  ,70   ,FALSE,0  ,0  ,0,0},                         
 { "skin"              ,"�����"           ,70   ,70   ,FALSE,0  ,0  ,0,0},                         
 { "silver"            ,"�������"         ,480  ,95   ,FALSE,RES_SILVER ,VULN_SILVER ,25,25},      
 { "�������"           ,"�������"         ,480  ,95   ,FALSE,RES_SILVER ,VULN_SILVER ,25,25},      
 { "silk"              ,"����"            ,160  ,40   ,FALSE,0  ,0  ,0,0},                         
 { "����"              ,"����"            ,160  ,40   ,FALSE,0  ,0  ,0,0},                         
 { "shell"             ,"��������"        ,75   ,100  ,FALSE,0  ,0  ,0,0},                         
 { "��������"          ,"��������"        ,75   ,100  ,FALSE,0  ,0  ,0,0},                         
 { "shadows"           ,"����"            ,170  ,20   ,FALSE,0  ,0  ,0,0},                         
 { "shadow"            ,"����"            ,170  ,20   ,FALSE,0  ,0  ,0,0},                         
 { "scale"             ,"�����"           ,450  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "satin"             ,"�����"           ,80   ,50   ,FALSE,0  ,0  ,0,0},                         
 { "�����"             ,"�����"           ,75   ,50   ,FALSE,0  ,0  ,0,0},                         
 { "rugs"              ,"��������"        ,20   ,50   ,FALSE,0  ,0  ,0,0},                         
 { "rubber"            ,"������"          ,140  ,50   ,FALSE,0  ,0  ,0,0},                         
 { "root"              ,"���������"       ,175  ,60   ,FALSE,0  ,0  ,0,0},                         
 { "porcelain"         ,"��������"        ,70   ,100  ,FALSE,0  ,0  ,0,0},                         
 { "platinum"          ,"�������"         ,330  ,120  ,TRUE ,0  ,0  ,0,0},                         
 { "plastic"           ,"�������"         ,140  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "pergament"         ,"���������"       ,110  ,80   ,FALSE,0  ,0  ,0,0},                         
 { "pearl"             ,"������"          ,600  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "parchment"         ,"���������"       ,110  ,60   ,FALSE,0  ,0  ,0,0},                         
 { "papirus"           ,"�������"         ,90   ,40   ,FALSE,0  ,0  ,0,0},                         
 { "paper"             ,"������"          ,50   ,40   ,FALSE,0  ,0  ,0,0},                         
 { "Opal"              ,"����"            ,310  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "opal"              ,"����"            ,310  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "onyx"              ,"�����"           ,430  ,110  ,FALSE,0  ,0  ,0,0},                         
 { "oldstyle"          ,"���"             ,1    ,10   ,FALSE,0  ,0  ,0,0},                         
 { "oil"               ,"�����"           ,10   ,30   ,FALSE,0  ,0  ,0,0},                         
 { "obsidian"          ,"��������"        ,770  ,110  ,FALSE,0  ,0  ,0,0},                         
 { "oak"               ,"���"             ,430  ,110  ,FALSE,RES_WOOD ,VULN_WOOD ,20,20},          
 { "nothingness"       ,"�����"           ,90   ,50   ,FALSE,0  ,0  ,0,0},                         
 { "NOTHING"           ,"�����"           ,90   ,100  ,FALSE,0  ,0  ,0,0},                         
 { "mramour"           ,"������"          ,470  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "moss"              ,"���"             ,40   ,80   ,FALSE,0  ,0  ,0,0},                         
 { "mithril"           ,"������"          ,850  ,150  ,FALSE,0  ,0  ,0,0},                         
 { "metal"             ,"������"          ,500  ,110  ,TRUE ,0  ,0  ,0,0},                         
 { "meat"              ,"�����"           ,100  ,80   ,FALSE,0  ,0  ,0,0},                         
 { "mat"               ,"���������"       ,120  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "marble"            ,"������"          ,460  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "Magic material"    ,"�����"           ,710  ,140  ,FALSE,RES_MAGIC ,VULN_MAGIC ,20,20},        
 { "magic material"    ,"�����"           ,710  ,140  ,FALSE,RES_MAGIC ,VULN_MAGIC ,20,20},        
 { "magic"             ,"�����"           ,710  ,140  ,FALSE,RES_MAGIC ,VULN_MAGIC ,20,20},        
 { "linen"             ,"��"             ,130  ,70   ,FALSE,0  ,0  ,0,0},                         
 { "light"             ,"����"            ,333  ,100  ,FALSE,RES_LIGHT ,VULN_LIGHT ,20,20},        
 { "leather steel"     ,"������ �����"    ,500  ,90   ,FALSE,0  ,0  ,0,0},                         
 { "leather"           ,"����"            ,110  ,50   ,FALSE,0  ,0 ,0,0 },   
 { "lead"              ,"�����"           ,350  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "kid leather"       ,"���� �������"    ,55   ,70   ,FALSE,0  ,0  ,0,0},                         
 { "ivory"             ,"�������� �����"  ,440  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "iron bone"         ,"�������� �����"  ,440  ,100  ,TRUE ,RES_IRON ,VULN_IRON ,10,10},          
 { "iron"              ,"������"          ,500  ,110  ,TRUE ,RES_IRON ,VULN_IRON ,20,20},          
 { "ice"               ,"��"             ,60   ,100  ,FALSE,RES_COLD ,VULN_COLD ,15,15},          
 { "human flesh"       ,"������� �����"   ,50   ,80   ,FALSE,0  ,0  ,0,0},                         
 { "human femur"       ,"������� �����"   ,50   ,80   ,FALSE,0  ,0  ,0,0},                         
 { "hell"              ,"���������� ���"  ,666  ,110  ,FALSE,0  ,0  ,0,0},                         
 { "hardwood"          ,"�������� ������" ,640  ,110  ,FALSE,RES_WOOD ,VULN_WOOD ,30,30},          
 { "hard leather"      ,"������� ����"    ,220  ,80   ,FALSE,0  ,0  ,0,0},                         
 { "hair"              ,"������"          ,35   ,60   ,FALSE,0  ,0  ,0,0},                         
 { "gut"               ,"������������"    ,30   ,70   ,FALSE,0  ,0  ,0,0},                         
 { "gourd"             ,"�����"           ,525  ,70   ,FALSE,0  ,0  ,0,0},                         
 { "gold amethyst"     ,"������� �������" ,400  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "gold"              ,"������"          ,380  ,100  ,TRUE ,0  ,0  ,0,0},                         
 { "Gold"              ,"������"          ,380  ,100  ,TRUE ,0  ,0  ,0,0},                         
 { "gem"               ,"��������"        ,310  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "gas"               ,"���"             ,20   ,60   ,FALSE,0  ,0  ,0,0},                         
 { "food"              ,"����"            ,25   ,50   ,FALSE,0  ,0  ,0,0},                         
 { "flesh"             ,"�����"           ,50   ,60   ,FALSE,0  ,0  ,0,0},                         
 { "fire"              ,"�����"           ,135  ,100  ,FALSE,RES_FIRE ,VULN_FIRE ,20,20},          
 { "felt"              ,"�������"         ,295  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "feathers"          ,"�����"           ,40   ,80   ,FALSE,0  ,0  ,0,0},                         
 { "etherealness"      ,"�����"           ,65   ,100  ,FALSE,0  ,0  ,0,0},
 { "energy"            ,"�������"         ,555  ,100  ,FALSE,RES_ENERGY ,VULN_ENERGY ,20,20},      
 { "electrum"          ,"�������"         ,555  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "electronic"        ,"�������"         ,555  ,100  ,FALSE,0  ,0  ,0,0},                         
 { "ebony"             ,"������"          ,470  ,90   ,FALSE,RES_WOOD ,VULN_WOOD ,10,10},          
 { "dragonscale"       ,"����� �������"   ,550  ,110  ,FALSE,0  ,0  ,0,0},
 { "diamond"           ,"��������"        ,350  ,110  ,FALSE,0  ,0  ,0,0},
 { "darkness"          ,"����"            ,666  ,100  ,FALSE,0  ,0  ,0,0},
 { "dark laen"         ,"����"            ,666  ,100  ,FALSE,0  ,0  ,0,0},
 { "dark"              ,"����"            ,333  ,100  ,FALSE,0  ,0  ,0,0},
 { "crystal"           ,"��������"        ,410  ,110  ,FALSE,0  ,0  ,0,0},
 { "cotton"            ,"������"          ,80   ,100  ,FALSE,0  ,0  ,0,0},
 { "coral"             ,"�����"           ,145  ,100  ,FALSE,0  ,0  ,0,0},
 { "copper"            ,"����"            ,415  ,100  ,TRUE ,0  ,0  ,0,0},
 { "cloud"             ,"������"          ,75   ,100  ,FALSE,0  ,0  ,0,0},
 { "cloth"             ,"�����"           ,80   ,80   ,FALSE,0  ,0  ,0,0},
 { "clay"              ,"�����"           ,125  ,100  ,FALSE,0  ,0  ,0,0},
 { "china"             ,"������"          ,190  ,100  ,FALSE,0  ,0  ,0,0},
 { "bronze"            ,"������"          ,510  ,100  ,TRUE ,0  ,0  ,0,0},
 { "brass"             ,"������"          ,370  ,100  ,FALSE,0  ,0  ,0,0},
 { "bone"              ,"�����"           ,350  ,100  ,FALSE,0  ,0  ,0,0},
 { "body"              ,"�����"           ,50   ,100  ,FALSE,0  ,0  ,0,0},
 { "Blood"             ,"�����"           ,50   ,100  ,FALSE,0  ,0  ,0,0},
 { "blood"             ,"�����"           ,50   ,100  ,FALSE,0  ,0  ,0,0},
 { "black dragonscale" ,"����� �������"   ,550  ,120  ,FALSE,0  ,0  ,0,0},
 { "bamboo"            ,"������"          ,260  ,100  ,FALSE,0  ,0  ,0,0},
 { "aluminum"          ,"��������"        ,390  ,90   ,TRUE ,0  ,0  ,0,0},
 { "air"               ,"������"          ,40   ,80   ,FALSE,0  ,0  ,0,0},
 { "adamantite"        ,"���������"       ,810  ,110  ,FALSE,0  ,0  ,0,0},
 { "admantite"         ,"���������"       ,810  ,120  ,FALSE,0  ,0  ,0,0},
 { "adamite"           ,"���������"       ,810  ,120  ,FALSE,0  ,0  ,0,0},
 { "acid"              ,"�������"         ,95   ,110  ,FALSE,RES_ACID ,VULN_ACID ,20,20},
 { "orihalc"           ,"�������"         ,1000 ,200  ,FALSE,0  ,0  ,0,0},
 { NULL                ,"BUG"             ,0    ,1    ,FALSE,0  ,0  ,0,0}
};                     
                       
// various flag tables
const struct flag_type raff_flags[] =
{
  { "bcloud",       RAFF_BLIND,     TRUE },
  { "nowhere",      RAFF_NOWHERE,   TRUE },
  { "distortion",   RAFF_HIDE,      TRUE },
  { "web",          RAFF_WEB,       TRUE },
  { "faerie_fog",   RAFF_ALL_VIS,   TRUE },
  { "oasis",        RAFF_OASIS,     TRUE },
  { "mind_ch",      RAFF_MIND_CH,   TRUE },
  { "evil_presence",RAFF_EVIL_PR,   TRUE },
  { "life_stream",  RAFF_LIFE_STR,  TRUE },
  { "safty place",  RAFF_SAFE_PLC,  TRUE },
  { "violence   ",  RAFF_VIOLENCE,  TRUE },
  { NULL,           0,              0    }
};

const struct flag_type penalty_flags[] =
{
  { "nochannels", P_NOCHANNELS,TRUE },
  { "nomlove",    P_NOMLOVE,   TRUE },
  { "nopost",     P_NOPOST,    TRUE },
  { "godcurse",   P_GODCURSE,  TRUE },
  { "nogsocial",  P_NOGSOCIAL, TRUE },
  { "freeze",     P_FREEZE,    TRUE },
  { "noemote",    P_NOEMOTE,   TRUE },
  { "notell",     P_NOTELL,    TRUE },
  { "tipsy",      P_TIPSY,     TRUE },
  { "nodelete",   P_NODELETE,  TRUE },
  { "nostalgia",  P_NOSTALGIA, TRUE },
  { NULL,         0,           0    }
};

// for fixing liquid names
const struct fix_liquid fix_liq_table[] =
{
 //name                  newname
 { "����",               "water"           },
 { "�����",              "vodka"           },
 { "����",               "beer"            },
 { "������",             "milk"            },
 { "{W�������� {y����{x","cool beer"       },
 { "�������� ����",      "cool beer"       },
 { "������",             "tarhun"          },
 { "������",             "tarhun"          },
 { "�����",              "blood"           },
 { "{Y������{x",         "bulyon"          },
 { "{r�����{x",          "blood"           },
 { "{G����� �������{x",  "acid"            },
 { "���",                "honey"           },
 { "���� ����",          "cream soda"      },
 { "slavutich",          "beer slavutich"  },
 { "premium",            "beer premium"    },
 { "oksamit",            "beer oskamit"    },
 { "porter22",           "beer porter22"   },
 { "jiv4ik",             "jivchik"         },
 { NULL,                 NULL              }
};

const struct dam_msg_type dam_msg_table[] =
{
 // from, to
 // damage other,
 // damage self,
 {   0,  0, "�� ���������",                "�� ��������"         },
 {   1,  2, "{w���������{x",               "{w��������{x"        },
 {   3,  5, "{w���������{x",               "{w��������{x"        },
 {   6,  8, "{w������ ���������{x",        "{w������ ��������{x" },
 {   9, 12, "{m������{x",                  "{m�����{x"           },
 {  13, 15, "{m������ ������{x",           "{m������ �����{x"    },
 {  16, 19, "{R<{y����������{R>{x",        "{R<{y���������{R>{x" },
 {  20, 25, "{R<{y�����������{R>{x",       "{R<{y����������{R>{x"},
 {  26, 30, "{R<{y�����������{R>{x",       "{R<{y����������{R>{x"},
 {  31, 35, "{R>{M��������{R<{x",          "{R>{M�������{R<{x"   },
 {  36, 43, "{R>{M�������{R<{x",           "{R>{M������{R<{x"    },
 {  44, 50, "{R>{M��������{R<{x",          "{R>{M�������{R<{x"   },
 {  51, 65, "{x<{R*{x> {c����������� {x<{R*{x>",
            "{x<{R*{x> {c���������� {x<{R*{x>"            },
 {  66, 80, "{x<{R*{x> {c����������� {x<{R*{x>",
            "{x<{R*{x> {c���������� {x<{R*{x>"            },
 {  81, 90, "{x>{R*{x< {c����������� {x>{R*{x<",
            "{x>{R*{x< {c���������� {x>{R*{x<"            },
 {  91,100, "{x>{R*{x< {c������������� {x>{R*{x<",
            "{x>{R*{x< {c������������ {x>{R*{x<"          },
 { 101,125, "{C<<< {W���������� {C>>>{x",
            "{C<<< {W��������� {C>>>{x"                   },
 { 126,150, "{C<<< {W�������� � ���� {C>>>{x",
            "{C<<< {W������� � ���� {C>>>{x"              },
 { 151,200, "{C>>> {W�������� � ������� {C<<<{x",
            "{C>>> {W������� � ������� {C<<<{x"           },
 { 201,250, "{W[{R*{W] {Y������������� {W[{R*{W]{x",
            "{W[{R*{W] {Y������������ {W[{R*{W]{x"        },
 { 251,300, "{W[{R*{W] {Y��������� � ������ {W[{R*{W]{x",
            "{W[{R*{W] {Y�������� � ������ {W[{R*{W]{x"   },
 { 301,400, "{W[{R*{W][{R*{W] {G���������� �� ����� {W[{R*{W][{R*{W]{x",
            "{W[{R*{W][{R*{W] {G��������� �� ����� {W[{R*{W][{R*{W]{x"  },
 { 401,500, "{W[{R*{W][{R*{W] {G���������������� {W[{R*{W][{R*{W]{x",
            "{W[{R*{W][{R*{W] {G��������������� {W[{R*{W][{R*{W]{x"      },
 { 501,1000, "{G��{R���{C��{Y��{M� {W�� {D��{B���{x",
            "{G��{R���{C��{Y�� {W�� {D��{B���{x"                            },
 { 1001,-1, "{M-{m={W* {R�{r�{R@{r#{R$${r�{R�{r�{R� � {R������ {W*{m={M-{x",
            "{M-{m={W* {R�{r�{R@{r#{R$${r�{R�{r�{R � ������ {W*{m={M-{x"    }
};

const struct item_cond_type item_cond_table[] =
{
 // oIndex
 // calccon
 // showcon
 // gshowcon - accusative cond. mess.
 // showshortcon
  { 0,   0, "{r�������{x",          "{r��������{x",          "{r���{x"   },
  { 1,  10, "{R����� �������{x",    "{R����� ��������{x",    "{R�.���{x" },
  { 2,  17, "{m������ ���������{x", "{m������ ����������{x", "{m�.���{x"   },
  { 3,  25, "{M���������{x",        "{M����������{x",        "{M���{x"   },
  { 4,  33, "{b������{x",           "{b������{x",            "{b���{x"   },
  { 5,  45, "{B��������{x",         "{B���������{x",         "{B���{x"   },
  { 6,  65, "{w����������{x",       "{w����������{x",        "{w���{x"   },
  { 7,  80, "{W�������{x",          "{W�������{x",           "{W���{x"   },
  { 8, 100, "{G��������{x",         "{G��������{x",          "{G���{x"   },
  { 9,  -1, "{R������������{x",     "{R������������{x",      "{R���{x"   },
  {10, 150, "{RReportToImms{x",     "{RReportToImms{x",      "{R���{x"   },
  {-1,   0, "{R�������{x",          "{R�������{x",           "{R���{x"   }
};
const struct item_durability_status item_durability_table[] =
{
 // from
 // to
 // d_message
 {    0,   10,  "������������" },
 {   10,   50,  "������" },
 {   50,  100,  "���������" }, 
 {  100,  140,  "���������" },
 {  140,  200,  "�������" },
 {  200,  260,  "��������" },
 {  260,  300,  "�������" },
 {  300,  350,  "�������" },
 {  350,  410,  "���� ��������" },
 {  410,  460,  "�������" },
 {  460,  510,  "{y�������������{x" },
 {  510,  560,  "{G�������{x" },
 {  560,  610,  "{w�������{x" },
 {  610,  700,  "{D����� �������{x" },
 {  700,  800,  "{W��������{x" },
 {  800,  900,  "{m����������{x" },
 {  900, 1000,  "{c�����������{x" },
 { 1001,   -1,  "{R������{x" }
};

const struct favour_output t_favour[] =
{
// from, to, amp, fav_nfstr, fav_nmstr, fav_afstr, fav_amstr
// defined MIDDLE_FAVOUR = 5
  {  -5000, -2001,  5, "������� �� ����",     "���������� ���� �� ����", "��������� �� ����",    "���������� ����� �� ����" },
  {  -2000, -1001,  4, "�������� �� ����",    "��������� �� ����",       "���������� �� ����",   "���������� �� ����"       },
  {  -1000,  -401,  3, "���������� ����",     "���������� ����",         "���������� ����",      "���������� ����"          },
  {   -400,  -101,  2, "�������� � ����",     "�������� � ����",         "�������� � ����",      "�������� � ����"          },
  {   -100,    -1,  2, "������������ � ����", "������������ � ����",     "������������ � ����",  "������������ � ����"      },
  {      0,   100,  1, "�������������",       "�����������������",       "��������������",       "������������������"       },
  {    101,   500,  1, "������� ������",      "������� ��������",        "������� ��������",     "������� ���������"        },
  {    501,  1000,  1, "������� ������",      "������� ��������",        "������� ��������",     "������� ���������"        },
  {   1001,  2500,  1, "�����",               "��������",                "�������",              "���������"                },
  {   2501,  4949,  1, "����",                "�����",                   "������",               "������"                   },
  {   4950,  5000,  1, "{G��������� {W����{x","{W��������� {w�����{x",   "{r��������� {W������", "{r��������� {W������{x"   },
  {      0,     0,  0, NULL,                  NULL,                      NULL,                   NULL                       }
};

const struct deity_apply_type deity_apply_table[] =
{
// param inform resist res_flag -skill -spell
 { "",            "",                  "",               ""        },
 { "����",        "������ ����",       "�������� ������","slash"   },
 { "���������",   "������ �������",    "�������� ������","pierce"  },
 { "��������",    "������ �����",      "������ ������",  "bash"    },
 { "��������",    "������ ����",       "������ ����",    "fire"    },
 { "������������","����� ����",        "����������",     "poison"  },
 {  NULL,         "����� ������",      "����� ������",   "mental"  },
 {  NULL,         "����� �����",       "����� �����",    "light"   },
 {  NULL,         "����� ����",        "������ �����",   "negative"},
 {  NULL,         "������� ���������", "�������",        "acid"    },
 {  NULL,         "�������� �����",    "������ �����",   "holy"    },
 {  NULL,         "������� ����������", NULL,            NULL      },
 {  NULL,         "������� ��������",   NULL,            NULL      },
 {  NULL,         "������� ���������",  NULL,            NULL      },
 {  NULL,         "������� ������",     NULL,            NULL      },
 {  NULL,         "������� ��������",   NULL,            NULL      },
 {  NULL,          NULL,                NULL,            NULL      }
};
