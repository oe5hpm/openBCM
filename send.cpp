/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------
  SEND-Befehl: Abspeichern von Nachrichten
  ----------------------------------------


  Copyright (C)       Florian Radlherr et al
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW removed define for /ex in fwd - removed this anti-bug-fix
//19980120 DG9MHZ fixed reject of mail
//19980121 DG9MHZ made reject parser resistant against space between
//19980124 OE3DZW added rejlog, removed wantmail-log for user@flood
//                removed support for DXL-bbs
//                removed "fwdmode", was not used
//                added ifdef for mailserver (not ready for DOS16)
//19980202 OE3DZW removed check for source-callsign
//                removed check for loop of mail in fwd
//19980202 OE3DZW added check for mailserver / dos16
//19980205 OE3DZW fixed writetext routine, \r was interpreted wrong
//19980205 OE3DZW wrong ifdef undefined mailserver under linux, fixed
//19980209 OE3DZW rejectlog/unknownlog not created, only appended
//19980215 OE3DZW added bytes=0,lines=0 in writetext (fixed double result)
//19980223 OE3DZW added "7+" in listfile
//19980224 OE3DZW fixed msgty, was not initialized
//19980301 OE3DZW fixed writelog, was static but returned 0
//                removed SSID in reply-to address
//                added bid and mailtype to "unknown", cosmetic changes
//                moved tmpbidvorhanden-check to end of wantmail
//                changed structure for check "tmpbidvorhanden"
//                added check for invalid bin-files in std-fwd
//19980304 OE3DZW fixed check for mailtype in reject, more robust now
//19980308 OE3DZW personal mails sysop @ <flood> will be accepted
//                generating new bid when personal mail for own bbs
//                with mybbs != own bbs
//19980309 OE3DZW bad bin-crc -> rejectlog
//19980405 OE3DZW will not accept mails w/o call
//19980404 hrx    info/user-dir as zielboard (not ziel)
//19980404 hrx    guest
//19980407 OE3DZW bytes korrekt
//19980421 OE3DZW no HOLD mail for CP-Mails
//19980424 OE3DZW no,rej w/o reason when no reason given
//                7+rejlog only for bulletins
//19980525 DH3MB  fixed add_dir error
//19980614 OE3DZW diskfull will disable bbs
//19980614 OE3DZW trace for re-routing of usermails
//19980614 OE3DZW added new function update_mail
//19980615 OE3DZW bid was not changed when mail was rerouted
//19980614 hrx    save mail() - displaying number of stored msg at
//                (ms(m_stored)). added condition for createboardfwd to
//                testmail directory()
//19980619 OE3DZW fixed update_mail - if was incorrect
//19980619 OE3DZW createboard will be "0" instead of "2" for fwd-connects
//19980830 OE3DZW fixed: fbb-fwd stopped on bin-err (output of "|")
//19980913 OE3DZW added savebroken-feature
//19980914 OE3DZW changed bin-header, filename w/o timestamp accepted
//19981001 OE3DZW now creates dir if empty inside savemail
//19981015 OE3DZW userfwd can be set to "off" or "passive"
//19990111 OE3DZW will no longer send binary messages to fbb
//19990117 OE3DZW added convat, converts adr for local bulletins
//19990208 OE3DZW moved remeraser to wp.cpp (used as function now)
//19990214 OE3DZW import will only acccept "nnnn" lower case at start of line
//19990215 OE3DZW removed TELL-server
//19990427 OE3DZW import will die if send fails
//19990618 DH3MB  Cleaned up writebin()
//19990621 OE3DZW fixed ^z-bug (e/m-Mails)
//19990816 DH3MB  Restructured wantmail() and testmaildirectory()
//19991024 OE3DZW fixed check for import/kill
//19991028 Jan    added filefwd, modifd writetext/writebin to accept NULL
//19991125 DF3VI  accept mail for unknown users again
//                (problems on sysop-import)
//19991204 DF3VI  added reject-line-parser and reject.bcm-editor,
//                changed searchreject()
//19991222 DF3VI  added mailty 8,9 for info/text-marks
//19991224 DF3VI  added Tell-cmd and Tellresp-Server
//20000101 OE3DZW removed check of ACK: in subject, obsolete
//20000103 DF3VI  (dzw) added flags for 7-info and 7-text
//20000108 OE3DZW added reverse match to reject
//20000109 OE3DZW sender (herkunft) will never be ignored and only assumed
//                to be logincall in user-mode
//20000111 OE3DZW uses homeadr(), fixed newcall
//                rewrote rearchreject()
//20000116 DK2UI  (dzw) added ui-options
//20000402 DF3VI  added reject-functions in searchreject() and wantmail()
//20000508 CZ4WAY (dk2ui) changes for filefwd and import
//20000607 DK2UI  byte count for fbb tx proposal
//20000623 DK2UI  correction in searchreject() from DF3VI
//20000624 DK2UI  changed parse_rej() for regular expressions
//20000713 DK2UI  changed writedida() for sema access
//20001004 DK2UI  save_em() changes address of b->betreff, corrected
//20001004 DK2UI  incoming mybbs limit to age os 30 days
//20001113 DK2UI  evaluation of from and to for swapped mails
//20001113 DK2UI  use of new function log_entry for trace log files
//20010621 DK2UI  own log file for m_filter
//20010715 DF3VI  make attach during import possible (writebin, writetext)
//20010805 DF3VI  force user to set lifetime on send
//20010914 DK2UI  DIDADIT with CRC in BIN header
//20010916 DK2UI  .attach with CRC in BIN header
//20021216 DH8YMB Option "E" und "O" in searchreject von DF3VI hinzu
//20030126 DH8YMB Invers-Matching bei searchreject wieder hinzu
//20030203 DH8YMB OR-Matching in searchreject hinzu
//20030307 DH8YMB #BIN# und #DIDADIT# bei HTTP-Session ignorieren
//20030517 DH6BB  Einbau Mime2Bin fuer SMTP mit Anhang
//20030529 DH6BB  BugFix im smtp_convert bei non-base64
//20030606 DH8YMB quoted2bin Funktion fuer SMTP mit Anhang hinzu
//20030727 DB1RAS HROUTE-Fix bei writeheader
//20030809 hpk    added CB-BCMNET gateway functions
//20030809 hpk    some changes in HOLD for CB-BCMNET Login-concept
//20041028 DH8YMB neue Funktion sysop_sysinfomail hinzu, Aufruf in run_crontab

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int parse_headerline (void)
//*************************************************************************
//
// Parses headerline of a mail
// uses: b->mailpath
//
//*************************************************************************
{
  lastfunc("parse_headerline");
  static char buf[BUFLEN];
  static long n; //dk2ui: bytes for FBB proposal
  FILE *f;

  strlwr(b->mailpath);
  f = s_fopen(b->mailpath, "srt");
  if (f)
  {
    fgets(buf, BUFLEN - 1, f);
    n = filesize(b->mailpath) - strlen(buf);
    fgets(buf, BUFLEN - 1, f);
    n -= strlen(buf);
    fgets(buf, BUFLEN - 1, f);
    n -= strlen(buf);
//    fgets(buf, BUFLEN - 1, f);       // title
//    fgets(buf, BUFLEN - 1, f);       // own R:-line
//    n -= strlen(buf);
    fseek(f, 0, SEEK_SET);
    fgets(buf, BUFLEN - 1, f); //first line = headerline
    s_fclose(f);
    buf[254] = 0; //should not be needed, but to be save..
    rm_crlf(buf);
    mbsend_parse(buf, 0); //parse headerline
    if (b->job == j_fbb_send_propose) b->bytes = n;
    return 1; //success
  }
  else
    return 0; //fail
}

/*---------------------------------------------------------------------------*/

int mbsend_parse (char *befbuf, int def)
//*************************************************************************
//
//  Examines a line containing the fields of the SEND-command and
//  writes the parameters to the corresponding variables of the
//  current task
//
//  def: 0 = It's the first line of a mail-file (including file-length, etc.)
//           eg used in add_dirlist (mbreorg.cpp)
//       1 = It's a line entered by a user or received in S&F
//       2 = Used for transfering a mail
//
//*************************************************************************
{
  lastfunc("mbsend_parse");
  char lt[6];

  if (def != 2)
  {
    *b->betreff = 0;
    *b->ziel = 0;
    *b->herkunft = 0;
    *b->at = 0;
    *b->bid = 0;
    b->bytes = 0;
    b->lines = 0;
    b->lifetime = 0;
    b->binstart = 0;
    *b->frombox = 0;
  }
  if (def != 1)
  {
    b->mailtype = '?'; // OE3DZW: vorbesetzen wenn lesen
    b->fwdhold = b->fwdprop = b->replied = b->conttype = b->eraseinfo = '!';
  }
  // Initialize the flags which are not used in this version
  b->flag7 = b->flag8 = b->flag9 = b->flaga =
  b->flagb = b->flagc = b->flagd = b->flage = b->flagf = b->flag0 = '!';
  // First parameter = destination
  befbuf = nexttoken(befbuf, b->ziel, 8);
  strupr(b->ziel);
  // b->ziel:      to-field,  to which the mail is destinated
  // b->zielboard: Board, to which we write the mail (may be different)
  // Don't allow (back-)slashes
  subst(b->ziel, '/', '_');
  subst(b->ziel, '\\', '_');
  // convert ziel ?
  // dh8ymb 27.11.05
  strcpy(b->zielboard, b->ziel);
  // Only used to decide if we store the message in the info- or in the
  // user-path
  b->usermail = (char) mbcallok(b->zielboard); //hrx
  // Read the remaining parameters
  while (*befbuf)
  {
    switch (*befbuf)
    {
    case '<':
      befbuf = nexttoken(befbuf + 1, b->herkunft, 6);
      strupr(b->herkunft);
      break;
    case '@':
      *b->at = 0;
      if (befbuf[2] != ' ' || befbuf[3] != ' ')
      {
        befbuf = nexttoken(befbuf + 1, b->at, HADRESSLEN);
        strupr(b->at);
        if (! strstr(b->at, m.boxname)) subst1(b->at, '-', 0);
      }
      else befbuf++;
      break;
    case '$':
      befbuf = nexttoken(befbuf + 1, b->bid, 12);
      strupr(b->bid);
      break;
    case '#':
      if (isdigit(befbuf[1]) || befbuf[1] == ' ')
      {
        befbuf = nexttoken(befbuf + 1, lt, 5);
        b->lifetime = atoi(lt);
        if (b->lifetime > 999 || b->lifetime < 0) b->lifetime = 0;
#ifdef USERLT
        if (! def)
        {
          if (*befbuf != '%')
          {
            befbuf = nexttoken(befbuf, lt, 3);
            b->boardlife_max = atoi(lt);
          }
          else //old mails without boardlife_max field (db1ras)
            b->boardlife_max = b->lifetime;
        }
#else
        if (! def) while (*befbuf != '%') befbuf++;
#endif
      }
      else goto betreff;
      break;
    case '%':
      if (! def)
      {
        b->lines = (unsigned) ascdez(befbuf + 1, 2);
        b->bytes = inonlin(ascdez(befbuf + 3, 3));
        befbuf += 6;
      }
      else goto betreff;
      break;
    case '~':
      if (! def)
      {
        befbuf = nexttoken(befbuf + 1, b->frombox, 6);
        strupr(b->frombox);
      }
      else goto betreff;
      break;
    // Mailflags (OE3DZW)
    case '|':
      if (b->forwarding == fwd_none || ! def)
      {
        b->mailtype = befbuf[1];
        b->fwdhold  = befbuf[2];
        b->fwdprop  = befbuf[3];
        b->replied  = befbuf[4];
        b->conttype = befbuf[5];
        if (b->conttype < '!')
          b->conttype = '!'; //if set to 0 (bug in bcm139x22)
        b->eraseinfo = befbuf[6];
      }
      befbuf += 17;
      break;
    case '=':
      if (! def)
      {
        b->binstart = ascdez(befbuf + 1, 3);
        befbuf += 4;
      }
      else goto betreff;
      break;
    case ' ': befbuf++; break;
    default:
    betreff:
      if (def != 2)
      {
        strncpy(b->betreff, befbuf, BETREFFLEN);
        b->betreff[BETREFFLEN] = 0;
      }
      befbuf += strlen(befbuf);
    }
  }
  // Guess the mailtype, if we don't already know it
  if (b->mailtype == '?') b->mailtype = mbmailt(b->ziel, b->at);
  // Attention: Mind the difference between frombox, herkunft and logincall:
  //        herkunft  = The call of the user, who has written the mail
  //        frombox   = BBS, *WE* have the mail received from
  //        logincall = AX.25-call of the partner without SSID
  //        DOS:     cr-lf 0xd 0xa ^M ^L  \r \n
  //        Unix:    lf    0xa     ^L     \n
  //        Packet:  cr    0xd     ^M     \r
  if (def == 1) //entered by user or received in s&f
  {
    //if the Sender was not given,we use the login-call instead
    //(only in user-mode)
    if (! *b->herkunft && b->forwarding == fwd_none)
      strcpy(b->herkunft, b->logincall);
    if (! *b->bid)
    {
      if (b->forwarding != fwd_none && ! b->usermail)
        strcpy(b->bid, "NOBID");
        // If we don't receive a BID in S&F, we defer this message
      else
        strcpy(b->bid, newbid());
        // But if we don't receive a MID, we create one
    }
    // We keep the mailbox, we have the mail received from in mind
    if (b->forwarding == fwd_standard
#ifdef _FILEFWD
        || b->forwarding == fwd_file
#endif
       )
      strcpy(b->frombox, b->logincall);
  }
  return *b->ziel;
}

/*---------------------------------------------------------------------------*/

