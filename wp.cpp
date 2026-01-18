/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------
  White-Pages (including TheBox E/M-Mails)
  ----------------------------------------


  Copyright (C)       Florian Radlherr et al
                      Taubenbergstr. 32
                      D-83627 Warngau

                      Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved GNU GPL

 ***************************************************************/

//19990412 OE3DZW wp-update only if filesize > 24lines approx.
//19990621 OE3DZW old trigger-files will no longer be removed
//19990726 OE3DZW trigger file was not deleted in the internet 1.41a dist
//19991111 OE3DZW changed mycall to [1] (fwdmycall)
//20000119 OE3DZW increased lifetime of WP-mail
//20000312 DF3VI  call remeraser with b->herkunft, originbbs is wrong!
//20001004 DK2UI  incoming mybbs limit to age of 30 days
//20030720 hpk    added CB-BCMNET gateway functions
//20030809 hpk    in CB-BCMNET Versions WP-Mail is generated in every fwd-cycle


#include "baycom.h"


typedef enum w_file_t
{
  fbb_file,
  w_file,
  unknown_file
} w_file_t;

/*---------------------------------------------------------------------------*/

void add_emfile (char *zeile, char *frombox, char *file_flag, char *boardname,
                 char *at)
//*************************************************************************
//
//  Legt die angegebene Verwaltungszeile in den Forward-Queue-Files ab
//  die zu den Nachbarboxen in (boxenliste) gehoeren.
//
//  zeile: line to be saved
//  frombox: bbs message in line comes from (to be excluded)
//  file_flag: defines file: m -> old em-style, w -> new wp-style
//
//*************************************************************************
{
  char fwdfname[FNAMELEN+1];
  char boxcall[CALLEN+1];
  char boxcall_space[CALLEN+2];
  char *boxenliste = b->destboxlist;
  FILE *fwdfile;
  char name[20];
  strcpy(name, "add_emfile");
  lastfunc(name);

  //trace(report, name, "from: %s '%s'", frombox, zeile);
  strcpy(b->at, at);
  weiterleiten(0, boardname);
  subst1(boxenliste, '.', 0);
  while (boxenliste = nexttoken(boxenliste, boxcall, CALLEN), *boxcall)
  {
    strcpy(boxcall_space, boxcall);
    strcat(boxcall_space, " "); // sonst stristr Problem DB0EA/DB0EAM!
    if (   isforwardpartner(boxcall) != NOTFOUND
        && strcmp(frombox, boxcall)
        && ! stristr(zeile, boxcall_space))
    {
      strcpy(fwdfname, FWDPATH "/");
      strcat(fwdfname, file_flag);
      strcat(fwdfname, "_");
      strcat(fwdfname, boxcall);
      strcat(fwdfname, ".bcm");
      strlwr(fwdfname);
      fwdfile = s_fopen(fwdfname, "sat"); // fwdtrigger
      if (! fwdfile) // wenn es nicht gegangen ist, ist evtl.
      {              // das Directory noch gar nicht angelegt
        mkdir(FWDPATH);
        fwdfile = s_fopen(fwdfname, "sat");
      }
      if (fwdfile)
      {
        fprintf(fwdfile, "%s\n", zeile);
        s_fclose(fwdfile);
      }
      else
        trace(serious, name, "fopen %s errno=%d %s", fwdfname, errno,
                                                     strerror(errno));
      waitfor(e_reschedule);
    }
  }
}

/*---------------------------------------------------------------------------*/

int get_wcs (char *line)
//*************************************************************************
//
//  calculates simple line-checksum used in W-files
//
//*************************************************************************
{
  int wcs = 0;
  char *c;

  c = line;
  while (*c) wcs += *c++;
  return (wcs & 0xff);
}

/*---------------------------------------------------------------------------*/

static void wp_checksum (char *p)
//*************************************************************************
//
//  put simple line-checksum at begin of line
//
//*************************************************************************
{
  char *s;
  short checksum = 0;

  s = p + 3;
  while (*s) checksum += *s++;
  sprintf(p, "%02X", checksum & 0x00ff);
  p[2] = ' '; //replace 0 by space
}

/*---------------------------------------------------------------------------*/

