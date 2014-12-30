/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------------
  diverse Zeit - Routinen
  -----------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW added ad_timezone()
//                added DOS-timezone instead of utcoffset
//20000130 OE3DZW time zone was not correct under Linux when TZ was set

#include "baycom.h"

//#define UTC_TIME      //for testing UTC output

/*---------------------------------------------------------------------------*/

struct tm *ad_comtime (time_t timeu)
{
  return localtime(&timeu);
}

/*---------------------------------------------------------------------------*/

time_t ad_mktime (struct tm *tt)
{
  return mktime(tt);
}

/*---------------------------------------------------------------------------*/

time_t loctime (void)
//*************************************************************************
//
//  Gibt Zeit im ANSI-Format zurueck (Sekunden seit 1970-01-01).
//  Haeufige Aufrufe werden nur dann zur Software durchgereicht,
//  wenn die Sekunde tatsaechlich umgeschlagen hat.
//
//  Hat sich die Zeit nach unten veraendert, so wird die Differenz
//  zur bisherigen Zeit gebildet und die tatsaechliche Zeit sukzessive
//  angenaehert.
//
//*************************************************************************
{
  static long lasttic = 0;
  static time_t time_correct = 0;
  static time_t last_decrement = 0;
  static time_t last_time = time(NULL);
  time_t newvalue;

  // mittels schnellen Ticker-Vergleich feststellen, ob sich die
  // Zeit veraendert hat.
  if (lasttic < (systic + (1000 / MS_PER_TIC) - 1) || lasttic > systic)
  {
    lasttic = systic;
    newvalue = time(NULL);
    // ist die Uhr zurueckgelaufen?
    if (newvalue < last_time)
    {
      // Differenz merken
      time_correct = last_time - newvalue;
      last_decrement = newvalue;
    }
    // wurde die Uhr vorgestellt aber es ist noch eine Differenz aktiv?
    if (newvalue > (last_time + 1))
    {
      if (time_correct > (newvalue - last_time + 1))
        time_correct -= (newvalue - last_time);
      else
        time_correct = 0;
      last_decrement = newvalue;
    }
    // ist eine Differenz aktiv?
    if (time_correct > 0  && newvalue > (last_decrement + 1))
    {
      // alle 2 Sekunden die Differenz um eine Sekunde verringern
      time_correct--;
      last_decrement = newvalue;
    }
    last_time = newvalue;
  }
  return last_time + time_correct;
}

/*---------------------------------------------------------------------------*/

#ifndef __LINUX__
time_t isdst (void)
//****************************************************************************
//
// returns daylight-saving in seconds depending on summertime
// used in WIN32 and DOS16
//
//****************************************************************************
{
  time_t ti = loctime();
  struct tm *tt;
  tt = ad_comtime(ti);
  if (tt->tm_mon > 2 && tt->tm_mon < 9)
    return ((time_t) m.summertime * 3600);
  else
  {
    if (   tt->tm_mon == 2  // March
        && tt->tm_mday > 23
        && (31 - tt->tm_mday) <= (7 - tt->tm_wday)
        && (tt->tm_wday || tt->tm_hour >= 2))
      return ((time_t) m.summertime * 3600);
    if (   tt->tm_mon == 9  //October
        && (   tt->tm_mday < 22
            || (30 - tt->tm_mday) > (7 - tt->tm_wday)
            || (! tt->tm_wday && tt->tm_hour <= 3)))
      return ((time_t) m.summertime * 3600);
  }
  return (time_t) 0L; // winter time
}
#endif

/*---------------------------------------------------------------------------*/

time_t ad_timezone (void)
//****************************************************************************
//
// returns offset between utc and local time in seconds
// ad_timezone() depends of setting the environment variable TZ
//
//****************************************************************************
{
#ifdef __FLAT__
#ifdef __LINUX__
  struct tm *tt;
  time_t ut = loctime(); //bcm local time
  tt = ad_comtime(ut);
  return tt->tm_gmtoff;
#else //_WIN32
  return (isdst() - _timezone);
//  _daylight   Nonzero if daylight saving time (DST) zone is specified in TZ;
//              otherwise, 0. Default value is 1.
//  _timezone   Difference in seconds between coordinated universal time and
//              local time. Default value is 28,800 (=8h).
//  _tzname[0]  Time-zone name derived from TZ environment variable.
//  _tzname[1]  DST zone name derived from TZ environment variable. Default
//              value is PDT (Pacific daylight time -8h).
//              If DST zone is omitted from TZ, _tzname[1] is empty string.
#endif
#else //__DOS16__
  if (m.pcisutc)
    return (time_t) 0L;
  else
    return (isdst() + m.stimeoffset * 3600);
#endif
}

