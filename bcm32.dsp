# Microsoft Developer Studio Project File - Name="bcm32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bcm32 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bcm32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bcm32.mak" CFG="bcm32 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bcm32 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bcm32 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bcm32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\bcm"
# PROP Intermediate_Dir "c:\temp\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Release/"
# ADD F90 /include:"c:\temp\release/"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /O1 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "L2COMPILE" /D "BOXCOMPILE" /YX /J /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /fo"obcm32.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib wsock32.lib winmm.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "bcm32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\bcm"
# PROP Intermediate_Dir "c:\temp\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Debug/"
# ADD F90 /include:"c:\temp\debug/"
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "L2COMPILE" /D "BOXCOMPILE" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /fo"obcm32.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib wsock32.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bcm32 - Win32 Release"
# Name "bcm32 - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\ad_dos.h
# End Source File
# Begin Source File

SOURCE=.\ad_linux.h
# End Source File
# Begin Source File

SOURCE=.\ad_win32.h
# End Source File
# Begin Source File

SOURCE=.\ax_util.h
# End Source File
# Begin Source File

SOURCE=.\baycom.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\crctab.h
# End Source File
# Begin Source File

SOURCE=.\crctab.h
# End Source File
# Begin Source File

SOURCE=.\didadit.h
# End Source File
# Begin Source File

SOURCE=.\fbb.h
# End Source File
# Begin Source File

SOURCE=.\fileio.h
# End Source File
# Begin Source File

SOURCE=.\filesurf.h
# End Source File
# Begin Source File

SOURCE=.\flexappl.h
# End Source File
# Begin Source File

SOURCE=.\grep.h
# End Source File
# Begin Source File

SOURCE=.\huffman.h
# End Source File
# Begin Source File

SOURCE=.\hufftab.h
# End Source File
# Begin Source File

SOURCE=.\l2_appl.h
# End Source File
# Begin Source File

SOURCE=.\l2host.h
# End Source File
# Begin Source File

SOURCE=.\layer1.h
# End Source File
# Begin Source File

SOURCE=.\layer2.h
# End Source File
# Begin Source File

SOURCE=.\mail.h
# End Source File
# Begin Source File

SOURCE=.\mailserv.h
# End Source File
# Begin Source File

SOURCE=.\md2md5.h
# End Source File
# Begin Source File

SOURCE=.\mdpw.cpp
# End Source File
# Begin Source File

SOURCE=.\msg.h
# End Source File
# Begin Source File

SOURCE=.\msg.h
# End Source File
# Begin Source File

SOURCE=.\paramsg.h
# End Source File
# Begin Source File

SOURCE=.\pocsag.h
# End Source File
# Begin Source File

SOURCE=.\pop3.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\window.h
# End Source File
# Begin Source File

SOURCE=.\yapp.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\ad_dos.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\ad_win32.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\alter.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\ax_util.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\bids.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\cfgflex.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\charset.cpp
# End Source File
# Begin Source File

SOURCE=.\check.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\cmd.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\convers.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\convert.cpp
# End Source File
# Begin Source File

SOURCE=.\crc.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\crontab.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\desktop.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\didadit.cpp
# End Source File
# Begin Source File

SOURCE=.\dir.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\extract.cpp
# End Source File
# Begin Source File

SOURCE=.\fileio.cpp
# End Source File
# Begin Source File

SOURCE=.\filesurf.cpp
# End Source File
# Begin Source File

SOURCE=.\ftp.cpp
# End Source File
# Begin Source File

SOURCE=.\fts.cpp
# End Source File
# Begin Source File

SOURCE=.\fwd.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\fwd_afwd.cpp
# End Source File
# Begin Source File

SOURCE=.\fwd_fbb.cpp
# End Source File
# Begin Source File

SOURCE=.\fwd_file.cpp
# End Source File
# Begin Source File

SOURCE=.\fwd_rli.cpp
# End Source File
# Begin Source File

SOURCE=.\grep.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\hadr.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\help.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\http.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\huffman.cpp
# End Source File
# Begin Source File

SOURCE=.\init.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\inout.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\interfac.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\l1axip_l.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l1kiss_l.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l1main_l.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2host.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2info.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2interf.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2main_l.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2mhlist.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2proto.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l2util.cpp
# SUBTRACT CPP /D "BOXCOMPILE"
# End Source File
# Begin Source File

SOURCE=.\l_flex.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\login.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\macro.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\mailserv.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\md2md5.cpp
# End Source File
# Begin Source File

SOURCE=.\memalloc.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\morse.cpp
# End Source File
# Begin Source File

SOURCE=.\msg.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\nntp.cpp
# End Source File
# Begin Source File

SOURCE=.\oldmaili.cpp
# End Source File
# Begin Source File

SOURCE=.\ping.cpp
# End Source File
# Begin Source File

SOURCE=.\pocsag.cpp
# End Source File
# Begin Source File

SOURCE=.\pop3.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\purge.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\pw.cpp
# End Source File
# Begin Source File

SOURCE=.\radio.cpp
# End Source File
# Begin Source File

SOURCE=.\read.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\reorg.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\runutils.cpp
# End Source File
# Begin Source File

SOURCE=.\sema.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\send.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\service.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\smtp.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\socket.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\sysop.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\task.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\telephon.cpp
# End Source File
# Begin Source File

SOURCE=.\tell.cpp
# End Source File
# Begin Source File

SOURCE=.\terminal.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\time.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\timerint.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\tnc.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\trace.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\transfer.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\tree.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\tty.cpp
# End Source File
# Begin Source File

SOURCE=.\users.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\vidinit.cpp
# SUBTRACT CPP /D "L2COMPILE"
# End Source File
# Begin Source File

SOURCE=.\wp.cpp
# End Source File
# Begin Source File

SOURCE=.\yapp.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bcm32.ico
# End Source File
# Begin Source File

SOURCE=.\bcm32.rc
# End Source File
# End Group
# End Target
# End Project
