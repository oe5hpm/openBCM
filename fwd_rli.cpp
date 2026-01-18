/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------
  Forward-Ein- und Ausgabe fuer W0RLI-Forwarding
  ----------------------------------------------


  Copyright (C)       Florian Radlherr et al
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980109 OE3DZW Added debug output isforwardpartner()
//                Removed check for D-Box (bin-fwd)
//                Added Call & BID in Errmsg "bin-mail could not be fwded"
//                Added seperator for "d p"
//19980115 OE3DZW reduced priority of err-msges in delfwdlistentry
//                Text "Info-mails" in "st f"
//                Added comment
//19980118 OE3DZW Removed REJ from trace (for s52d)
//19880118 OE3DZW sec in timestr
//19980120 OE3DZW Removed Space in st f, added fix 80 Bytes per
//                Mail (incl. E/M),
//19980124 OE3DZW fwdprop max. 127
//                Added Fwd-Style to Fwd-Log
//                Remove trace rx-sid, removed boxtypes
//                Fixed sidrtt
//                SID may be 119 chars long (was 79 before) - for Wingt-pw
//                removed support for -S-flag in forwarding
//19980126 OE3DZW fixed fwd-machine (went from FBB into ASCII-Mode)
//19980128 OE3DZW Twice sid in fwd log, fixed
//                added fbb-states to st-f command
//19980205 OE3DZW better alignment at st f
//                fixed sid-rtt
//19980211 OE3DZW fixed lastconok, was not set
//                added number of incoming/outgoing connects to st f
//                removed err-msg in fpara()
//19980215 OE3DZW alignment st f
//19980216 OE3DZW removed old entries in fwd-triggerfile
//                checked fwd-trigger before starting fwd for erased mail
//19980223 OE3DZW fwdget to 78 (was 119) due seg fault, will not accept wingt-pw
//19980225 OE3DZW fixed above bug, wingt-fwd should be ok now
//19980301 OE3DZW fixed fwdlog
//                removed debug output in killfwded
//                removed  CTRLZ from log
//                chkforward() entfernt alte eintraege, keine leeren vorschlaege im fbb-fwd
//19980302 OE3DZW added check for logincall when init fwd-list
//                added return-mail to hold-function
//                only mails type P/A are set to hold, not bulletins
//19980304 OE3DZW fixed bug, fwd bulletins were erased
//19980307 OE3DZW will not erase but hold if personal, also when o_r
//19980309 OE3DZW initfwdlist - output only when logincall
//                add_hold with reason
//19980311 DG9MHZ changes for Linux-Kernel AX25
//19980311 OE3DZW removed unused variables
//19980318 OE3DZW more robust against invalid entries in fwd-triggerfile
//                fixed bug in killfwded, added some debug-output
//19980329 OE3DZW E/M-lines will be put into the fwd-log (ascii-fwd)
//19980404 OE3DZW hold-msg only in fwd-standard,not in user-fwd
//19980404 hrx    support tty-fwd
//19980408 OE3DZW fwd should be started when there is a mail to send
//19980418 DH3MB  added space for diebox-pwd
//19980418 OE3DZW fwdlog shows taskid (was tnc-number)
//19980420 OE3DZW removed dashes after protests of flori,deti,chris an me
//                Total only if at least on mail in queue
//19980421 OE3DZW no UNKNOWN mail for CP-Mails
//19980423 OE3DZW hold due to loop only if not held before
//                included returnmailok()
//19980505 OE3DZW used rm_crlf instead of x[strlen(x)-1]=0..
//19980524 OE3DZW removed Fwd-Option -o in fwd.bcm
//19980609 OE3DZW killfwd will check for 0x00..0x00
//                usermails will be erased when replied with "no" in userfwd
//                sfhold - added parameter returnmail
//19980610 OE3DZW sfhold - not for  "sysop" mails of type "p"
//19980614 OE3DZW killfwded will also remove entries longer 50 chars
//19980615 OE3DZW new fwdproc_reply - common for ascii and fbb-fwding
//19980830 OE3DZW text fwdreply no changed to "already rcvd", was
//                not clear
//19980914 OE3DZW mails not twice in one proposal
//19980916 OE3DZW fwdlog will show taskid even on logout
//19981013 OE3DZW binary personal messages will be sent to fbb's
//19981015 OE3DZW users may totally disable active userforwarding
//19990201 OE3DZW commented out conat()
//19990211 OE3DZW added "W" to SID, added check for D2..D4 and B2..B4 in SID
//19990214 OE3DZW when fwd-times are incorrect, standard will be used
//19990215 OE3DZW generating sample fwd.bcm
//19990427 OE3DZW replaced E/M by WP in st f
//19990713 DH3MB  Created file
//200002.. DF3VI  Set short lifetime for not-wanted usermails
//20000818 DK2UI  changes in fwdsendfile()
//20121226 DG4MFN changes to make the Linux GCC 4.x.x Compilers happy :-)

