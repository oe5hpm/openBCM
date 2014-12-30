/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------
  Ausgaben und Zubehoer zum DIR/LIST-Befehl
  -----------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980118 OE3DZW removed sec from dir/list - timestr
//19980218 OE3DZW renamed flag5 to msgty
//19980308 OE3DZW dir-option "-r" -> only held msg
//19980312 OE3DZW above option replaced by "-p", above did not work correctly
//19980407 OE3DZW bytes korrekt
//19980423 OE3DZW new "holdusers" shows all usermails held
//19980430 OE3DZW fixed holdusers, only showed results on first call
//19980526 OE3DZW added debug-logoutput, to find "d b"  bug
//19980824 CZ4WAY added headerfile to mailbeacon
//                CZ4WAY (CB)
//                = Jan Wasserbauer <jan.wasserbauer@spsselib.hiedu.cz>
//19980831 OE3DZW added usermail=0 to dir news etc. (else wrong options
//                could eventually be read
//19980906 DG0CDC added macro support to headerfile of mailbeacon
//19980903 OE3DZW fixed s_fclose not open bug (related to beacon update)
//19980914 OE3DZW added header for dir/list
//19980915 OE3DZW fixed header - was missing for usermails
//                fixed search algorithm - will find correct boards
//19980924 OE3DZW d n shows correct boardinfo
//19980924 OE3DZW added oldestbeacon
//19980924 OE3DZW shows only local mail in mailbeacon
//19980925 OE3DZW fixed mailbeacon, will only show mail to local users
//                not logged in since mail received
//19981001 OE3DZW becon could become longer 256 Bytes
//19990824 DH3MB  Changed format of "DIR BOARDS *"
//19991212 DF3VI  added DIR DEST in mbdir()
//19991221 DF3VI  removed bereich for o_g in do_dir()
//20000101 dk2ui  dir news is only updated at logout
//20000116 DK2UI  (dzw) added ui-options
//20000622 DK2UI  list for HTTP changed to boardname
//20000713 DK2UI  added regex_match to dir and list
//20001005 DK2UI  dir headline option o_w and LF corrected
//20010120 DK2UI  option dir reject from DF3VI included
//20010620 DF3VI  putdirkopf() options corrected
//20010719 DF3VI  new options for dir
//20040529 DH8YMB renamed msgty to conttype
//20111005 OE5HPM added support to put mailbeacon into file

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#define BEACON_LEN 3000

/*---------------------------------------------------------------------------*/

int finddir (char *path, int one_letter_boards)
//***********************************************************************
//
//  Sucht einen Bulletin- oder Benutzernamen
//
//  Rückgabewerte:
//     0 Board nicht gefunden
//    -1 Usermail, keine Nachricht vorhanden
//    -2 Usermail, Nachricht vorhanden
//
//***********************************************************************
{
  int ret = (-2);
  unsigned j;
  char cmp[DIRLEN+1];
  unsigned int i, found;

  *b->boardname = 0;
  *b->boardpath = 0;
  *b->boardfullname = 0;
  strupr(path);
  j = strlen(path) - 1;
  while (j && (path[j - 1] != '/')) j--;
  strncpy(cmp, path + j, DIRLEN);
  cmp[DIRLEN] = 0;
  cut_blank(cmp);
  if ((strlen(cmp) == 1) && ! one_letter_boards && ! b->forwarding) return 0;
  if (mbcallok(path))
  {
    sprintf(b->boardpath, "%s/%s", m.userpath, path);
    strlwr(b->boardpath);
    if (! filetime(b->boardpath)) ret = (-1);
    strcpy(b->boardname, path);
    strcpy(b->boardfullname, path);
    b->boardlife_max = m.userlife;
    b->boardlife_min = 0;
    b->usermail = 1;
  }
  else
  {
    found = 0;
    b->usermail = 0;
    strcpy(b->boardpath, m.infopath);
    strcat(b->boardpath, "/");
  // first try to find a board with 8 characters matching, and
  // if not found try with 6 characters because a mailsystem which
  // only supports 6 characters may have cut the original board name
    for (i = 0; i < treelen; i++)
    {
      if (! strncmp(cmp, tree[i].name + tree[i].flatoffset, 8))
      {
        strcat(b->boardpath, tree[i].name);
        strcpy(b->boardfullname, tree[i].name);
        strcpy(b->boardname, tree[i].name + tree[i].flatoffset);
        b->boardlife_max = tree[i].lifetime_max;
        b->boardlife_min = tree[i].lifetime_min;
        found = 1;
        ret = i + 1;
        break;
      }
    }
    if (! found) return 0;
  }
  return ret;
}

/*---------------------------------------------------------------------------*/

char *make_mask (char *mask, char *path)
//*************************************************************************
//
//
//*************************************************************************
{
  if (mbcallok(path))
    strcpy(mask, m.userpath);
  else
    strcpy(mask, m.infopath);
  strcat(mask, "/");
  strcat(mask, path);
  strcat(mask, "/*.*");
  return mask;
}

/*---------------------------------------------------------------------------*/

