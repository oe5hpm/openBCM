/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -----------------------------------------------
  Definitions / Declarations L2 Service Interface
  -----------------------------------------------


  Copyright (c) 92-96 Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/



/*************************** Data Types *******************************/

#ifndef _L2_HOST_H
#define _L2_HOST_H

#define ALL_LINKS 256

// The following type is used whenever an ax.25 call sign shall be accessed
// It is only used for enhancing readability and therefore mapped to char
typedef byte call_t;

// Physical mode for a L1-channel. This list is subject to be extended
// if new devices are supported by l2.exe
typedef enum chmode_t
{ c_scc0,              // first scc card (uscc, 9k6uscc or uscc>8)
  c_scc1,
  c_scc2,
  c_scc3,              // 4th scc card
  c_rs232,             // com-port modem 1200Bd or 300Bd
  c_kiss,              // kiss-mode port
  c_fsk,               // PAR96 FSK modem on LPT port
  c_ser96,             // SER96 FSK modem on COM port
  c_ipx,               // LAN (Novell-IPX compatible driver)
  c_axip,              // AX over IP (Linux only)
  c_loop,
  c_mail,
  c_unassigned=255
} chmode_t;

// Setup parameter block for one physical channel
// A pointer to this structure is returned by function l2_ptr_l1_mode
typedef struct l1_mode_t
{ unsigned channel;      // the number of the physical channel
  unsigned hbaud;        // hdlc speed (if supported by the hardware)
  unsigned txdelay;
  unsigned dwait;
  unsigned maxframe;
  unsigned btime;        // beacon time
  unsigned txtail;       // time until transmitter turns off (10ms)
  unsigned tchannel;     // node only: transmit channel
  unsigned linkexcl;     // node only: channel is used for interlink
  unsigned reserved;     // for future expansion
  unsigned xmitok;       // flag: transmitter enabled
  call_t   badress[70];  // address field for beacon on this channel
  char     btext[80];    // beacon text
#ifdef __FLAT__
  char device[80];
  int  port;
  int  txport;
#endif
  chmode_t chmode;       // channel physical mode
  unsigned t:1;   // flag: transmit hardware clock
  unsigned r:1;   // flag: receive hardware clock
  unsigned z:1;   // flag: NRZ coding instead of NRZI
  unsigned d:1;   // flag: full duplex mode
  unsigned c:1;   // flag: software carrier detect
  unsigned m:1;   // flag: DAMA master enabled (not implemented)
} l1_mode_t;


// Setup parameter block for data link layer parameters
// A pointer to this structure is returned by function l2_ptr_l1_mode
typedef struct l2_value_t
{
  unsigned paclen;
  unsigned retry;
  unsigned frack;
  unsigned resptime;
  unsigned linktime;
  unsigned qtime;
  unsigned disctime;
  unsigned ipoll;
  unsigned itime;

  unsigned infocuton;
  unsigned headeron;
  unsigned blink;
  unsigned cwid;
  unsigned trx_channel;
  unsigned dama;
  unsigned mselect;
  unsigned reserved1;
  unsigned reserved2;
  unsigned reserved3;
  unsigned reserved4;

  long     monitor;
  long     reserved5;
  call_t   mcalls[70];
  call_t   mycall[35];
  call_t   dcall[7];
  call_t   ncall[8];
  call_t   cnot[70];
  char     l2ctext[80];
} l2_value_t;


typedef struct stream_mode_t
{ int l2ctext:1;
  int open:1;
  int messages:1;
  int tx_frame:1;
  int rx_frame:1;
} stream_mode_t;

typedef struct stream_t
{ unsigned number;
  lstate_t lstate;
  unsigned frack;         // time of frack-start in 100ms-intervals
  unsigned retries;       // remaining retries
  unsigned outstanding;   // not acknowledged info-frames
  unsigned channel;       // physical channel of stream
  unsigned outgoing;      // 0 incoming call    1 outgoing connection
  unsigned logintime;     // bios-minutes at beginning of connection
  unsigned mynumber;      // Nummer des verwendeten Mycalls aus Liste
  byte     adrfield[70];      // ax.25 address field (read only)
  char     partner[10];       // partnercall (ascii) read/write access
  stream_mode_t mode;
} stream_t;

#define MAXLEN 70+1+1+256+2     // Maximale Laenge eines Frames
//
// der Typ 'frame_t' wird generell fuer die Speicherung der Daten von
// Layer 1 und 2 verwendet. Bei kurzen Frames entsteht zwar viel Overhead,
// aber dafaer ist die Verwaltung recht schnell, weil sich fuer alle Typen
// von Daten die gleiche Struktur ergibt.
//
//
// Bei reinen Datenelementen, also keine AX.25-Frames, wird
//
// frame->len  als Einlesezeiger verwendet, und
// frame->index   als Auslesezeiger
//
// ist (frame->len==MAXLEN), so ist der Buffer voll belegt, in diesem
// Fall sollte in einem mit frame->next referierten Buffer die Fortsetzung
// der Daten stehen.
//
//
typedef struct frame_t
{ unsigned  len;      // Laenge des Frames brutto, als Adress+Ctl+Pid+Info
  unsigned  index;    // Offset vom Anfang des Adressfeldes zum Infofeld
  byte channel;   // SCC-Kanalnummer bei Empfang und Sendung
  byte txdtime;
  byte inhalt[MAXLEN+1]; // eigentliche Daten des Frames
  struct frame_t *next;  // Zeigt auf naechstes Frame in der Kette, bzw NULL
#if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
  unsigned segment;
#endif
} frame_t;

