/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------
  Pocsag-Server for BCM
  ---------------------

  Copyright (C)  Patrick, DF3VI
  Auf Grundlage des Maillistservers von Jonny, DH3MB

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/
//20010915 DK2UI  some corrections
//20030410 DH6BB  User mit SSID auch anpagebar.
//20030506 DH6BB  BugFix im SSID-Teil
//20060724 DH8YMB rename all SERVER-LIST -> GROUP

#include "baycom.h"

#ifdef DF3VI_POCSAG

/*---------------------------------------------------------------------------*/

void pocsagserv::parse_listheader (char *line, char **name,
                                   char **description)
//*************************************************************************
//
//  Extracts the data from a pocsag server list header and stores the field
//  to the according pointers
//
//*************************************************************************
// eg: DB0KOE DESCRIPTION
{
  *name = line;
  rm_crlf(line);
  if ((*description = strchr(*name, ' ')) != 0)
  {
    **description = 0;
    while (*(++*description) == ' ') **description = 0;
  }
  else *description = line + strlen(line);
  if (**description)
    while ((*description)[strlen(*description) - 1] == ' ')
      (*description)[strlen(*description) - 1] = 0;
}

/*---------------------------------------------------------------------------*/

char pocsagserv::seek_group (FILE *fold, FILE *f, char what, char *parameter,
                            char *user)
