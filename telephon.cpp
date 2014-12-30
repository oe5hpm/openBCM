/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------------
  Serielle Ein- Ausgaberoutinen zum Telefonlogin (DOS)
  ----------------------------------------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/
//010100 JJ Telefonforward mit automatischer Wahl und Anrufentgegennahme

#include "baycom.h"

#ifdef _TELEPHONE

#define RXLEN   200
#define TXLEN   200
#define HISTLEN 10
static char     history[HISTLEN][60];
static int      histwrite = 0;
static char     rxbuf[RXLEN];
static char     txbuf[TXLEN];
static int      irqvec[5] =  {0,     4,     3,     4,     3};
static int      portadr[5] = {0, 0x3f8, 0x2f8, 0x3e8, 0x2e8};
static int      txin = 0, txout = 0, rxin = 0, rxout = 0, txint = 0,
                txdiff = 0, rxdiff = 0;
static int      ttyace;
static int      ttyirq;
static int      ttystop;
static int      ttybits;
static int      ttyport;
static int      ttyparity;
static unsigned ttybaud;
static int      ttyecho;
static int      ttykilled;
static int      echopreset;
static int      rxcbuf;
static int      rxdafl;
static int      txstopped;
static char     ttycall[20];     // rufzeichen auf ttyport
static int      ttytaskid;       // automatischer fwd von cron, tasknummer
static int      dialup;          // dialup aktiv
static int      cmdmode;         // modem im kommandomodus
static char     linespeed[10];   // Gschwindigkeit der Telefonverbindung
static time_t   onlinetime;      // onlinezeit
static int      dialcountdown;   // countdown bis zum naechsten dialup
static int      maxconcountdown; // countdown fuer connectanzahllimit
static int      concounter;      // connect zaehler fuer zeitraum maxconcountdown
static int      ttyactive=1;     // 0 tty frei
                                 // 1 tty busy (default, bis ttyport initialisiert)
                                 // 2 ring
                                 // 3 dialup
                                 // 4 connect, loginprompt oder autologin
                                 // 5 connect, eingelogt
static int      carrier;         // modem status
static int      ring;
static int      dsr;
static int      cts;
static int      dcarrier;
static int      tring;
static int      ddsr;
static int      dcts;
static int      reserved1;       // modem control
static int      reserved2;
static int      out0;
static int      out1;
static int      out2;
static int      loop;
static int      rts;
static int      dtr;

/*---------------------------------------------------------------------------*/

static char *beftab[]=
{ "COMADR", "COMIRQ", "CMDATZ", "CMDATA", "CMDATD",
  "RINGINDICATOR", "CONNECTINDICATOR", "RINGTIMEOUT", "DIALTIMEOUT",
  "DIALDELAY", "CONNECTDELAY",
  "MAXCONTIME", "MAXCONS", "TTYTIMEOUT",
  NULL
};

/*---------------------------------------------------------------------------*/

static enum befnum
{ unsinn,
  comadr, comirq, cmdatz, cmdata, cmdatd,
  ringindicator, connectindicator, ringtimeout, dialtimeout,
  dialdelay, connectdelay,
  maxcontime, maxcons, ttytimeout
} cmd = unsinn;

/*---------------------------------------------------------------------------*/

void tty_counterreset ()
//*************************************************************************
//
// Countdown und Connectzaehler zuruecksetzen
//
//*************************************************************************
{
  maxconcountdown = 0;
  dialcountdown = 0;
  concounter = 0;
}

/*---------------------------------------------------------------------------*/

static void tty_status (void)
//*************************************************************************
//
// Modemleitungen abfragen
//
//*************************************************************************
{
  int i = inportb(ttyace+6);
  carrier = ((i&128)>0);
  ring = ((i&64)>0);
  dsr = ((i&32)>0);
  cts = ((i&16)>0);
  dcarrier = ((i&8)>0);
  tring = ((i&4)>0);
  ddsr = ((i&2)>0);
  dcts = ((i&1)>0);
}

/*---------------------------------------------------------------------------*/

static void tty_ctrl (int bit, int j)
//*************************************************************************
//
// Modemleitungen kontrollieren
//
//*************************************************************************
{ // Auslesen
  int i = inportb(ttyace+4);
  reserved1 = ((i&128)>0);
  reserved2 = ((i&64)>0);
  out0 = ((i&32)>0);
  loop = ((i&16)>0);
  out2 = ((i&8)>0);
  out1 = ((i&4)>0);
  rts = ((i&2)>0);
  dtr = ((i&1)>0);
  if (j != 0 && j != 1) return;
  //Ersetzen
  if (bit == 7) reserved1 = j;
  if (bit == 6) reserved2 = j;
  if (bit == 5) out0 = j;
  if (bit == 4) loop = j;
  if (bit == 3) out2 = j;
  if (bit == 2) out1 = j;
  if (bit == 1) rts = j;
  if (bit == 0) dtr = j;
  //Schreiben
  outportb(ttyace+4,
          dtr+rts*2+out1*4+out2*8+loop*16+out0*32+reserved2*64+reserved1*128);
}
/*---------------------------------------------------------------------------*/
static void tty_rtscts (void)
//*************************************************************************
//
//
//*************************************************************************
{
  cts = ((inportb(ttyace+6)&16) > 0);
  // Senden unterbrechen CTS
  if (txin < txout)
    txdiff = TXLEN-txout+txin;
  else
    txdiff = txin-txout;
  if (cts == 0 && txstopped == 0)
    txstopped = 1;
  else
    if (cts == 1 && txstopped != 0)
    {
      if (txstopped == 2)
      {
        if (txin != txout)
        {
          outportb(ttyace, txbuf[txout++]); // puffer senden
          if (txout == TXLEN)
            txout = 0;
          txint = 1;
        }
      }
      txstopped = 0;
    }
  // Empfangen unterbrechen RTS
  if (rxin < rxout)
    rxdiff = RXLEN-rxout+rxin;
  else
    rxdiff = rxin-rxout;
  if (rxdiff > 200)
    tty_ctrl(1, 0);
  else
    tty_ctrl(1, 1);
}

