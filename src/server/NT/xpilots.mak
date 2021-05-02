# Microsoft Developer Studio Generated NMAKE File, Based on xpilots.dsp
!IF "$(CFG)" == ""
CFG=XPilotServer - Win32 Release
!MESSAGE No configuration specified. Defaulting to XPilotServer - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "XPilotServer - Win32 Release" && "$(CFG)" != "XPilotServer - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilots.mak" CFG="XPilotServer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XPilotServer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "XPilotServer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "XPilotServer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\XPilotServer.exe" "$(OUTDIR)\xpilots.bsc"


CLEAN :
	-@erase "$(INTDIR)\asteroid.obj"
	-@erase "$(INTDIR)\asteroid.sbr"
	-@erase "$(INTDIR)\cannon.obj"
	-@erase "$(INTDIR)\cannon.sbr"
	-@erase "$(INTDIR)\cell.obj"
	-@erase "$(INTDIR)\cell.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\command.obj"
	-@erase "$(INTDIR)\command.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
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
	-@erase "$(INTDIR)\fileparser.obj"
	-@erase "$(INTDIR)\fileparser.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\id.sbr"
	-@erase "$(INTDIR)\item.obj"
	-@erase "$(INTDIR)\item.sbr"
	-@erase "$(INTDIR)\laser.obj"
	-@erase "$(INTDIR)\laser.sbr"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\list.sbr"
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
	-@erase "$(INTDIR)\parser.obj"
	-@erase "$(INTDIR)\parser.sbr"
	-@erase "$(INTDIR)\play.obj"
	-@erase "$(INTDIR)\play.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\randommt.obj"
	-@erase "$(INTDIR)\randommt.sbr"
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
	-@erase "$(INTDIR)\score.obj"
	-@erase "$(INTDIR)\score.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\ship.obj"
	-@erase "$(INTDIR)\ship.sbr"
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\shot.obj"
	-@erase "$(INTDIR)\shot.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\strdup.obj"
	-@erase "$(INTDIR)\strdup.sbr"
	-@erase "$(INTDIR)\strlcpy.obj"
	-@erase "$(INTDIR)\strlcpy.sbr"
	-@erase "$(INTDIR)\update.obj"
	-@erase "$(INTDIR)\update.sbr"
	-@erase "$(INTDIR)\UrlWidget.obj"
	-@erase "$(INTDIR)\UrlWidget.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\walls.obj"
	-@erase "$(INTDIR)\walls.sbr"
	-@erase "$(INTDIR)\wildmap.obj"
	-@erase "$(INTDIR)\wildmap.sbr"
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
	-@erase "$(INTDIR)\xpmemory.obj"
	-@erase "$(INTDIR)\xpmemory.sbr"
	-@erase "$(OUTDIR)\xpilots.bsc"
	-@erase "$(OUTDIR)\XPilotServer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xpilots.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xpilots.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\asteroid.sbr" \
	"$(INTDIR)\cannon.sbr" \
	"$(INTDIR)\cell.sbr" \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\command.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\event.sbr" \
	"$(INTDIR)\fileparser.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\id.sbr" \
	"$(INTDIR)\item.sbr" \
	"$(INTDIR)\laser.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\metaserver.sbr" \
	"$(INTDIR)\netserver.sbr" \
	"$(INTDIR)\objpos.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\parser.sbr" \
	"$(INTDIR)\play.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\robot.sbr" \
	"$(INTDIR)\robotdef.sbr" \
	"$(INTDIR)\rules.sbr" \
	"$(INTDIR)\saudio.sbr" \
	"$(INTDIR)\sched.sbr" \
	"$(INTDIR)\score.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\ship.sbr" \
	"$(INTDIR)\shot.sbr" \
	"$(INTDIR)\update.sbr" \
	"$(INTDIR)\walls.sbr" \
	"$(INTDIR)\wildmap.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
	"$(INTDIR)\ExitXpilots.sbr" \
	"$(INTDIR)\ReallyShutdown.sbr" \
	"$(INTDIR)\UrlWidget.sbr" \
	"$(INTDIR)\winSvrThread.sbr" \
	"$(INTDIR)\xpilots.sbr" \
	"$(INTDIR)\xpilotsDlg.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\list.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\randommt.sbr" \
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\strdup.sbr" \
	"$(INTDIR)\strlcpy.sbr" \
	"$(INTDIR)\xpmemory.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\wsockerrs.sbr"