handle preparedir (char *selektor, int printerror)
//*************************************************************************
//
//
//*************************************************************************
{
  handle fh = EOF;
  static char ts[25];

  waitfor(e_ticsfull);
  if ((b->index = finddir(selektor, b->sysop)) != 0)
  {
    make_mask(b->mask, b->boardfullname);
    strcpy(b->listpath, b->mask);
    if (! strstr(b->listpath, "*.*"))
      trace(fatal, "preparedir", "mask %s", b->listpath);
    strcpy(strstr(b->listpath, "*.*"), DIRLISTNAME);
    strlwr(b->listpath);
    if ((fh = s_open(b->listpath, "lrb")) != EOF)
    {
      if (filelength(fh) < LBLEN)
      {
        s_close(fh);
        trace(serious, "prepdir", "len0 %s", b->listpath);
        // not really serious - why not?? (DH3MB)
        xunlink(b->listpath);
        fh = EOF;
      }
      else
      {
        _read(fh, ts, 24);
        lseek(fh, 0L, SEEK_SET);
        if ((ts[7] != '<') || (ts[23] != '~'))
        {
          s_close(fh);
          trace(serious, "prepdir", "defect %s", b->listpath);
          xunlink(b->listpath);
          fh = EOF;
        }
      }
    }
    if ((fh == EOF) && ! b->semalock)
    {
      createdirlist(0);
      strlwr(b->listpath);
      fh = s_open(b->listpath, "lrb");
    }
  }
  else
    if (printerror && ! b->index)
      putf(ms(m_noboxfound), selektor);
  waitfor(e_ticsfull);
  return fh;
}

/*---------------------------------------------------------------------------*/

time_t dirline_data (char *dirline)
//*************************************************************************
//
//
//*************************************************************************
{
  time_t dirtime;
  int life;

  cut_blank(dirline);
  dirline[7] = 0;
  dirtime = filename2time(dirline);
  if (dirline[14] != '>') b->geloescht = dirline[14];
  else b->geloescht = 0;
  dirline[14] = 0;
  skip(dirline + 8);
  strcpy(b->herkunft, dirline + 8);
  dirline[23] = 0;
  skip(dirline + 15);
  strcpy(b->ziel, dirline + 15);
  b->usermail = mbcallok(b->ziel);
  if (b->usermail)
    b->gelesen = (dirline[22] == 'R');
  else
    b->gelesen = 0;
  dirline[30] = 0;
  skip(dirline + 24);
  strcpy(b->frombox, dirline + 24);
  dirline[37] = 0;
  skip(dirline + 31);
  strcpy(b->at, dirline + 31);
  dirline[50] = 0;
  skip(dirline + 38);
  strcpy(b->bid, dirline + 38);
  life = atoi(dirline + 51);
  if (life == 999) b->lifetime = 999;
  else b->lifetime = life - (int) ((ad_time() - dirtime) / DAY);
  if (b->lifetime < (-99)) b->lifetime = (-99);
  b->lines = (unsigned) ascdez(dirline + 55, 2);
  b->bytes = inonlin(ascdez(dirline + 57, 3));
//OE3DZW Anfangsposition von Betreff veraendert
  if (dirline[60] == '|')
  {
    b->mailtype = dirline[61];
    b->fwdhold = dirline[62];
    b->fwdprop = dirline[63];
    b->replied = dirline[64];
    b->conttype = dirline[65];
    b->eraseinfo = dirline[66];
    b->flag7 = dirline[67];
    b->flag8 = dirline[68];
    b->flag9 = dirline[69];
    b->flaga = dirline[70];
    b->flagb = dirline[71];
    b->flagc = dirline[72];
    b->flagd = dirline[73];
    b->flage = dirline[74];
    b->flagf = dirline[75];
    b->flag0 = dirline[76];
    strcpy(b->betreff, dirline + 78);
  }
//OE3DZW: alte Position war 61, falls noch altes dir/check vorhanden ist
  else
  {
    strcpy(b->betreff, dirline + 61);
    b->fwdhold = b->fwdprop = b->replied =
      b->conttype = b->eraseinfo = b->flag7 = b->flag8 = b->flag9 =
      b->flaga = b->flagb = b->flagc = b->flagd = b->flage =
      b->flagf = b->flag0 = '!';
    if (b->usermail)
      b->mailtype = 'P';
    else
      b->mailtype = 'B';
  }
  return dirtime;
}

/*---------------------------------------------------------------------------*/

