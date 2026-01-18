/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------------------
  Auswertung/Erzeugung des Konfigurationsfiles
  --------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW removed utcoffset, added pcisutc, stimeoffset
//19980202 OE3DZW added callformat
//                infolifetime 0 -> 999 (was 10)
//19980227 hrx    added 'guestcall' to beftab/cases/init.bcm - default is off
//19980311 DG9MHZ changes for Linux AX25-Kernel support
//19980324 OE3DZW fixed output of fwdssid
//19980404 hrx    support for tty-fwding
//19980408 hrx    autotrcwin added to beftab/befnum/cases/mbinit()/mbparsave()
//                commented out line 557, callformat should always be saved
//19980614 hrx    paclen added to beftab/befnum/cases/mbinit()/mbparsave(),
//                limited to dos16. added createboardfwd to beftab/befnum
//                cases/mbinit()/mbparsave()
//19980831 OE3DZW control-characters in the inifile will be ignored
//19980913 OE3DZW added savebroken
//19980924 Jan    added oldestbeacon
//19980924 OE3DZW fixed oldestbeacon,removed old parameters, again
//19981001 OE3DZW fixed oldestbeacon, (changed oldestfwd)
//19981015 OE3DZW added parameters for wxstation
//19981022 OE3DZW translated name of variables
//19990110 OE3DZW hadr will always be updates with correct own data
//19990215 OE3DZW removed TELL-server
//19990221 OE3DZW changed limit for maxbids, infolife,userlife
//19991028 Jan    added defstatus,paclen, feature_serial
//20000119 OE3DZW nntp port is always saved
//20000203 OE3DZW added parameter disableuser
//                df3vi: schlechte kopie von sf_only :-(
//20000107 DF3VI  invented user-timeout
//20000402 DF3VI  Hold-Timer
//20010620 DK2UI  m_filter name in init.bcm if enabled
//20010803 DF3VI  added oldumail and lt_query
//20020509 DH8YMB altboardinfo, timeoutwarning, nounknownroute, asklogin,
//                smoothheader added
//20021130 DH8YMB added nopopsmtp
//20021206 DH8YMB added httpaccount, httpguestfirst
//20021217 DH8YMB added httpttypw
//20030102 hpk    added minpaclen, maxpaclen, defrea
//20030102 hpk    added pwonly for the CB-BCMNET login-concept
//20030101 hpk    if the CB-BCMNET login-concept is active, m.holdtime's
//                default is 720h (1 month)
//20030104 DB1RAS added _PREPOSTEXEC (dospreexec, dospostexec)
//20030109 DH8YMB defhold hinzu
//20030209 DH8YMB unsecurettypw und maillistsender hinzu
//20030407 DH8YMB added summertime
//20030912 DH8YMB added defswaplf
//20031025 DH8YMB added deffbbcheckmode
//20040110 DH8YMB added addlinuxsystemuser
//20041017 DH8YMB added httprobots
//20041230 DH8YMB added fsbmail
//20050401 DH8YMB changed httpcss -> defhttpsurface
//20050917 DH8YMB added JJs INETMAILGATE
//20061110 DH8YMB added httpshowsysopcall

#include "baycom.h"
#include "l2_appl.h"

/*---------------------------------------------------------------------------*/

static char *beftab[]=
  { "BOXADRESS", "BOXADDRESS", "BOXHEADER", "SYSOPCALL", "SYSOPBELL",
    "INFOLIFE", "USERLIFE", "OLDESTFWD", "OLDESTBEACON", "TRACELEVEL",
    "FWDTRACE", "USERPATH", "INFOPATH", "FWDSSID",
    "USERQUOTA", "USVSENSE", "WATCHDOG", "MAXLOGINS", "DISABLE",
//    "DISABLEUSER",
    "DEFPROMPT", "DEFHELP", "DEFLINES", "DEFLF", "DEFSWAPLF", "DEFCMD", "DEFSPEECH",
    "DEFIDIR", "DEFUDIR", "DEFILIST", "DEFULIST", "MAILBEACON", "PURGEHOUR",
    "DEFIREAD", "DEFUREAD", "DEFCHECK", "DEFPS", "READLOCK",
    "NEXTBID", "NEXTFILE", "SCROLLDELAY", "TIMESLOT", "DOSINPUT", "HADRSTORE",
    "USERPW", "REMERASE", "ERASELOG", "CREATEBOARD", "MAXBIDS", "FWDTIMEOUT",
    "USRTIMEOUT", "MYCALL", "NOPURGE", "SAVEBROKEN", "BIDOFFSET", "CALLFORMAT",
    "PACLEN", "MINPACLEN", "MAXPACLEN", "ALTBOARDINFO", "TIMEOUTWARNING",
    "NOUNKNOWNROUTE", "ASKLOGIN", "SMOOTHHEADER", "SFONLY", "M_FILTER",
    "OLDUMAIL", "LTQUERY", "DEFSTATUS", "TELLMODE", "NOPOPSMTP", "HTTPACCOUNT",
    "DEFHTTPSURFACE", "HTTPGUESTFIRST", "HTTPTTYPW", "HTTPSHOWSYSOPCALL",
    "HTTPROBOTS", "UNSECURETTYPW",
    "HOLDTIME", "DEFREA", "DEFHOLD",
#ifdef HB9EAS_DEBUG
    "HB9EAS_DEBUG",
#endif
#ifdef __FLAT__
    "TCPIPTRACE",
#endif
#ifdef _LCF
    "LOGINCALLFORMAT",
#endif
#ifdef _AUTOFWD
    "AUTOFWDTIME",
#endif
#ifdef __FLAT__
    "SMTP_PORT", "POP3_PORT", "NNTP_PORT", "HTTP_PORT", "FTP_PORT", "TELNET_PORT",
    "SERV_PORT", "RADIO_PORT",
#ifdef INETMAILGATE
    "INTERNETMAILGATE",
#endif
#endif
#ifndef __LINUX__
    "SUMMERTIME",
#endif
#ifdef __DOS16__
    "CRTSAVE", "PCISUTC", "STIMEOFFSET",
#endif
#ifdef FILESURF // DH3MB
    "FSPATH", "FSBMAIL",
#endif
#ifdef MAILSERVER // DH3MB
    "MAILLISTSERV", "MAILLISTSENDER",
#endif
#ifdef DF3VI_POCSAG
    "POCSAGLISTSERVER",
#endif
#ifdef _AX25K
    "AX25K_IF",
#endif
#ifdef _GUEST
    "GUESTCALL",
#endif
#ifdef _AUTOTRCWIN
    "AUTOTRCWIN",
#endif
#ifdef _TELEPHONE
    "TTYDEVICE",
#endif
#ifdef _WXSTN
    "WXSTNNAME", "WXQTHALTITUDE", "WXSENSORALTITUDE", "WXTTY", "WXPATH",
#endif
#ifdef _MORSED
    "CWID", "CWDOID", "CWNEXTJOB", "CWFREQU", "CWMS",
#endif
#ifdef _BCMNET_LOGIN
    "PWONLY",
#endif
#if defined FEATURE_SERIAL || defined _TELEPHONE
    "TTYMODE",
#endif
#ifdef _PREPOSTEXEC //db1ras
    "DOSPREEXEC", "DOSPOSTEXEC",
#endif
#ifdef FBBCHECKREAD
    "DEFFBBCHECKMODE",
#endif
#ifdef __LINUX__
    "ADDLINUXSYSTEMUSER",
#endif
    NULL
  };

