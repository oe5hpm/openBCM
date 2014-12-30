/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  BayCom-Terminal
  BayCom-Node
  BayCom-Mailbox

  Routinen fuer Mailboxsystem


  --------------------------------------
  DIDADIT-Protocol for BCM (Definitions)
  --------------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

#define DIDADIT_VERSION "0.9.1"

#define DEFAULT_BLOCKSIZE 4096
#define MAX_BLOCKSIZE 8192

/*---------------------------------------------------------------------------*/

// KISS-Konstanten
#define FEND    0xC0    // FrameEND
#define FESC    0xDB    // FrameESCape
#define TFEND   0xDC    // Transponiertes FrameEND
#define TFESC   0xDD    // Transponiertes FrameESCape

/*---------------------------------------------------------------------------*/

#define R_BLOCK_INFO    1
#define R_BLOCK_START   2
#define R_BLOCK_ERR     3
#define R_BLOCK_DATA    4

// These are the codes accordings to spec 0.9.1
//#define R_BLOCK_REQ   5
//#define R_BLOCK_FIN   6

// These are the codes required by the current LinKT :-(((
#define R_BLOCK_REQ     6
#define R_BLOCK_FIN     5

#define R_BLOCK_FINACK  7
#define R_BLOCK_ECHOREQ 9
#define R_BLOCK_ECHOREP 10
#define R_BLOCK_ABORT   11
#define R_BLOCK_CHAT    12

/*---------------------------------------------------------------------------*/

#define R_RX_STATE_TYPE         1
#define R_RX_STATE_MD5HASH      2
#define R_RX_STATE_DATA         3
#define R_RX_STATE_INVALID      100

/*---------------------------------------------------------------------------*/

struct block_info
{
  unsigned short int type;      // Block type
  unsigned short int maxlen;    // Number of allocated bytes for the block
  char md5hash[16];             // The MD5 hash (if included in the R block)
  char *data;                   // Pointer to block data
  unsigned short int len;       // Length of received block
};

/*---------------------------------------------------------------------------*/

struct REQ_info
{
  unsigned long int offset;
  unsigned long int len;
};

/*---------------------------------------------------------------------------*/

class fileio_dida : public fileio
{

private:
  void tx_stuffed(char);
  char rx_stuffed(void);
  void tx_block(unsigned short int, char, char *, unsigned long int);
  void tx_block_buggy(unsigned short int, char, char *, unsigned short int); // TODO!!
  void rx_block(void);
  void alloc_blockmem(unsigned short int);
  void free_blockmem(void);
  char *parse_textblock(char *, char **, char **);
  void parse_INFO(char *, unsigned short int *rxed_blocksize);
  void parse_START(unsigned long int *, char *, unsigned short int *);
  void tx_INFO(void);
  void tx_START(unsigned long int, char *, unsigned short int);
  void tx_ERR(char *);
  void tx_DATA(unsigned long int, unsigned short int, char *);
  void tx_FIN(char *);
  void tx_REQ(struct REQ_info *, unsigned long int);
  void tx_FINACK(void);
  void tx_TYPEREJ(unsigned short int);
  void tx_ECHOREQ(void);
  void tx_ECHOREP(void);
  void tx_ABORT(void);
  void send_file_data(unsigned long int, unsigned long int,
                      unsigned long int);
  void update_r_file(char *, char *, unsigned long int, struct REQ_info *);
  char partner_ver[10];
  unsigned short int blocksize;
  struct block_info *bl;
  int stuffresult;
  int waitforheader;
  char sendname[FSPATHLEN + FNAMELEN + 1];

protected:
  void _rx(void);
  void _tx(void);

public:
  fileio_dida(void);
  ~fileio_dida(void);
  void set_path(const char *path);
  void set_waitforheader(int bo);
  void set_sendname(const char *name);
  void get_filename(char *name);
  unsigned long get_size();
};

/*---------------------------------------------------------------------------*/
