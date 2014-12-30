/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------
  Desktop-Hauptprogramm
  ---------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//19980215 OE3DZW tracelevel always 0 at startup
//19980923 OE3DZW added mkdir fwd-path

#include "baycom.h"

/*---------------------------------------------------------------------------*/

unsigned long cpuindex = 0;

#ifdef __FLAT__

struct   edstruct *e;
char     *buf;
int      boxopen = 0;
unsigned bildzeilen = 25;
unsigned bildspalten = 80;
int      wa = 0;
int      video_off = 1;

/*---------------------------------------------------------------------------*/

static void int_handler (int)
{
  runterfahren = 1;
  strcpy(stopreason, "SIGINT");
}

/*---------------------------------------------------------------------------*/

static void term_handler (int)
{
  runterfahren = 1;
  strcpy(stopreason, "SIGTERM");
}

/*---------------------------------------------------------------------------*/

static void ill_handler (int)
{
  post_mortem_dump("SIGILL");
  #ifdef __UNIX__
  kill(getpid(), SIGILL);
  #else
  abort();
  #endif
}

/*---------------------------------------------------------------------------*/

static void segv_handler (int)
{
  #ifdef __UNIX__
    post_mortem_dump("SIGSEGV");
    signal(SIGSEGV, SIG_DFL); //dg2fef
    raise(SIGSEGV);
    //kill(getpid(), SIGSEGV);
  #else
    abort();
  #endif
}

#else

/*---------------------------------------------------------------------------*/

unsigned alt_taste(unsigned scan)
//**********************************************************************
//
//  wird mit dem Bioscode einer Taste aufgerufen
//
//**********************************************************************
{
  static char tasttab[52]=
  { '1','2','3','4','5','6','7','8','9','0',0,0,0,0,0,0,
    'Q','W','E','R','T','Y','U','I','O','P',0,0,0,0,
    'A','S','D','F','G','H','J','K','L',0,0,0,0,0,
    'Z','X','C','V','B','N','M'
  };

  if (scan & 255)
    return 0;
  else
    scan>>=8;

  if (scan < 16)
    return 0;

  if ((scan>=104) && (scan<=113))  // Funktionstasten ALT-F1 .. ALT-F10
    scan-=104;
  if ((scan>=120) && (scan<=129))  // Tasten 1 .. 0
    scan-=120;

  if (scan<51)                   // Buchstaben-Code ?
    return tasttab[scan];
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

unsigned f_taste(unsigned scan)
//**********************************************************************
//
//  wertet einen Scancode aus, und gibt zurueck, wenn es eine Funktions-
//  Taste war.
//
//    Returnwerte:
//    0      keine Funktionstaste
//    1 -10     F1 -       F10
//    11-20  SHIFT F1 - SHIFT F10
//    21-30  CTRL  F1 - CTRL  F10
//    31-40  ALT   F1 - ALT   F10
//
//**********************************************************************
{
  if (scan&255)
    return 0;
  else
    scan>>=8;

  if (scan<59)
    return 0;

  if (scan<69)
    return scan-58;
  if ((scan>83) && (scan<=113))
    return scan-73;
  return 0;
}

/*---------------------------------------------------------------------------*/

static int desktopupdate (int full)
{
  if (full)
    wclear(color.desktop);
  return 1;
}

#endif // __FLAT__

/*---------------------------------------------------------------------------*/

void desktop (char *)
{
  lastfunc("desktop");
#ifndef __FLAT__
  char instr[50];
  char *msg[12] = { "",
                     "                 OpenBCM Packet-Radio Mailbox",
                     "  / \\",
                     " /   \\                 Version "VNUMMER,
                     " \\   /",
                     "  \\ /  ",
                     " __ __",
                     "                 (c) GNU Copyright 1992-"YEAR,
                     "                 Florian Radlherr DL8MBT et al.",
                     "                 Copy Policy is GNU GPL",
                     NULL
                   };
#endif
  mkdir(LOGPATH);
  mkdir(TRACEPATH);
  mkdir(TEMPPATH);
  mkdir(FWDPATH);   // OE3DZW
  mkdir(FWDEXPATH); // DH8YMB
  mkdir(FWDIMPATH); // DH8YMB
  mkdir(MSGPATH);
  mkdir(FBBPATH);   // DH3MB
  mkdir(WLOGPATH);  // DH8YMB
  m.tracelevel = 0;
#ifdef __FLAT__
  mkdir(HTTPPATH);  // DH8YMB
#ifndef _DEBUG
  signal(SIGTERM, term_handler);
  signal(SIGINT, int_handler);
  signal(SIGSEGV, segv_handler);
  signal(SIGILL, ill_handler);
#endif
#ifdef __LINUX__
  //ignore sigpipe
  signal(SIGPIPE, SIG_IGN);
#endif
  fork(P_WIND, 0, tracewindow, "Console");
 // fork(P_WIND, 0, monwindow, "Monitor");
  fork(P_MAIL | P_WIND | P_KEYB, 0, mbwindow, "Sysop");

  while (! runterfahren)
  {
    wdelay(146);
    desk_cyclic();
  }
#else
  e->x = 0;
  e->xend = bildspalten - 1;
  e->y = 0;
  e->yend = bildzeilen - 1;
  e->update = desktopupdate;
  wopen();
  wclear(color.desktop);
  fork(P_WIND, 0, monwindow, "Monitor");
  fork(P_WIND, 0, tracewindow, "Console");
  fork(P_MAIL | P_WIND | P_KEYB, 0, mbwindow, "Sysop");
  testcputask();
  deskkey = 1;
  message("About", msg, 6);
  deskkey = 0;
  while (! runterfahren)
  {
    wdelay(146);
    if (deskkey)
    {
      switch (deskkey)
      {
        case 23:
          *instr = 0;
          if (inputwin("Login", "Call [Boxcall]:", instr, 20) != ESC)
          {
            strupr(instr);
            fork(P_WIND | P_KEYB | P_MAIL, 0, mbwin2, instr);
          }
          break;
#ifdef FEATURE_EDITOR
        case 24:
          *instr = 0;
          if (inputwin("Editor", "Filename:", instr, 20) != ESC)
          {
            strupr(instr);
            fork(P_WIND | P_KEYB, 0, editfile, instr);
          }
          break;
#endif
        case 'Z':
        case 25: resize(keywindow(), 0); break;
        case 'N':
        case 26: newkeywin = keywindow() + 1; break;
#ifdef FEATURE_COLOR
        case 'C': colselect(); break;
#endif
        case 'R':
        case 27: resize(keywindow(), 1); break;
        case 33:
          if (yesno("Close window?", 1) == YES) killtask(keytaskid, 0);
          break;
        case 'S': video_checkoff(1); break;
#ifdef __MSDOS__
        case 35:
          set_watchdog(2);
          video_close();
          waitdoskey();
          video_open();
          set_watchdog(1);
          newkeywin = keywindow();
          break;
#endif
        case 'X':
          if (yesno("Mailbox shutdown?", 1) == YES)
          {
            runterfahren = 1;
            strcpy(stopreason, "alt-x pressed");
          }
          break;
        case '0':
        {
          sprintf(instr, "%d", keywindow());
          if (inputwin("Window", "new window:", instr, 20) != ESC)
          {
            if (atoi(instr))
              newkeywin = atoi(instr);
          }
        }
        break;
      }
      deskkey = 0;
    }
    video_checkoff(0);
    desk_cyclic();
  }
#endif
}
