/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------
  BINSPLIT-Protocol (Definitions)
  -------------------------------

  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990703 DH3MB  Begin of implementation

#ifdef FEATURE_BINSPLIT

#define BS_MAGIC ">BS<"
#define BINSPLIT_SIZE 20000

/*---------------------------------------------------------------------------*/

class fileio_binsplit //: public fileio
{

private:
  void putheaderitem(crcthp *, char *, ...);
  void putheader(char, char);
  char *fname;
  unsigned long int fsize;
  time_t ftime;

protected:
  //struct file_info_t *f;
//  void _rx(void);
//  void _tx(void);

public:
  void tx(void);
 // fileio_binsplit(void);
 // ~fileio_binsplit(void);
  void set_sendname(const char *name);

};

/*---------------------------------------------------------------------------*/

#endif
