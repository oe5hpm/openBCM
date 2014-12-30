/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------
  NNTP for BCM   Reference: RFC 977, RFC 850, RFC 1036, RFC 2980
  ------------


  Copyright (C)       Deti Fliegl, deti@lrz.de, DG9MHZ
                      Guardinistr. 47
                      81375 Muenchen

  Alle Rechte vorbehalten / All Rights Reserved

***************************************************************/

//19991213 Jan    surprise .. now it works again :)
//                (clean-up, uses mbread now, added XOVER .. and security)
//20030306 dh8ymb added remove cr/lf in subject (causes sigterm)
//20030306 dh6bb  xover: Outlook-Unterstuetzung
//20030308 dh8ymb xover: Zeilen/Bytes werden nun korrekt uebergeben,
//                help: hilfe wird komplett angezeigt
//20031221 dh6bb  Commando NEWNEWS
//20040501 dh8ymb added AUTHINFO and POST


#include "baycom.h"
#ifdef NNTP

#define MAX_RCPTCNT 100

#ifdef __UNIX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#ifdef _WIN32
#include "winsock.h"
#endif

/*---------------------------------------------------------------------------*/

static char not_supp[] = "%d not supported\n";

class nntp
{
public:
  void start_nntp (char *name);
private:
  char *extract_argument ();
  void list_groups (void);
  void cmd_date (void);
  void cmd_post (void);
  bool set_group (char *s, bool);
  void put_head (int mode, char *s);
  void cmd_xover (char *s);
  void cmd_newnews (void);
  void cmd_newgroups (void);

  FILE *f;
  char *tmp;
  char *args;
  char group[30];
  int finish;
  int autho;
  int badcmd;
  int currentmsg;
  int msgcnt;
};

/*---------------------------------------------------------------------------*/

void nntp::list_groups (void)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("nntp_list_groups");

  unsigned int i;
  handle fh;
  char *group_;
  char boxcall[CALLEN+1];

  putf("215 list of newsgroups follows\n");
  for (i = 0; i < treelen; i++)
  {
    if (strlen(tree[i].name) != 1)
    {
      fh = preparedir(tree[i].name, 0);
      msgcnt = 0;
      if (fh != EOF)
      {
        msgcnt = lseek(fh, 0L, SEEK_END) >> 7;
        s_close(fh);
      }
      group_ = strdup(tree[i].name);
      strlwr(group_);
      subst(group_, '/', '.');
      if (m.callformat > 0)
      {
        sprintf(boxcall, "%s", m.boxname);
        strlwr(boxcall);
        putf("%s.bbs.%s %d 1 y\n", boxcall, group_, msgcnt);
      }
      else
        putf("ampr.bbs.%s %d 1 y\n", group_, msgcnt);
          // "ampr.bbs.test 2 1 n" => posting not allowed
          // "ampr.bbs.test 2 1 y" => posting allowed
#ifdef DEBUG_NNTP
      trace(report, "list_groups", "%s %d", group_, msgcnt);
#endif
      free(group_);
    }
  }
  putf(".\n");
  msgcnt = 0;
}

/*---------------------------------------------------------------------------*/

bool nntp::set_group (char *s, bool silent)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("nntp_set_group");

  bool vorhanden=false;
  char mp[80];

  strcpy(mp, "411 Invalid group name.\n");
  if (! s || ! *s) putf(mp);
  else
  {
    handle fh;
    char *tmp_ = strdup(s);
    strupr(tmp_);
    subst(tmp_, '.', '/');
    if (! finddir(tmp_, b->sysop)) putf(mp);
    else
    {
      fh = preparedir(b->boardfullname, 0);
      msgcnt = 0;
      if (fh != EOF)
      {
        msgcnt = lseek(fh, 0L, SEEK_END) >> 7;
        s_close(fh);
      }
      if (! silent)
        putf("211 %d 1 %d %s\n", msgcnt, msgcnt, s);
#ifdef DEBUG_NNTP
      trace(report, "set_group", "%s %d", s, msgcnt);
#endif
      safe_strcpy(group, s);
      if (msgcnt) currentmsg = 1;
      else currentmsg = 0;
        vorhanden=true;
    }
    free(tmp_);
  }
  return vorhanden;
}

