/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------
  Betriebssystemabhaengige Teile fuer Linux
  -----------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980329 OE3DZW added vendor-id
//19980614 OE3DZW dfree in kbytes
//19980909 OE3DZW added cpu-ticks to cpuinfo(), added rdtsc()
//20000613 DK2UI  changed oshell for multi commands

#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <termios.h>
#include <crypt.h>

#include "baycom.h"

/*---------------------------------------------------------------------------*/

char *strupr (char *s)
//*************************************************************************
//
//*************************************************************************
{
/*
  char tmp[300];

  strcpy(tmp, s);
  char *tmp2 = tmp;
  if (tmp2)
  {
    while (*tmp2)
    {
      *tmp2 = toupper(*tmp2);
      tmp2++;
    }
  }
  return tmp2;
*/
  if (s)
  {
    while (*s)
    {
      if (islower(*s))
        *s = toupper(*s);
      s++;
    }
  }
  return s;
}

/*---------------------------------------------------------------------------*/

char *strlwr (char *s)
//*************************************************************************
//
//*************************************************************************
{
/*
  char tmp[300];
  int i;

  strcpy(tmp, s);
  if (s)
  {
    for (i = 0; i < 300; i++)
      tmp[i] = tolower(tmp[i]);
  }
  return tmp;
*/
  if (s)
  {
    while (*s)
    {
      if (isupper(*s))
        *s = tolower(*s);
      s++;
    }
  }
  return s;
}

/*---------------------------------------------------------------------------*/

int bioskey (int mode)
//*************************************************************************
//
//*************************************************************************
{
  return 0;
}

/*---------------------------------------------------------------------------*/

void cdhome (void)
//*************************************************************************
//
//*************************************************************************
{
  chdir(globalpath);
}

/*---------------------------------------------------------------------------*/

long filelength (int fd)
//*************************************************************************
//
//*************************************************************************
{
  long pos = lseek(fd, 0L, SEEK_CUR);
  long len = lseek(fd, 0L, SEEK_END);
  lseek(fd, pos, SEEK_SET);
  return len;
}

/*---------------------------------------------------------------------------*/

int mkdir (char const *path)
//*************************************************************************
//
//*************************************************************************
{
  return mkdir(path, 0777);
}

/*---------------------------------------------------------------------------*/

int random_max (int max)
//*************************************************************************
//
//*************************************************************************
/* from man page "rand" (Linux Programmers Manual":
       In Numerical Recipes in C: The Art of Scientific Computing
       (William  H.  Press, Brian P. Flannery, Saul A. Teukolsky,
       William T.  Vetterling;  New  York:  Cambridge  University
       Press,  1990 (1st ed, p. 207)), the following comments are
       made:
              "If you want to generate a random integer between 1
              and 10, you should always do it by

                     j=1+(int) (10.0*rand()/(RAND_MAX+1.0));

              and never by anything resembling

                     j=1+((int) (1000000.0*rand()) % 10);

              (which uses lower-order bits)."

       Random-number  generation is a complex topic.  The Numeric
       cal Recipes in C book (see reference  above)  provides  an
       excellent discussion of practical random-number generation
       issues in Chapter 7 (Random Numbers).



*/
{
  //return random()%max;
  return (int) ((1.0 * max) * rand() / (RAND_MAX + 1.0));
}

/*---------------------------------------------------------------------------*/

int l2aufruf1 (int, int, int, char *)
//*************************************************************************
//
//*************************************************************************
{
  return 0;
}

/*---------------------------------------------------------------------------*/

#ifdef oldpty
static int find_pty (char *ptyname) //ptyname = /dev/tty??
//*************************************************************************
//
//*************************************************************************
{
  char master[80]; // master = /dev/pty??
  int fdm, fds, num;

  // tries to open pty, this should fail, then tries to open tty, this
  // should succeed, then tries to open pty, this should succeed
  // there seems to be a difference between kernel 2.0.0 and 2.0.27
  for (num = 160; num > 0; num--)
  {
    sprintf(ptyname, "/dev/tty%c%x", 'q' + (num >> 4), num & 0xf);
    if (1) // ((fds = open(ptyname, O_RDWR | O_NONBLOCK |O_EXCL)) < 0)
    {
      sprintf(master, "/dev/pty%c%x", 'q' + (num >> 4), num & 0xf);
      if ((fdm = open(master, O_RDWR | O_NONBLOCK | O_EXCL)) >= 0)
      {
        if ((fds = open(ptyname, O_RDWR | O_NONBLOCK | O_EXCL)) >= 0)
        {
          //trace(report, "find_pty", "open %s", ptyname);
          close(fds);
          return fdm;
        }
        else close(fdm);
      }
    }
    else close(fds);
  }
  trace(serious, "find_pty", "no pty found");
  return EOF;
}
#endif

