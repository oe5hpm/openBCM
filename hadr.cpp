/***************************************************************

  BayCom(R)   Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------
  Verwaltung von Box-Adressen
  ---------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980126 OE3DZW head -> long, output of p -s
//19980202 OE3DZW removed trailing \n in adr of hadr-file
//19980205 OE3DZW removed above fix, added \r fix for lastheader
//19980309 OE3DZW added \r,\n fix for headerline
//19980428 OE3DZW cutting off "ro" from ".euro"
//19980614 hrx    removed structure definitions hadr_t and hadr_old_t,
//                moved to mail.h.
//19990110 OE3DZW removed "newformat" from code, was commented out anyway
//19990214 OE3DZW added dummy function for future extensions
//19990817 JJ     removed static near (needed in afwd)
//20000105 OE3DZW added fwd-check
//20000107 OE3DZW replaced -e by -n in "path -sfn"
//20000118 OE3DZW text "no fwd" -> "here", sounds better
//20021212 DF3VI  show_hadr: nur noch Statistik von vorhandenen Fwd-Partnern
//                browse_hadr: "p -sfn" fix mit dummy-user 3DA

#include "baycom.h"

/*---------------------------------------------------------------------------*/

static handle fa = EOF;
static handle fah = EOF;
static int is_null = 0;

/*---------------------------------------------------------------------------*/

long hadr_len (int max)
//*************************************************************************
//
//
//
//*************************************************************************
{
  if (max)
    return 65535L;
  else
    return filesize(HADRNAME) / sizeof(hadr_t);
}

/*---------------------------------------------------------------------------*/
//#ifdef _AUTOFWD
void
//#else
//static void near
//#endif
                  uclose (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  if (fa != EOF)
  {
    s_close(fa);
    fa = EOF;
  }
  if (fah != EOF)
  {
    s_close(fah);
    fah = EOF;
  }
}

/*---------------------------------------------------------------------------*/

void hadrfile_newformat (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  if (file_isreg(HADRNAME) < file_isreg(HADROLDNAME))
  {
    FILE *fi = s_fopen(HADROLDNAME, "srb");
    FILE *fo = s_fopen(HADRNAME, "swb");
    hadrold_t *hi = (hadrold_t *) t_malloc(sizeof(hadrold_t), "haol");
    hadr_t *ho = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");

    trace(replog, "hadr", "converting format");
    wdelay(334);
    if (fi && fo)
    {
      while (fread(hi, sizeof(hadrold_t), 1, fi))
      {
        memset(ho, 0, sizeof(hadr_t));
        ho->bulletins = hi->bulletins;
        ho->usermails = hi->usermails;
        memcpy(ho->delay, hi->delay, sizeof(hi->delay));
        memcpy(ho->lastupdate, hi->lastupdate, sizeof(hi->lastupdate));
        ho->lasthtime = hi->lasthtime;
        memcpy(ho->rel_mails, hi->rel_mails, sizeof(hi->rel_mails));
        memcpy(ho->hops, hi->hops, sizeof(hi->hops));
        ho->nextsamehash = hi->nextsamehash;
        memcpy(ho->adr, hi->adr, sizeof(hi->adr));
        memcpy(ho->adjacent, hi->adjacent, sizeof(hi->adjacent));
        memcpy(ho->lastheader, hi->lastheader, sizeof(hi->lastheader));
        memcpy(ho->lastbid, hi->lastbid, sizeof(hi->lastbid));
        memcpy(ho->lastboard, hi->lastboard, sizeof(hi->lastboard));
        memcpy(ho->lastuser, hi->lastuser, sizeof(hi->lastuser));
        fwrite(ho, sizeof(hadr_t), 1, fo);
      }
    }
    if (fi) s_fclose(fi);
    if (fo) s_fclose(fo);
    t_free(hi);
    t_free(ho);
  }
}

/*---------------------------------------------------------------------------*/

static void near newhadrfile (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");

  xunlink(HADRHASHNAME);
  fa = s_open(HADRNAME, "sw+b");
  if (fa == EOF)
    trace(tr_abbruch, "hadr_tryopen", "can't create");
  memset(ha, 0, sizeof(hadr_t));
  _write(fa, ha, sizeof(hadr_t));
  s_close(fa);
  t_free(ha);
}

/*---------------------------------------------------------------------------*/

//#ifdef _AUTOFWD
void
//#else
//static void near
//#endif
                  hadr_tryopen (void)
//*************************************************************************
//
//
//
//*************************************************************************
{
  static int first = 1;

  if (sizeof(hadr_t) != 1024 || is_null)
    trace(tr_abbruch, "hadr_t", "size %d", sizeof(hadr_t));
  if (fa == EOF)
  {
    if (! file_isreg(HADRNAME) || ! filesize(HADRNAME))
    {
      if (! first) return;
      xunlink(HADRHASHNAME);
      newhadrfile();
      fa = s_open(HADRNAME, "sr+b");
    }
    else
    {
      fa = s_open(HADRNAME, "sr+b");
      if (fa == EOF) return;
    }
  }
  if (fah == EOF)
    fah = s_open(HADRHASHNAME, "sr+b");
  if (fah == EOF)
  {
    if (first)
      fah = s_open(HADRHASHNAME, "sw+b");
    else return;
  }
  if (first) hashinit(fah, HASH16);
  first = 0;
}

