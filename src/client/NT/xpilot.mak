# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
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
# PROP Target_Last_Scanned "xpilot - Win32 ReleasePentium"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

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
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx3d.obj"
	-@erase "$(INTDIR)\gfx3d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
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
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
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
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
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

"$(OUTDIR)" :

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)

.c{$(CPP_OBJS)}.obj:

.c{$(CPP_SBRS)}.sbr:

.cpp{$(CPP_OBJS)}.obj:

.cpp{$(CPP_SBRS)}.sbr:

.cxx{$(CPP_OBJS)}.obj:

.cxx{$(CPP_SBRS)}.sbr:

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /I "..\..\common" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W3 /GX /I "..\..\common" /D "NDEBUG" /D "_MBCS" /D\
 "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING" /D\
 PAINT_FREE=0 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /c 
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
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx3d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
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
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
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
	"$(INTDIR)\xeventhandlers.sbr" \
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
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx3d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
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
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
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
	"$(INTDIR)\xeventhandlers.obj" \
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
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx3d.obj"
	-@erase "$(INTDIR)\gfx3d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
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
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
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
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
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

"$(OUTDIR)" :

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)

.c{$(CPP_OBJS)}.obj:

.c{$(CPP_SBRS)}.sbr:

.cpp{$(CPP_OBJS)}.obj:

.cpp{$(CPP_SBRS)}.sbr:

.cxx{$(CPP_OBJS)}.obj:

.cxx{$(CPP_SBRS)}.sbr:

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\common" /D "_DEBUG" /D "_MEMPOD" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\common" /D "_DEBUG" /D\
 "_MEMPOD" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING"\
 /D PAINT_FREE=0 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
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
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx3d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
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
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
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
	"$(INTDIR)\xeventhandlers.sbr" \
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
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx3d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
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
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
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
	"$(INTDIR)\xeventhandlers.obj" \
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
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx3d.obj"
	-@erase "$(INTDIR)\gfx3d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
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
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
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
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
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

"$(OUTDIR)" :

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)

".\Release\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)

.c{$(CPP_OBJS)}.obj:

.c{$(CPP_SBRS)}.sbr:

.cpp{$(CPP_OBJS)}.obj:

.cpp{$(CPP_SBRS)}.sbr:

.cxx{$(CPP_OBJS)}.obj:

.cxx{$(CPP_SBRS)}.sbr:

# ADD BASE CPP /nologo /MD /W3 /GX /Zd /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "x_BETAEXPIRE" /FR /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G5 /MD /W3 /GX /Zd /O2 /I "..\..\common" /D "NDEBUG" /D "_MBCS" /D "x_BETAEXPIRE" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING" /D PAINT_FREE=0 /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /G5 /MD /W3 /GX /Zd /O2 /I "..\..\common" /D "NDEBUG" /D\
 "_MBCS" /D "x_BETAEXPIRE" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D\
 "WINDOWSCALING" /D PAINT_FREE=0 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /c 
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
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx3d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
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
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
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
	"$(INTDIR)\xeventhandlers.sbr" \
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
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx3d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
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
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
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
	"$(INTDIR)\xeventhandlers.obj" \
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

SOURCE=.\xpilotView.cpp
DEP_CPP_XPILO=\
	"..\..\common\NT/winX.h"\
	".\BSString.h"\
	".\stdafx.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
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
	".\stdafx.h"\
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
	"..\..\common\NT/winX.h"\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\winXXPilot.h"\
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
	"..\..\server\NT\res\xpilots.ico"\
	".\res\Splsh16.bmp"\
	".\res\xpilot.ico"\
	".\res\xpilot.rc2"\
	

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

SOURCE=.\TalkWindow.cpp
DEP_CPP_TALKW=\
	".\stdafx.h"\
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

SOURCE=.\XPilotNT.cpp
DEP_CPP_XPILOTN=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
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
	"..\..\common\version.h"\
	".\Splash.h"\
	".\stdafx.h"\
	

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

SOURCE=.\BSString.cpp
DEP_CPP_BSSTR=\
	".\BSString.h"\
	".\stdafx.h"\
	

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
	"..\..\common\NT/winX.h"\
	"..\record.h"\
	

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

SOURCE=.\winAbout.cpp
DEP_CPP_WINAB=\
	"..\..\common\version.h"\
	".\stdafx.h"\
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

SOURCE=..\about.c
DEP_CPP_ABOUT=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_ABOUT=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\caudio.c
DEP_CPP_CAUDI=\
	"..\..\common\audio.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	

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

SOURCE=..\client.c
DEP_CPP_CLIEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CLIEN=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\colors.c
DEP_CPP_COLOR=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\xinit.h"\
	
NODEP_CPP_COLOR=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\configure.c
DEP_CPP_CONFI=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_CONFI=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\configure.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\configure.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\configure.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\configure.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


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

SOURCE=..\default.c
DEP_CPP_DEFAU=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_DEFAU=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\default.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\default.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


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

SOURCE=..\..\common\error.c
DEP_CPP_ERROR=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

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

SOURCE=..\join.c
DEP_CPP_JOIN_=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	
NODEP_CPP_JOIN_=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\..\common\math.c
DEP_CPP_MATH_=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\version.h"\
	

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

SOURCE=..\..\common\net.c
DEP_CPP_NET_C=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\packet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

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

SOURCE=..\netclient.c
DEP_CPP_NETCL=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_NETCL=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\painthud.c
DEP_CPP_PAINT=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINT=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\painthud.obj" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\paintdata.c
DEP_CPP_PAINTD=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTD=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\paint.c
DEP_CPP_PAINT_=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paint.obj" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\paintradar.c
DEP_CPP_PAINTR=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

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

