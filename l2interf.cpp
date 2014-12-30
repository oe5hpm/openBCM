/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  Layer1-Driver


  -------------------------------------------------
  AX.25 Data Link Layer Service Interface Functions
  -------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int glob_messages = 0;
unsigned channels = 0;
int l2int = L2INT;

/*---------------------------------------------------------------------------*/

void chrout (byte zeichen)
//*************************************************************************
//
//  sendet ein Zeichen auf einem bestimmten Port im AX.25-Protokol
//  Auf Port 0 wird grundsaetzlich UI gesendet
//
//*************************************************************************
{
  frame_t *td = p->txinfout;
  if (p->number && ! p->lstate) return;
  if (! td || td->len == MAXLEN)
  {
    disable();
    if (bfree)
    {
      alloc(td);
      enable();
      td->next = p->txinfout;
      p->txinfout = td;
      td->len = 0;
      td->index = 0;
    }
    else
    {
      enable();
    }
  }
  td->inhalt[(td->len)++] = zeichen;
  p->itimer = w.val.itime;
#ifdef __FLAT__
  if (chainlen(p->txinfout) > 1)
  {
    p->itimer = 0;
    tryinfo();
  }
#endif
}

/*---------------------------------------------------------------------------*/

int chrout_frame (frame_t far *frame)
{
  frame_t *tf;
  if (bfree)
  {
    alloc(tf);
    _fmemcpy(tf, frame, sizeof(frame_t));
    tf->next = p->txinfout;
    p->txinfout = tf;
    p->itimer = 0;
    tryinfo();
    return OK;
  }
  return NO;
}

/*---------------------------------------------------------------------------*/

unsigned getlinks (void)
{
  unsigned int i;
  unsigned logins = 0;
  for (i = 1; i < ports; i++)
    if (pfeld[i] && pfeld[i]->lstate) logins++;
  return logins;
}

/*---------------------------------------------------------------------------*/

static int chrin_frame (frame_t *fr)
{
  if (p->busy && (chainlen(p->rxinfout) <= (w.maxrx / 2)))
    rnrfrei(p->number);
  if (p->rxinfout)
  {
    frame_t *rd = vonhinten(&(p->rxinfout));
    _fmemcpy(fr, rd, sizeof(frame_t));
    return OK;
  }
  else return NO;
}

/*---------------------------------------------------------------------------*/

static int chrin (unsigned int channel)
//*************************************************************************
//
//  holt ein Zeichen aus dem Empfangspuffer.
//  ist nichts im Puffer, so wird EOF uebergeben.
//
//  Aus Port 0 kommt Monitorinformation, aufbereitet in ASCII,
//  allerdings mit Farbinformation (0xff, 0xfe, 0xfd) versehen
//
//*************************************************************************
{
  frame_t *rd = p->rxinfloc;

  if (rd && rd->len == rd->index)
  {
    disable();
    dealloc(p->rxinfloc);
    enable();
    rd = NULL;
  }
  if (! rd)
  {
    if (p->busy && (chainlen(p->rxinfout) <= (w.maxrx / 2)))
      rnrfrei(p->number);
    if (p->rxinfout)
    {
      rd = p->rxinfloc = vonhinten(&(p->rxinfout));
      rd->index++;                            // jump over PID byte
      if (rd->channel && p->number && ! glob_messages)
      {
        rd->index = rd->len;
        return EOF;
      }
    }
    else
      return EOF;
  }
  if (rd->channel == channel)
    return rd->inhalt[(rd->index)++];
  else
    return EOF;
}

/*---------------------------------------------------------------------------*/

static unsigned rxvalid (void)
{
  frame_t *rd = p->rxinfloc;
  if (rd && rd->len == rd->index)
  {
    disable();
    dealloc(p->rxinfloc);
    enable();
    rd = NULL;
  }
  if (! rd)
  {
    if (p->busy && (chainlen(p->rxinfout) <= (w.maxrx / 2)))
      rnrfrei(p->number);
    if (p->rxinfout)
    {
      rd = p->rxinfloc = vonhinten(&(p->rxinfout));
      rd->index++;         // jump over PID byte
      if (rd->channel && p->number && ! glob_messages)
      {
        rd->index = rd->len;
        return NO;
      }
    }
    else
      return NO;
  }
  if (! rd->channel) return OK;
  else return NO;
}

/*---------------------------------------------------------------------------*/

/*
static unsigned rxvalid(void)
{
  frame_t *rd=p->rxinfloc;

  if (rd->len == rd->index)
  {
    disable();
    dealloc(p->rxinfloc);
    enable();
    rd=NULL;
  }
  if (! rd)
  {
    if (p->busy && (chainlen(p->rxinfout) <= (w.maxrx / 2)))
      rnrfrei(p->number);
    if (p->rxinfout)
      rd = p->rxinfloc = vonhinten(&(p->rxinfout));
    else
      return 0;
  }
  return (rd->len - rd->index);
}
*/

/*---------------------------------------------------------------------------*/

#ifndef __FLAT__

