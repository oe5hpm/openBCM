/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  --------------------
  Wetterstation Zwettl
  --------------------


  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

#ifdef _WXSTN
class wx
{

private:
// version number compatible to old dos-wx-programme
#define WXNUMVERSION "01700"
//methods
char *get_wdir_str(double winddirection);
int orgdata(wxdata_t *wx, char *oneline);
int h4(char *instr, int posstr);
void show_wx(wxdata_t *wx, int detail);
void show_diags(int what, time_t t_n);
void diag_24h(char *title, char *yunit, double f[24]);
int read_tty(int fd, char *rawdata, int rawdatalen);
int configure_tty(char *wxtty);
void close_tty(int fd);
void savedata(char *rawdata, wxdata_t *wx);
char *time2wxfn(time_t t);
time_t read_wx_data(char *oneline, time_t time_m);

public:
void mbwxcmd(char *befbuf);
void mbwxsave(char *taskname);

};
#endif

/*---------------------------------------------------------------------------*/
