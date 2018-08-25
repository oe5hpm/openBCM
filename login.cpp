/***************************************************************

  BayCom(R)   Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------
  Alles beim Login und Logout
  ---------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980121 OE3DZW Added Logbook for http-access
//19980124 OE3DZW Removed Logbook for 0 bytes RX (=http-access),
//                it added Flexnet-link-checks...
//19980202 OE3DZW added support for CB (callformat)
//19980329 OE3DZW added log for http again
//19980404 hrx    support for tty-fwding
//19980404 hrx    support for guests
//19980408 hrx    added autotrcwin conditional
//19980408 OE3DZW fixed notvisible, string could become too long
//19980416 hrx    added asklogin(). added condition for asklogin()
//                to be called in mblogin()
//19980830 OE3DZW removed check for m.userpw at login
//19980830 OE3DZW corrected flag for pop/smtp login (was sysop before)
//19980914 OE3DZW guest are never sysop, mblogin
//19990206 OE3DZW added fspath at login
//19990423 OE3DZW fixed mblogout, caused watchdog-reset at db0aab
//19990621 OE3DZW fixed log command for 9axxx (call starting with number)
//19991028 Jan    added defstatus, clean up guest,asklogin
//19991128 Jan    new flag b->via_radio
//                no maxlogin for non-radio logins (HTTP,POP3,telnet ...)
//20000101 dk2ui  (dzw) initialize last-dir-news on login
//20000116 DK2UI  (dzw) added ui-options
//19991219 DF3VI  added (discon) in mblogout()
//20000524 DK2UI  putlogauszug() limit line length to 80 byte
//20021127 hpk    added CB-BCMNET user-login concept
//20021210 hpk    is the users paclen-value out of the sysops given range
//                (minpaclen, maxpaclen)
//                the users paclen-value will be set to paclen
//20030809 hpk    asklogin must not be active if user is a guest (endless loop)
//20040323 DH8YMB LOG Befehl ueberarbeitet, Zusammenfassung mit LOG -Z hinzu

#include "baycom.h"

static char *logkopf =
#if defined __DOS16__ && defined OLDTIMEFMT
"Call   Date     Start  End  TxBytes RxBytes CPUsec F  TXF  RXF  via\n";
#else
"Call   Date     Start End    TxBytes RxBytes CPU s F TXF RXF  via\n";
#endif
/*---------------------------------------------------------------------------*/

static char *msgpath (char *name, char *land)
//*************************************************************************
//
//  Setzt einen Filepfad fuer ein sprachabhaengiges Textfile zusammen
//
//*************************************************************************
{
  static char buf[20];

  strcpy(buf, MSGPATH "/");
  strcat(buf, name);
  strcat(buf, ".");
  strcat(buf, land);
  strlwr(buf);
  return buf;
}

/*---------------------------------------------------------------------------*/

