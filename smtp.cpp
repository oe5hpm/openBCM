/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------
  Sendmail for BCM   Reference: RFC 821
  ----------------


  Copyright (C)       Deti Fliegl, deti@lrz.de, DG9MHZ
                      Guardinistr. 47
                      81375 Muenchen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980121 DG9MHZ Struktur msg wurde nicht korrekt geloescht,
//                im Subj war gelegentlich Gammel hinten dran (fixed)
//19980126 OE3DZW fixed Subj again, Struktur
//19980505 DG9MHZ smtp only after pop3
//19980506 OE3DZW changes to pop3db
//19980514 OE3DZW changed smtp-message, string was wrong
//19990818 OE3DZW user may disable pop before smtp
//20030101 hpk    in the CB-BCMNET login-concept: user has to pop before smtp,
//                otherwise message is sent in guestmode
//20030517 DH6BB  Einbau Mime2Bin fuer SMTP mit Anhang
//20031112 DH6BB  Subject mit Umlauten nach RFC2047 werden richtig gewandelt


#include "baycom.h"
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


#define hexval(c) Index_hex[(unsigned int)(c)]
#define base64val(c) Index_64[(unsigned int)(c)]

static int Index_hex[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
     0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

static int Index_64[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};



/*---------------------------------------------------------------------------*/

typedef struct
{
    char *rcpt[MAX_RCPTCNT];
    char from[65];
    char subject[256];
    int rcptcnt;
} msg_t;

class smtp
{
public:
  void start_smtp (char *name);
  void onkill (void);

private:
  int popped_before ();
  int send_msg (void);
//  char *extract_call (char *line, int from);
  void rset (void);
  void charset (char *s);
  FILE *f;
  char tmpname[40];
  msg_t msg;
  int finish;
  int badcmd;
#ifdef INETMAILGATE
  int isinternetmail;
#endif
};

/*---------------------------------------------------------------------------*/

int smtp::popped_before (void)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("smtp_popped_before");
  char peeraddr[32];
  char name[32];
  time_t lastpop;

  strcpy(peeraddr, "");
  char *s = extract_call(msg.from, 1);
  int yes = FALSE;

  if (m.nopopsmtp)
    return TRUE; //dh8ymb
  if (get_smtp_security(s) == 1)
    return TRUE; //dzw
  if (! (f = s_fopen(POPDBNAME, "srt")))
    return FALSE;
  while (fscanf(f, "%31s\t%08lX\t%31s", peeraddr, &lastpop, name) == 3)
  {
#ifdef DEBUG_SMTP
    trace(report, "smtppopbef", "1: %s %s %s %lX %lX",
          peeraddr, name, s, ad_time(), lastpop);
#endif
    if (! strcmp(peeraddr, b->peerip))
    {
#ifdef DEBUG_SMTP
      trace(report, "smtppopbef", "2: %s %s %s %lX %lX",
            peeraddr, name, s, ad_time(), lastpop);
#endif
      if (*s && ! stricmp(s, name) && ((ad_time() - lastpop) < 300) )
                               //dh8ymb, 18.05.04: nur 5 Minuten, war < HOUR
      {
#ifdef DEBUG_SMTP
        trace(report, "smtppopbef", "3: %s %s", peeraddr,name);
#endif
        yes = TRUE;
        b->pwok = OK;
        break;
      }
    }
  }
  s_fclose(f);
  return yes;
}

/*---------------------------------------------------------------------------*/

char *extract_call (char *line, int from)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("extract_call");
  static char backup[65];
  char *tc;

#ifdef DEBUG_SMTP
  trace(report, "smtpextractcall", "%s", line);
#endif
  safe_strcpy(backup, line);
  char *s = strchr(backup, '<');
  if (! s) return "";
  s++;
  if ((tc = strchr(s, '>')) != NULL) *tc = 0;
  else return "";
  tc = strchr(s, '@'); //eg. oe3dzw@oe3xsr>
  if (! tc) return s;
  if (from == 1)
  {
    *tc = 0;
    return s;
  }
  subst1(tc + 1, '.', 0); // eg. oe3xsr.xx.xx -> oe3xsr
  if (from == 2)
  {
    subst1(s + 1, '@', 0); // eg. oe3dzw@oe3xsr -> oe3dzw
  }
  return s;
}

