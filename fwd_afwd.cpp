/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------
  Automatische Berechnung von Forwardpfaden
  -----------------------------------------


  Copyright (C)     Jan Wasserbauer


  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980618 OE3DZW -1 -> MAXBEREICH (neg value to unsigned variable)
//19980618 OE3DZW fixed defines for uclose, hadr_tryopen, oadhadr
//19981005 jj     search in infobase only manual, fwdlist-debugging
//19990818 OE3DZW Bugfix, now returns static strings
//19991218 Jan    fwdlist generates afwd.bcm (not used yet), clean-up
//20000110 Jan    (dzw) added fixes
//20000130 Jan    removed CHECK-file scan, finished afwd.bcm handling
//20000604 Jan    fwd data alloc'd dynamically, fixed some string lengths

#include "baycom.h"
#ifdef _AUTOFWD


//hadr.cpp
//extern void   uclose (void);
//extern void   hadr_tryopen (void);
//extern int    loadhadr (char *hadrcall, hadr_t *u, int anlegen);

//fwd.cpp
extern void strip_hadr (char *header);

typedef struct afwdpara_t
{ char    call[CALLEN+1];      // Rufzeichen der Nachbarboxen
  char    *dest;               // Index in boxdest[]-Liste
} afwdpara_t;

typedef struct afwd_data
{ char partner[CALLEN+1];
  char shortaddr[HADRESSLEN+1];
} afwd_data;

static afwdpara_t *afp = NULL;
static char *aboxdest = NULL;

unsigned afwdpartners = 0;

/*---------------------------------------------------------------------------*/

static int which_partner (char *call, hadr_t *ha)
//*************************************************************************
//
// decide which partner should be used for fwd from HA entry
// call param just for -C fwd opt. checking, may be NULL
//
//*************************************************************************
{
  time_t upd = 0;
  time_t oldest = ad_time() - m.autofwdtime * DAY;
  int num = -1, i;
  int hops = 1000;
  time_t del = ad_time();
  char oldcall[CALLEN+1];
  fwdpara_t *ff;

  if (call) strcpy(oldcall, b->logincall);
  for (i = 0; i < ADJNUM; i++)
  {
    if (   ha->adjacent[i][0] && isforwardpartner (ha->adjacent[i]) != NOTFOUND
        && ha->rel_mails[i] > 2 //at least two mails
        && ha->lastupdate[i] > oldest  //new enough
        && ha->delay[i] < del     //path with minimal delay, 5min tolerance
        && (ha->hops[i] < hops   //and minimal number of hops
            || (ha->hops[i] == hops && ha->lastupdate[i] > upd)))
                                      //or newer entry for same hop count
    {
      if (call)
      {
        strcpy(b->logincall, ha->adjacent[i]);
        ff = fpara();
        if (ff->options & o_c)
        {
#ifdef _LCF
          if (! fwdcallok(call, ff->fwdcallformat)
              || (*b->herkunft && ! fwdcallok(b->herkunft, ff->fwdcallformat)))
#else
          if (! mbcallok(call)
              || (*b->herkunft && ! mbcallok(b->herkunft)))
#endif
            continue;
        }
      }
      upd = ha->lastupdate[i];
      del = ha->delay[i] + 300; //5min tolerance
      hops = ha->hops[i];
      num = i;
    }
  }
  if (call) strcpy(b->logincall, oldcall);
  return num;
}

/*---------------------------------------------------------------------------*/

