/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -----------------
  Makro-Interpreter
  -----------------


  Copyright (c)       Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19880118 OE3DZW sec in timestr
//19980709 DH3MB  changed BCM_MACRO to MACRO (definable within baycom.h)
//20121226 DG4MFN changes to make the Linux GCC 4.x.x Compilers happy :-)

#include "baycom.h"

/*---------------------------------------------------------------------------*/

#ifdef MACRO

#define TOK_MIN 1
#define SYMBOL_LEN 20
#define MAX_CTL 10
#define LINE_LEN 80
#define NUM_LEN 12
#define script ((mk_script_t *)(b->script_p))

/*---------------------------------------------------------------------------*/

/*
example "macro.bcm":

#<event> [<call/command/wildcard>]: <macro>
login *: loginout
login dg3rbu: rbu
accept db0aab-3: aab3
cmd a*ktuell: akt
cmd pu*rge: disabled
scmd fwdadd: fwdadd
beforecmd v*ersion: b
beforecmd *:
aftercmd *:
newmail *:
unknown:
*/

/*---------------------------------------------------------------------------*/

/*
example "marco/aktuell":

begin
  ftime=filetime("msg/aktuell.dl");
  put("Aktuell vom " & datestr(ftime,12) & "\n");
  cmd("rt msg/aktuell.dl");
  cmd("o ls -l");
end;
*/

/*---------------------------------------------------------------------------*/

static char *mk_job_tab[]=
{ "LOGIN",
  "ACCEPT",
  "CMD",
  "SCMD",
  "BEFORECMD",
  "AFTERCMD",
  "MAILFROM",
  "MAILTO",
  "UNKNOWN",
  NULL
};

/*---------------------------------------------------------------------------*/

typedef struct mk_scripts_t
{ mk_job_t job;
  char para[21];
  unsigned abbrev;
  char scriptname[21];
} mk_scripts_t;

/*---------------------------------------------------------------------------*/

static mk_scripts_t *mk_jobs;

/*---------------------------------------------------------------------------*/

typedef enum mk_token_t
{ TOK_NOTHING,          // Example
  TOK_NE=TOK_MIN,       // Operator: !=
  TOK_CONCAT,           // Operator: &
  TOK_BRACK_L,          // Operator: )
  TOK_BRACK_R,          // Operator: (
  TOK_MUL,              // Operator: *
  TOK_ADD,              // Operator: +
  TOK_COMMA,            // Operator: ,
  TOK_SUB,              // Operator: -
  TOK_DOT,              // Operator: .
  TOK_DIV,              // Operator: /
  TOK_SEMI,             // Operator: ;
  TOK_LESS,             // Operator: <
  TOK_LE,               // Operator: <=
  TOK_ASSIGN,           // Operator: =
  TOK_EQU,              // Operator: ==
  TOK_GREA,             // Operator: >
  TOK_GE,               // Operator: >=
  TOK_SUBSCR_L,         // Operator: [
  TOK_SUBSCR_R,         // Operator: ]
  TOK_ABORT,            // abort("reason");
  TOK_AND,              // Operator: logical AND
  TOK_BEGIN,            // begin                   * Block, no parameter
  TOK_CALL,             // call("subprogram",var_a,...);
  TOK_CHR,              // x=chr(65);         * one ASCII-character
  TOK_CMD,              // x=cmd("v"); or cmd("e dl8mbt 1-");
  TOK_CONNECT,          // handle=connect("dl8mbt","db0aab");
  TOK_DATESTR,          // x=datestr(gettime,3);
  TOK_DISCONNECT,       // disconnect(handle);
  TOK_ELSE,             // if(condition) .. else .. endif;
  TOK_END,              // begin ... end;
  TOK_ENDIF,            // if(condition) .. else .. endif;
  TOK_ENDLOOP,          // loop .. exit_if(condition); ... endloop:
  TOK_EXIT,             // loop .. exit; ... endloop:
  TOK_EXIT_IF,          // loop .. exit_if(condition); ... endloop:
  TOK_FILELINE,         // line=fileline("filename",5);
  TOK_FILEREAD,         // string=fileread("filename");
  TOK_FILESIZE,         // size=filesize("filename");
  TOK_FILETIME,         // t=filetime("filename");
  TOK_FILEWRITE,        // filewrite("filename","string\n");
  TOK_GETFNAME,         // s=getfname("dl8mbt",25);
  TOK_GETLINE,          // s=getline;
  TOK_GETLIST,          // s=getlist("dl8mbt");
  TOK_GETMACRO,         // s=getmacro("%o");
  TOK_GETTIME,          // t=gettime; put("time=" & time(t));
  TOK_GETUSER,          // s=getuser("dl8mbt","forward");
  TOK_GETVALID,         // if(getvalid) s=getline; endif;
  TOK_GETVAR,           // call=getvar("logincall");
  TOK_IF,               // if(condition) .. else .. endif;
  TOK_LEN,              // i=len(string);
  TOK_LINES,            // a=lines(x);
  TOK_LOOP,             // loop .. exit_if(condition); ... endloop:
  TOK_MOD,              // Operator: modulo
  TOK_OR,               // Operator: logical OR
  TOK_OSHELL,           // x=oshell("ls -l"); or oshell("ls -l");
  TOK_PUT,              // put("hallo\n");
  TOK_SLEEP,            // sleep(7000);     * milliseconds
  TOK_STRLINE,          // s=strline(x,5);
  TOK_STRPOS,           // i=strpos(s,"test");
  TOK_TIMESTR,          // put("Zeit: " & datestr(gettime,4) & "\n");
  TOK_TIMEOUT,          // loop .. exit_if(timeout) .. endloop;
  TOK_TOKEN,            // s1=token(command,1);
  TOK_TRACE,            // trace(report,"test");
  TOK_VAL,              // x=val("A");
  TOK_XOR,              // Operator: logical EXOR
  TOK_EOF               // file end.
} mk_token_t;

/*---------------------------------------------------------------------------*/

typedef struct mk_keyword_t
{ char *word;
  mk_token_t token;
} mk_keyword_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ MK_NUM_CONST, MK_STR_CONST, MK_SYMBOL, MK_OPERATOR } mk_symbol_type_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ MK_NO_CTL, MK_LOOP, MK_REMOTE, MK_IF } ctl_struc_t;

/*---------------------------------------------------------------------------*/

typedef enum
{ MK_OK,
  MK_NEW,
  MK_BEGIN,
  MK_BLOCK,
  MK_ERROR,
  MK_RETURN,
  MK_END,
  MK_IDLE
} parse_state_t;

/*---------------------------------------------------------------------------*/

typedef struct mk_mem_t
{ struct mk_mem_t *prev;
  struct mk_mem_t *next;
  char data[1];
} mk_mem_t;

/*---------------------------------------------------------------------------*/

typedef struct var_t
{ mk_mem_t      *mem_block;
  struct var_t  *next;
  char          name[SYMBOL_LEN];
  unsigned      len;
  char          *s;
  struct var_t  *inherited;
} var_t;

/*---------------------------------------------------------------------------*/

