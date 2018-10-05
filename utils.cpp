/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  Diverse Utilities
  -----------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19880118 OE3DZW sec in timestr
//19980202 OE3DZW added support for CB-calls (hrx)
//19980203 OE3DZW added tm_isdst -> was not defined before
//19980307 OE3DZW added 7plus-check, added typecast
//19980311 OE3DZW Oscar satellite calls are accepted eg KO23
//19980404 hrx    support for guest-callsigns
//19980408 hrx    added case 'e' to chartomakro() (for checkcount)
//19980423 OE3DZW added returnmailok
//19980505 OE3DZW checkcount now long (was int)
//19980916 OE3DZW 7line-check strikter, lines must not be longer 72 chars
//19981001 OE3DZW added strupr to filename2time (problem under http)
//19981025 OE3DZW added temperature to macros (wx-station only)
//19990110 OE3DZW y2k in get_headertime, makeheader
//19990615 DH3MB  Added getmd2sum, getmd5sum, mdsum2str
//                Moved file_crcthp from mbcrc.c to this file
//19990628 DH3MB  Renamed getmd2/5sum -> file_md2/5sum
//                Adapted line_7p, filemd2/5sum, file_crcthp for
//                new CRC- and MD2/5-classes
//19990701 DH3MB  Fixed memory leak in file_md2sum, file_md5sum
//                Rewrote file_crcthp; data is read blockwise
//19990822 DH3MB  Removed file_crcthp, file_md2sum and file_md5sum
//                Use method readfile() in CRC- and MDx-classes instead!
//20000111 OE3DZW added makro %j -> sysop-prompt
//20000505 DH8YMB CB-callcheck mbcbcallok changed to more cb-callsign-formats
//20000528 DK2UI  new function rubrik_completition()
//20010620 DK2UI  line length limit to expandmakro()
//20021130 DL9CU  mbamateurcallok erweitert, Bugfix autosysop

#include "baycom.h"

/*---------------------------------------------------------------------------*/

char globalpath[80];

#ifdef __DOS16__
void fixpath (char *argv0)
//*************************************************************************
//
//  Stellt den Pfad fest, aus dem das Programm gestartet worden ist
//  und legt ihn in globalpath ab
//
//*************************************************************************
{
  unsigned int i = 0;
  char psave[80];

  strcpy(psave, argv0);
  i = strlen(psave);
  while (i && (psave[i] != '/' && psave[i] != '\\')) i--;
  for ( ; i + 1; i--)
    globalpath[i] = psave[i];
  killbackslash(globalpath);
}
#else
void fixpath(char *)
{
  char buf[80];
  char *s = getenv("BCMHOME");
  if (! s) s = getcwd(buf, 80);
  strcpy(globalpath, s);
  killbackslash(globalpath);
  trace(report, "working dir", "'%s'", globalpath);
  if (! *globalpath || globalpath[strlen(globalpath) - 1] != '/')
    strcat(globalpath, "/");
}
#endif

/*---------------------------------------------------------------------------*/

void subst (char *s, char i, char j)
//*************************************************************************
//
//  Replaces all matching characters by another in a string
//
//  DH3MB: replaced strchr() by strrchr(), so replacing with a 0-byte
//         is now possible
//
//*************************************************************************
{
  char *p;
  while ((p = strrchr(s, i)) != NULL) *p = j;
}

/*---------------------------------------------------------------------------*/

void subst1 (char *s, char i, char j)
//*************************************************************************
//
//  DK2UI: Replaces only first matching characters by another in a string
//
//*************************************************************************
{
  char *p;
  if ((p = strchr(s, i)) != NULL) *p = j;
}

/*---------------------------------------------------------------------------*/

void rm_crlf (char *s)
//*************************************************************************
//
//  Removes cr or linefeed from a line
//
//*************************************************************************
{
  subst1(s, LF, 0);
  subst1(s, CR, 0);
}

/*---------------------------------------------------------------------------*/

int blkill (char *s)
//*************************************************************************
//
//  Removes blank and CR and ,
//  Bsp: printf("%s\n", s+blkill(s)); gibt s ohne fuehrendes Blank/CR/, aus
//
//*************************************************************************
{
  unsigned int i = 0;
  while (s[i] == CR || s[i] == ' ' || s[i] == ',') i++;
  return i;
}

/*---------------------------------------------------------------------------*/

char *skip (char *s)
//*************************************************************************
//
//
//*************************************************************************
{
  if (! s) trace(tr_abbruch, "skip", "NULL pointer");
  if (! *s) return s;
  char *p;
  while (*s == ' ' || *s == ',') s++;
  if ((p = strchr(s, ' ')) != NULL)
  {
    *p++ = 0;
    while (*p == ' ' || *p == ',') p++;
    s = p;
    return s;
  }
  else
    if ((p = strchr(s, ',')) != NULL)
    {
      *p++ = 0;
      while (*p == ' ' || *p == ',') p++;
      s = p;
      return s;
    }
    else
      s += strlen(s);
  return s;
}

/*---------------------------------------------------------------------------*/

void cut_blank (char *s)
//*************************************************************************
//
//  Schneidet Leerzeichen (blank) am Ende ab
//
//*************************************************************************
{
  if (*s)
  {
    unsigned int i = strlen(s);
    while (i && s[i-1] <= ' ') i--;
    s[i] = 0;
  }
}

/*---------------------------------------------------------------------------*/