/*---------------------------------------------------------------------------*/

static unsigned near newhadr (hadr_t *hu, handle fau, handle fahu)
//*************************************************************************
//
//  legt einen hadr_t im HADR-File ab
//
//  Achtung: es muss an der Stelle sichergestellt sein, dass nicht
//           schon ein hadr_t mit gleichem Namen existiert!
//
//*************************************************************************
{
  lastfunc("newhadr");
  short unsigned hsh = strcrc(atcall(hu->adr));
  short unsigned idx = 0;

  lseek(fahu, (long) hsh * 2L, SEEK_SET); // Eintrag im Hashfile
  if (! _read(fahu, &idx, sizeof(short unsigned)))
    return 0;
  hu->nextsamehash = idx; // verketten
  lseek(fau, 0L, SEEK_END);
  lseek(fahu, (long) hsh * 2L, SEEK_SET);
  idx = (short unsigned) (filelength(fau) / sizeof(hadr_t));
  if (! _write(fahu, &idx, sizeof(short unsigned)))
    return 0; // zurueckschreiben
  if (! _write(fau, hu, sizeof(hadr_t)))
    return 0; // User hinausschreiben
  return idx;
}

/*---------------------------------------------------------------------------*/

static int near hdrin (char *hadrcall, handle fau, handle fahu)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("hdrin");
  short unsigned idx;
  hadr_t *hu = (hadr_t *) t_malloc(sizeof(hadr_t), "hadr");
  char atc[10];

  strcpy(atc, atcall(hadrcall));
  short unsigned hsh = strcrc(atc);
  lseek(fahu, (long) hsh * 2L, SEEK_SET);
  _read(fahu, &idx, sizeof(short unsigned));
  if (idx)
  {
    int num = 0;
    while (idx)
    {
      lseek(fau, (long) idx * sizeof(hadr_t), SEEK_SET);
      if (! _read(fau, hu, sizeof(hadr_t)))
      {
        t_free(hu);
        return 0;
      }
      if ((num++) > 200)
      {
        t_free(hu);
        return 0;
      }
      if (! strcmp(atc, atcall(hu->adr)))
      {
        t_free(hu);
        return idx;
      }
      idx = hu->nextsamehash;
    }
  }
  t_free(hu);
  return 0;
}

/*---------------------------------------------------------------------------*/
//#ifdef _AUTOFWD
int
//#else
//static int near
//#endif
                  loadhadr (char *hadrcall, hadr_t *hu, int anlegen)
//*************************************************************************
//
//
//*************************************************************************
{
  short unsigned idx;
  unsigned retwert = 0;
  char atc[10];

  strcpy(atc, atcall(hadrcall));
  short unsigned hsh = strcrc(atc);
  if (mbhadrok(hadrcall) == 1)
  {
    lseek(fah, (long) hsh * 2L, SEEK_SET);
    _read(fah, &idx, sizeof(short unsigned));
    if (idx)
    {
      int num = 0;
      while (idx)
      {
        lseek(fa, (long) idx * sizeof(hadr_t), SEEK_SET);
        if (! _read(fa, hu, sizeof(hadr_t)))
          goto ende;
        if ((num++) > 500)
          goto ende;
        if (! strcmp(atc, atcall(hu->adr)))
        {
          retwert = idx;
          goto ende;
        }
        idx = hu->nextsamehash;
      }
    }
    if (anlegen)
    {
      memset(hu, 0, sizeof(hadr_t));
      strcpy(hu->adr, hadrcall);
      retwert = newhadr(hu, fa, fah);
    }
  }
  ende:
  return retwert;
}

/*---------------------------------------------------------------------------*/

void reorg_hadr (void)
//*************************************************************************
//
//
//*************************************************************************
{
  handle ualt, uneu, uhneu;
  hadr_t *ux = (hadr_t *) t_malloc(sizeof(hadr_t), "hadr");
  unsigned pos = 0, errors = 0;
  time_t ti = ad_time(); // current UTC time

  if (m.hadrstore)
  {
    trace(replog, "reorg", "hadr");
    xunlink(HTMP);
    xunlink(HHTMP);
    ualt = s_open(HADRNAME, "lrb");
    if (hd_space(filelength(ualt)))
    {
      s_close(ualt);
      trace(serious, "reorghadr", "disk full");
      t_free(ux);
      return;
    }
    uneu = s_open(HTMP, "lw+b");
    uhneu = s_open(HHTMP, "lw+b");
    if (ualt != EOF && uneu != EOF && uhneu != EOF)
    {
      hashinit(uhneu, HASH16);
      _read(ualt, ux, sizeof(hadr_t));
      _write(uneu, ux, sizeof(hadr_t));
      while (1)
      {
        waitfor(e_ticsfull);
        if (! _read(ualt, ux, sizeof(hadr_t))) break;
        subst1(ux->lastheader, CR, 0);
        subst1(ux->lastheader, LF, 0);
        if (! *ux->adr) continue;
        if (   ux->lasthtime  < ti - (DAY * 180)   //180d
            && ux->lastwprcvd < ti - (DAY * 720) ) //2y if WP-received
          continue;
        if (   ux->adr[HADRESSLEN] || mbhadrok(ux->adr) != 1
            || hdrin(ux->adr, uneu, uhneu))
        {
          errors++;
          continue;
        }
        if (strlen(ux->lastheader) < 10)
          *ux->lastheader = 0;
        newhadr(ux, uneu, uhneu);
        pos++;
      }
      trace(replog, "reorghadr", "%u hadrs, %u errors", pos, errors);
      s_close(ualt);
      s_close(uneu);
      s_close(uhneu);
      xrename(HTMP, HADRNAME);
      xrename(HHTMP, HADRHASHNAME);
    }
  }
  t_free(ux);
}