void writeheader (FILE *sendf, int transfer)
//*************************************************************************
//
//  Creates a mail header
//
//*************************************************************************
{
  lastfunc("writeheader");
  unsigned int i = 0;

  fprintf(sendf, "%s < %s ", b->ziel, b->herkunft);
  if (*b->frombox) fprintf(sendf, "~%s ", b->frombox);
  fprintf(sendf, "@%-39s ", b->at);
  if (*b->bid) fprintf(sendf, "$%-12s ", b->bid);
#ifdef USERLT
  // 2 LT-fields in headerline:
  // 1st from user (0 if none) needed for forward, 2nd is the real box LT
  fprintf(sendf, "#%3.3d %3.3d ", b->lifetime, b->boardlife_max);
#else
  fprintf(sendf, "#%-3d ", b->lifetime);
#endif
  if (transfer)
  {
    fprintf(sendf, "%%%s", dezasc(b->lines, 2));
    fprintf(sendf, "%s ",  dezasc(nonlin(b->bytes), 3));
    fprintf(sendf, "=%s ", dezasc(b->binstart, 3));
  }
  else
    fprintf(sendf, "%%!!!!! =!!! "); // Set lines, bytes and
                                     // bin-offset to 0
  fprintf(sendf, "|%c!!!!!!!!!!!!!!! \n", b->mailtype);
  while (strlen(b->destboxlist) < DESTBOXLEN) strcat(b->destboxlist, ".");
  fprintf(sendf, "%s\n", b->destboxlist);
  while (i++ < 252) fputc('.', sendf); // Create an empty "Read:"-line
  fputc(LF, sendf);
  fprintf(sendf, "%s\n", b->betreff);
  if (! transfer)
  {
    if (b->mailtype != 'A' || ! *b->frombox)
      fprintf(sendf, "%s\n", makeheader(! *b->frombox));
    // do not generate any header-texts for wp-updates
    if (   b->mailtype == 'P' &&  ! strcmp(b->uplink, "Import")
        && ! strcmp(b->betreff, "WP Update"))
    {
      fputc(LF, sendf);
      return;
    }
    if (! *b->frombox)
    {
      if (b->forwarding == fwd_user)
        fprintf(sendf, "X-Info: User S&F received from %s at %s\n",
                b->logincall, m.boxadress);
      else
      {
        fprintf(sendf, "From: %s @ %s", b->herkunft, m.boxadress);
        loaduser(b->logincall, u, 0);
        if (*u->name) fprintf(sendf, " (%s)", u->name);
        if (m.smoothheader) fprintf(sendf, " To: %s", b->ziel);
        else fprintf(sendf, "\nTo:   %s", b->ziel);
        if (*b->at) fprintf(sendf, " @ %s", b->at);
        fputc(LF, sendf);
        if (! m.smoothheader)
        {
          if (u->mybbsok)
          {
            char my[HADRESSLEN + 1];
            strcpy(my, u->mybbs);
            subst1(my, '-', 0); // remove optional SSID in address
            expand_hadr(my, 3);
            if (strcmp(my, m.boxadress) && strcmp(b->herkunft, m.boxname))
              fprintf(sendf, "Reply-To: %s @ %s\n", b->herkunft, my);
          }
        }
      }
      if (! strcmp(b->uplink, "Import"))
        fprintf(sendf, "X-Info: This message was generated automatically\n");
      else if (b->forwarding == fwd_none
               && ! b->pwok && ! b->sysop && ! t->socket
               && strcmp(b->uplink, "Mailsv"))
        fprintf(sendf, "X-Info: No login password\n");
      else if (b->forwarding == fwd_user
               && ! *u->password && u->sfpwtype != 100)
        fprintf(sendf, "X-Info: No upload password\n");
      else if (b->forwarding == fwd_none
               && (b->pwok || b->sysop))
        fprintf(sendf, "X-Info: Sent with login password\n");
      else if (b->forwarding == fwd_user
               && *u->password && u->sfpwtype == 100)
        fprintf(sendf, "X-Info: Sent with upload password\n");
      else if (b->http)
           {
             if (! m.callformat)
               fprintf(sendf, "X-Info: Received by HTTP-frontend\n");
             else
               fprintf(sendf, "X-Info: Received from %s by HTTP-frontend\n",
                       b->peerip);
           }
      if (! strcmp(b->uplink, "Mailsv"))
        fprintf(sendf, "X-Info: Mail generated by mail server\n");
#ifdef _BCMNET_LOGIN
      if (b->logintype == 0 && ! b->sysop)
        fprintf(sendf, "X-Info: Mail was sent from a guest\n");
#endif
#ifdef __FLAT__
      switch (b->headermode)
      {
        case _SMTP:
          if (! m.callformat)
            fprintf(sendf, "X-Info: Received by SMTP-gateway\n");
          else
            fprintf(sendf, "X-Info: Received from %s by SMTP-gateway\n",
                    b->peerip);
        break;
        case _NNTP:
          if (! m.callformat)
            fprintf(sendf, "X-Info: Received by NNTP-gateway\n");
          else
            fprintf(sendf, "X-Info: Received from %s by NNTP-gateway\n", b->peerip);
        break;
        default: fputc(LF, sendf);
      }
#else
      fputc(LF, sendf);
#endif
    }
  }
}

/*---------------------------------------------------------------------------*/

void writelines (void)
//*************************************************************************
//
//  Writes length, number of lines and BIN-offset to a completely
//  received message
//
//*************************************************************************
{
  lastfunc("writelines");
  FILE *sendf;
  char *pos;
  char save;

  if (! (sendf = s_fopen(b->mailpath, "sr+t")))
    trace(fatal, "writelines", "fopen %s errno=%d %s", b->mailpath,
                                                 errno, strerror(errno));
  fgets(b->line, BUFLEN - 1, sendf);
  fseek(sendf, 0L, SEEK_SET);
  pos = strstr(b->line, " %");
  if (pos)
  {
    save = pos[7];
    sprintf(pos + 2, "%s", dezasc(b->lines, 2));
    sprintf(pos + 4, "%s", dezasc(nonlin (b->bytes), 3));
    pos[7] = save; // Overwrite 0-byte
  }
  pos = strstr(b->line, " =");
  if (pos)
  {
    save = pos[5];
    sprintf(pos + 2, "%s", dezasc(b->binstart, 3));
    pos[5] = save; // Overwrite 0-byte
  }
#ifdef USERLT
  pos = strstr(b->line, " #");
  if (pos)
  {
    save = pos[9];
    sprintf(pos + 2, "%3.3d %3.3d", b->lifetime, b->boardlife_max);
    pos[9] = save; // Overwrite 0-byte
  }
#endif
  fputs(b->line, sendf);
  s_fclose(sendf);
}

/*---------------------------------------------------------------------------*/

void writemailflags (void)
//*************************************************************************
//
//  Writes the 16 mail-flags into an existing mail
//
// used globals: mailpath, mailfname, all mailflags
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "writemailflags");
  lastfunc(name);
  char *pos;
  char save;
  char ok;
  FILE *sendf;
  handle fh;

  strlwr (b->mailpath);
  if (! *b->mailpath)
    trace(replog, name, "no mailpath %s %s %s", b->herkunft,
                                                b->ziel, b->bid);
  if (! (sendf = s_fopen(b->mailpath, "sr+t")))
    trace(fatal, name, "fopen %s errno=%d %s", b->mailpath,
                                               errno, strerror(errno));
  fgets(b->line, BUFLEN - 1, sendf);
  fseek(sendf, 0L, SEEK_SET);
  if ((pos = strstr(b->line, " |")) != NULL)
  {
    save = pos[8];
    sprintf(pos + 2, "%c%c%c%c%c%c",  //%c%c%c%c%c%c%c%c%c%c",
      b->mailtype, b->fwdhold, b->fwdprop, b->replied, b->conttype, b->eraseinfo
    //,b->flag7, b->flag8, b->flag9, b->flaga, b->flagb, b->flagc, b->flagd,
    // b->flage, b->flagf, b->flag0
      );
    pos[8] = save;
  }
  fputs(b->line, sendf);
  s_fclose(sendf);
  strlwr(b->listpath);
  if (! *b->listpath)
    trace(replog, name, "no listpath %s %s %s", b->herkunft,
                                                b->ziel, b->bid);
  if ((fh = s_open(b->listpath, "sr+b")) != EOF)
  {
    strupr(b->mailfname);
    seek_fname(fh, b->mailfname, &ok, 1);
    if (ok)
    {
      _read(fh, b->line, BLEN);
      b->line[61] = b->mailtype;
      b->line[62] = b->fwdhold;
      b->line[63] = b->fwdprop;
      b->line[64] = b->replied;
      b->line[65] = b->conttype;
      b->line[66] = b->eraseinfo;
      lseek(fh, -(LBLEN), SEEK_CUR);
      _write(fh, b->line, BLEN);
    }
    s_close(fh);
  }
  /* comment it out and see if everything is ok then..
  if (!b->usermail) //TODO dangerous?
  { fh = s_open(CHECKNAME, "sr+b");
    char found;
    if (fh != EOF)
    { seek_fname (fh, b->mailfname, &found, 1);
      if (found)
      { _read(fh, b->line, BLEN);
        b->line[61] = b->mailtype;
        b->line[62] = b->fwdhold;
        b->line[63] = b->fwdprop;
        b->line[64] = b->replied;
        b->line[65] = b->conttype;
        lseek (fh, -(LBLEN), SEEK_CUR);
        _write(fh, b->line, BLEN);
      }
      s_close(fh);
    }
    else trace(serious,name, "check");
  }
  */
}

/*---------------------------------------------------------------------------*/

long near writebin (char *binline, FILE *sendf)
//*************************************************************************
//
//  Schreibt ein AUTOBIN-File
//
//*************************************************************************
{
  lastfunc("writebin");
  char bincmd[BINCMDLEN];
  int a;
  off_t flen, len;
  char rxed_crc = 0;
  unsigned short int rx_crc = 0;
  crcthp crc_;
  unsigned long int bps;
  time_t rxtime, startt = ad_time();

  flen = len = atol(binline + 5); // Get the file length
  if (strstr(binline, "#|"))      // Get the CRC
  {
    rx_crc = (unsigned short int) atol(strstr(binline, "#|") + 2);
    rxed_crc = 1;
  }
  if (sendf)
  {
    fflush(sendf);
    b->binstart = ftell(sendf);
    s_fclose(sendf);
  }
  if (len)
  {
    if (b->forwarding == fwd_none)
    {
      putf("#OK#\n");
      putflush();
    }
    if ((sendf = s_fopen(b->mailpath, "lr+b")) != NULL)
    {
      s_fsetopt(sendf, 1);
      fseek(sendf, 0, SEEK_END);
      if (ftell(sendf) != b->binstart)
        trace(serious, "writebin", "wrong filesize");
      memset(bincmd, 0, BINCMDLEN);
      fwrite(bincmd, BINCMDLEN, 1, sendf);
    }
    while (len--)
    {
      a = bgetv();
      if (sendf) fputc(a, sendf);
      crc_.update(a);
      if (! (len & 511))
      {
        if (b->forwarding != fwd_none) timeout(m.fwdtimeout);
        else timeout(m.usrtimeout);
      }
    }
    if (sendf)
    {
      // DH3MB: The #BIN#-header will be written to the mail file
      // without any changes; only a CRC will be inserted, if the
      // #BIN#-header sent by the user/forward-partner did not include one
      if (rxed_crc)
      {
        binline[BINCMDLEN] = 0; //dh8ymb: just to be safe
        snprintf(bincmd, BINCMDLEN, "%s\n", binline);
      }
      else
      {

        if (! strchr(binline + 5, '#'))
          snprintf(bincmd, BINCMDLEN, "%s#|%05u\n", binline, crc_.result);
        else
        {
          subst1(binline + 5, '#', 0);
          snprintf(bincmd, BINCMDLEN, "%s#|%05u#%s\n",
                   binline, crc_.result, binline + strlen(binline) + 1);
        }
      }
      fseek(sendf, b->binstart, SEEK_SET);
      fwrite(bincmd, BINCMDLEN, 1, sendf);
      s_fclose(sendf);
    }
    if (rxed_crc && rx_crc != crc_.result)
    {
      flen = 0;
      if (b->forwarding != fwd_none && ! (b->opt & o_f) // Only in ASCII-fwd!
#ifdef _FILEFWD
         && b->forwarding != fwd_file
#endif
        )
        putf("|");
      if (b->forwarding == fwd_none) putf("\nCRC-Error\n");
#ifdef _FILEFWD
      if (b->forwarding == fwd_file)
        putf("\nAuto-Bin CRC error. (input file broken ?)\n");
#endif
    }
    else if (b->forwarding == fwd_none)
      {
        rxtime = ad_time() - startt;
        if (! rxtime) rxtime = 1L;
        bps = flen / rxtime;
        putf("\nBIN-RX OK %05u/%04Xh, %lu bit/s\n",
              crc_.result, crc_.result, bps << 3);
      }
  }
  if (b->inputfile  // DG9MHZ
#ifdef _FILEFWD
      && b->forwarding != fwd_file
#endif
    )
  {
    s_fclose(b->inputfile);
//    b->inputfile = NULL;
    t->input = (io_t) b->oldinput;
    if (b->oldinput == io_file) // Wenn IMP-Datei, Handle wiederherstellen
    {
      b->oldinput = b->oldoldinput;
      b->inputfile = b->oldfile;
    }
    else b->inputfile = NULL;
  }
  return flen;
}

/*---------------------------------------------------------------------------*/

#ifdef FEATURE_DIDADIT
long near writedida (FILE *sendf)
//*************************************************************************
//
//  Schreibt ein DIDADIT-File
//
//*************************************************************************
{
  lastfunc("writedida");
  char bincmd[BINCMDLEN];
  long binsize, len, bps;
  time_t rxtime, startt = ad_time();
  char filename[FNAMELEN + 1], fullname[FSPATHLEN + FNAMELEN + 1], buf[1024];
  handle fh;
  crcthp crc_;
  fileio_dida fio;

  fio.set_path(TEMPPATH);
  fio.set_waitforheader(0);
  fio.rx();
  fio.get_filename(filename);
  strcpy(fullname, TEMPPATH);
  strcat(fullname, "/");
  strcat(fullname, filename);
  crc_.readfile(fullname, 0);
  binsize = fio.get_size();
  if (sendf)
  {
    fflush(sendf);
    b->binstart = ftell(sendf);
    s_fclose(sendf);
  }
  if ((fh = s_open(fullname, "srb")) == EOF) return 0;
  s_setopt(fh, 2);
  if ((sendf = s_fopen(b->mailpath, "lr+b")) != NULL)
  {
    s_fsetopt(sendf, 1);
    fseek(sendf, 0L, SEEK_END);
    if (ftell(sendf) != b->binstart)
      trace(serious, "writedida", "wrong filesize");
    memset(bincmd, 0, BINCMDLEN);
    sprintf(bincmd, "#BIN#%05lu#|%05u#$%08lX#%s\n", binsize,
             crc_.result, getdostime(file_isreg(fullname)), filename);
    fwrite(bincmd, BINCMDLEN, 1, sendf);
    while ((len = _read(fh, buf, 1024)) > 0)
    {
      fwrite(buf, len, 1, sendf);
      waitfor(e_ticsfull);
    }
    s_close(fh);
    s_fclose(sendf);
  }
  rxtime = ad_time() - startt;
  if (! rxtime) rxtime = 1L;
  bps = binsize / rxtime;
  putf("\nDIDADIT-RX OK %05u/%04Xh, %ld bit/s\n",
        crc_.result, crc_.result, bps << 3);
  return binsize;
}
#endif

/*---------------------------------------------------------------------------*/

static void near put_rejectlog (char * reason)
//*************************************************************************
//
//  Fuegt einen Eintrag in trace\reject.bcm hinzu
//
//*************************************************************************
{
  char buf[120];

  sprintf(buf, "%c %-6s>%-8s@%.22s~%-6s$%-12s %s",
                b->mailtype, b->herkunft, b->ziel,
                b->at, b->frombox, b->bid, reason);
  log_entry(REJECTLOGNAME, buf);
}

/*---------------------------------------------------------------------------*/