#include "baycom.h"

/*---------------------------------------------------------------------------*/

/*typedef enum fwdstate_t*/ /*DG4MFN*/
enum fwdstate_t
{ fwd_send,
  fwd_delay,
  fwd_receive,
  fwd_prompt,
  fwd_done,
  fwd_end
};

/*---------------------------------------------------------------------------*/

/*typedef enum fwdput_t*/ /*DG4MFN*/
enum fwdput_t
{ fwdput_files_sent,
  fwdput_no_files,
  fwdput_double,
  fwdput_newmail,
  fwdput_error
};

/*---------------------------------------------------------------------------*/

/*typedef enum fwdsend_t*/ /*DG4MFN*/
enum fwdsend_t
{ fwdsend_ok,
  fwdsend_no,
  fwdsend_file_error,
  fwdsend_transfer_error,
  fwdsend_repeat_file,
  fwdsend_rej
};

/*---------------------------------------------------------------------------*/

/*typedef enum fwdcmd_t*/ /*DG4MFN*/
enum fwdcmd_t
{ fwdcmd_ok,
  fwdcmd_reverse,
  fwdcmd_end,
  fwdcmd_error
};

/*---------------------------------------------------------------------------*/

#ifndef _FILEFWD
static
#endif
       int fwdcmd (void);
static int fwdputfiles (void);
static int sendonemail (char *);

/*---------------------------------------------------------------------------*/

int chkforward (char *call)
//*************************************************************************
//
//  Gibt zurueck, ob zu <call> etwas zum Forwarding ansteht
//
//*************************************************************************
{
  lastfunc("chkforward");
  char fname[FNAMELEN+1];
  static char types[] = "uim";
  unsigned int i;
  if (b->forwarding == fwd_standard)
  {
    for (i = 0; i < 3; i++)
    {
      if (isforwardtime(call, i > 0)) // fwdtrigger
      {
        sprintf(fname, FWDPATH "/%c_%s.bcm", types[i], call);
        strlwr(fname);
        killforwarded(fname);
        if (file_isreg(fname)) return OK;
      }
    }
  }
  else
    return scan_userfile(call, 1);
  return NO;
}

/*---------------------------------------------------------------------------*/

int scan_userfile (char *call, int query)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("scan_userfile");
  char buf[32];
  char s[49];
  handle fh;

  // return mails only if user has not disabled user-forwarding
  if (get_ufwd(call)[0] == 1)
  {
    if (query) return 0;
    else return fwdput_no_files;
  }
  sprintf(b->listpath, "%s/%s/" DIRLISTNAME, m.userpath, call);
  strlwr(b->listpath);
  if ((fh = s_open(b->listpath, "srb")) != EOF)
  {
    long lastpos = 0L;
    while(1)
    {
      waitfor(e_ticsfull);
      lseek(fh, lastpos, SEEK_SET);
      if (_read(fh, buf, 32) != 32) break;
      lastpos += BLEN;
      if (buf[14] == '>') // Nachricht nicht geloescht?
      {
        if (query)
        {
          s_close(fh);
          return 1;
        }
        else
        {
          buf[7] = 0;
          sprintf(s, "%s/%s", call, buf);
          strupr(s);
          s_close(fh);
          if (sendonemail(s) != fwdsend_ok) return fwdput_error;
          if ((fh = s_open(b->listpath, "srb")) == EOF) break;
        }
      }
    }
    s_close(fh);
  }
  if (query) return 0;
  else return fwdput_no_files;
}

