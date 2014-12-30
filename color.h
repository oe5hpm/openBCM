/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------------------------
  Definitionen und Prototypen fuer Bildschirmfarben
  -------------------------------------------------


  Copyright (c) 92-96 Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#ifndef _COLOR_H
#define _COLOR_H
typedef char att;

typedef struct
{ att desktop;

  att put_text;
  att put_ctrl;

  att edit_text;
  att edit_block;

  att activ_frame;
  att back_frame;

  att box_frame;
  att box_text;
  att box_warning;
  att box_error;
  att box_input;
  att box_modified;
  att box_okbutton;

  att menu_text;
  att menu_high;
  att menu_select;
  att menu_frame;

  att help_frame;
  att help_text;
  att help_item;
  att help_selected;
} colstruct;

extern colstruct color;

#define fg_black  0x00
#define fg_blue   0x01
#define fg_green  0x02
#define fg_cyan   0x03
#define fg_red    0x04
#define fg_mag    0x05
#define fg_brown  0x06
#define fg_white  0x07
#define fg_gray   0x08
#define fg_lblue  0x09
#define fg_lgreen 0x0a
#define fg_lcyan  0x0b
#define fg_lred   0x0c
#define fg_lmag   0x0d
#define fg_yellow 0x0e
#define fg_lwhite 0x0f
#define bg_black  0x00
#define bg_blue   0x10
#define bg_green  0x20
#define bg_cyan   0x30
#define bg_red    0x40
#define bg_mag    0x50
#define bg_brown  0x60
#define bg_white  0x70
#define bg_bblack 0x80
#define bg_bblue  0x90
#define bg_bgreen 0xa0
#define bg_bcyan  0xb0
#define bg_bred   0xc0
#define bg_bmag   0xd0
#define bg_bbrown 0xe0
#define bg_bwhite 0xf0

void set_color(void);
void colselect(void);
#endif
