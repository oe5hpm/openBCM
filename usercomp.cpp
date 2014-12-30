/*
 *  usercomp.cpp - procedures for (de)compression with a static huffman-table
 *  Adapted from LinKT
 *
 *  Copyright (C) 1999 Johann Hanne, DH3MB. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Pascal2C-Translation of the #HUF#-Compression by DF8MT (Andy).
 * (*not* by p2c)
 *
 * Translated by Jochen Sarrazin, DG6VJ
 *
 * Original Pascal-Code is Copyright by Andy, DF8MT
 * This C-Translation is Copyright by Jochen Sarrazin, DG6VJ (1999)
 */

#include <stdlib.h>
#include <string.h>
#include "usercomp.h"

unsigned char DNotCAnz;
unsigned char DNotComp;
unsigned char Dml;
unsigned short DCount = 0;
long Dm = 0;
unsigned char Cml;
unsigned short CCount = 0;
long Cm = 0;



//#ifdef _USERCOMP

// Huffman-Tabelle
static struct TableTyp_struct HTable[] =
{
   {0xab2c,15},{0xaa84,15},{0x9fc4,15},{0xab3c,15},
   {0xab1c,15},{0xaafc,15},{0xaaec,15},{0xaad4,15},
   {0xaab4,15},{0xf340,10},{0xaaa4,15},{0x7d64,15},
   {0xaadc,15},{0xf400, 7},{0xaa94,15},{0x9ff4,15},
   {0x9fd4,15},{0x7d74,15},{0xab44,15},{0xab34,15},
   {0xab24,15},{0xab14,15},{0xab04,15},{0xaaf4,15},
   {0xaae4,15},{0xab60,14},{0xab0c,15},{0xaacc,15},
   {0xaabc,15},{0xaaac,15},{0xaa9c,15},{0xaa8c,15},
   {0xc000, 3},{0x3a80, 9},{0xabc0,10},{0x0060,11},
   {0x7d40,12},{0xab5c,14},{0x0000,12},{0xab58,14},
   {0x7c00, 9},{0x3c80, 9},{0x7d00,11},{0x0010,12},
   {0x1200, 7},{0x7a00, 7},{0xb800, 6},{0x3200, 7},
   {0x2200, 7},{0xf600, 8},{0x3d00, 8},{0x9e00, 9},
   {0xbd80, 9},{0x7c80, 9},{0x0080, 9},{0xaa00, 9},
   {0xbd00, 9},{0x9f00, 9},{0x0300, 8},{0xab78,13},
   {0xab68,13},{0x3c00, 9},{0x3000, 9},{0x0020,11},
   {0x7d50,12},{0x3800, 7},{0x7800, 7},{0x9c00, 7},
   {0xfe00, 7},{0x2400, 6},{0xbc00, 8},{0x0200, 8},
   {0x0100, 8},{0xf100, 8},{0x0040,11},{0x3100, 8},
   {0xf200, 8},{0x3400, 7},{0x1c00, 7},{0x1e00, 7},
   {0xbe00, 7},{0xaba0,11},{0x3e00, 7},{0x1400, 6},
   {0x3600, 7},{0xf380, 9},{0xf080, 9},{0x2000, 8},
   {0xfc00, 8},{0x9f80,10},{0x9e80, 9},{0xab90,12},
   {0x3b80, 9},{0xab80,12},{0xab54,14},{0x3a50,13},
   {0xab50,14},{0xa000, 5},{0x1800, 6},{0x9800, 6},
   {0x7000, 5},{0x4000, 3},{0x0400, 6},{0xac00, 6},
   {0xf800, 6},{0x6000, 4},{0x3a00,10},{0xfd00, 8},
   {0x2800, 5},{0xb000, 6},{0x8000, 4},{0xb400, 6},
   {0x1000, 7},{0x7d20,12},{0xe000, 5},{0x9000, 5},
   {0xe800, 5},{0x0800, 5},{0xf700, 8},{0xa800, 7},
   {0x7d80, 9},{0xf300,10},{0x7e00, 7},{0xab48,14},
   {0x3a48,13},{0xab4c,14},{0x3a60,12},{0x9ffc,15},
   {0x9fec,15},{0x2100, 8},{0x9fdc,15},{0x9fcc,15},
   {0xf000, 9},{0x7d7c,15},{0x7d6c,15},{0x3a40,14},
   {0xab40,15},{0xab38,15},{0xab30,15},{0xab28,15},
   {0xab20,15},{0xab18,15},{0xab70,13},{0xab10,15},
   {0xab08,15},{0xab00,15},{0xaaf8,15},{0xaaf0,15},
   {0x3b00, 9},{0xaae8,15},{0xaae0,15},{0xaad8,15},
   {0xaad0,15},{0xab64,14},{0x7d30,12},{0xaac8,15},
   {0xaac0,15},{0xaab8,15},{0xaab0,15},{0xaaa8,15},
   {0xaaa0,15},{0xaa98,15},{0xaa90,15},{0xaa88,15},
   {0xaa80,15},{0x9ff8,15},{0x9ff0,15},{0x9fe8,15},
   {0x9fe0,15},{0x9fd8,15},{0x9fd0,15},{0x9fc8,15},
   {0x9fc0,15},{0x7d78,15},{0x7d70,15},{0x3a58,13},
   {0x7d68,15},{0x7d60,15},{0xab46,15},{0xab42,15},
   {0xab3e,15},{0xab3a,15},{0xab36,15},{0xab32,15},
   {0xab2e,15},{0xab2a,15},{0xab26,15},{0xab22,15},
   {0xab1e,15},{0xab1a,15},{0xab16,15},{0xab12,15},
   {0xab0e,15},{0xab0a,15},{0xab06,15},{0xab02,15},
   {0xaafe,15},{0xaafa,15},{0xaaf6,15},{0xaaf2,15},
   {0xaaee,15},{0xaaea,15},{0xaae6,15},{0xaae2,15},
   {0xaade,15},{0xaada,15},{0xaad6,15},{0xaad2,15},
   {0xaace,15},{0xaaca,15},{0xaac6,15},{0xaac2,15},
   {0xaabe,15},{0xaaba,15},{0xaab6,15},{0xaab2,15},
   {0xaaae,15},{0xaaaa,15},{0xaaa6,15},{0xaaa2,15},
   {0xaa9e,15},{0x3a70,12},{0xaa9a,15},{0xaa96,15},
   {0xaa92,15},{0x3080, 9},{0xaa8e,15},{0xaa8a,15},
   {0xaa86,15},{0xaa82,15},{0x9ffe,15},{0x9ffa,15},
   {0x9ff6,15},{0x9ff2,15},{0x9fee,15},{0x9fea,15},
   {0x9fe6,15},{0x9fe2,15},{0x9fde,15},{0x9fda,15},
   {0x9fd6,15},{0x9fd2,15},{0x9fce,15},{0x9fca,15},
   {0x9fc6,15},{0x9fc2,15},{0x7d7e,15},{0x7d7a,15},
   {0x7d76,15},{0x7d72,15},{0x7d6e,15},{0x7d6a,15},
   {0x7d66,15},{0x7d62,15},{0x3a46,15},{0x3a44,15},
   {0xAAC4,15},{0x9FE4,15}
};

