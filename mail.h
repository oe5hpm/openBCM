/***************************************************************

  BayCom(R)   Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------------------
  Definitionen und Deklarationen fuer Mailbox
  -------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/


//19980115 OE3DZW removed obsolete defines, changed fbb-fwd-path
//                added ad_timezone()
//19980124 OE3DZW added define for REJECTLOGNAME
//19980202 OE3DZW added option for new callcheck (from hrx), id for cb-mode
//19980208 OE3DZW defined BETREFFLEN
//19980211 OE3DZW added bytes/connects to fwdpara_t
//19980215 OE3DZW added define for syslog_r.bcm
//19980218 OE3DZW added msgty(pe) to mailflags
//19980301 OE3DZW added ax25_set_pid (mbflex.cpp)
//19980226 hrx    added taskwin to functions (interfac.cpp)
//19980301 OE3DZW added killforwarded, add_hold
//19980311 DG9MHZ changes for Linux-Kernel AX25
//19980324 DH3MB  support for md2/5
//19980408 hrx    cbmodeid changed (from /cb to cb)
//                autotrcwin added to mailpara_t
//                mbcheckcount added to functions, section check.cpp
//                trcwin added to functions, section interfac.cpp
//19980411 hrx    changed type of mbcheckcount() to long
//                added autofwd as part of hrx_opt
//                added fwd_afwd.cpp to function section, currently
//                autofwd() and fwdlist() are used
//19980613 hrx    added _paclen conditional to hrx_opt
//                added _asklogin conditional tp hrx_opt
//                added _cfwd conditional tp hrx_opt
//                added CHECKNUMNAME  "checknum.bcm" and
//                CHECKNUMTMP "checknum.$$$". used for autofwd.
//                added paclen int to user_t, mailpara_t. added
//                structures hadr_t, hadr_old_t, used for autofwd.
//                added uclose(), hadr_tryopen(), loadhadr() to functions,
//                section mbhadr.cpp. added createboardfwd to mailpara_t
//                added autofwdtime to mailpara_t
//19980619 OE3DZW moved defines for options to baycom.h
//19980913 OE3DZW added savebroken
//19981015 OE3DZW added parameters for wxstation
//19990111 OE3DZW added dirformat to user-database
//19990714 DH3MB  Changed declarations for fwd.cpp and fwd_rli.cpp
//19991028 Jan    added defines for AX25K_ONLY
//20000624 DK2UI  changed size of struct reject_t for regular expressions
//20020724 DH8YMB name changed from "BCM" to "OpenBCM"
//20021130 DH8YMB HRX_OPT Optionen fest eingebaut
//20030102 hpk    added minpaclen, maxpaclen, defrea to mailpara_t
//20030102 hpk    added pwonly for the CB-BCMNET login-concept to mailpara_t
//20030102 hpk    added logintype for the CB-BCMNET login-concept to t_var_t
//20030109 DH8YMB defpaclen hinzu
//20030706 DB1RAS added CB-BCMNET modeid and support message
//20030706 DB1RAS added dospreexec and dospostexec in mailpara_t
//20030809 DB1RAS added CB-BCMNET gateway functions
//20030821 DH8YMB added fwd_file.cpp
//20031010 DH8YMB Userdatenbank neues Format in users4.bcm
//20031226 DH6BB  FTP-Server fuer den Filesurf
//20121226 DG4MFN added comment to AUTHOR_ID about modification for Linux GCC 4.x.x

/*---------------------------------------------------------------------------*/

#ifndef GITVERSION
#if defined(_WIN32) || defined(__MSDOS__)
 #include "gitver.h"
#else
 #warning "no GIT version available, using a dummy g????"
 #define GITVERSION	"?????"
#endif
#endif

#undef  VNUMMER
#define VNUMMER         GITVERSION
#define STD_BOXHEADER   "OpenBCM"
#define LOGINSIGN       STD_BOXHEADER" V"VNUMMER" ("OPSYSTEM")"
#define AUTHOR_ID	"(c) GNU GPL 1992-"YEAR" F. Radlherr, DL8MBT et al.\n" \
                        "OpenBCM is maintained by Markus Baumann, DH8YMB\n"

#if ! defined _BCMNET || ! defined _BCMNET_FWD || ! defined _BCMNET_LOGIN
  #define CBMODEID      "(CB)"
  #define CBSUPPORT     "CB-Version, Support by DJJ812/DO1MJJ\n"
  #define WEBSITE_CB    "http://www.juerging.net/projekte/dbo812"
#else
  #define CBMODEID      "*CB-BCMNET*"
  #define CBSUPPORT     "CB-BCMNET Version, Support by AS1GBF/AT5HPK/HF1BKM\n"
  #define WEBSITE_CB    "http://www.afulinux.de/afulinux/cb-bcmnet"
#endif
#define WEBSITE         "https://github.com/oe5hpm/openbcm"

#ifdef __MSDOS__
  #ifdef __FLAT__
    #ifdef _WIN32
      #define OPSYSTEM  "WIN32"
      #define BCMLOGO   "OBcm"
    #else
      #define OPSYSTEM  "DOS32"
      #define BCMLOGO   "OBCm"
    #endif
  #else
    #define OPSYSTEM    "DOS"
    #define BCMLOGO     "OBCM"
  #endif
#else
  #ifdef __LINUX__
    #ifndef __sparc__
      #define OPSYSTEM  "Linux"
    #else
      #define OPSYSTEM  "Slink" //Sparc-Linux is 'Slink' - F1ULQ
    #endif
    #ifndef __sparc__
      #define BCMLOGO   "obcm"
    #else
      #define BCMLOGO   "Sobcm"
    #endif
  #endif
#endif

//*************************************************************************
//  path and file names
//*************************************************************************
#define MSGPATH         "msg"
#define SYSTEMPATH      "system"
#define LOGPATH         "log"
#define TRACEPATH       "trace"
#define TEMPPATH        "temp"
#define FWDPATH         "fwd"
#define FBBPATH         "fwd/fbb"
#define FWDEXPATH       "fwd/export"
#define FWDIMPATH       "fwd/import"
#define WLOGPATH        "fwd/wlog"
#ifdef __FLAT__
#define HTTPPATH        "http"
#endif
#define CRONTABNAME     "crontab.bcm"
#define VIDNAME         "dosvid.bcm"
#define FWDLISTNAME     "fwd.bcm"
#define FWDLISTNAMEBAK  "fwd.bak"
#ifdef _AUTOFWD
#define AFWDLISTNAME    "afwd.bcm"
#define AFWDTMP         "temp/afwd1.tmp"
#endif
#define BEACONNAME      "beacon.bcm"
#define CONVFILENAME    "convert.bcm"
#define CONVTMPNAME     "convname.$$$"
#ifdef __MSDOS__
  #define USERNAMEOLD   "users.bcm"
#else
  #define USERNAMEOLD   "users3.bcm"
#endif
#define USERNAME        "users4.bcm"
#define USERNAMEBAK     "users4.bak"
#define USERNAMETMP     "users4.$$$"
#define UTMP            "users.$$$"
#ifdef __MSDOS__
#define HADROLDNAME     "hadr2.bcm"
#define BIDNAME         "bids2.bcm"
#define BIDNAME_L       "bids3.bcm"
#else
#define HADROLDNAME     "hadr3.bcm"
#define BIDNAME         "bids3.bcm"
#endif
#define HADRNAME         "hadr4.bcm"
#define HADRHASHNAME     "hadrhash.bcm"
#define USERHASHNAME_OLD "userhash.bcm"
#define UHTMP            "userhash.$$$"
#define USERHASHNAME     "userh2.bcm"
#define USERHASHNAMEBAK  "userh2.bak"
#define BIDHASHNAME      "bidh2.bcm"
#define BTMP            "bids2.$$$"
#define BHTMP           "bidh2.$$$"
#define BBAK            "bids2.bak"
#define BHBAK           "bidh2.bak"
#define DIRLISTNAME     "list.bcm"
#define DIRTMPNAME      "list.$$$"
#define MBINITNAME      "init.bcm"
#define MBINITOLDNAME   "bcm.ini"
#define INITTMPNAME     "init.$$$"
#define INITBAKNAME     "init.bak"
//#define LOGNAME         "log.bcm"
#define CHECKNAME       "check.bcm"
#define CHECKTMPNAME    "check.$$$"
#define CHECKNUMNAME    "checknum.bcm"
#define CHECKNUMTMP     "checknum.$$$"
#define PASSWDNAME      "passwd.bcm"
#define PRIVNAME        "pwlist.bcm"
#define TREENAME        "bulletin.bcm"
#define TREETMPNAME     "bulletin.$$$"
#define TREEBAKNAME     "bulletin.bak"
#define SYSLOGNAME      "syslog.bcm"
#define SYSLOGRNAME     "syslog_r.bcm"
#define CMDLOGNAME      "cmdlog.bcm"
#define PWLOGNAME       "pwlog.bcm"
#ifdef __FLAT__
#define NNTPLOGNAME     "nntplog.bcm"
#define HTTPLOGNAME     "httplog.bcm"
#define POP3LOGNAME     "pop3log.bcm"
#define SMTPLOGNAME     "smtplog.bcm"
#define FTPLOGNAME      "ftplog.bcm"
#endif
#define ERASELOGNAME    "eraselog.bcm"
#define UNKNOWNNAME     "unknown.bcm"
#define SFHOLDNAME      "sfhold.bcm"
#define SWAPLOGNAME     "swap.bcm"
#define MFILTERLOGNAME  "m_filter.bcm"
#define RUNDATNAME      "rundat.bcm"
#define BIDNAME_OLD     "bids.bcm"
#define BIDHASHNAME_OLD "bidhash.bcm"
#define RUNNAME         "runutil.bcm"
#define MAILLISTNAME    "mailserv.bcm"
#define MAILLISTTMPNAME "mailserv.$$$"
#ifdef DF3VI_POCSAG
  #define POCSAGLISTNAME "pocsag.bcm"
  #define POCSAGLISTTMP  "pocsag.$$$"
#endif
#define OLDUMAILLOGNAME "oldumail.bcm"
#define REJECTLOGNAME   "rejlog.bcm"
#define REJECTNAME      "reject.bcm"
#ifdef DF3VI_REJ_EDIT
  #define REJECTTMPNAME "reject.bak"
#endif
#define POPDBNAME       "popstate.bcm"
#define POPDBTMPNAME    "popstate.$$$"
#define BEACONHEADNAME  "beachead.bcm" //header for beacon
#define ISSUENAME       "issue.bcm" //telnet pre-login infotext
#define BOARDINFO       "boardinf.bcm"
#define CONVATNAME      "convat.bcm"
#define RHOSTSNAME      "rhosts.bcm"
#define L2INITNAME      "init.l2"
#define L2PATHNAME      "netpath.bcm"
#define HTMP            "hadrs.$$$"
#define HHTMP           "hadrhash.$$$"
#ifdef _TELEPHONE // JJ
 #define INITTTYNAME    "inittty.bcm"
 #define INITTTYTMPNAME "inittty.$$$"
 #define INITTTYBAKNAME "inittty.bak"
#endif
#ifdef MACRO
 #define EVENTNAME      "macro.bcm"
#endif
#ifdef _TELEPHONE
 #define conlogintext   "ctextcon"
 #define ttylogintext   "ctexttty"
#endif

/*---------------------------------------------------------------------------*/

// HTML-defines and tcpip ports
#define HTMLSTARTTAG    "<HTML>"
#ifdef __FLAT__
  #define MIME_JPG        "image/jpeg"
  #define MIME_GIF        "image/gif"
  #define MIME_TXT        "text/html"
  #define MIME_BIN        "application/x-binary"
  #define MIME_WAV        "audio/wav"
  #define DEFFTPPORT      8021
  #define DEFSMTPPORT     8025
  #define DEFPOP3PORT     8110
  #define DEFNNTPPORT     8119
  #define DEFHTTPPORT     8080
  #define DEFTELNPORT     4719
  #define DEFSERVPORT     8123
  #define DEFRADIOPORT    8124
#endif

/*---------------------------------------------------------------------------*/

// flags of "7plus" by dg1bbq
#define START7PFLAG     " go_7+. "
#define STOP7PFLAG      " stop_7+. "
#define STARTINFOFLAG   " go_info. "
#define STOPINFOFLAG    " stop_info."
#define STARTTEXTFLAG   " go_text. "
#define STOPTEXTFLAG    " stop_text."

/*---------------------------------------------------------------------------*/

#define FWDOK  0
#define FWDNO  1
#define FWDREJ 2
#define FWDERR 3

#define BINMODE_AUTOBIN  0
#define BINMODE_YAPP     1
#define BINMODE_DIDADIT  2
#define BINMODE_BINSPLIT 3

#define SEPLINE "-------------------------------------------------------------------------------"

/*---------------------------------------------------------------------------*/

// Betriebssystemabhaengige Definitionen
#ifdef __FLAT__
  #define US_CACHESZ     128 // user database cache
  #define biosminuten    ad_minutes()
  #define TASKS          250
  #define MAXTNCPORTS    220
  #define MAXBLKS        400
  #define MAXHELP        60
  #define MAXUSERMAILS   60000U
  #define MAXDIRENTRIES  300000U
  #define MAXTREEENTRIES 10000     // Anzahl der Boards
  #define MAXTREELENGTH  18        // Laenge eines vollen Boardnamen
  #define MAXCONVNAME    5000
  #define MAXFWD         50 // Anzahl der Boxen, zu denen geforwardet wird, nicht aendern!
  #define BUFLEN         500
  #define MAXDEST        1000      // Maximum number of destinations
  #define MAXDESTLEN     MAXDEST * HADRESSLEN // Groesse des Arrays fuer die Zieladressen
  #define FNAMELEN       50        //not more than 50 for BIN header, 120 needed for filesurf
  #define TASKSTACK      32000
  #define LOCK_MAX       200
  #define SEMA_MAX       150
