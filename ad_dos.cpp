/***************************************************************

  BayCom(R)   Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------
  Betriebssystemabhaengige Teile DOS
  ----------------------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980124 OE3DZW added CPU-type to DOS-Version output (fixed bug)
//                changed text for 486 to ">=486"
//19980208 DG9AML added support for >=486 CPU's
//20000108 F1ULQ  added date correction (y2k) for buggy BIOS
//20030705 DB1RAS added _PREPOSTEXEC (dospreexec, dospostexec)

#include "baycom.h"

/*---------------------------------------------------------------------------*/

void cdhome (void)
//*************************************************************************
//
//  Wechselt Laufwerk und Pfad zu dem, aus dem das Programm
//  gestartet wurde
//
//*************************************************************************
{
  char path[60];
  int l;

  strcpy(path, globalpath);
  strupr(path);
  l = strlen(path);
  if (path[l - 1] == '\\' || path[l - 1] == '/') path[l - 1] = 0;
#ifdef _WIN32
  _chdrive(*path - '@');
#else
  setdisk(*path - 'A');
#endif
  chdir(path);
}

/*---------------------------------------------------------------------------*/

void waitdoskey (void)
//*************************************************************************
//
//
//*************************************************************************
{
  time_t begin = ad_time();
  while (! bioskey(1))
  {
    if (ad_time() > (begin + 30)) return;
  }
  bioskey(0);
}

/*---------------------------------------------------------------------------*/

void resetreadonly (char *name)
//*************************************************************************
//
//
//*************************************************************************
{
#ifdef __DOS16__
  struct ffblk f;
  if (! findfirst(name, &f, 0xffff))
  {
    if (f.ff_attrib & FA_RDONLY)
    {
      trace(replog, "resetreadonly", "%s", name);
      if (! sema_access(name)) _chmod(name, 1, FA_ARCH);
    }
  }
#endif
}

/*---------------------------------------------------------------------------*/

