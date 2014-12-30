/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------------------------
  Procedures for generating and calculating passwords
  ---------------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

static char *pwtype[] = { "BayCom",
#ifdef FEATURE_MDPW
                       "MD2", "MD5",
#else
                       "inactive", "inactive",
#endif
                       "" };

/*---------------------------------------------------------------------------*/

// Moved to this position by DH3MB (combining procedures from sysop.cpp
// and fwd.cpp)

/*---------------------------------------------------------------------------*/

void genPWstring (char *pw, char *pwstring, char *result)
//*************************************************************************
//
//  Puts a string with five random numbers to *pwstring and stores
//  the according password in *result
//
//*************************************************************************
{
  int pwlen;
  int history[5] = { 1000, 1000, 1000, 1000, 1000 };
  int a, i, j;

  pwlen = strlen(pw);
  *pwstring = 0;
  for (i = 0; i < 5; i++)
  {
    do
    {
      a = random_max(pwlen) + 1;
      for (j = 0; j < 5; j++)
        if (history[j] == a && pwlen >= 5) a = 1000;
        history[i] = a;
    } while (a == 1000);
    sprintf(pwstring +strlen(pwstring), " %d", a);
    result[i] = pw[a - 1];
  }
  result[5] = 0;
}

/*---------------------------------------------------------------------------*/

char maxpwtype ()
//*************************************************************************
//
//  Returns the number of password-types supported
//
//*************************************************************************
{
  int num = 0;
  while (pwtype[num]) num++;
  return num;
}

/*---------------------------------------------------------------------------*/

char *pwtypestr (char pwnum)
//*************************************************************************
//
//  Returns a pointer to the description of a password-type
//
//*************************************************************************
{
  if (pwnum == 100) return "inactive";
  return pwtype[pwnum];
}

/*---------------------------------------------------------------------------*/

char pwtypenum (char *pwstr)
//*************************************************************************
//
//  Returns a pointer to the description of a password-type
//
//*************************************************************************
{
  char num = 0;

  if (! *pwstr) return 101;
  if (! stricmp(pwstr, "OFF") || ! stricmp(pwstr, "INACTIVE")) return 100;
  while (stricmp(pwstr, pwtype[num]))
    if (! pwtype[num++][0]) return 101;
  return num;
}

/*---------------------------------------------------------------------------*/

int pwauswert (char *pw)
//*************************************************************************
//
//  Sendet 5 Zahlen und erwartet einen String mit 5 Buchstaben,
//  der mit dem uebergebenen String zusammenpasst.
//
//*************************************************************************
{
  char request[30];
  char result[6];

  genPWstring(pw, request, result);
  putf("%s> %s\n", m.boxname, request);
  getline(b->line, BUFLEN - 1, 1); // accept up to 20 result sequences
  return !! strstr(b->line, result);
}

/*---------------------------------------------------------------------------*/

int getpw (void)
//*************************************************************************
//
//*************************************************************************
{
  char pwbuf[100];
  FILE *f;

  strcpy(pwbuf, "");
  *pwbuf = 0;
  if ((f = s_fopen(PASSWDNAME, "srt")) != NULL)
  {
    fgets(pwbuf, sizeof(pwbuf) - 1, f);
    subst(pwbuf, LF, 0); // dh3mb
    subst(pwbuf, CR, 0);
    if (strlen(pwbuf) > 80) // debug oe3dzw
    {
      trace(serious, "getpw", "sysop pw %d chars", strlen(pwbuf));
      pwbuf[80] = 0;
    }
    s_fclose(f);
  }
  if (! *pwbuf) return 0;
  if (pwauswert(pwbuf))
  {
    b->sysop = 1;
    cmdlog("*** PW ok");
    pwlog(b->logincall, b->uplink, "sysop ok");
    testdfull();
  }
  else
  {
    cmdlog("*** PW FAILURE ***");
    pwlog(b->logincall, b->uplink, "sysop failure");
  }
  return b->sysop;
}

/*---------------------------------------------------------------------------*/

int getpriv (char *befbuf, char *privname)
//*************************************************************************
//
//*************************************************************************
{
  char pwcmp[5];
  FILE *f;
  struct tm *tt;

  if (strlen(befbuf) == 4)
  {
    if ((f = s_fopen(privname, "srt")) != NULL)
    {
      tt = ad_comtime(b->logintime); // UTC
      fseek(f, ((tt->tm_min + tt->tm_mday) % 60) * 27 + tt->tm_hour, SEEK_SET);
      fread(pwcmp, 4, 1, f);
      pwcmp[4] = 0;
      s_fclose(f);
    }
    if (! strcmp(pwcmp, befbuf)) return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/
