# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=xpilots - Win32 Release
!MESSAGE No configuration specified.  Defaulting to xpilots - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "xpilots - Win32 Release" && "$(CFG)" !=\
 "xpilots - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilots.mak" CFG="xpilots - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xpilots - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xpilots - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "xpilots - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "xpilots - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\xpilots.exe" "$(OUTDIR)\xpilots.bsc"

CLEAN : 
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\ConfBounce.obj"
	-@erase "$(INTDIR)\ConfBounce.sbr"
	-@erase "$(INTDIR)\ConfGeneral.obj"
	-@erase "$(INTDIR)\ConfGeneral.sbr"
	-@erase "$(INTDIR)\ConfigDialog.obj"
	-@erase "$(INTDIR)\ConfigDialog.sbr"
	-@erase "$(INTDIR)\ConfigDlg.obj"
	-@erase "$(INTDIR)\ConfigDlg.sbr"
	-@erase "$(INTDIR)\ConfItems.obj"
	-@erase "$(INTDIR)\ConfItems.sbr"
	-@erase "$(INTDIR)\ConfPlayers.obj"
	-@erase "$(INTDIR)\ConfPlayers.sbr"
	-@erase "$(INTDIR)\ConfRobots.obj"
	-@erase "$(INTDIR)\ConfRobots.sbr"
	-@erase "$(INTDIR)\ConfShipsShots.obj"
	-@erase "$(INTDIR)\ConfShipsShots.sbr"
	-@erase "$(INTDIR)\ConfTeams.obj"
	-@erase "$(INTDIR)\ConfTeams.sbr"
	-@erase "$(INTDIR)\ConfWorld.obj"
	-@erase "$(INTDIR)\ConfWorld.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\event.sbr"
	-@erase "$(INTDIR)\ExitXpilots.obj"
	-@erase "$(INTDIR)\ExitXpilots.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\id.sbr"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\map.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\metaserver.obj"
	-@erase "$(INTDIR)\metaserver.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netserver.obj"
	-@erase "$(INTDIR)\netserver.sbr"
	-@erase "$(INTDIR)\objpos.obj"
	-@erase "$(INTDIR)\objpos.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\play.obj"
	-@erase "$(INTDIR)\play.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\ReallyShutdown.obj"
	-@erase "$(INTDIR)\ReallyShutdown.sbr"
	-@erase "$(INTDIR)\robot.obj"
	-@erase "$(INTDIR)\robot.sbr"
	-@erase "$(INTDIR)\robotdef.obj"
	-@erase "$(INTDIR)\robotdef.sbr"
	-@erase "$(INTDIR)\rules.obj"
	-@erase "$(INTDIR)\rules.sbr"
	-@erase "$(INTDIR)\saudio.obj"
	-@erase "$(INTDIR)\saudio.sbr"
	-@erase "$(INTDIR)\sched.obj"
	-@erase "$(INTDIR)\sched.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\update.obj"
	-@erase "$(INTDIR)\update.sbr"
	-@erase "$(INTDIR)\walls.obj"
	-@erase "$(INTDIR)\walls.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winSvrThread.obj"
	-@erase "$(INTDIR)\winSvrThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xpilots.obj"
	-@erase "$(INTDIR)\xpilots.res"
	-@erase "$(INTDIR)\xpilots.sbr"
	-@erase "$(INTDIR)\xpilotsDlg.obj"
	-@erase "$(INTDIR)\xpilotsDlg.sbr"
	-@erase "$(OUTDIR)\xpilots.bsc"
	-@erase "$(OUTDIR)\xpilots.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /Fr /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/xpilots.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/xpilots.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\ConfBounce.sbr" \
	"$(INTDIR)\ConfGeneral.sbr" \
	"$(INTDIR)\ConfigDialog.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
	"$(INTDIR)\ConfItems.sbr" \
	"$(INTDIR)\ConfPlayers.sbr" \
	"$(INTDIR)\ConfRobots.sbr" \
	"$(INTDIR)\ConfShipsShots.sbr" \
	"$(INTDIR)\ConfTeams.sbr" \
	"$(INTDIR)\ConfWorld.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\event.sbr" \
	"$(INTDIR)\ExitXpilots.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\id.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\metaserver.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netserver.sbr" \
	"$(INTDIR)\objpos.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\play.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\ReallyShutdown.sbr" \
	"$(INTDIR)\robot.sbr" \
	"$(INTDIR)\robotdef.sbr" \
	"$(INTDIR)\rules.sbr" \
	"$(INTDIR)\saudio.sbr" \
	"$(INTDIR)\sched.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\update.sbr" \
	"$(INTDIR)\walls.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winSvrThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xpilots.sbr" \
	"$(INTDIR)\xpilotsDlg.sbr"