/*---------------------------------------------------------------------------*/

#ifndef _FILEFWD
static
#endif
       int scan_fwdfile (char type)
//*************************************************************************
//
//  Schaut eine Forwardliste durch und veranlasst ggf. das Abschicken
//  der enthaltenen Files
//
//*************************************************************************
{
  lastfunc("scan_fwdfile");
  char s[50];
  char *p = s;
  fpara()->newmail = 0;
  while (getfwdlistentry(b->logincall, type, 1, &p))
  {
    if (sendonemail(s) != fwdsend_ok) return fwdput_error;
    delfwdlistentry(s);
    if (! b->usermail && fpara()->newmail == 2)
      return fwdput_newmail; // New usermail has arrived
    p = s; // Reset pointer (move in getfwdlistentry())
    waitfor(e_reschedule);
  }
  return fwdput_no_files;
}

/*---------------------------------------------------------------------------*/

#ifndef _FILEFWD
static
#endif
       int scan_emfile (char *path)
//*************************************************************************
//
//   Schaut eine Forwardliste durch und veranlasst ggf. das Abschicken
//   der enthaltenen Files
//
//*************************************************************************
{
  lastfunc("scan_emfile");
  char fwdfname[20];
  char s[162];
  FILE *f;
  char *hadr;
  int anzahl = 0;
  int len = 0;
  long lastpos = 0, prepos = 0;
  int broken = 0;
  fpara()->newmail = 0;
  strcpy(fwdfname, path);
  strcat(fwdfname, b->logincall);
  strcat(fwdfname, ".bcm");
  strlwr(fwdfname);
  killforwarded(fwdfname);
  if ((f = s_fopen(fwdfname, "srt")) != NULL) //fwdtrigger
  {
    if (b->boxtyp != b_thebox || b->boxversion > 17) // diebox < 1.8 garnix
    {
      while (fgets(s, sizeof(s) - 1, f))
      {
        lastpos = ftell(f);
        if (*s > ' ')
        {
          s_fclose(f);
          len = strlen(s);
          rm_crlf(s);
          if (len > 40 && len < 70) // Plausi-check
          {
            lastcmd(s);
            if (b->boxtyp == b_thebox) // keine H-Adressen
            {
              hadr = strchr(s, '.'); // sonst Absturz...
              if (hadr && (*s == 'M'))
              {
                char *hsave = hadr;
                while (*hsave && *hsave != ' ') hsave++;
                while (*hsave)
                {
                  *hadr++ = *hsave++;
                  *hadr = 0;
                }
              }
            }
            if (fpara()->options & o_e)
            {
              char *cp = strchr(s, '$');
              if (cp)
              {
                cp++;
                while (*cp && *cp != ' ') cp++;
                *cp = LF;
                putf("S %s\n\n", s);
                *cp = ' ';
              }
            }
            else putf("S %s ", s);

#ifdef _FILEFWD
            if (b->forwarding == fwd_file) putf("/EX\n");
            else
#endif
            putf("\032\n"); // CTRL-Z
            fwdlog("S ", s, 'S');
            waitfor(e_reschedule);
            if ((anzahl++) > 10 || b->boxtyp == b_thebox
                                || (fpara()->options & o_e))
            {
              while (anzahl)
              {
                if (! waitfwdprompt(b->line))
                  return fwdput_error;
                anzahl--;
              }
            }
          }
          waitfor(e_reschedule);
          if (! (f = s_fopen(fwdfname, "sr+t")))
            return fwdput_error;
          fseek(f, prepos, SEEK_SET);
          strcat(s, "\n");
          *s = ' ';
          fwrite(s, len, 1, f);
          if (fpara()->newmail)
          {
            broken = 1;
            break;
          }
          fseek(f, lastpos, SEEK_SET);
        }
        prepos = ftell(f);
      }
    }
    if (f) s_fclose(f);
    while (anzahl)
    {
      if (! waitfwdprompt(b->line))
        return fwdput_error;
      anzahl--;
    }
    if (! broken) xunlink(fwdfname);
    return fwdput_files_sent;
  }
  return fwdput_no_files;
}

