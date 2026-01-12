/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------
  Ausgabe des CHECK-Befehls
  -------------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980118 OE3DZW removed sec in timestr
//19980223 OE3DZW fixed fsetopt
//19980308 OE3DZW check for mails > 100000 bytes will show size in kbytes
//19980406 hrx    added clcount(), mbcheckcount()
//19980411 hrx    changed type of mbcheckcount() to long
//19980615 hrx    removed checknumname/checknumtmp meta definitions, moved
//                to mail.h
//19990711 DH3MB  Renamed nexthash to nextsamehash in struct check_typ
//19991121 DF3VI  format of check-opt -f fixed
//20000116 DK2UI  (dzw) added ui-options
//20001302 DF3VI  boardrej auch fuer call, verteiler
//20011206 DF3VI  use variablename nott instead of not
//20060214 DH8YMB checknum.bcm format changed from 16bit->32bit

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#define BUFLINES 32
#define BUFSIZE  (BUFLINES*BLEN)

/*---------------------------------------------------------------------------*/

typedef struct
{ char name[9];
  unsigned long nummer;
  unsigned nextsamehash;
} check_typ;

/*---------------------------------------------------------------------------*/
static int checkmerge_cmp(const char *a, const char *b)
{
	char cmpbuf_a[9], cmpbuf_b[9];

	strncpy(cmpbuf_a + 1, a, 7);
	strncpy(cmpbuf_b + 1, b, 7);
	cmpbuf_a[8] = '\0';
	cmpbuf_b[8] = '\0';

	seek_fname_preparg(cmpbuf_a);
	seek_fname_preparg(cmpbuf_b);

	return strncmp(cmpbuf_a, cmpbuf_b, 8);
}

static int checkmerge (handle lfh, handle cfh, char *outf)
//*************************************************************************
//
//
//*************************************************************************
{
	char *lbuf = (char *)t_malloc(CBUFLEN, "ch_l");
	char *cbuf = (char *)t_malloc(CBUFLEN, "ch_c");
	unsigned llen = 0, clen = 0, lpos = 0, cpos = 0;
	FILE *of;
	char *nextl, *nextc;

	if (!lbuf || !cbuf)
		return NO;

	of = s_fopen(outf, "swb");
	if (!of) {
		trace(serious, "checkmerge",
		      "fopen %s errno=%d %s",
		      outf, errno, strerror(errno));

		return NO;
	}
	s_fsetopt(of, 1);
	setvbuf(of, NULL, _IOFBF, CBUFLEN);

	do {
		if (lpos >= llen && lfh != EOF) {
			llen = _read(lfh, lbuf, CBUFLEN);
			waitfor(e_ticsfull);
			lpos = 0;
		}
		if (lpos < llen)
			nextl = lbuf + lpos;
		else
			nextl = NULL;

		if (cpos >= clen && cfh != EOF) {
			clen = _read(cfh, cbuf, CBUFLEN);
			waitfor(e_ticsfull);
			cpos = 0;
		}

		if (cpos < clen)
			nextc = cbuf + cpos;
		else
			nextc = NULL;

		if ((nextc && !nextl) ||
		    (nextc && nextl && checkmerge_cmp(nextc, nextl) < 0)) {
			fwrite(nextc, BLEN, 1, of);
			cpos += BLEN;
		} else if ((!nextc && nextl) ||
			   (nextc && nextl && checkmerge_cmp(nextc, nextl) >= 0)) {
			fwrite(nextl, BLEN, 1, of);
			lpos += BLEN;
		}
	} while (nextl || nextc);

	s_fclose(of);
	t_free(lbuf);
	t_free(cbuf);

	return OK;
}

/*---------------------------------------------------------------------------*/

static int clcount (unsigned long lfd, char *dirline,
                    int boardnum, time_t abwann)
//*************************************************************************
//
//
//*************************************************************************
{
  time_t timed;
  timed = dirline_data(dirline);
  if (timed >= abwann || (abwann && (lfd == 1))) return 1;
  else return 0;
}

/*---------------------------------------------------------------------------*/