void interrupt l2aufruf (int _bp, int _di, int _si, int ds, int es,
            unsigned _dx, unsigned _cx, unsigned _bx, unsigned _ax)
{
  static unsigned ss, sp, ax, bx, cx, dx, bp;
  static char far *ptr;
  asm jmp weiter;
  asm db 'BayCom'
  asm db 3
  asm db 0
  weiter:
  /*
  if (_ax < 128)
  { _ax = layer1 (_ax);
    if (_ax) _dx = _DS;
    else _dx = 0;
    return;
  }
  */
  disable();
  if (rekfl)
  {
    enable();
    sound(1800);
    delay(1000);
    nosound();
    return;
  }
  rekfl = TRUE;
  ax = _ax;
  bx = _bx;
  cx = _cx;
  dx = _dx;
  bp = _BP;
  ss = _SS;
  sp = _SP;
  _SS = FP_SEG(stack);             // Statisches Array zuweisen
  _SP = FP_OFF(stack) + STACKLEN;  // Stackpointer ganz oben
  _BP = _SP;
  enable();
  ptr = (char far *) MK_FP(dx, cx);
  dx = FP_SEG((void far *) &w);

#else

int l2aufruf(int ax, int bx, int cx, byte *ptr)
{

#endif

  p = pfeld[bx];
  switch (ax)
  {
    case l2_put_char: chrout(cx); break;     // BX stream    CX char
    case l2_set_pid: p->pid = cx; break;     // BX stream    CX contains new PID
    case l2_get_lstate: p->mode.open = 1; ax = p->lstate; break;
    case l2_get_outstanding: ax = p->outstanding; break;
    case l2_put_flush: p->itimer = 0; tryinfo(); break;  // BX stream
    case l2_put_frame: ax = chrout_frame((frame_t *) ptr); break; // BX stream *DX:CX info field (including PID) (char *)
    case l2_get_char:  ax = chrin(0); break; // BX stream    return AX
    case l2_get_msg:   ax = chrin(1); break; // BX stream    return AX
    case l2_get_frame: ax = chrin_frame((frame_t *) ptr); break; // BX stream    return *DX:AX (char *)
    case l2_put_free:                        // BX stream    return AX
     ax = 0;
     if (((int) bx) == (-1))
       dobeacon(TRUE);
     else
     {
#ifndef  __FLAT__
       if (chainlen(p->txinfout) <=
           ((w.mode[p->channel].maxframe * w.val.paclen) / 330 + 2) && bfree) ax = 255;
#else
       if (chainlen(p->txinfout) <= 15 && bfree) ax = 255;
#endif
     }
     break;
    case l2_get_valid: ax = rxvalid(); break;                  // BX stream    return AX
    case l2_connect_:                                          // BX stream
         _fmemcpy(l2buf, ptr + 1, 70);
         connect(l2buf, l2buf + 7, ptr[0]);
         break;
    case l2_disconnect: disconnect(cx); break;                 // BX stream    CX mode
    case l2_get_l1state: ax = l1state(bx); break;              // BX channel   return AX
    case l2_get_free: ax = chainlen(bfree); break;             //              return AX
    case l2_get_channels: ax = channels; break;                //              return AX
    case l2_get_streams: ax = ports; break;                    //              return AX
    case l2_get_mhlen: ax = w.mhlen; break;                    //              return AX
    case l2_get_parlen: ax = sizeof(w); break;                 //              return AX
    case l2_get_ticker:                                        //              return DX:AX  (time_t)
#ifndef __FLAT__
         ax = (unsigned) w.zeit;
         dx = (*(((unsigned *) &w.zeit) + 1));
#else
         ax = 0;        //     dx=(char *)time(NULL);
#endif
         break;
    case l2_get_newdata: ax = 0; break;                        //              return AX (stream)
    case l2_set_terminal:                                      // BX 0:terminal off  1:terminal on  2:return state
         if (bx != 2)
         {
           if (w.val.blink == 3) w.val.blink = 2;
           w.sccon = bx;
         }
         ax = w.sccon;
         break;
    case l2_set_handler: break;                                // *DX:CX void interrupt (*handler)(...) (AX=stream)
    case l2_set_msg: glob_messages = 1; break;
    case l2_ptr_stream: ax = (unsigned) pfeld[bx]; break;      // BX stream    return *DX:AX (stream_t *)
    case l2_ptr_l1_mode: ax = (unsigned) &w.mode[bx]; break;   // BX channel   return *DX:AX (l1_mode_t *)
    case l2_ptr_values: ax = (unsigned) &w.val; break;         //            return *DX:AX (l2_value_t *)
    case l2_ptr_mheard: ax = (unsigned) l2_mhlist; break;      //            return *DX:AX (mheard_t *)
    case l2_ptr_par: ax = (unsigned) &w; break;                //            return *DX:AX (unknown *)
    case l2_ptr_version: ax = (unsigned) VNUMMER; break;       //            return *DX:AX (char *)
#ifndef __FLAT__
    default:
      if (ax == 0xfffe)
      {
        for (unsigned int i = 0; pfeld[i]; i++)
        {
          if (pfeld[i]->lstate > disconnected)
          {
            printf("\aDisconnect streams before turning off.\n");
            ax = 0;
            goto end;
          }
        }
        ax++;
      }
      if (ax == 0xffff)
      { /*
        if (l1_user_on)
        {
          printf("\aL1-user still alive. Remove it and try again.\n");
          ax = 0;
          goto end;
        }
        */
        initax25(0);
        freemem(((unsigned far *) MK_FP(_psp,0x2c))[0]);
        freemem(_psp);
      }
#endif
  }
#ifndef __FLAT__
  end:
  disable();
  rekfl = 0;
  _SS = ss;
  _SP = sp;
  _BP = bp;
  _ax = ax;
  _dx = dx;
#else
  return ax;
#endif
}

/*---------------------------------------------------------------------------*/

void intinit (int ein)
{
#ifndef __FLAT__
  static void interrupt (*altvec47) (...);
  if (ein)
  {
    altvec47 = getvect(l2int);
    setvect(l2int, (void interrupt (*)(...)) l2aufruf);
  }
  else
    setvect(l2int, altvec47);
#endif
}