/*---------------------------------------------------------------------------*/

void ascii_fwdmachine (int send_) // ASCII-Fwd
//*************************************************************************
//
//  Genereller Forward-Automat. Wird sowohl zum Senden als auch
//  zum Empfangen aufgerufen
//
//  send: 0 = ankommender Connect => SLAVE
//        1 = abgehender Connect  => MASTER
//
//*************************************************************************
{
  lastfunc("ascii_fwdmachine");
  fwdstate_t fwdstate;
  int delay_count = 0;
  int do_delay = 0;
  int was_delay = 0; // for userforwarding, max. "fwd_delay" only once
  if (send_) fwdstate = fwd_send;
  else fwdstate = fwd_prompt;
  lastcmd("fwd_loop");
  while (fwdstate != fwd_end)
    //fwd_send,fwd_delay,fwd_receive,fwd_prompt,fwd_done
  {
    switch (fwdstate)
    {
      case fwd_send:
        b->job = j_fwdsend;
        lastcmd("fwd_send");
        switch (fwdputfiles())
        {
          case fwdput_double: fwdput("***send_2connects", ""); fwdstate = fwd_end; break;
          case fwdput_error: fwdput("***send_error", ""); fwdstate = fwd_end; break;
          case fwdput_files_sent:
          case fwdput_newmail: do_delay = 1; break; // state remains as it is, new trial
          case fwdput_no_files: fwdstate = fwd_delay; delay_count = 0; break;
        }
        break;
      case fwd_delay:
        b->job = j_fwdsend;
        lastcmd("fwd_delay");
        if (was_delay == 1) fwdstate=fwd_done; // OE3DZW: In user-fwd fwd-delay only once
        else
        {
          if (b->forwarding == fwd_user) was_delay = 1;
          if (chkforward(b->logincall)) fwdstate = fwd_send;
          else
          {
            if (b->forwarding == fwd_standard && do_delay) wdelay(510);
            if ((delay_count++) >= 300 || b->forwarding == fwd_user || ! do_delay)
            {
              if (   eingelogt(b->logincall, j_fwdrecv, 0)
                  || eingelogt(b->logincall, j_fwdrecvf, 0) || ! send_)
                fwdstate = fwd_done;
              else
              {
                fwdput("F>", "");
                fwdstate = fwd_receive;
              }
            }
          }
        }
        break;
      case fwd_receive:
        lastcmd("fwd_rec");
        switch (fwdcmd())
        {
          case fwdcmd_error: fwdput("***syntax_error", "");
          case fwdcmd_end: fwdstate = fwd_end; break;
          case fwdcmd_reverse:
            if (   eingelogt(b->logincall, j_fwdsend, 0)
                || eingelogt(b->logincall, j_fwdsendf, 0) || send_)
              fwdstate = fwd_done;
            fwdstate = fwd_delay;
            delay_count = 0;
            break;
          case fwdcmd_ok: fwdstate = fwd_prompt; break;
        }
        break;
      case fwd_prompt:
        lastcmd("fwd_pro");
        fwdput(">", "");
        fwdstate = fwd_receive;
        break;
      case fwd_done:
        fwdstate = fwd_end;
        fwdput("***done", "");
        break;
      default: break;
    }
    wdelay(156);
  }
  putflush();
  wdelay(5000);
}

/*---------------------------------------------------------------------------*/