/*---------------------------------------------------------------------------*/

int smtp::send_msg (void)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("smtp_send_msg");
  char login[32];
  char *s;
  char my_tmpname[40];
  int i;
  int j = 0;
  int a = 0;
  int attachments;
  char subject[256];
  char work_pfad[200];

  attachments = smtp_convert(tmpname);
  io_t tty = t->output;
#ifdef INETMAILGATE
  if (isinternetmail)
  {
    s = m.internetmailgate;
  }
  else
  {
    s = extract_call(msg.from, 1);
  }
#else
  s = extract_call(msg.from, 1);
#endif
  if (! *s) return 0;
  safe_strcpy(login, s);
  sprintf(work_pfad, "%s", tmpname);
  subst1(work_pfad + 1, '.', 0);
  strlwr(work_pfad);
  t->output = io_dummy;
  mblogin(login, login_silent, "SMTP");
  for (i = 0; i < msg.rcptcnt; i++)
  {
#ifdef INETMAILGATE
    if (isinternetmail)
    {
      s = extract_call(msg.rcpt[i], 2);
    }
    else
    {
      s = extract_call(msg.rcpt[i], 0);
    }
#else
    s = extract_call(msg.rcpt[i], 0);
#endif
    trace(report, "smtpgate", "%s -> %s", msg.from, s);
    if (*s)
    {
      for (a = 0; a <= attachments; a++)
      {
        if (attachments > 1)
        {
          if (a == 0)
            sprintf(subject, "%s", msg.subject);
          else
            sprintf(subject, "(File %d/%d) %s", a, attachments, msg.subject);
        }
        else
          sprintf(subject, "%s", msg.subject);
        b->oldinput = (char) t->input;
        if (attachments >= 0)
        {
#ifdef __LINUX__
          sprintf(my_tmpname, "%s/imp_%d.imp", work_pfad, a);
#else
          sprintf(my_tmpname, "%s\\imp_%d.imp", work_pfad, a);
#endif
          sysimport(my_tmpname);
          if (attachments == 1) a = attachments + 1; //Ausstieg
        }
        else
          return 0;
        snprintf(b->line, BUFLEN, "%s %s", s, subject);
        b->mailtype = '?';
        b->job = j_send;
        mbsend(b->line, 1);
        j++;
        if (b->inputfile) s_fclose(b->inputfile);
        t->input = (io_t) b->oldinput;
      }
    }
  }
  while (attachments >= 0)
  {
#ifdef __LINUX__
    sprintf(my_tmpname, "%s/imp_%d.imp", work_pfad, attachments);
#else
    sprintf(my_tmpname, "%s\\imp_%d.imp", work_pfad, attachments);
#endif
    unlink(my_tmpname);
    attachments--;
  }
  rmdir(work_pfad);
  t->output = tty;
  if (j) return 1;
  else return 0;
}

/*---------------------------------------------------------------------------*/

void smtp::rset (void)
//****************************************************************************
//
//****************************************************************************
{
  for (int i = 0; i < msg.rcptcnt; i++)
    t_free(msg.rcpt[i]);
  memset(&msg, 0, sizeof(msg_t));
  //msg.from[0] = 0;
  //msg.rcptcnt = 0;
  //msg.subject[0] = 0;
}

/*---------------------------------------------------------------------------*/

void smtp::onkill (void)
//****************************************************************************
//
//****************************************************************************
{
  if (f && *tmpname)
  {
    s_fclose(f);
    unlink(tmpname);
  }
}

/*---------------------------------------------------------------------------*/

static void onkill (void *p)
//****************************************************************************
//
//****************************************************************************
{
  ((smtp *)p)->onkill();
}

/*---------------------------------------------------------------------------*/

