/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------
  Zeitgesteuerte Job-Triggerung
  -----------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980212 OE3DZW delay before start of cron-daemon removed
//19980614 OE3DZW define as limit for disable
//20030706 DB1RAS nice, user-friendly default crontab file

#include "baycom.h"

#ifdef __DOS16__
extern time_t isdst (void);
#endif

/*---------------------------------------------------------------------------*/

static char cron_init[]=
";------------------------------------------\n"
";minute   hour  day month weekday   command\n"
";------------------------------------------\n"
"*            *    *     *       *   minute\n"
"*/5          *    *     *       *   fiveminute\n"
"0,15,30,45   *    *     *       *   quarter\n"
"0,30         *    *     *       *   halfhour\n"
"1,31         *    *     *       *   beacon\n"
"3,33         *    *     *       *   forward\n"
#ifdef _AUTOFWD
"35           1    *     *       *   afwdlist\n"
#endif
"5            2    *     *       *   postfwd\n"
"5            3    *     *       *   purge\n"
"35           3    *     *       *   save\n"
"5            4    *     *       0   reorg\n";


/*---------------------------------------------------------------------------*/

static int near cron_match (char *s, int si)
//*************************************************************************
//
//  test if given pattern matches the current time
//
//*************************************************************************
{
  char *c;

  c = s;
  if (*c == '*' && ! *(c+1)) return 1; // match everytime
  if (*c == '*'
      && *(c+1) == '/'
      && isdigit(*(c+2))
      && atoi(c+2) > 0)                // check for division
  {
    if ((si % atoi(c+2)) == 0) return 1;
  }
  while (isdigit(*c)) c++;             // remember next non-decimal
  if (*c == '-')                       // check for range
    return (atoi(s) <= si && atoi(c + 1) >= si) || cron_match(c + 1, si);
  if (*c == ',')                       // check for enumeration
    return (atoi(s) == si || cron_match(c + 1, si));
  if (c != s) return (atoi(s) == si);  // begin of string is decimal
  return 0;
}

/*---------------------------------------------------------------------------*/

static void near run_crontab (struct tm *tt)
//****************************************************************************
//
//****************************************************************************
{
  FILE *f;
  char s[200];
  char mi[51], h[51], d[51], mo[51], wd[51], script[51];
  int line = 0;

  f = s_fopen(CRONTABNAME, "srt");
  if (   cron_match("0", tt->tm_min)
      && cron_match("0", tt->tm_hour)
      && cron_match("*", tt->tm_mon + 1)
      && cron_match("1", tt->tm_mday)
      && cron_match("*", tt->tm_wday))
    sysop_sysinfomail();

  if (! f) // if crontab does not exist, create it
  {
    f = s_fopen(CRONTABNAME, "sw+t");
    if (! f) return; // file is not accessible (why ever...)
    fwrite(cron_init, sizeof(cron_init) - 1, 1, f);
    trace(replog, "cron", "crontab.bcm generated");
    rewind(f);
  }
  while (fgets(s, sizeof(s), f))
  {
    line++; // remember line for error message
    // recognise comment lines
    if ((*s != '*' && ! isdigit(*s)) || strlen(s) < 10) continue;
    if (sscanf(s, "%50s %50s %50s %50s %50s %50s\n", mi, h, d, mo, wd, script) == 6)
    {
      if (   cron_match(mi, tt->tm_min)
          && cron_match(h,  tt->tm_hour)
          && cron_match(mo, tt->tm_mon + 1)
          && cron_match(d,  tt->tm_mday)
          && cron_match(wd, tt->tm_wday))
      { //oe3dzw debug
        //trace(report, "run_crontab", "starting %s", script);
        if (! stricmp(script, "beacon"))
          fork(P_BACK | P_MAIL, 0, sendmailbake, "Beacon");
        else if (! stricmp(script, "purge"))
          fork(P_BACK | P_MAIL, 0, mbpurge, "Purge");
        else if (! stricmp(script, "forward"))
          fork(P_BACK, 0, startfwd, "Forward");
        else if (! stricmp(script, "postfwd"))
          fork(P_BACK | P_MAIL, 0, postfwd, "Postfwd");
        else if (stristr(script, "reorg") == script)
        {
          char *cmd;
          cmd = script + 5;
          while (*cmd == ' ') cmd++;
          fork(P_BACK | P_MAIL, 0, mbreorg, cmd);
        }
#ifdef _AUTOFWD
        else if (! stricmp(script, "afwdlist"))
          fork(P_BACK | P_MAIL, 0, afwdlist, "Afwdlist");
#endif
        else runbatch(script);
      }
    }
    else trace(serious, "crontab", "syntax line %d", line);
  }
  s_fclose(f);
}

