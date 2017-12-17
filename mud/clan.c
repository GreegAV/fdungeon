// $Id: clan.c,v 1.38 2004/07/29 06:39:13 mud Exp $
// Copyrights (C) 1998-2001, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"
#include "olc.h"

struct clan_opt
{
  char *name;
  int64 bit;
  bool on;
};

#define MAX_CLAN_OPTIONS 15

const struct clan_opt clanopt_table[] =
{
 { "������ ����� �������� ���� ��������������.",  CLAN_CANTLEAVE,  TRUE  },
 { "Second ����� ��������� � ����.            ",  CLAN_SEC_ACCEPT, FALSE },
 { "Second ����� �������� � ������.           ",  CLAN_SEC_REJECT, FALSE },
 { "Second ����� ������ ���� �� �������.      ",  CLAN_SEC_RAISE,  FALSE },
 { "Second ����� ��������� �� �����.          ",  CLAN_SEC_REMOVE, FALSE },
 { "Second ����� �������� ������� ����������. ",  CLAN_SEC_SHOW,   FALSE },
 { "Second ����� �������� �����.              ",  CLAN_SEC_WAR,    FALSE },
 { "Second ����� ��������� ��� � ������.      ",  CLAN_SEC_ALLI,   FALSE },
 { "Deputy ����� ��������� � ����.            ",  CLAN_DEP_ACCEPT, FALSE },
 { "Deputy ����� �������� � ������.           ",  CLAN_DEP_REJECT, FALSE },
 { "Deputy ����� ������ ���� �� Deputy.       ",  CLAN_DEP_RAISE,  FALSE },
 { "Deputy ����� ��������� �� �����.          ",  CLAN_DEP_REMOVE, FALSE },
 { "Deputy ����� �������� ������� ����������. ",  CLAN_DEP_SHOW,   FALSE },
 { "Deputy ����� �������� �����.              ",  CLAN_DEP_WAR,    FALSE },
 { "Deputy ����� ��������� ��� � ������.      ",  CLAN_DEP_ALLI,   FALSE }
};

void free_clan(CLAN_DATA *clan);
CLAN_DATA *new_clan();
void remove_pkiller(CHAR_DATA *ch, const char *name);
void save_clans();
char *clan_name(int64 comm_flags, bool turn);
void clan_toggle(CHAR_DATA *ch,int64 comm_flag);
bool rem_clanskill(CLAN_DATA *clan,int sn);
bool add_clanskill(CLAN_DATA *clan, int sn, int64 time);

void do_petition_list(CLAN_DATA *clan, CHAR_DATA *ch)
{
  DESCRIPTOR_DATA *d;
  bool flag = FALSE;

  for (d = descriptor_list; d; d = d->next)
  {
    CHAR_DATA *victim;
    victim = d->character;
        
    if (!victim || d->connected != CON_PLAYING || victim->clanpet!=clan) continue;

    if (!flag)
    {
      flag = TRUE;
      stc("��������� ������ ������ �������� � ���� ����:\n\n\r", ch);
    }
    ptc(ch,"[%3d %5s %s][%-10s%-16s]\n\r",
     victim->level,race_table[victim->race].who_name,
     (victim->remort>0) ? "MLT":class_table[victim->class[victim->remort]].who_name,
     victim->name,victim->pcdata->title);
  }
  if (!flag) stc("����� �� ������� ��������.\n\r", ch);
  return;
}