int autohadr (char *call, int ausgabe)
//*************************************************************************
//
// Search for address in HADR database
//
//*************************************************************************
{
  lastfunc("autohadr");
  int pos;
  int i;
  bitfeld options = b->optplus;
  int which = -1;
  hadr_t *ha = (hadr_t*) t_malloc(sizeof(hadr_t), "hadr");

  if (m.hadrstore)
  {
    strupr(call);
    hadr_tryopen();
    pos = loadhadr(call, ha, 0);
    uclose();
    if (pos)
    {
      which = which_partner(call, ha);
      if (ausgabe && (options & o_a))
      {
        time_t mindelay = ad_time();
        time_t mtime = ad_time();
        putf("Forward data from HADR-entry:\n");
        putf("H-Addr     : %s\n", ha->adr);
        putf("R-Header   : %s\n", ha->lastheader);
        putf("Last BID   : %s\n", ha->lastbid);
        putf("Last Board : %s\n", ha->lastboard);
        putf("Last User  : %s\n", ha->lastuser);
        putf("Header Date: %s\n", datestr(ha->lasthtime, 12));
        if (*ha->sysopcall)
          putf("Sysop:       %s %s\n", ha->sysopcall,
                                       get_name(ha->sysopcall, 1));
        if (*ha->protocol && *ha->hwaddress)
          putf("HW-Address:  %s: %s\n", ha->protocol, ha->hwaddress);
        if (ha->bversion && *ha->bstatus && ha->lastwprcvd)
          putf("BVersion:    %ld %s (%s)\n", ha->bversion, ha->bstatus,
                                             datestr(ha->lastwprcvd, 12));
        putf("Bulletins  : %ld\n", ha->bulletins);
        putf("Usermails  : %ld\n", ha->usermails);
        putf("Neighbour Count Update +Delay Hops\n");
        for (i = 0; i < ADJNUM; i++)
        {
          if (ha->adjacent[i][0])
          {
            if (ha->delay[i] < mindelay) mindelay = ha->delay[i];
          }
        }
        for (i = 0; i < ADJNUM; i++)
        { // Ausgabe HADR-Eintrag
          if (ha->adjacent[i][0])
          {
            putf("%-9s%6d%7s%7s%5d", ha->adjacent[i], ha->rel_mails[i],
                  zeitspanne(mtime - ha->lastupdate[i], zs_seconds),
                  zeitspanne(ha->delay[i] - mindelay, zs_seconds),
                  ha->hops[i]);
            if (i == which) putf(" <");
            if (isforwardpartner(ha->adjacent[i]) == NOTFOUND)
              putf(" fwd terminated");
            putv(LF);
          }
        }
        putv(LF);
      }
    }
  }
  if (which != NOTFOUND) strcpy(b->destboxlist, ha->adjacent[which]);
  t_free(ha);
  if (which == NOTFOUND) return unbekannt;
  return auto_bekannt;
}
/*---------------------------------------------------------------------------*/

static int autofile (char *call, int ausgabe)
//*************************************************************************
//
// Search in afwd.bcm
//
//*************************************************************************
{
  lastfunc("autofile");
  char hadr[HADRESSLEN+1];
  char tmp[HADRESSLEN+1];
  unsigned int box;

  strcpy(hadr, call);
  if (ausgabe)
  {
    expand_hadr(hadr, 2);
    strip_hadr(hadr);
  }
  // Suche nach am Ende nach Bezeichnern, die in der Destinations-Liste stehen
  for (box = 0; box < afwdpartners; box++)
  {
    char *ds = afp[box].dest;
    char *bl;
    char *found;
    unsigned int adrend = strlen(hadr);
    unsigned int len;
    while ((bl = strchr(ds, ' ')) != NULL)
    {
      strncpy(tmp, ds, HADRESSLEN);
      tmp[HADRESSLEN] = 0;
      found = strchr(tmp, ' ');
      if (found && found != tmp)
      {
        *found = 0;
        if (*tmp == '.' || mbhadrok(tmp) == 1)
        {
          len = strlen(tmp);
          if (len <= adrend && ! strcmp(hadr + (adrend - len), tmp))
          {
            strcpy(b->destboxlist, afp[box].call);
            return auto_bekannt;
          }
        }
      }
      ds = bl + 1;
    }
  }
  return unbekannt;
}

/*---------------------------------------------------------------------------*/

