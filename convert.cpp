/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------------
  Convert (automatische Rubrikkonvertierung)
  ------------------------------------------

  GNU GPL
  Copyright (C)          Wolfgang, DK2UI

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//98-10-30 DK2UI  automatic bulletin transfer on title scan
//20000116 OE3DZW added ui-options
//00-05-01 DK2UI  corrections for LT
//00-06-08 DK2UI  works now also with main boards
//20000624 DK2UI  added regular expression match
//20030323 DH8YMB convedit hinzu

#include "baycom.h"

typedef struct convert_
{ char      oldname[DIRLEN+1];
  char      newname[DIRLEN+1];
  short int life;
  char      string[32];
} convert_;

unsigned cvtlen = 0;
static convert_ *cvt = NULL;

/*---------------------------------------------------------------------------*/

void mbcvtload (void)
//*************************************************************************
//
// DK2UI: laedt die convert.bcm
//
//*************************************************************************
{
  FILE *f;
  f = s_fopen(CONVFILENAME, "srt");
  char s[LINELEN+1], *cp;
  cvtlen = 0;
  if (cvt)
  {
    t_free(cvt);
    cvt = NULL;
  }
  if (f)
  {
    cvt = (convert_*) t_malloc(sizeof(convert_) * MAXCONVNAME, "*cvt");
    memset(cvt, 0, sizeof(convert_) * MAXCONVNAME);
    while (fgets(s, LINELEN, f))
    {
      if (*s == ';') continue;
      subst(s, ';', 0); // skip comments
      cut_blank(s);
      if (*s)
      {
        strupr(s);
        cp = s;
        cp = nexttoken(cp, cvt[cvtlen].oldname, DIRLEN);
        cp = nexttoken(cp, cvt[cvtlen].newname, DIRLEN);
        if (*cp)
        {
          cp = nexttoken(cp, cvt[cvtlen].string, 3);
          cvt[cvtlen].life = atoi(cvt[cvtlen].string);
          if (*cp)
          {
            cp[31] = 0;
            strcpy(cvt[cvtlen].string, cp);
          }
          else
            *cvt[cvtlen].string = 0;
        }
        else
          cvt[cvtlen].life = (-1);
        if (*cvt[cvtlen].oldname && *cvt[cvtlen].newname)
          cvtlen++;
        if (cvtlen > MAXCONVNAME)
          break;
      }
    }
    s_fclose(f);
    if (cvtlen)
      cvt = (convert_*) t_realloc(cvt, sizeof(convert_) * cvtlen);
    else
    {
      t_free(cvt);
      cvt = NULL;
    }
    trace(report, "cvtload", "%d convert", cvtlen);
    if (*b->logincall)
      putf("%d convert.\n", cvtlen);
  }
}

/*---------------------------------------------------------------------------*/

