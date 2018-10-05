
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

install: all
	@echo Making backup of old version as ../../bcm.old
	@echo copying new bcm to ../../obcm-$(VERSION)
	@echo and installing ../../obcm-$(VERSION) as ../../bcm
	@mv -f ../../bcm ../../bcm.old
	@cp -fa bcm ../../obcm-$(VERSION)
	@cp -fa bcm ../../bcm

rpminstall: all
	@echo Making backup of old version as ../../bcm.old
	@echo copying new bcm to ../../obcm-$(VERSION)
	@echo and installing ../../obcm-$(VERSION) as ../../bcm
	@mv -f /bcm/bcm /bcm/bcm.old
	@cp -fa bcm /bcm/obcm-$(VERSION)
	@ln -s /bcm/obcm-$(VERSION) /bcm/bcm

new:    clean all

rpm:
	@echo "# spec file for package obcm" > obcm.spec
	@echo "#" >> obcm.spec
	@echo "# Copyright (c) 2013 DH8YMB Markus Baumann, Lippstadt, Germany" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "Summary: OpenBCM Packet Radio BBS" >> obcm.spec
	@echo "Name: obcm" >> obcm.spec
	@echo "Version: $(VERSION)" >> obcm.spec
	@echo "Release: 1" >> obcm.spec
	@echo "License: GPL" >> obcm.spec
	@echo "Group: unsorted" >> obcm.spec
	@echo "URL: http://dnx274.org/baybox" >> obcm.spec
	@echo "Source: sobcm-%{version}.tgz" >> obcm.spec
	@echo "BuildRoot: /var/tmp/%{name}-%{version}-root" >> obcm.spec
	@echo "Distribution: none" >> obcm.spec
	@echo "Vendor: Markus Baumann, DH8YMB, Lippstadt, Germany" >> obcm.spec
	@echo "Packager: Markus Baumann DH8YMB <dh8ymb(at)web.de>" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%description" >> obcm.spec
	@echo "OpenBCM is a full featured AX.25 packet radio mailbox for use" >> obcm.spec
	@echo "in ham radio as well as in citizen band with full store and forward" >> obcm.spec
	@echo "capabilities. It offers also a lot of TCP/IP services like HTTP," >> obcm.spec
	@echo "NNTP, POP3, SMTP and TELNET. The software is available for Linux," >> obcm.spec
	@echo "DOS and Windows NT/2000/XP." >> obcm.spec
	@echo " " >> obcm.spec
	@echo "Authors:" >> obcm.spec
	@echo "  Markus Baumann, DH8YMB" >> obcm.spec
	@echo "  Dietmar Zlabinger, OE3DZW" >> obcm.spec
	@echo "  Florian Radlheer, DL8MBT" >> obcm.spec
	@echo "  Johann Hanne, DH3MB" >> obcm.spec
	@echo "  Patrick Sesseler, DF3VI" >> obcm.spec
	@echo "  et al." >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%prep" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%setup" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%build" >> obcm.spec
	@echo "make" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%install" >> obcm.spec
	@echo "make rpminstall" >> obcm.spec
	@echo "rm -f %{buildroot}/bcm/bcm" >> obcm.spec
	@echo "mkdir -p %{buildroot}/bcm" >> obcm.spec
	@echo "mkdir -p %{buildroot}/bcm/msg" >> obcm.spec
	@echo "cp -f changes.txt %{buildroot}/bcm" >> obcm.spec
	@echo "cp -f /bcm/startbcm %{buildroot}/bcm" >> obcm.spec
	@echo "cp -f /bcm/msg/help.dl %{buildroot}/bcm/msg" >> obcm.spec
	@echo "cp -f /bcm/msg/help.gb %{buildroot}/bcm/msg" >> obcm.spec
	@echo "cp -f bcm %{buildroot}/bcm" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%clean" >> obcm.spec
	@echo "rm -f /bcm/bcm" >> obcm.spec
	@echo "rm -f /bcm/changes.txt" >> obcm.spec
	@echo " " >> obcm.spec
	@echo "%files" >> obcm.spec
	@echo "/bcm/bcm" >> obcm.spec
	@echo "/bcm/startbcm" >> obcm.spec
	@echo "/bcm/changes.txt" >> obcm.spec
	@echo "/bcm/msg/help.dl" >> obcm.spec
	@echo "/bcm/msg/help.gb" >> obcm.spec
	@echo " " >> obcm.spec
	@echo Setting up modes ...
	@chmod a-x `find . -type f`
	@echo Copying files ...
	@rm -rf $(SRCDIR)
	@mkdir $(SRCDIR)
	@cp -a `find . -type f`  $(SRCDIR)
	@echo Preparing files for archive ...
	@rm -f $(SRCDIR)/*.tgz $(SRCDIR)/*.zip $(SRCDIR)/*.o $(SRCDIR)/*~ \
		$(SRCDIR)/*.bak $(SRCDIR)/*.sym $(SRCDIR)/*.ncb \
		$(SRCDIR)/*.plg $(SRCDIR)/$(PROGRAM) $(SRCDIR)/*.orig \
		$(SRCDIR)/*.rej $(SRCDIR)/*.md5
	@md5sum -b *.cpp *.h > $(SRCDIR)/$(SRCDIR).md5
	@touch $(SRCDIR)/*
	@echo Creating $(TGZ) ...
	@tar zcf $(TGZ) $(SRCDIR)
	@rm -rf $(SRCDIR)
	@echo Creating RPM-File
	@cp *.tgz /usr/src/packages/SOURCES
	@rpmbuild -ba obcm.spec
	@rm -f obcm.spec
	@cp /usr/src/packages/BUILD/obcm-$(VERSION)/bcm /bcm/obcm-$(VERSION)
	@ln -s /bcm/obcm-$(VERSION) /bcm/bcm
	
distribute: rpm
	@cp *.tgz /http/htdocs/baybox/pre
	@cp changes.txt /http/htdocs/baybox/pre
	@cp /usr/src/packages/SRPMS/obcm-$(VERSION)-1.src.rpm /http/htdocs/baybox/pre
	@cp /usr/src/packages/RPMS/i586/obcm-$(VERSION)-1.i586.rpm /http/htdocs/baybox/pre/obcm-$(VERSION)-1.i586_OpenSuse_11.2.rpm

tgz: $(TGZ)

$(TGZ):
	@echo Setting up modes ...
	@chmod a-x `find . -type f`
	@echo Copying files ...
	@rm -rf $(SRCDIR)
	@mkdir $(SRCDIR)
	@cp -a `find . -type f`  $(SRCDIR)
	@echo Preparing files for archive ...
	@rm -f $(SRCDIR)/*.tgz $(SRCDIR)/*.zip $(SRCDIR)/*.o $(SRCDIR)/*~ \
		$(SRCDIR)/*.bak $(SRCDIR)/*.sym $(SRCDIR)/*.ncb \
		$(SRCDIR)/*.plg $(SRCDIR)/$(PROGRAM) $(SRCDIR)/*.orig \
		$(SRCDIR)/*.rej $(SRCDIR)/*.md5
	@md5sum -b *.cpp *.h > $(SRCDIR)/$(SRCDIR).md5
	@touch $(SRCDIR)/*
	@echo Creating $(TGZ) ...
	@tar zcf $(TGZ) $(SRCDIR)
	@rm -rf $(SRCDIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.