#else
  #define US_CACHESZ     1     // for DOS reduced because of DGROUP > 64k
  #define biosminuten ((unsigned) (*((long far *) MK_FP(0x40, 0x6c)) / 1092))
  #define coldstart() ((void far (*)(void)) MK_FP(0xffff, 0x0000))()
  #define TASKS          80
  #define MAXTNCPORTS    73
  #define MAXBLKS        150
  #define MAXHELP        60
  #define MAXUSERMAILS   2000U
  #define MAXDIRENTRIES  4000U
  #define MAXTREEENTRIES 800   // Anzahl der Boards
  #define MAXTREELENGTH  18    // Laenge eines vollen Boardnamen
  #define MAXCONVNAME    1000
  #define MAXFWD         24    // Anzahl der Forwardpartner, nicht aendern!
  #define BUFLEN         300
  #define MAXDEST        250
  #define MAXDESTLEN     3000  // Groesse des Arrays fuer die Zieladressen
  #define FNAMELEN       50
  #ifdef MACRO
      #define TASKSTACK  5000
  #else
    #ifdef _FILEFWD // Fileforward need more stack (db1ras)
      #define TASKSTACK  3250
    #else
      #define TASKSTACK  3000
    #endif
  #endif
  #define LOCK_MAX       60
  #define SEMA_MAX       40
#endif

/*---------------------------------------------------------------------------*/

// Bits fuer die Optionen
#define o_a  (0x00000001L)
#define o_b  (0x00000002L)
#define o_c  (0x00000004L)
#define o_d  (0x00000008L)
#define o_e  (0x00000010L)
#define o_f  (0x00000020L)
#define o_g  (0x00000040L)
#define o_h  (0x00000080L)
#define o_i  (0x00000100L)
#define o_j  (0x00000200L)
#define o_k  (0x00000400L)
#define o_l  (0x00000800L)
#define o_m  (0x00001000L)
#define o_n  (0x00002000L)
#define o_o  (0x00004000L)
#define o_p  (0x00008000L)
#define o_q  (0x00010000L)
#define o_r  (0x00020000L)
#define o_s  (0x00040000L)
#define o_t  (0x00080000L)
#define o_u  (0x00100000L)
#define o_v  (0x00200000L)
#define o_w  (0x00400000L)
#define o_x  (0x00800000L)
#define o_y  (0x01000000L)
#define o_z  (0x02000000L)
//oe3dzw Neue Flags
#define o_0  (0x04000000L)
#define o_b1 (0x04000000L)
#define o_1  (0x08000000L)
#define o_d1 (0x08000000L)
#define o_2  (0x20000000L)
#define o_3  (0x40000000L)
#define o_4  (0x80000000L)

/*---------------------------------------------------------------------------*/

//*************************************************************************
//  Defines of special strings
//*************************************************************************
#define CALLEN          6
#define DIRLEN          8      // boardname
#define CALLSSID        9      // call with SSID
#define BIDLEN          12
#define NAMELEN         14
#define MYBBSLEN        39     // length of complete MyBBS incl. H-Route
#define OLDMYBBSLEN     24     // for compatibility with the old mybbs field
#define PROMPTLEN       20
#define FIRSTCMDLEN     20
#define QTHLEN          30
#define ZIPLEN          8
#define HADRESSLEN      39     // complete H-Route
#define MAXPARSE        40
#define BINCMDLEN       80     // fix size of BIN-header within mail files
#define BETREFFLEN      80     // max. length of title
#define LINELEN         80     // standard line length
#define FSPATHLEN       100
#define BLEN            128    // line len of most binary files
#define LBLEN           128L
#define CMDBUFLEN       130
#define DESTBOXLEN      (MAXFWD*7)
#define CBUFLEN         8192   // copy buffer size (1L << 13)
#define ADJNUM          20
#define HEADERLEN       60
#define BSTATUSLEN      20
#define BHWADDRESSLEN   49
#define BPROTOLEN       10
#define BVERSION        10     // Version 1.0
#define BSTATUS         "0"    // Allow DFWD, 0=inactive, 1=active
#define WPSUBJECT       "WP Update"
#define WPROTSUBJECT    "WPROT Update"
#define BPROTMAX        10     // max. WPROT protocol version accepted
#define WPBUFSIZE       255
#define FBBMSGNUMOFFSET 100000L // Unterscheidung von FBB-Check-Nr.+DIR/LIST-Nr.
#define LOGLEN          20
#define MAXLOGSUM       1000
#define USERDBREST      88

#ifdef RUNUTILS
#define RUN_CMDLEN      11
#define RUN_EXELEN      19
#define RUN_COMLEN      65
#define RUN_PARAMLEN    130    // max. length of parameters for run-util
#endif

/*---------------------------------------------------------------------------*/

#define BOARDLEER       (-1)
#define MAXBEREICH      (ULONG_MAX)
#define HASH16          (65536L)    // (1L << 16)
#define HASH32          (65536L*2)  // (1L << 17)
#define MAXCOMPFILESIZE 80000000L   // 80 MB max. FBB-compressed maillength
#define HOUR            3600L       // 60 * 60
#define DAY             86400L      // 60 * 60 * 24
#define MAXAGE          2529000L    // 60 * 60 * 24 * 30 max. age of incomimg mybbs
#ifdef _BCMNET
  #define USERPROTECT   183L * DAY  // Zeitspanne fuer den Userschutz
  #define BCMNETID      ".BCMNET"
#endif

/*---------------------------------------------------------------------------*/

// Min. available diskspace in kbytes
#define USERDISKFULL    500L
#define INFODISKFULL    1000L
#define HOMEDISKFULL    1000L

/*---------------------------------------------------------------------------*/

//*************************************************************************
//  Characters used in YAPP-file-transfer and FBB-forwarding (DH3MB)
//*************************************************************************
#define NUL   0x00 // End of string / Separator
#define SOH   0x01 // Start of header
#define STX   0x02 // Start of transmission
#define ETX   0x03 // End of transmission
#define EOT   0x04 // End of Transfer
#define ENQ   0x05 //
#define ACK   0x06 // Acknowledgement
#define DLE   0x10 //
#define NAK   0x15 // Negative Acknowledgenment
#define CTRLX 0x18 // Cancel
#define CTRLZ 0x1a
#define ESC   0x1b

/*---------------------------------------------------------------------------*/

//*************************************************************************
// Use THIS!
//*************************************************************************
#define safe_strncpy(a, b, n)  { strncpy(a, b, n-1); a[n-1] = 0; }
#define safe_strcpy(a, b)      safe_strncpy(a, b, sizeof(a))

/*---------------------------------------------------------------------------*/

#define ltell(fh) lseek(fh, 0L, SEEK_CUR)
#define leof(fh)  (lseek(fh, 0L, SEEK_CUR) == filelength(fh))

/*---------------------------------------------------------------------------*/

typedef unsigned long bitfeld;

#pragma pack(1)  // Alignment auf 1 Byte setzen
typedef struct user_t
//*************************************************************************
//
//  Struktur fuer die users4.bcm Userdatenbank
//
//*************************************************************************
{ char     call[CALLEN+1];          //  7 Rufzeichen des Users
  char     name[NAMELEN+1];         // 15 A NAME
  char     qth[QTHLEN+1];           // 31 QTH (DH8YMB)
  char     qthok;                   //  1 QTH selbst eingegeben
  char     zip[ZIPLEN+1];           //  9 ZIP-Code (Postleitzahl) (DH8YMB)
  char     zipok;                   //  1 Zip-Code selbst eingegeben
  char     mybbs[MYBBSLEN+1];       // 40 A FORWARD
  char     mybbsok;                 //  1 MYBBS selbst eingegeben
  time_t   mybbstime;               //  4 Datum des letzten MYBBS (UNIX)
  char     uplink[CALLSSID+1];      // 10 uplink-Digi
  char     newcall[CALLEN+1];       //  7 New callsign (license update)
  char     prompt[PROMPTLEN+1];     // 21 A PROMPT
  char     sprache[4];              //  4 A SPEECH
  char     password[40];            // 40 A PW (pw off = 0x01 0x00 ...)
  char     notvisible[LINELEN];     // 80 A REJECT
  char     firstcmd[FIRSTCMDLEN+1]; // 21 A COMMAND
  short unsigned helplevel;         //  2 A HELPLEVEL
  long unsigned logins;             //  4 Anzahl an Logins
  short unsigned lf;                //  2 A LF
  short unsigned zeilen;            //  2 A LINES
  char     nopurge;                 //  1 Board wird nicht geputzt
  char     nameok;                  //  1 Name selbst eingegeben
  time_t   lastboxlogin;            //  4 letzter Login (UNIX)
  time_t   lastdirnews;             //  4 letztes DIR NEWS / Check (UNIX)
  time_t   lastquit;                //  4 letztes QUIT
  time_t   lastload;                //  4 interner Merker fuer Verwaltung
  long     mailsent;                //  4 gesendete Nachrichten
  long     mailgot;                 //  4 erhaltene Nachrichten
  long     mailread;                //  4 gelesene Nachrichten
  long unsigned  daybytes;          //  4 gelesene Bytes heute
  bitfeld  opt[8];                  // 32 eingestellte Optionen
  char     ufwd[HADRESSLEN+1];      // 40 Connect-Path fuer User-forwarding
  char     loginpwtype;             //  1 Password-type for ax-login (DH3MB)
                                    //    0..BayCom-Password
                                    //    1..MD2
                                    //    2..MD5
                                    //    100..disabled
  char     sfpwtype;                //  1 Password-type for S&F (DH3MB)
                                    //    0..BayCom-Password
                                    //    1..MD2
                                    //    2..MD5
                                    //    100..disabled
  char     ttypw[9];                //  9 A TTYPW (TTY-Password)
  short unsigned fdelay;            //  2 Verzoegerung fuer eigene Nachrichten
  short unsigned fhold;             //  2 Verzoegerung fuer Nachrichten
  short unsigned status;            //  2 A STATUS (Bit-field)
                                    //    0..default
                                    //    1..no quota-limits
                                    //    2..access denied
                                    //    4..access denied, sysop informed
                                    //    8..my not use transfer command
  char     readlock;                //  1 Privatmails nicht lesbar
  char     rlimit;                  //  1 fremde Privatmails nicht lesbar
  char     pwline;                  //  1 Leerzeile beim Login
  char     echo;                    //  1 Eigene Eingaben zurueckschicken
  short unsigned paclen;            //  2 Packetlength
  char     dirformat;               //  1 Dir/Check/List in new format
  char     binmode;                 //  1 AutoBIN/YAPP/DIDADIT bei BIN-Mails
  char     unsecure_smtp;           //  1  0=>needs to pop before smtp
                                    //     1=>smtp without pop allowed
  char     away;                    //  1 AWAY-Funktion aktiv/deaktiv (DH8YMB)
  time_t   awayendtime;             //  4 AWAY Endtime (UNIX)
  char     awaytext[LINELEN];       // 80 AWAY Text
  char     charset;                 //  1 A CHARSET (nicht implementiert)
  char     ttycharset;              //  1 TTY-Charset (nicht implementiert)
  char     fbbcheckmode;            //  1 FBB-Check/Read-Mode
  char     notification[CALLEN+1];  //  7 Mailbenachrichtigung
  char     linuxpw[9];              //  9 A LINUXPW (Linux-System-Passwort)
  char     comp;                    //  1 Huffman-COMP aktiv/deaktiv
  char     httpsurface;             //  1 Aussehen der HTTP-Oberflaeche
                                    //     0=>Sysopvoreinstellung (defhttpsurface wird genutzt)
                                    //     1=>alte Oberflaeche mit Frames und ohne CSS-Support
                                    //     2=>neue Oberflaeche ohne Frames und mit CSS-Support
                                    //     3=>alte Oberflaeche ohne Frames und ohne CSS-Support
  char     restplatz[USERDBREST];   // 88 Restplatz -> USERDBREST definieren!!

  short unsigned filepos_old;       //  2 Position im Userfile (altes Format)
  short unsigned nextsamehash_old;  //  2 Link Hashliste (altes Format)
  long unsigned filepos;            //  4 Position im Userfile
  long unsigned nextsamehash;       //  4 Link Hashliste
                                    //  = 640 Bytes
} user_t ;
#pragma pack() // Alignment zurueck auf Default

/*---------------------------------------------------------------------------*/

