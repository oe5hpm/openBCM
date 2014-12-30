/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------
  Anpassungen fuer WIN32-Umgebung
  -------------------------------

  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980831 OE3DZW added setenv for Win32
//19980914 DG9MHZ,OE3DZW fixes to oshell32
//19980921 OE3DZW fixed free_disk
//19980922 DG9MHZ,OE3DZW finaly fixed oshell32

#include "baycom.h"

#ifdef _WIN32

#include <fcntl.h>

/*---------------------------------------------------------------------------*/

int bioskey (int mode)
//*************************************************************************
//
//*************************************************************************
{
  return 0;
}

/*---------------------------------------------------------------------------*/

int random_max (int max)
//*************************************************************************
//
//*************************************************************************
{
  return rand()%max;
}

/*---------------------------------------------------------------------------*/

// struct _finddata_t {
//     unsigned attrib;
//     time_t   time_create;    /* -1 for FAT file systems */
//     time_t   time_access;    /* -1 for FAT file systems */
//     time_t   time_write;
//     _fsize_t size;
//     char     name[260];
// };

/*---------------------------------------------------------------------------*/

DIR *opendir (char *file)
//*************************************************************************
//
//*************************************************************************
{
  char fn[80];

  sprintf(fn, "%s/*.*", file);
  DIR *tmpdir = (DIR *) t_malloc(sizeof(DIR), "odir");
  tmpdir->handle = _findfirst(fn, &(tmpdir->ft));
  if (tmpdir->handle == -1)
  {
    t_free(tmpdir);
    return NULL;
  }
  else
    return tmpdir;
}

/*---------------------------------------------------------------------------*/

struct dirent *readdir (DIR *d)
//*************************************************************************
//
//*************************************************************************
{
  static struct dirent di;

  if (d == NULL)
    return NULL;
  if (d->ft.name[0] == 0)
  {
    if (_findnext(d->handle, &(d->ft)) == -1)
    {
      return NULL;
    }
  }
  strcpy(di.d_name, d->ft.name);
  d->ft.name[0] = 0;
  return &di;
}

/*---------------------------------------------------------------------------*/

void closedir (DIR *d)
//*************************************************************************
//
//*************************************************************************
{
  if (d != NULL)
  {
    _findclose(d->handle);
    t_free(d);
  }
}

/*---------------------------------------------------------------------------*/

unsigned long dfree (char *path, int total)
//*************************************************************************
//
// returns free space in kbytes (max. 4 GBytes)
//
//*************************************************************************
{
  static char cpath[80];
  char *pcpath = cpath;
  union _ULARGE_INTEGER a, b, c;

  strcpy(cpath, path);
  if (cpath[1] == ':')
  {
    cpath[2] = '\\';
    cpath[3] = 0;
  }
  else
    pcpath=NULL;

  if (! GetDiskFreeSpaceEx(pcpath, &a, &b, &c))
         return 1024L*1024L; //dummy  1MByte

  if (total)
    return (unsigned long) (b.QuadPart>>10);
  else
    return (unsigned long) (a.QuadPart>>10);
}

/*---------------------------------------------------------------------------*/

char *memfree (int swapfree)
//*************************************************************************
//
//*************************************************************************
{
  static char ret[20];
  static char ret2[20];
  MEMORYSTATUS st;

  st.dwLength=sizeof(st);

  GlobalMemoryStatus(&st);

  if(swapfree)
  {
    sprintf(ret, "%ld", (st.dwTotalPageFile-st.dwAvailPageFile) >> 10);
    return ret;
  }
  else
  {
    sprintf(ret2, "%ld", (st.dwAvailPhys) >> 10);
    return ret2;
  }
}

/*---------------------------------------------------------------------------*/

