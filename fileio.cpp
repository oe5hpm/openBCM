/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------------------------
  Filetransfer base class + text- and AutoBIN-methods
  ---------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980315 hrx    void fileio::send_autobin -> on rbin/rprg: if
//                requested file's length is 0 bytes we don't send
//                out the file.
//19980430 OE3DZW tmp-files in tmp-path, not in bcm-path
//19990206 OE3DZW added check for missing filename
//19990618 DH3MB  Fixed send_autobin(): 0-byte-files were not closed
//                Cleaned up recv_autobin()
//                Adapted for new CRC classes
//19990703 DH3MB  Changed "recv" to "rx" and "send" to "tx"
//19990804 DH3MB  Introduced base classes for file transfer stuff
//19990807 DH3MB  Extended AutoBIN-statistics
//                More informative error messages on text- and autobin-rx
//                Changed type of file size variables to off_t
//19991011 F1ULQ  Corrected bug in rprg , filename in header was shit
//19991025 OE3DZW shorter status
//19991026 OE3DZW removed F1ULQ's change, users complained about it...
//20000108 OE3DZW fixed /ex for wt
//20000116 DK2UI  (dzw) added ui-options
//20000505 DK2UI  added set file time to fileio_abin::_rx()
//20000713 DK2UI  changes to usefile() because missing f->path
//20000807 DK2UI  accept '/EX' only at line start in text rx
//20000910 DK2UI  length of file names limited to 50 char's

#include "baycom.h"

#ifdef __LINUX__
 #include <utime.h>
#endif

/*---------------------------------------------------------------------------*/

fileio::fileio (void)
//****************************************************************************
//
//****************************************************************************
{
  f = (file_info_t *) t_malloc(sizeof(file_info_t), "fio");
  *f->fullname = 0;
  *f->name = 0;
  *f->path = 0;
  f->size = 0;
  f->offset = 0;
}
/*---------------------------------------------------------------------------*/

fileio::~fileio (void)
//****************************************************************************
//
//****************************************************************************
{
  t_free(f);
}

/*---------------------------------------------------------------------------*/

char *fileio::cut_file (char *fname)
//****************************************************************************
//
//****************************************************************************
{
  static char filename[FSPATHLEN + FNAMELEN + 1];
  char *cp;
  strcpy(filename, fname);
  if (strlen(filename) > FNAMELEN)
  {
    if ((cp = strchr(filename, '.')) != NULL)
    {
      *cp++ = 0;
      filename[FNAMELEN - 1 - strlen(cp)] = '~';
      filename[FNAMELEN - strlen(cp)] = 0;
      strcat(filename, cp);
    }
    else
    {
      filename[FNAMELEN - 1] = '~';
      filename[FNAMELEN] = 0;
    }
  }
  return filename;
}

/*---------------------------------------------------------------------------*/

void fileio::usefile (char *filename)
//****************************************************************************
//
//****************************************************************************
{
  char *cp;

  if (! *filename) return;
#ifdef __UNIX__
  if (*filename == '/' || *filename == '\\')
    strcpy(f->fullname, filename);
  else
  {
    getcwd(f->fullname, FSPATHLEN + FNAMELEN - strlen(filename) - 1);
    strcat(f->fullname, "/");
    strcat(f->fullname, filename);
  }
#else
  strcpy(f->fullname, filename);
#endif
  killbackslash(f->fullname);
  if ((cp = strrchr(f->fullname, '/')) != NULL)
    strcpy(f->name, cut_file(cp + 1));
  else
    strcpy(f->name, cut_file(f->fullname));
  strcpy(f->path, f->fullname);
  if ((cp = strrchr(f->path, '/')) != NULL)
  {
    if (cp != f->path) *cp = 0;
    else f->path[1] = 0; // root
  }
#ifdef __UNIX__
  else
    *f->fullname = 0;
#endif
  if ((f->unixtime = file_isreg(f->fullname)) != 0L)
  {
    f->exists = 1;
    f->pos = 0L;
    f->size = filesize(f->fullname) - f->offset;
    f->dostime = getdostime(f->unixtime);
#ifdef FEATURE_MDPW
    if (f->requires & REQ_MD5HASH)
    {
      MD5 md5;
      md5.readfile(f->fullname, f->offset);
      md5.getdigest(f->md5hash);
      md5.gethexdigest(f->md5str);
    }
#endif
    if (f->requires & REQ_CRCTHP)
    {
      crcthp crc_;
      crc_.readfile(f->fullname, f->offset);
      f->crc = crc_.result;
    }
  }
  else
    f->exists = 0;
  f->eraseflag = 0;
}

