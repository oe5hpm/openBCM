/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------
  Forward-Ein- und Ausgabe fuer F6FBB-Forwarding
  ----------------------------------------------


  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980116 OE3DZW commented out unreachable code
//19980124 OE3DZW added rejreason (needed for logging)
//                trace for unpacking corr. position
//                fbb: at trace-msges
//                Added b->readfiles
//                When no mails to send, wait 5min, check every 18s
//19980125 OE3DZW Added waitfor() in "find a filename"
//                Added log when yapp-error rx occurs
//19980128 OE3DZW changed delay to max. 3min
//                fixed:starts with rx when no mails to send (sf..)
//19980202 OE3DZW workaround for blocking when cannot read fwd-triggerfile
//19980207 OE3DZW removed ugly bug under dos - huffman was set very long
//19980208 OE3DZW generates a new bid if there is an old mail without bid
//19980208 OE3DZW on rx length of subject and offset are tested
//                removed trailing \r and \n from subject
//                ignoring trailing fields on fbb-rx
//19980211 OE3DZW fixed: will not forward erased mails
//                found that workaround "cannot rd fwd-file" no longer here
//19980215 OE3DZW cosmetic changes to erasefwded()
//                new tem filename on rx -> stops after 99 tries
//                added crc-check for file-crc (fbb)
//                crc in proposal obligatory if [D1] in SID
//19980223 OE3DZW file-crc check only for [B1]-protocoll, not for B
//                make another check for tmp-bid if we already receive it
//19980302 OE3DZW bigger buffer for recvd_status (7->10char per proposal)
//                added b->fwdprop (before only in ascii-fwd)
//19980304 OE3DZW fixed bug, bulletins were erased after forwarding
//19980307 OE3DZW win32-typecast for rx-file-crc
//                added timeout to yapp-head, yapp-end
//                now uses time2filename for temporary files
//                will not erase but hold if personal, also when o_r
//                debug replog when sending empty proposal
//                moved timeout to correct position
//19980309 OE3DZW added reason to add_hold
//                better info in lastcmd
//19980311 OE3DZW added timeout-trigger to sendonemail
//                better info in last-cmd
//19980312 OE3DZW fixed sendnum, returns 0 if there is no mail to send
//19980313 OE3DZW black friday 13th
//19980318 OE3DZW more robust against trash in fwd-triggerfile
//19980329 OE3DZW corrected fwd-rx/tx byte-infos
//19980404 OE3DZW removed debug-output "not this one"
//                removed last_cmd/timeout in fwd-delay (not usefull)
//                bin mails caused empty-proposals, fixed
//19980408 OE3DZW mailpath into log when fopen err
//19980421 OE3DZW added memset when receiving mail subject
//                added two s_fsetopt=erase on disconnect
//19980427 OE3DZW fwd-option -u ->do not wait
//19980506 OE3DZW defer caused removal from fwd-list, fixed
//19980609 OE3DZW usermails will be erased when replied with "no" in userfwd
//                sfhold - added parameter
//19980610 OE3DZW sfhold - not for  "sysop" mails of type "p"
//19980614 OE3DZW added update_mail -> will reroute mail received in fbb-fwd
//19980615 OE3DZW using fwdproc_reply
//19980615 OE3DZW log/lostcommand with at
//19980620 OE3DZW string was too short, fixed (output of ps)
//19980904 OE3DZW fixed check for bid
//                started implementation of resume
//19980905 OE3DZW finished implementation of resume, first tests
//19980916 OE3DZW checked sema at check_fragement-routine
//                added signature
//19990206 OE3DZW added check for huffman-err
//19990411 OE3DZW fwd-proposal: if b->at==""->b->logincall used
//19990615 DH3MB  adapted for new CRC classes
//                CRC/checksum after proposal ist optional again (This is,
//                what the SPEC says!)
//19990616 DH3MB  reduced parseprop()-parameters to "char *prop",
//                CRC-/Checksum-calculation is now done in replyprop()
//19990703 DH3MB  Changed "recv" to "rx" and "send" to "tx"
//                Changed "& 0xff" to "% 0x100" when sending the
//                CRCs, should be architecture independent noew
//20000104 OE3DZW fixed problem with -u, did not delay in fwd
//20000116 DK2UI  (dzw) added ui-options
//20000116 OE3DZW fixed problem with -u again
//20000118 OE3DZW mail-f-open now binary
//20000131 OE3DZW added some lastcmds on fwd-rx (debug to find bug)

#include "baycom.h"


/*---------------------------------------------------------------------------*/

fbb::fbb (void)
//*************************************************************************
//
//  Initialization
//
//*************************************************************************
{
  strcpy(signature, SIGNATURSTR);
}

/*---------------------------------------------------------------------------*/

fbb::~fbb (void)
//*************************************************************************
//
//  Deinitialization
//
//*************************************************************************
{
  if (strcmp(signature, SIGNATURSTR))
    trace(fatal, "fbb", "sig broken");
}

/*---------------------------------------------------------------------------*/