/*---------------------------------------------------------------------------*/

void nntp::put_head (int mode, char *s)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("nntp_put_head");

  char *cp;
  char *cq = NULL;
  char *msg_ = NULL;
  int msgnum;

  if (! s || ! *s)
  {
    msgnum = currentmsg;
    cp = NULL;
  }
  else
  {
    cp = strchr(s, '<');
    cq = cp;
    msgnum = atoi(s);
  }
  if (cp && strchr(cp, '>') && (cq = strchr(cp, '@')))
  {
    *cq = 0;
    msg_ = bidvorhanden(cp + 1);
    if (! msg_) goto inv;
    setfullpath(msg_);
    handle fh = preparedir(b->boardfullname, 0);
    if (fh != EOF)
    {
      char ok;
      seek_fname(fh, strchr (msg_, '/') + 1, &ok, 0);

      if (ok)
        //msgnum=(tell(fh)/128)+1;
        msgnum = (lseek(fh, 0L, SEEK_CUR) >> 7) + 1;
      else msgnum = 0;
      s_close(fh);
    }
  }
  else if (! *group)
  {
    putf("412 no newsgroup has been selected\n");
    return;
  }
  else
    if (msgnum)
    {
      if (msgnum < 0 || (msgnum > msgcnt)) goto inv;
      handle fh = preparedir(b->boardfullname, 0);
      if (fh != EOF)
      {
        lseek(fh, (long) (msgnum - 1) << 7, SEEK_SET);
        _read(fh, b->line, BLEN);
        dirline_data(b->line);
        safe_strcpy(b->mailfname, b->line);
        strcpy(b->mailpath, b->mask);
        strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
        s_close(fh);
        currentmsg = msgnum;
      }
    }
    else
    {
      inv: putf("430 no such article found\n");
      return;
    }
  switch (mode)
  {
    case 0:
      putf("223 %d <%s@%s> article retrieved - request text separately\n",
           msgnum, b->bid, m.boxname);
      return;
    case 1:
      putf("221 %d <%s@%s> article retrieved - head follows\n",
           msgnum, b->bid, m.boxname);
      break;
    case 2:
      putf("222 %d <%s@%s> article retrieved - body follows\n",
           msgnum, b->bid, m.boxname);
      break;
    case 3:
      putf("220 %d <%s@%s> article retrieved - head and body follows\n",
           msgnum, b->bid, m.boxname);
      break;
  }
  //setfullpath (cp + 1);
  b->job = j_read;
  b->optgroup = o_ir;
  b->optminus = (unsigned) -1;
  b->optplus = 0;
  //nntp_read_file();
  char tmp2[100];
  sprintf(tmp2, "%s %d", b->boardfullname, msgnum);
  if (mode == 1) mbread(tmp2, 1); //header
  if (mode == 2) mbread(tmp2, 2); //body
  if (mode == 3) mbread(tmp2, 0); //head+body
  putf("\n.\n");
}

/*---------------------------------------------------------------------------*/

static void putmimeaddress (char *s, char *cp)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("nntp_putmimeaddress");

  strlwr(s);
  if (cp) strlwr(cp);
  if (*s && cp && *cp) putf("<%s@%s>", s, cp);
  else putf("<%s>", s);
}

/*---------------------------------------------------------------------------*/

char *nntp::extract_argument ()
//****************************************************************************
//
//****************************************************************************
{
  skip(args);
  if (*args)
    return args;
  else
    return NULL;
}

/*---------------------------------------------------------------------------*/