void addwp_m (wpdata_t *wp)
//*************************************************************************
//
//  Add mybbs-info (<bbs>) for <call> to triggerfiles
//
//*************************************************************************
// used globals: b->logincall, b->at (modified), b->destboxlist(modified)
{
  char *bef;
  bef = b->line;

#ifdef __FLAT__
  //first call "old" thebox-function (backwards compatibility)
  snprintf(bef, BUFLEN, "M @ THEBOX < %s $%s %s %ld",
           wp->call, wp->bid, wp->bbs, (wp->mybbstime+ad_timezone()));
  add_emfile(bef, b->logincall, "M", "M", "$EM");
  // WPROT-Format
  snprintf(bef, BUFLEN, "   M %s %s %s %ld %s %d \"%s\" %s \"%s\"",
           wp->call, wp->bbs, wp->bid, (wp->mybbstime+ad_timezone()), wp->origin,
           (wp->hops) + 1, wp->name, *wp->zip ? wp->zip : "?",
           *wp->qth ? wp->qth : "?");
#else
  //first call "old" thebox-function (backwards compatibility)
  snprintf(bef, BUFLEN, "M @ THEBOX < %s $%s %s %ld",
           wp->call, wp->bid, wp->bbs, (wp->mybbstime-(5*HOUR)));
  add_emfile(bef, b->logincall, "M", "M", "$EM");
  // WPROT-Format
  snprintf(bef, BUFLEN, "   M %s %s %s %ld %s %d \"%s\" %s \"%s\"",
           wp->call, wp->bbs, wp->bid, (wp->mybbstime-(5*HOUR)), wp->origin,
           (wp->hops) + 1, wp->name, *wp->zip ? wp->zip : "?",
           *wp->qth ? wp->qth : "?");
#endif
  wp_checksum(bef);
  add_emfile(bef, b->logincall, "W", "M", "$WP");
}

/*---------------------------------------------------------------------------*/

void addwp_b (wpdata_t *wp)
//*************************************************************************
//
//  Add bbs-info to triggerfiles
//
//*************************************************************************
// used globals: b->logincall, b->at (modified), b->destboxlist(modified)
{
  char *bef;
  bef = b->line;

#ifdef __FLAT__
  snprintf(bef, BUFLEN, "   B %s %ld %s %s %s %s %ld %d",
           wp->hadr, wp->bversion, wp->bstatus, wp->sysopcall, wp->protocol,
           wp->hwaddress, (wp->mybbstime+ad_timezone()), wp->hops + 1);
#else
  snprintf(bef, BUFLEN, "   B %s %ld %s %s %s %s %ld %d",
           wp->hadr, wp->bversion, wp->bstatus, wp->sysopcall, wp->protocol,
           wp->hwaddress, (wp->mybbstime-(5*HOUR)), wp->hops + 1);
#endif
  wp_checksum(bef);
  add_emfile(bef, b->logincall, "W", "B", "$WP");
}

/*---------------------------------------------------------------------------*/

void addwp_e (char *at, char *originbbs, char *bid, char *bid_erase,
              char *erase_user, short hops, char *reason, char *logincall)
//*************************************************************************
//
//  Puts a triggerline into w_<boxcall>.bcm (Used for remote erase in
//  W-Format)
//
//*************************************************************************
{
  char *bef;
  bef = b->line;

  snprintf(bef, BUFLEN, "   E %s %s %s %s %s %d \"%s\"",
           at, originbbs, bid, bid_erase, erase_user, hops + 1, reason);
  wp_checksum(bef);
  add_emfile(bef, b->logincall, "W", "E", "$WP");
}

/*---------------------------------------------------------------------------*/

