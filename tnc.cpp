/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------
  Interface zur TNC-Ansteuerung
  -----------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

                      Deti Fliegl, deti@gmx.de
                      01-Mar-98 Added Linux AX25-kernel support

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

//19980311 DG9MHZ changes for support of Linux Kernel AX25
//19980428 OE3DZW added OS and V-number to stop-msg
//19991028 Jan    added ax25k_only

/*---------------------------------------------------------------------------*/

int l2vorhanden = 0;

/*---------------------------------------------------------------------------*/
#ifndef _AX25K_ONLY

#include "l2_appl.h"
#include "l2host.h"
#include "layer2.h"

// Needed to get the Port MYCALLs
extern task_t *tfeld[TASKS];

typedef enum
{
  p_closed,
  p_open,
  p_linksetup,
  p_connected,
  p_session,
  p_discrequest,
  p_disconnected
} PORTTYP;

/*---------------------------------------------------------------------------*/

static int highestport = 0;

/*---------------------------------------------------------------------------*/

static unsigned int txfbuf[MAXTNCPORTS];
static char rxcbuf[MAXTNCPORTS];
static char rxdafl[MAXTNCPORTS];
static char portopen[MAXTNCPORTS];
static char wasput[MAXTNCPORTS];

/*---------------------------------------------------------------------------*/

void set_paclen_tnc (unsigned short paclen)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
    set_stream(t->port);
    set_paclen(paclen);
  }
}

/*---------------------------------------------------------------------------*/

static int near holtnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
    set_stream(t->port);
    return tnc_get();
  }
  else
  {
    wdelay(341);
    return CR;
  }
}

/*---------------------------------------------------------------------------*/

int putfree_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
#ifdef _AX25K
  if (t->processkind & P_AX25K) return ax25k_putfree();
#endif
  set_stream(t->port);
  return tnc_putfree();
}

/*---------------------------------------------------------------------------*/

int freeblocks_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  return 255;              // get_free();
}

/*---------------------------------------------------------------------------*/

void putv_tnc (int c)
//**************************************************************************
//
//
//**************************************************************************
{
  int port = t->port;
  static unsigned int bytes = 0;

#ifdef _AX25K
  if (t->processkind & P_AX25K)
  {
    ax25k_putv(c);
    return;
  }
#endif
  wasput[port] = 1;
  if (l2vorhanden)
  {
    if (! txfbuf[port])
    {
      while (! putfree_tnc()) wdelay(268);
      txfbuf[port] = 900;
    }
    if (! (bytes & 2047)) while (freeblocks_tnc () < 20) wdelay(367);
    else bytes = 0;
    set_stream(port);
    tnc_put(c);
    txfbuf[port]--;
    bytes++;
  }
}

/*---------------------------------------------------------------------------*/

void putflush_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  b->pacbytes = 0;
#ifdef _AX25K
  if (t->processkind & P_AX25K)
  {
    ax25k_flush();
    return;
  }
#endif
  set_stream(t->port);
  tnc_flush();
}

/*---------------------------------------------------------------------------*/

static void near openport_tnc (int port)
//**************************************************************************
//
//
//**************************************************************************
{
  set_stream(port);
  set_conok(1);
}

/*---------------------------------------------------------------------------*/

static void near closeport_tnc (int port)
//**************************************************************************
//
//
//**************************************************************************
{
  set_stream(port);
  set_conok(0);
}

/*---------------------------------------------------------------------------*/

static void near discport_tnc (int port)
//**************************************************************************
//
//
//**************************************************************************
{
  set_stream(port);
  tnc_disconnect(2);
}

/*---------------------------------------------------------------------------*/

static void near connport_tnc (int port, char *mycall, char *conncall)
//**************************************************************************
//
//
//**************************************************************************
{
  set_stream(port);
  if (mycall)
    tnc_connect(mycall, conncall);
  else
    tnc_connect(NULL, conncall);
}

