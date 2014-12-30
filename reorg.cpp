/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------
  Erzeugen/Update von LIST- und CHECK-Files
  -----------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980218 OE3DZW added msgty / "7+" to listline
//19980219 OE3DZW removed callcheck for "herkunft" (sender of mail)
//19980223 OE3DZW added parameter "keep_msgtype" to add_dirlist
//19980913 OE3DZW added msgty 6 - broken 7+ file
//19990818 OE3DZW fixed y2k-bug, did not recognise mails >2000
//20000103 DF3VI  (dzw) added flags for 7-info and 7-text
//20000116 DK2UI  (dzw) added ui-options

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int reorg_noetig = 0;

/*---------------------------------------------------------------------------*/

void reorgboard (char *board, int remdouble)
//*************************************************************************
//
//  Erzeugt LIST-File fuer ein bestimmtes Board neu;
//  funktioniert fuer user- und infofiles
//
//*************************************************************************
{
  b->index = finddir(board, b->sysop);
  lastcmd(board);
  waitfor(e_ticsfull);
  if (b->index)
  {
    trace(report, "reorgboard", "%s", b->boardfullname);
    make_mask(b->mask, b->boardfullname);
    strcpy(b->listpath, b->mask);
    if (! strstr(b->listpath, "*.*"))
      trace(fatal, "reorgboard", "mask %s", b->mailpath);
    strcpy(strstr(b->listpath, "*.*"), DIRLISTNAME);
    createdirlist(remdouble);
  }
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

void mbreorg (char *name)
//*************************************************************************
//
//  reorganisieren:
//  es werden alle LIST-Files und das CHECK-File neu aufgebaut
//  waehrend dieser Vorgang laeuft, werden send-Zugriffe nicht erlaubt
//
//*************************************************************************
{
  char *userlist;
  unsigned int i = 0;
  char befehl[20];
  int remdouble = 0;
  int old_disable = m.disable;
  int doall = 0;

  strncpy(befehl, name, 19);
  befehl[19] = 0;
  if (! sema_lock("purgereorg")) return;
  b->sysop = 1;
  strcpy(t->name, "Reorg");
//Moegliche Reorgtypen: X (F) B C U L P I H N
  if (! stricmp(befehl, "X"))
  {
    if (m.nopurge == 1 || m.nopurge >= 4)
      trace(replog, "reorg", "REORG X disabled because NOPURGE=1 or >=4");
    else
    {
      remdouble = 1;
      m.disable = 1;
      xunlink("bids2.reo");
      xunlink("bidh2.reo");
      xrename(BIDNAME, "bids2.reo");
      xrename(BIDHASHNAME, "bidh2.reo");
      bidinit(0);
      doall = 1;
    }
  }
  if (! stricmp(befehl, "F"))
  {
    doall = 1;
    remdouble = 2;
  }
  if (! *befehl) doall = 1;
  b->job = j_reorg;
  b->usermail = 0;
  if (! stricmp(befehl, "B") || ! stricmp(befehl, "C") || doall)
  {
    strcpy(t->name, "Reorg B");
    trace(replog, "reorg", "bulletins");
    for (i = 0; i < treelen; i++)
      reorgboard(tree[i].name, remdouble);
  }
  if (! stricmp(befehl, "C") || doall)
  {
    strcpy(t->name, "Reorg C");
    makecheck();
  }
  if (! stricmp(befehl, "X"))
  {
    if (m.nopurge == 1 || m.nopurge >= 4) {}
    else
    {
      strcpy(t->name, "Reorg X");
      m.disable = old_disable;
      xunlink(BIDNAME);
      xunlink(BIDHASHNAME);
      xrename("bids2.reo", BIDNAME);
      xrename("bidh2.reo", BIDHASHNAME);
    }
  }
  if (doall || ! stricmp(befehl, "U"))
  {
    strcpy(t->name, "Reorg U");
    trace(replog, "reorg", "usermails");
    b->usermail = 1;
    userlist = (char *) t_malloc(8U * MAXUSERMAILS, "u_re");
    if (userlist)
    {
      i = 0;
      struct dirent *di;
      DIR *d = opendir(m.userpath);
      while ((di = readdir(d)) != NULL)
      {
        if (*di->d_name != '.')
        {
          strncpy(userlist + (i * 8), di->d_name, 6);
          userlist[i * 8 + 6] = 0;
          i++;
          if (i >= MAXUSERMAILS)
            trace(fatal, "reorg", "too many usermails");
        }
        waitfor(e_ticsfull);
      }
      closedir(d);
      userlist = (char *) t_realloc(userlist, 8U * (i + 1));
      while (i--) reorgboard(userlist + (i * 8), 0);
      t_free(userlist);
    }
  }
  if (! stricmp (befehl, "L"))
  {
    strcpy(t->name, "Reorg L");
    reorguser(0);
  }
  if (! stricmp(befehl, "P"))
  {
    strcpy(t->name, "Reorg P");
    reorguser(1);
  }
  if (doall || ! stricmp(befehl, "I"))
  {
    strcpy(t->name, "Reorg I");
    reorgbid();
  }
  if (doall || ! stricmp(befehl, "H"))
  {
    strcpy(t->name, "Reorg H");
    reorg_hadr();
  }
  if (! stricmp(befehl, "N"))
  {
    strcpy(t->name, "Reorg N");
    make_checknum("reorg");
  }
  trace(replog, "reorg", "ready, %s sec cputime",
                zeitspanne(t->cputics ,zs_cpuexact));
  sema_unlock("purgereorg");
  reorg_noetig = 0;
}

/*---------------------------------------------------------------------------*/

static void near add_dirlist (char *filename, handle fh, int check,
                                          int remdouble, int keep_msgtype)
//*************************************************************************
//
//
//*************************************************************************
{
  FILE *tmpf = 0;
  handle tmpfh;
  char *buf = b->line;
  char *betr;
  unsigned long position = 0L;
  int i;
  char local_msgtype = b->conttype;
  char zielboard[9];

  waitfor(e_ticsfull);
  if (strlen(filename) != 7)
    goto error;
  strcpy(b->mailpath, b->mask);
  if (! strstr(b->mailpath, "*.*"))
    trace(fatal, "add_dirlist", "mask %s", b->mailpath);
  strcpy(strstr(b->mailpath, "*.*"), filename);
  strlwr(b->mailpath);
  if (sema_access(b->mailpath) > 1)
  {
    trace(report, "add_dirlist", "sema_access");
    return; // wenn ein schreibender Zugriff besteht
            // den Eintrag noch ignorieren
  }
  tmpf = s_fopen(b->mailpath, "srt");
  if (tmpf)
  {
    if (! fgets(b->line, BUFLEN - 1, tmpf))
      goto error; // Kopfzeile, entspricht Syntax SEND-Befehl
    strcpy(zielboard, b->zielboard);
    mbsend_parse(buf, 0);
    strcpy(b->zielboard, zielboard);
    if (keep_msgtype) b->conttype = local_msgtype;
//    if (! mbcallok(b->herkunft)) goto error;
    if (! *b->ziel)
      goto error;
    if (! fgets(b->line, BUFLEN - 1, tmpf))
      goto error; // Forwardzeile, * fuer geloescht
    if (*buf == '*') b->geloescht = buf[1];
    else b->geloescht = 0;
    if (! fgets(b->line, BUFLEN - 1, tmpf))
      goto error; // READ:-Zeile
    b->gelesen = !! strstr (buf, b->ziel); // Empfaenger hat gelesen?
    if (! b->bytes) goto error;
    b->at[CALLEN] = 0;
    subst1(b->at, '.', 0);
    subst1(b->at, '-', 0);
    if (! fgets(b->betreff, BETREFFLEN, tmpf))
      goto error; // Betreff-Zeile
    cut_blank(b->betreff);
#ifndef USERLT
    if (! b->lifetime)
    {
      b->lifetime = checklifeconv(b->betreff);
      if (b->lifetime < 0 || b->usermail)
        b->lifetime = b->boardlife_max;
    }
    if (! b->sysop && b->lifetime > (b->boardlife_max << 1))
      b->lifetime = (b->boardlife_max << 1);
    if (! b->sysop && b->lifetime < b->boardlife_min)
      b->lifetime = b->boardlife_min;
    if (b->lifetime > 999) b->lifetime = 999;
#endif
    if (*b->bid && (check || remdouble))
    {
      char bidbuf[50];
      sprintf(bidbuf, "%s/%s", b->boardname, filename);
      strupr(bidbuf);
      if (bidvorhanden(b->bid))
      {
        if (remdouble == 1) goto error;
        bidnewfilename(b->bid, bidbuf);
      }
      else
        bidrein(b->bid, bidbuf);
    }
    strcpy(buf, filename);
    strupr(buf);
    sprintf(buf + 7, "<%-6s>%-8s~%-6s@%-6s$%-12s#%3d%%%s",
            b->herkunft,
            b->boardname,
            b->frombox, b->at, b->bid,
#ifdef USERLT
            b->boardlife_max,
#else
            b->lifetime,
#endif
            dezasc(b->lines, 2));
// OE3DZW: Mailtype nur dann, wenn Usermail
    if (b->usermail && b->gelesen) buf[22] = 'R';
    if (b->geloescht) buf[14] = b->geloescht;
    sprintf(buf + strlen(buf), "%s", dezasc(nonlin(b->bytes), 3));
// OE3DZW: hier kann ein neues Flag eingefuegt werden!
//  16  neue Flags                1 2 3 4 5 6 7 8 9 A B C D E F 0
    sprintf(buf + strlen(buf), "|%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c-",
      b->mailtype, b->fwdhold, b->fwdprop, b->replied,
      b->conttype, b->eraseinfo, b->flag7, b->flag8, b->flag9,
      b->flaga, b->flagb, b->flagc, b->flagd, b->flage,
      b->flagf, b->flag0);
    if (b->binstart || b->conttype == 'B')
      strcat(buf, "(BIN) ");
    else
    {
      if (b->conttype == '6')      strcat(buf, "(7-) ");
      else if (b->conttype == '7') strcat(buf, "(7+) ");
      else if (b->conttype == '8') strcat(buf, "(7inf) ");
      else if (b->conttype == '9') strcat(buf, "(7txt) ");
      else if (b->conttype == 'H') strcat(buf, "(html) ");
    }
    betr = buf + strlen(buf);
    strcat(buf, b->betreff);
    for (i = 0; betr[i]; i++)
      if (betr[i] < ' ') betr[i] = '.';
    buf[BLEN - 2] = 0;
    while (strlen(buf) < (BLEN - 2))
      strcat(buf, " ");
    strcat(buf, "\r\n");
    _write(fh, buf, BLEN);
    position = (unsigned long) (ltell(fh) >> 7);
    s_fclose(tmpf);
    if (check && ! b->usermail)
    {
      do
      {
        tmpfh = s_open(CHECKNAME, "sab");
        if (b->semalock) wdelay(519);
      } while (b->semalock);
      if (tmpfh != EOF)
      {
        _write(tmpfh, buf, BLEN);
        s_close(tmpfh);
        tmpfh = s_open(CHECKNUMNAME, "sab");
        if (tmpfh != EOF)
        {
          _write(tmpfh, &position, 4); // 4 Byte=32bit
          s_close(tmpfh);
        }
      }
      else
        trace(serious, "add_dirlist", "check open");
    }
    waitfor(e_ticsfull);
    return;
  }
  else
    trace(replog, "add_dirlist", "text open %s", b->mailpath);
  return;
error:
  {
    s_fclose(tmpf);
    xunlink(b->mailpath);
    trace(serious, "add_dirlist", "%s>%s $%s", b->herkunft,
                                               b->boardname, b->bid);
  }
}

/*---------------------------------------------------------------------------*/

char *dirsort (char *mask, int users)
//*************************************************************************
//
//
//*************************************************************************
{
  char *list = NULL;
  unsigned pos = 0;
  int ok;

  strlwr(mask);
#ifdef __BORLANDC__
  struct ffblk di;
  char mask_wild[FNAMELEN+1];

  sprintf(mask_wild, "%s/*.*", mask);
  if (! findfirst(mask_wild, &di, 0xffff))
  {
    do
    {
      if (users)
        ok = (strlen(di.ff_name) > 3) && (strlen(di.ff_name) < 7);
      else
        ok = ! (di.ff_attrib & FA_DIREC) && (strlen(di.ff_name) == 7);
      if (ok)
      {
        if (! list)
          list = (char *) t_malloc(MAXDIRENTRIES * 8, "dirs");
        strcpy(list + pos, di.ff_name);
        list[pos + 7] = 0; // evtl zu langen Filenamen abschneiden
        pos += 8;
        if (pos >= (MAXDIRENTRIES * 8U))
        {
          trace(fatal, "dirsort", "dir overflow");
          t_free(list);
          return NULL;
        }
      }
      waitfor(e_ticsfull);
    }
    while (! findnext(&di));
  }
#else
  DIR *d = opendir(mask);
  struct dirent *di;
  char dircheckbuf[FNAMELEN+1];

  strncpy(dircheckbuf, mask, FNAMELEN - 7 - 1); // 7bytes: filename, 1byte: /
  dircheckbuf[FNAMELEN - 7 - 1] = 0;
  strcat(dircheckbuf, "/");
  char *ptr = dircheckbuf + strlen(dircheckbuf);
  while (d && (di = readdir(d)) != NULL)
  {
    if (users)
      ok = (strlen(di->d_name) > 3) && (strlen(di->d_name) < 7);
    else
    {
      if (strlen(di->d_name) != 7)
        ok = 0;
      else
      {
        strcpy(ptr, di->d_name);
        ok = ! isdir(dircheckbuf);
      }
    }
    if (ok)
    {
      if (! list)
        list = (char *) t_malloc(MAXDIRENTRIES * 8, "dirs");
      strcpy(list + pos, di->d_name);
      list[pos + 7] = 0; // evtl zu langen Filenamen abschneiden
      pos += 8;
      if (pos >= (MAXDIRENTRIES * 8U))
      {
        closedir(d);
        trace(fatal, "dirsort", "dir overflow");
        t_free(list);
        return NULL;
      }
    }
    waitfor(e_ticsfull);
  }
  closedir(d);
#endif
  if (pos)
  {
    list[pos] = 0;
    list = (char *) t_realloc(list, pos + 8);
    qsort((void *) list, pos >> 3, 8,
           (int(*) (const void *, const void *)) strcmp);
    return list;
  }
  else
  {
    if (list) t_free(list);
    return NULL;
  }
}

/*---------------------------------------------------------------------------*/
/*
static void near touchname (void)*/
//*************************************************************************
//
//  Setzt Datum/Uhrzeit des aktuellen Files auf die momentane Zeit
//
//*************************************************************************
/*
{ char oldname[FNAMELEN+1];

  strcpy(oldname, b->mailpath);
  make_mask(b->mailpath, b->boardfullname);
  strcpy(b->mask, b->mailpath);
  strcpy(b->mailfname, time2filename(0));
  if (! strstr(b->mailpath, "*.*"))
    trace(fatal, "touchname", "mask %s", b->mailpath);
  strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
  strlwr(oldname);
  strlwr(b->mailpath);
  xrename(oldname, b->mailpath);
}
*/
/*---------------------------------------------------------------------------*/

unsigned long appenddirlist (int check)
//*************************************************************************
//
//
//*************************************************************************
{
  handle fh;
  unsigned long nummer = 0L;

  while (   sema_test(b->boardname)
         || ((sema_test("sendlock") || m.disable) && ! b->usermail))
    wdelay(270);
  strcpy(b->listpath, b->mask);
  if (! strstr(b->listpath, "*.*"))
    trace(fatal, "appenddirlist", "mask %s", b->listpath);
  strcpy(strstr(b->listpath, "*.*"), DIRLISTNAME);
  strlwr(b->listpath);
  if ((fh = s_open(b->listpath, "sab")) == EOF)
    fh = s_open(b->listpath, "swb");
  if (fh != EOF)
  {
    add_dirlist(b->mailfname, fh, check, 0, 1);
    nummer = (unsigned long) (filelength(fh) >> 7);
    s_close(fh);
  }
  else
    trace(fatal, "appenddirlist", "open %s", b->listpath);
  return nummer;
}

/*---------------------------------------------------------------------------*/

int checkdirlist (void)
//*************************************************************************
//
//
//*************************************************************************
{
  handle fh;
  char buf[BLEN];
  int pos = 0;
  char *dirlist;
  char tmpmask[FNAMELEN+1];
  int ok = 1;
  int listda = 0;

  strcpy(tmpmask, b->mask);
  if (strstr(tmpmask, "/*.*")) tmpmask[strlen(tmpmask) - 4] = 0;
  dirlist = dirsort(tmpmask, 0);
  if (dirlist)
  {
    fh = s_open(b->listpath, "lrb");
    if (fh != EOF)
    {
      listda = 1;
      while (ok && dirlist[pos])
      {
        if (_read(fh, buf, BLEN) != BLEN)
        {
          sprintf(tmpmask + strlen(tmpmask), "/%s", dirlist + pos);
          if (sema_access(tmpmask) <= 1) ok = 0;
          break;
        }
        //oe3dzw Neue Positon des Titels ist buf[78]
        if (buf[77] != '-' || buf[23] != '~') ok = 0;
        if (! m.smoothheader) // b->ziel und directory brauchen nicht passen
        {
          //df3vi: boardnamen pruefen (zur Behebung von numofboard-fehlern)
          buf[23] = 0;
          subst(buf + 15, ' ', 0);
          if (strncmp(buf + 15, b->boardname,
                      strlen(b->boardname) < 6 ? strlen(b->boardname) : 6))
            ok = 0;
        }
        buf[7] = 0;
        if (stricmp(buf, dirlist + pos))
          ok = 0;
        pos += 8;
        waitfor(e_ticsfull);
      }
      if (pos != (filelength(fh) >> 4))
        ok = 0;
      s_close(fh);
    }
    else ok = 0;
    t_free(dirlist);
  }
  if (! ok && listda)
    trace(report, "reorg", "creating %s", b->listpath);
  return ok;
}

/*---------------------------------------------------------------------------*/


void createdirlist (int remdouble)
//*************************************************************************
//
//
//*************************************************************************
{
  handle fh;
  unsigned pos = 0;
  char *dirlist = NULL;
  char tmpfilename[FNAMELEN+1];
  char tmpmask[FNAMELEN+1];

  while (! sema_lock(b->boardname))
    wdelay(521);
  strlwr(b->mask);
  strlwr(b->listpath);
  if (remdouble || ! checkdirlist())
  {
    strcpy(tmpfilename, b->mask);
    if (! strstr(tmpfilename, "*.*"))
      trace(fatal, "createdirlist", "mask %s", tmpfilename);
    strcpy(strstr(tmpfilename, "*.*"), DIRTMPNAME);
    strcpy(tmpmask, b->mask);
    if (strstr(tmpmask, "/*.*"))
      tmpmask[strlen(tmpmask) - 4] = 0;
    //trace(report, "createdirlist", "%s", tmpmask);
    dirlist = dirsort(tmpmask, 0);
    if (dirlist)
    {
      strlwr(tmpfilename);
      fh = s_open(tmpfilename, "lwb");
      if (fh != EOF)
      {
        while (dirlist[pos])
        {
          //trace(report, "reorg", "add %s %s", tmpmask, dirlist+pos);
          add_dirlist(dirlist + pos, fh, 0, remdouble, 0);
          pos += 8;
          waitfor(e_ticsfull);
        }
        s_close(fh);
        xunlink(b->listpath);
        xrename(tmpfilename, b->listpath);
        if (! filesize(b->listpath)) xunlink(b->listpath);
      }
      else
        trace(fatal, "createdirlist", "error open %s", tmpfilename);
      t_free(dirlist);
    }
  }
  sema_unlock(b->boardname);
  if (dirlist && ! m.disable) wdelay(332);
}

/*---------------------------------------------------------------------------*/

void postfwd (char *)
//*************************************************************************
//
//
//*************************************************************************
{
  char *userlist;
  char bef[LINELEN+1];
  char mybbs[HADRESSLEN+1];
  unsigned i = 0, usermails = 0, forwards = 0;

  if (! sema_lock("postfwd")) return;
  b->sysop = 1;
  userlist = (char *) t_malloc(8U * MAXUSERMAILS, "fwdm");
  if (userlist)
  {
    struct dirent *di;
    DIR *d = opendir(m.userpath);
    while ((di = readdir(d)) != NULL)
    {
      waitfor(e_ticsfull);
      if (*di->d_name != '.')
      {
        strncpy(userlist + (i * 8), di->d_name, CALLEN);
        userlist[i * 8 + CALLEN] = 0;
        i++;
        if (i >= MAXUSERMAILS)
          trace(fatal, "postfwd", "too many usermails");
        waitfor(e_ticsfull);
      }
    }
    closedir(d);
    userlist = (char *) t_realloc(userlist, 8U * (i + 1));
    usermails = i;
    while (i)
    {
      i--;
      *mybbs = 0;
      get_mybbs(userlist + (i * 8), mybbs, 0);
      waitfor(e_ticsfull);
      if (! *mybbs || ! strcmp(atcall(mybbs), m.boxname)) continue;
      sprintf(bef, "%s 1- @ %s", userlist + (i * 8), mybbs);
      forwards += mbchange(bef, w_forward, 0);
      lastcmd(bef);
    }
    t_free(userlist);
    trace(replog, "postfwd", "%u users, %d forw, %ss cpu",
          usermails, forwards, zeitspanne(t->cputics, zs_cpuexact));
  }
}
