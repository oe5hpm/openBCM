/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ------------------------------
  Filesurf for BCM (Definitions)
  ------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#ifdef FILESURF

/*---------------------------------------------------------------------------*/

class filesurf
{

private:
  void putwelcome(void);
  void printallowed(void);
  int pathallowed(char *, int);
  int pathexists(char *);
  int showdir(char *, bitfeld);
  void processpath(char *);
  int addpath(char *, char *);
  void putsyntax(char *, char *);
  char sysopcmd(char *);

public:
  void execute(char *);
  void putprompt(void);
  void getfirstpath(char *);

};

/*---------------------------------------------------------------------------*/

#endif