/*---------------------------------------------------------------------------*/

int oshell (char *name, shellinput_t input)
//*************************************************************************
//
// Shell to operating system
//
//*************************************************************************
{
  int fd, ret;
  char s[256], cl[LINELEN+1];
  char c;
  int wasdagewesen = 0;
  int wasalife = 0;
  struct termios termios_;
  int childpid;
  char ptyname[100];
  time_t begintime = ad_time();

#ifdef oldpty
  fd = find_pty(ptyname);
  if (fd == EOF) return 1;
#else
  if ((fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK | O_EXCL)) >= 0)
  {
    grantpt(fd);
    unlockpt(fd);
    strcpy(ptyname, ptsname(fd));
   // trace(report, "find_pty", "open %s", ptyname);
  }
  else
  {
    trace(serious, "find_pty", "no pty found");
    return 1;
  }
#endif
  s_set(fd, ptyname, "lab");
  signal(SIGCLD, SIG_IGN);
  if (! (childpid = fork()))
  {
    int i,j;
    static struct itimerval rttimer, old_rttimer;

    memset(&rttimer, 0, sizeof(struct itimerval));
    setitimer(ITIMER_REAL, &rttimer, &old_rttimer);
    signal(SIGCLD, SIG_IGN);
    setpgrp();
    setsid();
    for (i = 0; i < FD_SETSIZE; i++) close(i);
    fd = open(ptyname, O_RDWR, 0666);
    dup(0);  // duplicate an open file descriptor
    dup(0);
    //chmod(ptyname, 0622);
    j = ioctl(0, TIOCSCTTY, (char *) 0);
    //trace(report, "oshell", "j = %d",j);
    memset((char *) &termios_, 0, sizeof(struct termios));
    termios_.c_iflag = ICRNL | IXOFF;
    termios_.c_oflag = OPOST | ONLCR;
    termios_.c_cflag = CS8 | CREAD | CLOCAL;
    termios_.c_lflag = ISIG | ICANON | ECHO;
    termios_.c_cc[VINTR]  =   3; /* ^C */
    termios_.c_cc[VSUSP]  =  26; /* ^Z */
    termios_.c_cc[VQUIT]  =  28; /* ^\ */
    termios_.c_cc[VERASE] =   8; /* ^H */
    termios_.c_cc[VKILL]  =  24; /* ^X */
    termios_.c_cc[VEOF]   =   4; /* ^D */
    cfsetispeed(&termios_, B1200);
    cfsetospeed(&termios_, B1200);
    tcsetattr(fd, TCSANOW, &termios_);
    tcsetpgrp(fd, getpgrp());
    if (! *name)
    {
      printf("Type `exit' or ctrl-d to return to " STD_BOXHEADER "...\n");
      execl("/bin/bash", "-bash", NULL);
    }
    else
    {
      char cmd[200];

      if (file_isreg("bashrc.bcm"))
        sprintf(cmd, "source bashrc.bcm;%s", name);
      else
        strcpy(cmd, name);
      execl("/bin/bash", "bash", "-c", cmd, NULL);
    }
    perror("execlp");
    exit(0); // sollte nie aufgerufen werden
  }
  t->childpid = childpid;
  while (fd != EOF)
  {
    while ((ret = read(fd, &c, 1)) > 0)
    {
      wasalife = 1;
      wasdagewesen = 1;
      putv(c);
    }
    if (ret < 0)
    {
      if (((ad_time() > (begintime + 2)) || wasalife) && errno != EAGAIN)
      {
        s_close(fd);
        break;
      }
    }
    else wasalife = 1;
    if (wasdagewesen)
    {
      putflush();
      wasdagewesen = 0;
    }
    if (! *name || input == sh_interactive)
//    if(input==sh_ifmultitask || input==sh_forceinput)
    {
      while (getvalid() && fd != EOF)
      {
        getline(s, sizeof(s) - 2, 1);
        if (*s != 4 && ! *name)
        {
          sprintf(cl, "oshell \"%.70s\"", s);
          cmdlog(cl);
          lastcmd(cl);
        }
        strcat(s, "\n");
        if (fd != EOF)
        {
          if (write(fd, s, strlen(s)) != (long) strlen(s))
          {
            s_close(fd);
            fd = EOF;
          }
        }
      }
    }
    wdelay(133);
  }
  putflush();
  kill(-childpid, 9);
  t->childpid = 0;
  return 0;
}