int convert (char *Name, int *convLT)
//*************************************************************************
//
// DK2UI: konvertiert ein Board
//
//*************************************************************************
{
 unsigned int i, found = FALSE;
 char Hcall[7], s[81], string[32], *cp;
 lastfunc("convert");

 memcpy(Hcall, b->at, 6);
 Hcall[6] = 0;
 subst1(Hcall, '.', 0);
// dh8ymb
 if (strlen(Name) == 6 && ! finddir(Name, 0))
 {
   for (i = 0; i < treelen; i++)
   {
     if ((cp = strchr(tree[i].name, '/')) != NULL) cp++;
     else cp = tree[i].name;
     if (strlen(cp) > 6 && ! strncmp(cp, Name, 6))
     {
       strcpy(Name, cp);
       break;
     }
   }
 }
 strcpy(s, b->betreff);
 strupr(s);
 if (cvt)
 {
  CRegEx reg;
  for (i = 0; i < cvtlen; i++)
  {
   if (! strcmp(cvt[i].oldname, Name) || *cvt[i].oldname == '*'
       || (   (cp = strchr(cvt[i].oldname + 2, '*')) != NULL      // min. 2 char
           && ! strncmp(cvt[i].oldname, Name, (int) (cp - cvt[i].oldname)) ) )
   {
    found = TRUE;
    if (cvt[i].life == (-1))
    { // new dirname, no title scan
     strcpy(Name, cvt[i].newname);
   //  putf("Converting: %s -> %s\n",cvt[i].oldname,cvt[i].newname);
     break;
    }
    else
     if (b->mailtype == 'B')
     { // title depending conversion
     strcpy(string, cvt[i].string);
     cp = *string ? strtok(string, " ,") : 0;
     while (cp)
     {
      found = FALSE;
      switch (*cp)
      {
       case '<': // sender
//      found = (! strcmp(++cp, b->herkunft)); break;
        found = reg.regex_match(++cp, b->herkunft); break;
       case '@': // region
//      found = (!strcmp(++cp, Hcall)); break;
        found = reg.regex_match(++cp, Hcall); break;
       case '$': // from box
        found = (b->forwarding != fwd_none); break;
       case '%': // from user
        found = (b->forwarding == fwd_none || b->forwarding == fwd_user); break;
       case '&': // from user without password
        found = ((b->forwarding == fwd_none || b->forwarding == fwd_user)
                 && !*u->password && u->sfpwtype != 100); break;
       case '~': // word check
        if (strlen(cp + 1) == strlen(s))
        { // title is one word
         found = ! strcmp(cp + 1, s); break;
        }
        *cp = ' '; strcat(cp, " ");
        found = (   strstr(s, cp) != NULL
                 || !strncmp(cp + 1, s, strlen(cp + 1))
                 || !strncmp(cp, s + strlen(s) - strlen(cp + 1),
                             strlen(cp + 1)));
        break;
       default : // item in titel ?
//      found = (strstr(s, cp) != NULL); }
        found = reg.regex_match(cp, s);
       }
      cp = found ? strtok(NULL, " ,") : 0;
      }
     if (found)
     {
      if (*cvt[i].newname != '*')
      {
        strcpy(Name, cvt[i].newname);
      //  putf("Converting: %s -> %s\n",cvt[i].oldname,cvt[i].newname);
        *convLT = cvt[i].life;
      }
      else
        if (*cvt[i].oldname != '*') *convLT = cvt[i].life;
      break;
     }
    }
   }
  }
 }
 return found;
}

/*---------------------------------------------------------------------------*/

#ifdef DF3VI_CONV_EDIT

