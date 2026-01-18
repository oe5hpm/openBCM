/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------
  READ und ERASE von Mails
  ------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980118 OE3DZW corrected binbug again, should be ok now..
//                time without sec in read-output
//19980119 DG9MHZ ifdef was set wrong for Linux, fixed
//19980120 OE3DZW ifdef was set wrong for DOS, fixed
//19980121 DG9MHZ new string-compare at scanmimetype
//19980125 OE3DZW Debugging wrong time in read-header -> read opt j
//                puts filename into that header
//19980126 OE3DZW fixed string compare at scanmimetype again
//                MIME is only recognised if the is only 1 empty line
//19980202 OE3DZW another fix to scanmimetype, tested now ;-)
//19980215 OE3DZW cosmetic changes to
//19980224 OE3DZW added Read (instead of X-Read), sneefers need it...
//19980302 OE3DZW renamed change_addess(), rub out old BIDs in header
//19980304 OE3DZW on comment "@" only if b->at is known
//19980307 OE3DZW added opt_h for head/kopf (long form of r-line-header)
//19980404 hrx    fix - mails to sysop will be marked as read,too
//19980407 OE3DZW bytes corrected
//19980408 OE3DZW r -5 should work now
//19980307 OE3DZW removed opt_h for head/kopf (long form of r-line-header)
//19980903 OE3DZW fix: comment to local mails caused invalid addr
//19980924 OE3DZW cleaned up code - removed markreply
//19990427 OE3DZW added eraseloglevel for WP-erase
//19990808 DH3MB  Added statistics after AutoBIN-TX
//                Now calculating crcthp while sending AutoBIN-Mail
//19991028 Jan    fixed forward cmd for P-mails to "SYSOP"
//19991211 Jan    restructured testreadbin - now sendbin and new sendencbin
//                HTTP read now normal cmd
//199911.. DF3VI  TRANSFER-problem fixed
//19991222 DF3VI  added extract()-utility, called via mbchange
//19991225 DF3VI  set o_x if via import (can't read BIN anyway)
//19991126 DF3VI  On transfer check destinaion befor calling mbtrans()
//                make also transfer of erased/forwarded mails possible
//20000103 OE3DZW replaced strcpy by safe_strcpy where applicable
//20000110 Jan    (dzw) fixes to dot -> dot dot (nntp etc.)
//20000111 OE3DZW will no longer modify R-lines (bidflag, z->Z)
//20000116 DK2UI  (dzw) added ui-options
//20000611 DK2UI  correction for 'forward -f' H-address was changed
//                from dirline()
//20000713 DK2UI  correction to DIDADIT sendname, starts with '#' and
//                not 0 terminating
//20001113 DK2UI  use of new function log_entry for trace log files
//20030727 DB1RAS HRoute-Fix

#include "baycom.h"

#ifndef __FLAT__ //does somebody know a better solution ?
  #define NOT_BHEADERMODE_AND
#else
  #define NOT_BHEADERMODE_AND !b->headermode&&
#endif

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
static char *mimestr (char *s)
//*************************************************************************
//
//
//*************************************************************************
{
  if (stristr(s, ".jpg")) return MIME_JPG;
  if (stristr(s, ".gif")) return MIME_GIF;
  if (stristr(s, ".wav")) return MIME_WAV;
  if (stristr(s, ".txt")) return MIME_TXT;
  return MIME_BIN;
}

/*---------------------------------------------------------------------------*/

static long scanmimetype (FILE *f)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("scanmimetype");
  char buffer[BUFLEN];
  long oldpos = ftell(f);
  long ret = 0;

  if (b->binstart) return b->binstart;
  if (b->conttype == '7' || b->conttype == '6')
    while (fgets(buffer, BUFLEN - 1, f))
      if (! strncmp(buffer, START7PFLAG, 8))
      {
        ret = ftell(f) - strlen(buffer);
        break;
      }
  fseek(f, oldpos, SEEK_SET);
  return ret;
}
#endif

/*---------------------------------------------------------------------------*/

static int owner (void)
//*************************************************************************
//
//  Stellt fest, ob eine Nachricht geloescht werden darf
//
//*************************************************************************
{
  if (b->sysop || b->forwarding == fwd_standard) return OK;
  if (! strcmp(b->ziel, b->logincall)) return OK;
  if (! strcmp(b->herkunft, b->logincall)) return OK;
  return NO;
}

/*---------------------------------------------------------------------------*/

static void putreadheader (time_t mtime)
//*************************************************************************
//
//  Gibt die erste Zeile beim READ aus
//
//*************************************************************************
{
  lastfunc("putreadheader");
  unsigned days_old = (unsigned) ((ad_time() - mtime) / DAY);
  char line[140];

  line[0] = 0;
  if (b->opt & o_a)
    sprintf(line + strlen(line), "%-6s ", b->herkunft);
  if (b->opt & o_e)
    sprintf(line + strlen(line), "> %-8s ", b->ziel);
  if (b->opt & o_d)
   sprintf(line + strlen(line), "%s ",
             datestr(mtime, !! (b->opt & o_y) + (8 * u->dirformat)));
  if (b->opt & o_j)
    sprintf(line + strlen(line), "%s ", b->mailfname);
  if (b->opt & o_t)
    sprintf(line + strlen(line), "%s ", datestr(mtime, 3));
  if (b->opt & o_z)
    sprintf(line + strlen(line), "%u Lines ", b->lines);
  if (b->opt & o_b)
  {
    if (b->bytes < 1000000L)
      sprintf(line + strlen(line), "%lu Bytes ", b->bytes);
    else
      sprintf(line + strlen(line), "%lu kBytes ", b->bytes >> 10);
  }
  if (b->opt & o_l)
  {
#ifdef USERLT
    if (b->boardlife_max == 999)
      sprintf(line + strlen(line), "#999 ");
#else
    if (b->lifetime == 999)
      sprintf(line + strlen(line), "#999 ");
    else
      if (b->lifetime)
        sprintf(line + strlen(line), "#%d ", b->lifetime - days_old);
#endif
    else
      sprintf(line + strlen(line), "#%d ", b->boardlife_max - days_old);
#ifdef USERLT
    //lifetime from user, 0=none
    sprintf(line + strlen(line), "(%d) ", b->lifetime);
#endif
  }
  if (b->opt & o_f)
    sprintf(line + strlen(line), "%s ", b->frombox);
  if (b->opt & o_m)
    sprintf(line + strlen(line), "@ %s", b->at);
  if (! (b->opt & o_c))
    line[LINELEN-1] = 0;
  putf(line);
  putv(LF);
  if (b->opt & o_i)
//oe3dzw: Unterscheidung BID/MID
    if (*b->bid)
    {
      if (b->mailtype == 'B')
        putf("BID : %s\n", b->bid);
      else
        putf("MID : %s\n", b->bid);
    }
    if (b->opt & o_h)
      putf("X-Flags: Type %c Hold %c Prop %c Rep %c Cont %c Erase %c\n",
           b->mailtype, b->fwdhold, b->fwdprop, b->replied, b->conttype,
           b->eraseinfo);
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
static void putmimeaddress (char *s, char *tc)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("putmimeaddress");
  strlwr(s);
  if (tc) strlwr(tc);
  if (*s && tc && *tc)
    putf("<%s@%s>\n", s, tc);
  else
    putf("<%s>\n", s);
}

/*---------------------------------------------------------------------------*/

static void putmimeheader (char *line, long mimeboundary)
//*************************************************************************
//
//  sendet den Mailheader fuer POP/NNTP
//
//*************************************************************************
{
  lastfunc("putmimeheader");
  char s[HADRESSLEN+1];

  strcpy(s, "");
  get_mybbs(b->herkunft, s, 0);

  if (b->headermode == _POP)
    subst1(s, '.', 0);
  else
  if (b->usermail) expand_hadr(s, 2);
  if (b->headermode == _NNTP)
  {
    char group[LINELEN+1];
    sprintf(group, "ampr.bbs.%s", b->boardfullname);
    char *xx = strchr(group, '/');
    if (xx) *xx = '.';
    strlwr(group);
    putf("Xref: %s:%d %s:%d\n", get_fqdn(), m.nntp_port, group, b->replynum);
    putf("Newsgroups: %s\n", group);
  }
  if (*b->bid) putf("Message-ID: <%s@%s>\n", b->bid, m.boxname);
  putf("From: ");
  putmimeaddress(b->herkunft, s);
  strcpy(s, b->at);

  if (b->headermode == _POP)
    subst1(s, '.', 0);
  else
  if (b->usermail) expand_hadr(s, 2);
  putf("To: ");
  putmimeaddress(b->ziel, s);
  putf("Date: %s\n", datestr(getheadertime(line + BLEN) - ad_timezone(), 13));
  putf("Subject: %s\n", b->betreff);
  if (b->headermode == _NNTP)
    putf("References: <%s@%s>\n", b->bid, m.boxname);
  if (mimeboundary > 0)
  {
    putf("MIME-Version: 1.0\n");
    putf("Content-Type: multipart/mixed; boundary=\"----%08X\"\n\n",
          mimeboundary);
    putf("This is a multi-part message in MIME format.\n------%08X\n",
          mimeboundary);
    putf("Content-Type: text/plain; charset=us-ascii\n");
    putf("Content-Transfer-Encoding: 7bit\n");
  }
  //putf("Content-Type: text/plain; charset=iso-8859-1\n");
  //putv(LF);
}
#endif