unsigned long mbcheckcount (void)
//*************************************************************************
//
//
//*************************************************************************
{
  time_t abwann = 0;
  b->job = j_search;
  b->optgroup = o_ch;
  b->usermail = 0;
  if (! b->lastdirnews) abwann = ad_time();
  else abwann = b->lastdirnews;
  unsigned long lfd = 1L;
  handle fh = EOF;
  handle posf = EOF;
  char *buf = (char *) t_malloc(BUFSIZE, "chek");
  int buflen = 0;
  long unsigned numbuf[BUFLINES * 2];
  long flen;

  fh = s_open(CHECKNAME, "lrb");
  if (! b->semalock && ! sema_access(CHECKNUMTMP))
  {
    if (fh == EOF) goto nocheckfile;
    posf = s_open(CHECKNUMNAME, "lrb");
    if (posf == EOF
        || filelength(posf) != 2*(filelength(fh) >> 6)) // 2* wegen 32bit!
      goto nocheckfile;
    if (fh != EOF)
    {
      lseek(fh, 0L, SEEK_END);
      flen = ltell(fh);
      b->beginn = 1;
      b->ende = MAXBEREICH;
      if (b->beginn <= (unsigned) (flen >> 7))
      {
        flen |= (BUFSIZE - 1);
        flen += 1;
        while (lfd <= (unsigned long) b->ende)
        {
          if (! buflen)
          {
            flen -= BUFSIZE;
            lseek(fh, flen, SEEK_SET);
            lseek(posf, 2*(flen >> 6), SEEK_SET); // 2* wegen 32bit!
            buflen = _read(fh, buf, BUFSIZE);
            _read(posf, numbuf, (BUFLINES * 2)*2); // 2* wegen 32bit!
          }
          if (buflen >= BLEN)
          {
            buflen -= BLEN;
            if (buf[buflen + 7] != ' ')
            {
              buf[buflen + 126] = 0;
              if (! clcount(lfd, buf+buflen, numbuf[buflen >> 7], abwann))
                goto checkcountok;
              lfd++;
            }
          }
          waitfor(e_ticsfull);
        }
      }
      else putf(ms(m_checkentries), ltell(fh) >> 7);
    }
    else trace(serious, "mbcheck", "fopen "CHECKNAME" errno=%d %s",
                                   errno, strerror(errno));
  }
  else
  {
nocheckfile:
    putf(ms(m_checknoaccess));
  }
checkcountok:
  if (fh != EOF) s_close(fh);
  if (posf != EOF) s_close(posf);
  t_free(buf);
  return lfd - 1L;
}

/*---------------------------------------------------------------------------*/

void makecheck (void)
//*************************************************************************
//
// Erstellt check.bcm Liste
//
//*************************************************************************
{
  handle lfh, cfh;
  int  tmpnum = 0;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int  level = 0;
  unsigned int i = 0, j;
  int  tmpfiles = 2;

  strcpy(tmpname3, "");
  if (! sema_lock("sendlock")) return;
  trace(replog, "makecheck", "create check list");
  if (hd_space(filesize(CHECKNAME) * 3L))
  {
    trace(serious, "makecheck", "disk full");
    return;
  }
  while (i < treelen)
  {
    lastcmd(tree[i].name);
    trace(report, "makecheck", "%s", tree[i].name);
    lfh = preparedir(tree[i++].name, 0);
    if (lfh != EOF)
    {
      cfh = EOF;
      while (cfh == EOF && i < treelen)
        cfh = preparedir(tree[i++].name, 0);
      sprintf(tmpname3, "c%d_%d.$$$", level, tmpnum++);
      if (! checkmerge(lfh, cfh, tmpname3)) goto error;
      s_close(lfh);
      if (cfh != EOF) s_close(cfh);
    }
    waitfor(e_ticsfull);
  }
  if (*tmpname3)
  {
    while (tmpfiles > 1)
    {
      i = 0;
      j = 0;
      tmpfiles = 0;
      trace(report, "makecheck", tmpname3);
      lastcmd(tmpname3);
      do
      {
        sprintf(tmpname1, "c%d_%d.$$$", level, i++);
        sprintf(tmpname2, "c%d_%d.$$$", level, i++);
        lfh = s_open(tmpname1, "srb");
        cfh = s_open(tmpname2, "srb");
        if (lfh != EOF || cfh != EOF)
        {
          sprintf(tmpname3, "c%d_%d.$$$", level + 1, j++);
          if (! checkmerge(lfh, cfh, tmpname3)) goto error;
          if (lfh != EOF) s_close(lfh);
          if (cfh != EOF) s_close(cfh);
          xunlink(tmpname1);
          xunlink(tmpname2);
          tmpfiles++;
        }
        else break;
        waitfor(e_ticsfull);
      } while(1);
      level++;
    }
    xunlink(CHECKNAME);
    xrename(tmpname3, CHECKNAME);
  }
  else
  {
    lfh = s_open(CHECKNAME, "swb"); // create empty file
    if (lfh != EOF) s_close(lfh);
  }
  error:
  sema_unlock("sendlock");
  make_checknum("reorg");
}