char fbb::tx_proposal (char *tosend, char *recvd_status)
//*************************************************************************
//
//  Checks if there is mail waiting in the outgoing queue, if there
//  is some mail it sends a proposal and receives the reply
//  Returns: 0    = there is no mail
//           1..5 = there were 1..5 mails in the proposal
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "fbb:tx_proposal");
  lastfunc(name);
  char sendnum = 0; // Number of mails in the list
  char *p; // Pointer for the list
  int i, j; // Counter
  FILE *f; // Pointer for mail file
  crcthp prop_crc; // CRCthp of the proposal (if [D] in SID)
  char prop_sum = 0; // Checksum of the proposal (if no [D] in SID)
  char was_error; // Was there an error in the last proposal?
  char error; // Was there an error in the current proposal?

  if (another_fwd_sender(b->logincall) || ! chkforward(b->logincall)) return 0;
  do
  {
    waitfor(e_ticsfull);
    p = tosend; // Move pointer to the begin of the list
    sendnum = 0; // No entries in the list yet
    was_error = 0; // No error - yet ;-)
    if (b->forwarding == fwd_standard) // Normal S&F -> forward-trigger-file
    {
      sendnum += getfwdlistentry(b->logincall, 'u', FBB_MAX_PROP - sendnum, &p);
      if (sendnum < FBB_MAX_PROP && isforwardtime(b->logincall, 1))
        sendnum += getfwdlistentry(b->logincall, 'i', FBB_MAX_PROP - sendnum, &p);
      // e/m-msges only if d(iebox) compatible system
      if (sendnum < FBB_MAX_PROP && (b->opt & o_d) && isforwardtime(b->logincall, 2))
        sendnum += getfwdlistentry(b->logincall, 'm', FBB_MAX_PROP - sendnum, &p);
    }    // User S&F -> list-file of the user
    else sendnum += getfwdlistentry(b->logincall, 'l', FBB_MAX_PROP, &p);
    waitfor(e_ticsfull);
    p = tosend; // Move pointer to the begin of the list
    if (sendnum > FBB_MAX_PROP)
      trace(fatal, name, "sendnum %d", sendnum); //debug, should never happen
    for (i = 0; i < sendnum; i++)
    {
      error = 0; // No error - yet
      if (p[1] != ' ') // Bulletin or personal mail -> Examine mail file
      {
        if (strlen(p) < 9) //sometimes there is trash in the triggerfile -> debug
        {
          trace(serious, name, "trash: %s", p);
          error = 1;
        }
        if (setfullpath(p))
        {
          strlwr(b->mailpath);
          if ((f = s_fopen(b->mailpath, "lrb")) != NULL)
          {
            if (! scanheader(f, fpara()))
              error = 1;  //checks if binary mail
          }
          else
          {
            trace(report, name, "mailfile not existing");
            error = 1;
          }
        }
        else
        {
          trace(serious, name, "dir %s", board_aus_path(p));
          error = 1;
        }
      }
      waitfor(e_ticsfull);
      if (error) // Error -> Delete from forward-list / mark as forwarded
      {
        if (b->forwarding == fwd_standard)
        {
          delfwdlistentry(p);  //eg. bin mail when no bin-fwd
          trace(report, name, "fwd del: %s", p);
        }
        else
        {
          markerased('F', 0, 0);
          delfwdlistentry(p);  //eg. bin mail when no bin-fwd
          trace(report, name, "fwd erased: %s", p);
        }
        was_error = 1;
      }
      p += strlen(p) + 1; // Move to next entry in the forward list
      waitfor(e_ticsfull);
    }
  }
  while (was_error);
  p = tosend; // Move pointer to the begin of the list
  for (i = 0; i < sendnum; i++)
  {
    if (p[1] == ' ') // E/M-mail
    {
      mbsend_parse(p, 0);
      b->mailtype = 'B';
      b->bytes = 80L;
    }
    else // Bulletin or personal mail
    {
      setfullpath(p);
      parse_headerline();
    }
    if (! *b->bid) //if there is no bid, generate a new one;
                    //this SHOULD not happen, but if someone makes
                    //an update from a very old version this is
                    //usefull (else we get a protocol error)
    {
      strcpy(b->bid, newbid());
      trace(replog, name, "gen newbid $%s %s>%s",
             b->bid, b->herkunft, b->ziel);
    }
    if (! (b->opt & o_d))
      b->ziel[6] = 0; // No [D] in SID -> use max. 6 characters in boardname
    if (b->forwarding == fwd_user || ! *b->at)
      strcpy(b->at, b->logincall);
    if (b->opt & o_d)
      strcpy(b->line, D_PROPOSE);
    else
    {
      if (b->opt & o_b)
        strcpy(b->line, FBB_COMPPROP); // only correct if text-msg
      else
        strcpy(b->line, FBB_PROPOSE);  // old ascii-protocol
    }
    snprintf(b->line + strlen(b->line), BUFLEN - strlen(b->line),
              " %c %s %s %s %s %ld", b->mailtype, b->herkunft,
              b->at, b->ziel, b->bid, b->bytes);
    if ((b->opt & o_d) && b->lifetime) // [D] in SID -> Send lifetime
      snprintf(b->line + strlen(b->line), BUFLEN - strlen(b->line),
                                          " %d", b->lifetime);
    fwdput(b->line, "");
    for (j = 0; j < strlen (b->line); j++)
      if (b->opt & o_d)
        prop_crc.update(b->line[j]); // use CRCthp/Checksum depending
                                     // on [D] in SID
      else
        prop_sum += b->line[j];
    if (! (b->opt & o_d)) prop_sum += 13; // CR is included in the checksum
    p += strlen(p) + 1; // Move to next entry in the forward list
  }
  if (! sendnum) //if no proposal was found to be valid,
  { //ignore that problem, fix it later..
    //trace(report, name, "nothing to send");
    wdelay(8909);  //should no longer loop at all, but it does..
    return 0;
  }
  if (b->opt & o_d) // Use CRCthp/Checksum depending on [D] in SID
    snprintf(b->line, BUFLEN, " %02X%02X",
                      prop_crc.result % 0x100, prop_crc.result / 0x100);
  else
    snprintf(b->line, BUFLEN, " %02X", (-prop_sum) & 0xff);
  fwdput(FBB_PROMPT, b->line);
  fwdget(recvd_status, 3 + 10 * FBB_MAX_PROP);
  return (sendnum);
}

/*---------------------------------------------------------------------------*/

