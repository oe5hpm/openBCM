/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  Layer1-Driver


  ---------------------------------------------------
  Definitions / Declarations for whole BAYCOM-Project
  ---------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/


#include "baycom.h"
#include "l2_appl.h"
#include "l2host.h"

/**************************** Interface *******************************/

/*---------------------------------------------------------------------------*/

/* service interface functions */
void f_tnc_flush (void)                 { }
void f_tnc_put (byte a)                 { }
int  f_tnc_putfree (void)               { return 255; }
int  f_tnc_get (void)                   { return EOF; }
int  f_tnc_get_msg (void)               { return EOF; }
void f_tnc_connect (char *mycall, char *call) { }
void f_tnc_disconnect (int mode)        { }
void f_tnc_exit (void)                  { }
int  f_tnc_init (int maxstreams)        { return 0; }

/*---------------------------------------------------------------------------*/

/* management functions, GET-operation */
char far *f_get_partner (void)          { return NULL; }
lstate_t f_get_lstate (void)            { return disconnected; }
char *f_get_mycall (void)               { return NULL; }
int  f_get_outstanding (void)           { return 0; }
int  f_get_retries (void)               { return 0; }
int  f_get_l1state (void)               { return 0; }
int  f_get_channel (void)               { return 0; }
int  f_get_outgoing (void)              { return 0; }
int  f_get_frack (void)                 { return 40; }
unsigned f_get_logintime (void)         { return 0; }
char *f_get_concall (void)              { return NULL; }

/*---------------------------------------------------------------------------*/

/* management functions, SET-operation */
void f_set_partner (char *call)                 { }
void f_set_conok (int on)                       { }
void f_set_messages (int on)                    { }
void f_set_channel (int channel)                { }
void f_set_qsonum (int qsonum,int lstate)       { }
int  f_get_qsonum (void)                        { return 0; }
void f_set_stream (int stream_number)           { }
void f_set_monitor (char *s)                    { }
char *f_get_monitor (void)                      { return NULL; }
void f_set_l1_txdelay (int channel,int txd)     { }
int  f_get_l1_txdelay (int channel)             { return 0; }
void f_set_l1_mode (int channel,char *mm)       { }
char *f_get_l1_mode (int channel)               { return ""; }
void f_set_l1_xmitok (int channel,int xmitok)   { }
int  f_get_l1_xmitok (int channel)              { return (-1); }
char *f_get_version (void)                      { return NULL; }