"$(OUTDIR)\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\XPilotServer.pdb" /machine:I386 /out:"$(OUTDIR)\XPilotServer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\asteroid.obj" \
	"$(INTDIR)\cannon.obj" \
	"$(INTDIR)\cell.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\command.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\fileparser.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\item.obj" \
	"$(INTDIR)\laser.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\metaserver.obj" \
	"$(INTDIR)\netserver.obj" \
	"$(INTDIR)\objpos.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\play.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\robot.obj" \
	"$(INTDIR)\robotdef.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\saudio.obj" \
	"$(INTDIR)\sched.obj" \
	"$(INTDIR)\score.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\ship.obj" \
	"$(INTDIR)\shot.obj" \
	"$(INTDIR)\update.obj" \
	"$(INTDIR)\walls.obj" \
	"$(INTDIR)\wildmap.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
	"$(INTDIR)\ExitXpilots.obj" \
	"$(INTDIR)\ReallyShutdown.obj" \
	"$(INTDIR)\UrlWidget.obj" \
	"$(INTDIR)\winSvrThread.obj" \
	"$(INTDIR)\xpilots.obj" \
	"$(INTDIR)\xpilotsDlg.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\randommt.obj" \
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\strdup.obj" \
	"$(INTDIR)\strlcpy.obj" \
	"$(INTDIR)\xpmemory.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xpilots.res"

"$(OUTDIR)\XPilotServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "XPilotServer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\XPilotServer.exe" "$(OUTDIR)\xpilots.bsc"


CLEAN :
	-@erase "$(INTDIR)\asteroid.obj"
	-@erase "$(INTDIR)\asteroid.sbr"
	-@erase "$(INTDIR)\cannon.obj"
	-@erase "$(INTDIR)\cannon.sbr"
	-@erase "$(INTDIR)\cell.obj"
	-@erase "$(INTDIR)\cell.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\cmdline.sbr"
	-@erase "$(INTDIR)\collision.obj"
	-@erase "$(INTDIR)\collision.sbr"
	-@erase "$(INTDIR)\command.obj"
	-@erase "$(INTDIR)\command.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
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
	-@erase "$(INTDIR)\fileparser.obj"
	-@erase "$(INTDIR)\fileparser.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\id.sbr"
	-@erase "$(INTDIR)\item.obj"
	-@erase "$(INTDIR)\item.sbr"
	-@erase "$(INTDIR)\laser.obj"
	-@erase "$(INTDIR)\laser.sbr"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\list.sbr"
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
	-@erase "$(INTDIR)\parser.obj"
	-@erase "$(INTDIR)\parser.sbr"
	-@erase "$(INTDIR)\play.obj"
	-@erase "$(INTDIR)\play.sbr"
	-@erase "$(INTDIR)\player.obj"
	-@erase "$(INTDIR)\player.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\randommt.obj"
	-@erase "$(INTDIR)\randommt.sbr"
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
	-@erase "$(INTDIR)\score.obj"
	-@erase "$(INTDIR)\score.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\ship.obj"
	-@erase "$(INTDIR)\ship.sbr"
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\shot.obj"
	-@erase "$(INTDIR)\shot.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\strdup.obj"
	-@erase "$(INTDIR)\strdup.sbr"
	-@erase "$(INTDIR)\strlcpy.obj"
	-@erase "$(INTDIR)\strlcpy.sbr"
	-@erase "$(INTDIR)\update.obj"
	-@erase "$(INTDIR)\update.sbr"
	-@erase "$(INTDIR)\UrlWidget.obj"
	-@erase "$(INTDIR)\UrlWidget.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\walls.obj"
	-@erase "$(INTDIR)\walls.sbr"
	-@erase "$(INTDIR)\wildmap.obj"
	-@erase "$(INTDIR)\wildmap.sbr"
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
	-@erase "$(INTDIR)\xpmemory.obj"
	-@erase "$(INTDIR)\xpmemory.sbr"
	-@erase "$(OUTDIR)\xpilots.bsc"
	-@erase "$(OUTDIR)\XPilotServer.exe"
	-@erase "$(OUTDIR)\XPilotServer.ilk"
	-@erase "$(OUTDIR)\XPilotServer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_XPILOTNTSERVER_" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xpilots.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xpilots.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\asteroid.sbr" \
	"$(INTDIR)\cannon.sbr" \
	"$(INTDIR)\cell.sbr" \
	"$(INTDIR)\cmdline.sbr" \
	"$(INTDIR)\collision.sbr" \
	"$(INTDIR)\command.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\event.sbr" \
	"$(INTDIR)\fileparser.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\id.sbr" \
	"$(INTDIR)\item.sbr" \
	"$(INTDIR)\laser.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\metaserver.sbr" \
	"$(INTDIR)\netserver.sbr" \
	"$(INTDIR)\objpos.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\parser.sbr" \
	"$(INTDIR)\play.sbr" \
	"$(INTDIR)\player.sbr" \
	"$(INTDIR)\robot.sbr" \
	"$(INTDIR)\robotdef.sbr" \
	"$(INTDIR)\rules.sbr" \
	"$(INTDIR)\saudio.sbr" \
	"$(INTDIR)\sched.sbr" \
	"$(INTDIR)\score.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\ship.sbr" \
	"$(INTDIR)\shot.sbr" \
	"$(INTDIR)\update.sbr" \
	"$(INTDIR)\walls.sbr" \
	"$(INTDIR)\wildmap.sbr" \
	"$(INTDIR)\ConfigDlg.sbr" \
	"$(INTDIR)\ExitXpilots.sbr" \
	"$(INTDIR)\ReallyShutdown.sbr" \
	"$(INTDIR)\UrlWidget.sbr" \
	"$(INTDIR)\winSvrThread.sbr" \
	"$(INTDIR)\xpilots.sbr" \
	"$(INTDIR)\xpilotsDlg.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\list.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\randommt.sbr" \
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\strdup.sbr" \
	"$(INTDIR)\strlcpy.sbr" \
	"$(INTDIR)\xpmemory.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\wsockerrs.sbr"