int autofwd (char *call, int ausgabe)
//*************************************************************************
//
//*************************************************************************
{
  lastfunc("autofwd");
  int ret = unbekannt;
  bitfeld options;

  if (! afp || ! aboxdest) return unbekannt;
  if (ausgabe)
  {
    scanoptions(call);
    options = b->optplus;
  }
  else
    options = 0;
  strupr(call);
  ret = autofile(call, ausgabe);
  call += blkill(call);
  if ((options & o_a) && ret == auto_bekannt)
    putf("Autopath: Found %s in "AFWDLISTNAME", using autofwd to %s.\n",
                                                call, b->destboxlist);
  else
  {
    if (ret == unbekannt)
    {
      if (*call && ! (strcmp(call,m.boxname)))
        ret = unbekannt; // eigenes Boxcall ausschliessen
      else
        ret = autohadr(call, ausgabe); //only really new adresses may not be found
    }
    if (ausgabe)
    {
      if (ret == auto_bekannt)
        putf("Autopath: forward %s to %s seems to be possible for autorouter.\n",
             call, b->destboxlist);
      else
        putf("Autopath: Autorouter found no path for %s\n", call);
    }
  }
  return ret;
}

/*---------------------------------------------------------------------------*/

int data_cmp (const void *a, const void *c)
//*************************************************************************
//
// for qsort
//
//*************************************************************************
{
  return strcmp(((afwd_data*) a)->shortaddr, ((afwd_data*) c)->shortaddr);
}

/*---------------------------------------------------------------------------*/

char *reverse_addr (char *addr)
//*************************************************************************
//
//reverses (short)HADR: "DB0AAB.#BAY.DEU" -> ".DEU.#BAY.DB0AAB"
//
//*************************************************************************
{
  char *c;
  char tmp[HADRESSLEN+1];

  *tmp = 0;
  while ((c = strrchr(addr, '.')) != NULL)
  {
    strcat(tmp, c + 1);
    strcat(tmp, ".");
    *c = 0;
  }
  if (*addr) strcat(tmp, addr);
  strcpy(addr, tmp);
  return addr;
}

/*---------------------------------------------------------------------------*/

static int process_afwd_data (afwd_data *adrs, unsigned count)
//*************************************************************************
//
// goes through sorted HADRs list and checks for same areas fwded to same
// neighbour
//
//*************************************************************************
{
  unsigned int i = 0, j, move = 0;
  char tmp[HADRESSLEN+1];
  char *x;
  char *y;
  char *cur;
  char ok;

  *tmp = 0;
  while (i < count)
  {
    cur = adrs[i].shortaddr;
    x = cur;
    while (*tmp)
    {
      if (! strncmp(cur, tmp, strlen(tmp)))
      {
        x = cur + strlen(tmp) + 1;
        break;
      }
      y = strrchr(tmp, '.');
      if (y) *y = 0;
      else *tmp = 0;
    }
    y = strchr(x, '.');
    if (! y) //BBS in our local area
    {
      if (move != i) memcpy(adrs + move, adrs + i, sizeof(afwd_data));
      move++;
      i++;
      continue;
    }
    while (y)
    {
      safe_strncpy(tmp, cur, ((unsigned int) (y - cur) + 2));
                        //we want (y-cur) + "." + 0(by safe_strncpy)
      j = i + 1;
      ok = 1;
      while (! strncmp(adrs[j].shortaddr, tmp, strlen(tmp)))
      {
        if (strcmp(adrs[j].partner, adrs[i].partner))
        {
          ok = 0;
          break;
        }
        j++;
      }
      if (ok)
      {
        strcpy(adrs[i].shortaddr, tmp);
        if (move != i) memcpy(adrs + move, adrs + i, sizeof(afwd_data));
        move++;
        i = j - 1;
        char *dot = strrchr(tmp, '.');
        if (! dot) trace(fatal, "process_afwd_data", "no dot in addr");
        else *dot = 0; //there should always be at least one dot
        break;
      }
      x = y;
      y = strchr(x + 1, '.');
    }
    if (! y)
    {
      if (move != i) memcpy(adrs + move, adrs + i, sizeof(afwd_data));
      move++;
    }
    i++;
  }
  return move;
}

