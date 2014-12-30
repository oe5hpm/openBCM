/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------
  Interface zur FlexNet-Ansteuerung
  ---------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980120 OE3DZW added cut parameter in inputline_tnc
//19980202 DG9MHZ fixed cut-parameter, R-lines should be ok again
//19980301 OE3DZW added ax25_set_pid
//19980408 OE3DZW limited ssid to 16
//19980428 OE3DZW added OS and V-number to stop-msg
//19980514 hrx    added last repeater before destination

#include "baycom.h"
#include "flexappl.h"

/*---------------------------------------------------------------------------*/

typedef enum
{ p_closed,
  p_open,
  p_linksetup,
  p_connected,
  p_session,
  p_discrequest,
  p_disconnected
} portstate_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ l2_idle = 0,
  l2_disconnected = 1,
  l2_link_setup = 2,
  l2_frmr = 3,
  l2_disc_request = 4,
  l2_info_transfer = 5,
  l2_rej_sent = 6,
  l2_wait_ack = 7
} linkstate_t;

/*---------------------------------------------------------------------------*/

static int l2vorhanden = 0;
static int highestport = 0;
static int flex = 0;
static TRACE tr;

/*---------------------------------------------------------------------------*/

typedef struct boxqso_t
{ unsigned tx_fbuf;
  unsigned qsonum;
  char portopen;
  char wasput;
  unsigned rx_index;
  const INFO far *rx;
} boxqso_t;

/*---------------------------------------------------------------------------*/

static boxqso_t qso[MAXTNCPORTS];
static char rxcbuf[MAXTNCPORTS];
static char rxdafl[MAXTNCPORTS];

/*---------------------------------------------------------------------------*/

static int near holtnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2vorhanden)
  {
    boxqso_t *q = &qso[t->port];
    char c;

    if (q->rx == NULL || q->rx_index == q->rx->len)
    {
      q->rx = l2_get_i(q->qsonum);
      q->rx_index = 0;
    }
    if (q->rx)
    {
      c = q->rx->text[q->rx_index++];
      if (q->rx_index == q->rx->len)
      {
        l2_i_ack(q->qsonum);
        q->rx = NULL;
      }
      return c;
    }
    else return EOF;
  }
  else
  {
    wdelay(341);
    return CR;
  }
}

/*---------------------------------------------------------------------------*/

int putfree_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  boxqso_t *q = &qso[t->port];

  if (q->tx_fbuf == 0 && l2_ialloc(q->qsonum, 256))
    q->tx_fbuf = 256;
  return q->tx_fbuf;
}

/*---------------------------------------------------------------------------*/

int freeblocks_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  return 255; // bei Flexnet nicht nutzbar
}

/*---------------------------------------------------------------------------*/

void putv_tnc (int c)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2vorhanden)
  {
    boxqso_t *q = &qso[t->port];
    q->wasput = 1;

    while (q->tx_fbuf == 0)
    {
      if (putfree_tnc())
        break;
      wdelay(167);
    }
    l2_send_char(q->qsonum, c);
    q->tx_fbuf--;
  }
}

/*---------------------------------------------------------------------------*/

void putflush_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  boxqso_t *q = &qso[t->port];

  b->pacbytes = 0;
  q->wasput = 0;
  q->tx_fbuf = 0;
  l2_pack(q->qsonum);
}

/*---------------------------------------------------------------------------*/

static void near discport_tnc (int port)
//*************************************************************************
//
//
//*************************************************************************
{
  boxqso_t *q = &qso[port];

  l2_stop_qso(q->qsonum);
}

/*---------------------------------------------------------------------------*/