#ifndef _WIN32
unsigned long dfree (char *path, int total)
//*************************************************************************
//
// returns free disc space in kBytes, max. 4 GBytes
//
//*************************************************************************
{
  struct dfree free;
  int drive;

  if (path[1] == ':') drive = toupper(*path) - 'A';
  else drive = getdisk();
  getdfree(drive + 1, &free);
  if (free.df_sclus != 0xFFFF)
  {
    if (total)
      return (long)((long)free.df_total * (long)free.df_bsec
                                        * (long)free.df_sclus) >> 10;
    else
      return (long)((long)free.df_avail * (long)free.df_bsec
                                        * (long)free.df_sclus) >> 10;
  }
  else return 0;
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef __DOS16__

#define L2STK 500

static char stack[L2STK];
static int chr = 0;
static unsigned sps, sss;
static void interrupt(*int10_old)(...);

//                    0    1    2    3    4    5    6    7    8    9
static
char keytab[256]= {   0,   0,   0,   0,   0,   0,   0,   0,   0,0x0f,
                   0x1c,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,0x39,0x02,0x01,0x2B,0x05,0x06,0x07,0x28,
                   0x09,0x0A,0x1B,0x1B,0x33,0x35,0x34,0x08,0x0B,0x02,
                   0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x34,0x33,
                   0x56,0x0B,0x56,0x0C,0x10,0x1E,0x30,0x2E,0x20,0x12,
                   0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,
                   0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x2c,
                   0x15,0x09,0x0C,0x0A,0x07,0x35,0x29,0x1E,0x30,0x2E,

                   0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32, //100
                   0x31,0x18,0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,
                   0x2D,0x2c,0x15,0x08,0x56,0x0B,   0,   0,   0,0x1a,
                      0,   0,0x28,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,0x28,   0,   0,   0,   0,   0,0x27,   0,
                      0,   0,   0,0x27,0x1a,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,

                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //200
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,0x0c,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                      0,   0,   0,   0,   0,   0
        };

/*---------------------------------------------------------------------------*/

static void near get (void)
//*************************************************************************
//
//
//*************************************************************************
{
  sss = _SS;
  sps = _SP;
  _SS = FP_SEG(stack + L2STK);
  _SP = FP_OFF(stack + L2STK);
  if (chr == EOF)
  {
    if (getvalid()) chr = getv();
  }
  _SS = sss;
  _SP = sps;
}

/*---------------------------------------------------------------------------*/

#pragma argsused
static void interrupt int16_handler (unsigned bp, unsigned di, unsigned si,
                                     unsigned ds, unsigned es, unsigned dx,
                                     unsigned cx, unsigned bx, unsigned ax,
                                     unsigned ip, unsigned cs, unsigned flags)
//*************************************************************************
//
//
//*************************************************************************
{
  switch ((ax >> 8) & 15)
  {
    case 0: while (chr == EOF) get();
            ax = chr | (unsigned) (keytab[chr] << 8);
            chr = EOF;
            break;
    case 1: get();
            if (chr == EOF)
            {
              flags |= 64;
              ax = 0;
            }
            else
            {
              flags &= (0xffff - 64);
              ax = chr | (unsigned) (keytab[chr] << 8);
            }
  }
}

/*---------------------------------------------------------------------------*/

static void interrupt put (void)
//*************************************************************************
//
//
//*************************************************************************
{
  static int cc;
  cc = _AX;
  if ((cc >> 8) == 0xe)
  {
    sss = _SS;
    sps = _SP;
    _SS = FP_SEG(stack + L2STK);
    _SP = FP_OFF(stack + L2STK);
    enable();
    cc &= 255;
    while (! putfree());
    if (cc && cc != CR)
      putv(cc);
    _SS = sss;
    _SP = sps;
  }
}

/*---------------------------------------------------------------------------*/

#pragma argsused
static void interrupt int10_handler (unsigned bp, unsigned di, unsigned si,
                                     unsigned ds, unsigned es, unsigned dx,
                                     unsigned cx, unsigned bx, unsigned ax,
                                     unsigned ip, unsigned cs, unsigned flags)
//*************************************************************************
//
//
//*************************************************************************
{
  put();
  _chain_intr(int10_old);
}

#endif

/*---------------------------------------------------------------------------*/

static int near oshell_old (char *befbuf)
//*************************************************************************
//
//
//*************************************************************************
{
  int retwert = 0;
  char s[120];
  char tmp_out[20];
  char tmp_err[20];
  strcpy(tmp_out, time2filename(0));
  strcpy(tmp_err, time2filename(0));
  if (*befbuf)
  {
    waitfor(e_reschedule);
#ifdef OLD_SHELL        //DF3VI
//voellig ueberfluessiger doppel-aufruf von command.com entfernt,
//da system() sowieso den kommando-interpreter aufruft.
    char *commandcom = getenv("COMSPEC");
    if (! commandcom) commandcom = "command";
    sprintf(s, "%s /c %s >%s", commandcom, befbuf, tmp_out);
#else
    sprintf(s, "%s >%s", befbuf, tmp_out);
#endif
    freopen(tmp_err, "wt", stderr);
    retwert = system(s);
    cdhome();
    freopen("con", "wt", stderr);
    waitfor(e_reschedule);
    if (! retwert)
    {
      if (filetime(tmp_out))
      {
        fileio_text fio;
        fio.usefile(tmp_out);
        fio.doerase();
        fio.tx();
      }
      waitfor(e_reschedule);
      fileio_text fio;
      fio.usefile(tmp_err);
      fio.doerase();
      fio.tx();
      waitfor(e_reschedule);
    }
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

#ifdef __DOS16__
  #ifdef __BORLANDC__
  #pragma argsused
  #endif
int oshell (char *befbuf, shellinput_t input)
//*************************************************************************
//
//
//*************************************************************************
{
  int retwert;
  user_t *uu = u; // save value of user pointer for restoring below
                  // a task suspend would cause a system crash
  set_watchdog(2);
#ifdef _PREPOSTEXEC //db1ras
  if (strcmp(m.dospreexec, "off")) system(m.dospreexec);
#endif
  disable_tnc();
  static void interrupt (*int16_old)(...);
  if (input == sh_forceinput)
  {
    video_close();
    if (! e) // only if no console window is active
    {
      u = NULL; // disable user pointer to avoid page stop
      int16_old = getvect(0x16);
      int10_old = getvect(0x10);
      chr = EOF;
      setvect(0x16, (void interrupt (*)(...))int16_handler);
      setvect(0x10, (void interrupt (*)(...))int10_handler);
    }
    if (*befbuf) retwert = system(befbuf);
    else
#ifdef OLD_SHELL
    {
      char *commandcom = getenv("COMSPEC");
      if (! commandcom) commandcom = "command";
      printf("Type EXIT to return to " STD_BOXHEADER "...");
      retwert = system(commandcom);
    }
#else
    {
      //ueberfluessiger Doppel-aufruf von command.com entfernt,
      //da system() sowieso den Kommando-Interpreter aufruft (db1ras)
      printf("Type EXIT to return to " STD_BOXHEADER "...");
      retwert = system("");
    }
#endif
    if (e)
    {
      if (*befbuf)
      {
        printf("\r-- press a key --");
        waitdoskey();
        printf("\r              \r");
      }
    }
    else
    {
      setvect(0x16, int16_old);
      setvect(0x10, int10_old);
    }
    cdhome();
    video_open();
  }
  else retwert = oshell_old(befbuf);
#ifdef _PREPOSTEXEC //db1ras
  if (strcmp(m.dospostexec, "off")) system(m.dospostexec);
#endif
  set_watchdog(1);
  allupdate++;
  u = uu; // restore value of user pointer
  return retwert;
}
#endif

/*---------------------------------------------------------------------------*/

#ifndef _WIN32
char *memfree (int swapfree)
//*************************************************************************
//
//
//*************************************************************************
{
  static char ret[20];
#ifdef __DPMI32__
  static char ret2[20];
  if (swapfree)
  {
    _BX = 0xffff;
    _AX = 0x100;
    geninterrupt(0x31);
    sprintf(ret, "%u", _BX >> 6);
    return ret;
  }
  else
  {
    sprintf(ret2, "%u", farcoreleft() >> 10);
    return ret2;
  }
#else
  if (swapfree) return "-";
  else
  {
    unsigned max = 0;
    sprintf(ret, "%u", allocmem(65535U, &max) >> 6);
    // allocmem() must be used instead of coreleft() because Borland
    // seems not to be able to provide a bug-free library
    return ret;
  }
#endif
}
#endif

/*---------------------------------------------------------------------------*/

static void near testusv (void)
//*************************************************************************
//
//  Leitung an LPT1 zur USV-Ueberwachung testen
//
//  wenn LPT1 Pin15 auf Masse geht,
//  soll die Box abgeschaltet werden
//  eventuell externen Pullup-R auf +5V
//
//*************************************************************************
{
#ifdef __DOS16__
  unsigned lpt1 = ad_peek(0x40, 8); // Adresse vom BIOS holen
  if (! (inportb(lpt1 + 1) & 0x08)) // 2tes Register von LPT1 ansprechen
  {
    runterfahren = 1;
    strcpy(stopreason, "usv failure");
    trace(serious, "desktop", "USV failure detected");
  }
#endif
}

/*---------------------------------------------------------------------------*/

#ifdef FEATURE_MOUSE
static void near testmaus (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (mausknopf())
  {
    mausaus();
    newkeywin = vorne[mauszeile() * bildspalten + mausspalte()];
    resize (newkeywin, 2);
    while (mausknopf()) wdelay(139);
    mausein();
  }
}
#endif

/*---------------------------------------------------------------------------*/

void testcputask (void)
//*************************************************************************
//
//
//*************************************************************************
{
  long tic = systic;
  int i = 0, j = 0;
#ifndef _WIN32
  while (tic == systic) j++;
  tic = systic;
  while (tic == systic)
  {
    i++;
    for (j = 1; j < 1200; j++);
  }
  if ((tic + 1) == systic && i > cpuindex) cpuindex = i;
#endif
}

/*---------------------------------------------------------------------------*/

void desk_cyclic (void)
//*************************************************************************
//
//
//*************************************************************************
{
#ifndef _WIN32
  static int zaehler = 0;
  void test_critical (void);
  int lptadr = ad_peek(0x40, 8);
  if (lptadr && (m.watchdog & 1)) outportb(lptadr, inportb(lptadr) ^ 1);
  if ((zaehler++) > 50)
  {
    zaehler = 0;
    if (m.usvsense) testusv();
    if (heapcheck() < 0) trace(tr_abbruch, "desktop", "heap corrupted");
  }
#ifdef FEATURE_MOUSE
  if (mausistda()) testmaus();
#endif
  test_critical();
#endif
}

/*---------------------------------------------------------------------------*/

#ifdef __DOS16__
void task_idle (int active)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! active)
  {
    _AX = _BX = _CX = 0;
    geninterrupt(0x28);
  }
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef __DPMI32__
typedef struct reg_t
//*************************************************************************
//
//
//*************************************************************************
{
  long unsigned edi;
  long unsigned esi;
  long unsigned ebp;
  long unsigned reserved;  // should be 0
  long unsigned ebx;
  long unsigned edx;
  long unsigned ecx;
  long unsigned eax;
  short unsigned flags;
  short unsigned es;
  short unsigned ds;
  short unsigned fs;       // should be 0
  short unsigned gs;       // should be 0
  short unsigned ip;
  short unsigned cs;
  short unsigned sp;
  short unsigned ss;
} reg_t;

/*---------------------------------------------------------------------------*/

void setvect (int num, void interrupt (*isr)( ... ))
//*************************************************************************
//
//
//*************************************************************************
{
  _EAX = 0x0303;
  _ESI = (long) isr;
  geninterrupt(0x31);
  _EBX = num;
  _EAX = 0x0201;
  geninterrupt(0x31);
}

/*---------------------------------------------------------------------------*/

void setvect1 (int num, void interrupt (*isr)( ... ))
//*************************************************************************
//
//
//*************************************************************************
{
  _ECX = _CS;
  _EBX = num;
  _EAX = 0x0303;
  _EDX = (long) isr;
  geninterrupt(0x31);
}

/*---------------------------------------------------------------------------*/

void interrupt (*getvect (int num))(...)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned ret;
  _EBX = num;
  _EAX = 0x0204;
  geninterrupt(0x31);
  ret = _EDX;
  return (void interrupt (*)(...))ret;
}

/*---------------------------------------------------------------------------*/

static int mausda = 0;

/*---------------------------------------------------------------------------*/

int mausistda (void)
//*************************************************************************
//
//
//*************************************************************************
{
  return mausda;
}

/*---------------------------------------------------------------------------*/

int mausinit (int zeilen, int spalten)
//*************************************************************************
//
//
//*************************************************************************
{
  _AX = 0;
  geninterrupt(0x33);
  if (_AX != 0xffff)
  {
    mausda = 0;
    return 0;
  }
  mausda = 1;
  _BX = 0;
  _CX = 0xffff;
  _DX = 0x7700;
  _AX = 0x0a;
  geninterrupt(0x33);
  _CX = (79 * 8);  // ins Eck positionieren
  _DX = 0;
  _AX = 4;
  geninterrupt(0x33);
  _AX = 1;             // Mauscursor anzeigen
  geninterrupt(0x33);
  zeilen <<= 3;
  _DX = zeilen;
  _CX = 1;
  _AX = 8;
  geninterrupt(0x33);
  spalten <<= 3;
  _DX = spalten;
  _CX = 1;
  _AX = 7;
  geninterrupt(0x33);
  return 1;
}

/*---------------------------------------------------------------------------*/

void maussetz (int xpos, int ypos)
//*************************************************************************
//
//
//*************************************************************************
{
  if (mausda)
  {
    xpos <<= 3;
    ypos <<= 3;
    _CX = xpos;
    _DX = ypos;
    _AX = 4;
    geninterrupt(0x33);
  }
}

/*---------------------------------------------------------------------------*/

void mausein (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (mausda == 2)
  {
    mausda = 1;
    _AX = 1;
    geninterrupt(0x33);
  }
}

/*---------------------------------------------------------------------------*/

void mausaus (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (mausda == 1)
  {
    mausda = 2;
    _AX = 2;
    geninterrupt(0x33);
  }
}

/*---------------------------------------------------------------------------*/

int  mauszeile (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! mausda) return 0;
  _AX = 3;
  geninterrupt(0x33);
  return _DX >> 3;
}