/*---------------------------------------------------------------------------*/

static int check_adr (char *adr)
//*************************************************************************
//
//check for valid addr, replace .EURO, remove .WW
//corrupted addresses do nasty things to afwd
//
//*************************************************************************
{
  char *x = strrchr(adr, '.');
  char *y;
  if (! x) return 0;
  //no adress, only boxcall - corrupt R: line
  //(could also be test-something - sysop should add it to fwd.bcm in that case)
  for (int i = 0; adr[i]; i++)
    if (! isalnum(adr[i]) && adr[i] != '.' && adr[i] != '#')
      return 0; //corrupt address ..
  //oh well .. we have to replace stupid addresses ..
  if (! strcmp(x, ".WW"))
  {
    *x = 0;
    if (! (x = strrchr(adr, '.'))) return 0;
  }
  if (! strcmp(x, ".EURO")) *(x + 3) = 0;
  unsigned int cnt = 1, maxlen = 0;
  if (! (y = strchr(adr, '.'))) return 0;
  y++;
  x = strchr(y, '.');
  while (x)
  {
    cnt++;
    if ((unsigned int) (x - y) > maxlen)
      maxlen = (unsigned int) (x - y);
    y = x + 1;
    x = strchr(y, '.');
  }
  if (cnt < 2 || cnt > 6)
    return 0; //adr must have 2-6 dots
  if (! maxlen || maxlen > 7)
    return 0; // and no part must be longer than 6 chars (+dot)
  return 1;
}

/*---------------------------------------------------------------------------*/

static int scan_hadr ()
//*************************************************************************
//
// process HADR database, find entries newer than autofwdtime,
// write afwd1.tmp
//
//*************************************************************************
{
  hadr_t *ha;
  unsigned int count = 0, found = 0, i = 0;
  char boxcall[CALLEN+1];
  char *header;
  char shortheader[HADRESSLEN+1];
  char name[20];
  int num;

  strcpy(name, "afwdlist");
  FILE *f = s_fopen(HADRNAME, "lrb");
  if (! f)
  {
    trace(serious, name, ms(m_filenotopen), HADRNAME);
    putf(ms(m_filenotopen), HADRNAME);
    return 0;
  }
  FILE *g = s_fopen(AFWDTMP, "lwt");
  if (! g)
  {
    trace(serious, name, ms(m_filenotopen), AFWDTMP);
    putf(ms(m_filenotopen), AFWDTMP);
    s_fclose(f);
    return 0;
  }
  ha = (hadr_t*) t_malloc(sizeof (hadr_t), "hadr");
  setvbuf(f, NULL, _IOFBF, 4096); // buffer for increasing performance
  fseek(f, sizeof (hadr_t), SEEK_SET);
  while (fread(ha, sizeof(hadr_t), 1, f))
  {
    if (! *ha->adr) continue;
    if (! strcmp(ha->adr, m.boxadress)) continue;
    count++;
    if (! check_adr(ha->adr)) continue;
    if ((num = which_partner(NULL, ha)) == NOTFOUND) continue;
    // Forwardweg suchen
    //no hadr, only boxcall (corrupt R: lines etc.)
    //(should not pass through check_adr() anyway)
    if (! (header = strchr(ha->adr, '.'))) continue;
    i = header - ha->adr;
    strcpy(shortheader, ha->adr);
    strip_hadr(shortheader);
    if (i > CALLEN) i = CALLEN;
    strncpy(boxcall, ha->adr, i);
    boxcall[i] = 0;
    if (mbcallok(boxcall))
    {
      found++;
      // putf("%i) %s%s -> %s to %s\n", count, boxcall, header, shortheader, fwdcall);
      // Routerfile schreiben
      fprintf(g, "%s %s \n", ha->adjacent[num], shortheader);
    }
    waitfor(e_ticsfull);
    if (testabbruch()) break;
  }
  t_free(ha);
  s_fclose(f);
  s_fclose(g);
#ifdef DEBUG_AFWD
  trace(report, name, "%u searched,  %u found", count, found);
#endif
  putf("%u searched, %u found\n", count, found);
  return found;
}

