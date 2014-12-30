/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------
  Video- und Fileinitialisierung
  ------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

#include "baycom.h"

#ifndef __FLAT__

int crit_error_device = 0;
int crit_error_num = 0;

/*---------------------------------------------------------------------------*/

static int break_handler (void)
{
  return 2;
}

/*---------------------------------------------------------------------------*/

void test_critical (void)
{
  if (crit_error_device || crit_error_num)
  {
    trace(serious, "critical_error", "device=%d errno=%d",
                                     crit_error_device, crit_error_num);
    crit_error_device = crit_error_num = 0;
  }
}

/*---------------------------------------------------------------------------*/

#ifndef __DPMI32__
#pragma argsused
static int crit_handler (int errval, int ax, int bp, int si)
{
  unsigned di = _DI;
  if (ax > 0) crit_error_device = ax & 0x00ff;
  else crit_error_device = (-1);
  crit_error_num = di & 0x00FF;
  hardretn(2);
  return 2;
}
#endif

/*---------------------------------------------------------------------------*/

static void near putfiles (void)
{
  FILE *f[100];
  char s[20];
  int i;
#undef fopen
#undef fclose
  for (i = 0; i < 100; i++)
  {
    sprintf(s, "$$%d.$$$", i);
    f[i] = fopen(s, "wb");
    if (! f[i]) break;
  }
  printf("FILES=%d\n", i--);
  for (; i >= 0; i--)
  {
    fclose(f[i]);
    sprintf(s, "$$%d.$$$", i);
    unlink(s);
  }
}

/*---------------------------------------------------------------------------*/

#ifdef __DPMI32__
static void create_swap (int kbytes)
{
  char *page = (char *) malloc(4096);
  int i;

  kbytes /= 4;
  if (page)
  {
    FILE *f = fopen("bcm32.swp", "wb");
    if (f)
    {
      memset(page, 0, 4096);
      setvbuf(f, NULL, _IOFBF, 16384);
      for (i = 0; i < kbytes; i++) fwrite(page, 1, 4096, f);
      printf("Swap file BCM32.SWP (%d kBytes, %d pages) generated.\n",
             kbytes * 4, kbytes);
    }
    else printf("create_swap: can't open swapfile.\n");
  }
  else printf("create_swap: can't allocate buffer.\n");
}
#endif

/*---------------------------------------------------------------------------*/

static void getparm (int argc, char *argv[])
{
  puts("OpenBCM Packet-Radio Mailbox V"VNUMMER"  \n(C) GNU "YEAR
       " F. Radlherr, DL8MBT et al.");
  while (argc > 1)
  {
    switch (toupper(argv[argc - 1][1]))
    {
      case 'C':
        cgadisp = 1;  break;
      case 'M':
        monodisp = 1; break;
      case 'N':
        novideo = 1;  break;
      case 'F':
        putfiles(); exit(0);
#ifdef __DPMI32__
      case 'S':
        create_swap (atoi(argv[argc - 1] + 2)); exit(0);
#endif
      default:
        printf("\n%s [/C] [/M] [/F]\n", argv[0]);
        puts(" /C  use only 25 display lines (no VGA screen)\n"
             " /M  use monochrome color set (for LCD display)\n"
             " /N  do not write into video memory\n"
             " /F  shows maximum of accessible file handles\n"
#ifdef __DPMI32__
             " /S<kBytes>  creates BCM32.SWP swap file for DPMI32 Server\n\n"
#endif
             "Note: FLEXNET.EXE must be resident for packet-radio usage.\n");
        exit(0);
    }
    argc--;
  }
}

/*---------------------------------------------------------------------------*/

static void main_close (void)
{
  video_close();
  if (*abortreason)
    printf("\a\n>>> unrecoverable error condition:\n    %s\n", abortreason);
}

/*---------------------------------------------------------------------------*/

void exelock (char *argv0)
{
  if (sopen(argv0, O_RDONLY, SH_DENYRD, S_IREAD) == -1)
    trace(tr_abbruch, "exelock", "OpenBCM is already running");
}


#else

/*---------------------------------------------------------------------------*/

static void abortmsg (void)
{
  if (*abortreason)
    printf("\a\n>>> unrecoverable error condition:\n    %s\n", abortreason);
}

/*---------------------------------------------------------------------------*/

static void getparm (int argc, char *argv[])
{
  printf("\n" LOGINSIGN AUTHOR_ID);
/*
  while (argc>1)
  {
    if (strlen(argv[argc-1])>1)
    {
      switch (toupper(argv[argc-1][1]))
      {
//oe3dzw: Upper case for parameters!!
        case 'T': m.tcp_port = atoi(argv[argc-1]+2);  break;
        case 'H': m.http_port = atoi(argv[argc-1]+2);  break;
        case 'S': m.smtp_port = atoi(argv[argc-1]+2);  break;
        case 'P': m.pop3_port = atoi(argv[argc-1]+2);  break;
        case 'N': m.nntp_port = atoi(argv[argc-1]+2);  break;
        case 'M': m.serv_port = atoi(argv[argc-1]+2);  break;

        default:
          printf("\n%s [-p <num>] [-h <num>]\n", argv[0]);
          puts("  -t [<portnum>] set tcp-port for login (default 4719)\n"
               "  -t (without port) disables tcp\n");
          puts("  -h [<portnum>] set tcp-port for http (default 8080)\n"
               "  -h (without port) disables http\n");
          puts("  -s [<portnum>] set tcp-port for smtp (default 8025)\n"
               "  -s (without port) disables smtp\n");
          puts("  -p [<portnum>] set tcp-port for pop3 (default 8110)\n"
               "  -p (without port) disables pop3\n");
          puts("  -n [<portnum>] set tcp-port for nntp (default 8119)\n"
               "  -n (without port) disables nntp\n");
          puts("  -m [<portnum>] set tcp-port for serv (default 8123)\n"
               "  -m (without port) disables serv\n");
          exit(0);
      }
      argc--;
    }
  }
*/
}

#endif

/*---------------------------------------------------------------------------*/

int main (int argc, char *argv[])
//*************************************************************************
//
// Hauptfunktion
//
//*************************************************************************
{
  lastfunc("main");
  getparm(argc, argv);
#ifndef __FLAT__
 #ifndef __DPMI32__
  harderr(crit_handler);
 #endif
 #ifdef BIOS_Y2K
  chk_y2k_comp();
 #endif
  ctrlbrk(break_handler);
  t_allocinit();
  video_open();
  fixpath(argv[0]);
  cdhome();
  atexit(main_close);
  //exelock(argv[0]);
#else
 #ifdef _WIN32
  OSVERSIONINFO OsVer;
  OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&OsVer);
  if (OsVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
      if (MessageBox(0, "Windows 95/98 or WIN32s is not recommended for bcm32!\n"
                         "Press OK to continue on your own risk or\n"
                         "Press Cancel to quit and install Windows NT/2000/XP.\n",
                         "Notification", MB_OKCANCEL) != IDOK)
        exit(0);
 #endif
  t_allocinit();
  fixpath(argv[0]);
  cdhome();
  atexit(abortmsg);
#endif
  maintask();
  return 0;
}

/*---------------------------------------------------------------------------*/