/*---------------------------------------------------------------------------*/

static long putreadpath (FILE *f, char *line, int kopf)
//*************************************************************************
//
// analysiert die R:-Zeilen und gibt sie ggf. aus
// Beispiel R:-Zeile:
//
// R:920503/1216z @DB0AAB.#BAY.DEU.EU [Muenchen-Nord]
//
//*************************************************************************
{
  lastfunc("putreadpath");
  long lsave = ftell(f);
  int pathshown = 0, pos = 0;
#ifdef __FLAT__
  char next[BLEN];
  char minibuff[LINELEN+1];
  char minibuffer[500];
  int cnt = 0;

  if (b->headermode)
  {
    strlwr(b->herkunft);
    strlwr(b->ziel);
    strlwr(b->at);
  }
#endif
  while (fgets(line, BLEN, f) != NULL && line[0] == 'R' && line[1] == ':')
  {
    int error = 0;
    line[BLEN - 1] = 0;
    cut_blank(line);
    lsave = ftell(f);
#ifdef __FLAT__
    if (b->headermode == _POP)
    {
      b->opt |= o_p;
      if (cnt)
      {
        safe_strcpy(next, getheaderadress(line + BLEN));
        char *adr = getheaderadress(line);
        char *info = strstr(line + BLEN, next) + strlen(next) + 1;
        if (*info)
          snprintf(minibuff, LINELEN, "with %s", info);
        else
          *minibuff = 0;
        if (adr != NULL)
          putf("X-Received: from %s by %s %s; %s\n", adr, next, minibuff,
                datestr(getheadertime(line + BLEN) - ad_timezone(), 13));
        else
        {
          putf("X-Received: from ? by %s %s; %s\n", next, minibuff,
                datestr(getheadertime(line + BLEN) - ad_timezone(), 13));
          trace(serious, "putreadpath", "%s", minibuff);
          error = 1;
        }
      }
      cnt++;
    }
#endif
    if (! error)
    {
      if (b->opt & o_h)
      {
        putf("%s\n", line);
        pathshown++;
      }
      else
        if ((b->opt & o_p) && kopf != 2)
        {
          char *adr;
          if (! pathshown)
        {
#ifdef __FLAT__
          if(b->headermode == _POP || b->headermode == _NNTP)
            sprintf(minibuffer, "X-Path: ");
          else
#endif
            putf("Path: ");
          pathshown++;
        }
        else
        {
#ifdef __FLAT__
          if(b->headermode == _POP || b->headermode == _NNTP)
            sprintf(minibuffer + strlen(minibuffer), "<");
          else
#endif
            putf("<");
        }
        if (NOT_BHEADERMODE_AND (pos++) > 63)
        {
#ifdef __FLAT__
          if(b->headermode == _POP || b->headermode == _NNTP)
            sprintf(minibuffer + strlen(minibuffer), "\n        ");
          else
#endif
            putf("\n      ");
          pos = 0;
        }
        adr = getheaderadress(line);
        if (adr)
        {
          subst1(adr, '.', 0);
#ifdef __FLAT__
          if (b->headermode == _POP || b->headermode == _NNTP)
            sprintf(minibuffer + strlen(minibuffer), "%s", adr);
          else
#endif
            putf("%s", adr);
          pos += strlen(adr);
        }
      }
    strcpy(line + BLEN, line);
    }
  }
#ifdef __FLAT__
  if (b->headermode == _POP || b->headermode == _NNTP)
  {
    char *s = minibuffer;
    while (*s) putv(*(s++));
    putv(LF);
  }
#endif
  if (pathshown)
  {
    if (! (b->opt & o_h) && kopf < 2)
    {
#ifdef __FLAT__
      if (! b->headermode)
#endif
      putv(LF);
    }
    if (NOT_BHEADERMODE_AND (b->opt & o_u) && ! (b->opt & o_h))
    {
      if (kopf < 2) putf("Sent: %0.73s\n", line + BLEN + 2);
    }
  }
  return lsave;
}

/*---------------------------------------------------------------------------*/

static void eraselog (char reason, char *eraser)
//*************************************************************************
//
//
//*************************************************************************
{
  char buf[120];

  if (reason == ' ') reason = 'U';
  if (reason == 'E' && b->sysop) reason = 'S';
  switch (reason)
  {
  case 'E': if (! (m.eraselog & 4)) return;
            break;
  case 'F': if (! (m.eraselog & 8)) return;
            break;
  case 'K':
  case 'X':
  case 'L': if (! (m.eraselog & 1)) return;
            break;
  case 'S': if (! (m.eraselog & 2)) return;
            break;
  case 'T': if (! (m.eraselog & 32)) return;
            break;
  case 'U': if (! (m.eraselog & 16)) return;
  case 'W': if (! (m.eraselog & 64)) return;
            break;
  }
  waitfor(e_ticsfull);
  snprintf(buf, sizeof(buf), "%6s: #%c %6s>%-8s %s %.40s",
                eraser, reason, b->herkunft, b->ziel, b->bid, b->betreff);
  log_entry(ERASELOGNAME, buf);
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

int markerased (char reason, int unerase, int checkerase)
//*************************************************************************
//
//  Vermerkt in einer Nachricht, dass sie geloescht ist
//
//*************************************************************************
{
  int len;
  char found;
  handle fh;
  FILE *f;
  int i;
  char buf[60];
  char eraser[CALLEN+1];
  int doerase = 1;
  int remerase = 0;
  long fwdpos;
  int notfound = 0;
  lastfunc("markerased");
  if (reason == 'L')
    safe_strcpy(eraser, b->herkunft)
  else
    strcpy(eraser, b->logincall);
  if (b->sysop && reason == 'K' && strcmp(b->herkunft, b->logincall))
  {
    putf("\007Warning:\n");
    putf("Don't erase in foreign boxes without a strict reason!\n");
    if (janein("Do you really want to erase this message globally") != JA)
      return 0;
  }
  if (! b->usermail && (! (reason == 'T') || checkerase))
  {
    fh = s_open(CHECKNAME, "sr+b");
    if (fh != EOF)
    {
      seek_fname(fh, b->mailfname, &found, 1); // von hinten durchsuchen
      if (found)
      {
        _read(fh, b->line, BLEN);
        if (unerase)
          b->line[14] = '>';
        else
          if (! (reason == 'L'
                  && (! m.remerase
                      || (m.remerase == 2
                             && ! strstr(b->bid, eraser)))))
            b->line[14] = reason;
        lseek(fh, -(LBLEN), SEEK_CUR);
        _write(fh, b->line, BLEN);
        b->line[23] = 0;
        cut_blank(b->line + 15);
        if (! stristr(b->mailpath, b->line + 15))
        {
          strcpy(b->boardname, b->line + 15);
          strlwr(b->boardname);
          if (! finddir(b->boardname, 1))
            notfound++;
        }
      }
      else
        notfound++;
      s_close(fh);
    }
    else
    {
      notfound++;
      trace(serious, "merase", "check");
    }
  }
  waitfor(e_reschedule);
  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "sr+t")) != NULL)
  {
    fgets(b->line, BUFLEN - 1, f);     //first line = headerline
    mbsend_parse(b->line, 0);          //parse headerline
    if (m.eraselog || reason == 'L')
    {
      fgets(b->line, BUFLEN - 1, f);   //read forward-line
      if (*b->line == '*' && ! unerase)
      {
        s_fclose(f);
        return NO;
      }
      fgets(b->line, BUFLEN - 1, f);    // read "readers" line
      fgets(b->betreff, BETREFFLEN, f); // read subject-line
      cut_blank(b->betreff);             // newline entfernen
      if (reason == 'L')
      {
        if (! strstr(b->bid, eraser))
        {
          if (m.remerase == 2) doerase = 0;
          reason = 'X';
        }
        if (! m.remerase) doerase = 0;
      }
      eraselog(reason, eraser);
      if (! doerase)
      {
        s_fclose(f);
        return NO;
      }
      fseek(f, 0, SEEK_SET);
      fgets(b->line, BUFLEN - 1, f); // nach Headerzeile positionieren
    }
    fwdpos = ftell(f);
    fgets(b->line, BUFLEN - 1, f); // Forwardzeile einlesen
    remerase = (strchr(b->line + 1, '*') != NULL);
    // nur Nachrichten fuer Fernloeschen freigeben, die tatsaechlich
    // per Forward rausgegangen sind
    len = strlen(b->line) - 1;
    for (i = 0; i < len; i++) b->line[i] = '.';
    // remove fwd-partners from line, seems to be obsolete,
    // now seperate trigger-file is used
    if (! unerase)
    {
      b->line[0] = '*';
      b->line[1] = reason; // store reason for erase
    }
    fseek(f, fwdpos, SEEK_SET);
/*
     // nachfolgend "SP SYSOP" nicht markieren
      int sysopmarkierung = 1;
      sysopmarkierung = strcmp(b->ziel, "SYSOP");
      trace(report, "markerased", "sysopmarkierung: %d", sysopmarkierung);
      if ((b->mailtype = 'P') && (reason == 'F') && (sysopmarkierung == 0))
      {
        //s_fclose(f);
        trace(report, "markerased", "not deleted: %s, %s", b->mailpath, b->ziel);
//      return NO;
      }
      else
*/
    fwrite(b->line, len, 1, f); // Forwardzeile zurueckschreiben
    s_fclose(f);
  }
  else notfound++;
  waitfor(e_reschedule);
  strlwr(b->listpath);
  if ((fh = s_open(b->listpath, "sr+b")) != EOF)
  {
    seek_fname(fh, b->mailfname, &found, 1);
    if (found)
    {
      _read(fh, b->line, BLEN);
      if (unerase)
        b->line[14] = '>';
      else
        b->line[14] = reason;
      lseek(fh, -(LBLEN), SEEK_CUR);
      _write(fh, b->line, BLEN);
      //trace(report, "merase", "found %s", b->mailpath);
    }
    else
    {
      notfound++;
      trace(report, "merase", "not found %s", b->mailpath);
    }
    s_close(fh);
  }
  else
  {
    notfound++;
    trace(report, "merase", "%s", b->listpath);
  }
  waitfor(e_reschedule);
  if (unerase)
    b->eraseinfo = '!'; //keep hold-flag as it is
  else
  {
    b->eraseinfo = reason;
    if (reason == 'F')
      b->fwdhold = 'F'; //mail will not be forwarded again after unerase
  }
  if (! notfound)
    writemailflags(); //only write flag if mail exists
  waitfor(e_reschedule);
  if (! strcmp(b->herkunft, b->logincall) || reason == 'K')
  {
    if (b->mailtype == 'B' && remerase)
    {
      switch (reason)
      {
      case 'E':
      case 'K':
        {
          char ebid[BIDLEN + 1];
          safe_strcpy(ebid, newbid());
          char at2[NAMELEN + 1];
          safe_strcpy(at2, b->at);
          subst(at2, '.', 0);
          char reason_str[4];
          sprintf(reason_str, "#%c", reason);
          addwp_e(at2, m.boxname, ebid, b->bid, b->logincall, 0,
                  reason_str, b->logincall);
          // Absender ist BOXNAME, sonst geht REMERASE = 2 nicht mehr!
          // ist ja bei E/M auch richtig so...
          sprintf(buf, "E @ THEBOX < %s $%s %s", m.boxname, ebid, b->bid);
          add_emfile(buf, b->logincall, "M", "E", "THEBOX");
        }
      }
    }
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

void marklifetime (int tage)
//*************************************************************************
//
//  Setzt die Lifetime einer Nachricht neu
//
//*************************************************************************
{
  int len;
  char found;
  FILE *f;
  handle fh;
  char *lt;
  char nlt[4];
  lastfunc("marklifetime");

  tage += (unsigned) ((ad_time() - filename2time(b->mailfname)) / DAY);
  if (tage > 999) tage = 999;
  sprintf(nlt, "%3.3d", tage);
  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "sr+t")) != NULL)
  {
    fgets(b->line, BUFLEN - 1, f); // Kopfzeile lesen
    len = strlen(b->line);
    lt = strstr(b->line, " #");
    if (lt)
    {
#ifdef USERLT
      lt += 5; //ui position of box LT
      if (lt[0] != '%' && lt[1] != '%' && lt[2] != '%' && lt[3] != '%')
        memcpy(++lt, nlt, 3);
#else
      lt += 2;
      if (lt[1] != '%' && lt[2] != '%' && lt[3] != '%')
        memcpy(lt, nlt, 3);
#endif
      fseek(f, 0, SEEK_SET);
      fwrite(b->line, len, 1, f); // Forwardzeile zurueckschreiben
    }
    s_fclose(f);
  }
  waitfor(e_reschedule);
  strlwr(b->listpath);
  if ((fh = s_open(b->listpath, "sr+b")) != EOF)
  {
    seek_fname(fh, b->mailfname, &found, 1);
    if (found)
    {
      _read(fh, b->line, BLEN);
      memcpy(b->line + 51, nlt, 3);
      lseek(fh, -(LBLEN), SEEK_CUR);
      _write(fh, b->line, BLEN);
    }
    else
      trace(serious, "msetl", "not found");
    s_close(fh);
  }
  else
    trace(serious, "msetl", "%s", b->listpath);
  waitfor(e_reschedule);
  if (! b->usermail)
  {
    if ((fh = s_open(CHECKNAME, "sr+b")) != EOF)
    {
      seek_fname(fh, b->mailfname, &found, 1);
      if (found)
      {
        _read(fh, b->line, BLEN);
        memcpy(b->line + 51, nlt, 3);
        lseek(fh, -(LBLEN), SEEK_CUR);
        _write(fh, b->line, BLEN);
      }
      s_close(fh);
    }
    else
      trace(serious, "msetl", "check");
  }
  waitfor(e_reschedule);
}

