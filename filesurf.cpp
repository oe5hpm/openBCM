/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------
  Filesurf
  --------

  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

/*---------------------------------------------------------------------------*/

//19980419 OE3DZW fixed support for DOS,path "off",slash vs backslash
//                now using normal help
//19980420 OE3DZW fixed support for Win32 21:12
//19980504 DH3MB  fixed problems with long filename - filenames
//                now may generally not be longer than FNAMELEN
//19980505 OE3DZW now accepting #ok#
//19980526 OE3DZW could not change to "home" directory under dos
//19980610 OE3DZW added log for commands when in fs-mode
//19980619 OE3DZW added fsintro-text again
//19980830 OE3DZW fixed hang of fs on too long input lines
//19980916 OE3DZW added prefix to lastcmd again
//19980924 OE3DZW cleaned up overlapping strcpy's, limited inputline
//19980925 OE3DZW limited size of dir to ca. 167 under DOS
//19981001 OE3DZW fixed wyapp
//20041227 DH6BB  add BMAIL command

#include "baycom.h"

#ifdef FILESURF

#ifdef __UNIX__
  //seperator for paths
  #define _SEP '/'
  #define _SEPS "/"
  //invalid seperator for paths
  #define _INVSEP '\\'
#else
  #define DRIVE_LETTERS
  #define _SEP '\\'
  #define _SEPS "\\"
  #define _INVSEP '/'
#endif

/*---------------------------------------------------------------------------*/

void filesurf::putwelcome ()
//*************************************************************************
//
//  Shows the intro-text
//
//*************************************************************************
{
  putf(ms(m_fs_welcome));
  readtext("fsintro");
}

/*---------------------------------------------------------------------------*/

void filesurf::printallowed ()
//*************************************************************************
//
//  Lists the directories, which the users may read from
//
//*************************************************************************
{
  char tempallowed[FSPATHLEN+1];
  char *pos;
  char *ptr;
  char accesslevel;

  putf(ms(m_fs_directories));
  strcpy(tempallowed, m.fspath);
  strcat(tempallowed, " ");
  ptr = tempallowed;
  while ((pos = strstr(ptr, " ")) != NULL)
  {
    *pos = 0;
    accesslevel = 1;
    if (*ptr == '+')
    { accesslevel = 2;
      ptr++;
    }
    putf("     %s", ptr);
    if (accesslevel == 2) putf(" (read/write) ");
    putv(LF);
    ptr = pos + 1;
  }
}

/*---------------------------------------------------------------------------*/

static near int fileexists (char *fname, char par)
//*************************************************************************
//
//  Checks the existence of a file and shows the according text
//
//*************************************************************************
{
  if (! file_isreg(fname))
  {
    if (! par) putf(ms(m_filenotfound), fname);
    return 0;
  }
  else
  {
    if (par == 1) putf(ms(m_filealexists), fname);
    return 1;
  }
}

/*---------------------------------------------------------------------------*/

int filesurf::pathallowed (char *path, int level)
//*************************************************************************
//
//  Checks, if a user may read from/write to a directory
//  If not, the allowed directories are shown
//
//  level 1..read only, 2..read/write
//
//  returns 1 if ok, 0 else
//
//*************************************************************************
{
  char tempallowed[FSPATHLEN+1];
  char *pos;
  char *ptr;
  char accesslevel;
  char pathpart[FSPATHLEN+1];
  FILE *dummy;

  if (b->sysop)
  {
    if (level == 1) return 1;  // Sysop darf ueberall lesen.
    if (! fileexists(path, 1)) // Aber vom System her nicht ueberall lesen
    {
      if ((dummy = s_fopen(path, "swt")) != NULL)
      {
        s_fclose(dummy); // File anlegen, wenns gut gegangen ist wieder
        remove(path);    // loeschen. Alles ok.
        return 1;
      }
      putf(ms(m_fs_nowrite));
      return 0;
    }
  }
  strcpy(tempallowed, m.fspath);
  strcat(tempallowed, " ");
  ptr = tempallowed;
  while ((pos = strstr(ptr, " ")) != NULL)
  {
    *pos = 0;
    accesslevel = 1;
    if (*ptr == '+')
    {
      accesslevel = 2;
      ptr++;
    }
    strcpy(pathpart, ptr);
    if ((! strncmp(path, pathpart, strlen(pathpart)))
        && (accesslevel >= level))
      return(1);
    ptr = pos + 1;
  }
  putf(ms(m_fs_noaccess), path);
  printallowed();
  return(0);
}

