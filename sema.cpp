/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------------
  Semaphoren fuer den gleichzeitigen Zugriff auf Files
  ----------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980223 OE3DZW fixed fsetopt, should not hang when file not open
//19980418 OE3DZW st sema will show option-flag
//19980925 OE3DZW dos reset-read-only only when writing to file

#include "baycom.h"

/*---------------------------------------------------------------------------*/

//
// Gesetzmaessigkeiten:
//
// Read-Zugriffe   -> beliebig viele gleichzeitig, aber nur wenn kein
//                    write-Zugriff laeuft
// Append-Zugriffe -> nur erlaubt, wenn sonst nur read-Zugriffe erfolgen
// Write-Zugriffe  -> nur erlaubt, wenn File sonst gar nicht im Zugriff ist
//
// Normale Zugriffe sind nach einer absehbaren Zeit beendet. D.h. es ist
// sinnvoll, den Programmablauf zu unterbrechen, und zu warten bis die
// Semaphore wieder zurueckgesetzt wird. Dies ist nur bei Zugriffen sinnvoll,
// die NICHT durch eine User-Ein- oder Ausgabe blockiert werden koennen.
//
// Long-Zugriffe koennen beliebig lange dauern, dass heisst es lohnt nicht
// auf das Ende des Zugriffs zu warten. Das ist genau dann der Fall, wenn
// der Zugriff durch eine User-Eingabe oder durch das Vollaufen des
// Sendepuffers blockiert werden kann. Da sowas beliebig lang dauern kann,
// muss die entsprechende Stelle anders verzweigen (Aufschub des Zugriffs
// durch Pufferung (z.B. beim Loeschen) oder Meldung an den Benutzer)
// option: 1=delete file on disconnect, 2=delete after close, 0 close

/*---------------------------------------------------------------------------*/

#define NO_SEMA (-1)
#undef fopen
#undef fclose

/*---------------------------------------------------------------------------*/

 typedef enum
{ s_read,
  s_longread,
  s_append,
  s_longappend,
  s_write,
  s_longwrite,
  s_maxmodes
} sema_mode;

/*---------------------------------------------------------------------------*/

typedef struct
{
#ifdef __FLAT__
  char filename[FSPATHLEN + FNAMELEN + 1];
#else
  char filename[FNAMELEN + 1];
#endif
  char mode[s_maxmodes];
  char option; // DH3MB: 1=delete file on disconnect, 2=delete after close
} sema_typ;

/*---------------------------------------------------------------------------*/

typedef struct
{
#ifdef __FLAT__
  char name[FSPATHLEN + FNAMELEN + 1];
#else
  char name[FNAMELEN + 1];
#endif
  int  tid;
} lock_typ;

/*---------------------------------------------------------------------------*/

typedef int sema_handle;

/*---------------------------------------------------------------------------*/

static sema_typ    sema[SEMA_MAX];
static sema_handle file_sema[_NFILE_];
static sema_mode   file_mode[_NFILE_];
static int         file_tid[_NFILE_];
static FILE       *file_ptr[_NFILE_];
static lock_typ    lock_sema[LOCK_MAX];
static int         hopens = 0;
static int         fopens = 0;
static int         hopens_max = 0;
static int         fopens_max = 0;

/*---------------------------------------------------------------------------*/

static sema_handle near sema_find (char *filename)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  for (i = 0; i < SEMA_MAX; i++)
#ifdef __UNIX__ // DH3MB
    if (! strcmp(sema[i].filename, filename))
#else
    if (! stricmp(sema[i].filename, filename))
#endif
      return i;
  return NO_SEMA;
}

/*---------------------------------------------------------------------------*/

static sema_handle near sema_new (void)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  for (i = 0; i < SEMA_MAX; i++)
    if (! sema[i].filename[0]) return i;
  return NO_SEMA;
}

/*---------------------------------------------------------------------------*/