/*---------------------------------------------------------------------------*/

static int checkline (long unsigned lfd, char *dirline,
                      long unsigned boardnum, time_t abwann)
//*************************************************************************
//
// Gibt je nach CHECK-Option und Uebergabeparameter eine Checkzeile aus
//
//*************************************************************************
{
  char line[140];
  time_t timed;
  int i = 0;
  unsigned int j = 0;
  unsigned int length = 73;

  timed = dirline_data(dirline);
#ifdef FBBCHECKREAD
  if (! u->fbbcheckmode)
  {
#endif
  if (timed >= abwann || (abwann && (lfd == 1)))
  {
    sprintf(line, "%-7s> %s", b->herkunft, b->ziel);
    if (b->opt & o_x || b->http)
    {
      if (strlen(b->ziel) == 8 && boardnum > 999999L)
        line[strlen(line) - 1] = 0;
      while ((strlen(line)-2) < (16U + (boardnum < 100000U) + (boardnum < 10000U)
             + (boardnum < 1000U) + (boardnum < 100U) + (boardnum < 10U)))
        strcat(line, ".");
    }
    else
    {
      if (strlen(b->ziel) == 8)
      {
        if (boardnum > 9999L) i = 1;
        if (boardnum > 99999L) i = 2;
      }
      if (strlen(b->ziel) == 7 && boardnum > 99999L) i = 1;
      if (i > 0) line[strlen(line) - i] = 0;
      while (strlen(line)+1 < (16U + (boardnum < 100000U) + (boardnum < 10000U)
             + (boardnum < 1000U) + (boardnum < 100U) + (boardnum < 10U)))
        strcat(line, ".");
    }
    sprintf(line + strlen(line), "%lu ", boardnum);
    if (b->opt & o_d)
      sprintf(line + strlen(line), "%s ",
               datestr(timed, !!(b->opt & o_y) + (8 * u->dirformat)));
    if (b->opt & o_t)
      sprintf(line + strlen(line), "%-5.5s ", datestr(timed, 3));
    if (b->opt & o_f)
      sprintf(line + strlen(line), "%-6s @", b->frombox);
    if (b->opt & o_m)
      sprintf(line + strlen(line), "%-6s ", b->at);
    if (b->opt & o_b)
    {
      if (b->bytes < 100000L)
        sprintf(line + strlen(line), "%5lu ", b->bytes);
      else
        sprintf(line + strlen(line), "%4luk ", b->bytes >> 10);
    }
    if (b->opt & o_z)
      sprintf(line + strlen(line), "%4u ", b->lines);
    if (b->opt & o_s)
      sprintf(line + strlen(line), "%c ", b->mailtype);
    if (b->opt & o_i)
      sprintf(line + strlen(line), "$%-12s ", b->bid);
    if (b->opt & o_l)
      sprintf(line + strlen(line), "%3d ", b->lifetime);
    if (b->opt & o_a || b->http)
    line[71] = 0;
    if (b->opt & o_a || b->http) length--;
    if (lfd > 99999L) length--;
    b->betreff[length - strlen(line)] = 0;
    cut_blank(b->betreff);
    if (b->opt & o_w) strcat(line, b->betreff);
    //only if bulletin or option s is set
    if (b->mailtype == 'B' || (b->opt & o_s))
    {
#ifdef __FLAT__
      if (b->http)
      {
        j = 0;
        strlwr(b->ziel);
        strlwr(dirline);
        putf("%6lu ", lfd);
        html_putf("<a href=\"/cmd?cmd=READ+%s+%lu\">", b->ziel, boardnum);
        while (line[j] != ' ') putv(line[j++]);
        html_putf("</a>");
        putf("%s\n", line + j);
      }
      else
#endif
        if (b->opt & o_a)
          putf("%6lu %s\n", lfd, line);
        else
          putf("%5lu %s\n", lfd, line);
    }
    return OK;
  }
#ifdef FBBCHECKREAD
  }
  else
  {
    if (timed >= abwann || (abwann && (lfd == 1)))
    {
      sprintf(line, "%c ", b->mailtype);
      if (b->bytes < 100000L)
        sprintf(line + strlen(line), "%5lu ", b->bytes);
      else
        sprintf(line + strlen(line), "%4luk ", b->bytes >> 10);
      sprintf(line + strlen(line), "%-8s", b->ziel);
      sprintf(line + strlen(line), "@%-6s ", b->at);
      sprintf(line + strlen(line), "%-7s", b->herkunft);
      sprintf(line + strlen(line), "%9s ", datestr(timed, 17));
      line[71] = 0;
      if (b->opt & o_a || b->http) length--;
      b->betreff[length-1 - strlen(line)] = 0;
      cut_blank(b->betreff);
      strcat(line, b->betreff);
      //only if bulletin
      lfd = FBBMSGNUMOFFSET + (filesize(CHECKNAME) >> 7) - lfd;
      if (b->mailtype == 'B' )
      {
#ifdef __FLAT__
        if (b->http)
        {
          j = 0;
          strlwr(b->ziel);
          strlwr(dirline);
          putf("%6lu ", lfd);
          html_putf("<a href=\"/cmd?cmd=READ+%lu\">", lfd);
          while (line[j] != ' ') putv(line[j++]);
          html_putf("</a>");
          putf("%s\n", line + j);
        }
        else
#endif
          if (b->opt & o_a)
            putf("%6lu %s\n", lfd, line);
          else
            putf("%5lu %s\n", lfd, line);
      }
      return OK;
    }
  }
#endif //von #ifdef FBBCHECKREAD
  return NO;
}

