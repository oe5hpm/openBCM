/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  Layer1-Driver


  ---------------------------------------------------
  Definitions / Declarations for whole BAYCOM-Project
  ---------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/


#include "baycom.h"
#include "l2host.h"

/**************************** Interface *******************************/

static unsigned streams = 0;
static unsigned l2_channels = 0;
static unsigned mhlen = 0;
static unsigned l2_parlen = 0;
static char far *l2_par = "";
static l1_mode_t far *l1_mode = NULL;
static l2_value_t far *l2_val = NULL;
static mheard_t far *mhlist = NULL;

/*---------------------------------------------------------------------------*/

static unsigned dummy = 0;
static stream_t far *stream = NULL;
//static int l2_version = 0;
static int l2found = 0;

#ifdef __DOS16__
static void far (far *ifunc) (void) = NULL;

/*---------------------------------------------------------------------------*/

void interrupt dummy_int (int _bp, int _di, int _si, int ds, int es,
                          int _dx, int _cx, int _bx, int _ax)
//*************************************************************************
//
//
//*************************************************************************
{
  _dx = _ax = 0;
}

/*---------------------------------------------------------------------------*/

static int isinstalled (void)
//*************************************************************************
//
//  sucht nach installiertem L2, setzt den INT-Vektor
//  und gibt zurueck, ob L2 gefunden wurde oder nicht
//
//*************************************************************************
{
  char far *adr;
  int i, j, intvec;

  for (intvec=0x2f ; intvec<0x100 ; intvec++)
  {
    adr = (char far *) getvect(intvec);
    for (i=0 ; i<30 ; i++)
    {
      j = 0;
      while (adr[i+j] == "OpenBCM"[j]) j++;
      if (j >= 6)
      {
        ifunc = (void far (far *)(void)) adr;
        //l2_version = adr[i+7];
        return adr[i+6];
      }
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

//*************************************************************************
//
//  Following service functions call the L2 interrupt
//  These functions might need to be modified for other compilers than
//  Borland C++ Version 3.1
//
//*************************************************************************


#pragma warn -rvl
static int l2_int (service_t func)
{
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return AX as integer
}

static int l2_int (service_t func, int port)
{
  _BX = port;
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return AX as integer
}

static int l2_int (service_t func, int port, int c)
{
  _CX = c;
  _BX = port;
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return AX as integer
}

static int l2_int (service_t func, int port, byte far *p)
{
  _CX = FP_OFF(p);
  _DX = FP_SEG(p);
  _BX = port;
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return AX as integer
}

static long l2_long (service_t func)
{
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return DX:AX long integer
}

static void far *l2_ptr (service_t func)
{
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return DX:AX as far pointer
}

static void far *l2_ptr (service_t func, int port)
{
  _BX = port;
  _AX = func;
  asm pushf;
  (*ifunc)();
  // implicitely return DX:AX as far pointer
}

#pragma warn +rvl
#else
static int isinstalled (void)
{
  return 1;
}

/*---------------------------------------------------------------------------*/

//*************************************************************************
//
//  Following service functions call the L2 service for LINUX
//
//*************************************************************************

static int l2_int (service_t func)
{
  return l2aufruf(func, 0, 0, 0);
}

static int l2_int (service_t func, int port)
{
  return l2aufruf(func, port, 0, 0);
}

static int l2_int (service_t func, int port, int c)
{
  return l2aufruf(func, port, c, 0);
}

static int l2_int (service_t func, int port, byte far *pp)
{
  return l2aufruf(func, port, 0, pp);
}

static long l2_long (service_t func)
{
  return l2aufruf(func, 0, 0, 0);
}

static void far *l2_ptr (service_t func)
{
  return (void *) l2aufruf(func, 0, 0, 0);
}

static void far *l2_ptr (service_t func, int port)
{
  return (void *) l2aufruf(func, port, 0, 0);
}
#endif

/*---------------------------------------------------------------------------*/

void tnc_dobeacon (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found)
    l2_int(l2_put_free, -1);
}

/*---------------------------------------------------------------------------*/

void tnc_flush (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found)
    l2_int(l2_put_flush, stream->number);
  else
    f_tnc_flush();
}

/*---------------------------------------------------------------------------*/

/*not used
void tnc_putf (char *format, ...)
// *************************************************************************
//
//
// *************************************************************************
{
  va_list argpoint;
  byte cbuf[100];
  byte *cb = cbuf;

  va_start(argpoint, format);
  vsprintf((char *) cbuf, format, argpoint);
  va_end(argpoint);
  if (l2found)
    while (*cb) l2_int(l2_put_char, stream->number, *(cb++));
  else
    while (*cb) f_tnc_put(*(cb++));
}
*/

/*---------------------------------------------------------------------------*/

void tnc_put (byte a)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found) l2_int(l2_put_char, stream->number, a);
  else f_tnc_put(a);
}

