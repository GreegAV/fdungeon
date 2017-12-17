// $Id: magic.c,v 1.130 2004/05/28 16:30:49 ghost Exp $
// Copyrights (C) 1998-2001, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'
#include <sys/types.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <time.h> 
#include "merc.h"
#include "magic.h" 
#include "interp.h" 
#include "recycle.h" 
 
// command procedures needed
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_say           );
 
// Local functions.
void say_spell   args( ( CHAR_DATA *ch, int sn ) );
int  cast_rate   args( ( int value) );
void add_pkiller (CHAR_DATA *ch, CHAR_DATA *killer);
bool remove_obj  args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void wear_obj    args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool w_left ) );
int  min_level   (CHAR_DATA *ch,int sn);
int  get_int_modifier args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int dt ) );
int calc_saves (CHAR_DATA *victim);
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn) ;
 
int skill_lookup( const char *name ) 
{ 
  int sn;
  if (EMPTY(name)) return -1;

  for ( sn = 0;sn < max_skill;sn++ ) 
  { 
   if ( skill_table[sn].name == NULL ) break;
   if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0]) 
     && !str_prefix( name, skill_table[sn].name ) ) return sn;
  } 
  return -1;
} 
 
int find_spell( CHAR_DATA *ch, const char *name ) 
{ 
  // finds a spell the character can cast if possible
  int sn, found = -1;
 
  if (IS_NPC(ch)) return skill_lookup(name);
 
  for ( sn = 0;sn < max_skill;sn++ ) 
  { 
    if (skill_table[sn].name == NULL) break;
    if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) 
      && !str_prefix(name,skill_table[sn].name)) 
    { 
      if ( found == -1) found = sn;
      if (ch->level >= min_level(ch,sn) && ch->pcdata->learned[sn] > 0) return sn;
    } 
  } 
  return found;
} 
 
// Utter mystical words for an sn.
void say_spell( CHAR_DATA *ch, int sn ) 
{ 
  char buf  [MAX_STRING_LENGTH];
  char buf2 [MAX_STRING_LENGTH];
  CHAR_DATA *rch;
  char *pName;
  int iSyl,length;
 
  struct syl_type 
  { 
    char *      old;
    char *      new;
  };
 
  static const struct syl_type syl_table[] = 
  { 
    { " ",     " "     }, 
    { "ar",    "abra"  }, 
    { "au",    "kada"  }, 
    { "bless", "fido"  }, 
    { "blind", "nose"  }, 
    { "bur",   "mosa"  }, 
    { "cu",    "judi"  }, 
    { "de",    "oculo" }, 
    { "en",    "unso"  }, 
    { "light", "dies"  }, 
    { "lo",    "hi"    }, 
    { "mor",   "zak"   }, 
    { "move",  "sido"  }, 
    { "ness",  "lacri" }, 
    { "ning",  "illa"  }, 
    { "per",   "duda"  }, 
    { "ra",    "gru"   }, 
    { "fresh", "ima"   }, 
    { "re",    "candus" }, 
    { "son",   "sabru"  },
    { "spr",   "wifl"   },
    { "tect",  "infra"  }, 
    { "tri",   "cula"   }, 
    { "ven",   "nofo"   }, 
    { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, 
    { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" }, 
    { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" }, 
    { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" }, 
    { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" }, 
    { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, 
    { "y", "l" }, { "z", "k" }, 
    { "", "" } 
  };
 
  buf[0] = '\0';
  for ( pName = skill_table[sn].name;*pName != '\0';pName += length ) 
  { 
    for ( iSyl = 0;(length = strlen(syl_table[iSyl].old)) != 0;iSyl++ ) 
    { 
      if ( !str_prefix( syl_table[iSyl].old, pName ) ) 
      { 
        strcat( buf, syl_table[iSyl].new );
        break;
      } 
    } 
    if ( length == 0 ) length = 1;
  } 
 
  do_printf( buf2, "$c1 �������� '%s'.", buf );
  do_printf( buf,  "$c1 �������� '%s'.", skill_table[sn].name );
 
  for ( rch = ch->in_room->people;rch;rch = rch->next_in_room ) 
  { 
    if ( rch != ch ) act((!IS_NPC(rch) && ch->class==rch->class) ? buf : buf2, 
        ch, NULL, rch, TO_VICT );
  } 
 
  return;
} 

// Calculate Int&bonus vs Wis&saves damage modifier
int  get_int_modifier ( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int dt )
{
    int m_int,m_wis,m_bonus,m_saves;
    
    if (IS_NPC(ch)) return 0;
    
    m_int=URANGE(-100,(get_curr_stat(ch,STAT_INT)-23)*10,100);
    m_wis=URANGE(-100,(get_curr_stat(victim,STAT_WIS)-23)*10,100);
    m_bonus=URANGE(-30,5*category_bonus(ch,skill_table[sn].group),30);
    m_saves=URANGE(0,(calc_saves(victim)-50)*20/ch->level,20);
    
    if (IS_NPC(victim)) { m_wis=0; m_saves=10; }
    
    return URANGE(-50,m_int+m_bonus-m_wis-m_saves,200);
} 
// calculate saves (4 easier use)
int calc_saves (CHAR_DATA *victim)
{
 int saves = victim->saving_throw;
 int lvl = victim->level;
 
 if (lvl <= 15)            saves *= 3;
 if (lvl >=16 && lvl < 40) saves *= 2;
// if (lvl >=40 && lvl < 60) // left for integrity
 if (lvl >=60 && lvl <  90) saves = saves *3/4;
 if (lvl >=90 && lvl < 100) saves = saves *2/3;
 if (lvl >=100 && lvl <102) saves /= 2;
  
 return 50 - saves;
} 

// Compute a saving throw. 
// Negative apply's make saving throw better.
bool saves_spell( int level, CHAR_DATA *victim, int dam_type ) 
{ 
  int save = calc_saves(victim) + UMIN((victim->level - level)*2 ,50);
 
  switch(check_immune(victim,dam_type,NULL))
  { 
    case IS_IMMUNE:     return TRUE;
    case IS_RESISTANT:  save += 15;     break;
    case IS_VULNERABLE: save -= 15;     break;
  } 
//  if (IS_AFFECTED(victim,AFF_BERSERK)) save += (victim->level/10)+(victim->level>70)?10:(victim->level>45)?7:5;
 
  if (!IS_NPC(victim)) 
  { 
    if (victim->classmag==1 || victim->classcle==1) save = 10 * save / 9;
    else save = 9 * save / 10;
  } 
  save = URANGE( 5, save, 95 );

  switch(check_immune(victim,dam_type,NULL)) 
  { 
    case IS_RESISTANT:  save += 3;     break;
    case IS_VULNERABLE: save -= 3;     break;
  } 
  return number_percent( ) < save;
} 
 
// RT save for dispels
bool saves_dispel( int dis_level, int spell_level, int duration) 
{ 
  int save;
       
  if (duration == -1) spell_level += 5; //very hard to dispel permanent affects
 
  save = 50 + (spell_level - dis_level) * 5;
  save = URANGE( 5, save, 95 );
  return number_percent( ) < save;
} 
 
// co-routine for dispel magic and cancellation
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn) 
{ 
  AFFECT_DATA *af;
 
  if (is_affected(victim, sn)) 
  { 
    for ( af = victim->affected;af != NULL;af = af->next ) 
    { 
      if ( af->type == sn ) 
      { 
        if (!saves_dispel(dis_level,af->level,af->duration)) 
        { 
          affect_strip(victim,sn);
          if ( skill_table[sn].msg_off ) 
               ptc(victim,"%s\n\r",skill_table[sn].msg_off, victim );
          return TRUE;
        } 
        else af->level--;
      } 
    } 
  } 
  return FALSE;
} 
 
// for finding mana costs -- temporary version
int mana_cost (CHAR_DATA *ch, int sn) 
{ 
  int mana=0,bonus=1, div;

  div=2+ch->level-min_level(ch,sn);
  if (div!=0) mana=UMAX(skill_table[sn].min_mana, 100 / div);
  else mana = 50;
  bonus=category_bonus(ch,skill_table[sn].group);
  bonus+=get_skill_bonus(ch,sn);
  if (bonus>=10)    mana=mana/2;
  else if (bonus>5 ) mana=(mana*2)/3;
  else if (bonus>3 ) mana=(mana*4)/5;
  else if (bonus>0 ) mana=(mana*10)/11;
  else if (bonus<-5) mana=(mana*3)/2;
  else if (bonus<-3) mana=(mana*5)/4;
  else if (bonus<0 ) mana=(mana*11)/10;
  return mana;
} 
 
// The kludgy global is for spells who want more stuff from command line.
const char *target_name;
 
void do_cast( CHAR_DATA *ch, const char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  void *vo;
  int sn, target, level;

  // Free NPC's can cast spells, but charmed can not.
  if ( IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
  {
    cant_mes(ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) && !IS_IMMORTAL(ch))
  {
    stc("���-�� ��������� ���� ���������� �����������.\n\r",ch);
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
  }

  target_name = one_argument( argument, arg1 );
  one_argument( target_name, arg2 );

  if ( arg1[0] == '\0' )
  {
    stc( "����� ����������(cast)? �� ����?\n\r", ch );
    return;
  }

  if (!IS_NPC(ch))
  {
    if ((sn = find_spell(ch,arg1)) < 1 
     || skill_table[sn].spell_fun == spell_null
     || ( ( ch->level < min_level(ch,sn) || ch->pcdata->learned[sn] == 0) ) )
    {
      stc( "�� �� ������ �� ������ ���������� � ����� ���������.\n\r", ch );
      return;
    }

    if ( CLANSPELL(sn) && !check_clanspell(sn,ch->clan) && !IS_IMMORTAL(ch) )
    {
      stc("���� ����� ���������� �� ����� ������� ����.\n\r",ch);
      return;
    }

    obj1=get_eq_char(ch, WEAR_RHAND);
    obj2=get_eq_char(ch, WEAR_LHAND);

    if (obj1!=NULL && obj1->item_type==ITEM_WEAPON &&
        obj2!=NULL && obj2->item_type==ITEM_WEAPON &&
        IS_SET(skill_table[sn].flag,C_NODUAL) && !IS_IMMORTAL(ch))
    {
      stc("�� �� ������ ��� ���������, ����� �������� ����� ��������.\n\r",ch);
      return;
    }

    if ( ch->position < skill_table[sn].minimum_position && !IS_IMMORTAL(ch))
    {
      stc( "�� �� ������ ���������� ������������������.\n\r", ch );
      return;
    }

    if (IS_SET(ch->act,PLR_TIPSY)) if (tipsy(ch,"cast")) return;// tipsy by Dinger
  }

  else sn = find_spell(ch,arg1);

  // Locate targets. 
  victim        = NULL;
  obj           = NULL;
  vo            = NULL;
  target        = TARGET_NONE;

  if (!can_attack(ch,0) &&
      ( skill_table[sn].target==TAR_CHAR_OFFENSIVE
     || skill_table[sn].target==TAR_OBJ_HERE_CHAR_OFF
     || skill_table[sn].target==TAR_OBJ_CHAR_OFF
     || skill_table[sn].target==TAR_IGNORE_OFFENSIVE)) return;

  switch ( skill_table[sn].target )
  {
  default:
    bug( "Do_cast: bad target for sn %d.", sn );
    return;

  case TAR_IGNORE:
       break;
        
  case TAR_CHAR_ROOM:
       if (EMPTY(target_name))
       {
         target=TARGET_ROOM;
         vo=(void *) ch->in_room;
       }
       else
       {
         if (!(victim= get_char_room( ch, target_name)))
         {
           stc("�� ���� ����� ������.\n\r",ch);
           return;
         }
         target=TARGET_CHAR;
         vo=(void *) victim;
       }
       break;

  case TAR_IGNORE_OFFENSIVE:
    if ( (IS_SET(ch->in_room->room_flags,ROOM_SAFE) && !IS_IMMORTAL(ch) )
        || (IS_SET(ch->in_room->ra, RAFF_SAFE_PLC) && !IS_IMMORTAL(ch)) )
    {
      stc("���� �� ��������� ��������� ��� �����.\n\r",ch);
      return;
    }
    break;

  case TAR_CHAR_OFFENSIVE:
    
    if ( arg2[0] == '\0' )
    {
      if ( ( victim = ch->fighting ) == NULL )
      {
        stc( "��������� ���������� �� ����?\n\r", ch );
        return;
      }
    }
    else
    {
      if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
      {
        stc( "��� ����� ���.\n\r", ch );
        return;
      }
    }
    
    if ( !IS_NPC(ch) )
    {
     if (ch->fighting!=victim && is_safe(ch,victim))
     {
      stc("�� �� ��� ����.\n\r",ch);
      return;
     }
     if (victim->fighting!=ch)
     {
      check_criminal(ch,victim,60);
      if (ch->level < victim->level-8 ) add_pkiller(victim,ch);
     }
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      stc( "�� �� ������ ��������� ��� �� ��������� �� �����.\n\r",ch );
      return;
    }

    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_CHAR_DEFENSIVE:
    if ( arg2[0] == '\0' ) victim = ch;
    else
    {
      if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
      {
        stc( "��� ����� ���.\n\r", ch );
        return;
      }
    }

    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_CHAR_SELF:
    if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
    {
      stc( "�� �� ������ ��������� ��� ���������� �� ������.\n\r", ch );
      return;
    }

    vo = (void *) ch;
    target = TARGET_CHAR;
    break;

  case TAR_OBJ_INV:
    if ( arg2[0] == '\0' )
    {
      stc( "�� ��� �� ������ ��������� ��� ����������?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
    {
      stc( "�� �� ������ �����.\n\r", ch );
      return;
    }

    vo = (void *) obj;
    target = TARGET_OBJ;
    break;

  case TAR_OBJ_HERE:
    if ( arg2[0] == '\0' )
    {
      stc( "�� ��� �� ������ ��������� ��� ����������?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_here( ch, target_name) ) == NULL )
    {
      stc( "�� �� ������ �����.\n\r", ch );
      return;
    }

    vo = (void *) obj;
    target = TARGET_OBJ;
    break;

  case TAR_OBJ_CHAR_OFF:
    if (arg2[0] == '\0')
    {
      if ((victim = ch->fighting) == NULL)
      {
        stc("��������� ��� ���������� �� ���� (��� �� ���)?\n\r",ch);
        return;
      }
      target = TARGET_CHAR;
    }
    else if ((victim = get_char_room(ch,target_name)) != NULL)
     target = TARGET_CHAR;

    if (target == TARGET_CHAR) /* check the sanity of the attack */
    {
      if(is_safe_spell(ch,victim,FALSE))
      {
        stc("�� �� ��� ����.\n\r",ch);
        return;
      }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
      {
        stc( "�� �� ������ ��������� ��� �� ��������� �� �����.\n\r",ch );
        return;
      }

      if (!IS_NPC(ch))
      {
        if (victim->fighting!=ch)
        {
          if (is_safe_spell(ch,victim,FALSE)) return;
          check_criminal(ch,victim,60);
          if (ch->level < victim->level-8 ) add_pkiller(victim,ch);
        }
      }
      vo = (void *) victim;
    }
    else if ((obj = get_obj_here(ch,target_name)) != NULL)
    {
      vo = (void *) obj;
      target = TARGET_OBJ;
    } 
    else
    {
      stc("�� �� ������ ����� ���.\n\r",ch);
      return;
    }
    break;

  case TAR_OBJ_HERE_CHAR_OFF:
    if (arg2[0] == '\0')
    {
      if ((victim = ch->fighting) == NULL)
      {
        stc("��������� ��� ���������� �� ���� (��� �� ���)?\n\r",ch);
        return;
      }
      target = TARGET_CHAR;
    }
    else if ((victim = get_char_room(ch,target_name)) != NULL) target = TARGET_CHAR;
    if (target == TARGET_CHAR) /* check the sanity of the attack */
    {
      if(is_safe_spell(ch,victim,FALSE) && victim != ch)
      {
        stc("�� �� ��� ����.\n\r",ch);
        return;
      }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
      {
        stc( "�� �� ������ ��������� ��� �� ��������� �� �����.\n\r",ch );
        return;
      }

      if (!IS_NPC(ch))
      {
         if (victim->fighting!=ch)
          {
           check_criminal(ch,victim,60);
           if (ch->level < victim->level-8 ) add_pkiller(victim,ch);
          }
      }
      vo = (void *) victim;
    }
    else if ((obj = get_obj_here(ch,target_name)) != NULL)
    {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else
    {
      stc("�� �� ������ ����� ���.\n\r",ch);
      return;
    }
    break;

  case TAR_OBJ_CHAR_DEF:
    if (arg2[0] == '\0')
    {
      vo = (void *) ch;
      target = TARGET_CHAR;                                                
    }
    else if ((victim = get_char_room(ch,target_name)) != NULL)
    {
      vo = (void *) victim;
      target = TARGET_CHAR;
    }
    else if ((obj = get_obj_here(ch,target_name)) != NULL)
    {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else
    {
      stc("�� �� ������ ����� ���.\n\r",ch);
      return;
    }
    break;
    }   

    if ( !IS_NPC(ch) && ch->mana < mana_cost(ch,sn))
    {
      stc( "� ���� �� ������� ����.\n\r", ch );
      return;
    }   
    if (str_cmp(skill_table[sn].name,"ventriloquate" ) ) say_spell( ch, sn );
    if (get_trust(ch)<102) WAIT_STATE( ch, skill_table[sn].beats );
    if (IS_AFFECTED(ch,AFF_BERSERK) && ch->race!=RACE_DWARF 
     && number_range(1,10)>8 && !IS_IMMORTAL(ch))
    {
       stc( "���� ������ �� ��������� ���� ������������������...\n\r", ch );
       check_improve(ch,sn,FALSE,1);
       ch->mana -= mana_cost(ch,sn) / 2;
       return;
    }
    else if (ch->level <= (MAX_LEVEL - 3)
          && number_percent( ) > URANGE(1,get_skill(ch,sn) * (100 + (cast_rate(get_curr_stat(ch,STAT_WIS)) + cast_rate(get_curr_stat(ch,STAT_INT)))*3)/100,100))
    {
     switch ( number_range(1,6) )
      {
       case 1: stc( "�� ������� ������������...\n\r", ch );break;
       case 2: stc( "�� �� ���� ���������� ����� ���������� ����������...\n\r", ch );break;
       case 3: stc( "�� �� ���� �������� ������� ���������� �������...\n\r", ch );break;
       case 4: stc( "����� ����������� ���� �������� ���� ���������������...\n\r", ch );break;
       case 5: stc( "\"��������, ������� - ��������!\"- ������ �� ���� � ����� ������..\n\r",ch);break;
       case 6: stc( "�� �� ���� ������� ������ � ������ ������...\n\r", ch );break;
      }
       check_improve(ch,sn,FALSE,1);
       ch->mana -= mana_cost(ch,sn) / 2;
       return;
    }
    else
    {
//      if( IS_MAGIC_DEITY(ch) ) change_favour( ch, 1);
///      if( IS_MIGHT_DEITY(ch) ) change_favour( ch, -1);

      level = ch->level;
      ch->mana -= mana_cost(ch,sn);

      if (!IS_NPC(ch))
      {
       if(ch->classmag && skill_table[sn].skill_level[CLASS_MAG] <= LEVEL_HERO)
       {
        if(ch->classcle && skill_table[sn].skill_level[CLASS_CLE] <= LEVEL_HERO)
        {
         level+=cast_rate(UMAX(get_curr_stat(ch,STAT_WIS),get_curr_stat(ch,STAT_INT)));
        }
        else level+=cast_rate(get_curr_stat(ch,STAT_INT));
       }
       else
       {  
        if((ch->classcle && skill_table[sn].skill_level[CLASS_CLE] <= LEVEL_HERO))
        {
         level+=cast_rate(get_curr_stat(ch,STAT_WIS));
        }
       }
      } 
      else level = (level*3)/4;
       
      level+=category_bonus(ch,skill_table[sn].group);
      level+=get_skill_bonus(ch,sn);
      if (is_affected(ch,skill_lookup("vbite"))) level-=level/20;
      level=URANGE(1,level,IS_IMMORTAL(ch) ? 200:120);

      (*skill_table[sn].spell_fun) (sn, level, ch, vo,target);
      check_improve(ch,sn,TRUE,1);
      
      if (!IS_NPC(ch) && ch->pcdata->charged_num>0 && ch->pcdata->charged_spell==sn)
      {
        for (; ch->pcdata->charged_num>0; ch->pcdata->charged_num--)
        {
          if (victim!=NULL && victim->in_room!=ch->in_room) break;

          (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);
           check_improve(ch,sn,TRUE,1);
        }
        ch->pcdata->charged_spell=0;
        ch->pcdata->charged_num=0;
      }
    }
    
    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for ( vch = ch->in_room->people;vch;vch = vch_next )
      {
        vch_next = vch->next_in_room;
        if ( victim == vch && victim->fighting == NULL )
        {       
          if (victim->fighting!=ch)
          {
            check_criminal(ch,victim,60);
//            if (ch->level < victim->level-8 ) add_pkiller(victim,ch);
            if (!PK_RANGE(victim,ch)) add_pkiller(victim,ch);
          }
          multi_hit( victim, ch);
          break;
        }
      }
  }
}

// Cast spells at targets using a magical object.
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) 
{ 
  void *vo;
  int target = TARGET_NONE;
 
  if (IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) && !IS_IMMORTAL(ch))
  {
    stc("���-�� ���������� ��� ���������� ���� �� ����������.\n\r",ch);
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
  }

  if ( sn <= 0 ) return;
 
  if ( sn >= max_skill || skill_table[sn].spell_fun == 0 ) 
  { 
    bug( "Obj_cast_spell: bad sn %d.", sn );
    return;
  } 
 
  switch ( skill_table[sn].target ) 
  { 
    default: 
      bug( "Obj_cast_spell: bad target for sn %d.", sn );
      return;
 
    case TAR_IGNORE: 
      vo = NULL;
      break;
 
    case TAR_CHAR_OFFENSIVE: 
      if ( (IS_SET(ch->in_room->room_flags,ROOM_SAFE) && !IS_IMMORTAL(ch) )
           || (IS_SET(ch->in_room->ra, RAFF_SAFE_PLC) && !IS_IMMORTAL(ch) ) )
      { 
        stc("���� �� ��������� ��������� ��� �����.\n\r",ch);
        return;
      } 
      if ( victim == NULL ) victim = ch->fighting;
      if ( victim == NULL ) 
      { 
        stc( "�� �� ������ ����� �������.\n\r", ch );
        return;
      } 
      if (is_safe(ch,victim) && ch != victim) 
      { 
        stc("��� �����������...\n\r",ch);
        return;
      } 
      vo = (void *) victim;
      target = TARGET_CHAR;
    if ( !IS_NPC(victim) && victim->clan != NULL && IS_SET(victim->clan->flag, CLAN_WARRIOR) 
         && IS_AFFECTED(victim,AFF_FORCEFIELD)
         && number_percent() < 65 )
       {
        if (ch == victim)
         {
          stc("{y���� ��������� � ����� ��������� ����������!{x\n\r",ch);
          act("{y��������� {W$c2{y � ����� ��������� ����������!{X",ch,NULL,NULL,TO_ROOM);
         }
        else
         {
          stc("{y���� ��������� � ����� ��������� ����������!{x\n\r",victim);
          act("{y��������� {W$c2{y � ����� ��������� ����������!{X",victim,NULL,NULL,TO_ROOM);
          multi_hit(victim,ch);
         }
        return;
       } 
      break;
 
    case TAR_CHAR_DEFENSIVE: 
    case TAR_CHAR_SELF: 
      if ( victim == NULL ) victim = ch;
      vo = (void *) victim;
      target = TARGET_CHAR;
    if ( !IS_NPC(victim) && victim->clan != NULL && IS_SET(victim->clan->flag, CLAN_WARRIOR) 
         && IS_AFFECTED(victim,AFF_FORCEFIELD) )
       {
        if (ch == victim)
         {
          stc("{y���� ��������� � ����� ��������� ����������!{x\n\r",ch);
          act("{y��������� {W$c2{y � ����� ��������� ����������!{X",ch,NULL,NULL,TO_ROOM);
         }
        else
         {
          stc("{y���� ��������� � ����� ��������� ����������!{x\n\r",victim);
          act("{y��������� {W$c2{y � ����� ��������� ����������!{X",victim,NULL,NULL,TO_ROOM);
         }
        return;
       } 
      break;
 
    case TAR_OBJ_INV: 
      if ( obj == NULL ) 
      { 
        stc( "�� �� ������ ����� �������.\n\r", ch );
        return;
      } 
      vo = (void *) obj;
      target = TARGET_OBJ;

      if ( !IS_NPC(ch) && ch->clan != NULL
           && IS_SET(ch->clan->flag, CLAN_WARRIOR)
           && IS_AFFECTED(ch,AFF_FORCEFIELD) )
       {
          stc("{y���� ��������� � ����� ��������� ����������!{x\n\r",ch);
          act("{y��������� {W$c2{y � ����� ��������� ����������!{X",ch,NULL,NULL,TO_ROOM);
          return;
       }
      break;
 
     case TAR_OBJ_HERE: 
      if ( obj == NULL ) 
      { 
        stc( "�� �� ������ ����� �������.\n\r", ch );
        return;
      } 
      vo = (void *) obj;
      target = TARGET_OBJ;
      break;
 
   case TAR_OBJ_CHAR_OFF: 
      if (!victim && !obj) 
      {
        if (ch->fighting) victim = ch->fighting;
        else 
        {
          stc("�� �� ������ ����� �������.\n\r",ch);
          return;
        }
      }
      if (victim)
      { 
        if (is_safe_spell(ch,victim,FALSE) && ch != victim) 
        { 
          stc("��� �����������...\n\r",ch);
          return;
        } 
        vo = (void *) victim;
        target = TARGET_CHAR;
      } 
      else 
      { 
        vo = (void *) obj;
        target = TARGET_OBJ;
      } 
      break;
 
 
    case TAR_OBJ_CHAR_DEF: 
      if (victim == NULL && obj == NULL) 
      { 
        vo = (void *) ch;
        target = TARGET_CHAR;
      } 
      else if (victim != NULL) 
      { 
        vo = (void *) victim;
        target = TARGET_CHAR;
      } 
      else 
      { 
        vo = (void *) obj;
        target = TARGET_OBJ;
      } 
      break;
    } 
 
    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);
 
    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch 
    &&   victim->master != ch ) 
    { 
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
 
      for ( vch = ch->in_room->people;vch;vch = vch_next ) 
      { 
        vch_next = vch->next_in_room;
        if ( victim == vch && victim->fighting == NULL ) 
        { 
          if (victim->fighting==NULL) 
          { 
            check_criminal(ch,victim,60);
            if (ch->level < victim->level-8 ) add_pkiller(victim,ch);
          } 
          multi_hit( victim, ch);
          break;
        } 
      } 
    } 
} 
 
// Spell functions.
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = dice( level, 14 );
  if ( saves_spell( level, victim, DAM_ACID ) ) dam /= 2;
  if (ch->classmag==0) dam/=2;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("acid blast"),DAM_ACID)/100;
  damage( ch, victim, dam, sn, DAM_ACID, TRUE, FALSE, NULL);
///  if( IS_WATER_DEITY(ch) ) change_favour(ch, 2);
}

void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found=FALSE;
 
  if (is_affected(victim,skill_lookup("armor"))) 
  { 
    for ( aff=victim->affected;aff;aff=aff->next ) 
     if ( aff->type == skill_lookup("armor")) 
     {
       if (ch==victim || level >= aff->level) 
       { 
         affect_strip(victim,skill_lookup("armor"));
         found=TRUE;
       } 
       else  
       { 
         act("$C1 ��� �������.",ch,NULL,victim,TO_CHAR);
         return;
       } 
     }
  }

//  if(ch->classcle==0 && ch->classmag!=0)  level= 7*level/8;

  af.where = TO_AFFECTS;
  af.type  = sn;
  if (ch!=victim) level=UMIN(level,victim->level);
  af.duration  = 5 + level;
  af.level = level;
  af.modifier = -20*(1+level/12)-10*category_bonus(ch,skill_table[sn].group);
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( victim, &af );
///  if( IS_PROTECT_DEITY(ch) ) change_favour(ch, 1);

  if (found) 
  { 
    act( "���������� ������ ������ $c1 �����������.", victim, NULL, NULL, TO_ROOM );
    stc( "�� ����������, ��� ���-�� �������� ���� � ����� �����.\n\r", victim );
  } 
  else 
  { 
    stc( "�� ����������, ��� ���-�� �������� ����.\n\r", victim );
    if ( ch != victim ) act("$C1 ������� ����� ������.",ch,NULL,victim,TO_CHAR);
  } 
} 
 
void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target) 
{ 
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int bonus = -1;
 
  /* deal with the object case first */ 

  if (target == TARGET_OBJ) 
  { 
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_BLESS)) 
    { 
      act("�� $i6 ��� ����� �������������.",ch,obj,NULL,TO_CHAR);
      return;
    } 
 
    if (IS_OBJ_STAT(obj,ITEM_EVIL)) 
    { 
      AFFECT_DATA *paf;
 
      paf = affect_find(obj->affected,gsn_curse);
      if ((IS_ORDEN(ch) && obj->carried_by==ch) || !saves_dispel(level,paf != NULL ? paf->level : obj->level,0)) 
      { 
        if (paf != NULL) affect_remove_obj(obj,paf);
        act("$i1 ���������� ������ ������� ������.",ch,obj,NULL,TO_ROOM);
        act("$i1 ���������� ������ ������� ������.",ch,obj,NULL,TO_CHAR);
        REM_BIT(obj->extra_flags,ITEM_EVIL);
        return;
      }
      else
      {
        act("������ ���� ������ $i2 ������� ������...",ch,obj,NULL,TO_CHAR);
        return;
      } 
    } 

   if (BAD_ORDEN(ch)) 
      bonus = 1; 
         
    af.where    = TO_OBJECT;
    af.type     = sn;
    af.level    = level;
    af.duration = 6 + level;
    af.location = APPLY_SAVES;
    af.modifier = bonus;
    af.bitvector= ITEM_BLESS;
    affect_to_obj(obj,&af);
 
    act("$i1 ���������� �������������� ������.",ch,obj,NULL,TO_ROOM);
    act("$i1 ���������� �������������� ������.",ch,obj,NULL,TO_CHAR);
///    if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 1);
    if (obj->wear_loc != WEAR_NONE) ch->saving_throw += bonus;
    return;
  } 
 
  /* character target */ 
  victim = (CHAR_DATA *) vo;

  if (ch->race==RACE_VAMPIRE && victim->race==RACE_VAMPIRE)
  {
   stc("{D��������� {x�� ����� ���� {W�������������{x!\n\r",ch);
   return;
  }
  
  if (is_affected( victim, sn ) ) 
  { 
    if (victim == ch) 
       stc("�� ��� ������������.\n\r",ch);
    else 
       act("$C1 ��� ����� ������������ ���������������.",ch,NULL,victim,TO_CHAR);
    return;
  } 
  
  bonus=0;
//  if (IS_ORDEN(ch) || IS_ORDEN(victim))
   if (ch->alignment >= 750 && victim->alignment >=750) bonus = 1;
   else if(ch->alignment <=-750 && victim->alignment <=-750) bonus = -1;
    
//  if(ch->classcle==0 && ch->classmag!=0) level= 3*level/4;

  af.where  = TO_AFFECTS;
  af.type   = sn;
  af.location  = APPLY_HITROLL;
  af.bitvector = 0;
  if (ch!=victim) level=UMIN(level,victim->level);
  af.level = level;
  af.duration  = 6+level;
  if (bonus==0) af.modifier = 4 + level / 9;
  if (bonus>0) af.modifier  = 5 + level / 7;
  if (bonus<0) af.modifier  = 3 + level / 14;
  affect_to_char( victim, &af );
  
  af.type=sn;
  af.location  = APPLY_SAVING_SPELL;
  af.level=level;
  af.duration=6+level;
  if (bonus==0) af.modifier = 0-4-level / 9;
  if (bonus>0) af.modifier  = 0-5-level / 7;
  if (bonus<0) af.modifier  = 0-3-level / 14;
  affect_to_char( victim, &af );
  stc( "�� ���������� ���� ����� ���������.\n\r", victim );
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 2);
///  if( IS_LIGHTMAGIC_DEITY(ch) && IS_ENCHANT_DEITY(ch) ) change_favour(ch, 5);
  if ( ch != victim ) 
     act("�� ������� $C1 ������������ ���������������.",ch,NULL,victim,TO_CHAR);
} 
 