/*---------------------------------------------------------------------------*/

void fileio::set_offset (off_t offset)
//*************************************************************************
//
//  Specifies the position, where to start with the file transmission
//  That's nothing to do with resume - the file will be sent as
//  if the preciding part of the file did not exist!
//
//  This method *MUST* be called before usefile()!
//
//*************************************************************************
{
  f->offset = offset;
}

/*---------------------------------------------------------------------------*/

void fileio::doerase (void)
//****************************************************************************
//
//****************************************************************************
{
  f->eraseflag = 1;
}

/*---------------------------------------------------------------------------*/

void fileio::tx (void)
//****************************************************************************
//
//****************************************************************************
{
  if (! *f->fullname)
  {
    putf(ms(m_filemissing));
    return;
  }
  if (! f->exists)
  {
    putf(ms(m_filenotfound), f->fullname);
    return;
  }
  _tx();
}

/*---------------------------------------------------------------------------*/

void fileio::rx (void)
//****************************************************************************
//
//****************************************************************************
{
  if (! *f->fullname)
  {
    putf(ms(m_filemissing));
    return;
  }
 _rx();
}

/*---------------------------------------------------------------------------*/

fileio_text::fileio_text (void) : fileio()
//****************************************************************************
//
//****************************************************************************
{
  tail = 0;
  f->requires = 0;
}

/*---------------------------------------------------------------------------*/

void fileio_text::settail (signed short int si)
//****************************************************************************
//
//****************************************************************************
{
  tail = si;
}

/*---------------------------------------------------------------------------*/

void fileio_text::_tx(void)
//*************************************************************************
//
//  Sends a text file
//
//*************************************************************************
{
  int a, last = 0;

  if ((f->ptr = s_fopen(f->fullname, "lrt")) != NULL)
  {
    if (f->eraseflag) s_fsetopt(f->ptr, 2);
    if (tail < 0 && (f->size) > (-tail))
    {
      fseek(f->ptr, tail, SEEK_END);
      while ((a = fgetc(f->ptr)) != LF && a != EOF); // go to begin of next line
    }
    while ((a = fgetc(f->ptr)) != EOF)
    {
      last = a;
      putv(a);
      waitfor(e_ticsfull);
      if (testabbruch())  break;
      if (tail > 0 && (f->pos) > tail && a == LF) break;
      f->pos++; //wg mancher compileroptimierungen ausserhalb der Abfrage
    }
    if (last != LF) putv(LF);
    s_fclose(f->ptr);
  }
  else
    putf(ms(m_filenotopen), f->fullname);
}

/*---------------------------------------------------------------------------*/

void fileio_text::_rx (void)
//*************************************************************************
//
//  Receives a text file
//
//*************************************************************************
{
  char *line = b->line;
  char *eof;
  char tmpname[20];

  if (f->exists) putf(ms(m_fileexists));
  putf(ms(m_filewaiting), "TEXT", f->fullname);
  putflush();
  sprintf(tmpname, TEMPPATH "/%s", time2filename(0));
  if ((f->ptr = s_fopen(tmpname, "lwt")) != NULL)
  {
    s_fsetopt(f->ptr, 1);
    do
    {
      getline(line, BUFLEN - 1, 1);
//      subst(line, 0x01, CTRLZ); //also accept CTRLA as eof-marker
      if (! strnicmp(line, "/EX", 3)) eof = line;
      else eof = strchr(line, CTRLZ);
      if (eof)
      {
        *eof = 0;
        if (strlen(line))
        {
          fputs(line, f->ptr);
          fputc(LF, f->ptr);
        }
      }
      else
      {
        fputs(line, f->ptr);
        fputc(LF, f->ptr);
      }
    }
    while (! eof);
    if (! ftell(f->ptr))
    {
      s_fclose(f->ptr);
      putf(ms(m_filenotstored));
      xunlink(tmpname);
    }
    else
    {
      s_fclose(f->ptr);
      xunlink(f->fullname);
      if (file_isreg(f->fullname))
        putf(ms(m_filenotrm), tmpname);
      else
      {
        if (xrename(tmpname, f->fullname))
          putf(ms(m_filewriteerror), tmpname);
        else
          putf(ms(m_filestored), f->fullname);
      }
    }
  }
  else putf(ms(m_filenotopen), tmpname);
}