/*---------------------------------------------------------------------------*/

static void change_address (char *zielbuf)
//*************************************************************************
//
//  Changes the forward-address of a mail if necessary and
//  adds it to the forward-queue
//
//*************************************************************************
{
  char name[20];
  char found;
  FILE *f;
  handle fh;
  char *at, *db = NULL;
  char atcall_org[CALLEN+1], atcall_new[BIDLEN+1];
  int newbbs = 0;

  strcpy(name, "change_address");
  lastfunc(name);
  safe_strcpy(b->at, zielbuf);
  // check if mail is addressed to our bbs or was sent in our bbs
  if (! homeadr(zielbuf) && ! homeadr(b->frombox))
    add_fwdfile("", 0, 0); //add to triggerfile but do not change address
  // check if mybbs of user has been changed in the meantime
  if (b->usermail || (b->optplus & o_f)) // change address in the mail file
  {
    strlwr(b->mailpath);
    if ((f = s_fopen(b->mailpath, "sr+t")) != NULL)
    {
      fgets(b->line, BUFLEN - 1, f); // read the first lines
      // change the address
      at = strchr(b->line, '@'); // pointer to @
      safe_strcpy(atcall_new, atcall(zielbuf));
      safe_strcpy(atcall_org, atcall(at + 1));
      newbbs = !! strcmp(atcall_new, atcall_org);
      if (at && (strlen(at + 1) >= 24))
      {
        // backward compatibility for old mail headers
        if (strchr(b->line, '$') - at < 41)
        { //old mail header
          if (strlen(b->at) > 24) strcpy(b->at, atcall(b->at));
          sprintf(at + 1, "%-24s", b->at);
        }
        else
        { //new mail header
          sprintf(at + 1, "%-39s", b->at); //we use b->at, because zielbuf
        }                                  //has up to 60 chars
        at[strlen(at)] = ' '; // replace the 0-byte
      }
      // If it's a usermail, we will change the MID, but only
      // if the address (bbs) has changed, else the original
      // MID remains
      at = strchr(b->line, '$');
      if (b->usermail && at && newbbs)
      {
        at++;
        db = newbid();
        if (isalnum(*at) && at[strlen(db)] == ' ') // enough space?
        {
          strcpy(at, db);
          at[strlen(at)] = ' ';
        }
      }
      fseek(f, 0L, SEEK_SET);
      fputs(b->line, f); // write back the changed stuff
      fwrite(b->destboxlist, DESTBOXLEN, 1, f);
      s_fclose(f);
    }
    // Change the address in the list file
    // (only save the BBS-call, not the whole hierarchical address)
    subst1(zielbuf, '.', 0);
    if (strlen(zielbuf) > CALLEN)
    {
      trace(serious, name, "call %s too long", zielbuf);
      return;
    }
    while (strlen(zielbuf) < CALLEN) strcat(zielbuf, " ");
    waitfor(e_reschedule);
    strlwr(b->listpath);
    if ((fh = s_open(b->listpath, "sr+b")) != EOF)
    {
      seek_fname(fh, b->mailfname, &found, 1);
      if (found)
      {
        _read(fh, b->line, BLEN);
        memcpy(b->line + 31, zielbuf, CALLEN);
        if (b->usermail && db && *db && newbbs)
        {
          strcpy(atcall_new, db); // new BID
          memcpy(b->line + 38, atcall_new, BIDLEN);
        }
        lseek(fh, -(LBLEN), SEEK_CUR);
        _write(fh, b->line, BLEN);
      }
      else
        trace(serious, name, "not found %s", b->mailfname);
      s_close(fh);
    }
    else
      trace(serious, name, "%s", b->listpath);
  }
  waitfor(e_reschedule);
  add_fwdfile("", 0, 0); //no delay here
  //dh8ymb 18.04.03: warum hier ggf. zweifacher Aufruf?
}

/*---------------------------------------------------------------------------*/

