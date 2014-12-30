/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------
  POP3 for BCM   Reference: RFC 1725
  ------------


  Copyright (C)       Deti Fliegl, deti@lrz.de, DG9MHZ
                      Guardinistr. 47
                      81375 Muenchen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980505 DG9MHZ added pop3db
//19980506 OE3DZW changes to pop3db
//19980619 OE3DZW show_pop - added fclose
//19991003 DH3WW  added APOP command
//19991128 Jan    close connection on too long cmd(80 chars) - avoids overflow
//19991212 Jan    clean-up, RETR uses mbread - no tempfile now
//20030101 hpk    for the CB-BCMNET login-concept: if user is authenticated,
//                his logintype is 'full'
//20030407 dap900 POP3 TOP-Befehl fuer fetchmail hinzu
//                (dap900 = Daniel Parthey [dap900@gmx.de])


#include "baycom.h"

/*---------------------------------------------------------------------------*/

void pop3::onkill (void)
//*************************************************************************
//
//*************************************************************************
{
  if (login) unlink(lockfile);
}

/*---------------------------------------------------------------------------*/

static void onkill (void *p)
//*************************************************************************
//
//*************************************************************************
{
  ((pop3 *)p)->onkill();
}

/*---------------------------------------------------------------------------*/

void pop3::add_to_popdb (void)
//*************************************************************************
//
//*************************************************************************
{
  char peeraddr[32];
  char name[32];

  strcpy(peeraddr, "");
  time_t lastpop, now = ad_time();
  char added = FALSE;
  FILE *g = s_fopen(POPDBTMPNAME, "swt");
  if ((fi = s_fopen(POPDBNAME, "srt")) != NULL)
  {
    while (! feof(fi))
    {
      fscanf(fi, "%31s\t%08lX\t%31s\n", peeraddr, &lastpop, name);
      if (! strcmp(peeraddr, b->peerip) && ! stricmp(user, name))
      {
        lastpop = now;
        added = TRUE;
      }
      if (   (now-lastpop) < DAY * 365  //keep data max. 1 year
          && (lastpop-now) < HOUR )             //max 1h
        fprintf(g, "%s\t%08lX\t%s\n", peeraddr, lastpop, name);
    }
    s_fclose(fi);
  }
  if (! added) fprintf(g, "%-15.15s\t%08lX\t%s\n", b->peerip, now, user);
  s_fclose(g);
  xunlink(POPDBNAME);
  xrename(POPDBTMPNAME, POPDBNAME);
}

/*---------------------------------------------------------------------------*/

void pop3::retr_msg (int msg_)
//*************************************************************************
//
//*************************************************************************
{
  msg_ = search_msg(msg_);
  if (msg_)
  {
    b->usermail = OK;
    b->optgroup = o_ur;
    u->opt[b->optgroup] = o_w | o_r;
    sprintf(b->line, "%s %d", user, msg_);
    mbread(b->line, 0); //mbread puts +OK/-ERR now
    putv(LF);
    putv('.');
    putv(LF);
  }
  else
    putf("-ERR no such message.\n");
  unlink(tmp);
}

/*---------------------------------------------------------------------------*/

int pop3::search_msg (int msg_)
//*************************************************************************
//
//*************************************************************************
{
  int dummy;
  int msgcount = 0;
  int msgnum = 0;
  if (! (fi = s_fopen(lockfile, "srt"))) return 0;
  while (fscanf(fi, "%d %d %s", &msgnum, &dummy, b->line) == 3)
  {
    msgcount++;
    if (msg_ == msgcount) break;
  }
  s_fclose(fi);
  if (msg_ == msgcount) return msgnum;
  return 0;
}

/*---------------------------------------------------------------------------*/

