/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------
  Online-Help fuer Mailboxbenutzer
  --------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/
//19991128 DF3VI  added subsubject to HELP INDEX
//20010625 DF3VI  index length %1.37s
//20130630 OE5HPM safe usage of given ptr *cmd within mbreadhelp

#include "baycom.h"

/*---------------------------------------------------------------------------*/

class mbhelp_t
{
public:
  void dohelp (char *subject);
  ~mbhelp_t();
private:
  int helpindex (void);
  int helpopen (char *speech);
  void charstr (int number, char character);

  FILE *helpf;
  FILE *idxf;
  char helpname[40];
  char idxname[40];
  char inbuf[LINELEN+2];
};

/*---------------------------------------------------------------------------*/

mbhelp_t::~mbhelp_t ()
//*************************************************************************
//
//*************************************************************************
{
  if (helpf) s_fclose(helpf);
  if (idxf) s_fclose(idxf);
}

/*---------------------------------------------------------------------------*/

int mbhelp_t::helpindex (void)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 0;

  if (! helpf) return 0;
  idxf = s_fopen(idxname, "sw+t");
  if (! idxf) return 0;
  s_fsetopt(idxf, 1);
  fprintf(idxf, "%ld\n", file_isreg(helpname));
  while (fgets(inbuf, LINELEN, helpf))
  {
    if (*inbuf == '\\' && inbuf[1] == '\\')
    {
      inbuf[strlen(inbuf) - 1] = 0; // delete newline
      fprintf(idxf, "%1.37s %ld\n", inbuf + 2, ftell(helpf));
      i++;
    }
  }
  rewind(idxf);
  trace(replog, "helpindex", "%d subjects in %s", i, helpname);
  return 1;
}

/*---------------------------------------------------------------------------*/

int mbhelp_t::helpopen (char *speech)
//*************************************************************************
//
//*************************************************************************
{
  time_t helptime = 0L;

  sprintf(helpname, MSGPATH "/help.%s", speech);
  strlwr(helpname);
  if ((helpf = s_fopen(helpname, "srt")) != NULL)
  {
    sprintf(idxname, MSGPATH "/helpidx.%s", speech);
    strlwr(idxname);
    idxf = s_fopen(idxname, "srb");
    {
      if (idxf)
      {
        fscanf(idxf, "%ld", &helptime);
        if (file_isreg(helpname) != helptime)
        {
          s_fclose(idxf);
          idxf = NULL;
        }
      }
      if (! idxf)
      {
        if (! helpindex()) // generate index file
        {
          s_fclose(helpf); // do not loose filehandles if no correct index
          helpf = NULL;
          return 0;
        }
      }
      fgets(inbuf, LINELEN, idxf); // skip the first line containing filetime
      return 1;
    }
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

void mbhelp_t::charstr (int number, char character)
//*************************************************************************
//
//*************************************************************************
{
  int i;
  for (i = 0; i < number; i++)
    putv(character);
  putv(LF);
}

/*---------------------------------------------------------------------------*/

void mbhelp_t::dohelp (char *subject)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 0;
  long seekpos = 0;
  int helpall = ! stricmp(subject, "ALL");
  int found = 0;
  char idx[20];
  char *subsubject = strchr(subject, ' ');
  char *mlink = NULL;
  char *sublink = NULL;
  char linkstr[40];
  char sublinkstr[40];
  int linkcnt = 0;
  char oldsubject[60];

  strcpy(linkstr, "");
  strcpy(sublinkstr, "");
  strcpy(oldsubject, "");
  strcpy(idx, "INDEX");
  helpf = NULL;
  idxf = NULL;
  if (! subsubject) subsubject = strchr(subject, '.');
  if (subsubject) *subsubject++ = 0;
  if (! *subject) subject = "HELP";
  if (! helpopen(ms(m_helpext)))
  {
    if (! helpopen("GB"))
    {
      if (! helpopen("DL"))
      {
        putf(ms(m_nohelp));
        trace(serious, "dohelp", ms(m_nohelp));
        return;
      }
    }
  }
  if (strstr(idx, subject) == idx)
  {
    putf(ms(m_helpkeywords));
    while (fscanf(idxf, "%s %ld", inbuf, &seekpos) == 2)
    {
      subst1(inbuf, '=', 0);
      if (! subsubject || strstr(inbuf, subsubject) == inbuf)
      {
#ifdef __FLAT__
        if (! strcmp(b->uplink, "HTTP"))
          html_putf("<a href=\"/cmd?cmd=h+%s\">%-19s</a>", inbuf, inbuf);
        else
#endif
          putf("%-19s", inbuf);
        i++;
        if (! (i % 4)) putv(LF);
        else putv(' ');
      }
    }
    putv(LF);
  }
  else
  {
    while (fscanf(idxf, "%s %ld\n", inbuf, &seekpos) == 2)
    {
      if (strstr(inbuf, subject) == inbuf || helpall)
      {
        if (subsubject && ! helpall)
        {
          char *dot = strchr(inbuf, '.');
          if (dot)
          {
            dot++;
            if (strstr(dot, subsubject) != dot) continue;
          }
          else continue;
        }
        mlink = strchr(inbuf, '=');
        if (mlink)
        {
          if (helpall) continue;
          *mlink = 0;
          mlink++;
          strcpy(linkstr, mlink);
          sublink = strchr(linkstr, '.');
          sprintf(oldsubject, "%s -> ", inbuf);
          if (sublink)
          {
            *sublink = 0;
            strcpy(sublinkstr, sublink + 1);
            subsubject = sublinkstr;
          }
          else
            subsubject = NULL;
          subject = linkstr;
          rewind(idxf);
          fgets(inbuf, LINELEN, idxf); // skip first line containing filetime
          if (++linkcnt > 10)
          {
            trace(serious, "help", "cyclic link %s.%s", subject, subsubject);
            break;
          }
          continue;
        }
        found++;
        subst1(inbuf, '.', ' ');
        subst1(oldsubject, '.', ' ');
        putv(LF);
        charstr(strlen(inbuf) + 6 + strlen(oldsubject), ':');
        putf(":: %s%s ::", oldsubject, inbuf);
        putv(LF);
        charstr(strlen(inbuf) + 6 + strlen(oldsubject), ':');
        fseek(helpf, seekpos, SEEK_SET);
        while (fgets(inbuf, LINELEN, helpf))
        {
          if (*inbuf == '\\' && inbuf[1] == '\\') break;
          putf("%s", inbuf);
          if (b->abbruch) break;
        }
        if (! helpall) break;
      }
      if (b->abbruch) break;
    }
    //testabbruch();
    if (! found)
      putf(ms(m_helpnotfound), subject, subsubject ? subsubject : "*");
  }
}

/*---------------------------------------------------------------------------*/

void mbreadhelp (char *cmd)
//*************************************************************************
//
//*************************************************************************
{
  mbhelp_t help;
  char helpCmd[LINELEN];

  strncpy(helpCmd, cmd, sizeof(helpCmd));
  helpCmd[sizeof(helpCmd)-1] = 0;
  strupr(helpCmd);
  help.dohelp(helpCmd);
}

/*---------------------------------------------------------------------------*/