void control_tnc (char *befehl)
//*************************************************************************
//
//
//*************************************************************************
{
  static char *beftab[] =
  { "TRACE", "TXDELAY", "MODE",
    NULL
  };

  enum befnum
  { unsinn, trace, txdelay, mode
  } cmd = unsinn;

  cmd = (befnum) readcmd(beftab, &befehl, 0);
  switch (cmd)
  {
    case trace:
      if (isdigit(befehl[0]) || befehl[0] == '-')
      {
        tr.ch_mask = 0;
        if (befehl[0] == '-')
          tr.ch_mask = 0xffff;
        else
          tr.ch_mask = 1 << atoi(befehl);
        tr.trxfilter = 3;
        putf("TRACE 0x%04X\n", tr.ch_mask);
      }
      else
      {
        tr.trxfilter = 0;
        putf("TRACE off\n");
      }
      break;
    case unsinn:
    default:
      putf("Syntax: tnc trace <value>\n");
      break;
  }
}

/*---------------------------------------------------------------------------*/

static void get_newconnect (char *mycall, int port, char *callbuf)
//*************************************************************************
//
//
//*************************************************************************
{
  boxqso_t *q = &qso[port];
  int pnum;

  q->qsonum = l2_get_sabm((byte *)mycall);
  if (q->qsonum)
  {
    if (port == 0)
    {
      l2_sabm_dm(q->qsonum);
      q->qsonum = 0;
      return;
    }
    else
    {
      const FRAME far *fr = l2_get_f(q->qsonum);
      strcall((byte *)callbuf, (byte *)fr->dest);
      if (fr->digis)
      {
        strcat(callbuf, " ");
        strcall((byte *)callbuf+strlen(callbuf), fr->digi[fr->digis-1]);
        strcat(callbuf, " ");
        strcall((byte *)callbuf+strlen(callbuf), fr->digi[0]);
      }
      l2_sabmresp(q->qsonum);
      if ((l2_state(q->qsonum)&7) <= l2_disconnected)
      {
        q->qsonum = 0;
        callbuf[0] = 0;
        return;
      }
      // Nachschaun, ob dasselbe QSO schon woanders existiert. Wenn ja, dann
      // keine neue Verbindung aufmachen, sondern nur UA sagen und sonst nix
      // tun.
      for (pnum = 1 ; pnum < highestport ; pnum++)
      {
        if (pnum != port && qso[pnum].qsonum == q->qsonum)
        {
          q->qsonum = 0;
          callbuf[0] = 0;
          break;
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void putbeacon_tnc (char *adressfeld, char *bakentext)
//*************************************************************************
//
//
//*************************************************************************
{
  char adr2[80];
  char *dest;

  strcpy(adr2, adressfeld);
  if (l2vorhanden)
  {
    dest = skip(adr2);
    l2_send_ui((byte *)adr2, (byte *)dest,
               0xf0, 0, strlen(bakentext), (byte *)bakentext);
  }
}

/*---------------------------------------------------------------------------*/

static void exit_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2_chk_monitor())
    l2_clr_monitor();
  ax_exit();
}

/*---------------------------------------------------------------------------*/

static int init_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int i;

  flex = ax_init();
  if (flex)
  {
    l2vorhanden = 1;
    atexit(exit_tnc);
    for (i = 0 ; i < MAXTNCPORTS ; i++)
    {
      boxqso_t *q = &qso[i];
      memset(q, 0, sizeof(boxqso_t));
      rxdafl[i] = 0;
    }
    trace(report, "init_tnc", "FlexNet %s", l2_version());
    return MAXTNCPORTS-2;
  }
  l2vorhanden = 0;
  return 0;
}

/*---------------------------------------------------------------------------*/

int getvalid_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int tmp, port = t->port;

  if (l2vorhanden)
  {
    if (rxdafl[port])
    {
      if (rxcbuf[port] == CR)
        return CR;
      return 1;
    }
    if ((tmp = holtnc()) != EOF)
    {
      rxcbuf[port] = tmp;
      rxdafl[port] = 1;
      if (tmp == CR)
        return CR;
      return 1;
    }
    return 0;
  }
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

int getv_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int port = t->port;
  int tmp;

  if(rxdafl[port])
  {
    rxdafl[port] = 0;
    return rxcbuf[port];
  }
  while (1)
  {
    tmp = holtnc();
    if (tmp != EOF)
      return tmp;
    wdelay(242);
  }
}

/*---------------------------------------------------------------------------*/

void inputline_tnc (char *s, int maxlen, char cut)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = 0;
  int semi = 0;

  if (maxlen < 0)
  {
    if (cut) semi = 1;
    maxlen = (-maxlen);
  }
  if (qso[t->port].wasput && ! getvalid_tnc()) putflush_tnc();
  while (1)
  {
    if (/*! cut &&*/ i == maxlen) break;
    do s[i] = getv_tnc ();
    while (s[i] == LF || ! s[i]); // ignore LFs and 0-bytes
    if (semi && s[i] == ';') break;
    if (s[i] == CR)
    {
      if (! cut) s[i++] = LF;
      break;
    }
    if (i < maxlen) i++;
  }
  s[i] = 0;
}

