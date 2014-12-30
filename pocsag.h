/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ---------------------
  Pocsag-Server for BCM
  ---------------------


  Copyright (C)  Patrick, DF3VI
  Auf Grundlage des Maillistservers von Jonny, DH3MB

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#ifdef DF3VI_POCSAG

/*---------------------------------------------------------------------------*/

#define pocsag_creategroup      1
#define pocsag_deletegroup      2
#define pocsag_setdescription   3
#define pocsag_setoptions       4
#define pocsag_adduser          6
#define pocsag_deleteuser       7
#define pocsag_subscribe        10
#define pocsag_unsubscribe      11

/*---------------------------------------------------------------------------*/

#define pocsag_seek_entry  1
#define pocsag_seek_group  2
#define pocsag_seek_user   3
#define pocsag_seek_end    4

/*---------------------------------------------------------------------------*/

#define POCSAGSEMA  "pocsag"

class pocsagserv
{
private:
  void parse_listheader(char *line, char **name, char **description);
  char seek_group(FILE *fold, FILE *f, char what, char *parameter, char *user);
  void modify_group(char what, char *listname, char *parameter);
  void list_groups(void);
  void show_info(char *listname, char longformat);

public:
  void execute(char *command);
  void examine_mail(void);
};

/*---------------------------------------------------------------------------*/

#endif