static int writetext (FILE *sendf)
//*************************************************************************
//
//  Gets the text from the current input and stores it into a mail file
//
//  Returns 1, if a ACK-message should be generated, 0 otherwise
//
//*************************************************************************
{
  lastfunc("writetext");
  char *eof = NULL;
  char *line = b->line;
  int scanr = 1;
  char headeradress[HADRESSLEN + 1] = { 0 };
  char rline[BLEN];
  char *adrp;
  int hops = 0;
  int invalid = 0;
  int broken7p = 0;
  int send_ack = 0;
  char linestart = 1; // DH3MB: We are at the beginning of a new line
  int attachabbruch = 0; // DH8YMB: Falls attach abgebrochen wurde...
  int inside7p = 0;   // inside a 7p-encoded file
  int inside7inf = 0; // inside a 7p-encoded text
  int inside7txt = 0; // inside a 7p-encoded info
  unsigned char found = FALSE;
  b->binstart = 0;
  b->bytes = 0;
  b->lines = 0;
  line[251] = 0;  // we never know what will happen..
  b->conttype = '!'; //conttype is not known
  char *a;
  char *kl;
  char at[CALLEN+1];
  char attachname[LINELEN+1]; // DH3MB
  char pline[LINELEN];
  char fromto[DIRLEN+1];
  char n[NAMELEN + 1];
  int i = 0;

  strcpy(rline, "");
  int no_import = strcmp(b->uplink, "Import");
  do
  {
    waitfor(e_ticsfull);
    if (! getline(b->line, BUFLEN, 0))
      eof = line; // DH3MB: feof(b->inputfile) is checked
    {
      if ((a = strchr(line, CR)) != NULL)
      {
        *a++ = LF;
        *a = 0;
      }
    }
    //check if user wants to abort mail input
    if (b->forwarding == fwd_none
        && ( strchr(line, CTRLX)
             || ! stricmp(line, "/ab\n") )) // Received Ctrl-X or /ab
    {
      b->bytes = 0;
      b->lines = 0;
      if (sendf) s_fclose(sendf);
      return NO;
    }
    if (linestart)
    {
      //check for enquiry of acknowledge mail
      // ACK-messages only on personal mails
      if (sendf && ! stricmp(line, "/ack\n") && b->mailtype == 'P')
        send_ack = 1;
      //check for 7plus-flags
      if (inside7p)
      {
        if (strstr(line, STOP7PFLAG) == line)
        {
          inside7p = 0;
          if (! broken7p) b->conttype = '7';
        }
        else
          if ((strlen(line) > 1) && (! line_7p(line))) broken7p = 1;
     //Zeilen mit Return machen 7+ nicht kaputt
      }
      else
      {
        if (strlen (line) == 70 && strstr(line, START7PFLAG) == line)
          inside7p = 1;
        //recognise 7plus info and text flags
        //if not 7plus-encoded (else text/info is ignored)
        if (b->conttype == '!' && strstr(line, STARTINFOFLAG) == line)
          inside7inf = 1;
        if (b->conttype == '!' && strstr(line, STARTTEXTFLAG) == line)
          inside7txt = 1;
        //only accept balanced flags (else it also could be some text
        //inside a mail
        if (inside7inf && b->conttype == '!'
            && strstr(line, STOPINFOFLAG) == line)
          b->conttype = '8';
        if (inside7txt && b->conttype == '!'
            && strstr(line, STOPTEXTFLAG) == line)
          b->conttype = '9';
        if (b->conttype == '!' && ! inside7txt && ! inside7inf
            && strstr(line, HTMLSTARTTAG) == line)
          b->conttype = 'H'; //html
      }
    }
    //check for end-of-mail-marker
    if (! eof) eof = strchr(line, CTRLZ); // Received Ctrl-Z
#ifdef _FILEFWD
    if (b->forwarding == fwd_file && ! strcmp(line, "/EX\n"))
      eof = line;
#endif
    // Received an end-of-mail-specifier
    if (b->forwarding == fwd_none)
    {
      if (no_import)
      { // DH3MB: "NNNN" or "nnnn" at end of line
        if (linestart && ! stricmp(line, "/EX\n")) eof = line;
        if (! eof) eof = strstr(line, "nnnn\n");
        if (! eof) eof = strstr(line, "NNNN\n");
        // DH3MB: "***END" at end of line, ignore case
        if (! eof) eof = stristr(line, "***end\n");
        // DH3MB: "/EX" at beginning of line, ignore case
      }
      else  //on import "nnnn" is only accepted at start of a new line
        if (linestart && ! strcmp(line, "nnnn\n")) eof = line;
    }
    if (eof)
    {
      *eof = 0;
      if (*line && sendf) fputs(line, sendf);
    }
    else
    { // OE3DZW: Attach a file to the mail as BIN-part (modified by DH3MB)
      if (   linestart && b->sysop && ! strncmp (line, ".attach ", 8)
          && b->forwarding == fwd_none) // && t->input != io_file)
      {
        subst1(line, LF, 0);
        subst(line, '\\', '/');
        cut_blank(line);
        if ((adrp = strrchr(line + 8, '/')) != NULL)
          strncpy(attachname, ++adrp, LINELEN);
        else strncpy(attachname, line + 8, LINELEN);
        attachname[LINELEN] = 0;
        // Handle zwischenspeichern, wenn IMP-Datei
        if (t->input == io_file)
        {
          b->oldoldinput = b->oldinput;
          b->oldfile = b->inputfile;
        }
        if (*attachname)
        {
          crcthp crc_;
          crc_.readfile(line + 8, 0L);
          if ((b->inputfile = s_fopen(line + 8, "lrb")) != NULL)
          {
            fseek(b->inputfile, 0L, SEEK_END);
            unsigned long binsize = ftell(b->inputfile);
            rewind(b->inputfile);
            waitfor(e_ticsfull);
            sprintf(line, "#BIN#%05lu#|%05u#$%08lX#%s\n", binsize,
                            crc_.result, getdostime(file_isreg(line + 8)),
                            attachname);
            b->oldinput = t->input;
            t->input = io_file;
            attachabbruch = 0;
            waitfor(e_ticsfull);
          }
          else
          {
            *line = 0;
            attachabbruch = 1;
            putf(ms(m_filenotfound), line + 8);
          }
        }
        else
        {
          *line = 0;
          attachabbruch = 1;
          putf("\".attach\" without a filename is not working.\n");
        }
      }
#ifdef FEATURE_DIDADIT
      if (linestart && ! strncmp(line, "#DIDADIT#", 9))
      {
        // do NOT send binary personal messages to a fbb-bbs
        // it turned out that this bbs is not able to deal with
        // such messages correctly
        // ...and do NOT send #DIDADIT# via HTTP, NNTP or SMTP
        if (   boxbinok(0)
            && (! b->http)
            && stricmp(b->uplink, "SMTP")
            && stricmp(b->uplink, "NNTP"))
        {
          subst1(line, LF, 0);
          b->bytes = writedida(sendf);
          if (! b->bytes && b->forwarding == fwd_standard)
            put_rejectlog("bin crc");
          b->conttype = 'B'; // 'B' didadit wird als Autobin gespeichert
          eof = line;
          *line = 0;
          sendf = NULL;
        }
        else
        {
          if (b->http
              || ! stricmp(b->uplink, "SMTP")
              || ! stricmp(b->uplink, "NNTP"))
            subst(line, '#', '+'); // replace "#DIDADIT#" by "+DIDADIT+"
          else
            if (sendf)
              fprintf(sendf, " %s", line); // replace "#DIDADIT#" by " #DIDADIT#"
        }
      }
#endif
      if (linestart && ! strncmp(line, "#BIN#", 5))
      {
        // do NOT send binary personal messages to a fbb-bbs
        // it turned out that this bbs is not able to deal
        // with such messages correctly
        // ...and do NOT send #BIN# via HTTP
        if (boxbinok(0) && isdigit(line[5])
            && (! b->http))
        {
          subst1(line, LF, 0);
          b->bytes = writebin(line, sendf);
          if (! b->bytes && b->forwarding == fwd_standard)
            put_rejectlog("bin crc");
          b->conttype = 'B'; // 'B' autobin
          eof = line;
          *line = 0;
          sendf = NULL;
        }
        else
        {
          if (b->http)
            subst(line, '#', '+'); // replace "#BIN#" by "+BIN+"
          else
            if (sendf)
              fprintf(sendf, " %s", line); // replace "#BIN#" by " #BIN#"
        }
      }
      else
      {
        if (linestart && stristr(line, "//e") == line)
          *line = '\\';
        if (scanr && b->forwarding == fwd_user && strncmp(line, "R:", 2))
          scanr = 0;
        if (! (scanr && b->forwarding == fwd_user) && sendf)
          fputs(line, sendf);
        if (scanr && (   b->forwarding == fwd_standard
#ifdef _FILEFWD
                      || b->forwarding == fwd_file
#endif
           )         )
        {
          if ((adrp = getheaderadress(line)) != NULL)
          {
            strcpy(headeradress, adrp);
            if (    ! b->usermail
                 && ! strcmp(adrp, m.boxadress)
                 && line[13] == 'z')
            {
              if (strcmp(b->logincall, m.boxname)) invalid = 1;
              trace(report, "writetext", "loop %s>%s@%s $%s",
                                         b->herkunft, b->ziel, b->at, b->bid);
            }
            if (*rline)
              update_hadr(rline, hops, 0);
            hops++;
            strncpy(rline, line, 127);
            rline[127] = 0;
            // swaplog von bid-Aenderungen
            if (! found && (adrp = strstr(rline, "$:")) != NULL)
            {
              if (strncmp(adrp + 2, b->bid, strlen(b->bid)))
              {
                found = TRUE;
                snprintf(pline, LINELEN,
                          "%-6s>%-8s@%.22s~%-6s $%-.12s->%-.12s",
                          b->herkunft, b->ziel, b->at, b->frombox,
                          adrp + 2, b->bid);
                log_entry(SWAPLOGNAME, pline);
                if (! b->usermail) b->boardlife_max = m.defswaplf;
              }
            }
            strncpy(rline, line, BLEN);
            rline[BLEN - 1] = 0;
          }
          else //last R-line (rline) and first line after (line)
          {
            if (*rline)
            {
              if ((adrp = strstr(rline, "LT:")) != NULL)
                b->lifetime = atoi(adrp + 3);
              update_hadr(rline, hops, 1);
            }
            scanr = 0; b->bytes = 0L; b->lines = 0U;
            if (   ! strncasecmp(line, "from: ", 6)
                || ! strncmp(line, "de: ", 4)
                || ! strncmp(line, "de ", 3)
                || ! strncmp(line, "fm ", 3))
            {
              kl = strchr(line, '(');
              if (kl && strchr(kl, ')') && strstr(line, b->herkunft))
              {
                i = 0;
                kl++;
                n[0] = 0;
                while (i < NAMELEN
                       && (kl[i] > '@' || kl[i] == ' ' || kl[i] == '-'))
                {
                  n[i] = kl[i];
                  n[++i] = 0;
                }
                if (strlen(n) > 1)
                  set_name(b->herkunft, n);
              }
              if (! b->usermail)
              {
                kl = strchr(line, ' ') + 1;
                while (*kl == ' ') kl++;
                strncpy(fromto, kl, CALLEN);
                fromto[CALLEN] = 0;
                subst1(fromto, '@', 0);
                subst1(fromto, ' ', 0);
                subst1(fromto, '-', 0);
                if (strcasecmp(fromto, b->herkunft))
                {
                  sprintf(rline, "%-6s>%-8s@%.22s~%-6s $%-12s from: %s",
                           b->herkunft, b->ziel, b->at, b->frombox,
                           b->bid, fromto);
                  log_entry(SWAPLOGNAME, rline);
                  b->boardlife_max = m.defswaplf;
                }
                if (! stristr(line, "to: "))
                {
                  b->bytes += strlen(line);
                  if (getline(b->line, BUFLEN, 0))
                  {
                    //char *a;
                    if ((a = strchr(line, CR)) != NULL)
                    {
                      *a++ = LF;
                      *a = 0;
                    }
                    if (sendf) fputs(line, sendf);
                  }
                  else eof = line;
                }
                if ((kl = stristr(line, "to: ")) != NULL
                    || ! strncmp(line, "to ", 3))
                {
                  if (! strncmp(line, "to ", 3)) kl = line;
                  kl += 3;
                  while (*kl == ' ') kl++;
                  strncpy(fromto, kl, DIRLEN);
                  fromto[DIRLEN] = 0;
                  subst1(fromto, '@', 0);
                  subst1(fromto, ' ', 0);
                  if (   (strlen(b->ziel) != 6
                          && strcasecmp(fromto, b->ziel))
                      || (strlen(b->ziel) == 6
                          && strncasecmp(fromto, b->ziel, 6)))
                  {
                    sprintf(rline, "%-6s>%-8s@%.22s~%-6s $%-12s to: %s",
                             b->herkunft, b->ziel, b->at, b->frombox,
                             b->bid, fromto);
                    log_entry(SWAPLOGNAME, rline);
                    b->boardlife_max = m.defswaplf;
                  }
                  if ((kl = strchr(kl, '@')) != NULL)
                  {
                    kl++;
                    while (*kl == ' ') kl++;
                    strncpy(fromto, kl, DIRLEN);
                    fromto[DIRLEN] = 0;
                    subst1(fromto, LF, 0);
                    subst1(fromto, '.', 0);
                    subst1(fromto, ' ', 0);
                    strncpy(at, b->at, CALLEN);
                    at[CALLEN] = 0;
                    subst1(at, '.', 0);
                    if (*at && *fromto && strcasecmp(fromto, at))
                    {
                      sprintf(rline, "%-6s>%-8s@%.22s~%-6s $%-12s @: %s",
                               b->herkunft, b->ziel, b->at, b->frombox,
                               b->bid, fromto);
                      log_entry(SWAPLOGNAME, rline);
                      b->boardlife_max = m.defswaplf;
                    }
                  }
                }
                else
                {
                  sprintf(rline,
                       "%-6s>%-8s@%.22s~%-6s $%-12s from: %s to: not found",
                       b->herkunft, b->ziel, b->at, b->frombox,
                       b->bid, fromto);
                  log_entry(SWAPLOGNAME, rline);
                  b->boardlife_max = m.defswaplf;
                }
              }
            }
          }
        }
      }
    }
    b->bytes += strlen(line);
    if (b->forwarding != fwd_none) timeout(m.fwdtimeout);
    else timeout(m.usrtimeout);
    if (line[strlen(line) - 1] == LF || attachabbruch == 1) // DH3MB
    {
      b->lines++;
      linestart = 1;
    }
    else linestart = 0;
  }
  while (! eof);
  if (eof != line)
  {
    b->lines++;
    b->bytes++;
    if (sendf) fputc(LF, sendf);
  }
  if (sendf) s_fclose(sendf);
  if (broken7p)
  {
    trace(report, "writetext", "bad7+ %s>%s $%s", b->herkunft,
                                                  b->ziel, b->bid);
    b->conttype = '6'; //its a broken 7+ file  (7-)
    if (b->forwarding == fwd_none)
    {
       putf("Sorry, this 7+ file is not correct!\n");
       invalid = 1;
    }
    if (b->mailtype == 'B'
        && (! m.savebroken || (m.savebroken == 1 && ! (b->opt & o_b))))
    {
      invalid = 1;
      put_rejectlog("bad 7+");
    }
  }
  if (invalid)
  {
    b->bytes = 0;
    b->lines = 0;
  }
  else
    if (mbhadrok(headeradress) == 1)
      get_mybbs(b->herkunft, headeradress, 2);
  return send_ack;
}