void do_petition(CHAR_DATA *ch, const char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  short status;
  CLAN_DATA *clan;
  int gn = 0;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  status = ch->clan ? ch->clanrank : 0;

  if (arg1[0] == 0)
  {
    if (!ch->clan)
    {
      if (!ch->clanpet) stc("{R�� �� ������� �������� � ����.{x\n\r", ch);
      else ptc(ch, "{G�� ����� �������� � ���� {Y%s{G.{x\n\r", ch->clanpet->name);
    }
    else if (ch->clanrank>=DEPUTY) do_petition_list(ch->clan, ch);
    stc("{C�������� petition help ��� ������� �� ��������.{x\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "help") )
  {
    stc("���������: petition                ���������� ���� ������ ��������\n\r", ch);
    stc("                                   ��� ��� ����� ��������.\n\r", ch);
    stc("���������: petition to <clanname>  ������ �������� � ����.\n\r", ch);
    stc("���������: petition cancel         ������� ��������.\n\r", ch);
    stc("���������: petition list           ������ ������.\n\r\n\r", ch);
    stc("���������: petition <accept|reject> <��� ������>\n\r", ch);
    stc("���������: petition raise <��� ������> <������>\n\r", ch);
    stc("���������: petition remove <��� ������>\n\r",ch);
    return;
  }

  if (!str_cmp(arg1, "cancel") )
  {
    if (ch->clanpet)
    {
      ch->clanpet = NULL;
      stc("{R�� ��������� ���� ���������.{x\n\r", ch);
    }
    else stc( "�� �� ����� �������� � ����.\n\r", ch);
    return;
  }

  if (!str_cmp(arg1, "list") )
  {
    stc("{C������ ������������ ������.{x\n\r", ch);
    for (clan=clan_list;clan!=NULL;clan=clan->next)
      ptc(ch, "%s %s\n\r", clan->show_name, clan->name);
    return;
  }

  if (!str_cmp (arg1, "to"))
  {

    if (IS_SET(ch->act,PLR_LASTREMORT))
    {
      stc("����������� �� ����� � ����.\n\r",ch);
      return;
    }
    if((clan = clan_lookup(arg2)) == NULL )
    {
      stc("������ ����� ���.\n\r",ch);
      return;
    }

    if (clan == ch->clan)
    {
      ptc( ch,"�� ��� ���� ����� {G%s{x.\n\r", clan->name);
      return;
    }

    if (ch->clan!=NULL && IS_SET(ch->clan->flag, CLAN_CANTLEAVE))
    {
      stc("{R���� ������� ������ ��������� �� �����.{x\n\r", ch);
      return;
    }

    if (ch->clanrank==LEADER)
    {
      stc("Leader �� ����� ���� �� �����, �� ������� ������ ���������.\n\r", ch);
      return;
    }

    ch->clanpet = clan;
    ptc(ch,"�� ����� �������� � �������� � ���� {G%s{x.\n\r",clan->name);
    return;
  }

  if (ch->clan==NULL || IS_SET(ch->clan->flag, CLAN_LONER))
  {
    stc("{R�� ������ ���� � �����.{x\n\r", ch);
    return;
  }

  if (!str_prefix(arg1, "accept"))
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_ACCEPT))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_ACCEPT))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

    if ((victim = get_pchar_world(ch, arg2)) == NULL)
    {
      stc("����� ������ ���.\n\r", ch);
      return;
    }

   if (victim->clanpet != ch->clan)
   {
     stc("�������� �� ����� ������ �����������.\n\r", ch);
     return;
   }

   victim->clan = ch->clan;
   victim->clanrank = MEMBER;
   gn=group_lookup(ch->clan->name); //Add a clan skills
   if ( gn > -1 ) gn_add(victim,gn);
   if (!str_cmp(victim->clan->name,"chaos")) victim->clanrank = SECOND;
   victim->clanpet = NULL;
   stc("{C�� ������������ ��������.{x\n\r", ch);
   stc("{G���� �������� � �������� � ���� ���������.{x\n\r", victim);
   ptc(victim,"������ �� ������������ ���� ����� {G%s{x.\n\r",victim->clan->name);
   return;
  }

 if (!str_prefix(arg1, "reject"))
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_REJECT))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_REJECT))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

   if ((victim = get_pchar_world(ch, arg2)) == NULL)
        { stc("����� ������ ���.\n\r", ch);
          return; }

   if (victim->clanpet != ch->clan)
        { stc("�������� �� ����� ������ ���.\n\r", ch);
          return; }

   victim->clanpet = NULL;
   stc("{R�� ���������� ��������.{x\n\r", ch);
   stc("{R���� ���� �������� � ��������.{x\n\r", victim);
   return;
  }

  if (!str_prefix(arg1, "raise") && status>=DEPUTY)
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_RAISE))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_RAISE))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }
    if ((victim = get_pchar_world(ch, arg2)) == NULL)
    {
      stc("����� ������ ���.\n\r", ch);
      return;
    }

    if (ch==victim)
    {
      stc("�� �� ������ �������� ���� ������. ������� �������.\n\r",ch);
      return;
    }

   if (victim->clan != ch->clan )
   {
     stc("��� �� ���� ������ �����.\n\r", ch);
     return;
   }

   if (!str_prefix(arg3,"junior")) victim->clanrank=JUNIOR;
   else if (!str_prefix(arg3,"senior")) victim->clanrank=SENIOR;
   else if (!str_prefix(arg3,"deputy")) victim->clanrank=DEPUTY;
   else if (!str_prefix(arg3,"second"))
   {
     if (ch->clanrank==DEPUTY)
     {
       stc("Only Deputy allowed to you.\n\r",ch);
       return;
     }
     victim->clanrank=SECOND;
   }
   else if (!str_cmp(arg3,"leader"))
   {     if (ch->clanrank != LEADER)
     {
       stc("{RLeader and IMMORTALS can raise to Leader status.{x\n\r",ch);
       return;
     }
     victim->clanrank=LEADER;
     ch->clanrank=SECOND;
   }
   else
   {
     stc("�������� �����: junior senior deputy second leader\n\r",ch);
     stc("                Leader ������� �������� ���������.\n\r",ch);
     return;
   }
   ptc(ch,"{Y%s{x ������ {G%s{x � �����.\n\r",victim->name,clan_ranks[victim->clanrank]);
   ptc(victim,"�� ������ {G%s{x � �����.\n\r",clan_ranks[victim->clanrank]);
   return;
 }

 if (!str_prefix(arg1, "remove") && status>=DEPUTY)
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_REMOVE))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_REMOVE))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

   if ((victim = get_pchar_world(ch, arg2)) == NULL)
        { stc("����� ������ ���.\n\r", ch);
        return; }

   if (ch==victim)
        { stc("�� �� ������ ������� ����.\n\r",ch);
        return; }

   if (victim->clan != ch->clan )
        { stc("��� �� ���� ������ �����.\n\r", ch);
        return; }

   stc("{R�� �������� �� �����!{x\n\r",victim);
   stc("{R�� ��������� ��� �� �����.{x\n\r",ch);
   victim->clan=clan_lookup("loner");
   victim->clanrank=0;
   gn=group_lookup(ch->clan->name);
   if ( gn > -1 ) gn_remove(victim,gn);
   return;
  }
  do_petition(ch, "help");
}

void do_cleader( CHAR_DATA *ch, const char *argument )
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CLAN_DATA *clan;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  // Lists all possible clans
  if (arg1[0] == '\0'|| arg2[0] == '\0')
  {
    stc( "������ ������:{x\n\r",ch);
    for(clan = clan_list; clan !=NULL; clan=clan->next)
    {
      if(!IS_SET(clan->flag, CLAN_LONER)) ptc( ch, "   {G%s{x\n\r", clan->name);
    }
        
    stc( "\n\r���������: {Gcleader {c<{w��� ������{c> <{w��� �����{c>{x\n\r",ch);
    stc( "If {c<{w��� �����{c>{x is {r'{wnone{r'{x ��������� ����� ������.\n\r",ch);
    return;
  }

  if ( ( victim = get_pchar_world( ch, arg1 ) ) == NULL || IS_NPC(victim))
  {
    stc( "����� � ����� ������ ���.\n\r", ch );
    return;
  }

  if(!str_cmp(arg2,"none"))
  {
    if(victim->clanrank == LEADER)
    {
      if (!victim->clan || !victim->clan->name)
      {
        ptc( ch,"�� �������� %s � ����� ������ ������������ �����.\n\r",victim->name);
        ptc( victim, "�� ������ �� ����� ������������ �����.\n\r",victim->clan->name);
        victim->clanrank = MEMBER;
        return;
      }
      ptc( ch,"�� �������� %s � ����� ������ ����� {G%s{x.\n\r",victim->name,victim->clan->name);
      ptc( victim, "�� ������ �� ����� {G%s{x!\n\r",victim->clan->name);
      victim->clanrank = MEMBER;
      return;
    }
    else
    {
      ptc( ch,"%s � ��� �� �����.\n\r",victim->name);
      return;
    }
  }

  if((clan = clan_lookup(arg2)) == NULL)
  {
    stc("������ ����� ���.\n\r",ch);
    return;
  }
  else
  {
    if(victim->clanrank == LEADER && victim->clan && victim->clan->name)
    {
       ptc( ch, "�� ����� ����� %s.������� ������� ���.\n\r",victim->clan->name);
       return;
    }
     ptc(ch,"%s ������ {G�����{x ����� {G%s{x.\n\r",victim->name,clan->name);
     ptc(victim,"�� ������ {G�����{x ����� {G%s{x.\n\r",clan->name);
     victim->clan = clan;
     victim->clanrank = LEADER;
   }
}

