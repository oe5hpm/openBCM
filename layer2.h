/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  Layer2-Driver


  ----------------------------------------------
  Definitions / Declarations for Data Link Layer
  ----------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/


#ifdef L2COMPILE
#define BQX 1
#endif

#define KISS      0         // 1 : Treiber aktiv
#define SCC       0
#define FSK       0
#define IPX       0

#define SCCKARTEN    4
#define SSID63       0      // 1 : SSID bis 63 zulassen
#define ATU          0      // 1 : Aenderungen von OE1ATU verwenden
#define MEMPAGES     8      // Anzahl der Schnipsel im Upper-Memory

#ifdef __FLAT__
#define PORTLIMIT    500    // max. zulaessige Ports
#else
#define PORTLIMIT    80     // max. zulaessige Ports
#endif
#define KANAELE      20     // max. zulaessige SCC-Kanaele
#define MAXLINKS     8      // Anzahl der mit L eintragbaren Rfz.
#define MHLEN        63     // (2^X)-1! Anzahl der Calls in der MH-Liste
#define STACKLEN     500

#define L2INT        0x47
#define RNRTIME      150    // bei RNR nur alle 30sec pollen

#ifdef BQX
#undef VNUMMER
#define VNUMMER "1.60o(bqx)"
#endif

#define MSGPORT  (PORTLIMIT-1)

//
//    AX.25 spezifische Definitionen
//
#define SABM 0x2f
#define DISC 0x43
#define UA   0x63
#define DM   0x0f
#define UI   0x03
#define RR   0x01
#define REJ  0x09
#define RNR  0x05
#define FRMR 0x87

//
// Im 'port_t' werden die portbezogenen Variablen gehalten. Hier
// liegen nur die reinen Layer2-Parameter, die fuer alle Ports getrennt
// global verwaltet werden.
//
// Die Port-Struktur wird ueber ein Pointerfeld referiert, in dem nach
// der Portnummer indiziert, ein Zeiger auf die jeweilige Struktur
// gefunden wird.
//
//
typedef struct port_t : public stream_t
{
  byte busy;         // Link kann keine weiteren Daten mehr aufnehmen
  byte rembusy;      // Gegenstation kann keine Daten aufnehmen
  byte infostop;     // Info gesendet, keine weitere Info senden
  byte vr;           // Empfangene Folgezaehlernummer
  byte vs;           // hoechstes bereitstehendes I-Frame+1
  byte v1s;          // hoechstes bestaetigtes I-Frame+1
  byte v2s;          // letztes gesendetes I-Frame+1
  byte pid;          // auf diesem Port verwendetes PID-Byte

  unsigned timer1;
  unsigned timer2;
  unsigned timer3;
  unsigned itimer;
  unsigned qtimer;
  unsigned roundtrip;
  unsigned disctimer;
  byte wasrej;
  byte qsumme[8];       // Quersummen gegen Sammler-Doppeleintraege
  frame_t *sammler[8];  // Zeiger auf Frames im Sammler
  frame_t *txinfout;    // Zeiger auf naechste zu sendende Daten
  frame_t *txinfptr[8]; // Zeiger auf zu sendende I-Frames
  frame_t *rxinfout;    // Zeiger auf empfangene Daten
  frame_t *rxinfloc;    // Zeiger auf isolierte RX-Daten
  port_t *downlink;
  byte reconnectflag;   // 0=kein Reconnect, 1=Node, 2=Gate
  unsigned short paclen;
#if WITHNUDL
  frame_t *nudlinfo;    // Zeiger auf Node-Frames
  nstate_t nstate;
  unsigned linkzeit;
#endif
} port_t;

typedef enum pf_t       // moegliche Werte fuer PF-Variable
{ command,
  response,
  poll,
  final,
  pf_v1
} pf_t;

