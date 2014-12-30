/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------
  Wetterstation Zwettl
  --------------------


  designed for "Peetbros, Ulitimiter 2000"
  Homepage: http://www.peetbros.com/


  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19981015 OE3DZW fixes over fixes
//19981022 OE3DZW not is running! fine, lets try it
//19981023 OE3DZW its running at oe3xzr, fine
//19981024 OE3DZW better i/o (fewer overruns)
//19990602 OE3DZW removed blkill, was used incorrectly

#include "baycom.h"

#ifdef _WXSTN
#include <math.h>
#include <termios.h>
#include <sys/signal.h>
#include <sys/types.h>

/*---------------------------------------------------------------------------*/

int wx::h4 (char *instr, int posstr)
//*************************************************************************
// instr...input string, contains 4 digit Hex-Number
// posstr..start position of 4 digit hex-nr
// returns hex-nr
//*************************************************************************
{
  char lstr[5];
  int res = 0;

  strncpy(lstr, instr+posstr-1, 4);
  lstr[4] = 0;
  sscanf(lstr, "%X", &res);
  //printf("instr %d %s\nlstr %s res %d\n", posstr, instr, lstr, res);
  return res;
}

/*---------------------------------------------------------------------------*/

char *wx::get_wdir_str (double wind_direction)
//*************************************************************************
//
//*************************************************************************
{
  static char *wdirname[] = { "Err",
                              "N", "NNE", "NE", "ENE",
                              "E", "ESE", "SE", "SSE",
                              "S", "SSW", "SW", "WSW",
                              "W", "WNW", "NW", "NNW"};
  if (wind_direction < 0 || wind_direction > 360)
    return wdirname[0];
  else
    return wdirname[(int)((wind_direction+11.25)/22.5)%16+1];
}

/*---------------------------------------------------------------------------*/

