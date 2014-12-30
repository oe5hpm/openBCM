/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -----------------------------------------------------------------
  Filetransfer base class + text- and AutoBIN-methods (definitions)
  -----------------------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

#define REQ_CRCTHP      128
#define REQ_MD5HASH      64

/*---------------------------------------------------------------------------*/

struct file_info_t {
  FILE *ptr;                    // Pointer
  int handle;                   // Handle
  char exists;                  // 1, if file exists
  char name[FNAMELEN+1];        // Name without path
  char path[FSPATHLEN + FNAMELEN + 1];  // Path
  char fullname[FSPATHLEN + FNAMELEN + 1];  // Name with path
  off_t size;                   // Size
  off_t offset;                 // Offset, from which the file will be sent
  time_t unixtime;              // Date/time in unix-format
  unsigned long dostime;        // Date/time in DOS-format
  char eraseflag;               // Erase file after tx?
  off_t pos;                    // Current position
  char requires;                // Required hashes/CRCs for protocol
  char md5hash[16];             // 128 bit MD5 hash
  char md5str[33];              // 128 bit MD5 hash, hex-string
  unsigned short int crc;       // 16 bit CRCthp
};

/*---------------------------------------------------------------------------*/

class fileio
{
private:
  char *cut_file (char *);
public:
  fileio(void);
  virtual ~fileio(void);
  void usefile(char *);
  void doerase(void);
  void setfilename(char *);
  void set_offset(off_t);
  void rx(void);
  void tx(void);

protected:
  struct file_info_t *f;
  virtual void _rx(void)=0; // abstract
  virtual void _tx(void)=0; // abstract

};

/*---------------------------------------------------------------------------*/

class fileio_text : public fileio
{

public:
  fileio_text(void);
  void settail(signed short int);

protected:
  virtual void _rx(void);
  virtual void _tx(void);

private:
  signed short int tail;

};

/*---------------------------------------------------------------------------*/

class fileio_abin : public fileio
{

public:
  fileio_abin(void);
  void set_header(char *);

protected:
  virtual void _rx(void);
  virtual void _tx(void);

private:
  char autobin_header[LINELEN+1];

};

/*---------------------------------------------------------------------------*/
