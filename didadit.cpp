/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------
  DIDADIT-Protocol for BCM
  ------------------------

  Copyright (C)       Johann Hanne, DH3MB, jonny@1409.org
                      Rot-Kreuz-Gasse 9
                      D-85072 Eichstaett

  Alle Rechte vorbehalten / All Rights Reserved

  DIDADIT is (c) 1409.org, see http://1409.org/projects/didadit/

 ***************************************************************/

//19990401 DH3MB  Begin of implementation (SPEC V0.5.3)
//19990406 DH3MB  Switched to SPEC V0.5.4
//19990416 DH3MB  Switched to SPEC V0.5.5
//19990520 DH3MB  Switched to SPEC V0.5.7
//20000312 DH3MB  Switched to SPEC V0.9.1
//20000316 DH3MB  Phew - transmission finally works somehow
//20000322 DH3MB  Reception also works
//                However, there are severel things to do
//20000711 DG4IAD Now ::usefile() is used to get the path
//                to store in. fixed unstuffing
//20000712 DG4IAD Reverted to previous behaviour. Now ::usepath()
//                sets the path. New methods getfilename,getsize
//                for binmail.
//20000713 DK2UI  added #define DEBUG_DIDADIT
//20020524 DK2UI  added setfiletime
//20020927 DF3VI  DGET Zugriffsrecht wie bei BGET
//20030708 DH8YMB Fixed MD5-Problem with RX
//20030710 DH8YMB CPS-Ausgabe bei erfolgreichem RX/TX


#include "baycom.h"

#ifdef FEATURE_DIDADIT

/*---------------------------------------------------------------------------*/

fileio_dida::fileio_dida (void) : fileio()
//*************************************************************************
//
//*************************************************************************
{
  bl = (struct block_info *) t_malloc(sizeof(struct block_info), "dida");
  bl->maxlen = 0;
  f->requires = REQ_MD5HASH;
  waitforheader = 1;
  *sendname = 0;
}

/*---------------------------------------------------------------------------*/

