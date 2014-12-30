/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------------------------
  Zugriff auf persoenliche Einstellungen der Mailboxuser
  ------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980404 hrx    added guest-support
//19980408 OE3DZW added check for more strings
//19980427 OE3DZW deleted users will not be logged as error but deleted,
//                added check for newcall
//19980506 OE3DZW added check for fdelay,fhold,rlimit
//19980615 OE3DZW only every 1000th user is put into log
//20000716 DK2UI  change shadow variables and make code better readable

#include "baycom.h"

/*---------------------------------------------------------------------------*/

static handle uf = EOF;
static handle uhf = EOF;
static time_t lastreorg = 0;
static int is_null = 0;
static user_t us_cache[US_CACHESZ];

/*---------------------------------------------------------------------------*/


static void near userhashfile_newformat(void);
#ifdef _USERS4CONVERT
static void near usersfile_newformat(void);
#endif

void hashinit (handle fhu, long hashsize)
//*************************************************************************
//
//  Wenn die Hashtabelle nicht existiert, wird sie hier angelegt
//  und mit Null aufgefuellt.
//
//*************************************************************************
{
  if (fhu != EOF)
  {
    if (lseek(fhu, 0L, SEEK_END) != hashsize * 2)
    {
      char *buffer = (char *) t_malloc(16384, "hshi");
      unsigned int i;
      memset(buffer, 0, 16384);
      for (i = 0; i < (unsigned) (hashsize / 8192); i++)
        _write(fhu, buffer, 16384);
      t_free(buffer);
    }
  }
  else
    trace(tr_abbruch, "hashinit", "hash not open");
}

/*---------------------------------------------------------------------------*/

static void us_close (void)
//*************************************************************************
//
//  schliesst ggf ein offenes user-file
//
//*************************************************************************
{
  if (uf != EOF)
  {
    s_close(uf);
    uf = EOF;
  }
  if (uhf != EOF)
  {
    s_close(uhf);
    uhf = EOF;
  }
}

/*---------------------------------------------------------------------------*/

void defaultuser (char *usercall, user_t *uu)
//*************************************************************************
//
//  Ueberschreibt einen vorhandenen User mit Defaultwerten
//  Eine ggf. vorhandene Hash-Verkettung wird beibehalten.
//
//*************************************************************************
{
  unsigned int i;
  time_t lastload = uu->lastload;
  unsigned long uhsh = uu->nextsamehash;
  unsigned long fpos = uu->filepos;
  unsigned short statuu = uu->status;

  memset(uu, 0, sizeof(user_t)); // neuen Eintrag vorbesetzen
  uu->nextsamehash = uhsh;
  uu->filepos = fpos;
  uu->status = statuu;
  //                    ^ aber filepos und nextsamehash lassen
  strcpy(uu->call, usercall); // Rufzeichen hineinschreiben
#ifdef _GUEST
  if (! strcmp(uu->call, m.guestcall)) strcpy(uu->mybbs, m.boxname);
#endif
  strcpy(uu->prompt, m.prompt);
  strcpy(uu->sprache, msg_landsuch(usercall));
  strcpy(uu->firstcmd, m.firstcmd);
  uu->helplevel = m.helplevel;
  uu->zeilen = m.zeilen;
  uu->lf = m.deflf;
  uu->status = m.defstatus;
#ifdef FBBCHECKREAD
  uu->fbbcheckmode = m.deffbbcheckmode;
#endif
  uu->fhold = m.defhold;
  uu->paclen = m.paclen;
  uu->readlock = m.defrea;
  uu->httpsurface = m.defhttpsurface;
  for (i = 0; i < 8; i++) uu->opt[i] = m.opt[i];
  uu->lastload = lastload; // damits abgespeichert wird
  us_deletecache();
}

/*---------------------------------------------------------------------------*/

static void newuserfile (void)
//*************************************************************************
//
//  Legt ein neues File mit Userdaten an.
//  Darf nicht aufgerufen werden, wenn nicht sichergestellt ist,
//  dass das alte File ueberschrieben werden soll.
//
//*************************************************************************
{
  user_t ux;

  xunlink(USERHASHNAME);
  trace(serious, "us_open", "new users4.bcm created");
  uf = s_open(USERNAME, "sw+b");
  if (uf == EOF) trace(tr_abbruch, "us_open", "can't create");
  memset(&ux, 0, sizeof(user_t));
  defaultuser("DUMMY", &ux);
  _write(uf, &ux, sizeof(user_t));
  s_close(uf);
}

/*---------------------------------------------------------------------------*/

