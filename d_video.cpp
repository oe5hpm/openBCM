/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------
  DOS-Video-Routinen
  ------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

video far *vram=NULL;
unsigned short vseg=0xb800;
unsigned bildzeilen=25;
unsigned bildspalten=80;
unsigned monochrome=0;
int      novideo=0;
int      cgadisp=0;
int      monodisp=0;
char     *vorne;       // fuer jeden Bildpunkt, welches Fenster sichtbar ist
int      wa = 0;       // aktuell aktives Fenster
int      video_off=0;
time_t   video_timer=0;

static int vramcolor=1;
static unsigned doscursor;
static unsigned dossize;
#ifdef __HWSCREEN__
static struct text_info dosbildschirm;
#endif

static int vgabildaktiv=0;

/*---------------------------------------------------------------------------*/

static void near settextinfo (struct text_info *mode)
//*************************************************************************
//
//  Momentanen Textmodus setzen
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  textattr(mode->attribute);
  textmode(mode->currmode);
  gotoxy(mode->curx,mode->cury);
#endif
}

/*---------------------------------------------------------------------------*/

static void near egalines (void)
//*************************************************************************
//
//  Umschalten auf 43/50 Zeilenmodus bei EGA/VGA
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  _AX=3;
  geninterrupt(0x10);
  _BX=0;
  _AL=18;
  _AH=17;
  geninterrupt(0x10);
  _BX=32;
  _AL=0;
  _AH=32;
  geninterrupt(0x10);
#endif
}

/*---------------------------------------------------------------------------*/

static int near setmaxlines (void)
//*************************************************************************
//
//  Setzt den Bildschirm auf einen 8x8-Zeichensatz
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  union REGS rg;
  if (ad_peekb(0x40, 0x84) == 24)
  {
    rg.x.ax = 0x1200;             // Test, ob EGA vorhanden
    rg.x.bx = 0xff10;
    ad_int(0x10, &rg, &rg);       // wenn EGA, bh=0-1 und bl=0-3

    if (! (rg.x.bx & 0xfefc))
      egalines();
  }
  return ad_peekb(0x40, 0x84)+1;
#else
  return 25;
#endif
}

/*---------------------------------------------------------------------------*/

void video_close (void)
//*************************************************************************
//
//  DOS-Bildschirm wiederherstellen
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  video *dossave;
  int   ok = 0;
  handle fh;

  if (vgabildaktiv && ! novideo)
  {
    settextinfo(&dosbildschirm);
    dossave = (video *) t_malloc(dossize, "dosv");
    if (dossave)
    {
      fh = _open(VIDNAME, O_RDONLY | O_BINARY);
      if (fh != EOF)
      {
        if (_read(fh, dossave, dossize) == dossize)
        {
          copyToLow(vseg, 0, dossave, dossize);
          ok = 1;
        }
        _close(fh);
      }
      t_free(dossave);
    }
    unlink(VIDNAME);
    if (! ok)
      clrscr();

    _CX = doscursor;
    _AX = 0x100;
    geninterrupt(0x10);
  }
#endif
  vgabildaktiv = 0;
}

/*---------------------------------------------------------------------------*/

void dosabspeichern (int platz)
//*************************************************************************
//
//  Speichert das die momentane Umgebung des DOS-Bildschirmes ab
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  handle fh;
  video *dossave;
  if (! novideo)
  {
    if (! vgabildaktiv)
    {
      gettextinfo(&dosbildschirm);
      dossize = ad_peek(0x40, 0x4c);
      if (platz)
        dossave = (video *) t_malloc(dossize, "doss");
      if (dossave)
      {
        copyFromLow(dossave, vseg, 0, dossize);
        fh = _creat(VIDNAME, 0);
        if (fh != EOF)
        {
          _write(fh, dossave, dossize);
          _close(fh);
        }
        t_free(dossave);
      }
      doscursor = ad_peek(0x40, 0x60);
      if (vramcolor && ! cgadisp)
        bildzeilen = setmaxlines();
      else
        bildzeilen = ad_peekb(0x40,0x84)+1;
      bildspalten = ad_peekb(0x40,0x4a);
      if ((bildspalten<40) || (bildspalten>200)) // Plausi-Check
        bildspalten = 80;
      if ((bildzeilen<10) || (bildzeilen>100))
        bildzeilen = 25;
      _setcursortype(_SOLIDCURSOR);
      putchar(0);                                // Cursor initialisieren
      mausinit(bildzeilen-1, bildspalten-1);
    }
  vgabildaktiv=1;
  }
  else
    video_off=1;
#endif
}

