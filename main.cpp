/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------
  Hauptprogramm der Mailbox
  -------------------------

  GNU GPL
  Copyright (C)       Florian Radlherr et al.
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW added "options" to version-cmd
//                added time in utc in "time" cmd
//19880118 OE3DZW sec in timestr
//19980124 OE3DZW renamed ELOG to ERLOG (for run-util EL) für DOS
//                added rejlog (Log of all rejects)
//                date/time in more detail opt -a,-f
//19980202 OE3DZW added support for callformat (output of "version")
//19980211 OE3DZW added option for serv
//19980212 OE3DZW moved "start"-msg to top of start-sequence
//19980308 OE3DZW cd with no parameter will cd to home directory
//19980311 DG9MHZ changes for Linux AX25-Kernel support
//19980329 OE3DZW new osver
//19980404 OE3DZW added options in version, os in startup-msg
//19980404 hrx    support for guests
//19980408 hrx    putversion() - line with cbmodeid changed
//                added checkcount to beftab/befnum/cases
//19980411 hrx    added autopath to beftab/befnum/cases, added
//                autofwd as installed options identifier
//19980418 OE3DZW added status ipsockets
//19980419 OE3DZW added command ? = help
//19980423 OE3DZW added serv-if to "info", removed "user" from fwd
//19980614 hrx    added paclen as installed options identifier,
//                changed autopath syntax help
//19980619 OE3DZW added parameter for m_commandnotavail
//19980707 DH3MB  Made runutils-interface optional (#define RUNUTILS)
//19980709 DH3MB  added "cmdlist"
//19980830 OE3DZW added obligatory string to command not avail - guest-bug
//19980909 OE3DZW fixed DH3MB's runutil-list, showed only last line
//19981013 OE3DZW commented out nntp server (not finished)
//19990117 OE3DZW added convat
//19990220 DK2UI  "lt" (=setlifetime)
//19990221 OE3DZW added loadaverage of system (Linux only)
//19990703 DH3MB  VERSION shows unix-time
//19990726 DF3VI  next will show next mail even if following mail is deleted
//19990814 DH3MB  Changed the behaviour of sending CRs after a command
//                has been received (see below for details)
//19990816 DH3MB  If box is disabled, also "MD2" and "MD5" can be used
//                for sysop authentication
//19991125 DF3VI  in mailbef(): don't call convers or logouts from convers
//19991220 Jan    new cmdnotavail(char *), prints invalid cmd msgs
//19991225 DF3VI  Added job j_tell, block commands you cannot tell.
//19999999 DF3VI  moved reject- and rlog-cmd to mbsysop!
//20000101 dk2ui  (dzw) added QT (quit without quit) and optional UI_LDN
//20000106 F1ULQ  "T" will start TALK, not show TIME
//20000116 DK2UI  (dzw) added ui-options
//20000414 DF3VI  Used Alter Grep for PS, need some changes calling listprocess
//20021211 hpk    if user is logged in as guest, some commands like erase,
//                unerase, ect. are not allowed for him
//20030726 DH8YMB added emptytempdir
//20041009 DH8YMB added CT (sets lastdirnews to current time)

#include "baycom.h"

mailpara_t m;
#ifdef _TELEPHONE // JJ
ttypara_t tty;
#endif
time_t einschaltzeit, initzeit;

/*---------------------------------------------------------------------------*/

