// Copyrights (C) 1998-2003, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'

// Thanx for Vassago for 'Automated Quest code by Vassago of MOONGATE' snippet
// Quest code by Sergey Kulik aka Saboteur (1998-2000)

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

DECLARE_DO_FUN2( do_say );
void create_gquest(int min,int max, int mobs);
void quest_update       args(( void ));
void gquest_update      args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
ROOM_INDEX_DATA  *find_location( CHAR_DATA *ch, char *arg );
void complete_quest(CHAR_DATA *ch, CHAR_DATA *questman, char *buf);
void set_quest_time(CHAR_DATA *ch, int q_type);
CHAR_DATA *find_questmob(int level);
CHAR_DATA *questman_lookup(CHAR_DATA *ch, bool request);
void clear_gquest();

struct quest_type {
  char *name;
  char *show_name;
  int   cost;
  int64 vnum;
  int   level;
};

const struct quest_type quest_table[] =
{
// "command_name",  "show_list_name",                     qp,  vnum, level

// Special features
   { "practices",        "{C30 �������{x",                   500,    -1,    1 },
   { "gold",             "{C20 000 ������� �����{x",         500,    -1,    1 },

// Quest Items
   { "divan",            "{Y�����{x",                       1000,    29,    1 },
   { "scuba",            "{Y��������{x",                     350,  3395,    1 },
   { "waybottle",        "{Y��������� �����{x",              550,    28,    1 },

// Items artefacts
   { "bless",            "{WBless{G artefact{x",             500,   131,   -1 },
   { "burnproof",        "{MBurnProof{G artefact{x",         650,   135,   -1 },
   { "humming",          "{C�������{G ��������{x{x",        1500,   146,   -1 },
   { "nodrop",           "{wNodrop{G artefact{x",           2500,   133,   -1 },
   { "nolocate",         "{DNolocate{G artefact{x",          500,   137,   -1 },
   { "noremove",         "{wNoremove{G artefact{x",         5000,   134,   -1 },
   { "nouncurse",        "{WNouncurse{G artefact{x",        2500,   138,   -1 },
   { "reminvisible",     "{DRemove Invisible{G ��������{x",  100,   136,   -1 },
   { "remvampiric",      "{rRemove Vampiric{G artefact{x",   350,   132,   -1 },
   { "vampiric",         "{RAdd Vampiric{G artefact{x",     5000,   142,   -1 },

// Order for Item Rename
   { "restring",         "{RR{GE{WN{YA{BM{CE {ycoupon{x",    150,   499,   -1 },

// Ancient scrolls of Quenia
   { "scroll restore",   "Ancient Scroll of Restore",        250,   400,   -1 },
   { "scroll enforce",   "Ancient Scroll of Enforce",        350,   401,   -1 },
   { "scroll curse",     "Ancient Scroll of Curse",          400,   402,   -1 },
   { "scroll power",     "Ancient Scroll of Power",          150,   403,   -1 },
   { "scroll bless",     "Ancient Scroll of Bless",          300,   404,   -1 },
   { "scroll fireproof", "Ancient Scroll of FireProof",      250,   405,   -1 },
   { "none",             "none",                               0,     0,    0 }

/*
   { "antievil",    "{CRemove Anti-Evil{x artefact",       20000,   140,   -1 },
   { "anineutral",  "{WRemove Anti-Neutral{x artefact",    20000,   141,   -1 },
   { "antigood",    "{RRemove Anti-Good{x artefact",       20000,   139,   -1 },
   { "sharp",       "{YSharp{x artefact",                  15000,   147,   -1 },
   { "onehand",     "{DOne-handed artefact{x",             15000,   144,   -1 },
   { "flaming",     "{RFlaming{x artefact",                15000,   143,   -1 },
*/
};

