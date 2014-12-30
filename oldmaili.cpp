/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------------------
  Import von Mailbestaenden anderer Baybox-Systeme
  ------------------------------------------------

  Copyright (C)       Markus Baumann
                      Am Altenfelder Kreuz 43
                      59302 Oelde

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/



#include "baycom.h"

#ifdef OLDMAILIMPORT

void oldmailimport (char *directory)
//*************************************************************************
//
//  Importiert Maildateien anderer Baybox-Systeme aus einem Verzeichnis
//
//*************************************************************************
{
  char name[20];

  strcpy(name, "oldmailimport");
  lastfunc(name);
  unsigned import_num = 0, skip_num = 0, error_num = 0;
  char boardname[8];
  char bid[BIDLEN+1];
  char zeile[LINELEN+1];
  char sourcefilename[FNAMELEN+1];
  char destinationfilename[FNAMELEN+1];
  char boardfilename[FNAMELEN+1];
  int z, y, ret, fehler;
  FILE *mf = NULL;
  struct dirent *di;
  DIR *d;
  *sourcefilename = 0;
  *destinationfilename = 0;

  if ((d = opendir(directory)) != NULL)
  {
    while ((di = readdir(d)) != NULL)
    {
      char *pkt = strchr(di->d_name, '.');
      if (! pkt)
      {
        sprintf(sourcefilename, "%s/%s", directory, di->d_name);
        if ((mf = s_fopen(sourcefilename, "srt")) != NULL)
        {
          fgets(zeile, LINELEN - 1, mf);
          s_fclose(mf);
          fehler = 0;
          *boardname = 0;
          *bid = 0;
          for (z = 0; z < 8; z++)
          {
            if (zeile[z] == ' ' || zeile[z] == '\n' || zeile[z] == '\r')
              break;
            boardname[z] = zeile[z];
          }
          if (z > 0)
            boardname[z] = 0;
          else
          {
            putf(ms(m_omi_errboard), sourcefilename);
            fehler++;
          }
          if (! fehler)
            for (y = z; y < (LINELEN-z); y++)
            {
              if (zeile[y] == '$') break;
              if (zeile[y] == '\n' || zeile[y] == '\r' || y == (LINELEN-z-1))
              {
                putf(ms(m_omi_nobid), sourcefilename);
                fehler++;
              }
            }
          if (! fehler)
          {
            for (z = y + 1; z < LINELEN; z++)
            {
              if (zeile[z] == ' ' || zeile[z] == '\n' || zeile[z] == '\r')
                break;
              else
                bid[z-y-1] = zeile[z];
            }
            if (z > y+1) bid[z-y-1] = 0;
            else
            {
              putf(ms(m_omi_errbid), sourcefilename);
              fehler++;
            }
          }
          if (fehler > 0) error_num++;
          else
          {
            if (bidvorhanden(bid))
            {
              if (b->opt & o_q)
                putf(ms(m_omi_bidisknown), sourcefilename, boardname, bid);
              skip_num++;
            }
            else
            {
              convert(boardname, 0);
              if (b->opt & o_q)
                putf(ms(m_omi_bidisnew), sourcefilename, boardname, bid);
              finddir(boardname, 1);
              if (! *b->boardname)
              {
                putf(ms(m_omi_boardcreated), boardname);
                ret = mkboard("TMP", boardname, 0);
                if (ret != 0)
                  trace(serious, "oldmailimport", "mkboard %d error", ret);
                finddir(boardname, 1);
                if (xmkdir(b->boardpath))
                   trace(serious, "oldmailimport", "mkdir %s error",
                                                   b->boardpath);
              }
              sprintf(destinationfilename, "%s/%s", b->boardpath, di->d_name);
              sprintf(boardfilename, "%s/%s", b->boardname, di->d_name);
              strlwr(destinationfilename);
              //strlwr(sourcefilename);
              if (filecopy(sourcefilename, destinationfilename))
              {
                trace(serious, "oldmailimport", "filecopy %s %s\n",
                               sourcefilename, destinationfilename);
                error_num++;
              }
              else
              {
                bidrein(bid, boardfilename);
                import_num++;
              }
            }
          }
        }
      }
    }
    closedir(d);
  }
  if (import_num > 0)
  {
    while (sema_test("purgereorg"))
      wdelay(200);
    if (! (b->opt & o_r))
    {
      putf(ms(m_omi_reorg));
      fork(P_BACK | P_MAIL, 0, mbreorg, "f");
    }
  }
  putf(ms(m_omi_result),
       import_num+skip_num+error_num, import_num, skip_num, error_num);
}

#endif
