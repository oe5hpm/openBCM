/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------
  Multitasking-Scheduler
  ----------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#include "baycom.h"

//19980202 OE3DZW added " " in "users" for fbb
//19980301 hrx    added dashes to ps-command
//19980307 OE3DZW moved "t->port=0" to position after disconnect ->
//                fwdlog will show port of logout
//19980311 DG9MHZ changes for support of Linux AX25 Kernel
//19980329 OE3DZW ax25kshutdown only for linux
//19980420 OE3DZW removed dashes after protests of flori,deti,chris and me
//19980428 DH3MB  update of mailserver for dos (task cloning)
//19990323 OE3DZW changed define for ax25kshutdown (was linux)
//19990422 OE3DZW removed some debug tjob code, left 92/93
//19991028 Jan    fixed parm handling for new tasks
//19991231 OE3DZW cleaned up eingloggt (logged-in)
//19991118 DF3VI  count also sysop-logins in eingelogt() (needed for convers)
//20000107 DF3VI  invented user-timeout
//20000106 DF3VI  fixed tabs on ps (listprocess())
//20000716 DK2UI  changed shadow variables and make code better readable
//20021130 DH8YMB listprocess: opt & o_a: Laenge auf 12
//20031109 DB1RAS added stack overflow detection

/*---------------------------------------------------------------------------*/

// global sichtbare Variablen
//
task_t *tfeld[TASKS]; // Zeiger auf die Task-Kontrollbloecke
task_t *t = NULL;     // Zeiger auf aktuellen TCB
tvar_t *b;
user_t *u;
int allupdate = 0; // gesetzt wenn alle Fenster neu geschrieben werden
int newkeywin = 0; // enthaelt neues Fenster, Umschaltung im Scheduler
int deskkey = 0;   // enthaelt Taste, die vom Desktop ausgewertet wird.
int keytaskid = 0; // Task-Nummer, zu der die Tastatur geleitet wird.
int init_off = 0;  // Flag zum stoppen des Schedulers

int runterfahren = 0;
char *stopreason = " ??";
char abortreason[60];

/*---------------------------------------------------------------------------*/

// Scheduler lokale Variablen
//
static int taskrunning = NOTASK; // Index fuer laufende Task
static jmp_buf jb;
static int waitstack[TASKS+1];   // Warteschlange fuer laufende Prozesse
static unsigned int waitsp = 0;  // Zeiger in Warteschlange
static int nextkill = 0;         // task-id die geloescht werden soll.
static unsigned updtic = 0;      // Timer fuer zwangsweisen Fenster-update
static unsigned alltic = 0;      // Timer fuer zwangsweisen Bildschirm-update

/*---------------------------------------------------------------------------*/

// Parameterblock fuer neue Tasks. Diese muessen statisch gespeichert werden,
// damit sie das Umschalten des Stack-Kontextes ueberleben
static int      newtask;          // Flag: Task muss erzeugt werden
static int     *newtasknum;       // Pointer auf TID von neu erzeugter Task
static int      newport;          // ggf. TNC-Verbindug erzeugen
static unsigned newkind;          // Bitfeld fuer Task-Resourcen
static void     (*start)(char *); // Zeiger auf Startfunktion
static char     *param;           // Zeiger auf Parameter der Startfunktion
static task_t   *newtaskclone;    // Zeiger auf TCB des zu clonenden Tasks

static unsigned tjob = 0; // Merker fuer Stelle, die momentan bearbeitet wird
                          // (Dient nur zum Debugging)

static unsigned int maxstackdepth = TASKSTACK; // Schleppzeiger fuer Stacktiefe

/*---------------------------------------------------------------------------*/

void t_lock (void)
//**************************************************************************
//
//  Sperrt einen Prozess gegen Kill und Taskumschaltung
//
//**************************************************************************
{
  if (t) t->lock++;
}

/*---------------------------------------------------------------------------*/

void t_unlock (int force)
//*************************************************************************
//
//  Gibt einen Prozess wieder zum Kontextwechsel frei
//
//*************************************************************************
{
  if (t)
  {
    if (force)
      t->lock = 0;
    else
    {
      if (t->lock)
        t->lock--;
      else
        trace(serious, "t_unlock", "no lock %d \"%s\"", t->taskid, t->name);
    }
  }
}

/*---------------------------------------------------------------------------*/

void suicide (void)
//*************************************************************************
//
//  killt den momentan laufenden Prozess
//
//*************************************************************************
{
  //lastfunc ("suicide");
  if (taskrunning != NOTASK)
  {
    while (nextkill) wdelay(249); // Warten bis Funktion frei ist
    nextkill = taskrunning;
    wdelay(24096); // Rescheduling erzwingen (kommt nie zurueck)
  }
}

/*---------------------------------------------------------------------------*/

char *gettaskname (int tid)
//*************************************************************************
//
//
//*************************************************************************
{
  if (tid > 0 && tid < TASKS)
  {
    if (tfeld[tid])
      return tfeld[tid]->name;
  }
  return "SYSTEM";
}

/*---------------------------------------------------------------------------*/

int gettaskid (void)
//*************************************************************************
//
//  momentane TASK-ID zurueckliefern (zur Kapselung)
//
//*************************************************************************
{
  return taskrunning;
}

/*---------------------------------------------------------------------------*/

static void near inittcb (int taskid)
//*************************************************************************
//
//  allokiert einen Task-Kontrollblock
//  besetzt den Stack-Bereich mit 0xaa vor, um
//  im Betrieb die Stacktiefe feststellbar zu machen.
//
//*************************************************************************
{
  task_t *tt;
  int size = sizeof(task_t);
  if (newkind & P_MAIL)
    size = sizeof(tvar_t); // Wenn Mailbox, dann erweiterten TCB
  tfeld[taskid] = (task_t *) t_malloc(size, "task");
  tt = tfeld[taskid];
  if (newtaskclone)
  {
    memcpy(tt, newtaskclone, size); // Copy TCB
    tt->port = 0;
    tt->socket = 0;
    tt->processkind &= ~ P_AX25K;
    tt->input = io_dummy;
    tt->output = io_dummy;
    newtaskclone = 0;
  }
  else memset(tt, 0, size); // definiert vorbesetzen
  memset(tt->stack, 0xaa, TASKSTACK);
  tt->stack[TASKSTACK] = 0;
  tt->taskid = taskid;
}