void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim->race==RACE_SKELETON)
  {
   stc("�� �� ������ �������� �������! � ���� ���� ����!\n\r",ch);
   act("$c1{x �� ����� ���� ��������.",victim,NULL,NULL,TO_ROOM);
   return;
  }
   
  if ( IS_AFFECTED(victim, AFF_BLIND)) 
  { 
   act("{y$O{x ��� ��������.",ch,NULL,victim,TO_CHAR);
   return;
  } 
 
  if (ch!=victim && saves_spell(level,victim,DAM_OTHER)) 
  { 
   stc( "�������.\n\r",ch);
   return;
  } 
 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.location  = APPLY_HITROLL;
  af.modifier  = -level/4;
  af.duration  = 1+level;
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );
  stc( "�� ��������!\n\r", victim );
  act("$c1, ������, ��������.",victim,NULL,NULL,TO_ROOM);
///  if( IS_FIRE_DEITY(ch) ) change_favour(ch, 5);
//  check_criminal( ch, victim , 60);
  return;
} 
 
void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  level = URANGE(0, level, 110);
  dam = category_bonus(ch,skill_table[sn].group)*5+number_range( level/2, level*2);
  if ( saves_spell( level, victim,DAM_FIRE) )   dam /= 2;
  if (ch->classmag==0) dam/=2;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("burning hands"),DAM_FIRE)/100;
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE, FALSE, NULL);
///  if( IS_FIRE_DEITY(ch) ) change_favour(ch, 10);
} 
 
void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *vch;
  CHAR_DATA *vnext;
  int dam;
  DESCRIPTOR_DATA *d;
  AFFECT_DATA af;
 
  if ( !IS_OUTSIDE(ch) ) 
  { 
    stc( "������ �� ������ � ����������.\n\r", ch );
    return;
  } 
 
  if ( weather_info.sky < SKY_RAINING ) 
  { 
    stc( "����� ������ ��� ������� ������?\n\r", ch );
    return;
  } 
 
  dam = dice(level/2+2, 30);
 
  stc( "������������� {W������{x ������� � ����� ����� ����� ���!{*\n\r", ch );
  act( "$c1 ������ {W������{x, ��������� � ����� �� �������� ��������!{*", 
        ch, NULL, NULL, TO_ROOM );
 
  for ( vch = ch->in_room->people;vch != NULL;vch = vnext ) 
  { 
    vnext = vch->next_in_room;
    if ( vch != ch && !IS_NPC(ch) && !is_safe_spell(ch,vch,TRUE)) 
      damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING)  
      ? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE, FALSE, NULL);
///    if( IS_AIR_DEITY(ch) ) change_favour(ch, 5);
    if (vch && !is_affected(vch,skill_lookup("deaf")) && number_range(0,5)>4)
    {
      stc("������ ������ �������� ����!\n\r",vch);
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 1;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = 0;
      affect_to_char( vch, &af );
    }
  } 
  for (d=descriptor_list;d!=NULL;d=d->next) 
  { 
    if (!d->character || d->connected!=CON_PLAYING) continue;
    else vch=d->character;
    if (!vch->in_room || vch==ch) continue;
    if (vch->in_room->area==ch->in_room->area 
     && IS_OUTSIDE(vch) && IS_AWAKE(vch)) 
     stc( "������ ������������ ����.\n\r", vch );
  } 
} 
 
void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;
 
  if (IS_SET(ch->in_room->room_flags,ROOM_SAFE) 
      || IS_SET(ch->in_room->ra, RAFF_SAFE_PLC) )
  {
    stc("���� �������� ��� ����� �� ��������.\n\r",ch);
    return;
  }

  for (vch = ch->in_room->people;vch != NULL;vch = vch->next_in_room)
  { 
    if (vch->position == POS_FIGHTING) 
     { 
       count++;
       if (IS_NPC(vch)) mlevel += vch->level;
       else  mlevel += vch->level/2;
       high_level = UMAX(high_level,vch->level);
     } 
  } 
 
  // compute chance of stopping combat
  chance = 4 * level - high_level + 2 * count;
 
  if (IS_IMMORTAL(ch)) mlevel = 0;
 
  if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */ 
  { 
    for (vch = ch->in_room->people;vch != NULL;vch = vch->next_in_room) 
    { 
      if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) || 
                         IS_SET(vch->act,ACT_UNDEAD))) 
      { 
        stc("�������.\n\r",vch);
        return;
      } 

      if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK) 
      ||  is_affected(vch,skill_lookup("frenzy"))) 
      { 
        stc("�������.\n\r",vch);
        return;
      } 
       
      stc("����� ������ ����������� ����.\n\r",vch);
 
      if (vch->fighting || vch->position == POS_FIGHTING) stop_fighting(vch,FALSE);
 
      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = level/4;
      af.location = APPLY_HITROLL;
      if (!IS_NPC(vch)) af.modifier = -5;
      else af.modifier = -2;
      af.bitvector = AFF_CALM;
      affect_to_char(vch,&af);
 
      af.location = APPLY_DAMROLL;
      affect_to_char(vch,&af);
