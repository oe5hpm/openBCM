/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------
  Verwaltung und Vorbereitung der Meldungsausgabe
  -----------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980831 OE3DZW should also accept 0x0a linebreaks in message-files
//20000415 DF3VI  only one allocation for all messages
//20021208 DH8YMB speech.bcm zur Verwaltung der Landeskenner

#include "baycom.h"
#include "msg.h"

/*---------------------------------------------------------------------------*/

#define MAXSPRACHEN 40

typedef struct msg_t
{ char msgkenner[4];
  char **msgpointer;
  char landeskenner[LINELEN+1];
  char beschreibung[LINELEN+1];
  long last_usage;
} msg_t;

typedef struct speech_t
{ char msgkenner[4];
  char landeskenner[LINELEN+1];
} speech_t;

static msg_t *msg = NULL;
static speech_t *speech = NULL;
static unsigned int  sprachen = 0;
static int msg_loadnum = 1;

/*---------------------------------------------------------------------------*/

unsigned msg_limit (int current)
//*************************************************************************
//
//  gibt aktuelle oder maximale Anzahl Sprachen zurueck
//
//*************************************************************************
{
  if (current)
    return sprachen;
  else
    return MAXSPRACHEN;
}

/*---------------------------------------------------------------------------*/

void msg_listspeech (void)
//*************************************************************************
//
//  listet alle verfuegbaren Sprachen auf
//
//*************************************************************************
{
  unsigned int sprache;

  for (sprache = 0; sprache < sprachen; sprache++)
    putf("%3s: %s (%s )\n", msg[sprache].msgkenner,
                            msg[sprache].beschreibung,
                            msg[sprache].landeskenner);
}

/*---------------------------------------------------------------------------*/

static void create_speechbcm (void)
//*************************************************************************
//
//  speech.bcm neu anlegen
//
//*************************************************************************
{
  FILE *speechfile = NULL;

  speechfile = s_fopen("speech.bcm", "sat"); //schreibend oeffnen
  if (m.callformat == 0)
  {
    fputs("GB A EI G K M N VE VK W\r\n", speechfile);
    fputs("CT CS CT CU PY\r\n", speechfile);
    fputs("DL DB DC DD DEU DF DG DH DJ DK DL DM DN DO OE HB IN3 IW3\r\n",
                          speechfile);
    fputs("EA EA EB EC ED XE LU CE\r\n", speechfile);
    fputs("FF F ON 3A 3C VE2 VA2 VE9 T\r\n", speechfile);
    fputs("HA HA HG\r\n", speechfile);
    fputs("HRV 9A T9 Z3\r\n", speechfile);
    fputs("I I HV\r\n", speechfile);
    fputs("JA JA JD JE JF JG JH JI JK JL JM JN JO JP JQ JR JS 7K 7L 7M 7N 8J 8K 8L 8M 8N\r\n", speechfile);
    fputs("LX LX\r\n", speechfile);
    fputs("NL PD PE PA PB PI\r\n", speechfile);
    fputs("OK OK\r\n", speechfile);
    fputs("OM OM\r\n", speechfile);
    fputs("PL SN SO SP SQ SR 3Z HF\r\n", speechfile);
    fputs("RUS R U EW EX EY EZ\r\n", speechfile);
    fputs("S5 S5 SVN SI\r\n", speechfile);
    fputs("TA TA\r\n", speechfile);
    fputs("TRK\r\n", speechfile);
    fputs("BAD\r\n", speechfile);
    fputs("BAY\r\n", speechfile);
    fputs("BW\r\n", speechfile);
    fputs("KL\r\n", speechfile);
    fputs("PF\r\n", speechfile);
    fputs("DLA\r\n", speechfile);
    trace(serious, "msg_search", "speech.bcm (ham) created");
  }
  else
  {
    fputs("GB AUS UK ENG SCO USA\r\n", speechfile);
    fputs("CT POR\r\n", speechfile);
    fputs("DL D DB DC DD DEU DF DG DH DJ DK DL DM DN DO AT HE\r\n",
                            speechfile);
    fputs("EA ESP SPA\r\n", speechfile);
    fputs("FF FRA\r\n", speechfile);
    fputs("HA HUN\r\n", speechfile);
    fputs("HRV CRO\r\n", speechfile);
    fputs("I ITA IT\r\n", speechfile);
    fputs("JA JAP\r\n", speechfile);
    fputs("LX LUX\r\n", speechfile);
    fputs("NL NL BEL\r\n", speechfile);
    fputs("OK CZ\r\n", speechfile);
    fputs("OM\r\n", speechfile);
    fputs("PL POL\r\n", speechfile);
    fputs("RUS RUS\r\n", speechfile);
    fputs("S5 SLO\r\n", speechfile);
    fputs("TA TUR\r\n", speechfile);
    fputs("TRK\r\n", speechfile);
    fputs("BAD\r\n", speechfile);
    fputs("BAY\r\n", speechfile);
    fputs("BW\r\n", speechfile);
    fputs("KL\r\n", speechfile);
    fputs("PF\r\n", speechfile);
    fputs("DLA\r\n", speechfile);
    trace(serious, "msg_search", "speech.bcm (cb) created");
  }
  s_fclose(speechfile);
}