int wx::orgdata (wxdata_t *wx, char *oneline)
//*************************************************************************
//
// calculates raw data values from onelineformat
//
//   3 YYYY = wind speed (0.1km/h)
//   7 zzZZ = wind direction (ZZ 0..255) zz indicates calibration (00/ff)
//  11 OOOO = outdoor temperature (0.1 fahrenheit)
//  15 XXXX = rain, long term total (0.01 inches)
//  19 PPPP = barometer  (0.1 hPa=0.1 mBar)
//  23 IIII = indoor temperature (0.1 fahrenheit)
//  27 WWWW = outdoor humidity  (0.1 per cent)
//  31 RRRR = indoor humidity (0.1 per cent), inactive
//  35 DDDD = day of year (1.jan = 0000)
//  39 MMMM = minute of day (00:00 = 0000)
//  43 YYYY = today's rain total (0.01 inches)
//  47 UUUU = 1 minute wind adverage (0.1km/h)
//     <CR><LF>
//
//              YYYYZZZZOOOOXXXXPPPPIIIIWWWWRRRRDDDDMMMMYYYYUUUU
//            !!0000000F00DC05D7254202CA03AB----0164045D00000000
//            12345678901234567890123456789012345678901234567890
//                     1   2       3       4       5
//*************************************************************************
{
 char name[20];
 strcpy(name, "wx:orgdata");
 double khdif;
 int bad = 0; //line must be incorrect

 // simple check if string is correct
 if (strlen(oneline) != 50 || oneline[0] != '!')
   return 0;

 // measured data
 // using 1 min average here
 wx->windspeed = h4(oneline, 47) / 10.0 / 3.6; // m/s, orginally in 0.1km/h
 wx->wind_direction = h4(oneline, 7); // 0..255
 wx->wind_direction = wx->wind_direction * 360.0/255.0; // 0..360 degrees
 wx->outdoort = h4(oneline, 11) / 10.0; // 0.1fahrenheit
 wx->outdoort = (wx->outdoort - 32.0) / 1.8; // celsius
 wx->rainltt = h4(oneline, 15); // 0.01 inches, long term total
 wx->rainltt = wx->rainltt/2.54; // 1mm
 wx->air_pres_qfe = h4(oneline, 19) / 10.0; // 0.1hPa or 0.1mBar
 wx->indoort = h4(oneline, 23) / 10.0; // 0.1fahrenheit
 wx->indoort = (wx->indoort -32.0) / 1.8; // 1celsius
 wx->humidity = h4(oneline, 27) / 10.0;
 wx->hum_indoor = h4(oneline, 31);
 wx->dayoyear = h4(oneline, 35)+1;
 wx->minuteofday = h4(oneline, 39);
 wx->winddir_str = get_wdir_str(wx->wind_direction);
 wx->t = ad_time();
 // check data
 if (wx->windspeed < 0 || wx->windspeed > 300)
 {
   wx->windspeed = 0;
   bad++;
 }
 if (wx->humidity > 99.0)
 {
   wx->humidity = 99.0; // must be less than 100%
   bad++;
 }
 if (wx->humidity < 0)
 {
   wx->humidity = 0;
   bad++;
 }
 if (wx->outdoort < -50 ||wx->outdoort > 70)
 {
   wx->outdoort = 0;
   bad++;
 }
 if (wx->indoort < -50 || wx->indoort > 70)
 {
   wx->indoort = 0;
   bad++;
 }
 if (wx->air_pres_qfe < 0)
 {
   wx->air_pres_qfe = 0;
   bad++;
 }
 if (bad)
 {
   trace(report, name, "bad: %s", oneline);
   return 0;
 }
 // calculated data

 // wind-chill
 // formulas by DL1ZAJ

 // luftdruck qnh
 wx->air_pres_qnh = wx->air_pres_qfe + (m.wxqthaltitude /
                     (8.7 - (m.wxqthaltitude * 0.0005)));

 // Fühlbare Temperatur (Windchill) in C
 //    FT = (-0.0432 * (SQR(100 * WMS) + 10.45 - WMS) * (33 - T)) + 31.46
 //  adopted: windspeed+1.0 => better results with low speeds,
 //   result must be lower than real temperatur (DerStandard,
 //   28.12.96)
 wx->wind_chill = (-0.0432 * ((double)sqrt(100.0*(wx->windspeed+1.0))+10.45 -
                   (wx->windspeed+1.0))*(33.0-wx->outdoort))+31.46;
 if (wx->wind_chill > wx->outdoort)
   wx->wind_chill = wx->outdoort;

 // Saettigungsdampfdruck
 //   Saettigungsdampfdruck in hPa
 //   IF T > 0 THEN
 //   SDAD = 6.09 * 10 ^ ((7.5 * T) / (237.3 + T))
 //   ELSE
 //   SDAD = 6.09 * 10 ^ ((9.5 * T) / (265.5 + T))
 //   END IF
 if (wx->outdoort > 0)
   wx->sat_sp = 6.09 * (double) exp(((7.5*wx->outdoort)/(273.3 +
                                     wx->outdoort))*log(10.0));
 else
   wx->sat_sp = 6.09 * exp(((9.5*wx->outdoort)/(265.5 +
                             wx->outdoort))*log(10));

 // steam_pressure
 wx->steam_pressure = (wx->humidity * wx->sat_sp) / 100.0;

 // Absolute Feuchte
 //   Absolute Feuchte in Gramm H2O / m3
 //   LDI = QFE / (2.8704 * TKE) Luftdichte in Kilogramm / m3
 //   SPFEU = (623 * DAD) / (QFE - 0.377 * DAD) Spezifische Feuchte
 //                                             in Gramm H2O / Kilogramm
 //   AFEU = SPFEU * LDI
 if (wx->air_pres_qfe > 0)
   wx->abs_humidity = ((623*wx->steam_pressure)/(wx->air_pres_qfe - 0.377 *
                          wx->steam_pressure)) *
                       (wx->air_pres_qfe/(2.8704*(wx->outdoort+273.15)));
 else
   wx->abs_humidity = 0;

 // Taupunkt
 //     Taupunkt in °C
 //     TP = (234.67 * (LOG(DAD) / LOG(10)) - 184.2) /
 //          (8.233 - (LOG(DAD) / LOG(10)))
 if (wx->steam_pressure > 0)
   wx->dew_point = (234.67 * (log(wx->steam_pressure)/log(10))-184.2)/
                         (8.233 - (log(wx->steam_pressure)/log(10)));
 else
   wx->dew_point = 0;

 // Kondensationsnivau
 //     Kondensationsniveau (Wolkenuntergrenze für Cumuluswolken) in Meter über NN
 //     KON = (T + ((SSH - 2) / 100) - TP) * 120
 wx->kond_niveau = (wx->outdoort + ((m.wxsensoraltitude-2.0)/100.0)-
                                     wx->dew_point)*120.0;

 // Schneefallgrenze in m
 //     Schneefallgrenze/Null-Grad-Niveau in KILOMETER über NN
 //     SFG = (T / 10) + (HOEHE / 1000)
 //     KHDIF = (KON - HOEHE) / 100
 //     IF (T * 100) > KON THEN SFG = SFG + (KHDIF / 6) - (KHDIF / 10)
 //     IF SFG < 0 THEN SFG = 0
 //     Anzeige der SFG (in km) in METERN (in Schritten von 10 Metern) :
 //     SFG = CINT(SFG * 100) * 10
 wx->snowfall_limit = (wx->outdoort/10)+(m.wxqthaltitude/1000);
 khdif = (wx->kond_niveau - m.wxqthaltitude) / 100;
 if ((wx->outdoort * 100) > wx->kond_niveau)
   wx->snowfall_limit = wx->snowfall_limit + (khdif/6) - (khdif/10);
 if (wx->snowfall_limit < 0)
   wx->snowfall_limit = 0;
 else // not in 1km units but in 1m units
   wx->snowfall_limit = 1000 * wx->snowfall_limit;

 // Windstaerke in Beaufort
 //     BEAU = CINT(10 ^ ((LOG(((WMS - 0.07) / 0.834) ^ 2) / LOG(10)) / 3))
 //     Different data from "Zentralanstalt für Metereologie und Geodynamik
 //     Wien" used here
 //     newer data from Willy, oe3wyc
                    //0    1    2    3    4    5     6
 double beau_tab[] = {0, 0.3, 1.5, 3.3, 5.4, 7.9, 10.7,
                     //  7     8     9    10    11    12
                      13.8, 17.1, 20.7, 24.4, 28.4, 32.6};
 wx->wind_beaufort = 12; // Maximum
 while (wx->wind_beaufort && beau_tab[wx->wind_beaufort] > wx->windspeed)
   wx->wind_beaufort--;

 wx->data_valid = 1;
 return 1;
}

