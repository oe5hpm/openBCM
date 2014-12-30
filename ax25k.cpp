/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------
  Linux Kernel AX25 Support
  -------------------------

  Copyright (C)       Deti Fliegl, deti@gmx.de, DG9MHZ

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980810 DH3MB  fixed bug of single cr's
//19980824 CZ4WAY added multiple ax-interfaces
//                CZ4WAY (CB) = Jan Wasserbauer <jan.wasserbauer@spsselib.hiedu.cz>
//19980831 OE3DZW merged both versions, used indent to get a common code
//                addrlen is SIGNED int!
//19980914 OE3DZW added unsigned for glibc (from 1.40c, dh3mb)
//19980917 OE3DZW made Jans interface optional (if _JAN is set)
//19980924 OE3DZW fixes by Jan
//19990205 DG9MHZ included Jans improvements again; fixes
//19991028 Jan    fixed connect failure check; fixed flush
//20000112 OE3DZW fixed bug corrupting data

#include "baycom.h"

#ifdef _AX25K
//#define OLD_FLUSH

#include <sys/socket.h>
#include <linux/ax25.h>
#include <sys/ioctl.h>

#define MAXMYCALLS 4
#define MAXLSOCK 20
#define TXLEN 512
#define RXLEN 512
#define MAXSOCK 256

/*---------------------------------------------------------------------------*/

typedef struct sock_t
{
  char rxbuf[RXLEN];
  char txbuf[TXLEN];
  char mycall[16];
  int handle;
  int rxc;
  int txc;
  int rxlen;
  int wasput;
  int wasda;
  char dasda;
  time_t lastflush;
}
sock_t;

static sock_t so[MAXSOCK];
static int ax25k_ok = 0;
static int ax25k_default = 0;
static char ax25k_default_port[80];
static int listen_sock[MAXLSOCK];
static unsigned int listen_sock_num = 0;

/*---------------------------------------------------------------------------*/

static int find_free_sock (void)
//****************************************************************************
//
//****************************************************************************
{
  for (unsigned int i = 1; i < MAXSOCK; i++)
    if (so[i].handle == EOF)
    {
      memset(&so[i], 0, sizeof(sock_t));
      //so[i].handle = EOF;
      return i;
    }
  return EOF;
}

/*---------------------------------------------------------------------------*/

static char *ax2asc (ax25_address * a)
//****************************************************************************
//
//****************************************************************************
{
  static char buf[CALLSSID+1];
  char c, *s;
  unsigned int n;

  for (n = 0, s = buf; n < 6; n++)
  {
    c = (a->ax25_call[n] >> 1) & 0x7F;
    if (c != ' ') *s++ = c;
  }
  *s++ = '-';
  if ((n = ((a->ax25_call[6] >> 1) & 0x0F)) > 9)
  {
    *s++ = '1';
    n -= 10;
  }
  *s++ = n + '0';
  *s++ = 0;
  return buf;
}

/*---------------------------------------------------------------------------*/

static int convert_call_entry (const char *name, char *buf)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int ct = 0;
  int ssid = 0;
  const char *p = name;
  char c;

  while (ct < 6)
  {
    c = toupper(*p);
    if (c == '-' || ! c) break;
    if (! isalnum(c))
    {
      trace(serious, "convert_call_entry",
                     "invalid symbol in callsign %s", name);
      return EOF;
    }
    buf[ct++] = c << 1;
    p++;
  }
  while (ct < 6) buf[ct++] = ' ' << 1;
  if (*p)
  {
    p++;
    if (sscanf(p, "%d", &ssid) != 1 || ssid < 0 || ssid > 15)
    {
      trace(serious, "convert_call_entry",
          "SSID must follow '-' and be numeric in the range 0-15 - %s", name);
      return NOTFOUND;
    }
  }
  buf[6] = ((ssid + '0') << 1) & 0x1E;
  return NO;
}

/*---------------------------------------------------------------------------*/