"$(OUTDIR)\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug /nodefaultlib
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/xpilots.pdb" /machine:I386 /out:"$(OUTDIR)/xpilots.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\ConfBounce.obj" \
	"$(INTDIR)\ConfGeneral.obj" \
	"$(INTDIR)\ConfigDialog.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
	"$(INTDIR)\ConfItems.obj" \
	"$(INTDIR)\ConfPlayers.obj" \
	"$(INTDIR)\ConfRobots.obj" \
	"$(INTDIR)\ConfShipsShots.obj" \
	"$(INTDIR)\ConfTeams.obj" \
	"$(INTDIR)\ConfWorld.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\ExitXpilots.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\metaserver.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netserver.obj" \
	"$(INTDIR)\objpos.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\play.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\ReallyShutdown.obj" \
	"$(INTDIR)\robot.obj" \
	"$(INTDIR)\robotdef.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\saudio.obj" \
	"$(INTDIR)\sched.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\update.obj" \
	"$(INTDIR)\walls.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winSvrThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xpilots.obj" \
	"$(INTDIR)\xpilots.res" \
	"$(INTDIR)\xpilotsDlg.obj"

"$(OUTDIR)\xpilots.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\xpilots.exe" "$(OUTDIR)\xpilots.bsc"

CLEAN : 
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\ConfBounce.obj"
	-@erase "$(INTDIR)\ConfBounce.sbr"
	-@erase "$(INTDIR)\ConfGeneral.obj"
	-@erase "$(INTDIR)\ConfGeneral.sbr"
	-@erase "$(INTDIR)\ConfigDialog.obj"
	-@erase "$(INTDIR)\ConfigDialog.sbr"
	-@erase "$(INTDIR)\ConfigDlg.obj"
	-@erase "$(INTDIR)\ConfigDlg.sbr"
	-@erase "$(INTDIR)\ConfItems.obj"
	-@erase "$(INTDIR)\ConfItems.sbr"
	-@erase "$(INTDIR)\ConfPlayers.obj"
	-@erase "$(INTDIR)\ConfPlayers.sbr"
	-@erase "$(INTDIR)\ConfRobots.obj"
	-@erase "$(INTDIR)\ConfRobots.sbr"
	-@erase "$(INTDIR)\ConfShipsShots.obj"
	-@erase "$(INTDIR)\ConfShipsShots.sbr"
	-@erase "$(INTDIR)\ConfTeams.obj"
	-@erase "$(INTDIR)\ConfTeams.sbr"
	-@erase "$(INTDIR)\ConfWorld.obj"
	-@erase "$(INTDIR)\ConfWorld.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\event.sbr"
	-@erase "$(INTDIR)\ExitXpilots.obj"
	-@erase "$(INTDIR)\ExitXpilots.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\id.sbr"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\map.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\metaserver.obj"
	-@erase "$(INTDIR)\metaserver.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netserver.obj"
	-@erase "$(INTDIR)\netserver.sbr"
	-@erase "$(INTDIR)\objpos.obj"
	-@erase "$(INTDIR)\objpos.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\play.obj"
	-@erase "$(INTDIR)\play.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\ReallyShutdown.obj"
	-@erase "$(INTDIR)\ReallyShutdown.sbr"
	-@erase "$(INTDIR)\robot.obj"
	-@erase "$(INTDIR)\robot.sbr"
	-@erase "$(INTDIR)\robotdef.obj"
	-@erase "$(INTDIR)\robotdef.sbr"
	-@erase "$(INTDIR)\rules.obj"
	-@erase "$(INTDIR)\rules.sbr"
	-@erase "$(INTDIR)\saudio.obj"
	-@erase "$(INTDIR)\saudio.sbr"
	-@erase "$(INTDIR)\sched.obj"
	-@erase "$(INTDIR)\sched.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\update.obj"
	-@erase "$(INTDIR)\update.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\walls.obj"
	-@erase "$(INTDIR)\walls.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winSvrThread.obj"
	-@erase "$(INTDIR)\winSvrThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xpilots.obj"
	-@erase "$(INTDIR)\xpilots.res"
	-@erase "$(INTDIR)\xpilots.sbr"
	-@erase "$(INTDIR)\xpilotsDlg.obj"
	-@erase "$(INTDIR)\xpilotsDlg.sbr"
	-@erase "$(OUTDIR)\xpilots.bsc"
	-@erase "$(OUTDIR)\xpilots.exe"
	-@erase "$(OUTDIR)\xpilots.ilk"
	-@erase "$(OUTDIR)\xpilots.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /Gy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /Gy /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/xpilots.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/xpilots.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\ConfBounce.sbr" \
	"$(INTDIR)\ConfGeneral.sbr" \
	"$(INTDIR)\ConfigDialog.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
	"$(INTDIR)\ConfItems.sbr" \
	"$(INTDIR)\ConfPlayers.sbr" \
	"$(INTDIR)\ConfRobots.sbr" \
	"$(INTDIR)\ConfShipsShots.sbr" \
	"$(INTDIR)\ConfTeams.sbr" \
	"$(INTDIR)\ConfWorld.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\event.sbr" \
	"$(INTDIR)\ExitXpilots.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\id.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\metaserver.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netserver.sbr" \
	"$(INTDIR)\objpos.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\play.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\ReallyShutdown.sbr" \
	"$(INTDIR)\robot.sbr" \
	"$(INTDIR)\robotdef.sbr" \
	"$(INTDIR)\rules.sbr" \
	"$(INTDIR)\saudio.sbr" \
	"$(INTDIR)\sched.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\update.sbr" \
	"$(INTDIR)\walls.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winSvrThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xpilots.sbr" \
	"$(INTDIR)\xpilotsDlg.sbr"

