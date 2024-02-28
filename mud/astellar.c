// Copyrights (C) 1998-2003, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'
#include <sys/types.h>
#include <time.h>
#include <ctype.h> 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include "merc.h" 
#include "tables.h"
#include "interp.h"
#include "recycle.h"

void load_deities       ( );
void save_deities       ( );
void panish_effect      ( CHAR_DATA *ch);
void dec_worship        ( CHAR_DATA *ch);
void show_deity_applies ( CHAR_DATA *ch, int deity);

void save_deities()
{
  FILE * fp;
  int counter = 0, i, deity;

  fclose (fpReserve);
  if( (fp = fopen(DEITY_FILE, "w")) == NULL )
  {
    bug ("Write permission denied to 'DEITY_FILE'.", 0);
    fpReserve = fopen (NULL_FILE, "r");
    return;
  }
 
  do_fprintf(fp, "Version 2\n");
  for ( deity = 0;deity<MAX_DEITIES; deity++)
  {
    if ( !deity_table[deity].name) break;
    do_fprintf(fp, "#Deity\n");
    do_fprintf(fp, "Name %s~\n", deity_table[deity].name);
    do_fprintf(fp, "Russian %s~\n", deity_table[deity].russian);
    do_fprintf(fp, "MinAlign %d\n", deity_table[deity].min_align);
    do_fprintf(fp, "MaxAlign %d\n", deity_table[deity].max_align);
    do_fprintf(fp, "Descr %s~\n", deity_table[deity].descr);
    do_fprintf(fp, "Worship %d\n", deity_table[deity].worship);
    do_fprintf(fp, "App");
    for( i = 0; i < MAX_DEITY_APP; i++)
      do_fprintf(fp," %d", deity_table[deity].d_apply[i] );
    do_fprintf(fp,"\nEND\n");
    counter++;
  }
  do_fprintf(fp,"#END\n");
  do_fprintf(fp,"; Detected and saved %d deities.\n", counter);
  log_printf("Saved %d deities.", counter);
  fclose  (fp);

  fpReserve = fopen (NULL_FILE, "r");
}

int get_vacant_deity()
{
  int v_deity;
  for( v_deity = 0; v_deity < MAX_DEITIES ; v_deity++)
  {
    if( deity_table[v_deity].name == NULL && deity_table[v_deity+1].name == NULL ) 
      return v_deity;
  }
  bug("Error in deity data file!", 0);
  return -1;
}

bool may_devote( CHAR_DATA *ch, int dn)
{
  if( IS_NPC(ch) ) return FALSE;
  switch (ch->race)
  {
     case RACE_HUMAN:
     case RACE_UNIQUE:
       break;
     case RACE_ELF:
       if( dn != 0  && dn != 1  && dn != 4 && dn != 9 && dn != 10 
        && dn != 13 && dn != 15 && dn != 16 ) return FALSE;
       break;
     case RACE_DWARF:
       if( dn != 4 && dn != 5  && dn != 6 && dn != 7 && dn != 8  
        && dn != 9 && dn !=11 && dn != 12 && dn != 17) return FALSE;
       break;
     case RACE_GIANT:
       if( dn != 6 && dn != 7 && dn != 8 && dn != 11 && dn != 12 )
           return FALSE;
       break;
     case RACE_VAMPIRE:
       if( dn != 2  && dn != 3 && dn != 5  && dn != 6  && dn != 8 && dn != 14 )
           return FALSE;
       break;
     case RACE_OGRE:
       if( dn != 2  && dn !=  3 && dn != 7 && dn != 8 && dn != 14
        && dn != 17 ) return FALSE;
       break;
     case RACE_HOBBIT:
       if( dn != 3 &&  dn != 9 &&  dn != 13 ) return FALSE;
       break;
     case RACE_ORC:
       if( dn !=  2 && dn != 6 && dn != 7 && dn != 8 && dn != 12 )
           return FALSE;
       break;
     case RACE_DROW:
       if( dn !=  2 && dn != 3  && dn != 5 && dn != 8
        && dn != 13 && dn != 14 && dn != 17 ) 
           return FALSE;
       break;
     case RACE_CENTAUR:
       if( dn != 0 && dn != 1 && dn !=  3 && dn !=  4 && dn != 7 
        && dn != 8 && dn != 9 && dn != 10 && dn != 12 ) return FALSE;
       break;
     case RACE_HGRYPHON:
       if( dn != 0  && dn !=  1 && dn != 2  && dn != 7  && dn != 8 && dn != 9 
        && dn != 12 && dn != 15 && dn !=16  && dn != 17 ) return FALSE;
       break;
     case RACE_LIZARD:
       if( dn !=  1 && dn != 3  && dn != 7 && dn != 9 && dn != 14 && dn != 16)
           return FALSE;
       break;
     case RACE_ETHERIAL:
       if( dn !=  0 && dn != 2  && dn != 4  && dn != 6 && dn != 9 && dn != 14
        && dn != 15 && dn != 16 && dn != 17 ) return FALSE;
       break;
     case RACE_SPRITE:
       if( dn != 1  && dn != 2  && dn != 4  && dn != 5 && dn != 6 && dn != 9 
        && dn != 10 && dn != 13 && dn != 14 && dn != 15 && dn != 16 && dn != 17 ) 
           return FALSE;
       break;
     case RACE_DRUID:
       if( dn != 0  && dn != 1  && dn != 4 && dn != 9 && dn != 10 && dn != 15
        && dn != 16 && dn != 17 ) return FALSE;
       break;
     case RACE_SKELETON:
       if( dn != 2  && dn != 5 && dn != 6 && dn != 7 && dn != 8 && dn != 14 ) 
           return FALSE;
     case RACE_ZOMBIE:
       if( dn != 2  && dn != 5 && dn != 8 && dn != 12 && dn != 14 ) return FALSE;
       break;

  }
  if( deity_table[dn].min_align > ch->alignment
   || deity_table[dn].max_align < ch->alignment ) return FALSE;

  return TRUE;
};

void dec_worship( CHAR_DATA *ch)
{
  if( !IS_DEVOTED_ANY(ch) ) return;
  deity_table[ch->pcdata->dn].worship -= 1;
  if( deity_table[ch->pcdata->dn].worship < 0 )
    deity_table[ch->pcdata->dn].worship = 0;
};

bool favour( CHAR_DATA *ch, int dvalue )
{
  int fcount;

  if( !IS_DEVOTED_ANY(ch) ) return FALSE;
  if( IS_ELDER(ch) ) return TRUE;
  if( IS_SET( ch->act, PLR_HIGHPRIEST ) ) dvalue /= 2;
  else
   for( fcount=0; t_favour[fcount].fav_afstr; fcount++)
     if( (ch->pcdata->favour >= t_favour[fcount].from) && (ch->pcdata->favour <= t_favour[fcount].to) )
     {
       dvalue *= t_favour[fcount].amp;
       break;
     }
  if( dvalue < 0 && (ch->pcdata->favour + dvalue >= -50) )
  {
    ch->pcdata->favour += dvalue;
    return TRUE;
  }
  else if( dvalue > 0 )
  {
    ch->pcdata->favour += dvalue;
    return TRUE;
  }
  ptc( ch, "{C%s{x �� ������ ����� ������...\n\r", get_rdeity( deity_table[ch->pcdata->dn].russian, '1') );
  return FALSE;
}

void change_favour( CHAR_DATA *ch, int cvalue)
{
//  if( !IS_DEVOTED_ANY(ch) ) return;
  if( cvalue < 0 && IS_SET( ch->act, PLR_HIGHPRIEST ) ) cvalue /= 2;
//  if( number_range(1,3) > 2 - (IS_SET( ch->act, PLR_HIGHPRIEST )) ) return;
  ch->pcdata->favour += cvalue;
  if( ch->pcdata->favour < -5000 ) ch->pcdata->favour = -5000;
  if( ch->pcdata->favour >  5000 ) ch->pcdata->favour =  5000;
};

int favour_string( CHAR_DATA *ch)
{
 int fcount;

 for( fcount=0; t_favour[fcount].fav_afstr; fcount++)
   if( (ch->pcdata->favour >= t_favour[fcount].from) && (ch->pcdata->favour <= t_favour[fcount].to) )
     return fcount;
 return MIDDLE_FAVOUR;
};

#define dtab deity_table[ch->pcdata->dn]
int deity_char_power( CHAR_DATA *ch, int type, int subtype)
{
  int sum=0;

  if( !IS_DEVOTED_ANY(ch) ) return sum;
  switch(type)
  {
    case 1:
      if( dtab.d_apply[0] == subtype ) sum += dtab.d_apply[1];
      if( dtab.d_apply[2] == subtype ) sum -= dtab.d_apply[3];
      break;
    case 2:
      if( dtab.d_apply[4] == subtype ) sum += dtab.d_apply[5];
      if( dtab.d_apply[6] == subtype ) sum -= dtab.d_apply[7];
      break;
    case 3:
/*
      if( str_cmp( deity_apply_table[dtab.d_apply[8]].res_flag,
          deity_apply_table[dtab.d_apply[10]].res_flag ) )
      {
        if( deity_apply_table[dtab.d_apply[8]].res_flag )
        {
           do_printf( buf," char %s resist %s", ch->name, 
            deity_apply_table[dtab.d_apply[8]].res_flag );
          do_function( ch, &do_flag, buf);
          free_buf(buf);
        }
        if( deity_apply_table[dtab.d_apply[10]].res_flag )
        {
          do_printf( buf,"char %s vuln %s", ch->name,
            deity_apply_table[dtab.d_apply[10]].res_flag );
          do_function( ch, &do_flag, buf);
        }
      }
*/
    default:
      break;
  }
  return sum;
};
#undef dtab