/*---------------------------------------------------------------------------*/

void wx::show_wx (wxdata_t *wx, int detail)
//*************************************************************************
// Userinterface for wx-data
//  detail:  0..raw format (debugging-output)
//           1..short format
//           2..long (detailed) format
//           3..dl3baa-format
//*************************************************************************
{
  static char *beaunam[] = {
   "Windstille", "leiser Zug", "leichte Briese", "maessige Briese",
   "frische Briese", "starker Wind", "steifer Wind", "stuermischer Wind",
   "Sturm", "schwerer Sturm", "orkanartiger Sturm", "Orkan"};
  if (! wx->data_valid)
  {
    putf("Sorry, no data available.\n");
    return;
  }
  putf("WX-Station %s\n", m.wxstnname);
  switch (detail)
  {
   case 1: //short format
     putf("Aussentemperatur.....%6.1f C\n", wx->outdoort);
     putf("Raumtemperatur.......%6.1f C\n", wx->indoort);
     putf("Luftdruck QNH........%6.1f hPa\n", wx->air_pres_qnh);
     putf("Rel.Luftfeuchte......%6.1f %%\n", wx->humidity);
     putf("Windgeschwindigkeit..%6.1f km/h\n", wx->windspeed*3.6);
     if (wx->windspeed > 0.1) // wind_direction only valid if there is wind
       putf("Windrichtung........%6s\n", wx->winddir_str);
     putf("Messzeit............%s\n", datestr(wx->t,12));
     putf("\nMehr Infos sind mit WX INFO abrufbar!\n");
   break;

   case 2: //detailed format
      putf("Aussentemperatur.......%6.1f C\n", wx->outdoort);
      putf("Windchill-Temp.........%6.1f C\n", wx->wind_chill);
      putf("Taupunkt...............%6.1f C\n", wx->dew_point);
      putf("Schneefallgrenze.......%6.0f m\n", wx->snowfall_limit);
      putf("Raumtemperatur.........%6.1f C\n", wx->indoort);
      putf("Luftdruck QNH..........%6.1f hPa\n", wx->air_pres_qnh);
      putf("Luftdruck lokal QFE....%6.1f hPa\n", wx->air_pres_qfe);
      putf("Saettigungsdampfdruck..%6.1f hPa\n", wx->sat_sp);
      putf("Dampfdruck.............%6.1f hPa\n", wx->steam_pressure);
      putf("Rel.Luftfeuchte........%6.1f %%\n", wx->humidity);
      if (wx->humidity < 100.0)
        //only defined when < 100%
        putf("Absolute Feuchte.......%6.1f g/m3\n", wx->abs_humidity);
      putf("Windgeschwindigkeit....%6.1f m/s = %4.1f km/h (%ld Bf %s)\n",
            wx->windspeed, wx->windspeed*3.6, wx->wind_beaufort,
            beaunam[wx->wind_beaufort]);
      if (wx->windspeed > 0.1)     /*wind-direction only when there is wind*/
        putf("Windrichtung...........%6.1f Grad (%s)\n",
              wx->wind_direction, wx->winddir_str);
      putf("Messzeit...............%s\n", datestr(wx->t, 12));
   break;

   case 3: //special format
/* Format compatible with DC4FS:
============================================================================
Temperatur           :  -9.0 C      15.8 F      konstant
Luftdruck QNH        :   997 hPa    748 mmHg    steigend
relative Feuchte     :    88 %                  konstant
Sonnenscheindauer    :   2.5 h
Niederschlagsmenge   :   0.0 mm     0.0 l/m2
Windgeschwindigkeit  :   0.8 m/s    2.9 km/h    konstant
Windrichtung         :   100 Grad     O         konstant
============================================================================
*/
    putf("Gekuerzte Ausgabe kompatibel zu DC4FS's WX-Software\n");
    putf("Temperatur        :%6.1f C\n", wx->outdoort);
    putf("Luftdruck QNH  :%6.0f hPa\n", wx->air_pres_qnh);
    putf("relative Feuchte      :%6.0f %%\n", wx->humidity);
    putf("Windgeschwindigkeit   :%6.1f m/s\n", wx->windspeed);
    if (wx->windspeed > 0.1) // wind-direction only when there is wind...
      putf("Windrichtung   :%6.0f Grad\n", wx->wind_direction);
   break;
  }
}