void markread (int resetread, int dummy)
//*************************************************************************
//
//  Vermerkt in einer Nachricht, dass sie gelesen worden ist.
//  (dummy was used for replys in earlier versions)
//
//*************************************************************************
{
  lastfunc("markread");
  char line[BUFLEN];
  char *nxtpos;
  unsigned int len;
  FILE *f;
  handle fh;
  long pos;

  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "sr+b")) != NULL)
  {
    fgets(line, BUFLEN - 1, f); // Headerzeile wegmachen
    fgets(line, BUFLEN - 1, f); // Forwardzeile wegmachen
    pos = ftell(f);
    fgets(line, BUFLEN - 1, f); // 'Gelesen' Zeile einlesen
    len = strlen(line);
    if (resetread)
    {
      if (b->sysop) //dk2ui: only sysops can remove calls
      {
        unsigned int i = 0;
        while (i < (len - 1)) line[i++] = '.';
        fseek(f, pos, SEEK_SET);
        fwrite(line, len, 1, f);
      }
    }
    else
    {
      nxtpos = strstr(line, ".......");
      if (! strstr(line, b->logincall) && nxtpos)
      {
        strcpy(nxtpos, b->logincall);
        nxtpos[strlen(b->logincall)] = ' ';
        fseek(f, pos, SEEK_SET);
        fwrite(line, len, 1, f);
      }
    }
    s_fclose(f);
  }
  if (! strcmp(b->boardname, b->logincall))
  {
    strlwr(b->listpath);
    fh = s_open(b->listpath, "sr+b");
    if (fh != EOF)
    {
      strupr(b->mailfname);
      seek_fname(fh, b->mailfname, line, 1);
      if (*line)
      {
        _read(fh, b->line, BLEN);
        if (! resetread && (b->line[22] == ' ')) b->line[22] = 'R';
        if (resetread  && (b->line[22] == 'R')) b->line[22] = ' ';
        lseek(fh, -(LBLEN), SEEK_CUR);
        _write(fh, b->line, BLEN);
      }
      s_close(fh);
    }
  }
}

/*---------------------------------------------------------------------------*/

static void mbreply (int comment)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("mbreply");
  FILE *f;
  char befbuf[180];
  long nrep;
  char *eptr;
  char *bbuf;

  bbuf = (char *) t_malloc(sizeof(tvar_t) - sizeof(task_t), "repl");
  if (! bbuf) return;
  memcpy(bbuf, (char *) b + sizeof(task_t), sizeof(tvar_t) - sizeof(task_t));
  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "sr+t")) != NULL)
  {
    fgets(b->line, BUFLEN - 1, f);     // Kopfzeile lesen
    mbsend_parse(b->line, 0);
    // oe3dzw Reply ist immer eine Personal-Mail
    // andere Flags sind immer auf defaults gesetzt
    b->fwdhold = b->fwdprop = b->replied = '!';
    fgets(b->line, BUFLEN - 1, f);     // Forwardzeile einlesen
    fgets(b->line, BUFLEN - 1, f);     // READ-Zeile einlesen
    fgets(b->betreff, BETREFFLEN, f);  // Betreff-Zeile einlesen
    s_fclose(f);
    cut_blank(b->betreff);
    eptr = b->betreff; //an answer is not a question
    do
    {
      if (*eptr == '?') *eptr = ' ';
    } while (*++eptr);
    if (comment)
    {
      sprintf(befbuf, "%s", b->ziel);
      if (*b->at)
        sprintf(befbuf + strlen(befbuf), " @ %s", b->at);
      if (b->lifetime)
        sprintf(befbuf + strlen(befbuf), " #%3.3d", b->lifetime);
      else
        sprintf(befbuf + strlen(befbuf), " #%3.3d", b->boardlife_max);
      //bei Comment org. Absender im Titel hinzufuegen
      //sprintf(b->line, "Re: %s %s", b->herkunft, b->betreff);
      //das sieht sehr unschoen aus, wenn mehrere Leute jeweils
      //mit comment gemailt haben, daher Absender wieder weg, 22.04.04 DH8YMB
      sprintf(b->line, "Re: %s", b->betreff);
    }
    else
    {
      b->mailtype = 'P';
//      if (m.ltquery) sprintf(befbuf, "%s # %d", b->herkunft, b->lifetime);
//      else sprintf(befbuf, "%s ", b->herkunft);
      sprintf(befbuf, "%s #%3.3d", b->herkunft, m.userlife);
      sprintf(b->line, "Re: %s", b->betreff); //kein Absender bei Reply
    }
    if (   toupper(b->betreff[0]) == 'R'
        && toupper(b->betreff[1]) == 'E')
    {
      if (b->betreff[2] == ':')
        sprintf(b->line, "Re^2:%s", b->betreff + 3);
      else
        if (b->betreff[2] == '^')
        {
          nrep = strtol(b->betreff + 3, &eptr, 10);
          if (nrep > 0 && nrep < 1000)
            sprintf(b->line, "Re^%ld%s", nrep + 1, eptr);
        }
    }
    sprintf(befbuf + strlen(befbuf), " %.78s", b->line);
    //generierten Titel ausgeben
    putf("%s%s\n", ms(m_title), b->line);
    mbsend(befbuf, 0);
  }
  conv_t *reply_conv = b->conv;
  memcpy((char *) b + sizeof(task_t), bbuf, sizeof(tvar_t) - sizeof(task_t));
  t_free(bbuf);
  b->conv = reply_conv;
  b->sentfiles++;
  if (b->mailtype == 'P' && ! strcmp(b->ziel, b->logincall)) b->replied = '1';
  writemailflags();
}

/*---------------------------------------------------------------------------*/
#ifdef __FLAT__
static void http_put_bin_href (char *fname, long length, char *parts)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("http_put_bin_href");
  long showlength = (length >> 10) + 1;

  //size in kBytes, 0kBytes does not look nice, 1k looks better
  if (stristr(fname, ".jpg") || stristr(fname, ".gif"))
    html_putf("<IMG SRC=\"/bread/%s?read=%s+%u\" "
              "ALT=\"%s (%ld kBytes)\">",
              fname, b->boardname, b->replynum, fname, showlength);
  else if (*parts)
    html_putf("7plus file: <A HREF=\"/bread/%s?read=%s+%u\">%s</A> "
              "(%ld kBytes) - %s",
              fname, b->boardname, b->replynum, fname, showlength, parts);
  else
    html_putf("Binary file: <A HREF=\"/bread/%s?read=%s+%u\">%s</A>"
              " (%ld kBytes)",
              fname, b->boardname, b->replynum, fname, showlength);
  putflush();
}

/*---------------------------------------------------------------------------*/

static void putb64 (FILE* f, int length)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("putb64");
  static char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
                     "ghijklmnopqrstuvwxyz0123456789+/";
  long in24 = 0;
  int chr = 0;
  int out = 0;
  int c;
  int cnt = 0;

  while (((c = fgetc(f)) != EOF) && length--)
  {
    in24 <<= 8;
    in24 |= c;
    //printf("in24 %d %08lx %02x\n",chr,in24,c);
    chr++;
    if (chr == 3)
    { //printf("%08lx %02lx %02lx %02lx %02lx\n",
      //        in24,in24&63,(in24>>6)&63,(in24>>12)&63,(in24>>18)&63);
      putv(b64[(in24 >> 18) & 63]);
      putv(b64[(in24 >> 12) & 63]);
      putv(b64[(in24 >> 6) & 63]);
      putv(b64[in24 & 63]);
      in24 = 0;
      chr = 0;
      out += 4;
    }
    if (out == 76)
    {
      putv(LF);
      out = 0;
    }
    if (! ((cnt++) & 255)) waitfor(e_ticsfull);
  }
  if (chr)
  {
    in24 <<= 8 << (2 - chr);
    putv(b64[(in24 >> 18) & 63]);
    putv(b64[(in24 >> 12) & 63]);
  }
  switch (chr)
  {
    case 1: putf("=="); break;
    case 2: putf("%c=", b64[(in24 >> 6) & 63]); break;
  }
}

/*---------------------------------------------------------------------------*/

static void put_mime_msg (FILE *f, long mimeboundary, char *fname, int length)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("put_mime_msg");
  char f2name[255];

  safe_strcpy(f2name, fname);
  subst1(f2name, '/', 0);
  putf("------%08X\n", mimeboundary);
  putf("Content-Type: %s\n  name=\"%s\"\n", mimestr(fname), f2name);
  putf("Content-Transfer-Encoding: base64\n");
  putf("Content-Disposition: attachment;\n");
  putf("  filename=\"%s\"\n\n", f2name);
  //putf("Content-Length: %d Bytes\n\n",length);
  //putf("Content-Disposition: inline; filename=\"%s\"\n\n", f2name);
  putb64(f, length);
  putf("\n------%08X--\n", mimeboundary);
}

/*---------------------------------------------------------------------------*/