///      if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 15);
///      if( IS_NATURE_DEITY(ch) ) change_favour(ch, 10);
    } 
  } 
} 
 
void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
  char buf[100];
  
  level += 2;
 
  if ((!IS_NPC(ch) && IS_NPC(victim) &&  
        !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) || 
        (IS_NPC(ch) && !IS_NPC(victim) && target!=100) ) 
  { 
    stc("���������� �� �������, �������� 'dispel magic'.\n\r",ch);
    return;
  } 
 
  if ( (ch != victim) && !IS_NPC(ch) && !IS_NPC(victim)  
        && IS_SET(victim->act,PLR_NOCANCEL)) 
  { 
    stc("���� �������� ������� �� ����� cancellation.\n\r",ch);
    return;
  } 
 
  /* unlike dispel magic, the victim gets NO save */ 
  /* begin running through the spells */ 
  if (check_dispel(level,victim,skill_lookup("armor"))) found = TRUE;
  if (check_dispel(level,victim,skill_lookup("bless"))) found = TRUE;
  if (check_dispel(level,victim,gsn_blindness)) 
  { 
    found = TRUE;
    act("$c1 ����� ����� ������.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("calm"))) 
  { 
    found = TRUE;
    act("$c1 ������ �� �������� ����������...",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("wbreath"))) 
  { 
    found = TRUE;
    act("$c1 ������, ����� ������� ������ ������...",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("change sex"))) 
  { 
    found = TRUE;
    act("$c1 �������� ����� ������� �� ����.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,gsn_charm_person)) 
  { 
    found = TRUE;
    act("$c1 ����� ������ ������ ����.",victim,NULL,NULL,TO_ROOM);
    stop_follower(victim);
  } 

  if (check_dispel(level,victim,skill_lookup("chill touch"))) 
  { 
    found = TRUE;
    act("$c1 ��������� ���������.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("fireshield"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect evil"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect good"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect hidden"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect invis"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect magic"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("faerie fire"))) 
  { 
    act("������� ������ ������ ���� $c1 ��������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("fly"))) 
  { 
    act("$c1 ������ �� �����!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("frenzy"))) 
  { 
    act("$c1 ����� �� �������� ��������.",victim,NULL,NULL,TO_ROOM);;
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("giant strength"))) 
  { 
    act("$c1 ����� �� �������� �������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("haste"))) 
  { 
    act("$c1 ��� �� ��������� ��� ������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("infravision"))) found = TRUE;

  if (check_dispel(level,victim,gsn_invis)) 
  { 
    act("$c1 ���������� �� ��������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("mass invis"))) 
  { 
    act("$c1 ���������� �� ��������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("pass door"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("protection evil"))) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("protection good"))) found = TRUE; 

  if (check_dispel(level,victim,skill_lookup("sanctuary"))) 
  { 
    do_printf(buf,"����� ���� ������ ���� $c2 ��������.");
    act(buf,victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("shield"))) 
  { 
    act("����, ���������� $c4, ���������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,gsn_sleep)) found = TRUE;

  if (check_dispel(level,victim,skill_lookup("slow"))) 
  { 
    act("$c1 ����� �� ��������� ��� ��������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 
  
  if (check_dispel(level,victim,skill_lookup("ensnare")))
  {
    found=TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("stone skin"))) 
  { 
    act("���� $c2 ������������ � ���������� ���������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("weaken"))) 
  { 
    act("$c1 �������� �������.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("regeneration"))) found = TRUE;

  if (found)
  {
     stc("�� ���� �����-�� ����������.\n\r",ch);
///     if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 10);
  }
  else       stc("���������� �� �������.\n\r",ch);
} 
 
void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  damage(ch,(CHAR_DATA *) vo, dice(1, 8) + level / 3, sn,DAM_HARM,TRUE, FALSE, NULL);
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  damage(ch,(CHAR_DATA *) vo, dice(3, 8) + level - 6, sn,DAM_HARM,TRUE, FALSE, NULL);
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  damage(ch,(CHAR_DATA *) vo, dice(2, 8) + level / 2, sn,DAM_HARM,TRUE, FALSE, NULL);
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict,*last_vict,*next_vict;
  bool found;
  int dam;
 
  /* first strike */ 
  doact("������ ������ ��������� � ������� $c2 � ������� � $C4.",ch,NULL,victim,TO_ROOM,SPAM_OTHERF);
  act("������ ������ ��������� � ����� ������� � ������� � $C4.", ch,NULL,victim,TO_CHAR);
  act("������ ������ ��������� � ������� $c2 � �������� ����!", ch,NULL,victim,TO_VICT);  
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 2); 

  dam = dice(level,6);
  if (saves_spell(level,victim,DAM_NONE)) dam /= 4;
  else dam /= 2;
  if (ch->classmag==0) dam/=2;
 
  damage(ch,victim,dam,sn,DAM_NONE,TRUE, FALSE, NULL);
  if (!ch || !victim) return;
  last_vict = victim;
  level -= 4; // decrement damage
 
  // new targets
  while (level > 0) 
  { 
    found = FALSE;
    for (tmp_vict = ch->in_room->people;tmp_vict != NULL; 
         tmp_vict = next_vict)  
    { 
      next_vict = tmp_vict->next_in_room;
      if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict) 
      { 
        found = TRUE;
        last_vict = tmp_vict;
        dam = dice(level,6);
        if (saves_spell(level,tmp_vict,DAM_NONE)) dam /= 4;
         else dam /= 2;
        damage(ch,tmp_vict,dam,sn,DAM_NONE,TRUE, FALSE, NULL);
        if (!ch) return;
        level -= 4; /* decrement damage */ 
      }
    }
     
    if (!found) // no target found, hit the caster
    { 
      if (ch == NULL) return;

      if (last_vict == ch) // no double hits
      { 
        act("������ ������ ���� ����.",ch,NULL,NULL,TO_ROOM);
        act("������ �������� ����� ���� ���� � �����, �� �������� �����.", 
            ch,NULL,NULL,TO_CHAR);
        return;
      } 
     
      last_vict = ch;
      doact("���������� ������ �������� � $c4...",ch,NULL,NULL,TO_ROOM,SPAM_OTHERF);
      stc("�� ������� ����� ����������� �������!\n\r",ch);
      dam = dice(level,6);
      if (saves_spell(level,ch,DAM_NONE)) dam /= 4;
       else dam /= 2;
      damage(ch,ch,dam,sn,DAM_NONE,TRUE, FALSE, NULL);
      level -= 4; /* decrement damage */
      if (!ch) return;
    } 
  // now go back and find more targets
  } 
} 
           
void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn )) 
  { 
    if (victim == ch) stc("�� ��� ������� ���!\n\r",ch);
    else act("$C1 ��� �������...��������...�������...���.",ch,NULL,victim,TO_CHAR);
    return;
  } 
  if (saves_spell(level , victim,DAM_OTHER)) return;    
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2 * level;
  af.location  = APPLY_SEX;
  do 
  { 
    af.modifier  = number_range( 0, 2 ) - victim->sex;
  } 
  while ( af.modifier == 0 );
  af.bitvector = 0;
  affect_to_char( victim, &af );
  stc( "�� ���������� ���� ���-�� �������...\n\r", victim );
  act("$c1 �������� ��������� �� ����...",victim,NULL,NULL,TO_ROOM);
} 
 
void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;
 
  dam = category_bonus(ch,skill_table[sn].group)*5+number_range( level / 2, level * 2 );
  if (ch->classmag==0) dam/=2;
 
  if ( !saves_spell( level, victim,DAM_COLD ) ) 
  { 
    act("$c1 ����������� ��������� � ������.",victim,NULL,NULL,TO_ROOM);
    stc("�� ������������ ��������� � �������.",victim);
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 6;
    af.location  = APPLY_STR;
    af.modifier  = -1;
    af.bitvector = 0;
    affect_join( victim, &af );
  } 
  else dam /= 2;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("chill touch"),DAM_COLD)/100;
  damage( ch, victim, dam, sn, DAM_COLD,TRUE, FALSE, NULL );
///  if( IS_WATER_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int d_temp=3*(level-1)+level/2;
  int dam=(level>54)? dice(level-25,12) : number_range(d_temp/2,d_temp*2);
  if ( saves_spell( level, victim,DAM_LIGHT) ) dam /= 2;
  else spell_blindness(gsn_blindness, level/2,ch,(void *) victim,TARGET_CHAR);
  if (ch->classmag==0) dam/=2;

  dam+=dam*get_int_modifier(ch,victim,skill_lookup("colour spray"),DAM_LIGHT)/100;
  damage( ch, victim, dam, sn, DAM_LIGHT,TRUE, FALSE, NULL );
///  if( IS_WATER_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  OBJ_DATA *light;

  if (target_name[0] != '\0')  /* do a glow on some object */ 
  { 
    light = get_obj_here(ch,target_name);
    
    if (light == NULL) 
    { 
      stc("�� �� ������ ����� ���.\n\r",ch);
      return;
    } 

    if (light->item_type==ITEM_LIGHT) 
    { 
      light->value[2]+=ch->level*2+10;
      act("$i1 ���������� ����.",ch,light,NULL,TO_ROOM);
      act("$i1 ���������� ����.",ch,light,NULL,TO_CHAR);
///      if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 2);
      return;
    } 

    if (IS_OBJ_STAT(light,ITEM_GLOW)) 
    { 
      act("$i1 ��� ��������.",ch,light,NULL,TO_CHAR);
      return;
    } 

    SET_BIT(light->extra_flags,ITEM_GLOW);
    act("$i1 ���������� ����� ����� ������.",ch,light,NULL,TO_ROOM);
    act("$i1 ���������� ����� ����� ������.",ch,light,NULL,TO_CHAR);
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
    return;
  } 
  if((light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 )))
  {
   obj_to_room( light, ch->in_room );
   act( "$c1 ���������� ������, �������� $i4.",   ch, light, NULL, TO_ROOM );
   act( "�� ����������� ������, �������� $i4.", ch, light, NULL, TO_CHAR );
///   if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 5);
  }
  else stc("{RBUG! Unable to create obj! Report to Imms NOW!{x\n\r",ch);
} 
 
void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target)  
{ 
  if ( !str_cmp( target_name, "better" ) )
  {
    weather_info.change += dice( level / 3, 4 );
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
  }
  else if ( !str_cmp( target_name, "worse" ) )
  {
    weather_info.change -= dice( level / 3, 4 );
///    if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 5);
  }
  else 
  { 
    stc ("�� ������ ��������(better) ��� ��������(worse) ������?\n\r", ch );
    return;
  } 
  stc( "Ok.\n\r", ch );
} 
 
void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  OBJ_DATA *mushroom;
 
  if((mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 )))
  {
   mushroom->value[0] = level / 2;
   mushroom->value[1] = level;
   obj_to_room( mushroom, ch->in_room );
   act( "$i1 �������� ��������� ��-��� �����.", ch, mushroom, NULL, TO_ROOM );
   act( "$i1 �������� ��������� ��-��� �����.", ch, mushroom, NULL, TO_CHAR );
///   if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 5);
  }
  else stc("{RBUG! Unable to create obj! Report to Imms NOW!{x\n\r",ch);
} 
 
void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  OBJ_DATA *rose;
  if ((rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0)))
  {
   act("$c1 ������� �������������� ������� ����.",ch,rose,NULL,TO_ROOM);
   stc("�� �������� �������������� ������� ����.\n\r",ch);
   obj_to_char(rose,ch);
///   if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 5);
///   if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 5);
  }
  else stc("{RBUG! Unable to create obj! Report to Imms NOW!{x\n\r",ch);
} 
 
void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  OBJ_DATA *spring;
 
  if((spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 )))
  {
   spring->timer = level;
   obj_to_room( spring, ch->in_room );
   act( "$i1 ����������� ��-��� �����.", ch, spring, NULL, TO_ROOM );
   act( "$i1 ����������� ��-��� �����.", ch, spring, NULL, TO_CHAR );
///   if( IS_WATER_DEITY(ch) ) change_favour(ch, 5);
///   if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 5);
  }
  else stc("{RBUG! Unable to create obj! Report to Imms NOW!{x\n\r",ch);
}
 
void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  OBJ_DATA *obj;
  int water;
//  AFFECT_DATA *af;

  if (target == TARGET_OBJ)
  {
    obj = (OBJ_DATA *) vo;
    if ( obj->item_type != ITEM_DRINK_CON ) 
    { 
      stc( "��� �� ����� ��� �������� ����!.\n\r", ch );
      return;
    } 
    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 ) 
    { 
      stc( "����� �����-�� ������ ��������.\n\r", ch );
      return;
    } 
    water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2), 
                (int)(obj->value[0] - obj->value[1]));
   
    if ( water > 0 ) 
    { 
      obj->value[2] = LIQ_WATER;
      obj->value[1] += water;
      if ( !is_name( "water", obj->name ) ) 
      { 
        char buf[MAX_STRING_LENGTH];
 
        do_printf( buf, "%s water", obj->name );
        free_string( obj->name );
        obj->name = str_dup( buf );
      } 
      act( "$i1 ������ �������� �����.", ch, obj, NULL, TO_CHAR );
///      if( IS_WATER_DEITY(ch) ) change_favour(ch, 8);
    } 
    return;
  }

  if ( is_affected(ch, gsn_dirt) && number_range(1,10)>7 )
  {
    affect_strip(ch,gsn_dirt);
    if ( skill_table[gsn_dirt].msg_off )
         ptc(ch,"%s\n\r",skill_table[gsn_dirt].msg_off, ch );
    return;
  }
  if ( is_affected(ch, skill_lookup("fire breath")) && number_range(1,10)>9 )
  {
    affect_strip(ch, skill_lookup("fire breath"));
    if ( skill_table[skill_lookup("fire breath")].msg_off )
         ptc(ch,"%s\n\r",skill_table[skill_lookup("fire breath")].msg_off, ch );
    return;
  }
  return;
} 
 
void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_blindness ) ) 
  { 
      if (victim == ch) stc("�� �� ��������.\n\r",ch);
      else act("$C1, ������, � ��� �����.",ch,NULL,victim,TO_CHAR);
      return;
  } 

  if( IS_SET( victim->act, PLR_DISAVOWED) )
  {
    stc("��������� ����� �� �������...!\n\r", ch);
    return;
  }

  if (check_dispel(level,victim,gsn_blindness)) 
  {
    act("$c1 ������ �� ��������.",victim,NULL,NULL,TO_ROOM);
///    if( IS_WATER_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 2);
  }
  else stc("���������� �� ���������.\n\r",ch);
} 
 
void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;
  bool cleric;

  if (victim->hit>=victim->max_hit)
  {
    stc("�������� ��������� ��������������.\n\r",ch);
    update_pos( victim );
    return;
  }
  if (IS_NPC(ch) && IS_SET(ch->act,ACT_CLERIC)) cleric=TRUE;
  else cleric=ch->classcle;

  heal = number_range(level/(2-ch->classcle),level*(1+cleric));
  victim->hit = UMIN( victim->hit + heal, victim->max_hit );
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
///  if( IS_NATURE_DEITY(ch) ) change_favour(ch, 1);
  update_pos( victim );
  stc( "�� ���������� ���� ������� �����!\n\r", victim );
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 
 
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_plague ) && !IS_SET(victim->act,PLR_SIFILIS))
  { 
      if (victim == ch) stc("�� �� �����.\n\r",ch);
      else act("$C1 � ��� ������.",ch,NULL,victim,TO_CHAR);
      return;
  } 

//  if(ch->classcle==0 && ch->classmag!=0) level= 3*level/4;

   
  if (check_dispel(level,victim,gsn_plague)) 
  { 
    stc("����, ����������� ����, �������.\n\r",victim);
    act("$c1 �������� �������������, � $g ���� �������.",victim,NULL,NULL,TO_ROOM);
///    if( IS_NATURE_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
  } 
  else if (!IS_NPC(victim) && IS_SET(victim->act,PLR_SIFILIS) && number_percent()<(IS_IMMORTAL(ch))?25:5)
  {
    REM_BIT(victim->act,PLR_SIFILIS);
    stc("�� ��������� ������� ��������� �� ��������.\n\r",victim);
    act("$c1 ��������� �� ��������.",victim,NULL,NULL,TO_ROOM);
///    if( IS_NATURE_DEITY(ch) ) change_favour(ch, 2);
  }
  else stc("���������� �� ���������.\n\r",ch);
} 
 
void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;
  bool cleric;

  if (victim->hit>=victim->max_hit)
  {
    stc("�������� ��������� ��������������.\n\r",ch);
    update_pos( victim );
    return;
  }
  if (IS_NPC(ch) && IS_SET(ch->act,ACT_CLERIC)) cleric=TRUE;
  else cleric=ch->classcle;

  heal = number_range(level/(2-ch->classcle),level*(1+cleric))/2;
  victim->hit = UMIN( victim->hit + heal, victim->max_hit );
///  if( IS_NATURE_DEITY(ch) ) change_favour(ch, 1);
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
  update_pos( victim );
  stc( "�� ���������� ���� ������� �����.\n\r", victim );
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 
 
void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_poison ) ) 
  { 
    if (victim == ch) stc("�� �� ��������.\n\r",ch);
    else act("$C1 �� ��������.",ch,NULL,victim,TO_CHAR);
    return;
  } 

//  if(ch->classcle==0 && ch->classmag!=0) level= 3*level/4;


  if (check_dispel(level,victim,gsn_poison)) 
  { 
    stc("����� �������� ������ ����, ����� ����.\n\r",victim);
    act("$c1 �������� ����������� �����.",victim,NULL,NULL,TO_ROOM);
///    if( IS_NATURE_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
  } 
  else stc("���������� �� ���������.\n\r",ch);
} 
 
void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;
  bool cleric;

  if (victim->hit>=victim->max_hit)
  {
    stc("�������� ��������� ��������������.\n\r",ch);
    update_pos( victim );
    return;
  }
  if (IS_NPC(ch) && IS_SET(ch->act,ACT_CLERIC)) cleric=TRUE;
  else cleric=ch->classcle;

  heal = number_range(level/(2-ch->classcle),level*(1+cleric))/3*2;
  victim->hit = UMIN( victim->hit + heal, victim->max_hit );
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
///  if( IS_NATURE_DEITY(ch) ) change_favour(ch, 1);
  update_pos( victim );
  stc( "�� ���������� ���� �����!\n\r", victim );
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 

