/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------
  MD2/MD5 message digest algorithms   Reference: RFC 1319/1321
  ---------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990626 DH3MB  Heavily cleaned up the code - it has a quite nice
//                OOP-interface now

#include "baycom.h"

#ifdef FEATURE_MDPW

/*---------------------------------------------------------------------------*/

void MDx::update (char *str)
//*************************************************************************
//
//  Update method for 0-terminated strings (used by the MD2/MD5 password
//  functions)
//
//*************************************************************************
{
  update((char *) str, strlen(str));
}

/*---------------------------------------------------------------------------*/

void MDx::readfile (char *fname, off_t offset)
//*************************************************************************
//
//  Reads a file and pushes the content into update()
//
//*************************************************************************
{
  handle fd;
  unsigned short int num;
  char *buf;

  if ((fd = s_open(fname, "srb")) == EOF) return;
  buf = (char *) t_malloc(1024, "mdfi");
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

void MDx::getdigest (char *p)
//*************************************************************************
//
//  Copies the MD2/MD5 digest (binary form: 16 byte = 128 bit) to the
//  given pointer
//
//*************************************************************************
{
  if (! finalized) finalize();
  memcpy(p, digest, 16);
}

/*---------------------------------------------------------------------------*/

void MDx::gethexdigest (char *p)
//*************************************************************************
//
//  Writes the hexadecimal representation of the MD2/MD5 digest to
//  the given pointer
//
//*************************************************************************
{
  int i;

  if (! finalized) finalize();
  for (i = 0; i < 16; i++)
  {
    sprintf(p, "%02x", (char) digest[i]);
    p++;
    p++;
  }
}

/*---------------------------------------------------------------------------*/

// Code taken out of the appendix of RFC 1319, changes by DH3MB

/*---------------------------------------------------------------------------*/

// -*-*- Begin of RSA copyrighted code -*-*-

// Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD2/5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
//
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.

// -*-*- RSA Data Security, Inc., MD2 message-digest algorithm -*-*-

// Permutation of 0..255 constructed from the digits of pi. It gives a
// "random" nonlinear byte substitution operation.
static char PI_SUBST[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6, 19, 98,
  167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188, 76, 130, 202, 30,
  155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24, 138, 23, 229, 18, 190, 78,
  196, 214, 218, 158, 222, 73, 160, 251, 245, 142, 187, 47, 238, 122, 169,
  104, 121, 145, 21, 178, 7, 63, 148, 194, 16, 137, 11, 34, 95, 33, 128, 127,
  93, 154, 90, 144, 50, 39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48,
  179, 72, 165, 181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56,
  210, 150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157, 112,
  89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27, 96, 37, 173,
  174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15, 85, 71, 163, 35, 221,
  81, 175, 58, 195, 92, 249, 206, 186, 197, 234, 38, 44, 83, 13, 110, 133, 40,
  132, 9, 211, 223, 205, 244, 65, 129, 77, 82, 106, 220, 55, 200, 108, 193,
  171, 250, 36, 225, 123, 8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99,
  232, 109, 233, 203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88,
  208, 228, 166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

static char *PADDING2[] = {
  (char *)"",
  (char *)"\001",
  (char *)"\002\002",
  (char *)"\003\003\003",
  (char *)"\004\004\004\004",
  (char *)"\005\005\005\005\005",
  (char *)"\006\006\006\006\006\006",
  (char *)"\007\007\007\007\007\007\007",
  (char *)"\010\010\010\010\010\010\010\010",
  (char *)"\011\011\011\011\011\011\011\011\011",
  (char *)"\012\012\012\012\012\012\012\012\012\012",
  (char *)"\013\013\013\013\013\013\013\013\013\013\013",
  (char *)"\014\014\014\014\014\014\014\014\014\014\014\014",
  (char *)"\015\015\015\015\015\015\015\015\015\015\015\015\015",
  (char *)"\016\016\016\016\016\016\016\016\016\016\016\016\016\016",
  (char *)"\017\017\017\017\017\017\017\017\017\017\017\017\017\017\017",
  (char *)"\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020"
};

/*---------------------------------------------------------------------------*/

void MD2::init (void)
//*************************************************************************
//
//  MD2 initialization. Begins an MD2 operation, writing a new context.
//
//*************************************************************************
{
  context.count = 0;
  memset((char *) context.state, 0, sizeof(context.state));
  memset((char *) context.checksum, 0, sizeof(context.checksum));
}

/*---------------------------------------------------------------------------*/

void MD2::update (char *input, unsigned short int inputLen)
//*************************************************************************
//
//  MD2 block update operation. Continues an MD2 message-digest
//  operation, processing another message block, and updating the
//  context.
//
//*************************************************************************
{
  unsigned int i, indexu, partLen;

  if (finalized) init(); // Digest has already been finalized, start new digest
  // Update number of bytes mod 16
  indexu = context.count;
  context.count = (indexu + inputLen) & 0xf;
  partLen = 16 - indexu;
  // Transform as many times as possible.
  if (inputLen >= partLen)
  {
    memcpy((char *) (context.buffer + indexu), (char *) input, partLen);
    transform(context.buffer);
    for (i = partLen; i + 15 < inputLen; i += 16)
      transform(input + i);
    indexu = 0;
  }
  else i = 0;
  // Buffer remaining input
  memcpy((char *) (context.buffer + indexu), (char *) (input + i), inputLen - i);
}

/*---------------------------------------------------------------------------*/

void MD2::finalize (void)
//*************************************************************************
//
//  MD2 finalization. Ends an MD2 message-digest operation, writing the
//  message digest and zeroizing the context.
//
//*************************************************************************
{
  unsigned int indexu, padLen;
  // Pad out to multiple of 16.
  indexu = context.count;
  padLen = 16 - indexu;
  update(PADDING2[padLen], padLen);
  // Extend with checksum
  update(context.checksum, 16);
  // Store state in digest
  memcpy((char *) digest, (char *) context.state, 16);
  // Zeroize sensitive information.
  memset((char *) &context, 0, sizeof(MD2_CTX));
  finalized = 1;
}

/*---------------------------------------------------------------------------*/

void MD2::transform (char block[16])
//*************************************************************************
//
//  MD2 basic transformation. Transforms state and updates checksum
//  based on block.
//
//*************************************************************************
{
  unsigned int i, j, k;
  char x[48];

  // Form encryption block from state, block, state ^ block.
  memcpy((char *) x, (char *) context.state, 16);
  memcpy((char *) x + 16, (char *) block, 16);
  for (i = 0; i < 16; i++)
    x[i + 32] = context.state[i] ^ block[i];
  // Encrypt block (18 rounds).
  k = 0;
  for (i = 0; i < 18; i++)
  {
    for (j = 0; j < 48; j++)
      k = x[j] ^= PI_SUBST[k];
    k = (k + i) & 0xff;
  }
  // Save new state.
  memcpy((char *) context.state, (char *) x, 16);
  // Update checksum.
  k = context.checksum[15];
  for (i = 0; i < 16; i++)
    k = context.checksum[i] ^= PI_SUBST[block[i] ^ k];
  // Zeroize sensitive information.
  memset((char *) x, 0, sizeof(x));
}

// -*-*- End of RSA copyrighted code -*-*-

/*---------------------------------------------------------------------------*/

MD2::MD2 (void)
//*************************************************************************
//
//*************************************************************************
{
  finalized = 0;
  init();
}

/*---------------------------------------------------------------------------*/

// Code taken out of the appendix of RFC 1321, changes by DH3MB

/*---------------------------------------------------------------------------*/

// -*-*- Begin of RSA copyrighted code -*-*-

// Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD2/5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
//
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.

// -*-*- RSA Data Security, Inc., MD5 message-digest algorithm -*-*-

// Constants for Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static char PADDING5[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// F, G, H and I are basic MD5 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
 (a) = ROTATE_LEFT((a), (s)); \
 (a) += (b); \
 }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
 (a) = ROTATE_LEFT((a), (s)); \
 (a) += (b); \
 }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
 (a) = ROTATE_LEFT((a), (s)); \
 (a) += (b); \
 }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
 (a) = ROTATE_LEFT((a), (s)); \
 (a) += (b); \
 }