"$(OUTDIR)\xpilots.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\XPilotServer.pdb" /debug /machine:I386 /out:"$(OUTDIR)\XPilotServer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\asteroid.obj" \
	"$(INTDIR)\cannon.obj" \
	"$(INTDIR)\cell.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\collision.obj" \
	"$(INTDIR)\command.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\fileparser.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\item.obj" \
	"$(INTDIR)\laser.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\metaserver.obj" \
	"$(INTDIR)\netserver.obj" \
	"$(INTDIR)\objpos.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\play.obj" \
	"$(INTDIR)\player.obj" \
	"$(INTDIR)\robot.obj" \
	"$(INTDIR)\robotdef.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\saudio.obj" \
	"$(INTDIR)\sched.obj" \
	"$(INTDIR)\score.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\ship.obj" \
	"$(INTDIR)\shot.obj" \
	"$(INTDIR)\update.obj" \
	"$(INTDIR)\walls.obj" \
	"$(INTDIR)\wildmap.obj" \
	"$(INTDIR)\ConfigDlg.obj" \
	"$(INTDIR)\ExitXpilots.obj" \
	"$(INTDIR)\ReallyShutdown.obj" \
	"$(INTDIR)\UrlWidget.obj" \
	"$(INTDIR)\winSvrThread.obj" \
	"$(INTDIR)\xpilots.obj" \
	"$(INTDIR)\xpilotsDlg.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\randommt.obj" \
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\strdup.obj" \
	"$(INTDIR)\strlcpy.obj" \
	"$(INTDIR)\xpmemory.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xpilots.res"

"$(OUTDIR)\XPilotServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("xpilots.dep")
!INCLUDE "xpilots.dep"
!ELSE 
!MESSAGE Warning: cannot find "xpilots.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "XPilotServer - Win32 Release" || "$(CFG)" == "XPilotServer - Win32 Debug"
SOURCE=..\asteroid.c

"$(INTDIR)\asteroid.obj"	"$(INTDIR)\asteroid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\cannon.c

"$(INTDIR)\cannon.obj"	"$(INTDIR)\cannon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\cell.c

"$(INTDIR)\cell.obj"	"$(INTDIR)\cell.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\cmdline.c

"$(INTDIR)\cmdline.obj"	"$(INTDIR)\cmdline.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\collision.c

"$(INTDIR)\collision.obj"	"$(INTDIR)\collision.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\command.c

"$(INTDIR)\command.obj"	"$(INTDIR)\command.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\contact.c

"$(INTDIR)\contact.obj"	"$(INTDIR)\contact.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\event.c

"$(INTDIR)\event.obj"	"$(INTDIR)\event.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\fileparser.c

"$(INTDIR)\fileparser.obj"	"$(INTDIR)\fileparser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\frame.c

"$(INTDIR)\frame.obj"	"$(INTDIR)\frame.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\id.c

