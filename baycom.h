/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  --------------------------------------------------
  Definitionen und Deklarationen fuer BayCom-Projekt
  --------------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19981011 OE3DZW added wx-code
//19990215 OE3DZW clean up
//19991028 Jan    clean up / moved features selection to config.h
//20000502 DK2UI  config.h after lib-includes and before bcm-includes
//                so all defines from baycom.h can be used in config.h
//                and all defines from config.h are valid only in bcm

#ifndef _BAYCOM_H
  #define   _BAYCOM_H

  #define YEAR "2013"
  #define Year 1900     // 1900 + 100 = 2000

  #ifdef __DPMI32__
    #define __MSDOS__
  #endif

  #ifdef _WIN32
    #define __MSDOS__
    #define __FLAT__
	#define _USE_32BIT_TIME_T
	#define _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_SECURE_NO_WARNINGS
  #endif

  #if defined(__MSDOS__) && (!defined(_WIN32))
    #define __HWSCREEN__
    #define __DOS16__
  #endif

  #ifndef __MSDOS__
    #define __LINUX__
    #define __UNIX__
    #define __FLAT__
  #endif

  #include <ctype.h>
  #include <fcntl.h>
  #include <stdarg.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/stat.h>
  #include <time.h>

  #ifndef errno // DH3MB, for compatibility with glibc
    #include <errno.h>
  #endif

  #ifdef _WIN32
    #include <setjmpex.h>
  #else
    #include <setjmp.h>
  #endif

  #define WITHNUDL     0

  typedef unsigned char byte;
  typedef int handle;

  // to make the WIN32/DOS-Compiler happy...
  #ifndef __LINUX__
    #define strcasecmp stricmp
    #define strncasecmp strnicmp
  #endif

//
//  Fixe Konstanten, die nie geaendert werden
//
 #define REMLEN 80       // Laenge des Remote-Buffers
 #define FALSE  0
 #define NO     0
 #define NEIN   0
 #define AUS    0
 #define ERROR  0
 #define SAME   0
 #define TRUE   1
 #define OK     1
 #define YES    1
 #define JA     1
 #define EIN    1
 #define LOWER  1
 #define HIGHER 2
 #define JEIN   -1
 #define NIL    (-1)
 #define NOTFOUND (-1)
 #define NIX    4711

#define CTRLF   6
#define BACKSPACE 8
#define TAB     9
#define CTRLK   11
#define CTRLL   12
#define CTRLN   14
#define CTRLQ   17
#define LINEDEL 25
#define INSERT  82
#define K_DELETE 83
#define LINKS   75
#define RECHTS  77
#define POS1    71
#define ENDE    79
#define RUNTER  80
#define RAUF    72
#define PG_RUNTER       81
#define PG_RAUF         73
#define CTL_PG_RAUF     132
#define CTL_PG_RUNTER   118

#define CR          '\r'
#define LF          '\n'
#ifdef __UNIX__
#define NEWLINE         "\n"
#else
#define NEWLINE         "\r\n"
#endif

#define LINLEN          142
#define EBENEN          255
#define MAXWND          50

/*---------------------------------------------------------------------------*/
  #include "config.h"
/*---------------------------------------------------------------------------*/

  #ifdef __MSDOS__
    #include "ad_win32.h"
    #include "ad_dos.h"
    #include "color.h"
    #include "d_window.h"
  #endif

  #ifdef __LINUX__
    #include "ad_linux.h"
  #endif

  #ifdef L2COMPILE
    #include "l2_appl.h"
    #include "l2host.h"
    #include "ax_util.h"
    #include "layer2.h"
    #include "layer1.h"
  #endif

  #include "mail.h"
  #include "crc.h"
  #include "fbb.h"
  #include "huffman.h"
  #include "filesurf.h"
  #include "mailserv.h"
#ifdef DF3VI_POCSAG
  #include "pocsag.h"
#endif
  #include "fileio.h"
  #include "yapp.h"
  #include "didadit.h"
  #include "binsplit.h"
  #include "grep.h"
  #include "pop3.h"
  #include "md2md5.h"
  #ifdef _WXSTN
    #include "wx.h"
  #endif
#endif // _BAYCOM_H

/*---------------------------------------------------------------------------*/