void make_corpse( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  DESCRIPTOR_DATA *d;

  if ( IS_NPC(ch) )
  {
    if (IS_SET(ch->act, ACT_EXTRACT_CORPSE))
    {
      act( "������� ���� ��������� ����������� � ����.", ch, 0, 0,TO_ROOM);
      return;
    } 
    corpse              = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
    corpse->timer       = number_range( 3, 6 );
    if ( ch->gold > 0 )
    {
      obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
      ch->gold = 0;
      ch->silver = 0;
    }
    corpse->cost = 0;
  }
  else
  {
    corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer   = number_range( 25, 40 );

    corpse->owner = str_dup(ch->name);
    if (ch->gold > 1 || ch->silver > 1)
    {
      obj_to_room(create_money(ch->gold, ch->silver), ch->in_room);
      act( "���������� {y$n{x ������������ �� �����", ch, 0, 0,TO_ROOM);
      ch->gold  = 0;
      ch->silver = 0;
    }
    corpse->cost = 0;
  }

  corpse->level = ch->level;
  ptc(ch,"{R[{x%s{x]\n\r",ch->name);
  do_printf( buf, corpse->short_descr, get_char_desc(ch,'2') );
  free_string( corpse->short_descr );
  corpse->short_descr = str_dup( buf );

  do_printf( buf, corpse->description, get_char_desc(ch,'2') );
  free_string( corpse->description );
  corpse->description = str_dup( buf );

  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
  {
    bool floating = FALSE;

    obj_next = obj->next_content;
    if (obj->wear_loc == WEAR_FLOAT) floating = TRUE;
    obj_from_char( obj );
    if (obj->item_type == ITEM_POTION) obj->timer = number_range(500,1000);
    if (obj->item_type == ITEM_SCROLL) obj->timer = number_range(1000,2500);
    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
    {
       obj->timer = number_range(5,10);
       REM_BIT(obj->extra_flags,ITEM_ROT_DEATH);
    }
    REM_BIT(obj->extra_flags,ITEM_VIS_DEATH);

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) extract_obj( obj );
    else if (floating)
    {
      if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
      { 
        if (obj->contains != NULL)
        {
          OBJ_DATA *in, *in_next;

          act("{y$p{x p�����p�����, ��� ����p����� ������������.",ch,obj,NULL,TO_ROOM);
          for (in = obj->contains; in != NULL; in = in_next)
          {
            in_next = in->next_content;
            obj_from_obj(in);
            obj_to_room(in,ch->in_room);
          }
        }
        else act("{y$p{x p�����p�����.", ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
      }
      else
      {
        act("{y$p{x ������ �� �����.",ch,obj,NULL,TO_ROOM);
        obj_to_room(obj,ch->in_room);
      }
    }
    else
    obj_to_obj( obj, corpse );
  }

  if (IS_NPC(ch))   obj_to_room( corpse, ch->in_room );
  else if (IS_SET(ch->act,PLR_ARMY)) obj_to_room( corpse, get_room_index(ROOM_VNUM_ARMY_BED));
  else if (ch->clan==NULL) obj_to_room( corpse,get_room_index(ROOM_VNUM_ALTAR) );
  else obj_to_room(corpse,get_room_index(ch->clan->clandeath) );

  if (!IS_NPC(ch))
  {
    /* 
     * So, if he/she dies when I was in disconnect, he/she left in my PK?
     *  - heh: currently, only mob's can `forget' (:
     */
    for ( d = descriptor_list; d != NULL; d = d->next )
    if (d->connected == CON_PLAYING) remove_pkiller(d->character,ch->name);
  }
}

void do_ear( CHAR_DATA *victim, CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];

  if (ch==victim) return;
  obj = create_object(get_obj_index(ITEM_EAR),victim->level);
  do_printf( buf, "�|��|��|��|��|���|��|��|��� %s", victim->name);
  free_string( obj->short_descr );
  obj->short_descr = str_dup( buf );
                  
  do_printf( buf1, "��� %s (%s %d �������), ���������� ",victim->name ,clasname(victim),victim->level);
  do_printf( buf,"%s %s (%s %d �������).",buf1,ch->name, clasname(ch),ch->level);

  free_string( obj->description );
  obj->description = str_dup( buf );
  obj_to_char( obj, ch );
  act( "$c1 � �������� ������ �������� ���� ���.", ch, NULL,victim, TO_VICT );
  act( "�� � �������� ������ ��������� ��� $C2.",  ch, NULL,victim, TO_CHAR);
  act( "$c1 � �������� ������ �������� ��� $C2.",  ch, NULL,victim, TO_ROOM);
}

