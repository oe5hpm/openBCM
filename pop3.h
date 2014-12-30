/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ------------
  POP3 for BCM   Reference: RFC 1725 (Definitions)
  ------------


  Copyright (C)       Deti Fliegl, deti@lrz.de, DG9MHZ
                      Guardinistr. 47
                      81375 Muenchen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//20000111 OE3DZW fixed size of pass-variable, needed by APOP

#ifdef __FLAT__

#define MAX_RCPTCNT 100

#ifdef __UNIX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#ifdef _WIN32
#include "winsock.h"
#endif

/*---------------------------------------------------------------------------*/

class pop3
{
public:
  void start_pop3 (void);
  void show_popdb (void);
  void onkill (void);
private:
  void add_to_popdb (void);
  int search_msg (int msg_);
  int init_list (void);
  int put_list (int mode);
  int put_uidl (int msg_);
  void retr_msg (int msg_);
  int dele_msg (int msg_);
  char *extract_argument ();
  int extract_2argument (char *s, int size, char *s2, int size2);
  FILE *fi;
  char lockfile[42];
  char *tmp;
  char *args;
  int finish;
  int badcmd;
  int login;
  int msgcnt;
  int bytecnt;
  char user[CALLEN+2]; //NEEDS TO BE TWO CHARACTERS LONGER (why ever)
  char pass[34]; //DO NOT CHANGE, NEEDED BY APOP (info by dh3ww)
  int msg;
  int waslocked;
//dh3ww begin
  char md5result[33];
  char md5pwstring[13];
//dh3ww ende
};
#endif

/*---------------------------------------------------------------------------*/