/*---------------------------------------------------------------------------*/

static void near putno (char *reason)
//*************************************************************************
//
//  Defers the message with "NO - ..."
//
//*************************************************************************
{
  lastfunc("putno");

  if (! strcmp(b->uplink, "Import")
#ifdef _FILEFWD
      &&  b->forwarding != fwd_file
#endif
     )
    trace(fatal, "putno", "bad import file, %s", reason);
  if (b->forwarding == fwd_none)
  {
    if (reason && *reason) putf("No, %s\n", reason);
  }
  else
  {
    if (reason)
    {
      strupr(reason);
      fwdput("NO - ", reason);
    }
    else fwdput("NO", "");
  }
}

/*---------------------------------------------------------------------------*/

static void near putrej (char *reason)
//*************************************************************************
//
//  Defers the message with "REJ - ..."
//
//*************************************************************************
{
  lastfunc("putrej");
  if (! strcmp(b->uplink, "Import")
#ifdef _FILEFWD
        &&  b->forwarding != fwd_file
#endif
     )
    trace(fatal, "putrej", "bad import file, %s", reason);
  if (reason && *reason)
  {
    strupr(reason);
    fwdput("REJ - ", reason);
  }
  else
    fwdput("REJ", "");
}

/*---------------------------------------------------------------------------*/

static void near putlater (char *reason)
//*************************************************************************
//
//  Tells the forward-partner to send the mail again later
//
//*************************************************************************
{
  lastfunc("putlater");

  if (! strcmp(b->uplink, "Import") || ! strcmp(b->uplink, "Mailsv"))
    trace(fatal, "putlater", "failed to import, %s", reason);
  if (reason && *reason)
  {
    strupr(reason);
    fwdput("LATER - ", reason);
  }
  else
    fwdput("LATER", "");
}

/*---------------------------------------------------------------------------*/

static void near putok (void)
//*************************************************************************
//
//  Accepts the message
//
//*************************************************************************
{
  if (b->forwarding != fwd_none) fwdput("OK", "");
}

/*---------------------------------------------------------------------------*/

void save_em (void)
//*************************************************************************
//
//  Wertet eine E (Erase) oder M (Mybbs) Meldung aus
//
//*************************************************************************
{
  lastfunc("save_em");
  char *p;

  if (! *b->bid || bidvorhanden(b->bid)) return;
  if (strlen(b->betreff) > 75) return;
  strupr(b->betreff);
#ifdef _FILEFWD
  if (b->forwarding == fwd_file)
  {
    if ((p = strstr(b->betreff, "/EX")) != NULL) *p = 0;
  }
#endif
  if (*b->ziel == 'M')
  {
    if (! (p = strchr(b->betreff, ' '))) return;
    time_t mybbszeit = atol(++p);
    bidrein (b->bid, "E/M");
    if ((ad_time() - mybbszeit) < MAXAGE)
    {
      *strchr(b->betreff, ' ') = 0;
#ifdef _BCMNET_FWD
      if (is_bcmnet_bid(b->bid, 1, b->herkunft) == 1)
        return; //ignore MYBBS correction
#endif
#ifdef _BCMNET_GW
      //Maybe users MyBBS is protected by the CB-BCMNET login concept?
      if (is_bcmnet_mybbs(b->herkunft, b->bid, b->betreff, mybbszeit, "?",
                           "?", 0) == 1) return;
#endif
      set_mybbs(b->herkunft, b->betreff, mybbszeit, b->bid, "?", 0, NULL);
    }
  }
  else if (*b->ziel == 'E')
         remeraser("THEBOX", b->bid, b->herkunft, b->betreff,
                             b->logincall, "?", 0, "?");
}

/*---------------------------------------------------------------------------*/

static int near diskfull (void)
//*************************************************************************
//
//  Platte voll
//
//*************************************************************************
{
  lastfunc("diskfull");
  int full = 0;

  if (b->usermail)
    full = (dfree(m.userpath, 0) < USERDISKFULL);
  else
    full = (dfree(m.infopath, 0) < INFODISKFULL);
  if (full)
  {
    if (! m.disable)
      trace(serious, "diskfull", "disabled, disk full");
    m.disable = 1;
    if (b->forwarding != fwd_none)
      fwdlog("disk is full", "", 'S');
    return TRUE;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------*/

void add_unknown (void)
//*************************************************************************
//
//  Creates an entry in "unknown.bcm"
//  Generates a message to the sender if appropriate
//
//*************************************************************************
{
  lastfunc("add_unknown");
  char sendcmd[CALLEN + CALLEN + DIRLEN + HADRESSLEN + BETREFFLEN + 11 + 1];
  char content[255];

  if (returnmailok())
  {
    sprintf(sendcmd, "SP %s < %s UNKNOWN:%.30s",
            b->herkunft, m.boxname, b->betreff);
    sendcmd[79] = 0;
    sprintf(content,
            "Your msg for %s @ %s $%s\ncould not be forwarded at %s.\n",
            b->ziel, b->at, b->bid, m.boxname);
    genmail(sendcmd, content);
  }
  snprintf(sendcmd, sizeof(sendcmd), "%6s: %-6s > %-8s @%-10s %.22s",
           b->logincall, b->herkunft, b->zielboard, b->at, b->betreff);
  log_entry(UNKNOWNNAME, sendcmd);
}

/*---------------------------------------------------------------------------*/

void generate_ack (char *to, char *from, char *subj)
//*************************************************************************
//
//  Creates an "ACK: ..."-message
//
//*************************************************************************
{
  lastfunc("generate_ack");
  char sendcmd[LINELEN+1];
  char content[LINELEN+1];

  if (returnmailok())
  {
    sprintf(sendcmd, "SA %s < %s ACK:%.30s", to, m.boxname, subj);
    sprintf(content, "%s @ %s acknowledged (%s)",
            from, m.boxadress, datestr(ad_time(), 12));
    genmail(sendcmd, content);
  }
}

/*---------------------------------------------------------------------------*/

int parse_rej (reject_t *rej, char *s)
//*************************************************************************
//
//  Parses reject.bcm
//  Rueckgabe:  0   = kein reject-Eintrag/Ungueltige Aktion
//              >1  = reject-Eintrag gefunden
//
//*************************************************************************
{
   lastfunc("parse_rej");
   int found = 0;
   if (! s || *s == ';' || ! *s || strlen(s) < 3)
     return 0;
   subst(s, ';',0); //remote comments
   subst(s, LF, 0); //remove any line-breaks
   subst(s, CR, 0);
   subst(s, CTRLZ, 0); //remove eof-marker of some editors
   rej->action = toupper(*s);
#ifdef __FLAT__
   if (! strchr("RGEFHLPO", rej->action))
   {
     trace(replog, "parse_rej", "invalid rej-action: %s", s);
     return 0;
   }
#else
   if (! strchr("RGFHLP", rej->action))
   {
     trace(replog, "parse_rej", "invalid rej-action: %s", s);
     return 0;
   }
#endif
   s++;
   while (*s == ' ') s++;
   while (*s)
   {
     waitfor(e_ticsfull);
     switch (*s)
     {
       case '<':                // Sender
         s = nextword(s + 1, rej->sender, LINELEN);
         if (*rej->sender) found |= 1;
         break;
       case '>':                // Destination (Board/Call)
         s = nextword(s + 1, rej->destin, LINELEN);
         if (*rej->destin) found |= 2;
         break;
       case '$':                // BID
         s = nextword(s + 1, rej->bid, LINELEN);
         if (*rej->bid) found |= 4;
         break;
       case '@':                // Address
         s = nextword(s + 1, rej->at, LINELEN);
         if (*rej->at) found |= 8;
         break;
       case '.':                // Type (Bulletin/Personal)
         rej->type = s[1];
         if (rej->type > ' ') found |= 16;
       default: while (*s && *s != ' ') s++;
     }
     while (*s == ' ') s++;
   }
   return (found);
}

/*---------------------------------------------------------------------------*/

char searchreject (void) //df3vi: neu geschrieben, return nun char statt int!
//*************************************************************************
//
//  Scans "reject.bcm" for a match with the message-data. Returns:
//   0 = Message must be accepted (and forwarded if necessary)
//   R = Message must be rejected
//   G = Message must be rejected, if entered locally without AX25-PW
//   E = Message must be rejected, if entered locally without AX25-PW/TTYPW
//   F = Message must be set on hold, if forwarded without Forward-PW
//   H = Message must be set on hold
//   L = Message must be set on hold, if entered locally
//   P = Message must be set on hold, if entered locally without AX25-PW
//   O = Message must be set on hold, if entered locally without AX25-PW/TTYPW
//
//  (DH3MB, based on searchbadname())
//  (DG9MHZ, added RegEx);
//
//*************************************************************************
{
  lastfunc("searchreject");
  FILE *f = s_fopen(REJECTNAME, "srt");
  char buf[LINELEN+1], s[LINELEN+1], *p;
  char holdc = 0;
  char match = 0;
  int imatch = 0; // inverse match
  char *pt; // pointer to current position in token
  CRegEx reg;
  reject_t rej_line;
  int found;

#ifdef _BCMNET_LOGIN
  if (! b->logintype && ! b->sysop &&
      (b->forwarding == fwd_none || b->forwarding == fwd_user))
    return 'Z';
#endif

  if (! f) return 0;
  while (! feof(f))
  {
    p = fgets(s, LINELEN, f);
    if (! (found = parse_rej(&rej_line, p))) continue;
    holdc = toupper(rej_line.action);
    switch (holdc)
    {
      case 'R': break;
      case 'H': break;
      case 'G': if ( (b->forwarding == fwd_none || b->forwarding == fwd_user)
                     && ! b->pwok ) break;
                else continue;
#ifdef __FLAT__
      case 'E': if ((b->forwarding == fwd_none || b->forwarding == fwd_user)
                    &&
                    ((! b->pwok) &&
                     (! ((! strcmp(b->uplink, "POP3")) ||
                         (! strcmp(b->uplink, "HTTP")) ||
                         (! strcmp(b->uplink, "SMTP")) ||
                         (! strcmp(b->uplink, "NNTP")) ||
                         (! strcmp(b->uplink, "TELNET"))
                        ) && (isamprnet(b->peerip) == 0)
                     )
                    )
                  ) break;
                else continue;
#endif
      case 'F': if (b->forwarding != fwd_none && ! b->pwok ) break;
                else continue;
      case 'P':
                if ((! *b->frombox || b->forwarding == fwd_user)
                    && (! b->pwok || ! strcmp(b->uplink, "Import"))) break;
                else continue;
#ifdef __FLAT__
      case 'O': if ((! *b->frombox || b->forwarding == fwd_user) &&
                    ((! b->pwok) &&
                     (! (((! strcmp(b->uplink, "POP3")) ||
                          (! strcmp(b->uplink, "HTTP")) ||
                          (! strcmp(b->uplink, "SMTP")) ||
                          (! strcmp(b->uplink, "NNTP")) ||
                          (! strcmp(b->uplink, "TELNET"))
                         ) && (isamprnet(b->peerip) == 0))
                         || (! strcmp(b->uplink, "Import"))
                     )
                    )
                  ) break;
                else continue;
#endif
      case 'L': if (! *b->frombox || b->forwarding == fwd_user) break;
                else continue;
      default:
        trace(replog, "searchreject", "action %c invalid", rej_line.action);
        holdc = 0;
        continue;
    }
    waitfor(e_ticsfull);
    if (found & 1)
    {
      if (rej_line.sender[0] == '!') //inverse matching
      {
        imatch = 1;
        pt = rej_line.sender + 1;
      }
      else
      {
        imatch = 0;
        pt = rej_line.sender + 0;
      }
      snprintf(buf, sizeof(buf), "%s\n", b->herkunft);
      if (pt[0] == '(') //OR
      {
        char worte[MAXPARSE] = {0};
        int anz, i = 0;
        pt = pt + 1;
        anz = parseline(pt,worte);
        for (i = 0; i < anz; i++)
        {
          match = reg.regex_match(pt+worte[i], buf);
          if (match==1) break;
        }
      }
      else
      {
        match = reg.regex_match(pt, buf);
      }
      if (imatch) match = !match;
      if (! match) continue;
    }
    if (found & 2)
    {
      if (rej_line.destin[0] == '!') //inverse matching
      {
        imatch = 1;
        pt = rej_line.destin + 1;
      }
      else
      {
        imatch = 0;
        pt = rej_line.destin + 0;
      }
      snprintf(buf, sizeof (buf), "%s\n", b->ziel);
      if (pt[0] == '(') //OR
      {
        char worte[MAXPARSE] = {0};
        int anz, i = 0;
        pt = pt + 1;
        anz = parseline(pt,worte);
        for (i = 0; i < anz; i++)
        {
          match = reg.regex_match(pt+worte[i], buf);
          if (match==1) break;
        }
      }
      else
      {
        match = reg.regex_match(pt, buf);
      }
      if (imatch) match = ! match;
      if (! match) continue;
    }
    if (found & 4)
    {
      if (rej_line.bid[0] == '!') //inverse matching
      {
        imatch = 1;
        pt = rej_line.bid + 1;
      }
      else
      {
        imatch = 0;
        pt = rej_line.bid + 0;
      }
      snprintf(buf, sizeof(buf), "%s\n", b->bid);
      if (pt[0] == '(') //OR
      {
        char worte[MAXPARSE] = {0};
        int anz, i = 0;
        pt = pt + 1;
        anz = parseline(pt, worte);
        for (i = 0; i < anz; i++)
        {
          match = reg.regex_match(pt+worte[i], buf);
          if (match == 1) break;
        }
      }
      else
      {
        match = reg.regex_match (pt, buf);
      }
      if (imatch) match = ! match;
      if (! match) continue;
    }
    if (found & 8)
    {
      if (rej_line.at[0] == '!') //inverse matching
      {
        imatch = 1;
        pt = rej_line.at + 1;
      }
      else
      {
        imatch = 0;
        pt = rej_line.at + 0;
      }
      snprintf(buf, sizeof(buf), "%s\n", b->at);
      subst1(buf, '.', 0);
      match = reg.regex_match(pt, buf);
      if (imatch) match = ! match;
        if (! match) continue;
    }
    if (found & 16)
    {
      match = (rej_line.type == b->mailtype);
      if (! match) continue;
    }
    if (match) break;
  }
  s_fclose(f);
  if (match) return holdc;
  else return 0;
}

/*---------------------------------------------------------------------------*/

#ifdef DF3VI_REJ_EDIT
void rejectlistedit (char * befbuf)
//*************************************************************************
//
//  Editor for reject.bcm
//
//*************************************************************************
{
  lastfunc("rejectlistedit");
  struct reject_t rej_line;
  char *s;
  char such[20];
  char p[LINELEN+1];
  int rej, found = 0;
  FILE *f, *g;

  strupr(befbuf);
  strcat(befbuf, " ");
  while (*befbuf == ' ') befbuf++;
  s = befbuf + 1;
  while (*s && *s != ' ') s++;
  switch (*befbuf)
  {
    case 'L': nexttoken(s, such, 19);
              if (! (f = s_fopen(REJECTNAME, "srt")))
              {
                trace(serious, "rejectlistedit", "fopen err");
                break;
              }
              if (*such) putf("search for %s:\n", such);
              while (! feof(f))
              {
                *p = 0;
                fgets(p, LINELEN, f);
                if (stristr(p, such) || ! *such) putf(p);
              }
              s_fclose(f);
              break;
    case 'A': while (*s == ' ') s++;
              rej = parse_rej(&rej_line, s);
              if (rej)
              {
                if ((f = s_fopen(REJECTNAME, "sat")) != NULL)
                {
                  char buf[LINELEN+1];
                  sprintf(buf, "%c", rej_line.action);
                  if (rej & 1) sprintf(buf + strlen(buf),
                                       " <%s", rej_line.sender);
                  if (rej & 2) sprintf(buf + strlen(buf),
                                       " >%s", rej_line.destin);
                  if (rej & 4) sprintf(buf + strlen(buf),
                                       " $%s", rej_line.bid);
                  if (rej & 8) sprintf(buf + strlen(buf),
                                       " @%s", rej_line.at);
                  if (rej & 16) sprintf(buf + strlen(buf),
                                        " .%c", rej_line.type);
                  putf("Comment: ");
                  getline(p, LINELEN, 1);
                  if (*p)
                  {
                    putf("\nAdding %s ;%s to %s\n", buf, p, REJECTNAME);
                    fputs(buf, f);
                    sprintf(buf, " ;%s\n", p);
                    fputs(buf, f);
                  }
                  else putf("No reject without reason!\n");
                  s_fclose(f);
                }
                else trace(serious, "rejectlistedit", "fopen err");
              }
              else
              {
                putf("No valid reject-line: %s\n", s);
                putf("Syntax: REJECTEDIT ADD <RGFHLP> [<sender] [>dest] [$bid] [@at] [.type]\n");
              }
              break;
    case 'D': s = nexttoken(s, such, 19);
              if (*such)
              {
                if (! (f = s_fopen(REJECTNAME, "srt")))
                {
                  trace(serious, "rejectlistedit", "fopen err");
                  break;
                }
                if (! (g = s_fopen(REJECTTMPNAME, "swt")))
                {
                  trace(serious, "rejectlistedit", "fopen err");
                  s_fclose(f);
                  break;
                }
                while (! feof(f))
                {
                  *p = 0;
                  fgets(p, LINELEN, f);
                  if (found || ! stristr(p, such))
                    fputs(p, g);
                  else
                  {
                    putf("Deleting %s\n", p);
                    found = 1;
                  }
                }
                s_fclose(f);
                s_fclose(g);
                if (found)
                {
                  xunlink(REJECTNAME);
                  xrename(REJECTTMPNAME, REJECTNAME);
                }
                else
                {
                  putf(ms(m_noentryfound));
                  xunlink(REJECTTMPNAME);
                }
              }
              else putf("Syntax: REJECTEDIT DEL <subject>\n");
              break;
    default: putf("Syntax: REJECTEDIT <cmd> [param]\n");
  }
}
#endif //DF3VI_REJ_EDIT

/*---------------------------------------------------------------------------*/

void convat (char *adr)
//*************************************************************************
//
// converts address of bulletins
//
//*************************************************************************
{
  lastfunc("convat");
  char buf[LINELEN+1];
  char *p;
  char oldat[HADRESSLEN+1];
  FILE *f = s_fopen(CONVATNAME, "srt");

  if (! f) return;
  while (! feof(f))
  {
    if (! (p = fgets(buf, LINELEN, f))) break;
    subst(p, ';', 0);
    cut_blank(p);
    while (*p == ' ') p++;
    if (strlen(p) < 2) continue;
    p = nexttoken(p, oldat, HADRESSLEN);
    while (*p == ' ') p++;
    if (stricmp(oldat, adr) || strlen(p) > HADRESSLEN) continue;
    nexttoken(p, adr, HADRESSLEN);
    strupr(adr);
    if (b->forwarding == fwd_none)
      putf("Converting: %s -> %s\n", oldat, adr);
  }
  s_fclose(f);
}

/*---------------------------------------------------------------------------*/

char testmaildirectory (void)
//*************************************************************************
//
//  Returns: 0 .. board not wanted
//           1 .. board accepted
//
//  19990816 DH3MB: Restructured the code; not using "dirok" anymore,
//                  directly using return; returning 0 instead of 2
//
//*************************************************************************
{
  lastfunc("testmaildirectory");
  unsigned char i;
  unsigned cbd;

  if (finddir(b->zielboard, b->sysop))
    return OK; // DH3MB: Board exists, so we will accept the mail
  for (i = 0; b->zielboard[i]; i++)
    if (! isalnum(b->zielboard[i])
        //&& ! ispunct (b->zielboard[i]))
        && b->zielboard[i] != '_' && b->zielboard[i] != '-')
      return NO; // DH3MB
  cbd = m.createboard;
  if (   (b->forwarding == fwd_none || b->forwarding == fwd_user)
      && (cbd > 1))
    cbd = 0;
  switch (cbd)
  {
  case 2: return NO; // Sysop doesn't want any TMP-Board
  case 1: // TMP-Board + Sub-Boards
    if (! finddir("TMP", b->sysop) && mkboard("/", "TMP", 0))
      return NO;
    else
    {
      if (! mkboard("TMP", b->zielboard, 0))
        return OK;
      else
        return NO;
    }
  case 0: // TMP-Board yes, but no Sub-Boards
  default:
    if (! finddir("TMP", b->sysop) && mkboard("/", "TMP", 0))
      return NO;
    else
    {
      strcpy(b->zielboard, "TMP");
      return OK;
    }
  }
}

/*---------------------------------------------------------------------------*/

#ifdef _BCMNET_FWD
int is_bcmnet_bid(char *bid, int cflag, char *call)
//*************************************************************************
//
// Checks the BID/MID if it is a valid CB-BCMNET BID/MID
//
// bid     the bid that should be checked
// cflag   0 : normal bid
//         1 : correction bid
// call    call of the user (optional, only for reports)
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "is_bcmnet_bid");
  lastfunc(name);

  if (   (strlen(bid) == 12)                   //12 chars long
      && (bid[1] >= 'D' && bid[1] <= 'Z')      //year between 2003..2026
      && (   (bid[2] >= '1' && bid[2] <= '9')  //month between 1..
          || (bid[2] >= 'A' && bid[2] <= 'C')) //               ..12
      && (   (cflag == 0 && bid[9] == '_')     //underscore
          || (cflag == 1 && bid[9] == '=')))   //equal sign
  {
    if (call && cflag == 1)
      trace(report, name, "correction MyBBS for %s ignored", call);
    return YES;
  }
  return NO;
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef _BCMNET_GW
int is_bcmnet_user(char *call)
//*************************************************************************
//
// Checks H-Route of users HomeBBS for BCMNETID
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "is_bcmnet_user");
  lastfunc(name);
  char mybbs[HADRESSLEN+1];
  int retwert = NO;

  strcpy(mybbs, "");
  if (get_mybbstime(call) > (ad_time() - USERPROTECT)) //protect this user
  {
    get_mybbs(call, mybbs, 0);
    expand_hadr(mybbs, 64);
    if (   strlen(mybbs) > strlen(BCMNETID) //MyBBS with H-Route
        && strcmp(mybbs + strlen(mybbs) - strlen(BCMNETID), BCMNETID) == 0)
      //BCMNETID is last item
      retwert = YES;
  }
  trace(report, name, "%s %s", call, retwert ? "yes" : "no");
  return retwert;
}

