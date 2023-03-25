/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------
  Ausgabe des FTS-Befehls
  -----------------------


  Copyright (C)       Markus Baumann
                      Am Altenfelder Kreuz 43
                      59302 Oelde

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/



#include "baycom.h"

#ifdef FULLTEXTSEARCH

/*---------------------------------------------------------------------------*/

#define BUFLINES 32
#define BUFSIZE  (BUFLINES*BLEN)

/*---------------------------------------------------------------------------*/

static int ftscheckline (long unsigned lfd, char *dirline,
                         long unsigned boardnum, time_t abwann)
//*************************************************************************
//
// Gibt eine Checkzeile je nach eingestellten CHECK-Optionen aus
//
//*************************************************************************
{
  char line[140];
  time_t timed;
  int i = 0;
  unsigned int length = 73;

  timed = dirline_data(dirline);
  if (timed >= abwann || (abwann && (lfd == 1)))
  {
    sprintf(line, "%-7s> %s", b->herkunft, b->ziel);
    if (b->opt & o_x)
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
    //added -s - output of mailtype
    if (b->opt & o_s)
      sprintf(line + strlen(line), "%c ", b->mailtype);
    if (b->opt & o_i)
      sprintf(line + strlen(line), "$%-12s ", b->bid);
    if (b->opt & o_l)
      sprintf(line + strlen(line), "%3d ", b->lifetime);
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
        unsigned int j = 0;
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
  return NO;
}

/*---------------------------------------------------------------------------*/

static int ftsboardreject (char *line)
//*************************************************************************
//
// Reject nach Absender/Verteiler
//
//*************************************************************************
{
  char board[12];
  unsigned int i = 0, j;
  int nott = (u->notvisible[1] != '-');
  if (u->notvisible[1])
  {
    for (j = 0; j < 3; j++) //df3vi:
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

void mbfts (char *selektor, char *rubrik)
//*************************************************************************
//
// Durchsucht zeilenweise die CHECK.BCM nach Mailfile-Name und Boardname
// und sucht dann per GREP nach dem Suchbegriff SELEKTOR in der Mail;
// falls der Begriff in der Mail enthalten ist, wird die Mail als
// CHECKZEILE ueber die Funktion FTSCHECKLINE ausgegeben
//
//*************************************************************************
{
  long unsigned lfd = 1;
  int doprint = 0;
  handle fh = EOF;
  handle posf = EOF;
  char *buf = (char *) t_malloc(BUFSIZE, "fts");
  int z, buflen = 0, found = 0;
  long unsigned numbuf[BUFLINES*2];
  char filename[7];
  char boardname[8];
  char searchfile[FNAMELEN + 1 + 1 + 7];
  long flen;
  char era;
  FILE *sf = NULL;
  *filename = 0;
  *boardname = 0;
  *searchfile = 0;

  if (*selektor == '<' && selektor[1] == ' ') selektor++;
  selektor += blkill(selektor);
  fh = s_open (CHECKNAME, "lrb");
  if (! b->semalock && ! sema_access (CHECKNUMTMP))
  {
    if (fh == EOF)
    {
      fork(P_BACK | P_MAIL, 0, mbreorg, "c");
      goto nocheckfile;
    }
    posf = s_open (CHECKNUMNAME, "lrb");
    if (posf == EOF || filelength(posf) != 2*((filelength(fh) >> 6))) // 2* da 32bit!
    {
      fork(P_BACK | P_MAIL, 0, mbreorg, "n");
      goto nocheckfile;
    }
    if (*rubrik)
    {
      *rubrik = toupper(*rubrik);
      putf("FTS (Full Text Search) \"%s\" in \"%s\":\n\n", selektor, rubrik);
    }
    else
      putf("FTS (Full Text Search) \"%s\":\n\n", selektor);
    if (fh != EOF)
    {
      lseek(fh, 0L, SEEK_END);
      flen = ltell(fh);
      // check 3 should show checknum 1..3
      // (check 3-3 does not work then)
      if (! b->beginn || b->beginn == b->ende)
        b->beginn = 1;
      if (b->beginn <= (unsigned) (flen >> 7))
      {
        flen |= (BUFSIZE - 1);
        flen += 1;
        // Kopfzeile abhaengig von CHECK-Optionen erzeugen
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
        putflush();
#ifdef __FLAT__
        html_putf("<br>");
#endif
        while (lfd <= b->ende && ! testabbruch())
        {
        if (! buflen)
          {
            flen -= BUFSIZE;
            lseek(fh, flen, SEEK_SET);
            lseek(posf, 2*(flen >> 6), SEEK_SET); // 2* da 32bit!
            buflen = _read(fh, buf, BUFSIZE);
            _read(posf, numbuf, (BUFLINES * 2)*2); // 2* da 32bit!
          }
          if (buflen >= BLEN)
          {
            buflen -= BLEN;
            if (buf[buflen + 7] != ' ')
            {
              buf[buflen + BLEN - 2] = 0;
              era = buf[buflen + 14];
              if (*selektor)
              {
                CRegEx reg;
                char cmpline[BLEN];
                strncpy(cmpline, buf + buflen, BLEN - 1);
                cmpline[BLEN - 1] = 0;
                cut_blank(cmpline);
                strcat(cmpline, "\n");
                doprint = reg.regex_match(selektor, cmpline);
              }
              else doprint = 1;
              doprint = doprint && (era == '>'
                                    || (b->opt & (o_v | o_r)));
              doprint = doprint && (era == 'L'
                                    || era == 'X'
                                    || ! (b->opt & o_r));
              if (! *selektor)
                doprint = doprint && ! ftsboardreject(buf + buflen);
              for (z = 0; z < 7; z++)
              {
                filename[z] = buf[buflen+z];
              }
              filename[7] = 0;
              for (z = 15; z < 23; z++)
              {
                if (buf[buflen+z] == ' ') break;
                else boardname[z-15] = buf[buflen+z];
              }
              boardname[z-15] = 0;
              if (! *rubrik || ! strcmp(boardname, rubrik))
              {
                finddir(boardname, 0);
                sprintf(searchfile, "%s/%s", b->boardpath, filename);
                strlwr(searchfile);
                if ((sf = s_fopen(searchfile, "srt")) != NULL)
                {
                  s_fclose(sf);
                  if (grep(searchfile, selektor, o_i+o_c+o_x))
                  {
                    found++;
                    ftscheckline(lfd, buf+buflen, numbuf[buflen >> 7], 0);
                  }
                }
                lfd++;
              }
            }
          }
          if (! buflen && ! flen) break;
          waitfor(e_ticsfull);
        }
      }
      else putf(ms(m_checkentries), ltell(fh) >> 7);
    }
    else trace(serious, "mbfts", "fopen "CHECKNAME" errno=%d %s",
               errno, strerror(errno));
  }
  else
  {
    nocheckfile:
    putf(ms(m_checknoaccess));
  }
  if (! found) putf(ms(m_notfound), selektor);
  if (fh != EOF) s_close(fh);
  if (posf != EOF) s_close(posf);
  t_free(buf);
}

#endif