void do_quest(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *questman=NULL;
  MOB_INDEX_DATA *Pquestman=NULL;
  OBJ_DATA *obj=NULL;
  OBJ_INDEX_DATA *questinfoobj;
  char buf [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];                        
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if ((arg1[0] == '\0') || !str_prefix(arg1, "help") || !str_cmp(arg1, "?")) {
    stc("{Gquest help       {x- �������� ��� �������\n\r", ch);
    stc("{Gquest request    {x- �������� �������\n\r", ch);
    stc("{Gquest complete   {x- �������� � ���������� ������� ��� ������ ��������� ����\n\r", ch);
    stc("{Gquest info       {x- ���������� ���������� �� �������� �������\n\r", ch);
    stc("{Gquest points     {x- ������ ���������� ����� ��������� ����� \n\r", ch);
    stc("{Gquest time       {x- ������ ����� �� ������� ��� �� ���������� �������\n\r", ch);
    stc("{Gquest list       {x- ���������� ������ ����������� �� qp ���������\n\r", ch);
    stc("{Gquest buy <item> {x- ������ ���-�� � ��������\n\r", ch);
    stc("{Gquest cancel     {x- �������� ������� (�� ������ ����������� �� ��������� ���-�� qp)\n\r", ch);
    if (ch->level>100)
    {
      stc("{Gquest convert    {x- ���������� ���������� � QP.\n\r", ch);
      stc("{Gquest buy <item> train {x- ������ ���-�� � �������� �� ����������. 1 train=165 qp.\n\r", ch);
    }
    return;
  }

  if (!str_prefix(arg1, "info")) {
    if (!IS_SET(ch->act, PLR_QUESTOR)) stc("� ���� ������ ��� �������.\n\r",ch);
    else if (QSTAT(ch, Q_KILL_MOB)) {
      Pquestman=get_mob_index(ch->questgiver);
      if (QSTAT(ch,Q_MOB_KILLED)) 
       ptc(ch, "�� ����� �������� ���� �������!\n\r����������� � %s ���� �� ������� �p���!\n\r",get_mobindex_desc(Pquestman, '3'));
      else {
       ptc(ch, "{C���� ����� {R����� {C�������� {Y%s{C!\n\r",get_char_desc(ch->questmob,'4'));
       ptc(ch, "{C� ��������� ��� ��� ������ ���-�� � {W%s{C.\n\r", ch->questroom->name);
       ptc(ch, "��� ��������� ���-�� � ���� {Y%s{C.{x\n\r",ch->questroom->area->name);
      }
    }
    else if (QSTAT(ch, Q_FIND_ITEM)) {
      questinfoobj = get_obj_index(ch->questobj);
      ptc(ch, "{C���� ����� {G����� {Y%s{C!\n\r",questinfoobj->name);
      ptc(ch, "������� ������ ����������, ��� ��� ��������� � {W%s{C.\n\r", ch->questroom->name);
      ptc(ch, "� ���� {Y%s{C.{x\n\r",ch->questroom->area->name);
    }
    else if (QSTAT(ch, Q_BRING_ITEM)) {
      Pquestman=get_mob_index(ch->questgiver);
      questinfoobj = get_obj_index(ch->questobj);
      ptc(ch, "{C���� ����� ����� � ������� {C%s{x ��������� ������.\n\r",questinfoobj->name);
      ptc(ch, "��� � {Y%s{C � ���� {G%s{x.\n\r", ch->questroom->name,ch->questroom->area->name);
      ptc(ch, "{Y%s{x ���� ���� � {G%s{x.\n\r",get_mobindex_desc(Pquestman,'1'),Pquestman->area->name);
      stc("������ ������� ����� �������� quest complete.\n\r",ch);
    }
    return;
  }

  if (!str_prefix(arg1, "points")) {
    ptc(ch, "{G� ���� {W%d {G��������� �����.{x\n\r",ch->questpoints);
    return;
  }

  if (!str_prefix(arg1, "time")) {
    if (!IS_SET(ch->act, PLR_QUESTOR)) {
      stc("  � ���� ������ ��� �������.\n\r",ch);
      if (ch->nextquest > 1)
        ptc(ch, "  ��p�� {W%d {G����� �� ������� ����� ��������� �������.{x\n\r",ch->nextquest);
      else if (ch->nextquest==1) stc("{G�������� ������ ������ �� ���� ��� �� ������� ����� �p���� �������.{x\n\r",ch);
    }
    else if (ch->countdown > 0)
      ptc(ch, "� ���� ���� ��� %d ����� �� ���������� �������.\n\r",ch->countdown);
    return;
  }

  if ( ch->position <= POS_SLEEPING ) {
    stc( "�� �� ������ ������ ����� ������.\n\r", ch );
    return;
  }

  if (!str_prefix(arg1, "request")) {
    int objvnum = 0;
    CHAR_DATA *victim,*tmp;
    OBJ_DATA *questitem;

    if ( (questman=questman_lookup(ch, TRUE))==NULL) {
      stc("��� �� � ���� ��������� �������.\n\r", ch);
      return;
    }

    act( "$c1 �p���� � $C2 �������.", ch, NULL, questman, TO_ROOM);
    act ("�� �p����� � $C2 �������.",ch, NULL, questman, TO_CHAR);

    if (IS_SET(ch->act, PLR_QUESTOR)) {
      do_say(questman, "H� � ���� ��� ���� �������!");
      return;
    }

    if (ch->nextquest > 0) {
      do_printf(buf, "�������� %d ����� �� ���������� �������. ������� ����, ����� �p������.",ch->nextquest);
      do_say(questman, buf);
      return;
    }
    if (! ch->clan && ! ch->remort && ch->qcomplete[0] >= 400) {
        act( "$c1 ���������� '{G������, $C1{G, �� � ���� ������������ ����� ��� ���������� �������!{x'", questman, NULL, ch, TO_ROOM);
        return;
    }
    act( "$c1 ���������� '{G������� ����, ������$Y $C1!{x'", questman, NULL, ch, TO_ROOM);
    if (ch->questmob!=NULL) ch->questmob->questmob=NULL;
    ch->questmob = NULL;
    ch->questobj = 0;

    switch(number_range(1, 20)) // Quest Types select
    {
     case 3: // QUEST FIND ITEM
     case 4:
       victim=find_questmob(ch->level);
       if (!victim)
       {
         do_say(questman, "������, �� � ������ �� ���� ���� ���� �������.\n\r�p����� �����.");
         ch->nextquest = 2;
         return;
       }

       switch(number_range(0,4))
       {
        case 0: objvnum = QUEST_OBJQUEST1;break;
        case 1: objvnum = QUEST_OBJQUEST2;break;
        case 2: objvnum = QUEST_OBJQUEST3;break;
        case 3: objvnum = QUEST_OBJQUEST4;break;
        case 4: objvnum = QUEST_OBJQUEST5;break;
       }
       questitem = create_object( get_obj_index(objvnum), ch->level );
       if(questitem)
       {
        obj_to_room(questitem, victim->in_room);
        do_printf(buf, "������ ��p���� ��p�� {Y%s{x �� ��p�������� �����!",get_obj_desc(questitem,'4'));
        do_say(questman, buf);
        do_say(questman, "��� ��� ��������, � �� ���������� ��p�����, ����p����� ��� ��������������.");
        do_printf(buf, "��� � a��� {G%s{x ���-�� ����� {Y%s{x!",victim->in_room->area->name, victim->in_room->name);
        do_say(questman, buf);
        ch->questobj = questitem->pIndexData->vnum;
        ch->questroom= victim->in_room;
        set_quest_time(ch, Q_FIND_ITEM );
        ch->questmob=NULL;
        ch->questgiver=questman->pIndexData->vnum;
        SET_BIT(ch->act, PLR_QUESTOR);
        SET_BIT(ch->q_stat, Q_FIND_ITEM);
       }
       else
       {
        do_say(questman, "������, �� ��� ��������� � � �� ���� ������� ��� ���� ��������� �������..");
        do_say(questman, "��� ��� ����� ���� � ����� ������� ��� - ���� qp �����");
        return;
       } 
       break;
     case 1: // QUEST FOR FIND AND BRING ITEM
     case 2:
       tmp=find_questmob(ch->level);
       if (!tmp)
       {
         do_say(questman, "������, �� � ������ �� ���� ���� ���� �������.\n\r�p����� �����.");
         ch->nextquest = 2;
         return;
       }

       ch->questroom= tmp->in_room;
       victim=find_questmob(ch->level);
       if (!victim || victim==tmp)
       {
         do_say(questman, "������, �� � ������ �� ���� ���� ���� �������.\n\r������� �����.");
         ch->nextquest = 2;
         ch->questroom=NULL;
         return;
       }
       switch(number_range(0,4))
       {
        case 0: objvnum = QUEST_OBJQUEST1;break;
        case 1: objvnum = QUEST_OBJQUEST2;break;
        case 2: objvnum = QUEST_OBJQUEST3;break;
        case 3: objvnum = QUEST_OBJQUEST4;break;
        case 4: objvnum = QUEST_OBJQUEST5;break;
       }
       questitem = create_object( get_obj_index(objvnum), ch->level );
       if(questitem)
       {
       obj_to_room(questitem, ch->questroom);
       ch->questmob=NULL;
       ch->questobj = questitem->pIndexData->vnum;
       do_printf(buf, "�� ������ ����� {C%s{x � ������� ����� ����������.",get_obj_desc(questitem,'1'));
       do_say(questman, buf);
       do_printf(buf, "��� {C%s{x � a��� {G%s{x ���-�� ����� {Y%s{x!", get_obj_desc(questitem,'1'),ch->questroom->area->name, ch->questroom->name);
       do_say(questman, buf);
       do_printf(buf, "{Y%s{x ���� ���� � ���� {G%s{x!",get_char_desc(victim, '1'),victim->in_room->area->name);
       do_say(questman, buf);
       set_quest_time(ch, Q_BRING_ITEM );
       ch->questgiver=victim->pIndexData->vnum;
       SET_BIT(ch->act, PLR_QUESTOR);
       SET_BIT(ch->q_stat, Q_BRING_ITEM);
       }
       else
       {
       do_say(questman, "���.. ����! �� ���� ������� ��������� ����!");
       do_say(questman, "������ ����� �����: �������� ����� � ��������.");
       do_say(questman, "� ���� 2 ���� �� ���������� �������.");
       return;
       }
       break;

     default: // QUEST TO KILL MOB
       victim=find_questmob(ch->level);
       if (!victim)
       {
         do_say(questman, "������, �� � ������ �� ���� ���� ���� �������.\n\r������� �����.");
         ch->nextquest = 2;
         return;
       }
       switch(number_range(0,1))
       {
        case 0:
         do_printf(buf, "��� �p��, {R%s{x, ������� ��p��� ��p���.\n\r��� ��p��� ���������� ����������!",get_char_desc(victim,'1'));
         do_say(questman, buf);
         break;
        case 1:
         do_printf(buf, "������� �p��������, {R%s{G, ������ �� �������!",get_char_desc(victim,'1'));
         do_say(questman, buf);
         do_printf(buf, "� ��� ��p ������, {R%s{G ���� {R%d{G �������!",get_char_desc(victim,'1'), number_range(2,20));
         do_say(questman, buf);
         do_say(questman,"H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!");
         break;
       }
       do_printf(buf, "��� {R%s{G ���-�� � p����� {Y%s{G!",get_char_desc(victim,'4'),victim->in_room->name);
       do_say(questman, buf);
       do_printf(buf, "���� p���� ��������� � area {Y%s{G.",victim->in_room->area->name);
       do_say(questman, buf);
       set_quest_time(ch, Q_KILL_MOB);
       ch->questmob = victim;
       victim->questmob=ch;
       ch->questroom=victim->in_room;
       ch->questgiver=questman->pIndexData->vnum;
       SET_BIT(ch->act, PLR_QUESTOR);
       SET_BIT(ch->q_stat, Q_KILL_MOB);
       break;
    }
    do_printf(buf, "� ���� ���� %d ����� �� ���������� �������.\n\r�� �p������ � ����� ����!",ch->countdown);
    do_say(questman, buf);
    return;
  }

  if (!str_prefix(arg1, "complete")) {
    CHAR_DATA *questman;

    if (!IS_SET(ch->act, PLR_QUESTOR)) {
       stc("������� ����p��� REQUEST ��� ��������� �������.\n\r",ch);
       return;
    }

    if ( (questman=questman_lookup(ch, FALSE))==NULL) {
      stc("��� ����� �� ��������, � ��� �� ��������.\n\r", ch);
      return;
    }

    act( "{y$c1{x �������� {y$C3{x � ���������� �������.", ch, NULL, questman, TO_ROOM);
    act ("�� ��������� {y$C3{x, ��� ��������$r �������.",ch, NULL, questman, TO_CHAR);

    if ( QSTAT(ch, Q_KILL_MOB) && QSTAT(ch, Q_MOB_KILLED)) {
      int64 reward;
      int pointreward;

      reward= number_range ((ch->level+1)/2, (ch->level+10)*2)+1;
      ch->gold+=reward;
      pointreward = number_range(7,25);
      ch->questpoints+=pointreward;
      do_printf(buf,"� ���p���, � ��� ���� %d ��������� �����, � %u ������� �����.",pointreward,reward);
      complete_quest(ch, questman, buf);
      save_char_obj(ch);
      return;
    }
    if ( QSTAT(ch, Q_FIND_ITEM ) || QSTAT(ch, Q_BRING_ITEM)) {
      bool obj_found = FALSE;

      for (obj = ch->carrying; obj != NULL; obj= obj->next_content) {
        if (obj != NULL && obj->pIndexData->vnum == ch->questobj) {
          obj_found = TRUE;
          break;
        }
      }
      if (obj_found == TRUE) {
        int pointreward;
        int64 reward;

        obj_from_char( obj );
        reward=number_range ((ch->level+1)/2, (ch->level+10)*2)+1;
        pointreward = number_range(7,30);
        ch->gold+=reward;
        ch->questpoints+=pointreward;
        act("�� ��p������ $p $C3.",ch, obj, questman, TO_CHAR);
        act("{y$n{x ��p����� $p $C3.",ch, obj, questman, TO_ROOM);
        extract_obj(obj);
        do_printf(buf,"� ���p���, � ��� ���� %d ��������� �����, � %u ������� �����.",pointreward,reward);
        complete_quest(ch, questman, buf);
        return;
      }
    }
    do_printf(buf, "�� �� �������� ��� �������, �� � ���� ��� ���� �����. ����������!");
    do_say(questman, buf);
    return;
  }

  for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room ) {
    if (!IS_NPC(questman)) continue;
    if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
  }

  if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" )) {
    stc("�� �� ������ ����� ������� �����.\n\r",ch);
    return;
  }

  if ( questman->fighting != NULL) {
    stc("������� ���� ��������� �����.\n\r",ch);
    return;
  }

  if (!str_prefix(arg1, "list")) {
    int i;
    act( "$c1 ��p������� � $C2 ������ ����������.", ch, NULL, questman, TO_ROOM);
    act ("�� ��p�������� � $C2 ������ ���������� �� �������.",ch, NULL, questman, TO_CHAR);
    stc("{Y H� �p����� ���������� ��������� ��������:\n\r", ch);
    for (i=0;quest_table[i].level!=0;i++)
      ptc(ch, "%30s (%15s) - %5dqp\n\r", quest_table[i].show_name, quest_table[i].name, quest_table[i].cost);
    stc("��� ������� item, ����p� 'QUEST BUY <item>'.\n\r", ch);
    return;
  }

  if (!str_prefix(arg1, "cancel")) {
    if (!IS_SET(ch->act,PLR_QUESTOR)) {
      stc("� ���� ��� �������.\n\r",ch);
      return;
    }
    act( "$c1 �p���� $C2 �������� �������.", ch, NULL, questman, TO_ROOM);
    act ("�� �p����� $C2 �������� ������� .",ch, NULL, questman, TO_CHAR);
    ptc(ch, "�� ������� {Y%d{x ��������� �����.\n\r",cancel_quest(ch, TRUE,15,25));
    ptc(ch, "��p�� %d ����� �� ������� ����� ��������� �������.\n\r",ch->nextquest);
    save_char_obj(ch);
    return;
  }


  if (!str_prefix(arg1, "convert")) {
    if (ch->level < 100) {
     stc("�� ������� ���, ����� ������������ ���� ������������.\n\r", ch);
     return;
    } 
    
    if (ch->practice >= 10) {
      ch->practice -= 10;
      ch->questpoints += (15*150)/UMAX(ch->perm_stat[STAT_WIS],15);
      act( "$C1 ������ �������� $c2 �� QP.", ch, NULL, questman, TO_ROOM );
      ptc(ch, "{Y%s{x ������ ���� {W10{x ������� �� {W%d{x QP.\n\r",get_char_desc(questman,'1'),(15*150)/UMAX(ch->perm_stat[STAT_WIS],15));
      return;
    }
    do_printf(buf, "������, %s, �� � ���� �� ������� �������.",ch->name);
    do_say(questman,buf);
    return;
  }


  else if (!str_prefix(arg1, "buy")) {
    int item=-1, i, ttmp=0, qtmp=0;
   
    if (EMPTY(arg2)) {
        stc("��� ������� ��������, ����p� 'QUEST BUY <item>'.\n\r",ch);
        return;
    }

    for (i=0;quest_table[i].level!=0;i++) {
      if (is_name(arg2, quest_table[i].name)) {
        item=i;
        break;
      }
    }

    if (item==-1) {
      stc("������, �� ����� ��� � ������.\n\r",ch);
      return;
    }

    if (!str_cmp(arg3,"train")) {
      if (ch->level<100) {
        stc("�� ������� ���, ����� ������������ ���� ������������.\n\r", ch);
        return;
      }

      if (ch->train*165<quest_table[item].cost) {
        stc("� ���� ��� ������� ����������.\n\r", ch);
        return;
      }
      ttmp=quest_table[item].cost/165;
      if (ttmp*165<quest_table[item].cost) ttmp++;
      ch->train-=ttmp;
      qtmp=ttmp*165-quest_table[item].cost;
      ch->practice+=qtmp/20;
    }
    else
    {
      if (ch->questpoints<quest_table[item].cost) {
        stc("� ���� ��� ������� ��������� �����.\n\r", ch);
        return;
      }
      ch->questpoints-= quest_table[item].cost;
    }
    switch (item)
    {
      case 0:
        ch->practice += 30;
        act( "$C1 ���� 30 ������� $c3.", ch, NULL, questman, TO_ROOM );
        act( "$C1 ���� ���� 30 �������.",   ch, NULL, questman, TO_CHAR );
        break;
      case 1: 
        ch->gold += 20000;
        act( "$C1 ���� 20,000 ������� ����� $c3.", ch, NULL, questman, TO_ROOM );
        act( "$C1 ���� ���� 20,000 ������� �����.",ch, NULL, questman, TO_CHAR );
        break;
      default:
        if (quest_table[item].vnum!=-1) {
          obj = create_object(get_obj_index(quest_table[item].vnum),1);
          if(obj) {
            if (quest_table[item].level==-1) obj->level=ch->level;
            else obj->level=quest_table[item].level;
          }
          else {
            do_say(questman,"�����, �� ���� ������� ������ ���� ����.. ���� � �����!");
            ch->questpoints+= quest_table[item].cost;
          }
        } 
        break;
    }

    if (obj) {
      act( "$C1 ���� {w$p{x $c3.", ch, obj, questman, TO_ROOM );
      act( "$C1 ���� ���� {w$i4{x.",   ch, obj, questman, TO_CHAR );
      obj_to_char(obj, ch);
    }
    if (ttmp>0) ptc(ch,"��������� {G%d{x ����������. ����� {G%d{x �������.\n\r",ttmp,qtmp/20);
    return;
  }
  stc("QUEST �������: POINTS INFO TIME REQUEST COMPLETE LIST BUY CONVERT.\n\r",ch);
  stc("��� ������� ����p�����, ����p��� 'QUEST HELP'.\n\r",ch);
}

