/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------
  Zeichen Ein- Ausgabe fuer TNC und Fenster
  -----------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980307 OE3DZW bugfix: inputline_file stopped on 0x00 0x00
//19980404 hrx    tty: putv(), case io_tty: extension for fwding via tty
//19980614 hrx    added paclen extensions to void putv(int c), and void
//                bputv(int c) Binary (transparent) putv (OE3DZW)
//19990817 OE3DZW added JJ code
//19991028 Jan    fixes to paclen, added init_paclen,feature_serial
//19991122 Jan    added #ifdef MACRO to io_memory since only macros use it
//19991213 Jan    preliminary charsets support, new bgetv()

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int janein (char *prompt)
//*************************************************************************
//
//*************************************************************************
{
  char s[5];
  char *yes;

  putf(ms(m_yesnoquestion), prompt);
  getline(s, 4, 1);
  putv(LF);
  strupr(s);
  if ((yes = strchr(ms(m_yesnoquestion), '(')) != NULL && *yes) yes++;
  else yes = NULL;
  if (*s == 'Y' || *s == 'J' || *s == 'O' || (yes != NULL && *s == *yes))
    return JA;
  if (*s == 'N') return NEIN;
  else return JEIN;
}

/*---------------------------------------------------------------------------*/

void putf (char *format, ...)
//*************************************************************************
//
//*************************************************************************
{
  va_list argpoint;
  char cbuf[260];
  char *s = cbuf;

  va_start(argpoint, format);
  vsprintf(cbuf, format, argpoint);
  va_end(argpoint);
#ifdef __FLAT__
  if (! stricmp(b->uplink, "NNTP")
      && ((m.tcpiptrace == 2) || (m.tcpiptrace == 8)) )
    nntplog("TX", s);
  if (! stricmp(b->uplink, "POP3")
      && ((m.tcpiptrace == 3) || (m.tcpiptrace == 8)) )
    pop3log("TX", s);
  if (! stricmp(b->uplink, "SMTP")
      && ((m.tcpiptrace == 4) || (m.tcpiptrace == 8)) )
    smtplog("TX", s);
  if (! stricmp(b->uplink, "FTP")
      && ((m.tcpiptrace == 5) || (m.tcpiptrace == 8)) )
    ftplog("TX", s);
#endif
  if (strlen(s) > 257)
  {
    s[LINELEN] = 0;
    trace(tr_abbruch, "putf", "long %s", s);
  }
#ifdef _USERCOMP
  if (u->comp == 1)
  {
//#include "usercomp.h"

    char output[260];
    char output2[260];
    int i, il = 0;
//    il = comp_sp_stat_huff(s, strlen(s), output);
    il = comp_top_stat_huff(true, s, output, strlen(s));
    //printf("\nOut:\n%s\n\n",output);
    for (i = 1; i < il ; i++)
      bputv(output[i]);
 printf("%d\nOutput:\n%s\n\n",il,output);
    ahuf_top_init();
    il = decomp_top_stat_huff(output2, output, il);
 printf("%d\nOutput2:\n%s\n\n",il,output2);

  }
  else
#endif
    while (*s) putv(*(s++));
}

/*---------------------------------------------------------------------------*/

int testabbruch (void)
//*************************************************************************
//
//*************************************************************************
{
  waitfor(e_ticsfull);
  if (b)
  {
    if (getvalid() == CR && ! b->abbruch) b->abbruch++;
    if (b->abbruch == 1)
    {
      putf(ms(m_aborted));
      b->abbruch++;
    }
    return b->abbruch;
  }
  else return 0;
}

/*---------------------------------------------------------------------------*/

#ifdef _TELEPHONE
void
#else
static void near
#endif
                 mon (int c, bitfeld opt)
