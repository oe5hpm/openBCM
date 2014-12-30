/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------------------
  Loeschen von Files mit abgelaufener Lifetime
  --------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980223 OE3DZW fixed fsetopt
//19980614 OE3DZW purge will remove max. 10000 bulletins (lifetime)
//20010725 DF3VI  write a return-message if usermail has expired unread
//20030308 DH8YMB bei "pu <board>" kann man nun auch "pu <board1> <board2>..."
//                anwenden


#include "baycom.h"

FILE *unr;

/*---------------------------------------------------------------------------*/

static int near purgeboard (char *name, unsigned *er, unsigned *obs,
                                                          unsigned *all)
//*************************************************************************
//
// Purge eines Boards (User oder Bulletin)
//
//*************************************************************************
{
  char fname[FNAMELEN+1];
  int npos;
  char timestr[8];
  time_t akt = ad_time();
  int lt;
  int mail_lt;
  int loeschen_noetig;
  handle cf = EOF;
  FILE *f, *of;
  char listlockname[20];
  char boardtest[DIRLEN+1];
  unsigned int i = 0;

  listlockname[0] = 0;
  if (get_nopurge(name)) return 2;
  i = strlen(name) - 1;
  while (i && name[i - 1] != '/') i--;
  if (strlen(name + i) < 10)
  {
    strcpy(listlockname, name + i);
    if (! sema_lock(listlockname)) return 0;
  }
  lastcmd(name);
  if (mbcallok(name))
    sprintf(fname, "%s/%s/", m.userpath, name);
  else
    sprintf(fname, "%s/%s/", m.infopath, name);
  npos = strlen(fname);
  strcat(fname, DIRLISTNAME);
  strlwr(fname);
  if ((f = s_fopen(fname, "srb")) != NULL)
  {
    loeschen_noetig = 0;
    while (fread(b->line, BLEN, 1, f))
    {
      if (b->line[23] != '~' || b->line[77] != '-')
      {
        trace(serious, "purgeboard", "corrupt %s", fname);
        s_fclose(f);
        f = NULL;
        xunlink(fname);
        loeschen_noetig = 0;
        reorg_noetig++;
        break;
      }
      strncpy(timestr, b->line, 7);
      timestr[7] = 0;
      mail_lt = atoi(b->line + 51);
      if (mail_lt == 999)
        lt = 999;
      else
        lt = mail_lt - (int) ((akt - filename2time(timestr)) / DAY);
      if ((b->line[61] != 'B'))
      { // Usermails
        if (lt <= 0
            || (b->line[14] != '>' && (b->line[14] != 'X' || m.remerase != 3)))
          loeschen_noetig++;
      }
      else
      { // Bulletins
        if (   (lt <= 0 && !(m.nopurge & 4))
            || (b->line[14] != '>' && (b->line[14] != 'X' || m.remerase != 3)))
          loeschen_noetig++;
      }
      if (   (b->line[60] == '|')    //new format
          && (b->line[61] != 'B')    //mailtype is personal/ack
          && (b->line[62] != '!')    //forward hold is set
          && !(m.nopurge & 8)        //nopurge = 8 is not set
          && ((akt - filename2time(timestr)) > MAXAGE) ) //older 1 month
        loeschen_noetig++;
      waitfor(e_ticsfull);
      (*all)++;
    }
    if (loeschen_noetig)
    {
      fseek(f, 0, SEEK_SET);
      strcpy(fname + npos, DIRTMPNAME);
      strlwr(fname);
      of = s_fopen(fname, "swb");
      if (! b->usermail) cf = s_open(CHECKNAME, "sr+b");
      if (of)
      {
        s_fsetopt(of, 1);
        while (fread(b->line, BLEN, 1, f))
        {
          int rem_now = 0;
          strncpy(timestr, b->line, 7);
          timestr[7] = 0;
          mail_lt = atoi(b->line + 51);
          if (mail_lt == 999)
            lt = 999;
          else
            lt = mail_lt - (int) ((akt - filename2time(timestr)) / DAY);
          if ((b->line[61] != 'B'))
          { // Usermails
            if (lt <= 0
                || (b->line[14] != '>' && (b->line[14] != 'X' || m.remerase != 3)))
              rem_now++;
          }
          else
          { // Bulletins
            if (   (lt <= 0 && !(m.nopurge & 4))
                || (b->line[14] != '>' && (b->line[14] != 'X' || m.remerase != 3)))
              rem_now++;
          }
          if (   (b->line[60] == '|')    //new format
              && (b->line[61] != 'B')    //mailtype is personal/ack
              && (b->line[62] != '!')    //forward hold is set
              && !(m.nopurge & 8)        //nopurge = 8 is not set
              && ((akt - filename2time(timestr)) > MAXAGE) ) //older 1 month
            rem_now++;
          if (rem_now)
          {
            if (lt > 0) (*er)++;
            else (*obs)++;
            // Benachrichtigung ungelesener Usermail NUR bei abgelaufener LT.
            // Aber KEINE Benachrichtigung bei ungelesen geloeschter
            // Usermail! (LT>0)
            if (unr && lt <= 0 && b->usermail && toupper(b->line[22]) != 'R')
            {
              char sendcmd[LINELEN+1];
              char content[LINELEN+1];
              time_t zeit = dirline_data(b->line);
              //Wichtig: Dies setzt die im folgenden benutzten b-> Werte !
              if (returnmailok())
              {
                snprintf(content, LINELEN, "%-6s > %-6s @%-6s %.22s",
                  b->herkunft, b->ziel, b->at, b->betreff);
                log_entry(OLDUMAILLOGNAME, content);
                sprintf(sendcmd, "\nSP %s < %s\nNot read: %.30s",
                  b->herkunft, m.boxname, b->betreff);
                sprintf(content,
                  "Your mail to %s @ %s received %s\nhas expired unread.",
                  b->ziel, b->at, datestr(zeit, 10));
                fprintf(unr, "%s\n", sendcmd);
                fprintf(unr, "%s\nnnnn\n", content);
              }
            }
            strcpy(fname + npos, timestr);
            strlwr(fname);
            xunlink(fname);
            if (cf != EOF)
            {
              seek_fname(cf, timestr, timestr, 0);
              if (*timestr)
              {
                if (_read(cf, b->line, BLEN) > 0)
                {
                  b->line[7] = ' ';
            //      strncpy(boardtest, b->line + 15, DIRLEN);
                    safe_strcpy(boardtest, b->line+15);
                    boardtest[DIRLEN] = 0;
                    cut_blank(boardtest);
                  if (strstr(name, boardtest) != NULL)
                  { //dh8ymb: nur loeschen wenn gleiches Board!
                     lseek(cf, -(LBLEN), SEEK_CUR);
                     _write(cf, b->line, BLEN);
                  }
                }
                else trace(serious, "purgeboard", "check read");
              }
            }
          }
          else fwrite(b->line, BLEN, 1, of);
          waitfor(e_ticsfull);
        }
        if (cf != EOF) s_close(cf);
        s_fclose(of);
        s_fclose(f);
        f = NULL;
        strcpy(b->line, fname);
        strcpy(b->line + npos, DIRTMPNAME);
        strcpy(fname + npos, DIRLISTNAME);
        strlwr(fname);
        if (xunlink(fname))
          trace(replog, "purgeboard", "unlink %s", fname);
        if (xrename(b->line, fname))
          trace(serious, "purgeboard", "rename %s", b->line);
      }
      else
        trace(serious, "purgeboard", "fopen %s errno=%d %s", fname,
                                            errno, strerror(errno));
    }
    if (f) s_fclose(f);
  }
  waitfor(e_ticsfull);
  fname[npos-1] = 0; // remove slash
  sema_unlock(listlockname);
  return rmemptydir(fname);
}

