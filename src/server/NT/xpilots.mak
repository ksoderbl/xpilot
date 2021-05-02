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

ALL : "$(OUTDIR)\XPilot-server.exe" "$(OUTDIR)\xpilots.bsc"

CLEAN : 
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\ConfigDlg.obj"
	-@erase "$(INTDIR)\ConfigDlg.sbr"
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
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
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
	-@erase "$(OUTDIR)\XPilot-server.exe"
	-@erase "$(OUTDIR)\xpilots.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /Fr /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W3 /GX /I "..\..\common" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /c 
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
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
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
	"$(INTDIR)\shipshape.sbr" \
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
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"Release/XPilot-server.exe"
# SUBTRACT LINK32 /debug /nodefaultlib
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/XPilot-server.pdb" /machine:I386\
 /out:"$(OUTDIR)/XPilot-server.exe" 
LINK32_OBJS= \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
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
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\update.obj" \
	"$(INTDIR)\walls.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winSvrThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xpilots.obj" \
	"$(INTDIR)\xpilots.res" \
	"$(INTDIR)\xpilotsDlg.obj"

"$(OUTDIR)\XPilot-server.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\ConfigDlg.obj"
	-@erase "$(INTDIR)\ConfigDlg.sbr"
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
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
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
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /Gy /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /Gy /I "..\..\common" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS"\
 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
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
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
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
	"$(INTDIR)\shipshape.sbr" \
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
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
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
	"$(INTDIR)\shipshape.obj" \
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
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	".\ConfigDlg.h"\
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

SOURCE=.\winSvrThread.c
DEP_CPP_WINSV=\
	"..\..\common\config.h"\
	"..\..\common\NT/winNet.h"\
	"..\sched.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	

"$(INTDIR)\winSvrThread.obj" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"

"$(INTDIR)\winSvrThread.sbr" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"


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

SOURCE=..\cmdline.c
DEP_CPP_CMDLI=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\defaults.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	".\winServer.h"\
	
NODEP_CPP_CMDLI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\cmdline.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\cmdline.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\collision.c
DEP_CPP_COLLI=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_COLLI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\collision.obj" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\collision.sbr" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\contact.c
DEP_CPP_CONTA=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\sched.h"\
	"..\server.h"\
	".\winServer.h"\
	
NODEP_CPP_CONTA=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\contact.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\contact.sbr" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\error.c
DEP_CPP_ERROR=\
	"..\..\client\NT\winClient.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
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

SOURCE=..\event.c
DEP_CPP_EVENT=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_EVENT=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\event.sbr" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\frame.c
DEP_CPP_FRAME=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\saudio.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_FRAME=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\frame.sbr" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\id.c
DEP_CPP_ID_C18=\
	"..\..\common\bit.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	".\winServer.h"\
	
NODEP_CPP_ID_C18=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\id.sbr" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\map.c
DEP_CPP_MAP_C=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_MAP_C=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\math.c
DEP_CPP_MATH_=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\version.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\metaserver.c
DEP_CPP_METAS=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\metaserver.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\saudio.h"\
	".\winServer.h"\
	
NODEP_CPP_METAS=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\metaserver.obj" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\metaserver.sbr" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\net.c
DEP_CPP_NET_C=\
	"..\..\client\NT\winClient.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\packet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
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

SOURCE=..\netserver.c
DEP_CPP_NETSE=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\net.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\packet.h"\
	"..\..\common\setup.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\sched.h"\
	"..\server.h"\
	".\winServer.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_NETSE=\
	"..\strcasecmp.h"\
	"..\winNet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\netserver.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\netserver.sbr" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\objpos.c
DEP_CPP_OBJPO=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	
NODEP_CPP_OBJPO=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\objpos.obj" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\objpos.sbr" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\option.c
DEP_CPP_OPTIO=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\defaults.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	
NODEP_CPP_OPTIO=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\option.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\option.sbr" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\play.c
DEP_CPP_PLAY_=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_PLAY_=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\play.obj" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\play.sbr" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\player.c
DEP_CPP_PLAYE=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_PLAYE=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\player.obj" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\player.sbr" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

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
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\robot.c
DEP_CPP_ROBOT=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\robot.h"\
	"..\saudio.h"\
	"..\score.h"\
	"..\server.h"\
	".\winServer.h"\
	
NODEP_CPP_ROBOT=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\robot.obj" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\robot.sbr" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\robotdef.c
DEP_CPP_ROBOTD=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\robot.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_ROBOTD=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\robotdef.obj" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\robotdef.sbr" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\rules.c
DEP_CPP_RULES=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\rules.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	
NODEP_CPP_RULES=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\rules.obj" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\rules.sbr" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\saudio.c
DEP_CPP_SAUDI=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\saudio.h"\
	
NODEP_CPP_SAUDI=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\saudio.obj" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\saudio.sbr" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\sched.c
DEP_CPP_SCHED=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\sched.h"\
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

SOURCE=..\server.c
DEP_CPP_SERVE=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\proto.h"\
	"..\sched.h"\
	"..\server.h"\
	".\winServer.h"\
	".\WinSvrThread.h"\
	
NODEP_CPP_SERVE=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\server.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\server.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\socklib.c
DEP_CPP_SOCKL=\
	"..\..\common\cmw.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SOCKL=\
	"..\..\common\termnet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\update.c
DEP_CPP_UPDAT=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	".\winServer.h"\
	
NODEP_CPP_UPDAT=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\update.sbr" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\walls.c
DEP_CPP_WALLS=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\click.h"\
	"..\global.h"\
	"..\netserver.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	"..\walls.h"\
	".\winServer.h"\
	
NODEP_CPP_WALLS=\
	"..\strcasecmp.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\walls.obj" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\walls.sbr" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\wsockerrs.c

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\winNet.c
DEP_CPP_WINNE=\
	"..\..\common\error.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

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
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\shipshape.c
DEP_CPP_SHIPS=\
	"..\..\client\NT\winClient.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	".\winServer.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfigDlg.cpp
DEP_CPP_CONFI=\
	".\ConfigDlg.h"\
	".\StdAfx.h"\
	".\WinSvrThread.h"\
	".\xpilots.h"\
	

"$(INTDIR)\ConfigDlg.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"

"$(INTDIR)\ConfigDlg.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