"$(OUTDIR)\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/xpilots.pdb" /debug /machine:I386 /out:"$(OUTDIR)/xpilots.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\ConfBounce.obj" \
	"$(INTDIR)\ConfGeneral.obj" \
	"$(INTDIR)\ConfigDialog.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
	"$(INTDIR)\ConfItems.obj" \
	"$(INTDIR)\ConfPlayers.obj" \
	"$(INTDIR)\ConfRobots.obj" \
	"$(INTDIR)\ConfShipsShots.obj" \
	"$(INTDIR)\ConfTeams.obj" \
	"$(INTDIR)\ConfWorld.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\ExitXpilots.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\metaserver.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netserver.obj" \
	"$(INTDIR)\objpos.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\play.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\ReallyShutdown.obj" \
	"$(INTDIR)\robot.obj" \
	"$(INTDIR)\robotdef.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\saudio.obj" \
	"$(INTDIR)\sched.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\update.obj" \
	"$(INTDIR)\walls.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winSvrThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xpilots.obj" \
	"$(INTDIR)\xpilots.res" \
	"$(INTDIR)\xpilotsDlg.obj"

"$(OUTDIR)\xpilots.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "xpilots - Win32 Release"
# Name "xpilots - Win32 Debug"

!IF  "$(CFG)" == "xpilots - Win32 Release"

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "xpilots - Win32 Release"

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilots.cpp
DEP_CPP_XPILO=\
	".\StdAfx.h"\
	".\WinSvrThread.h"\
	".\xpilots.h"\
	".\xpilotsDlg.h"\
	

"$(INTDIR)\xpilots.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

"$(INTDIR)\xpilots.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilotsDlg.cpp
DEP_CPP_XPILOT=\
	"..\..\..\src\version.h"\
	".\ConfBounce.h"\
	".\ConfGeneral.h"\
	".\ConfigDialog.h"\
	".\ConfigDlg.h"\
	".\ConfItems.h"\
	".\ConfPlayers.h"\
	".\ConfRobots.h"\
	".\ConfShipsShots.h"\
	".\ConfTeams.h"\
	".\ConfWorld.h"\
	".\ExitXpilots.h"\
	".\ReallyShutdown.h"\
	".\StdAfx.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	".\xpilots.h"\
	".\xpilotsDlg.h"\
	

"$(INTDIR)\xpilotsDlg.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

"$(INTDIR)\xpilotsDlg.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilots.rc
DEP_RSC_XPILOTS=\
	".\res\icon1.ico"\
	".\res\xpilots.ico"\
	".\res\xpilots.rc2"\
	

