# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=xpilot - Win32 Release
!MESSAGE No configuration specified.  Defaulting to xpilot - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "xpilot - Win32 Release" && "$(CFG)" != "xpilot - Win32 Debug"\
 && "$(CFG)" != "xpilot - Win32 ReleasePentium"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilot.mak" CFG="xpilot - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xpilot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xpilot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "xpilot - Win32 ReleasePentium" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "xpilot - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "xpilot - Win32 Release"

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

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

CLEAN : 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\BetaCheck.obj"
	-@erase "$(INTDIR)\BetaCheck.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\SysInfo.obj"
	-@erase "$(INTDIR)\SysInfo.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\xpilot.exe"
	-@erase "$(OUTDIR)\xpilot.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W3 /GX /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXDLL" /D "SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/xpilot.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\BetaCheck.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\SysInfo.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /map /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/xpilot.pdb" /map:"$(INTDIR)/xpilot.map" /machine:I386\
 /out:"$(OUTDIR)/xpilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\BetaCheck.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\SysInfo.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

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

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

CLEAN : 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\BetaCheck.obj"
	-@erase "$(INTDIR)\BetaCheck.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\SysInfo.obj"
	-@erase "$(INTDIR)\SysInfo.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\xpilot.exe"
	-@erase "$(OUTDIR)\xpilot.ilk"
	-@erase "$(OUTDIR)\xpilot.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /D "_MEMPOD" /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXDLL" /D "SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /D "_MEMPOD"\
 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/xpilot.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\BetaCheck.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\SysInfo.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /map
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/xpilot.pdb" /debug /machine:I386 /out:"$(OUTDIR)/xpilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\BetaCheck.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\SysInfo.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xpilot__"
# PROP BASE Intermediate_Dir "xpilot__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "xpilot__"
# PROP Intermediate_Dir "xpilot__"
# PROP Target_Dir ""
OUTDIR=.\xpilot__
INTDIR=.\xpilot__

ALL : ".\Release\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

CLEAN : 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\BetaCheck.obj"
	-@erase "$(INTDIR)\BetaCheck.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\SysInfo.obj"
	-@erase "$(INTDIR)\SysInfo.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\XPilot.map"
	-@erase ".\Release\xpilot.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zd /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_BETAEXPIRE" /FR /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MD /W3 /GX /Zd /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_BETAEXPIRE" /D "SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /G5 /MD /W3 /GX /Zd /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_BETAEXPIRE" /D "SOUND" /D "WINDOWSCALING" /D\
 PAINT_FREE=0 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\xpilot__/
CPP_SBRS=.\xpilot__/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/xpilot.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\BetaCheck.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\SysInfo.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /map /machine:I386 /out:"Release/XPilotNT.exe"
# ADD LINK32 winmm.lib /nologo /subsystem:windows /map /machine:I386 /out:"Release/XPilot.exe"
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/XPilot.pdb" /map:"$(INTDIR)/XPilot.map" /machine:I386\
 /out:"Release/XPilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\BetaCheck.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\SysInfo.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

".\Release\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "xpilot - Win32 Release"
# Name "xpilot - Win32 Debug"
# Name "xpilot - Win32 ReleasePentium"

!IF  "$(CFG)" == "xpilot - Win32 Release"

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=..\..\..\src\client.c
DEP_CPP_CLIEN=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_CLIEN=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\client.obj" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\client.obj" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\client.obj" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\xinit.c
DEP_CPP_XINIT=\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\configure.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\dbuff.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\icon.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\items\itemAfterburner.xbm"\
	"..\..\..\src\items\itemAutopilot.xbm"\
	"..\..\..\src\items\itemCloakingDevice.xbm"\
	"..\..\..\src\items\itemEcm.xbm"\
	"..\..\..\src\items\itemEmergencyShield.xbm"\
	"..\..\..\src\items\itemEmergencyThrust.xbm"\
	"..\..\..\src\items\itemEnergyPack.xbm"\
	"..\..\..\src\items\itemLaser.xbm"\
	"..\..\..\src\items\itemMinePack.xbm"\
	"..\..\..\src\items\itemRearShot.xbm"\
	"..\..\..\src\items\itemRocketPack.xbm"\
	"..\..\..\src\items\itemSensorPack.xbm"\
	"..\..\..\src\items\itemTank.xbm"\
	"..\..\..\src\items\itemTractorBeam.xbm"\
	"..\..\..\src\items\itemTransporter.xbm"\
	"..\..\..\src\items\itemWideangleShot.xbm"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_XINIT=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xinit.obj" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xinit.obj" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xinit.obj" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\usleep.c