#ifdef _USERS4CONVERT
#ifdef __MSDOS__
typedef struct userold_t
//*************************************************************************
//
//  Struktur fuer die alten Userdaten users.bcm (DOS)
//
//*************************************************************************
{ char     call[CALLEN+1];          //  7 Rufzeichen des Users
  char     name[NAMELEN+1];         // 15 A NAME
  char     oldmybbs[OLDMYBBSLEN+1]; // 25 for compatibility with old mybbs field
  char     prompt[PROMPTLEN+1];     // 21 A PROMPT
  char     sprache[4];              //  4 A SPEECH
  char     password[40];            // 40 A PW ; pw off = 0x01 0x00 ..
  char     notvisible[LINELEN];     // 80 A REJECT
  char     firstcmd[FIRSTCMDLEN+1]; // 21 A COMMAND
  short unsigned helplevel;         //  2 A HELPLEVEL
  short unsigned zeilen;            //  2 A LINES
  char     nopurge;                 //  1 Board wird nicht geputzt
  char     nameok;                  //  1 Name selbst eingegeben
  time_t   lastboxlogin;            //  4 letzter Login (UNIX)
  time_t   lastdirnews;             //  4 letztes DIR NEWS / Check (UNIX)
  time_t   lastquit;                //  4 letztes QUIT
  long     mailsent;                //  4 gesendete Nachrichten
  long     mailgot;                 //  4 erhaltene Nachrichten
  long     mailread;                //  4 gelesene Nachrichten
  long unsigned logins;             //  4 logins
  char     mybbsok;                 //  1 MYBBS selbst eingegeben (1)
  bitfeld  opt[8];                  // 32 eingestellte Optionen
  short unsigned lf;                //  2 A LF
  char     charset;                 //  1 A CHARSET (nicht impl)
  time_t   mybbstime;               //  4 Datum des letzten MYBBS (UNIX)
  char     ttypw[9];                //  9 A TTYPW (TTY-Password)
  time_t   lastload;                //  4 interner Merker fuer Verwaltung
  long unsigned  daybytes;          //  4 gelesene Bytes heute
  short unsigned status;            //  2 A STATUS (Bit-field)
                                    //    0..default
                                    //    1..no quota-limits
                                    //    2..access denied
                                    //    4..access denied, sysop informed
                                    //    8..my not use transfer command
  char     uplink[CALLSSID+1];      // 10 uplink-Digi
  char     readlock;                //  1 Privatmails nicht lesbar (")
  char     pwline;                  //  1 Leerzeile beim Login
  char     echo;                    //  1 Eigene Eingaben zurueckschicken
  short unsigned fdelay;            //  2 Verzoegerung fuer eigene Nachrichten
  short unsigned fhold;             //  2 Verzoegerung fuer Nachrichten
  char     rlimit;                  //  1 fremde Privatmails nicht lesbar
  char     ufwd[40];                // 40 Connect-Path fuer User-forwarding
  char     loginpwtype;             //  1 Password-type for ax-login (DH3MB)
                                    //    0..BayCom-Password
                                    //    1..MD2
                                    //    2..MD5
                                    //    100..disabled
  char     sfpwtype;                //  1 Password-type for S&F (DH3MB)
                                    //    0..BayCom-Password
                                    //    1..MD2
                                    //    2..MD5
                                    //    100..disabled
  char newcall[CALLEN+1];           //  7 New callsign (license update)
  short unsigned paclen;            //  2 Packetlength
  char dirformat;                   //  1 Dir/Check/List in new format
  char binmode;                     //  1 AutoBIN/YAPP/DIDADIT bei BIN-Mails
  char unsecure_smtp;               //  1  0=>needs to pop before smtp
                                    //     1=>smtp without pop allowed
  char mybbs[MYBBSLEN+1];           // 40 A FORWARD
  char away;                        //  1 AWAY-Funktion aktiv (DH8YMB)
  char restplatz[81];               // 81 ist noch frei (definiert 0)
  long unsigned filepos;            //  4 Position im Userfile
  long unsigned nextsamehash;       //  4 hash list link
  short unsigned filepos_old;       //  2 (obsolete) Position im Userfile
  short unsigned nextsamehash_old;  //  2 (obsolete) hash list link
                                    //  = 512 Bytes
} userold_t;
#else
typedef struct userold_t
//*************************************************************************
//
//  Struktur fuer die alten Userdaten users3.bcm der WinNT/Linux-Version,
//  beruecksichtigt Alignment vom GCC (mind. 4 Byte)
//
//*************************************************************************
{ time_t   lastboxlogin;                //  4 letzter Login (UNIX)
  time_t   lastdirnews;                 //  4 letztes DIR NEWS / Check (UNIX)
  time_t   lastquit;                    //  4 letztes QUIT
  time_t   mybbstime;                   //  4 Datum des letzten MYBBS (UNIX)
  time_t   lastload;                    //  4 interner Merker fuer Verwaltung
  long unsigned daybytes;               //  4 gelesene Bytes heute
  long     mailsent;                    //  4 gesendete Nachrichten
  long     mailgot;                     //  4 erhaltene Nachrichten
  long     mailread;                    //  4 gelesene Nachrichten
  long unsigned logins;                 //  4 logins
  bitfeld  opt[8];                      // 32 eingestellte Optionen
  long     spare_long[10];              //
  short unsigned helplevel;             //  2 A HELPLEVEL
  short unsigned zeilen;                //  2 A LINES
  short unsigned lf;                    //  2 A LF
  short unsigned status;                //  2 A STATUS (Bit-Field)
  short unsigned filepos_old;           //  2 (obsolete)Position im Userfile
  short unsigned nextsamehash_old;      //  2 (obsolete)hash list link
  short unsigned fdelay;                //  2 Verzoegerung fuer eigene Nachrichten
  short unsigned fhold;                 //  2 Verzoegerung fuer Nachrichten
  short unsigned paclen;                //  2 packet length
  short unsigned spare_short[19];       //
  char     call[CALLEN+1];              //  7 Rufzeichen des Users
  char     name[NAMELEN+1];             // 15 A NAME
  char     oldmybbs[OLDMYBBSLEN+1];     // 25 for compatibility with old mybbs field
  char     prompt[PROMPTLEN+1];         // 21 A PROMPT
  char     sprache[4];                  //  4 A SPEECH
  char     password[40];                // 40 A PW
  char     notvisible[LINELEN];         // 80 A REJECT
  char     firstcmd[FIRSTCMDLEN+1];     // 21 A COMMAND
  char     nopurge;                     //  1 Board wird nicht geputzt
  char     nameok;                      //  1 Name selbst eingegeben
  char     mybbsok;                     //  1 MYBBS selbst eingegeben (1)
  char     charset;                     //  1 A CHARSET (nicht impl)
  char     ttypw[9];                    //  9 A TTYPW (TTY-Password)
  char     uplink[CALLSSID+1];          // 10 uplink-Digi
  char     readlock;                    //  1 Privatmails "nicht lesbar"
  char     pwline;                      //  1 Leerzeile beim Login
  char     echo;                        //  1 Eigene Eingaben zurueckschicken
  char     rlimit;                      //  1 fremde Privatmails nicht lesbar
  char     ufwd[40];                    // 40 Connect-Path fuer User-forwarding
  char     loginpwtype;                 //  1 Password-type for ax-login (DH3MB)
  char     sfpwtype;                    //  1 Password-type for S&F (DH3MB)
  char     newcall[CALLEN+1];           //  7 New callsign (lizence update)
  char     dirformat;                   //  1 Dir/Check/List in new format
  char     unsecure_smtp;               //  1  0=>needs to pop before smtp
                                        //     1=>smtp without pop allowed
  char     binmode;                     //  1 AutoBIN/YAPP/DIDADIT bei BIN-Mails
  char     ttycharset;                  //  1 A TTYCHARSET (nicht impl)
  char     mybbs[MYBBSLEN+1];           // 40 A FORWARD
  char     away;                        //  1 AWAY-Funktion aktiv (DH8YMB)
  char     restplatz[2];                //  2 ist noch frei (definiert 0)
  unsigned long filepos;                //  4 position within file
  unsigned long nextsamehash;           //  4 hash to next record
                                        // = 512 Bytes (473)
} userold_t;
#endif
#endif


/*---------------------------------------------------------------------------*/

typedef enum
{ j_idle,
  j_fwdlink,
  j_fwdsend,
  j_fwdsendf,
  j_fwdrecv,
  j_fwdrecvf,
  j_fbb_send_propose,
  j_fbb_send_mail,
  j_fbb_send_delay,
  j_fbb_recv_propose,
  j_fbb_recv_mail,
  j_fbb_end,
  j_fbb_error,
  j_send,
  j_read,
  j_cmd,
  j_search,
  j_purge,
  j_reorg,
  j_tell,
  j_node,
  j_ftrans,
  j_router,
  j_convers
} job_typ;

typedef int handle;


#define NOTASK (-1)
// Flags fuer laufenden Prozess (wird in t->processkind hinterlegt)
// Die Flags koennen beliebig kombiniert werden
//
#define P_MAIL 1      // hat einen Mailbox-Kontrollblock
#define P_WIND 2      // hat einen Editor-Kontrollblock
#define P_KEYB 4      // benutzt Tastatureingaben
#define P_BACK 8      // laeuft im Hintergrund (keinerlei Ein- und Ausgabe)
#define P_CLON 16     // DH3MB: Mailbox-Kontrollblock wird in die neue
                      // Task uebernommen
#define P_AX25K  0x100
#define P_TELNET 0x200


typedef unsigned long bitfeld;

/*---------------------------------------------------------------------------*/

typedef enum event_t
{ e_nothing,
  e_ticsfull,
  e_reschedule,
  e_delay,
  e_kbhit
} event_t;

/*---------------------------------------------------------------------------*/

typedef enum io_t
{ io_dummy,
  io_window,
  io_editwin,
  io_tnc,
  io_tty,
  io_file,
  io_socket,
  io_memory,
  io_comp // DH3MB: Online-COMP
} io_t;

/*---------------------------------------------------------------------------*/

#define MONBUFLEN 2048

typedef struct monbuf_t
{ int      tid;
  bitfeld  opt;
  unsigned in;
  unsigned out;
  int      congested;
  char     buf[MONBUFLEN];
} monbuf_t;

/*---------------------------------------------------------------------------*/

#ifdef _AX25K
/*
#define TXLEN 512
#define RXLEN 512
#define MAXSOCK 256

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

extern sock_t so[MAXSOCK];
extern int ax25k_ok ;
*/
#endif

/*---------------------------------------------------------------------------*/

typedef void (* atkill_t)(void);
typedef void (* atkill2_t)(void *);
#define MAX_ATKILL 4

/*---------------------------------------------------------------------------*/

typedef struct task_t
{ event_t  event;
  int      taskid;
  int      port;
  int      lock;                // >0 avoids re-scheduling
  int      socket;
  int      childpid;
  io_t     input;
  io_t     output;
  monbuf_t *monitor;
  unsigned lastdelay;
  unsigned begintic;
  long     cputics;
  jmp_buf  j;
  unsigned processkind;
  time_t   created;
  time_t   last_input;
  time_t   kill_time;
  time_t   warn_time;
  int      kill_warn;
  unsigned keybuffer[32];
  int      keybufin;
  int      keybufout;
  void     *e;
  void     *b;
  atkill2_t atkill[MAX_ATKILL];
  void     *atkill_p[MAX_ATKILL];
  unsigned atkillcnt;
  char     name[20];
  // the sequence of lastcmd, lastfunc and stack is important for the stack
  // overflow detection verify_stack(), don't change it! (db1ras)
  char     lastcmd[60];        // letztes eingegebenes Kommando
  char     lastfunc[40];       // letzte aufgerufene Funktion
  char     stack[TASKSTACK+4]; // Stack
  char     downlink[10];       // letztes Digirufzeichen
} task_t;

/*---------------------------------------------------------------------------*/

#define _s_report      0
#define _s_replog      1
#define _s_serious     2
#define _s_fatal       3
#define _s_tr_abbruch  4

#ifdef __FLAT__
#define report      _s_report, __LINE__, __FILE__
#define replog      _s_replog, __LINE__, __FILE__
#define serious     _s_serious, __LINE__, __FILE__
#define fatal       _s_fatal, __LINE__, __FILE__
#define tr_abbruch  _s_tr_abbruch, __LINE__, __FILE__
#else
#define report      _s_report, 0, ""
#define replog      _s_replog, 0, ""
#define serious     _s_serious, 0, ""
#define fatal       _s_fatal, 0, ""
#define tr_abbruch  _s_tr_abbruch, 0, ""
#endif

extern int newkeywin;
extern int keytaskid;
extern int deskkey;
extern int init_off;
extern task_t *t;       // Zeiger auf aktuellen TCB
extern long systic;

void call_l2 (int off);

/*---------------------------------------------------------------------------*/

#ifdef _WXSTN
// for wx-station
typedef struct
{ int data_valid;
  long
        dayoyear,
        minuteofday,
        wind_beaufort;
  double
        sat_sp,            // saturation steam pressure
        rainltt,           // rain long term total
        steam_pressure,
        wind_chill,
        windspeed,
        outdoort,
        snowfall_limit,
        air_pres_qnh,
        air_pres_qfe,
        abs_humidity,
        dew_point,
        kond_niveau,
        indoort,           // indoor temperature
        humidity,
        hum_indoor,        // humidity indoor
        wind_direction;    // 0..360 degrees
        char *winddir_str;
        time_t t;
} wxdata_t;
#endif

/*---------------------------------------------------------------------------*/

typedef enum option_index_t
//*************************************************************************
//
//  Index fuer das Feld mit den Optionen
//
//*************************************************************************
{ o_id=0,  // info-dir
  o_ud,    // user-dir
  o_il,    // info-list
  o_ul,    // user-list
  o_ir,    // info-read
  o_ur,    // user-read
  o_ch,    // check
  o_ps     // ps
} option_index_t;

/*---------------------------------------------------------------------------*/

typedef enum umlaut_t
{ um_none,   // keine Umlautwandlung
  um_ibm,    // IBM-Umlaute
  um_din,    // DIN-Umlaute (statt [\]{|}~)
  um_aeoeue, // Wandlung nach AE OE UE
  um_iso     // Wandlung nach ISO (Linux,Windows)
} umlaut_t;

/*---------------------------------------------------------------------------*/

typedef enum fwdtype_t
{ fwd_none,
  fwd_standard,
  fwd_user,
  fwd_file
} fwdtype_t;

/*---------------------------------------------------------------------------*/

typedef enum loginmode_t
{ login_silent,
  login_standard,
  login_ufwd,
  login_fwd,
  login_nofwd
} loginmode_t;

/*---------------------------------------------------------------------------*/

typedef struct bulletin_t
{ char name[MAXTREELENGTH];
  byte flatoffset; //offset of name w/o path
  short unsigned lifetime_min;
  short unsigned lifetime_max;
  //time_t created;
  time_t newestmail;
} bulletin_t;

/*---------------------------------------------------------------------------*/

#ifdef _TELEPHONE // JJ
typedef struct ttypara_t
{ int      comirq;
  int      comadr;
  char     cmdatz[60];
  char     cmdata[60];
  char     cmdatd[60];
  char     ringindicator[60];
  char     connectindicator[60];
  char     dialprefix[60];
  char     dialsuffix[60];
  int      ringtimeout;
  int      dialtimeout;
  int      dialdelay;
  int      connectdelay;
  int      maxcontime;
  int      maxcons;
  int      debug;
  int      ttytimeout;
} ttypara_t;
#endif

/*---------------------------------------------------------------------------*/

typedef struct reject_t //DF3VI
{ char action;                 // R, G, E, F, H, L, P, O
  char sender[LINELEN+1];      // <
  char destin[LINELEN+1];      // >
  char bid[LINELEN+1];         // $
  char at[LINELEN+1];          // @
  char type;                   // .
} reject_t;

/*---------------------------------------------------------------------------*/

#define MK_MYCALLS   8

