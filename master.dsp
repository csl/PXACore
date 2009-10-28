# Microsoft Developer Studio Project File - Name="master" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=master - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "master.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "master.mak" CFG="master - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "master - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "master - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "master - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x110000" /entry:"?__init@@YAXXZ" /dll /map /machine:I386 /ALIGN:16
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "master - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x110000" /entry:"?__init@@YAXXZ" /dll /debug /machine:I386 /pdbtype:sept /ALIGN:16
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "master - Win32 Release"
# Name "master - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "NetCore_Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetCore\IPV4_IMP.CPP
# End Source File
# Begin Source File

SOURCE=.\NetCore\LOW_API.CPP
# End Source File
# Begin Source File

SOURCE=.\NetCore\NET_COMM.CPP
# End Source File
# Begin Source File

SOURCE=.\NetCore\NETBUFF.CPP
# End Source File
# Begin Source File

SOURCE=.\NetCore\UDP_IMP.CPP
# End Source File
# End Group
# Begin Group "Drivers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DRIVERS\COMDRV.CPP
# End Source File
# Begin Source File

SOURCE=.\Drivers\PCI_DRV.CPP
# End Source File
# Begin Source File

SOURCE=.\Drivers\RT8139.CPP
# End Source File
# End Group
# Begin Group "Kernel"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Kernel\BUFFMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\COMQUEUE.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\DEVMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\dim.cpp
# End Source File
# Begin Source File

SOURCE=.\Kernel\HEAP.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\HELLOCN.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\IDEHDDRV.CPP
# End Source File
# Begin Source File

SOURCE=.\KERNEL\IOMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\KERNEL\KAPI.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\KEYHDLR.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\KMEMMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\KTHREAD.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\KTMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\KERNEL\KTMGR2.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\MAILBOX.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\memmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Kernel\OBJMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\OBJQUEUE.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\PAGEIDX.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\PERF.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\SHELL.CPP
# End Source File
# Begin Source File

SOURCE=.\KERNEL\STATCPU.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\SYN_MECH.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\SYNOBJ.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\SYSTEM.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\TASKCTRL.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\TIMER.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\TYPES.CPP
# End Source File
# Begin Source File

SOURCE=.\Kernel\VMM.CPP
# End Source File
# End Group
# Begin Group "LIB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LIB\L_STDIO.CPP
# End Source File
# Begin Source File

SOURCE=.\LIB\STRING.CPP
# End Source File
# End Group
# Begin Group "KTHREAD"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\KTHREAD\IOCTRL_S.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\STAT_S.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\SYSD_S.CPP
# End Source File
# End Group
# Begin Group "ARCH"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ARCH\ARCH_X86.CPP
# End Source File
# End Group
# Begin Source File

SOURCE=.\DRVENTRY.CPP
# End Source File
# Begin Source File

SOURCE=.\EXTCMD.CPP
# End Source File
# Begin Source File

SOURCE=.\FIBONACCI.CPP
# End Source File
# Begin Source File

SOURCE=.\OS_ENTRY.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "NetCore"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetCore\DEFINES.H
# End Source File
# Begin Source File

SOURCE=.\NetCore\IPV4_DEF.H
# End Source File
# Begin Source File

SOURCE=.\NetCore\LOW_API.H
# End Source File
# Begin Source File

SOURCE=.\NetCore\NET_COMM.H
# End Source File
# Begin Source File

SOURCE=.\NetCore\NETBUFF.H
# End Source File
# Begin Source File

SOURCE=.\NetCore\NetStd.h
# End Source File
# Begin Source File

SOURCE=.\NetCore\UDP_DEF.H
# End Source File
# End Group
# Begin Group "INCLUDE"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\INCLUDE\ARCHSTD.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\BUFFMGR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\COMMOBJ.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\COMQUEUE.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\DEVMGR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\dim.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\GLOBVAR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\HEAP.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\HELLOCN.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\IDEHDDRV.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\IOCTRL_S.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\iomgr.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\IPV6DEF.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KAPI.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KMEMMGR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KRNLSTD.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KTHREAD.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KTMGR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KTMSG.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\L_STDIO.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\LIST.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\MAILBOX.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\memmgr.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\OBJQUEUE.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PAGEIDX.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PCI_DRV.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PERF.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\RT8139.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\SHELL.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\STAT_S.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\STATCPU.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\STRING.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\SYN_MECH.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\SYNOBJ.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\SYSD_S.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\SYSTEM.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\TASKCTRL.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\TIMER.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\TYPES.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\VMM.H
# End Source File
# End Group
# Begin Group "DrvHdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DRIVERS\COMMDRV.H
# End Source File
# End Group
# Begin Source File

SOURCE=.\EXTCMD.H
# End Source File
# Begin Source File

SOURCE=.\FIBONACCI.H
# End Source File
# Begin Source File

SOURCE=.\HCNAPI.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
