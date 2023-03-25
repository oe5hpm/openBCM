/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------------------------
  Erzeugung und Behandlung des Bulletin-Baumes
  --------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980124 OE3DZW text from "Maximum" to "Limit" in status limits
//                added +10k to maxbids, is caused stress
//19980202 OE3DZW fixed bug when reloading empty convname/life
//19980205 OE3DZW removed debug output
//19980913 OE3DZW users will usually not see temporery boards
//19991125 DF3VI  Changed nameconvert() to handle unknown boards
//19991129 DF3VI  Added header to "ced list"
//20000116 DK2UI  (dzw) added ui-options
//20000415 DF3VI  added CONVFROM.BCM
//20030323 DH8YMB removed all old CONVNAME/CONVFROM
//20030508 DH8YMB falls in bulletin.bcm mehr als 1 Blank, Zeile ignorieren

#include "baycom.h"
//#include "convert.cpp"

/*---------------------------------------------------------------------------*/

unsigned treelen = 0;
bulletin_t *tree = NULL;

/*---------------------------------------------------------------------------*/

void limit_list (void)
//*************************************************************************
//
//  ST L ausgeben
//
//*************************************************************************
{
  struct dirent *di;
  DIR *d = opendir(m.userpath);
  unsigned umailcalls = 0;

  while ((di = readdir(d)) != NULL)
    if (*di->d_name != '.') umailcalls++;
  closedir (d);
  putf("Item          Current      Limit\n");
  putf("boardnames  %8u %11u\n", treelen, MAXTREEENTRIES);
  putf("convert     %8u %11u\n", cvtlen, MAXCONVNAME);
  putf("fwdpartners %8u %11u\n", fwdpartners, MAXFWD);
  putf("fwddest     %8u %11u\n", fwddest, MAXDEST);
  putf("fwddestlen  %8u %11u\n", fwddestlen, MAXDESTLEN);
  putf("h-addrs     %8lu %11lu\n", hadr_len(0), hadr_len(1));
  putf("bids        %8lu %11lu\n", bid_len(), m.maxbids + 10000);
  putf("users       %8lu         -\n", filesize(USERNAME) / sizeof(user_t));
  putf("infomails   %8lu %11lu\n", filesize(CHECKNAME) >> 7,
        (filesize(CHECKNAME) >> 7) + (dfree(m.infopath, 0) >> 3));
  putf("u-mailcalls %8u %11u\n", umailcalls, MAXUSERMAILS);
  putf("languages   %8u %11u\n", msg_limit(1), msg_limit(0));
}

/*---------------------------------------------------------------------------*/

static void near intree (struct dirent *p, char *path, char *mask)
//*************************************************************************
//
//  Legt eine Zeile des Directorybaumes im tree-Array ab
//
//*************************************************************************
{
  char fname[256 + 1 + 1];

  sprintf(fname, "%s/%s", mask, p->d_name);
  if (isdir(fname) && *p->d_name != '.')
  {
    strcpy(tree[treelen].name, path);
    if (path[0])
      strcat(tree[treelen].name, "/");
    strcat(tree[treelen].name, p->d_name);
    if (strlen(tree[treelen].name) >= MAXTREELENGTH)
      trace(tr_abbruch, "intree", "name too long");
    strupr(tree[treelen].name);
    char *sl = strrchr(tree[treelen].name, '/');
    if (sl)
      tree[treelen].flatoffset = (byte) (sl + 1 - tree[treelen].name);
    else
      tree[treelen].flatoffset = 0;
    tree[treelen].newestmail = 0;
    tree[treelen].lifetime_min = 1;
    tree[treelen].lifetime_max = 999;
    treelen++;
    if (treelen > MAXTREEENTRIES)
      trace(tr_abbruch, "intree", "too many boards");
  }
}

/*---------------------------------------------------------------------------*/