typedef struct mailpara_t
{ char     boxname[CALLEN+1];
  char     boxadress[HADRESSLEN+1];
  char     boxheader[45];
#ifdef __FLAT__
  int      tcp_port;
  int      ftp_port;
  int      http_port;
  int      smtp_port;
  int      pop3_port;
  int      nntp_port;
  int      serv_port;
  int      radio_port;
  unsigned nopopsmtp;
  unsigned httpaccount;
  unsigned httpguestfirst;
  unsigned httpttypw;
  unsigned httpshowsysopcall;
  unsigned httprobots;
  unsigned unsecurettypw;
#ifdef INETMAILGATE
  char     internetmailgate[CALLEN+1];
#endif
#endif
#if defined __LINUX__ && defined _AX25K
  char     ax25k_if[80];
#endif
#ifndef __LINUX__
  int      summertime;
#endif
#ifdef __DOS16__
  int      pcisutc;
  int      stimeoffset;
#endif
  unsigned short paclen;
  int minpaclen;
  int maxpaclen;
  unsigned altboardinfo;
  unsigned mailbeacon;
  unsigned timeoutwarning;
  unsigned nounknownroute;
  unsigned asklogin;
  unsigned smoothheader;
#ifdef __LINUX__
  unsigned addlinuxsystemuser;
#endif
  unsigned fwdssid;
  unsigned callformat; //format amateur/cb/both
#ifdef _LCF //JJ
  unsigned logincallformat;
#endif
  unsigned savebroken; // save broken 7+files:
           // 0..do not save, 1..save if fbb-fwding, 2..always save it
  char     sysopcall[CALLEN+1];
#ifdef _GUEST
  char     guestcall[CALLEN+1];
#endif
  unsigned infolife;
  unsigned userlife;
  unsigned oldestfwd;
  unsigned oldestbeacon;
  char     userpath[FNAMELEN+1];
  char     infopath[FNAMELEN+1];
#ifdef FILESURF // DH3MB
  char     fspath[FSPATHLEN+1];
  unsigned fsbmail;
#endif
  char     m_filter[FNAMELEN+1];
  char     prompt[PROMPTLEN+1];
  char     sprache[4];
  char     firstcmd[FIRSTCMDLEN+1];
  char     mycall[4][CALLSSID+1];
  char     macro_mycall[MK_MYCALLS][CALLSSID+1];
  unsigned mycalls;
  unsigned macro_mycalls;
  unsigned helplevel;
  unsigned zeilen;
  unsigned deflf;
  unsigned short defstatus;
  unsigned short defhold;
#ifdef FBBCHECKREAD
  unsigned deffbbcheckmode;
#endif
  unsigned tracelevel;
  unsigned fwdtrace;
  unsigned tcpiptrace;
#ifdef HB9EAS_DEBUG
  unsigned hb9eas_debug;
#endif
  unsigned dosinput;
  unsigned autotrcwin;
  unsigned sysopbell;
  unsigned usvsense;
  unsigned watchdog;
  unsigned userquota;
  unsigned maxlogins;
  unsigned disable;
  unsigned sf_only;
  unsigned disable_user;        //no user-logins allowed
  unsigned scrolldelay;
  unsigned fwdtimeout;
  unsigned usrtimeout;
  unsigned holdtime;
  unsigned crtsave;
  unsigned timeslot;
  unsigned hadrstore;
  unsigned userpw;
  unsigned readlock;
  unsigned eraselog;
  unsigned remerase;
  unsigned createboard;         // 0: TMP wird erzeugt, keine Subboards
                                // 1: TMP wird erzeugt, Subboards
                                // 2: kein TMP, keine Subboards
  long     maxbids;             // max. nr of bids
  unsigned bidoffset;           // offset of bids (multiple bbs)
  bitfeld  opt[8];              // eg. fwd-options
  unsigned long maxusers;       // max. nr. of users
#ifdef _AUTOFWD
  unsigned autofwdtime; // hrx
#endif
  unsigned nopurge;
#ifdef MAILSERVER // DH3MB
  unsigned maillistserv;
  unsigned maillistsender;
#endif
#ifdef DF3VI_POCSAG
  unsigned pocsaglistserv;
#endif
  unsigned oldumail;
  unsigned defswaplf;
#ifdef _TELEPHONE
  unsigned ttydevice;
#endif
#ifdef _WXSTN
  wxdata_t wx;
  char wxstnname[31];
  long wxqthaltitude;    // absolute altitude of qth in meter
  long wxsensoraltitude; // relative altitude of sensor over ground
  char wxtty[21];
  char wxpath[30];
#endif
#ifdef _MORSED
  char cwid[50];        //CW station ID  eg = QST DE OE1XUR =
  long cwdoid;          //1 should send ID now, 0 else
  char cwnextjob[50];   //defines next job to do eg. TXFILE morse.txt
  long cwfreq;          //frequ. of cw-signal in hz. eg. 800
  long cwms;            //duration of a "dot" in ms
#endif
  unsigned tellmode;    //0..inactive, 1..send ok, 2..send/receive ok
  unsigned ltquery;
#ifdef _BCMNET_LOGIN
  int pwonly;
#endif
  int defrea;
  int defhttpsurface;
#if defined FEATURE_SERIAL || defined _TELEPHONE
  char ttymode[20]; // z.B. 1:9600,8,N,1,E or /dev/tty7
#endif
#ifdef _PREPOSTEXEC //db1ras
  char dospreexec[CMDBUFLEN];
  char dospostexec[CMDBUFLEN];
#endif
} mailpara_t;

/*---------------------------------------------------------------------------*/

typedef struct conv_t
{ struct conv_t *next;
  char line[100];
} conv_t;

/*---------------------------------------------------------------------------*/

typedef struct tvar_t : public task_t
{ char     logincall[CALLEN+1]; // Rufzeichen der eingelogten Station
  char     uplink[CALLSSID+1];  // erstes Digirufzeichen bzw. 'Console'
  char     via_radio;           // 1 = connection via radio (for maxlogin)
  char     downlink[CALLSSID+1];// letztes Digirufzeichen
  char     peerip[18];          // IP-Nummer
  char     mycall[CALLSSID+1];  // Connect-Rufzeichen ZUR Box
  int      semalock;            // Flag: File war gesperrt
  char     bereich;             // Flag fuer Bereich wurde angegeben
  bitfeld  optplus;             // gesetzte Optionen der Kommandozeile
  bitfeld  optminus;            // geloeschte Optionen
  int      optgroup;            // Index fuer Option (vom Befehl abhaengig)
  bitfeld  opt;                 // modifizierte Option (abh. Bef/Opt/U/I)
  char     line[BUFLEN];        // Mehrzweckpuffer
  long     txbytes;             // Zaehler fuer gesendete Daten
  long     rxbytes;             // Zaehler fuer empfangene Daten
  unsigned sentfiles;           // Zaehler fuer gesendete Files
  unsigned readfiles;           // Zaehler fuer gelesene Files
  int      linecount;           // Zaehler fuer Zeilenumbruch (Ausgabe)
  int      abbruch;             // Flag fuer Ausgabe-Stop
  int      quit;                // Flag fuer 1=logout 2=fast logout
  char     eingabefehler;       // counter for input-errors
  char     mailtype;            // type of mail: B(ulletin), P(ersonal), ...
  char     fwdhold;             // mail will not be forwarded automatically
                                // !..mail will be forwarded
                                // B..binary, partner does not support
                                // E..error, no bid (bulletin)
                                // F..deleted after forward, mail will not be forwarded again
                                // F..mail on hold if S&F without pwd (reject.bcm)
                                // H..general hold (reject.bcm)
                                // K..no BIN mails
                                // L..loop, fwd loop
                                // L..mail on hold if entered local (reject.bcm)
                                // M..no 7plus mails
                                // M..m_filter, no fwd (errorlevel 2)
                                // N..FBB_NO already rcvd, partner had mail before
                                // O..mail on hold if without ax25/tty-pwd (reject.bcm)
                                // P..mail on hold if without ax25-pwd (reject.bcm)
                                // R..FBB_REJECT
                                // S..size, mail too large
                                // X..set to hold by sysop
                                // Z..mail on hold if sent from a guest
  char     fwdprop;             // mail was proposed to the forward-partner
  char     replied;             // mail was replied
  char     conttype;            // contenttype of mail
                                // !..unknown
                                // 6..7plus with errors
                                // 6..7plus encoded file
                                // 8..7plus info-file
                                // 9..7plus text-file
                                // H..html-encoded mail
                                // B..AutoBIN
                                // D..Didadit
  char     eraseinfo,           // duplicate to geloescht
                                // '!'.. not erased
                                //  F..forwarded
                                //  E..user-erase
                                //  S..sysop-erase
                                //  W..server(WP,mail,tell)-erase
                                //  K..local remote-erase
                                //  L..remote-erased (bbs=orig.bbs)
                                //  X..remote-erased (bbs differ)
                                //  T..erase due to transfer
           flag7,flag8,
           flag9,flaga,flagb,flagc,
           flagd,flage,flagf,flag0; // further flags, reserved for future use
                                    // set to ! (default)
#if 0
  char     chatbuffer[100];     // Zeile fuer Mitteilungen von anderen
#else
  conv_t  *conv;
  int      convtable;
#endif
  char     sysop;               // Flag: Benutzer hat Sysoprechte
  char     pwok;                // Benutzer hat Userrechte (PW wie DPBOX)
  FILE     *inputfile;          // Filepointer, wenn Eingabe umgeleitet ist
  FILE     *outputfile;         // Filepointer, wenn Ausgabe umgeleitet ist
  FILE     *oldfile;            // Merker fuer Filepointer
  char     oldoldinput;         // Merker fuer Standardeingabemerker
  char     oldinput;            // Merker fuer Standardeingabe
  char     oldoutput;           // Merker fuer Standardausgabe
  char     continous;           // Flag: Ausgabe wird nicht angehalten
  time_t   logintime;           // Merker, wann der Login war
  int      privoffset;          // index in PRIV-File (fuer Forward-PWD)
#ifdef FEATURE_MDPW
  char     password[33];        // zuletzt aktives Passwort (fuer Fwd)
#else
  char     password[6];         // zuletzt aktives Password (fwd!)
#endif
  int      job;                 // Wird in User-Liste angezeigt

  char     prompt[30];          // Call/Boardname im Prompt
  char     mask[FNAMELEN+1];    // momentan aktive Dir-Maske (ende *.*)
  int      index;               // Index in Board-Tree (nur temporaer)
  char     mailpath[FNAMELEN+1];// Name der aktiven Mail mit Pfad
  char     mailfname[10];       // nur Filename (=Datum/Uhrzeit) der Mail
  char     listpath[FNAMELEN+1];// Name des akiven Listfiles mit Pfad
  char     boardpath[FNAMELEN+1]; // Dateipfad fuer Board (z.B. "info/soft/gp")
  char     boardname[10];       // Name des Boards ohne Pfad (z.B. "GP")
  char     boardfullname[FNAMELEN+1]; // Boardname komplett (z.B. "SOFT/GP")
  char     ziel[DIRLEN+1];      // evtl unvollst. Ziel nach SEND
  char     zielboard[DIRLEN+1]; // DH3MB: Board nach convert.bcm
  char     herkunft[CALLEN+1];  // Call des Verfassers der Mail
  char     frombox[CALLEN+1];   // Box, von der WIR die Mail bekommen haben
  char     at[HADRESSLEN+1];    // vollstaendige H-Adresse der Nachricht
  char     bid[BIDLEN+1];       // BID der letzten Nachricht, evtl neu
  long     binstart;            // Position des BIN-Teils in der Mail-Datei
  char     destboxlist[DESTBOXLEN+1];   // Forward-Liste der Nachricht
  char     usermail;            // Nachricht ist User-Nachricht
  char     gelesen;             // Nachricht ist schon gelesen
  char     geloescht;           // Nachricht ist geloescht
  int      lifetime;            // Lifetime (insgesamt, nicht aktuell)
  unsigned lines;               // Anzahl Zeilen der letzten Nachricht
  long     bytes;               // Anzahl Bytes der letzten Nachricht
  char     betreff[BETREFFLEN+1]; // Titel der Nachricht
  time_t   abwann;              // Zeitangabe fuer DIR AFTER etc.
  unsigned long beginn;         // Bereich fuer DIR READ ERASE CHECK
  unsigned long ende;           // Bereich fuer DIR READ ERASE CHECK
  char     replyboard[10];      // Boardname der zuletzt gelesenen Nachricht
  unsigned replynum;            // Nummer der zuletzt gelesenen Nachricht
  char     tmpbid[5][BIDLEN+1]; // Momentan empfangene BIDs
  int      boardlife_max;       // momentan aktive maximale Lifetime
  int      boardlife_min;       // momentan aktive minimale Lifetime
  int      msg_loadnum;         // "magic number" fuer geladene MSG-Version
  int      msg_offset;
  time_t   lastdirnews;         // exakter Zeitpunkt des D N (Check)
  fwdtype_t forwarding;         // fwd_none, fwd_standard, fwd_user
  void    *fwd_parameter;       // ggf. Zeiger auf fpara-struktur
  char     boxtyp;              // Software, die beim Partner rennt
  char     boxversion;          // Version der Forwardpartner-Box
  void    *script_p;
  char    *inbuffer;
  int      inbindex;
  int      inbmax;
  char    *outbuffer;
  int      outbindex;
  int      outbmax;
  int      http;
  int      ftp;
  unsigned charset;
  jmp_buf  io_mem_jmp;
  user_t   u;                   // USER-Struktur fuer eingeloggten User
#ifdef __FLAT__
  int      headermode;
#endif
#ifdef FILESURF // DH3MB
  char     fspath[100];
  char     fsmode;
#endif
  unsigned long fbboffset; // DH3MB
  char     pacbytes;            // like txbytes but one packet only
#ifdef _BCMNET_LOGIN
  char     logintype;           // hpk: Is the call a normal user or a guest?
                                //      0 ... guest / 1 ... normal
#endif
} tvar_t;

/*---------------------------------------------------------------------------*/

#define WPROT_MIN_ROUTING   32767