/*---------------------------------------------------------------------------*/
#ifdef _TNC
void control_tnc (char *befehl)
//**************************************************************************
//
//
//**************************************************************************
{                              // *** dummy!
  putf("L2: %s\n", befehl);
}
#endif
/*---------------------------------------------------------------------------*/

void disable_tnc (void)
//*************************************************************************
//
//*************************************************************************
{
}     // *** dummy!

/*---------------------------------------------------------------------------*/

static int near linkstate_tnc (int port)
//**************************************************************************
//
//
//**************************************************************************
{
  set_stream(port);
  return get_lstate();
}

/*---------------------------------------------------------------------------*/

static void near linkcall_tnc (int port, char *buf)
//**************************************************************************
//
//
//**************************************************************************
{
  char callbuf[80];
  unsigned int i = 0;

  set_stream(port);
  strcpy(callbuf, get_concall());
  strupr(callbuf);
  while (callbuf[i] > ' ') *buf++ = callbuf[i++];
  *buf = 0;
  if (! callbuf[i]) return;
  i = strlen (callbuf);
  while (i && callbuf[i - 1] > ' ') i--;
  if (i)
  {
    *buf++ = ' ';
    while (callbuf[i] > ' ') *buf++ = callbuf[i++];
    *buf = 0;
  }
}

/*---------------------------------------------------------------------------*/

void putbeacon_tnc (char *adressfeld, char *bakentext)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
#ifdef _AX25K
    CRegEx reg;
    if (ax25k_isdefault() || reg.regex_match("ax.*:", adressfeld))
    {
      for (unsigned int i = 0; bakentext[i]; i++)
        if (bakentext[i] == LF) bakentext[i] = CR;
      ax25k_putbeacon(adressfeld, bakentext);
      return;
    }
#endif
    set_stream(0);
    tnc_connect(NULL, adressfeld);
    for (unsigned int i = 0; bakentext[i]; i++)
    {
      if (bakentext[i] == LF) bakentext[i] = CR;
      tnc_put(bakentext[i]);
    }
    tnc_flush();
  }
}

/*---------------------------------------------------------------------------*/

int init_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  unsigned int i;
  int highport = tnc_init(MAXTNCPORTS);
  if (highport > 0)
  {
    l2vorhanden = 1;
    for (i = 0; i < MAXTNCPORTS; i++)
    {
      portopen[i] = p_closed;
      rxdafl[i] = 0;
      txfbuf[i] = 0;
    }
    trace(report, "init_tnc", "%d ports", highport - 1);
    return highport;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

int getvalid_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
#ifdef _AX25K
    if (t->processkind & P_AX25K) return ax25k_getvalid_tnc();
#endif
    int tmp, port = t->port;
    if (rxdafl[port])
    {
      if (rxcbuf[port] == CR) return CR;
      return OK;
    }
    if ((tmp = holtnc()) != EOF)
    {
      rxcbuf[port] = tmp;
      rxdafl[port] = 1;
      if (tmp == CR) return CR;
      return OK;
    }
    return NO;
  }
  else return NO;
}

/*---------------------------------------------------------------------------*/

int getv_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
#ifdef _AX25K
    if (t->processkind & P_AX25K) return ax25k_getv();
#endif
    int port = t->port;
    int tmp;
    if (rxdafl[port])
    {
      rxdafl[port] = 0;
      return rxcbuf[port];
    }
    while (1)
    {
      if ((tmp = holtnc()) != EOF) return tmp;
      wdelay(242);
//      wdelay(24);
    }
  }
  else return CR;
}

/*---------------------------------------------------------------------------*/

void getclear_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
#ifdef _AX25K
    if (t->processkind & P_AX25K)
    {
      ax25k_getclear();
      return;
    }
#endif
    while (holtnc() != EOF); // RX-Puffer ausleeren
  }
}

/*---------------------------------------------------------------------------*/

void getmycall_tnc (char *outcall)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
#ifdef _AX25K
    if (t->processkind & P_AX25K)
    {
      ax25k_getmycall(outcall);
      return;
    }
#endif
    set_stream(t->port);
    char *mycall = get_mycall();
    if (mycall) strcpy(outcall, mycall);
  }
  else *outcall = 0;
}