static sema_handle near sema_set (char *filename, sema_mode mode)
//****************************************************************************
//
//****************************************************************************
{
  sema_typ *s;
  handle shandle;

  shandle = sema_find(filename);
  if (shandle == NO_SEMA)
  {
    shandle = sema_new();
    if (shandle == NO_SEMA)
    {
      trace(tr_abbruch, "sema_set", "semas full");
      return NO_SEMA;
    }
  }
  s = &sema[shandle];
  if (s->mode[s_longwrite])
  {
    //trace(replog, "sema_set", "longwrite %s", filename);
    return NO_SEMA;
  }
  switch (mode)
  {
  case s_read:
  case s_longread:
    while (s->mode[s_write] || s->mode[s_append])
    {
#ifdef _DEBUG_SEMA
      trace(report, "sema_set", "longread %s", filename);
#endif
      if (gettaskid() == NOTASK) return NO_SEMA;
      wdelay(528);
    }
    break;
  case s_append:
  case s_longappend:
  case s_write:
  case s_longwrite:
    if (s->mode[s_longappend])
    {
      trace(report, "sema_set", "longappend %s", filename);
      return NO_SEMA;
    }
    while (s->mode[s_write] || s->mode[s_append])
    {
      // trace(report, "sema_set", "wrd %s", filename);
      if (gettaskid() == NOTASK) return NO_SEMA;
      wdelay(529);
    }
    break;
  default: break;
  }
  if (! *s->filename) strcpy(s->filename, filename);
  s->mode[mode]++;
  if (shandle == NO_SEMA)
    trace(replog, "sema_set", "no sema %s", filename);
  return shandle;
}

/*---------------------------------------------------------------------------*/

int sema_access (char *name)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  if (! name) return 0;
  for (i = 0; i < SEMA_MAX; i++)
#ifdef __UNIX__ // DH3MB
    if (! strcmp(sema[i].filename, name))
#else
    if (! stricmp(sema[i].filename, name))
#endif
    {
      if (sema[i].mode[s_longwrite] || sema[i].mode[s_write]) return 3;
      if (sema[i].mode[s_longappend] || sema[i].mode[s_append]) return 2;
      else return 1;
    }
  return 0;
}

/*---------------------------------------------------------------------------*/

//static int sema_send (void)
//****************************************************************************
//
//****************************************************************************
/*
{
  unsigned int i;
  for (i = 0; i < SEMA_MAX; i++) if (sema[i].mode[s_longwrite]) return 1;
  return 0;
}
*/

/*---------------------------------------------------------------------------*/

static void near sema_clr (sema_handle shandle, sema_mode mode)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;
  if (shandle < SEMA_MAX && shandle >= 0 && sema[shandle].filename[0])
  {
    if (sema[shandle].mode[mode])
      sema[shandle].mode[mode]--;
    else
      trace(serious, "s_clr", "%s: mode %d not set",
                               sema[shandle].filename, mode);
    for (i = 0; i < 6; i++)
      if (sema[shandle].mode[i]) return;
    memset(&sema[shandle], 0, sizeof(sema_typ));
  }
  else
    trace(fatal, "s_clr", "%d: no such handle", shandle);
}

/*---------------------------------------------------------------------------*/

static sema_mode near s_fmode (char *mode)
//****************************************************************************
//
//****************************************************************************
{
  sema_mode smode;

  smode = (sema_mode) NO_SEMA;
  switch (mode[1])
  {
    case 'r': if (mode[2] != '+') { smode = s_read; break; }
    case 'w': smode = s_write; break;
    case 'a': smode = s_append; break;
    default:  goto error;
  }
  switch (mode[0])
  {
    case 'l': smode = (sema_mode) (smode + 1);
    case 's': break;
    default:  goto error;
  }
  return smode;
error: trace(tr_abbruch, "s_fmode", "mode syntax %s", mode);
  return smode;
}

/*---------------------------------------------------------------------------*/

