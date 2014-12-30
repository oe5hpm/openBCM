/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------------
  LHarc Compression/Decompression
  -------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*----------------------------------------------------------------*/
/*      LHarc Encoding/Decoding module                            */
/*      This is part of LHarc UNIX Archiver Driver                */
/*                                                                */
/*  LZSS Algorithm                          Haruhiko.Okumura      */
/*  Adaptic Huffman Encoding    1989.05.27  Haruyasu.Yoshizaki    */
/*                                                                */
/*                                                                */
/*----------------------------------------------------------------*/

//19980129 OE3DZW modified header
//19990206 OE3DZW added code for external huffman-decoder
//19990615 DH3MB  adapted for new CRC classes
//20001210 DK2UI  unsigned cast in update() for DOS compiler work around

#include "baycom.h"

#ifdef _WIN32
#include <io.h>
#endif

#include "hufftab.h"

#ifndef __FLAT__
#pragma warn -sig
#endif

/*---------------------------------------------------------------------------*/

huffcod::huffcod (crcfbb *crc_class)
//*************************************************************************
//
//*************************************************************************
{ getbuf = 0;
  getlen = 0;
  putbuf = 0;
  putlen = 0;
  numchar = 0UL;

#ifdef __DOS16__
  while (sema_test("huffman")) wdelay(1327);
  sema_lock("huffman");
#endif
  crc_fbb = crc_class;
  text_buf = (char *) t_malloc((N4K + F_ - 1) * sizeof (char), "hu1");
  lson = (int *) t_malloc((N4K + 1) * sizeof (int), "hu2");
  rson = (int *) t_malloc((N4K + 1 + N4K) * sizeof (int), "hu3");
  dad  = (int *) t_malloc((N4K + 1) * sizeof (int), "hu4");
  same = (char *) t_malloc((N4K + 1) * sizeof (char), "hu5");
  freq = (unsigned *) t_malloc((T + 1) * sizeof (unsigned), "hu6");
  prnt = (int *) t_malloc((T + N_CHAR) * sizeof (int), "hu7");
  son  = (int *) t_malloc((T) * sizeof (int), "hu8");
  err  = ! (text_buf && lson && rson && dad && same && freq && prnt && son);
}

/*---------------------------------------------------------------------------*/

huffcod:: ~huffcod (void)
//*************************************************************************
//
//*************************************************************************
{
  t_free(text_buf);
  t_free(lson);
  t_free(rson);
  t_free(dad);
  t_free(same);
  t_free(freq);
  t_free(prnt);
  t_free(son);
#ifdef __DOS16__
  sema_unlock("huffman");
#endif
}

/*---------------------------------------------------------------------------*/

int huffcod::read_char ()
//*************************************************************************
//
// read byte from input
//
//*************************************************************************
{
  if (! (numchar++ % 256)) waitfor(e_ticsfull);
  return (getc(infile));
}

/*---------------------------------------------------------------------------*/

unsigned int huffcod::read_char1 ()
//*************************************************************************
//
// read byte from input, ignore EOF
//
//*************************************************************************
{
  unsigned int chr;

  if ((chr = getc(infile)) > 255) chr = 0;
  if (! (numchar++ % 256)) waitfor(e_ticsfull);
  return chr;
}

/*---------------------------------------------------------------------------*/

int huffcod::wri_char (int chr)
//*************************************************************************
//
// write byte to output
//
//*************************************************************************
{
  crc_fbb->update((char) chr);
  if (! (numchar++ % 256)) waitfor(e_ticsfull);
  return putc(chr, outfile);
}

/*---------------------------------------------------------------------------*/

void huffcod::InitTree ()
//*************************************************************************
//
// Initialize Tree
//
//*************************************************************************
{
  register int *p, *e;
  for (p = rson + N2K + 1, e = rson + N4K + N4K; p <= e;) *p++ = NILHUF;
  for (p = dad, e = dad + N4K; p < e;) *p++ = NILHUF;
}

/*---------------------------------------------------------------------------*/