static void sendencbin (FILE *f, long mimeboundary)
//*************************************************************************
//
//  Sends encoded bin/7+ part (for POP3,HTTP,NNTP)
//
//*************************************************************************
{
  lastfunc("sendencbin");
  char *fname = NULL;
  unsigned long length = 0;
  unsigned long cnt = 0;
  long pos;
  char parts[8];
  char bincmd[BINCMDLEN+1];
  unsigned int i;

  strcpy(parts, "");
  pos = ftell(f);
  //determine file name and length
  if (b->binstart)
  {
    fread(bincmd, BINCMDLEN, 1, f);
    bincmd[BINCMDLEN] = 0;
    cut_blank(bincmd);
    if (strncmp("#BIN#", bincmd, 5))
      trace(fatal, "sendencbin", "not at binstart");
//    if (bincmd[strlen(bincmd) - 2] != '#')
//    {
      for (i = strlen(bincmd);
           bincmd[i] != '/'
           && bincmd[i] != '#'
           && bincmd[i] != '\\'
           && i > 0; i--);
        fname = bincmd + i + 1;
//    }
    if (! strlen(fname)) fname = "unknown.bin";
    strtok(bincmd, "#");
    length = atol(strtok(NULL, "#"));
  }
  else //if (strncmp(bincmd, START7PFLAG, 7)==0)
  {
    fgets(bincmd, BINCMDLEN, f);
    if (strncmp(START7PFLAG, bincmd, 8))
      trace(fatal, "sendencbin", "not at 7p-start");
    strncpy(parts, bincmd + 8, 3);
    strcat(parts, "/");
    strncat(parts, bincmd + 15, 3);
    fname = "unknown.7pl";
    length += strlen(bincmd);
    while (fgets(bincmd, BINCMDLEN, f) != NULL)
    {
      length += strlen(bincmd);
      if (! strncmp(bincmd, STOP7PFLAG, 10))
      {
        if ((fname = strchr(bincmd, ')')) != NULL) *fname = 0;
        else bincmd[23] = 0;
        subst1 (bincmd + 11, '/', 0);
        fname = bincmd + 11;
        break;
      }
    }
    fseek(f, pos, SEEK_SET);
  }
  if (! b->http)         //POP3, NNTP - put MIME-compliant msg ..
    put_mime_msg(f, mimeboundary, fname, length);
  else if (b->http == 2)  //HTTP - first page - create hyperlinks
      http_put_bin_href(fname, length, parts);
  else
  { //HTTP - send file (http==1)
    while (cnt < length)
    {
      i = fgetc(f);
      bputv(i);
      if (! (cnt++ & 255)) waitfor(e_ticsfull);
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/

void sendbin (FILE *f, char *mailpath)
//*************************************************************************
//
//  Sends binary part of mail (not for HTTP, POP3 etc.)
//
//*************************************************************************
{
  lastfunc("sendbin");
  unsigned long cnt = 0;
  unsigned long bps;
  char bincmd[BINCMDLEN+1];
  char binok[LINELEN+1];
  int c;
  crcthp crc_;
  time_t startt, txtime;
  unsigned long fpos = ftell(f);

  fread(bincmd, BINCMDLEN, 1, f);
  fpos += BINCMDLEN;
  if (strncmp("#BIN#", bincmd, 5))
    trace(fatal, "sendbin", "not at binstart");
  if (b->forwarding == fwd_none && (b->opt & o_x))
  {
    putf("*** %s", bincmd);
    return;
  }
#ifdef FEATURE_DIDADIT
  if (u->binmode == BINMODE_DIDADIT && (b->forwarding == fwd_none))
  {
    fileio_dida fio;
    fio.set_offset(fpos);
    putf(ms(m_startdidadit));
    putflush();
    fio.usefile(mailpath);
    bincmd[BINCMDLEN] = 0;
    cut_blank(bincmd);
    fio.set_sendname(strrchr(bincmd, '#') + 1);
    fio.tx();
  }
  else
#endif
#ifdef FEATURE_BINSPLIT
  if (u->binmode == BINMODE_BINSPLIT && (b->forwarding == fwd_none))
  {
//    fileio_binsplit fio;
    putf(ms(m_startbinsplit));
    putflush();
    bincmd[BINCMDLEN] = 0;
    cut_blank(bincmd);
    putf(ms(m_notimplemented));
//    fio.set_sendname(strrchr(bincmd, '#') + 1);
//    fio.tx();
  }
  else
#endif
  {
    if (! ((b->opt & o_q) && b->forwarding == fwd_none))
      putf("%s", bincmd);
    if (b->forwarding == fwd_none
        && strcmp(b->uplink, "Import")
        && strcmp(b->uplink, "Pocsag")
        && strcmp(b->uplink, "Mailsv"))
    { //wait for #OK# at normal user interface
      do
      {
        getline(binok, LINELEN, 1);
      }
      while (strncmp(binok, "#", 1));
    }
    else
    {
      strcpy(binok, "#OK#");
    }
    if (! strncmp(binok, "#OK#", 4))
    {
      startt = ad_time();
      while ((c = fgetc(f)) != EOF)
      {
        bputv(c);
        crc_.update(c);
        if (! (cnt++ & 255))
        {
          waitfor(e_ticsfull);
          if (b->forwarding != fwd_none) timeout(m.fwdtimeout);
          else timeout(m.usrtimeout);
        }
      }
      if (b->forwarding == fwd_none && ! (b->opt & o_q))
      {
        txtime = ad_time() - startt;
        if (! txtime)
          putf("\nBIN-TX OK %05u/%04Xh\n", crc_.result, crc_.result);
        else
        {
          bps = cnt / txtime;
          putf("\nBIN-TX OK %05u / %04Xh, %lu bit/s\n",
                             crc_.result, crc_.result, bps * 8L);
        }
      }
    }
    else
    {
      putf(ms(m_bintxabort));
    }
  }
}

/*---------------------------------------------------------------------------*/

static void read_file (int mode)
//*************************************************************************
//
//  Gibt ein READ-File zum User aus
//
//  mode: 0 = Usual output of mail to user (command READ)
//        1 = Only output of the header (commands HEADER, KOPF)
//        2 = Output only msg body (for NNTP)
//        3 = Output for HTTP-reply: Put a '>' in front of every line
//        4 = Output only bin/7+ part of mail for HTTP bread
//
//*************************************************************************
{
  lastfunc("read_file");
  time_t fntime = filename2time(b->mailfname);
  int end, fromagain = 0, toagain;
  char *line;
#ifdef __FLAT__
  long mimeboundary = 0;
#endif
  FILE *f;
  char *l2;
  unsigned int i;

  line = b->line;
  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "lrb")) != NULL)
  {
    fgets(b->line, BUFLEN - 1, f);
    mbsend_parse(line, 0);
    formoptions();
    if (! strcmp(b->uplink, "Import"))
      b->opt |= o_x; // kein BIN bei Import (Tell)
    if (mode == 1)   // nur Header
      b->opt |= o_a + o_b + o_d + o_e + o_f + o_i + o_l +
        o_m + o_p + o_r + o_t + o_u + o_w + o_y + o_z;
#ifdef __FLAT__
    else if (mode == 3) b->opt = o_x; //overwrite all options (db1ras)
    else if (mode == 4) b->opt |= o_q; //HTTP binread
#endif
    if (
#ifdef __FLAT__
        NOT_BHEADERMODE_AND mode < 2 &&
#endif
        ! (b->opt & o_q))
      putreadheader(fntime);
    waitfor(e_ticsfull);
    fgets(b->line, BUFLEN - 1, f);
    if (NOT_BHEADERMODE_AND mode == 1 && ! (b->opt & o_q))
    {
      end = 0;
      while ((end < 73) && (line[end] != '.')) end++;
      line[end] = 0;
      cut_blank(line);
      if (*line) putf("Forw: %s\n", line);
    }
    fgets(b->line, BUFLEN - 1, f);
    end = 0;
    while (end < 252 && line[end] != '.') end++;
    line[end] = 0;
    cut_blank(line);
    b->gelesen = !! strstr(line, u->call);
    if (NOT_BHEADERMODE_AND strlen(line) && (b->opt & o_r))
    {
      l2 = line;
      while (*l2)
      {
        i = strlen(l2);
        if (i > 73)
        {
          i = 66;
          while (l2[i] && l2[i] != ' ') i++;
          l2[i++] = 0;
        }
        if (mode != 2 && ! (b->opt & o_q))
        {
          if (mode < 3) putf("Read: %s\n", l2);
          else putf("X-Read: %s\n", l2);
        }
        l2 += i;
      }
    }
    fgets(b->line, BUFLEN - 1, f);
    cut_blank(line);
    line[BETREFFLEN-1] = 0;
    safe_strcpy(b->betreff, line);
    if (NOT_BHEADERMODE_AND (b->opt & o_w) && mode < 2 && ! (b->opt & o_q))
    {
      if (m.smoothheader) putf("%s\n", line);
      else putf("Subj: %s\n", line);
    }
#ifdef __FLAT__
    if ((b->headermode || b->http) && mode != 1)
      mimeboundary = scanmimetype(f); //POP3, NNTP, HTTP
#endif
    if (! (b->opt & o_q))
    {
      long bodypos = putreadpath(f, line, mode);
#ifdef __FLAT__
      if (b->headermode)
      {
        if (mode != 2) putmimeheader(line, mimeboundary);
        fgets(b->line, BUFLEN - 1, f);
      }
      char transf = 0;
#endif
      if (mode <= 1
#ifdef __FLAT__
          || b->headermode
#endif
        )
        fromagain = 0;
        toagain = 0;
        while(   (! strncasecmp(line, "from", 4) && ! fromagain)
              || (! strncasecmp(line, "to", 2) && ! toagain)
              || (! strncasecmp(line, "de", 2) && ! toagain)
              || ! strncasecmp(line, "reply-to", 8)
              || ! strncasecmp(line, "transfer", 8)
              || ! strncmp(line, "Date:", 5)
              || ! strncmp(line, "BID:", 4)
              || ! strncmp(line, "X-Transfer", 10)
              || ! strncmp(line, "X-Received", 10)
              || ! strncmp(line, "X-Info:", 7)
              || ! strncmp(line, "X-Flags:", 8)
              || ! strncmp(line, "X-BID:", 6)
              || ! strncmp(line, "X-Old-MID:", 10)
              || ! strncmp(line, "X-Originally:", 13)
              || ! strncmp(line, "X-Original-Date:", 16)
              || ! strncmp(line, "X-Transfer", 10)
              || *line == LF
              || *line == CR)
        {
#ifdef __FLAT__
          if (b->headermode)
            if (   ! strncmp(line, "X-Transfer", 10)
                || ! strncasecmp(line, "transfer", 8))
             transf = 1;
#endif
          if (*line != LF && *line != CR)
          {
            bodypos = ftell(f);
#ifdef __FLAT__
            if (! b->headermode || transf)
#endif
            {
              if (strlen(line) > 80) {
                trace(serious,
                      "read",
                      "expected hdr, already within content.");
                 bodypos = ftell(f) - strlen(line);
              } else {
                 putf("%s", line);
              }
            }
          }
          if (! strncasecmp(line, "from", 4)) fromagain = 1;
          if (! strncasecmp(line, "to", 2)) toagain = 1;
          if (! strncasecmp(line, "de", 2)) toagain = 1;
          if (! fgets(b->line, BUFLEN - 1, f)) break;
        }
      fseek(f, bodypos, SEEK_SET);
    }
    if (mode == 1 && ! (b->opt & o_q))
    {
#ifdef __FLAT__
      if (! b->headermode)
#endif
        putf("...\n");
    }
    else
    {
      long cnt = ftell(f);
      int c;
      int last = 0;
      int firstchar = TRUE;
      // This loop sends the mail
        while ((c = fgetc(f)) != EOF)
        {
          cnt++;
          if (! (b->opt & o_q))
          {
#ifdef __FLAT__
            if (mode == 3)
            {
              if (last == 0 && c == LF)
              {
                firstchar = FALSE;
                continue;
              }
              if (last == 0 || last == LF)
              {
                putv('>');
                firstchar = FALSE;
              }
            }
#endif
          if (firstchar == TRUE && c != LF && c != CR)
            putv('\n');
          firstchar = FALSE;
          putv(c);
          }
          // Test for AutoBIN-part
          if (
#ifdef __FLAT__
                  ! b->headermode && ! b->http &&
#endif
                  b->binstart && cnt == b->binstart)
          {
            if (firstchar == TRUE && c != LF && c != CR)
              putv('\n');
            firstchar = FALSE;
            sendbin(f, b->mailpath);
            s_fclose(f);
            f = NULL;
            break;
          }
#ifdef __FLAT__
          if (mimeboundary && cnt == mimeboundary)
          {
            if (firstchar == TRUE && c != LF && c != CR)
              putv('\n');
            firstchar = FALSE;
            if (mode != 3) sendencbin(f, mimeboundary);
            s_fclose(f);
            f = NULL;
            break;
          }
          //POP3/NNTP: double dots at line start
          if (b->headermode && ! (b->opt & o_q) && c == '.' && last == LF)
            putv(c);
#endif
          last = c;
          if (c == LF)
          {
            if (testabbruch()) break;
            waitfor(e_ticsfull);
          }
      }
      // Bestaetigung senden, wenn P-Mail erstes mal gelesen wurde.
      // Datei muss offen sein (ist sie nicht mehr nach BIN-read)
      if (   f && ! (b->optplus & o_g) && b->mailtype == 'P' && ! b->gelesen
          && ! strcmp(b->ziel, b->logincall) )
      {
        // Untersuchen, ob "/ack" (6 Zeichen mit CR/LF) am Mail-Ende steht.
        fseek(f, -6L, SEEK_CUR);
        fgets(b->line, 6, f);
        if (*line == LF) line++;
        if ((stristr(line, "/ack") == line) && returnmailok())
        {
          char sendcmd[80];
          char content[80];
          sprintf(sendcmd, "SA %s < %s ACK:%.30s",
                           b->herkunft, m.boxname, b->betreff);
          sprintf(content, "Your mail to %s @ %s was read on %s",
                           b->ziel, b->at, datestr(ad_time(), 10));
          genmail(sendcmd, content);
        }
      }
      if (last != LF || ! (b->opt & o_q)) putv(LF);
      if (mode == 0 || mode == 2)
      {
        b->readfiles++;
        loaduser(b->logincall, u, 1);
        u->mailread++;
        saveuser(u);
      }
    }
    if (f) s_fclose(f);
    waitfor(e_ticsfull);
  }
  else
    putf(ms(m_filenotopen), b->mailpath);
}