/*---------------------------------------------------------------------------*/

int update_hadr (char *headerline, int hops, int uplink)
//*************************************************************************
//
//
// used globals
//   b->bid       (if uplink != 0)
//   b->boardname (if uplink != 0)
//   b->usermail  (if uplink != 0)
//   b->frombox   (not if hops = -1)
//
//*************************************************************************
{
  lastfunc("update_hadr");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  int i;
  char *hpos;
  char *ptr;
  char ptr2[HEADERLEN];
  unsigned pos; // position within hadr-file
  char hadr[HADRESSLEN+1]; // received H-Address

  subst1(headerline, CR, 0);
  subst1(headerline, LF, 0);
  time_t htime = getheadertime(headerline); // received time
  time_t mtime = ad_time(); // current UTC time
  time_t hdelay = mtime - htime; // delay of delivery
  int adj = (-1); // index of adjacent box call
  // if the mail is much newer than current time, the clock of the uplink-box
  // seems to be adjusted incorrectly. Add a time for always positive delay
  hdelay += 7200;
  hpos = getheaderadress(headerline);
  if (hpos && m.hadrstore)
    strcpy(hadr, hpos);
  else
  {
    t_free(ha);
    return 0;
  }
  waitfor(e_ticsfull);
  if (hadr)
  {
    strupr(hadr);
    hadr_tryopen();
    if (fa == EOF || fah == EOF)
    {
      uclose();
      t_free(ha);
      return 0;
    }
    pos = loadhadr(hadr, ha, 1);
    if (pos)
    {
      if (uplink)
      {
        if (b->usermail)
        {
          ha->usermails++;
          strcpy(ha->lastuser, b->boardname);
        }
        else
        {
          ha->bulletins++;
          strcpy(ha->lastboard, b->boardname);
        }
        if (*b->bid && strstr(b->bid, atcall(ha->adr)))
          strcpy(ha->lastbid, b->bid);
      }
      if (htime > mtime)
        htime = mtime;
      char adrname[CALLEN + 1];
      strcpy(adrname, atcall(hadr));
      if (hops >= 0 && ha->lasthtime <= htime)
        // only accept headers in ascending order
      {
        // look for an appropriate entry in list of adjacent box calls
        strcpy(ha->adr, hadr);
        for (i = 0; i < ADJNUM; i++)
        {
          if (! strcmp(b->frombox, ha->adjacent[i]))
          {
            adj = i; // found the current active call
            break;
          }
        }
        // if no such entry exists, look for an empty location in the list
        // of the adjacent box calls
        if (adj < 0)
        {
          for (i = 0; i < ADJNUM; i++)
          {
            if (! ha->adjacent[i][0])
            {
              strcpy(ha->adjacent[i], b->frombox);
              adj = i;
              break;
            }
          }
        }
        // if no empty position is left, remove the oldest position which
        // is reached through more hops or with a longer delivery time
        if (adj < 0)
        {
          time_t last = mtime;
          for (i = 0; i < ADJNUM; i++)
          { // search for the oldest entry with one of following criteria TRUE
            //  - the hop-count is greater than the current one
            //  - the delay timer is longer than the current one
            //  - the entry has not been updated for more than 30 days
            if (   ha->lastupdate[i] < last
                && (hops < ha->hops[i] || hdelay < ha->delay[i])
                || ha->lastupdate[i] < (mtime - MAXAGE) )
            {
              last = ha->lastupdate[i];
              adj = i;
            }
          }
          if (adj >= 0)
          {
            strcpy(ha->adjacent[adj], b->frombox); // change call
            ha->rel_mails[adj] = 0; // call has changed, count new
          }
          else
          {
            uclose();
            t_free(ha);
            return 0; // direction could not be updated, ignore the mail
          }
        }
        ha->rel_mails[adj]++;        // increase normalised mail counter
        if (ha->rel_mails[adj] > 50) // normalise the counters to values < 50
        {
          for (i = 0; i < ADJNUM; i++)
            ha->rel_mails[i] >>= 1;  // take the half
        }
        ha->delay[adj] = hdelay;
        ha->lastupdate[adj] = mtime;
        ha->hops[adj] = hops;
        ha->lasthtime = htime;
        // now search for the rest of the "received"-header, excluding
        // date/time and h-address. Both are stored in other members of ha.
        hpos = strstr(headerline, hadr);
        if (hpos)
        {
          hpos += strlen(hadr);
          while (*hpos == ' ') hpos++;
          // jetzt noch evtl BID und LT herausfiltern (DH8YMB)
          ptr = strstr(hpos, "$:");
          if (ptr)
          {
            strncpy(ptr2, hpos, (strlen(hpos) - strlen(ptr)));
            ptr2[(strlen(hpos) - strlen(ptr))]= 0;
            strcpy(hpos, ptr2);
          }
          ptr = strstr(hpos, "LT:");
          if (ptr)
          {
            strncpy(ptr2, hpos, (strlen(hpos) - strlen(ptr)));
            ptr2[(strlen(hpos) - strlen(ptr))]= 0;
            strcpy(hpos, ptr2);
          }
          if (strlen(hpos) > 10)
          {
            if (uplink || ! strstr(ha->lastheader, BCMLOGO))
            {
              strncpy(ha->lastheader, hpos, HEADERLEN);
              ha->lastheader[HEADERLEN] = 0;
            }
          }
        }
//    trace(report, "update_hadr", "lastheader: %s", ha->lastheader);
      }
      if (! strcmp(adrname, m.boxname))
      { //dummy for local bbs
        // folgender Dummy-Aufruf von makeheader verhindert Speicherüberlauf
        // von ha->lastheader[61] bei zu langem m.boxheader[60] (db1ras)
        makeheader(1);
        sprintf(ha->lastheader, "[%s] "BCMLOGO VNUMMER, m.boxheader);
        ha->lasthtime = htime;
        ha->lastwprcvd = htime;
        strncpy(ha->sysopcall, m.sysopcall, sizeof(ha->sysopcall) - 1);
        ha->sysopcall[sizeof(ha->sysopcall) - 1] = 0;
        strcpy(ha->protocol, "AX25");
        strncpy(ha->hwaddress, m.mycall[0], sizeof(ha->hwaddress) - 1);
        ha->hwaddress[sizeof(ha->hwaddress) - 1] = 0;
        ha->bversion = BVERSION;
        strcpy(ha->bstatus, BSTATUS);
      }
      lseek(fa, (long) pos * sizeof(hadr_t), SEEK_SET);
      _write(fa, ha, sizeof(hadr_t));
    }
  }
  uclose();
  t_free(ha);
  return 1;
}