static struct huff_comp_table_struct huff_comp_table[] = {
/* Begin of automatically built huffman compression table */
{0xab2c,15},{0xaa84,15},{0x9fc4,15},{0xab3c,15},
{0xab1c,15},{0xaafc,15},{0xaaec,15},{0xaad4,15},
{0xaab4,15},{0xf340,10},{0xaaa4,15},{0x7d64,15},
{0xaadc,15},{0xf400, 7},{0xaa94,15},{0x9ff4,15},
{0x9fd4,15},{0x7d74,15},{0xab44,15},{0xab34,15},
{0xab24,15},{0xab14,15},{0xab04,15},{0xaaf4,15},
{0xaae4,15},{0xab60,14},{0xab0c,15},{0xaacc,15},
{0xaabc,15},{0xaaac,15},{0xaa9c,15},{0xaa8c,15},
{0xc000, 3},{0x3a80, 9},{0xabc0,10},{0x0060,11},
{0x7d40,12},{0xab5c,14},{0x0000,12},{0xab58,14},
{0x7c00, 9},{0x3c80, 9},{0x7d00,11},{0x0010,12},
{0x1200, 7},{0x7a00, 7},{0xb800, 6},{0x3200, 7},
{0x2200, 7},{0xf600, 8},{0x3d00, 8},{0x9e00, 9},
{0xbd80, 9},{0x7c80, 9},{0x0080, 9},{0xaa00, 9},
{0xbd00, 9},{0x9f00, 9},{0x0300, 8},{0xab78,13},
{0xab68,13},{0x3c00, 9},{0x3000, 9},{0x0020,11},
{0x7d50,12},{0x3800, 7},{0x7800, 7},{0x9c00, 7},
{0xfe00, 7},{0x2400, 6},{0xbc00, 8},{0x0200, 8},
{0x0100, 8},{0xf100, 8},{0x0040,11},{0x3100, 8},
{0xf200, 8},{0x3400, 7},{0x1c00, 7},{0x1e00, 7},
{0xbe00, 7},{0xaba0,11},{0x3e00, 7},{0x1400, 6},
{0x3600, 7},{0xf380, 9},{0xf080, 9},{0x2000, 8},
{0xfc00, 8},{0x9f80,10},{0x9e80, 9},{0xab90,12},
{0x3b80, 9},{0xab80,12},{0xab54,14},{0x3a50,13},
{0xab50,14},{0xa000, 5},{0x1800, 6},{0x9800, 6},
{0x7000, 5},{0x4000, 3},{0x0400, 6},{0xac00, 6},
{0xf800, 6},{0x6000, 4},{0x3a00,10},{0xfd00, 8},
{0x2800, 5},{0xb000, 6},{0x8000, 4},{0xb400, 6},
{0x1000, 7},{0x7d20,12},{0xe000, 5},{0x9000, 5},
{0xe800, 5},{0x0800, 5},{0xf700, 8},{0xa800, 7},
{0x7d80, 9},{0xf300,10},{0x7e00, 7},{0xab48,14},
{0x3a48,13},{0xab4c,14},{0x3a60,12},{0x9ffc,15},
{0x9fec,15},{0x2100, 8},{0x9fdc,15},{0x9fcc,15},
{0xf000, 9},{0x7d7c,15},{0x7d6c,15},{0x3a40,14},
{0xab40,15},{0xab38,15},{0xab30,15},{0xab28,15},
{0xab20,15},{0xab18,15},{0xab70,13},{0xab10,15},
{0xab08,15},{0xab00,15},{0xaaf8,15},{0xaaf0,15},
{0x3b00, 9},{0xaae8,15},{0xaae0,15},{0xaad8,15},
{0xaad0,15},{0xab64,14},{0x7d30,12},{0xaac8,15},
{0xaac0,15},{0xaab8,15},{0xaab0,15},{0xaaa8,15},
{0xaaa0,15},{0xaa98,15},{0xaa90,15},{0xaa88,15},
{0xaa80,15},{0x9ff8,15},{0x9ff0,15},{0x9fe8,15},
{0x9fe0,15},{0x9fd8,15},{0x9fd0,15},{0x9fc8,15},
{0x9fc0,15},{0x7d78,15},{0x7d70,15},{0x3a58,13},
{0x7d68,15},{0x7d60,15},{0xab46,15},{0xab42,15},
{0xab3e,15},{0xab3a,15},{0xab36,15},{0xab32,15},
{0xab2e,15},{0xab2a,15},{0xab26,15},{0xab22,15},
{0xab1e,15},{0xab1a,15},{0xab16,15},{0xab12,15},
{0xab0e,15},{0xab0a,15},{0xab06,15},{0xab02,15},
{0xaafe,15},{0xaafa,15},{0xaaf6,15},{0xaaf2,15},
{0xaaee,15},{0xaaea,15},{0xaae6,15},{0xaae2,15},
{0xaade,15},{0xaada,15},{0xaad6,15},{0xaad2,15},
{0xaace,15},{0xaaca,15},{0xaac6,15},{0xaac2,15},
{0xaabe,15},{0xaaba,15},{0xaab6,15},{0xaab2,15},
{0xaaae,15},{0xaaaa,15},{0xaaa6,15},{0xaaa2,15},
{0xaa9e,15},{0x3a70,12},{0xaa9a,15},{0xaa96,15},
{0xaa92,15},{0x3080, 9},{0xaa8e,15},{0xaa8a,15},
{0xaa86,15},{0xaa82,15},{0x9ffe,15},{0x9ffa,15},
{0x9ff6,15},{0x9ff2,15},{0x9fee,15},{0x9fea,15},
{0x9fe6,15},{0x9fe2,15},{0x9fde,15},{0x9fda,15},
{0x9fd6,15},{0x9fd2,15},{0x9fce,15},{0x9fca,15},
{0x9fc6,15},{0x9fc2,15},{0x7d7e,15},{0x7d7a,15},
{0x7d76,15},{0x7d72,15},{0x7d6e,15},{0x7d6a,15},
{0x7d66,15},{0x7d62,15},{0x3a46,15},{0x3a44,15}
/* End of automatically built huffman compression table */
};