static int convert_call_arglist (char *call, struct full_sockaddr_ax25 *sax)
//****************************************************************************
//
//****************************************************************************
{
  char *bp;
  char *addrp;
  char *seps = " ";
  unsigned int n = 0;

  addrp = sax->fsa_ax25.sax25_call.ax25_call;
  if ((bp = strtok(call, seps)) != NULL)
    do
    {
      // Process the token
      if (convert_call_entry(bp, addrp) == NOTFOUND) return -1;
      n++;
      if (n == 1)
        addrp = sax->fsa_digipeater[0].ax25_call; // First digipeater address
      else
        addrp += sizeof (ax25_address);
    }
    while (n < AX25_MAX_DIGIS && ((bp = strtok(NULL, seps)) != NULL));
  // Tidy up
  sax->fsa_ax25.sax25_ndigis = n - 1;
  sax->fsa_ax25.sax25_family = AF_AX25;
  return sizeof (struct full_sockaddr_ax25);
}

/*---------------------------------------------------------------------------*/

static int linkcall (struct full_sockaddr_ax25 *addr, char *s)
//****************************************************************************
//
//****************************************************************************
{
  sprintf(s, "%s ", ax2asc(&addr->fsa_ax25.sax25_call));
#ifdef _DEBUG
  printf("ndigis: %d -> %s\n", addr->fsa_ax25.sax25_ndigis, s);
  for (int i = 0; i < addr->fsa_ax25.sax25_ndigis; i++)
  {
    printf("digi: %d ", i);
    for (unsigned int j = 0; j < 6; j++)
      printf("%02X ", addr->ax25.fsa_digipeater[i].ax25_call[j]);
    putc(LF);
  }
#endif
  if (addr->fsa_ax25.sax25_ndigis < AX25_MAX_DIGIS)
  {
    if (addr->fsa_ax25.sax25_ndigis > 0)
      strcat(s, ax2asc(&addr->fsa_digipeater[addr->fsa_ax25.sax25_ndigis - 1]));
  }
  else
  {
    //trace(serious, "linkcall", "ndigis: %d", addr->fsa_ax25.sax25_ndigis);
    return NO;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int holsock (sock_t *s)
//****************************************************************************
//
//****************************************************************************
{
  fd_set rfds;
  struct timeval tv;
  lastfunc("ax25k_holsock");

  if (t->port == EOF) suicide();
  if (s->rxc >= s->rxlen)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(s->handle, &rfds);
    if (select(s->handle + 1, &rfds, NULL, NULL, &tv) < 1) return EOF;
    if ((s->rxlen = read(s->handle, s->rxbuf, RXLEN)) < 1)
    {
      if ((s->rxlen == EOF || errno == ENOTCONN || errno == EBADF)
          && errno != EAGAIN)
        suicide();
      return EOF;
    }
    s->rxc = 0;
  }
  return s->rxbuf[s->rxc++];
}

/*---------------------------------------------------------------------------*/

int ax25k_putfree ()
//****************************************************************************
//
//****************************************************************************
{
  return OK;
}

/*---------------------------------------------------------------------------*/

void ax25k_putv (int c)
//****************************************************************************
//
//****************************************************************************
{
  if (t->port == EOF) suicide();
  sock_t *s = &so[t->port];
  s->txbuf[s->txc++] = c;
  s->wasput = 1;
  if (s->txc >= TXLEN) ax25k_flush();
}

/*---------------------------------------------------------------------------*/

void ax25k_flush()
//****************************************************************************
//
//****************************************************************************
{
  int ok = 0, sum = 0;

  b->pacbytes = 0;
  if (t->port == EOF) suicide();
  sock_t *s = &so[t->port];
  if (s->txc)
  {
    while (1)
    {
      if (t->port == EOF) suicide();
#ifdef OLD_FLUSH
      fd_set wfds;
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      FD_ZERO(&wfds);
      FD_SET(s->handle, &wfds);
      if (select(s->handle + 1, NULL, &wfds, NULL, &tv))
      {
#endif
      //using select() before write is cleaner but in some cases
      //disconnected socket is indefinitely not ready for writing
      //and task keeps running forever
      if ((ok = write(s->handle, s->txbuf + sum, s->txc - sum)) < 0)
      {
        if (errno != EAGAIN && errno != EINTR) suicide();
      }
      else sum += ok;
      if (sum == s->txc) break;
#ifdef OLD_FLUSH
      }
#endif
      wdelay(40);
    }
    s->txc = 0;
  }
}

/*---------------------------------------------------------------------------*/

void ax25k_putbeacon (char *addr, char *beacontext)
//****************************************************************************
//
//****************************************************************************
{
  char name[20];
  int sock, addrlen = sizeof(struct full_sockaddr_ax25);
  struct full_sockaddr_ax25 axbind, axconnect;
  char *port;
  char *path;
  char *cp;

  strcpy(name, "ax25k_putbeacon");
  if ((cp = strchr(addr, ':')) != NULL)
  {
    port = strtok(cp + 1, " ");
    path = cp + strlen(port) + 2;
    for (; *path == ' '; path++);
  }
  else
  {
    port = ax25k_default_port;
    path = addr;
  }
  axconnect.fsa_ax25.sax25_family = axbind.fsa_ax25.sax25_family = AF_AX25;
  axbind.fsa_ax25.sax25_ndigis = 1;
  if (convert_call_entry(port, axbind.fsa_digipeater[0].ax25_call) == NOTFOUND)
  {
    trace(serious, name, "invalid AX.25 interface callsign - %s", addr);
    return;
  }
  if (convert_call_entry(m.mycall[0], axbind.fsa_ax25.sax25_call.ax25_call) == NOTFOUND)
  {
    trace(serious, name, "invalid callsign - %s", m.mycall[0]);
    return;
  }
  if (convert_call_arglist(path, &axconnect) == NOTFOUND)
  {
    trace(serious, name, "invalid destination callsign or digipeater");
    return;
  }
  if ((sock = socket(AF_AX25, SOCK_DGRAM, 0)) < 0)
  {
    trace(serious, name, "cannot open AX.25 socket, %s", strerror(errno));
    return;
  }
  if (bind(sock, (struct sockaddr *) &axbind, addrlen))
  {
    trace(serious, name, "cannot bind AX.25 socket, %s", strerror(errno));
    close (sock);
    return;
  }
  if (connect(sock, (struct sockaddr *) &axconnect, addrlen))
  {
    switch (errno)
    {
      case ECONNREFUSED:
           trace(report, name, "*** Connection refused - aborting"); break;
      case ENETUNREACH:
           trace(report, name, "*** No known route - aborting"); break;
      case EINTR:
           trace(report, name, "*** Connection timed out - aborting"); break;
      default:
           trace(report, name, "cannot connect to %s, %s", addr, strerror(errno)); break;
    }
    close(sock);
    return;
  }
  write(sock, beacontext, strlen(beacontext));
  close(sock);
}

/*---------------------------------------------------------------------------*/

int ax25k_getvalid_tnc ()
//****************************************************************************
//
//****************************************************************************
{
  int a;
  if (t->port == EOF) suicide();
  sock_t *s = &so[t->port];
  if (s->wasda)
  {
    if (s->dasda == CR) return CR;
    else return OK;
  }
  if ((a = holsock(s)) != EOF)
  {
    s->wasda = OK;
    s->dasda = (char) a;
  }
  if (s->wasda)
  {
    if (s->dasda == CR) return CR;
    else  return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

int ax25k_getv ()
//****************************************************************************
//
//****************************************************************************
{
  int a;
  if (t->port == EOF) suicide();
  sock_t *s = &so[t->port];
  if (s->wasda)
  {
    s->wasda = 0;
    return s->dasda;
  }
  while ((a = holsock(s)) == EOF) wdelay(47);
  return a;
}

/*---------------------------------------------------------------------------*/

void ax25k_getclear ()
//****************************************************************************
//
//****************************************************************************
{
  sock_t *s = &so[t->port];
  while (holsock(s) != EOF) ;  // RX-Puffer ausleeren

}

/*---------------------------------------------------------------------------*/

void ax25k_getmycall (char *s)
//****************************************************************************
//
//****************************************************************************
{
  strcpy(s, so[t->port].mycall);
}

/*---------------------------------------------------------------------------*/

void ax25k_inputline (char *s, int maxlen, char cut)
//****************************************************************************
//
//****************************************************************************
{
  int i = 0;
  int semi = 0;

  if (maxlen < 0)
  {
    // db7mh if (cut) semi = 1;
    maxlen = (-maxlen);
  }
  if (so[t->port].wasput && ! ax25k_getvalid_tnc()) ax25k_flush();
  while (1)
  {
    if ( /*! cut &&*/ i == maxlen) break;
    do s[i] = ax25k_getv();
    while (s[i] == LF);
    if (semi && s[i] == ';') break;
    if (s[i] == CR)
    {
      if (! cut) s[i++] = LF;
      break;
    }
    if (s[i] == 8 || s[i] == 127)
    {
      if (i) i--;
      continue;
    }
    if (i < maxlen) i++;
  }
  s[i] = 0;
}

/*---------------------------------------------------------------------------*/

int ax25k_connect (char *mycall, char *conncall)
//****************************************************************************
//
//****************************************************************************
{
  char name[20];
  int addrlen = sizeof(struct full_sockaddr_ax25);
  struct full_sockaddr_ax25 axbind, axconnect;
  int s;
  char *port;
  char *path;
  char *cp;

  strcpy(name, "ax25k_putbeacon");
  if ((cp = strchr(conncall, ':')) != NULL)
  {
    port = strtok(cp + 1, " ");
    path = cp + strlen(port) + 2;
    for (; *path == ' '; path++) ;
  }
  else
  {
    port = ax25k_default_port;
    path = conncall;
  }
  axconnect.fsa_ax25.sax25_family = axbind.fsa_ax25.sax25_family = AF_AX25;
  axbind.fsa_ax25.sax25_ndigis = 1;
  if (convert_call_entry(port, axbind.fsa_digipeater[0].ax25_call) == NOTFOUND)
  {
    trace(serious, name, "invalid AX.25 port callsign - %s", port);
    return NO;
  }
  if (convert_call_entry(mycall, axbind.fsa_ax25.sax25_call.ax25_call) == NOTFOUND)
  {
    trace(serious, name, "invalid callsign - %s", mycall);
    return NO;
  }
  if (convert_call_arglist(path, &axconnect) == NOTFOUND)
  {
    trace(serious, name, "invalid destination callsign or digipeater %s", path);
    return NO;
  }
  if ((s = socket(AF_AX25, SOCK_SEQPACKET, 0)) < 0)
  {
    trace(serious, name, "cannot open AX.25 socket, %s", strerror (errno));
    return NO;
  }
  if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) < 0)
  {
    trace(serious, name, "fcntl: %s", strerror(errno));
    close(s);
    return NO;
  }
  if (bind(s, (struct sockaddr *) &axbind, addrlen))
  {
    trace(serious, name, "cannot bind AX.25 socket, %s", strerror (errno));
    close(s);
    return NO;
  }
  if (connect(s, (struct sockaddr *) &axconnect, addrlen))
  {
    if (errno != EINPROGRESS)
    {
      trace(report, name, "conn to %s, %s", conncall, strerror (errno));
      close(s);
      return NO;
    }
  }
  fd_set wfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&wfds);
  int ret = 0;
  #ifdef __GLIBC__
  unsigned
  #endif
  int retlen = sizeof(int);
  do
  {
    FD_SET(s, &wfds);
    wdelay(243);
  }
  while (! (ret = select(s + 1, NULL, &wfds, NULL, &tv)));
  if (ret == NOTFOUND)
  {
    trace(report, name, "select: %s", strerror(errno));
    close(s);
    return NO;
  }
  getsockopt(s, SOL_SOCKET, SO_ERROR, &ret, &retlen);
  if (ret)
  {
    trace(report, name, "fail %s %s", path, strerror(ret));
    close(s);
    return NO;
  }
  int socknum;
  if ((socknum = find_free_sock()) == NOTFOUND)
  {
    trace(serious, name, "socket array full");
    close(s);
    return NO;
  }
  so[socknum].handle = s;
  strcpy(so[socknum].mycall, mycall);
  t->port = socknum;
  t->processkind |= P_AX25K;
  trace(report, name, "ok %s", path);
  return OK;
}

