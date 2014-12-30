/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  --------------------------------------------------------
  Definitionen fuer saemtliche CRC-routinen in der Mailbox
  --------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990615 DH3MB  Cleaned up the whole CRC-stuff

/*---------------------------------------------------------------------------*/

class crc
{

public:
  virtual void update (char) = 0; // abstract
  virtual void update (char *, unsigned short int);
  void readfile(char *, off_t);
  virtual ~crc (void) {};
};

/*---------------------------------------------------------------------------*/

class crcthp : public crc
{

public:
  crcthp(void);
  unsigned short int result;
  virtual void update (char);
  virtual void update (char *pp, unsigned short int num)
               { crc::update(pp, num); }

};

/*---------------------------------------------------------------------------*/

class crcfbb : public crc
{

public:
  crcfbb(void);
  unsigned short int result;
  virtual void update (char);
  virtual void update (char *pp, unsigned short int num)
               { crc::update(pp, num); }

};

/*---------------------------------------------------------------------------*/

class crcfcs : public crc
{

public:
  crcfcs(void);
  unsigned short int result;
  virtual void update (char);
  virtual void update (char *pp, unsigned short int num)
               { crc::update(pp, num); }

};

/*---------------------------------------------------------------------------*/

class crc16 : public crc
{

public:
  crc16(void);
  unsigned short int result;
  virtual void update (char);
  virtual void update (char *pp, unsigned short int num)
               { crc::update(pp, num); }

};

/*---------------------------------------------------------------------------*/

class crc32 : public crc
{

public:
  crc32(void);
  unsigned long int result;
  virtual void update (char);
  virtual void update (char *pp, unsigned short int num)
               { crc::update(pp, num); }

};

/*---------------------------------------------------------------------------*/