/*---------------------------------------------------------------------------*/

void align_mycalls (void)
//**************************************************************************
//
//
//**************************************************************************
{
  char mycallstr[120];

  strcpy(mycallstr, "");
  if (l2vorhanden)
  {
    unsigned int i;
    for (i = 0; i < m.mycalls; i++)
    {
      strcat(mycallstr, m.mycall[i]);
      strcat(mycallstr, " ");
    }
    for (i = 0; i < m.macro_mycalls; i++)
    {
      strcat(mycallstr, m.macro_mycall[i]);
      strcat(mycallstr, " ");
    }
    set_mycall(mycallstr);
  }
}

/*---------------------------------------------------------------------------*/

void inputline_tnc (char *s, int maxlen, char cut)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
    unsigned int i = 0;
    unsigned int semi = NO;

#ifdef _AX25K
    if (t->processkind & P_AX25K)
    {
      ax25k_inputline(s, maxlen, cut);
      return;
    }
#endif
    if (maxlen < 0)
    {
      //if (cut) semi = OK; db7mh bug ... ich will ja NICHT nur bis zum ersten ; lesen ...
      maxlen = (-maxlen);
    }
    if (wasput[t->port] && ! getvalid_tnc()) putflush_tnc();
    while (1)
    {
      if ( /*! cut &&*/ i == (unsigned) maxlen) break;
      do s[i] = getv_tnc();
      while (s[i] == LF || ! s[i]); // ignore LFs and 0-bytes
      if (semi && s[i] == ';') break;
      if (s[i] == CR)
      {
        if (! cut) s[i++] = LF;
        break;
      }
      if (i < (unsigned) maxlen) i++;
    }
    s[i] = 0;
  }
  else
  {
    *s = 0;
    wdelay(500);
  }
/*
  if (u->comp == 1)
  {
    char output2[256] = { 0 };
    int il = 0;

    il = decomp_sp_stat_huff(s, strlen(s), output2);
    printf("Out2:%d\n-%s-\n",il,output2);
    strcpy(s,output2);
    //s[il] = 0;
  }
*/
//printf("line: %s\n",s);
}

/*---------------------------------------------------------------------------*/

void mbdisconnect (int wait)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
    int port = t->port;
#ifdef _AX25K
    if (t->processkind & P_AX25K)
    {
      ax25k_disconnect(wait);
      return;
    }
#endif
    if (port > 0)
    {
      discport_tnc(port);
      if (wait)
      {
        portopen[port] = p_discrequest;
        while (linkstate_tnc(port) != disconnected) wdelay(244);
        portopen[port] = p_closed;
      }
      getclear_tnc(); // db7mh: Workarround um den Buffer zu leeren
    }
  }
}

/*---------------------------------------------------------------------------*/

int loginpnum, nextopen = 0, portsopen = 0;

/*---------------------------------------------------------------------------*/

void putversion_tnc (void)
//**************************************************************************
//
//
//**************************************************************************
{
  if (l2vorhanden)
  {
    if (highestport > 0) putf("OpenBCM-L2 %d ports"
#ifdef _AX25K
            ", Linux Kernel AX25"
#endif
            "\n", highestport - 1);
  }
  else putf("-\n");
}

/*---------------------------------------------------------------------------*/

