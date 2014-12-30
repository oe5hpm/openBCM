/***************************************************************

  OpenBayCom-Mailbox

  ----------------------------
  Compile-time features config
  ----------------------------

 ***************************************************************/

// edit this file for wanted or not wanted features


/*---------------------------------------------------------------------------*/
//#define ALPHA            // This is an alpha version
/*---------------------------------------------------------------------------*/

#define SMALL_DOS          // Undefines some features for a small DOS version
                           // (have a look at end of this file!)

/*---------------------------------------------------------------------------*/
// general options

#define RUNUTILS           // Enable the runutility interface
#define MAILSERVER         // Enable the mailinglist server
#define FILESURF           // Enable the filesurf server
#define DF3VI_POCSAG       // PocSag-Message-Server
#define FEATURE_YAPP       // Enable the YAPP file transfer protocol
#define FEATURE_MDPW       // Support for MD2 and MD5 passwords
#define DF3VI_FWD_EDIT     // Editor for fwd.bcm (sysop)
#define DF3VI_REJ_EDIT     // Editor for reject.bcm (sysop)
#define DF3VI_CONV_EDIT    // Editor for convert.bcm (sysop)
#define DF3VI_EXTRACT      // Extract 7+/bin (sysop)
#define USERLT             // Lifetime from sender visible
#define _AUTOFWD           // Automatic fwd-routing
#define _GUEST             // Enable guest-callsign (for telnet/TTY access)
#define _FILEFWD           // Enable file-forwarding
#define FBBCHECKREAD       // Enable FBB-Style CHECK/READ Mode
#define FULLTEXTSEARCH     // Enable full text search (FTS) command
#define MACRO              // Enable macro interpreter support
#define FEATURE_DIDADIT    // Enable the DIDADIT file transfer protocol
//#define _LCF             // Sep. callformat for login and forward
//#define _BCMNET          // CB-BCMNET features, only useful for Citizen Band

#ifdef _BCMNET
  #undef SMALL_DOS
  #define _BCMNET_LOGIN    // Enable the CB-BCMNET Login-Concept
  #define _BCMNET_FWD      // Forward only with CB-BCMNET mailboxes possible
  #define _BCMNET_GW       // Enable the Gateway Functions for CB-BCMNET
//#define _BCMNET_DEBUG    // Some CB-BCMNET Debug Options
  #ifdef _BCMNET_GW        // CB-BCMNET dependencies
    #define _BCMNET_FWD
  #endif
#endif

/*---------------------------------------------------------------------------*/
// Beta options, see "changes" and sources for more info

// not ready yet:
//#define FEATURE_BINSPLIT // Enable BIN split capability
//#define _MORSED          // CW-broadcast
//#define BCAST            // Mailbox-Broadcast nach SAT-Verfahren
#ifdef __LINUX__
  //#define _USERCOMP           // COMP nach Huffman, DO NOT USE!!
#endif

/*---------------------------------------------------------------------------*/
// Debug options
//#define HB9EAS_DEBUG     // for active routing testing with dpbox hb9eas
//#define DEBUG_AFWD       // Write many infos for Autofwd debug to syslog
//#define DEBUG_FWD        // Write many infos for Forward debug to syslog
//#define DEBUG_DIDADIT    // Write many infos for DIDADIT debug to syslog
//#define DEBUG_FTP        // Write many infos for FTP debug to syslog
//#define DEBUG_HTTP       // Write many infos for HTTP debug to syslog
//#define DEBUG_POP3       // Write many infos for POP3 debug to syslog
//#define DEBUG_NNTP       // Write many infos for NNTP debug to syslog
//#define DEBUG_SMTP       // Write many infos for SMTP debug to syslog
//#define _DEBUG_SEMA      // add_fwdfile NO_SEMA debug, see changes

/*---------------------------------------------------------------------------*/
// Local options

//#define _WXSTN           // WX station Peetbros, Ulitimiter 2000
//#define DIEBOX_UIMPORT   // Import user database from DieBox files user3.dat
                           // and user3.idx (normally not needed)
#define _USERS4CONVERT     // Convert user database old format
                           // users.bcm/users3.bcm -> users4.bcm
                           // (only necessary if users4.bcm is not existing
                           // and old user database should be converted)
//#define OLDTIMEFMT       // Use old time format in log files
#define OLDMAILIMPORT      // Import mail files from another Baybox

/*---------------------------------------------------------------------------*/
// Linux and Win32 options

#ifdef __FLAT__
  #define NNTP             // Enable NNTP support
  #define SERVIF           // Enable the telnet service interface
  #define _TELNETFWD       // Enable telnet forward
  #define INETMAILGATE     // Internet-Mailgate (JJ), nur fuer CB-Funk!
