/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ----------------------------------------------
  Einbindungsmoeglichkeit fuer externe Programme
  ----------------------------------------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19970915 DH3MB  Continued working on runutils.cpp
//19980125 OE3DZW option -p also for sysop (not only for pw-holders..)
//19980307 OE3DZW removed .run from temporary file names
//19980318 OE3DZW fixed segfault due to invalid string-pointer
//19980428 DH3MB  fixed erase of bin_output
//19980505 DH3MB  fixed user_imp (was not erased, was not correct)
//19980707 DH3MB  made runutil-interface modular (#define RUNUTILS)
//19980724 DH3MB  added DPBOXUSERIMPOUT, activated option -D for Win32
//19980831 OE3DZW fixed -D for Win32, could not even be compiled
//19990206 OE3DZW added fspath
//20000103 OE3DZW excluded tell (to be finished)
//20000420 DF3VI  included TELL again, changes for DPbox-like PW
//20000627 DK2UI  added option interactive (A)

#include "baycom.h"

#ifdef RUNUTILS

/* sample runutil.bcm
;Befehl Programm     Parameter Kommentar
fwda*dd "fwdadd"     -sq       Hilfsmittel zum Eintragen von Forwardadressen
o*shell "command /c" -sqi      Shell starten
*/

typedef struct mbrun_t
{
  char command[RUN_CMDLEN+1];
  char executable[RUN_EXELEN+1];
  unsigned int abbreviation;
  bitfeld opt;
  // o_a: interactive
  // o_s: Only executable by sysop
  // o_f: Only executable in FileSurf-mode
  // o_q: Do NOT create "rundat.bcm"
  // o_c: Do NOT pass the given parameters to the external program
  // o_i: Input from user is NOT passed to the external program
  //      (Linux/Win32 only)
  // o_i: Interactive programm ist not available for Tell
  // o_p: Only executable by users, who have set a password
  // o_d: Enable DPBOX-compatibility, i.e. set some variables in the
  //      environment-table (Linux/Win32 only)
  // o_t: Even pass |, > and < to the external programm (Linux only)
  char comment[RUN_COMLEN+1];
} mbrun_t;

mbrun_t *runs = NULL;
unsigned int run_num;

/*---------------------------------------------------------------------------*/

void parse_runfile (char *s, mbrun_t *r)
{
  unsigned int i = 0;

  r->abbreviation = 0;
  while (*s && *s != ' ' &&  i < RUN_CMDLEN)
  {
    if (*s == '*')
      r->abbreviation = i;
    else
      r->command[i++] = *s;
    s++;
  }
  r->command[i++] = 0;
  strupr(r->command);
  if (! r->abbreviation) r->abbreviation = strlen(r->command);
  s = nexttoken(s, r->executable, RUN_EXELEN);
  scanoptions(s);
  r->opt = b->optplus;
  while (*s && *s == ' ' && i < RUN_CMDLEN) s++;
  strncpy(r->comment, s, RUN_COMLEN);
  r->comment[RUN_COMLEN] = 0;
}

/*---------------------------------------------------------------------------*/

void read_runfile (void)
{
  FILE *f = s_fopen(RUNNAME, "srt");
  char s[255];

  run_num = 0;
  if (f)
  {
    if (runs)
    {
      t_free(runs);
      runs = NULL;
    }
    while (fgets(s, sizeof(s) - 1, f))
      if (*s != ';') run_num++;
    runs = (struct mbrun_t *) t_malloc(run_num * sizeof(mbrun_t), "*run");
    rewind(f);
    run_num = 0;
    while (fgets(s, sizeof(s) - 1, f))
    {
      if (*s == ';') continue;
      cut_blank(s);
      parse_runfile(s, runs + run_num);
      run_num++;
    }
    s_fclose(f);
    trace(report, "runutils", "%d runutils activated", run_num);
    if (*b->logincall && run_num) putf("%d runutils.\n", run_num);
  }
}

/*---------------------------------------------------------------------------*/

