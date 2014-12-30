/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------
  RADIO-Interface
  ---------------

  simple net-cmd interface
  supported commands:  binary, ascii, connect ax25
  default port number: 8124

  Copyright (C)       Dietmar Zlabinger, oe3dzw

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/


#include "baycom.h"
#ifdef RADIOIF

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

static char *beftab[] =
{ "CONNECT", "ASCII", "BINARY", "IMAGE", "QUIT", NULL };

static enum befnum
{ unsinn, _connect, _ascii, _binary, _image, _quit } cmd = unsinn;

class radio
{
  public:
    void start_radio (char *name);
    radio ();
  private:
    int ascii;
    void link (char *mycall, char *path);
    void get_path (char *destination, int len);
    void puthelp (void);
    void input_clx (char *s, int maxlen);
};

/*---------------------------------------------------------------------------*/

radio::radio (void)
//*************************************************************************
//
//*************************************************************************
{
  ascii = 1;
}

/*---------------------------------------------------------------------------*/

void radio::input_clx (char *s, int maxlen)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 0;
  while (1)
  {
    s[i] = getv_sock();
#ifdef DEBUG //some debug output
    trace(report, "inputline_sock", "s[%d]=%02X=%c", i, s[i], s[i]);
#endif
    s[i + 1] = 0;
    if (s[i] == CR || s[i] == LF || i == (unsigned)(maxlen - 1)) break;
    if (strstr(s, "OE1XLR-5")) break; //this is a ugly hack for a bug in clx!
    i++;
  }
}

/*---------------------------------------------------------------------------*/

void radio::link (char *mycall, char *path)
//*************************************************************************
//
//*************************************************************************
{
  char lcmds[200];
  int outgoing = 0;
  int income = 0;
  int data = 0;

  if (! makeconnect(mycall, path)) return;
  snprintf(lcmds, sizeof(lcmds), "connected %s>%s", mycall, path);
  lastcmd(lcmds);
  trace(report, "radio", lcmds);
  while (t->port)
    {
      while (t->port && getvalid())
      {
        int a = getv();
        if (ascii && a == LF) putv_tnc(CR);
        else putv_tnc(a);
        outgoing = 1;
      }
      if (outgoing)
      {
        putflush_tnc();
    data = 1;
        outgoing = 0;
      }
      while (t->port && getvalid_tnc())
      {
        int a = getv_tnc();
        income = 1;
        if (ascii && a == CR) bputv(LF);
        else bputv(a);
      }
      if (income)
      {
        putflush();
        data = 1;
        income = 0;
      }
      if (! data) wdelay(153);
      else waitfor(e_ticsfull);
      data = 0;
    }
  snprintf(lcmds, sizeof(lcmds), "disconnected %s>%s", mycall, path);
  lastcmd(lcmds);
  trace(report, "radio", lcmds);
}

/*---------------------------------------------------------------------------*/

void radio::get_path (char *destination, int len)
//*************************************************************************
//
//*************************************************************************
{
  char *path = 0;
  char tocall[11];
  char line[101];

//format:  <tocall>:<path>

  FILE *f = s_fopen(L2PATHNAME, "srt"); //eg. netpath.bcm
  if (! f) return;
  while (! path && ! feof(f))
    if (fgets(line, sizeof(line) - 1, f))
    {
      char *comment = strchr(line, ';');
      if (comment) *comment = 0;
      char *lf = strchr(line, LF);
      if (lf) *lf = 0;
      if ((path = strchr(line, ':')) != NULL)
      {
        *path = 0;
        nexttoken(line, tocall, sizeof(tocall) - 1);
        if (! stricmp(tocall, destination))
        {
          strncpy(destination, path + 1, len - 1);
          destination[len - 1] = 0;
        }
        else path = 0;
      }
    }
  s_fclose(f);
  return;
}
/*---------------------------------------------------------------------------*/

void radio::start_radio (char *name)
//*************************************************************************
//
//*************************************************************************
{ //connect to radio given in first line (cmdline)
  //syntax: connect mycall tocall [digis]

  char buffer[200];
  strcpy(buffer, "");
  char *pbuffer = buffer;
  char destination[100];
  char source[11];
  char subcmd[21];
#ifdef DEBUG
  static char n[] = "radio:start";
#endif

  t->port = 0;
#ifdef DEBUG
  trace(report, n, "connect");
#endif
  while (! getvalid_sock()) //wait for input, no timeout
    wdelay(204);
  do
  {
    input_clx(buffer, sizeof(buffer));
    //getline(buffer, sizeof(buffer) - 1, 1);
#ifdef DEBUG
    trace(report, n, "rcvd -%s-", buffer);
#endif
    lastcmd(buffer);
    pbuffer = buffer;
    cmd = (befnum) readcmd(beftab, &pbuffer, 0);
    switch (cmd)
    {
      case _ascii: ascii = 1; break;
      case _binary: //fall through
      case _image: ascii = 0; break;
      case _quit:  //fall through
      case unsinn: puthelp(); return;
      default: break;
    };
  } while (cmd != _connect);
  pbuffer = nexttoken(pbuffer, subcmd, 20);
  if (! stristr(subcmd, "ax"))
  {
    puthelp();
    return;
  }
  pbuffer = nexttoken(pbuffer, destination, sizeof(destination) - 1);
  if (! *destination)
  {
    puthelp();
      return;
  }
  nexttoken(pbuffer, source, sizeof(source) - 1);
  if (! *source) strncpy(source, m.mycall[0], sizeof(source) - 1);
  get_path(destination, sizeof(destination));
  link(source, destination);
}

/*---------------------------------------------------------------------------*/
void radio::puthelp (void)
//*************************************************************************
//
//*************************************************************************
{
  putf("\n"LOGINSIGN" Radio Server\n"
       "Syntax: binary|ascii|connect ax25 <destination> [<source>]\n"
       "If no source callsign is given the primary boxcall will be used.\n");
}
/*---------------------------------------------------------------------------*/

void mblogin_radio (char *name)
//*************************************************************************
//
//*************************************************************************
{
  radio s;
  strcpy(b->name, "radio");
  strcpy(b->uplink, "RADIO");
  strcpy(b->peerip, name);
  *b->logincall = 0;
  s.start_radio(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/
#endif