/*---------------------------------------------------------------------------*/

int mausspalte (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! mausda) return 0;
  _AX = 3;
  geninterrupt(0x33);
  return _CX >> 3;
}

/*---------------------------------------------------------------------------*/

int mausknopf (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! mausda) return 0;
  _AX = 3;
  geninterrupt(0x33);
  return _BX & 1;
}

/*---------------------------------------------------------------------------*/

int mausrechts (void)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! mausda) return 0;
  _AX = 3;
  geninterrupt(0x33);
  return _BX & 2;
}
#endif

/*---------------------------------------------------------------------------*/

#if 0
char *ad_modifymask (char *mask)
//*************************************************************************
//
//
//*************************************************************************
{
  struct ffblk direntry;
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char file[MAXFILE];
  char ext[MAXEXT];
  int flags;
  int exist = (findfirst(mask, &direntry, 0) == 0);
  flags = fnsplit(mask, drive, dir, file, ext);

  if (drive[0] == 0)
  {
    drive[0] = getdisk() + 'A';
    drive[1] = ':';
    drive[2] = 0;
  }
  if (exist == 0)
  {
    if (! strchr(mask, '*') && (findfirst(mask, &direntry, FA_DIREC) == 0))
    {
      strcat(dir, file);
      strcat(dir, "\\");
      strcpy(file, "*");
      strcpy(ext, ".*");
      flags|=DIRECTORY;
    }
    else
    {
      if (! (flags & FILENAME)) strcpy(file, "*");
      if (! (flags & EXTENSION)) strcpy(ext, ".*");
    }
  }

  if (dir[0] != '\\')
  {
    char curdir[MAXDIR];
    curdir[0] = '\\';
    if (getcurdir(drive[0]-'@', curdir+1) == 0)
    {
      if (curdir[strlen(curdir)-1] != '\\')
        strcat(curdir, "\\");
      strcat(curdir, dir);
      strcpy(dir, curdir);
    }
  }

  fnmerge(mask, drive, dir, file, ext);
  strupr(mask);
  return mask;
}