/*---------------------------------------------------------------------------*/

void browse_hadr (char *search)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("browse_hadr");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  time_t upd;
  int i, ok, f_mode = 0;
  unsigned count = 0, found = 0;
  long head = 0;
  long f_known = 0;
  long f_active = 0;
#ifdef _AUTOFWD
  long f_auto = 0;
#endif
  long f_unknown = 0;
  long f_nofwd = 0;

  search += blkill(search);
  strupr(search);
  if (m.hadrstore)
  {
    FILE *f = s_fopen(HADRNAME, "lrb");
    if (f)
    {
      setvbuf(f, NULL, _IOFBF, 4096); // buffer for increasing performance
      fseek(f, sizeof(hadr_t), SEEK_SET);
      while (fread(ha, sizeof(hadr_t), 1, f))
      {
        if (! *ha->adr) continue;
        count++;
        ok = 0;
        if (*search)
        {
          if (! (b->optplus & o_p))
            ok += (stristr(ha->lastheader, search) != NULL);
          if (! (b->optplus & o_h))
            ok += (strstr(ha->adr, search) != NULL);
          if (b->optplus & o_r) ok++;
        }
        else
          ok++;
        if (ok)
        {
          found++;
          upd = 0;
          for (i = 0; i < ADJNUM; i++)
          {
            if (ha->adjacent[i][0] && ha->lastupdate[i] > upd)
              upd = ha->lastupdate[i];
          }
          if (b->optplus & o_f || b->optplus & o_r)
          {
            safe_strcpy(b->at, ha->adr);
            f_mode = weiterleiten(0, "3DA"); //dummy user Swaziland benutzen
        //    *search = 0;
        //    f_mode = weiterleiten(0, search);
            switch (f_mode) //update statistics
            {
              case bekannt: f_known++; break;
              case active_bekannt: f_active++; break;
#ifdef _AUTOFWD
              case auto_bekannt: f_auto++; break;
#endif
              case unbekannt: f_unknown++; break;
              case bleibtliegen: f_nofwd++; break;
            }
          }
          if (b->optplus & o_r && (f_mode != active_bekannt))
            found--;
          else
          {
            //output result, on -nf only fwd-errors
            //on -c do not output anything (count)
            if ( (! (b->optplus & o_n)
                  || (b->optplus & o_f && f_mode == unbekannt)
                 )
                 && ! (b->optplus & o_c) )
            {
              if (! (head++))
              {
                if (b->optplus & o_r)
                  putf("   Time  Address                                 Forward  Qlty Hops\n");
                else
                {
                  putf("Mails Since Address");
                  if (b->optplus & o_f) //check forwardpath (-f)
                    putf("                                Forward\n");
                  else
                    if (b->optplus & o_h)
                      putf("                Header\n");
                    else
                      putf("/Header\n");
                }
              }
              if (b->optplus & o_r)
              {
                if (*search)
                {
                  if (strstr(b->destboxlist, search) != NULL)
                    putf("%7s: %-39s %-6.6s  %5ld %3d\n",
                         zeitspanne((ad_time() - ha->r_time_rx), zs_seconds),
                         ha->adr, b->destboxlist, ha->r_qual_rx, ha->r_hops);
                  else
                    found--;
                }
                else
                  putf("%7s: %-39s %-6.6s  %5ld %3d\n",
                       zeitspanne((ad_time() - ha->r_time_rx), zs_seconds),
                       ha->adr, b->destboxlist, ha->r_qual_rx, ha->r_hops);
              }
              else
              {
              if (b->optplus & o_h)
              {
                char cut_adr[40];
                cut_adr[0] = 0;
                sprintf(cut_adr, "%s", ha->adr);
                cut_adr[22] = 0;
                putf("%5ld%5s: %-22s", ha->usermails + ha->bulletins,
                     zeitspanne(ad_time() - upd, zs_seconds), cut_adr);
              }
              else
                putf("%5ld%5s: %-39s", ha->usermails + ha->bulletins,
                     zeitspanne(ad_time() - upd, zs_seconds), ha->adr);
              if (b->optplus & o_f) //output fwd-result
                switch (f_mode)
                {
                  case bekannt: putf("%s\n", b->destboxlist); break;
                  case active_bekannt: putf("%s (Active Routing)\n", b->destboxlist);
                                     break;
#ifdef _AUTOFWD
                  case auto_bekannt: putf("%s(AutoFwd)\n", b->destboxlist);
                                     break;
#endif
                  case unbekannt: putf("-unknown-\n"); break;
                  case bleibtliegen: putf("-here-\n"); break;
                  default: putf("???\n"); //this should never happen
                }
              else
                if (b->optplus & o_h)
                  putf(" %1.44s\n", ha->lastheader);
                else
                  putf("\n            %1.44s\n", ha->lastheader);
              }
            }
          }
        }
        waitfor(e_ticsfull);
        if (testabbruch()) break;
      }
      //output summary
      if ((b->optplus & o_f) && found)
#ifndef _AUTOFWD
          putf("%ld active-routing, %ld fwd, %ld unknown, %ld nofwd, ",
               f_active, f_known, f_unknown, f_nofwd);
#else
          putf("%ld active-routing, %ld fwd, %ld autofwd, %ld unknown, %ld nofwd, ",
               f_active, f_known, f_auto, f_unknown, f_nofwd);
#endif
      putf("\n%u searched, %u found.\n", count, found);
      s_fclose(f);
    }
  }
  t_free(ha);
}

