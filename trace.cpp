/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------------------------------------
  Fehlerbehandlung. Ausgabe und Abspeichern von Systemmeldungen
  -------------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved


 ***************************************************************/

//19980120 OE3DZW everything is saved in syslog_r.bcm
//19980121 OE3DZW report output was in syslog too, fixed
//19980215 OE3DZW non report output was in syslog_r twice, fixed
//                define for syslog_r in mail.h
//19980323 OE3DZW log on Linux/Win32-screen only showed report-msges
//19990206 OE3DZW Lines may be max. 130 chars long (before limit was 80)
//19990206 OE3DZW Linebreaks will be LF under Linux - not CRLF
//20001113 DK2UI  new function log_entry for trace log files

#include "baycom.h"

/*---------------------------------------------------------------------------*/

static char tracebuffer[BUFLEN];
char lasttrace[132];
static int tracein = 0;
static int traceout = 0;

/*---------------------------------------------------------------------------*/

void log_entry (char *logname, char *msg)
//*************************************************************************
//
//
//*************************************************************************
{
  char fname[FNAMELEN+1];
  FILE *f;

  sprintf(fname, "%s/%s", TRACEPATH, logname);
  f = s_fopen(fname, "sab");
  if (! f) s_fopen(fname, "swb");
  if (f)
  {
    fprintf(f, "%s %s" NEWLINE, datestr(ad_time(), 11), msg);
    s_fclose(f);
  }
}

/*---------------------------------------------------------------------------*/

static void near tracechr (char ch)
//*************************************************************************
//
//
//*************************************************************************
{
  while (((tracein + 1) % BUFLEN) == traceout)
  {
    // db7mh: Auskommentiert, da es hier zu einer Endlosschleife kommt
    //if (gettaskid() == NOTASK)
      traceout = (traceout + 1) % BUFLEN;
    //else
    //  wdelay(49);
    if (runterfahren) return;
  }
  tracebuffer[tracein] = ch;
  tracein = (tracein + 1) % BUFLEN;
}

/*---------------------------------------------------------------------------*/

static void near tracestr (char *s)
//*************************************************************************
//
//
//*************************************************************************
{
  while (*s) tracechr(*(s++));
}

/*---------------------------------------------------------------------------*/

void tracelog (char *message, unsigned int mode)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[132];
  handle fh;
  char name[FNAMELEN+1];

  if (strlen(message) > 103)
    message[103] = 0;
  subst(message, CTRLZ, 0x9f); //oe3dzw: Hex-Wert
  sprintf(lbuf, "%s %6s: %s" NEWLINE, datestr(ad_time (), 11),
                (t && b && *b->logincall) ? b->logincall : "SYSTEM", message);
#ifdef __FLAT__
  //?? if(mode==_s_report)
  {
    for (unsigned i = 0; lbuf[i]; i++)
    {
      if ((lbuf[i] < ' ' || lbuf[i] > 0x7e) && lbuf[i] != LF && lbuf[i] != CR)
        printf(".");
      else
        printf("%c", lbuf[i]);
    }
  }