void do_clanwork(CHAR_DATA *ch, const char *argument)
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 char arg3[MAX_INPUT_LENGTH];
 char arg4[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];
 const char *argtemp;
 CLAN_DATA *clan;
 int count=0;
 bool found=FALSE;
 AFFECT_DATA *apply;
 int64 value;

 if (!*argument)
 {
   stc("Syntax: clanwork [list help save <name>]\n\r",ch);
   return;
 }

 if (!str_prefix(argument,"help"))
 {
   stc ("���������:\n\r",ch);
   stc ("clanwork list                  - �������� ������������ ������\n\r",ch);
   stc ("clanwork create <name>         - ������� ����\n\r",ch);
   stc ("clanwork save                  - ��������� ���� clans.dat\n\r",ch);
   stc ("clanwork <name>                - ��������� ���������� �� �����\n\r",ch);
   stc ("clanwork <name> <command> <arg>- �������� �������� �����\n\r",ch);
   stc ("���������� ����� ����� ����:\n\r",ch);
   stc ("                 deleteclan   - ������� ���� (������ ���������).\n\r",ch);
   stc ("                 name         - ��� �����.\n\r",ch);
   stc ("                 showname     - �������� �����.\n\r",ch);
   stc ("                 recal        - vnum clanrecal room\n\r",ch);
   stc ("                 death        - vnum death room\n\r",ch);
   stc ("                 msg1         - ��������� ��� clanrecal (�������)\n\r",ch);
   stc ("                 msg2         - ��������� ��� clanrecal (������� �����)\n\r",ch);
   stc ("                 wear         - wear location of clanenchant\n\r",ch);
   stc ("                 short        - short name of clanenchant\n\r",ch);
   stc ("                 long         - long name of clanenchant\n\r",ch);
   stc ("                 apply        - add apply to clanenchant\n\r",ch);
   stc ("                 addskill     - add clanskill\n\r",ch);
   stc ("                 remskill     - remove clanskill\n\r",ch);
   stc ("                 remove <num> - remove apply to clanenchant\n\r",ch);
   return;                  
 }

 if (!str_cmp(argument,"save"))
 {
   save_clans();
   stc("{R���� ��������.\n\r{x",ch);
   return;
 }

 if (!str_cmp(argument,"list"))
 {
   ptc(ch,"\n\rNum   Name       Show_name\n\r");
   for (clan=clan_list;clan!=NULL;clan=clan->next)
   {
    count++;
    ptc(ch,"%5d %10s %s\n\r",count,clan->name,clan->show_name);
   }
   return;
 }

 argument = one_argument(argument, arg1);

 if (!str_cmp(arg1,"create"))
 {
   argument = one_argument(argument, arg2);

   if (!*arg2)
   {
     stc("������� �������� ������ �����.\n\r",ch);
     return;
   }

   clan=clan_lookup(arg2);
   if (clan)
   {
     stc("��� ���� ���� � ������� ������. ������� ������ ���.\n\r",ch);
     return;
   }
   clan = new_clan ();
   clan->next=clan_list;
   clan_list=clan;
   clan->clanrecal=ROOM_VNUM_TEMPLE;
   clan->clandeath=ROOM_VNUM_ALTAR;
   clan->name=str_dup(arg2);
   clan->recalmsg1="{y$c1{x ����� ����� ��������� ���!";
   clan->recalmsg2="{y$c1{x ��������.";
   clan->show_name=str_dup(arg2);
   ptc(ch,"������ ����� ���� '%s'.\n\r",clan->name);
   do_printf(buf,"show %s",clan->name);
   do_clanwork(ch,buf);
   return;
 }

 clan=clan_lookup(arg1);

 if(!clan)
 {
   stc ("��� ����� ������� ��� �����.\n\r", ch);
   return;
 }

 argument = one_argument(argument, arg2);
 argtemp = one_argument(argument, arg3);

 if (EMPTY(arg2))
 {
   ptc(ch,"\n\rName      : %s\n\rShowName  : %s\n\r",clan->name,clan->show_name);
   ptc(ch,"Recalmsg1 : %s\n\rRecalmsg2 : %s\n\r",clan->recalmsg1,clan->recalmsg2);
   ptc(ch,"Death     : %u\n\rRecal     : %u\n\r",clan->clandeath,clan->clanrecal);
   ptc(ch,"War       : %s\n\r",(clan->war)? clan->war:"<empty>");
   ptc(ch,"Alli      : %s\n\r",(clan->alli)? clan->alli:"<empty>");
   ptc(ch,"AcceptAlli: %s\n\r",(clan->acceptalli)? clan->acceptalli:"<empty>");
   if (clan->wear_loc!=-1)
   {
     int number=1;
     ptc(ch,"ClanEnchant: wear %s\n\r",flag_string(wear_flags,clan->wear_loc));
     ptc(ch,"Shortname  : %s\n\rLongname   : %s\n\rApplies:\n\r",clan->short_desc,clan->long_desc);
     if (!clan->mod) ptc(ch,"nothing\n\r");
     else
      for(apply=clan->mod;apply!=NULL;apply=apply->next)
      {
       ptc(ch,"[%3d] %s by %d\n\r",number,flag_string(apply_flags,apply->location),apply->modifier); 
       number++;
      }
   }
   for (count=0;count<20;count++)
   {
     if (clan->clansn[count]==0) break;
     if (clan->clansnt[count]==-1)
          ptc(ch,"�����: [{G%s{x] ��� �����������.\n\r",skill_table[clan->clansn[count]].name);
     else ptc(ch,"�����: [{G%s{x] {G%d{x �����\n\r",skill_table[clan->clansn[count]].name,clan->clansnt[count]);
   }
   return;
 }

 if (!str_cmp(arg2,"deleteclan"))
 {
   CLAN_DATA *clanold;

   if (clan_list==clan) clan_list=clan->next;
   else
   for (clanold=clan_list;clanold;clanold=clanold->next)
   {
     if (clanold->next==clan)
     {
       clanold->next=clan->next;
       break;
     }
   }
   ptc(ch,"���� %s ������.\n\r���� ����� ������������ clans.dat,\n\r�� ����������� clanwork save �� reboot-�.\n\r",clan->name);
   free_clan(clan);
   return;
 }

 if (!str_cmp(arg2,"name"))
 {
   free_string(clan->name);
   clan->name=str_dup(arg3);
   found=TRUE;
 }
 else if (!str_cmp(arg2,"showname"))
 {
   if (argument[0]=='\0') return;
   free_string(clan->show_name);
   clan->show_name=str_dup(argument);
   found=TRUE;
 }
 else if (!str_cmp(arg2,"msg1"))
 {
   free_string(clan->recalmsg1);
   clan->recalmsg1=str_dup(argument);
   found=TRUE;
 }
 else if (!str_cmp(arg2,"msg2"))
 {
   free_string(clan->recalmsg2);
   clan->recalmsg2=str_dup(argument);
   found=TRUE;
 }
 else if (!str_cmp(arg2,"short"))
 {
   free_string(clan->short_desc);
   clan->short_desc=str_dup(argument);
   found=TRUE;
 }
 else if (!str_cmp(arg2,"long"))
 {
   free_string(clan->long_desc);
   clan->long_desc=str_dup(argument);
   found=TRUE;
 }
 else 
 {
  if (!str_cmp(arg2,"wear"))
  {
    if ((value = flag_value(wear_flags,arg3)) != NO_FLAG)
    {
      clan->wear_loc=value;
      found=TRUE;
    }
    else
    {
      stc( "���������� ��������:\n\r", ch );
      show_help( ch, "? wear" );
      found=TRUE;
    }
  }
  else if (!str_cmp(arg2,"recal") && is_number(arg3))
  {
     clan->clanrecal=atoi64(arg3);
     found=TRUE;
  }
  else if (!str_cmp(arg2,"death") && is_number(arg3))
  {
    clan->clandeath=atoi64(arg3);
    found=TRUE;
  }
  else if (!str_cmp(arg2,"addskill"))
  {
    int64 time;
    int sn=skill_lookup(arg3);
 
    argtemp=one_argument(argtemp,arg4);

    if (EMPTY(arg4)) time=-1;
    else if (is_number(arg4)) time=atoi64(arg4);
    else
    {
      stc("������� ���������� �����.\n\r",ch);
      return;
    }
    if (add_clanskill(clan,sn,time))
         ptc(ch,"�� ������� ����� {G%s{x ��� ����� %s.\n\r",skill_table[sn].name,clan->name);
    else stc("�� ������ ����� ����� ��� � ����� ��� ����� ��� ����� �������.\n\r",ch);
    found=TRUE;
  }
  else if (!str_cmp(arg2,"remskill") )
  {
    int sn;
    argtemp=one_argument(argtemp,arg4);
    sn=skill_lookup(arg3);
    if (rem_clanskill(clan,sn))
         ptc(ch,"�� ����� ����� {G%s{x ��� ����� %s.\n\r",skill_table[sn].name,clan->name);
    else stc("� ����� ��� ������ ������ ��� ����������.\n\r",ch);
    found=TRUE;
  }
  else if (!str_cmp(arg2,"apply"))
  {
   argtemp=one_argument(argtemp,arg4);

   if ((value = flag_value(apply_flags,arg3)) == NO_FLAG)
   {
      stc( "���������� ��������:\n\r", ch );
      show_help( ch, "? affect" );
      found=TRUE;
   }
   else
   {
     if (!is_number(argtemp))
     {
       stc("�������� ������ ���� ������.\n\r",ch);
       found=TRUE;
     }
     apply            = new_affect();
     apply->location  = (int)value;
     apply->modifier  = atoi(arg4);
     apply->type      = 0;
     if( apply->duration > 0)
       apply->duration  = atoi(arg4);
     apply->bitvector = 0;
     apply->next      = clan->mod;
     clan->mod      = apply;
     found=TRUE;
   }
 }
 else if (!str_cmp(arg2,"remove"))
 {
   int number;
   AFFECT_DATA *tmp;

   if (!clan->mod)
   {
     stc("���� �� ����� ��������.\n\r",ch);
     return;
   }
   if (!is_number(arg4))
   {
     stc("�������� ������ ���� ������.\n\r",ch);
     found=TRUE;
   }

   number=atoi(arg3);
   if (number<1 || number>100)
   {
     stc("������� ������ ����� �������.\n\r",ch);
     return;
   }
   if (number==1)
   {
     apply=clan->mod;
     clan->mod=apply->next;
     free_affect(apply);
     found=TRUE;
   }
   else
   {
     for (apply=clan->mod;apply;apply=apply->next)
     {
       number--;
       if (number==1)
       {
         if (!apply->next)
         {
           stc("� ����� ��� ������� ������������.\n\r",ch);
           return;
         }
         tmp=apply->next;
         apply->next=tmp->next;
         free_affect(tmp);
         break;
       }
     }
     found=TRUE;
   }
 }
 }
 if (found) do_clanwork(ch,clan->name);
 else stc("������������ �������.\n\r",ch);
}