/*---------------------------------------------------------------------------*/

void showpath (char *adr, int alles)
//*************************************************************************
//
//  Zeigt die Nachbarbox(en) zur angegebenen H-Adresse an
//
//*************************************************************************
{
  lastfunc("showpath");
  int fwdmode;

  if (alles)
  {
    scanoptions(adr);
    if (b->optplus & o_s)
    {
      browse_hadr(adr);
      return;
    }
    if (b->sysop && (b->optplus & o_e))
    {
      edit_hadr(b->optplus, adr);
      return;
    }
  }
  if (*adr == '@') adr++;
  while (*adr == ' ') adr++;
  if (*adr)
  {
    strupr(adr);
    strncpy(b->at, adr, HADRESSLEN);
    b->at[HADRESSLEN] = 0;
    if (strchr(b->at, ' '))
      *strchr(b->at, ' ') = 0;
    fwdmode = weiterleiten(0, b->at);
    if (alles || fwdmode == unbekannt)
      putf(ms(m_address), b->at);
    switch (fwdmode)
    {
      case unbekannt: putf(ms(m_fwdunknown)); break;
      case bekannt:
      case active_bekannt:
#ifdef _AUTOFWD
      case auto_bekannt:
#endif
      {
        if (! alles) break;
        putf(ms(m_isknown));
        if (strlen(b->destboxlist) > 37) putf(":\n");
        putf("%s", b->destboxlist);
        if (fwdmode == active_bekannt) putf(" (Active Routing)");
#ifdef _AUTOFWD
        if (fwdmode == auto_bekannt) putf(" (AutoFwd)");
#endif
        putv(LF);
      }
      break;
      case bleibtliegen:
      if (! alles) break;
      putf(ms(m_noforwarding));
      break;
    }
    if (alles == 2 || (alles && (b->optplus & o_a)))
      show_hadr(adr);
  }
  else putf(ms(m_syntaxpath));
}

 /*---------------------------------------------------------------------------*/

