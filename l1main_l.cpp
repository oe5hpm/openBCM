/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------
  AX.25 Physical layer1 main functions
  ------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//#include <stdio.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>
//#include <termios.h>
#include "baycom.h"

/*---------------------------------------------------------------------------*/

frame_t *rxout = NULL;    // Zeiger auf den Layer 1 Empfangspuffer
frame_t *bfree = NULL;    // Zeiger auf naechsten freien Buffer, NULL=voll
l2_para_t w;

/*---------------------------------------------------------------------------*/

int l1state (int channel)
//*************************************************************************
//
//*************************************************************************
{
  int mask;
  switch (w.mode[channel].chmode)
  {
    case c_kiss: mask = l1state_kiss(channel); break;
    case c_axip: mask = l1state_axip(channel); break;
    default:     mask = 0;
  }
  if (rxout) mask |= CH_RXB;
  return mask;
}

/*---------------------------------------------------------------------------*/

void pactx (frame_t *packl)
//*************************************************************************
//
//  legt ein Frame in den Sendepuffer
//
//*************************************************************************
{
  int kanal = packl->channel;
  packl->next = NULL;
  if (w.mode[kanal].xmitok)
  {
    switch (w.mode[kanal].chmode)
    {
      case c_kiss: pactx_kiss(packl); break;
      case c_axip: pactx_axip(packl); break;
      default: goto nichtsenden;
    }
  }
  else
  {
    nichtsenden:
    packl->channel |= 0x80;
    vornehin(packl, rxout);
  }
}

/*---------------------------------------------------------------------------*/

int l1init (int einschalt)
//*************************************************************************
//
//*************************************************************************
{
  l1init_kiss(einschalt);
  l1init_axip(einschalt);
  return 1;
}

/*---------------------------------------------------------------------------*/

void layer1 (void)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int channel;
  for (channel = 0; channel < KANAELE; channel++)
  {
    switch (w.mode[channel].chmode)
    {
      case c_kiss: cyclic_kiss(channel); break;
      case c_axip: cyclic_axip(channel); break;
      default: break;
     }
  }
}
