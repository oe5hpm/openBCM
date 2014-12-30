/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------
  Timer-Interrupt
  ---------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//990404 OE3DZW added minwatch - find blocking code

#include "baycom.h"
#ifdef _WIN32
  #include "windows.h"
#endif

#define CPUVALUES 8

/*---------------------------------------------------------------------------*/

long systic = 0;
int lastjob; // is written by showtasknum in tasking.cpp

/*---------------------------------------------------------------------------*/

static char idletics[CPUVALUES] = "";
static char updatics[CPUVALUES] = "";
static char runtics[CPUVALUES] = "";
static int t_index = 0;
static unsigned int watchdog = 1000;
static unsigned int minwatch = 0; //min watchdog
static char minwcmd[60] = ""; //command with min. watchdog timer

/*---------------------------------------------------------------------------*/

void mbtimer (void)
//*************************************************************************
//
// Callback from ad_settimer
//
//*************************************************************************
{
  static char l_idletics = 0;
  static char l_updatics = 0;
  static char l_runtics  = 0;
  static char l_tics     = 0;

  systic++;
  switch (lastjob)
  {
    case 'D':
    case 'I': l_idletics++; break;
    case 'N':
    case 'R': l_runtics++;  break;
    case 'M':
    case 'U': l_updatics++; break;
  }
  l_tics++;
  if (l_tics >= 100)
  {
    idletics[t_index] = l_idletics;
    updatics[t_index] = l_updatics;
    runtics[t_index] = l_runtics;
    l_tics =
    l_idletics =
    l_updatics =
    l_runtics = 0;
    t_index = (t_index + 1) % CPUVALUES;
  }
  watchdog--;     // watchdog was set to a positive value
  if (! watchdog) // now it has expired
  {
    if (m.watchdog & 2) // is sw-watchdog enabled?
    {
      if (m.watchdog & 4)
        post_mortem_dump("watchdog"); // is post mortem dump enabled?
      disable(); // be careful...
      coldstart(); // hope this still runs...
    }
  }
  if (t && (! minwatch || watchdog < minwatch))
  {
    minwatch = watchdog;
    strncpy(minwcmd, t->lastcmd, sizeof(minwcmd) - 1);
    minwcmd[sizeof(minwcmd) - 1] = 0; //add terminating 0-byte
  }
}

/*---------------------------------------------------------------------------*/

int cpuload (void)
//*************************************************************************
//
//*************************************************************************
{
  int ret = 0;

  for (unsigned int i = 0; i < CPUVALUES; i++)
  {
    ret += runtics[i];
    ret += updatics[i];
  }
  return ret / CPUVALUES;
}

/*---------------------------------------------------------------------------*/

static void near load_line (char *val, char *nam)
//*************************************************************************
//
//*************************************************************************
{
  int i = t_index, x = t_index;
  putf("%s ", nam);
  do
  {
    i--;
    if (i < 0) i = CPUVALUES - 1;
    putf("%3d ", val[i]);
  } while (i != x);
  putf("%\n");
}

/*---------------------------------------------------------------------------*/

void put_cpuload (void)
//*************************************************************************
//
//*************************************************************************
{
  putf(ms(m_cpuload));
  putf("Sample:  now -5s -10s ...\n");
  load_line(runtics, "Running:");
  load_line(updatics, "Screen: ");
  load_line(idletics, "Idle:   ");
  putf("Max. tics: %u at \"%s\"\n", 5460 - minwatch, minwcmd);
}

/*---------------------------------------------------------------------------*/

static void clear_watchdog (void)
//*************************************************************************
//
//*************************************************************************
{
  set_watchdog(0);
}

/*---------------------------------------------------------------------------*/

void set_watchdog (int on)
//*************************************************************************
//
//*************************************************************************
{
  static int first = 1;

  if (on && first)
  {
    atexit(clear_watchdog);
    first = 0;
  }
  if (on == 2) watchdog = 32760; // 30min for DOS calls etc
  else watchdog = 5460;          // 2min for normal operation
  ad_settimer(on, mbtimer);
}

/*---------------------------------------------------------------------------*/

#ifdef __DOS16__
static void interrupt far(far*oldvec)(...);
#endif

static void (*timer_callback)(void);

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
void CALLBACK win_handler(UINT IDEvent,UINT uReserved,
                 DWORD  dwUser, DWORD  dwReserved1, DWORD  dwReserved2)
