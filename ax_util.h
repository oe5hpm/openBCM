/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------------------------------
  Deklarationen fuer AX.25-Konvertierungsroutinen
  -----------------------------------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

void    putcall (byte *rufz, int stern);
void    putclist (byte *af);
int     readcall (char *bf, byte *axcall);
int     readclist (char *bf, byte *axlist, int maxanzahl);
void    callcopy (byte *, byte *);
int     callcmp (byte *, byte *);
int     callcmp1 (byte *, byte *);
int     callwildcmp (byte *call1, byte *call2);
void    callasc (byte *rufz, char *buf);
void    onlycall (byte *rufz, char *buf);
int     callord (byte *call1, byte *call2, int wildcard);
int     callok (byte *call);
int     ischannel (byte *call);