void list_runutils (char *befbuf)
{
  unsigned int i;
  mbrun_t *r;
  char cmdtemp[RUN_CMDLEN+3];

  if (! run_num)
  {
    putf(ms(m_norunutils));
    return;
  }
  scanoptions(befbuf);
  r = runs;
  putf("%-14s", "Command");
  if (b->optplus & o_l) putf(" %-19s ASFQCIPDT", "External program");
  putv(LF);
  for (i = 0; i < run_num; i++)
  {
   if (! (r->opt & o_s) || (b->optplus & o_s))
     if (! (b->optplus & o_i) || (r->opt & o_i))
     {
//      if (b->optplus & o_l) putv(LF);
      strcpy(cmdtemp, r->command);
      if (r->abbreviation != strlen(r->command))
      {
        cmdtemp[r->abbreviation] = '(';
        strcpy(&cmdtemp[r->abbreviation + 1], &r->command[r->abbreviation]);
        strcat(cmdtemp, ")");
      }
      putf("%-14s ", cmdtemp);
      if (b->optplus & o_l)
      {
        putf("%-19s ", r->executable);
        if (r->opt & o_a) putv('1'); else putv('0');
        if (r->opt & o_s) putv('1'); else putv('0');
        if (r->opt & o_f) putv('1'); else putv('0');
        if (r->opt & o_q) putv('1'); else putv('0');
        if (r->opt & o_c) putv('1'); else putv('0');
        if (r->opt & o_i) putv('1'); else putv('0');
        if (r->opt & o_p) putv('1'); else putv('0');
        if (r->opt & o_d) putv('1'); else putv('0');
        if (r->opt & o_t) putv('1'); else putv('0');
      }
      putf("  %s\n", r->comment);
    }
    r++;
  }
  putf(ms(m_runutilsactive), run_num);
}

/*---------------------------------------------------------------------------*/