/*---------------------------------------------------------------------------*/

void MD5::init (void)
//*************************************************************************
//
//  MD5 initialization. Begins an MD5 operation, writing a new context.
//
//*************************************************************************
{
  context.count[0] = context.count[1] = 0;
  // Load magic initialization constants.
  context.state[0] = 0x67452301L;
  context.state[1] = 0xefcdab89L;
  context.state[2] = 0x98badcfeL;
  context.state[3] = 0x10325476L;
}

/*---------------------------------------------------------------------------*/

void MD5::update (char *input, unsigned short int inputLen)
//*************************************************************************
//
//  MD5 block update operation. Continues an MD5 message-digest
//  operation, processing another message block, and updating the
//  context.
//
//*************************************************************************
{
  unsigned int i, indexu, partLen;

  if (finalized) init(); // Digest has already been finalized, start new digest
  // Compute number of bytes mod 64
  indexu = (unsigned int) ((context.count[0] >> 3) & 0x3F);
  // Update number of bits
  if ((context.count[0] += ((unsigned long int) inputLen << 3)) <
      ((unsigned long int) inputLen << 3))
    context.count[1]++;
  context.count[1] += ((unsigned long int) inputLen >> 29);
  partLen = 64 - indexu;
  // Transform as many times as possible.
  if (inputLen >= partLen)
  {
    memcpy((char *) &context.buffer[indexu], (char *) input, partLen);
    transform(context.state, context.buffer);
    for (i = partLen; i + 63 < inputLen; i += 64)
      transform(context.state, &input[i]);
    indexu = 0;
  }
  else i = 0;
  // Buffer remaining input
  memcpy((char *) &context.buffer[indexu], (char *) &input[i], inputLen - i);
}