/*---------------------------------------------------------------------------*/

int taskkbhit (void)
//*************************************************************************
//
//  Gibt einen Tastendruck abhaengig vom Task-Kontext zurueck
//
//*************************************************************************
{
  if (t && (t->keybufin != t->keybufout))
  {
    if (t->keybuffer[t->keybufout] & 255)
      return t->keybuffer[t->keybufout] & 255;
    else
      return t->keybuffer[t->keybufout] >> 8;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

int iskeytask (void)
//*************************************************************************
//
//  Gibt zurueck, ob momentanes Fenster die Tastatur besitzt
//
//*************************************************************************
{
  if (t) return keytaskid == t->taskid;
  else return 0;
}

/*---------------------------------------------------------------------------*/
#ifdef __DOS16__
int keywindow (void)
//*************************************************************************
//
//  Gibt Window-ID des momentan mit der Tastatur assozierten Fensters zur.
//
//*************************************************************************
{
  if (tfeld[keytaskid])
  {
    struct edstruct *ee = (edstruct *) tfeld[keytaskid]->e;
    return ee->wa;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

static void near setkeytask (int tid)
//*************************************************************************
//
//  Weist die Tastatur einer Task-ID zu. Fenster kommt in den Vordergrund
//
//*************************************************************************
{
  if (tfeld[tid])
  {
    edstruct *ee = (edstruct *) tfeld[tid]->e;
    if (ee)
    {
      keytaskid = tid;
      nachvorne(ee->wa);
      setzcurs(ee->curx, ee->cury, ee);
      allupdate++;
      return;
    }
  }
}

/*---------------------------------------------------------------------------*/

int setkeywindow (int fenster)
//*************************************************************************
//
//  Weist die Tastatur einer Window-ID zu. Fenster kommt in den Vordergrund
//
//*************************************************************************
{
  unsigned int i;
  for (i = 1; i < TASKS; i++)
  {
    if (tfeld[i])
    {
      edstruct *ee = (edstruct *) tfeld[i]->e;
      if (ee && (ee->wa == fenster))
      {
        keytaskid = i;
        nachvorne(ee->wa);
        setzcurs(ee->curx, ee->cury, ee);
        allupdate++;
        return 1;
      }
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void setkeyhere (void)
//*************************************************************************
//
//  Legt Tastatureingabe auf laufende Task um
//  Beim fork() muss die Tastatur-Berechtigung P_KEYB eingeschaltet sein
//
//*************************************************************************
{
  if ((taskrunning != NOTASK) && (t->processkind & P_KEYB))
    setkeytask(t->taskid);
}

/*---------------------------------------------------------------------------*/

static int near scankey (void)
//*************************************************************************
//
//  Holt Taste und schaut, ob ALT-Tastenkombination gedrueckt ist
//  Wenn nein, wird die Taste im zusaetzlichen Tastaturpuffer abgelegt
//
//*************************************************************************
{
  int alt;
  unsigned key;
  task_t *t;

  tjob = 1;
  while ((ad_peek(0x40, 0x1a) != ad_peek(0x40, 0x1c)) && bioskey(1))
  {
    video_switchon();
    key = bioskey(0);
    alt = f_taste(key);
    if (alt)
    {
      if (alt < 11)
      {
        if (! deskkey && ! boxopen) setkeywindow(alt);
      }
      else
        deskkey = alt;
    }
    else
    {
      alt = alt_taste(key);
      if (alt)
      {
        if (alt == 'K')
          return 1;
        else
          if (! deskkey) deskkey = alt;
      }
      else
      {
        if (deskkey) t = tfeld[0];
        else t = tfeld[keytaskid];
        if (((t->keybufin + 1) % 32) != t->keybufout)
        {
          t->keybuffer[t->keybufin] = key;
          t->keybufin = (t->keybufin + 1) % 32;
        }
      }
    }
  }
  if (newkeywin)
  {
    if (! setkeywindow(newkeywin)) setkeywindow(1);
    newkeywin = 0;
  }
  return 0;
}
#endif
/*---------------------------------------------------------------------------*/

static int near testevent (event_t event)
//*************************************************************************
//
//  Prueft auf das Eintreten eines Ereignisses. Es wird lediglich
//  der Zustand geprueft, aber sonst nichts unternommen.
//
//  return: 0=Event nicht eingetreten, >0 Event eingetreten
//
//*************************************************************************
{
  tjob = 2;
  switch (event)
  {
    case e_kbhit: return taskkbhit();
    case e_ticsfull: return TRUE;
    default: return FALSE;
  }
}

/*---------------------------------------------------------------------------*/

static void global_process (void)
//*************************************************************************
//
//  legt den Stack nach Aufruf von waitfor() wieder
//  zum Hauptprogramm um. Die Funktion ist als interrupt
//  deklariert, damit alle Register vor dem Umschalten auf
//  dem Stack gesichert werden.
//
//*************************************************************************
{
  if (! setjmp(t->j))
  {
    /*
    if (heapcheck() < 0)
    {
      post_mortem_dump();
      trace(tr_abbruch, "global_process", "heap corrupted");
    }
    */
#ifdef __DOS16__
    if (e)
    {
      e->buf = buf;
      e->wa = wa;
    }
    t->e = e;
#endif
    t->b = b;
    taskrunning = NOTASK;
    t = NULL;
    tjob = 4;
    longjmp(jb, 1);
  }
}

/*---------------------------------------------------------------------------*/

int waitfor (event_t event)
//*************************************************************************
//
//  setzt das laufende Programm aus, bis der Event im Argument
//  eingetreten ist. Ist die Bedingung bereits beim Einsprung
//  in waitfor erfuellt, so wird ganz normal weitergemacht, ohne
//  zu unterbrechen.
//  Tritt waehrend der Warteposition ein Disconnect auf dem
//  korrespondierenden Port auf, so wird der Prozess geloescht,
//  und ein offenes File geschlossen
//
//*************************************************************************
{
//  lastfunc ("waitfor");
  static unsigned int i; // 'lokale Variablen', nicht am Stack!!

  if (taskrunning == NOTASK) return OK;
  if (   (unsigned) systic < (tfeld[taskrunning]->begintic + m.timeslot)
      && testevent (event))
    return OK; // Event ist bereits gesetzt
  if (tfeld[taskrunning]->lock) return OK;
  for (i = waitsp; i; i--)           // Prozess in die Warteschlange
    waitstack[i + 1] = waitstack[i]; // aufnehmen. Warteschlange aufruecken
  waitsp++;
  waitstack[1] = taskrunning; // vorne ist die niedrigste Prioritaet
  if (event == e_reschedule) event = e_ticsfull;
  t->event = event;
  t->cputics += (unsigned) ((unsigned) systic - (unsigned) t->begintic);
  t->begintic = (unsigned) (systic + 1); // Zeit zum Aussetzen speichern
  tjob = 5;
  global_process();
  tjob = 6;
  return NO;
}

/*---------------------------------------------------------------------------*/
#ifdef __DOS16__
unsigned waitkey (void)
//*************************************************************************
//
//  suspendiert einen laufenden Prozess, bis eine Taste gedrueckt wird und
//  das zugehoerige Fenster dem momentenan Tastaturfenster entspricht
//
//*************************************************************************
{
  unsigned key;

  waitfor(e_kbhit);
  key = t->keybuffer[t->keybufout];
  t->keybufout = (t->keybufout + 1) % 32;
  mausaus();
  mausein();
  return key;
}
#endif
/*---------------------------------------------------------------------------*/

int wdelay (unsigned zeit)
//*************************************************************************
//
//  suspendiert einen laufenden Prozess fuer die angegebene Zeit
//  (in Millisekunden) Die tatsaechlich vergangene Zeit ist minimal gleich
//  bzw. groesser als die angegebene Zeit.
//
//*************************************************************************
{
//  lastfunc ("wdelay");
  static unsigned int i; // 'lokale Variablen', nicht am Stack!!

  if (taskrunning == NOTASK || t->lock) return OK;
  tjob = 23;
  t->cputics += (unsigned) ((unsigned) systic - (unsigned) t->begintic);
  t->begintic = (unsigned) (systic + (zeit / MS_PER_TIC) + 1);
  t->event = e_delay;
  t->lastdelay = zeit;
  for (i = waitsp; i; i--)           // Prozess in die Warteschlange
    waitstack[i + 1] = waitstack[i]; // aufnehmen. Warteschlange aufruecken
  waitsp++;
  waitstack[1] = taskrunning; // vorne ist die niedrigste Prioritaet
  tjob = 7;
  global_process();
  tjob = 8;
  return NO;
}

/*---------------------------------------------------------------------------*/

static void dealloctask (int tid, int discon_) //unterscheide kill/disconnect
//*************************************************************************
//
//  Loescht eine komplette Task-Umgebung. Diese Routine ist das Herz von
//  jedem kill()-Aufruf und wird auch nach einer regulaeren Beendigung
//  aufgerufen.
//
//*************************************************************************
{
  task_t   *tt = t; // momentanen Kontext sichern
  user_t   *uu = u;
#ifdef __DOS16__
  edstruct *ee = e;
  char   *buff = buf;
  int      waa = wa;
#endif
  tvar_t   *bb = b;
  int     trun = taskrunning;

  tjob = 9;
  taskrunning = NOTASK;
  t = tfeld[tid];
  if (t != 0)
  {
    lastfunc("dealloctask");
    unsigned int j = 0;
    t_lock();
    while ((char) (t->stack[j]) == 0xaa) j++; //belegte Stacktiefe feststellen
    if (j < maxstackdepth) maxstackdepth = j;
#ifdef __DOS16__
    e = (edstruct *) t->e;         // Window-Struktur feststellen
#endif
    b = (tvar_t *) t->b;           // Feststellen, ob Box-Kontrollblock
    if (b)
      u = &b->u;                   // Userdaten-Kontext herstellen
    for (j = 0; j < t->atkillcnt; j++)
      (*t->atkill[j]) (t->atkill_p[j]);
#ifdef __DOS16__
    if (e)
    {
      buf = e->buf;                // Window-Kontext herstellen
      wa = e->wa;
      if (wa)
        wclose();                  // Fenster schliessen
    }
#endif
    if (t->port) mbdisconnect(0);  // ist eine AX25-Connection aktiv? ... trennen.
#ifdef __FLAT__
    if (t->socket) disconnect_sock();
#ifdef __UNIX__
    if (t->childpid) kill(-(t->childpid), 9);
#endif
#ifdef _WIN32
    if (t->childpid) _beginthread(kill, 0, (void*) t->childpid);
#endif
#endif
    tjob = 92;
    //t->port = 0;
    if (b && ! (t->processkind & P_CLON))
    {
      wdelay(2048); // db7mh
      //trace(report, "debug", "mblogout aus task.cpp:585");
      mblogout(1 + discon_); // oe3dzw: ordentlichen Logout durchfuehren
    }
    tjob = 93;
    t->port = 0;
    t->event = e_nothing;       // kein Wartezustand mehr.
    close_task(tid);            // alle Files und Semaphoren schliessen
    t_allfree(tid);             // alle Speicherbloecke freigeben
    tfeld[tid] = NULL;          // TCB ist nun inaktiv.
    for (j = 0; j < TASKS; j++) // ggf. Monitor-Server schliessen.
    {
      if (tfeld[j] && tfeld[j]->monitor)
      {
        monbuf_t *mm = tfeld[j]->monitor;
        if (mm->tid == tid) tfeld[j]->monitor = NULL;
      }
    }
  }
  t = tt;                       // laufenden Kontext wiederherstellen
  b = bb;
#ifdef __DOS16__
  e = ee;
  buf = buff;
  wa = waa;
#endif
  u = uu;
  taskrunning = trun;
  tjob = 10;
//  trace(report, "debug", "hier");
}

/*---------------------------------------------------------------------------*/

static void start_newprocess (void)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("start_newprocess");

  if (param)
  {
    char *prm = (char *) t_malloc(strlen(param) + 1, "parm");
    //t_malloc param so it will be freed when task is killed
    strcpy(prm, param);
    free(param); //free copy from strdup
    param = prm;
  }
  t->begintic = (unsigned) systic;
  t->processkind = newkind;
  if (newport > 0)
  {
    t->port = newport;
    t->input = io_tnc;
    t->output = io_tnc;
  }
#ifdef __FLAT__
  else if (newport < 0)
  {
    t->socket = (-newport);
    t->input = t->output = io_socket;
  }
#endif
  else
  {
    t->input = io_dummy;
    t->output = io_dummy;
  }
  t->created = loctime();
#ifdef __DOS16__
  t->e = e = NULL;
#endif
  t->b = b = NULL;
  strncpy(t->name, param, 15);
  t->name[15] = 0;
#ifdef __DOS16__
  if (newkind & P_WIND)
  {
    e = (edstruct *) t_malloc(sizeof(edstruct), "edst");
    memset(e, 0, sizeof(edstruct));
    e->frame = 1;
    t->e = e;
    t->input = t->output = io_window;
  }
#endif
  if (newkind & P_MAIL)
  {
    b = (tvar_t *) t;
    t->b = b;
    u = &b->u;
    b->job = j_idle;
  }
  tjob = 11;
  (*start)(param); // Prozess starten
  tjob = 12;
  suicide();
  trace(serious, "start_newprocess", "%d not terminated", taskrunning);
}

/*---------------------------------------------------------------------------*/

static void newprocess (void)
//*************************************************************************
//
//  startet einen Prozess, der bisher noch keinen Stack reserviert
//  hatte. Dabei muss in taskrunning bereits eine Task-Nummer
//  vermerkt sein, die auf einen freien TASK-Kontrollblock zeigt.
//
//*************************************************************************
{
//lastfunc("newprocess");
  if (! setjmp(jb))
  {
    t = tfeld[taskrunning];
    ad_setsp(t->stack + TASKSTACK); // Stackpointer umschalten (makro)
    start_newprocess();
    longjmp(jb, 1);
  }
  t_checkfree();
}

/*---------------------------------------------------------------------------*/

void atkill (atkill2_t func, void *p)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("atkill2");

  if (t->atkillcnt == MAX_ATKILL)
  {
    trace(serious, "atkill", "table full");
    return;
  }
  t->atkill[t->atkillcnt] = func;
  t->atkill_p[t->atkillcnt++] = p;
}

/*---------------------------------------------------------------------------*/

void atkill (atkill_t func)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("atkill");

  if (t->atkillcnt == MAX_ATKILL)
  {
    trace(serious, "atkill", "table full");
    return;
  }
  t->atkill[t->atkillcnt] = (atkill2_t) func;
  t->atkill_p[t->atkillcnt++] = NULL;
}

/*---------------------------------------------------------------------------*/

int fork (int typ, int port, void(*stfunc)(char *), char *stpar)
//*************************************************************************
//
//  Verzweigt zu einem neuen Prozess. Dieser laeuft so lange, bis entweder
//  ein KILL kommt oder sich das angegebene Programm beendet.
//
//*************************************************************************
{
  int newtasknum_local = 0;

  if (   (typ & (P_CLON | P_MAIL)) == (P_CLON | P_MAIL)
      && (t->processkind & P_MAIL))
    newtaskclone = t;
  else
    newtaskclone = NULL;
  start = stfunc;
  param = (! stpar ? (char *) NULL : strdup(stpar));
  newkind = typ;
  newport = port;
  newtask = 1;
  newtasknum = &newtasknum_local;
  waitfor(e_reschedule);
  return newtasknum_local;
}

/*---------------------------------------------------------------------------*/

static void near showtasknum (int tnum, char job)
//*************************************************************************
//
//  zeigt am oberen Bildschirmrand den Prozesstatus an
//
//*************************************************************************
{
  lastjob = job;
#ifdef __DOS16__
  if (video_off) return;
  static unsigned shtic = 0;
  static int lasttnum = 0;
  int wasave = wa;
  wa = 0;
  if (! shtic || (((int) ((unsigned) systic - shtic)) > 0))
  {
    wf(0, 0, 7, "%30s", memfree (0));
    shtic = (unsigned) (systic + 8);
  }
  video_putc(lasttnum, 0, 7, ' ');
  if (tnum < 47)
  {
    video_putc(tnum, 0, 7, job);
    lasttnum = tnum;
  }
  wa = wasave;
#endif
}

/*---------------------------------------------------------------------------*/

int killtask (int tid, int discon_) //unterscheide kill/disconnect/logout
//*************************************************************************
//
//  Loescht den angegebenen Prozess,
//  gibt allen belegten Speicher frei und schliesst ggf. offene Files
//  Ist ein Mailbox-Login vorhanden, so wird ein ordentlicher LOGOUT
//  veranlasst.
//
//*************************************************************************
{
  lastfunc("killtask");
  unsigned int j, i = waitsp;

  if (newtaskclone == t)
    newtaskclone = 0; //DH3MB: Zu klonende Task wird gekillt
  tjob = 14;
  i = waitsp;
//  trace(report, "kill", "task %d", tid);
  while (i)
  {
    if (tid == waitstack[i])
    {
      for (j = i; j < waitsp; j++) // Task aus der Warteschlange entfernem
        waitstack[j] = waitstack[j + 1];
      if (waitsp) waitsp--;
      if (tfeld[tid]) dealloctask(tid, discon_);
      tjob = 15;
      t_checkfree();
//  trace(report, "kill", "hier %d", i);
      return OK;
    }
    i--;
  }
//  trace(report, "kill", "%d not found", tid);
  tjob = 16;
  return NO;
}

/*---------------------------------------------------------------------------*/

int killport (int port)
//*************************************************************************
//
//  Loescht einen Prozess mit angegebenem Funkport. Wird bei erkanntem
//  Disconnect aufgerufen
//
//*************************************************************************
{
  if (port)
  {
    for (unsigned i = 0; i < TASKS; i++)
    {
      if (tfeld[i] && (tfeld[i]->port == port))
        return killtask(i, 1); //unterscheide kill/discon/logout
    }
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

int taskport0 (int port)
//*************************************************************************
//
//  Teilt einem Prozess die Portnummer 0 zu
//
//*************************************************************************
{
  unsigned int i;

  if (port)
  {
    for (i = 0; i < TASKS; i++)
    {
      if (tfeld[i] && (tfeld[i]->port == port))
      {
        tfeld[i]->port = 0;
        return OK;
      }
    }
  }
  trace(report, "taskport0", "%d not found", port);
  return NO;
}

/*---------------------------------------------------------------------------*/

void maintask (void)
//*************************************************************************
//
//  wird vom Hauptprogramm aufgerufen. Hier wird nachgesehen, ob eine
//  Anforderung in der Vorbereiteschlange (scanstack) ansteht, ggf.
//  wird ein neuer Prozess gestartet.
//
//*************************************************************************
{
  lastfunc("maintask");
  unsigned int i, j;
  int active;
//  event_t ev;
  event_t ev = e_nothing;
#ifndef _AX25K_ONLY
  extern void call_l2(int);
#endif
  memset(&m, 0, sizeof(m));
  sema_init();
#ifdef __FLAT__
  call_l2(0);
#endif
  randomize(); //sonst jedesmal gleiche Zufallsfolge,
               //wenn der Sysop immer als erstes einloggt...
  set_watchdog(1);
  updtic = (unsigned) systic;
  alltic = (unsigned) systic;
  fork(P_WIND, 0, desktop, "Desktop"); // Erste Task, alles andere wird von
                                       // dort aus gestartet.
  while (newtask || waitsp)
  {
    active = 0;
    if (newtask)                  // Ist eine Anforderung vorhanden?
    {
      tjob = 17;
      active = 1;
      ev = e_nothing;
      for (i = 0; i < TASKS; i++) // freien TASK-Block suchen
      {
        if (! tfeld[i])
        {
          taskrunning = i;        // dieser Block wird genommen
          *newtasknum = i;
          ev = e_reschedule;
          inittcb(i);
          break;
        }
      }
      if (ev == e_nothing)
        trace(serious, "maintask", "tasks full");
      else
      {
        newtask = 0;
        showtasknum(taskrunning, 'N');
        if (taskrunning != NOTASK)
          newprocess(); // war noch einer frei? und starten
      }
    }
    else
    {
      tjob = 18;
      i = waitsp;
      while (i)
      {
        t = tfeld[waitstack[i]];
        ev = t->event;
        if (ev == e_delay && (((int) ((unsigned) systic - t->begintic)) >= 0))
          break; // Delay Vorrang
        if (testevent(ev) || ! ev) break;
        i--;
      }
      t = NULL;
      if (i) // wurde einer mit gueltigem Event gefunden?
      {
        tjob = 19;
        active = 1;
        taskrunning = waitstack[i];
        for (j = i; j < waitsp; j++) // Task aus der Warteschlange entfernen
          waitstack[j] = waitstack[j + 1];
        if (waitsp) waitsp--;
        if (ev)
        {
          showtasknum(taskrunning, 'R');
          if (! setjmp(jb))
          {
            t = tfeld[taskrunning]; // Ausgesetzten Prozess fortsetzen
            t->begintic = (unsigned) systic;
#ifdef __DOS16__
            e = (edstruct *) t->e;
            if (e)
            {
              buf = e->buf;
              wa = e->wa;
            }
#endif
            b = (tvar_t *) t->b;
            if (b) u = &b->u;
            tjob = 3;
            longjmp(t->j, 1);
          }
          t_checkfree();
        }
        else
        {
          taskrunning = NOTASK;
          trace(serious, "maintask", "no event");
        }
      }
#ifdef __DOS16__
      if (! video_off)
      {
        if (allupdate && ! boxopen
            && (! alltic || (((int) ((unsigned) systic - alltic)) > 0)))
        {
          tjob = 20;
          active = 1;
          updtic = (unsigned) (systic + 5);
          alltic = allupdate = 0;
          vorne_reorg ();
          for (i = 0; i <= waitsp; i++) // Alle Fenster updaten
          {
            t = tfeld[waitstack[i]];
            if (t)
            {
              showtasknum(waitstack[i], 'U');
              taskrunning = NOTASK;
              if (t->e)
              {
                e = (edstruct *) t->e;
                wa = e->wa;
                buf = e->buf;
                if (e->update)
                {
                  if (! (*e->update)(1))
                  {
                    allupdate++;
                    alltic = (unsigned) (systic + 8);
                  }
                }
              }
            }
          }
          mausaus();
          video_update();
          mausein();
          t = NULL;
        }
        else
          if (((int) ((unsigned) systic - updtic)) > 0)
          {
            tjob = 21;
            active = 1;
            int updates = 0;
            updtic = (unsigned) (systic+(m.scrolldelay ? m.scrolldelay : 2));
            for (i = 0; i <= waitsp; i++) // Alle Fenster updaten
            {
              t = tfeld[waitstack[i]];
              taskrunning = NOTASK;
              if (t)
              {
                e = (edstruct *) t->e;
                if (e && t->event && e->update && e->shouldupdate)
                {
                  e->shouldupdate = 0;
                  updates++;
                  showtasknum(waitstack[i], 'M');
                  wa = e->wa;
                  buf = e->buf;
                  if (! (*e->update)(0)) e->shouldupdate++;
                  t = NULL;
                }
              }
            }
            if (updates)
            {
              mausaus();
              video_update();
              mausein();
            }
          }
      }
#endif
    }
#ifdef __DOS16__
    if (scankey()) break; // Tastatur abfragen und bei ALT-K hart abbrechen
#endif
    if (nextkill)
    {
      killtask(nextkill, 0); // unterscheide kill/discon/logout
      nextkill = 0;
    }
    showtasknum(0, 'D');
    tjob = 22;
    task_idle(active + (init_off ? 2 : 0));
    showtasknum(0, 'I');
    if (init_off) break;
  }
}

/*---------------------------------------------------------------------------*/

static int convin (int tid, tvar_t *bb, char *fromcall, char *msg, int first)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("convin");
  conv_t *newc = (conv_t *) t_malloc(sizeof(conv_t), "conv");
  int pending = 1;
  t_settaskid(newc, tid);
  newc->next = NULL;
  if (first)
  {
    if (strlen(msg) > 68)
      snprintf(newc->line, sizeof(newc->line), "*%s*:\n%s", fromcall, msg);
    else
      snprintf(newc->line, sizeof(newc->line), "*%s*: %s", fromcall, msg);
  }
  else
    snprintf(newc->line, sizeof(newc->line), "%s", msg);
  if (bb->conv)
  {
    conv_t *cp = bb->conv;
    while (cp->next)
    {
      cp = cp->next;
      pending++;
    }
    pending++;
    if (pending < 20) cp->next = newc;
    else t_free(newc);
  }
  else bb->conv = newc;
  return pending;
}

/*---------------------------------------------------------------------------*/

int mbtalk (char *fromcall, char *tocall, char *message)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("mbtalk");
  unsigned int i;
  task_t *tt;
  tvar_t *bb;
  int pending = 0;
  int first = 1;
  int toall;
  char *current_msg = message;
  char *next_msg = NULL;

  strupr(fromcall);
  strupr(tocall);
  toall = ! stricmp(tocall, "ALL");
  while (current_msg)
  {
    next_msg = NULL;
    if (strlen(current_msg) > LINELEN)
    {
      next_msg = current_msg + LINELEN - 1;
      i = LINELEN - 1;
      while (i && current_msg[i] != ' ') i--;
      if (i) next_msg = current_msg + i;
      *next_msg++ = 0;
    }
    for (i = 0; i < TASKS; i++)
    {
      tt = tfeld[i];
      if (tt && tt->b && (i != (unsigned) taskrunning))
      {
        bb = (tvar_t *) tt->b;
        if (! strcmp(bb->logincall, tocall) || toall)
          pending = convin(i, bb, fromcall, current_msg, first);
      }
    }
    first = 0;
    if (next_msg) current_msg = next_msg;
    else current_msg = NULL;
  }
  return pending;
}
/*---------------------------------------------------------------------------*/

void test_killtime (void)
//*************************************************************************
//
//
//*************************************************************************
//WICHTIG: timeout-funktionen muessen NACH MBTALK/CONVIN stehen!
{
  lastfunc("test_killtime");
  unsigned int i;
  task_t *tt;
  tvar_t *bb;
  time_t act_time = ad_time();
  for (i = 0; i < TASKS; i++)
  {
    if (tfeld[i])
    {
      tt = tfeld[i];
      bb = (tvar_t *) tt->b;
      if (tt->event == e_ticsfull) continue;
      if (bb)
      {
        if (bb->sysop) continue;
        if (bb->forwarding != fwd_none)
        {
          switch (bb->job) // "*" aus PS-anzeige
          {
            case j_idle:
            case j_send:
            case j_fbb_send_propose:
            case j_fbb_send_delay:
            case j_fbb_recv_mail:
            case j_fbb_recv_propose:
            case j_fbb_end:
            case j_fbb_error:
            case j_fwdlink:
            case j_fwdrecv: break;
            default: continue;
          }
        }
        else
        {
          switch (bb->job)
          {
            case j_idle:
            case j_send: break;
            default: continue;
          }
        }
      }
      if (m.timeoutwarning) // 0=ohne Vorwarnung, 1=mit Vorwarnung
      {
        if (! tt->kill_warn && tt->warn_time && act_time > tt->warn_time)
        {
          if (bb)
            convin(i, (tvar_t *) tt->b, "SYSTEM", "*** Timeout warning!\n",
                                        1);
          tt->kill_warn = 1; // einmal Vorwarnung senden
        }
        if (tt->kill_warn && tt->warn_time && act_time < tt->warn_time)
          tt->kill_warn = 0;
      }
      if (tt->kill_time && act_time > tt->kill_time)
      {
        trace(report, "timeout", "%s: \"%s\"", tt->name, tt->lastcmd);
        if (bb)
          convin(i, (tvar_t *) tt->b, "SYSTEM", "*** TIMEOUT\n", 1);
        wdelay(2409);
        killtask(i, 0);
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void timeout (int minutes) //df3vi: user-timeout eingebaut
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("timeout");
  if (minutes)
  {
    time_t adtime = ad_time();
    t->kill_time = adtime + 60 * minutes;
    t->warn_time = adtime + 50 * minutes;
  }
  else
  {
    t->kill_time = 0;
    t->warn_time = 0;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef _TELEPHONE // JJ
int taskexists (int tid)
//*************************************************************************
//
//
//*************************************************************************
{
  task_t *tt;
  tt = tfeld[tid];
  if (tt) return OK;
  else return NO;
}
#endif
/*---------------------------------------------------------------------------*/

int monitor (int tid, bitfeld opt)
//*************************************************************************
//
//
//*************************************************************************
{
  lastfunc("monitor");
  task_t *tt;
  monbuf_t *mm;
  int c = 0;

  tt = tfeld[tid];
  if (! tt || tid == gettaskid())
  {
    putf(ms(m_invalidtaskid));
    return NO;
  }
  if (tt->monitor)
  {
    putf(ms(m_taskmonbusy));
    return NO;
  }
  mm = (monbuf_t *) t_malloc(sizeof(monbuf_t), "monb");
  tt->monitor = mm;
  mm->tid = gettaskid();
  mm->in = 0;
  mm->out = 0;
  mm->congested = 0;
  mm->opt = opt;
  putf(ms(m_montask), tid, tt->name);
  while (! getvalid() && tfeld[tid])
  {
    while (mm->in != mm->out)
    {
      c = mm->buf[mm->out++];
      if (mm->out >= MONBUFLEN) mm->out = 0;
      putv(c);
    }
    if (mm->congested)
    {
      putf("\n");
      putf(ms(m_taskbuffer));
      c = 0;
      break;
    }
    wdelay(274);
  }
  if (c != LF) putv(LF);
  if (! tfeld[tid])
    putf(ms(m_taskmondied));
  t_free(mm);
  if (tfeld[tid]) tfeld[tid]->monitor = NULL;
  return OK;
}

/*---------------------------------------------------------------------------*/

unsigned int eingelogt (char *logincall, int job, int count)
//*************************************************************************
//
//
//*************************************************************************
//does not ignore via_radio logins when !count
{
  unsigned int i;
  task_t *tt;
  tvar_t *bb;
  unsigned int logins = 0;

  for (i = 0; i < TASKS; i++)
  {
    tt = tfeld[i];
    if (tt && tt->b && i != (unsigned) taskrunning)
    {
      bb = (tvar_t *) tt->b;
      if (! strcmp(bb->logincall, logincall) && (! job || job == bb->job))
      {
        if (! count) return 1;
        if (bb->via_radio) logins++;
      }
    }
  }
  return logins;
}

/*---------------------------------------------------------------------------*/

void listprocess (long opt)
//*************************************************************************
//
//  gibt die Task-Kontrollbloecke aller Prozesse aus.
//
//*************************************************************************
{
  lastfunc("listprocess");
  unsigned int i, j, width = 45;
  task_t *tt;
  char *evstr;
  char evdummy[] = "     ";

  putf("Task ");
  if (opt & o_t) { width -= 3;  putf("TP "); }
  if (opt & o_a) { width -= 12; putf("Event  Kdeb "); }
  putf(" CPUt Create Input Name    ");
  if (opt & o_k) { width -= 6;  putf("Killt "); }
  if (opt & o_n) { width -= 14; putf("User ID       "); }
  if (opt & o_m) { width -= 8;  putf("Mycall   "); }
  if (opt & o_u) { width -= 10; putf("Uplink    "); }
  if (opt & o_d) { width -= 14; putf("Downlink       "); }
  if (opt & o_c) { width -= 6;  putf("Bytes "); }
  if (opt & o_r) { width -= 8;  putf("Board   "); }
  if (! (opt & (o_u | o_r | o_m))) putv(' ');
  putf(" Command\n");
  putf(SEPLINE "\n");
  for (i = 0; i < TASKS; i++)
  {
    tt = tfeld[i];
    if (tt)
    {
      tvar_t *bb = (tvar_t *) tt->b;
      if (bb || (opt & o_b))
      {
        if (bb && bb->sysop) putv('s');
        else if (bb && bb->pwok && b->sysop) putv('p');
        else putv(' ');
//      putf("%c%3d ", (tt->taskid == taskrunning) ? '*' : ' ', i);
        putf("%3d ", i);
        if (opt & o_t) putf("%2d ", tt->port);
        if (opt & o_a)
        {
          switch (tt->event)
          {
            case e_ticsfull:    evstr = "slice";   break;
            case e_delay:       evstr = evdummy;
                                sprintf(evstr, "%u", tt->lastdelay);
                                break;
            case e_kbhit:       evstr = "kbhit";   break;
            default:            evstr = "other";
          }
          putf("%-6s%3d%d%d ", evstr, tt->processkind, !! tt->e, !! tt->b);
        }
        putf("%5s ", zeitspanne(tt->cputics, zs_cputime));
        //trace(serious, "created", "%lu", t->created);
        putf("%6s",  zeitspanne(loctime() - tt->created, zs_seconds));
        putf("%6s ", zeitspanne(ad_time() - tt->last_input, zs_seconds));
        if (*tt->name && strstr(tt->name, "."))
          putf("(system)");
        else
          putf("%-8.8s", tt->name);
        if (opt & o_k)
        {
          if (tt->kill_time)
            putf("%5s ", zeitspanne(tt->kill_time - ad_time(), zs_seconds));
          else
            putf("      ");
        }
        if (opt & o_n)
        {
          if (mbcallok(tt->name))
          putf("%-14.14s", get_name(tt->name, 0));
          else putf("              ");
        }
        if (opt & o_m)
        {
          if (bb) putf("%-9.9s", bb->mycall);
          else putf("         ");
        }
        if (opt & o_u)
        {
          if (bb)
          {
            if (   ! strcmp(bb->uplink, "TELNET")
                || ! strcmp(bb->uplink, "HTTP"))
              putf("TCP/IP    ");
            else
              putf("%-9.9s ", bb->uplink);
          }
          else putf("          ");
        }
        if (opt & o_d) putf("%-15.15s", tt->downlink);
        if (opt & o_c)
        {
          if (bb)
          {
            long tbytes = bb->rxbytes + bb->txbytes;
            if (tbytes < 10000L)
              putf("%4lu  ", tbytes); //limit to 4 digits
            else
              if (tbytes < (10000L << 10))
                putf("%4luk ", tbytes >> 10);
              else
                putf("%4luM ", tbytes >> 20);
          }
          else putf("      ");
        }
        if (opt & o_r)
        {
          if (bb && bb->forwarding == fwd_none)
          {
            char *board = strchr(bb->prompt, '/');
            if (board) board++;
            else board = bb->prompt;
            putf("%-8.8s", board);
          }
          else putf("        ");
        }
        if (! (opt & (o_u | o_r | o_m))) putv(' ');
        int working = (tt->event == e_ticsfull);
        if (bb) switch (bb->job)
        { case j_idle:
          case j_fwdlink:
        //case j_fwdsend:
        //jobs neu eingestuft, nur wo die Box sendet, gehoert ein '*' dran!
          case j_send:
          case j_fbb_send_propose:
          case j_fbb_send_delay:
          case j_fbb_recv_mail:
          case j_fbb_recv_propose:
          case j_fbb_end:
          case j_fbb_error:
          case j_fwdrecv: break;
          default: working = 1;
        }
        putv(working ? '*' : ' ');
        if (opt & o_w) putf("%s\n", tt->lastcmd);
        else putf("%.*s\n", width, tt->lastcmd);
      }
    }
  }
  if (opt & o_s)
  {
    putf("Wait:  ");
    for (i = 1; i <= waitsp; i++) putf("%d ", waitstack[i]);
    putf("\nStack: ");
    for (i = 0; i < TASKS; i++)
    {
      if (tfeld[i])
      {
        j = 0;
        while ((char) tfeld[i]->stack[j] == 0xaa) j++;
        putf("%d ", TASKSTACK - j);
      }
    }
    putf("(max=%d limit=%d)\n", TASKSTACK - maxstackdepth, TASKSTACK);
  }
  putf(SEPLINE "\n");
}

/*---------------------------------------------------------------------------*/

#ifndef __FLAT__
static void near verify_stack (void)
//*************************************************************************
//
// Stack overflow detection (db1ras)
//
//*************************************************************************
{
  char reason[70];

  if (t && *t->stack != 0xaa)
  {
    enable();
    sprintf(reason, "#W stack overflow state=%c tid=%d tjob=%d stk=%d ev=%d dly=%u",
      lastjob, taskrunning, tjob, TASKSTACK, t->event, t->lastdelay);
    tracelog(reason, _s_serious);
#ifdef _AX25K
    ax25k_shutdown();
#else
    delay(5000); // if smartdrv.exe is still alive give it a chance to
#endif           // write outstanding data if write behind cache is on
    disable(); // be careful...
    coldstart(); // hope this still runs...
  }
}
#endif

/*---------------------------------------------------------------------------*/

void lastcmd (char *cmd)
//*************************************************************************
//
//
//*************************************************************************
{
  if (t && *cmd)
  {
    while (*cmd == ' ') cmd++;
#ifndef __FLAT__
    verify_stack(); //stack overflow detection
#endif
    strncpy(t->lastcmd, cmd, 59);
    t->lastcmd[59] = 0;
  }
}

/*---------------------------------------------------------------------------*/

void lastfunc (char *cmd)
//*************************************************************************
//
//
//*************************************************************************
{
  if (t && *cmd)
  {
    while (*cmd == ' ') cmd++;
#ifndef __FLAT__
    verify_stack(); //stack overflow detection
#endif
    strncpy(t->lastfunc, cmd, 39);
    t->lastcmd[39] = 0;
  }
}

/*---------------------------------------------------------------------------*/

void post_mortem_dump (char *problem)
//*************************************************************************
//
//
//*************************************************************************
{
  char reason[70];
  extern char lasttrace[];

  enable();
  sprintf(reason, "#W %s state=%c tid=%d tjob=%d maxs=%d",
                   problem, lastjob, taskrunning, tjob,
                   TASKSTACK - maxstackdepth);
  tracelog(reason, _s_serious);
  lasttrace[1] = 'W';
  tracelog(lasttrace, _s_serious);
  if (taskrunning != NOTASK)
  {
    task_t *tt = tfeld[taskrunning];
    unsigned int j = 0;
    while (tt->stack[j] == 0xaa) j++;
    sprintf(reason, "#W %s: stk=%d ev=%d dly=%u \"%s\" %s",
                     tt->name, TASKSTACK - j, tt->event, tt->lastdelay,
                     tt->lastcmd, tt->lastfunc);
    #ifdef _AX25K
    ax25k_shutdown();
    #endif
    tracelog(reason, _s_serious);
    if (tt->b)
    {
      b = (tvar_t *) tt->b; // global b
      sprintf(reason, "#W board=%s userjob=%d", b->prompt, b->job);
      tracelog(reason, _s_serious);
    }
  }
  #ifdef _AX25K
  ax25k_shutdown();
  #else
  delay(5000); // if smartdrv.exe is still alive give it a chance to
               // write outstanding data if write behind cache is on
  #endif
}

/*---------------------------------------------------------------------------*/

unsigned int listusers (unsigned int putit)
//*************************************************************************
//
//  gibt die momentan aktiven Benutzer aus
//
//*************************************************************************
{
  unsigned int i, pos = putit;
  task_t *tt;
  tvar_t *bb;
  int logins = 0;

  for (i = 0; i < TASKS; i++)
  {
    tt = tfeld[i];
    if (tt && tt->b)
    {
      bb = (tvar_t *) tt->b;
      if (*bb->logincall)
      {
        logins++;
        if (putit)
        {
          if (putit == 1 || putit == 2)
          {
            if (tt->taskid == taskrunning) putf("You    ");
            else switch (bb->job)
            {
              case j_idle:    putf("Idle   "); break;
              case j_fwdsendf:
              case j_fwdsend: putf("FwdSend"); break;
              case j_fwdlink: putf("FwdLink"); break;
              case j_fwdrecvf:
              case j_fwdrecv: putf("FwdRecv"); break;
              case j_send:    putf("Send   "); break;
              case j_read:    putf("Read   "); break;
              case j_cmd:     putf("Cmd    "); break;
              case j_tell:    putf("Tell   "); break;
              case j_search:  putf("Search "); break;
              case j_convers: putf("Convers"); break;
              case j_fbb_send_propose:
              case j_fbb_send_mail:
              case j_fbb_send_delay:
              case j_fbb_recv_propose:
              case j_fbb_recv_mail:
              case j_fbb_end:
              case j_fbb_error: putf("FBBfwd "); break;
            }
            putf("  %-6s", bb->logincall);
            if (putit == 2)
            {
              if (*bb->mycall) putf(" > %-9s", bb->mycall);
              else putf(" > (n/a)    ");
            }
            if (*bb->uplink) putf("  via %s", bb->uplink);
            if (! strcmp(bb->uplink, "TELNET")
                || ! strcmp(bb->uplink, "HTTP")
                || ! strcmp(bb->uplink, "SMTP")
                || ! strcmp(bb->uplink, "POP3")
                || ! strcmp(bb->uplink, "FTP")
                || ! strcmp(bb->uplink, "NNTP"))
              putf(" [%s]", bb->peerip);
            putv(LF);
          }
          else
          {
            if (pos < putit) putf(" ");
            else
            {
              pos = 2;
              putv(LF);
            }
            pos++;
            putf("%d:%-6s", bb->job, bb->logincall);
          }
        }
      }
    }
  }
  return logins;
}

/*---------------------------------------------------------------------------*/