void smtp::start_smtp (char *name)
//****************************************************************************
//
// Kommando-Interpreter SMTP
//
//****************************************************************************
{
  static char *beftab[] =
  {
    "HELO", "EHLO", "MAIL", "RCPT", "DATA", "RSET", "NOOP", "QUIT",
    NULL
  };
  enum befnum
  {
    _ERR, _HELO, _EHLO, _MAIL, _RCPT, _DATA, _RSET, _NOOP, _QUIT
  };
  *msg.from = 0;
  *msg.subject = 0;
  msg.rcptcnt = 0;
  f = NULL;
  finish = 0;
  badcmd = 0;
  *tmpname = 0;
  b->charset = 1;
  char sendercall[CALLEN+1];
#ifdef INETMAILGATE
  isinternetmail = 0;
  char newsubject[81];
#endif

  lastfunc("start_smtp");
  if (myhostname(b->line, BUFLEN - 1) == -1) return;
  rset(); //OE3DZW clear buffer
  atkill(::onkill, (void *) this);
  if (m.disable)
  {
    putf("221 Mailbox disabled due to maintenance. Try later again!\n");
    return;
  }
  putf("220 %s OpenBCM Mailbox V" VNUMMER " SMTP Server\n", b->line);
  while (! runterfahren)
  {
    getline(b->line, BUFLEN - 1, 1);
    if ((m.tcpiptrace == 4) || (m.tcpiptrace == 8))
    {
      if (! strstr(b->line, "PASS"))
        smtplog("RX", b->line);
      else
        smtplog("RX", "PASS (hidden)");
    }
    char *args = b->line;
    switch ((befnum) readcmd(beftab, &args, 0))
    {
      case _EHLO:
        if (*args)
        {
          putf("250-Hello, pleased to meet you\n");
          putf("250 8BITMIME\n");
        }
        else putf("501 EHLO requires domain address\n");
        break;
      case _HELO:
        if (*args)
        {
//        myhostname(b->line, BUFLEN - 1);
          putf("250 Hello, pleased to meet you\n");
        }
        else
          putf("501 HELO requires domain address\n");
        break;
      case _MAIL:
        if (*msg.from)
        {
          putf("503 Sender already specified\n");
          break;
        }
        safe_strcpy(msg.from, args);
        if (stristr(msg.from, "FROM:"))
        {
          char *tc = strchr(msg.from, '<');
          if (! tc) goto err;
          char *s = extract_call(tc, 1);
          // "MAIL FROM:<>" ist auch OK. Hier muessen wir einen Absender
          // selber einsetzen, aber dieser Absender will keine Return-Mail.
          // Wir nehmen einfach mal "INFO".
          if ((! *s) && (! strcmp(tc, "<>")))
            s = "INFO";
          if (*s)
          {
            if (mbcallok(s) // falls man Mails von System-Usern importieren will (DL9CU)
                || ! strcmp(s, "WWWRUN") || ! strcmp(s, "wwwrun")
                || ! strcmp(s, "ROOT")   || ! strcmp(s, "root")
                || ! strcmp(s, "GAST")   || ! strcmp(s, "gast")
                || ! strcmp(s, "NEWS")   || ! strcmp(s, "news")
                || ! strcmp(s, "INFO")
                || ! strcmp(s, "WWW")    || ! strcmp(s, "www") )
            {
              safe_strcpy(sendercall, s);
              strupr(sendercall);
#ifdef DEBUG_SMTP
              trace(report, "smtpmailfrom", "-%s-%s-%s-", sendercall, s, msg.from);
#endif
              if (popped_before())
              {
                putf("250 %s Sender ok\n", sendercall);
                break;
              }
              else
              {
                pwlog(sendercall, b->peerip, "smtp w/o pop");
                putf("500 %s has to POP before SMTP.\n", sendercall);
              }
            }
#ifdef INETMAILGATE
            else
            {
              if (strcasecmp(m.internetmailgate, "OFF"))
              { // Internet Mailgateway aktivieren
                safe_strcpy(sendercall, m.internetmailgate);
                putf("250 %s Sender ok\n", sendercall);
                isinternetmail = 1;
                break;
              }
              else putf("553 %s contains no valid callsign\n", msg.from);
            }
#else
            else putf("553 %s contains no valid callsign\n", msg.from);
#endif
          }
          else putf("553 %s contains no valid callsign\n", msg.from);
        }
        else
        err: putf("501 Syntax error in parameters scanning \"%s\"\n", msg.from);
        *msg.from = 0;
        break;
      case _RCPT:
        if (msg.rcptcnt < MAX_RCPTCNT)
        {
          char localbuf[65];
          *localbuf = 0;
          safe_strcpy(localbuf, args);
          if (stricmp(localbuf, "TO:"))
          {
            char *s = msg.rcpt[msg.rcptcnt] = (char *) t_malloc(65, "SMTP");
            if (s)
            {
              strcpy(s, localbuf);
              msg.rcptcnt++;
              putf("250 OK\n");
            }
            else goto tomany;
          }
          else
            putf("501 Syntax error in parameters scanning \"%s\"\n",
                 localbuf);
        }
        else
        tomany: putf("552 To many recipients.\n");
        break;
      case _DATA:
        if (msg.rcptcnt && *msg.from)
        {
          char *s;
          int header = 1;
#ifdef __LINUX__
          sprintf(tmpname, "%s/%s.imp", TEMPPATH, time2filename(0));
#else
          sprintf(tmpname, "%s\\%s.imp", TEMPPATH, time2filename(0));
#endif
          strlwr(tmpname);
          if (! (f = s_fopen(tmpname, "swt")))
          {
            putf("452 Insufficient system storage\n");
            break;
          }
          putf("354 Start mail input; end with <CRLF>.<CRLF>\n");
          getline(b->line, BUFLEN - 1, 1);
          if (*b->line)
            while (1)
            {
              if (! *b->line) header = 0;
              if (*b->line == '.' && ! b->line[1])
              {
                if ((m.tcpiptrace == 4) || (m.tcpiptrace == 8))
                  smtplog("RX", "<mail content received>");
                break;
              }
              if (*b->line == '.' && b->line[1])
                memmove(b->line, b->line + 1, strlen(b->line) - 1);
              if (header && (s = stristr(b->line, "subject: ")) != NULL)
              {
                strncpy(msg.subject, s + 9, 81);
                rfc2047_decode(msg.subject, msg.subject,
                                            strlen(msg.subject)+1);
#ifdef INETMAILGATE
                if (isinternetmail)
                {
                  // "FROM:<>" -> "<>"
                  char *email = strrchr(msg.from, '<')+1; // position des "<" ermitteln
                  strchr(email, '>')[0]=0; // position des ">" ermitteln
                  sprintf(newsubject, "%s - %s", email, msg.subject);
                  safe_strcpy(msg.subject, newsubject);
                }
#endif
                msg.subject[81] = 0;
              }
              fprintf(f, "%s\n", b->line);
              getline(b->line, BUFLEN - 1, 1);
              waitfor(e_ticsfull);
            }
          s_fclose(f);
          f = NULL;
          if (send_msg())
          {
            wdelay(100);
            while (sema_test(MSMODSEMA)
#ifdef DF3VI_POCSAG
                    || sema_test(POCSAGSEMA)
#endif
                   )
              wdelay(300); // wegen moeglichen Mailserv/POCSAG-Import warten
            putf("250 Message accepted for delivery\n");
          }
          else
            putf("451 Local error in processing\n");
          unlink(tmpname);
          rset();
        }
        else putf("503 Bad sequence of commands\n");
        break;
      case _RSET: rset();
      case _NOOP: putf("250 OK\n"); break;
      case _QUIT:
//      myhostname(b->line, BUFLEN - 1);
        putf("221 Server closing transmission channel\n");
        finish = 1;
        break;
      default:
        if (badcmd > 30)
        {
          putf("421 Too many bad commands; closing connection\n");
          finish = 1;
        }
        putf("500 Command unrecognized\n");
        badcmd++;
    }
    if (finish) break;
  }
  for (int i = 0; i < msg.rcptcnt; i++)
    t_free(msg.rcpt[i]);
}