void nntp::cmd_xover (char *s)
//****************************************************************************
//
// Abarbeiten von XOVER
//
//****************************************************************************
{
  lastfunc("nntp_cmd_xover");

  int num1 = 0;
  int num2 = 0;
  int i;
  char *x = (s ? strchr(s, '-') : (char *) NULL);
  char st[MYBBSLEN+1];
  char xref[LINELEN+1];

  if (! *group)
  {
    putf("412 no newsgroup has been selected\n");
    return;
  }
  putf("224 data follows\n");
  if (! x) num1 = num2 = (s ? atoi(s) : currentmsg);
  else
  {
    *x = 0;
    num1 = atoi(s);
    num2 = atoi(x + 1);
  }
  handle fh;
  if ((fh = preparedir(b->boardfullname, 0)) != EOF)
  {
    snprintf(xref, sizeof(xref), "%s:%d %s", get_fqdn(), m.nntp_port, group);
    for (i = num1; i <= num2; i++)
    {
      if (   lseek(fh, (long) (i - 1) << 7, SEEK_SET) == -1
          || _read(fh, b->line, BLEN) < 1)
        break;
      dirline_data(b->line);
      if (b->geloescht) continue;
      b->line[7] = 0;
      *st = 0;
      get_mybbs(b->herkunft, st, 0);
      rm_crlf(b->betreff); //dh8ymb: moegl. CR/LF entfernen, sonst Absturz
      cut_blanktab(b->betreff); //   ebenso Blanks/Tabs am Ende entfernen
            //num subj
      putf("%d\t%s\t", i, b->betreff);
                      //from
      putmimeaddress(b->herkunft, st);
            //time   id   refs bytes lines xref
      putf("\t%s\t<%s@%s>\t%s\t%d\t%d\tXref: %s:%d\n",
            datestr(filename2time(b->line) - ad_timezone(), 13),
            b->bid, m.boxname, m.boxname, b->bytes, b->lines, xref, i);
            //time is incorrect, should be sent-time
    }
    s_close(fh);
  }
  putf(".\n");
}

/*---------------------------------------------------------------------------*/

void nntp::cmd_newnews (void)
//****************************************************************************
//
// Abarbeiten von NEWNEWS
// Wir verarbeiten nur 1 Group und verstehen (noch) kein [GMT] oder
// [distribution]
//
//****************************************************************************
{
  lastfunc("nntp_cmd_newnews");

  handle fh;
  unsigned int anz;
  char worte[MAXPARSE];
  char s1[80];
  char *Board;
  char *Datum;
  char *Zeit;
  struct tm tt;
  memset(&tt, 0, sizeof(struct tm));

  strcpy(s1, args);
  anz = parseline(s1, worte);
  if (anz != 3)
  {
    putf("599 wrong amount of arguments\n");
    return;
  }
  Board = (s1+worte[0]);
  Datum = (s1+worte[1]);
  Zeit = (s1+worte[2]);
  if (! set_group(Board, 1)) return; //Wenn Board nicht vorhanden, dann raus
  tt.tm_isdst = -1;
  tt.tm_mday = atoi(Datum + 4); Datum[4] = 0;
  tt.tm_mon  = atoi(Datum + 2)-1; Datum[2] = 0;
  tt.tm_year = atoi(Datum);
  tt.tm_sec  = atoi(Zeit + 4); Zeit[4] = 0;
  tt.tm_min  = atoi(Zeit + 2); Zeit[2] = 0;
  tt.tm_hour = atoi(Zeit);
  if (   tt.tm_mday < 1 || tt.tm_mday > 31
      || tt.tm_mon < 0  || tt.tm_mon > 11)
  {
    putf(ms(m_date_invalid));
    return;
  }
  if (tt.tm_year < 70) tt.tm_year += 100; //y2k
  putf("230 list of new articles by message-id follows\n");
  if ((fh = preparedir(b->boardfullname, 0)) != EOF)
  {
    for (int i = 1; i <= msgcnt; i++)
    {
      if (   lseek(fh, (long) (i - 1) << 7, SEEK_SET) == -1
          || _read(fh, b->line, BLEN) < 1)
      break;
      dirline_data(b->line);
      if (b->geloescht)
            continue;
      b->line[7] = 0;
// DH6BB 21.12.02
// Hier muss nochmal geprueft werden, wie das genau mit den Zeiten ist....
      if (  (ad_mktime(&tt) - ad_timezone())
          > (filename2time(b->line) - ad_timezone()) )
        continue; // Datei zu alt
      putf("<%s@%s>\n", b->bid, m.boxname);
    }
    s_close(fh);
  }
  putf(".\n");
}