/*---------------------------------------------------------------------------*/

void write_afwd (FILE *f, afwd_data *adrs, int count)
//*************************************************************************
//
// write data to afwd.bcm
//
//*************************************************************************
{
  char fp[CALLEN+1];
  int i;
  int len;

  while (1)
  {
    i = 0;
    while (! adrs[i].partner[0] && i < count) i++;
    if (i == count) break;
    strcpy(fp, adrs[i].partner);
    fprintf(f, "%s\n", fp);
    len = 0;
    while (i < count)
    {
      if (adrs[i].partner[0] && ! strcmp(fp, adrs[i].partner))
      {
        len += strlen(adrs[i].shortaddr) + 1;
        if (len > 40)
        {
          fputc(LF, f);
          len = strlen(adrs[i].shortaddr) + 1;
        }
        fprintf(f, " %s", reverse_addr(adrs[i].shortaddr));
        adrs[i].partner[0] = 0;
      }
      i++;
    }
    fputc(LF, f);
  }
}

/*---------------------------------------------------------------------------*/

void afwdlist (char *)
//*************************************************************************
//
// afwd.bcm is generated here ..
//
//*************************************************************************
{
  char name[20];

  strcpy(name, "afwdlist");
  if (! m.hadrstore || ! m.autofwdtime) return; //afwd disabled
  // Alle boxen durchsuchen
//  trace(report, "afwdlist", "scanning HADR");
  unsigned int found = 0, i = 0;
  if (! (found = scan_hadr())) return;
  // Sort file by reversed short addr
  char linetmp[LINELEN+1];
  char *xx;
  afwd_data *adrs = (afwd_data*) t_malloc(sizeof(afwd_data) * found, "hadr");
  if (! adrs)
  {
    trace(serious, name, "out of memory");
    putf("not enough memory");
    return;
  }
#ifdef DEBUG_AFWD
  trace(report, name, "reading entries in memory");
#endif
  FILE *f = s_fopen(AFWDTMP, "srt");
  if (f)
  {
    while (fgets(linetmp, LINELEN, f))
    {
      //fwdcall, shortheader
      xx = strchr(linetmp, ' ');
      *xx++ = 0;
      strcpy(adrs[i].partner, linetmp);
      strcpy(adrs[i].shortaddr, reverse_addr(xx));
      i++;
    }
    s_fclose(f);
  }
  else
  {
    trace(serious, name, ms(m_filenotopen), AFWDTMP);
    putf(ms(m_filenotopen), AFWDTMP);
    t_free(adrs);
    return;
  }
#ifdef DEBUG_AFWD
  trace(report, name, "read %u", i);
  trace(report, name, "sorting ..");
#endif
  qsort(adrs, i, sizeof(afwd_data), data_cmp);
  found = process_afwd_data(adrs, found);
#ifdef DEBUG_AFWD
  trace(report, name, "writing "AFWDLISTNAME);
#endif
  f = s_fopen(AFWDLISTNAME, "lwt");
  if (! f)
  {
    trace(serious, name, ms(m_filenotopen), AFWDLISTNAME);
    putf(ms(m_filenotopen), AFWDLISTNAME);
    t_free(adrs);
    return;
  }
  fprintf(f, "; OpenBCM AutoFwd File !!! DO NOT EDIT !!!\n");
  fprintf(f, "; This file is generated automatically. (use %s for manual editing!)\n", FWDLISTNAME);
  write_afwd(f, adrs, found);
  s_fclose(f);
  t_free(adrs);
  initafwdlist();
}

