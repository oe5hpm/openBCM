/***************************************************************

  BayCom(R) Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------
  Mini-Terminalfunktionen fuer Mailbox
  ------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

//19990206 OE3DZW fixed automatic pw-reply

#define REMLEN 80

/*---------------------------------------------------------------------------*/

class TTermcmd
{
public:
  void cmd (char *befbuf);
  int  loccmd (char *befbuf);
  void remcmd (void);
  void remchr (int c);
  void putpw (char *befbuf);
  TTermcmd (void);
  char partner[20];
private:
  int  pwcmd;
  int  conncmd;
  int  fileruns;
  int  remote;
  char rembuf[REMLEN+1];
  int  remindex;
};

/*---------------------------------------------------------------------------*/

TTermcmd::TTermcmd (void)
//*************************************************************************
//
//*************************************************************************
{
  remindex = 0;
  pwcmd = 0;
  conncmd = 0;
  remote = 0;
}

/*---------------------------------------------------------------------------*/

int get_password (char *call, char *buffer, unsigned maxlen)
//*************************************************************************
//
//  search in password file for appropriate box or node call and return
//  the corresponding password string entry
//
//*************************************************************************
{
  int found = 0;
  FILE *f = s_fopen(PASSWDNAME, "srt");
  if (f)
  {
    while (fgets(buffer, maxlen - 1, f))
    {
      unsigned i = strlen(buffer) - 1;
      while (i && ! isalnum(buffer[i])) i--;  // remove line feed etc
      buffer[i + 1] = 0;
      if (stristr(call, buffer) == call)     // call matches
      {
        found = 1;
        break;
      }
    }
    if (! found) rewind(f);          // no hit -> return default
    fgets(buffer, maxlen - 1, f);          // password string
    s_fclose(f);
    if (*buffer) //only return 1 when a pw was found
    {
      while (strlen(buffer) < maxlen) strcat(buffer, "0");
      return OK;
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

void TTermcmd::putpw (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{ //known bugs: when using the ax25-kernel interface the
  //  password is not generated correctly because the password
  //  is not flushed.
  //  does not work with "standard" user-pw - only with pwline set.
  //  when the password is shorter than 80 characters it is filled up
  //  with "0" characters.
  //  rmnc no longer uses the "rmnc-pw"
  //but: this function did not work at all for a long time - no one
  //complained - therefore the current "solution" should be good enought

  int rmnc = strpos(befbuf, ")");
  unsigned int i = 0;
  int zahl = 0;
  int n[5];
  char reference[90];

  if (! get_password(partner, reference, 85)) return;
  reference[80] = 0;
  io_t old_out = t->output;
  t->output = io_tnc;   // all outputs go only to tnc or to dummy
                        // if no tnc is present
  if (rmnc > 1 && rmnc < 5 && strlen(befbuf) < 12
      && befbuf[rmnc + 1] == ' ')
  {
    for (i = 0; i < 5; i++)
    {
      if (befbuf[i + rmnc + 2])
        zahl += (befbuf[i + rmnc + 2] - '0') * (reference[i] - '0');
    }
    putf("%d\n", zahl);
    putflush();
    pwcmd = 0;
  }
  else
  {
    //randomize();
    unsigned rndpos = random_max(6) + 1;
    i = strlen(befbuf) - 5;
    while (i && (isdigit(befbuf[i]) || befbuf[i] == ' ')) i--;
    if (i > 4 && i < 60 && (befbuf[i] == '>'))
    {
      if (sscanf(befbuf + i + 1, "%d%d%d%d%d", n, n + 1,
                                               n + 2, n + 3, n + 4) == 5)
      {
        trace(report, "pw", befbuf);
        for (unsigned block = 1; block <= 6; block++)
        {
          for (i = 0; i < 5; i++)
            putf("%c", reference[(block == rndpos) ? (n[i] - 1) : random_max(strlen(reference))]);
        }
        putv(LF);
        putflush();
        pwcmd = 0;
      }
    }
  }
  t->output = old_out;
}

/*---------------------------------------------------------------------------*/

void TTermcmd::cmd (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  if (*befbuf == 'd')
  {
    mbdisconnect(1);
    t->port = 0;
  }
}

/*---------------------------------------------------------------------------*/

int TTermcmd::loccmd (char *befbuf)
//*************************************************************************
//
//*************************************************************************
{
  if (   pwcmd == FALSE
      && (! strcmp(befbuf, "pw") || ! strcmp(befbuf, "sy")
          || ! strcmp(befbuf, "//j")))
    pwcmd = TRUE;
  else pwcmd = FALSE;
  if (stristr(befbuf, "C ") == befbuf || ! stricmp(befbuf, "M"))
    conncmd = TRUE;
  else
    conncmd = FALSE;
  if (*befbuf == ':')
  {
    cmd(befbuf + 1);
    return OK;
  }
  else return NO;
}

/*---------------------------------------------------------------------------*/

void TTermcmd::remcmd (void)
//*************************************************************************
//
//*************************************************************************
{
  if (pwcmd) putpw(rembuf);
}

/*---------------------------------------------------------------------------*/

void TTermcmd::remchr (int c)
//*************************************************************************
//
//*************************************************************************
{
  if (c == LF)
  {
    rembuf[remindex] = 0;
    remindex = 0;
    remcmd();
  }
  else
  {
    if (remindex < REMLEN)
      rembuf[remindex++] = c;
  }
}

/*---------------------------------------------------------------------------*/

void termqso (char *calls)
//*************************************************************************
//
//*************************************************************************
{
  TTermcmd term;
  char befbuf[REMLEN+1];
  int income = 0;
  strupr(calls);
  // remove interface-name at beginning
  if (strchr(calls, ':') && strchr(calls, ' '))
    strncpy(term.partner, strchr(calls, ' ') + 1, 19);
  else strncpy(term.partner, calls, 19);
  term.partner[19] = 0;
  subst(term.partner, ' ', 0); // cut off trailing via digis
  subst(term.partner, '-', 0); // cut off ssid
  putf("Downlink to %s setup...\n\n", term.partner);
  if (makeconnect(b->logincall, calls))
  {
    putf(">>> Connected to %s\n", term.partner);
    while (t->port)
    {
      while (getvalid())
      {
        unsigned i;
        getline(befbuf, 79, 1);
        if (! term.loccmd(befbuf))
        {
          for (i = 0; befbuf[i]; i++)
            putv_tnc(befbuf[i]);
          putv_tnc(CR);
          putflush_tnc();
        }
      }
      while (t->port && getvalid_tnc())
      {
        int a = getv_tnc();
        income = 1;
        if (a == CR)
        {
          putv(LF);
          term.remchr(LF);
        }
        else
        {
          putv(a);
          term.remchr(a);
        }
      }
      if (income)
      {
        putflush();
        income = 0;
      }
      wdelay(53);
    }
    putf(">>> Disconnected from %s\n", term.partner);
  }
  else putf(">>> Failure with %s\n", term.partner);
}
