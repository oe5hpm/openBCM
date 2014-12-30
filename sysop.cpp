/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------------
  Sysop-Kommandoauswertung und Zubehoer
  -------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980126 OE3DZW lastlog 2k -> 2k5
//19980126 OE3DZW started implementation of ocp omv orm omd
//19980226 hrx    added 'twin' (sysbef, case)
//19980311 OE3DZW added rbin=rprg, wbin=wprg
//19980329 OE3DZW added new pwauswert by DH3MB
//19980404 hrx    support for tty-controls ttyinit,ttycmd,ttydial
//19980408 hrx    trwin added to beftab/befnum/cases
//                case twin changed (no keyboard control)
//19980428 OE3DZW added logincall to shutdown
//19980505 OE3DZW added debug code for sysop-pw-length
//19980714 OE3DZW msg-number is reloaded after setuser
//19980709 DH3MB  added "scmdlist"
//19980914 OE3DZW fixed pwgen, break was missing (scmdlist)
//19990405 DH3MB  moved getpriv, getpw and pwauswert to pw.cpp
//                added commands MD2SUM and MD5SUM
//19990814 DH3MB  Fix: File was not closed after "TAIL -F"
//                Added RDIDADIT and WDIDADIT
//                Added #ifdef FEATURE_YAPP around RYAPP and WYAPP
//199911.. DF3VI  Added FWDedit
//199912.. DF3VI  Added REJedit
//19991223 DF3VI  Added sysop-cmd extract, calling mbchange()
//20000116 DK2UI  (dzw) added ui-options
//20000414 DF3VI  Used Alter Grep for PS, need some changes calling putlog
//20000716 DK2UI  changed shadow variables

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#ifdef _FILEFWD
extern int fwdcmd ();
extern int scan_fwdfile (char type);
extern int scan_emfile (char *path);
#endif

/*---------------------------------------------------------------------------*/

void testdfull (void)
//*************************************************************************
//
//*************************************************************************
{
  if (   (dfree(".", 0) < HOMEDISKFULL)
      || (dfree(m.userpath, 0) < USERDISKFULL)
      || (dfree(m.infopath, 0) < INFODISKFULL))
    putf("***************** HDD is full ******************\n");
}

/*---------------------------------------------------------------------------*/

void sysimport (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("sysimport");

  if (! *befbuf)
    putf("Syntax: IMPORT <filename>\n");
  else
    if (t->input != io_file && t->output != io_file)
    {
      b->inputfile = s_fopen(defext(befbuf, "imp"), "lrb");
      if (b->inputfile)
      {
        b->oldinput = t->input;
        t->input = io_file;
      }
      else
        putf(ms(m_filenotfound), befbuf);
    }
    else
    {
      char *im = stristr(befbuf, ".imp");
      if (im) *im = 0;
      runbatch(befbuf);
    }
}

/*---------------------------------------------------------------------------*/

void putlog (char *name, char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
#ifdef __FLAT__
  html_putf("<b>%s:</b> <a href=cmd?cmd=slog>Syslog</a> | ", ms(m_otherlogs));
  html_putf("<a href=cmd?cmd=erlog>Erase</a> | ");
  html_putf("<a href=cmd?cmd=swaplog>Swap</a> | ");
  html_putf("<a href=cmd?cmd=pwlog>PW</a> | ");
  html_putf("<a href=cmd?cmd=unknown>Unknown</a> | ");
  html_putf("<a href=cmd?cmd=rlog>Reject</a> | ");
  html_putf("<a href=cmd?cmd=sfhold>S&F-Hold</a>");
  if ((m.tcpiptrace == 1) || (m.tcpiptrace == 8))
    html_putf(" | <a href=cmd?cmd=httplog>HTTP</a>");
  if ((m.tcpiptrace == 2) || (m.tcpiptrace == 8))
    html_putf(" | <a href=cmd?cmd=nntplog>NNTP</a>");
  if ((m.tcpiptrace == 3) || (m.tcpiptrace == 8))
    html_putf(" | <a href=cmd?cmd=pop3log>POP3</a>");
  if ((m.tcpiptrace == 4) || (m.tcpiptrace == 8))
    html_putf(" | <a href=cmd?cmd=smtplog>SMTP</a>");
  if ((m.tcpiptrace == 5) || (m.tcpiptrace == 8))
    html_putf(" | <a href=cmd?cmd=ftplog>FTP</a>");
//  html_putf("<form method=get action=\"/cmd\" target=\"txt\">\n");
//  html_putf("Search: <input name=\"search\" value=\"\" size=40>");
//  html_putf("<input type=\"submit\" value=\"Execute\">");
  html_putf("<br><hr>");
#endif
  if (*befbuf)
  { // Optionen durchreichen
    if (b->optminus & o_i) b->optplus |= o_i;
    // wenn nicht explizit "-i-", ist "-i" default
    befbuf += blkill(befbuf);
    grep(name, befbuf, b->optplus);
  }
  else
  {
    fileio_text fio;
    fio.usefile(name);
    fio.settail(-2500);
    fio.tx();
  }
}

