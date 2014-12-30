/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------------------
  Schnittstelle Mailbox / Benutzeroberflaeche
  -------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980226 hrx    new: taskwindow - to be called with twin (sysopcommand).
//                like the standard uwin, outfit is like ps -ru.
//19980408 hrx    taskwin has been changed, replaced old with new.
//                added trcwin()
//19980526 OE3DZW Flat-batch only when file exists
//19980608 OE3DZW added support for user-fwd at getlogin - not in _TELEPHONE
//19981001 OE3DZW removed filename at genmail
//19990206 OE3DZW genmail will only generate mails if boxheader is set
//19990621 OE3DZW import blocked during sendlock (fixed)
//20030706 DB1RAS command.com double call removed
//20030706 DB1RAS added _PREPOSTEXEC (dospreexec, dospostexec)

#include "baycom.h"

#ifdef __DOS16__
static int einrueck = 1;

/*---------------------------------------------------------------------------*/

void userwin (char *name)
//*************************************************************************
//
//*************************************************************************
{
  int keysave = keywindow();
  int width;
//  e->x=bildspalten-19;
//  e->xend=bildspalten-1;
//  e->y=0;
//  e->yend=8;
  e->x=0;
  e->xend=29;
  e->y=2;
  e->yend=bildzeilen/3;
  preparewin(putupdate, name, 400);
  newkeywin=keysave;
  while (! runterfahren)
  {
    wdelay(534);
    e->index = e->modified=e->curline=0;
    width = (e->xlen-1)/9;
    if (! width) width=1;
    listusers(2+width);
    e->shouldupdate++;
  }
  wclose();
}

/*---------------------------------------------------------------------------*/

void taskwin (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  int keysave = keywindow();

  e->x=0;
  e->xend=79;
  e->y=3;
  e->yend=bildzeilen/3;

  scanoptions(befbuf);
  strcpy(t->name, "Tasks");

  preparewin(putupdate, "Tasks", 2000);
  newkeywin=keysave;
  while (! runterfahren)
  {
    wdelay(492);
    e->index=e->modified=e->curline=0;
    putv(LF);
    listprocess(b->optplus);
    e->shouldupdate++;
  }
  wclose();
}

/*---------------------------------------------------------------------------*/

#ifdef _AUTOTRCWIN
void trcwin (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  int keysave = keywindow();
  char *name = 0;
  e->x=0;
  e->xend=79;
  e->y=9;
  e->yend=35;
  scanoptions(befbuf);
  b->continous=1;
  sprintf(name, "Trace: Task %s", befbuf);
  strcpy(t->name, "Trace");
  preparewin(putupdate, name, 2000);
  newkeywin=keysave;
  e->index=e->modified=e->curline=0;
  putv(LF);
  monitor(atoi(befbuf), b->optplus);
  e->shouldupdate++;
  wdelay(4234);
  wclose();
}
#endif

/*---------------------------------------------------------------------------*/

void mbwin2 (char *name)
//*************************************************************************
//
//*************************************************************************
{
  char call[10];
  char mycall[10];

  name = nexttoken(name, call, 9);
  nexttoken(name, mycall, 9);
#ifdef _LCF // JJ
  if (! fwdcallok(call, m.logincallformat))
    strcpy(call, m.sysopcall);
#else
  if (! mbcallok(call))
    strcpy(call, m.sysopcall);
#endif
  e->x=einrueck;
  e->xend=bildspalten-1;
  e->y=bildzeilen/3+einrueck;
  e->yend=bildzeilen-1;
  einrueck++;
  preparewin(putupdate, call, 3000);
  putv(LF);
  mbmain(call, "Console", mycall);
  wclose();
  t_free(buf);
  buf=0;
  einrueck--;
}

/*---------------------------------------------------------------------------*/

void doodle (int soundnum)
//*************************************************************************
//
//*************************************************************************
{
  int i;

  if (! m.sysopbell)
    return;

  switch (soundnum)
  {
    case 0:
    {
      for (i=0 ; i<3 ; i++)
      {
        delay(2);
        nosound();
        sound(800);
        delay(20);
        sound(1100);
        delay(15);
        sound(1700);
        delay(10);
        nosound();
      }
    } break;
    case 1:
    {
      sound(800);
      delay(50);
      nosound();
      delay(50);
      sound(1100);
      delay(50);
      nosound();
      delay(50);
      sound(1700);
      delay(50);
      nosound();
      delay(100);
    } break;
    case 2:
    {
      sound(2000);
      delay(50);
      nosound();
      delay(50);
      sound(2000);
      delay(50);
      nosound();
      delay(100);
    } break;
    case 3:
    {
      sound(2000);
      delay(50);
      nosound();
      delay(50);
      sound(1100);
      delay(50);
      nosound();
      delay(100);
    } break;
  }
}