"$(INTDIR)\xpilots.res" : $(SOURCE) $(DEP_RSC_XPILOTS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\walls.c

!IF  "$(CFG)" == "xpilots - Win32 Release"

DEP_CPP_WALLS=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\walls.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_WALLS=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\walls.obj" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\walls.sbr" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

DEP_CPP_WALLS=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\walls.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\walls.obj" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\walls.sbr" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\update.c
DEP_CPP_UPDAT=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_UPDAT=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\update.sbr" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\socklib.c
DEP_CPP_SOCKL=\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SOCKL=\
	"..\..\..\src\termnet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\server.c
DEP_CPP_SERVE=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\portability.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\sched.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	
NODEP_CPP_SERVE=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\server.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\server.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\saudio.c
DEP_CPP_SAUDI=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	
NODEP_CPP_SAUDI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\saudio.obj" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\saudio.sbr" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\rules.c
DEP_CPP_RULES=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	
NODEP_CPP_RULES=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\rules.obj" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\rules.sbr" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\robot.c
DEP_CPP_ROBOT=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\robot.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_ROBOT=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\robot.obj" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\robot.sbr" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\player.c
DEP_CPP_PLAYE=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_PLAYE=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\player.obj" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\player.sbr" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\play.c

!IF  "$(CFG)" == "xpilots - Win32 Release"

DEP_CPP_PLAY_=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_PLAY_=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\play.obj" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\play.sbr" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

DEP_CPP_PLAY_=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\play.obj" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\play.sbr" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\option.c
DEP_CPP_OPTIO=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\defaults.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	
NODEP_CPP_OPTIO=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\option.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\option.sbr" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\netserver.c
DEP_CPP_NETSE=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\net.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\packet.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\sched.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_NETSE=\
	"..\..\..\src\strcasecmp.h"\
	"..\..\..\src\winNet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\netserver.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\netserver.sbr" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\net.c
DEP_CPP_NET_C=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\net.h"\
	"..\..\..\src\packet.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winClient.h"\
	"..\xpilot\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\math.c
DEP_CPP_MATH_=\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winClient.h"\
	"..\xpilot\winX.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\map.c
DEP_CPP_MAP_C=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_MAP_C=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\frame.c
DEP_CPP_FRAME=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_FRAME=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\frame.sbr" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\event.c
DEP_CPP_EVENT=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_EVENT=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\event.sbr" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\error.c
DEP_CPP_ERROR=\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winClient.h"\
	"..\xpilot\winX.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\collision.c

!IF  "$(CFG)" == "xpilots - Win32 Release"

DEP_CPP_COLLI=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_COLLI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\collision.obj" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\collision.sbr" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

DEP_CPP_COLLI=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\collision.obj" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\collision.sbr" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\cmdline.c
DEP_CPP_CMDLI=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\defaults.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_CMDLI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\cmdline.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\cmdline.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\xpilot\wsockerrs.cpp

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfigDlg.cpp
DEP_CPP_CONFI=\
	".\ConfBounce.h"\
	".\ConfGeneral.h"\
	".\ConfigDialog.h"\
	".\ConfigDlg.h"\
	".\ConfItems.h"\
	".\ConfPlayers.h"\
	".\ConfRobots.h"\
	".\ConfShipsShots.h"\
	".\ConfTeams.h"\
	".\ConfWorld.h"\
	".\StdAfx.h"\
	".\WinSvrThread.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfigDlg.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"

"$(INTDIR)\ConfigDlg.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ExitXpilots.cpp
DEP_CPP_EXITX=\
	".\ExitXpilots.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ExitXpilots.obj" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"

"$(INTDIR)\ExitXpilots.sbr" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\xpilot\winNet.c
DEP_CPP_WINNE=\
	"..\..\..\src\error.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\metaserver.c
DEP_CPP_METAS=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\metaserver.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_METAS=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\metaserver.obj" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\metaserver.sbr" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\objpos.c
DEP_CPP_OBJPO=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\objpos.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	
NODEP_CPP_OBJPO=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\objpos.obj" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\objpos.sbr" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\contact.c
DEP_CPP_CONTA=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\net.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\sched.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_CONTA=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\contact.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\contact.sbr" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\sched.c
DEP_CPP_SCHED=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\portability.h"\
	"..\..\..\src\sched.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\sched.obj" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\sched.sbr" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\id.c
DEP_CPP_ID_C3a=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_ID_C3a=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C3a) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\id.sbr" : $(SOURCE) $(DEP_CPP_ID_C3a) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winSvrThread.c
DEP_CPP_WINSV=\
	"..\..\..\src\sched.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	