/*---------------------------------------------------------------------------*/

void wx::show_diags (int what, time_t t_n)
//*************************************************************************
//
//*************************************************************************
{
  //show diagram
  int i;
  wxdata_t wxt;
  char oneline[80];
  double temp[48];
  time_t next_t;

  for (i = 0; i < 48; i++)
  {
    next_t = t_n-DAY-400+1800*(i+1);
    if (! read_wx_data(oneline, next_t))
    {
      putf("No data for %s available.\n", datestr(next_t, 12));
      return;
    }
    //printf("oneline %s\n", oneline);
    if (orgdata(&wxt, oneline))
    switch (what)
    {
      case 3:
        temp[i] = wxt.humidity;
        break;

      case 2:
        temp[i] = wxt.air_pres_qnh;
        break;

      case 1:
        temp[i] = wxt.windspeed;
        break;

      default:
        temp[i] = wxt.outdoort;
        break;
    }
    else
    {
      putf("Data invalid.\n");
      return;
    }
  }
  switch (what)
  {
    case 1:
      diag_24h("Windstaerke der letzten 24h",
          "m/s", temp);
      break;

    case 2:
      diag_24h("Luftdruck QNH der letzten 24h",
          "hPa", temp);

      break;

    case 3:
      diag_24h("Luftfeuchtigkeit der letzten 24h",
          "%", temp);

      break;

    default:
      diag_24h("Temperatur der letzten 24h",
           "Grad C",
           temp);
      break;
  }
}

/*---------------------------------------------------------------------------*/

