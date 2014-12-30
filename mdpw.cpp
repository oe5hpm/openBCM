/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------
  MD2/MD5 authentication procedures
  ---------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

//19980330 DH3MB  fix to fwd-pw
//19980420 OE3DZW sysop-md-pw will not give details to user

#include "baycom.h"

#ifdef FEATURE_MDPW

/*---------------------------------------------------------------------------*/

void calcMD2pw (char *MDstring, char *pw, char *MD2result)
//****************************************************************************
//
//****************************************************************************
{
  MD2 md2;
  md2.update(MDstring);
  md2.update(pw);
  md2.gethexdigest(MD2result);
}

/*---------------------------------------------------------------------------*/

void calcMD5pw (char *MDstring, char *pw, char *MD5result)
//****************************************************************************
//
//****************************************************************************
{
  MD5 md5;
  md5.update(MDstring);
  md5.update(pw);
  md5.gethexdigest(MD5result);
}

/*---------------------------------------------------------------------------*/

void genMDstring (char MDtype, char *pw, char *pwstring, char *result)
//****************************************************************************
//
//****************************************************************************
{
  char c;
  *pwstring = '[';
  for (c = 1; c < 11; c++) //only lower case characters are generated here
    pwstring[c] = random_max(10) + 48;
  pwstring[11] = 0;
  pwstring[12] = 0;
  switch (MDtype)
  {
    case 2: calcMD2pw(pwstring + 1, pw, result); break;
    case 5: calcMD5pw(pwstring + 1, pw, result); break;
    default:
      trace(serious, "genMDstring", "MD%d not implemented", MDtype);
      *result = 0;
  }
  pwstring[11] = ']';
}

/*---------------------------------------------------------------------------*/

char getMDpw (char MDtype, char *pw)
//****************************************************************************
//
//****************************************************************************
{
  char result[33];
  char pwstring[13];

  genMDstring(MDtype, pw, pwstring, result);
  putf("%s> %s\n", m.boxname, pwstring);
  getline(b->line, 32, 1);
  if (! stricmp(b->line, result)) return OK;
  else return NO;
}

/*---------------------------------------------------------------------------*/

void getMDsysoppw (char MDtype)
//****************************************************************************
//
//****************************************************************************
{
  FILE *f;
  *b->line = 0;
  if ((f = s_fopen(PASSWDNAME, "srt")) != 0)
  {
    fgets(b->line, BUFLEN - 1, f); // skip BayCom-Password
    if (! fgets(b->line, BUFLEN - 1, f)) *b->line = 0; // MD2-Password
    if (MDtype == 5)
      if (! fgets(b->line, BUFLEN - 1, f)) *b->line = 0; // MD5-Password
    rm_crlf(b->line);
    s_fclose(f);
  }
  int pwfound = !! (*b->line);
  if (! pwfound) strcpy(b->line, datestr(ad_time(), 11)); //dummy pw
  if (getMDpw(MDtype, b->line) && pwfound)
  {
    b->sysop = 1;
    cmdlog("*** MD2/5 ok");
    pwlog(b->logincall, b->uplink, "sysop ok");
    testdfull();
    return;
  }
  cmdlog("*** MD2/5 FAILURE ***");
  pwlog(b->logincall, b->uplink, "sysop failure");
  return;
}

/*---------------------------------------------------------------------------*/
#endif