/*---------------------------------------------------------------------------*/

void ax25k_disconnect (int wait)
//****************************************************************************
//
//****************************************************************************
{
  if (t->port == EOF) return;
  close(so[t->port].handle);
  so[t->port].handle = EOF;
  t->port = EOF;
}

/*---------------------------------------------------------------------------*/

void ax25k_checklogin ()
//****************************************************************************
//
//****************************************************************************
{
  if (! ax25k_ok) return;
  char name[20];
  unsigned int i;
  struct full_sockaddr_ax25 newconn;
#ifdef __GLIBC__
  unsigned
#endif
  int addrlen;
  int ret;
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO (&rfds);
  int maxsock = 0;

  strcpy(name, "ax25k_checklogin");
  for (i = 0; i < listen_sock_num; i++)
  {
    if (listen_sock[i] > maxsock) maxsock = listen_sock[i];
    FD_SET(listen_sock[i], &rfds);
  }
  if (select(maxsock + 1, &rfds, NULL, NULL, &tv) <= 0) return;
  for (i = 0; i < listen_sock_num; i++)
  {
    if (! FD_ISSET (listen_sock[i], &rfds)) continue;

    // from ax25d:
    /*
     * Setting up a non-blocking accept() so is does not hang up
     *  - I am not sure at this time why I didn't/don't assign
     *  the socket non-blocking to start with.
     */
    // in bcm sockets are assigned non-blocking in ax25k_init
    // -> FIONBIO not not needed
    /*
    int j = TRUE;
    ioctl (listen_sock[i], FIONBIO, &j);
    */
    addrlen = sizeof(struct full_sockaddr_ax25);
    ret = accept(listen_sock[i], (struct sockaddr *) &newconn, &addrlen);
    /*
    j = FALSE;
    ioctl (listen_sock[i], FIONBIO, &j);
    */
    if (ret >= 0)
    {
      char buf[LINELEN+1];
      //printf("login: %d, %d addrlen: %d\n", i, ret, addrlen);
      if (! linkcall(&newconn, buf))
      {
        close(ret);
        break;
      }
      buf[30] = 0;
      if (fcntl(ret, F_SETFL, fcntl(ret, F_GETFL) | O_NONBLOCK) < 0)
      {
        trace(serious, name, "fcntl: %d %s", errno, strerror(errno));
        close(ret);
        break;
      }
      int socknum = find_free_sock();
      if (socknum == NOTFOUND)
      {
        trace(serious, name, "socket array full");
        close(ret);
        break;
      }
      so[socknum].handle = ret;
      strcpy(so[socknum].mycall, m.mycall[i % m.mycalls]);
      fork(P_MAIL | P_AX25K, socknum, boxusertask, buf);
    }
    else
    {
      trace(report, name, "accept: %d %s", errno, strerror(errno));
      if (errno==22) ax25k_ok=0; // Unser Interface ist nicht OK ;=(
    }
  }
}