/*---------------------------------------------------------------------------*/

void tnc_set_pid (byte newpid)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found) l2_int(l2_set_pid, stream->number, newpid);
//  else f_tnc_set_pid(newpid);
}

/*---------------------------------------------------------------------------*/

int tnc_putfree (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found) return l2_int(l2_put_free, stream->number);
  else return f_tnc_putfree();
}

/*---------------------------------------------------------------------------*/

int tnc_get (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found) return l2_int(l2_get_char, stream->number);
  else return f_tnc_get();
}

/*---------------------------------------------------------------------------*/

int tnc_get_msg (void)
//*************************************************************************
//
//
//*************************************************************************
{
  static int first = 1;

  if (l2found)
  {
    if (first)
    {
      first = 0;
      l2_int(l2_set_msg);
    }
    return l2_int(l2_get_msg, stream->number);
  }
  else
    return f_tnc_get_msg();
}

/*---------------------------------------------------------------------------*/

void tnc_connect (char *mycall, char *call)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found)
  {
    byte buf[71];
    *buf = 0;
    if (get_lstate() < info_transfer)
    {
      if (mycall)
        readcall(mycall, buf + 1);
      else
        memcpy(buf + 1, l2_val->mycall, 7);
      if (readclist(call, buf + 8, 8))
        l2_int(l2_connect_, stream->number, (byte far *) buf);
    }
  }
  else
    f_tnc_connect(mycall, call);
}

/*---------------------------------------------------------------------------*/

void tnc_disconnect (int mode)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found)
    l2_int(l2_disconnect, stream->number, mode);
  else
    f_tnc_disconnect(mode);
}

/*---------------------------------------------------------------------------*/

void tnc_exit (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (l2found)
    l2_int(l2_set_terminal, 0);
}

/*---------------------------------------------------------------------------*/

int tnc_init (unsigned int maxstreams)
//*************************************************************************
//
//
//*************************************************************************
{
  if (isinstalled())
  {
    l2found = 1;
    l1_mode = (l1_mode_t far *) l2_ptr(l2_ptr_l1_mode, 0);
    l2_val = (l2_value_t far *) l2_ptr(l2_ptr_values);
    mhlist = (mheard_t far *) l2_ptr(l2_ptr_mheard);
    l2_par = (char far *) l2_ptr(l2_ptr_par);
    mhlen = l2_int(l2_get_mhlen);
    l2_parlen = l2_int(l2_get_parlen);
    streams = l2_int(l2_get_streams);
    l2_channels = l2_int(l2_get_channels);
    if (streams > maxstreams) streams = maxstreams;
    if (l2_int(l2_set_terminal, 2)) return -2;
    l2_int(l2_set_terminal, 1);
    atexit(tnc_exit);
    return streams;
  }
  else
    return f_tnc_init(maxstreams);
}

/*---------------------------------------------------------------------------*/

char far *get_partner (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->partner;
  else return f_get_partner();
}


/*---------------------------------------------------------------------------*/

lstate_t get_lstate (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->lstate;
  else return f_get_lstate();
}

/*---------------------------------------------------------------------------*/

char *get_mycall (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found)
  {
    static char mybuf[11];
    if (stream->lstate > disconnected) callasc(stream->adrfield + 7, mybuf);
    else callasc(l2_val->mycall, mybuf);
    return mybuf;
  }
  else return f_get_mycall();
}

/*---------------------------------------------------------------------------*/

int get_outstanding (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->outstanding;
  else return f_get_outstanding();
}

/*---------------------------------------------------------------------------*/

int get_retries (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->retries;
  else return f_get_retries();
}

/*---------------------------------------------------------------------------*/

int get_l1state (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return l2_int(l2_get_l1state, get_channel());
  else return f_get_l1state();
}

/*---------------------------------------------------------------------------*/

unsigned int get_channel (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->channel;
  else return f_get_channel();
}

/*---------------------------------------------------------------------------*/

int get_outgoing (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->outgoing;
  else return f_get_outgoing();
}

/*---------------------------------------------------------------------------*/

int get_frack (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->frack * 2;
  else return f_get_frack();
}

/*---------------------------------------------------------------------------*/

int get_free (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  return l2_int(l2_get_free);
}

/*---------------------------------------------------------------------------*/