void edit_hadr (bitfeld opt, char *line)
//*************************************************************************
//
//  Edit hadress entry
//  Only ONE of the following commands can be given at a time:
//
//  Syntax:
//    p -ea <h-address>        sets a new address
//                             (entry will be created if it does not exist)
//    p -ed <call>             deletes an entry
//    p -eh <call> <header>    sets a new R: header line
//    p -et <call> <date>      sets header received date to <date>
//    p -ew <call>             deletes active routing info
//
//*************************************************************************
{
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  unsigned pos;
  char call[HADRESSLEN+1];
  time_t mtime = ad_time();

  line = nexttoken(line, call, HADRESSLEN);
  strupr(call);
  if (m.hadrstore)
  {
    strupr(call);
    hadr_tryopen();
    pos = loadhadr(call, ha, 0);
    if (pos)
    {
      if (opt & o_d)
      {
        *ha->adr = 0;
        *ha->lastheader = 0;
      }
      else if (opt & o_w)
      {
        ha->r_time_rx = 0;
        ha->r_time_tx = 0;
        strcpy(ha->r_from, call);
        ha->bversion = 10;
        ha->r_hops = 1;
        ha->r_qual_rx = 0;
      }
      else if (opt & o_h)
      {
        strncpy(ha->lastheader, line, HEADERLEN);
        ha->lastheader[HEADERLEN] = 0;
        ha->lasthtime = mtime;
      }
      else if (opt & o_a)
      {
        strncpy(ha->adr, call, HADRESSLEN);
        ha->adr[HADRESSLEN] = 0;
        ha->lasthtime = mtime;
      }
      else if (opt & o_t)
        ha->lasthtime = parse_time(line);
      lseek(fa, (long) pos * sizeof(hadr_t), SEEK_SET);
      _write(fa, ha, sizeof(hadr_t));
    }
    else if (opt & o_a)
    {
      loadhadr(call, ha, 1);
      ha->lasthtime = mtime;
    }
    else
      putf(ms(m_notfound), call);
    uclose();
    show_hadr(call);
  }
  t_free(ha);
}

/*---------------------------------------------------------------------------*/

int wpupdate_hadr (wpdata_t *wp)
//*************************************************************************
//
// Store WP data received in store and forward
//
//*************************************************************************
{
  lastfunc("wpupdate_hadr");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  unsigned pos;
  int isnew = 0;
  time_t ti = ad_time();

  if (m.hadrstore)
  {
    strupr(wp->hadr);
    hadr_tryopen();
    pos = loadhadr(wp->hadr, ha, 0);
    if (pos)
    {
      if (ha->lastwprcvd < wp->mybbstime)
      {
        ha->lastwprcvd = wp->mybbstime;
        if (*wp->hadr) strcpy(ha->adr, wp->hadr);
        strupr(wp->sysopcall);
        if (*wp->sysopcall)
        {
          if (strcmp(ha->sysopcall, wp->sysopcall))
            isnew++;
          strcpy(ha->sysopcall, wp->sysopcall);
        }
        if (*wp->protocol && *wp->hwaddress)
        {
          if (   stricmp(ha->protocol, wp->protocol)
              || stricmp(ha->hwaddress, wp->hwaddress))
            isnew++;
          strcpy(ha->protocol, wp->protocol);
          strcpy(ha->hwaddress, wp->hwaddress);
        }
        if (*wp->bstatus) strcpy(ha->bstatus, wp->bstatus);
        ha->bversion = wp->bversion;
        if (isnew || ha->lastwpsend < (ti - MAXAGE))
        {
          ha->lastwpsend = ti;
          isnew++;
        }
        if (wp->hops > 50) isnew = 0;
        lseek(fa, (long) pos * sizeof(hadr_t), SEEK_SET);
        _write(fa, ha, sizeof(hadr_t));
      }
    }
    uclose();
  }
  t_free(ha);
  return isnew;
}
/*---------------------------------------------------------------------------*/

time_t find_hadr (char *call)
//*************************************************************************
//
// checks if call is found in hadr-database
// returns 0    .. not found
//         else .. time of last update
//
//*************************************************************************
{
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  unsigned pos;
  time_t stamp = 0;

  if (m.hadrstore)
  {
    strupr(call);
    hadr_tryopen();
    pos = loadhadr(call, ha, 0);
    if (pos)
    {
      stamp = ha->lastwprcvd;
      if (! stamp) stamp = ha->lasthtime;
    }
    uclose();
  }
  t_free(ha);
  return stamp;
}

/*---------------------------------------------------------------------------*/