static struct huff_decomp_tree_struct huff_decomp_tree[] = {
/* Begin of automatically built huffman decompression tree */
{ 79,  1},{  2, 66},{ 24,  3},{  4,208},
{292,  5},{  6,298},{317,  7},{ 16,  8},
{  9,173},{ 10,116},{ 41, 11},{ 12, 37},
{125, 13},{357, 14},{ 15,438},{  0,  0},
{229, 17},{ 18, 46},{ 19, 61},{ 20, 99},
{ 21,161},{404, 22},{ 23,483},{  1,  0},
{306, 25},{313, 26},{294, 27},{245, 28},
{221, 29},{231, 30},{277, 31},{ 32,103},
{ 33,108},{ 34,339},{421, 35},{ 36,500},
{  2,  0},{122, 38},{353, 39},{ 40,434},
{  3,  0},{131, 42},{128, 43},{361, 44},
{ 45,442},{  4,  0},{ 56, 47},{ 52, 48},
{135, 49},{370, 50},{ 51,450},{  5,  0},
{138, 53},{375, 54},{ 55,454},{  6,  0},
{148, 57},{ 58, 94},{381, 59},{ 60,460},
{  7,  0},{ 75, 62},{ 63,152},{392, 64},
{ 65,469},{  8,  0},{164, 67},{311, 68},
{ 69,246},{ 70, 97},{253, 71},{257, 72},
{ 73,270},{319, 74},{  9,  0},{ 76,155},
{396, 77},{ 78,473},{ 10,  0},{165, 80},
{296, 81},{300, 82},{295, 83},{206, 84},
{ 85,320},{193, 86},{ 87,318},{199, 88},
{184, 89},{ 90,112},{ 91,346},{430, 92},
{ 93,508},{ 11,  0},{379, 95},{ 96,458},
{ 12,  0},{ 98,218},{ 13,  0},{100,158},
{400,101},{102,478},{ 14,  0},{331,104},
{105,328},{408,106},{107,487},{ 15,  0},
{109,336},{417,110},{111,496},{ 16,  0},
{113,343},{425,114},{115,504},{ 17,  0},
{117,141},{118,186},{119,321},{351,120},
{121,432},{ 18,  0},{355,123},{124,436},
{ 19,  0},{359,126},{127,440},{ 20,  0},
{364,129},{130,444},{ 21,  0},{132,145},
{368,133},{134,448},{ 22,  0},{372,136},
{137,452},{ 23,  0},{377,139},{140,456},
{ 24,  0},{142,234},{143,236},{144,383},
{ 25,  0},{366,146},{147,446},{ 26,  0},
{387,149},{385,150},{151,462},{ 27,  0},
{390,153},{154,467},{ 28,  0},{394,156},
{157,471},{ 29,  0},{398,159},{160,475},
{ 30,  0},{402,162},{163,481},{ 31,  0},
{ 32,  0},{175,166},{214,167},{211,168},
{169,195},{243,170},{171,281},{286,172},
{ 33,  0},{265,174},{ 34,  0},{176,202},
{177,315},{178,297},{179,232},{180,252},
{181,228},{189,182},{255,183},{ 35,  0},
{185,242},{ 36,  0},{284,187},{192,188},
{ 37,  0},{190,241},{191,201},{ 38,  0},
{ 39,  0},{194,227},{ 40,  0},{196,267},
{197,220},{237,198},{ 41,  0},{200,309},
{ 42,  0},{ 43,  0},{203,260},{204,268},
{308,205},{ 44,  0},{244,207},{ 45,  0},
{304,209},{210,223},{ 46,  0},{212,258},
{238,213},{ 47,  0},{215,303},{216,249},
{273,217},{ 48,  0},{219,316},{ 49,  0},
{ 50,  0},{222,278},{ 51,  0},{224,264},
{250,225},{230,226},{ 52,  0},{ 53,  0},
{ 54,  0},{ 55,  0},{ 56,  0},{ 57,  0},
{251,233},{ 58,  0},{363,235},{ 59,  0},
{ 60,  0},{ 61,  0},{239,256},{240,480},
{ 62,  0},{ 63,  0},{ 64,  0},{ 65,  0},
{ 66,  0},{ 67,  0},{299,247},{275,248},
{ 68,  0},{ 69,  0},{ 70,  0},{ 71,  0},
{ 72,  0},{271,254},{ 73,  0},{ 74,  0},
{ 75,  0},{ 76,  0},{259,269},{ 77,  0},
{293,261},{262,263},{ 78,  0},{ 79,  0},
{ 80,  0},{279,266},{ 81,  0},{ 82,  0},
{ 83,  0},{ 84,  0},{ 85,  0},{342,272},
{ 86,  0},{274,335},{ 87,  0},{276,302},
{ 88,  0},{ 89,  0},{ 90,  0},{283,280},
{ 91,  0},{374,282},{ 92,  0},{ 93,  0},
{291,285},{ 94,  0},{301,287},{288,326},
{323,289},{290,427},{ 95,  0},{ 96,  0},
{ 97,  0},{ 98,  0},{ 99,  0},{100,  0},
{101,  0},{102,  0},{103,  0},{104,  0},
{105,  0},{106,  0},{107,  0},{108,  0},
{305,307},{109,  0},{110,  0},{111,  0},
{112,  0},{310,384},{113,  0},{312,314},
{114,  0},{115,  0},{116,  0},{117,  0},
{118,  0},{119,  0},{120,  0},{121,  0},
{122,  0},{322,325},{123,  0},{349,324},
{124,  0},{125,  0},{327,476},{126,  0},
{406,329},{330,485},{127,  0},{412,332},
{410,333},{334,489},{128,  0},{129,  0},
{415,337},{338,494},{130,  0},{419,340},
{341,498},{131,  0},{132,  0},{423,344},
{345,502},{133,  0},{428,347},{348,506},
{134,  0},{350,510},{135,  0},{352,433},
{136,  0},{354,435},{137,  0},{356,437},
{138,  0},{358,439},{139,  0},{360,441},
{140,  0},{362,443},{141,  0},{142,  0},
{365,445},{143,  0},{367,447},{144,  0},
{369,449},{145,  0},{371,451},{146,  0},
{373,453},{147,  0},{148,  0},{376,455},
{149,  0},{378,457},{150,  0},{380,459},
{151,  0},{382,461},{152,  0},{153,  0},
{154,  0},{386,463},{155,  0},{388,464},
{389,466},{156,  0},{391,468},{157,  0},
{393,470},{158,  0},{395,472},{159,  0},
{397,474},{160,  0},{399,477},{161,  0},
{401,479},{162,  0},{403,482},{163,  0},
{405,484},{164,  0},{407,486},{165,  0},
{409,488},{166,  0},{411,490},{167,  0},
{413,491},{414,493},{168,  0},{416,495},
{169,  0},{418,497},{170,  0},{420,499},
{171,  0},{422,501},{172,  0},{424,503},
{173,  0},{426,505},{174,  0},{175,  0},
{429,507},{176,  0},{431,509},{177,  0},
{178,  0},{179,  0},{180,  0},{181,  0},
{182,  0},{183,  0},{184,  0},{185,  0},
{186,  0},{187,  0},{188,  0},{189,  0},
{190,  0},{191,  0},{192,  0},{193,  0},
{194,  0},{195,  0},{196,  0},{197,  0},
{198,  0},{199,  0},{200,  0},{201,  0},
{202,  0},{203,  0},{204,  0},{205,  0},
{206,  0},{207,  0},{208,  0},{209,  0},
{ -1,465},{210,  0},{211,  0},{212,  0},
{213,  0},{214,  0},{215,  0},{216,  0},
{217,  0},{218,  0},{219,  0},{220,  0},
{221,  0},{222,  0},{223,  0},{224,  0},
{225,  0},{226,  0},{227,  0},{228,  0},
{229,  0},{230,  0},{231,  0},{232,  0},
{233,  0},{234,  0},{235,  0},{ -1,492},
{236,  0},{237,  0},{238,  0},{239,  0},
{240,  0},{241,  0},{242,  0},{243,  0},
{244,  0},{245,  0},{246,  0},{247,  0},
{248,  0},{249,  0},{250,  0},{251,  0},
{252,  0},{253,  0},{512,511},{254,  0},
{255,  0}
/* End of automatically built huffman decompression tree */
};




