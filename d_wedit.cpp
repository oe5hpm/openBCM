/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------
  Fenster-Editor
  --------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//19980202 OE3DZW corrected IBM-characters
//19990817 DH3MB  made this feature optional (define FEATURE_EDITOR in
//                baycom.h)

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#define EDLEN 64000U

/*---------------------------------------------------------------------------*/

/*
curline     current line -1     0..n
curcol      current column -1   0..n
maxbuf      last valid index into buf -1
maxlines    last valid line number -1
index
readindex
blockbegin  begin line
blockend
blockmark
modified
wordwrap
xoffset
readlines
*/

/*---------------------------------------------------------------------------*/

char *buf;
struct edstruct *e;

#ifdef FEATURE_EDITOR

static char searchstr[81];
static unsigned shiftgedrueckt = 0;

/*---------------------------------------------------------------------------*/

static void near putpos (void)
//*************************************************************************
//
//  Zeigt momentane Cursorposition im Buffer als Balken am Fensterrand
//
//*************************************************************************
{
  unsigned i, ypos =
     (unsigned)(((long)e->curline*(e->ylen-2-2*e->frame))/(e->maxlines+1));
  unsigned col;

  if (iskeytask())
    col = color.activ_frame;
  else
    col = color.back_frame;
  wc(e->xend, e->y+e->frame, col, 30);    // Pfeil hoch am oberen Ende
  wc(e->xend, e->yend-e->frame, col, 31); // Pfeil runter am unteren Ende
  for (i = (e->y+1+e->frame); i < (e->yend-e->frame); i++)
  // Balken am rechten Rand des Fensters
  {
    if (i == (ypos+e->y+2))
      wc(e->xend, i, col, (char)'þ'); // momentane Position markieren
    else
      wc(e->xend, i, col, (char)' '); // sonst frei lassen
  }
  if (e->frame)
  {
      wf(e->x+3, e->yend, col, " %d << %d:%d >>>",
                               e->modified, e->curline+1, e->curcol+1);
  }
}

/*---------------------------------------------------------------------------*/

static void near lineout (void)
//*************************************************************************
//
//  Gibt die aktuelle Zeile fuer den Horizontal-Editor aus
//  und setzt den Hardwarecursor
//
//*************************************************************************
{
  int i;

  setzcurs(e->curcol-e->xoffset+e->frame, e->readlines+e->frame, e);
  if (e->linbuf)
  {
    int zeile = e->y+e->readlines+e->frame;
    int block = (e->readindex>=e->blockbegin) && (e->readindex<e->blockend);
    int farbe = block ? color.edit_block : color.edit_text;
    int xpos = e->xoffset-e->x-e->frame;
    for (i = e->x+e->frame; i<e->xend; i++)
      wc(i, zeile, farbe, e->linbuf[xpos+i]);
  }
}

/*---------------------------------------------------------------------------*/

static int edupdate (int full)
//*************************************************************************
//
//  Uebertraegt den Puffer an der aktuellen Position in das Bildschirmfenster
//
//*************************************************************************
{
  int x, y, i=e->index;
  int attr, blank; // vorbesetzen fuer hoehere Geschwindigkeit
  int yline;
  int a;
  int pos;
  int xend = e->xend;
  unsigned maxbuf = e->maxbuf;
  unsigned col;

  if (video_off)
    return OK;
  if (iskeytask())
    col = color.activ_frame;
  else
    col = color.back_frame;
  if (full)
    wframe(col, e->title, 0);
  putpos(); // Position anzeigen
  if (taskkbhit())
    return 0; // nur weitermachen wenn keine Taste gedrueckt
  for (y = e->y+e->frame; y <= (e->yend-e->frame); y++)
  {
    x = e->xoffset;
    yline = (bildspalten*y);
    if ((i >= e->blockbegin) && (i < e->blockend))
      attr = color.edit_block << 8;
    else
      attr = color.edit_text << 8;
    blank = attr | ' ';
    while (x && (i < maxbuf) && (buf[i] != LF))
    {
      i++;
      x--;
    }
    x = e->x+e->frame;
    while (((a = buf[i]) != LF) && (i < maxbuf))
    {
      if (x < xend)
      {
        if (vorne[pos = yline+(x++)] == wa)
          vram[pos] = a | attr;
      }
      i++;
    }
    i++;
    while (x < xend)
    {
      if (vorne[pos = yline+(x++)] == wa)
        vram[pos] = blank;
    }
  }
  lineout();
  return OK;
}