static void near dir_line (unsigned long lfd, char *dirline)
//*************************************************************************
//
// Gibt eine DIR Zeile aus
//
//*************************************************************************
{
  char line[140];
  time_t timed;
  char flag;

  if (b->usermail)
    flag = ')';
  else
  {
    if (b->opt & o_r) flag = ')';
    else flag = ' ';
  }
  if (strlen(dirline) == 7) timed = filename2time(dirline);
  else timed = dirline_data(dirline);
  if (b->gelesen) flag = 'R';
  if (b->replied == '1') flag = 'r';
  if (b->fwdhold != '!') flag = 'H'; //display hold flag (db1ras)
  if (b->geloescht) flag = b->geloescht;
  if (b->opt & o_k)
    sprintf(line, "%c ", flag);
  else
    strcpy(line, " ");
  if (b->opt & o_a) sprintf(line + strlen(line), "%-6s ", b->herkunft);
  if (b->opt & o_e) sprintf(line + strlen(line), "> %-8s ", b->ziel);
  if (b->opt & o_d)
    sprintf(line + strlen(line), "%s ",
             datestr(timed, !! (b->opt & o_y) + (8 * u->dirformat)));
  if (b->opt & o_t)
    sprintf(line + strlen(line), "%-5.5s ", datestr(timed, 3));
  if (b->opt & o_z) sprintf(line + strlen(line), "%4u ", b->lines);
  if (b->opt & o_b)
  {
    if (b->bytes < 1000000L || ! stricmp(b->uplink, "POP3"))
      //dh8ymb: bei POP3 muss Lockfile Bytesanzahl haben!
      sprintf(line + strlen(line), "%6lu ", b->bytes);
    else
      sprintf(line + strlen(line), "%5luk ", b->bytes >> 10);
  }
  if (b->opt & o_i) sprintf(line + strlen(line), "$%-12s ", b->bid);
  if (b->opt & o_l) sprintf(line + strlen(line), "#%-3d ", b->lifetime);
  if (b->opt & o_f)
  {
    sprintf(line + strlen(line), "%-6s ", b->frombox);
    if (! (b->opt & o_m)) strcat(line, "@ ");
  }
  if (b->opt & o_m) sprintf(line + strlen(line), "@%-6s ", b->at);
  if (b->opt & o_x) strcat(line, " ");
  line[73] = 0;
  if (b->opt & o_p)
    sprintf(line + strlen(line), "%c%c%c%c%c%c ", b->mailtype, b->fwdhold,
             b->fwdprop, b->replied, b->conttype, b->eraseinfo);
  b->betreff[74 - strlen(line)] = 0; //df3vi: betreff erst nach flags kuerzen
  cut_blank(b->betreff);
  if (b->opt & o_w) strcat(line, b->betreff);
#ifdef __FLAT__
  if (b->http)
  {
    int i = 1;
    putf("%4ld", lfd);
    putv(*line);
    if (*line != ' ') putv(line[i++]);
    if (b->opt & o_k && flag == ' ') putv(line[i++]);
    if (! (b->opt & o_u))
      html_putf("<a href=\"/cmd?cmd=READ+%s+%lu\">", b->boardname, lfd);
    while (line[i] != ' ') putv(line[i++]);
    html_putf("</a>");
    putf("%s\n", line + i);
  }
  else
#endif
    putf("%4ld%s\n", lfd, line);
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

static char near dir_users (char *bake)
//*************************************************************************
//
//
//*************************************************************************
{
  int n = 0, i = 0, cnt = 0;
  char *userlist;
  handle fh;
  char buf[32];
  time_t timed = (ad_time() - (long) m.oldestbeacon * DAY);
  time_t usrlastlogin;
  int prozent = 0;
  char buffer[50];
  char *msg;
  char *btmp = NULL;

  if (! bake)
    putf(ms(m_dirofusers));
  else
  {
    FILE *f;
    if ((f = s_fopen(BEACONHEADNAME, "srt")) != NULL)
    {
      i = fgetc(f);
      while (i != EOF && n < 150)
      { // DG0CDC: added support for macros
        if (prozent)
        {
          msg = chartomakro(i, buffer);
          if (n + strlen(msg) < BEACON_LEN - 100)
            while (*msg)
            {
              bake[n] = *msg;
              msg++;
              n++;
            }
            prozent = 0;
        }
        else if (i == '%') prozent = 1;
        else
        {
          bake[n] = i;
          n++;
        }
        i = fgetc(f);
      }
      if (bake[n - 1] != LF)
      {
        bake[n] = LF;
        n++;
      }
      s_fclose(f);
    }
    bake[n] = 0;
    btmp = bake + n;
  }
  i = 0;
  n = 0;
  sprintf(b->mask, "%s/*.*", m.userpath);
  userlist = dirsort(m.userpath, 1);
  if (userlist)
  {
    for (i = 0; userlist[i * 8]; i++)
    {
      sprintf(b->listpath, "%s/%s/" DIRLISTNAME, m.userpath, userlist+i*8);
      strlwr(b->listpath);
      if ((fh = s_open(b->listpath, "srb")) != EOF)
      {
        unsigned len = (unsigned) (filelength(fh) >> 7);
        while (len)
        {
          len--;
          waitfor(e_ticsfull);
          lseek(fh, LBLEN * (long)len, SEEK_SET);
          _read(fh, buf, 32);
          if (   (buf[14] == '>' || (b->optplus & o_v))
              && (! bake || toupper(buf[22]) != 'R'))
          {
            if (bake)
            {
              buf[7] = 0;
              time_t mailtime = filename2time(buf);
              if (   mailtime > timed
                  && get_usr_local(userlist + i * 8, usrlastlogin)
                  && mailtime > usrlastlogin)
              {
              // end of frame ?
                if (strlen(bake) > 220 && ! n)
                { // insert zero delimiter
                  strupr(btmp);
                  bake += (strlen(bake) + 1);
                  *bake = 0;
                  btmp = bake;
                  n = 0;
                }
                // + 7 Bytes
                strcat(bake, userlist + i * 8);
                cnt++;
                //make sure that beacon is not longer than 256 bytes
                if (++n > 10 || strlen(bake) > 243)
                { // + 2 Bytes
                  strcat(bake, "\n");
                  n = 0;
                }
                else
                  strcat(bake, " ");
              }
            }
            else
            {
              putf("%-7s", userlist + i * 8);
              if (++n > 10)
              {
                putv(LF);
                n = 0;
              }
            }
            break;
          }
        }
        s_close(fh);
      }
    }
    t_free(userlist);
  }
  if (n)
  {
    if (bake) strcat(bake, "\n");
    else putv(LF);
  }
  if (bake)
  {
    if (cnt == 0 && m.mailbeacon == 1) strcat(bake, "NONE\n");
    // append double zero to terminate beacon string
    i = strlen(bake);
    bake[i] = 0;
    bake[i + 1] = 0;
    strupr(btmp);
  }
  return cnt;
}

/*---------------------------------------------------------------------------*/

void sendmailbake (char *)
//*************************************************************************
//
//
//*************************************************************************
{
  char s[100];
  FILE *f, *f1;
  unsigned int i = 0;
  char *buffer = (char *) t_malloc(BEACON_LEN, "beac");
  char *pb = buffer;
  int frames = 0;
  int users = 0;

  if (! sema_lock("beacon")) return;
  if (! m.disable && ! sema_test("purgereorg"))
  {
    users = dir_users(buffer);
    if (m.mailbeacon || users)
    {
      f = s_fopen(BEACONNAME, "srt");
      if (f)
      {
        while (fgets(s, 70, f))
        {
          if (*s)
          {
            s[strlen(s) - 1] = 0; // gelesenen String 0-terminieren
            if (strlen(s) >= 4 && s[0] == 'F' && s[1] == 'I' && s[2] == 'L' && s[3] == 'E')
            {
              f1 = s_fopen("beacon.txt", "swt");
              fputs(buffer, f1);
              s_fclose(f1);
              frames++;
            }
            else
            {
              pb = buffer;
              while (*pb)
              {
                for (unsigned int j = 0; pb[j]; j++)
                  if (pb[j] == LF) pb[j] = CR;
                putbeacon_tnc(s, pb);
                pb += (strlen(pb) + 1);
                frames++;
              }
            }
            i++;
          }
        }
        s_fclose(f);
      }
    }
    trace(report, "mailbeacon", "len=%lu num=%d frames=%d users=%d",
          ((unsigned long) pb - (unsigned long) buffer), i, frames, users);
  }
  t_free(buffer);
  sema_unlock("beacon");
}

/*---------------------------------------------------------------------------*/

static time_t near seek_time (handle fh, time_t zeit)
//*************************************************************************
//
//
//*************************************************************************
{
  char schwelle[8];
  strcpy(schwelle, time2filename(zeit));
  return seek_fname(fh, schwelle, schwelle, 0);
}

/*---------------------------------------------------------------------------*/

static void near dir_boards (char *selektor)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int i, tab = 0;
  handle fh;
  //char name[20];
  //strcpy(name, "dir_boards");
  strupr(selektor);
  if (! *selektor)
  {
    putf(ms(m_dirofbulletins));
    //trace(report, name, "size %d", treelen);
    for (i = 0; i < treelen; i++)
    {
      if ( b->sysop || strstr(tree[i].name, "TMP") != tree[i].name)
      {
        char *slash = strchr(tree[i].name, '/');
        if (slash)
        { //Einbuchstabige Subboards nur fuer Sysop
          if (! b->sysop && strlen(slash) < 3) continue;
          if (tab < 9)
          {
            putf(" /");
            tab = 11;
          }
          else
            if((tab + strlen(slash)) > 79)
            {
              putf("\n%9c/", ' ');
              tab = 11;
            }
          putf(" ");
#ifdef __FLAT__
          if (b->http)
            html_putf("<a href=\"/cmd?cmd=d+%s+-50\">", slash + 1);
#endif
          putf("%s", slash + 1);
#ifdef __FLAT__
          if (b->http) html_putf("</a>");
#endif
          tab += strlen(slash);
        }
        else
        {
          if ((strlen(tree[i].name) != 1) || b->sysop)
          {
            if(tab)
            putv(LF);
#ifdef __FLAT__
            if (b->http)
              html_putf("<a href=\"/cmd?cmd=d+%s+-50\">", tree[i].name);
#endif
            putf("%-8s", tree[i].name);
#ifdef __FLAT__
            if (b->http) html_putf("</a>");
#endif
            tab = 8;
          }
        }
        if (testabbruch()) break;
      }
    }
    if (tab) putv(LF);
  }
  else
  {
    putf(ms(m_boardlifemails));
    //trace(report, name, "size %d,%s", treelen, selektor);
    for (i = 0; i < treelen; i++)
    {
      if (*selektor == '*' || strstr(tree[i].name, selektor))
      {
        char *slash = strchr(tree[i].name, '/');
        //Anzeige nur Main- oder auch Subboards
        if (slash && ! (b->opt & o_s)) continue;
        //Anzeige einbuchstabiger Boards nur fuer Sysops
        if (! b->sysop && (strlen(tree[i].name) < 2)) continue;
        //Anzeige einbuchstabiger Sub-Boards nur fuer Sysops
        if (! b->sysop && (slash && (strlen(slash) < 3))) continue;
        {
          if ((! m.altboardinfo) && (b->opt & o_j))
            board_info(tree[i].name);
          putf("%-18s %3d-%3d", tree[i].name, tree[i].lifetime_min,
                                tree[i].lifetime_max);
          fh = preparedir(tree[i].name, 0);
          if (fh != EOF)
          {
            time_t tim = seek_time(fh, ad_time());
            if (tim > 0)
            {
              putf("%6ld %4s", ltell(fh) >> 7,
              zeitspanne(ad_time() - tim, zs_seconds));
            }
            else
            {
              if (m.altboardinfo)
                putf("           ");
            }
            s_close(fh);
            waitfor(e_reschedule);
          }
          else
          {
            if (m.altboardinfo)
              putf("           ");
          }
          if ((m.altboardinfo) && (b->opt & o_j))
            board_info(tree[i].name);
          putv(LF);
        }
      }
      if (testabbruch()) break;
    }
  }
}

