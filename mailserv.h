/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ----------------------------------------
  Mailinglist-Server for BCM (Definitions)
  ----------------------------------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

#ifdef MAILSERVER

/*---------------------------------------------------------------------------*/

#define ma_creategroup      1
#define ma_deletegroup      2
#define ma_setdescription   3
#define ma_setoptions       4
#define ma_setnumber        5
#define ma_adduser          6
#define ma_deleteuser       7
#define ma_addmaintainer    8
#define ma_deletemaintainer 9
#define ma_subscribe        10
#define ma_unsubscribe      11

/*---------------------------------------------------------------------------*/

#define ma_seek_list 1
#define ma_seek_user 2
#define ma_seek_end  3

/*---------------------------------------------------------------------------*/

#define MSMODSEMA "msmod"

class mailserv
{
private:
  void parse_listheader(char *line, char **name, char **number,
                                    char **options, char **description);
  char seek_group(FILE *fold, FILE *f, char what, char *parameter);
  void modify_group(char what, char *listname, char *parameter);
  void list_groups(void);
  void show_info(char *listname, char longformat);

public:
  void execute(char *command);
  void examine_mail(void);
};

/*---------------------------------------------------------------------------*/

#endif