/*---------------------------------------------------------------------------*/

fileio_abin::fileio_abin (void) : fileio()
//****************************************************************************
//
//****************************************************************************
{
  *autobin_header = 0;
  f->requires = REQ_CRCTHP;
}

/*---------------------------------------------------------------------------*/

void fileio_abin::set_header (char *str)
//****************************************************************************
//
//****************************************************************************
{
  strncpy(autobin_header, str, LINELEN);
  autobin_header[LINELEN] = 0;
}

/*---------------------------------------------------------------------------*/

void fileio_abin::_tx (void)
//*************************************************************************
//
//  Sends a file using the AutoBIN protocol
//
//*************************************************************************
{
  int a;
  unsigned long int bps;
  time_t startt, txtime;
  if (! f->size)
  {
    putf(ms(m_filenobytes), f->fullname);
    return;
  }
  if ((f->ptr = s_fopen(f->fullname, "lrb")) != NULL)
  {
    if (f->eraseflag) s_fsetopt(f->ptr, 2);
    fseek(f->ptr, f->offset, SEEK_SET);
    if (*autobin_header)
      putf("%s", autobin_header);
    else
      putf("#BIN#%ld#|%05u#$%08lX#%s\n", f->size, f->crc, f->dostime, f->name);
    startt = ad_time();
    while ((a = fgetc (f->ptr)) != EOF) bputv(a);
    txtime = ad_time() - startt;
    if (! txtime) txtime = 1L;
    bps = f->size / txtime;
    putf("\n");
    putf(ms(m_rxok), "BIN", f->crc, f->crc, bps << 3);
    s_fclose(f->ptr);
  }
  else putf(ms(m_filenotopen), f->fullname);
}

/*---------------------------------------------------------------------------*/

void fileio_abin::_rx (void)
//*************************************************************************
//
//  Receives a file using the AutoBIN protocol
//
//*************************************************************************
{
  char *line = b->line;
  char tmpname[20];
  int a;
  off_t flen = 0L, len;
  char rxed_crc = 0;
  unsigned short int rx_crc = 0;
  crcthp crc_;
  unsigned long int bps;
  time_t startt = ad_time(), rxtime;
  unsigned long bintime;

  sprintf(tmpname, TEMPPATH "/%s", time2filename(0));
  if (f->exists) putf(ms(m_fileexists));
  putf(ms(m_filewaiting), "AutoBIN", f->fullname);
  putflush();
  if ((f->ptr = s_fopen(tmpname, "lwb")) != NULL)
  {
    s_fsetopt(f->ptr, 1);
    // Wait for the #BIN#-Header, ignore empty lines
    do getline(line, BUFLEN - 1, 1);
    while (! *line);
    if (strstr(line, "#BIN#") == line)
    {
      putf("#OK#\n");
      putflush();
      // Get the file length
      flen = len = atol(line+5);
      // Get the CRC
      if (strstr(line, "#|"))
      {
        rx_crc = (unsigned short int) atol(strstr(line, "#|") + 2);
        rxed_crc = 1;
      }
      sscanf(strchr(line, '$'), "$%lX#", &bintime);
      while (len--)
      {
        a = getv();
        fputc(a, f->ptr);
        crc_.update(a);
      }
    }
    if (! ftell(f->ptr))
    {
      s_fclose(f->ptr);
      putf(ms(m_filenotstored));
      xunlink(tmpname);
      return;
    }
    s_fclose(f->ptr);
    if (rxed_crc && (rx_crc != crc_.result))
    {
      putf(ms(m_filecrcerror));
      xunlink(tmpname);
      return;
    }
    rxtime = ad_time() - startt;
    if (! rxtime) rxtime = 1L;
    bps = flen / rxtime;
    putf("\n");
    putf(ms(m_rxok), "BIN", crc_.result, crc_.result, bps << 3);
    if (f->exists && xunlink(f->fullname))
      putf(ms(m_filenotrm), tmpname); // DH3MB
    else
    {
      if (xrename(tmpname, f->fullname))
        putf(ms(m_filewriteerror), tmpname);
      else
        setfiletime(f->fullname, bintime);
    }
  }
  else putf(ms(m_filenotopen), tmpname);
}

/*---------------------------------------------------------------------------*/
