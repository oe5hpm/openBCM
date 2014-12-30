/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------------------------
  Layer2-Funktionen zur Abwicklung des AX.25-Protokolles
  ------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int get_inbios (void);
void set_watch08 (void);

byte   pdigifl;
port_t *p = NULL;
byte   *l2buf; // Mehrzweckpuffer
unsigned ports = 0;
frame_t *deadlockbuffer = NULL;


static byte control;
static byte pid;
static byte match;
static byte mynumber;
static byte digipfl;
static byte linkloopmatch;
static pf_t pf; // moegliche Werte fuer PF-Variable

static byte infotx[KANAELE];

//
// Speicherreservierung fuer die Betriebsparameter
//
port_t *pfeld[PORTLIMIT]; // pro Port getrennte Layer 2 Variablen
char *stack;

/*---------------------------------------------------------------------------*/

void restt1 (int pnummer)
//*************************************************************************
//
//  initialisiert den Timer 1 auf den fuer die Verbindung
//  vorgesehenen Wert, wenn der Linkstatus ungleich 'info_transfer'
//  ist oder Infos zum Senden bereitstehen.
//
//*************************************************************************
{
  port_t *pp = pfeld[pnummer];

  if (pp->lstate != info_transfer || pp->v1s != pp->vs || pp->txinfout)
  {
    if (pp->rembusy)
      pp->timer1 = (w.val.frack >> 1) + RNRTIME;
    else
      pp->timer1 = pp->frack;
  }
}

/*---------------------------------------------------------------------------*/

void stopt1 (void)
//*************************************************************************
//
//  stopt1 stoppt den Timer 1, initialisiert den Retry-Counter
//
//*************************************************************************
{
  p->timer1 = 0;
  p->retries = w.val.frack;
}

/*---------------------------------------------------------------------------*/

void paccodsup (pf_t pft, int pnummer)
//*************************************************************************
//
//  erzeugt ein Supervisory-Frame, wobei auf den Zustand
//  busy==TRUE und lstate==rej_sent Ruecksicht genommen wird
//
//*************************************************************************
{
  port_t *pp = pfeld[pnummer];
  char ctl = RR; // Default ist RR

  if (pp->lstate == rej_sent) // hier REJ senden
  {
    if (pft == final) ctl = REJ;
    else return;
  }
  if (pp->busy) ctl = RNR;
  paccod(pp->channel, pp->adrfield, (ctl | (pp->vr << 5)), pft, 0, NULL);
}

/*---------------------------------------------------------------------------*/

void paccodrej (pf_t pft)
//*************************************************************************
//
//
//*************************************************************************
{
  byte ctl = REJ;
  if (p->lstate != rej_sent && pft == command) // hier REJ senden
  {
    pft = response;
    if (p->busy) ctl = RNR;
    paccod(p->channel, p->adrfield, (ctl | (p->vr << 5)), pft, 0, NULL);
    p->lstate = rej_sent;
  }
}

/*---------------------------------------------------------------------------*/

void infosend (byte ch, byte vbegin, byte vend)
//*************************************************************************
//
//  sendet eine Anzahl von Infoframes aus, deren Folgezaehler-
//  fenster mit den Uebergabeparametern festgelegt wird.
//  Im Spezialfall von begin==end wird nichts gesendet.
//
//  Auf Port 0 wird alles UI gesendet, stets mit Poll
//
//*************************************************************************
{
  byte ctl;
  frame_t *iframe;

  if (vbegin == vend) return; // es steht keine Info zum senden
  while (vbegin != vend)
  {
    if (p != pfeld[0]) // Port > 0 ?
      ctl = (p->vr << 5) | (vbegin << 1); // ja, dann Kontollfeld einsetzen
    else
    {
      ctl = UI + 0x10; // nein, also UI senden
      ch = final;
    }
    iframe = p->txinfptr[vbegin]; // Zeiger auf Info holen
    if (iframe)
    {
      paccod(p->channel, p->adrfield, ctl, ch, iframe->len - iframe->index,
                         iframe->inhalt + iframe->index);
      p->timer2 = 0; // etwaige Bestaetigungen fallen lassen
      if (p == pfeld[0])
      {
        disable();
        dealloc(p->txinfptr[vbegin]);
        enable();
      }
    }
    vbegin = (++vbegin) % 8;
  }
  restt1(p->number); // Timer 1 laufen lassen
  infotx[p->channel] = 1;
  p->roundtrip = 1; // Roundtrip-Delay Messung starten
  p->wasrej = 0;
}

/*---------------------------------------------------------------------------*/