/*---------------------------------------------------------------------------*/

void mbpurge (char *)
//*************************************************************************
//
// Purge aller Boards ("purge all")
//
//*************************************************************************
{
  char *userlist;
  char name[20];
  unsigned int i = 0;
  unsigned er = 0, obs = 0, all = 0;

  strcpy(name, "mbpurge");
  if (m.nopurge & 1) return;
  char tmpname[FNAMELEN+1];
  if (m.oldumail)
  {
    sprintf(tmpname, TEMPPATH "/%s", time2filename(0));
    strcat(tmpname, ".imp");
    strlwr(tmpname);
    if ((unr = s_fopen(tmpname, "lwt")) == NULL)
      trace(serious, name, "fopen %s errno=%d %s", tmpname,
                                      errno, strerror(errno));
  }
  else
    unr = NULL;
  strcpy(t->name, "Purge");
  while (! sema_lock("purgereorg")) wdelay(1026);
  b->job = j_purge;
  b->usermail = 0;
  if (! (m.nopurge & 4))
  {
    i = 0;
    trace(replog, name, "bulletins");
    while (i < treelen)
    {
      if (! purgeboard(tree[i].name, &er, &obs, &all)) i++;
      if (obs > 10000) trace(fatal, name, "%u lt aborting");
    }
    trace(replog, name, "%u era, %u lt, %u all", er, obs, all);
  }
  if (! (m.nopurge & 8))
  {
    trace(replog, name, "users");
    b->usermail = 1;
    er = obs = all = 0;
    userlist = (char *) t_malloc(8U * MAXUSERMAILS, "u_pu");
    if (userlist)
    {
      i = 0;
#ifdef __BORLANDC__
      struct ffblk di;
      char mask_wild[FNAMELEN+1];
      sprintf(mask_wild, "%s/*.*", m.userpath);
      if (! findfirst(mask_wild, &di, 0xffff))
      {
        do
        {
          if (*di.ff_name != '.')
          {
            strncpy(userlist + (i * 8), di.ff_name, 6);
            userlist[i * 8 + 6] = 0;
            i++;
            if (i >= MAXUSERMAILS)
              trace(fatal, name, "too many usermails");
          }
          waitfor(e_ticsfull);
        }
        while (! findnext(&di));
      }
#else
      DIR *d;
      struct dirent *di;
      d = opendir(m.userpath);
      while (d && (di = readdir(d)) != NULL)
      {
        if (*di->d_name != '.')
        {
          strncpy(userlist + (i * 8), di->d_name, 6);
          userlist[i * 8 + 6] = 0;
          i++;
          if (i >= MAXUSERMAILS)
            trace(fatal, name, "too many usermails");
          waitfor(e_ticsfull);
        }
      }
      closedir(d);
#endif
      userlist = (char *) t_realloc(userlist, 8U * (i + 1));
      while (i)
      {
        i--;
        purgeboard(userlist + (i * 8), &er, &obs, &all);
      }
      t_free(userlist);
      trace(replog, name, "%u era, %u lt, %u all", er, obs, all);
    }
  }
  if (! (m.nopurge & 4)) purgecheck("");
  trace(replog, name, "ready, %s sec cputime",
                       zeitspanne(t->cputics, zs_cpuexact));
  sema_unlock("purgereorg");
  if (unr)
  {
    fprintf(unr, "\nimpdel\n");
    s_fclose(unr);
    fork(P_BACK | P_MAIL, 0, mbimport, tmpname);
  }
  if (reorg_noetig) fork(P_BACK | P_MAIL, 0, mbreorg, "");
}

