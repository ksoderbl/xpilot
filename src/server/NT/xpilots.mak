# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
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
CPP=cl.exe
MTL=mktyplib.exe
RSC=rc.exe

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

ALL : ".\Release\XPilot-server.exe" ".\Release\xpilots.bsc"

CLEAN : 
	-@erase ".\Release\cannon.obj"
	-@erase ".\Release\cannon.sbr"
	-@erase ".\Release\checknames.obj"
	-@erase ".\Release\checknames.sbr"
	-@erase ".\Release\cmdline.obj"
	-@erase ".\Release\cmdline.sbr"
	-@erase ".\Release\collision.obj"
	-@erase ".\Release\collision.sbr"
	-@erase ".\Release\ConfigDlg.obj"
	-@erase ".\Release\ConfigDlg.sbr"
	-@erase ".\Release\contact.obj"
	-@erase ".\Release\contact.sbr"
	-@erase ".\Release\error.obj"
	-@erase ".\Release\error.sbr"
	-@erase ".\Release\event.obj"
	-@erase ".\Release\event.sbr"
	-@erase ".\Release\ExitXpilots.obj"
	-@erase ".\Release\ExitXpilots.sbr"
	-@erase ".\Release\frame.obj"
	-@erase ".\Release\frame.sbr"
	-@erase ".\Release\id.obj"
	-@erase ".\Release\id.sbr"
	-@erase ".\Release\map.obj"
	-@erase ".\Release\map.sbr"
	-@erase ".\Release\math.obj"
	-@erase ".\Release\math.sbr"
	-@erase ".\Release\metaserver.obj"
	-@erase ".\Release\metaserver.sbr"
	-@erase ".\Release\net.obj"
	-@erase ".\Release\net.sbr"
	-@erase ".\Release\netserver.obj"
	-@erase ".\Release\netserver.sbr"
	-@erase ".\Release\objpos.obj"
	-@erase ".\Release\objpos.sbr"
	-@erase ".\Release\option.obj"
	-@erase ".\Release\option.sbr"
	-@erase ".\Release\play.obj"
	-@erase ".\Release\play.sbr"
	-@erase ".\Release\player.obj"
	-@erase ".\Release\player.sbr"
	-@erase ".\Release\portability.obj"
	-@erase ".\Release\portability.sbr"
	-@erase ".\Release\ReallyShutdown.obj"
	-@erase ".\Release\ReallyShutdown.sbr"
	-@erase ".\Release\robot.obj"
	-@erase ".\Release\robot.sbr"
	-@erase ".\Release\robotdef.obj"
	-@erase ".\Release\robotdef.sbr"
	-@erase ".\Release\rules.obj"
	-@erase ".\Release\rules.sbr"
	-@erase ".\Release\saudio.obj"
	-@erase ".\Release\saudio.sbr"
	-@erase ".\Release\sched.obj"
	-@erase ".\Release\sched.sbr"
	-@erase ".\Release\server.obj"
	-@erase ".\Release\server.sbr"
	-@erase ".\Release\shipshape.obj"
	-@erase ".\Release\shipshape.sbr"
	-@erase ".\Release\socklib.obj"
	-@erase ".\Release\socklib.sbr"
	-@erase ".\Release\update.obj"
	-@erase ".\Release\update.sbr"
	-@erase ".\Release\walls.obj"
	-@erase ".\Release\walls.sbr"
	-@erase ".\Release\winNet.obj"
	-@erase ".\Release\winNet.sbr"
	-@erase ".\Release\winSvrThread.obj"
	-@erase ".\Release\winSvrThread.sbr"
	-@erase ".\Release\wsockerrs.obj"
	-@erase ".\Release\wsockerrs.sbr"
	-@erase ".\Release\XPilot-server.exe"
	-@erase ".\Release\xpilots.bsc"
	-@erase ".\Release\xpilots.obj"
	-@erase ".\Release\xpilots.res"
	-@erase ".\Release\xpilots.sbr"
	-@erase ".\Release\xpilotsDlg.obj"
	-@erase ".\Release\xpilotsDlg.sbr"

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
	".\Release\cannon.sbr" \
	".\Release\checknames.sbr" \
	".\Release\cmdline.sbr" \
	".\Release\collision.sbr" \
	".\Release\ConfigDlg.sbr" \
	".\Release\contact.sbr" \
	".\Release\error.sbr" \
	".\Release\event.sbr" \
	".\Release\ExitXpilots.sbr" \
	".\Release\frame.sbr" \
	".\Release\id.sbr" \
	".\Release\map.sbr" \
	".\Release\math.sbr" \
	".\Release\metaserver.sbr" \
	".\Release\net.sbr" \
	".\Release\netserver.sbr" \
	".\Release\objpos.sbr" \
	".\Release\option.sbr" \
	".\Release\play.sbr" \
	".\Release\player.sbr" \
	".\Release\portability.sbr" \
	".\Release\ReallyShutdown.sbr" \
	".\Release\robot.sbr" \
	".\Release\robotdef.sbr" \
	".\Release\rules.sbr" \
	".\Release\saudio.sbr" \
	".\Release\sched.sbr" \
	".\Release\server.sbr" \
	".\Release\shipshape.sbr" \
	".\Release\socklib.sbr" \
	".\Release\update.sbr" \
	".\Release\walls.sbr" \
	".\Release\winNet.sbr" \
	".\Release\winSvrThread.sbr" \
	".\Release\wsockerrs.sbr" \
	".\Release\xpilots.sbr" \
	".\Release\xpilotsDlg.sbr"