int putinfo (byte rxframe, frame_t **inf)
//*************************************************************************
//
//  uebernimmt aus einem durch einen Pointer referierten Datenstrom
//  ein Frame in die Liste der auszusendenden Frames.
//
//  Dabei wird nichts gemacht wenn
//  -der Linkstatus nicht zum Info senden taugt
//  -die Gegenstation busy ist
//  -MAXFRAME ueberschritten ist
//  -INFOSTOP gesetzt ist, also der Sender aus ist und noch keine
//   Bestaetigung zu alten Infos eingetroffen ist
//
//  Die uebergebenen Daten werden verworfen, wenn der angewaehlte
//  Port disconnected ist.
//  Auf Port 0 wird grundsaetzlich gesendet, stets als UNPTOTO
//
//  Der Rueckgabewert FALSE signalisiert, dass das Frame NICHT
//  zur Kenntnis genommen werden konnte, egal aus welchem Grund
//  Dasselbe Frame muss also ggf. nochmal zu putinfo gegeben werden,
//  wenn es trotzdem gesendet werden soll
//
//  Im durch den Pointer referierten Listenelement steht die zu
//  sendende Info ab inf->index, und dauert bis inf->len
//
//  Ist der Zeiger auf das naechste Listenelement ungleich NULL,
//  so wird die Laenge des ausgesendeten Infofeldes von der Daten-
//  menge, maximal aber von PACLEN begrenzt.
//
//  Der Schalter rxframe bewirkt, dass das Listenelement stets in
//  seiner eigenen Laenge uebertragen wird, es findet kein Zertrennen
//  oder verketten von Frames statt. Das PID wird in diesem Fall
//  vom ersten Zeichen im Frame uebernommen, bei rxframe=FALSE wird
//  das PID stets 0f0h gesetzt.
//
//*************************************************************************
{
  if (! (*inf)) return FALSE;
  if ((   ((unsigned) ((8 + p->vs - p->v1s) % 8) < w.mode[p->channel].maxframe)
       && ((p->lstate == info_transfer) || (p->lstate == rej_sent))
       && ! p->rembusy && ! p->infostop
       && ((p->v1s == p->vs) || (l1state (p->channel) & (CH_TXB | CH_FDX))) )
     || (p == pfeld[0]) // auf Port 0 immer alles senden (UIs)
    )
  {
    frame_t *info;
    frame_t *src = hintenhol(*inf);
    int oldvs = p->vs;
    unsigned int i = 0;
    unsigned short j = 0;
    disable();
    if (bfree)
      alloc(info);
    else
    {
      enable();
      return FALSE;
    }
    enable();
    info->index = i;
    if (rxframe) // ist es ein Frame, das vorher bereits empfangen wurde?
    {
      while (src->index != src->len) // Infofeldlaenge erreicht?
        info->inhalt[i++] = src->inhalt[(src->index)++]; // Info uebertragen
      hintenweg (inf); // Frame hat nun seine Schuldigkeit getan
    }
    else                             // Hier Frame aus dem Layer 7 kommend
    {
      info->inhalt[i++] = p->pid; // also das fuer den Port vorgesehene PID
      if (! p->paclen) p->paclen = m.paclen ? m.paclen : 256;
      while (j++ < p->paclen && src->index != src->len)
           info->inhalt[i++] = src->inhalt[(src->index)++]; // Info uebertragen
    if (j < p->paclen && (*inf)->next) // gehts im naechsten Buffer weiter?
      {
        hintenweg(inf); // und gleich alten deallokieren
        src = hintenhol(*inf);
        while (j++ < p->paclen && src->index != src->len)
          info->inhalt[i++] = src->inhalt[(src->index)++]; // Info uebertragen
      }
    if (src->index == src->len) hintenweg(inf); // nix mehr drin, also weg
    }
    info->len = i;
    info->channel = p->channel;
    p->txinfptr[oldvs] = info;
    p->vs = p->v2s = (++p->vs) % 8;
    infosend(command, oldvs, p->vs);
    p->retries = w.val.retry;
    return TRUE;
  }
  else if (p->lstate == disconnected)
  {
    while (*inf) hintenweg(inf);
    return TRUE; // Auf den Datenports UIs verwerfen!
  }
  else return FALSE; // in Zweifelsfaellen Info speichern
}

/*---------------------------------------------------------------------------*/

void rnrfrei (int pnummer)
//*************************************************************************
//
//  gibt RNR-Zustand wieder frei
//
//*************************************************************************
{
  port_t *pp = pfeld[pnummer];
  pp->busy = FALSE;
  paccodsup(response, pnummer);
  pp->timer1 = pp->frack;
}

/*---------------------------------------------------------------------------*/