void show_deity_applies( CHAR_DATA *ch, int deity)
{
  int number;
  char *incdec;

  if( ch == NULL ) return;

  for( number=0; number < MAX_DEITY_APP; number +=2 )
  {
    if( number % 4 == 0) incdec = "{G�����������{x";
    else incdec = "{R���������{x";
    if( deity_table[deity].d_apply[number] == 0 ) continue;
    else 
     switch(number)
     {
       case 0 :
       case 2 :
         if( deity_apply_table[deity_table[deity].d_apply[number]].param )
         {
           ptc( ch,"%s {m%s{x �� {Y%d{x.\n\r", incdec,
             deity_apply_table[deity_table[deity].d_apply[number]].param,
             deity_table[deity].d_apply[number+1] );
         }
         break;
       case 4 :
       case 6 :
         if( deity_apply_table[deity_table[deity].d_apply[number]].inform )
         {
           ptc( ch,"%s ������ {m%s{x �� {Y%d{x.\n\r", incdec,
             deity_apply_table[deity_table[deity].d_apply[number]].inform,
             deity_table[deity].d_apply[number+1] );
         }
         break;
       case 8 :
       case 10:
         if( deity_apply_table[deity_table[deity].d_apply[number]].resist )
         {
           ptc( ch,"%s ������ �� {m%s{x �� {Y%d{x.\n\r", incdec,
             deity_apply_table[deity_table[deity].d_apply[number]].resist,
             deity_table[deity].d_apply[number+1] );
         }
         break;
       case 12:
       case 14:
       case 16:
       case 18:
       default:
         break;
     }
  }
};

void punish_effect( CHAR_DATA *ch )
{
  if( ch && ch->deity)
  {
    ch->pcdata->dc = 1000;
    ch->pcdata->dr = 0;
    ch->godcurse = 5;
    SET_BIT(ch->act, PLR_DISAVOWED);
    stc( "��� ���� ������ ������� ������ ������...\n\r", ch );
    act("���� ���������� $c1.",ch,NULL,NULL,TO_ROOM);
    save_char_obj(ch);
  }
  WAIT_STATE( ch, 3 * PULSE_VIOLENCE);
};

void do_gfix( CHAR_DATA *ch, const char *argument)
{
  int gvalue, gdef, i;

  if( EMPTY(argument) || !str_cmp( argument, "help") )
  {
    stc("{cGlobal fix command for defined conception.\n\r", ch);
    stc("{cThis command is quiet {Rdangerouse{c. Use it wisely!\n\r", ch);
    stc("{cUsage:\n\r", ch);
    stc("{G  gfix <parameter>\n\r", ch);
    stc("{cAvailiable parameters:{G clearworships clearapplies{x.\n\r", ch);
    return;
  }

  if( !IS_ELDER(ch) )
  {
    stc("{RYou have no access to this command.{x\n\r", ch);
  }

  if( !IS_SET(global_cfg, CFG_GTFIX) )
  {
    stc("GTFix global configuration has to be toggled 'on' to use this option.\n\r", ch);
    return;
  }

  if( !str_cmp( argument,"clearworships") )
  {
    gdef = gvd();
    if (gdef==-1)
    {
      stc("Error - Max Deities reached.\n\r",ch);
      return;
    }
    for( gvalue=0; gvalue < gdef; gvalue++)
      deity_table[gvalue].worship=0;
    stc("����� �������� ����� {R����� �������� ���������{x �������������� deity{x.\n\r", ch);
    REM_BIT( global_cfg, CFG_GTFIX);
    return;
  }

  if( !str_cmp( argument,"clearapplies") )
  {
    gdef = gvd();
    if (gdef==-1)
    {
      stc("Error - max Deities reached.\n\r",ch);
      return;
    }
    for( gvalue=0; gvalue < gdef; gvalue++)
      for( i=0; i < MAX_DEITY_APP; i++)
        deity_table[gvalue].d_apply[i] = 0;
    stc("����� �������� ����� {R����� �������� ���������{x �������� deity{x.\n\r", ch);
    REM_BIT( global_cfg, CFG_GTFIX);
    return;
  }
  stc("Type 'gfix help' for extra information.\n\r", ch);
};

void do_cfix( CHAR_DATA *ch, const char *argument)
{
  int dtemp;

  if( ch == NULL ) return;

//  if( IS_SET(ch->act, PLR_HIGHPRIEST) ) ch->pcdata->favour = 1000;

  if( ch->version > 10 || ch->version < 1 ) ch->version = 1;
  if( ch->version < 5) 
  {
    ch->long_descr = str_dup(ch->name);
    if( IS_IMMORTAL(ch) && ch->pcdata->pseudoname == NULL )
      do_function( ch, &do_pseudoname, "clean");
    ch->pcdata->protect = 0;
    if( IS_DEVOTED_ANY(ch) )
    {
      free_string(ch->deity);
      ch->deity = NULL;
      ch->pcdata->dn = 25;
      ch->pcdata->dc = 0;
      ch->pcdata->dr = 0;
      ch->pcdata->carma = 0;
      ch->pcdata->favour = 0;
    }
    ch->version = 5;
    save_char_obj(ch);
    return;
  }
  else if( ch->version >= 5 )
  {
    dtemp = gvd();
    if (dtemp==-1)
    {
      stc("Error - max Deities reached.\n\r",ch);
      return;
    }
    if( ch->pcdata->dn >= dtemp ) ch->deity = NULL;
    else ch->deity = str_dup(deity_table[ch->pcdata->dn].name);
    return;
  }
  save_char_obj(ch);
  if( IS_ELDER(ch) ) stc("Fixed.\n\r", ch);
}

void do_polyanarecall( CHAR_DATA *ch, const char *argument)
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];

  if( EMPTY(argument) || !str_cmp(argument,"help") )
  {
    stc("{c������� '������' � '�����'(� �����-���������� 'out') �� ������.\n\r", ch);
    stc("{cC��������:\n\r", ch);
    stc("{G   polyanarecall <parameter>\n\r", ch);
    stc("{GActual parameters: help, beer, pepsi, talk{x\n\r", ch);
    stc("{GBonus parameters : out\n\r", ch);
    stc("{G (polyanarecall pepsi out){x\n\r", ch);
    return;
  }

  argument = one_argument( argument, arg);

  if( ch->fighting != NULL || ch->pcdata->condition[COND_ADRENOLIN] > 0 )
  {
    stc( "��������, ���������, � ����� ������... :)\n\r", ch);
    return;
  }

  if (EMPTY(arg)) return;
  else if( !str_prefix(arg,"beer") )
    do_printf(buf, "{RP{Wo{RL{Wy{RA{Wn{RA{Wr{RE{Wc{RA{Wl{RL {B!{G!{B! {DPiFfO {BBeEr {mCeRvEsA {RBiEr {G!{Y!{W!{x");
  else if( !str_prefix(arg,"pepsi") )
    do_printf(buf, "{W!{R!{W! {YG{Go{YT{Go{YP{Go{YL{Gy{YA{Gn{YA {RP{BE{RP{BS{RI {D-d.r.i.n.k.i.n.g {W!{R!{W!{x");
  else if( !str_prefix(arg,"talk") )
    do_printf( buf, "{D�� ��������, �����, ������� � ������...{x");
  else do_printf( buf, "\0");

  if( EMPTY(buf) )
  {
    ptc( ch, "����������� �������� %s!\n\r", argument);
    stc( "������ 'polyanarecall help' ��� ������� :)\n\r", ch);
    return;
  }

  for( d = descriptor_list; d; d = d->next )
  {
    if ( !d->character || d->connected!=CON_PLAYING) continue;
    ptc( d->character, "{Y%s{x ��� ����i� ����i� ��������i�� ����������: %s\n\r",PERS(d->character,ch), buf);
  }

  if( !IS_ELDER(ch) ) WAIT_STATE( ch, 4);

  if( !str_cmp( argument,"out") )
  {
    save_char_obj( ch);
    do_function( ch, &do_quit, "");
  }
  return;
}