".\Release\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
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
	".\Release\cannon.obj" \
	".\Release\checknames.obj" \
	".\Release\cmdline.obj" \
	".\Release\collision.obj" \
	".\Release\ConfigDlg.obj" \
	".\Release\contact.obj" \
	".\Release\error.obj" \
	".\Release\event.obj" \
	".\Release\ExitXpilots.obj" \
	".\Release\frame.obj" \
	".\Release\id.obj" \
	".\Release\map.obj" \
	".\Release\math.obj" \
	".\Release\metaserver.obj" \
	".\Release\net.obj" \
	".\Release\netserver.obj" \
	".\Release\objpos.obj" \
	".\Release\option.obj" \
	".\Release\play.obj" \
	".\Release\player.obj" \
	".\Release\portability.obj" \
	".\Release\ReallyShutdown.obj" \
	".\Release\robot.obj" \
	".\Release\robotdef.obj" \
	".\Release\rules.obj" \
	".\Release\saudio.obj" \
	".\Release\sched.obj" \
	".\Release\server.obj" \
	".\Release\shipshape.obj" \
	".\Release\socklib.obj" \
	".\Release\update.obj" \
	".\Release\walls.obj" \
	".\Release\winNet.obj" \
	".\Release\winSvrThread.obj" \
	".\Release\wsockerrs.obj" \
	".\Release\xpilots.obj" \
	".\Release\xpilots.res" \
	".\Release\xpilotsDlg.obj"

".\Release\XPilot-server.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

ALL : ".\Debug\xpilots.exe" ".\Debug\xpilots.bsc"

