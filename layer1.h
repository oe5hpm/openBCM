/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  Layer1-Driver


  ---------------------------------------
  Definitions for physical layer routines
  ---------------------------------------


  Copyright (c) 12/91 Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/



typedef unsigned char byte;
typedef signed char   i8;

typedef signed short   int  i16;
typedef unsigned short int  u16;

// Tables for CRC checker
extern char chktabl[];
extern char chktabh[];

// some watchdog globals
extern int watch081;
extern int intrun;              // aus todos.asm

// used for l1scc-timing
typedef enum timerstate_t
{ t_idle,
  t_dwait,
  t_txdelay,
  t_tail,
  t_sending,
  t_calon,
  t_caloff
} timerstate_t;

//
//
// In den Kanal-Vars werden die SCC-Kanalbezogenen Variablen gehalten. Hier
// liegen nur die reinen Layer1-Parameter, die fuer alle Kanaele getrennt
// global verwaltet werden.
//
//
typedef struct channel_t
{ char     txon;         // Flag: Sender eingeschaltet
  char     calibrate;    // Flag: Nullen senden auf diesem Kanal
  char     timerstate;
  unsigned l1timer;      // Mehrzwecktimer
  unsigned rxdt;         // Timer fuer TXDELAY feststellen empfang
  unsigned txindex;      // Index innerhalb des momentanen Frames
  unsigned rxindex;      // Zeiger innerhalb des aktuellen RX-Frames
  frame_t *txout;   // Zeiger auf erstes Frame im Layer1-Buffer
  frame_t *txloc;   // Zeiger auf das momentan zu sendende Frame
  frame_t *rxloc;   // Zeiger auf das momentan empf.
  unsigned sccdata;
  unsigned sccctl;
  char     scckanal;
} channel_t;

extern channel_t kk[];


extern unsigned irqok_com;
extern unsigned irqok_kiss;
extern unsigned irqok_scc;
extern unsigned irqok_fsk;
extern unsigned irqok_ser96;
extern unsigned irqok_main;



extern frame_t *rxout;
extern frame_t *l1_rxbuf;
extern int l1_user_on;
#define L1_LOOP 15

//
// Prototyp fuer Layer 2 - Aufruf
//
int layer1(int service);
void layer1(void);

void     layer2(void);
unsigned chainlen(frame_t *pnt);

int  l1state(int kanal);

int  l1state_com(void);
void pactx_com(frame_t *packl);
int  l1init_com(int einschalt);
void cyclic_com(void);

int  l1state_scc(int kanal);
void pactx_scc(frame_t *packl);
int  l1init_scc(int einschalt);
void cyclic_scc(void);

int  l1state_kiss(int channel);
void pactx_kiss(frame_t *packl);
int  l1init_kiss(int einschalt);
void cyclic_kiss(int channel);

int  l1state_axip(int channel);
void pactx_axip(frame_t *packl);
int  l1init_axip(int einschalt);
void cyclic_axip(int channel);

int  l1state_fsk(void);
void pactx_fsk(frame_t *packl);
int  l1init_fsk(int einschalt);
void cyclic_fsk(void);

int  l1state_ser96(void);
void pactx_ser96(frame_t *packl);
int  l1init_ser96(int einschalt);
void cyclic_ser96(void);

void pactx_ipx(frame_t *packl);
int  l1init_ipx(int einschalt);
void mon_ipx(frame_t *packl);
void cyclic_ipx(void);


//
//  Makros fuer SCC-Referenzen
//
#define output(reg,wert)  (outportb(adr,reg),outportb(adr,wert))
#define input(reg)      (outportb(adr,reg),inportb(adr))

#define sync     (inportb(adr)&0x10)
//#define dcd      (inportb(adr)&0x08)
#define underrun (inportb(adr)&0x40)
#define txempty  (inportb(adr)&0x04)

#define rxavail  (inportb(adr)&0x01)
#define cts      (inportb(adr)&0x20)
#define abort    (inportb(adr)&0x80)

#define crcerr  (outportb(adr,1),(inportb(adr)&0x40))
#define framend (outportb(adr,1),(inportb(adr)&0x80))
#define overrun (outportb(adr,1),(inportb(adr)&0x20))

#ifndef __FLAT__

/*
 *   Makros zur dynamischen Speicherverwaltung
 */
#define vornehin(von,zu) \
{ asm pushf;         \
  disable();         \
  von->next=zu;   \
  zu=von;               \
  von=NULL;           \
  asm popf;           \
}


/*
 *  alloc    Dieses Makro besetzt einen Buffer.
 *
 *         Zwingende Voraussetzung ist, dass noch mindestens ein Buffer
 *         frei ist, andernfalls ist mit Absturz zu rechnen.
 *
 *         Ausserdem findet keine Verkettung statt, es wird lediglich
 *         ein isoliertes Element geschaffen.
 *
 */
#define alloc(neu)   \
{ asm pushf;     \
  disable();     \
  (neu)=bfree;       \
  bfree=bfree->next; \
  (neu)->next=NULL;  \
  asm popf;       \
}


/*
 *  dealloc  gibt einen Buffer frei.
 *
 *         Dieses Makro funktioniert nur bei isolierten Elementen,
 *         da der Nachfolgerzeiger verloren geht.
 *
 *         Es findet keine Pruefung statt, ob wirklich ein Buffer
 *         durch den Zeiger referiert wird, dies ist beim Aufruf
 *         sicherzustellen.
 *
 */
#define dealloc(alt)  \
{ asm pushf;      \
  disable();      \
  (alt)->next=bfree;  \
  bfree=(alt);  \
  (alt)=NULL;    \
  asm popf;        \
}

#else

/*
 *   Makros zur dynamischen Speicherverwaltung
 */
#define vornehin(von,zu)  (von->next=zu,zu=von,von=NULL)


/*
 *  alloc    Dieses Makro besetzt einen Buffer.
 *
 *         Zwingende Voraussetzung ist, dass noch mindestens ein Buffer
 *         frei ist, andernfalls ist mit Absturz zu rechnen.
 *
 *         Ausserdem findet keine Verkettung statt, es wird lediglich
 *         ein isoliertes Element geschaffen.
 *
 */
#define alloc(neu)      ((neu)=bfree,bfree=bfree->next,(neu)->next=NULL)


/*
 *  dealloc  gibt einen Buffer frei.
 *
 *         Dieses Makro funktioniert nur bei isolierten Elementen,
 *         da der Nachfolgerzeiger verloren geht.
 *
 *         Es findet keine Pruefung statt, ob wirklich ein Buffer
 *         durch den Zeiger referiert wird, dies ist beim Aufruf
 *         sicherzustellen.
 *
 */
#define dealloc(alt)      ((alt)->next=bfree,bfree=(alt),(alt)=NULL)

#endif