fileio_dida::~fileio_dida (void) // TODO? : fileio::~fileio()
//*************************************************************************
//
//*************************************************************************
{
  if (bl->maxlen > 0) free_blockmem();
  t_free(bl);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::set_path (const char *path)
//*************************************************************************
//
//*************************************************************************
{
  strcpy(f->path, path);
  strcpy(f->fullname, "/dev/null");
}

/*---------------------------------------------------------------------------*/

void fileio_dida::set_waitforheader (int bo)
//*************************************************************************
//
//*************************************************************************
{
  waitforheader = bo;
}

/*---------------------------------------------------------------------------*/

void fileio_dida::get_filename (char *name)
//*************************************************************************
//
//*************************************************************************
{
  strcpy(name, f->name);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::set_sendname (const char *name)
//*************************************************************************
//
//*************************************************************************
{
  strcpy(sendname, name);
}

/*---------------------------------------------------------------------------*/

unsigned long fileio_dida::get_size ()
//*************************************************************************
//
//*************************************************************************
{
//  return (f->size);
  return filesize(f->fullname);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_stuffed (char ch)
//*************************************************************************
//
//*************************************************************************
{
  switch (ch)
  {
  case FEND: bputv(FESC); bputv(TFEND); break;
  case FESC: bputv(FESC); bputv(TFESC); break;
  default:   bputv(ch); break;
  }
}

/*---------------------------------------------------------------------------*/

char fileio_dida::rx_stuffed (void)
//*************************************************************************
//
//*************************************************************************
{
  char ch = getv();
  stuffresult = 0;

  switch (ch)
  {
  case FEND: stuffresult = 1; return(0);
  case FESC: ch = getv();
    switch (ch)
    {
    case TFEND: return(FEND);
    case TFESC: return(FESC);
    default:
      trace(serious, "debug", "unstuff, received FESC, then: %c", ch);
      stuffresult = 2;
      return(0);
    }
  default: return(ch);
  }
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_block (unsigned short int type, char tx_md5hash,
                            char *data, unsigned long int length)
//*************************************************************************
//
//*************************************************************************
{
  int i;
  crcthp crc_;
  bputv(FEND);
  MD5 md5;
  md5.readfile(f->fullname, f->offset);
  md5.getdigest(f->md5hash);
#ifdef DEBUG_DIDADIT
//  trace(report, "debug", "md5hash: %s", f->md5hash);
#endif
  tx_stuffed(type % 256);
  tx_stuffed(type / 256);
  crc_.update(type % 256);
  crc_.update(type / 256);
  if (tx_md5hash)
    for (i = 0; i < 16; i++)
    {
      tx_stuffed(f->md5hash[i]);
      crc_.update(f->md5hash[i]);
    }
  while (length--)
  {
    tx_stuffed(*data);
    crc_.update(*(data++));
  }
  tx_stuffed(crc_.result % 256);
  tx_stuffed(crc_.result / 256);
  bputv(FEND);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::rx_block ()
//*************************************************************************
//
//  Empfängt ein DIDADIT-Frame; es wird dabei solange empfangen, bis
//  ein korrektes Frame angekommen ist; alle ungültigen Frames werden
//  einfach verworfen (egal, ob es sich um einen CRC-Fehler handelt,
//  das Frame zu lang war, etc.).
//
//  An das letzte empfange Byte in data wird ein 0-Byte angehängt, was
//  für die Auswertung von Text-Blöcken ganz nützlich ist.
//
//*************************************************************************
{
  char rxstate;
  char typelen; // Number of type-bytes already received
  int md5len; // Number of md5hash-bytes already received
  unsigned short int datalen; // Number of data-bytes already received
  char ch;
  crcthp crc_;
  char crcbuf[2]; // Die CRC wird um 2 Bytes verzögert mitgerechnet, um
                  // die empfangene CRC selbst nicht miteinzuberechnen

  if (! bl->maxlen)
  {
    trace(serious, "rx didadit block", "no mem allocated");
    return;
  }

  for (;;)
  {
    rxstate = R_RX_STATE_TYPE;
    typelen = 0;
    md5len = 0;
    datalen = 0;
    crc_.result = 0; // TODO: Make crc_.reset() method

    for (ch = rx_stuffed(); stuffresult != 1; ch = rx_stuffed())
    {
      if (stuffresult == 2)
      { // Received an invalid ESCape-sequence
        rxstate = R_RX_STATE_INVALID;
#ifdef DEBUG_DIDADIT
        trace(serious, "debug", "***1");
#endif
      }
      // Wenn die Typ-ID (zwei Bytes) vollständig empfangen wurde, kann mit
      // der Berechnung der CRC begonnen werden
      if (typelen == 2) crc_.update(*crcbuf);
      crcbuf[0] = crcbuf[1];
      crcbuf[1] = ch;
      switch (rxstate)
      {
      case R_RX_STATE_TYPE:
        if (typelen == 0) *((char *) &bl->type) = ch; // Received first type-byte
        else // Received second type-byte
        {
          *(((char *) &bl->type) + 1) = ch;
          // Frame-types which include a md5-hash:
          if (   bl->type == R_BLOCK_ERR
              || bl->type == R_BLOCK_DATA
              || bl->type == R_BLOCK_REQ
              || bl->type == R_BLOCK_FIN
              || bl->type == R_BLOCK_FINACK
              || bl->type == R_BLOCK_ABORT)
            rxstate = R_RX_STATE_MD5HASH;
          else
            rxstate = R_RX_STATE_DATA;
        }
        typelen++;
        break;
      case R_RX_STATE_MD5HASH:
        bl->md5hash[md5len] = ch;
#ifdef DEBUG_DIDADIT
//      trace(report, "debug", "state %c", bl->md5hash[md5len]);
#endif
        if (++md5len == 16) rxstate = R_RX_STATE_DATA;
        break;
      case R_RX_STATE_DATA:
        bl->data[datalen++] = ch;
        if (datalen > (bl->maxlen + 2)) rxstate = R_RX_STATE_INVALID;
        break;
      case R_RX_STATE_INVALID:
        // Frame is already known to be invalid, do nothing
        break;
      }
    }
    // Frame not complete?
    if (! (rxstate == R_RX_STATE_DATA && datalen >= 2))
    {
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "***2");
#endif
      continue;
    }
    // Check the CRC (stored in the last two bytes of the frame)
    datalen -= 2;
    if (*(unsigned short int*) (bl->data + datalen) == crc_.result)
    { // CRC ok
      bl->data[datalen] = 0;
      break;
    }
    else
    {
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "***3");
#endif
      waitfor(e_ticsfull);
    }
  }
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "rx block ended");
#endif
  bl->len = datalen;
}

/*---------------------------------------------------------------------------*/

void fileio_dida::alloc_blockmem (unsigned short int size)
//*************************************************************************
//
//  Allocates memory for receiving a DIDADIT block
//
//  rx_block checks, if the allocated memory for the block is sufficient
//  if not, the received block is silently ignored
//
//*************************************************************************
{
  if (bl->maxlen > 0) free_blockmem();
  bl->data = (char *) t_malloc(size + 2, "dida");
  bl->maxlen = size;
}

/*---------------------------------------------------------------------------*/

void fileio_dida::free_blockmem ()
//*************************************************************************
//
//  Frees memory allocated with alloc_blockmem()
//
//*************************************************************************
{
  t_free(bl->data);
  bl->maxlen = 0;
}

/*---------------------------------------------------------------------------*/

char *fileio_dida::parse_textblock (char *block, char **id, char **value)
//*************************************************************************
//
//  Wird zum parsen eines DIDADIT-Text-Blocks (INFO, START)
//  verwendet, das Informationen in der Form "ID=Wert\r" enthält
//  Es werden das "=" und das "\r" (=CR) durch ein 0-Byte ersetzt
//  *id wird auf den Anfang der ID gesetzt, *value auf den Anfang
//  des Werts
//  Zurückgegeben wird ein Pointer auf das nächste Feld oder NULL,
//  wenn kein Feld gefunden wurde
//
//  Wichtig: Der gesamte Block muß mit einem 0-Byte abgeschlossen sein,
//  außerdem wird der übergebene Block verändert!
//
//*************************************************************************
{
  char *equal, *end;

  if ((equal = strchr(block, '=')) != 0 && (end = strchr (block, CR)) != 0)
  {
    *equal = 0;
    *end = 0;
    *id = block;
    *value = equal + 1;
    return(end + 1);
  }
  else return(0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::parse_INFO (char *md5str, unsigned short int *rxed_blocksize)
//*************************************************************************
//
//
//
//*************************************************************************
{
  char *block = bl->data, *id, *value;

  *f->fullname = 0;
  *f->name = 0;
//  *f->path = 0;
  f->size = 0L;
  *md5str = 0;
  f->unixtime = 0L;
  *rxed_blocksize = 0;
  *partner_ver = 0;

  while ((block = parse_textblock(block, &id, &value)) != NULL)
  {
    if (! strcasecmp(id, "FILENAME"))
    {
      strncpy(f->fullname, value, FSPATHLEN + FNAMELEN);
      f->fullname[FSPATHLEN + FNAMELEN] = 0;
    }
//    else if (! strcasecmp(id, "PATH"))
//    {
//      strncpy(f->path, value, FSPATHLEN);
//      f->path[FSPATHLEN] = 0;
//    }
    else if (! strcasecmp(id, "SIZE")) f->size = atol(value);
    else if (! strcasecmp(id, "MD5"))
    {
      strncpy(md5str, value, 32);
      md5str[32] = 0;
    }
    else if (! strcasecmp(id, "TIME")) f->unixtime = atol(value);
    else if (! strcasecmp(id, "BLOCKSIZE")) *rxed_blocksize = atoi(value);
    else if (! strcasecmp(id, "VERSION"))
    {
      strncpy(partner_ver, value, 9);
      partner_ver[9] = 0;
    }
  }
}

/*---------------------------------------------------------------------------*/

void fileio_dida::parse_START (unsigned long int *resume_offset,
                               char *part_md5str,
                               unsigned short int *rxed_blocksize)
//*************************************************************************
//
//
//
//*************************************************************************
{
  char *block = bl->data, *id, *value;

  *resume_offset = 0L;
  *part_md5str = 0;
  *rxed_blocksize = 0;
  *partner_ver = 0;
  while ((block = parse_textblock(block, &id, &value)) != NULL)
  {
    if (! strcasecmp(id, "OFFSET")) *resume_offset = atol(value);
    else if (! strcasecmp(id, "PART_MD5"))
    {
      strncpy(part_md5str, value, 32);
      part_md5str[32] = 0;
    }
    else if (! strcasecmp(id, "BLOCKSIZE")) *rxed_blocksize = atoi(value);
    else if (! strcasecmp(id, "VERSION"))
    {
      strncpy(partner_ver, value, 9);
      partner_ver[9] = 0;
    }
  }
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_INFO()
//*************************************************************************
//
//*************************************************************************
{
#ifdef DEBUG_DIDADIT
  trace(serious, "tx_INFO", "s:%s, p:%s, n:%s,", sendname, f->path, f->name);
#endif
  // TODO: overrun check
  if (! *sendname) strcpy(sendname, f->name);
  sprintf(b->line,
          "FILENAME=%s\r"
          "PATH=%s\r"
          "SIZE=%ld\r"
          "MD5=%s\r"
          "TIME=%ld\r"
          "BLOCKSIZE=%d\r"
          "VERSION=" DIDADIT_VERSION "\r",
          sendname, f->path, f->size, f->md5str,
          f->unixtime, DEFAULT_BLOCKSIZE);
  tx_block(R_BLOCK_INFO, 0, b->line, strlen(b->line));
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_START (unsigned long int offset, char *part_md5str,
                            unsigned short int blcksize)
//*************************************************************************
//
//*************************************************************************
{
  sprintf(b->line, "OFFSET=%ld\r", offset);
/* dh8ymb: in der Variablen is nur Schrott, daher erstmal weglassen
  if (offset > 0)
    sprintf(b->line + strlen(b->line), "PART_MD5=%s\r", part_md5str);
*/
  if (blcksize)
    sprintf(b->line + strlen(b->line), "BLOCKSIZE=%d\r", blcksize);
  sprintf(b->line + strlen(b->line), "VERSION=" DIDADIT_VERSION "\r");
  tx_block(R_BLOCK_START, 0, b->line, strlen(b->line));
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_ERR (char *errorinfo)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_ERR, 1, b->line, strlen(b->line));
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_DATA (unsigned long int offset,
                           unsigned short int length, char *data)
//*************************************************************************
//
//*************************************************************************
{
  char *datptr = (char *) t_malloc(length + 6, "dida");
  memcpy(datptr, &offset, 4);
  memcpy(datptr + 4, &length, 2);
  memcpy(datptr + 6, data, length);
  tx_block(R_BLOCK_DATA, 1, datptr, length + 6);
  t_free(datptr);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_REQ (struct REQ_info *REQ, unsigned long int len)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_REQ, 1, (char *) REQ, len * sizeof(struct REQ_info));
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_FIN (char *data)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_FIN, 1, 0, 0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_FINACK (void)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_FINACK, 1, 0, 0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_ECHOREQ (void)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_ECHOREQ, 0, 0, 0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_ECHOREP (void)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_ECHOREP, 0, 0, 0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::tx_ABORT (void)
//*************************************************************************
//
//*************************************************************************
{
  tx_block(R_BLOCK_ABORT, 1, 0, 0);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::update_r_file (char *r_filename, char *md5str,
                                 unsigned long int REQc, struct REQ_info *REQ)
//*************************************************************************
//
//*************************************************************************
{
  int fh;

  if (! REQc)
  {
#ifdef DEBUG_DIDADIT
    trace(serious, "debug", "unlinking rfile");
#endif
    xunlink(r_filename);
    return;
  }
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "update rfile: %d REQs", REQc);
#endif
  fh = s_open(r_filename, "swb");
  // Write the first 32 bytes of the file (the MD5 hex string)
  write(fh, md5str, 32);
  // Write a LF
  write(fh, "\012", 1);
  // Read the list of the parts which we are missing due to invalid
  // DATA frames during the reception of the first part
  write(fh, REQ, REQc*8);
  s_close(fh);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::send_file_data (unsigned long int startbyte,
                                  unsigned long int length,
                                  unsigned long int blcksize)
//*************************************************************************
//
//*************************************************************************
{
  char *buf = (char *) t_malloc(blcksize, "dida");
  unsigned long int rbytes; // Bytes to be read
  unsigned long int rnum; // Bytes successfully read
  f->pos = startbyte;
  lseek(f->handle, f->offset + startbyte, SEEK_SET);
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "start at %d, total len %d", startbyte, length);
#endif
  do
  {
    rbytes = blcksize;
    if (rbytes > length) rbytes = length;
    if ((rnum = read(f->handle, buf, rbytes)) != 0)
    {
      tx_DATA(f->pos, (unsigned short) rnum, buf);
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "offset: %ld, len: %ld", f->pos, rnum);
#endif
      f->pos += rnum;
      length -= rnum;
      waitfor(e_ticsfull);
      // TODO: Ueberpruefen, ob inzwischen etwas empfangen wurde
    }
  }
  while (length && rnum);
  t_free(buf);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::_rx(void)
//*************************************************************************
//
// Empfangen von DIDADIT
//
//*************************************************************************
{
  char md5str[33];
  unsigned short int rxed_blocksize;
  char *data_start = NULL;
  long int rxed_offset;
  unsigned short int rxed_len; // TODO: Auswerten!
  struct REQ_info *REQ = 0;
  unsigned long int REQc = 0, c;
  char found;
  unsigned long int resume_offset;
  unsigned long int bps;
  time_t startt = ad_time(), rxtime;
  char r_filename[FSPATHLEN * 2 + FNAMELEN + 3];
  char r_filemd5str[33];
  char r_filehandle;
  char *cp;

#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "in fileio_dida::_rx");
#endif
  if (waitforheader)
  {
    putf(ms(m_filewaiting), "DIDADIT", "");
    do
      getline(b->line, sizeof(b->line) - 1, 1);
    while (! *b->line); // ignore empty lines
    if (strcmp(b->line, "#DIDADIT#"))
    {
      putf(ms(m_filenotstored));
      return;
    }
  }
  putf("\n#OK#\n");
  putflush();
  alloc_blockmem(DEFAULT_BLOCKSIZE); // TODO: 1000 bytes max.?
  // We now expect an INFO-frame
  rx_block();
  if (bl->type != R_BLOCK_INFO)
  {
    tx_ABORT();
    putf("*** Expected INFO-Frame, got 0x%04X.\n", bl->type);
    return;
  }
  parse_INFO(md5str, &rxed_blocksize);
/*
printf("FILENAME: %s\n"
       "PATH: %s\n"
       "SIZE: %ld\n"
       "MD5: %s\n"
       "TIME: %ld\n"
       "BLOCKSIZE: %d\n"
       "VERSION: %s\n",
       f->name, f->path, f->size, md5str, f->unixtime, rxed_blocksize,
       partner_ver);
*/
  if ((cp = strrchr(f->fullname, '/')) != NULL)
    strcpy(f->fullname, cp + 1);
  if (*f->path && f->path[strlen(f->path) - 1] != '/')
    strcat(f->path, "/");
  strcpy(r_filename, f->path);
  strcat(r_filename, f->fullname);
  time_t modtime = f->unixtime;
  usefile(r_filename); // this deletes f->unixtime
  f->unixtime = modtime;
//  usefile (r_filename);
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "p:%s, n:%s, f:%s, s:%ld", f->path, f->name,
                                                     f->fullname, f->size);
#endif
  strcpy(r_filename, f->fullname);
  strcat(r_filename, ".r");
  // If the file already exists, we will resume the transfer
  if (filetime(f->fullname))
  {
    // Look for "filename.extension.r": this file contains information
    // about missing parts of the file and the MD5 hash (only exists
    // if some of the DATA blocks during the reception of the
    // first part have been invalid)

    if (filetime(r_filename))
    { // Ok, the index file exists: We will read it's contents and
      // check if the MD5 hash is still the same
      // TODO: Check, if the length of r_filename is valid!
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "found index file");
#endif
      REQc = (filesize(r_filename) - 33) / 8;
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "REQc: %d", REQc);
#endif
      REQ = (struct REQ_info *) t_malloc(sizeof(struct REQ_info)
                                         * ((REQc / 5) + 1) * 5, "dida");
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "reserved mem for %d REQ_infos",
                              ((REQc / 5) + 1) * 5);
#endif
      r_filehandle = s_open(r_filename, "srb");
      // Read the first 32 bytes of the file (the MD5 hex string)
      read(r_filehandle, r_filemd5str, 32);
      // Skip the following LF
      lseek(r_filehandle, 1L, SEEK_CUR);
      // Read the list of the parts which we are missing due to invalid
      // DATA frames during the reception of the first part
      read(r_filehandle, REQ, REQc * 8);
      s_close(r_filehandle);
    }
    else
    { // Nope, there's no index file containg information about gaps
      // in the part of the while we have already received
      // As we don't know the MD5 hash of the complete file we will
      // send the MD5 hash of the part already received to the sender
      // who will (hopefully) check if we are talking about the same file
      // TODO
    }
    f->size = resume_offset = filesize(f->fullname);
  }
  else
  {
    resume_offset = 0;
    //*r_filemd5str = 0;
#ifdef DEBUG_DIDADIT
    trace(serious, "debug", "no file to resume");
#endif
  }
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "\nr_filemd5str: %s\nmd5str: %s",
                          r_filemd5str, md5str);
#endif
  if (! REQ)
    REQ = (struct REQ_info *) t_malloc(sizeof(struct REQ_info) * 5, "dida");
  if (rxed_blocksize <= MAX_BLOCKSIZE) blocksize = rxed_blocksize;
  else blocksize = MAX_BLOCKSIZE;
  tx_START(resume_offset, r_filemd5str, blocksize);
  putflush();
  // TODO: Vor dem Überschreiben Warnung ausgeben...
  f->handle = s_open(f->fullname, "lr+b");
  if (resume_offset) lseek(f->handle, resume_offset, SEEK_SET);
  alloc_blockmem(MAX_BLOCKSIZE + 6); // TODO
//  alloc_blockmem(blocksize+6); // +4 bytes for the offset
                               // +2 bytes for the blocklength
  f->pos = resume_offset;
#ifdef DEBUG_DIDADIT
  trace(serious, "debug", "filesize: %d", f->size);
  trace(serious, "debug", "resuming at: %d", resume_offset);
#endif
  for (;;)
  {
    rx_block();
    waitfor(e_ticsfull);
#ifdef DEBUG_DIDADIT
    trace(serious, "debug", "didadit: type:%i", bl->type);
#endif
    if (bl->type == R_BLOCK_DATA)
    { // TODO: the memcpy()s only work on Intel-like architectures...
      memcpy((void *) &rxed_offset, bl->data, 4);
      memcpy((void *) &rxed_len, bl->data+4, 2);
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "didadit: type:%i, offs:%i, len:%i",
                              bl->type, rxed_offset, rxed_len);
#endif
      data_start = bl->data + 6;
    }
    else if (bl->type == R_BLOCK_FIN)
      // If we have received a FIN block, we set rxed_offset to one
      // byte after the last byte of the file - the next lines
      // will check, if the filepointer also points to this
      // position; if it doesn't, one or more blocks are missing at
      // the end and we have to send a REQ-block for them
      rxed_offset=f->size;
    else break;
    if (rxed_offset>f->pos)
    {
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "didadit: rxed_offset: %i > f->pos: %i",
                              rxed_offset, f->pos);
#endif
      // Grow the buffer for storing REQs by 5 units, if there ist not
      // sufficient space left
      if (REQc > 0 && ! (REQc % 5))
        REQ = (struct REQ_info *) t_realloc(REQ, sizeof(struct REQ_info)
                                                 * (REQc + 5));
      REQ[REQc].offset = f->pos;
      REQ[REQc].len = rxed_offset - f->pos;
      REQc++;
      do
        write(f->handle, "*", 1); // Fill the file with "*" (TODO: improve this)
      while (++f->pos < rxed_offset);
      update_r_file(r_filename, r_filemd5str, REQc, REQ);
    }
    else if (rxed_offset < f->pos)
    {
#ifdef DEBUG_DIDADIT
      trace(serious, "debug", "ignoring data: offset %d, len %d",
                              rxed_offset, bl->len-6);
#endif
    }
    if (bl->type == R_BLOCK_FIN) break;
    write(f->handle, data_start, bl->len-6);
    f->pos += bl->len - 6;
    waitfor(e_ticsfull);
  }
  switch (bl->type)
  {
  case R_BLOCK_FIN:
#ifdef DEBUG_DIDADIT
    trace(serious, "debug", "sending REQ");
#endif
    while (REQc) // As long as we don't have received everything correct
    {
      tx_REQ(REQ, REQc);
      putflush();
      for (;;)
      {
        rx_block();
        if (bl->type == R_BLOCK_FIN) break;
        // TODO handle invalid block type
        if (bl->type != R_BLOCK_DATA)
#ifdef DEBUG_DIDADIT // dh8ymb: correct? missing {} ????
          trace(serious, "debug", "unknown block");
#endif
        // TODO: the memcpy()s only work on Intel-like architectures...
        memcpy((void *) &rxed_offset, bl->data, 4);
        memcpy((void *) &rxed_len, bl->data+4, 2);
        data_start = bl->data + 6;
        // Find the block we just received in our REQ list
        found = 0;
        for (c = 0; c < REQc; c++)
          if (   (rxed_offset >= (long) REQ[c].offset)
              && ((rxed_offset + rxed_len) <= (long) (REQ[c].offset + REQ[c].len)))
          {
            found = 1;
#ifdef DEBUG_DIDADIT
            trace(serious, "debug", "found block at %d, len %d in REQs",
                                    rxed_offset, rxed_len);
            trace(serious, "debug", "REQ at %d, len %d",
                                    REQ[c].offset, REQ[c].len);
#endif
            break;
          }
        if (found)
        {
          lseek(f->handle, rxed_offset, SEEK_SET);
          write(f->handle, data_start, bl->len-6);
          // Look, if this DATA block includes all the bytes we requested
          // in a REQ block or it's just a part of one
          if (rxed_offset == (long) REQ[c].offset)
          {
            if (rxed_len == REQ[c].len)
            { // Fine - The received data block exactly fits to one entry in
              // our REQ list.
              // => We will delete this REQ entry from the list and decrement
              // the number of REQs.
#ifdef DEBUG_DIDADIT
              trace(serious, "debug", "situation 1");
#endif
              REQc--;
              memcpy(REQ+c, REQ + REQc, sizeof(struct REQ_info));
            }
            else
            { // Hmmm - The received data block fits to the beginning of an
              // entry in our REQ list; however, it does not cover ALL of the
              // data we are missing according to this REQ entry.
              // => We only shorten the length of REQ entry.
#ifdef DEBUG_DIDADIT
              trace(serious, "debug", "situation 2");
#endif
              REQ[c].offset += rxed_len;
              REQ[c].len -= rxed_len;
            }
          }
          else
          {
            if ((rxed_offset + rxed_len) == (long) (REQ[c].offset + REQ[c].len))
            { // Sigh - The received data block covers exactly the end
              // of an entry in our REQ list.
              // The handling of this situation is not that difficult
              // anyway: => We just have to decrease the length of the
              // REQ entry
#ifdef DEBUG_DIDADIT
              trace(serious, "debug", "situation 3");
#endif
              REQ[c].len -= rxed_len;
            }
            else
            { // Damn! - This situation is weird! The received data block
              // fits to an entry in our REQ list; however, it only supplies
              // data, which is located in the middle of the block we
              // are missing; the easy way would be to ignore the block,
              // but the DIDADIT protocol is intended to only transfer
              // data, which is really necessary and so we will go the
              // difficult way: => We modify the yet existing entry and
              // create a new one
#ifdef DEBUG_DIDADIT
              trace(serious, "debug", "situation 4");
#endif
              // Grow the buffer for storing REQs by 5 units, if there ist not
              // sufficient space left
              if (REQc > 0 && ! (REQc % 5))
                REQ = (struct REQ_info *) t_realloc(REQ, sizeof(struct REQ_info)
                                                         * (REQc + 5));
              // First, create the new entry:
              REQ[REQc].offset = rxed_offset + rxed_len;
              REQ[REQc].len = (REQ[c].offset+REQ[c].len) - REQ[REQc].offset;
              REQc++;
              // Now, modify the existing entry:
              REQ[c].len -= (rxed_len + REQ[REQc - 1].len);
            }
          }
          update_r_file(r_filename, r_filemd5str, REQc, REQ);
        }
        else
        { // OOPS: Block not found in our REQs - TODO: abort transfer
#ifdef DEBUG_DIDADIT
          trace(serious, "debug", "NOT found block at %d, len %d in REQs",
                                  rxed_offset, rxed_len);
#endif
        }
      }
    }
    s_close(f->handle);
    tx_FINACK();
    putflush();
    rxtime = ad_time() - startt;
    if (! rxtime) rxtime = 1L;
    bps = f->size / rxtime;
    putf("\n");
    putf(ms(m_rxok), "DIDADIT", bps << 3);
    putflush();
    setfiletime(f->fullname, getdostime(f->unixtime));
    break;
  default:
    s_close(f->handle);
    tx_ABORT();
    putf("*** Expected FIN- or DATA-frame, got type 0x%04X.\n", bl->type);
    break;
  }
  t_free(REQ);
}