void task_idle (int active)
//*************************************************************************
//
//*************************************************************************
{
//static long lasttic=0;
  if (active == 0)
    Sleep(20);
//  if (systic != lasttic)
//  {
  call_l2(active == 2);
//    lasttic = systic;
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

typedef struct  { char *befbuf;
                  char *buffer;
                  HANDLE hProcess;
                  int hStdOutPipe[2];
                  int hStdInPipe[2];
                } shellthread_t;

#define    OUT_BUFF_SIZE 256
#define    READ_HANDLE 0
#define    WRITE_HANDLE 1

/*---------------------------------------------------------------------------*/

void oshell95 (void *p)
//*************************************************************************
//
//*************************************************************************
{
  int retwert = 0;
  int nExitCode = STILL_ACTIVE;
  char tmp_out[20];
  char tmp_err[20];
  shellthread_t *s = (shellthread_t*)p;

  strcpy(tmp_out, time2filename(0));
  strcpy(tmp_err, time2filename(0));
  if (s->befbuf[0])
  {
    char *commandcom = getenv("COMSPEC");
    if (commandcom == NULL)
      commandcom = "command";
    sprintf(s->buffer, "/c %s >%s", s->befbuf, tmp_out);
    freopen(tmp_err, "wt", stderr);
    s->hProcess = (HANDLE) spawnl(P_NOWAIT, commandcom,
                                  commandcom, s->buffer, NULL);
    cdhome();
    freopen("con", "wt", stderr);
    sprintf(s->befbuf+1, "%s %s", tmp_err, tmp_out);
    s->befbuf[0] = 0;
    while (nExitCode == STILL_ACTIVE)
    {
      if (! GetExitCodeProcess(s->hProcess, (unsigned long*) &nExitCode))
        ExitThread(4);
      Sleep(100);
    }
  ExitThread(nExitCode);
  }
  ExitThread(retwert);
}

/*---------------------------------------------------------------------------*/

void oshell32 (void *p)
//*************************************************************************
//
//*************************************************************************
{
  shellthread_t *s = (shellthread_t*)p;
  int hStdOut;
  int hStdIn;
  int hStdErr;
  int nExitCode = STILL_ACTIVE;
  char *cmdline;

  // Create the pipe
  if (_pipe(s->hStdOutPipe, 512, O_BINARY | O_NOINHERIT) == -1 ||
      _pipe(s->hStdInPipe, 512, O_BINARY | O_NOINHERIT) == -1)
    ExitThread(1);
  // Duplicate stdout handle (next line will close original)
  hStdOut = _dup(_fileno(stdout));
  hStdIn  = _dup(_fileno(stdin));
  hStdErr = _dup(_fileno(stderr));
  // Duplicate write end of pipe to stdout handle
  if (_dup2(s->hStdOutPipe[WRITE_HANDLE], _fileno(stdout)) != 0 ||
      _dup2(s->hStdInPipe[READ_HANDLE], _fileno(stdin)) != 0 ||
      _dup2(s->hStdOutPipe[WRITE_HANDLE], _fileno(stderr)) != 0)
    ExitThread(2);
  // Close original write end of pipe
  close(s->hStdOutPipe[WRITE_HANDLE]);
  close(s->hStdInPipe[READ_HANDLE]);
  cmdline = getenv("COMSPEC");
  if (cmdline == NULL)
    cmdline = "cmd.exe";
  if (s->befbuf && s->befbuf[0])
    sprintf(s->buffer, "/c%s", s->befbuf);
  else
    s->buffer[0] = 0;
  // Spawn process
  s->hProcess = (HANDLE) spawnl(P_NOWAIT, cmdline,
                                cmdline, s->buffer, NULL);
  // Duplicate copy of original stdout back into stdout
  if (_dup2(hStdOut, _fileno(stdout)) != 0 ||
      _dup2(hStdIn, _fileno(stdin)) != 0  ||
      _dup2(hStdErr, _fileno(stderr)) != 0)
    ExitThread(3);
  // Close duplicate copy of original stdout
  close(hStdOut);
  close(hStdIn);
  close(hStdErr);
  while (nExitCode == STILL_ACTIVE)
  {
    if (! GetExitCodeProcess(s->hProcess, (unsigned long*) &nExitCode))
      ExitThread(4);
    Sleep(100);
  }
  close(s->hStdOutPipe[READ_HANDLE]);
  close(s->hStdInPipe[WRITE_HANDLE]);
  CloseHandle(s->hProcess);
  //ExitThread(nExitCode);
  _endthread();
}

/*---------------------------------------------------------------------------*/

void kill (void *p)
//*************************************************************************
//
//*************************************************************************
{
  TerminateProcess((HANDLE)p, -1);
  _endthread();
}

/*---------------------------------------------------------------------------*/

int oshell (char *befbuf, shellinput_t input)
//*************************************************************************
//
//*************************************************************************
{
  HANDLE hdl;
  OSVERSIONINFO OsVer;
  shellthread_t s;
  int ProcessExitCode = STILL_ACTIVE;
  int ThreadExitCode = STILL_ACTIVE;
  int nOutRead;
  char buffer[OUT_BUFF_SIZE+1];
  s.befbuf = befbuf;
  s.buffer = buffer;
  s.hProcess = 0;
  OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&OsVer);

  switch (OsVer.dwPlatformId)
  {
    case VER_PLATFORM_WIN32_NT:
      if (! befbuf || ! befbuf[0])
        putf("Type EXIT to return to " STD_BOXHEADER "...\n");
      if ((hdl = (HANDLE)_beginthread(oshell32, 0, (void*)&s)) != 0)
      {
        wdelay(500);
        if (s.hProcess)
        {
          t->childpid=(int) s.hProcess;
          while (ProcessExitCode == STILL_ACTIVE && ThreadExitCode == STILL_ACTIVE)
          {
            if (getvalid())
            {
              getline(buffer, OUT_BUFF_SIZE - 2, 1);
              strcat(buffer, "\n");
              write(s.hStdInPipe[WRITE_HANDLE], buffer, strlen(buffer));
            }
            while (! eof(s.hStdOutPipe[READ_HANDLE]))
            {
              nOutRead = read(s.hStdOutPipe[READ_HANDLE], buffer, OUT_BUFF_SIZE);
              if (nOutRead)
              {
                buffer[nOutRead]=0;
                putf("%s", buffer);
                putflush();
              }
            }
            wdelay(11);
            if (! GetExitCodeProcess(s.hProcess, (unsigned long*) &ProcessExitCode))
              return 4;
            if (! GetExitCodeThread(hdl, (unsigned long*) &ThreadExitCode))
              return 5;
          }
        }
        putv(LF);
      }
      t->childpid=0;
      _beginthread(kill, 0, (void*)s.hProcess);
      break;

    default:

      if ((hdl = (HANDLE)_beginthread(oshell95, 0, (void*)&s)) != 0)
      {
        wdelay(500);
        if (s.hProcess)
        {
          t->childpid=(int) s.hProcess;
          while (ThreadExitCode == STILL_ACTIVE)
          {
            if (! GetExitCodeThread(hdl, (unsigned long*) &ThreadExitCode))
              return 5;
            wdelay(100);
          }
          if (! befbuf[0])
          {
            befbuf++;
            char *s = strchr(befbuf, 32);
            if (s)
            {
              s[0]=0;
              s++;
            }
            if (! access(befbuf, 0))
            {
              fileio_text fio;
              fio.usefile(befbuf);
              fio.doerase();
              fio.tx();
            }
            if (s && !access(s, 0))
            {
              fileio_text fio;
              fio.usefile(s);
              fio.doerase();
              fio.tx();
            }
          }
          t->childpid=0;
        }
      }
  }
  return ThreadExitCode;
}