void do_repair( CHAR_DATA *ch, const char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *robj=NULL;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];
  int64 g_delta;

  argument=one_argument( argument, arg1 );
  argument=one_argument( argument, arg2 );

  if( EMPTY(arg1) || !str_prefix( arg1, "help" ) )
  {
    stc("{c������� ������� ��������� ���������� � �����.", ch);
    stc("{c������ ����� ������ ������� � �������-�������.\n\r", ch);
    stc("{c��� ������� ������� ������ ����� �������� �������.\n\r", ch);
    stc("{c������� ���� - ���������.\n\r", ch);
    stc("{c���������:\n\r", ch);
    stc("{G  repair [help]               - ��� �������;{x\n\r", ch);
    stc("{G  repair <object> [self]      - �������� ������� � ���� � ���������{x\n\r", ch);
    stc("{G  repair <object> <char>      - �������� ������� � ��������� � �������{x\n\r", ch);
    stc("{G  repair <object> <mob>       - �������� ������� � ����-������� � �������{x\n\r", ch);
    stc("{G  repair <object> <mob> value - ���������������� ���������� ������� ����.\n\r", ch);
    return;
  }
  if (!IS_SET(ch->in_room->room_flags,ROOM_RFORGE) && !IS_IMMORTAL(ch) )
  {
    stc("������� ����� � �������.\n\r", ch);
    return;
  }
  if( EMPTY(arg2) || !str_cmp(arg2,"self") ) victim=ch; 
  else victim=get_char_room(ch, arg2);
  if (!victim)
  {
    stc("��� ��� �����.\n\r", ch);
    return;
  }

  if( IS_NPC(victim) )
  {
    if( IS_SET(victim->act, ACT_FORGER) )
    {
      if ( !(robj = get_obj_carry( ch, arg1, ch)) )
      {
        stc( "� ���� ��� ���� ����.\n\r", ch );
        return;
      }
      if( !IS_ELDER(ch) ) switch ( robj->item_type )
      {
        case ITEM_WEAPON:
        case ITEM_ARMOR:
        case ITEM_WAND:
        case ITEM_STAFF:
          break;
        default:
          stc("������ ����� ������ ������, ���� � ������, ����� � ������.\n\r",ch);
          return;
      }
      if( robj->durability == 0 ) robj->durability = material_table[material_num(robj->material)].d_dam;
      if( robj->durability == -1 || (robj->condition)*100/((robj->durability < 1)?1:robj->durability) >= 100) 
      {
        ptc(ch,"��� ���� �� ��������� � �������...\n\r");
        return;
      }

      if( (g_delta = ( robj->durability - robj->condition) * robj->cost / ((robj->durability==-1)?1:(robj->durability)) * victim->level / 10000 ) > ch->gold )
      {
        ptc(ch,"� ���� �� ������ ����� �� ������� %s!\n\r", get_obj_desc(robj,'2') );
        return;
      }
      
      if( !str_prefix( argument, "value") && !EMPTY(argument) )
      {
        if( g_delta == 0) ptc( ch, "%s ����������: '{G� ������ ���� {C%s {G������!{x'\n\r", get_obj_desc( robj, '2'));
        else
        {
          do_printf( buf1, "{G������� {C%s {G����� ������ {Y%d{G ������� �����.{x",
             get_obj_desc( robj, '2'),
             g_delta );
          do_function( victim, &do_say, buf1);
        }
        return;
      }
      else
      {
        robj->condition = robj->durability;
        ptc(ch, "%s {G����� ��� ���� {C%s{x �� {Y%d {G������� �����.{x\n\r",
            get_char_desc( victim, '1'),
            get_obj_desc( robj, '4'),
            g_delta );                      
        ch->gold -= g_delta;
      }
      ptc(ch, "������ {C%s {x� %s ���������.\n\r",
          get_obj_desc( robj, '1'),
          get_obj_cond( robj, 1 ) );

      do_printf(buf, "{Y$c1{x ����������� {C%s{x ��� {Y$C2{x.",
          get_obj_desc(robj,'4') );
      act(buf, victim, NULL, ch, TO_NOTVICT);
      if (!IS_IMMORTAL(ch)) WAIT_STATE(ch,3*PULSE_VIOLENCE);
      return;
    }
    else
    { 
      stc("������� ����� �������!\n\r",ch);
      return; 
    }
  }
  else if( !IS_SET( race_table[ch->race].spec, SPEC_BLACKSMITH) && !IS_IMMORTAL(ch) )
  {
    stc( "�� �� �������� ��������� �����!\n\r", ch);
    return;
  }

  if( !(robj=get_obj_carry( victim, arg1, ch)) )
  {
    stc( "�� �� ����� �����.\n\r", ch );
    return;
  }

  if( !IS_ELDER(ch) ) switch ( robj->item_type )
  {
    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_WAND:
    case ITEM_STAFF:
      break;
    default:
      stc("������ ����� ������ ������, ���� � ������, ����� � ������.\n\r",ch);
      return;
  }
  if( robj->durability == 0 ) robj->durability = material_table[material_num(robj->material)].d_dam;
  if( robj->durability == -1 || (robj->condition)*100/((robj->durability < 1)?1:robj->durability) >= 100) 
  {
    ptc(ch,"��� ���� �� ��������� � �������...\n\r");
    return;
  }

  if( victim != ch && (victim->gold < (g_delta = (robj->durability - robj->condition) * robj->cost*victim->level / 80 / (robj->durability + 2))) )
  {
    stc( "� ������� �� ������ ����� �� ������ ����� ������...\n\r", ch );
    ptc( victim, "� ���� �� ������ ����� �� ������ ������ %s...\n\r", ch->name);
    return;
  }

/*
  if( !str_prefix( argument, "value") && !IS_NPC(victim) )
  {
    do_printf( buf1,"{G������� %s ����� ������ {Y%d{x ������� �����.{x\n\r", 
       get_obj_desc( robj,'2'),
       g_delta );
    do_function( victim, &do_say, buf1);
    return;
  }
*/

  if( victim != ch)
  {
    ptc(ch, "%s {G����� ��� ���� {C%s {G�� {Y%d {G������� �����.{x\n\r{G������ {C%s {G� %s {G���������.{x\n\r",
         get_char_desc( victim, '1'),
         get_obj_desc( robj, '4'),
         g_delta,
         get_obj_desc( robj, '1'),
         get_obj_cond( robj, 1 ) );  

    victim->gold -= g_delta;
    ch->gold += g_delta;
  }

  robj->condition = robj->durability; 
  do_printf(buf, "{Y$c1{x ����������� ��� {Y%s{x {y%s{x.",
    (ch==victim)?"����":get_char_desc(victim, 2), get_obj_desc(robj,'4'));
  act(buf, ch, NULL, victim, TO_NOTVICT);
  if( robj->condition > robj->durability) robj->condition = robj->durability;
  ptc( ch, "�� ������ {C%s{x%s%s.\n\r������ ��� ���� � %s ���������.\n\r",
      get_obj_desc(robj, '4'), (ch==victim)?"":" ��� ",
      (ch==victim)?"":get_char_desc(victim, '2'),
      get_obj_cond( robj, 1) );
  if (!IS_IMMORTAL(ch)) WAIT_STATE(ch,2*PULSE_VIOLENCE);
}

void do_reward( CHAR_DATA *ch, const char *argument )
{
 CHAR_DATA *victim;
 char buf[MAX_INPUT_LENGTH];
 char arg1[MAX_STRING_LENGTH];
 char arg2[MAX_STRING_LENGTH];
 char arg3[MAX_STRING_LENGTH];
 int value;

 argument=one_argument( argument, arg1);
 argument=one_argument( argument, arg2);
 argument=one_argument( argument, arg3);

 if( EMPTY(arg1) || !str_prefix(arg1,"help") )
 {
   stc("{c������������ ������� �����.\n\r",ch);
   stc("{c���������:{x\n\r",ch);
   stc("{Greward <char_name> <reward_type> <reward_value>\n\r",ch);
   stc("{G  ���� reward_type(����������� ���������): qp, gold, silver\n\r",ch);
   stc("{G  ���� reward_value �������� �������� ��������� �������.\n\r",ch);
   return;
 }

 if( !(victim=get_pchar_world(ch,arg1)) )
 {
   stc("����� � ���� ���.\n\r",ch);
   return;
 }

   if(EMPTY(arg3))
   {
     stc("����������� �������� ������� �������.\n\r",ch);
     return;
   }

 value=atoi(arg3);

 if( !str_cmp( arg2,"qp") )
 {
   if( value > 5000 || value < 1 )
   {
     stc("�������� ������� � ��������� ����� ����� ���� �� 1 �� 5000.\n\r",ch);
     return;
   }
   victim->questpoints += value;
   if ( ch != victim && !IS_NPC(ch) )
   {
     do_printf( buf,"{Y%s{x �������%s �� {c%s{x ������� � ������� {G%d{x ��������� �����.\n\r",victim->name,victim->sex==1?"":"�",ch->name,value);
     if( value) ptc( ch, "{Y%s {x�������� �� ���� ������� � ������� {R%d {c%s{x! (� ����� ������: {G%d{x).\n\r", get_char_desc( victim, '1'), value, arg2, victim->questpoints);
     ptc( victim, "�� ��������� � ������� �� {C%s {G%d{x ��������� �����!\n\r", ch->name, value);
     send_news( buf, NEWS_REWARD);
   }
   return;
 }
 if( !str_cmp( arg2, "gold") )
 {
   if( value > 10000 || value < 1 )
   {
     stc("�������� ������� � ������� ������� ����� ���� �� 1 �� 10000.\n\r",ch);
     return;
   }
   victim->gold += value;
   if( victim != ch && !IS_NPC(ch) ) 
   {
     do_printf( buf,"{Y%s{x �������%s �� {c%s{x ������� � ������� {Y%d{x �������.",victim->name,victim->sex==1?"":"�",ch->name,value);
     if( victim != NULL && value) ptc( ch, "{Y%s {x�������� �� ���� ������� � ������� {R%d {c%s{x! (� ����� ������: {w%d{x).\n\r", get_char_desc( victim, '1'), value, arg2, victim->gold);
     ptc( victim, "�� ��������� � ������� �� {C%s {Y%d{x ������� �����!\n\r", ch->name, value);
     send_news( buf, NEWS_REWARD);
   }
   return;
 }
 if( !str_cmp( arg2, "silver") )
 {
   if( value > 50000 || value < 1 )
   {
     stc("�������� ������� � ���������� ������� ����� ���� �� 1 �� 50000.\n\r",ch);
     return;
   }
   victim->silver += value;
   if ( ch != victim && !IS_NPC(ch) )
   {
     do_printf( buf,"{Y%s{x �������%s �� {c%s{x ������� � ������� {W%d{x ���������� �����!",victim->name,victim->sex==1?"":"�",ch->name,value);
     if( value) ptc( ch, "{Y%s {x�������� �� ���� ������� � ������� {R%d {c%s{x! (� ����� ������: {G%d{x).\n\r", get_char_desc( victim, '1'), value, arg2, victim->silver);
     ptc( victim, "�� ��������� � ������� �� {C%s {W%d{x ���������� �����! (� ����� ������: {G%d{x).\n\r", ch->name, value, victim->silver);
     send_news( buf, NEWS_REWARD);
   }
 }

}