void fbb::tx_yapp_header (void)
//*************************************************************************
//
//  Sends the mail header and calculate the CRCs
//
//*************************************************************************
{
  int i;
  //lastcmd("fwd: Send mailheader");
  fwdlog("---- ", "[mail header]", 'S');
/* accept any offset
  if (b->fbboffset > 999999L)
  {
    trace(replog, "fbb:tx_yapp_", "fbboffset %ld", b->fbboffset);
    b->fbboffset = 999999L;
  }
*/
  snprintf(b->line, BUFLEN, "%6lu", b->fbboffset);
  bputv(SOH);
  bputv(strlen(b->betreff) + strlen(b->line) +2); //variable length of offset
  putf("%s", b->betreff);
  bputv(NUL);
  putf("%s", b->line);
  bputv(NUL);
  waitfor(e_ticsfull);
  // CRCfbb only covers the subject, if [D] in SID
  if (b->opt & o_d)
    for (i = 0; i < strlen(b->betreff); i++)
      mail_crcfbb->update(b->betreff[i]);
  // CRCthp covers subject, offset and null-bytes
  for (i = 0; i < strlen(b->betreff); i++)
    mail_crcthp->update(b->betreff[i]);
  mail_crcthp->update((char) 0);
  for (i = 0; i < strlen(b->line); i++)
    mail_crcthp->update(b->line[i]);
  mail_crcthp->update((char) 0);
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

void fbb::tx_yapp_data (char *buf, unsigned short int length)
//*************************************************************************
//
//  Sends an FBB-YAPP block
//
//*************************************************************************
{
  unsigned short int i;
  crcfbb block_crc;
  char name[20]; // Name for syslog

  strcpy(name, "fbb:tx_yapp_block");
  if (! length)
  {
    trace(serious, name, "Try to send 0-byte YAPP block");
    return;
  }
  bputv(STX);
  if (length == 256) bputv(0);
  else bputv(length);
  for (i = 0; i < length; i++) // Send the YAPP-block
  {
    bputv(buf[i]);
    block_crc.update(buf[i]);
    mail_crcthp->update(buf[i]);
    mail_checksum += buf[i];
  }
  if (b->opt & o_d1) // Send block CRC if [D1] in SID
  {
    bputv(block_crc.result % 0x100);
    bputv(block_crc.result / 0x100);
  }
  timeout(m.fwdtimeout);
  t->last_input = ad_time();
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

void fbb::tx_yapp_end (void)
//*************************************************************************
//
//  Sends the end-of-transfer block (EOT+Checksum/CRCthp)
//
//*************************************************************************
{
  //lastcmd("fwd: Send EOT");
  fwdlog("---- ", "[end of mail]", 'S');
  bputv(EOT); // End of transfer identifier
  if ((b->opt & o_d) && ! (b->opt & o_b1)) // [D], no [B1] -> send CRCthp
  {
    bputv(mail_crcthp->result % 0x100);
    bputv(mail_crcthp->result / 0x100);
  }
  else
    bputv(-mail_checksum); // Send the checksum otherwise
}

/*---------------------------------------------------------------------------*/

char fbb::tx_onemail (void)
//*************************************************************************
//
//  Sends a mail in FBB/YAPP-format
//  Returns 1 on success, 0 otherwise
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "fbb:tx_onemail"); // Name for syslog
  lastfunc(name);
  int retwert = 0;
  FILE *f;                        // Pointer for the mail file
  handle fh;                      // Handle for the compressed mail file
  unsigned short int bytes;       // Number of bytes in the YAPP block
                                  // (max. 256 Bytes)
  char unpackedfname[25];         // Name of the uncompressed file
  char packedfname[25];           // Name of the compressed file
  char predatanum = 0;            // Number of bytes we must send
  unsigned long real_tx_bytes = 0; // uncompressed amount of TX-Bytes
  time_t starttime, txtime = 0L, delta_lastmeasure;
  fwdpara_t *ff = fpara();
  int i;
  long routing_quality_old = 0;
  char lbuf[80];

  if (! (b->opt & o_b)) // Partner does not support compressed forward :-(
  {
    fwdlog("---- ", "[mail title]", 'S');
    putf("%s\n", b->betreff);
    timeout(m.fwdtimeout);
    t->last_input = ad_time();
    fwdlog("---- ", "[text file]", 'S');
    b->readfiles++;
    if (! strcmp(b->ziel, "M") || ! strcmp(b->ziel, "E"))
      putf("\n***sysfile\n\n");
    else
      if ((f = s_fopen(b->mailpath, "srb")) != NULL)
      { // Skip the whole header
        fgets(b->line, BUFLEN - 1, f); // mail header line
        fgets(b->line, BUFLEN - 1, f); // forward trigger line
        fgets(b->line, BUFLEN - 1, f); // 'Read:' line
        fgets(b->line, BUFLEN - 1, f); // title line
        readmail(f, 0); // f is closed within readmail()
      }
      else
      {
        trace(serious, name, ms(m_filenotopen), b->mailpath);
        retwert = 0;
        goto ende;
      }
    putf("\032\n"); // Ctrl-Z
    real_tx_bytes = b->txbytes;
  }
  else
  {
    // Initialize CRCs and checksum
    mail_crcthp = new (crcthp);
    mail_crcfbb = new (crcfbb);
    mail_checksum = 0;
    huffcod huff(mail_crcfbb);
    if (huff.err)
      trace(fatal, name, "huffman error");
    timeout(m.fwdtimeout);
    t->last_input = ad_time();
    tx_yapp_header();
    waitfor(e_ticsfull);
    snprintf(unpackedfname, sizeof(unpackedfname),
             FBBPATH "/%s.fbb", time2filename(0));
    strlwr(unpackedfname);
    snprintf(packedfname, sizeof(packedfname),
             FBBPATH "/%s.lha", time2filename(0));
    strlwr(packedfname);
    //trace(report, name, " pack fn %s", unpackedfname);
    if (! strcmp(b->ziel, "M") || ! strcmp(b->ziel, "E"))
    {
      if ((f = s_fopen(unpackedfname, "swb")) != NULL)
      {
        fprintf(f, NEWLINE "***sysfile" NEWLINE NEWLINE); // Create a dummy
                                                          // mail body
        s_fclose(f);                                      // for system-files
      }
      else
      {
        trace(serious, name, "fopen %s errno=%d %s", unpackedfname,
              errno, strerror(errno));
        delete(mail_crcthp);
        delete(mail_crcfbb);
        retwert = 0;
        goto ende;
      }
    }
    else
      if ((f = s_fopen(b->mailpath, "lrb")) != NULL)
      { // Skip the whole header
        fgets(b->line, BUFLEN - 1, f); // mail header line
        fgets(b->line, BUFLEN - 1, f); // forward trigger line
        fgets(b->line, BUFLEN - 1, f); // 'Read:' line
        fgets(b->line, BUFLEN - 1, f); // title line
        b->oldoutput = t->output;
        t->output = io_file;
        if (! (b->outputfile = s_fopen(unpackedfname, "swb")))
        {
          trace(serious, name, "fopen %s errno=%d %s", unpackedfname,
                errno, strerror(errno));
          delete(mail_crcthp);
          delete(mail_crcfbb);
          retwert = 0;
          goto ende;
        }
        s_fsetopt(b->outputfile, 1);
        unsigned long true_rx = b->rxbytes;
        unsigned long true_tx = b->txbytes;
        readmail(f, 1); // f is closed within readmail()
        real_tx_bytes = b->txbytes;
        b->rxbytes = true_rx;
        b->txbytes = true_tx;
        s_fclose(b->outputfile);
        t->output = (io_t) b->oldoutput;
      }
      else
      {
        trace(serious, name, "fopen %s", b->mailpath);
        delete(mail_crcthp);
        delete(mail_crcfbb);
        retwert = 0;
        goto ende;
      }
    waitfor(e_ticsfull);
    huff.encode(unpackedfname, packedfname);
    xunlink(unpackedfname);
    fwdlog("---- ", "[mail body]", 'S');
    b->readfiles++;
    if ((fh = s_open(packedfname, "lrb")) == EOF)
    {
      trace(serious, name, "open packedfile error");
      delete(mail_crcthp);
      delete(mail_crcfbb);
      retwert = 0;
      goto ende;
    }
    s_setopt(fh, 2); // Remove file on disconnect or after close()
    // [B1] in SID: The CRCfbb over the whole file is included in
    // the first datablock
    if (b->opt & o_b1)
    {
      b->line[0] = mail_crcfbb->result % 0x100;
      b->line[1] = mail_crcfbb->result / 0x100;
      predatanum = 2;
      // If the partner wishes to resume a file, we must also repeat
      // the first 4 bytes (= the filelength) before sending the rest
      // of the file
      if (b->fbboffset)
      {
        _read(fh, b->line + 2, 4);
        predatanum = 6;
        lseek(fh, (long) b->fbboffset - 2L, SEEK_SET);
      }
    }
    starttime = ad_time();
    while ((bytes =
            _read(fh, b->line + predatanum, 256 - predatanum)+predatanum) > 0)
    {
      predatanum = 0;
      tx_yapp_data(b->line, bytes);
    }
    txtime = ad_time() - starttime;
    if (! txtime) txtime = 1L;
    s_close(fh); // File is removed automatically via s_setopt()
    tx_yapp_end();
    // Delete the CRC objects
    delete(mail_crcthp);
    delete(mail_crcfbb);
  } // end of huffcod (end of locking in DOS version)
  ff->txf++;
  retwert = 1;

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

ende:
  return (retwert);
}

/*---------------------------------------------------------------------------*/

