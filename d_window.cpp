/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  Fensterverwaltung
  -----------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW Frame was recoded to some strange characters, fixed
//19980124 OE3DZW deto at sizeframe(), fixed

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int boxopen = 0;
static edstruct *efeld[MAXWND];
static int lastwnd = 0;
static int frontebene = 0;

/*---------------------------------------------------------------------------*/

void setzcurs (int x, int y, edstruct *e)
//*************************************************************************
//
//  Hardwarecursor positionieren
//
//*************************************************************************
{
  int index = (e->y+y)*bildspalten+(e->x+x);
  unsigned xx = e->x+x;
  unsigned yy = e->y+y;
  e->curx = x;
  e->cury = y;


  if (iskeytask() || gettaskid()==0)
  {
    if (xx < bildspalten && yy < bildspalten && vorne[index] == e->wa)
      video_setcursor(xx, yy);
    else
      video_setcursor(255, 255);
  }
}

/*---------------------------------------------------------------------------*/

void wf (int x, int y, int attr, char *format,...)
{
  va_list argpoint;
  char cbuf[80];

  va_start(argpoint, format);
  vsprintf(cbuf, format, argpoint);
  va_end(argpoint);
  ws(x, y, attr, cbuf);
}

/*---------------------------------------------------------------------------*/

void wstr (int line, int col, char *s)
{
  int x = e->x+1;
  int xmax = e->xlen+e->x-1;

  if (*s == CR)
  {
    e->wpos = x;
    return;
  }
  if (e->ylen == 1)
  {
    x = e->wpos;
    line = e->y;
  }
  else
    line += e->y;
  wc(x++, line, col, ' ');
  while (*s)
    wc(x++, line, col, *(s++));
  if (e->ylen == 1)
  {
    wc(x++, line, col, ' ');
    e->wpos = x;
  }
  else
  { while (x < xmax)
      wc(x++, line, col, ' ');
  }
}

/*---------------------------------------------------------------------------*/

void wframe (int col, char *headerln, int doppelstrich)
//OE3DZW  0xb3,0xc4,0xc0,0xd9,0xda,0xbf,0xba,0xcd,0xc8,0xbc,0xc9,0xbb
{
  static char *fr[] = {"³ÄÀÙÚ¿", "ºÍÈ¼É»", "|-++++", "|-++++"};
  int i;
  char *f = fr[doppelstrich];
  char title[40];
  int x = e->x;
  int y = e->y;
  int xlen = e->xlen-1;
  int ylen = e->ylen-1;

  if (e->frame == 0) return;
  mausaus();
  i = strlen(headerln);
  if (i)
  {
    strcpy(title+1, headerln);
    title[0] = title[i+1] = ' ';
    title[i+2] = 0;
  }
  else
    title[0] = 0;

  wc(x, y, col, f[4]);
  wc(x+xlen, y, col, f[5]);
  wc(x, y+ylen, col, f[2]);
  wc(x+xlen, y+ylen, col, f[3]);
  for (i = x+1; i < (x+xlen); i++)
  {
    wc(i, y, col, f[1]);
    wc(i, y+ylen, col, f[1]);
  }
  for (i = y+1; i < (y+ylen); i++)
  {
    wc(x, i, col, f[0]);
    wc(x+xlen, i, col, f[0]);
  }
  ws(1+x+(xlen-strlen(title))/2, y, col, title);
  if (! e->klein)
  {
    if(wa > 9)
      wc(x+xlen-3, y, col, wa/10+'0');
    wc(x+xlen-2, y, col, wa%10+'0');
  }
}

/*---------------------------------------------------------------------------*/