// The following type enumerates all services of L2. The service is called
// by using the appropriate Interrupt (determined by tnc_init())
// All parameters are supplied within processor registers AX,BX,CX,DX
// Far pointers to well-known structures are supplied within DX:AX. This is
// the same practice as used for far pointer returns of Borland C++ code.
//
// This list is subject to be extended in future versions of BayCom L2
//
typedef enum service_t
{ /* simple interface function calls */
  l2_put_char=128, // BX stream  CX char
  l2_put_flush,    // BX stream
  l2_put_free,     // BX stream          return AX
  l2_get_char,     // BX stream          return AX
  l2_get_valid,    // BX stream          return AX
  l2_connect_,     // BX stream      CX pseudodigis
  l2_disconnect,   // BX stream      CX mode
  l2_get_lstate,   // BX stream          return AX
  l2_get_outstanding,// BX stream              return AX
  /* enhanced interface function calls */
  l2_ptr_stream,   // BX stream          return *DX:AX (stream_t *)
  l2_put_frame,    // BX stream  *DX:CX info field (including PID) (char *)
  l2_get_msg,      // BX stream          return AX
  l2_get_frame,    // BX stream          return *DX:AX (char *)
  l2_get_l1state,  // BX channel                return AX
  l2_get_free,     //                      return AX
  l2_get_channels, //                      return AX
  l2_get_streams,  //                      return AX
  l2_get_connects, // BX channel or ALL_LINKS   return AX
  l2_get_mhlen,    //                      return AX
  l2_get_parlen,   //                      return AX
  l2_get_ticker,   //                      return DX:AX  (time_t)
  l2_get_newdata,  //                      return AX (stream)
  l2_set_terminal, // BX 0:terminal off  1:terminal on  2:return state
  l2_set_handler,  // *DX:CX void interrupt (*handler)(...) (AX=stream)
  l2_ptr_l1_mode,  // BX channel                return *DX:AX (l1_mode_t *)
  l2_ptr_values,   //                      return *DX:AX (l2_value_t *)
  l2_ptr_mheard,   //                      return *DX:AX (mheard_t *)
  l2_ptr_par,      //                      return *DX:AX (unknown *)
  l2_ptr_version,  //                      return *DX:AX (char *)
  l2_set_msg,      // no parameter
  l2_set_pid       // BX stream      CX PID

} service_t;       // service number is supplied in AX

/**************************** Interface *******************************/


//extern unsigned streams;
//extern unsigned channels;
//extern unsigned mhlen;
//extern unsigned l2_parlen;

//extern char       far *l2_par;
//extern l1_mode_t  far *l1_mode;
//extern l2_value_t far *l2_val;
//extern mheard_t   far *mhlist;

int f_tnc_init(int maxstreams);

/* service interface functions */

void f_tnc_flush(void);
void f_tnc_putf(char *format,...);
void f_tnc_put(byte a);
int  f_tnc_putfree(void);
int  f_tnc_putframe(frame_t *fr);
int  f_tnc_get(void);
int  f_tnc_get_msg(void);
int  f_tnc_getvalid(void);
int  f_tnc_getframe(frame_t *fr);
void f_tnc_connect(char *mycall,char *call);
void f_tnc_disconnect(int mode);

/* management functions, GET-operation */
char *f_get_partner(void);
lstate_t f_get_lstate(void);
int f_get_frack(void);
int f_get_outstanding(void);
int f_get_retries(void);
int f_get_channel(void);
int f_get_outgoing(void);
int f_get_l1state(void);
char *f_get_mycall(void);
char *f_get_concall(void);
unsigned f_get_logintime(void);

time_t f_get_ticker(void);
int f_get_channels(void);

char *f_get_l1_mode(int channel);
int f_get_l1_txdelay(int channel);
int f_get_l1_xmitok(int channel);
char *f_get_monitor(void);
int f_get_qsonum(void);
char *f_get_version(void);

/* management functions, SET-operation */
void f_set_partner(char *call);
void f_set_conok(int on);
void f_set_messages(int on);
void f_set_channel(int channel);
void f_set_stream(int stream_number);
void f_set_l1_mode(int channel,char *str);
void f_set_l1_txdelay(int channel,int txd);
void f_set_l1_xmitok(int channel,int xmitok);
void f_set_monitor(char *s);
void f_set_qsonum(int qsonum,int lstate);
#endif