typedef struct fwdpara_t
{
  char    call[CALLEN+1];       // Rufzeichen der Nachbarbox
  char    concall[FSPATHLEN];   // Connect-Rufzeichen der Nachbarbox
  char    times[25];            // Bytefeld fuer die Uhrzeiten
  long    maxbytes;             // maximale Anzahl pro geforwardetes File
  bitfeld options;              // eingestellte Optionen
  char    connectbar;           // war beim letzten Versuch erreichbar
  int     nolink;               // Zaehler wie oft hintereinander kein Link moeglich war
  time_t  lastconok;            // Zeitpunkt letzter i.O.-Connect
  time_t  lastlogin;            // Zeitpunkt letzter eingehender Connect
  time_t  lastwp;               // Zeitpunkt letzter WPROT-Update
  time_t  sidrtt;
  long    timeout_cnt;          // number of timeouts
  long    n_conok;              // number of outgoing connects
  long    n_login;              // number of incoming connects
  long    txf;                  // number of files sent
  long    rxf;                  // number of files received
  char    ssid;
  char    newmail;
  char    mid;
  char    u_onlydelay;
  char    i_onlydelay;
  int     fwdcallformat;        // callformat allowed for forwarding

  // WPROT routing
  int     wprot_r_partner;      // Fwd-Partner versteht WPROT R-Zeilen
  time_t  lastmeasure;          // Zeitpunkt letzte WPROT-R-Line-Messung
  time_t  lastwpr;              // Zeitpunkt letzter WPROT-R-Line-Update
  long    routing_quality;      // Quality der letzten 5 Stunden
  long    routing_txf;          // number of files sent since last WPROT
//  short   routing_hops;         // Hops der letzten 5 Stunden
  long    current_routing_quality; // Quality der letzten Messung
//  short   current_routing_hops;    // Hops der letzten Messung
//  char    routing_neighbour[CALLEN+1];  // Routing ueber diese Partnerbox

} fwdpara_t;

/*---------------------------------------------------------------------------*/

typedef struct wpdata_t
{
  char   call[CALLEN+1];
  char   bid[BIDLEN+1];
  char   bbs[HADRESSLEN+1];
  char   logincall[CALLEN+1];
  char   origin[CALLEN+1];
  char   name[NAMELEN+1];
  char   zip[ZIPLEN+1];
  char   qth[QTHLEN+1];
  char   hadr[HADRESSLEN+1];
  char   bstatus[BSTATUSLEN+1];
  char   sysopcall[CALLEN+1];
  char   protocol[BPROTOLEN+1];
  char   hwaddress[BHWADDRESSLEN+1];
  time_t mybbstime;
  short  hops;
  long   qual;
  long   bversion;
} wpdata_t;

/*---------------------------------------------------------------------------*/

typedef enum boxkind_t
{ b_baycom,
  b_dxl,
  b_thebox,
  b_fbb,
  b_rli,
  b_mbl,
  b_dp,
  b_wgt,
  b_unknown
} boxkind_t;

/*---------------------------------------------------------------------------*/

typedef enum mail_modify_t
{ w_erase,
  w_unerase,
  w_forward,
  w_transfer,
  w_setlifetime,
  w_reply,
  w_comment,
#ifdef DF3VI_EXTRACT
  w_extract, //extract-funktion
#endif
  w_unread,  //unread-funktion (Ruecksetzen des gelesen-flags)
  w_hold
} mail_modify_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ unbekannt,
  bekannt,
  bleibtliegen,
  auto_bekannt,
  active_bekannt
} fwd_weiter_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ sh_noinput,
  sh_forceinput,
  sh_ifmultitask,
  sh_interactive
} shellinput_t;

/*---------------------------------------------------------------------------*/

typedef struct hadr_t
{
  long   bulletins;                  // 4
  long   usermails;                  // 4
  time_t delay[ADJNUM];              // 80
  time_t lastupdate[ADJNUM];         // 80
  time_t lasthtime;                  // 4 latest header-update scanned (mail)
  time_t lastwprcvd;                 // 4 latest wp-update received
  time_t lastwpsend;                 // 4 latest wp-update sent
  long   bversion;                   // 4
  time_t r_time_rx;                  // 4
  time_t r_time_tx;                  // 4
  time_t reserved_t[22];             // 88
  short  int rel_mails[ADJNUM];      // 40
  short  int hops[ADJNUM];           // 40
  short  unsigned nextsamehash;      // 2
  short  reserved_s[25];             // 50
  char   adr[HADRESSLEN+1];          // 40
  char   adjacent[ADJNUM][7];        // 140
  char   lastheader[HEADERLEN+1];    // 61
  char   lastbid[BIDLEN+1];          // 13
  char   lastboard[DIRLEN+1];        // 9
  char   lastuser[CALLEN+1];         // 7
  char   bstatus[BSTATUSLEN+1];      // 21
  char   sysopcall[CALLEN+1];        // 7
  char   protocol[BPROTOLEN+1];      // 11
  char   hwaddress[BHWADDRESSLEN+1]; // 50
  char   r_from[CALLEN+1];           // 7
  short  r_hops;                     // 2
  long unsigned r_qual_rx;           // 4
  char   reserved[240];              // 240
  } hadr_t;                          // = 1024 Bytes

/*---------------------------------------------------------------------------*/

#define ADJOLDNUM 5

#ifdef __MSDOS__
typedef struct hadrold_t
{ char   adr[HADRESSLEN+1];           // 40
  char   adjacent[ADJOLDNUM][7];      // 35
  short  int rel_mails[ADJOLDNUM];    // 10
  time_t delay[ADJOLDNUM];            // 20
  time_t lastupdate[ADJOLDNUM];       // 20
  short  int hops[ADJOLDNUM];         // 10
  long   bulletins;                   // 4
  long   usermails;                   // 4
  time_t lasthtime;                   // 4
  char   lastheader[HEADERLEN+1];     // 61
  char   lastbid[BIDLEN+1];           // 13
  char   lastboard[9];                // 9
  char   lastuser[7];                 // 7
  char   reserved[17];                // 51
  short unsigned nextsamehash;        // 2
} hadrold_t;
#else
typedef struct hadrold_t              // 290
{
  long   bulletins;                   // 4
  long   usermails;                   // 4
  time_t delay[ADJOLDNUM];            // 20
  time_t lastupdate[ADJOLDNUM];       // 20
  time_t lasthtime;                   // 4
  short  int rel_mails[ADJOLDNUM];    // 10
  short  int hops[ADJOLDNUM];         // 10
  short  unsigned nextsamehash;       // 2
  char   adr[HADRESSLEN+1];           // 40
  char   adjacent[ADJOLDNUM][7];      // 35
  char   lastheader[HEADERLEN+1];     // 61
  char   lastbid[BIDLEN+1];           // 13
  char   lastboard[9];                // 9
  char   lastuser[7];                 // 7
  char   reserved[17];                // 51
} hadrold_t;
#endif

/*---------------------------------------------------------------------------*/

