/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------
  Diverse Utilities zum Layer2
  ----------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

#include "baycom.h"

long unsigned xramused = 0;

/*---------------------------------------------------------------------------*/

unsigned chainlen (frame_t *pnt)
//*************************************************************************
//
//  bestimmt die Laenge einer Kette, die in einem Buffer
//  abgelegt ist. Dadurch koennen Obergrenzen bestimmt werden,
//  wieviele Listenelemente in einem Buffer maximal
//  abgelegt werden sollen.
//
//*************************************************************************
{
  unsigned len = 0;
  int reksave = rekfl;
  rekfl = TRUE;
  disable();
  while (pnt)
  {
    pnt = pnt->next;
    enable();
    len++;
  }
  enable();
  rekfl = reksave;
  return len;
}

/*---------------------------------------------------------------------------*/

void chroutstr (byte *s)
{
  while (*s) chrout(*s++);
}

/*---------------------------------------------------------------------------*/

frame_t *hintenhol (frame_t *buf)
{
  if (buf)              // ist ueberhaupt was drin ?
  {
    while (buf->next)   // ist es schon das letzte Element der Kette?
      buf = buf->next;  // nein, dann in der Kette weiterspringen
  }
  return buf;
}

/*---------------------------------------------------------------------------*/

void hintenweg (frame_t **buf)
//*************************************************************************
//
//  deallokiert das letzte Element einer Liste
//  die Spezialfaelle 'Liste leer' und
//  'nur 1 Element in der Liste' werden beruecksichtigt
//
//*************************************************************************
{
  frame_t *pbuf = (*buf);
  frame_t *save = pbuf;
  disable();
  if (pbuf)               // ist ueberhaupt was drin ?
  {
    if (pbuf->next)       // ist es das letzte Element in der Kette?
    {
      while ((pbuf->next)->next) // schon das vorletzte Element der Kette?
      {
        pbuf = pbuf->next; // nein, dann in der Kette weiterspringen
        enable();
      }
      save = pbuf->next;
      pbuf->next = NULL;
      enable();
    }
    else
    {
      enable();
      *buf = NULL;
    }
    disable();
    dealloc(save);
    enable();
  }
  enable();
}

/*---------------------------------------------------------------------------*/