//*************************************************************************
//
//*************************************************************************
{
  monbuf_t *mm = t->monitor;
  if (b)
  {
    switch (b->job)
    {
      case j_fwdrecvf:       //fall through
      case j_fbb_recv_mail:  //fall through
      case j_send:   if (mm->opt & o_s) return; break;
      case j_fwdsendf:       //fall through
      case j_fbb_send_mail:  //fall through
      case j_read:   if (mm->opt & o_r) return; break;
      case j_search: if (mm->opt & o_d) return; break;
    }
  }
  if (! (mm->opt & opt))
  {
    int congestion_count = 0;
    while (t->monitor && ((mm->in + 1) % MONBUFLEN) == mm->out)
    {
      if ((congestion_count++) > 20)
      {
        mm->congested = 1;
        break;
      }
      wdelay(282);
    }
    if (! t->monitor) return;
    mm->buf[mm->in++] = c;
    if (mm->in >= MONBUFLEN) mm->in = 0;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef __FLAT__ //for now ..
static int charset_tx (int c)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 0;
/*  if(charsets[b->charset-1]->chars == NULL)
          load_charset(b->charset-1);*/
  while (charsets[b->charset - 1]->chars[i].a)
  {
    if (charsets[b->charset - 1]->chars[i].a == c)
      return charsets[b->charset - 1]->chars[i].b;
    i++;
  }
  return c;
}

/*---------------------------------------------------------------------------*/

static int charset_rx (int c)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i = 0;
//  if(charsets[b->charset-1]->chars == NULL)
//          load_charset(b->charset-1);
  while (charsets[b->charset - 1]->chars[i].b)
  {
    if (charsets[b->charset - 1]->chars[i].b == c)
      return charsets[b->charset-1]->chars[i].a;
    i++;
  }
  return c;
}
#endif

/*---------------------------------------------------------------------------*/

void putv (int c)
//*************************************************************************
//
//*************************************************************************
{
  if (c == CR) return;
  if (c == LF)
  {
    if (b && u)
    {
      if (u->zeilen && ! b->continous && (++b->linecount) >= u->zeilen)
      {
        char line[4];
        putf("+?>");
        getline(line, 3, 1);
        b->linecount = 0;
        switch (toupper(*line))
        {
        case 'C': b->continous = 1; break;
        case 'Q':
        case 'B': b->abbruch++; b->quit = 1;
        case ' ': // DH3MB
        case 0: break;
        default: b->abbruch++; break;
        }
      }
      else if ((getvalid() == CR) && (! b->abbruch)) b->abbruch++;
    }
  }
  if (b && b->http == 2)
  {
    switch (c)
    { // Conversion IBM -> HTML
      case '>':  putf("&gt;"); return;
      case '<':  putf("&lt;"); return;
/* 05.04.2004 dh8ymb: is this really needed any more with newer browsers?
                      THIS makes problems with charset in japan and china!
      case 0x84: putf("&auml;"); return;
      case 0x94: putf("&ouml;"); return;
      case 0x81: putf("&uuml;"); return;
      case 0x8e: putf("&Auml;"); return;
      case 0x99: putf("&Ouml;"); return;
      case 0x9a: putf("&Uuml;"); return;
      case 0xe1: putf("&szlig;"); return;
*/
    }
  }
#ifdef __FLAT__
//  if (b && b->charset) c = charset_tx(c);
#endif
  if (c == LF)
  {
    if (t->output == io_socket || t->output == io_tty) bputv(CR);
    else if (t->output == io_tnc) c = CR;
  }
  bputv(c);
}

/*---------------------------------------------------------------------------*/

void bputv (int c) // Binary (transparent) putv (OE3DZW)
//*************************************************************************
//
//*************************************************************************
{
 if (t->monitor) mon(c, o_o);
 switch (t->output)
 {
#ifdef __DOS16__
   case io_window: putvo_win(c); break;
#endif
   case io_tnc: putv_tnc(c);
#ifndef __FLAT__
    if ((u->paclen>0 && ++b->pacbytes == u->paclen)
       || (u->paclen==0 && m.paclen != 0 && ++b->pacbytes == m.paclen)
       )
      putflush();
#endif
     break;
#ifdef __FLAT__
   case io_socket: putv_sock(c); break;
#endif
#ifdef FEATURE_SERIAL
   case io_tty: putv_tty(c); break;
#endif
   case io_file: if (b) fputc(c, b->outputfile); break;
#ifdef MACRO
   case io_memory:
     if (b && b->outbuffer)
     {
       b->outbuffer[b->outbindex++] = (char) c;
       if (b->outbindex == b->outbmax) longjmp(b->io_mem_jmp, 2);
     }
     break;
#endif
    default: break;
  }
  if (b) b->txbytes++;
}