/*---------------------------------------------------------------------------*/


//static void checkvalid_speechbcm (void)
//*************************************************************************
//
//  speech.bcm auf Vollstaendigkeit pruefen
//
//*************************************************************************
/*{
  FILE *speechfile = NULL;

  speechfile = s_fopen("speech.bcm", "srt"); //lesend oeffnen

  s_fclose(speechfile);

}
*/

/*---------------------------------------------------------------------------*/

static void near msg_search (void)
//*************************************************************************
//
//  schaut nach, welche Sprachen existieren, speichert die vorhandenen
//  im msgkenner-Array ab und legt evtl. speech.bcm an
//
//*************************************************************************
{
  struct dirent *di;
  char filename[30];
  FILE *f = NULL;
  FILE *speechfile = NULL;
  DIR *d;
  char *buf;
  char s[LINELEN+1];
  char *pkt;
  char *beschr;
  int j, found, k;

#ifdef __FLAT__
  //generate standard messages.gb if file is not existing
  if (! file_isreg(STDLANG) && (f = s_fopen(STDLANG, "sw+t")) != 0)
  {
    if (f) // file is not accessible (why ever...)
    {
      fwrite(stdmsg_init, sizeof(stdmsg_init) - 1, 1, f);
      trace(replog, "msg", STDLANG" generated");
      s_fclose(f);
    }
  }
#endif
  sprachen = 0;
//  DH8YMB: "speech.bcm" zur Verwaltung der Landeskenner
//  falls "speech.bcm" noch nicht existiert, neu generieren ansonsten
//  auf Vollstaendigkeit ueberpruefen
  if ((speechfile = s_fopen("speech.bcm", "srt")) != NULL)
  {
 //   checkvalid_speechbcm();
    s_fclose(speechfile);
  }
  else
    create_speechbcm();
  if ((speechfile = s_fopen("speech.bcm", "srt")) != NULL)
  {
    j = 0;
    found = 0;
    if (speech)
    {
      t_free(speech);
      speech = NULL;
    }
    speech = (speech_t *) t_malloc(sizeof(speech_t) * MAXSPRACHEN, "*msd");
    while (fgets(s, LINELEN, speechfile))
    {
      if (*s == ';') continue;
      subst(s, ';', 0); // comments
      cut_blank(s);
      if (*s)
      {
        strupr(s);
        buf = nexttoken(s, speech[j].msgkenner, 3);
        rm_crlf(buf); // CRLF abschneiden
        if (strlen(buf) == 0) strcpy(buf, "NONE");
        strcpy(speech[j].landeskenner, " "); //Wichtig: blank vor landeskenner!
        strcat(speech[j].landeskenner, buf);
      }
      else
        trace(serious, "msg_search",
                       "speech.bcm has corrupt format, line %d", (j+1));
      j++;
    }
    if ((d = opendir(MSGPATH)) != NULL)
    {
      if (msg)
      {
        t_free(msg);
        msg = NULL;
      }
      msg = (msg_t *) t_malloc(sizeof(msg_t) * MAXSPRACHEN, "*msd");
      while ((di = readdir(d)) != NULL)
      {
        if (stristr(di->d_name, "messages"))
        {
          pkt = strchr(di->d_name, '.');
          if (pkt && stricmp(pkt + 1, "BAK") && isalpha(pkt[1]))
          {
            pkt[4] = 0;
            msg[sprachen].msgpointer = NULL;
            strcpy(msg[sprachen].msgkenner, pkt + 1);
            strupr(msg[sprachen].msgkenner);
            found = 0;
            for (k = 0; k < j; k++)
            {
              if (! strcmp(msg[sprachen].msgkenner, speech[k].msgkenner))
              {
                found = 1;
                strcpy(msg[sprachen].landeskenner, speech[k].landeskenner);
                break;
              }
            }
            if (! found)
            {
              trace(serious, "msg_search",
                             "%s not defined in speech.bcm",
                             msg[sprachen].msgkenner);
              strcpy(msg[sprachen].landeskenner, " NONE");
            }
            sprintf(filename, MSGPATH "/%s", di->d_name);
            if ((f = s_fopen(filename, "srt")) != NULL)
            {
              beschr = msg[sprachen].beschreibung;
              *beschr = ' ';
              fgets(beschr + 1, 69, f);
              if (beschr[strlen(beschr) - 1] == LF)
                beschr[strlen(beschr) - 1] = 0;
              if (! beschr[1])
                strcpy(beschr + 1, msg[sprachen].msgkenner);
              strupr(beschr);
              s_fclose(f);
            }
            if (strlen(msg[sprachen].landeskenner) == 0)
              strcpy(msg[sprachen].landeskenner, msg[sprachen].beschreibung);
            sprachen++;
            if (sprachen >= MAXSPRACHEN)
            {
              trace(serious, "msg_search", "too many languages");
              break;
            }
          }
        }
      }
    closedir(d);
    }
    s_fclose(speechfile);
    msg_loadnum++;
  }
  else
    trace(serious, "msg_search", "speech.bcm fatal error");
//  trace(report, "msg_search", "Erkannte Sprachen: %d",sprachen);
}