/*---------------------------------------------------------------------------*/

int sortbydate (const signed long *num1, const signed long *num2)
//****************************************************************************
//
//****************************************************************************
{
  waitfor(e_ticsfull);
  if (*num1 > *num2) return(1);
  if (*num1 < *num2) return(-1);
  return(0);
}

/*---------------------------------------------------------------------------*/

int sortbysize (const signed long *num1, const signed long *num2)
//****************************************************************************
//
//****************************************************************************
{
  waitfor(e_ticsfull);
  if (*(num1 + 1) > *(num2 + 1)) return(1);
  if (*(num1 + 1) < *(num2 + 1)) return(-1);
  return(0);
}

/*---------------------------------------------------------------------------*/

int sortbyname (const char *s1, const char *s2)
//****************************************************************************
//
//****************************************************************************
{
  waitfor(e_ticsfull);
  return(strcmp(s1 + 8, s2 + 8));
}

/*---------------------------------------------------------------------------*/

int filesurf::showdir (char *path, bitfeld options)
//*************************************************************************
//
//  Lists the files in a directory
//
//*************************************************************************
{
  struct direntry
  {
    signed long date;
    signed long size;
    char name[40];
  };
  DIR *ptr;
  struct dirent *dirdat;
  struct stat info;
  struct direntry *dirinfo;
  char fullname[200];
  unsigned int count;
  unsigned int counter = 0;
  DIR *dummy;
  char maxlength = 0;
  char smaxlength[6];
  char maxc = 0;
  char filemask[80];

  if (! (ptr = opendir(path)))
  {
#ifdef DRIVE_LETTERS
    if (strlen(path) > 3)
#endif
      path[strlen(path) - 1] = 0; // Kill trailing slash
    strcpy(filemask, strrchr(path, _SEP) + 1);
    *(strrchr(path, _SEP) + 1) = 0;
    if (! (ptr = opendir(path)))
    {
      putf(ms(m_dirunknown), path);
      return 0;
    }
  }
  else
    strcpy(filemask, "*");
  strcpy(fullname, path);
  strcat(fullname, ".fs_text");
  if (file_isreg(fullname))
  {
    fileio_text fio;
    fio.usefile(fullname);
    fio.tx();
    putv(LF);
  }
  putf(ms(m_fs_filesindir), path);
#ifdef __DOS16__
  //good old dos makes no difference between upper/lower case
  strupr(filemask);
#endif
  putf(ms(m_fs_filemask), filemask);
  while ((dirdat = readdir(ptr)) != NULL)
    if (   jokcmp(dirdat->d_name, filemask, 0)
        && strcmp(dirdat->d_name, "TRANS.TBL")
        && *dirdat->d_name != '.')
      {
        if ((counter++) * sizeof(struct direntry) > MAXDIRENTRIES)
        {
          putf(ms(m_fs_dirtoocomplex));
          closedir(ptr);
          return 1;
        }
      }
  if (! counter)
  {
    closedir(ptr);
    putf(ms(m_filenotfound), filemask);
    return(1);
  }
  dirinfo = (struct direntry*) t_malloc(counter* sizeof(struct direntry),
                                         "fsvr");
#ifdef _WIN32
  closedir(ptr);
  ptr = opendir(path);
#else
  rewinddir(ptr);
#endif
  waitfor(e_ticsfull);
  counter = 0;
  while ((dirdat = readdir(ptr)) != NULL && counter <= 1300)
  {
    if (   ! jokcmp(dirdat->d_name, filemask, 0)
        || ! strcmp(dirdat->d_name, "TRANS.TBL")
        || *dirdat->d_name == '.') continue;
    strcpy(fullname, path);
    strcat(fullname, dirdat->d_name);
    if (stat(fullname, &info)) continue;
    dirinfo->date = info.st_mtime;
    dirinfo->size = (signed long) info.st_size;
    strncpy(&dirinfo->name[1], dirdat->d_name, 38);
    dirinfo->name[38] = 0;
    if (strlen(&dirinfo->name[1]) > maxlength)
      maxlength = strlen(&dirinfo->name[1]);
    if ((dummy = opendir(fullname)) != NULL)
    {
      closedir(dummy);
      if (! (options & o_l)) strcat(&dirinfo->name[1], _SEPS);
      *dirinfo->name = 1;
      dirinfo->date = - dirinfo->date;
      dirinfo->size = - 1;
    }
    else
      *dirinfo->name = 2;
    counter++;
    dirinfo++;
  }
  closedir(ptr);
  dirinfo -= counter;
  if (options & o_s)
    qsort(dirinfo, counter, 48,
          (int(*) (const void *, const void *)) sortbysize);
  else if (options & o_d)
    qsort(dirinfo, counter, 48,
          (int(*) (const void *, const void *)) sortbydate);
  else if (! (options & o_n))
    qsort(dirinfo, counter, 48,
          (int(*) (const void *, const void *)) sortbyname);
  waitfor(e_ticsfull);
  maxlength += 3;
  sprintf(smaxlength, "%%-%ds", maxlength);
  count = counter;
  for (counter = 0; counter < count; counter++)
  {
    if (options & o_l)
    {
      putf("%-40s", &dirinfo->name[1]);
      if (dirinfo->size == -1)
      {
        dirinfo->date = - dirinfo->date;
        putf(" <DIR>     ");
      }
      else
        putf(" %10ld", dirinfo->size);
      putf(" %s\n", datestr((time_t) dirinfo->date - ad_timezone(), 12));
    }
    else
    {
      putf(smaxlength, &dirinfo->name[1]);
      maxc += maxlength;
      if ((maxc + maxlength) >= 80)
      {
        putv(LF);
        maxc = 0;
      }
    }
    dirinfo++;
  }
  if (maxc) putv(LF);
  putf(ms(m_fs_filesfound), count);
  dirinfo -= count;
  t_free(dirinfo);
  return(1);
}