/*---------------------------------------------------------------------------*/

void fileio_dida::_tx (void)
//*************************************************************************
//
// Senden von DIDADIT
//
//*************************************************************************
{
  char part_md5str[33];
  unsigned long int bps;
  time_t startt, txtime;
  unsigned long int resume_offset;
  unsigned short int rxed_blocksize;
  unsigned short int reqc; // Counter for REQ block
  char *reqp; // Pointer for crawling through the REQ block
  unsigned long int reqoffset, reqlen;
  char complete = 0;
  char binok[LINELEN+1];
  char next_cmd[LINELEN+1];
  int z, j = 0;
  unsigned i = 0;
  char befehl[LINELEN+1];
  binok[0] = 0;
  next_cmd[0] = 0;
//  *part_md5str = 0;

#ifdef DEBUG_DIDADIT
  trace(serious, "debug_tx", "full:%s,", f->fullname);
#endif
  if (! f->size)
  {
    putf(ms(m_filenobytes), f->fullname);
    return;
  }
  if ((f->ptr = s_fopen(f->fullname, "lrb")) == NULL)
  {
    putf(ms(m_filenotopen), f->fullname);
    return;
  }
  putf("\n#DIDADIT#\n");
  startt = ad_time();
  putflush();
  tx_INFO();
  putflush();
  do
  {
    getline(binok, LINELEN, 1);
    if (*binok && strncmp(binok, "#OK#", 4))
    {
      strcpy(next_cmd, binok);
      getline(binok, LINELEN, 1);
    }
  }
  while (! *binok); // ignore empty lines
  if (strncmp(binok, "#OK#", 4))
  {
    tx_ABORT();
    putflush();
    putf(ms(m_aborted));
    putflush();
    s_fclose(f->ptr);
    return;
  }
  alloc_blockmem(1000); // TODO: 1000 bytes max.?
  // We now expect START/ECHOREQ/ERR
  do
  {
    rx_block();
    switch (bl->type)
    {
      case R_BLOCK_START: break;
      case R_BLOCK_ECHOREQ:
        tx_ECHOREP();
        putflush();
        break;
      case R_BLOCK_ERR:
        putf("*** Transfer aborted by ERR-Frame\n");
        putflush();
        return;
      default:
        tx_ERR("100 Unexpected block type");
        putflush();
        putf("*** Expected START/ECHO-REQUEST/ERR, got type 0x%04X.\n",
             bl->type);
        putflush();
        s_fclose(f->ptr);
        return;
    }
  }
  while (bl->type != R_BLOCK_START);
  parse_START(&resume_offset, part_md5str, &rxed_blocksize);
  if (rxed_blocksize && rxed_blocksize < DEFAULT_BLOCKSIZE)
    blocksize = rxed_blocksize;
  else
    blocksize = DEFAULT_BLOCKSIZE;
  f->handle = s_open(f->fullname, "lrb");
  // TODO: If the partner supplied the PART_MD5-field, check it
  if (*part_md5str)
  {
    if (strlen(part_md5str) != 32)
    {
      tx_ERR("103 PART_MD5 format error");
      putflush();
      putf("*** PART_MD5 format error\n");
      putflush();
      s_close(f->handle);
      s_fclose(f->ptr);
      return;
    }
    if (resume_offset)
    {
#ifdef DEBUG_DIDADIT
      trace(serious, "debug_tx", "rxed partmd5: %s", part_md5str);
      trace(serious, "debug_tx", "our  partmd5: %s", f->md5str);
#endif
      if (strcmp(f->md5str, part_md5str))
      {
        tx_ERR("--- MD5 hashes are not the same");
        putflush();
        putf("*** MD5 hashes are not the same\n");
        putflush();
        s_close(f->handle);
        s_fclose(f->ptr);
        return;
      }
    }
    else
    { // Abort, if the partner supplied the PARTMD5-field without
      // an offset-field
      tx_ERR("--- PART_MD5 without OFFSET");
      putflush();
      putf("*** START frame: Received PART_MD5-field without OFFSET-field\n");
      putflush();
      s_close(f->handle);
      s_fclose(f->ptr);
      return;
    }
  }
  // Now send the whole file
  send_file_data(resume_offset, f->size-resume_offset, blocksize);
  putflush();
  tx_FIN("???");
  putflush();
  do
  {
    // Expect FINACK- or REQ-block
    alloc_blockmem(DEFAULT_BLOCKSIZE); // TODO: Max. 1000 Bytes?
    rx_block();
    switch (bl->type)
    {
    case R_BLOCK_FINACK:
      txtime = ad_time() - startt;
      if (! txtime) txtime = 1L;
      bps = f->size / txtime;
      putf("\n");
      putf(ms(m_txok), "DIDADIT", bps << 3);
      putflush();
      complete = 1;
      break;
    case R_BLOCK_REQ:
      if (bl->len % 8)
      {
        tx_ERR("--- Length of REQ block is not dividable by 8");
        putflush();
        putf("*** Length of REQ block is not dividable by 8.\n");
        putflush();
        complete = 1;
        break;
      }
      reqc = bl->len;
      reqp = bl->data;
      while (reqc)
      { // TODO: the memcpy()'s will only work on Intel-like architectures :-(
        memcpy((void *) &reqoffset, (void *) reqp, 4);
        memcpy((void *) &reqlen, (void *) (reqp + 4), 4);
        send_file_data(reqoffset, reqlen, blocksize);
        reqp += 8;
        reqc -= 8;
      }
      putflush();
      tx_FIN("???");
      putflush();
      break;
    case R_BLOCK_ECHOREQ:
      tx_ECHOREP();
      putflush();
      complete = 1;
      break;
    default:
      tx_ERR("100 Unexpected block type");
      putflush();
      putf("*** Expected FINACK/REQ/ECHO-REQUEST, got type 0x%04X.\n", bl->type);
      putflush();
      complete = 1;
      break;
    }
  }
  while (! complete);
  s_close(f->handle);
  s_fclose(f->ptr);
  putflush();
  if (*next_cmd)
  {
    for (i = 0 ; i <= strlen(next_cmd) ; i++)
    {
      if (next_cmd[i] == ';' || i == (strlen(next_cmd)))
      {
        befehl[i] = 0;
        mailbef(befehl, 0);
        j = i + 1;
        for (z = 0 ; z < LINELEN+1 ; z++)
          befehl[z] =  0;
      }
      else
        befehl[i-j] = next_cmd[i];
    }
  }
}

/*---------------------------------------------------------------------------*/
#endif
