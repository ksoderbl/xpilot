# Microsoft Developer Studio Project File - Name="xpilots" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=xpilots - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xpilots.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilots.mak" CFG="xpilots - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xpilots - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xpilots - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xpilots - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /D "_BETAEXPIRE" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:".\Release\XPilotNT-server.exe"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /Gy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /D "_BETAEXPIRE" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "xpilots - Win32 Release"
# Name "xpilots - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\xpilot\BetaCheck.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cmdline.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\collision.c
# End Source File
# Begin Source File

SOURCE=.\ConfBounce.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfItems.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfPlayers.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfRobots.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfShipsShots.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfTeams.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfWorld.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\contact.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\event.c
# End Source File
# Begin Source File

SOURCE=.\ExitXpilots.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\frame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\id.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\map.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\math.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\metaserver.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\net.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netserver.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\objpos.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\option.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\play.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\player.c
# End Source File
# Begin Source File

SOURCE=.\ReallyShutdown.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\robot.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\robotdef.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\rules.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\saudio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\sched.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\server.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socklib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\update.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\walls.c
# End Source File
# Begin Source File

SOURCE=..\xpilot\winNet.c
# End Source File
# Begin Source File

SOURCE=.\winSvrThread.c
# End Source File
# Begin Source File

SOURCE=..\xpilot\wsockerrs.cpp
# End Source File
# Begin Source File

SOURCE=.\xpilots.cpp
# End Source File
# Begin Source File

SOURCE=.\xpilots.rc
# End Source File
# Begin Source File

SOURCE=.\xpilotsDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\ConfBounce.h
# End Source File
# Begin Source File

SOURCE=.\ConfGeneral.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfPlayers.h
# End Source File
# Begin Source File

SOURCE=.\ConfRobots.h
# End Source File
# Begin Source File

SOURCE=.\ConfShipsShots.h
# End Source File
# Begin Source File

SOURCE=.\ConfTeams.h
# End Source File
# Begin Source File

SOURCE=.\ConfWorld.h
# End Source File
# Begin Source File

SOURCE=.\ExitXpilots.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\metaserver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netserver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\objpos.h
# End Source File
# Begin Source File

SOURCE=.\ReallyShutdown.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\robot.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\rules.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\saudio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\sched.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socklib.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\walls.h
# End Source File
# Begin Source File

SOURCE=..\xpilot\winNet.h
# End Source File
# Begin Source File

SOURCE=.\winServer.h
# End Source File
# Begin Source File

SOURCE=.\WinSvrThread.h
# End Source File
# Begin Source File

SOURCE=.\xpilots.h
# End Source File
# Begin Source File

SOURCE=.\xpilotsDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\xpilots.ico
# End Source File
# Begin Source File

SOURCE=.\res\xpilots.rc2
# End Source File
# End Group
# End Target
# End Project