CLEAN : 
	-@erase ".\Debug\cannon.obj"
	-@erase ".\Debug\cannon.sbr"
	-@erase ".\Debug\checknames.obj"
	-@erase ".\Debug\checknames.sbr"
	-@erase ".\Debug\cmdline.obj"
	-@erase ".\Debug\cmdline.sbr"
	-@erase ".\Debug\collision.obj"
	-@erase ".\Debug\collision.sbr"
	-@erase ".\Debug\ConfigDlg.obj"
	-@erase ".\Debug\ConfigDlg.sbr"
	-@erase ".\Debug\contact.obj"
	-@erase ".\Debug\contact.sbr"
	-@erase ".\Debug\error.obj"
	-@erase ".\Debug\error.sbr"
	-@erase ".\Debug\event.obj"
	-@erase ".\Debug\event.sbr"
	-@erase ".\Debug\ExitXpilots.obj"
	-@erase ".\Debug\ExitXpilots.sbr"
	-@erase ".\Debug\frame.obj"
	-@erase ".\Debug\frame.sbr"
	-@erase ".\Debug\id.obj"
	-@erase ".\Debug\id.sbr"
	-@erase ".\Debug\map.obj"
	-@erase ".\Debug\map.sbr"
	-@erase ".\Debug\math.obj"
	-@erase ".\Debug\math.sbr"
	-@erase ".\Debug\metaserver.obj"
	-@erase ".\Debug\metaserver.sbr"
	-@erase ".\Debug\net.obj"
	-@erase ".\Debug\net.sbr"
	-@erase ".\Debug\netserver.obj"
	-@erase ".\Debug\netserver.sbr"
	-@erase ".\Debug\objpos.obj"
	-@erase ".\Debug\objpos.sbr"
	-@erase ".\Debug\option.obj"
	-@erase ".\Debug\option.sbr"
	-@erase ".\Debug\play.obj"
	-@erase ".\Debug\play.sbr"
	-@erase ".\Debug\player.obj"
	-@erase ".\Debug\player.sbr"
	-@erase ".\Debug\portability.obj"
	-@erase ".\Debug\portability.sbr"
	-@erase ".\Debug\ReallyShutdown.obj"
	-@erase ".\Debug\ReallyShutdown.sbr"
	-@erase ".\Debug\robot.obj"
	-@erase ".\Debug\robot.sbr"
	-@erase ".\Debug\robotdef.obj"
	-@erase ".\Debug\robotdef.sbr"
	-@erase ".\Debug\rules.obj"
	-@erase ".\Debug\rules.sbr"
	-@erase ".\Debug\saudio.obj"
	-@erase ".\Debug\saudio.sbr"
	-@erase ".\Debug\sched.obj"
	-@erase ".\Debug\sched.sbr"
	-@erase ".\Debug\server.obj"
	-@erase ".\Debug\server.sbr"
	-@erase ".\Debug\shipshape.obj"
	-@erase ".\Debug\shipshape.sbr"
	-@erase ".\Debug\socklib.obj"
	-@erase ".\Debug\socklib.sbr"
	-@erase ".\Debug\update.obj"
	-@erase ".\Debug\update.sbr"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\walls.obj"
	-@erase ".\Debug\walls.sbr"
	-@erase ".\Debug\winNet.obj"
	-@erase ".\Debug\winNet.sbr"
	-@erase ".\Debug\winSvrThread.obj"
	-@erase ".\Debug\winSvrThread.sbr"
	-@erase ".\Debug\wsockerrs.obj"
	-@erase ".\Debug\wsockerrs.sbr"
	-@erase ".\Debug\xpilots.bsc"
	-@erase ".\Debug\xpilots.exe"
	-@erase ".\Debug\xpilots.ilk"
	-@erase ".\Debug\xpilots.obj"
	-@erase ".\Debug\xpilots.pdb"
	-@erase ".\Debug\xpilots.res"
	-@erase ".\Debug\xpilots.sbr"
	-@erase ".\Debug\xpilotsDlg.obj"
	-@erase ".\Debug\xpilotsDlg.sbr"

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
	".\Debug\cannon.sbr" \
	".\Debug\checknames.sbr" \
	".\Debug\cmdline.sbr" \
	".\Debug\collision.sbr" \
	".\Debug\ConfigDlg.sbr" \
	".\Debug\contact.sbr" \
	".\Debug\error.sbr" \
	".\Debug\event.sbr" \
	".\Debug\ExitXpilots.sbr" \
	".\Debug\frame.sbr" \
	".\Debug\id.sbr" \
	".\Debug\map.sbr" \
	".\Debug\math.sbr" \
	".\Debug\metaserver.sbr" \
	".\Debug\net.sbr" \
	".\Debug\netserver.sbr" \
	".\Debug\objpos.sbr" \
	".\Debug\option.sbr" \
	".\Debug\play.sbr" \
	".\Debug\player.sbr" \
	".\Debug\portability.sbr" \
	".\Debug\ReallyShutdown.sbr" \
	".\Debug\robot.sbr" \
	".\Debug\robotdef.sbr" \
	".\Debug\rules.sbr" \
	".\Debug\saudio.sbr" \
	".\Debug\sched.sbr" \
	".\Debug\server.sbr" \
	".\Debug\shipshape.sbr" \
	".\Debug\socklib.sbr" \
	".\Debug\update.sbr" \
	".\Debug\walls.sbr" \
	".\Debug\winNet.sbr" \
	".\Debug\winSvrThread.sbr" \
	".\Debug\wsockerrs.sbr" \
	".\Debug\xpilots.sbr" \
	".\Debug\xpilotsDlg.sbr"