static int near testaktiv (char *call, char *uplink)
//*************************************************************************
//
//  Testet, ob die Mailbox disabled/enabled ist
//
//*************************************************************************
{
  char s[LINELEN+1];

  if (   (m.disable && strcmp(uplink, "Import"))
      || (   m.sf_only && strcmp(uplink, "Import")
          && isforwardpartner(call) == NOTFOUND)) //df3vi: Nur S&F/SYSOPs
  {
    putf(ms(m_maintenance), m.boxname);
    b->msg_loadnum--;
    if (stricmp(call, m.sysopcall))
    {
      getline(s, LINELEN, 1); // accept only pw and sys
      if (! strncasecmp(s, "pw", 2) || ! strncasecmp(s, "sy", 2))
        getpw();
#ifdef FEATURE_MDPW
      else
        if (! stricmp(s, "md2"))
          getMDsysoppw(2);
        else
          if (! stricmp(s, "md5"))
            getMDsysoppw(5);
#endif
      return b->sysop;
    }
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

int checkquota (void)
//*************************************************************************
//
//  Testet, ob QUOTA erreicht wurde
//
//*************************************************************************
{
  if (m.userquota && ! b->sysop && (u->status != 1)
#ifdef FEATURE_SERIAL
     && strcmp(b->uplink, "TTY")
#endif
     )
  {
    if ((u->daybytes+b->rxbytes+b->txbytes) > ((unsigned) m.userquota*1000L))
    {
      putf(ms(m_quotaexceed), m.userquota);
      putf(ms(m_nobulletinmore));
      return OK;
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

static char *near makepath (char *path, char *part)
//*************************************************************************
//
//*************************************************************************
{
  if (*part == '/' || *part == '\\' || part[1] == ':') return part;
  strcat(path, part);
  strlwr(path);
  return path;
}

/*---------------------------------------------------------------------------*/

static char *mb (char *path, int total)
//*************************************************************************
//
//*************************************************************************
{
  static char s[30];
  long kb = dfree(path, total);
  if (kb < 10000L) sprintf(s, "%ld kB", kb);
  else sprintf(s, "%ld MB", kb >> 10);
  return s;
}

/*---------------------------------------------------------------------------*/

#ifdef __LINUX__

static void near putlddversion (void)
//*************************************************************************
//
// Gibt die Ausgabe von "LDD BCM" unter Linux zurueck
//
//*************************************************************************
{
  FILE *bcmfile;

  if (! (bcmfile = s_fopen("bcm", "srt")))
    putf("Can't access file 'bcm' - exiting lddversion.");
  else
  {
    putf("Used Libarys:\n");
    oshell("ldd bcm", sh_noinput);
    putf("\n");
    s_fclose(bcmfile);
  }

/*
#include "elf.h"
#include <sys/mman.h>
#include <sys/stat.h>

  Elf32_Ehdr *ehdr = NULL;
  struct stat statbuf;
  FILE *thefile;

  if (!(thefile = s_fopen("bcm", "srt")))
    putf("Can't open BCM");
  else
  {
    fstat(fileno(thefile), &statbuf);
    ehdr = (Elf32_Ehdr *)mmap(0, statbuf.st_size,
                              PROT_READ|PROT_WRITE, MAP_PRIVATE,
                              fileno(thefile), 0);
    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN)
      putf("This is not a dynamic executable %d\n", ehdr->e_type);
    else
      putf("This is a dynamic executable %d\n", ehdr->e_type);
  }
*/
}
#endif

/*---------------------------------------------------------------------------*/

static void near putversion (void)
//*************************************************************************
//
// Gibt Versionsinfo aus
//
//*************************************************************************
{
  char path[40];
  unsigned bs = strlen(globalpath) - 1;
#ifdef __LINUX__
  char osver[256];
  char *ptmp;
  FILE *f = s_fopen("/proc/version", "srt");
  if (f)
  {
    fgets(osver, sizeof(osver), f);
    s_fclose(f);
  }
  else *osver = 0;
#endif
  strcpy(path, globalpath);
  strlwr(path);
  putf("Date: %s\n", datestr(ad_time(), 12));
  putf("\n" LOGINSIGN);
  if (m.callformat)
   putf(" "CBMODEID);
  putf("\n" AUTHOR_ID);
#ifdef __FLAT__
  if (b->http)
    html_putf("<a href=" WEBSITE ">" WEBSITE "</a>");
  else
#endif
    putf(WEBSITE);
  putf("\n");
  if (m.callformat)
  {
    putf(CBSUPPORT);
#ifdef __FLAT__
    if (b->http)
      html_putf("<a href=" WEBSITE_CB ">" WEBSITE_CB "</a>");
    else
#endif
      putf(WEBSITE_CB);
    putf("\n");
  }
#ifdef __LINUX__
  putf("Compiled "__DATE__" "__TIME__ " with GCC "__VERSION__);
#else
  putf("Compiled "__DATE__" "__TIME__);
#endif
  char features[] = ""
  #ifdef FILESURF
      "filesurf "
  #endif
  #ifdef MAILSERVER
      "mailserver "
  #endif
  #ifdef DF3VI_POCSAG
      "pocsagserver "
  #endif
  #ifdef SERVIF
      "servif "
  #endif
  #ifdef RADIOIF
      "radioif "
  #endif
  #ifdef FEATURE_MDPW
      "mdpw "
  #endif
  #ifdef FEATURE_SERIAL
      "serial "
  #endif
  #ifdef FEATURE_EDITOR
      "editor "
  #endif
  #ifdef _TELEPHONE
      "telephone "
  #endif
  #ifdef _GUEST
      "guest "
  #endif
  #ifdef _AX25K
      "ax25k "
  #endif
  #ifdef _AUTOFWD
      "autofwd "
  #endif
  #ifdef _MORSED
      "morsed "
  #endif
  #ifdef RUNUTILS
      "runutils "
  #endif
  #ifdef MACRO
      "macro "
  #endif
  #ifdef _WXSTN
      "wxstn "
  #endif
  #ifdef FEATURE_YAPP
      "yapp "
  #endif
  #ifdef _FILEFWD
      "filefwd "
  #endif
  #ifdef _TELNETFWD
      "telnetfwd "
  #endif
  #ifdef FEATURE_DIDADIT
      "didadit "
  #endif
  #ifdef USERLT
      "userLT "
  #endif
  #ifdef BIOS_Y2K
      "y2k "
  #endif
  #ifdef DF3VI_FWD_EDIT         // fwd-editor
      "fwd-edit "
  #endif
  #ifdef DF3VI_REJ_EDIT         // reject.bcm-editor
      "rej-edit "
  #endif
  #ifdef DF3VI_CONV_EDIT        // convert.bcm-editor
      "conv-edit "
  #endif
  #ifdef DF3VI_EXTRACT          // extract-befehl
      "extract "
  #endif
  #ifdef _BCMNET_GW             // CB-BCMNET Gateway features
      "cb-bcmnet-gateway "
  #elif defined _BCMNET_FWD     // CB-BCMNET features
      "cb-bcmnet "
  #endif
  #ifdef FBBCHECKREAD
      "fbbcheckmode "
  #endif
  #ifdef _USERS4CONVERT
      "users4convert "
  #endif
  #ifdef DIEBOX_UIMPORT
    "diebox-uimport "
  #endif
  #ifdef FULLTEXTSEARCH
    "fts "
  #endif
  #ifdef OLDMAILIMPORT
    "oldmailimport "
  #endif
  #ifdef LINUXSYSTEMUSER
    "linuxsystemuser "
  #endif
      "";
  char *p_features = features;
#ifdef __LINUX__
  if (strlen(p_features)) putf("\nand support for:");
#else
  if (strlen(p_features)) putf(" with support for:");
#endif
  putv(LF);
  while (strlen(p_features) > 40 && strchr(p_features + 40, ' ')) // DH3MB
  {
    subst1(p_features + 40, ' ', 0);
    putf("%s\n", p_features);
    p_features += strlen(p_features) + 1;
  }
  if (strlen(p_features))
    putf("%s\n", p_features);
  putf("Uptime: %s\n", zeitspanne(ad_time() - einschaltzeit, zs_runtime));
  putf("%s\n", cpuinfo());
#ifdef __LINUX__
  ptmp = strchr(osver, '.');
  if (ptmp != NULL) {
	  ptmp = strchr(ptmp, ' ');
	  if (ptmp != NULL)
		  *ptmp = '\0';
  }
  putf("OS: %-40s\n", osver);
#endif
  putf("PR-Interface: ");
  putversion_tnc();
#ifdef __DPMI32__
  putf("Available Memory Extended/Real              : %s kB / %s kB\n",
                                                      memfree(0), memfree(1));
#elif defined(__DOS16__)
  putf("Available Memory                            : %s kB\n",
                                                      memfree(0));
#else
  putf("Available Memory / Used Swap                : %s kB / %s kB\n",
                                                      memfree(0), memfree(1));
#endif
  putf("Info-Path: %-32s : %s", makepath(path, m.infopath), mb(m.infopath,0));
  putf("  (%s)\n", mb(m.infopath, 1));
  path[bs + 1] = 0;
  putf("User-Path: %-32s : %s", makepath(path, m.userpath), mb(m.userpath,0));
  putf("  (%s)\n", mb(m.userpath, 1));
  path[bs + 1] = 0;
  if (bs > 3) path[bs] = 0;
  putf("BCM-Home:  %-32s : %s", path, mb(".", 0));
  putf("  (%s)\n", mb(".", 1));
#ifdef FILESURF
  unsigned int i, anz;
  char worte[MAXPARSE];
  char s1[FSPATHLEN+1];
  char *s2;
  strcpy(s1, m.fspath);
  anz = parseline(s1, worte);
  putf("Filesurf:  ");
  if (! anz)
    putf("disabled\n");
  else
  {
    for (i = 0; i < anz; i++)
    {
      s2 = (s1 + worte[i]);
      if (! strcmp(s2, "off"))
        putf("not defined\n");
      else
      {
        if (s2[0] == '+')
        {
          s2++;
          if (filetime(s2) > 0)
            putf("*%-31s : %s", makepath(path, s2), mb(s2, 0));
          else
            putf("*%-31s : not existing", makepath(path, s2));
        }
        else
          if (filetime(s2) > 0)
            putf("%-32s : %s", makepath(path, s2), mb(s2, 0));
          else
            putf("%-32s : not existing", makepath(path, s2));
        if (filetime(s2) > 0)
          putf("  (%s)\n", mb(s2, 1));
          else
            putf("\n");
        if (((i+1) < anz)) putf("           ");
      }
    }
  }
#endif
}

/*---------------------------------------------------------------------------*/

int mailbef (char *befline, int echo)
//*************************************************************************
//
//*************************************************************************
{
  int cmdnum;
  char cbuf[CMDBUFLEN+1];
  char *befbuf;
  char searchitem[31];
  char wlogbuf[40];
  char boxcall[CALLEN+1];
#ifdef __LINUX__
  char loadavg[256];
#endif
#ifdef __FLAT__
  char hostname[65];
#endif
#ifdef FILESURF
  filesurf fs;
#endif

  static char *beftab[]=
    { "ALTER", "DIR", "LIST", "SEND", "SA", "SB", "SP", "READ", "TALK", "WRITE", "WALL",
      "ERASE", "FORWARD", "TRANSFER", "REPLY", "HELP", "?", "HEADER", "KOPF",
      "CHECK", "CD", "QUIT", "CT", "QT", "BYE", "MSG", "MYBBS", "NEXT", "NH", "USERS", "TIME", "DATE",
      "AKTUELL", "PATH", "CONVERS", "CHAT", "UNERASE", "SETLIFETIME", "LT", "STATUS",
      "LOG", "INFO", "VERSION", "BIDLIST", "PS", "SEMAPHORES", "MEM", "PW", "SYSOP", "PRIV",
      "WHOAMI", "QUOTA", "PURGE", "PARAMETER", "FIND", "#OK#", "BIN-RX", "BIN-TX", "(BIN-RX):", "(BIN-TX):",
      "(YAPP-RX):", "(YAPP-TX):", "(DIDADIT-RX):", "(DIDADIT-TX):", "<GP>:", "(WPP)",
      "CRONTAB", "CONVAT", "REJECT", "CONVERT", "NAME", "EXIT", "F>", "FINGER", "SLOG", "SWAPLOG", "ERLOG",
      "UNKNOWN", "SLEEP", "SF", "MAN", "COMMENT", "FOLLOWUP", "LOGOUT", "PWLOG", "CP", "SFHOLD",
      "FWDCHECK",
#ifdef __FLAT__
      "NNTPLOG", "HTTPLOG", "POP3LOG", "SMTPLOG", "FTPLOG", "L2INFO",
#endif
#ifdef FULLTEXTSEARCH
      "FTS",
#endif
#ifdef _WXSTN
      "WX",
#endif
#ifdef HB9EAS_DEBUG
      "HB9EAS_DEBUG",
#endif
#ifdef __LINUX__
      "LDDVERSION",
#endif
#ifdef FEATURE_MDPW
      "MD2", "MD5",
#endif
      "RLOG",
#ifdef FILESURF
      "FILESURF", "FS", "FILESERVER",
#endif
#ifdef MAILSERVER
      "MAILSERVER",
#endif
#ifdef DF3VI_POCSAG
      "POCSAGSERVER",
#endif
      "CHECKCOUNT",
#ifdef _AUTOFWD
      "AUTOPATH",
#endif
#ifdef RUNUTILS
      "RUNUTILS",
#endif
      "TELL", "PING", "UNREAD", "IMPDEL", "CMDLIST", "WLOG",
      NULL
    };

  enum befnum
    { unsinn,
      alter, dir, list, send_, sa, sb, sp, read_, talk, write_, wall,
      erase, forward, transfer, reply, help, help_, header, kopf,
      check, cd, quit, ct, qt, bye, msg, mybbs, next, nh, users, showtime, showtime_,
      aktuell, path, convers_, chat, unerase, setlifetime, lt, status,
      log, info, version, bidlist, ps, semas, mem, pw, sysop, priv,
      whoami, quota, purge, parameter, find, d_ok, d_binrx, d_bintx, d_binrx2, d_bintx2,
      d_yapprx, d_yapptx, d_didaditrx, d_didadittx, d_gp, d_wpp,
      crontab, convat_, reject_, convert_, name, exit_, fwd_, finger, slog, swaplog, erlog,
      unknown, sleep_, sf, man, comment, followup, logout, pwlog_, cp, sfhold,
      fwdcheck_,
#ifdef __FLAT__
      nntplog_, httplog_, pop3log_, smtplog_, ftplog_, l2info,
#endif
#ifdef FULLTEXTSEARCH
      fts,
#endif
#ifdef _WXSTN
      _wx,
#endif
#ifdef HB9EAS_DEBUG
      hb9eas_debug,
#endif
#ifdef __LINUX__
      lddversion,
#endif
#ifdef FEATURE_MDPW
      md2, md5,
#endif
      rlog,
#ifdef FILESURF
      filesurf_, fs_, fileserver,
#endif
#ifdef MAILSERVER
      mailserver,
#endif
#ifdef DF3VI_POCSAG
      pocsagserver,
#endif
      checkcount,
#ifdef _AUTOFWD
      autopath_,
#endif
#ifdef RUNUTILS
      runutils,
#endif
      send_tell_, ping, unread, impdel, cmdlist, wprotlog_
    } cmd = unsinn;

  if (strlen(befline) > CMDBUFLEN)
  {
    befline[CMDBUFLEN] = 0;
  }
  // use ESCAPE-[-A (upper arrow) to get back to last cmd-line
  if (*befline == ESC
      && befline[1] == '['
      && befline[2] == 'A'
      && *t->lastcmd)
    strcpy(befline, t->lastcmd);
  strncpy(cbuf, befline, CMDBUFLEN);
  cbuf[CMDBUFLEN] = 0;
  lastcmd(befline);
  if (b->quit)
    return 1;
  cut_blank(cbuf);
  befbuf = cbuf;
  befbuf += blkill(befbuf);
  if (echo && u->echo && t->input == io_tnc)
    putf("%s", befbuf);
#ifdef FILESURF
  if (b->fsmode)
  {
    fs.execute(befbuf);
    return 0;
  }
#endif
  if (! *befbuf)
  {
    if (u->lf != 6)
      putv(LF);
    b->eingabefehler++;
    return 1;
  }
  trace(report, b->logincall, "%s", befbuf);
  if (b->sysop)
    cmdlog(befbuf);
#ifdef MACRO
  if (mk_perform(MK_CMD, befbuf))
    return 0;
  if (b->sysop && mk_perform(MK_SCMD, befbuf))
    return 0;
#endif
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  switch (cmd)
  {
    case bye:
    case quit:
    case exit_:
    case logout:
    case read_:
    case unsinn:
               break;
    default:
               if (u->lf != 6)
                 putv(LF);
               if (cmd != pw && cmd != sysop && b->job != j_convers)
                 leerzeile(); // Sends the number of CRs stored in "ALTER LF"
  }
//Negative lists of commands (not available in some cases)

//*** GUEST ***
#ifdef _GUEST
    if (! strcmp(b->logincall, m.guestcall))
    switch (cmd)  //list of commands not available for guests
    {
      case talk:
      case msg:
      case chat:
      case convers_:
      case write_:
      case wall:
      case priv:
      case pw:
      case sysop:
#ifdef FEATURE_MDPW
      case md2:
      case md5:
#endif
#ifdef MAILSERVER
      case mailserver:
#endif
#ifdef DF3VI_POCSAG
      case pocsagserver:
#endif
               {
                 b->eingabefehler += 5;
                 putf(ms(m_cmdinvalid), befline);
                 return 1;
               }
      default: break;
    }
#endif
//*** SPAMER ***
    if (u->status & 8)
    switch (cmd)  //list of commands not available for spamer
    {
      case wall:
      case cp:
      case transfer:
#ifdef FILESURF
      case filesurf_:
      case fs_:
      case fileserver:
#endif
#ifdef MAILSERVER
      case mailserver:
#endif
#ifdef DF3VI_POCSAG
      case pocsagserver:
#endif
               {
                 b->eingabefehler += 5;
                 putf(ms(m_cmdinvalid), befline);
                 return 1;
               }
      default: break;
    }
//*** TELL ***
    if (b->job == j_tell) //list of commands not available for "tell"
    switch (cmd)
    {
      case next:
      case purge:
      case nh:
      case mybbs:
      case bye:
      case exit_:
      case logout:
      case quit:
      case ct:
      case qt:
      case sb:
      case sp:
      case sa:
      case send_:
      case reply:
      case followup:
      case comment:
      case name:
      case wall:
      case talk:
      case msg:
      case chat:
      case convers_:
      case write_:
      case cp:
      case transfer:
#ifdef FILESURF
      case filesurf_:
      case fs_:
      case fileserver:
#endif
      case fwd_:
      case sleep_:
      case impdel:
#ifdef MAILSERVER
      case mailserver:
#endif
      case priv:
      case pw:
      case sysop:
#ifdef FEATURE_MDPW
      case md2:
      case md5:
#endif
      case help:
      case help_:
      case man:
               {
                 b->eingabefehler += 5;
                 putf(ms(m_cmdinvalid), befline);
                 return 1;
               }
      default: break;
    }
//*** CONVERS ***
    if (b->job == j_convers) //list of commands not available during convers
    switch (cmd)
    {
      case bye:
      case exit_:
      case logout:
      case quit:
      case ct:
      case qt:
      case talk:
      case msg:
      case chat:
      case convers_:
      case write_:
      case wall:
      case priv:
      case pw:
#ifdef FEATURE_MDPW
      case md2:
      case md5:
#endif
#ifdef FILESURF
      case filesurf_:
      case fs_:
      case fileserver:
#endif
               {
                 b->eingabefehler += 5;
                 putf(ms(m_cmdinvalid), befline);
                 return 1;
               }
      default: break;
    }
#ifdef __FLAT__
//*** HTTP ***
  if (! strcmp (b->uplink, "HTTP")) //list of commands not available via http
    switch (cmd)
    {
      case next:
      case bye:
      case exit_:
      case logout:
      case quit:
      case reply:
      case comment:
      case followup:
      case talk:
      case msg:
      case chat:
      case convers_:
      case write_:
      case priv:
      case pw:
      case sysop:
#ifdef FEATURE_MDPW
      case md2:
      case md5:
#endif
#ifdef FILESURF
      case sleep_:
      case filesurf_:
      case fs_:
      case fileserver:
#endif
               {
                 b->eingabefehler += 5;
                 putf(ms(m_http_not));
                 return 1;
               }
      default: break;
    }
#endif
//*** WITHOUT PW ***
  if (m.userpw >= 2 && ! b->pwok && ! b->sysop)
   switch (cmd) // list of command not available without pw
   {
     case send_:
     case sp:
     case sb:
     case sa:
     case reply:
     case followup:
     case comment:
     case erase:
     case unerase:
     case transfer:
     case cp:
     case purge:
     case priv:
     case sysop:
#ifdef FEATURE_MDPW
     case md2:
     case md5:
#endif
               {
                 putf(ms(m_no_pw));
                 return 1;
               }
    default:   break;
   }
#ifdef _BCMNET_LOGIN
//*** WRONG LOGINTYPE ***
  if (! b->logintype && ! b->sysop)
   switch (cmd) //list of commands not available with wrong logintype
   {
     case erase:
     case unerase:
     case transfer:
     case cp:
     case lt:
     case purge:
               {
                 putf(ms(m_no_pw));
                 return 1;
               }
    default:   break;
   }
#endif
  mk_perform(MK_BEFORECMD, cbuf);
  switch (cmd)
  {
    case unsinn:
               if (
#ifdef RUNUTILS
                   ! start_runutil(cbuf) &&
#endif
                   ! (b->sysop && sysbef(cbuf)))
               {
                 if (u->lf != 6)
                   putv(LF);
                 putf(ms(m_cmdinvalid), befline);
                 if (u->helplevel)
                   putf(ms(m_possiblecommands));
                 b->eingabefehler += 5;
                 return 1;
               }
               break;
    case quota:
               mbalter("QUOTA", befbuf, b->logincall);
               break;
    case purge:
               if (! b->sysop)
                 *befbuf = 0;
               userpurge(befbuf);
               break;
    case nh:
    case mybbs:
               if (*befbuf)
                 mbalter("FORWARD", befbuf, b->logincall);
               else
                 putf(ms(m_syntaxmybbs));
               break;
    case finger:
    case users:
               if (mbcallok(befbuf) == 1)
                 display_parameter(befbuf, 0);
               else
                 if (! *befbuf)
                   listusers(1);
                 else
                   if (*befbuf == '*')
                     listusers(2);
                   else
                   {
                     scanoptions(befbuf);
                     befbuf += blkill(befbuf);
                     usersuch(befbuf);
                   }
               break;
    case alter:
               mbalter(NULL, befbuf, b->logincall);
               break;
    case bye:
               b->quit = 1;
               break;
    case exit_:
    case logout:
    case quit:
               if (cmd == quit)
                 b->quit = 2;
               else
                 b->quit = 1;
               loaduser(b->logincall, u, 1);
               u->lastdirnews = b->lastdirnews;
               u->lastquit = ad_time();
               saveuser(u);
               break;
    case ct:
#ifdef _GUEST
               if (! strcmp(b->logincall, m.guestcall))
               {
                 putf(ms(m_cmdinvalid), "ct");
                 break;
               }
               else
#endif
               {
                 loaduser(b->logincall, u, 1);
                 u->lastdirnews = ad_time();
                 b->lastdirnews = ad_time();
                 saveuser(u);
                 putf("Last D N (CHECK) is set to:       %s\n",
                       datestr(u->lastdirnews, 12));
                 break;
               }
    case qt:
#ifdef _GUEST
               if (! strcmp(b->logincall, m.guestcall))
               {
                 putf(ms(m_cmdinvalid), "qt");
                 break;
               }
               else
#endif
               {
                 loaduser(b->logincall, u, 1);
                 u->lastdirnews = b->lastdirnews;
                 u->lastquit = ad_time();
                 saveuser(u);
                 putf("Last QUIT is set to current time: %s\n",
                       datestr(u->lastquit, 12));
                 break;
               }
    case send_:
    case sb:
    case sp:
    case sa:
               if (cmd == send_)
                 b->mailtype = '?';
               else
                 if (cmd == sb)
                   b->mailtype = 'B';
                 else
                   if (cmd == sp)
                     b->mailtype = 'P';
                   else
                     if (cmd == sa)
                       b->mailtype = 'A';
               b->job = j_send;
           /*    if (b->http > 0)
               {
                 putf("Not possible in HTTP session!\n");
                 break;
               }
           */
               if (*befbuf)
                 mbsend(befbuf, 1);
               else
                 putf(ms(m_syntaxsend));
               break;
    case reply:
               b->job = j_send;
               if (b->replynum || *befbuf)
                 mbchange(befbuf, w_reply, 1);
               else
                 putf(ms(m_noreplypossible));
               break;
    case comment:
    case followup:
               b->job = j_send;
               if (b->replynum || *befbuf)
                 mbchange(befbuf, w_comment, 1);
               else
                 putf(ms(m_noreplypossible));
               break;
    case send_tell_:
               if (m.tellmode)
                 send_tell(befbuf); //only tell if sysop allows tell
               else
                 putf(ms(m_notavailable), "TELL");
               break;
    case ping:
               send_ping(befbuf);
               break;
    case name:
               mbalter("NAME", befbuf, b->logincall);
               break;
    case next:
               scanoptions(befbuf);
               if ((b->optplus & o_r) && b->replynum > 1)
                 sprintf(befbuf, "%s %d", b->replyboard, b->replynum -= 1);
               else
                 sprintf(befbuf, "%s %d", b->replyboard, b->replynum += 1);
               // fall through
    case read_:
    case kopf:
    case header:
               if (b->job != j_tell)
                 // bei Tell Job-Kennzeichnung erhalten fuer Abbruch nach EINER Mail
                 b->job = j_read;
               b->optgroup = o_ir;
               mbread(befbuf, (cmd == read_ || cmd == next) ? 0 : 1);
               break;
    case erase:
               mbchange(befbuf, w_erase, 1);
               break;
    case unerase:
               mbchange(befbuf, w_unerase, 1);
               break;
    case unread:
               mbchange(befbuf, w_unread, 1);
               break;
    case lt:
    case setlifetime:
               mbchange(befbuf, w_setlifetime, 1);
               break;
    case forward:
               mbchange(befbuf, w_forward, 1);
               break;
    case cp:
    case transfer:
               mbchange(befbuf, w_transfer, 1);
               break;
    case list:
               b->job = j_search;
               b->optgroup = o_il;
               mbdir(befbuf, 1);
               break;
    case dir:
               b->job = j_search;
               b->optgroup = o_id;
               mbdir(befbuf, 0);
               break;
    case check:
               {
                 strcpy(searchitem, "");
                 time_t times = 0;
                 b->job = j_search;
                 b->optgroup = o_ch;
                 scanoptions(befbuf);
                 b->usermail = 0;
                 formoptions();
                 befbuf += blkill(befbuf);
                 if (! *befbuf && ! (b->optplus & o_r))
                 {
                   loaduser(b->logincall, u, 1);
                   times = u->lastdirnews;
                   b->lastdirnews = ad_time();
                 }
                 findbereich(befbuf);
                 get_searchitem(befbuf, searchitem, 30);
                 befbuf += blkill(befbuf);
                 if (*searchitem)
                   mbcheck(searchitem, times);
                 else
                   mbcheck(befbuf, times);
               }
               break;
    case checkcount:
               putf(ms(m_entriesfound), mbcheckcount());
               break;
#ifdef _AUTOFWD
    case autopath_:
               if (*befbuf)
                 autofwd(befbuf, 1);
               else
                 putf("Syntax: AUTOPATH [<options>] <boxcall>\n");
               break;
#endif
    case status:
               {
                 char opt = toupper(*befbuf);
                 while (isalpha(*befbuf))
                   befbuf++;
                 befbuf += blkill(befbuf);
                 switch (opt)
                 {
                   case 'F':
                             fwdstat(befbuf);
                             break;
                   case 'C':
                             put_cpuload();
#ifdef __LINUX__
                             {
                               FILE *f = s_fopen("/proc/loadavg", "srt");
                               if (f)
                               {
                                 fgets(loadavg, sizeof(loadavg), f);
                                 s_fclose(f);
                                 putf("CPU: loadavg %s", loadavg);
                               }
                             }
#endif
                             break;
                   case 'S':
                             sema_list();
                             break;
                   case 'L':
                             limit_list();
                             break;
                   case 'O':
                             openfile_list();
                             break;
                   case 'M':
                             t_alloclist(befbuf);
                             break;
#ifdef __FLAT__
                   case 'I':
                             show_sock();
                             break;
                   case 'P':
                             {
                               pop3 p;
                               p.show_popdb();
                             }
                             break;
#endif
                   default:
                             putf("Syntax: STATUS FORWARD\n"
                                  "        STATUS CPU\n"
                                  "        STATUS LIMITS\n"
                                  "        STATUS MEMORY\n"
                                  "        STATUS OPENFILES\n"
                                  "        STATUS SEMAPHORES\n"
#ifdef __FLAT__
                                  "        STATUS IPSOCKETS\n"
                                  "        STATUS POPSTATE\n"
#endif
                                 );
                 }
               }
               break;
    case crontab:
               scanoptions(befbuf);
               putlog(CRONTABNAME, befbuf);
               break;
    case convat_:
               scanoptions(befbuf);
               putlog(CONVATNAME, befbuf);
               break;
    case convert_:
               scanoptions(befbuf);
               putlog(CONVFILENAME, befbuf);
               break;
    case help:
    case help_:
    case man:
               mbreadhelp(befbuf);
               break;
    case cd:
               if (! *befbuf)
                 strcpy(befbuf, b->logincall);
               strupr(befbuf);
               befbuf[MAXTREELENGTH] = 0;
               if (! strchr(befbuf, '/') && ! mbcallok(befbuf))
                 rubrik_completition(befbuf);
               if (! finddir(befbuf, b->sysop))
                 putf(ms(m_dirunknown), befbuf);
               else
                 strcpy(b->prompt, b->boardfullname);
               break;
    case showtime:
    case showtime_:
               {
                 time_t tt = ad_time();
                 putf("%s\n", datestr(tt, 14));
                 scanoptions(befbuf);
                 befbuf += blkill(befbuf);
                 if (b->optplus & o_a)
                 {
                   putf("\nLocal time:\n");
                   putf(" ANSI: %ld s (1970-01-01)\n", tt);
/*                   putf("      %ld\n", (tt-ad_timezone()));
                   putf("      %ld\n", loctime());
                   putf("      %ld\n", ad_time());
                   putf("      %ld\n", time(NULL)); */
                   putf(" File: %s\n", time2filename(tt));
                   putf(" ISO : %s\n", datestr(tt, 15));
                   putf("\nTimezone:\n");
#ifndef __LINUX__
                   putf("TZ : UTC%+03d:%02d",
                         (int) (ad_timezone() / HOUR),
                         abs((int) (ad_timezone() % 60L)));
                   if (m.summertime)
                     putf(" (summer time autocorrection is activated if needed)\n");
                   else
                     putf("\n");
#else
                   putf(" TZ : \"%s\" (UTC%+03d:%02d)\n",
                        *tzname[1] ? tzname[1] : tzname[0],
                        (int) (ad_timezone() / HOUR),
                        abs((int) (ad_timezone() % 60L)));
#endif
                   putf("\nUTC:\n");
#ifdef UTC_TIME
                   putf(" ANSI: %ld s (1970-01-01)\n", tt);
                   putf(" ISO : %-19.19sz\n", datestr(tt, 15));
#else
#ifndef __FLAT__ //DOS
                   putf(" ANSI: %ld s (1970-01-01)\n", (tt-(5*HOUR)));
                   putf(" ISO : %-19.19sz\n", datestr(tt-ad_timezone(), 15));
#else
                   putf(" ANSI: %ld s (1970-01-01)\n", (tt+ad_timezone()));
                   putf(" ISO : %-19.19sz\n", datestr(tt-ad_timezone(), 15));
#endif
#endif
#ifdef __FLAT__
                   putf(" RFC : %s\n", datestr(tt-ad_timezone(), 13));
#endif
                 }
               }
               break;
    case aktuell:
         {
           putaktuell(1);
         } break;
    case info:
         {
           putf("Info for BBS %s [%s]\n", m.boxadress, m.boxheader);
           putf("Sysop:       %s %s\n", m.sysopcall,
                                        get_name(m.sysopcall, 1));
           putf("Logincall:   %s\n", m.mycall[0]);
           if (*m.mycall[1])
             putf("Forward:     %s\n", m.mycall[1]);
#ifdef __FLAT__
           if (myhostname(hostname, 64) != -1)
           { //strcat(hostname, ".");
             //if (getdomainname(hostname+strlen(hostname), 64-strlen(hostname)))
             hostname[65] = 0;
             if (m.tcp_port)
               putf("Telnet:      %s:%d\n", hostname, m.tcp_port);
             if (m.http_port)
               putf("HTTP:        %s:%d\n", hostname, m.http_port);
             if (m.ftp_port)
               putf("FTP:         %s:%d\n", hostname, m.ftp_port);
             if (m.smtp_port)
               putf("SMTP:        %s:%d\n", hostname, m.smtp_port);
             if (m.pop3_port)
               putf("POP3:        %s:%d\n", hostname, m.pop3_port);
#ifdef NNTP
             if (m.nntp_port)
               putf("NNTP:        %s:%d\n", hostname, m.nntp_port);
#endif
#ifdef RADIOIF
             if (m.radio_port)
               putf("Radio:       %s:%d\n", hostname, m.radio_port);
#endif
#ifdef SERVIF
             if (m.serv_port)
               putf("Service:     %s:%d\n", hostname, m.serv_port);
#endif
           }
#endif
           putv(LF);
           readtext("info");
         } break;
    case find:
         {
           showpath(befbuf, 2);
         } break;
    case path:
         {
           showpath(befbuf, 1);
         } break;
#ifdef FULLTEXTSEARCH
    case fts:
         {
           strcpy(searchitem, "");
           b->job = j_search;
           b->optgroup = o_ch;
           scanoptions(befbuf);
           b->usermail = 0;
           formoptions();
           findbereich(befbuf);
           get_searchitem(befbuf, searchitem, 30);
           befbuf += blkill(befbuf);
           if (*befbuf)
           {
             if (*searchitem)
             {
               // Bei Boardangabe Bereichsangabe ignorieren, da
               // sich diese nur auf Checknum beziehen wuerde, nicht
               // auf die Board-Nummerierung, wie ein User denken wuerde
               b->beginn = 1;
               b->ende = MAXBEREICH;
               mbfts(befbuf, searchitem);
             }
             else
               mbfts(befbuf, "");
           }
           else
             putf("Syntax: fts [<range>] <searchitem> [\"<board>\"]\n");
         } break;
#endif
    case parameter:
         {
           scanoptions(befbuf);
           //df3vi: wenn nicht explizit "-i-", ist "-i" default
           if (b->optminus & o_i)
             b->optplus |= o_i;
           grep(MBINITNAME, befbuf, b->optplus);
         } break;
    case talk:
    case msg:
    case chat:
    case convers_:
    case write_:
         {
           convers(befbuf, beftab[cmd - 1]);
         } break;
    case wall:
         {
           if (*befbuf)
             mbtalk(b->logincall, "ALL", befbuf);
           else
             putf(ms(m_syntaxwall));
         } break;
    case log:
         {
           scanoptions(befbuf);
           putlogauszug(befbuf);
         } break;
    case version:
         {
           putversion();
         } break;
#ifdef HB9EAS_DEBUG
    case hb9eas_debug:
         {
           if (b->sysop == 1 || ! strcmp(b->logincall, "DK2GO"))
           {
             m.hb9eas_debug = atoi(befbuf);
             if (m.hb9eas_debug == 1)
               putf("ACTIVE ROUTING to HB9EAS is activated!\n");
             else
             {
               putf("ACTIVE ROUTING to HB9EAS is deactivated!\n");
               m.hb9eas_debug = 0;
             }
             break;
           }
           else
             putf("Only sysops and dk2go can set this parameter!\n");
         } break;
#endif
#ifdef __LINUX__
    case lddversion:
         {
           putlddversion();
         } break;
#endif
    case bidlist:
         {
           listbids(befbuf);
         } break;
    case ps:
         {
           scanoptions(befbuf);
           b->optgroup = o_ps;
           formoptions();
           listprocess(b->opt);
         } break;
    case semas:
         {
           sema_list();
           break;
         }
    case mem:
         {
           t_alloclist(befbuf);
         } break;
    case whoami:
         {
           putf("You are %s(%s).\n", get_name(b->logincall, 0),
                                     b->logincall);
#ifdef _BCMNET_LOGIN
           if (b->logintype != 1)
             putf("Logged in as guest.\n");
#endif
         } break;
    case priv:
         {
           if (getpriv(befbuf, PRIVNAME))
           {
             b->sysop = 1;
             cmdlog("******** PRIV ok");
             pwlog(b->logincall, b->uplink, "sysop priv ok");
             testdfull();
           }
           else
             putf("Login: %s\n", datestr(b->logintime, 10));
         } break;
    case pw:
               // fuer Login-PW nach DP-Box-Art: USERPW = 3
               // Login immer moeglich, PW nur fuer bestimmte Aktionen noetig
               if (m.userpw == 3)
               {
                 if (   *u->password > 1 && ! t->socket
#ifdef _TELEPHONE
                     && strcmp(b->uplink, "TTY")
#endif
                    )
                 {
                   putv(LF);
                   char pwok = FALSE;
                   switch(u->loginpwtype) // DH3MB
                   {
                     case 0: pwok = pwauswert(u->password); break;
#ifdef FEATURE_MDPW
                     case 1: pwok = getMDpw(2, u->password); break;
                     case 2: pwok = getMDpw(5, u->password); break;
#endif
                     default: pwok = FALSE;
                   }
                   if (! pwok)
                     pwlog(b->logincall, b->uplink, "userpw failure");
                   b->pwok = pwok;
                   break;
                 }
                 if (*u->password <= 1 && ! b->sysop)
                 {
                   readtext("pwonly");
                   putv(LF);
                   break;
                 }
               }
    case sysop:
               if (tolower(*befbuf) == 'o')
                 b->sysop = FALSE;
               else
                 getpw();
               break;
#ifdef FEATURE_MDPW
      case md2:
               getMDsysoppw(2);
               break;
      case md5:
               getMDsysoppw(5);
               break;
#endif
    case fwd_:
               b->forwarding = fwd_user;
               fwdmainloop();
               b->quit = 1;
               break;
    case fwdcheck_:
               fwdcheck(befbuf);
               break;
    case slog:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" SYSLOGNAME, befbuf);
               break;
    case rlog:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" REJECTLOGNAME, befbuf);
               break;
    case erlog:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" ERASELOGNAME, befbuf);
               break;
    case pwlog_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" PWLOGNAME, befbuf);
               break;
#ifdef __FLAT__
    case httplog_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" HTTPLOGNAME, befbuf);
               break;
    case nntplog_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" NNTPLOGNAME, befbuf);
               break;
    case pop3log_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" POP3LOGNAME, befbuf);
               break;
    case smtplog_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" SMTPLOGNAME, befbuf);
               break;
    case ftplog_:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" FTPLOGNAME, befbuf);
               break;
#ifndef _AX25K_ONLY
    case l2info:
               putl2info();
               break;
#endif
#endif
    case swaplog:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" SWAPLOGNAME, befbuf);
               break;
    case unknown:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" UNKNOWNNAME, befbuf);
               break;
    case sfhold:
               scanoptions(befbuf);
               putlog(TRACEPATH "/" SFHOLDNAME, befbuf);
               break;
    case reject_:
               scanoptions(befbuf);
               putlog(REJECTNAME, befbuf);
               break;
    case wprotlog_:
               wlogbuf[0] = 0;
               boxcall[0] = 0;
               befbuf = nexttoken(befbuf, boxcall, CALLEN);
               scanoptions(befbuf);
               if (*boxcall && mbcallok(boxcall))
               {
                 strlwr(boxcall);
                 sprintf(wlogbuf, WLOGPATH "/r_%s.bcm", boxcall);
                 putlog(wlogbuf, befbuf);
               }
               break;
    case sf: // DH3MB: only sysops may start S&F (except user-S&F)
               strupr(befbuf);
               if (! *befbuf)
               {
                 if (get_ufwd(b->logincall)[0])
                 {
                   putf("Starting User-S&F.\n");
                   fork(P_BACK | P_MAIL, 0, fwdsend, b->logincall);
                 }
                 else
                   putf("User-S&F not enabled.\n");
                 break;
               }
               // hpk: only sysops can start the s&f
               if (b->sysop != 1)
               {
                 putf("Only sysops can start forward.\n");
                 break;
               }
               if (! stricmp(befbuf, "ALL"))
               {
                 putf("Starting forward to all partners.\n");
                 fork(P_BACK, 0, startfwd, "Forward");
                 break;
               }
               if (get_ufwd(befbuf)[0] || isforwardpartner(befbuf) >= 0)
               {
                 putf("Starting forward to %s.\n", befbuf);
                 fork(P_BACK | P_MAIL, 0, fwdsend, befbuf);
               }
               else
                 putf("Syntax: SF <adjacent bbs call>\n");
               break;
    case sleep_:
               if (isdigit(*befbuf))
                 wdelay(1000 * atoi(befbuf));
               else
                 putf("Syntax: SLEEP <seconds>\n");
               break;
#ifdef FILESURF
    case filesurf_:
    case fs_:
    case fileserver:
               if (! *befbuf && *m.fspath != 'O' && *m.fspath != 'o')
                 strcpy(befbuf, "cmdmode");
               fs.execute(befbuf);
               break;
#endif
#ifdef MAILSERVER
    case mailserver:
               {
                 mailserv ma;
                 ma.execute(befbuf);
               }
               break;
#endif
#ifdef DF3VI_POCSAG
    case pocsagserver:
               {
                 pocsagserv poc;
                 poc.execute(befbuf);
               }
               break;
#endif
#ifdef RUNUTILS
    case runutils:
               list_runutils(befbuf);
               break;
#endif
#ifdef _WXSTN
    case _wx:
               {
                 wx wxstn;
                 wxstn.mbwxcmd(befbuf);
               }
               break;
#endif
    case impdel: // DH3MB
               if (t->input == io_file && b->inputfile)
                 s_fsetopt(b->inputfile, 2);
               break;
    case cmdlist:
               {
                 cmdnum = 0;
                 while (beftab[cmdnum])
                   putf("(%2d) %s\n", ++cmdnum, beftab[cmdnum]);
               }
               break;
    default:
               break;
  }
  mk_perform(MK_AFTERCMD, cbuf);
  b->eingabefehler = 0;
  return 0;
}

/*---------------------------------------------------------------------------*/

int test_job (job_typ job)
//*************************************************************************
//
//*************************************************************************
{
  if (b->job == job)
  {
    putf(ms(m_cmdinvalid), "");
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void mbmainloop (int doprompt)
//*************************************************************************
//
//*************************************************************************
{
#ifdef __FLAT__
  char s[CBUFLEN+1];
#else
  char s[LINELEN+1];
#endif
#ifdef FILESURF
  filesurf fs;
#endif

  b->job = j_idle;
  while (! b->quit)
  {
    if (doprompt)
    {
      leerzeile();
#ifdef FILESURF
      if (b->fsmode)
        fs.putprompt();
      else
      {
#endif
        if (u->helplevel)
          putf(ms(m_mainmenu));
        putf("%s", expandmakro(u->prompt, s));
#ifdef FILESURF
      }
#endif
      putflush();
    }
    if (getvalid())
    {
#ifdef __FLAT__
      getline(s, -CBUFLEN, 0); //bei negativer Laenge Trennung mit ; zulassen
#else
      getline(s, -LINELEN, 0); //bei negativer Laenge Trennung mit ; zulassen
#endif
      b->job = j_cmd;
      rm_crlf(s);
      char *kommando_buffer;
      char *last;
      if (strpos(s, ";") > 0)
      {
        kommando_buffer = safestrtok(s, ";", &last);
        while (kommando_buffer != NULL)
        {
          mailbef(kommando_buffer, 1);
          kommando_buffer = safestrtok(NULL, ";", &last);
        }
      }
      else
        mailbef(s, 1);
      b->job = j_idle;
      b->continous = 0;
      timeout(m.usrtimeout);
      doprompt = (! getvalid() || (u->echo && t->input == io_tnc));
      waitfor(e_reschedule);
    }
    else
    {
      if (conversout(1))
        doprompt = 1;
      else
      {
        wdelay(235);
        doprompt = 0;
      }
    }
    if (b->eingabefehler > 30) break;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef _WXSTN
void wxdaemon (char *name)
//*************************************************************************
//
//*************************************************************************
{
  wx wxstn;
  wxstn.mbwxsave(name);
}
#endif

/*---------------------------------------------------------------------------*/

void emptytempdir (void)
//*************************************************************************
//
// Loescht alle Dateien aus dem TEMP Verzeichnis
//
//*************************************************************************
{
  lastfunc("emptytempdir");
  DIR *d;
  struct dirent *di;
  char filename[80];
  int loeschen_ok;

  loeschen_ok = FALSE;
  mkdir(TEMPPATH); // TEMP ggf. neu anlegen
  d = opendir(TEMPPATH);
  while (d && (di = readdir(d)) != NULL)
  {
#ifdef __LINUX__
    sprintf(filename, "%s/%s", TEMPPATH, di->d_name);
#else
    sprintf(filename, "%s\\%s", TEMPPATH, di->d_name);
#endif
    if (remove(filename) == 0) // dh8ymb: temp-Verzeichnis aufraeumen
      loeschen_ok = TRUE;
  }
  if (d)
    closedir(d);
  if (loeschen_ok)
    trace(replog, "emptytempdir", "Cleaned temp directory");
  mkdir(TEMPPATH); // TEMP sicherheitshalber neu anlegen (db0pra)
}

/*---------------------------------------------------------------------------*/

void mbmainbegin (void)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("mbmainbegin");

  trace(replog, "Start", "V"VNUMMER" "OPSYSTEM" mem %skB hd %ldMB",
                memfree(0), dfree(".", 0) >> 10);
  emptytempdir();
  hadrfile_newformat();
  fork(0, 0, logindaemon, "logind");
  userinit();
  mbinit();
#ifdef _TELEPHONE // JJ
  ttyinit();
#endif
#ifdef _AX25K
  extern int l2vorhanden;
  l2vorhanden |= ax25k_init();
#endif
  bidinit(1);
  initfwdlist();
#ifdef _AUTOFWD
  initafwdlist();
#endif
  fork(0, 0, mbcron, "crond");
  mbreadtree(m.infopath);
  mbcvtload();
#ifdef RUNUTILS
  read_runfile();
#endif
  mk_read_jobs();
#ifdef __FLAT__   //useless in DOS (now)
  init_charsets();
#endif
#ifdef ALPHA
  trace(replog, "mbmainbegin", "Warning: This is an alpha version!");
#endif
  einschaltzeit = ad_time();
#ifdef __FLAT__
  fork(0, 0, logindaemon_sock, "telnetd");
  fork(0, 0, logindaemon_sock, "httpd");
  fork(0, 0, logindaemon_sock, "ftpd");
  fork(0, 0, logindaemon_sock, "sendmail");
  fork(0, 0, logindaemon_sock, "popper");
  fork(0, 0, logindaemon_sock, "nntpd");
  fork(0, 0, logindaemon_sock, "radiod");
  fork(0, 0, logindaemon_sock, "serv");
#endif
#ifdef _WXSTN
  fork(0, 0, wxdaemon, "wxsave");
#endif
#ifdef _MORSED
  fork(0, 0, morsedaemon, "morsed");
#endif
}

/*---------------------------------------------------------------------------*/

void mbmain (char *call, char *uplink, char *mycall)
//*************************************************************************
//
//*************************************************************************
{
  strupr(mycall);
  strcpy(b->mycall, mycall);
  // get the language of the user because it is unknown prior to a
  // correct login
  strcpy(u->sprache, msg_landsuch(call));
  b->msg_loadnum--;
#ifdef _LCF // JJ
  if (testaktiv(call, uplink) && fwdcallok(call, m.logincallformat))
#else
  if (testaktiv(call, uplink) && mbcallok(call))
#endif
  {
    if (   ! *mycall
        || ! strcmp(mycall, m.mycall[0])
        || ! strcmp(mycall, m.mycall[3]))
    {
      mblogin(call, login_standard, uplink);
    }
    else
      if (! strcmp(mycall, m.mycall[1]))
        mblogin(call, login_ufwd, uplink);
      else
        if (! m.sf_only
             && ! strcmp(mycall, m.mycall[2]))
          mblogin(call, login_nofwd, uplink);
        else
        {
          mk_perform(MK_ACCEPT, call);
          return;
        }
    if (*b->logincall)
    {
      if (b->forwarding != fwd_none) fwdmainloop();
      else mbmainloop(1);
      mblogout(0);
    }
  }
}

/*---------------------------------------------------------------------------*/