/*---------------------------------------------------------------------------*/

int is_bcmnet_mybbs (char *call, char *bid, char *bbs, long btime,
                     char *origin, char *w_name, short hops)
//*************************************************************************
//
// Checks if the mybbs is protected by the CB-BCMNET login concept and
// creates an correction mybbs entry if necessary.
//
// Return 1 if the MyBSS is protected.
//
//*************************************************************************
{
  lastfunc("is_bcmnet_mybbs");
  char *nbid;
  char mybbs[HADRESSLEN+1];
  char logincall[CALLEN+1];
  char sendcmd[LINELEN+1];
  char content[550];
  char w_hops[20];

  strcpy(mybbs, "");
  strcpy(w_hops, "");
  if (   is_bcmnet_user(call) == 1        //a CB-BCMNET user
      && is_bcmnet_bid(bid, 0, call) != 1 //but no CB-BCMNET BID
     )
  {
    get_mybbs(call, mybbs, 0);
    expand_hadr(mybbs, 64);
    //if the new mybbs is the same as the old one (mostly caused by WP
    //MyBBS echo due to lack of BID in the WP protocol), we ignore it, but
    //we do not create a correction mybbs. (db1ras)
    if (strcmp(mybbs, bbs) != 0) //no WP MyBBS echo
    {
      strcpy(logincall, b->logincall); //backup b->logincall
      strcpy(b->logincall, m.boxname); //use boxname as logincall
      nbid = newbid();
      nbid[9] = '=';
      set_mybbs(call, mybbs, 0, nbid, m.boxname, 0, NULL);
      strcpy(b->logincall, logincall); //restore b->logincall

      //if we got a valid mybbs but without a h-addr then we send a
      //correction mybbs to publish the h-addr, but we do not send a
      //warning message. (db1ras)
      if (strcmp(atcall(mybbs), bbs) != 0)
      {
        if (hops) sprintf(w_hops, " Hops: %i", hops);
        sprintf(content, "%s @ %s is protected by the CB-BCMNET.\n"
                         "\n"
                         "His MyBBS should be changed to %s.\n"
                         "Origin: %s ID: $%s Date: %s Name: %s%s\n"
                         "This has been prevented and corrected by %s\n"
                         "due to strong suspicion of call-abuse!\n",
                 call, mybbs, bbs, origin, bid, datestr (btime, 2), w_name,
                 w_hops, m.boxadress);
        //mail to sysop
        sprintf(sendcmd, "SP %s < %s %s: possible MyBBS-abuse\n",
                m.sysopcall, m.boxname, call);
        genmail(sendcmd, content);
        //mail to F @BCMNET
        sprintf(sendcmd, "S F @BCMNET < %s %s: possible MyBBS-abuse\n",
                m.boxname, call);
        genmail(sendcmd, content);
  #ifndef _BCMNET_DEBUG
        //mail to user
        sprintf(sendcmd, "SP %s < %s %s: possible MyBBS-abuse\n",
                call, m.boxname, call);
        genmail(sendcmd, content);
  #endif
      }
    }
    return YES;
  }
  return NO;
}
#endif

/*---------------------------------------------------------------------------*/

