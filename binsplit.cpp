/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  BINSPLIT-Protocol
  -----------------

  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990703 DH3MB  Begin of implementation

#include "baycom.h"

#ifdef FEATURE_BINSPLIT

/*---------------------------------------------------------------------------*/

//fileio_binsplit::fileio_binsplit (void) //: fileio()
//{
//  f = (file_info_t *) t_malloc(sizeof(file_info_t), "fio");
//  *fname = 0;
//}

/*---------------------------------------------------------------------------*/

//fileio_binsplit::~fileio_binsplit (void)
//{
//  t_free(f);
//}


/*---------------------------------------------------------------------------*/

void fileio_binsplit::set_sendname (const char *name)
{
  strcpy(fname, name);
}

/*---------------------------------------------------------------------------*/

void fileio_binsplit::putheaderitem (crcthp *headercrc, char *format, ...)
{
  va_list argpoint;
  char cbuf[260];
  char *s;

  s = cbuf;
  va_start(argpoint, format);
  vsprintf(cbuf, format, argpoint);
  va_end(argpoint);
  if (strlen(s) > 257)
  {
    s[50] = 0;
    trace(tr_abbruch, "putbs", "long %s", s);
  }
  while (*s)
  {
    headercrc->update(*s);
    putv(*(s++));
  }
  headercrc->update(LF);
  bputv(LF);
}

/*---------------------------------------------------------------------------*/

void fileio_binsplit::putheader (char num, char total)
{
  crcthp headercrc;

  putf("Part %02Xh of %02Xh of \"%s\" (splitsize 20000):\n",
                num, total, fname);
  bputv(LF);
  unsigned long dostime;
  unsigned short int crc;
  unsigned long int size;
  size = filesize(fname);
  dostime = getdostime(file_isreg(fname));
  crcthp crc_;
  crc_.readfile(fname, 0);
  crc = crc_.result;
  putf("#BIN#%ld#|%05u#$%08lX#%s\n", size, crc, dostime, fname);
  putheaderitem(&headercrc, BS_MAGIC);
  putheaderitem(&headercrc, "%02X", num);
  putheaderitem(&headercrc, "%02X", total);
  putheaderitem(&headercrc, "%ld", BINSPLIT_SIZE);
  putheaderitem(&headercrc, "%ld", fsize);
  putheaderitem(&headercrc, "%s", fname);
  putheaderitem(&headercrc, "%s", datestr(ftime, 6));
  bputv(0);
  headercrc.update((char) 0);
  bputv(headercrc.result % 0x100);
  bputv(headercrc.result / 0x100);
  bputv(LF);
}

/*---------------------------------------------------------------------------*/