/*---------------------------------------------------------------------------*/

void MD5::finalize ()
//*************************************************************************
//
//  MD5 finalization. Ends an MD5 message-digest operation, writing the
//  the message digest and zeroizing the context.
//
//*************************************************************************
{
  char bits[8];
  unsigned int indexu, padLen;
  // Save number of bits
  encode(bits, context.count, 8);
  // Pad out to 56 mod 64.
  indexu = (unsigned int) ((context.count[0] >> 3) & 0x3f);
  padLen = (indexu < 56) ? (56 - indexu) : (120 - indexu);
  update(PADDING5, padLen);
  // Append length (before padding).
  update(bits, 8);
  // Store state in digest.
  encode((char *) digest, context.state, 16);
  // Zeroize sensitive information.
  memset((char *) &context, 0, sizeof(MD5_CTX));
  finalized = 1;
}

/*---------------------------------------------------------------------------*/

void MD5::transform (unsigned long int state[4], char block[64])
//*************************************************************************
//
//  MD5 basic transformation. Transforms state based on block.
//
//*************************************************************************
{
  unsigned long int a=state[0], g=state[1], c=state[2], d=state[3], x[16];

  decode(x, block, 64);

  // Round 1
  FF(a, g, c, d, x[ 0], S11, 0xd76aa478L); // 1
  FF(d, a, g, c, x[ 1], S12, 0xe8c7b756L); // 2
  FF(c, d, a, g, x[ 2], S13, 0x242070dbL); // 3
  FF(g, c, d, a, x[ 3], S14, 0xc1bdceeeL); // 4
  FF(a, g, c, d, x[ 4], S11, 0xf57c0fafL); // 5
  FF(d, a, g, c, x[ 5], S12, 0x4787c62aL); // 6
  FF(c, d, a, g, x[ 6], S13, 0xa8304613L); // 7
  FF(g, c, d, a, x[ 7], S14, 0xfd469501L); // 8
  FF(a, g, c, d, x[ 8], S11, 0x698098d8L); // 9
  FF(d, a, g, c, x[ 9], S12, 0x8b44f7afL); // 10
  FF(c, d, a, g, x[10], S13, 0xffff5bb1L); // 11
  FF(g, c, d, a, x[11], S14, 0x895cd7beL); // 12
  FF(a, g, c, d, x[12], S11, 0x6b901122L); // 13
  FF(d, a, g, c, x[13], S12, 0xfd987193L); // 14
  FF(c, d, a, g, x[14], S13, 0xa679438eL); // 15
  FF(g, c, d, a, x[15], S14, 0x49b40821L); // 16

  // Round 2
  GG(a, g, c, d, x[ 1], S21, 0xf61e2562L); // 17
  GG(d, a, g, c, x[ 6], S22, 0xc040b340L); // 18
  GG(c, d, a, g, x[11], S23, 0x265e5a51L); // 19
  GG(g, c, d, a, x[ 0], S24, 0xe9b6c7aaL); // 20
  GG(a, g, c, d, x[ 5], S21, 0xd62f105dL); // 21
  GG(d, a, g, c, x[10], S22,  0x2441453L); // 22
  GG(c, d, a, g, x[15], S23, 0xd8a1e681L); // 23
  GG(g, c, d, a, x[ 4], S24, 0xe7d3fbc8L); // 24
  GG(a, g, c, d, x[ 9], S21, 0x21e1cde6L); // 25
  GG(d, a, g, c, x[14], S22, 0xc33707d6L); // 26
  GG(c, d, a, g, x[ 3], S23, 0xf4d50d87L); // 27
  GG(g, c, d, a, x[ 8], S24, 0x455a14edL); // 28
  GG(a, g, c, d, x[13], S21, 0xa9e3e905L); // 29
  GG(d, a, g, c, x[ 2], S22, 0xfcefa3f8L); // 30
  GG(c, d, a, g, x[ 7], S23, 0x676f02d9L); // 31
  GG(g, c, d, a, x[12], S24, 0x8d2a4c8aL); // 32

  // Round 3
  HH(a, g, c, d, x[ 5], S31, 0xfffa3942L); // 33
  HH(d, a, g, c, x[ 8], S32, 0x8771f681L); // 34
  HH(c, d, a, g, x[11], S33, 0x6d9d6122L); // 35
  HH(g, c, d, a, x[14], S34, 0xfde5380cL); // 36
  HH(a, g, c, d, x[ 1], S31, 0xa4beea44L); // 37
  HH(d, a, g, c, x[ 4], S32, 0x4bdecfa9L); // 38
  HH(c, d, a, g, x[ 7], S33, 0xf6bb4b60L); // 39
  HH(g, c, d, a, x[10], S34, 0xbebfbc70L); // 40
  HH(a, g, c, d, x[13], S31, 0x289b7ec6L); // 41
  HH(d, a, g, c, x[ 0], S32, 0xeaa127faL); // 42
  HH(c, d, a, g, x[ 3], S33, 0xd4ef3085L); // 43
  HH(g, c, d, a, x[ 6], S34,  0x4881d05L); // 44
  HH(a, g, c, d, x[ 9], S31, 0xd9d4d039L); // 45
  HH(d, a, g, c, x[12], S32, 0xe6db99e5L); // 46
  HH(c, d, a, g, x[15], S33, 0x1fa27cf8L); // 47
  HH(g, c, d, a, x[ 2], S34, 0xc4ac5665L); // 48

  // Round 4
  II(a, g, c, d, x[ 0], S41, 0xf4292244L); // 49
  II(d, a, g, c, x[ 7], S42, 0x432aff97L); // 50
  II(c, d, a, g, x[14], S43, 0xab9423a7L); // 51
  II(g, c, d, a, x[ 5], S44, 0xfc93a039L); // 52
  II(a, g, c, d, x[12], S41, 0x655b59c3L); // 53
  II(d, a, g, c, x[ 3], S42, 0x8f0ccc92L); // 54
  II(c, d, a, g, x[10], S43, 0xffeff47dL); // 55
  II(g, c, d, a, x[ 1], S44, 0x85845dd1L); // 56
  II(a, g, c, d, x[ 8], S41, 0x6fa87e4fL); // 57
  II(d, a, g, c, x[15], S42, 0xfe2ce6e0L); // 58
  II(c, d, a, g, x[ 6], S43, 0xa3014314L); // 59
  II(g, c, d, a, x[13], S44, 0x4e0811a1L); // 60
  II(a, g, c, d, x[ 4], S41, 0xf7537e82L); // 61
  II(d, a, g, c, x[11], S42, 0xbd3af235L); // 62
  II(c, d, a, g, x[ 2], S43, 0x2ad7d2bbL); // 63
  II(g, c, d, a, x[ 9], S44, 0xeb86d391L); // 64

  state[0]+=a;
  state[1]+=g;
  state[2]+=c;
  state[3]+=d;

  // Zeroize sensitive information.
  memset((char *) x, 0, sizeof(x));
}