typedef struct  mk_script_t
{ char          *buffer;
  char          *pos;
  unsigned      line;
  parse_state_t state;
  int           syntaxcheck;
  char          *linebegin;
  char          *error;
  int           ctl_count;
  char          *ctl_pos[MAX_CTL];
  unsigned      ctl_line[MAX_CTL];
  mk_token_t    ctl_type[MAX_CTL];
  time_t        ctl_time[MAX_CTL];
  char          word[LINE_LEN];
  char          name[SYMBOL_LEN];
  int           remote;
  mk_token_t    token;
  mk_symbol_type_t type;
  var_t         *var_root;
  struct mk_script_t *return_ptr;
  int           nesting_level;
  char          ermessage[80];
  int           erline;
  jmp_buf       jmpbuf;
} mk_script_t;

/*---------------------------------------------------------------------------*/

// Prototypes
//
static int mk_parse_script(void);
static int mk_expect(mk_token_t token);
static int mk_parse_error(void);
static int mk_parse_body(void);
static int mk_prepare_script(void);
static int mk_getbegin(void);
static int mk_next_word(void);
static int mk_get_token(void);
static int mk_op_concat(var_t *a,var_t *b,var_t *result);
static int mk_get_mem(var_t *var);
static int mk_free_mem(var_t *var);
static int mk_put_var(char *name, var_t *var, int inherited, mk_script_t *scr);
static int mk_modify_var(char *name, char *value);
static var_t *mk_get_var( char *name);
static int mk_free_all( void);
static int mk_and_or(var_t *result);
static void mk_error(char *msg);
static int mk_abort;

//static void mk_dump_vars( void);
//static mk_script_t *script=NULL;

/*---------------------------------------------------------------------------*/

#define issymbol(c)   (isalnum(c) || (c=='_'))
#define issymbegin(c) (isalpha(c) || (c=='_'))

/*---------------------------------------------------------------------------*/

static mk_mem_t *mem_root = NULL;

static mk_keyword_t mk_wordtab[] =
{
  { "!=",       TOK_NE          },
  { "&",        TOK_CONCAT      },
  { "(",        TOK_BRACK_L     },
  { ")",        TOK_BRACK_R     },
  { "*",        TOK_MUL         },
  { "+",        TOK_ADD         },
  { ",",        TOK_COMMA       },
  { "-",        TOK_SUB         },
  { ".",        TOK_DOT         },
  { "/",        TOK_DIV         },
  { ";",        TOK_SEMI        },
  { "<",        TOK_LESS        },
  { "<=",       TOK_LE          },
  { "=",        TOK_ASSIGN      },
  { "==",       TOK_EQU         },
  { ">",        TOK_GREA        },
  { ">=",       TOK_GE          },
  { "[",        TOK_SUBSCR_L    },
  { "]",        TOK_SUBSCR_R    },
  { "abort",    TOK_ABORT       },
  { "and",      TOK_AND         },
  { "begin",    TOK_BEGIN       },
  { "call",     TOK_CALL        },
  { "chr",      TOK_CHR         },
  { "cmd",      TOK_CMD         },
  { "connect",  TOK_CONNECT     },
  { "datestr",  TOK_DATESTR     },
  { "disconnect", TOK_DISCONNECT},
  { "else",     TOK_ELSE        },
  { "end",      TOK_END         },
  { "endif",    TOK_ENDIF       },
  { "endloop",  TOK_ENDLOOP     },
  { "exit",     TOK_EXIT        },
  { "exit_if",  TOK_EXIT_IF     },
  { "fileline", TOK_FILELINE    },
  { "fileread", TOK_FILEREAD    },
  { "filesize", TOK_FILESIZE    },
  { "filetime", TOK_FILETIME    },
  { "filewrite",TOK_FILEWRITE   },
  { "getfname", TOK_GETFNAME    },
  { "getline",  TOK_GETLINE     },
  { "getlist",  TOK_GETLIST     },
  { "getmacro", TOK_GETMACRO    },
  { "gettime",  TOK_GETTIME     },
  { "getuser",  TOK_GETUSER     },
  { "getvalid", TOK_GETVALID    },
  { "getvar",   TOK_GETVAR      },
  { "if",       TOK_IF          },
  { "len",      TOK_LEN         },
  { "lines",    TOK_LINES       },
  { "loop",     TOK_LOOP        },
  { "mod",      TOK_MOD         },
  { "or",       TOK_OR          },
  { "oshell",   TOK_OSHELL      },
  { "put",      TOK_PUT         },
  { "sleep",    TOK_SLEEP       },
  { "strline",  TOK_STRLINE     },
  { "strpos",   TOK_STRPOS      },
  { "timestr",  TOK_TIMESTR     },
  { "timeout",  TOK_TIMEOUT     },
  { "token",    TOK_TOKEN       },
  { "trace",    TOK_TRACE       },
  { "val",      TOK_VAL         },
  { "xor",      TOK_XOR         },
  { NULL,       TOK_EOF         },
};

/*---------------------------------------------------------------------------*/

int mk_perform (mk_job_t job, char *parameter)
{
  lastfunc("mk_perform");
  unsigned int i = 0;
  int ret = 0;
  char word[20];

  if (! mk_jobs) return 0;
  parameter += blkill(parameter);
  while (parameter[i] > ' ' && i < 19)
  {
    word[i] = parameter[i];
    i++;
  }
  word[i] = 0;
  for (i = 0; mk_jobs[i].job != MK_NOJOB; i++)
  {
    if (job == mk_jobs[i].job)
    { //putf("job=%d, para=%s, abbr=%d, script=%s\n", mk_jobs[i].job,
      //     mk_jobs[i].para, mk_jobs[i].abbrev, mk_jobs[i].scriptname);
      if (mk_jobs[i].para[0])
      { // Konstrukte wie "accept dl*"
        if (mk_jobs[i].abbrev == strlen(mk_jobs[i].para))
        {
          if (stristr(word, mk_jobs[i].para) != word) continue;
        }
        // konstrukte wie "cmd ne*wcommand"
        else if (mk_jobs[i].abbrev)
        {
          if (stristr(mk_jobs[i].para, word) != mk_jobs[i].para ||
              strlen(word) < mk_jobs[i].abbrev) continue;
        }
        // exakte Uebereinstimmung
        else if (stricmp(mk_jobs[i].para, word)) continue;
      }
      ret += mk_start(mk_jobs[i].scriptname);
    }
  }
  return ret;
}

/*---------------------------------------------------------------------------*/