bool quest_level_diff(int clevel, int mlevel)
{
   if      (clevel < 5   && (mlevel>5))              return FALSE;
   else if (clevel < 10  && (abs(mlevel-clevel)>4))  return FALSE;
   else if (clevel < 15  && (abs(mlevel-clevel)>6))  return FALSE;
   else if (clevel < 35  && (abs(mlevel-clevel)>7))  return FALSE;
   else if (clevel < 65  && (abs(mlevel-clevel)>9))  return FALSE;
   else if (clevel < 90  && (abs(mlevel-clevel)>10)) return FALSE;
   else if (clevel < 95  && (abs(mlevel-clevel)>12)) return FALSE;
   else if                  (abs(mlevel-clevel)>15)  return FALSE;
   return TRUE;
}
                
void quest_update(void)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch;

  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if (!d->character || d->connected != CON_PLAYING) continue;
    
    ch = d->character;
    if (IS_SET(ch->act,PLR_QUESTOR)) {
      if (--ch->countdown <= 0) {
        ch->nextquest=number_range(7,23);
        ptc(ch, "�p��� ���������� �� ���������� ������� �����������!\n\r����� �� ������� ����� ������� ��p�� %d �����.\n\r",ch->nextquest);
        REM_BIT(ch->act, PLR_QUESTOR);
        ch->qcounter++;
        ch->questgiver = 0;
        ch->countdown = 0;
        if (ch->questmob!=NULL) ch->questmob->questmob=NULL;
        ch->questmob = NULL;
        ch->q_stat=0;
        if (ch->questobj!=0) ch->questobj=0; 
        continue;
      }

      if (ch->countdown > 0 && ch->countdown < 6) {
        stc("����� ����p�����, � ���� ����� �� �������� �p����� �� ���������� �������!\n\r",ch);
        continue;
      }
    }
    if (ch->nextquest > 0) {
      ch->nextquest--;
      if (ch->nextquest == 0) stc("�� ����� ������ ����� �������.\n\r",ch);
    }
  }
}