".\Debug\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/xpilots.pdb" /debug /machine:I386 /out:"$(OUTDIR)/xpilots.exe" 
LINK32_OBJS= \
	".\Debug\cannon.obj" \
	".\Debug\checknames.obj" \
	".\Debug\cmdline.obj" \
	".\Debug\collision.obj" \
	".\Debug\ConfigDlg.obj" \
	".\Debug\contact.obj" \
	".\Debug\error.obj" \
	".\Debug\event.obj" \
	".\Debug\ExitXpilots.obj" \
	".\Debug\frame.obj" \
	".\Debug\id.obj" \
	".\Debug\map.obj" \
	".\Debug\math.obj" \
	".\Debug\metaserver.obj" \
	".\Debug\net.obj" \
	".\Debug\netserver.obj" \
	".\Debug\objpos.obj" \
	".\Debug\option.obj" \
	".\Debug\play.obj" \
	".\Debug\player.obj" \
	".\Debug\portability.obj" \
	".\Debug\ReallyShutdown.obj" \
	".\Debug\robot.obj" \
	".\Debug\robotdef.obj" \
	".\Debug\rules.obj" \
	".\Debug\saudio.obj" \
	".\Debug\sched.obj" \
	".\Debug\server.obj" \
	".\Debug\shipshape.obj" \
	".\Debug\socklib.obj" \
	".\Debug\update.obj" \
	".\Debug\walls.obj" \
	".\Debug\winNet.obj" \
	".\Debug\winSvrThread.obj" \
	".\Debug\wsockerrs.obj" \
	".\Debug\xpilots.obj" \
	".\Debug\xpilots.res" \
	".\Debug\xpilotsDlg.obj"

".\Debug\xpilots.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\xpilots.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

".\Release\xpilots.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\xpilots.obj" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"

".\Debug\xpilots.sbr" : $(SOURCE) $(DEP_CPP_XPILO) "$(INTDIR)"


!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\xpilotsDlg.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

".\Release\xpilotsDlg.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\xpilotsDlg.obj" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"