/*---------------------------------------------------------------------------*/

static void tty_watch (char *)
// Ueberwachungsschleife
//*************************************************************************
//
//
//*************************************************************************
{
  char *a = 0;

  time_t sektick = ad_time();
  while (! runterfahren && eingelogt("getty", 0, 0))
  {
    tty_status();
    tty_ctrl(-1, -1);
    tty_rtscts();
    sprintf(a, "%i%i%i%i%i%i%i%i%i%i%i",
                cmdmode, ttyactive, carrier, ring, dsr, rts, cts, dtr,
                ttyecho, (txin != txout && !txstopped),
                (rxin != rxout && rts));
    lastcmd(a);
    if (ttyactive)
      wdelay(32);
    else
      wdelay(324);
    if (time(0)-sektick >= 1) // Sekundeninterval
    {
      sektick = ad_time();
      if (dialcountdown)
        dialcountdown--;
      if (maxconcountdown)
        maxconcountdown--;
      else
        concounter = 0;
    }
  }
}

/*---------------------------------------------------------------------------*/

static int near parse_mode (char *mode)
//*************************************************************************
//
//  untersucht einen String  1:9600,n,8,1,e
//  und setzt die entsprechenden Werte
//
//*************************************************************************
{
  char *k;

  if (strlen(mode) < 13)
    return 0;
  ttyport = mode[0] & 7;
  mode += 2;
  k = strchr(mode, ',');
  if (k)
  {
    ttybaud = atoi(mode);
    if (ttybaud < 1200)
      return 0;
    if ((k[2] != ',') || (k[4] != ',') || (k[6] != ',') || k[8])
      return 0;
    switch (k[1])
    {
      case 'N': ttyparity = 0; break;
      case 'E': ttyparity = 3; break;
      case 'O': ttyparity = 1; break;
      default: return 0;
    }
    if ((k[3] > '4') && (k[3] < '9'))
      ttybits = k[3]-'5';
    else
      return 0;
    if ((k[5] > '0') && (k[5] < '3'))
      ttystop = k[5]-'1';
    else
      return 0;
    echopreset = (k[7] == 'E');
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static void near initace (void)
//*************************************************************************
//
//  Initialisierung des 8250 fuer asynchronen Betrieb
//
//*************************************************************************
{
  inportb(ttyace);                        // Datenregister ausleeren
  outportb(ttyace+4, 0x0b);               // COM aktiv, DTR High
  outportb(ttyace+3, 0x81);               // Divisor latch access
  outportb(ttyace, (19200L*6)/ttybaud);   // Baudrate setzen
  outportb(ttyace+1, 0);
  outportb(ttyace+3, ttybits+(ttystop<<2)+(ttyparity<<3));
  outportb(ttyace+1, 0x03);               // rx / rx interrupt
  outportb(ttyace+2, 0x87);               // 16550 FIFO enable/reset
  if (inportb(ttyace+2) & 0xc0)
    trace(report, "initace", "16550 found");
  inportb(ttyace);                        // rx ausleeren
  inportb(ttyace+5);                      // clear flags
  inportb(ttyace+2);
}

/*---------------------------------------------------------------------------*/

static void interrupt ttyisr (void)
//*************************************************************************
//
//  Interruptroutine für die serielle Schnittstelle
//
//*************************************************************************
{
  do
  {
    // Leitungen abfragen
    //tty_xonxoff();
    tty_rtscts();
    // Eingang
    if (inportb(ttyace+2) & (char)4)
    {
      register char ch = inportb(ttyace);
      rxbuf[rxin++] = ch;
      if (rxin==RXLEN) rxin = 0;
    }
    // Ausgang
    if ((inportb(ttyace+2) & (char)2)
        || (inportb(ttyace+5) & (char)32)) // tx frei?
    {
      if ((txin != txout) && ! txstopped)
      {
        outportb(ttyace, txbuf[txout++]); // senden
        if (txout == TXLEN)
          txout = 0;
      }
      else
      {
        txint = 0;
        if (txstopped) // senden blockiert
          txstopped = 2;
      }
    }
  }
  while (inportb(ttyace+2) & (char)6); // noch weitere Bytes zu bearbeiten?
  outportb(0x20, 0x20);
}

/*---------------------------------------------------------------------------*/

static void near irqinit_tty (int ein)
//*************************************************************************
//
//
//*************************************************************************
{
  static char intmask;
  static void interrupt(*altvec)(...);

  disable();
  if (ein)
  {
    intmask=inportb(0x21);
    if (! tty.comirq)
    {
      altvec = getvect(irqvec[ttyport]+8);
      setvect(irqvec[ttyport]+8, (void interrupt (*)(...))ttyisr);
      ttyirq = irqvec[ttyport];
    }
    else
    {
      altvec = getvect(tty.comirq+8);
      setvect(tty.comirq+8, (void interrupt (*)(...))ttyisr);
      ttyirq = tty.comirq;
    }
    if (! tty.comadr)
      ttyace = portadr[ttyport];
    else
      ttyace = tty.comadr;
    outportb(0x21, intmask & (~(1<<irqvec[ttyport])));
  }
  else
  {
    outportb(ttyace+3, 0);
    outportb(ttyace+1, 0);
    outportb(ttyace+4, 0);
    outportb(0x21, intmask);
    setvect((irqvec[ttyport]+8), altvec);
  }
  enable();
}

/*---------------------------------------------------------------------------*/

static int near holtty (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int retwert = EOF;

  disable();
  if (rxin != rxout)
  {
    retwert = rxbuf[rxout++];
    if (rxout == RXLEN)
      rxout = 0;
  }
  enable();
  return retwert;
}

/*---------------------------------------------------------------------------*/

int getv_tty (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int tmp;

  if (rxdafl)
  {
    rxdafl = 0;
    if (t->monitor)
      mon(rxcbuf, o_o);
    return rxcbuf;
  }
  while (1)
  {
    tmp = holtty();
    if (tmp != EOF)
    {
      if (t->monitor)
        mon(tmp, o_o);
      return tmp;
    }
    else
      wdelay(14);
  }
}

/*---------------------------------------------------------------------------*/

int getvalid_tty (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int tmp;

  if (rxdafl)
  {
    if (rxcbuf == CR)
      return CR;
    return 1;
  }
  if ((tmp = holtty()) != EOF)
  {
    rxcbuf = tmp;
    rxdafl = 1;
    if (tmp == CR)
      return CR;
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static void histflush ()
//*************************************************************************
//
//
//*************************************************************************
{
  int i;

  for (i = 0; i != HISTLEN; i++)
    history[i][0] = 0;
  histwrite = 0;
}

/*---------------------------------------------------------------------------*/

void inputlineusr_tty (char *s, int maxlen, char cut, int tio)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = 0;
  int j = 0;
  int semi = 0;
  char si = 0;
  int siok = 0;
  int maxi = 0;
  int insert = 1;
  int histread = histwrite;
  int refresh = 0;
  s[0] = 0;
  time_t tstart = ad_time();
  if (cut == 2)
  {
    cut = 1;
    tio = 10;
  }
  //if (tio) tio++;
  if (maxlen < 0)
  {
    // db7mh semi = 1;
    maxlen = (-maxlen);
  }
  while (ad_time()-tstart < tio || ! tio)
  {
    si = 0; // TTY Input
    siok = 1;
    if (rxdafl)
    {
      rxdafl = 0;
      si = rxcbuf;
    }
    while (! si && (ad_time()-tstart < tio || ! tio))
    {
      j = holtty();
      if (j != EOF)
        si = j;
      else
        wdelay(14);
    }
    if (si)
      tstart = ad_time();
    else
      siok = 0;
    if (semi && s[i] == ';') // semi
    {
      if (ttyecho != 0)
        putv_tty(LF);
      break;
    }
    if (si == 8) // Backspace
    {
      if (i > 0)
      {
        putv_tty(8);
        i--;
        for (j = i; j != maxi-1; j++)
        {
          s[j] = s[j+1];
          if (ttyecho == echopreset)
            putv_tty(s[j]);
          if (ttyecho == 2)
            putv_tty('*');
        }
        if (ttyecho != 0)
        {
          putv_tty(32);
          for (j = i; j != maxi; j++)
            putv_tty(8);
        }
        s[maxi] = 0;
        maxi--;
      }
      siok = 0;
    }
    if (si == 127) // Delete
    {
      if (maxi > i)
      {
        for (j = i; j != maxi-1; j++)
        {
          s[j] = s[j+1];
          if (ttyecho == echopreset)
            putv_tty(s[j]);
          if (ttyecho == 2)
            putv_tty('*');
        }
        if (ttyecho != 0)
        {
          putv_tty(32);
          for (j = i; j != maxi; j++)
            putv_tty(8);
        }
        s[maxi] = 0;
        maxi--;
      }
      siok = 0;
    }
    if (si == 9) // Insert
    {
      insert = ! insert;
      siok = 0;
    }
    if (si == 27) // ANSI ESC-Sequenzfilter
    {
      siok = 0;
      getv_tty();
      si = getv_tty();
      if (si == 67 && i < maxi) // rechts
      {
        i++;
        siok = 1;
      }
      if (si == 68 && i > 0) // links
      {
        i--;
        siok = 1;
      }
      if (si == 72) // pos1
      {
        for (j = i; j != 0; j--)
          if (ttyecho != 0)
            putv_tty(8);
        i = 0;
      }
      if (si == 74) // end
      {
        for (j = i; j != maxi; j++)
          if (ttyecho != 0)
            putv_tty(s[j]);
        i = maxi;
      }
      if (si == 66) // runter
        if (histread != histwrite)
        {
          histread = (histread+1) % HISTLEN;
          goto histcopy;
        }
      if (si == 65) // rauf
        if (histread != 0)
        {
          histread = (histread+HISTLEN-1) % HISTLEN;
          histcopy:
            strncpy(s, history[histread], maxlen-1);
            siok = 0;
            refresh = maxi+1;
            maxi = strlen(s);
          }
      if (siok && ttyecho != 0)
      {
        putv_tty(27);
        putv_tty(91);
        if (ttyecho == echopreset)
          putv_tty(si);
        else putv_tty('*');
        siok = 0;
      }
    }
    //if (si == LF) siok = 0;
    if (si == CR && siok == 1) // LF/CR Zeilenende
    {
      if (ttyecho)
      {
        putv_tty(CR);
        putv_tty(LF);
      }
      i = maxi;
      if (! cut)
      {
        s[i] = si;
        i++;
      }
      if (strlen(s))
      {
        strncpy(history[histwrite], s, maxi);
        history[histwrite][maxi] = 0;
        histread = histwrite = (histwrite+1) % HISTLEN;
      }
      // LF filtern
      if (rxbuf[rxout] == LF) getv_tty();
      break;
    }
    // Filter fuer "+++"
    if (i > 1 && si == '+')
      if (s[i-2] == '+' && s[i-1] == '+')
        siok = 0;
    // Ausgabe
    if (siok)
    {
      if (t->monitor && ttyecho == 0)
        mon(si, o_o);
      if (ttyecho == echopreset) // normal
        putv_tty(si);
      if (ttyecho == 2) // sterne
        putv_tty('*');
      if (insert)
      {
        for (j = i; j != maxi; j++)
          if (ttyecho == echopreset)
            putv_tty(s[j]);
          else
            putv_tty('*');
        for (j = i; j != maxi; j++)
          if (ttyecho != 0)
            putv_tty(8);
        if (maxi < maxlen) maxi++;
        for (j = maxi; j != i-1; j--)
          s[j+1] = s[j];
      }

      s[i] = si;
      if (i < maxlen)
      {
        i++;
        if (i > maxi)
          maxi++;
      }
    }
    if (refresh)
    {
      for (j = i; j != 0; j--)
        if (ttyecho != 0)
          putv_tty(8);
      for (j = 0; j != refresh-1; j++)
        if (ttyecho != 0)
          putv_tty(32);
      for (j = refresh-1; j != 0; j--)
        if (ttyecho != 0)
          putv_tty(8);
      for (j = 0; j != maxi; j++)
        if (ttyecho == echopreset)
          putv_tty(s[j]);
        else
          putv_tty('*');
      i = maxi;
      refresh = 0;
    }
  }
  s[i] = 0;
}

/*---------------------------------------------------------------------------*/

void inputlinefwd_tty (char *s, int maxlen, char cut)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = 0;
  if (maxlen < 0)
    maxlen = (-maxlen);
  while (1)
  {
    if (! cut && i == maxlen)
      break;
    do
      s[i] = getv_tty();
    while (s[i] == LF);
    if (s[i] == CR)
    {
      if (! cut)
        i++;
      // LF filtern
      wdelay(200);
      if (rxbuf[rxout] == LF)
        getv_tty();
      break;
    }
    if (i < maxlen)
      i++;
  }
  s[i] = 0;
}

/*---------------------------------------------------------------------------*/

void inputline_tty (char *s, int maxlen, char cut)
//*************************************************************************
//
//
//*************************************************************************
{
  if (b->forwarding == fwd_none)
    inputlineusr_tty(s, maxlen, cut, 0); // User-Terminal
  else
    inputlinefwd_tty(s, maxlen, cut); // FWD
}

/*---------------------------------------------------------------------------*/

int putfree_tty (void)
//*************************************************************************
//
//
//*************************************************************************
{
  return (((txin+1) % TXLEN) != txout);
}

/*---------------------------------------------------------------------------*/

void putflush_tty (void)
//*************************************************************************
//
//
//*************************************************************************
{
}

/*---------------------------------------------------------------------------*/

void putv_tty (int ch)
//*************************************************************************
//
//
//*************************************************************************
{
  if (txint || txstopped)
  {
    if (txstopped)
      txstopped = 2;
    enable();
    while (((txin+1) % TXLEN) == txout)
      wdelay(60);
    disable();
    if (txint || txstopped)
    {
      txbuf[txin++] = ch;
      if (t->monitor)
        mon(ch, o_o);
      if (txin == TXLEN)
        txin = 0;
      enable();
      return;
    }
  }
  disable();
  txint = 1;
  while (! cts)
    wdelay(123); // debug reaktionszeit
  outportb(ttyace, ch);
  enable();
  if (t->monitor)
    mon(ch, o_o);
}

/*---------------------------------------------------------------------------*/

void putf_tty (char *format, ...)
//*************************************************************************
//
//
//*************************************************************************
{
  va_list argpoint;
  char cbuf[260];
  char *s = cbuf;

  va_start(argpoint, format);
  vsprintf(cbuf, format, argpoint);
  va_end(argpoint);
  if (strlen(s) > 257)
  {
    s[50] = 0;
    trace(tr_abbruch, "putf_tty", "long %s", s);
  }
  while(*s)
    putv_tty(*(s++));
}

/*---------------------------------------------------------------------------*/

void tty_kill (void)
//*************************************************************************
//
//
//*************************************************************************
{
  ttykilled = 1;
}

/*---------------------------------------------------------------------------*/

int tty_cmd (char *cmd)      // Modembefehl abschicken
//*************************************************************************
//
//
//*************************************************************************
{
  char s[15];
  time_t tstart = ad_time();
  if (txstopped) return 0;
  // Puffer loeschen
  ttysetecho(0);
  do
    inputlineusr_tty(s, 80, 0, 1);
  while (strlen(s));
  putf_tty("%s\r", cmd);
  while (ad_time() - tstart < 4)
  {
    waitokline:
    inputlineusr_tty(s, 10, 0, 1);
    if (strncmp(s, "AT", 2) == 0) goto waitokline; // MODEM ECHO
    if (strstr(s, "NO CARRIER")) goto waitokline; // MODEM NC
    if (strlen(s) > 1)
    {
      s[strlen(s)-1] = 0;
      trace(report, "TTY", "%s - %s", cmd, s);
    }
    if (strstr(s, "OK"))
    {
      cmdmode = 1;
      return 1;
    }
    if (strstr(s, "ERROR")) return 0;
    if (strstr(s, "NO")) return 0;
  }
  if (strlen(s) > 0)
    trace(report, "TTY", "%s - invalid response (%s)", cmd, s);
  else
    trace(report, "TTY", "%s - no response", cmd);
  return 0;
}

/*-----------------------------------------------------------------------------*/

int tty_dial (char *befbuf, int taskid)
//*************************************************************************
//
//
//*************************************************************************
{
  char call[7];
  char *nummer = 0;

  if (ttyactive || ! m.ttydevice || txstopped || m.disable ||
     dialcountdown || (maxconcountdown && concounter >= tty.maxcons))
    return 0;
  befbuf = nexttoken(befbuf, call, 6);
  if (strncmp(befbuf, "TTY", 3) == 0)
    nummer = nexttoken(befbuf, 0, 80);
  else
    return 0;
  strupr(call);
#ifdef _LCF
  if (fwdcallok(call, m.logincallformat) && (nummer || m.ttydevice == 1))
#else
  if (mbcallok(call) && (nummer || m.ttydevice == 1))
#endif
  {
    strcpy(ttycall, call);
    dialup = 1;
    ttytaskid = taskid;
    if (m.ttydevice > 1)
      putf_tty("%s%s\r", tty.cmdatd, nummer);
    time_t tstart = ad_time();
    ttyactive = 3;
    dialcountdown = tty.dialdelay;
    // warte bis connect oder zeit abgelaufen
    while (ad_time()-tstart < tty.dialtimeout
           && (ttyactive == 3 || ttyactive == 4))
      wdelay(145);
    if (ttyactive==5) return 1; // login ok
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static int tty_autologin (void)
//*************************************************************************
//
// Automatisch in andere Mailbox einloggen
//
//*************************************************************************
{
  char userpw[10];
  char s[80];

  trace(report, "TTY", "autologin started");
  get_ttypw(ttycall, userpw);
  time_t tstart = ad_time();
  lastcmd("logging in");
  if (m.ttydevice == 1)
    putf_tty("\r"); // bei Nullmodemlink "login:" erzeugen
  while (ad_time()-tstart < 15)
  {
    ttysetecho(0);
    inputlineusr_tty(s, 80, 0, 1);
    if (strstr(s, "login:") || strstr(s, "Callsign :")) // loginaufruf
    {
      tstart = ad_time();
      lastcmd("sending call");
      putf_tty("%s\r", m.boxname);
    }
    if (strstr(s, "password:") || strstr(s, "Password :"))
    {
      lastcmd("sending password");
      putf_tty("%s\r", userpw);
      inputlineusr_tty(s, 80, 0, 1); // Echo loeschen
      trace(report, "TTY", "autologin ok");
      return 1;
    }
    if (strstr(s, "autologin failure"))
      trace(serious, "TTY", "login incorrect");
    if (strstr(s, "good bye."))
    {
      trace(serious, "TTY", "userid/pwd login failure");
      return 0;
    }
  }
  trace(serious, "TTY", "autologin time out");
  return 0;
}

/*---------------------------------------------------------------------------*/

static int tty_waitring (void)
//*************************************************************************
//
// Warteschleife fuer Anruf oder Dialup
//
//*************************************************************************
{
  char s[80];
  char dummy[80];
  time_t tstart = 0;
  time_t tdif;
  while (1)
  {
    ttysetecho(0);
    if(cts)
    {
      tdif = ad_time() - tstart;
      if (ttyactive == 0) sprintf(dummy, "wait for ring");
      if (ttyactive == 1) sprintf(dummy, "busy");
      if (ttyactive == 2) sprintf(dummy, "ring(%i)", tty.ringtimeout-tdif);
      if (ttyactive == 3) sprintf(dummy, "dialup(%i)", tty.dialtimeout-tdif);
      lastcmd(dummy);
    }
    else
      lastcmd("standby");
    inputlineusr_tty(s, 80, 0, 1);
    strupr(s);
    if (! m.disable)
    { // Vorgang unterbrochen
      if (ttyactive == 1)
      {
        ttycall[0] = 0;
        ttytaskid = 0;
        ttyactive = 0;
        dialup = 0;
      }
      // Eingehender Anruf
      if (! ttyactive && (! strncmp(s, tty.ringindicator, 4) || ring))
      {
        trace(report, "TTY", "RING");
        putf_tty("%s\r", tty.cmdata);
        tstart = ad_time();
        ttyactive = 2;
      }
      // Ausgehender Dialup
      if (dialup && ! tstart)
      {
        trace(report, "TTY", "DIALUP to %s", ttycall);
        tstart = ad_time();
        // Ausgehender Nullmodemlink
        if (m.ttydevice == 1) sprintf(s, tty.connectindicator);
      }
      // Verbindungsaufbau ok
      if (strncmp(s, tty.connectindicator, 7) == 0 || strncmp(s, "CARRIER", 7) == 0)
      {
        s[strlen(s)-1] = 0;
        trace(report, "TTY", "%s", s);
        strncpy(linespeed, nexttoken(s, dummy, 80), 10);
        lastcmd("connect");
        cmdmode = 0;
        ttyactive = 4;
        onlinetime = ad_time();
        if (dialup) concounter++;
        if (! maxconcountdown) maxconcountdown = tty.maxcontime;
        return 1;
      }
      // Verbindungsaufbau bei ring fehlgeschlagen
      if (ttyactive == 2 && tstart
          && (! strncmp(s, "NO", 2) || time(0)-tstart > tty.ringtimeout || ! cts))
      {
        tstart = 0;
        trace(report, "TTY", "connect timeout");
        tty_hangup();
        ttyactive = 0;
      }
      // Verbindungsaufbau bei dialup fehlgeschlagen
      if (   ttyactive == 3
          && tstart
          && (strncmp(s, "NO", 2) == 0
              || strncmp(s, "BUSY", 4) == 0
              || strncmp(s, "ERROR", 5) == 0
              || strncmp(s, "WAIT", 4) == 0
              || strncmp(s, "DELAY", 5) == 0
              || ad_time()-tstart > tty.dialtimeout
              || ! cts))
      {
        ttycall[0] = 0;
        ttytaskid = 0;
        tstart = 0;
        dialup = 0;
        trace(report, "TTY", "dialup error %s", s);
        tty_hangup();
        ttyactive = 0;
      }
    }
    else
      ttyactive = 1;
  }
}

/*---------------------------------------------------------------------------*/

int tty_hangup (void)
//*************************************************************************
//
//
//*************************************************************************
{
  ttyactive = 1; // Dialup und andere Aktionen unterbrechen
  tty_ctrl(0, 0); // DTR low
  wdelay(834);
  tty_ctrl(0, 1); // DTR High
  return ! carrier;
}

/*---------------------------------------------------------------------------*/

static void ttylogin (char *call)
//*************************************************************************
//
//
//*************************************************************************
{
  t->input = io_tty;
  t->output = io_tty;
  mbmain(call, "TTY", m.mycall[0]);
}
/*---------------------------------------------------------------------------*/

static void tty_clearbuffer (void)
//*************************************************************************
//
//
//*************************************************************************
{
  while (holtty() != EOF);
    wdelay(234);
}

/*---------------------------------------------------------------------------*/

static void tty_initmodem (void)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = 0;
  ttysetecho(0);
  do
  { cmdmode = 1;
    cmdmode = tty_cmd(tty.cmdatz);
    i++;
  }
  while (! cmdmode && i < 2);
}

/*---------------------------------------------------------------------------*/

static void getty (char *)
//*************************************************************************
//
//
//*************************************************************************
{
  int loginok = 0;
  fork(0, 0, tty_watch, "ttywatch");
  t->input = io_tty;
  t->output = io_tty;
  enable();
  wdelay(1123);
  while (! runterfahren)
  {
    ttyactive = 1;
    ttycall[0] = 0;
    dialup = 0;
    ttysetecho(0);
    if (m.ttydevice > 1) cmdmode = 1;
    // Puffer entleeren
    lastcmd("clear buffer");
    tty_clearbuffer();
    // Auf Verbindung warten
    if (m.ttydevice == 1)
    {
      ttysetecho(0);
      trace(report, "TTY", "nullmodem mode");
      lastcmd("wait for start");
      ttyactive = 0;
      while (! dialup)
      {
        tty_clearbuffer();
        wdelay(246);
      }
    }

    if (m.ttydevice > 1)
    {
      trace(report, "TTY", "init modem");
      lastcmd("init modem");
      tty_initmodem();
      if (! cmdmode && m.ttydevice == 3) break;
      ttyactive = 0;
      tty_waitring();
    }

    ttyactive = 4;
    histflush();
    // Einloggen
    if (dialup)
    {
      loginok = tty_autologin(); // ausgehender connect, automatisch einloggen
      ttysetecho(0);
    }
    else
    {
      tty_clearbuffer();
      loginok = getlogin(ttycall, 1, "TTY"); // eingehender connect, einloggen
      ttysetecho(0);
      if (isforwardpartner(ttycall) >= 0)
        trace(report, "TTY", "forward connect");
      else
        ttysetecho(1);
    }
    // Mainloop starten
    // trace(report, "TTYdebug", "e%i l%i c%i", taskexists(ttytaskid), eingelogt(ttycall, 0), carrier);
    histflush();
    if (loginok)
    {
      ttyactive = 5;
      lastcmd("login");
      if (! dialup)
        ttytaskid = fork(P_BACK | P_MAIL, 0, ttylogin, ttycall); // eingehender connect, task wird aufgebaut
      // Standbyschleife wenn eingeloggt
      while (loginok)
      {
        if (! taskexists(ttytaskid)) // wenn task killed
        {
          strcpy(ttycall, "task closed");
          loginok = 0;
        }
//        if (! eingelogt(ttycall, 0, 0)) // wenn ausgeloggt
//        {
//          strcpy(ttycall, "logged out");
//          loginok = 0;
//        }
        if (! carrier) // CARRIER
        {
          killtask(ttytaskid, 2);
          strcpy(ttycall, "NO CARRIER");
          loginok = 0;
        }
        wdelay(295);
      }
      lastcmd("disconnected");
    }
    // Verbindung aufheben
    ttyactive = 1;
    ttytaskid = 0;
    *linespeed = 0;
    dialup = 0;
    if (m.ttydevice > 1)
    {
      trace(report, "TTY", "%s", ttycall);
      wdelay(624);
      lastcmd("hang up modem");
      tty_hangup();
      if (carrier) trace(serious, "TTY", "hang up error");
    }
    else
    {
      wdelay(623);
      tty_hangup();
    }
    ttycall[0] = 0;
    dialcountdown = tty.connectdelay;
  }
  trace(serious, "TTY", "getty terminated");
}

/*---------------------------------------------------------------------------*/

static void tty_off (void)
//*************************************************************************
//
//
//*************************************************************************
{
  irqinit_tty(0);
}

/*---------------------------------------------------------------------------*/

void ttysetecho (int echo)
//*************************************************************************
//
//
//*************************************************************************
{
  if (echo == 2) ttyecho = 2;
  if (echo == 1) ttyecho = echopreset;
  if (echo == 0) ttyecho = 0;
}

/*---------------------------------------------------------------------------*/

void init_tty (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (m.ttymode[0])
  {
    if (parse_mode(m.ttymode))
    {
      irqinit_tty(1);
      if (atexit(tty_off) != 0)
        trace(tr_abbruch, "init_tty", "atexit full");
      trace(report, "init_tty", "start %s(irq %i,adr %i)",
                                m.ttymode, ttyirq, ttyace);
      initace();
      fork(P_MAIL, 0, getty, "getty");
    }
    else
      trace(serious, "init_tty", "syntax %s", m.ttymode);
  }
}

/*---------------------------------------------------------------------------*/

void tty_statustext ()
//*************************************************************************
//
//
//*************************************************************************
{
  putf("TTY  ");
  if (ttyactive == 0 && cts) putf("free   ");
  if (ttyactive == 0 && ! cts) putf("busy   ");
  if (ttyactive == 1) putf("busy   ");
  if (ttyactive == 2) putf("ring   ");
  if (ttyactive == 3) putf("dialup ");
  if (ttyactive == 4) putf("connect");
  if (ttyactive == 5) putf("login  ");
  putf(" POS %i/%i    dialcountdown    %i", ttyactive, dialup, dialcountdown);
  putf("\nCALL ");
  if (ttyactive == 5 || dialup)
    putf("%-7s", ttycall);
  else
    putf("-      ");
  putf(" CD  %i      maxconcountdown  %i", carrier, maxconcountdown);
  putf("\nTASK %-7i RI  %i      concounter       %i", ttytaskid, ring, concounter);
  putf("\nCMD  %-7i DSR %i      dialup     %i", cmdmode, dsr, dialup);
  putf("\nECH  %-7i DTR %i", ttyecho, dtr);
  putf("\nRX ");
  if (rxdiff)
    putf("!");
  else
    putf(" ");
  if (! rts)
    putf(">");
  else
    putf(" ");
  putf("%-7i RTS %i", rxdiff, rts);
  putf("\nTX ");
  if (txdiff)
    putf("!");
  else
    putf(" ");
  if (txstopped > 0)
    putf(">");
  else
    putf(" ");
  putf("%-7i CTS %i", txdiff, cts);
  if (! dialup
      && (dialcountdown || (ttyactive == 0 && ! cts)
                        || (maxconcountdown && concounter >= tty.maxcons)))
    putf("      cannot use ttyport");
  putf("\nLINE %-7s ", linespeed);
  if (ttyactive > 3)
    putf("%s", datestr(ad_time()-onlinetime, 4));
  else
  {
    putf("offline   ");
    if (dialcountdown)
      putf(" dialdelay %is", dialcountdown);
    if (maxconcountdown && concounter >= tty.maxcons)
      putf(" maxcondelay %i", maxconcountdown);
    if (ttyactive == 0 && ! cts)
      putf(" modem busy/not connected");
  }
  wdelay(48);
}

/*---------------------------------------------------------------------------*/

void tty_win (char *befbuf)
//*************************************************************************
//
//
//*************************************************************************
{
  int keysave = keywindow();
  e->x = 60;
  e->xend = 79;
  e->y = 1;
  e->yend = 10;
  strcpy(t->name, "ttystat");
  preparewin(putupdate, "TTY", 400);
  newkeywin = keysave;
  while (! runterfahren)
  {
    wdelay(492);
    e->index = e->modified = e->curline = 0;
    putv(LF);
    tty_statustext();
    e->shouldupdate++;
  }
  wclose();
}

/*---------------------------------------------------------------------------*/

int ttyinitbef (char *befbuf,int inifile)
//*************************************************************************
//
//
//*************************************************************************
{
  int i;

  befbuf += blkill(befbuf);
  if ((befbuf[0] == 0) || (befbuf[0] == ';') || (befbuf[0] == '['))
    return 1;
  if (inifile)
  {
    if (strchr(befbuf, ';')) // OE3DZW cut off comment
      *strchr(befbuf,';') = 0;
    //replace all controll-characters by space (tab/linefeed etc.)
    for (i = 0; befbuf[i]; i++)
    {
      if (befbuf[i] < 0x20)
        befbuf[i] = 0x20;
    }
  }
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  if (befbuf[0] == '=') befbuf++;
  befbuf += blkill(befbuf);
  if (befbuf[0])
  {
    switch (cmd)
    {
      case unsinn:
        return 0;

      case comadr:
        tty.comadr = atoi(befbuf);
        break;

      case comirq:
        tty.comirq = atoi(befbuf);
        break;

      case cmdatz:
        befbuf[59] = 0;
        strcpy(tty.cmdatz, befbuf);
        break;

      case cmdata:
        befbuf[59] = 0;
        strcpy(tty.cmdata, befbuf);
        break;

      case cmdatd:
        befbuf[59] = 0;
        strcpy(tty.cmdatd, befbuf);
        break;

      case ringindicator:
        befbuf[59] = 0;
        strcpy(tty.ringindicator, befbuf);
        break;

      case connectindicator:
        befbuf[59] = 0;
        strcpy(tty.connectindicator, befbuf);
        break;

      case ringtimeout:
        tty.ringtimeout = atoi(befbuf);
        break;

      case dialtimeout:
        tty.dialtimeout = atoi(befbuf);
        break;

      case dialdelay:
        tty.dialdelay = atoi(befbuf);
        break;

      case connectdelay:
        tty.connectdelay = atoi(befbuf);
        break;

      case maxcontime:
        tty.maxcontime = atoi(befbuf);
        break;

      case maxcons:
        tty.maxcons = atoi(befbuf);
        break;

      case ttytimeout:
         tty.ttytimeout = atoi(befbuf);
        break;
    }
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

void ttyinit(void)
//*************************************************************************
//
//
//*************************************************************************
{
  FILE *f;
  int line = 0;
  char s[256];

  memset(&tty, 0, sizeof(tty));
  tty.comadr = 0;
  tty.comirq = 0;
  strcpy(tty.cmdatz, "ATZ");
  strcpy(tty.cmdata, "ATA");
  strcpy(tty.cmdatd, "ATD");
  strcpy(tty.ringindicator, "RING");
  strcpy(tty.connectindicator, "CONNECT");
  tty.ringtimeout = 30;
  tty.dialtimeout = 60;
  tty.dialdelay = 0;
  tty.connectdelay = 0;
  tty.maxcontime = 0;
  tty.maxcons = 0;
  tty.ttytimeout = 5;
  f = s_fopen(INITTTYNAME, "srt");
  if (f)
  {
    while(fgets(s, 255, f))
    {
      if(s[strlen(s)-1] == LF)
        s[strlen(s)-1] = 0;
      if(s[strlen(s)-1] == CR)
        s[strlen(s)-1] = 0;
      if (! ttyinitbef(s, 1))
        trace(serious, "ini", "%d %s", line, s);
      line++;
    }
    s_fclose(f);
  }
}

/*---------------------------------------------------------------------------*/

void ttyparsave(void)
//*************************************************************************
//
//
//*************************************************************************
{
  FILE *f;
  f = s_fopen(INITTTYTMPNAME, "swt");
  if (f)
  {
    s_fsetopt(f, 1);
    fprintf(f, "; --- tty config port data ---\n");
    fprintf(f, "comadr %d\n", tty.comadr);
    fprintf(f, "comirq %d\n", tty.comirq);
    fprintf(f, "; --- modem commands ---\n");
    fprintf(f, "cmdatz %s\n", tty.cmdatz);
    fprintf(f, "cmdata %s\n", tty.cmdata);
    fprintf(f, "cmdatd %s\n", tty.cmdatd);
    fprintf(f, "ringindicator %s\n", tty.ringindicator);
    fprintf(f, "connectindicator %s\n", tty.connectindicator);
    fprintf(f, "; --- time setups ---\n");
    fprintf(f, "ringtimeout %d\n", tty.ringtimeout);
    fprintf(f, "dialtimeout %d\n", tty.dialtimeout);
    fprintf(f, "dialdelay %d\n", tty.dialdelay);
    fprintf(f, "connectdelay %d\n", tty.connectdelay);
    fprintf(f, "maxcontime %d\n", tty.maxcontime);
    fprintf(f, "maxcons %d\n", tty.maxcons);
    fprintf(f, "ttytimeout %d\n", tty.ttytimeout);
    s_fclose(f);
    xunlink(INITTTYBAKNAME);
    xrename(INITTTYNAME, INITTTYBAKNAME);
    xrename(INITTTYTMPNAME, INITTTYNAME);
  }
  else
    trace(serious, "ttyparsave", "fopen errno=%d", errno);
  return;

  fehler:
    trace(serious, "ttyparsave", "errno=%d", errno);
    s_fclose(f);
}

/*---------------------------------------------------------------------------*/

#endif
