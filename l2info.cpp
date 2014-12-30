/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------------
  AX.25-Funktionen zur Infoverarbeitung
  -------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

char *doszeit (unsigned minuten)
//*************************************************************************
//
//*************************************************************************
{
  static char zeitstr[10];
  sprintf(zeitstr, " %02u:%02u ", minuten / 60, minuten % 60);
  return zeitstr;
}

/*---------------------------------------------------------------------------*/

void iot (byte zeichen)
//*************************************************************************
//
//  Ein Zeichen in den Empfangspuffer des aktuellen Ports ausgeben
//
//*************************************************************************
{
  frame_t *td = p->rxinfout;
  if (! td || td->len == MAXLEN)
  {
    disable();
    if (bfree && bfree->next)
    {
      alloc(td);
      enable();
      td->next = p->rxinfout;
      p->rxinfout = td;
      td->len = 1; // jump over PID byte
      td->index = 0;
      td->channel = 0;
    }
    else
    {
      enable();
    }
  }
  td->inhalt[(td->len)++] = zeichen;
}

/*---------------------------------------------------------------------------*/

void iot_frame (frame_t *info)
//*************************************************************************
//
//*************************************************************************
{
  info->next = p->rxinfout;
  p->rxinfout = info;

}

/*---------------------------------------------------------------------------*/

void iotf (char *format, ...)
//*************************************************************************
//
//  String in Empfangspuffer ausgeben
//
//*************************************************************************
{
  va_list argpoint;
  char cbuf[60];
  char *cb = cbuf;
  va_start(argpoint, format);
  vsprintf(cbuf, format, argpoint);
  va_end(argpoint);
  while (*cb) iot(*(cb++));
}

/*---------------------------------------------------------------------------*/

void iotcall (byte *rufz, int stern)
//*************************************************************************
//
//  Rufzeichen im AX.25-Format auf dem Bildschirm ausgeben
//
//*************************************************************************
{
  char asc[10];
  callasc(rufz, asc);
  iotf("%s", asc);
  if (stern && rufz[6] & 0x80) iot('*');
}

/*---------------------------------------------------------------------------*/