void wscrollup (int col)
{
  int i, j, k;
  int xmax;
  int x = e->x+e->frame;
  int ymax = e->ylen-1+e->y-e->frame;
  int line;
  video blank = ' '|(col<<8);

  mausaus();
  e->wpos = 0;
  for (j = e->y+e->frame+1; j <= ymax; j++)
  {
    line = bildspalten*j;
    xmax = e->xlen-1+e->x+line-e->frame;
    k = x+line-bildspalten;
    for (i = x+line; i <= xmax; i++, k++)
    {
      if (vorne[k] == wa)
      {
        if (vorne[i] == wa)
          vram[k] = vram[i];
        else
        {
          vram[k] = blank;
          e->shouldupdate++;
        }
      }
    }
  }
  for (i = x+line; i <= xmax; i++)
  {
    if (vorne[i] == wa)
      vram[i] = blank;
  }
  mausein();
}

/*---------------------------------------------------------------------------*/

void wopen (void)
{
  int i = 0;

  while (boxopen)
    wdelay(100);
  while (efeld[i] != NULL)
  {
    if (i >= MAXWND)
    {
      trace(tr_abbruch, "wopen", "windows full");
      return;
    }
    else
      i++;
  }
  efeld[i] = e;
  e->wa = wa = i;
  e->xlen = e->xend-e->x+1;
  e->ylen = e->yend-e->y+1;
  e->save = NULL;
  e->klein = 0;
  frontebene = e->ebene=wa;
  e->cury = (char)255;
  for (i = e->y; i < e->ylen+e->y; i++)
    memset(vorne+e->x+bildspalten*i, wa, e->xlen);
  lastwnd++;
}

/*---------------------------------------------------------------------------*/

void boxwopen (int x, int y, int xlen, int ylen)
{
  video far *ptr;
  int i = 0, j, jj, z = 0;

  if (! boxopen && allupdate)
    wdelay(101);
  boxopen++;
  while (efeld[i] != NULL)
  {
    if(i >= MAXWND)
    {
      trace(tr_abbruch, "boxwopen", "windows full");
      return;
    }
    else
      i++;
  }
  efeld[i] = (edstruct *)t_malloc(sizeof(edstruct), "wins");
  memset(efeld[i], 0, sizeof(edstruct));
  efeld[i]->esave = e;
  e = efeld[i];
  e->wa = wa = i;
  e->save = NULL;
  e->klein = 1;
  e->frame = 1;
  e->ebene = wa;
  e->x = x;
  e->y = y;
  e->xlen = xlen;
  e->ylen = ylen;
  xlen += x;
  ylen += y;
  ptr = (video far *)t_malloc((xlen+1-x)*(ylen+1-y)*2, "popu");
  if (ptr)
  {
    e->save = ptr;
    for (j = y; j < ylen; j++)
    {
      jj = bildspalten*j;
      for (i = x; i < xlen; i++)
        ptr[z++] = vram[i+jj];
    }
  }
  for (j = y; j < ylen; j++)
    memset(vorne+x+bildspalten*j, wa, e->xlen);
  lastwnd++;
}

/*---------------------------------------------------------------------------*/