char fbb::tx_mails (char *tosend, char sendnum, char *recvd_status)
//*************************************************************************
//
//  Sends the mails to the partner
//
//*************************************************************************
{
  lastfunc("fbb:tx_mails");
  char *p = tosend;
  char *state = recvd_status + 3;
  char i;
  FILE *f;
  int was_locked = 0;
  if (   strncmp(FBB_STATUS, recvd_status, strlen(FBB_STATUS))
      || recvd_status[strlen(FBB_STATUS)] != ' ')
  {
    fwdput("*** received status prompt invalid", "");
    return (0);
  }
  for (i = 0; i < sendnum; i++)
  {
    {
      char c[40];
      snprintf(c, sizeof(c), "fwd: tx mail %d/%d", i + 1, sendnum);
      lastcmd(c);
    }
    b->fbboffset = 0L;
    switch (*state)
    {
    case FBB_NO:
    case FBB_NO_:
    case FBB_REJECT:
    case FBB_ERROR:
    case FBB_LATER:  //TODO
    case FBB_LATER_: break; //TODO
    case FBB_RESUME:
    case FBB_RESUME_: b->fbboffset = atol(state + 1);
                      while (isdigit(*(state + 1))) state++;
      // fall through
    case FBB_HOLD:
    case FBB_YES:
    case FBB_YES_:
      if (p[1] == ' ')
        mbsend_parse(p, 0); // E/M-mail
      else // Bulletin or user-mail
      {
        setfullpath(p);
        strlwr(b->mailpath);
        if (! (f = s_fopen(b->mailpath, "lrb")))
        {
          trace(serious, "fbb:tx_mails", "open mailfile error");
          return (0);
        }
        if (mbhadrok(b->at) == 1) do
        {
          if (! sema_lock(b->mailpath))
          {
            s_fclose(f);
            wdelay(7077);
            f = s_fopen(b->mailpath, "lrb");
            was_locked = 1;
          }
          else was_locked = 0;
        }
        while (was_locked);
        rewind(f);
        fgets(b->line, BUFLEN - 1, f); // Read command header line
        b->line[254] = 0;
        mbsend_parse(b->line, 0);
        fgets(b->line, BUFLEN - 1, f); // Read forward trigger line
        if (*b->line == '*')    // Has msg been erased/fwded in the meanwhile?
        {
          fwdput("*** msg is erased", "");
          sema_unlock(b->mailpath);
          s_fclose(f);
          return (0);
        }
        fgets(b->line, BUFLEN - 1, f);  // Skip 'Read:' line
        fgets(b->betreff, BETREFFLEN, f);
        rm_crlf(b->betreff);
        b->betreff[79] = 0;
        s_fclose(f);
      }
      // Mark mail as proposed
      if (! (b->mailtype == 'B'))
      {
        char c[LINELEN+1];
        if (b->fwdprop < 127) b->fwdprop++;
        writemailflags();
        snprintf(c, LINELEN, "fwd: tx $%s (%s%c%s < %s)",
                             b->bid, b->ziel, b->at[0] ? '@' : ' ',
                             atcall(b->at), b->herkunft);
        lastcmd(c);
        trace(report, "fwd-tx", "$%s (%s%c%s < %s)", b->bid, b->ziel,
                      *b->at ? '@' : ' ', atcall (b->at), b->herkunft);
//dh8ymb: nun in tx_onemail
//        fwdpara_t *ff = fpara();
//        ff->txf++;
      }
      waitfor (e_ticsfull);
      if (! tx_onemail())
      {
        fwdput("*** internal error sending mail", "");
        sema_unlock(b->mailpath);
        return (0);
      }
      //sema_unlock(b->mailpath); // -> now when all mails of proposal
                                  //    have been forwarded
      waitfor(e_ticsfull);
      break;
    default:
      if (*state) fwdput("*** invalid fbbstatus character", "");
      else fwdput("*** fbbstatus too short", "");
      return (0);
    }
    state++;
    p += (strlen(p) + 1); // Move pointer to next entry
    waitfor(e_ticsfull);
  }
  return (1);
}

/*---------------------------------------------------------------------------*/

void fbb::erase_forwarded (char sendnum, char *tosend, char *recvd_status)
//*************************************************************************
//
//  Removes forwarded mails from the forward-queue
//
//*************************************************************************
{
  lastfunc("fbb:erase_forwarded");
  char *p = tosend;             // list of strings from fwd-triggerfile
  char *state = recvd_status+3; // received status from fwd-partner,
                                // e.g. "++--="
  char i;
  for (i = 0; i < sendnum; i++)
  {
    if (p[1] == ' ')
      delfwdlistentry(p); // E/M-mail
    else // normal mail (user/info of type B,P,A)
    {
      setfullpath(p);
      strlwr(b->mailpath);
      if (! parse_headerline())
      {
        trace(serious, "fbb:eraseforwarded", "open mailfile error");
        return;
      }
      else
      {
        switch (*state)
        {
        case FBB_LATER:    // "I am just receiving this mail, maybe
        case FBB_LATER_:   // its me, so better let it remain in the queue
          wdelay(31341);  //on local links to make it slower
          break;
        case FBB_NO:       // "I have this mail"
        case FBB_NO_:
          // erase this mail if personal & proposed
          // hold if personal and not proposed
          if (b->forwarding == fwd_standard) delfwdlistentry(p);
          fwdproc_reply(FBB_NO);
          break;
        case FBB_REJECT:   // "I don't have the mail and I don't want it"
        case FBB_ERROR:
          if (b->forwarding == fwd_standard) delfwdlistentry(p);
          fwdproc_reply(FBB_REJECT);
          break;
        case FBB_RESUME:   // "I want this mail"
        case FBB_RESUME_:
          if (b->forwarding == fwd_standard) delfwdlistentry(p);
          b->fbboffset = atol(state + 1);
          while (isdigit(*(state + 1))) state++;
        // fall through
        case FBB_HOLD:
        case FBB_YES:
        case FBB_YES_:
        default:
          // erase this mail if personal mail, mark as forwarded else
          if (b->forwarding == fwd_standard) delfwdlistentry(p);
          fwdproc_reply(FBB_YES);
          break;
        }
      }
      sema_unlock(b->mailpath); // now sema will be unlocked when
                                 // all proposals are forwarded
    }
    state++;
    p += strlen(p) + 1; // Move pointer to next entry
    waitfor(e_ticsfull);
  }
}

/*---------------------------------------------------------------------------*/

char fbb::parse_proposal (char *prop)
//*************************************************************************
//
//  Scans one line of a proposal
//  Returns 1, if the format was correct, 0 if not
//
//*************************************************************************
{
  lastfunc("fbb:parse_proposal");
  char *p;
  char buf[11];

  // A proposal line always starts with "FA ", "FB " or "FD "
  if (prop[0] != 'F'
      || (prop[1] != 'A' && prop[1] != 'B' && prop[1] != 'D')
      || prop[2] != ' ')
    return (0);
  p = prop + 3;
  b->mailtype = *p;
  p += 2;
  p = nexttoken(p, b->herkunft, CALLEN);
  p = nexttoken(p, b->at, HADRESSLEN);
  p = nexttoken(p, b->ziel, DIRLEN);
  strcpy(b->zielboard, b->ziel);
  // If a mail is addressed to the local SYSOP-board, we will deliver
  // it directly to the sysop (the one specified in init.bcm)
  if (! strcmp(b->zielboard, "SYSOP") && homeadr(b->at))
  {
    strcpy(b->zielboard, m.sysopcall);
    strcpy(b->ziel, m.sysopcall); //df3vi: auch ziel aendern!
    b->mailtype = 'P';
  }
  p = nexttoken(p, b->bid, BIDLEN);
  p = nexttoken(p, buf, 10);
  b->bytes = atoi(buf);
  p = nexttoken(p, buf, 3);
  if (*buf) b->lifetime = atoi(buf); // Lifetime is optional
  else b->lifetime = 0;
  waitfor(e_ticsfull);
  // We keep the mailbox, we have the mail received from in mind
  if (b->forwarding == fwd_standard) strcpy(b->frombox, b->logincall);
  else *b->frombox = 0;
  // If we receive a mail addressed to us in S&F, we will throw
  // away the address, so that the message will be forwarded to the
  // user's home BBS
  if (b->forwarding != fwd_none && ! strcmp(atcall(b->at), m.boxname))
    *b->at = 0;
  return (b->mailtype && *b->herkunft && *b->ziel && *b->bid && b->bytes);
}

