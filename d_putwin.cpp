/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------
  DOS: Ausgabe-Fenster
  --------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

// Variablen
//
// e->index    Zeiger auf die Stelle, in die das naechste Zeichen in den
//           Puffer geschrieben wird.
//
// e->maxbuf   Zeiger auf das Ende des Seitenpuffers. An diesem Ende
//           wird hart umgebrochen, d.h. bei index=0 weitergemacht
//
// e->modified ist verschieden von null, wenn auf der Bildschirmseite
//           mindestens 1 Zeichen steht. e->index ist naemlich sowohl dann
//           null, wenn die Seite leer ist, als auch wenn die Seite neu
//           beginnt weil ein wrap-around erfolgt ist.
//
//
// Anordnung des Puffers:
//
// Der Puffer sollte stets mindestens die Groesse des maximalen Bildschirm-
// inhaltes haben. Er kann aber auch beliebig groesser sein, max 64k.
//
// Neue Zeichen werden stets nach   buf[e->index]  geschrieben. Eine Zeile
// wird durch '\n' beendet

/*---------------------------------------------------------------------------*/

int putupdate(int full)
//*************************************************************************
//
//  uebertraegt den Puffer an der aktuellen Position in das Bildschirmfenster
//
//*************************************************************************
{
  int a,x;
  int y,i=e->index;
  int attr=color.put_text<<8;
  int cattr=color.put_ctrl<<8;
  int blank=attr|' '; // vorbesetzen fuer hoehere Geschwindigkeit
  int lines=0,wlines=e->ylen-2*e->frame;
  int yline;
  int index=e->index;
  int pos;
  int xend=e->xend;
  unsigned maxbuf=e->maxbuf;
  int firstline=0;
  unsigned col;

  if (video_off)
    return OK;
  if (iskeytask())
    col=color.activ_frame;
  else
    col=color.back_frame;
  if (full)
    wframe(col, e->title, 0);
  //
  // Startposition suchen (ab momentaner Cursorposition zurueckgehen)
  //
  while (i && lines < wlines)
  {
    if (buf[i-1] == LF)
    {
      firstline = i;
      lines++;
    }
    i--;
  }
  if ((i == 0) && e->modified)
  {
    i=e->maxbuf;
    while ((i > index) && lines < wlines)
    {
      if (buf[i-1] == LF)
      {
        if (i == maxbuf)
          firstline = 0;
        else
          firstline = i;
        lines++;
      }
      i--;
    }
  }
  e->curline = lines;
  i = firstline;
  for (y=e->y+e->frame; y<e->yend; y++)
  {
    yline = (bildspalten * y);
    x = e->x + e->frame;
    while (((a=buf[i])!=LF) && (i!=index))
    {
      if (a < 32)
      {
        if(a == TAB)
        {
          int j = (x-e->x-1) % 8;
          while (j < 8)
          {
            if (x < xend)
            {
              if (vorne[pos = yline+(x++)] == wa)
                vram[pos] = blank;
            }
            j++;
          }
        }
        else
        {
          if (x < xend)
          {
            if (vorne[pos = yline+(x++)] == wa)
              vram[pos] = a | cattr;
          }
        }
      }
      else
      {
        if (x < xend)
        {
          if (vorne[pos = yline+(x++)] == wa)
            vram[pos] = a | attr;
        }
      }
      if ((++i) == maxbuf) i = 0;
    }
    while (x < xend)
    {
      if (vorne[pos = yline+(x++)] == wa)
        vram[pos] = blank;
    }
    if (i != index)
    {
      if ((++i) == maxbuf)
        i = 0;
    }
  }
  if (e->linbuf)
  {
    wf(e->x+e->curcol, e->y+e->curline, color.put_text, "%*s",
       e->xlen-e->curcol-(2*e->frame), e->linbuf);
  }
  setzcurs(e->curx, e->cury, e);
  return OK;
}

/*---------------------------------------------------------------------------*/

void putflush_win(void)
{
  setzcurs(e->curcol, e->curline, e);
  if (m.scrolldelay)
    putupdate(0);
}

/*---------------------------------------------------------------------------*/

void putvo_win(int c)
{
  if (c != 7)
  {
    buf[e->index++]=c;
    if (e->index==e->maxbuf)
    {
      e->index=0;
      e->modified++;
    }
    if (c == LF)
    {
      if (e->curline==e->ylen-(e->frame<<1))
      {
        if (m.scrolldelay)
          e->shouldupdate++;
        else
          wscrollup(color.put_text);
      }
      else
        e->curline++;
      waitfor(e_ticsfull);
      e->curcol=e->frame;
    }
    else
    {
      if (e->curcol<=(e->xlen-(e->frame<<1)))
      {
        if (m.scrolldelay)
          e->shouldupdate++;
        else
          wc(e->x+e->curcol, e->y+e->curline, color.put_text, c);
      }
      e->curcol++;
    }
  }
  else
  {
    sound(1600);
    wdelay(138);
    nosound();
  }
}

/*---------------------------------------------------------------------------*/

void inputline_win(char *s, int maxlen, char cut)
{
  lastfunc("inputline_win");
  putflush_win();
  s[0]=0;
  if (maxlen < 0)
    maxlen = (-maxlen);
  if (! cut)
    maxlen--;
  editline(e->curcol, e->curline, maxlen, s);
  putf("%s\n", s);
  putupdate(1);
  if (! cut)
    strcat(s, "\n");
}

/*---------------------------------------------------------------------------*/

void preparewin(int (*update)(int),char *name,unsigned buflen)
{
  if (! e->xend || ! e->yend)
    trace(tr_abbruch, "preparewin", "win size");
  e->update = update;
  if (strlen(name) < 16)
    strcpy(e->title,name);
  else
    trace(tr_abbruch, "preparewin", "strlen name");
  e->maxbuf = buflen;
  if (buflen)
  {
    e->buf = (char *) t_malloc(e->maxbuf, "winb");
    buf = e->buf;
  }
  wopen();
  setkeyhere();
  (*update)(1);
}

/*---------------------------------------------------------------------------*/
/*
void editwin(int);

static void putwin_edit(void)
{
  int i;
  int maxbuf=e->maxbuf;

  if (e->modified)
  {
    i=e->index;
    char *newbuf=(char *) t_malloc(e->maxbuf+1000, "newb");
    while (i<e->maxbuf && buf[i]!=LF)
      i++;
    memcpy(newbuf, buf+i, e->maxbuf-i);
    memcpy(newbuf+e->maxbuf-i, buf, e->index);
    e->index+=(e->maxbuf-i);
    t_free(buf);
    buf=newbuf;
    e->buf=newbuf;
  }
  e->maxbuf=e->index;
  e->maxlines=0;
  for (i=0; i<e->maxbuf; i++)
  {
    if (buf[i]==LF)
    {
      e->maxlines++;
      e->index=i;
    }
  }
  e->readlines=1;
  e->curline=e->maxlines-1;
  e->curcol--;
  editwin(0);
  e->curcol++;
  e->curline=e->readlines;
  e->index=e->maxbuf;
  e->update=putupdate;
  e->modified=0;
  e->maxbuf=maxbuf;
}
*/