void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
 
  if (target == TARGET_OBJ) 
  { 
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_EVIL)) 
    { 
      act("$i1 ��� �������� ���.",ch,obj,NULL,TO_CHAR);
      return;
    } 
 
    if (IS_OBJ_STAT(obj,ITEM_BLESS)) 
    { 
      AFFECT_DATA *paf;
 
      paf = affect_find(obj->affected,skill_lookup("bless"));

      if (paf!=NULL && paf->duration==-1)
      {
        act("���� ����� ������ ���������� �� ����� ���������� ������ ������ $i2 ...",ch,obj,NULL,TO_CHAR);
        return;
      }

      if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0)) 
      { 
        if (paf != NULL) affect_remove_obj(obj,paf);
        act("$i1 ���������� ������� ������.",ch,obj,NULL,TO_ROOM);
        act("$i1 ���������� ������� ������.",ch,obj,NULL,TO_CHAR);
        REM_BIT(obj->extra_flags,ITEM_BLESS);
        return;
      } 
      else 
      { 
        act("������ ���� ������ $i2 ������� ������...",ch,obj,NULL,TO_CHAR);
        return;
      } 
    } 
 
    af.where        = TO_OBJECT;
    af.type         = sn;
    af.level        = level;
    af.duration     = 2 * level;
    af.location     = APPLY_SAVES;
    af.modifier     = +1;
    af.bitvector    = ITEM_EVIL;
    affect_to_obj(obj,&af);
 
    act("$i1 ���������� ���� �����.",ch,obj,NULL,TO_ROOM);
    act("$i1 ���������� ���� �����.",ch,obj,NULL,TO_CHAR);
 
    if (obj->wear_loc != WEAR_NONE) ch->saving_throw += 1;
    return;
  } 
 
  victim = (CHAR_DATA *) vo;
 
  if (is_safe(ch,victim) && victim != ch) 
  { 
    stc("�� �����.\n\r",ch);
    return; 
  } 
 
  if ( IS_AFFECTED(victim,AFF_CURSE)
    || check_immune(victim,DAM_NEGATIVE,NULL)==IS_IMMUNE)
  { 
    stc("�������.\n\r",ch);
    return;
  } 
/*
  if ( is_affected(victim, skill_lookup("shine")) && number_percent() < 30)
  {
    ptc(ch,"���� %s �������� ��������� �����.\n\r",get_char_desc(victim,'1'));
    ptc(victim,"���� ���� �������� ��������� %s �����.\n\r",get_char_desc(ch,'1'));
    return;
  }
*/

  if (ch->race==RACE_ZOMBIE && number_percent()<40)
    ptc(ch,"���� ��� ������, ��� ��������� ������ %s.\n\r",get_char_desc(victim,'1'));
  else if (ch!=victim && saves_spell(level,victim,DAM_NEGATIVE))
  {
    stc("�������.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2*level;
  af.location  = APPLY_HITROLL;
  af.modifier  = -1 * (level / 8);
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );
 
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = level / 8;
  affect_to_char( victim, &af );
///  if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 5);
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 1);
//  check_criminal( ch, victim , 10);
  stc( "�� ���������� ���� ���������.\n\r", victim );
  if ( ch != victim ) act("$C1 �������� ���������.",ch,NULL,victim,TO_CHAR);
} 
 
void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  ISORDEN(ch)
  if ( !IS_NPC(ch) && !IS_EVIL(ch) ) 
  { 
    victim = ch;
    stc("������� ������� ���������� �� ����!\n\r",ch);
  } 
 
  ch->alignment = UMAX(-1000, ch->alignment - 50);
 
  if (victim != ch) 
  { 
    act("$c1 ������ ������� ���, ����� �������� $C4!",ch,NULL,victim,TO_ROOM);
    act("$c1 ������ �� ���� ������� ���!",ch,NULL,victim,TO_VICT);
    stc("�� ������� �� ������ ������� ���!\n\r",ch);
///    if( IS_EVIL_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 3);
  } 
//  if(ch->classcle==0 && ch->classmag!=0) level= 3*level/4;
  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_NEGATIVE) ) dam /= 2;
  dam = dam * 3/2;
  if (level>21) dam+=category_bonus(ch,skill_table[sn].group)*3;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("demonfire"),DAM_NEGATIVE)/100;
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL);
  spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
} 
 
void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found=FALSE;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) ) 
  { 
    for ( aff=victim->affected;aff!=NULL;aff=aff->next ) 
     if ( aff->type ==skill_lookup("detect evil")) 
     { 
       if (ch==victim || level >= aff->level) 
       { 
         affect_strip(victim,skill_lookup("detect evil"));
         found=TRUE;
       } 
     } 
    if (!found) 
    { 
      if (victim == ch) stc("�� � ��� ������ �������� �������� ���� �� ������.\n\r",ch);
      else act("$C1 ��� ����� �������� �������� ���� �� ������.",ch,NULL,victim,TO_CHAR);
      return;
    } 
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char( victim, &af );
 
  if (found) { 
  stc( "���� ����� ����� �����������.\n\r",victim);
  stc( "Ok.\n\r", ch);} 
  else 
  { 
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
    stc( "���� ����� �����������.\n\r", victim );
    stc( "Ok.\n\r", ch);
  } 
} 
 
void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found=FALSE;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) ) 
  { 
    for ( aff=victim->affected;aff!=NULL;aff=aff->next ) 
     if ( aff->type ==skill_lookup("detect good")) 
     { 
      if (ch==victim || level >= aff->level) 
      { 
       affect_strip(victim,skill_lookup("detect good"));
       found=TRUE;
      } 
     } 
 
    if (!found) 
    { 
      if (victim == ch) stc("�� � ��� ������ �������� �������� ����� �� ������.\n\r",ch);
      else act("$C1 ��� ����� �������� �������� ����� �� ������.",ch,NULL,victim,TO_CHAR);
      return;
    } 
  } 

 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_GOOD;
  affect_to_char( victim, &af );
 
  if (found) 
  { 
     stc( "���� ����� ����� �����������.\n\r",victim);
     stc( "Ok.\n\r", ch);
  } 
  else 
  { 
///    if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
    stc( "���� ����� �����������.\n\r", victim );
    stc( "Ok.\n\r", ch);
  } 
} 
 
void spell_detect_hidden( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found=FALSE;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) ) 
  { 
    for ( aff=victim->affected;aff!=NULL;aff=aff->next ) 
     if ( aff->type == skill_lookup("detect hidden")) 
     { 
      if (ch==victim || level >= aff->level) 
      { 
        affect_strip(victim,skill_lookup("detect hidden"));
        found=TRUE;
      } 
     } 

    if (!found) 
    { 
    if (victim == ch) stc("�� � ��� ����� ����������. \n\r",ch);
    else act("$C1 ��� �������� ������ ��������.",ch,NULL,victim,TO_CHAR);
    return;
    } 
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level       = UMAX(level,ch->level+category_bonus(ch,PERCEP));
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_HIDDEN;
  affect_to_char( victim, &af );
 
  if (found) { 
  stc( "���� �������� ����������� �����.\n\r",victim);
  stc( "Ok.\n\r", ch);} 
  else 
  { 
///    if( IS_STEALTH_DEITY(ch) ) change_favour(ch, 4);
///    if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
    stc( "�� ����������� ����� ������������.\n\r", victim );
    stc( "Ok.\n\r", ch );
  }
} 
 
void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found=FALSE;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) ) 
  { 
    for ( aff=victim->affected;aff!=NULL;aff=aff->next ) 
     if ( aff->type ==skill_lookup("detect invis")) 
     { 
      if (ch==victim || level >= aff->level) 
      { 
       affect_strip(victim,skill_lookup("detect invis"));
       found=TRUE;
      } 
     } 
 
    if (!found) 
    { 
     if (victim == ch) stc("�� � ��� ��� ������ ���������.\n\r",ch);
     else act("$C1 ��� �������� ������ ���������.",ch,NULL,victim,TO_CHAR);
     return;
    } 
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char( victim, &af );
 
  if (found) { 
  stc( "���� ����� ����� �����������.\n\r",victim);
  stc( "Ok.\n\r", ch);} 
  else 
  { 
///    if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 2);
///    if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 3);
    stc( "���� ����� �����������.\n\r", victim );
    stc( "Ok.\n\r", ch );
  }
} 
 
void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) ) 
  { 
     if (victim == ch) stc("�� � ��� ������ ���������� ����.\n\r",ch);
     else act("$C1 ��� ����� ���������� ���������� ����.",ch,NULL,victim,TO_CHAR);
     return;
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  affect_to_char( victim, &af );
  stc( "���� ����� �����������.\n\r", victim );
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
  if ( ch != victim )   stc( "Ok.\n\r", ch );
} 
 
void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
 OBJ_DATA *obj = (OBJ_DATA *) vo;
 
 if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD ) 
 { 
  if ( obj->value[3] != 0 ) stc( "�� ���������� �������� ����.\n\r", ch );
  else stc( "��� �������� ���������.\n\r", ch );
 } 
 else stc( "��� �� �������� �����������.\n\r", ch );
/// if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 3);
} 
 
void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
   
    if ( !IS_NPC(ch) && IS_EVIL(ch) ) victim = ch;
   
    if ( IS_GOOD(victim) ) 
    { 
      act( "������� �������� $C1.", ch, NULL, victim, TO_ROOM );
      return;
    } 
 
    if ( IS_NEUTRAL(victim) ) 
    { 
      act( "$C1, ������, ���������� � ����������� ����.", ch, NULL, victim, TO_CHAR );
      return;
    }
//    if(ch->classcle==0 && ch->classmag!=0)  level= 3*level/4;
    if (victim->hit > (ch->level * 4)) dam = dice( level, 4 );
    else dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) ) dam /= 2;
    dam+=3*dam*get_int_modifier(ch,victim,skill_lookup("dispel evil"),DAM_HOLY)/200;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, FALSE, NULL);
///    if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 3);
///    if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  if ( !IS_NPC(ch) && IS_GOOD(ch) ) victim = ch;
  if ( IS_EVIL(victim) ) 
  { 
    if (number_range(1,10)>5) act( "����� �������� $C1.", ch, NULL, victim, TO_ROOM );
    else act( "Saboteur �������� $C1.", ch, NULL, victim, TO_ROOM );
    return;
  } 
  if ( IS_NEUTRAL(victim) ) 
  { 
    act( "$C1, ������, ���������� � ����������� �����.", ch, NULL, victim, TO_CHAR );
    return;
  } 
