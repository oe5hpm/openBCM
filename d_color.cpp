/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------
  DOS: Festlegen der Farbattribute
  --------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

#include "baycom.h"

colstruct color;

/*---------------------------------------------------------------------------*/

static colstruct defcolor=
{ fg_white  + bg_black,

  fg_black  + bg_white,
  fg_red    + bg_white,

  fg_black  + bg_white,
  fg_red    + bg_white,

  fg_lwhite + bg_green,
  fg_lwhite + bg_brown,

  fg_white  + bg_blue,
  fg_lwhite + bg_blue,
  fg_lwhite + bg_brown,
  fg_lwhite + bg_red,
  fg_yellow + bg_mag,
  fg_lwhite + bg_mag,
  fg_lwhite + bg_green,

  fg_yellow + bg_blue,
  fg_cyan   + bg_blue,
  fg_yellow + bg_mag,
  fg_white  + bg_blue,

  fg_white  + bg_blue,
  fg_white  + bg_blue,
  fg_yellow + bg_blue,
  fg_yellow + bg_mag
};

/*---------------------------------------------------------------------------*/

static colstruct defmono=
{ fg_white  + bg_black,

  fg_white  + bg_black,
  fg_black  + bg_white,

  fg_white  + bg_black,
  fg_black  + bg_white,

  fg_lwhite + bg_black,
  fg_white  + bg_black,

  fg_black  + bg_white,
  fg_black  + bg_white,
  fg_black  + bg_white,
  fg_black  + bg_white,
  fg_white  + bg_black,
  fg_lwhite + bg_black,
  fg_lwhite + bg_black,

  fg_white  + bg_black,
  fg_lwhite + bg_black,
  fg_black  + bg_white,
  fg_white  + bg_black,

  fg_black  + bg_white,
  fg_black  + bg_white,
  fg_lwhite + bg_black,
  fg_lwhite + bg_bblack,
};

/*---------------------------------------------------------------------------*/

void set_color(void)
{
  if (monochrome)
    memcpy(&color, &defmono, sizeof(colstruct));
  else
    memcpy(&color, &defcolor, sizeof(colstruct));
}

/*---------------------------------------------------------------------------*/
#ifdef FEATURE_COLOR
typedef enum wtype
{ wt_console,
  wt_monitor,
  wt_box,
  wt_boxlogin,
  wt_users,
  wt_editor
};

/*---------------------------------------------------------------------------*/

typedef struct wdefault
{ char x;
  char y;
  char xlen;
  char ylen;
  int  curx;
  int  cury;
  char isopen;
} wdefault;


/*---------------------------------------------------------------------------*/

/*
static struct
{ colstruct color;
  colstruct mono;
  wdefault console;
  wdefault monitor;
  wdefault sysop;
  wdefault users;
  wdefault login[10];
  wdefault editor[4];
} desktop_par;
*/


/*---------------------------------------------------------------------------*/

static char *farbname[]=
{ "Desktop",
  "Output-text",
  "Output-ctrl",
  "Edit-text",
  "Edit-block",
  "Active-frame",
  "Inactive-frame",
  "Box-frame",
  "Box-text",
  "Box-warning",
  "Box-error",
  "Box-input",
  "Box-modified",
  "Box-okbutton",
  "Menu-text",
  "Menu-highlight",
  "Menu-select",
  "Menu-frame",
  "Help-frame",
  "Help-text",
  "Help-item",
  "Help-selected"
};

/*---------------------------------------------------------------------------*/

void colselect(void)
//*************************************************************************
//
//  Ein Fenster, mit dem alle vordefinierten Farben ausgewaehlt
//  werden koennen
//
//*************************************************************************
{
  int i,j,a;
  static int line = 0,linebegin = 0;
  char *colarr = (char *)&color;
  int boxsave;
  int doupdate = 1;

  boxwopen(21, 5, 36, 14);
  wclear(color.box_text);
  for (i=0; i<16; i++) // Farbtafel zeichnen
  {
    for (j=0; j<8; j++) // Kein blinkendes Attribut
      wc(e->x+18+i, e->y+2+j, i+16*j, 'X');
  }
  do
  {
    if (doupdate)
    {
      wframe(color.box_frame, "Select colors", 0);
      wf(e->x+2, e->y+11, color.box_text,
                 "select: page  item, %c color", 24);
      allupdate++;
      doupdate=0;
    }
    for (i=0; i<8; i++)
    {
      wf(e->x+2, e->y+2+i,
         (line-linebegin==i)?color.menu_select:color.menu_text,
         "%-15s", farbname[i+linebegin]);
    }
    setzcurs((colarr[line]&15)+18, (colarr[line]>>4)+2, e);
    boxsave = boxopen;
    boxopen = 0; // damit allgemeiner Update nicht blockiert wird
    a = waitkey();
    boxopen = boxsave; // Fensterzustand wiederherstellen
    setzcurs(0, 255, e);
    switch (a >> 8)
    {
      case LINKS:
           if (colarr[line] & 15)
           {
             colarr[line]--;
             doupdate++;
           }
           break;

      case RECHTS:
           if ((colarr[line]&15) != 15)
           {
             colarr[line]++;
             doupdate++;
           }
           break;

      case RAUF:
           if (colarr[line] >> 4)
           {
             colarr[line]-=16;
             doupdate++;
           }
           break;

      case RUNTER:
           if ((colarr[line]>>4) != 7)
           {
             colarr[line]+=16;
             doupdate++;
           }
           break;

      case POS1:
           line = linebegin = 0;
           break;

      case ENDE:
           line = sizeof(colstruct)-1;
           linebegin = line-8;
           break;

      case PG_RUNTER:
           if (line < (sizeof(colstruct)-1))
           {
             line++;
             if ((line-linebegin) > 7)
               linebegin++;
           }
           break;

      case PG_RAUF:
           if (line)
           {
             line--;
             if (line < linebegin)
               linebegin--;
           }
           break;
    }
    if ((a&255) == 13) // bei ESC die alten Werte zurueckschreiben
    {
      if (monochrome)
        memcpy(&defmono, &color, sizeof(colstruct));
      else
        memcpy(&defcolor, &color, sizeof(colstruct));
      break;
    }
  } while ((a&255) != 27); // Return = Ende
  boxwclose();
  set_color();
}
#endif