".\Debug\xpilotsDlg.sbr" : $(SOURCE) $(DEP_CPP_XPILOT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\xpilots.rc
DEP_RSC_XPILOTS=\
	".\res\icon1.ico"\
	".\res\xpilots.ico"\
	".\res\xpilots.rc2"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\xpilots.res" : $(SOURCE) $(DEP_RSC_XPILOTS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\xpilots.res" : $(SOURCE) $(DEP_RSC_XPILOTS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ExitXpilots.cpp
DEP_CPP_EXITX=\
	".\ExitXpilots.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\ExitXpilots.obj" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"

".\Release\ExitXpilots.sbr" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\ExitXpilots.obj" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"

".\Debug\ExitXpilots.sbr" : $(SOURCE) $(DEP_CPP_EXITX) "$(INTDIR)"


!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\winSvrThread.obj" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"

".\Release\winSvrThread.sbr" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\winSvrThread.obj" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"

".\Debug\winSvrThread.sbr" : $(SOURCE) $(DEP_CPP_WINSV) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ReallyShutdown.cpp
DEP_CPP_REALL=\
	".\ReallyShutdown.h"\
	".\StdAfx.h"\
	".\xpilots.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\ReallyShutdown.obj" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"

".\Release\ReallyShutdown.sbr" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\ReallyShutdown.obj" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"

".\Debug\ReallyShutdown.sbr" : $(SOURCE) $(DEP_CPP_REALL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\cmdline.c
DEP_CPP_CMDLI=\
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
	"..\defaults.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	".\winServer.h"\
	
NODEP_CPP_CMDLI=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\cmdline.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

".\Release\cmdline.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\cmdline.obj" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

".\Debug\cmdline.sbr" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\collision.obj" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

".\Release\collision.sbr" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\collision.obj" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

".\Debug\collision.sbr" : $(SOURCE) $(DEP_CPP_COLLI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\contact.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

".\Release\contact.sbr" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\contact.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

".\Debug\contact.sbr" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

".\Release\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

".\Debug\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

".\Release\event.sbr" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

".\Debug\event.sbr" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

".\Release\frame.sbr" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

".\Debug\frame.sbr" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\id.obj" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

".\Release\id.sbr" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\id.obj" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

".\Debug\id.sbr" : $(SOURCE) $(DEP_CPP_ID_C18) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

".\Release\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(BuildCmds)

".\Debug\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

".\Release\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\math.obj" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

".\Debug\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\metaserver.obj" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

".\Release\metaserver.sbr" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\metaserver.obj" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

".\Debug\metaserver.sbr" : $(SOURCE) $(DEP_CPP_METAS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

".\Release\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\net.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

".\Debug\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\netserver.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

".\Release\netserver.sbr" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\netserver.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

".\Debug\netserver.sbr" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\objpos.obj" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

".\Release\objpos.sbr" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\objpos.obj" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

".\Debug\objpos.sbr" : $(SOURCE) $(DEP_CPP_OBJPO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\option.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

".\Release\option.sbr" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\option.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

".\Debug\option.sbr" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	"..\cannon.h"\
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\play.obj" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

".\Release\play.sbr" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\play.obj" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

".\Debug\play.sbr" : $(SOURCE) $(DEP_CPP_PLAY_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\player.obj" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

".\Release\player.sbr" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\player.obj" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
   $(BuildCmds)

".\Debug\player.sbr" : $(SOURCE) $(DEP_CPP_PLAYE) "$(INTDIR)"
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

".\Release\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\portability.obj" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

".\Debug\portability.sbr" : $(SOURCE) $(DEP_CPP_PORTA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\robot.obj" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

".\Release\robot.sbr" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\robot.obj" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

".\Debug\robot.sbr" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\robotdef.obj" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

".\Release\robotdef.sbr" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\robotdef.obj" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

".\Debug\robotdef.sbr" : $(SOURCE) $(DEP_CPP_ROBOTD) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	"..\cannon.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\proto.h"\
	
NODEP_CPP_RULES=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\rules.obj" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

".\Release\rules.sbr" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\rules.obj" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

".\Debug\rules.sbr" : $(SOURCE) $(DEP_CPP_RULES) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\saudio.obj" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

".\Release\saudio.sbr" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\saudio.obj" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

".\Debug\saudio.sbr" : $(SOURCE) $(DEP_CPP_SAUDI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\sched.obj" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

".\Release\sched.sbr" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\sched.obj" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

".\Debug\sched.sbr" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\server.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

".\Release\server.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\server.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

".\Debug\server.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
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
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SOCKL=\
	"..\..\common\termnet.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

".\Release\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\socklib.obj" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

".\Debug\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	"..\cannon.h"\
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

".\Release\update.sbr" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\update.obj" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

".\Debug\update.sbr" : $(SOURCE) $(DEP_CPP_UPDAT) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\walls.obj" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

".\Release\walls.sbr" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\walls.obj" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

".\Debug\walls.sbr" : $(SOURCE) $(DEP_CPP_WALLS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\wsockerrs.c

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

".\Release\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\wsockerrs.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

".\Debug\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\common\NT\winNet.c
DEP_CPP_WINNE=\
	"..\..\common\error.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

".\Release\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\winNet.obj" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
   $(BuildCmds)

".\Debug\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE) "$(INTDIR)"
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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

".\Release\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\checknames.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

".\Debug\checknames.sbr" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

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
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

".\Release\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\shipshape.obj" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

".\Debug\shipshape.sbr" : $(SOURCE) $(DEP_CPP_SHIPS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConfigDlg.cpp
DEP_CPP_CONFI=\
	".\ConfigDlg.h"\
	".\StdAfx.h"\
	".\WinSvrThread.h"\
	".\xpilots.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


".\Release\ConfigDlg.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"

".\Release\ConfigDlg.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


".\Debug\ConfigDlg.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"

".\Debug\ConfigDlg.sbr" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\users\dick\xpilot\src\server\cannon.c
DEP_CPP_CANNO=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT/winNet.h"\
	"..\..\common\version.h"\
	"..\cannon.h"\
	"..\click.h"\
	"..\global.h"\
	"..\object.h"\
	"..\objpos.h"\
	"..\proto.h"\
	"..\saudio.h"\
	"..\score.h"\
	"..\walls.h"\
	".\winServer.h"\
	
NODEP_CPP_CANNO=\
	"..\strcasecmp.h"\
	

!IF  "$(CFG)" == "xpilots - Win32 Release"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Release\cannon.obj" : $(SOURCE) $(DEP_CPP_CANNO) "$(INTDIR)"
   $(BuildCmds)

".\Release\cannon.sbr" : $(SOURCE) $(DEP_CPP_CANNO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "xpilots - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

".\Debug\cannon.obj" : $(SOURCE) $(DEP_CPP_CANNO) "$(INTDIR)"
   $(BuildCmds)

".\Debug\cannon.sbr" : $(SOURCE) $(DEP_CPP_CANNO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
