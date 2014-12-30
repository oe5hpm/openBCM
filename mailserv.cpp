/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------
  Mailinglist-Server for BCM
  --------------------------

  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980223 OE3DZW fixed fsetopt
//19980429 DH3MB  fixed several things...
//19980430 OE3DZW info/list are equal, both will show lists in detail
//                when called
//                removed texts under dos
//                included normal help
//                b->continous is always set
//                made commands shorter
//19980505 DH3MB  fixes for DOS-version, mailfile was corrupt
//19980505 OE3DZW several fixes
//19980527 DH3MB  fixes
//20000103 OE3DZW fixed, added semaphor for modifying list
//20000408 DF3VI  Hinzufuegen/Loeschen mehrerer User
//20000715 DK2UI  added missing options for readmail() for BIN
//20000821 DK2UI  cut off illegal input
//20010615 DF3VI  bug fix for ma_deleteuser by maintainer,
//                semaphore was not unlocked if no list file available
//20010620 DF3VI  ma_deleteuser in modify_group()
//20010621 DF3VI  ma_deletelist in modify_group()
//20010915 DK2UI  corrections of text messages
//20060724 DH8YMB renamed SERVER-LIST -> GROUP

#include "baycom.h"

#ifdef MAILSERVER

/*---------------------------------------------------------------------------*/

void mailserv::parse_listheader (char *line, char **name, char **number,
                                 char **options, char **description)
//*************************************************************************
//
//  Extracts the data from a mailing list header and stores the field
//  to the according pointers
//
//*************************************************************************
// eg: TEST 21 DESCRIPTION
{
  *name = line;
  if (! strchr(line, ' '))
    strcat(line, " 1 "); //append number 1 if no number in list
  rm_crlf(line);
  *(*number = strchr(line, ' ')) = 0;
  strupr(*name);
  while (*(++*number) == ' ') **number = 0;
  if ((*options = strchr(*number, ' ')) != 0)
  {
    **options = 0;
    while (*(++*options) == ' ') **options = 0;
    if (**options == '-')
    {
      if ((*description = strchr(*options, ' ')) != 0)
      {
        **description = 0;
        while (*(++*description) == ' ') **description = 0;
      }
      else *description = line + strlen(line);
    }
    else
    {
      *description = *options;
      *options = line + strlen(line);
    }
  }
  else
    *options = *description = line + strlen(line);
  if (**description)
    while ((*description)[strlen(*description) - 1] == ' ')
      (*description)[strlen(*description) - 1] = 0;
}

/*---------------------------------------------------------------------------*/