//  if(ch->classcle==0 && ch->classmag!=0) level= 3*level/4;
  if (victim->hit > (ch->level * 4)) dam = dice( level, 4 );
  else dam = UMAX(victim->hit, dice(level,4));
  if ( saves_spell( level, victim,DAM_NEGATIVE) ) dam /= 2;
  dam+=3*dam*get_int_modifier(ch,victim,skill_lookup("dispel good"),DAM_NEGATIVE)/200;
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL);
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 5);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
  char buf[100];

  if (saves_spell(level, victim,DAM_OTHER)) 
  { 
      stc( "�� ����������, ��� ����� ������ ���� �� ��� �������.\n\r",victim);
      stc( "���������� �� ���������.\n\r", ch);
      return;
  } 

  /* begin running through the spells */  
  if (check_dispel(level,victim,skill_lookup("armor"))) 
  { 
      found = TRUE;
      act("���������� Armor ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("bless"))) 
  { 
      found = TRUE;
      act("���������� Bless ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,gsn_blindness)) 
  { 
      found = TRUE;
      act("$c1 ����� ����� ������.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("calm"))) 
  { 
      found = TRUE;
      act("$c1 ������ �� �������� ����� �����������...",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("wbreath")))
  {
    found = TRUE;
    act("$c1 ������, ����� ������� ������ ������...",victim,NULL,NULL,TO_ROOM);
  }

  if (check_dispel(level,victim,skill_lookup("change sex"))) 
  { 
      found = TRUE;
      act("$c1 ����� �������� ����� �����.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,gsn_charm_person)) 
  { 
      found = TRUE;
      act("$c1 ����� ������ ��� ����.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("chill touch"))) 
  { 
      found = TRUE;
      act("$c1 ���������� ���������.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("detect evil"))) 
  { 
      found = TRUE;
      act("���������� Detect Evil ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("detect good"))) 
  { 
      found = TRUE;
      act("���������� Detect Good ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("detect hidden"))) 
  { 
      found = TRUE;
      act("���������� Detect Hidden ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("detect invis"))) 
  { 
      found = TRUE;
      act("���������� Detect Invisible ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("detect magic"))) 
  { 
      found = TRUE;
      act("���������� Detect Magic ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("faerie fire"))) 
  { 
      act("������ ������ ���� $c2 ��������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("fly"))) 
  { 
      act("$c1 ������ �� �����!",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("frenzy"))) 
  { 
      act("$c1 ����� �� �������� ��������.",victim,NULL,NULL,TO_ROOM);;
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("fireshield"))) 
  { 
      act("�������� ��� ������ $c1 ������.",victim,NULL,NULL,TO_ROOM);;
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("giant strength"))) 
  { 
      act("$c1 ����� �� �������� �������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("haste"))) 
  { 
      act("$c1 ����� �� ��������� ��� ������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("infravision"))) 
  { 
      found = TRUE;
      act("���������� Infravision ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,gsn_invis)) 
  { 
      act("$c1 ���������� �� ��������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("mass invis"))) 
  { 
      act("$c1 ���������� �� ��������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("pass door"))) 
  { 
      found = TRUE;
      act("$c1 ������ ���� ������������.",victim,NULL,NULL,TO_ROOM);
  } 


  if (check_dispel(level,victim,skill_lookup("protection evil"))) 
  { 
      found = TRUE;
      act("���������� ������ �� {D������ ���{x ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("protection good"))) 
  { 
      found = TRUE;
      act("���������� ������ �� {W������� ���{x ������� � $c2.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("sanctuary"))) 
  { 
     do_printf(buf,"����� ���� ������ ���� $c2 ��������.");
     act(buf,victim,NULL,NULL,TO_ROOM);
     found = TRUE;
  } 

  if (IS_AFFECTED(victim,AFF_SANCTUARY)  
      && !saves_dispel(level, victim->level,-1) 
      && !is_affected(victim,skill_lookup("sanctuary"))) 
  { 
      REM_BIT(victim->affected_by,AFF_SANCTUARY);
      do_printf(buf,"����� ���� ������ ���� $c2 ���������.");
      act(buf,victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("shield"))) 
  { 
      act("����, ���������� $c1, ���������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,gsn_sleep)) 
  { 
      act("$c1 ������ ����� ����������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("slow"))) 
  { 
      act("$c1 ������ �� ��������� ��� ��������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("stone skin"))) 
  { 
      act("���� $c2 ������������� � ���������� ���������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (check_dispel(level,victim,skill_lookup("weaken"))) 
  { 
      act("$c1 �������� �������.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  } 

  if (found) stc("Ok.\n\r",ch);
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 1);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);

  act("���������� �� $N ������� ������...",ch,NULL,victim,TO_CHAR);
} 
 
void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *vch;
  CHAR_DATA *vnext;
  DESCRIPTOR_DATA *d;
 
  stc( "����� �������� ��� ������ ������!\n\r", ch );
  act( "$c1 ���������� ����� ��������.", ch, NULL, NULL, TO_ROOM );
 
  for (vch = ch->in_room->people;vch != NULL;vch = vnext) 
  { 
    vnext=vch->next_in_room;
     if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
     {
       if (IS_AFFECTED(vch,AFF_FLYING)) 
        damage(ch,vch,0,sn,DAM_BASH,TRUE, FALSE, NULL);
       else 
       { 
         damage( ch,vch,(dice(level, 4)+category_bonus(ch,skill_table[sn].group)*3)/(2-ch->classcle),sn,DAM_BASH,TRUE,FALSE,NULL);
///         if( IS_EARTH_DEITY(ch) ) change_favour(ch, 6);
///         if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
         DAZE_STATE(vch, PULSE_VIOLENCE);
       } 
     }
  } 
  for (d=descriptor_list;d!=NULL;d=d->next) 
  { 
    if (!d->character || d->connected!=CON_PLAYING) continue;
    else vch=d->character;
    if ( vch->in_room!=NULL && vch!=ch && vch->in_room->area == ch->in_room->area )
      stc( "����� �������� � ������.\n\r", vch );
  } 
} 
 
// Drain XP, MANA, HP. Caster gains HP.
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  ISORDEN(ch)
  if (victim != ch) ch->alignment = UMAX(-1000, ch->alignment - 50);
  if (IS_SET(race_table[ch->race].spec,SPEC_ENERGY)) level+=5;
 
  if ( saves_spell( level, victim,DAM_NEGATIVE) ) 
  { 
     stc("�� �� ��������� ����������� �������.\n\r",victim);    
     return;
  } 
 
  if ( victim->level <= 2 ) dam = ch->hit + 1;
  else 
  { 
    gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
    victim->mana        /= 2;
    victim->move        /= 2;
    if (IS_SET(race_table[ch->race].spec, SPEC_ENERGY)) 
          dam = number_range(level, level*2);
    else  dam = number_range(level/2, level);
    ch->hit += dam;
// hp>max - tuda je, kuda i double aid
    ch->hit=UMIN(ch->hit,ch->max_hit);
// was:
//    ch->hit=UMIN(ch->hit,(ch->max_hit+ch->max_hit/10));
  } 
 
  stc("�� ����������, ��� ����� ������ �� ����!\n\r",victim);
  stc("�� ����������, ��� ����� ���� ��������� � ����!\n\r",ch);
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("energy drain"),DAM_NEGATIVE)/100;
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL);
///  if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 5);
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
//  int dam=(level>44)? dice(level-9,10) : dice(level-5,9);
//  dam+=category_bonus(ch,skill_table[sn].group)*level/10;
  int dam=dice(level,12);
  if(ch->classmag==0) dam/=2;
  if ( saves_spell( level, victim, DAM_FIRE) ) dam /= 2;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("fireball"),DAM_FIRE)/100;
  damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, FALSE, NULL);
///  if( IS_FIRE_DEITY(ch) ) change_favour(ch, 3);
} 
 
void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA af;
  
  if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) 
  { 
    act("$i1 ��� ������� �� ����.",ch,obj,NULL,TO_CHAR);
    return;
  } 
  
  af.where     = TO_OBJECT;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy(level / 4)+category_bonus(ch,MAKE)*3+category_bonus(ch,FIRE)*3;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = ITEM_BURN_PROOF;
  
  affect_to_obj(obj,&af);
  
  act("�� ��������� $i4 �� ����.",ch,obj,NULL,TO_CHAR);
  act("�������� ���� ��������� $i4, ������� �� ����.",ch,obj,NULL,TO_ROOM);
///  if( IS_FIRE_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_PROTECT_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(6 + level / 2, 15);
  if ( saves_spell( level, victim,DAM_FIRE) ) dam /= 2;
//  if (ch->classcle==0) dam/=2;
  dam+=category_bonus(ch,skill_table[sn].group)*level/10;
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("flamestrike"),DAM_FIRE)/100;
  damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, FALSE, NULL);
  if (number_percent() < number_fuzzy(level/4)) 
    fire_effect(victim,level/2,dam,TARGET_CHAR);
///  if( IS_FIRE_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) return;
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = 2 * level*1+category_bonus(ch,skill_table[sn].group);
  af.bitvector = AFF_FAERIE_FIRE;
  affect_to_char( victim, &af );
  stc( "������ ���� ���������� ������� ������.\n\r", victim );
  act( "������ $c2 ���������� ������� ������.", victim, NULL, NULL, TO_ROOM );
///  if( IS_STEALTH_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 2);
} 
 
void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *vch,*vch_next;
  bool found=FALSE;
  RAFFECT *af;
 
  if (IS_SET(ch->in_room->ra,RAFF_ALL_VIS))
  {
   stc("��� � ��� ����� ��� � ���",ch);
   return;
  }
    
  act( "$c1 ��������� ���� �������� ����.", ch, NULL, NULL, TO_ROOM );
  stc( "�� ���������� ���� �������� ����.\n\r", ch );
 
  af=new_raffect();
  af->level=level;
  af->duration=number_range(0,level/10+1);
  af->bit=RAFF_ALL_VIS;
  raffect_to_room( af,ch->in_room);
  
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
   vch_next = vch->next_in_room;
   if (IS_AFFECTED(vch,AFF_INVISIBLE) || IS_AFFECTED(vch,AFF_HIDE) || IS_AFFECTED (vch,AFF_SNEAK))
   {
///    if( IS_FIRE_DEITY(ch) ) change_favour(ch, 3);
///    if( IS_PROTECT_DEITY(ch) ) change_favour(ch, -1);
    stc ("�� � ���������� �������������, ��� ���� �����..\n\r", vch);    
    found = TRUE;
   }            
  }
   if(found) act ("� ����� ���������� ������� ������� �������..\n\r",ch,NULL,NULL,TO_ROOM);
   else stc("�� ������ �� ���������",ch);
} 

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target ) 
{ 
  OBJ_DATA *disc, *floating;

  floating = get_eq_char(ch,WEAR_FLOAT);
  if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE)) 
  { 
      act("�� �� ������ ����� $i4.",ch,floating,NULL,TO_CHAR);
      return;
  } 
  if((disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0)))
  {
   disc->value[0] = ch->level * 10;/* 10 pounds per level capacity */ 
   disc->value[3] = ch->level * 5;/* 5 pounds per level max per item */ 
   disc->timer    = ch->level * 2 - number_range(0,level / 2); 

   act("$c1 ������� ������ �������� ����.",ch,NULL,NULL,TO_ROOM);
   stc("�� �������� �������� ����.\n\r",ch);
   obj_to_char(disc,ch);
   wear_obj(ch,disc,TRUE,FALSE);
///   if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 2);
  }
  else stc("{RBUG! Unable to create obj! Report to Imms NOW!{x\n\r",ch);
} 
 
void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FLYING) ) 
  { 
   if (victim == ch) stc("�� ��� �������.\n\r",ch);
   else act("$C1 �� ��������� � ����� ������ ��� �������.",ch,NULL,victim,TO_CHAR);
   return;
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = 10 + level*UMAX(0, 1 + category_bonus(ch,skill_table[sn].group));
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( victim, &af );
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 1);
///  if( IS_NEUTRAL_DEITY(ch) ) change_favour(ch, 2);
  stc( "���� ���� ���������� �� �����.\n\r", victim );
  act( "���� $c2 ���������� �� �����.", victim, NULL, NULL, TO_ROOM );
} 
 
void spell_wbreath( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, skill_lookup("wbreath")) ) 
  { 
   if (victim == ch) stc("�� ��� �������������.\n\r",ch);
   else act("$C1 ��� ������� ��� ����.",ch,NULL,victim,TO_CHAR);
   return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = 3 + level / 6;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = 0;
  affect_to_char( victim, &af );
///  if( IS_WATER_DEITY(ch) ) change_favour(ch, 6);
  act( "$c1 ������ ����� ������ ��� �����.", victim, NULL, NULL, TO_ROOM );
  stc( "�� ������ ������ ������ ��� �����.\n\r", victim );
} 


void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK)) 
  { 
    if (victim == ch) stc("�� � ��� ���������.\n\r",ch);
    else act("$C1 � ��� ���������.",ch,NULL,victim,TO_CHAR);
    return;
  } 
 
  if (is_affected(victim,skill_lookup("calm"))) 
  { 
      if (victim == ch) stc("������ �� ���� ������ �� ������������?\n\r",ch);
      else  act("$C1 ��� �� ����� ���������.", ch,NULL,victim,TO_CHAR);
      return;
  } 
 
  if ((IS_GOOD(ch) && !IS_GOOD(victim)) || 
      (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) || 
      (IS_EVIL(ch) && !IS_EVIL(victim)) ||
      BAD_ORDEN(ch) || BAD_ORDEN(victim)
     ) 
  { 
      act("����, ������, �� ����� $C1",ch,NULL,victim,TO_CHAR);
      return;
  } 
//  if(ch->classcle==0 && ch->classmag!=0)  level= 3*level/4;
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = 5 + level / 3;
  af.modifier  = 1 + level / 2;
  af.bitvector = 0;
 
  af.location  = APPLY_HITROLL;
  affect_to_char(victim,&af);
 
  af.location  = APPLY_DAMROLL;
  affect_to_char(victim,&af);
 
  af.modifier  = level*3;
  af.location  = APPLY_AC;
  affect_to_char(victim,&af);

///  if( IS_FANATIC_DEITY(ch) ) change_favour(ch, 14);
  stc("�� ����������, ��� ���� ��������� ������ ������!\n\r",victim);
  act("����� $c2 ���������� ������ � ���������!",victim,NULL,NULL,TO_ROOM);
} 
 
void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) ) 
  { 
    if (victim == ch) stc("�� � ��� ���������� �����!\n\r",ch);
    else act("$C1 �� ����� ����� ��� �������.",ch,NULL,victim,TO_CHAR);
    return;
  } 
 
//  if (ch->classmag==0 && ch->classcle != 0) level= 7*level/8;
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level       = level;
  af.duration    = level+number_range(5,9);
  af.modifier    = 1 + level/7;
  af.location  = APPLY_STR;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  if (!IS_NPC(victim) && victim->clan != NULL && IS_SET(victim->clan->flag, CLAN_WARRIOR) )
   {    
    af.modifier = level/3;
    af.location = APPLY_DAMROLL; 
    affect_to_char( victim, &af );
   }
///  if( IS_MIGHT_DEITY(ch) ) change_favour(ch, 4);
///  if( IS_EARTH_DEITY(ch) ) change_favour(ch, 2);
  stc( "���� ������� ���������� ����������� �����!\n\r", victim );
  act("������� $c2 ���������� ����������� �����.",victim,NULL,NULL,TO_ROOM);
} 
 
void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = 100;
  damage( ch, victim, dam, sn, DAM_HARM ,TRUE, FALSE, NULL);
} 
 
void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
//  if (ch->classmag==0 && ch->classcle!=0) level= 7*level/8;

  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE) 
  ||   IS_SET(victim->off_flags,OFF_FAST)) 
  { 
    if (victim == ch) stc("�� �� ������ ��������� �������!\n\r",ch);
    else act("�������� $C1 ��� ���������� �������.",ch,NULL,victim,TO_CHAR);
    return;
  } 
 
  if (IS_AFFECTED(victim,AFF_SLOW)) 
  { 
    if (!check_dispel(level,victim,skill_lookup("slow"))) 
    { 
      if (victim != ch)  stc("���������� �� ���������.\n\r",ch);
      stc("���� �������� �� ��������� ���������� �������.\n\r",victim);
      return;
    } 
    act("$c1 �������� ��������� ����� ��������.",victim,NULL,NULL,TO_ROOM);
    return;
  } 
 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = 3 + level/2;
  af.modifier  = 1 + level/7;
  af.location  = APPLY_DEX;
  af.bitvector = AFF_HASTE;
  affect_to_char( victim, &af );
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 2);
  stc( "���� �������� ���������� �������� ��� �����.\n\r", victim );
  act("$c1 �������� ��������� ������.",victim,NULL,NULL,TO_ROOM);
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 
 
void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  int heal_rate=100;
  bool cleric;
  
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  if (victim->hit>=victim->max_hit)
  {
    stc("�������� ��������� ��������������.\n\r",ch);
    update_pos( victim );
    return;
  }
  if (IS_NPC(ch) && IS_SET(ch->act,ACT_CLERIC)) cleric=TRUE;
  else cleric=ch->classcle;

  if (level>=50 && cleric) heal_rate+=25;
  if (level>=75 && cleric) heal_rate+=25;
  if (level>=100 ) heal_rate+=25;
  victim->hit = UMIN( victim->hit + heal_rate, victim->max_hit );
  update_pos( victim );
  stc( "����� ��������� ���� ����.\n\r", victim );
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 
 
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;
    
  bless_num = skill_lookup("bless");
  curse_num = gsn_curse; 
  frenzy_num = skill_lookup("frenzy");

  if (ch->hit < ch->level * 2)
  {
    stc("�� ������� �������.\n\r",ch);
    return;
  }
 
  act("$c1 ���������� ����� ������������ ����!",ch,NULL,NULL,TO_ROOM);
  stc("�� ����������� ����� ������������ ����.\n\r",ch);
  
  for ( vch = ch->in_room->people;vch != NULL;vch = vch_next ) 
  { 
    vch_next = vch->next_in_room;
 
    if ((IS_GOOD(ch) && IS_GOOD(vch)) || 
     (IS_EVIL(ch) && IS_EVIL(vch)) || 
     (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) ) 
    { 
///      if( IS_EVIL_DEITY(ch) ) change_favour(ch, 15);
///      if( IS_GOOD_DEITY(ch) ) change_favour(ch, 15);
///      if( IS_NEUTRAL_DEITY(ch) ) change_favour(ch, 15);
      stc("�� ���������� ���� ����� ��������������.\n\r",vch);
      spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
      spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
    } 
    else if ((IS_GOOD(ch) && IS_EVIL(vch)) || (IS_EVIL(ch) && IS_GOOD(vch)) ) 
    { 
      if (!is_safe_spell(ch,vch,TRUE)) 
      { 
        level+=category_bonus(ch,skill_table[sn].group)*2;
        spell_curse(gsn_curse,level*3/4,ch,(void *) vch,TARGET_CHAR);
        stc("�� �������� ������!\n\r",vch);
        dam = dice(level,6);
        damage(ch,vch,dam,sn,DAM_HOLY,TRUE, FALSE, NULL);
      } 
    } 
    else if (IS_NEUTRAL(ch)) 
    { 
      if (!is_safe_spell(ch,vch,TRUE)) 
      { 
        spell_curse(gsn_curse,level/2,ch,(void *) vch,TARGET_CHAR);
        stc("�� �������� ������!\n\r",vch);
        dam = dice(level,4);
        damage(ch,vch,dam,sn,DAM_HOLY,TRUE, FALSE, NULL);
      } 
    } 
  }   
  stc("�� ���������� ���� ����������.\n\r",ch);
  ch->move /= 2;
  ch->hit -= ch->level * 2;
} 
  
void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_INFRARED) ) 
    { 
        if (victim == ch) 
          stc("�� � ��� ������ � �������.\n\r",ch);
        else 
          act("$C1 ��� ����� � �������.\n\r",ch,NULL,victim,TO_CHAR);
        return;
    } 
    act( "����� $c2 ���������� �������.\n\r", ch, NULL, NULL, TO_ROOM );
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
///    if( IS_FIRE_DEITY(ch) ) change_favour(ch, 15);
    stc( "���� ����� ���������� �������.\n\r", victim );
} 
 
void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
 
//  if(ch->classmag==0 && ch->classcle!=0)  level= 3*level/4;
  if (target == TARGET_OBJ) 
  { 
    obj = (OBJ_DATA *) vo;  
 
    if (IS_OBJ_STAT(obj,ITEM_INVIS)) 
    { 
      act("$i1 ��� ���������.",ch,obj,NULL,TO_CHAR);
      return;
    } 
         
    af.where        = TO_OBJECT;
    af.type         = sn;
    af.level        = level;
    af.duration     = level + 12;
    af.location     = APPLY_NONE;
    af.modifier     = 0;
    af.bitvector    = ITEM_INVIS;
    affect_to_obj(obj,&af);
 
    act("$i1 ��������� �� ����.",ch,obj,NULL,TO_ROOM);
    return;
  } 
 
  victim = (CHAR_DATA *) vo;
 
  if ( IS_AFFECTED(victim, AFF_INVISIBLE) ) return;
 
  act( "$c1 ��������� �� ���������.", victim, NULL, NULL, TO_ROOM );
 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = level + 12;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char( victim, &af );
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 5);
  stc( "�� ���������� �� ���������.\n\r", victim );
} 
 
void spell_know_person(int sn,int level,CHAR_DATA *ch,void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  ptc(ch,"��� {Y%s{x, %s{x\n\r",get_char_desc(victim,'1'), race_table[victim->race].who_name);
  ptc(ch,"������� �������� {Y%d{x, �������� %s{x\n\r",URANGE(1,number_range(victim->level-1,victim->level+1),110),get_align(victim));
  ptc(ch,"{M������ ���������� ��� {Y%s{M:{x\n\r",get_char_desc(victim,'2'));
  if (IS_NPC(victim))
  {
     stc(" - {C������������ ��������{x\n\r",ch);
    if (victim->spec_fun==spec_lookup("spec_questmaster"))
     stc(" - {M����� ������ �������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_IS_KEEPER))
     stc(" - {C��������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_ASSASIN_MASTER))
     stc(" - {C��������� ������� ��� ������� ���������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_TRAIN))
     stc(" - {G����� ����������� �������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_PRACTICE))
     stc(" - {G����� ������������ ������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_ACCOUNTER))
     stc(" - {C�������� �������� �����{x\n\r",ch);
    if (IS_SET(victim->act,ACT_IS_HEALER))
     stc(" - {Y����� ������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_GAIN))
     stc(" - {G����� ������� ����� �������� � �����������{x\n\r",ch);
    if (IS_SET(victim->act,ACT_FORGER))
     stc(" - {Y�������� ��������{x\n\r",ch);

    if (IS_SET(victim->act,ACT_CLANENCHANTER))              /*(c) Wagner */
     stc(" - {Y����� ������������ ���� �����{x\n\r",ch);
    
    if (victim->spec_fun==spec_lookup("spec_executioner"))
     stc(" - ����� �� ������ ������.\n\r",ch);
    if (victim->spec_fun==spec_lookup("spec_summoner"))
     stc(" - {C����� ���������� �������{x\n\r",ch);
    if (victim->spec_fun==spec_lookup("spec_summonera"))
     stc(" - {C����� ���������� �������{x\n\r",ch);
  }
  else
  {
     stc(" - {C�����{x\n\r",ch);
    if (victim->remort>0)
     ptc(ch," - {G������������ {Y%d {G���{x\n\r",victim->remort);
    if (victim->clan) ptc(ch," - � ����� %s{x\n\r",get_clan_rank(victim));
///    if( IS_EVIL_DEITY(ch) ) change_favour(ch, 15);
  }
}
 
void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam=(level>59)? dice(level-41,15) : dice(level+1,4);
  if ( saves_spell( level, victim,DAM_LIGHTNING) ) dam /= 2;
  if (ch->classmag==0) dam/=2;
  dam+=(1+level/10)*category_bonus(ch,skill_table[sn].group);
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("lightning bolt"),DAM_LIGHTNING)/100;
  damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, FALSE, NULL);
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 5);
} 
 
void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;
 
  found = FALSE;
  number = 0;
  max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;
 
  buffer = new_buf();
  
  for ( obj = object_list;obj != NULL;obj = obj->next ) 
  { 
    if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name )  
      ||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level 
      ||   ch->level+ch->remort*3 < obj->level) continue;

    found = TRUE;
    number++;
 
    for ( in_obj = obj;in_obj->in_obj != NULL;in_obj = in_obj->in_obj ) ;
 
    if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by,CHECK_LVL))
    { 
      do_printf( buf, "���� ��������� � ����� %s\n\r", PERS(in_obj->carried_by, ch) );
    } 
    else 
    { 
      if (IS_IMMORTAL(ch) && in_obj->in_room != NULL) 
        do_printf( buf, "���� �  %s [Room %u]\n\r", in_obj->in_room->name, in_obj->in_room->vnum);
      else  
        do_printf( buf, "���� �  %s\n\r", in_obj->in_room == NULL 
          ? "����������� �����" : in_obj->in_room->name );
    } 

///    if( IS_EVIL_DEITY(ch) ) change_favour(ch, 2);
    buf[0] = UPPER(buf[0]);
    add_buf(buffer,buf);
 
    if (number >= max_found) break;
  } 
 
  if ( !found ) stc( "�� �� ����� �� �� ������� ��� ������ ���������.\n\r", ch );
  else page_to_char(buf_string(buffer),ch);
 
  free_buf(buffer);
} 
 
void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target) 
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  int dam;
  int number;
  int count=level/35+1;
  if (level>=101) count++;
  if (IS_SET(victim->vuln_flags,VULN_ENERGY)) count++;
  level=URANGE(1, level,110);
  for(number=0;number<count;number++) 
  { 
    dam=3+number_range(level / 4, level/2);
    if (saves_spell(level,victim,DAM_ENERGY)) dam/=2;
    if (ch->classmag==0) dam/=2;
    dam+=dam*get_int_modifier(ch,victim,skill_lookup("magic missile"),DAM_ENERGY)/100;
    damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, FALSE, NULL);
  } 
} 
 
void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target) 
{ 
  CHAR_DATA *gch;
  int heal_num, refresh_num;
  bool healall=TRUE;
     
  heal_num = skill_lookup("heal");
  refresh_num = skill_lookup("refresh"); 
 
  for ( gch = ch->in_room->people;gch != NULL;gch = gch->next_in_room ) 
  { 
    if (!is_same_group(ch,gch)) continue;
    if ((IS_NPC(ch) && IS_NPC(gch)) || (!IS_NPC(ch) && !IS_NPC(gch))) 
    { 
      healall=FALSE;
      spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
      spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
    } 
  } 
  /* if anybody from your group is exist in this room, heal only group 
     else heal all in room */ 
  if (!healall) return;
  for ( gch = ch->in_room->people;gch != NULL;gch = gch->next_in_room ) 
  { 
    if ((IS_NPC(ch) && IS_NPC(gch)) || (!IS_NPC(ch) && !IS_NPC(gch))) 
    { 

      spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
      spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
    } 
  } 
} 
             
