/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  Layer1-Driver


  --------------------------------------------
  AX.25 KISS mode driver for tty (Linux/Win32)
  --------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//19980128 OE3DZW added wdelay() when sigpipe occurs (hope that helps..)
//19980222 DG2FEF removed that again, did not help ;-)
//19980309 OE3DZW fixed above for Linux
//19980523 OE3DZW err-msg "no CRC" only when serial line
//19990111 OE3DZW added fix by DG0OBQ
//19990117 OE3DZW removed sigpipe-stuff

#include "baycom.h"

#ifdef __UNIX__
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#define w_write write
#define w_read read
#endif

/*---------------------------------------------------------------------------*/

#define FEND       0xC0     // Kiss Frame Ende
#define FESC       0xDB     // Kiss Frame Escape
#define TFEND      0xDC     // transponiertes Frame Ende
#define TFESC      0xDD     // transponiertes Frame Escape
#define FRAME_ENDE    (-1)  // Uebergabeparameter fuer next_tx
#define FRAME_ANFANG  (-3)  // Uebergabeparameter fuer next_tx
#define NIX_MEHR_DRIN (-2)  // Uebergabeparameter fuer next_tx

#define RXLEN 100000
#define TXLEN 20000

/* CRC-Sachen */
#define CRCTYP 0x20
static u16 crctab[256];

typedef struct chkiss_t
{
  int      fd;
  char    *rxbuf;
  char    *txbuf;
  int      txin;
  int      txout;
  int      rxin;
  int      rxout;
  frame_t *rxloc;
  unsigned rxindex;
  int      txon;
  int      dcd;
  int      rx_state;
  l1_mode_t *m;
  int      do_crc;
  u16      rx_crc;
  u16      tx_crc;
  int      rx_errno;
  int      tx_errno;
  int      channel;
} chkiss_t;

static chkiss_t ki[KANAELE];

static int  restart_kiss = 0; //dg2fef
static void kissisr(chkiss_t *k);

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define USECOMM
#include <windows.h>

/*---------------------------------------------------------------------------*/

static int w_read (int fd, void *data, int maxlen)
{
  DWORD ret = 0;
  static OVERLAPPED ovl = { 0, 0, 0 };
  ReadFile((HANDLE) fd, data, maxlen, &ret, &ovl);
  return ret;
}

/*---------------------------------------------------------------------------*/

static int w_write (int fd, void *data, int maxlen)
{
  DWORD ret = 0;
  static OVERLAPPED ovl = { 0, 0, 0 };
  WriteFile((HANDLE) fd, data, maxlen, &ret, &ovl);
  return ret;
}

/*---------------------------------------------------------------------------*/

