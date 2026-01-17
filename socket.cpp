/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------
  Interface zur Socket-Ansteuerung
  --------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980124 DG9MHZ/OE3DZW send()->-1 means buffer full -> no err
//19980211 OE3DZW added option for SERVIF
//19980223 DG2FEF/OE3DZW fixed send() again, hope this helps
//19980301 OE3DZW removed suicide, therefore changed trace to "fatal"
//19980310 DG9MHZ fixed EAGAIN,EINTR
//19980311 OE3DZW more nice err-msges
//19980318 OE3DZW fixed peerips - trailing 0x00 was missing
//19980323 DG9MHZ fixed Win32-version
//19980418 OE3DZW added show_sockets
//19980506 OE3DZW update show_sokets
//19980609 OE3DZW removed "feature" for db0rgb
//19980831 OE3DZW addrlen is signed again (was unsigned in bcm140b)
//19980905 OE3DZW fix from 19980301 was no good idea, removed
//19980914 OE3DZW added unsigned addrlen for glibc, from bcm1.40c (dh3mb)
//19990215 OE3DZW default rhosts.bcm will be generated; comments allowed (;)
//19990220 DK2UI  added define for tcp/ip timeout
//19991028 Jan    holsock binary transparent, putv/getv a bit RFC854 compliant
//19991028 Jan    added telnet_fwd_connect()
//20040101 DH8YMB Telnet-Forward-Timeout hinzu
//20040102 DH6BB  added ftp
//20040103 DH8YMB Telnet-Forward NON-Blocking Mode fuer Windows

#include "baycom.h"

#ifdef __FLAT__

#define TIMEOUT 10 // 29.12.06 dh8ymb: war 120 Minuten, nun 10 Minuten

/*---------------------------------------------------------------------------*/

#ifdef __UNIX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#ifdef _WIN32
#include "winsock.h"
#define EINPROGRESS WSAEINPROGRESS
#undef errno
#define errno h_errno
WORD    wVersionRequested;
WSADATA wsaData;
u_long  blockmode = FIONBIO;
        // if blockmode = 0, blocking is enabled;
        // if blockmode != 0, non-blocking mode is enabled.
#endif

/*---------------------------------------------------------------------------*/

#define MAXSOCK 256
#define TXLEN 512
#define RXLEN 512

/*---------------------------------------------------------------------------*/

typedef struct sock_t
{
  char rxbuf[RXLEN];
  char txbuf[TXLEN];
  int handle;
  int rxc;
  int txc;
  int rxlen;
  int wasput;
  int wasda;
  char dasda;
  time_t lastflush;
  time_t tcreate; //time when socket was created
}
sock_t;

static sock_t so[MAXSOCK];

/*---------------------------------------------------------------------------*/

static char *in_ntoa (unsigned long in)
//*************************************************************************
//
//  Display an IP address in readable format
//  (from Linux-Kernel net/utils.c)
//
//*************************************************************************
{
  static char buff[18];
  char *p = (char *) &in;
  sprintf(buff, "%d.%d.%d.%d",
                 (p[0] & 255), (p[1] & 255), (p[2] & 255), (p[3] & 255));
  return(buff);
}

/*---------------------------------------------------------------------------*/

char *get_fqdn ()
//*************************************************************************
//
//
//
//*************************************************************************
{
  char temp[LINELEN+1];
  struct hostent *name;

  if (gethostname(temp, LINELEN) == NIL)
    trace(serious, "get_fqdn", "gethostname NIL - DNS disabled?");
  temp[LINELEN] = 0;
  name = gethostbyname(temp);
  if (name)
    return name->h_name;
  else
    return NULL;
}

/*---------------------------------------------------------------------------*/

int myhostname (char *s, int len)
//*************************************************************************
//
//
//
//*************************************************************************
{
  if (gethostname(s, len - 2) == NIL)
  {
    trace(serious, "myhostname", "gethostname NIL - DNS disabled?");
    return NIL;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

int find_free_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  for (unsigned i = 1; i < MAXSOCK; i++)
    if (so[i].handle == EOF)
      return i;
  return NIL;
}

/*---------------------------------------------------------------------------*/

void show_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  int head = FALSE;
  for (unsigned i = 1; i < MAXSOCK; i++)
    if (so[i].handle != EOF)
    {
      if (! head)
      {
        putf("Nr ha   rxc txc created\n");
        head = TRUE;
      }
      putf("%2d %.4d %3d %3d %s\n", i, so[i].handle, so[i].rxc, so[i].txc,
                               datestr(so[i].tcreate - ad_timezone(), 13));
    }
  if (! head)
    putf("No sockets active.\n");
}