typedef enum
{ m_sendnotpossible,    // send.cpp
  m_boardnotpossible,   // read.cpp, send.cpp
  m_messagenotforward,
  m_forwardunknown,
  m_title,
  m_textinput,
  m_textsend,
  m_linesfor,
  m_stored,
  m_msgarrived,
  m_nomessagestored,
  m_bidexists,
  m_address,
  m_fwdunknown,
  m_isknown,
  m_isadjacent,         // obsolete
  m_noforwarding,
  m_syntaxpath,
  m_adrunknown,         // read.cpp, tell.cpp, ping.cpp
  m_givenumber,
  m_erased,
  m_unerased,
  m_forwarded,
  m_syntaxforward,
  m_lifetimeset,
  m_syntaxlifetime,
  m_tr_ok,              // read.cpp
  m_tr_syntax,          // read.cpp
  m_permissiondenied,
  m_messagewillbe,
  m_messageswillbe,
  m_nomessagefound,     // read.cpp
  m_erasemessage,
  m_messageerased,
  m_messageiserased,
  m_noboxfound,         // dir.cpp
  m_dirofusers,
  m_dirofbulletins,
  m_boardlifemails,
  m_directoryof,
  m_list_nr,            // check.cpp, dir.cpp
  m_list_call,          // check.cpp, dir.cpp
  m_list_board,         // check.cpp, dir.cpp
  m_list_date,          // check.cpp, dir.cpp
  m_list_time,          // check.cpp, dir.cpp
  m_list_lines,         // check.cpp, dir.cpp
  m_list_bytes,         // check.cpp, dir.cpp
  m_list_bid,           // check.cpp, dir.cpp
  m_list_life,          // check.cpp, dir.cpp
  m_list_forw,          // check.cpp, dir.cpp
  m_list_subject,       // check.cpp, dir.cpp
  m_lastmessage,
  m_nounread,
  m_forcallnomessage,
  m_dirofusermails,
  m_dirafter,
  m_searchfor,
  m_nomessagesfound,
  m_dirpath,
  m_list_mbx,           // check.cpp
  m_checkentries,
  m_checknoaccess,
  m_logoutput,
  m_noentryfound,
  m_entriesfound,
  m_oneentryfound,
  m_actual,
  m_noactual,
  m_servus,
  m_helplines,
  m_lastlogin,
  m_firstlogin,
  m_inputmybbs,
  m_maintenance,        // main.c
  m_userbusynologin,
  m_conversto,
  m_conversend,
  m_userbusy,
  m_syntaxconvers,
  m_quotaexceed,
  m_nobulletinmore,
  m_cmdinvalid,
  m_possiblecommands,
  m_syntaxmybbs,
  m_syntaxname,
  m_syntaxsend,
  m_noreplypossible,
  m_dirunknown,         // sysop.cpp
  m_nofwdmycall,
  m_date_invalid,
  m_syntaxwall,
  m_mainmenu,
  m_mybbsset,           // alter.cpp
  m_mybbsforward,
  m_neverlogin,
  m_a_headline,
  m_a_mybbs,
  m_a_reject,
  m_a_prompt,
  m_a_firstcmd,
  m_a_name,
  m_a_speech,
  m_a_lines,
  m_a_helplevel,
  m_a_linefeed,
  m_a_idir,
  m_a_udir,
  m_a_ilist,
  m_a_ulist,
  m_a_iread,
  m_a_uread,
  m_a_check,
  m_nohelp,
  m_a_name2,
  m_a_lastdirnew,
  m_a_lastquit,
  m_a_lastlogin,
  m_a_loginmails,
  m_isunknown,
  m_optionsfor,
  m_optionsset,
  m_alterunknown,
  m_dispmybbs,
  m_inputaddress,
  m_adrinvalid,         // alter.cpp
  m_cmdtoolong,
  m_commandquit,
  m_cmdstored,
  m_prompttoolong,
  m_promptstored,
  m_pwdtoolong,
  m_pwderased,
  m_pwdlength,
  m_helpvalues,
  m_helpstored,
  m_lfvalues,
  m_nolinefeeds,
  m_onelinefeed,
  m_linefeeds,
  m_nolines,
  m_linesset,
  m_linesvalues,
  m_default,
  m_defaultquery,
  m_defaultmessage,
  m_nametoolong,
  m_nameset,
  m_toomanyreject,
  m_rejectedboards,
  m_languageok,
  m_possiblespeech,
  m_userstateset,
  m_quotaset,
  m_quotaoutput,
  m_maxquota,
  m_yesnoquestion,      // inout.cpp
  m_aborted,
  m_helpkeywords,
  m_helpnotfound,
  m_boardpurge,
  m_purgeresult,
  m_tr_notpossible,     // read.cpp
  m_msg_processed,      // read.cpp
  m_msges_processed,    // read.cpp
  m_notallowed,         // read.cpp
  m_notavailable,
  m_http_not,           // main.cpp
  m_no_pw,              // main.cpp
  m_a_no_pw,            // alter.cpp
  m_too_short,          // utils.cpp
  m_filenotfound,       // binsplit.cpp, fileio.cpp, sysop.cpp
  m_filemissing,        // fileio.cpp
  m_filenotopen,        // fileio.cpp, sysop.cpp
  m_fileexists,         // fileio.cpp
  m_filealexists,       // filesurf.cpp
  m_filewaiting,        // fileio.cpp
  m_filenotstored,      // fileio.cpp
  m_filenotrm,          // fileio.cpp
  m_filewriteerror,     // fileio.cpp
  m_filestored,         // fileio.cpp
  m_filenobytes,        // fileio.cpp
  m_filecrcerror,       // fileio.cpp
  m_fs_welcome,         // filesurf
  m_fs_unknown,         // filesurf
  m_fs_end,             // filesurf
  m_fs_directories,     // filesurf
  m_fs_nowrite,         // filesurf
  m_fs_noaccess,        // filesurf
  m_fs_filesfound,      // filesurf
  m_fs_inputtoolong,    // filesurf
  m_fs_nametoolong,     // filesurf
  m_fs_filesindir,      // filesurf
  m_fs_filemask,        // filesurf
  m_fs_dirtoocomplex,   // filesurf
  m_fs_oldnewname,      // filesurf
  m_fs_driveunavailable, // filesurf
  m_fs_bmaildisabled,    // filesurf
  m_fs_currentpath,      // filesurf
  m_notimplemented,     // filesurf
  m_file,               // filesurf
  m_directory,          // filesurf
  m_error,              // filesurf/sysop.cpp
  m_moving,             // filesurf/sysop.cpp
  m_copying,            // filesurf/sysop.cpp
  m_nach,               // filesurf/sysop.cpp
  m_fs_alreadyexist,    // filesurf
  m_directorycreated,      // filesurf/sysop.cpp
  m_directorycantcreated,  // filesurf/sysop.cpp
  m_fs_directorydeleted,      // filesurf
  m_fs_directorycantdeleted,  // filesurf
  m_fs_directorynotexist,     // filesurf
  m_filedeleted,        // filesurf/sysop.cpp
  m_filecantdeleted,    // filesurf/sysop.cpp
  m_fs_filenotexist,    // filesurf
  m_filename,           // filesurf
  m_directoryname,      // filesurf
  m_fwd_toomanydest,
  m_fwd_toomanycalls,
  m_unknownopt,
  m_http_commands,      // http menue
  m_http_home,          // http menue
  m_http_user,          // http menue
  m_http_login,         // http menue
  m_http_createaccount, // http menue
  m_http_removecookie,  // http menue
  m_http_mail,          // http menue
  m_http_listusermail,  // http menue
  m_http_sendsysop,     // http menue
  m_http_send,          // http menue
  m_http_check,         // http menue
  m_http_setchecktime,  // http menue
  m_http_newestmails,   // http menue
  m_http_boards,        // http menue
  m_http_baybox,        // http menue
  m_http_system,        // http menue
  m_http_loginstats,    // http menue
  m_http_logging,       // http menue
  m_http_sfstats,       // http menue
  m_http_stuck,         // http menue
  m_http_unknownbbs,    // http menue
  m_http_ps,            // http menue
  m_http_u,             // http menue
  m_http_ver,           // http menue
  m_http_help,          // http menue
  m_http_helpindex,     // http menue
  m_http_helpnewbies,   // http menue
  m_forwardqueue,       // dir.cpp
  m_extract_errorread,        // extract.cpp
  m_extract_errorwrite,       // extract.cpp
  m_extract_nostartline,      // extract.cpp
  m_extract_invalidstartline, // extract.cpp
  m_extract_invalidheader,    // extract.cpp
  m_extract_filenamemissing,  // extract.cpp
  m_extract_filecantcreate,   // extract.cpp
  m_extract_filecreate,       // extract.cpp
  m_extract_extract7p,        // extract.cpp
  m_extract_errorwritebin,    // extract.cpp
  m_extract_extractbin,       // extract.cpp
  m_nomem,                    // extract.cpp
  m_usersfound,         // users.cpp
  m_usersknown,         // users.cpp
  m_usersknownlocal,    // users.cpp
  m_nouser3,            // users.cpp
  m_usersimported,      // users.cpp
  m_yapp_fileexists,    // yapp.cpp
  m_yapp_waiting,       // yapp.cpp
  m_yapp_nr,            // yapp.cpp
  m_yapp_abortrx,       // yapp.cpp
  m_yapp_aborttx,       // yapp.cpp
  m_yapp_cn,            // yapp.cpp
  m_yapp_noat,          // yapp.cpp
  m_rxok,               // fileio/yapp/didadit.cpp
  m_txok,               // fileio/yapp/didadit.cpp
  m_incomingbids,       // bids.cpp
  m_conv_notloggedon,    // convers.cpp
  m_conv_linespending,   // convers.cpp
  m_conv_toomanypending, // convers.cpp
  m_cvt_toomany,         // convert.cpp
  m_cvt_sorted,          // convert.cpp
  m_cvt_added,           // convert.cpp
  m_cvt_deleted,         // convert.cpp
  m_cvt_created,         // convert.cpp
  m_cvt_notexist,        // convert.cpp
  m_cvt_oldexist,        // convert.cpp
  m_notfound,           // fts/hadr.cpp
  m_fwd_lineerr,        // fwd.cpp
  m_fwd_boxexist,       // fwd.cpp
  m_fwd_newentryadded,  // fwd.cpp
  m_fwd_novalidcall,    // fwd.cpp
  m_fwd_entrydeleted,   // fwd.cpp
  m_fwd_optionset,      // fwd.cpp
  m_fwd_optionreset,    // fwd.cpp
  m_fwd_callchanged,    // fwd.cpp
  m_fwd_callnotfound,   // fwd/fwd_file.cpp
  m_fwd_entryexist,     // fwd.cpp
  m_fwd_entryadded,     // fwd.cpp
  m_fwd_entrynotfound,  // fwd.cpp
  m_fwd_cannotdelete,   // fwd.cpp
  m_fwd_novalidtime,    // fwd.cpp
  m_fwd_pathtoolong,    // fwd.cpp
  m_fwd_timechanged,    // fwd.cpp
  m_fwd_pathchanged,    // fwd.cpp
  m_matchinglines,      // grep.cpp
  m_badregex,           // grep.cpp
  m_fwd_import,         // fwd_file.cpp
  m_fwd_badimport,      // fwd_file.cpp
  m_fwd_export,         // fwd_file.cpp
  m_login_maxlog,       // login.cpp
  m_disableaway,        // login.cpp
  m_omi_errboard,       // oldmaili.cpp
  m_omi_nobid,          // oldmaili.cpp
  m_omi_errbid,         // oldmaili.cpp
  m_omi_boardcreated,   // oldmaili.cpp
  m_omi_bidisknown,     // oldmaili.cpp
  m_omi_bidisnew,       // oldmaili.cpp
  m_omi_reorg,          // oldmaili.cpp
  m_omi_result,         // oldmaili.cpp
  m_purgedisabled,      // purge.cpp
  m_purgeallstarted,    // purge.cpp
  m_purgerunning,       // purge.cpp
  m_lifetimelimits,     // read.cpp
  m_norunutils,         // runutils.cpp
  m_runutilsactive,     // runutils.cpp
  m_dieboximported,     // users.cpp
  m_omi_started,        // sysop.cpp
  m_beaconstarted,      // sysop.cpp
  m_beaconframes,       // sysop.cpp
  m_flexstarted,        // sysop.cpp
  m_reorginvoked,       // sysop.cpp
  m_postfwdinvoked,     // sysop.cpp
  m_loginchanged,       // sysop.cpp
  m_boxdisabled,        // sysop.cpp
  m_boxenabled,         // sysop.cpp
  m_loginpw,            // sysop.cpp
  m_userpwlines,        // sysop.cpp
  m_hadrstat,           // sysop.cpp
  m_lifetimestat,       // sysop.cpp
  m_cantkilltask,       // sysop.cpp
  m_boardcreated,       // sysop.cpp
  m_mainboardnotfound,  // sysop.cpp
  m_toomanyboards,      // sysop.cpp
  m_boardnameexist,     // sysop.cpp
  m_invalidboardname,   // sysop.cpp
  m_boardremoved,       // sysop.cpp
  m_boardnotfound,      // sysop.cpp
  m_boardnotempty,      // sysop.cpp
  m_boardmoved,         // sysop.cpp
  m_newboardnotfound,   // sysop.cpp
  m_oldboardnotfound,   // sysop.cpp
  m_startphonefwd,      // sysop.cpp
  m_nomodem,            // sysop.cpp
  m_nottyactive,        // sysop.cpp
  m_hangupmodem,        // sysop.cpp
  m_ttyactive,          // sysop.cpp
  m_invalidtaskid,      // task.cpp
  m_taskmonbusy,        // task.cpp
  m_montask,            // task.cpp
  m_taskbuffer,         // task.cpp
  m_taskmondied,        // task.cpp
  m_cpuload,            // timerint.cpp
  m_usingnewcall,       // transfer.cpp
  m_unread,             // read.cpp
  m_startdidadit,       // read.cpp
  m_startbinsplit,      // read.cpp
  m_bintxabort,         // read.cpp
  m_sysoponly,          // pocsag.cpp
  m_nogroupactive,      // pocsag/mailserv.cpp
  m_groupusers,         // pocsag/mailserv.cpp
  m_poc_disabled,       // pocsag.cpp
  m_poc_nouser,         // pocsag.cpp
  m_nameofgroup,        // pocsag/mailserv.cpp
  m_description,        // pocsag/mailserv.cpp
  m_groupdesc,          // pocsag/mailserv.cpp
  m_poc_userc,          // pocsag.cpp
  m_groupnewdesc,       // pocsag/mailserv.cpp
  m_groupremoved,       // pocsag/mailserv.cpp
  m_groupexists,        // pocsag/mailserv.cpp
  m_groupcreated,       // pocsag/mailserv.cpp
  m_groupnotexist,      // pocsag/mailserv.cpp
  m_alreadyuseringroup,    // pocsag/mailserv.cpp
  m_youalreadyuseringroup, // pocsag/mailserv.cpp
  m_nowuseringroup,        // pocsag/mailserv.cpp
  m_younowuseringroup,     // pocsag/mailserv.cpp
  m_userdeleteingroup,     // pocsag/mailserv.cpp
  m_youdeleteingroup,      // pocsag/mailserv.cpp
  m_nouseringroup,         // pocsag/mailserv.cpp
  m_younouseringroup,      // pocsag/mailserv.cpp
  m_mailservdisabled,      // mailserv.cpp
  m_onlymaintainer,        // mailserv.cpp
  m_invalidnr,             // mailserv.cpp
  m_groupoptionset,        // mailserv.cpp
  m_groupoptionreset,      // mailserv.cpp
  m_msv_serialset,         // mailserv.cpp
  m_msv_nowmaint,          // mailserv.cpp
  m_msv_alreadymaint,      // mailserv.cpp
  m_msv_nomaint,           // mailserv.cpp
  m_msv_notmaint,          // mailserv.cpp
  m_msv_userc,             // mailserv.cpp
  m_msv_nextnr,            // mailserv.cpp
  m_msv_options,           // mailserv.cpp
  m_msv_maints,            // mailserv.cpp
  m_awayendtimedisabled,   // alter.cpp
  m_awayendtime,           // alter.cpp
  m_awaydisablednoend,     // alter.cpp
  m_awayenabled,           // alter.cpp
  m_awaydisabled,          // alter.cpp
  m_awaymsgtoolong,        // alter.cpp
  m_notidisabled,          // alter.cpp
  m_noticallinvalid,       // alter.cpp
  m_notienabled,           // alter.cpp
  m_zipset,                // alter.cpp
  m_ziptoolong,            // alter.cpp
  m_qthset,                // alter.cpp
  m_qthtoolong,            // alter.cpp
  m_newcallremoved,        // alter.cpp
  m_newcallinvalid,        // alter.cpp
  m_newcallstored,         // alter.cpp
  m_dirformatsaved,        // alter.cpp
  m_reallynolinefeeds,     // alter.cpp
  m_validpaclenrange,      // alter.cpp
  m_ttypwdisabled,         // alter.cpp
  m_ttypwok,               // alter.cpp
  m_ttypw8char,            // alter.cpp
  m_featnotincluded,       // alter.cpp
  m_linuxpwset,            // alter.cpp
  m_linuxpwok,             // alter.cpp
  m_linuxpw8char,          // alter.cpp
  m_huffcompon,            // alter.cpp
  m_huffcompoff,           // alter.cpp
  m_nopurgeoff,            // alter.cpp
  m_nopurgeon,             // alter.cpp
  m_paclendisable,         // alter.cpp
  m_deleted,               // alter.cpp
  m_unsecuresmtpon,        // alter.cpp
  m_unsecuresmtpoff,       // alter.cpp
  m_echoon,                // alter.cpp
  m_echooff,               // alter.cpp
  m_usingpw,               // alter.cpp
  m_notpossguest,          // alter.cpp
  m_nopwset,               // alter.cpp
  m_pwdisabled,            // alter.cpp
  m_mybbsnotset,           // alter.cpp
  m_userfwdoff,            // alter.cpp
  m_userfwdpassive,        // alter.cpp
  m_userfwdon,             // alter.cpp
  m_loginpwinactive,       // alter.cpp
  m_loginpwtype,           // alter.cpp
  m_sfpwinactive,          // alter.cpp
  m_sfpwtype,              // alter.cpp
  m_binpartssent,          // alter.cpp
  m_protocol,              // alter.cpp
  m_fbbcheckmodeon,        // alter.cpp
  m_fbbcheckmodeoff,       // alter.cpp
  m_httpsurface0,          // alter.cpp
  m_httpsurface1,          // alter.cpp
  m_httpsurface2,          // alter.cpp
  m_httpsurface3,          // alter.cpp
  m_saved,                 // alter.cpp
  m_otherlogs,             // sysop.cpp
  m_currentlog,            // login.cpp
  m_userlog,               // login.cpp
  m_fwdlog,                // login.cpp
  m_http_goup,             // http.cpp
  m_http_goback,           // http.cpp
  m_http_sendmail,         // http.cpp
  m_http_clearform,        // http.cpp
  m_readnextmail,          // read.cpp
  m_readprevmail,          // read.cpp
  m_erase,                 // read.cpp
  m_erasereply,            // read.cpp
  m_commenttext,           // read.cpp
  m_comment,               // read.cpp
  m_replytext,             // read.cpp
  m_reply,                 // read.cpp
  m_http_subject,          // http.cpp
  m_http_lifetime,         // http.cpp
  m_http_to,               // http.cpp

  NUM_MESSAGES,            // needs a dummy line in messages.*
  m_helpext=10000
} MSG_TYP;

/*---------------------------------------------------------------------------*/

typedef enum delta_t
{ zs_runtime,
  zs_seconds,
  zs_cputime,
  zs_cpuexact
} delta_t;

/*---------------------------------------------------------------------------*/

//RX: A->B
//TX: B->A
typedef struct
{
  char a;
  char b;
} charset_entry;

typedef struct
{
  unsigned id;
  char *name;
  charset_entry *chars;
} charset;

/*---------------------------------------------------------------------------*/

extern charset** charsets;
extern unsigned treelen;
extern bulletin_t *tree;
extern unsigned fwdpartners;
extern unsigned fwddestlen;
extern unsigned fwddest;
extern mailpara_t m;
#ifdef _TELEPHONE //JJ
extern ttypara_t tty;
#endif
extern time_t einschaltzeit;
extern time_t initzeit;
extern tvar_t *b;
extern user_t *u;
extern int boxinaktiv;
extern int reorg_noetig;
extern char abortreason[];
extern int runterfahren;
extern char stopreason[];
extern unsigned cvtlen;

/*---------------------------------------------------------------------------*/

//****************************
//       sema.cpp
//****************************
#define         fopen error
#define         fclose error
void            sema_list (void);
void            openfile_list (void);
FILE            *s_fopen (char *fname, char *mode);
handle          s_open (char *fname, char *mode);
int             s_set (handle fh, char *fname, char *mode);
void            s_fsetopt (FILE *f, char opt);
void            s_setopt (handle fh, char opt);
void            s_fclose (FILE *f);
void            s_close (handle fh);
void            close_task (int tid);
void            sema_init (void);
int             sema_send (void);
int             sema_access (char *name);
int             sema_test (char *name);
int             sema_lock (char *name);
void            sema_unlock (char *name);

//****************************
//       task.cpp
//****************************
int             fork (int typ, int port, void(*stfunc)(char *), char *stpar);
void            atkill (atkill_t func);
void            atkill (atkill2_t func, void *p);
void            rm_atkill (atkill_t func);
void            suicide (void);
int             waitfor (event_t event);
#ifdef __MSDOS__
unsigned        waitkey (void);
#endif
unsigned        waitalt (void);
int             wdelay (unsigned zeit);
void            maintask (void);
void            initstack (void);
void            setkeyhere (void);
int             iskeytask (void);
int             keywindow (void);
int             gettaskid (void);
char            *gettaskname (int tid);
void            listprocess (long opt);
int             killtask (int tid, int discon_);
int             killport (int port);
int             taskport0 (int port);
unsigned        listusers (unsigned putit);
unsigned        eingelogt(char *logincall, int job, int count);
#ifdef _TELEPHONE
int             taskexists (int tid); //JJ
#endif
int             taskkbhit (void);
int             mbtalk (char *fromcall, char *tocall, char *message);
int             monitor (int tid, bitfeld opt);
int             setkeywindow (int fenster);
void            lastcmd (char *cmd);
void            lastfunc (char *cmd);
void            post_mortem_dump (char *);
void            task_idle (int active);
void            timeout (int minutes);
void            test_killtime (void);
void            t_lock (void);
void            t_unlock (int force);