static void us_open (void)
//*************************************************************************
//
//  Oeffnet ein vorhandenes Userfile.
//  Beim ersten Aufruf wird es ggf. angelegt, wenn sichergestellt
//  ist, dass es nicht existiert.
//
//*************************************************************************
{
  static int first = 1;

#ifdef _USERS4CONVERT
  usersfile_newformat();
#endif
  if (uf == EOF)
  {
    userhashfile_newformat();
    if (! file_isreg(USERNAME) || ! filesize(USERNAME))
    {
      if (! first)
        trace(tr_abbruch, "us_open", "users not available");
      xunlink(USERHASHNAME);
      newuserfile();
      uf = s_open(USERNAME, "sr+b");
    }
    else
    {
      uf = s_open(USERNAME, "sr+b");
      if (uf == EOF)
        trace(tr_abbruch, "us_open", "user open errno=%d", errno);
    }
  }
  if (uhf == EOF)
    userhashfile_newformat();
  uhf = s_open(USERHASHNAME, "sr+b");
  if (uhf == EOF)
  {
    if (first) uhf = s_open(USERHASHNAME, "sw+b");
    else trace(tr_abbruch, "us_open", "hash fopen errno=%d", errno);
  }
  first = 0;
}

/*---------------------------------------------------------------------------*/