DWORD FAR PASCAL isr (LPSTR lpdata)
{
  chkiss_t *k = (chkiss_t *) lpdata;
  while (1)
  {
    kissisr(k);
    Sleep(20);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int open_win (char *name, chkiss_t *k, int baud)
{
  HANDLE h;

  //OSVERSIONINFO ov;
  //ov.dwOSVersionInfoSize = 128;
  //GetVersionEx(&ov);
  //trace(report, "ver", "%ld.%ld: %s", ov.dwMajorVersion,
  //               ov.dwMinorVersion, ov.szCSDVersion);
  /*
  COMMPROP cp;
  GetCommProperties(h, &cp);
  trace(report, "GetCommProp", "%d %s", h, strerror(GetLastError()));
  printf("CommProps dwMaxTxQueue %d\n", cp.dwMaxTxQueue);
  printf("CommProps dwMaxRxQueue %d\n", cp.dwMaxRxQueue);
  */

  h = CreateFile(name, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED,
                       NULL);
  if ((int) h == EOF)
  {
    trace(serious, "CreateFile", "%s", strerror(GetLastError()));
    return EOF;
  }
  if (SetCommMask(h, 0) == FALSE)
    trace(serious, "SetCommMask", "%s", strerror(GetLastError()));
  if (SetupComm(h, 4096, 4096) == FALSE)
    trace(serious, "SetupComm", "%s", strerror(GetLastError()));
  if (PurgeComm(h, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == FALSE)
    trace(serious, "PurgeComm", "%s", strerror(GetLastError()));
  COMMTIMEOUTS ct;
  ct.ReadIntervalTimeout = 0xFFFFFFFF;
  ct.ReadTotalTimeoutMultiplier = 0;
  ct.ReadTotalTimeoutConstant = 0;
  ct.WriteTotalTimeoutMultiplier = 0;
  ct.WriteTotalTimeoutConstant = 0;
  if (SetCommTimeouts(h, &ct) == FALSE)
    trace(serious, "SetCommTimeout", "%s", strerror(GetLastError()));
  DCB dcb;
  memset(&dcb, 0, sizeof(DCB));
  dcb.DCBlength = sizeof(DCB);
  GetCommState(h, &dcb);
  dcb.BaudRate = baud;
  dcb.fBinary = 1;
  dcb.fParity = 0;
  dcb.fOutxCtsFlow = 0;      // CTS output flow control
  dcb.fOutxDsrFlow = 0;      // DSR output flow control
  dcb.fDtrControl = 0;       // DTR flow control type
  dcb.fDsrSensitivity = 0;   // DSR sensitivity
  dcb.fTXContinueOnXoff = 1; // XOFF continues Tx
  dcb.fOutX = 0;             // XON/XOFF out flow control
  dcb.fInX = 0;              // XON/XOFF in flow control
  dcb.fErrorChar = 0;        // enable error replacement
  dcb.fNull = 0;             // enable null stripping
  dcb.fRtsControl = 0;       // RTS flow control
  dcb.fAbortOnError = 0;     // abort reads/writes on error
  dcb.fDummy2 = 0;           // reserved
  dcb.wReserved = 0;         // not currently used
  dcb.XonLim = 0;            // transmit XON threshold
  dcb.XoffLim = 0;           // transmit XOFF threshold
  dcb.ByteSize = 8;          // number of bits/byte, 4-8
  dcb.Parity = 0;            // 0-4=no,odd,even,mark,space
  dcb.StopBits = 0;          // 0,1,2 = 1, 1.5, 2
  dcb.XonChar = 0;           // Tx and Rx XON character
  dcb.XoffChar = 0;          // Tx and Rx XOFF character
  dcb.ErrorChar = 0;         // error replacement character
  dcb.EofChar = 0;           // end of input character
  dcb.EvtChar = 0;           // received event character
  if (SetCommState(h, &dcb) == FALSE)
    trace(serious, "SetCommState", "%s", strerror(GetLastError()));
  DWORD t_id;
  if (! CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)isr, (void *) k, 9, &t_id))
    trace(serious, "CreateThread", "%s", strerror(GetLastError()));
  return (int) h;
}
#endif

/*---------------------------------------------------------------------------*/

static void init_crc (void)
//*************************************************************************
//
//  Fuellen der globalen CRC-Tabelle 'crctab'
//
//*************************************************************************
{
  short int i, j;
  u16 accum, data;

  for (i = 0; i < 256; i++) // fill table with CRC of values...
  {
    accum = 0xffff;
    data = i;
    for (j = 0; j < 8; ++j)
    {
      if ((data ^ accum) & 0x0001) // if msb of data^accum is TRUE
        accum = (accum >> 1) ^ 0x8408; // then shift and subtract poly
      else
        accum >>= 1; // otherwise: transparent shift
      data >>= 1; // move up next bit for XOR
    }
    crctab[i] = accum;
  }
}

/*---------------------------------------------------------------------------*/

static void kissisr (chkiss_t *k)
//*************************************************************************
//
//  Lese- und Schreiboperationen am Device
//
//*************************************************************************
{
  int active = 1;
  int counter = 0;
  int num;

  while (active && counter < 50)
  {
    active = 0;
    if (((k->rxin + 1) % RXLEN) == k->rxout)
    {
      trace(serious, "l1kiss", "RX-BUFFER-Overflow");
      break;
    }
    num = w_read(k->fd, k->rxbuf + k->rxin, RXLEN - k->rxin);
    if (num > 0)
    {
      k->rxin += num;
      if (k->rxin == RXLEN) k->rxin = 0;
      active = 1;
      k->rx_errno = 0;
    }
#ifndef _WIN32
    else if (errno && errno != k->rx_errno && errno != EAGAIN)
    {
      trace(report, "l1kiss", "rx_read: %d %s", errno, strerror(errno));
      k->rx_errno = errno;
      if (errno == EIO) restart_kiss = 1; // DG2FEF
    }
#endif
    if (k->txout != k->txin)
    {
      active = 1;
      if (k->txin > k->txout) num = k->txin - k->txout;
      else num = TXLEN - k->txout;
      num = w_write(k->fd, k->txbuf + k->txout, num);
      if (num > 0)
      {
        k->txout += num;
        if (k->txout == TXLEN) k->txout = 0;
        k->tx_errno = 0;
/*
#ifndef _WIN32
        if (errno == EIO) // dg2fef
          restart_kiss = 1;
#endif
*/
      }
#ifndef _WIN32
      else if (errno && errno != k->tx_errno && errno != EAGAIN)
      {
        trace(report, "l1kiss", "tx_write: %d %s", errno, strerror(errno));
        k->tx_errno = errno;
      }
#endif
    }
    else k->txon = 0;
    counter++;
  }
}

/*---------------------------------------------------------------------------*/

static int getserial (chkiss_t *k)
//*************************************************************************
//
//  1 Byte aus dem RX-Puffer holen
//
//*************************************************************************
{
  int retwert = EOF;

  if (k->rxin != k->rxout)
  {
    retwert = k->rxbuf[k->rxout++];
    if (k->rxout == RXLEN) k->rxout = 0;
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

static int putserial (int ch, chkiss_t *k)
//*************************************************************************
//
//  1 Byte in den TX-Puffer schreiben
//
//*************************************************************************
{
  k->txon = 1;
  static int er = 0;

  if (((k->txin + 1) % TXLEN) == k->txout)
  {
    if (! er)
    {
      trace(serious, "l1kiss", "TX-BUFFER-Overflow");
      er = 1;
    }
    return 0;
  }
  er = 0;
  k->txbuf[k->txin++] = ch;
  if (k->txin == TXLEN) k->txin = 0;
  return 1;
}

/*---------------------------------------------------------------------------*/

static void put_ubyte (char c, chkiss_t *k)
//*************************************************************************
//
//  Sendet und konvertiert ein Byte fuer KISS (incl. CRC)
//
//*************************************************************************
{
  if (c == FEND)
  {
    putserial(FESC, k);
    putserial(TFEND, k);
  }
  else
  {
    putserial(c, k);
    if (c == FESC) putserial(TFESC, k);
  }
  k->tx_crc = (k->tx_crc << 8) ^ crctab[(k->tx_crc >> 8) ^ ((u16) (c & 255))];
}

/*---------------------------------------------------------------------------*/

static void rbyte (char c, chkiss_t *k)
//*************************************************************************
//
//  Holt ein Byte fuer den Kissmode (inc. CRC)
//
//*************************************************************************
{
  if (! k->rxloc && bfree) alloc(k->rxloc);
  if (k->rxloc && (k->rxindex < MAXLEN))
  {
    k->rxloc->inhalt[k->rxindex++] = c;
    k->rx_crc = (k->rx_crc << 8) ^ crctab[(k->rx_crc >> 8) ^ c];
  }
}

/*---------------------------------------------------------------------------*/

static void kissbuffer (chkiss_t *k)
//*************************************************************************
//
//  Bufferverwaltung fuer KISS-Betrieb
//
//*************************************************************************
{
  static int rxbyte;

  while ((rxbyte = getserial(k)) != EOF) // rx ist was angekommen
  {
    switch (k->rx_state)
    {
      case 0: // Frame Anfang erwartet
        if (rxbyte == FEND) k->rx_state = 1; // Frame Anfang entdeckt
        break;
      case 1:
        if (rxbyte != FEND) // FEND-Zeichen ignorieren
        {
          if (rxbyte & 15) // nur 0 entspricht Datenframe
          {
            k->rx_state = 0; // ist ein Befehl, also ignorieren
            k->dcd = 0;
            break;
          }
          k->do_crc |= (rxbyte == CRCTYP);
          k->rx_crc = (k->rx_crc << 8) ^ crctab[(k->rx_crc >> 8) ^ rxbyte];
          k->rx_state = 3; // naeaechster Status: Frame empfangen
          k->dcd = 1;
        }
        break;
      case 3:
        switch (rxbyte)
        {
          case FEND: // Frame-Ende erreicht?
            if (k->rxloc && (k->rxindex > 14))
            {
              if (! k->do_crc || k->rx_crc == 0x7070)
              {
                k->rxloc->channel = k->channel;
                k->rxloc->len = k->rxindex - 2 * k->do_crc; // Laenge eintragen
                k->rxloc->txdtime = 1; // txd bei KISS nicht messbar
                vornehin(k->rxloc, rxout);
              }
              //else
              //  trace(report, "l1kiss", "crc error len=%d", k->rxindex);
            }
            k->rxindex = 0; // fuer naechstes Frame
            k->rx_crc = 0xffff;
            k->dcd = 0;
            k->rx_state = 1; // warten auf naechstes Frame
            break;
          case FESC: k->rx_state = 4; break;
          default: rbyte(rxbyte, k);
        }
        break;
      case 4:
        if (rxbyte == TFEND) rbyte(FEND, k);
        if (rxbyte == TFESC) rbyte(FESC, k);
        k->rx_state = 3;
        break;
    }
  }
}

/*---------------------------------------------------------------------------*/

static void putsstring (char *str, chkiss_t *k)
//*************************************************************************
//
//  Sendet einen String zum KISS-TNC
//
//*************************************************************************
{
  while (*str) putserial(*(str++), k);
}

/*---------------------------------------------------------------------------*/

static void set_tnc (int pnum, int parameter, chkiss_t *k)
//*************************************************************************
//
//  Sendet einen Parameter zum KISS-TNC
//
//*************************************************************************
{
  putserial(FEND, k);
  putserial(pnum, k);
  putserial(parameter, k);
  putserial(FEND, k);
}

/*---------------------------------------------------------------------------*/

static void set_parameter (chkiss_t *k)
//*************************************************************************
//
//  Sendet die eingestellten L1-Parameter zum KISS-TNC
//  schaltet vorher den TNC in den Kissmode
//
//*************************************************************************
{
  char ss[10] = { 13, 0x11, 13, 13, 27, '@', 'K', 13, 0};
  unsigned int i;

  putsstring(ss, k);
  for (i = 0; i < 1000; i++) putserial(FEND, k);
  set_tnc(1, k->m->txdelay, k);
  set_tnc(2, 64, k);
  set_tnc(3, k->m->dwait, k);
  set_tnc(4, k->m->txtail, k);
  set_tnc(5, k->m->d, k);
}

/*---------------------------------------------------------------------------*/

//*************************************************************************
//
//  Interface-Routinen
//
//*************************************************************************

/*---------------------------------------------------------------------------*/

int l1state_kiss (int channel)
//*************************************************************************
//
//  gibt den Modem-Status eines Kanals zurueck
//
//*************************************************************************
{
  int mask = 0;
  chkiss_t *k = &ki[channel];

  if (k->dcd)       mask |= CH_DCD;
  if (k->txon)      mask |= CH_PTT;
  if (0)            mask |= CH_TXB;
  /*if (k->m->d)*/  mask |= CH_FDX;
  if (rxout)        mask |= CH_RXB;
  return mask;
}

/*---------------------------------------------------------------------------*/

void pactx_kiss (frame_t *packl)
//*************************************************************************
//
//  legt ein Frame in den Sendepuffer
//
//*************************************************************************
{
  unsigned int i;
  chkiss_t *k = &ki[packl->channel];

  if (k->fd != EOF)
  {
    putserial(FEND, k);
    if (k->do_crc)
    {
      k->tx_crc = 0xffff;
      put_ubyte(CRCTYP, k);
    }
    else
      put_ubyte(0, k);
    for (i = 0; i < packl->len; i++)
      put_ubyte(packl->inhalt[i], k);
    if (k->do_crc)
    {
      u16 uu = k->tx_crc;
      put_ubyte(uu / 256, k);
      put_ubyte(uu & 255, k);
    }
    putserial(FEND, k);
    packl->next = NULL;
    packl->channel |= 0x80;
    vornehin(packl, rxout);
  }
}

/*---------------------------------------------------------------------------*/

int l1init_kiss (int einschalt)
//*************************************************************************
//
//  Initialisiert den Kissmode bzw. schaltet ihn wieder ab
//
//*************************************************************************
{
#ifdef _WIN32
  int channel;

  for (channel = 0; channel < KANAELE; channel++)
  {
    chkiss_t *k = &ki[channel];
    l1_mode_t *m = &w.mode[channel];
    if (m->chmode == c_kiss)
    {
      if (einschalt)
      {
        memset(k, 0, sizeof(chkiss_t));
        k->m = m;
        k->rx_crc = 0xffff;
        k->tx_crc = 0xffff;
        k->channel = channel;
        k->rxbuf = (char *) malloc (RXLEN);
        k->txbuf = (char *) malloc (TXLEN);
        if (! k->rxbuf || ! k->txbuf)
          trace(tr_abbruch, "l1kiss", "malloc fail");
        k->rxloc = NULL; // Zeiger auf das im Moment empfangene Frame
        k->rxindex = 0;
        k->do_crc = m->c;
        init_crc();
        k->fd = open_win(m->device, k, m->hbaud);
        if (k->fd == EOF)
        {
          trace(serious, "l1kiss", "%s: %s", m->device, strerror(errno));
          return 0;
        }
        set_parameter(k);
        strupr(m->device); // COM1 oder COM2 muessen GROSSGESCHRIEBEN sein
        trace(report, "l1kiss", "%s is at %d Bd", m->device, m->hbaud);
        trace(report, "l1kiss", "initialised to channel %d", k->channel);
      }
      else s_close(k->fd);
    }
    else k->fd = EOF;
  }
#else
  struct termios mode;
  unsigned int channel;

  for (channel = 0; channel < KANAELE; channel++)
  {
    chkiss_t *k = &ki[channel];
    l1_mode_t *mm = &w.mode[channel];
    if (mm->chmode == c_kiss)
    {
      if (einschalt)
      {
        memset(k, 0, sizeof(chkiss_t));
        k->m = mm;
        k->rx_crc = 0xffff;
        k->tx_crc = 0xffff;
        k->channel = channel;
        k->fd = open(mm->device, O_NONBLOCK|O_RDWR);
        if (k->fd == EOF)
        {
          trace(serious, "l1kiss", "%s: %d %s", mm->device, errno,
                                                strerror(errno));
          return 0;
        }
        s_set(k->fd, mm->device, "lrb");
        tcgetattr(k->fd,&mode);
        /* Raw mode. */
#ifdef __LINUX__
        cfmakeraw(&mode);
#else
        mode.c_iflag = 0;
        mode.c_oflag &= ~OPOST;
        mode.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO
                               | ECHOE | ECHOK | ECHONL);
        mode.c_cc[VMIN] = 1;
        mode.c_cc[VTIME] = 0;
#endif
        speed_t hbaud = B9600;
        switch (mm->hbaud)
        {
          case 1200:   hbaud = B1200;   break;
          case 2400:   hbaud = B2400;   break;
          case 4800:   hbaud = B4800;   break;
          case 9600:   hbaud = B9600;   break;
          case 19200:  hbaud = B19200;  break;
          case 38400:  hbaud = B38400;  break;
          case 57600:  hbaud = B57600;  break;
          case 115200: hbaud = B115200; break;
          default:
            trace(serious, "l1kiss", "%d: invalid baud rate, taking 9600Bd",
                                      mm->hbaud);
            mm->hbaud = 9600;
        }
        cfsetospeed(&mode, hbaud);
        cfsetispeed(&mode, hbaud);
        tcsetattr(k->fd, TCSANOW, &mode);
        k->rxbuf = (char *) malloc(RXLEN);
        k->txbuf = (char *) malloc(TXLEN);
        if (! k->rxbuf || ! k->txbuf)
          trace(tr_abbruch, "l1kiss", "malloc fail");
        k->rxloc = NULL; // Zeiger auf das im Moment empfangene Frame
        k->rxindex = 0;
        k->do_crc = mm->c;
        init_crc();
        set_parameter(k);
        trace(report, "l1kiss", "%s is at %d Bd", mm->device, mm->hbaud);
        trace(report, "l1kiss", "initialised to channel %d", k->channel);
        if (! mm->c
            && //err-msg only when serial line
               (strstr(mm->device, "cua") || strstr(mm->device, "ttyS") ))
          trace(serious, "l1kiss", "Warning: NO CRC on ch %d at %s",
                                   k->channel, mm->device);
     }
      else  // deinit
      {
        tcgetattr(k->fd, &mode);
        // Cooked mode.
        mode.c_iflag |= BRKINT | IGNPAR | ISTRIP | ICRNL | IXON;
        mode.c_oflag |= OPOST;
        mode.c_lflag |= ISIG | ICANON;
        tcsetattr(k->fd, TCSANOW, &mode);
        s_close(k->fd);
      }
    }
    else k->fd = EOF;
  }
#endif
  return 1;
}

/*---------------------------------------------------------------------------*/

void cyclic_kiss (int channel)
//*************************************************************************
//
//
//*************************************************************************
{
  chkiss_t *k = &ki[channel];
  if (k->fd != EOF)
  {
#ifndef _WIN32
    kissisr(k);
#endif
    kissbuffer(k);
  }
}

/*---------------------------------------------------------------------------*/