void do_leader( CHAR_DATA *ch, const char *argument )
{
  DESCRIPTOR_DATA d;
  bool found = FALSE;
  char name[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if (!ch->clan)
  {
    stc("���.\n\r",ch);
    return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, name);

  if (!*arg1 || !str_prefix(arg1, "status"))
  {
    int count;

    for (count=0; count < MAX_CLAN_OPTIONS ;count++ )
    {
      ptc(ch,"{C%d {G%s{x %s\n\r",count, clanopt_table[count].name,
       IS_SET(ch->clan->flag, clanopt_table[count].bit) ?
       ((clanopt_table[count].on)?"���":"��"):
       ((clanopt_table[count].on)?"��":"���"));
    }
    for (count=0;count<20;count++)
    {
      if (ch->clan->clansn[count]==0) break;
      if (ch->clan->clansnt[count]==-1)
           ptc(ch,"�����: [{G%s{x] ��� �����������.\n\r",skill_table[ch->clan->clansn[count]].name);
      else ptc(ch,"�����: [{G%s{x] {G%d{x �����\n\r",skill_table[ch->clan->clansn[count]].name,ch->clan->clansnt[count]);
    }
    return;
  }

  if (IS_SET(ch->clan->flag,CLAN_LONER))
  {
    stc("���.\n\r",ch);
    return;
  }

  if (!str_prefix(arg1, "help"))
  {
    stc("������� �� ���������� ������, ��������� ������� ������� � �������.\n\r",ch);
    stc(" ���������:  leader <command> <name>\n\r",ch);
    stc(" �������: status toggle show remove raise.",ch);
    return;
  }

  if (!str_prefix(arg1, "toggle") && ch->clanrank==LEADER)  
  {
    int count;

    if (!is_number(name))
    {
      stc ("���������: leader toggle <number>\n\r", ch);
      return;
    }

    count=atoi(name);

    if (count >= MAX_CLAN_OPTIONS) stc("��� ����� �����.\n\r", ch);
    else
    {
      ch->clan->flag=toggle_int64(ch->clan->flag, clanopt_table[count].bit);
      ptc(ch,"{C%d {G%s{x %s\n\r",count, clanopt_table[count].name,
       IS_SET(ch->clan->flag, clanopt_table[count].bit) ?
       ((clanopt_table[count].on)?"���":"��"):
       ((clanopt_table[count].on)?"��":"���"));
    }
    return;
  }

  name[0] = UPPER(name[0]);

  if (!str_prefix(arg1,"remove"))
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_REMOVE))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_REMOVE))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

    if ( get_pchar_world( ch, name ) != NULL)
    {
      stc( "���� �������� ������ � ����.\n\r", ch );
      return;
    }

    found = load_char_obj(&d, name, SAVE_NORMAL);

    if (!found)
    {
      stc("������ ����� �� ����������������.\n\r", ch);
      return;
    }

    victim=d.character;

    if (victim->clan==ch->clan && ch->clanrank>victim->clanrank)
    {
      victim->clanrank=0;
      victim->clan=clan_lookup("loner");
      ptc(ch,"%s ������ �����.\n\r",victim->name);
      save_char_obj(victim);
    } 
    else stc ("�� �� ������ ����� �������\n\r",ch);
    extract_char(victim,TRUE);
    return;
  }

  if (!str_prefix(arg1,"show"))
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_SHOW))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_SHOW))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

    if ( get_pchar_world( ch, name ) != NULL)
    {
      stc( "���� �������� ������ � ����.\n\r", ch );
      return;
    }
    found = load_char_obj(&d, name, SAVE_NORMAL);

    if (!found)
    {
      stc("������ ����� �� ����������������.\n\r", ch);
      return;
    }

    victim=d.character;

    if (victim->clan==ch->clan)
    {
      ptc(ch,"\n\r|Level|Name        |Race      |Class       |Clanrank    |\n\r");
      ptc(ch,"|%5d|%12s|%10s|%12s|%12s\n\r",
        victim->level,           
        victim->name,
        race_table[victim->race].who_name,
        classname(victim),
        clan_ranks[victim->clanrank]);
    } 
    else stc ("�� �� ������ ����� �������\n\r",ch);
    extract_char(victim,TRUE);
    return;
  }

  if (!str_prefix(arg1,"raise"))
  {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_RAISE))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_RAISE))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

    if ( get_pchar_world( ch, name ) != NULL)
    {
      stc( "���� �������� ������ � ����.\n\r", ch );
      return;
    }

    found = load_char_obj(&d, name, SAVE_NORMAL);

    if (!found)
    {
      stc("������ ����� �� ����������������.\n\r", ch);
      return;
    }

    victim=d.character;

    argument = one_argument(argument, arg2);

    if (victim->clan==ch->clan)
    {

      if (arg2[0]=='\0' || !str_prefix(arg2,"member")) victim->clanrank=0;
      else if (!str_prefix(arg2,"junior")) victim->clanrank=1;
      else if (!str_prefix(arg2,"senior")) victim->clanrank=2;
      else if (!str_prefix(arg2,"deputy")) victim->clanrank=3;
      else if (!str_prefix(arg2,"second")) victim->clanrank=4;
      else
      {
        stc("�������� �����: member junior senior deputy second.",ch);
        return;
      }
      ptc(ch,"%s ������ %s\n\r",victim->name,clan_ranks[victim->clanrank].name);
      save_char_obj(victim);
    } 
    else stc ("�� �� ������ ����� �������\n\r",ch);
    extract_char(victim, TRUE );
    return;
  }
}