void wx::mbwxcmd (char *befbuf)
//*************************************************************************
// WX command interpreter
//*************************************************************************
{
 static char *beftab[] = {"ALL", "TEMPERATURE", "DRUCK", "PRESSURE",
                          "WINDSPEED", "FEUCHTIGKEIT", "HUMIDITY",
                          "DC4FS", "INFO", "LOOP", NULL};
 enum befnum {unsinn, _all, temperatur, luftdruck, luftdruck_,
                     wind, humidity, humidity_, dc4fs, info,
             _loop
             } cmd = unsinn;
 befbuf += blkill(befbuf);
 cmd = (befnum) readcmd(beftab, &befbuf, 0);
 switch(cmd)
 {
  case unsinn:
    show_wx(&m.wx, 1);
  break;
  case _all:
    if (befbuf[0])
    {
      char oneline[80];
       wxdata_t wxt;
       time_t t1 = atol(befbuf); //simple but works for _any_ date
      if (t1 && read_wx_data(oneline, t1)
             && orgdata(&wxt, oneline))
        show_wx(&wxt, 2);
      else
        putf("No data available.\n");
    }
    else
      show_wx(&m.wx, 2);
  break;
  case _loop:
    while (1)
    {
      show_wx(&m.wx, 1);
      putflush();
      wdelay(2000);
    }
  case temperatur:
    show_diags(0, ad_time());
    break;
  case wind:
    show_diags(1, ad_time());
    break;
  case luftdruck:
  case luftdruck_:
    show_diags(2, ad_time());
    break;
  case humidity:
  case humidity_:
    show_diags(3, ad_time());
    break;
  case dc4fs:
    show_wx(&m.wx, 3);
  break;
  case info:
    readtext("wxinfo");
  break;
 }
}

/*---------------------------------------------------------------------------*/

time_t wx::read_wx_data (char *oneline, time_t time_m)
//*************************************************************************
// Reads wx-data which is next to time_m, returns data and
// timestamp of data found.
// returns 0 when no data is found
//*************************************************************************
{
 char name[20];
 int found = 0;
 char times[13];
 char data[80];
 time_t times_l;
 long line = 0;

 strcpy(name, "wx:read_wx_data");
 FILE *wxf = s_fopen(time2wxfn(time_m), "srt");
 if (! wxf)
 {
   trace(report, name, "fopen %s err", time2wxfn(time_m));
   return 0L;
 }
 while (! found && ! feof(wxf))
 {
   fgets(data, 80, wxf);
   line++;
   rm_crlf(data);
   if (strlen(data) != 70)
   {
     trace(replog, name, "%s line %ld len %d invalid",
                   time2wxfn(time_m), line, strlen(data));
     s_fclose(wxf);
     return 0L;
   }
   //extract time stamp
   strncpy(times, data+57, 12);
   times[12] = 0;
   times_l = atol(times);
   //printf("%ld %ld %s\n",line,times_l,datestr(times_l,12));
   if (times_l>=time_m-360 && //max. 6min in advance (midnight)
       times_l< time_m+3960)  //max. 1h 6min past (daylightsaving)
     found++;
 }
 s_fclose(wxf);
 if (! found)
 {
   trace(report, name, "data missing");
   return 0L;
 }
 else
 {
   data[56] = 0;
   strcpy(oneline, data+6);
   return times_l;
 }
}

/*---------------------------------------------------------------------------*/

int wx::configure_tty (char *wxtty)
//*************************************************************************
/* configures tty, returns file descriptor on success */
//*************************************************************************
{
  char name[20];
  strcpy(name, "wx:configure tty");
#ifdef DUMMYWX
  trace(report, name, "using dummy wx-station");
  return 1;
#else
  int fd;
  struct termios tio; //terminal input/output controls

  //open serial line, no locking is performed/checked
  fd=open(wxtty, O_RDWR, O_NOCTTY, O_NONBLOCK);
  if (fd < 0)
    trace(replog, name, "can not open %s", wxtty);
  else
  {
    //configure serial line
    //see "man termios" for details

    memset(&tio, 0, sizeof(tio));
    //control mode flags, eg. speed of serial line
    tio.c_cflag = B2400|CRTSCTS|CS8|CLOCAL|CREAD;
    //input mode flags
    tio.c_iflag = IGNPAR;
    //output mode flags
    tio.c_oflag = 0;
    //local mode flags (no echo, non canonical..)
    tio.c_lflag = 0;

    //read should not wait (block)

    //inter character timer (*0.1s)
    tio.c_cc[VTIME] = 0;
    //blocking read until .. characters received
    tio.c_cc[VMIN] = 1;

    //remove existing data from queue
    tcflush(fd, TCIFLUSH);

    //set new settings,old settings are not saved (exclusive port)
    tcsetattr(fd, TCSANOW, &tio);

    trace(replog, name, "open %s ok", wxtty);

  }
  return fd;
#endif
}

