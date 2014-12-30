/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------
  Adressfeldkonvertierung fuer Autorouting
  ----------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

mheard_t *l2_mhlist;

/*---------------------------------------------------------------------------*/

int incssid (call_t *src, call_t *dest, int modify)
//*************************************************************************
//
//  sucht nach etwaigem gleichen Rufzeichenpaar in den Links
//  und erhoeht ggf. die SSID des Quellrufzeichens um 1
//
//*************************************************************************
{
  unsigned int j = 0;
  int rufzeichengleich;
  do
  {
    unsigned int portzaehler;
    port_t *pp;
    rufzeichengleich = 0;
    for (portzaehler = 0; portzaehler < ports; portzaehler++)
    {
      if ((pp = pfeld[portzaehler]) != NULL)
      {
        if (   callcmp(pp->adrfield, dest)
            && callcmp(pp->adrfield + 7, src)
            && (pp->lstate > disconnected))
          rufzeichengleich++;
      }
    }
    if (rufzeichengleich && modify)
    {
      if ((src[6] & 0x1e) == 0x1e)
        src[6] &= 0xe3; // ist SSID=15 ?? dann auf 1 setzen
      else
        src[6] += 2;    // ansonsten um 1 erhoehen
    }
    if (j++ > 16) break; // Notbremse gegen Zerstoerer
  } while (rufzeichengleich && modify);
  return rufzeichengleich;
}

/*---------------------------------------------------------------------------*/

int nachbar (call_t *nextcall)
//*************************************************************************
//
//  durchsucht die LINKS-Liste nach dem angegebenen Call und gibt
//  den Linkoffset zurueck.
//  Ist das Call nicht enthalten wird NOTFOUND zurueckgegeben.
//
//*************************************************************************
{
  unsigned int i;
  for (i = 0; i < MAXLINKS; i++) // zuerst mit passender SSID suchen
  {
    if (w.linkcall[i][0] && callcmp(nextcall, w.linkcall[i])) return i;
  }
  for (i = 0; i < MAXLINKS; i++) // dann nochmal mit evtl unpassender SSID
  {
    if (w.linkcall[i][0] && callwildcmp(nextcall, w.linkcall[i])) return i;
  }
  return NOTFOUND;
}

/*---------------------------------------------------------------------------*/

mheard_t *findmh (call_t *findcall)
//*************************************************************************
//
//  sucht in der MH-Liste einen Eintrag und gibt einen Zeiger
//  darauf zurueck.
//  Verwendet einen binaeren Suchalgorithmus, um moeglichst schnelle
//  Zugriffszeit zu ermoeglichen, da die Routine bei jedem
//  empfangenen Frame durchlaufen wird.
//
//*************************************************************************
{
  int step = (w.mhlen + 1) >> 1;
  int mindex = step - 1;
  do
  {
    switch (callord(l2_mhlist[mindex].call, findcall, 0))
    {
      case LOWER:  step >>= 1; mindex += step; break;
      case HIGHER: step >>= 1; mindex -= step; break;
      case SAME: return &l2_mhlist[mindex];
    }
  } while (step);
  return NULL;
}

/*---------------------------------------------------------------------------*/

int oldest (void)
//*************************************************************************
//
//  sucht nach dem aeltesten Eintrag in der MH-Liste
//
//*************************************************************************
{
  long unsigned oldtime = (long unsigned) -1; // hoechstmoegliche Zahl:
                                              // 4294967295UL
  int oldindex = NOTFOUND;
  unsigned int i;
  for (i = 0; i < w.mhlen; i++)
    if ((unsigned long) l2_mhlist[i].rtime < oldtime && l2_mhlist[i].call[0])
    {
      oldtime = (unsigned long) l2_mhlist[i].rtime;
      oldindex = (int) i;
    }
  return oldindex;
}

/*---------------------------------------------------------------------------*/

int oldest114 (void)
//*************************************************************************
//
//  sucht nach dem aeltesten Eintrag in der MH-Liste SSID 1-14
//
//*************************************************************************
{
  long unsigned oldtime = (long unsigned) -1; // hoechstmoegliche Zahl:
                                              // 4294967295UL
  int oldindex = NOTFOUND;
  unsigned int i, anzahl = 0;
  for (i = 0; i < w.mhlen; i++)
  {
    int ssid = (l2_mhlist[i].call[6] >> 1) & 15;
    if (l2_mhlist[i].call[0] && (ssid > 0))
    {
      if ((unsigned long) l2_mhlist[i].rtime < oldtime)
      {
        oldtime = (unsigned long) l2_mhlist[i].rtime;
        oldindex = (int) i;
      }
      anzahl++;
    }
  }
  if (anzahl > 80) return oldindex;
  else return NOTFOUND;
}

/*---------------------------------------------------------------------------*/