void addwp_r (wpdata_t *wp)
//*************************************************************************
//
//  Add routing-info to triggerfiles
//
//*************************************************************************
{
  char *bef;
  bef = b->line;

//  if (wp->hops < 2)
//  {
    // these are our direct neighbours, and we will prefer routing them
    // by using the bare link quality divided by 2 for comparision with
    // other routes
//    wp->qual = wp->qual / 2; verschoben nach unten, sonst
//    wird die quality mit der zeit immer kleiner!
//  }
  if (wp->hops > 1) //&& wp->qual != WPROT_MIN_ROUTING)
                    //WPROT_MIN_ROUTING 1:1 durchreichen, damit es sich verbreitet?
  {
    // laut spec Erhoehung der Qual um +100 und +10% bei hops+1
    wp->qual = wp->qual + (wp->qual / 10); // +10%
    wp->qual = wp->qual + 100; // +100
  }
  if (strcmp(wp->bbs, m.boxname))
  {
/*
    if (phantom)
    if (rupdate_hadr(wp)) // save received routing infos to hadr-database
    { // Phantom-Routings mit 15 Stunden fixem aging
#ifdef __FLAT__
      snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
           wp->bbs, wp->bversion, (wp->mybbstime + ad_timezone() - (15 * HOUR)),
           (wp->hops + 1), wp->qual);
#else
      snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
           wp->bbs, wp->bversion, (wp->mybbstime - (5 * HOUR) - (15 * HOUR)),
           (wp->hops + 1), wp->qual);
#endif
      wp_checksum(bef);
      add_emfile(bef, b->logincall, "W", "R", "$WP");
    }
    else
*/
    if (rupdate_hadr(wp)) // save received routing infos to hadr-database
    {
#ifdef __FLAT__
      snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
           wp->bbs, wp->bversion, (wp->mybbstime + ad_timezone()),
           (wp->hops + 1), wp->qual);
#else
      snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
           wp->bbs, wp->bversion, (wp->mybbstime - (5 * HOUR)),
           (wp->hops + 1), wp->qual);
#endif
      wp_checksum(bef);
      add_emfile(bef, b->logincall, "W", "R", "$WP");
    }
  }
  else
  { // eigene Box-Routing-Info ausgeben
#ifdef __FLAT__
    snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
         wp->bbs, wp->bversion, (wp->mybbstime + ad_timezone()),
         (wp->hops + 1), wp->qual);
#else
    snprintf(bef, BUFLEN, "   R %s %ld %ld %d %ld",
         wp->bbs, wp->bversion, (wp->mybbstime - (5 * HOUR)),
         (wp->hops + 1), wp->qual);
#endif
    wp_checksum(bef);
    add_emfile(bef, b->logincall, "W", "R", "$WP");
  }
}

/*---------------------------------------------------------------------------*/

void genwp_r (void)
//*************************************************************************
//
//  generate own routing-info to triggerfiles all 5 hours
//
//*************************************************************************
{
  static time_t lastsend;
  wpdata_t wp;

  if (ad_time() - lastsend < (5 * HOUR)) return;
  memset(&wp, 0, sizeof(wpdata_t));
  safe_strcpy(wp.bbs, m.boxname);
  wp.bversion = BVERSION;
  wp.hops = 0;
  wp.qual = 0;
  lastsend = wp.mybbstime = ad_time(); // UTC
  if (((wp.mybbstime / (5 * HOUR)) * (5 * HOUR)) == wp.mybbstime)
    wp.mybbstime++;
  addwp_r(&wp);
}

/*---------------------------------------------------------------------------*/

void wpgenheader (FILE *f)
//*************************************************************************
//
//  Generates a W header
//
//*************************************************************************
{
  char tmp[120];
  // Version
  snprintf(tmp, sizeof(tmp), "   V 10");
  wp_checksum(tmp);
  fprintf(f, "%s\n", tmp);
  // BBS-Info
  long bver = BVERSION;
#ifdef __FLAT__
  snprintf(tmp, sizeof(tmp), "   B %s %ld " BSTATUS " %s AX25 %s %ld 1",
           m.boxadress, bver, m.sysopcall, m.mycall[1], ad_time()+ad_timezone());
#else
  snprintf(tmp, sizeof(tmp), "   B %s %ld " BSTATUS " %s AX25 %s %ld 1",
           m.boxadress, bver, m.sysopcall, m.mycall[1], ad_time()-(5*HOUR));
#endif
  wp_checksum(tmp);
  fprintf(f, "%s\n", tmp);
  // Routing-Info
  genwp_r();
}

/*---------------------------------------------------------------------------*/