int cancel_quest(CHAR_DATA *ch, bool reward, int from,int to)
{
  int pointreward=0;

  if (!ch || IS_NPC(ch)) return 0;
  if (reward) pointreward = number_range(1,5);
  ch->questpoints=UMAX(0, ch->questpoints-pointreward);
  if (ch->questmob!=NULL) {
    ch->questmob->questmob=NULL;
    ch->questmob=NULL;
  }
  ch->questgiver= 0;
  if (ch->questobj!=0) ch->questobj=0;
  REM_BIT(ch->act, PLR_QUESTOR);
  ch->nextquest=number_range(from,to);
  if (reward) ch->qcounter++;
  ch->countdown = 0;
  ch->q_stat=0;
  return pointreward;
}

void complete_quest(CHAR_DATA *ch, CHAR_DATA *questman, char *buf)
{
  do_say(questman,"����p����� � ����������� �������!");
  do_say(questman,buf);

  if (number_percent()<15 || IS_SET(global_cfg,CFG_NEWYEAR)) {
    int pracreward = number_range(1,6);
    ptc(ch, "�� ��������� %d �������!\n\r",pracreward);
    ch->practice += pracreward;
  }
  else if (number_percent()<15) {
    int movereward = number_range(1,3);
    ptc(ch,"�� ��������� %d ��������!\n\r",movereward);
    ch->max_move += movereward;
    ch->pcdata->perm_move += movereward;
  }
  else if (number_percent()<5) {
    stc("�� ��������� 1 ����������!\n\r",ch);
    ch->train+=1;
  }
  else if (number_percent()<5) {
    int count;
    int qword;

    for(count=0;quenia_table[count].name!=Q_END;count++);
    qword=number_range(0,count-1);
    ptc(ch,"%s ������� ���� '{G� ������, ��� {C%s{G �� quenia ������ ��� '{C%s{x'\n\r",get_char_desc(questman,'1'),
      quenia_table[qword].descr,quenia_table[qword].word);
  }
  REM_BIT(ch->act, PLR_QUESTOR);
  ch->questgiver = 0;
  ch->countdown = 0;
  ch->qcomplete[0]++;
  ch->qcomplete[1]++;
  ch->qcounter++;
  ch->questobj = 0;
  ch->q_stat=0;
  ch->nextquest = number_range(3,10);
}

