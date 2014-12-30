/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ------------------------------------
  AX.25 Data Link Layer main functions
  ------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19990215 OE3DZW added sample init.l2
//19991028 Jan    changed printf to trace, removed paramsg.h

#include "baycom.h"

#undef fopen
#undef fclose

l2_para_t *wl = &w;
static unsigned linenumber = 0;
static int assignkanal = -1;
static int validl1 = 0;
static int errors = 0;
unsigned damatimer[KANAELE]; // wenn DAMA-Master erkannt wurde
byte damafree[KANAELE]; // Es darf ein Schwung Frames gesendet werden
int  tfaellig;          // Flag fuer Layer2-Timer zu bedienen
int  rekfl;
byte rxdaflag;

/*---------------------------------------------------------------------------*/

char *beftab[]=
    {
      "CONNECT", "MONITOR", "MYCALL", "LINKS", "MODE",
      "MCALLS", "CNOT", "CNUSER", "L2CTEXT",
      "DCALL", "BADRESS", "ASSIGN", "MHLEN",
      "TXDELAY", "TAIL", "DWAIT", "BEACON",
      "MAXFRAME", "XMITOK", "BTEXT", "DEVICE", "PEER", "PORT", "TXPORT",
      "BUFFER", "MBUFFER", "RBUFFER", "TPORTS",
      "PACLEN", "ITIME", "RETRY", "FRACK", "RESPTIME",
      "LINKTIME", "IPOLL", "DISCTIME", "MSELECT", NULL
    };

static enum befnum
    { unsinn,
      uc, moni, my, li, mode,
      mcalls, cnot, cnuser, l2ct,
      dc, ba, assign, xmhlen,
      txdelay, txtail, dwait, btime,
      maxframe, xmitok, _btext, device, peer, port, txport,
      dezwerte
    } cmd = unsinn;

static unsigned *vartab[55];

/*---------------------------------------------------------------------------*/

void l2init(void)
{
  port_t *allocblk;
  unsigned int pnummer;

  allocblk = (port_t *) xalloc((ports + 2) * sizeof(port_t));
  for (pnummer = 0; pnummer <= ports; pnummer++)
  {
    p = pfeld[pnummer] = allocblk + pnummer;
    callcopy(w.val.mycall, p->adrfield + 7);
    callcopy(w.unproto, p->adrfield);
    p->adrfield[13] |= 1;
    p->pid = 0xf0;
    p->lstate = disconnected;
    p->adrfield[13] |= 1;
    p->number = pnummer;
    p->mode.l2ctext = 1;
    lreset(0);
  }
  pfeld[MSGPORT] = allocblk + ports;
  dobeacon(TRUE);
}

/*---------------------------------------------------------------------------*/

void initax25 (int einschalt)
{
  if (einschalt)
  {
    ports = w.maxports;
    meminit();
    l2init();
  }
  l1init(einschalt);
}

/*---------------------------------------------------------------------------*/

void varset (void)
{
  unsigned int x = 0;
  vartab[x++]=&wl->maxbuf;
  vartab[x++]=&wl->maxmon;
  vartab[x++]=&wl->maxrx;
  vartab[x++]=&wl->maxports;
  vartab[x++]=&wl->val.paclen;
  vartab[x++]=&wl->val.itime;
  vartab[x++]=&wl->val.retry;
  vartab[x++]=&wl->val.frack;
  vartab[x++]=&wl->val.resptime;
  vartab[x++]=&wl->val.linktime;
  vartab[x++]=&wl->val.ipoll;
  vartab[x++]=&wl->val.disctime;
  vartab[x++]=&wl->val.mselect;
}

/*---------------------------------------------------------------------------*/