void putl2info (void)
//**************************************************************************
//
// Gibt eine Info zu verbundenen Layer2-Port aus
//
//**************************************************************************
{
  port_t *pp;
#ifdef _AX25K
//  sock_t *ss;
#endif
  task_t *tt;
  int pnummer;
  int i, found;
  lastfunc("putl2info");
  if (l2vorhanden)
  {
    putf("OpenBCM-L2 %d ports\n\n", highestport-1);
    putf("Layer2 parameter of connected ports:\n");
    for (pnummer = 0; pnummer <= (highestport-1); pnummer++)
      if ((pp = pfeld[pnummer]) != NULL && ! pp->lstate == disconnected)
      {
        putf("Port %-3d: ", pnummer);
        found = false;
        for (i = 0; i < TASKS; i++)
        {
          tt = tfeld[i];
          if (tt)
          {
            tvar_t *bb = (tvar_t *) tt->b;
            if (bb)
              if (tt->port == pnummer)
              {
                found = true;
                putf("%-9.9s via %-9.9s\n", tt->name, bb->uplink);
              }
          }
        }
        if (! found) putf("\n");
        putf("          Frack  %-4d ", pp->frack);
        putf("Paclen %-4d ", pp->paclen);
        putf("Timer1 %-4d ", pp->timer1);
        putf("Timer2 %-4d ", pp->timer2);
        putf("Timer3 %-4d\n", pp->timer3);
        putf("          ITimer %-4d ", pp->itimer);
        putf("QTimer %-4d ", pp->qtimer);
        putf("Retries %-3d ", pp->retries);
        putf("Roundtrip %d ", pp->roundtrip);
        putf("DiscTimer %d\n", pp->disctimer);
      }
    putf("\n");
  }
#ifdef _AX25K
/*  if (ax25k_ok)
  {
    putf("OpenBCM Linux Kernel AX25\n\n");
    putf("Kernel AX25 parameter of connected ports:\n");
    for (pnummer = 0; pnummer <= (highestport-1); pnummer++)
      if ((ss = so[pnummer]))
      {
            putf("Port %-3d: ",pnummer);
        found = false;
        for (i = 0; i < TASKS; i++)
        { tt = tfeld[i];
          if (tt)
          { tvar_t *bb = (tvar_t *) tt->b;
            if (bb)
              if (tt->port == pnummer)
              {
                found = true;
                putf("%-9.9s via %-9.9s\n", tt->name, bb->uplink);
              }
          }
        }
        if (! found) putf("\n");
      }
  }
*/
#endif
}

/*---------------------------------------------------------------------------*/


//void monwindow (char *name)
//**************************************************************************
//
// monitoring into dummy output .. nice feature :)
//
//**************************************************************************
/*
{
  if (l2vorhanden)
  {
    unsigned c = 0, d = LF;

    e->x = 0;
    e->xend = 30;
    e->y = 1;
    e->yend = bildzeilen / 3;

    preparewin(putupdate, name, 50 * bildzeilen);
    putupdate(1);

    t->port = 0;

    while (! runterfahren)
    {
      if (c)
      {
#ifdef __FLAT__
        putchar (c);
#endif
        putv (d);
        d = c;
      }
      waitfor(e_ticsfull);
      c = (char) getv_tnc();
      if (c == 7)
        c = '#';
      if (c >= (char) 0xfd)
      {
        if (c == (char) 0xff)
          c = LF;
        else
          c = 0;
      }
      if (c == CR)
        c = LF;
    }
    wclose();
  }
} */

#endif
/*---------------------------------------------------------------------------*/

void setsession (void)
//**************************************************************************
//
//
//**************************************************************************
{
  t->input = io_tnc;
  t->output = io_tnc;

#ifndef _AX25K_ONLY
  if (l2vorhanden)
  {
    if ((t->port > 0) && (portopen[t->port] == p_connected))
      portopen[t->port] = p_session;
  }
#endif
}


/*---------------------------------------------------------------------------*/