/*---------------------------------------------------------------------------*/

static int near linelength (int index)
//*************************************************************************
//
//  Bestimmt die Laenge der mit 'index' referierten Zeile ( 1 == nur LF )
//
//*************************************************************************
{
  int indexalt = index;
  while (buf[index++] != LF);
  return index-indexalt;
}

/*---------------------------------------------------------------------------*/

static void near testxoffset (void)
//*************************************************************************
//
//  Prueft Cursorposition und tatsaechliches Bildschirmfenster auf
//  Uebereinstimmung und veranlasst ggf. horizontales Scrolling
//
//*************************************************************************
{
  while (e->curcol >= (e->xoffset + e->xend- e->x - 1))
    e->xoffset++;
  while (e->curcol < (e->xoffset + 5*(e->curcol > 4)))
    e->xoffset--;
}

/*---------------------------------------------------------------------------*/

static void near crechts (void)
//*************************************************************************
//
//  Positioniert den Cursor um 1 nach rechts
//
//*************************************************************************
{
  if(e->curcol < (LINLEN-1))
  {
    e->curcol++;
    testxoffset();
  }
}

/*---------------------------------------------------------------------------*/

static void near calcrlines (void)
//*************************************************************************
//
//  Stellt vertikale Editorpositon des Cursors im Buffer fest
//
//*************************************************************************
{
  int r = e->readlines;

  e->readindex = e->index;
  if (e->index < e->maxbuf)
  {
    while (r)
    {
      while (buf[e->readindex++] != LF);
      r--;
    }
  }
}

/*---------------------------------------------------------------------------*/

static void near lineplus (int zeilenanzahl)
//*************************************************************************
//
//  Scrollt um eine Anzahl Zeilen nach oben
//
//*************************************************************************
{
  int i = zeilenanzahl;
  while (i)
  {
    if (e->curline<e->maxlines)
    {
      if (e->index < e->maxbuf) e->curline++;
      while ((e->index < e->maxbuf) && (buf[e->index++] != LF));
    }
    else
      zeilenanzahl--;
    i--;
  }
  if (zeilenanzahl != 1)
    calcrlines();
  else
    while ((e->readindex < e->maxbuf) && buf[e->readindex++] != LF);
}

/*---------------------------------------------------------------------------*/

static void near lineminus (int zeilenanzahl)
//*************************************************************************
//
//  Scrollt um eine Anzahl Zeilen nach unten
//
//*************************************************************************
{
  int i = zeilenanzahl;
  while (i)
  {
    if (e->index)
    {
      e->curline--;
      e->index--;
    }
    while (e->index && (buf[e->index-1] != LF))
      e->index--;
    i--;
  }
  if (zeilenanzahl > 1)
    calcrlines();
  else
  {
    if (e->readindex) e->readindex--;
    while (e->readindex && (buf[e->readindex-1] != LF))
      e->readindex--;
  }
}

/*---------------------------------------------------------------------------*/