//
//
//  Programmglobale Setup-Parameter
//
//
typedef struct l2_para_t
{
  l2_value_t val;
  l1_mode_t  mode[KANAELE+1];

  call_t   unproto[7];
  unsigned sccon;
  long unsigned zeit;
  byte     linkkanal[MAXLINKS];
  call_t   linkcall[MAXLINKS][7];
  byte     linkoption[MAXLINKS];
  long     lastl3reset[MAXLINKS];

  unsigned maxports;
  unsigned maxbuf;
  unsigned maxmon;
  unsigned maxrx;
  unsigned mhlen;
  unsigned vseg;
  byte     scckanal[SCCKARTEN][8];
  int      comkanal;
  int      kisskanal;
  int      fskkanal;
  int      ser96kanal;
  int      ipxkanal;
  int      axipkanal;
  int      internkanal;
  unsigned comnummer;
  unsigned combase;
  unsigned comint;
  unsigned ser96nummer;
  unsigned ser96base;
  unsigned ser96int;
  unsigned fsknummer;
  unsigned fskbase;
  unsigned fskint;
  unsigned ipxaktiv;
  unsigned axipaktiv;
  unsigned kissnummer;
  unsigned kissbase;
  unsigned kissint;
  byte     scckarten;
  unsigned scckanaele[SCCKARTEN];
  unsigned sccctl[SCCKARTEN];
  unsigned sccbase[SCCKARTEN];
  unsigned sccint[SCCKARTEN];

  call_t   boxcall[56];
  unsigned inidate;     // Datum von SCC.INI ganz am Ende, damit jede
  unsigned initime;     // Aenderung im Format von SCC.PAR erkannt wird
  unsigned kennung;
} l2_para_t;

/*
 *  globale Variablen fuer Layer 1 und 2
 */
extern byte *l2buf;                 /* Mehrzweckpuffer */
extern char *l1stack;
extern l2_para_t w;

extern mheard_t *l2_mhlist;
extern frame_t *bfree;
extern byte pdigifl;
extern int rekfl;
extern byte rxdaflag;
extern int  tfaellig;
extern port_t *pfeld[];
extern port_t *p;
extern char *stack;
extern unsigned nudlports;
extern unsigned ports;
extern unsigned channels;
extern int kisskanal;
extern int comkanal;
extern int glob_messages;
extern unsigned damatimer[];
extern byte     damafree[];

extern frame_t *deadlockbuffer;

extern int l2int;


#define l2call(arg) {_AX=arg;asm pushf;(*ifunc)();}

// l2main
void initax25(int);

/*
 * l1
 */
int  l1init(int);      /* int=1 Initialisierung, int=0 abschalten */
int  l1state(int);     /* Modem-Status:   int=kanal, 0=frei, 1=DCD, 2=SEND */
void pactx(frame_t *); /* Frame aussenden                   */


/*
 * l2util
 */
unsigned int      findport(int);
int      aflen(byte *);
void     setaf(byte *,byte *);
void     paccod(int kanal,byte *adress,byte ctrl,int pf,int inflen,byte *info);
void     dmsend(frame_t *);
void     rauswerfen(frame_t *packl,byte *text);
unsigned chainlen(frame_t *);
frame_t *hintenhol(frame_t *);
void     hintenweg(frame_t **buf);
frame_t *vonhinten(frame_t **);
int      meminit(void);
unsigned stackreport(void);
unsigned getlinks(void);
int      paralloc(void);
void     chroutstr(byte *s);
void     *xalloc(unsigned size);

#define  isuser(kanal) (w.userports & (1<<kanal))
#define  isdamamaster(packl) ((packl->inhalt[13]&0x20)==0)

// l2interf
void intinit(int ein);
int  l2aufruf(int ax,int bx,int cx,byte *dx);

//l2mhlist
void mhput(frame_t *packl);

/*
 * l2port
 */
void connect(byte *,byte *,int);
void disconnect(int);
void lreset(int caufbau);
void tryinfo(void);
void dobeacon(int start);
void paccodsup(pf_t pf,int pnummer);
void rnrfrei(int pnummer);

/*
 * l2info
 */
void iot_frame(frame_t *);
void monitor(frame_t *);
void linkmsg(linkmsg_t message);
void chrout(byte zeichen);

// l2mhlist
int  nachbar(byte *call);
int  incssid(call_t *src,call_t *dest,int modify);
int  router(byte *altesAdressfeld,byte *neuesAdressfeld);