/*---------------------------------------------------------------------------*/

static void near cron_hour (void)
//****************************************************************************
//
//****************************************************************************
{
  if (hd_space(0L) && ! m.disable)
  {
    trace(serious, "bcm home", "disk full");
    m.disable = 1;
  }
  if (dfree(m.userpath, 0) < USERDISKFULL)
  {
    trace(serious, "user path", "disk full");
    m.disable = 1;
  }
  if (dfree(m.infopath, 0) < INFODISKFULL)
  {
    trace(serious, "info path", "disk full");
    m.disable = 1;
  }
  if (mbhadrok(m.boxadress) != 1)
  {
    trace(serious, "cron", "no valid H-boxaddress %s", m.boxadress);
    m.disable = 1;
  }
  //oe3dzw Test auf .# //regional address
  if ((strstr(m.boxadress, ".DEU") || strstr(m.boxadress, ".AUT"))
      && ! strstr(m.boxadress, ".#"))
  {
    trace(serious, "cron", "no '.#'in boxaddress %s", m.boxadress);
    m.disable = 1;
  }
  //oe3dzw Test auf kein EURO
  if (strstr(m.boxadress, ".EURO"))
  {
    trace(serious, "cron", ".EURO in boxaddress %s", m.boxadress);
    m.disable = 1;
  }
  if (ad_time() < 842639000L)
  {
    trace(serious, "cron", "date setup incorrect");
    m.disable = 1;
  }
  if (! file_isreg(CHECKNUMNAME) && file_isreg(CHECKNAME))
    fork(P_BACK | P_MAIL, 0, mbreorg, "n");
}

/*---------------------------------------------------------------------------*/

void mbcron (char *)
//****************************************************************************
//
//****************************************************************************
{
  int oldmin = 0;
  int oldhour = 25;
  struct tm *tt;
  struct tm tmbuf;
#ifdef __DOS16__
  struct time tdos;
  long olddst = isdst();
  long thisdst;
#endif

  wdelay(685);
  testcputask();
  runbatch("startup");
  trace(report, "mbcron", "started");
  while (! runterfahren)
  {
    wdelay(2514);
    tt = ad_comtime(ad_time() + ad_timezone()); //local time
    memcpy(&tmbuf, tt, sizeof(struct tm));
    tt = &tmbuf;
    if (tt->tm_hour != oldhour)
    {
      if (! tt->tm_hour && oldhour != 25) // new local day
#ifdef __DOS16__
      if (oldhour != 25 && ! m.pcisutc && (thisdst = isdst()) != olddst)
      {
        gettime(&tdos);
//      tdos.ti_hour += (olddst + thisdst) / HOUR;
        tdos.ti_hour += thisdst ? 1 : -1;
        settime(&tdos);
        trace(replog, "mbcron", "new daylightsaving: %lds", thisdst);
        olddst = thisdst;
      }
      else olddst = isdst();
#endif
      oldhour = tt->tm_hour;
      cron_hour();
    }
    if (tt->tm_min != oldmin)
    {
      oldmin = tt->tm_min;
#ifdef _AX25K
      ax25k_reinit();
#endif
      run_crontab(tt);
      if ((oldmin % 10) == 7) testcputask();
      msg_dealloc(0); // purge unused messages
    }
    test_killtime();  // callback to scheduler
  }
}

/*---------------------------------------------------------------------------*/