/* Compresses srclen bytes, beginning at *src; the result is stored at *dest
 *
 * Note: If you want to compress 1 byte, then set srclen to 1, not to 0!
 *
 * Only up to 255 bytes can be compressed, but you have to allocate
 * 256 bytes at *dest!
 *
 * Return value: Length of the compressed data, i.e. of the data
 * stored at *dest, or 0 if the parameters are invalid
 */
unsigned short int comp_sp_stat_huff(char *src, unsigned short int srclen,
                                     char *dest)
{
  unsigned short int destlen = 1;
  unsigned short int bytes_left;
  unsigned char bitmask = 0x01;
  unsigned short int bitmask16 = 0x0001;
  unsigned short int codeend = 0x0000;
  unsigned short huffcode = 0x0000;

  if (srclen == 0 || srclen > 255)
    return(0);
  *dest = (char)(srclen-1); /* First byte = length of uncompressed data - 1 */
  src--; /* Will be incremented again later */
  bytes_left = srclen + 1;
  for(;;)
  {
    bitmask16 >>= 1;
    if (bitmask16 == codeend)
    {
      if (--bytes_left == 0)
        break;
      src++;
      huffcode = huff_comp_table[(unsigned char)*src].code;
      codeend = 0x8000 >> huff_comp_table[(unsigned char)*src].len;
      bitmask16 = 0x8000;
    }
    bitmask >>= 1;
    if (bitmask == 0)
    {
      destlen++;
      if (destlen >= srclen)
      {
        dest -= (destlen - 2);
        src -= (srclen - bytes_left);
        dest[0] = 255;
        memcpy(dest + 1, src, srclen);
        return(srclen + 1);
      }
      dest++;
      *dest = 0;
      bitmask = 0x80;
    }
    if (huffcode & bitmask16)
      *dest |= bitmask;
  }
  return(destlen);
}

/* Decompresses a frame, which is stored at *src and srclen bytes long; the
 * result is stored at *dest
 *
 * Note: If the compressed frame is 1 byte long, then set srclen to 1,
 *       not to 0!
 *
 * The compressed frame may be up to 256 bytes long, but due to the
 * limitations of this compression method the uncompressed frame the
 * will never become longer than 255 bytes
 *
 * Return value: Length of the uncompressed data, i.e. of the data
 * stored at *dest; 0 if the parameters are invalid or if the decompression
 * failed (this happens if the frame was not compressed)
 */
unsigned short int decomp_sp_stat_huff(char *src, unsigned short int srclen,
                                       char *dest)
{
  unsigned short int destlen;
  unsigned short int bytes_left;
  unsigned char bitmask;
  signed short int treepos = 0;

  if (srclen < 2 || srclen > 256)
    return(0);
  destlen = (unsigned short int)((unsigned char)(*src) + 1);
  if (destlen == 256)
  {
    /* frame is not really compressed */
    memcpy(dest, src + 1, srclen - 1);
    return(srclen - 1);
  }
  bytes_left = destlen;
  do
  {
    src++;
    if (--srclen == 0)  /* Decompression not complete, but      */
      return(0);        /* no data left => no compressed frame! */
    for (bitmask = 0x80; bitmask > 0; bitmask >>= 1)
    {
      if (*src & bitmask)
        treepos = huff_decomp_tree[treepos].node2;
      else
        treepos = huff_decomp_tree[treepos].node1;
      /* If this happens, we have found a bit-sequence, which is
         not used (101010101100010 or 100111111110010) */
      if (treepos == -1)
        return(0);
      if (huff_decomp_tree[treepos].node2 == 0)
      {
        *(dest++) = (char)huff_decomp_tree[treepos].node1;
        if (--bytes_left == 0)
          break; /* Decompression complete */
        treepos = 0;
      }
    }
  }
  while (bytes_left);
  if (srclen > 1) /* Decompression complete, but some       */
    return(0);    /* bytes are left => no compressed frame! */
  /* Now check, if all remaining bits are set to 0; if not, it's
     no compressed frame (well, hopefully...) */
  while (bitmask > 0)
  {
    bitmask >>= 1;
    if (*src & bitmask)
      return(0);
  }
  return(destlen);
}

/* -------------------------------------------------------------------- */


void UpdateCmComp(unsigned short w, unsigned short &t, unsigned char &bl,
                  long &m, unsigned char &ml, unsigned short &Count,
                  char *OBuf)
{
   t = HTable[w].code;
   bl = HTable[w].len;
   m = m << bl;
   m = m | (t >> (16 - bl));
   ml += bl;
   while (ml >= 8)
   {
      OBuf[Count] = (unsigned char)(m >> (ml - 8));
      Count++;
      ml -= 8;
   }
}


int Compress(unsigned char &ml, long &m, unsigned short &Count,
             char *OBuf, bool flag, char *zeile, int len)
{
   unsigned char i, bl;
   unsigned short t;
   unsigned char Mml;
   long Mm;

   for (i = 0; i < sizeof(OBuf); i++)
      OBuf[i] = '\0';
   Count = 0;
   Mml = ml;
   Mm = m;
   for (i = 0; i < len; i++)
      UpdateCmComp((unsigned char)zeile[i], t, bl, m, ml, Count, OBuf);
   if (flag)
   {
      UpdateCmComp(256, t, bl, m, ml, Count, OBuf);
      if (ml > 0)
      {
         OBuf[Count] = (unsigned char)(m << (8 - ml));
         Count++;
      }
      m = 0;
      ml = 0;
   }
   if (Count > strlen(zeile))
   {
      for (i = 0; i < sizeof(OBuf); i++)
         OBuf[i] = '\0';
      Count = 0;
      ml = Mml;
      m = Mm;
      UpdateCmComp(257, t, bl, m, ml, Count, OBuf);
      if (ml > 0)
      {
         OBuf[Count] = (unsigned char)(m << (8 - ml));
         Count++;
      }
      m = 0;
      ml = 0;
      OBuf[Count] = len;
      Count++;
      for (i = 0; i < len; i++)
      {
         OBuf[Count] = (unsigned char)zeile[i];
         Count++;
      }
   }
   return Count;
}


