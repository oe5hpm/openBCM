/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------------------
  Definitionen und Deklarationen fuer Mailbox
  -------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*----------------------------------------------------------------------*/
/*                                                                      */
/*      LZSS ENCODING                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
#define N4K       4096 /* buffer size (original 4096) */
#define N2K       2048
#define F_          60 /* pre-sence buffer size */
#define THRESHOLD    2
#define NILHUF     N2K /* term of tree */

/*----------------------------------------------------------------------*/
/*                                                                      */
/*      HUFFMAN ENCODING                                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
#define N_CHAR  (256 - THRESHOLD + F_)  /* {code: 0..N_CHAR-1} */
#define T       (N_CHAR * 2 - 1)        /* size of table */
#define R       (T - 1)                 /* root position */
#define MAX_FREQ 0x8000          /* tree update timing from frequency */

class huffcod
{
private:
  char          *text_buf;
  int           *lson, *rson, *dad;
  char          *same;
  unsigned int  *freq;   /* frequency table */
  int           *prnt;   /* points to parent node */
  int           *son;    /* points to son node (son[i],son[i+]) */
  int           in_memory, out_memory;
  FILE          *infile, *outfile;
  unsigned long textsize, codesize;
  char          *srcbuf, *destbuf;
  char          *srcbufptr, *destbufptr;
  long          srclen, destlen;
  long          srcbuflen, destbuflen;
  unsigned int  match_position, match_length;
  unsigned int  getbuf;
  char          getlen;
  unsigned int  putbuf;
  char          putlen;
  unsigned long numchar;

  int           read_char ();
  unsigned int  read_char1 ();
  int           wri_char (int chr);
  void          InitTree ();
  void          InsertNode (register int r);
  void          link (int n, int p, int q);
  void          linknode (int p, int q, int r);
  void          DeleteNode (register int p);
  int           GetBit ();
  int           GetByte ();
  int           GetNBits (register unsigned int n);
  int           Putcode (register int l, register unsigned int c);
  void          StartHuff ();
  void          reconst ();
  void          update (unsigned c);
  unsigned int  EncodeChar (unsigned c);
  int           EncodePosition (unsigned c);
  int           EncodeEnd ();
  int           DecodeChar ();
  int           DecodePosition ();
  unsigned int  Encode ();
  int           Decode (unsigned long int textsize);  /* recover */
  void          init_huf ();
  crcfbb        *crc_fbb;

public:
  int           err;
                huffcod (crcfbb *crc_class);
                ~huffcod (void);
  unsigned int  encode (char *inputfile, char *outputfile);
  unsigned int  decode (char *inputfile, char *outputfile, unsigned long inputoffset);
};
