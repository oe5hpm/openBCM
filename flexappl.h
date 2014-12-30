/* Definitionen f. AX25-Applikationen auf FlexNet (C) 11/91 DK7WJ G.Jost */
/* definitions for AX25 applications with FlexNet (C) 11/91 DK7WJ G.Jost */
/* translation 08/95 by Tom Sailer HB9JNX */

/* Stand/Date: 31.12.96 */

#ifndef _FLEXTYPES_DEFINED
#define _FLEXTYPES_DEFINED
typedef unsigned char byte;
typedef signed char i8;
typedef short int i16;
typedef unsigned short int u16;
typedef signed long int i32;
typedef unsigned long int u32;
#endif

#ifdef _WIN32
#define far
#pragma pack(push, flexpack)
#pragma pack(1)
#endif

#define FLXCALL     7   /* Laenge FlexNet-Call, SSID in 7. Byte */
                        /* Length of a FlexNet callsign, SSID is in the 7th byte */
#define DIGIS       8   /* Max. Anzahl Digis */
                        /* maximum number of digipeaters */
#define STRCALL    16   /* Max. Laenge String v. strcall() incl. 0 */
                        /* maximum length of a string returned by strcall() including the terminating 0 */
#define STRPATH   131   /* Max. Laenge String v. strpath() incl. 0 */
                        /* maximum length of a string returned by strpath() including the terminating 0 */
#ifndef _FLEXAPPL
#define _FLEXAPPL

/* Struct f. Kanalstatistik */
/* struct for channel statistics */
typedef struct
    {
    u32 tx_error;          /* Underrun oder anderes Problem - underrun or some other problem */
    u32 rx_overrun;      /* Wenn Hardware das unterstuetzt - if the hardware supports it... */
    u32 rx_bufferoverflow;
    u32 tx_frames;        /* Gesamt gesendete Frames - the number of frames sent */
    u32 rx_frames;        /* Gesamt empfangene Frames - the number of frames received */
    u32 io_error;          /* Reset von IO-Device - number of resets of the IO device */
    u32 reserve[4];      /* f. Erweiterungen, erstmal 0 lassen! - reserved for extensions, leave these 0 */
    } L1_STATISTICS;

/* Framestructure */
typedef struct
    {
    byte dest[FLXCALL];   /* Zielcall - destination callsign */
    byte source[FLXCALL]; /* Absendercall - source callsign */
    byte digis;  /* Anzahl Digipeater (0-8) - number of digipeaters (0-8) */
    byte nextdigi;      /* Nummer des naechsten Digis (hat noch nicht) - number of the next digipeater (that has not already repeated) */
    byte kanal;  /* Kanalnummer - channel number */
    byte axv2;    /* TRUE wenn AX25 Version 2 - true if AX25 version 2 */
    byte pid;      /* Protocol Identifier */
    byte typ;      /* Frametyp siehe unten - frame type, see below */
    byte cmd;      /* TRUE wenn V2 und Command - true if V2 and command */
    byte pf;        /* TRUE wenn V2 und Poll/Final - true if V2 and poll/final */
    byte nr;        /* RX Sequence Number */
    byte ns;        /* TX Sequence Number */
    byte dama;    /* Flag: f. Monitor oder TX - flag if DAMA, used for tracing or transmission */
    byte tx;        /* f. Trace: 0 wenn RX - used for tracing, 0 if received */
    byte txdel;  /* f. Trace: gemessenes TxDelay - used for tracing, measured txdelay */
    byte reserved[3];   /* for future expansion, now 0 */
    u16 textlen;        /* Laenge Text wenn I, UI oder FRMR - length of the text if I, UI or FRMR */
    const byte far *text;/* Ptr auf Text - pointer to the text */
    u16 qsonum;  /* von L2: Frame gehoert zu QSO - from L2: frame belongs to this QSO */
    u16 partner_num;    /* von L2: Qsonummer bei Partner - from L2: QSO number at the peers side */
    byte digi[DIGIS][FLXCALL]; /* Digicalls, nur gueltig entsprechend Anz. 'digis' - callsigns of the digipeaters; only valid according to 'digis' */
    } FRAME;