int DeCompress(unsigned char &ml, long &m, unsigned short &Count,
               char *OBuf, char *zeile, int len)
{
   unsigned char i, l;
   unsigned short w, t;

   for (i = 0; i < sizeof(OBuf); i++)
     OBuf[i] = '\0';
   Count = 0;
   for (i = 0; i < len; i++)
   {
      if (DNotComp == 0)
      {
         m = m << 8;
         ml += 8;
         m = m | (unsigned char)zeile[i];
         w = 0;
         do
         {
            t = HTable[w].code;
            l = HTable[w].len;
            if (ml >= l)
            {
               if (t == (unsigned short)(m >> (ml-l) << (16 - l)))
               {
                  if (w == 256)
                  {
                     ml = 0;
                     m = 0;
                  }
                  else
                     if (w == 257)
                     {
                        ml = 0;
                        m = 0;
                        DNotComp = 255;
                     }
                     else
                     {
                        OBuf[Count] = (unsigned char)w;
                        Count++;
                        ml -= l;
                     }
                     w = 0;
               }
               else w++;
            }
            else w++;
         }
         while (w <= 257 && ml >= 3);
      }
      else
         if (DNotComp == 255)
         {
            DNotCAnz = (unsigned char) zeile[i];
            DNotComp = 1;
         }
         else
            if (DNotComp == 1)
            {
               OBuf[Count] = (unsigned char)zeile[i];
               Count++;
               DNotCAnz--;
               if (DNotCAnz <= 0) DNotComp = 0;
            }
   }
   return Count;
}

int comp_top_stat_huff(bool flag, char *OBuf, char *zeile, int len)
{
/*
   CCount = 0;
   Cml = 0;
   Cm = 0;
*/
   return Compress(Cml, Cm, CCount, OBuf, flag, zeile, len);
}

int decomp_top_stat_huff(char *OBuf, char *zeile, int len)
{
/*
   DCount = 0;
   Dml = 0;
   Dm = 0;
*/
   return DeCompress(Dml, Dm, DCount, OBuf, zeile, len);
}

void ahuf_top_init()
{
   DNotCAnz = 0;
   DNotComp = 0;
   CCount = 0;
   Cml = 0;
   Cm = 0;
   DCount = 0;
   Dml = 0;
   Dm = 0;
}



//#endif