/*---------------------------------------------------------------------------*/

void align_mycalls (void)
//*************************************************************************
//
//
//*************************************************************************
{
  // Dummy due to compatibiliy with BayCom-L2
}

/*---------------------------------------------------------------------------*/

void getmycall_tnc (char *outcall)
//*************************************************************************
//
//
//*************************************************************************
{
  int port = t->port;
  boxqso_t *q = &qso[port];

  if (port > 0)
  {
    const FRAME far *fr = l2_get_f(q->qsonum);
    strcall((byte *)outcall, (byte *)fr->source);
  }
  else
    outcall[0] = 0;
}

/*---------------------------------------------------------------------------*/

void setsession (void)
//*************************************************************************
//
//
//*************************************************************************
{
  t->input = io_tnc;
  t->output = io_tnc;
  if (t->port > 0)
  {
    boxqso_t *q = &qso[t->port];
    if (q->portopen == p_connected)
      q->portopen = p_session;
  }
}

/*---------------------------------------------------------------------------*/

void getclear_tnc(void)
//*************************************************************************
//
//
//*************************************************************************
{
  while (holtnc() != EOF); // RX-Puffer ausleeren
}

/*---------------------------------------------------------------------------*/

int makeconnect(char *mycall, char *conncall)
//*************************************************************************
//
//
//*************************************************************************
{
  int port;
  char tmp_mycall[12];
  char tmp_mycall2[12];
  int count = 0;
  int qsonum = 0;
  char *str;

  strupr(conncall);
  strupr(mycall);
  strcpy(tmp_mycall, mycall);
  if (! l2vorhanden)
    return 0;
  while ((qsonum = l2_check_path((byte *)tmp_mycall, (byte *)conncall)) != 0)
  {
    trace(report, "incssid", "%s>%s", tmp_mycall, conncall);
    str = strchr(tmp_mycall, '-');
    if (count++ > 15)
      return 0;
    if (l2_state(qsonum)<2)
    {
      l2_kill_qso(qsonum);
      break;
    }
    if (str)
    {
      int ssid = atoi(str+1);
      str[0] = 0;
      ssid = (ssid+1) % 16; //limit ssid to 15
      sprintf(tmp_mycall2, "%s-%d", tmp_mycall, ssid);
    }
    else
      sprintf(tmp_mycall2, "%s-1", tmp_mycall);
    strcpy(tmp_mycall, tmp_mycall2);
  }
  if (t->port)
  {
    trace(serious, "makeconn", "port busy");
    return 0;
  }
  for (port=2; port < highestport; port++)
  {
    boxqso_t *q = &qso[port];
    if (q->portopen == p_closed)
    {
      q->portopen = p_linksetup;
      q->tx_fbuf = 0;
      q->wasput = 0;
      q->rx = NULL;
      rxdafl[port] = rxcbuf[port] = 0;
      q->qsonum = l2_connect((byte *)tmp_mycall, (byte *)conncall);
      while (q->portopen == p_linksetup)
      {
        switch (l2_state(q->qsonum) & 7)
        {
          case l2_info_transfer:
          case l2_rej_sent:
          case l2_wait_ack:
            t->port = port;
            q->portopen = p_connected;
            trace(report, "makeconn", "ok %s", conncall);
            break;
          case l2_idle:
          case l2_disconnected:
            q->portopen = p_closed;
            q->qsonum = 0;
            trace(report, "makeconn", "fail %s", conncall);
            break;
          default:
          {
            wdelay(245);
            if (testabbruch())
            {
              q->portopen = p_closed;
              q->qsonum = 0;
              discport_tnc(port);
              return 0;
            }
          }
        }
      }
      return(q->portopen == p_connected);
    }
  }
  trace(report, "makeconn", "no port");
  return 0;
}

