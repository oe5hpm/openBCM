/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  ------------------------------------------------
  Definitionen und Deklarationen für Linux-Version
  ------------------------------------------------


  Copyright (c) 92-96 Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/
#ifdef __LINUX__
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>

#if !defined(__sparc__) && !defined(__arm__)      //Always Assembler .... F1ULQ
#define ad_setsp(sp) register unsigned _esp asm("esp");_esp=(unsigned)(sp);
#endif

#if defined(__sparc__) || defined (__arm__)        //Dummy for Sparc ..... F1ULQ
#define ad_setsp(sp) register unsigned _esp asm("sp");_esp=(unsigned)(sp);
#endif

#define MS_PER_TIC 55
#define CREATMODE 0666
#define fncase strlwr
#define far
#define near
#define interrupt
#define sound(x)
#define nosound()
#define disable()
#define enable()
#define delay(x)
#define _fmemcpy memcpy
#define desk_cyclic()
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _open  open
#define sopen(x,y,z) open(x,y)
#define _fsopen(x,y,z) fopen(x,y)
#define _creat creat
#define _close close
#define _read  read
#define _write write
#define FA_DIREC 1
#define O_BINARY 0
#define _NFILE_ 256
#define randomize() srandom(time(NULL))

struct  ffblk
{
  DIR  *ff_dir;
  char ff_attrib;
  long ff_fsize;
  char ff_name[13];
};

unsigned ad_minutes(void);
void coldstart(void);
char *strupr(char *s);
char *strlwr(char *s);
int chdir(const char *path);
int mkdir(const char *path);
int bioskey(int);
long filelength(int handle);
int random_max(int);
int setlinuxpasswd(char *calltmp, char *passwort);

#endif