/*---------------------------------------------------------------------------*/

void resetreadonly (char *name)
//*************************************************************************
//
// dummy, only valid for DOS
//
//*************************************************************************
{
}

/*---------------------------------------------------------------------------*/

unsigned long dfree (char *path,int total)
//*************************************************************************
//
// returns available space in kbytes (max. 4 TBytes)
//
//*************************************************************************
{
  struct statfs s;
  long kblock;
  statfs(path, &s);

  if (total) kblock = s.f_blocks >> 10;
  else kblock = s.f_bavail >> 10; // f_bfree would be correct for "root"
  return s.f_bsize * kblock;
}

/*---------------------------------------------------------------------------*/

char *memfree (int swap)
//*************************************************************************
//
// ermittelt aus /proc/meminfo den...
//                  bei swap = 0  ... freien Arbeitsspeicher
//                  bei swap = 1  ... freien Swap-Speicher
//
//*************************************************************************
{
  FILE *f;
  char buf[100], b1[20], b2[20], b3[20], b4[20], b5[20], b6[20];
  char swaptotal[20], swapfree[20], memfree[20], buffer[20], cached[20];
  static char memresult[20], swapresult[20];
/* altes Format bis Kernel 2.6.x:
        total:    used:    free:  shared: buffers:  cached:
Mem:  394739712 370683904 24055808        0 30818304 201142272
Swap: 320745472  9101312 311644160
MemTotal:       385488 kB
MemFree:         23492 kB
MemShared:           0 kB
Buffers:         30096 kB
Cached:         190644 kB
SwapCached:       5784 kB
Active:         110084 kB
Inactive:       212676 kB
HighTotal:           0 kB
HighFree:            0 kB
LowTotal:       385488 kB
LowFree:         23492 kB
SwapTotal:      313228 kB
SwapFree:       304340 kB
BigFree:             0 kB

   neues Format ab Kernel 2.6.x:
MemTotal:       255396 kB
MemFree:         28776 kB
Buffers:         18324 kB
Cached:         114140 kB
SwapCached:          0 kB
Active:         136928 kB
Inactive:        68876 kB
HighTotal:           0 kB
HighFree:            0 kB
LowTotal:       255396 kB
LowFree:         28776 kB
SwapTotal:      538168 kB
SwapFree:       538168 kB
Dirty:             384 kB
Writeback:           0 kB
Mapped:          97204 kB
Slab:            15856 kB
Committed_AS:   161592 kB
PageTables:       1232 kB
VmallocTotal:   770040 kB
VmallocUsed:      2440 kB
VmallocChunk:   767188 kB
HugePages_Total:     0
HugePages_Free:      0
Hugepagesize:     4096 kB
*/
  strcpy(cached, "0");
  strcpy(swapresult, "-");
  strcpy(memresult, "-");
  f = s_fopen("/proc/meminfo", "srt");
  if (f)
  {
    fgets(buf, sizeof(buf)-1, f); // try find headerline old format
    if (sscanf(buf, "%s %s %s %s %s %s", b1, b2, b3, b4, b5, b6) == 6)
    {
      fgets(buf, sizeof(buf)-1, f); // read memory
      if (sscanf(buf, "%s %s %s %s %s %s %s", b1, b2, b3, memfree,
                                               b4, buffer, cached) >= 6)
        sprintf(memresult, "%lu", (atol(memfree) + atoi(buffer)
                                   + atoi(cached)) >> 10);
      fgets(buf, sizeof(buf)-1, f); // read swap
      if (sscanf(buf, "%s %s %s", b1, b2, b3) == 3)
        sprintf(swapresult, "%lu", atol(b3) >> 10);
    }
    else // no headerline means new format
    {
      while (! feof(f))
      {
        if (sscanf(buf, "%s %s %s", b1, b2, b3) == 3)
        {
          if (! strcmp(b1, "SwapTotal:")) strcpy(swaptotal,b2);
          if (! strcmp(b1, "SwapFree:"))  strcpy(swapfree, b2);
          if (! strcmp(b1, "MemFree:"))   strcpy(memfree, b2);
          if (! strcmp(b1, "Buffers:"))   strcpy(buffer, b2);
          if (! strcmp(b1, "Cached:"))    strcpy(cached, b2);
        }
        fgets(buf, sizeof(buf)-1, f); // read next line
      }
      sprintf(memresult, "%lu",
               ((atol(memfree) + atol(buffer) + atol(cached)) * 1024) >> 10);
      sprintf(swapresult, "%lu",
               ((atol(swaptotal) - atol(swapfree)) * 1024) >> 10);
    }
    s_fclose(f);
  }
  if (swap) return swapresult;
  else return memresult;
}