//*************************************************************************
//
//  Moves the file-pointer to the searched string and copies the
//  file to f, if f!=0
//
//  If searching an entry, the file-pointer must point to the header
//  of the pocsag server list
//
//*************************************************************************
{
  char s[128], line[128];
  static char server[CALLEN+1];
  char *s2;
  long posbuf;

  if (what == pocsag_seek_user)
  {
    fgets(line, 127, fold);
    if (f) fprintf(f, "%s", line);
  }
  posbuf = ftell(fold);
  while (! feof(fold) && fgets(line, 127, fold))
  {
    strcpy(s, line);
    rm_crlf(s);
    switch (what)
    {
    case pocsag_seek_entry:
      if (*s != ' ')
      {
        if ((s2 = strchr(s, ' ')) != 0) *s2++ = 0;
        strcpy(server, s);
      }
      else
      {
        while (*s == ' ') strcpy(s, s+1);
        if (strchr(s, ' ')) *strchr(s, ' ') = 0;
        strupr(s);
        strcpy(user, s); // user mit SSID sichern
        subst(s, '-', 0); // SSID zum vergleich abtrennen
        if (! stricmp(s, parameter))
        { // Position hier NICHT zuruecksetzen!!!
          strcpy(parameter, server);
          return OK; // server;
        }
        user[0]=0;
      }
      break;
    case pocsag_seek_group:
      if (*s != ' ')
      {
        if ((s2 = strchr(s, ' ')) != 0) *s2++ = 0;
        if (! stricmp(s, parameter))
        {
          fseek(fold, posbuf, SEEK_SET);
          return OK; // parameter;
        }
      }
      if (f) fprintf(f, "%s", line);
      break;
    case pocsag_seek_user:
      if (*s == ' ')
      {
        while (*s == ' ') strcpy(s, s+1);
        if (strchr(s, ' ')) *strchr(s, ' ') = 0;
        strupr(s);
        if (! stricmp(s, parameter))
        {
          fseek(fold, posbuf, SEEK_SET);
          return OK; // parameter;
        }
        else
          if (f) fprintf(f, "%s", line);
      }
      else
      {
        fseek(fold, posbuf, SEEK_SET);
        return NO;
      }
      break;
    case pocsag_seek_end:
      if (f) fprintf(f, "%s", line);
      break;
    default:
      trace(serious, "pocsagserver", "invalid seek-code");
      break;
    }
    posbuf = ftell(fold);
    waitfor(e_ticsfull);
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

void pocsagserv::modify_group (char what, char *servername, char *parameter)
//*************************************************************************
//
//  Changes something in the pocsag server list settings
//
//*************************************************************************
{
  char buffer[90];
  char *name, *description;

  while (! sema_lock(POCSAGSEMA)) wdelay(934);
  FILE *f = s_fopen(POCSAGLISTTMP, "swt");
  if (f)
    s_fsetopt(f, 1);
  else
    trace(fatal, "pogsagsv:mod", "nopen tmp-file");
  FILE *fold = s_fopen(POCSAGLISTNAME, "srt");
  if (! fold)
    fold = s_fopen(POCSAGLISTNAME, "swt"); //dummy for opening a file, stupid
  if (! fold)
    trace(fatal, "pocsagsv:mod", "nopen server-list");
  switch (what)
  {
    case pocsag_creategroup:
      if ((m.pocsaglistserv == 1) && ! b->sysop)
      {
        putf(ms(m_sysoponly));
        break;
      }
      if (seek_group(fold, f, pocsag_seek_group, servername, ""))
        putf(ms(m_groupexists), servername);
      else
      {
        fprintf(f, "%s\n", servername);
        putf(ms(m_groupcreated), servername);
      }
      break;
    case pocsag_deletegroup:
      if (! b->sysop)
      {
        putf(ms(m_sysoponly));
        break;
      }
      if (seek_group(fold, f, pocsag_seek_group, servername, ""))
      { // seek to end of this list
        seek_group(fold, 0, pocsag_seek_user, "", "");
        putf(ms(m_groupremoved), servername);
      }
      else
      {
        putf(ms(m_groupnotexist), servername);
//      fseek(fold, 0, SEEK_SET);
      }
      break;
    case pocsag_setdescription:
      if (seek_group(fold, 0, pocsag_seek_group, servername, ""))
      {
        if (! b->sysop)
        {
          putf(ms(m_sysoponly));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, pocsag_seek_group, servername, "");
        fgets(buffer, sizeof(buffer) - 1, fold);
        rm_crlf(buffer);
        parse_listheader(buffer, &name, &description);
        while (*parameter == '-') parameter++;
        parameter[60] = 0;
        fprintf(f, "%s %s\n", name, parameter);
        putf(ms(m_groupnewdesc), servername, parameter);
      }
      else
      {
        putf(ms(m_groupnotexist), servername);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case pocsag_subscribe:
    case pocsag_adduser:
/*
      if (! strchr(parameter, '@') && mbcallok(parameter))
      {
        get_mybbs(parameter, mybbs, 0);
        if (! *mybbs)
        {
          putf(ms(m_isunknown), parameter);
          break;
        }
      }
*/
      if (seek_group(fold, 0, pocsag_seek_group, servername, ""))
      {
        if (what == pocsag_adduser && ! b->sysop)
        {
          putf(ms(m_sysoponly));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, pocsag_seek_group, servername, "");
        if (seek_group(fold, f, pocsag_seek_user, parameter, ""))
        {
          if (what == pocsag_adduser)
            putf(ms(m_alreadyuseringroup), parameter, servername);
          else
            putf(ms(m_youalreadyuseringroup), servername);
        }
        else
        {
          fprintf(f, " %s\n", parameter);
          if (what == pocsag_adduser)
            putf(ms(m_nowuseringroup), parameter, servername);
          else
            putf(ms(m_younowuseringroup), servername);
        }
      }
      else
      {
        putf(ms(m_groupnotexist), servername);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    case pocsag_unsubscribe:
    case pocsag_deleteuser:
      if (seek_group(fold, 0, pocsag_seek_group, servername, ""))
      {
        if (what == pocsag_deleteuser && ! b->sysop)
        {
          putf(ms(m_sysoponly));
          rewind(fold);
          break;
        }
        rewind(fold);
        seek_group(fold, f, pocsag_seek_group, servername, "");
        if (seek_group (fold, f, pocsag_seek_user, parameter, ""))
        {
          fgets(buffer, sizeof(buffer) - 1, fold);
          if (what == pocsag_deleteuser)
            putf(ms(m_userdeleteingroup), parameter, servername);
          else
            putf(ms(m_youdeleteingroup), servername);
        }
        else
        {
          if (what == pocsag_deleteuser)
            putf(ms(m_nouseringroup), parameter, servername);
          else
            putf(ms(m_younouseringroup), servername);
        }
      }
      else
      {
        putf(ms(m_groupnotexist), servername);
        fseek(fold, 0, SEEK_SET);
      }
      break;
    default:
      trace(serious, "pocsagserver", "illegal modify-code");
      break;
  }
  seek_group(fold, f, pocsag_seek_end, "", "");
  if (fold) s_fclose(fold);
  s_fclose(f);
  xunlink(POCSAGLISTNAME);
  xrename(POCSAGLISTTMP, POCSAGLISTNAME);
  sema_unlock(POCSAGSEMA);
}

/*---------------------------------------------------------------------------*/

void pocsagserv::list_groups (void)
//*************************************************************************
//
//  Shows all groups
//
//*************************************************************************
{
  char s[255];
  char *p;
  int userc = 0;

  while (! sema_lock(POCSAGSEMA)) wdelay(3093);
  FILE *f = s_fopen(POCSAGLISTNAME, "srt");
  if (f)
  {
    while (fgets(s, 255, f) || userc)
    {
      if (*s) rm_crlf(s);
      if (*s != ' ')
      {
        if (userc) putf(ms(m_groupusers), userc);
        userc = 0;
        if ((p = strchr(s, ' ') + 1) != (char *)1)
        {
          while (*p == ' ') p++;
          *(p - 1) = 0;
        }
        if (*s)
        {
          strupr(s);
          putf("%-10s", s);
        }
        if (*s) putf("\n");
      }
      else userc++;
      *s = 0;
      waitfor(e_ticsfull);
    }
    s_fclose(f);
  }
  else
    putf(ms(m_nogroupactive));
  sema_unlock(POCSAGSEMA);
}

/*---------------------------------------------------------------------------*/

void pocsagserv::show_info (char *name, char longformat)
//*************************************************************************
//
//  Shows users of a pocsag server list
//
//*************************************************************************
{
  char s[256];
  char format[6];
  char cols;
  char row;
  int  userc = 0, i;
  char maxlen = 6;
  char *userlist;
  char *servername, *description;

  //do not sema-lock here, this info is read during distribution!
  FILE *f = s_fopen(POCSAGLISTNAME, "lrt");
  if (f && seek_group(f, 0, pocsag_seek_group, name, ""))
  {
    fgets(s, 255, f);
    rm_crlf(s);
    parse_listheader(s, &servername, &description);
    if (longformat)
      putf(ms(m_nameofgroup), servername);
    if (*description)
    {
      if (longformat)
        putf(ms(m_description), description);
      else
        putf(ms(m_groupdesc), description);
    }
    while (fgets(s, 50, f))
    {
      rm_crlf(s);
      if (*s != ' ') break;
      {
        userc++;
        while(*s == ' ') strcpy(s, s+1);
        if (strlen(s) > maxlen) maxlen = strlen(s);
      }
    }
    if (userc)
    {
      maxlen += 2; // for the 0-byte and the user-byte
      rewind(f);
      seek_group(f, 0, pocsag_seek_group, name, "");
      userlist = (char *) t_malloc(maxlen * (userc), "pocs");
      fgets(s, 255, f);
      for (i = 0; i < userc; i++)
      {
        fgets(s, 50, f);
        rm_crlf(s);
        if (s[1] == ' ')
          *(userlist + (i * maxlen)) = 0;
        else
          *(userlist + (i * maxlen)) = 1;
        while (*s == ' ') strcpy(s, s+1);
        strcpy(userlist + (i * maxlen) + 1, s);
      }
      qsort(userlist, userc, maxlen,
            (int(*)(const void *, const void *)) strcmp);
      cols = 79 / (maxlen - 1);
      sprintf(format, "%%-%ds", maxlen - 1);
      if (userc)
      {
        putf(ms(m_poc_userc), userc);
        row = 1;
        for (i = 0; i < userc; i++)
        {
          if (! (i % cols) && i)
          {
            putf("\n");
            row++;
            //if (! longformat && row == 6)
            //{
            //  putf("(Not all listed)");
            //  break;
            //}
          }
          putf(format, userlist + (i * maxlen) + 1);
        }
        putf("\n");
      }
      t_free(userlist);
    }
    s_fclose(f);
  }
}

/*---------------------------------------------------------------------------*/

void pocsagserv::execute (char *command)
//*************************************************************************
//
//  Executes an user-command destined to the pocsag server
//
//*************************************************************************
{
  if (! m.pocsaglistserv)
  {
    putf(ms(m_poc_disabled));
    return;
  }
  char *befbuf;
  char *para2;
  int i, callcount;
  char callpos[MAXPARSE];
  char user[CALLSSID+1];
  char usercall[CALLSSID+1];
  int longform = 0;
  befbuf = command;
  static char *beftab[] =
    {  "NEWGROUP", "+G", "DELGROUP", "-G",
       "DESCRIPTION",
       "ADDUSER", "+U", "DELUSER", "-U",
       "INFO", "LIST",
       "SUBSCRIBE", "UNSUBSCRIBE",
       "HELP", "PAGE", NULL
    };
  enum befnum
    { unsinn,
      newgroup, newgroup_, delgroup, delgroup_,
      description,
      adduser, adduser_, deluser, deluser_,
      info, list,
      subscribe, unsubscribe,
      help, page
    } cmd = unsinn;

  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  befbuf += blkill(befbuf);
  switch (cmd)
  {
  case newgroup:
  case newgroup_:
    if (! *befbuf || ! mbcallok(befbuf))
    {
      putf("Syntax: POcsagserver +G <group>\n");
      break;
    }
    strupr(befbuf);
    modify_group(pocsag_creategroup, befbuf, "");
    break;
  case delgroup:
  case delgroup_:
    if (! *befbuf)
    {
      putf("Syntax: POcsagserver -G <group>\n");
      break;
    }
    strupr(befbuf);
    modify_group(pocsag_deletegroup, befbuf, "");
    break;
  case description:
    if (! strchr(befbuf, ' '))
    {
      putf("Syntax: POcsagserver DEScription <group> <text>\n");
      break;
    }
    para2 = strchr(befbuf, ' ') + 1;
    if (strchr(befbuf, ' ')) *strchr(befbuf, ' ') = 0;
    strupr(befbuf);
    modify_group(pocsag_setdescription, befbuf, para2);
    break;
  case adduser:
  case adduser_:
    strupr(befbuf);
    callcount = parseline(befbuf, callpos);
    if (callcount < 2 || callcount >= MAXPARSE)
    {
      putf("Syntax: POcsagserver +U <group> <user>\n");
      break;
    }
    for (i = 1; i < callcount; i++)
      modify_group(pocsag_adduser, befbuf, befbuf + callpos[i]);
    break;
  case deluser:
  case deluser_:
    strupr(befbuf);
    callcount = parseline(befbuf, callpos);
    if (callcount < 2 || callcount >= MAXPARSE)
    {
      putf("Syntax: POcsacserver -U <group> <user>\n");
      break;
    }
    for (i = 1; i < callcount; i++)
      modify_group(pocsag_deleteuser, befbuf, befbuf + callpos[i]);
    break;
  case info: longform = 1;
  case list:
    if (*befbuf)
    {
      strupr(befbuf);
      show_info(befbuf, longform);
    }
    else list_groups();
    break;
  case subscribe:
    if (! *befbuf)
    {
      putf("Syntax: POcsagserver Subscribe <group>\n");
      break;
    }
    strupr(befbuf);
    modify_group(pocsag_subscribe, befbuf, u->call);
    break;
  case unsubscribe:
    if (! *befbuf)
    {
      putf("Syntax: POcsagserver Unsubscribe <group>\n");
      break;
    }
    strupr(befbuf);
    modify_group(pocsag_unsubscribe, befbuf, u->call);
    break;
  case help:
    if (*befbuf) mbreadhelp(befbuf);
    else mbreadhelp("pocsag");
     //readtext("pochelp");
    break;
  case page:
    para2 = nextword(befbuf, usercall, CALLSSID);
    if (*para2)
    {
      //do not sema-lock here, this info is read during distribution!
      FILE *f = s_fopen(POCSAGLISTNAME, "lrt");
      if (f)
      {
        i = 0;
        strupr(usercall);
        strcpy(b->at, usercall);
        while (seek_group(f, 0, pocsag_seek_entry, b->at, user))
        {
          char tempfilename[FNAMELEN+1];
          FILE *F;
          sprintf(tempfilename, TEMPPATH "/%s", time2filename (0));
          strlwr(tempfilename);
          if (! (F = s_fopen(tempfilename, "swt")))
          {
            trace(serious, "pocsagsv", "fopen %s errno=%d", tempfilename, errno);
            return;
          }
          fprintf(F, "SP P @ %s\nMessage for %s\n", b->at, usercall);
          fprintf(F, "FROM  %s\n", u->call);
          fprintf(F, "TO    %s\n", user);
          fprintf(F, "TEXT  %s\n", para2);
          fprintf(F, "nnnn\nimpdel\n");
          s_fclose(F);
          mkboard("/", "P", 0);
          fork(P_BACK | P_MAIL, 0, mbimport, tempfilename); //import it as mail
          i++;
          strcpy(b->at, usercall);
        }
        if (i == 1)
          putf(ms(m_messagewillbe), ms(m_stored));
        else
          if (i > 1)
            putf(ms(m_messageswillbe), i, ms(m_stored));
          else
            putf(ms(m_poc_nouser), usercall);
        s_fclose(f);
      }
      else
        putf(ms(m_nogroupactive));
    }
    else
    {
      putf("Syntax: POcsacserver Page <call> <short text>\n");
      break;
    }
    break;
  default:
      putf("Syntax: POcsagserver +G | -G | +U | -U | I | P | S | U\n");
      //readtext("pochelp");
    break;
  }
}

/*---------------------------------------------------------------------------*/

void pocsagserv::examine_mail (void)
//*************************************************************************
//
//  Checks, if a mail is destinated to a user of a pocsag server
//  and processes the mail (called within send.cpp)
//
//*************************************************************************
{
  char username[DIRLEN+1];
  char user[CALLSSID+1];
  char servername[DIRLEN+1];
  char tempfilename[FNAMELEN+1];
  char mailfilename[FNAMELEN+1];
  FILE *serverfile;
  FILE *mailfile;
  FILE *F;

  if (! (serverfile = s_fopen(POCSAGLISTNAME, "srt")))
    return; // Keine Server-Datei
  strcpy(username, b->ziel);
  strcpy(servername, username);
  strcpy(mailfilename, b->mailpath);
  strcpy(user, b->ziel);
  waitfor(e_ticsfull);
  while (! sema_lock(POCSAGSEMA) && ! sema_lock(MSMODSEMA))
    wdelay(306);
  lastcmd("Pocsag list server");
  strcpy(b->uplink, "Pocsag");
  strcpy(t->name, ".");
  b->forwarding = fwd_none;
  while (seek_group(serverfile, 0, pocsag_seek_entry, servername, user))
  {
    sprintf(tempfilename, TEMPPATH "/%s", time2filename(0));
    strlwr(tempfilename);
    if (! (F = s_fopen(tempfilename, "swt")))
    {
      trace(serious, "pocsagsv", "fopen %s errno=%d", tempfilename, errno);
      sema_unlock(POCSAGSEMA);
   //   sema_unlock("forward");
      return;
    }
    mailfile = s_fopen(mailfilename, "srt");
    fgets(b->line, BUFLEN, mailfile);        // kopf-zeile
    mbsend_parse(b->line, 0);
    fgets(b->line, BUFLEN, mailfile);        // forward-zeile
    fgets(b->line, BUFLEN, mailfile);        // read-zeile
    fgets(b->betreff, BETREFFLEN, mailfile); // titel-zeile
    s_fclose(mailfile);
    fprintf(F, "SP P @ %s\nMail for %s\n", servername, b->ziel);
    fprintf(F, "FROM  %s\n", b->herkunft);
    fprintf(F, "TO    %s\n", user);
    fprintf(F, "AT    %s\n", b->at);
    fprintf(F, "BID   %s\n", b->bid);
    fprintf(F, "LT   #%i\n", b->lifetime);
    fprintf(F, "BYTES %li\n", b->bytes);
    fprintf(F, "TITEL ");
    switch (b->conttype)
      {
        case '6': fprintf(F, "(7-) "); break;
        case '7': fprintf(F, "(7+) "); break;
        case '8': fprintf(F, "(7inf) "); break;
        case '9': fprintf(F, "(7txt) "); break;
        case 'H': fprintf(F, "(html) "); break;
        default:  if (b->binstart) fprintf(F, "(BIN) ");
      }
    fprintf(F, "%s\n", b->betreff);
    fprintf(F, "nnnn\nimpdel\n");
    s_fclose(F);
    mkboard("/", "P", 0);
    mbimport(tempfilename);
    xunlink(tempfilename);
    strcpy(servername, username);
  }
  s_fclose(serverfile);
  sema_unlock(POCSAGSEMA);
  sema_unlock(MSMODSEMA);
}

/*---------------------------------------------------------------------------*/
#endif