void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
  AFFECT_DATA af;
  CHAR_DATA *gch;
 
  for ( gch = ch->in_room->people;gch != NULL;gch = gch->next_in_room ) 
  { 
    if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) ) continue;
    act( "$c1 �������� �������� �� ����.", gch, NULL, NULL, TO_ROOM );
    stc( "�� �������� ��������� �� ����.\n\r", gch );
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level/2;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( gch, &af );
///    if( IS_AIR_DEITY(ch) ) change_favour(ch, 1);
///    if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 2);
  } 
  stc( "Ok.\n\r", ch );
} 
 
void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
  stc( "��� �� ����������!\n\r", ch );
} 
 
void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_PASS_DOOR) ) 
  { 
    if (victim == ch)  stc("�� � ��� ������ ��������� ������ �����.\n\r",ch);
    else  act("$C1 ��� �� ����� ���������.",ch,NULL,victim,TO_CHAR);
    return;
  } 

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 4 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char( victim, &af );
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 2);
  act( "$c1 ���������� ��������������.", victim, NULL, NULL, TO_ROOM );
  stc( "�� ����������� ��������������.\n\r", victim );
} 
 
// RT plague spell, very nasty
void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  ISORDEN(ch)

  if ((ch!=victim && saves_spell(level,victim,DAM_DISEASE)) ||
      (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)) ||
      check_immune(victim,DAM_DISEASE,NULL)==IS_IMMUNE) 
  { 
    if (ch == victim) 
      stc("�� ���������� ���� �� ��������� �����, �� ��� ��������.\n\r",ch);
    else  
      act("$C1 �� �������� �������.",ch,NULL,victim,TO_CHAR);
    return;
  } 

  af.where     = TO_AFFECTS;
  af.type       = sn;
  af.level      = level * 3/4;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = -5; 
  af.bitvector = AFF_PLAGUE;
  affect_join(victim,&af);
  
///  if( IS_WATER_DEITY(ch) ) change_favour(ch, 4);
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 2);
  stc("�� ������������ � ������, ����� ������ ���� ��������� ���� ����.\n\r",victim);
  act("$c1 ����������� �� ���� � �������� ������ �����.",victim,NULL,NULL,TO_ROOM);
} 
 
void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target ) 
{ 
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;


  ISORDEN(ch)

  if (target == TARGET_OBJ) 
  { 
    obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) 
    { 
      if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) 
      { 
        act("���� ���������� �� �������� $i4.",ch,obj,NULL,TO_CHAR);
        return;
      } 
      obj->value[3] = 1;
      act("$i4 ����������� �������� ������.",ch,obj,NULL,TO_ROOM);
      return;
    } 

    if (obj->item_type == ITEM_WEAPON) 
    { 
      if (IS_WEAPON_STAT(obj,WEAPON_FLAMING) 
       || IS_WEAPON_STAT(obj,WEAPON_FROST) 
       || IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC) 
       || IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) 
      { 
        act("�� �� ������ �������� $i4.",ch,obj,NULL,TO_CHAR);
        return;
      } 
      if (IS_WEAPON_STAT(obj,WEAPON_POISON)) 
      { 
        act("���-�� ��� ������� $i4.",ch,obj,NULL,TO_CHAR);
        return;
      } 

      af.where     = TO_WEAPON;
      af.type      = sn;
      af.level     = level / 2;
      af.duration  = level/8;
      af.location  = 0;
      af.modifier  = 0;
      af.bitvector = WEAPON_POISON;
      affect_to_obj(obj,&af);

      act("$i1 ����������� ����.",ch,obj,NULL,TO_ROOM);
      return;
    } 
    act("�� �� ������ �������� $i4.",ch,obj,NULL,TO_CHAR);
    return;
  } 

  victim = (CHAR_DATA *) vo;

  if (is_safe(ch,victim) && victim != ch) 
  { 
    stc("�� �����.\n\r",ch);
    return; 
  } 

  check_criminal(ch,victim,60);
  if ( saves_spell( level, victim,DAM_POISON) ) 
  { 
    act("{Y$c1{x �� ������� ��������, �� ��� ��������.",victim,NULL,NULL,TO_ROOM);
    stc("�� ���������� ����, �� ��� ��������.\n\r",victim);
    return;
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = -2;
  af.bitvector = AFF_POISON;
  affect_join( victim, &af );
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_STEALTH_DEITY(ch) ) change_favour(ch, 3);
  stc( "�� ���������� ���� ����� �������.\n\r", victim );
  act("$c1 �������� ����� �������.",victim,NULL,NULL,TO_ROOM);
} 
 
void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int mod;

  if (!IS_GOOD(ch))
  {
    stc("�� �� ������ ��������� ��� ����������.\n\r",ch);
    return;
  }
  if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)  
  ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD)) 
  { 
    if (victim == ch) stc("�� ��� ������� �� ��� ��� �����.\n\r",ch);
    else              act("$C1 ��� ������� �� ��� ��� �����.",ch,NULL,victim,TO_CHAR);
    return;
  } 
  
  if (BAD_ORDEN(ch) || BAD_ORDEN(victim))
  mod = 1;
  else mod = -1;
   
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = mod;
  af.bitvector = AFF_PROTECT_EVIL;
  affect_to_char( victim, &af );
  stc( "�� ���������� ���� ������ � ������.\n\r", victim );
///  if( IS_GOOD_DEITY(ch) ) change_favour(ch, 7);
  if ( ch != victim ) act("$C1 ������� �� ���.",ch,NULL,victim,TO_CHAR);
} 
  
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (!IS_EVIL(ch))
  {
    stc("�� �� ������ ��������� ��� ����������.\n\r",ch);
    return;
  }
  if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)  
  ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL)) 
  { 
      if (victim == ch) stc("�� ��� ������� �� �����.\n\r",ch);
      else              act("$C1 ��� ������� �� �����.",ch,NULL,victim,TO_CHAR);
      return;
  } 

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -1;
  af.bitvector = AFF_PROTECT_GOOD;
  affect_to_char( victim, &af );
  stc( "�� ���������� ���� � ��������������� � �����.\n\r", victim );
///  if( IS_EVIL_DEITY(ch) ) change_favour(ch, 7);
  if ( ch != victim ) act("$C1 ������� �� �����.",ch,NULL,victim,TO_CHAR);
} 
 
void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
  int bonus = 0;

//  if (ch->classcle==0 && ch->classmag!=0) level= 3*level/4;

  if (IS_EVIL(ch) ) 
  { 
    victim = ch;
    stc("������� ��������� ���� �������!\n\r",ch);
  } 

  if (IS_ORDEN(ch)) 
  {
   bonus = 15;
   if (ch->alignment<750)
   {
    victim = ch;
    stc("�� �� ������ ������� ������ � �����!\n\r",ch);
   }
  }

  if (victim != ch) 
  { 
    act("$c1 ���������� ����, �������� ����� ����� �����!",ch,NULL,NULL,TO_ROOM);
    stc("�� ���������� ����, � ����� ����� ����� ������� � ����� ������!\n\r",ch);
  } 
else {
      if (IS_EVIL(ch) ) 
                      { 
                       dam = dice( level, 10 );
                       damage( ch, ch, dam/3, sn, DAM_HOLY ,TRUE, FALSE, NULL);
                       spell_blindness(gsn_blindness, level , ch, (void *) victim,TARGET_CHAR);
                       return;
                      } 
      stc("���� �� ��������� �����, ����� ��������� ����.\n\r",ch);
      return;
     } 
                                                
  if (IS_GOOD(victim) && victim != ch) 
  { 
    act("$c1 �� ��������� ������.",victim,NULL,victim,TO_ROOM);
    stc("���� �� ��������� �����, ����� ��������� ����.\n\r",victim);
    return;
  } 

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_HOLY) ) dam /= 2;

  align = victim->alignment;
  align -= 350;

  if (align < -1000) align = 1000 + 2*align ;

  dam = -(dam * align) / 1000;
  if (ch->classcle==0) dam/=2;

  if (bonus > 0) dam += dam*bonus/100;
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 3);
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("ray of truth"),DAM_HOLY)/100;
  damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, FALSE, NULL);
  spell_blindness(gsn_blindness, level , ch, (void *) victim,TARGET_CHAR);
} 
 
void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int chance, percent;
 
  if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF) 
  { 
    stc("��� ���� �� ����� ���� ��������.\n\r",ch);
    return;
  } 
 
  if (obj->value[3] >= 3 * level / 2) 
  { 
    stc("������ ������ ������������.\n\r",ch);
    return;
  } 
 
  chance = 40 + 2 * level+10*category_bonus(ch,skill_table[sn].group);
  chance -= (int)obj->value[3];/* harder to do high-level spells */ 
  chance -= (int)(obj->value[1] - obj->value[2]) * 
            (int)(obj->value[1] - obj->value[2]);
 
  chance = UMAX(level/2,chance);
 
  percent = number_percent();
 
  if (percent < chance / 2) 
  { 
    act("$i1 ���������� ������ ������.",ch,obj,NULL,TO_CHAR);
    act("$i1 ���������� ������ ������.",ch,obj,NULL,TO_ROOM);
///    if( IS_EARTH_DEITY(ch) ) change_favour(ch, 5);
    obj->value[2] = UMAX(obj->value[1],obj->value[2]);
    obj->value[1] = 0;
    return;
  } 
 
  else if (percent <= chance) 
  { 
    int chargeback,chargemax;

    act("$i1 ���������� ������ ������.",ch,obj,NULL,TO_CHAR);
    act("$i1 ���������� ������ ������.",ch,obj,NULL,TO_CHAR);
///    if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 8);

    chargemax = (int)(obj->value[1] - obj->value[2]);
     
    if (chargemax > 0) 
        chargeback = UMAX(1,chargemax * percent / 100);
    else 
        chargeback = 0;

    obj->value[2] += chargeback;
    obj->value[1] = 0;
    return;
  }      
 
  else if (percent <= UMIN(95, 3 * chance / 2)) 
  { 
    stc("������ �� ���������.\n\r",ch);
    if (obj->value[1] > 1) obj->value[1]--;
    return;
  } 
 
  else /* whoops! */ 
  { 
    act("$i1 ���� ���������� � ����������!",ch,obj,NULL,TO_CHAR);
    act("$i1 ���� ���������� � ����������!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
  } 
} 
 
void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN( victim->move + level, victim->max_move );
  if (victim->max_move == victim->move) 
      stc("�� ���������� ���� ��������� �����������!\n\r",victim);
  else stc( "�� ���������� ���� ����� ��������.\n\r", victim );
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_EARTH_DEITY(ch) ) change_favour(ch, -4);
  if ( ch != victim ) stc( "Ok.\n\r", ch );
} 
 
void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found = FALSE;
 
  if (target == TARGET_OBJ) 
  { 
    obj = (OBJ_DATA *) vo;

    if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE)) 
    { 
        if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE) 
        && (( (obj->carried_by==ch) && IS_ORDEN(ch)) 
              ||!saves_dispel(level + 2,obj->level,0))) 
        { 
            REM_BIT(obj->extra_flags,ITEM_NODROP);
            REM_BIT(obj->extra_flags,ITEM_NOREMOVE);
            act("$i1 ���������� ������� ������.",ch,obj,NULL,TO_ROOM);
            act("$i1 ���������� ������� ������.",ch,obj,NULL,TO_CHAR);
            return;
        } 
        act("��������� �� $i6 ���� ����� ������������.",ch,obj,NULL,TO_CHAR);
        return;
    } 
    act("�� $i6, ������, ��� ���������.",ch,obj,NULL,TO_CHAR);
    return;
  } 
 
  victim = (CHAR_DATA *) vo;

//  if (ch->classcle==0 && ch->classmag!=0) level*= 3/4;

  if( IS_SET( victim->act, PLR_DISAVOWED) )
  {
    stc("��������� ����� �� �������...!\n\r", ch);
    return;
  }

  if (check_dispel(level+IS_ORDEN(ch)*level/2,victim,gsn_curse)) 
  { 
///    if( IS_EARTH_DEITY(ch) ) change_favour(ch, 5);
    stc("�� ���������� ���� �����.\n\r",victim);
    act("$c1 ��������� ���� ����� ������������.",victim,NULL,NULL,TO_ROOM);
  }
  else if (ch==victim) stc("��������� �� ���� ������� ������.\n\r",ch);
 
  if (IS_NPC(victim) || (ch==victim)) return;
 
  for (obj = victim->carrying;(obj != NULL && !found);obj = obj->next_content) 
  { 
    if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE)) 
    &&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE)) 
    {    
      if (!saves_dispel(level+IS_ORDEN(ch)*level/2,obj->level,0)) 
      { 
        found = TRUE;
        REM_BIT(obj->extra_flags,ITEM_NODROP);
        REM_BIT(obj->extra_flags,ITEM_NOREMOVE);
///        if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
        act("$i1 ���������� ������� ������.",victim,obj,NULL,TO_CHAR);
        act("$i1 � ����� $c2 ���������� ������� ������.",victim,obj,NULL,TO_ROOM);
      } 
    } 
  } 
} 
 
void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{  
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  AFFECT_DATA *aff;
  bool found;
  char buf[256];
 
  found=FALSE;
  if (IS_AFFECTED(victim, AFF_SANCTUARY) || affect_find(victim->affected,sn))
  { 
    found=TRUE;
    
    aff=affect_find(victim->affected,skill_lookup("sanctuary"));
    /* Ooops... which is the right combination? FIXME? */
    if (ch==victim || ((aff && level>=aff->level) && !IS_NPC(victim)))
    {
      affect_strip(victim,skill_lookup("sanctuary"));
    } 
    else  
    { 
      act("$C1 ��� ������� ����� ���������.",ch,NULL,victim,TO_CHAR);
      return;
    } 
  } 

  if (BAD_ORDEN(victim))
  {
    stc("���� �� �������� ���������.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim!=ch)  level=UMIN(level,victim->level);
  af.level=level;
  af.duration  = 3 + level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SANCTUARY;
  affect_to_char( victim, &af );
  
  if (found) 
  { 
    act( "����� ���� ������ $c1 ���������� � ����� �����.", victim, NULL, NULL, TO_ROOM );
    stc( "����� ���� ������ ���� ���������� � ����� �����.\n\r", victim );
  } 
  else 
  { 
///    if( IS_PROTECT_DEITY(ch) ) change_favour(ch, 4);
    do_printf(buf,"$c1 ����������� ����� �����.");
    act(buf, victim, NULL, NULL, TO_ROOM );
    ptc(victim,"�� ������������ ����� �����.\n\r");
  } 
} 

void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) 
  { 
      if (victim == ch) 
        stc("�� ��� ����������� �������� �����.\n\r",ch);
      else 
        act("$C1 ��� ����������� �������� �����.",ch,NULL,victim,TO_CHAR);
      return;
  } 
  af.where     = TO_AFFECTS;
  af.type      = sn;
  if (victim != ch) level=UMIN(level,victim->level);
  af.level     = level;
  af.duration  = 9 + level;
  af.modifier  = 0 - level*3;
  af.location  = APPLY_AC;
  af.bitvector = AFF_SHIELD;
  affect_to_char( victim, &af );
///  if( IS_PROTECT_DEITY(ch) ) change_favour(ch, 4);
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, 5);
  act( "$c1 ������� �������� �����.", victim, NULL, NULL, TO_ROOM );
  stc( "�� ������� �������� �����.\n\r", victim );
} 
 
void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    { 
         0, 
         0,  0,  0,  0,  0,      0, 20, 25, 29, 33, 
        36, 39, 39, 39, 40,     40, 41, 41, 42, 42, 
        43, 43, 44, 44, 45,     45, 46, 46, 47, 47, 
        48, 48, 49, 49, 50,     50, 51, 51, 52, 52, 
        53, 53, 54, 54, 55,     55, 56, 56, 57, 57, 
        58, 58, 59, 59, 61,     61, 62, 62, 63, 63, 
        64, 64, 65, 65, 66,     66, 67, 67, 68, 68, 
        69, 69, 71, 71, 72,     72, 73, 73, 74, 74, 
        75, 75, 76, 76, 77,     77, 78, 78, 79, 79, 
        81, 81, 82, 82, 83,     83, 84, 84, 85, 85, 
        86, 86, 87, 87, 88,     88, 89, 89, 91, 355 
 
    };
    int dam;
 
    level = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level = URANGE(0, level,110);
    dam   = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_LIGHTNING) ) dam /= 2;
    dam+=dam*get_int_modifier(ch,victim,skill_lookup("shocking grasp"),DAM_LIGHTNING)/100;
    if (ch->classmag==0) dam/=2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, FALSE, NULL);
///    if( IS_AIR_DEITY(ch) ) change_favour(ch, 7);
} 
 
void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (IS_SET(race_table[ch->race].spec, SPEC_PSY)) level+=level/20;
//  if (ch->classmag==0 && ch->classcle!=0) level= 3*level/4;

  if ( is_affected(ch,gsn_sleep) 
  ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)) 
  ||   ((level + 2) < victim->level) 
  ||   (ch!=victim && saves_spell( level-4, victim,DAM_CHARM)) ) 
  {
//    check_criminal( ch, victim , 60);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2+level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_join( victim, &af );
///  if( victim!=ch && IS_MAGIC_DEITY(ch) ) change_favour(ch, 7);

  if ( IS_AWAKE(victim) ) 
  { 
      stc( "�� ���������....\n\r", victim );
      act( "$c1 ��������.", victim, NULL, NULL, TO_ROOM );
      victim->position = POS_SLEEPING;
  } 
//  check_criminal( ch, victim , 60);
} 
 
void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn )) 
  { 
      if (victim == ch) stc("�� �� ������ ��������� ��� ���������!\n\r",ch);
      else act("$C1 � ��� ������� ��� ������.", ch,NULL,victim,TO_CHAR);
      return;
  } 
  if ((ch!=victim && saves_spell(level,victim,DAM_OTHER)  )
  ||  IS_SET(victim->imm_flags,IMM_MAGIC)) 
  { 
    if (victim != ch) stc("������, ������ �� ���������.\n\r",ch);
    stc("�� �� ��������� �������� � ��������.\n\r",victim);
    return;
  } 
  if (IS_AFFECTED(victim,AFF_HASTE)) 
  { 
   if (ch!=victim)
   {
     if (!check_dispel(level,victim,skill_lookup("haste"))) 
     { 
       if (victim != ch) stc("���������� �� ���������.\n\r",ch);
       stc("�� ���������� ������ ���������������, �� ��� ������ ��������.\n\r",victim);
       return;
     }
     act("$c1 ��������� ���������.",victim,NULL,NULL,TO_ROOM);
     return;
   } 
   else
   {
     affect_strip(ch,skill_lookup("haste"));
     act("$c1 ��������� ���������.",victim,NULL,NULL,TO_ROOM);
     stc("�� ������ �� ���������� ������.\n\r",victim);
     return;
   }
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/2;
  af.location  = APPLY_DEX;
  af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
  af.bitvector = AFF_SLOW;
  affect_to_char( victim, &af );
///  if( IS_EARTH_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_AIR_DEITY(ch) ) change_favour(ch, -6);
  stc( "�� ����������, ��� ���� �������� ���������� ����������...\n\r", victim );
  act("$c1 �������� ��������� ��������.",victim,NULL,NULL,TO_ROOM);
} 