char wantmail (char *reason)
//*************************************************************************
//
//  Checks if we accept a mail, or not and returns reason why
//
//*************************************************************************
{
  lastfunc("wantmail");
  char mybbs[HADRESSLEN+1];
  char atl[HADRESSLEN+1];
  char usermail = (char) mbcallok(b->ziel);
  char retval;
  char holdc = searchreject();
  int cvtlife = 0;

  strcpy(mybbs, "");
  if (reason)
  {
    *reason = 0;
    //df3vi: BIDs als erstes testen
    if (bidvorhanden(b->bid))
    {
      if (reason) strcpy(reason, "BID already received");
      retval = FBB_NO;
      goto notwanted;
    }
    if (tmpbidvorhanden(b->bid))
    {
      if (reason) strcpy(reason, "already receiving BID");
      retval = FBB_LATER;
      goto notwanted;
    }
    // In User-/User-S&F-mode we only accept messages from the uplink call
    // war vorher in mbsend_parse abgefragt
    if (   strcmp(b->herkunft, b->logincall)
#ifdef INETMAILGATE
        && strcmp(m.internetmailgate, b->logincall)
#endif
        && (   b->forwarding == fwd_user
           || (b->forwarding == fwd_none && ! b->sysop)) )
    {
      if (reason) strcpy(reason, "only from login call");
      retval = FBB_REJECT;
      goto notwanted;
    }
    if (! *b->herkunft) //If no sender is given
    {
      if (reason) strcpy(reason, "no sender");
      retval = FBB_REJECT;
      goto notwanted;
    }
    // National Traffic Services are not supported
    if (b->mailtype == 'T')
    {
      if (reason) strcpy(reason, "NTS not supported");
      retval = FBB_REJECT;
      goto notwanted;
    }
    // Ack-mails are only accepted in forward-mode, otherwise a message
    // without routing-information would be created
    if (b->mailtype == 'A' && ! b->sysop && b->forwarding == fwd_none)
    {
      if (reason) strcpy(reason, "SA not allowed");
      retval = FBB_REJECT;
    goto notwanted;
    }
    // No destination specified
    if (! *b->ziel)
    {
      if (reason) strcpy(reason, "destination missing");
      retval = FBB_REJECT;
      goto notwanted;
    }
#ifdef _BCMNET_GW
    // CB-BCMNET user protection (b->bid, b->herkunft and b->ziel must be valid,
    // mybbs must be empty)
    if (   is_bcmnet_user(b->herkunft) == 1           //a CB-BCMNET user
        && is_bcmnet_bid(b->bid, 0, b->herkunft) != 1 //but no CB-BCMNET BID
        && ! (b->opt & o_z)                           //and no CB-BCMNET S&F
        && strcmp(b->ziel, "M") != 0                  //no MyBBS mail
        && strcmp(b->ziel, "E") != 0                  //no Erase mail
       )
    {
      if (reason) strcpy(reason, "possible call-abuse");
      retval = FBB_REJECT;
      char sendcmd[LINELEN+1];
      char content[400];
      char at[HADRESSLEN+3+1];
      char lt[3+2+1];
      char id[3+1];
      strcpy(lt, "");
      strcpy(at, "");
      if (*b->at) sprintf(at, "@ %s ", b->at);
      if (b->lifetime) sprintf(lt, "#%i ", b->lifetime);
      if (mbcallok(b->ziel)) strcpy(id, "MID");
      else strcpy(id, "BID");
      get_mybbs(b->herkunft, mybbs, 0);
      sprintf(content, "%s @ %s is protected by the CB-BCMNET.\n"
                        "\n"
                        "The mail to %s %s%sfrom %s\n"
                        "with %s $%s is most likely not from %s,\n"
                        "therefor %s has rejected this mail\n"
                        "due to strong suspicion of call-abuse!\n",
               b->herkunft, mybbs, b->ziel, at, lt, b->herkunft, id, b->bid,
               b->herkunft, m.boxadress);
      //mail to sysop
      sprintf(sendcmd, "SP %s < %s %s: possible call-abuse\n",
              m.sysopcall, m.boxname, b->herkunft);
      genmail(sendcmd, content);
      //mail to F @BCMNET
      sprintf(sendcmd, "S F @BCMNET < %s %s: possible call-abuse\n",
              m.boxname, b->herkunft);
      genmail(sendcmd, content);
    #ifndef _BCMNET_DEBUG
      //mail to destination
      sprintf(sendcmd, "S %s %s%s < %s %s: possible call-abuse\n",
              b->ziel, at, lt, m.boxname, b->herkunft);
      genmail(sendcmd, content);
      //mail to source
      sprintf(sendcmd, "SP %s < %s %s: possible call-abuse\n",
              b->herkunft, m.boxname, b->herkunft);
      genmail(sendcmd, content);
    #endif
      goto notwanted;
    }
#endif
    waitfor(e_ticsfull);
    // We won't accept messages destined to an unknown user,
    // if the sender doesn't specify an destination-BBS
    // but in userfwd?
    if (usermail)
    {
      get_mybbs(b->ziel, mybbs, 0);
      if (   b->forwarding == fwd_none
          && strcmp(b->uplink, "Import")
          && ! *b->at && ! *mybbs)
      {
        if (reason) strcpy(reason, "user unknown");
        retval = FBB_REJECT;
        goto notwanted;
      }
    }
#ifdef _GUEST
    // Guests aren't allowed to send except mails to the sysop.
    if ((! strcmp(b->herkunft, m.guestcall))
        && strcmp(b->ziel, m.sysopcall)
        && (strcmp(b->ziel, "SYSOP") || *b->at))
    {
      if (reason) strcpy(reason, "guest");
      retval = FBB_REJECT;
      goto notwanted;
    }
#endif
    // df3vi: User ohne PW duerfen nur LOKALES schreiben
    if (   m.userpw >= 2
        && ! (m.holdtime && strchr("HLPO", holdc)) // Wenn Holdtime
                                                   // und Hold, uebergehen
        && (b->forwarding == fwd_none || b->forwarding == fwd_user)
        && ! b->pwok && ! b->sysop && weiterleiten(1, b->ziel) != bleibtliegen)
    {
      if (reason) strcpy(reason, "only local messages");
      retval = FBB_REJECT;
      goto notwanted;
    }
    waitfor(e_ticsfull);
    // Did the user exceed his quota?
    if (b->forwarding != fwd_standard && b->mailtype == 'B' && checkquota())
    {
      if (reason) strcpy(reason, "user quota exceeded");
      retval = FBB_LATER_;
      goto notwanted;
    }
    // Is the disk full?
    if (diskfull())
    {
      if (reason) strcpy(reason, "disk is full");
      retval = FBB_LATER_;
      goto notwanted;
    }
    waitfor(e_ticsfull);
    // If the mail-files are being purged, no send is possible
    if ((sema_test("sendlock") || m.disable) && ! usermail)
    {
      if (   b->forwarding != fwd_none
          || strcmp(b->uplink, "Import")
          || strcmp(b->uplink, "Mailsv"))
        while (sema_test("sendlock") || m.disable) wdelay(2330);
      else
      {
        if (reason) strcpy(reason, "mails are being purged");
        retval = FBB_LATER_;
        goto notwanted;
      }
    }
    // df3vi: User ohne PW duerfen lokal nur an Sysop schreiben
    if (holdc == 'G'
        && (strcmp(b->ziel, m.sysopcall)
            && strcmp(b->ziel, "SYSOP") || *b->at)
        && strcmp(b->logincall, m.boxname) //fuer WP-Generierung
       )
     {
       if (reason) strcpy(reason, "only message to SYSOP without AX25PW");
       retval=FBB_REJECT;
       goto notwanted;
     }
#ifdef __FLAT__
    // df3vi: User ohne PW duerfen lokal nur an Sysop schreiben
    if (holdc == 'E'
        && (strcmp(b->ziel, m.sysopcall)
            && strcmp(b->ziel, "SYSOP") || *b->at)
        && strcmp(b->logincall, m.boxname) //fuer WP-Generierung
       )
     {
       if (reason) strcpy(reason, "only message to SYSOP without PW");
       retval=FBB_REJECT;
       goto notwanted;
     }
#endif
    // df3vi: Check, if the sysop wishes to reject the mail
    if (holdc == 'R')
    {
      if (reason)
        strcpy(reason, "not wanted");
               //nicht zu deutlich auf reject.bcm hinweisen!
      retval = FBB_REJECT;
      goto notwanted;
    }
    waitfor(e_ticsfull);
    // Usermails may not be addressed to a bulletin address
    // execpt personal mails to "SYSOP"
    if (b->mailtype != 'B' && *b->at && mbhadrok(b->at) != 1
        && strcmp(b->ziel, "SYSOP"))
    {
      if (reason) strcpy(reason, "invalid address");
      retval = FBB_REJECT;
      goto notwanted;
    }
    // We don't allow a bulletin to an user
    if (b->mailtype == 'B' && mbcallok(b->ziel))
    {
      if (reason) strcpy(reason, "bulletin to user");
      retval = FBB_REJECT;
      goto notwanted;
    }
    waitfor(e_ticsfull);
#ifndef _GUEST
    // Is the source call valid?
    if (! mbcallok(b->herkunft))
    {
      if (reason) strcpy(reason, "invalid source call");
      retval = FBB_REJECT;
      goto notwanted;
    }
#endif
    // If a mail is addressed to the local SYSOP-board, we will deliver
    // it directly to the sysop (the one specified in init.bcm)
    if (! strcmp(b->zielboard, "SYSOP"))
    {
      strcpy(atl, b->at);
      expand_hadr(atl, 3);
      if (! *atl || ! strcmp(atl, m.boxadress))
      {
        usermail = 1;
        strcpy(b->ziel, m.sysopcall);      //df3vi: beides beschreiben
        strcpy(b->zielboard, m.sysopcall);
        b->mailtype = 'P';
      }
    }
    waitfor(e_ticsfull);
    // We only accept mails to "one-letter-boards" from a
    // forward-partner and from sysops
    if (strlen(b->zielboard) == 1
        && ! b->sysop && b->forwarding != fwd_standard)
    {
      if (b->forwarding == fwd_none) putf(ms(m_boardnotpossible), b->ziel);
      else if (reason) strcpy(reason, "board not wanted");
      retval = FBB_REJECT;
      goto notwanted;
    }
    if (m.nounknownroute)
    {
      // wie DP-BOX, unknown gar nicht erst annehmen...
      // If there is no route for the message that we receive, we will defer it
      if (b->forwarding == fwd_standard
          && weiterleiten(1, b->ziel) == unbekannt && b->mailtype != 'B')
      {
        if (reason)
          strcpy(reason, "unknown @region");
        add_unknown();
        retval = FBB_REJECT;
        goto notwanted;
      }
    }
  }
  // convert local boards
  if (! usermail)
    convert(b->zielboard, &cvtlife);
  else
  {
    // only local users can be converted
    strcpy(atl, "");
    get_mybbs(b->zielboard, atl, 0);
    if (! strcmp(atl, m.boxadress))
      convert(b->zielboard, &cvtlife);
  }

  if (reason)
  {
    // if converted to "one-letter-board"
    if (strlen(b->zielboard) == 1
        && ! b->sysop && b->forwarding != fwd_standard)
    {
      if (b->forwarding == fwd_none) putf(ms(m_boardnotpossible), b->ziel);
      else if (reason) strcpy(reason, "board not wanted");
      retval = FBB_REJECT;
      goto notwanted;
    }
  }
/*
  // If the only route for the message leads back to the BBS, which the mail
  // is coming from, we will defer it
  if (b->forwarding == fwd_standard && strstr(b->destboxlist, b->logincall)
      && strlen(b->destboxlist) < 9  && b->mailtype != 'B')
  {
    if (reason) strcpy(reason, "loop");
    retval = FBB_REJECT;
    goto notwanted;
  }
*/

  if (! testmaildirectory())
  {
    if (reason) strcpy(reason, "board not wanted");
    retval = FBB_REJECT;
    goto notwanted;
  }
  waitfor(e_ticsfull);
  finddir(b->zielboard, b->sysop);
  if (xmkdir(b->boardpath))
  {
    trace(serious, "wantmail", "mkdir %s error", b->boardpath);
    if (reason) strcpy(reason, "invalid board name");
    retval = FBB_REJECT;
    goto notwanted;
  }
  if (cvtlife) b->boardlife_max = cvtlife;
  set_boardlife_max(0);
  if (   b->mailtype == 'B' && mbhadrok(b->at) == 2
      && (b->forwarding == fwd_none || b->forwarding == fwd_user))
    convat(b->at);
  return FBB_YES;

notwanted:
  // If we will reject the mail, we put a note into the reject log
  if (retval == FBB_REJECT)
  {
    // damit nur einmal im RLOG eingetragen
    if (*b->bid && ! bidvorhanden(b->bid)) bidrein(b->bid, b->ziel);
    if (reason) put_rejectlog(reason);
    else put_rejectlog("-");
  }
  // If the partner does not support extended reject-messages
  if (b->forwarding != fwd_none && ! (b->opt & o_r))
    switch (retval)
    {
    case FBB_LATER:
    case FBB_LATER_:
      if (b->opt & o_f) retval = FBB_LATER;
      else retval = FBB_NO;
      break;
    case FBB_REJECT:
    case FBB_ERROR: retval = FBB_NO; break;
    case FBB_HOLD:  retval = FBB_YES; break;
    }
  if (b->forwarding == fwd_none && retval != FBB_YES) retval = FBB_NO;
  return retval;
}

/*---------------------------------------------------------------------------*/

void set_boardlife_max (short int old_lt) //db1ras
//*************************************************************************
//
//  Set b->boardlife_max to the maximum allowed lifetime
//
//  old_lt   0 : don't use it
//  old_lt   1..999 : use this lifetime if b->lifetime is empty and don't
//                    give sysop more rights
//
//*************************************************************************
{
  if (b->lifetime
      && (b->lifetime < b->boardlife_max || (! old_lt && b->sysop)))
    b->boardlife_max = b->lifetime;
  else
    if (old_lt && old_lt < b->boardlife_max)
      b->boardlife_max = old_lt;
  if (b->boardlife_max < b->boardlife_min)
    b->boardlife_max = b->boardlife_min;
}

/*---------------------------------------------------------------------------*/

#ifdef MAILSERVER
void launch_mailserver (char *)
//*************************************************************************
//
//  Checks, if a mail destined to the BBS-call contains a command for the
//  mailing list server
//
//*************************************************************************
{
  mailserv ma;
  ma.examine_mail();
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef DF3VI_POCSAG
void launch_pocsagserver (char *)
//*************************************************************************
//
//  Checks, if a user-mail destined to the BBS-call contains the user-call
//  in the pocsag server list
//
//*************************************************************************
{
  pocsagserv poc;
  poc.examine_mail();
}
#endif

/*---------------------------------------------------------------------------*/

void savemail (void)
//*************************************************************************
//
//  Saves a mail to a file in the bcm mail-tree
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "savemail");
  lastfunc(name);
  FILE *sendf;
  FILE *mdatei = NULL;
  unsigned long nummer = 0L;
  char send_ack = 0;

  int fwdmode = weiterleiten(1, b->zielboard);
  make_mask(b->mailpath, b->boardfullname);
  strcpy(b->mask, b->mailpath);
  strcpy(b->mailfname, time2filename(0));
  if (! strstr(b->mailpath, "*.*"))
    trace(fatal, name, "mask %s", b->mailpath);
  strcpy(strstr(b->mailpath, "*.*"), b->mailfname);
  strlwr(b->mailpath);
  if ((sendf = s_fopen(b->mailpath, "lw+t")) != NULL)
  {
    s_fsetopt(sendf, 1);
    writeheader(sendf, 0);
    waitfor(e_ticsfull);
    send_ack = writetext(sendf); // sendf is closed within writetext()
    waitfor(e_ticsfull);
    writelines();
    waitfor(e_ticsfull);
    if (b->bytes && (stricmp(m.m_filter, "off") < 0))
    {
      char befehl[BLEN];
      if ((mdatei = s_fopen(m.m_filter, "srb")) != NULL)
      {
        sprintf(befehl, "%s %s", m.m_filter, b->mailpath);
        // DL9CU: Sonderzeichenkonvertierung
        while (strchr(befehl, '(')) *strchr(befehl, '(') = '[';
        while (strchr(befehl, ')')) *strchr(befehl, ')') = ']';
        while (strchr(befehl, '<')) *strchr(befehl, '<') = '[';
        while (strchr(befehl, '>')) *strchr(befehl, '>') = ']';
        while (strchr(befehl, '&')) *strchr(befehl, '&') = '+';
//      trace(replog, name, "m_filter <%s>", befehl);
        log_entry(MFILTERLOGNAME, befehl);
        int retwert = oshell(befehl, sh_noinput);
//      trace(report, "m_filter", "returnwert: %d", retwert);
        switch (retwert)
        {
          case 2 : b->fwdhold = 'M';
                   add_hold("m_filter", 0);
                   break;       //2: no fwd
          case 1 : b->bytes = 0;
                   break;       //1: delete
          default: break;       //0: no change
        }
        s_fclose(mdatei);
      }
      else
        trace(replog, name, "m_filter '%s' not found!", m.m_filter);
    }
    if (b->bytes)
    {
      if (fwdmode == unbekannt && *b->at) add_unknown();
      nummer = appenddirlist(1);
      waitfor(e_reschedule);
    }
    if (nummer)
    {
      add_fwdfile(b->logincall, get_fdelay(b->herkunft), 0);
      writemailflags();
      if (b->forwarding == fwd_none)
      {
        if (b->binstart)
        {
          if (b->bytes < 1000000L) putf("%ld Bytes, ", b->bytes);
          else putf("%ld kBytes, ", b->bytes >> 10);
        }
        if (stricmp(b->uplink, "SMTP") && stricmp(b->uplink, "NNTP"))
        {
          putf(ms(m_linesfor), b->lines, b->boardfullname);
          if (*b->at)
            putf(" @ %s", b->at);
          putf(" (%lu) ", nummer);
          putf(ms(m_stored));
        }
        waitfor(e_ticsfull);
      }
      int findex = finddir(b->zielboard, b->sysop);
      if (findex > 0)
        tree[findex - 1].newestmail = ad_time();
      waitfor(e_ticsfull);
      inc_mailgot(b->boardname);
      waitfor(e_ticsfull);
      inc_mailsent(b->herkunft);
      if (fwdmode == bleibtliegen && b->usermail) //df3vi: nur bei Ziel-BBS
      {
        sprintf(b->line, "%s %lu", b->herkunft, nummer);
        mbtalk("\001", b->zielboard, b->line);
        trigger_ufwd(b->zielboard);
      }
      b->sentfiles++;
      if (fwdmode == bleibtliegen && send_ack)
        generate_ack(b->herkunft, b->ziel, b->betreff);
      mk_perform(MK_MAILFROM, b->herkunft);
      mk_perform(MK_MAILTO, b->zielboard);
      scan_wp();
#ifdef MAILSERVER
      if (! strcmp(b->ziel, m.boxname) && m.maillistserv)
         fork(P_BACK | P_MAIL | P_CLON, 0, launch_mailserver, "MailServ");
#endif
#ifdef DF3VI_POCSAG
      if (m.pocsaglistserv && fwdmode == bleibtliegen && b->mailtype == 'P')
         fork(P_BACK | P_MAIL | P_CLON, 0, launch_pocsagserver, "Pocsag");
#endif

      if (fwdmode == bleibtliegen
          && ! strcmp(b->ziel, "T") && m.tellmode == 2)
         fork(P_BACK | P_MAIL | P_CLON, 0, tellresp, "Tellresp");
      if (fwdmode == bleibtliegen && ! strcmp(b->ziel, "PING"))
         fork(P_BACK | P_MAIL | P_CLON, 0, pingresp, "Pingresp");
    }
    else
    {
      xunlink(b->mailpath);
      if (b->forwarding == fwd_none)
        putf(ms(m_nomessagestored));
    }
  }
  else
    trace(fatal, name, "fopen %s errno=%d %s", b->mailpath,
                                               errno, strerror(errno));
}

