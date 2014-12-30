/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ---------------------------
  YAPP-Protocol (Definitions)
  ---------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

typedef enum yapp_state_t
{ YAPP_STATE_R,
  YAPP_STATE_RH,
  YAPP_STATE_RD,
  YAPP_STATE_S,
  YAPP_STATE_SH,
  YAPP_STATE_SD,
  YAPP_STATE_SE,
  YAPP_STATE_ST
} yapp_state_t;

/*---------------------------------------------------------------------------*/

typedef enum yapp_packet_t
{ YAPP_PACKET_UNKNOWN,
  YAPP_PACKET_RR,
  YAPP_PACKET_RF,
  YAPP_PACKET_RT,
  YAPP_PACKET_AF,
  YAPP_PACKET_AT,
  YAPP_PACKET_SI,
  YAPP_PACKET_HD,
  YAPP_PACKET_DT,
  YAPP_PACKET_EF,
  YAPP_PACKET_ET,
  YAPP_PACKET_NR,
  YAPP_PACKET_RE,
  YAPP_PACKET_CN,
  YAPP_PACKET_CA
} yapp_packet_t;

/*---------------------------------------------------------------------------*/

class fileio_yapp : public fileio
{

private:
  void tx_RR(void);
  void tx_RF(void);
  void tx_RT(void);
  void tx_AF(void);
  void tx_AT(void);
  void tx_RE(long int, char);
  void tx_SI(void);
  void tx_CN(char *);
  void tx_CA(void);
  void tx_HD(void);
  void tx_ET(void);
  void tx_DT(int);
  void tx_EF(void);
  void wait_CA(void);
  yapp_packet_t YAPP_packet(char *);
  void YAPP_unk(char *, char *);
  char yappc;
  yapp_state_t state;
  yapp_packet_t packettype;

protected:
  virtual void _rx(void);
  virtual void _tx(void);

public:
  fileio_yapp(void);

};

/*---------------------------------------------------------------------------*/
