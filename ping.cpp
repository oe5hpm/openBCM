/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------------------------
  Ping-Server
  ----------------------------------------------------------------


  Copyright (C) 2000     Patrick, DF3VI
                         Dietmar Zlabinger, OE3DZW
  (GPL)

 ***************************************************************/


#include "baycom.h"

/*---------------------------------------------------------------------------*/

void pingresp (char *name)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("pingresp");
  char line[BUFLEN];
  char impfile[30];
  FILE *f;
  FILE *mailf;
  strlwr(b->mailpath);
  if (! (mailf = s_fopen(b->mailpath, "sr"))) return;
  fgets(line, BUFLEN - 1, mailf);
  mbsend_parse(line, 0);  //parse header
  if (! mbcallok(b->herkunft)) //sender is not a valid call
  {
    s_fclose(mailf);
    return;
  }
  fgets(line, BUFLEN - 1, mailf); //skip forward-dummy-triggers
  fgets(line, BUFLEN - 1, mailf); //skip read-line
  sprintf(impfile, TEMPPATH "/%s", time2filename(0));
  if ((f = s_fopen(impfile, "swt")) != NULL)
  {
    fprintf(f, "SP %s ", b->herkunft);   //send-line
    fprintf(f, "\nRe: * Ping server *\n" //subject
                "PING for OpenBCM\n"      //copyright-line
                "Pinged at %-19.19sz\n\n",
                datestr (ad_time() - ad_timezone(), 12));
    fprintf(f, "PING> S%c %s @ %s\n", b->mailtype, b->ziel, b->at);
    while (! feof(mailf))               //copy of original mail
    {
      if (fgets(line, BUFLEN - 1, mailf))
        fprintf(f, "PING> %s", line);
    }
    s_fclose(mailf);
    markerased('W', 0, 0);
    fprintf(f, "\nEnd of ping - I'm alive.\n");
    fprintf(f, "\nnnnn\nimpdel\n");
    s_fclose(f);
    fork(P_BACK | P_MAIL, 0, mbimport, impfile); //import it as mail
  }
};

/*---------------------------------------------------------------------------*/

void send_ping (char *befbuf)
//****************************************************************************
//
//****************************************************************************
{
  lastfunc("send_ping");
  char sendcmd[LINELEN+1];

  if (! *befbuf)
  {
    putf("Syntax: PING <bbs>\n"); //short help
    return;
  }
  *b->at = 0;
  nexttoken(befbuf, b->at, HADRESSLEN);
  if (find_hadr(b->at)) //address is known bbs
  {
    if (weiterleiten(0, "PING") != unbekannt) //address is unknown
    {
      snprintf(sendcmd, sizeof(sendcmd),
                        "SP PING @%s <%s\n@ %s\n", b->at, u->call,
                                 u->mybbsok ? u->mybbs : m.boxname);
      mkboard("/", "PING", 0); //create board if not existing
      genmail(sendcmd, ""); //empty body
      putf(ms(m_linesfor), 2, b->at);
      putf(" ");
      putf(ms(m_stored)); //output msg
    }
    else putf(ms(m_adrunknown), b->at);
  }
  else putf(ms(m_adrunknown), b->at);
}

/*---------------------------------------------------------------------------*/
