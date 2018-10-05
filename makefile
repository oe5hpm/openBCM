
# Makefile for OpenBCM-Mailbox
HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/x86/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/ppc64/powerpc/ \
	    -e s/ppc/powerpc/ \
	    -e s/macppc/powerpc/\
	    -e s/sh.*/sh/)
export HOSTARCH

GPP_VERSION3 := \
  $(shell $(CROSS_COMPILE)g++ --version | grep g++ | sed 's/.*g++ (.*) //g' | sed 's/\..*//' | grep 3)
GPP_VERSION33 := \
  $(shell $(CROSS_COMPILE)g++-3.3 --version | grep g++ | sed 's/.*g++ (.*) //g' | sed 's/\..*//' | grep 3)
GPP_VERSION4 := \
  $(shell $(CROSS_COMPILE)g++ --version | grep g++ | sed 's/.*g++ (.*) //g' | sed 's/\..*//' | grep 4)

GIT_VERSION := $(shell (git describe --abbrev=4 --dirty --always --tags || echo "?????") | sed s/-dirty/D/)

ifeq "$(GPP_VERSION33)" "3"
  CC = g++-3.3
  LD = $(CC)
else
  CC = $(CROSS_COMPILE)g++
  LD = $(CC)
endif
# ---------------------- armv6 (raspberry) specific ---------------------------
ifeq ($(PLATTFORM), armv6)
ARCHSPEC = -march=armv6zk -mfpu=vfp -mfloat-abi=hard -mcpu=arm1176jzf-s
LFLAGS =
# ---------------------- armv7hf (bur am335x pp) specific ---------------------
else ifeq ($(PLATTFORM), armv7hf)
ARCHSPEC = -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a8
LFLAGS = 
# -------------------------- x86 (default) specific ---------------------------
else ifeq ($(HOSTARCH), x86_64)
ARCHSPEC = -m32
LFLAGS   = -m32
endif

INC =
LIB =
PROGRAM = bcm
#-----------------------------------------------------------
# uncomment desired options:
LD_OPT = -dynamic $(LFLAGS)
#LD_OPT = -static $(LFLAGS)
# use static option if you compile with other linux as later running
#
# -lm math library is needed for wx-station
#LD_OPT += -lm 
#
# Debugging...
#DEBUG = -g -O0
DEBUG = -g
#-----------------------------------------------------------
LD_OPT += -lcrypt
OPT = -fno-delete-null-pointer-checks -funsigned-char -DGITVERSION=\"$(GIT_VERSION)\" $(ARCHSPEC) -std=gnu++98

OPT_WARN = -Wcomment -Wno-conversion -Wformat -Wno-unused \
	   -Wreturn-type -Wno-write-strings -Wuninitialized -Wswitch -Wshadow

ifeq "$(GPP_VERSION4)" "4"
  ifeq "$(GPP_VERSION33)" "3"
    override OPT = -O2 -fno-delete-null-pointer-checks -funsigned-char -fwritable-strings $(ARCHSPEC)
  else
    override OPT = -fno-delete-null-pointer-checks -funsigned-char -DGITVERSION=\"$(GIT_VERSION)\" $(ARCHSPEC)
    override OPT_WARN = -Wcomment -Wno-conversion -Wformat -Wno-unused -Wreturn-type -Wno-write-strings 
  endif
endif

	   
# enable following line to see all warnings
#OPT_WARN += -Wall  

.SUFFIXES: .cpp .o

DEF_BCM = -DBOXCOMPILE
DEF_L2 = -DL2COMPILE

CFLAGS = $(INC) $(DEBUG) $(OPT) $(OPT_WARN) 



BCM_OBJ=ad_linux.o alter.o ax25k.o bcast.o bids.o binsplit.o cfgflex.o charset.o \
	check.o cmd.o convers.o convert.o crc.o crontab.o desktop.o didadit.o dir.o \
	extract.o fileio.o filesurf.o ftp.o fts.o fwd.o fwd_afwd.o fwd_rli.o fwd_fbb.o \
	fwd_file.o grep.o hadr.o http.o help.o huffman.o init.o inout.o interfac.o \
	l_flex.o login.o mailserv.o main.o macro.o memalloc.o md2md5.o mdpw.o \
	morse.o msg.o nntp.o oldmaili.o ping.o pocsag.o pop3.o purge.o pw.o \
	radio.o read.o reorg.o runutils.o sema.o send.o service.o smtp.o socket.o \
	sysop.o task.o tell.o terminal.o time.o timerint.o tnc.o trace.o transfer.o \
	tree.o tty.o usercomp.o users.o utils.o vidinit.o yapp.o wp.o wx.o \
	
L2_OBJ= l1main_l.o l1kiss_l.o l1axip_l.o  \
	l2host.o l2info.o l2interf.o l2main_l.o \
	l2mhlist.o l2proto.o l2util.o ax_util.o

VERSION := $(shell grep VNUM mail.h | tr '"' '\n' | grep "1")
VERSION_NO_DOT := $(shell grep VNUM mail.h | tr '"' '\n' | grep "1" | tr -d '.')
SRCDIR = obcm-$(VERSION)
TGZ = s$(SRCDIR).tgz

all: $(PROGRAM)

.cpp.o: $(BCM_OBJ:%.o=%.cpp)
	@echo Compiling $<
	@$(CC) $(DEF_BCM) $(CFLAGS) -c $<

l1main_l.o: l1main_l.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l1kiss_l.o:  l1kiss_l.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l1axip_l.o:  l1axip_l.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2host.o: l2host.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2info.o: l2info.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2interf.o: l2interf.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2main_l.o: l2main_l.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2mhlist.o: l2mhlist.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2proto.o: l2proto.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

l2util.o: l2util.cpp
	@echo Compiling $<
	@$(CC) $(DEF_L2) $(CFLAGS) -c $<

prepare: 
ifeq "$(GPP_VERSION3)" "3"
	@echo "Usage of GCC 3.x compiler..."  
else
 ifeq "$(GPP_VERSION4)" "4"
  ifeq "$(GPP_VERSION33)" "3"
	@echo "Usage of GCC 3.3 compiler..."  
  else
	@echo "Usage of GCC 4.x compiler..."  
  endif
 endif
endif

$(PROGRAM): prepare $(L2_OBJ) $(BCM_OBJ)
	@echo Linking OpenBCM version $(GIT_VERSION)...
	$(LD) -o $(PROGRAM) $(L2_OBJ) $(BCM_OBJ) $(LD_OPT)
	$(CROSS_COMPILE)strip $(PROGRAM)

lib:    libbcm.a

libbcm.a: $(L2_OBJ) $(BCM_OBJ)
	@echo Creating libbcm.a ...
	@ar r libbcm.a $(L2_OBJ) $(BCM_OBJ)

clean:
	@echo Removing obsolete files ...
	rm -f *.o *.obj *~ *.bak *.sym *.ncb *.plg *.rej *.orig $(PROGRAM)

new:    clean all



# DO NOT DELETE THIS LINE -- make depend depends on it.