static int fwdsendbefehl (void)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("fwdsendbefehl");
  char s[LINELEN+1];
  // DH3MB
  sprintf(b->line, "%s < %s @%s%s%s $%s", b->ziel, b->herkunft, b->at,
          *b->frombox ? " ~" : "", b->frombox, b->bid);
  lastcmd(b->line);
  // Only send 6 characters for the boardname, if no [D] in SID
  // DH3MB: Removed check for TheBox, DXL-Box and BayCom-Mailbox, because
  // only very old versions do not send [D] in the SID
  if (! (b->opt & o_d)) b->ziel[6] = 0;
  // OE3DZW: Mailtype is saved and used when sending the mail
  sprintf(s, "S%c %s", b->mailtype, b->ziel);
  // OE3DZW: Only use mailtype 'A', if it's supported by the neighbour
  if (b->mailtype == 'A' && ! (b->opt & o_a)) s[1] = 'P';
  if (   *b->at && (strcmp(b->at, b->logincall) || ! b->usermail)
      && (   b->forwarding == fwd_standard
#ifdef _FILEFWD
          || b->forwarding == fwd_file
#endif
     )   )
    sprintf(s + strlen(s), " @ %s", b->at);
  sprintf(s + strlen(s), " < %s", b->herkunft);
  //Always send MID, even if there is no M in SID of partner-bbs
  sprintf(s + strlen(s), " $%s", b->bid);
  // DH3MB: Removed check for TheBox, DXL-Box and BayCom-Mailbox, because
  // only very old versions do not send [D] in the SID
  if (b->lifetime && (b->opt & o_d))
    sprintf(s + strlen(s), " # %u", b->lifetime);
  fwdput(s, "");
  trace(report, b->logincall, ">%s", s);
#ifdef _FILEFWD
  if (b->forwarding != fwd_file)
  {
#endif
  do
    fwdget(s, 39);
  while (! *s || s[strlen(s) - 1] == '>'); // If there were too many prompts
#ifdef _FILEFWD
  }
  else *s = 'O';
#endif
  return toupper(*s);
}

/*---------------------------------------------------------------------------*/

static fwdsend_t fwdsendfile (void)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("fwdsendfile");
  char *line = b->line;
  int was_locked = 0;
  int was_ok = 0;
  int fwdsendcmd = 0;

  unsigned long real_tx_bytes = 0; // uncompressed amount of TX-Bytes
  time_t starttime, txtime, delta_lastmeasure;
  int i;
  long routing_quality_old = 0;
  char lbuf[80];

  FILE *f = s_fopen(b->mailpath, "lrb");
  if (! f || ! scanheader(f, fpara()) )
    return fwdsend_file_error; // f is closed in scanheader
//  if (! mbhadrok (b->at)) return fwdsend_file_error;
  f = s_fopen(b->mailpath, "lrb");
  if (mbhadrok(b->at) == 1) do // why for user mails ?
  {
    if (! sema_lock(b->mailpath))
    {
      s_fclose(f);
      wdelay(7079);
      f = s_fopen(b->mailpath, "lrb");
      was_locked = 1;
    }
    else was_locked = 0;
  }
  while (was_locked);