// Select mob as quest target
CHAR_DATA *find_questmob(int level)
{
  CHAR_DATA *victim=NULL;
  int count;

  count=number_range(0,310)*15;
  for (victim=char_list;;victim=victim->next) {
    if (!victim) victim=char_list;
    count--;
    if (count<1) break;
  }

  for (;;) {
    victim=victim->next;
    if (!victim) victim=char_list;
    if (++count > 4621) break;

    if (!IS_NPC(victim) || !victim->in_room) continue;
    if (IS_SET(victim->act,ACT_IS_HEALER|ACT_TRAIN|ACT_PRACTICE|ACT_IS_CHANGER|ACT_PET|ACT_NOQUEST)) continue;
    if (IS_SET(victim->affected_by, AFF_CHARM)) continue;
    if (victim->questmob!=NULL) continue;
    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) continue;
    if (victim->in_room->area!=victim->pIndexData->area) continue;
    if (IS_SET(victim->in_room->area->area_flags, AREA_NOQUEST)) continue;
    if (victim->hit<victim->max_hit/2) continue;
    if (victim->fighting) continue;
    if (victim->pIndexData->pShop) continue;
    if (quest_level_diff(level, victim->level) != TRUE) continue;
    if (IS_SET(victim->in_room->room_flags,ROOM_MAG_ONLY)
     || IS_SET(victim->in_room->room_flags,ROOM_CLE_ONLY)
     || IS_SET(victim->in_room->room_flags,ROOM_THI_ONLY)
     || IS_SET(victim->in_room->room_flags,ROOM_WAR_ONLY))  continue;

    if (number_range(1,10)>7) return victim;
  }
  return NULL;
}

