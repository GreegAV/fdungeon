cmake_minimum_required(VERSION 3.31)
project(fdungeon C)

set(CMAKE_C_STANDARD 11)

include_directories(mud)

# -fcommon: merc.h contains ~159 tentative definitions of globals that land in
# every translation unit. GCC 10+ defaults to -fno-common, which breaks linking
# with ~550 "multiple definition" errors. Keep in sync with mud/Makefile.
add_compile_options(-fcommon -Wall)
add_compile_definitions(OLD_RAND)

add_executable(fdungeon
        mud/act_comm.c
        mud/act_info.c
        mud/act_move.c
        mud/act_obj.c
        mud/act_wiz.c
        mud/astellar.c
        mud/bit.c
        mud/charset.c
        mud/clan.c
        mud/comm.c
        mud/const.c
        mud/db.c
        mud/db.h
        mud/fight.c
        mud/files.h
        mud/guild.c
        mud/handler.c
        mud/interp.c
        mud/interp.h
        mud/io.c
        mud/magic.c
        mud/magic.h
        mud/magic2.c
        mud/mem.c
        mud/merc.h
        mud/merc_def.h
        mud/merc_str.h
        mud/mob_cmds.c
        mud/mob_cmds.h
        mud/mob_prog.c
        mud/music.c
        mud/music.h
        mud/note.c
        mud/olc.c
        mud/olc.h
        mud/olc_act.c
        mud/olc_mpcode.c
        mud/olc_save.c
        mud/quest.c
        mud/recycle.c
        mud/recycle.h
        mud/saboteur.c
        mud/save.c
        mud/skills.c
        mud/social_edit.c
        mud/special.c
        mud/string.c
        mud/tables.c
        mud/tables.h
        mud/telnet.h
        mud/update.c)