/*---------------------------------------------------------------------------*/

void wx::close_tty (int fd)
//*************************************************************************
// close tty
//*************************************************************************
{
  close(fd);
  trace(report, "debug", "tty closed");
}

/*---------------------------------------------------------------------------*/

char *wx::time2wxfn (time_t t)
//*************************************************************************
//
//*************************************************************************
{
  static char wxfn[FNAMELEN+1];
  struct tm *tt = ad_comtime(t);
  sprintf(wxfn, "%s/wx%02d%02d%02d.dat", m.wxpath,
                tt->tm_mday, tt->tm_mon+1, tt->tm_year%100);
  return wxfn;
}

/*---------------------------------------------------------------------------*/

void wx::savedata (char *rawdata, wxdata_t *wx)
//*************************************************************************
//file-format: wx-data:
//
//filename: directory: 'wx'
//          filename:  'wx251296.dat' 96..year,12..month,25..day
//
//         1       2       3       4       5       6       7
//1234567890123456789012345678901234567890123456789012345678901234567890
//007200!!00000271003C0000255F02B503E8----0167044800000000-000851548504Y
//      !!0000000F00DC05D7254202CA03AB----0164045D00000000
//      12345678901234567890123456789012345678901234567890
//               1         2       3       4       5
//vvvvvxwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww-ttttttttttttc
//12345                                               123456789012
//v...Version of Software,5digits  as defined in header, defining file-format
//    (numversion)
//x...Type of line: '0': data
//                  '1': comment
//                  else: undefined
//t...time of System (not WX-station) in Unix-Format
//w...oneline-wx-data, as received from wx-station
//c...simple check sum of string  (mod 64)+32
//-...Error in privious versions, inserted "-" for compatibility
//
//*************************************************************************
{
#define DASH "-"
#define DATA_TYPE "0"
 char name[20];
 strcpy(name, "wx:savedata");
 char wxfilen[FNAMELEN+1];
 FILE *wxfile;
 char filestr[200];
 int i;
 long csum;

 // create wxpath
 strcpy(wxfilen, time2wxfn(wx->t));
 // open file if existing, else create it
 wxfile = s_fopen(wxfilen, "sat");
 if (! wxfile)
   wxfile = s_fopen(wxfilen, "swt");
 if (! wxfile)
 {
   trace(serious, name, "create %s", wxfile);
   return;
 }
 trace(report, name, "store to %s", wxfilen);
 sprintf(filestr, WXNUMVERSION""DATA_TYPE"%s"DASH"%012ld",
                  rawdata, wx->t);
 csum = 0;
 for (i=0; filestr[i]; i++)
 {
   csum += filestr[i];
 }
 csum = (csum%64)+32; //this check is stupid, but necassary for compatiblity
 sprintf(filestr+strlen(filestr), "%c\n", (int) csum);
 fputs(filestr, wxfile);
 s_fclose(wxfile);
}

/*---------------------------------------------------------------------------*/

void wx::diag_24h (char *title, char *yunit, double f[48])
//*************************************************************************
//
//*************************************************************************
{
  double min = f[0];
  double max = f[0];
  long imin, step;
  int hour, line;
  int i;

  for (i=1; i<48; i++)
  {
    if (f[i] < min)
      min = (long int) f[i];
    if (f[i] > max)
      max = (long int) f[i];
  }
  if (max - min < 10)
  {
    imin = (long int) (max+min)/2-5;
    step = 1;
  }
  else
  {
    imin = (long int) min-1;
    step = (long int) (max-min+1)/9 +1;
  }
  putf(" %s\n ",title);
  for (i = 0 ; title[i] ; i++) // line of dashes as long as title
    putf("-");
  putf("\n");
  putf("     %s\n",yunit);
  for (line = 0 ; line <= 10 ; line++)
  {
    long int nom = imin+step*(10-line);
    putf("   %5d |", nom);
    for (hour = 0; hour<48; hour++)
    {
      if (f[hour] >= nom && f[hour] < (nom+step))
        putf("-");
      else
        putf(" ");
    }
      putf("\n");
  }
  putf(
  "       ------------------------------------------------>\n");
  putf(
  "              2                 1                 Stunden\n");
  putf(
  " gestern   3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 heute\n");
  putf("\n");
}