/*---------------------------------------------------------------------------*/

void setenv (char *lpname, char *lpvalue, int dummy)
//*************************************************************************
//
// replacement for Linux setenv under Win32
//
//*************************************************************************
{
  SetEnvironmentVariable(lpname, lpvalue);
}

/*---------------------------------------------------------------------------*/

char *cpuinfo (void)
//*************************************************************************
//
//*************************************************************************
{
  static char retbuf[256];
  char cputype[40];
  char osname[40];
  char *s = retbuf;

  strcpy(cputype, "??");
  strcpy(osname, "??");
  SYSTEM_INFO si;
  OSVERSIONINFO OsVer;
  OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&OsVer);
  if (OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    GetSystemInfo(&si);
    switch (si.dwProcessorType)
    {
      case PROCESSOR_INTEL_386:
          sprintf(cputype, "I386");
          break;
      case PROCESSOR_INTEL_486:
          sprintf(cputype, "I486");
          break;
      case PROCESSOR_INTEL_PENTIUM:
          sprintf(cputype, "Pentium");
          break;
    }
    s += sprintf(s, "CPU: %s ", cputype);
  }
  if (GetSystemMetrics(SM_SLOWMACHINE))
    s += sprintf(s, "Processor too slow!");

  switch (OsVer.dwPlatformId)
  {
    case VER_PLATFORM_WIN32s:
      sprintf(osname, "WIN32s");
      break;
    case VER_PLATFORM_WIN32_WINDOWS:
      sprintf(osname, "Windows 95");
      break;
    case VER_PLATFORM_WIN32_NT:
      sprintf(osname, "Windows NT");
      break;
  }
  s += sprintf(s, "\nOS: %s V%d.%d Build %d %s",
                  osname, OsVer.dwMajorVersion, OsVer.dwMinorVersion,
                  OsVer.dwBuildNumber, OsVer.szCSDVersion);
  return retbuf;
}

#endif

/*---------------------------------------------------------------------------*/