/*---------------------------------------------------------------------------*/

static int rfc2047_decode_word (char *d, const char *s, size_t len)
//****************************************************************************
//
// rfc2047_decode_word und rfc2047_decode stammen aus dem "bbsgate" von
// thomas, dl9sau. Tnx fuer die Ueberlassung.
//
//****************************************************************************
{
#define ENCQUOTEDPRINTABLE             1
#define ENCBASE64                      2
  char buf[1024];
  char *p;
  char *pp;
  char *pd;
  char *tt;
  int enc = 0, filter = 0, count = 0, c1, c2, c3, c4;
  char *charset = NULL;

  lastfunc("rfc2047_decode_word");
  strcpy(buf, s);
  p = pp = buf;
  pd = d;
  while ((pp = strtok(pp, "?")) != NULL)
  {
    count++;
    waitfor(e_ticsfull);
    switch (count)
    {
      case 2:
        // ignore language specification a la RFC 2231
        if ((tt = strchr(pp, '*')))
          *tt = '\0';
        charset = pp;
        filter = 1;
        break;
      case 3:
        if (toupper(*pp) == 'Q')
          enc = ENCQUOTEDPRINTABLE;
        else if (toupper(*pp) == 'B')
          enc = ENCBASE64;
        else
          return (-1);
        break;
      case 4:
        if (enc == ENCQUOTEDPRINTABLE)
        {
          while (*pp && len > 0)
          {
            if (*pp == '_')
            {
              *pd++ = ' ';
              len--;
            }
            else if (*pp == '=')
            {
              if (pp[1] == 0 || pp[2] == 0)
                break; // something wrong
              *pd++ = (hexval(pp[1]) << 4) | hexval(pp[2]);
              len--;
              pp += 2;
            }
            else
            {
              *pd++ = *pp;
              len--;
            }
            pp++;
          }
          *pd = 0;
        }
        else if (enc == ENCBASE64)
        {
          while (*pp && len > 0)
          {
            if (pp[0] == '=' || pp[1] == 0 || pp[1] == '=')
              break; // something wrong
            c1 = base64val(pp[0]);
            c2 = base64val(pp[1]);
            *pd++ = (c1 << 2) | ((c2 >> 4) & 0x3);
            if (--len == 0) break;
            if (pp[2] == 0 || pp[2] == '=') break;
            c3 = base64val(pp[2]);
            *pd++ = ((c2 & 0xf) << 4) | ((c3 >> 2) & 0xf);
            if (--len == 0)
              break;
            if (pp[3] == 0 || pp[3] == '=')
              break;
            c4 = base64val(pp[3]);
            *pd++ = ((c3 & 0x3) << 6) | c4;
            if (--len == 0)
              break;
            pp += 4;
          }
          *pd = 0;
        }
        break;
    }
    pp = 0;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void rfc2047_decode (char *d, const char *s, size_t dlen)
//****************************************************************************
//
// try to decode anything that looks like a valid RFC2047 encoded
// header field, ignoring RFC822 parsing rules
//
//****************************************************************************
{
  lastfunc("rfc2047_decode");
  const char *p, *q;
  size_t n;
  int found_encoded = 0;

  dlen--; // save room for the terminal nul

  while (*s && dlen > 0)
  {
    if ((p = strstr(s, "=?")) == NULL ||
        (q = strchr(p + 2, '?')) == NULL ||
        (q = strchr(q + 1, '?')) == NULL ||
        (q = strstr(q + 1, "?=")) == NULL)
    {
      // no encoded words
      if (d != s)
        strncpy(d, s, dlen + 1);
      return;
    }
    if (p != s)
    {
      n = (size_t) (p - s);
      // ignore spaces between encoded words
      if (! found_encoded || strspn(s, " \t\r\n") != n)
      {
        if (n > dlen)
          n = dlen;
        if (d != s)
          memcpy(d, s, n);
        d += n;
        dlen -= n;
      }
    }
    rfc2047_decode_word(d, p, dlen);
    found_encoded = 1;
    s = q + 2;
    n = strlen(d);
    dlen -= n;
    d += n;
  }
  *d = 0;
}

/*---------------------------------------------------------------------------*/

void mblogin_smtp (char *name)
//****************************************************************************
//
// Login-Session fuer SMTP
//
//****************************************************************************
{
  smtp sm;
  strcpy(b->uplink, "SMTP");
  strcpy(b->peerip, name);
  b->headermode = _SMTP;
  *b->logincall = 0;
  sm.start_smtp(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/