void huffcod::InsertNode (register int r)
//*************************************************************************
//
// Insert to node
//
//*************************************************************************
{
  register int p;
  int cmp;
  register char *key;
  register unsigned int c;
  register unsigned int i, j;

  cmp = 1;
  key = &text_buf[r];
  i = key[1] ^ key[2];
  i ^= i >> 4;
  p = N2K + 1 + key[0] + ((i & 0x0f) << 8);
  rson[r] = lson[r] = NILHUF;
  match_length = 0;
  i = j = 1;
  for ( ; ; )
  {
    if (cmp >= 0)
    {
      if (rson[p] != NILHUF)
      {
        p = rson[p];
        j = same[p];
      }
      else
      {
        rson[p] = r;
        dad[r] = p;
        same[r] = i;
        return;
      }
    }
    else
    {
      if (lson[p] != NILHUF)
      {
        p = lson[p];
        j = same[p];
      }
      else
      {
        lson[p] = r;
        dad[r] = p;
        same[r] = i;
        return;
      }
    }
    if (i > j)
    {
      i = j;
      cmp = key[i] - text_buf[p + i];
    }
    else
      if (i == j)
      {
        for (; i < F_; i++)
          if ((cmp = key[i] - text_buf[p + i]) != 0) break;
      }
    if (i > THRESHOLD)
    {
      if (i > match_length)
      {
        match_position = ((r - p) & (N2K - 1)) - 1;
        if ((match_length = i) >= F_) break;
      }
      else
        if (i == match_length)
        {
          if ((c = ((r - p) & (N2K - 1)) - 1) < match_position)
            match_position = c;
        }
    }
  }
  same[r] = same[p];
  dad[r]  = dad[p];
  lson[r] = lson[p];
  rson[r] = rson[p];
  dad[lson[p]] = r;
  dad[rson[p]] = r;
  if (rson[dad[p]] == p) rson[dad[p]] = r;
  else lson[dad[p]] = r;
  dad[p] = NILHUF;  // remove p
}

/*---------------------------------------------------------------------------*/

void huffcod::link (int n, int p, int q)
//*************************************************************************
//
//*************************************************************************
{
  register char *s1, *s2, *s3;

  if (p >= NILHUF)
  {
    same[q] = 1;
    return;
  }
  s1 = text_buf + p + n;
  s2 = text_buf + q + n;
  s3 = text_buf + p + F_;
  while (s1 < s3)
  {
    if (*s1++ != *s2++)
    {
      same[q] = s1 - 1 - text_buf - p;
      return;
    }
  }
  same[q] = F_;
}

/*---------------------------------------------------------------------------*/

void huffcod::linknode (int p, int q, int r)
//*************************************************************************
//
//*************************************************************************
{
  int cmp;

  if (! (cmp = same[q] - same[r])) link(same[q], p, r);
  else if (cmp < 0) same[r] = same[q];
}

/*---------------------------------------------------------------------------*/

void huffcod::DeleteNode (register int p)
//*************************************************************************
//
//*************************************************************************
{
  register int q;

  if (dad[p] == NILHUF) return; // has no linked
  if (rson[p] == NILHUF)
  {
    if ((q = lson[p]) != NILHUF) linknode(dad[p], p, q);
  }
  else
    if (lson[p] == NILHUF)
    {
      q = rson[p];
      linknode(dad[p], p, q);
    }
    else
    {
      q = lson[p];
      if (rson[q] != NILHUF)
      {
        do
          q = rson[q];
        while (rson[q] != NILHUF);
        if (lson[q] != NILHUF) linknode(dad[q], q, lson[q]);
        link(1, q, lson[p]);
        rson[dad[q]] = lson[q];
        dad[lson[q]] = dad[q];
        lson[q] = lson[p];
        dad[lson[p]] = q;
      }
      link(1, dad[p], q);
      link(1, q, rson[p]);
      rson[q] = rson[p];
      dad[rson[p]] = q;
    }
  dad[q] = dad[p];
  if (rson[dad[p]] == p) rson[dad[p]] = q;
  else lson[dad[p]] = q;
  dad[p] = NILHUF;
}

/*---------------------------------------------------------------------------*/