void monitor (frame_t *packl)
//*************************************************************************
//
//  Ein Frame im Monitor ausgeben
//
//*************************************************************************
{
  byte mcommand = 0, mresponse = 0, mpoll = 0, ctrl;
  byte *desti;
  int  plen = packl->len;
  byte *inh = packl->inhalt;
  int mindex = 0;
  int pid, i = 0;
  int damafl = 0;
  port_t *psave = p;
  p = pfeld[0];
  while (! (inh[i] & 1)) i++;
  if (i == 1)
  {
    iot(0xfd);
    iot(packl->channel & 0x80 ? 'T' : 'R');
    if (channels > 1)
      iotf("%d", packl->channel & 0x7f);
    if (packl->txdtime != 255)
      iotf(":%02d", (packl->txdtime * 13) / 10);
    iotf(doszeit(biosminuten));
    iotf("%d>", (inh[0] << 6) + (inh[1] >> 2));
    iot((inh[2] >> 2) + 0x20);
    iot((((inh[2] & 0x03) << 4) | ((inh[3] & 0xf0) >> 4)) + 0x20);
    iot((((inh[3] & 0x0f) << 2) | ((inh[4] & 0xc0) >> 4)) + 0x20);
    iot((inh[4] & 0x3f) + 0x20);
    iot((inh[5] >> 2) + 0x20);
    iot((((inh[5] & 0x03) << 4) | ((inh[6] & 0xf0) >> 4)) + 0x20);
    if (inh[6] & 15)
      iotf("-%d", inh[6] & 15);
    mindex = 7;
    mcommand = !! (inh[1] & 2);
    mresponse = ! mcommand;
  }
  else
  {
    if ((i % 7) != 6 || i > 69) goto mend;
    damafl = ! (inh[13] & 0x20);
    mhput(packl);
    if (chainlen(p->rxinfout) > w.maxmon) goto mend;
    if (! ((w.val.monitor >> (packl->channel & 127)) & 1)) goto mend;
    if (w.val.mselect)
    {
      int jawoll = 0;
      i = 0;
      do
      {
        switch (w.val.mselect % 10)
        {
          case 1:
            if (   (   (   callwildcmp(inh + 7, w.val.mcalls + i)
                        && callwildcmp(inh, w.val.mcalls + i + 7))
                    || (   callwildcmp(inh + 7, w.val.mcalls + i + 7)
                        && callwildcmp(inh, w.val.mcalls + i) ))
                && ! (i %14)) jawoll++;
            break;
          case 2: if (callwildcmp(inh, w.val.mcalls + i)) jawoll++;
            break;
          case 3: if (callwildcmp(inh + 7, w.val.mcalls + i)) jawoll++;
            break;
        }
        i += 7;
      }
      while (! (w.val.mcalls[i - 1] & 1));
      if (w.val.mselect >= 10) jawoll = ! jawoll;
      if (! jawoll) goto mend;
    }
    mcommand = inh[6] & 0x80;
    mresponse = inh[13] & 0x80;
    desti = inh;
    mindex += 7;
    iot(0xfd);
    iot(packl->channel & 0x80 ? 'T' : 'R');
    if (channels > 1)
      iotf("%d", packl->channel & 0x7f);
    if (packl->txdtime != 255)
      iotf(":%02d", (packl->txdtime * 13) / 10);
    iotf(doszeit(biosminuten));
    iotcall(inh + 7, 0);               // Sourcecall ausgeben
    while (! (inh[6 + mindex] & 0x01)) // Digicalls ausgeben
    {
      mindex += 7;
      iot('/');
      iotcall(inh + mindex, 1);
    }
    iot('>');                          // Destinationcall ausgeben
    iotcall(desti, 0);
    mindex += 7;
  }
  iot('>');
  mpoll = inh[mindex] & 0x10;
  ctrl = inh[mindex++] & 0xef;
  if (! (ctrl & 1)) iot ('I'), iot(((ctrl >> 1) & 7) | '0'), iot(((ctrl >> 5) & 7) | '0');
  else if (ctrl == SABM) iotf("SABM");
  else if (ctrl == DISC) iotf("DISC");
  else if (ctrl == UA)   iotf("UA");
  else if (ctrl == UI)   iotf("UI");
  else if (ctrl == DM)   iotf("DM");
  else if (ctrl == FRMR) iotf("FRMR");
  else if ((ctrl & 3) == 1)
  {
    if ((ctrl & 0x0f) == RR) iotf("RR");
    else if ((ctrl & 0x0f) == REJ) iotf("REJ");
    else if ((ctrl & 0x0f) == RNR) iotf("RNR");
    iot((ctrl >> 5) | '0');
  }
  else iotf ("%02X", ctrl);                // Kontrollfeld ausgeben
  if (mcommand ^ mresponse)
  {
    iot(',');
    if (mcommand  &&   mpoll) iot('P');
    if (mcommand  && ! mpoll) iot('C');
    if (mresponse &&   mpoll) iot('F');
    if (mresponse && ! mpoll) iot('R');
  }
  else if (mpoll) iotf(",S");
  if (damafl) iotf(",DAMA"); // "DAMA"
  if (mindex < plen)
  {
    byte ch;
    iotf(",%02X:\xfe", pid = inh[mindex++]);
    if (w.val.headeron) ch = CR;
    else ch = ' ';
    if (pid == 0xcf && ctrl != UI)
    {
      if ((mindex + 19) < plen)
      {
        iot(ch);
        iotcall(inh + mindex, 0);
        iot('>');
        iotcall(inh + mindex + 7, 0);
        mindex += 14;
        for (i = 0; i < 6; i++) iotf(" %02X", inh[mindex++]);
        iot(' ');
        ch = CR;
      }
    }
    else if (pid == 0xcc || pid == 0xcd)
    {
      mindex += 40;
      iotf("<TCP/IP>");
    }
    else if ((plen - mindex) > 40)
    {
      int binfl = 0, j;
      for (j = mindex; j < (mindex + 40); j++)
        if (inh[j] > 'z' || inh[j] < ' ') binfl++;
      if (binfl > 10)
      {
        iotf("%c<BIN %d Bytes>", ch, plen - mindex);
        mindex = plen;
        ch = CR;
      }
    }
    while (mindex < plen)
    {
      if (ch != inh[mindex] && ch < 0xfd) iot(ch);
      else if (ch != CR && ch < 0xfd) iot(ch);
      ch = inh[mindex++];
    }
    if (ch != CR) iot(ch);
  }
  iot(0xff);
  mend:
  enable();
  p = psave;      // wieder auf alten Port zurueckschalten
}

/*---------------------------------------------------------------------------*/

void linkmsg (linkmsg_t message)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 7;
  char *s;
  if (glob_messages)
  {
    frame_t *info;
    if (bfree) alloc(info);
    else return;
    info->len = 0;
    info->index = 0;
    info->channel = 1;
    s = (char *) info->inhalt;
    if (message != msg_disconnected)
      p->logintime = biosminuten;
    sprintf(s, " %c%s", (int) message, doszeit(biosminuten));
    callasc(p->adrfield, s + strlen(s));
    while (! (p->adrfield[i + 6] & 1))
    {
      i += 7;
      strcat(s, " ");
      callasc(p->adrfield + i, s + strlen(s));
    }
    if (w.val.blink == 2) w.val.blink = 3;
    strcat(s, "\r");
    info->len = strlen(s);
    iot_frame(info);
  }
}
