/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------
  Forward-Ein- und Ausgabe fuer File-Forwarding
  ----------------------------------------------


  Copyright (C)       Markus Baumann
                      Am Altenfelder Kreuz 43
                      59302 Oelde

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/


#include "baycom.h"

#ifdef _FILEFWD
extern int fwdcmd();
extern int scan_fwdfile(char type);
extern int scan_emfile(char *path);


/*---------------------------------------------------------------------------*/

int fwd_import (char *befbuf)
//*************************************************************************
//
//  Importiert Mails aus einer Datei im FBB-Forward-Format
//
//*************************************************************************
{
  lastfunc("fwd_import");
  char partner[CALLEN+1];
  char file[51];
  char orglogincall[CALLEN+1];

  mkdir(FWDIMPATH);
  if (*befbuf)
  {
    trace(report, "fwd_import", "%s", befbuf);
    // For RX use SID with BCM extensions - import almost anything
    b->opt = o_a | o_d | o_f | o_h | o_m | o_r | o_w; //SID ADFHMRW($)
    if (*befbuf == '-') //handle options (skip them)
    {
      char opt = toupper(befbuf[1]);
      befbuf = nexttoken(befbuf, file, 50); //skip to next token
      if (opt == 'B' || opt == 'D' || opt == 'F') {}
      else
        if (opt == 'S')
          befbuf = nexttoken(befbuf, file, 20);
        else
        {
          putf(ms(m_unknownopt), opt);
          return NO;
        }
    }
    befbuf = nexttoken(befbuf, partner, CALLEN);
    nexttoken(befbuf, file, 50);
    strupr(partner);
    if (isforwardpartner(partner) == NOTFOUND)
    {
      putf(ms(m_fwd_callnotfound), partner);
      return NO;
    }
    if (! *file)
    {
      putf("Syntax: FWDIMPORT <partner_call> <file>\n");
      return NO;
    }
    putf(ms(m_fwd_import), file, partner);
    FILE* oldinpfile = b->inputfile;
    b->inputfile = s_fopen(file, "lrb");
    if (! b->inputfile)
    {
      b->inputfile = oldinpfile;
      putf(ms(m_filenotopen), file);
      return NO;
    }
    char oldinp = b->oldinput; //we have to save those extra
    char oldinp2 = t->input;   //for fwdimport in .imp files to work
    char oldout = b->oldoutput;
    b->forwarding = fwd_file;
//  strcpy(b->frombox, partner);
    strcpy(orglogincall, b->logincall);
    strcpy(b->logincall, partner);
    b->oldinput = io_dummy;
    t->input = io_file;
    b->oldoutput = t->output;
    t->output = io_dummy;
    fwdlog("---- ", "fwdimport - start", '-');
    while (! fwdcmd());
      if (t->input == io_file)
      {
        fwdlog("---- ", "fwdimport - terminated - bad import file", '-');
        putf(ms(m_fwd_badimport));
        s_fclose(b->inputfile);
        b->inputfile = NULL;
        t->input = (io_t) b->oldinput;
      }
      else
        fwdlog("---- ", "fwdimport - end", '-');
    b->inputfile = oldinpfile;
    b->oldinput = oldinp;
    t->input = (io_t) oldinp2;
    t->output = (io_t) b->oldoutput;
    b->oldoutput = oldout;
    b->forwarding = fwd_none;
    fwdpara_t *ff = fpara();
    ff->lastlogin = ad_time();
    ff->n_login++;
    strcpy(b->logincall, orglogincall);
    return OK;
  }
  else
  {
    putf("Syntax: FWDIMPORT <partner_call> <file>\n");
    return NO;
  }
}

/*---------------------------------------------------------------------------*/