static unsigned newuser (user_t *uu, handle fusr, handle fhu)
//*************************************************************************
//
//  legt einen USER im User-File ab
//
//  Achtung: es muss an der Stelle sichergestellt sein, dass nicht
//           schon ein USER mit gleichem Namen existiert!
//
//*************************************************************************
{
  long unsigned hsh = strcrc(uu->call);
  long unsigned idx = 0;

  lseek(fhu, (long) hsh * 4L, SEEK_SET);       // Eintrag im Hashfile
  if (! _read(fhu, &idx, sizeof(long unsigned)))
  {
    trace(serious, "newuser", "read hash %s", uu->call);
    return NO;
  }
  uu->nextsamehash = idx;                      // verketten
  lseek(fusr, 0L, SEEK_END);
  uu->filepos = (long unsigned) (filelength (fusr) / sizeof(user_t));
  lseek(fhu, (long) hsh * 4L, SEEK_SET);
  if (! _write(fhu, &(uu->filepos), sizeof(long unsigned))) //zurueckschreiben
  {
    trace(serious, "newuser", "write hash %s", uu->call);
    return NO;
  }
  if (! _write(fusr, uu, sizeof(user_t)))      // User hinausschreiben
  {
    trace(serious, "newuser", "write data %s", uu->call);
    return NO;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static unsigned long udrin (char *usercall, handle fusr, handle fhu)
//*************************************************************************
//
//  Prueft, ob ein USER mit dem angegebenen Call schon
//  vorhanden ist.
//
//*************************************************************************
{
  long unsigned hsh = strcrc(usercall);
  long unsigned idx;
  user_t uu;

  lseek(fhu, (long) hsh * 4L, SEEK_SET);
  _read(fhu, &idx, sizeof(long unsigned));
  if (idx)
  {
    int num = 0;
    while (idx)
    {
      waitfor(e_ticsfull);
      lseek(fusr, (long) idx * sizeof(user_t), SEEK_SET);
      if (!_read(fusr, &uu, sizeof(user_t)))
      {
        trace(serious, "udrin", "read %08X %08X %s %d", idx, hsh,
                                                        usercall, num);
        return 0UL;
      }
      if ((num++) > 200) // zyklische Verkettung, abbrechen
      {
        trace(serious, "udrin", "loop %08X %08X %s", idx, hsh, usercall);
        return 0UL;
      }
      if (! strcmp(usercall, uu.call)) return idx;
      idx = uu.nextsamehash;
    }
  }
  return 0UL;
}

/*---------------------------------------------------------------------------*/

int us_readcache (char *usercall, user_t *uu)
//*************************************************************************
//
//  Sucht einen Eintrag im cache und holt ihn
//
//*************************************************************************
{
  unsigned int i;

  for (i = 0; i < US_CACHESZ; i++)
  {
    if (! strcmp(usercall, us_cache[i].call))
    {
      memcpy(uu, &us_cache[i], sizeof(user_t));
      return OK;
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

void us_putcache (user_t *uu)
//*************************************************************************
//
//  Legt einen Eintrag im Cache ab oder ggf. aktualisiert ihn
//
//*************************************************************************
{
  unsigned int i;
  static int put_pos = 0;

  for (i = 0; i < US_CACHESZ; i++)
  {
    if (! strcmp(uu->call, us_cache[i].call))
    {
      memcpy(&us_cache[i], uu, sizeof(user_t));
      return;
    }
  }
  memcpy(&us_cache[put_pos], uu, sizeof(user_t));
  put_pos = (put_pos + 1) % US_CACHESZ;
}

/*---------------------------------------------------------------------------*/

void us_deletecache (void)
//*************************************************************************
//
//  Loescht alle Cache-Eintraege (nach reorg)
//
//*************************************************************************
{
  unsigned int i;

  for (i = 0; i < US_CACHESZ; i++)
    us_cache[i].call[0] = 0;
}

/*---------------------------------------------------------------------------*/

int loaduser (char *usercall, user_t *uu, int anlegen)
//*************************************************************************
//
//  Laedt die Benutzerdaten fuer ein bestimmtes Call.
//  Ggf. wird ein neuer Eintrag erzeugt.
//
//*************************************************************************
{
  int retwert = NO;
  long unsigned idx;
  long unsigned hsh;
  int num;

  num = 0;
  strupr(usercall);
  if (us_readcache(usercall, uu))
    retwert = OK;
  else
  {
    hsh = strcrc(usercall);
    us_open();
    lseek(uhf, (long) hsh * 4L, SEEK_SET);
    _read(uhf, &idx, sizeof(long unsigned));
    if (idx)
    {
      while (idx)
      {
        lseek(uf, (long) idx * sizeof(user_t), SEEK_SET);
        if (! _read(uf, uu, sizeof(user_t)))
        {
          trace(serious, "loaduser", "read %08X %08X %s %d",
                                      idx, hsh, usercall, num);
          reorg_noetig++;
          goto ende;
        }
        if ((num++) > 500)
        {
          trace(serious, "loaduser", "loop %08X %08X %s",
                                      idx, hsh, usercall);
          reorg_noetig++;
          goto ende;
        }
        if (! strcmp(usercall, uu->call))
        {
          retwert = OK;
          goto ende;
        }
        idx = uu->nextsamehash;
      }
    }
    if (anlegen)
    {
      defaultuser(usercall, uu);
      retwert = newuser(uu, uf, uhf);
    }
    ende:
    us_close();
    if (retwert)
    {
      uu->lastload = ad_time();
      us_putcache(uu);
    }
    else uu->filepos = 0;
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

void reorguser (int delempty)
//*************************************************************************
//
//  Generiert die Hashtabelle neu und ueberprueft die
//  Eintraege auf Plausibilitaet. Geloeschte Eintraege werden
//  entfernt.
//  Bei delempty==1 werden veraltete Eintraege ohne
//  eingegebenem Namen hinausgeworfen.
//
//*************************************************************************
{
  handle ualt, uneu, uhneu;
  user_t ux;
  unsigned long pos = 0L, errors = 0L, ndeleted = 0L;
  time_t ti = ad_time();
  long maxdays = m.userlife * 2;
  int stelle;

  trace(replog, "reorg", "userlist");
  xunlink(USERNAMEBAK);
  xunlink(USERHASHNAMEBAK);
  xunlink(UTMP);
  xunlink(UHTMP);
  ualt = s_open(USERNAME, "lrb");
  if (hd_space(filelength(ualt)))
  {
    s_close(ualt);
    trace(serious, "reorguser", "disk full");
    return;
  }
  if (maxdays < 180) maxdays = 180;
  uneu = s_open(UTMP, "lw+b");
  uhneu = s_open(UHTMP, "lw+b");
  if (ualt != EOF && uneu != EOF && uhneu != EOF)
  {
    hashinit(uhneu, HASH32);
    _read(ualt, &ux, sizeof(user_t));
    defaultuser("DUMMY", &ux);
    _write(uneu, &ux, sizeof(user_t));
    while (1)
    {
      waitfor(e_ticsfull);
      if (! _read(ualt, &ux, sizeof(user_t))) break;
      ux.nextsamehash = 0;
      *ux.restplatz = 0;
      if (delempty && ! *ux.name)
      {
        if ((! ux.mybbsok && ux.mybbstime < (ti - (DAY * maxdays)))
                          || ux.mybbstime < (ti - (DAY * 2 * maxdays)))
          continue;
      }
      if (ux.mybbstime > (ti + MAXAGE)) //remove incorrect mybbs
      {
        *ux.mybbs = 0;
        ux.mybbstime = 0;
        errors++;
      }
      if (! *ux.call)
      {
        ndeleted++;
        continue;
      }
      if (strlen(ux.call) > CALLEN
          || strlen(ux.ttypw) > 8
          || ! mbcallok(ux.call))
      {
        errors++;
        continue;
      }
      if (*ux.mybbs && mbhadrok(ux.mybbs) != 1)
      {
        *ux.mybbs = 0;
        ux.mybbsok = 0;
        errors++;
      }
      ti = ad_time();
      // --------------------------------------------------------------------
      // - Strings auf Einhaltung der maximalen Laenge pruefen
      // - Zahlenwerte auf Einhaltung des Wertebereiches pruefen (nicht alle)
      // - Fehlerhafte Eintraege werden im Syslog vermerkt
      //   (nicht geloescht!)                           DG9FDL, 06.05.97
      if (   (strlen(ux.name)         > NAMELEN)        // Name
          || (strlen(ux.qth)          > QTHLEN)         // QTH
          || (strlen(ux.zip)          > ZIPLEN)         // ZIP
          || (strlen(ux.mybbs)        > MYBBSLEN)       // MyBBS
          || (strlen(ux.uplink)       > CALLSSID)       // uplink
          || (strlen(ux.newcall)      > CALLEN)         // newcall
          || (strlen(ux.prompt)       > PROMPTLEN)      // Prompt
          || (strlen(ux.sprache)      > 3)              // Sprache
          || (strlen(ux.password)     > 39)             // Passwort
          || (strlen(ux.notvisible)   > (LINELEN-1))    // Reject
          || (strlen(ux.firstcmd)     > FIRSTCMDLEN)    // Command
          || (strlen(ux.ufwd)         > HADRESSLEN)     // ufwd
          || (strlen(ux.ttypw)        > 8)              // TTY-PW
          || (strlen(ux.linuxpw)      > 8)              // Linux-PW
          || (strlen(ux.notification) > CALLEN)         // Notification-Call
          || (strlen(ux.awaytext)     > (LINELEN-1))    // Away-Text
          || (ux.qthok         > 1)                     // qthok (0..1)
          || (ux.zipok         > 1)                     // zipok (0..1)
          || (ux.comp          > 1)                     // comp (0..1)
          || (ux.mybbsok       > 1)                     // mybbsok (0..1)
          || (ux.helplevel     > 2)                     // helplevel (0..2)
          || (ux.lf            > 6)                     // lf (0..6) 6 = -1
          || (ux.nopurge       > 1)                     // nopurge (0..1)
          || (ux.nameok        > 1)                     // nameok (0..1)
          || (ux.fdelay        > 60)                    // fdelay (0..60)
          || (ux.fhold         > 1)                     // fhold (0..1)
          || (ux.status        > 15)                    // status (0..15)
          || (ux.readlock      > 2)                     // readlock (0..2)
          || (ux.rlimit        > 1)                     // rlimit (0..1)
          || (ux.pwline        > 1)                     // pwline (0..1)
          || (ux.echo          > 1)                     // echo (0..1)
          || (ux.paclen        > 256)                   // paclen (0..256)
          || (ux.dirformat     > 1)                     // dirformat (0..1)
          || (ux.binmode       > 4)                     // binmode (0..3)
          || (ux.unsecure_smtp > 1)                     // unsec.-smtp (0..1)
          || (ux.away          > 1)                     // away (0..1)
          || (ux.charset       > 1)                     // charset (0..1)
          || (ux.ttycharset    > 1)                     // ttycharset (0..1)
          || (ux.fbbcheckmode  > 1)                     // fbbcheckmode (0..1)
          || (ux.httpsurface   > 2)                     // httpsurface (0..2)
          || (ux.mybbstime     > ti)                    // mybbstime
          || (ux.lastboxlogin  > ti)                    // lastboxlogin
          || (ux.lastdirnews   > ti)                    // lastdirnews
          || (ux.lastquit      > ti)                    // lastquit
          || (ux.lastload      > ti)                    // lastload
          /* es fehlen:
           loginpwtype, sfpwtype, logins, mailsent, mailgot, mailread,
           daybytes, opt[8], awayendtime
          */
               )
      {
        if (errors < 100)
          trace(replog, "reorguser", "data error %s", ux.call);
        errors++;
      }
      // dh8ymb: div. leere Zellen definiert mit 0x00 fuellen, wenn unbenutzt
      for (stelle = strlen(ux.name); stelle < NAMELEN; stelle++)
        if (ux.name[stelle] != 0) ux.name[stelle] = 0;
      for (stelle = strlen(ux.qth); stelle < QTHLEN; stelle++)
        if (ux.qth[stelle] != 0) ux.qth[stelle] = 0;
      for (stelle = strlen(ux.zip); stelle < ZIPLEN; stelle++)
        if (ux.zip[stelle] != 0) ux.zip[stelle] = 0;
      for (stelle = strlen(ux.mybbs); stelle < MYBBSLEN; stelle++)
        if (ux.mybbs[stelle] != 0) ux.mybbs[stelle] = 0;
      for (stelle = strlen(ux.uplink); stelle < CALLSSID; stelle++)
        if (ux.uplink[stelle] != 0) ux.uplink[stelle] = 0;
      for (stelle = strlen(ux.newcall); stelle < CALLEN; stelle++)
        if (ux.newcall[stelle] != 0) ux.newcall[stelle] = 0;
      for (stelle = strlen(ux.prompt); stelle < PROMPTLEN; stelle++)
        if (ux.prompt[stelle] != 0) ux.prompt[stelle] = 0;
      for (stelle = strlen(ux.password); stelle < 39; stelle++)
        if (ux.password[stelle] != 0) ux.password[stelle] = 0;
      if (! *ux.notvisible) // leeres ux.notvisible definiert mit 0 belegen
      {
        for (stelle = 1; stelle < LINELEN; stelle++)
          if (ux.notvisible[stelle] != 0) ux.notvisible[stelle] = 0;
      }
      else
      {
        for (stelle = strlen(ux.notvisible); stelle < LINELEN; stelle++)
          if (ux.notvisible[stelle] != 0) ux.notvisible[stelle] = 0;
      }
      for (stelle = strlen(ux.firstcmd); stelle < FIRSTCMDLEN; stelle++)
        if (ux.firstcmd[stelle] != 0) ux.firstcmd[stelle] = 0;
      for (stelle = strlen(ux.ufwd); stelle < HADRESSLEN; stelle++)
        if (ux.ufwd[stelle] != 0) ux.ufwd[stelle] = 0;
      for (stelle = strlen(ux.ttypw); stelle < 8; stelle++)
        if (ux.ttypw[stelle] != 0) ux.ttypw[stelle] = 0;
      for (stelle = strlen(ux.linuxpw); stelle < 8; stelle++)
        if (ux.linuxpw[stelle] != 0) ux.linuxpw[stelle] = 0;
      for (stelle = strlen(ux.notification); stelle < CALLEN; stelle++)
        if (ux.notification[stelle] != 0) ux.notification[stelle] = 0;
      for (stelle = strlen(ux.awaytext); stelle < LINELEN-1; stelle++)
        if (ux.awaytext[stelle] != 0) ux.awaytext[stelle] = 0;
      for (stelle = 1; stelle < USERDBREST; stelle++)
        if (ux.restplatz[stelle] != 0) ux.restplatz[stelle] = 0;

      if (udrin(ux.call, uneu, uhneu))
      {
        trace(replog, "reorguser", "double %s", ux.call);
        errors++;
      }
      else newuser(&ux, uneu, uhneu);
      pos++;
      if (! (pos % 5000))
      {
        char s[20];
        sprintf(s, "%lu:%s", pos, ux.call);
        lastcmd(s);
        trace(report, "reorguser", s);
      }
    }
    trace(replog, "reorguser", "%lu user, %lu errors, %lu deleted",
                               pos, errors, ndeleted);
    s_close(ualt);
    s_close(uneu);
    s_close(uhneu);
    xrename(USERNAME, USERNAMEBAK);
    xrename(USERHASHNAME, USERHASHNAMEBAK);
    xrename(UTMP, USERNAME);
    xrename(UHTMP, USERHASHNAME);
    us_deletecache();
    lastreorg = ad_time();
  }
}

/*---------------------------------------------------------------------------*/
#ifdef DIEBOX_UIMPORT
void uimport (void)
//*************************************************************************
//
//  Importiert Benutzerdaten aus dem Thebox-Format
//
//*************************************************************************
{
  struct
  {
    char speech[4];
    char lastlog[15];
    char mybbs[7];
    char name[16];
    int  status;
    long lastmybbs;
  } udat;
  struct
  {
    char call[8];
    long pos;
  } uidx;
  FILE *fd = s_fopen("user3.dat", "lrb");
  FILE *fi = s_fopen("user3.idx", "lrb");
  user_t ux;
  unsigned num = 0, errors = 0;

  if (fd && fi)
  {
    while (1)
    {
      waitfor(e_ticsfull);
      if (testabbruch()) break;
      if (! fread(&uidx, sizeof(uidx), 1, fi)) break;
      fseek (fd, uidx.pos, SEEK_SET);
      if (! fread(&udat, sizeof(udat), 1, fd)) break;
      strlwr(udat.name);
      *udat.name = toupper(*udat.name);
      while (uidx.call[strlen(uidx.call) - 1] == ' ')
        uidx.call[strlen(uidx.call) - 1] = 0;
      num++;
      waitfor(e_ticsfull);
      if (mbcallok(uidx.call) && mbcallok(udat.mybbs))
      {
        if (!(num % 100)) putf("%u:%s\n", num, ux.call);
        if (loaduser(uidx.call, &ux, 1))
        {
          if (! ux.mybbsok) strcpy(ux.mybbs, udat.mybbs);
          ux.mybbsok = 1;
          ux.mybbstime = udat.lastmybbs;
          udat.name[13] = 0;
          if (! *ux.name) strcpy(ux.name, udat.name);
          saveuser(&ux);
        }
      }
      else errors++;
    }
    s_fclose(fd);
    s_fclose(fi);
  }
  else
    putf(ms(m_nouser3));
  putf(ms(m_dieboximported), num, errors);
}
#endif
/*---------------------------------------------------------------------------*/

void saveuser (user_t *uu)
//*************************************************************************
//
//  USER abspeichern
//
//*************************************************************************
{
#ifdef _GUEST
  if (! strcmp(uu->call, m.guestcall) && strcmp(uu->mybbs, m.boxname))
  {
    return; // guestcall-mybbs nur abspeichern wenn gleich eigener box
  }
#endif
  if (uu->filepos && (uu->lastload > lastreorg))
  {
    us_putcache(uu);
    us_open();
    lseek(uf, (long) uu->filepos * sizeof(user_t), SEEK_SET);
    _write(uf, uu, sizeof(user_t));
    us_close();
  }
}

/*---------------------------------------------------------------------------*/

void userinit (void)
//*************************************************************************
//
//   Zugriff USER-File vorbereiten
//
//*************************************************************************
{
  if (sizeof(user_t) != 640 || is_null)
    trace(tr_abbruch, "user_t", "size %d", sizeof(user_t));
  us_open();
  if (uf == EOF)
  {
    trace(tr_abbruch, "userinit", "not open");
    return;
  }
  hashinit(uhf, HASH32);
  us_close();
}

/*---------------------------------------------------------------------------*/

void listuser (char *wildcard, int lokal)
//*************************************************************************
//
//  Listet alle bekannten Benutzerrufzeichen
//
//*************************************************************************
{
  user_t us;
  int spalte = 0;

  handle fh = s_open(USERNAME, "lrb");
  if (lokal)
    putf(ms(m_usersknownlocal));
  else
    putf(ms(m_usersknown));
  lseek(fh, sizeof(user_t), SEEK_SET);
  while (_read(fh, &us, sizeof(user_t)))
  {
    if (! *wildcard || strstr(us.call, wildcard))
    {
      if (! lokal || us.lastboxlogin)
      {
        putf("%-7s", us.call);
        if ((++spalte) > 10)
        {
          putv(LF);
          spalte = 0;
        }
      }
    }
    waitfor(e_ticsfull);
    if (testabbruch()) break;
  }
  if (spalte) putv(LF);
  s_close(fh);
}

/*---------------------------------------------------------------------------*/

static void scanpar (char *buf, char *ziel, char *such)
//*************************************************************************
//
//  Hilfsroutine, sucht nach einem Parameter, der mit einem
//  Suchbegriff beginnt (Suchbegriff ist nicht Bestandteil)
//
//*************************************************************************
{
  char *found;
  unsigned int l = strlen(such);

  found = strstr(buf, such);
  *ziel = 0;
  if (found)
  {
    while (l--) *found++ = ' ';
    l = 0;
    while (*found > ' ')
    {
      if (l < 11) ziel[l++] = *found;
      *found++ = ' ';
    }
    ziel[l] = 0;
  }
}

/*---------------------------------------------------------------------------*/

void usersuch (char *befbuf)
//*************************************************************************
//
//  Durchsucht saemtliche Benutzerdaten nach Suchbegriffen
//
//*************************************************************************
{
  FILE     *fusr;
  user_t   us;
  unsigned hitcounter = 0;
  unsigned allcounter = 0;
  bitfeld  options      = b->optplus;
  unsigned onelogin     = !!(options & o_l);
  long unsigned nologin =   (options & o_o) ? 0UL : ULONG_MAX;
  int      mybbsunknown =   (options & o_u) ? 0 : 255;
  int      mybbsknown   = !!(options & o_n);
  int      mybbsok      = !!(options & o_m);
  int      pwset        = !!(options & o_p);
  int      newcall      = !!(options & o_q);
  int      ttypwset     = !!(options & o_t);
  int      helplevel0   =   (options & o_h) ? 0 : 255;
  int      status       = !!(options & o_s);
  int      readlock     = !!(options & o_r);
  int      userfwd      = !!(options & o_w);
  int      nopurge      = !!(options & o_y);
  int      fhold        = !!(options & o_d);
  int      rlimit       = !!(options & o_i);
  char     call[12];
  char     name[12];
  char     qth[12];
  char     zip[12];
  char     mybbs[12];
  char     uplink[12];
  char     sprache[12];
  char     lastls[12]; // last login string
  time_t   lastlogin = 0L;

  strupr(befbuf);
  scanpar(befbuf, call,    "C=");
  scanpar(befbuf, name,    "N=");
  scanpar(befbuf, qth,     "Q=");
  scanpar(befbuf, zip,     "Z=");
  scanpar(befbuf, mybbs,   "@=");
  scanpar(befbuf, uplink,  "V=");
  scanpar(befbuf, sprache, "S=");
  scanpar(befbuf, lastls,  "L=");
  if (*lastls) lastlogin = parse_time(lastls);
  if (! *call)
  {
    befbuf += blkill(befbuf);
    befbuf[11] = 0;
    strcpy(call, befbuf);
  }
  if ((fusr = s_fopen(USERNAME, "lrb")) != 0)
  {
    setvbuf(fusr, NULL, _IOFBF, 16384);
    if (! (b->optplus & o_c))
      putf("Call   Mybbs   Name         QTH          ZIP    Logs Last via     PTHRSUNAFS\n");
           //example:
           //Call   Mybbs   Name         QTH          ZIP    Logs Last via     PTHRSUNAFS
           //DGT274 @DBO274 Markus       Oelde        59302    24   8s Console 010000001DL
    fseek(fusr, sizeof(user_t), SEEK_SET);
    while (fread(&us, sizeof(user_t), 1, fusr))
    {
      subst1(us.mybbs, '.', 0);
      if (*us.call &&
        us.logins      >= onelogin              &&
        us.logins      <= nologin               &&
        us.status      >= status                &&
        us.readlock    >= readlock              &&
        us.nopurge     >= nopurge               &&
        us.fhold       >= fhold                 &&
        us.rlimit      >= rlimit                &&
        *us.mybbs      <= mybbsunknown          &&
        *us.mybbs      >= mybbsknown            &&
        us.mybbsok     >= mybbsok               &&
        *us.password   >= pwset                 &&
        *us.ttypw      >= ttypwset              &&
        *us.ufwd       >= userfwd               &&
        us.helplevel   <= helplevel0            &&
        us.lastboxlogin>= lastlogin             &&
        *us.newcall    >= newcall               &&
        (! *call       || strstr(us.call, call))      &&
        (! *name       || stristr(us.name, name))     &&
        (! *qth        || stristr(us.qth, qth))     &&
        (! *zip        || stristr(us.zip, zip))     &&
        (! *mybbs      || strstr(us.mybbs, mybbs))    &&
        (! *uplink     || stristr(us.uplink, uplink)) &&
        (! *sprache    || strstr(us.sprache, sprache)) )
      {
        if (! (b->optplus & o_c))
        {
          putf("%-6s %c%-6.6s ",
               us.call, us.mybbsok ? '@' : '?', us.mybbs);
          putf("%-12.12s %-12.12s %-5.5s %5ld %4s %-7.7s ", us.name, us.qth, us.zip, us.logins,
               zeitspanne(ad_time() - us.lastboxlogin, zs_seconds), us.uplink);
          putf("%u%u%u%u%u%u%u%u%u%s\n", !! *us.password + (*us.password == 1),
               !! *us.ttypw, us.helplevel, us.readlock, us.status,
               !! *us.ufwd, us.nopurge, us.away, us.fhold, us.sprache);
        }
        hitcounter++;
        if (b->optplus & o_f) break;
      }
      if (! ((allcounter++) % 32))
      {
        waitfor(e_ticsfull);
        if (testabbruch()) break;
      }
    }
    s_fclose(fusr);
    putf(ms(m_usersfound), allcounter, hitcounter);
  }
}


/*---------------------------------------------------------------------------*/

static void near userhashfile_newformat (void)
//*************************************************************************
//
//  Konvertiert 16bit Hashfile in 32bit Hashfile
//
//*************************************************************************
{
  FILE *fi;
  FILE *fo;
  struct
  {
    short unsigned l;
    short unsigned h;
  } sbb = { 0, 0 };

  //no scheduling here - watchdog is not active at this state
  //on K6/200: 14s under Linux, 24s under Win95/DOS for 64000 users
  if ((file_isreg(USERHASHNAME_OLD) > file_isreg(USERHASHNAME))
      && (file_isreg(USERHASHNAME_OLD) > 0))
  {
    user_t *us = (user_t*) t_malloc(sizeof(user_t), "usnf");
    trace(replog, "user", "converting format "USERHASHNAME_OLD"->"USERHASHNAME);
    fi = s_fopen(USERHASHNAME_OLD, "srb");
    fo = s_fopen(UHTMP, "swb");
    if (fo) s_fsetopt(fo,1);
    if (fi && fo)
      while (fread(&sbb.l, 2, 1, fi))
        fwrite(&sbb, 4, 1, fo);
    if (fo) s_fclose(fo);
    if (fi) s_fclose(fi);
    fi = s_fopen(USERNAME, "srb");
    fo = s_fopen(USERNAMETMP, "swb");
    if (fo) s_fsetopt(fo, 1);
    if (fi && fo)
    {
      while (fread(us, sizeof(user_t), 1, fi))
    //copy old 16bit hash to 32bit hash, but leave old hash
    //if user wants to return to an older version
      {
        if (! us->nextsamehash)
        {
          us->nextsamehash = us->nextsamehash_old;
          us->filepos = us->filepos_old;
          //us->filepos_old = 0;
          //us->nextsamehash_old = 0;
        }
        fwrite (us, sizeof(user_t), 1, fo);
      }
    }
    if (fi) s_fclose(fi);
    if (fo) s_fclose(fo);
    xunlink(USERHASHNAME);
    xrename(UHTMP, USERHASHNAME);
    xunlink(USERNAME);
    xrename(USERNAMETMP, USERNAME);
    t_free(us);
  }
}

/*---------------------------------------------------------------------------*/

#ifdef _USERS4CONVERT
static void near usersfile_newformat (void)
//*************************************************************************
//
//  Konvertiert users.bcm/users3.bcm nach users4.bcm
//  Dauer bei Pentium1 mit 133MHz und ca. 16.800 Usern: ca. 4 Sekunden
//
//*************************************************************************
{
  lastfunc("usersfile_newformat");
  FILE *fo;
  FILE *fn;
  int i;
  long anzahl=0;

  if (! file_isreg(USERNAME))
  {
    user_t *us = (user_t*) t_malloc(sizeof(user_t), "usnf");
    userold_t *uso = (userold_t*) t_malloc(sizeof(userold_t), "usof");
    fo = s_fopen(USERNAMEOLD, "srb");
    fn = s_fopen(USERNAMETMP, "swb");
    if (fo && fn)
    {
      trace(serious, "usersfile_newformat", "start converting");
      anzahl = 0;
      while (fread(uso, sizeof(userold_t), 1, fo) == 1)
      {
        us->lastboxlogin=uso->lastboxlogin;
        us->lastdirnews=uso->lastdirnews;
        us->lastquit=uso->lastquit;
        us->mybbstime=uso->mybbstime;
        us->lastload=uso->lastload;
        us->daybytes=uso->daybytes;
        us->mailsent=uso->mailsent;
        us->mailgot=uso->mailgot;
        us->mailread=uso->mailread;
        us->logins=uso->logins;
        for (i = 0; i < 8; i++)
          us->opt[i]=uso->opt[i];
        us->helplevel=uso->helplevel;
        us->zeilen=uso->zeilen;
        us->lf=uso->lf;
        us->status=uso->status;
        us->filepos=uso->filepos;
        us->nextsamehash=uso->nextsamehash;
        us->fdelay=uso->fdelay;
        us->fhold=uso->fhold;
        us->paclen=uso->paclen;
        us->readlock=uso->readlock;
        us->pwline=uso->pwline;
        us->echo=uso->echo;
        us->rlimit=uso->rlimit;
        us->nopurge=uso->nopurge;
        us->nameok=uso->nameok;
        us->mybbsok=uso->mybbsok;
        us->charset=uso->charset;
        us->loginpwtype=uso->loginpwtype;
        us->sfpwtype=uso->sfpwtype;
        us->dirformat=uso->dirformat;
        us->unsecure_smtp=uso->unsecure_smtp;
        us->binmode=uso->binmode;
#ifndef __MSDOS__
        us->ttycharset=uso->ttycharset;
#endif
        us->away = uso->away;
        us->awayendtime = 0;
        *us->awaytext = 0;
        strcpy(us->qth, "?");
        us->qthok = 0;
        strcpy(us->zip, "?");
        us->zipok = 0;
        *us->restplatz = 0;
        strcpy(us->call, uso->call);
        strcpy(us->name, uso->name);
        if (*uso->mybbs)
          strcpy(us->mybbs, uso->mybbs);
        else
          strcpy(us->mybbs, uso->oldmybbs);
        strcpy(us->prompt, uso->prompt);
        strcpy(us->sprache, uso->sprache);
        strcpy(us->password, uso->password);
        strcpy(us->notvisible, uso->notvisible);
        strcpy(us->firstcmd, uso->firstcmd);
        strcpy(us->ttypw, uso->ttypw);
        strcpy(us->uplink, uso->uplink);
        strcpy(us->ufwd, uso->ufwd);
        strcpy(us->newcall, uso->newcall);
        fwrite(us, sizeof(user_t), 1, fn);
        anzahl++;
      }
      trace(serious, "usersfile_newformat", "%ld users converted", anzahl);
    }
    if (fo) s_fclose(fo);
    if (fn) s_fclose(fn);
    xrename(USERNAMETMP, USERNAME);
    xunlink(USERNAMETMP);
    t_free(us);
    t_free(uso);
    while (! sema_lock("purgereorg"))
      wdelay(100);
    reorguser(0);
    sema_unlock("purgereorg");
  }
}
#endif

/*---------------------------------------------------------------------------*/