void convedit(char *befbuf)
//*************************************************************************
//
// DF3VI: Editor zu convert.bcm
//
//*************************************************************************
{
  lastfunc("convedit");
  FILE *f;
  char cmd[5];
  char board1[9];
  char board2[9];
  char buf[50];
  short int lifetime;
  char subject[32];

  befbuf=nextword(befbuf, cmd, 4);
  befbuf=nextword(befbuf, board1, 8);
  befbuf=nextword(befbuf, board2, 8);
  if (*befbuf)
  {
     befbuf = nexttoken(befbuf, subject, 3);
     lifetime = atoi(subject);
     if (*befbuf)
     {
       befbuf[31] = 0;
       strcpy(subject, befbuf);
     }
     else *subject = 0;
  }
  else
  {
    *subject = 0;
    lifetime = (-1);
  }
  strupr(board1);
  strupr(board2);
  strupr(subject);
  strupr(cmd);
  switch (cmd[0])
  {
    case 'A': // add
      {
        short found = 0;
        if (cvtlen == MAXCONVNAME)
        {
          putf(ms(m_cvt_toomany));
          putf("\n");
        }
        else
        {
          if (board2[0] == 0 || finddir(board2, 1) <= 0)
          {
            putf(ms(m_cvt_notexist));
            putf("\n");
          }
          if (finddir(board1, 1))
          {
            putf(ms(m_cvt_oldexist));
            putf("\n");
          }
          if (cvtlen == 0) //bei 1. Eintrag ist cvt nicht im Speicher!
          {
            putf(ms(m_cvt_created));
            putf("\n");
            putf("\n");
            putf(ms(m_cvt_added), board1, board2);
            if (lifetime > 0) putf(" %d", lifetime);
            if (subject[0]) putf(" %s", subject);
            putf("\n");
            f = s_fopen(CONVTMPNAME, "swt");
            if (f)
            {
              fprintf(f, "; OpenBCM Mailbox convert.bcm\n;\n;\n");
              if (board1[0])
              {
                if (lifetime > 0)
                {
                 if (subject[0])
                   sprintf(buf, "%s %s %d %s\n", board1, board2,
                                                 lifetime, subject);
                 else
                   sprintf(buf, "%s %s %d\n", board1, board2, lifetime);
                }
                else
                  sprintf(buf, "%s %s\n", board1, board2);
                strlwr(buf);
                fputs(buf, f);
              }
            s_fclose(f);
            xunlink(CONVFILENAME);
            xrename(CONVTMPNAME, CONVFILENAME);
            }
            else
              trace(serious, "saveconvnames", "fopen err");
            mbcvtload();
          }
          else
          {
            for (unsigned int i=0; i<cvtlen; i++)
            {
              if (strcmp(cvt[i].oldname, board1)==0)
              {
                strcpy(cvt[i].newname, board2);
                found++;
              }
            }
            if (! found)
            {
              strcpy(cvt[cvtlen].oldname, board1);
              strcpy(cvt[cvtlen].newname, board2);
              cvt[cvtlen].life = lifetime;
              strcpy(cvt[cvtlen].string, subject);
              trace(report, "", "%s %s", board1, cvt[cvtlen].oldname);
              cvtlen++;
              found++;
            }
          }
        }
        if (found)
        {
          putf(ms(m_cvt_added), board1, board2);
          if (lifetime > 0) putf(" %d", lifetime);
          if (subject[0]) putf(" %s", subject);
          putf("\n");
          saveconvnames();
        }
      }
      break;
    case 'D': // delete
      {
        for (unsigned int i=0; i<cvtlen; i++)
        {
          if (strcmp(cvt[i].oldname, board1) == 0 ||
              strcmp(cvt[i].newname, board1) == 0 )
          {
            putf(ms(m_cvt_deleted), cvt[i].oldname, cvt[i].newname);
            if (cvt[i].life > 0) putf(" %d", cvt[i].life);
            if (cvt[i].string[0]) putf(" %s", cvt[i].string);
            putf("\n");
            cvt[i].oldname[0]=0;
          }
        }
        saveconvnames();
        break;
      }
    case 'S': // sort
      {
        if (cvtlen)
        {
          qsort((void *) cvt, cvtlen, sizeof(convert_),
                (int(*)(const void *, const void *)) strcmp);
        }
      putf(ms(m_cvt_sorted));
      putf("\n");
      saveconvnames();
      break;
      }
    case 'L': // list
      {
        for (unsigned int i=0; i<cvtlen; i++)
        {
          if (board1[0] == 0 || strcmp(cvt[i].oldname, board1) == 0
                             || strcmp(cvt[i].newname, board1) == 0)
           {
            putf("%-8s -> %-8s", cvt[i].oldname, cvt[i].newname);
            if (cvt[i].life > 0) putf(" %d", cvt[i].life);
            if (cvt[i].string[0]) putf(" %s", cvt[i].string);
            putf("\n");
           }
        }
        break;
      }
    default:
      putf("Syntax: convedit add <sourceboard> <targetboard> [[LT] [string]]\n"
           "        convedit del <board>\n"
           "        convedit sort\n"
           "        convedit list [<board>]\n");
  }
}
/*---------------------------------------------------------------------------*/

void saveconvnames (void)
//*************************************************************************
//
// DF3VI: Speichert die convert.bcm
//
//*************************************************************************
{
  char buf[50];
  FILE *f;

  f = s_fopen(CONVTMPNAME, "swt");
  if (f)
  {
    fprintf(f, "; OpenBCM Mailbox convert.bcm\n;\n;\n");
    for (unsigned int i = 0; i < cvtlen; i++)
    {
      if (cvt[i].oldname[0])
      {
        if (cvt[i].life >= 0)
        {
          if (cvt[i].string[0])
            sprintf(buf, "%s %s %d %s\n", cvt[i].oldname, cvt[i].newname,
                                          cvt[i].life, cvt[i].string);
          else
            sprintf(buf, "%s %s %d\n", cvt[i].oldname, cvt[i].newname,
                                       cvt[i].life);
        }
        else
          sprintf(buf, "%s %s\n", cvt[i].oldname, cvt[i].newname);
        strlwr(buf);
        fputs(buf, f);
      }
    }
    s_fclose(f);
    xunlink(CONVFILENAME);
    xrename(CONVTMPNAME, CONVFILENAME);
  }
  else
    trace(serious, "saveconvnames", "fopen err");
  mbcvtload();

}
/*---------------------------------------------------------------------------*/

#endif // ifdef DF3VI_CONV_EDIT