/* Frame-Typen - frame types */
#define    I 1
#define   RR 2
#define  RNR 3
#define  REJ 4
#define SABM 5
#define DISC 6
#define   DM 7
#define   UA 8
#define FRMR 9
#define   UI 10

/* Infoblock aus L2 - info block from L2 */
typedef struct
    {
    i16 len;
    byte pid;
    byte text[256]; /* 256 ist Dummy, Laenge variabel (Maximum siehe MAXTEXT) */
                    /* 256 is a placeholder, length varies (maximum see MAXTEXT) */
    } INFO;

#define MAXTEXT 300   /* Max. Frametextlaenge, sendeseitig sollte man 256
                         nicht ueberschreiten! */
                      /* Maximum frame text length, the sender should not send
                         more than 256 bytes! */

/* Masken fuer den L1-Kanalstatus - bitmasks for L1 channel status */
#define CH_PTT 0x20
#define CH_DCD 0x10
#define CH_TBY 0x04

#define MODE_y   0x0200
#define MODE_a   0x0100
#define MODE_m   0x0400
#define MODE_s   0x0800
#define MODE_d   0x0080
#define MODE_r   0x0040
#define MODE_t   0x0020
#define MODE_z   0x0010
#define MODE_c   0x0002
#define MODE_off 0x0001   /* Special: Wenn 1, Kanal abgeschaltet */
                          /* special: if 1, channel is turned off */

/* Datenstruktur f. Filter - data structures for filter */
typedef struct
    {
    u16 ch_mask;     /* channels to trace must have 1 in their bit position */
    byte typfilter;     /*  1: RR/REJ/RNR ausblenden - filter out all RR/REJ/RNR
                           64: show received txdelay and qso numbers
                          128: I- und UI-Frames ohne Text ausgeben - report I and UI frames without text */
#define TRACETYP_NO_S       1   /* (auch kombinierbar) (may also be combined) */
#define TRACETYP_DIAG      64   /* show received txdelay and qso numbers */
#define TRACETYP_NO_TEXT  128
    byte trxfilter;     /* 0: Aus/off; 1: RX; 2: TX; 3: RX+TX */
    byte call[FLXCALL]; /* call[0] != 0: Nur dieses Call monitoren, muss
                           Quelle oder Ziel sein; SSID-Byte==0: alle SSIDs */
                        /* call[0] != 0: trace only this call, must be source
                           or destination; SSID byte == 0: all SSIDs */
    u16 handle;  /* Fuer interne Zwecke - for internal purposes only */
    } TRACE;