#endif
  /*
#ifdef _WIN32
  if (mode==_s_report)
  {
    void win_out(char *);
    win_out(lbuf);
  }
#endif
  */
  if (mode >= m.tracelevel)
  {
    if (m.tracelevel == _s_report) //OE3DZW
    {
      strcpy(name, TRACEPATH "/" SYSLOGRNAME);
      if ((fh = sopen(name, O_RDWR | O_BINARY, SHAREMODE)) == EOF)
        fh = _creat(name, CREATMODE);
      else
        lseek(fh, 0L, SEEK_END);    // Workaround for bug
      if (fh != EOF)
      {
        _write(fh, lbuf, strlen(lbuf));
        _close(fh);
      }
    }
    if (mode != _s_report)
    {
      strcpy(name, TRACEPATH "/" SYSLOGNAME);
      if ((fh = sopen(name, O_RDWR | O_BINARY, SHAREMODE)) == EOF)
        fh = _creat(name, CREATMODE);
      else
        lseek(fh, 0L, SEEK_END);
      if (fh != EOF)
      {
        _write(fh, lbuf, strlen(lbuf));
        _close(fh);
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void cmdlog (char *befehl)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[120];

  if (! strcmp(b->uplink, "Import")) return;
  subst(befehl, CTRLZ, 0x9f);
  if (strlen(befehl) > LINELEN)
    befehl[LINELEN] = 0;
  sprintf(lbuf, "%6s: %s", b->logincall, befehl);
  log_entry(CMDLOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

void pwlog (char *logincall, char *uplink, char *msg)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[120];

  sprintf(lbuf, "%s via %-9s %s", logincall, uplink, msg);
  log_entry(PWLOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
void nntplog (char *mode, char *string)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[301];

  if (strlen(string) >= 300) string[300] = 0;
  sprintf(lbuf, "%-6s %s %s", b->logincall, mode, string);
  rm_crlf(lbuf);
  log_entry(NNTPLOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

void httplog (char *mode, char *string)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[301];

  if (strlen(string) >= 300) string[300] = 0;
  sprintf(lbuf, "%-6s %s %s", b->logincall, mode, string);
  rm_crlf(lbuf);
  log_entry(HTTPLOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

void pop3log (char *mode, char *string)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[301];

  if (strlen(string) >= 300) string[300] = 0;
  sprintf(lbuf, "%-6s %s %s", b->logincall, mode, string);
  rm_crlf(lbuf);
  log_entry(POP3LOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

void smtplog (char *mode, char *string)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[301];

  if (strlen(string) >= 300) string[300] = 0;
  sprintf(lbuf, "%-6s %s %s", b->logincall, mode, string);
  rm_crlf(lbuf);
  log_entry(SMTPLOGNAME, lbuf);
}

/*---------------------------------------------------------------------------*/

void ftplog (char *mode, char *string)
//*************************************************************************
//
//
//*************************************************************************
{
  char lbuf[301];

  if (strlen(string) >= 300) string[300] = 0;
  sprintf(lbuf, "%-6s %s %s", b->logincall, mode, string);
  rm_crlf(lbuf);
  log_entry(FTPLOGNAME, lbuf);
}
#endif

/*---------------------------------------------------------------------------*/

void wprotlog (char *string, char *call)
//*************************************************************************
//
//  Speichert WPROT-Log
//
//*************************************************************************
{
  char logname[80];
  FILE *logf;

  strlwr(call);
  snprintf(logname, 79, WLOGPATH "/r_%s.bcm", call);
  logf = s_fopen(logname, "sat");
  if (logf)
  {
    fseek(logf, 0, SEEK_END);
    fprintf(logf, "%s", string);
    s_fclose(logf);
  }
  strupr(call);
}

/*---------------------------------------------------------------------------*/
#define TLEN 80
void trace (unsigned int mode, int line, char *file, char *programm, char *string, ...)
//*************************************************************************
//
//
//*************************************************************************
{
  va_list argpoint;
  char cbuf[256];
  static char global_sema = 0;
  char msm[] = " ";

  va_start(argpoint, string);
  vsprintf(cbuf, string, argpoint);
  va_end(argpoint);
  if (init_off) return;
  if (strlen(cbuf) > 149)
  {
    tracelog("trace overflow", _s_serious);
//    cbuf[TLEN - 1] = 0;
//    tracelog (cbuf, _s_serious);
//    exit (1);
  }
  cbuf[TLEN - 1] = 0;
  switch (mode)
  {
    case _s_report:  strcpy(msm, "R"); break;
    case _s_replog:  strcpy(msm, "L"); break;
    case _s_serious: strcpy(msm, "S"); break;
    case _s_fatal:   strcpy(msm, "F"); break;
    case _s_tr_abbruch: strcpy(msm, "A"); break;
  }
  sprintf(cbuf + TLEN, "#%s %s: %s", msm, programm, cbuf);
  tracelog(cbuf + TLEN, mode);
  strcpy(lasttrace, cbuf + TLEN);  // capture the last trace message
  if (runterfahren) return;
  while (global_sema) wdelay(21);
  global_sema = 1;
  cbuf[TLEN] = LF;
  tracestr(cbuf + TLEN);
  global_sema = 0;
  if (mode == _s_tr_abbruch)
  {
    cbuf[TLEN + 61] = 0;
    strcpy(abortreason, cbuf + TLEN + 3);
    exit(1);
  }
  else if (mode == _s_fatal)
  {
    if (b) trace(serious, "terminated", "%d:%s \"%0.30s\"",
                          gettaskid(), b->logincall, t->lastcmd);
    if (t) suicide();
  }
}

/*---------------------------------------------------------------------------*/

#ifdef __DOS16__
static void near winclock (void)
//*************************************************************************
//
//
//*************************************************************************
{
  static time_t last = 0;       // remember the last displayed time
  time_t current = ad_time();
  unsigned col;

  if (current > last)
  {
    last = current;            // one second later
    if (iskeytask())
      col = color.activ_frame;
    else
      col = color.back_frame;
    wf(e->xend - 8, e->y, col, datestr(current, 4));
  }
}
#endif

/*---------------------------------------------------------------------------*/

void tracewindow (char *name)
//*************************************************************************
//
//
//*************************************************************************
{
  int action = 0;
#ifdef __DOS16__
  e->x = 30;
  e->xend = bildspalten - 1;
  e->y = 0;
  e->yend = bildzeilen / 3;
  preparewin(putupdate, name, 30 * bildzeilen);
  putupdate(1);
#endif
  while (! init_off)
  {
    if (tracein != traceout)
    {
      putv(tracebuffer[traceout]);
      traceout = (traceout + 1) % BUFLEN;
      action = 2;
    }
    else
    {
      if (action)
      {
        action--;
        wdelay(22);
      }
      else
        wdelay(251);
      set_watchdog(1);
#ifdef __DOS16__
      winclock();
#endif
    }
  }
  for (unsigned i = TASKS; i; i--) killtask(i, 0);
#ifdef __DOS16__
  wclose();
  t_free(buf);
  buf = NULL;
#endif
}

/*---------------------------------------------------------------------------*/