DEP_CPP_USLEE=\
	".\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\usleep.obj" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\usleep.obj" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\usleep.obj" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\texture.c
DEP_CPP_TEXTU=\
	"..\..\..\lib\textures\ball.xpm"\
	"..\..\..\lib\textures\rock4.xpm"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	"..\..\..\src\xpmread.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\socklib.c
DEP_CPP_SOCKL=\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	".\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SOCKL=\
	"..\..\..\src\termnet.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\paint.c
DEP_CPP_PAINT=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\dbuff.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\netclient.c
DEP_CPP_NETCL=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\net.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\packet.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winNet.h"\
	".\winX.h"\
	".\winXThread.h"\
	
NODEP_CPP_NETCL=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\netclient.obj" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\netclient.sbr" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\netclient.obj" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\netclient.sbr" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\netclient.obj" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\netclient.sbr" : $(SOURCE) $(DEP_CPP_NETCL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	".\winClient.h"\
	".\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	"..\xpilots\winServer.h"\
	".\winClient.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\join.c
DEP_CPP_JOIN_=\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_JOIN_=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\join.obj" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\join.obj" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\join.obj" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\error.c
DEP_CPP_ERROR=\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\version.h"\
	"..\xpilots\winServer.h"\
	".\winClient.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\default.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_DEFAU=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_DEFAU=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\default.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_DEFAU=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_DEFAU=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\default.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_DEFAU=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\bit.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_DEFAU=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\default.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilotView.cpp
DEP_CPP_XPILO=\
	".\BSString.h"\
	".\StdAfx.h"\
	".\SysInfo.h"\
	".\winClient.h"\
	".\winX.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\xpilotView.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

"$(INTDIR)\xpilotView.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\xpilotView.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

"$(INTDIR)\xpilotView.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\xpilotView.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

"$(INTDIR)\xpilotView.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilotDoc.cpp
DEP_CPP_XPILOT=\
	".\StdAfx.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\xpilotDoc.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\xpilotDoc.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\xpilotDoc.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\StdAfx.h"\
	".\winX.h"\
	".\XPilotNT.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilot.rc
DEP_RSC_XPILOT_=\
	"..\xpilots\res\xpilots.ico"\
	".\res\Splsh16.bmp"\
	".\res\Toolbar.bmp"\
	".\res\xpilot.ico"\
	".\res\xpilot.rc2"\
	".\res\xpilotDoc.ico"\
	".\res\xpnt.ico"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\xpilot.res" : $(SOURCE) $(DEP_RSC_XPILOT_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\xpilot.res" : $(SOURCE) $(DEP_RSC_XPILOT_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\xpilot.res" : $(SOURCE) $(DEP_RSC_XPILOT_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\xevent.c
DEP_CPP_XEVEN=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\packet.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xevent.obj" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xevent.obj" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xevent.obj" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TalkWindow.cpp
DEP_CPP_TALKW=\
	".\StdAfx.h"\
	".\TalkWindow.h"\
	".\XPilotNT.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\TalkWindow.obj" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"

"$(INTDIR)\TalkWindow.sbr" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\TalkWindow.obj" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"

"$(INTDIR)\TalkWindow.sbr" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\TalkWindow.obj" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"

"$(INTDIR)\TalkWindow.sbr" : $(SOURCE) $(DEP_CPP_TALKW) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BetaCheck.cpp
DEP_CPP_BETAC=\
	".\BetaCheck.h"\
	".\StdAfx.h"\
	".\XPilotNT.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\BetaCheck.obj" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"

"$(INTDIR)\BetaCheck.sbr" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\BetaCheck.obj" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"

"$(INTDIR)\BetaCheck.sbr" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\BetaCheck.obj" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"

"$(INTDIR)\BetaCheck.sbr" : $(SOURCE) $(DEP_CPP_BETAC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\XPilotNT.cpp
DEP_CPP_XPILOTN=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\StdAfx.h"\
	".\TalkWindow.h"\
	".\winAbout.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\XPilotNT.obj" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"

"$(INTDIR)\XPilotNT.sbr" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\XPilotNT.obj" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"

"$(INTDIR)\XPilotNT.sbr" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\XPilotNT.obj" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"

"$(INTDIR)\XPilotNT.sbr" : $(SOURCE) $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Splash.cpp
DEP_CPP_SPLAS=\
	"..\..\..\src\version.h"\
	".\Splash.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\Splash.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"

"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\Splash.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"

"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\Splash.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"

"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SysInfo.cpp
DEP_CPP_SYSIN=\
	".\StdAfx.h"\
	".\SysInfo.h"\
	".\XPilotNT.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\SysInfo.obj" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"

"$(INTDIR)\SysInfo.sbr" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\SysInfo.obj" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"

"$(INTDIR)\SysInfo.sbr" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\SysInfo.obj" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"

"$(INTDIR)\SysInfo.sbr" : $(SOURCE) $(DEP_CPP_SYSIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BSString.cpp
DEP_CPP_BSSTR=\
	".\BSString.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\BSString.obj" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"

"$(INTDIR)\BSString.sbr" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\BSString.obj" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"

"$(INTDIR)\BSString.sbr" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\BSString.obj" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"

"$(INTDIR)\BSString.sbr" : $(SOURCE) $(DEP_CPP_BSSTR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\xpilot.c
DEP_CPP_XPILOT_C=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\cmw.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\net.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\portability.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	".\winClient.h"\
	".\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_XPILOT_C=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xpilot.obj" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xpilot.obj" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xpilot.obj" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILOT_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winXKey.c
DEP_CPP_WINXK=\
	".\winX.h"\
	".\winX_.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winX.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINX_=\
	"..\..\..\src\client.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\error.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	".\winX_.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\..\..\src\strcasecmp.h"\
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINX_=\
	"..\..\..\src\client.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\error.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	".\winX_.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\..\..\src\strcasecmp.h"\
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINX_=\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\error.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	".\winX_.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\..\..\src\strcasecmp.h"\
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winNet.c

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winNet.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\winNet.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winNet.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\winNet.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winNet.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\winNet.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winAudio.c
DEP_CPP_WINAU=\
	".\winAudio.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winAudio.obj" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"

"$(INTDIR)\winAudio.sbr" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winAudio.obj" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"

"$(INTDIR)\winAudio.sbr" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winAudio.obj" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"

"$(INTDIR)\winAudio.sbr" : $(SOURCE) $(DEP_CPP_WINAU) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RecordDummy.c
DEP_CPP_RECOR=\
	"..\..\..\src\record.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\RecordDummy.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"

"$(INTDIR)\RecordDummy.sbr" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\RecordDummy.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"

"$(INTDIR)\RecordDummy.sbr" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\RecordDummy.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"

"$(INTDIR)\RecordDummy.sbr" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\caudio.c
DEP_CPP_CAUDI=\
	"..\..\..\src\audio.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\version.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\caudio.obj" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\caudio.obj" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\caudio.obj" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\paintradar.c
DEP_CPP_PAINTR=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintradar.obj" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintradar.sbr" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintradar.obj" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintradar.sbr" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintradar.obj" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintradar.sbr" : $(SOURCE) $(DEP_CPP_PAINTR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\painthud.c
DEP_CPP_PAINTH=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTH=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\paintmap.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTM=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTM=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintmap.obj" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTM=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTM=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintmap.obj" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTM=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintmap.obj" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\paintobjects.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTO=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTO=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintobjects.obj" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintobjects.sbr" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTO=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTO=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintobjects.obj" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintobjects.sbr" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTO=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\texture.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintobjects.obj" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintobjects.sbr" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\paintdata.c
DEP_CPP_PAINTD=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\record.h"\
	"..\..\..\src\rules.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	
NODEP_CPP_PAINTD=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintdata.obj" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintdata.sbr" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintdata.obj" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintdata.sbr" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintdata.obj" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintdata.sbr" : $(SOURCE) $(DEP_CPP_PAINTD) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\colors.c
DEP_CPP_COLOR=\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\dbuff.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	
NODEP_CPP_COLOR=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\colors.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\colors.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\colors.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\query.c
DEP_CPP_QUERY=\
	"..\..\..\src\config.h"\
	"..\..\..\src\pack.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\socklib.h"\
	"..\..\..\src\version.h"\
	".\winClient.h"\
	".\winNet.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_QUERY=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\query.obj" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\query.obj" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\query.obj" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\configure.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CONFI=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\configure.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_CONFI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\configure.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\configure.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CONFI=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\configure.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_CONFI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\configure.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\configure.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CONFI=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\configure.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\setup.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_CONFI=\
	"..\..\..\src\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\configure.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\configure.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\widget.c
DEP_CPP_WIDGE=\
	"..\..\..\src\bit.h"\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\paintdata.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winClient.h"\
	".\winX.h"\
	
NODEP_CPP_WIDGE=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\widget.obj" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\widget.obj" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\widget.obj" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winAbout.cpp
DEP_CPP_WINAB=\
	"..\..\..\src\version.h"\
	".\StdAfx.h"\
	".\winAbout.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winAbout.obj" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"

"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winAbout.obj" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"

"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winAbout.obj" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"

"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\src\about.c
DEP_CPP_ABOUT=\
	"..\..\..\src\client.h"\
	"..\..\..\src\config.h"\
	"..\..\..\src\configure.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\dbuff.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\keys.h"\
	"..\..\..\src\netclient.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	"..\..\..\src\version.h"\
	"..\..\..\src\widget.h"\
	"..\..\..\src\xinit.h"\
	".\winX.h"\
	
NODEP_CPP_ABOUT=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\about.obj" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\about.obj" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\about.obj" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winX11.c
DEP_CPP_WINX1=\
	"..\..\..\src\client.h"\
	"..\..\..\src\const.h"\
	"..\..\..\src\draw.h"\
	"..\..\..\src\error.h"\
	"..\..\..\src\item.h"\
	"..\..\..\src\paint.h"\
	"..\..\..\src\protoclient.h"\
	".\winClient.h"\
	".\winX.h"\
	".\winX_.h"\
	
NODEP_CPP_WINX1=\
	"..\..\..\src\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winXThread.c
DEP_CPP_WINXT=\
	"..\..\..\src\error.h"\
	".\winX.h"\
	".\winX_.h"\
	".\winXThread.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winXThread.obj" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"

"$(INTDIR)\winXThread.sbr" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\winXThread.obj" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"

"$(INTDIR)\winXThread.sbr" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\winXThread.obj" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"

"$(INTDIR)\winXThread.sbr" : $(SOURCE) $(DEP_CPP_WINXT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wsockerrs.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

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
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
################################################################################
# Section xpilot : {0FEA6ED2-A2FE-11CF-80DA-00A02423A46C}
# 	0:8:Splash.h:D:\users\dick\buckosof\xpilot\contrib\NT\xpilot\Splash.h
# 	0:10:Splash.cpp:D:\users\dick\buckosof\xpilot\contrib\NT\xpilot\Splash.cpp
# 	1:10:IDB_SPLASH:102
# 	2:10:ResHdrName:resource.h
# 	2:11:ProjHdrName:stdafx.h
# 	2:10:WrapperDef:_SPLASH_SCRN_
# 	2:12:SplClassName:CSplashWnd
# 	2:21:SplashScreenInsertKey:4.0
# 	2:10:HeaderName:Splash.h
# 	2:10:ImplemName:Splash.cpp
# 	2:7:BmpID16:IDB_SPLASH
# End Section
################################################################################