void beftest (char *befbuf)
{
  unsigned int i;
  char *befbuf1 = befbuf;

  befbuf += blkill(befbuf);
  if (! (*befbuf)) return;     /* kein Kommando eingegeben */
  cmd = (befnum) readcmd(beftab, &befbuf, 0);
  for (i = strlen(befbuf) - 1; i; i--)
  {
    if (befbuf[i] == ' ') befbuf[i] = 0;
    else break;
  }
  switch (cmd)
  {
    case unsinn:
    {
      error:
      trace(replog, "init.l2", "line %d: %s", linenumber, befbuf1);
      errors++;
    } break;
    case moni:
    {
      long unsigned mm = 0;
      while (*befbuf)
      {
        if (*befbuf > '9' || *befbuf < '0') goto error;
        mm |= 1 << (nextdez(&befbuf));
        wl->val.monitor = mm;
      }
    } break;
    case dc:
    {
      if (readcall(befbuf, wl->val.dcall) < 3) goto error;
    } break;
    case uc:
    {
      readcall(befbuf, wl->unproto);
    } break;
    case my:
    {
      readclist(befbuf, wl->val.mycall, 4);
    } break;
    case mcalls:
    {
      readclist(befbuf, wl->val.mcalls, 10);
    } break;
    case cnot:
    {
      readclist(befbuf, wl->val.cnot, 10);
    } break;
    case l2ct:
    {
      sprintf(wl->val.l2ctext, "%.79s\r", befbuf);
    } break;
    case xmhlen:
    {
      int l = nextdez(&befbuf);
      i = 0;
      while (l)
      {
        i++;
        l /= 2;
      }
      wl->mhlen = (1 << i) - 1;
    } break;
    case assign:
    {
      static int firstassign = 1;
      if (firstassign) firstassign = 0;
      if (assignkanal >= KANAELE)
      {
        trace(serious, "init.l2", "too many channels defined");
        errors++;
      }
      else
      {
        strupr(befbuf);
        validl1 = 1;
        if (! strcmp(befbuf, "KISS"))
          wl->mode[++assignkanal].chmode = c_kiss;
        else if (! strcmp (befbuf, "AXIP"))
          wl->mode[++assignkanal].chmode=c_axip;
        else if (! strcmp (befbuf, "LOOP"))
        {
          ++assignkanal;
          wl->internkanal = assignkanal;
        }
        else goto error;
        channels = assignkanal;
      }
    } break;
    case _btext:
    {
      if (validl1)
      {
        wl->mode[assignkanal].btext[0] = 0xf0;
        strcpy(wl->mode[assignkanal].btext, befbuf);
      }
    } break;
    case peer:
    case device:
      if (validl1) strcpy(wl->mode[assignkanal].device, befbuf);
      break;
    case port:
      if (validl1)
      {
        wl->mode[assignkanal].port = atoi(befbuf);
        wl->mode[assignkanal].txport = atoi(befbuf);
      }
      break;
    case txport:
      if (validl1) wl->mode[assignkanal].txport = atoi(befbuf);
      break;
    case ba:
    {
      if (validl1) readclist(befbuf, wl->mode[assignkanal].badress, 8);
    } break;
    case mode:
    {
      if (validl1)
      {
        l1_mode_t *mm = &wl->mode[assignkanal];
        mm->hbaud = nextdez(&befbuf);
        mm->t = mm->r = mm->z = mm->d = mm->c=0;
        while (*befbuf)
        {
          switch (toupper(*befbuf))
          {
            case 'T': mm->t = 1; break;
            case 'R': mm->r = 1; break;
            case 'Z': mm->z = 1; break;
            case 'D': mm->d = 1; break;
            case 'C': mm->c = 1; break;
          }
          befbuf++;
        }
      }
    } break;
    case li:
    { byte fromcall[7];
      byte tocall[7];
      int j;
      if (*befbuf)
      {
        befbuf += readcall(befbuf, tocall);
        if (*befbuf)
        {
          for (j = 0; j < 2; j++)
          {
            for (i = 0; i < MAXLINKS; i++)
            {
              if ((callcmp(tocall, wl->linkcall[i]) && ! j) || (! wl->linkcall[i][0] && j))
              {
                befbuf += readcall(befbuf, fromcall);
                if ((j = ischannel(fromcall)) < 0)
                {
                  for (j = 0; j < MAXLINKS; j++)
                  {
                    if (callcmp(fromcall, wl->linkcall[j]))
                    {
                      wl->linkkanal[i] = j + 128;
                      goto found;
                    }
                  }
                  goto error;
                }
                else wl->linkkanal[i] = j;
 found:  wl->linkoption[i] = *befbuf;
                callcopy(tocall, wl->linkcall[i]);
                goto endlinks;
              }
            }
          }
        }
 endlinks:;
      }
    } break;
    case txdelay:
    case txtail:
    case dwait:
    case btime:
    case maxframe:
    case xmitok:
      {
        if (validl1)
        {
          sscanf(befbuf, "%d", &i);
          if (assignkanal < 0)
          {
            trace(serious, "init.l2", "ASSIGN statement missing");
            goto error;
          }
          switch (cmd)
          {
            case txdelay:   wl->mode[assignkanal].txdelay = i;  break;
            case txtail:    wl->mode[assignkanal].txtail = i;   break;
            case dwait:     wl->mode[assignkanal].dwait = i;    break;
            case btime:     wl->mode[assignkanal].btime = i;    break;
            case maxframe:  wl->mode[assignkanal].maxframe = i; break;
            case xmitok:    wl->mode[assignkanal].xmitok = i;   break;
            default: break;
          }
        }
      } break;
    default:
    {
      if (cmd >= dezwerte)
      {
        if (*befbuf == '$') sscanf(befbuf + 1, "%x", &i);
        else sscanf(befbuf, "%d", &i);
//      *vartab[cmd - dezwerte] = i;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void initparameter (void)
{
  unsigned int i;
  // default l2 parameters
  memset(wl, 0, sizeof(l2_para_t));
  wl->val.headeron = 1;
  wl->val.mcalls[6] = 1;
  wl->val.cnot[6] = 1;
  wl->val.paclen = 256;
  wl->val.itime = 2;
  wl->val.ipoll = 255;
  wl->val.dama = 36;
  wl->val.retry = 40;
  wl->val.frack = 30;
  wl->val.resptime = 1;
  wl->val.linktime = 3;
  wl->val.qtime = 30;
  wl->val.disctime = 0;
  wl->val.monitor = 0xffff;
  wl->val.blink = 1;
  wl->val.mycall[6] = 1;
  for (i = 0; i < KANAELE; i++)
  {
    l1_mode_t *mm = &wl->mode[i];
    mm->txdelay = 20;
    mm->txtail = 2;
    mm->dwait = 30;
    mm->hbaud = 9600;
    mm->c = 1;
    mm->tchannel = i;
    mm->maxframe = 7;
    mm->xmitok = 1;
    mm->chmode = c_unassigned;
    strcpy(mm->device, "/dev/null");
  }
  wl->mhlen = MHLEN;
  wl->sccint[0] = 7;
  wl->comkanal = wl->fskkanal = wl->internkanal = (-1);
  wl->maxports = 201;
  wl->maxbuf = BUFLEN;
  wl->maxmon = 20;
  wl->maxrx = 20;
  wl->ipxkanal = (-1);
  wl->unproto[6] = 1;
  wl->kennung = 0xaa55;
}

/*---------------------------------------------------------------------------*/
static char l2f_init[] =
"; Sample OpenBCM Layer2 Parameterfile for AX.25\n"
";\n"
"dcall    df0ar-8    ; Digicall\n"
"monitor  4\n"
"mselect  0\n"
"mcalls   -0\n"
";\n"
";assign   kiss\n"
";mode     19200cd\n"
";device   /dev/ttyS0 ; under Windows: device COM2\n"
";\n"
";assign axip\n"
";peer   digi.local.net\n"
";port   4866\n"
";\n"
"assign axip\n"
"peer   localhost\n"
"port   4866 ; rx-port number (udp port, no raw axip)\n"
"txport 4867 ; different tx-port number\n"
";------\n";

/*---------------------------------------------------------------------------*/

void parainit (void)
{
  FILE *parmf;
  unsigned int i = 0;
  char inbuf[142];
  parmf = fopen(L2INITNAME, "rt");
  if (! parmf)
  {
    parmf = fopen(L2INITNAME, "w+t");
    if (parmf)
    {
      fwrite(l2f_init, sizeof(l2f_init) - 1, 1, parmf);
      rewind(parmf);
    }
  }
  if (parmf)
  {
    while (fgets(inbuf, sizeof(inbuf) - 1, parmf))
    {
      i = 0;
      while (inbuf[i] && inbuf[i] != LF && inbuf[i] != CR && inbuf[i] != ';')
      {
        if (inbuf[i] == 9) inbuf[i] = ' ';  // Tab weg
        i++;
      }
      linenumber++;
      if (i)
      {
        inbuf[i] = 0;
        beftest(inbuf);
      }
    }
    fclose(parmf);
  }
  else
  {
    perror("init.l2");
    errors = 100;
  }
}

/*---------------------------------------------------------------------------*/

void call_l2 (int off)
// ruft layer2 routinen auf
{
  static int sequence = 0;
  if (! sequence)
  {
    sequence = 1;
    paralloc();
    initparameter();
    varset();
    parainit();
    initax25(1);
  }
  if (off && sequence == 1)
  {
    sequence = 2;
    initax25(0);
  }
  tfaellig += 2;
  if (sequence == 1)
  {
    layer1();
    layer2();
  }
}
