/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------
  Berechnung von CRC-Pruefsummen
  ------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990407 DH3MB  CRCthp + CRCfbb: Changed unsigned long int to
//                unsigned short int
//19990615 DH3MB  Cleaned up the complete code, added CRC32 methods,
//                made classes for every single CRC type,
//                added #ifdef for all CRC types,
//                moved file_crcthp to util.cpp
//19990626 DH3MB  Added methods for updating the CRC using data blocks, rather
//                than just passing one single byte every time
//19990703 DH3MB  Added methods for updating the CRC with a string

#include "baycom.h"

/*---------------------------------------------------------------------------*/

// Include support for the various CRC types depending on what
// we actually need for the requested features

#define _CRC_THP // We need this for AutoBIN

#define _CRC_FBB // Only needed for FBB style forwarding

#ifdef FEATURE_BINSPLIT
  #define _CRC_32 // Only needed when BIN split support is desired
#endif // FEATURE_BINSPLIT

#include "crctab.h" // This file must be included _here_

/*---------------------------------------------------------------------------*/

void crc::update (char *block, unsigned short int len)
//*************************************************************************
//
// CRC base class
//
//*************************************************************************
{
  unsigned short int i = 0;
  while (len--) update (block[i++]);
}

void crc::readfile (char *fname, off_t offset)
//*************************************************************************
//
//  Reads a file and pushes the content into update()
//
//*************************************************************************
{
  handle fd;
  unsigned short int num;
  char *buf;

  if ((fd = s_open(fname, "srb")) == -1) return;
  buf = (char *) t_malloc(1024, "crcf");
  lseek(fd, offset, SEEK_SET);
  while ((num = _read(fd, buf, 1024)) > 0)
  {
    update(buf, num);
    waitfor(e_ticsfull);
  }
  t_free(buf);
  s_close(fd);
}

/*---------------------------------------------------------------------------*/

#ifdef _CRC_THP

crcthp::crcthp (void)
//*************************************************************************
//
//  Constructor: Just initialize the CRC
//
//*************************************************************************
{
  result = (unsigned short int) 0x0000;
}

/*---------------------------------------------------------------------------*/

void crcthp::update (char ch)
//*************************************************************************
//
//  Update the CRC-THP (used for AutoBin, 7plus line CRC)
//
//*************************************************************************
{
  result = ccitt_crctab[result >> 8] ^ ((result << 8) | ch);
}

#endif // _CRC_THP

/*---------------------------------------------------------------------------*/

#ifdef _CRC_FBB

crcfbb::crcfbb (void)
//*************************************************************************
//
//  Constructor: Just initialize the CRC
//
//*************************************************************************
{
  result = (unsigned short int) 0x0000;
}

/*---------------------------------------------------------------------------*/

void crcfbb::update (char ch)
//*************************************************************************
//
//  Update the CRC-FBB (used for FBB-type forwarding)
//
//*************************************************************************
{
  result = ((result) << 8) ^ ccitt_crctab[((result) >> 8) ^ ch];
}

#endif // _CRC_FBB

/*---------------------------------------------------------------------------*/

#ifdef _CRC_FCS

crcfcs::crcfcs(void)
//*************************************************************************
//
//  Constructor: Just initialize the CRC
//
//*************************************************************************
{
  result = (unsigned short int) 0x0000;
}

/*---------------------------------------------------------------------------*/

void crcfcs::update (char ch)
//*************************************************************************
//
//  Update the CRC-FCS
//
//*************************************************************************
{
  result = (result >> 8) ^ ccitt_crctab[(result ^ ch) & 0xff];
}

#endif // _CRC_FCS

/*---------------------------------------------------------------------------*/

#ifdef _CRC_16

crc16::crc16 (void)
//*************************************************************************
//
//  Constructor: Just initialize the CRC
//
//*************************************************************************
{
  result = (unsigned short int) 0x0000;
}

/*---------------------------------------------------------------------------*/

void crc16::update (char ch)
//*************************************************************************
//
//  Update the CRC-16
//
//*************************************************************************
{
  result = (result >> 8) ^ crc16_table[(result ^ ch) & 0xff];
}

#endif // _CRC_16

/*---------------------------------------------------------------------------*/

#ifdef _CRC_32

//
// CRC32 functions derived from article Copyright (C) 1986 Stephen Satchell.
//
// Programmers may incorporate any or all code into their programs, giving
// proper credit within the source. Publication of the source routines is
// permitted so long as proper credit is given to Stephen Satchell, Satchell
// Evaluations and Chuck Forsberg, Omen Technology.
//

crc32::crc32 (void)
//*************************************************************************
//
//  Constructor: Just initialize the CRC
//
//*************************************************************************
{
  result = (unsigned long int) 0xffffffffL;
}

/*---------------------------------------------------------------------------*/

void crc32::update (char ch)
//*************************************************************************
//
//  Update the CRC-32 (used for BIN splitting)
//
//*************************************************************************
{
  result = crc32tab[((int)(result) ^ (char)(ch)) & 0xff] ^ ((result) >> 8);
}

#endif // _CRC_32

/*---------------------------------------------------------------------------*/
