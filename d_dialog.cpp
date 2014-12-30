/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------
  DOS: Behandlung von Dialogboxen
  -------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#define HISTLEN 10
static char history[HISTLEN][60];
static int histwrite=0;

/*---------------------------------------------------------------------------*/

int editline(unsigned &spalte, unsigned &zeile, int maxlen, char *string)
//*************************************************************************
//
//  Zeileneditor, in dem alle horizontalen Cursorbewegungen und Eingaben
//  verarbeitet werden. Die Zeile wird komplett in einem Puffer bearbeitet
//
//*************************************************************************
{
  int retwert = OK;
  int i = strlen(string), a;
  int curcol = 0;
  int modified = 0;
  int histread = histwrite;

  e->linbuf = string;
  if (maxlen > (e->xlen-spalte-1))
    maxlen = e->xlen-spalte-1;
  memset(string+i, ' ', maxlen-i); // Rest der Zeile mit Blanks vorbesetzen
  string[maxlen] = 0;
  setzcurs(curcol+spalte, zeile, e);
  wf(e->x+spalte, e->y+zeile, color.put_text/*box_input*/,
     "%*s", maxlen, string);
  while (((a = waitkey()) & 255) != ESC)
  {
    if ((a&255) == 0)
    {
      a >>= 8;
      modified++;
      retwert = a;
      switch (a)
      {
        case INSERT:
          break;
        case K_DELETE:
          for (i = curcol; i < maxlen-1; i++)
            string[i] = string[i+1];
          string[maxlen-1] = ' ';
          break;
        case LINKS:
          if (curcol)
            curcol--;
          break;
        case RECHTS:
          if (curcol < (maxlen-1))
            curcol++;
          break;
        case POS1:
          curcol = 0;
          break;
        case ENDE:
          for (i = maxlen-1; i&&(string[i-1] == ' '); i--);
            curcol = i;
          break;
        case RUNTER:
          if (histread != histwrite)
          {
            histread = (histread+1)%HISTLEN;
            goto histcopy;
          }
          else
          {
            memset(string, ' ', maxlen);
            curcol = 0;
          }
          break;
        case RAUF:
          {
            histread = (histread+HISTLEN-1)%HISTLEN;
            histcopy:
            strncpy(string, history[histread], maxlen-1);
            string[maxlen-1] = 0;
            curcol = strlen(string);
            memset(string+curcol, ' ', maxlen-curcol);
          }
          break;
      }
    }
    else
    {
      a &= 255;
      if (a == CR)
        goto ende;
      if (a == TAB)
      {
        retwert = TAB;
        goto ende;
      }
      if (a == LINEDEL) // CTRL-Y
      {
        for (i = 0; i < maxlen; i++)
          string[i] = ' ';
        curcol = 0;
      }
      else if (a == BACKSPACE)
      {
        modified++;
        if (curcol)
        {
          curcol--;
          for (i = curcol; i < maxlen-1; i++)
            string[i] = string[i+1];
          string[maxlen-1] = ' ';
        }
      }
      else
      {
        if (!modified)
        {
          for (i = 0; i < maxlen; i++)
            string[i] = ' ';
          modified++;
        }
        if(curcol<maxlen)
        {
          for (i = maxlen-1; i > curcol; i--)
            string[i] = string[i-1];
          string[curcol] = a;
          if (curcol < (maxlen-1))
            curcol++;
        }
      }
    }
    setzcurs(curcol+spalte, zeile, e);
    wf(e->x+spalte, e->y+zeile,
       color.put_text/*modified?color.box_modified:color.box_input*/,
       "%*s", maxlen, string);
  }
  retwert = ESC;
  ende:
  i = maxlen-1;                          // von hinten her
  while ((i >= 0) && (string[i] == ' ')) // Ende der Eingabe feststellen
    string[i--] = 0;
  setzcurs(0, 255, e);
  if (strlen(string))
  {
    strncpy(history[histwrite], string, 59);
    history[histwrite][59] = 0;
    histread = histwrite = (histwrite+1)%HISTLEN;
  }
  e->linbuf = NULL;
  return retwert;
}

/*---------------------------------------------------------------------------*/

int inputwin (char *title, char *prompt, char *string, int maxlen)
{
  int retwert;
  unsigned zeile = 4;
  unsigned spalte = 2;
  if (maxlen > 40)
    trace(tr_abbruch, "inputwin", "maxlen %d", maxlen);
  boxwopen(20, 10, maxlen+4, 7);
  wclear(color.box_text);
  wframe(color.box_frame, title, 0);
  wstr(2, color.box_text, prompt);
  retwert = editline(spalte, zeile, maxlen, string);
  boxwclose();
  return retwert;
}

/*---------------------------------------------------------------------------*/

#if 0
void choiceout (int x, int y, int col, char *text, int width, int tag)
{
  char txt[81];
  char idx[12];
  int i = 0, num = 1;

  if (strlen(text) > 80)
    trace(tr_abbruch, "choiceout", "strlen %s", text);
  idx[0] = 0;
  while (*text)
  {
    if (text[0] == LF)
    {
      txt[i++] = 0;
      idx[num++] = i;
    }
    else
      txt[i++] = text[0];
    text++;
  }
  txt[i] = 0;
  for (i = 0; i < num; i++)
    wf(x, y+i, col, " (%c) %-*s", (tag==i)?7:' ', width, txt+idx[i]);
}

/*---------------------------------------------------------------------------*/