//****************************
//       trace.cpp
//****************************
void            log_entry (char *logname, char *msg);
void            trace (unsigned mode, int line, char *file,
                       char *programm, char *string, ...);
void            tracewindow (char *name);
void            cmdlog (char *befehl);
void            tracelog (char *message, unsigned mode);
void            pwlog (char *mycall, char *digi, char *msg);
#ifdef __FLAT__
void            nntplog (char *mode, char *string);
void            httplog (char *mode, char *string);
void            pop3log (char *mode, char *string);
void            smtplog (char *mode, char *string);
void            ftplog (char *mode, char *string);
#endif
void            wprotlog (char *string, char *call);


//****************************
//       memalloc.cpp
//****************************
void            *t_malloc (size_t size, char *name);
void            *t_realloc (void *blk, size_t newsize);
void            t_free (void *blk);
void            t_alloclist (char *befbuf); //df3vi: sortieroptionen
void            t_allocinit (void);
void            t_allfree (int task);
void            t_checkfree (void);
int             t_settaskid (void *blk, int tid);

//****************************
//      utils.cpp
//****************************
extern char     globalpath[];
void            fixpath(char *argv0);
void            subst (char *s, char i, char j);
void            subst1 (char *s, char i, char j);
void            rm_crlf (char *line);
char            *skip (char *s);
int             blkill (char *s);
void            cut_blank (char *s);
void            cut_blanktab (char *s);
#ifdef __FLAT__
void            rm_esc (char *s);
#endif
int             nextdez (char **bf);
int             readcmd (char *beftab[], char **buffer, unsigned int minimum);
char            *makeheader(int full);
char            *dezasc(long dez,int stellen);
long            nonlin(long bytes);
long            inonlin(long bytes);
long            ascdez(char *s,int stellen);
char            deznib(int dez);
int		nibdez (char nibble);
char            *time2filename(time_t unixtime);
time_t          filename2time(char *name);
int             mbcallok(char *call);
#ifdef _LCF
int             fwdcallok(char *call, int fwdcallformat); //JJ
#endif
char            mbmailt(char *ziel,char *hadr);
int             mbhadrok(char *hadr);
void            findbereich(char *buf);
void		seek_fname_preparg(char *a);
time_t          seek_fname(handle fh, char *fname, char *ok, int alle);
int             seek_lastentry(handle fh, unsigned long anzahl);
void            scanoptions(char *buf);
void            formoptions(void);
char            *chartomakro(char c,char *ms);
char            *expandmakro(char *s,char *os);
int             filecopy(char *src,char *dest);
int             xmkdir(char *path);
void            leerzeile(void);
char            *getheaderadress(char *rline);
char            *getlt(char *rline);
char            *getbid(char *rline);
time_t          getheadertime(char *rline);
char            *atcall(char *at);
int             line_7p(char *p);
int             returnmailok(void);
int             homeadr(char *adr);
int             autosysop(void); //JJ
void            rubrik_completition(char *); //dk2ui
unsigned int    hd_space (long flen);
char            interactive (void); //db1ras
unsigned        strcrc (char *s);
char            *stristr (char *s1, char *s2);
int             strpos (char *s1, char *s2);
char            *nexttoken (char *buf, char *token, int maxlen);
char            *nextword (char *buf, char *token, int maxlen);
char            *safestrtok(char *, const char *, char **);
int             parseline (char *s, char *indizes);
char            *get_searchitem (char *instr, char *str, int maxlen);
int             filecopy (char *src, char *dest);
int             xunlink (char *name);
int             xrename (char *src, char *dest);
long            filesize (char *name);
int             isdir (char *name);
time_t          filetime (char *name);
time_t          file_isreg (char *name);
char            *defext (char *name, char *ext);
void            killbackslash (char *s);
void            remove_emptyadd (char *s);
char            jokcmp (char *string, char *jokex, char negotiation);
int             setfiletime (char *filename, unsigned long binfiletime);
//void            filepath(char *name,char *filename);
//file            *sysopen(char *name,char *mode);
//char            *syspath(char *name);

//****************************
//      bids.cpp
//****************************
char            *bidvorhanden(char *bidname);
int             tmpbidvorhanden(char *bidname);
char            *bidnewfilename(char *bidname,char *newfilename);
void            tmpbidrein(char *bidname, int j);
void            tmpbidraus(int j);
unsigned        bidrein(char *bidname,char *filename);
void            bidinit(int begin);
void            listbids(char *sel);
char            *newbid(void);
long            bid_len(void);
void            reorgbid(void);

//****************************
//      dir.cpp
//****************************
int             finddir(char *path,int one_letter_boards);
handle          preparedir(char *selektor,int printerror);
void            mbdir(char *selektor,int listcmd);
char            *make_mask(char *mask,char *path);
time_t          dirline_data(char *dirline);
void            sendmailbake(char *name);
void            board_info(char *board);

//****************************
//      check.cpp
//****************************
void            makecheck(void);
void            mbcheck(char *selektor,time_t abwann);
void            make_checknum(char *from);
//void            make_checknum_board(char *from, char *board);
void            purgecheck(char *name);
//void            purgecheck_board(char *board);
unsigned long   mbcheckcount(void);
char            *checklinetoboard (unsigned long nummer);

//****************************
//      fts.cpp
//****************************
#ifdef FULLTEXTSEARCH
void            mbfts(char *selektor, char *rubrik);
#endif

//****************************
//      send.cpp
//****************************
char            testmaildirectory(void);
int             mbsend(char *befbuf, int changedir);
void            savemail(void);
char            wantmail(char *reason);
char            searchreject(void);
#ifdef DF3VI_REJ_EDIT
void            rejectlistedit(char *befbuf);
#endif
int             parse_headerline(void);
int             mbsend_parse(char *befbuf, int def);
void            writeheader(FILE *sendf, int transfer);
void            writelines();
void            writemailflags();
void            generate_ack(char *to, char *from, char *subj);
void            add_unknown(char *zielboard);
void            save_em();
void            update_mail(void);
void            convat(char *adr);
#ifdef __FLAT__
int             smtp_convert(char *);
char            *create_bin_header(char *);
bool            mime2bin(FILE *, FILE *);
bool            quoted2bin(FILE *, FILE *, char *);
#endif
#ifdef _BCMNET_FWD
int             is_bcmnet_bid(char *bid, int cflag, char *call);
#endif
#ifdef _BCMNET_GW
int             is_bcmnet_user(char *call);
int             is_bcmnet_mybbs(char *call, char *bid, char *bbs, long btime,
                                char *origin, char *w_name, short hops);
#endif
void            set_boardlife_max(short int old_lt); //db1ras
void            sysop_sysinfomail(void);

//****************************
//      transfer.cpp
//****************************
int             mbtransfer(char *ziel);

//****************************
//      read.cpp
//****************************
int             mbread(char *befbuf, int mode);
int             mbchange(char *befbuf, int wasmachen, int kommentar);
int             rmemptydir(char *mask);
int             markerased(char reason, int unerase, int checkerase);
void            marklifetime(int tage);
void            sendbin(FILE *f, char *mailpath); //dg4iad: need current
                                                  // filename for didadit-tx
time_t          get_mybbstime(char *call);

//****************************
//      tree.cpp
//****************************
void            mbreadtree(char *beginpath);
void            mbtreesave(void);
int             rmboard(char *fullname);
int             mkboard(char *mainboard, char *newboard, int sameboard);
int             mvboard(char *old, char *sub, char *newboard);
void            limit_list(void);

//****************************
//     convert.cpp
//****************************
void            mbcvtload(void);
int             convert(char *Name, int *convLT);
#ifdef DF3VI_CONV_EDIT
 void           convedit(char *befbuf);
 void           saveconvnames(void);
#endif

//****************************
//      init.cpp
//****************************
void            mbinit(void);
int             mbinitbef(char *befbuf, int inifile);
int             mbinitdisp(char *befbuf);
void            mbparsave(void);

//****************************
//      users.cpp
//****************************
void            listuser(char *wildcard, int lokal);
int             loaduser(char *usercall, user_t *u, int anlegen);
void            saveuser(user_t *u);
void            userinit(void);
void            defaultuser(char *usercall, user_t *u);
void            reorguser(int delempty);
void            uimport(void);
void            usersuch(char *befbuf);
void            hashinit(handle uhf, long hashsize);
void            us_deletecache(void);

//****************************
//      oldmaili.cpp
//****************************
void            oldmailimport(char *directory);

//****************************
//      fwd.cpp
//****************************
int             isforwardpartner(char *);
char            another_fwd_sender(char *);
fwdpara_t       *fpara(void);
void            fwdlog(char *, char *, char);
void            fwdput(char *, char *);
void            fwdget(char *, int);
int             chkforward(char *);
void            initfwdlist(void);
void            dirfwdlist(char * dest);
void            printfwdlist(char *);
#ifdef DF3VI_FWD_EDIT
void            savefwdlist(char *);
void            fwdlistedit(char *befbuf);
#endif
void            fwdstat(char *);
int             isforwardtime(char *, int);
int             boxbinok(int);
void            add_hold(char *, int);
int             scanheader(FILE *, fwdpara_t *);
char            *board_aus_path(char *);
void            markfwd(void);
void            readmail(FILE *, char);
int             setfullpath(char *);
void            killforwarded(char *);
char            getfwdlistentry(char *, char, char, char **);
void            delfwdlistentry(char *);
void            fwdsend(char *);
int             fwdmainloop(void);
int             weiterleiten(int, char *);
void            add_fwdfile(char *, short unsigned, int);
void            trigger_ufwd(char *);
void            startfwd(char *);
int             waitfwdprompt(char *);

//****************************
//      fwd_file.cpp
//****************************
#ifdef _FILEFWD
int             fwd_import(char *);
int             fwd_export(char *);
#endif

//****************************
//      fwd_rli.cpp
//****************************
int             scan_userfile(char *, int);
void            fwdproc_reply(char);
void            ascii_fwdmachine(int);

//****************************
//      wp.cpp
//****************************
void            add_emfile (char *zeile, char *frombox, char *file_flag,
                            char *boardname, char *at);
int             get_wcs (char *line);
void            addwp_m (wpdata_t *wp);
void            addwp_r (wpdata_t *wp);
void            addwp_e (char *at, char *originbbs, char *bid,
                         char *bid_erase, char *erase_user, short hops,
                         char *reason, char *logincall);
void            wpgenheader (FILE *f);
void            gen_wpmail (char *call);
int             wpparse_fbb (char *buf, wpdata_t *wp);
int             wpparse_v (char *buf,long &protnum);
int             wpparse_b (char *buf, char *hadr, long &bversion,
                           char *bstatus, char *sysop, char *protocol,
                           char *hwaddress, time_t &timestamp, short &hops);
int             wpparse_m (char *buf, wpdata_t *wp);
int             wpparse_e (char *buf, char *at, char *bid, char *originbbs,
                           char *bid_erase, char *origin, short &hops,
                           char *comment);
int             wpparse_r (char *buf, wpdata_t *wp);
void            scan_wp (void);
void            remeraser (char *at, char *bid, char *herkunft,
                           char *bid_erase, char *logincall, char *erase_user,
                           short hops, char *reason);

//****************************
//      login.cpp
//****************************
void            mblogin(char *logincall,loginmode_t lmode,char *uplink);
void            mblogout(int discon_);
void            putlogauszug(char *selektor);
void            putaktuell(int immer);
int             readtext(char *text);

//****************************
//      reorg.cpp
//****************************
void            reorgboard (char *board, int remdouble);
void            mbreorg(char *name);
void            createdirlist(int remdouble);
unsigned long   appenddirlist(int check);
char            *dirsort(char *mask,int users);
void            postfwd(char *);

//****************************
//      alter.cpp
//****************************
int             get_mybbs(char *call,char *bbs,int setmybbs);
void            set_mybbs(char *call,char *bbs,long bime,char *bid,
                             char *origin, short hops,char *w_name);
void            set_name(char *call,char *name);
void            set_qth(char *call,char *qth);
void            set_zip(char *call,char *zip);
char            *get_name(char *call,int par);
char            *get_ttypw(char *call,char *pw);
#ifdef __FLAT__
int             put_ttypw(char *call,char *pw);
int             get_smtp_security(char *call);
char            *get_httppw(char *call, char *pw);
char            *get_pw(char *call, char *pw);
#endif
void            inc_mailgot(char *call);
void            inc_mailsent(char *call);
int             mbalter(char *bef,char *selektor,char *call);
char            *optstr(bitfeld option);
int             get_nopurge(char *call);
int             get_fdelay(char *call);
int             get_readlock(char *call);
char            *get_ufwd(char *call);
void            display_parameter(char *call,int full);
int             get_newcall(char *call,char *newcall);
int             get_usr_local(char *call, time_t &lastlogin);

//****************************
//      sysop.cpp
//****************************
int             sysbef(char *befbuf);
void            sysimport(char *befbuf);
void            testdfull(void);
void            putlog(char *name,char *befbuf);

//****************************
//      grep.cpp
//****************************
int             grep(char *name,char *pattern,bitfeld options);

//****************************
//      main.cpp
//****************************
int             mailbef(char *befbuf,int echo);
void            emptytempdir(void);
void            mbmainbegin(void);
void            mbmain(char *call,char *uplink,char *mycall);
int             checkquota(void);
void            mbmainloop(int doprompt);
int             test_job(job_typ);
#ifdef _WXSTN
 void           wxdaemon(char *name);
#endif

//****************************
//      inout.cpp
//****************************
char            getline(char *s,int maxlen,char cut);
int             janein(char *prompt);
void            putv(int c);
void            bputv(int c);
void            putf(char *format, ...);
int             getv(void);
int             bgetv(void);
int             getvalid(void);
int             putfree(void);
void            putflush(void);
int             testabbruch(void);
#ifdef _TELEPHONE
void            mon(int c,bitfeld opt);
#endif