/*---------------------------------------------------------------------------*/

int sysbef (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  static char shutdown_reason[23];
  char xbuf[20];
  static char *beftab[] =
    {
      "CONNECT", "LOGIN", "CALL", "OSHELL",
#ifdef __DOS16__
#ifdef _TNC
      "TNC",
#endif
      "W2", "UWIN", "TWIN",
#ifdef _AUTOTRCWIN
      "TRWIN",
#endif
#endif
      "RTEXT", "CAT", "RPRG", "WTEXT", "WPRG", "RBIN", "WBIN",
      "SLR", "CLOG", "REORG",
      "LIFETIME", "MKBOARD", "RMBOARD", "MVBOARD", "SHUTDOWN", "NEW",
      "KILL", "TEST", "IMPORT", "DISABLE", "ENABLE",
#ifdef DIEBOX_UIMPORT
      "UIMPORT",
#endif
#ifdef OLDMAILIMPORT
      "OLDMAILIMPORT",
#endif
      "SETUSER", "BEACON", "GREP", "TGREP", "TAIL", "BEGIN", "BATCH",
      "EXPORT", "PWGEN", "POSTFWD", "MONITOR", "NOTE", "MACRO", "CFGFLEX",
      "ORM", "OMV", "OCP", "OMD", "APPEND",
#ifdef DF3VI_EXTRACT
      "EXTRACT",
#endif
      "HOLD", "SETPW",
#ifdef FEATURE_YAPP
      "WYAPP", "RYAPP",
#endif
#ifdef FEATURE_DIDADIT
      "WDIDADIT", "RDIDADIT",
#endif
#if defined FEATURE_SERIAL || defined _TELEPHONE
      "TTYINIT",
#endif
#ifdef _TELEPHONE // JJ
      "TTYCMD", "TTYDIAL", "TTYHANGUP", "TTYSTATUS", "TTYWIN",
      "TTYCOUNTERRESET",
#endif
#ifdef _AUTOFWD
      "AFWDLIST",
#endif
#ifdef FEATURE_MDPW
      "MD2SUM", "MD5SUM",
#endif
#ifdef FEATURE_EDITOR
      "EDIT", "FEDIT", "REDIT",
#else
 #ifdef DF3VI_FWD_EDIT
      "FEDIT", "FWDEDIT",
 #endif
 #ifdef DF3VI_REJ_EDIT
      "REDIT", "REJECTEDIT",
 #endif
 #ifdef DF3VI_CONV_EDIT
      "CEDIT", "CONVEDIT",
 #endif
#endif
#ifdef _FILEFWD
      "FWDIMPORT", "FWDEXPORT",
#endif
      "YMBTEST",
      "SCMDLIST", // Dies ist immer das letzte Kommando!
      NULL
    };

  enum befnum
    { unsinn, connect_, login, call_, oshell_,
#ifdef __DOS16__
#ifdef _TNC
      tnc,
#endif
      w2, uwin, twin,
#ifdef _AUTOTRCWIN
      trwin,
#endif
#endif
      rtext, rtext_, rprg, wtext, wprg, rbin, wbin,
      slr, clog, reorg,
      life_, mkb, rmb, mvb, shutdown_, new_,
      kill_, test, import, disable_, enable_,
#ifdef DIEBOX_UIMPORT
      uimport_,
#endif
#ifdef OLDMAILIMPORT
      oldmailimport_,
#endif
      setuser, beacon, grep_, tgrep, tail, begin, batch,
      export_, pwgen, postfwd_, monitor_, note, macro, cfgflex_,
      orm, omv, ocp, omd, _append,
#ifdef DF3VI_EXTRACT
      extract_,
#endif
      hold, setpw,
#ifdef FEATURE_YAPP
      wyapp, ryapp,
#endif
#ifdef FEATURE_DIDADIT
      wdidadit, rdidadit,
#endif
#if defined FEATURE_SERIAL || defined _TELEPHONE
      ttyinit,
#endif
#ifdef _TELEPHONE // JJ
      ttycmd, ttydial, ttyhangup, ttystatus, ttywin_,
      ttycounterreset,
#endif
#ifdef _AUTOFWD
      afwdlist_,
#endif
#ifdef FEATURE_MDPW
      md2sum, md5sum,
#endif
#ifdef FEATURE_EDITOR
      edit, fedit, redit,
#else
 #ifdef DF3VI_FWD_EDIT
      fedit, fwdedit,
 #endif
 #ifdef DF3VI_REJ_EDIT
      redit, rejectedit,
 #endif
 #ifdef DF3VI_CONV_EDIT
      cedit, convedit_,
 #endif
#endif
#ifdef _FILEFWD
      fwdimport, fwdexport,
#endif
      ymbtest,
      scmdlist // Dies ist immer das letzte Kommando!
    } cmd = unsinn;
  befbuf += blkill(befbuf);
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  switch (cmd)
  {
#ifdef FEATURE_YAPP
    case ryapp:
#endif
#ifdef FEATURE_DIDADIT
    case rdidadit:
#endif
#if defined(FEATURE_YAPP) || defined(FEATURE_DIDADIT)
      break;
#endif
    default:
      if (u->lf != 6)
        putv(LF);
      leerzeile(); // Sends the number of CRs stored in "ALTER LF"
  }

  switch (cmd)
  {
    case unsinn:
      if (   mbinitbef(befbuf, 0)
      #ifdef _TELEPHONE // JJ
          || ttyinitbef(befbuf, 0)
      #endif
         )
      {
        strcpy(xbuf, "^");
        mbparsave();
#ifdef _TELEPHONE // JJ
        ttyparsave();
#endif
        strncpy(xbuf + 1, befbuf, 18);
        xbuf[19] = 0;
        subst1(xbuf, ' ', 0);
        subst1(xbuf, '=', 0);
        grep(MBINITNAME, xbuf, o_i);
#ifdef _TELEPHONE // JJ
        grep(INITTTYNAME, xbuf, o_i);
#endif
        return OK;
      }
      else return NO;
    case note: trace(replog, "note", "%s", befbuf); break;
#ifdef DF3VI_EXTRACT
    case extract_: mbchange(befbuf, w_extract, 1); break;
#endif
    case hold: mbchange(befbuf, w_hold, 1); break;
    case omv:
    {
      char f1[50], f2[50];
      befbuf = nexttoken(befbuf, f1, 49);
      befbuf = nexttoken(befbuf, f2, 49);
      if (! *f1 || ! *f2 || xrename(f1, f2))
        putf(ms(m_error));
      else
      {
        putf(ms(m_moving));
        putf(ms(m_nach), f1, f2);
      }
      break;
    }
    case ocp:
    {
      char f1[50], f2[50];
      befbuf = nexttoken(befbuf, f1, 49);
      befbuf = nexttoken(befbuf, f2, 49);
      if (! *f1 || ! *f2 || filecopy(f1, f2))
        putf(ms(m_error));
      else
      {
        putf(ms(m_copying));
        putf(ms(m_nach), f1, f2);
      }
      break;
    }
    case orm:
    {
      char f1[50];
      befbuf = nexttoken(befbuf, f1, 49);
      if (! *f1 || xunlink(f1))
        putf(ms(m_filecantdeleted), f1);
      else
        putf(ms(m_filedeleted), f1);
      break;
    }
    case omd:
    {
      char f1[50];
      befbuf = nexttoken(befbuf, f1, 49);
      killbackslash(f1);
      if (! *f1 || xmkdir(f1))
        putf(ms(m_directorycantcreated), f1);
      else
        putf(ms(m_directorycreated), f1);
      break;
    }
    case _append:
    {
      char textline[LINELEN+1];
      befbuf = nexttoken(befbuf, textline, LINELEN);
      if (! *befbuf)
      {
        putf("Syntax: APPEND <textline> <filename>\n");
        break;
      }
      FILE *f = s_fopen(befbuf, "sat");
      if (f)
      {
        fprintf(f, "%s\n", textline);
        s_fclose(f);
      }
      else putf(ms(m_filenotfound), befbuf);
    } break;
    case macro:
    {
      mk_start(befbuf);
    } break;
    case setuser:
    {
      char call[CALLEN+1];
      befbuf = nexttoken(befbuf, call, CALLEN+1);
      strupr(call);
      if (mbcallok(call) && *befbuf)
      {
        trace(report, "setuser", "%s", befbuf);
        {
          if (! mbalter(NULL, befbuf, call))
            putf(ms(m_isunknown), call);
          b->msg_loadnum = 0; //reload msg
          loaduser(b->logincall, u, 1);
        }
      }
      else
        putf("Syntax: SETUSER <call> <option> <value>\n");
    } break;
    case setpw: //automatisierte Passwort-Vergabe
    {
      char call[CALLEN+1];
      char passwd[40];
      int i, pwline = 0;
      FILE *upwf;
      befbuf = nexttoken(befbuf, call, CALLEN+1);
      strupr(call);
      pwline = atoi(befbuf);
      if (! mbcallok(call) || ! pwline)
      {
        putf("Syntax: SETPW <call> <number>\n");
        break;
      }
      if (! loaduser(call, u, 0))
      {
        putf(ms(m_isunknown), call);
        break;
      }
      upwf = s_fopen("userpw.bcm", "sr");
      if (! upwf)
      {
        putf(ms(m_filenotfound), "userpw.bcm");
        break;
      }
      for (i = 0; i < pwline && ! feof(upwf); i++)
        fgets(passwd, sizeof(passwd) - 1, upwf);
      s_fclose(upwf);
      if (i < pwline)
      {
        putf(ms(m_userpwlines), i - 1);
        break;
      }
      strcpy(u->password, passwd);
      saveuser(u);
      pwline = strlen(u->password);
      putf(ms(m_loginpw), pwtypestr(u->loginpwtype), pwtypestr(u->sfpwtype));
      putf(" ");
      putf(ms(m_pwdlength), pwline);
      sprintf(passwd, "pw set to %i char", pwline);
      pwlog(call, b->logincall, passwd);
      b->msg_loadnum = 0; //reload msg
      loaduser(b->logincall, u, 1);
    } break;
#ifdef DIEBOX_UIMPORT
    case uimport_:
    {
      putf(ms(m_dieboximport));
      uimport();
    } break;
#endif
#ifdef OLDMAILIMPORT
    case oldmailimport_:
    {
      scanoptions(befbuf);
      formoptions();
      befbuf += blkill(befbuf);
      if (isdir(befbuf))
      {
        putf(ms(m_omi_started), befbuf);
        putflush();
        oldmailimport(befbuf);
      }
      else
        putf(ms(m_dirunknown), befbuf);
    } break;
#endif
    case disable_:
    {
      putf(ms(m_boxdisabled));
      m.disable = 1;
      mbparsave();
    } break;
    case enable_:
    {
      putf(ms(m_boxenabled));
      m.disable = 0;
      mbparsave();
    } break;
    case test:
    {
      if (*befbuf == 'W') { while (1); } // for testing watchdog
      else if (*befbuf == 'S') { trace(fatal, "test", "abort"); }
      else if (*befbuf == 'V') { *(char *)0 = 1; }
      else mk_start(befbuf);
    } break;
    case batch:
    {
      runbatch(befbuf);
    } break;
    case rtext:
    case rtext_:
    {
      fileio_text fio;
      fio.usefile(befbuf);
      fio.tx();
      putf("\032\n"); // CTRL-Z
    } break;
    case wtext:
    {
      fileio_text fio;
      fio.usefile(befbuf);
      fio.rx();
    } break;
    case rbin:
    case rprg:
    {
      fileio_abin fio;
      fio.usefile(befbuf);
      fio.tx();
    } break;
    case wbin:
    case wprg:
    {
      fileio_abin fio;
      fio.usefile(befbuf);
      fio.rx();
    } break;
#ifdef FEATURE_YAPP
    case ryapp:
    {
      fileio_yapp fio;
      fio.usefile(befbuf);
      fio.tx();
    } break;
    case wyapp:
    {
      fileio_yapp fio;
      fio.usefile(befbuf);
      fio.rx();
    } break;
#endif // FEATURE_YAPP
#ifdef FEATURE_DIDADIT
    case rdidadit:
    {
      fileio_dida fio;
      fio.usefile(befbuf);
      fio.tx();
    } break;
    case wdidadit:
    {
      fileio_dida fio;
      if (! *befbuf) fio.usefile("dummy");
      else
      {
        if (befbuf[strlen(befbuf) - 1] != '/') strcat(befbuf, "/");
        strcat(befbuf, "dummy");
        fio.usefile(befbuf);
      }
      fio.rx();
    } break;
#endif // FEATURE_DIDADIT
    case slr:
    {
      scanoptions(befbuf);
      putlog(TRACEPATH "/" SYSLOGRNAME, befbuf);
    } break;
    case clog:
    {
      scanoptions(befbuf);
      putlog(TRACEPATH "/" CMDLOGNAME, befbuf);
    } break;
    case tail:
    {
      scanoptions(befbuf);
      befbuf += blkill(befbuf);
      if (b->optplus&o_f && *befbuf)
      {
        int a;
        FILE *f = s_fopen(befbuf, "lrt");
        if (f)
        {
          fseek(f, 0, SEEK_END);
          do
          {
            while ((a = fgetc(f)) != EOF) putv(a);
            wdelay(349);
          }
          while (! testabbruch());
          s_fclose(f);
        }
      }
      else
      {
        fileio_text fio;
        fio.usefile(befbuf);
        fio.settail(-2000);
        fio.tx();
      }
    } break;
    case begin:
    {
      fileio_text fio;
      fio.usefile(befbuf);
      fio.settail(2000);
      fio.tx();
    } break;
    case monitor_:
    {
      if (*befbuf)
      {
        scanoptions(befbuf);
        b->continous = 1;
        monitor(atoi(befbuf), b->optplus);
      }
      else
        putf("Syntax: MONITOR [-iords] <task-id>\n");
    } break;
    case tgrep:
    {
      char string[61];
      char name[40];
      scanoptions(befbuf);
      if (b->optminus & o_i) b->optplus |= o_i;
      //wenn nicht explizit "-i-", ist "-i" default
      b->usermail = 0;
      if (! *befbuf)
      {
        putf("Syntax: TGREP [pattern] <fwd-bbs>\n");
        break;
      }
      befbuf = nexttoken(befbuf, string, 60);
      if (*befbuf)
      {
        sprintf(name, TRACEPATH "/t_%s.bcm", befbuf);
        grep(name, string, b->optplus);
      }
      else
      {
        sprintf(name, TRACEPATH "/t_%s.bcm", string);
        putlog(name, "");
      }
    } break;
    case grep_:
    {
      char string[61];
      scanoptions(befbuf);
      if (b->optminus & o_i) b->optplus |= o_i;
      //wenn nicht explizit "-i-", ist "-i" default
      b->usermail = 0;
      befbuf = nexttoken(befbuf, string, 60);
      grep(befbuf, string, b->optplus);
    } break;
#ifdef _AUTOFWD
    case afwdlist_:
    {
      afwdlist(befbuf);
    } break;
#endif
    case life_:
    {
      if (*befbuf)
      {
        strupr(befbuf);
        char *life = skip(befbuf);
        int board = finddir(befbuf, b->sysop);
        if (board > 0)
        {
          board -= 1;
          if (life && *life)
          {
            tree[board].lifetime_max = atoi(life);
            tree[board].lifetime_min = 1;
            if (tree[board].lifetime_max > 999)
              tree[board].lifetime_max = 999;
            if (tree[board].lifetime_max < 1)
              tree[board].lifetime_max = 1;
            char *life_min = skip(life);
            if (life_min && *life_min)
            {
              tree[board].lifetime_min = atoi(life_min);
              if (tree[board].lifetime_min > 999)
                tree[board].lifetime_min = 999;
              if (tree[board].lifetime_min < 1)
                tree[board].lifetime_min = 1;
            }
            mbtreesave();
          }
          putf(ms(m_lifetimestat), b->boardfullname, tree[board].lifetime_max,
                                                     tree[board].lifetime_min);
        }
        else
          putf(ms(m_notfound), befbuf);
      }
      else
        putf("Syntax: LIFETIME <board> <days_maximum> [<days_minimum>]\n"
             "            (with 1 <= days_maximum/days_minimum <= 999)\n\n");
    } break;
#ifdef _TNC
    case tnc:
    {
      control_tnc(befbuf);
    } break;
#endif
    case connect_:
    {
      termqso(befbuf);
    } break;
    case login:
    {
      if (mbcallok(befbuf))
      {
        mblogin(befbuf, login_standard, b->uplink);
        cmdlog("login changed");
        putf(ms(m_loginchanged));
      }
      else
        putf("Syntax: LOGIN <new logincall>\n");
    } break;
    case call_:
    {
      if (mbcallok(befbuf))
      {
        mblogin(befbuf, login_silent, b->uplink);
        b->msg_loadnum--; //Sonst wird falsche Sprache benutzt
        cmdlog("call changed");
      }
      else
        putf("Syntax: CALL <new logincall>\n");
    } break;
    case oshell_:
    {
      if (t->input == io_file || t->output == io_file)
        oshell(befbuf, sh_noinput);
      else oshell(befbuf, m.dosinput ? sh_forceinput : sh_ifmultitask);
    } break;
    case reorg:
    {
      if (sema_test("purgereorg") == 1)
        putf(ms(m_purgeallstarted));
      else
      {
        putf(ms(m_reorginvoked));
        fork(P_BACK | P_MAIL, 0, mbreorg, befbuf);
      }
    } break;
    case postfwd_:
    {
      putf(ms(m_postfwdinvoked));
      fork(P_BACK | P_MAIL, 0, postfwd, "Postfwd");
    } break;
#ifdef FEATURE_EDITOR
    case edit:
    {
      fork(P_WIND | P_KEYB, 0, editfile, befbuf);
    } break;
    case fedit:
    {
      fork(P_WIND | P_KEYB, 0, editfile, FWDLISTNAME);
    } break;
    case redit:
    {
      fork(P_WIND | P_KEYB, 0, editfile, REJECTNAME);
    } break;
#else
 #ifdef DF3VI_FWD_EDIT
    case fedit: //wenn kein editor vorhanden remote-editor aufrufen
    case fwdedit:
    {
      fwdlistedit(befbuf);
    } break;
 #endif
 #ifdef DF3VI_REJ_EDIT
    case redit:
    case rejectedit:
    {
      rejectlistedit(befbuf);
    } break;
 #endif
 #ifdef DF3VI_CONV_EDIT
    case cedit:
    case convedit_:
    {
      convedit(befbuf);
    } break;
 #endif
#endif
    case new_:
    {
      scanoptions(befbuf);
      mbinit();
      initfwdlist();
#ifdef _AUTOFWD
      initafwdlist();
#endif
      if (! (b->optplus & o_q)) // skip statistics on "new -q"
      {
        b->optplus = o_s | o_f | o_c;
        putf(ms(m_hadrstat));
        browse_hadr("");
      }
      mbcvtload();
#ifdef RUNUTILS
      read_runfile();
#endif
      msg_dealloc(1);
      mk_read_jobs();
    } break;
    case kill_:
    {
      if (atoi(befbuf))
      {
        while (atoi(befbuf))
        {
          if (! killtask(nextdez(&befbuf), 1))
            putf(ms(m_cantkilltask));
        }
      }
      else
        putf("Syntax: KILL <task-id>\n");
    } break;
#ifdef __DOS16__
    case w2:
    {
      fork(P_WIND | P_KEYB|P_MAIL, 0, mbwin2, m.sysopcall);
    } break;
    case uwin:
    {
      fork(P_WIND | P_KEYB|P_MAIL, 0, userwin, "Users");
    } break;
    case twin:
    {
      fork(P_WIND | P_MAIL, 0, taskwin, befbuf);
    } break;
#ifdef _AUTOTRCWIN
    case trwin:
    {
      if (*befbuf) fork(P_WIND | P_MAIL, 0, trcwin, befbuf);
      else putf("Syntax: TRWIN [-iords] <task-id>\n");
    } break;
#endif
#endif //__DOS16__
    case mkb:
    {
      char mainboard[30];
      char newboard[20];
      char *slash;
      befbuf = nexttoken(befbuf, mainboard, 29);
      befbuf = nexttoken(befbuf, newboard, 19);
      slash = strchr(mainboard + 1, '/');
      if (slash && (! *newboard))
      {
        *slash = 0;
        strcpy(newboard, slash + 1);
      }
      if (! *newboard && *mainboard == '/')
      {
        strcpy(newboard, mainboard + 1);
        mainboard[1] = 0;
      }
      if (*mainboard && *newboard)
      {
        switch (mkboard(mainboard, newboard, 0))
        {
          case 0: putf(ms(m_boardcreated)); break;
          case 1: putf(ms(m_mainboardnotfound)); break;
          case 2: putf(ms(m_toomanyboards)); break;
          case 3: putf(ms(m_boardnameexist)); break;
          case 4: putf(ms(m_invalidboardname)); break;
        }
      }
      else
        putf("Syntax: MKBOARD <mainboard> <subboard>\n");
    } break;
    case rmb:
    {
      if (*befbuf)
      {
        subst1(befbuf, ' ', '/');
        if (*befbuf == '/' && befbuf[1] == '/') befbuf++;
        switch (rmboard(befbuf))
        {
          case 0: putf(ms(m_boardremoved)); break;
          case 1: putf(ms(m_boardnotfound)); break;
          case 2: putf(ms(m_boardnotempty)); break;
        }
      }
      else
        putf("Syntax: RMBOARD <mainboard> <subboard>\n");
    } break;
    case mvb:
    {
      char oldboard[20];
      char subboard[20];
      char neuboard[20];
      befbuf = nexttoken(befbuf, oldboard, 19);
      befbuf = nexttoken(befbuf, subboard, 19);
      befbuf = nexttoken(befbuf, neuboard, 19);
      if (*oldboard && *subboard && *neuboard)
      {
        switch (mvboard(oldboard, subboard, neuboard))
        {
          case 0: putf(ms(m_boardmoved)); break;
          case 1: putf(ms(m_newboardnotfound)); break;
          case 2: putf(ms(m_toomanyboards)); break;
          case 4: putf(ms(m_oldboardnotfound)); break;
        }
      }
      else
        putf("Syntax: MVBOARD <oldboard> <subboard> <newboard>\n");
    } break;
    case shutdown_:
    {
      scanoptions(befbuf);
#ifdef __DOS16__
      if (b->optplus & o_r) atexit((void(*)()) MK_FP(0xffff, 0x0000));
#endif
      runterfahren = 1;
      sprintf(shutdown_reason, "shutdown by %s", b->logincall);
      stopreason = shutdown_reason;
    } break;
    case cfgflex_:
    {
      if (*befbuf && file_isreg("cfgflex.bcm"))
      {
        putf(ms(m_flexstarted));
        fork(P_BACK | P_MAIL, 0, cfgflex, befbuf);
      }
      else
        putf("Syntax: CFGFLEX <flexcall> (cfgflex.bcm must exist)\n");
    } break;
    case import:
    {
      sysimport(befbuf);
    } break;
    case export_:
    {
      if (*befbuf)
      {
        if ((t->input != io_file || t->output == io_dummy)
            && t->output != io_file)
        {
          char fname[51];
          scanoptions(befbuf);
          befbuf = nexttoken(befbuf, fname, 50);
          if (b->optplus & o_a) // neue Option -b fuer binaer
          {
            if (b->optplus & o_b) b->outputfile = s_fopen(fname, "sab");
            else b->outputfile = s_fopen(fname, "sat");
          }
          else
          {
            if (b->optplus & o_b) b->outputfile = s_fopen(fname, "swb");
            else b->outputfile = s_fopen(fname, "swt");
          }
          if (b->outputfile)
          {
            s_fsetopt(b->outputfile, 1);
            b->oldinput = t->input;
            b->oldoutput = t->output;
            t->input = io_dummy;
            t->output = io_file;
            b->continous = 1;
            if (b->optplus & o_u) b->sysop = 0;
            if (*befbuf) mailbef(befbuf, 0);
            b->sysop = 1;
          }
          else
            putf(ms(m_filenotopen), fname);
        }
      }
      else
        putf("Syntax: EXPORT <filename> <box-command>\n");
    } break;
    case beacon:
    {
      if (*befbuf)
      {
        FILE *f = s_fopen(BEACONNAME, "srt");
        unsigned int i = 0;
        char s[LINELEN+1];
        if (f)
        {
          while (fgets(s, LINELEN, f))
          {
            if (*s)
            {
              s[strlen(s) - 1] = 0;
              putbeacon_tnc(s, befbuf);
              i++;
            }
          }
          s_fclose(f);
        }
        putf(ms(m_beaconframes), i);
      }
      else
      {
        fork(P_BACK | P_MAIL, 0, sendmailbake, "Beacon");
        putf(ms(m_beaconstarted));
      }
    } break;
    case pwgen:
    {
      FILE *f;
      if (*befbuf && (f = s_fopen(befbuf, "swt")) != 0)
      {
        unsigned int i;
        int upw;
        upw = ! stricmp(befbuf, "userpw.bcm"); // file fuer setpw
        for (i = 0; i < 1620; i++)
        {
          char c = 0;
          while (! isalnum(c)) c = random_max('z');
          fputc(c, f);
          //pw-file fuer setpw erzeugen (81 Zeilen mit je 20 Zeichen)
          if (upw && (i % 20) == 19) fputc(LF, f);
        }
        trace(report, "pwgen", "%s created", befbuf);
        s_fclose(f);
      }
      else //ohne Parameter immer userpw.bcm erzeugen
      if (! *befbuf && (f = s_fopen("userpw.bcm", "swt")) != 0)
      {
        unsigned int i;
        for (i = 0; i < 1620; i++)
        {
          char c = 0;
          while (! isalnum(c)) c = random_max('z');
          fputc(c, f);
          //pw-file fuer setpw erzeugen (81 Zeilen mit je 20 Zeichen)
          if ((i % 20) == 19) fputc(LF, f);
        }
        trace(report, "pwgen", "userpw.bcm created");
        s_fclose(f);
      }
    } break;
    case scmdlist: // DH3MB
    {
      unsigned int i = 0;
      while (beftab[i]) putf("(%02d) %s\n", ++i, beftab[i]);
    } break;
#ifdef FEATURE_MDPW
    case md2sum:
    {
      if (! *befbuf)
      {
        putf("Syntax: MD2SUM <filename>\n");
        break;
      }
      if (! file_isreg(befbuf))
        putf(ms(m_filenotfound), befbuf);
      else
      {
        MD2 md2;
        md2.readfile(befbuf, 0L);
        md2.gethexdigest(b->line);
        putf("%s  %s\n", b->line, befbuf);
      }
    } break;
    case md5sum:
    {
      if (! *befbuf)
      {
        putf("Syntax: MD5SUM <filename>\n");
        break;
      }
      if (! file_isreg(befbuf))
        putf(ms(m_filenotfound), befbuf);
      else
      {
        MD5 md5;
        md5.readfile(befbuf, 0L);
        md5.gethexdigest(b->line);
        putf("%s  %s\n", b->line, befbuf);
      }
    } break;
#endif
#if defined FEATURE_SERIAL || defined _TELEPHONE
    case ttyinit:
    {
      if (eingelogt("getty", 0, 0))
        putf(ms(m_ttyactive));
      else
        init_tty();
    } break;
#endif
#ifdef _TELEPHONE // JJ
    case ttycmd:
    {
      if (*befbuf)
      {
        if (m.ttydevice > 1)
          putf_tty("%s\r", befbuf);
        else
          putf(ms(m_nomodem));
      }
      else
        putf("Syntax: TTYCMD <command>\n");
    } break;
    case ttydial:
    {
      strupr(befbuf);
      char *nummer;
      char call[8];
      nummer = nexttoken(befbuf, call, 8);
      if (*befbuf && mbcallok(call))
      {
        if (m.ttydevice
            && (get_ufwd(call)[0] || isforwardpartner(call) >= 0))
        {
          putf(ms(m_startphonefwd), call, nummer);
          sprintf(befbuf, "%s TTY %s", call, nummer);
          fork(P_BACK | P_MAIL, 0, fwdsend, befbuf);
        }
        else
          putf(ms(m_nottyactive));
      }
      else
        putf("Syntax: TTYDIAL <call> <number>\n");
    } break;
    case ttyhangup:
    {
      tty_hangup();
      putf(ms(m_hangupmodem));
    } break;
    case ttystatus:
    {
      tty_statustext();
      putv(LF);
    } break;
    case ttywin_:
    {
      fork(P_WIND | P_MAIL, 0, tty_win, befbuf);
    } break;
    case ttycounterreset:
    {
      tty_counterreset();
      putv(LF);
    } break;
#endif
#ifdef _FILEFWD
    case fwdimport:
    {
      if (*befbuf) fwd_import(befbuf);
    } break;
    case fwdexport:
    {
      if (*befbuf) fwd_export(befbuf);
    } break;
#endif
    case ymbtest:
    {
#ifdef _USERCOMP
      if (u->comp == 1)
      {
/*
    char output[256] = { 0 };
    char output2[256] = { 0 };
    int i, il = 0;
*/
        putf("//COMP 1\n\n");
        putflush();

/*
    il = comp_sp_stat_huff(befbuf, strlen(befbuf), output);
//    printf("il: %d strlen: %d\n",il,strlen(befbuf));
//    printf("befbuf:\n-%s-\nOut:\n-%s-\n",befbuf,output);
    //putflush();
    for (i = 1; i < il ; i++)
      bputv(output[i]);

    putv(LF);
    putflush();
    output[0] = '\0';
    strcpy(befbuf, "dies ist noch ein laengerer text 2");
    il = comp_sp_stat_huff(befbuf, strlen(befbuf), output);
    for (i = 1; i < il ; i++)
      bputv(output[i]);

    putv(LF);
    putflush();
    output[0] = '\0';
    strcpy(befbuf, "dies ist ein noch laengerer text 3");
    il = comp_sp_stat_huff(befbuf, strlen(befbuf), output);
    for (i = 1; i < il ; i++)
      bputv(output[i]);


    putv(LF);
    putflush();
    putf("\n");
    il = decomp_sp_stat_huff(output, strlen(output), output2);
    printf("il: %d strlen: %d\n",il,strlen(output));
    printf("Out2:\n-%s-\n",output2);
*/

/*
#include "ahuf.h"
   // TOP-Huffman
class AHUF;
   AHUF *ahuf;
   ahuf = new AHUF();

    il = ahuf->Komprimieren(true, befbuf, output, strlen(befbuf) );
    printf("il: %d strlen: %d\n",il,strlen(befbuf));
    printf("befbuf:\n-%s-\nOut:\n-%s-\n",befbuf,output);
    putflush();
    putf("%s",output);
    putflush();
    putf("\n");
*/
      }
#endif
    } break;
  }
  return OK;
}

/*--------------------------------------------------------------------------*/
