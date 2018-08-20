/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------------------------
  TRANSFER-Befehl: Uebertragen von Nachrichten in ein anderes Board
  -----------------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19880118 OE3DZW sec in timestr
//19980308 OE3DZW old R-lines will be ignored on transfer
//19980423 OE3DZW using newcall when transfering
//19981001 OE3DZW started bin-transfer 1 byte earlier, added date of
//                original mail
//19990715 OE3DZW newcall - invalid pointer, variable was missing
//19991126 DF3VI  Removed check for destination - done in mbchange()
//20000116 DK2UI  (dzw) added ui-options
//20001019 DK2UI  added exit label
//20020927 DF3VI  Transfer-Bug behoben
//20041114 DH8YMB Variable ZIELCALL eingefuehrt wegen Bug

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int mbtransfer (char *ziel)
//*************************************************************************
//
//  TRANSFER-Prozedur
//  (wird fuer jedes transferierte File einzeln aufgerufen)
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "mbtransfer");
  lastfunc(name);
  char *line = b->line;
  int findex;
  FILE *oldf, *newf;
  char usermail = 0;
  int retwert = NO;
  int nocp; //no-copy flag
  char oldsubject[101];
  unsigned long int oldfpos;
  char oldmsgty;
  char oldmailfname[10];
  time_t oldmsgtime;
  char old_ziel[DIRLEN+1];
  int old_usermail;
  char oldboardname[10];
  char oldmailpath[FNAMELEN+1];
  char zielcall[CALLEN+1];
  char newcall[CALLEN+1];
  int gleichesboard = NO;
  unsigned long nummer;
#ifdef USERLT
  short int old_lt;
#endif

#ifdef DEBUG_FWD
  trace(report, "mbtransfer", "ziel: %s", ziel);