/*---------------------------------------------------------------------------*/

int is_msgkenner (char *kenn)
//*************************************************************************
//
//  stellt fest, ob der uebergebene Kenner existiert
//
//*************************************************************************
{
  if (! *kenn) return 0;
  if (! sprachen) msg_search();
  for (unsigned int sprache = 0; sprache < sprachen; sprache++)
  {
    if (! stricmp(msg[sprache].msgkenner, kenn)) return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

char *msg_landsuch (char *call)
//*************************************************************************
//
//  sucht fuer ein eingegebenes Rufzeichen die Tabelle der Landeskenner
//  durch und uebergibt die Kennung der dazu passenden Sprache
//
//*************************************************************************
{
  unsigned int sprache, laenge;
  char vgl[5];
  char *found;

  if (! sprachen) msg_search();
  *vgl = ' ';
  strncpy(vgl + 1, call, 3);
  vgl[4] = 0;
  for (laenge = 3; laenge > 0; laenge--)
  {
    vgl[laenge + 1] = 0;
    for (sprache = 0; sprache < sprachen; sprache++)
    {
      msg_t *mm = &msg[sprache];
      found = stristr(mm->landeskenner, vgl);
      if (found && ! isalnum(found[strlen(vgl)])) return mm->msgkenner;
      if (! strcmp(mm->msgkenner, call)) return mm->msgkenner;
    }
  }
  return "??";
}

/*---------------------------------------------------------------------------*/

static int near msg_getkenn (char *kenn)
//*************************************************************************
//
//  Sucht den Index fuer eine bestimmte Sprache
//
//*************************************************************************
{
  unsigned int i;

  if (! sprachen) msg_search();
  for (i = 0; i < sprachen; i++)
  {
    if (! strcmp(kenn, msg[i].msgkenner)) return i;
  }
  for (i = 0; i < sprachen; i++)
  {
    if (! strcmp("GB", msg[i].msgkenner)) return i;
  }
  for (i = 0; i < sprachen; i++)
  {
    if (! strcmp("DL", msg[i].msgkenner)) return i;
  }
  return EOF;
}

/*---------------------------------------------------------------------------*/

static int near msg_getline (char *buf, FILE *f)
//*************************************************************************
//
//  Liest eine Zeile aus einem Message-Quellfile
//  Rueckgabe: Gelesene Zeilenlaenge INCL(!) Null-Terminierung
//             oder 0 wenn nichts gelesen werden konnte
//
//*************************************************************************
{
  char s[200];
  unsigned int i = 1, j = 0;

  while (fgets(s, 199, f))
  {
    if (*s == '"') // Nur Zeilen die mit " anfangen
    {
      while (   (s[i] == ESC || s[i] >= ' ')
             && (s[i] != '"' || (i && s[i - 1] == '\\'))
             &&  s[i] != LF && i < 199)
      {
        if (s[i] == '\\')
        {
          i++;
          switch (s[i])
          {
            case 'n':  buf[j] = LF; break;
            case 'r':  buf[j] = CR; break;
            case '"':  buf[j] = '"'; break;
            case 'a':  buf[j] = '\a'; break;
            case '\\': buf[j] = '\\'; break;
          }
        }
        else
          buf[j] = s[i];
        j++;
        i++;
      }
      buf[j++] = 0;
      return j;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void msg_dealloc (int all)
//*************************************************************************
//
//
//*************************************************************************
{
  unsigned int sprache;

  if (all)
  {
    for (sprache = 0; sprache < sprachen; sprache++)
    {
      if (msg[sprache].msgpointer)
      {
        t_free(msg[sprache].msgpointer[0]);
        t_free(msg[sprache].msgpointer);
        msg[sprache].msgpointer = NULL;
      }
    }
    msg_search();
    trace(report, "msg_dealloc", "new messages");
  }
  for (sprache = 0; sprache < sprachen; sprache++)
  {
    msg_t *mm = &msg[sprache];
    if (mm->msgpointer && (systic-mm->last_usage) > 3276) //3min
    {
      t_free(mm->msgpointer[0]);
      t_free(mm->msgpointer);
      mm->msgpointer = NULL;
      msg_loadnum++;
      trace(report, "msg_dealloc", "%s remove", mm->msgkenner);
    }
  }
}

/*---------------------------------------------------------------------------*/

static int near msg_load (char *msgkenn)
//*************************************************************************
//
//  Laedt eine bestimmte Sprache von der Platte
//
//*************************************************************************
{
  FILE *f;
  char fname[20];
  int sprache = msg_getkenn(msgkenn);
  char *msgplatz;
  unsigned len, msgnum = 0;
  msg_t *mm = &msg[sprache];

  if (sprache != EOF)
  {
    strcpy(fname, MSGPATH "/messages.");
    strcat(fname, mm->msgkenner);
    strlwr(fname);
    if ((f = s_fopen(fname, "srt")) != NULL)
    {
      if (mm->msgpointer)
      {
        t_free(mm->msgpointer[0]);
        t_free(mm->msgpointer);
      }
      len = (unsigned) filelength(fileno(f)) - (NUM_MESSAGES * 3); // LF und "" weg
      mm->msgpointer = (char **) t_malloc(NUM_MESSAGES * 4, "*msp");
      msgplatz       = (char  *) t_malloc(len, "*msg");
      do
      {
        len = msg_getline(msgplatz, f);
        /*
        if (len > 199)
        {
          trace(report, "msg_getline", "%s: line %d maybe too long (%d)",
                          mm->msgkenner, msgnum+1, len);
        }
        */
        if (! len)
        {
          trace(serious, "msg_load", "%s: %d lines missing",
                          mm->msgkenner, NUM_MESSAGES - msgnum);
          while (msgnum < NUM_MESSAGES)
            mm->msgpointer[msgnum++] = "!old messages file!";
          break;
        }
        mm->msgpointer[msgnum++] = msgplatz;
        msgplatz += len;
      }
      while (msgnum < NUM_MESSAGES);
      s_fclose(f);
    }
  }
  trace(report, "msg_load", "%s: %d lines", mm->msgkenner, msgnum);
  return sprache;
}

/*---------------------------------------------------------------------------*/

char *ms (MSG_TYP msgnum)
//*************************************************************************
//
//  Gibt den Zeiger auf eine bestimmte Message zurueck
//
//*************************************************************************
{
  lastfunc("ms");
  register int offset;
  static char ext[4];
  unsigned int i;
  static char tmp1[200];
  static char tmp2[200];
  static char tmp3[200];
  static char tmp4[200];
  static char tmp5[200];
  static int zahl;
  char *kl;

  if (b->msg_loadnum != msg_loadnum)
  {
    offset = msg_getkenn(u->sprache);
    if (offset == EOF)
    {
      trace(tr_abbruch, "ms", "no msg file ('msg/messages.gb' must exist)");
      return "";
    }
    if (! msg[offset].msgpointer) msg_load(u->sprache);
    b->msg_offset = offset;
    b->msg_loadnum = msg_loadnum;
  }
  else
    offset = b->msg_offset;
  msg[offset].last_usage = systic;
  if (msgnum == m_helpext)
  {
    kl = strchr(msg[offset].landeskenner, '(');
    i = 0;
    if (kl)
    {
      kl++;
      while (i < 3 && kl[i] > ')')
      {
        ext[i] = kl[i];
        i++;
      }
      ext[i] = 0;
      return ext;
    }
    else return u->sprache;
  }
  else
  {
#ifdef __FLAT__
    // bei HTTP ESC-Sequenzen herausfiltern
    if (b->http > 0)
    {
      //maximal 5 ms-char-Zeiger pro putf-Aufruf
      zahl++;
      if ((zahl % 5) == 0)
      {
        zahl = 0;
        strcpy(tmp5, msg[offset].msgpointer[msgnum]);
        rm_esc(tmp5);
        return tmp5;
      }
      else
        if ((zahl % 4) == 0)
        {
          strcpy(tmp4, msg[offset].msgpointer[msgnum]);
          rm_esc(tmp4);
          return tmp4;
        }
        else
          if ((zahl % 3) == 0)
          {
            strcpy(tmp3, msg[offset].msgpointer[msgnum]);
            rm_esc(tmp3);
            return tmp3;
          }
          else
            if ((zahl % 2) == 0)
            {
              strcpy(tmp2, msg[offset].msgpointer[msgnum]);
              rm_esc(tmp2);
              return tmp2;
            }
            else
            {
              strcpy(tmp1, msg[offset].msgpointer[msgnum]);
              rm_esc(tmp1);
              return tmp1;
            }
    }
    else
#endif
      return msg[offset].msgpointer[msgnum];
  }
}

/*---------------------------------------------------------------------------*/