/*---------------------------------------------------------------------------*/
/* Source:
        Linux I/O port programming mini-HOWTO

        Author: Riku Saikkonen <Riku.Saikkonen@hut.fi>
        Last modified: Mar 30 1997
*/
//Other Processors do not support i386 Assembler -F1ULQ
#ifndef __sparc__
   extern __inline__ unsigned long long int rdtsc()
   {
     unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
   }
#endif
/*---------------------------------------------------------------------------*/

char *cpuinfo (void)
//*************************************************************************
//
// CPU-Infos vom Betriebssystem holen
//
//*************************************************************************
{
  FILE *f;
  char buf[100], cpu[20], bogo[20];
  char vendor[30];
  int tsc = NO;
  static char cpuresult[200];
  char *pk, *ptmp;

  pk = NULL;
  strcpy(cpu, "??");
  strcpy(bogo, "??");
  strcpy(vendor, "");
  strcpy(cpuresult, "-");
  f = s_fopen("/proc/cpuinfo", "srt");
  if (f)
  {
    while (fgets(buf, sizeof(buf) - 1, f))
    {
      #if defined (__arm__)
      if (stristr(buf, "model name"))
      #else
      if (stristr(buf, "cpu"))
      #endif
      {
        pk = strstr(buf, ": ");
        if (pk && *cpu == '?') {
            ptmp = strchr(pk + 2, ' ');
            if (ptmp != NULL)
            	*ptmp = '\0';
            sscanf(pk + 2, "%s", cpu);
        }
      }
      if (stristr(buf, "BogoMips"))
      {
        pk = strstr(buf, ": ");
        if (pk) sscanf(pk + 2, "%s", bogo);
      }
      if (stristr(buf, "vendor_id"))
      {
        pk = strstr(buf, ": ");
        if (pk) sscanf(pk + 2, "%s", vendor);
      }
      if (stristr(buf, "flags"))
      {
        pk = strstr(buf, ": ");
        if (pk) tsc = strstr(buf, "tsc") ? OK : NO;
      }
    }
    s_fclose(f);
#ifdef __i386__       //rdtsc() is i386 specific
    if (*vendor) //only newer cpus show infos
    {
      if (*cpu >= '5' && tsc)
       sprintf(cpuresult, "CPU: %s(%s) %lld ticks BogoMips: %s",
                           cpu, vendor, rdtsc(), bogo);
      else sprintf(cpuresult, "CPU: %s(%s) BogoMips: %s", cpu, vendor, bogo);
    }
    else
#endif
      sprintf(cpuresult, "CPU: %s  BogoMips: %s", cpu, bogo);
  }
  return cpuresult;
}

/*---------------------------------------------------------------------------*/

void testcputask (void)
//*************************************************************************
//
//*************************************************************************
//dummy
{
}

/*---------------------------------------------------------------------------*/

void task_idle (int active)
//*************************************************************************
//
//*************************************************************************
{
//  static long lasttic = 0;
//  int i;

  if (active == 0) pause();
//  if (i == 10 && systic != lasttic)
//  {
//    i = 0;
//    call_l2(active == 2);
//    lasttic = systic;
//  }
//  else
//  {
//    i++;
    call_l2(active == 2);
//  }
}

/*---------------------------------------------------------------------------*/

void coldstart (void)
//*************************************************************************
//
//*************************************************************************
{
  strcpy(stopreason, "WATCHDOG");
  exit(1);
}

/*---------------------------------------------------------------------------*/