/*---------------------------------------------------------------------------*/

static long unsigned searchboardlen (char *name, char *mailfilename)
//*************************************************************************
//
// Stellt die Anzahl an Mails in einem Board fest
//
//*************************************************************************
{
  handle fh;
  int retwert = (-1);

  fh = preparedir(name, 0);
  if (fh != EOF)
  {
    retwert = (long unsigned) (filelength(fh) >> 7);
    s_close(fh);
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

static long unsigned numofboard (int *hash,
                                  char *line, check_typ *p, int *listlen)
//*************************************************************************
//
// Gibt die Boardnummer einer Mail zu einer CHECK-Zeile zurueck,
// zunaechst wird in p[entry] die Gesamtzahl eines Boards ermittelt und
// anschliessend (pro Zeile) runtergezaehlt, falls das Board gleich ist
//
//*************************************************************************
{
  char name[DIRLEN+1];
  char mailfilename[7];
  unsigned int code, entry;

  nexttoken(line + 15, name, 8);
  strncpy(mailfilename, line, 7);
  code = strcrc(name) & 255;
  entry = hash[code];
  while (entry && (strcmp(name, p[entry].name)))
    entry = p[entry].nextsamehash;
  if (! entry)
  {
    entry = ((*listlen)++);
    if (entry >= (treelen + 50))
    {
      trace(serious, "numofboard", "num dirs %d %s", entry, name);
      return 0;
    }
    if (hash[code])
      p[entry].nextsamehash = hash[code];
    hash[code] = entry;
    p[entry].nummer = searchboardlen(name, mailfilename);
    strcpy(p[entry].name, name);
  }
  else
  {
    if (! p[entry].nummer && ! reorg_noetig)
    {
      trace(serious, "numofboard", "%s", name);
      reorg_noetig++;
      fork(P_BACK | P_MAIL, 0, mbreorg, "c");
    }
  }
  if (p[entry].nummer > 0)
    return p[entry].nummer--; // negativ: Board unsichtbar, 0: Fehler
  else return 0;
}

/*---------------------------------------------------------------------------*/

static int boardreject (char *line)
//*************************************************************************
//
// df3vi: reject nach call, verteiler
//
//*************************************************************************
{
  char board[12];
  unsigned int i = 0, j;
  int nott = (u->notvisible[1] != '-');
  if (u->notvisible[1])
  {
    for (j = 0; j < 3; j++)
    {
      switch (j)
      {
        case 0: i = 8;
          strncpy(board + 1, line + 15, i);
          break;
        case 1: i = 7;
          //Auf Absender pruefen " <CALL "
          strncpy(board + 1, line + 7, i);
          break;
        case 2: i = 7;
          //Auf Verteiler pruefen " @ADR "
          strncpy(board + 1, line + 30, i);
          break;
      }
      *board = ' ';
      board[i + 1] = 0;
      cut_blank(board);
      strcat(board, " ");
      if (strstr(u->notvisible, board)) return nott;
    }
  }
  return ! nott;
}

/*---------------------------------------------------------------------------*/

void make_checknum (char *from)
//*************************************************************************
//
// baut checknum.bcm komplett neu auf
// Format: 4 Byte binaer aneinandergehaengt
//
//*************************************************************************
{
  char line[BLEN];
  long unsigned boardnr = 0;
  handle fh;
  unsigned filepos;
  int listlen = 1;
  int sysop = b->sysop;

  if (sema_access(CHECKNUMTMP)) return;
  if (! sema_lock("sendlock")) return;
  FILE *posf = s_fopen(CHECKNUMTMP, "lwb");
  if (! posf) return;
  b->sysop = 1; //for F-Board etc.
  s_fsetopt(posf, 1);
  trace(replog, from, "create check numbers");
  lastcmd("checknum");
  check_typ *cp=(check_typ*) t_malloc(sizeof(check_typ)*(treelen+52), "c_bo");
  int *hash = (int*) t_malloc(256 * sizeof(int), "c_ha");
  memset(cp, 0, sizeof(check_typ) * (treelen+52));
  // DH3MB: Changed value from 2 to 52   ^^
  memset(hash, 0, 256 * sizeof(int)); // Hash loeschen
  fh = s_open(CHECKNAME, "lrb");
  if (! b->semalock)
  {
    if (fh == EOF && ! sema_test("purgereorg"))
    {
      makecheck();
      fh = s_open(CHECKNAME, "lrb");
    }
    if (fh != EOF)
    {
      if (lseek(fh, 0L, SEEK_END))
      {
        lseek(fh, -(LBLEN), SEEK_END);
        filepos = unsigned (ltell(fh) >> 7);
        do
        {
          _read(fh, line, BLEN);
          if (line[7] != ' ')
          {
            boardnr = numofboard(hash, line, cp, &listlen);
            fseek(posf, 2*(((long) filepos) << 1),
                        SEEK_SET); // 2* wegen 32bit!
            fwrite(&boardnr, 4, 1, posf); // 4 Byte=32 bit schreiben
          }
          if (! filepos) break;
          lseek(fh, -(LBLEN << 1), SEEK_CUR);
          filepos--;
          waitfor(e_ticsfull);
        } while (1);
      }
      s_close(fh);
    }
  }
  t_free(cp);
  t_free(hash);
  s_fclose(posf);
  xunlink(CHECKNUMNAME);
  xrename(CHECKNUMTMP, CHECKNUMNAME);
  sema_unlock("sendlock");
  b->sysop = sysop;
}

/*---------------------------------------------------------------------------*/

void purgecheck (char *)
//*************************************************************************
//
// loescht aus check.bcm geloeschte Mails und ruft anschliessend
// make_checknum auf
//
//*************************************************************************
{
  handle fi = s_open(CHECKNAME, "srb");
  handle fo = s_open(CHECKTMPNAME, "swb");
  char line[BLEN];

  if (! sema_lock("sendlock")) return;
  if (hd_space(filelength(fi)))
    trace(serious, "purgecheck", "disk full");
  else
    if (fi != EOF && fo != EOF)
    {
      while (_read(fi, line, BLEN) == BLEN)
      {
        if (line[7] != ' ') _write(fo, line, BLEN);
        waitfor(e_ticsfull);
      }
      s_close(fi);
      s_close(fo);
      xunlink(CHECKNAME);
      xrename(CHECKTMPNAME, CHECKNAME);
    }
  sema_unlock("sendlock");
  mbreorg("c");
  make_checknum("mbpurge");
}

/*---------------------------------------------------------------------------*/

void mbcheck (char *selektor, time_t abwann)
//*************************************************************************
//
// Sucht nach Checkeintraegen und gibt diese ueber "checkline" aus
//
//*************************************************************************
{
  long unsigned lfd = 1;
  int doprint = 0;
  handle fh = EOF;
  handle posf = EOF;
  char *buf = (char *) t_malloc(BUFSIZE, "chek");
  int buflen = 0;
  long unsigned numbuf[BUFLINES*2];
  long flen;
  char era;

  if (*selektor == '<' && selektor[1] == ' ') selektor++;
  selektor += blkill(selektor);
  fh = s_open(CHECKNAME, "lrb");
  if (! b->semalock && ! sema_access(CHECKNUMTMP))
  {
    if (fh == EOF)
    {
      fork(P_BACK | P_MAIL, 0, mbreorg, "c");
      goto nocheckfile;
    }
    posf = s_open(CHECKNUMNAME, "lrb");
    if (posf == EOF
        || filelength(posf) != 2*(filelength(fh) >> 6)) // 2* wegen 32bit!
    {
      trace(replog, "mbcheck", "checknum.bcm not valid, force REORG N");
      fork(P_BACK | P_MAIL, 0, mbreorg, "n");
      goto nocheckfile;
    }
    if (fh != EOF)
    {
      lseek(fh, 0L, SEEK_END);
      flen = ltell(fh);
      // check 3 should show checknum 1..3
      // (check 3-3 does not work then)
      if (! b->beginn || b->beginn == b->ende) b->beginn = 1;
      if (b->beginn <= (unsigned) (flen >> 7))
      {
        flen |= (BUFSIZE - 1);
        flen += 1;
#ifdef FBBCHECKREAD
        if (! u->fbbcheckmode)
        {
#endif
          if (b->opt & o_j)
          {
            if (b->opt & o_a || b->http)
              putf("     # ");
            else
              putf("    # ");
            putf(ms(m_list_call));
            putf("  ");
            putf(ms(m_list_board));
            putf(" ");
            if (b->opt & o_x || b->http)
              putf("   ");
            putf(ms(m_list_nr));
            if (b->opt & o_d) putf(ms(m_list_date));
            if (b->opt & o_y) putf("   ");
            if (b->opt & o_t) putf(ms(m_list_time));
            if (b->opt & o_f) putf(ms(m_list_forw));
            if (b->opt & o_m) putf(ms(m_list_mbx));
            if (b->opt & o_f) putf(" ");
            if (b->opt & o_b) putf(ms(m_list_bytes));
            if (b->opt & o_z) putf(ms(m_list_lines));
            if (b->opt & o_s) putf("T ");
            if (b->opt & o_i) putf(ms(m_list_bid));
            if (b->opt & o_l) putf(ms(m_list_life));
            if (b->opt & o_w) putf(ms(m_list_subject));
            putf("\n");
          }
#ifdef FBBCHECKREAD
        }
        else
          putf("#MSG   T Bytes To      @BBS    From   Date      Title\n\n");
#endif
        while (lfd <= b->ende && ! testabbruch())
        {
          if (! buflen)
          {
            flen -= BUFSIZE;
            lseek(fh, flen, SEEK_SET);
            lseek(posf, 2 * (flen >> 6), SEEK_SET); // 2* wegen 32bit!
            buflen = _read(fh, buf, BUFSIZE);
            _read(posf, numbuf, (BUFLINES * 2) * 2); // 2* wegen 32bit!
          }
          if (buflen >= BLEN)
          {
            buflen -= BLEN;
            if (buf[buflen + 7] != ' ')
            {
              buf[buflen + BLEN - 2] = 0;
              era = buf[buflen + 14];
              if (*selektor)
                //doprint = !!stristr(buf+buflen, selektor); // OE3DZW
              {
                CRegEx reg;
                char cmpline[BLEN];
                strncpy(cmpline, buf + buflen, BLEN - 1);
                cmpline[BLEN - 1] = 0;
                cut_blank(cmpline);
                strcat(cmpline, "\n");
                doprint = reg.regex_match(selektor, cmpline);
              }
              else
                doprint = 1;
              doprint = doprint && (era == '>'
                                    || (b->opt & (o_v | o_r)));
              doprint = doprint && (era == 'L'
                                    || era == 'X'
                                    || ! (b->opt & o_r));
              if (! *selektor)
                doprint = doprint && ! boardreject(buf + buflen);
              if (   doprint && lfd >= b->beginn
                  && (buf[buflen + 16] != ' ' || b->sysop))
              {
                if (! checkline(lfd, buf+buflen,
                                numbuf[buflen >> 7], abwann)) break;
              }
              lfd++;
            }
          }
          if (! buflen && ! flen) break;
          waitfor(e_ticsfull);
        }
      }
      else putf(ms(m_checkentries), ltell(fh) >> 7);
    }
    else trace(serious, "mbcheck", "fopen "CHECKNAME" errno=%d %s",
                                   errno, strerror(errno));
  }
  else
  {
    nocheckfile:
    putf(ms(m_checknoaccess));
  }
  if (fh != EOF) s_close(fh);
  if (posf != EOF) s_close(posf);
  t_free(buf);
}

/*---------------------------------------------------------------------------*/

#ifdef FBBCHECKREAD

char *checklinetoboard (long unsigned nummer)
//*************************************************************************
//
// sucht im FBB-Check/Read-Modus nach einer Mail zu der
// FBB-Check-Nummer <nummer> und gibt dann den tatsaechlichen
// Boardnamen und die tatsaechliche Boardnummer als STRING zurueck
// Bei Abbruch/Mail nicht gefunden etc. wird eine "0" zurueckgegeben
//
//*************************************************************************
{
  lastfunc("checklinetoboard");
  long unsigned lfd = 1;
  handle fh2 = EOF;
  handle posf = EOF;
  char *buf = (char *) t_malloc(BUFSIZE, "chek2");
  int buflen = 0;
  long unsigned numbuf[BUFLINES*2];
  unsigned long flen;
  static char zeile[20];
  int abbruch;
  time_t timed;

  abbruch = FALSE;
  b->beginn = b->ende = nummer;
  b->bereich = 0;
  b->optgroup = o_ch;

  fh2 = s_open(CHECKNAME, "lrb");
  if (! b->semalock && ! sema_access(CHECKNUMTMP))
  {
    if (fh2 == EOF)
    {
      fork(P_BACK | P_MAIL, 0, mbreorg, "c");
      goto nocheckfile;
    }
    posf = s_open(CHECKNUMNAME, "lrb");
    if (posf == EOF
        || filelength(posf) != 2*(filelength(fh2) >> 6)) // 2* wegen 32bit!
    {
      trace(replog, "checklinetoboard",
                    "checknum.bcm not valid, force REORG N");
      fork(P_BACK | P_MAIL, 0, mbreorg, "n");
      goto nocheckfile;
    }
    if (fh2 != EOF)
    {
      lseek(fh2, 0L, SEEK_END);
      flen = ltell(fh2);
      if (b->beginn-1 <= (unsigned) (flen >> 7))
      {
        flen |= (BUFSIZE - 1);
        flen += 1;
        while (lfd <= b->ende && ! testabbruch())
        {
          if (! buflen)
          {
            flen -= BUFSIZE;
            lseek(fh2, flen, SEEK_SET);
            lseek(posf, 2*(flen >> 6), SEEK_SET); // 2* wegen 32bit!
            buflen = _read(fh2, buf, BUFSIZE);
            _read(posf, numbuf, (BUFLINES * 2)*2); // 2* wegen 32bit!
          }
          if (buflen >= BLEN)
          {
            buflen -= BLEN;
            if (buf[buflen + 7] != ' ')
            {
              buf[buflen + BLEN - 2] = 0;
              if (lfd >= b->beginn
                  && (buf[buflen + 16] != ' ' ))
              {
                  timed = dirline_data(buf+buflen);
                  if (! (timed >= 0 || (lfd == 1))) break;
              }
              lfd++;
            }
          }
          if (! buflen && ! flen) break;
          waitfor(e_ticsfull);
        }
      }
      else
      {
        putf(ms(m_checkentries), ((ltell(fh2) >> 7)+FBBMSGNUMOFFSET-1));
        abbruch = TRUE;
      }
    }
    else
    {
      trace(serious, "mbcheck", "fopen "CHECKNAME" errno=%d %s",
                                errno, strerror(errno));
      abbruch = TRUE;
    }
  }
  else
  {
    nocheckfile:
    putf(ms(m_checknoaccess));
    abbruch = TRUE;
  }
  if (! abbruch)
    sprintf(zeile, "%s %lu", b->ziel, numbuf[buflen >> 7]);
  else
    sprintf(zeile, "0");
  if (fh2 != EOF) s_close(fh2);
  if (posf != EOF) s_close(posf);
  t_free(buf);
//  trace(report, "checklinetoboard", "nr: %ld zeile: %s", nummer, zeile);
  return zeile;
}

#endif

