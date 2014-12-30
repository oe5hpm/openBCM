/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------
  Konfiguration des Flexnet-Digis von der Box aus
  -----------------------------------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980215 OE3DZW cleared rx-buffer
//19981010 jj     created outputfile

#include "baycom.h"

/*---------------------------------------------------------------------------*/

void cfgflex (char * befbuf)
//*************************************************************************
//
// cfgflex
//
//*************************************************************************
{
  int a, i;
  int promptz = 0; // Zaehler der Nodeprompts
  char fn[20]; // Dateiname des Configfiles
  char fo[20]; // Dateiname des Antwortfiles
  char ziel[30]; // Nodecall
  char line[255]; // Noderueckgabe

  strcpy(fn, "cfgflex.bcm");
  strcpy(fo, "cfgflex.out");
  strncpy(ziel, befbuf, 29);
  ziel[29] = 0;
  trace(report, "cfgflex", "connect %s", ziel);
  timeout(1);
  lastcmd("Connect");
  if (makeconnect(m.mycall[1], ziel))
  {
    setsession();
    *b->logincall = 0;
    if (filetime(fn))
    {
      trace(report, "cfgflex", "sending %s", fn);
      FILE *f = s_fopen(fn, "lrt");
      lastcmd("TX cfg_file");
      while ((a = fgetc(f)) != EOF)
      {
        getclear_tnc();
        timeout(1);
        waitfor(e_ticsfull);
        putv(a);
        if (a == LF) promptz++; // zaehlt prompts vorwaerts
      }
      putv(LF);
      putv(LF);
      s_fclose(f);
      trace(report, "cfgflex", "receive %s", fo); // speichert Rueckgaben
      FILE *g = s_fopen(fo, "lwt");
      lastcmd("RX cfg_result");
      if (g) // JJ
      {
        while (promptz > 0)
        {
          getline(line, BUFLEN - 1, 1);
          if (! strncmp(line, "=>", 2))
            promptz--; // zaehlt prompts rueckwaerts
          if (! strncmp(line, "***", 3))
            promptz = 0; // link failure
          fprintf(g, line);
          fputc(LF, g);
        }
      }
      s_fclose(g);
    }
    lastcmd("Delay");
    for (i = 0; i < 100; i++)
    {
      wdelay(226);
      getclear_tnc();
    }
    mblogout(0);
    mbdisconnect(1); // wait for data to be sent
  }
}

/*---------------------------------------------------------------------------*/