void spell_ensnare( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  int is_done, mv_value;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af,*afp;
  
  afp=&af;
  is_done=70+(level-victim->level)*3/2;
  if ( IS_AFFECTED(victim,AFF_FIRESHIELD) ) is_done-=10;
  if ( IS_AFFECTED(victim,AFF_HASTE) )      is_done-=10;
  if ( IS_AFFECTED(victim,AFF_FLYING) )     is_done-=10;
  if ( IS_AFFECTED(victim,AFF_SLOW) )       is_done+=25;
  if ( IS_AFFECTED(victim,AFF_BERSERK) )    is_done-=10;
  if ( IS_AFFECTED(victim,AFF_FAERIE_FIRE)) is_done-=10;
  is_done+=(victim->saving_throw * 5/9 - 35) * 15 / UMAX(1,victim->level);
  is_done-=URANGE(-10,(get_curr_stat(victim,STAT_DEX)-24)*3,20);
  if ( number_percent() > is_done )
  {
      stc("���� ������� ������������ ���������� ���� �� �������.\n\r",ch);
      return;
  }
  
  mv_value=victim->move;
  if ( is_affected (victim,skill_lookup("ensnare")))
  {
      afp=affect_find(victim->affected,skill_lookup("ensnare"));
      if (!afp) return;
      
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = afp->duration+1;
      af.location  = APPLY_DEX;
      af.modifier  = afp->modifier;
      af.bitvector = 0;
      affect_strip(victim,skill_lookup("ensnare"));
      
///      if( IS_EARTH_DEITY(ch) ) change_favour(ch, 7);
///      if( IS_AIR_DEITY(ch) ) change_favour(ch, -7);
      stc("�� ����������, ��� ���������� ���� ������ ���� ��������� �������!\n\r",victim);
      act("���������� ���� ������ $c1 ��������� �������.",victim,NULL,NULL,TO_ROOM);
      if ( number_percent() < (IS_NPC(victim) ? 20 : 25) )
      {
          if ( level < 40 ) af.modifier--;
          else if ( level < 65 ) af.modifier-=(1+number_percent()/60);
          else if ( level < 80 ) af.modifier-=(1+number_percent()/43);
          else if ( level < 95 ) af.modifier-=(1+number_percent()/37);
          else af.modifier-=(1+number_percent()/30);
      }
      
      affect_to_char( victim, &af );
      victim->move-=( ch->level*11/10 + number_percent()/5 );
  }
  else
  {
      stc("�� ������� ���������� �����.\n\r",victim);
      act("$c1 ������� ���������� �����.",victim,NULL,NULL,TO_ROOM);
      
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 1;
      af.location  = APPLY_DEX;
      af.modifier  = -2;
      af.bitvector = 0;
      affect_to_char( victim, &af );
      victim->move-=ch->level*3/2;
  }
  
  if ( !IS_NPC(victim) && mv_value > 0 && victim->move < 0 )
      stc("�� �������� �������� � �����!\n\r",victim);

  if ( !IS_NPC(victim) && victim->move < 0 )
      act("$c1 �������� �������� � �����!",victim,NULL,NULL,TO_ROOM);

  if ( !IS_NPC(victim) && victim->move < victim->max_move/5 && victim->move > 0)
      stc("�� ���������, ��� ���� ���� �������� � �����.\n\r",victim);

  if ( get_curr_stat(victim,STAT_DEX) < 5 )
  {
      stc("�� ����� ��������� �����������!\n\r",victim);
      act("$c1 ����� ��������� �����������!",victim,NULL,NULL,TO_ROOM);
  }
  else if ( get_curr_stat(victim,STAT_DEX) < 15 )
  {
      stc("���� �������� ������ ������������.\n\r",victim);
      act("�������� $c1 ������ ������������.",victim,NULL,NULL,TO_ROOM);
  }

  spell_slow(skill_lookup("slow"),ch->level*3/4,ch,vo,target);
  
  return;  
}
 
void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( ch, sn ) ) 
  { 
   if (victim == ch) stc("���� ���� � ��� ������, ��� ������.\n\r",ch); 
   else act("���� $C1 � ��� ������, ��� ������.",ch,NULL,victim,TO_CHAR);
   return;
  } 

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level + 1;
  af.location  = APPLY_AC;
  af.modifier  = -30*(int)(ch->level/10);
  af.bitvector = 0;
  affect_to_char( victim, &af );
///  if( IS_EARTH_DEITY(ch) ) change_favour(ch, 10);
  act( "���� $c2 ������������ � ������.", victim, NULL, NULL, TO_ROOM );
  stc( "���� ���� ������������ � ������.\n\r", victim );
} 
 
void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target) 
{ 
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;

  target_name = one_argument( target_name, speaker );
  speaker[0]=UPPER(speaker[0]);

  do_printf( buf1, "%s ���������� '%s'.\n\r",              speaker, target_name );
  do_printf( buf2, "����� ���������� %s ���������� '%s'.\n\r", speaker, target_name );
  buf1[0] = UPPER(buf1[0]);

  for ( vch = ch->in_room->people;vch != NULL;vch = vch->next_in_room ) 
  { 
      if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch)) 
      stc( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
///      if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 3);
  } 
} 
 
void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) || (ch!=victim && saves_spell( level, victim,DAM_OTHER))) 
  { 
    stc("�������.\n\r",ch);
    return;
  } 

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 2;
  af.location  = APPLY_STR;
  af.modifier  = -1 * (level / 5);
  af.bitvector = 0;
  affect_to_char( victim, &af );
///  if( IS_EARTH_DEITY(ch) ) change_favour(ch, 5);
///  if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 10);
  stc( "�� ����������, ��� ���� ������ �� ����.\n\r", victim );
  act("$c1 �������� ������ � ��������.",victim,NULL,NULL,TO_ROOM);
} 

void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  dam = number_range( 25, 100 );
  if ( saves_spell( level, victim, DAM_PIERCE) )  dam /= 2;
  damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, FALSE,  NULL);
} 
 
void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  dam = number_range( 30, 120 );
  if ( saves_spell( level, victim, DAM_PIERCE) ) dam /= 2;
  damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, FALSE, NULL);
} 
 
void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  OBJ_DATA *obj;
  CHAR_DATA *mob;
  AFFECT_DATA af;
  int i;

  obj = get_obj_here( ch, target_name );

  if ( obj == NULL ) 
  { 
      stc( "������� ���?\n\r", ch );
      return;
  } 

  if (IS_SET(ch->in_room->room_flags,ROOM_LAW)) 
  { 
      stc("�� � �������� ������.\n\r",ch);
      return;
   } 

  if(obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC) 
  { 
    stc( "��� ���� ������� �� �����...\n\r", ch );
    return;
  } 
  if (is_exact_name(obj->name,"zombie"))
  {
    stc( "��� ���� ������� �� �����...\n\r", ch );
    return;
  }

  if( ch->pet != NULL ) 
  { 
      stc( "� ���� ��� ���� �����.\n\r", ch );
      return;
  } 

  // add dendroid
  // Chew on the zombie a little bit, recalculate level-dependant stats
  if ((mob = create_mobile( get_mob_index( 1 ) ))==NULL) // VNUM 1 - mob zombie(LIMBO)
  {
   stc("{RBug! Unable to create mob! Report to Imms  N O W!{x\n\r",ch);
   return;
  }
  
  mob->level = obj->level;

  if (obj->item_type == ITEM_CORPSE_PC)
  {
   char buff[MAX_STRING_LENGTH];
   do_printf(buff,"%s %s",mob->short_descr,
    (obj->owner == NULL) ? "" : capitalize(obj->owner));
   free_string(mob->short_descr);
   mob->short_descr = str_dup(buff);
  }

  if (mob->level < 30) mob->max_hit =mob->level * 30 + number_range( mob->level * mob->level/4, mob->level * mob->level);
  else if (mob->level < 60) mob->max_hit =mob->level * 60 + number_range( mob->level * mob->level/2, mob->level * mob->level);
  else if (mob->level < 90) mob->max_hit =mob->level * 90 + number_range( mob->level * mob->level*2/3, mob->level * mob->level*3/2);
  else mob->max_hit =mob->level * 100 + number_range( mob->level * mob->level, mob->level * mob->level*2);
  mob->hitroll = mob->level;
  mob->damroll = (short) (mob->level*0.7);
  mob->max_hit = UMIN((int)(mob->max_hit*.9),29000);
  mob->hit                    = mob->max_hit;
  mob->max_mana               = 100 + dice(mob->level,10);
  mob->mana                   = mob->max_mana;
  for (i = 0;i < 3;i++) 
  mob->armor[i]               = interpolate(mob->level,100,-100);
  mob->armor[3]               = interpolate(mob->level,100,0);

  for (i = 0;i < MAX_STATS;i++) 
      mob->perm_stat[i] = 15 + mob->level/8;

  /* You rang? */ 
  char_to_room( mob, ch->in_room );
  act( "$i1 ������������ � ����� � ���� �������� �����!", ch, obj, NULL, TO_ROOM );
  act( "$i1 ������������ � ����� � ���� �������� �����!", ch, obj, NULL, TO_CHAR );

  extract_obj(obj);

  if (number_range(1,100) < (100-(mob->level - level)*5)) 
  { 
  // Yessssss, massssssster...  
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_PET);
    SET_BIT(mob->act, ACT_EXTRACT_CORPSE);
  // Zombie is a member of its' master clan 
    if (ch->clan!=NULL) 
    { 
     mob->clan=ch->clan;
     mob->clanrank=0;
    }
///    if( IS_DARKMAGIC_DEITY(ch) ) change_favour(ch, 10);
    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;
    /* For a little flavor... */ 
    do_say( mob, "��� � ���� ���������, ��������?" );
    return;
  } 
  else 
  { 
   SET_BIT(mob->act, ACT_AGGRESSIVE);
   SET_BIT(mob->act, ACT_EXTRACT_CORPSE);
   do_emote( mob , "������ ������ �����, �������� ��������� �������");
   if (((mob->level - ch->level)>25 || (mob->level - level)>5) && number_percent()>33) 
   {
     act("{y$c1{x ������� ���� �����, ���� ������ ������..",mob,NULL,ch,TO_VICT); 
     act("{y$c1{x � ������� ��������� ����� � ����� {y$C4{x!",mob,NULL,ch,TO_NOTVICT); 
 
     WAIT_STATE(ch, PULSE_VIOLENCE*3);

     af.where  = TO_AFFECTS;
     af.type   = skill_lookup("nostalgia");
     af.level  = mob->level;
     af.duration  = number_range(2,6);
     af.location  = APPLY_NONE;
     af.modifier  = 0;
     af.bitvector = AFF_NOSTALGIA;
     affect_to_char(mob,&af);
   }
   multi_hit(mob,ch);
   return;
  } 
} 
 
void spell_aid(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (is_affected(victim,sn))
  { 
   if (victim == ch) stc("�� ��� ������� ���� ���������.\n\r",ch);
   else  act("$C1 ��� ������� ���� ���������.",ch,NULL,victim,TO_CHAR);
   return;
  } 
 
  if ((IS_GOOD(ch) && !IS_GOOD(victim)) 
   || (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) 
   || (IS_EVIL(ch) && !IS_EVIL(victim))) 
  { 
   act("����, ������, �� ����� $C1",ch,NULL,victim,TO_CHAR);
   return;
  } 
 
  af.where      = TO_AFFECTS;
  af.type       = sn;
  af.level      = level;
  af.duration   = 4+(level>49)+(level>99);
  if (!victim->classwar && !victim->classthi)
       af.modifier= 60 + victim->max_hit / 4;
  else af.modifier= 60 + (level>10)*10 + (level>20)*10 + (level>30)*10
                       + (level>40)*10 + (level>50)*10 + (level>60)*10
                       + (level>70)*5  + (level>80)*5  + (level>90)*5;
  af.bitvector  = 0;
  af.location   = APPLY_HIT;
  affect_to_char(victim,&af);
  af.location   = APPLY_MANA;
  affect_to_char(victim,&af);
  victim->hit  += af.modifier;
  victim->mana += af.modifier;
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 2);
  stc("�� ����������, ��� ��� ��������� ����������!\n\r",victim);
  act("��������� $c2 ����������!",victim,NULL,NULL,TO_ROOM);
//double aid removed nahren
  if (victim->hit>victim->max_hit) victim->hit=victim->max_hit;
  if (victim->mana>victim->max_mana) victim->mana=victim->max_mana;
} 
 
void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if (is_affected(victim, skill_lookup("regeneration"))) 
  { 
    if (victim==ch) stc("�� ��� ��� ������������ regeneration\n\r",ch);
    else act("$C1 ��� ��� ������������ regeneration.",ch,NULL,victim,TO_CHAR);
    return;
  } 
 
  af.where     = TO_AFFECTS;
  af.type      = sn;
 
  if (IS_SET(race_table[ch->race].spec,SPEC_REGENSP)) af.level = level;
  else af.level = 1;
  if (IS_SET(race_table[ch->race].spec,SPEC_REGENSP)) 
  { 
    if (ch->race==RACE_ELF || ch->race==RACE_DROW) af.duration  = 3+level/2;
    else af.duration = 2+level/4;
  } 
  else af.duration  = UMAX(3,level/8);
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_REGENERATION;
  affect_to_char( victim, &af );
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_NATURE_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_MIGHTMAGIC_DEITY(ch) ) change_favour(ch, 5);
  ptc(victim,"�� ����������, ��� ���� ���� �������� ����������������� �������.\n\r" );
  stc("Ok.\n\r",ch);
} 
 
void spell_sunray( int sn, int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
 
  if (ch->classmag==0 && ch->classcle==0)
  {
    if (!IS_OUTSIDE(ch)) 
    { 
      stc( "�� �� ������ ������������ ��������� ���� � ���������.\n\r", ch );
      return;
    } 
    if (weather_info.sky >SKY_CLOUDY) 
    { 
      stc("������ ������ ������.\n\r", ch);
      return;
    } 
    if (weather_info.sunlight==SUN_DARK) 
    { 
     stc("���� �� �����! ���-�� ����� �����...\n\r",ch);
     return;
    } 
  }
  dam = dice(level/2, 20+category_bonus(ch,LIGHT));
  for ( vch = ch->in_room->people;vch != NULL;vch = vch_next ) 
  { 
    vch_next = vch->next_in_room;
    if (vch != ch && can_see(ch,vch,CHECK_LVL) && !is_safe_spell(ch,vch,TRUE))
    { 
      damage(ch, vch, saves_spell( level,vch,DAM_LIGHT)? dam / 2 : dam,
            sn,DAM_LIGHT,TRUE, FALSE, NULL);
///      if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 3);
///      if( IS_DARKMAGIC_DEITY(vch) ) change_favour(ch, 3);
      spell_blindness(gsn_blindness, level*2/3,ch,(void *) vch,TARGET_CHAR);
    } 
  } 
} 
      