/*---------------------------------------------------------------------------*/

unsigned long int fbb::check_fragment (unsigned long &offset, int &idx)
//*************************************************************************
//
//  Checks if we already have received a part of that mail before,
//  returns
//  0..this mail is new, no fragment available
//  else..offset of mail
//  checking for access (sema) is necessary, but no locking needs
//  to be done - there is a bid...
//
//  note: it is not necessary to check if the old part is equal to the
//  new one because if the fragments differ, the crc over the whole
//  file will be invalid - and things will start at the very beginning.
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "fbb:check_fragment");
  lastfunc(name);
  FILE *pf; //compressed file
  char packedfname[25];
  int found_file = 0; //found correct part
  time_t pft; //time of packed file
  time_t now = ad_time();
  unsigned long fpsize; //size of file
  header_t  header;

  idx = 0; //counter for compressed files
  do
  {
    idx++;
    // check if file is correct
    // try to open file
    snprintf(packedfname, sizeof(packedfname),
                          FBBPATH "/%s%02d.lhb", b->logincall, idx);
    strlwr(packedfname);
    // existing?
    if (! (pft = file_isreg(packedfname))) continue;
    // too old?
    if ((now - pft) > (7L * DAY)) // older than one week
    {
      xunlink(packedfname); continue;
    }
    // check if this file is used by another task
    if (sema_access(packedfname))   continue;
    // try to open
    if (! (pf = s_fopen(packedfname, "lrb")))
    {
      trace(serious, name, ms(m_filenotopen), packedfname);
      continue;
    }
    // check size, at least 2kBytes
    fseek (pf, 0, SEEK_END);
    if ((fpsize = ftell(pf)) < 2048UL)
    {
      s_fclose(pf);
      xunlink(packedfname);
      continue;
    }
    rewind(pf);
    // read header
    fread(&header, sizeof(header_t), 1, pf);
    // check signature
    if (   strcmp(header.signature ,FBBHSIGN)
        || strcmp(header.signatur2 ,FBBHSIGN))
    {
      trace(serious, name, "sign broken %s", packedfname);
      s_fclose(pf);
      xunlink(packedfname);
      continue;
    }
    // check version
    if (strcmp(header.version, FBBHVERS))
    {
      s_fclose(pf);
      xunlink(packedfname);
      continue;
    }
    // check if correct file
    if (strcmp(header.bid, b->bid))
    {
      s_fclose(pf);
      continue;
    }
    // now we are sure that it is the correct file and that the
    // file is good.
    s_fclose(pf);
    offset = fpsize - 1024UL - (unsigned long) sizeof(header_t);
    found_file = 1;
  }
  while (! found_file && idx < MAXCOMPRFILE);
  if (! found_file)
  {
    idx = 0;
    return 0L;
  }
  return offset;
}

/*---------------------------------------------------------------------------*/

char fbb::reply_proposal (want_status_t *want_status)
//*************************************************************************
//
//  Reads proposal and checks, which mails we want
//  Then the reply is sent
//  Returns: 0..5 = We want 0..5 mails from the partner
//           FBB_RECV_FINE  = Partner has no mail (has sent "FF")
//           FBB_RECV_QUIT  = Partner wants to quit (has sent "FQ")
//           FBB_RECV_ERROR = An error occured
//
//*************************************************************************
{
  lastfunc("fbb:reply_proposal");
  crcthp prop_crc;          // Calculated CRCthp of the proposal ([D])
  char prop_sum = 0;        // Calculated Checksum of the proposal (no [D])
  unsigned short int check; // Received CRCthp/checksum for the proposal
  int propcount = 0;        // Number of mails which we wish to receive
  int number = 0;           // Number of lines in the received proposal
  char reply[60];           // The FBB-status which we will send
  unsigned long offset;     // Filelength already received in earlier session
  int idx;                  // index of file
  int i;
  // Initialize list of wanted mails
  memset(want_status, 0, sizeof(want_status_t));
  snprintf(reply, sizeof(reply),
                  "%s ", FBB_STATUS); // Put "FS " in front of the reply
  for (;;)
  {
    fwdget(b->line, 100); // Get one line of the proposal
    number++;
    // We received "F>", i.e. the end of the proposal
    if (! strncmp(b->line, FBB_PROMPT, strlen(FBB_PROMPT)))
    {
      if (number == 1)
      {
        fwdput("*** empty proposal", "");
        return FBB_RECV_ERROR;
      }
      else break;
    }
    if (number == 1) // FF or FQ cannot be received in the mid of the proposal
    {
      if (! strncmp(b->line, FBB_FINE, strlen(FBB_FINE))) // Partner has
        return (FBB_RECV_FINE);                   // no mails
      if (! strncmp(b->line, FBB_QUIT, strlen(FBB_QUIT))) // Partner wants
        return (FBB_RECV_QUIT);                   // to quit
    }
    if (number > FBB_MAX_PROP)
    {
      fwdput("*** too many lines in proposal", "");
      return (FBB_RECV_ERROR);
    }
    waitfor(e_ticsfull);
    for (i = 0; i < strlen(b->line); i++)
      if (b->opt & o_d)
        prop_crc.update(b->line[i]); // CRCthp of the proposal ([D] in SID)
      else
        prop_sum += b->line[i]; // Checksum of the proposal (no [D] in SID)
    if (! (b->opt & o_d)) prop_sum += CR; // CR is included in the checksum
    waitfor(e_ticsfull);
    if (parse_proposal(b->line))
    {
      char rejreason[30];
      char want_it = wantmail(rejreason);
      if (want_it == FBB_YES || want_it == FBB_HOLD) // We want the mail
      {
        if (! tmpbidvorhanden(b->bid)) //another check for temp-bid
        {
          tmpbidrein(b->bid, propcount); // Lock the BID
          strcpy(want_status->proptxt[propcount], b->line);
          //is forward-partner understands resume, search for fragment
          if (! (b->opt & o_b1) || ! check_fragment(offset, idx))
          { //we want (ACCEPT or HOLD) that mail
            snprintf(reply + strlen(reply), sizeof(reply) - strlen(reply),
                                            "%c", want_it);
            want_status->offset[propcount] = 0L;
          }
          else
          { //we want (ACCEPT or HOLD) that mail with RESUME - we have a
            //part received already
            snprintf(reply + strlen(reply), sizeof(reply) - strlen(reply),
                                            "%c%ld", FBB_RESUME, offset);
            want_status->offset[propcount] = offset;
            want_status->fragment[propcount] = idx;
          }
          propcount++;
        }
        else
        { //originally we wanted this mail, but in the meantime
          //some other process also receives this mail, defer it
          snprintf(reply + strlen(reply), sizeof(reply) - strlen(reply),
                                          "%c", FBB_LATER);
        }
      }
      else
      { //we do not want this mail (NO, REJECT)
        snprintf(reply + strlen(reply), sizeof(reply) - strlen(reply),
                                        "%c", want_it);
      }
    }
    else // this partner is stupid, give up
      snprintf(reply + strlen(reply), sizeof(reply) - strlen(reply),
                                      "%c", FBB_ERROR); // Proposal invalid
    waitfor(e_ticsfull);
  }
  // Always remember: The checksum/the CRCthp in the proposal is _optional_!
  if (b->line[2] == ' ') // If we receive a checksum/CRCthp, we'll test it
  {
    check = (unsigned short int) strtoul(b->line + 3, (char **) NULL, 16);
    if (b->opt & o_d) // We have received a CRCthp
    {
      if (   ((check / 0x100) != (prop_crc.result % 0x100))
          || ((check % 0x100) != (prop_crc.result / 0x100)))
      {
        fwdput("*** proposal CRCthp error", "");
        return (FBB_RECV_ERROR);
      }
    }
    else // We have received a checksum
    {
      if ((check + prop_sum) & 0xff)
      {
        fwdput("*** proposal checksum error", "");
        return (FBB_RECV_ERROR);
      }
    }
  }
  else
    if (b->opt & o_d1)
    { // There should be a CRC, don't accept it without one
      fwdput("*** CRC missing", "");
      return (FBB_RECV_ERROR);
    }
  fwdput(reply, ""); // Send the reply
  putflush();
  return (propcount);
}