static void near search (char *path, char *root)
//*************************************************************************
//
//  Durchsucht das aktuelle Directory nach weiteren Subdirectories
//
//*************************************************************************
{
  char mask[80];
  struct dirent *p;
  DIR *d;

  strcpy(mask, root);
  if (*path)
  {
    strcat(mask, "/");
    strcat(mask, path);
  }
  strlwr(mask);
  d = opendir(mask);
  while (d && (p = readdir(d)) != NULL)
  {
    waitfor(e_ticsfull);
    intree(p, path, mask);
  }
  closedir(d);
  waitfor(e_ticsfull);
}

/*---------------------------------------------------------------------------*/

static void near treesort (void)
//*************************************************************************
//
//  Sortiert den Baum alphabetisch
//
//*************************************************************************
{
  qsort((void *) tree, treelen, sizeof(bulletin_t),
        (int (*)(const void *, const void *)) strcmp);
  tree = (bulletin_t *) t_realloc(tree, sizeof(bulletin_t) * (treelen + 2));
}

/*---------------------------------------------------------------------------*/

static int near mbtreeload (void)
//*************************************************************************
//
//  Laedt eine Bulletin-Struktur von der Datei "bulletin.bcm"
//  Rueckgabe: Bulletin-Liste gueltig
//  (ignoriert die Verzeichnisse auf der Platte)
//
//*************************************************************************
{
  FILE *f;
  char s[40];
  char *name, *life, *n2;
  int blanks, linesok = 0;
  char lastpath[MAXTREELENGTH];
  bulletin_t *tr = NULL;

  lastpath[0] = 0;
  f = s_fopen(TREENAME, "srt");
  if (f)
  {
    while (fgets(s, sizeof(s) - 1, f))
    {
      if (*s != ';')
      {
        blanks = 0;
        strupr(s);
        name = s;
        while (name[0] == ' ')
        {
          name++;
          blanks++;
        }
        life = name;
        while (isgraph(life[0])) life++;
        if (life[0]) life[0] = 0;
        life++;
        life += blkill(life);
        if (strlen(name) > 8) name[8] = 0;
        cut_blank(name);
        if (blanks > 1)
        {
          trace(serious, "mbtreeload", "bulletin.bcm corrupt line: %s", name);
          continue;
        }
        if (finddir(name, 1) > 0) continue;
        tr = &tree[treelen];      // zum einfacheren Zugriff
        tr->lifetime_max = nextdez(&life);
        tr->lifetime_min = nextdez(&life);
        if (tr->lifetime_max > 999)
          tr->lifetime_max = 999;
        if (tr->lifetime_min > 999)
          tr->lifetime_min = 999;
        if (tr->lifetime_max == 0)
          tr->lifetime_max = 999; // fuer alte Mails/bulletin.bcm
        if (tr->lifetime_max < tr->lifetime_min)
          tr->lifetime_min = tr->lifetime_max;
        if (name[0] && (strlen(name) < 9))
        {
          linesok++;
          n2 = lastpath;
          while (blanks)
          {
            if (n2[0] == '/') n2++;
            while (isalnum(n2[0])) n2++;
            blanks--;
          }
          n2[0] = 0;

          if (lastpath[0]) strcat(lastpath, "/");
          strcat(lastpath, name);
          if (strlen(lastpath) >= MAXTREELENGTH)
          {
            trace(serious, "mbtreeload", "path %s too long", lastpath);
            continue;
          }
          strcpy(tr->name, lastpath);
          char *sl = strrchr(tr->name, '/');
          if (sl)
            tr->flatoffset = (byte) (sl + 1 - tr->name);
          else
            tr->flatoffset = 0;
          tr->newestmail = 0;
          treelen++;
          if (treelen >= MAXTREEENTRIES)
          {
            trace(serious, "mbtreeload", "too many directories");
            treelen--;
            break;
          }
        }
      }
    }
    s_fclose(f);
    return linesok;
  }
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

void mbtreesave (void)
//*************************************************************************
//
//  Speichert den Bulletin-Baum auf Platte ab
//  Dabei wird darauf geachtet, dass zu keinem Zeitpunkt ein
//  fehlerhaftes File auf der Platte ist.
//
//*************************************************************************
{
  FILE *f;
  unsigned int i;
  char *slash, *name;
  int blanks;
  char nbuf[12];

  treesort();
  if ((f = s_fopen(TREETMPNAME, "swt")) != NULL)
    {
      if (fprintf(f, "; " STD_BOXHEADER " Bulletin Listfile\n;\n") < 0)
        goto fehler;
      for (i = 0; i < treelen; i++)
        {
          blanks = 0;
          name = tree[i].name;
          while ((slash = strchr(name, '/')) != NULL)
            {
              name = slash + 1;
              blanks++;
            }
          nbuf[blanks] = 0;
          while (blanks) nbuf[--blanks] = ' ';
          strcat(nbuf, name);
          strlwr(nbuf);
          if (fprintf(f, "%-10s%3d %3d\n",
                        nbuf, tree[i].lifetime_max, tree[i].lifetime_min) < 0)
            goto fehler;
        }
      s_fclose(f);
      xunlink(TREEBAKNAME);
      xrename(TREENAME, TREEBAKNAME);
      xrename(TREETMPNAME, TREENAME);
    }
  else
    trace(serious, "mbtreesave", "fopen %s errno=%d %s",
                                  TREETMPNAME, errno, strerror (errno));
  return;
fehler:
  trace(serious, "mbtreesave", "write %s errno=%d %s",
                                TREETMPNAME, errno, strerror (errno));
  s_fclose(f);
}

/*---------------------------------------------------------------------------*/

int rmboard (char *fullname)
//*************************************************************************
//
//  Entfernt ein Board. Der volle Pfadname muss angegeben werden
//  Return:
//    0     ok
//    1     Board nicht gefunden
//    2     Board nicht leer
//
//*************************************************************************
{
  unsigned i = 0;
  char path[FNAMELEN + 1 + 1];
  DIR *d;
  struct dirent *di;

  strupr(fullname);
  while (strcmp(tree[i].name, fullname))
  {
    i++;
    if (i >= treelen) return 1;
  }
  sprintf(path, "%s/%s", m.infopath, fullname);
  strlwr(path);
  d = opendir(path);
  while (d && (di = readdir(d)) != NULL)
  {
    if (*di->d_name != '.')
    {
      closedir(d);
      return 2;
    }
  }
  closedir(d);
                    //  12345678
  strcpy(tree[i].name, "zzzzzzzz");
  tree[i].name[0] = (char) 0xff;
  treesort();
  treelen--;
  mbtreesave();
  return 0;
}

/*---------------------------------------------------------------------------*/

int mkboard(char *mainboard, char *newboard, int sameboard)
//*************************************************************************
//
//  Erzeugt ein neues Board. Angegeben wird
//  - Das Hauptboard (oder / wenn ein Hauptboard eingerichtet wird)
//  - Das Subboard (ohne /)
//
//  Return:
//    0     ok
//    1     Hauptboard nicht gefunden
//    2     Baum voll
//    3     Board schon vorhanden
//    4     Boardname ungueltig
//
//*************************************************************************
{
	unsigned i = 0;
	char name[40];
	char *sl;
	char *mboard;
	char *nboard;

	if (treelen >= MAXTREEENTRIES) {
		trace(serious, "mkboard", "too many boards %s", newboard);
		return 2; /* tree full */
	}

	mboard = strdup(mainboard);
	nboard = strdup(newboard);
	strupr(mboard);
	strupr(nboard);

	if (!*nboard || *nboard == '/' ||
	    strlen(nboard) > 8 || strlen(mboard) > 20) {
		free(nboard);
		free(mboard);

		return 4; /* board invalid */
	}

	if (!sameboard && finddir(nboard, 1)) {
		free(mboard);
		free(nboard);

		return 3; /* board exists */
	}

	if (*mboard == '/')
		*mboard = 0;
	*name = 0;
	if (*mboard) {
		while (strcmp(tree[i].name, mboard))
			if (++i >= treelen) {
				free(mboard);
				free(nboard);

				return 1; /* mainboard not found */
			}
		strncpy(name, mboard, 20);
		name[20] = 0;
		strcat(name, "/");
	}
	strcat(name, nboard);
	strcpy(tree[treelen].name, name);
	sl = strrchr(tree[treelen].name, '/');
	if (sl)
		tree[treelen].flatoffset = (byte) (sl + 1 - tree[treelen].name);
	else
		tree[treelen].flatoffset = 0;
	tree[treelen].newestmail = 0;
	tree[treelen].lifetime_min = 1;
	tree[treelen].lifetime_max = m.infolife;
	treelen++;
	treesort();
	mbtreesave();
	free(mboard);
	free(nboard);

	return 0;
}

/*---------------------------------------------------------------------------*/

int mvboard (char *old, char *sub, char *newboard)
//*************************************************************************
//
//  Benennt ein Board um. Dabei werden alle Files in dem angegebenen
//  Board in das neue Board verschoben.
//  Sowohl bei old als auch bei new sind die kompletten Boardnamen
//  (main und sub) anzugeben, z.B. SOFTWARE/BAYCOM
//
//  Return:
//   0  alles ok
//   1  neues Hauptboard nicht gefunden
//   2  Baum voll
//   4  altes Board nicht gefunden
//
//*************************************************************************
{
  char oldpath[FNAMELEN + 2 + 1];
  char newpath[FNAMELEN + 1 + 1];
  char oldbname[30]; // is this save?
  char dirpath[FNAMELEN+1];
  int oldmask, newmask;
  unsigned int i = 0;
  DIR *d;
  struct dirent *di;

  strupr(old);
  strupr(sub);
  if (*old == '/')
    *old = 0;
  else
    strcat(old, "/");
  snprintf(oldbname, sizeof(oldbname), "%s%s", old, sub);
  while (strcmp(tree[i].name, oldbname))
    if (++i >= treelen)
      return 4;
  i = mkboard(newboard, sub, 1);
  if (i)
    return i;
  if (*newboard)
    strcat(newboard, "/");
  snprintf(oldpath, sizeof(oldpath), "%s/%s%s/", m.infopath, old, sub);
  snprintf(newpath, sizeof(newpath), "%s/%s%s", m.infopath, newboard, sub);
  xmkdir(newpath);
  strcat(newpath, "/");
  oldmask = strlen(oldpath);
  newmask = strlen(newpath);
  strcpy(dirpath, oldpath);
  dirpath[oldmask - 1] = 0;
  set_watchdog(2);
  strlwr(dirpath);
  if (filetime(dirpath))
  {
    d = opendir(dirpath);
    while ((di = readdir(d)) != NULL)
    {
      if (*di->d_name != '.')
      {
        strncpy(oldpath + oldmask, di->d_name, sizeof(oldpath) - oldmask);
        oldpath[sizeof(oldpath) - 1] = 0;
        strncpy(newpath + newmask, di->d_name, sizeof(newpath) - newmask);
        newpath[sizeof(newpath) - 1] = 0;
        strlwr(oldpath);
        strlwr(newpath);
        if (xrename(oldpath, newpath))
          trace(serious, "mvboard", "rename");
        waitfor(e_ticsfull);
      }
    }
    closedir(d);
  }
  set_watchdog(1);
  rmemptydir(dirpath);
  rmboard(oldbname);
  return 0;
}

/*---------------------------------------------------------------------------*/

void mbreadtree (char *beginpath)
//*************************************************************************
//
//  Liest einen kompletten Directorybaum ab 'beginpath' ein,
//  speichert ihn im globalen Array tree und sortiert ihn
//  Falls die Datei bulletin.bcm existiert, werden die Informationen
//  aus dieser Datei uebernommen.
//
//*************************************************************************
{
  unsigned i, j, k = 0;

  treelen = 0;
  if (! tree)
    tree = (bulletin_t*) t_malloc(sizeof(bulletin_t) * MAXTREEENTRIES, "*tre");
  if (! mbtreeload())
  {
    search("", beginpath);
    do
    {
      j = treelen;
      for (i = k; i < j; i++)
        search(tree[i].name, beginpath);
      k = j;
    }
    while (treelen > j);
    mbtreesave();
  }
  treesort();
  trace(report, "readtree", "%d bulletins", treelen);
  if (*b->logincall)
    putf("%d boards.\n", treelen);
}

/*---------------------------------------------------------------------------*/

