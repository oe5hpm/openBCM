/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------------------
  Definitionen und Deklarationen fuer Mailbox
  -------------------------------------------

  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

// FBB-commands
#define FBB_FINE        "FF"
#define FBB_RECV_FINE    97
#define FBB_QUIT        "FQ"
#define FBB_RECV_QUIT    98
#define FBB_PROMPT      "F>"
#define FBB_PROPOSE     "FB" //old text mode or
                             //binary compressed file ("autobin")
#define FBB_COMPPROP    "FA" //ascii compressed mail
#define D_PROPOSE       "FD" //dl8hbs compressed mail
#define FBB_STATUS      "FS"
#define FBB_RECV_ERROR   99

// FBB-status-characters
#define FBB_NO          '-' // used in binary compressed forward version 0
#define FBB_NO_         'N' // used in binary compressed forward version 1
#define FBB_YES         '+' // used in binary compressed forward version 0
#define FBB_YES_        'Y' // used in binary compressed forward version 1
#define FBB_LATER       '=' // used in binary compressed forward version 0
#define FBB_LATER_      'L' // used in binary compressed forward version 1
#define FBB_HOLD        'H'
#define FBB_REJECT      'R'
#define FBB_ERROR       'E'
#define FBB_RESUME      '!' // used in binary compressed forward version 0
#define FBB_RESUME_     'A' // used in binary compressed forward version 1

#define FBB_MAX_PROP     5
#define SIGNATURSTR     "FBBFWDBCM"
#define MAXCOMPRFILE 50     //max nr of mail fragments per fwdpartner to store
#define FBBHSIGN        "FBBHSIGN" //signature of fbb-fragment cache
#define FBBHVERS        "001"      //version id of fragment cache

/*---------------------------------------------------------------------------*/

enum fbb_fwdstate_t
{ fbb_send_propose,
  fbb_send_mail,
  fbb_send_delay,
  fbb_recv_propose,
  fbb_recv_mail,
  fbb_end,
  fbb_error
};

/*---------------------------------------------------------------------------*/

typedef struct
{ char signature[11];
  char version[4];
  char bid[13];
  char reserved[88];
  char signatur2[11];
} header_t;

typedef struct
{ char proptxt[FBB_MAX_PROP][100];        // line 1:1 of proposal
  int fragment[FBB_MAX_PROP];             // index of fragment-filename
  unsigned long int offset[FBB_MAX_PROP]; // offset, already received
} want_status_t;

/*---------------------------------------------------------------------------*/

class fbb
{
private:
  // Data
  char signature[sizeof(SIGNATURSTR)];
  crcthp *mail_crcthp; // These are used when sending
  crcfbb *mail_crcfbb; // or receiving a mail in FBB
  char mail_checksum;  // mode (covering header, body, etc.)
  // Methods
  char tx_proposal(char *, char *);
  void tx_yapp_header(void);
  void tx_yapp_data(char *, unsigned short int);
  void tx_yapp_end(void);
  char tx_onemail(void);
  char tx_mails(char *, char, char *);
  void erase_forwarded(char, char *, char *);
  char parse_proposal(char *);
  unsigned long int check_fragment(unsigned long &, int &);
  char reply_proposal(want_status_t *);
  char rx_yapp_header(void);
  char rx_yapp_data(char *, unsigned short int &, char);
  char rx_yapp_end(void);
  char rx_mails(char, want_status_t *);

public:
  fbb(void);
  ~fbb(void);
  void fwdmachine(char send);
};

/*---------------------------------------------------------------------------*/