FILE *s_fopen (char *fname, char *mode)
//****************************************************************************
//
//****************************************************************************
{
  sema_mode smode;
  sema_handle sh;
  FILE *f;
  handle fh;

  f = NULL;
  if (! *fname)
  {
    trace(serious, "s_fopen", "fname empty");
    return NULL;
  }
  if (! stristr(mode, "r") || stristr(mode, "r+"))
    resetreadonly(fname);
  smode = s_fmode(mode);
  sh = sema_set(fname, smode);
  if (sh == NO_SEMA)
  {
    if (b) b->semalock = 1;
  }
  else
  {
    if (b) b->semalock = 0;
    f = _fsopen(fname, mode + 1, SHAREMODE);
    if (f)
    {
      fh = fileno(f);
      if (fh < 0 || fh > _NFILE_ || file_sema[fh] != NO_SEMA)
      {
        trace(fatal, "s_fopen", "handle busy %s", fname);
        fclose(f);
        return NULL;
      }
      file_sema[fh] = sh;
      file_mode[fh] = smode;
      file_tid[fh] = gettaskid();
      file_ptr[fh] = f;
      fopens++;
      if (fopens > fopens_max) fopens_max = fopens;
    }
    else
      sema_clr(sh, smode);
  }
  return f;
}

/*---------------------------------------------------------------------------*/

int s_set (handle fh, char *fname, char *mode)
//****************************************************************************
//
//****************************************************************************
{
  sema_mode smode;
  sema_handle sh;

  smode = s_fmode(mode);
  sh = sema_set(fname, smode);
  if (sh == NO_SEMA)
  {
    if (b) b->semalock = 1;
    trace(report, "s_set", "semalock %s", fname);
  }
  else
  {
    if (b) b->semalock = 0;
    if (fh < 0 || fh > _NFILE_ || file_sema[fh] != NO_SEMA)
    {
      trace(serious, "s_set", "handle busy: %d:%s", fh, fname);
      trace(serious, "s_set", "old: %d:%s",
                               file_sema[fh], sema[file_sema[fh]].filename);
      return EOF;
    }
    file_sema[fh] = sh;
    file_mode[fh] = smode;
    file_tid[fh] = gettaskid();
    hopens++;
    if (hopens > hopens_max) hopens_max = hopens;
  }
  if (b) return b->semalock;
  else return 0;
}

/*---------------------------------------------------------------------------*/

handle s_open (char *fname, char *mode)
//****************************************************************************
//
//****************************************************************************
{
  sema_mode smode;
  sema_handle sh;
  handle fh;

  fh = EOF;
  if (! *fname)
  {
    trace(serious, "s_open", "fname empty");
    return EOF;
  }
  if (! stristr(mode, "r") || stristr(mode, "r+")) resetreadonly(fname);
  smode = s_fmode(mode);
  sh = sema_set(fname, smode);
  if (sh == NO_SEMA)
  {
    if (b)
      b->semalock = 1;
  }
  else
  {
    if (b)
      b->semalock = 0;
    switch (smode)
    {
      case s_read:
      case s_longread:
        fh = sopen(fname, O_RDONLY | O_BINARY, SHAREMODE); break;
      case s_append:
      case s_longappend:
        fh = sopen(fname, O_APPEND | O_BINARY | O_RDWR, SHAREMODE);
        if (fh != EOF)
          lseek(fh, 0L, SEEK_END); // Workaround for bug of Borland C++ 3.1
        break;
      case s_write:
      case s_longwrite:
        if (mode[2] == '+')
        {
          fh = sopen(fname, O_RDWR | O_BINARY, SHAREMODE);
          if (fh == EOF)
          {
            if (file_isreg(fname))
              trace(fatal, "s_open", "EOF, but file exists: %s", fname);
            fh = _creat(fname, CREATMODE);
            if (fh != EOF)
            { _close(fh);
              fh = sopen(fname, O_RDWR | O_BINARY, SHAREMODE);
            }
          }
        }
        else
#ifdef _WIN32
         fh = sopen(fname,
                O_RDWR | O_BINARY | O_TRUNC | O_CREAT, SHAREMODE, CREATMODE);
#else
         fh = _creat(fname, CREATMODE);
#endif
      default: break;
    }
    if (fh != EOF)
    {
      if (fh < 0 || fh > _NFILE_ || file_sema[fh] != NO_SEMA)
      {
        trace(fatal, "s_open", "handle busy %s", fname);
        _close(fh);
        return EOF;
      }
      file_sema[fh] = sh;
      file_mode[fh] = smode;
      file_tid[fh] = gettaskid();
      hopens++;
      if (hopens > hopens_max) hopens_max = hopens;
    }
    else sema_clr(sh, smode);
  }
  return fh;
}