/*---------------------------------------------------------------------------*/

int rmemptydir (char *mask)
//*************************************************************************
//
//  Loescht eine leere Rubrik als Directory auf der Platte
//  Gibt 1 zurueck, wenn eine Rubrik unter TMP entfernt wurde
//
//*************************************************************************
{
  lastfunc("rmemptydir");
  int fileanzahl = 0;
  char *wildcard;
  DIR *d;
  struct dirent *di;

  strlwr(mask);
  sprintf(b->listpath, "%s/", mask);
  wildcard = b->listpath + strlen(b->listpath);
  if (strlen(mask) > 40 || mask[strlen(mask) - 1] == '/')
  {
    trace(serious, "rmemptydir", "format %s", mask);
    return 0;
  }
  d = opendir(mask);
  while (d && (di = readdir(d)) != NULL)
  {
    if (*di->d_name != '.')
    {
      if (strlen(di->d_name) == 7) fileanzahl++;
    }
  }
  if (d) closedir(d);
  if (! fileanzahl)
  {
    strcpy(wildcard, DIRLISTNAME);
    xunlink(b->listpath);
    *(wildcard - 1) = 0;
    rmdir(b->listpath);
    wildcard = strstr(b->listpath, "tmp/");
    if (wildcard)
    {
      if (rmboard(wildcard))
      {
        trace(serious, "rmemdir", "%s rmboard", wildcard);
        return 0;
      }
      return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int hold (unsigned pos)
//*************************************************************************
//
// Manuelle HOLD/RELEASE Funktion
//
//*************************************************************************
{
  lastfunc("hold");
  int retval = 0;

  if (parse_headerline())
  {
    if (b->fwdhold == '!' && ! (b->optplus & o_u))
    {
      b->fwdhold = 'X'; //set to hold by sysop
// dh8ymb, 14.03.03:
// An dieser Stelle fehlt ein Loeschen des Eintrags in fwd queue(s),
// sonst geht die Mail evtl dennoch zu den Partnerboxen!?
      retval = 1;
    }
    if (b->fwdhold != '!' && (b->optplus & o_u))
    {
      b->fwdhold = '!'; //not held
      // dh8ymb: nun neuer Eintrag in fwd queue files
      weiterleiten(0, ""); //benoetigt, um an b->destboxlist zu kommen
      add_fwdfile("", 0, 1); //no delay here
      retval = 1;
    }
  }
  if (retval) writemailflags();
  return retval;
}
/*---------------------------------------------------------------------------*/

int mbchange (char *selektor, int thiscmd, int interactive)
//*************************************************************************
//
// Erases,forwards,transfers,replies and sets lifetime
//
// thiscmd:     w_erase,w_unerase,w_unread,w_forward,w_transfer,w_setlifetime,
//              w_reply,w_comment,w_extract,w_hold
//              (enum mail_modify_t, mail.h)
//
// selektor:    eg. command-line buffer
//
// interactive: 1..interactive i/o 0..no interactive i/o
//
//*************************************************************************
{
  lastfunc("mbchange");
  char *line = b->line;
  unsigned files = 0, pos = 0;
  long lastpos;
  char *jobtext = m.sysopcall; //dummy init
  char *ziel;
  char zielbuf[61] = { 0 };
  handle fh;
  char limiter;
  char searchitem[31];

  strcpy(searchitem, "");
  switch (thiscmd)
  {
    case w_forward:  limiter = '@'; break;
    case w_transfer: limiter = '>'; break;
    default:         limiter = '#';
  }
  findbereich(selektor);
  scanoptions(selektor);
  if (get_searchitem(selektor, searchitem, 30)) b->bereich = 1;
  strupr(selektor);
  selektor += blkill(selektor);
  if ((ziel = strchr(selektor, limiter)) != NULL)
  {
    *ziel++ = 0;
    ziel += blkill(ziel);
  }
  else
    ziel = skip(selektor);
  if (*ziel)
  {
    if (*ziel == limiter)
    {
      ziel++;
      ziel += blkill(ziel);
    }
    if (thiscmd == w_forward)
    {
      if (strlen(ziel) > HADRESSLEN) ziel[HADRESSLEN - 1] = 0;
      safe_strcpy(b->at, ziel);
      if (weiterleiten(0, selektor) == unbekannt)
      {
        if (interactive) putf(ms(m_adrunknown), b->at);
        return 0;
      }
      strcpy(ziel, b->at);
      while (strlen(b->destboxlist) < DESTBOXLEN)
        strcat(b->destboxlist, ".");
    }
    if (strlen(ziel) > 60) ziel[60] = 0;
    safe_strcpy(zielbuf, ziel);
  }
  skip (selektor);
  if ( // ! b->beginn ||
       //dh8ymb: b->beginn nicht abpruefen, sonst geht ein "-2"-Bereich nicht!
       ! b->bereich)
  {
    if (b->replynum && ! *selektor)
    {
      b->beginn = b->ende = b->replynum;
      strcpy(selektor, b->replyboard);
    }
    else
    {
      if (interactive) putf(ms(m_givenumber));
      return 0;
    }
  }
  if (! *selektor) strcpy(selektor, b->prompt);
  fh = preparedir(selektor, 1);
  if (fh != EOF)
  {
    safe_strcpy(b->prompt, b->boardfullname);
    lastpos = filelength (fh) >> 7;
    if (! b->beginn)
    {
      b->beginn = (unsigned) lastpos - b->ende +1;
      b->ende = lastpos;
    }
    if (b->ende > (unsigned) lastpos) b->ende = (unsigned) lastpos;
    while (_read(fh, line, BLEN))
    {
      waitfor(e_ticsfull);
      pos++;
      if ((pos >= b->beginn) && (pos <= b->ende))
      {
        line[BLEN - 2] = 0;
        if (*searchitem && ! stristr(line, searchitem)) continue;
        dirline_data(line); //this changes b->at, b->gelesen, etc.
        lastpos = ltell(fh);
        s_close(fh);
        fh = EOF;
        line[7] = 0;
        safe_strcpy(b->mailfname, line);
        safe_strcpy(b->mailpath, b->mask);
        if (! strstr(b->mailpath, "*.*"))
          trace(fatal, "mbchange", "mask %s", b->mailpath);
        strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
        if (owner() || thiscmd == w_reply || thiscmd == w_comment)
        {
          switch (thiscmd)
          {
#ifdef DF3VI_EXTRACT
          case w_extract:
            extract(pos);
            files++;
            break;
#endif
          case w_erase:
            if (! b->geloescht)
            {
              if (b->optplus & o_f) files += markerased('K', 0, 0);
              else files += markerased('E', 0, 0);
            }
            jobtext = ms(m_erased);
            break;
          case w_unerase:
            if (b->geloescht)
            {
              files++;
              markerased(' ', 1, 0);
            }
            jobtext = ms(m_unerased);
            break;
          case w_unread:
            if (b->gelesen)
            {
              files++;
              markread(1, 0);
            }
            jobtext = ms(m_unread);
            break;
          case w_hold:
            if (hold(pos)) files++;
            break;
          case w_forward:
            if (*zielbuf)
            {
              if (! b->geloescht)
              {
                // OE3DZW: Abfrage nach Holdflag eingebaut
                if (   (   b->mailtype == 'B'
                        || mbhadrok(zielbuf) == 1
                        || ! strcmp(b->ziel, "SYSOP"))
                    && (   b->fwdhold == '!'
                        || b->optplus & o_h)
                   )
                {
                  safe_strcpy(b->at, ziel);
                  files++;
                  if (b->optplus & o_h)
                  //dh8ymb: hold-flag in Mail zuruecksetzen
                  {
                    b->fwdhold = '!';
                    writemailflags();
                  }
                  change_address(zielbuf);
                }
                jobtext = ms(m_forwarded);
/*
                else
                {
                  putf("Message not on HOLD status.\n");
                  return NO;
                }
*/
              }
            }
            else
            {
              putf(ms(m_syntaxforward));
              return NO;
            }
            break;
          case w_setlifetime:
            if (*zielbuf)
            {
              if (atoi(zielbuf) >= 0)
              {
                if (! b->geloescht)
                {
                  files++;
                  marklifetime(atoi(zielbuf));
                }
                jobtext = ms(m_lifetimeset);
              }
              else
              {
                putf(ms(m_lifetimelimits), zielbuf);
                return NO;
              }
            }
            else
            {
              putf(ms(m_syntaxlifetime));
              return NO;
            }
            break;
          case w_transfer:
            if (*zielbuf)
            {
              mbsend_parse(zielbuf, 2);
              //df3vi:
              //abfrage vor aufruf - sonst ggf. zich meldungen -> absturz!
              if (testmaildirectory()
                  && finddir(b->zielboard, b->sysop)
                  && ! (strlen(b->zielboard) == 1 && ! b->sysop))
              {
                switch (mbtransfer(zielbuf))
                {
                  case NO: putf(ms(m_tr_notpossible), b->ziel);
                           return NO;
                  case OK: files++;
// wird nun innerhalb mbtransfer ausgefuehrt
//                         if (! b->usermail) markerased('T', 0);
                  default: break;
                }
              }
              else
              {
                putf(ms(m_boardnotpossible), b->zielboard);
                return NO;
              }
              jobtext = ms(m_tr_ok);
            }
            else
            {
              putf(ms(m_tr_syntax));
              return NO;
            }
            break;
          case w_reply:
          case w_comment:
            mbreply(thiscmd == w_comment);
            files++;
            break;
          }
        }
        else
        {
          if (! files)
          {
            if (interactive) putf(ms(m_permissiondenied));
            return NO;
          }
        }
        waitfor(e_reschedule);
        strlwr(b->listpath);
        fh = s_open(b->listpath, "srb");
        lseek(fh, lastpos, SEEK_SET);
        if (testabbruch()) break;
      }
    }
    s_close(fh);
    if (interactive && thiscmd != w_reply
                    && thiscmd != w_comment
#ifdef DF3VI_EXTRACT
                    && thiscmd != w_extract
#endif
                    && thiscmd != w_hold )
    {
      if (files == 1) putf(ms(m_messagewillbe), jobtext);
      else if (files) putf(ms(m_messageswillbe), files, jobtext);
    }
    else
      if (interactive && (thiscmd == w_hold
#ifdef DF3VI_EXTRACT
              || thiscmd == w_extract
#endif
              ))
      {
        if (files == 1) putf(ms(m_msg_processed));
        else if (files) putf(ms(m_msges_processed), files);
      }
  }
  if (! files && interactive) putf(ms(m_nomessagefound));
  return files;
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
static void http_postmsg (char *in, char *out, int outlen)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("http_postmsg");
  int n = 0;
  char *pin = in;
  *out = 0;
  while (*in)
  {
    out[1] = 0;
    if ((n + 2) > outlen)
    {
      trace(serious, "postmsg", "too short %s (%d)", pin, outlen);
      out[n] = 0;
      return;
    }
    switch (*in)
    {
      case '#':
      case '<':
      case '>':
      case '+':
      case '&':
      case '%':
      case '?':
      case '"':
      case LF:
      case CR:
         sprintf(out, "%%%02X", (unsigned) *in);
         out += 2;
         n += 2;
         break;
      default:
         if (*in == ' ') *out = '+';
         else *out = *in;
    }
    out++;
    in++;
    n++;
  }
}

/*---------------------------------------------------------------------------*/

static void http_put_read_cmds (int mode)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("http_put_read_cmds");
  long nrep;
  char *eptr;
  char to[LINELEN+1];
  char post_subj_reply[BETREFFLEN * 2 + 5];
  char post_subj_comment[BETREFFLEN * 2 + 5 + 8];
  int lifetime;

  if (mode > 1) return;
  if (mode == 1)
  {
    html_putf("<BR><HR WIDTH=\"80%%\"><BR><CENTER>");
//  html_putf("<a href=\"%s\">Go back</a> | ", referer);
    html_putf("<a href=\"/cmd?cmd=READ+%s+%d\">Read message</a>",
              b->replyboard, b->replynum);
    html_putf("</CENTER><BR><HR WIDTH=\"100%%\"><P>\n");
    return;
  }
  // Reply
  if (toupper(b->betreff[0]) == 'R' &&
      toupper(b->betreff[1]) == 'E' &&
      b->betreff[2] == ':')
    sprintf(b->line, "Re^2:%s", b->betreff + 3);
  else
    if (toupper(b->betreff[0]) == 'R' &&
           toupper(b->betreff[1]) == 'E' &&
           b->betreff[2] == '^')
    {
      nrep = strtol(b->betreff + 3, &eptr, 10);
      if (nrep > 0 && nrep < 10000)
        sprintf(b->line, "Re^%ld%s", nrep + 1, eptr);
    }
    else
      sprintf(b->line, "Re: %s", b->betreff);
  sprintf(b->betreff, "%.50s", b->line);
  http_postmsg(b->betreff, post_subj_reply, BETREFFLEN * 2);
/*
  // Comment
  strcpy(b->betreff, betreff);
  if (toupper(b->betreff[0]) == 'R' &&
      toupper(b->betreff[1]) == 'E' &&
      b->betreff[2] == ':')
    sprintf(b->line, "Re^2: %s %s", b->herkunft, b->betreff + 3);
  else if (toupper(b->betreff[0]) == 'R' &&
           toupper(b->betreff[1]) == 'E' &&
           b->betreff[2] == '^')
       {
         if (nrep > 0 && nrep < 10000)
           sprintf(b->line, "Re^%ld: %s %s", b->herkunft, nrep + 1, eptr);
       }
       else sprintf(b->line, "Re: %s %s", b->herkunft, b->betreff);
  sprintf(b->betreff, "%.50s", b->line);
*/
  http_postmsg(b->betreff, post_subj_comment, BETREFFLEN * 2);
  if (b->lifetime)
    lifetime = b->lifetime;
  else
    lifetime = b->boardlife_max;
  strlwr(b->herkunft);
  strlwr(b->ziel);
  strlwr(b->at);
  http_postmsg(b->at, to, LINELEN);
#ifdef _GUEST
  if (strcmp(m.guestcall, b->logincall)) //dh8ymb: Guests duerfen nur lesen
  {
#endif
    html_putf("<BR><HR WIDTH=\"85%%\"><BR><CENTER>");
    html_putf("<a href=\"/send?to=%s&life=%d&subj=%s\">%s</a> | ",
              b->herkunft, m.userlife, post_subj_reply, ms(m_reply));
    html_putf("<a href=\"/send?to=%s&life=%d&subj=%s&read=%s+%d\">"
              "%s</a> | ",
              b->herkunft, m.userlife, post_subj_reply, b->replyboard,
              b->replynum, ms(m_replytext));
    html_putf("<a href=\"/send?to=%s+@%s&life=%d&subj=%s\">%s</a> | ",
              b->ziel, to, lifetime, post_subj_comment, ms(m_comment));
    html_putf("<a href=\"/send?to=%s+@%s&life=%d&subj=%s&read=%s+%d\">"
              "%s</a>",
              b->ziel, to, lifetime, post_subj_comment, b->replyboard,
              b->replynum, ms(m_commenttext));
    if (   ! strcasecmp(b->ziel, b->logincall)
        || ! strcasecmp(b->herkunft, b->logincall)
        || b->sysop) //sysop has the permission to erase all mails (db1ras)
    {
      html_putf(" | <a href=\"/send?to=%s&subj=%s&era=%s+%d\">%s</a>",
                b->herkunft, post_subj_reply, b->replyboard, b->replynum,
                ms(m_erasereply));
      html_putf(" | <a href=\"/cmd?cmd=e+%s+%d\">%s</a>",
                b->replyboard, b->replynum, ms(m_erase));
    }
    if (b->sysop)
    {
      if (b->fwdhold == '!') //HOLD function for sysop (db1ras)
        html_putf(" | <a href=\"/cmd?cmd=hold+%s+%d\">Hold</a>",
                  b->replyboard, b->replynum);
      else
        html_putf(" | <a href=\"/cmd?cmd=hold+-u+%s+%d\">"
                  "<font color=\"#ff0000\">Hold -u</font></a>",
                  b->replyboard, b->replynum);
    }
#ifdef _GUEST
  }
#endif
  html_putf("<BR><a href=\"/cmd?cmd=READ+%s+%d\">"
            "%s</a>",
            b->replyboard, (b->replynum-1), ms(m_readprevmail));
  html_putf(" | <a href=\"/cmd?cmd=READ+%s+%d\">"
            "%s</a>",
            b->replyboard, (b->replynum+1), ms(m_readnextmail));
  html_putf("</CENTER><BR><HR WIDTH=\"100%%\"><P>\n");
}
#endif

/*---------------------------------------------------------------------------*/

int mbread (char *selektor, int mode)
//*************************************************************************
//
//  Outputs one or more mails (parses the command line)
//
//  mode: 0 = Usual output of mail to user (command READ)
//        1 = Only output of the header (commands HEADER, KOPF)
//        2 = Output only msg body (for NNTP)
//        3 = Output for HTTP-reply: Put a '>' in front of every line
//        4 = Output only bin/7+ part of mail for HTTP bread
//
//*************************************************************************
{
  lastfunc("mbread");
  char *line = b->line;
  char oldline[BUFLEN];
  unsigned kopf = 0, wargeloescht = 0;
  long lastpos;
  int nachrichtgekommen = 0;
  handle fh;
  char searchitem[31];
  int readlock = 0;

  strcpy(searchitem, "");
#ifdef FBBCHECKREAD
  if (strpos(selektor, "0") == 0) // Eingabe "0" von checklinetoboard abfangen
    return NO;
#endif
  findbereich(selektor);
#ifdef FBBCHECKREAD
    if (u->fbbcheckmode && (b->beginn > FBBMSGNUMOFFSET)) //&& (! b->sysop))
    // FBB-Style Read: next lines search the real board+boardnum of mail,
    // abort this mbread call and restart mbread with real board+boardnum
    {
      mbread(checklinetoboard((FBBMSGNUMOFFSET
                               + (filesize(CHECKNAME) >> 7)
                               - b->beginn)), 0);
      return NO;
    }
#endif
  scanoptions(selektor);
  if (
#ifdef __FLAT__
      mode < 3 && ! b->headermode &&
#endif
      ! (b->optplus & o_q))
  {
    putv(LF);
    leerzeile();
  }
  if (get_searchitem(selektor, searchitem, 30)) b->bereich = 1;
  selektor += blkill(selektor);
  if (! *selektor) strcpy(selektor, b->prompt);
  else
  {
    if (! b->usermail) rubrik_completition(selektor);
    if (! *selektor) return NO;
  }
  fh = preparedir(selektor, 1);
  if (fh != EOF)
  {
    safe_strcpy(b->prompt, b->boardfullname);
    if (b->usermail || ! checkquota())
    {
#ifdef _GUEST
      if (   b->usermail
          && ! strcmp(b->logincall, m.guestcall)
          && strcmp(b->boardname, b->logincall) )
      {
      // Guests aren't allowed to read user mails, but bulletin mails
        putf("Guests aren't allowed to read foreign user mails.\n");
        return NO;
      }
#endif
      if (! b->bereich) // Wenn kein Bereich angegeben worden ist, dann
      {                 // die neueste Nachricht ausgeben.
        b->beginn = 0;
        b->ende = 1;
      }
      if (b->beginn > 0)
        lseek(fh, (long) (b->beginn - 1) << 7, SEEK_SET);
      else
      {
        seek_lastentry(fh, b->ende);
        b->ende = MAXBEREICH;
      }
      if (mbcallok(b->boardname) && strcmp(b->boardname, b->logincall))
      {
        readlock = get_readlock(b->boardname);
        if (readlock != 1 && u->rlimit) readlock = u->rlimit;
      }
      // Wenn Readlock, PW noetig fuer eigenes Board
      if (m.userpw >= 2 && ! strcmp(b->boardname, b->logincall)
          && ! b->pwok && ! b->sysop)
        readlock = get_readlock(b->boardname);
      while (_read(fh, line, BLEN)
             && ((unsigned) (ltell(fh) >> 7) <= b->ende))
      {
        line[BLEN - 2] = 0;
        if (*searchitem && ! stristr(line, searchitem)) continue;
        if (readlock)
        {
          char fromcall[CALLEN+1];
          strncpy(fromcall, line + 8, CALLEN);
          fromcall[CALLEN] = 0;
          cut_blank(fromcall);
          if (strcmp(fromcall, b->logincall) && ! b->sysop)
            if (readlock == 2 || line[22] != 'R') continue;
        }
        if (! b->usermail && checkquota()) break;
        if (line[14] == '>' || b->sysop)
        {
          lastpos = ltell(fh);
          s_close(fh);
          fh = EOF;
          line[7] = 0;
          safe_strcpy(b->mailfname, line);
          safe_strcpy(b->mailpath, b->mask);
          if (! strstr(b->mailpath, "*.*"))
            trace(fatal, "mbread", "mask %s", b->mailpath);
          strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
          if (! b->http && nachrichtgekommen && ! (b->optplus & o_q))
            putf(SEPLINE "\n");
#ifdef __FLAT__
          if (b->headermode == _POP) putf("+OK message follows\n");
#endif
          b->replynum = (unsigned) (lastpos >> 7);
          strcpy(oldline, line);
          if (! mbcallok(b->boardname) && b->optplus & o_s)
          {
            fileio_abin fio;
            strlwr(b->mailpath);
            fio.usefile(b->mailpath);
            fio.tx();
            //return OK;
          }
          else
          {
            read_file(mode);
//            return OK;
          }
          strcpy(line, oldline);
          safe_strcpy(b->replyboard, b->boardname);
          nachrichtgekommen++;
          if ((mode == 0 || mode == 2) && ! (owner() && (b->optplus & o_g)))
            markread(0, 0);
          kopf++;
          if (b->job == j_tell) break; // Tell-Begrenzung auf EINE Datei!
          if (
#ifdef __FLAT__
              ! b->http && ! b->headermode &&
#endif
              (b->opt & o_n) && owner() && ! mode && ! (b->optplus & o_q))
          {
            leerzeile();
            if (janein(ms(m_erasemessage)) == JA)
            {
              markerased('E', 0, 0);
              putf(ms(m_messageerased));
            }
          }
#ifdef __FLAT__
          if (b->http == 2) http_put_read_cmds(mode);
#endif
          if (testabbruch()) break;
          if ((fh = s_open(b->listpath, "srb")) == EOF) break;
          lseek(fh, lastpos, SEEK_SET);
        }
        else
          wargeloescht++;
      }
    }
    else kopf = 1;
    if (fh != EOF) s_close(fh);
  }
  if (! kopf && ! (b->optplus & o_q))
  {
#ifdef __FLAT__
    if (b->headermode == _POP)
      putf("-ERR no such message.\n");
    else
      if (b->headermode == _NNTP)
        putf("430 no such article found\n");
      else
#endif
        if (wargeloescht)
          putf(ms(m_messageiserased));
        else
          putf(ms(m_nomessagefound));
    return NO;
  }
  return OK;
}