#endif

/*---------------------------------------------------------------------------*/
// Linux options

#ifdef __LINUX__
  #define FEATURE_SERIAL   // Enable serial TTY-Login
  #define RADIOIF          // Enable radio interface (Net-CMD TNT/Wampes)
  #define LINUXSYSTEMUSER  // Enable ALTER LINUXPW for create system user
  //#define oldpty         // use old obsolet pseudo-tty/pty for oshell
  #ifndef __sparc__        // No AX25-Kernel under Sparc Processor
    #define _AX25K         // Support for Kernel AX25
    //#define _AX25K_ONLY  // without internal L2 support (use makefile.nol2)
  #endif
#endif

/*---------------------------------------------------------------------------*/
// DOS options

#ifdef __DOS16__
  #define _TNC             // TNC/FlexNet for DOS
  #define _TELEPHONE       // Enable DOS Telephone-Forward
//#define _AUTOTRCWIN      // (Auto) trace window
//#define FEATURE_EDITOR   // Enable internal editor
//#define FEATURE_COLOR    // Enable color selection (ALT-C)
//#define FEATURE_MOUSE    // Enable mouse support (add MOUSE.ASM to PRJ)
//#define BIOS_Y2K         // Fixes problems with buggy y2k-bioses
//#define OLD_SHELL        // Execute command.com twice (needless)
  #define _PREPOSTEXEC     // External DOS watchdog (dospreexec, dospostexec)
  #ifdef _BCMNET
  // Special features for CB-BCMNET version
    #define FEATURE_EDITOR // Enable internal editor
    #undef MAILSERVER      // Disable the mailinglist server
    #undef FILESURF        // Disable the filesurf server
    #undef _TELEPHONE      // Disable DOS Telephone-Forward
    #undef DF3VI_POCSAG    // Disable POCSAG Feature
    #undef DF3VI_EXTRACT   // Disable extract 7+/bin (sysop)
    #undef FEATURE_YAPP    // Disable the YAPP file transfer protocol
    #undef _GUEST          // Disable guest-callsign (for telnet/TTY access)
    #undef DF3VI_FWD_EDIT  // Disable editor for fwd.bcm (sysop)
    #undef DF3VI_REJ_EDIT  // Disable editor for reject.bcm (sysop)
    #undef DF3VI_CONV_EDIT // Editor for convert.bcm (sysop)
    #undef FBBCHECKREAD    // Disable FBB-Style CHECK/READ Mode
    #undef FULLTEXTSEARCH  // Disable full text search (FTS) command
    #undef MACRO           // Disable macro interpreter support
    #undef FEATURE_DIDADIT // Disable the DIDADIT file transfer protocol
    #undef OLDMAILIMPORT   // Disable import mail files from another Baybox
  #endif
  #ifdef SMALL_DOS
  // Disable here unwanted features for a small DOS version
  //#undef RUNUTILS        // Disable the runutility interface
    #undef MAILSERVER      // Disable the mailinglist server
    #undef FILESURF        // Disable the filesurf server
    #undef _TELEPHONE      // Disable DOS Telephone-Forward
    #undef _PREPOSTEXEC    // External DOS watchdog (dospreexec, dospostexec)
    #undef DF3VI_POCSAG    // Disable POCSAG Feature
    #undef DF3VI_EXTRACT   // Disable extract 7+/bin (sysop)
  //#undef FEATURE_YAPP    // Disable the YAPP file transfer protocol
  //#undef FEATURE_MDPW    // Disable support for MD2 and MD5 passwords
    #undef _AUTOFWD        // Disable automatic fwd-routing
    #undef _GUEST          // Disable guest-callsign (for telnet/TTY access)
    #undef _FILEFWD        // Disable file-forwarding
    #undef DF3VI_FWD_EDIT  // Disable editor for fwd.bcm (sysop)
    #undef DF3VI_REJ_EDIT  // Disable editor for reject.bcm (sysop)
    #undef DF3VI_CONV_EDIT // Disable Editor for convert.bcm (sysop)
  //#undef FBBCHECKREAD    // Disable FBB-Style CHECK/READ Mode
    #undef FULLTEXTSEARCH  // Disable full text search (FTS) command
    #undef MACRO           // Disable macro interpreter support
    #undef FEATURE_DIDADIT // Disable the DIDADIT file transfer protocol
    #undef OLDMAILIMPORT   // Disable import mail files from another Baybox
  #endif
#endif

/*---------------------------------------------------------------------------*/