/*---------------------------------------------------------------------------*/

void s_fsetopt (FILE *f, char opt) // DH3MB
//****************************************************************************
//
//****************************************************************************
{
  if (! f)
    trace(fatal, "s_fsetopt", "%c, no file", opt); //this should never happen
  s_setopt(fileno(f), opt);
}

/*---------------------------------------------------------------------------*/

void s_setopt (handle fh, char opt) // DH3MB
//****************************************************************************
//
//****************************************************************************
{
  sema_handle sh = file_sema[fh];
  if (sh == NO_SEMA)
  {
    trace(serious, "s_setopt", "no sema set");
    return;
  }
  if (opt > sema[sh].option) sema[sh].option = opt;
}

/*---------------------------------------------------------------------------*/

void s_fclose (FILE *f)
//****************************************************************************
//
//****************************************************************************
{
#ifdef __FLAT__
  char fname[FSPATHLEN + FNAMELEN + 1];
#else
  char fname[FNAMELEN + 1];
#endif
  strcpy(fname, "");
  if (! f)
    trace(serious, "s_fclose", "file not open");
  else
  {
    handle fh = fileno(f);
    if (file_sema[fh] == NO_SEMA)
      trace(serious, "s_fclose", "no sema set");
    else
    {
      if ((errno = fclose(f)) != 0)
        trace(serious, "s_fclose", "fclose %s errno=%d",
                        sema[file_sema[fh]].filename, errno);
      if (sema[file_sema[fh]].option == 2)
        strcpy(fname, sema[file_sema[fh]].filename);
      sema_clr(file_sema[fh], file_mode[fh]);
      file_sema[fh] = NO_SEMA;
      file_mode[fh] = (sema_mode) NO_SEMA;
      file_tid[fh] = NOTASK;
      file_ptr[fh] = NULL;
      if (fopens) fopens--;
      else trace(serious, "s_fclose", "fopens=0");
      if (*fname) xunlink(fname);
    }
  }
}

/*---------------------------------------------------------------------------*/

void s_close (handle fh)
//****************************************************************************
//
//****************************************************************************
{
#ifdef __FLAT__
  char fname[FSPATHLEN + FNAMELEN + 1];
#else
  char fname[FNAMELEN + 1];
#endif
  strcpy(fname, "");
  if (fh == EOF)
    trace(serious, "s_close", "file not open");
  else
  {
    if (file_sema[fh] == NO_SEMA)
      trace(serious, "s_close", "no sema set");
    else
    {
      if (_close(fh))
        trace(serious, "s_close", "_close");
      if (sema[file_sema[fh]].option == 2)
        strcpy(fname, sema[file_sema[fh]].filename);
      sema_clr(file_sema[fh], file_mode[fh]);
      file_sema[fh] = NO_SEMA;
      file_mode[fh] = (sema_mode) NO_SEMA;
      file_tid[fh] = NOTASK;
      if (hopens) hopens--;
      else trace(serious, "s_close", "hopens=0");
      if (*fname) xunlink(fname);
    }
  }
}

/*---------------------------------------------------------------------------*/

void close_task (int tid)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;
#ifdef __FLAT__
  char fname[FSPATHLEN + FNAMELEN + 1];
#else
  char fname[FNAMELEN + 1];