//  rewind(f);
  fgets(b->line, BUFLEN - 1, f);    // Skip command header line
  fgets(b->line, BUFLEN - 1, f);    // Read forward trigger line
  if (*line == '*')      // Has message been erased/forwarded in the meanwhile?
  {
    trace(report, "fwdsendfile", "erased %s>%s %s",
                                 b->herkunft, b->ziel, b->logincall);
    s_fclose(f);
    return fwdsend_file_error;
  }
  if (! fgets(b->line, BUFLEN - 1, f)) // Skip 'Read:' line
  {
    s_fclose(f);
    return fwdsend_file_error;
  }
  fwdsendcmd = fwdsendbefehl();
  fwdpara_t *ff = fpara();
  switch (fwdsendcmd)
  {
    case 'O': // OK
      ff->txf++;
    case 'H': // HOLD (F6FBB)
      // mark mail as proposed
      fgets(b->line, BUFLEN - 1, f);
      rm_crlf(line);
      fwdput(line, "");
      if (b->boxtyp == b_thebox) putflush();
      fwdlog("---- ", "[text file]", 'S');

      b->job = j_fwdsendf;
      starttime = ad_time();
      real_tx_bytes = b->txbytes;
      readmail(f, 0); // file is closed within readmail()
      real_tx_bytes = b->txbytes - real_tx_bytes; // nur von letzter Mail!
      txtime = ad_time() - starttime;
      f = NULL;
      if (b->mailtype != 'B')
      {
        if (b->fwdprop < 127) b->fwdprop++;
        writemailflags();
      }
#ifdef _FILEFWD
      if (b->forwarding == fwd_file)
      {
        if (! b->binstart) putf("/EX\n");
        else putv(LF);
      }
      else
#endif
      if (! b->binstart) putf("\032\n"); // CTRL-Z
      b->job = j_fwdsend;
      b->readfiles++;
      was_ok = 1;


//----

  if (ff->routing_quality == WPROT_MIN_ROUTING)
  {
    ff->routing_txf = 0;
    ff->lastmeasure = 0;
    ff->lastwpr = 0;
  }
  delta_lastmeasure = ad_time() - ff->lastmeasure;
  if (real_tx_bytes > 0 && (real_tx_bytes > 512 || delta_lastmeasure > 3600))
  {
    ff->routing_txf++;
    ff->current_routing_quality = txtime * 100000L / real_tx_bytes;
    if (ff->current_routing_quality == 0) ff->current_routing_quality = 1;
                                  // quality auf 100kByte normalisieren
    sprintf(lbuf, "M %-6.6s %-6.6s %s (%ld) 10 1   %-5ld (%ldm/%-5ldg)\n",
                  ff->call, b->logincall, datestr(ad_time(), 12),
                  ad_time(), ff->current_routing_quality, ff->routing_txf,
                  ff->routing_quality);
    wprotlog(lbuf, ff->call);
    //Aging of quality
    if (delta_lastmeasure <= DAY)
    {
      for (i = 1; i < 12; i++)
      {
        if ((delta_lastmeasure > i*HOUR) && (delta_lastmeasure < (i+1)*HOUR))
        {
          routing_quality_old = ff->routing_quality;
          ff->routing_quality = ff->routing_quality + ff->routing_quality*i/5; //+20% pro Stunde
          if (ff->routing_quality == routing_quality_old)
            ff->routing_quality = ff->routing_quality + i; //mindestens aber um +1 pro Stunde
        }
      }
    }
    if (delta_lastmeasure > DAY) ff->routing_quality = WPROT_MIN_ROUTING;
    if (delta_lastmeasure > 3*DAY) ff->routing_quality = 0; //? correct?
    sprintf(lbuf, "A %-6.6s %-6.6s %s (%ld/%ld) (%-5ldg) %ldb\n",
                  ff->call, b->logincall, datestr(ff->lastmeasure, 12),
                  ff->lastmeasure, delta_lastmeasure, ff->routing_quality, real_tx_bytes);
    wprotlog(lbuf, ff->call);
    //The quality is the mean value of all measurements
    ff->routing_quality = (ff->routing_quality*(ff->routing_txf - 1)
                            + ff->current_routing_quality)/ff->routing_txf;
    if (ff->routing_quality < 1)
      ff->routing_quality = 1; //mindestens Wert von 1
    ff->lastmeasure = ad_time();
    sprintf(lbuf, "N %-6.6s %-6.6s %s (%ld) 10 1   %-5ld\n",
                  ff->call, b->logincall, datestr(ff->lastmeasure, 12),
                  ff->lastmeasure, ff->routing_quality);
    wprotlog(lbuf, ff->call);
  }
  if ((ad_time() - ff->lastwpr) > 5*HOUR) // after 5h
  {
    wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wps");
    safe_strcpy(wp->bbs, ff->call);
    wp->bversion = BVERSION;
    ff->lastwpr = wp->mybbstime = ff->lastmeasure;
    wp->hops = 1; // wird in addwp_r +1 erhoeht
    wp->qual = ff->routing_quality;
    wprotlog("C ", wp->bbs);
    addwp_r(wp);
    ff->routing_txf = 1;
    t_free(wp);
  }
//----



      // fall through
    case 'L': // LATER (W0RLI)
      // TODO: send later OE3DZW
    case 'N': // NO
    case 'E': // ERROR (F6FBB)
    case 'R': // REJECT (F6FBB)
      if (f)
      {
        s_fclose(f);
        f = NULL;
      }
      switch (waitfwdprompt(line))
      {
        case 1:
          if (was_ok) return fwdsend_ok;
          else
          {
            if (fwdsendcmd == 'R') return fwdsend_rej;
            else return fwdsend_no;
          }
        case 2:
          if (b->binstart)
          {
            trace(replog, "fwds", "AutoBIN CRC failure %s>%s @%s",
                                  b->herkunft, b->ziel, b->at);
            return fwdsend_repeat_file;
          }
          else return fwdsend_ok;
      }
  }
  if (f) s_fclose(f);
  return fwdsend_transfer_error;
}