/*---------------------------------------------------------------------------*/

void update_mail (void)
//*************************************************************************
//
// Updates Address and/or call of a mail received in forward
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "update_mail");
  lastfunc(name);
  // If user has a new call -> update  at, ziel
  char newcall[CALLEN+1];
  char sendcmd[LINELEN+1];
  char content[260];

  if (  (   (b->forwarding != fwd_none && homeadr(b->at))
         || b->forwarding == fwd_none)
      && b->mailtype == 'P' && get_newcall(b->ziel, newcall))
  {
    trace(report, name, "%s new call %s ($%s)", b->ziel, newcall, b->bid);
    //send a mail to sender to prevent missuse
    if (b->forwarding != fwd_none)
    {
      if (returnmailok())
      {
        sprintf(sendcmd, "SP %s < %s New call: %s -> %s",
                b->herkunft, m.boxname, b->ziel, newcall);
        sprintf(content,
                "The new callsign of %s is %s.\n"
                "Your msg $%s will be sent to %s.\n",
                b->ziel, newcall, b->bid, newcall);
        genmail(sendcmd, content);
      }
    }
    else
      putf("New call: %s -> %s\n", b->ziel, newcall);
    strcpy(b->ziel, newcall);
    strcpy(b->zielboard, newcall);
    *b->at = 0;
    // Message may go back..
    strcpy(b->bid, newbid());
  }
  // If we receive a mail addressed to us in S&F, we will throw
  // away the address, so that the message will be forwarded to the
  // user's MYBBS
  if (b->forwarding != fwd_none && homeadr(b->at))
  {
    *b->at = 0; //change bid if address is changed
    if (weiterleiten(0, b->ziel) == bekannt)
    {
      bidrein(b->bid, "DUMMY"); // alte bid abspeichern, sonst Endlosschleife
      strcpy(b->bid, newbid());
      trace(report, name, "new adr %s@%s $%s", b->ziel, b->at, b->bid);
    }
  }
  if (b->mailtype == 'P')
  {
    static user_t us;
    // away function
    if (mbcallok(b->ziel)
        && ! strstr(b->betreff, "is marked as away")
        && loaduser(b->ziel, &us, 1))
    {
      if (us.away && (us.awayendtime > ad_time() || us.awayendtime == 0))
      {
        sprintf(sendcmd, "SP %s < %s %s is marked as away",
                b->herkunft, m.boxname, b->ziel);
        sprintf(content,
                "Note: This is an automatic reply to your msg \"%s\".\n"
                "%s is currently marked as away at %s.\n\n%s\n",
                b->betreff, b->ziel, m.boxname, us.awaytext);
        genmail(sendcmd, content);
      }
      // notification function
      if (*us.notification)
      {
        sprintf(sendcmd, "SP %s < %s %s received new mail",
                us.notification, m.boxname, b->ziel);
        sprintf(content,
                "Note: This is an automatic message generated by %s,\n"
                "because %s has enabled mail notification to your callsign %s.\n"
                "\nThe title of %s's new mail is:\n%s\n",
                m.boxname, b->ziel, us.notification, b->ziel, b->betreff);
        genmail(sendcmd, content);
      }
    }
  }
  return;
}

/*---------------------------------------------------------------------------*/

int mbsend (char *befbuf, int changedir)
//*************************************************************************
//
//  SEND-procedure (used for normal box-logins and (user-)ascii-forwarding,
//  but NOT for fbb-type forwarding !!!
//
//  Returns: 0 = Not accepting mail
//           1 = Mail has been saved
//
//*************************************************************************
{
  lastfunc("mbsend");
  int fwdmode;
  char rejreason[40]; // TODO: Use snprintf when writing to this var

  waitfor(e_reschedule);
  mbsend_parse(befbuf, 1);
#ifdef INETMAILGATE
  if (strcasecmp(m.internetmailgate, "OFF") &&
      ! strcmp(b->logincall, m.internetmailgate))
  {
    *b->at = 0;
    safe_strcpy(b->herkunft, m.internetmailgate);
  }
#endif
  if (! b->usermail && b->forwarding == fwd_none)
  {
    rubrik_completition(b->ziel); //dk2ui
    if (! *b->ziel) return NO;
    strcpy(b->zielboard, b->ziel);
  }
  update_mail();
  waitfor(e_ticsfull);
  switch (wantmail(rejreason))
  {
  case FBB_REJECT:
    putrej(rejreason);
#ifdef _FILEFWD
    if (b->forwarding == fwd_file) writetext(NULL);
#endif
    return NO;
  case FBB_LATER:
    putlater(rejreason);
#ifdef _FILEFWD
    if (b->forwarding == fwd_file) writetext(NULL);
#endif
    return NO;
  case FBB_YES:
    if (! tmpbidvorhanden(b->bid))
    {
      tmpbidrein(b->bid, 0);
      putok();
      break;
    }
    // fall through
  case FBB_NO:
    putno(rejreason);
#ifdef _FILEFWD
    if (b->forwarding == fwd_file) writetext(NULL);
#endif
    return NO;
  }
  if ((b->forwarding == fwd_standard
#ifdef _FILEFWD
      || b->forwarding == fwd_file
#endif
      ) && (! strcmp("M", b->ziel) || ! strcmp("E", b->ziel)))
  {
    if (! *b->betreff) getline(b->betreff, BETREFFLEN, 1);
    strtok(b->line, "\n\r");
    //putok();
    if (! strchr(b->betreff, CTRLZ)
#ifdef _FILEFWD
        && (b->forwarding == fwd_file && ! strstr(b->betreff, "/EX"))
#endif
      )
      do
      {
        getline(b->line, BUFLEN - 1, 1);
        strtok(b->line, "\n\r");
      }
      while (! strchr(b->line, CTRLZ)
#ifdef _FILEFWD
             && (b->forwarding == fwd_file && strcmp(b->line, "/EX"))
#endif
           );
    save_em();
    return OK;
  }
  fwdmode = weiterleiten(1, b->zielboard);
  if (changedir) strcpy(b->prompt, b->boardfullname);
  if (b->forwarding != fwd_none)
  {
    char *bf1 = befbuf;
    getline(befbuf, LINELEN, 1);
    if (fpara()->options & o_e) putok(); // Good old Pavillion Cluster
                                         // needs that hack
                                         // (2nd OK after subject)
/*
    if (b->forwarding != fwd_none && (b->boxtyp == b_dxl))
    { if (strstr(befbuf, "0 ") == befbuf) bf1 += 2;
      if (strstr(befbuf, "#0 ") == befbuf) bf1 += 3;
    }
*/
    strcpy(b->betreff, bf1);
  }
  else
  {
    if ((fwdmode == bleibtliegen) && ! homeadr(b->at))
      putf(ms(m_messagenotforward));
    else if (fwdmode == unbekannt) putf(ms(m_forwardunknown));
    else if (fwdmode == bekannt || fwdmode == active_bekannt
#ifdef _AUTOFWD
             || fwdmode == auto_bekannt
#endif
    )
    {
      if (stricmp(b->uplink, "SMTP") && stricmp(b->uplink, "NNTP"))
      {
        putf(ms(m_address), b->at);
        putf(ms(m_isknown));
        if (strlen(b->destboxlist) > 37) putf(":\n");
        putf("%s\n", b->destboxlist);
      }
    }
    if (interactive() && ! b->lifetime
        && (   (m.ltquery == 1 && b->mailtype == 'B')
            || (m.ltquery == 2)))
    {
      do
      {
        putf("\nLifetime (1-999) #");
        getline(befbuf, LINELEN, 1);
        if (*befbuf == '#') befbuf++; // Ignoriere #-Zeichen
        //Bugfix: zuerst den Wert in b->lifetime setzen und erst dann die 0
        //abfragen, sonst steht in b->lifetime unter Umstaenden Muell. (db1ras)
        b->lifetime = atoi(befbuf);
        if (! strcmp(befbuf, "0"))
          break; // Explizit Null, keine falsche Eingabe
        if (b->lifetime > 0)
          set_boardlife_max(0); //nur positive Werte verwenden (db1ras)
      }
      while (b->lifetime < 1 || b->lifetime > 999);
    }
    if (! *b->betreff)
    {
#ifdef __FLAT__
      if (! stricmp(b->uplink, "HTTP"))
      {
        html_putf("Not possible in HTTP mode, use SEND in menue!\n");
        return NO;
      }
#endif
      do
      {
        putf(ms(m_title));
        getline(befbuf, LINELEN, 1);
        putv(LF);
        if (strchr(befbuf, CTRLZ) || strchr(befbuf, CTRLX)) return NO;
      }
      while (strlen(befbuf) < 2);
      strcpy(b->betreff, befbuf);
    }
    if (stricmp(b->uplink, "SMTP") && stricmp(b->uplink, "NNTP"))
    {
      putf(ms(m_textinput), b->ziel);
      if (*b->at)
        putf("@ %s ", b->at);
      putf(ms(m_textsend), get_name(b->ziel, 1)); //konvertiertes Board
                                                  //beruecksichtigen?
    }
  }
  waitfor(e_ticsfull);
  wantmail(NULL); // 27.11.05 dh8ymb hinzu wegen convert.bcm
  savemail();
  tmpbidraus(0);
  waitfor(e_ticsfull);
  return OK;
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__

int smtp_convert (char *infile)
//*************************************************************************
//
// DH6BB:
// Konvertiert eine empfangene SMTP-Mail, trennt die Anhaenge ab
// und wandelt diese ins BIN-Format.
// Returnwert ist die Anzahl der Import-Dateien (sofern >0),
// negative Werte sind Fehler.
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "smtp_convert");
  lastfunc(name);
  char from[LINELEN+1];
  char to[LINELEN+1];
  char date[LINELEN+1];
  char subject[LINELEN+1];
  char type[LINELEN+1];
  char transfer[LINELEN+1];
  char encoding[LINELEN+1];
  char boundary[LINELEN+1];
  char filename[LINELEN+1];
  char p[LINELEN+1];
  char importfile[1024];
  char bin_filename[1024];
  char *pointer;
  char work_pfad[200];
  bool Sheader_end=false;
  bool Ende=false;
  bool merker=false;
  bool return_receipt=false;
  int Teil=1;
  int ch;
  FILE *in, *out, *import, *bin_file;
  *from=0;
  *to=0;
  *date=0;
  *subject=0;
  *type=0;
  *boundary=0;
  *encoding=0;

  // Versuch die Original-Maildatei zu oeffnen
  if (! (in = s_fopen(infile, "sr+t")))
  {
    trace(serious, name, "fopen error");
    return -1;
  }
  sprintf(work_pfad, "%s", infile);
  subst1(work_pfad + 1, '.', 0);
  strlwr(work_pfad);
  mkdir(work_pfad);
  // Und jetzt eine temporaere Datei, wo alles wieder reingeschrieben wird
#ifdef __LINUX__
  sprintf(importfile, "%s/imp_0.imp", work_pfad);
#else
  sprintf(importfile, "%s\\imp_0.imp", work_pfad);
#endif
  if (! (out = s_fopen(importfile, "sw+t")))
  {
    trace(serious, name, "fopen error");
    s_fclose(in);
    return -2;
  }
  // Der SMTP-Header endet mit einer Leerzeile
  // Den SMTP-Header verwerfen wir. Wir speichern nur seine Werte.
  while (! Sheader_end && fgets(p, LINELEN, in))
  {
    if (! strncmp(p, "From:", 5))
      strcpy(from, p);
    if (! strncmp(p, "To:", 3))
      strcpy(to, p);
    if (! strncmp(p, "Subject:", 8))
      strcpy(subject, p);
    if (! strncmp(p, "Date:", 5))
      strcpy(date, p);
    if (! strncmp(p, "Content-Type:", 13))
      strcpy(type, p);
    if (! strncmp(p, "Return-Receipt-To:", 18))
      return_receipt=true;
    if (! strncmp(p, "Content-Transfer-Encoding:", 26))
      strcpy(transfer, p);
    if ((pointer=strstr(p, "boundary=")))
    {
      pointer = p+(pointer-p)+10;
      strcpy(boundary, pointer);
      boundary[strlen(boundary)-2] = 0;
    }
    if (! Sheader_end && *p == LF && *subject)
      Sheader_end = true; // Wir sind durch mit dem SMTP-Header
//trace(serious, "Header", p);
  }