/*---------------------------------------------------------------------------*/

void MD5::encode (char *output, unsigned long int *input, unsigned int len)
//*************************************************************************
//
//  Encodes input (unsigned long int) into output (char).
//  Assumes len is a multiple of 4.
//
//*************************************************************************
{
  unsigned int i, j;
  for (i = 0, j = 0; j < len; i++,j += 4)
  {
    output[j]   = (char) (input[i]      & 0xff);
    output[j+1] = (char)((input[i] >>  8) & 0xff);
    output[j+2] = (char)((input[i] >> 16) & 0xff);
    output[j+3] = (char)((input[i] >> 24) & 0xff);
  }
}

/*---------------------------------------------------------------------------*/

void MD5::decode (unsigned long int *output, char *input, unsigned int len)
//*************************************************************************
//
//  Decodes input (char) into output (unsigned long int). Assumes
//  len is a multiple of 4.
//
//*************************************************************************
{
  unsigned int i, j;
  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((unsigned long int) input[j]) |
               (((unsigned long int) input[j + 1]) <<  8) |
               (((unsigned long int) input[j + 2]) << 16) |
               (((unsigned long int) input[j + 3]) << 24);
}

// -*-*- End of RSA copyrighted code -*-*-

/*---------------------------------------------------------------------------*/

MD5::MD5 (void)
//*************************************************************************
//
//*************************************************************************
{
  finalized = 0;
  init();
}

/*---------------------------------------------------------------------------*/
#endif
