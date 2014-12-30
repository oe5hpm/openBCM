/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  -------------------------------------------------
  Definitionen und Deklarationen fuer Win32-Version
  -------------------------------------------------


  Copyright (c) 92-96 Florian Radlherr
                      Taubenbergstr. 32
                      D-83627 Warngau

  Alle Rechte vorbehalten / All Rights reserved

 ***************************************************************/


#ifdef _WIN32

#include <windows.h>
#include <process.h>
#include <memory.h>
#include <signal.h>
#include <io.h>
#include <direct.h>
#include <share.h>
#include <limits.h>

#define ENOTCONN    WSAENOTCONN
#define EBADF       WSAEBADF
#define EWOULDBLOCK WSAEWOULDBLOCK

#define _NFILE_ 50
#define SH_DENYNONE _SH_DENYNO
#define O_RDWR _O_RDWR
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#define O_APPEND _O_APPEND
#define CREATMODE (_S_IREAD | _S_IWRITE)
#define chdir _chdir
#define heapcheck() 0
#define coreleft() (1000000L)
#define randomize() srand((unsigned)time(NULL))
int random_max(int);
int bioskey(int);
#define lseek _lseek
#define filelength _filelength
typedef struct DIR
{ _finddata_t ft;
  long handle;
} DIR;
typedef struct dirent
{ char d_name[80];
} dirent;
DIR *opendir(char *);
struct dirent *readdir(DIR *);
void closedir(DIR *);
#pragma warning(disable:4018)
#pragma warning(disable:4305)
#pragma warning(disable:4309)
#define ad_setsp(newsp) static unsigned splocal; \
        splocal=(unsigned)(newsp); __asm mov esp, splocal;
#define _fmemcpy memcpy
#define far
#define near
#define interrupt
#define sound(x)
#define nosound()
#define disable()
#define enable()
#define delay(x)
#define _fmemcpy memcpy

unsigned ad_minutes(void);
void coldstart(void);
void kill(void *p);
// replacement for Linux setenv function under Win32
void setenv(char *lpname, char *lpvalue, int dummy);

#endif
