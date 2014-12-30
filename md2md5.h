/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -----------------------------------------------
  MD2/MD5 message digest algorithms (definitions)   Reference: RFC 1319/1321
  -----------------------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

class MDx
{
protected:
  virtual void finalize (void) = 0; // abstract
  char digest[16];
  char finalized;

public:
  virtual void update (char *, unsigned short int) = 0; // abstract
  virtual void update (char *);
  void readfile (char *, off_t);
  void getdigest (char *);
  void gethexdigest (char *);
  virtual ~MDx (void) {};
};

/*---------------------------------------------------------------------------*/

#ifdef FEATURE_MDPW

/*---------------------------------------------------------------------------*/

// Code taken out of the appendix of RFC 1319, changes by DH3MB

/*---------------------------------------------------------------------------*/

typedef struct MD2_CTX
{ char state[16];       // state
  char checksum[16];    // checksum
  unsigned int count;           // number of bytes, modulo 16
  char buffer[16];      // input buffer
} MD2_CTX;

class MD2 : public MDx
{

private:
  void init(void);
  virtual void finalize(void);
  void transform (char[16]);
  MD2_CTX context;

public:
  MD2(void);
  virtual void update (char *, unsigned short int);
  virtual void update (char *str) { MDx::update(str); }
  virtual ~MD2 (void) {};
};

/*---------------------------------------------------------------------------*/

// Code taken out of the appendix of RFC 1321, changes by DH3MB

/*---------------------------------------------------------------------------*/

typedef struct MD5_CTX
{ unsigned long int state[4];   // state (ABCD)
  unsigned long int count[2];   // number of bits, modulo 2^64 (lsb first)
  char buffer[64];      // input buffer
} MD5_CTX;

class MD5 : public MDx
{
private:
  void init(void);
  virtual void finalize (void);
  void transform (unsigned long int[4], char[64]);
  void encode (char *, unsigned long int *, unsigned int);
  void decode (unsigned long int *, char *, unsigned int);
  MD5_CTX context;

public:
  MD5(void);
  virtual void update(char *, unsigned short int);
  virtual void update(char *str) { MDx::update(str); }
  virtual ~MD5 (void) {};
};

/*---------------------------------------------------------------------------*/

#endif