/*---------------------------------------------------------------------------*/

static enum befnum
  { unsinn,
    boxadress, boxaddress, boxheader, sysopcall, sysopbell,
    infolife, userlife, oldestfwd, oldestbeacon, tracelevel,
    fwdtrace, userpath, infopath, fwdssid,
    userquota, usvsense, watchdog, maxlogins, disable_,
//    disable_user,
    defprompt, defhelp, deflines, deflf, defswaplf, defcmd, defspeech,
    defidir, defudir, defilist, defulist, mailbeacon, purgehour,
    defiread, defuread, defcheck, defps, readlock,
    nextbid, nextfile, scrolldelay, timeslot, dosinput, hadrstore,
    userpw, remerase, eraselog, createboard, maxbids, fwdtimeout,
    usrtimeout, mycall, nopurge, savebroken, bidoffset, callformat,
    paclen, minpaclen, maxpaclen, altboardinfo, timeoutwarning,
    nounknownroute, asklogin, smoothheader, sfonly, m_filter,
    oldumail, ltquery, defstatus, tellmode, nopopsmtp, httpaccount,
    defhttpsurface, httpguestfirst, httpttypw, httpshowsysopcall,
    httprobots, unsecurettypw,
    holdtime, defrea, defhold
#ifdef HB9EAS_DEBUG
    ,hb9eas_debug
#endif
#ifdef __FLAT__
    ,tcpiptrace
#endif
#ifdef _LCF // JJ
    ,logincallformat
#endif
#ifdef _AUTOFWD
    ,autofwdtime
#endif
#ifdef __FLAT__
    ,smtp_port, pop3_port, nntp_port, http_port, ftp_port, telnet_port
    ,serv_port, radio_port
#ifdef INETMAILGATE
    ,internetmailgate
#endif
#endif
#ifndef __LINUX__
    ,summertime
#endif
#ifdef __DOS16__
    ,crtsave, pcisutc, stimeoffset
#endif
#ifdef FILESURF
    ,fspath, fsbmail
#endif
#ifdef MAILSERVER
    ,maillistserv, maillistsender
#endif
#ifdef DF3VI_POCSAG
    ,pocsaglistserv
#endif
#ifdef _AX25K
    ,ax25k_if
#endif
#ifdef _TELEPHONE
    ,ttydevice
#endif
#ifdef _GUEST
    ,guestcall
#endif
#ifdef _AUTOTRCWIN
    ,autotrcwin
#endif
#ifdef _WXSTN
    ,wxstnname, wxqthaltitude, wxsensoraltitude, wxtty, wxpath
#endif
#ifdef _MORSED
    ,cwid, cwdoid, cwnextjob, cwfreq, cwms
#endif
#ifdef _BCMNET_LOGIN
    ,pwonly
#endif
#if defined FEATURE_SERIAL || defined _TELEPHONE
    ,ttymode    //DOS Telephone-Forward or /dev/tty7 under Linux
#endif
#ifdef _PREPOSTEXEC //db1ras
    ,dospreexec, dospostexec
#endif
#ifdef FBBCHECKREAD
    ,deffbbcheckmode
#endif
#ifdef __LINUX__
    ,addlinuxsystemuser
#endif
}
cmd = unsinn;

/*---------------------------------------------------------------------------*/

