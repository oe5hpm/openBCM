/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------
  Einstellung der persoenlichen Benutzerparameter
  -----------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//19980115 OE3DZW utcoffset -> ad_timezone
//19980118 OE3DZW sec in timestr
//19980215 OE3DZW text for userfwd with no path "passive" instead of
//                "disabled"
//19980324 DH3MB  support for md-pw
//19980329 OE3DZW changes to md-pw
//19980330 OE3DZW pwtype is reset to BayCom when pw is erased/set to off
//19980404 hrx    support for guests
//19980408 OE3DZW "notvisible" was too long -> DO NOT TOUCH!!
//19980423 OE3DZW sfpwtype instead of fwdpwtype
//                added newcall, get_newcall
//19980426 OE3DZW ttypw may be 8 chars long (was 7 only)
//19980427 OE3DZW pwline only when pw set, info when pw is set
//19980430 OE3DZW fixed readlock, could only be set to 0 or 1
//19980505 OE3DZW alter sfpwtype/loginpwtype will only show valid types
//19980506 OE3DZW fixes to fwdhold
//19980615 OE3DZW commented out newcall
//19980614 hrx    added paclen to display_parameter, mbalter befnum/beftab
//                and cases
//19980618 OE3DZW temp variable for fdelay short unsigned (warn. dos)
//19980619 OE3DZW typecast for short unsigned, removed check for < 0
//19980830 OE3DZW alter default only when password setting has not been
//                switched off by user (alter pw off)
//19980830 OE3DZW name is only put to upper or lower case when all
//                digits are in upper or lower case and there is no
//                non-character in between.
//                lastcmd - at top of pw-function
//19980914 OE3DZW guest excluded
//19980925 OE3DZW will not allow "." in userfwd-path
//19981015 OE3DZW md-pw-string was unterminated, fixed
//19981015 OE3DZW userpw off -> "disabled"
//19981015 OE3DZW ufwd can be set to "off" or "passive"
//19981016 OE3DZW changed text for ufwd (removed dot at end)
//19990220 DK2UI  elseif instead of if (alter pw)
//19990227 DK2UI  alter pw, fix
//19990227 OE3ZDW removed multiple mybbs-infos, fixed ufwd=0
//19990329 OE3DZW if userpw contains "DUMMY" http/pop3-logins are allowed
//19990412 OE3DZW mybbs update only if setting different/>1week
//19990812 DH3MB  Removed trailing blank from "Password removed."-pwlog-entry
//19990823 DH3MB  Added option "BINMODE"
//20000102 OE3DZW commented out BINMODE, not finished
//19991225 DF3VI  blocked alter-options that cannot be J_TELL-ed
//20000414 DF3VI  changed alter grep to alter ps
//20021210 hpk    the user can set his paclen only within from the sysop given
//                range (minpaclen, maxpaclen)
//20021213 hpk    users settings only will be saved if his logintype is full
//                (b->logintype==1) [CB-BCMNET login-concept]
//20021210 hpk    is the user logged on as guest, settings will not be saved
//20030101 hpk    if user queries information about own call he gets all infos
//                in the CB-BCMNET Login-concept, user only gets full
//                information about his call if he is not a guest; if user
//                queries information about another call he gets limited infos
//20030104 hpk    only if m.maxpaclen is 256, paclen can be disabled.
//20030105 hpk    change in CB-BCMNET Login-concept:
//                user-settings now will be saved, but some 'dangerous'
//                commands like 'alter forward', etc. will not work.
//20030727 DB1RAS changes in paclen

#include "baycom.h"

static user_t us;    // do not waste stack. The following functions
                     // no not need to be re-entrant

/*---------------------------------------------------------------------------*/

char *get_ttypw (char *call, char *pw)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1) && ! (us.status & 2))
    strcpy(pw, us.ttypw);
  else
    *pw = 0;
  return pw;
}

/*---------------------------------------------------------------------------*/
#ifdef __FLAT__

int put_ttypw (char *call, char *pw)
//****************************************************************************
//
// wird fuer httpaccount benoetigt
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, u, 1))
  {
    strcpy(u->ttypw, pw);
    saveuser(u);
    return 1;
  }
  else
    return 0;
}
/*---------------------------------------------------------------------------*/

char *get_pw (char *call, char *pw)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1)
      && *us.name && ! (us.status & 2))
    strcpy(pw, us.password);
  else
    *pw = 0;
  return pw;
}
/*---------------------------------------------------------------------------*/

char *get_httppw (char *call, char *pw)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("get_httppw");
  strupr(call);
  if (mbcallok(call)
      && loaduser(call, &us, 1) && *us.name && ! (us.status & 2)
      && (! *us.password || (*us.password == 1 && ! us.password[1])
          || strstr(us.password, "DUMMY")))
    //do not allow login when password is set!
    //but accept login when password contains "DUMMY"!
    strcpy(pw, us.name);
  else
    *pw = 0;
  return pw;
}

/*---------------------------------------------------------------------------*/

int get_smtp_security (char *call)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("get_smtp_security");
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return (us.unsecure_smtp & 1);
  else return 0; // 0 => only secure smtp
}
#endif
/*---------------------------------------------------------------------------*/

int get_nopurge (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return us.nopurge;
  else return 0;
}

/*---------------------------------------------------------------------------*/

int get_fdelay (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return (b->forwarding == fwd_none) * us.fdelay
            + (us.fhold * 60) * (! b->sysop) * (! b->usermail);
  else return 0;
}

/*---------------------------------------------------------------------------*/

char *get_ufwd (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return us.ufwd;
  else return "";
}

/*---------------------------------------------------------------------------*/

time_t get_mybbstime (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return us.mybbstime;
  else return 0;
}

/*---------------------------------------------------------------------------*/

int get_readlock (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
    return us.readlock;
  else return 0;
}

/*---------------------------------------------------------------------------*/

void inc_mailgot (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
  {
    us.mailgot++;
    saveuser(&us);
  }
}

/*---------------------------------------------------------------------------*/

void inc_mailsent (char *call)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1))
  {
    us.mailsent++;
    saveuser(&us);
  }
}

/*---------------------------------------------------------------------------*/

int get_mybbs (char *call, char *bbs, int setmybbs)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (*call && mbcallok(call) && loaduser(call, &us, 1))
  {
    if (! bbs) return us.mybbsok; //null-pointer is given
    if (! *bbs) strcpy(bbs, us.mybbs);
    else if (setmybbs && (! *us.mybbs || (! us.mybbsok && setmybbs == 2)))
    {
      if (strlen(bbs) <= MYBBSLEN)
        strcpy(us.mybbs, bbs);
      else
        strcpy(us.mybbs, atcall(bbs));
      us.mybbsok = 0;
      us.mybbstime = ad_time();
      saveuser(&us);
    }
    return us.mybbsok;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void set_name (char *call, char *name)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (*name && mbcallok(call) && loaduser(call, &us, 1))
  {
    if (! *us.name || ! us.nameok)
    {
      safe_strcpy(us.name, name);
      saveuser(&us);
    }
  }
}