/*---------------------------------------------------------------------------*/

int putfree_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  return OK;
}

/*---------------------------------------------------------------------------*/

void putv_sock (int c)
//*************************************************************************
//
//
//
//*************************************************************************
{
  sock_t *s = &so[t->socket];
  if (t->processkind & P_TELNET && (char) c == 0xFF)
  {  //0xFF (IAC) - send it twice (RFC854)
    s->txbuf[s->txc++] = (char) c;
    s->wasput = 1;
    if (s->txc >= TXLEN)
      putflush_sock();
  }
  s->txbuf[s->txc++] = (char) c;
  s->wasput = 1;
  if (s->txc >= TXLEN)
    putflush_sock();
}

/*---------------------------------------------------------------------------*/

void putflush_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  lastfunc("putflush_sock");
  sock_t *s = &so[t->socket];
  int ok = 0;
  int sum = 0;

  if (s->txc)
  {
    while (1)
    {
      fd_set wfds;
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      FD_ZERO(&wfds);
      FD_SET(s->handle, &wfds);
      if (select(s->handle + 1, NULL, &wfds, NULL, &tv))
      {
        ok = send(s->handle, s->txbuf + sum, s->txc - sum, 0);
        if (ok < 0)
        {
#ifdef _WIN32
         if (errno && (errno != EWOULDBLOCK))
#endif
#ifdef __LINUX__
         if (errno != EAGAIN && errno != EINTR)
#endif
         {
           trace(report, "putflush_sock", "kill %d: %s",
                                          errno, strerror(errno));
           suicide();
         }
        }
        else sum += ok;
        if (sum == s->txc) break;
      }
      wdelay(11);
    }
    s->txc = 0;
  }
}

/*---------------------------------------------------------------------------*/

static int holsock (sock_t *s)
//*************************************************************************
//
//
//
//*************************************************************************
{
  int sret;
  fd_set rfds;
  struct timeval tv;
  lastfunc("holsock");

  if (s->rxc >= s->rxlen)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(s->handle, &rfds);
  lastfunc("holsock1");
    if (! (sret = select(s->handle + 1, &rfds, NULL, NULL, &tv)))
      return EOF;
        //if ((s->rxlen = read(s->handle, s->rxbuf, RXLEN)) < 1)
  lastfunc("holsock2");
    if ((s->rxlen = recv(s->handle, s->rxbuf, RXLEN, 0)) < 1)
    {
  lastfunc("holsock3");
      if (! s->rxlen && sret) suicide();
  lastfunc("holsock4");
      if ((
#ifdef __LINUX__
            s->rxlen == EOF ||
#endif
            errno == ENOTCONN || errno == EBADF)
#ifdef __LINUX__
          && errno != EAGAIN
#endif
         )
      {
  lastfunc("holsock5");
        trace(report, "holsock", "kill: %d %s", errno, strerror(errno));
        suicide();
      }
  lastfunc("holsock6");
      return EOF;
    }
  lastfunc("holsock7");
    s->rxc = 0;
  }
  lastfunc("holsock8");
  return s->rxbuf[s->rxc++];
}

/*---------------------------------------------------------------------------*/

static int holsock_ftp (sock_t *s)
//*************************************************************************
//
// Speziell fuer FTP. Kein Abbruch wenn Socket nicht mehr vorhanden!
//
//*************************************************************************
{
  int sret;
  fd_set rfds;
  struct timeval tv;
  lastfunc("holsock_ftp");

  if (s->rxc >= s->rxlen)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(s->handle, &rfds);
    if (! (sret = select(s->handle + 1, &rfds, NULL, NULL, &tv)))
      return EOF;
    if ((s->rxlen = recv(s->handle, s->rxbuf, RXLEN, 0)) < 1)
    {
      if (! s->rxlen && sret) return 256;
      if ((
#ifdef __LINUX__
            s->rxlen == EOF ||
#endif
            errno == ENOTCONN || errno == EBADF)
#ifdef __LINUX__
          && errno != EAGAIN
#endif
         )
      {
        trace(report, "holsock_ftp", "kill: %d %s", errno, strerror(errno));
        suicide();
      }
      return EOF;
    }
    s->rxc = 0;
  }
  return s->rxbuf[s->rxc++];
}
/*---------------------------------------------------------------------------*/