void do_diplomacy( CHAR_DATA *ch, const char *argument )
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 CLAN_DATA *clan;

 if (ch->clan==NULL)
 {
   stc ("���?\n\r", ch);
   return;
 }

 argument = one_argument( argument, arg1 );
 argument = one_argument( argument, arg2 );

 if (arg1[0]=='\0' || !str_cmp(arg1, "show"))
 {
   stc("{GCurrent Status:{x\n\r", ch);
   if (IS_SET(ch->clan->flag,CLAN_LONER))
   {
     stc("� �������� ��� ��������������� ���������.\n\r", ch);
     return;
   }
   for(clan=clan_list;clan!=NULL;clan=clan->next)
   {
    if (ch->clan==clan || IS_SET(clan->flag,CLAN_LONER)) continue;
    if (is_exact_name(clan->name,ch->clan->war))
    {
      ptc(ch, "%s {RWAR        {x", clan->show_name);
      if (is_exact_name(ch->clan->name,clan->acceptalli)) stc(" ({Gthey proposing peace{x)", ch);
      if (is_exact_name(clan->name,ch->clan->acceptalli)) stc(" ({Cyou proposing peace{x)", ch);
    }
    else if (is_exact_name(clan->name,ch->clan->alli)) ptc(ch, "%s {GALLIANCE   {x", clan->show_name);
    else
    {
      ptc(ch, "%s {xNeutralitet{x", clan->show_name);
      if (is_exact_name(ch->clan->name,clan->acceptalli)) stc(" ({Gthey proposing alliance{x)", ch);
      if (is_exact_name(clan->name,ch->clan->acceptalli)) stc(" ({Cyou proposing alliance{x)", ch);
    }
    stc("\n\r", ch);
   }
   stc("Usage: diplomacy <show|war|alli|help>\n\r", ch);
   return;
 }

 if (!str_cmp(arg1, "help"))
 {
   stc("����������:\n\r", ch);
   stc("���������� ����� {R�� ����������� �� ��������{x. �������� {G�� ����� ������� ���� �� �����{x.\n\r", ch);
   stc("{CDipl war <clan>{x  - �������� ����� �����. ��� ������� ��������� �����������.\n\r", ch);
   stc("{CDipl alli <clan>{x - ���������� ���� ������������ ����� ��� ���������� ���\n\r", ch);
   stc("                   ����������� �����. ��� ���������� �������� ������ ���� ������\n\r", ch);
   stc("                   ��� �� ������� {Cdiplomacy alli <your clan>{x. ���� �������\n\r", ch);
   stc("                   {Cdiplomacy alli <clan>{x} ��������, ����������� ����������.\n\r", ch);
   return;
 }

 if (ch->clan==NULL || IS_SET(ch->clan->flag,CLAN_LONER) || ch->clanrank<DEPUTY)
 {
   stc("������ ������� �������� ���������� �� ������� Deputy � ����.\n\r", ch);
   return;
 }

 if (!str_cmp(arg1, "war"))
 {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_WAR))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_WAR))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

   if (arg2[0]=='\0' || (clan=clan_lookup(arg2))==NULL)
   {
     stc("������ ����� ���.\n\r", ch);
     return;
   }

   if (clan==ch->clan || IS_SET(clan->flag,CLAN_LONER))
   {
     stc("��� ����� ����� ������� �� ����� :/.\n\r", ch);
     return;
   }

   if (is_exact_name(clan->name, ch->clan->war))
   {
     stc("{R�� ��� ��������� � ���� ������.{x\n\r", ch);
     return;
   }
   clan->alli=remove_word(clan->alli, ch->clan->name);
   ch->clan->alli=remove_word(ch->clan->alli, clan->name);
   clan->acceptalli=remove_word(clan->acceptalli, ch->clan->name);
   ch->clan->acceptalli=remove_word(ch->clan->acceptalli, clan->name);

   clan->war=add_word(clan->war, ch->clan->name);
   ch->clan->war=add_word(ch->clan->war, clan->name);
   ptc(ch, "{R�� ���������� ����� ����� {Y%s{R.{x\n\r", clan->name);
   save_clans();
   return;
 }

 if (!str_cmp(arg1, "alli"))
 {
    if ((ch->clanrank==SECOND && !IS_SET(ch->clan->flag, CLAN_SEC_ALLI))
     || (ch->clanrank==DEPUTY && !IS_SET(ch->clan->flag, CLAN_DEP_ALLI))
     || ch->clanrank<DEPUTY)
    {
      stc ("� ���� �� ������� ����.\n\r", ch);
      return;
    }

   if (arg2[0]=='\0' || (clan=clan_lookup(arg2))==NULL)
   {
     stc("������ ����� ���.\n\r", ch);
     return;
   }

   if (clan==ch->clan || IS_SET(clan->flag,CLAN_LONER))
   {
     stc("{G�� ��������� ���� ��� ����, �������� ������ ������� :/.{x\n\r", ch);
     return;
   }

   if (is_exact_name(clan->name, ch->clan->acceptalli))
   {
     ch->clan->acceptalli=remove_word(ch->clan->acceptalli, clan->name);
     ptc(ch,"{C�� ��������� ����������� ����� ����� {Y%s{C.{x\n\r",clan->name);
     save_clans();
     return;
   }

   if (is_exact_name(clan->name, ch->clan->alli))
   {
     stc("{G�� ��� �������� � ���� ������.{x\n\r", ch);
     return;
   }

   if (!is_exact_name(ch->clan->name, clan->acceptalli))
   {
     ch->clan->acceptalli=add_word(ch->clan->acceptalli, clan->name);
     if (is_exact_name(ch->clan->name,clan->war))
          ptc(ch, "{G�� ����������� ������ ������� ����� {Y%s{x\n\r", clan->name);
     else ptc(ch, "{G�� ����������� ���� ����� {Y%s{x\n\r", clan->name);
     save_clans();
     return;
   }

   if (is_exact_name(clan->name, ch->clan->war))
   {
     clan->war=remove_word(clan->war, ch->clan->name);
     ch->clan->war=remove_word(ch->clan->war, clan->name);
     clan->acceptalli=remove_word(clan->acceptalli, ch->clan->name);
     ch->clan->acceptalli=remove_word(ch->clan->acceptalli, clan->name);
     ptc(ch, "{G�� ��������� ������ ������� � ������ {Y%s{G.{x\n\r", clan->name);
     save_clans();
     return;
   }

   clan->acceptalli=remove_word(clan->acceptalli, ch->clan->name);
   ch->clan->acceptalli=remove_word(ch->clan->acceptalli, clan->name);
   clan->alli=add_word(clan->alli, ch->clan->name);
   ch->clan->alli=add_word(ch->clan->alli, clan->name);
   ptc(ch, "{G�� ��������� ���� � ������ {Y%s{G.{x\n\r", clan->name);
   save_clans();
   return;
 }

 stc("Usage: diplomacy <alli|show|war|help>\n\r", ch);
}