/*---------------------------------------------------------------------------*/

time_t ad_time (void) // dh6bb
//****************************************************************************
//
// make UTC, Referenz for all internal ANSI times of bcm
// like (loctime() - ad_timezone())
//
//****************************************************************************
{
  time_t ut = loctime(); //bcm local time
#ifdef __FLAT__
#ifdef __LINUX__
  struct tm *tt;
  tt = ad_comtime(ut);
  return (ut - tt->tm_gmtoff);
#else //_WIN32
  return (ut - (isdst() - _timezone));
//  _daylight   Nonzero if daylight saving time (DST) zone is specified in TZ;
//              otherwise, 0. Default value is 1.
//  _timezone   Difference in seconds between coordinated universal time and
//              local time. Default value is 28,800.
//  _tzname[0]  Time-zone name derived from TZ environment variable.
//  _tzname[1]  DST zone name derived from TZ environment variable. Default
//              value is PDT (Pacific daylight time).
//              If DST zone is omitted from TZ, _tzname[1] is empty string.
#endif
#else //__DOS16__
  if (m.pcisutc)
    return (ut - ((time_t) 0L));
  else
    return (ut - (isdst() + m.stimeoffset * 3600));
#endif
}

/*---------------------------------------------------------------------------*/

unsigned ad_minutes (void)
{
  time_t ti = ad_time();
  struct tm *tt;
  tt = ad_comtime(ti);
  return 60 * tt->tm_hour + tt->tm_min;
}

/*---------------------------------------------------------------------------*/