//****************************
//      interfac.cpp
//****************************
#ifdef __DOS16__
void            mbwin2(char *name);
void            userwin(char *name);
void            taskwin(char *name);
void            doodle(int soundnum);
void            trcwin(char *befbuf);
#else
#define         doodle(x)
#endif
void            mbwindow(char *name);
void            boxusertask(char *usercall);
int             getlogin(char *s,int withpw,char *source);
void            mbimport(char *filename);
void            genmail(char *sendline,char *content);
void            runbatch(char *batch);

//****************************
//      runutils.cpp
//****************************
#ifdef RUNUTILS
void            read_runfile(void);
int             start_runutil(char *befbuf);
void            list_runutils(char *befbuf);
#endif

//****************************
//      help.cpp
//****************************
void            mbinithelp(void);
void            mbreadhelp(char *befehl);

//****************************
//      purge.cpp
//****************************
void            mbpurge(char *procname);
void            userpurge(char *befbuf);

//****************************
//      cfgflex.cpp
//****************************
void            cfgflex(char *zielcall);

//****************************
//      tnc.cpp
//****************************
#ifdef __DOS16__
void            monwindow(char *name);
#endif
#ifndef _AX25K_ONLY
#ifdef __FLAT__
void            putl2info(void);
#endif
void            set_paclen_tnc (unsigned short paclen);
void            putv_tnc(int c);
int             putfree_tnc(void);
void            putflush_tnc(void);
int             getv_tnc(void);
int             getvalid_tnc(void);
void            getclear_tnc(void);
void            inputline_tnc(char *s,int maxlen,char cut);
void            getmycall_tnc(char *outcall);
int             makeconnect(char *mycall,char *conncall);
void            mbdisconnect(int wait);
int             freeblocks_tnc(void);
#ifdef _TNC
 void           control_tnc(char *befehl);
#endif
void            putversion_tnc(void);
void            logindaemon(char *name);
void            setsession(void);
void            putbeacon_tnc(char *adressfeld, char *bakentext);
void            align_mycalls(void);
#else  // _AX25K_ONLY
void            setsession(void);
int             makeconnect(char *mycall,char *conncall);
void            logindaemon(char *name);

  #define putv_tnc(c)           ax25k_putv(c)
  #define putfree_tnc()         ax25k_putfree()
  #define putflush_tnc()        ax25k_flush()
  #define getv_tnc()            ax25k_getv()
  #define getvalid_tnc()        ax25k_getvalid_tnc()
  #define getclear_tnc()        ax25k_getclear()
  #define inputline_tnc(s,maxlen,cut)  ax25k_inputline(s,maxlen,cut)
  #define getmycall_tnc(s)      ax25k_getmycall(s)
  #define mbdisconnect(wait)    ax25k_disconnect(wait)
  #define freeblocks_tnc()      255
#ifdef _TNC
  #define control_tnc(bef)
#endif
  #define putversion_tnc()      extern int l2vorhanden; \
                                (l2vorhanden?putf("Linux Kernel AX25\n"):putf("-\n"))
  #define putbeacon_tnc(addr,text)     ax25k_putbeacon(addr,text)
  #define align_mycalls()
  #define call_l2(x)
#endif

#ifdef __FLAT__

//****************************
//      socket.cpp
//****************************
void            putv_sock(int c);
int             putfree_sock(void);
void            putflush_sock(void);
int             getv_sock(void);
int             getv_sock_ftp(void);
int             getvalid_sock(void);
void            inputline_sock(char *s,int maxlen,char cut);
void            logindaemon_sock(char *name);
void            disconnect_sock(void);
void            show_sock(void);
int             myhostname(char *s, int len);
char            *get_fqdn();
#ifdef _TELNETFWD
int             telnet_fwd_connect(char *addr);
#endif
int             ftp_connect (char *, unsigned int);
enum
{
  _TELNET=-1, _HTTP=-2, _SMTP=-3, _POP=-4,
  _NNTP=-5, _SERV=-6, _RADIO=-7, _FTP=-8
};

//****************************
//      http.cpp
//****************************
void            mblogin_http(char *);
void            html_putf(char *format, ...);
int             isamprnet(char *ip);

//****************************
//      smtp.cpp
//****************************
void            rfc2047_decode (char *, const char *, size_t);
void            mblogin_smtp(char *);
char            *extract_call (char *line, int from);

//****************************
//      pop3.cpp
//****************************
void            mblogin_pop3(char *name);
void            show_popdb(void);

//****************************
//      nntp.cpp
//****************************
#ifdef NNTP
void            mblogin_nntp(char *name);
#endif
//****************************
//      service.cpp
//****************************
void            mblogin_serv(char *name);

//****************************
//      radio.cpp
//****************************
void            mblogin_radio(char *name);

//****************************
//      ftp.cpp
//****************************
void            mblogin_ftp(char *);

#endif //__FLAT__

//****************************
//      flex.cpp
//****************************
#ifdef __MSDOS__
void            f_putv_tnc(int c);
int             f_putfree_tnc(void);
void            f_putflush_tnc(void);
int             f_getv_tnc(void);
int             f_getvalid_tnc(void);
void            f_getclear_tnc(void);
void            f_getmycall_tnc(char *outcall);
void            f_inputline_tnc(char *s, int maxlen, char cut);
int             f_makeconnect(char *mycall, char *conncall);
void            f_mbdisconnect(int wait);
int             f_freeblocks_tnc(void);
void            f_control_tnc(char *befehl);
void            f_putversion_tnc(void);
void            f_logindaemon(char *name);
void            f_monwindow(char *name);
void            f_setsession(void);
void            f_putbeacon_tnc(char *adressfeld, char *bakentext);
void            disable_tnc(void);
void            f_ax25_set_pid(unsigned short int qsonum, char pid);
#endif

//****************************
//      tty.cpp
//****************************
#ifdef FEATURE_SERIAL
void            putv_tty(int ch);
void            putflush_tty(void);
int             putfree_tty(void);
int             getv_tty(void);
int             getvalid_tty(void);
void            inputline_tty(char *s, int maxlen, char cut);
void            init_tty(void);
void            ttysetecho(int echo);
#endif

//****************************
//      telephon.cpp
//****************************
#ifdef _TELEPHONE // JJ
void            putv_tty(int ch);
void            putflush_tty(void);
int             putfree_tty(void);
int             getv_tty(void);
int             getvalid_tty(void);
void            inputline_tty(char *s, int maxlen, char cut);
void            init_tty(void);
void            ttysetecho(int echo);
void            putf_tty(char *format, ...);
int             tty_cmd(char *cmd);
int             tty_waitring(void);
int             tty_hangup(void);
void            tty_kill(void);
int             tty_dial(char *befbuf, int fwdtaskid);
int             tty_autologin(char *s);
void            tty_statustext(void);
void            tty_win(char *befbuf);
void            ttyinit(void);
int             ttyinitbef(char *befbuf, int inifile);
int             ttybef(char *befbuf);
void            ttyparsave(void);
void            tty_counterreset(void);
#endif

//****************************
//      msg.cpp
//****************************
char            *ms(MSG_TYP msgnum);
char            *msg_landsuch(char *call);
void            msg_listspeech(void);
void            msg_dealloc(int all);
int             is_msgkenner(char *kenn);
unsigned        msg_limit(int current);

//****************************
//      timerint.cpp
//****************************
void            set_watchdog(int on);
void            ad_settimer(int on, void (*callback)(void));
void            put_cpuload(void);
int             cpuload(void);
extern int      lastjob;

//****************************
//      hadr.cpp
//****************************
void            show_hadr(char *call);
void            fwdcheck(char *call);
int             update_hadr(char *headerline, int hops, int uplink);
void            reorg_hadr(void);
void            browse_hadr(char *search);
void            showpath(char *adr, int alles);
void            expand_hadr(char *adr, int force);
void            edit_hadr(bitfeld opt, char *line);
long            hadr_len(int max);
int             wpupdate_hadr(wpdata_t *wp);
void            hadrfile_newformat(void);
time_t          find_hadr(char *call);
int             rupdate_hadr(wpdata_t *wp);
void            uclose(void);
void            hadr_tryopen(void);
int             loadhadr(char *hadrcall, hadr_t *u, int anlegen);

//****************************
//      terminal.cpp
//****************************
void            termqso(char *calls);

//****************************
//      convers.cpp
//****************************
int             conversout(int prompt);
void            convers(char *befbuf, char *befname);

//****************************
//      ad_dos.cpp
//      ad_linux.cpp
//      ad_win32.cpp
//****************************
int             oshell(char *befbuf, shellinput_t input);
char            *memfree(int swapfree);
char            *cpuinfo(void);
unsigned long   dfree(char *path, int total);
void            cdhome(void);
void            resetreadonly(char *name);
#ifdef BIOS_Y2K
void            chk_y2k_comp();
#endif
#ifndef _GNU_SOURCE
 #ifdef _WIN32
  #define snprintf _snprintf
 #else
  int           snprintf(char *str, size_t n, const char *format,...);
 #endif
#endif
void            testcputask(void);
void            desktop(char *);

//****************************
//      time.cpp
//****************************
struct tm       *ad_comtime(time_t time);
time_t          ad_mktime(struct tm *tt);
time_t          loctime(void);
time_t          ad_timezone(void);
time_t          ad_time(void);
char            *datestr(time_t utime, int ymode);
int             newdate(time_t d1, time_t d2);
char            *zeitspanne(time_t zeit, delta_t mode);
time_t          parse_time(char *datum);
#ifdef __FLAT__
time_t          getunixtime(unsigned long dosfiletime);
#endif
unsigned long   getdostime(time_t utime);
time_t          yymmdd2ansi(char *yymmdd);

//****************************
//      crontab.cpp
//****************************
void            mbcron(char *);

#ifdef __MSDOS__
//****************************
//      d_video.cpp
//****************************
void            video_open(void);
void            video_close(void);
void            video_putchar(int x, int y, int attr, int c);
void            video_setcursor(unsigned x, unsigned y);
void            video_putc(int x, int y, unsigned attr, unsigned c);
void            video_puts(int x, int y, unsigned attr, char *s);
void            video_update(void);
void            video_checkoff(int force);
void            video_switchon(void);
extern          int cgadisp;
extern          int monodisp;
extern          int novideo;
#endif

extern          int video_off;

//****************************
//      macro.cpp
//****************************
typedef enum mk_job_t
{ MK_NOJOB,
  MK_LOGIN,
  MK_ACCEPT,
  MK_CMD,
  MK_SCMD,
  MK_BEFORECMD,
  MK_AFTERCMD,
  MK_MAILFROM,
  MK_MAILTO,
  MK_UNKNOWN
} mk_job_t;
#ifdef MACRO
int     mk_start(char *name);
void    mk_read_jobs(void);
int     mk_perform(mk_job_t job, char *parameter);
#else
 #define        mk_start(x) 0
 #define        mk_read_jobs()
 #define        mk_perform(x,y) 0
#endif

//****************************
//      ax25k.cpp
//****************************
int     ax25k_putfree();
void    ax25k_putv(int c);
void    ax25k_flush();
void    ax25k_putbeacon(char *addr, char *beacontext);
int     ax25k_isdefault();
int     ax25k_init();
int     ax25k_reinit();
int     ax25k_getvalid_tnc();
int     ax25k_getv();
void    ax25k_getclear();
void    ax25k_getmycall(char *s);
void    ax25k_inputline(char *s, int maxlen, char cut);
int     ax25k_connect(char *mycall, char *conncall);
void    ax25k_disconnect(int wait);
void    ax25k_checklogin();
void    ax25k_shutdown();

#ifdef _USERCOMP
//****************************
//      usercomp.cpp
//****************************
extern    unsigned char DNotCAnz;
extern    unsigned char DNotComp;
extern    unsigned char Dml;
extern    unsigned short DCount;
extern    long Dm;
extern    unsigned char Cml;
extern    unsigned short CCount;
extern    long Cm;
#define maxBufTyp 768

unsigned short int comp_sp_stat_huff(char *src, unsigned short int srclen,
                                     char *dest);
unsigned short int decomp_sp_stat_huff(char *src, unsigned short int srclen,
                                       char *dest);
void ahuf_top_init();
int comp_top_stat_huff(bool flag, char *OBuf, char *zeile, int len);
int decomp_top_stat_huff(char *OBuf, char *zeile, int len);
void UpdateCmComp(unsigned short w, unsigned short & t, unsigned char & bl, long & m, unsigned char & ml, unsigned short & Count, char *OBuf);
int Compress(unsigned char & ml, long & m, unsigned short & Count, char *OBuf, bool flag, char *zeile, int len);
int DeCompress(unsigned char & ml, long & m, unsigned short & Count, char *OBuf, char *zeile, int len);


#endif

//****************************
//      userhuf.cpp
//****************************


//****************************
//      pw.cpp
//****************************
void    genPWstring(char *pw, char *pwstring, char *result);
char    maxpwtype();
char    *pwtypestr(char pwnum);
char    pwtypenum(char *pwstr);
int     getpw(void);
int     pwauswert(char *pw);
int     getpriv(char *befbuf,char *privname);

//****************************
//      mdpw.cpp
//****************************
#ifdef FEATURE_MDPW
void    calcMD2pw(char *MDstring, char *pw, char *MD2result);
void    calcMD5pw(char *MDstring, char *pw, char *MD5result);
void    genMDstring(char MDtype, char *pw, char *pwstring, char *result);
char    getMDpw(char MDtype, char *pw);
void    getMDsysoppw(char MDtype);
#endif

#ifdef _AUTOFWD
//****************************
//      fwd_afwd.cpp
//****************************
int     autofwd(char *call, int ausgabe);
void    initafwdlist();
void    afwdlist(char *);
#endif

#ifdef _MORSED
//****************************
//      morse.cpp
//****************************
void    morsedaemon(char *tname);
void    morse_char(char x);
void    morse_word(char *text);
void    morse_line(char *line);
#endif

//****************************
//      charset.cpp
//****************************
void    init_charsets();

//****************************
//      tell.cpp
//****************************
void    send_tell(char *befbuf);
void    tellresp(char *name);

//****************************
//      ping.cpp
//****************************
void    send_ping(char *befbuf);
void    pingresp(char *name);

//****************************
//      extract.cpp
//****************************
#ifdef DF3VI_EXTRACT
void    extract(unsigned pos);
#endif
//****************************
//      end of file
//****************************