char mailserv::seek_group (FILE *fold, FILE *f, char what, char *parameter)
//*************************************************************************
//
//  Moves the file-pointer to the searched string and copies the
//  file to f, if f!=0
//
//  If searching an user, the file-pointer must point to the header
//  of the mailing list
//
//  If searching an mailing list header, the options are automatically
//  stored to b->optplus
//
//*************************************************************************
{
  char s[128], line[128];
  char *s2;
  long posbuf;

  if (what == ma_seek_user)
  {
    fgets(line, sizeof(line) - 1, fold);
    if (f) fprintf(f, "%s", line);
  }
  posbuf = ftell(fold);
  while (fgets(line, sizeof(line) - 1, fold) && ! feof(fold))
  {
    strcpy(s, line);
    rm_crlf(s);
    switch (what)
    {
    case ma_seek_list:
      if (*s != ' ')
      {
        if ((s2 = strchr(s, ' ')) != 0) *s2++ = 0;
        if (! stricmp(s, parameter))
        {
          b->optplus = 0;
          if (s2)
          {
            while (*s2 == ' ' || isdigit(*s2)) s2++;
            if (*s2 == '-') scanoptions(s2);
          }
          fseek(fold, posbuf, SEEK_SET);
          return 1;
        }
      }
      if (f) fprintf(f, "%s", line);
      break;
    case ma_seek_user:
      if (*s == ' ')
      {
        while (*s == ' ') strcpy(s, s + 1);
        subst1(s, ' ', 0);
        strupr(s);
        if (! strcmp(s, parameter))
        {
          fseek(fold, posbuf, SEEK_SET);
          if (line[1] == ' ') return 2;
          else return 1;
        }
        else
          if (f) fprintf(f, "%s", line);
      }
      else
      {
        fseek(fold, posbuf, SEEK_SET);
        return 0;
      }
      break;
    case ma_seek_end:
      if (f) fprintf(f, "%s", line);
      break;
    default:
      trace(serious, "mailsv", "invalid seek-code");
      break;
    }
    posbuf = ftell(fold);
    waitfor(e_ticsfull);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void mailserv::modify_group (char what, char *listname, char *parameter)
//*************************************************************************
//
//  Changes something in the mailserv groups
//
//*************************************************************************
{
  char mybbs[HADRESSLEN+1];
  char buffer[90];
  char *name, *number, *options, *description;
  char *s;
  char numok = 1;
  char i;

  strcpy(mybbs, "");
  while (! sema_lock(MSMODSEMA)) wdelay(3094);
  FILE *f = s_fopen(MAILLISTTMPNAME, "swt");
  if (f)
    s_fsetopt(f, 1);
  else
    trace(fatal, "mailsv:mod", "nopen tmp-file");
  FILE *fold = s_fopen(MAILLISTNAME, "srt");
  if (! fold)
    fold = s_fopen(MAILLISTNAME, "swt"); //dummy for opening a file, stupid...
  if (! fold)
    trace(fatal, "mailsv:mod", "nopen mail-lisl");
  switch (what)
  {
    case ma_creategroup:
      if ((m.maillistserv == 2) && ! b->sysop)
      {
        putf(ms(m_sysoponly));
        break;
      }
      if (seek_group(fold, f, ma_seek_list, listname))
        putf(ms(m_groupexists), listname);
      else
      {
        fprintf(f, "%s 1 \n", listname);
        // if (! b->sysop)
        fprintf(f, "  %s\n", b->logincall);
        putf(ms(m_groupcreated), listname);
      }
      break;
    case ma_deletegroup:
      if (! b->sysop)
      {
        putf(ms(m_sysoponly));
        break;
      }
      if (seek_group(fold, f, ma_seek_list, listname))
      {
        seek_group(fold, 0, ma_seek_user, "dummy"); // seek to end of this list
        putf(ms(m_groupremoved), listname);
      }
      else
        putf(ms(m_groupnotexist), listname);
      break;
    case ma_setdescription:
      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (! b->sysop && (seek_group(fold, 0, ma_seek_user, u->call) != 2))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        fgets(buffer, sizeof(buffer) - 1, fold);
        rm_crlf(buffer);
        parse_listheader(buffer, &name, &number, &options, &description);
        while (*parameter == '-') parameter++;
        parameter[60] = 0;
        fprintf(f, "%s %s%s%s %s\n", name, number, *options ? " " : "",
                                           options, parameter);
        putf(ms(m_groupnewdesc), listname, parameter);
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_setoptions:
      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (! b->sysop && (seek_group(fold, 0, ma_seek_user, u->call) != 2))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        // Clean up the given options
        qsort(parameter, strlen(parameter), 1,
              (int(*) (const void *, const void *)) strcmp);
        s = parameter;
        while (*s)
          if (strchr(s + 1, *s) || (*s < 'A') || (*s > 'Z')) strcpy(s, s + 1);
          else s++;
        // parameter now only consists of letters in alphabetical order
        fgets(buffer, sizeof(buffer) - 1, fold);
        rm_crlf(buffer);
        parse_listheader(buffer, &name, &number, &options, &description);
        if (*parameter)
        {
          fprintf(f, "%s %s -%s %s\n", name, number, parameter, description);
          putf(ms(m_groupoptionset), listname, parameter);
        }
        else
        {
          fprintf(f, "%s %s %s\n", name, number, description);
          putf(ms(m_groupoptionreset), listname);
        }
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_setnumber:
      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (! b->sysop && (seek_group(fold, 0, ma_seek_user, u->call) != 2)
            && strcmp(b->uplink, "Mailsv"))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        for (i = 0; i < strlen(parameter); i++)
          if (! isdigit(parameter[i])) numok = 0;
        if (! numok)
        {
          putf(ms(m_invalidnr));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        fgets(buffer, sizeof(buffer) - 1, fold);
        rm_crlf(buffer);
        parse_listheader(buffer, &name, &number, &options, &description);
        fprintf(f, "%s %s%s%s %s\n", name, parameter, *options ? " " : "",
                                     options, description);
        putf(ms(m_msv_serialset), listname, parameter);
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_subscribe:
    case ma_adduser:
      if (! strchr(parameter, '@') && mbcallok(parameter))
      {
        get_mybbs(parameter, mybbs, 0);
        if (! *mybbs)
        {
          putf(ms(m_isunknown), parameter);
          break;
        }
      }

      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (   (b->optplus & o_c)
            && (what == ma_subscribe)
            && ! (b->sysop && strcmp(b->uplink, "Import")))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        if (   what == ma_adduser && ! b->sysop
            && (seek_group(fold, 0, ma_seek_user, u->call) != 2))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        if (seek_group(fold, f, ma_seek_user, parameter))
        {
          if (what == ma_adduser)
            putf(ms(m_alreadyuseringroup), parameter, listname);
          else
            putf(ms(m_youalreadyuseringroup), listname);
        }
        else
        {
          fprintf(f, " %s\n", parameter);
          if (what == ma_adduser)
            putf(ms(m_nowuseringroup), parameter, listname);
          else
            putf(ms(m_younowuseringroup), listname);
        }
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_unsubscribe:
    case ma_deleteuser:
      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (   what == ma_deleteuser && ! b->sysop
            && (seek_group(fold, 0, ma_seek_user, u->call) != 2))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        if (seek_group(fold, f, ma_seek_user, parameter))
        {
          fgets(buffer, sizeof(buffer) - 1, fold);
          if (what == ma_deleteuser)
            putf(ms(m_userdeleteingroup), parameter, listname);
          else
            putf(ms(m_youdeleteingroup), listname);
        }
        else
        {
          if (what == ma_deleteuser)
            putf(ms(m_nouseringroup), parameter, listname);
          else
            putf(ms(m_younouseringroup), listname);
        }
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_addmaintainer:
      if (! strchr(parameter, '@') && mbcallok(parameter))
      {
        get_mybbs(parameter, mybbs, 0);
        if (! *mybbs)
        {
          putf(ms(m_isunknown), parameter);
          break;
        }
      }
      if (seek_group(fold, 0, ma_seek_list, listname))
      {
        if (! b->sysop && (seek_group(fold, 0, ma_seek_user, u->call) != 2))
        {
          putf(ms(m_onlymaintainer));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, ma_seek_list, listname);
        switch (seek_group(fold, f, ma_seek_user, parameter))
        {
        case 1: fgets(buffer, sizeof(buffer) - 1, fold);
                // fall through
        case 0: putf(ms(m_msv_nowmaint), parameter, listname);
                break;
        case 2: putf(ms(m_msv_alreadymaint), parameter, listname);
                fgets(buffer, sizeof(buffer) - 1, fold);
                break;
        }
        fprintf(f, "  %s\n", parameter);
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case ma_deletemaintainer:
      if (! b->sysop)
      {
        putf(ms(m_onlymaintainer));
        break;
      }
      rewind(fold);
      if (seek_group(fold, f, ma_seek_list, listname))
      {
        if (seek_group(fold, f, ma_seek_user, parameter) == 2)
        {
          fgets(buffer, sizeof(buffer) - 1, fold);
          fprintf(f, " %s\n", parameter);
          putf(ms(m_msv_nomaint), parameter, listname);
        }
        else
          putf(ms(m_msv_notmaint), parameter, listname);
      }
      else
      {
        putf(ms(m_groupnotexist), listname);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    default: trace(serious, "maillistserv", "illegal modify-code"); break;
  }
  seek_group(fold, f, ma_seek_end, "");
  if (fold) s_fclose(fold);
  s_fclose(f);
  xunlink(MAILLISTNAME);
  xrename(MAILLISTTMPNAME, MAILLISTNAME);
  sema_unlock(MSMODSEMA);
}

/*---------------------------------------------------------------------------*/

void mailserv::list_groups (void)
//*************************************************************************
//
//  Shows all mailing lists
//
//*************************************************************************
{
  char s[255];
  char *p;
  int userc = 0, maintc = 0;

  while (! sema_lock(MSMODSEMA)) wdelay(3093);
  FILE *f = s_fopen(MAILLISTNAME, "srt");
  if (f)
  {
    while (fgets(s, sizeof(s) - 1, f) || (maintc + userc))
    {
      if (*s) rm_crlf(s);
      if (*s != ' ')
      {
        if (maintc + userc)
          putf(ms(m_msv_userc), userc, maintc);
        userc = 0;
        maintc = 0;
        if ((p = strchr(s, ' ') + 1) != (char *) 1)
        {
          while (*p == ' ') p++;
          *(p - 1) = 0;
        }
        if (*s)
        {
          strupr(s);
          putf("%-10s", s);
        }
        if ((p != (char *) 1) && ((p = strchr(p, ' ') + 1) != (char *) 1))
        {
          scanoptions(p);
          p += blkill(p);
          if (*p) putf(" - %s", p);
        }
        if (*s) putv(LF);
      }
      else
      {
        if (s[1] == ' ') maintc++;
        else userc++;
      }
      *s = 0;
      waitfor(e_ticsfull);
    }
    s_fclose(f);
  }
  else
    putf(ms(m_nogroupactive));
  sema_unlock(MSMODSEMA);
}

/*---------------------------------------------------------------------------*/

void mailserv::show_info (char *name, char longformat)
//*************************************************************************
//
//  Shows users, maintainers and options of a mailing list
//
//*************************************************************************
{
  char s[256];
  char format[6];
  char cols;
  char row;
  int userc = 0, maintc = 0, i;
  char maxlen = 6;
  char *userlist;
  char *listname, *number, *options, *description;

  //do not sema-lock here, this info is read during distribution!
  FILE *f = s_fopen(MAILLISTNAME, "lrt");
  if (f && seek_group(f, 0, ma_seek_list, name))
  {
    fgets(s, sizeof(s) - 1, f);
    rm_crlf(s);
    parse_listheader(s, &listname, &number, &options, &description);
    if (longformat)
    {
      putf(ms(m_nameofgroup), listname);
      putf(ms(m_msv_nextnr), number);
    }
    if (longformat && *options)
    {
      scanoptions(options);
      if (b->optplus & (o_c | o_m | o_u))
      {
        putf(ms(m_msv_options));
        putf(" ");
        if (b->optplus & o_c) putf("C");
        if (b->optplus & o_m) putf("M");
        if (b->optplus & o_u) putf("U");
        putv(LF);
      }
    }
    if (*description)
    {
      if (longformat)
        putf(ms(m_description), description);
      else
        putf(ms(m_groupdesc), description);
    }
    while (fgets(s, sizeof(s) - 1, f))
    {
      rm_crlf(s);
      if (*s != ' ') break;
      if (s[1] == ' ') maintc++;
      else
      {
        userc++;
        while (*s == ' ') strcpy(s, s + 1);
        if (strlen(s) > maxlen) maxlen = strlen(s);
      }
    }
    if (maintc + userc)
    {
      maxlen += 2; // for the 0-byte and the user/maintainer-byte
      rewind(f);
      seek_group(f, 0, ma_seek_list, name);
      userlist = (char *) t_malloc(maxlen * (userc + maintc), "msvs");
      fgets(s, sizeof(s) - 1, f);
      for (i = 0; i < (userc + maintc); i++)
      {
        fgets(s, sizeof(s) - 1, f);
        rm_crlf(s);
        if (s[1] == ' ')
          *(userlist + i * maxlen) = 0;
        else
          *(userlist + i * maxlen) = 1;
        while (*s == ' ') strcpy(s, s + 1);
        strcpy(userlist + i * maxlen + 1, s);
      }
      qsort(userlist, userc + maintc, maxlen,
            (int(*) (const void *, const void *)) strcmp);
      cols = 79 / (maxlen - 1);
      sprintf(format, "%%-%ds", maxlen - 1);
      if (maintc)
      {
        putf(ms(m_msv_maints), maintc);
        row = 1;
        for (i = 0; i < maintc; i++)
        {
          if (! (i % 11) && i)
          {
            putv(LF);
            row++;
            //if (! longformat && row==6) //Better use a bulletin then..
            //{
            //  putf("(Not all listed)");
            //  break;
            //}
          }
          putf("%-7s", userlist + i * maxlen + 1);
        }
        putv(LF);
      }
      if (userc)
      {
        putf(ms(m_groupusers), userc);
        row = 1;
        for (i = 0; i < userc; i++)
        {
          if (! (i % cols) && i)
          {
            putv(LF);
            row++;
            //if (! longformat && row==6)
            //{
            //  putf("(Not all listed)");
            //  break;
            //}
          }
          putf(format, userlist + (i + maintc) * maxlen + 1);
        }
        putv(LF);
      }
      t_free(userlist);
    }
    s_fclose(f);
  }
  else
    putf(ms(m_groupnotexist), name);
}

/*---------------------------------------------------------------------------*/

void mailserv::execute (char *command)
//*************************************************************************
//
//  Executes an user-command destined to the mailing
//  list server subsystem
//
//*************************************************************************
{
  if (! m.maillistserv)
  {
    putf(ms(m_mailservdisabled));
    return;
  }
  char *befbuf;
  char *para2;
  int i, callcount;
  char callpos[MAXPARSE];

  befbuf = command;

  static char *beftab[] =
    {  "NEWGROUP", "+G", "DELGROUP", "-G",
       "DESCRIPTION", "OPTIONS",
       "ADDUSER", "+U", "DELUSER", "-U",
       "ADDMAINTAINER", "+M", "DELMAINTAINER", "-M",
       "INFO", "SETNUMBER", "LIST",
       "SUBSCRIBE", "UNSUBSCRIBE",
       "HELP", NULL
    };

  enum befnum
    { unsinn,
      newgroup, newgroup_, delgroup, delgroup_,
      description, options,
      adduser, adduser_, deluser, deluser_,
      addmaintainer, addmaintainer_, delmaintainer, delmaintainer_,
      info, setnumber, list,
      subscribe, unsubscribe,
      help
    } cmd = unsinn;

  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  befbuf += blkill(befbuf);
  switch (cmd)
  {
  case newgroup:
  case newgroup_:
    if (! *befbuf || strchr(befbuf, ' ') || strlen(befbuf) > 20)
    {
      putf("Syntax: MAILserver +G <group>\n");
      break;
    }
    if ((para2 = strchr(befbuf, ' ')) != NULL) *para2++ = 0;
    strupr(befbuf);
    modify_group(ma_creategroup, befbuf, "");
    break;
  case delgroup:
  case delgroup_:
    if (! *befbuf)
    {
      putf("Syntax: MAILserver -G <group>\n");
      break;
    }
    if ((para2 = strchr(befbuf, ' ')) != NULL) *para2++ = 0;
    strupr(befbuf);
    modify_group(ma_deletegroup, befbuf, "");
    break;
  case description:
    if (! (para2 = strchr(befbuf, ' ')))
    {
      putf("Syntax: MAILserver DEScription <group> <text>\n");
      break;
    }
    *para2++ = 0;
    strupr(befbuf);
    modify_group(ma_setdescription, befbuf, para2);
    break;
  case options:
    if (! *befbuf)
    {
      putf("Syntax: MAILserver Options <group> [<options>]\n");
      break;
    }
    strupr(befbuf);
    if ((para2 = strchr(befbuf, ' ')) != NULL) *para2++ = 0;
    else para2 = &befbuf[strlen (befbuf)]; // empty string
    modify_group(ma_setoptions, befbuf, para2);
    break;
  case adduser:
  case adduser_:
    strupr(befbuf);
    callcount = parseline(befbuf, callpos);
    if (callcount < 2 || callcount == MAXPARSE)
    {
      putf("Syntax: MAILserver +U <group> <call|board>\n");
      break;
    }
    for (i = 1; i < callcount; i++)
      modify_group(ma_adduser, befbuf, befbuf + callpos[i]);
    break;
  case deluser:
  case deluser_:
    strupr(befbuf);
    callcount = parseline(befbuf, callpos);
    if (callcount < 2 || callcount == MAXPARSE)
    {
      putf("Syntax: MAILserver -U <group> <call|board>\n");
      break;
    }
    for (i = 1; i < callcount; i++)
      modify_group(ma_deleteuser, befbuf, befbuf + callpos[i]);
    break;
  case addmaintainer:
  case addmaintainer_:
    if (! (para2 = strchr(befbuf, ' ')))
    {
      putf("Syntax: MAILserver +M <group> <call|board>\n");
      break;
    }
    strupr(befbuf);
    *para2++ = 0;
    modify_group(ma_addmaintainer, befbuf, para2);
    break;
  case delmaintainer:
  case delmaintainer_:
    if (! (para2 = strchr(befbuf, ' ')))
    {
      putf("Syntax: MAILserver -M <group> <call/board>\n");
      break;
    }
    strupr(befbuf);
    *para2++ = 0;
    modify_group(ma_deletemaintainer, befbuf, para2);
    break;
  case info:
  case list:
    if (*befbuf)
    {
      if ((para2 = strchr(befbuf, ' ')) != NULL) *para2++ = 0;
      strupr(befbuf);
      show_info(befbuf, 1);
    }
    else list_groups();
    break;
  case setnumber:
    if (! (para2 = strchr(befbuf, ' ')))
    {
      putf("Syntax: MAILserver SEtnumber <group> <number>\n");
      break;
    }
    *para2++ = 0;
    strupr(befbuf);
    modify_group(ma_setnumber, befbuf, para2);
    break;
  case subscribe:
    if (! *befbuf)
    {
      putf("Syntax: MAILserver SUbscribe <group>\n");
      break;
    }
    strupr(befbuf);
    modify_group(ma_subscribe, befbuf, u->call);
    break;
  case unsubscribe:
    if (! *befbuf)
    {
      putf("Syntax: MAILserver Unsubscribe <group>\n");
      break;
    }
    if ((para2 = strchr(befbuf, ' ')) != NULL) *para2++ = 0;
    strupr(befbuf);
    modify_group(ma_unsubscribe, befbuf, u->call);
    break;
  case help:
    if (*befbuf) mbreadhelp(befbuf);
    else mbreadhelp("mailserver");
     //readtext("mahelp");
    break;
  default:
    putf("Syntax: MAILserver +G | -G | +U | -U | +M | -M | I | SE | SU | U | H\n");
    //readtext("mahelp");
    break;
  }
}

/*---------------------------------------------------------------------------*/

void mailserv::examine_mail (void)
//*************************************************************************
//
//  Checks, if a mail is destined to the mailserver
//  subsystem and processes the mail (called within send.cpp)
//
//*************************************************************************
{
  int usercontinous = 0;
  char *command;
  char *listname;
  char *newtitle;
  char tempfilename[FNAMELEN+1];
  FILE *serverfile;
  FILE *mailfile;
  unsigned long int lastrline = 0L;
  char distribnum[11];
  char *name, *number, *options, *description;
  char *betreffbuf = (char *) t_malloc(BETREFFLEN + 1, "msva");
  char *betreffbak = (char *) t_malloc(BETREFFLEN + 1, "msvb");

  strcpy(betreffbuf, b->betreff);
  strcpy(betreffbak, b->betreff);
  command = betreffbuf;
  if ((listname = strchr(betreffbuf, ' ')) != NULL) *listname++ = 0;
  else return;
  if ((newtitle = strchr(listname, ' ')) != NULL) *newtitle++ = 0;
  else newtitle = command + strlen(command); // Move pointer to a 0-byte
  strupr(command);
  strupr(listname);
  waitfor(e_ticsfull);
  if (   ! strcmp(command, "MAILTO")
      || ! strcmp(command, "SUBSCRIBE")
      || ! strcmp(command, "UNSUBSCRIBE") )
  {
    lastcmd("Mailserver");
    strcpy(b->uplink, "Mailsv");
    strcpy(t->name, ".");
    b->forwarding = fwd_none;
    markerased('W', 0, 0);
    sprintf(tempfilename, TEMPPATH "/%s", time2filename(0));
    strlwr(tempfilename);
    if (! (b->outputfile = s_fopen(tempfilename, "swt")))
    {
      trace(serious, "mailsv:ex", "fopen %s errno=%d %s",
                     tempfilename, errno, strerror(errno));
      return;
    }
    usercontinous = b->continous;
    b->continous = 1;
    b->oldoutput = t->output;
    t->output = io_file;
    switch (*command)
    {
    case 'M':
      while (! sema_lock(MSMODSEMA)) wdelay(3096);
      if (! (serverfile = s_fopen(MAILLISTNAME, "srt")))
      {
        putf("SEND %s Re: %s\n", b->herkunft, betreffbak);
//      putf("Sorry - there are no mailing lists available at this bbs.\n");
        putf(ms(m_notavailable), "MAILSERVER");
        break;
      }
      if (! seek_group(serverfile, 0, ma_seek_list, listname))
      {
        putf("SEND %s Re: %s\n", b->herkunft, betreffbak);
        putf(ms(m_groupnotexist), listname);
        s_fclose(serverfile);
        sema_unlock(MSMODSEMA);
        break;
      }
      if (b->optplus & o_m)
      {
        if (seek_group(serverfile, 0, ma_seek_user, b->herkunft) != 2)
        {
          putf("SEND %s Re: %s\n", b->herkunft, betreffbak);
          putf("Only maintainers may write a mail to the list %s.\n",
               listname);
          s_fclose(serverfile);
          sema_unlock(MSMODSEMA);
          break;
        }
        else
        {
          fseek(serverfile, 0, SEEK_SET);
          seek_group(serverfile, 0, ma_seek_list, listname);
        }
      }
      if (b->optplus & o_u)
      {
        if (! seek_group(serverfile, 0, ma_seek_user, b->herkunft))
        {
          putf("SEND %s Re: %s\n", b->herkunft, betreffbak);
          putf("Only users may write a mail to the list %s.\n", listname);
          s_fclose(serverfile);
          sema_unlock(MSMODSEMA);
          break;
        }
        else
        {
          fseek(serverfile, 0, SEEK_SET);
          seek_group(serverfile, 0, ma_seek_list, listname);
        }
      }
      show_info(listname, 0);
      putf("\nFor server redistribution write a mail to %s with title\n",
           m.boxname);
      putf("\"MAILTO %s <title of the mail to be distributed>\".\n",
           listname);
      putf(SEPLINE "\n");
      mailfile = s_fopen(b->mailpath, "srt");
      // Throw away the header and all the R:-lines, but the last one
      do
        fgets(b->line, BUFLEN - 1, mailfile);
      while (strncmp(b->line, "R:", 2));
      do
      {
        if (! strncmp(b->line, "R:", 2))
          lastrline = ftell(mailfile) - strlen(b->line);
        fgets(b->line, BUFLEN - 1, mailfile);
      }
      while (! strncmp(b->line, "R:", 2));
      fseek(mailfile, lastrline, SEEK_SET);
      loaduser(m.boxname, u, 0);
      b->optgroup = o_ur;
      formoptions();
      readmail(mailfile, 0); // mailfile is closed within readmail()
      s_fclose(b->outputfile);
      fgets(b->line, BUFLEN - 1, serverfile);
      parse_listheader(b->line, &name, &number, &options, &description);
      strncpy(distribnum, number, 10);
      distribnum[10] = 0;
      if (m.maillistsender)
        strcpy(b->logincall, m.boxname);
      else
      {
        strcpy(b->logincall, b->herkunft);
        loaduser(b->logincall, u, 0);
      }
      t->output = io_dummy;
      while (fgets(b->line, BUFLEN - 1, serverfile))
      {
        if (*b->line != ' ') break; // End of mailing list?
        rm_crlf(b->line);
        if (m.userpw >= 2 && ! b->pwok && ! b->sysop)
        { //Nachricht wird nur LOKAL gespeichert
          if (strchr(b->line, '@')) *strchr(b->line, '@') = 0;
          sprintf(b->line + strlen(b->line), " @ %s", m.boxname);
        }
        b->inputfile = s_fopen(tempfilename, "srt");
        b->oldinput = t->input;
        t->input = io_file; // Will be restored by mbsend()
        b->mailtype = '?'; // Sonst kein Bulletin-Verteiler!
        // Lifetime von Mail an Server nehmen!
        sprintf(b->line + strlen(b->line), " #%d (%s %s) %s",
                b->lifetime, listname, distribnum, newtitle);
        if (! mbsend(b->line, 0))
        {
          t->input = (io_t) b->oldinput;
          b->oldinput = io_dummy;
          trace(serious, "mailsv:ex", "err send %s", b->line);
        }
        if (b->inputfile) s_fclose(b->inputfile);
        waitfor(e_ticsfull);
      }
      s_fclose(serverfile);
      sema_unlock(MSMODSEMA);
      xunlink(tempfilename);
      sprintf(b->line, "%ld", atol(distribnum) + 1);
      modify_group(ma_setnumber, listname, b->line);
      // The rest of the function needs this file to be open
      b->outputfile = s_fopen(tempfilename, "swt");
      break;
    case 'S':
    case 'U':
      putf("SEND %s Re: %s\n", b->herkunft, betreffbak);
      if (*command == 'S')
        modify_group(ma_subscribe, listname, b->herkunft);
      else
        modify_group(ma_unsubscribe, listname, b->herkunft);
      break;
    }
    putf("nnnn\nimpdel\n");
    s_fclose(b->outputfile);
    mbimport(tempfilename);
    t->output = (io_t) b->oldoutput;
    xunlink(tempfilename);
    b->continous = usercontinous;
  }
}

/*---------------------------------------------------------------------------*/

#endif