#endif
  strcpy(fname, "");
  for (i = 0; i < _NFILE_; i++)
    if (file_tid[i] == tid)
    {
      if (file_ptr[i])
      {
        if (sema[file_sema[i]].option == 1)
          strcpy(fname, sema[file_sema[i]].filename);
        s_fclose(file_ptr[i]);
      }
      else
      {
        if (sema[file_sema[i]].option == 1)
          strcpy(fname, sema[file_sema[i]].filename);
        s_close(i);
      }
      if (*fname) xunlink(fname);
    }
  for (i = 0; i < LOCK_MAX; i++)
  {
    if (lock_sema[i].tid == tid)
    {
      lock_sema[i].name[0] = 0;
      lock_sema[i].tid = NOTASK;
    }
  }
}

/*---------------------------------------------------------------------------*/

void sema_list (void)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i, j;
  sema_typ *s;

  putf("#   Rd LRd  Ap LAp  Wr LWr  O Name\n");
  for (i = 0; i < SEMA_MAX; i++)
  {
    s = &sema[i];
    if (*s->filename)
    {
      putf("%-3d", i);
      for (j = 0; j < 6; j++) putf("%2d  ", s->mode[j]);
      putf(" %d", s->option);
      putf(" %s", s->filename);
      putv(LF);
    }
  }
  putf("\nfopens=%d (max=%d), hopens=%d (max=%d).\n",
       fopens, fopens_max, hopens, hopens_max);
  for (i = 0; i < LOCK_MAX; i++)
  {
    if (lock_sema[i].name[0])
      putf("tid=%d lock=\"%s\"\n", lock_sema[i].tid, lock_sema[i].name);
  }
}

/*---------------------------------------------------------------------------*/

void openfile_list (void)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;
  static char *mm[] = { "Rd", "LRd", "Ap", "LAp", "Wr", "LWr"};

  putf("Desc P Sema Mode Task Owner    Name\n");
  for (i = 0; i < _NFILE_; i++)
  {
    if (file_sema[i] != NO_SEMA)
    {
      putf("%3d  %c%4d%5s%5d  %-8.8s %s\n",
           i,
           file_ptr[i] ? '*' : ' ',
           file_sema[i],
           mm[file_mode[i]],
           file_tid[i],
           gettaskname(file_tid[i]),
           sema[file_sema[i]].filename);
    }
  }
  putf("\nNFILE=%d SEMA_MAX=%d\n", _NFILE_, SEMA_MAX);
}

/*---------------------------------------------------------------------------*/

int sema_lock (char *name)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  for (i = 0; i < LOCK_MAX; i++)
    if (! stricmp(lock_sema[i].name, name)) return 0;
  for (i = 0; i < LOCK_MAX; i++)
    if (lock_sema[i].name[0] == 0)
    {
      strcpy(lock_sema[i].name, name);
      lock_sema[i].tid = gettaskid();
      return 1;
    }
  return 0;
}

/*---------------------------------------------------------------------------*/

int sema_test (char *name)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  for (i = 0; i < LOCK_MAX; i++)
    if (! stricmp(lock_sema[i].name, name)) return 1;
  return 0;
}

/*---------------------------------------------------------------------------*/

void sema_unlock (char *name)
//****************************************************************************
//
//****************************************************************************
{
  int tid = gettaskid();
  unsigned int i;

  for (i = 0; i < LOCK_MAX; i++)
    if (! stricmp(lock_sema[i].name, name) && lock_sema[i].tid == tid)
    {
      lock_sema[i].name[0] = 0;
      lock_sema[i].tid = NOTASK;
    }
}

/*---------------------------------------------------------------------------*/

void sema_init (void)
//****************************************************************************
//
//****************************************************************************
{
  unsigned int i;

  memset(sema, 0, SEMA_MAX * sizeof(sema_typ));
  for (i = 0; i < _NFILE_; i++)
  {
    file_sema[i] = NO_SEMA;
    file_mode[i] = (sema_mode) NO_SEMA;
    file_tid[i] = NOTASK;
    file_ptr[i] = NULL;
  }
  for (i = 0; i < LOCK_MAX; i++)
  {
    lock_sema[i].name[0] = 0;
    lock_sema[i].tid = NOTASK;
  }
}

/*---------------------------------------------------------------------------*/
