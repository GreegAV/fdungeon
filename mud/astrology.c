// $Id: astrology.c,v 1.6 2002/01/02 13:06:31 saboteur Exp $
// Copyrights (C) 1998-2001, Forgotten Dungeon team.
// Read ours copyrights and license terms in 'license.fd'

���� ��� �����:
 ������ ������������� ���:
 ���� (����� 24, � ������� �������)
 ������ (12, �� �������)
 ��� (���������� ��� ��� � ������ ������, ������� ���������� ����������� ����)
 ���� ���� (� ���� ������)

��� ����� ������ �������������� ����������. �� ���� ���� ���������� ��� 199x �� � � ����
������ ���� ������ ��

��������� date
 int day (1-28..31)
 int month (12)_
 int year (xx)
 int moonphase (28 phases)

char: ->pcdata->bd
���� �������� ����.
 int day
 int month
 int year
 int hour

 int element (air,earth,fire,water)
����� ������� ������� �� 4 �������� (�� ��� ����� �������)
���������� ����� ������� - ����, �� �� ����-�� �������.
�� ������ ������ � ��������� ������� ���� ��� ���� �������, � ������ ���������� ��� ����
���� �������, �������� ������ ���.

struct zodiac_data
{
  const char * name;
  const char * ename;
  int bday;
  int bmonth;
  int eday;
  int emonth;
  int element;
  int64 bonus;
};

struct zodiac_data zodiac_table[] =
{
//  name       ename         bday bmonth eday emonth element         bonus
  { "����",    "aries",      21,  03,    20,   04,   ELEMENT_FIRE,   BONUS_STR},
  { "�����",   "taurus",     21,  04,    20,   05,   ELEMENT_EARTH,  BONUS_STR},
  { "��������","gemini",     21,  05,    21,   06,   ELEMENT_AIR,    BONUS_STR},
  { "���",     "cancer",     22,  06,    22,   07,   ELEMENT_WATER,  BONUS_STR},
  { "���",     "leo",        23,  07,    23,   08,   ELEMENT_,       BONUS_STR},
  { "����",    "virgo",      24,  08,    23,   09,   ELEMENT_,       BONUS_STR},
  { "����",    "libra",      24,  09,    23,   10,   ELEMENT_AIR,    BONUS_STR},
  { "��������","scorpio",    24,  10,    22,   11,   ELEMENT_WATER,  BONUS_STR},
  { "�������", "saggitarius",23,  11,    21,   12,   ELEMENT_,       BONUS_STR},
  { "�������", "capricorn",  22,  12,    20,   01,   ELEMENT_,       BONUS_STR},
  { "�������", "aquarius",   21,  01,    20,   02,   ELEMENT_WATER,  BONUS_STR},
  { "����",    "pisces",     21,  02,    20,   03,   ELEMENT_AIR,    BONUS_STR},
  { "end",     "end",         1,   1,     1,    1,   0,              0}
};

struct date_data
{
  int month;
  int mdays;
  char *name;
  char *ename;
}

struct date_data month_table[]=
{
//  month mdays name        ename
  {  1,   31,   "������",   "January"   },
  {  2,   29,   "�������",  "February"  },
  {  3,   31,   "����",     "March"     },
  {  4,   30,   "������",   "April"     },
  {  5,   31,   "���",      "May"       },
  {  6,   31,   "����",     "June"      },
  {  7,   30,   "����",     "July"      },
  {  8,   31,   "������",   "August"    },
  {  9,   30,   "��������", "September" },
  { 10,   31,   "�������",  "October"   },
  { 11,   30,   "������",   "November"  },
  { 12,   31,   "�������",  "December"  }
}

struct year_data
{
  int year;
  char *name;
  char *rname1;
  char *rname2;
}

struct year_data year_table[]=
{

  { 1920, "monkey",  "��������", "��������" },
  { 1921, "Cock",    "�����",    "������"   },
  { 1922, "dog",     "������",   "������"   },
  { 1923, "boar",    "�����",    "������"   },
  { 1924, "rat",     "�����",    "�����"    },
  { 1925, "bullock", "���",      "����"     },
  { 1926, "tiger",   "����",     "�����"    },
  { 1927, "rabbit",  "������",   "�������"  },
  { 1928, "dragon",  "������",   "�������"  },
  { 1929, "snake",   "����",     "����"     },
  { 1930, "horse",   "������",   "������"   },
  { 1931, "sheep",   "����",     "����"     }
}