void dia_update (dia_form *d)
{
  int col = color.box_okbutton;
  while (1)
  {
    switch (d->input % DIA_END)
    {
      case dia_bool:             // Eingabe Ein/Aus (zum Ankreuzen)
        wf(e->x+d->x, e->y+d->y,col,
           " [%c] %-*s ", d->dest[0]?'X':' ', d->width-4, d->prompt);
        break;

      case dia_choice:          // Auswahl mehrerer Punkte
        choiceout(d->x+e->x, d->y+e->y, col, d->prompt, d->width-4,
                  d->dest[0]);
        break;

      case dia_str:               // Eingabe eines Strings
        wf(e->x+d->x, e->y+d->y, col, " %s %-*s ", d->prompt,
           d->width, d->dest);
        break;

      case dia_text:             // keine Eingabe, nur Ausgabe
        wf(e->x+d->x, e->y+d->y, col, " %-*s ", d->width, d->prompt);
        break;
    }
    if(d->input >= DIA_END)
      break;
    d++;                     // um einen Eintrag weiter
  }
}

/*---------------------------------------------------------------------------*/

int dialogbox (char *title, dia_form *d)
{
  int retwert = OK;
  boxwopen(15, 3, 45, 20);
  wclear(color.box_text);
  wframe(color.box_frame, title, 0);
  dia_update(d);
  waitkey();
  boxwclose();
  return retwert;
}

/*---------------------------------------------------------------------------*/

/*
typedef struct dia_form
{ dia_input input;        // Art des Eingabefeldes
  int       width;        // max. Eingabebreite
  int       x;        // relative Position im Fenster
  int       y;
  char      *prompt;    // Ausgabe vor (nach) dem Eingabefeld
  char      *dest;        // Vorbesetzung und Resultat
} dia_form;
*/

/*---------------------------------------------------------------------------*/

void dia_test (void)
{
  static dia_form d[10]=
  {
    { dia_text, 10, 2, 2, "Screen Size", NULL },
    { dia_choice, 10, 2, 3, "25    Lines\n43/50 Lines", "\0" },
    { dia_bool, 10, 2, 6, "Mouse", "\1" },
    { dia_bool, 10, 2, 7, "Color", "\1" },
    { dia_bool, 10, 2, 8, "Insert", "\1" },
    { dia_bool, 10, 2, 9, "Auto save", "\1" },
    { dia_text, 10, 22, 2, "Echo", NULL },
    { dia_choice, 10, 22, 3, "none\nprompt\nalways", "\0" },
    { dia_bool, 10, 22, 7, "Bool1", "\0" },
    { dia_bool+DIA_END, 10, 22, 8, "Bool2", "\1" },
  };
  dialogbox("Screen Control", d);
}
#endif

/*---------------------------------------------------------------------------*/

int yesno (char *s, int vorbesetzung)
//*************************************************************************
//
//  Fragt in einem Fenster einen String ab mit der Moeglichkeit
//  folgende Sachen einzugeben:
//
//     J Y O  -> YES
//     N      -> NO
//     Esc    -> ESC
//
//  Vorbesetzung Ja=1 / Nein=0
//
//*************************************************************************
{
  int a;
  int retwert=NIX;
  char msg[80];

  boxwopen(20, 10, strlen(s)+13, 5);
  wclear(color.box_frame);
  wframe(color.box_frame, "Message", 0);
  sprintf(msg, "%s  (Y/N) %c", s, vorbesetzung?'Y':'N');
  wstr(2, color.box_text, msg);
  setzcurs(strlen(msg)+1, 2, e);

  do
  {
    a = waitkey()&255;
    switch (toupper(a))
    {
      case 'Y':
      case 'J':
      case 'O':
        retwert=YES;
        break;
      case 'N':
        retwert=NO;
        break;
      case ESC:
        retwert=ESC;
        break;
      case CR:
        retwert=vorbesetzung;
        break;
    }
  } while (retwert==NIX);

  boxwclose();
  setzcurs(0, 255, e);
//  if (retwert == ESC)
//    dia_test();
  return retwert;
}

/*---------------------------------------------------------------------------*/

void message (char *title, char *s[], unsigned zeit)
//*************************************************************************
//
//  Schreibt in einem Fenster eine Nachricht auf den Schirm und
//  wartet auf Tastendruck
//
//*************************************************************************
{
  int xmax = 0, ymax = 0;
  int x, y;
  int i = 0, a;

  while (s[ymax])
  {
    if ((a = strlen(s[ymax]))>xmax)
      xmax = a;
    ymax++;
  }
  x = 38-(xmax/2);
  y = 10-(ymax/2);
  if ((x<0) || (y<0))
    trace(tr_abbruch, "message", "size x=%d y=%d", x, y);
  boxwopen(x, y, xmax+4, ymax+6);
  setzcurs(0, 255, e);
  wclear(color.box_frame);
  wframe(color.box_frame, title, 0);
  while (s[i])
  {
    wstr(2+i, color.box_text, s[i]);
    i++;
  }
  zeit<<=3;
  if (zeit)
  {
    while (zeit)
    {
      setzcurs(0, 255, e);
      wdelay(127);
      zeit--;
      if (taskkbhit())
      {
        waitkey();
        break;
      }
    }
  }
  else
  {
    ws(x+(xmax/2), y+i+3, color.box_okbutton, "<ESC>");
    waitkey();
  }
  boxwclose();
}

/*---------------------------------------------------------------------------*/

void msgline (char *s)
{
  char *ms[2];
  ms[0] = s;
  ms[1] = NULL;
  message("", ms, 0);
}
