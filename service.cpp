/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  Service-Interface
  -----------------


  Copyright (C)       Deti Fliegl, deti@lrz.de, DG9MHZ
                      Guardinistr. 47
                      81375 Muenchen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980211 OE3DZW added option for SERV

#include "baycom.h"

#ifdef SERVIF

#define MAX_RCPTCNT 100

#ifdef __UNIX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#ifdef _WIN32
#include "winsock.h"
#endif

//extern char lasttrace[90];

/*---------------------------------------------------------------------------*/
class serv
{
  public:
  void start_serv (char *name);
  private:
  int badcmd;
  int finished;
  int monitor;
  int trace;
  int a;
  FILE *f;
  unsigned long pos;
  char *args;

  enum
  {
    _SIGTRACE, _SIGMONITOR, _SIGCMD, _SIGSTAT
  }
  _sig;
  void putsig (int type);
};

/*---------------------------------------------------------------------------*/

void serv::putsig (int type)
//*************************************************************************
//
//*************************************************************************
{
   switch (type)
   {
     case _SIGTRACE:   putf("\n%c%c\n", 0xff, 0xff); break;
     case _SIGMONITOR: putf("\n%c%c\n", 0xff, 0xfe); break;
     case _SIGCMD:     putf("\n%c%c\n", 0xff, 0xfd); break;
     case _SIGSTAT:
     default:          putf("\n%c%c\n", 0xff, 0xfc);
   }
}

/*---------------------------------------------------------------------------*/

void serv::start_serv (char *name)
//*************************************************************************
//
//*************************************************************************
{
  static char *beftab[] =
  { "TRACE", "MONITOR", "CMD", "QUIT", NULL
  };
  enum befnum
  {
    _ERR, _TRACE, _MONITOR, _CMD, _QUIT
  };

  char c = 0, d = LF;
  t->port = 0;
  badcmd = 0;
  finished = 0;
  monitor = 0;
  trace = 0;
  b->sysop = 1;
  //char tracebuf[90];

  if ((f = s_fopen(TRACEPATH "/" SYSLOGRNAME, "lrt")) != NULL)
  {
    fseek(f, 0, SEEK_END);
    pos = ftell(f);
    if (pos > 1000)
    {
      fseek(f, -1000, SEEK_END);
      while ((a = fgetc(f)) != LF && a != EOF);
    }
  }
  mblogin(m.boxname, login_silent, "SERV");
  putsig(_SIGSTAT);
  putf("\n" LOGINSIGN " Service Interface\n");
  while (! finished)
  {
    if (getvalid_sock())
    {
      getline(b->line, BUFLEN - 1, 1);
      args = b->line;
      switch ((befnum) readcmd(beftab, &args, 0))
      {
        case _TRACE:
          if (! stricmp(args, "on")) trace = 1;
          else trace = 0;
          putsig(_SIGSTAT);
          putf("TRACE %s\n", trace ? "ON" : "OFF");
          break;
        case _MONITOR:
          if (! stricmp(args, "on")) monitor = 1;
          else monitor = 0;
          putsig(_SIGSTAT);
          putf("MONITOR %s\n", monitor ? "ON" : "OFF");
          break;
        case _CMD:
          putsig(_SIGCMD);
          mailbef(args, 0);
          break;
        case _QUIT:
          finished = 1;
          putsig(_SIGSTAT);
          putf("Goodbye\n");
          break;
        case _ERR: break;
      }
    }
    if (trace && f)
    {
      if ((a = fgetc(f)) != EOF)
      {
        putsig(_SIGTRACE);
        do
          putv(a);
        while ((a = fgetc(f)) != EOF);
      }
    }
    if (monitor && getvalid_tnc())
    {
      putsig(_SIGMONITOR);
      while (getvalid_tnc())
      {
        c = (char) getv_tnc();
        if (c == 7) c = '#';
        if (c >= 0xfd)
        {
          if (c == 0xff) c = LF;
          else c = 0;
        }
        if (c == CR) c = LF;
        if (c)
        {
          putv(d);
          d = c;
        }
      }
    }
    wdelay(76);
  }
  if (f) s_fclose(f);
}

/*---------------------------------------------------------------------------*/

void mblogin_serv (char *name)
//*************************************************************************
//
//*************************************************************************
{
  serv s;
  strcpy(b->uplink, "SERV");
  strcpy(b->peerip, name);
  *b->logincall = 0;
  s.start_serv(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/

#endif