char *datestr (time_t utime, int ymode) //dk2ui
//*************************************************************************
//
//  Legt das Datum in einem String ab
//
//  0 .. without year, used for check, list     03.11
//  1 .. 2 digit year, (for compatiblity)       03.11.00
//  2 .. 4 digit-year, (where possible)         03.11.2000
//  3 .. hour and minute                        15:09z
//  4 .. hour, minute and seconds               15:09:17z
//  5 .. 2 digit-year, used for R:-header       001103/1509
//  6 .. 4 digit-year, used for binsplit        20001103150917
//  7 .. 2 digit-year, used for forward         0311001509
//  8 .. 2 digit-year, used for WP-mails        001103
//  9 .. 4 digit-year, used for dirformat       20001103
//  10.. 2 digit year, without seconds          03.11.00 15:09z
//  11.. 2 digit year, with seconds             03.11.00 15:09:17z
//  12.. 4 digit year, with seconds             03.11.2000 15:09:17z
//  13.. RFC-date (obsolete) use ISO            Mon, 07 Jul 1997 20:13:48
//  14.. 4 digit-year, ISO 8601/EN28601         Fri, 2000-Nov-03 15:09:07z
//  15.. 4 digit-year, ISO 8601/EN28601         2000-11-03 15:09:07z
//  16.. 1 digit-year, used for log file        01103
//  17.. 2 digit-month+day, 2digithour+minute   1023/1635
//  18.. RFC-date (obsolete) use ISO            Mon, 07-Jul-1997 20:13:48 ?
//
//*************************************************************************
{
  static char datum[27];
#ifdef UTC_TIME
  int local = FALSE;
#else
  int local = ad_timezone() ? TRUE : FALSE;
#endif
#ifdef OLDTIMEFMT
  #define TZ 32
#else
  #define TZ local?'l':'z'
#endif
  static char *wd[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static char *mn[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  struct tm *tt = ad_comtime(local ? utime + ad_timezone() : utime);
  if (tt)
  {
    switch (ymode)
    {
      case 0: sprintf(datum, "%02d.%02d",
                tt->tm_mday, tt->tm_mon + 1); break;
      case 1: sprintf(datum, "%02d.%02d.%02d",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year % 100); break;
      case 2: sprintf(datum, "%02d.%02d.%04d",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year + Year); break;
      case 3: sprintf(datum, "%02d:%02d%c",
                tt->tm_hour, tt->tm_min, TZ); break;
      case 4: sprintf(datum, "%02d:%02d:%02d%c",
                tt->tm_hour, tt->tm_min, tt->tm_sec, TZ); break;
      case 5: sprintf(datum, "%02d%02d%02d/%02d%02d",
                tt->tm_year % 100, tt->tm_mon + 1, tt->tm_mday,
                tt->tm_hour, tt->tm_min); break;
      case 6: sprintf(datum, "%04d%02d%02d%02d%02d%02d",
                tt->tm_year + Year, tt->tm_mon + 1, tt->tm_mday,
                tt->tm_hour, tt->tm_min, tt->tm_sec); break;
      case 7: sprintf(datum, "%02d%02d%02d%02d%02d",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year % 100,
                tt->tm_hour, tt->tm_min); break;
      case 8: sprintf(datum, "%02d%02d%02d",
                tt->tm_year % 100, tt->tm_mon + 1, tt->tm_mday); break;
      case 9: sprintf(datum, "%4d%02d%02d",
                tt->tm_year + Year, tt->tm_mon + 1, tt->tm_mday); break;
      case 10: sprintf(datum, "%02d.%02d.%02d %02d:%02d%c",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year % 100,
                tt->tm_hour, tt->tm_min, TZ); break;
      case 11: sprintf(datum, "%02d.%02d.%02d %02d:%02d:%02d%c",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year % 100,
                tt->tm_hour, tt->tm_min, tt->tm_sec, TZ); break;
      case 12: sprintf(datum, "%02d.%02d.%04d %02d:%02d:%02d%c",
                tt->tm_mday, tt->tm_mon + 1, tt->tm_year + Year,
                tt->tm_hour, tt->tm_min, tt->tm_sec, TZ); break;
      case 13: sprintf(datum, "%s, %d %s %d %02d:%02d:%02d",
                wd[tt->tm_wday], tt->tm_mday, mn[tt->tm_mon], tt->tm_year + Year,
                tt->tm_hour, tt->tm_min, tt->tm_sec); break;
      case 14: sprintf(datum, "%s, %04d-%s-%02d %02d:%02d:%02d%c",
                wd[tt->tm_wday], tt->tm_year + Year, mn[tt->tm_mon], tt->tm_mday,
                tt->tm_hour, tt->tm_min, tt->tm_sec, TZ); break;
      case 15: sprintf(datum, "%04d-%02d-%02d %02d:%02d:%02d%c",
                tt->tm_year + Year, tt->tm_mon + 1, tt->tm_mday,
                tt->tm_hour, tt->tm_min, tt->tm_sec, TZ); break;
      case 16: sprintf(datum, "%1d%02d%02d",
                tt->tm_year % 10, tt->tm_mon + 1, tt->tm_mday); break;
      case 17: sprintf(datum, "%02d%02d/%02d%02d",
                tt->tm_mon + 1, tt->tm_mday, tt->tm_hour, tt->tm_min); break;
      case 18: sprintf(datum, "%s, %d-%s-%d %02d:%02d:%02d GMT",
                wd[tt->tm_wday], tt->tm_mday, mn[tt->tm_mon], tt->tm_year + Year,
                tt->tm_hour, tt->tm_min, tt->tm_sec); break;
      default: *datum = 0;
    }
  }
  else *datum = 0;
#undef TZ
  return datum;
}

/*---------------------------------------------------------------------------*/

int newdate (time_t d1, time_t d2)
//*************************************************************************
//
//  Vergleicht das Datum 2er Tage im UNIX-Format
//
//*************************************************************************
{
  char datum[12];
  if (d1 && d2)
  {
    strcpy(datum, datestr(d1, 1));
    return strcmp(datum, datestr(d2, 1));
  }
  else return 1;
}

/*---------------------------------------------------------------------------*/

char *zeitspanne (time_t zeit, delta_t mode)
//*************************************************************************
//
//  Gibt einen Zeitunterschied als String zurueck.
//
//  Werte fuer mode:
//  zs_runtime  : Zeit (in sec) wird als [dd.]hh.mm  ausgegeben
//  zs_seconds  : Zeit (in sec) wird als [t]s [t]m [t]h [t]d ausgegeben
//  zs_cputime  : Zeit (in 55msec) wird wie <seconds> ausgegeben
//  zs_cpuexact : Zeit (in 55msec) wird dezimal sss.ms ausgegeben
//
//*************************************************************************
{
  unsigned long hours = zeit / HOUR;
  unsigned long days = zeit / DAY;
  static char s[16];

  *s = 0;
  if (mode > zs_runtime)
  {
    if (mode >= zs_cputime)
    {
      zeit *= MS_PER_TIC; // from now on take milliseconds
      if (mode == zs_cpuexact)
      {
        sprintf(s, "%u.%02u", (unsigned) (zeit / 1000),
                              ((unsigned) (zeit % 1000) / 10));
        return s;
      }
      if (zeit < 10000)
      {
        sprintf(s, "%u.%us", (unsigned) (zeit / 1000),
                             ((unsigned) (zeit % 1000)) / 100);
        return s;
      }
      zeit /= 1000L;                   // too much for decimal display
    }
    if (zeit < 240L)
      sprintf(s, "%lds", zeit);        // below 4min show seconds
    else if (zeit < (HOUR * 4))
      sprintf(s, "%ldm", zeit / 60);   // below 4h show minutes
    else if (zeit < (DAY * 4))
      sprintf(s, "%ldh", zeit / HOUR); // below 4days in show hours
    else if (zeit < (DAY * 365))
      sprintf(s, "%ldd", zeit / DAY);  // from year on show in years
    else
      if ((unsigned) zeit < (unsigned long) (DAY * 365 * 25)) // maximal 25 Jahre
      sprintf(s, "%ldy", zeit / (DAY * 365));
  }
  else
  {
    if (days > 999UL)
      sprintf(s, "-:--");
    else
    {
      zeit -= hours * HOUR;
      hours -= days * 24UL;
      if (days) sprintf(s, "%lu.", days);
      sprintf(s + strlen(s), "%02u:%02u", (unsigned) hours,
                                          (unsigned) zeit / 60);
    }
  }
  return s;
}

/*---------------------------------------------------------------------------*/

time_t parse_time (char *datum)
//*************************************************************************
//
//  Wandelt einen String, der ein Lokal-Datum enthaelt, in eine
//  ANSI-UTC-Zeit (UTC) um.
//  Dabei werden unbekannte Teile mit dem aktuellen Datum ersetzt.
//
//*************************************************************************
{
  struct tm tt;
  char *olddatum = datum;

  memcpy(&tt, ad_comtime(ad_time()), sizeof(struct tm));
  tt.tm_hour = 0;
  tt.tm_min = 0;
  tt.tm_sec = 0;
  tt.tm_isdst = -1;
  while (*datum && ! isalnum(*datum)) datum++;
  if (isdigit(*datum))
  {
    tt.tm_mday = atoi(datum);
    while (isdigit(*datum++));
    if (isdigit(*datum))
    {
      tt.tm_mon = atoi(datum) - 1;
      while (isdigit(*datum++));
      if (isdigit(*datum))
      {
        tt.tm_year = atoi(datum);
        if (tt.tm_year < 70) tt.tm_year += 100;
        if (tt.tm_year > Year) tt.tm_year -= Year; //y2k, accept 4 digit date
        while (isdigit(*datum)) datum++;
      }
    }
    if (tt.tm_mday < 1 || tt.tm_mday > 31 || tt.tm_mon < 0 || tt.tm_mon > 11)
    {
      putf(ms(m_date_invalid));
      return 0L;
    }
  }
  while ((olddatum != datum) && *olddatum)
  {
    *olddatum = ' ';
    olddatum++;
  }
  return ad_mktime(&tt) - ad_timezone();
}

/*---------------------------------------------------------------------------*/

/* not used yet
time_t parseISOtime (char *datums)      //dk2ui
//-------------------------------------------------------------------------
//
//  Wandelt einen String, der ein ISO-Datum(localtime) enthaelt in eine ANSI-Zeit(UTC) um.
//
//-------------------------------------------------------------------------
{ struct tm tt;
  char *datptr = datums;

  memset(&tt, 0, sizeof(struct tm));
  tt.tm_isdst = -1;
  while (*datptr && ! isalnum(*datptr)) datptr++;
  if (isdigit(*datptr))
  { tt.tm_year = atoi (datptr);
    if (tt.tm_year > Year) tt.tm_year -= Year;
    if (tt.tm_year < 70)   tt.tm_year += 100;
    while (isdigit(*datptr++));
    if (isdigit(*datptr))
    { tt.tm_mon = atoi(datptr) - 1;
      while (isdigit(*datptr++));
      if (isdigit(*datptr))
      { tt.tm_mday = atoi(datptr);
        while (isdigit(*datptr++));
        if (isdigit(*datptr))
        { tt.tm_hour = atoi(datptr);
          while (isdigit(*datptr++));
          if (isdigit(*datptr))
          { tt.tm_min = atoi(datptr);
            while (isdigit(*datptr++));
            if (isdigit(*datptr))
              tt.tm_sec = atoi(datptr);
          }
        }
      }
    }
    if (tt.tm_mday < 1 || tt.tm_mday > 31 || tt.tm_mon < 0 || tt.tm_mon > 11)
    { putf(ms(m_date_invalid));
      return 0L;
    }
  }
  return ad_mktime(&tt) - ad_timezone();
}
*/
/*---------------------------------------------------------------------------*/

#ifdef __FLAT__
    struct ftime                // 32 bit
    {
      unsigned ft_tsec  : 5;    // 0..29 * 2 (!)
      unsigned ft_min   : 6;    // 0..59
      unsigned ft_hour  : 5;    // 0..23
      unsigned ft_day   : 5;    // 1..31
      unsigned ft_month : 4;    // 1..12
      unsigned ft_year  : 7;    // years from 1980
    };

#ifdef __LINUX__
 #include <utime.h>
#else   // _Win32
 #include <winbase.h> // FILETIME, DosDateTimeToFileTime, SetFileTime
#endif
#endif

#ifdef __FLAT__
time_t getunixtime (unsigned long dosfiletime)
//*************************************************************************
//
//dk2ui: the BIN file time format is from a Borland C struct and unknown
//       in Linux
//
//*************************************************************************
{
  struct ftime *ft;
  struct tm tt;
  ft = (struct ftime *) &dosfiletime;
  memset(&tt, 0, sizeof(struct tm));
  tt.tm_isdst = -1;
  tt.tm_sec  = ft->ft_tsec << 1;
  tt.tm_min  = ft->ft_min;
  tt.tm_hour = ft->ft_hour;
  tt.tm_mday = ft->ft_day;
  tt.tm_mon  = ft->ft_month - 1;
  tt.tm_year = ft->ft_year + 80;
  return ad_mktime(&tt); // localtime
}
#endif

/*---------------------------------------------------------------------------*/

unsigned long getdostime (time_t timeu)
//*************************************************************************
//
// returns a 32 bit value in Borland C ftime format for #BIN#
//
//*************************************************************************
{
  static union u2d
  {
    struct ftime ft;
    unsigned long dostime;
  } ud;

  if (! timeu) return 0L;
    struct tm *tt = ad_comtime(timeu);
    ud.ft.ft_tsec  = tt->tm_sec >> 1;
    ud.ft.ft_min   = tt->tm_min;
    ud.ft.ft_hour  = tt->tm_hour;
    ud.ft.ft_day   = tt->tm_mday;
    ud.ft.ft_month = tt->tm_mon + 1;
    ud.ft.ft_year  = tt->tm_year - 80;
  return ud.dostime;
}

/*---------------------------------------------------------------------------*/

time_t yymmdd2ansi (char *yymmdd)
//*************************************************************************
//
//  converts WP-time to ANSI time (WP is in UTC)
//
//*************************************************************************
{
  struct tm tt;
  char datstr[7];
/* Example
  981012
  012345
*/
  //tt muss geloescht werden, damit das Ergebnis von mktime stimmt! (deti)
  memset(&tt, 0, sizeof(struct tm));
  tt.tm_isdst = -1;
  strncpy(datstr, yymmdd, 6);
  datstr[6] = 0;
  //minutes
  tt.tm_min = 0;
  //hours
  tt.tm_hour = 0;
  //day of month
  tt.tm_mday = atoi(datstr + 4);
  //month
  datstr[4] = 0;
  tt.tm_mon = atoi(datstr + 2) - 1;
  //year
  datstr[2] = 0;
  tt.tm_year = atoi(datstr);
  if (tt.tm_year < 70) tt.tm_year += 100;  //y2k
  if (tt.tm_mday && tt.tm_mon < 12 && tt.tm_year) return ad_mktime(&tt);
  return 0L;
}

/*---------------------------------------------------------------------------*/