void do_damage( CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int value;

  argument=one_argument( argument, arg1);
  argument=one_argument( argument, arg2);

  if(EMPTY(arg1) || !str_cmp( arg1,"help") )
  {
    stc("{c�������� ��������� ����.{x\n\r", ch);
    stc("{c����� ���� ���������� � ����, ���� �������� [character] ����,{x\n\r", ch);
    stc("{c� � ������� ��������� (����, ����), ���� �� ����.{x\n\r", ch);
    stc("{c���������:\n\r", ch);
    stc("{G  damage <object_name> <damage_value> [character]{x\n\r", ch);
    return;
  }
  if ( get_trust(ch) < 109) return;

  if ( EMPTY(argument) ) victim=ch;
  else victim = get_char_world( ch, argument);

  if(!victim)
  {
    stc("����� ��� � ����.\n\r",ch);
    return;
  }

  if( (obj=get_obj_wear( victim, arg1)) != NULL );
  else if( !(obj=get_obj_victim( ch, victim, arg1)) )
  {
    stc("������� �� ������!\n\r", ch);
    return;
  }

  if( !(value=atoi(arg2)) ) value=1;

  if( obj->durability == -1 ) 
  {
    stc("�� �� ������ ��������� ��� ����.\n\r", ch);
    return;
  }

  if( obj->condition < 1) 
  {
    stc("��� ���� � ��� �������.\n\r", ch);
    return;
  }
  if( obj->condition < value ) value = obj->condition;
  obj->condition -= value;
  ptc(ch, "�� �������� {c%s{x � {Y%s{x �� {R%d{x ������. �������� {R%d{x.\n\r",
  get_obj_desc( obj,'4'), (victim==ch)?"����":get_char_desc(victim,'2'), value, 
  obj->condition );
}

void do_seize( CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1);
  argument = one_argument( argument, arg2);

  if(EMPTY(arg1) || !str_cmp( arg1,"help") )
  {
    stc("{c������� �������� ������� � ���������.{x\n\r", ch);
    stc("{c������ ������������ ��� ����������� ��������.{x\n\r", ch);
    stc("{c���������:\n\r", ch);
    stc("{G  seize <obj_name> <char_name> [quiet]{x\n\r", ch);
    stc("{G     <obj_name>  - �������� ����;{x\n\r", ch);
    stc("{G     <char_name> - ��� ������;{x\n\r", ch);
    if( IS_ELDER(ch) )
    {
      stc("{cElders' ���������:\n\r", ch);
      stc("{G     [quiet]     - �����-��������(��� ��������� ������){x\n\r", ch);
    }
    return;
  }

  if( ( victim = get_char_room( ch, arg2)) == NULL ) 
  {
    stc("����� ��������� ��� � �������!\n\r", ch);
    return;
  }

  if( (obj=get_obj_wear( victim, arg1)) != NULL );
  else if( !(obj=get_obj_victim( ch, victim, arg1)) )
  {
    stc("������� � ����� ������ � ��������� �� ������!\n\r", ch);
    return;
  }
  
  if( get_trust(ch) < get_trust(victim) )
  {
     stc("������� ���������...\n\r", ch);
     ptc( victim,"{R%s{x ������� ������������ � ���� {R%s{x!\n\r",
          ch->name, get_obj_desc( obj, '4') );
     return;
  }
  if( victim == ch )
  {
    stc("����� ����� ������ �����?\n\r", ch);
    return;
  }

  obj_from_char(obj);
  obj_to_char( obj, ch);
  ptc( ch, "�� ��������������� {c%s{x � {Y%s{x.\n\r", 
       get_obj_desc( obj, '4'), get_char_desc( victim, '2') );

  if( !str_cmp(argument,"quiet") && IS_ELDER(ch) && !IS_ELDER(victim) ) return;
  else
  {
    ptc( victim, "{Y%s{x �������������� � ���� {c%s{x.\n\r", 
         get_char_desc( ch, '1'), get_obj_desc( obj, '4') );
  }
};

#define dtab deity_table[deity]
void do_devote( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_STRING_LENGTH];
  int deity = 0;

   if( EMPTY(argument) || !str_cmp( argument, "help") )
   {
     stc("{c��������� ���� ����� � ������ ��������.\n\r", ch);
     stc("{c����������� ������ �� ���� ����������� �������� ����� ���� ������ ����������.\n\r", ch);
     stc("{c���������:\n\r", ch);
     stc("{G   devote, devote help    - ��� �������\n\r", ch);
     stc("{G   devote list            - ������ �����\n\r", ch);
     stc("{G   devote show            - ���� ������ ����������\n\r", ch);
     stc("{G   devote stat <deity>    - ������� ������� � ��������\n\r", ch);
     if( !IS_DEITY(ch) )
     stc("{G   devote <deity>         - ��������� ���� ��������\n\r", ch);
//     stc("{G   devote disavow         - ����� � ���� ������������\n\r", ch);
     if( IS_DEITY(ch) )
     {
       stc("{G   devote <char> stat     - ���������� ���������� ������������\n\r{x", ch);
       stc("{G   devote <char> clear    - ������� ��������� ��������\n\r{x", ch);
       stc("{G   devote <char> <deity>  - ��������� ��������� ��������\n\r{x", ch);
     }
     if( IS_CREATOR(ch) )
     {
       stc("{G   devote <char> excuse    - �������� ����� ��������� (disavow)\n\r{x", ch);
       stc("{G   devote <char> pardon    - �������� ����� ��������� (carma,favour)\n\r{x", ch);
       stc("{G   devote <char> reward    - �������� �� ���������� ������� (+favour)\n\r{x", ch);
       stc("{G   devote <char> punish    - �������� �� ������������ ������� (-favour)\n\r{x", ch);
     }
     return;
   }

   if ( !str_prefix( argument, "list") )
   {
     stc("{D ��� �������� {G������� ���    {c��������\n\r", ch);
     for( deity = 0; dtab.name && deity<MAX_DEITIES; deity++)
     { 
       ptc( ch, "{C %12s [{Y%12s{C] %s\n\r", dtab.name, get_rdeity( dtab.russian, '1' ), dtab.descr);
     }
     return;
   }

   if( ch->deity) deity = ch->pcdata->dn;

   if( !str_prefix( argument, "show") && !IS_NPC(ch) )
   {
     if( IS_DEITY(ch) )
     {
       ptc( ch, "�� ���%s ��������� ���������.\n\r", (ch->sex==2)?"�":"");
       return;
     }
     else ptc( ch, "�� %s��������%s %s.\n\r", (ch->deity)?"":"�� ", (ch->sex==2)?"�":"",
             (ch->deity)?get_rdeity(dtab.russian,'3'):"��������");
     if( IS_DEVOTED_ANY(ch) ) ptc( ch, "�� ��������� {y%s{W {R%s{x.\n\r",
        ( ch->sex==2)?t_favour[favour_string(ch)].fav_afstr:t_favour[favour_string(ch)].fav_amstr,
        get_rdeity( dtab.russian, '2') );
     return;
   }