void cut_blanktab (char *s)
//*************************************************************************
//
//  Schneidet Leerzeichen (blank) und Tabulatoren (tab) am Ende ab
//
//*************************************************************************
{
  if (*s)
  {
    unsigned int i = strlen(s);
    while (i && (s[i-1] == ' ' || s[i-1] == '\t')) i--;
    s[i] = 0;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
void rm_esc (char *s) //dh8ymb
//*************************************************************************
//
//  Removes ESC-Sequenz from s
//
//*************************************************************************
{
  char p[200];
  if (*s)
  {
    unsigned int i = strlen(s);
    unsigned int j = 0;
    while (i)
    {
      if (i>4 && s[i-1] == 'm')
      {
        if (isdigit(s[i-2]))
        {
          if (s[i-3] == '[' && s[i-4] == '')
          { // einstellige Sequenz
          //  printf("einstellig %d\n",i);
            for (j = 0 ; j < strlen(s) ; j++)
            {
              if (j >=(i-4))
                p[j] = s[j+4];
              else
                p[j] = s[j];
            }
            p[strlen(s)-4] = 0;
            safe_strcpy(s, p);
          }
          if (isdigit(s[i-3]) && s[i-4] == '[' && s[i-5] == '')
          { // zweistellige Sequenz
          //  printf("zweistellig %d\n",i);
            for (j = 0 ; j < strlen(s) ; j++)
            {
              if (j >=(i-5))
                p[j] = s[j+5];
              else
                p[j] = s[j];
            }
            p[strlen(s)-5] = 0;
            safe_strcpy(s, p);
          }
        }
      }
      i--;
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/

int nextdez (char **bf)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = -1, a = 0;

  if (! (**bf)) return -1;
  while (**bf && ((**bf < '0') || (**bf > '9'))) (*bf)++;
  if (! (**bf)) return -1;
  i = atoi(*bf);
  while ((**bf >= '0') && (**bf <= '9'))
  {
    (*bf)++;
    a++;
  }
  (*bf) += blkill(*bf);
  if (a) return i;
  else  return -1;
}

/*---------------------------------------------------------------------------*/

char *getheaderadress (char *rline)
//*************************************************************************
//
//  Extrahiert aus einer R: -Headerzeile die H-Adresse
//  der Mailbox, die das File uebertragen hat.
//
//*************************************************************************
{
  static char headercall[HADRESSLEN+1];
  char *ls;

  if (*rline == 'R' && rline[1] == ':')
  {
    ls = strchr(rline, '@');
    if (! ls) ls = strchr(rline, '&'); // wegen AMTOR, kein @
    if (ls)
    {
      if (ls[1] == ':') ls += 2;
      else ls += 1;
      nexttoken(ls, headercall, HADRESSLEN);
      return headercall;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

time_t getheadertime (char *rline)
//*************************************************************************
//
//  Extrahiert aus einer R: -Headerzeile Datum und Uhrzeit, zu dem
//  das File uebertragen worden ist.
//  Rueckgabe im ANSI-Format (UTC). (R:-Zeile hat UTC)
//
//*************************************************************************
{
  struct tm tt;
  char datstr[12];
/* Example
012345678901234567890
R:981012/0043z @:OE3DZW.OE1XAB.#OE1.AUT.EU [OpenBCM] obcm1.05
       '/'   ' '
  yymmdd hhmm
  01234567890
*/
  //tt muss geloescht werden, damit das Ergebnis von mktime stimmt! (deti)
  memset(&tt, 0, sizeof(struct tm));
  if ((rline[8] == '/') && ((rline[13] == ' ') || (rline[14] == ' ')))
  {
    strncpy(datstr, rline + 2, 11);
    tt.tm_isdst = -1;
    //minutes
    datstr[11] = 0;
    tt.tm_min = atoi(datstr + 9);
    //hours
    datstr[9] = 0;
    tt.tm_hour = atoi(datstr + 7);
    //day of month
    datstr[6] = 0;
    tt.tm_mday = atoi(datstr + 4);
    //month
    datstr[4] = 0;
    tt.tm_mon = atoi(datstr + 2) - 1;
    //year
    datstr[2] = 0;
    tt.tm_year = atoi(datstr);
    if (tt.tm_year < 70) tt.tm_year += 100;  //y2k
    if (tt.tm_mday && tt.tm_mon < 12 && tt.tm_year) return ad_mktime(&tt);
  }
  return 0L;
}

/*---------------------------------------------------------------------------*/

long nonlin (long bytes)
//*************************************************************************
//
//  Nichtlineare Darstellung von Byteangaben - Komprimierung
//
//*************************************************************************
#define NONLINMAX 1048576L      // (1L << 20)
{
  if (bytes > NONLINMAX) bytes = NONLINMAX + (bytes >> 10);
  return bytes;
}

/*---------------------------------------------------------------------------*/

long inonlin (long bytes)
//*************************************************************************
//
//  Nichtlineare Darstellung von Byteangaben - Entkomprimierung
//
//*************************************************************************
{
  if (bytes > NONLINMAX) bytes = (bytes - NONLINMAX) << 10;
  return bytes;
}

/*---------------------------------------------------------------------------*/

char *dezasc (long dez, int stellen)
//*************************************************************************
//
//  Wandelt eine Zahl in einen druckbaren String um
//
//*************************************************************************
{
  static byte s[10];

  if (stellen > 9)
  {
    trace(fatal, "dezasc", "too long %d", stellen);
    stellen = 9;
  }
  s[stellen] = 0;
  while (stellen)
  {
    stellen--;
    s[stellen] = (dez & 127) + '!';
    dez >>= 7;
  }
  if (dez) trace(serious, "dezasc", "too short: %d/%d", dez, stellen);
  return (char *) s;
}

/*---------------------------------------------------------------------------*/

long ascdez (char *sc, int stellen)
//*************************************************************************
//
//  Wandelt einen druckbaren String in eine Zahl um
//
//*************************************************************************
{
  long retwert = 0;
  byte *s = (byte *) sc;
  if (stellen > 9)
  {
    trace(fatal, "ascdez", "too long %d:%s", stellen, s);
    stellen = 9;
  }
  while (*s && stellen)
  {
    if (*s < '!')
    {
      trace(serious, "ascdez", "range %s", s);
      return 0;
    }
    retwert <<= 7;
    retwert |= (*s - '!');
    stellen--;
    s++;
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

static int near nibdez (char nibble)
//*************************************************************************
//
//  Macht aus einem Zeichen im Zahlensystem zur Basis 36
//  eine Dezimalzahl (fuer Filenamen).
//
//*************************************************************************
{
  if (nibble >= '0' && nibble <= '9') return nibble - '0';
  else
    if (nibble >= 'A' && nibble <= 'Z') return nibble - ('A' - 10);
    else return 0;
}

/*---------------------------------------------------------------------------*/

char deznib (int dez)
//*************************************************************************
//
//  Erzeugt eine einstellige Zahl zur Basis 36 (fuer Filenamen).
//
//*************************************************************************
{
  if (dez < 10) return dez + '0';
  else return dez + ('A' - 10);
}

/*---------------------------------------------------------------------------*/

unsigned strcrc (char *s)
//*************************************************************************
//
//  Berechnet ueber den gegebenen String eine CRC-Pruefsumme
//  Wird fuer Hashing verwendet
//
//*************************************************************************
{
  short int word = 0;
  unsigned int i;
  byte byt;

  while ((byt = *(s++)) != 0)
  {
    for (i = 0; i < 8; i++)
    {
      if (((word < 0) + (byt < 128)) == 1) word <<= 1;
      else word = (word << 1) ^ 0xa097;
      byt <<= 1;
    }
  }
  return (short unsigned) word;
}

/*---------------------------------------------------------------------------*/

char *stristr (char *s1, char *s2)
//*************************************************************************
//
//  Sucht nach einem Teilstring in einem String.
//  Die Funktion ist analog zur Bibliotheksfunktion strstr,
//  unterscheidet aber nicht zwischen Gross - und Kleinschreibung
//
//*************************************************************************
{
  int i;
  int l = strlen(s2);

  while (s1[0])
  {
    i = 0;
    while ((! ((s1[i] ^ s2[i]) & 0x5f)) && s2[i]) i++;
    if (i == l) return s1;
    s1++;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

int strpos (char *s1, char *s2)
//*************************************************************************
//
//  strpos(String,Substring)
//
//  Gibt die Position von Substring in String zurueck
//  Wird Substring nicht gefunden, oder ist einer der beiden Strings leer,
//  so wird -1 zurueckgegeben.
//
//*************************************************************************
{
  unsigned int i;
  unsigned int l = strlen(s2);
  int pos = 0;

  while (*s1)
  {
    i = 0;
    while ((! ((s1[i] ^ s2[i]) & 0x5f)) && s2[i]) i++;
    if (i == l) return pos;
    s1++;
    pos++;
  }
  return NOTFOUND;
}

/*---------------------------------------------------------------------------*/


char *safestrtok (char *s, const char *delim, char **last)
//*************************************************************************
//
// sollte die Funktion strtok ersetzen, die nicht Threadsicher ist
// Quelle:
// http://www.freebsd.org/cgi/cvsweb.cgi/src/lib/libc/string/strtok.c?rev=1.9
//
//*************************************************************************
{
  char *spanp, *tok;
  int c, sc;

  if (s == NULL && (s = *last) == NULL)
  return (NULL);

  //Skip (span) leading delimiters (s += strspn(s, delim), sort of).
cont:
  c = *s++;
  for (spanp = (char *)delim; (sc = *spanp++) != 0;)
  {
    if (c == sc)
      goto cont;
  }

  if (c == 0)
  { // no non-delimiter characters
    *last = NULL;
    return (NULL);
  }
  tok = s - 1;

  // Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
  // Note that delim must have one NUL; we stop if we see that, too.
  for (;;)
  {
    c = *s++;
    spanp = (char *)delim;
    do
    {
      if ((sc = *spanp++) == c)
      {
        if (c == 0)
          s = NULL;
        else
          s[-1] = '\0';
        *last = s;
        return (tok);
      }
    } while (sc != 0);
  }
  // NOTREACHED
}

/*---------------------------------------------------------------------------*/

char *nexttoken (char *buf, char *token, int maxlen)
//*************************************************************************
//
//  Extrahiert den naechsten alphanumerischen Teilstring aus einem
//  String. Dabei wird entweder blank/tab/@ als Ende betrachtet, oder
//  der String kann mit "Anfuehrungszeichen" eingeklammert sein
//
//*************************************************************************
{
  int i = 0;

  while (*buf == ' ') buf++;
  if (*buf == '"')
  {
    buf++;
    while ((*buf != '"') && (*buf >= ' ') && (i < maxlen))
      token[i++] = *(buf++);
    while (*buf && (*buf) != '"')
      buf++;
    if (*buf == '"') buf++;
  }
  else
  {
    while (*buf != '@' && *buf > ' ' && i < maxlen)
      token[i++] = *(buf++);
    while (*buf && *buf != '@' && *buf != ' ')
      buf++;
  }
  while (*buf == ' ') buf++;
  token[i] = 0;
  return buf;
}

/*---------------------------------------------------------------------------*/

char *nextword (char *buf, char *token, int maxlen)
//*************************************************************************
//
//  Extrahiert den naechsten alphanumerischen Teilstring aus einem
//  String. Dabei wird entweder blank oder NULL als Ende betrachtet.
//
//*************************************************************************
{
  int i = 0;
  char *buf2 = buf; //do not modify argument

  while (*buf2 == ' ') buf2++;
  while (*buf2 > ' ' && i < maxlen) token[i++] = *(buf2++);
  token[i] = 0;
  while (*buf2 == ' ') buf2++;
  return buf2;
}

/*---------------------------------------------------------------------------*/

int parseline (char *s, char *indizes) //df3vi: aus fwd.cpp
//*************************************************************************
//
//  Teilt eine Zeile in einzelne Woerter auf.
//  Rueckgabe: Anzahl der Woerter
//             Array mit den Indizes der einzelnen Woerter
//
//*************************************************************************
{
  int offset = 0, i = 0, word = 0;

  while (*s >= ' ')
  {
    if (*s == ' ' || *s == ',')
    {
      *s = 0;
      word = 0;
    }
    else
    {
      if (! word)
      {
        if ((i + 1) < MAXPARSE) indizes[i++] = offset;
        else break;
      }
      word = 1;
    }
    s++;
    offset++;
  }
  *s = 0;
  indizes[i] = 0;
  return i;
}

/*---------------------------------------------------------------------------*/

char *get_searchitem (char *instr, char *str, int maxlen)
//*************************************************************************
//
//  Extrahiert in einer Befehlszeile einen Suchbegriff in
//  "Anfuehrungszeichen". Dieser wird herauskopiert und gegen Blanks
//  ersetzt. Blanks am Zeilenende werden entfernt.
//
//*************************************************************************
{
  char *qm = strchr(instr, '"');

  if (qm)
  {
    char *end = strchr(qm + 1, '"');
    if (end)
    {
      while (qm != end)
      {
        *qm = ' ';
        qm++;
        if (maxlen && qm != end)
        {
          *str = toupper(*qm);
          str++;
          maxlen--;
        }
      }
      *str = 0;
      *end = ' ';
      while (*instr && instr[strlen(instr) - 1] == ' ')
        instr[strlen(instr) - 1] = 0;
      return str;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

int filecopy (char *src, char *dest)
//*************************************************************************
//
//  Kopiert eine Datei
//  Returnwert:  0 = ok
//              -1 = Fehler
//
//*************************************************************************
{
  handle inf, outf;
  char *buf;
  unsigned z;

  inf = s_open(src, "srb");
  if (inf == EOF)
  {
    s_close(inf);
    return (-1);
  }
  outf = s_open(dest, "swb");
  if (outf == EOF)
  {
    s_close(outf);
    return (-1);
  }
  buf = (char *) t_malloc(CBUFLEN, "copy");
  do
  {
    z = _read(inf, buf, CBUFLEN);
    if (z > 0) _write(outf, buf, z);
    waitfor(e_ticsfull);
  }
  while (z == CBUFLEN);
  t_free(buf);
  s_close(outf);
  s_close(inf);
  return 0;
}

/*---------------------------------------------------------------------------*/

int xunlink (char *name)
//*************************************************************************
//
//  Loescht ein File unter Beruecksichtigung der Mehrfachzugriffe
//  Ggf. wird gewartet bis das File frei ist.
//
//*************************************************************************
{
  while (sema_access(name)) wdelay(107);
  resetreadonly(name);
  return unlink(name);
}

/*---------------------------------------------------------------------------*/

int xrename (char *src, char *dest)
//*************************************************************************
//
//  Verschiebt ein File unter Beruecksichtigung der Mehrfachzugriffe
//  Ggf. wird gewartet bis das File frei ist.
//  Wenn Verschieben nicht moeglich ist wird kopiert und danach geloescht.
//
//*************************************************************************
{
  while (sema_access(src)) wdelay(124);
  while (sema_access(dest)) wdelay(125);
  resetreadonly(src);
  if (rename(src, dest))
  {
    filecopy(src, dest);
    return unlink(src);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

long filesize (char *name)
//*************************************************************************
//
//  Gibt die Groesse eines Files zurueck
//
//*************************************************************************
{
  struct stat st;
  if (! stat(name, &st)) return st.st_size;
  else return 0L;
}

/*---------------------------------------------------------------------------*/

int isdir (char *name)
//*************************************************************************
//
//  Stellt fest, ob der uebergebene Name ein Verzeichnis ist
//
//*************************************************************************
{
  struct stat st;
  if (! strcmp(name + strlen(name) - 2 , ".."))
    return 1; // workaround WIN32 Bug
  if (! stat(name, &st))
    return ((st.st_mode & S_IFDIR) != 0);
  else return 0;
}

/*---------------------------------------------------------------------------*/

time_t filetime (char *name)
//*************************************************************************
//
//  Stellt die Zeit der letzten Modifikation eines Files oder Verzeichnisses
//  fest (wird auch zum Feststellen der Existenz eines Files oder
//  Verzeichnisses verwendet)
//
//*************************************************************************
{
  struct stat st;
  if (! stat(name, &st)) return st.st_mtime;
  else return 0L;
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
#ifdef __LINUX__
 #include <utime.h>
#else   // _Win32
 #include <winbase.h> // FILETIME, DosDateTimeToFileTime, SetFileTime
#endif
#endif

int setfiletime (char *filename, unsigned long dosfiletime)     //dk2ui
//*************************************************************************
//
//*************************************************************************
{
  if (! dosfiletime) return FALSE;
#ifdef __FLAT__
#ifdef __LINUX__
    struct utimbuf ut;
    ut.actime = ut.modtime = getunixtime(dosfiletime);
    if (! utime(filename, &ut)) return TRUE;
    return FALSE;
#else   // _WIN32
    HANDLE fh = OpenFileMapping(FILE_MAP_WRITE, TRUE, filename);
    if (! fh) return FALSE;
    FILETIME wfiletime;
    LPFILETIME wft = &wfiletime;
    if (! DosDateTimeToFileTime (
      (unsigned int)((dosfiletime & 0xffff0000) >> 16),
      (unsigned int) (dosfiletime & 0x0000ffff), wft))
      return FALSE;
    int result = SetFileTime(fh, wft, wft, wft);
    CloseHandle(fh);
    return result;
#endif
#else   // DOS16
    int fh;
    union fzeit
          {
          ftime ft;
          unsigned long fz;
          } fzeit;
    fzeit.fz = dosfiletime;
    fh = s_open(filename, "srb");
    setftime(fh, &fzeit.ft);
    s_close(fh);
    return TRUE;
#endif
}

/*---------------------------------------------------------------------------*/

time_t file_isreg (char *name)
//*************************************************************************
//
//  Stellt die Zeit der letzten Modifikation eines Files fest
//  (wird auch zum Feststellen der Existenz eines Files verwendet)
//
//*************************************************************************
{
  struct stat st;
  if (! stat(name, &st)
#ifdef __LINUX__
      && S_ISREG(st.st_mode))
#else
      && (st.st_mode & S_IFREG))
#endif
    return st.st_mtime;
  else return 0L;
}

/*---------------------------------------------------------------------------*/

char *defext (char *name, char *ext)
//*************************************************************************
//
//  Fuegt eine default extension zu einem Filename, sofern eine solche
//  noch nicht existiert.
//
//*************************************************************************
{
  static char nam2[FNAMELEN-1];
  strlwr(name);
  strlwr(ext);
  if (strchr(name, '.')) return name;
  strcpy(nam2, name);
  strcat(nam2, ".");
  strcat(nam2, ext);
  return nam2;
}

/*---------------------------------------------------------------------------*/

void killbackslash (char *s)
//*************************************************************************
//
//  Macht aus allen Backslashes einen Slash /
//  (Wird verwendet zur Umwandlung von DOS-Filepfaden nach UNIX)
//
//*************************************************************************
{
  while (*s)
  {
    if (*s == '\\') *s = '/';
    s++;
  }
}

/*---------------------------------------------------------------------------*/

void remove_emptyadd (char *s)
//*************************************************************************
//
// DH8YMB
// Entfernt bei HTTPD aus dem TO-Feld ein leeres " @ "
// Bsp: aus "TEST @ " wird "TEST "
//
//*************************************************************************
{
  unsigned int i = 0;
  unsigned int leng = strlen(s);
  while (*s)
  { if (*s == '@' && (i >= leng-2)) *s = 0;
    s++; i++;
  }
}

/*---------------------------------------------------------------------------*/

char jokcmp (char *string, char *jokex, char negotiation)
//*************************************************************************
//
//  Eine einfache Match-Routine (NICHT regex!), die die Verwendung der
//  Jokerzeichen '?' und '*' (auch mitten im String) erlaubt.
//  Stimmt "negotiation" mit dem ersten Zeichen von "jokex" ueberein,
//  so wird der Ausdruck negiert.
//  (DH3MB)
//
//*************************************************************************
{
  char i1 = 0, i2 = 0, neg = 0;

  if (negotiation == *jokex)
  {
    neg = 1;
    jokex++;
  }
  for (;i2 < strlen(jokex); i2++)
  {
    switch (jokex[i2])
    {
    case '?':
      if (i1 == strlen(string)) return neg; // '?' stated, but no char left in string
      i1++;
      break;
    case '*':
      if (i2 == (strlen(jokex) - 1)) return 1 - neg; // '*' at right bound of jokex
      while (jokex[i2] == '*') i2++;
      while (string[i1] != jokex[i2] && jokex[i2] != '?')
      {
        i1++;
        if (i1 == strlen(string)) return neg;
      }
      i1++;
      break;
    default:
      if (string[i1] != jokex[i2]) return 0+neg;
      i1++;
      break;
    }
  }
  if (i1 == strlen(string)) return 1 - neg;
  else return neg;
}

/*---------------------------------------------------------------------------*/

char *makeheader (int full)
//*************************************************************************
//
//  Erzeugt einen Forward-Header
//
//*************************************************************************
{
  static char h[LINELEN + 1];
  int len;

  sprintf(h, "R:%sz @:%s",
             datestr(ad_time() - ad_timezone(), 5), m.boxadress);
  len = strlen(h);
  if (full)
  {
    m.boxheader[LINELEN - len - strlen(BCMLOGO) - strlen(VNUMMER) - 11] = 0;
//    m.boxheader[62 - len] = 0;
    sprintf(h + len, " [%s] " BCMLOGO VNUMMER, m.boxheader);
    if (b->lifetime) sprintf(h + strlen(h), " LT:%03d", b->lifetime);
  }
  else sprintf(h + len, " $:%s", b->bid);
  return h;
}

/*---------------------------------------------------------------------------*/

char *time2filename (time_t unixtime)
//*************************************************************************
//
//  Erzeugt einen neuen Filenamen, abhaengig von der uebergebenen Zeit
//  wird die Funktion in einer Minute 2mal aufgerufen, so wird die laufende
//  Nummer der Nachricht erhoeht
//
//*************************************************************************
{
  static time_t lasttime = 0;
  static byte name[8];
  struct tm *tt;
  static unsigned nextfile = 0;

  if (! unixtime)
  {
    unixtime = ad_time(); // UTC
    while (nextfile == 35 && (unixtime >> 2) == lasttime)
    {
      wdelay(117);
      unixtime = ad_time();
    }
    if ((unixtime >> 2) != lasttime)
    {
      nextfile = 0;
      lasttime = (unixtime >> 2);
    }
    else nextfile++;
  }
  tt = ad_comtime(unixtime); // UTC
  name[0] = deznib(tt->tm_year - 90);
  name[1] = deznib(tt->tm_mon + 1);
  name[2] = deznib(tt->tm_mday);
  name[3] = deznib(tt->tm_hour);
  name[4] = deznib(tt->tm_min >> 1);
  name[5] = deznib((tt->tm_min & 1) * 15 + (tt->tm_sec >> 2));
  name[6] = deznib(nextfile);
  name[7] = 0;
  return (char *) name;
}

/*---------------------------------------------------------------------------*/

time_t filename2time (char *name)
//*************************************************************************
//
//  Extrahiert aus einem Filenamen die Zeit im ANSI-Format in UTC
//  und interpretiert die laufende Nummer als Sekunde
//
//*************************************************************************
{
  struct tm tt;
  byte s[9];

  memset(&tt, 0, sizeof(struct tm));
  if (strlen(name) > 8 || *name == ' ')
  {
    trace(fatal, "f2time", "%s too long", name);
    name[8] = 0;
  }
  strcpy((char *) s, name);
  strupr((char *) s);
  tt.tm_sec  = nibdez(s[6]); // laufende Nummer als Sekunde interpretieren
  tt.tm_min  = nibdez(s[5]) / 15;
  tt.tm_min += nibdez(s[4]) << 1;
  tt.tm_hour = nibdez(s[3]);
  tt.tm_mday = nibdez(s[2]);
  tt.tm_mon  = nibdez(s[1]) - 1;
  tt.tm_year = nibdez(s[0]) + 90;
  tt.tm_isdst = -1; //OE3DZW
  return ad_mktime(&tt); //ANSI time in UTC
}

/*---------------------------------------------------------------------------*/

static near int mbamateurcallok (char *call)
//*************************************************************************
//
//  Prueft einen String, ob ein gueltiges AFU-Call drinsteht
//  wird zur Unterscheidung Info/User verwendet
//  Aber nicht zur Unterscheidung Bulletin/Personal/Ack..
//
//  1..is call 0..is no call
//
//*************************************************************************
{
  int i, j = 0, len = strlen(call);
  for (i = 0; i < len; i++) if (isdigit(call[i])) j++;
  if (j == 1 && isdigit(call[0])) return 0;
  if (j == 2 && isdigit(call[0]) && isdigit(call[1])) return 0;
  // Oscar satellite eg KO23
  if (len == 4 && call[1] == 'O' && isalpha(*call)
      && isdigit(call[2]) && isdigit(call[3]) )
    return 1;
  return (isalpha(call[3])
          && (len < 5 || isalpha(call[4]))
          && (len < 6 || isalpha(call[5]))
          && isalnum(*call)
          && isalnum(call[1])
          && isalnum(call[2])
          && (j > 0) && (j < 3));
}

/*---------------------------------------------------------------------------*/

static near int mbcbcallok (char *call)
//*************************************************************************
//
//  Checks if a call is a valid CB callsign
//  (supports much callsigns)
//  X = alpha
//  # = digit
//*************************************************************************
{
  return
  (   // callformat (XXX###)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isdigit(call[3]) && isdigit(call[4]) && isdigit(call[5])
    ) ||
       // callformat (XX####)
    (isalpha(call[0]) && isalpha(call[1]) && isdigit(call[2]) &&
          isdigit(call[3]) && isdigit(call[4]) && isdigit(call[5])
    ) ||
       // callformat (XXXX##)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isalpha(call[3]) && isdigit(call[4]) && isdigit(call[5])
    ) ||
       // callformat (XXXXX#)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isalpha(call[3]) && isalpha(call[4]) && isdigit(call[5])
    ) ||
       // callformat (XX##XX)
    (isalpha(call[0]) && isalpha(call[1]) && isdigit(call[2]) &&
          isdigit(call[3]) && isalpha(call[4]) && isalpha(call[5])
    ) ||
      // callformat (XXX##X)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isdigit(call[3]) && isdigit(call[4]) && isalpha(call[5])
    ) ||
       // callformat (XXXX#X)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isalpha(call[3]) && isdigit(call[4]) && isalpha(call[5])
    ) ||
      // callformat (XXX#XX)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isdigit(call[3]) && isalpha(call[4]) && isalpha(call[5])
    ) ||
      // callformat (XX#XXX)
    (isalpha(call[0]) && isalpha(call[1]) && isdigit(call[2]) &&
          isalpha(call[3]) && isalpha(call[4]) && isalpha(call[5])
    ) ||
       // callformat (XXX#X)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isdigit(call[3]) && isalpha(call[4])
    ) ||
       // callformat (XX#XX)
    (isalpha(call[0]) && isalpha(call[1]) && isdigit(call[2]) &&
          isalpha(call[3]) && isalpha(call[4])
    ) ||
       // callformat (XX##X)
    (isalpha(call[0]) && isalpha(call[1]) && isdigit(call[2]) &&
          isdigit(call[3]) && isalpha(call[4])
    ) ||
       // callformat (##XX##)
    (isdigit(call[0]) && isdigit(call[1]) && isalpha(call[2]) &&
          isalpha(call[3]) && isdigit(call[4]) && isdigit(call[5])
    ) ||
       // callformat (###XX#)
    (isdigit(call[0]) && isdigit(call[1]) && isdigit(call[2]) &&
          isalpha(call[3]) && isalpha(call[4]) && isdigit(call[5])
    ) ||
       // callformat (SF##)
    ((call[0] == 'S') && (call[1] == 'F') && isdigit(call[2]) &&
          isdigit(call[3])
    ) ||
       // callformat (XXXX#)
    (isalpha(call[0]) && isalpha(call[1]) && isalpha(call[2]) &&
          isalpha(call[3]) && isdigit(call[4])
    )
  );
}
/*---------------------------------------------------------------------------*/

#ifdef _LCF // JJ
int fwdcallok (char *call, int fwdcallformat)
//*************************************************************************
//
// check for forwarding
//
//*************************************************************************
{
 if (fwdcallformat == 1)
   return mbcbcallok(call); //only CB-calls
 if (fwdcallformat == 2)
   return (mbcbcallok(call) || mbamateurcallok(call)); //both
 return mbamateurcallok(call); //default: only amateur-calls
}
#endif

/*---------------------------------------------------------------------------*/

int mbcallok (char *call)
//*************************************************************************
//
//  Checks if a string is a valid callsign
//
//*************************************************************************
{
  if (! *call)
    return 0;
  if (strlen(call) > CALLEN)
    return 0;
  strupr(call);
#ifdef _GUEST
  if (! strcmp(call, m.guestcall) && strcmp(m.guestcall, "OFF"))
    return 1;
#endif
  if (strstr(call, "3DA") == call)
    return 1;
  // 3DA=Swaziland, wird auch in mbhadr.cpp "browse_hadr" ausgewertet!
  if (m.callformat == 1)
    return mbcbcallok(call); //only CB-calls
  if (m.callformat == 2)
    return (mbcbcallok(call) || mbamateurcallok(call)); //both
  return mbamateurcallok(call); //default: only amateur-calls
}

/*---------------------------------------------------------------------------*/

int mbhadrok (char *hadr)
//*************************************************************************
//
//  Prueft einen String, ob er eine gueltige Box-Adresse darstellt
//  Rueckgabe:
//    0  ungueltig (<trash->)
//    1  Mailbox   (<call>.<h-adress>)
//    2  Verteiler (<verteiler>)
//
//*************************************************************************
{
  char tmp[HADRESSLEN+1];

  if (strlen(hadr) > HADRESSLEN)
    return 0; // too long
  if (strlen(hadr) < 2)
    return 0; // to short
  strcpy(tmp, hadr);
  strupr(tmp);
  subst1(tmp, '.', 0); // remove h-address
  if (strstr(tmp, m.boxname))
    subst1(tmp, '-', 0); // remove ssid of own boxaddress
  if (strlen(tmp) > 6)
    return 0; //too long to be a callsign
  if (mbcallok(tmp))
    return 1; //is a valid callsign
  else
    return 2; //is no valid callsign but a valid boxaddress
}

/*---------------------------------------------------------------------------*/

char mbmailt (char *ziel, char *hadr)
//*************************************************************************
//
//  Stellt die Mailtype fest, falls sie nicht angegeben wurde
//  Rueckgabe:
//    'P' personal
//    'B' bulletin
//
//*************************************************************************
{
  if (mbcallok(ziel))
    return 'P';
  else
    return 'B';
/* dh8ymb: wozu?
  char tmp[HADRESSLEN+1];

  strcpy(tmp, hadr);
  strupr(tmp);
  subst1(tmp, '.', 0); // remove h-address
  if (strstr(tmp, m.boxname))
    subst1(tmp, '-', 0); // remove ssid of own boxaddress
  if (strcmp(m.boxname, tmp) && mbcallok(tmp))
    return 'P';
*/
}

/*---------------------------------------------------------------------------*/

int seek_lastentry (handle fh, unsigned long anzahl)
//*************************************************************************
//
//  Positioniert vom Ende einer Liste um 'anzahl' ungeloeschte
//  (bzw bei -v auch geloeschte) Eintraege in einem LIST-File zurueck
//  Rueckgabe 0: Kein passender Eintrag enthalten
//
//*************************************************************************
{
  long len = filelength(fh) >> 7;
  char buf[16];
  unsigned long found = 0;

  while (len--)
  {
    lseek(fh, LBLEN * len, SEEK_SET);
    _read(fh, buf, 16);
    if (buf[14] == '>' || (b->opt & o_v))
    {
      found++;
      if (found == anzahl)
      {
        lseek(fh, LBLEN * len, SEEK_SET);
        return 1;
      }
    }
  }
  lseek(fh, 0L, SEEK_SET);
  return found;
}

/*---------------------------------------------------------------------------*/

time_t seek_fname_all (handle fh, char *fname, char *ok)
//*************************************************************************
//
//  Durchsucht alle Zeilen in einem List-File nach dem uebergebenem
//  Filenamen
//  Rueckgabewert ist Zeitstempel des letzten Eintrags (obsolet)
//                    und *ok
//
//*************************************************************************
{
  lastfunc("seek_fname_all");
  time_t lasttime = 0L;
  long i = 0;
  long len = filelength(fh);

  if (fh == EOF)
  {
    *ok = NO;
    trace(serious, "seek_fname_all", "no file");
      return 0L;                      // kein File offen, Fehler --->
  }
  lseek(fh, -(LBLEN), SEEK_END);      // zum letzten Eintrag
  _read(fh, b->line, BLEN);           // steht auf letztem Eintrag im File
  b->line[7] = 0;
  lasttime = filename2time(b->line);  // letzten (neuesten) Eintrag bestimmen
  if (! strncmp(b->line, fname, 7))
  {
    lseek(fh, -(LBLEN), SEEK_CUR);    // Eintrag genau gefunden, eins
    *ok = OK;                         // zurueck, damit es genau passt
    return lasttime;
  }
  do
  {
    lseek(fh, -(2*LBLEN), SEEK_CUR);
    i = i + LBLEN;
    _read(fh, b->line, BLEN);
    b->line[7] = 0;
//      lasttime = filename2time(b->line);
    if (! strncmp(b->line, fname, 7))
    {
      lseek(fh, -(LBLEN), SEEK_CUR); // Eintrag genau gefunden, eins
      *ok = OK;                      // zurueck, damit es genau passt
      return lasttime;
    }
  }
  while (i < len);
  if (strncmp(b->line, fname, 7) > 0)
   lseek(fh, -(LBLEN), SEEK_CUR);    // nichts gefunden
  *ok = NO;                          // Eintrag nicht gefunden
  return lasttime;
}


/*---------------------------------------------------------------------------*/

time_t seek_fname (handle fh, char *fname, char *ok, int all)
//*************************************************************************
//
//  Sucht in einem List-File nach dem uebergebenem Filenamen
//  Da die Filenamen in der Liste sortiert sind, erfolgt die
//  Suche binaer, d.h. es sind nur ld(listenlaenge)+1 Filezugriffe notwendig
//
//  Wird der Name genau gefunden, so wird fuer den naechsten Lesezugriff
//  auf diesen Positioniert und ok[0]==1 zurueckgegeben.
//  Wird der Name nicht gefunden, so wird im File hinter die Stelle
//  positioniert, an der der Filename bei richtiger Einsortierung stehen
//  wuerde.
//
//*************************************************************************
{
  lastfunc("seek_fname");
  time_t lasttime = 0L;
  int  vgl;
  char found = NO;
  long lastseek;
  long lindex = 1, step, seekindex;

  if (fh == EOF)
  {
    *ok = NO;
    trace(serious, "seek_fname", "no file");
      return 0L;                         // kein File offen, Fehler --->
  }
  lseek(fh, -(LBLEN), SEEK_END);         // zum letzten Eintrag
  lastseek = (unsigned) (lseek(fh, 0L, SEEK_CUR) >> 7);
  while ((lindex - 1) < lastseek) lindex <<= 1; // Filelaenge binaer aufrunden
  step = lindex / 2;                     // Anfangswerte festlegen
  if (step) lindex = step - 1;
  else lindex = 0;
  _read(fh, b->line, BLEN);              // steht auf letztem Eintrag im File
  b->line[7] = 0;
  lasttime = filename2time(b->line);     // neuesten Eintrag bestimmen
  if (! strncmp(b->line, fname, 7))
  {
    lseek(fh, -(LBLEN), SEEK_CUR);       // Eintrag genau gefunden, eins
    *ok = OK;                            // zurueck, damit es genau passt
    return lasttime;
  }
  if (strncmp(b->line, fname, 7) > 0)
  {
    do
    {
      if (lindex > lastseek)
        seekindex = lastseek; // nicht hinter dem Ende lesen
      else
        seekindex = lindex;
      lseek(fh, (long) seekindex << 7, SEEK_SET);
      _read(fh, b->line, BLEN);
//      b->line[7] = 0;
//      lasttime = filename2time(b->line);
      vgl = strncmp(b->line, fname, 7);
      if (vgl < 0)
      {
        step >>= 1;
        lindex += step;
      }
      else if (vgl > 0)
      {
        step >>= 1;
        lindex -= step;
      }
      else
      {
        lseek(fh, -(LBLEN), SEEK_CUR);  // Eintrag genau gefunden, eins
        *ok = OK;                       // zurueck, damit es genau passt
        return lasttime;
      }
    }
    while (step);
    if (strncmp(b->line, fname, 7) > 0) lseek(fh, -(LBLEN), SEEK_CUR);
  }
  if (all)
  {
    // Eintrag nicht gefunden, genauere Suche durchfuehren
    lasttime = seek_fname_all(fh, fname, &found);
    *ok = found;
  }
  else
    *ok = NO;
  return lasttime;
}

/*---------------------------------------------------------------------------*/

/*
time_t seek_fname2 (handle fh, char *fname, char *ok)
// *************************************************************************
//
//  Sucht in einem List-File wie seek_fname, nur das nach einem zweiten
//  Eintrag gesucht wird!
//  Da die Filenamen in der Liste sortiert sind, erfolgt die
//  Suche binaer, d.h. es sind nur ld(listenlaenge)+1 Filezugriffe notwendig
//
//  Wird der Name genau gefunden, so wird fuer den naechsten Lesezugriff
//  auf diesen Positioniert und ok[0]==1 zurueckgegeben.
//  Wird der Name nicht gefunden, so wird im File hinter die Stelle
//  positioniert, an der der Filename bei richtiger Einsortierung stehen
//  wuerde.
//
// *************************************************************************
{
  time_t lasttime = 0L;
  int vgl,eins;
  long lastseek;
  long lindex = 1, step, seekindex;
  eins = 0;

  lastfunc("seek_fname2");
  if (fh == EOF) return 0L;            // kein File offen, Fehler --->
  lseek(fh, -(LBLEN), SEEK_END);      // zum letzten Eintrag
  lastseek = (unsigned) (lseek(fh, 0L, SEEK_CUR) >> 7);
  while ((lindex - 1) < lastseek) lindex <<= 1; // Filelaenge binaer aufrunden
  step = lindex / 2;                   // Anfangswerte festlegen
  if (step) lindex = step - 1;
  else lindex = 0;
  _read(fh, b->line, BLEN);           // steht auf letztem Eintrag im File
  b->line[7] = 0;
  lasttime = filename2time(b->line);  // neuesten Eintrag bestimmen
  if (! strncmp(b->line, fname, 7))
  {
    lseek(fh, -(LBLEN), SEEK_CUR);    // Eintrag genau gefunden, eins
    *ok = OK;                          // zurueck, damit es genau passt
    return lasttime;
  }
  if (strncmp(b->line, fname, 7) > 0)
  {
    do
    {
      if (lindex > lastseek)
        seekindex = lastseek; // nicht hinter dem Ende lesen
      else
        seekindex = lindex;
      lseek(fh, (long) seekindex << 7, SEEK_SET);
      _read(fh, b->line, BLEN);
//      b->line[7] = 0;
//      lasttime = filename2time(b->line);
      vgl = strncmp(b->line, fname, 7);
      if (vgl < 0)
      {
        step >>= 1;
        lindex += step;
      }
      else if (vgl > 0)
      {
        step >>= 1;
        lindex -= step;
      }
      else
      {
        if (eins)
        {
          lseek(fh, -(LBLEN), SEEK_CUR); // Eintrag genau gefunden, eins
          *ok = OK;                       // zurueck, damit es genau passt
          return lasttime;
        }
        else
        {
          step >>= 1;
          lindex -= step;
          eins++;
        }
      }
    } while (step);
    if (strncmp(b->line, fname, 7) > 0) lseek(fh, -(LBLEN), SEEK_CUR);
  }
  *ok = NO;        // Eintrag nicht gefunden, aber File steht
  return lasttime; // nun an der richtigen Position
}
*/

/*---------------------------------------------------------------------------*/

void scanoptions (char *buf)
//*************************************************************************
//
// ScanOptions
//
//*************************************************************************
{
  b->optminus = b->optplus = 0;
  buf += blkill(buf);
  while (*buf == '-')
  {
    do
    {
      if (isalpha(buf[1]))
      {
        long bit = (1L << (toupper(buf[1]) - 'A'));
        if (buf[2] == '-')
        {
          b->optminus |= bit;
          *buf = buf[1] = buf[2] = ' ';
          buf += 1;
        }
        else if (buf[2] == '+')
        {
          b->optplus |= bit;
          *buf = buf[1] = buf[2] = ' ';
          buf += 1;
        }
        else
        {
          b->optplus |= bit;
          *buf = buf[1] = ' ';
        }
      }
      buf += 1;
    }
    while (isalpha(buf[1]));
    buf += 1;
    buf += blkill(buf);
  }
  b->optminus = ( ~ b->optminus);
}

/*---------------------------------------------------------------------------*/

void formoptions (void)
//*************************************************************************
//
// FormOptions
//
//*************************************************************************
{
  if (! (b->optgroup & 1) && b->usermail)
    b->optgroup++;
  b->opt = ((u->opt[b->optgroup] & b->optminus) | b->optplus);
  if (! b->continous)
    b->continous = !! (b->opt & o_c);
}

/*---------------------------------------------------------------------------*/

void findbereich (char *buf)
//*************************************************************************
//
//  Extrahiert aus einem String eine Bereichsangabe in der Form
//
//   z.B.   1-   -5   3-5
//
//  wird eine gueltige Bereichsangabe erkannt, so wird diese aus dem String
//  durch Blanks ersetzt und der Rueckgabewert beinhaltet den Bereich.
//  Bei keiner oder ungueltiger Angabe wird der maximale Bereich
//  zurueckgeliefert.
//
//*************************************************************************
{
  unsigned long beg, end;
  char *first;

  b->ende = MAXBEREICH; // maximal darstellbare Zahl
  b->beginn = 1;
  b->bereich = 0;
  while (*buf)          // ganzen String durchsuchen
  {
    first = buf;
    beg = 0;            // default: von hinten listen ("l -5")
    end = b->ende;
    while (*buf == ' ') buf++;       // Blanks entfernen
    if (*buf > '0' && *buf <= '9')   // nach erster Zahl suchen (1..9)
    {
      beg = atol(buf);
      while (isdigit(*buf)) buf++;   // Zahl ueberspringen
    }
    if (*buf == '-')
    {
      buf++;
      if (*buf > '0' && *buf <= '9') // evtl zweite Zahl
      {
        end = atol(buf);
        while (isdigit(*buf)) buf++;
      }
    }
    else end = beg;
    // Bereich nur uebernehmen wenn
    // - ein Blank oder Zeilenende folgt
    // - ueberhaupt ein Bereich angegeben wurde
    // - die Bereichsgrenzen plausibel sind
    //
    if ((*buf == ' ' || ! *buf) && first != buf && beg <= end)
    {
      b->beginn = beg;
      b->ende = end;
      b->bereich = 1;
      if (*buf)
      {
        while (first != buf)
        {
          *first = ' '; // Bereichsangabe ueberschreiben
          first++;
        }
      }
      else *first = 0;
      break;
    }
    else
    {
      buf++;
      while (*buf && *buf != ' ') buf++;
    }
  }
}

/*---------------------------------------------------------------------------*/

char *chartomakro (char c, char *msm)
//*************************************************************************
//
// Char to Makro
//
//*************************************************************************
{
  lastfunc("chartomakro");
  *msm = 0;

  switch (c)
  {
    case 'a': sprintf(msm, "%d", cpuload()); break;
    case 'b': strcpy(msm, b->prompt); break;
    case 'c': strcpy(msm, b->logincall); break;
    case 'd': strcpy(msm, datestr(ad_time(), 2)); break;
    case 'e': sprintf(msm, "%lu", mbcheckcount()); break;
    case 'f': sprintf(msm, "%d", freeblocks_tnc() / 3); break;
    case 'h': sprintf(msm, "%d", u->helplevel); break;
    case 'i': sprintf(msm, "%s", datestr(b->logintime, 4)); break;
#ifdef _WXSTN
    case '1': //temperature
              if (m.wx.data_valid)
                sprintf(msm, "%3.1fC", m.wx.outdoort);
              else sprintf(msm, "-");
              break;
    case '2': //humidity
              if (m.wx.data_valid)
                sprintf(msm, "%3.1f%%", m.wx.humidity);
              else sprintf(msm, "-");
              break;
    case '3': //air pressure qnh
              if (m.wx.data_valid)
                sprintf(msm, "%3.1fhPa", m.wx.air_pres_qnh);
              else sprintf(msm, "-");
              break;
    case '4': //wind speed
              if (m.wx.data_valid)
                sprintf(msm, "%3.1fm/s", m.wx.windspeed);
              else sprintf(msm, "-");
              break;
#endif
    case 'j':
              if (b->sysop) sprintf(msm, "#"); // sysop-prompt
              else sprintf(msm, ">"); // user-prompt
              break;
    case 'k': sprintf(msm, "%lu", filesize(CHECKNAME) >> 7); break;
    case 'l':
              if (u->lastboxlogin)
                sprintf(msm, "%s", datestr(u->lastboxlogin, 12));
              else
                sprintf(msm, "Never");
              break;
    case 'm': strcpy(msm, m.boxname); break;
    case 'n': strcpy(msm, u->name); break;
    case 'o': sprintf(msm, "%d", listusers(0)); break;
    case 'p': sprintf(msm, "%s",
                       zeitspanne(t->cputics, zs_cpuexact)); break;
    case 'r': strcpy(msm, "\n"); break;
    case 's': strcpy(msm, memfree(0)); break;
    case 't': sprintf(msm, "%s", datestr (ad_time(), 4)); break;
    case 'u': strcpy(msm,
                       zeitspanne(ad_time() - einschaltzeit, zs_runtime));
                      break;
    case 'v': sprintf(msm, "%s", VNUMMER); break;
    case 'w': sprintf(msm, "%ld Bytes", b->rxbytes + b->txbytes); break;
    case '%': strcpy(msm, "%"); break;
    default:  sprintf(msm, "%%%c", c);
  }
  return msm;
}

/*---------------------------------------------------------------------------*/

char *expandmakro (char *s, char *os)
//*************************************************************************
//
// Expandmakro
//
//*************************************************************************
{
  char msm[20];
  unsigned int i = 0;

  while (*s)
  {
    if (*s == '%')
    {
      s++;
      chartomakro(*s, msm);
      if ((i + strlen(msm)) < LINELEN)
      {
        strcpy(os + i, msm);
        i += strlen(msm);
      }
      else break;
    }
    else if (i < LINELEN) os[i++]= *s;
    else break;
    s++;
  }
  os[i] = 0;
  return os;
}

/*---------------------------------------------------------------------------*/

void leerzeile (void)
//*************************************************************************
//
// Leerzeile ausgeben
//
//*************************************************************************
{
  int i = u->lf;
  if (i == 6) //6 means -1 really no linefeeds
    return;
  else
    while (i--) putv(LF);
}

/*---------------------------------------------------------------------------*/

char *atcall (char *at)
//*************************************************************************
//
// Truncate the h-addr from the call and returns the call (with SSID)
//
//*************************************************************************
{
  static char atc[CALLSSID+1];

  strncpy(atc, at, CALLSSID);
  atc[CALLSSID] = 0;
  subst1(atc, '.', 0);
  return atc;
}

/*---------------------------------------------------------------------------*/

int xmkdir (char *path)
//*************************************************************************
//
//  Creates a path, even if several directories have to be created
//
//*************************************************************************
{
  char pathcopy[FNAMELEN+1];
  char *pptr;
  int retwert = 0;

  if (strlen(path) > FNAMELEN)
  {
    trace(fatal, "xmkdir", "path %s too long", path);
    path[FNAMELEN] = 0;
  }
  pptr = pathcopy; // Move the pointer to the first char of the pathname
  do
  {
    strcpy(pathcopy, path);
    strlwr(pathcopy);
    if ((pptr = strchr(pptr, '/')) != NULL)
      *pptr++ = 0; // Move the pointer to the next slash
    if (filetime(pathcopy))
    {
      if (isdir(pathcopy))
      {
        if (! pptr) return 0;
      }
      else
      {
        xunlink(pathcopy); // delete an existing file
        trace(serious, "xmkdir", "file %s erased", pathcopy);
      }
    }
    retwert = mkdir(pathcopy); // we return the last code we got from mkdir
  }
  while (pptr); // redo as long as the path contains slashes
  return retwert;
}

/*---------------------------------------------------------------------------*/

static char decode_7p[256]=
{
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0x0 , 0x1 , 0x2 , 0x3 , 0x4 , 0x5 , 0x6 ,
0x7 , 0x8 , 0xFF, 0x9 , 0xA , 0xB , 0xC , 0xD , 0xE , 0xF ,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0xFF, 0x5D, 0x5E,
0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0xFF, 0x6E, 0xFF, 0x6F, 0x70,
0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84,
0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2,
0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC,
0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0,
0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
0xD5, 0xD6, 0xD7, 0xFF, 0xFF, 0xFF};

/*---------------------------------------------------------------------------*/

int line_7p (char * p)
//*************************************************************************
//
// Check the CRC of a line 7plus line p, returns 1 if ok, else 0
//
// This part of code has been taken from 7plus by Axel, DG1BBQ
// (modificiations by oe3dzw)
//
//*************************************************************************

/*
  ------------------------------------------------------
  DG1BBQ > OE3DZW   16.02.98 18:12 9 Lines 186 Bytes #999 @OE3XSR.#OE3.AUT.EU
  Subj: RE:Verwendung 7plus Source-Code fuer BCM?
  From: DG1BBQ @ DB0VER.#NDS.DEU.EU (Axel)
  To:   OE3DZW @ OE3XSR.#OE3.AUT.EU
  Hallo Dietmar,
  hab' ich nix gegen, solange im Quelltext sowie im Manual daruf hingewiesen
  wird.
  73s, Axel.
  ------------------------------------------------------
*/

{
// Calculate CRC
  crcthp csequence;
  unsigned short int crc_7p;
  unsigned long cs;
  unsigned int i;
  unsigned int slen = strlen(p);

  if (slen < 64 || slen > 71) // line must be at least 64 chars long
     return (0);              // usually 69 chars, +2 extra cr or lf
  if (   ( (char) p[62]) == (char) 0xb0
      && ( (char) p[63]) == (char) 0xb1
      && ( (char) p[64]) == (char) 0xb2) //header-line eg. filename
     return (2);
  for (i = 0; i < 64; i++)
  //crc_calc(csequence, p[i]);
  csequence.update(p[i]);
  csequence.result &= 0x3fff; // strip calculated CRC to 14 bits
  // Get crc from code line
   cs = 0xb640L * decode_7p[(char) p[66]] +
        0xd8L   * decode_7p[(char) p[65]] +
                  decode_7p[(char) p[64]];
   //linenumber = (int) (cs >> 14);    // upper 9 bits are the line number
   crc_7p = (unsigned short int) (cs & 0x3fffL); // lower 14 bits are the CRC
  return (int) (csequence.result == crc_7p);
}

/*---------------------------------------------------------------------------*/

int returnmailok (void)
//*************************************************************************
//
//  Checks if a return mail should be sent or not
//  returns 1 when mail should be send
//
//*************************************************************************
{
  if (   strncmp(b->betreff, "UNKNOWN", 7)
      && strncmp(b->betreff, "HOLD"   , 4)
      && strncmp(b->betreff, "CP "    , 3)
      && strncmp(b->betreff, "New cal", 7)
      && mbcallok(b->herkunft)
      && strcmp(b->herkunft, b->ziel)
      && b->usermail
      && b->mailtype == 'P'
      && ! find_hadr(b->herkunft) )
    return OK;
  else return NO;
}

/*---------------------------------------------------------------------------*/

int autosysop (void) // JJ
//*************************************************************************
//
//  Autosysop, wertet die Datei "asysop.bcm" aus
//  Datei muss mind. 1 Zeile mit Return beinhalten!
//  Format:
//  call uplink downlink
//
//*************************************************************************
{
  int a = 0, i = 0, issysop = 0;
  char *fname;
  char line[255];
  char call[255];
  char uplink[255];
  char downlink[255];
  FILE *f;

  fname = "asysop.bcm";
  if ((f = s_fopen(fname, "lrt")) != NULL)
  {
    while ((a = fgetc(f)) != EOF)
    {
      line[i] = a;
      i++;
      if (a == CR || a == LF)
      {
        line[i] = 0;
        i = sscanf(line, "%s%s%s", call, uplink, downlink);
        if (   ! strcmp(b->logincall, call)
            && (! strcmp(b->uplink, m.boxname)
                && ! strcmp(uplink, "none")
                || ! strcmp(b->uplink, uplink))
            && (! *t->downlink && ! strcmp(downlink, "none")
                || ! strcmp(t->downlink, downlink))
           ) issysop = 1;
        i = 0;
      }
    }
    s_fclose(f);
  }
  if (issysop)
    trace(report, "autosysop", "sysoplink for %s detected", call);
  return issysop;
}

/*---------------------------------------------------------------------------*/

int homeadr (char *adr)
//*************************************************************************
//
// Checks if address given is equal to the address of the local
// bbs. The hierarchical part of the address is ignored
//
// returns: 0 ... not equal
//          2 ... no address given (empty string)
//          1 ... calls are equal
//
//*************************************************************************
{
  // no address was given
  if (! *adr) return 2;
  // calls are equal
  if (! strcmp(atcall(adr), m.boxname)) return 1;
  // not equal
  return 0;
}

/*---------------------------------------------------------------------------*/

void rubrik_completition (char *rubrik)
//*************************************************************************
//
// DK2UI: completition of rubrik names
//
//*************************************************************************
{
  unsigned n, ui, found = FALSE;
  char rname[DIRLEN+1], *cp;

  if ((n = strlen(rubrik)) == 1 || mbcallok(rubrik)) return;
  for (ui = 0; ui < treelen; ui++)
  {
    if ((cp = strchr(tree[ui].name, '/')) != NULL) cp++;
    else cp = tree[ui].name;
    if (strlen(cp) == n && ! strcasecmp(cp, rubrik)) return;
  }
  for (ui = 0; ui < treelen; ui++)
  {
    if ((cp = strchr(tree[ui].name, '/')) != NULL) cp++;
    else cp = tree[ui].name;
    if (strlen(cp) > n && ! strncasecmp(cp, rubrik, n))
    {
      if (found)
      {
        putf(ms(m_too_short));
        *rubrik = 0;
        return;
      }
      else
      {
        found = TRUE;
        strcpy(rname, cp);
      }
    }
  }
  if (found) strcpy(rubrik, rname);
}

/*---------------------------------------------------------------------------*/

unsigned int hd_space (long flen)
//*************************************************************************
//
// Returns Harddisk-Space
//
//*************************************************************************
{
  return (dfree(".", 0) < (unsigned long)((flen >> 10) + HOMEDISKFULL));
}

/*---------------------------------------------------------------------------*/

char interactive (void)
//*************************************************************************
//
// Checks if the terminal has the ability to act interactive
//
//*************************************************************************
{
  if (! (! stricmp(b->uplink, "Import") ||
         ! stricmp(b->uplink, "TELNET") ||
         ! stricmp(b->uplink, "HTTP") ||
         ! stricmp(b->uplink, "POP3") ||
         ! stricmp(b->uplink, "SMTP") ||
         ! stricmp(b->uplink, "NNTP")))
  {
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
