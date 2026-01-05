/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------
  Verwaltung von Bulletin-IDs
  ---------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980208 OE3DZW changed output format for bid -i
//19980208 OE3DZW fixed above format
//19980223 OE3DZW fixed fsetopt
//19990219 OE3DZW fixed check for unique bid
//19991111 OE3DZW 11:11h ;-) removed check for length of BID
//20000108 OE3DZW added bidoffset (multiple bcm's)
//20000625 DK2UI  newbid() resets counter itself and not from crond
//20000814 DK2UI  newbid() searches for uniqe BID
//20030720 hpk    newbid() CB-BCMNET bbs'es have now a unique BID/MID

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#define HLEN 4
#define BID_FILENAMELEN 16
#define nextpos(fptr) (filelength(fileno(fptr))/sizeof(BID))

/*---------------------------------------------------------------------------*/

// Needed to get the temp-BIDs
extern task_t *tfeld[TASKS];

/*---------------------------------------------------------------------------*/

typedef FILE *fp;
static int is_null = 0;

/*---------------------------------------------------------------------------*/

typedef struct
{ char name[BIDLEN+1];
  char filename[BID_FILENAMELEN+1];
  long nextsamehash;
} BID;

/*---------------------------------------------------------------------------*/

typedef struct
{ char name[BIDLEN+1];
  short unsigned nextsamehash;
  char filename[BID_FILENAMELEN+1];
} BID_OLD;

/*---------------------------------------------------------------------------*/

typedef struct
{ char reserved[BIDLEN + BID_FILENAMELEN];
  short unsigned lastbid; // letzte vergebene BID-Nummer
  long filesize;          // Groesse des BID-File
} KOPF;

/*---------------------------------------------------------------------------*/

long bid_len (void)
//*************************************************************************
//
//*************************************************************************
{
  return filesize(BIDNAME) / sizeof(BID);
}

/*---------------------------------------------------------------------------*/

//static void near bidfile_newformat(void)
//*************************************************************************
//
//*************************************************************************
/*
{
  FILE *fi;
  FILE *fo;
  BID b;
  BID_OLD bo;

  if (filetime(BIDNAME) < filetime(BIDNAME_OLD))
  {
    trace(replog, "bid", "converting format");
    fi = s_fopen(BIDHASHNAME_OLD, "srb");
    fo = s_fopen(BHTMP, "swb");
    if (fo)
      s_fsetopt(fo, 1);
    if (fi && fo)
    {
      struct
      { short unsigned l;
        short unsigned h;
      } b={0, 0};
      while (fread(&b.l, 2, 1, fi))
        fwrite(&b, 4, 1, fo);
    }
    if (fo) s_fclose(fo);
    if (fi) s_fclose(fi);
    fi = s_fopen(BIDNAME_OLD, "srb");
    fo = s_fopen(BTMP, "swb");
    if (fo)
       s_fsetopt(fo, 1);
    if (fi && fo)
    {
      while (fread(&bo, sizeof(BID_OLD), 1, fi))
      {
        strcpy(b.name, bo.name);
        strcpy(b.filename, bo.filename);
        b.nextsamehash = bo.nextsamehash;
        fwrite(&b, sizeof(BID), 1, fo);
      }
    }
    if (fi) s_fclose(fi);
    if (fo) s_fclose(fo);
    xunlink(BIDNAME);
    xunlink(BIDHASHNAME);
    xrename(BTMP, BIDNAME);
    xrename(BHTMP, BIDHASHNAME);
  }
}
*/
/*---------------------------------------------------------------------------*/

void tmpbidrein (char *bid, int number)
//*************************************************************************
//
//  Locks a BID temporarily.
//  This prevents, that we accept a BID, which we are currently
//  receiving on another channel
//
//  (DH3MB)
//
//*************************************************************************
{
  strcpy(b->tmpbid[number], bid);
}

/*---------------------------------------------------------------------------*/