void set_quest_time(CHAR_DATA *ch, int q_type)
{
  if (ch->level > 50) ch->countdown = number_range(10,25);
  else ch->countdown = number_range(10,20);
  if (q_type==Q_KILL_MOB || q_type==Q_BRING_ITEM) ch->countdown+=5;
}

CHAR_DATA *questman_lookup(CHAR_DATA *ch, bool request)
{
  CHAR_DATA *questman=NULL;

  for (questman=ch->in_room->people;questman!=NULL;questman=questman->next_in_room)
  {
    if (!IS_NPC(questman)) continue;
    if (request && questman->spec_fun==spec_lookup( "spec_questmaster" )) return questman;
    if (!request && ch->questgiver==questman->pIndexData->vnum) return questman;
  }
  return NULL;
}

void do_gquest(CHAR_DATA *ch, const char *argument)
{
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  int i;
  MOB_INDEX_DATA *mob;
  argument = one_argument(argument, arg1);

  if (!*arg1 || !str_prefix(arg1,"info")) {
    if (gquest.status==0) stc("������ ��� �������.\n\r",ch);
    else if (gquest.status==GQ_STARTING) {
      ptc(ch,"������� ��� ������� {Y%d-%d{x �������� ����� {Y%d{x �����\n\r",
        gquest.min_level,gquest.max_level,gquest.counter);
      ptc(ch,"����� ����� %d �������� �� %d �����.\n\r",gquest.mobs,gquest.tmp_counter);
      ptc(ch,"�� {Y%s{x ����� ������ ��� �������", ch->pcdata->gquest.status==GQ_JOINED?"���":"��� ��");
    }
    else {
      ptc(ch,"���� ������� ��� ������� %d-%d. ����� ����� %d �������� �� %d �����.\n\r",
        gquest.min_level,gquest.max_level,gquest.mobs,gquest.counter);

      if (ch->pcdata->gquest.status == GQ_JOINED) {
        stc("���� ����� �����:\n\r",ch);
        for (i=0; gquest.target[i]!=0 && i<20;i++) {
          mob=get_mob_index(gquest.target[i]);
          if (mob!=0) {
            if (IS_IMMORTAL(ch))
              ptc(ch,"[%2d �� %2d] %20s (%s) %s\n\r",ch->pcdata->gquest.target[i],gquest.target_counter[i],get_mobindex_desc(mob,'1'),mob->area->name,mob->player_name);
            else
              ptc(ch,"[%2d �� %2d] %20s (%s)\n\r",ch->pcdata->gquest.target[i],gquest.target_counter[i],get_mobindex_desc(mob,'1'),mob->area->name);
          }
        }
      }
      else stc("�� �� ������� ������ �� ������� � �������.",ch);
    }
    return;
  }

  if (!str_prefix(arg1,"join")) {
    if (gquest.status==0) {
      stc("������ ��� �������.\n\r",ch);
      return;
    }
    if (ch->pcdata->gquest.status==GQ_JOINED) {
      stc("�� ��� �������� � ���������� �������.\n\r",ch);
      return;
    }
    if (ch->level>gquest.max_level || ch->level<gquest.min_level) {
      stc("�� �� ������ ������� ������� � ���� �������.\n\r",ch);
      return;
    }
    for (i=0;i<20;i++) ch->pcdata->gquest.target[i]=0;
    ch->pcdata->gquest.status=GQ_JOINED;
    if (gquest.status==GQ_STARTING)
      ptc(ch,"�� ����� ������ �� �������. ������� �������� ����� {Y%d{x �����\n\r",gquest.counter);
    else if (gquest.status==GQ_STARTED)
      ptc(ch,"�� ����������� ���������� �������. �������� {%Y%d{x �����. �����!\n\r",gquest.counter);
    ch->gqcounter++;
    return;
  }

  if (!str_prefix(arg1,"complete"))
  {
    int reward,prac,train;
    char buf[MAX_INPUT_LENGTH];

    if (gquest.status!=GQ_STARTED) {
      stc ("������ ��� �������.\n\r",ch);
      return;
    }
    if (ch->pcdata->gquest.status!=GQ_JOINED) {
      stc("�� �� ���������� � �������.\n\r",ch);
      return;
    }
    for (i=0;gquest.target[i]!=0 && i<20;i++) {
      if (ch->pcdata->gquest.target[i]<gquest.target_counter[i]) {
        stc("�� ��� �� ���� ���� ��������.\n\r",ch);
        return;
      }
    }
    stc("�� ����������� ����� � ���������� �������.\n\r",ch);
    act_new("������� ����$b ��������$r $n!",ch,NULL,NULL,TO_ALL,POS_DEAD);
    reward=number_range(20,30);      
    prac=number_range(1,6);
    train=number_range(0,10)>8?1:0;
    ptc(ch,"� ������� �� ��������� %d quest points.\n\r",reward);
    if (prac>0) ptc(ch,"�� ��������� %d �������.\n\r",prac);
    if (train>0) ptc(ch,"�� ��������� 1 ����������.\n\r");

    obj = create_random_item(ch->level);
    obj_to_char(obj, ch);
    ptc(ch, "�� ��������� � �������� ������� %s.\n\r", get_obj_desc(obj, '4'));
    ch->questpoints+=reward;
    ch->practice+=prac;
    ch->train++;
    if (IS_DEVOTED_ANY(ch)) change_favour(ch, number_range(5,20));
    do_printf(buf,"%s ��������%s ������� �����.",ch->name,ch->sex==2?"a":"");
    send_news(buf,NEWS_GQUEST);
    ch->gqcomplete++;
    clear_gquest();
    return;
  }

  if (!str_prefix(arg1,"help")) {
    stc ("? or help - ��� �������\n\r",ch);
    stc ("join      - ������� ������� � ������� �������\n\r",ch);
    stc ("progress  - �������� ��������� � ������� ���������� �������� �������\n\r",ch);
    stc ("info      - ���������� � ������� (��� ���������� - ������ �����)\n\r",ch);
    stc ("complete  - �������� � ���������� �������\n\r",ch);
    if (IS_IMMORTAL(ch)) {
      stc ("stop      - ���������� ������� �������\n\r",ch);
      stc ("create    - �������� ������� ('gq create ?' ��� �������)\n\r",ch);
      stc ("show      - ���������� ��������� ���������� ��� �������\n\r",ch);
    }
    return;
  }

  if (IS_IMMORTAL(ch) && !str_prefix(arg1,"stop")) {
    clear_gquest();
    gecho("������� ����������� ������.");
    return;
  }

  if (IS_IMMORTAL(ch) && !str_prefix(arg1,"show")) {
    ptc(ch,"Gquest status: %d (levels %d-%d)\n\r",gquest.status,gquest.min_level,gquest.max_level);
    ptc(ch,"Counter: %d/%d\n\r",gquest.counter,gquest.tmp_counter);
    ptc(ch,"Mobiles: %d\n\r",gquest.mobs);
    for (i=0;i<20;i+=2)
    {
      ptc(ch,"{YMob:{x %20s %6u(%2d)  ",get_mobindex_desc(get_mob_index(gquest.target[i]),'1'),gquest.target[i],gquest.target_counter[i]);
      ptc(ch,"{YMob:{x %20s %6u(%2d)\n\r",get_mobindex_desc(get_mob_index(gquest.target[i+1]),'1'),gquest.target[i+1],gquest.target_counter[i+1]);
    }
    for (i=0;i<=90;i+=10) ptc(ch,"Rooms:%6u %6u %6u %6u %6u %6u %6u %6u %6u %6u\n\r",gquest.room[i],gquest.room[i+1],gquest.room[i+2],gquest.room[i+3],gquest.room[i+4],gquest.room[i+5],gquest.room[i+6],gquest.room[i+7],gquest.room[i+8],gquest.room[i+9]);
    return;
  }

  if (IS_IMMORTAL(ch) && !str_prefix(arg1,"create"))
  {
    int min, max, mobs;
    if (gquest.status!=0) {
      stc("������� ��� ����!\n\r",ch);
      return;
    }
    argument = one_argument(argument, arg1);
    if (!str_prefix(arg1,"help") || *arg1=='?') {
      stc("type: gquest create <min_level> <max_level> <mob count>\n\r",ch);
      return;
    }
    if (!is_number(arg1)) {
      stc("����� ������� �����\n\r",ch);
      return;
    }
    min=URANGE(1,atoi(arg1),101);

    argument = one_argument(argument, arg1);
    if (!is_number(arg1)) {
      stc("����� ������� �����\n\r",ch);
      return;
    }
    max=URANGE(1,atoi(arg1),101);
    argument = one_argument(argument, arg1);
    if (!is_number(arg1)) {
      stc("����� ������� �����\n\r",ch);
      return;
    }
    mobs=atoi(arg1);
    create_gquest(min,max,mobs);
    return;
  }

  if (!str_prefix(arg1,"progress")) {
    CHAR_DATA *tmp;
    int progress;

    if (gquest.status==0) {
      stc("������ ��� �������.\n\r",ch);
      return;
    }

    if(gquest.status==GQ_STARTING)
         ptc(ch,"������� ��� ������� {Y%d-%d{x �������� ����� {Y%d{x �����\n\r",
           gquest.min_level,gquest.max_level,gquest.counter);
    else ptc(ch,"���� ����� ��� ������� {Y%d-%d{x\n\r",gquest.min_level,gquest.max_level);

    for (tmp=char_list;tmp;tmp=tmp->next) {
      if (IS_NPC(tmp) || tmp->pcdata->gquest.status!=GQ_JOINED) continue;
      if (gquest.status==GQ_STARTING) ptc(ch,"%12s  0\n\r",tmp->name);
      else {
        progress=0;
        for(i=0;gquest.target[i]!=0 && i<20;i++) progress+=tmp->pcdata->gquest.target[i];
        ptc(ch,"%12s %3d%%\n\r",tmp->name,progress==gquest.mobs?100:100/gquest.mobs*progress);
      }
    }
    if (gquest.status==GQ_STARTED)
      ptc(ch,"�������� {Y%d{x ����� �� ����������\n\r",gquest.counter);
    return;
  }
}