int setlinuxpasswd (char *calltmp, char *passwort)
//*************************************************************************
//
//  Legt einen neuen System-User in /etc/passwd bzw. /etc/shadow an
//  Existiert der User schon, wird das Passwort neu gesetzt
//  Rueckgabewerte:  0 Abbruch, es wurde nichts gemacht
//                   1 User wurde angelegt
//                   2 User existiert, Passwort neu gesetzt
//*************************************************************************
{
#ifndef LINUXSYSTEMUSER
  return 0;
#else
  #define PASSWDFILE          "/etc/passwd"
  #define SPASSWDFILE         "/etc/shadow"
  #define SPASSWDTMPFILE      "/etc/shadow.$$$"
  #define MAXUID              32000

  char homedirparent[80];
  char homedir[80];
  char Homedir[80];
  char Shell[80];
  int Gid = 400;
  unsigned int Maxuid = MAXUID;
  unsigned int Minuid = 800;
  static char bitmap[MAXUID + 1];
  struct passwd *pw;
  struct passwd *pwexist;
  struct stat statbuf;
  FILE *fp = 0, *ft = 0;
  int secured = 0;
  unsigned int uid;
  char salt[3];
  char name[20];
  char s[301];
  char call[CALLEN+1];


  strcpy(name, "setlinuxpasswd");
  strcpy(Shell, "/bin/false");
  strcpy(Homedir, "/home/ax25");
  strcpy(call, calltmp); // nicht mit calltmp arbeiten!
  strlwr(call);
  if (! *passwort)
    return 0;

  // Search existing passwd entry and change password
  if ((pwexist = getpwnam(call)))
  {
    xunlink(SPASSWDTMPFILE);
    if (! (fp = s_fopen(SPASSWDFILE, "srb"))) return 0;
    if (! (ft = s_fopen(SPASSWDTMPFILE, "swb"))) return 0;
    while (fgets(s, 300, fp))
    {
      if (strstr(s, call))
      {
        //hier ist der user
        salt[0] = passwort[0];
        salt[1] = passwort[1];
        salt[2] = 0;
        fprintf(ft, "%s:%s:%d:0:::::\n", call,
                    crypt(passwort, salt), pwexist->pw_uid);
      }
      else
      {
        fprintf(ft, "%s", s);
      }
    }
    s_fclose(fp);
    s_fclose(ft);
    xrename(SPASSWDTMPFILE, SPASSWDFILE);
    xunlink(SPASSWDTMPFILE);
    trace(report, name, "linux login changed for %s", call);
    return 2; //User existiert bereits
  }

  // Find free user id
  memset(bitmap, 0, sizeof(bitmap));
  while ((pw = getpwent()))
  {
    if (! strcmp(call, pw->pw_name)) break;
    if (pw->pw_uid <= Maxuid) bitmap[pw->pw_uid] = 1;
  }
  endpwent();
  if (pw)
  {
    return 0;
  }
  for (uid = Minuid; uid <= Maxuid && bitmap[uid]; uid++) ;
    if (uid > Maxuid) return 0;

  // Add user to passwd file(s)
  sprintf(homedirparent, "%s/%.3s...", Homedir, call);
  sprintf(homedir, "%s/%s", homedirparent, call);
  if (   *SPASSWDFILE
      && ! stat(SPASSWDFILE, &statbuf)
      && (fp = s_fopen(SPASSWDFILE, "sab")))
  {
    salt[0] = passwort[0];
    salt[1] = passwort[1];
    salt[2] = 0;
    /* Definition einer /etc/shadow Zeile

    dgt123:ðHJhpnm5X3kQ:804:0:99999:7:::
    ^- usercall
           ^- password (crypted)
                        ^- days since Jan 1, 1970 that password was
                           last changed
                            ^- days before password may be changed
                              ^- days after which password must be changed
                                    ^- days before password is to expire
                                       that user is warned
                                     ^- days after password expires that
                                        account is disabled
                                      ^- days since Jan 1, 1970 that
                                         account is disabled
                                       ^- reserved field
    */
    fprintf(fp, "%s:%s:%d:0:::::\n", call, crypt(passwort, salt), uid);
    s_fclose(fp);
    secured = 1;
  }
  if (! (fp = s_fopen(PASSWDFILE, "sab"))) return 0;
  fprintf(fp, "%s:%s:%d:%d:AX25 %s:%s:%s\n", call, secured ? "x" : "",
              uid, Gid, call, homedir, Shell);
  s_fclose(fp);
  pw = getpwuid(uid);
  // Create home directory
  mkdir(Homedir, 0755);
  mkdir(homedirparent, 0755);
  mkdir(homedir, 0755);
  chown(homedir, uid, Gid);
  trace(report, name, "linux login created for %s", call);
  return 1;
#endif
}