#endif // __DOS16__
/*---------------------------------------------------------------------------*/

#ifdef _TELEPHONE
//warning *-option (fwd) not supported here!

int getlogin (char *s, int withpw, char *source)
//*************************************************************************
//
//*************************************************************************
{
  lastcmd("wait for login");
  char pwstr[21];
  char userpw[10];
  unsigned int count = 0;
  int tty = 0;

  tty = (! strcmp(source, "TTY"));
  if (tty)
  {
    ttysetecho(1);
    putv(12);
  }
  if (m.callformat)
    putf("\n\nOpenBCM-Mailbox V"VNUMMER" ("OPSYSTEM") "CBMODEID"\n");
  else
    putf("\n\nOpenBCM-Mailbox V"VNUMMER" ("OPSYSTEM")\n");
  if (tty) readtext(ttylogintext);
  if (t->name == "Sysop") readtext(conlogintext);
  while (count < 3)
  {
    ttysetecho(1);
    putf("\nlogin: ");
    *s = 0;
    getline(s, 20, 1);
    strupr(s);
    if (! strcmp(s, "NO CARRIER")) return 0;
    if (! strcmp(s, ".")) strcpy(s, m.sysopcall);
#ifdef _GUEST
    if (! strcmp(s, "gast")) strcpy(s, m.guestcall);
#endif
#ifdef _LCF // JJ
    if (fwdcallok(s, m.logincallformat))
#else
    if (mbcallok(s))
#endif
    {
      if (withpw
#ifdef _GUEST
          && strcmp(s, m.guestcall)
#endif
          )
      {
        get_ttypw(s, userpw);
        if (! *userpw && tty)
        {
          putf("\nSorry, tty-password is not set.\n");
          trace(serious, "TTY", "ttypw not set");
        }
        else
        {
          putf("\npassword: ");
          ttysetecho(2);
          getline(pwstr, 20, 1);
          if (! strcmp(pwstr, "NO CARRIER")) return NO;
          ttysetecho(1);
          if (*userpw && ! strcmp(pwstr, userpw)) break;
          else
          {
            pwlog(s, source, "tty login failure.");
            trace(serious, "TTY", "pw incorrect");
          }
        }
      }
      else break;
    }
    if (*s)
    {
      count++;
      putf("\nLogin incorrect");
      if (tty) putf(" - %i tries left.\n", 3 - count);
    }
    if (count >= 3)
    {
      putf("\nGood bye.\n");
      trace(serious, "TTY", "tty login failure");
      wdelay(534);
      return NO;
    }
  }
  putv(LF);
  return OK;
}

#else

int getlogin (char *s, int withpw, char *source)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("getlogin");
  char pwstr[10];
  char userpw[10];
  char inputline[10];
  int tty_fwd = 0;
  unsigned int count = 0;
  int c;
  FILE *f;

  if ((f = s_fopen(ISSUENAME, "srt")) != NULL)
  {
    while ((c = fgetc(f)) != EOF) putf("%c", (char) c);
    s_fclose(f);
  }
  if (m.callformat)
    putf("\n\nOpenBCM-Mailbox V"VNUMMER" ("OPSYSTEM") "CBMODEID"\n");
  else
    putf("\n\nOpenBCM-Mailbox V"VNUMMER" ("OPSYSTEM")\n");
  while (count < 3)
  {
    putf("\n%s login: ", m.boxname);
    inputline[0] = 0;
    inputline[1] = 0;
    getline(inputline, 8, 1);
    strupr(inputline);
    tty_fwd = (*inputline == '*');
    strcpy(s, inputline + tty_fwd);
    if (*s == '.' && ! withpw)
    {
      strcpy(s, m.sysopcall);
      break;
    }
    if (withpw)
    {
      putf("password: ");
#ifdef FEATURE_SERIAL
      ttysetecho(0);
#endif
      getline(pwstr, 9, 1);
#ifdef FEATURE_SERIAL
      ttysetecho(1);
#endif
    }
#ifdef _LCF // JJ
    if (fwdcallok(s, m.logincallformat))
#else
    if (mbcallok(s))
#endif
    {
      if (withpw)
      {
        get_ttypw(s, userpw);
        if (*userpw && ! strcmp(pwstr, userpw)) break;
        else pwlog(s, source, "tty login failure");
      }
      else break;
    }
    if (*s)
    {
      putf("\nLogin incorrect.\n");
      wdelay(1000);
    }
    if (++count >= 3)
    {
      putf("\nSorry.\n\n");
      wdelay(534);
      return NO;
    }
  }
  putv(LF);
  return 1 + tty_fwd;
}
#endif

/*---------------------------------------------------------------------------*/

void mbwindow (char *name)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("mbwindows");
#ifdef __DOS16__
  e->x = 0;
  e->xend = bildspalten - 1;
  e->y = bildzeilen / 3;
  e->yend = bildzeilen - 1;
  preparewin(putupdate, name, 60 * bildzeilen);
  putv(LF);