/*---------------------------------------------------------------------------*/

int getvalid (void)
//*************************************************************************
//
//*************************************************************************
{
  switch (t->input)
  {
#ifdef __DOS16__
  case io_window: return taskkbhit();
#endif
  case io_tnc:    return getvalid_tnc();
#ifdef __FLAT__
  case io_socket: return getvalid_sock();
#endif
#ifdef FEATURE_SERIAL
  case io_tty:    return getvalid_tty();
#endif
  case io_file:
  case io_dummy:
#ifdef MACRO
  case io_memory:
#endif
    return OK;
  default: break;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

int bgetv (void)
//*************************************************************************
//
//*************************************************************************
{
  int c = 0;

  switch (t->input)
  {
#ifdef __DOS16__
    case io_window: c = waitkey() & 255; break;
#endif
    case io_tnc:    c = getv_tnc(); break;
#ifdef __FLAT__
    case io_socket: c = getv_sock(); break;
#endif
#ifdef FEATURE_SERIAL
    case io_tty:    c = getv_tty(); break;
#endif
    case io_file:   c = fgetc(b->inputfile); break;
#ifdef MACRO
    case io_memory:
      if (b && b->inbuffer)
      {
        if (b->inbindex < b->inbmax)
          c = b->inbuffer[b->inbindex++];
        else
          if (b->inbindex == b->inbmax)
          {
            c = LF;
            b->inbindex++;
          }
          else
            longjmp(b->io_mem_jmp, 1);
      }
      break;
#endif
    default: break;
  }
  if (t->monitor) mon(c, o_i);
  if (b) b->rxbytes++;
  return c;
}

/*---------------------------------------------------------------------------*/

int getv (void)
//*************************************************************************
//
//*************************************************************************
{
#ifdef __FLAT__
  if (b && b->charset) return charset_rx(bgetv());
#endif
  return bgetv();
}

/*---------------------------------------------------------------------------*/

int putfree (void)
//*************************************************************************
//
//*************************************************************************
{
  switch (t->input)
  {
  case io_tnc:    return putfree_tnc();
#ifdef __FLAT__
  case io_socket: return putfree_sock();
#endif
#ifdef FEATURE_SERIAL
  case io_tty:    return putfree_tty();
#endif
  case io_file:
#ifdef MACRO
  case io_memory:
#endif
#ifdef __DOS16__
  case io_window:
#endif
    return 1;
  default: break;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void putflush (void)
//*************************************************************************
//
//*************************************************************************
{
  switch (t->input)
  {
#ifdef __DOS16__
  case io_window: putflush_win(); break;
#endif
  case io_tnc:    putflush_tnc(); break;
#ifdef __FLAT__
  case io_socket: putflush_sock(); break;
#endif
#ifdef FEATURE_SERIAL
  case io_tty:    putflush_tty(); break;
#endif
  case io_file:
#ifdef MACRO
  case io_memory:
#endif
  default: break;
  }
}

/*---------------------------------------------------------------------------*/

#ifdef MACRO
static void near inputline_mem (char *s, int maxlen, char cut)
//*************************************************************************
//
//*************************************************************************
{
  int i = 0;
  *s = 0;
  if (t->input == io_memory && b)
  {
    if (maxlen < 0) maxlen = (-maxlen);
    while (i < maxlen)
    {
      s[i++] = getv();
      s[i] = 0;
      if (s[i - 1] == LF)
      {
        if (cut) s[i - 1] = 0;
        break;
      }
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/

static char near inputline_file (char *s, int maxlen, char cut)
//*************************************************************************
//
//*************************************************************************
{
  *s = 0;
  if (t->input == io_file && b && b->inputfile)
  {
    if (maxlen < 0) maxlen = (-maxlen);
    do
    {
      if (! fgets(s, maxlen, b->inputfile))
      {
        s_fclose(b->inputfile);
        b->inputfile = NULL;
        t->input = (io_t) b->oldinput;
        return NO;
      }
      waitfor(e_ticsfull);
    }
    while (! *s);
    if (cut)
    {
      if (s[strlen(s) - 1] == LF)
        s[strlen(s) - 1] = 0;
    }
    // DH3MB Removed check for "impdel" (this is now a mailbox-command) and
    // "end" (is not needed any more), so this function now is completely transparent
    return OK;
  }
  else return NO;
}

/*---------------------------------------------------------------------------*/


//static char near inputline_comp (char *s, int maxlen, char cut)
//*************************************************************************
//
//*************************************************************************
/*
{
  *s = 0;
  if (t->input == io_file && b && b->inputfile)
  {
    if (maxlen < 0) maxlen = (-maxlen);
    do
    {
      if (! fgets(s, maxlen, b->inputfile))
      {
        s_fclose (b->inputfile);
        b->inputfile = NULL;
        t->input = (io_t) b->oldinput;
        return NO;
      }
      waitfor (e_ticsfull);
    }
    while (! *s);
    if (cut)
    {
      if (s[strlen(s) - 1] == LF)
        s[strlen(s) - 1] = 0;
    }
    // DH3MB Removed check for "impdel" (this now is a mailbox-command) and
    // "end" (is not needed any more), so this function now is completely transparent
    return OK;
  }
  else return NO;

}
*/

/*---------------------------------------------------------------------------*/

char getline (char *s, int maxlen, char cut)
//*************************************************************************
//
//  Gets one line with at most maxlen characters from the current input
//  (t->input)
//
//  If cut==1, the line will be cut if there are more than maxlen
//  characters and the final '\n' is not added to the returned string
//  If cut==0 the function will return after having read maxlen bytes or
//  a '\n' (DH3MB)
//
//  If maxlen<0, there may be more commands stated in one line
//  seperated by semicolons (";"). Note that the seperation with ";"s
//  will not work on io_file, io_win and io_mem, and not if cut==0
//
//  Returns 0, if(t->input==io_file && feof(b->inputfile)), 1 otherwise
//
//*************************************************************************
{
  lastfunc("getline");

  char retval = OK;
  if (b)
    b->linecount = b->abbruch = 0;
  t_unlock(1);
  switch (t->input)
  {
#ifdef __DOS16__
    case io_window: inputline_win(s, maxlen, cut); break;
#endif
    case io_tnc:    inputline_tnc(s, maxlen, cut); break;
#ifdef __FLAT__
    case io_socket: inputline_sock(s, maxlen, cut); break;
#endif
#ifdef FEATURE_SERIAL
    case io_tty:    inputline_tty(s, maxlen, cut); break;
#endif
#ifdef MACRO
    case io_memory: inputline_mem(s, maxlen, cut); break;
#endif
//    case io_comp:   retval = inputline_comp(s, maxlen, cut); break;
    case io_file:   retval = inputline_file(s, maxlen, cut); break;
    case io_dummy:
                    if (t->output == io_file)
                    {
                      s_fclose(b->outputfile);
                      b->outputfile = NULL;
                      t->input = (io_t) b->oldinput;
                      t->output = (io_t) b->oldoutput;
                    }
                    *s = 0; // return empty line
    default: break;
  }
  if (b)
    b->rxbytes += strlen(s) + 1;
  t->last_input = ad_time();
  if (t->monitor)
  {
    unsigned int i = 0;
    while (s[i]) mon(s[i++], o_i);
    mon(LF, o_i);
  }
  return retval;
}

/*---------------------------------------------------------------------------*/