/*---------------------------------------------------------------------------*/

static int near line_reject (char *line)
//*************************************************************************
//
// df3vi: reject nach call, verteiler
//
//*************************************************************************
{
  char board[12];
  int nott = (u->notvisible[1] != '-');
  unsigned int i = 0, j;
  if (   u->notvisible[1]
      && (strchr(u->notvisible, '<') || strchr(u->notvisible, '@')))
  {
    for (j = 1; j < 3; j++) //df3vi: bei DIR ist board schon geprueft.
    {
      *board = ' ';
      switch (j)
      {
//      case 0: i = 8;
//        strncpy(board + 1, line + 15, i);
//        break;
        case 1: i = 7; //Auf Absender pruefen " <CALL "
          strncpy(board + 1, line + 7, i);
          break;
        case 2: i = 7; //Auf Verteiler pruefen " @ADR "
          strncpy(board + 1, line + 30, i);
          break;
      }
      board[i + 1] = 0;
      cut_blank(board);
      strcat(board, " ");
      if (strstr(u->notvisible, board)) return nott;
    }
    return ! nott;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

static int near do_dir (char *search, char *dirline,
                        int *oldmail, int *notfound, int readlock)
//*************************************************************************
//
//
//*************************************************************************
{
  CRegEx reg;
  char cmpline[BLEN];
  char fromcall[CALLEN+1];

  if (search && *search)
  {
    strcpy(cmpline, dirline);
    cut_blank(cmpline);
    strcat(cmpline, "\n");
    if (! reg.regex_match(search, cmpline))
    {
      (*notfound)++;
      return 0; // Suchkriterium nicht erfuellt
    }
  }
  if (readlock)
  {
    strncpy(fromcall, dirline + DIRLEN, CALLEN);
    fromcall[CALLEN] = 0;
    cut_blank(fromcall);
    if (strcmp(fromcall, b->logincall) && ! b->sysop)
    {
      if (readlock == 2 || toupper(dirline[22]) != 'R') return 0;
    }
  }
  // -g ... only show unread mail // dirline[22] ... R marks mail as read
  if ((b->opt & o_g) && toupper(dirline[22]) == 'R' && b->usermail)
  {
    (*oldmail)++;
    return 0; // G aktiv und Nachricht schon gelesen
  }
  if (! (b->opt & o_v) && (dirline[14] != '>'))
    return 0; // V nicht aktiv aber Nachricht geloescht
  if ((b->opt & o_p) && (dirline[62] == '!') && (dirline[77] == '-'))
    return 0; // Only held messages, check for new format
  if (! *search && ! mbcallok(b->boardname)) return ! line_reject(dirline);
  return OK;
}

/*---------------------------------------------------------------------------*/

void board_info (char *board)
//*************************************************************************
//
// outputs some info on a certain bulletin board
//
//*************************************************************************
{
  char line[LINELEN+1];
  char *p;
  char *tmpb = board;
  FILE *f = s_fopen(BOARDINFO, "srt");

  if (! f) return;
  if (strchr(tmpb, '/')) tmpb = strchr(tmpb, '/') + 1;
  while (! feof(f))
  {
    if (fgets(line, LINELEN, f))
    {
      p = line;
      while (*p == ' ') p++;
      if (   stristr(p, tmpb) == p          //Board is found
          && (strlen(p) > strlen(tmpb) + 2) //String is long enough
          && p[strlen(tmpb)] == ' ')  //It is not a fragment (eg T TAUSCH)
      {
        p = strchr(p, ' ') + 1;
        while (*p == ' ') p++;
        strtok(p, "\n\r");
        if (m.altboardinfo)
          putf(" - %s", p);
        else
          putf("%s\n", p);
      }
    }
  }
  s_fclose(f);
}

/*---------------------------------------------------------------------------*/

static int near putdirkopf (void)
//*************************************************************************
//
//
//*************************************************************************
{
  char mybbs[HADRESSLEN + 1];
  int ok = 0;
  int is_user = mbcallok(b->boardname);

  strcpy(mybbs, "");
  if (is_user)
  {
    ok = get_mybbs(b->boardname, mybbs, 0);
    if ((b->opt & o_n) && ! strcmp(atcall(mybbs), m.boxname)) return 0;
  }
  if (! (b->opt & o_h))
  {
    putf(ms(m_directoryof), b->boardfullname);
    if (mybbs[0]) putf(" %c %s", ok ? '@' : '?', mybbs);
    if (! m.altboardinfo) putf(":\n");
    if (! is_user) board_info(b->boardfullname);
    if (m.altboardinfo) putf(":\n");
    if (b->opt & o_j)
    {
      putf("  ");
      putf(ms(m_list_nr));
      if (b->opt & o_k)
        putf(" ");
      if (b->opt & o_a)
        putf(ms(m_list_call));
      if (b->opt & o_e)
      {
        putf("  ");
        putf(ms(m_list_board));
      }
      if (b->opt & o_d)
      {
        putf(ms(m_list_date));
        if (b->opt & o_y) putf("   ");
      }
      if (b->opt & o_t)
        putf(ms(m_list_time));
      if (b->opt & o_z)
        putf(ms(m_list_lines));
      if (b->opt & o_b)
      {
        putf(" ");
        putf(ms(m_list_bytes));
      }
      if (b->opt & o_i)
        putf(ms(m_list_bid));
      if (b->opt & o_l)
      {
        putf(ms(m_list_life));
        putf(" ");
      }
      if ((b->opt & o_f) || (b->opt & o_m))
      {
        putf(ms(m_list_forw));
        putf("  ");
      }
      if (b->opt & o_x)
        putf(" ");
      if (b->opt & o_w)
        putf(ms(m_list_subject));
      else
        putv(LF);
      leerzeile();
    }
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static void near dir_einboard (char *selektor,
                               int printerror, char *search, char *lastkopf)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned kopf = 0;
  unsigned long pos = 0;
  char *line= b->line;
  int oldmail = 0, notfound = 0;
  handle fh;
  int nixneues = 0;
  int readlock = 0;
  char callmybbs[HADRESSLEN + CALLEN + 4];
  char mybbs[HADRESSLEN + 1];

  strcpy(mybbs, "");
  strcpy(b->ziel, "x");
  strcpy(b->zielboard, b->ziel);
  time_t ftime;
  fh = preparedir(selektor, printerror);
  formoptions();
  if (fh != EOF)
  {
    if (b->abwann)
    {
      ftime = seek_time(fh, b->abwann);
      if (b->index > 0) tree[b->index - 1].newestmail = ftime;
      if (printerror && leof(fh)) nixneues = 1;
    }
    else if (b->beginn > 0)
         {
           if (b->beginn > (unsigned long) (filelength(fh) >> 7))
             nixneues = 1;
           else
             lseek(fh, (long) (b->beginn - 1) << 7, SEEK_SET);
         }
         else
         {
           if (! seek_lastentry(fh, b->ende))
             nixneues = 1;
           b->ende = MAXBEREICH;
         }
    if (nixneues)
    {
      if (seek_lastentry(fh, 1)) putf(ms(m_lastmessage));
      else pos = b->ende;
    }
    if (mbcallok(b->boardname) && strcmp(b->boardname, b->logincall))
    {
      readlock = get_readlock(b->boardname);
      if (readlock != 1 && u->rlimit) readlock = u->rlimit;
    }
    while (_read(fh, line, BLEN) && pos < b->ende && ! testabbruch())
    {
      waitfor(e_ticsfull);
      line[BLEN-2] = 0;
      pos = (unsigned) (ltell(fh) >> 7);
      if (do_dir(search, line, &oldmail, &notfound, readlock))
      {
        if (! kopf)
        {
          if (! printerror && ! mbcallok(b->boardname))
          {
            char *slash = strchr(b->boardfullname, '/');
            if (slash) *slash = 0;
            if (! strcmp(b->boardfullname, lastkopf))
              kopf = OK;
            else
              strcpy(lastkopf, b->boardfullname);
          }
          if (! kopf) kopf = putdirkopf();
        }
        if (kopf) dir_line(pos, line);
      }
    }
    s_close(fh);
  }
  else if (b->index > 0) tree[b->index - 1].newestmail = BOARDLEER;
  if (! kopf && printerror && *b->boardname)
  { // oe3dzw: Ausgabe der Mybbs, falls keine (ungelesenen)
    //         Nachrichten gefunden wurden.
    if (! mbcallok (b->boardname))
      strcpy(callmybbs, b->boardname);
    else
      if (get_mybbs(b->boardname, mybbs, 0))
        sprintf(callmybbs, "%s @ %s", b->boardname, mybbs);
      else
        if (*mybbs)
          sprintf(callmybbs, "%s ? %s", b->boardname, mybbs);
        else
          sprintf(callmybbs, "%s @ ???", b->boardname);
    if (oldmail)
      putf(ms(m_nounread), callmybbs);
    else
      if (notfound)
        putf(ms(m_nomessagefound));
      else
        putf(ms(m_forcallnomessage), callmybbs);
  }
}

/*---------------------------------------------------------------------------*/

static int near boardreject (char *boardpath)
//*************************************************************************
//
//
//*************************************************************************
{
  char board[30];
  char *slash;
  int nott = (u->notvisible[1] != '-');

  *board = ' ';
  strcpy(board + 1, boardpath);
  strcat(board, " ");
  slash = strchr(board, '/');
  if (slash)
  {
    *slash = ' ';
    if (strstr(u->notvisible, slash)) return nott;
    slash[1] = 0;
  }
  if (strstr(u->notvisible, board)) return nott;
  else return ! nott;
}

/*---------------------------------------------------------------------------*/

static void near dmusers (char *search)
//*************************************************************************
//
//
//*************************************************************************
{
  char *userlist;
  char lastkopf[FNAMELEN+1];
  unsigned int i = 0;

  strcpy(lastkopf, "");
  b->usermail = 1;
  formoptions();
  putf(ms(m_dirofusermails));
  if ((b->opt & o_q) && (! search ||  ! *search)) b->abwann = u->lastquit;
  else b->abwann = 0;
  userlist = (char *) t_malloc(8U * MAXUSERMAILS, "user");
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
          trace(fatal, "dmusers", "too many usermails");
      }
      waitfor(e_ticsfull);
    }
    closedir(d);
    while (i)
    {
      i--;
      b->beginn = 1;
      b->ende = MAXBEREICH;
      dir_einboard(userlist + (i * 8), 0, search, lastkopf);
      if (testabbruch()) break;
    }
    t_free(userlist);
  }
}