int start_runutil (char *befbuf)
{
#ifdef __FLAT__
  char command[2 * RUN_PARAMLEN];
  char temp[RUN_PARAMLEN];
#else
  char command[RUN_PARAMLEN];
#endif
  char yapptmp[20];
  char bintmp[20];
  char abintmp[20];
  char txttmp[20];
  char imptmp[20];
  char uimptmp[20];
  char *origbef;
  unsigned int i;
  mbrun_t *r;
  FILE *f;

  if (! run_num) return 0;
  r = runs;
  origbef = befbuf;
  befbuf = nexttoken(befbuf, command, RUN_CMDLEN);
  if (strlen (befbuf) >= RUN_PARAMLEN) befbuf[RUN_PARAMLEN-1] = 0;
  if (*command)
  {
    strupr(command);
    for (i = 0; i < run_num; i++)
    {
      if (! strncmp(r->command, command, strlen(command))
          && strlen(command) >= r->abbreviation)
      {
        strcpy(command, r->executable);
        break;
      }
      if (i == (run_num - 1)) *command = 0;
      r++;
    }
    if ((r->opt & o_s) && ! b->sysop) return test_job(j_tell);
    //Unausfuehrbares Run-Util: So tun als ob :-)
    if (! (r->opt & o_i) && test_job (j_tell)) return 1;
    if ((r->opt & o_p) && (! b->pwok) && (! b->sysop)) return 0;
#ifdef FILESURF
    if ((r->opt & o_f) && (! b->fsmode)) return 0;
    if (! (r->opt & o_f) && (b->fsmode)) return 0;
#endif
    if (*command)
      {
        if (! (r->opt & o_q) || (r->opt & o_d))
        {
          sprintf(txttmp, TEMPPATH "/%s", time2filename(0));
          strlwr(txttmp);
          sprintf(bintmp, TEMPPATH "/%s", time2filename(0));
          strlwr(bintmp);
          sprintf(abintmp, TEMPPATH "/%s", time2filename(0));
          strlwr(abintmp);
          sprintf(yapptmp, TEMPPATH "/%s", time2filename(0));
          strlwr(yapptmp);
          sprintf(imptmp, TEMPPATH "/%s", time2filename(0));
          strlwr(imptmp);
          sprintf(uimptmp, TEMPPATH "/%s", time2filename(0));
          strlwr(uimptmp);
        }
        if (! (r->opt & o_q))
        {
          if ((f = s_fopen(RUNDATNAME, "swt")) != NULL)
          {
            s_fsetopt(f, 1);
            fprintf(f, "%s %s %s %s %d %d %d %d %s\n", u->call, u->mybbs,
                    u->name, u->sprache, b->pwok, b->sysop, u->status,
                    u->helplevel, u->uplink);
            fprintf(f, "call       %s\n", u->call);
            fprintf(f, "mybbs      %s\n", u->mybbs);
            fprintf(f, "name       %s\n", u->name);
            fprintf(f, "language   %s\n", u->sprache);
            fprintf(f, "password   %d\n", b->pwok);
            fprintf(f, "sysop      %d\n", b->sysop);
            fprintf(f, "status     %d\n", u->status);
            fprintf(f, "helplevel  %d\n", u->helplevel);
            fprintf(f, "digi       %s\n", u->uplink);
            fprintf(f, "txt_output %s\n", txttmp);
            fprintf(f, "bin_output %s\n", bintmp);
            fprintf(f, "imp_output %s\n", imptmp);
            fprintf(f, "user_imp   %s\n", uimptmp);
            fprintf(f, "cmdline    %s\n", origbef);
            fprintf(f, "login_time %ld\n", b->logintime);
            fprintf(f, "board      %s\n", b->prompt);
            fprintf(f, "abinoutput %s\n", abintmp);
            fprintf(f, "yappoutput %s\n", yapptmp);
#ifdef FILESURF // DH3MB
            if (b->fspath)
              fprintf(f, "fspath     %s\n", b->fspath);
#endif
            s_fclose(f);
          }
        }
        if (! (r->opt & o_c))
        {
          strcat(command, " ");
#ifdef __LINUX__
          // Running LinuX, even the characters |, > and < can be passed
          // to the external programm (if the parameter -T is given)
          if (! (r->opt & o_t))
#endif
          {
            subst1(befbuf, '|', 0);
            subst1(befbuf, '>', 0);
            subst1(befbuf, '<', 0);
          }
#ifdef __LINUX__
          // Running LinuX, propose a backslash to every character of
          // the parameters (not to the spaces, of course)
          while (*befbuf)
          {
            if (*befbuf == ' ')
              sprintf(command + strlen(command), "%c", *befbuf);
            else
              sprintf(command + strlen(command), "\\%c", *befbuf);
            befbuf++;
          }
#else
          strcat(command, befbuf);
#endif
        }
#ifdef __FLAT__
        if (r->opt & o_d)
        {
          setenv("TERM", "dumb", 1);
          setenv("LOGNAME", u->call, 1);
          setenv("CALLSIGN", u->call, 1);
          setenv("DPBOXUSERCALL", u->call, 1);
          setenv("DPBOXUSERMYBBS", u->mybbs, 1);
          setenv("DPBOXUSERNAME", u->name, 1);
          setenv("DPBOXUSERLANGUAGE", u->sprache, 1);
          setenv("DPBOXBOARD", b->boardname, 1);
          setenv("DPBOXLASTDATE", datestr(u->lastboxlogin, 11), 1);
          sprintf(temp, "%ld", u->lastboxlogin);
          setenv("DPBOXIXLASTDATE", temp, 1);
          setenv("DPBOXSENDERASEOK", "1", 1);     // dummy
          setenv("DPBOXHIDDEN", "0", 1);          // dummy
          sprintf(temp, "%d", b->pwok);
          setenv("DPBOXPWOK", temp, 1);
          sprintf(temp, "%d", b->sysop);
          setenv("DPBOXBOARDSYSOP", temp, 1);     // dummy
          sprintf(temp, "%d", b->sysop);
          setenv("DPBOXSYSOP", temp, 1);
          sprintf(temp, "%d", b->sysop * 127);
          setenv("DPBOXUSERLEVEL", temp, 1);      // dummy
          setenv("DPBOXBOXCALL", m.boxname, 1);
#ifdef FILESURF  // DF3VI
          sprintf(temp, "%d", b->fsmode);
          setenv("DPBOXSERVERMODE", temp, 1);
          setenv("DPBOXSERVERROOT", "/", 1);
          setenv("DPBOXSERVERPATH", b->fspath, 1);
#endif
          strcpy(temp, u->call);
          strcat(temp, " ");
          strcat(temp, b->uplink);
          setenv("DPBOXCONNECTPATH", temp, 1);
          setenv("DPBOXUNPROTOOK", "0", 1);       // dummy
          if (strncmp(u->notvisible + 1, "-R ", 3)==0)
          {
            setenv("DPBOXNOCHECK", "", 1);
            setenv("DPBOXWANTCHECK", u->notvisible+4, 1);
          }
          else
          {
            setenv("DPBOXNOCHECK", u->notvisible + 1, 1);
            setenv("DPBOXWANTCHECK", "", 1);
          }
          setenv("DPBOXLASTCMD", origbef, 1);
          setenv("DPBOXDEBUGLEVEL", "0", 1);      // dummy
          setenv("DPBOXUSERTIMEOUT", "300", 1);   // dummy
          setenv("DPBOXVERSION", "5.07.00", 1);   // dummy
          sprintf(temp, "%d", gettaskid());
          setenv("DPBOXUSERNUMBER", temp, 1);
          sprintf(temp, "%d", listusers(0));
          setenv("DPBOXUSERCOUNT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, txttmp);
          setenv("DPBOXTEXTOUT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, bintmp);
          setenv("DPBOXTRANSOUT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, abintmp);
          setenv("DPBOXBINOUT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, yapptmp);
          setenv("DPBOXYAPPOUT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, imptmp);
          setenv("DPBOXIMPOUT", temp, 1);
          strcpy(temp, globalpath);
          strcat(temp, uimptmp);
          setenv("DPBOXUSERIMPOUT", temp, 1);
#ifdef FILESURF // DH3MB
          if (b->fspath)
            setenv("DPBOXFSPATH", b->fspath, 1);
#endif
        }
#endif  //__FLAT__
        putv(LF); //add linebreak before start of runutil (OE3DZW)
        if (r->opt & o_a)
          oshell(command, sh_interactive);
        else
          if (r->opt & o_i)
            oshell(command, sh_noinput);
          else
            oshell(command, sh_ifmultitask);
        if (! (r->opt & o_q) || (r->opt & o_d))
        {
          if (file_isreg(txttmp))
          {
            fileio_text fio;
            fio.usefile(txttmp);
            fio.doerase();
            fio.tx();
          }
          if (file_isreg(bintmp))
          {
            int a;
            f = s_fopen(bintmp, "lrb");
            s_fsetopt(f, 2);
            while ((a = fgetc(f)) != EOF) bputv(a);
            s_fclose(f);
          }
          if (file_isreg(abintmp))
          {
            fileio_abin fio;
            fio.usefile(abintmp);
            fio.doerase();
            fio.tx();
          }
#ifdef FEATURE_YAPP
          if (file_isreg(yapptmp))
          {
            fileio_yapp fio;
            fio.usefile(yapptmp);
            fio.doerase();
            fio.tx();
          }
#endif
          if ((t->input != io_file || t->output == io_dummy)
              && t->output != io_file
              && file_isreg(uimptmp)) // Only if no file-i/o
          {
            b->inputfile = s_fopen(uimptmp, "lrt");
            s_fsetopt(b->inputfile, 2);
            b->oldinput = t->input;
            t->input = io_file;
          }
          if (file_isreg(imptmp))
            fork(P_BACK|P_MAIL, 0, mbimport, imptmp);
        }
        return 1;
      }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
#endif