int rupdate_hadr(wpdata_t *wp)
//*************************************************************************
//
// saves received/calculated WPROT routing infos in hadr-database
// falls Bedingungen erfuellt sind, Returnwert = 1, sonst 0
//
//*************************************************************************
{
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  unsigned pos;
  char lbuf[80];
  int is_better = 0;
  int retwert = 0;
  fwdpara_t *ff = fpara();

  strupr(wp->bbs);
  sprintf(lbuf, "%-6.6s %-6.6s %s (%ld) %ld %-3d %-5ld",
                wp->bbs, b->logincall, datestr(wp->mybbstime, 12),
                wp->mybbstime, wp->bversion, wp->hops, wp->qual);
  wprotlog(lbuf, wp->bbs);
  if (m.hadrstore)
  {
    hadr_tryopen();
    pos = loadhadr(wp->bbs, ha, 0);
    if (pos)
    {
      is_better = (wp->qual <= ha->r_qual_rx);
      if (((! strcmp(ha->r_from, b->logincall)) // falls routingcall gleich altem routingcall
            || is_better // bessere quality
            || ha->r_qual_rx == 0 // letzte quality war 0
            || ((ad_time() - ha->r_time_rx) > (72 * HOUR)) ) // letzte Info aelter 3 Tage
          && ((ha->r_time_rx < wp->mybbstime) // letzte Info ist aelter
               || ((ha->r_time_rx == wp->mybbstime) && is_better)) // oder gleich alt, aber bessere quality
          && (wp->qual > 0) // bei Defaultwert 0 nicht speichern
          && (wp->qual <= WPROT_MIN_ROUTING) // Plausible Quality?
          && ((ad_time() - wp->mybbstime) >= 0) // keine Zukunftsroutings speichern
           )
      {
        if (( isforwardpartner(wp->bbs) >= 0
              && (wp->mybbstime >= ff->lastmeasure
                  && (((wp->mybbstime - ff->lastmeasure) > 5*HOUR)
                      || (ff->lastwpr == ff->lastmeasure))))
            || (isforwardpartner(wp->bbs) == NOTFOUND))
        {
          ha->r_time_rx = wp->mybbstime;
          safe_strcpy(ha->r_from, b->logincall);
          ha->bversion = wp->bversion;
          ha->r_hops = wp->hops;
          ha->r_qual_rx = wp->qual;
          if (isforwardpartner(wp->bbs) >= 0)
          {
            ff->routing_txf = 1;
            ff->routing_quality = wp->qual;
            ff->lastmeasure = wp->mybbstime;
          }
          lseek(fa, (long) pos * sizeof(hadr_t), SEEK_SET);
          _write(fa, ha, sizeof(hadr_t));
          wprotlog(" UPDATE HADR", wp->bbs);
          retwert = 1;
        }
      }
    }
    uclose();
  }
  t_free(ha);
  wprotlog("\n", wp->bbs);
  return (retwert);
}

/*---------------------------------------------------------------------------*/

void show_hadr (char *call)
//*************************************************************************
//
// Gibt H-ADRESS-Daten zu "call" aus
//
//*************************************************************************
{
  lastfunc("show_hadr");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  int i;
  time_t mtime = ad_time(); // current UTC time
  long mindelay = DAY * 100;
  if (m.hadrstore)
  {
    strupr(call);
    hadr_tryopen();
    if (loadhadr(call, ha, 0))
    {
      uclose();
      putf("\nH-Addr:      %s\n", ha->adr);
      putf("R-Header:    %s\n", ha->lastheader);
      if (*ha->lastbid)
        putf("Last BID:    %s\n", ha->lastbid);
      if (*ha->lastboard)
        putf("Last Board:  %s\n", ha->lastboard);
      if (*ha->lastuser)
        putf("Last User:   %s\n", ha->lastuser);
      putf("Header Date: %s\n", datestr(ha->lasthtime, 12));
      if (*ha->sysopcall)
        putf("Sysop:       %s %s\n", ha->sysopcall,
                                     get_name(ha->sysopcall, 1));
      if (*ha->protocol && *ha->hwaddress)
        putf("HW-Address:  %s: %s\n", ha->protocol, ha->hwaddress);
      if (ha->bversion && *ha->bstatus && ha->lastwprcvd)
        putf("BVersion:    %ld %s (%s)\n",
             ha->bversion, ha->bstatus, datestr(ha->lastwprcvd, 12));
      if (ha->bulletins)
        putf("Bulletins:   %ld\n", ha->bulletins);
      if (ha->usermails)
        putf("Usermails:   %ld\n", ha->usermails);
      int phead = 0;
      for (i = 0; i < ADJNUM; i++)
      {
        if (ha->adjacent[i][0] && (ha->delay[i] < mindelay))
          mindelay = ha->delay[i];
      }
      for (i = 0; i < ADJNUM; i++)
      {
        if (ha->adjacent[i][0] && (isforwardpartner(ha->adjacent[i]) > -1))
        {
          if (! phead)
          {
            phead++;
            putf("Neighbour Count Update +Delay Hops\n");
          }
          putf("%-9s%6d%7s%7s%5d\n", ha->adjacent[i], ha->rel_mails[i],
               zeitspanne(mtime-ha->lastupdate[i], zs_seconds),
               zeitspanne(ha->delay[i] - mindelay, zs_seconds), ha->hops[i]);
        }
      }
      if ((b->optplus & o_r) && ha->r_time_rx)
      {
        putf("WPROT Routing Info:\n%-6.6s hops: %d, quality: %ld (%s)\n",
             ha->r_from, ha->r_hops, ha->r_qual_rx,
             datestr(ha->r_time_rx, 12));
      }
    }
    uclose();
  }
  t_free(ha);
}

/*---------------------------------------------------------------------------*/