int strsplit (char *s, char *sc[], unsigned int max, char *sep)
{
  unsigned int i;
  if (! sc) return 0;
  if (! s || ! *s)
  {
    sc[0] = NULL;
    return 0;
  }
  sc[0] = s;
  for (i = 1; i < max; i++)
  {
    while (strpbrk(sc[i - 1], sep) == sc[i - 1]) sc[i - 1]++;
    sc[i] = strpbrk(sc[i - 1], sep);
    if (! sc[i]) return i;
    sc[i][0] = 0;
    sc[i]++;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void mk_read_jobs (void)
{
  char s[101];
  int number = 0;
  char *sc[10];
  FILE *f = s_fopen(EVENTNAME, "srt");

  if (mk_jobs)
  {
    t_free(mk_jobs);
    mk_jobs = NULL;
  }
  if (f)
  {
    while (fgets(s, sizeof(s) - 1, f)) number++;
    rewind(f);
    mk_jobs = (mk_scripts_t *) t_malloc(sizeof(mk_scripts_t) *(number + 1),
                                        "*evt");
    number = 0;
    m.macro_mycalls = 0;
    while (fgets(s, sizeof(s) - 1, f))
    {
      s[strlen(s) - 1] = 0;
      int l = strsplit(s, sc, 10, " :");
      if (l < 2 || l > 3 || *s == '#') continue;
      mk_jobs[number].job = (mk_job_t) readcmd(mk_job_tab, &(sc[0]), 0);
      mk_jobs[number].para[0] = 0;
      mk_jobs[number].abbrev = 0;
      if (l == 2) strcpy(mk_jobs[number].scriptname, sc[1]);
      else
      {
        char *wc = strchr(sc[1], '*');
        if (wc)
        {
          *wc = 0;
          mk_jobs[number].abbrev = strlen(sc[1]);
          strcpy(mk_jobs[number].para, sc[1]);
          strcat(mk_jobs[number].para, wc + 1);
        }
        else
          strcpy(mk_jobs[number].para, sc[1]);
        strcpy(mk_jobs[number].scriptname, sc[2]);
      }
      if (mk_jobs[number].job == MK_ACCEPT)
      {
        if (m.macro_mycalls < MK_MYCALLS)
          strcpy(m.macro_mycall[m.macro_mycalls++], mk_jobs[number].para);
        else
          trace(serious, "mk_read_jobs", "to many accepts");
      }
      number++;
    }
    mk_jobs[number].job = MK_NOJOB;
    trace(report, "mk_read_jobs", "%d macros activated", number);
    if (*b->logincall && number) putf("%d macros.\n", number);
    s_fclose(f);
    align_mycalls();
  }
}

/*---------------------------------------------------------------------------*/

int mk_getuser (char *bef, char *result, char *call)
{
  static char *beftab[] =
    { "COMMAND", "SPEECH", "FORWARD", "HELPLEVEL", "LINES",
      "NAME", "PROMPT", "REJECT", "TTYPW", "PW", "LF",
      "IDIR", "UDIR", "ILIST", "ULIST", "IREAD", "UREAD", "CHECK", "PS",
      "STATUS", "QUOTA", "NOPURGE", "PWLINE", "READLOCK", "ECHO", "MYBBS",
      "FDELAY", "FHOLD", "RLIMIT",
      NULL
    };
  enum befnum
    { unsinn,
      command, speech, forward, helplevel, lines,
      name, prompt, reject, ttypw, pw, lf,
      idir, udir, ilist, ulist, iread, uread, check, ps,
      status, quota, nopurge, pwline, readlock, echo, mybbs_,
      fdelay, fhold, rlimit
    };
  int optgroup = 0;
  if (! loaduser(call, u, 1)) return 0;
  switch ((befnum) readcmd(beftab, &bef, 0))
  {
    case unsinn:   return 0;
    case forward:
    case mybbs_:   sprintf(result, "%s %lu", u->mybbs, u->mybbstime);  break;
    case command:  strcpy(result, u->firstcmd);                        break;
    case prompt:   strcpy(result, u->prompt);                          break;
    case pw:       sprintf(result, "%u", strlen(u->password));         break;
    case pwline:   sprintf(result, "%u", u->pwline);                   break;
    case echo:     sprintf(result, "%u", u->echo);                     break;
    case fdelay:   sprintf(result, "%u", u->fdelay);                   break;
    case fhold:    sprintf(result, "%u", u->fhold);                    break;
    case rlimit:   sprintf(result, "%u", u->rlimit);                   break;
    case readlock: sprintf(result, "%u", u->readlock);                 break;
    case helplevel:sprintf(result, "%u", u->helplevel);                break;
    case lf:       sprintf(result, "%u", u->lf);                       break;
    case lines:    sprintf(result, "%u", u->zeilen);                   break;
    case name:     strcpy(result, u->name);                            break;
    case reject:   strcpy(result, u->notvisible);                      break;
    case speech:   strcpy(result, u->sprache);                         break;
    case ps:       optgroup++;
    case check:    optgroup++;
    case uread:    optgroup++;
    case iread:    optgroup++;
    case ulist:    optgroup++;
    case ilist:    optgroup++;
    case udir:     optgroup++;
    case idir:     strcpy(result, optstr(optgroup));                   break;
    case status:   sprintf(result, "%u", u->status);                   break;
    case nopurge:  sprintf(result, "%u", u->nopurge);                  break;
    case quota:    sprintf(result, "%u", (unsigned) (u->daybytes >> 10)); break;
    case ttypw:    sprintf(result, "%u", strlen(u->ttypw));            break;
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

int mk_getvar (char *bef, char *result)
{
  static char *beftab[] =
    { "LOGINCALL", "UPLINK", "TXBYTES", "RXBYTES", "SYSOP", "LOGINTIME", "BOARD",
      "DEST", "SRC", "FROMBOX", "AT", "BID", "BOXLIST", "USERMAIL", "LIFETIME",
      "LINES", "BYTES", "SUBJECT", "REPLYBOARD", "REPLYNUM",
      "TASKID", "TASKNAME", "CREATED", "LASTINPUT", "LASTCMD", NULL
    };
  enum befnum
    { unsinn,
      logincall, uplink, txbytes, rxbytes, sysop, logintime, board,
      dest, src, frombox, at, bid, boxlist, usermail, lifetime,
      lines, bytes, subject, replyboard, replynum,
      taskid, taskname, created, lastinput, lastcmd_
    };
  switch ((befnum) readcmd(beftab, &bef, 0))
  {
    case unsinn:                                               return 0;
    case logincall:strcpy(result, b->logincall);               break;
    case uplink:   strcpy(result, b->uplink);                  break;
    case board:    strcpy(result, b->prompt);                  break;
    case dest:     strcpy(result, b->ziel);                    break;
    case src:      strcpy(result, b->herkunft);                break;
    case at:       strcpy(result, b->at);                      break;
    case bid:      strcpy(result, b->bid);                     break;
    case boxlist:  strcpy(result, b->destboxlist);             break;
    case subject:  strcpy(result, b->betreff);                 break;
    case replyboard:strcpy(result, b->replyboard);             break;
    case taskname: strcpy(result, t->name);                    break;
    case lastcmd_: strcpy(result, t->lastcmd);                 break;
    case txbytes:  sprintf(result, "%lu", b->txbytes);         break;
    case rxbytes:  sprintf(result, "%lu", b->rxbytes);         break;
    case sysop:    sprintf(result, "%u", b->sysop);            break;
    case taskid:   sprintf(result, "%u", b->taskid);           break;
    case replynum: sprintf(result, "%u", b->replynum);         break;
    case logintime:sprintf(result, "%lu", b->logintime);       break;
    case created:  sprintf(result, "%lu", b->created);         break;
    case lastinput:sprintf(result, "%lu", b->last_input);      break;
    default: break;
  }
  return 1;
}

/*---------------------------------------------------------------------------*/

static int mk_get_mem (var_t *var)
{
  mk_mem_t *mem = (mk_mem_t *) t_malloc(sizeof(mk_mem_t) + var->len, "mkva");
  mem->next = mem_root;
  mem->prev = NULL;
  if (mem->next)
    mem->next->prev = mem;
  mem_root = mem;
  var->s = mem->data;
  var->mem_block = mem;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_free_mem (var_t *var)
{
  if (! var->s || ! var->mem_block)
    trace(fatal, "mk_free_mem", "no block allocated");
  mk_mem_t *mem = var->mem_block;
  if (mem->next) mem->next->prev = mem->prev;
  if (mem->prev) mem->prev->next = mem->next;
  if (mem_root == mem) mem_root = mem->next;
  t_free(mem);
  var->len = 0;
  var->s = NULL;
  var->mem_block = NULL;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_put_var (char *name, var_t *var, int inherited, mk_script_t *scr)
{
  var_t *var_temp;
  var_temp = scr->var_root;

  while (var_temp)
  {
    if (! strcmp(var_temp->name, name)) break;
    var_temp = var_temp->next;
  }
  if (! var_temp)
  {
    var_temp = (var_t *) t_malloc(sizeof(var_t), "mkpu");
    memset(var_temp, 0, sizeof(var_t));
    if (inherited == TRUE) var_temp->inherited = var;
    var_temp->next = scr->var_root;
    scr->var_root = var_temp;
    strcpy(var_temp->name, name);
  }
  if (var_temp->s) mk_free_mem(var_temp);
  var_temp->len = var->len;
  var_temp->s = var->s;
  var_temp->mem_block = var->mem_block;
  var->s = NULL;
  var->mem_block = NULL;
  var->len = 0;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_modify_var (char *name, char *value)
{
  var_t var_temp;
  var_temp.len = strlen(value);
  mk_get_mem(&var_temp);
  strcpy(var_temp.s, value);
  return mk_put_var(name, &var_temp, 0, script);
}

/*---------------------------------------------------------------------------*/

static var_t *mk_get_var (char *name)
{
  var_t *var_temp = script->var_root;
  while (var_temp)
  {
    if (! strcmp(var_temp->name, name)) return var_temp;
    var_temp = var_temp->next;
  }
  mk_error("undefined symbol");
  return NULL;
}

/*---------------------------------------------------------------------------*/

/*
static void mk_dump_vars(void)
{
  var_t *var_temp = script->var_root;
  int len=0;

  while (var_temp)
  {
    putf("%s=\"", var_temp->name);
    for (len=0; len<var_temp->len; len++)
    {
      if (var_temp->s[len]>=' ')
        putchar(var_temp->s[len]);
      else
      {
        putchar('^');
        putchar(var_temp->s[len]+'@');
      }
      if ((len+strlen(var_temp->name))>70)
      {
        putf("...");
        break;
      }
    }
    putf("\";\n");
    var_temp = var_temp->next;
  }
}
*/

/*---------------------------------------------------------------------------*/

static int mk_free_all (void)
{
  if (script->buffer != NULL)
  {
    t_free(script->buffer);
    script->buffer = NULL;
  }
  if (script->var_root)
  {
    int count = 0;
    int inh_count = 0;
    var_t *var = script->var_root;
    do
    {
      var_t *next = var->next;
      if (! var->inherited)
      {
        if (var->s) mk_free_mem(var);
        count++;
      }
      else
      {
        var_t *prev = var->inherited;
        prev->len = var->len;
        prev->s = var->s;
        prev->mem_block = var->mem_block;
        inh_count++;
      }
      t_free(var);
      var = next;
    } while (var != NULL);
    //trace(report, "mk_free_all", "%d inherited vars", inh_count);
    //trace(report, "mk_free_all", "%d var blocks released", count);
  }
  if (! script->return_ptr)
  {
    int count = 0;
    mk_mem_t *mem = mem_root;
    while (mem)
    {
      mk_mem_t *next = mem->next;
      t_free(mem);
      count++;
      mem = next;
    }
    mem_root = NULL;
    //trace(report, "mk_free_all", "%d mem blocks released", count);
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_parse_error (void)
{
  char line[LINE_LEN];

  strcpy(line, "");
  if (script->error)
  {
    sprintf(script->ermessage, "%s, word '%s'", script->error, script->word);
    script->erline = script->line;
    if (*script->linebegin != LF)
    {
      int linelen = 0;
      *line = 0;
      while (*script->linebegin == ' ') script->linebegin++;
      while (*script->linebegin != LF && *script->linebegin)
      {
        int idx = strlen(line);
        line[idx] = *script->linebegin;
        line[idx + 1] = 0;
        script->linebegin++;
        linelen++;
        if (linelen > 70) break;
        if (idx >= (LINE_LEN - 11)) break;
      }
    }
    putf("macro: %s(%u): %s\n", script->name, script->line, script->ermessage);
    if (*line) putf("line: %s\n", line);
    return ERROR;
  }
  else return OK;
}

/*---------------------------------------------------------------------------*/

/*
static int mk_read_hexdump(char *hexdump, char *binary, int *length, int maxlen)
{
  int incount=0;

  while (maxlen && hexdump[0])
  {
    unsigned hex;
    while (hexdump[incount] && hexdump[incount]<=' ') incount++;
    if (sscanf(hexdump+incount, "%2x", &hex) != 1)
      break;
    binary[0] = (char) hex;
    binary++;
    (*length)++;
    maxlen--;
    incount++;
    if (isalnum(hexdump[incount])) incount++;
  }
  return incount;
}
*/

/*---------------------------------------------------------------------------*/

static int mk_lex (void)
{
  mk_next_word();
  return mk_get_token();
}

/*---------------------------------------------------------------------------*/

static int mk_get_token (void)
{
  int indext = 0;
  char firstchar = *script->word;

  while (mk_wordtab[indext].word)
  {
    if (   firstchar == mk_wordtab[indext].word[0]
        && ! strcmp(mk_wordtab[indext].word, script->word))
    {
      script->token = mk_wordtab[indext].token;
      return OK;
    }
    indext++;
  }
  script->token = TOK_NOTHING;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_next_word (void)
{
  enum { NO_COMM, C_COMM, CPP_COMM } comment = NO_COMM;

  while (*script->pos)
  {
    if (*script->pos == LF)
    {
      script->line++;
      if (comment == CPP_COMM) comment = NO_COMM;
      script->linebegin = script->pos + 1;
    }
    if (comment == C_COMM)
    {
      if (*script->pos == '*' && script->pos[1] == '/')
      {
        comment = NO_COMM;
        script->pos += 2;
        continue;
      }
    }
    else if (comment == NO_COMM)
    {
      if (*script->pos == '/')
      {
        if (script->pos[1] == '/')
        {
          comment = CPP_COMM;
          script->pos++;
          continue;
        }
        else if (script->pos[1] == '*')
        {
          comment = C_COMM;
          script->pos++;
          continue;
        }
      }
      if (*script->pos > ' ')
      {
        *script->word = *script->pos;
        if (*script->pos == '"')
        {
          unsigned int i = 0;
          script->pos++;
          while (*script->pos != '"')
          {
            if (*script->pos == LF) mk_error("'\"' expected");
            if (*script->pos == '\\')
            {
              script->pos++;
              switch (*script->pos)
              {
                case 'n': script->word[i++] = LF; break;
                case 'a': script->word[i++] = '\a'; break;
                default:  script->word[i++] = *script->pos;
              }
            }
            else script->word[i++] = *script->pos;
            script->pos++;
            if (i >= LINE_LEN)
            {
              script->word[20] = 0;
              mk_error("constant too long");
            }
          }
          script->word[i] = 0;
          script->pos++;
          script->type = MK_STR_CONST;
          return OK;
        }
        else if (isdigit(*script->pos))
        {
          unsigned int i = 0;
          while (isdigit(*script->pos))
          {
            script->word[i++] = *script->pos;
            script->pos++;
            if (i >= NUM_LEN)
            {
              script->word[NUM_LEN - 1] = 0;
              mk_error("constant range");
            }
          }
          script->word[i] = 0;
          script->type = MK_NUM_CONST;
          return OK;
        }
        else if (issymbegin(*script->pos))
        {
          unsigned int i = 0;
          while (issymbol(*script->pos))
          {
            script->word[i++] = tolower(*script->pos);
            script->pos++;
            if (i >= SYMBOL_LEN)
            {
              script->word[20] = 0;
              mk_error("symbol too long");
            }
          }
          script->word[i] = 0;
          script->type = MK_SYMBOL;
          return OK;
        }
        else
        {
          script->word[0] = *script->pos;
          script->word[1] = 0;
          switch (*script->pos)
          {
            case '=':
            case '!':
            case '<':
            case '>':
              if (script->pos[1] == '=')
              {
                script->word[1] = script->pos[1];
                script->word[2] = 0;
                script->pos++;
              } break;
          }
          script->pos++;
          script->type = MK_OPERATOR;
          return OK;
        }
      }
    }
    script->pos++;
  }
  *script->word = 0;
  script->token = TOK_EOF;
  mk_error("unexpected end of file");
  return ERROR;
}

/*---------------------------------------------------------------------------*/

int mk_expression (var_t *result)
{
  var_t left, right;

  mk_and_or(&left);
  if (script->token == TOK_CONCAT)
  {
    mk_lex();
    mk_expression(&right);
    mk_op_concat(&left, &right, result);
  }
  else *result = left;
  return OK;
}

/*---------------------------------------------------------------------------*/

int mk_get_para (int number, var_t par[])
{
  int indext = 0;
  mk_lex();
  mk_expect(TOK_BRACK_L);
  do
  {
    mk_expression(&par[indext++]);
    if (indext == number) break;
    mk_expect(TOK_COMMA);
  }
  while (indext < number);
  mk_expect(TOK_BRACK_R);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_get_var_value (var_t *var)
{
  unsigned int slicebegin = 0, sliceend;
  var_t exp;
  var_t *var_temp;
  int check = 1;

  var_temp = mk_get_var(script->word);
  if (! var_temp) return ERROR;
  mk_lex();
  if (script->token == TOK_SUBSCR_L)
  {
    mk_lex();
    if (script->token == TOK_DOT) check = 0;
    else
    {
      mk_expression(&exp);
      slicebegin = atoi(exp.s);
    }
    sliceend = slicebegin + 1;
    if (script->token == TOK_DOT)
    {
      mk_lex();
      mk_expect(TOK_DOT);
      if (script->token == TOK_SUBSCR_R) sliceend = var_temp->len;
      else
      {
        mk_expression(&exp);
        sliceend = atoi(exp.s) + 1;
      }
    }
    if (slicebegin < 0 || sliceend < 0) mk_error("subscript negative");
    if (   slicebegin > sliceend
        || (sliceend > var_temp->len && check && ! script->syntaxcheck))
      mk_error("subscript out of range");
    mk_expect(TOK_SUBSCR_R);
  }
  else sliceend = var_temp->len;
  var->len = sliceend - slicebegin;
  mk_get_mem(var);
  memcpy(var->s, var_temp->s + slicebegin, var->len);
  var->s[var->len] = 0;
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_num_result (long value, var_t *result)
{
  result->len = NUM_LEN;
  mk_get_mem(result);
  sprintf(result->s, "%ld", value);
  result->len = strlen(result->s);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_op_num (var_t *a, var_t *bb, var_t *result, mk_token_t token)
{
  long output = 0;
  long left = atol(a->s);
  long right = atol(bb->s);
  int numeric = 1;
  char *s;

  s = a->s;
  while (*s)
  {
    if (! isdigit(*s)) numeric = 0;
    s++;
  }
  s = bb->s;
  while (*s)
  {
    if (! isdigit(*s)) numeric = 0;
    s++;
  }
  switch (token)
  {
    case TOK_ADD: output = left + right; break;
    case TOK_SUB: output = left - right; break;
    case TOK_MUL: output = left * right; break;
    case TOK_DIV: if (right) output = left / right; // the divisor must not be equal to zero
                  else mk_error("zero division");
                  break;
    case TOK_MOD: if (right) output = left % right; // same as for dividing
                  else mk_error("zero modulo");
                  break;
    case TOK_AND: output = left && right; break;
    case TOK_OR:  output = left || right; break;
    case TOK_XOR: output = (! left) ^ (! right); break;
    case TOK_EQU: if (numeric) output = left == right;   // numeric?
                  else output = ! stricmp(a->s, bb->s); // no, compare string
                  break;
    case TOK_NE:  if (numeric) output = left != right;   // numeric?
                  else output = stricmp(a->s, bb->s); // no, compare string
                  break;
    case TOK_GREA:output = left > right; break;
    case TOK_LESS:output = left < right; break;
    case TOK_GE:  output = left >= right; break;
    case TOK_LE:  output = left <= right; break;
    default: break;
  }
  mk_num_result(output, result);
  mk_free_mem(a);
  mk_free_mem(bb);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_op_concat (var_t *a, var_t *bb, var_t *result)
{
#ifdef __DOS16__
  if (((long) a->len + (long) bb->len) > 65534L) mk_error("string too long");
#endif
  result->len = a->len + bb->len;
  mk_get_mem(result);
  memcpy(result->s, a->s, a->len);
  memcpy(result->s + a->len, bb->s, bb->len);
  result->s[result->len] = 0;
  mk_free_mem(a);
  mk_free_mem(bb);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_exec_function (var_t *result)
{
  var_t parameter[2];
  //int i;
  parameter[0].s = NULL;
  parameter[1].s = NULL;
  switch (script->token)
  {
    /*
    case TOK_BIN_HEX:
      if (mk_get_para(1, parameter);
      result->len=parameter[0].len*3;
      if (mk_get_mem(result);
      result->s[0] = 0;
      result->len = parameter[0].len*3;
      for (i=0; i<parameter[0].len; i++)
      {
        sprintf(result->s+strlen(result->s), "%02X ",
                (char)parameter[0].s[i]);
      }
      break;
    case TOK_HEX_BIN:
    {
      char *source;

      if (mk_get_para(1,parameter);
      result->len=parameter[0].len/2;
      if (mk_get_mem(result);
      source=parameter[0].s;
      result->len=0;
      while (source[0]!=0)
      {
        unsigned hex;

        while (source[0] && source[0]<=' ') source++;
        if (sscanf(source, "%2x", &hex) != 1)
          break;
        result->s[result->len]=(char)hex;
        result->len++;
        if (result->len >= parameter[0].len/2)
          break;
        if (isalnum(source[0])) source++;
        if (isalnum(source[0])) source++;
      }
      result->s[result->len] = 0;
    } break;
    case TOK_DEC_HEX:
    {
      long unsigned dec_num;
      if (mk_get_para(1, parameter);
      result->len=parameter[0].len*3;
      if (mk_get_mem(result);
      dec_num=atol(parameter[0].s);
      result->s[0]=0;
      if (dec_num>>24)
        sprintf(result->s, "%02X ", (dec_num>>24)&255);
      if (dec_num>>16)
        sprintf(result->s+strlen(result->s), "%02X ", (dec_num>>16)&255);
      if (dec_num>>8)
        sprintf(result->s+strlen(result->s), "%02X ", (dec_num>>8)&255);
      sprintf(result->s+strlen(result->s), "%02X ", (dec_num)&255);
      result->len=strlen(result->s);
    } break;
    case TOK_HEX_DEC:
    {
      char *source;
      long result_num=0;
      if (mk_get_para(1, parameter);
      result->len = NUM_LEN;
      if (mk_get_mem(result);
      source = parameter[0].s;
      while (source[0]!=0)
      { unsigned hex;
        while (source[0] == ' ') source++;
        if (sscanf(source, "%2x", &hex) != 1)
          break;
        result_num*=256;
        result_num+=hex;
        while (source[0]>' ') source++;
      }
      sprintf(result->s, "%ld", result_num);
      result->len = strlen(result->s);
    } break;
    */
    case TOK_CHR:
      mk_get_para(1,parameter);
      result->len = 1;
      mk_get_mem(result);
      result->s[0] = (char) atoi(parameter[0].s);
      result->s[1] = 0;
      break;
    case TOK_CMD:
    {
      io_t insave = t->input;
      io_t outsave = t->output;
      mk_get_para(1, parameter);
      result->len = 30000;
      mk_get_mem(result);
      b->inbuffer = parameter[0].s;
      b->inbindex = 0;
      b->inbmax = strlen(parameter[0].s);
      b->outbuffer = result->s;
      b->outbindex = 0;
      b->outbmax = result->len - 1;
      if (! *u->call) mblogin(m.boxname, login_silent, "Macro");
      t->input = io_memory;
      t->output = io_memory;
      if (! setjmp(b->io_mem_jmp)) mbmainloop(0);
      b->inbuffer = NULL;
      b->outbuffer = NULL;
      t->input = insave;
      t->output = outsave;
      result->len = b->outbindex;
      result->s[result->len] = 0;
    } break;
    /*
    case TOK_FILL:
      {
        int number;
        mk_get_para(2,parameter);
        number=atoi(parameter[0].s);
        if (number<=0)
        {
          if (script->syntaxcheck)
            number=1;
          else
          {
            mk_error("fill range constraint");
            return ERROR;
          }
        }
        result->len=number*parameter[1].len;
        mk_get_mem(result);
        for (i=0; i<number; i++)
          memcpy(result->s+i*parameter[1].len, parameter[1].s, parameter[1].len);
        result->s[result->len]=0;
      }
      break;
    */
    case TOK_GETLINE:
      result->len = LINE_LEN;
      mk_get_mem(result);
      getline(result->s, LINE_LEN, 1);
      result->len = strlen(result->s);
      break;
    case TOK_GETVALID:
      result->len = NUM_LEN;
      mk_get_mem(result);
      sprintf(result->s, "%d", getvalid());
      result->len = strlen(result->s);
      break;
    case TOK_GETUSER:
      mk_get_para(2, parameter);
      result->len = 256;
      mk_get_mem(result);
      mk_getuser(parameter[1].s, result->s, parameter[2].s);
      result->len = strlen(result->s);
      break;
    case TOK_DATESTR:
      mk_get_para(1, parameter);
      result->len = 20;
      mk_get_mem(result);
      strcpy(result->s, datestr(atol(parameter[0].s), 3));
      result->len = strlen(result->s);
      break;
    case TOK_TIMESTR:
      mk_get_para(1, parameter);
      result->len = 20;
      mk_get_mem(result);
      sprintf(result->s, "%s", datestr(atol(parameter[0].s), 4));
      result->len = strlen(result->s);
      break;
    case TOK_FILEREAD:
      mk_get_para(1, parameter);
      {
        FILE *f;
        char dataname[200];
        sprintf(dataname, "%s", parameter[0].s);
        if ((f = s_fopen(dataname, "srt")) != NULL)
        {
          fseek(f, 0, SEEK_END);
          result->len = (int) ftell(f) + 1;
          if (mk_get_mem(result) != OK)
          {
            s_fclose(f);
            return ERROR;
          }
          rewind(f);
          result->len--;
          fread(result->s, result->len, 1, f);
          result->s[result->len] = 0;
          s_fclose(f);
        }
        else mk_error("fread: cannot open file");
      }
      break;
    case TOK_LEN:
      mk_get_para(1, parameter);
      mk_num_result((long) parameter[0].len, result);
      break;
    case TOK_STRPOS:
    {
      char *match;
      long  pos = (-1);
      mk_get_para(2, parameter);
      match = stristr(parameter[0].s, parameter[1].s);
      if (match) pos = (long) (match - parameter[0].s);
      mk_num_result(pos, result);
    } break;
    case TOK_GETTIME:
      mk_lex();
      mk_num_result(ad_time(), result);
      break;
    case TOK_TIMEOUT:
    {
      static int timeout_init = 20;
      static int env = 0;
      if (! env)
      {
        if (getenv("MK_LOOP_TIMEOUT"))
          timeout_init = atoi(getenv("MK_LOOP_TIMEOUT"));
        env = 1;
      }
      mk_lex ();
      if (   script->ctl_count
          && ad_time() > (script->ctl_time[script->ctl_count - 1] + timeout_init))
      {
        trace(replog, "loop", "timeout line %d", script->line);
        mk_num_result(1, result);
      }
      else mk_num_result(0, result);
    } break;
    case TOK_VAL:
      mk_get_para(1, parameter);
      mk_num_result ((long) ((char) parameter[0].s[0]), result);
      break;
    default: mk_error("function call not implemented");
  }
  if (parameter[0].s) mk_free_mem(&parameter[0]);
  if (parameter[1].s) mk_free_mem(&parameter[1]);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_value (var_t *result)
{
  int minus = 0;

  switch (script->type)
  {
    case MK_OPERATOR:
      if (script->token == TOK_SUB) // minus sign found
      {
        mk_lex();
        minus = 1; // remember for later completion
      }
      else mk_error("operand expected");
    case MK_NUM_CONST:
    case MK_STR_CONST:
      result->len = strlen(script->word) + minus;
      mk_get_mem(result);
      if (minus) sprintf(result->s, "-%s", script->word); // concat with minus-sign
      else strcpy(result->s, script->word); // copy contents
      mk_lex();
      break;
    case MK_SYMBOL:
      if (script->token > TOK_NOTHING) return mk_exec_function(result);
      else return mk_get_var_value(result);
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_factor (var_t *result)
{
  if (script->token == TOK_BRACK_L)
  {
    mk_lex();
    mk_expression(result);
    mk_expect(TOK_BRACK_R);
  }
  else return mk_value(result);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_mul_div (var_t *result)
{
  var_t left, right;
  mk_token_t token;
  mk_factor(&left);
  token = script->token;        // spare for recursive call
  switch (token)
  {
    case TOK_MUL:
    case TOK_DIV:
    case TOK_MOD:
           mk_lex();
           mk_mul_div(&right);
           mk_op_num(&left, &right, result, token);
           break;
    default: *result = left;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_add_sub (var_t *result)
{
  var_t left, right;
  mk_token_t token;
  mk_mul_div(&left);
  token = script->token;        // spare for recursive call
  switch (token)
  {
    case TOK_ADD:
    case TOK_SUB:
           mk_lex();
           mk_add_sub(&right);
           mk_op_num(&left, &right, result, token);
           break;
    default: *result = left;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_compare (var_t *result)
{
  var_t left, right;
  mk_token_t token;
  mk_add_sub(&left);
  token = script->token;        // spare for recursive call
  switch (token)
  {
    case TOK_GREA:
    case TOK_LESS:
    case TOK_GE:
    case TOK_LE:
    case TOK_EQU:
    case TOK_NE:
           mk_lex();
           mk_compare(&right);
           mk_op_num(&left, &right, result, token);
           break;
    default: *result = left;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_and_or (var_t *result)
{
  var_t left, right;
  mk_token_t token;
  mk_compare(&left);
  token = script->token;        // spare for recursive call
  switch (token)
  {
    case TOK_AND:
    case TOK_OR:
    case TOK_XOR:
           mk_lex();
           mk_and_or(&right);
           mk_op_num(&left, &right, result, token);
           break;
    default: *result = left;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

static void mk_error (char *ermsg)
{
  if (! script)
    trace(tr_abbruch, "mk_error", "no script");
  script->error = ermsg;
  script->state = MK_ERROR;
  trace(report, "mk_error", "msg \"%s\"", ermsg);
  longjmp(script->jmpbuf, 1);
}

/*---------------------------------------------------------------------------*/

int mk_parse_script (void)
{
  if (! script) return ERROR;
  if (! setjmp(script->jmpbuf))
  {
    switch (script->state)
    {
      case MK_NEW:
        mk_prepare_script();
        script->state = MK_BEGIN;
        break;
      case MK_BEGIN:
        mk_getbegin();
        script->state = MK_BLOCK;
        break;
      case MK_BLOCK:
        if (mk_abort)
        {
          if (! script->return_ptr) mk_abort = 0;
          mk_error("abort");
        }
        else if (mk_parse_body() == OK) break;
        else
        {
          if (script->token == TOK_END)
          {
            if (script->ctl_count)
            {
              switch (script->ctl_type[script->ctl_count - 1])
              {
                case TOK_IF:     mk_expect(TOK_ENDIF);     break;
                case TOK_LOOP:   mk_expect(TOK_ENDLOOP);   break;
                default:         mk_error("unexpected");
              }
            }
            else
            {
              mk_lex();
              //mk_expect(TOK_SEMI);
              //if(script->token==TOK_EOF)
              //  script->error=NULL;
            }
          }
        }
        script->state = MK_ERROR;
        break;
      case MK_ERROR:
        mk_parse_error();
        script->state = MK_RETURN;
        break;
      case MK_RETURN:
      {
        mk_script_t *return_ptr = script->return_ptr;
        if (script->erline)
          trace(report, "macro_finish", "%s, line %d: %s",
            script->name, script->erline, script->ermessage);
        else
          trace(report, "macro_finish", "%s", script->name);
        mk_free_all();
        if (! return_ptr)
          script->state = MK_END;
        else
        {
          t_free(script);
          b->script_p = return_ptr;
          script->state = MK_BLOCK;
          trace(report, "macro", "return to %s", script->name);
        }
      } break;
      case MK_END:
      {
        t_free(script);
        b->script_p = NULL;
        return ERROR;
      }
      default: break;
    }
  }
  return OK;
}

/*---------------------------------------------------------------------------*/

int mk_expect (mk_token_t token)
{
  static char errtmp[LINE_LEN];
  int indext;
  char *keyword;

  keyword = "unknown";
  if (script->token != token)
  {
    for (indext = TOK_MIN; indext < TOK_EOF; indext++)
    {
      if (mk_wordtab[indext].token == (mk_token_t) token)
        keyword = mk_wordtab[indext].word;
    }
    sprintf(errtmp, "'%s' expected", keyword);
    mk_error(errtmp);
  }
  mk_lex();
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_call_command (void)
{
  var_t result;
  mk_script_t *new_script;
  var_t *var_temp;

  mk_lex();
  mk_expect(TOK_BRACK_L);
  mk_expression(&result);
  if (result.len >= SYMBOL_LEN) mk_error("script name too long");
  new_script = (mk_script_t *) t_malloc(sizeof(mk_script_t), "scri");
  memset(new_script, 0, sizeof(mk_script_t));
  strcpy(new_script->name, result.s);
  new_script->state = MK_NEW;
  new_script->return_ptr = script;
  new_script->nesting_level = script->nesting_level;
  while (script->token == TOK_COMMA)
  {
    mk_lex();
    var_temp = mk_get_var(script->word);
    mk_lex();
    if (! var_temp) return ERROR;
    mk_put_var(var_temp->name, var_temp, TRUE, new_script);
  }
  mk_expect(TOK_BRACK_R);
  mk_expect(TOK_SEMI);
  mk_free_mem(&result);
  b->script_p = (void *) new_script;
  trace(report, "mk_call", "%s", script->name);
  return OK;
}


/*---------------------------------------------------------------------------*/

static int mk_parse_body (void)
{
  var_t result;
  static char err[LINELEN+1];
  mk_token_t token = script->token;
  int ctl = script->ctl_count;
  int semi_expected = 1;
  result.s = NULL;

  if (token > TOK_NOTHING)
  {
    switch (token)
    {
      case TOK_END: return ERROR;
      case TOK_PUT:
      {
        char *s;
        mk_get_para(1, &result);
        s = result.s;
        while (*s) putv(*(s++));
      } break;
      case TOK_ABORT:
        mk_get_para(1, &result);
        trace(report, "abort", "%s", result.s);
        mk_abort = 1;
        break;
      case TOK_SLEEP:
        mk_get_para(1, &result);
        wdelay(atoi(result.s));
        break;
      case TOK_CMD:
        {
          io_t insave = t->input;
          mk_get_para(1, &result);
          if (! *u->call) mblogin(m.boxname, login_silent, "Macro");
          b->inbuffer = result.s;
          b->inbindex = 0;
          b->inbmax = strlen(result.s);
          t->input = io_memory;
          if (! setjmp(b->io_mem_jmp)) mbmainloop(0);
          b->inbuffer = NULL;
          t->input = insave;
        }
        break;
      case TOK_FILEWRITE:
        {
          var_t par[2];
          FILE *f;
          char dataname[200];
          char *datadir;
          datadir = getenv("Dump_DIR");
          if (! datadir) datadir = ".";
          mk_get_para(2, par);
          if (! strstr(par[0].s, "/"))
            sprintf(dataname, "%s/%s", datadir, par[0].s);
          else
            strcpy(dataname, par[0].s);
          if ((f = s_fopen(dataname, "sat")) != NULL)
          {
            fwrite(par[1].s, par[1].len, 1, f);
            s_fclose(f);
            mk_modify_var("last_error", "");
          }
          else
            mk_modify_var("last_error", "can't open file");
        }
        break;
        /*
      case TOK_NSUADD:
      {
        mk_get_para(1,&result);
        int nsuid = atoi(result.s);

          //if (nw_NSUadd(nsuid,tm_receive_pdu) == ERROR)
          {
            script->error = "NSUADD has failed";
            return ERROR;
          }
      }  break;
         */
      case TOK_OSHELL:
      {
        int ret_value = 0;
        char ret_str[LINELEN+1];
        mk_get_para(1, &result);
        ret_value = oshell(result.s, sh_noinput);
        if (ret_value)
        {
          if (ret_value >= 256) ret_value >>= 8;
          sprintf(ret_str, "%d", ret_value);
          mk_modify_var("last_error", ret_str);
        }
        else mk_modify_var("last_error", "");
      } break;
      case TOK_CALL: return mk_call_command();
      case TOK_IF:
      case TOK_LOOP:
        if (script->ctl_count == MAX_CTL)
          mk_error("too many nested control structures");
        script->ctl_pos[script->ctl_count] = script->pos;
        script->ctl_line[script->ctl_count] = script->line;
        script->ctl_type[script->ctl_count] = script->token;
        script->ctl_time[script->ctl_count] = ad_time();
        script->ctl_count++;
        semi_expected = 0;
        switch (token)
        {
          case TOK_LOOP: mk_lex(); break;
          case TOK_IF:
            mk_get_para(1, &result);
            if (! atoi(result.s) && script->syntaxcheck == FALSE)
            {
              int end_ifs = 1;
              while (end_ifs)
              {
                switch (script->token)
                {
                  case TOK_IF: end_ifs++; break;
                  case TOK_ENDIF:
                  {
                    end_ifs--;
                    if (! end_ifs) script->ctl_count--;
                    semi_expected = 1;
                  } break;
                  case TOK_ELSE: if (end_ifs == 1) end_ifs = 0; break;
                  case TOK_END:
                    sprintf(err, "'if' of line %d not closed",
                            script->ctl_line[script->ctl_count - 1]);
                    mk_error(err);
                  default: break;
                }
                mk_lex();
              }
            }
            mk_free_mem(&result);
            break;
          default: break;
        }
        break;
      case TOK_ENDLOOP:
      case TOK_EXIT:
      case TOK_EXIT_IF:
      case TOK_ELSE:
      case TOK_ENDIF:
        {
          mk_token_t type = TOK_NOTHING;
          switch (token)
          {
            case TOK_ENDLOOP:
            case TOK_EXIT:
            case TOK_EXIT_IF:    type=TOK_LOOP;   break;
            case TOK_ELSE:
            case TOK_ENDIF:      type=TOK_IF;     break;
            default: break;
          }
          if (ctl && script->ctl_type[ctl-1] == type)
          {
            switch (token)
            {
              case TOK_ENDLOOP:
                if (script->syntaxcheck == FALSE)
                {
                  script->pos = script->ctl_pos[script->ctl_count - 1];
                  script->line = script->ctl_line[script->ctl_count - 1];
                  semi_expected = 0;
                }
                else script->ctl_count--;
                mk_lex();
                break;
              case TOK_EXIT: goto doexit;
              case TOK_EXIT_IF:
                mk_get_para(1, &result);
                if (atoi (result.s) && script->syntaxcheck == FALSE)
                {
                  doexit:
                  int end_loops = 1;
                  mk_expect(TOK_SEMI);
                  while (end_loops)
                  {
                    switch (script->token)
                    {
                      case TOK_LOOP: end_loops++; break;
                      case TOK_ENDLOOP: end_loops--; break;
                      case TOK_END:
                        sprintf(err, "'loop' of line %d not closed",
                                script->ctl_line[script->ctl_count - 1]);
                        mk_error(err);
                      default: break;
                    }
                    mk_lex();
                  }
                  script->ctl_count--;
                }
                break;
              case TOK_ENDIF:
                mk_lex();
                script->ctl_count--;
                break;
              case TOK_ELSE:
              {
                int end_ifs = 1;
                mk_lex();
                if (script->syntaxcheck) return OK;
                while (end_ifs)
                {
                  switch (script->token)
                  {
                    case TOK_IF: end_ifs++; break;
                    case TOK_ENDIF: end_ifs--; break;
                    case TOK_END:
                      sprintf(err, "'if' of line %d not closed",
                              script->ctl_line[script->ctl_count - 1]);
                      mk_error(err);
                    default: break;
                  }
                  mk_lex();
                }
                script->ctl_count--;
              } break;
              default: break;
            }
          }
          else mk_error("misplaced");
        }
        break;
      default: mk_error("command not implemented");
    }
  }
  else if ((script->word[0] != ';') && (script->type == MK_SYMBOL))
  {
    char varname[SYMBOL_LEN];
    strcpy(varname, script->word);
    mk_lex();
    mk_expect(TOK_ASSIGN);
    mk_expression(&result);
    mk_put_var(varname, &result, FALSE, script);
  }
  if (result.s) mk_free_mem(&result);
  if (semi_expected) mk_expect(TOK_SEMI);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_prepare_script (void)
{
  size_t length;
  FILE *testf;

  testf = s_fopen(script->name, "srt");
  script->line = 1;
  script->linebegin = "\n";
  script->ctl_count = 0;
  script->ctl_type[0] = (mk_token_t) MK_NO_CTL;
  script->state = MK_BEGIN;
  script->ermessage[0] = 0;
  script->nesting_level++;
  strcpy(script->word, script->name);
  trace(report, "macro_start", "%s", script->name);
  if (! testf) mk_error("cannot open file");
  fseek(testf, 0, SEEK_END);
  length = (unsigned) ftell(testf);
  script->buffer = (char *) t_malloc(length + 2, "mkbu");
  if (! script->buffer)
  {
    s_fclose(testf);
    mk_error("cannot allocate script buffer");
  }
  script->pos = script->buffer;
  script->linebegin = script->buffer;
  fseek(testf, 0, SEEK_SET);
  length = fread(script->buffer, sizeof(char), length, testf);
  script->buffer[length] = 0;
  s_fclose(testf);
  return OK;
}

/*---------------------------------------------------------------------------*/

static int mk_getbegin (void)
{
  mk_lex();
  switch (script->token)
  {
    case TOK_BEGIN: break;
    default: mk_expect(TOK_BEGIN); return ERROR;
  }
  mk_lex();
  mk_modify_var("last_error", "");
  /*
  FILE *constf;
  char case_name[LINE_LEN];
  sprintf(case_name, "macro/const.bcm");
  constf = s_fopen(case_name, "srt");
  if (constf)
  {
    char s[101];
    char *value;
    int line=0;

    while (fgets(s, 100, constf))
    {
      if (strlen(s)<3)
        continue;
      line++;
      s[strlen(s)-1]=0;

      value=strchr(s, '=');
      if (value)
      {
        unsigned i;
        value[0]=0;
        do
          value++;
        while (value[0] == ' ' || value[0] == '"');

        if (strchr(value, ';'))
          strchr(value, ';')[0]=0;
        if (strchr(value, '"'))
          strchr(value, '"')[0]=0;
        if (strchr(s, ' '))
          strchr(s, ' ')[0]=0;
        for (i=0; i<strlen(s); i++)
          s[i]=tolower(s[i]);
        mk_modify_var(s, value);
      }
      else
        trace(replog, "mk_getbegin", "constants line %d syntax", line);
    }
    s_fclose(constf);
  }
  */
  return OK;
}

/*---------------------------------------------------------------------------*/

int mk_start (char *name)
{
  b->script_p = (void *) t_malloc(sizeof(mk_script_t), "scr1");
  memset(script, 0, sizeof(mk_script_t));
  sprintf(script->name, "macro/%s", name);
  if (! filesize(script->name))
  {
    t_free(script);
    b->script_p = NULL;
    return NO;
  }
  script->state = MK_NEW;
  while (mk_parse_script() != ERROR) waitfor(e_reschedule);
  return OK;
}

/*---------------------------------------------------------------------------*/
#endif