/*---------------------------------------------------------------------------*/

void video_open (void)
//*************************************************************************
//
//  Stellt den Videomodus fest und schaltet ggf auf anderen Modus um
//
//*************************************************************************
{
#ifdef __HWSCREEN__
  int i;
  static int firstcall=1;
  if (novideo)
  {
    video_off = 1;
    vorne = (char *) t_malloc(bildzeilen*bildspalten, "*vsi");
    for (i=0; i < (bildzeilen*bildspalten); i++)
      vorne[i] = 0;
    vram = (video *) t_malloc(bildzeilen*bildspalten*2, "*ssi");
    vseg = FP_SEG(vram);
    return;
  }
  gettextinfo(&dosbildschirm);
#ifdef __DPMI32__
  _AH=0x0f;
  geninterrupt(0x10);
  if (_AL == 7)
  {
    vseg = 0xb000;
    dosbildschirm.currmode = MONO;
  }
  else
    vseg = 0xb800;
  vram = (video *) t_malloc(20000, "*vid");
#else
  if (dosbildschirm.currmode == MONO)
  {
    vram = (video far *) MK_FP(0xb000, 0);
    vramcolor = 0;
    vseg = 0xb000;
  }
  else
  {
    vram = (video far *) MK_FP(0xb800, 0);
    vseg = 0xb800;
  }
#endif
  dosabspeichern(1);
  if (firstcall)
  {
    firstcall = 0;
    if ((dosbildschirm.currmode == MONO)
        ||(dosbildschirm.currmode == BW80) || monodisp)
      monochrome = 1;
    set_color();
    vorne = (char *) t_malloc(bildzeilen*bildspalten, "*vor");
    for (i=0; i < (bildzeilen*bildspalten); i++)
      vorne[i]=0;
  }
#endif
}

/*---------------------------------------------------------------------------*/

void video_setcursor (unsigned x, unsigned y)
{
#ifdef __HWSCREEN__
  if (! video_off)
  {
    _DH=y;
    _DL=x;
    _BH=0;
    _AH=2;
    geninterrupt(0x10);
  }
#endif
}

/*---------------------------------------------------------------------------*/

void video_putc (int x, int y, unsigned attr, unsigned c)
{
  if (video_off)
    return;
  int index = x+bildspalten*y;
  if (vorne[index] == wa)
  {
    vram[index] = c | (attr << 8);
#ifdef __DPMI32__
    ad_poke(vseg, index << 1, c | attr << 8);
#endif
  }
}

/*---------------------------------------------------------------------------*/

void video_puts (int x, int y, unsigned attr, char *s)
{
  if (video_off)
    return;
  int index = x+bildspalten*y;
  attr <<= 8;
  while (*s)
  {
    if (vorne[index] == wa)
    {
      vram[index] = (*s) | attr;
#ifdef __DPMI32__
      ad_poke(vseg, index << 1, (*s) | attr);
#endif
    }
    index++;
    s++;
  }
}

/*---------------------------------------------------------------------------*/

void wclear (int col)
{
  int i, j;
  int xmax;
  int ymax = e->ylen-1+e->y;
  int line;
  video blank = ' ' | (col << 8);
  if (video_off)
    return;
  e->wpos = 0;
  for (j = e->y; j <= ymax; j++)
  {
    line = bildspalten*j;
    xmax = e->xlen-1+e->x+line;
    for (i = e->x+line; i <= xmax; i++)
    {
      if (vorne[i] == wa)
      {
        vram[i] = blank;
#ifdef __DPMI32__
        ad_poke(vseg, i << 1, blank);
#endif
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void video_update (void)
{
#ifdef __DPMI32__
  if (! video_off)
  {
    copyToLow(vseg, 0, vram, bildzeilen * bildspalten * 2);
  }
#endif
}

/*---------------------------------------------------------------------------*/

void video_switchon (void)
{
   if (novideo)
     return;
   if (video_off)
   {
     video_off = 0;
     allupdate++;
   }
   video_timer = ad_time()+m.crtsave*60;
}

/*---------------------------------------------------------------------------*/

void video_checkoff (int force)
{
  if (novideo)
  {
    video_off = 1;
    return;
  }
  if (video_timer == 0)
  {
    if (m.crtsave)
      video_switchon();
    else
      return;
  }
  if (force || (ad_time() > video_timer && video_timer))
  {
    if(! video_off && m.crtsave)
    {
      video_setcursor(255, 255);
      memset(vram, 0, bildzeilen * bildspalten*2);
      video_update();
      video_off = 1;
    }
  }
}

/*---------------------------------------------------------------------------*/