/*
   if( !str_cmp( argument, "disavow") && !IS_NPC(ch) )
   {
     if( ch->deity )
     {
       deity = deity_lookup(ch->deity);
       dec_worship(ch);
       if( get_trust(ch) < 102 )
       {
         ptc( ch, "�� ����������� �� ���������� %s...\n\r", get_rdeity(dtab.russian,'3') );
         stc("{R�� ������ ��� ���, ��������...{x\n\r", ch);
         WAIT_STATE( ch, 2*PULSE_VIOLENCE);
         ch->pcdata->carma -= 10;
         ch->pcdata->favour = -50;
         punish_effect( ch);
       }
       else ptc( ch, "�� ����������� �� ��������������� %s...\n\r", get_rdeity(dtab.russian,'2') );
       ch->pcdata->dn = 49;
       free_string(ch->deity);
       ch->deity = NULL;
       save_deities();
       return;
     }
     stc("� ���� ��� �����������!\n\r", ch);
     return;
   }
*/
   argument = one_argument( argument, arg);

   if( !str_cmp(arg,"stat"))
   {
    deity = deity_lookup(argument);
     if( !EMPTY(argument) )
       if( !str_cmp( argument, dtab.name) )
       {
         ptc( ch, "{c%s{x, %s{x.\n\r������������ ��������: �� {D%d {x�� {W%d{x.\n\r", get_rdeity( dtab.russian, '1'),
           dtab.descr, dtab.min_align, dtab.max_align );
         show_deity_applies( ch, deity);
         ptc( ch,"{C%s %s�������{x ������� ���� ����������.{x\n\r",
             get_rdeity( dtab.russian,'1'),
             may_devote(ch,deity)?"{G":"{R�� " );
         return;
       }
     stc("������ �������� �� ����������.\n\r", ch);
     return;
   }

   if( ch->deity && !IS_DEITY(ch) )
   {
     ptc( ch, "�� ��������� ������������%s %s... ������� �������� �� ������ ��������...\n\r", (ch->sex==2)?"������":"��", get_rdeity(deity_table[deity_lookup(ch->deity)].russian,'2'));
     return;
   }

    if( !EMPTY(arg) && !IS_DEITY(ch) && !str_cmp( arg, deity_table[deity_lookup(arg)].name) )
    {
      deity = deity_lookup(arg);
      if( !may_devote( ch, deity) )
      {
        ptc( ch, "�� �� �������������� ����������� %s.\n\r", get_rdeity(dtab.russian,'2') );
        return;
      }

      if( IS_SET(ch->act,PLR_DISAVOWED) )
      {
        stc("�� ������� ������...\n\r", ch);
        return;
      }

      free_string(ch->deity);
      arg[0] = UPPER(arg[0]);
      ch->deity = str_dup(arg);
      ch->pcdata->dn = deity_lookup(arg);
      deity_table[deity_lookup(arg)].worship++;
      if( !IS_NPC(ch) )
      {
        ch->pcdata->carma += 5;
        ch->pcdata->favour += 50;
        save_deities();
      }
      ptc(ch, "�� ��������%s ���� %s.\n\r", (ch->sex==2)?"�":"", get_rdeity(deity_table[deity_lookup(ch->deity)].russian,'3') );
      return;
    }

    if( EMPTY(argument) || EMPTY(arg) ) 
    {
       stc("{c���������: {Gdevote help {c��� ����������.{x\n\r", ch);
       return;
    }

   if( IS_DEITY(ch) )
   {
     CHAR_DATA *victim;

     if( (victim = get_pchar_world( ch, arg)) == NULL )
     {
       stc("� ���� ��� �����.\n\r", ch);
       return;
     }

     if( victim->deity) deity = victim->pcdata->dn;

     if( !str_cmp( argument, "clear") )
     {
       if( victim->deity )
       {
         dec_worship(victim);
         ptc( ch, "%s ������ ������%s.\n\r", victim->name, (victim->sex==2)?"��":"" );
         ptc( victim, "�� ������ {R������%s{x!\n\r", (victim->sex==2)?"��":"" );
         victim->pcdata->dn = 25;
         free_string(victim->deity);
         victim->deity = NULL;
         if( !IS_NPC(victim) )
         {
           victim->pcdata->favour = 0;
           save_deities();
         }
         return;
       }
       ptc( ch, "%s �� ����� �����������...\n\r", victim->name );
       return;
     }

     if( !str_cmp( argument, "highpriest") && IS_ELDER(ch) )
     {
       if( !IS_NPC(victim) && victim->deity )
       {
         if( IS_SET( victim->act, PLR_HIGHPRIEST ) ) REM_BIT( victim->act, PLR_HIGHPRIEST );
         else
         {
           SET_BIT( victim->act, PLR_HIGHPRIEST );
           ptc( ch, "%s ������ {W%s {R%s{x!\n\r", victim->name, (victim->sex==2)?t_favour[favour_string(victim)].fav_nfstr:t_favour[favour_string(victim)].fav_nmstr, get_rdeity( deity_table[victim->pcdata->dn].russian,'2'));
           ptc( victim, "�� ������ {W%s {R%s{x!\n\r", (victim->sex==2)?t_favour[favour_string(victim)].fav_nfstr:t_favour[favour_string(victim)].fav_nmstr, get_rdeity( deity_table[victim->pcdata->dn].russian,'2'));
         }
         return;
       }
       ptc( ch, "%s �� ����� �����������...\n\r", victim->name );
       return;
     }

     if( !str_cmp( argument, "increment") && IS_ELDER(ch) )
     {
       if( str_cmp( arg, deity_table[deity_lookup(arg)].name) )
       {
         stc("������� ������������ ��������� deity!\n\r", ch);
         return;
       }
       deity_table[deity_lookup(arg)].worship +=1;
       ptc( ch, "������ � %s %d �����������.\n\r", 
            get_rdeity( deity_table[deity_lookup(arg)].russian, '2'),
            deity_table[deity_lookup(arg)].worship );
       return;
     }

     if( !str_cmp( argument, "pardon") && !IS_NPC(victim) && IS_ELDER(ch) )
     {
      if( victim->pcdata->carma < 0) victim->pcdata->carma = 0;
      if( victim->pcdata->favour < 0) victim->pcdata->favour = 0;
      ptc( ch, "�� ���������� ����� %s.\n\r", victim->name);
      ptc( victim, "%s ��������� ���� �����.\n\r", PERS(ch,victim));
      return;
     }

     if( !str_cmp( argument, "excuse") && !IS_NPC(victim) && IS_ELDER(ch) )
     {
       if( IS_SET( victim->act, PLR_DISAVOWED) ) REM_BIT( victim->act, PLR_DISAVOWED);
       else ptc( ch, "%s �� ������� ������.\n\r", victim->name);
       if( victim->godcurse >0 ) victim->godcurse = 0;
       else ptc( ch, "%s �� ������� ������.\n\r", victim->name);
       ptc( ch, "�� �������� ��������� %s.\n\r", victim->name);
       if( can_see( victim, ch, CHECK_LVL) ) ptc( victim, "%s ������� ���� ���������.\n\r", PERS(ch,victim));
       return;
     }

     if( !str_prefix( argument, "reward") && IS_CREATOR(ch) )
     {
       if( !IS_NPC(victim) ) change_favour( victim, 50);
       else stc("������ ��� ������� ����������!{x\n\r", ch);
       return;
     }

     if( !str_prefix( argument, "punish") && IS_CREATOR(ch) )
     {
       if( !IS_NPC(victim) ) change_favour( victim, -50);
       else stc("������ ��� ������� ����������!{x\n\r", ch);
       return;
     }

     if( !str_cmp( argument, "stat") && !IS_NPC(victim) )
     {
        if( victim->deity != NULL )
        {
          stc( "{c+-------------------------------------------------------------+\n\r", ch);
          ptc( ch, "{c| {G���������� Deity ��� {Y%s{G, %s\n\r", victim->name, IS_NPC(victim)?"none":victim->pcdata->title);
          stc( "{c+-------------------------------------------------------------+\n\r", ch);
          ptc( ch, "{c| {G��������{W: {R%12s{x {G���������� %s{W: {Y%5d {G�����{W: {w%d\n\r",
             get_rdeity(dtab.russian, '3'), get_rdeity( dtab.russian,'2'),
             IS_NPC(ch)?0:victim->pcdata->favour, IS_NPC(ch)?0:victim->pcdata->carma );
          ptc( ch, "{c| {G�������� Deity{W: {x%s{G\n\r", dtab.descr);
          ptc( ch, "{c| {G���������� �������������� � %s{W: {m%d\n\r",
             get_rdeity( dtab.russian,'2'), dtab.worship );
          ptc( ch, "{c| {G����������� ��������{W: {D%5d{c/{W%5d{G, �������� ���������{W: {R%5d{x\n\r",
             dtab.min_align, dtab.max_align, victim->alignment );
          ptc( ch, "{c| {YDeityNumber{w:{R%d {YDeityRank{w: {R%d {DDeityCurseCounter{w: {R%d{x\n\r",
             victim->pcdata->dn, victim->pcdata->dr, victim->pcdata->dc );
          stc( "{c+-------------------------------------------------------------+\n\r", ch);
          return;
        }
        stc( "{c+-------------------------------------------------------------+\n\r", ch);
        ptc( ch, "{c| {G���������� Deity ��� {Y%s{G, %s\n\r", victim->name, victim->pcdata->title);
        stc( "{c+-------------------------------------------------------------+\n\r", ch);
        ptc( ch, "{c| {Y%s {G�� ����� �����������.{x\n\r", victim->name );
        ptc( ch, "{c| {YCarma{c: {W%6d {YFavour{c: {m%6d {wGodCurse{c: {D%2d {YAlign{c: {y%5d{x\n\r",
             victim->pcdata->carma, victim->pcdata->favour, victim->godcurse, victim->alignment);
        ptc( ch, "{c| {YDeityNumber{w:{R%2d {YDeityRank{w: {R%d {DDeityCurseCounter{w: {R%3d {x\n\r",
             victim->pcdata->dn, victim->pcdata->dr, victim->pcdata->dc );
        stc( "{c+-------------------------------------------------------------+\n\r", ch);
        return;
     }

     deity = deity_lookup(argument);

     if( !str_cmp( argument, dtab.name) )
     {
       if( !IS_NPC(ch) && IS_SET(victim->act,PLR_DISAVOWED) )
       {
         ptc( ch, "%s ������� ������...\n\r", victim->name);
         return;
       }
       if( !may_devote( victim, deity_lookup(argument) ) && !IS_ELDER(ch) )
       { 
          stc("No match.\n\r", ch); 
          return; 
       }
       else
       {
         if( !IS_NPC(victim) ) dec_worship(victim);
         free_string(victim->deity);
       }
       victim->deity = str_dup(argument);
       victim->pcdata->dn = deity_lookup(argument);
       if( !IS_NPC(victim) ) deity_table[victim->pcdata->dn].worship++;
       ptc( ch, "�� ���������� %s %s.\n\r", victim->name, get_rdeity(dtab.russian,'3') );
       ptc( victim, "������ ���� ����� ��������� %s.\n\r", get_rdeity(dtab.russian,'3') );
       if( !IS_NPC(victim) )
       {
         victim->pcdata->favour += 5;
         victim->pcdata->carma +=5;
         save_deities();
       }
       return;
     }
    stc("{C������{x �� ������ ����� ������...{x\n\r", ch);
    stc("������ '{Gdevote help{x'��� ����������.{x\n\r", ch);
  }
}
#undef dtab