/*---------------------------------------------------------------------------*/

void userpurge (char *befbuf)
//*************************************************************************
//
// Verarbeitung: Benutzer hat Purge angestossen
//
//*************************************************************************
{
  char board[80];
  unsigned er = 0, obs = 0, all = 0;
  unsigned int i, anz;
  char worte[MAXPARSE];
  char s1[80];
  char *s2;

  if (m.nopurge & 1)
  {
    putf(ms(m_purgedisabled));
    return;
  }
  if (sema_test("purgereorg") || sema_test("sendlock"))
  {
    putf(ms(m_purgerunning));
    return;
  }
  if (! stricmp(befbuf, "A"))
  {
    putf(ms(m_purgeallstarted));
    fork(P_BACK | P_MAIL, 0, mbpurge, befbuf);
    return;
  }
  if (*befbuf)
    strncpy(board, befbuf, 79);
  else
    strcpy(board, b->logincall);
  board[79] = 0;
  strupr(board);
  strcpy(s1,board);
  anz = parseline(s1, worte);
  for (i = 0; i < anz; i++)
  {
    s2 = (s1 + worte[i]);
    if (finddir(s2, b->sysop))
    {
      if (! sema_lock("purgereorg")) return;
      putf(ms(m_boardpurge), b->boardfullname);
      if (purgeboard(b->boardfullname, &er, &obs, &all) == 2)
      {
        putf(ms(m_purgedisabled));
        sema_unlock("purgereorg");
      }
      else
      {
        sema_unlock("purgereorg");
        if (! mbcallok(b->boardname) && ((i+1) == anz))
        {
          putf(ms(m_purgeresult), all, er, obs);
          fork(P_BACK | P_MAIL, 0, purgecheck, "P_Check");
        }
      }
    }
    else
      putf(ms(m_noboxfound), board);
  }
}
