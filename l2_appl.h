/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -----------------------------------------------
  Definitions / Declarations L2 Service Interface
  -----------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/



/*************************** Data Types *******************************/

#ifndef _L2_APPL_H
#define _L2_APPL_H

typedef enum lstate_t    // Linkstate according AX.25 Version 2
{ disconnected,
  link_setup,
  frame_reject,
  disc_request,
  info_transfer,
  rej_sent,
  wait_ack
} lstate_t;


typedef enum linkmsg_t
{ msg_nomessage,
  msg_connected,
  msg_disconnected,
  msg_failure,
  msg_frmr,
  msg_busy,
  msg_conrequest
} linkmsg_t;


typedef struct mheard_t
{ byte     call[7];
  byte     viacall[7];
  unsigned frames;
  long     rtime;
  byte     kanal;
} mheard_t;

/**************************** Interface *******************************/

/* service interface functions */


int tnc_init(unsigned int maxstreams);
/* setup of tnc connection. Returns maximum of handled streams */

void tnc_flush(void);
/* pack l2-data and send it, if possible */

//void tnc_putf(char *format,...); // not used anywhere
/* put formatted data to selected stream, like printf */

void tnc_put(byte a);
/* put one character to selected stream */

int  tnc_putfree(void);
/* return free bytes to send */

int  tnc_get(void);
/* get one character. Returns EOF if nothing present */

int  tnc_get_msg(void);
/* get one character of next message. Resturns EOF if no messages has to be
   processed */

int  tnc_getvalid(void);
/* returns, if RX-data is present. RX-Data is not fetched from RX-buffer */

void tnc_connect(char *mycall,char *call);
/* establishes a connection */

void tnc_disconnect(int mode);
/* closes a connection
   mode 0: hard disconnect
        1: normal disconnect
        2: quit, wait for data acknowledged
        5: disconnect all streams */

void tnc_dobeacon(void);

/* management functions, GET-operation */
char *get_partner(void);
lstate_t get_lstate(void);
int get_frack(void);
int get_outstanding(void);
int get_retries(void);
unsigned int get_channel(void);
int get_outgoing(void);

int get_l1state(void);
#define CH_TXB 0x80
#define CH_RXB 0x40
#define CH_PTT 0x20
#define CH_DCD 0x10
#define CH_FDX 0x08
#define CH_SLR 0x04

char *get_mycall(void);
char *get_concall(void);
unsigned get_logintime(void);

int get_free(void);
unsigned int get_channels(void);
int get_connects(unsigned int channel);

char *get_l1_mode(unsigned int channel);
int get_l1_txdelay(unsigned int channel);
int get_l1_xmitok(unsigned int channel);
char *get_monitor(void);
int get_qsonum(void);
char *get_version(void);

mheard_t *get_mhlist(void);
long get_ticker(void);

/* management functions, SET-operation */
void set_partner(char *call);
void set_mycall(char *calls);
void set_conok(int on);
void set_messages(int on);
void set_channel(unsigned int channel);
void set_stream(unsigned int stream_number);
void set_l1_mode(unsigned int channel,char *str);
void set_l1_txdelay(unsigned int channel,int txd);
void set_l1_xmitok(unsigned int channel,int xmitok);
void set_monitor(char *s);
void set_qsonum(int qsonum,int lstate);
void set_paclen(unsigned short paclen);
#endif