#define dtab deity_table[deity]
void do_deity ( CHAR_DATA *ch, const char *argument)
{
   char   buf[MAX_INPUT_LENGTH],  arg[MAX_STRING_LENGTH];
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
   char arg3[MAX_STRING_LENGTH];
   int  deity, value, i;

   if( EMPTY(argument) || !str_prefix( argument, "help") )
   {
     stc("{c������� �������������� ������� ��� ����������.{x\n\r", ch);
     stc("{c��������� � ������� ������� �������� ���������!{x\n\r", ch);
     stc("{R�������� {c������������ {R������{c ��� �������� ���������� ��������!{x\n\r", ch);
     stc("{c��������� {R������������� �� ����������� {c� �����!{x\n\r", ch);
     stc("{c���������:{x\n\r", ch);
     stc("{G  deity [help]           - ��� �������{x\n\r", ch);
     stc("{G  deity <stat>           - ����������� ���������� deity.{x\n\r", ch);
     stc("{G  deity <stat> [deity]   - ����������� ���������� � ���������� ��������.{x\n\r", ch);
     stc("{G  deity <save>           - ��������� ������� DEITY_FILE{x\n\r", ch);
     stc("{G  deity <load>           - ��������� DEITY_FILE{x\n\r", ch);
     stc("{G  deity <deity> create   - ������� ����� ��������{x\n\r", ch);
     stc("{G  deity <deity> delete   - ������� ��������{x\n\r", ch);
     stc("{G  deity <deity> <name>     <new_name>    - ������� ��� ��������{x\n\r", ch);
     stc("{G  deity <deity> <russian>  <new_russian> - ������� ������� ��� ��������{x\n\r", ch);
     stc("{G  deity <deity> <descr>    <new_descr>   - ������� �������� ��������{x\n\r", ch);
     stc("{G  deity <deity> <alignmin> <value> - ��������� ����������� ��������� ��������{x\n\r", ch);
     stc("{G  deity <deity> <alignmax> <value> - ��������� ������������ ���������� ��������{x\n\r", ch);
     stc("{G  deity <deity> apply   <what> <value> - ����������� ������� ��������{x\n\r", ch);
     stc("{G  deity <deity> penalty <what> <value> - ����������� �������� ��������{x\n\r", ch);
     return;
   }

   if( !str_cmp( argument, "save") )
   {
     stc( "{RSaving deity data file.{x\n\r",ch);
     save_deities();
     stc( "{RDone.{x\n\r", ch);
     return;
   }

   if( !str_cmp(  argument, "load") )
   {
     stc( "{RLoading new deity list...\n\r", ch);
     load_deities();
     stc( "{RDone.{x\n\r", ch);
     return;
   }

   argument = one_argument( argument, arg);

   if( !str_cmp( arg, "stat") )
   {
     if( EMPTY(argument) )
     {
       stc("{w ___   ____   ____________   ____________   ___________{x\n\r", ch);
       stc("{w[{g#No{w] [{rWsh {w] [{CName        {w] [{yRussianName {w] [{DDescription{w]{x\n\r", ch);
       stc("{w >-<   >--<   >----------<   >----------<   >---------<{x\n\r", ch);
       for( deity = 0; dtab.name && deity<MAX_DEITIES; deity++)
         ptc( ch, "{w[{G%3d{w] [{R%4d{w] [{c%12s{w] [{Y%12s{w] [{x%s{x\n\r", deity, dtab.worship, 
             dtab.name, get_rdeity( dtab.russian,'1'), dtab.descr);
       return;
     }
     else 
       if( !str_cmp( argument, deity_table[deity_lookup(argument)].name ) )
       {
         stc( "{w[{DNum{w] [{DName        {w] [{DR_Name      {w] [{DWorship{w] [{DMinAlign{w] [{DMaxAlign{w]{x\n\r", ch);
         ptc( ch, "{w[{w%3d{w] [{y%12s{w] [{Y%12s{w] [  {R%5d{w] [  {G%5d {w] [  {R%5d {w]\n\r{r >{w%s\n\r",
              deity_lookup(argument),
              deity_table[deity_lookup(argument)].name,
              get_rdeity(deity_table[deity_lookup(argument)].russian,'1'),
              deity_table[deity_lookup(argument)].worship,
              deity_table[deity_lookup(argument)].min_align,
              deity_table[deity_lookup(argument)].max_align,
              deity_table[deity_lookup(argument)].descr);
         show_deity_applies( ch, deity_lookup(argument) );
         return;
       }
     stc("������ �������� �� ����������!{x\n\r", ch);
     return;
   }

   if( !str_cmp( arg, "astat") && IS_ELDER(ch) )
   {
     stc("{D+--+{y---------+--+-----+-----+-----------+-----------+-----------+-----------+-----------+{x\n\r{x", ch);
     stc("{D|{wN#{D|{BName     {y|{MWs{y|{galMin{y|{ralMax{y| {G+{cParams{R-  {y| {G+{cInform{R-  {y| {G+{cResists{R- {y| {G+{cSkills{R-  {y| {G+{cBonus{R-   {y|{x\n\r", ch);
     stc("{D+--+{y---------+--+-----+-----+-----------+-----------+-----------+-----------+-----------+{x\n\r{x", ch);
     for( deity = 0; dtab.name && deity<MAX_DEITIES ; deity++)
     {
       ptc( ch, "{D|{W%2d{D|{Y%9s{y|{c%2d{y|{G%5d{y|{m%5d{y|", deity, dtab.name, dtab.worship,
            dtab.min_align, dtab.max_align);
       for( i=0; i < MAX_DEITY_APP; i++)
       {
         switch( dtab.d_apply[i])
         {
          case 0:  stc("{w", ch); break;
          case 1:  stc("{G", ch); break;
          case 2:  stc("{Y", ch); break;
          case 3:  stc("{c", ch); break;
          case 4:  stc("{m", ch); break;
          case 5:  stc("{R", ch); break;
          default: stc("{r", ch); break;
         }
         ptc( ch, "%2d", dtab.d_apply[i] );
         if( (i+1)%4 == 0 ) stc("{y|", ch);
         else stc(" ", ch);
       }
       stc("{x\n\r", ch);
     }
     stc("{D+--+{y---------+--+-----+-----+-----------+-----------+-----------+-----------+-----------+{x\n\r{x", ch);
     return;
   }

   if( !str_cmp( argument, "create") && !EMPTY(arg) )
   {
     if( !str_cmp( arg, deity_table[deity_lookup(arg)].name) )
     {
       ptc( ch, "�������� %s ��� ����������!\n\r", arg );
       return;
     }
     deity = gvd();
     if (deity==-1)
     {
       ptc(ch,"���������� ������������ ���������� �������. (%d)\n\r",MAX_DEITIES);
       return;
     }
     arg[0]=UPPER(arg[0]);
     dtab.name = str_dup(arg);
     dtab.russian = "��������";
     dtab.min_align = -1000;
     dtab.max_align = 1000;
     dtab.descr = "�������� ������ ��������";
     dtab.worship = 0;
     for( i=0; i < MAX_DEITY_APP; i++)
       dtab.d_apply[i] = 0;
     ptc( ch, "����� deity {y%s{x ������� �������!\n\r", dtab.name);
     return;
   }

   if(  !str_cmp( arg, deity_table[deity_lookup(arg)].name ) && !EMPTY(argument) )
   {
     if( !str_cmp( argument, "delete") && !str_cmp(arg, deity_table[deity_lookup(arg)].name) )
     {
       deity = deity_lookup(arg); 

       ptc( ch, "������� �������� {y%s{x...", deity_table[deity_lookup(arg)].name);
       for( ; deity < MAX_DEITIES; deity++)
       {
         if (!deity_table[deity+1].name) break;
         dtab.name=deity_table[deity+1].name;
         dtab.russian   = deity_table[deity+1].russian;
         dtab.descr     = deity_table[deity+1].descr;
         dtab.min_align = deity_table[deity+1].min_align;
         dtab.max_align = deity_table[deity+1].max_align;
         dtab.worship   = deity_table[deity+1].worship;
         for( i=0; i < MAX_DEITY_APP; i++)
           dtab.d_apply[i] = deity_table[deity+1].d_apply[i];
         if( deity < 0 ) return;
       }
       free_string(dtab.russian);
       free_string(dtab.name);
       free_string(dtab.descr);
       dtab.name = NULL;
       dtab.min_align = 0;
       dtab.max_align = 0;
       dtab.worship = 0;
       for( i=0; i < MAX_DEITY_APP; i++) dtab.d_apply[i] = 0;
       stc (" ������\n\r",ch);
       return;
     }

     argument = one_argument( argument, arg1);

     if ( !str_prefix( arg1, "name") && !EMPTY(argument) )
     {
         ptc(ch,"��� �������� {W%s{x �������� �� '{Y%s{x'.\n\r",
               deity_table[deity_lookup(arg)].name,
               str_dup( argument) );
         do_printf( buf, "%s", argument);
         buf[0]=UPPER(buf[0]);
         free_string( deity_table[deity_lookup(arg)].name);
         deity_table[deity_lookup(arg)].name = str_dup(buf);
         return;
     }

     if ( !str_prefix( arg1, "russian") && !EMPTY(argument) )
     {
         ptc(ch,"������� ��� {W%s{x �������� �� '{Y%s{x'.\n\r",
              get_rdeity( deity_table[deity_lookup(arg)].russian,'2'),
              str_dup( argument) );
         do_printf( buf, "%s", argument);
         buf[0]=UPPER(buf[0]);
         free_string( deity_table[deity_lookup(arg)].russian);
         deity_table[deity_lookup(arg)].russian = (char *)str_dup(buf);
         return;
     }

     if ( !str_prefix( arg1, "descr") && !EMPTY(argument) )
     {
         ptc(ch,"�������� {W%s{x �������� �� '{Y%s{x'.\n\r",
             get_rdeity( deity_table[deity_lookup(arg)].russian, '2'), 
             str_dup( argument) );
         do_printf( buf, "%s", argument);
         buf[0]=UPPER(buf[0]);
         free_string( deity_table[deity_lookup(arg)].descr);
         deity_table[deity_lookup(arg)].descr = str_dup(buf);
         return;
     }

     argument = one_argument( argument, arg2);

     if ( !str_cmp( arg1, "alignmin"))
     {
         if( !(value = atoi(arg2)) ) value = 0;
         if( value < -1000) value = -1000;
         if( value > 1000 ) value = 1000;
 
         if( value >= deity_table[deity_lookup(arg)].max_align )
         {
           stc( "����������� �������� �� ����� ���� ������ �������������!\n\r", ch);
           return;
         }
         ptc(ch,"{GMinalign {W%s{x �������� � '{y%d{x' �� '{Y%d{x'.\n\r",
             get_rdeity( deity_table[deity_lookup(arg)].russian, '2'),
             deity_table[deity_lookup(arg)].min_align, value);
         deity_table[deity_lookup(arg)].min_align = value;
         return;
     }

     if ( !str_cmp( arg1, "alignmax"))
     {
         if( !(value = atoi(arg2)) ) value = 0;
         if( value < -1000) value = -1000;
         if( value > 1000 ) value = 1000;
 
         if( value <= deity_table[deity_lookup(arg)].min_align )
         {
           stc( "������������ �������� �� ����� ���� ������ ������������!\n\r", ch);
           return;
         }
         ptc(ch,"{GMaxalign {W%s{x �������� � '{y%d{x' �� '{Y%d{x'.\n\r",
             get_rdeity( deity_table[deity_lookup(arg)].russian, '2'), 
             deity_table[deity_lookup(arg)].max_align, value);
         deity_table[deity_lookup(arg)].max_align = value;
         return;
     }

     argument = one_argument( argument, arg3);

     if( !str_cmp( arg1, "apply") || !str_cmp( arg1, "penalty") )
     {
         int iarg3;
         bool isapply=TRUE;

         deity = deity_lookup(arg);
         if( EMPTY(arg2) )
         {
           stc("{c����������� �������� ������������ ������� � ��������:{x\n\r", ch);
           stc("{c���������:{x\n\r", ch);
           stc("{G  str, int, wis, dex, con{x\n\r", ch);
           stc("{c����������������\\����������:{x\n\r", ch);
           stc("{G  rslash,  rpierce, rbash,     rfire, rpoison,{x\n\r", ch);
           stc("{G  rmental, rlight,  rnegative, racid, rholy{x\n\r", ch);
           stc("{c������\\��������:{x\n\r", ch);
           stc("{G  water,      air,      earth,   fire,       spirit{x\n\r", ch);
           stc("{G  mind,       light,    dark,    fortitude,  curative{x\n\r", ch);
           stc("{G  perception, learning, offence, protection, makemastery{x\n\r", ch);
           return;
         }

         if( str_cmp( arg2, "spell") && str_cmp( arg2, "skill") )
         {
           if( EMPTY(arg3) || (iarg3=atoi(arg3)) < 0 ) iarg3=0;
           if( iarg3 < 0 || iarg3 > 10 ) iarg3 = 0; 
           if( is_exact_name( arg2,"water rslash str" ) ) value = 1;
           else if( is_exact_name( arg2,"air rpierce int" ) ) value = 2;
           else if( is_exact_name( arg2,"earth rbash wis" ) ) value = 3;
           else if( is_exact_name( arg2,"fire rfire dex" ) ) value = 4;
           else if( is_exact_name( arg2,"spirit rpoison con" ) ) value = 5;
           else if( is_exact_name( arg2,"mind rmental" ) ) value = 6;
           else if( is_exact_name( arg2,"light rlight" ) ) value = 7;
           else if( is_exact_name( arg2,"dark rnegative" ) ) value = 8;
           else if( is_exact_name( arg2,"fortitude racid" ) ) value = 9;
           else if( is_exact_name( arg2,"curative rholy" ) ) value = 10;
           else if( !str_cmp( arg2,"perception") ) value=11; 
           else if( !str_cmp( arg2,"learning" ) ) value = 12;
           else if( !str_cmp( arg2,"offence") ) value = 13;
           else if( !str_cmp( arg2,"protection" ) ) value = 14;
           else if( !str_cmp( arg2,"makemastery" ) ) value = 15;
           else { stc("������ ������ �� ����������!\n\r", ch); return; }

           i = 0;
           if( !str_cmp( arg1, "penalty" ) )
           {
             i += 2;
             isapply = FALSE;
           }
           if( is_exact_name(arg2,"str int wis dex con") 
            && deity_apply_table[value].param )
           {
             i += 0;
             if( iarg3 == 0 )
             {
               ptc( ch, "������ {C%s{x �� ������ {G%s{x.\n\r", get_rdeity( dtab.russian,'1'),
                    deity_apply_table[value].param );
               dtab.d_apply[i]=0;
               dtab.d_apply[i+1]=0;
               return;
             }
             dtab.d_apply[i]=value;
             dtab.d_apply[i+1]=iarg3;
             ptc(ch,"������ ���������� {C%s{x %s {G%s{x �� {R%d{x.\n\r",
               get_rdeity(dtab.russian,'3'),isapply?"�����������":"���������", deity_apply_table[value].param,iarg3);
             return;
           }
           if( (is_exact_name(arg2,"water air earth fire spirit mind light dark fortitude")
            || is_exact_name(arg2,"curative perception learning offence protection makemastery"))
            && deity_apply_table[value].inform )
           {
             i += 4;
             if( iarg3 == 0 )
             {
               ptc( ch, "������ {C%s{x �� ������ ����� {G%s{x.\n\r", get_rdeity( dtab.russian,'1'),
                    deity_apply_table[value].inform );
               dtab.d_apply[i]=0;
               dtab.d_apply[i+1]=0;
               return;
             }
             dtab.d_apply[i]=value;
             dtab.d_apply[i+1]=iarg3;
             ptc(ch,"������ ���������� {C%s{x %s ����� {Y%s{x �� {R%d{x.\n\r",
               get_rdeity(dtab.russian,'3'),isapply?"�����������":"���������",deity_apply_table[value].inform,iarg3);
             return;
           }
           if( (is_exact_name(arg2,"rslash rpierce rbash rfire rpoison rmental")
            || is_exact_name(arg2,"rlight rnegative racid rholy"))
            && deity_apply_table[value].resist )
           {
             i += 8;
             if( iarg3 == 0 )
             {
               ptc( ch, "������ {C%s{x �� ������ ������ �� {D%s{x.\n\r", get_rdeity( dtab.russian,'1'),
                    deity_apply_table[value].resist );
               dtab.d_apply[i]=0;
               dtab.d_apply[i+1]=0;
               return;
             }
             dtab.d_apply[i]=value;
             dtab.d_apply[i+1]=iarg3;
             ptc(ch,"������ ���������� {C%s{x %s ������ �� {G%s{x �� {R%d{x.\n\r",
               get_rdeity(dtab.russian,'3'),isapply?"�����������":"���������",deity_apply_table[value].resist,iarg3);
             return;
           }
           return;
         }
         if( !str_cmp( arg2, "spell") || !str_cmp( arg2, "skill") )
         {
           return;
         }

         if( !str_cmp( arg2, "bonus") )
         {
           return;
         }
       }
     }
   stc("�� ������� ��������� ��� ���������.\n\r", ch);
   stc("������ '{Gdeity help{x' ��� �������.\n\r", ch);
   return;
}
#undef dtab

