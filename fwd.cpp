/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------------------------
  Allgemeine Forward-Ein- und Ausgabe sowie Adressierungsauswertung
  -----------------------------------------------------------------


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
//19990726 OE3DZW fixed security bug
//19990817 OE3DZW added JJ's code, added fwdoption -l
//19990918 OE3DZW added Jan's changes: filefwd
//19991028 Jan    changes for telnetfwd
//20000101 OE3DZW fixed y2k-problem with DieBox-PW in S&F (100 -> 00)
//         dk2ui  (dzw) st f with em/wp
//20000111 OE3DZW activated fwd-by-size again, added sema-locks
//19991127 DF3VI  added savefwdlist(), fwdlistedit(): edit fwd.bcm
//19991129 DF3VI  reinvented E/M in "st f", summary also in Kb, Mb
//19991212 DF3VI  changed structure of initfwdlist etc.
//19991221 DF3VI  changed weiterleiten for new fwd-structure
//19991227 DF3VI  if weiterleiten == unbekannt, take b->at from u.mybbs
//20000402 DF3VI  added HOLD-features to add_fwdfile()
//                needed changes in fwd-password-routines!
//20000518 DK2UI  dest_idx array from stack to malloc
//20000520 DK2UI  weiterleiten4() complete revised
//20000623 DK2UI  hold with reject corrected
//20000818 DK2UI  send file is closed in scanheader()
//20001113 DK2UI  use of new function log_entry for fwdlog
//20010216 DF3VI  fixed bugs in fwdlog and sidauswert
//20020110 DH8YMB RAUSCALL auf 15Zeichen statt 10, TRACE-Hinweise bei
//                waitconnect() hinzu!
//20020723 JJ     Unterdrueckung BIN/7+ Mails in Forward verfeinert
//20021031 hpk    Sending Z-Sign in FWD-SID for a connection with the CB-BCMNET
//20030101 hpk    if a mail is coming from a guest, the mail will be set on hold
//                this action will be active if searchreject returns 'Z'.
//20030104 hpk    fix: autorouter can now route with the full-destination, if
//                neccesary.
//20030326 DH8YMB Funktion checkdouble_fwdfile hinzu
//20030416 DH8YMB fix: Autorouter may crashed if tempdest was uninitialized
//20030418 DH8YMB checkdouble_fwdfile deaktiviert (zu schlechte Performance),
//                dafuer Funktion remove_oldentry hinzu
//20030809 hpk    added CB-BCMNET gateway functions
//20030809 hpk    some changes in HOLD for CB-BCMNET login-concept


#include "baycom.h"

//hadr.cpp
//extern void uclose (void);
//extern void hadr_tryopen (void);
//extern int  loadhadr (char *hadrcall, hadr_t *u, int anlegen);


/*---------------------------------------------------------------------------*/

#define DUMMY "DUMMY"
#define MIN_TRIG_LEN    11 // fwd file

 unsigned fwdpartners = 0;
 unsigned fwddestlen = 0;
 unsigned fwddest = 0;
#ifdef __FLAT__
 #ifdef __LINUX__
  static unsigned long long *dest_idx = NULL;
  #define LONG_ZERO 0LL
  #define LONG_ONE  1LL
 #else // Win32
  static __int64 *dest_idx = NULL;
  #define LONG_ZERO 0I64
  #define LONG_ONE  1I64
 #endif
#else
  static unsigned long *dest_idx = NULL;
  #define LONG_ZERO 0L
  #define LONG_ONE  1L
#endif
 static fwdpara_t *fp = NULL; // dynamisch bei "new" allocieren,
 static char *boxdest = NULL; // kein Datensegmentplatz verschwenden!
// boxdest = Liste mit allen Zielbezeichnern, in der Reihenfolge des
// Forwardfiles, jeweils durch ein Blank von einander getrennt. Durch
// dest_idx[] kann festgestellt werden, zu welchen Boxen das jeweilige Ziel
// gehoert.

/*---------------------------------------------------------------------------*/

int waitfwdprompt (char *buf)
//*************************************************************************
//
//  Wartet auf ein Forward-Prompt (Wird benutzt beim SID-Austausch
//  und beim RLI-Forwarding)
//
//  Rueckgabe:
//    0..Linkfehler
//    1..Prompt > (Normales Prompt)
//    2..CRC-Fehler-BIN-Prompt
//
//*************************************************************************
{
  lastfunc("waitfwdprompt");
#ifdef _FILEFWD
  if (b->forwarding == fwd_file) return 1;
#endif
  do
  {
    fwdget(buf, 118);
    if (stristr(buf, "RECONNECT")) return 0;
    if (stristr(buf, "FAILURE")) return 0;
    if (strstr(buf, "|>")) return 2;
  }
  while (buf[strlen(buf) - 1] != '>');
  return 1;
}

/*---------------------------------------------------------------------------*/

int isforwardpartner (char *call)
//*************************************************************************
//
//  Gibt zurueck, ob das uebergebene Call ein eingetragener
//  Forwardpartner ist.
//
//*************************************************************************
{
  unsigned int i = 0;
  if (! strcmp(call, DUMMY)) return NOTFOUND;
  while (i < fwdpartners)
  {
    if (! strcmp(fp[i].call, call)) return i;
    else i++;
  }
  return NOTFOUND;
}

/*---------------------------------------------------------------------------*/

char another_fwd_sender (char *call)
//*************************************************************************
//
//  Checks, if we are sending files in another forward-session
//
//*************************************************************************
{ // eingelogt ignores own (running) task
  return (   eingelogt(call, j_fwdsend, 0)
          || eingelogt(call, j_fwdsendf, 0)
          || eingelogt(call, j_fwdlink, 0)
          || eingelogt(call, j_fbb_recv_propose, 0)
          || eingelogt(call, j_fbb_recv_mail, 0) // bei 5er-wechsel kein
                                                 // Doppel-fwd noetig!
          || eingelogt(call, j_fbb_send_propose, 0)
          || eingelogt(call, j_fbb_send_mail, 0)
          || eingelogt(call, j_fbb_send_delay, 0)
          || eingelogt(call, 0, 1) > 1);
}

/*---------------------------------------------------------------------------*/

fwdpara_t *fpara (void)
//*************************************************************************
//
//  Gibt einen Zeiger auf die forward-Parameterstruktur zurueck
//  Beim User-Forwarding wird online ein Dummy befuellt
//
//*************************************************************************
{
  lastfunc("fpara");
  int box = isforwardpartner(b->logincall);
  fwdpara_t *ff;

  if (box != NOTFOUND) return &fp[box];
  if (b->forwarding == fwd_none)
    return fp; //df3vi: wird in mbdir fuer dir_outstanding_all-ersatz benutzt
  if (! b->fwd_parameter)
    b->fwd_parameter = t_malloc(sizeof(fwdpara_t), "fwdp");
  ff = (fwdpara_t *) b->fwd_parameter;
  memset(ff, 0, sizeof(fwdpara_t));
  strcpy(ff->call, b->logincall);
  strcpy(ff->concall, get_ufwd(ff->call));
  if (*ff->concall == 1) *ff->concall = 0; //user has disabled userfwding
  strcpy(ff->times, "AAAAAAAAAAAAAAAAAAAAAAAA");
  ff->connectbar = 1;
  ff->nolink = 0;
  ff->n_conok = 0;
  ff->n_login = 0;
  ff->lastwp = 0;
  ff->txf = 0;
  ff->rxf = 0;
  ff->wprot_r_partner = 0;
  ff->current_routing_quality = 0;
  ff->routing_quality = 0;
  ff->routing_txf = 0;
  ff->lastmeasure = 0;
  ff->lastwpr = 0;
  ff->sidrtt = 0;
  ff->timeout_cnt = 0;
  ff->ssid = m.fwdssid;
  if (b->forwarding == fwd_standard)
    trace(fatal, "fpara", "%s unknown fwd params", b->logincall);
  return ff;
}

/*---------------------------------------------------------------------------*/

void fwdlog (char *s1, char *s2, char dir)
//*************************************************************************
//
//  Schreibt ggf (nur wenn konfiguriert) jede FWD-Zeile in ein
//  zum Loginrufzeichen passendes Logfile
//
//*************************************************************************
{
  char buf[120];
  char filename[13];

  if (m.fwdtrace)
  {
    if (m.fwdtrace == 2 && ! (fpara()->options & o_t)) return;
    sprintf(filename, "t_%s.bcm", b->logincall);
    strlwr(filename);
    sprintf(buf, "%2d%c %-.80s", t->taskid, dir, s1);
    subst1(buf, CTRLZ, 0);
    strcat(buf, s2);
    subst1(buf, CTRLZ, 0);
    log_entry(filename, buf);
  }
}
/*---------------------------------------------------------------------------*/

void fwdput (char *s1, char *s2)
//*******************************************************************
//
//
//*******************************************************************
{
  putf("%s%s\n", s1, s2);
  fwdlog(s1, s2, 'S');
}

/*---------------------------------------------------------------------------*/

void fwdget (char *s, int maxlen)
//*******************************************************************
//
//
//*******************************************************************
{
  getline(s, maxlen, 1);
  timeout(m.fwdtimeout);
  fwdlog(s, "", 'R');
}

/*---------------------------------------------------------------------------*/