static int near horizontalswitch (int a)
//*************************************************************************
//
//  Verteilfunktion fuer alle horizontalen Cursorbewegungen
//
//*************************************************************************
{
  int i;
  switch (a)
  {
    case INSERT:
      break;
    case K_DELETE:
      for (i = e->curcol; i < LINLEN-1; i++)
        e->linbuf[i] = e->linbuf[i+1];
      e->linbuf[LINLEN-1] = ' ';
      e->modified++;
      break;
    case LINKS:
      if (e->curcol)
        e->curcol--;
      break;
    case RECHTS:
      crechts();
      break;
    case POS1:
      e->curcol = 0;
      break;
    case ENDE:
      for (i = LINLEN-1; i && (e->linbuf[i-1] == ' '); i--);
      e->curcol = i;
      break;
    default:
      return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static void near gotoline (int line, int immer)
//*************************************************************************
//
//  Springt zur angegebenen Zeilennummer
//
//  Dabei wird die momentane Cursorposition beruecksichtigt und der
//  Bildschirm nur neu aufgebaut, wenn die Position ausserhalb liegt.
//
//*************************************************************************
{ unsigned i = 0, ok = 1;
  unsigned max = e->maxbuf;
  unsigned zeilenzaehler = 0;
  unsigned yl = e->ylen-1;

  if (! immer)
  {
    if (line == e->curline);
      // **** Alte Zeile gleich neue Zeile -> nichts machen
    else if ((line > e->curline) && (line < (e->curline+(yl-1-e->readlines))))
      e->readlines += line-e->curline;
      // **** Neue Zeile unterhalb alter Zeile, aber am gleichen Bildschirm
    else if ((line < e->curline) && (line > (e->curline-e->readlines)))
      e->readlines -= e->curline-line;
      // **** Neue Zeile oberhalb alter Zeile, aber am gleichen Bildschirm
    else if (line < (yl-2))
    {
      e->index = 0;
      e->readlines = e->curline=line;
      // **** Neue Zeile auf der ersten Bildschirmseite im Buffer
    }
    else if ((line > (e->maxlines-yl-1)) && (line<e->maxlines))
    {
      e->readindex = e->index = e->maxbuf;
      e->curline = e->maxlines;
      lineminus(yl-2);
      e->readlines = (yl-2-(e->maxlines-line));
      // **** Neue Zeile auf der letzten Bildschirmseite im Buffer
    }
    else
      ok = 0;
  }
  if (immer || ! ok)
  { // **** Neue Zeile liegt ganz wo anders im Buffer, Zeile suchen
    while (i < max)
    {
      if (buf[i++] == LF)
      {
        zeilenzaehler++;
        if (zeilenzaehler == line)
          break;
      }
    }
    if (i >= max) // Zeilennummer nicht gefunden (zu hoch)
    {
      msgline("Line not found");
      line = zeilenzaehler; // go to last line
      while (i && buf[i] != LF) i--;
    }
    e->index = e->readindex = i; // zur gefundenen Stelle springen
    if (immer == 2)
    {
      e->readlines = yl-1;
    }
    else
    {
      e->curline = line+(yl/2); // BS-Start wird zurueckgesetzt
      lineminus(yl/2); // damit die Stelle in Bildmitte kommt
      e->readlines = yl/2; // Cursor in Bildmitte setzen
    }
  }
  e->curline=line; // Neue Zeile uebernehmen
  calcrlines(); // Zeiger reorganisieren
}

/*---------------------------------------------------------------------------*/

static void near contsearch (void)
//*************************************************************************
//
//  Sucht nach 'searchstr' und baut den Bildschirm an der gefundenen
//  Position neu auf
//
//*************************************************************************
{
  int count = 0;
  int slines = e->curline;
  unsigned sindex = e->readindex;
  int found = 0;
  int size = strlen(searchstr);
  unsigned i = linelength(e->readindex);

  if (e->curcol > i)
    i += e->readindex - 1;
  else
    i = e->readindex + e->curcol + 1;
  while (i < e->maxbuf)
  {
    while (((buf[i+count] ^ searchstr[count]) & 0x5f) == 0) count++;
    if (count == size)
    {
      found++;
      break;
    }
    count = 0;
    if (buf[i++] == LF)
    {
      slines++;
      sindex = i;
    }
  }
  if (found)
  {
    e->curcol = i-sindex;
    gotoline(slines, 0);
    testxoffset();
  }
  else
    msgline("Pattern not found");
}

/*---------------------------------------------------------------------------*/

static void near ctrlk (void)
//*************************************************************************
//
//*************************************************************************
{
  int a;

  setzcurs(0, 255, e);
  wf(e->x+2, e->y, color.activ_frame, "^K");
  a=waitkey() & 255;
  wf(e->x+2, e->y, color.activ_frame, "ŽŽ");
  switch (a)
  {
    case 'b':
         e->blockbegin = e->readindex;
         break;
    case 'k':
         e->blockend = e->readindex;
         break;
  }
}

/*---------------------------------------------------------------------------*/

static void near zuzeile (void)
//*************************************************************************
//
//*************************************************************************
{
  char linstr[12];
  int l;

  linstr[0] = 0;
  if (inputwin("Goto", "Line:", linstr, 10) != ESC)
    l = atoi(linstr);
  if (l > 0)
    gotoline(l-1, 0);
  else
    msgline("Invalid line number");
}

/*---------------------------------------------------------------------------*/

static void near suchen (void)
//*************************************************************************
//
//*************************************************************************
{
  if (inputwin("Search", "pattern:", searchstr, 40) != ESC)
    contsearch();
}

/*---------------------------------------------------------------------------*/

static void near ctrlq (void)
//*************************************************************************
//
//*************************************************************************
{
  int a;

  setzcurs(0, 255, e);
  wf(e->x+2, e->y, color.activ_frame, "^Q");
  a = waitkey() & 255;
  wf(e->x+2, e->y, color.activ_frame, "ŽŽ");
  switch (a)
  {
    case 'f':
    case 'a':
         suchen();
         break;
    case 'n':
         zuzeile();
         break;
  }
}

/*---------------------------------------------------------------------------*/

static int near edhorizontal (void)
//*************************************************************************
//
//  Zeileneditor, in dem alle horizontalen Cursorbewegungen und Eingaben
//  verarbeitet werden. Die Zeile wird komplett in einem Puffer bearbeitet
//
//  Vertikale Aktionen werden an den Aufrufer zurueckgegeben
//
//*************************************************************************
{
  int retwert = OK;
  int i = strlen(e->linbuf);
  unsigned a;

  memset(e->linbuf+i, ' ', LINLEN-i); // Rest der Zeile mit Blanks vorbesetzen
  e->linbuf[LINLEN] = 0;

  while (((a = (unsigned) waitkey()) & 255) != ESC)
  {
    retwert = a & 255;
    if ((bioskey(2) & 3) == 0)
      shiftgedrueckt = 0;
    if ((a & 255) == 0)
    {
      a >>= 8;
      retwert = (unsigned) a;
      if (horizontalswitch(a))
        goto ende;
    }
    else
    {
      a &= 255;
      if (a == CR) a = LF;
      if ((a < ' ') && (a != LF) && (a != LINEDEL) && (a != BACKSPACE))
        goto ende;
      e->modified++;
      if (a == LINEDEL) // CTRL-Y
      {
        for (i = 0; i < LINLEN; i++)
          e->linbuf[i] = ' ';
        e->curcol = 0;
        goto ende;
      }
      if (a == BACKSPACE)
      {
        if (e->curcol)
        {
          e->curcol--;
          for (i = e->curcol; i < LINLEN-1; i++)
            e->linbuf[i] = e->linbuf[i+1];
          e->linbuf[LINLEN-1] = ' ';
        }
        else
          goto ende;
      }
      else
      {
        if (e->curcol < LINLEN)
        {
          for (i = LINLEN-1; i > e->curcol; i--)
            e->linbuf[i] = e->linbuf[i-1];
          e->linbuf[e->curcol] = a;
          crechts();
          if ((e->curcol >= e->wordwrap) && (a != LF))
          {
            for (i = e->curcol-1; i && (e->linbuf[i] != ' '); i--);
            if (i == 0)
            {
              i = e->curcol-1;
              e->linbuf[e->curcol] = e->linbuf[i];
              e->curcol = 1;
            }
            else
              e->curcol -= (i+1);
            e->linbuf[i] = LF;
            e->xoffset = 0;
            e->maxlines++;
            calcrlines();
            retwert = LF;
            goto ende;
          }
        }
      }
      if (a == LF)
      {
        retwert = LF;
        e->curcol = 0;
        testxoffset();
        e->maxlines++;
        calcrlines();
        goto ende;
      }
    }
    testxoffset();
    lineout();
  }
  retwert = a & 255;
  ende:
  if (e->readlines > (e->ylen-1))
    gotoline(e->curline, 1);
  i = LINLEN-1; // von hinten her
  while ((i >= 0) && (e->linbuf[i] == ' ')) // Ende der Eingabe feststellen
    e->linbuf[i--] = 0;
  return retwert;
}

/*---------------------------------------------------------------------------*/

static int near changeline (int a, int alt, char *linbuf)
//*************************************************************************
//
//  Wird aufgerufen, wenn eine Zeile editiert worden ist
//  Die Aenderungen werden in den Buffer uebernommen
//
//*************************************************************************
{
  int i;
  int neu = strlen(linbuf);

  if (a == BACKSPACE)
  {
    if (e->index || e->readindex)
    {
      e->maxlines--;
      if (e->readindex)
        for (i = e->readindex-1; i && (buf[i-1] != LF); i--)
          e->curcol++;
      testxoffset();
      if(neu)
      {
        neu--;
        e->readindex--;
      }
      else
        alt++;
    }
    else
      a = RAUF;
  }
  if ((a == LINEDEL) && (e->curline != e->maxlines))
    alt++;
  e->maxbuf += (neu-alt);
  if ((alt != neu) && (((long)e->maxbuf-(long)e->readindex-(long)neu) > 0))
  {
    memmove(buf+e->readindex+neu, buf+e->readindex+alt,
                                  e->maxbuf-e->readindex-neu);
  }
  if (neu)
  {
    strcpy(buf + e->readindex, linbuf);
    buf[e->readindex + neu + (a == BACKSPACE)] = LF;
  }
  return a;
}

/*---------------------------------------------------------------------------*/

static int near vertikalswitch (unsigned a)
//*************************************************************************
//
//  Hier werden alle vertikalen Cursorsteuerungen bearbeitet
//
//*************************************************************************
{
  int bslines = e->yend - e->y - 2*e->frame;
  int retwert = 0;

  if (bioskey(2) & 3)
  {
    if (! shiftgedrueckt)
    {
      e->blockmark = e->blockbegin = e->blockend = e->readindex;
      shiftgedrueckt++;
    }
  }
  else
    shiftgedrueckt = 0;
  switch(a)
  {
    case LINEDEL:
           calcrlines();
           if (e->curline != e->maxlines)
             e->maxlines--;
         break;
    case CTRLF:
           suchen();
           break;
    case CTRLN:
           zuzeile();
           break;
    case CTRLQ:
           ctrlq();
           break;
    case CTRLL:
           contsearch();
           break;
    case CTRLK:
           ctrlk();
           break;
    case BACKSPACE:
           if (e->readlines)
           {
             e->readlines--;
             e->curline--;
           }
           else
             lineminus(1);
           calcrlines();
         break;
    case RAUF:
           if (e->readlines)
           {
             e->readlines--;
             e->curline--;
             if (e->readindex) e->readindex--;
             while (e->readindex && (buf[e->readindex-1] != LF))
               e->readindex--;
           }
           else
             lineminus(1);
         break;
    case LF:
    case RUNTER:
           if ((e->readindex < e->maxbuf) && (e->curline < e->maxlines))
           {
             if (e->readlines < bslines)
             {
               e->readlines++;
               e->curline++;
               while ((e->readindex < e->maxbuf) && buf[e->readindex++] != LF);
             }
             else
               lineplus(1);
           }
         break;
    case PG_RAUF:
           if (e->index)
             lineminus(bslines);
           else
           {
             e->readlines = 0;
             e->curline = 0;
             calcrlines();
           }
         break;
    case PG_RUNTER:
           if (e->readlines < bslines)
           {
             while (e->readlines < bslines)
             {
               if (e->readlines < e->maxlines)
               {
                 e->readlines++;
                 e->curline++;
               }
               else
                 bslines = 0;
             }
             calcrlines();
           }
           else
           {
             if (e->readindex < e->maxbuf)
               lineplus(bslines);
           }
         break;
    case CTL_PG_RAUF:
           gotoline(0, 0);
         break;
    case CTL_PG_RUNTER:
           e->index = e->maxbuf;
           while (buf[--e->index] != LF);
             e->readlines = bslines;
           while (e->readlines > e->maxlines)
             e->readlines--;
           lineminus(bslines);
           e->curline = e->maxlines;
         break;
    default: retwert = a;
  }

  if (shiftgedrueckt)
  {
    if (e->blockmark >= e->readindex)
      e->blockbegin = e->readindex;
    if (e->blockmark <= e->readindex)
      e->blockend = e->readindex;
  }
  return retwert;
}

/*---------------------------------------------------------------------------*/

static int near edvertikal (void)
//*************************************************************************
//
//  Diese Routine wird zum Bearbeiten einer Zeile aufgerufen und
//  koordiniert alle Cursorsteuerungen und Modifikationen
//
//*************************************************************************
{
  int a;
  int i = 0;
  char linbuf[LINLEN+1];
  int alt;
  int mod = e->modified;

  while((i < LINLEN) && ((linbuf[i] = buf[e->readindex+i]) != LF)
        && ((e->readindex+i) < e->maxbuf)) i++;
  linbuf[i] = 0;
  alt = strlen(linbuf);
  e->linbuf = linbuf;
  a = edhorizontal(); // Zeileneditor
  e->linbuf = NULL;
  if (a == LF && e->linefunc)
  {
    char *nl = strchr(linbuf, LF);
    if (nl && strlen(nl) > 1)
      memmove(nl, nl+1, strlen(nl));
    if (linbuf[strlen(linbuf)-1] == LF)
      linbuf[strlen(linbuf)-1] = 0;
    changeline(0, alt, "");
    (*(e->linefunc)) (linbuf);
    calcrlines(); // Zeiger reorganisieren
  }
  else
  {
    if (mod != e->modified) // geaendert?
      a = changeline(a, alt, linbuf); // ja, Aenderungen uebernehmen
  }
  a = vertikalswitch(a); // Vertikale Cursorsteuerung

  edupdate(0); // ggf. Bildschirm neu schreiben
  return a;
}

/*---------------------------------------------------------------------------*/

/*
static void putvo_edit (char c)
//*************************************************************************
//
//*************************************************************************
{
  char s[4];
  s[0] = c;
  s[1] = LF;
  s[2] = 0;
  changeline(0, 1, s);
  e->readindex++;
  if(c == LF)
  {
    e->maxlines++;
    e->curline++;
  }
}
*/

/*---------------------------------------------------------------------------*/

static void near filespeichern (void)
//*************************************************************************
//
//*************************************************************************
{
  FILE *ef = s_fopen(e->title, "swt");

  if(ef)
  {
    setvbuf(ef, NULL, _IOFBF, 8192);
    fwrite(buf, e->maxbuf, 1, ef);
    s_fclose(ef);
  }
}

/*---------------------------------------------------------------------------*/

void editwin (int file)
//*************************************************************************
//
//  Eroeffnet ein Fenster zum Editieren und ruft zyklisch den Editor auf
//
//*************************************************************************
{
  int a;
  int schlussmachen = 0;

  e->update = edupdate;
  e->wordwrap = 80;
  if (file)
    ;//gotoline(1, 1);
  else
    gotoline(e->curline, 2);
  edupdate(1);
  while(! schlussmachen)
  {
    e->shouldupdate++;
    a = edvertikal();
    if (a == LF)
    {
      sound(2000);
      delay(50);
      nosound();
    }
    if (a == ESC)
      schlussmachen = 1;
    if (schlussmachen && e->modified && file)
    {
      switch (yesno("Save changes?", 1))
      {
        case ESC:
          schlussmachen = 0;
          break;
        case YES:
          filespeichern();
          break;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void editfile (char *name)
//*************************************************************************
//
//*************************************************************************
{
  FILE *ef = s_fopen(name, "srt");
  int a;
  int pos = 0;
  int lines = 0;

  strupr(name);
  strncpy(e->title, name, 39);
  e->title[39] = 0;
  e->buf = (char *) t_malloc(EDLEN, "edbu");
  buf = e->buf;
  e->x = 3;
  e->xend = 70;
  e->y = 3;
  e->yend = bildzeilen-5;
  e->update = edupdate;
  wopen();
  setkeyhere();
  edupdate(1);
  if (buf)
  {
    if (ef)
    {
      setvbuf(ef, NULL, _IOFBF, 8192);
      while ((a = fgetc(ef)) != EOF)
      {
        if (a == LF)
        {
          pos = 0;
          lines++;
          waitfor(e_ticsfull);
        }
        if (pos < (LINLEN-8))
        {
          if (a == TAB)
          {
            do
            {
              buf[e->index++] = ' ';
              pos++;
            } while ((pos%8) != 0);
          }
          else
          {
            buf[e->index++] = a;
            pos++;
          }
        }
        if (e->index > (EDLEN-100)) break;
      }
      s_fclose(ef);
      if (buf[e->index-1] != LF)
      {
        buf[e->index++] = LF;
        lines++;
      }
      e->maxbuf = e->index;
      e->maxlines = lines;
      e->index = 0;
      e->curline = 1;
      editwin(1);
    }
    else
    {
      if (yesno("Create file?", 1) == YES)
        editwin(1);
    }
  }
  else
    msgline("Out of memory");
  wclose();
  t_free(buf);
  buf=NULL;
}

/*---------------------------------------------------------------------------*/

#if 0
void qsowin (int (*linefunc)(char *), unsigned buflen)
//*************************************************************************
//
//*************************************************************************
{
  e->buf = (char *) t_malloc(buflen, "qsob");
  buf = e->buf;

/*  e->x=5;
  e->xend=60;
  e->y=10;
  e->yend=bildzeilen-5;
  e->wordwrap=50;
*/
  e->linefunc = linefunc;
  wopen();
  setkeyhere();
  edupdate(1);
  if(buf)
    editwin(0);
  else
    msgline("Out of memory");
  wclose();
  t_free(buf);
  buf=NULL;
}
#endif

/*---------------------------------------------------------------------------*/

#endif