/*---------------------------------------------------------------------------*/

#define OFFSETLEN 10

char fbb::rx_yapp_header (void)
//*************************************************************************
//
//  Receives FBB-mail-header in YAPP-like protocol
//  Returns 1, if the header was received correctly, 0 otherwise
//
//*************************************************************************
{
  int c, i = 0, len, fbegin = 0, field = 0;
  char offsetbuf[OFFSETLEN+1];

  if (getv() != SOH)
  {
    fwdput("*** not recvd start block-id", "");
    return (0);
  }
  len = getv();
  memset(b->betreff, 0, BETREFFLEN); // wipe out betreff
  memset(offsetbuf, 0, OFFSETLEN);
  do
  {
    c = getv();
    // CRCthp covers all bytes of the header
    mail_crcthp->update(c);
    switch (field)
    {
    case 0:
      if (i < BETREFFLEN - 1) // Subject must not be longer!
        b->betreff[i] = c;    // rest of suject is 0x0
      // This is stupid: The CRCfbb does not cover the header by
      // default; however, if there's a [D] in the SID, it DOES,
      // but ONLY the subject :-(
      if ((b->opt & o_b1) && (b->opt & o_d) && c) mail_crcfbb->update(c);
      break;
    case 1:
      if ((i - fbegin) <= 10)
        offsetbuf[i - fbegin] = c;
      else
      {
        fwdput("*** offset too long", "");
        return 0;
      }
      break;
    }
    if (! c)
    {
      field++;
      fbegin = i + 1;
    }
    i++;
  }
  while (i < len);
  waitfor(e_ticsfull);
  b->fbboffset = atol(offsetbuf);
  if (b->fbboffset) fwdlog("---- Offset: ", offsetbuf, 'R');
  timeout(m.fwdtimeout);
  t->last_input = ad_time();
  return (1);
}

/*---------------------------------------------------------------------------*/

char fbb::rx_yapp_data (char *buf, // Buffer for data-block
                        unsigned short int &length, // Length of data-block
                        char recstatus) // 0=first block 1=non-first block
//*************************************************************************
//
//  Receives FBB-data in YAPP-like protocol
//  D-FBB: Additional 2-byte CRC at end of data block
//
//  Returns:  0 = an error occured
//            1 = everything is fine
//            2 = received EOT
//
//*************************************************************************
{
  char c;
  unsigned int i;                  // Length of the YAPP block
  crcfbb block_crc;                // Calculated block CRC ([B1D1])
  unsigned short int rx_block_crc; // Received block CRC ([B1D1])

  switch (getv())
  {
  case STX: break;
  case EOT: return (2);
  default: fwdput("*** rcvd invalid block-id", "");
           //trace(report, "rxyapp", "%d", i);
           return (0);
  }
  if (! (length = getv())) length = 256;
  waitfor(e_ticsfull);
  for (i = 0; i < length; i++)
  {
    c = getv();
    // CRCfbb of current YAPP block [B1D1]
    block_crc.update(c);
    // Checksum of the compressed file: no [D], no [B1]
    mail_checksum += c;
    // CRCthp of the compressed file: [D], but not [B1]
    mail_crcthp->update(c);
    // CRCfbb of compressed file: [B1]
    if (   recstatus
        || (i > (unsigned)(1 + 4 * !! b->fbboffset))) // CRC is not part of CRC
      mail_crcfbb->update(c);
    buf[i] = c;
  }
  waitfor(e_ticsfull);
  // Check CRCfbb of current block (only used if [B1D1] in SID)
  if ((b->opt & o_b1) && (b->opt & o_d1))
  {
    char low  = getv();               // !! DO     !! (ask DH3MB for details)
    char high = getv();               // !! NOT    !!
    rx_block_crc = low + (high << 8); // !! CHANGE !!
    if (rx_block_crc != block_crc.result)
    {
      fwdput("*** block CRCfbb error", "");
      return (0);
    }
  }
  timeout(m.fwdtimeout);
  t->last_input = ad_time();
  return (1);
}

/*---------------------------------------------------------------------------*/

char fbb::rx_yapp_end (void)
//*************************************************************************
//
//  Receives FBB-end-of-mail in YAPP-like protocol
//  (EOT is received within rx_yapp_data())
//  Returns 1, if the checksum/CRC is ok, 0 if not
//
//*************************************************************************
{
  if ((b->opt & o_d) && ! (b->opt & o_b1)) // [D], no [B1] => 2 bytes CRCthp
  {
    if ((getv() + getv() * 0x100) != mail_crcthp->result)
    {
      fwdput("*** file CRCthp error", "");
      return (0);
    }
  }
  else // no [D], or [B1] => 1 byte checksum
  {
    if((((unsigned char) getv() + mail_checksum) & 0xff) != 0)
    {
      fwdput("*** file checksum error", "");
      return(0);
    }
  }
  timeout(m.fwdtimeout);
  t->last_input = ad_time();
  return (1);
}

/*---------------------------------------------------------------------------*/

