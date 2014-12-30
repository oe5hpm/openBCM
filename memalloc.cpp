/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------
  Memory-Ueberwachung
  -------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/
// 20000609 DK2UI  added Jans numsort for mem

#include "baycom.h"

/*---------------------------------------------------------------------------*/

static void *alloctab[MAXBLKS];
static char allocname[MAXBLKS][5];
static char alloctask[MAXBLKS];
static size_t allocsize[MAXBLKS];
void *nextfree = NULL;

struct t_liste_s
{
  size_t size;
  int reihe;
};

/*---------------------------------------------------------------------------*/

void *t_malloc (size_t size, char *name)
{
  unsigned int i;
  for (i = 0; i < MAXBLKS; i++)
  {
    if (! alloctab[i])
    {
      alloctab[i] = malloc(size);
      if (! alloctab[i])
        trace(fatal, "t_malloc", "%s fail %ld Bytes", name, size);
      strncpy(allocname[i], name, sizeof(allocname[0]));
      allocname[i][sizeof(allocname[0]-1)] = 0;
      if (*name == '*')
        alloctask[i] = (char) 255;
      else
         alloctask[i] = gettaskid();
      allocsize[i] = size;
      return alloctab[i];
    }
  }
  trace(fatal, "t_malloc", "%s blk full", name);
  return NULL;
}

/*---------------------------------------------------------------------------*/

int t_settaskid (void *blk, int tid)
{
  unsigned int i;
  if (blk)
  {
    for (i = 0; i < MAXBLKS; i++)
    {
      if (alloctab[i] == blk)
      {
        alloctask[i] = tid;
        return 1;
      }
    }
    trace(serious, "t_settaskid", "%lp no alloc", blk);
  }
  else
    trace(serious, "t_settaskid", "blk NULL pointer");
  return 0;
}

/*---------------------------------------------------------------------------*/

void t_free (void *blk)
{
  unsigned int i;
  if (blk)
  {
    for (i = 0; i < MAXBLKS; i++)
    {
      if (alloctab[i] == blk)
      {
        free(blk);
        alloctab[i] = NULL;
        alloctask[i] = (char) 255;
        return;
      }
    }
    trace(serious, "t_free", "%lp no alloc", blk);
  }
  else
    trace(serious, "t_free", "blk NULL pointer");
}

/*---------------------------------------------------------------------------*/

void t_checkfree (void)
{
  if (nextfree)
  {
    free(nextfree);
    nextfree = 0;
  }
}

/*---------------------------------------------------------------------------*/

void t_allfree (int task)
{
  unsigned int i;

  for (i = 0; i < MAXBLKS; i++)
  {
    if (alloctask[i] == task)
    {
      if (alloctab[i] != t)
      // eigenen TCB noch nicht loeschen
      {
        if (alloctab[i])
          free(alloctab[i]);
        else
          trace(serious, "t_allfree", "NULL %d %s \"%s\"",
                         task, gettaskname(task), allocname[i]);
      }
      else
      {
        if (nextfree == NULL)
          nextfree = alloctab[i];
        else
          trace(serious, "t_allfree", "nextfree busy");
      }
      alloctab[i] = NULL;
      alloctask[i] = (char) 255;
    }
  }
}

/*---------------------------------------------------------------------------*/

void *t_realloc (void *blk, size_t newsize)
{
  unsigned int i;
  for (i = 0; i < MAXBLKS; i++)
  {
    if (alloctab[i] == blk)
    {
      alloctab[i] = realloc(blk, newsize);
      allocsize[i] = newsize;
      if (! alloctab[i])
      {
        trace(fatal, "realloc", "%s fail %u Bytes", allocname[i], newsize);
        return NULL;
      }
      return (alloctab[i]);
    }
  }
  trace(fatal, "realloc", "%lp no alloc", blk);
  return NULL;
}

/*---------------------------------------------------------------------------*/