/*---------------------------------------------------------------------------*/

void mbdisconnect (int wait)
//*************************************************************************
//
//
//*************************************************************************
{
  int port = t->port;
  boxqso_t *q = &qso[port];

  if (port > 0)
  {
    discport_tnc(port);
    if (wait)
    {
      q->portopen = p_discrequest;
      while (q->qsonum)
      {
        linkstate_t ls = (linkstate_t)(l2_state(q->qsonum) & 7);
        if (ls == l2_disconnected || ls == l2_idle)
          break;
        wdelay(244);
      }
      q->portopen = p_closed;
      q->qsonum = 0;
    }
  }
}

/*---------------------------------------------------------------------------*/

void putversion_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2vorhanden)
    putf("FlexNet V%s %s\n", (char *) l2_version(),
                             ((flex==1) ? "mini" : "digi"));
  else
    putf("-\n");
}

/*---------------------------------------------------------------------------*/

void disable_tnc (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int loginpnum;
  for (loginpnum = 1 ; loginpnum < highestport ; loginpnum++)
  {
    boxqso_t *q = &qso[loginpnum];
    if (q->portopen != p_closed)
      l2_bef(18, q->qsonum);
  }
}

/*---------------------------------------------------------------------------*/

void ax25_set_pid (unsigned short int qsonum, char pid)
//*************************************************************************
//
//
//*************************************************************************
{
  l2_set_pid(qsonum, pid);
}
/*---------------------------------------------------------------------------*/

void logindaemon (char *)
//*************************************************************************
//
//
//*************************************************************************
{
  int loginpnum;
  int mycallnum = 0;

  highestport = init_tnc();
  if (! highestport)
  {
    trace(report, "logind", "l2 absent");
    return;
  }
  while (! runterfahren)
  {
    mycallnum = 0;
    for (loginpnum = 1 ; loginpnum < highestport ; loginpnum++)
    {
      boxqso_t *q = &qso[loginpnum];
      switch (q->portopen)
      {
        case p_closed:
          if (mycallnum < m.mycalls)
          {
            char cstate[70];
            get_newconnect(m.mycall[mycallnum],
                           coreleft() > 50000L ? loginpnum : 0, cstate);
            cstate[30] = 0;
            if(q->qsonum)
            {
              q->portopen = p_session;
              q->tx_fbuf = 0;
              q->wasput = 0;
              q->rx = NULL;
              rxdafl[loginpnum] = rxcbuf[loginpnum] = 0;
              fork(P_MAIL, loginpnum, boxusertask, cstate);
            }
            else
            {
              mycallnum++;
              q->qsonum = 0;
            }
          }
          break;
        case p_session:
          if ((l2_state(q->qsonum) & 7) <= l2_disconnected)
          {
            q->portopen = p_closed;
            q->qsonum = 0;
            killport(loginpnum);
            doodle(3);
          }
          break;
        case p_connected:
          if ((l2_state(q->qsonum) & 7) <= l2_disconnected)
          {
            q->portopen = p_closed;
            q->qsonum = 0;
            taskport0(loginpnum);
          }
          break;
      }
    }
    wdelay(66);
  }
  mbtalk("SYSTEM", "ALL", "Mailbox shutdown.");
  trace(replog, "logind", "stop V"VNUMMER" "OPSYSTEM" (%s)", stopreason);
  wdelay(447);
  for (loginpnum = 1; loginpnum < highestport; loginpnum++)
  {
    boxqso_t *q = &qso[loginpnum];
    if ((q->portopen == p_session) || (q->portopen == p_connected))
      killport(loginpnum);
  }
  for (loginpnum = 1 ; loginpnum < highestport ; loginpnum++)
    discport_tnc(loginpnum);
  wdelay(347);
  init_off = 1;
}