int makeconnect (char *mycall, char *conncall)
//**************************************************************************
//
//
//**************************************************************************
{
  lastfunc("makeconnect");
  unsigned int port;
  //unsigned int port_org;

  if (l2vorhanden)
  {
    strupr(conncall);
    strupr(mycall);
#ifdef _AX25K
#ifndef _AX25K_ONLY
    CRegEx reg;
    if (ax25k_isdefault() || reg.regex_match("ax.*:", conncall))
#endif
      return ax25k_connect(mycall, conncall);
#endif
#ifndef _AX25K_ONLY
    if (t->port)
    {
      trace(serious, "makeconn", "port busy");
      return NO;
    }
    for (port = 2; port < (unsigned) highestport; port++)
    {
  //    port_org = port;
  //    port = random_max(highestport-port_org)+port_org-1;
      if (portopen[port] == p_closed)
      {
        portopen[port] = p_linksetup;
        rxdafl[port] = rxcbuf[port] = 0;
        connport_tnc(port, mycall, conncall);
        while (portopen[port] == p_linksetup)
        {
          switch (linkstate_tnc(port))
          {
          case info_transfer:
          case rej_sent:
          case wait_ack:
            t->port = port;
            portopen[port] = p_connected;
            trace(report, "makeconn", "ok %s", conncall);
            break;
          case disconnected:
            portopen[port] = p_closed;
            trace(report, "makeconn", "fail %s", conncall);
            break;
          default:
            {
              wdelay(245);
              if (testabbruch())
              {
                portopen[port] = p_closed;
                discport_tnc(port);
                return NO;
              }
            }
          }
        }
        return (portopen[port] == p_connected);
      }
  //    port = port_org;
    }
    trace(report, "makeconn", "no port");
    return NO;
#endif
  }
  else
    return NO;
}

/*---------------------------------------------------------------------------*/

void logindaemon (char *name)
//**************************************************************************
//
//
//**************************************************************************
{
#ifndef _AX25K_ONLY
  highestport = init_tnc();
  wdelay(548);
  while (! runterfahren)
  {
    nextopen = 0;
    portsopen = 0;
    for (loginpnum = 1; loginpnum < highestport; loginpnum++)
    {
      switch (portopen[loginpnum])
      {
      case p_closed:
        if (! nextopen) nextopen = loginpnum;
        break;
      case p_open:
        if (linkstate_tnc(loginpnum) >= info_transfer)
        {
          char cstate[70];
          linkcall_tnc(loginpnum, cstate);
          cstate[30] = 0;
          portopen[loginpnum] = p_session;
          closeport_tnc(loginpnum);
          rxdafl[loginpnum] = rxcbuf[loginpnum] = 0;
          fork(P_MAIL, loginpnum, boxusertask, cstate);
        }
        else portsopen++;
        break;
      case p_session:
        if (linkstate_tnc(loginpnum) == disconnected)
        {
          portopen[loginpnum] = p_closed;
          killport(loginpnum);
          doodle(3);
        }
        break;
      case p_connected:
        if (linkstate_tnc(loginpnum) == disconnected)
        {
          portopen[loginpnum] = p_closed;
          taskport0(loginpnum);
        }
        break;
      }
    }
    if (nextopen > 0 && portsopen < 3)
    {
#ifdef __MSDOS__
      if (coreleft() > 20000L) // nur wenn noch genug Speicher frei ist
#endif
      {
        openport_tnc(nextopen); // einen neuen Login zulassen
        portopen[nextopen] = p_open;
      }
    }
#ifdef _AX25K
    ax25k_checklogin();
#endif
    wdelay(246);
  }
#else  //_AX25K_ONLY
  wdelay(548);
  while (! runterfahren)
  {
    ax25k_checklogin();
    wdelay(246);
  }
#endif
  mbtalk("SYSTEM", "ALL", "Mailbox shutdown.");
  trace(replog, "logind", "stop V" VNUMMER " " OPSYSTEM " (%s)", stopreason);
  wdelay(447);
#ifndef _AX25K_ONLY
//  int loginpnum;
  for (loginpnum = 1; loginpnum < highestport; loginpnum++)
    closeport_tnc(loginpnum);
  for (loginpnum = 1; loginpnum < highestport; loginpnum++)
  {
    if ((portopen[loginpnum] == p_session)
        || (portopen[loginpnum] == p_connected))
      killport(loginpnum);
  }
  for (loginpnum = 1; loginpnum < highestport; loginpnum++)
    discport_tnc(loginpnum);
#endif
#ifdef _AX25K
    ax25k_shutdown();
#endif
/*  // tracewin() does that
    for (loginpnum = TASKS; loginpnum; loginpnum--)  //neu
    kill(loginpnum);      //neu
*/
  wdelay(347);
  init_off = 1;
}

/*---------------------------------------------------------------------------*/