void gquest_update(void)
{
  gquest.counter--;
  if (gquest.counter>0) return;
  if (gquest.status==GQ_STARTED) {
    clear_gquest();
    gecho("����� ���������� �� ���������� ������� �����.");
    return;
  }
  if (gquest.status==GQ_STARTING) {
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *tmp;
    bool found=FALSE;

    for (tmp=char_list;tmp;tmp=tmp->next) {
      if (IS_NPC(tmp)) continue;
      if (tmp->pcdata->gquest.status!=GQ_JOINED) continue;
      found=TRUE;
      break;
    }
    if (!found) {
      gecho("����� �� ���������� ������� �������, ������� ����������");
      clear_gquest();
      return;
    }
    do_printf(buf,"������� ��� ������� %d-%d ����������.�������� 'gquest ?' ��� �������",gquest.min_level,gquest.max_level);
    gecho(buf);
    gquest.status=GQ_STARTED;
    gquest.counter=gquest.tmp_counter;
    return;
  }
  if (IS_SET(global_cfg,CFG_GQUEST)) create_gquest(0,0,0);
}

void create_gquest(int min,int max, int mobs)
{
  int i=0,t;
  CHAR_DATA *mob;
  bool found;
  char buf[MAX_INPUT_LENGTH];

  if (mobs<=0 || mobs>20) mobs=number_range(3,12);
  gquest.mobs=mobs;
  if (min<=0 || max<min)
  {
    min=number_range(1,95);
    max=URANGE(8,min+8,101);
  }
  gquest.min_level=min;
  gquest.max_level=max;

  for (;mobs>0;mobs--)
  {
    mob=find_questmob(min);
    if (!mob) mob=find_questmob(min+1);
    if (!mob) return;
    found=FALSE;
    for (t=0;t<=i;t++) {
      if(gquest.target[t]==mob->pIndexData->vnum) {
        found=TRUE;
        gquest.target_counter[t]++;
        break;
      }
    }
    if (!found) {
      gquest.target[i]=mob->pIndexData->vnum;
      gquest.target_counter[i]=1;
      i++;
    }
  }
  t=0;
  for (i=0;i<20;i++) {
    if (gquest.target[i]==0) break;
    for (mob=char_list;mob;mob=mob->next) {
      if (!IS_NPC(mob) || gquest.target[i]!=mob->pIndexData->vnum || !mob->in_room) continue;
      gquest.room[t]=mob->in_room->vnum;
      /* 
       * `break' breaks only current `for' 
       *
       * (unicorn)
       */
      if (++t>=99) break;
    }
    /* 
     * Ugly fix for gquest.room[] overflow.
     * APAR: [*****] BUG: Bad sn 10 in get_skill. 9515
     *       (gsn_dodge follows the gquest static info and was rewritten by
     *       room number 9515 (somewhere in New Thalos), thus that skills 
     *       doesn't work properly)
     * TODO: increase enthrophy in rooms election.
     * (uni)
     */
    if (t >= 99) break;
  }
  gquest.status=GQ_STARTING;
  gquest.counter=3;
  gquest.tmp_counter=gquest.mobs*(3+(min>5)+(min>15)+(min>25)+(min>45)+(min>60)+(min>75)+(min>90)+(min>95))+2;
  do_printf(buf,"����������� ����� ������� ��� ������� %d-%d.������� �������� ����� 3 ����",min,max);
  gecho(buf);
}

void clear_gquest()
{
  int i;
  CHAR_DATA *tmp;

  gquest.status=0;
  for (i=0;i<20;i++) {
    gquest.target[i]=0;
    gquest.target_counter[i]=0;
  }
  gquest.min_level=0;
  gquest.max_level=0;
  gquest.tmp_counter=0;
  gquest.mobs=0;
  for (tmp=char_list;tmp;tmp=tmp->next) {
    if (IS_NPC(tmp)) continue;
    tmp->pcdata->gquest.status=0;
    for (i=0;i<20;i++) tmp->pcdata->gquest.target[i]=0;
  }
  for (i=0;i<100;i++) gquest.room[i]=0;
  gquest.counter=number_range(10,20);
}

bool is_gqmob(int64 vnum)
{
  int i;
  if (gquest.status!=GQ_STARTED) return FALSE;

  for (i=0;i<20;i++) if (gquest.target[i]==vnum) return TRUE;
  return FALSE;
}