/*---------------------------------------------------------------------------*/

void initafwdlist ()
//*************************************************************************
//
// read afwd.bcm into memory
//
//*************************************************************************
{
  FILE *f;
  char s[LINELEN+1];
  char word[MAXPARSE];
  char dstb[HADRESSLEN+1];
  int boxindex = 0;
  int line = 0;
  int i;
  int destlen = 0;
  afwdpara_t *ff;
  char name[20];

  strcpy(name, "initafwdlist");
  if (! m.autofwdtime) return;
  if (afp) t_free(afp);
  afp = (afwdpara_t *) t_malloc(MAXFWD * sizeof(afwdpara_t), "*afp");
  if (aboxdest) t_free(aboxdest);
  aboxdest = (char *) t_malloc(MAXDESTLEN * sizeof(char), "*ads");
  if (! afp || ! aboxdest)
  {
    trace(fatal, name, "out of memory");
    return;
  }
  memset(afp, 0, MAXFWD * sizeof(afwdpara_t));
  ff = afp;
  if ((f = s_fopen(AFWDLISTNAME, "srt")) != NULL)
  {
    while (fgets(s, LINELEN, f))
    {
      line++;
      subst1(s, ';', 0);
      if (strlen(s) > 3)
      {
        i = (*s == ' ');
        strupr(s);
        parseline(s, word);
        if (i)
        {
          if (! boxindex)
            goto error; // erster Eintrag: Boxcall fehlt
          else
          {
            char *dest = afp[boxindex - 1].dest;
            i = 0;
            while (word[i])
            {
              strncpy(dstb, s + word[i++], HADRESSLEN);
              dstb[HADRESSLEN] = 0;
              //strip_hadr(dstb);
              if (*dstb && (destlen + strlen(dstb) + 1) < MAXDESTLEN)
              {
                strcat(dest, dstb);
                strcat(dest, " ");
                destlen += (strlen(dstb) + 1);
              }
              else
              {
                trace(serious, name, ms(m_fwd_toomanydest));
                if (*b->logincall) putf(ms(m_fwd_toomanydest));
                break;
              }
            }
          }
        }
        else
        {
          if (! boxindex)
            ff->dest = aboxdest;
          else
          {
            ff->dest = afp[boxindex - 1].dest
                       + strlen(afp[boxindex - 1].dest) + 1;
            destlen += 1;
          }
          *ff->dest = 0;
          *ff->call = 0;
          if (strlen(s + *word) <= 6) strcpy(ff->call, s + *word);
          else goto error;                      // Boxcall zu lang
          if (++boxindex == MAXFWD)
          {
            trace(serious, name, ms(m_fwd_toomanycalls));
            if (*b->logincall) putf(ms(m_fwd_toomanycalls));
            break;
          }
          ff = &afp[boxindex];
        }
      }
    }
    *ff->call = 0;
    s_fclose(f);
#ifdef DEBUG_AFWD
    trace(report, name, "%d partners", boxindex);
#endif
    if (*b->logincall) putf("%s: %d partners.\n", name, boxindex);
    afwdpartners = boxindex;
    if (! boxindex)
    {
      t_free(afp);
      t_free(aboxdest);
      afp = NULL;
      aboxdest = NULL;
    }
    return;
  }
  else
  {
    trace(serious, name, AFWDLISTNAME" not found");
    if (*b->logincall) putf("%s: "AFWDLISTNAME" not found.\n", name);
    return;
  }
error: //that should never happen
#ifdef __DOS16__
    sound(1000);
    wdelay(106);
    nosound();
#endif
    trace(serious, name, "line %d error", line);
    if (*b->logincall) putf("%s: line %d error.\n", name, line);
    s_fclose(f);
}
#endif