void damage_obj( CHAR_DATA *ch, OBJ_DATA *obj, int dam_v, bool inf_char )
{
  if( !IS_SET(global_cfg,CFG_DAMAGEOBJ) ) return;

  if( ch == NULL || !obj ) return;

  if( !str_cmp( get_obj_cond(obj, 0), SHOW_COND_ETERNAL ) ) return;

  if( obj->durability != -1)
  {
    obj->condition -= dam_v;

    if( obj->condition < 0 ) obj->condition = 0;

    if( number_range(1,2) == 1 && inf_char )
      act("� $i2 ������� �����!",ch,obj,NULL,TO_ROOM);

    if( obj->condition == 0 )
    {
      if( IS_IMMORTAL(ch) )
      {
        ptc( ch,"{RCondition of {x%s {Rrestored!\n\r", get_obj_desc( obj, '1') );
        obj->condition = obj->durability;
        return;
      }
      act("$i1 � ������� ��������!",ch,obj,ch->fighting,TO_ROOM);
      if( obj->carried_by != NULL)
        if( material_table[material_num(obj->material)].d_dam > 450 )
        {
          unequip_char( ch, obj);
          return;
        }
      obj_from_char(obj);
      extract_obj(obj);
    }
  }
};

void do_pk ( CHAR_DATA *ch, const char *argument)
{
};