/*---------------------------------------------------------------------------*/

int wx::read_tty (int fd, char *rawdata, int rawdatalen)
//*************************************************************************
// receives data from tty, returns 0 when no data is available
//*************************************************************************
{
#ifdef DUMMYWX
  strcpy(rawdata, //"!!0000000F00DC05D7254202CA03AB----0164045D00000000\n");
                    "!!00000018020A2545258402850326----011C038500000000\n");
  //printf("read_tty\n");
  return strlen(rawdata);
#else
  int len = 0;
  fd_set sfds;
  FD_ZERO(&sfds);
  FD_SET(fd, &sfds);
  if (select(fd+1, &sfds, NULL, NULL, NULL) == 1)
  {
    len=read(fd, rawdata, rawdatalen);
    rawdata[len] = 0;
  }

  if (len && m.wxstnname[0] == '_') //simple hack to get debug output
    trace(report, "wx", "%s", rawdata);
  return len;

#endif
}

/*---------------------------------------------------------------------------*/

void wx::mbwxsave (char *taskname)
//*************************************************************************
// this task reads wx-data from serial line, saves it in intervals
// of 5min and puts current + recent (-1h) wx-data into global
// variable.
//*************************************************************************
{
  char name[20];
  strcpy(name, "wx:mbwxsave");
  time_t nextsavet = ad_time();
  int fd;
  char rawdata[80];
  long total_num = 0; // total number of measurements
  long comerr = 0; // total number of communication errors
  char lastcmd_s[50];
  int orgok;

  lastcmd("init");
  // configure tty device
  fd = configure_tty(m.wxtty);
  if (fd < 0)
    return;
  trace(replog, name, "WX logging started");
  // initialize wx-structure
  memset(&m.wx, 0, sizeof(m.wx));
  m.wx.data_valid = 0;
  // wait for data
  while (! runterfahren)
  {
    rawdata[0]=0;
    do
    {
      wdelay(50);
      while (! read_tty(fd, rawdata+strlen(rawdata), 79-strlen(rawdata)))
      {
        wdelay(50);
        // if no data received for >1min, old data is no longer valid
        if (m.wx.data_valid && ad_time()-m.wx.t > 600)
        {
          m.wx.data_valid = 0;
          lastcmd("no data");
          trace(report, name, "no data received");
        }
      }
      if (m.wxstnname[0] == '_')  //simple hack to get debug output
        trace(report, "wx", "rawdata: \"%s\"", rawdata);
      //correct string starts with !!...
      if (strlen(rawdata) > 2 && rawdata[0] != '!')
      {
        char *a = strstr(rawdata, "!!");
        if (a)
          memmove(rawdata, a, strlen(a)+1);
        else
        {
          rawdata[0]=0;
          comerr++;
        }
      }
    }
    // wait until line is complete (or buffer full)
    while(strlen(rawdata) < 70 && ! strstr(rawdata, "\n") &&
                                  ! strstr(rawdata, "\r"));
    rm_crlf(rawdata);
    //calculate data, and if correct test if data should be saved
    orgok = orgdata(&m.wx, rawdata);
    if (orgok)
      comerr++;
    else
      total_num++;
    sprintf(lastcmd_s, "data %ld/%ld", total_num, comerr);
    lastcmd(lastcmd_s);
    if (orgok && nextsavet <= m.wx.t)
    {
      if (nextsavet < ad_time())
        nextsavet = ad_time(); // if time is changed
      nextsavet += 300; //next time to save data is in 5min
      savedata(rawdata, &m.wx);
    }
  }
  close_tty(fd);
}

/*---------------------------------------------------------------------------*/
#endif
/*---------------------------------------------------------------------------*/