/*---------------------------------------------------------------------------*/

void filesurf::processpath (char *path)
//*************************************************************************
//
//  - Replaces two or more slashes in a row by a single slash
//  - Replaces "/./" by "/"
//  - Removes "/../" and the preceding part of the path
//  - Puts a slash at the end, if there isn't one
//
//*************************************************************************
{
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

int filesurf::addpath (char *path, char *add_path)
//*************************************************************************
//
//  Fuegt den zweiten angegebenen Pfad an den ersten Pfad an bzw. ersetzt
//  den ersten Pfad durch den zweiten, wenn dieser mit einem Slash beginnt
//
//*************************************************************************
{
  strcpy(b->line, path);
#ifdef DRIVE_LETTERS
  if (add_path[1] == ':') strcpy(b->line, add_path);
  else
#endif
  if (*add_path == _SEP)
#ifdef DRIVE_LETTERS
    strcpy(b->line + 2, add_path);
#else
    strcpy(b->line, add_path);
#endif
  else strcat(b->line, add_path);
  processpath(b->line);
  if (strlen(b->line) > FSPATHLEN)
  {
    putf(ms(m_fs_nametoolong));
    return NO;
  }
  else
  {
    strcpy(path, b->line);
    return OK;
  }
}

/*---------------------------------------------------------------------------*/

void filesurf::putsyntax (char *cmd, char *para)
//*************************************************************************
//
//  Shows the syntax of a filesurf-command and puts a "filesurf " in
//  front of the command, if the user isn't in filesurf-mode
//
//*************************************************************************
{
  putf("Syntax: ");
  if (! b->fsmode) putf("FileSurf ");
  putf("%s %s\n", cmd, para);
}

/*---------------------------------------------------------------------------*/

char filesurf::sysopcmd (char *command)
//*************************************************************************
//
//  Executes a filesurf-sysop-command
//
//*************************************************************************
{
  char *befbuf;
  char temp[FSPATHLEN+1];

  befbuf = command;
  static char *beftab[] =
    { "RM", "DEL", "MD", "MKDIR", "RD", "RMDIR",
      "CP", "COPY", "MV", "MOVE", NULL };
  enum befnum
    { unsinn, rm, del, md, mkdir_, rd, rmdir_,
      cp, copy, mv, move,
    } cmd = unsinn;

  cmd =(befnum) readcmd(beftab, &befbuf, 0);
  befbuf += blkill(befbuf);
  switch (cmd)
  {
  case md:
  case mkdir_:
       {
         if (! *befbuf)
         {
           putsyntax("MD", ms(m_directoryname));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
           if (temp)
             temp[strlen(temp) - 1] = 0; //remove trailing (back)slash
           if (! filetime(temp))
             if (! mkdir(temp))
               putf(ms(m_directorycreated), temp);
             else
               putf(ms(m_directorycantcreated), temp);
           else
             putf(ms(m_fs_alreadyexist), temp);
         }
       } break;
  case rd:
  case rmdir_:
       {
         if (! *befbuf)
         {
           putsyntax("RD", ms(m_directoryname));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
           if (temp)
             temp[strlen(temp) - 1] = 0; //remove trailing (back)slash
           if (filetime(temp))
             if (! rmdir(temp))
               putf(ms(m_fs_directorydeleted), temp);
             else
               putf(ms(m_fs_directorycantdeleted), temp);
           else
             putf(ms(m_fs_directorynotexist), temp);
         }
       } break;
  case cp:
  case copy:
  case mv:
  case move:
       {
         if (! *befbuf || ! strchr(befbuf, ' '))
         {
           if (cmd == mv || cmd == move)
             putsyntax("MV", ms(m_fs_oldnewname));
           else
             putsyntax("CP", ms(m_fs_oldnewname));
           break;
         }
         char name1[FSPATHLEN+1], name2[FSPATHLEN+1];
         int okwert = 0;
         strcpy(name1, b->fspath);
         strcpy(name2, b->fspath);
         okwert = addpath(name2, strrchr(befbuf, ' ') + 1);
         *strrchr(befbuf, ' ') = 0;
         okwert = okwert + addpath(name1, befbuf);
         if (okwert == 2)
         {
#ifdef DRIVE_LETTERS
           if (strlen(name1) > 3)
#endif
             name1[strlen(name1) - 1] = 0;
#ifdef DRIVE_LETTERS
           if (strlen(name2) > 3)
#endif
             name2[strlen(name2) - 1] = 0;
           if (! file_isreg(name1))
           {
             putf(ms(m_filenotfound), name1);
             break;
           }
           if (file_isreg(name2))
           {
             putf(ms(m_filealexists), name2);
             break;
           }
           if (cmd == mv || cmd == move)
             putf(ms(m_moving));
           else
             putf(ms(m_copying));
           putf(ms(m_nach), name1, name2);
           if (! filecopy(name1, name2))
           {
             if (cmd == mv || cmd == move)
               unlink(name1);
             else
               putf(ms(m_error));
           }
         }
       } break;
  case rm:
  case del:
       {
         if (! *befbuf)
         {
           putsyntax("RM", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (file_isreg(temp))
           {
             if (! unlink(temp))
               putf(ms(m_filedeleted), temp);
             else
               putf(ms(m_filecantdeleted), temp);
           }
           else
             putf(ms(m_fs_filenotexist), temp);
         }
       } break;
  default: return 0;
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

void filesurf::execute (char *command)
//*************************************************************************
//
//  Executes a filesurf-command
//
//*************************************************************************
{
  char *befbuf;
  char temp[FSPATHLEN+1];

  if (b->fsmode) trace(report, b->logincall, "fs %s", command);
  if (! strncmp(b->fspath, "off", 3)) getfirstpath(b->fspath);
  if (! strncmp(b->fspath, "off", 3))
  {
    putf(ms(m_fs_unknown));
    return;
  }
  if (! b->sysop && ! pathallowed(b->fspath, 1)) getfirstpath(b->fspath);
  befbuf = command;

  static char *beftab[] =
    { "PATH", "DIR", "LS", "CD", "CHDIR", "CD..", "READ", "TYPE",
      "GET", "CAT", "RTEXT", "PUT", "WTEXT", "BGET", "BIN", "RPRG",
      "BPUT", "WPRG", "HELP", "?", "QUIT", "BYE", "EXIT", "CMDMODE",
      "BMAIL",
#ifdef FEATURE_YAPP
      "YGET", "RYAPP", "YPUT", "WYAPP",
#endif
#ifdef FEATURE_DIDADIT
      "DGET", "DPUT",
#endif
#ifdef FEATURE_BINSPLIT
      "BSGET",
#endif
      "#OK#", "BIN-RX", "BIN-TX", "(BIN-RX):", "(BIN-TX):", "(YAPP-RX):",
      "(YAPP-TX):", "(DIDADIT-RX):", "(DIDADIT-TX):", "<GP>:", "(WPP)",
      NULL
    };

  enum befnum
    { unsinn,
      path, dir, ls, cd, chdir_, cd__, read_, type, get, cat, rtext,
      put, wtext, bget, bin, rprg, bput, wprg, help, help_, quit,
      bye, exit_, cmdmode, bmail,
#ifdef FEATURE_YAPP
      yget, ryapp, yput, wyapp,
#endif
#ifdef FEATURE_DIDADIT
      dget, dput,
#endif
#ifdef FEATURE_BINSPLIT
      bsget,
#endif
      bin_ok, d_binrx, d_bintx, d_binrx2, d_bintx2, d_yapprx, d_yapptx,
      d_didaditrx, d_didadittx, d_gp, d_wpp
    } cmd = unsinn;

  //this is dirty, but it should help to ensure that the input line
  //is not too long..
  if (strlen(befbuf) > FSPATHLEN)
  {
    *befbuf = 0;
    putf(ms(m_fs_inputtoolong));
  }
  if (b->fsmode)
  {
    putv(LF);
    //oe3dzw: tmp-string is very short, cut off command if too long
    strcpy(temp, "fs: ");
    strncpy(temp + 4, command, FSPATHLEN - 5);
    temp[FSPATHLEN - 1] = 0;
    lastcmd(temp);
  }
  while (strchr(befbuf, _INVSEP)) *strchr(befbuf, _INVSEP) = _SEP;
  if (! strncmp(b->fspath, "off", 3)) getfirstpath(b->fspath);
#ifdef DRIVE_LETTERS
  if (befbuf[1] == ':' && befbuf[2] == 0)
  {
    char *pos;
    char *ptr;
    char tempallowed[FSPATHLEN+1];
    *befbuf |= 32;
    strcpy(tempallowed, m.fspath);
    strcat(tempallowed, " ");
    ptr = tempallowed;
    while ((pos = strstr(ptr, " ")) != NULL)
    {
      *pos = 0;
      if (*ptr == '+') ptr++;
      if (! strncmp(befbuf, ptr, 2))
      {
        strcpy(b->fspath, ptr);
        processpath(b->fspath);
        return;
      }
      ptr = pos + 1;
    }
    if (b->sysop)
    {
      strcpy(b->fspath, befbuf);
      processpath(b->fspath);
    }
    else
      putf(ms(m_fs_driveunavailable), befbuf);
    return;
  }
#endif
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  befbuf += blkill(befbuf);
  switch (cmd)
  {
  case path:
       {
         printallowed();
       } break;
  case dir:
  case ls:
       {
         strcpy(temp, b->fspath);
         scanoptions(befbuf);
         if (cmd == dir) b->optplus += o_l;
         befbuf += blkill(befbuf);
         if (addpath(temp, befbuf))
           if (pathallowed(temp, 1))
             showdir(temp, b->optplus);
       } break;
  case cd__:
       {
         strcpy(befbuf, "..");
       }
  case cd:
  case chdir_:
       {
         if (! *befbuf)
         {
           putf(ms(m_fs_currentpath), b->fspath);
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(b->fspath, befbuf))
         {
#ifdef __MSDOS__
           if (strlen(b->fspath) > 3)
             b->fspath[strlen(b->fspath) - 1] = 0;
#endif
           if (! pathallowed(b->fspath, 1))
             strcpy(b->fspath, temp);
           else
             if (! isdir(b->fspath))
             {
               putf(ms(m_dirunknown), b->fspath);
               strcpy(b->fspath, temp);
             }
#ifdef __MSDOS__
           if (b->fspath[strlen(b->fspath) - 1] != _SEP)
             strcat(b->fspath, _SEPS);
#endif
         }
       } break;
  case get:
  case read_:
  case type:
  case cat:
  case rtext:
       {
         if (! *befbuf)
         {
           putsyntax("GET", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 1) && fileexists(temp, 0))
           {
             fileio_text fio;
             fio.usefile(temp);
             fio.tx();
           }
         }
       } break;
  case bget:
  case bin:
  case rprg:
       {
         if (! *befbuf)
         {
            putsyntax("BGET", ms(m_filename));
            break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 1) && fileexists(temp, 0))
           {
             fileio_abin fio;
             fio.usefile(temp);
             fio.tx();
           }
        }
       } break;
#ifdef FEATURE_BINSPLIT
  case bsget:
       {
         if (! *befbuf)
         {
           putsyntax("BSGET", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 1) && fileexists(temp, 0))
           {
             //fileio_binsplit fio;
             //fio.set_sendname(temp);
             //fio.tx();
             putf(ms(m_notimplemented));
           }
         }
       } break;
#endif
  case bmail:
       {
         if (m.fsbmail == 1)
         {
           if (! *befbuf)
           {
             putsyntax("BMAIL", ms(m_filename));
             break;
           }
           strcpy(temp, b->fspath);
           if (addpath(temp, befbuf))
           {
#ifdef DRIVE_LETTERS
             if (strlen(temp) > 3)
#endif
               temp[strlen(temp) - 1] = 0;
             if (pathallowed(temp, 1) && fileexists(temp, 0))
             {
               char tempfilename[FNAMELEN+1];
               FILE *F;
               sprintf(tempfilename, TEMPPATH "/%s.imp", time2filename(0));
               strlwr(tempfilename);
               if (! (F = s_fopen(tempfilename, "swt")))
               {
                 trace(serious, "filesurf", "fopen %s errno=%d", tempfilename, errno);
                 return;
               }
               fprintf(F, "SP %s @ %s\n%s\n", b->logincall, b->at, befbuf);
               fprintf(F, ".attach %s\n", temp);
               fprintf(F, "impdel\n");
               s_fclose(F);
               fork(P_BACK | P_MAIL, 0, mbimport, tempfilename);
             }
           }
         }
         else
           putf(ms(m_fs_bmaildisabled));
       } break;
#ifdef FEATURE_YAPP
  case yget:
  case ryapp:
       {
         if (! *befbuf)
         {
            putsyntax("YGET", ms(m_filename));
            break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
            if (strlen(temp) > 3)
#endif
               temp[strlen(temp) - 1] = 0;
            if (pathallowed(temp, 1) && fileexists(temp, 0))
            {
              fileio_yapp fio;
              fio.usefile(temp);
              fio.tx();
            }
         }
       } break;
  case yput:
  case wyapp:
       {
         if (! *befbuf)
         {
           putsyntax("YPUT", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 2) && ! fileexists(temp, 1))
           {
             fileio_yapp fio;
             fio.usefile(temp);
             fio.rx();
           }
         }
       } break;
#endif
#ifdef FEATURE_DIDADIT
  case dget:
       {
         if (! *befbuf)
         {
           putsyntax("DGET", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 1) && fileexists(temp, 0))
           {
             fileio_dida fio;
             fio.usefile(temp);
             fio.tx();
           }
         }
       } break;
  case dput:
       {
         if (*befbuf)
         {
           putsyntax("DPUT", "");
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, "dummy"))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 2) && ! fileexists(temp, 1))
           {
             fileio_dida fio;
             fio.usefile(b->fspath); //attn: semantics differ from normal fio
             fio.rx();
           }
         }
       } break;
