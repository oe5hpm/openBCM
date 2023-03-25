/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------
  FTP-Server for BCM   See RFC959
  ------------------

  Copyright (C)       Jens Schoon, DH6BB

  Alle Rechte vorbehalten / All Rights reserved

// 20040000 DH6BB  Implementierung des FTP-Servers in die OBCM
// 20040205 DH6BB  Implementierung NLST
// 20040205 DH6BB  Implementierung CDUP
// 20040205 DH6BB  Rewrite PORT
// 20040207 DH6BB  Check ob Path erreichbar/gueltig
// 20040207 DH6BB  Implementierung STOR
// 20040208 DH6BB  Check wo geschrieben werden darf
// 20040514 DH6BB  Wiederaufnahme abgebrochener Downloads mit "REST" moeglich

 ***************************************************************/

#include "baycom.h"

#ifdef __UNIX__
  #include <grp.h>
  #include <pwd.h>
#endif
#define FTPSIGSTR "OBCMFTPD"

  //seperator for paths
  #define _SEP '/'
  #define _SEPS "/"
  #define _DSEP '//'
  #define _DSEPS "//"

#ifndef __UNIX__
  #define DRIVE_LETTERS
#endif

static char *Monat[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

class ftpd
{
public:
    ftpd (void);
    ~ftpd (void);
    void start_ftp (char *name);

private:
    void permissions (unsigned long);
    void processpath (char *);
    void get_intropath (char *);
    char signature[sizeof(FTPSIGSTR)];
    char *extract_argument (void);
    bool chpath (char *, char *);
    bool loggedin (void);
    bool pathallowed (char *path);
    FILE *f;
    DIR  *dir;
    char *tmp;
    char *args;
    char tmp2[256];
    char group[30];
    char user[15];
    char pass[34];
    char fspath[FSPATHLEN+1];
    char fullpath[FSPATHLEN+1];
    char intropath[FSPATHLEN+1];
    char permission[11];
    bool finish;
    struct stat statbuf;
};

/*---------------------------------------------------------------------------*/

ftpd::ftpd (void)
//*************************************************************************
//
//  Initialization
//
//*************************************************************************
{
  strcpy(signature, FTPSIGSTR);
}

/*---------------------------------------------------------------------------*/

ftpd::~ftpd (void)
//*************************************************************************
//
//  Deinitialization
//
//*************************************************************************
{
  if (strcmp(signature, FTPSIGSTR))
    trace(fatal, "ftpd", "sig broken");
}

/*---------------------------------------------------------------------------*/

void ftpd::start_ftp (char *name)
//*************************************************************************
//
// Startet den FTP-Server und arbeitet die Befehle ab
//
//*************************************************************************
{
  lastfunc("start ftp");
  static char *Type[]=
  {
    "UNKNOWN", "ASCII", "BINARY"
  };

  static char *beftab[] =
  {
    "BYE",
    "CDUP", "CWD",
    "DIR",
    "EXIT", "EPSV",
    "FEAT",
    "HELP",
    "LIST",
    "MDTM",
    "NLST", "NOOP",
    "PASS", "PASV", "PORT", "PWD",
    "QUIT",
    "REST", "RETR",
    "SIZE", "STOR", "SYST",
    "TYPE",
    "USER",
    NULL
  };

  enum befnum
  {
    _ERR,
    _BYE,
    _CDUP, _CWD,
    _DIR,
    _EXIT, _EPSV,
    _FEAT,
    _HELP,
    _LIST,
    _MDTM,
    _NLST, _NOOP,
    _PASS, _PASV, _PORT, _PWD,
    _QUIT,
    _REST, _RETR,
    _SIZE, _STOR, _SYST,
    _TYPE,
    _USER
  };

  struct stat st;
  struct tm *ptm;
#ifdef __LINUX__
  struct dirent **namelist;
#else
  struct dirent *di;
#endif
  int filecount = 0;
  int old_sock;
  int c;
  int i = 0;
  int error = 0;
  int Typ = 0;
  unsigned ip[5];
  unsigned ipport[3];
  unsigned long int filesize = 0;
  unsigned long int rest = 0;
  char tempallowed[FSPATHLEN+1];
  char mdtmstr[80];
  char port[16];
  char test[FSPATHLEN + 256];
  bool nlist = false; // NLST-Commando oder LIST-Commando
  FILE *datei;
  b->ftp = 1;
  b->charset = 1; //get_ttycharset..
  finish = false;
  *group = 0;
  *user = 0;

  if (myhostname(b->line, BUFLEN - 1) == -1) return;
  putf("220 %s FTP-Server OpenBCM V" VNUMMER "\n", b->line);
  while (! runterfahren)
  {
    getline(b->line, BUFLEN - 1, 1);
    if ((m.tcpiptrace == 5) || (m.tcpiptrace == 8))
    {
      if (! strstr(b->line, "PASS"))
        ftplog("RX", b->line);
      else
        ftplog("RX", "PASS (hidden)");
    }
    args = b->line;
    switch ((befnum) readcmd(beftab, &args, 0))
    {
      case _USER:
        if (! (tmp = extract_argument()))
        {
            putf("500 '%s': command not understood.\n", b->line);
            finish = true;
            break;
        }
        strcpy(user, tmp);
        if (! strcasecmp(user, "anonymous") ||
            ! strcasecmp(user, "ftp") ||
            ! strcasecmp(user, "guest") ||
            ! strcasecmp(user, "gast"))
        {
#ifdef _GUEST
          putf("331 Guest login ok. Type your mail-address as password.\n");
          strcpy(user, "guest");
#else
          putf("500 No guest login. Sri\n");
          return;
#endif
        }
        else
        {
          putf("331 Password required for %s.\n", user);
        }
        break;
      case _PASS:
        if (! user)
        {
          putf("503 Login with USER first.\n");
          break;
        }
        if (strcasecmp(user, "guest")) // kein Gast
        {
          if (! (tmp = extract_argument()))
          {
            putf("500 '%s': command not understood.\n", b->line);
            finish = true;
            break;
          }
          strcpy(pass, tmp);
          if (mbcallok(user))
          {
            if (isamprnet(b->peerip) > 0) get_httppw(user, b->line);
            else get_ttypw(user, b->line);
          }
          else *b->line = 0;
          if (! *b->line || strcasecmp(b->line, pass))
          {
            pwlog(b->peerip, b->uplink, "bad pw");
            putf("530 Login incorrect.\n");
            finish = true;
            break;
          }
        }
/*
        else // Guest
        {
          if (! strstr(pass, "@")) // ein @ sollte in einer Mailadresse sein
          {
            putf("530 Login incorrect. No valid mail-address\n");
            break;
          }
        }
*/
        b->pwok = OK;
        mblogin(user, login_silent, "FTP");
        putf("230- OpenBCM FTP-Server\n");
        readtext("ftpintro"); // -> msg/ftpintro.dl
        if (m.disable)
        {
          putf("230- Service not available due to maintenance!\n");
          putf("221 OpenBCM FTP-Server signing off. Goodbye.\n");
          finish = true;
          break;
        }
        if (*m.fspath == 'O' && *m.fspath == 'o')
        {
          putf("230- Service not available!\n");
          putf("221 OpenBCM FTP-Server signing off. Goodbye.\n");
          finish = true;
          break;
        }
        putf("230 User %s logged in.\n", user);
        // Hier suchen wir den Path zum filesurf
        strcpy(tempallowed, m.fspath);
        strcat(tempallowed, " ");
        *strstr(tempallowed, " ") = 0;
        if (*tempallowed == '+') strcpy(tempallowed, tempallowed + 1);
        strcpy(fspath, tempallowed);
        get_intropath(fspath);
        strcpy(fspath, _SEPS);
        break;
      case _BYE:
      case _QUIT:
        finish = true;
        wdelay(200);
        putf("221 OpenBCM FTP-Server signing off. Goodbye.\n");
        break;
      case _HELP:
        if (! loggedin()) break;
        putf("HELP\n");
        putf(".\n");
        break;
      case _CDUP:        // Change Working Directory 1 up
        if (! loggedin()) break;
        chpath(fspath, "..");
        putf("250 CWD Command successful.\n");
        break;
      case _CWD:        // Change Working Directory
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 Unknown command.\n");
            break;
        }
        if (chpath(fspath, tmp))
        {
          putf("250 CWD Command successful.\n");
        }
        else
          putf("550 %s: No such file or directory.\n", tmp);
        break;
      case _MDTM:
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 Unknown command.\n");
          break;
        }
        if (stat(tmp, &statbuf))
        {
          ptm = gmtime(&statbuf.st_mtime);
          sprintf(mdtmstr, "%04d%02d%02d%02d%02d%02d",
                           ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
                           ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
          putf("213 %s\n", mdtmstr);
        }
        else
        {
          putf("550 %s: No such file or directory.\n", tmp);
        }
        break;
      case _SIZE:        // Size of File
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 Unknown command.\n");
          break;
        }
        if (tmp[0] != _SEP)
          sprintf(test, "%s%s", fullpath, tmp);
        else
          sprintf(test, "%s%s", intropath, tmp);
        if (! (error = stat(test, &st)))
        {
          putf("213 %ld\n", st.st_size);
        }
        else
        {
          putf("550 %s: error stat file.\n");
        }
        break;
      case _SYST:        // System Type
        if (! loggedin()) break;
#ifdef __UNIX__
        putf("215 UNIX Type: L8\n");
#else
        putf("215 Windows_NT\n");
#endif
        break;
      case _TYPE:
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 '%s' command not understood.\n", args);
        }
        else
        {
          if (! strcmp(args, "A"))
          {
            putf("200 Type set to A.\n");
            Typ = 1;
          }
          else if (! strcmp(args, "I"))
          {
            putf("200 Type set to I.\n");
            Typ = 2;
          }
          else putf("501 Unknown Type '%s'.\n", args);
        }
        break;
      case _PASV:        // PASV
        if (! loggedin()) break;
/*
        ipport[0] = random_max(20);
        ipport[1] = random_max(20);
        putf("227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
               ip[0], ip[1], ip[2], ip[3], ipport[0], ipport[1]);
*/
      //ToDo:
      //Sollte man sicherlich noch einbauen
        putf("500 PASV Unknown command.\n");
        break;
      case _EPSV:        // EPSV
        if (! loggedin()) break;
      //ToDo:
      //Kann man sicherlich noch einbauen
        putf("500 EPSV not understood.\n");
        break;
      case _FEAT:        // FEAT
        if (! loggedin()) break;
        putf("500 FEAT is not supported by the remote server.\n");
      //ToDo:
      //Kann man sicherlich noch einbauen
        break;
      case _NOOP:        // Noop
        if (! loggedin()) break;
        putf("200 NOOP command successful.\n");
        break;
      case _PORT:        // Port
        if (! loggedin()) break;
        if (*args)
        {
          int nummer = 0;
          int a = 0;
          safe_strcpy(tmp2, args);
          for (i=0; i<4; i++)
          {
            while (tmp2[a] != ',')
            {
              nummer *= 10;
              nummer += tmp2[a] - 48;
              a++;
            }
            ip[i] = nummer;
            nummer = 0;
            a++;
          }
          for (i=0; i<2; i++)
          {
            while (tmp2[a] != ',' && isdigit(tmp2[a]))
            {
              nummer *= 10;
              nummer += tmp2[a] - 48;
              a++;
            }
            ipport[i]=nummer;
            nummer=0;
            a++;
          }
          sprintf(port, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
          putf("200 PORT command successful.\n");
        }
        else
          putf("500 Unknown command.\n");
        break;
      case _PWD:
        if (! loggedin()) break;
        putf("257 \"%s\" is the current directory.\n", fspath);
        break;
      case _RETR:
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 %s Unknown command.\n", tmp);
          break;
        }
        if (! port)
        {
          putf("500 No data port.\n"); // Meldung OK?
          break;
        }
        if (tmp[0] != _SEP)
          sprintf(test, "%s%s", fullpath, tmp);
        else
          sprintf(test, "%s%s", intropath, tmp);
        if (! (datei = s_fopen(test, "lrb")))
        {
          putf("550 %s: No such file or directory.\n", tmp);
        }
        else
        {
          stat(test, &st);
          putf("150 Opening %s mode data connection for '%s' (%ld bytes).\n",
               Type[Typ], tmp, st.st_size);
          putflush_sock();      // alten puffer leeren
          old_sock = t->socket; // alten socket retten
          // Daten werden auf einem anderen Port uebertragen
          sprintf(port, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
          if (! ftp_connect(port, ipport[0]*256+ipport[1]))
          {
            trace(report, "ftpd", "ftp_connect failed %s", port);
            *b->logincall = 0;
//            putflush_sock();   // puffer leeren
//            disconnect_sock(); // Daten socket schliessen
            t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
            putf("500 Can't connect data port.\n");
            break;
          }
          filesize=0;
          t->processkind &= ~P_TELNET;
          while ((c = fgetc(datei)) != EOF)
          {
            if (filesize >= rest) bputv(c);
            filesize++;
          }
          t->processkind |= P_TELNET;
#ifdef DEBUG_FTP
          trace(report, "ftpd", "Submit-Size: %d", filesize);
#endif
          s_fclose(datei);
          rest = 0;
          putflush_sock();   // puffer leeren
          disconnect_sock(); // Daten socket schliessen
          t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
          putf("226 Transfer complete.\n");
        }
        break;
      case _REST:        // Wiederaufnahme abgebrochener Downloads
        if (! loggedin()) break;
        if (! (tmp = extract_argument()))
        {
          putf("500 %s Unknown command.\n", tmp);
          break;
        }
        rest=atoi(tmp);
        putf("350 Restarting at %d. Send STORE or RETRIEVE to initiate transfer.\n", rest);
        break;
      case _STOR:
        if (! loggedin()) break;
        if (! strcasecmp(user, "guest"))
        { // Wenn GUEST, dann schreiben verboten!
          putf("553 Permission denied.\n");
          break;
        }
        if (! (tmp = extract_argument()))
        {
          putf("500 %s Unknown command.\n", tmp);
          break;
        }
        if (! port)
        {
          putf("500 No data port.\n");
          break;
        }
        if (tmp[0] != _SEP)
          sprintf(test, "%s%s", fullpath, tmp);
        else
          sprintf(test, "%s%s", intropath, tmp);
        // Checken ob wir hier ueberhaupt schreiben duerfen!
        if (! pathallowed(test))
        {
          putf("553 Permission denied.\n");
          break;
        }
        if ((datei = s_fopen(test, "sr+t")) != NULL)
        {
          // Entgegen der FTP-Spec ueberschreiben wir keine Dateien!
          // Es sei denn, es wurde ein "Rest" angegeben.
          s_fclose(datei);
          if (rest == 0)
          {
            putf("550 File exists (%s)\n", tmp); // ToDo: Meldung checken
            break;
          }
        }
        if (! (datei=s_fopen(test, "sa+b")))
        {
          putf("550 Cannot open file (%s)\n", tmp);      // ToDo: Meldung checken
          break;
          // 550 '%s': Permission denied
        }
        else
        {
          putf("150 Opening %s mode data connection for '%s'.\n", Type[Typ], tmp);
          // Daten werden auf einem anderen Port uebertragen
          sprintf(port, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
          putflush_sock();     // alten puffer leeren
          old_sock = t->socket; // alten socket retten
          if (! ftp_connect(port, ipport[0]*256+ipport[1]))
          {
             trace(report, "ftpd", "ftp_connect failed %s", port);
             *b->logincall = 0;
//           putflush_sock();    // puffer leeren
//           disconnect_sock();  // Daten socket schliessen
             t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
             putf("500 Can't connect data port.\n");
             break;
          }
          while ((c = getv_sock_ftp()) != 256)
          {
            fputc(c, datei);
            filesize++;
          }
          s_fclose(datei);
          putflush_sock(); // puffer leeren
          disconnect_sock(); // Daten socket schliessen
          t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
          putf("226 Transfer complete.\n");
        }
        break;
      case _NLST:
        nlist = true; // Nur Dateinamen. kein break!
      case _LIST:        // List (DIR)
        if (! loggedin()) break;
        putf("150 Opening %s mode data connection for file list.\n", Type[Typ]);
        if (! port)
        {
          putf("500 No data port.\n");
          break;
        }
        filecount = 0;
        putflush_sock();      // alten puffer leeren
        old_sock = t->socket; // alten socket retten
      // Daten werden auf einem anderen Port uebertragen
#ifdef __UNIX__
        filecount = scandir(fullpath, &namelist, 0, alphasort);
#else
        dir = opendir(fullpath);
        while (dir && (di = readdir(dir)) != NULL)
          filecount++;
        if (dir) closedir(dir);
#endif
        if (filecount < 0)
        {
#ifdef DEBUG_FTP
          trace(report, "ftpd", "Filecount error: %d", filecount);
#endif
          putf("550 No files found. Please inform the sysop.\n");
          nlist = false;
          break;
        }
        else
        {
          if (! port)
          {
            putf("500 No data port.\n");
            break;
          }
          if (! ftp_connect(port, ipport[0]*256+ipport[1]))
          {
            trace(report, "ftpd", "ftp_connect failed %s", port);
            *b->logincall = 0;
//          putflush_sock();    // puffer leeren
//          disconnect_sock();  // Daten socket schliessen
            t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
            putf("500 Can't connect data port.\n");
            break;
          }
#ifdef __UNIX__
          while (i < filecount)
          {
            if (nlist) //nur Dateinamen
              putf("%s\n", namelist[i]->d_name);
            else
            {
              sprintf(test, "%s%s", fullpath, namelist[i]->d_name);
              error = stat(test, &st);
              if (! error)
              {
                permissions(st.st_mode);
                putf("%s %4u %-8s %-8s ", permission, st.st_nlink,
                                          getpwuid(st.st_uid)->pw_name,
                                          getgrgid(st.st_gid)->gr_name);
                putf("%8ld ", st.st_size);
                ptm = gmtime(&st.st_mtime);
                if (difftime(time(0), st.st_mtime)>365*24*60*60)
                  putf("%03s %02d %04d ", Monat[ptm->tm_mon],
                                          ptm->tm_mday, ptm->tm_year+1900);
                else
                  putf("%03s %02d %02d:%02d ", Monat[ptm->tm_mon],
                                               ptm->tm_mday, ptm->tm_hour,
                                               ptm->tm_min);
                putf("%s\n", namelist[i]->d_name);
              }
              else
              {
#ifdef DEBUG_FTP
                trace(report, "ftpd", "%s %s", namelist[i]->d_name,
                                               strerror(errno));
#endif
              }
            }
            free(namelist[i]);
            i++;
          }
        }
#else
        dir = opendir(fullpath);
        while (dir && (di = readdir(dir)) != NULL)
        {
          if (nlist)    //nur Dateinamen
            putf("%s\n", di->d_name);
          else
          {
            sprintf(test, "%s%s", fullpath, di->d_name);
            error = stat(test, &st);
            if (! error)
            {
              permissions(st.st_mode);
              putf("%s %4u %-8s %-8s ", permission,
                                        st.st_nlink, "user", "group");
              putf("%8ld ", st.st_size);
              ptm = gmtime(&st.st_mtime);
              if (difftime(time(0), st.st_mtime)>31536000) //365*24*60*60
                putf("%03s %02d %04d ", Monat[ptm->tm_mon],
                                        ptm->tm_mday, ptm->tm_year+1900);
              else
                putf("%03s %02d %02d:%02d ", Monat[ptm->tm_mon],
                                             ptm->tm_mday, ptm->tm_hour,
                                             ptm->tm_min);
              putf("%s\n", di->d_name);
            }
            else
            {
#ifdef DEBUG_FTP
              trace(report, "ftpd", "%s %s", di->d_name, strerror(errno));
#endif
            }
          }
        }
      }
      if (dir) closedir(dir);
#endif
      putflush_sock();    // puffer leeren
      disconnect_sock();  // Daten socket schliessen
      t->socket = old_sock; // Zum Glueck haben wir unseren alten Socket noch
      if (filecount == 0)
        putf("550 No files found.\n");
      else
        putf("226 Transfer complete.\n");
      nlist = false;
      break;
      default:
        putf("500 Unknown command.\n");
  }
  if (finish) break;
 }
}