/*---------------------------------------------------------------------------*/

char *ad_dirline (char *mask, int mode)
//*************************************************************************
//
//  mode = 0 1 :  Datum / Uhrzeit
//  mode = 2 3 :  Filename / ext / Filegroesse
//
//  gerade: findfirst  ungerade=findnext
//
//*************************************************************************
{
  static struct ffblk direntry;
  static char line[60];
  char *ext;

  do
  {
    if ((mode & 1)==0)
    {
      if ((-1) == findfirst(mask, &direntry, 0xffff))
        return NULL;
    }
    else
    {
      if ((-1) == findnext(&direntry))
        return NULL;
    }
    line[0] = 0;
    switch (mode & 6)
    {
      case 2:
        ext = strchr(direntry.ff_name, '.');
        if (ext > direntry.ff_name)
          ext[0] = 0;
        else
          ext = ".";
        if (direntry.ff_attrib & FA_DIREC)
          sprintf(line, "%-8s %-3s <DIR>    ", direntry.ff_name, ext+1);
        else
          sprintf(line, "%-8s %-3s %8lu ", direntry.ff_name, ext+1,
                  direntry.ff_fsize);
      case 0:
        sprintf(line+strlen(line), "%02d.%02d.%02d %02d:%02d",
                direntry.ff_fdate & 31, (direntry.ff_fdate>>5) & 15,
                ((direntry.ff_fdate>>9)+80)%100, (direntry.ff_ftime>>11),
                (direntry.ff_ftime>>5) & 63);
      break;
    }
    mode|=1;
  }
  while (direntry.ff_name[0]=='.');
  return line;
}
#endif