char fbb::rx_mails (char recvnum, want_status_t *want_status)
//*************************************************************************
//
//  Receives mails in FBB-format
//  Returns: 1 = everything was ok
//           0 = error occured (protocol error, etc.)
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "fbb:rx_mails");
  lastfunc(name);
  unsigned short int rx_crcfbb = 0; // received CRCfbb
  char *data = (char *) t_malloc(256, "fbbd");
                              // Buffer for received YAPP-block
  unsigned short int datalen; // Number of bytes received in a YAPP-block
  char recstatus;             // 0 = Receiving first YAPP-block
                              // 1 = Receiving non-first YAPP-block
                              // 2 = Reception complete
  handle packedfile;          // Filehandle for the compressed mail file
  char packedfname[25];       // Name of compressed mail file
  char unpackedfname[25];     // Name of uncompressed mail file
  char i;                     // Used to find a new filename for the
                              // compressed file
  int count = 0;              // Number of the mail we are receiving
  char c[LINELEN+1];

  while (recvnum)
  { // The lines of the proposal can't be invalid suddenly, as they
    // have been parsed in reply_proposal() before, so we will ignore
    // the return-value of parse_proposal() here
    parse_proposal(want_status->proptxt[count]);
    {
      snprintf(c, LINELEN, "fwd: rx $%s (%s%c%s < %s)",
        b->bid, b->ziel, *b->at ? '@' : ' ', atcall(b->at), b->herkunft);
      lastcmd(c);
      trace(report, "fwd-rx", "$%s (%s%c%s < %s)",
        b->bid, b->ziel, *b->at ? '@' : ' ', atcall(b->at), b->herkunft);
      fwdpara_t *ff = fpara();
      ff->rxf++;
    }
    if (! (b->opt & o_b)) // Partner does not support compressed forward :-(
    {
      fwdlog("---- ", "[mail title]", 'R');
      getline(b->betreff, BETREFFLEN, 1);
      fwdlog("---- ", "[text file]", 'R');
      update_mail();
      wantmail(NULL);
      savemail();
    }
    else
    {
      mail_crcfbb = new crcfbb(); // [B1]: use CRCfbb
      mail_crcthp = new crcthp(); // [D], no [B1]: use CRCthp
      mail_checksum = 0;          // no [D], or [B1]: use checksum
      fwdlog("---- ", "[mail header]", 'R');
      if (! rx_yapp_header())
      {
        fwdput("*** Mail header invalid", "");
        t_free(data);
        delete(mail_crcfbb);
        delete(mail_crcthp);
        return (0);
      }
      waitfor(e_ticsfull);
      fwdlog("---- ", "[mail body]", 'R');
      // Find a new filename for both - packed und unpacked file
      if (b->fbboffset) // (want_status->offset[count])
      { // check if offset is acceptable (not bigger than fragement)
        if (b->fbboffset > want_status->offset[count])
        {
          fwdput("*** Offset > filesize", "");
          t_free(data);
          delete(mail_crcfbb);
          delete(mail_crcthp);
          return (0);
        }
        // use old fragment
        snprintf(packedfname, sizeof(packedfname), FBBPATH "/%s%02d.lhb",
                 b->logincall, want_status->fragment[count]);
        strlwr(packedfname);
      }
      else
      {
        i = 0;
        do
        {
          i++;
          waitfor(e_ticsfull);
          snprintf(packedfname, sizeof(packedfname), FBBPATH "/%s%02d.lhb",
                                                     b->logincall, i);
          strlwr(packedfname);
          if (i == 99)
          {
            trace(serious, name, "no fn");
            t_free(data);
            delete(mail_crcfbb);
            delete(mail_crcthp);
            return (0);
          }
        }
        while (file_isreg(packedfname));
      }
      snprintf(unpackedfname, sizeof(unpackedfname), FBBPATH "/%s.fbb",
                                                     time2filename(0));
      strlwr(unpackedfname);
      //trace(report, name, " unpack fn %s", packedfname);
      if ((packedfile = s_open(packedfname, "lw+b")) != EOF)
      { // File will not be deleted on disconnect!
        // when not in resume, save header
        if (! want_status->offset[count])
        {
          header_t header;
          memset(&header, 0, sizeof(header_t));
          strcpy(header.signature, FBBHSIGN);
          strcpy(header.signatur2, FBBHSIGN);
          strcpy(header.version, FBBHVERS);
          strcpy(header.bid, b->bid);
          _write(packedfile, &header, sizeof(header_t));
        }
        else
        { // calculate CRC, seek position
          lseek(packedfile, (long) sizeof(header_t) - 2L, SEEK_SET); // Shit
          unsigned long int cur_offset = 0L;
          char data_b;
          while (cur_offset < b->fbboffset)
          {
            if (_read(packedfile, &data_b, 1))
            { // Calculate CRC of data_b
              // Use checksum or CRCthp depending on [D] in SID
              if ((b->opt & o_d) && ! (b->opt & o_b1))
                mail_crcthp->update(data_b); // CRC thp: [D], but not [B1]
              // CRCfbb: [B1]
              if ((b->opt & o_b1) && (cur_offset > 1))
                mail_crcfbb->update(data_b);
              cur_offset++;
            }
            else
            {
              trace(serious, name, "fragment corrupted");
              t_free(data);
              delete(mail_crcfbb);
              delete(mail_crcthp);
              return (0);
            }
          }
        }
        recstatus = 0; // We will soon receive the first YAPP-block :-)
        while (recstatus != 2)
        {
          waitfor(e_ticsfull);
          switch (rx_yapp_data(data, datalen, recstatus))
          {
          case 0: // an error occured
            s_close(packedfile);
            //store first part for resume
            if (! (b->opt & o_b1)) xunlink(packedfname);
            fwdlog("---- ", "[yapp error]", 'R');
            t_free(data);
            delete(mail_crcfbb);
            delete(mail_crcthp);
            return (0);
          case 1: // everything is fine
            if (! recstatus && (b->opt & o_b1)) // CRCfbb at beginning of file
            {
              rx_crcfbb = (char) *data + ((char) data[1] << 8);
              if (b->fbboffset) // Ignore 4 bytes of file on resume
                _write(packedfile, data + 6, datalen - 6);
              else
                _write(packedfile, data + 2, datalen - 2);
              recstatus = 1;
            }
            else _write(packedfile, data, datalen);
            break;
          case 2: // received EOT
            s_close(packedfile);
            fwdlog("---- ", "[end of mail]", 'R');
            if (rx_yapp_end())
            {
              if (! strcmp(b->ziel, "E") || ! strcmp(b->ziel, "M"))
              {
                save_em();
                xunlink(packedfname);
              }
              else
              {
                if ((b->opt & o_b1) && (mail_crcfbb->result != rx_crcfbb))
                {
                  fwdput("*** CRCfbb of file invalid", "");
                  trace(replog, name, "fcrc 0x%04X != rx 0x%04X",
                        mail_crcfbb->result, rx_crcfbb);
                  // The file is broken, remove it, next time
                  // we start from the very beginning :-(
                  xunlink(packedfname);
                  t_free(data);
                  delete(mail_crcfbb);
                  delete(mail_crcthp);
                  return (0);
                }
                char cc[LINELEN+1];
                snprintf(cc, LINELEN, "fwd: rx decomp $%s", b->bid);
                lastcmd(cc);
                huffcod huff(mail_crcfbb);
                if (huff.err) trace(fatal, name, "huffman error");
                if (huff.decode(packedfname, unpackedfname, sizeof(header_t)))
                {
                  trace(serious, name, "huff.decode error %s", packedfname);
                  xunlink(packedfname);
                  t_free(data);
                  delete(mail_crcfbb);
                  delete(mail_crcthp);
                  return(0);
                }
                xunlink(packedfname);
                if (! (b->inputfile = s_fopen(unpackedfname, "lrb")))
                {
                  trace(serious, name, "open %s error", unpackedfname);
                  t_free(data);
                  delete(mail_crcfbb);
                  delete(mail_crcthp);
                  return (0);
                }
                s_fsetopt(b->inputfile, 1); // remove file on disconnect
                b->oldinput = t->input;
                t->input = io_file;
                unsigned long int true_rx = b->rxbytes;
                unsigned long int true_tx = b->txbytes;
                lastcmd("fwd-rx: update");
                update_mail();
                wantmail(NULL);
                //char c[LINELEN+1];
                snprintf(c, LINELEN, "fwd: rx save $%s", b->bid);
                lastcmd(c);
                savemail(); // f is closed within savemail()
                b->rxbytes = true_rx;
                b->txbytes = true_tx;
                lastcmd("fwd-rx: saved");
                xunlink(unpackedfname);
              }
            }
            else
            {
              xunlink(packedfname);
              t_free(data);
              delete(mail_crcfbb);
              delete(mail_crcthp);
              return (0);
            }
            recstatus = 2; // Reception complete
            break;
          }
        }
      }
      else
      {
        trace(serious, name, "fopen %s errno=%d %s",
                             packedfname, errno, strerror(errno));
        t_free(data);
        delete(mail_crcfbb);
        delete(mail_crcthp);
        return (0);
      }
      delete(mail_crcfbb);
      delete(mail_crcthp);
    }
    tmpbidraus(count);
    count++;
    recvnum--;
    waitfor(e_ticsfull);
  }
  t_free(data);
  return (1);
}