/*---------------------------------------------------------------------------*/

void fwdproc_reply (char reply)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("fwdproc_reply");

  switch (reply)
  {
    case FBB_YES:
      if (mbhadrok(b->at) == 1 || b->forwarding == fwd_user)
        markerased('F', 0, 0);
      else
        markfwd();
      break;
    case FBB_REJECT:
      b->fwdhold = 'R';
      add_hold("rejected", 1);
      if (b->mailtype == 'P' || b->mailtype == 'A')
      {
        writemailflags();
        //Nachricht bei Partnerbox unerwuenscht - auf 30 Tage setzen
        marklifetime(30);
      }
      break;
    case FBB_NO:
    default:
      if (b->mailtype == 'P' || b->mailtype == 'A')
       {
         if (b->fwdprop > '!' || b->forwarding == fwd_user || ! mbcallok(b->ziel))
           markerased('F', 0, 0);
         else
         {
           b->fwdhold = 'N';
           add_hold("already rcvd", 0);
           //db1ras 16.05.2009: Wir geben dem Absender bescheid, dass die Mail
           //nicht geforwarded werden konnte!
           add_hold("already rcvd", 1);
           writemailflags();
         //df3vi: Nachricht bei Partnerbox schon empfangen - auf 7 Tage setzen
         //  marklifetime(7);
         //dh8ymb 02.03.06: ist loeschen bei Usermails nicht besser?
           markerased('F', 0, 0);
           /*db1ras 14.05.09: "already rcvd" bei Usermails bedeutet, dass die
             MID der Mail bei der Partnerbox bereits bekannt ist, die Mail also
             schon einmal darüber geforwardet wurde und die Partnerbox offenbar
             keinen korrekten Weg zum Ziel kennt. Die Mail ist deshalb aber
             *nicht* in der Partnerbox vorhanden, da Usermails im Netz nur
             einmal existieren und aktuell haben wir sie. Wir können die
             Usermail deshalb nicht still und heimlich einfach löschen!

             Stattdessen informieren wir mit der obigen add_hold()-Änderung den
             Absender und setzen die Mail auf LT 30, so dass der Absender
             (oder Empfänger) ausreichend Zeit hat, die Mail ggf. nochmals
             auszulesen. Mir ist kein Grund bekannt, warum die Zeit kürzer sein
             sollte als bei "rejected" weiter oben, deshabvl LT 30 und nicht 7.
           */
           marklifetime(30);
         }
       }
       if (b->mailtype == 'B' && mbhadrok(b->at) == 1)
         markerased('F', 0, 0);
       markfwd();
       break;
  }
}

/*---------------------------------------------------------------------------*/

static int sendonemail (char *mailname)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("sendonemail");
  int fwdsend_result;
  if (setfullpath(mailname))
  {
    strlwr(b->mailpath);
    int retry = 1;
    while (1)
    {
      fwdsend_result = fwdsendfile();
      waitfor(e_reschedule);
      switch (fwdsend_result)
      {
        case fwdsend_transfer_error: sema_unlock(b->mailpath); return fwdput_error;
        case fwdsend_rej: fwdproc_reply(FBB_REJECT); break;
        case fwdsend_no:  fwdproc_reply(FBB_NO); break;
        case fwdsend_ok:  fwdproc_reply(FBB_YES); break;
        case fwdsend_repeat_file: if (retry++ < 5) continue;
        case fwdsend_file_error: markfwd(); break;
      }
      waitfor(e_reschedule);
      break;
    }
    sema_unlock(b->mailpath);
  }
  else trace(report, "sendonemail", "dir %s", board_aus_path(mailname));
  return fwdsend_ok;
}