"$(INTDIR)\winSvrThread.obj" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"

"$(INTDIR)\winSvrThread.sbr" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\robotdef.c

!IF  "$(CFG)" == "xpilots - Win32 Release"

DEP_CPP_ROBOTD=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\click.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\robot.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	"..\xpilot\winNet.h"\
	".\winServer.h"\
	
NODEP_CPP_ROBOTD=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\robotdef.obj" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\robotdef.sbr" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"

DEP_CPP_ROBOTD=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\global.h"\
	"..\..\..\src\netserver.h"\
	"..\..\..\src\object.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\proto.h"\
	"..\..\..\src\robot.h"\
	"..\..\..\src\saudio.h"\
	"..\..\..\src\score.h"\
	"..\..\..\src\version.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\robotdef.obj" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\robotdef.sbr" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ReallyShutdown.cpp
DEP_CPP_REALL=\
	".\ReallyShutdown.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ReallyShutdown.obj" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"

"$(INTDIR)\ReallyShutdown.sbr" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfigDialog.cpp
DEP_CPP_CONFIG=\
	".\ConfBounce.h"\
	".\ConfGeneral.h"\
	".\ConfigDialog.h"\
	".\ConfItems.h"\
	".\ConfPlayers.h"\
	".\ConfRobots.h"\
	".\ConfShipsShots.h"\
	".\ConfTeams.h"\
	".\ConfWorld.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfigDialog.obj" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"

"$(INTDIR)\ConfigDialog.sbr" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfWorld.cpp
DEP_CPP_CONFW=\
	".\ConfWorld.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfWorld.obj" : $(SOURCE) $(DEP_CPP_CONFW) "$(INTDIR)"

"$(INTDIR)\ConfWorld.sbr" : $(SOURCE) $(DEP_CPP_CONFW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfGeneral.cpp
DEP_CPP_CONFG=\
	".\ConfGeneral.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfGeneral.obj" : $(SOURCE) $(DEP_CPP_CONFG) "$(INTDIR)"

"$(INTDIR)\ConfGeneral.sbr" : $(SOURCE) $(DEP_CPP_CONFG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfItems.cpp
DEP_CPP_CONFIT=\
	"..\..\..\src\defaults.h"\
	".\ConfItems.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfItems.obj" : $(SOURCE) $(DEP_CPP_CONFIT) "$(INTDIR)"

"$(INTDIR)\ConfItems.sbr" : $(SOURCE) $(DEP_CPP_CONFIT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfPlayers.cpp
DEP_CPP_CONFP=\
	".\ConfPlayers.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfPlayers.obj" : $(SOURCE) $(DEP_CPP_CONFP) "$(INTDIR)"

"$(INTDIR)\ConfPlayers.sbr" : $(SOURCE) $(DEP_CPP_CONFP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfRobots.cpp
DEP_CPP_CONFR=\
	".\ConfRobots.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfRobots.obj" : $(SOURCE) $(DEP_CPP_CONFR) "$(INTDIR)"

"$(INTDIR)\ConfRobots.sbr" : $(SOURCE) $(DEP_CPP_CONFR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfShipsShots.cpp
DEP_CPP_CONFS=\
	".\ConfShipsShots.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfShipsShots.obj" : $(SOURCE) $(DEP_CPP_CONFS) "$(INTDIR)"

"$(INTDIR)\ConfShipsShots.sbr" : $(SOURCE) $(DEP_CPP_CONFS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfTeams.cpp
DEP_CPP_CONFT=\
	".\ConfTeams.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfTeams.obj" : $(SOURCE) $(DEP_CPP_CONFT) "$(INTDIR)"

"$(INTDIR)\ConfTeams.sbr" : $(SOURCE) $(DEP_CPP_CONFT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfBounce.cpp
DEP_CPP_CONFB=\
	".\ConfBounce.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfBounce.obj" : $(SOURCE) $(DEP_CPP_CONFB) "$(INTDIR)"

"$(INTDIR)\ConfBounce.sbr" : $(SOURCE) $(DEP_CPP_CONFB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\portability.c
DEP_CPP_PORTA=\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\portability.h"\
	"..\..\..\src\version.h"\
	
NODEP_CPP_PORTA=\
	"..\..\..\src\username.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
# End Project
################################################################################