/*---------------------------------------------------------------------------*/

#define FWDMACHINE "fbb:fwdmachine"

void fbb::fwdmachine (char send_)
//*************************************************************************
//
//  Manages a FBB-forward-session
//
//  send_: 0 = incoming connection (We first receive a proposal)
//         1 = outgoing connection (We first send a proposal)
//
//*************************************************************************
{
  char name[20];
  strcpy(name, FWDMACHINE);
  lastfunc(FWDMACHINE);
  want_status_t *want_status;
  char recvnum = 0;
  char *tosend;
  char sendnum = 0;
  char recvd_status_m[55];
  char *recvd_status = recvd_status_m;
  // longest possible status: !123456 7890
  int partner_had_mail = 1; // defines if partner had mail
  fbb_fwdstate_t fbb_state;
  int waitc;
  int wasindelaybefore = 0;

  want_status = (want_status_t *) t_malloc(sizeof(want_status_t), "fbbw");
  tosend = (char *) t_malloc(LINELEN * FBB_MAX_PROP, "fbbs");
  lastcmd("fwd: start");
  if (send_) fbb_state = fbb_send_propose;
  else fbb_state = fbb_recv_propose;
  while (fbb_state != fbb_end)
  {
    switch (fbb_state)
    {
    case fbb_send_propose:
         {
           b->job = j_fbb_send_propose;
           lastcmd("fwd: tx proposal");
           sendnum = 0;
           //chkforward makes very simple check, tx_prop checks in more detail
           if (chkforward(b->logincall) //OE3DZW do not send empty proposals
               && ((sendnum = tx_proposal(tosend, recvd_status)) != 0) )
           {
             wasindelaybefore = 0;
             fbb_state = fbb_send_mail;
           }
           else
           {
             if (partner_had_mail)
             {
               wasindelaybefore = 0;
               fwdput(FBB_FINE, "");
               fbb_state = fbb_recv_propose;
             }
             else
               fbb_state = fbb_send_delay;
           }
         } break;
    case fbb_send_mail:
         {
            b->job = j_fbb_send_mail;
            lastcmd("fwd: tx mails");
            if (tx_mails(tosend, sendnum, recvd_status))
              fbb_state = fbb_recv_propose;
            else
              fbb_state = fbb_error;
         } break;
    case fbb_send_delay: // Wait some time if there are no mails to send
         {
           b->job = j_fbb_send_delay;
           lastcmd("fwd: waiting");
           if (! wasindelaybefore && b->forwarding == fwd_standard)
           {
             if (! (fpara()->options & o_u))
             {
               waitc = 15; //wait max. 3min, check every 18s
               while (waitc--)
               {
                 wdelay(17730 - waitc);
                 //timeout(m.fwdtimeout);
                 //t->last_input=ad_time();
                 if (chkforward(b->logincall) && ! another_fwd_sender(b->logincall))
                   break;
               }
             }
           }
           else
           wdelay(1032);
           if (! wasindelaybefore
               || (chkforward(b->logincall) && ! another_fwd_sender(b->logincall)))
           {
             fbb_state = fbb_send_propose;
             wasindelaybefore = 1;
             wdelay(1032);
           }
           else
           {
             b->job = j_fbb_end;
             lastcmd("fwd: finished");
             fwdput(FBB_QUIT, "");
             fbb_state = fbb_end;
           }
         } break;
    case fbb_recv_propose:
         {
           b->job = j_fbb_recv_propose;
           lastcmd("fwd: rx proposal");
           // Receive proposal and send the reply
           switch (recvnum = reply_proposal(want_status))
           {
           case FBB_RECV_FINE: // Partner has sent "FF", he has got no mails to send
                  partner_had_mail = 0;
                  fbb_state = fbb_send_propose;
                  break;
           case FBB_RECV_QUIT: // Partner has sent "FQ", i.e. he wants to quit
                  fbb_state = fbb_end;
                  lastcmd("fwd: rx FQ");
                  break;
           case FBB_RECV_ERROR: // Proposal is invalid
                  fbb_state = fbb_error;
                  break;
           default: // We will receive 0..5 (recvnum) mails from the partner
                  partner_had_mail = 1;
                  fbb_state = fbb_recv_mail;
                  break;
           }
           // When we have received a correct proposal ("FF" or "FQ" respectively),
           // we can be sure, that the partner has received the mails we have
           // sent to him and so we may delete them from the forward-queue
           if (recvnum != FBB_RECV_ERROR && sendnum)
             erase_forwarded(sendnum, tosend, recvd_status);
         } break;
    case fbb_recv_mail:
         {
           b->job = j_fbb_recv_mail;
           lastcmd("fwd: rx mails");
           if (rx_mails(recvnum, want_status))
             fbb_state = fbb_send_propose;
           else
             fbb_state = fbb_error;
         } break;
    case fbb_error: // fall through
    default:
         {
           b->job = j_fbb_error;
           lastcmd("fwd: error");
           fwdput("*** FBB forwarding error", "");
           fbb_state = fbb_end;
         } break;
    }
    waitfor(e_ticsfull);
  }
  t_free(want_status);
  t_free(tosend);
  putflush();
  //check signature
  if (strcmp(name, FWDMACHINE)) trace(fatal, FWDMACHINE, "signature broken");
  lastcmd("fwd: terminating");
  wdelay(1000);
}

