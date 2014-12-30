/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------
  Morse Daemon
  ------------

  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"
#ifdef _MORSED
#include "morse.h"

/*---------------------------------------------------------------------------*/

void morsedaemon (char *tname)
//****************************************************************************
//
//****************************************************************************
{
  trace(report, "morsed", "start");
  do
  {
//   printf("*\n");  //dummy
    wdelay(1000);
  }
  while (! runterfahren);
}

/*---------------------------------------------------------------------------*/

void morse_char (char x)
//****************************************************************************
//
//****************************************************************************
{
  int i, pos = -1;

  putf("%c -", x);
  //search for char;
  for (i = 0; cs[i] && (pos == -1); i++)
    if (x == cs[i])
      pos = i;
  if (pos == -1) return; //letter can not be morsed
  putf("(%d)> %s\n", i, mos[pos]);
  for (i=0; mos[pos][i]; i++)
  {
    if (mos[pos][i]=='.')
    {
      sound(DOTFREQ);
      delay(DOTDUR);
      nosound();
      delay(INTRASPACING);
    }
    else
    {
      sound(DASHFREQ);
      delay(DASHDUR);
      nosound();
      delay(INTRASPACING);
    }
  }
  wdelay(CHARSPACING);
  return;
}

/*---------------------------------------------------------------------------*/

void morse_word (char *text)
//****************************************************************************
//
//****************************************************************************
{
  int i;

  putf("word: %s\n", text);
  strlwr(text); //lower case
  for (i = 0; text[i]; i++)
  {
    if (text[i]=='·' && i > 0) // ? -> ss
    {
      text[i]='s';
      i--;
      text[i]='s';
    }
    if (text[i]=='c'&& text[i+1]=='h') //ch -> #
    {
      i++;
      text[i]='#';
    }
    if (text[i]=='@')
      text[i]='$';
    if (text[i]=='é')
      text[i]='Ñ';
    if (text[i]=='ô')
      text[i]='î';
    if (text[i]=='ö')
      text[i]='Å';

    morse_char(text[i]);
  }
  wdelay(WORDSPACING-CHARSPACING);
  return;
}

/*---------------------------------------------------------------------------*/

void morse_line (char *line)
//****************************************************************************
//
//****************************************************************************
{
  char text[40];
  char *pline = line;

  putf("line: %s\n", line);
  do
  {
    pline = nexttoken(pline, text, 39);
    if (text[0])
      morse_word(text);
  }
  while (text[0] && strlen(text) < 38);
  return;
}

#endif