int tmpbidvorhanden (char *bid)
//*************************************************************************
//
//  Checks, if we receive a specific BID at the moment
//
//  (DH3MB)
//
//*************************************************************************
{
  unsigned int i, j;
  task_t *tt;

  for (i = 0; i < TASKS; i++)
  {
    tt = tfeld[i];
    if (tt)
    {
      tvar_t *bb = (tvar_t *) tt->b;
      if (bb)
        for (j = 0; j < 5; j++)
          if (! strcmp(bb->tmpbid[j], bid)) return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void tmpbidraus (int number)
//*************************************************************************
//
//  Deletes a temporary BID
//
//  (DH3MB)
//
//*************************************************************************
{
  b->tmpbid[number][0] = 0;
}

/*---------------------------------------------------------------------------*/

static void near listtmpbid (char *bid)
//*************************************************************************
//
//  Lists all temporary BIDs
//  (i.e. the BIDs, which we are receiving at the moment)
//  (DH3MB)
//
//*************************************************************************
{
  unsigned int i, j, firstfound;
  task_t *tt;

  putf(ms(m_incomingbids));
  putf("\n");
  for (i = 0; i < TASKS; i++)
  {
    tt = tfeld[i];
    if (tt)
    {
      tvar_t *bb = (tvar_t *) tt->b;
      if (bb)
      {
        firstfound = 1;
        for (j = 0; j < 5; j++)
        {
          if (bb->tmpbid[j][0] && strstr(bb->tmpbid[j], bid))
          {
            if (firstfound) putf("\n%-6s: ", bb->logincall);
            else putf("        ");
            putf("$%-12s", bb->tmpbid[j]);
            if (firstfound)
            {
              putf(" (%c %-6s > %-8s @ %.22s)",
                   bb->mailtype, bb->herkunft, bb->ziel, bb->at);
              firstfound = 0;
            }
            putv(LF);
          }
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

static void near tryopen (int first, fp &bidf, fp &bidhf)
//*************************************************************************
//
//  oeffnet Bid-File und Hash-File
//  ggf. werden die Files neu angelegt, wenn sie nicht geoeffnet
//  werden koennen
//
//*************************************************************************
{
  KOPF kopf = { "", 0, 0 }; // Speicher fuer BID-File Kopf

  bidf = s_fopen(BIDNAME, "sr+b");
  if (! bidf)
  {
    if (first)
    {
      bidf = s_fopen(BIDNAME, "sw+b");
      if (bidf)
      {
        s_fsetopt(bidf, 1);
        fwrite(&kopf, sizeof(KOPF), 1, bidf);
        xunlink(BIDHASHNAME);
      }
    }
    if (! bidf)
      trace(tr_abbruch, "BID-file", "fopen errno=%d %s",
                                    errno, strerror(errno));
  }
  bidhf = s_fopen(BIDHASHNAME, "sr+b");
  if (! bidhf)
  {
    if (first)
    {
      bidhf = s_fopen(BIDHASHNAME, "sw+b");
      if (bidhf) s_fsetopt(bidhf, 1);
    }
    if (! bidhf)
      trace(tr_abbruch, "BID-hash", "fopen errno=%d %s",
                                    errno, strerror(errno));
  }
  first = 0;
}

/*---------------------------------------------------------------------------*/

char *newbid (void)
//*************************************************************************
//
//  Erzeugt ein neues Bulletin-ID (aus Datum, Call und laufender Nummer)
//
//*************************************************************************
{
	static char bid[BIDLEN+1];
	struct tm *d;
	static int lastday = 0;
	static KOPF k = { "", 0, 0 };
	FILE *bidf;      // offenes BID-File
	FILE *bidhf;     // offenes BID-Hashfile
	char callbuf[7]; // DH3MB
	short unsigned lbid;
	byte year_multi;
	int year_base;

	d = ad_comtime(ad_time());
	if (!lastday)
		lastday = d->tm_mday;
	tryopen(0, bidf, bidhf);
	rewind(bidf);
	fread(&k, sizeof(k), 1, bidf);
	s_fclose(bidf);
	s_fclose(bidhf);

	if (lastday != d->tm_mday) {
	#ifndef _BCMNET_FWD
		k.lastbid = 36U * 36U * 8U * m.bidoffset;
	#else
		k.lastbid = 0; //CB-BCMNET: bidoffset not supported
	#endif
		lastday = d->tm_mday;
	} else { // on multi-bbs systems:
		 // if more than 10368 bid are needed, bid are no longer unique.
	#ifndef _BCMNET_FWD
	if (k.lastbid >= (36U * 36U * 36U))
		k.lastbid = 0; //= 46656 < 65535 (short unsigned) ok
	#else
	if (k.lastbid >= (36U * 36U))
		k.lastbid = 0;
	#endif
	}

	strcpy(callbuf, m.boxname);
	while (strlen(callbuf) < 6)
		strcat(callbuf, "_");
	do {
		lbid = ++k.lastbid;

		year_base = d->tm_year - 90;
		year_multi = (year_base > 35) ? year_base % 35 : 0;

	#ifndef _BCMNET_FWD
		// format:
		// <day><month/year_mutli><year><call><num>  <num>.. 0..9,A..Z
		// DMYCCCCCC123
		// 012345678901     0  1 2   9 0 1
		sprintf(bid,
			"%c%c%c%s%c%c%c",
			deznib(d->tm_mday),
			deznib((year_multi << 4) | (d->tm_mon + 1)),
			deznib(d->tm_year - 90),
			callbuf,
			deznib((lbid / (36 * 36))),
			deznib((lbid / 36) % 36),
			deznib((lbid) % 36));
	#else
		// the BID/MID for the CB-BCMNET is built as following
		// <day><year><month/year_multi><call>_<num>
		// DYMCCCCCC_23
		sprintf(bid, "%c%c%c%s_%c%c",
		deznib(d->tm_mday),
		deznib(d->tm_year - 90),
		deznib((year_multi << 4) | (d->tm_mon + 1)),
		callbuf,
		deznib((lbid / 36) % 36),
		deznib((lbid) % 36));
	#endif
	} while (bidvorhanden(bid));

	tryopen(0, bidf, bidhf);
	rewind(bidf);
	fwrite(&k, sizeof(k), 1, bidf);
	s_fclose(bidf);
	s_fclose(bidhf);

	return bid;
}

/*---------------------------------------------------------------------------*/

static void near updatehash (FILE *bidhf, long lindex, BID *bid)
//*************************************************************************
//
//  Traegt einen neuen Eintrag in die Hashtabelle ein
//  und verkettet ihn mit evtl bereits bestehenden Eintraegen
//  Dabei wird der neue Eintrag stets zu dem Eintrag, auf den
//  der Hashzeiger zeigt. Ggf. alte Eintraege werden vom neuen
//  Eintrag aus angesprochen.
//
//*************************************************************************
{
  long unsigned hsh = strcrc(bid->name); // CRC als Hashfunktion
  fseek(bidhf, hsh * HLEN, SEEK_SET); // bestehenden Hashzeiger lesen
  fread(&bid->nextsamehash, HLEN, 1, bidhf);
  fseek(bidhf, hsh * HLEN, SEEK_SET); // neuen Zeiger schreiben
  if (fwrite(&lindex, HLEN, 1, bidhf) != 1)
    trace(serious, "BID-upd", "fwrite $%s", bid->name);
}

/*---------------------------------------------------------------------------*/

char *bidvorhanden (char *bidname)
//*************************************************************************
//
// nur fuer Kompatibilitaet
//
//*************************************************************************
{
  return bidnewfilename(bidname, NULL);
}

/*---------------------------------------------------------------------------*/

char *bidnewfilename (char *bidname, char *newfilename)
//*************************************************************************
//
//  schaut nach, ob bidname schon im Bid-File enthalten ist
//  Zurueckgegeben wird ein damit verbundener Filename, falls das
//  BID gefunden worden ist, oder NULL wenn das BID nicht vorhanden ist
//
//*************************************************************************
{
  long unsigned hsh;
  long unsigned idx;
  static BID bidtmp;
  FILE *bidf = NULL;      // offenes BID-File
  FILE *bidhf = NULL;     // offenes BID-Hashfile

  if (! *bidname) return NULL;
  strupr(bidname);
  hsh = strcrc(bidname);
  tryopen(0, bidf, bidhf);
  if (*bidname)
  {
    fseek(bidhf, hsh * HLEN, SEEK_SET);
    if (fread(&idx, HLEN, 1, bidhf) != 1)
      trace(serious, "bidnewfilename", "hash %s", bidname);
    if (idx)
    {
      int num = 0;
      while (idx)
      {
        fseek(bidf, idx * sizeof(BID), SEEK_SET);
        if (fread (&bidtmp, sizeof(BID), 1, bidf) != 1)
        {
          if (! sema_test("purgereorg"))
            fork(P_BACK | P_MAIL, 0, mbreorg, "i");
          else
            reorg_noetig++;
          trace(fatal, "bidnewfilename", "fread %s", bidname);
        }
        if ((num++) > 100)
        {
          trace(serious, "bidnewfilename", "loop");
          s_fclose(bidf);
          s_fclose(bidhf);
          if (! sema_test("purgereorg"))
            fork(P_BACK | P_MAIL, 0, mbreorg, "i");
          else
            reorg_noetig++;
          return "";
        }
        if (! strcmp(bidname, bidtmp.name))
        {
          if (newfilename)
          {
            if (strlen(newfilename) > BID_FILENAMELEN)
            {
              trace(serious, "bidnewfilename", "file %s", newfilename);
              newfilename[BID_FILENAMELEN] = 0;
            }
            strcpy(bidtmp.filename, newfilename);
            fseek(bidf, idx * sizeof(BID), SEEK_SET);
            fwrite(&bidtmp, sizeof(BID), 1, bidf);
          }
          s_fclose(bidf);
          s_fclose(bidhf);
          return bidtmp.filename;
        }
        idx = bidtmp.nextsamehash;
      }
    }
  }
  s_fclose(bidf);
  s_fclose(bidhf);
  return NULL;
}

/*---------------------------------------------------------------------------*/

unsigned bidrein (char *bidname, char *filename)
//*************************************************************************
//
//  legt ein BID im bullid-File ab
//
//  Achtung: es muss an der Stelle sichergestellt sein, dass nicht
//           schon ein BID mit gleichem Namen existiert!
//
//*************************************************************************
{
  BID new_bid;
  FILE *bidf = NULL;  // offenes BID-File
  FILE *bidhf = NULL; // offenes BID-Hashfile

  strupr(bidname);
  if (*bidname)
  {
    tryopen(0, bidf, bidhf);
    memset(&new_bid, 0, sizeof(BID)); // neuen Eintrag vorbesetzen
    if (strlen(bidname) > BIDLEN)
    {
      trace(serious, "bidrein", "BID %s too long", bidname);
      bidname[BIDLEN] = 0;
    }
    if (strlen(filename) > BID_FILENAMELEN)
    {
      trace(serious, "bidrein", "filename %s too long", filename);
      filename[BID_FILENAMELEN] = 0;
    }
    strcpy(new_bid.name, bidname); // String hineinschreiben
    strcpy(new_bid.filename, filename);
    fseek(bidf, 0L, SEEK_END);
    updatehash(bidhf, ftell(bidf) / sizeof(BID), &new_bid);
    if (fwrite(&new_bid, sizeof(BID), 1, bidf) != 1)
      trace(serious, "bidrein", "fwrite $%s", new_bid.name);
    if ((ftell(bidf) / sizeof(BID)) > ((unsigned long) m.maxbids + 10000))
    {
      if (! sema_test("purgereorg"))
        fork(P_BACK | P_MAIL, 0, mbreorg, "i");
      trace(replog, "bidrein", "maxbids");
    }
    s_fclose(bidf);
    s_fclose(bidhf);
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static void near listhash (char *bidname)
//*************************************************************************
//
//  Gibt hashcode zu <bidname> aus
//
//*************************************************************************
{
  long unsigned hsh = strcrc(bidname);
  long unsigned idx;
  static BID bidtmp;
  FILE *bidf = NULL;  // offenes BID-File
  FILE *bidhf = NULL; // offenes BID-Hashfile

  putf("Hashcode=%04X\n", hsh);
  if (tmpbidvorhanden(bidname)) putf("BID in TMP buffer.\n");
  tryopen(0, bidf, bidhf);
  if (*bidname)
  {
    fseek(bidhf, hsh * HLEN, SEEK_SET);
    fread(&idx, HLEN, 1, bidhf);
    while (idx)
    {
      fseek(bidf, idx * sizeof(BID), SEEK_SET);
      fread(&bidtmp, sizeof(BID), 1, bidf);
      putf("%-6lu%17s:%-12s --> %lu\n",
           idx, bidtmp.filename, bidtmp.name, bidtmp.nextsamehash);
      idx = bidtmp.nextsamehash;
    }
  }
  s_fclose(bidf);
  s_fclose(bidhf);
}

/*---------------------------------------------------------------------------*/

void listbids (char *sel)
//*************************************************************************
//
//  Shows extracts of the BID-table
//
//  Options:
//     -c     Only counts the found entries
//     -k     Only lists entries with collisions
//     -h     gibt vollstaendige Hashverkettung aus
//     -s     Input a BID manually
//     -i     Lists the BIDs, which are currently received (DH3MB)
//     -f     Find a bid using hash; faster but no fragments allowed
//
//*************************************************************************
{
  BID bid;
  KOPF k;
  long count = 0;
  FILE *bidf;
  char fn[BID_FILENAMELEN+1];
  char *p;

  scanoptions(sel);
  sel += blkill(sel);
  strupr(sel);
  if (b->optplus & o_f)
  {
    p = bidnewfilename(sel, NULL);
    if (p)
    {
      safe_strcpy(fn,p);
      setfullpath(fn);
      putf("%s -> %s -> %s\n", sel, fn, b->mailpath);
      if (parse_headerline())
      {
        putf("Found: %c %-6s > %-8s @ %.22s\n",
             b->mailtype, b->herkunft, b->ziel, b->at);
      }
    }
    else
      putf(ms(m_noentryfound));
    return;
  }
  if (b->optplus & o_i) // DH3MB
  {
    listtmpbid(sel);
    return;
  }
  if (b->optplus & o_h) listhash(sel);
  else if (b->optplus & o_s)
  {
    if (! bidvorhanden(sel) && b->sysop)
      bidrein(sel, "DUMMY"); //df3vi: nicht fuer user!
  }
  else
  {
    bidf = s_fopen(BIDNAME, "srb");
    if (bidf)
    {
      fread(&k, sizeof(BID), 1, bidf); // delete first entry
      putf("Filesize=%lu lastbid=%u\n", nextpos(bidf), k.lastbid);
      while (fread(&bid, sizeof(BID), 1, bidf))
      {
        if ((! *sel || strstr(bid.filename, sel) || strstr(bid.name, sel))
           && (! (b->optplus & o_k) || bid.nextsamehash))
        {
          if (! (b->optplus & o_c))
          {
            putf("%-6lu", (ftell(bidf) / sizeof(BID)) - 1);
            putf("%17s:%-12s", bid.filename, bid.name);
            if (bid.nextsamehash)
            {
              long save = ftell(bidf);
              putf(" --> %ld", bid.nextsamehash);
              fseek(bidf, sizeof(BID) * (long) bid.nextsamehash, SEEK_SET);
              fread(&bid, sizeof(BID), 1, bidf);
              putf(" (%s) %ld", bid.name, bid.nextsamehash);
              fseek(bidf, save, SEEK_SET);
            }
            putv(LF);
          }
          count++;
        }
        if (testabbruch()) break;
        waitfor(e_ticsfull);
      }
      s_fclose(bidf);
    }
    putf(ms(m_entriesfound), count);
  }
}

/*---------------------------------------------------------------------------*/

void bidinit (int begin)
//*************************************************************************
//
//  Zugriff BID-File vorbereiten
//
//*************************************************************************
{
  FILE *bidf;     // offenes BID-File
  FILE *bidhf;    // offenes BID-Hashfile

  if (sizeof(BID) != sizeof(KOPF) || is_null)
    trace(tr_abbruch, "bidinit", "size %d %d", sizeof(BID), sizeof(KOPF));
  //obsolete if(begin) bidfile_newformat();
  tryopen(1, bidf, bidhf);
  if (! bidf)
  {
    trace(tr_abbruch, "bidinit", "fopen bidf errno=%d %s",
                                  errno, strerror(errno));
    return;
  }
  if (bidhf) hashinit(fileno(bidhf), HASH32);
  else
  {
    trace(tr_abbruch, "bidinit", "fopen bidhf errno=%d %s",
           errno, strerror(errno));
    return;
  }
  s_fclose(bidf);
  s_fclose(bidhf);
}

/*---------------------------------------------------------------------------*/

void reorgbid (void)
//*************************************************************************
//
//  Reorganises BID-file and generates the hashing file new.
//
//*************************************************************************
{
  FILE *balt, *bneu, *bhneu;
  int error;
  KOPF kopf = { "", 0, 0 };
  BID bid;
  long currpos, maxpos;
  unsigned e_nlen = 0, e_flen = 0;
  unsigned int len, i;

  if (! sema_lock("sendlock")) return;
  trace(replog, "reorgbid", "bidlist");
  lastcmd("BID-Reorg");
  unlink(BTMP); // remove old temporary files
  unlink(BHTMP);
  balt = s_fopen(BIDNAME, "lrb"); // the current BID file
  if (hd_space(filelength(fileno(balt))))
  {
    s_fclose(balt);
    trace(serious, "reorgbid", "disk full");
    return;
  }
  bneu=s_fopen(BTMP, "lw+b");   // open temporary write file
  if (bneu) s_fsetopt(bneu,1);
  bhneu = s_fopen(BHTMP, "lw+b");
  if (bhneu) s_fsetopt(bhneu, 1);
  if (balt && bneu && bhneu)
  {
    setvbuf(balt, NULL, _IOFBF, CBUFLEN); // buffer for increasing performance
    setvbuf(bneu, NULL, _IOFBF, CBUFLEN);
    hashinit(fileno(bhneu), HASH32); // fill new hash file with zero value
    maxpos = nextpos(balt);
    if (maxpos > m.maxbids) // if more than maxbids
      currpos = maxpos-m.maxbids; // the beginning of the file
    else currpos = 1;
    fread(&kopf, sizeof(KOPF), 1, balt); // read old header
    fwrite(&kopf, sizeof(KOPF), 1, bneu); // write first header
    fseek(balt, currpos * sizeof(BID), SEEK_SET);
    while (currpos != maxpos)
    {
      waitfor(e_ticsfull); // do not stand still
      error = 0;
      if (! fread(&bid, sizeof(BID), 1, balt)) break;
      len = strlen(bid.name);
      if (bid.name[BIDLEN] || len > BIDLEN)
      {
        error = 1;
        e_nlen++;
      }
      else
      {
        for (i = 0; i < len; i++)
        {
          if (bid.name[i] < '!' || bid.name[i] > 128) error = 1;
        }
      }
      len = strlen(bid.filename);
      if (bid.filename[BID_FILENAMELEN] || len > BID_FILENAMELEN || len < 3)
      {
        error = 1;
        e_flen++;
      }
      else
      {
        for (i = 0; i < len; i++)
        {
          if (bid.filename[i] < '!' || bid.filename[i] > 127) error = 1;
        }
      }
      if (! error)
      {
        fseek(bneu, 0L, SEEK_END);
        updatehash(bhneu, ftell(bneu) / sizeof(BID), &bid);
        fwrite(&bid, sizeof(BID), 1, bneu);
      }
      else
      {
        if ((e_nlen + e_flen) < 20)
          trace(report, "reorgbid", "er:%1.20s:%1.20s",
                                     bid.name, bid.filename);
      }
      if (! (currpos % 10000))
      {
        char s[40];
        sprintf(s, "BID %lu/%lu:%s", currpos, maxpos-1, bid.name);
        trace(report, "reorgbid", s); // damits dem Zuschauer nicht fad wird
        lastcmd(s);                   // ... und auch im PS auftaucht
      }
      currpos++;
    }
    currpos = nextpos(bneu);
    //if (currpos) currpos--; dh8ymb
    trace(replog, "reorgbid", "%lu bids, %u/%u errors",
                               currpos, e_nlen, e_flen);
    s_fclose(balt);
    s_fclose(bneu);
    s_fclose(bhneu);
    xunlink(BBAK);
    xunlink(BHBAK);
    xrename(BIDNAME, BBAK);
    xrename(BIDHASHNAME, BHBAK);
    xrename(BTMP, BIDNAME);
    xrename(BHTMP, BIDHASHNAME);
    sema_unlock("sendlock");
  }
}

/*---------------------------------------------------------------------------*/