void fileio_binsplit::tx (void)
{
  char partnum, part;
  long byte;
  long offset = 0L;
  int a;
  FILE *bf;

  if ((ftime = file_isreg(fname)) == 0L)
  {
    putf(ms(m_filenotfound), fname);
    return;
  }
  fsize = filesize(fname);
  partnum = fsize / BINSPLIT_SIZE;
  if (fsize > partnum * BINSPLIT_SIZE)
    partnum++;
  for (part = 1; part <= partnum; part++)
  {
     //plen = BINSPLIT_SIZE;
     putheader(partnum, part);
     byte = 0;
     if ((bf = s_fopen(fname, "lrb")) != NULL)
     {
       fseek(bf, offset, SEEK_SET);
       while ((a = fgetc(bf)) != EOF)
       {
         byte++;
         if (byte <= BINSPLIT_SIZE)
           bputv(a);
         else break;
       }
       offset = offset + byte;
       s_fclose(bf);
     }
     bputv(LF);

/* from BS.C Axel Bauda
    // calculate part size (last part may be smaller than splitsize!)
//    if (part != partnum-1)
//      plen = ilen;
//    else
//      plen = len;
//    len -= ilen;


    // skip parts, if required
//    if ((flags & RECODE) && !range[part])
//    {
//      if (bigcrc)
//      {
//
//       upd32crc(fgetc(inf), crc32all);
//      }
//      else
//        fseek(inf, plen, SEEK_CUR);
//      continue;
//    }


    // use u01-uff for seperate parts that contain #BIN#-headers
    if (flags & BINHEAD)
      _ext = 'u';

    sprintf(pfn , "%s.%c%02x", name, _ext, part);

    if (*uplname)
    {
      if (flags & JOIN)
      {
        if (uplname[(int)strlen(uplname)-1] == PATHCHAR)
          sprintf(pfn, "%s%s.upl", uplname, name);
        else
          strcpy(pfn, uplname);
      }
      else
      {
        if (uplname[(int)strlen(uplname)-1] == PATHCHAR)
          sprintf(pfn, "%s%s.%c%02x", uplname, name, _ext, part);
        else
          sprintf(pfn, "%s.%c%02x", uplname, _ext, part);
      }
    }
    else
      if (flags & JOIN)
        sprintf(pfn , "%s.upl", name);

    if (!(flags & JOIN) ||
        ((flags & JOIN) && (part == 1 || part == first_part)))
      if ((otf = fopen(pfn, OPEN_WR_BIN)) == NULL)
      {
        fprintf(o, g?"\nKann \"%s\" nicht erzeugen.\n":
                     "\nCannot create \"%s\".\n", pfn);
        exit(7);
      }

    sprintf(pfn , "%s.%c%02x", name, _ext, part);

    setvbuf(otf, NULL, _IOFBF, 16384);

    if ((flags & SEND) && (flags & BINHEAD))
    {
      sprintf(s, "%s", name);
      if (!skipbshead)
        sprintf(s, "%s.b%02x/%02lx", name, part, nparts);
      else
        if (flags & FORCE8_3)
          sprintf(s, "%s.%s", name, ext);

      if (!*sendhead)
        fprintf(otf, "%s %s%s%s", send, (flags & ADDCR)?lsep:"", s, lsep);
      else
        top(otf, buffer, ofn, part, (int)nparts);
    }

       sprintf(s, ">BS<\012%02X\012%02lX\012%ld\012%ld\012%s\012%s\012"
                               ".BS v"VERSION" (c) DG1BBQ\012",
                               part, nparts, ilen, orglen, ofn, datestring);
    // Do not use pointer s from here to rem'd out copy of above command!
    if (flags & BINHEAD)
    {
      long l;

      if (!skipbshead)
        fprintf(otf, "%s%sPart %02Xh of %02lXh of \"%s\" (splitsize %ld):%s%s",
                                                       lsep, lsep, part, nparts,
                                                       ofn, ilen, lsep, lsep);

      fprintf(otf, "<BS"VERSION"/"DATE" by DG1BBQ>\%s", lsep);

      if (part == 1 || ((flags & RECODE) && part == first_part))
        binstartpos = ftell(otf);

      fprintf(otf, "#BIN#");

      if (!skipbshead)
      {
        if (bigcrc)
          l = strlen(s) + 24L;
        else
          l = 8L + (long) strlen(ofn);

        if (part == nparts)
        {
          l += plen;
          if (!bigcrc)
            l += 4L;
        }
        else
          l += ilen;
      }
      else
        l = plen;

      fprintf(otf, "%ld#|", l);
      bincrcpos = ftell(otf);

      if (!skipbshead)
      {
        fprintf(otf, "00000#$%08lX#", curtime);
        sprintf(pfn, "%s.b%02x", name, part);
      }
      else
      {
        fprintf(otf, "00000#$%08lX#", timestamp);
        if (flags & FORCE8_3)
          sprintf(pfn, "%s%s%s", name, (*ext?".":""), ext);
        else
          strcpy(pfn, name);
      }
      fprintf(otf, "%s\x0d", pfn);
      bsstartpos = ftell(otf);
    }

    if (!skipbshead)
    {
      if (bigcrc)
      {
        crc = 0;
        i = 0;
        while (s[i])
          updcrc(s[i++], crc);
        updcrc(0, crc);

        fwrite(s, 1, i, otf);
        putc(0, otf);
        fprintf(otf, "%04X\012", crc);
      }
      else
      {
        write_16(MAGIC, otf);
        fputc((uint) nparts, otf);
        fputc(part, otf);
        bscrcpos = ftell(otf);
        write_16(0, otf);

        if (part == nparts)
          fputc((int) strlen(ofn) + 5, otf);
        else
          fputc((int) strlen(ofn) + 1, otf);
        fwrite(ofn, (int) strlen(ofn) + 1, 1, otf);

        if (part == nparts)
          write_32(timestamp, otf);
      }
    }

    crc = 0;
    crc32 = 0xffffffffL;
    while (plen--)
    {
      ch = fgetc(inf);
      if (bigcrc)
      {
        upd32crc(ch, crc32);
        upd32crc(ch, crc32all);
      }
      else
        updcrc(ch, crc);
      fputc(ch, otf);
    }

    if (! skipbshead)
    {
      if (bigcrc)
      {
        char dummy[10];
        sprintf(dummy, "%08lX\012", crc32all ^ 0xffffffffL);
        fprintf(otf, "%08lX\012", crc32all ^ 0xffffffffL);
        i = 0;
        while (i < 9)
          upd32crc(dummy[i++], crc32);

        fprintf(otf, "%08lX\012", crc32 ^ 0xffffffffL);
      }
      else
      {
        fseek(otf, bscrcpos, SEEK_SET);
        write_16(crc, otf);
      }
    }

    if (flags & BINHEAD)
    {
      crc = 0;
      fseek(otf, bsstartpos, SEEK_SET);
      while ((ch = fgetc(otf)) != EOF)
        updcrc(ch, crc);
      fseek(otf, bincrcpos, SEEK_SET);
      fprintf(otf, "%05u", crc);
    }

    fseek(otf, 0L , SEEK_END);
    // This is for terminals that can't handle back-to-back BIN files.
    // Not very elegant, I admit
    if ((flags & JOIN)     &&
        (flags & DUMB_BIN) &&
         binstartpos)
    {
      int i, j;
      long k;

      k = 256L - binstartpos;
      for (i = 0; i < 3; i++)
      {
        for (j = 79; j > 0; j--)
        {
          fputc(' ', otf);
          if (--k == 1)
            break;
        }
        fputc('\x0d', otf);
        if (!--k)
          break;
      }
    }

    if (!(flags & JOIN) || part == last_part || part == nparts)
      fclose(otf);

*/
  }
}

/*---------------------------------------------------------------------------*/

#endif