SOURCE=..\paintobjects.c
DEP_CPP_PAINTO=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\gfx3d.h"\
	"..\guiobjects.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINTO=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintobjects.obj" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintobjects.sbr" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintobjects.obj" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintobjects.sbr" : $(SOURCE) $(DEP_CPP_PAINTO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


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

SOURCE=..\paintmap.c
DEP_CPP_PAINTM=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\guimap.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTM=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintmap.obj" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\paintmap.obj" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


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

SOURCE=..\..\common\portability.c
DEP_CPP_PORTA=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	
NODEP_CPP_PORTA=\
	"..\..\common\username.h"\
	

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
################################################################################
# Begin Source File

SOURCE=..\query.c
DEP_CPP_QUERY=\
	"..\..\common\config.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_QUERY=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\..\common\socklib.c
DEP_CPP_SOCKL=\
	"..\..\common\cmw.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_SOCKL=\
	"..\..\common\termnet.h"\
	

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

SOURCE=..\usleep.c
DEP_CPP_USLEE=\
	"..\..\common\NT/winNet.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

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

SOURCE=..\texture.c
DEP_CPP_TEXTU=\
	"..\..\..\lib\textures\ball.xpm"\
	"..\..\..\lib\textures\rock4.xpm"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\texture.h"\
	"..\xinit.h"\
	"..\xpmread.h"\
	

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

SOURCE=..\widget.c
DEP_CPP_WIDGE=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_WIDGE=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\..\common\NT\winNet.c
DEP_CPP_WINNE=\
	"..\..\common\error.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\winX.c
DEP_CPP_WINX_=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\error.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT\winX_.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX.obj" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\winX11.c
DEP_CPP_WINX1=\
	"..\..\common\error.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT\winX_.h"\
	".\winClient.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winX11.obj" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\winXKey.c
DEP_CPP_WINXK=\
	"..\..\common\NT/winX.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winXKey.obj" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winXThread.c
DEP_CPP_WINXT=\
	"..\..\common\error.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT\winX_.h"\
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

SOURCE=..\..\common\NT\wsockerrs.c

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\xinit.c
DEP_CPP_XINIT=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\icon.h"\
	"..\items\itemAfterburner.xbm"\
	"..\items\itemArmor.xbm"\
	"..\items\itemAutopilot.xbm"\
	"..\items\itemCloakingDevice.xbm"\
	"..\items\itemDeflector.xbm"\
	"..\items\itemEcm.xbm"\
	"..\items\itemEmergencyShield.xbm"\
	"..\items\itemEmergencyThrust.xbm"\
	"..\items\itemEnergyPack.xbm"\
	"..\items\itemHyperJump.xbm"\
	"..\items\itemLaser.xbm"\
	"..\items\itemMinePack.xbm"\
	"..\items\itemMirror.xbm"\
	"..\items\itemPhasingDevice.xbm"\
	"..\items\itemRearShot.xbm"\
	"..\items\itemRocketPack.xbm"\
	"..\items\itemSensorPack.xbm"\
	"..\items\itemTank.xbm"\
	"..\items\itemTractorBeam.xbm"\
	"..\items\itemTransporter.xbm"\
	"..\items\itemWideangleShot.xbm"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_XINIT=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\xevent.c
DEP_CPP_XEVEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT/winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xeventhandlers.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

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

SOURCE=..\xpilot.c
DEP_CPP_XPILOT_C=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_XPILOT_C=\
	"..\strcasecmp.h"\
	

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

SOURCE=..\..\common\shipshape.c
DEP_CPP_SHIPS=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\checknames.c
DEP_CPP_CHECK=\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\guimap.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_GUIMA=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guimap.obj" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_GUIMA=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guimap.obj" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_GUIMA=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guimap.obj" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\syslimit.c

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\syslimit.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\syslimit.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\syslimit.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\syslimit.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\syslimit.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\syslimit.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\xeventhandlers.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT/winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xeventhandlers.obj" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT/winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xeventhandlers.obj" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\NT/winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\xeventhandlers.obj" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE) $(DEP_CPP_XEVENT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\datagram.c
DEP_CPP_DATAG=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_DATAG=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\datagram.obj" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\datagram.obj" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\datagram.obj" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\textinterface.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_TEXTI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\textinterface.obj" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\textinterface.sbr" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_TEXTI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\textinterface.obj" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\textinterface.sbr" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_TEXTI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\textinterface.obj" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\textinterface.sbr" : $(SOURCE) $(DEP_CPP_TEXTI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\config.c
DEP_CPP_CONFIG=\
	"..\..\common\config.h"\
	"..\..\common\version.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\config.obj" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\config.obj" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\config.obj" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFIG) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\gfx3d.c
DEP_CPP_GFX3D=\
	"..\..\common\const.h"\
	"..\gfx3d.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\gfx3d.obj" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gfx3d.sbr" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\gfx3d.obj" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gfx3d.sbr" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\gfx3d.obj" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gfx3d.sbr" : $(SOURCE) $(DEP_CPP_GFX3D) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\guiobjects.c
DEP_CPP_GUIOB=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_GUIOB=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guiobjects.obj" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guiobjects.sbr" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guiobjects.obj" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guiobjects.sbr" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\guiobjects.obj" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\guiobjects.sbr" : $(SOURCE) $(DEP_CPP_GUIOB) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\talkmacros.c
DEP_CPP_TALKM=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\NT/winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_TALKM=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilot - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\talkmacros.obj" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\talkmacros.sbr" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\talkmacros.obj" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\talkmacros.sbr" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\talkmacros.obj" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\talkmacros.sbr" : $(SOURCE) $(DEP_CPP_TALKM) "$(INTDIR)"
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