#endif
  case put:
  case wtext:
       {
         if (! *befbuf)
         {
           putsyntax("PUT", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 2) && ! fileexists(temp, 1))
           {
             fileio_text fio;
             fio.usefile(temp);
             fio.rx();
           }
         }
       } break;
  case bput:
  case wprg:
       {
         if (! *befbuf)
         {
           putsyntax("BPUT", ms(m_filename));
           break;
         }
         strcpy(temp, b->fspath);
         if (addpath(temp, befbuf))
         {
#ifdef DRIVE_LETTERS
           if (strlen(temp) > 3)
#endif
             temp[strlen(temp) - 1] = 0;
           if (pathallowed(temp, 2) && ! fileexists(temp, 1))
           {
             fileio_abin fio;
             fio.usefile(temp);
             fio.rx();
           }
         }
       } break;
  case help:
  case help_:
       {
         if (*befbuf)
           mbreadhelp(befbuf);
         else
         {
           mbreadhelp("fs");
           if (b->fsmode)
           {
             putv(LF);
             readtext("fshelp");
           }
           if (b->fsmode && b->sysop)
           {
             putv(LF);
             readtext("fsshelp");
           }
         }
       } break;
  case quit:
  case bye:
  case exit_:
       {
         if (b->fsmode) putf(ms(m_fs_end));
         b->fsmode = 0;
       } break;
  case cmdmode:
       {
         if (! b->fsmode)
         {
           putwelcome();
           b->fsmode = 1;
           break;
         }
       }
  case d_binrx:
  case d_binrx2:
  case d_bintx:
  case d_bintx2:
  case d_yapprx:
  case d_yapptx:
  case d_didaditrx:
  case d_didadittx:
  case d_gp:
  case d_wpp:
  case bin_ok: break;

  default:
       {
          b->fsmode++; // b->fsmode must be set for start_runutil
        if (
#ifdef RUNUTILS
            ! start_runutil(command) &&
#endif
            ! (b->sysop && sysopcmd(command)))
          if (*befbuf)
            putf(ms(m_cmdinvalid), befbuf);
        b->fsmode--;
       } break;
  }
}

/*---------------------------------------------------------------------------*/

void filesurf::putprompt ()
//*************************************************************************
//
//  Shows the FileSurf-prompt
//
//*************************************************************************
{
  if (! strncmp(b->fspath, "off", 3))
    getfirstpath(b->fspath);
  putf("FileSurf:%s> ", b->fspath);
}

/*---------------------------------------------------------------------------*/

void filesurf::getfirstpath (char *string)
//*************************************************************************
//
//  Puts the first allowed path to the given pointer
//
//*************************************************************************
{
  char tempallowed[FSPATHLEN+1];

  strcpy(tempallowed, m.fspath);
  strcat(tempallowed, " ");
  *strstr(tempallowed, " ") = 0;
  if (*tempallowed == '+') strcpy(tempallowed, tempallowed + 1);
  strcpy(string, tempallowed);
  processpath(string);
}

/*---------------------------------------------------------------------------*/
#endif
