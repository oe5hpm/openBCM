/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  Kommandoprozessor
  -----------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int readcmd (char *beftab[], char **buffer, unsigned int minimum)
//*************************************************************************
//
//  vergleicht die Kommandos in einer Befehlstabelle mit einem
//  Stringanfang. Wird das Kommando gefunden, so wird der String-
//  zeiger entsprechend nachgefuehrt.
//
//  beftab[] = Pointer-Array, dessen Eintraege auf die Befehls-Strings
//             zeigen. Das Array muss an letzter Stelle stets mit
//             dem NULL-Pointer besetzt sein.
//  *buffer    Zeiger auf einen String, in dem am Anfang der Befehl
//             steht.
//  minimum    Gibt die Anzahl der mindestens ~uebereinstimmenden
//             Zeichen an, damit ein Befehl als solcher akzeptiert
//             wird.
//             0 bedeutet, dass der Befehl stets mit String-Ende (\0)
//             oder mit einem Blank abgeschlossen sein muss (fuer Node)
//
//*************************************************************************
{
  unsigned int i = 0, j = 0, cmd = 0, maxcmd = 0;

  while (beftab[i])
  {
    j = 0;
    while (beftab[i][j] && (toupper (buffer[0][j]) == beftab[i][j])) j++;
    i++;
    if (j > maxcmd && j >= minimum)
    { // if(minimum || (buffer[0][j]==0) || (buffer[0][j]==' '))
      if (! buffer[0][j] || buffer[0][j] == ' ' || buffer[0][j] == '=')
      {
        maxcmd = j;
        cmd = i;
      }
    }
  }
  *buffer += maxcmd;
  *buffer += blkill(*buffer);
  return cmd;
}

/*---------------------------------------------------------------------------*/