int huffcod::GetBit ()
//*************************************************************************
//
// get one bit
// returning in Bit 0
//
//*************************************************************************
{
  register unsigned int dx = getbuf;
  register unsigned int c;

  if (getlen <= 8)
  {
    c = read_char1();
    dx |= c << (8 - getlen);
    getlen += 8;
  }
  getbuf = dx << 1;
  getlen--;
  return (dx & 0x8000) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/

int huffcod::GetByte ()
//*************************************************************************
//
// get one byte
// returning in Bit7...0
//
//*************************************************************************
{
  register unsigned int dx = getbuf;
  register unsigned c;

  if (getlen <= 8)
  {
    c = read_char1();
    dx |= c << (8 - getlen);
    getlen += 8;
  }
  getbuf = dx << 8;
  getlen -= 8;
  return (dx >> 8) & 0xff;
}

/*---------------------------------------------------------------------------*/

int huffcod::GetNBits (register unsigned int n)
//*************************************************************************
//
// get N bit
// returning in Bit(N-1)...Bit 0
//
//*************************************************************************
{
  register unsigned int dx = getbuf;
  register unsigned int c;

  static int mask[17] =
  {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f,
    0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff,
    0x1fff, 0x3fff, 0x0fff, 0xffff
  };

  static int shift[17] =
  {
    16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
  };

  if (getlen <= 8)
  {
    c = read_char1();
    dx |= c << (8 - getlen);
    getlen += 8;
  }
  getbuf = dx << n;
  getlen -= n;
  return (dx >> shift[n]) & mask[n];
}

/*---------------------------------------------------------------------------*/

int huffcod::Putcode (register int l, register unsigned int c)
//*************************************************************************
//
// Output C bits
//
//*************************************************************************
{
  register int len = putlen;
  register unsigned int pb = putbuf;

  pb |= c >> len;
  if ((len += l) >= 8)
  {
    if (wri_char(pb >> 8) == EOF) return 1;
    if ((len -= 8) >= 8)
    {
      if (wri_char(pb) == EOF) return 1;
      codesize += 2;
      len -= 8;
      pb = c << (l - len);
    }
    else
    {
      pb <<= 8;
      codesize++;
    }
  }
  putbuf = pb;
  putlen = len;
  return 0;
}

/*---------------------------------------------------------------------------*/

void huffcod::StartHuff ()
//*************************************************************************
//
// Initialize tree
//
//*************************************************************************
{
  register unsigned int i, j;

  for (i = 0; i < N_CHAR; i++)
  {
    freq[i] = 1;
    son[i] = i + T;
    prnt[i + T] = i;
  }
  i = 0; j = N_CHAR;
  while (j <= R)
  {
    freq[j] = freq[i] + freq[i + 1];
    son[j] = i;
    prnt[i] = prnt[i + 1] = j;
    i += 2; j++;
  }
  freq[T] = 0xffff;
  prnt[R] = 0;
  putlen = getlen = 0;
  putbuf = getbuf = 0;
}

/*---------------------------------------------------------------------------*/

void huffcod::reconst ()
//*************************************************************************
//
// reconstruct tree
//
//*************************************************************************
{
  register int i, j, k;
  register unsigned f;

  // correct leaf node into of first half,
  // and set these frequency to (freq+1)/2
  j = 0;
  for (i = 0; i < T; i++)
    if (son[i] >= T)
    {
      freq[j] = (freq[i] + 1) >> 1;
      son[j] = son[i];
      j++;
    }
  // build tree. Link sons first
  for (i = 0, j = N_CHAR; j < T; i += 2, j++)
  {
    k = i + 1;
    f = freq[j] = freq[i] + freq[k];
    for (k = j - 1; f < freq[k]; k--);
    k++;
    {
      register unsigned *p, *e;
      for (p = &freq[j], e = &freq[k]; p > e; p--) p[0] = p[-1];
      freq[k] = f;
    }
    {
      register int *p, *e;
      for (p = &son[j], e = &son[k]; p > e; p--) p[0] = p[-1];
      son[k] = i;
    }
  }
  // link parents
  for (i = 0; i < T; i++)
  {
    if ((k = son[i]) >= T) prnt[k] = i;
    else prnt[k] = prnt[k + 1] = i;
  }
}

/*---------------------------------------------------------------------------*/

void huffcod::update (unsigned c)
//*************************************************************************
//
// update given code's frequency, and update tree
//
//*************************************************************************
{
  register unsigned *p, k;
  register int i, j, l;

  if (freq[R] == MAX_FREQ) reconst();
  c = prnt[c + T];
  do
  {
    k = ++freq[c];
    // swap nodes when become wrong frequency order.
    if (k > freq[l = c + 1])
    {
      for (p = freq + l + 1; k > *p++; );
      l = p - freq - 2;
      freq[c] = p[-2];
      p[-2] = k;
      i = son[c];
      prnt[i] = l;
      if (i < T) prnt[i + 1] = l;
      j = son[l];
      son[l] = i;
      prnt[j] = c;
      if (j < T) prnt[j + 1] = c;
      son[c] = j;
      c = (unsigned) l;
    }
  }
  while ((c = prnt[c]) != 0); // loop until reach to root
}

/*---------------------------------------------------------------------------*/

unsigned int huffcod::EncodeChar (unsigned c)
//*************************************************************************
//
// static unsigned code, len;
//
//*************************************************************************
{
  register int *p;
  register unsigned long i;
  register int j, k;

  i = 0;
  j = 0;
  p = prnt;
  k = p[c + T];
  // trace links from leaf node to root
  do
  {
    i >>= 1;
    // if node index is odd, trace larger of sons
    if (k & 1) i += 0x80000000L;
    j++;
  }
  while ((k = p[k]) != R);
  if (j > 16)
  {
    if (Putcode(16, (unsigned int) (i >> 16))) return (1);
    if (Putcode(j - 16, (unsigned int) i)) return (1);
  }
  else
    if (Putcode(j, (unsigned int) (i >> 16))) return (1);
//  code = i;
//  len = j;
  update(c);
  return (0);
}

/*---------------------------------------------------------------------------*/

int huffcod::EncodePosition (unsigned c)
//*************************************************************************
//
//*************************************************************************
{
  unsigned i;

  // output upper 6bit from table
  i = c >> 6;
  if (Putcode((int) (p_len[i]), (unsigned int) (p_code[i]) << 8)) return (1);
  // output lower 6 bit
  if (Putcode(6, (unsigned int) (c & 0x3f) << 10)) return (1);
  return (0);
}

/*---------------------------------------------------------------------------*/

int huffcod::EncodeEnd ()
//*************************************************************************
//
//*************************************************************************
{
  if (putlen)
  {
    if (wri_char(putbuf >> 8) == EOF) return 1;
    codesize++;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int huffcod::DecodeChar ()
//*************************************************************************
//
//*************************************************************************
{
  register unsigned c;

  c = son[R];
  // trace from root to leaf,
  // got bit is 0 to small(son[]), 1 to large (son[]+1) son node
  while (c < T)
  {
    c += GetBit();
    c = son[c];
  }
  c -= T;
  update(c);
  return c;
}

/*---------------------------------------------------------------------------*/

int huffcod::DecodePosition ()
//*************************************************************************
//
//*************************************************************************
{
  unsigned i, j, c;

  // decode upper 6bit from table
  i = GetByte();
  c = (unsigned) d_code[i] << 6;
  j = d_len[i];
  // get lower 6bit
  j -= 2;
  return c | (((i << j) | GetNBits(j)) & 0x3f);
}

/*---------------------------------------------------------------------------*/

unsigned int huffcod::Encode ()
//*************************************************************************
//
// Kodiert nach "huffman"
//
//*************************************************************************
{
  register int i, c, r, s, last_match_length;
  unsigned len;

  textsize = 0;
  StartHuff();
  InitTree();
  s = 0;
  r = N4K - F_; // 4096 - 60 = 4036
  for (i = s; i < r; i++) text_buf[i] = ' ';
  r = N2K - F_; // 2048 - 60 = 1988 this is important!
  for (len = 0; len < F_ && (c = read_char()) != EOF; len++)
    text_buf[r + len] = (char) c;
  textsize = len;
  for (i = 1; i <= F_; i++)
    InsertNode(r - i);
  InsertNode(r);
  do
  {
    if (match_length > len) match_length = len;
    if (match_length <= THRESHOLD)
    {
      match_length = 1;
      if (EncodeChar((unsigned) text_buf[r])) return 1;
    }
    else
    {
      if (EncodeChar(255 - THRESHOLD + match_length)) return 1;
      if (EncodePosition(match_position)) return 1;
    }
    last_match_length = match_length;
    for (i = 0; i < last_match_length && (c = read_char()) != EOF; i++)
    {
      DeleteNode(s);
      text_buf[s] = c;
      if (s < F_ - 1) text_buf[s + N2K] = c;
      s = (s + 1) & (N2K - 1);
      r = (r + 1) & (N2K - 1);
      InsertNode(r);
    }
    textsize += i;
    while (i++ < last_match_length)
    {
      DeleteNode(s);
      s = (s + 1) & (N2K - 1);
      r = (r + 1) & (N2K - 1);
      if (--len) InsertNode(r);
    }
  }
  while (len > 0);
  if (EncodeEnd()) return 1;
  return 0;
}

/*---------------------------------------------------------------------------*/

int huffcod::Decode (unsigned long int txtsize)
//*************************************************************************
//
// Dekodiert nach "huffman"
//
//*************************************************************************
{
  register unsigned int i, j, k, r, c;
  register unsigned long int count;

  StartHuff();
  r = N4K - F_;
  for (i = 0; i < r; i++) text_buf[i] = ' ';
  for (count = 0; count < txtsize;)
  {
    c = DecodeChar();
    if (c < 256)
    {
      if (wri_char(c) == EOF) return 1;
      text_buf[r++] = c;
      r &= (N4K - 1);
      count++;
    }
    else
    {
      i = (r - DecodePosition() - 1) & (N4K - 1);
      j = c - 255 + THRESHOLD;
      for (k = 0; k < j; k++)
      {
        c = text_buf[(i + k) & (N4K - 1)];
        if (wri_char(c) == EOF) return 1;
        text_buf[r++] = c;
        r &= (N4K - 1);
        count++;
      }
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void huffcod::init_huf ()
//*************************************************************************
//
// "huffman" Initialisierung
//
//*************************************************************************
{
  textsize = 0;
  codesize = 0;
  getbuf = 0;
  getlen = 0;
  putbuf = 0;
  putlen = 0;
}

/*---------------------------------------------------------------------------*/

unsigned int huffcod::encode (char *inputfile, char *outputfile)
//*************************************************************************
//
// Kodiert Datei nach "huffman"
// Rueckgabe:  0 = ok
//             1 = Error
//
//*************************************************************************
{
  int error = 0;
  char i;

  init_huf();
  if (! (infile = s_fopen(inputfile, "srb")))
    return 1;
  if (! (outfile = s_fopen(outputfile, "swb")))
  {
    s_fclose(infile);
    return 1;
  }
  s_fsetopt(infile, 1); //delete input file (tmp-file)
  s_fsetopt(outfile, 1);
  fseek(infile, 0L, SEEK_END);
  textsize = ftell(infile);
  // Store size of the original file...
  if (fwrite(&textsize, sizeof(textsize), 1, outfile) < 1) error = 1;
  else // ...and don't forget to calculate the CRCfbb
    for (i = 0; i < sizeof(textsize); i++)
      crc_fbb->update(*((char *) & textsize + i));
  if (! error && ! textsize) error = 1;
  if (! error)
  {
    rewind(infile);
    if (Encode()) error = 1;
  }
  s_fclose(infile);
  s_fclose(outfile);
  return error;
}

/*---------------------------------------------------------------------------*/

unsigned int huffcod::decode (char *inputfile, char *outputfile,
                              unsigned long inputoffset)
//*************************************************************************
//
// Dekodiert Datei nach "huffman"
// Rueckgabe:  0 = ok
//             1 = Error
//
//*************************************************************************
{
  int error = 0;
  init_huf();
  if (! (infile = s_fopen(inputfile, "srb")))
    return 1;
  fseek(infile, inputoffset, SEEK_SET);
  if (! (outfile = s_fopen(outputfile, "swb")))
  {
    s_fclose(infile);
    return 1;
  }
  s_fsetopt(outfile,1);
  if (! (numchar++ % 256)) waitfor(e_ticsfull);
  // Get the size of the uncompressed file
  if (fread (&textsize, sizeof(textsize), 1, infile) < 1) error = 1;
  if (! error && (! textsize || textsize > MAXCOMPFILESIZE)) error = 1;
  if (! error && Decode(textsize)) error = 1;
  s_fclose(infile);
  s_fclose(outfile);
  return error;
}


#ifndef __FLAT__
#pragma sig warn +sig
#endif