int mbinitbef (char *befbuf, handle inifile)
//*************************************************************************
//
//  Initialisiert die globalen Mailbox-Variablen aus der Datei init.bcm
//
//*************************************************************************
{
  int optgroup = 0;
  unsigned int i, j;

  befbuf += blkill(befbuf);
  if (! *befbuf || *befbuf == ';' || *befbuf == '[') return OK;
  if (inifile)
  {
    subst1 (befbuf, ';', 0);
    //replace all control-characters by space (tab, linefeed etc.)
    for (i = 0; befbuf[i]; i++)
      if (befbuf[i] < ' ') befbuf[i] = ' ';
  }
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  if (*befbuf == '=') befbuf++;
  befbuf += blkill(befbuf);
  cut_blank(befbuf);
  if (*befbuf)
  {
    switch (cmd)
    {
      case unsinn: return 0;
      case boxaddress:
      case boxadress:
           {
             strupr(befbuf);
             befbuf[HADRESSLEN] = 0;
             strcpy(m.boxadress, befbuf); // set address
             subst1(befbuf, '.', 0);
             befbuf[CALLEN] = 0;
             subst1(befbuf, '-', 0);
             strcpy(m.boxname, befbuf); // set name
             if (! inifile && mbcallok(m.boxname)) m.disable = 0;
           } break;
      case boxheader:
           {
             safe_strcpy(m.boxheader, befbuf);
             makeheader(1); //dummy call to cut boxheader
           } break;
#ifdef __FLAT__
      case smtp_port:
           {
             m.smtp_port = atoi(befbuf);
           } break;
      case pop3_port:
           {
             m.pop3_port = atoi(befbuf);
           } break;
      case nntp_port:
           {
             m.nntp_port = atoi(befbuf);
           } break;
      case radio_port:
           {
             m.radio_port = atoi(befbuf);
           } break;
      case telnet_port:
           {
             m.tcp_port = atoi(befbuf);
           } break;
      case serv_port:
           {
             m.serv_port = atoi(befbuf);
           } break;
      case http_port:
           {
             m.http_port = atoi(befbuf);
           } break;
      case ftp_port:
           {
             m.ftp_port = atoi(befbuf);
           } break;
      case nopopsmtp:
           {
             m.nopopsmtp = atoi(befbuf);
             if (m.nopopsmtp > 1) m.nopopsmtp = 1;
             if (m.nopopsmtp < 0) m.nopopsmtp = 0;
           } break;
      case httpaccount:
           {
             m.httpaccount = atoi(befbuf);
             if (m.httpaccount > 1) m.httpaccount = 1;
             if (m.httpaccount < 0) m.httpaccount = 0;
           } break;
      case defhttpsurface:
           {
             m.defhttpsurface = atoi(befbuf);
             if (m.defhttpsurface > 2) m.defhttpsurface = 2;
             if (m.defhttpsurface < 0) m.defhttpsurface = 0;
           } break;
      case httpguestfirst:
           {
             m.httpguestfirst = atoi(befbuf);
             if (m.httpguestfirst > 1) m.httpguestfirst = 1;
             if (m.httpguestfirst < 0) m.httpguestfirst = 0;
           } break;
      case httpttypw:
           {
             m.httpttypw = atoi(befbuf);
             if (m.httpttypw > 1) m.httpttypw = 1;
             if (m.httpttypw < 0) m.httpttypw = 0;
           } break;
      case httpshowsysopcall:
           {
             m.httpshowsysopcall = atoi(befbuf);
             if (m.httpshowsysopcall > 1) m.httpshowsysopcall = 1;
             if (m.httpshowsysopcall < 0) m.httpshowsysopcall = 0;
           } break;
      case httprobots:
           {
             m.httprobots = atoi(befbuf);
             if (m.httprobots > 1) m.httprobots = 1;
             if (m.httprobots < 0) m.httprobots = 0;
           } break;
      case unsecurettypw:
           {
             m.unsecurettypw = atoi(befbuf);
             if (m.unsecurettypw > 2) m.unsecurettypw = 2;
             if (m.unsecurettypw < 0) m.unsecurettypw = 0;
           } break;
#ifdef INETMAILGATE
      case internetmailgate:
           {
             befbuf[CALLEN+1] = 0;
             strupr(befbuf);
             strcpy(m.internetmailgate, befbuf);
           } break;
#endif
#endif
#ifdef _AX25K
      case ax25k_if:
           {
             befbuf[80] = 0;
             strupr(befbuf);
             strcpy(m.ax25k_if, befbuf);
           } break;
#endif
#ifdef _GUEST
      case guestcall:
           {
             strupr(befbuf);
             befbuf[CALLEN] = 0;
    //       if (mbcallok(befbuf) || ! strcmp(befbuf, "OFF"))
               safe_strcpy(m.guestcall, befbuf)
    //       else
    //       {
    //         trace(serious, "mbinitbef",
    //                        "guestcall %s not valid, switched off", befbuf);
    //         safe_strcpy(m.guestcall, "off");
    //       }
           } break;
#endif
#ifdef HB9EAS_DEBUG
      case hb9eas_debug:
           {
             m.hb9eas_debug = atoi(befbuf);
           } break;

#endif
#ifndef __LINUX__
      case summertime:
           {
             m.summertime = atoi(befbuf);
             if (! (m.summertime==0 || m.summertime==1))
             {
               trace(serious, "mbinitbef",
                              "summertime only 0 or 1, set to 1");
               m.summertime = 1;
             }
           } break;
#endif
#ifdef __DOS16__
      case crtsave:
           {
             m.crtsave = atoi(befbuf);
           } break;
      case pcisutc:
           {
             m.pcisutc = atoi(befbuf);
           } break;
      case stimeoffset:
           {
             m.stimeoffset = atoi(befbuf);
           } break;
      case dosinput:
           {
             m.dosinput = atoi(befbuf);
           } break;
#endif
#ifdef __LINUX__
      case addlinuxsystemuser:
           {
             m.addlinuxsystemuser = atoi(befbuf);
             if (m.addlinuxsystemuser > 1) m.addlinuxsystemuser = 1;
             if (m.addlinuxsystemuser < 0) m.addlinuxsystemuser = 0;
           } break;
#endif
      case paclen:
           {
             m.paclen = atoi(befbuf);
             if (! m.paclen==0 && (m.paclen < 40 || m.paclen > 255))
             {
               trace(serious, "mbinitbef", "paclen only between 40-255 or 0");
               m.paclen = 0;
             }
           } break;
      case minpaclen:
           {
             m.minpaclen = atoi(befbuf);
             if (m.minpaclen < 40)
             {
               trace(serious, "mbinitbef", "minpaclen too small, now 40");
               m.minpaclen = 40;
             }
           } break;
      case maxpaclen:
           {
             m.maxpaclen = atoi(befbuf);
             if (m.maxpaclen > 256)
             {
               trace(serious, "mbinitbef", "maxpaclen too big, now 256");
               m.maxpaclen = 256;
             }
             if (m.maxpaclen <= m.minpaclen)
             {
               trace(serious, "mbinitbef", "maxpaclen <= minpaclen, now 256");
               m.maxpaclen = 256;
             }
           } break;
      case mailbeacon:
           {
             m.mailbeacon = atoi(befbuf);
           } break;
      case altboardinfo:
           {
             m.altboardinfo = atoi(befbuf);
           } break;
      case nounknownroute:
           {
             m.nounknownroute = atoi(befbuf);
           } break;
      case timeoutwarning:
           {
             m.timeoutwarning = atoi(befbuf);
           } break;
      case asklogin:
           {
             m.asklogin = atoi(befbuf);
           } break;
      case smoothheader:
           {
             m.smoothheader = atoi(befbuf);
           } break;
#ifdef _AUTOTRCWIN
      case autotrcwin:
           {
             m.autotrcwin = atoi(befbuf);
             if (m.autotrcwin > 1U) m.autotrcwin = 0;
           } break;
#endif
#ifdef _WXSTN
      case wxstnname:
           {
             befbuf[30] = 0;
             strcpy(m.wxstnname, befbuf);
           } break;
      case wxqthaltitude:
           {
             m.wxqthaltitude = atol(befbuf);
           } break;
      case wxsensoraltitude:
           {
             m.wxsensoraltitude = atol(befbuf);
           } break;
      case wxtty:
           {
             befbuf[20] = 0;
             strcpy(m.wxtty, befbuf);
           } break;
      case wxpath:
           {
             strlwr(befbuf);
             killbackslash(befbuf);
             befbuf[28] = 0;
             if (befbuf[strlen(befbuf) - 1] == '/')
               befbuf[strlen(befbuf) - 1] = 0; // remove trailing slash
             strcpy(m.wxpath, befbuf);
             xmkdir(befbuf);
           } break;
#endif
#ifdef _MORSED
      case cwid:
           {
             befbuf[49] = 0;
             strcpy(m.cwid, befbuf);
           } break;
      case cwdoid:
           {
             m.cwdoid = atol(befbuf);
           } break;
      case cwnextjob:
           {
             befbuf[49] = 0;
             strcpy(m.cwnextjob, befbuf);
           } break;
      case cwfreq:
           {
             m.cwfreq = atol(befbuf);
           } break;
      case cwms:
           {
             m.cwms = atol(befbuf);
           } break;
#endif
      case tellmode:
           {
             m.tellmode = atoi(befbuf);
             if (m.tellmode > 2U) m.tellmode = 2;
           } break;
      case sysopcall:
           {
             strupr(befbuf);
             befbuf[CALLEN] = 0;
             safe_strcpy(m.sysopcall, befbuf);
           } break;
      case callformat: // 0..amateur, 1..cb, 2..both
           {
#ifdef _BCMNET_LOGIN
              m.callformat = 2;
#else
              m.callformat = atoi(befbuf);
              if (m.callformat > 2) m.callformat = 0;
              if (! mbcallok(m.boxname))
              {
                trace(serious, "mbinitbef",
                               "box callformat wrong, not saved");
                m.callformat = 2;
              }
#endif
           } break;
#ifdef _LCF
      case logincallformat: // 0..amateur, 1..cb, 2..both
           {
#ifdef _BCMNET_LOGIN
              m.logincallformat = 2;
#else
              m.logincallformat = atoi(befbuf);
              if (m.logincallformat > 2U) m.logincallformat = 0;
#endif
           } break;
#endif
      case savebroken:
           {
             m.savebroken = atoi(befbuf);
           } break;
      case bidoffset:
           {
             m.bidoffset = atoi(befbuf);
             if (m.bidoffset > 3) m.bidoffset = 0; // 0..3 is ok
           } break;
      case infolife:
           {
             m.infolife = atoi(befbuf);
             if (! m.infolife) m.infolife = 999;
           } break;
      case userlife:
           {
             m.userlife = atoi(befbuf);
             if (! m.userlife) m.userlife = 999;
           } break;
      case nopurge:
           {
             m.nopurge = atoi(befbuf);
           } break;
      case maxbids:
           {
             m.maxbids = atol(befbuf);
             if (m.maxbids < 131072L) m.maxbids = 131072L; //2^17
           } break;
      case createboard:
           {
             m.createboard = atoi(befbuf);
           } break;
      case remerase:
           {
             m.remerase = atoi(befbuf);
           } break;
      case eraselog:
           {
             m.eraselog = atoi(befbuf);
           } break;
      case oldestfwd:
           {
             m.oldestfwd = atoi(befbuf);
             if (! m.oldestfwd) m.oldestfwd = 999;
#ifdef _BCMNET_LOGIN
             if (m.oldestfwd < 30) m.oldestfwd = 30;
             if (m.oldestfwd > 60) m.oldestfwd = 60;
#endif
           } break;
      case oldestbeacon:
           {
             m.oldestbeacon = atoi(befbuf);
             if (! m.oldestbeacon || m.oldestbeacon > 1000U)
               m.oldestbeacon = 1;
           } break;
      case userquota:
           {
             m.userquota = atoi(befbuf);
           } break;
      case readlock:
           {
             m.readlock = atoi(befbuf);
           } break;
      case usvsense:
           {
             m.usvsense = atoi(befbuf);
           } break;
      case watchdog:
           {
             m.watchdog = atoi(befbuf);
           } break;
      case maxlogins:
           {
             m.maxlogins = atoi(befbuf);
           } break;
      case disable_:
           {
             m.disable = atoi(befbuf);
           } break;
/*
      case disable_user:
           {
             m.disable_user = atoi(befbuf);
           } break;
*/
      case sfonly:
           {
             m.sf_only = atoi(befbuf);
           } break;
      case tracelevel:
           {
             m.tracelevel = atoi(befbuf);
           } break;
      case fwdtrace:
           {
             m.fwdtrace = atoi(befbuf);
           } break;
#ifdef __FLAT__
      case tcpiptrace:
           {
             m.tcpiptrace = atoi(befbuf);
           } break;
#endif
      case sysopbell:
           {
             m.sysopbell = atoi(befbuf);
           } break;
      case userpath:
           {
             strlwr(befbuf);
             killbackslash(befbuf);
             befbuf[28] = 0;
             if (befbuf[strlen(befbuf) - 1] == '/')
               befbuf[strlen(befbuf) - 1] = 0; // remove trailing slash
             strcpy(m.userpath, befbuf);
             xmkdir(befbuf);
           } break;
      case infopath:
           {
             strlwr(befbuf);
             killbackslash(befbuf);
             befbuf[28] = 0;
             if (befbuf[strlen(befbuf) - 1] == '/')
               befbuf[strlen(befbuf) - 1] = 0; // remove trailing slash
             strcpy(m.infopath, befbuf);
             xmkdir(befbuf);
           } break;
#if defined (FEATURE_SERIAL) || defined (_TELEPHONE)
      case ttymode:
           {
             if (stricmp(befbuf, "off") && strlen(befbuf) > 5)
             {
               fncase(befbuf);
               befbuf[19] = 0;
               strcpy(m.ttymode, befbuf);
             }
             else
               *m.ttymode = 0;
           } break;
#endif
#ifdef _TELEPHONE
      case ttydevice:
           {
             m.ttydevice = atoi(befbuf);
           } break;
#endif
      case fwdssid:
           {
             m.fwdssid = atoi(befbuf);
           } break;
      case fwdtimeout:
           {
             m.fwdtimeout = atoi(befbuf);
           } break;
      case usrtimeout:
           {
             m.usrtimeout = atoi(befbuf);
#ifdef _BCMNET_LOGIN
             //nur usrtimeout >= 60 zulassen
             if (m.usrtimeout < 60) m.usrtimeout = 60;
#endif
           } break;
      case holdtime:
           {
             m.holdtime = atoi(befbuf);
#ifdef _BCMNET_LOGIN
             //nur 168 > holdtime < 720 zulassen
             if (m.holdtime < 168) m.holdtime = 168;
             if (m.holdtime > 720) m.holdtime = 720;
#endif
           } break;
#ifdef _AUTOFWD
      case autofwdtime:
           {
             m.autofwdtime = atoi(befbuf);
           } break;
#endif
      case defprompt:
           {
             befbuf[PROMPTLEN] = 0;
             strcpy(m.prompt, befbuf);
           } break;
      case defcmd:
           {
             strupr(befbuf);
             befbuf[FIRSTCMDLEN] = 0;
             strcpy(m.firstcmd, befbuf);
           } break;
      case defhelp:
           {
             m.helplevel = atoi(befbuf);
           } break;
      case deflines:
           {
             m.zeilen = atoi(befbuf);
           } break;
      case defstatus:
           {
             m.defstatus = atoi(befbuf);
           } break;
      case defhold:
           {
             m.defhold = atoi(befbuf);
           } break;
#ifdef FBBCHECKREAD
      case deffbbcheckmode:
           {
             m.deffbbcheckmode = atoi(befbuf);
           } break;
#endif
      case userpw:
           {
             m.userpw = atoi(befbuf);
           } break;
      case mycall:
           {
             strupr(befbuf);
             j = sscanf(befbuf, "%s%s%s%s",
                                m.mycall[0], m.mycall[1],
                                m.mycall[2], m.mycall[3]);
             if (j > 0 && j < 5)
               m.mycalls = j;
             else
               m.mycalls = 0;
             align_mycalls();
           } break;
      case ltquery:
           {
             m.ltquery = atoi(befbuf);
#ifdef _BCMNET_LOGIN
             // nur 1..2 zulassen
             if (m.ltquery>2) m.ltquery = 1;
             if (m.ltquery<1) m.ltquery = 1;
#else
             if (m.ltquery>2) m.ltquery = 0; // 0..2 is ok
#endif
           } break;
      case oldumail:
           {
             m.oldumail= atoi(befbuf);
           } break;
      case defswaplf:
           {
             m.defswaplf = atoi(befbuf);
           } break;
      case m_filter:
           {
             befbuf[FNAMELEN] = 0;
             strcpy(m.m_filter, befbuf);
           } break;
#ifdef FILESURF
      case fspath:
           {
             befbuf[FSPATHLEN] = 0;
             strcpy(m.fspath, befbuf);
           } break;
      case fsbmail:
           {
             m.fsbmail = atoi(befbuf);
           } break;
#endif
#ifdef MAILSERVER
      case maillistserv:
           {
             m.maillistserv = atoi(befbuf);
           } break;
      case maillistsender:
           {
             m.maillistsender = atoi(befbuf);
           } break;
#endif
#ifdef DF3VI_POCSAG
      case pocsaglistserv:
           {
             m.pocsaglistserv = atoi(befbuf);
           } break;
#endif
      case deflf:
           {
             m.deflf = atoi(befbuf);
           } break;
      case defps:
           {
             optgroup++;
           }
      case defcheck:
           {
             optgroup++;
           }
      case defuread:
           {
             optgroup++;
           }
      case defiread:
           {
             optgroup++;
           }
      case defulist:
           {
             optgroup++;
           }
      case defilist:
           {
             optgroup++;
           }
      case defudir:
           {
             optgroup++;
           }
      case defidir:
           {
             while (*befbuf)
             {
               if (isalpha(*befbuf))
                 m.opt[optgroup] |= (1L << (*befbuf - 'A'));
               befbuf++;
             }
           } break;
      case defrea:
           {
             m.defrea = atoi(befbuf);
           } break;
#ifdef _BCMNET_LOGIN
      case pwonly:
           {
             m.pwonly = atoi(befbuf);
             if (m.pwonly > 1) m.pwonly = 0; // 0..1 is ok
           } break;
#endif
#ifdef _PREPOSTEXEC //db1ras
      case dospreexec:
           {
             befbuf[CMDBUFLEN] = 0;
             strcpy(m.dospreexec, befbuf);
           } break;
      case dospostexec:
           {
             befbuf[CMDBUFLEN] = 0;
             strcpy(m.dospostexec, befbuf);
           } break;
#endif
      default: break;
    }
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

void mbinit (void)
//*************************************************************************
//
//  Vorgabewerte fuer die globalen Mailbox-Variablen
//
//*************************************************************************
{
  FILE *f;
  int line = 0;
  char s[256];

  memset(&m, 0, sizeof(m));
  initzeit = ad_time();
  strcpy(m.boxname, "MYCALL");
  strcpy(m.boxadress, "MYCALL.#BAY.DEU.EU");
  strcpy(m.boxheader, STD_BOXHEADER);
  strcpy(m.sysopcall, "DL8MBT");
#ifdef _GUEST
  strcpy(m.guestcall, "off");
#endif
#ifdef _BCMNET_LOGIN
  m.callformat = 2;
#else
  m.callformat = 0; //default is amateur radio calls
#endif
#ifdef _LCF // JJ
#ifdef _BCMNET_LOGIN
  m.logincallformat = 2;
#else
  m.logincallformat = 0;
#endif
#endif
  m.savebroken = 0;
  strcpy(m.infopath, "info");
  strcpy(m.userpath, "user");
#ifdef FILESURF
  strcpy(m.fspath, "off");
  m.fsbmail = 1; // 0=BMAIL in FS disabled, 1=BMAIL enabled
#endif
#ifdef _BCMNET_LOGIN
  m.remerase = 2;
#else
  m.remerase = 1;
#endif
  m.maxbids = 8388608L; // 2^23
  m.bidoffset = 0;
  m.fwdssid = 8;
#ifdef _AX25K
  strcpy(m.ax25k_if, "off");
#endif
#ifdef __FLAT__
  m.http_port = DEFHTTPPORT;
  m.ftp_port  = DEFFTPPORT;
  m.smtp_port = DEFSMTPPORT;
  m.pop3_port = DEFPOP3PORT;
  m.nntp_port = DEFNNTPPORT;
  m.tcp_port  = DEFTELNPORT;
  m.serv_port = DEFSERVPORT;
  m.radio_port = 0;
  m.nopopsmtp = 0;       // 0=SMTP nur mit vorherigen POP,
                         // 1=SMTP auch ohne POP moeglich
  m.httpaccount = 0;     // 0="create account" bei guest-login nicht moeglich
                         // 1="create account" bei guest-login moeglich
  m.defhttpsurface = 1;  // 0=Frames, kein CSS Support
                         // 1=keine Frames, CSS Support
                         // 2=keine Frames, kein CSS Support
  m.httpguestfirst = 0;  // 0=bei httpd-Zugriff normaler Userlogin
                         // 1=immer erst Gastlogin bei httpd-Zugriff
#ifdef INETMAILGATE
  strcpy(m.internetmailgate, "off");
#endif
#ifdef _BCMNET_LOGIN
  m.httpttypw = 1;
#else
  m.httpttypw = 0;       // 0=bei httpd-Zugriff Unterscheidung Ampr/Internet:
                         //   Amprnet-Passwort="Name", sonst Passwort="TTYPW"
                         // 1=es wird immer das ttypw verlangt zum Einloggen
#endif
  m.httpshowsysopcall = 1; // 0=http sysopcallsign is not shown at guest send
                           //   dialog (antispam), 1=call is shown
  m.httprobots = 1;      // 0=http robots access denied, 1=allowed
  m.unsecurettypw = 0;   // 0=nur Sysop und User mit bereits gesetztem TTYPW
                         //   kann TTYPW aendern
                         // 1=wie 0, User mit PWOK koennen ebenfalls aendern
                         // 2=Fuer alle moeglich
#endif
  m.infolife = 999;
  m.userlife = 999;
  m.oldestfwd = 30;
  m.oldestbeacon = 1;
  m.fwdtimeout = 60;
#ifdef _BCMNET_LOGIN
  m.usrtimeout = 60;
#else
  m.usrtimeout = 0; //default no timeout
#endif
#ifdef _AUTOFWD
#ifdef _BCMNET_LOGIN
  m.autofwdtime = 90;
#else
  m.autofwdtime = 0;
#endif
#endif
#ifndef _BCMNET_LOGIN
  m.holdtime = 0; //default no hold
#else
  m.holdtime = 336;
#endif
  m.tracelevel = 1;
  m.fwdtrace = 0;
#ifdef __FLAT__
  m.tcpiptrace = 0;
#endif
#ifdef HB9EAS_DEBUG
  m.hb9eas_debug = 0;
#endif
  m.eraselog = 0;
  m.hadrstore = 2;
  m.disable = 1;
  m.sf_only = 0;
  m.disable_user = 0;
  m.usvsense = 0;
  m.watchdog = 6;
  m.timeslot = 2;
#if defined (FEATURE_SERIAL) || defined (_TELEPHONE)
  *m.ttymode = 0;
#endif
#ifdef _TELEPHONE
  m.ttydevice = 0;
#endif
#ifdef _AUTOTRCWIN
  m.autotrcwin = 0;
#endif
#ifndef __LINUX__
  m.summertime = 1;    // summertime is enabled=1 or disabled=0
#endif
#ifdef __DOS16__
  m.sysopbell = 0;
  m.crtsave = 0;
  m.pcisutc = 0;        // local time
  m.stimeoffset = 1;    // localtime offset (MEZ = 1)
  m.scrolldelay = 1;
  m.dosinput = 1;
#else
  m.dosinput = 0;
#endif
#ifdef __LINUX__
  m.addlinuxsystemuser = 0;  // 0 = deaktiviert, 1 = aktiviert
#endif
#ifdef _BCMNET_LOGIN
  m.paclen=128;
#else
  m.paclen = 0;
#endif
  m.minpaclen=40;
  m.maxpaclen=256;
#ifdef _BCMNET_LOGIN
  m.mailbeacon = 0;
#else
  m.mailbeacon = 1;      // 1=immer Mailbake ausgeben (auch leere), 0=Bake nur bei Usermails
#endif
  m.altboardinfo = 0;    // 0=Boardinfotext in eigener Zeile (mit Return), 1=ohne Return
  m.timeoutwarning = 1;  // 0=ohne Vorwarnung, 1=mit Vorwarnung
  m.nounknownroute = 0;  // 0=alle Routen annehmen, 1=unknown ablehnen
#ifdef _BCMNET_LOGIN
  m.asklogin = 1;
#else
  m.asklogin = 0;        // 0=ohne Loginabfrage, 1=mit Loginabfrage
#endif
  m.smoothheader = 0;    // 0=Mailheader wie bisher, 1=reduzierter Mailheader
  m.userquota = 0;
  m.maxlogins = 0;
#ifdef _BCMNET_LOGIN
  m.userpw = 0;
#else
  m.userpw = 1;
#endif
  m.readlock = 0;
  m.createboard = 1;
  m.nopurge = 0;
  m.defhttpsurface = 1;
#ifdef _BCMNET_LOGIN
  m.ltquery = 1;
#else
  m.ltquery = 0;
#endif
  strcpy(m.m_filter, "off");
#ifdef MAILSERVER
  m.maillistserv = 0;
  if (m.callformat == 0) // absoluter Wunsch DF3VI fuer Afu-Boxen Wert auf 0
    m.maillistsender = 0;
  else
    m.maillistsender = 1;
#endif
#ifdef DF3VI_POCSAG
  m.pocsaglistserv = 0;
#endif
  m.tellmode = 2;  //tell available
#ifdef _BCMNET_LOGIN
  m.oldumail = 1;
#else
  m.oldumail = 0;
#endif
  m.defswaplf = 2;
  m.defstatus = 0;
#ifdef FBBCHECKREAD
  m.deffbbcheckmode = 0;
#endif
  m.defhold = 0;
  strcpy(m.prompt, "(%b)-->");
  strcpy(m.firstcmd, "D");
  m.helplevel = 2;
  m.zeilen = 0;
  m.deflf = 0;
#ifdef _WXSTN
  strcpy(m.wxstnname, "OpenBCM");
  m.wxqthaltitude = 585L;
  m.wxsensoraltitude = 11L;
  strcpy(m.wxtty, "/dev/wxbcm");
  strcpy(m.wxpath, "wx");
#endif
#ifdef _MORSED
  strcpy(m.cwid, "= QST de BCM =");
  m.cwdoid = 0;
  *m.cwnextjob = 0;
  m.cwfreq = 800L; //800Hz
  m.cwms = 50;     //50ms
#endif
#ifdef _BCMNET_LOGIN
  m.defrea=2;
#else
  m.defrea=0;
#endif
#ifdef _BCMNET_LOGIN
  m.pwonly=0;
#endif
#ifdef _PREPOSTEXEC //db1ras
  strcpy(m.dospreexec, "off");
  strcpy(m.dospostexec, "off");
#endif
  // Add here new parameters if necessary.
  // Don't add it at the end of this definition!
  // ...
  m.opt[o_id] = o_a+    o_d+o_e+o_k+o_l+o_m+                o_w+o_y+o_z;
  m.opt[o_ud] = o_a+    o_d+    o_k+o_l+o_m+        o_t+    o_w+o_y+o_z;
  m.opt[o_il] = o_a+o_b+o_d+o_j+o_q+                o_t+    o_w+o_x+o_y;
  m.opt[o_ul] = o_a+o_b+o_d+o_j+o_q+                o_t+    o_w+o_x+o_y;
  m.opt[o_ir] = o_a+o_b+o_d+o_e+o_i+o_l+o_m+o_p+o_r+o_t+o_u+o_w+o_y+o_z;
  m.opt[o_ur] = o_a+o_b+o_d+o_e+o_i+o_l+o_m+o_p+o_r+o_t+o_u+o_w+o_y+o_z;
  m.opt[o_ch] = o_a+o_b+o_d+o_e+o_j+o_l+o_m+                o_w+o_x+o_y;
  m.opt[o_ps] = 0;
  if (file_isreg(MBINITOLDNAME) > file_isreg(MBINITNAME)) // only if newer
    xrename(MBINITOLDNAME, MBINITNAME);
  if ((f = s_fopen(MBINITNAME, "srt")) != NULL)
  {
    while (fgets(s, sizeof(s) - 1, f))
    {
      cut_blank(s);
      if (! mbinitbef(s, 1)) trace(replog, "ini", "line %d: %s", line, s);
      line++;
    }
    s_fclose(f);
  }
  else
  {
    /* create default directories on initial startup */
    xmkdir(m.userpath);
    xmkdir(m.infopath);
  #ifdef _WXSTN
    xmkdir(m.wxpath);
  #endif
  }
  mbparsave(); // save new defaults
  if (! m.mycalls && mbcallok(m.boxname))
  {
    sprintf(m.mycall[0], "%s-8", m.boxname);
    m.mycalls = 1;
  }
  if (m.mycalls == 1 && mbcallok(m.boxname))
  {
    sprintf(m.mycall[1], "%s-7", m.boxname);
    m.mycalls = 2;
  }
  align_mycalls();
  update_hadr(makeheader(1), -1, 0);
}

/*---------------------------------------------------------------------------*/

void mbparsave (void)
//*************************************************************************
//
//  Speichert die globalen Mailbox-Variablen in der Datei init.bcm
//
//*************************************************************************
{
  FILE *f;
  unsigned int i;

  if (! mbcallok (m.boxname) && (f=s_fopen(MBINITNAME, "srt")) != NULL)
  {
    trace(serious, "parsave", "BOXADDR not specified, not saved");
    s_fclose(f);
    return;
  }
  if ((f = s_fopen(INITTMPNAME, "swt")) != NULL)
  {
    s_fsetopt(f, 1);
    fprintf(f, "; --- mailbox ---\n");
    fprintf(f, "boxaddress %s\n", m.boxadress);
    fprintf(f, "boxheader %s\n", m.boxheader);
    fprintf(f, "sysopcall %s\n", m.sysopcall);
    if (m.callformat) //only save this parameter if not default
      fprintf(f, "callformat %d\n", m.callformat);
#ifdef _GUEST
    fprintf(f, "guestcall %s\n", m.guestcall);
#endif
#ifdef _LCF // JJ
    if (m.logincallformat) //only save this parameter if not default
      fprintf(f, "logincallformat %d\n", m.logincallformat);
#endif
    fprintf(f, "savebroken %d\n", m.savebroken);
    fprintf(f, "infopath %s\n", m.infopath);
    fprintf(f, "userpath %s\n", m.userpath);
#ifdef FILESURF // DH3MB, 30 Aug 1997
    fprintf(f, "fspath %s\n", m.fspath);
    fprintf(f, "fsbmail %d\n", m.fsbmail);
#endif
    fprintf(f, "remerase %u\n", m.remerase);
    fprintf(f, "maxbids %lu\n", m.maxbids);
    if (m.bidoffset)
      fprintf(f, "bidoffset %u\n", m.bidoffset);
    fprintf(f, "; --- packet interface ---\n");
    fprintf(f, "mycall");
    for (i = 0; i < m.mycalls; i++) fprintf(f, " %s", m.mycall[i]);
    fputc(LF, f);
    fprintf(f, "fwdssid %u\n", m.fwdssid);
#ifdef _AX25K
    fprintf(f, "ax25k_if %s\n", m.ax25k_if);
#endif
#ifdef __FLAT__
    fprintf(f, "; --- tcp/ip interface ---\n");
    fprintf(f, "http_port %d\n", m.http_port);
    fprintf(f, "ftp_port %d\n", m.ftp_port);
    fprintf(f, "smtp_port %d\n", m.smtp_port);
    fprintf(f, "pop3_port %d\n", m.pop3_port);
    fprintf(f, "nntp_port %d\n", m.nntp_port);
    fprintf(f, "telnet_port %d\n", m.tcp_port);
    fprintf(f, "serv_port %d\n", m.serv_port);
    fprintf(f, "radio_port %d\n", m.radio_port);
    fprintf(f, "nopopsmtp %d\n", m.nopopsmtp);
    fprintf(f, "httpaccount %d\n", m.httpaccount);
    fprintf(f, "httpguestfirst %d\n", m.httpguestfirst);
    fprintf(f, "httpttypw %d\n", m.httpttypw);
    fprintf(f, "httpshowsysopcall %d\n", m.httpshowsysopcall);
    fprintf(f, "httprobots %d\n", m.httprobots);
    fprintf(f, "unsecurettypw %d\n", m.unsecurettypw);
#ifdef INETMAILGATE
    fprintf(f, "internetmailgate %s\n", m.internetmailgate);
#endif
#endif
    fprintf(f, "; --- timers ---\n");
    fprintf(f, "infolife %u\n", m.infolife);
    fprintf(f, "userlife %u\n", m.userlife);
    fprintf(f, "oldestfwd %u\n", m.oldestfwd);
    fprintf(f, "oldestbeacon %u\n", m.oldestbeacon);
    fprintf(f, "fwdtimeout %u\n", m.fwdtimeout);
    fprintf(f, "usrtimeout %u\n", m.usrtimeout);
#ifdef _AUTOFWD
    fprintf(f, "autofwdtime %u\n", m.autofwdtime);
#endif
    fprintf(f, "holdtime %u\n", m.holdtime);
    fprintf(f, "; --- logging ---\n");
#ifdef HB9EAS_DEBUG
    fprintf(f, "hb9eas_debug %u\n", m.hb9eas_debug);
#endif
    fprintf(f, "tracelevel %u\n", m.tracelevel);
    fprintf(f, "fwdtrace %u\n", m.fwdtrace);
#ifdef __FLAT__
    fprintf(f, "tcpiptrace %u\n", m.tcpiptrace);
#endif
    fprintf(f, "eraselog %u\n", m.eraselog);
//    fprintf(f, "hadrstore %u\n", m.hadrstore); //fix 2
    fprintf(f, "; --- system ---\n");
    fprintf(f, "disable %d\n", m.disable);
    fprintf(f, "sfonly %d\n", m.sf_only);
//    if (m.disable_user) fprintf(f, "disableuser %d\n", m.disable_user);
    fprintf(f, "usvsense %d\n", m.usvsense);
    fprintf(f, "watchdog %d\n", m.watchdog);
    fprintf(f, "timeslot %d\n", m.timeslot);
#if defined (FEATURE_SERIAL) || defined (_TELEPHONE)
    fprintf(f, "ttymode %s\n", m.ttymode[0] ? m.ttymode : "off");
#endif
#ifdef _TELEPHONE
    fprintf(f, "ttydevice %d\n", m.ttydevice);
#endif
    fprintf(f, "dosinput %d\n", m.dosinput);
#ifdef _PREPOSTEXEC //db1ras
    fprintf(f, "dospreexec %s\n", m.dospreexec);
    fprintf(f, "dospostexec %s\n", m.dospostexec);
#endif
#ifdef _AUTOTRCWIN
    fprintf(f, "autotrcwin %d\n", m.autotrcwin);
#endif
#ifndef __LINUX__
    fprintf(f, "summertime %d\n", m.summertime);
#endif
#ifdef __DOS16__
    fprintf(f, "sysopbell %u\n", m.sysopbell);
    fprintf(f, "crtsave %u\n", m.crtsave);
    fprintf(f, "pcisutc %d\n", m.pcisutc);
    fprintf(f, "stimeoffset %d\n", m.stimeoffset);
    fprintf(f, "scrolldelay %d\n", m.scrolldelay);
#endif
#ifdef __LINUX__
    fprintf(f, "addlinuxsystemuser %d\n", m.addlinuxsystemuser);
#endif
    fprintf(f, "; --- user constraints ---\n");
    fprintf(f, "paclen %i\n", m.paclen);
    fprintf(f, "minpaclen %i\n", m.minpaclen);
    fprintf(f, "maxpaclen %i\n", m.maxpaclen);
    fprintf(f, "mailbeacon %d\n", m.mailbeacon);
    fprintf(f, "altboardinfo %d\n", m.altboardinfo);
    fprintf(f, "timeoutwarning %d\n", m.timeoutwarning);
    fprintf(f, "nounknownroute %d\n", m.nounknownroute);
    fprintf(f, "asklogin %d\n", m.asklogin);
    fprintf(f, "smoothheader %d\n", m.smoothheader);
    fprintf(f, "userquota %d\n", m.userquota);
    fprintf(f, "maxlogins %d\n", m.maxlogins);
    fprintf(f, "userpw %d\n", m.userpw);
    fprintf(f, "readlock %d\n", m.readlock);
    fprintf(f, "createboard %d\n", m.createboard);
    fprintf(f, "nopurge %u\n", m.nopurge);
    fprintf(f, "ltquery %u\n", m.ltquery);
    fprintf(f, "m_filter %s\n", m.m_filter);
#ifdef _BCMNET_LOGIN
    fprintf(f, "pwonly %i\n", m.pwonly);
#endif
    fprintf(f, "; --- server ---\n");
#ifdef MAILSERVER
    fprintf(f, "maillistserv %d\n", m.maillistserv);
    fprintf(f, "maillistsender %d\n", m.maillistsender);
#endif
#ifdef DF3VI_POCSAG
    fprintf(f, "pocsaglistserv %d\n", m.pocsaglistserv);
#endif
    fprintf(f, "tellmode %u\n", m.tellmode);
    fprintf(f, "oldumail %u\n", m.oldumail);
    fprintf(f, "defswaplf %u\n", m.defswaplf);
#ifdef _WXSTN
    fprintf(f, "; --- wx station ---\n");
    fprintf(f, "wxstnname %s\n", m.wxstnname);
    fprintf(f, "wxqthaltitude %ld\n", m.wxqthaltitude);
    fprintf(f, "wxsensoraltitude %ld\n", m.wxsensoraltitude);
    fprintf(f, "wxtty %s\n", m.wxtty);
    fprintf(f, "wxpath %s\n", m.wxpath);
#endif
#ifdef _MORSED
    fprintf(f, "; --- cw configuration ---\n");
    fprintf(f, "cwid %s\n", m.cwid);
    fprintf(f, "cwdoid %ld\n", m.cwdoid);
    fprintf(f, "cwnextjob %s\n", m.cwnextjob);
    fprintf(f, "cwfreq %ld\n", m.cwfreq);
    fprintf(f, "cwms %ld\n", m.cwms);
#endif
    fprintf(f, "; --- user defaults ---\n");
    fprintf(f, "defstatus %u\n", m.defstatus);
#ifdef FBBCHECKREAD
    fprintf(f, "deffbbcheckmode %u\n", m.deffbbcheckmode);
#endif
    fprintf(f, "defprompt %s\n", m.prompt);
    fprintf(f, "defcmd %s\n", m.firstcmd);
    fprintf(f, "defhelp %u\n", m.helplevel);
    fprintf(f, "deflines %u\n", m.zeilen);
    fprintf(f, "deflf %u\n", m.deflf);
    fprintf(f, "defidir %s\n", optstr(m.opt[o_id]));
    fprintf(f, "defudir %s\n", optstr(m.opt[o_ud]));
    fprintf(f, "defilist %s\n", optstr(m.opt[o_il]));
    fprintf(f, "defulist %s\n", optstr(m.opt[o_ul]));
    fprintf(f, "defiread %s\n", optstr(m.opt[o_ir]));
    fprintf(f, "defuread %s\n", optstr(m.opt[o_ur]));
    fprintf(f, "defcheck %s\n", optstr(m.opt[o_ch]));
    if (fprintf(f, "defps %s\n", optstr(m.opt[o_ps])) < 0) goto fehler;
    fprintf(f, "defrea %i\n", m.defrea);
    fprintf(f, "defhold %u\n", m.defhold);
    fprintf(f, "defhttpsurface %d\n", m.defhttpsurface);
    s_fclose(f);
    xunlink(INITBAKNAME);
    xrename(MBINITNAME, INITBAKNAME);
    xrename(INITTMPNAME, MBINITNAME);
  }
  else
    trace(serious, "mbparsave", "fopen errno=%d", errno);
  update_hadr(makeheader(1), -1, 0);
  return;
fehler:
  trace(serious, "mbparsave", "errno=%d", errno);
  s_fclose(f);
}