#ifdef __DOS16__

/*---------------------------------------------------------------------------*/

extern int getproz (void);    // zur Einbindung der Assemblerroutine
extern int getco (void);

static char *prozname[] =
//*************************************************************************
//
//
//*************************************************************************
{
  "unknown",     // Code 0
  "8086",        // Code 1
  "80286",       // Code 2
  "80386",       // Code 3
  "80486",       // Code 4
  "80586",       // Code 5
  "80686",       // Code 6
  "80786",       // Code 7
  "80886",       // Code 8
};

/*---------------------------------------------------------------------------*/

char *cpuinfo (void)
//*************************************************************************
//
//
//*************************************************************************
{
  char cpuresult[50];
  strcpy(cpuresult, "CPU: ??");
  if (cpuindex)
  {
    int proz = getproz();
    sprintf(cpuresult, "CPU: %s  Index: %ld", prozname[proz], cpuindex);
  }
  return cpuresult;
}
#endif


/*---------------------------------------------------------------------------*/

int snprintf (char *str, size_t n, const char *format,...)
//*************************************************************************
//
//
//*************************************************************************
{
  va_list argpoint;
  size_t len;

  va_start(argpoint, format);
  vsprintf(str, format, argpoint);
  va_end(argpoint);
  len = strlen(str);
  if (len > n) trace(fatal, "snprintf", "strlen %d overflow %s", len, format);
  return (int) len;
}

/*---------------------------------------------------------------------------*/

#ifdef BIOS_Y2K
void chk_y2k_comp () //Correct Y2K problem in Bios 08/01/2000 F1ULQ
//*************************************************************************
//
//
//*************************************************************************
{
  date sys_date;
  int y2k = atoi(YEAR);

  getdate(&sys_date);
  if (sys_date.da_year < y2k || sys_date.da_year > (y2k + 5))
  {
    sys_date.da_year = y2k;
    setdate(&sys_date);
    trace(serious, "y2k", "incorrect date, set to %d", y2k);
  }
}
#endif

/*---------------------------------------------------------------------------*/