/*---------------------------------------------------------------------------*/

static int fwdputfiles (void)
//*************************************************************************
//
//  Start des Forwardbetriebs zu einer Box, mit der bereits die
//  Verbindung besteht und das SID ausgetauscht ist.
//
//*************************************************************************
{
  lastfunc("fwdputfiles");
  int retwert = fwdput_double;

  b->job = j_fwdsend;
  if (! another_fwd_sender(b->logincall))
  {
    if (b->forwarding == fwd_standard)
    {
      retwert = scan_fwdfile('u');
      if (retwert != fwdput_no_files) return retwert;
      if (isforwardtime(b->logincall, 1))
      {
        retwert = scan_fwdfile('i');
        if (retwert != fwdput_no_files) return retwert;
      }
      // E/M nur anbieten, wenn "D" im SID oder DieBox 1.8-1.9
      if (b->opt & o_d)
        if (isforwardtime(b->logincall, 2))
          retwert = scan_emfile(FWDPATH "/m_");
    }
    else retwert = scan_userfile(b->logincall, 0);
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

#ifndef _FILEFWD
static
#endif
       int fwdcmd (void)
//*************************************************************************
//
//  Hier werden die ankommenden Kommandos beim Forward-Empfang
//  abgearbeitet. Es werden nur Kommandos akzeptiert, die fuer den
//  Forwardbetrieb relevant sind, alles andere fuehrt zum Abbruch
//  des Forwarding.
//
//*************************************************************************
{
  lastfunc("fwdcmd");
  static char *beftab[] =
  { "SEND", "SB", "SP", "ST", "SA", "F>", ">", NULL
  };

  enum befnum
  { unsinn, send_, sb, sp, st, sa, reverse, prompt
  } cmd = unsinn;

  char buf[121];
  char *bufptr = buf;

  b->continous = 1;
  b->job = j_fwdrecv;
  fwdget(buf, 118);
#ifdef _FILEFWD
  if (b->forwarding == fwd_file && t->input == io_dummy) return fwdcmd_end;
#endif
  bufptr += blkill(bufptr);
  if (! *bufptr) return fwdcmd_ok;
  trace(report, b->logincall, bufptr);
  lastcmd(bufptr);
  if (*bufptr == '[') return fwdcmd_ok;
  if (*bufptr == '*') return fwdcmd_end;
  cmd = (befnum) readcmd(beftab, &bufptr, 0);
  fwdpara_t *ff = fpara();
  switch(cmd)
  {
    case unsinn: return fwdcmd_error;
    case sb:
      b->job = j_fwdrecvf;
      b->mailtype = 'B';
      if (mbsend(bufptr, 0)) ff->rxf++;
      b->job = j_fwdrecv;
      b->eingabefehler = 0;
      break;
    case sp:
      b->job = j_fwdrecvf;
      b->mailtype = 'P';
      if (mbsend(bufptr, 0)) ff->rxf++;
      b->job = j_fwdrecv;
      b->eingabefehler = 0;
      break;
    case st: // was damit tun??
      b->job = j_fwdrecvf;
      b->mailtype = 'T';
      if (mbsend(bufptr, 0)) ff->rxf++;
      b->job = j_fwdrecv;
      b->eingabefehler = 0;
      break;
    case sa:
      b->job = j_fwdrecvf;
      b->mailtype = 'A';
      if (mbsend(bufptr, 0)) ff->rxf++;
      b->job = j_fwdrecv;
      b->eingabefehler = 0;
      break;
    case send_:
      b->job = j_fwdrecvf;
      b->mailtype = '?';
      if (mbsend(bufptr, 0)) ff->rxf++;
      b->job = j_fwdrecv;
      b->eingabefehler = 0;
      break;
    case reverse: return fwdcmd_reverse;
    case prompt: if ((b->eingabefehler++) > 5) return fwdcmd_error;
  }
  return fwdcmd_ok;
}

/*---------------------------------------------------------------------------*/