char *get_concall (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found)
  {
    static char buf[80];
    unsigned int i = 13;
    callasc(stream->adrfield, buf);
    while (! (stream->adrfield[i] & 1))
    {
      strcat(buf, " ");
      callasc(stream->adrfield + i + 1, buf + strlen(buf));
      i += 7;
    }
    return buf;
  }
  else return f_get_concall();
}

/*---------------------------------------------------------------------------*/

time_t get_ticker (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return l2_long(l2_get_ticker);
  else return time(NULL);
}

/*---------------------------------------------------------------------------*/

void set_partner (char *call)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (strlen(call) < 10)
  {
    if (l2found) strcpy(stream->partner, call);
    else f_set_partner(call);
  }
}

/*---------------------------------------------------------------------------*/

void set_conok (int on)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  stream->mode.open = on;
}

/*---------------------------------------------------------------------------*/

void set_messages (int on)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  stream->mode.messages = on;
}

/*---------------------------------------------------------------------------*/

void set_mycall (char *s)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (l2found) readclist(s, l2_val->mycall, 4);
}

/*---------------------------------------------------------------------------*/

void set_channel (unsigned int channel)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (l2found)
  {
    if (channel >= l2_channels) channel = 0;
    stream->channel = channel;
  }
  else
    f_set_channel(channel);
}

/*---------------------------------------------------------------------------*/

void set_stream (unsigned int stream_number)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (l2found)
  {
    if (! dummy)
    {
      if (stream_number < streams)
        stream = (stream_t far *) l2_ptr(l2_ptr_stream, stream_number);
      else
        trace(serious, "l2host", "illegal stream %d", stream_number);
    }
  }
  else
    f_set_stream(stream_number);
}

/*---------------------------------------------------------------------------*/

void set_qsonum (int qsonum, int lstate)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (l2found)
  {
    stream->number = qsonum;
  /*
    //q->last_lstate = lstate;
    const FRAME far *fr = l2_get_f(qsonum);
    strcall(q->mycall, fr->source);
    stream->channel = fr->kanal;
    */
  }
  else f_set_qsonum(qsonum, lstate);
}

/*---------------------------------------------------------------------------*/

int get_qsonum (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (l2found) return stream->number;
  else return f_get_qsonum();
}

/*---------------------------------------------------------------------------*/

void set_monitor (char *s)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (! l2found) f_set_monitor(s);
/*
  if (l2found)
    l2_clr_monitor();
  else
    return;
  strupr(s);
  memset(&tr, 0, sizeof(TRACE));
  if (strcmp(s, "OFF")==0)
    return;
  tr.trxfilter = 3;
  tr.kanal = (-1);
  while (s[0])
  {
    s += blkill(s);
    if(s[0] == 0) break;
    switch (s[0])
    {
      case '>': tr.trxfilter = 2; break;
      case '<': tr.trxfilter = 1; break;
      case '#': tr.typfilter |= TRACETYP_NO_S; break;
      case '$': tr.typfilter |= TRACETYP_NO_TEXT; break;
      default:
        if (s[0] >= '-' && s[0] <= '9' && s[1] <= '9')
        {
          tr.kanal = atoi(s);
          while (s[0]>' ') s++;
        }
        else if (s[0] == 'O' && s[1] == 'N' && s[2] <= ' ')
        {
          tr.kanal = (-1);
          s += 2;
        }
        else if (isalnum(s[0]))
        {
          int i = 0;
          while (s[0] >= '0' && i < 6)
          {
            tr.call[i] = s[0];
            s++;
            i++;
          }
          while (i < 6) tr.call[i++] = ' ';
          if (s[0] == '-')
          {
            s++;
            i = atoi(s);
            tr.call[6] = i + 0x30;
            while (s[0]>' ') s++;
          }
        }
    }
    s++;
  }
  */
}



/*---------------------------------------------------------------------------*/

char *get_monitor (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  static char erg[30];

  if (! l2found) return f_get_monitor();
  /*
  if (tr.trxfilter == 0)
    return "OFF";
  if (! l2found || tr.kanal == (-1))
    strcpy(erg, "ON");
  else
    sprintf(erg, "%d", tr.kanal);
  if (tr.call[0])
  {
    strcat(erg, " ");
    strcall(erg+strlen(erg), tr.call);
  }
  if (tr.trxfilter != 3)
  { if (tr.trxfilter == 2)
      strcat(erg, " >");
    if (tr.trxfilter == 1)
      strcat(erg, " <");
  }
  if (tr.typfilter & TRACETYP_NO_S)
    strcat(erg, " #");
  if (tr.typfilter & TRACETYP_NO_TEXT)
    strcat(erg, " $");
  */
  return erg;
}

/*---------------------------------------------------------------------------*/