int fwd_export (char *befbuf)
//*************************************************************************
//
//  Exportiert Mails in eine Datei im FBB-Forward-Format
//
//*************************************************************************
{
  lastfunc("fwd_export");
  char partner[CALLEN+1];
  char orglogincall[CALLEN+1];
  char file[51];
  char sid[21];

  mkdir(FWDEXPATH);
  if (*befbuf)
  {
    trace(report, "fwd_export", "%s", befbuf);
    b->opt = o_a | o_f | o_h | o_m | o_r; //default SID AFHMR($) (FBB)
    if (*befbuf == '-') //handle options
    {
      char opt = toupper(befbuf[1]);
      befbuf = nexttoken(befbuf, file, 50); //skip to next token
      if (opt == 'B')       //BCM extensions (ABin, W-format)
        b->opt = o_a | o_f | o_d | o_h | o_m | o_r | o_w;
      else
        if (opt == 'F') {}  //FBB format - default (nobin, WP)
        else
          if (opt == 'D')   //DieBox format (ABin, E/M msgs)
            b->opt = o_a | o_d | o_h | o_m | o_r;
          else
            if (opt == 'S') //use user defined SID
            {
              befbuf = nexttoken(befbuf, sid, 20);
              unsigned int i = 0;
              b->opt = 0;
              while (sid[i])
              {
                if (isalpha(sid[i]))
                  b->opt |= (1L << (toupper(sid[i]) - 'A'));
                i++;
              }
            }
            else
            {
              putf(ms(m_unknownopt), opt);
              return NO;
            }
    }
    befbuf = nexttoken(befbuf, partner, CALLEN);
    nexttoken(befbuf, file, 50);
    strupr(partner);
    if (isforwardpartner(partner) == NOTFOUND)
    {
      putf(ms(m_fwd_callnotfound), partner);
      return NO;
    }
    if (! *file)
    {
      putf("Syntax: FWDEXPORT [-<option>] <partner_call> <file>\n");
      return NO;
    }
    putf(ms(m_fwd_export), file, partner);
    b->forwarding = fwd_file;
    strcpy(orglogincall, b->logincall);
    strcpy(b->logincall, partner);
    if ((t->input != io_file || t->output == io_dummy) && t->output != io_file)
    {
      b->outputfile = s_fopen(file, "sab");
      if (b->outputfile)
      {
        b->oldinput = t->input;
        b->oldoutput = t->output;
        t->input = io_dummy;
        t->output = io_file;
        b->continous = 1;
        fwdlog("---- ", "fwdexport - start", '-');
        if ((b->opt & o_w) || (b->opt & o_f))
        {
          gen_wpmail(b->logincall);
          wdelay(349);
        }
        else
          if (b->opt & o_d) scan_emfile(FWDPATH "/m_");
        scan_fwdfile('u');
        scan_fwdfile('i');
        fwdlog("---- ", "fwdexport - end", '-');
        s_fclose(b->outputfile);
        b->outputfile = NULL;
        t->input = (io_t) b->oldinput;
        t->output = (io_t) b->oldoutput;
        b->oldinput = io_dummy;
        b->oldoutput = io_dummy;
      }
      else
        putf(ms(m_filenotopen), file);
    }
    fwdpara_t *ff = fpara();
    ff->lastconok = ad_time();
    ff->n_conok++;
    ff->nolink = 0;
//---
    if ((ad_time() - ff->lastwpr) > 5*HOUR) // after 5h
    {
      wpdata_t *wp = (wpdata_t*) t_malloc(sizeof(wpdata_t), "wps");
      strcpy(wp->bbs, ff->call);
      wp->bversion = BVERSION;
      ff->lastwpr = wp->mybbstime = ad_time();
      wp->hops = 1; // wird in addwp_r +1 erhoeht
      wp->qual = WPROT_MIN_ROUTING / 2;
      wprotlog("F ", wp->bbs);
      addwp_r(wp);
      t_free(wp);
    }
//---
    strcpy(b->logincall, orglogincall);
    b->forwarding = fwd_none;
    return OK;
  }
  else
  {
    putf("Syntax: FWDEXPORT [-<option>] <partner_call> <file>\n");
    return NO;
  }
}

/*---------------------------------------------------------------------------*/


#endif