frame_t *vonhinten (frame_t **buf)
//*************************************************************************
//
//  holt das letzte Element einer Liste und isoliert es
//  Die Spezialfaelle 'Liste leer' und
//  'nur 1 Element in der Liste' werden beruecksichtigt
//
//*************************************************************************
{
  frame_t *pbuf = (*buf);

  if (pbuf)                    // ist ueberhaupt was drin ?
  {
    frame_t *save = pbuf;
    disable();
    if (pbuf->next)            // ist es das letzte Element in der Kette?
    {
      while (pbuf->next->next) // schon das vorletzte Element der Kette?
      {
        pbuf = pbuf->next;     // nein, dann in der Kette weiterspringen
        enable();
      }
      save = pbuf->next;
      pbuf->next = NULL;
    }
    else *buf = NULL;
    enable();
    save->next = NULL;
    return save;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/

unsigned int findport (int bereich)
//*************************************************************************
//
//  sucht einen freien L2-Port.
//
//*************************************************************************
{
  unsigned int i;

  for (i = 1; i < w.maxports; i++)
  {
    if (pfeld[i] && pfeld[i]->lstate == disconnected && pfeld[i]->mode.open)
      return(i);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

#if WITHNUDL
void umb (int enable)
{ if (enable)
  { _AX = 0x5803;
    _BX = 0x0001;
    asm int 21h;
    _AX = 0x5801;
    _BX = 0x0082;
    asm int 21h;
  }
  else
  { _AX = 0x5801;
    _BX = 0x0000;
    asm int 21h;
    _AX = 0x5803;
    _BX = 0x0001;
    asm int 21h;
  }
}

/*---------------------------------------------------------------------------*/

void *xalloc (unsigned size)
//*************************************************************************
//
//  Sucht Speicher zunaechst im Erweiterungsspeicher-Bereich
//  oberhalb 640k. Ist dort nichts verfuegbar, so wird normal
//  mit Farmalloc Speicher angefordert
//
//*************************************************************************
{
  unsigned xsize, sizeseg, i;
  static unsigned tmpbegin[MEMPAGES];
  unsigned minimum = 0xffff;
  int optimum = -1;
  void *memory = NULL;

  if (size)
  {
    size += 1;          // zur Sicherheit gegen etwaige Ueberlappungen
    sizeseg = size >> 4;
    if (size % 16) sizeseg++;
    for (i = 0; i < MEMPAGES; i++)     // Suche nach kleinstem, gerade
    {
      xsize = w.xend[i] - tmpbegin[i]; // noch passendem Speicherblock
      if (xsize > sizeseg && xsize < minimum)
      {
        minimum = xsize;
        optimum = i;
      }
    }
    if (optimum >= 0)                  // Passender Speicherblock
    {
      xsize = tmpbegin[optimum];       // gefunden
      xramused += size;
      tmpbegin[optimum] += sizeseg;    // untere Grenze aufruecken
      memory = MK_FP(xsize, 0);
      goto memfound;
    }
    else
    { memory = farmalloc(size); // kein upper Memory -> normaler Speicher
      memfound:
      if (memory) memset(memory, 0, size);
      return memory;
    }
  }
  else
  {
    unsigned block = CBUFLEN;
    int erg;
    umb(1);
    for (i = 0; i < MEMPAGES;)
    {
      erg = allocmem(block, &w.xbegin[i]);
      if (erg >= 0 || w.xbegin[i] < 0xb000U)
      {
        if (erg < 0) freemem(w.xbegin[i]);
        w.xbegin[i] = 0;
        block = (unsigned) ((block * 100L) / 101);
        if (block < 8) break;
      }
      else
      {
        w.xend[i] = w.xbegin[i] + block;
        //if(i==0)
        //   printf("UMB: ");
        //printf("%ld:%04X ",block*16L,w.xbegin[i]);
        i++;
      }
    }
    if (w.xbegin[0]) putc(LF);
    umb(0);
  }
  for (i = 0; i < MEMPAGES; i++) // bei 0 Speicherverwaltung initialisieren
    tmpbegin[i] = w.xbegin[i];
  return NULL;
}

/*---------------------------------------------------------------------------*/

#else
void *xalloc (unsigned size)
{
  void *memory = malloc(size + 1);
  if (memory) memset(memory, 0, size);
  else
  {
    puts("\n\al2: xalloc: out of memory");
    exit(0);
  }
  return memory;
}
#endif

/*---------------------------------------------------------------------------*/

int paralloc (void)
//*************************************************************************
//
//  allokiert Speicher fuer Routen- und MH-Liste. Diese
//  Bereiche muessen bereits vor dem Laden der Files
//  bereitgestellt werden.
//
//*************************************************************************
{
#if WITHNUDL
  xalloc(0); // externen Speicher initialisieren
  rlist = xalloc((RLEN + 4) * sizeof(route_t));
#endif
  return OK;
}

/*---------------------------------------------------------------------------*/

int meminit(void)
//*************************************************************************
//
//  legt eine Freispeicherkette an
//  und initalisiert die dynamischen Variablenbereiche
//
//*************************************************************************
{
  unsigned i;
  frame_t *allocblk;
  char *mem;
  bfree = NULL;
  stack = (char *) xalloc(STACKLEN);
  l2_mhlist = (mheard_t *) xalloc((w.mhlen + 5) * sizeof(struct mheard_t));
#if WITHNUDL
  l1stack = xalloc(STACKLEN);
  tfeld = xalloc(TASKS * sizeof (TASK));
  l2buf = xalloc(256);
//#error "mach was"
#else
  l2buf = (byte *) xalloc(70);
#ifdef __MSDOS__
  unsigned memfree_ = (unsigned)
    ((coreleft() - (3000 + ports * sizeof(port_t))) / sizeof(frame_t));
#else
  unsigned memfree_ = 600;
#endif
  if (w.maxbuf > memfree_) w.maxbuf = memfree_;
  mem = (char *) xalloc(w.maxbuf * sizeof(frame_t));
  for (i = 0; i < w.maxbuf; i++)
  {
    allocblk = (frame_t *) mem;
    dealloc(allocblk);
    mem += sizeof(frame_t);
  }
#endif
  return 1;
}

/*---------------------------------------------------------------------------*/

int aflen (byte *af)
//*************************************************************************
//
//  Uebergibt die Laenge eines Adressfeldes
//
//*************************************************************************
{
  unsigned int i = 0;
  while (! (af[i++] & 1));
  return i;
}

/*---------------------------------------------------------------------------*/

void setaf (byte *af1, byte *af2)
//*************************************************************************
//
//  dreht zur Aussendung ein Adressfeld nebst Digipeaterkette um
//  und kopiert es dabei in einen neuen Speicherbereich.
//  Quell- und Zieladressfeld duerfen sich nicht ueberlappen
//
//  Beim Aufruf muss pdigifl einen gueltigen Wert haben, also
//  ein Aufruf von chkaf vorangegangen sein
//
//  pdigifl ist ein Zaehler auf das erste Rufzeichen im empfangenen
//  Adressfeld, bei dem das eigene Digicall entdeckt wurde.
//
//*************************************************************************
{
  int i, callcnt;
  int hcnt = 0;
  callcnt = aflen(af1) / 7;
  callcopy(af1, af2 + 7);
  callcopy(af1 + 7, af2);
  for (i = 2; i < callcnt; i++)
  {
    callcopy(af1 + 7 * (i), af2 + 7 * ((callcnt - i) + 1));
    if (pdigifl && (++hcnt > (pdigifl - 2)))
      af2[7 * ((callcnt - i) + 1) + 6] |= 0x80;
  }
  af2[7 * callcnt - 1] |= 1;       // Endebit setzen
}

/*---------------------------------------------------------------------------*/

void paccod (int kanal, byte *adress, byte ctrl, int pf, int inflen, byte *info)
//*************************************************************************
//
//  bereitet ein Frame jeglichen Typs fuer die Sendung auf und legt
//  es im Buffer fuer den Layer 1 ab. Ein mit PACCOD in Auftrag
//  gegebenes Frame wird in jedem Fall mit dem Freiwerden des DCD
//  gesendet, kann also nicht mehr zurueckgenommen oder modifiziert
//  werden.
//
//  pf..poll-flag
//*************************************************************************
{
  frame_t *txin;
  int i = 0,tx = 255;
  disable();
  if (bfree && bfree->next)
  {
    alloc(txin);
    enable();
    while (0 == (1 & (txin->inhalt[i++] = (*(adress++)))));  // Adressfeld

    if (pf == poll || pf == command)
    {
      txin->inhalt[6] |= 0x80;
      txin->inhalt[13] &= 0x7f;
    }
    else
    {
      if (pf == pf_v1) txin->inhalt[13] &= 0x7f;
      else txin->inhalt[13] |= 0x80;
      txin->inhalt[6] &= 0x7f;
    }
    if (pf == poll || pf == final) ctrl |= 0x10;
    txin->inhalt[i++] = ctrl;             // Kontrollfeld einsetzen
    if (inflen > 0)
    {
      while (inflen--)
        txin->inhalt[i++] = *(info++); // Infofeld uebertragen
    }
    txin->len = i;
    if (kanal < KANAELE)                   // nicht bei unbrauchbar-Kanal
    {
      if (l1state(kanal) & CH_TXB) tx = 2;
      else tx = (w.mode[kanal].txdelay * 3) / 4;
#if WITHNUDL
      txbytes[infoslot][kanal] +=        // fuer Statistik
        (i + ((long) tx * w.hbaud[kanal]) / 800);
      if (w.damaports & (1 << kanal))
        txin->inhalt[13] &= 0xdf;         // Dama-Bit setzen
#endif
    }
    txin->txdtime = tx;
    txin->index = 0;
    txin->channel = kanal;
    pactx(txin);
  }
#if WITHNUDL
  else txmemfails++;
#endif
  enable();
}

/*---------------------------------------------------------------------------*/

void dmsend (frame_t *packl)
//*************************************************************************
//
//  beantwortet ein ueber einen Zeiger referiertes Frame mit DM
//
//  Es wird stets mit gesetztem Final beantwortet, da im normal-
//  Ablauf des AX.25 nur ein Poll-Frame zu DM fuehrt.
//
//*************************************************************************
{
  byte af[70];
  setaf(packl->inhalt, af);
  paccod(packl->channel, af, DM, final, 0, NULL);
}

/*---------------------------------------------------------------------------*/

#if WITHNUDL
void rauswerfen (frame_t *packl, char *text)
//*************************************************************************
//
//  beantwortet einen Connect mit einem Text
//  ohne eine AX.25-Verbindung aufzubauen
//
//  Text muss zunaechst mit einem gueltigen PID (z.B. 0xf0)
//  beginnen.
//
//*************************************************************************
{
  char af[70];
  setaf(packl->inhalt, af);
  paccod(packl->channel, af, UA, final, 0, NULL);
  paccod(packl->channel, af, 0, command, strlen(text), text);
  paccod(packl->channel, af, DISC, poll, 0, NULL);
}
#endif
