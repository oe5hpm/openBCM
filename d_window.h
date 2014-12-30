/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  Routinen fuer DOS-Benutzeroberflaeche


  --------------------------------------------------
  Definitionen und Prototypen fuer Window-Verwaltung
  --------------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#ifndef _WINDOW_H
#define _WINDOW_H

typedef short unsigned video;
#define GROSS 0
#define KLEIN 1

struct edstruct
{ unsigned curline;
  unsigned curcol;
  char     *linbuf;
  unsigned maxbuf;
  unsigned maxlines;
  unsigned index;
  unsigned readindex;
  unsigned blockbegin;
  unsigned blockend;
  unsigned blockmark;
  unsigned modified;
  unsigned shouldupdate;
  int      wpos;
  void     *esave;
  char     *buf;
  video    *save;
  char     title[40];
  int      (*update)(int);
  int      (*linefunc)(char *);
  char     wa;
  char     klein;
  char     ebene;
  char     wincnt;
  char     curx;
  char     cury;
  char     x;
  char     y;
  char     xend;
  char     yend;
  char     x2;
  char     y2;
  char     xend2;
  char     yend2;
  char     xlen;
  char     ylen;
  char     wordwrap;
  int      xoffset;
  int      readlines;
  int      pos;
  int      frame;
};
typedef struct edstruct edstruct;

//*************************************************************************
//
//  Datenstrukturen für die Eingabe von Dialogboxen
//
//*************************************************************************

#define DIA_END 100     // wird bei letztem Feld zu 'input' addiert

typedef enum dia_input
{ dia_bool,             // Eingabe Ein/Aus (zum Ankreuzen)
  dia_choice,           // Auswahl mehrerer Punkte
  dia_str,              // Eingabe eines Strings
  dia_text              // keine Eingabe, nur Ausgabe
} dia_input;

typedef struct dia_form
{ dia_input input;      // Art des Eingabefeldes
  int       width;      // max. Eingabebreite
  int       x;          // relative Position im Fenster
  int       y;
  char      *prompt;    // Ausgabe vor (nach) dem Eingabefeld
  char      *dest;      // Vorbesetzung und Resultat
} dia_form;

extern struct edstruct *e;
extern int allupdate;

extern char     *vorne; // für jeden Bildpunkt, welches Fenster sichtbar ist
extern int       wa;    // aktuell aktives Fenster

extern unsigned bildzeilen;
extern unsigned bildspalten;
extern unsigned monochrome;
extern int boxopen;

extern video far *vram;
extern char      *fg;
extern char      *buf;

// d_wedit.cpp
void editfile(char *);
//void qsowin(int (*linefunc)(char *), unsigned buflen);
//void putvo_edit(char c);

// d_video.cpp
void wclear(int col);
#define wc video_putc
#define ws video_puts

// d_window.cpp
void wf(int x, int y, int attr, char *format,...);
void wstr(int line, int col, char *s);
void setzcurs(int x, int y,edstruct *e);
void wframe(int col, char *headerln, int doppelstrich);
void wopen(void);
void wclose(void);
void boxwopen(int x, int y, int xlen, int ylen);
void boxwclose(void);
void nachvorne(int new_win);
void resize(int winnum, int mode);
void vorne_reorg(void);
void wscrollup(int col);

// d_dialog.cpp
int yesno(char *s, int vorbesetzung);
void message(char *title, char *s[], unsigned zeit);
void msgline(char *s);
int inputwin(char *title, char *prompt, char *string, int maxlen);
int editline(unsigned &spalte, unsigned &zeile, int maxlen, char *string);

// d_putwin.cpp
int  putupdate(int full);
void preparewin(int (*update)(int), char *name, unsigned buflen);
void putvo_win(int);
void putflush_win(void);
void inputline_win(char *s, int maxlen, char cut);

// desktop.cpp
extern unsigned long cpuindex;
unsigned alt_taste(unsigned scan);
unsigned f_taste(unsigned scan);

// vidinit.cpp
void dosabspeichern(int platz);
void dosvideo(void);
void cdhome(void);

// d_mouse.asm
#ifdef FEATURE_MOUSE
#ifdef __MSDOS__
extern "C"
{
#else
#define cdecl
#endif
  int  cdecl mausistda(void);
  int  cdecl mausinit(int zeilen, int spalten);
  void cdecl maussetz(int xpos, int ypos);
  void cdecl mausein(void);
  void cdecl mausaus(void);
  int  cdecl mauszeile(void);
  int  cdecl mausspalte(void);
  int  cdecl mausknopf(void);
  int  cdecl mausrechts(void);
#ifdef __MSDOS__
}
#endif
#else
  #define mausistda(x)
  #define mausinit(x, y)
  #define maussetz(x, y)
  #define mausein()
  #define mausaus()
  #define mauszeile()  0
  #define mausspalte() 0
  #define mausknopf()  0
  #define mausrechts() 0
#endif
#endif