static int scanforwardoption (char *opt, int boxindex)
//*******************************************************************
//
//
//*******************************************************************
{
  lastfunc("scanforwardoption");

  if (*opt == '-' && isalpha(opt[1]))
  {
    fp[boxindex].options |= (1L << (toupper(opt[1]) - 'A'));
    switch (opt[1])
    {
      case 'B': fp[boxindex].maxbytes = atol(opt + 2); break;
#ifdef _LCF
      case 'C': fp[boxindex].fwdcallformat = atoi(opt + 2); break; // JJ
#endif
    }
    return 1;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

#ifndef _AUTOFWD
static
#endif
void strip_hadr (char *header)
//*******************************************************************
//
//
//*******************************************************************
{
  lastfunc("strip_hadr");
  int i = strlen(header) - 1;
  int j = strlen(m.boxadress) - 1;

  while (i >= 0 && j >= 0 && m.boxadress[j] == header[i])
  {
    i--;j--;
  }
  i++;
  while (header[i] != '.' && header[i]) i++;
  header[i] = 0;
}

/*---------------------------------------------------------------------------*/

static void erase_igate (char *connectpfad)
//*******************************************************************
//
// loescht "IGATE" aus dem Connectpfad-String
//
//*******************************************************************
{
  int i = 0;
  int startpos, endpos = 0;
  int pos = strpos(connectpfad, "IGATE");
  if (pos != -1)
  {
    int laenge = strlen(connectpfad);
    while (connectpfad[pos+i] != ' ' && ((pos+i) <= laenge))
    {
      endpos = pos + i;
      i++;
    }
    if (pos > 3
        && connectpfad[pos-3] == ' '
        && connectpfad[pos-2] == '/'
        && connectpfad[pos-1] == ' ')
      startpos = pos - 3;
    else
    {
      if (pos > 1
          && connectpfad[pos-1] == ' ') startpos = pos - 1;
      else startpos = pos;
    }
    for (i = 0 ; i <= (laenge-(endpos-startpos)) ; i++)
    {
      if (i >= startpos)
        connectpfad[i] = connectpfad[i+(endpos-startpos)+1];
      else
        connectpfad[i] = connectpfad[i];
    }
    connectpfad[i] = 0;
  }
}

/*---------------------------------------------------------------------------*/

void initfwdlist (void)
//*************************************************************************
//
//  Legt die Forwardliste an. Die Liste wird aus einem File eingelesen
//  und bleibt dann staendig im Speicher liegen.
//
//*************************************************************************
{
  char name[20];
  FILE *f;
  char s[BLEN];
  char word[MAXPARSE];
  char eintrag[HADRESSLEN+1];
  char *dest;
  int boxindex = 0, destindex = 0, anzdest = 0, line = 0, i;
  fwdpara_t *ff;

  strcpy(name, "initfwdlist");
  lastfunc(name);
#ifdef __FLAT__
 #ifdef __LINUX__
  if (! dest_idx)
    dest_idx =
    (unsigned long long *) t_malloc(MAXDEST * sizeof(unsigned long long),
                                     "*fid");
 #else //Win32
  if (! dest_idx)
    dest_idx =
    (__int64 *) t_malloc(MAXDEST * sizeof(_int64), "*fid");
 #endif
#else
  if (! dest_idx)
    dest_idx =
    (unsigned long *) t_malloc(MAXDEST * sizeof(unsigned long), "*fid");
#endif
  if (! fp)
    fp = (fwdpara_t *) t_malloc(MAXFWD * sizeof(fwdpara_t), "*fwd");
  if (! boxdest)
    boxdest = (char *) t_malloc(MAXDESTLEN, "*fds");
  if (! dest_idx || ! fp || ! boxdest)
  {
    if (! dest_idx) trace(tr_abbruch, name, "no dest_idx");
    if (! fp) trace(tr_abbruch, name, "no fp");
    if (! boxdest) trace(tr_abbruch, name, "no boxdest");
  }
  for (i = 0; i < MAXDEST; i++) dest_idx[i] = LONG_ZERO;
  memset(fp, 0, MAXFWD * sizeof(fwdpara_t));
  *boxdest = 0;
  ff = &fp[0];
  fwdpartners = 0;
  fwddestlen = 0;
  fwddest = 0;
  if ((f = s_fopen(FWDLISTNAME, "srt")) != NULL)
  {
    while (fgets(s, sizeof(s) - 1, f))
    {
      line++;
      subst1(s, ';', 0);
      if (strlen(s) > 2)
      {
        i = (*s == ' ');
        strupr(s);
        parseline(s, word);
        if (i)
        {
          if (! boxindex) goto error;  // erster Eintrag: Boxcall fehlt
          else
          {
            i = 0;
            while (word[i])
            {
              waitfor(e_ticsfull);
              char dstb[HADRESSLEN+1];
              strncpy(dstb, s + word[i], HADRESSLEN);
              dstb[HADRESSLEN] = 0;
              strip_hadr(dstb);
              if (*dstb && ! scanforwardoption(dstb, boxindex - 1))
              {
                dest = boxdest;
                for (destindex = 0; destindex < anzdest; destindex++)
                {
                  dest = nextword(dest, eintrag, HADRESSLEN);
                  if (! stricmp(eintrag, dstb))
                  { //Eintrag existiert schon -> Index setzen
                    dest_idx[destindex] |= (LONG_ONE << (boxindex - 1));
                    break;
                  }
                }
                if (destindex >= (MAXDEST - 1))
                {
                  trace(serious, name, ms(m_fwd_toomanydest));
                  if (*b->logincall) putf(ms(m_fwd_toomanydest));
                  break;
                }
                if (destindex >= anzdest)
                { //neuer Eintrag
                  if ((fwddestlen + strlen(dstb) + 1) >= MAXDESTLEN)
                  {
                    trace(serious, name, ms(m_fwd_toomanydest));
                    if (*b->logincall) putf(ms(m_fwd_toomanydest));
                    break;
                  }
                  strcat(boxdest, dstb);
                  strcat(boxdest, " ");
                  dest_idx[anzdest] = (LONG_ONE << (boxindex - 1));
                  anzdest++;
                  fwddestlen = strlen(boxdest);
                }
              }
              i++;
            }
          }
        }
        else
        {
          *ff->call = 0;
          *ff->concall = 0;
          ff->sidrtt = 0;
          *ff->times = 0;
          ff->n_conok = 0;
          ff->lastwp = 0;
          ff->n_login = 0;
          ff->txf = 0;
          ff->rxf = 0;
          ff->wprot_r_partner = 0;
          ff->current_routing_quality = 0;
          ff->routing_quality = 0;
          ff->routing_txf = 0;
          ff->lastmeasure = 0;
          ff->lastwpr = 0;
          ff->timeout_cnt = 0;
          ff->connectbar = 0;
          ff->nolink = 0;
          if (   strlen(s + word[0]) <= CALLEN
              && (mbcallok(s + word[0]) || ! stricmp(s + word[0], DUMMY)))
            strcpy(ff->call, s + word[0]);
          else goto error;                      // Boxcall ungueltig
          if (word[1])
          {
            if (strlen (s + word[1]) == 24) strcpy(ff->times, s + word[1]);
            else if (*(s + word[1]) == '-')
              strcpy(ff->times, "APAAAAAAAAAAAAAAAAAAAAAA");
            else goto error;
          }
          i = 2;
          while (word[i])
          {
            if (   (strlen(s + word[i]) <= 14
                || (! strncasecmp("=", s + word[i], 1)
                     && strlen(s + word[i]) <= 40)
#ifdef _AX25K
                || (! strncasecmp("ax", s + word[i], 2)
                     && strlen(s + word[i]) <= 15)
#endif
#ifdef _TELNETFWD
                || ! strncasecmp("telnet:", s + word[i], 7)
#endif
#ifdef _FILEFWD
                || ! strncasecmp("file:", s + word[i], 5)
#endif
#ifdef _TELEPHONE
                || (! strncasecmp("tty:", s + word[i], 3)
                     && strlen(s + word[i]) <= 20)
#endif
                   )
                && (strlen(ff->concall) + strlen(s+word[i])+1)
                    < sizeof(ff->concall))
            {
              strcat(ff->concall, s + word[i]);
              strcat(ff->concall, " ");
            }
            else goto error; // Connectcall zu lang
            i++;
          }
          if (*ff->concall)
          {
            ff->connectbar = 1;
            ff->ssid = m.fwdssid;
          }
          if (strcmp(ff->call, m.boxname) && strcmp(ff->call, DUMMY))
          {
            dest = boxdest;
            for (destindex = 0; destindex < anzdest; destindex++)
            {
              dest = nextword(dest, eintrag, HADRESSLEN);
              if (! stricmp(eintrag, ff->call))
              { //Eintrag existiert schon -> Index setzen
                dest_idx[destindex] |= (LONG_ONE << (boxindex));
                break;
              }
            }
            if (destindex >= MAXDEST - 1)
            {
              trace(serious, name, ms(m_fwd_toomanydest));
              if (*b->logincall) putf(ms(m_fwd_toomanydest));
              break;
            }
            if (destindex >= anzdest)
            { //neuer Eintrag
              if ((fwddestlen + CALLEN + 1) >= MAXDESTLEN)
              {
                trace(serious, name, ms(m_fwd_toomanydest));
                if (*b->logincall) putf(ms(m_fwd_toomanydest));
                break;
              }
              strcat(boxdest, ff->call);
              strcat(boxdest, " ");
              dest_idx[anzdest] = (LONG_ONE << boxindex);
              anzdest++;
              fwddestlen = strlen(boxdest);
            }
          }
          if (++boxindex >= MAXFWD)
          {
            trace(serious, name, ms(m_fwd_toomanycalls));
            if (*b->logincall) putf(ms(m_fwd_toomanycalls));
            break;
          }
          ff = &fp[boxindex];
        }
      }
    }
    *ff->call = 0;
    s_fclose(f);
    trace(report, name, "%d partners, %d destinations", boxindex, anzdest);
    if (*b->logincall)
      putf("%s: %d partners, %d destinations.\n", name, boxindex, anzdest);
    fwdpartners = boxindex;
    fwddest = anzdest;
    fwddestlen = strlen(boxdest);
    return;
  }
  else
  {
    if (! (f = s_fopen(FWDLISTNAME, "swt")))
    { // file is not accessible (why ever...)
      trace(serious, name, "open "FWDLISTNAME);
      if (*b->logincall) putf(ms(m_filenotopen), FWDLISTNAME);
      return;
    }
    char fwd_init[] =
      ";sample fwd file\n"
      ";\n"
      ";db0aab - db0aab-8 hb9ra-2\n"
      ";     db0aab  $wp\n"
      ";\n";
    fwrite(fwd_init, sizeof(fwd_init) - 1, 1, f);
    s_fclose(f);
    trace(replog, name, "sample "FWDLISTNAME" generated");
    return;
  }

  error:
#ifdef __DOS16__
    sound(1000);
    wdelay(106);
    nosound();
#endif
    trace(serious, name, "line %d error", line);
    if (*b->logincall) putf(ms(m_fwd_lineerr), name, line);
    s_fclose(f);
}

/*---------------------------------------------------------------------------*/

void dirfwdlist (char * dest)
//*************************************************************************
//
//  Gibt die Forwardeintraege aus
//
//*************************************************************************
{
  lastfunc("dirfwdlist");
  unsigned int boxindex = 0;
  unsigned int di, tab;
  char *dst = boxdest;
  char wort[HADRESSLEN+1];

  if (! *dest)
  {
    di = 1;
    for (tab = 0; tab < fwddest; tab++)
    {
      dst = nextword(dst, wort, HADRESSLEN);
      if (dest_idx[tab])
      {
        putf("%-19s", wort);
        if (di == 4)
        {
          putv(LF);
          di = 0;
          if (testabbruch()) break;
        }
        else putf(" ");
        di++;
        waitfor(e_ticsfull);
      }
    }
    putv (LF);
    return;
  }
  if (! strcmp(dest, "+")) *dest = 0;

  for (tab = 0; tab < fwddest; tab++)
  {
    dst = nextword(dst, wort, HADRESSLEN);
    if (! *dest || strstr(wort, dest))
    {
      di = 0;
      putv(LF);
      if (testabbruch()) break;
      putf("%19s : ", wort);
#if TEST_FWD
      strcpy(b->at, wort);
      weiterleiten(0, "");
      putf("%s ", b->destboxlist);
#else
      for (boxindex = 0; boxindex < fwdpartners; boxindex++)
      {
        if ((dest_idx[tab] >> boxindex) & LONG_ONE)
        {
          if (di == 8)
          {
            putv(LF);
            if (testabbruch()) break;
            putf("                       ");
            di = 0;
          }
          putf("%-6s ", fp[boxindex].call);
          di++;
        }
        waitfor(e_ticsfull);
      }
#endif
    }
  }
  putv(LF);
}

/*---------------------------------------------------------------------------*/

void printfwdlist (char *box)
//*************************************************************************
//
//  Gibt die Forwardliste aus. Die Ausgabe erfolgt im gleichen
//  Format wie die Eingabe ueber das Initialisierungsfile
//
//*************************************************************************
{
  lastfunc("printfwdlist");
  int boxindex = 0;
  unsigned int di = 0, tab = 0;
  char *dest;
  char eintrag[HADRESSLEN+1];
  char cc[FSPATHLEN];
  fwdpara_t *ff;
  ff = &fp[boxindex];

  while (*ff->call)
  {
    if (testabbruch()) break;
    if (! *box || stristr(ff->call, box))
    {
      putv(LF);
      if (testabbruch()) break;
      putf("; BBS  012345678901234567890123 Path\n");
      if (testabbruch()) break;
      putf("%-6s %s %s", ff->call, ff->times, ff->concall);
      if (ff->nolink == 0 && strpos(ff->concall, "IGATE") != -1)
      {
        strcpy(cc, ff->concall);
        erase_igate(cc);
        putv(LF);
        putf("                currently used: %s", cc);
      }
      if (ff->options)
      {
        int i;
        putv(LF);
        if (testabbruch()) break;
        for (i = 0; i < 26; i++)
        {
          if ((ff->options >> i) & 1)
          {
            int ch = i + 'A';
            putf(" -%c", ch);
            switch (ch)
            {
              case  'B': putf("%ld", ff->maxbytes); break;
#ifdef _LCF
              case  'C': putf("%ld", ff->fwdcallformat); break;
#endif
            }
          }
        }
      }
      di = 80;
      dest = boxdest;
      for (tab = 0; tab < fwddest; tab++)
      {
        dest = nextword(dest, eintrag, HADRESSLEN);
        if ((dest_idx[tab] >> boxindex) & LONG_ONE)
        {
          di += strlen(eintrag) + 1;
          if (di > 78)
          {
            putv(LF);
            if (testabbruch()) break;
            di = strlen(eintrag) + 1;
          }
          putf(" %s", eintrag);
        }
        waitfor(e_ticsfull);
      }
    }
    boxindex++;
    ff = &fp[boxindex];
  }
  putv(LF);
}

/*---------------------------------------------------------------------------*/
#ifdef DF3VI_FWD_EDIT
void savefwdlist (void)
//*************************************************************************
//
//  Schreibt die Forwardliste in FWD.BCM. Die Ausgabe erfolgt im gleichen
//  Format wie die Eingabe ueber das Initialisierungsfile
//
//*************************************************************************
{
  lastfunc("savefwdlist");
  int boxindex = 0;
  unsigned int di = 0, tab = 0;
  FILE *f;
  char eintrag[HADRESSLEN+1];
  fwdpara_t *ff;
  char buf[FSPATHLEN + CALLEN + 25 + 2];
  char *dest;

  xunlink(FWDLISTNAMEBAK);
  xrename(FWDLISTNAME, FWDLISTNAMEBAK);
  if ((f = s_fopen(FWDLISTNAME, "swt")) != NULL)
  {
    ff = &fp[boxindex];
    sprintf(buf, "; Forwardfile " STD_BOXHEADER " %s-%i\n;",
                 m.boxname, m.fwdssid);
    fputs(buf, f);
    while (*ff->call)
    {
      sprintf(buf, "\n; BBS  012345678901234567890123 Path");
      fputs(buf, f);
      sprintf(buf, "\n%-6s %s %s", ff->call, ff->times, ff->concall);
      fputs(buf, f);
      if (ff->options)
      {
        char i;
        fputs("\n ", f);
        for (i = 0; i < 26; i++)
        {
          if ((ff->options >> i) & 1L)
          {
            char ch = i + 'A';
            sprintf(buf, " -%c", ch);
            fputs(buf, f);
            switch (ch)
            {
              case  'B': sprintf(buf, "%ld", ff->maxbytes);
                         fputs(buf, f);
                         break;
#ifdef _LCF // JJ
              case  'C': sprintf(buf, "%ld", ff->fwdcallformat);
                         fputs(buf, f);
                         break;
#endif
            }
          }
        }
      }
      di = LINELEN + 1;
      dest = boxdest;
      for (tab = 0; tab < fwddest; tab++)
      {
        dest = nextword(dest, eintrag, HADRESSLEN);
        if ((dest_idx[tab] >> boxindex) & LONG_ONE)
        {
          di += strlen(eintrag) + 1;
          if (di >= (LINELEN - 1))
          {
            fputs("\n ", f);
            di = strlen(eintrag) + 1;
          }
          sprintf(buf, " %s", eintrag);
          fputs(buf, f);
        }
      }
      boxindex++;
      ff = &fp[boxindex];
    }
    fputs("\n;\n;End\n", f);
    s_fclose(f);
    putf(ms(m_filestored), FWDLISTNAME);
  }
  else
    trace(serious, "savefwdlist", ms(m_filenotopen), FWDLISTNAME);
}

/*---------------------------------------------------------------------------*/

void fwdlistedit (char *befbuf)
//*************************************************************************
//
// df3vi: remote-editor fuer FWD.BCM
//
//*************************************************************************
{
  lastfunc("fwdlistedit");
  char cmd[5], box[CALLEN+1], call[CALLEN+1], timesf[25], path[FSPATHLEN+1],
       worte[MAXPARSE], vgl[HADRESSLEN+1], eintrag[HADRESSLEN+1];
  char *entry, *s_ptr, *dst;
  char syntax1[80];
  char syntax2[80];
  unsigned int indexf, boxindex = 0, i, anz;
  fwdpara_t *ff;

  strcpy(syntax1, "Syntax: FWDEDIT <cmd> [[entries] boxcall]\n");
  strcpy(syntax2, "<cmd>: CALL, TIME, PATH, ADD, DEL, NEW, REMOVE, BACKUP\n");
  if (! *befbuf) //keine Parameter
  {
    putf(syntax1);
    putf(syntax2);
    return;
  }
  strupr(befbuf);
  entry = nextword(befbuf, cmd, 4); //Kommando suchen, danach sind Eintraege
  s_ptr = befbuf + strlen(befbuf);  //Vom Stringende her Leerzeichen suchen
  while (s_ptr > befbuf && *s_ptr != ' ') s_ptr--;
  strncpy(box, ++s_ptr, CALLEN);    //dahinter steht das Box-Call
  box[CALLEN] = 0;                  //max. 6 Zeichen
  *s_ptr = 0;   //das kann nun weg, uebrig bleiben Eintraege
  switch (*cmd) // Befehle, die sich nicht auf einen Eintrag beziehen
  {
/*     case 'S': //save changes - nur vorlaeufig
               savefwdlist();
               return;
*/
     case 'B': //backup old changes
               if (file_isreg(FWDLISTNAMEBAK))
               {
                 xunlink(FWDLISTNAME);
                 filecopy(FWDLISTNAMEBAK, FWDLISTNAME);
                 initfwdlist();
               }
               else putf(ms(m_filenotfound), FWDLISTNAMEBAK);
               return;
     case 'N': //new entry
               if (! strcmp(box, m.boxname))
               {
                 putf(ms(m_nofwdmycall));
                 return;
               }
               if (mbcallok(box) || ! strcmp(box, DUMMY))
               { //Boxcall sieht sinnvoll aus
                 boxindex = 0;
                 ff = &fp[boxindex];
                 //Suche leeren Eintrag
                 while (*ff->call && boxindex < MAXFWD)
                 { //Existiert Eintrag schon?
                   if (! stricmp(ff->call, box))
                   {
                     putf(ms(m_fwd_boxexist), box);
                     return;
                   }
                   ff = &fp[++boxindex];
                 }
                 if (boxindex >= MAXFWD)
                 { //Alles voll
                   putf(ms(m_fwd_toomanycalls));
                   return;
                 }
                 if (! *ff->call)
                 { //Leeren Eintrag fuellen
                   strncpy(ff->call, box, CALLEN);
                   ff->call[CALLEN] = 0;
                   strcpy(ff->times, "APAAAAAAAAAAAAAAAAAAAAAA");
                   ff->sidrtt = 0;
                   ff->n_conok = 0;
                   ff->lastwp = 0;
                   ff->n_login = 0;
                   ff->txf = 0;
                   ff->rxf = 0;
                   ff->wprot_r_partner = 0;
                   ff->current_routing_quality = 0;
                   ff->routing_quality = 0;
                   ff->routing_txf = 0;
                   ff->lastmeasure = 0;
                   ff->lastwpr = 0;
                   ff->timeout_cnt = 0;
                   ff->connectbar = 0;
                   ff->nolink = 0;
                   ff->ssid = m.fwdssid;
                   if (strcmp(box, DUMMY))
                   {
                     strncpy(ff->concall, box, CALLEN);
                     //Fwd-Eintrag fuer neues Call machen
                     dst = boxdest;
                     for (indexf = 0;indexf < fwddest; indexf++)
                     {
                       dst = nextword(dst, eintrag, HADRESSLEN);
                       if (! stricmp(eintrag, ff->call))
                       { //Eintrag existiert schon -> Index setzen
                         dest_idx[indexf] |= (LONG_ONE << boxindex);
                         break;
                       }
                     }
                     //Geht noch einer rein?
                     if (indexf >= (MAXDEST - 1))
                     {
                       putf(ms(m_fwd_toomanydest));
                       break;
                     }
                     //Wenn nicht gefunden
                     if (indexf >= fwddest)
                     { //neuer Eintrag
                       if ((fwddestlen + 7) >= MAXDESTLEN)
                       {
                         putf(ms(m_fwd_toomanydest));
                         break;
                       }
                       strcat(boxdest, ff->call);
                       strcat(boxdest, " ");
                       dest_idx[fwddest] = (LONG_ONE << boxindex);
                       fwddest++;
                       fwddestlen = strlen(boxdest);
                     }
                   }
                   else
                     strcpy(ff->concall, DUMMY);
                   //Status Limits nachfuehren
                   fwdpartners++;
                   putf(ms(m_fwd_newentryadded), box);
                   savefwdlist();
                 }
               }
               else
               {
                 putf(ms(m_fwd_novalidcall), box);
                 putf("Syntax: FWDEDIT NEW <newcall>\n");
               }
               return;
     default:  //Vorbereitung fuer naechsten switch()
               boxindex = 0;
               ff = &fp[boxindex];
               while (*ff->call && boxindex < MAXFWD)
               {      //Box-Eintrag suchen
                 if (! strcmp(box, ff->call)) break; // Gefunden!
                 ff = &fp[++boxindex];
               }
               if (boxindex >= MAXFWD || ! *ff->call) // Nicht gefunden
               {
                 if (*box)
                   putf(ms(m_fwd_callnotfound), box);
                 else
                   putf(syntax1);
                 return;
               }
   }
   switch (*cmd) // Befehle, die sich auf einen FWD-Eintrag beziehen
   {
     case 'R': //remove fwd-box completely!
               //Index fuer alle Eintraege korrigieren
        {
#ifdef __FLAT__
 #ifdef __LINUX__
               unsigned long long trenner1 =
                                  ((LONG_ONE << boxindex) - LONG_ONE);
               unsigned long long trenner2 =
                                  (~((LONG_ONE << boxindex) - LONG_ONE));
 #else // WIN32
               __int64 trenner1 = ((LONG_ONE << boxindex) - LONG_ONE);
               __int64 trenner2 = (~((LONG_ONE << boxindex) - LONG_ONE));
 #endif
#else
               unsigned long trenner1 =
                             ((LONG_ONE << boxindex) - LONG_ONE);
               unsigned long trenner2 =
                             (~((LONG_ONE << boxindex) - LONG_ONE));
#endif
               dst = boxdest;
               for (indexf = 0; indexf < fwddest; indexf++)
               {
                 dst = nextword(dst, eintrag, HADRESSLEN);
                 // Bitfeld um eins herunterschieben
                 if (! boxindex) dest_idx[indexf] >>= LONG_ONE;
                 else
                 {
                   dest_idx[indexf] = (dest_idx[indexf] & trenner1)
                                       + ((dest_idx[indexf] & trenner2)
                                           >> LONG_ONE);
                 // Bitfeld unterhalb trennpos1 erhalten,
                 //       oberhalb trennpos2 eins herunterschieben
                 }
               }
               //Fuer alle folgenden Eintraege pointer aufruecken
               while (fp[boxindex + 1].call[0] && boxindex < MAXFWD)
               {
                 fp[boxindex] = fp[boxindex + 1];
                 boxindex++;
               }
               //letzten Eintrag freimachen
               memset(&fp[boxindex], 0, sizeof(fwdpara_t));
               //Status Limits nachfuehren
               fwdpartners--;
               putf(ms(m_fwd_entrydeleted), box);
               savefwdlist();
               return;
        }
     case 'C': //change box-call
               nextword(entry, call, CALLEN); //erster Eintrag ist neues Call
               if (strcmp(call, m.boxname))
               {
                 putf(ms(m_nofwdmycall));
                 return;
               }
               if (strlen(call) > 0 && mbcallok(call))
               { //gueltiges Rufzeichen -> schon drin?
                 //NICHT boxindex verwenden, wird noch unveraendert benoetigt!
                 for (indexf = 0; indexf < MAXFWD && *fp[indexf].call; indexf++)
                   if (! stricmp(fp[indexf].call, call))
                   {
                     putf(ms(m_fwd_boxexist), call);
                     return;
                   }
                 //Nicht drin: eintragen...
                 strncpy(ff->call, call, CALLEN);
                 ff->call[CALLEN] = 0;
                 putf(ms(m_fwd_callchanged), ff->call);
                 savefwdlist();
               }
               else
               {
                 putf(ms(m_fwd_novalidcall), entry);
                 putf("Syntax: FWDEDIT CALL <newcall> <oldcall>\n");
                 return;
               }
               //Noch neues Call in Route hinzufuegen!
               entry = call;
               //fall through...
     case 'A': //add entry
               anz = parseline(entry, worte);
               if (! anz) //Nix drin?
               {
                 putf("Syntax: FWDEDIT ADD <entries> <boxcall>\n");
                 return;
               }
               for (i = 0;i < anz; i++)
               {
                 strncpy(eintrag, entry+worte[i], HADRESSLEN); //I-ten Eintrag
                 eintrag[HADRESSLEN] = 0;
                 if (scanforwardoption(eintrag, boxindex)) //Option?
                 {
                   putf(ms(m_fwd_optionset), eintrag);
                   continue;
                 }
                 strip_hadr(eintrag);
                 dst = boxdest;
                 for (indexf = 0; indexf < fwddest; indexf++)
                 {
                   dst = nextword(dst, vgl, HADRESSLEN);
                   if (! strcmp(vgl, eintrag)) break;
                 }
                 if (! strcmp(vgl,eintrag))       // gefunden
                 {
                   if (dest_idx[indexf] & (LONG_ONE<<boxindex)) //schon drin!
                   {
//                 if (cmd[0]=='A')   //silent on FWD C
                       putf(ms(m_fwd_entryexist), eintrag);
                   }
                   else
                   {
                     dest_idx[indexf] |= (LONG_ONE<<boxindex);  //hinzufuegen
//                 if (cmd[0]=='A')        //silent on FWD C
                       putf(ms(m_fwd_entryadded), eintrag);
                   }
                 }
                 else //neuen Eintrag anfuegen
                 {
                   //Geht noch einer rein?
                   if (indexf >= (MAXDEST - 1))
                   {
                     putf(ms(m_fwd_toomanydest));
                     break;
                   }
                   if ((fwddestlen + strlen(eintrag)) >= MAXDESTLEN)
                   {
                     putf(ms(m_fwd_toomanydest));
                     break;
                   }
                   strcat(boxdest, eintrag);
                   strcat(boxdest, " ");
                   dest_idx[fwddest] = (LONG_ONE << boxindex);
//               if (cmd[0]=='A')            //silent on FWD C
                     putf(ms(m_fwd_entryadded), eintrag);

                   //Status Limits nachfuehren
                   fwddest++;
                   fwddestlen = strlen(boxdest);
                 }
               }
               savefwdlist();
               return;
     case 'D': //delete entry
               anz = parseline(entry, worte);
               if (! anz) //Nix drin?
               {
                 putf("Syntax: FWDEDIT DEL <entries> <boxcall>\n");
                 return;
               }
               for (i = 0; i < anz; i++)
               {
                 strncpy(eintrag, entry+worte[i], HADRESSLEN); //I-ten Eintrag
                 eintrag[HADRESSLEN] = 0;
                 if (scanforwardoption(eintrag, boxindex))   //Option?
                 {  //obige Funktion setzt immer, XOR setzt dann zurueck!
                   ff->options ^= (1L << (toupper (eintrag[1]) - 'A'));
                   putf(ms(m_fwd_optionreset), eintrag);
                   continue;
                 }
                 else if (! stricmp(ff->call, eintrag))
                 {    //Nicht Pfad zu sich selbst loeschen
                   putf(ms(m_fwd_cannotdelete));
                   continue;
                 }
                 dst = boxdest;
                 for (indexf = 0; indexf < fwddest; indexf++)
                 {
                   dst = nextword(dst, vgl, HADRESSLEN);
                   if (! strcmp(vgl, eintrag)) break;
                 }
                 if (strcmp(vgl, eintrag))    //Nicht drin?
                   putf(ms(m_fwd_entrynotfound), eintrag);
                 else //Eintrag loeschen, wenn vorhanden
                 {
                   if (dest_idx[indexf] & (LONG_ONE << boxindex))
                   {
                     dest_idx[indexf] &= (~(LONG_ONE << boxindex));
                     putf(ms(m_fwd_entrydeleted), eintrag);
                   }
                   else
                     putf(ms(m_fwd_entrynotfound), eintrag);
                 }
               }
               savefwdlist();
               return;
     case 'T': //change time
               nextword(entry, timesf, 24);
               if (! strlen(timesf))
                 putf("Syntax: FWDEDIT TIME <timestring> <boxcall>\n");
               else
                 if (24 % strlen(timesf)) // Nur 1, 2, 3, 4, 6, 8, 12, 24 lang
                   putf(ms(m_fwd_novalidtime), timesf);
                 else
                 {
                   if (*timesf == '-')    // Faulheitsoption
                     strcpy(ff->times, "APAAAAAAAAAAAAAAAAAAAAAA");
                   else
                   {
                     *ff->times = 0;      // Vervielfachen bis es passt
                     while (strlen(ff->times) < 24)
                       strcat(ff->times, timesf);
                   }
                   putf(ms(m_fwd_timechanged), ff->call, ff->times);
                   savefwdlist();
                 }
               return;
     case 'P': //change path
               if (strlen(entry) >= FSPATHLEN)
               {
                 putf(ms(m_fwd_pathtoolong));
                 return;
               }
               else if (*entry)
               {
                 for (s_ptr = path, i = 0;
                      entry[i] && (s_ptr - entry < FSPATHLEN); i++)
                 {  //Schraegstrich mit Leerzeichen umgeben, wenn keine da
                   if (entry[i] == '/' && entry[i - 1] != ' ') *s_ptr++ = ' ';
                   *s_ptr++ = entry[i];
                   if (entry[i] == '/' && entry[i + 1] != ' ') *s_ptr++ = ' ';
                 }  //Fehlen zu viele Leerzeichen, geht evtl was verloren :(
                 *s_ptr = 0;
                 strncpy(ff->concall, path, FSPATHLEN-1);
                 ff->concall[FSPATHLEN - 1] = 0;
                 putf(ms(m_fwd_pathchanged), ff->call, ff->concall);
                 ff->connectbar = 1;
                 ff->ssid = m.fwdssid;
                 savefwdlist();
               }
               else
               {
                 putf("Syntax: FWDEDIT PATH <fwdpath> <boxcall>\n");
                 return;
               }
               return;
     default:  //short help
               putf(syntax1);
               putf(syntax2);
   }
}
#endif //df3vi_fwd_edit

/*---------------------------------------------------------------------------*/

void fwdstat (char *box)
//*************************************************************************
//
//  Gibt die Anzahl der anstehenden Nachrichten fuer alle Nachbarboxen aus
//
//*************************************************************************
{
  lastfunc("fwdstat");
  int boxidx = 0;
  char fname[FNAMELEN+1];
  char s[LINELEN+41];
  long mails;
  int bulletin;
  FILE *f;
  int ok;
  double bytes;
  double bytessum = 0L;
  long int mailsum[3] = { 0L, 0L, 0L };
  long int mailrtx[2] = { 0L, 0L };
  fwdpara_t *ff = &fp[boxidx];
//      123456 1234567 12345 1234 12345 1234 1234 1234567 1234567 1234567 123456
//putf("   BBS  Status  inconnect outconnect  RTT    User    Info   WP/EM  Bytes");

//      123456 1234567 12345 12345 123456 123456 1234 1234567 1234567 1234567 123456
  putf("   BBS  Status    In   Out MailTX MailRX  RTT    User    Info   WP/EM  Bytes");

  putf("\n" SEPLINE);
  while (*ff->call)
  {
    if (! *box || stristr(ff->call, box))
    {
      if (strcmp(ff->call, DUMMY))
      {
        bytes = 0L;
        if (eingelogt(ff->call, j_fbb_send_propose, 0))       ok = 11;
        else if (eingelogt(ff->call, j_fbb_send_mail, 0))     ok = 10;
        else if (eingelogt(ff->call, j_fbb_send_delay, 0))    ok = 9;
        else if (eingelogt(ff->call, j_fbb_recv_propose, 0))  ok = 8;
        else if (eingelogt(ff->call, j_fbb_recv_mail, 0))     ok = 7;
        else if (eingelogt(ff->call, j_fbb_end, 0))           ok = 6;
        else if (eingelogt(ff->call, j_fbb_error, 0))         ok = 5;
        else if (eingelogt(ff->call, j_fwdsend, 0))           ok = 4;
        else if (eingelogt(ff->call, j_fwdlink, 0))           ok = 3;
        else if (eingelogt(ff->call, j_fwdrecv, 0))           ok = 2;
        else if (strlen(ff->concall) < 1)                     ok = 12;
        else if (! strncasecmp(ff->concall, "FILE", 4))       ok = 13;
        else ok = ff->connectbar;
        putf("\n%-6s ", ff->call);
        if (ok == 0)                    putf("no link ");
        if (ok == 1)                    putf("ok      ");
        if (ok == 2)                    putf("a recv  ");
        if (ok == 3)                    putf("setup   ");
        if (ok == 4)                    putf("a send  ");
        if (ok == 5 || ok == 6)         putf("f end   ");
        if (ok == 7)                    putf("f recv  ");
        if (ok == 8)                    putf("f recvp ");
        if (ok == 9)                    putf("f delay ");
        if (ok == 10)                   putf("f send  ");
        if (ok == 11)                   putf("f sendp ");
        if (ok == 12)                   putf("        ");
        if (ok == 13)                   putf("filefwd ");
/*
        putf("%4s ", zeitspanne(ad_time() - ff->lastlogin, zs_seconds));
        if (ff->n_login) putf("%5ld ", ff->n_login);
        else putf("      ");
        putf("%4s ", zeitspanne(ad_time() - ff->lastconok, zs_seconds));
        if (ff->n_conok) putf("%5ld ", ff->n_conok);
        else putf("      ");
        if (ff->sidrtt)  putf("%4s", zeitspanne(ff->sidrtt, zs_seconds));
        else putf("    ");
        //if (ff->timeout_cnt) putf("%5ld", ff->timeout_cnt);
        //else putf("     ");
*/
        putf("%5s ", zeitspanne(ad_time() - ff->lastlogin, zs_seconds));
        putf("%5s ", zeitspanne(ad_time() - ff->lastconok, zs_seconds));
        //if (ff->n_login) putf("%5ld ", ff->n_login);
        //else putf("      ");
        //if (ff->n_conok) putf("%5ld ", ff->n_conok);
        //else putf("      ");
        if (ff->txf && ff->txf > 0)
        {
          putf("%6ld ", ff->txf);
          mailrtx[0] += ff->txf;
        }
        else
          putf("       ");
        if (ff->rxf && ff->rxf > 0)
        {
          putf("%6ld ", ff->rxf);
          mailrtx[1] += ff->rxf;
        }
        else
          putf("       ");
        if (ff->sidrtt)
          putf("%4s", zeitspanne(ff->sidrtt, zs_seconds));
        else
          putf("    ");
        //if (ff->timeout_cnt) putf("%5ld", ff->timeout_cnt);
        //else putf("     ");

        for (bulletin = 0; bulletin < 3; bulletin++)
        {
          switch (bulletin)
          {
            case 0: strcpy(fname, FWDPATH "/u_"); break;
            case 1: strcpy(fname, FWDPATH "/i_"); break;
            case 2: strcpy(fname, FWDPATH "/w_"); break;
          }
          strcat(fname, ff->call);
          strcat(fname, ".bcm");
          strlwr(fname);
          mails = 0L;
          f = s_fopen(fname, "lrt"); //fwdtrigger
          if (! f && bulletin == 2)  //wenn kein WP, dann E/M
          {
            sprintf(fname, FWDPATH"/m_%s.bcm", ff->call);
            f = s_fopen(fname, "lrt"); //fwdtrigger
          }
          if (f)
          {
            while (fgets(s, LINELEN+40, f))
            {
              if (*s != ' ') mails++;
              if (bulletin < 2)
              {
                if (strchr(s, ' '))
                  bytes += atof(strchr(s, ' ') + 1);
                bytes += 80; //OE3DZW fix 80 Bytes per Mail;
              }
              waitfor(e_ticsfull);
            }
            s_fclose(f);
            if (bulletin == 2) bytes += filesize(fname);
          }
          if (mails)
          {
            mailsum[bulletin] += mails;
            putf(" %7ld", mails);
          }
          else
            putf("        ");
        }
        if (bytes > 1073741824L)
        {
          if (bytes > 2073741824L)
            putf(" %6.0fG", bytes / 1073741824L);
          else
            putf(" %6.1fG", bytes / 1073741824L);
        }
        else if (bytes > 10485760L) putf(" %6.0fM", bytes / 1048576L);
             else if (bytes > 10240) putf(" %6.0fk", bytes / 1024);
                  else if (bytes) putf(" %6.0f", bytes);
        bytessum += bytes;
      }
    }
    boxidx++;
    ff = &fp[boxidx];
  }
  putf("\n" SEPLINE);
        //"Total (03.11.00 15:09z):   "
  putf("\nTotal (%s):   ", datestr(initzeit, 10));
  if (mailrtx[0])
    putf("%6ld ", mailrtx[0]);
  else
    putf("       ");
  if (mailrtx[1])
    putf("%6ld ", mailrtx[1]);
  else
    putf("       ");
  if (bytessum)
  {
    putf("    ");
    for (bulletin = 0; bulletin < 3; bulletin++)
      if (mailsum[bulletin])
        putf(" %7ld", mailsum[bulletin]);
      else
        putf("        ");
    if (bytessum > 1073741824L)
    {
      if (bytessum > 2073741824L)
         putf(" %6.0fG", bytessum / 1073741824L);
      else
         putf(" %6.1fG", bytessum / 1073741824L);
    }
    else if (bytessum > 10485760L) putf(" %6.0fM", bytessum / 1048576L);
         else if (bytessum > 10240) putf(" %6.0fk", bytessum / 1024);
              else if (bytessum) putf(" %6.0f", bytessum);
  }
  putv(LF);
}

/*---------------------------------------------------------------------------*/

#ifdef _TELEPHONE
int isdialtime (char *call)
//*************************************************************************
//
// JJ:
// isdialtime - erkennt ob gewaehlt werden muss
//
//*************************************************************************
{
  unsigned int i = 0;
  struct tm *tt;
  time_t ti = ad_time();
  char ch;

  tt = ad_comtime(ti);
  while (fp[i].call[0])
  {
    if (! stricmp(fp[i].call, call))
    {
      ch = fp[i].times[tt->tm_hour];
      if (ch == 'D') return 1;
      return 0;
    }
    else i++;
  }
  return 0;
}
#endif

/*---------------------------------------------------------------------------*/

int isforwardtime (char *call, int bulletins)
//*************************************************************************
//
//  Gibt zurueck, ob gerade zu einem Forwardpartner geforwardet
//  werden soll
//
//  Bei bulletins=3 wird zurueckgegeben, ob fwdpartner gepollt werden soll
//
//*************************************************************************
{
  lastfunc("isforwardtime");
  unsigned int i = 0;
  struct tm *tt;
  time_t ti = ad_time();
  char ch;
  int retwert = 1;

  tt = ad_comtime(ti);
  while (fp[i].call[0])
  {
    if (! stricmp(fp[i].call, call))
    {
      ch = fp[i].times[tt->tm_hour];
      if (fp[i].connectbar) retwert = 2;
      if (bulletins == 3)
      {
        if (ch == 'P') return retwert;
        else return 0;
      }
#ifdef _TELEPHONE
      if (ch == 'A' || ch == 'P' || ch == 'D') // JJ
#else
      if (ch == 'A' || ch == 'P')
#endif
        return retwert;
      if (! bulletins && ch == 'U') return retwert;
      else return 0;
    }
    i++;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int boxbinok (int fbball)
//*************************************************************************
//
// Boxbinok
//
//*************************************************************************
{
  if (b->forwarding == fwd_none) return OK;
  if (b->opt & o_d) return OK;
  if (b->boxtyp == b_thebox && b->boxversion >= 19) return OK;
  // Send personal binary messages to fbb
  if (b->opt & o_b && (fbball || b->mailtype == 'P')) return OK;
  if ((b->forwarding == fwd_standard) && (fpara()->options & o_i)) return OK;
  return NO;
}

/*---------------------------------------------------------------------------*/

void add_hold (char *reason, int returnmail)
//*************************************************************************
//
//  Erzeugt eine Zeile in sfhold
//
//*************************************************************************
{
  lastfunc("add_hold");
  char sendcmd[LINELEN+1];
  char content[255];
  FILE *f;

  f = s_fopen(TRACEPATH "/" SFHOLDNAME, "sab");
  if (! f) f = s_fopen(TRACEPATH "/" SFHOLDNAME, "swb");
  if (f)
  {
    fprintf(f, "%s ", datestr(ad_time(), 11));
    fprintf(f, "#%c %-6s:%-6s > %-6s ",
            b->fwdhold, b->logincall, b->herkunft, b->ziel);
    fprintf(f, "@ %s $%s %s" NEWLINE, b->at, b->bid, reason);
    s_fclose(f);
  }
  if (returnmail && returnmailok() && b->forwarding == fwd_standard)
  {
    sprintf(sendcmd, "SP %s < %s HOLD: Your mail for %s",
                     b->herkunft, m.boxname, b->zielboard);
    sprintf(content, "Your msg for %s @ %s $%s could\n"
                     "not be forwarded from %s to %s.\nReason: %s.",
                     b->zielboard, b->at, b->bid, m.boxname,
                     b->logincall, reason);
    genmail(sendcmd, content);
  }
}

/*---------------------------------------------------------------------------*/

int scanheader (FILE *f, fwdpara_t *ff)
//*************************************************************************
//
//  Investigate the header of a message against plausibility
//  faults before forwarding the mail.
//
//  return value:
//
//    - 1 if message was found correctly
//    - 0 if an error had occurred. message is not forwarded
//      reason is remarked into syslog
//
//*************************************************************************
{
  lastfunc("scanheader");
  char s[BUFLEN];
  char sendcmd[DIRLEN + CALLEN + HADRESSLEN + HADRESSLEN + 19 + 1];
  char content[LINELEN+1];
  int i = 3;
  int ownbox = 0;
  time_t lasttime = ad_time();

  if (! f || ! ff) return NO;
  rewind(f);
  fgets(s, BUFLEN - 1, f);
  mbsend_parse(s, 0);
  // Max. Mailgroesse wurde ueberschritten
  // oe3dzw: Mail generieren falls nicht Status HOLD gesetzt
  if ((ff->options & o_b) && b->bytes > ff->maxbytes && ff->maxbytes >> 10)
  {
    s_fclose(f);
    if (returnmailok() && b->fwdhold == '!')
    {
      sprintf(sendcmd, "SP %s @ %s < %s Mail at %s\n",
                       b->ziel, b->at, b->herkunft, m.boxadress);
      sprintf(content, "Message too large, no forward to %s.\n",
                       b->logincall);
      genmail(sendcmd, content);
      b->fwdhold = 'S';
      add_hold("too large", 1);
      writemailflags();
    }
    return NO;
  }
  // Nachbarbox kann kein BoxBin
  if (b->binstart && ! boxbinok(0))
  { //oe3dzw: Mailtype..
    s_fclose(f);
    if (b->mailtype == 'P' && b->fwdhold == '!')
    {
      b->fwdhold = 'B';
      add_hold("no binary fwd", 1);
      writemailflags();
    }
    return NO;
  }
  // oe3dzw: Bulletin kam von dieser Box
  if (b->mailtype == 'B' && ! strcmp(b->frombox, b->logincall))
  {
    s_fclose(f);
    return NO;
  }
  // JJ no BIN bulletins
  if (   b->mailtype == 'B' && (ff->options & o_k)
      && b->binstart)
  {
    s_fclose(f);
    //b->fwdhold='K';
    //add_hold("no bin fwd", 1); // -> do not log, not interesting
    //writemailflags(); // -> bulletin may still be fwd to other partners
    return NO;
  }
  // JJ No BIN personal mails
  if (   b->mailtype != 'B' && (ff->options & o_l)
      && b->binstart)
  {
    s_fclose(f);
    b->fwdhold = 'K';
    add_hold("no bin fwd", 1);
    writemailflags();
    return NO;
  }
  // JJ no 7+ bulletins
  if (   b->mailtype == 'B' && (ff->options & o_m)
      && (b->conttype == '7' || b->conttype == '6') )
  {
    s_fclose(f);
    //b->fwdhold='M';
    //add_hold("no 7plus fwd",1); // -> do not log, not interesting
    //writemailflags(); // -> bulletin may still be fwd to other partners
    return NO;
  }
  // JJ No 7+ personal mails
  if (   b->mailtype != 'B' && (ff->options & o_n)
      && (b->conttype == '6' || b->conttype == '7') )
  {
    s_fclose(f);
    b->fwdhold = 'M';
    add_hold("no 7plus fwd", 1);
    writemailflags();
    return NO;
  }
  // BID verloren?
  if (b->mailtype == 'B'  && ! *b->bid)
  {
    s_fclose(f);
    trace(serious, "scanheader", "bid missing %s>%s", b->herkunft, b->ziel);
    b->fwdhold = 'E';
    add_hold("no bid", 1);
    writemailflags();
    return NO;
  }
  while (fgets(s, BUFLEN - 1, f))
  {
    if (i == 3 && *s == '*')
    {
      s_fclose(f);
      return NO; // mail is erased
    }
    if (i)
      i--; // Kommandokopf, Forwardcalls und Betreff der Nachricht wegmachen
    else
    {
      char *ha;
      if (*s == 'R' && s[1] == ':')
      {
        if (b->mailtype == 'P')
          lasttime = ad_time(); // oldestfwd soll nur bei Bulletins greifen
        else
        {
          lasttime = getheadertime(s);
          if (! lasttime) lasttime = ad_time();
        }
        ha = getheaderadress(s);
        if (ha && ! strcmp(ha, m.boxadress) && s[13] == 'z') ownbox++;
      }
      else break;
    }
  } // loop
  if (   b->fwdhold == '!'       // check only if not held before
      && ownbox > (1 + ((! (b->mailtype == 'B') & 1) * 2)))
      // Pers. mails duerfen 3mal durchlaufen
  {
    s_fclose(f);
    if (b->mailtype == 'P')
    {
      trace(replog, "scanheader", "loop %s>%s @ %s",
                                  b->herkunft, b->ziel, b->at);
      b->fwdhold = 'L';
      add_hold("fwd loop", 1);
      writemailflags();
    }
    return NO;
  }
  // too old..
  else if (lasttime < (ad_time() - ((long) m.oldestfwd * DAY)))
  {
    s_fclose(f);
    trace(replog, "scanheader", "too old %s>%s $%s",
                                b->herkunft, b->ziel, b->bid);
    return NO;
  }
  s_fclose(f);
  return OK;
}

/*---------------------------------------------------------------------------*/

static char *fname_aus_path (char *path)
//*************************************************************************
//
//  Extrahiert aus einem Pfadnamen den Filenamen (steht am Ende, bis /)
//
//  z.B. Eingang:  INFO/GERAETE/YAESU/12DEAD6
//       Rueckgabe: 12DEAD6
//
//*************************************************************************
{
  unsigned int i = strlen(path);
  while (i && (path[i - 1] != '/')) i--;
  return path + i;
}

/*---------------------------------------------------------------------------*/

char *board_aus_path (char *path)
//*************************************************************************
//
//  Extrahiert den Namen eines Boards aus dem vollstaendigen Pfadnamen
//
//  z.B. Eingang:  INFO/GERAETE/YAESU/12DEAD6
//       Rueckgabe: YAESU
//
//*************************************************************************
{
  lastfunc("board_aus_path");
  static char boardname[DIRLEN+1];
  int i = strlen(path), j = 0, end = 0;

  while (i && (path[i - 1] != '/')) i--;
  if (! i)
    trace(serious, "board_aus_path", "path %s", path);
  else
  {
    i = end = i - 1;
    while (i && (path[i - 1] != '/')) i--;
    for (;i < end; i++)
    {
      boardname[j++] = path[i];
      if (j > DIRLEN)
      {
        trace(serious, "board_aus_path", "fn %s", path);
        break;
      }
    }
    boardname[j] = 0;
    return boardname;
  }
  return "??";
}

/*---------------------------------------------------------------------------*/

void markfwd (void)
//*************************************************************************
//
//  Vermerkt in einer Nachricht, dass sie geforwardet worden ist
//
//*************************************************************************
{
  lastfunc("markfwd");
  char line[256];
  char *pos;
  char *crlf;
  FILE *f;

  strlwr(b->mailpath);
  if ((f = s_fopen(b->mailpath, "sr+b")) != NULL)
  {
    fread(line, 255, 1, f); // Header- und Forwardzeile einlesen
    line[255] = 0;
    if ((crlf = strchr(line, LF)) != NULL)
    {
      if ((pos = strstr(crlf + 1, b->logincall)) != NULL)
      {
        while (isalnum(*pos)) pos++;
        if (*pos == ' ' || *pos == '.')
        {
          *pos = '*';
          rewind(f);
          fwrite(line, 255, 1, f);
        }
      }
    }
    s_fclose(f);
  }
}

/*---------------------------------------------------------------------------*/

void readmail (FILE *f, char crlf)
//*************************************************************************
//
// DH3MB: Readmail
//
//*************************************************************************
{
  lastfunc("readmail");
  int c, last = 0;
  long cnt = ftell(f);

  while ((c = fgetc(f)) != EOF)
  {
    cnt++;
    if (crlf && c == LF) bputv(CR); // DH3MB: Force CR/LF as EOL-specifier
    putv(c);
    if (b->binstart && cnt == b->binstart)
    { //mail is open in binary mode!
      sendbin(f, "");
      s_fclose(f);
      f = NULL;
      break;
    }
    last = c;
    if (c == LF)
    {
      waitfor(e_ticsfull);
      timeout(m.fwdtimeout);
    }
  }
  if (! b->binstart && last != LF)
  {
    if (crlf) bputv(CR); // DH3MB: Force CR/LF as EOL-specifier
    putv(LF);
  }
  if (f) s_fclose(f);
}

/*---------------------------------------------------------------------------*/

int setfullpath (char *path)
//*************************************************************************
//
// Setfullpath
//
//*************************************************************************
{
  lastfunc("setfullpath");

  if (finddir(board_aus_path(path), b->sysop))
  {
    strcpy(b->mailpath, b->boardpath);
    strcat(b->mailpath, "/");
    strcpy(b->listpath, b->mailpath);
    strcpy(b->mailfname, fname_aus_path(path));
    strcat(b->mailpath, b->mailfname);
    strcat(b->listpath, DIRLISTNAME);
    return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

void killforwarded (char *fname)
//*************************************************************************
//
//  Durchsucht eine Forwardliste und loescht Files, die bereits
//  geforwardet sind.
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "killforwarded");
  lastfunc(name);
  FILE *f1, *f2, *of;
  char s[LINELEN+1];
  char mailname[30];
  long n = 0; // number of mails in output-triggerfile
  long x = 0; // number of mails in input-triggerfile
  int good;   // this entry is valid
  char tmpname[FNAMELEN+1];
  char *a;

  if (strstr(fname, "/m_")) return; //only if not e/m-Mails
  if (strstr(fname, ".bcm"))
  {
    strcpy(tmpname, fname);
    a = strstr(tmpname, ".bcm");
    strcpy(a, ".tmp"); // eg. fwd/i_db0aab.tmp
  }
  else
    trace(fatal, name, "fname %s", fname);
  while (! sema_lock(fname)) wdelay(1543);
  if ((f1 = s_fopen(fname, "srt")) != NULL)
  {
    if (! (of = s_fopen(tmpname, "swt")))
    {
      trace(serious, name, "fopen errno=%d %s", errno, strerror(errno));
      s_fclose(f1);
      return;
    }
    s_fsetopt(of, 1);
    while (fgets(s, LINELEN, f1))
    {
      if (! *s || strlen(s) > 50) // if there is trash in the forward file,
                                  // remove it
      {
        fgets(s, LINELEN, f1); //skip next entry
        trace(serious, name, "trash");
        continue;
      }
      x++;
      if (*s > ' ')
      {
        strncpy(mailname, s, 29);
        mailname[29] = 0;
        subst1(mailname, ' ', 0); //remove trailing infos
        if (strlen(s) < MIN_TRIG_LEN)
        {
          trace(serious, name, "%s trash %s", fname, s);
          good = 0;
        }
        if (n > 7) good = 1; // at least the first 7 entries should be ok
        else
        {
          setfullpath(mailname);
          strlwr(b->mailpath);
          if ((f2 = s_fopen(b->mailpath, "srt")) != NULL)
          {
            fgets(b->line, BUFLEN - 1, f2); //skip header line
            fgets(b->line, BUFLEN - 1, f2); //read trigger line
            s_fclose(f2);
            if (*b->line == '*') good = 0;
            else good = 1;
          }
          else good = 0;
        }
        if (good)
        {
          n++;
          fputs(s, of);
        }
      }
      //waitfor(e_ticsfull);
    }
    s_fclose(f1);
    s_fclose(of);
    xunlink(fname);
    if (n) xrename(tmpname, fname);
    else xunlink(tmpname);
    //trace(report, "killfwded", "%s: %ld->%ld", fname, x, n);
  }
  sema_unlock(fname);
}

/*---------------------------------------------------------------------------*/

char getfwdlistentry (char *call, char type, char num, char **lines)
//*************************************************************************
//
//  Scans the list- or fwd-trigger-file of "call" and returns a list
//  seperated with 0-bytes of at most <num> mails we must forward to
//  the partner
//
//  Returns number of mails in the list
//
//  "*lines" is set to one byte after the final 0-byte of the list
//
//  type: 'u', 'i', 'm': Scan a forward-trigger-file (fwd/<type>_<call>.bcm)
//        'l': Scan a list-file (<userpath>/<user>/<list.bcm>)
//        'c': Scan a list-file, but only check, if there are mails for S&F
//
//  (DH3MB)
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "getfwdlistentry");
  lastfunc(name);
  char found = 0;
  char *bytepos;
  char *delaypos;
  char mailname[30];
  char fpath[30];
  FILE *f, *f2 = NULL;
  handle fh;
  char *lineptr[FBB_MAX_PROP];

  if (num <= 0) return 0;
  if (type == 'u' || type == 'i' || type == 'm')
  {
    sprintf(fpath, FWDPATH"/%c_%s.bcm", type, call);
    strlwr(fpath);
    killforwarded(fpath);
    if ((f = s_fopen(fpath, "srt")) != NULL) // fwdtrigger
    {
      while (found < num && fgets(b->line, LINELEN, f)) // swapped
      {
#ifdef DEBUG_FWD
        trace(report, "getfle1", "%s: %s", call, b->line);
#endif
        if (*b->line == ' ') continue;      // skip already forwarded entries
        if (strlen(b->line) < MIN_TRIG_LEN) // Simple plausibility check
        {
          trace(serious, name, "trash: %s", b->line);
          continue;
        }
        rm_crlf(b->line);
        waitfor(e_ticsfull);
        if (type != 'm')
        {
          if ((bytepos = strchr(b->line, ' ')) != 0) // Find byte-field
          {
            *(bytepos++) = 0;                // Replace space by end-of-string
            while (*bytepos == ' ') bytepos++; // Skip blanks
            if ((delaypos = strchr (bytepos, ' ')) != 0)
            {
              *(delaypos++) = 0;             // Replace space by end-of-string
              while (*delaypos == ' ') delaypos++; // Skip blanks
            }
            else bytepos = 0;
          }
          else delaypos = 0;
        }
        else
        {
          bytepos = delaypos = 0;
        }
        if (! (delaypos && atol(delaypos) > ad_time()))
        {
          if (type != 'm')
          {
            strncpy(mailname, b->line, 29);
            mailname[29] = 0;
            setfullpath(mailname);
            strlwr(b->mailpath);
#ifdef DEBUG_FWD
            trace(report, "getfle2", "%s: %s", call, mailname);
#endif
            //check for sema-locking of b->mailpath
            if (! sema_test(b->mailpath)
                && (f2 = s_fopen(b->mailpath, "srt")) != NULL)
            {
              fgets(b->line, BUFLEN - 1, f2); // Skip header line
              fgets(b->line, BUFLEN - 1, f2); // Read trigger line
              s_fclose(f2);
              if (*b->line == '*')
                trace(report, name, "erased: %s", mailname);
              else
              {
                unsigned int i = 0, d = 0;
                while (i < found)
                {
                  if (! strcmp(lineptr[i], mailname)) d++;
                  i++;
                }
                if (! d)
                {
                  strcpy(*lines, mailname);
                  lineptr[found] = *lines;
                  *lines += strlen(*lines) + 1;
                  found++;
                }
              }
            }
          }
          else
          {
            int unsigned i = 0, d = 0;
            while (i < found)
            {
              if (! strcmp(lineptr[i], b->line)) d++;
              i++;
            }
            if (! d)
            {
              strcpy(*lines, b->line);
              lineptr[found] = *lines;
              *lines += strlen(*lines) + 1;
              found++;
            }
          }
        }
      }
      s_fclose(f);
      return found;
    }
    else return 0;
  }
  else // type 'l' or 'c'
  {
    sprintf(b->listpath, "%s/%s/" DIRLISTNAME, m.userpath, call);
    strlwr(b->listpath);
    if ((fh = s_open(b->listpath, "srb")) != EOF)
    {
      while ((_read(fh, b->line, BLEN) == BLEN) && (found < num))
      {
        waitfor(e_ticsfull);
        if (b->line[14] == '>') // Is the message erased?
        {
          if (type == 'c')
          {
            s_close(fh);
            return 1;
          }
          else // type 'l'
          {
            b->line[7] = 0;
            sprintf(*lines, "%s/%s", b->logincall, b->line);
            strupr(*lines);
            *lines += strlen(*lines) + 1;
            found++;
          }
        }
      }
      s_close(fh);
      return found;
    }
    else return 0;
  }
}

/*---------------------------------------------------------------------------*/

void delfwdlistentry (char *entry)
//*************************************************************************
//
//  Deletes an entry from a forward-list and deletes the whole
//  forward-trigger-file if there is no entry left
//  The function decides, in which forward-trigger-file the entry
//  must be
//
//  (DH3MB)
//
//  19980115 OE3DZW reduced priority of err-msges
//
//*************************************************************************
{
  lastfunc("delfwdlistentry");
  char type;
  char entry_left = 0;
  char fwdfname[FNAMELEN+1];
  char found = 0;
  unsigned long int lastpos = 0L, curpos;
  FILE *f;

  if (entry[1] == ' ') type = 'm';
  else
  {
    if (! strchr(entry, '/'))
    {
      trace(replog, "delfwdlistentry", "invalid: %s", entry);
      return;
    }
    *strchr(entry, '/') = 0;
    if (mbcallok(entry)) type = 'u';
    else type = 'i';
    *(entry + strlen(entry)) = '/';
  }
  sprintf(fwdfname, "%s/%c_%s.bcm", FWDPATH, type, b->logincall);
  strlwr(fwdfname);
  waitfor(e_ticsfull);
  if ((f = s_fopen(fwdfname, "sr+t")) != NULL) //fwdtrigger
  {
    while (fgets(b->line, BUFLEN - 1, f))
    {
      curpos = ftell(f);
      if (*b->line != ' ')
      {
        if (stristr(b->line, entry) == b->line)
        {
          fseek(f, lastpos, SEEK_SET);
          fputc(' ', f);
          fseek(f, curpos, SEEK_SET);
          found++;
        }
        else entry_left = 1;
      }
      lastpos = ftell(f);
      waitfor(e_reschedule);
    }
    s_fclose(f);
    if (! entry_left) xunlink(fwdfname);
  }
  if (! found) trace(report, "delfwdlistentry", "not found: %s", entry);
  if (found > 1) trace(report, "delfwdlistentry", "ambiguous: %s", entry);
}

/*---------------------------------------------------------------------------*/

static void putsid (int tx)
//*************************************************************************
//
//  tx = 0 .. incoming connection
//       1 .. outgoing connection
//
//  Erzeugt das SID (System-ID) der eigenen Station.
//  Es wird beim Beginn jeder Forward-Session ausgetauscht
//
//*************************************************************************
{
  lastfunc("putsid");
  char pwdfname[20];
  char pwds[30];
  char sid[60];
  int i;
  FILE *f;

  strcpy(pwds, "");
  sprintf(pwdfname, "%s.pwd", b->logincall);
  strlwr(pwdfname);
  if (file_isreg(pwdfname)) // TheBox style password
  {
    if (tx)
    {
      if (b->privoffset < 0)
      {
        fwdput("*** no logintime", "");
        return;
      }
      else
      {
        f = s_fopen(pwdfname, "srt");
        if (f)
        {
          fseek(f, b->privoffset, SEEK_SET);
          fread(pwds, 4, 1, f);
          pwds[4] = 0;
          s_fclose(f);
        }
        else
          trace(replog, "putsid", ms(m_filenotopen), pwdfname);
      }
    }
    else
      strcpy(pwds, datestr(b->logintime - ad_timezone(), 7)); // UTC
  }
  else
    if (u->password[0] > 1 && u->sfpwtype <= 2) //BayCom-/MD2-/MD5-password
    {
      if (tx)
      {
        if (b->password[0])
        {
          switch (u->sfpwtype)
          {
            case 0:
              { //randomize();
                int rndpos = random_max(6) + 1;
                sprintf(pwds, " ");
                for (int block = 1; block <= 6; block++)
                {
                  for (i = 0; i < 5; i++)
                  sprintf(pwds + strlen(pwds), "%c", (block == rndpos) ?
                     b->password[i] :
                     (u->password[random_max(strlen(u->password))]));
                }
              } break;
            case 1:
            case 2: strcpy(pwds, b->password); break;
          }
        }
      }
      else
      {
        switch (u->sfpwtype)
        {
          case 0: genPWstring(u->password, pwds, b->password); break;
#ifdef FEATURE_MDPW
          case 1: genMDstring(2, u->password, pwds, b->password); break;
          case 2: genMDstring(5, u->password, pwds, b->password); break;
#endif
        }
      }
    }
  sprintf(sid, "[OpenBCM-"VNUMMER"-");
#ifdef _BCMNET_FWD
  if (! tx) strcat(sid, "AB1D1FHMRWZ");
#else
  if (! tx) strcat(sid, "AB1D1FHMRW");
#endif
  else
  {
    if (b->opt & o_a) strcat(sid, "A");
    if (b->opt & o_b)
    {
      strcat(sid, "B");
      if (b->opt & o_b1) strcat(sid, "1");
    }
    if (b->opt & o_d)
    {
      strcat(sid, "D");
      if (b->opt & o_d1) strcat(sid, "1");
    }
    if (b->opt & o_f) strcat(sid, "F");
    strcat (sid, "H");
    if (b->opt & o_m) strcat(sid, "M");
    if (b->opt & o_r) strcat(sid, "R");
    if (b->opt & o_w) strcat(sid, "W");
#ifdef _BCMNET_FWD
    if (b->opt & o_z) strcat(sid, "Z");
#endif
  }
  strcat(sid, "$]");
  if (*pwds) strcat(sid, " ");
  fwdput(sid, pwds);
}

/*---------------------------------------------------------------------------*/

static int fwd_privauswert (char *date, char *pwdfname)
//*************************************************************************
//
//  Hier wird bei einem ankommenen SID der Teil hinter der Klammer
//  ausgewertet, wenn fuer den Partner ein PRIV-File existiert.
//  Abhaengig von der Laenge wird der String entweder als Aufforderung
//  oder als Antwort interpretiert.
//
//*************************************************************************
{
  int day, hour, min;
  lastfunc("fwd_privauswert");

  b->pwok = 0;
  if (strlen(date) != 4)
  { // 2301941530    23.01.94 15:30
    // 0123456789
    while (*date && ! isdigit(*date)) date++;
    if (! *date)
    {
      fwdput("*** no priv logindate", "");
      return 0;
    }
    min = atoi(date + 8);
    date[8] = 0;
    hour = atoi(date + 6);
    date[2] = 0;
    day = atoi(date);
    b->privoffset = ((min + day) % 60) * 27 + hour;
  }
  else
  {
    if (! getpriv(date, pwdfname))
    {
      fwdput("*** priv failure", "");
      pwlog(b->logincall, b->uplink, "fwd priv failure");
      return 0;
    }
  }
  b->pwok = OK;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int fwd_pwauswert (char *str)
//*************************************************************************
//
//  Hier wird bei einem ankommenden SID der Teil hinter der Klammer
//  ausgewertet, wenn kein PRIV-Password gesetzt ist.
//
//  Returns:  0 .. pw failed
//            1 .. pw ok
//
//*************************************************************************
{
  lastfunc("fwd_pwauswert");
  int i = 0;
  unsigned int n[5];

  b->pwok = 0;
  /* sfpwtype:   0  ... BayCom
   *             1  ... MD2
   *             2  ... MD5
   *           100  ... inactive (for fwd/login)
   *           101  ... Errmsg (internal)
   *
   *  password[0]:  0 ... no password set
   *                1 ... password disabled
   *               >1 ... 1st character of password
   */
  if (u->sfpwtype > 99 || u->password[0] <= 1)
  {
  // No password set and the partner did not send one
    if (! *str) return OK;
  // No password set, but the partner did send one
    trace(report, "fwd_pwauswert", "str='%s'", str);
    fwdput("*** no password expected", "");
    return NO;
  }
  // Password set, but partner did not send one
  if (! *str)
  {
    fwdput("*** password expected", "");
    pwlog(b->logincall, b->uplink, "fwd pw empty");
    return NO;
  }
  switch (u->sfpwtype) // DH3MB
  {
  case 0:
    if (sscanf(str, "%d%d%d%d%d", n, n + 1, n + 2, n + 3, n + 4) == 5)
    {
      for (i = 0; i < 5; i++)
      {
        if ((n[i] - 1) < strlen(u->password))
          b->password[i] = u->password[(n[i] - 1)];
        else
        {
          fwdput("*** pw value out of range", "");
          *b->password = 0;
          return NO;
        }
      }
      b->password[5] = 0;
    }
    else
    {
      if (*b->password && strstr(str, b->password))
      {
        b->pwok = OK;
        return OK;
      }
      else
      {
        fwdput("*** pw failure", "");
        pwlog(b->logincall, b->uplink, "fwd pw failure");
        return NO;
      }
    }
    break;
#ifdef FEATURE_MDPW
  case 1: // MD2
  case 2: // MD5
    if (*str == '[')
    {
      char MDstr[12];
      strncpy(MDstr, str + 1, 11);
      if (MDstr[10] != ']')
      {
        fwdput("*** pw request string invalid", "");
        return NO;
      }
      MDstr[10] = 0;
      switch (u->sfpwtype)
      {
        case 1: calcMD2pw(MDstr, u->password, b->password); break;
        case 2: calcMD5pw(MDstr, u->password, b->password); break;
      }
    }
    else
    {
      if (*b->password && ! strcmp(str, b->password))
      {
        b->pwok = OK;
        return OK;
      }
      else
      {
        fwdput("*** pw failure", "");
        pwlog(b->logincall, b->uplink, "fwd pw failure");
        return NO;
      }
    }
    break;
#endif
  }
  b->pwok = OK;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int sidauswert (char *sidp)
//*************************************************************************
//
//  Wertet das ankommende SID des Forwardpartners aus. Hier koennen
//  unterschiedliche Gegebenheiten im Forwardprotokoll festgestellt werden
//
//*************************************************************************
{
  lastfunc("sidauswert");
  char *pkt, *sid;
  char *klzu;
  unsigned int i;
  char pwdfname[20];

  sprintf(pwdfname, "%s.pwd", b->logincall);
  strlwr(pwdfname);
  for (i = 0; i < 50; i++) // 8 Versuche, falls TNC-CText eingestellt (auf 50 dh8ymb 02.07.06)
  { // bei mehr Versuchen -> Funktion kann haengen, da keine Zeilen ankommen!
    sid = sidp; //for that '>' workaround
    fwdget(sid, 118);
    while (*sid == '>') sid++; // workaround for unknown bug at unknown site
    if (*sid == '[' && strchr(sid, '$'))
    {
      klzu = strchr(sid, ']');
      if (klzu && klzu[1] == ' ')
      {
        if (file_isreg(pwdfname))
        {
          if (! fwd_privauswert(klzu + 2, pwdfname)) return NO;
        }
        else
          if (! fwd_pwauswert(klzu + 2)) return NO;
      }
      b->opt = 0;
      klzu = strchr(sid, ']');
      //OE3DZW: Auswertung der Optionsbuchstaben, Versionsnummer wird nicht
      //ausgewertet, $ wird fix vorausgesetzt
      while (klzu && *(--klzu) != '-' && klzu > sid)
      {
        if (isalpha(*klzu))
          b->opt |= (1L << (toupper (*klzu) - 'A'));
      }
#ifdef _BCMNET_GW
      //Mindestanforderungen fuers CB-BCMNET Gateway (db1ras)
      if (! ((b->opt & o_m) && ((b->opt & o_w) || (b->opt & o_d))))
        return NO;
#elif defined _BCMNET_FWD
      if (! (b->opt & o_z))
        return NO;
#endif
      // test for D1..D4, B1..B4
      if (strstr(sid, "D1") > strrchr(sid, '-')) b->opt |= o_d1;
      if (strstr(sid, "D2") > strrchr(sid, '-')) b->opt |= o_d1;
      if (strstr(sid, "D3") > strrchr(sid, '-')) b->opt |= o_d1;
      if (strstr(sid, "D4") > strrchr(sid, '-')) b->opt |= o_d1;
      if (strstr(sid, "B1") > strrchr(sid, '-')) b->opt |= o_b1;
      if (strstr(sid, "B2") > strrchr(sid, '-')) b->opt |= o_b1;
      if (strstr(sid, "B3") > strrchr(sid, '-')) b->opt |= o_b1;
      if (strstr(sid, "B4") > strrchr(sid, '-')) b->opt |= o_b1;
      strupr(sid);
      if (strstr(sid, "BAYCOM") || strstr(sid, "OPENBCM"))
        b->boxtyp = b_baycom;
      else if (strstr(sid, "THEBOX"))
             b->boxtyp = b_thebox;
      else if (strstr(sid, "DP"))
             b->boxtyp = b_dp;
      else if (strstr(sid, "FBB"))
             b->boxtyp = b_fbb;
      else if (strstr(sid, "RLI"))
             b->boxtyp = b_rli;
      else if (strstr(sid, "WINGT") || strstr(sid, "WINBOX"))
             b->boxtyp = b_wgt;
      else
             b->boxtyp = b_unknown;
      b->boxversion = 0;
      if ((pkt = strchr(sid, '-')) != 0)
      {
        if (isdigit(pkt[1])) b->boxversion = (pkt[1] & 15) * 10;
        if (isdigit(pkt[3])) b->boxversion += (pkt[3] & 15);
      }
      return OK;
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

static int waitconnect (char *buf, int fpac)
//*******************************************************************
//
// Wartet/Wertet Connect-Meldung aus
//
// OE3DZW: Diese Texte sind fix eingestellt, vgl. FBB, dort ist eine
// Sysop-Konfiguration notwendig.
//*******************************************************************
{
  lastfunc("waitconnect");

  for (;;)
  {
    fwdget(buf, LINELEN);
    strupr(buf);
    //trace(report, "waitconnect", "%d:%s", fpac, buf);
    if (strstr(buf, "RECONNECTED"))
    { trace(report, "waitconnect", "'RECONNECTED' detected"); return NO; }
    if (! fpac) //BayCom, Flexnet, Nordlink-Connect-String
    {
      if (strstr(buf, "CONNECTED TO")) return OK;
    }
    else  //FPAC-Connect-String
    { //france
      if (strstr(buf, " @ ") && strstr(buf, "CONNECTE"))
        return OK; // "Connecte a F6KIF-1 @ 208651501"
      //slovenia vzpostavljena /croatia
      if (strstr(buf, "ZPOSTAVLJEN"))
        return OK;
    }
    if (strstr(buf, "INVALID"))
    { trace(report, "waitconnect", "'INVALID' detected"); return NO; }
    if (strstr(buf, "FAILURE"))
    { trace(report, "waitconnect", "'FAILURE' detected"); return NO; }
    if (strstr(buf, "BUSY"))
    { trace(report, "waitconnect", "'BUSY' detected"); return NO; }
    if (strstr(buf, "LOOP"))
    { trace(report, "waitconnect", " 'LOOP' detected"); return NO; }
    if (strstr(buf, "FULL"))
    { trace(report, "waitconnect", " 'FULL' detected"); return NO; }
    if (strstr(buf, "NO CHAN"))
    { trace(report, "waitconnect", "'NO CHAN' detected"); return NO; }
    if (strstr(buf, "NO ROUT"))
    { trace(report, "waitconnect", "'NO ROUT' detected"); return NO; }
    if (strstr(buf, "T ROUTE ")) // dh8ymb: das space am ende ist wichtig!
    { trace(report, "waitconnect", "'T ROUTE' detected"); return NO; }
    //slovenia
    if (strstr(buf, "PODRTA"))
    { trace(report, "waitconnect", "'PODRTA' detected"); return NO; }
    //croatia
    if (strstr(buf, "PREKINUTA"))
    { trace(report, "waitconnect", "'PREKINUTA' detected"); return NO; }
  }
}

/*---------------------------------------------------------------------------*/

void fwdsend (char *call)
//*******************************************************************
//
//
//*******************************************************************
{
  lastfunc("fwdsend");
  unsigned int i = 0, j = 0;
  int fpac, newsearch;
  char buf[132];
  char hilfsbuf[132];
  char mycall[20];
  char rauscall[15];
  char connpfad[FSPATHLEN];
  char *cc;
  time_t sidt;
  fwdpara_t *ff;
  char lbuf[80];
  static handle fa = EOF;
  unsigned pos;
#ifdef _FILEFWD
  char *test;
  char parabuf[50];
  char optionfilefwd[5];
  char importfile[20];
  char exportfile[20];
#endif

  *buf = 0;
  *hilfsbuf = 0;
  if (m.disable == 1) return;
  b->continous = 1;
  b->job = j_fwdlink;
  strupr(call);
  strcpy(b->logincall, call);
  if (isforwardpartner(call) == NOTFOUND)
    b->forwarding = fwd_user;
  else
    b->forwarding = fwd_standard;
  if (another_fwd_sender(call)) return;
  ff = fpara();
  strcpy(connpfad, ff->concall);
  if (ff->nolink == 0)
  {
    // falls dies der erste Connect ist oder der letzte Connect ok war
    erase_igate(connpfad);
  }
  cc = connpfad;
  if (! strncasecmp(ff->concall, "FILE", 4))
  {
#ifdef _FILEFWD
    test = ff->concall;
    test = nextword(test, "FILE:", 5);
    if (*test)
    {
      test = nextword(test, importfile, 20);
      if (*test)
      {
        test = nextword(test, exportfile, 20);
        if (*test) test = nextword(test, optionfilefwd, 1);
        else optionfilefwd[0] = 0;
      }
      else exportfile[0] = optionfilefwd[0] = 0;
    }
    else
      importfile[0] = exportfile[0] = optionfilefwd[0] = 0;
    if (*importfile)
      sprintf(parabuf, "%s %s/%s", ff->call, FWDIMPATH, importfile);
    else
      sprintf(parabuf, "%s %s/%s.imp", ff->call, FWDIMPATH, ff->call);
    fwd_import(parabuf);
    if (*exportfile)
      if (*optionfilefwd)
        sprintf(parabuf, "%s %s %s/%s", ff->call, optionfilefwd,
                                        FWDEXPATH, exportfile);
      else
        sprintf(parabuf, "%s %s/%s", ff->call, FWDEXPATH, exportfile);
    else
      if (*optionfilefwd)
        sprintf(parabuf, "%s %s %s/%s.exp", ff->call, optionfilefwd,
                                            FWDEXPATH, ff->call);
      else
        sprintf(parabuf, "%s %s/%s.exp", ff->call, FWDEXPATH, ff->call);
    fwd_export(parabuf);
    return;
#endif
  }
  strcpy(mycall, m.mycall[0]);
  subst1(mycall, '-', 0);
  if (ff->ssid)
    sprintf(rauscall, "%s-%u", mycall, ff->ssid);
  else
    strcpy(rauscall, mycall);
  if (ff->ssid == m.fwdssid) // change SSID in relation to last connect
  {                          // to avoid similar address fields
    ff->ssid += 1;
    if (ff->ssid > 15) ff->ssid = 1; // avoid ssid zero
  }
  else
    ff->ssid = m.fwdssid; // toggle between both ssids
  strcpy(b->logincall, ff->call);
  while (*cc && *cc != '/')
    buf[i++] = *cc++;
  buf[i] = 0;
  cut_blank(buf);
  ff->connectbar = 0;
  trace(report, "fwdsend", "%s (%s > %s)", call, rauscall, buf);
  lastcmd("fwd_connect");
  sidt = ad_time();
  /*------------------*/
  // ACTIVE ROUTING
  ff->nolink++;
  if (mbcallok(ff->call) && ff->nolink == 4)
  {
    wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wps");
    strcpy(wp->bbs, ff->call);
    wp->bversion = BVERSION;
    ff->lastwpr = wp->mybbstime = ff->lastmeasure = ad_time();
    wp->hops = 1; // wird in addwp_r +1 erhoeht
    wp->qual = WPROT_MIN_ROUTING;
    hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
    hadr_tryopen();
    pos = loadhadr(ff->call, ha, 0);
    if (pos)
    {
       ha->r_qual_rx = WPROT_MIN_ROUTING;
       lseek(fa, (long) pos * sizeof(hadr_t), SEEK_SET);
       _write(fa, ha, sizeof(hadr_t));
       sprintf(lbuf, "M %-6.6s %-6.6s %s (%ld) %ld 1   %-5ld NO LINK\n",
                     ff->call, m.boxname, datestr(ff->lastmeasure, 12),
                     ff->lastmeasure, wp->bversion, wp->qual);
       wprotlog(lbuf, ff->call);
       wprotlog("D ", ff->call);
       addwp_r(wp);
    }
    uclose();
    t_free(ha);
    t_free(wp);
  }
  /*------------------*/
#ifdef _TELEPHONE // JJ
  if (strstr(buf, "DUMMY")) return;
  if (! strstr(buf, "TTY") && strncmp(b->uplink, "TTY", 3))
  {
#endif
#ifdef _TELNETFWD
  if (! stristr(buf, "telnet"))
  {
#endif
  if (! makeconnect(rauscall, buf))
  {
    fwdlog("---- ", "connect failed", '-');
    *b->logincall = 0;
    return;
  }
  setsession();
  *b->logincall = 0;
  mblogin(ff->call, login_fwd, "");
#ifdef _TELNETFWD
  }
  else
  {
    timeout(m.fwdtimeout);
    if (! telnet_fwd_connect(buf))
    {
      fwdlog("---- ", "connect failed", '-');
      *b->logincall = 0;
      return;
    }
    *b->logincall = 0;
    mblogin(ff->call, login_fwd, "TELNET");
  }
#endif
#ifdef _TELEPHONE
  }
  else
  { // JJ: TTY-FWD start
    if (strncmp(b->uplink, "TTY", 3))
    {
      if (! isdialtime(call)) return;
      if (! tty_dial(buf, t->taskid))
      {
        fwdlog("---- ", "connect failed", '-');
        *b->logincall = 0;
        return;
      }
    }
    setsession();
    t->input = io_tty;
    t->output = io_tty;
    *b->logincall = 0;
    mblogin(ff->call, login_fwd, "TTY");
    //JJ: UPLINK WICHTIG fuer spaetere Abfragen
  }
#endif
// trace(serious, "fwdsend", "conn ok - lo %s, ff %s, ca %s",
//                            b->logincall, ff->call, call);
  fwdlog("---- ", "connect ok", '-');
  timeout(m.fwdtimeout);
  while (*cc == '/')
  {
    cc++;
    i = 0;
//    trace(report, "fwdsend", "%s", buf);
    while (*cc && *cc != '/') buf[i++] = *cc++;
    buf[i] = 0;
    cut_blank(buf); // Remove tailing blanks
    i = 0;
    while (buf[i] == ' ') i++;
    // simple hack for fpac-digis:
    // when connect-string starts with "%" -> expect fpac-connect-text
    fpac = 0;
    if (buf[i] == '%')
    {
      i++;
      fpac = 1;
    }
    if (buf[i] == '=')
    {
       newsearch = TRUE;
       while (newsearch)
       {
         for (j = 0 ; j < 132 ; j++) hilfsbuf[j] = 0; // Initialisierung
         j = i+1;
         while (buf[j] && buf[j] != '=') // weiteres = suchen
         {
           hilfsbuf[j-i-1] = buf[j];
           j++;
         }
         hilfsbuf[j-2] = 0;
         if (*hilfsbuf)
         {
           wdelay(1241);
           cut_blank(hilfsbuf); // Remove tailing blanks
           fwdput(hilfsbuf, "");
           i = j;
         }
         else
           newsearch = FALSE;
       }
    }
    else
      fwdput("C ", buf + i);
    putflush();
    lastcmd(buf);
    if (! waitconnect(buf, fpac))
    {
      mblogout(1);
      return;
    }
  }
  ff = fpara();
  if (ff->options & o_f) // OE3DZW: We send CR to TCP/IP-boxes
  {
    fwdput("", "");
    putflush();
  }
#ifdef _TELNETFWD
  if (t->input == io_socket) // send our call&pwd
  {
    fwdput(m.boxname, "");
    putflush();
    if (*u->ttypw)
    {
      fwdput(u->ttypw, "");
      putflush();
    }
  }
#endif
  // OE3DZW: ...for...
  lastcmd("wait_for_sid");
  // DH3MB: Changed structure of sending/receiving prompts/SIDs (for FBBfwd)
  if (sidauswert(buf))
  {
    lastcmd("wait_pr1");
    if (waitfwdprompt(buf)) // First prompt (we now may send our SID)
    {
      putsid(1);
      ff->connectbar = 1;
      ff->lastconok = ad_time();
      ff->n_conok++;
      ff->nolink = 0;
      ff->sidrtt = ff->lastconok - sidt + 1;
      gen_wpmail(b->logincall);
      if (b->opt & o_f)
      {
        fbb fbbfwd;
        fwdlog("---- ", "[FBB Fwd outgoing connection]", '-');
        fbbfwd.fwdmachine(1);
      }
      else
      {
        fwdlog("---- ", "[RLI Fwd outgoing connection]", '-');
        lastcmd("wait_pr2");
        if (waitfwdprompt(buf)) //second prompt (we now send a RLI-command)
          ascii_fwdmachine(1);
      }
    }
  }
#ifdef _TELNETFWD
  if (t->input == io_socket)
    disconnect_sock();
  else
#endif
  {
    mblogout(0);
  }
}

/*---------------------------------------------------------------------------*/

int fwdmainloop (void)
//*************************************************************************
//
//  Handles an incoming S&F-connect
//
//*************************************************************************
{
  lastfunc("fwdmainloop");
  char s[121];

  strcpy(s, "");
  b->continous = 1;
  b->job = j_fwdrecv;
  trace(report, "fwdlogin", "%s via %s", b->logincall, b->uplink);
  fwdlog("---- ", "login", '-');
  fpara()->lastlogin = ad_time();
  fpara()->n_login++;
  if (! eingelogt(b->logincall, j_fwdrecv, 0))
  {
    if (fpara()->options & o_f)
    {
      fwdput("", "");
      putflush();
    }
    putsid(0);
    lastcmd("rx_wait_sid");
    timeout(m.fwdtimeout);
    if (! (fpara()->options & o_p)) fwdput(">", "");
    if (sidauswert(s))
    {
      gen_wpmail(b->logincall);
      if (b->opt & o_f)
      {
        fbb fbbfwd;
        fwdlog("---- ", "[FBB Fwd incoming connection]", '-');
        fbbfwd.fwdmachine(0);
      }
      else
      {
        fwdlog("---- ", "[RLI Fwd incoming connection]", '-');
        ascii_fwdmachine(0);
      }
    }
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

int weiterleiten4 (int setmybbs, char *boardname, int expandforce)
//*************************************************************************
//
//  Hier werden ankommende Adressen/Verteiler ausgewertet.
//
//  Dabei koennen folgende Ergebnisse herauskommen: (enum fwd_weiter_t)
//
//   0=unbekannt    - Adresse konnte nicht sinnvoll ausgewertet werden
//                  bzw. es ist kein Forwardpartner bekannt, der einen
//                  passenden Eintrag hat.
//
//   1=bekannt      - Eintrag wurde gefunden, zurueckgelieferte Boxenliste
//                  zeigt die Boxen, zu denen das File geschickt werden
//                  soll
//
//   2=bleibtliegen - Es ist entweder gar keine Adresse angegeben, oder
//                  die Adresse ist das eigene Call. In beiden Faellen
//                  ist die Angabe gueltig, fuehrt aber nicht zum
//                  Weiterleiten der Nachricht.
//
//   3=auto_bekannt - Eintrag wurde vom Autorouter gefunden,
//                  zurueckgelieferte Boxenliste zeigt die Boxen, zu denen
//                  das File geschickt werden soll
//
//   4=active_bekannt - Eintrag wurde vom Active-Routing gefunden
//
//*************************************************************************
{
  lastfunc("weiterleiten4");
  char *dst;
  dst = boxdest;
  char adr[HADRESSLEN+1], *ptr;
  char wort[HADRESSLEN+1];
  char dest[HADRESSLEN+1];
  char testboard[DIRLEN+2];
  char atc[CALLSSID+1];
//  char *substring;
  unsigned int tab, i, match = 0;
  int callok = mbcallok(boardname);
#ifdef __FLAT__
 #ifdef __LINUX__
  unsigned long long ziele = LONG_ZERO, reject = LONG_ZERO;
 #else // Win32
  __int64 ziele = LONG_ZERO, reject = LONG_ZERO;
 #endif
#else
  unsigned long ziele = LONG_ZERO, reject = LONG_ZERO;
#endif

  *b->destboxlist = 0;
  strcpy(adr, b->at);
#ifdef DEBUG_FWD //df3vi: for debugging
  trace(report, "weiterleiten4", "callok1: %d adr1: %s", callok, adr);
#endif
  i = mbhadrok(adr);
  // 1.: Adresse ist nicht sinnvoll
  // 2.: Usermail @ flood
  if ((*adr && ! i) || (callok && i == 2)) return unbekannt;
  // 3.: Bulletin @ flood
  // 4.: Bulletin @ h-adr
  // 5.: Usermail @ h-adr
  //Ist Adresse eigenes Call?
  strcpy(atc, atcall(adr));
  if (*atc && strstr(m.boxname,atc))
    subst1(atc, '-', 0); //teste Eintrag ohne SSIDs, falls nicht eigene Box
  //Wenn es von einer anderen Box kommt, dann die Adresse entfernen,
  //damit evtl. MYBBS greifen kann.
  if (mbcallok(atc) && ! strcmp(atc, m.boxname) && b->forwarding != fwd_none)
    *adr = 0;
  //User-Mail ohne Adresse? -> Mybbs suchen
  if (callok == 1 && (! *adr || setmybbs))
  {
    get_mybbs(boardname, adr, setmybbs);
    if (*adr) strcpy(b->at, adr);
  }
#ifdef DEBUG_FWD //df3vi: for debugging
  trace(report, "weiterleiten4", "callok2: %d adr2: %s", callok, adr);
#endif
  if (! *adr)
    return callok ? unbekannt : bleibtliegen; //"rufzeichen@ " ist unbekannt,
                                              //"rubrik@ " bleibt liegen
#ifdef DEBUG_FWD
  trace(report, "weiterleiten4", "atc1: %s", atc);
#endif
  strcpy(atc, atcall(adr));
  if (*atc && strstr(m.boxname, atc))
    subst1(atc, '-', 0); //SSID entfernen, falls nicht eigene Box
  if (*atc && mbcallok(atc)) //H-Adresse beginnt mit gueltigem Rufzeichen
  {
    if (! strcmp(atc, m.boxname))
      return bleibtliegen; // ist es an das eigene Boxcall gerichtet?
#ifdef DEBUG_FWD
  trace(report, "weiterleiten4", "atc2: %s", atc);
#endif
    strcpy(dest, atc);
    expand_hadr(dest, expandforce); //Adresse expandieren
    if (! strchr(dest, '.')) strcpy(dest, adr);
    else strcpy(b->at, dest);
    strip_hadr(dest); //H-Adr um Teil der eigenen Adr. befreien
    if ((ptr = strchr(dest, '.')) != NULL)
      strcpy(adr, ptr); //und Rufzeichen davor weg
    else strcpy(adr, dest);
  }
  else
    strcpy(dest, adr);
  if (! callok) //falls Board, Board-Test vorbereiten
  {
    strncpy(testboard + 1, boardname, DIRLEN);
    testboard[DIRLEN + 1] = 0;
    *testboard = '*'; //Ergebnis sieht z.B. so aus: *TECHNIK
  }
  else
    *testboard = 0;

  //Suche nach H-Adressen
  i = strlen(adr);
  for (tab = 0; tab < fwddest; tab++)
  {
    waitfor(e_ticsfull);
    dst = nextword(dst, wort, HADRESSLEN);
    if (   (*testboard && ! strcmp(wort,testboard) && dest_idx[tab]) //found reject board
        || (*atc && *wort == '*' && ! strcmp(atc, wort + 1)) //found reject address
       )
      reject = dest_idx[tab]; //Testboard gefunden
    if (strlen(wort) > match && dest_idx[tab])
    {
      if (*atc && ! strcmp(atc, wort))
      { //direkten Eintrag des Zielrufzeichens gefunden
        match = HADRESSLEN; //auf Maximum setzen - besser geht nicht
        ziele = dest_idx[tab];
        continue; //weitersuchen nach *board
      }
      if (   (*adr && ! strcmp(adr, wort)) //wort passt exakt
          || (   *adr && i > strlen(wort)  //wort kleiner als adr
              && (*wort == '.' || mbhadrok(wort) == 1) //wort ist H-Adr.-Teil
              && ! strcmp(adr + i - strlen(wort), wort) ) ) //wort passt
      { //Adresse/Verteiler gefunden
        match = strlen(wort);
        ziele = dest_idx[tab]; //Ziele speichern
      }
    }
  }
#ifdef _AUTOFWD
  strcpy(adr, dest); //dest in adr sichern fuer Autorouter
#endif

/*------------------*/

// ACTIVE ROUTING
char my[HADRESSLEN + 1] = { 0 };
int dlnet = FALSE;
unsigned pos;

if (mbcallok(atc))
{
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");
  hadr_tryopen();
  pos = loadhadr(atc, ha, 0);
  if (pos)
  {
    strcpy(my, m.boxname);
    expand_hadr(my, 3);
    if (strstr(my, "DLNET") && strstr(ha->adr, "DLNET"))
      dlnet = TRUE;
    else
      if (ha->r_qual_rx > 0
          && ha->r_qual_rx < WPROT_MIN_ROUTING
          && ((ad_time() - ha->r_time_rx) < 3*DAY)
          && ((ad_time() - ha->r_time_rx) >= 0) // keine Zukunft
         )
        strcat(b->destboxlist, ha->r_from);
  }
  uclose();
  t_free(ha);
  if (*b->destboxlist
      && (isforwardpartner(b->destboxlist) >= 0)
      && ! dlnet)
    return active_bekannt;
}

/*------------------*/

  if (! ziele)
  {
    //Keine Adresse gefunden? Dann Praefixsuche...
    subst1(dest, '.', 0); //H-Teil weg
    if (! *dest) return unbekannt;
    dst = boxdest; //wieder von vorne suchen...
    for (tab = 0; tab < fwddest; tab++)
    {
      waitfor(e_ticsfull);
      dst = nextword(dst, wort, HADRESSLEN);
      if (! strcmp(wort, "**") && match == 0) //Defaultroute
      {
        //der Defaultroute sollte die geringste Bedeutung zukommen und nur
        //benutzt werden, wenn nichts anderes passt. Deshalb setzen wir
        //an dieser Stelle kein "match" (db1ras)
        ziele = dest_idx[tab]; //Ziele speichern
        continue;
      }
      if ((ptr = strchr(wort, '*')) > wort) //Stern ist NICHT am Anfang
      {
        *ptr = 0;
        if (strstr(dest, wort) == dest) //Praefix gefunden
        {
          if (strlen(wort) > match && dest_idx[tab]) //UND passt besser
          {
            match = strlen(wort);
            ziele = dest_idx[tab]; //Ziele speichern
          }
        }
      }
    }
#ifdef _AUTOFWD
    if (m.autofwdtime && ! match)
    {
      //we give the autorouter the full h-addr (inclusive boxcall), this is
      //needed if the route only can be determinated by the boxcall (hpk)
      if (autofwd(adr, 0) == auto_bekannt)
      {
        //autofwd hat das Boxrufzeichen bereits in b->destboxlist abgelegt
        if (strlen(b->destboxlist) < DESTBOXLEN) strcat(b->destboxlist, " ");
        return auto_bekannt;
      }
    }
#endif
    //Defaultroute nicht vergessen, welche nur an gueltigem ziele erkannt
    //werden kann (db1ras)
    if (! ziele)
      return unbekannt; //nichts gefunden
  }
  //Board-Reject Pruefung
  for (i = 0; i < fwdpartners; i++) //alle Boxen auf Eintrag pruefen
  {
    waitfor(e_ticsfull);

    if (reject & (LONG_ONE << i))
      ziele &= (~(LONG_ONE << i)); // wenn reject, Ziel loeschen (DK2UI)
    if ((ziele & (LONG_ONE << i))
        || (reject & (LONG_ONE << i))) //Box hat Eintrag
    {
      //wenn userfile, immer senden, sonst:
      //1. bed: board enthalten und option -R (nur boards)
      //2. bed: board fehlt und nicht option -R
      if (   callok
          || (reject & (LONG_ONE << i) && !! (fp[i].options & o_r))
          || (! (reject & (LONG_ONE << i)) && ! (fp[i].options & o_r)))
      {
        if ((strlen(b->destboxlist) + strlen(fp[i].call) + 1) < DESTBOXLEN)
        {
          strcat(b->destboxlist, fp[i].call);
          strcat(b->destboxlist, " ");
        }
      }
    }
  }
  if (*b->destboxlist) return bekannt;
  else return unbekannt;
}

/*---------------------------------------------------------------------------*/

int weiterleiten (int setmybbs, char *boardname)
//*******************************************************************
//
// Check for converted address if original address was not found
//
//*******************************************************************
{
  int erg1;
  char adr[HADRESSLEN+1];
  user_t us;

  lastfunc("weiterleiten");
  if ((erg1 = weiterleiten4(setmybbs, boardname, m.hadrstore))
      == unbekannt && mbhadrok(b->at))
  {
#ifdef DEBUG_FWD //df3vi: for debugging
    trace(report, "weiterleiten", "%d", erg1);
#endif
    strcpy(adr, b->at);
    convat(b->at);
    if (strcmp(b->at, adr)) //dk2ui, nur wenn convat erfolgreich
    {
      if ((erg1 = weiterleiten4(setmybbs, boardname, m.hadrstore + 1))
          != unbekannt) return erg1;
    }
    //df3vi: wenn unbekannt, homebbs aus users.bcm suchen
    strcpy(b->at, adr);
    if (mbcallok(boardname))
    {
      if (loaduser(boardname, &us, 0))
      {
        if (*us.mybbs)
        {
          strcpy(b->at, us.mybbs); //adresse wird korrigiert
          if ((erg1 = weiterleiten4(setmybbs, boardname, m.hadrstore + 1))
              == unbekannt)
            strcpy(b->at, adr);
        }
      }
    }
  }
#ifdef DEBUG_FWD //df3vi: for debugging
  trace(report, "weiterleiten", "%s@%s %d", boardname, b->at, erg1);
#endif
  return erg1;
}

/*---------------------------------------------------------------------------*/

int remove_oldentry (char *fwdfname, char *entry)
//*******************************************************************
//
// Sucht gleiche Eintraege im Fwd-Queue-File und loescht diese indem
// das erste Zeichen auf SPACE gesetzt wird
//
//*******************************************************************
{
  lastfunc("remove_oldentry");
  FILE *fwdfile = NULL;
  int found = 0;
  unsigned long int lastpos = 0L, curpos;

  waitfor(e_ticsfull);
  //sema lock
  while (! sema_lock(fwdfname)) wdelay(1242);
  if ((fwdfile = s_fopen(fwdfname, "sr+t")) != NULL) //fwdtrigger
  {
    while (fgets(b->line, 31, fwdfile))
    {
      curpos = ftell(fwdfile);
      if (*b->line != ' ')
      {
        if (strpos(b->line, entry) > 0)
        {
          fseek(fwdfile, lastpos, SEEK_SET);
          fputc(' ', fwdfile);
          fseek(fwdfile, curpos, SEEK_SET);
#ifdef DEBUG_FWD
          trace(report, "remove_oldentry", "one: %s %s", entry, b->line);
#endif
          found++;
        }
      }
      lastpos = ftell(fwdfile);
      waitfor(e_reschedule);
    }
    s_fclose(fwdfile);
  }
#ifdef DEBUG_FWD
  if (found == 1) trace(report, "remove_oldentry", "one: %s", entry);
  if (found > 1) trace(report, "remove_oldentry", "more: %s", entry);
#endif
  sema_unlock(fwdfname);
  return found;
}

/*---------------------------------------------------------------------------*/

void add_fwdfile (char *frombox, short unsigned delaytime, int replace)
//*************************************************************************
//
//  Legt den angegebenen Filenamen in den Forwardlisten ab, die
//  zu den Nachbarboxen in (boxenliste) gehoeren.
//
//*************************************************************************
{
  char fwdfname[20];
  char boxcall[CALLEN+1];
  char *boxenliste = b->destboxlist;
  FILE *fwdfile;
  char ds[20] = "";
  int nt = 0, removed = 0;
  char entry[40];
  char entrywritten;
  char fwdfnametmp[30];
  char holdc;
  FILE *fwdnewfile;
  long length;
  int box;

  lastfunc("add_fwdfile");
  subst1(boxenliste, '.', 0);
  b->usermail = mbcallok(b->boardname);
#ifdef DEBUG_FWD
  trace(report, "add_fwdfile", "check: %s/%s", b->boardname, b->mailfname);
  trace(report, "add_fwdfile", "destboxlist: %s", b->destboxlist);
#endif
  //df3vi: echtes HOLD eingebaut (auch fuer transfer und forward!!!)
  //       (Weiterleitung wird um "m.holdtime" Stunden verzoegert)
  if (m.holdtime && *b->at && ! b->sysop)
  {
    holdc = searchreject();
#ifdef DEBUG_FWD
    trace(report, "add_fwdfile", "holdflag: %s %c", b->mailfname, holdc);
#endif
//  H = All Messages must be set on hold
//  F = Message must be set on hold, if forwarded without fwd-pw
//  L = Message must be set on hold, if entered locally
//  P = Message must be set on hold, if entered locally without ax25pw
//  O = Message must be set on hold, if entered locally without ax25pw/ttypw
//  Z = Message must be set on hold, if it is sent from a CB-BCMNET guest
    switch (holdc)
    {
      case 'H':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("general hold", 0);
        if (b->forwarding == fwd_none)
          putf("Message on hold for %i hours\n", m.holdtime);
        break;
      case 'F':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("hold FWD w/o PW", 0);
        break;
      case 'L':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("local hold", 0);
        if (b->forwarding == fwd_none)
          putf("Local message on hold for %i hours\n", m.holdtime);
        break;
      case 'P':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("hold w/o PW", 0);
        if (b->forwarding == fwd_none)
          putf("Message without AX25PW on hold for %i hours\n", m.holdtime);
        break;
#ifdef _BCMNET_LOGIN
      case 'Z':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("hold for guest", 0);
        if (b->forwarding == fwd_none)
        {
          if (delaytime/60 < 24)
            putf("Message from a guest on hold for %i hours.\n",
                 delaytime);
          else
            putf("Message from a guest on hold for %i days.\n",
                 delaytime/(60*24));
        }
        break;
#endif
#ifdef __FLAT__
      case 'O':
        b->fwdhold = holdc;
        delaytime = m.holdtime * 60;
        add_hold("hold w/o PW", 0);
        if (b->forwarding == fwd_none)
          putf("Message without AX25PW/TTYPW on hold for %i hours\n",
               m.holdtime);
        break;
#endif
    }
  }
  // DH3MB: Removed delaytime check
  if (delaytime)
    sprintf(ds, " %ld", ad_time() + ((long) delaytime) * 60);
#ifdef DEBUG_FWD
  trace(report, "add_fwdfile", "mailfname1: %s %s",
                               b->mailfname, boxenliste);
#endif
  while (boxenliste = nexttoken(boxenliste, boxcall, CALLEN), *boxcall)
  {
    box = isforwardpartner(boxcall);
#ifdef DEBUG_FWD
    trace(report, "add_fwdfile", "boxcall: %s", boxcall);
#endif
    if (box != NOTFOUND && (b->usermail || strcmp(frombox, boxcall)))
    {
      if (b->usermail)
      {
        fp[box].newmail = 2;
        strcpy(fwdfname, FWDPATH"/u_");
      }
      else
      {
        fp[box].newmail = 1;
        strcpy(fwdfname, FWDPATH"/i_");
      }
      strcat(fwdfname, boxcall);
      strcat(fwdfname, ".bcm");
      strlwr(fwdfname);
#ifdef DEBUG_FWD
      trace(report, "add_fwdfile", "mailfname2: %s", b->mailfname);
#endif
      if (strcmp(boxcall, DUMMY))
      {
        // forward by size
        if (file_isreg(fwdfname)
            && (fp[box].options & o_o) // wieder optional...
#ifdef _BCMNET_FWD
            && ! b->binstart                      // and no binary mail
            && (b->conttype < '6' || b->conttype > '9') // and no 7plus mail
#endif
           )
        {
          sprintf(entry, "%s", b->mailfname);
#ifdef DEBUG_FWD
          trace(report, "add_fwdfile", "mailfname3: %s", b->mailfname);
#endif
          removed = remove_oldentry(fwdfname, entry);
          if (! replace
              || (replace && removed > 0))
          {
            entrywritten = 0;
            strcpy(fwdfnametmp, fwdfname);
            subst1(fwdfnametmp, '_', '-'); // Replace '_' by '-'
            while (! sema_lock(fwdfname)) wdelay(1542); //sema lock
            while (! sema_lock(fwdfnametmp)) wdelay(1542); //sema lock
            if (   ((fwdfile = s_fopen(fwdfname, "srt")) != NULL)
                && ((fwdnewfile = s_fopen(fwdfnametmp, "swt")) != NULL))
            {
              while (fgets(b->line, BUFLEN - 1, fwdfile))
              {
               // nur nicht bereits als geloescht markierte Eintraege
               // bearbeiten (db1ras)
               if (*b->line != ' ')
               {
                if (strchr(b->line, ' '))
                  length = atol(strchr(b->line, ' ') + 1);
                else
                  length = 0L;
                if (b->bytes < length && ! entrywritten)
                {
                  fprintf(fwdnewfile, "%s/%s %ld%s\n",
                                    b->boardname, b->mailfname, b->bytes, ds);
                  entrywritten = 1;
                }
                fputs(b->line, fwdnewfile);
               }
                //switch task every 200 lines
                if (nt++ == 200)
                {
                  waitfor(e_ticsfull);
                  nt = 0;
                }
              }
              if (! entrywritten)
                fprintf(fwdnewfile, "%s/%s %ld%s\n",
                                    b->boardname, b->mailfname, b->bytes, ds);
              s_fclose(fwdnewfile);
              s_fclose(fwdfile);
              xunlink(fwdfname);
              xrename(fwdfnametmp, fwdfname);
            }
            else if (fwdfile != NULL)
              s_fclose(fwdfile);
            sema_unlock(fwdfname);
            sema_unlock(fwdfnametmp);
          }
        }
        else
        {
          sprintf(entry, "%s", b->mailfname);
#ifdef DEBUG_FWD
          trace(report, "add_fwdfile", "mailfname3: %s", b->mailfname);
#endif
          removed = remove_oldentry(fwdfname, entry);
          if (! replace
              || (replace && removed > 0))
          {
            while (! sema_lock(fwdfname)) wdelay(1542); //sema lock
            fwdfile = s_fopen(fwdfname, "sat");
            if (! fwdfile) // wenn es nicht gegangen ist, ist evtl.
            {              // das Directory noch gar nicht angelegt
              mkdir(FWDPATH);
              fwdfile = s_fopen(fwdfname, "sat");
            }
            if (fwdfile)
            {
              fprintf(fwdfile, "%s/%s %ld%s\n",
                      b->boardname, b->mailfname, b->bytes, ds);
#ifdef DEBUG_FWD
              trace(report, "add_fwdfile", "added: %s/%s %ld%s\n",
                     b->boardname, b->mailfname, b->bytes, ds);
#endif
              s_fclose(fwdfile);
            }
            else
              trace(serious, "add_fwd", "fopen %s errno=%d %s",
                    fwdfname, errno, strerror(errno));
            sema_unlock(fwdfname);
            waitfor(e_reschedule);
            if (// ! delaytime &&
                   isforwardtime(boxcall, ! b->usermail) == 2
                && ! (fp[box].options & o_d))
              fork(P_BACK | P_MAIL, 0, fwdsend, boxcall);
          }
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void trigger_ufwd (char *call)
//*******************************************************************
//
//
//*******************************************************************
{
  if (get_ufwd(call)[0] && scan_userfile(call, 1))
    fork(P_BACK | P_MAIL, 0, fwdsend, call);
}

/*---------------------------------------------------------------------------*/

static int testfwd (char *call, int bulletin)
//*******************************************************************
//
// Checkt ob Forward fuer eine Partnermailbox vorliegt und stoesst
// ggf. den Forward-Connect an
//
//*******************************************************************
{
  lastfunc("testfwd");
  char fname[FNAMELEN+1];
  long mails = 0L;
  FILE *f = NULL;
  char s[LINELEN+41];

  wdelay(1861);
  if (bulletin == 4)
  {
    strcpy(fname, FWDPATH"/w_");
    strcat(fname, call);
    strcat(fname, ".bcm");
    strlwr(fname);
    if (file_isreg(fname))
    {
      if ((f = s_fopen(fname, "lrt")) != NULL) //fwdtrigger)
      {
         while (fgets(s, LINELEN+40, f))
         {
           if (*s != ' ') mails++;
         }
         s_fclose(f);
      }
      if (mails > 10 // at least 10 lines
          && isforwardtime(call, 1)) // only when it is time for bulletins
      {
        fork(P_BACK | P_MAIL, 0, fwdsend, call);
        wdelay(15012); // erst nach 15s naechste Box
        return OK;
      }
      else
        return NO;
    }
    else
      return NO;
  }
  if (isforwardtime(call, bulletin))
  {
    switch (bulletin)
    {
      case 0: strcpy(fname, FWDPATH"/u_"); break;
      case 1: strcpy(fname, FWDPATH"/i_"); break;
      case 2: strcpy(fname, FWDPATH"/m_"); break;
      case 3: goto dofwd;
    }
    strcat(fname, call);
    strcat(fname, ".bcm");
    strlwr(fname);
    if (file_isreg(fname))
    {
      dofwd:
      fork(P_BACK | P_MAIL, 0, fwdsend, call);
      wdelay(15012); // erst nach 15s naechste Box
      return OK;
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

void startfwd (char *) // fork() => char *
//*******************************************************************
//
//
//*******************************************************************
{
  lastfunc("startfwd");
  unsigned int i = 0;

//  if (! sema_lock("forward"))
//  {
//    trace(report, "startfwd", "sema was set");
//    return;
//  }
  while (fp[i].call[0])
  {
    trace(report, "startfwd", "checking %s", fp[i].call);
//    printf("%s\n", fp[i].call);
    testfwd(fp[i].call, 0); // Usermails?
    testfwd(fp[i].call, 4); // WPROT-Files?
    testfwd(fp[i].call, 1); // Bulletins?
    testfwd(fp[i].call, 2); // E/M-Files?
    testfwd(fp[i].call, 3); // Pollen?
    i++;
  }
//  sema_unlock("forward"); // dh8ymb 15.07.06
}

/*---------------------------------------------------------------------------*/