/*---------------------------------------------------------------------------*/

static int dir_outstanding (char *call, char typ)
//*************************************************************************
//
//  Gibt alle (bzw. eine Auswahl) Mails aus, die zu einem bestimmten
//  FWD-Partner zum Forward anstehen
//  df3vi: selektion nur user/info
//
//*************************************************************************
{
  char fwdfname[20];
  char s[BUFLEN];
  FILE *f, *mailf;
  unsigned int count = 0, imail;
  char *path[2] = {"u_", "i_"};
  int nokopf = 1;
  int retwert = 1;
  unsigned int min = 0, max = 2;

  if (typ == 'U') max = 1; //df3vi: nur user
  if (typ == 'I') min = 1; //df3vi: nur info
  if (! strcmp(call, "DUMMY")) return NOTFOUND;
  for (imail = min; imail < max; imail++)
  {
    sprintf(fwdfname, FWDPATH "/%s%s.bcm", path[imail], call);
    strlwr(fwdfname);
    b->opt = o_a | o_b | o_d | o_e | o_f | o_k | o_m | o_u | o_w | o_y ;
    b->gelesen = 0;
    if ((f = s_fopen(fwdfname, "lrt")) != NULL)
    {
      while (fgets(s, BUFLEN - 1, f))
      {
        if (testabbruch())
        {
          retwert = 0;
          break;
        }
        waitfor(e_ticsfull);
        if (*s > ' ')
        {
          cut_blank(s); // remove newline
          count++;
          skip(s);
          if (  (! b->beginn || count >= b->beginn)
              && count <= b->ende && setfullpath(s))
          {
            strlwr(b->mailpath);
            if (! (mailf = s_fopen(b->mailpath, "srt")))
            {
              count--;
              waitfor(e_reschedule);
              continue;
            }
            fgets(s, BUFLEN - 1, mailf);
            mbsend_parse(s, 0);
            fgets(s, BUFLEN - 1, mailf); // fwd trigger
            if (*s == '*')
            {
              count--;          //df3vi: nur anzeigen was WIRKLICH
              s_fclose(mailf);       // noch nicht geforwarded ist!
              waitfor(e_reschedule); // also nicht geloescht oder
              continue;              // auf anderem Wege
            }                        // geforwarded ist
            fgets(s, BUFLEN - 1, mailf); // read: line
            if (fgets(b->betreff, BETREFFLEN, mailf))
            {
              cut_blank(b->betreff);
              strcpy(b->at, atcall(b->at));
              if (nokopf)
              {
                nokopf = 0;
                putf(ms(m_forwardqueue), call);
              }
              safe_strcpy(b->ziel,b->boardname);
              dir_line(count, b->mailfname);
            }
            s_fclose(mailf);
            waitfor(e_reschedule);
          }
        }
      }
      s_fclose(f);
    }
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

static void near dir_messages (char *search)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int i;
  char lastkopf[FNAMELEN+1];

  strcpy(lastkopf, "");
  if (b->abwann)
    putf(ms(m_dirafter), datestr(b->abwann, 12));
  if (search && *search)
    putf(ms(m_searchfor), search);
  for (i = 0; i < treelen; i++)
  {
    if (! tree[i].newestmail || tree[i].newestmail >= b->abwann)
    {
      b->beginn = 1;
      b->ende = MAXBEREICH;
      if (! boardreject(tree[i].name))
        dir_einboard(tree[i].name, 0, search, lastkopf);
    }
    if (testabbruch()) break;
  }
  if (! *lastkopf)
    putf(ms(m_nomessagesfound));
}

/*---------------------------------------------------------------------------*/

void mbdir (char *selektor, int listcmd)
//*************************************************************************
//
//
//*************************************************************************
{
  char sub;
  char boardname[MAXTREELENGTH+1];
  char searchitem[31];
  char *search;

  static char *beftab[] =
  { "USERS", "BOARDS", "MESSAGES", "AFTER", "NEWS", "PATH",
    "SENT", "OUTSTANDING", "HOLD",
    "HOLDUSERS", "HEADERS", "DESTINATIONS", "UNKNOWN", NULL
  };

  enum befnum
  {
    unsinn,
    users, boards, messages, after, news, path,
    sent, outstanding, hold,
    holdusers, headers, dest, unknown
  }
  cmd = unsinn;

  strcpy(searchitem, "");
  scanoptions(selektor);
  get_searchitem(selektor, searchitem, 30);
  selektor += blkill(selektor);
  strupr(selektor);
  if (! listcmd) cmd = (befnum) readcmd(beftab, &selektor, 0);
  if (*searchitem) search = searchitem;
  else search = selektor;
  switch (cmd)
  {
    case unsinn:
      findbereich(selektor);
      if (! *searchitem)
      {
        search = strchr(selektor, '<');
        if (search)
        {
          *search = 0;
          search++;
          skip(selektor);
        }
        else
          search = skip(selektor);
      }
      if (search)
      {
        search += blkill(search);
        if (*search) putf(ms(m_searchfor), search);
      }
      selektor += blkill(selektor);
      if (! *selektor)
        strcpy(boardname, b->prompt);
      else
      {
        strncpy(boardname, selektor, MAXTREELENGTH);
        boardname[MAXTREELENGTH] = 0;
        if (! mbcallok(boardname)) rubrik_completition(boardname); //dk2ui
        if (! *boardname) return;
      }
      if (finddir(boardname, b->sysop)) strcpy(b->prompt, b->boardfullname);
      formoptions();
      if ((b->opt & o_q) && ! b->bereich && (! search || ! *search))
        b->abwann = u->lastquit;
      else
        b->abwann = 0;
      if (b->opt & o_s)
      {
        char lastkopf[10];
        unsigned int i;
        unsigned long beg = b->beginn;
        unsigned long end = b->ende;
        strcpy(lastkopf, "");
        for (i = 0; i < treelen; i++)
        {
          if (! strncmp(tree[i].name, boardname, strlen(boardname)))
          {
            b->beginn = beg;
            b->ende = end;
            dir_einboard(tree[i].name, 0, search, lastkopf);
          }
          if (testabbruch()) break;
        }
      }
      else dir_einboard(boardname, 1, search, "");
      break;
    case outstanding:
      findbereich(selektor);
      skip(selektor);
      strupr(selektor);
      if (! *selektor || strlen(selektor) == 1) //typen-auswahl
      { //df3vi: effektiver als dir_outstandig_all()
        unsigned int i = 0;
        fwdpara_t *ff = fpara();
        while (*ff[i].call) dir_outstanding(ff[i++].call, *selektor);
      }
      else
        if (isforwardpartner(selektor) >= 0)
          dir_outstanding(selektor, 0);
        else
          putf("Syntax: DIR OUTSTANDING <adjacent bbs call>\n");
      break;
    case hold:  //Bulletins ab last_check
      if (! b->lastdirnews) b->lastdirnews = ad_time();
      b->abwann = ad_time() - m.holdtime * HOUR; //since holdtime (db1ras)
      if (b->lastdirnews < b->abwann)             //or if lower,
        b->abwann = b->lastdirnews;               //since last dir news
      b->usermail = 0;
      b->optplus = o_p | o_h | o_e;
      b->optminus = ~(b->optplus | o_v);
      dir_messages(search);
      //Usermails
    case holdusers:
      b->optplus = o_n | o_p | o_h | o_e;
      b->optminus = ~(b->optplus | o_v | o_q);
      dmusers(selektor);
      break;
    case unknown:
      b->optplus = o_n | o_h | o_e;
      b->optminus = ~(b->optplus | o_v | o_q);
      dmusers(selektor);
      break;
    case headers:
      if (*selektor)
        putlog(BOARDINFO, selektor);
      else
      {
        fileio_text fio;
        fio.usefile(BOARDINFO);
        fio.tx();
      }
      break;
    case users:
      sub = *selektor;
      if (sub)
      {
        while (*selektor > ' ') selektor++;
        selektor += blkill(selektor);
        if (! *searchitem) search = selektor;
        switch (toupper(sub))
          {
            case 'A': listuser(search, 0); break;
            case 'L': listuser(search, 1); break;
            case 'M': dmusers(search); break;
          }
      }
      else dir_users(NULL);
      break;
    case boards:
      b->opt = (((o_s | o_j) & b->optminus) | b->optplus);
      b->continous = !! (b->opt & o_c);
      dir_boards(search);
      break;
    case after:
      b->abwann = parse_time(selektor);
      selektor += blkill(selektor);
      b->usermail = 0;
      dir_messages(selektor);
      break;
    case sent:
      if (! *search) strcpy(search, b->logincall);
      // intentionally NO break;
    case messages:
      b->abwann = 0;
      b->usermail = 0;
      dir_messages(search);
      break;
    case news:
      loaduser(b->logincall, u, 1);
      b->abwann = u->lastdirnews;
      b->lastdirnews = ad_time();
      b->usermail = 0;
      dir_messages(search);
      break;
    case path:
      putf(ms(m_dirpath));
      printfwdlist(search);
      break;
    case dest:
      putf(ms(m_dirpath));
      dirfwdlist(search);
      break;
    }
}