/*---------------------------------------------------------------------------*/

void monwindow (char *name)
//*************************************************************************
//
//
//*************************************************************************
{
  int c = LF, i;
  char s[80];
  int on = 0;
  const FRAME *fr;

  e->x = 0;
  e->xend = 30;
  e->y = 1;
  e->yend = bildzeilen/3;
  memset(&tr, 0, sizeof(TRACE));
  preparewin(putupdate, name, 50*bildzeilen);
  putupdate(1);
  wdelay(1345);
  if (l2vorhanden)
  {
    while (! runterfahren)
    {
      if (on && tr.trxfilter == 0)
      {
        l2_clr_monitor();
        on = 0;
      }
      if (tr.trxfilter && (! on || l2_chk_monitor() == 0))
        on = l2_set_monitor(&tr);
      if (on)
      {
        fr = l2_get_monitor();
        if (fr)
        {
          strcall((byte *)s, fr->source);
          putf("\n%c%d %s", fr->tx?'T':'R', fr->kanal, s);
          for (i = 0 ; i < fr->digis ; i++)
          {
            strcall((byte *)s, fr->digi[i]);
            putf("/%s",s);
            if (i < fr->nextdigi) putv('*');
          }
          strcall((byte *)s, fr->dest);
          putf(">%s>", s);
          switch (fr->typ)
          {
            case I:    putf("I%d%d", fr->nr, fr->ns); break;
            case RR:   putf("RR%d", fr->nr);   break;
            case RNR:  putf("RNR%d", fr->nr);  break;
            case REJ:  putf("REJ%d", fr->nr);  break;
            case SABM: putf("SABM"); break;
            case DISC: putf("DISC"); break;
            case DM:   putf("DM");   break;
            case UA:   putf("UA");   break;
            case FRMR: putf("FRMR"); break;
            case UI:   putf("UI");   break;
            default:   putf("??");
          }
          if (fr->cmd) putf(fr->pf ? ",P" : ",C");
          else putf(fr->pf ? ",F" : ",R");
          if (fr->typ == I || fr->typ == UI) putf(",%02X", fr->pid);
          if (fr->dama) putf(",DAMA");
          i = 0;
          if ((fr->typ == I || fr->typ == UI)
              && (fr->pid == 0xcc || fr->pid == 0xcd))
          {
            i = 40;
            putf(" <TCP/IP>");
          }
          else
            if ((fr->textlen - i) > 40)
            {
              int binfl = 0, j;
              for (j = i; j < (i + 40); j++)
                if (fr->text[j] > 'z' || fr->text[j] < ' ') binfl++;
              if (binfl > 10)
              {
                putf("\n<BIN %d Bytes>", fr->textlen - i);
                i = fr->textlen;
              }
            }
          c = LF;
          for (; i < fr->textlen ; i++)
          {
            putv(c);
            c = fr->text[i];
            if (c == '\a') c = '#';
            if (c == CR) c = LF;
          }
          if (c != LF) putv(c);
          l2_ack_monitor();
          fr = NULL;
          waitfor(e_reschedule);
        }
        else wdelay(279);
      }
      else wdelay(299);
    }
    l2_clr_monitor();
  }
  else
  {
    while (! runterfahren) wdelay(548);
  }
  wclose();
  if (! l2vorhanden)
  {
    trace(replog, "l2sim", "stop V" VNUMMER " " OPSYSTEM " (%s)", stopreason);
    init_off = 1;
  }
}

/*---------------------------------------------------------------------------*/