void fwdcheck (char *call)
//*************************************************************************
//
// Checkt FWD-Einstellung zu "call"
//
//*************************************************************************
{
  lastfunc("fwdcheck");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  int hops;
  char bestfwdcall[DESTBOXLEN+1];
  char currentfwdcall[DESTBOXLEN+1];
  int i;
  int fail;
  int fwdmode;
  time_t mtime = ad_time(); // current UTC time
  long mindelay = DAY * 100;
  hops = 1000; // max. Hops
  bestfwdcall[0] = 0;
  fail = FALSE;

  scanoptions(call);
  if (*call == '@') call++;
  while (*call == ' ') call++;
  if (*call)
  {
    strupr(call);
    strncpy(b->at, call, HADRESSLEN);
    b->at[HADRESSLEN] = 0;
    if (strchr(b->at, ' ')) *strchr(b->at, ' ') = 0;
    fwdmode = weiterleiten(0, b->at);
    switch (fwdmode)
    {
     case unbekannt: putf("%s ", call);
                     putf(ms(m_fwdunknown));
                     fail = TRUE;
                     break;
     case bekannt:
     case active_bekannt:
#ifdef _AUTOFWD
     case auto_bekannt:
#endif
                     safe_strcpy(currentfwdcall, b->destboxlist);
                     cut_blank(currentfwdcall);
                     break;
     case bleibtliegen:
                     putf("%s ", call);
                     putf(ms(m_noforwarding));
                     fail = TRUE;
                     break;
    }
    if (m.hadrstore && (! fail))
    {
      strupr(call);
      hadr_tryopen();
      if (loadhadr(call, ha, 0))
      {
        uclose();
        putf("Fwd-Check:        %s (%s)\n\n", call, ha->adr);
        int phead = 0;
        for (i = 0; i < ADJNUM; i++)
        {
          if (ha->adjacent[i][0] && (ha->delay[i] < mindelay))
            mindelay = ha->delay[i];
        }
        for (i = 0; i < ADJNUM; i++)
        {
          if (ha->adjacent[i][0] && (isforwardpartner(ha->adjacent[i]) > -1))
          {
            if (! phead && (b->optplus & o_a))
            {
              phead++;
              putf("Neighbour Count Update +Delay Hops\n");
            }
            if (ha->hops[i] == hops)
            {
              strcat(bestfwdcall, " ");
              strcat(bestfwdcall, ha->adjacent[i]);
            }
            if (ha->hops[i] < hops)
            {
              hops = ha->hops[i];
              safe_strcpy(bestfwdcall, ha->adjacent[i]);
            }
            if (b->optplus & o_a)
              putf("%-9s%6d%7s%7s%5d\n", ha->adjacent[i], ha->rel_mails[i],
                   zeitspanne(mtime-ha->lastupdate[i], zs_seconds),
                   zeitspanne(ha->delay[i] - mindelay, zs_seconds), ha->hops[i]);
          }
        }
      }
      uclose();
      if (b->optplus & o_a) putv(LF);
      putf("Current Routing:        %s\n", currentfwdcall);
      putf("Possible best Routing:  %s\n", bestfwdcall);
      if (strcmp(currentfwdcall, bestfwdcall))
        putf("\nChange fwd.bcm or switch on autorouter!\n");
    }
    t_free(ha);
  }
}

/*---------------------------------------------------------------------------*/

void expand_hadr (char *adr, int force)
//*************************************************************************
//
//  Macht aus einer vom Benutzer angegebenen Adresse eine
//  (wenn moeglich) korrekte H-Adresse.
//  Wird das eigene Boxcall angegeben, so wird die eigene Boxadresse
//  eingesetzt, es sei denn es ist eine SSID ungleich dem ersten
//  Mycall angegeben.
//
//  force=1  nur eigenes Boxcall ergaenzen
//  force=2  nur ergaenzen, wenn keine komplette H-Adresse angegeben ist
//  force=3  immer ergaenzen, notfalls vorherige H-Adresse ueberschreiben
//  force=64 CB-BCMNET: nur ergaenzen, wenn die uebergebene H-Adresse
//           keine BCMNETID enthaelt oder das eigene Boxcall ist
//
//*************************************************************************
{
  lastfunc("expand_hadr");
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  int notok = 2; // mehrere Versuche, File koennte gelockt sein

  strupr(adr);
#ifdef _BCMNET_GW
  if (force == 64)
    if (   strlen(adr) > strlen(BCMNETID) //MyBBS with h-addr
        && strcmp(adr + strlen(adr) - strlen(BCMNETID), BCMNETID) == 0)
      //BCMNETID is last item
      force = 1;
    else
      force = 3;
#endif
  if (! strcmp(atcall(adr), m.boxname))
  {
    if (strchr(adr, '-') && strcmp(atcall(adr), m.mycall[0]))
    {
      t_free(ha);
      return;
    }
    strcpy(adr, m.boxadress);
  }
  else
    if (force > 1 && (! strchr(adr, '.') || force > 2))
    {
      while (notok)
      {
        notok--;
        hadr_tryopen();
        if (loadhadr(adr, ha, 0)
            && strlen(ha->adr) > 9 && strchr(ha->adr, '.'))
        {
          strcpy(adr, ha->adr);
          if (! notok)
            trace(replog, "expand_hadr", "load retry %s", adr);
          notok = 0;
        }
        uclose();
      }
    }
  //if address ends with ".euro" cut off "ro" so that ".eu" remains
  char *a;
  a = strstr(adr, ".EURO");
  if (a && (strlen(a) == 5))
    *(a + 3) = 0;
  t_free(ha);
}

/*---------------------------------------------------------------------------*/