/*---------------------------------------------------------------------------*/

int ax25k_isdefault ()
//****************************************************************************
//
//****************************************************************************
{
  return ax25k_default;
}

/*---------------------------------------------------------------------------*/

int ax25k_reinit ()
//****************************************************************************
//
//****************************************************************************
{
  if (! strcasecmp(m.ax25k_if, "off"))
  {
    return 0;
  }
  else if (! ax25k_ok)
  {
    trace(report, "ax25k_reinit", "trying reinit Interface");
    ax25k_init();
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

int ax25k_init ()
//****************************************************************************
//
//****************************************************************************
{
  static int sockinit = 0;
  char name[20];
  char port[80];
  unsigned int i;
  int addrlen = sizeof(struct full_sockaddr_ax25);
  struct full_sockaddr_ax25 axbind;

  strcpy(name, "ax25k_init");
  if (! sockinit)
  {
    sockinit++;
    for (i = 0; i < MAXSOCK; i++) so[i].handle = EOF;
  }
  if (! strcasecmp(m.ax25k_if, "off"))
  {
    trace(report, name, "Kernel AX25 disabled");
    return 0;
  }
  axbind.fsa_ax25.sax25_family = AF_AX25;
  axbind.fsa_ax25.sax25_ndigis = 1;
  unsigned int j = 0, k = 0;
  // remove trailing spaces - would be interpreted as "empty" interfaces
  i = strlen(m.ax25k_if);
  while (i && m.ax25k_if[i - 1] == ' ') i--;
  m.ax25k_if[i] = 0;
  for (i = 0; i <= strlen(m.ax25k_if); i++)
  {
    if (m.ax25k_if[i] != ' ' && m.ax25k_if[i])
      port[j++] = m.ax25k_if[i];
    else
    {
      port[j] = 0;
      trace(report, name, "Initializing Interface %s", port);
      if (   m.ax25k_if[i] == ' ' && toupper(m.ax25k_if[i + 1]) == 'D'
          && (m.ax25k_if[i + 2] == ' ' || ! m.ax25k_if[i + 2]))
      {
        ax25k_default = 1;
        strcpy(ax25k_default_port, port);
        i += 2;
      }
      if (convert_call_entry(port, axbind.fsa_digipeater[0].ax25_call) == -1)
      {
        trace(serious, name, "invalid AX.25 interface callsign - %s",
                             m.ax25k_if);
        return 0;
      }
      for (j = 0; j < m.mycalls; j++)
      {
        if (convert_call_entry(m.mycall[j], axbind.fsa_ax25.sax25_call.ax25_call) == -1)
        {
          trace(serious, name, "invalid callsign - %s", m.mycall[j]);
          return 0;
        }
        if ((listen_sock[k] = socket(AF_AX25, SOCK_SEQPACKET, 0)) < 0)
        {
          trace(serious, name, "cannot open AX.25 socket, %s",
                               strerror(errno));
          return 0;
        }
        if (fcntl(listen_sock[k], F_SETFL, fcntl(listen_sock[k], F_GETFL) | O_NONBLOCK) < 0)
        {
          trace(serious, name, "fcntl: %s", strerror(errno));
          close(listen_sock[k]);
          return 0;
        }
        if (bind(listen_sock[k], (struct sockaddr *) &axbind, addrlen))
        {
          trace(serious, name, "cannot bind AX.25 socket, %s",
                               strerror(errno));
          close(listen_sock[k]);
          return 0;
        }
        if (listen(listen_sock[k], 5))
        {
          trace(serious, name, "cannot listen to AX.25 socket, %s",
                               strerror(errno));
          close(listen_sock[k]);
          return 0;
        }
        k++;
      }
      j = 0;
    }
  }
  ax25k_ok = 1;
  listen_sock_num = k;
  trace(report, name, "Kernel AX25 enabled");
  if (ax25k_default)
    trace(report, name, "Using Default Interface %s", ax25k_default_port);
  return 1;
}

/*---------------------------------------------------------------------------*/

void ax25k_shutdown ()
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;
  if (! ax25k_ok) return;
  for (i = 0; i < listen_sock_num; i++) close(listen_sock[i]);
  for (i = 0; i < MAXSOCK; i++)
  {
    if (so[i].handle != EOF)
    {
      //trace(serious, "ax25k_shutdown", "open socket %d %d", i, so[i].handle);
      close(so[i].handle);
    }
  }
}

/*---------------------------------------------------------------------------*/

#endif