/*---------------------------------------------------------------------------*/

void set_qth (char *call, char *qth)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (*qth && mbcallok(call) && loaduser(call, &us, 1))
  {
    if (! *us.qth || ! us.qthok)
    {
      safe_strcpy(us.qth, qth);
      saveuser(&us);
    }
  }
}

/*---------------------------------------------------------------------------*/

void set_zip (char *call, char *zip)
//****************************************************************************
//
//****************************************************************************
{
  strupr(call);
  if (*zip && mbcallok(call) && loaduser(call, &us, 1))
  {
    if (! *us.zip || ! us.zipok)
    {
      safe_strcpy(us.zip, zip);
      saveuser(&us);
    }
  }
}

/*---------------------------------------------------------------------------*/

char *get_name (char *call, int par)
//****************************************************************************
//
//****************************************************************************
{
  static char name[NAMELEN+5];
  *name = 0;
  strupr(call);
  if (mbcallok(call) && loaduser(call, &us, 1) && *us.name)
  {
    if (par) sprintf(name, "(%s) ", us.name);
    else sprintf(name, "%s ", us.name);
  }
  return name;
}

/*---------------------------------------------------------------------------*/

int get_usr_local (char *call, time_t &lastlogin)
//****************************************************************************
//
// returns 1 if a user is known and local, 0 else
// lastlogin: last login of user when found (else 0)
//
//****************************************************************************
{
  strupr(call);
  lastlogin = 0;
  if (mbcallok(call) && loaduser(call, &us, 1) && us.mybbsok)
  {
    lastlogin = us.lastboxlogin;
    if (! strcmp(m.boxname, atcall(us.mybbs))) return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int get_newcall (char *call, char *newcall)
//****************************************************************************
//
//
//****************************************************************************
{
  char name[20];
  strcpy(name, "get_newcall");
  unsigned int i = 0;
  if (strlen(call) > CALLEN) return 0;
  strupr(call);
  strcpy(newcall, call);
  while (i < 20
         && mbcallok(newcall)
         && loaduser(newcall,&us,1)
         && *us.newcall)
  {
    i++;
    us.newcall[CALLEN] = 0;
    strcpy(newcall, us.newcall);
    if (! strcmp(newcall, call))
    {
      trace(serious, name, "loop %s", call);
      return 0;
    }
  }
  if (i == 20 || i == 0)
  {
    strcpy(newcall, call);
    return 0;
  }
  else
  {
    //trace(report, name, "upd %s -> %s", call, newcall);
    return 1;
  }
}

/*---------------------------------------------------------------------------*/

void set_mybbs (char *call, char *bbs, long btime, char *bid,
                char *origin, short hops, char *w_name)
//****************************************************************************
//
//   btime... 0 on user/sysop-input
//        ... time-stamp of remote-msg
//
//****************************************************************************
{
  char name[20];
  time_t current = ad_time();
  int isnew = 0;

  strcpy(name, "set_mybbs");
  hops++;
  strupr(call);
  if (! mbcallok(call))
  {
    trace(replog, name, "call %s", call);
    return;
  }
  strupr(bbs);
  if (mbhadrok(bbs) != 1)
  {
    trace(report, name, "hadr %s@%s", call, bbs);
    return;
  }
  if (bid)
  {
    strupr(bid);
    if (strlen(bid) > BIDLEN || strlen(bid) < 10)
    {
      trace(serious, name, "BID %s", bid);
      return;
    }
  }
  if (loaduser(call, &us, 1))
  {
    if (us.mybbstime > current) us.mybbstime = current;
    if (btime > current) btime = current;
    if (! btime || btime > us.mybbstime || ! us.mybbsok)
    {
      isnew = 1;
      char bbs_call[CALLEN+1];
      char usbbs_call[CALLEN+1];
      safe_strcpy(bbs_call, atcall(bbs));
      safe_strcpy(usbbs_call, atcall(us.mybbs));

      //only send remote update if local setting is different and
      //older than one week (no re-send within 1 week)
      if (    btime  //is remote msg
           && ! strcmp(bbs_call, usbbs_call)
           && (current - us.mybbstime) < (DAY * 7))
      {
        isnew = 0;
      }
      if (   btime      //is remote msg
          && us.mybbsok //bbs was correct
          && ! strcmp(usbbs_call, m.boxname) //bbs was home-bbs before
          && strcmp(bbs_call, m.boxname))    //is now different from home-bbs
      {
        char sendcmd[LINELEN+1];
        char content[255];
        sprintf(sendcmd, "SP %s < %s Warning: MYBBS set to %s",
                         call, m.boxname, bbs_call);
        sendcmd[LINELEN] = 0;
        sprintf(content, "Your home-bbs was set to %s.\n"
                         "Origin: %s ID: %s Date: %s\n"
                         "\n"
                         "Please check if this is correct!\n",
                         bbs, origin, bid, datestr(btime, 2));
        genmail(sendcmd, content);
        if (strlen(us.password) > 3) // user has pw set
          isnew = 0; //do not save/fwd that information
      }
      if (isnew)
      //set data in user-database
      {
        if (btime) us.mybbstime = btime;
        else us.mybbstime = current;
        if (strlen(bbs) <= MYBBSLEN) strcpy(us.mybbs, bbs);
        else strcpy(us.mybbs, atcall(bbs));
        us.mybbsok = 1;
        if (strcmp(atcall(bbs), m.boxname) && *us.ufwd > 1) *us.ufwd = 0;
        // Active userfwd only if bbs is home-bbs
        // User may set parameter to disabled/passive
        saveuser(&us);
      }
      if (bid && isnew) //remote-update
      {
        wpdata_t *wp = (wpdata_t *) t_malloc(sizeof(wpdata_t), "wpa");
        if (w_name)
          strcpy(wp->name, w_name); //check for name received in WP/WPROT
        else
        {
          if (! strcmp(b->logincall, origin) && *us.name && us.nameok)
          {
            //strncpy(wp->name, us.name, NAMELEN); //use local name
            //wp->name[NAMELEN+1] = 0;
            safe_strcpy(wp->name, us.name); //use local name
          }
          else
            safe_strcpy(wp->name, "?"); //name is unknown
        }
        strcpy(wp->call, call);
        strcpy(wp->bid, bid);
        wp->mybbstime = us.mybbstime;
        strcpy(wp->logincall, b->logincall);
        strcpy(wp->origin, origin);
        wp->hops = hops;
        strcpy(wp->bbs, bbs);
        if (us.zip) strcpy(wp->zip, us.zip);
        else strcpy(wp->zip, "?");
        if (us.qth) strcpy(wp->qth, us.qth);
        else strcpy(wp->qth, "?");
#ifdef _GUEST
        if (strcmp(wp->call, m.guestcall)) // guestcall nicht weiterreichen
#endif
          addwp_m(wp);
        t_free(wp);
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

static int near mybbs (char *adr, int weitergeben, time_t timeu)
//****************************************************************************
//
//****************************************************************************
{
  char *bid;
  char bbs[HADRESSLEN+1];

  bid = NULL;
  safe_strcpy(bbs, adr); //to be safe (db1ras)
  if (weitergeben) subst1(bbs, '-', 0);
  if (! weitergeben || mbhadrok(bbs) == 1)
  {
    if (weitergeben)
    {
      expand_hadr(bbs, m.hadrstore);
      if (! strcmp(u->call, bbs)) return NO; // bbs=call only if hadr given..
      bid = newbid();
    }
    set_mybbs(u->call, bbs, timeu, bid, b->logincall, 0, NULL);
    loaduser(u->call, u, 0);
    putf(ms(m_mybbsset), bbs);
    if (bid) putf(ms(m_mybbsforward));
    showpath(bbs, 0);
    return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

char *optstr (bitfeld option)
//****************************************************************************
//
//****************************************************************************
{
  static char str[33];
  unsigned int i, j = 0;
  for (i = 0; i < 32; i++)
  {
    if (option & 1) str[j++] = i + 'A';
    option >>= 1;
  }
  str[j] = 0;
  return str;
}

/*---------------------------------------------------------------------------*/

void display_parameter (char *call, int full)
//****************************************************************************
//
//****************************************************************************
{
  user_t uu;
  char *z;
//  char *z = ms(m_neverlogin);

  strupr(call);
  if (loaduser(call, &uu, 0))
  {
    if (! *uu.notvisible) uu.notvisible[1] = 0; // Vorsichtsmassnahme
    putf(ms(m_a_headline), uu.call);
    if (*uu.newcall || b->sysop)
      putf("New callsign..(A NE).%s\n", *uu.newcall ? uu.newcall : "none");
    putf(ms(m_a_mybbs), uu.mybbsok ? '@' : '?', uu.mybbs);
    if (uu.mybbstime) putf(" (%s)", datestr(uu.mybbstime, 12));
    putv(LF);
    if (full)
    {
      putf(ms(m_a_reject), uu.notvisible+1);
      putf(ms(m_a_prompt), uu.prompt);
      putf(ms(m_a_firstcmd), uu.firstcmd);
      putf(ms(m_a_name), uu.name);
      putf("QTH...........(A QT).%s\n", uu.qth);
      putf("ZIP-Code.......(A Z).%s\n", uu.zip);
      putf(ms(m_a_speech), uu.sprache);
      putf(ms(m_a_lines), uu.zeilen);
      putf(ms(m_a_helplevel), uu.helplevel);
      if (uu.lf == 6)
        putf("Linefeeds..(A LF).-1\n");
      else
        putf(ms(m_a_linefeed), uu.lf);
      putf(ms(m_a_idir), optstr(uu.opt[o_id]));
      putf(ms(m_a_udir), optstr(uu.opt[o_ud]));
      putf(ms(m_a_ilist), optstr(uu.opt[o_il]));
      putf(ms(m_a_ulist), optstr(uu.opt[o_ul]));
      putf(ms(m_a_iread), optstr(uu.opt[o_ir]));
      putf(ms(m_a_uread), optstr(uu.opt[o_ur]));
      putf(ms(m_a_check), optstr(uu.opt[o_ch]));
      putf("PS-Options.(A PS).%s\n", optstr(uu.opt[o_ps]));
      if (strcmp(b->logincall, call) == 0 || b->sysop)
      {
        putf("FHold.........(A FH).%-20d", uu.fhold);
        putf("FWD-Delay..(A FD).%d min\n", uu.fdelay);
      }
      else
        putf("FWD-Delay.....(A FD).%d min\n", uu.fdelay);
      putf("User-Forward..(A UF).");
      // userforward is either off, passive or passive and active
      if (! *uu.ufwd) putf("passive\n");
      else if (*uu.ufwd == 1) putf("off\n");
      else putf("%s\n", uu.ufwd);
      if (uu.dirformat) putf("Dirformat.....(A DI).%d\n", uu.dirformat);
#ifdef __FLAT__
      if (uu.unsecure_smtp)
        putf("Unsecure SMTP.(A UN).%d\n", (uu.unsecure_smtp & 1));
#endif
      if ((m.paclen || b->sysop) && uu.paclen)
        putf("Paclen........(A PA).%i\n", uu.paclen);
#ifdef _USERCOMP
      if (uu.comp == 1)
        putf("Comp........(A COMP).active\n");
      else
        putf("Comp........(A COMP).not active\n");
#endif
      if (strcmp(b->logincall, call) == 0 || b->sysop)
      {
        putf("Status........(A ST).%-20d", uu.status);
        putf("No-Purge...(A NO).%d\n", uu.nopurge);
        putf("Passwordlen...(A PW).%-20d",
             *uu.password == 1 ? -1 : (int) strlen(uu.password));
        putf("TTYPWlen..(A TTY).%d\n", strlen(uu.ttypw));
#ifdef __LINUX__
        if (*uu.linuxpw && m.addlinuxsystemuser)
          putf("Linux-PWlen.(A LINU).%d\n", strlen(uu.linuxpw));
#endif
        if (*uu.password > 1)
        {
#ifdef FEATURE_MDPW
          putf("Login-pwtype..(A LO).%-20s", pwtypestr (uu.loginpwtype));
          putf("S&F-pwtype.(A SF).%s\n", pwtypestr (uu.sfpwtype));
#endif
          putf("PW-Line......(A PWL).%d\n", uu.pwline);
        }
        putf("Readlock.....(A REA).%-20d", uu.readlock);
#ifdef FBBCHECKREAD
        putf("FBBCheckmode (A FBB).%d\n", uu.fbbcheckmode);
#else
        putf("\n");
#endif
      }
      putf("Binmode........(A B).");
      switch (u->binmode)
      {
#ifdef FEATURE_DIDADIT
             case BINMODE_DIDADIT: putf("Didadit "); break;
#endif
#ifdef FEATURE_BINSPLIT
             case BINMODE_BINSPLIT: putf("BinSplit"); break;
#endif
#ifdef FEATURE_YAPP
             case BINMODE_YAPP: putf("Yapp    "); break;
#endif
             default: putf("AutoBIN ");
      }
#ifdef __FLAT__
      putf("            HTTP-Surface.........%d\n", uu.httpsurface);
#else
      putf("\n");
#endif
      putf("Away...........(A A).%-20d", uu.away);
      if (uu.away == 1 && uu.awayendtime)
        putf("Awayend.(A AWAYE).%s\n", datestr(uu.awayendtime, 2));
      else
        putf("\n");
      if (uu.away == 1 && *uu.awaytext)
        putf("Away Message: %s\n", uu.awaytext);
      if (*uu.notification || b->sysop)
        putf("Notification.(A NOT).%s\n",
             *uu.notification ? uu.notification : "off");
      //putf("RLimit........(A RL).%d\n", uu.rlimit);
    }
    else putf(ms(m_a_name2), uu.name);
    if (uu.lastboxlogin)
    {
      if (uu.lastdirnews)
        putf(ms(m_a_lastdirnew), datestr(uu.lastdirnews, 12));
      if (uu.lastquit)
        putf(ms(m_a_lastquit), datestr(uu.lastquit, 12));
      z = datestr(uu.lastboxlogin, 12);
    }
    else
      z = ms(m_neverlogin);
    putf(ms(m_a_lastlogin), z);
    if (*uu.uplink && uu.lastboxlogin) putf("  via %s", uu.uplink);
    putv(LF);
    putf(ms(m_a_loginmails), uu.logins, uu.mailgot, uu.mailread, uu.mailsent);
  }
  else
  {
    putf(ms(m_isunknown), call);
  }
}

/*---------------------------------------------------------------------------*/

static void near changeoption (char *buf, int optgroup)
//****************************************************************************
//
//****************************************************************************
{
  b->optminus = b->optplus = 0;

  if (*buf)
  {
    if (strlen(buf) == 1 || strchr(buf, '-') || strchr(buf, '+'))
    {
      if (isalpha(*buf))
      {
        if (buf[1] == '-')  b->optminus |= (1L << (*buf - 'A'));
        else                b->optplus  |= (1L << (*buf - 'A'));
      }
      else if (*buf == '-') b->optminus |= (1L << (buf[1] - 'A'));
      else if (*buf == '+') b->optplus  |= (1L << (buf[1] - 'A'));
      b->optminus = (~b->optminus);
      u->opt[optgroup] = (u->opt[optgroup] & b->optminus) | b->optplus;
    }
    else
    {
      u->opt[optgroup] = 0;
      while (*buf)
      {
         if (isalpha(*buf)) u->opt[optgroup] |= (1L << (*buf - 'A'));
        buf++;
      }
    }
  }
  putf(ms(m_optionsfor), optstr(u->opt[optgroup]));
  switch (optgroup)
  {
    case o_id: putf("Info-Dir");  break;
    case o_ud: putf("User-Dir");  break;
    case o_il: putf("Info-List"); break;
    case o_ul: putf("User-List"); break;
    case o_ir: putf("Info-Read"); break;
    case o_ur: putf("User-Read"); break;
    case o_ch: putf("Check");     break;
    case o_ps: putf("PS");        break; //df3vi: A GREP ausgebaut
  }
  putf(ms(m_optionsset));
}

/*---------------------------------------------------------------------------*/

int mbalter (char *bef, char *selektor, char *call)
//****************************************************************************
//
//****************************************************************************
{
  static char *beftab[]=
    { "COMMAND", "SPEECH", "FORWARD", "HELPLEVEL", "LINES", "DEFAULT",
      "NAME", "PROMPT", "REJECT", "TTYPW", "PW", "LF", "UMLAUT",
      "IDIR", "UDIR", "ILIST", "ULIST", "IREAD", "UREAD", "CHECK", "PS",
      "STATUS", "QUOTA", "NOPURGE", "PWLINE", "READLOCK", "ECHO", "MYBBS",
      "DELETE", "FDELAY", "FHOLD", "RLIMIT", "UFWD", "BINMODE", "PACLEN",
#ifdef __FLAT__
      "UNSECURESMTP", "HTTPSURFACE",
#endif
#ifdef FBBCHECKREAD
      "FBBCHECKMODE",
#endif
      "LOGINPWTYPE", "SFPWTYPE", "NEWCALL", "DIRFORMAT", "AWAY", "QTH",
      "ZIP", "AWAYENDTIME", "NOTIFICATION",
#ifdef __LINUX__
      "LINUXPW",
#endif
#ifdef _USERCOMP
      "COMP",
#endif
      NULL
    };

  enum befnum
    { unsinn,
      command, speech, forward, helplevel, lines, adefault,
      name, prompt, reject, ttypw, pw, lf, umlaut,
      idir, udir, ilist, ulist, iread, uread, check, ps,
      status, quota, nopurge, pwline, readlock, echo, mybbs_,
      delete_, fdelay, fhold, rlimit, ufwd, binmode,
      paclen,
#ifdef __FLAT__
      unsecuresmtp, httpsurface,
#endif
#ifdef FBBCHECKREAD
      fbbcheckmode,
#endif
      loginpwtype, sfpwtype, newcall, dirformat, away, qth,
      zip, awayendtime, notification,
#ifdef __LINUX__
      linuxpw,
#endif
#ifdef _USERCOMP
      comp_
#endif

    } cmd = unsinn;
  static char funcname[] = "mbalter";
#ifdef _GUEST
    if (! strcmp(b->logincall, m.guestcall))
    {
      putf(ms(m_notpossguest));
      return 0;
    }
#endif
  int optgroup = 0;
  int del_user_cache = 0;

  if (! loaduser(call, u, 1)) return 0;
  selektor += blkill(selektor);
  if (! bef && ! *selektor)
  {
    display_parameter(call, 1);
    return 1;
  }
  if (bef)
    cmd = (befnum) readcmd(beftab, &bef, 0);
  else
    cmd = (befnum) readcmd(beftab, &selektor, 0);
//Negative lists
    if (b->job == j_tell)
    switch (cmd) //list of commands not available for "tell"
    {
      case command: case prompt: case newcall: case pw: case pwline:
      case fhold: case rlimit: case readlock: case delete_: case helplevel:
      case dirformat: case lf: case lines:
      case adefault: case name: case reject: case ps: case check: case ulist:
      case ilist: case idir: case udir: case iread: case uread: case status:
      case nopurge: case ttypw: case ufwd: case loginpwtype: case sfpwtype:
      case paclen: case away: case qth: case zip: case awayendtime:
      case notification:
#ifdef __FLAT__
      case httpsurface:
#endif
#ifdef __LINUX__
      case linuxpw:
#endif
#ifdef _USERCOMP
      case comp_:
#endif
#ifdef FBBCHECKREAD
      case fbbcheckmode:
#endif
      {
        b->eingabefehler += 5;
        putf(ms(m_alterunknown));
        return 1;
      }
      default:
        *selektor = 0; // do not allow parameters to be changed
        break;
    }
    if (m.userpw >= 2 && ! b->pwok && ! b->sysop)
    switch (cmd) // list of commands not available for "unpriv" users
    {
      case forward: case mybbs_: case newcall: case pw: case pwline:
      case fhold: case rlimit: case readlock: case delete_:
      case adefault: case ps: case check: case ulist: case ilist:
      case idir: case udir: case iread: case uread: case status:
      case nopurge: case ttypw: case ufwd: case loginpwtype: case sfpwtype:
      case paclen: case away: case awayendtime:
      case notification:
#ifdef __FLAT__
      case httpsurface:
#endif
#ifdef __LINUX__
      case linuxpw:
#endif
#ifdef _USERCOMP
      case comp_:
#endif
#ifdef FBBCHECKREAD
      case fbbcheckmode:
#endif
      {
        putf(ms(m_a_no_pw));
        return 1;
      }
      default: *selektor = 0;   //do not allow parameters to be changed
    }
#ifdef _BCMNET_LOGIN
    if (! b->logintype && ! b->sysop)
    switch (cmd)  //list of commands not available for guests
    {
      case name: case prompt: case command: case fdelay:
      case forward: case mybbs_: case newcall: case pw: case pwline:
      case fhold: case rlimit: case readlock: case delete_:
      case adefault: case ps: case check: case ulist: case ilist:
      case idir: case udir: case iread: case uread: case status:
      case nopurge: case ttypw: case ufwd: case loginpwtype: case sfpwtype:
      case paclen: case away: case qth: case zip: case awayendtime:
      case notification:
#ifdef __FLAT__
      case httpsurface:
#endif
#ifdef __LINUX__
      case linuxpw:
#endif
#ifdef _USERCOMP
      case comp_:
#endif
#ifdef FBBCHECKREAD
      case fbbcheckmode:
#endif
      {
        putf(ms(m_a_no_pw));
        return 1;
      }
      default: {}
    }
#endif
  switch (cmd)
  {
    case unsinn:
         {
           if (mbcallok(selektor) == 1)
             display_parameter(selektor, 1);
           else
             putf(ms(m_alterunknown));
         } break;
    case forward:
    case mybbs_:
         {
           char *cp;
           cp = NULL;
           time_t tmx = 0;
           scanoptions(selektor);
           selektor += blkill(selektor);
           strupr(selektor);
           cp = skip(selektor);
           if (cp) tmx = atol(cp);
           if (! *selektor || b->job == j_tell)
           { //df3vi: tell nur ohne argument
             if (*u->mybbs) putf(ms(m_dispmybbs), u->mybbs);
             else putf(ms(m_inputaddress));
           }
           else if (! mybbs(selektor, ! (b->optplus & o_l), tmx))
                  putf(ms(m_adrinvalid));
         } break;
    case command:
         {
           strupr(selektor);
           if (strlen(selektor) > FIRSTCMDLEN)
             putf(ms(m_cmdtoolong));
           else
             if (*selektor == 'Q')
               putf(ms(m_commandquit));
             else
             {
               strcpy(u->firstcmd, selektor);
               putf(ms(m_cmdstored), selektor);
             }
         } break;
    case prompt:
         {
           if (strlen(selektor) > PROMPTLEN)
             putf(ms(m_prompttoolong));
           else
           {
             if (*selektor) strcpy(u->prompt, selektor);
             else strcpy(u->prompt, m.prompt);
             putf(ms(m_promptstored));
           }
         } break;
    case newcall:
         {
           selektor += blkill(selektor);
           strupr(selektor);
           if (! *selektor || ! strcmp(selektor, u->call))
           {
             memset(u->newcall, 0, CALLEN+1);
             putf(ms(m_newcallremoved));
             break;
           }
           if (strlen(selektor) > CALLEN || ! mbcallok(selektor))
           {
             putf(ms(m_newcallinvalid));
             break;
           }
           strcpy(u->newcall, selektor);
           putf(ms(m_newcallstored), u->newcall);
           trace(replog, funcname, "%s newcall %s", u->call, u->newcall);
         } break;
    case pw:
         {
           lastcmd("-");
           if (! stricmp(selektor, "disable"))
           {
             u->password[0] = 1;
             u->password[1] = 0;
             u->loginpwtype = 0;
             u->sfpwtype = 0;
             u->pwline = 0;
           }
           else
           {
             if ((m.userpw && *u->password != 1) || b->sysop)
             {
               if ((strlen(selektor) + strlen(u->password)) > 39)
                 putf(ms(m_pwdtoolong));
               else
               {
                 if (*selektor == 0 || ! stricmp(selektor, "off"))
                 {
                   *u->password = 0;
                   u->loginpwtype = 0;
                   u->sfpwtype = 0;
                   u->pwline = 0;
                   putf(ms(m_pwderased));
                   pwlog(b->logincall, b->uplink, "Password removed.");
                 }
                 else
                 {
                   if ((*u->password == 1) && b->sysop) *u->password = 0;
                   char pwlstr[60];
                   int oldlen = strlen(u->password);
                   strcat(u->password, selektor);
                   int newlen = strlen(u->password);
                   putf(ms(m_usingpw), pwtypestr(u->loginpwtype),
                                       pwtypestr(u->sfpwtype));
                   putf(" ");
                   putf(ms(m_pwdlength), newlen);
                   sprintf(pwlstr, "pw appended (len %d -> %d)",
                                   oldlen, newlen);
                   pwlog(b->logincall, b->uplink, pwlstr);
                 }
               }
             }
             else
               putf(ms(m_sysoponly));
           }
           if (*u->password == 1)
             putf(ms(m_pwdisabled));
         } break;
    case pwline:
         {
           if (*u->password < 2)
           {
             putf(ms(m_nopwset));
             break;
           }
           if (*selektor) u->pwline = !! atoi(selektor);
           putf("PWLINE %d\n", u->pwline);
         } break;
    case echo:
         {
           if (*selektor) u->echo = !! atoi(selektor);
           if (u->echo)
             putf(ms(m_echoon));
           else
             putf(ms(m_echooff));
         } break;
#ifdef __FLAT__
    case unsecuresmtp:
         {
           if (*selektor && b->job != j_tell) //df3vi: tell nur ohne argument
             u->unsecure_smtp = !! atoi(selektor);
           if (u->unsecure_smtp)
             putf(ms(m_unsecuresmtpon));
           else
             putf(ms(m_unsecuresmtpoff));
         } break;
#endif
    case fdelay: // usermails and bulletins will be delayed by xx min
         {
           if (*selektor && b->job != j_tell) //df3vi: tell nur ohne argument
           {
             short unsigned a = (short unsigned) atoi(selektor);
             if (a <= 60) u->fdelay = a;
             else
             {
               putf("Syntax: FDELAY 0..60\n");
               break;
             }
           }
           putf("FDELAY %d min\n", u->fdelay);
         } break;
    case fhold: // bulletins will not be forwarded automatically
         {
           if (*selektor && b->sysop) u->fhold = !! atoi(selektor);
           putf("FHOLD %d\n", u->fhold);
         } break;
    case rlimit:
         {
           if (*selektor && b->sysop) u->rlimit = !! atoi(selektor);
             putf("RLIMIT %d\n", u->rlimit);
         } break;
    case readlock:
         {
           if (*selektor)
           {
             int num = atoi(selektor);
             if (num < 0 || num > 2)
             {
               putf("Syntax: ALTER READLOCK 0..2\n");
               break;
             }
             if (b->sysop || m.readlock)
               u->readlock = num;
             else
               putf(ms(m_sysoponly));
           }
           putf("READLOCK %d\n", u->readlock);
         } break;
    case delete_:
         {
           if (b->sysop && strcmp(u->call, b->logincall))
           {
             putf(ms(m_deleted), u->call);
             *u->call = 0;
             us_deletecache();
             del_user_cache = 1;
           }
         } break;
    case umlaut:
         {
          /*
          um iso ascii-pc
          Ae c4 8e
          Oe d6 99
          Ue dc 9a
          ae e4 84
          oe f6 94
          ue fc 81
          ss df e1
          strupr(selektor);
          if (strstr("AEOEUE", selektor))
          {
            u->umlaut=um_aeoeue;
            putf("Umlaute werden als ae oe ue dargestellt.\n");
          }
          else if (strstr("DIN", selektor))
          {
            u->umlaut=um_din;
            putf("DIN-Umlaute aktiviert.\n");
          }
          else if (strstr("IBM", selektor))
          {
            u->umlaut=um_ibm;
            putf("IBM-Umlaute aktiviert.\n");
          }
          else
          {
            u->umlaut=um_none;
            putf("Umlautwandlung abgeschaltet.\n");
          }
          */
         } break;
    case helplevel:
         {
           int h = atoi(selektor);
           if (h < 0 || h > 2)
             putf(ms(m_helpvalues));
           else
           {
             u->helplevel = h;
             putf(ms(m_helpstored), h);
           }
         } break;
    case dirformat:
         {
           int h = atoi(selektor);
           if (h < 0 || h > 1)
             putf("Syntax: ALTER DIRFORMAT 0 | 1\n");
           else
           {
             u->dirformat = h;
             putf(ms(m_dirformatsaved), u->dirformat);
           }
         } break;
    case lf:
         {
           int h = atoi(selektor);
           if (h < -1 || h > 5)
             putf(ms(m_lfvalues));
           else
           {
             if (h == -1) h=6;
             u->lf = h;
             if (! u->lf)
               putf(ms(m_nolinefeeds));
             else
               if (u->lf == 1)
                 putf(ms(m_onelinefeed), h);
               else
                 if (u->lf == 6)
                   putf(ms(m_reallynolinefeeds));
                 else
                   putf(ms(m_linefeeds), h);
           }
         } break;
    case lines:
         {
           int linecnt = atoi(selektor);
           if (! linecnt)
           {
             u->zeilen = 0;
             putf(ms(m_nolines));
           }
           else
             if (linecnt > 3)
             {
               u->zeilen = linecnt;
               putf(ms(m_linesset), linecnt);
             }
             else
               putf(ms(m_linesvalues));
         } break;
    case paclen:
         {
           if (b->sysop || m.paclen)
           {
             int h = atoi(selektor);
             if (   (h <= m.maxpaclen && h >= m.minpaclen) //range min-maxpaclen
                 || (m.maxpaclen == 256 && h == 0)       //also 0 if maxp.=256
                 || (b->sysop && h <= 256 && h >= 0)     //0-256 if sysop
                 || (!*selektor)                         //display paclen
                )
             {
               if (*selektor) u->paclen = h;
               putf("Paclen %i\n", u->paclen);
#ifndef _AX25K_ONLY
 #ifdef __FLAT__
               set_paclen_tnc(u->paclen);
 #endif
#endif
             }
             else
             {
               putf(ms(m_validpaclenrange), m.minpaclen, m.maxpaclen);
               if (m.maxpaclen == 256)
                 putf(ms(m_paclendisable));
             }
           }
           else
             putf(ms(m_sysoponly));
         } break;
    case adefault:
         {
           if (! b->pwok && ! b->sysop)
             putf(ms(m_sysoponly));
           else
           {
             putf(ms(m_default));
             if (janein(ms(m_defaultquery)) == JA)
             {
               putf(ms(m_defaultmessage));
               defaultuser(call, u);
               u->status = 0;
             }
           }
         } break;
    case name:
         {
           if (strlen(selektor) > NAMELEN)
             putf(ms(m_nametoolong));
           else
           {
             unsigned int i = 0, gross = 0;
             for (i = 0; selektor[i]; i++)
             {
               if (isupper(selektor[i])) gross++;
             }
             if (islower(*selektor) || gross == strlen(selektor))
             {
               strlwr(selektor);
               *selektor = toupper(*selektor);
             }
             strcpy(u->name, selektor);
             u->nameok = 1;
             putf(ms(m_nameset), selektor);
           }
         } break;
    case qth:
         {
           if (strlen(selektor) > QTHLEN)
             putf(ms(m_qthtoolong));
           else
           {
             unsigned int i = 0, gross = 0;
             for (i = 0; selektor[i]; i++)
             {
               if (isupper(selektor[i])) gross++;
             }
             if (islower(*selektor) || gross == strlen(selektor))
             {
               strlwr(selektor);
               *selektor = toupper(*selektor);
             }
             strcpy(u->qth, selektor);
             u->qthok = 1;
             putf(ms(m_qthset), selektor);
           }
         } break;
    case zip:
         {
           if (strlen(selektor) > ZIPLEN)
             putf(ms(m_ziptoolong));
           else
           {
             unsigned int i = 0, gross = 0;
             for (i = 0; selektor[i]; i++)
             {
               if (isupper(selektor[i])) gross++;
             }
             if (islower(*selektor) || gross == strlen(selektor))
             {
               strlwr(selektor);
               *selektor = toupper(*selektor);
             }
             strcpy(u->zip, selektor);
             u->zipok = 1;
             putf(ms(m_zipset), selektor);
           }
         } break;
    case reject:
         {
           strupr(selektor);
           if (strlen(selektor) > (LINELEN - 4))
             putf(ms(m_toomanyreject));
           else
           {
             strcpy(u->notvisible + 1, selektor);
             *u->notvisible = ' ';
             strcat(u->notvisible, " ");
             putf(ms(m_rejectedboards), selektor);
           }
         } break;
    case speech:
         {
           char *spkenn;
           strupr(selektor);
           spkenn = msg_landsuch(selektor);
           if (*spkenn != '?' && b->job != j_tell) //df3vi: tell nur ohne argument
           {
             strcpy(u->sprache, spkenn);
             b->msg_loadnum = 0;
             putf(ms(m_languageok), spkenn);
           }
           else
           {
             putf(ms(m_possiblespeech));
             msg_listspeech();
           }
         } break;
    case ps:    optgroup++;
    case check: optgroup++;
    case uread: optgroup++;
    case iread: optgroup++;
    case ulist: optgroup++;
    case ilist: optgroup++;
    case udir:  optgroup++;
    case idir:
         {
           if (*selektor && test_job(j_tell))
             break; //df3vi: tell nur ohne argument
           strupr(selektor);
           changeoption(selektor, optgroup);
         } break;
    case status:
         {
           if (b->sysop)
           {
             if (*selektor) u->status = atoi(selektor);
             putf(ms(m_userstateset), call, u->status);
           }
           else
             putf(ms(m_sysoponly));
         } break;
    case nopurge:
         {
           if (b->sysop || (m.nopurge & 2))
           {
             if (*selektor) u->nopurge = !! atoi(selektor);
           }
           if (u->nopurge)
             putf(ms(m_nopurgeoff));
           else
             putf(ms(m_nopurgeon));
         } break;
    case quota:
         {
           if (*selektor && b->sysop)
           {
             b->rxbytes = b->txbytes = 0;
             u->daybytes = atol(selektor);
             putf(ms(m_quotaset), u->daybytes);
             u->daybytes <<= 10;
           }
           else
           {
             long rambytes = 0;
             if (! strcmp(b->logincall, call))
               rambytes = b->rxbytes + b->txbytes;
             putf(ms(m_quotaoutput), (u->daybytes + rambytes) >> 10);
             if (m.userquota)
               putf(ms(m_maxquota), m.userquota);
           }
         } break;
    case ttypw:
         {
#ifdef __FLAT__
           switch (m.unsecurettypw)
           {
             case 1:
                  {
                     if (b->sysop || *u->ttypw || b->pwok)
                     {
                       if (strlen(selektor) < 9)
                       {
                         strcpy(u->ttypw, selektor);
                         if (strlen(selektor) == 0)
                           putf(ms(m_ttypwdisabled));
                         else
                           putf(ms(m_ttypwok));
                       }
                       else
                         putf(ms(m_ttypw8char));
                     }
                     else
                       putf(ms(m_sysoponly));
                     lastcmd("-");
                     break;
                  }
             case 2:
                  {
                    if (strlen(selektor) < 9)
                    {
                      strcpy(u->ttypw, selektor);
                      if (strlen(selektor) == 0)
                        putf(ms(m_ttypwdisabled));
                      else
                        putf(ms(m_ttypwok));
                    }
                    else
                      putf(ms(m_ttypw8char));
                    lastcmd("-");
                    break;
                  }
             default:
#endif
                  {
                    if (b->sysop || *u->ttypw)
                    {
                      if (strlen(selektor) < 9)
                      {
                        strcpy(u->ttypw, selektor);
                        if (strlen(selektor) == 0)
                          putf(ms(m_ttypwdisabled));
                        else
                          putf(ms(m_ttypwok));
                      }
                      else
                        putf(ms(m_ttypw8char));
                    }
                    else
                      putf(ms(m_sysoponly));
                    lastcmd("-");
                  }
#ifdef __FLAT__
           }
#endif
         } break;
#ifdef __LINUX__
    case linuxpw:
         {
#ifndef LINUXSYSTEMUSER
           putf(ms(m_featnotincluded));
#else
           if (m.addlinuxsystemuser)
           {
             {
               if (strlen(selektor) < 9)
               {
                 if (strlen(selektor) == 0)
                   putf(ms(m_linuxpwset));
                 //hier ggf. mal spaeter einen aufruf zum loeschen des
                 //linux-users
                 else
                 {
                   strcpy(u->linuxpw, selektor);
                   if (setlinuxpasswd(u->call, u->linuxpw) > 0)
                      putf(ms(m_linuxpwok));
                 }
               }
               else
                 putf(ms(m_linuxpw8char));
               lastcmd("-");
             }
           }
#endif
         } break;
#endif
#ifdef _USERCOMP
    case comp_:
         {
           if (! stricmp(selektor, "1"))
           {
             putf(ms(m_huffcompon));
             u->comp = 1;
           }
           else
           {
             putf(ms(m_huffcompoff));
             u->comp = 0;
           }
         } break;
#endif
    case ufwd:
         { //check for key words
           if (! stricmp(selektor, "PASSIV")
               || ! stricmp(selektor, "PASSIVE"))
             *u->ufwd = 0;
           else
             if (! stricmp(selektor, "OFF"))
             {
               u->ufwd[0] = 1;
               u->ufwd[1] = 0;
             }
             //check for correct length/syntax of path
             else if (   strlen(selektor) < 5
               || strlen(selektor) > (sizeof(u->ufwd) - 1)
               || strstr(selektor, "."))
               putf("Syntax: ALTER UFWD <path> | PASSIVE | OFF\n");
             //check for correct mybbs
             else if (strcmp (atcall (u->mybbs), m.boxname))
               putf(ms(m_mybbsnotset), m.boxadress);
             else strcpy(u->ufwd, selektor);
             //messages
           if (! *u->ufwd)
             putf(ms(m_userfwdpassive));
           else if (*u->ufwd == 1)
             putf(ms(m_userfwdoff));
           else
             putf(ms(m_userfwdon), u->ufwd);
         } break;
    case loginpwtype:
         {
           if (*u->password < 2 )
           {
             putf(ms(m_nopwset));
             break;
           }
           char type = pwtypenum(selektor);
           switch (type)
           {
             case 100:
                  putf(ms(m_loginpwinactive));
                  u->loginpwtype = 100;
                  break;
             case 101:
#ifdef FEATURE_MDPW
                  putf("Syntax: ALTER LOGINPWTYPE BAYCOM | MD2 | MD5 | INACTIVE\n");
#else
                  putf("Syntax: ALTER LOGINPWTYPE BAYCOM | INACTIVE\n");
#endif
                  break;
             default:
                  u->loginpwtype = type;
                  putf(ms(m_loginpwtype), pwtypestr(type));
           }
         } break;
    case sfpwtype:
         {
           if (*u->password < 2)
           {
             putf(ms(m_nopwset));
             break;
           }
           char type = pwtypenum(selektor);
           switch (type)
           {
             case 100:
                  putf(ms(m_sfpwinactive));
                  u->sfpwtype = 100;
                  break;
             case 101:
#ifdef FEATURE_MDPW
                  putf("Syntax: ALTER LOGINPWTYPE BAYCOM | MD2 | MD5 | INACTIVE\n");
#else
                  putf("Syntax: ALTER LOGINPWTYPE BAYCOM | INACTIVE\n");
#endif
                  break;
             default:
                  u->sfpwtype = type;
                  putf(ms(m_sfpwtype), pwtypestr(type));
           }
         } break;
    case binmode: // DH3MB
         {
           if (*selektor)
           {
             if (! stricmp(selektor, "AUTOBIN")) u->binmode = BINMODE_AUTOBIN;
#ifdef FEATURE_YAPP
             else if (! stricmp(selektor, "YAPP")) u->binmode = BINMODE_YAPP;
#endif
#ifdef FEATURE_DIDADIT
             else if (! stricmp(selektor, "DIDADIT")) u->binmode = BINMODE_DIDADIT;
#endif
#ifdef FEATURE_BINSPLIT
             else if (! stricmp(selektor, "BINSPLIT")) u->binmode = BINMODE_BINSPLIT;
#endif
             else
             {
               putf("Syntax: ALTER BINMODE AUTOBIN"
#ifdef FEATURE_YAPP
                    " | YAPP"
#endif
#ifdef FEATURE_DIDADIT
                    " | DIDADIT"
#endif
#ifdef FEATURE_BINSPLIT
                    " | BINSPLIT"
#endif
                    "\n");
               break;
             }
           }
           putf(ms(m_binpartssent));
           putf(" ");
           switch (u->binmode)
           {
#ifdef FEATURE_DIDADIT
             case BINMODE_DIDADIT: putf("DIDADIT"); break;
#endif
#ifdef FEATURE_BINSPLIT
             case BINMODE_BINSPLIT: putf("BINSPLIT"); break;
#endif
#ifdef FEATURE_YAPP
             case BINMODE_YAPP: putf("YAPP"); break;
#endif
             default: putf("AutoBIN");
           }
           putf(" ");
           putf(ms(m_protocol));
           break;
         }
#ifdef FBBCHECKREAD
    case fbbcheckmode:
         {
           if (*selektor) u->fbbcheckmode = !! atoi(selektor);
           if (u->fbbcheckmode)
             putf(ms(m_fbbcheckmodeon));
           else
             putf(ms(m_fbbcheckmodeoff));
         } break;
#endif
    case notification:
         {
           selektor += blkill(selektor);
           strupr(selektor);
           if (! *selektor || ! strcmp(selektor, u->call))
           {
             memset(u->notification, 0, CALLEN+1);
             putf(ms(m_notidisabled));
             break;
           }
           if (strlen(selektor) > CALLEN || ! mbcallok(selektor))
           {
             putf(ms(m_noticallinvalid));
             break;
           }
           strcpy(u->notification, selektor);
           putf(ms(m_notienabled), u->notification);
         } break;
#ifdef __FLAT__
    case httpsurface:
         {
           if (*selektor)
           {
             int h = atoi(selektor);
             if (h < 0 || h > 3)
               putf("Syntax: ALTER HTTPSURFACE 0 | 1 | 2 | 3\n");
             else
             {
               u->httpsurface = h;
               putf("HTTP-Surface = %d ", u->httpsurface);
               switch (u->httpsurface)
               {
                 case 0: putf(ms(m_httpsurface0)); break;
                 case 1: putf(ms(m_httpsurface1)); break;
                 case 2: putf(ms(m_httpsurface2)); break;
                 case 3: putf(ms(m_httpsurface3)); break;
               }
               putf(" ");
               putf(ms(m_saved));
               if (b->http)
                 html_putf("Click <a href=\"/\" target=\"_parent\">here</a> to use this surface immediately!\n");
             }
           }
           else
           {
             putf("HTTP-Surface = %d ", u->httpsurface);
             switch (u->httpsurface)
             {
               case 0: putf(ms(m_httpsurface0)); break;
               case 1: putf(ms(m_httpsurface1)); break;
               case 2: putf(ms(m_httpsurface2)); break;
               case 3: putf(ms(m_httpsurface3)); break;
             }
           }
         } break;
#endif
    case away:
         if (strlen(selektor) > LINELEN-1)
         {
           putf(ms(m_awaymsgtoolong));
           u->away = 0;
         }
         else
         {
           if (*selektor)
           {
             if (! stricmp(selektor, "0") ||
                 ! stricmp(selektor, "OFF") ||
                 ! stricmp(selektor, "off"))
             {
               putf(ms(m_awaydisabled));
               u->away = 0;
             }
             else
             {
               strcpy(u->awaytext, selektor);
               putf(ms(m_awayenabled));
               putf("%s\n", u->awaytext);
               u->away = 1;
               u->awayendtime = 0;
             }
           }
         } break;
    case awayendtime:
         {
           if (! u->away)
           {
             putf(ms(m_awaydisablednoend));
             break;
           }
           if (*selektor)
           {
             if (! stricmp(selektor, "0") ||
                 ! stricmp(selektor, "OFF") ||
                 ! stricmp(selektor, "off"))
             {
               putf(ms(m_awayendtimedisabled));
               u->awayendtime = 0;
             }
             else
             {
               u->awayendtime = parse_time(selektor);
               if (u->awayendtime > 0)
               putf(ms(m_awayendtime), datestr(u->awayendtime, 2));
             }
           }
         } break;
  }
  if (b->job == j_tell) return 1; // do not allow any changes via tell
  saveuser(u);
  if (del_user_cache) us_deletecache();
  return 1;
}

/*---------------------------------------------------------------------------*/