#else
#ifdef __UNIX__
static void timer_handler (int)
#else
static void interrupt far timer_handler (...)
#endif
#endif
//*************************************************************************
//
//*************************************************************************
{
  static char recursive = 0;
#ifdef __DOS16__
  (oldvec)();
  disable();
#endif
  if (! recursive)
  {
    recursive = 1;
    enable();
    (*timer_callback)();
    recursive = 0;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef __DPMI32__
#define PUSH_EBP __emit__(0x55)
#define PUSH_EAX __emit__(0x50)
#define PUSH_EBX __emit__(0x53)
#define PUSH_ECX __emit__(0x51)
#define PUSH_EDX __emit__(0x52)
#define PUSH_ESI __emit__(0x56)
#define PUSH_EDI __emit__(0x57)
#define PUSH_DS  __emit__(0x1E)
#define PUSH_ES  __emit__(0x06)

#define POP_ES   __emit__(0x07)
#define POP_DS   __emit__(0x1F)
#define POP_EDI  __emit__(0x5F)
#define POP_ESI  __emit__(0x5E)
#define POP_EDX  __emit__(0x5A)
#define POP_ECX  __emit__(0x59)
#define POP_EBX  __emit__(0x5B)
#define POP_EAX  __emit__(0x58)
#define POP_EBP  __emit__(0x5D)
#define IRETD    __emit__(0xCF)
#define NOP      __emit__(0x90)

short unsigned dpmi32_ds;
void timer_dpmi32(void)
//*************************************************************************
//
//*************************************************************************
{
  PUSH_EBP;
  PUSH_EAX;
  PUSH_EBX;
  PUSH_ECX;
  PUSH_EDX;
  PUSH_ESI;
  PUSH_EDI;
  PUSH_DS;
  PUSH_ES;
  _AX=_CS;
  _DS=_AX;
  _AX=dpmi32_ds;
  _DS=_AX;
  timer_handler();
  POP_ES;
  POP_DS;
  POP_EDI;
  POP_ESI;
  POP_EDX;
  POP_ECX;
  POP_EBX;
  POP_EAX;
  POP_EBP;
  IRETD;
}
#endif

/*---------------------------------------------------------------------------*/

void ad_settimer (int on, void (*callback) (void))
//*************************************************************************
//
//*************************************************************************
{
  timer_callback = callback;
#ifdef _WIN32
  static UINT ev_id;
  static int ison = 0;
  if (on)
  {
    if (! ison)
    {
      ison = 1;
      ev_id = timeSetEvent(55, 5, (LPTIMECALLBACK) win_handler, 0x0,
                           TIME_PERIODIC);
    }
  }
  else if (ison)
  {
    timeKillEvent(ev_id);
    ison = 0;
  }
#elif defined(__DPMI32__)
  static unsigned long oldisr;
  static unsigned long oldiseg;
  static int ison = 0;

  if (on)
  {
    if (! ison)
    {
      ison = 1;
      _AX = _DS;
      dpmi32_ds = _AX;
      _EBX = 0x1c;
      _EAX = 0x204;
      geninterrupt(0x31);
      oldiseg = _ECX;
      oldisr = _EDX;
      _EBX = 0x1c;
      _CX = _CS;
      _EDX = (long) timer_dpmi32;
      _EAX = 0x205;
      geninterrupt(0x31);
    }
  }
  else
  if (ison)
  {
    _EBX = 0x1c;
    _ECX = oldiseg;
    _EDX = oldisr;
    _EAX = 0x205;
    geninterrupt(0x31);
    ison = 0;
  }
#elif defined(__MSDOS__)
  static int ison = 0;
  if (on)
  {
    if (! ison)
    {
      ison = 1;
      oldvec = getvect(0x1c);
      setvect(0x1c, timer_handler);
    }
  }
  else
  if (ison)
  {
    setvect(0x1c, oldvec);
    ison = 0;
  }
#else
  static int first = 1;
  if (first && on)
  {
    static struct itimerval rttimer;
    static struct itimerval old_rttimer;
    struct sigaction sa;

    first = 0;
    memset(&rttimer, 0, sizeof(struct itimerval));
    rttimer.it_value.tv_sec     = 0;
    rttimer.it_value.tv_usec    = MS_PER_TIC * 1000;
    rttimer.it_interval.tv_sec  = 0;
    rttimer.it_interval.tv_usec = MS_PER_TIC * 1000;
    sa.sa_handler = timer_handler;
    sa.sa_flags = SA_RESTART | SA_NOMASK;
    sigaction(SIGALRM, &sa, NULL);
    setitimer(ITIMER_REAL, &rttimer, &old_rttimer);
  }
#endif
}

/*---------------------------------------------------------------------------*/