int getvalid_sock (void)
//*************************************************************************
//
//  prueft ob ein neues Zeichen vom Socket gekommen ist
//
//*************************************************************************
{
  int a;
  lastfunc("getvalid_sock");

  sock_t *s = &so[t->socket];
  if (s->txc && s->lastflush != ad_time())
  {
    s->lastflush = ad_time();
    putflush_sock();
  }
  if (s->wasda)
  {
    if (s->dasda == CR) return CR;
    else return OK;
  }
  a = holsock(s);
  if (a != EOF && a != LF)
  {
    s->wasda = OK;
    s->dasda = (char) a;
  }
  if (s->wasda)
  {
    if (s->dasda == CR) return CR;
    else return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

int getv_sock_ftp (void)
//*************************************************************************
//
// Speziell fuer FTP, da FTP nach dem Uebertragen einfach das Socket schliesst
//
//*************************************************************************
{
  int a;
  lastfunc("getv_sock_ftp");

  sock_t *s = &so[t->socket];
  timeout(TIMEOUT);
  t->last_input = ad_time();
  if (s->wasda)
  {
    s->wasda = NO;
    return s->dasda;
  }
  while ((a = holsock_ftp(s)) == EOF) wdelay(78);
  if (b) b->rxbytes++;
  return a;
}

/*---------------------------------------------------------------------------*/

int getv_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  int a;
  lastfunc("getv_sock");

  sock_t *s = &so[t->socket];
  t->last_input = ad_time();
  if (s->wasda)
  {
    s->wasda = NO;
    return s->dasda;
  }
  while (1)
  {
  lastfunc("getv_sock1");
    while ((a = holsock(s)) == EOF) wdelay(77);
  lastfunc("getv_sock2");
    if (! (t->processkind & P_TELNET)) break;
  lastfunc("getv_sock3");
    if (a == 0xFF) //IAC (Interpret as command) see RFC854
    {
      while ((a = holsock(s)) == EOF) wdelay(78);
      if (a == 0xFF) break; //2xIAC -> 0xFF data byte
      if (a < 251) continue; //various control codes etc. .. we could support some
      // a >= 251 -> WILL/WON'T/DO/DON'T .. read third byte (and ignore it)
      while ((a = holsock(s)) == EOF) wdelay(79);
    }
    else break;
  }
  lastfunc("getv_sock4");
  return a;
}

/*---------------------------------------------------------------------------*/

void inputline_sock (char *s, int maxlen, char cut)
//*************************************************************************
//
//
//
//*************************************************************************
{
  lastfunc("inputline_sock");
  unsigned int i = 0;
  int semi = NO;

  if (maxlen < 0)
  {
    // db7mh if (cut) semi = OK;
    maxlen = (-maxlen);
  }
  if (so[t->socket].wasput && ! getvalid_sock())
    putflush_sock();
  while (1)
  {
    if ( /*! cut &&*/ i == (unsigned) maxlen) break;
    do s[i] = getv_sock();
    while (s[i] == LF);
    if (semi && s[i] == ';') break;
    if (s[i] == CR)
    {
      if (! cut)
        s[i++] = LF;
      break;
    }
    if (s[i] == 8 || s[i] == 127)
    {
      if (i) i--;
      continue;
    }
    if (i < (unsigned) maxlen) i++;
  }
  s[i] = 0;
}

/*---------------------------------------------------------------------------*/

void disconnect_sock (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  closesocket(so[t->socket].handle);
  so[t->socket].handle = EOF;
  t->socket = 0;
}

/*---------------------------------------------------------------------------*/

static void mblogin_socket (char *name)
//*************************************************************************
//
//
//
//*************************************************************************
{
  char s[21];
  char s1[CALLSSID+1] = "";
  int unknown = *name == '*';
  if (unknown) name++;
  char *up = "TELNET";

  t->processkind |= P_TELNET;
  int ok = getlogin(s, unknown, up);
  if (ok)
  {
    b->sysop = ! unknown;
    strcpy(b->peerip, name);
    strcpy(t->name, s);
    if (ok == 2)
      strcpy(s1, m.mycall[1]);
    mbmain(s, up, s1);
  }
  wdelay(320);
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/

static int checkrhost (unsigned long addr)
//*************************************************************************
//
// rhosts.bcm ueberpruefen
//
//*************************************************************************
{
  FILE *f = s_fopen(RHOSTSNAME, "srt");
  char s[LINELEN+1];
  if (! f)
  {
    char rhosts_init[] =
      "; OpenBCM sample rhosts.bcm file\n"
      "; might be unsecure!\n"
      ";\n"
      "127.0.0.1\n";
    if ((f = s_fopen(RHOSTSNAME, "sw+t")) != NULL)
    {
      fwrite(rhosts_init, sizeof(rhosts_init) - 1, 1, f);
      trace(replog, "checkrhost", "sample "RHOSTSNAME" generated");
      rewind(f);
    }
  }
  if (f)
  {
    struct hostent *hp;
    while (fgets(s, LINELEN, f))
    {
      s[LINELEN] = 0;
      subst(s, ';', 0); //remove comments
      cut_blank(s);
      if (! *s) continue;
      hp = gethostbyname(s);
      if (hp && addr == ((struct in_addr *) (hp->h_addr))->s_addr)
      {
        trace(report, "checkrhost", "%s found", s);
        s_fclose(f);
        return OK;
      }
    }
    s_fclose(f);
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
void cleanup (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  WSACleanup();
}

/*---------------------------------------------------------------------------*/

void ws_init (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  static int first = 1;
  if (first)
  {
    first = 0;
    wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData))
      trace(tr_abbruch, "ws_init", "winsock.dll not found");
    atexit(cleanup);
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE (wsaData.wVersion) != 1)
      trace(tr_abbruch, "ws_init", "wrong winsocket implementation");
    trace(report, "ws_init", "Sockets %u, UDP size %u",
                  wsaData.iMaxSockets, wsaData.iMaxUdpDg);
    trace(report, "ws_init", "Sockets Version: %d.%d - High Version: %d.%d",
                  LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
                  LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
    trace(report, "ws_init", "%s %s",
                  wsaData.szDescription, wsaData.szSystemStatus);
  }
}
#else
#define ws_init()
#endif

/*---------------------------------------------------------------------------*/

void logindaemon_sock (char *name)
//*************************************************************************
//
// Socket Login-Daemon
//
//*************************************************************************
{
  static int sockinit = 0;
#ifdef __GLIBC__
  unsigned
#endif
  int addrlen;
  int i, s = 0, ls;
  struct servent *sp;             // pointer to service information
  struct sockaddr_in myaddr_in;   // for local socket address
  struct sockaddr_in peeraddr_in; // for peer socket address
  long strue = 1;
  unsigned long peerip = 0;
  char peerips[17];
  int service = _TELNET;

  if (! sockinit)
  {
    sockinit++;
    for (i = 0; i < MAXSOCK; i++) so[i].handle = EOF;
  }
  if (! strcmp(name, "httpd"))    service = _HTTP;
  if (! strcmp(name, "ftpd"))     service = _FTP;
  if (! strcmp(name, "sendmail")) service = _SMTP;
  if (! strcmp(name, "popper"))   service = _POP;
  if (! strcmp(name, "nntpd"))    service = _NNTP;
  if (! strcmp(name, "radiod"))   service = _RADIO;
  if (! strcmp(name, "serv"))     service = _SERV;
//  wdelay(1198);
  ws_init();
  // clear out address structures
  memset((char *) &myaddr_in,   0, sizeof(struct sockaddr_in));
  memset((char *) &peeraddr_in, 0, sizeof(struct sockaddr_in));
  myaddr_in.sin_family = AF_INET;
  myaddr_in.sin_addr.s_addr = INADDR_ANY;
  switch (service)
  {
  case _HTTP:
    if (! m.http_port)
    {
      trace(report, name, "http connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.http_port);
    break;
  case _FTP:
    if (! m.ftp_port)
    {
      trace(report, name, "ftp connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.ftp_port);
    break;
  case _SMTP:
    if (! m.smtp_port)
    {
      trace(report, name, "smtp connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.smtp_port);
    break;
  case _POP:
    if (! m.pop3_port)
    {
      trace(report, name, "pop3 connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.pop3_port);
    break;
  case _NNTP:
#ifdef NNTP
    if (! m.nntp_port)
    {
      trace(report, name, "nntp connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.nntp_port);
    break;
#else
    return;
#endif
  case _RADIO:
#ifdef RADIOIF
    if (! m.radio_port)
    {
      trace(report, name, "radio connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.radio_port);
    break;
#else
    return;
#endif
  case _SERV:
#ifdef SERVIF
    if (! m.serv_port)
    {
      trace(report, name, "remote service connectivity disabled");
      return;
    }
    myaddr_in.sin_port = htons(m.serv_port);
    break;
#else
    return;
#endif
  case _TELNET:
    if (! m.tcp_port)
    {
      trace(report, name, "telnet connectivity disabled");
      return;
    }
    else if (m.tcp_port == 1)
    {
      m.tcp_port = DEFTELNPORT;
#ifdef _WIN32
      sp = NULL;
#else
      sp = getservbyname("bcm", "tcp");
#endif
      if (! sp)
      {
        trace(report, name, "bcm not found in /etc/services");
        myaddr_in.sin_port = htons(DEFTELNPORT);
      }
      else myaddr_in.sin_port = sp->s_port;
    }
    else myaddr_in.sin_port = htons(m.tcp_port);
    break;
  }
//------------------
  if ((ls = socket(AF_INET, SOCK_STREAM, 0)) == NIL)
  {
    trace(serious, name, "create: %d %s", errno, strerror(errno));
    return;
  }
#ifdef _WIN32  //WINDOWS: non-blockingmode einstellen
  if (ioctlsocket (ls, FIONBIO, &blockmode))
  {
    trace(serious, name, "ioctlsocket: %d %s", errno, strerror(errno));
    return;
  }
#else
  if (fcntl(ls, F_SETFL, fcntl(s,F_GETFL) | O_NONBLOCK) < 0) //dg2fef
  {
    trace(serious, name, "fcntl: %d %s", errno, strerror (errno));
    return;
  }
  if (setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &strue, 4) == NIL)
  {
    trace(tr_abbruch, name, "setsockopt: %d %s", errno, strerror(errno));
    return;
  }
#endif
  if (bind(ls, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == NIL)
  {
    trace(tr_abbruch, name, "bind: %d %s", errno, strerror(errno));
    return;
  }
  if (listen(ls, 5) == -1)
  {
    trace(serious, name, "listen: %d %s", errno, strerror(errno));
    return;
  }
  trace(report, name, "initialised to port %d", ntohs(myaddr_in.sin_port));
  while (! runterfahren)
  {
    addrlen = sizeof(struct sockaddr_in);
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    wdelay(80);
    FD_ZERO(&rfds);
    FD_SET(ls, &rfds);
    if (! select(ls + 1, &rfds, NULL, NULL, &tv)) continue;
    s = accept(ls, (struct sockaddr *) &peeraddr_in, &addrlen);
    if (s != EOF && (i = find_free_sock()) != NIL)
    {
      memset(&so[i], 0, sizeof(sock_t));
      so[i].handle = s;
      so[i].tcreate = ad_time();
#ifdef _WIN32 //WINDOWS: non-blockingmode einstellen
      if (ioctlsocket(s, FIONBIO, &blockmode))
      {
        trace(serious, name, "ioctlsocket2: %d %s", errno, strerror(errno));
        continue;
      }
#else
      if (fcntl(s, F_SETFL, O_NONBLOCK) < 0)
      {
        trace(serious, name, "fcntl2: %d %s", errno, strerror(errno));
        continue;
      }
#endif
      peerip = ((struct sockaddr_in) peeraddr_in).sin_addr.s_addr;
      strcpy(peerips, in_ntoa(peerip));
      trace(report, name, "socket %d / ip: [%s]", s, peerips);
#ifdef __UNIX__
      struct linger lli; // allow a lingering, graceful close;
      lli.l_onoff = 1;
      lli.l_linger = 1;
      if (setsockopt(s, SOL_SOCKET, SO_LINGER, &lli, sizeof(struct linger)) == NIL)
      {
        trace(serious, name, "setsockopt: %d %s", errno, strerror(errno));
        continue;
      }
#endif
      switch (service)
      {
#ifdef SERVIF
      case _SERV:
        if (checkrhost(((struct sockaddr_in) peeraddr_in).sin_addr.s_addr))
          fork(P_MAIL, -i, mblogin_serv, peerips);
        break;
#endif
      case _HTTP: fork(P_MAIL, -i, mblogin_http, peerips); break;
      case _FTP:  fork(P_MAIL, -i, mblogin_ftp, peerips); break;
      case _SMTP: fork(P_MAIL, -i, mblogin_smtp, peerips); break;
      case _POP:  fork(P_MAIL, -i, mblogin_pop3, peerips); break;
#ifdef NNTP
      case _NNTP: fork(P_MAIL, -i, mblogin_nntp, peerips); break;
#endif
#ifdef RADIOIF
      case _RADIO:
        if (checkrhost(((struct sockaddr_in) peeraddr_in).sin_addr.s_addr))
          fork(P_MAIL, -i, mblogin_radio, peerips);
        break;
#endif
      case _TELNET:
        int wellknown = checkrhost(((struct sockaddr_in) peeraddr_in).sin_addr.s_addr);
        if (! wellknown)
        {
          memmove(peerips + 1, peerips, strlen(peerips) + 1);
          *peerips = '*';
          peerips[16] = 0 ;
        }
        fork(P_MAIL, -i, mblogin_socket, peerips);
        break;
      }
    }
  }
  closesocket(ls);
}

/* ------------------------------------------------------------------------- */

#ifdef _TELNETFWD
int telnet_fwd_connect (char *addr)
//*************************************************************************
//
// creates a telnet connection, works only for fwd (not with CONN cmd)
//
//*************************************************************************
{
  char name[20];
  char *uc;
  char *host = NULL;
  char *port;
  int s, socket_result;
  struct hostent *hp; // pointer to host info for remote host
  struct sockaddr_in peeraddr_in; // for peer socket address

  memset((char *) &peeraddr_in, 0, sizeof(struct sockaddr_in));
  strcpy(name, "telnet_fwd_connect");
  lastfunc(name);
//  strupr(mycall);
  if (! (uc = strchr(addr, ':')) || ! (host = strtok(uc + 1, ": ")))
    return NO;
  if (! (port = strtok(NULL, ": ")))
  {
    trace(serious, name, "Port number must be specified: %s", host);
    return NO;
  }
  if (! *host)
  {
    trace(serious, name, "No hostname");
    return NO;
  }
  if (! (hp = gethostbyname(host)))
  {
    trace(serious, name, "Name lookup error: %s", host);
    return NO;
  }
  peeraddr_in.sin_family = AF_INET;
  peeraddr_in.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  peeraddr_in.sin_port = htons(atoi(port));
  trace(report, name, "Connecting to %s (%s) with port %d", hp->h_name,
                in_ntoa(peeraddr_in.sin_addr.s_addr), atoi(port));
  s = socket(peeraddr_in.sin_family, SOCK_STREAM, 0);
  if (s == NIL)
  {
    trace(serious, name, "socket %d: %d %s", s, errno, strerror(errno));
    return NO;
  }
#ifdef _WIN32 //WINDOWS: non-blockingmode einstellen
  if (ioctlsocket(s, FIONBIO, &blockmode))
  {
    trace(serious, name, "ioctlsocket: %d %s", errno, strerror(errno));
    return NO;
  }
#else
  if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) < 0)
  {
    trace(serious, name, "fcntl: %d %s", errno, strerror(errno));
    close(s);
    return NIL;
  }
#endif
  if ( (socket_result = connect(s, (struct sockaddr *) &peeraddr_in,
                                sizeof(struct sockaddr_in))) == NIL
        && errno != EINPROGRESS)
  {
    if (errno != 10035)
    {
      trace(report, name, "connect: %d %d %s", socket_result,
                                               errno, strerror(errno));
      close(s);
      return NO;
    }
  }
  fd_set wfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&wfds);
  int tret = 0;
#ifdef __GLIBC__
  unsigned
#endif
  int retlen = sizeof(int);
  do
  {
    FD_SET(s, &wfds);
    wdelay(243);
  }
  while (! (tret = select(s + 1, NULL, &wfds, NULL, &tv)));
  if (tret == NIL)
  {
    trace(report, name, "select: %d %s", errno, strerror(errno));
    close(s);
    return NO;
  }
#ifndef _WIN32
  getsockopt(s, SOL_SOCKET, SO_ERROR, &tret, &retlen);
  if (tret)
  {
    trace(report, name, "fail %s: %s", hp->h_name, strerror(tret));
    close(s);
    return NO;
  }
#endif
  int socknum;
  if ((socknum = find_free_sock()) == NIL)
  {
    trace(serious, name, "socket array full");
    close(s);
    return NO;
  }
  memset(&so[socknum], 0, sizeof(sock_t));
  so[socknum].tcreate = ad_time();
  so[socknum].handle = s;
//  strcpy(so[socknum].mycall, mycall);
  t->socket = socknum;
  t->input = t->output = io_socket;
  strcpy(b->peerip, hp->h_name);
  t->processkind |= P_TELNET;
  trace(report, name, "ok %s:%d", hp->h_name, atoi(port));
  return OK;
}
#endif

/* ------------------------------------------------------------------------- */

int ftp_connect (char *host, unsigned int port)
//*************************************************************************
//
// Baut eine Verbindung fuer FTP auf....
//
//*************************************************************************
{
  char name[20];
  int socknum;
  int s, socket_result;
  struct hostent *hp; // pointer to host info for remote host
  struct sockaddr_in peeraddr_in; // for peer socket address
  memset((char *) &peeraddr_in, 0, sizeof(struct sockaddr_in));

  strcpy(name, "ftp_connect");
  lastfunc(name);

  if (! *host)
  {
    trace(serious, name, "No hostname");
    return NO;
  }
  if (! (hp = gethostbyname(host)))
  {
    trace(serious, name, "Name lookup error: %s", host);
    return NO;
  }
  peeraddr_in.sin_family = AF_INET;
  peeraddr_in.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  peeraddr_in.sin_port = htons(port);
  trace(report, name, "Connecting %s:%d",
                 in_ntoa(peeraddr_in.sin_addr.s_addr), port);
  if ((s = socket(peeraddr_in.sin_family, SOCK_STREAM, 0)) == NIL)
  {
    trace(serious, name, "socket: %d %s", errno, strerror(errno));
    return NO;
  }
#ifdef _WIN32 //WINDOWS: non-blockingmode einstellen
  if (ioctlsocket(s, FIONBIO, &blockmode))
  {
    trace(serious, name, "ioctlsocket: %d %s", errno, strerror(errno));
    return NO;
  }
#else
  if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) < 0)
  {
    trace(serious, name, "fcntl: %d %s", errno, strerror(errno));
    close(s);
    return NIL;
  }
#endif
  if ( (socket_result = connect(s, (struct sockaddr *) &peeraddr_in,
          sizeof(struct sockaddr_in))) == NIL
        && errno != EINPROGRESS)
  {
    if (errno != 10035)
    {
      trace(report, name, "connect: %d %d %s", socket_result,
                                               errno, strerror(errno));
      close(s);
      return NO;
    }
  }
  fd_set wfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&wfds);
  int tret = 0;
#ifdef __GLIBC__
  unsigned
#endif
  int retlen = sizeof(int);
  do
  {
    FD_SET(s, &wfds);
    wdelay(243);
  }
  while (! (tret = select(s + 1, NULL, &wfds, NULL, &tv)));
  if (tret == NIL)
  {
    trace(report, name, "select: %d %s", errno, strerror(errno));
    close(s);
    return NO;
  }
#ifndef _WIN32
  getsockopt(s, SOL_SOCKET, SO_ERROR, &tret, &retlen);
  if (tret)
  {
    trace(report, name, "fail %s: %s", hp->h_name, strerror(tret));
    close(s);
    return NO;
  }
#endif
  if ((socknum = find_free_sock()) == NIL)
  {
    trace(serious, name, "socket array full");
    close(s);
    return NO;
  }
  memset(&so[socknum], 0, sizeof(sock_t));
  so[socknum].tcreate = ad_time();
  so[socknum].handle = s;
  t->socket = socknum;
  t->input = t->output = io_socket;
  t->output = io_socket;
  strcpy(b->peerip, hp->h_name);
  t->processkind |= P_TELNET;
  trace(report, name, "ok %s:%d", hp->h_name, port);
//  *my_socknum = socknum;
  return OK;
}
#endif