void do_clanbank( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  if (!IS_IMMORTAL(ch))
    if (!ch->clan || !str_cmp(ch->clan->name, "loner"))
    {
      stc("{R�� ������ ���� � �����.{x\n\r", ch);
      return;
    } 

  if (EMPTY(argument))
  {
    stc("���������: clanbank show\n\r", ch);
    stc("           clanbank put <���-��> qp/gold/diamonds/crystals\n\r", ch);
    if (IS_IMMORTAL(ch))
    {
     stc("           clanbank show <����>\n\r", ch);
     stc("           clanbank get <���-��> qp   <����>\n\r", ch);
     stc("           clanbank get <���-��> gold <����>\n\r", ch);
    }
    return;
  }

  argument=one_argument(argument, arg);

  if (!str_prefix(arg, "show"))
  {
    CLAN_DATA *clan;

    clan=ch->clan;

    if (IS_IMMORTAL(ch))
    {
      if (!str_cmp(argument, "all"))
      {
        for (clan=clan_list;clan;clan=clan->next)
        {
         if (IS_SET(ch->clan->flag, CLAN_LONER)) continue;
         ptc(ch, "{C%15s: {M%10d{C qp,  {Y%10u{C ������.{x\n\r", clan->name, clan->qp, clan->gold);
        }
        return;
      }
      if (!EMPTY(argument)) clan=clan_lookup(argument);
      if (clan==NULL)
      {
        stc("������ ����� ���.\n\r", ch);
        return;
      }
    }
    if (clan==NULL || IS_SET(ch->clan->flag, CLAN_LONER))
         stc("{R��� ������ �����,  ��� �������� ���� � ��� �����������.{x\n\r", ch);
    else ptc(ch, "{G�������� ����{x %s\n\r{G----------------------------------------{x\n\r{MQuest Points{x: %d\n\r{Y������      {x: %u\n\r", clan->show_name, clan->qp, clan->gold);
    return;
  }

  if (!str_prefix(arg, "put"))
  {
    if (ch->clan==NULL || IS_SET(ch->clan->flag, CLAN_LONER))
    {
      stc("{R�� ������ ���� � �����.{x\n\r", ch);
      return;
    }
    argument=one_argument(argument, arg);

    if (!is_number(arg))
    {
      do_clanbank(ch, "");
      return;
    }

    if (!str_prefix(argument, "gold"))
    {
      int64 count;
      count=atoi64(arg);
      if (count<1 || ch->gold < count)
      {stc("� ���� ��� ������� ������\n\r", ch);return;}
      ch->gold-=count;
      ptc(ch, "�� ������� {Y%u{x ������ � ��������\n\r", count, ch->clan->show_name);
      ch->clan->gold+=count;
      ptc(ch,"�� ����� � %s ������ {Y%u{C.{x\n\r", ch->clan->show_name,ch->clan->gold);
    }
    else if (!str_prefix(argument, "diamonds"))
    {
      int64 count;
      count=sell_gem(ch,atoi(arg),OBJ_VNUM_DIAMOND);
      ptc(ch, "�� ������� ���������� � ���������� %u ���� �� ���� �����. ��� %u ������.\n\r", count, (int64)count*90);
      ch->clan->gold+=count*90;
      ptc(ch,"�� ����� � %s ������ {Y%u{C.{x\n\r", ch->clan->show_name,ch->clan->gold);
    }
    else if (!str_prefix(argument, "crystals"))
    {
      int64 count;
      count=sell_gem(ch,atoi(arg),OBJ_VNUM_CRYSTAL);
      ptc(ch, "�� ������� ��������� � ���������� %u ���� �� ���� �����. ��� %u ������.\n\r", count, (int64)(count*9000));
      ch->clan->gold+=count*9000;
      ptc(ch,"�� ����� %s ������ {Y%u{C.{x\n\r",ch->clan->show_name,ch->clan->gold);
    }
    else if (!str_prefix(argument, "qp"))
    {
      int count=0;
      count=atoi(arg);

      if (count<1 || ch->questpoints < count)
      {stc("� ���� ��� ������� ����� �������\n\r", ch);return;}
      ch->questpoints-=count;
      ch->clan->qp+=count;
      ptc(ch, "�� ������� {M%d{x qp � �������� %s\n\r", count, ch->clan->show_name);
      ptc(ch,"�� ����� � %s ������ {Y%d{C.{x\n\r", ch->clan->show_name,ch->clan->qp);
    }
    else 
    {
      do_clanbank(ch, "");
      return;
    }
    save_char_obj(ch);
    save_clans();
    return;
  }

  if (!str_prefix(arg, "get"))
  {
    CLAN_DATA *clan;
    char arg1[MAX_INPUT_LENGTH];

    argument=one_argument(argument, arg);
    if (!is_number(arg))
    {
      do_clanbank(ch, "");
      return;
    }

    if (!IS_IMMORTAL(ch)) argument=str_dup(ch->clan->name);
    else argument=one_argument(argument, arg1);

    if (!str_prefix(arg1, "gold"))
    {
      int64 count=atoi64(arg);
      clan=clan_lookup(argument);
      if (!clan) {stc("������ ����� ���.\n\r", ch);return;}
      if (count<1 || clan->gold<count) {stc("� ����� ��� ������� ������.\n\r", ch);return;}
      clan->gold-=count;
      if (ch->clan==clan) ch->gold+=count;
      ptc(ch, "�� ����� %s ����� {Y%u{x ������.\n\r", clan->show_name, count);
      ptc(ch,"�� ����� � %s ������ {Y%u{C.{x\n\r", clan->show_name,clan->gold);
    }
    else if (!str_prefix(arg1, "qp") && IS_IMMORTAL(ch))
    {
      int count=atoi(arg);
      if (count<0) return;
      clan=clan_lookup(argument);
      if (!clan) {stc("������ ����� ���.\n\r", ch);return;}
      if (count<1 || clan->qp<count) {stc("� ����� ��� ������� qp.\n\r", ch);return;}
      clan->qp-=count;
      ptc(ch,"�� ����� %s ����� {M%d{x qp.\n\r", clan->show_name, count);
      ptc(ch,"�� ����� � %s ������ {Y%d{C.{x\n\r", clan->show_name,clan->qp);
    }
    else
    {
      do_clanbank(ch, "");
      return;
    }
    save_clans();
    return;
  }
  do_clanbank(ch, "");
}