#endif
  strupr(ziel);
  nexttoken(ziel, zielcall, CALLEN);
  strcpy(oldmailpath, b->mailpath);
  if (sema_test("sendlock") || m.disable)
  {
    putf(ms(m_sendnotpossible));
    return NO;
  }
  if (mbcallok(zielcall))
  { // Use newcall
    if (get_newcall(zielcall, newcall))
    {
      strcpy(zielcall, newcall);
      putf(ms(m_usingnewcall), newcall);
    }
  }
  char *bbuf = (char *) t_malloc(sizeof(tvar_t) - sizeof(task_t), "tran");
  char *oldfname;
  oldfname = (char *) t_malloc(sizeof(char) * (FNAMELEN+1), "tra2");
  if (! bbuf || ! oldfname)
    return NO;
  memcpy(bbuf, (char *) b + sizeof(task_t), sizeof(tvar_t) - sizeof(task_t));
  strlwr(b->mailpath);
  strcpy(oldfname, b->mailpath);
  strcpy(oldmailfname, b->mailfname);
  oldmsgtime = filename2time(b->mailfname);
  if ((oldf = s_fopen(oldfname, "srt")) != NULL)
  {
    fgets(b->line, BUFLEN - 1, oldf);    // Befehlszeile einlesen
    mbsend_parse(line, 0);
    if (b->mailtype == 'A'
        || (b->mailtype == 'B' && b->eraseinfo == 'T'))
    {
      retwert = NIL;
      s_fclose(oldf);
      goto error_exit;
    }
    oldmsgty = b->conttype;              // save mailflags ...(conttype)
    fgets(b->line, BUFLEN - 1, oldf);    // Forwardzeile vernichten
    fgets(b->line, BUFLEN - 1, oldf);    // Read-Zeile
    fgets(oldsubject, BETREFFLEN, oldf); // Betreff
    cut_blank(oldsubject);               // Newline entfernen
    if (mbcallok(zielcall)) *b->at = 0;  // Verteiler loeschen -> Neubestimmen
    strcpy(old_ziel, b->ziel);
#ifdef USERLT
    old_lt = b->lifetime;
    b->lifetime = 0;
#endif
    old_usermail = b->usermail;
    mbsend_parse(ziel, 2);
    if (! stricmp(b->ziel, b->mailpath+strlen(b->mailpath)-strlen(b->ziel)-8))
    {
      retwert = NIL;
      s_fclose(oldf);
      goto error_exit;
    }
    b->conttype = oldmsgty;              // set mailflags again.. (conttype)
    if (b->mailtype == 'B' && ! old_usermail)
      strcpy(b->ziel, old_ziel);
    weiterleiten(1, b->zielboard);
    waitfor(e_ticsfull);
    findex = finddir(b->zielboard, b->sysop);
    if (findex && !(strlen(b->zielboard)==1 && !b->sysop))
    {
      if (xmkdir(b->boardpath))
      {
        trace(serious, name, "mkdir %s error", b->boardpath);
        s_fclose(oldf);
        goto error_exit;
      }
      nocp = ! strcmp(b->herkunft, b->logincall) || b->sysop;
             //kein "CP " wenn Sysop!
      if (nocp || b->mailtype == 'B')
        strcpy(b->betreff, oldsubject);
      else
        sprintf(b->betreff, "CP %s: %.67s", b->logincall, oldsubject);
      //Bei Nicht-Bulletin BID neu generieren bzw. wenn BID fehlt
      if (b->mailtype != 'B'|| ! *b->bid) strcpy(b->bid, newbid());
      make_mask(b->mailpath, b->boardfullname);
      strcpy(b->mask, b->mailpath);
      // Ursprungsboardnamen aus b->mailpath holen und mit ziel vergleichen
      strcpy(oldboardname, b->boardname);
      oldmailpath[strlen(oldmailpath)-8] = 0;
      if (finddir(oldmailpath,0) > 0)
      {
        if (! strcmp(ziel, b->boardname))
          gleichesboard = YES;
      }
      else
        gleichesboard = NO;
      strcpy(b->boardname,oldboardname); // b->boardname wieder herstellen
      // bei Usermail oder gleichem Board zum Sortieren
      // Filezeit/-namen neu setzen
      if (mbcallok(zielcall) || gleichesboard)
      {
        strcpy(b->mailfname, time2filename(0));
      }
      else
        strcpy(b->mailfname, oldmailfname);
      if (! strstr(b->mailpath, "*.*"))
        trace(fatal, name, "mask %s", b->mailpath);
      strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
      strlwr(b->mailpath);
      if ((newf = s_fopen(b->mailpath, "sw+t")) != NULL)
      {
        s_fsetopt(newf, 1);
#ifdef USERLT
        set_boardlife_max(old_lt);
        b->lifetime = old_lt;
#endif
        writeheader(newf, 1);
        if (b->mailtype != 'B')
        {
          if (fgets(b->line, BUFLEN - 1, oldf))
          {
            do
            { // fputs(line,f); skip old R-lines
              waitfor(e_ticsfull);
              fgets(b->line, BUFLEN - 1, oldf);
            }
            while (! feof(oldf) && *line == 'R' && line[1] == ':');
          }
          fprintf(newf, "%s\n", makeheader(1));
          fprintf(newf, "X-Transfer: %s by %s @ %s\n",
                  datestr(ad_time(), 12), b->logincall, m.boxadress);
          fprintf(newf, "X-Original-Date: %s\n", datestr(oldmsgtime, 10));
          do
          {
            if((   ! strncasecmp(line, "from", 4)
                || ! strncmp(line, "de: ", 4)
                || ! strncmp(line, "de ", 3)
                || ! strncmp(line, "fm ", 3)) && ! nocp)
              fprintf(newf, "X-Originally %s", line);
            else if ((   ! strncasecmp(line, "to: ", 4)
                      || ! strncasecmp(line, "to ", 3)) && ! nocp)
              fprintf(newf, "X-Originally %s", line);
            else if (! strncmp(line, "X-MID: ", 7))
              fprintf(newf, "X-Old-MID: %s", line + 7);
            else if (! strncmp(line, "X-BID: ", 7))
              fprintf(newf, "X-Old-BID: %s", line + 7);
            else fputs(line, newf);
            waitfor(e_ticsfull);
            fgets(b->line, BUFLEN - 1, oldf);
          }
          while (! feof(oldf) && *line != LF);
          fputc(LF, newf);
        }
        //Files binaer behandeln
        oldfpos = ftell(oldf);
        s_fclose(oldf);
        s_fclose(newf);
        writemailflags();
        if ((oldf = s_fopen(oldfname, "srb")) != NULL)
        {
          if ((newf = s_fopen(b->mailpath, "sab")) != NULL) //append to file
          {
            fseek(oldf, oldfpos, SEEK_SET);
            do
            { //oe3dzw: Transferroutine transparent
              int len;
              len = fread(line, 1, 255, oldf);
              if (len) fwrite(line, 1, len, newf);
              waitfor(e_ticsfull);
            }
            while (! feof(oldf));
            if (b->binstart)  //bei binaeren Mails offset neu berechnen
            {
              b->binstart += ftell(newf) - ftell(oldf);
              if (b->binstart < 0)
                trace(serious, name, "offset %ld in %s", b->binstart, b->mailpath);
            }
            s_fclose(newf);
            if (b->binstart) writelines(); //Neuen Binstart speichern
            if (b->bytes)
            {
              if (old_usermail || gleichesboard)
                nummer = appenddirlist(1);
              else
                nummer = appenddirlist(0); //change old CHECKLINE
              if (b->usermail)
              {
                add_fwdfile("", get_fdelay(b->herkunft), 0);
                sprintf(b->line, "%s %lu", b->herkunft, nummer);
                mbtalk("\001", b->zielboard, b->line);
                trigger_ufwd(b->zielboard);
              }
              else
              {
                handle fh;
                char found;
                if (old_usermail)
                  add_fwdfile("", get_fdelay(b->herkunft), 0);
                else
                  add_fwdfile("", get_fdelay(b->herkunft), 1);
                if (findex > 0)
                  tree[findex - 1].newestmail = oldmsgtime;
                if ((fh = s_open(CHECKNAME, "sr+b")) != EOF)
                {
                  seek_fname(fh, b->mailfname, &found, 1);
                  long pos = ltell(fh);
                  if (found)
                  {
                    _read(fh, b->line, BLEN);
                    sprintf(b->line + 15, "%-8s", b->boardname);
                    b->line[23] = '~';
#ifdef USERLT
                    char nlt[4];
                    sprintf(nlt, "%3.3d", b->boardlife_max);
                    memcpy(b->line + 51, nlt, 3);
#endif
                    lseek(fh, -(LBLEN), SEEK_CUR);
                    _write(fh, b->line, BLEN);
                  }
                  s_close(fh);
                  if (found && (fh = s_open(CHECKNUMNAME, "sr+b")) != EOF)
                  {
                    lseek(fh, 2*(pos >> 6), SEEK_SET); //2* da 32bit!
                    _write(fh, &nummer, 4); // 4Byte=32 bit
                    s_close(fh);
                  }
                }
                else
                  trace(serious, name, "check");
              }
              inc_mailgot(b->boardname);
              writemailflags();
              retwert = OK;
            }
          }
          else
            trace(fatal, name, "freopen %s errno=%d %s",
                                b->mailpath, errno, strerror(errno));
        }
        else
          trace(fatal, name, "freopen %s errno=%d %s",
                              oldfname, errno, strerror(errno));
      }
      else
        trace(fatal, name, "fopen %s errno=%d %s",
                            b->mailpath, errno, strerror(errno));
    }
    s_fclose(oldf);
  }
  else
    trace(serious, name, "fopen %s errno=%d %s",
                          oldfname, errno, strerror(errno));

error_exit:
  usermail = b->usermail;
  conv_t *trans_conv = b->conv; //save conv-buffers
  memcpy((char *) b + sizeof(task_t), bbuf, sizeof(tvar_t) - sizeof(task_t));
  t_free(bbuf);
  t_free(oldfname);
  b->usermail = usermail;
  b->conv = trans_conv;
  if (retwert == OK)
    if (! b->usermail) markerased('T', 0, gleichesboard);
  return retwert;
}

/*---------------------------------------------------------------------------*/