/*---------------------------------------------------------------------------*/

void nntp::cmd_newgroups (void)
//****************************************************************************
//
// Abarbeiten von NEWGROUPS
// Es wird nur eine leere Liste zu Kompatibilitaetszwecken ausgegeben
//
//****************************************************************************
{
  lastfunc("nntp_cmd_newgroups");

  putf("231 New newsgroups follow.\n");
  putf(".\n");
}

/*---------------------------------------------------------------------------*/

void nntp::cmd_date (void)
//****************************************************************************
//
// Abarbeiten von DATE
//
//****************************************************************************
{
  lastfunc("nntp_cmd_date");

  putf("111 %s\n", datestr(ad_time(), 6));
}

/*---------------------------------------------------------------------------*/

void nntp::cmd_post (void)
//****************************************************************************
//
// Abarbeiten von POST
// Posten einer Rubrikmail per NNTP (ueber Mailimport)
//
//****************************************************************************
{
  lastfunc("nntp_cmd_post");

  char to[LINELEN+1];
  char subj[LINELEN+1];
  char cmd[200];
  char tmpname[30];
 // FILE *f;
  unsigned int anz;
  char worte[MAXPARSE];
  int header;
//  int attachments;

  putf("340 send article to be posted. End with <CR-LF>.<CR-LF>\n");
  b->line[0] = 0;
  header = true;
  sprintf(tmpname, "%s/%s.imp", TEMPPATH, time2filename(0));
  strlwr(tmpname);
  if ((f = s_fopen(tmpname, "swt")) != NULL)
  {
    s_fsetopt(f, 1);
    while (b->line[0] != '.')
    {
      getline(b->line, BUFLEN - 1, 1);
      if (header) // Header auswerten, sonstige Zeilen verwerfen
      {
        if (strpos(b->line, "Subject:") == 0)    // Nach Titel suchen
        {
          sprintf(subj, "%s", b->line + 8); // "Subject:" herausschneiden
        }
        if (strpos(b->line, "Newsgroups:") == 0) // Nach Rubrik suchen
        {
          sprintf(to, "%s", b->line + 11); // "Newsgroups:" herausschneiden
          subst (to, '.', ' ');        // replace "." durch Space
          anz = parseline(to, worte);  // aufdroeseln...
          strcpy(to, to+worte[anz-1]); // letztes Wort ist Rubrik
        }
        if (! isalpha(b->line[0])) // zwischen Header und Mail steht ein LF
          header = false;
      }
      else // nur tatsaechlichen Mailinhalt abspeichern
      {
        if (b->line[0] == '.' && strlen(b->line) == 1)
          break;
        else
          fprintf(f, "%s\n", b->line);
      }
    }
    fprintf(f, "\nnnnn\nend\n");
    s_fclose(f);
    putf("240 article posted ok\n");
//  attachments=nntp_convert(tmpname); todo uudecode for binary attachments
    b->oldoutput = t->output;
    t->output = io_dummy;
    sysimport(tmpname);
    sprintf(cmd, "s %s %s", to, subj);
    mailbef(cmd, 0);
    t->output = (io_t) b->oldoutput;
#ifdef DEBUG_NNTP
    trace(report, "nntp", "debug: mail sent %s", to);
#endif
    unlink(tmpname);
  }
  else
    putf("441 posting failed\n");
}

/*---------------------------------------------------------------------------*/