/*

char *compress (char *zeile)
{ lastfunc ("compress");
  char Hstr[BUFLEN] = { 0 };
  char s,t,ch;
  byte i,b,c;
  int  a;
  bool long;
  a = 7;
  b = 1;
  long = false;

  i = 0;
  do
  {
    i++;
    t = HTable[((int)(unsigned char)(zeile[i]))].Tab;
    s = $8000;
    C = 0;

    do
    {
      C++;
      if (t && s = s)
        Hstr[b] = ((char)(((int)(unsigned char)(Hstr[b])))) + 1 << a;
      s = s >> 1;
      a--;
      if (a < 0)
      {
        a = 7;
        b++;
        if (b > 254)
          long = true;
      }
    }
    While (C < HTable[((int)(unsigned char)(zeile[i]))].Len && !long)
    Hstr[0] :=  ((char)(b));
  }
  while (i < strlen(zeile) && !long)


  if (strlen(Hstr) > strlen(zeile) || long)
  {
    Hstr = zeile[0] + zeile;
    ch = #255;
  }
  else
    ch = ((char)(strlen(Hstr)));
  return (ch + Hstr);
}


     HTable : Array[0..257] of Table_Typ =

  ((Tab : $AB2C; Len : 15), (Tab : $AA84; Len : 15), (Tab : $9FC4; Len : 15),
   (Tab : $AB3C; Len : 15), (Tab : $AB1C; Len : 15), (Tab : $AAFC; Len : 15),
   (Tab : $AAEC; Len : 15), (Tab : $AAD4; Len : 15), (Tab : $AAB4; Len : 15),
   (Tab : $F340; Len : 10), (Tab : $AAA4; Len : 15), (Tab : $7D64; Len : 15),
   (Tab : $AADC; Len : 15), (Tab : $F400; Len :  7), (Tab : $AA94; Len : 15),
   (Tab : $9FF4; Len : 15), (Tab : $9FD4; Len : 15), (Tab : $7D74; Len : 15),
   (Tab : $AB44; Len : 15), (Tab : $AB34; Len : 15), (Tab : $AB24; Len : 15),
   (Tab : $AB14; Len : 15), (Tab : $AB04; Len : 15), (Tab : $AAF4; Len : 15),
   (Tab : $AAE4; Len : 15), (Tab : $AB60; Len : 14), (Tab : $AB0C; Len : 15),
   (Tab : $AACC; Len : 15), (Tab : $AABC; Len : 15), (Tab : $AAAC; Len : 15),
   (Tab : $AA9C; Len : 15), (Tab : $AA8C; Len : 15), (Tab : $C000; Len :  3),
   (Tab : $3A80; Len :  9), (Tab : $ABC0; Len : 10), (Tab : $0060; Len : 11),
   (Tab : $7D40; Len : 12), (Tab : $AB5C; Len : 14), (Tab : $0000; Len : 12),
   (Tab : $AB58; Len : 14), (Tab : $7C00; Len :  9), (Tab : $3C80; Len :  9),
   (Tab : $7D00; Len : 11), (Tab : $0010; Len : 12), (Tab : $1200; Len :  7),
   (Tab : $7A00; Len :  7), (Tab : $B800; Len :  6), (Tab : $3200; Len :  7),
   (Tab : $2200; Len :  7), (Tab : $F600; Len :  8), (Tab : $3D00; Len :  8),
   (Tab : $9E00; Len :  9), (Tab : $BD80; Len :  9), (Tab : $7C80; Len :  9),
   (Tab : $0080; Len :  9), (Tab : $AA00; Len :  9), (Tab : $BD00; Len :  9),
   (Tab : $9F00; Len :  9), (Tab : $0300; Len :  8), (Tab : $AB78; Len : 13),
   (Tab : $AB68; Len : 13), (Tab : $3C00; Len :  9), (Tab : $3000; Len :  9),
   (Tab : $0020; Len : 11), (Tab : $7D50; Len : 12), (Tab : $3800; Len :  7),
   (Tab : $7800; Len :  7), (Tab : $9C00; Len :  7), (Tab : $FE00; Len :  7),
   (Tab : $2400; Len :  6), (Tab : $BC00; Len :  8), (Tab : $0200; Len :  8),
   (Tab : $0100; Len :  8), (Tab : $F100; Len :  8), (Tab : $0040; Len : 11),
   (Tab : $3100; Len :  8), (Tab : $F200; Len :  8), (Tab : $3400; Len :  7),
   (Tab : $1C00; Len :  7), (Tab : $1E00; Len :  7), (Tab : $BE00; Len :  7),
   (Tab : $ABA0; Len : 11), (Tab : $3E00; Len :  7), (Tab : $1400; Len :  6),
   (Tab : $3600; Len :  7), (Tab : $F380; Len :  9), (Tab : $F080; Len :  9),
   (Tab : $2000; Len :  8), (Tab : $FC00; Len :  8), (Tab : $9F80; Len : 10),
   (Tab : $9E80; Len :  9), (Tab : $AB90; Len : 12), (Tab : $3B80; Len :  9),
   (Tab : $AB80; Len : 12), (Tab : $AB54; Len : 14), (Tab : $3A50; Len : 13),
   (Tab : $AB50; Len : 14), (Tab : $A000; Len :  5), (Tab : $1800; Len :  6),
   (Tab : $9800; Len :  6), (Tab : $7000; Len :  5), (Tab : $4000; Len :  3),
   (Tab : $0400; Len :  6), (Tab : $AC00; Len :  6), (Tab : $F800; Len :  6),
   (Tab : $6000; Len :  4), (Tab : $3A00; Len : 10), (Tab : $FD00; Len :  8),
   (Tab : $2800; Len :  5), (Tab : $B000; Len :  6), (Tab : $8000; Len :  4),
   (Tab : $B400; Len :  6), (Tab : $1000; Len :  7), (Tab : $7D20; Len : 12),
   (Tab : $E000; Len :  5), (Tab : $9000; Len :  5), (Tab : $E800; Len :  5),
   (Tab : $0800; Len :  5), (Tab : $F700; Len :  8), (Tab : $A800; Len :  7),
   (Tab : $7D80; Len :  9), (Tab : $F300; Len : 10), (Tab : $7E00; Len :  7),
   (Tab : $AB48; Len : 14), (Tab : $3A48; Len : 13), (Tab : $AB4C; Len : 14),
   (Tab : $3A60; Len : 12), (Tab : $9FFC; Len : 15), (Tab : $9FEC; Len : 15),
   (Tab : $2100; Len :  8), (Tab : $9FDC; Len : 15), (Tab : $9FCC; Len : 15),
   (Tab : $F000; Len :  9), (Tab : $7D7C; Len : 15), (Tab : $7D6C; Len : 15),
   (Tab : $3A40; Len : 14), (Tab : $AB40; Len : 15), (Tab : $AB38; Len : 15),
   (Tab : $AB30; Len : 15), (Tab : $AB28; Len : 15), (Tab : $AB20; Len : 15),
   (Tab : $AB18; Len : 15), (Tab : $AB70; Len : 13), (Tab : $AB10; Len : 15),
   (Tab : $AB08; Len : 15), (Tab : $AB00; Len : 15), (Tab : $AAF8; Len : 15),
   (Tab : $AAF0; Len : 15), (Tab : $3B00; Len :  9), (Tab : $AAE8; Len : 15),
   (Tab : $AAE0; Len : 15), (Tab : $AAD8; Len : 15), (Tab : $AAD0; Len : 15),
   (Tab : $AB64; Len : 14), (Tab : $7D30; Len : 12), (Tab : $AAC8; Len : 15),
   (Tab : $AAC0; Len : 15), (Tab : $AAB8; Len : 15), (Tab : $AAB0; Len : 15),
   (Tab : $AAA8; Len : 15), (Tab : $AAA0; Len : 15), (Tab : $AA98; Len : 15),
   (Tab : $AA90; Len : 15), (Tab : $AA88; Len : 15), (Tab : $AA80; Len : 15),
   (Tab : $9FF8; Len : 15), (Tab : $9FF0; Len : 15), (Tab : $9FE8; Len : 15),
   (Tab : $9FE0; Len : 15), (Tab : $9FD8; Len : 15), (Tab : $9FD0; Len : 15),
   (Tab : $9FC8; Len : 15), (Tab : $9FC0; Len : 15), (Tab : $7D78; Len : 15),
   (Tab : $7D70; Len : 15), (Tab : $3A58; Len : 13), (Tab : $7D68; Len : 15),
   (Tab : $7D60; Len : 15), (Tab : $AB46; Len : 15), (Tab : $AB42; Len : 15),
   (Tab : $AB3E; Len : 15), (Tab : $AB3A; Len : 15), (Tab : $AB36; Len : 15),
   (Tab : $AB32; Len : 15), (Tab : $AB2E; Len : 15), (Tab : $AB2A; Len : 15),
   (Tab : $AB26; Len : 15), (Tab : $AB22; Len : 15), (Tab : $AB1E; Len : 15),
   (Tab : $AB1A; Len : 15), (Tab : $AB16; Len : 15), (Tab : $AB12; Len : 15),
   (Tab : $AB0E; Len : 15), (Tab : $AB0A; Len : 15), (Tab : $AB06; Len : 15),
   (Tab : $AB02; Len : 15), (Tab : $AAFE; Len : 15), (Tab : $AAFA; Len : 15),
   (Tab : $AAF6; Len : 15), (Tab : $AAF2; Len : 15), (Tab : $AAEE; Len : 15),
   (Tab : $AAEA; Len : 15), (Tab : $AAE6; Len : 15), (Tab : $AAE2; Len : 15),
   (Tab : $AADE; Len : 15), (Tab : $AADA; Len : 15), (Tab : $AAD6; Len : 15),
   (Tab : $AAD2; Len : 15), (Tab : $AACE; Len : 15), (Tab : $AACA; Len : 15),
   (Tab : $AAC6; Len : 15), (Tab : $AAC2; Len : 15), (Tab : $AABE; Len : 15),
   (Tab : $AABA; Len : 15), (Tab : $AAB6; Len : 15), (Tab : $AAB2; Len : 15),
   (Tab : $AAAE; Len : 15), (Tab : $AAAA; Len : 15), (Tab : $AAA6; Len : 15),
   (Tab : $AAA2; Len : 15), (Tab : $AA9E; Len : 15), (Tab : $3A70; Len : 12),
   (Tab : $AA9A; Len : 15), (Tab : $AA96; Len : 15), (Tab : $AA92; Len : 15),
   (Tab : $3080; Len :  9), (Tab : $AA8E; Len : 15), (Tab : $AA8A; Len : 15),
   (Tab : $AA86; Len : 15), (Tab : $AA82; Len : 15), (Tab : $9FFE; Len : 15),
   (Tab : $9FFA; Len : 15), (Tab : $9FF6; Len : 15), (Tab : $9FF2; Len : 15),
   (Tab : $9FEE; Len : 15), (Tab : $9FEA; Len : 15), (Tab : $9FE6; Len : 15),
   (Tab : $9FE2; Len : 15), (Tab : $9FDE; Len : 15), (Tab : $9FDA; Len : 15),
   (Tab : $9FD6; Len : 15), (Tab : $9FD2; Len : 15), (Tab : $9FCE; Len : 15),
   (Tab : $9FCA; Len : 15), (Tab : $9FC6; Len : 15), (Tab : $9FC2; Len : 15),
   (Tab : $7D7E; Len : 15), (Tab : $7D7A; Len : 15), (Tab : $7D76; Len : 15),
   (Tab : $7D72; Len : 15), (Tab : $7D6E; Len : 15), (Tab : $7D6A; Len : 15),
   (Tab : $7D66; Len : 15), (Tab : $7D62; Len : 15), (Tab : $3A46; Len : 15),
   (Tab : $3A44; Len : 15), (Tab : $AAC4; Len : 15), (Tab : $9FE4; Len : 15));


Function Compress (* Zeile : String) : String *);
Var   Hstr : String;
      t    : Word;
      s    : Word;
      i    : Byte;
      a    : Integer;
      b,c  : Byte;
      ch   : Char;
      long : Boolean;
Begin
  FillChar(Hstr,SizeOf(Hstr),0);
  a := 7;
  b := 1;
  long := false;

  i := 0;
  While (i < length(Zeile)) and not long do
  begin
    inc(i);
    t := HTable[ord(Zeile[i])].Tab;
    s := $8000;
    C := 0;

    While (C < HTable[ord(Zeile[i])].Len) and not long do
    begin
      inc(C);
      if t and s = s then Hstr[b] := Chr(ord(Hstr[b]) + 1 shl a);
      s := s shr 1;
      dec(a);
      if a < 0 then
      begin
        a := 7;
        inc(b);
        if b > 254 then long := true;
      end;
    end;
    Hstr[0] := chr(b);
  end;

  if (length(Hstr) > length(Zeile)) or long then
  begin
    Hstr := Zeile[0] + Zeile;
    ch := #255;
  end else ch := Chr(length(Hstr));
  Compress := ch + Hstr;
End;


Function DeCompress (* Zeile : String) : String *);
Var   Hstr  : String;
      b,i,l : Byte;
      a     : Integer;
      t,t2  : Word;
      Bit   : LongInt;
      ch    : Char;

Begin
  ch := Zeile[1];
  delete(Zeile,1,1);
  if ch = #255 then delete(Zeile,1,1);
  if (ch < #255) and (Zeile[0] > #0) then
  begin
    Hstr := '';
    l := 0;
    Bit := 0;

    for i := 1 to length(Zeile) do
    begin
      Bit := (Bit shl 8) or ord(Zeile[i]);
      l := Byte(l + 8);

      a := 0;

      Repeat
        b := HTable[a].Len;
        if l >= b then
        begin
          t := HTable[a].Tab;
          t2 := Word(Bit shr (l-b)) shl (16-b);

          if t = t2 then
          begin
            Hstr := Hstr + chr(a);
            l := l - b;
            a := -1;
          end;
        end;
        inc(a);
      Until (a > 257) or (l < 3);
    end;
  end else Hstr := Zeile;
  DeCompress := Hstr;
End;


int enchufmem(int gzip, char *membase, long osize,char *outbase, long *outsize, char *outputfile, int crlfconv)
{
  int error;
  char *ptr;
  int i;
  long size;
  int conv;
  char *tmpbuf = NULL;
  long tmplen;
  int bin;
  int prebin;
  int bintest;
  char c;
  char *tmpptr;
  char *srcptr;
  char tempname[80];

  if (osize == 0) return(1);
  srcbuf = membase;
  size = osize;
  conv = 0;

  if (crlfconv) {
    tmpbuf = malloc(osize*2);
    if (tmpbuf == NULL) return(1);
    tmpptr = tmpbuf;
    tmplen = 0;
    srcbuf = membase;
    srcptr = srcbuf;
    srclen = 0;
    bin = 0;
    prebin = 0;
    bintest = 0;

    while (srclen < osize) {
      if (!bin) {
        if (*(srcptr) == '\n') {
          *(tmpptr) = '\r';
          tmpptr++;
          tmplen++;
          if (prebin) bin = 1;
        }
      }
      *(tmpptr) = *(srcptr);
      if (!prebin) {
        c = *(srcptr);
        switch (bintest) {
        case 0:
          if (c == '\n') bintest++;
          break;
        case 1:
          if (c == '#') bintest++;
          else if (c != '\n') bintest = 0;
          break;
        case 2:
          if (c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if (c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if (c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:
          if (c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:
          if (isdigit((int)c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
      tmplen++;
      srclen++;
      tmpptr++;
      srcptr++;
    }
    size = tmplen;
    srcbuf = tmpbuf;
    conv = 1;
  }

  if (gzip) {
    strcpy(tempname, tempdir);
    strcat(tempname, "gzipXXXXXX");
    mymktemp(tempname);
    if ((outfile = fopen(tempname, "wb")) == NULL) {
      if (conv)
        free(tmpbuf);
      return(1);
    }
    if (fwrite(srcbuf, size, 1, outfile) != 1) {
      fclose(outfile);
      sfdelfile(tempname);
      if (conv)
        free(tmpbuf);
      return 1;
    }
    fclose(outfile);
    if (conv)
      free(tmpbuf);

    error = enchuf(1, 0, tempname, outputfile, 0);
    sfdelfile(tempname);
    *outbase = NULL;
    *outsize = 0;
    return error;
  }

  in_memory = 1;
  out_memory = 1;
  init_huf();

  srcbufptr = srcbuf;
  srclen = 0;
  srcbuflen = size;

  // first try memory
  error = 0;
  destlen = 0;
  destbuflen = size + size/6;
  destbuf = malloc(destbuflen);   // allocate buffer with security margin
  if (destbuf != NULL) {
    destbufptr = destbuf;
    ptr = (char *)&size;
    // send length
    for (i = 0 ; i < sizeof(size) ; i++) {
      if (wri_char(*ptr++) == EOF) {
        error = 1;
        break;
      }
    }
    if (!error) {
      if (!Encode()) {
        *outbase = destbuf;
        *outsize = destlen;
        if (conv)
          free(tmpbuf);
        return(0);
      }
    }
  }
  free(destbuf);

  // output to memory failed, write to file
  out_memory = 0;
  init_huf();

  srcbufptr = srcbuf;
  srclen = 0;
  *outbase = NULL;
  *outsize = 0;

  if ((outfile = fopen(outputfile, "wb")) == NULL) {
    if (conv)
      free(tmpbuf);
    return(1);
  }
  error = 0;
  if (fwrite(&size, sizeof size, 1, outfile) < 1) {
    error = 1;
  }
  if (!error) {
    if (Encode())
      error = 1;
  }
  fclose(outfile);
  if (conv)
    free(tmpbuf);
  return(error);
}

int dechufmem(gzip, membase, size, outbase, outsize, outputfile, crlfconv)
char *membase;
long size;
char **outbase;
long *outsize;
char *outputfile;
int crlfconv, gzip;
{
  int error;
  char *ptr;
  int i;
  long packsize;
  char tempname[80];
  char *outputfptr;
  int conv;
  long worlen;
  long tmplen;
  char *tmpbuf;
  int bin;
  int prebin;
  int bintest;
  int c;
  int only_cr;

  if (gzip) {
    strcpy(tempname, tempdir);
    strcat(tempname, "gzipXXXXXX");
    mymktemp(tempname);
    strcat(tempname, ".gz");
    if ((outfile = fopen(tempname, "wb")) == NULL) {
      return(1);
    }
    error = 0;
    if (fwrite(membase, size, 1, outfile) != 1) {
      fclose(outfile);
      sfdelfile(tempname);
      return 1;
    }
    fclose(outfile);

    error = dechuf(1, 0, tempname, outputfile, crlfconv);
    sfdelfile(tempname);
    *outbase = NULL;
    *outsize = 0;
    return error;
  }

  conv = 0;
  outputfptr = outputfile;
  if (crlfconv) {
    strcpy(tempname, tempdir);
    strcat(tempname, "hufXXXXXX");
    mymktemp(tempname);
    conv = 1;
    outputfptr = tempname;
  }

  in_memory = 1;
  out_memory = 1;
  init_huf();

  srcbuf = membase;
  srcbufptr = srcbuf;
  srclen = 0;
  srcbuflen = size;
  if (srcbuflen < sizeof(packsize)) return(1);
  // get length
  ptr = (char *)&packsize;
  for (i = 0 ; i < sizeof(packsize) ; i++) {
    *ptr++ = read_char();
  }
  if (packsize == 0) return(1);

  // first try memory
  error = 0;
  destlen = 0;
  destbuflen = packsize;
  destbuf = malloc(destbuflen);   // allocate buffer
  if (destbuf != NULL) {
    destbufptr = destbuf;
    if (!Decode(packsize)) {
      if (conv) {
        tmpbuf = malloc(packsize);
        if (tmpbuf != NULL) {
          tmplen = 0;
          worlen = 0;
          bin = 0;
          prebin = 0;
          bintest = 0;
          only_cr = 0;
          while (worlen < packsize) {
            if (!bin) {
              if (only_cr && (*(destbuf + worlen) != '\n')) {
                *(tmpbuf + tmplen) = '\n';
                tmplen++;
              }
              only_cr = 0;
            }
            if (bin || (*(destbuf + worlen) != '\r')) {
              *(tmpbuf + tmplen) = *(destbuf + worlen);
              tmplen++;
            }
            if (!bin && (*(destbuf + worlen) == '\r')) only_cr = 1;
            if (prebin && (*(destbuf + worlen) == '\n')) bin = 1;
            if (!prebin) {
              c = (int)(*(destbuf + worlen));
              switch (bintest) {
              case 0:
                if ((char)c == '\n') bintest++;
                break;
              case 1:
                if ((char)c == '#') bintest++;
                else if ((char)c != '\n') bintest = 0;
                break;
              case 2:
                if ((char)c == 'B') bintest++;
                else bintest = 0;
                break;
              case 3:
                if ((char)c == 'I') bintest++;
                else bintest = 0;
                break;
              case 4:
                if ((char)c == 'N') bintest++;
                else bintest = 0;
                break;
              case 5:
                if ((char)c == '#') bintest++;
                else bintest = 0;
                break;
              case 6:
                if (isdigit(c)) prebin = 1;
                else bintest = 0;
                break;
              default:
                bintest = 0;
                break;
              }
            }
            worlen++;
          }
          *outbase = tmpbuf;
          *outsize = tmplen;
          free(destbuf);
          return(0);
        }
      }
      else {
        *outbase = destbuf;
        *outsize = packsize;
        return(0);
      }
    }
  }
  free(destbuf);

  // output to memory failed, write to file
  out_memory = 0;
  init_huf();

  srcbuf = membase;
  srcbufptr = srcbuf + sizeof(packsize);
  srclen = sizeof(packsize);
  srcbuflen = size;
  *outbase = NULL;
  *outsize = 0;

  if ((outfile = fopen(outputfptr, "wb")) == NULL) {
    return(1);
  }
  if (Decode(packsize))
    error = 1;
  fclose(outfile);
  if ((conv) && (!error)) {
    bin = 0;
    bintest = 0;
    prebin = 0;
    if ((infile = fopen(tempname, "rb")) == NULL) {
      return(1);
    }
    if ((outfile = fopen(outputfile, "wb")) == NULL) {
      fclose(infile);
      unlink(tempname);
      return(1);
    }
    only_cr = 0;
    while ((c = getc(infile)) != EOF) {
      if (!bin) {
        if (only_cr && ((char)c != '\n')) {
          putc('\n',outfile);
        }
        only_cr = 0;
      }
      if (bin || ((char)c != '\r')) putc(c,outfile);
      if (!bin) {
        if ((char)c == '\r') only_cr = 1;
        if (prebin && ((char)c == '\n')) bin = 1;
      }
      if (!prebin) {
        switch (bintest) {
        case 0:
          if ((char)c == '\n') bintest++;
          break;
        case 1:
          if ((char)c == '#') bintest++;
          else if ((char)c != '\n') bintest = 0;
          break;
        case 2:
          if ((char)c == 'B') bintest++;
          else bintest = 0;
          break;
        case 3:
          if ((char)c == 'I') bintest++;
          else bintest = 0;
          break;
        case 4:
          if ((char)c == 'N') bintest++;
          else bintest = 0;
          break;
        case 5:
          if ((char)c == '#') bintest++;
          else bintest = 0;
          break;
        case 6:
          if (isdigit(c)) prebin = 1;
          else bintest = 0;
          break;
        default:
          bintest = 0;
          break;
        }
      }
    }
    fclose(infile);
    fclose(outfile);
  }
  if (conv) {
    unlink(tempname);
  }
  return(error);
}

*/