void do_pray( CHAR_DATA *ch, const char* argument) 
{ 
  AFFECT_DATA af;
  char buf[MAX_STRING_LENGTH];
  int level = number_range(102,110), 
      lvl   = UMAX(ch->level+10,110);
  int sn=-1;
  bool ordencheck = IS_ORDEN(ch);

  CHAR_DATA *vch;
  CHAR_DATA *vch_next;


  if ( !IS_DEVOTED_ANY(ch) )
  { 
    stc("���� �� ���� ������ ��� �������� �������...\n\r",ch);
    return;
  } 

  if( EMPTY(argument) || str_prefix( argument, ch->deity) )
  {
    stc("�� �������� �������������. ������������� ������� ��� ����� ��������������.\n\r", ch);
    return;
  }

// pray near statue
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (IS_NPC(vch) && vch->pIndexData->vnum==(ch->pcdata->dn+23080))
    {
      change_favour(ch,250);
      ch->hit                 = ch->max_hit;
      ch->mana                = ch->max_mana;
      ch->move                = ch->max_move;
      update_pos(ch);
      ptc( ch, "{C%s{x ����������� ���� ����.\n\r", get_rdeity(deity_table[ch->pcdata->dn].russian,'2') );
      ptc( ch, "���� ���� � {C%s{x ������, ��� �������!\n\r", get_rdeity(deity_table[ch->pcdata->dn].russian,'2') );
      statue_moving(vch);
      return;
    }
  }

  if (ch->hit < ch->level || ch->mana < ch->level || ch->move < ch->level) 
  { 
    stc("�� ������� ������� ��� �������.\n\r",ch);
    return;
  } 
  ch->mana-=ch->level;
  ch->move-=ch->level;
  ch->hit-=ch->level;
  update_pos(ch);
  if( !IS_IMMORTAL(ch) ) WAIT_STATE(ch,PULSE_VIOLENCE);

  ptc( ch, "�� �������� {C%s{x...\n\r", get_rdeity( deity_table[ch->pcdata->dn].russian,'3') );
  do_printf( buf, "%s ������� {C%s{x...", ch->name, get_rdeity( deity_table[ch->pcdata->dn].russian,'3') );
  act( buf, ch, NULL, NULL, TO_ROOM);

  if ( is_affected(ch,skill_lookup("pray")) )
   if( ( ( (ch->classcle == 1) && !ordencheck 
    && (number_percent() < number_fuzzy(1) + 50 - get_skill(ch, gsn_pray)/2 - 3*category_bonus(ch,SPIRIT))) 
     || BAD_ORDEN(ch) )
     || ((ch->classcle != 1) && (number_percent() > ch->pcdata->favour / 7)) )
    { 
      // bad 
      if (IS_IMMORTAL(ch)) return;
      ptc( ch, "�� ��������� {C%s{x ������ ���������!\n\r", get_rdeity(deity_table[ch->pcdata->dn].russian,'2') );
      if (!is_affected(ch,skill_lookup("weaken")))
      { 
        af.where     = TO_AFFECTS;
        af.type      = skill_lookup("weaken");
        af.level     = lvl;
        af.duration  = lvl/15;
        af.location  = APPLY_STR;
        af.modifier  = -1 * (lvl / 4);
        af.bitvector = 0;
        affect_to_char( ch, &af );
        stc( "�� ����������, ��� ���� ������ �� ����.\n\r",ch);
        act("$c1 �������� ������ � ��������.",ch,NULL,NULL,TO_ROOM);
      } 
//    else if (!IS_AFFECTED(ch,AFF_CURSE) && !IS_SET(ch->imm_flags,IMM_NEGATIVE)) 
      else if (!IS_AFFECTED(ch,AFF_CURSE) ) // God Curse?Vampires?Why not? (c)Ast
      { 
        af.where     = TO_AFFECTS;
        af.type      = gsn_curse;
        af.level     = lvl;
        af.duration  = lvl/10;
        af.location  = APPLY_HITROLL;
        af.modifier  = -1 * (lvl / 7);
        af.bitvector = AFF_CURSE;
        affect_to_char( ch, &af );
        af.location  = APPLY_SAVING_SPELL;
        af.modifier  = lvl / 7;
        affect_to_char( ch, &af );
        stc( "�� ���������� ���� ���������.\n\r", ch);
      }  
      else  
      { 
        if (ch->position==POS_FIGHTING) 
        { 
          stc( "���� ������� ��������� ���� ������������...\n\r", ch );
          DAZE_STATE(ch,3*PULSE_VIOLENCE);
          WAIT_STATE(ch,PULSE_VIOLENCE);
        } 
      else 
      { 
        af.where     = TO_AFFECTS;
        af.type      = gsn_sleep;
        af.level     = lvl;
        af.duration  = 3;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = 0;
        affect_join( ch, &af );
        if (IS_AWAKE(ch)) 
        { 
          stc( "�� ���������....\n\r", ch );
          act( "$c1 ��������.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_SLEEPING;
        } 
      }   
    } 
    check_improve(ch,gsn_pray,FALSE,2);
    return;
  } 
 
  if ((!ordencheck && number_percent() > ch->level*3*get_skill(ch, gsn_pray)/100) 
    || (ordencheck && number_percent() > ch->alignment/10))
  { 
    // nothing 
    ptc( ch, "%s �� �������� �� ���� �������...\n\r",get_rdeity(deity_table[ch->pcdata->dn].russian,'1') );
    check_improve(ch,gsn_pray,FALSE,1);
    return;
  } 
  // you did it! 
  if( !IS_ELDER(ch) )
  {
    af.where        = TO_AFFECTS;
    af.type         = skill_lookup("pray");
    af.level        = ch->level;
    af.duration     = (IS_DEVOTED_ANY(ch)?0:5) + number_fuzzy(2 + ch->level / 8);
    af.location     = APPLY_NONE;
    af.modifier     = 0;
    af.bitvector    = 0;
    affect_to_char(ch,&af);
  }
///  change_favour(ch, 10);

  if( ch->classcle!=1 && !IS_ELDER(ch) ) return;

  // additional random effects for clerics
  ptc( ch, "������������� %s �������� �� ����!\n\r", get_rdeity(deity_table[ch->pcdata->dn].russian,'2') );
 
  if (ch->position==POS_FIGHTING && ch->fighting!=NULL) 
  { 
   switch(number_range(0,7)) 
   { 
    case 0: 
            if (ch->alignment>350 && ch->fighting->alignment<-350)
              sn=skill_lookup("ray of truth");
            else if (ch->alignment<-350 && ch->fighting->alignment>350)
              sn=skill_lookup("demonfire");
            else sn=skill_lookup("flamestrike");
            break;
    case 2: 
            sn=gsn_curse;
            break;
    case 5: 
            sn=skill_lookup("blind");
            break;
    case 7: 
            sn=skill_lookup("cause critical");
            break;
    default: 
            break;
   } 
   if (sn!=-1) skill_table[sn].spell_fun(sn,level,ch,ch->fighting,TARGET_CHAR);
   WAIT_STATE(ch,PULSE_VIOLENCE);
  } 
  else 
  { 
   switch(number_range(0,31)) 
   { 
    case 0:  //sanctuary 
             sn=skill_lookup("sanctuary");
             break;
    case 1:  //bless 
             sn=skill_lookup("bless");
             break;
    case 3: 
    case 30: 
             // heal 
             sn=skill_lookup("heal");
             break;
    case 6:  // haste 
             sn=skill_lookup("haste");
             break;
    case 8: 
    case 28: 
             //refresh 
             sn=skill_lookup("refresh");
             break;
    case 12: //shield 
             sn=skill_lookup("shield");
             break;
    case 15: //stone skin 
             sn=skill_lookup("stone skin");
             break;     
    case 17: 
    case 25: 
             //armor 
             sn=skill_lookup("armor");
             break;
    case 19: //prot.good 
             if (ch->alignment<-350) sn=skill_lookup("protection good");
             break;
    case 20: //prot.evil 
             if (ch->alignment>350) sn=skill_lookup("protection evil");
             break;
    case 13: 
    case 22: 
             //giant strength 
             sn=skill_lookup("giant strength");
             break;
    case 23: //stone skin 
             sn=skill_lookup("stone skin");
             break;
    case 31: //frenzy 
             sn=skill_lookup("frenzy");
             break;
    default: 
             break;
   } 
   if (sn!=-1) skill_table[sn].spell_fun(sn,level,ch,ch,TARGET_CHAR);
  } 
  check_improve(ch,gsn_pray,TRUE,3);
} 
 
void spell_observation( int sn, int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  AFFECT_DATA *paf, *paf_last = NULL;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *room;
 
  if (number_percent() > get_skill(ch, skill_lookup("observation"))*7/8) 
  { 
    stc("� ���� ������ �� �����.\n\r",ch);
    return;
  } 

  if (target==TARGET_CHAR)
  {
    victim = (CHAR_DATA *) vo;    
    if ( victim->affected != NULL ) 
    { 
      ptc(ch,"%s ��������� ��� �������� ��������� ����������:\n\r",victim->name);
///      if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 7);

     for (paf = victim->affected; paf != NULL; paf = paf->next)
     {
        if (paf->location==APPLY_SPELL_AFFECT)
        {
         ptc(ch,"{CSpellaffect:{x %-15s\n\r",affect_bit_name(paf->modifier));
         continue;
        }
        else 
        {
         if (paf_last && paf->type == paf_last->type) 
         stc("                            ",ch);
         else ptc(ch, "���������� : %-15s", skill_table[paf->type].name);
   
         if (level+number_range(0,5) > paf->level)
         {
          ptc(ch, ": �������� %s �� %d ", affect_loc_name(paf->location),paf->modifier);
          if (paf->duration == -1) stc("���������",ch); else ptc(ch, "�� %d �����", paf->duration);
         }
         stc("\n\r", ch);
        }
       paf_last = paf;
     }
      
    } 
    else act("$C1 �� ��������� ��� ������������ ����������.",ch,NULL,victim,TO_CHAR);
  }
  if (target==TARGET_ROOM)
  {
    RAFFECT *ra;
    bool found=FALSE;
    room = (ROOM_INDEX_DATA *) vo;

    if (IS_SET(room->room_flags,ROOM_NOMORPH))   {found=TRUE; stc ("�� ���������� ���������� ������������ �����������\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_SAFE))      {found=TRUE; stc ("�� ���������� ������ �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_NOFLEE))    {found=TRUE; stc ("�� �� ������ ������� ������ � ��������\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_MAG_ONLY))  {found=TRUE; stc ("������ ��� ����� �������� ��� �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_WAR_ONLY))  {found=TRUE; stc ("������ ���� ����� �������� ��� �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_THI_ONLY))  {found=TRUE; stc ("������ ��� ����� �������� ��� �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_CLE_ONLY))  {found=TRUE; stc ("������ ��������� ����� �������� ��� �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_ALL_VIS))   {found=TRUE; stc ("����� �� ����� ���� ����� � ���� �����\n\r",ch);}
    if (IS_SET(room->room_flags,ROOM_NO_RECALL)) {found=TRUE; stc ("�� ���������� ��������� �����\n\r",ch);}
    if (ch->in_room->raffect)
    {
      found=TRUE;
      ptc(ch,"�� ���������� ������� �����:\n\r��. ����� ����������\n\r");
      for (ra=room->raffect;ra;ra=ra->next_in_room)
      {
        if (level+10 > ra->level)
             ptc(ch,"{D[{C%3d{D] {Y%4d   {G%s{x\n\r" ,ra->level,ra->duration,raff_name(ra->bit));
        else ptc(ch,"{D[   ]        {G%s{x\n\r" ,raff_name(ra->bit));
      }
///     if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 5);
///     if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 2);
    }
    if (room->heal_rate!=100 || IS_SET(room->ra,RAFF_OASIS))
    {
      ptc(ch,"�������������� ��������: %d%%\n\r",room->heal_rate+(IS_SET(room->ra,RAFF_OASIS)?100:0));
      found=TRUE;
    }
    if (room->mana_rate!=100 || IS_SET(room->ra,RAFF_MIND_CH))
    {
      ptc(ch,"�������������� ������� : %d%%\n\r",room->mana_rate+(IS_SET(room->ra,RAFF_MIND_CH)?100:0));
      found=TRUE;
    }
    if (!found) stc("�� �� ���������� ������ ���������� � ���� �����.\n\r",ch);
  }
} 

void spell_spirit_lash(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam=(level>59)? dice(level/2-15,18) :
         ((level>33)? dice(level/2,9) : dice(level/3+1,9));
  dam+=dam*get_int_modifier(ch,victim,skill_lookup("spirit lash"),DAM_MENTAL)/100;
  damage( ch, victim, dam, sn, DAM_MENTAL ,TRUE, FALSE, NULL);
///  if( IS_NATURE_DEITY(ch) ) change_favour(ch, 3);
///  if( IS_LIGHTMAGIC_DEITY(ch) ) change_favour(ch, 1);
} 

void do_heal(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *mob;
  char arg[MAX_INPUT_LENGTH];
  int sn;
  int64 cost;
  SPELL_FUN *spell;
  char *words;
  int targ=TARGET_CHAR;//specially for cancellation        

  for ( mob = ch->in_room->people;mob;mob = mob->next_in_room )
  {
    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) ) break;
  }

  if ( mob == NULL )
  {
    stc( "��� ��� ������.\n\r", ch );
    return;
  }

  if (IS_SET(ch->act,PLR_TIPSY)) if (tipsy(ch,"heal")) return;

  one_argument(argument,arg);

  if (arg[0] == '\0')
  {
    act("$C1 ���������� '{G��� ��� � ���� ���� ��������� ��� ������� �����:{x'",ch,NULL,mob,TO_CHAR);
    stc("  {Wlight{x  : ������ ������ �������       {Y10 gold{x\n\r",ch);
    stc("  {Wserious{x: ������ ��������� �������    {Y15 gold{x\n\r",ch);
    stc("  {Wcritic{x : ������ ����������� �������  {Y25 gold{x\n\r",ch);
    stc("  {Wheal{x   : ������������ ����������     {Y50 gold{x\n\r",ch);
    stc("  {Wblind{x  : ������ �������              {Y20 gold{x\n\r",ch);
    stc("  {Wdisease{x: ������ ����                 {Y15 gold{x\n\r",ch);
    stc("  {Wpoison{x : ������ ����������           {Y25 gold{x\n\r",ch);
    stc("  {Wuncurse{x: ����� ���������             {Y50 gold{x\n\r",ch);
    stc("  {Wrefresh{x: ������������ ��������       {Y 5 gold{x\n\r",ch);
    stc("  {Wmana{x   : ������������ ����           {Y10 gold{x\n\r",ch);
    stc("  {Wrestore{x: �������������              {Y250 gold{x\n\r",ch);
    stc("  {Wcancell{x: ����� �����                 {Y30 gold{x\n\r",ch);
    stc("  {Wrelease{x: ������ �������� �� �����  {Y1000 gold{x\n\r",ch);
    stc(" ������ {Wheal <���>{x ��� ���������.\n\r",ch);
    return;
  }

  if ( !IS_NPC(ch) && ch->clan != NULL && IS_SET(ch->clan->flag, CLAN_WARRIOR)  && ch->in_room->vnum != 80000 )
   {
    stc("������ ������-������� ������ ������ ����.\n\r",ch);
    return;
   }

  if (!str_prefix(arg,"light"))
  {
    spell = spell_cure_light;
    sn    = skill_lookup("cure light");
    words = "judicandus dies";
    cost  = 1000;
  }
  else if (!str_prefix(arg,"cancell")) // heal cancel do not check anticancellation
  {
    spell = spell_cancellation;
    sn    = skill_lookup("cancellation");
    words = "qaiqzrrahuai";
    cost  = 3000;
    targ  = 100;
  }
  else if (!str_prefix(arg,"serious"))
  {
    spell = spell_cure_serious;
    sn    = skill_lookup("cure serious");
    words = "judicandus gzfuajg";
    cost  = 1500;
  }
  else if (!str_prefix(arg,"critical"))
  {
    spell = spell_cure_critical;
    sn    = skill_lookup("cure critical");
    words = "judicandus qfuhuqar";
    cost  = 2500;
  }
  else if (!str_prefix(arg,"heal"))
  {
    spell = spell_heal;
    sn = skill_lookup("heal");
    words = "pzar";
    cost  = 5000;
  }
  else if (!str_prefix(arg,"blindness"))
  {
    spell = spell_cure_blindness;
    sn    = skill_lookup("cure blindness");
    words = "judicandus noselacri";        
    cost  = 2000;
  }
  else if (!str_prefix(arg,"disease"))
  {
    spell = spell_cure_disease;
    sn    = skill_lookup("cure disease");
    words = "judicandus eugzagz";
    cost = 1500;
  }
  else if (!str_prefix(arg,"poison"))
  {
    spell = spell_cure_poison;
    sn    = skill_lookup("cure poison");
    words = "judicandus sausabru";
    cost  = 2500;
  }
  else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
  {
    spell = spell_remove_curse;
    sn    = skill_lookup("remove curse");
    words = "candussido judifgz";
    cost  = 5000;
  }
  else if (!str_prefix(arg,"mana"))
  {
    spell = NULL;
    sn = -1;
    words = "energizer";
    cost = 1000;
  }
  else if (!str_prefix(arg,"restore"))
  {                                                                               
    spell = NULL;
    sn = -1;
    words = "judicanudus energizer";
    cost = 25000;
  }
  else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
  {
    spell = spell_refresh;
    sn    = skill_lookup("refresh");
    words = "candusima";
    cost  = 500;
  }
  else if (!str_prefix(arg,"release"))
  {                                                                               
    if (IS_NPC(ch)) return;
    spell = NULL;
    sn = -1;
    words = "judicanudus nirgwedyxjwe";
    cost = 100000;
  }
  else 
  {
    act("$C1 ���������� '������ 'heal' ��� ������ ���������, ������.'",ch,NULL,mob,TO_CHAR);
    return;
  }

  if (cost > (ch->gold * 100 + ch->silver))
  {
    act("$C1 ���������� '������� ����� �������.'",ch,NULL,mob,TO_CHAR);
    return;
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);

  deduct_cost(ch,cost);
  mob->gold += cost / 100;
  mob->silver += cost % 100;
  act("$c1 �������� '$T'.",mob,NULL,words,TO_ROOM);

  if (spell==NULL)
  {
    if (!str_prefix(arg,"restore"))
    {
      ch->mana += dice(20,40) + mob->level / 3;
      ch->mana = UMIN(ch->mana,ch->max_mana);
      ch->hit  += dice(10,15) + mob->level / 3;
      ch->hit  = UMIN(ch->hit,ch->max_hit);
      stc("����� �������� ������ ���� ������ �������.\n\r",ch);
      stc("�� ����������, ��� ����� ��������� � ����.\n\r",ch);
      return;
    }

    if (!str_prefix(arg,"mana"))
    {
      ch->mana += dice(2,8) + mob->level / 3;
      ch->mana = UMIN(ch->mana,ch->max_mana);
      stc("����� �������� ������ ���� ������ �������.\n\r",ch);
      return;
    }

    if (!str_prefix(arg,"release"))
    {
      if (ch->pcdata->condition[COND_ADRENOLIN] == 0)
      {
        while ( ch->affected )
        affect_remove( ch, ch->affected );
        ch->affected_by = race_table[ch->race].aff;
        stc("������ �� ������. ������ ����.\n\r",ch);
        WAIT_STATE(ch,5*PULSE_VIOLENCE);
      }
      else act("$C1 ���������� '�� ������� ���-��. ���������, ������� �����.'",ch,NULL,mob,TO_CHAR);
      return;
    }
 }
 if (sn == -1) return;
 spell(sn,mob->level,mob,ch,targ);
}

int cast_rate (int value)
{
 if (value <= 21) return -4;
 if (value <= 22) return -3;
 if (value <= 23) return -2;
 if (value <= 24) return -1;
 if (value <= 25) return  0;
 if (value <= 26) return  1;
 if (value <= 27) return  2;
 if (value <= 28) return  3;
 if (value <= 29) return  3;
 if (value <= 30) return  4;
                  return  5;
}

void do_charge( CHAR_DATA *ch, const char *argument )
{
  int sn,max=3;

  sn=skill_lookup(argument);
  
  if ( !get_skill(ch,gsn_charge) || !ch->classmag || IS_NPC(ch) )
  {
      stc("���?",ch);
      return;
  }

  if (!argument || !argument[0])
  {
      stc("Syntax: charge <spell>\n\r",ch);
      return;
  }
  
  
  if ( sn == -1 || !IS_SET(skill_table[sn].group,SPELL) )
  {
      ptc(ch,"�� �� ������ ���������� \'%s\'\n\r",argument);
      return;
  }

  if ( IS_SET(skill_table[sn].flag, S_CLAN) )
  {
      stc("�� �� ������ ������������������ �� �������� ����������.\n\r",ch);
      return;
  }
  
  if ( ch->mana < ( skill_table[sn].min_mana*2 ) + ch->level )
  {
      stc("� ���� �� ������� ����\n\r",ch);
      return;
  }
  
  if ( number_percent() > get_skill(ch,gsn_charge) )
  {
      stc("�� ������� ������������...\n\r",ch);
      check_improve(ch,gsn_charge,FALSE,1);
      WAIT_STATE(ch,skill_table[sn].beats);
      ch->mana -= ((( skill_table[sn].min_mana*2 ) + ch->level)/2);
      return;
  }
  
  if ( ch->pcdata->charged_spell != sn ) ch->pcdata->charged_num=0;
  ch->pcdata->charged_spell=sn;
  
  if (ch->level>min_level(ch,sn)*2) max++;
  if (ch->level>min_level(ch,sn)*3) max++;
  if (IS_SET(skill_table[sn].flag,C_NODUAL) || sn==gsn_charm_person) max=1;

  ch->pcdata->charged_num=UMIN(max,ch->pcdata->charged_num+1);

  ch->mana -= ( skill_table[sn].min_mana*2 ) + ch->level;
  ptc(ch,"�� ���������������� �� ���������� {R\'%s\'{x\n\r",skill_table[sn].name);
  check_improve(ch,gsn_charge,TRUE,1);
///  if( IS_MAGIC_DEITY(ch) ) change_favour(ch, 7);
///  if( IS_INTELLIGENT_DEITY(ch) ) change_favour(ch, 5);
///  if( IS_ENCHANT_DEITY(ch) ) change_favour(ch, 2);
  WAIT_STATE(ch,skill_table[sn].beats);
  return;
}