void vorne_reorg (void)
{
  int i, ebene;
  edstruct *e;
  int x;
  int xlen;
  int ymax;
  int yy;

  if (! boxopen)
  {
    memset(vorne, 0, bildzeilen*bildspalten);
    for (ebene = 0; ebene < lastwnd; ebene++)
    {
      for (i = 0; i < lastwnd; i++)
      {
        e = efeld[i];
        if(e && (e->ebene == ebene))
        {
          x = e->x;
          xlen = e->xlen;
          ymax = e->ylen+e->y;
          for (yy = e->y; yy < ymax; yy++)
            memset(vorne+x+bildspalten*yy, i, xlen);
          frontebene = ebene;
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void nachvorne (int new_win)
{
  int i;

  if (! boxopen && (new_win < lastwnd)
      && (efeld[new_win]->ebene != (lastwnd-1)))
  {
    for (i = 0; i < lastwnd; i++)
    {
      if (efeld[i] && (efeld[i]->ebene>efeld[new_win]->ebene))
      {
        efeld[i]->ebene--;
      }
    }
    efeld[new_win]->ebene = (lastwnd-1);
    allupdate++;
  }
}

/*---------------------------------------------------------------------------*/

void wclose (void)
{
  int key = (e->wa == keywindow());
  e->wa = 0;
  efeld[wa] = NULL;
  while (lastwnd && (efeld[lastwnd-1] == NULL))
    lastwnd--;
  if (key)
  {
    int i, newkey = 0;
    frontebene = 0;
    for (i = 0; i < lastwnd; i++)
    {
      if (efeld[i] && efeld[i]->ebene>=frontebene)
      {
        frontebene = efeld[i]->ebene;
        newkey = i;
      }
    }
    setkeywindow(newkey);
  }
  allupdate++;
}

/*---------------------------------------------------------------------------*/

void boxwclose (void)
{
  video *save;
  int i, j, jj, z = 0;
  int x = e->x;
  int xlen = e->xlen+x;
  int y = e->y;
  int ylen = e->ylen+y;
  save = e->save;
  for (j = y; j < ylen; j++)
  {
    jj = bildspalten*j;
    for (i = x; i < xlen; i++)
      vram[i+jj] = save[z++];
  }
  t_free(save);
  i = wa;
  e = (edstruct *)e->esave;
  wa = e->wa;
  t_free(efeld[i]);
  efeld[i] = 0;
  while (lastwnd && (efeld[lastwnd-1] == NULL))
    lastwnd--;
  if (boxopen)
    boxopen--;
  else
    trace(fatal, "boxwclose", "no box open");
  allupdate++;
}

/*---------------------------------------------------------------------------*/

static video near fwc (int x, int y, int attr, char c)
{
  int index = x+bildspalten*y;
  video alt = vram[index];
  vram[index] = c|(attr<<8);
  vorne[index] = (char)255;
  return alt;
}

/*---------------------------------------------------------------------------*/

static video *near size_frame (edstruct *e)
{
  static char f[] = "³ÄÀÙÚ¿"; //OE3DZW do not touch!
  int i, z = 0;
  int x = e->x;
  int y = e->y;
  int xlen = e->xend-e->x;
  int ylen = e->yend-e->y;
  video *buf = (video *)t_malloc((bildzeilen+bildspalten+2)*4, "size");
  int col = 0x2e;
  buf[z++] = fwc(x, y, col, f[4]);
  buf[z++] = fwc(x+xlen, y, col, f[5]);
  buf[z++] = fwc(x, y+ylen, col, f[2]);
  buf[z++] = fwc(x+xlen, y+ylen, col, f[3]);
  for (i = x+1; i < (x+xlen); i++)
  {
    buf[z++] = fwc(i, y, col, f[1]);
    buf[z++] = fwc(i, y+ylen, col, f[1]);
  }
  for (i = y+1; i < (y+ylen); i++)
  {
    buf[z++] = fwc(x, i, col, f[0]);
    buf[z++] = fwc(x+xlen, i, col, f[0]);
  }
  return buf;
}

/*---------------------------------------------------------------------------*/

static void near size_frame_weg (edstruct *e, video *buf)
{
  int i, z = 0;
  int x = e->x;
  int y = e->y;
  int xlen = e->xend-e->x;
  int ylen = e->yend-e->y;

  vram[x+bildspalten*y] = buf[z++];
  vram[x+xlen+bildspalten*y] = buf[z++];
  vram[x+bildspalten*(y+ylen)] = buf[z++];
  vram[x+xlen+bildspalten*(y+ylen)] = buf[z++];
  for (i = x+1; i < (x+xlen); i++)
  {
    vram[i+bildspalten*y] = buf[z++];
    vram[i+bildspalten*(y+ylen)] = buf[z++];
  }
  for (i = y+1; i < (y+ylen); i++)
  {
    vram[x+bildspalten*i] = buf[z++];
    vram[x+xlen+bildspalten*i] = buf[z++];
  }
  t_free(buf);
}

/*---------------------------------------------------------------------------*/

static void near swap (char *a, char *b)
{
  char tmp = *a;
  *a = *b;
  *b = tmp;
}

/*---------------------------------------------------------------------------*/

#ifdef FEATURE_MOUSE
static void near maus_resize (edstruct *e)
{
  video *buf;
  int move;
  int x, x0, xx, yy, y, y0;

  x = mausspalte();
  y = mauszeile();
  x0 = x-e->x;
  y0 = y-e->y;
  if ((x == e->xend) && (y == e->yend))
    move = 0;
  else if (x == e->xend)
    move = 1;
  else if (y == e->yend)
    move = 2;
  else if ((x == e->x) && (y == e->y))
    move = 3;
  else if (x == e->x)
    move = 4;
  else if (y == e->y)
    move = 5;
  else return;
  while (mausknopf())
  {
    mausaus();
    buf = size_frame(e);
    mausein();
    boxopen++;
    while ((mauszeile() == y) && (mausspalte() == x) && mausknopf())
      wdelay(52);
    boxopen--;
    x = mausspalte();
    y = mauszeile();
    mausaus();
    size_frame_weg(e, buf);
    mausein();
    switch (move)
    {
      case 0:
        if (y > (e->y+3))
          e->yend = y;
        else
          e->yend = e->y+3;
      case 1:
        if (x > (e->x+8))
          e->xend = x;
        else
          e->xend = e->x+8;
        break;
      case 2:
        if (y > (e->y+3))
          e->yend = y;
        else
          e->yend = e->y+3;
        break;
      case 3:
        if (y < (e->yend-3))
          e->y = y;
        else
          e->y = e->yend-3;
      case 4:
        if (x < (e->xend-8))
          e->x = x;
        else
          e->x = e->xend-8;
        break;
      case 5:
        yy = y-y0;
        xx = x-x0;
        if (yy < 0) yy = 0;
        if (xx < 0) xx = 0;
        if (yy+e->ylen > (int)bildzeilen) yy = bildzeilen-e->ylen;
        if (xx+e->xlen > (int)bildspalten) xx = bildspalten-e->xlen;
        e->y = yy;
        e->x = xx;
        e->yend = yy+e->ylen-1;
        e->xend = xx+e->xlen-1;
        break;
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/

static void near key_resize (edstruct *e)
{
  video *buf;
  unsigned key, shift;

  do
  {
    buf = size_frame(e);
    key = waitkey();
    size_frame_weg(e, buf);
    shift = bioskey(2) & 3;
    switch (key >> 8)
    {
      case RUNTER:
        if (e->yend < ((int)bildzeilen-1))
        {
          if (shift)
            e->yend++;
          else
          {
            e->y++;
            e->yend++;
          }
        }
        break;
      case RAUF:
        if (shift)
        {
          if (e->yend > (e->y+2))
            e->yend--;
        }
        else
        {
          if (e->y)
          {
            e->y--;
            e->yend--;
          }
        }
        break;
      case LINKS:
        if (shift)
        {
          if (e->xend > (e->x+2))
            e->xend--;
        }
        else
        {
          if (e->x)
          {
            e->x--;
            e->xend--;
          }
        }
        break;
      case RECHTS:
        if (e->xend < ((int)bildspalten-1))
        {
          if (shift)
            e->xend++;
          else
          {
            e->x++;
            e->xend++;
          }
        }
        break;
    }
  }
  while (((key&255)!=ESC) && ((key&255)!=CR));
}

/*---------------------------------------------------------------------------*/

void resize(int winnum,int mode)
{
  edstruct *e = efeld[winnum];
  switch(mode)
  {
    case 0:
      if (! e->xend2 || ! e->yend2)
      {
        e->x2 = 0;
        e->y2 = 0;
        e->xend2 = bildspalten-1;
        e->yend2 = bildzeilen-1;
      }
      swap(&e->x2, &e->x);
      swap(&e->y2, &e->y);
      swap(&e->xend2, &e->xend);
      swap(&e->yend2, &e->yend);
      break;
    case 1:
      key_resize(e);
      break;
#ifdef FEATURE_MOUSE
    case 2:
      maus_resize(e);
      break;
#endif
  }
  e->xlen = e->xend-e->x+1;
  e->ylen = e->yend-e->y+1;
  allupdate++;
  newkeywin = winnum;
  setzcurs(e->curx, e->cury, e);
}