void gen_wpmail (char *call)
//*************************************************************************
//
//  Will generate a wp-mail out of data stored in the wp-file
//  This mail will be forwarded like any "standard" personal mail
//
//  Format of new mail: WP or W
//
//*************************************************************************
{
  lastfunc("gen_wpmail");
  char fwdfname[FNAMELEN+1];
  char fwdoldname[FNAMELEN+1];
  int fbbformat = ! (b->opt & o_w); // W or WP format
  FILE *fwdfile = NULL;
  long lines = 0;
  char tmpname[30];
  char boxcall_space[CALLEN+2];
  long mails = 0L;
  FILE *f = NULL;
  char *buf;
  char s[LINELEN+41];

  // do not try to generate files during reorg, it
  // can easily be done later...
  if (sema_test("purgereorg")) return;
  // create boards if not existing
  mkboard("/", "WP", 0);
  mkboard("/", "W", 0);
  // open new trigger file
  strcpy(fwdfname, FWDPATH "/W_");
  strcat(fwdfname, call);
  strcat(fwdfname, ".bcm");
  strlwr(fwdfname);

  // simple hack to get rid off _very_ big files
  // due to broken links or sw-bugs
  if (filesize(fwdfname) > 200000L) xunlink(fwdfname);
  fwdpara_t *ff = fpara();
  time_t ti = ad_time();
  if ((f = s_fopen(fwdfname, "srt")) != NULL) // check fwdtrigger
  {
    while (fgets(s, LINELEN+40, f))
      if (*s != ' ') mails++;
    s_fclose(f);
  }
#ifndef _BCMNET_FWD
  if (   (   mails > 50 // at least 50 lines
#else
  if (   (   mails > 10 // at least 10 lines
#endif
          || (ff->wprot_r_partner == 1
              && (ti - ff->lastwp) > (DAY / 48)) // or if R-Line Partner after startup or every 30 Min.
          || (ff->wprot_r_partner != 1
              && (ti - ff->lastwp) > 300)) // or if NOT R-Line Partner after startup or every 300 Min.
      && isforwardtime(ff->call, 1) // only when it is time for bulletins
      && (fwdfile = s_fopen(fwdfname, "srt")) != NULL)
  {
    ff->lastwp = ti;
    // remove old trigger file
    strcpy(fwdoldname, FWDPATH "/M_");
    strcat(fwdoldname, call);
    strcat(fwdoldname, ".bcm");
    strlwr(fwdoldname);
    xunlink(fwdoldname);
    buf = b->line;
    snprintf(tmpname, sizeof(tmpname), TEMPPATH "/%s", time2filename(0));
    FILE *wptmpfile = s_fopen(tmpname, "swt");
    if (! wptmpfile)
    {
      trace(serious, "gen_wpmail", "fopen %s", tmpname);
      s_fclose(fwdfile);
      return;
    }
    if (fbbformat)
    {
      fprintf(wptmpfile, "SP WP@%s", call);
      fprintf(wptmpfile, "\n" WPSUBJECT "\n");
    }
#ifndef _BCMNET_FWD
    else
    {
      fprintf(wptmpfile, "SP W@%s #4", call);
#else
    else
    {
      fprintf(wptmpfile, "SP W@%s #14", call); // increased lifetime
#endif
      fprintf(wptmpfile, "\n" WPROTSUBJECT "\n");
    }
    wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wpg");
    if (! wp) trace(tr_abbruch, "gen_wpmail", "no mem for wp");
    trace(report, "gen_wpmail", "%s type %s", call, fbbformat ? "fbb" : "w");
    if (! fbbformat) wpgenheader(wptmpfile);
    // copy data
    while (! feof(fwdfile))
    {
      if (fgets(b->line, BUFLEN - 1, fwdfile) != NULL)
      {
        buf[250] = 0;
        if (strlen(buf) < 20) continue; // skip empty lines
        if (fbbformat)
        {
          if (buf[3] == 'M' && wpparse_m(buf + 5, wp))
          {
            subst(wp->name, ' ', '-'); // replace spaces by dashes
            subst(wp->qth, ' ', '-');  // replace spaces by dashes
            expand_hadr(wp->bbs, 2);   // FBB needs full address
            if (strchr(wp->bbs, '.'))  // check for h-address
            {
              snprintf(buf, BUFLEN, "On %s %s/U @ %s zip %s %s %s\n",
                       datestr(wp->mybbstime - ad_timezone(), 8), wp->call,
                       wp->bbs, *wp->zip ? wp->zip : "?", wp->name,
                       *wp->qth ? wp->qth : "?");
              lines++;
              fputs(buf, wptmpfile);
            }
          }
        }
        else
        {
          lines++;
       //   fwdpara_t *ff = fpara();
//printf("ff->call: %s r-partner: %d\n",ff->call,ff->wprot_r_partner);
          strcpy(boxcall_space, m.boxname);
          strcat(boxcall_space, " "); // sonst stristr Problem DB0EA/DB0EAM!
          if (buf[3] != 'R'
              || (buf[3] == 'R' && stristr(buf, boxcall_space))
              || (buf[3] == 'R' && ff->wprot_r_partner == 1))
          {
//printf("buf: %s\n",buf);
            fputs(buf, wptmpfile);
          }
        }
      }
    waitfor(e_reschedule);
    }
    s_fclose(fwdfile);
    fprintf(wptmpfile, "\nnnnn\nimpdel\n");
    s_fclose(wptmpfile);
    xunlink(fwdfname);
    if (lines)
      fork(P_BACK | P_MAIL, 0, mbimport, tmpname);
    else
      xunlink(tmpname);
    t_free(wp);
  }
}

/*---------------------------------------------------------------------------*/

int wpparse_fbb (char *buf, wpdata_t *wp)
//*************************************************************************
//
// Parses a single WP-line (FBB/W0RLI)
// returns 1 on success, else 0
//
//*************************************************************************
{
#ifdef _BCMNET_FWD
  //CB-BCMNET: we can't support this because we do not have a bid
  return NO;
#else
  char *p;
  char yymmdd[7];
  char tmp[21];
  p = buf;
  lastfunc("wpparse_fbb");
  p = nexttoken(buf, tmp, 20);
  if (strcmp(tmp, "On")) return NO;
  p = nexttoken(p, yymmdd, 6);
  wp->mybbstime = yymmdd2ansi(yymmdd);
  if ((ad_time() - wp->mybbstime) > MAXAGE) return NO;
  p = nexttoken(p, tmp, 20);
  if (! strstr(tmp, "/U")) return NO;
  subst(tmp, '/', 0);
  if (strlen (tmp) > 6 || ! mbcallok(tmp)) return NO;
  strcpy(wp->call, tmp);
  if (*p != '@' || strlen(p) < 4) return NO;
  else p += 2;
  p = nexttoken(p, wp->bbs, HADRESSLEN);
  if (mbhadrok(wp->bbs) != 1) return NO;
  p = nexttoken(p, tmp, 20);
  if (! strcmp(tmp, "zip"))
  {
    p = nexttoken(p, wp->zip, ZIPLEN);
    if (! *wp->zip) strcpy(wp->zip, "?");
    p = nexttoken(p, wp->name, NAMELEN);
    if (! *wp->name) strcpy(wp->name, "?");
    p = nexttoken(p,wp->qth, QTHLEN);
    if (! *wp->qth) strcpy(wp->qth, "?");
  }
  else
  {
    strcpy(wp->zip, "?");
    strcpy(wp->name, "?");
    strcpy(wp->qth, "?");
  }
  strcpy(wp->logincall, "?");
  strcpy(wp->origin, "WP");
  wp->hops = 1;
  strcpy(wp->bid, newbid()); //jede Mail eine neue bid??
  return OK;
#endif
}

/*---------------------------------------------------------------------------*/

int wpparse_v (char *buf, long &protnum)
//*************************************************************************
//
//  Parses version line
//  returns 1 on success
//
//*************************************************************************
{
  protnum = atol(buf);
  if (protnum) return OK;
  return NO;
}

/*---------------------------------------------------------------------------*/

int wpparse_b (char *buf, wpdata_t *wp)
//*************************************************************************
//
//  Parses bbs line
//  returns 1 on success
//
//*************************************************************************
{
  char *p;
  char tmp[21];

  p = nexttoken(buf, wp->hadr, HADRESSLEN);
  if (mbhadrok(wp->hadr) != 1) return NO;
  p = nexttoken(p, tmp, 20);
  wp->bversion = atol(tmp);
  if (! wp->bversion) return NO;
  p = nexttoken(p, wp->bstatus, BSTATUSLEN);
  p = nexttoken(p, wp->sysopcall, CALLEN);
  p = nexttoken(p, wp->protocol, BPROTOLEN);
  p = nexttoken(p, wp->hwaddress, BHWADDRESSLEN);
  p = nexttoken(p, tmp, 20);
#ifdef __FLAT__
  wp->mybbstime = atol(tmp) - ad_timezone(); // UTC
#else
  wp->mybbstime = atol(tmp) + (5* HOUR); // UTC
#endif
  if (! wp->mybbstime) return NO;
  p = nexttoken(p, tmp, 20);
  wp->hops = atoi(tmp);
  if (! wp->hops) return NO;
  return OK;
}

/*---------------------------------------------------------------------------*/

int wpparse_m (char *buf, wpdata_t *wp)
//*************************************************************************
//
//  Parses mybbs line
//  returns 1 on success
//
//*************************************************************************
{
//sample m-line (including checksum and flag)
//0B M DGT274 DBO274.#NRW.DEU.EU BADDBO27400Q 1065899763 DGT274 1 "Markus" 59302 "Oelde"
  char *p, tmp[21];

  p = nexttoken(buf, wp->call, CALLEN);
  if (! mbcallok(wp->call)) return NO;
  p = nexttoken(p, wp->bbs, HADRESSLEN);
  if (mbhadrok(wp->bbs) != 1) return NO;
  p = nexttoken(p, wp->bid, BIDLEN);
  if (strlen(wp->bid) < 5) return NO;
  p = nexttoken(p, tmp, 20);
#ifdef __FLAT__
  wp->mybbstime = atol(tmp) - ad_timezone(); // UTC
#else
  wp->mybbstime = atol(tmp) + (5 * HOUR); // UTC
#endif
  if (! wp->mybbstime || (ad_time() - wp->mybbstime) > MAXAGE) return NO;
  p = nexttoken(p, wp->origin, CALLEN);
  p = nexttoken(p, tmp, 20);
  wp->hops = atoi(tmp);
  if (! wp->hops) return NO;
  p = nexttoken(p, wp->name, NAMELEN);
  if (*wp->name == '\"') strcpy(wp->name, "?"); //ignore names with bad quotes
  p = nexttoken(p, wp->zip, ZIPLEN);
  if (! *wp->zip) strcpy(wp->zip, "?");
  p = nexttoken(p, wp->qth, QTHLEN);
  if (! *wp->qth) strcpy(wp->qth, "?");
  strcpy(wp->logincall, "?");
#ifdef _BCMNET_FWD
  if (is_bcmnet_bid(wp->bid, 1, wp->call)==1)
    return NO; //ignore MYBBS correction entrys
#endif
#ifdef _BCMNET_GW
  //Maybe users MyBBS is protected by the CB-BCMNET login concept?
  if (is_bcmnet_mybbs(wp->call, wp->bid, wp->bbs, wp->mybbstime, wp->origin,
                      wp->name, wp->hops) == 1) return NO;
#endif
  return OK;
}

/*---------------------------------------------------------------------------*/

int wpparse_e (char *buf, char *at, char *bid, char *originbbs,
               char *bid_erase, char *origin, short &hops, char *comment)
//*************************************************************************
//
//  Parses erase line
//  returns 1 on success
//
//*************************************************************************
{
  char *p, tmp[21];

  p = nexttoken(buf, at, HADRESSLEN);
  p = nexttoken(p, originbbs, CALLEN);
  p = nexttoken(p, bid, BIDLEN);
  if (strlen(bid) < 5) return NO;
  p = nexttoken(p, bid_erase, BIDLEN);
  if (strlen(bid_erase) < 5) return NO;
  p = nexttoken(p, origin, CALLEN);
  p = nexttoken(p, tmp, 20);
  hops = atoi(tmp);
  if (! hops) return NO;
  nexttoken(p, comment, 99);
  return OK;
}

/*---------------------------------------------------------------------------*/

int wpparse_r (char *buf, wpdata_t *wp)
//*************************************************************************
//
//  Parses routing line
//  returns 1 on success
//
//*************************************************************************
{
  char *p, tmp[21];
  char lbuf[80];

  p = nexttoken(buf, wp->bbs, HADRESSLEN);
  if (mbhadrok(wp->bbs) != 1) return NO;
  p = nexttoken(p, tmp, 20);
  wp->bversion = atol(tmp);
  if (! wp->bversion) return NO;
  p = nexttoken(p, tmp, 20);
#ifdef __FLAT__
  wp->mybbstime = atol(tmp) - ad_timezone(); // UTC
#else
  wp->mybbstime = atol(tmp) + (5 * HOUR); // UTC
#endif
  if ((ad_time() - wp->mybbstime) < 0) return NO; // keine Zukunftsroutings
  if (! wp->mybbstime) return NO;
  p = nexttoken(p, tmp, 20);
  wp->hops = atoi(tmp);
  if (! wp->hops || wp->hops > 50) return NO;
  p = nexttoken(p, tmp, 20);
  wp->qual = atol(tmp);
  if (wp->qual == 0) wp->qual = 100;
  if (wp->qual > WPROT_MIN_ROUTING) return NO;
//  if (! wp->qual) return NO;
  if (*p) return NO; // must be empty
  sprintf(lbuf, "G %-6.6s %-6.6s %s (%ld) %ld %-3d %-5ld\n",
                wp->bbs, b->logincall, datestr(wp->mybbstime, 12),
                wp->mybbstime, wp->bversion, wp->hops, wp->qual);
  wprotlog(lbuf, wp->bbs);

  fwdpara_t *ff = fpara();
  if (b->boxtyp == b_dp) // momentan das Weiterleiten von ACTIVE ROUTING an
                         // DPBoxen ausschliessen, solange diese mit den
                         // Informationen nicht 100% klarkommen
  {
    ff->wprot_r_partner = 0;
  }
  else
  {
    ff->wprot_r_partner = 1;
  }
  // testweise fuer HB9EAS wprot freigeben
#ifdef HB9EAS_DEBUG
  if (m.hb9eas_debug && ! strcmp(b->logincall, "HB9EAS"))
    ff->wprot_r_partner = 1;
#endif
  return OK;

}

/*---------------------------------------------------------------------------*/

void scan_wp (void)
//*************************************************************************
//
//  Check if incoming mail is valid
//
//*************************************************************************
// Used globals: b->herkunft, b->ziel,b->betreff,b->at,b->mailpath
// m.boxname,b->line (modified)
{
  char name[20];
  w_file_t ftype = unknown_file;
  int protok = 1;
  long line = 0;
  FILE *wpfile;

  strcpy(name, "scan_wp");
  lastfunc(name);
  // check if this mail is an wp-mail
  if (   strcmp(b->uplink, "Import")
      && ! (b->forwarding == fwd_standard)
      && b->mailtype == 'P'
      && ! strcmp(b->herkunft, b->frombox)
      && (! strcmp(atcall(b->at), m.boxname)) || (! *b->at))
  {
    if (! strcmp(b->ziel, "WP") && !(b->opt & o_w)) ftype = fbb_file;
    if (! strcmp(b->ziel, "W")  &&  (b->opt & o_w)) ftype = w_file;
    //subject of mail is not checked
    if (ftype == unknown_file) return;
    markerased('W', 0, 0);
    char *buf = (char *) t_malloc(WPBUFSIZE + 1, "wp");
    wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wps");
    trace(report, name, "scan %s", b->mailpath);
    if ((wpfile = s_fopen(b->mailpath, "srt")) != NULL)
    {
      while (! feof(wpfile) && protok)
      {
        if (! fgets(buf, WPBUFSIZE, wpfile)) continue;
        line++;
        if (line < 7 || buf[2] != ' ') continue; //skip mail header
        buf[WPBUFSIZE] = 0;
        subst(buf, LF, 0);
        if (strlen(buf) < 6) continue; //skip empty lines
        if (ftype == fbb_file)
        {
          if (wpparse_fbb(buf, wp))
          {
            if (*wp->name && *wp->name != '?') set_name(wp->call, wp->name);
            if (*wp->qth && *wp->qth != '?') set_qth(wp->call, wp->qth);
            if (*wp->zip && *wp->zip != '?') set_zip(wp->call, wp->zip);
            set_mybbs(wp->call, wp->bbs, wp->mybbstime - DAY,
                      wp->bid, wp->origin, wp->hops, wp->name);
          }
        }
        else
        {
          int wcs = 0;
          sscanf(buf, "%02X", &wcs);
          char *pbuf = buf + 3;
        //  if (wcs != get_wcs(pbuf))
        //  {
        //    trace(report, name, "checksum error");
        //    continue;
        //  }
          if (*pbuf == 'V')
          {
            long protnum = 0;
            if (wpparse_v(buf + 5, protnum) && protnum <= BPROTMAX)
            {
              //trace(report, name, "v %ld", protnum);
            }
            else protok = 0;
          }
          if (*pbuf == 'B')
          {
            //wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wpb");
            if (wpparse_b(buf + 5, wp))
            //store hadr-information in local data base
            //check if information is new and update is needed
            if (wpupdate_hadr(wp)) addwp_b(wp);
            //t_free (wp);
          }
          if (*pbuf == 'M')
          {
            if (wpparse_m(buf + 5, wp))
            {
              if (*wp->name && *wp->name != '?') set_name(wp->call, wp->name);
              if (*wp->qth && *wp->qth != '?') set_qth(wp->call, wp->qth);
              if (*wp->zip && *wp->zip != '?') set_zip(wp->call, wp->zip);
              set_mybbs(wp->call, wp->bbs, wp->mybbstime,
                        wp->bid, wp->origin, wp->hops, wp->name);
            }
          }
          if (*pbuf == 'E')
          {
            char at[HADRESSLEN + 1];
            char bid[BIDLEN + 1];
            char bid_erase[BIDLEN + 1];
            char origin[CALLEN + 1];
            char *comment;
            comment = (char *) t_malloc(100, "wpe");
            short hops;
            if (wpparse_e(buf + 5, at, bid, b->herkunft,
                          bid_erase, origin, hops, comment))
            {
              remeraser(at, bid, b->herkunft,
                        bid_erase, b->logincall, origin, hops, comment);
            }
            t_free(comment);
          }
          if (*pbuf == 'R')
          {
            if (wpparse_r(buf + 5, wp))
            {
              if (strcmp(wp->bbs, m.boxname))
              {
                wprotlog("R ", wp->bbs);
                addwp_r(wp); // R-Meldung an Nachbarboxen durchreichen
              }
            }
          }
        }
      waitfor(e_reschedule);
      }
      s_fclose(wpfile);
    }
    t_free(wp);
    t_free(buf);
  }
}

/*---------------------------------------------------------------------------*/

void remeraser (char *at, char *bid, char *herkunft, char *bid_erase,
                char *logincall, char *erase_user, short hops, char *reason)
//*************************************************************************
//
//  Remote erase of mails
//
//*************************************************************************
{
  char *bidname;
  char name[20];
  char s[LINELEN+1];

  strcpy(name, "remerase");
  lastfunc(name);
  //trace(report, name, "erase $%s<%s (from $%s)", bid_erase, at, bid);
  skip(bid_erase);
  if (*bid_erase)
  {
    bid_erase[BIDLEN] = 0; // limit max. length
    if (! bidvorhanden(bid) && ! tmpbidvorhanden(bid))
    {
      bidrein(bid, "E/M");
      snprintf(s, LINELEN, "E @ %s < %s $%s %s", at, herkunft,
                                                 bid, bid_erase);
      add_emfile(s, logincall, "M", "E", "$EM");
      addwp_e(at, herkunft, bid, bid_erase, erase_user, hops,
              reason, logincall);
    }
    bidname = bidvorhanden(bid_erase);
    if (bidname && *bidname && strcmp(bidname, "E/M")
        && strchr(bidname, '/') // syntax-check
       )
    {
      strcpy(s, bidname);
      if (setfullpath(s))
      {
        trace(report, name, "%s ($%s)", s, bid_erase);
        markerased('L', 0, 0); //swapped commands
      }
    }
    else
      trace(report, name, "not found %s", bid_erase);
  }
}

/*---------------------------------------------------------------------------*/