void set_l1_txdelay (unsigned int channel, int txd)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (! l2found) f_set_l1_txdelay(channel, txd);
  /*
  if(l2found && ch_active(channel))
    set_txdelay(channel,txd);
  */
}

/*---------------------------------------------------------------------------*/

int get_l1_txdelay (unsigned int channel)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (! l2found) return f_get_l1_txdelay(channel);
 /*
  if(l2found && ch_active(channel))
    return get_txdelay(channel);
  else  */
    return 0;
}

/*---------------------------------------------------------------------------*/

#define MODE_x   0x8000
#define MODE_a   0x0100
#define MODE_y   0x0200
#define MODE_m   0x0400
#define MODE_s   0x0800
#define MODE_d   0x0080
#define MODE_r   0x0040
#define MODE_t   0x0020
#define MODE_z   0x0010
#define MODE_u   0x0008
#define MODE_b   0x0004
#define MODE_c   0x0002
#define MODE_off 0x0001   // Special: Wenn 1, Kanal abgeschaltet

/*---------------------------------------------------------------------------*/

void set_l1_mode (unsigned int channel, char *cp)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  if (! l2found) f_set_l1_mode(channel, cp);
/*
  long baud=atol(cp);
  unsigned mode=0;
  if(l2found && ch_active(channel))
  { strlwr(cp);
    while(*cp)
    { switch(*cp)
      { case 'u': mode|=MODE_u; break;
        case 't': mode|=MODE_t; break;
        case 'r': mode|=MODE_r; break;
        case 'z': mode|=MODE_z; break;
        case 'd': mode|=MODE_d; break;
        case 'c': mode|=MODE_c; break;
        case 'a': mode|=MODE_a; break;
        case 'm': mode|=MODE_m; break;
        case 'y': mode|=MODE_y; break;
        case 's': mode|=MODE_s; break;
        case 'x': mode|=MODE_x; break;
        case 'b': mode|=MODE_b; break;
        case '-': mode|=MODE_off; break;
      }
      m++;
    }
    set_chmode(channel,(u16)mode,(u16)(baud/100));
  }
  */
}


/*---------------------------------------------------------------------------*/

char *get_l1_mode (unsigned int channel)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  if (! l2found) return f_get_l1_mode(channel);
  /*
  static char erg[30];
  unsigned mode;

  if (l2found && ch_active(channel))
  {
    int baud = get_chbaud(channel);
    if (baud)
      sprintf(erg, "%ld", 100L*baud);
    else
      erg[0] = 0;
    mode = get_chmode(channel);
    if(mode == 0 && strlen(erg)<2)
      return "";
    if(mode & MODE_s)
      strcat(erg, "s");
    if(mode & MODE_m)
      strcat(erg, "m");
    if(mode & MODE_y)
      strcat(erg, "y");
    if(mode & MODE_u)
      strcat(erg, "u");
    if(mode & MODE_d)
      strcat(erg, "d");
    if(mode & MODE_t)
      strcat(erg, "t");
    if(mode & MODE_r)
      strcat(erg, "r");
    if(mode & MODE_z)
      strcat(erg, "z");
    if(mode & MODE_c)
      strcat(erg, "c");
    if(mode & MODE_a)
      strcat(erg, "a");
    if(mode & MODE_x)
      strcat(erg, "x");
    if(mode & MODE_b)
      strcat(erg, "b");
    if(mode & MODE_off)
      strcat(erg, "-");
    return erg;
  }
  else */
    return "";
}

/*---------------------------------------------------------------------------*/

void set_l1_xmitok (unsigned int channel, int xmitok)
//*************************************************************************
//
// management functions, SET-operation
//
//*************************************************************************
{
  return;
/*if(l2found && ch_active(channel))
  { unsigned mode=get_chmode(channel);
    unsigned baud=get_chbaud(channel);
    if(xmitok==0)
      mode|=MODE_x;
    else
      mode&=(~MODE_x);
    set_chmode(channel,mode,baud);
  } */
}

/*---------------------------------------------------------------------------*/

int get_l1_xmitok (unsigned int channel)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
 //if (l2found && ch_active(channel))
 //   return !(get_chmode(channel)&MODE_x);
 // else
    return (-1);
}

/*---------------------------------------------------------------------------*/

char *get_version (void)
//*************************************************************************
//
// management functions, GET-operation
//
//*************************************************************************
{
  static char v[20];
  if (l2found)
  {
    sprintf(v, "L2 %s", (char *) l2_ptr(l2_ptr_version));
    return v;
  }
  else
    return f_get_version();
}

/*---------------------------------------------------------------------------*/