#define rdeity deity_table[ch->pcdata->dn].russian
void do_supplicate ( CHAR_DATA *ch, const char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  if( ch->deity == NULL || IS_SET( ch->act, PLR_DISAVOWED) )
  {
    stc("���� �� ������ ����.\n\r", ch);
    return;
  }
  if( EMPTY(argument) || !str_prefix( argument, "help") )
  {
    stc("{c������� ���� �������� � ������.{x\n\r", ch);
    stc("{c���������:{x\n\r", ch);
    stc("{G  supplicate <blessing>{x\n\r", ch);
    stc("{c��������� �����:{x\n\r", ch);
    stc("{G  recall  - ����������� � ����.{x\n\r", ch);
    stc("{G  peace   - ���������� ��� � �������.{x\n\r", ch);
    stc("{G  aid     - ��������������� �������.{x\n\r", ch);
    stc("{G  uprise  - ����� ������� ���������� ( �� �� �����).{x\n\r", ch);
//    stc("{G  protect - ���������� �� ����� �� �������� ����.{x\n\r", ch);
//    stc("{c��������� ���� ���� �������������� ���������.{x\n\r", ch);
//    stc("{c������ ���������� � ����� ��������.{x\n\r", ch);
    return;
  }
  if( ch->deity == NULL )
  {
    stc("���� �� ������ ����.\n\r", ch);
    return;
  }
  if( ch->pcdata->favour < 1 )
  {
    ptc( ch, "{C%s{x �� ������ ����� ������...\n\r", get_rdeity( rdeity, '0') );
    return;
  }
  if( IS_SET(ch->in_room->room_flags, ROOM_ARENA) || IS_SET( ch->act, PLR_ARMY) )
  {
    ptc( ch, "{C%s{x �� ����� �������� ���� �����...\n\r", get_rdeity( rdeity,'0') );
    return;
  }
  if (ch->pcdata->condition[COND_ADRENOLIN]!=0 && ch->level<102)
  {
    stc( "{r�� ������� ���������� ��� �������.{x\n\r", ch );
    return;
  }

  if( is_affected(ch,skill_lookup("pray")) )
  {
    ptc( ch, "{C%s{x ����� �� ����� ������...\n\r",
         get_rdeity( deity_table[ch->pcdata->dn].russian, '0') );
    change_favour(ch, -20);
    return;
  }
  else if( !IS_ELDER(ch) )
  {
    AFFECT_DATA paf;

    paf.where        = TO_AFFECTS;
    paf.type         = skill_lookup("pray");
    paf.level        = ch->level;
    paf.duration     = ch->level / 15+5;
    paf.location     = APPLY_NONE;
    paf.modifier     = 0;
    paf.bitvector    = 0;
    affect_to_char( ch, &paf);
  }

  if( !IS_ELDER(ch) ) WAIT_STATE( ch, 2-(IS_SET(ch->act,PLR_HIGHPRIEST)?1:0) );

  if( !str_prefix( argument, "recall") )
  {
    if( ch->move < 2 ) return;
    if( ROOM_EVIL(ch) && !favour(ch, -350) ) return;
    if( is_affected( ch, AFF_CURSE ) ) return;
    if( ch->fighting != NULL )
    {
      if( !favour(ch, -30) ) return;
      stop_fighting( ch, TRUE);
    }
    if( ch->pcdata->condition[COND_ADRENOLIN] != 0 && !favour(ch,-550) ) return;
    do_printf(buf,"{Y$c1{x ����� {C%s{x ��������� ���!{/{C%s{x �������� {Y$c2{x!",
      get_rdeity( rdeity ,'4'), get_rdeity( rdeity, '0') );
    act(buf,ch,NULL,NULL,TO_ROOM);
    ptc( ch, "�� ������ {C%s{x ��������� ����!{x\n\r", get_rdeity( rdeity,'4') );
    ptc( ch, "{C%s{x ��������� ���� � ����!{x\n\r", get_rdeity( rdeity,'0') );
    char_from_room(ch);
    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE) );
    do_function( ch, &do_look, "");
    return;
  }

  if( !str_prefix( argument, "peace") )
  {
     CHAR_DATA *vch;

     if( ROOM_EVIL(ch) && !favour(ch, -50) ) return;

     for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
     {
       if( vch->fighting != NULL )
       {
         if( !IS_NPC( vch) && !favour(ch, -35) ) continue;
         stop_fighting( vch, TRUE);
       }
       if( IS_NPC(vch) && IS_SET( vch->act, ACT_AGGRESSIVE) )
       {
         if( !favour( ch, -25) ) continue;
         REM_BIT( vch->act, ACT_AGGRESSIVE);
       }
       if( !IS_CREATOR(ch) ) DAZE_STATE( ch, 2);
     }
    do_printf(buf,"{Y%s{x ����� {C%s{x ���������� ���!{/��������� ����� ��������� �������, ��������� �������...",
      ch->name, get_rdeity( rdeity ,'4') );
    act(buf,ch,NULL,NULL,TO_ROOM);
    ptc(ch,"�� ������ {C%s{x ���������� ���!{x\n\r", get_rdeity(rdeity,'4') );
    ptc(ch,"��������� ���� ��������� �������, ��������� �������!{x\n\r");
    return;
  }

  if( !str_cmp( argument, "aid") )
  {
    int heal;

    if( !favour( ch, -55) ) return;
    else
    {
      heal = number_range( ch->level, 5 * ch->level);
      ch->mana = UMIN( ch->mana + heal, ch->max_mana );
      heal = number_range( ch->level, 4 * ch->level);
      ch->hit =   UMIN( ch->hit + heal, ch->max_hit );
      heal = number_range( ch->level, 3 * ch->level);
      ch->move = UMIN( ch->move + heal, ch->max_move );
      update_pos(ch);
      do_printf(buf,"{Y%s{x ����� {C%s{x � ����� �����!{/� ������ {Y%s{x ���������� ���� �������...", ch->name,
      get_rdeity( rdeity ,'4'), ch->name );
      act(buf,ch,NULL,NULL,TO_ROOM);
      ptc(ch,"�� ������ {C%s{x � ����� �����!{x\n\r", get_rdeity(rdeity,'4') );
      stc("�� ����������, ��� ������ ���� ���� �������� �������!{x\n\r", ch);
      return;
    }
    return;
  }

  if( !str_cmp( argument, "uprise") )
  {
    int dfav=0;

    if( ch->fighting != NULL ) dfav -= 45;
    if( ch->pcdata->condition[COND_ADRENOLIN] != 0 ) dfav -= 200;
    if( ROOM_EVIL(ch) ) dfav -= 45;

    if( is_affected( ch, gsn_blindness) )
      if( favour( ch, dfav -= 45) )
      {
        affect_strip( ch, gsn_blindness );
        REM_BIT(ch->affected_by,AFF_BLIND);
        ptc( ch, "%s ��������� ���� ������.\n\r", get_rdeity( rdeity,'0') );
        act("$c1 ������ �� ��������.",ch,NULL,NULL,TO_ROOM);
      }
    if( is_affected( ch, gsn_curse) )
      if( favour( ch, dfav -= 65) )
      {
        affect_strip( ch, gsn_curse );
        REM_BIT(ch->affected_by,AFF_CURSE);
        stc("�� ���������� ���� �����.\n\r", ch);
        act("$c1 ��������� ���� ����� ������������.",ch,NULL,NULL,TO_ROOM);
      }
    if( is_affected( ch, gsn_poison) )
      if( favour( ch, dfav -= 40) )
      {
        affect_strip( ch, gsn_poison );
        REM_BIT(ch->affected_by,AFF_POISON);
        stc("����� �������� ������ ����, ����� ����.\n\r", ch);
        act("$c1 �������� ����������� �����.",ch,NULL,NULL,TO_ROOM);
      }
    if( is_affected( ch, gsn_plague) )
      if( favour( ch, dfav -= 85) )
      {
        affect_strip( ch, gsn_plague );
        REM_BIT(ch->affected_by,AFF_PLAGUE);
        stc("����, ����������� ����, �������.\n\r", ch);
        act("$c1 �������� �������������, � $g ���� �������.",ch,NULL,NULL,TO_ROOM);
      }
    do_printf( buf,"�� ��������� ��� ����� ���������� �������� ��� {C%s{x", get_rdeity( rdeity,'2') );
    act( buf,ch,NULL,NULL,TO_ALL_IN_ROOM);
    return;
  }

  if (!str_prefix(argument,"protect") && ch->pcdata->condition[COND_ADRENOLIN] == 0 && !ch->fighting)
  {
    if( favour( ch, -50) )
    {
      AFFECT_DATA caff;

      if( is_affected( ch, gsn_gaseous_form) )
      {
        stc("�� ��� � ����� �������...\n\r", ch);
        return;
      }
      caff.where     = TO_AFFECTS;
      caff.type      = skill_lookup("gaseous form");
      caff.level     = 120;
      caff.duration  = deity_table[ch->pcdata->dn].worship / 3 + 1;
      caff.location  = APPLY_NONE;
      caff.modifier  = 0;
      caff.bitvector = AFF_GASEOUS_FORM;
      affect_to_char( ch, &caff );
      do_printf( buf,"�� ��������� ��� ����� ���������� �������� ��� {C%s{x", get_rdeity( rdeity,'2') );
      act( buf,ch,NULL,NULL,TO_ALL_IN_ROOM);
      stc( "�� ���������� ��� ������ �������� ��������� ������ ���� ����.\n\r", ch);
      return;
    }
    ptc( ch, "{C%s{x ������������ �� ����...\n\r", get_rdeity( deity_table[ch->pcdata->dn].russian,'1') );
  }
#undef rdeity
}

void do_forge( CHAR_DATA *ch, const char *argument)
{
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg);

  if( EMPTY(arg) || !str_cmp( argument, "help") )
  {
    stc("{c������� ��������� ��������.{x\n\r", ch);
    stc("{c������������ ��������� ����.{x\n\r", ch);
    stc("{c���������:{x\n\r", ch);
    stc("{G  forge <obj> <new_material> [char]{x\n\r", ch);
    return;
  }

  if( ch == NULL )
  {
    bug("Null char in do_forge", 0);
    return;
  }

  argument = one_argument( argument, arg1);

  if ( ( victim = get_char_room( ch, argument) ) == NULL) victim = ch;

  if( ( obj = get_obj_wear( victim, arg1)) != NULL );
  else if( !(obj = get_obj_victim( ch, victim, arg)) )
  {
    stc("������� �� ������!\n\r", ch);
    return;
  }

  if( !str_cmp( arg1, material_table[material_num(arg1)].name ) )
  {
    ptc( ch,"�� ���������� � %s %s %s{x.\n\r", material_table[material_num(obj->material)].real_name,
      get_obj_desc( obj, '2'), material_table[material_num(material_lookup(arg1))].real_name );
    if( victim != ch )
      ptc( victim,"%s ��������� � %s %s %s{x.\n\r", ch->name, material_table[material_num(obj->material)].real_name,
           get_obj_desc( obj, '2'), material_table[material_num(material_lookup(arg1))].real_name );
    free_string(obj->material);
    obj->material = str_dup(arg1);
    save_char_obj(victim);
    return;
  }
  stc("������ '{Gforge help{x' ��� �������.{x\n\r", ch);
};

void do_ldefend( CHAR_DATA *ch, const char *argument)
{
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int lp=0;

  argument = one_argument( argument, arg);
  argument = one_argument( argument, arg1);

  if( EMPTY(arg) || !str_cmp( arg, "help") )
  {
    stc("{c�������� ��������� �� ������. ���� ������ ��������������{x\n\r", ch);
    stc("{c������ �� ������ ��������� �������� �� ����������� ���������.{x\n\r", ch);
    stc("{c���������:\n\r", ch);
    stc("{G   ldefend <char> <level>\n\r", ch);
    return;
  }

  if( (victim = get_pchar_world( ch, arg)) == NULL || IS_NPC(victim) )
  {
    stc("��� ����� ���.\n\r", ch);
    return;
  }

  if( (lp=atoi(arg1)) < 1 || lp > get_trust(ch) || victim->pcdata->protect > get_trust(ch) )
  {
    stc("�� �����.\n\r", ch);
    return;
  }
  ptc( ch, "�� ��������� {Y%s{x ������� {R%d{x.\n\r", victim->name, lp);
  do_printf(buf,"%s ������ ������ ��������� %s �� %d �� %d.\n\r",
      ch->name,victim->name,victim->pcdata->protect,lp);
  send_note("{YSystem{x","Dragon","Character was protected",buf,3);
  victim->pcdata->protect = lp;
};

