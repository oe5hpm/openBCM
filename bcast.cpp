/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------
  Mailbox-Broadcast nach SAT-Verfahren
  ----------------------------------------------


  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

// NOT FINISHED - DO NOT USE!

#include "baycom.h"
#include "flexappl.h"

#ifdef BCAST

#define L2_BCAST_DEST  "QST-1"
#define L2_BCAST_PID   0xbb

void bcast (char * befbuf)
//****************************************************************************
//
//****************************************************************************
{
  boxqso_t *q = &qso[port];
  int a;
  char fn[20];
  char ziel[30];
  char target[11]; //target call
  int i;
  unsigned long n = 0;
  char lcmd[40];

  befbuf = nexttoken(befbuf, target, 10);
  strupr(target); //must be uppercase
  befbuf = nexttoken(befbuf, ziel, 29);
  trace(report, "bcast", "connect %s", ziel);
  timeout(1);
  lastcmd("Connect");
  if (makeconnect(m.mycall[0], ziel))
  {
    setsession();
    *b->logincall = 0;
    putf("%c%cBROADCAST %s\n", 0x80, 0xff, L2_BCAST_DEST);
    putflush();
    sprintf(fn, "bcast.bcm");
    if (file_isreg(fn))
    {
      trace(report, "bcast", "sending %s", fn);
      FILE *f = s_fopen(fn, "lrt");
      lastcmd("TX");
      while ((a = fgetc(f)) != EOF)
      {
        getclear_tnc();
        timeout(1);
        waitfor(e_ticsfull);
        putv(a);
        n++;
        if (a == LF) //line by line mode
        {
          l2_set_pid(q->qsonum, BCAST_PID);
          putflush();
          sprintf(lcmd, "bcast %ld Bytes", n);
          lastcmd(lcmd);
        }
      }
      s_fclose(f);
    }
    lastcmd("Delay");
    for (i = 0; i < 100; i++)
    {
      wdelay(226);
      getclear_tnc();
    }
    mblogout(0);
    mbdisconnect(1); //wait for data to be sent
  }
}

#endif