int pop3::put_uidl (int msg_)
//*************************************************************************
//
//*************************************************************************
{
  int dummy;
  int msgcount = 0;
  if (! (fi = s_fopen(lockfile, "srt")))
  {
    putf("-ERR Sorry, I lost my lockfile... shit happens.\n");
    return NO;
  }
  putf("+OK ");
  while (fscanf(fi, "%d %d %s", &dummy, &dummy, b->line) == 3)
  {
    msgcount++;
    if (! msg_)
      putv(LF);
    if (! msg_ || msg_ == msgcount)
      putf("%d %s", msgcount, b->line);
  }
  s_fclose(fi);
  putv(LF);
  if (! msg_)
  {
    putv('.');
    putv(LF);
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

// messages normally are to be deleted in update state (which is entered
// right after the quit command) we wont do this here.
int pop3::dele_msg (int msg_)
//*************************************************************************
//
//*************************************************************************
{
  int ret = 0;
  msg_ = search_msg(msg_);
  if (msg_)
  {
    b->oldoutput = (char) t->output;
    t->output = io_dummy;
    sprintf(b->line, "%s %d", user, msg_);
    ret = mbchange(b->line, w_erase, 1);
    t->output = (io_t) b->oldoutput;
  }
  if (ret) putf("+OK message deleted\n");
  else putf("-ERR message not deleted\n");
  return ret;
}

/*---------------------------------------------------------------------------*/

int pop3::init_list (void)
//*************************************************************************
//
//*************************************************************************
{
  b->job = j_search;
  b->optgroup = o_ud;
  u->opt[b->optgroup] = o_i | o_h | o_b;  // | o_v;
  b->oldoutput = (char) t->output;
  t->output = io_file;
  if (! (b->outputfile = s_fopen(lockfile, "swt")))
  {
    putf("-ERR Sorry, I could not open a lockfile.\n");
    return NO;
  }
  mbdir(user, 0);
  s_fclose(b->outputfile);
  t->output = (io_t) b->oldoutput;
  return put_list(-1);
}

/*---------------------------------------------------------------------------*/

int pop3::put_list (int msg_)
//*************************************************************************
//
//*************************************************************************
//octets count is incorrect. should include message headers.
{
  int bytes;
  int rdmsg = 0;
  if (! (fi = s_fopen(lockfile, "srt")))
  {
    putf("-ERR Sorry, I have lost my lockfile... shit happens.\n");
    return NO;
  }
  if (! msg_) putf("+OK %d messages (%d octets)\n", msgcnt, bytecnt);
  bytecnt = msgcnt = 0;
  while (fscanf(fi, "%d %d %32s", &rdmsg, &bytes, b->line) == 3)
  {
    bytecnt += bytes;
    msgcnt++;
    if (! msg_) putf("%d %d\n", msgcnt, bytes);
    if (msg_ && msg_ == msgcnt) break;
  }
  s_fclose(fi);
  if (msg_ == -1) return OK;
  if (! msg_)
  {
    putv('.');
    putv(LF);
  }
  else if (msg_ == msgcnt) putf("+OK %d %d\n", msgcnt, bytes);
  else putf("-ERR no such message\n");
  return OK;
}

/*---------------------------------------------------------------------------*/

#ifdef FEATURE_MDPW
int pop3::extract_2argument (char *s, int size, char *s2, int size2)
//*************************************************************************
//
//*************************************************************************
{
  tmp = skip(args);
  if (! tmp || ! *tmp) return NO;
  safe_strncpy(s, args, size);
  safe_strncpy(s2, tmp, size2);
  return OK;
}
#endif

/*---------------------------------------------------------------------------*/

char *pop3::extract_argument ()
//*************************************************************************
//
//*************************************************************************
{
  skip (args);
  if (*args) return args;
  else return NULL;
}

/*---------------------------------------------------------------------------*/

void pop3::start_pop3 (void)
//*************************************************************************
//
//*************************************************************************
{
  static char *beftab[] =
  {
    "USER", "PASS", "QUIT", "STAT", "LIST",
    "RETR", "TOP", "DELE", "NOOP", "RSET", "UIDL", "APOP", NULL
  };
  enum befnum
  {
    _ERR, _USER, _PASS, _QUIT, _STAT, _LIST,
    _RETR, _TOP, _DELE, _NOOP, _RSET, _UIDL, _APOP
  } cmd_num;
  finish = NO;
  badcmd = 0;
  msgcnt = 0;
  *user = *pass = 0;
  *lockfile = 0;
  msg = 0;
  login = NO;
  waslocked = 0;

  atkill(::onkill, (void *) this);
  if (myhostname(b->line, BUFLEN - 1) == -1) return;
//dh3ww begin
  *md5pwstring = '<';
  for (char c = 0; c < 10; c++) // only lower case characters are generated here
    md5pwstring[c + 1] = random_max(10) + 48;
  md5pwstring[11] = '>';
  md5pwstring[12] = 0;
  putf("+OK %s " LOGINSIGN " POP3 Server %s\n", b->line, md5pwstring);
  md5pwstring[11] = 0;
//dh3ww ende
  while (! runterfahren)
  {
    getline(b->line, BUFLEN - 1, 1);
    if (*b->line)
    {
      if ((m.tcpiptrace == 3) || (m.tcpiptrace == 8))
      {
        if (! strstr(b->line, "PASS"))
          pop3log("RX", b->line);
        else
          pop3log("RX", "PASS (hidden)");
      }
      if (strlen (b->line) > LINELEN)
      {
        trace(replog, "pop3", "too long input from [%s]", b->peerip);
        putf("-ERR Too long input.\n");
        break;
      }
      args = b->line;
      cmd_num = (befnum) readcmd(beftab, &args, 0);
      if (! *user && cmd_num != _USER && cmd_num != _APOP && cmd_num != _QUIT)
        cmd_num = _ERR;
      if (*user && ! *pass && cmd_num != _PASS && cmd_num != _QUIT)
        cmd_num = _ERR;
      if (*user && *pass && (cmd_num == _USER || cmd_num == _PASS || cmd_num == _APOP))
        cmd_num = _ERR;
      switch (cmd_num)
      {
      case _USER:
        if (*user) goto err;
        if (! (tmp = extract_argument()))
          putf("-ERR Too few arguments for the user command.\n");
        else
        {
          safe_strcpy(user, tmp);
          putf("+OK Password required for %s\n", user);
        }
        break;
#ifdef FEATURE_MDPW     //dh3ww begin
      case _APOP:
        if (*pass || *user) goto err;
        if (! extract_2argument(user, sizeof(user) - 1, pass, sizeof(pass) - 1))
        {
          putf("-ERR Too few arguments.\n");
          goto ERR1;
        }
        else if (! mbcallok(user))
        {
          putf("-ERR Authorization failed.\n");
          goto ERR1;
        }
        else
        {
            get_pw(user, b->line); //PLEASE DO NOT CHANGE THIS!!
            calcMD5pw(md5pwstring + 1, b->line, md5result);
            if (! *md5result || stricmp(md5result, pass))
            {
//            trace(report, "pop3", "Authorization for %s failed [%s]", user, pass);
              pwlog(b->peerip, b->uplink, "bad APOP pw");
              putf("-ERR Authorization failed.\n");
              goto ERR1;
            }
            goto PASS1;
        }
        break;
#endif  //dh3ww end
      case _PASS:
        if (*pass || ! *user) goto err;
        if (m.disable) goto disabled;
        if (! (tmp = extract_argument()))
        {
          putf("-ERR Too few arguments for the pass command.\n");
          break;
        }
        else
        {
            safe_strcpy(pass, tmp);
            if (mbcallok(user))
            {
              if (isamprnet(b->peerip) > 0) get_httppw(user, b->line);
              else get_ttypw(user, b->line);
            }
            else *b->line = 0;
            if (! *b->line || strcasecmp(b->line, pass))
            {
//            trace(report, "pop3", "Authorization for %.40s failed [%.20s]", user, pass);
              pwlog(b->peerip, b->uplink, "bad pw");
              putf("-ERR Authorization failed.\n");
              goto ERR1;
            }
 PASS1:     sprintf(lockfile, TEMPPATH "/lock.%s", user);
            if (! waslocked && ! access(lockfile, 0))
            {
              putf("-ERR Maildrop allready locked.\n");
              waslocked++;
              goto ERR1;
            }
            add_to_popdb();    //fi is closed
            b->pwok = OK;
            login = OK;
            mblogin(user, login_silent, "POP3");
            b->continous = 1;
            b->charset = 1;
            b->oldoutput = (char) t->output;
            t->output = io_dummy;
            t->output = (io_t) b->oldoutput;
            init_list();
            goto rset;
        }
 ERR1:  *user = *pass = 0;
        break;
      case _QUIT:
        finish = 1;
        putf("+OK POP3 server signing off\n");
        break;
      case _STAT:
        putf("+OK %d %d\n", msgcnt, bytecnt);
        break;
      case _LIST:
        tmp = extract_argument();
        msg = (tmp ? atoi(tmp) : 0);
        put_list(msg);
        break;
      case _RETR:
        if (! (tmp = extract_argument()))
          putf("-ERR Too few arguments for the retr command.\n");
        else
        {
          msg = atoi(tmp);
          retr_msg(msg);
        }
        break;
      case _TOP:
        if (! (tmp = extract_argument()))
          putf("-ERR Too few arguments for the top command.\n");
        else
        {
          msg = atoi(tmp);
          retr_msg(msg);
        }
        break;
      case _DELE:
        if (! (tmp = extract_argument()))
          putf("-ERR Too few arguments for the dele command.\n");
        else
        {
          msg = atoi(tmp);
          dele_msg(msg);
        }
        break;
      case _NOOP:
        putf("+OK\n");
        break;
      case _RSET:
 rset:  putf("+OK %s maildrop has %d messages (%d octets).\n",
             user, msgcnt, bytecnt);
        break;
      case _UIDL:
        if (! (tmp = extract_argument())) msg = 0;
        else msg = atoi(tmp);
#ifdef DEBUG_POP3
        trace(report, "POP UIDL", "%d", msg);
#endif
        put_uidl(msg);
        break;
      default:
 err:   if (++badcmd > 30)
        {
          putf("-ERR Too many bad commands.\n");
          finish = OK;
          break;
        }
        putf("-ERR Unknown command.\n");
        break;
disabled:
        putf("-ERR Mailbox disabled due to maintenance. Try later again!\n");
      }
    }
    if (finish) break;
  }
  if (login) unlink(lockfile);
}

/*---------------------------------------------------------------------------*/

void pop3::show_popdb (void)
//*************************************************************************
//
//*************************************************************************
{
  char peeraddr[32];
  char name[32];
  strcpy(peeraddr, "");
  time_t lastpop;
  int kopf = FALSE;

  if ((fi = s_fopen(POPDBNAME, "lrt")) != NULL)
  {
    while (! feof(fi))
    {
      fscanf(fi, "%31s\t%08lX\t%31s\n", peeraddr, &lastpop, name);
      if (! kopf)
      {
        kopf = TRUE;
        putf("Peer IP         Call     Time\n");
      }
      putf("%-15.15s %-8.8s %s\n", peeraddr, name,
                                   datestr(lastpop - ad_timezone(), 13));
    }
    s_fclose(fi);
  }
  if (! kopf)
    putf(ms(m_noentryfound));
}

/*---------------------------------------------------------------------------*/

void mblogin_pop3 (char *name)
//*************************************************************************
//
//*************************************************************************
{
  pop3 p;

  strcpy(b->uplink, "POP3");
  safe_strcpy(b->peerip, name);
  *b->logincall = 0;
  b->headermode = _POP;
  p.start_pop3();
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/
