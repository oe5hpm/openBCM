/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -----------------------------------------------
  Definitionen und Deklarationen fuer DOS-Version
  -----------------------------------------------

  Copyright (c) 92-96 Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/

//19980610 OE3DZW altered LOCK_MAX,SEMA_MAX,MAXCONVLIFE
//19980709 DH3MB  moved definition of BCM_MACRO to baycom.h

#ifdef __MSDOS__

#ifndef _WIN32
#include <alloc.h>
#include <bios.h>
#include <conio.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <dirent.h>
#include <limits.h>
#include <share.h>

//some BC-compilers need this setting, some not..
#ifndef _NFILE_
        #define _NFILE_        25
        //df3vi: reduced from 50 to 25, it's enough anyway
#endif
        #define random_max random
        #define CREATMODE      0
        #ifdef __DPMI32__
                #define ad_setsp(newsp) static unsigned splocal; \
                        splocal=(unsigned)(newsp); _ESP=splocal;
        #else
                #define ad_setsp(newsp) static unsigned sslocal,splocal; \
                        sslocal=FP_SEG((void far *)(newsp)); \
                        splocal=FP_OFF((void far *)(newsp)); _SS=sslocal; _SP=splocal;
        #endif
        #define BIOS        ((unsigned far *)MK_FP(0x40,0))

#endif

#define SHAREMODE SH_DENYNONE
#define MS_PER_TIC  55
#define fncase strupr

#ifndef _OFF_T_DEFINED
//for MSVCPP
typedef unsigned long int off_t; // DH3MB
#endif

void desk_cyclic(void);

#ifdef __DPMI32__
        void setvect(int num,void interrupt (*isr)( ... ));
        void interrupt(*getvect(int num))(...);
#endif

#if defined(__DPMI32__)

        #define ad_int int386

        void copyToLow(unsigned short dst_seg, unsigned short dst_offset,
                        void * src, unsigned int size);
        void copyFromLow(void * dst, unsigned short src_seg,
                        unsigned short src_offset, unsigned int size);

        unsigned long  ad_peekd(unsigned short seg, unsigned short offset);
        unsigned short ad_peek(unsigned short seg, unsigned short offset);
                 char  ad_peekb(unsigned short seg, unsigned short offset);

        void ad_poked(unsigned short seg, unsigned short offset, unsigned long dw);
        void ad_poke(unsigned short seg, unsigned short offset, unsigned short w);
        void ad_pokeb(unsigned short seg, unsigned short offset, char  c);

#else

        #define ad_int int86
        #define ad_poke poke
        #define ad_pokeb pokeb
        #define ad_poked poked
        #define ad_peek peek
        #define ad_peekb peekb
        #define ad_peekd peekd

        #define copyToLow(dst_seg,dst_offset,src,size)   \
                          _fmemcpy(MK_FP(dst_seg,dst_offset), src, size)
        #define copyFromLow(dst,src_seg,src_offset,size) \
                          _fmemcpy(dst, MK_FP(src_seg,src_offset), size)

#endif

#endif
void waitdoskey(void);