int sizecmp (t_liste_s *l1, t_liste_s *l2)
{
  size_t num1 = l1->size;
  size_t num2 = l2->size;

  if (num1 > num2) return (1);
  if (num1 < num2) return (-1);
  return (0);
}

/*---------------------------------------------------------------------------*/

void t_alloclist (char *opts)  //df3vi: sortier-optionen eingebaut
{
  unsigned int i, j, k;

  scanoptions(opts);
  putf("Blk  Size Address  Task Name\n");
  if (b->optplus & o_s)
  { //sort by size
    t_liste_s *liste;
    liste = (t_liste_s *) t_malloc(sizeof(t_liste_s) * MAXBLKS, "mems");
    for (j = 0; j < MAXBLKS; j++)
    {
      liste[j].size = allocsize[j];
      liste[j].reihe = j;
    }
    qsort((void *) liste, MAXBLKS, sizeof(t_liste_s),
          (int(*) (const void *, const void *)) sizecmp);
    for (j = 0; j < MAXBLKS; j++)
    {
      i = liste[j].reihe;
      if (alloctab[i])
        putf("%3d %5u %08lX%5d %s\n",
             i, allocsize[i], (long) alloctab[i], alloctask[i], allocname[i]);
    }
    t_free(liste);
  }
  else if (b->optplus & o_a)
  { //sort by address
    struct t_liste_a
    {
      char address[9];
      int reihe;
    } *liste;
    liste = (t_liste_a *) t_malloc(sizeof(t_liste_a) * MAXBLKS, "mema");
    for (j = 0; j < MAXBLKS; j++)
    {
      sprintf(liste[j].address, "%08lX", (long) alloctab[j]);
      liste[j].reihe = j;
    }
    qsort((void *) liste, MAXBLKS, sizeof(liste[0]),
          (int(*) (const void *, const void *)) stricmp);
    for (j = 0; j < MAXBLKS; j++)
    {
      i = liste[j].reihe;
      if (alloctab[i])
        putf("%3d %5u %08lX%5d %s\n",
             i, allocsize[i], (long) alloctab[i], alloctask[i], allocname[i]);
    }
    t_free (liste);
  }
  else if (b->optplus & o_t)
  { // sort by task-nr
    for (k = 0; k <= 255; k++)
      for (i = 0; i < MAXBLKS; i++)
      {
        if (alloctab[i] && alloctask[i] == (char) k)
          putf("%3d %5u %08lX%5d %s\n",
               i, allocsize[i], (long) alloctab[i], alloctask[i], allocname[i]);
      }
  }
  else if (b->optplus & o_n)
  { //sort by name
    struct t_liste_n
    {
      char name[6];
      int reihe;
    } *liste;
    liste = (t_liste_n *) t_malloc(sizeof(t_liste_n) * MAXBLKS, "memn");
    for (j = 0; j < MAXBLKS; j++)
    {
      strncpy(liste[j].name, allocname[j], 5);
      liste[j].name[5] = 0;
      liste[j].reihe = j;
    }
    qsort((void *) liste, MAXBLKS, sizeof(liste[0]),
          (int(*) (const void *, const void *)) stricmp);
    for (j = 0; j < MAXBLKS; j++)
    {
      i = liste[j].reihe;
      if (alloctab[i])
        putf("%3d %5u %08lX%5d %s\n",
             i, allocsize[i], (long) alloctab[i], alloctask[i], allocname[i]);
    }
    t_free(liste);
  }
  else
  { //normal output (by block-nr)
    for (i = 0; i < MAXBLKS; i++)
    {
      if (alloctab[i])
        putf("%3d %5u %08lX%5d %s\n",
             i, allocsize[i], (long) alloctab[i], alloctask[i], allocname[i]);
    }
  }
}

/*---------------------------------------------------------------------------*/

void t_allocinit (void)
{
  unsigned int i;
  for (i = 0; i < MAXBLKS; i++)
  {
    alloctab[i] = NULL;
    allocname[i][0] = 0;
    alloctask[i] = (char) 255;
  }
}
