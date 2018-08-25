/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  Layer1-Driver


  --------------------
  AX.25 Call Utilities
  --------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/
//000930 DK2UI  index i goes below 0 in some functions, corrected

#include "baycom.h"

#define cbuchst(c) ((c>=('A'*2))&&(c<=('Z'*2)))
#define czahl(c)   ((c>=('0'*2))&&(c<=('9'*2)))
#define cbz(x)     (cbuchst(x)||czahl(x))

/*---------------------------------------------------------------------------*/

int callcmp (byte *call1, byte *call2)
//*************************************************************************
//
//  vergleicht 2 Rufzeichen incl SSID - optimiert auf Tempo
//
//*************************************************************************
{
  unsigned int i;

  for (i = 5; i > 0; --i)
  {
    if (call1[i] != call2[i]) return 0;
  }
  return (! ((call1[6] ^ call2[6]) & 0x1e));
}

/*---------------------------------------------------------------------------*/

int callwildcmp (byte *call1, byte *call2)
//*************************************************************************
//
//  vergleicht 2 Rufzeichen, Joker erlaubt
//  Joker duerfen nur im zweiten Rufzeichen enthalten sein
//
//*************************************************************************
{
  unsigned int i;

  for (i = 5; i > 0; --i)
  {
    if (call1[i] != call2[i] && call2[i] != ('?' << 1)) return 0;
  }
  return (! ((call1[6] ^ call2[6]) & 0x1e) || ((call2[6] & 0x7e) == 0x60));
}

/*---------------------------------------------------------------------------*/

void callasc (byte *rufz, char *buf)
//*************************************************************************
//
//  Rufzeichen im AX.25-Format in String umwandeln
//
//*************************************************************************
{
  unsigned int i;

  for (i = 0; i < 6; i++) if (rufz[i] > 0x40) *(buf++) = rufz[i] >> 1;
  if ((rufz[6] & 0x1e) || *rufz == 0x40)
    sprintf((char *) buf, "-%d", (rufz[6] & 0x1e) >> 1);
  else
    *buf = 0;
}

/*---------------------------------------------------------------------------*/

int callcmp1 (byte *call1, byte *call2)
//*************************************************************************
//
//  vergleicht 2 Rufzeichen ohne SSID - optimiert auf Tempo
//
//*************************************************************************
{
  unsigned int i;

  for (i = 5; i > 0; --i)
  {
    if (call1[i] != call2[i]) return FALSE;
  }
  return TRUE;
}

/*---------------------------------------------------------------------------*/

int callord (byte *call1, byte *call2, int wildcard)
//*************************************************************************
//
//  vergleicht 2 Rufzeichen mit/ohne SSID    groesser/kleiner
//
//*************************************************************************
{
  unsigned int i;

  if (! *call1) return HIGHER; // fuer binaeres Suchen erforderlich
  for (i = 0; i < 6; i++)
  {
    if (call1[i] != call2[i])
    {
      if (call1[i] < call2[i]) return LOWER;
      if (call1[i] > call2[i]) return HIGHER;
    }
  }
  if (wildcard && ((call1[6] & 0x7e) == 0x60)) return SAME;
  else if ((call1[6] & 0x1e) < (call2[6] & 0x1e)) return LOWER;
  else if ((call1[6] & 0x1e) > (call2[6] & 0x1e)) return HIGHER;
  else return SAME;
}

/*---------------------------------------------------------------------------*/

void callcopy(byte *call1,byte *call2)
//*************************************************************************
//
//  kopiert 1 Rufzeichen und loescht H-Bit und Endebit
//
//*************************************************************************
{
  unsigned int i;

  for (i = 0; i < 7; i++) call2[i] = call1[i];
  call2[6] = call2[6] & 0x7e; // etwaiges C- oder H-Bit loeschen
}

/*---------------------------------------------------------------------------*/

int ischannel (byte *call)
//*************************************************************************
//
//  hier wird festgestellt, ob sich in einem Rufzeichen eine
//  gueltige Kanalnummer
//  der Art  A B C
//  oder     0 1 2 3  ... etc.
//  oder     P0 P1 P2 ... etc.
//  befindet. Wenn ja, wird die Kanalnummer zurueckgegeben, wenn nicht
//  dann wird -1 zurueckgegeben
//
//*************************************************************************
{
  int cindex = (*call == ('P' * 2)); // P am anfang eliminieren
  int a;

  if (cbuchst(*call) && (call[1] == 0x40))
    return (*call >> 1) - 'A'; // Einstelliger Buchstabe? wert des Kanals
  if (! czahl(call[cindex]))
    return -1;                 // Zahl am Anfang?
  a = (call[cindex] >> 1) - '0'; // Wert des Kanals
  if (call[cindex + 1] == 0x40) return a;
  if (czahl(call[cindex + 1]) && (call[cindex + 2] == 0x40))
    return a * 10 + ((call[cindex + 1] >> 1) - '0'); // 2stelliger Wert
  else return -1;                                    // unpassend
}

/*---------------------------------------------------------------------------*/

int readcall (char *bf, byte *axcall)
//*************************************************************************
//
//*************************************************************************
{
  int i, j, k;

  j = i = blkill(bf);
  if ((toupper(bf[i]) == 'V'))
  {
    i++;
    if (toupper(bf[i]) == 'I')
    {
      i++;
      if (toupper(bf[i]) == 'A') i++;
    }
  }
  if (bf[i] != ' ') i = j;
  j = 0;
  i += blkill(bf + i);
  if (! bf[i]) return 0;
  for (k = 0; k < 6; k++) axcall[k] = 0x40; // Call vorbesetzen
  axcall[6] = 0x60;
  while ((bf[i] != '-') && (bf[i] != ',') && (bf[i] > ' ') && (j < 6))
  {
    axcall[j++] = toupper(bf[i]) * 2; i++;
  }
  if (bf[i] == '-')
  {
    i++;
    if (! bf[i]) axcall[6] = 0;
    else if (0x62 == (axcall[6] = bf[i++] * 2))
    {
      if (bf[i] < '6' && bf[i] >= '0')
        axcall[6] += bf[i++] * 2 - 0x4e;
    }
  }
  while (bf[i] && bf[i] != ' ' && bf[i] != ',') i++;
  return i + blkill(bf + i);
}

/*---------------------------------------------------------------------------*/

int readclist (char *bf, byte *axlist, int maxanzahl)
//*************************************************************************
//
//*************************************************************************
{
  int x = 0, z = 0, i = 0;

  while (maxanzahl-- && (z = readcall(bf + x, axlist + i)) > 0)
  {
    x += z;
    i += 7;
  }
  if (i) axlist[i - 1] |= 1;
  return x;
}