/*---------------------------------------------------------------------------*/

bool ftpd::loggedin (void)
//*************************************************************************
//
// Fragt ab, ob wir schon eingelogged sind.
// TRUE  = eingelogged mit Username und Passwort (auch anonymous)
// FALSE = nicht eingelogged
//
//*************************************************************************
{
  lastfunc("loggedin");
  if (! user || ! b->pwok)
  {
    putf("503 Login with USER and PASS first\n");
    return false;
  }
  else
    return true;
}

/*---------------------------------------------------------------------------*/

void mblogin_ftp (char *name)
//*************************************************************************
//
//  Haupt-Login
//
//*************************************************************************
{
  lastfunc("mblogin_ftp");
  ftpd ft;

  strcpy(b->uplink, "FTP");
  strcpy(b->peerip, name);
  *b->logincall = 0;
  ft.start_ftp(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/

char *ftpd::extract_argument ()
//*************************************************************************
//
//  Gibt nur die Argumente nach dem Befehl zurueck.
//  Achtung! "," gilt als Trennzeichen....
//
//*************************************************************************
{
  lastfunc("extract_argument");
  skip(args);
  if (*args)
    return args;
  else
    return NULL;
}
/*---------------------------------------------------------------------------*/

bool ftpd::chpath (char *path, char *add_path)
//*************************************************************************
//
//  Fuegt den zweiten angegebenen Pfad an den ersten Pfad an bzw. ersetzt
//  den ersten Pfad durch den zweiten, wenn dieser mit einem Slash beginnt
//  Rueckgabewert TRUE wenn Verzeichnis gueltig, sonst FALSE
//  geklaut aus "filesurf"
//
//*************************************************************************
{
  lastfunc("chpath");
  char tmppath[FSPATHLEN+FNAMELEN+2];
  char stat_path[FSPATHLEN+FNAMELEN+2];
  bool gueltig = false;

  strcpy(tmppath, path); // alten Path sichern
  if (*add_path == _SEP)
    strcpy(tmppath, add_path);
  else
    strcat(tmppath, add_path);
  processpath(tmppath);
  sprintf(stat_path, "%s%s", intropath, tmppath);
  processpath(stat_path);
  if ((dir = opendir(stat_path)) != NULL)
  {
    closedir(dir);
    strcpy(path, tmppath);
    gueltig = true;
  }
  else
    gueltig = false;
  sprintf(fullpath, "%s%s", intropath, path);
  processpath(fullpath);
  return gueltig;
}

/*---------------------------------------------------------------------------*/

void ftpd::processpath (char *path)
//*************************************************************************
//
//  - Replaces two or more slashes in a row by a single slash
//  - Replaces "/./" by "/"
//  - Removes "/../" and the preceding part of the path
//  - Puts a slash at the end, if there isn't one
//
//*************************************************************************
{
  lastfunc("processpath");
  char *pointer, *pointer2;

  strcat(path, _SEPS);
  while ((pointer = strstr(path, _SEPS""_SEPS)) != NULL)
    memmove(pointer, pointer + 1, strlen(pointer + 1) + 1);
  while ((pointer = strstr(path, _SEPS"."_SEPS)) != NULL)
    memmove(pointer, pointer + 2, strlen(pointer + 2) + 1);
  while ((pointer = strstr(path, _SEPS".."_SEPS)) != NULL)
  {
    memmove(pointer, pointer + 3, strlen(pointer + 3) + 1);
    if (pointer == path) break;
    pointer2 = pointer;
    do pointer--; while (*pointer != _SEP);
    memmove(pointer, pointer2, strlen(pointer2) + 1);
  }
}

/*---------------------------------------------------------------------------*/

void ftpd::get_intropath (char *path)
//*************************************************************************
//
//  - Replaces two or more slashes in a row by a single slash
//  - Replaces "/./" by "/"
//  - Removes "/../" and the preceding part of the path
//  - Puts a slash at the end, if there isn't one
//
//*************************************************************************
{
#ifndef __UNIX__
  #undef _SEP
  #undef _SEPS
  #define _SEP '\\'
  #define _SEPS "\\"
#endif
  lastfunc("intropath");
  char *pointer, *pointer2;

  strcat(path, _SEPS);
  while ((pointer = strstr(path, _SEPS""_SEPS)) != NULL)
    memmove(pointer, pointer + 1, strlen(pointer + 1) + 1);
  while ((pointer = strstr(path, _SEPS"."_SEPS)) != NULL)
    memmove(pointer, pointer + 2, strlen(pointer + 2) + 1);
#ifndef __UNIX__
  while ((pointer = strstr(path, _SEPS)) != NULL)
    memmove(pointer, "//", strlen("//") + 1);
#endif
  while ((pointer = strstr(path, _SEPS".."_SEPS)) != NULL)
  {
    memmove(pointer, pointer + 3, strlen(pointer + 3) + 1);
    if (pointer == path) break;
    pointer2 = pointer;
    do pointer--; while (*pointer != _SEP);
    memmove(pointer, pointer2, strlen(pointer2) + 1);
  }
  safe_strcpy(fullpath, path);
  safe_strcpy(intropath, fullpath);
#ifndef __UNIX__
  #undef _SEP
  #undef _SEPS
  #define _SEP '/'
  #define _SEPS "/"
#endif
}

/*---------------------------------------------------------------------------*/

void ftpd::permissions (unsigned long perm)
//*************************************************************************
//
// Get File-Permission and returns a String like "drwxrw-rw-"
//
//*************************************************************************
{
  lastfunc("permissions");
  strcpy(permission, "----------");
#ifndef __UNIX__
  #define S_IFSOCK 0140000
  #define S_IFLNK  0120000
  #define S_IFBLK  0060000
  #define S_IFIFO  0010000
  #define S_ISUID  0004000
  #define S_ISGID  0002000
  #define S_ISVTX  0001000
  #define S_IRWXU  00700
  #define S_IRUSR  00400
  #define S_IWUSR  00200
  #define S_IXUSR  00100
  #define S_IRWXG  00070
  #define S_IRGRP  00040
  #define S_IWGRP  00020
  #define S_IXGRP  00010
  #define S_IRWXO  00007
  #define S_IROTH  00004
  #define S_IWOTH  00002
  #define S_IXOTH  00001
#endif

  if ((perm & S_IFMT) == S_IFSOCK) permission[0]='s';
  else if ((perm & S_IFMT) == S_IFLNK) permission[0]='l';
  else if ((perm & S_IFMT) == S_IFREG) permission[0]='-';
  else if ((perm & S_IFMT) == S_IFBLK) permission[0]='b';
  else if ((perm & S_IFMT) == S_IFBLK) permission[0]='b';
  else if ((perm & S_IFMT) == S_IFDIR) permission[0]='d';
  else if ((perm & S_IFMT) == S_IFCHR) permission[0]='c';
  else if ((perm & S_IFMT) == S_IFIFO) permission[0]='p';
  else permission[0]='?';

  if (perm & S_IRUSR) permission[1]='r';
  if (perm & S_IWUSR) permission[2]='w';
  if (perm & S_IXUSR)
    if (perm & S_ISUID) permission[3]='s';
    else permission[3]='x';
  else
    if (perm & S_ISUID) permission[3]='S';
    else permission[3]='-';

  if (perm & S_IRGRP) permission[4]='r';
  if (perm & S_IWGRP) permission[5]='w';
  if (perm & S_IXGRP)
    if (perm & S_ISGID) permission[6]='s';
    else permission[6]='x';
  else
    if (perm & S_ISGID) permission[6]='S';
    else permission[6]='-';

  if (perm & S_IROTH) permission[7]='r';
  if (perm & S_IWOTH) permission[8]='w';
  if (perm & S_IXOTH)
    if (perm & S_ISVTX) permission[9]='t';
    else permission[9]='x';
  else
    if (perm & S_ISVTX) permission[9]='T';
    else permission[9]='-';
  permission[10]='\0';
}

/*---------------------------------------------------------------------------*/

bool ftpd::pathallowed (char *path)
//*************************************************************************
//
//  Checks, if a user may write to a directory
//  returns TRUE if ok, FALSE else
//
//*************************************************************************
{
  lastfunc("permissions");
  char tempallowed[FSPATHLEN+1];
  char *pos;
  char *ptr;
  bool access = false;
  char pathpart[FSPATHLEN+1];

  strcpy(tempallowed, m.fspath);
  strcat(tempallowed, " ");
  ptr = tempallowed;
  while ((pos = strstr(ptr, " ")) != NULL)
  {
    *pos = 0;
    if (*ptr == '+')
    {
      access = true;
      ptr++;
    }
    strcpy(pathpart, ptr);
    if (access && (! strncmp(path, pathpart, strlen(pathpart))))
        return true;
    ptr = pos + 1;
  }
  return false;
}

/*---------------------------------------------------------------------------*/