#ifdef __cplusplus
extern "C" {
#endif
/* --------------- Prototyp-Deklarationen -------------------- */
/* Initialisierung: Muss erster Aufruf der Applikation sein!
    Return: 1 wenn Mini-Kernel geladen und aktiviert und passende Version
            2 wenn Digi-Kernel geladen und aktiviert und passende Version
*/
/* Initialisation: Must be the first call of the application!
   Return value: 1 if the mini kernel is loaded and activated and a compatible version
                 2 if the digipeater kernel is loaded and activated and a compatible version
*/
byte ax_init(void);

/* Abmeldung: aufrufen beim Beenden der Applikation */
/* Uninitalize the kernel. Should be called when the application terminates */
byte ax_exit(void);

/* FlexNet-Versionsnummernstring abholen */
/* get the FlexNet version number string */
const byte far *l2_version(void);

#ifdef _WIN32
/* Returns 1 if FlexNet node is running */
byte check_node(void);
#endif

/* ---------------------- Abgehende Verbindungen ----------------------- */
/* Initiieren einer Verbindung
   l2_connect(String mit eigenem Rufzeichen, String mit Zielrufzeichen,
        letzeres ev. mit Angabe von Vias);
        Rufzeichen werden in Grossbuchstaben umgewandelt, "v" und "via"
        als 1. Digi wird ueberlesen
        z.B. "l2_connect("DK7WJ-2", "DB0GV v db0odw");
        ret: QSO-Nummer falls Speicherplatz vorhanden und Syntax ok */
/* set up a connection (virtual circuit)
   l2_connect(string with the source call (mycall), string with the destination
        calls with the digipeater calls if needed);
        The callsigns are converted to uppercase letters, "v" and "via" as
        first digipeater are removed.
        Example: l2_connect("DK7WJ-2", "DB0GV v db0odw");
        return value: the QSO number if there is enough memory and the syntax
                      is ok */
u16 l2_connect(const byte far *source, const byte far *destination);

/* QSO-Nummer einer Verbindung liefern, falls sie schon besteht
    src und destination siehe l2_connect(..)
    ret: QSO-Nummer falls gefunden, sonst 0. Beendete QSOs werden noch bis zu
    3 Minuten lang verwaltet. Ggf. muss also noch mit l2_state() geprueft
    werden, ob sie aktiv sind */
/* return the QSO number of a connection (virtual circuit), if it already
    exists; src and destination see l2_connect(...)
    return value: the QSO number if found, 0 otherwise. Terminated QSOs are
    kept 3 minutes. So the returned value should be checked with l2_state()
    if the QSO is still active */
u16 l2_check_path(byte far *source, byte far *destination);


/* ------- Ankommende Verbindungen ------ Incoming Connects --------------- */
/* Verbindungswunsch anmelden; muss zyklisch so oft wie moeglich aufgerufen
   werden, wenn erwuenscht
   Call liefert gleichzeitig QSO-Nummer eines angekommenen Verbindungswunsches
   l2_get_sabm (Rufzeichenstring)
             ret: QSO-Nummer falls jemand Verbindung angefordert hat!
                  0 wenn nicht
    Das Rufzeichen der anrufenden Station und sein Pfad kann mit l2_get_f()
    ermittelt werden, dann muss mit l2_sabmresp(num) oder l2_sabm_dm(num)
    reagiert werden, ausser wenn man sich totstellen will */
/* Ask for incoming connects; should be called as often as possible, if
   incoming connects are welcome.
   The call returns the QSO number of the incoming connect request.
   l2_get_sabm(callsign string)
     return value: QSO number if someone requested a connect
                   0 if not
   The callsign of the calling station and its path may be found with
   l2_get_f(), then one should answer with l2_sabmresp(num) or l2_sabm_dm(num)
   if one does not want to keep silent */
u16 l2_get_sabm(byte far *call);

/* Beantworte Verbindungswunsch positiv */
/* acknowledge (positively) a connect request */
#define l2_sabmresp(qsonum)      (l2_bef( 2, qsonum))

/* Lehne Verbindungswunsch mit DM (Busy) ab */
/* deny a connect request (send DM (busy)) */
#define l2_sabm_dm(qsonum)       (l2_bef( 3, qsonum))

/* ------------------------ Unproto-Behandlung ------------------------- */
/* Unproto-Frame senden; src und destination siehe l2_connect(..)
   Return: 1 wenn ok oder wenn falsche Kanalnummer oder sonstiger Fehler (!)
           0 wenn kein Platz in TX-Puffer */
/* send an unproto (UI) frame; src and destination see l2_connect(...)
   Return value: 1 if ok or if channel number invalid or if any other error (!)
                 0 if there is no place in the TX buffer */
byte l2_send_ui(const byte far *src, const byte far *destination, byte pid,
                byte poll, u16 textlen, const byte far *text);

/* Unproto-Frame empfangen
    Return: *FRAME wenn etwas empfangen, sonst 0 */
/* receive unproto frames
    Return value: *FRAME if something received, 0 if not */
const FRAME far *l2_get_ui(byte far *call);
/* ------- Sende/Empfangsroutinen ---- receive/trasmit routines --------- */

/* Bytes zwecks Sendung allokieren; falls vom letzten Aufruf noch Platz frei,
    wird er subtrahiert; man kann also mit Reserve allokieren!
   l2_ialloc(nummer, len); ret: TRUE/FALSE */
/* allocate memory for the sender; if there is memory left from the last call,
    it is subtracted; thus one may allocate spare memory!
    return value: TRUE (if succeeded) / FALSE */
i8 l2_ialloc(u16 qsonum, i16 len);

/* formatierten Bytestream in QSO senden, geht nach Format ueber l2_puts() */
/* send a formatted byte stream to the QSO, passes l2_puts() after formatting */
void l2_printf(u16 qsonum, const byte *fmt, ...);

/* Bytestream senden, wandelt \n in \r und bearbeitet TABs (8er Schritte) */
/* send a byte stream, converts \n to \r and evaluates TABs (8 columns) */
void l2_puts(u16 qsonum, const byte far *string);

/* 1 Byte senden (transparent) */
/* send one byte (transparently) */
void l2_send_char(u16 qsonum, byte character);

/* Packe angebrochene Frames und deallokiere freie Restbuffer;
   Nicht noetig nach send_frame() */
/* Packs and sends frames already begun and deallocates unused sender buffers
   Not necessary after send_frame() */
#define l2_pack(qsonum)   (l2_bef( 7, qsonum))

/* Empfangenen Frame abholen, 0 wenn nichts empfangen */
/* Get received frames, 0 if nothing in the receiver queue */
const INFO far *l2_get_i(u16 qsonum);

/* Bestaetige letzten empfangenen Frame (Frame danach nicht mehr lesbar!) */
/* acknowledge last received frame (the frame is not readable afterwards!) */
#define l2_i_ack(qsonum)         (l2_bef(15, qsonum))

/* Beende Verbindung, nachdem ausstehende Texte abgesendet */
/* terminate the connect (virtual circuits), after all buffered sender texts
   have been sent */
#define l2_stop_qso(qsonum)      (l2_bef( 4, qsonum))

/* Beende Verbindung sofort, loesche ausstehende Texte */
/* terminate the connect (virtual circuit) now, and clear all buffered sender
   texts */
#define l2_cancel_qso(qsonum)    (l2_bef( 5, qsonum))

/* Verbindung hart abbrechen */
/* hard disconnect */
#define l2_kill_qso(qsonum)      (l2_bef( 6, qsonum))

/* Loesche Texte in Empfangsbuffer */
/* clear the receiver buffer */
#define l2_clr_i(qsonum)         (l2_bef(10, qsonum))

/* RX-Infos zeilenweise abholen, d.h. in INFO stehen nicht mehr Frames,
   sondern Zeilen, durch CR getrennt. RX-Daten werden zeilenweise zusammen-
   gefasst, nicht frameweise. Umschalten in laufendem QSO kann zu Datenver-
   lust fuehren, also bei Bedarf einschalten sobald QSO-Nummer bekannt!
   Sinnvoll zu verwenden nur bei User-Dialogen...
   Zeilentrenner sind '\0' und '\r' */
/* Receive texts one line at a time, i.e. INFO does not contain frames, but
   merely lines (separated by CR). Changing the linemode during a QSO may
   lead to loss of data, so set the linemode as soon as the QSO number is
   known! Only makes sense with user dialogs...
   Line separators are '\0' and '\r' */
#define l2_set_linemode(qsonum)  (l2_bef(14, qsonum))

/* State der Verbindung: 0: Partner nicht (mehr) gehoert
                         1: Partner busy oder Verbindung wurde getrennt
                         2: Verbindungsaufbau laeuft
                         3: Protokollfehler aufgetreten, Resync laeuft,
                                ev. Datenverlust moeglich
                         4: Verbindungsabbau laeuft
                         5: Verbindung steht und ist idle
                         6: Verbindung hat REJ gesendet
                         7: Verbindung pollt

Bit 3 (0x08): Verbindung ist z.Zt. busy (zuviele RX-Frames erhalten)
But 4 (0x10): Gegenseite ist z.Zt. busy
Bit 6 (0x40): Texte in Empfangsbuffer

Die Anzeige in der Infobox-Userliste zeigt Bits 0-4 als numerischen Wert!

Dieser Call triggert gleichzeitig den Watchdog der Verbindung und muss daher
min. alle 3 Minuten aufgerufen werden */
/* state of a circuit: 0: peer not heard
                       1: peer busy or circuit was disconnected
                       2: connect in progress
                       3: protocol violation occured, resync in progress,
                               loss of data possible
                       4: disconnect in progress
                       5: circuit is connected and idle
                       6: REJ was sent
                       7: circuit is being polled

Bit 3 (0x08): Circuit is busy at this time (too many Rx frames received)
Bit 4 (0x10): Peer is busy at this time
Bit 6 (0x40): Rx buffer is not empty (i.e. contains text)

The infobox user list displays bits 0 to 4 as numerical value!

The call triggers also the watchdog of the circuit, so it must be called at
least every 3 minutes */
#define l2_state(qsonum)         ((byte)l2_stat(0, qsonum))

/* Anzahl der noch unbestaetigten Frames in Sendebuffer melden */
/* report the number of the unacknowledged frames in the transmitter buffer */
#define l2_unack(qsonum)         ((byte)l2_stat(1, qsonum))

/* Holen der QSO-beschreibenden Framestruct, falls es interessiert */
/* get the frame struct that describes the QSO */
const FRAME far *l2_get_f(u16 qsonum);

/* Timer-Tic liefern; inkrementiert alle 100ms und faltet */
/* return timer-tic; increments once per 100ms and wraps around */
u16 get_tic(void);

/* Applikations-Watchdog triggern; wird fuer jede Applikation separat
   verwaltet (maximal 8 Stueck). Muss zyklisch aufgerufen werden sofern
   gewuenscht. Abmelden beim terminieren nicht vergessen
       Parameter: Zeit in Minuten bis Reboot (minimal 5), 0 = Abmeldung */
/* Trigger the application watchdog; every application has its own watchdog
   (at most 8 watchdogs). Must be called periodically if desired. Do not
   forget to cancel the watchdog at application termination
      Parameter: time to reboot in minutes (minimum 5), 0 = cancel the
                 watchdog */
void appl_watchdog(u16 time);

/*------------------------------------------------------------------------*/
/* Monitorfunktionen - trace functions */

/* Monitor einschalten; tr: Ausgefuellte Filterstruktur
   Return: 1 wenn ok; wenn schon ein QSO-Trace laeuft oder kein RAM frei,
   Return = 0 */
/* switch on tracing; tr: initialized filter struct
   return value: 1 if ok; if already another QSO trace is running or if there
                 is no RAM left, return value = 0 */
byte l2_set_monitor(TRACE *tr);

/* Monitor abschalten; nur aufrufen wenn l2_set_monitor erfolgreich war! */
/* switch off tracing; should be called only if l2_set_monitor was successful! */
void l2_clr_monitor(void);

/* Test ob Monitor noch aktiv (bricht ab bei Ueberlauf)
   Return: 0 wenn inaktiv */
/* Tests if tracing is still active (is cancelled at overrun)
   Return: 0 if inactive */
 byte l2_chk_monitor(void);

/* Monitorframe abholen; Return: Framestruct oder 0 wenn nichts auf Lager */
/* Get a traced frame; return value: FRAME struct or 0 if nothing in the queue */
const FRAME far *l2_get_monitor(void);

/* Monitorframe bestaetigen */
/* acknowledge a traced frame */
void l2_ack_monitor(void);

/*------------------------------------------------------------------------*/
/* Hilfsfunktionen - utility functions */

/* Rufzeichen nach FlexNet-Konvention in ASCII-String wandeln;
   Achtung: Wegen Darstellung von Kontrollzeichen in fehlerhaften Rufzeichen
   kann der String max. 16 Bytes lang werden!
   Return: <call>
*/
/* convert a callsign from FlexNet's internal representation to an ASCII string;
   Warning: Because erroneous callsigns may contain control characters, the
   returned string may contain up to STRCALL (=16) bytes!
   Return: <call>
*/
byte *strcall(byte *call, const byte far *flexcall);

/* Digipeater-Pfad nach FlexNet-Konvention in ASCII-String wandeln;
   Achtung: Wegen Darstellung von Kontrollzeichen in fehlerhaften Rufzeichen
   kann der String max. 131 Bytes (STRPATH) lang werden!
   Return: <path>
*/
/* convert a digipeater path from FlexNet's internal representation to an ASCII string;
   Warning: Because erroneous callsigns may contain control characters, the
   returned string may contain up to STRPATH (=131) bytes!
   Return: <path>
*/
byte *strpath(byte *path, const FRAME far *f);
/*------------------------------------------------------------------------*/
/* Ueberfluessiges und Informatives sowie Spezielles fuer Spezialisten... */
/* superfluous, informative and special functions for specialists... */

/* Aus Pfad (siehe l2_connect(..)) einen FRAME bauen
   Keine Fehlermeldungen, irgendetwas wird immer generiert...
   Ausgefuellt wird: source, dest, digis, nextdigi, kanal und digi[] soweit
   definiert */
/* build a FRAME struct from a path (see l2_connect(...))
   No error messages, something will be filled in anyway...
   The following fields are filled in: source, dest, digis, nextdigi, kanal and
   digi[] as defined */
void build_frame(FRAME far *f, const byte far *source, const byte far *destination);

/* Frame senden, Framedata muss komplett ausgefuellt sein. Insbesondere
   muss, wenn textlen>0, text auf einen gueltigen Textbuffer zeigen!
   f.dama wird automatisch gesetzt entsprechend Kanalbetriebsart
   Return: 1 wenn ok oder wenn falsche Kanalnummer oder sonstiger Fehler (!)
           0 wenn derzeit kein Platz in TX-Puffer */
/* send a frame, FRAME must be initialized completely. Especially f.text must
   point to a valid text buffer if f.textlen>0!
   f.dama is set automatically according to the channel mode
   Return value: 1 if ok, or if the channel number is invalid or if any other
                   error (!)
                 0 if there is no place in the TX buffer at this time */
byte l2_send_f(FRAME far *f);

/* QSO starten mit Framestruct, siehe Kommentare zu l2_connect() und
   build_frame() */
/* start a QSO with the path specified in a FRAME struct, see comments about
   l2_connect() and build_frame() */
u16 l2_fconnect(FRAME far *f);

/* I-Frame transparent senden
    ACHTUNG: hierbei muss die l2_allokierte Laenge auf einer Framegrenze
    liegen! Also vor dem ersten Aufruf unbedingt Reste packen und dann
    immer genau die Laenge des Frames (netto) allokieren */
/* transparently send an I frame
    WARNING: the l2_ialloc'd length must lie on a frame boundary! Therefore,
    frames already begun must be packed before first calling this function, and
    always allocate the exact length of the frame (text length only) */
void l2_send_frame(u16 qsonum, const INFO far *info);

#define l2_maxframe(qsonum)      ((byte)l2_stat(2, qsonum))
#define l2_tries(qsonum)         ((byte)l2_stat(3, qsonum))
#define l2_frack(qsonum)         (l2_stat(4, qsonum))
#define l2_idletime(qsonum)      (l2_stat(5, qsonum))

/* Nummer eines QSO anhand Framedaten holen */
/* get the number of the QSO from the FRAME struct. Note: This function does
   NOT return f->qsonum, it tries to find the QSO from the source, destination
   and digipeater callsigns */
u16 l2_get_qnum(FRAME *f);

/* Anzahl noch abzuholende RX-Frames melden */
/* return the number of Rx frames in the queue */
#define l2_rx_cnt(qsonum)       (l2_stat(9, qsonum))

/* Laenge des naechsten bereitstehenden Empfangsframes melden */
/* return the length of the next received frame */
#define l2_i_len(qsonum)        (l2_stat(6, qsonum))

/* Veranlasse Pollen der Verbindung (Test ob Gegenstation noch da) */
/* initiate a poll of the circuit (tests if the peer is still there) */
#define l2_init_poll(qsonum)     (l2_bef(11, qsonum))

/* Setze Retry-Zaehler der Verbindung zurueck */
/* resets the retry counter of the circuit */
#define l2_reset_tries(qsonum)   (l2_bef(12, qsonum))

/* (Re)starte Verbindungsaufbau */
/* (re)start the connect setup */
#define l2_start_qso(qsonum)     (l2_bef( 1, qsonum))

void l2_bef(byte, u16); /* Nur intern! Internal only! */
u16 l2_stat(byte, u16); /* dito */

/* Protocol Identifier setzen, nur relevant fuer folgende puts, sollte
   normalerweise unveraendert bleiben (default nach QSO-Start ist $F0) */
/* set the protocol identifier, affects only subsequent l2_puts(), should
   therefore normally stay unchanged (default after QSO start is $F0) */
void l2_set_pid(u16 qsonum, byte pid);

/*------------------------------------------------------------------------*/
/* Low Level Calls in Kanaltreiber f. Diagnose usw. */
/* low level calls to channel drivers for diagnosis etc. */

/* Gibt verschiedene Kanalzustaende zurueck, Bits sind 1 wenn wahr:
         0x20   PTT Sender ist an
         0x10   DCD Empfaenger ist aktiv
         0x08   FDX Kanal ist Vollduplex, kann also immer empfangen
         0x04   TBY Sender ist nicht bereit, z.B. wg. Calibrate */
/* returns different channel states, bits are set if true:
         0x20   PTT transmitter is keyed on
         0x10   DCD receiver is active
         0x08   FDX channel is full duplex, i.e. is always able to receive
         0x04   TBY transmitter is busy, for example due to calibrate */
byte far ch_state(byte kanal);

/* Return TRUE wenn Kanal lebt */
/* returns true if the channel is alive */
byte ch_active(byte kanal);

/* Kanalbetriebsart und Baudrate setzen

    Return: 1 = OK; 0: Fehler

    Mode: Siehe Bitmaskendefinitionen in FLEXDRV.H
    Baud: Numerischer Wert / 100, also z.B. 96 f. 9600 Baud
*/
/* set the channel mode and baud rate

    Return: 1 = OK; 0 = error

    Mode: see definition of bit masks in FLEXDRV.H
    Baud: numerical value / 100, for example 96 for 9600 baud
*/
byte set_chmode(byte kanal, u16 mode, u16 baud);

/* TxDelay setzen; txd: Wert in 10ms-Schritten */
/* set the transmitter keyup time (TxDelay); txd: value in 10ms units */
void set_txdelay(byte kanal, byte txd);

/* Calibrate Mode schalten; Ergebnisse sind treiberspezifisch */
/* switch on calibrate mode; result is driver specific */
void calibrate(byte kanal, byte minutes);

/* Kanalparameter lesen */
/* retrieve channel parameters */
u16 get_chbaud(byte kanal);     /* Liest Baudrate - retrieve the baud rate */
u16 get_chmode(byte kanal);     /* Liest Mode - retrieve the mode */
u16 get_txdelay(byte kanal);    /* Liest Txdelay - retrieve the TxDelay */
/* Name des Kanals lesen, Nullpointer wenn Kanal nicht aktiv */
/* retrieve the name of the channel, null pointer if the channel is not active */
const byte far *get_ident(byte kanal);

/* Kanalstatistik lesen, bei User-Version koennen mit del!=0 die Werte
   geloescht werden */
/* get the channel statistics; if del!=0 and the mini kernel active, the
   statistics may be cleared */
const L1_STATISTICS far *get_stat(byte kanal, byte del);
/*------------------------------------------------------------------------*/
#ifdef _WIN32
void set_debug_mode(int mode);
void flxdbg(int level,char *fmt, ...);
#endif
/*------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif

#ifdef _WIN32
#pragma pack(pop, flexpack)
#endif