//trace(serious, "Boundary", boundary);
//trace(serious, "Boundary", "%d", strlen(boundary));
//        trace(serious, "Type", type);
  if (! Sheader_end)
  {
    s_fclose(in);
    s_fclose(out);
    trace(serious, name, "No SMTP header found");
    xunlink(importfile);
    return -3;
  }
  filename[0] = 0;
  if (boundary[0] != 0)
  {
    while (fgets(p, LINELEN, in))
    {
      if (strstr(p, boundary) && p[0] != ' ')
        break;
    }
    while (fgets(p, LINELEN, in) && *p != LF)
    {
      if ((pointer = strstr(p, "name="))) // Filename ermitteln
      {
        pointer = p+(pointer-p)+6;
        strcpy(filename, pointer);
        filename[strlen(filename)-2] = 0;
      }
      if (! strncasecmp(p, "Content-Transfer-Encoding:", 26))
        strcpy(encoding, p);
    }
  }
  // Nur Text, ohne Anhang (der kommt noch)
  if (filename[0] == 0)
  {
    if (strstr(transfer, "base64")) // base64
    {
      fprintf(out, "\n");
      if (! (mime2bin(in, out)))
      {
        trace(serious, name, "mime2bin: convert error");
        s_fclose(in);
        s_fclose(out);
        return -22;
      }
    }
    else
    {
      if (strstr(transfer, "quoted-printable")
          || strstr(encoding, "quoted-printable")) // quoted-printable
      {
        fprintf(out, "\n");
        if (! (quoted2bin(in, out, boundary)))
        {
          trace(serious, name, "quoted2bin: convert error");
          s_fclose(in);
          s_fclose(out);
          return -21;
        }
      }
      else
      {
        fprintf(out, "%s", p);
        while (fgets(p, LINELEN, in))
        {
          if (boundary[0] != 0 && strstr(p, boundary))
          // hier beginnt der Anhang!
            break;
          else
            fprintf(out, "%s", p);
        }
      }
    }
  }
  while (fgets(p, LINELEN, in) && *p != LF)
  {
    if ((pointer = strstr(p, "name="))) // Filename ermitteln
    {
      pointer = p+(pointer-p)+6;
      strcpy(filename, pointer);
      filename[strlen(filename)-2] = 0;
    }
    if (! strncasecmp(p, "Content-Transfer-Encoding:", 26))
      strcpy(encoding, p);
  }
  if (filename[0] == 0)
    Ende = true;
   // Anhang (es kommen jetzt nur noch Anhaenge!)
  while (! Ende)
  {
#ifdef __LINUX__
    sprintf(bin_filename, "%s/%s", work_pfad, filename);
#else
    sprintf(bin_filename, "%s\\%s", work_pfad, filename);
#endif
    strlwr(bin_filename);
    if (! (bin_file = s_fopen(bin_filename, "sw+b")))
    {
      s_fclose(in);
      s_fclose(out);
      trace(serious, "mime2bin", "fopen error");
      return -4;
    }
    encoding[strlen(encoding)-1] = 0;
//trace(report, "Encoding", encoding);
    if (strstr(encoding, "base64")) // base64
    {
      if (! (mime2bin(in, bin_file)))
      {
        s_fclose(in);
        s_fclose(out);
        s_fclose(bin_file);
        trace(serious, "mime2bin", "error while decoding");
        return -5;
      }
    }
    else
    {
      if (strstr(encoding, "quoted-printable")) // quoted-printable
      {
        if (! (quoted2bin(in, bin_file, boundary)))
        {
          s_fclose(in);
          s_fclose(out);
          s_fclose(bin_file);
          trace(serious, "quoted2bin", "error while decoding");
          return -10;
        }
      }
      else // kein base64 oder quoted-printable. Also wandeln wir nicht,
      {    // setzten aber trotzdem spaeter einen BIN-Header drueber.
        while (fgets(p, LINELEN, in))
        {
          if (strstr(p, boundary))
            break;
          else
            fprintf(bin_file, "%s", p);
        }
      }
    }
    s_fclose(bin_file);
    // Und jetzt eine temporaere Datei, wo alles wieder
    // reingeschrieben wird
#ifdef __LINUX__
    sprintf(importfile, "%s/imp_%d.imp", work_pfad, Teil);
#else
    sprintf(importfile, "%s\\imp_%d.imp", work_pfad, Teil);
#endif
    strlwr(importfile);
    if (! (import = s_fopen (importfile, "sw+t")))
    {
      trace(serious, name, "fopen error");
      s_fclose(in);
      s_fclose(out);
      return -3;
    }
    fprintf(import, "\n"); // Return vor #BIN# sieht besser aus
    //den BIN-Header berechnen
    fprintf(import, "%s", create_bin_header(bin_filename));
    s_fclose(import);
    if (! (bin_file = s_fopen(bin_filename, "sr+b")))
    {
      s_fclose(in);
      s_fclose(out);
      trace(serious, "mime2bin", "fopen error");
      return -4;
    }
    if (! (import = s_fopen(importfile, "sa+b")))
    {
      trace(serious, name, "fopen error");
      s_fclose(in);
      s_fclose(out);
      s_fclose(bin_file);
      return -6;
    }
    fseek(import, 0L, SEEK_END);
    rewind(bin_file);
    ch = fgetc(bin_file);
    while(! feof(bin_file))
    {
      fputc(ch, import);
      ch=fgetc(bin_file);
    }
    s_fclose(bin_file);
    xunlink(bin_filename); // Wir nutzen nur noch das imp-File
    s_fclose(import);
    if (! (import = s_fopen(importfile, "sa+t")))
    {
      trace(serious, "smtp_convert", "fopen error");
      s_fclose(in);
      s_fclose(out);
      return -7;
    }
    Teil++;
    fseek(import, 0L, SEEK_END);
    fprintf(import, "\nend\n");
    s_fclose(import);
    filename[0] = 0; // und zuruecksetzen.
    encoding[0] = 0;
    merker = false;
    while (fgets(p, LINELEN, in))
    {
      if (*p == LF && merker) break;
      if ((pointer = strstr(p, "name="))) // Filename ermitteln
      {
        pointer = p+(pointer-p)+6;
        strcpy(filename, pointer);
        filename[strlen(filename)-2] = 0;
        merker = true;
      }
      if (! strncasecmp(p, "Content-Transfer-Encoding:", 26))
      {
        strcpy(encoding, p);
        merker = true;
      }
    }
    if (filename[0]==0)
      Ende = true;
  } // Ende Anhang
  if (Teil==2)
  {
    s_fclose(out);
#ifdef __LINUX__
    sprintf(importfile, "%s/imp_0.imp", work_pfad);
#else
    sprintf(importfile, "%s\\imp_0.imp", work_pfad);
#endif
    strlwr(importfile);
    if (! (out = s_fopen(importfile, "sa+b")))
    {
      trace(serious, name, "fopen error");
      s_fclose(in);
      return -8;
    }
    fseek(out, 0L, SEEK_END);
    if (return_receipt) fprintf(out, "\n/ACK\n");
#ifdef __LINUX__
    sprintf(importfile, "%s/imp_1.imp", work_pfad);
#else
    sprintf(importfile, "%s\\imp_1.imp", work_pfad);
#endif
    strlwr(importfile);
    if (! (bin_file = s_fopen(importfile, "sr+b")))
    {
      s_fclose(in);
      s_fclose(out);
      trace(serious, "mime2bin", "fopen error");
      return -9;
    }
    ch = fgetc(bin_file);
    while (! feof(bin_file))
    {
      fputc(ch,out);
      ch = fgetc(bin_file);
    }
    s_fclose(bin_file);
    xunlink(importfile);
  }
  else
  {
    if (Teil > 2)
    {
      fprintf(out, "\nNote:\n");
      fprintf(out,
        "Binary parts of this mail are following in %d separated mails!\n",
              Teil-1);
    }
    if (return_receipt) fprintf(out, "\n/ACK");
    fprintf(out, "\nnnnn\nend\n");
  }
  s_fclose(in);
  s_fclose(out);
  trace(report, name, "Detected parts: %d", Teil-1);
  return (Teil-1);
}

/*---------------------------------------------------------------------------*/

char *create_bin_header (char *file)
//*************************************************************************
//
// Fuer File wird der BIN-Header berechnet und zurueckgegeben.
// Achtung, ein
// "printf(%s %s, create_bin_header(file), create_bin_header(file));"
// wird nicht gehen!!
//
//*************************************************************************
{
  lastfunc("create_bin_header");
  static char bin_line[LINELEN+1];
  crcthp my_crc;
  FILE *inputfile;
  char *adrp;
  char attachname[LINELEN+1];

#ifdef __LINUX__
  if ((adrp = strrchr(file, '/')) != NULL) // Nur den Filenamen ohne Pfad
#else
  if ((adrp = strrchr(file, '\\')) != NULL)
#endif
    strncpy(attachname, ++adrp, LINELEN);
  else
    strncpy(attachname, file, LINELEN);

  attachname[LINELEN] = 0;
  my_crc.readfile(file, 0L);
  if ((inputfile = s_fopen(file, "lrb")) != NULL)
  {
    fseek(inputfile, 0L, SEEK_END);
    unsigned long binsize = ftell(inputfile);
    rewind(inputfile);
    sprintf(bin_line, "#BIN#%05lu#|%05u#$%08lX#%s\n", binsize,
            my_crc.result, getdostime(file_isreg(file)), attachname);
    s_fclose(inputfile);
    return bin_line;
  }
  trace(serious, "bin_header", "error opening File");
  return 0;
}

/*---------------------------------------------------------------------------*/

bool mime2bin (FILE *infile, FILE *bin_file)
//*************************************************************************
//
// Ein MIME-Codierter Anhang wird in BIN gewandelt
//
//*************************************************************************
{
  lastfunc("mime2bin");
  #define char64(c) (((c)<0 || (c)>127) ? -1 :index_64[(c)])
  int index_64[128] =
  {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
  };
  int c1, c2, c3, c4;
  bool DataDone = false;

  while ((c1 = getc(infile)) !=feof(infile))
  {
    if (DataDone)
      break;
    if (c1=='\n')
      continue;
    do
    {
      c2 = getc(infile);
    } while (c2 != EOF && isspace(c2));

    do
    {
      c3 = getc(infile);
    } while (c3 != EOF && isspace(c3));

    do
    {
      c4 = getc(infile);
    } while (c4 != EOF && isspace(c4));

    if (c2 == EOF || c3 == EOF || c4 == EOF)
    {
      trace(serious, "mime2bin",
                     "Warning: base64 decoder saw premature EOF!");
      s_fclose(bin_file);
      return false;
    }
    if (c1 == '=' || c2 == '=')
    {
      DataDone=true;
      continue;
    }
    if (c1 == '-' && c2 == '-' && c3 == '-' && c4 == '-')
    {
      DataDone=true;
      return true;
    }
    c1 = char64(c1);
    c2 = char64(c2);
    fprintf(bin_file, "%c", ((c1<<2) | ((c2&0x30)>>4)));
    if (c3 == '=')
    {
      DataDone = true;
    }
    else
    {
      c3 = char64(c3);
      fprintf(bin_file, "%c", (((c2&0XF) << 4) | ((c3&0x3C) >> 2)));
      if (c4 == '=')
      {
        DataDone = true;
      }
      else
      {
        c4 = char64(c4);
        fprintf(bin_file, "%c", (((c3&0x03) <<6) | c4));
      }
    }
  }
  return true;
}

/*---------------------------------------------------------------------------*/

bool quoted2bin (FILE *infile, FILE *bin_file, char boundary[LINELEN+1])
//*************************************************************************
//
// Eine QUOTED-PRINTABLE codierte Datei wird in Plain Text gewandelt
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "quoted2bin");
  lastfunc(name);
  #define hexval(c) Index_hex[(unsigned int)(c)]
  static int Index_hex[128] =
  {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
     0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
  };
  unsigned char c1, ch1, ch2;
  unsigned int pos;
  char line[1026];
  char inlinie[1026];
  int my_pos=0;

//trace(report, "Quoted2Bin", "Quoted2Bin");
  while (fgets(inlinie, 1024, infile))
  {
    if (boundary[0] != 0 && strstr(inlinie, boundary))
      return true;
    pos = 0;
    my_pos = 0;
    line[0] = 0;
    while(pos < (strlen(inlinie)))
    {
      c1 = (inlinie[pos] & 0xff);
      if (c1 == '=')
      {
        ch1=(inlinie[++pos] & 0xff);
        ch2=(ch1) ? ((inlinie[++pos]) & 0xff) : 0;
        if (! ch1 || ! ch2)
        {
         fprintf(bin_file, "%s", line);
         my_pos = 0;
         line[my_pos] = 0;
         continue;
        }
        c1 = hexval(ch1) << 4;
        c1 |= hexval(ch2);
        if (c1 <= 0)
        {
           pos++;
           continue;
        }
        line[my_pos] = c1;
        pos++;
        my_pos++;
        line[my_pos] = 0;
        continue;
      }
      line[my_pos] = c1;
      my_pos++;
      pos++;
      line[my_pos] = 0;
    }
    if (line[0] != 0)
    {
      line[my_pos-1] = 0;
      fprintf(bin_file, "%s\r\n", line);
    }
    inlinie[0] = 0;
  }
  return true;
}

#endif

/*---------------------------------------------------------------------------*/

void sysop_sysinfomail (void)
//*************************************************************************
//
// Erinnert den Sysop ueber Missstaende der Mailbox-Konfiguration einmal
// pro Monat
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "sysop_sysinfomail");
  lastfunc(name);
  char impdatei[30];
  char outdatei[30];
  FILE *f;
  FILE *g;

  sprintf(impdatei, TEMPPATH "/%s", time2filename(0));
  f = s_fopen(impdatei, "sw+b");
  if (! f)
  {
    trace(serious, "sysop_sysinfomail", "can't open %s", impdatei);
    return;
  }
  fprintf(f, "SP %s < %s Monthly sysinfo from %s\n",
                  m.sysopcall, m.boxname, m.boxname);
  fprintf(f, "Monthly sysinfo from %s\n", m.boxname);
  fprintf(f, "===========================\n");
  fprintf(f, "This mail is generated automatically once a month and should inform\n");
  fprintf(f, "you about not forwarded usermails and unknown target bbs.\n");
  fprintf(f, "Use \"f [-h] <call> 1- @ BBS\" to put held usermails in forward again\n");
  fprintf(f, "and check your fwd.bcm for correct forward settings! A correct fwd.bcm\n");
  fprintf(f, "file will avoid confusing users because mails are delivered in expected\n");
  fprintf(f, "time and of course will reduce the next sysinfo mail in lenght! ;-)\n\n");
  fprintf(f, "\nNot forwarded usermails \"d -n u m\":\n");
  fprintf(f, "------------------------------------------------------------------------------\n");
  s_fclose(f);
  sprintf(outdatei, TEMPPATH "/%s.imp", time2filename(0));
  g = s_fopen(outdatei, "sw+b");
  if (! g)
  {
    trace(serious, "sysop_sysinfomail", "can't open %s", outdatei);
    return;
  }
  fprintf(g, "exp -a %s d -n u m\n", impdatei);
  s_fclose(g);
  fork(P_BACK | P_MAIL, 0, mbimport, outdatei);
  while (sema_access(impdatei) || sema_access(outdatei)) wdelay(107);
  xunlink(outdatei);
  f = s_fopen(impdatei, "sa+b");
  fprintf(f, "\nUnknown target bbs \"p -sfn\":\n");
  fprintf(f, "------------------------------------------------------------------------------\n");
  s_fclose(f);
  sprintf(outdatei, TEMPPATH "/%s.imp", time2filename(0));
  g = s_fopen(outdatei, "sw+b");
  if (! g)
  {
    trace(serious, "sysop_sysinfomail", "can't open %s", outdatei);
    return;
  }
  fprintf(g, "exp -a %s p -sfn\n", impdatei);
  s_fclose(g);
  fork(P_BACK | P_MAIL, 0, mbimport, outdatei);
  while (sema_access(impdatei) || sema_access(outdatei)) wdelay(107);
  xunlink(outdatei);
  f = s_fopen(impdatei, "sa+b");
  fprintf(f, "\nnnnn\nimpdel\n");
  s_fclose(f);
  fork(P_BACK | P_MAIL, 0, mbimport, impdatei); // Ausgabe importieren
}