"$(INTDIR)\id.obj"	"$(INTDIR)\id.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\item.c

"$(INTDIR)\item.obj"	"$(INTDIR)\item.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\laser.c

"$(INTDIR)\laser.obj"	"$(INTDIR)\laser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\map.c

"$(INTDIR)\map.obj"	"$(INTDIR)\map.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\metaserver.c

"$(INTDIR)\metaserver.obj"	"$(INTDIR)\metaserver.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\netserver.c

"$(INTDIR)\netserver.obj"	"$(INTDIR)\netserver.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\objpos.c

"$(INTDIR)\objpos.obj"	"$(INTDIR)\objpos.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\option.c

"$(INTDIR)\option.obj"	"$(INTDIR)\option.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\parser.c

"$(INTDIR)\parser.obj"	"$(INTDIR)\parser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\play.c

"$(INTDIR)\play.obj"	"$(INTDIR)\play.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\player.c

"$(INTDIR)\player.obj"	"$(INTDIR)\player.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\robot.c

"$(INTDIR)\robot.obj"	"$(INTDIR)\robot.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\robotdef.c

"$(INTDIR)\robotdef.obj"	"$(INTDIR)\robotdef.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\rules.c

"$(INTDIR)\rules.obj"	"$(INTDIR)\rules.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\saudio.c

"$(INTDIR)\saudio.obj"	"$(INTDIR)\saudio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\sched.c

"$(INTDIR)\sched.obj"	"$(INTDIR)\sched.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\score.c

"$(INTDIR)\score.obj"	"$(INTDIR)\score.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\server.c

"$(INTDIR)\server.obj"	"$(INTDIR)\server.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\ship.c

"$(INTDIR)\ship.obj"	"$(INTDIR)\ship.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shot.c

"$(INTDIR)\shot.obj"	"$(INTDIR)\shot.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\update.c

"$(INTDIR)\update.obj"	"$(INTDIR)\update.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\walls.c

"$(INTDIR)\walls.obj"	"$(INTDIR)\walls.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\wildmap.c

"$(INTDIR)\wildmap.obj"	"$(INTDIR)\wildmap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ConfigDlg.cpp

"$(INTDIR)\ConfigDlg.obj"	"$(INTDIR)\ConfigDlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ExitXpilots.cpp

"$(INTDIR)\ExitXpilots.obj"	"$(INTDIR)\ExitXpilots.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ReallyShutdown.cpp

"$(INTDIR)\ReallyShutdown.obj"	"$(INTDIR)\ReallyShutdown.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\UrlWidget.cpp

"$(INTDIR)\UrlWidget.obj"	"$(INTDIR)\UrlWidget.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winSvrThread.c

"$(INTDIR)\winSvrThread.obj"	"$(INTDIR)\winSvrThread.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xpilots.cpp

"$(INTDIR)\xpilots.obj"	"$(INTDIR)\xpilots.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xpilots.rc

"$(INTDIR)\xpilots.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\xpilotsDlg.cpp

"$(INTDIR)\xpilotsDlg.obj"	"$(INTDIR)\xpilotsDlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\common\checknames.c

"$(INTDIR)\checknames.obj"	"$(INTDIR)\checknames.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\config.c

"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\error.c

"$(INTDIR)\error.obj"	"$(INTDIR)\error.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\list.c

"$(INTDIR)\list.obj"	"$(INTDIR)\list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\math.c

"$(INTDIR)\math.obj"	"$(INTDIR)\math.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\net.c

"$(INTDIR)\net.obj"	"$(INTDIR)\net.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\portability.c

"$(INTDIR)\portability.obj"	"$(INTDIR)\portability.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\randommt.c

"$(INTDIR)\randommt.obj"	"$(INTDIR)\randommt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\shipshape.c

"$(INTDIR)\shipshape.obj"	"$(INTDIR)\shipshape.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\socklib.c

"$(INTDIR)\socklib.obj"	"$(INTDIR)\socklib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\strdup.c

"$(INTDIR)\strdup.obj"	"$(INTDIR)\strdup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\strlcpy.c

"$(INTDIR)\strlcpy.obj"	"$(INTDIR)\strlcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\xpmemory.c

"$(INTDIR)\xpmemory.obj"	"$(INTDIR)\xpmemory.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\NT\winNet.c

"$(INTDIR)\winNet.obj"	"$(INTDIR)\winNet.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\NT\wsockerrs.c

"$(INTDIR)\wsockerrs.obj"	"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