#endif
  mbmainbegin();
#ifdef FEATURE_SERIAL
  init_tty();
#endif
#ifdef __DOS16__
  char s[10];
  while (! runterfahren)
  {
    getlogin(s, 0, "Console");
    mbmain(s, "Console", "");
    b->quit = 0;
  }
  wclose();
#endif
}

/*---------------------------------------------------------------------------*/

void boxusertask (char *upt)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("boxusertask");
  char call[CALLEN+1];
  char mycall[CALLSSID+1];
  char uplink[CALLSSID+1];
  unsigned int i = 0;

  getclear_tnc();
  while (isalnum(*upt) && i < CALLEN) call[i++] = *upt++;
  call[i] = 0;
  while (*upt && *upt != ' ') upt++; // skip SSID
  if (*upt) upt++;
  i = 0;
  while (*upt > ' ' && i < CALLSSID) uplink[i++] = *upt++;
  uplink[i] = 0;
  if (*upt) upt++;
  i = 0;
  while (*upt > ' ' && i < CALLSSID) t->downlink[i++] = *upt++;
  t->downlink[i] = 0;
  doodle(1);
  strcpy(t->name, call);
  getmycall_tnc(mycall);
  mbmain(call, uplink, mycall); // *** entry to BCM ***
  if (b->quit == 2) //when "quit" do not wait
  { //trace(report, "boxusertask", "fast logout");
    mbdisconnect(0);
  }
  else
  { //trace(report, "boxusertask", "slow logout");
    mbdisconnect(1); //wait for data to be sent
  }
  getclear_tnc();
  doodle(2);
}

/*---------------------------------------------------------------------------*/

void mbimport (char *filename)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("mbimport");

  t->output = io_dummy;
  b->oldinput = io_dummy;
  if ((b->inputfile = s_fopen(filename, "lrt")) != NULL)
  {
    t->input = io_file;
    while (sema_test("sendlock"))
    {
      lastcmd("wait sendlock");
      wdelay(30130);
    }
    mbmain(m.boxname, "Import", "");
  }
}

/*---------------------------------------------------------------------------*/

void genmail (char *sendline, char *content)
//*************************************************************************
//
//*************************************************************************
{
  char name[20];
  strcpy(name, "genmail");
  lastfunc(name);
  char tmpname[FNAMELEN+1];
  FILE *f;

  //do not generate automatic mails if Boxheader is
  //standard header. Test-BCM's should be silent ;-)
  if (! strcmp(m.boxheader, STD_BOXHEADER))
  {
    trace(report, name, "ignored - boxheader %s", m.boxheader);
    return;
  }
  sprintf(tmpname, TEMPPATH "/%s", time2filename(0));
  strcat(tmpname, ".imp");
  strlwr(tmpname);
  if ((f = s_fopen(tmpname, "swt")) != NULL)
  {
    fprintf(f, "%s\n", sendline);
    fprintf(f, "%s\nnnnn\nimpdel\n", content);
    s_fclose(f);
    fork(P_BACK | P_MAIL, 0, mbimport, tmpname);
  }
  else
    trace(serious, name, "fopen %s errno=%d %s",
                         tmpname, errno, strerror(errno));
}

/*---------------------------------------------------------------------------*/

void runbatch (char *batch)
//*************************************************************************
//
//*************************************************************************
{
  char cpy[FNAMELEN+1];
  subst1(batch, '.', 0);
  snprintf(cpy, sizeof(cpy), "%s.bat", batch);
  if (file_isreg(cpy))
#ifdef __MSDOS__
  {
    trace(report, "runbatch", cpy);
    set_watchdog(2);
#ifdef _PREPOSTEXEC //db1ras
    if (strcmp(m.dospreexec, "off"))
      system(m.dospreexec);
#endif
    sprintf(cpy, "%s.err", batch);
    freopen(cpy, "wt", stderr);
#ifdef OLD_SHELL
    sprintf(cpy, "command /c %s > %s.out", batch, batch);
#else
    //ueberfluessiger Doppel-Aufruf von command.com entfernt,
    //da system() sowieso den Kommando-Interpreter aufruft (db1ras)
    sprintf(cpy, "%s > %s.out", batch, batch);
#endif
    system(cpy);
    freopen("con", "wt", stderr);
#ifdef _PREPOSTEXEC //db1ras
    if (strcmp(m.dospostexec, "off"))
      system(m.dospostexec);
#endif
    set_watchdog(1);
  }
#else
  oshell(cpy, sh_noinput);
#endif
  snprintf(cpy, sizeof(cpy), "%s.imp", batch);
  if (file_isreg(cpy))
    fork(P_BACK | P_MAIL, 0, mbimport, cpy);
}

/*---------------------------------------------------------------------------*/