mheard_t *newmh (call_t *newcall)
//*************************************************************************
//
//  eroeffnet in der MH-Liste einen neuen Eintrag und gibt einen
//  Zeiger darauf zurueck.
//  Ist die MH-Liste voll, so wird der aelteste Eintrag aus der
//  Liste entfernt. Die Liste wird stets alphabetisch angelegt, um
//  binaere Suche zu ermoeglichen.
//
//*************************************************************************
{
  int oldbuf;
  unsigned int i = 0;
  mheard_t *entry;
  entry = findmh(newcall); // bereits enthalten?
  if (entry) return entry;
  if (l2_mhlist[w.mhlen-1].call[0]) // letzter Eintrag besetzt?
  {
    oldbuf = oldest114();
    if (oldbuf == NOTFOUND) oldbuf = oldest();
    memmove(&l2_mhlist[oldbuf], &l2_mhlist[oldbuf + 1],
            (w.mhlen - (oldbuf + 1)) * (sizeof(mheard_t)));
    l2_mhlist[w.mhlen-1].call[0] = 0;
  }
  while ((callord(l2_mhlist[i].call, newcall, 0) == LOWER)
         && (i < (w.mhlen - 1)))
    i++;
  entry = &l2_mhlist[i];
  if (i < (w.mhlen - 1) && ! callcmp(entry->call, newcall))
    memmove(entry + 1, entry, (w.mhlen - (i + 1)) * (sizeof(mheard_t)));
  callcopy(newcall, entry->call);
  entry->viacall[0] = 0;
  entry->frames = 0;
  entry->rtime = 0;
  return entry;
}

/*---------------------------------------------------------------------------*/

void mhput (frame_t *packl)
//*************************************************************************
//
//  sucht das Herkunftsrufzeichen eines empfangenen Frames
//  in der MH-Liste.
//  Wird das Rufzeichen gefunden, erfolgt ein Update der Daten,
//  andernfalls wird es neu angelegt.
//
//*************************************************************************
{
  mheard_t *mhentry;

  if (packl->channel > 127) return;
  mhentry = findmh(packl->inhalt + 7);
  if (! mhentry)
  {
#if WITHNUDL
    if (callok(packl->inhalt + 7)) mhentry = newmh(packl->inhalt + 7);
    else return;
#else
    mhentry = newmh(packl->inhalt + 7);
#endif
  }
  mhentry->kanal = packl->channel & 127;
  mhentry->rtime = w.zeit;
  mhentry->frames++;
  mhentry->viacall[0] = 0;
  if (! (packl->inhalt[13] & 1) && packl->inhalt[20] > 128)
    callcopy(packl->inhalt + 14, mhentry->viacall);
}

/*---------------------------------------------------------------------------*/

int router (byte *altadr, byte *neuadr)
//*************************************************************************
//
//  ist die gesamte Adressfeldkonvertierung. Es wird der in der
//  Digikette folgende Linkpartner untersucht und entsprechend
//  Digipeaterrufzeichen ein- und ausgefuegt. Ausserdem wird
//  der Linkkanal festgestellt.
//
//*************************************************************************
{
  byte *src = altadr + 7;
  byte *dest = altadr;
  byte *firstdigi = NULL;
  byte *pseudodigi = NULL;
  byte *nextcall = dest;
  unsigned int i = 14, pdcount = 0, dcount = 0;
  int kanal = -1;
  while (! (altadr[i - 1] & 1) && (altadr[i + 6] & 0x80) == 0x80 )
  {
    pseudodigi = altadr + 14;
    i += 7;
    pdcount++;
  }
  if (! (altadr[i - 1] & 1))
  {
    firstdigi = altadr + i;
    nextcall = firstdigi;
  }
  if (firstdigi && ischannel(firstdigi) >= 0)
  {
    kanal = ischannel(firstdigi);
    if (firstdigi[6] & 1)
    {
      firstdigi = NULL;
      nextcall = dest;
    }
    else
    {
      firstdigi += 7;
      nextcall = firstdigi;
    }
  }

  if (firstdigi)
  {
    dcount++;
    while (! (firstdigi[7 * dcount - 1] & 1)) dcount++;
  }
  if (kanal < 0)
  {
    kanal = nachbar(nextcall);
    if (kanal >= 0) kanal = w.linkkanal[kanal];
    if (kanal >= (int) channels) kanal= -1;
  }
  if (kanal < 0)
  {
    mheard_t *mm = findmh(nextcall);
    if (mm)
    {
      if (! *mm->viacall) kanal = mm->kanal;
    }
  }
  incssid(src, dest, 1);
  callcopy(src, neuadr + 7);
  callcopy(dest, neuadr);
  for (i = 0; i < (pdcount * 7); i += 7)
  {
    callcopy(pseudodigi + i, neuadr + i + 14);
    neuadr[i + 20] |= 0x80; // H-Bit setzen
  }
  for (i = 0; i < (dcount * 7); i += 7)
    callcopy(firstdigi + i, neuadr + i + pdcount * 7 + 14);
  neuadr[13 + 7 * pdcount + 7 * dcount] |= 1; // Endebit
  if (kanal < 0) kanal = 0;
  return kanal;
}