void tryinfo (void)
//*************************************************************************
//
//
//*************************************************************************
{
  //if(l1state(p->channel)!=1)
  {
    if (! p->itimer)
    {
      if (p->txinfout) while (putinfo(0, &p->txinfout)); // Layer 7 Info ausgeben
      if ((p->vs != p->v1s) || p->txinfout)
      {
         if (! p->timer1 && ! p->rembusy) restt1(p->number);
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void lreset (int caufbau)
//*************************************************************************
//
//  fuehrt einen Linkreset durch. Dies ist waehrend einer laufenden
//  Verbindung nicht zulaessig, da lreset alle Daten im Sendepuffer
//  verwirft, also definitiv Datenfehler entstehen.
//
//*************************************************************************
{
  int i;
  stopt1();
  p->roundtrip =
  p->timer2 = p->vr = p->vs = p->v1s = p->v2s = p->busy = p->rembusy = p->infostop =
  p->mynumber = p->qtimer = p->disctimer = p->wasrej = p->outgoing = 0;
  p->partner[0] = 0;
  p->pid = 0xf0; // Setzt abgehende PID auf default
  p->frack = w.val.frack >> 1;
  for (i = 0; i < 8; i++)
  {
    if (p->sammler[i])
    {
      disable();
      dealloc(p->sammler[i]);
      enable();
    }
    if (p->txinfptr[i])
    {
      disable();
      dealloc(p->txinfptr[i]);
      enable();
    }
  }
  if (p->rxinfloc)
  {
    disable();
    dealloc(p->rxinfloc);
    enable();
  }
  while (p->txinfout)
  {
    frame_t *sav = p->txinfout;
    p->txinfout = sav->next;
    disable();
    dealloc(sav);
    enable();
  }
  while (p->rxinfout && (p->number >= w.maxports))
  {
    frame_t *sav = p->rxinfout;
    p->rxinfout = sav->next;
    disable();
    dealloc(sav);
    enable();
  }
  if (p->lstate > info_transfer) p->lstate = info_transfer;
}

/*---------------------------------------------------------------------------*/

void dodisc (void)
//*************************************************************************
//
//  setzt einen Port in den DISCONNECTED-Zustand
//
//  Evtl. Daten im Sendepuffer werden verworfen
//  Alle Statusvariablen werden rueckgesetzt
//
//*************************************************************************
{
  unsigned int i;

  if (damatimer[p->channel])
  {
    int con = 0;
    for (i = 1; i < ports; i++)
    {
      if ((pfeld[i]->lstate > disconnected)
          && (pfeld[i]->channel == p->channel))
        con = 1;
    }
    if (! con)
    {
      damatimer[p->channel] = 0;
      damafree[p->channel] = 1;
    }
  }
  if (p->lstate == link_setup) linkmsg(msg_failure);
  else if (p->lstate > disconnected) linkmsg(msg_disconnected);
  p->lstate = disconnected; // Linkstatus ruecksetzen
  lreset(0);
}

/*---------------------------------------------------------------------------*/

int chkaf (frame_t *packl)
//*************************************************************************
//
//  Ueberprueft das Adressfeld eines Frames auf Richtigkeit,
//  vergleicht es mit allen vorhandenen Adressfeldern,
//  und extrahiert das Kontrollfeld
//  sowie Poll, Final, Command, Response
//
//  globale Variablen:  pf, match, control, pid, mynum, digipfl, pdigifl
//                      linkloopcall
//
//  Uebergibt Null bei falschen oder nicht uebereinstimmenden Adressfeld
//  und ungleich Null bei korrelierenden Rufzeichen
//
//*************************************************************************
{
  unsigned int i, j, adrend = 0;
  int notall = FALSE;
  int hashint = *((short int *) (packl->inhalt + 11));
  linkloopmatch = match = mynumber = pdigifl = digipfl = 0;
  while (! (packl->inhalt[adrend++] & 1))
    if (adrend >= 70) return 0; // Ungueltig: Laenge > 70
  if ((adrend % 7) || (adrend < 14)) return 0; // kein Faktor 7 oder zu kurz
  if ((control = (packl->inhalt[adrend])) & 0x10) pf = poll;
  else pf = command;
  control = control & 0xef;
  for (i = 1; i < ports; i++)
  {
    p = pfeld[i];
    if (hashint == *((short int *) (p->adrfield + 4)))
    {
      if ((p->lstate > disconnected) && (p->channel == packl->channel))
      {
        if (callcmp (p->adrfield + 7, packl->inhalt))
        {
          if (callcmp (p->adrfield, packl->inhalt + 7))
          {
            if (((unsigned) aflen (p->adrfield) == adrend) && control != UI)
            {
              int z = 14;
              match = i;
              while (! (packl->inhalt[z-1] & 1))
              {
                if (! callcmp(p->adrfield + z, packl->inhalt + 7 + adrend - z))
                  match = 0;
                if (! ((p->adrfield[z + 6] ^ packl->inhalt[13 + adrend - z]) & 0x80))
                  match = 0;
                z += 7;
              }
            }
          }
        }
      }
      if (match) break;
    }
  }
  if (! match)
  {
    for (i = 14; i < adrend; i += 7)
    {
      if (0 == (packl->inhalt[i + 6] & 0x80)) // ist H-Bit gesetzt ?
      {
        if (callcmp(packl->inhalt + i, w.val.dcall)) // nein, aber digipeaten?
        {
          for (j = 14; j < i; j += 7) // ist das frame schon
          {
            if (callcmp (packl->inhalt + j, w.val.dcall)) // durch den eigenen
              return 0; // Digi gelaufen? Wenn ja -> wegwerfen
          }
          packl->inhalt[i + 6] |= 0x80; //  H-Bit setzen
          digipfl = TRUE;
          if (! pdigifl) pdigifl = i / 7;
        }
        else if (! pdigifl) return 0; // Frame muss noch digipeatet werden
        else notall = TRUE; // es kommt noch ein anderer Digi hinterher
      }
    }
  }
  if (! notall && ! match)
  {
    i = 0;
    do
    {
      if (   callwildcmp(packl->inhalt, w.val.cnot + i)
          || callwildcmp(packl->inhalt + 7, w.val.cnot + i)) return(0);
      i += 7;
    }
    while (! (w.val.cnot[i - 1] & 1));
    i = 0;
    do
    {
      if (callcmp(w.val.mycall + i, packl->inhalt))
        mynumber = (i / 7) + 6;
      i += 7;
    }
    while (! (w.val.mycall[i - 1] & 1));
  }
  pid = (packl->inhalt[adrend + 1]);
  if (! ((packl->inhalt[6] ^ packl->inhalt[13]) & 0x80))
  {
    pf = pf_v1;
    if (! digipfl) return(0); // ignore V1
  }
  else if (packl->inhalt[13] & 0x80)
  {
    if (pf == poll) pf = final;
    else pf = response;
  }
  return digipfl + pdigifl + mynumber + match;
}

/*---------------------------------------------------------------------------*/

int vomnachbarn (frame_t *packl)
//*************************************************************************
//
//
//*************************************************************************
{
  int i = 7;
  int mask = 0;

  while (! (packl->inhalt[i - 1] & 1))
  {
    if (   (nachbar(packl->inhalt + i) != NOTFOUND)
        && ((packl->inhalt[i + 6] & 0x80) == mask)
        && (! callcmp(packl->inhalt + i, w.val.dcall)))
      return 1;
    else i += 7;
    mask = 0x80;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void conack (frame_t *packl, int digiack)
//*************************************************************************
//
//  beantwortet einen SABM mit UA, und sucht bei nicht ueberein-
//  stimmenden Adressfeld einen freien Port
//  Das Adressfeld wird incl. Digipeaterweg gesetzt, der
//  Linkstatus ist hinterher 'info_transfer'
//
//  Ausserdem wird hier der CTEXT gesendet
//
//  Ist kein Port frei, so wird mit DM geantwortet
//
//  Die globale Variable match enthaelt den Port, auf dem das
//  Adressfeld mit dem gesendeten uebereinstimmt, bzw 0,
//  wenn kein passender Port gefunden wurde
//
//*************************************************************************
{
  unsigned int pnum;

  if (! match)
  {
    {
      pnum = findport(0);
      if (! pnum) // kein Port mehr frei
      {
        dmsend(packl);
        return;
      }
    }
    p = pfeld[pnum];
    dodisc();
    p->channel = packl->channel;
    p->mynumber = mynumber - 1;
    p->lstate = info_transfer;
    callasc(packl->inhalt + 7, p->partner);
    setaf(packl->inhalt, p->adrfield);
  }
  else p = pfeld[match];
  {
    if (! match) linkmsg(msg_connected);
    paccod(p->channel, p->adrfield, UA, final, 0, NULL);
    if ((mynumber > 5) && (! match) && (mynumber != 10))
    {
      if (w.val.l2ctext[0] && ! w.sccon)
      {
        chroutstr((byte *) w.val.l2ctext);
        p->itimer = 1;
      }
    }
    else
      restt1(p->number);
    p->retries = w.val.retry;
  }
}

/*---------------------------------------------------------------------------*/

void checkfracktime (void)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int min = 16100U / (w.mode[p->channel].hbaud + 1); // min 2.5sec bei 1200bd
  if (min < 10) min = 10;
  if (w.mode[p->channel].chmode == c_kiss) min = 25; // bei Kissmode 5sec FRACK minimum
  if (p->frack < min) p->frack=min;              // Anschlag festlegen
  if (p->frack > 40) p->frack=40;                // max 8sec
  if (w.mode[p->channel].hbaud == 300) p->frack = w.val.frack >> 1; // auf KW gilt das Faustrecht
  //if (w.mode[p->channel].chmode == c_axip) p->frack = w.val.frack = 0;
//  printf("Frack: %d %d\n",p->frack, (w.val.frack >> 1));
}

/*---------------------------------------------------------------------------*/

void t1tout (void)
//*************************************************************************
//
//  behandelt das Ablaufen des Timer 1 bzw. Timer 3
//  und beruecksichtigt dabei den Linkstatus, sendet ggf Kommandos
//  aus und aendert den Linkstatus wenn der Wiederholungszaehler
//  abgelaufen ist
//
//*************************************************************************
{
  if (p == pfeld[0]) return;
  if (p->lstate != link_setup) p->roundtrip = 0;
  p->timer3 = w.val.linktime;
  switch (p->lstate)
  {
    case link_setup:
      {
        if ((--p->retries))
        {
          paccod(p->channel, p->adrfield, SABM, poll, 0, NULL);
          p->timer1 = (++p->frack);
        }
        else
        {
          paccod(p->channel, p->adrfield, DISC, poll, 0, NULL);
          p->lstate = disconnected;
          linkmsg(msg_failure);
          dodisc();
        }
      } break;
    case disc_request:
      {
        if ((--p->retries))
        {
          paccod(p->channel, p->adrfield, DISC, poll, 0, NULL);
          p->timer1 = p->frack;
        }
        else dodisc();
      } break;
    case rej_sent:
    case info_transfer:
      {
        p->lstate = wait_ack;
        p->retries = w.val.retry;
        t1tout(); // hier Rekursion, aber andere Voraussetzungen
      } break;
    case wait_ack:
      {
        if (--p->retries)
        {
          frame_t *txi = p->txinfptr[p->v1s];
          if (   txi && (8 + p->vs - p->v1s) % 8 == 1
              && (txi->len - txi->index) < w.val.ipoll
              && (w.val.retry - p->retries) < 4)
          {
            p->v2s = p->vs;
            infosend(poll, p->v1s, p->vs);
          }
          else paccodsup(poll, p->number);
          if (p->rembusy) p->timer1 = (w.val.frack >> 1) + RNRTIME;
          else p->timer1 = p->frack;
          p->frack++;
          checkfracktime();
        }
        else
        {
          p->retries = 2;
          p->lstate = disc_request;
          t1tout(); // Rekursion
        }
      } break;
    case frame_reject:
      {
        paccod(p->channel, p->adrfield, FRMR, response, 3, (byte *) "\0\0\0"); //leeres Info
        p->lstate = disc_request;
        p->retries = 2;
        p->timer1 = w.val.frack;
      } break;
    default: break;
  }
}

/*---------------------------------------------------------------------------*/

void disconnect (int mode)
//*************************************************************************
//
//  trennt eine Verbindung bzw. bricht einen Verbindungsaufbau
//  ab. Ein zweimaliger Aufruf fuehrt zum sofortigen Trennen
//  einer bestehenden Verbindung.
//
//  Der Parameter mode bestimmt dabei den Zeitpunkt, wann
//  die Verbindung getrennt wird.
//
//  mode==0: sofortiger Abbruch, keine weitere Aussendung
//  mode==1: normaler Disconnect
//  mode==2: quit, also warten bis alles bestaetigt ist
//  mode==4: disconnect ohne qtimer
//  mode==5:
//
//*************************************************************************
{
  unsigned int pnum;

  if (mode == 4)
  {
    p->qtimer = 0;
    return;
  }
  else
    if (mode == 5)
    {
      int psave = p->number;
      for (pnum = 1; pnum < w.maxports; pnum++)
      {
        p = pfeld[pnum];
        disconnect(1);
      }
      p = pfeld[psave];
      return;
    }
  if ((p->lstate == disc_request) || (! mode))
    dodisc();
  else
    if (p->lstate == link_setup)
    {
      paccod(p->channel, p->adrfield, DISC, poll, 0, NULL);
      dodisc();
    }
    else
      if ((mode == 2) && (p->txinfout || (p->vs != p->v1s)))
        p->qtimer = w.val.qtime;
      else
        if (p->lstate >= info_transfer)
        {
          p->lstate = disc_request;
          p->qtimer = 0;
          p->timer2 = 0;
          paccod(p->channel, p->adrfield, DISC, poll, 0, NULL);
          restt1(p->number);
          p->retries = w.val.retry / 4;
        }
}

/*---------------------------------------------------------------------------*/

void connect (byte *my, byte *dest, int pdnum)
//*************************************************************************
//
//  baut eine Verbindung auf. Ein Aufruf waehrend einer bestehenden
//  Verbindung bewirkt gar nichts.
//
//  Waehrend 'link_setup' wird der Verbindungsaufbau mit DISC
//  abgebrochen und die neue Verbindung aufgebaut.
//
//  Uebergeben wird ein Zeiger auf das mycall, sowie ein Zeiger auf
//  das Zielcall mit anschliessender Digipeaterkette.
//
//  Ausserdem wird die Anzahl der Pseudodigis uebergeben, bei diesen
//  wird im Laufe der Verbindung bei der Aussendung das H-Bit
//  stets gesetzt, und beim Empfang in nicht gesetztem Zustand
//  erwartet.
//
//  Alle Rufzeichen sind im 'shifted'-Format, der Abschluss
//  der Digikette erfolgt wie ueblich mit Bit0 = 1
//
//*************************************************************************
{
  byte cbf[70];
  int i = 0;
  int pdcnt = pdnum;

  if (p->lstate < info_transfer)
  {
    disconnect(1);
    callcopy(my, cbf + 7);
    callcopy(dest, cbf);
    cbf[6] |= 0x60;
    incssid(cbf + 7, cbf, 1);
    if (p == pfeld[0]) callcopy(dest, w.unproto);
    while (! (dest[i + 6] & 1))
    {
      i += 7;
      callcopy(dest + i, cbf + 7 + i);
      if (pdcnt)
      {
        pdcnt--;
        cbf[13 + i] |= 0x80; // Pseudodigi H-Bit setzen
      }
    }
    cbf[i + 13] |= 1;
    p->channel = router(cbf, p->adrfield);
    if (p != pfeld[0])
    {
      lreset(1);
      p->lstate = link_setup;
      restt1(p->number);
      if (p->adrfield[13 + (7 * pdnum)] & 1) // ist im Adressfeld ein Digipeater?
      {
        p->retries = w.val.retry / 4 + 1; // kein Digi, nur N2/4
      }
      else
      {
        p->retries = w.val.retry; // koennte RMNC sein, also viel Retries
      }
      p->outgoing = 1;
      paccod(p->channel, p->adrfield, SABM, poll, 0, NULL);
      callasc(cbf, p->partner);
    }
  }
}

/*---------------------------------------------------------------------------*/

void dobeacon (int start)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int i;
  static unsigned btimer[KANAELE + 1] = { 0 };

  for (i = 0; i < KANAELE; i++)
  {
    l1_mode_t *mm = &w.mode[i];
    if (start)
      btimer[i] = mm->btime * 5;
    else
    {
      if (btimer[i] && ! (--btimer[i]))
      {
        paccod(i, mm->badress, UI, poll, strlen(mm->btext), (byte *) mm->btext);
        btimer[i] = mm->btime * 5;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void timerset (void)
//*************************************************************************
//
//  setzt alle fuer den Protokollablauf notwendigen Zeitgeber
//  und zieht erforderlichenfalls Konsequenzen daraus.
//
//*************************************************************************
{
  unsigned int pnum;
  unsigned int i;

//  printf("timerset %ldms %ld\n", systic*55, systic);
  if (tfaellig > 14)
  {
    tfaellig = 0;
    {
      static int t3vorteiler = 50;
#ifndef __FLAT__
      if (w.val.blink&1 && !w.sccon)
        pokeb(w.vseg, 159, peekb(w.vseg, 159) ^ 0x77); // Ecke blinken lassen
#endif
      p = pfeld[0];
      dobeacon(FALSE);
      if (p)
      {
        if (p->itimer) --p->itimer; // laeuft Infotimer?
        if (! p->itimer && p->txinfout) tryinfo();
        for (i = 0; i < channels; i++)
        {
          if (! (l1state(i) & CH_TXB)) infotx[i] = 0;
        }
        if (! (--t3vorteiler))
        {
          t3vorteiler = 50;
          for (i = 0; i < KANAELE; i++)
          {
            if (damatimer[i]) damatimer[i]--;
          }
          p->disctimer = damatimer[0];
          w.zeit++;
          for (pnum = 1; pnum < ports; pnum++)
          {
            p = pfeld[pnum];
            if (p)
            {
              if (p->lstate && p->timer3)
              {
                if (! (--p->timer3)) t1tout();
              }
              if (p->qtimer)
              {
                if (--p->qtimer)
                {
                  if ((! p->txinfout) && (p->vs == p->v1s)) disconnect(1);
                }
                else
                  disconnect(1);
              }
              if (p->disctimer)
                if (! (--p->disctimer)) disconnect(1);
            }
          }
        }
        for (pnum = 1; pnum < ports; pnum++)
        {
          p = pfeld[pnum];
          if (p)
          {
            if ((p->lstate) && ((p->itimer | p->timer1 | p->timer2) || (p->txinfout)))
            {
              if (p->itimer) --p->itimer; // laeuft Infotimer?
              if (! p->itimer && p->txinfout) tryinfo();
              i = l1state(p->channel);
              if (p->timer1 && ! damatimer[p->channel])
              {
                if ((i & CH_PTT) && (p->timer1 < 3)) p->timer1 = 0;
                if (! (i & CH_PTT))
                {
                  if (! (i&CH_DCD)) --p->timer1;
                  else if (! (t3vorteiler % 4)) --p->timer1;
                }
                if (! p->timer1) t1tout();
              }
              if (p->timer2)
              {
                if (i & CH_PTT) p->timer2 = 0;
                if (! (i&(CH_DCD|CH_PTT))) --p->timer2;
                if (! p->timer2) paccodsup(response, p->number);
              }
            }
          }
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void infack (int nr)
//*************************************************************************
//
//  deallokiert eine Serie gesendeter Info-Frames, sobald sie
//  bestaetigt worden sind.
//
//  wird Info bestaetigt, die nie gesendet wurde, so fuehrt
//  das zum Linkstatus 'frame_reject'.
//
//*************************************************************************
{
//  printf("infack %ldms %ld\n", systic*55,systic);
  while (p->v1s != nr)
  {
    p->disctimer = w.val.disctime;
    if (p->txinfptr[p->v1s])
    {
      disable();
      dealloc(p->txinfptr[p->v1s]);
      enable();
    }
    else p->lstate = frame_reject;
    p->v1s = (++p->v1s) % 8;
  }
  if (p->vs == p->v1s)
  {
    p->infostop = FALSE;
    if (p->lstate != wait_ack) stopt1();
  }
  else
  {
    if (! (l1state(p->channel) & CH_FDX)) p->infostop = TRUE;
  }
}

/*---------------------------------------------------------------------------*/

void pacdecua (frame_t *packl)
//*************************************************************************
//
//
//*************************************************************************
{
  switch (p->lstate)
  {
    case link_setup:
    {
      stopt1();
      p->frack = w.val.frack >> 1;
      p->lstate = info_transfer;
      p->disctimer = w.val.disctime;
      linkmsg(msg_connected);
    } break;
    case disc_request: dodisc();
    default: break;
  }
}

/*---------------------------------------------------------------------------*/

void pacdecsup (byte ch)
//*************************************************************************
//
//
//*************************************************************************
{
  int lsave = p->lstate;

//  printf("pacdecsup: %ldms %ld\n", systic*55,systic);
  infack((ch >> 5) % 8);
  if (pf == response && p->roundtrip)
  { /*
    if (p->channel) p->frack = ((p->roundtrip + p->frack) * 3) / 5;
    else
    */
    p->frack = ((p->roundtrip + p->frack) * 4) / 5;
    checkfracktime();
    p->roundtrip = 0;
  }
  if (lsave == wait_ack && pf == final)
  {
    p->lstate = info_transfer;
    if (p->vs == p->v1s) stopt1();
  }
  if (   (p->v1s != p->vs)
      && ((p->v2s == p->vs) || (pf == final))
      && ((ch & 15) == REJ) )
  {
    p->v2s = (p->v1s + 1) % 8;
    infosend(command, p->v1s, p->v2s);
    p->retries = w.val.retry;
    p->wasrej = 1;
  }
  else
  {
    if ((lsave == wait_ack && pf == final) || (p->vs != p->v2s && p->wasrej))
    {
      if (p->vs != p->v1s && ! p->rembusy)
      {
        p->v2s = p->vs;
        infosend(command, p->v1s, p->v2s); // Infos wiederholen
        p->retries = w.val.retry;
        p->infostop = FALSE; // neue Infos gleich mit hinhaengen
      }
    }
    p->v2s = p->vs;
  }
  if (pf == poll) paccodsup(final, p->number); // Poll mit Final beantworten
  if (pf == command) paccodsup(response, p->number); // Command mit Response
  if ((ch & 0xf) == RNR)
  {
    p->rembusy = TRUE;
    p->timer1 = (w.val.frack >> 1) + RNRTIME;
  }
  else p->rembusy = FALSE;
}

/*---------------------------------------------------------------------------*/

int pacdecinf (frame_t *packl, int aussammler)
//*************************************************************************
//
//
//*************************************************************************
{
  int aufheb = 0;
  unsigned int l;
  int i = aflen(packl->inhalt) + 1; // hier einige Werte vorbereiten
  byte sum = 0;                     // um die Uebertragung der Info
  int len = packl->len;             // und die Quersummenbildung
  byte *info = packl->inhalt;       // moeglichst schnell zu machen

//  printf("pacdecinf %ldms %ld\n", systic*55,systic);
  p->disctimer = w.val.disctime;
  infack ((control >> 5) % 8); // evtl gesendete Infos dealloc
  if ((p->vr == ((control >> 1) % 8)) || aussammler) // passt N(S)?
  {
    if (p->lstate == rej_sent) p->lstate = info_transfer;
    if (! p->qtimer)
    { //von aussen reinkommende pid setzt lokale pid symmetrisch dazu
      p->pid = info[i];
      if (! w.sccon && strstr((char *) info + i + 1, "//q\r"))
        disconnect(1);
      if (w.val.blink == 2) w.val.blink = 3;
      {
        l = chainlen(p->rxinfout);
        if (l < (w.maxrx + 8))
        {
          aufheb = TRUE;
          packl->index = i;
          packl->channel = 0;
          iot_frame(packl);
          while ((++i) < len)
            sum += info[i]; // Info uebertragen, Quersumme bilden
          if (l > w.maxrx) p->busy = TRUE;
        }
      }
    }
    p->qsumme[p->vr] = sum;
    p->vr = (++p->vr) % 8;
    while (! aussammler && p->sammler[p->vr]) // Sammler checken ob was ist
    {
      i = p->vr;
      pacdecinf(p->sammler[i], 1); // einfache Rekursion erlauben
      p->sammler[i] = NULL;
    }
  }
  else
    if (p->vr != (((control >> 1) + 1) & 7)) // isses evtl soeben gekommenes?
    {
      paccodrej(pf); // Folgezaehler passt nicht, REJ
      while ((++i) < len) // ueber das nicht passende Frame Quersumme
        sum += info[i];   // berechnen, und schaun ob es in den Sammler darf
      i = (control >> 1) % 8;
      // nur in den Sammler reintun wenn
      //  - Quersumme ungleich alter Info
      //  - es das uebernaechste (!) Frame ist (also nicht 'irgendeins')
      //  - Quittimer nicht laeuft
      //
      if ((p->qsumme[i] != sum)
          && ! p->qtimer
          && (p->vr == (((control >> 1) + 7) & 7)))
      {
        p->qsumme[i] = sum;
        p->sammler[i] = packl; // also hinein damit
        aufheb = TRUE;
     }
    }
  if (pf == poll)
  {
    p->timer2 = 0;
    paccodsup(final, p->number);
  }
  else
  {
    if ((w.mode[p->channel].d) && ! aussammler)
      paccodsup(response, p->number);
    else
      p->timer2 = 1;//(w.val.resptime >> 1) + 1;
  }
  if (aussammler && ! aufheb)
  {
    disable();
    dealloc(packl);
    enable();
  }
  return aufheb;
}

/*---------------------------------------------------------------------------*/

void testdamafree (frame_t *packl)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int i;

  if (isdamamaster(packl) && pf == poll)
  {
    damatimer[packl->channel] = w.val.dama;
    damafree[packl->channel] = 1;
    for (i = 0; i < ports; i++)
    {
      int psave = p->number;
      p = pfeld[i];
      if ((p->lstate > disconnected) && (p->channel == packl->channel))
      {
        if((p->timer1) && (! infotx[packl->channel]))
        {
          p->timer1 = 0;
          t1tout();
        }
        if (p->timer2)
        {
          if (l1state (p->channel) & (CH_DCD | CH_PTT)) p->timer2 = 1;
          else
          {
            p->timer2 = 0;
            paccodsup(response, p->number);
          }
        }
      }
      p = pfeld[psave];
    }
  }
}

/*---------------------------------------------------------------------------*/

int pacdec (frame_t *packl)
//*************************************************************************
//
//  ist die generelle LAYER 2 - Empfangsroutine
//  hier werden die Frames geprueft, ausgewertet und
//  ggf. auf einem bestimmten Port weiterverarbeitet.
//
//  Jeder Aufruf von pacdec verarbeitet genau 1 Frame,
//  das mit dem uebergebenen Pointer referiert wird.
//
//*************************************************************************
{
  int aufheb = 0;
  int ok = chkaf(packl);

//printf("pacdec %ldms %ld\n", systic*55, systic);
  if (ok)
  {
    if (match) // Frame ist fuer einen bestimmten Port
    {
      p = pfeld[match];
      if (p->qtimer && (! p->txinfout) && (p->vs == p->v1s))
        disconnect(1);
      p->timer3 = w.val.linktime;
      switch (control)
      {
        case SABM: {
                     conack(packl, 0);
                   } break;
        case UA:   {
                     pacdecua(packl);
                   } break;
        case DISC: {
                     paccod(p->channel, p->adrfield, DM, final, 0, NULL);
                   }
        case DM:   {
                     if (p->lstate == link_setup)
                     {
                       p->lstate = disconnected;
                       linkmsg(msg_busy);
                     }
                     dodisc();
                   } break;
        case FRMR: {
                     disconnect(1);
                   } break;
        default:
                     if (p->lstate >= info_transfer)
                     {
                       if ((control & 3) == 1) // Supervisory-Frames
                         pacdecsup(control);
                       if (! (control & 1)) // Info-Frames
                       {
                         aufheb = pacdecinf(packl, 0);
                       }
                     }
      }
      tryinfo();
    }
    else
    {
      if (mynumber && pf == poll)
      {
        if (control == SABM) conack(packl, 0);
        else dmsend(packl);
      }
      else if (digipfl)
      {
        {
          {
            int digikanal;
            digikanal = router(packl->inhalt, l2buf);
            if (digikanal || (digipfl != 2))
              paccod(digikanal, l2buf,control, pf,
                     packl->len - (aflen(packl->inhalt) + 1),
                     packl->inhalt + aflen(packl->inhalt) + 1);
          }
        }
      }
    }
    testdamafree(packl);
  }
  p->outstanding = ((8 + p->vs - p->v1s) % 8) + (p->txinfout != NULL);
  return aufheb;
}

/*---------------------------------------------------------------------------*/

void layer2 (void)
//*************************************************************************
//
//  ist das Hauptprogramm fuer den Link-Layer. Hier werden
//  hereinkommende Frames verarbeitet und Timer abgefragt.
//
//   ACHTUNG: layer2 ist nicht reentrant. Es muss Vorsorge getroffen werden,
//            dass es im Interrupt nicht mehr aufgerufen wird, wenn es
//            bereits laeuft.
//
//*************************************************************************
{
#ifndef __FLAT__
  static unsigned stacksegbuf = 0; // im statischen, initialisierten Bereich
  static unsigned stackptrbuf = 0;
#endif
  static frame_t *rxframe = NULL;
  static int aufheb = 0;
  static int looplen = 0;
  static int kanal;

  if (rekfl) return;
  rekfl = TRUE;
#ifndef __FLAT__
  enable();
  stacksegbuf = _SS;              // Stacksegment retten
  stackptrbuf = _SP;              // und Pointer auch
  disable();
  _SS = FP_SEG(stack);            // Statisches Array zuweisen
  _SP = FP_OFF(stack) + STACKLEN; // Stackpointer ganz oben
#endif
  looplen = 0;
  {
    while (rxout)
    {
      rxframe = vonhinten(&rxout);
      enable();
      aufheb = 0;
      kanal = rxframe->channel;
      monitor(rxframe);
      rxframe->channel = kanal = (kanal & 0x7f);
      aufheb = pacdec(rxframe);
      if (aufheb == 2)
        aufheb = pacdec(rxframe); // evtl nochmal versuchen bei Reconnect
      disable();
      if (! aufheb)       // deallokieren nur wenns nicht fuer Node oder
        dealloc(rxframe); // Sammler nochmal gebraucht wird
    }
    enable();
    rxdaflag = FALSE; // kein Frame mehr im Puffer
    timerset();
  }
#ifndef __FLAT__
  disable();
  _SS = stacksegbuf; // Alten Stack wiederherstellen
  _SP = stackptrbuf;
#endif
  rekfl = FALSE;
  enable();
}

/*---------------------------------------------------------------------------*/

void set_paclen (unsigned short paclen)
//*************************************************************************
//
// Setzt PACLEN
//
//*************************************************************************
{
  p->paclen = paclen;
}