bool clan_cfg(CLAN_DATA *clan, int64 flag)
{
  if (!clan) return FALSE;
  if (!IS_SET(clan->flag,flag)) return FALSE;
  return TRUE;
}

bool rem_clanskill(CLAN_DATA *clan,int sn)
{
  int i;

  if (sn < 1) return FALSE;
  for (i=0;i<20;i++)
  {
    if (clan->clansn[i]==0) break;
    if (clan->clansn[i]==sn)
    {
      for (;i<20;i++)
      {
        clan->clansn[i]=clan->clansn[i+1];
        clan->clansnt[i]=clan->clansnt[i+1];
      }
      clan->clansn[20]=0;
      clan->clansnt[20]=0;
      return TRUE;;
    }
  }
  return FALSE;
}

bool add_clanskill(CLAN_DATA *clan, int sn, int64 time)
{
  int i;
  if (sn<1) return FALSE;
  for (i=0;i<20;i++)
  {
    if (clan->clansn[i]==sn)
    {
      if (time==-1) clan->clansnt[i]=time;
      else if (clan->clansnt[i]==-1) clan->clansnt[i]=time;
      else clan->clansnt[i]+=time;
      return TRUE;
    }
    if (clan->clansn[i]!=0) continue;
    clan->clansn[i]=sn;
    clan->clansnt[i]=time;
    return TRUE;
  }
  return FALSE;
}