void nntp::start_nntp (char *name)
//****************************************************************************
//
// Hier werden die NNTP-Befehle ausgewertet und abgearbeitet
//
//****************************************************************************
{
  lastfunc("nntp_start_nntp");

  static char *beftab[] =
  {
    "AUTHINFO", "ARTICLE", "BODY", "CHECK", "DATE", "GROUP", "HEAD", "HELP",
    "IHAVE", "LAST", "LIST", "LISTGROUP", "MODE", "NEWGROUPS", "NEWNEWS", "NEXT",
    "POST", "QUIT", "SLAVE", "STAT", "XOVER",
    NULL
  };

  enum befnum
  {
    _ERR, _AUTHINFO, _ARTICLE, _BODY, _CHECK, _DATE, _GROUP, _HEAD, _HELP,
    _IHAVE, _LAST, _LIST, _LISTGROUP, _MODE, _NEWGROUPS, _NEWNEWS, _NEXT,
    _POST, _QUIT, _SLAVE, _STAT, _XOVER
  };
  unsigned int anz;
  char worte[MAXPARSE];
  char s1[80];
  char *s2;
  char *pw;

  finish = 0;
  autho  = 0;
  badcmd = 0;
  currentmsg = 0;
  msgcnt = 0;
  *group = 0;
  b->charset = 1; //get_ttycharset..
  if (m.disable)
  {
    putf("500 Mailbox disabled due to maintenance. Try later again!\n");
    return;
  }
  if (myhostname(b->line, BUFLEN - 1) == -1) return;
  putf("200 %s NNTP[auth] Server OpenBCM V" VNUMMER "\n", b->line);
  while (! runterfahren)
  {
    getline(b->line, BUFLEN - 1, 1);

  lastfunc("start_nntp2");

    if ((m.tcpiptrace == 2) || (m.tcpiptrace == 8))
    {
      if (! strstr(b->line, "PASS"))
        nntplog("RX", b->line);
      else
        nntplog("RX", "AUTHINFO PASS (hidden)");
    }
    args = b->line;

  lastfunc("start_nntp3");

    switch ((befnum) readcmd(beftab, &args, 0))
    {
      case _LAST:
          if (currentmsg > 2)
          {
            currentmsg--;
            put_head(0, "");
          }
          else
            putf("422 no previous article in this group\n");
        break;
      case _NEXT:
          if (currentmsg < msgcnt)
          {
            currentmsg++;
            put_head(0, "");
          }
          else
            putf("421 no next article in this group\n");
        break;
      case _STAT:
          put_head(0, extract_argument());
        break;
      case _HEAD:
          put_head(1, extract_argument());
        break;
      case _BODY:
          put_head(2, extract_argument());
        break;
      case _AUTHINFO:
          if (*args)
          {
            strcpy(s1, args);
            anz = parseline(s1, worte);
            if (! strcasecmp((s1 + worte[0]), "SIMPLE"))
              putf("501 Command not supported.\n");
            if (! strcasecmp((s1 + worte[0]), "GENERIC"))
              putf("501 Command not supported.\n");
            if (! strcasecmp((s1 + worte[0]), "USER"))
            {
              s2 = (s1 + worte[1]);
              if (*s2)
              {
                strupr(s2);
                if (mbcallok(s2))
                {
                  strcpy(b->logincall, s2);
                  loaduser(b->logincall, u, 1);
                  putf("381 PASS required\n");
                }
                else
                {
                  putf("482 Authentication rejected (user not valid)\n");
                  break;
                }
                getline(b->line, BUFLEN - 1, 1);
                if ((m.tcpiptrace == 2) || (m.tcpiptrace == 8))
                {
                  if (! strstr(b->line, "PASS"))
                    nntplog("RX", b->line);
                  else
                    nntplog("RX", "AUTHINFO PASS (hidden)");
                }
                strcpy(s1, b->line);
                anz = parseline(s1, worte);
                if (! strcasecmp((s1 + worte[1]), "PASS"))
                {
                  pw = s2 = (s1 + worte[2]);
                  if (*s2)
                  {
                    if (isamprnet(b->peerip) > 0)
                      get_httppw(b->logincall, s2);
                    else
                      get_ttypw(b->logincall, s2);
                    if (! strcmp(s2, pw))
                    {
                      putf("281 Authentication accepted.\n");
                      if (! autho) //bei Doppelaufruf kein neuer Login
                        mblogin(b->logincall, login_silent, "NNTP");
                      autho = 1;
                    }
                    else
                      putf("482 Authentication rejected (password not valid)\n");
                  }
                  else
                    putf("482 Authentication rejected (no password)\n");
                }
                else
                  putf("502 No permission (no PASS argument)\n");
              }
              else
                putf("502 No permission (no USER argument)\n");
            }
            else
              putf("502 No permission (no argument)\n");
          }
          else
            putf("502 No permission (no authinfo argument)\n");
        break;
      case _CHECK:
          putf(not_supp, 501);
        break;
      case _ARTICLE:
  lastfunc("start_nntp31");
          put_head(3, extract_argument());
  lastfunc("start_nntp32");
        break;
      case _XOVER:
          cmd_xover(extract_argument());
        break;
      case _GROUP:
          set_group(extract_argument(), 0);
        break;
      case _LIST:
          tmp = extract_argument();
          if (! tmp || ! strcasecmp(tmp, "newsgroups")) //Win lacks strcasecmp
          //Lynx does "LIST NEWSGROUPS"
            list_groups();
          else
            putf(not_supp, 501);
        break;
      case _LISTGROUP:
          putf(not_supp, 501);
        break;
      case _HELP:
          putf("100 OpenBCM V" VNUMMER " NNTP Server - Legal commands\n");
          putf("  authinfo user Name|pass Password\n");
          putf("  article [MessageID|Number]\n");
          putf("  body [MessageID|Number]\n");
          putf("  *check MessageID\n");
          putf("  date\n");
          putf("  group newsgroup\n");
          putf("  head [MessageID|Number]\n");
          putf("  help\n");
          putf("  *ihave\n");
          putf("  last\n");
          putf("  list [active|active.times|newsgroups|subscriptions]\n");
          putf("  *listgroup newsgroup\n");
          putf("  mode stream\n");
          putf("  mode reader\n");
          putf("  *newgroups yymmdd hhmmss [GMT] [<distributions>]\n");
          putf("  newnews newsgroups yymmdd hhmmss [GMT] [<distributions>]\n");
          putf("  next\n");
          putf("  post\n");
          putf("  quit\n");
          putf("  *slave\n");
          putf("  stat [MessageID|Number]\n");
          putf("  xover [range]\n");
          putf("  * means not implemented at the moment\n");
          putf(".\n");
        break;
      case _QUIT:
          finish = 1;
          putf("205 OpenBCM NNTP server signing off. Goodbye.\n");
        break;
      case _POST:
          if (!autho)
            putf("480 Authorization required\n");
          else
            cmd_post();
        break;
      case _NEWGROUPS:
          cmd_newgroups();
        break;
      case _IHAVE:
          putf(not_supp, 501);
        break;
      case _NEWNEWS:
          cmd_newnews();
        break;
      case _SLAVE:
          putf(not_supp, 202);
        break;
      case _DATE:
          cmd_date();
        break;
      case _MODE:
        if (! (tmp = extract_argument()) || stricmp(tmp, "READER"))
          putf(not_supp, 501);
        else
        {
          if (! autho)
            putf("480 Authorization required\n");
          else
            putf("200 ok\n");
        }
        break;
      default:
        if (badcmd > 30)
        {
          putf("205 Too many bad commands.\n");
          finish = 1;
        }
        putf("500 Unknown command.\n");
        badcmd++;
    }

  lastfunc("start_nntp4");

    if (finish) break;
  }
}

/*---------------------------------------------------------------------------*/

void mblogin_nntp (char *name)
//****************************************************************************
//
// Login-Session fuer NNTP
//
//****************************************************************************
{
  nntp p;

  strcpy(b->uplink, "NNTP");
  strcpy(b->peerip, name);
#ifdef _GUEST
  if (strcmp("OFF", m.guestcall))
    strcpy(b->logincall, m.guestcall);
  else
#endif
    strcpy(b->logincall, m.boxname);
  b->headermode = _NNTP;
  timeout(10); // Timeout auf 10 Minuten
  p.start_nntp(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/
#endif
