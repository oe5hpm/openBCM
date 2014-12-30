/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------------------
  Extract 7plus and AutoBin Files out of a mail
  ---------------------------------------------


  Copyright (C) 2000     Patrick, DF3VI

  (GPL)

 ***************************************************************/

//20000103 OE3DZW removed german umlaute, removed from mbread, optional
//20000103 OE3DZW added comments, please fix problems marked
//20000322 DK2UI  some corrections
//20000415 DF3VI  changed output to TEMPPATH
//20000505 DK2UI  file time for Linux

#include "baycom.h"

#ifdef DF3VI_EXTRACT

/*---------------------------------------------------------------------------*/

void extract (unsigned msgnr)
//*************************************************************************
//
//  EXTRACT-Utility by DF3VI, extrahiert Binaerteile einer Mail auf
//  die Mailbox-Festplatte ins Box-temp-Verzeichnis
//
//*************************************************************************
{ //is this re-entrant? if not, pse set semaphor
  FILE *f, *g;
  char name[FNAMELEN+1], s[BUFLEN], st[FNAMELEN+1], *ptr;
  unsigned long posi, bytes;
  int err;
  unsigned int w, r;
  char *puffer;

  strlwr(b->mailpath);
  if (! (f = s_fopen(b->mailpath, "sr")))
  {
    putf(ms(m_extract_errorread), b->mailpath);
    return;
  }
  fgets(s, BUFLEN - 1, f);
  mbsend_parse(s, 0);
  do
  {
    waitfor(e_ticsfull);
    fgets(s, BUFLEN - 1, f);
  }
  while (! feof(f)
         && ! (strstr(s, " go_") == s) && ! (strstr(s, "#BIN#") == s));
  if (feof(f)) // keine Markierung gefunden
  {
    putf(ms(m_extract_nostartline), b->boardname, msgnr);
    s_fclose(f);
    return;
  }
  do
  {
    if (strstr(s, "#BIN#") == s)
    {
      s_fclose(f);
      subst1(s, LF, 0);
      if (! sscanf(s, "#BIN#%lu", &bytes)) // Laenge lesen
      {
        putf(ms(m_extract_invalidheader), b->boardname, msgnr);
        return;
      }
      unsigned long bintime;
      sscanf(strchr(s, '$'), "$%lX#", &bintime);
      ptr = s + strlen(s);
      while (*ptr != '#' && ptr > s) ptr--;    // Dateinamen suchen
      if (*(ptr - 1) != '#') ptr++;            // # im Dateinamen ?
      if (! *ptr || *ptr == '$' || *ptr == '|' )
      {
        putf(ms(m_extract_filenamemissing));
        err = 0;
        do                              // Ersatzname RUBRIK.lfn erzeugen
        {
          waitfor(e_ticsfull);
          sprintf(s, TEMPPATH "/%s.%03i", b->boardname, ++err);
          strlwr(s);
        }
        while (! access(s, 0) && err <= 999);
        if (err > 999)
        {
          putf(ms(m_extract_filecantcreate));
          return;
        }
        else
          putf(ms(m_extract_filecreate), s);
      }
      else
      {
        strcpy(name, ptr);
        ptr = name + strlen(name);
        while (ptr >= name && *ptr != ':' && *ptr != '\\' && *ptr != '/')
          ptr--; // LW/Pfad abtrennnen
//      putf("Dateiname: %s\n", ptr+1);
        sprintf(s, TEMPPATH "/%s", ptr + 1);
        strlwr(s);
      }
      if (! (g = s_fopen(s, "swb")))
      {                                  // Ziel-Datei oeffnen
        putf(ms(m_extract_errorwritebin), errno, s, strerror(errno));
//      perror("");
        return;
      }
      if (! (puffer = (char *) t_malloc(2048, "extr")))
      {                                  // 2 kb Puffer zum Daten-Kopieren
        putf(ms(m_nomem));
        s_fclose(g);
        return;
      }
      f = s_fopen(b->mailpath, "srb"); // BIN-Datei oeffnen
      fseek(f, b->binstart + BINCMDLEN, SEEK_SET); // Position auf Anfang
      posi = r = 0; // Byte-Zaehler gesamt/gelesen
      do
      {
        r = fread(puffer, 1, 2048, f);
        posi += r;
        fwrite(puffer, 1, r, g);
        waitfor(e_ticsfull);
      }
      while (r == 2048);
      t_free(puffer);
      s_fclose(g);
      s_fclose(f);
      setfiletime(s, bintime);
      putf(ms(m_extract_extractbin), s, posi, bytes, b->boardname, msgnr);
      return; // BIN ist immer Fileende
    }
    // Ansonsten DG1BBQ-FFR-Markierung
    if ((strstr(s, " go_text. ") == s) || (strstr(s, " go_info. ") == s))
    {
      sscanf(s + 10, "%s", st); // Text-Datei-Namen lesen
      sprintf(name, TEMPPATH "/%s", st);
      strlwr(name);
      if (! access(name, 0)) // Wenn vorhanden, Extension hochzaehlen
      {
        err = 1;
        switch (name + strlen(name) - strchr(name, '.'))
        {
          case 1:                              // "name."    -> "name.__"
          case 2: strcat(name, "__" ); break;  // "name.x"   -> "name.x__"
          case 3: strcat(name, "_");           // "name.xx"  -> "name.xx_"
          case 4: break;                       // "name.xxx" -> "name.xxx"
          default: strcat(name, ".__");        // "name"     -> "name.__"
        }
        *(name + strlen(name) - 2) = 0;
        do                            // ZWEI-stellige laufenden Nummer
        {
          waitfor(e_ticsfull);
          sprintf(s, "%s%02x", name, err++);
        }
        while (! access(s, 0) && err < 255);
        if (err >= 255)
        {
          putf(ms(m_extract_filecantcreate));
          *name = 0;
        }
        else
        {
          strcpy(st, s);
          strlwr(st);
        }
      }
      *s = 0; // Keine Startmarkierung schreiben
    }
    else
    if (strstr(s, " go_7+. ") == s) // 7PLUS-Datei-Kopf auswerten
    {                               // lfn, Bereich, File-Name
      if (sscanf(s + 8, "%u of %u %s", &w, &r, name) != 3)
      {
        putf(ms(m_extract_invalidstartline), b->boardname, msgnr);
        *name = 0;
      }
      else
      {
        subst1(name, '.', 0); // Extension abtrennen
        if (r == 1)
          sprintf(st, TEMPPATH "/%s.7pl", name); // Einteilig: Endung .7PL
        else
          sprintf(st, TEMPPATH "/%s.p%02x", name, w); // Mehrteilig: End. .PXX
      }
    }
    else
    {
      putf(ms(m_extract_invalidstartline), b->boardname, msgnr);
      *st = 0;
    }
    if (*st)
    {
      strlwr(st);
      if (! (g = s_fopen(st, "sw"))) // Ziel-Datei schreiben
      {
        putf(ms(m_extract_errorwrite), st);
        return;
      }
      do
      {
        fputs(s, g);
        *s = 0;
        waitfor(e_ticsfull);
        fgets(s, BUFLEN - 1, f);
      }                            // Bis File-Ende oder Stop-Markierung
      while (! feof(f) && ! (strstr(s, " stop_") == s));
      if (strstr(s, " stop_7+"))
        fputs(s, g); //nur bei 7+ Endemarkierung schreiben
      s_fclose(g);
      putf(ms(m_extract_extract7p), st, b->boardname, msgnr);
    }
    do
    {
      waitfor(e_ticsfull);
      *s = 0;
      fgets(s, BUFLEN - 1, f); // Naechste Start-Markierung suchen
    }
    while (! feof(f)
           && ! (strstr(s, " go_") == s) && ! (strstr(s, "#BIN#") == s));
  }
  while (! feof(f));                     // solange 7+ folgt
  s_fclose(f);                           // oder File-Ende
}

#else

/*---------------------------------------------------------------------------*/

void extract (int msgnum) //dummy
{
   putf(ms(m_notavailable), "EXTRACT");
}

/*---------------------------------------------------------------------------*/

#endif   // _EXTRACT
