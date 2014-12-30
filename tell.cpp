/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------------------------
  Tell-Server
  ----------------------------------------------------------------


  Copyright (C) 2000     Patrick, DF3VI
                         Dietmar Zlabinger, OE3DZW
  (GPL)

 ***************************************************************/


#include "baycom.h"

/*---------------------------------------------------------------------------*/

void send_tell (char *befbuf)
//****************************************************************************
//
//****************************************************************************
{
  char sendcmd[121];
  char *ptr;
  lastfunc("send_tell");
  if (! *befbuf)
  {
    putf("Syntax: TELL <bbs> <remote-command>\n");
    return;
  }
  ptr = nextword(befbuf, b->at, HADRESSLEN);
  if (*ptr)                        // Befehl vorhanden?
  {
    if (strlen(ptr) > 46)
    {
      putf(ms(m_cmdtoolong));
      return;
    }
    if (find_hadr(b->at))         //address is known bbs
    {
      if (weiterleiten(0, "T") != unbekannt) //Adresse ist gueltig
      {
        strupr(ptr);
        subst(ptr, '@', 0);       // remove any @ symbols
        mkboard("/", "T", 0);     // Board T anlegen
        snprintf(sendcmd, sizeof(sendcmd),
                          "SP T @%s <%s\n%s  @ %s\n", b->at, u->call,
                                      ptr, u->mybbsok ? u->mybbs : m.boxname);
        genmail(sendcmd, "");     // Tell ist textlos
        putf(ms(m_linesfor), 2, b->at);
        putf(" ");
        putf(ms(m_stored));      // Meldung ausgeben
      }
      else putf(ms(m_adrunknown), b->at);
    }
    else putf(ms(m_adrunknown), b->at);
  }
  else putf(ms(m_nomessagestored));
}

/*---------------------------------------------------------------------------*/

void tellresp (char *name)
//****************************************************************************
//
//****************************************************************************
{
//  int lines;
  int ch;
  char line[BUFLEN];
  char impdatei[30];                // Dateiname fuer die Eingabe
  char outdatei[30];                // Dateiname fuer die Ausgabe
  char ziel[HADRESSLEN];
  lastfunc("tellresp");

  FILE *f = s_fopen(b->mailpath, "sr");
  FILE *g;
  if (! f) return; //file not accessible - why ever...
  fgets(line, BUFLEN - 1, f);          // Kopfzeile der T-Mail
  mbsend_parse(line, 0);               // auswerten
  fgets(line, BUFLEN - 1, f);          // Forward ueberlesen
  fgets(line, BUFLEN - 1, f);          // Read ueberlesen
  fgets(line, BUFLEN - 1, f);          // Titel lesen
  s_fclose(f);
  if (strchr(line, '@')) // Adresse fuer Rueckweg
  {
    safe_strcpy(ziel, strchr(line, '@') + 1);
    subst(ziel, LF, 0);
    subst(line, '@', 0);
    strcpy(b->at, ziel + blkill(ziel)); // Rueckadresse weg, wenn unbekannt
    if (weiterleiten(0, b->herkunft) == unbekannt) *ziel = 0;
  }
  else return; //no valid address has been found, ignore mail
  //automatically generate file-name (must be re-entrant)
  sprintf(impdatei, TEMPPATH "/%s", time2filename(0));
  f = s_fopen(impdatei, "sw+b"); // Ausgabedatei oeffnen
  if (! f)
  {
    trace(serious, "tellresp", "can't open %s", impdatei);
    return;
  }

  fprintf(f, "SP %s ", b->herkunft); // Send-Zeile fuer Import vorbereiten
  if (*ziel) fprintf(f, "@%s\n", ziel);
  fprintf(f, "TELL-Response de %s: %s\n", m.boxname, line);
  fprintf(f, "------------------------------------------------------\n");
  sprintf(outdatei, TEMPPATH "/%s", time2filename(0));
  g = s_fopen(outdatei, "sw+b"); // Ausgabedatei oeffnen
  if (! g)
  {
    trace(serious, "tellresp", "can't open %s", outdatei);
    return;
  }
  markerased('W', 0, 0); // T-Mail loeschen
  if (*line) // EXPORT-Funktion aus SYSOP.CPP
  {
    mblogin(b->herkunft, login_silent, name); //Als User einloggen
    b->msg_loadnum--;      // Sonst wird falsche Sprache benutzt :(
    //if((t->input!=io_file||t->output==io_dummy) && t->output!=io_file)
    {
      b->outputfile = g;
      s_fsetopt(b->outputfile, 0); // Ausgabedatei NICHT loeschen
      b->oldinput = t->input;
      b->oldoutput = t->output;
      t->input = io_dummy;
      t->output = io_file;
      b->continous = 1;
      b->sysop = 0;                // Keine Sysop-Rechte !
      b->job = j_tell;             // Job kennzeichnen
      mailbef(line, 0);            // Befehl ausfuehren
    }
  }
  s_fclose(g);
  g = s_fopen(outdatei, "sr+b");
//  lines = 1;
  ch=fgetc(g);
  while (! feof(g))
  {
    fputc(ch,f);
    ch=fgetc(g);
  }
/*
  fgets(line, BUFLEN - 1, g);
  do
  {
    fputs(line, f);
    fgets(line, BUFLEN - 1, g);
    lines++;
  }
// Wozu eine Begrenzung auf 530 Zeilen??
  while (! feof(g) && lines < 530); //Begrenzung auf maximal 530 Zeilen
  if (! feof(g)) fprintf(f, "\n %s\n", ms(m_aborted));
*/
  fprintf(f, "\nnnnn\nimpdel\n");
  s_fclose(f);
  s_fclose(g);
  xunlink(outdatei);
  fork(P_BACK | P_MAIL, 0, mbimport, impdatei); // Ausgabe importieren
};