int readtext (char *text)
//*************************************************************************
//
//  Gibt einen Text abhaengig von der Landessprache aus
//
//*************************************************************************
{
  lastfunc("readtext");
  char buf[50];
#ifdef __FLAT__
  char buf2[(2*LINELEN)+1];
  buf2[0] = 0;
  unsigned int i = 0;
#endif
  int a, last = 0;
  char *msg;
  int prozent = 0;
  FILE *f;

  f = s_fopen(msgpath(text, ms(m_helpext)), "lrt");
  if (! f) f = s_fopen(msgpath(text, "GB"), "lrt");
  if (! f) f = s_fopen(msgpath(text, "DL"), "lrt");
  if (f)
  {
    while ((a = fgetc(f)) != EOF)
    {
      last = a;
#ifdef __FLAT__
      if (prozent)
      {
        msg = chartomakro(a, buf);
        strcat(buf2, msg);
        prozent = 0;
      }
      else if (a == '%') prozent = 1;
           else sprintf(buf2, "%s%c", buf2, a);
      if (last == LF)
      {
        if (b->http > 0) rm_esc(buf2);
        for (i = 0 ; i < strlen(buf2); i++)
          putv(buf2[i]);
        buf2[0] = 0;
      }
#else
      if (prozent)
      {
        msg = chartomakro(a, buf);
        putf("%s", msg);
        prozent = 0;
      }
      else if (a == '%') prozent = 1;
      else putv(a);
#endif
    }
    if (last != LF) putv(LF);
    s_fclose(f);
    return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

static void near logbuch (void)
//*************************************************************************
//
//  Schreibt einen Eintrag ins Logbuch
//
//*************************************************************************
{
  lastfunc("logbuch");
  FILE *logf;
  char logname[20];
  //char dat[10];
  char flag = ' ';
  time_t ti = ad_time();

  if (u->lastquit > (ti - 8)) flag = 'Q';
  if (b->forwarding == fwd_standard) flag = 'F';
  if (b->forwarding == fwd_user) flag = 'U';
  if (! strcmp (b->uplink, "SMTP")) flag = 'E';
  if (! strcmp (b->uplink, "POP3")) flag = 'P';
  if (! strcmp (b->uplink, "NNTP")) flag = 'N';
  if (! strcmp (b->uplink, "TELNET")
      && b->forwarding != fwd_standard) flag = 'T';     //telnet
  if (! strcmp (b->uplink, "HTTP")) flag = 'H';
  if (b->sysop && flag != 'E' && flag != 'P') flag = 'S';
  if (t->cputics > 30000L)  // Falls durch Uhrzeitaenderung Mist drin
    t->cputics = 100;       // steht, hier eine Kruecke damit im Log kein
                            // zu grosser Unsinn auftaucht
  if (b->rxbytes > 1
      || flag =='E'
      || flag =='P'
      || flag =='N'
      || flag =='T'
      || flag =='H'
      || flag =='S') // z.B. Flexnet-Linktest weg, aber HTTP etc. behalten
  {
    snprintf(logname, 19, LOGPATH "/log%s.bcm", datestr(ti, 16));
    logf = s_fopen(logname, "sat");
    if (logf)
    {
      fseek(logf, 0, SEEK_END);
      if (! ftell(logf)) fprintf(logf, "%s", logkopf);
      fprintf(logf, "%-6.6s %-14.14s", b->logincall,
                                       datestr(b->logintime, 10));
#if defined __DOS16__ && defined OLDTIMEFMT
      fprintf(logf, "-%.5s%8lu%8lu%7s %c%5u%5u",
#else
      fprintf(logf, "-%s%8lu%8lu%6s %c%4u%4u",
#endif
                  datestr(ad_time(), 3), b->txbytes, b->rxbytes,
                  zeitspanne(t->cputics, zs_cpuexact),
                  flag, b->readfiles, b->sentfiles);
      if (*b->uplink)
      {
        if (*b->peerip)
        {
          if (! m.callformat)
          {
            if (flag == 'H') fprintf(logf, "  via HTTP");
            if (flag == 'T') fprintf(logf, "  via TELNET");
            if (flag == 'E') fprintf(logf, "  via SMTP");
            if (flag == 'N') fprintf(logf, "  via NNTP");
            if (flag == 'P') fprintf(logf, "  via POP3");
          }
          else
            fprintf(logf, "  %s", b->peerip);
        }
        else
          fprintf(logf, "  via %s", b->uplink);
      }
      fputc(LF, logf);
      s_fclose(logf);
    }
  }
}

/*---------------------------------------------------------------------------*/

void putlogauszug (char *selektor)
//*************************************************************************
//
//  Gibt einen Logbuchauszug aus. Suchbegriffe und Optionen werden
//  verarbeitet.
//
//*************************************************************************
{
  lastfunc("putlogauszug");
  typedef struct userlog_t
  { char call[CALLEN+1];
    double txbytes;
    double rxbytes;
    double txfiles;
    double rxfiles;
    int logins;
  } USERLOG;
  static userlog_t *userlog = NULL;
  char *buf;
  char call[CALLEN+1];
  char txbytes[9];
  char rxbytes[9];
  char txfiles[5];
  char rxfiles[5];
  char temp[15];
  double txb, rxb, txf, rxf;
  int gefunden, logins;
  char s[100];
  FILE *f;
  char logname[20];
  time_t logtime = ad_time();
  char *suche;
  char *logarr = (char *) t_malloc((LOGLEN + 1) * 81, "putl");
  int pmin = 0, pmax = 0, i, j, tage = 1;
  long newestlog, anzahl = 0;

  if (! logarr) return;
  selektor += blkill(selektor);
  strupr(selektor);
  if (! mbcallok(selektor) && isdigit(*selektor))
  {
    suche = skip(selektor);
    logtime = parse_time(selektor);
  }
  else
    suche = selektor;
  if (b->optplus & o_w) tage = 8;
  if (b->optplus & o_m) tage = 31;
  if (b->optplus & o_q) tage = 91;
  if (b->optplus & o_y) tage = 366;
  newestlog = logtime;
  logtime -= ((tage - 1) * DAY);
#ifdef __FLAT__
  html_putf("<b>%15s:</b> <a href=cmd?cmd=log>Latest 20 logins</a> |", ms(m_currentlog));
  html_putf("<a href=cmd?cmd=log+-a>User today</a> |");
  html_putf("<a href=cmd?cmd=log+-aephn>User today without TCPIP</a> |");
  html_putf("<a href=cmd?cmd=log+-ag>Forward today</a><br>");
  html_putf("<b>%15s:</b> <a href=cmd?cmd=log+-z>Today</a> |", ms(m_userlog));
  html_putf("<a href=cmd?cmd=log+-zw>Last week</a> |");
  html_putf("<a href=cmd?cmd=log+-zm>Last month</a> |");
  html_putf("<a href=cmd?cmd=log+-zy>Last year</a><br>");
  html_putf("<b>%15s:</b> <a href=cmd?cmd=log+-zg>Today</a> |", ms(m_fwdlog));
  html_putf("<a href=cmd?cmd=log+-zgw>Last week</a> |");
  html_putf("<a href=cmd?cmd=log+-zgm>Last month</a> |");
  html_putf("<a href=cmd?cmd=log+-zgy>Last year</a><br>");
  html_putf("<br>\n");
#endif
  if (! (b->optplus & o_z))
  {
    if (! (b->optplus & o_c))
    {
      putf(ms(m_logoutput));
      if (! (b->optplus & o_a)) putf("(max %d) ", LOGLEN);
      putf("%s", datestr(logtime, 2));
      if (tage > 1) putf("-%s", datestr(newestlog, 2));
      if (*suche) putf(" (\"%s\")", suche);
      putf(":\n%s", logkopf);
    }
    for (i = 0; i < tage; i++)
    {
      snprintf(logname, 19, LOGPATH "/log%s.bcm", datestr(logtime, 16));
      f = s_fopen(logname, "lrt");
      if (f)
      {
        fgets(s, sizeof(s) - 1, f);
        while (fgets(s, sizeof(s) - 1, f))
        {
          if (! suche || strstr(s, suche))
          {
            if ((s[51] != 'F') || (b->optplus & (o_f | o_g)))
            {
              if ((s[51] == 'S') || !(b->optplus & o_s))
              {
                if ((s[51] == 'U') || !(b->optplus & o_u))
                {
                  if ((s[51] != 'H') || !(b->optplus & o_h))
                  {
                    if ((s[51] != 'E') || !(b->optplus & o_e))
                    {
                      if ((s[51] != 'T') || !(b->optplus & o_t))
                      {
                        if ((s[51] != 'P') || !(b->optplus & o_p))
                        {
                          if ((s[51] != 'N') || !(b->optplus & o_n))
                          {
                            if ((s[51] == 'F') || !(b->optplus & o_g))
                            {
                              if (b->sysop || ! strstr(s + 61, "TTY"))
                              {
                                s[80] = 0;
                                if (! strchr(s, LF)) s[79] = LF;
                                strcpy(logarr + pmax * 81, s);
                                if (b->optplus & o_a) putf("%s", s);
                                else
                                {
                                  pmax = (pmax + 1) % (LOGLEN + 1);
                                  if (pmin == pmax)
                                  pmin = (pmin + 1) % (LOGLEN + 1);
                                }
                                anzahl++;
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          waitfor(e_ticsfull);
          if (testabbruch()) break;
        }
        s_fclose(f);
      }
      logtime += DAY; // einen Tag weiter
      if (testabbruch()) break;
    }
    if (! testabbruch())
    {
      if (! anzahl)
        putf(ms(m_noentryfound));
      else
      {
        if (! (b->optplus & o_c))
        {
          while (pmin != pmax)
          {
            putf("%s", logarr + pmin * 81);
            pmin = (pmin + 1) % (LOGLEN + 1);
          }
        }
        if (anzahl > 1) putf(ms(m_entriesfound), anzahl);
        else putf(ms(m_oneentryfound));
      }
    }
  }
  else // Option -z
  {
    putf("Summary %s", datestr(logtime, 2));
    if (tage > 1) putf("-%s", datestr(newestlog, 2));
    if (*suche) putf(" (\"%s\")", suche);
    putf(":\n\n");
    putf("Call      TXBytes    RXBytes    TXFiles    RXFiles     Logins\n");
    putf("--------------------------------------------------------------\n");
    userlog = (userlog_t*) t_malloc(sizeof(userlog_t) * MAXLOGSUM, "putlz");
    for (i = 0; i < tage; i++)
    {
      snprintf(logname, 19, LOGPATH "/log%s.bcm", datestr(logtime, 16));
      f = s_fopen(logname, "lrt");
      if (f)
      {
        fgets(s, sizeof(s) - 1, f);
        while (fgets(s, sizeof(s) - 1, f) && anzahl < MAXLOGSUM )
        {
          if (! suche || strstr(s, suche))
          {
            if ((s[51] != 'F') || (b->optplus & (o_f | o_g)))
            {
              if ((s[51] == 'S') || !(b->optplus & o_s))
              {
                if ((s[51] == 'U') || !(b->optplus & o_u))
                {
                  if ((s[51] != 'H') || !(b->optplus & o_h))
                  {
                    if ((s[51] != 'E') || !(b->optplus & o_e))
                    {
                      if ((s[51] != 'T') || !(b->optplus & o_t))
                      {
                        if ((s[51] != 'P') || !(b->optplus & o_p))
                        {
                          if ((s[51] != 'N') || !(b->optplus & o_n))
                          {
                            if ((s[51] == 'F') || !(b->optplus & o_g))
                            {
                              if (b->sysop || ! strstr(s + 61, "TTY"))
                              {
                                s[80] = 0;
                                // Datensatz definiert initialisieren
                                safe_strcpy(userlog[anzahl].call, "");
                                userlog[anzahl].txbytes = 0;
                                userlog[anzahl].rxbytes = 0;
                                userlog[anzahl].txfiles = 0;
                                userlog[anzahl].rxfiles = 0;
                                userlog[anzahl].logins = 0;
                                gefunden = FALSE;
                                buf = nexttoken(s, call, 6);
                                buf = nexttoken(buf, temp, 8);
                                buf = nexttoken(buf, temp, 12);
                                buf = nexttoken(buf, txbytes, 8);
                                buf = nexttoken(buf, rxbytes, 8);
                                buf = nexttoken(buf, temp, 4);
                                if (   buf[0] == 'F'
                                    || buf[0] == 'Q'
                                    || buf[0] == 'S'
                                    || buf[0] == 'U'
                                    || buf[0] == 'E'
                                    || buf[0] == 'P'
                                    || buf[0] == 'T'
                                    || buf[0] == 'H'
                                    || buf[0] == 'N')
                                  buf = nexttoken(buf, temp, 1);
                                buf = nexttoken(buf, txfiles, 4);
                                buf = nexttoken(buf, rxfiles, 4);
                                for (j = 0; j <= anzahl; j++)
                                {
                                  if (! strcmp(userlog[j].call, call))
                                  { // Datensatz existiert, Werte addieren
                                    userlog[j].txbytes = userlog[j].txbytes
                                                         +atof(txbytes);
                                    userlog[j].rxbytes = userlog[j].rxbytes
                                                         +atof(rxbytes);
                                    userlog[j].txfiles = userlog[j].txfiles
                                                         +atof(txfiles);
                                    userlog[j].rxfiles = userlog[j].rxfiles
                                                         +atof(rxfiles);
                                    userlog[j].logins++;
                                    gefunden = TRUE;
                                  }
                                }
                                if (gefunden == FALSE)
                                { // Datensatz hinzufuegen
                                  safe_strcpy(userlog[anzahl].call, call);
                                  userlog[anzahl].txbytes = atof(txbytes);
                                  userlog[anzahl].rxbytes = atof(rxbytes);
                                  userlog[anzahl].txfiles = atof(txfiles);
                                  userlog[anzahl].rxfiles = atof(rxfiles);
                                  userlog[anzahl].logins = 1;
                                  anzahl++;
                                }
                                if (anzahl >= MAXLOGSUM)
                                {
                                  putf(ms(m_login_maxlog), MAXLOGSUM);
                                  t_free(userlog);
                                  t_free(logarr);
                                  return;
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          waitfor(e_ticsfull);
          if (testabbruch()) break;
        }
        s_fclose(f);
      }
      logtime += DAY; // einen Tag weiter
      if (testabbruch()) break;
    }
    if (! testabbruch())
    {
      if (! anzahl) putf(ms(m_noentryfound));
      else
      {
        txb = rxb = txf = rxf = 0;
        logins = 0;
        for (j = 0; j < anzahl; j++)
        {
          txb = txb+userlog[j].txbytes;
          rxb = rxb+userlog[j].rxbytes;
          txf = txf+userlog[j].txfiles;
          rxf = rxf+userlog[j].rxfiles;
          logins = logins+userlog[j].logins;
          putf("%6s %10.0f %10.0f %10.0f %10.0f %10d\n",
               userlog[j].call,
               userlog[j].txbytes,
               userlog[j].rxbytes,
               userlog[j].txfiles,
               userlog[j].rxfiles,
               userlog[j].logins);
        }
        putf("--------------------------------------------------------------\n");
        putf("       %10.0f %10.0f %10.0f %10.0f %10d\n\n", txb, rxb, txf,
                                                            rxf, logins);
      }
    }
    t_free(userlog);
  }
  t_free(logarr);
}

/*---------------------------------------------------------------------------*/

void putaktuell (int immer)
//*************************************************************************
//
//  Gibt einen Aktuelltext mit Datum und Uhrzeit aus
//
//*************************************************************************
{
  time_t akttime = file_isreg(msgpath("aktuell", u->sprache));
  if (! akttime) akttime = file_isreg(msgpath("aktuell", "GB"));
  if (! akttime) akttime = file_isreg(msgpath("aktuell", "DL"));
  if ((akttime > u->lastboxlogin) || immer)
  {
    if (akttime)
    {
      putf(ms(m_actual), datestr(akttime, 12));
      readtext("aktuell");
    }
    else
      putf(ms(m_noactual));
    if (! immer) leerzeile();
  }
}

/*---------------------------------------------------------------------------*/

void asklogin (char *logincall)
//*************************************************************************
//
//  Generates a short query about the most important user settings.
//  Will be displayed during the first login or if any of the following
//  conditions is true: No username stored, no mybbs entered.
//
//*************************************************************************
{
  char x[64];
  char x1[10];
  char sendcmd[100];
  char content[255];

  if (interactive())
  {
#ifdef _BCMNET_LOGIN
    if (! b->logintype) return;
#endif
    if (! *u->name || ! *u->mybbs)
    {
      readtext("cein");
      while (! *u->name)
      {
        putf("\nName     : ");
        getline(x, 32, 1);
        putv(LF);
        if (! *x) continue;
        mbalter("NAME", x, logincall);
      }
      while (! *u->qth)
      {
        putf("\nQTH      : ");
        getline(x, 32, 1);
        putv(LF);
        if (! *x) continue;
        mbalter("QTH", x, logincall);
      }
      while (! *u->zip)
      {
        putf("\nZIP-Code : ");
        getline(x, 32, 1);
        putv(LF);
        if (! *x) continue;
        mbalter("ZIP", x, logincall);
      }
      while (! *u->mybbs)
      {
        putf("HomeBBS  : ");
        getline(x, 32, 1);
        putv(LF);
        mbalter("MYBBS", x, logincall);
        putv(LF);
      }
    }
    if (m.userquota && u->helplevel == 2 && ! (u->status & 1))
    {
      putf("If you want to switch off the userquota (READ/SEND)\n"
           "limitation, answer 'yes'. The system will send an automatically\n"
           "generated mail to the sysop informing him/her about your request.\n"
           "Yes/No:");
      getline(x1, 1, 1);
      if (toupper(*x1) == 'Y')
      {
        putv(LF);
        b->sysop = 1;
        mbalter("H", "1", logincall);
        sprintf(sendcmd, "SP %s < %s NO_QUOTA: %s(%s)",
                m.sysopcall, m.boxname, logincall, u->name);
        sprintf(content, "User : %s Name: %s Home-bbs: %s\n No userquota.\n",
                logincall, u->name, u->mybbs);
        genmail(sendcmd, content);
        b->sysop = 0;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/

void mblogin (char *logincall, loginmode_t lmode, char *uplink)
//*************************************************************************
//
//  Fuehrt einen ordentlichen Login aus. Bei einem Userlogin
//  wird CTEXT etc. gesendet und das erste Kommando ausgefuehrt.
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "mblogin");
  lastfunc(name);
  char buf[LINELEN+1];
  char *semicolon, *bufptr;
  char i;
  char x1[1];

  if (*b->logincall) mblogout(1);
  if (! *uplink) uplink = m.boxname;
  logincall[CALLEN] = 0;
  strupr(logincall);
  safe_strcpy(b->logincall, logincall);
  safe_strcpy(b->prompt, b->logincall);
  b->logintime = ad_time();
  b->txbytes = 0;
  b->pacbytes = 0;
#ifdef __FLAT__
 #ifdef FILESURF
  if (strcmp(b->uplink, "HTTP")) *b->fspath = 0;
 #endif
#endif
  b->rxbytes = b->sentfiles = b->readfiles = 0;
  b->privoffset = (-1);
  b->sysop = (*uplink && islower(uplink[1])) || b->sysop;
  if (b->sysop) testdfull();
  if (*b->peerip) b->via_radio = 0;
  else b->via_radio = 1;
  loaduser(logincall, u, 1);
  u->logins++;
#ifndef _AX25K_ONLY
 #ifdef __FLAT__
   set_paclen_tnc(u->paclen ? u->paclen : m.paclen ? m.paclen : 256);
 #endif
#endif
  if (! is_msgkenner(u->sprache))
    strcpy(u->sprache, msg_landsuch(b->logincall));
  for (i = 0; i < 5; i++) tmpbidraus(i);
  if (! u->lastdirnews)
    u->lastdirnews = ad_time(); //initialize last-dir-news date
  b->lastdirnews = u->lastdirnews;
  switch (lmode)
  {
    case login_ufwd: b->forwarding = fwd_user; break;
    case login_fwd:  b->forwarding = fwd_standard; break;
    case login_standard:
                     b->forwarding = (fwdtype_t) (isforwardpartner(logincall) >= 0); break;
    case login_nofwd:
    default:         b->forwarding = fwd_none; break;
  }
  if (b->forwarding == fwd_standard && isforwardpartner(logincall) < 0)
    b->forwarding = fwd_user;
  if (b->forwarding == fwd_user && isforwardpartner(logincall) >= 0)
    b->forwarding = fwd_standard;
  if (! strcmp(uplink, "Import")) b->forwarding = fwd_none;
  strcpy(b->uplink, uplink);
  strcpy(u->uplink, uplink);
#ifdef _AUTOTRCWIN
  if (   m.autotrcwin
      && strcmp(uplink, "Console")
      && strcmp(uplink, "Import")
      && (strlen(uplink) > 0
#ifdef _LCF
          || fwdcallok(logincall, m.logincallformat)))
#else
          || mbcallok(logincall)))
#endif
  {
    char tids[2];
    sprintf(tids, "%i", gettaskid());
    trace(report, "trcwin", "task %s", tids);
    fork(P_WIND | P_MAIL, 0, trcwin, tids);
  }
#endif
  if (   m.disable_user
      && (b->forwarding == fwd_none || b->forwarding == fwd_user)
      && stricmp(b->logincall, m.sysopcall)) //users are not allowed to login
  {
    readtext("nousr");
    putflush();
    wdelay(2000);
    *b->logincall = 0;
    return;
  }
  if (b->forwarding == fwd_none) // PSE JJ DO NOT TOUCH
  {
    if (newdate(u->lastboxlogin, b->logintime)) u->daybytes = 0;
    trace(report, name, "%s %s", logincall, uplink);
    if (   ! b->sysop && m.maxlogins && b->via_radio
        && (eingelogt(logincall, 0, 1) >= m.maxlogins))
    {
      *b->logincall = 0;
      trace(report, name, "%s - login rejected - maxlogin", logincall);
      return;
    }
    if ((u->status & 2) && ! b->sysop)
    {
      *b->logincall = 0;
      trace(report, name, "%s - login rejected - status 2", logincall);
      return;
    }
    if ((u->status & 4) && b->forwarding != fwd_standard)
    {
      trace(report, name, "%s - connect rejected - status 4 - request sent",
                          logincall);
      readtext("cstat4");
      putflush();
      u->status = 2; //sysop will be informed only once
      char sendcmd[50];
      char content[50];
      snprintf(sendcmd, sizeof(sendcmd), "SP %s < %s NEW_USER: %s",
                        m.sysopcall, m.boxname, logincall);
      snprintf(content, sizeof(content), "User: %s - Status 4.\n", logincall);
      genmail(sendcmd, content);
      saveuser(u);
      *b->logincall = 0;
      return;
    }
    if (autosysop()) b->sysop = 1; //JJ
#ifdef _BCMNET_LOGIN
    b->logintype = b->pwok; //if we are already authenticated (HTTP,POP3,..)
#endif
    if (lmode != login_silent)
    {
      if (
          m.userpw < 3 && //alter login-pw-modus
          *u->password > 1 && u->loginpwtype <= 2
          && ! b->sysop && ! t->socket
#ifdef _TELEPHONE
          && strcmp(b->uplink, "TTY")
#endif
         )
      {
        if (u->pwline)
        {
          putf("Password: ");
          getline(buf, LINELEN, 1);
        }
        putv(LF);
        char pwok = 1;
        switch (u->loginpwtype) // DH3MB
        {
        case 0: pwok = pwauswert(u->password); break;
#ifdef FEATURE_MDPW
        case 1: pwok = getMDpw(2, u->password); break;
        case 2: pwok = getMDpw(5, u->password); break;
#endif
        default: pwok = 1;
        }
        if (getvalid() == CR) *buf = getv();
        if (! pwok)
        {
          pwlog(b->logincall, b->uplink, "userpw failure");
          *b->logincall = 0;
          return;
        }
        b->pwok = pwok;
      }
      putf("\n" LOGINSIGN);
      if (m.callformat)
        putf(" " CBMODEID " ");
#ifdef _GUEST
      if (strcmp(logincall, m.guestcall))
      {
#endif
      if (! readtext("ctext")) putv(LF);
      if (u->helplevel == 2) readtext("cnew");
       leerzeile();
#ifdef _BCMNET_LOGIN
       // CB-BCMNET Loginconcept, please see documentation for details
       b->logintype = 1; //full login
       if (! b->pwok) //no password
         if (   u->mybbs[0] != 0 //mybbs is set
             && u->mybbstime > (ad_time() - USERPROTECT) //young mybbs
             && strcmp(atcall(u->mybbs), m.boxname) != 0 //not at home
            )
         {
           b->logintype = 0; //guest
           readtext("bnguest"); //login as guest
         }
         else //no mybbs set || old mybbs || at home
           if (m.pwonly == 1)  //login only with password allowed
           {
             b->logintype = 0; //guest
             readtext("bnpwonly"); //password necessary for full login
           }
       if (! b->logintype)
       {
         leerzeile();
         trace(report, name, "%s login as guest", logincall);
       }
#endif
       if (m.asklogin) asklogin(logincall);
       putaktuell(0);
#ifdef _GUEST
     }
     else
     {
       b->sysop = 0; //guest can not be sysops!
       readtext("cguest");
       putv(LF);
     }
#endif
      //df3vi: Hinweistext auf Beschraenkungen ohne Password
     if (m.userpw >= 2 && *u->password <= 1 && ! b->sysop)
     {
       readtext("pwonly");
       putv(LF);
     }
     if (*u->name) putf(ms(m_servus), u->name);
     if (u->helplevel) putf(ms(m_helplines), u->helplevel, u->zeilen);
     if (u->lastboxlogin) putf(ms(m_lastlogin), datestr(u->lastboxlogin, 12));
     else putf(ms(m_firstlogin));
     if (! u->mybbsok)
       putf(ms(m_inputmybbs));
     else
     {
       char usbbs[CALLEN+1];
       safe_strcpy(usbbs, atcall(u->mybbs));
       usbbs[CALLEN] = 0;
#ifdef _BCMNET_LOGIN
       if (! stricmp(usbbs, m.boxname) && u->mybbstime < (ad_time() - DAY * 30))
       //update mybbs-setting if this is home-bbs and setting is older than 1 months
#else
       if (! stricmp(usbbs, m.boxname) && u->mybbstime < (ad_time() - DAY * 180))
       //update mybbs-setting if this is home-bbs and setting is older than 6 months
#endif
       {
         set_mybbs(u->call, u->mybbs, ad_time(), newbid(), b->logincall, 0, NULL);
         loaduser(u->call, u, 0);
       }
     }
     leerzeile();
     if (*u->notvisible) // Zur Beseitigung von Altlasten
     {
       u->notvisible[LINELEN - 2] = 0;
       if (u->notvisible[strlen(u->notvisible) - 1] != ' ')
         strcat(u->notvisible, " ");
     }
     *buf = 0; //clear buffer
     if (strcmp(b->uplink, "Import"))
       safe_strcpy(buf, u->firstcmd); //no login-cmd on imports
     bufptr = buf;
     u->lastboxlogin = b->logintime;
     saveuser(u);
#ifdef FILESURF // DH3MB
     filesurf fs;
     fs.getfirstpath(b->fspath);
#endif
      if (*buf) do
      {
        if ((semicolon = strchr(bufptr, ',')) != NULL) *semicolon = 0;
        mailbef(bufptr, 0);
        b->quit = 0;
        if (semicolon) bufptr = semicolon + 1;
        timeout(m.usrtimeout);
      }
      while (semicolon);
    }
  }
  else
    b->charset = 0;
  u->lastboxlogin = b->logintime;
  if (u->away == 1)
  {
    if (u->awayendtime !=0 && u->awayendtime < b->logintime)
    {
      u->away=0;
      u->awayendtime = 0;
    }
    else
    {
      if (interactive())
      {
        putf("\n");
        putf(ms(m_disableaway));
        getline(x1, 1, 1);
        if (toupper(*x1) == 'Y' || toupper(*x1) == 'J') u->away = 0;
      }
    }
  }
  saveuser(u);
}

/*---------------------------------------------------------------------------*/

void mblogout (int discon_int)
//*************************************************************************
//
//  Fuehrt einen ordentlichen Logout aus.
//  Wird auch beim Kill eines Prozesses aufgerufen.
//
//*************************************************************************
{
  char name[] = "mblogout";
  lastfunc(name);

  if (b && u)
  {
    if (*b->logincall && b->logintime) // gegen login-fakes
    {
      FILE *uf = s_fopen(USERNAME, "sr+b"); //User file accessable?
      if (uf)
      {
        s_fclose(uf);
        if (loaduser(b->logincall, u, 0))
        {
          u->daybytes += (b->rxbytes + b->txbytes);
          u->lastboxlogin = b->logintime; //10.07.04 diese zeile fehlte?
          saveuser(u);
          if (! discon_int && b->forwarding == fwd_none)
          {
            readtext("qtext");
            putflush();
          }
          //df3vi: kein Log schreiben bei internen Servern
          if (   strcmp(b->uplink, "Import")
              && strcmp(b->uplink, "Mailsv")
              && strcmp(b->uplink, "Pocsag")) logbuch();
        }
        else
          trace(serious, name, "no user %s", b->logincall);
      }
      else
        trace(serious, name, "no access to userfile");
      if (b->sysop && strcmp(b->uplink, "POP3")
                   && strcmp(b->uplink, "SMTP")) //not smtp/pop3
        cmdlog("******** Logout");
      if (b->forwarding != fwd_none)
        switch (discon_int) //df3vi: Unterscheidung kill/discon
        {
          case 2:  fwdlog("---- ", "disconnect", '-'); break;
          case 1:  fwdlog("---- ", "kill", '-'); break;
          default: fwdlog("---- ", "logout", '-');
        }
      else
        wdelay(246);
      switch (discon_int) //df3vi: Unterscheidung kill/discon
      {
        case 2:  trace(report, name, "%s (disc)", b->logincall); break;
        case 1:  trace(report, name, "%s (kill)", b->logincall); break;
        default: trace(report, name, "%s (logout)", b->logincall);
                 /* if (b->forwarding == fwd_none)  wdelay(246); */
      }
      *b->logincall = 0;
      *b->prompt = 0;
      if (gettaskid() != NOTASK)
        t->cputics = 0;
    }
  }
}
