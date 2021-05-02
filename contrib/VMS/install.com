$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!
$! A VMS Command procedure to install the XPilot server and client program.
$! Keep this command proc in the [.CONTRIB.VMS] directory and it will get 
$! the pointers setup correctly.
$!
$! Written by Jens Koerber (koerber@ttkax1.ciw.uni-karlsruhe.de) 18-NOV-1995
$!
$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!
$ SAVED_VERIFY='F$VERIFY(0)
$ SET NOVERIFY
$ ON CONTROL_Y THEN GOTO EXIT
$ SET NOON
$ SAY = "WRITE SYS$OUTPUT"
$!
$ SAY " "
$ SAY " INSTALLING XPILOT V3.5.0 ..."
$!
$! ---------------------------------------------------------------------------
$! CPU type (VAX, Alpha)
$! ---------------------------------------------------------------------------
$!
$ IF F$GETSYI("HW_MODEL") .GE. 1024
$ THEN
$   CPU="AXP"
$ ELSE
$   CPU="VAX"
$ ENDIF
$!
$! ---------------------------------------------------------------------------
$! Set up Directories
$! ---------------------------------------------------------------------------
$!
$ XP_DEV  = F$PARSE("INSTALL.COM",,,"DEVICE")
$ XPI_DIR = F$DIRECTORY()
$ LENGTH  = F$LENGTH(XPI_DIR)
$ XPS_DIR = F$EXTRACT(0,(LENGTH-12),XPI_DIR) + "SRC]"
$ T_VMS   = F$EXTRACT((LENGTH-4),3,XPI_DIR)
$! 
$ IF T_VMS .NES. "VMS" 
$ THEN
$   SAY " "
$   SAY " Please start INSTALL in the directory [.CONTRIB.VMS]
$   SAY " "
$   GOTO EXIT
$ ENDIF
$!
$! ---------------------------------------------------------------------------
$! Edit configuration file
$! ---------------------------------------------------------------------------
$!
$INTER1:
$ SAY " "
$ INQUIRE CONFIG " Do you have changed the configuration file [.SRC]CONFIG.H (Y/N)"
$ IF CONFIG .EQS. "" THEN GOTO INTER1
$ IF CONFIG .NES. "Y" .AND. CONFIG .NES. "y"
$ THEN
$   CALL EDIT_CONFIG
$ ENDIF
$ SAY " "
$!
$! ---------------------------------------------------------------------------
$! Get compiler 
$! ---------------------------------------------------------------------------
$!
$ IF F$SEARCH("SYS$SYSTEM:DECC$COMPILER.EXE") .EQS."" 
$ THEN 
$   COMPILER = "G"
$ ELSE
$   COMPILER = "D"
$ ENDIF
$!
$! ---------------------------------------------------------------------------
$! Create directories and copy files
$! ---------------------------------------------------------------------------
$!  
$ SET DEFAULT 'XPI_DIR'
$!
$ COPY [-.TKXPINTERFACE.SRC]*.SHP  [-.-.LIB]*.*
$ SET FILE /PROT=(G:RE,W:RE)       [-.-.LIB]*.SHP
$ COPY *.C                         [-.-.SRC]*.*
$ COPY *.H                         [-.-.SRC]*.*
$ COPY M*.COM                      [-.-.SRC]*.*
$ COPY *.OPT                       [-.-.SRC]*.*
$ COPY *.MMS                       [-.-.SRC]*.*
$ CREATE/DIR                       [-.-.SRC.VMS_INCLUDE]
$ COPY [.VMS_INCLUDE]*.H           [-.-.SRC.VMS_INCLUDE]*.*
$ COPY [.VMS_INCLUDE.SYS]*.H       [-.-.SRC.VMS_INCLUDE]*.*
$ CREATE/DIR                       [-.-.START]
$ COPY X*.COM                      [-.-.START]*.*
$!
$ PURGE/NOLOG/NOCONFIRM            [-.-.LIB]
$ PURGE/NOLOG/NOCONFIRM            [-.-.SRC...]
$ PURGE/NOLOG/NOCONFIRM            [-.-.START...]
$!
$! ---------------------------------------------------------------------------
$! Compile the Client program
$! ---------------------------------------------------------------------------
$!
$ SET DEFAULT 'XPS_DIR'
$ @MAKEXP  'COMPILER'
$ @MAKEXP CLEAN
$ SET DEFAULT 'XPI_DIR'
$!
$! ---------------------------------------------------------------------------
$! Ensure you are running on OpenVMS VAX V7.0 or higher (DECC)
$! ---------------------------------------------------------------------------
$!
$ IF COMPILER .EQS. "D" .OR. COMPILER .EQS. "d"
$ THEN
$   VERSION = F$GETSYI("VERSION")
$   ARCH_NAME = F$GETSYI("ARCH_NAME")
$   VMS_VERS = ARCH_NAME + "/VMS " + VERSION
$   IV = F$INTEGER(F$EXTRACT(1,1,VERSION))
$   IF IV.LE.6 
$   THEN
$     SAY " "
$     SAY " "
$     SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$     SAY "          Can't build the XPilot 3.5.0 server !                 "
$     SAY "   ''VMS_VERS' doesn't have the sigset family, *sigh* !         "
$     SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$     SAY " "
$     SAY " Trying @INSTALL_VMS_SERVER.COM "
$     IF F$SEARCH("INSTALL_VMS_SERVER.COM") .NES.""
$     THEN
$       @INSTALL_VMS_SERVER.COM
$     ELSE
$       SAY " "
$       SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$       SAY "    Either upgrade to VMS 7.x or install XPILOT342-VMS-SERVER    "
$       SAY "    from ftp://ttkax1.ciw.uni-karlsruhe.de/pub/xgames/xpilot/    "
$       SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$     ENDIF
$     GOTO EXIT 
$   ENDIF
$ ENDIF
$!
$! ---------------------------------------------------------------------------
$! Ensure DECC Version is V5.2 or higher
$! ---------------------------------------------------------------------------
$!
$ IF COMPILER .EQS. "D" .OR. COMPILER .EQS. "d"
$ THEN
$   SET MESSAGE/NOTEXT/NOFACILITY/NOSEVERITY/NOIDENTIFICATION
$   DEFINE/USER_MODE SYS$OUTPUT VERSION.DAT
$   CC /DECC /VERSION NL:
$   DEASSIGN SYS$OUTPUT
$   SET MESSAGE/TEXT/FACILITY/SEVERITY/IDENTIFICATION
$   OPEN/READ VERFIL VERSION.DAT
$   READ VERFIL VERSION
$   CLOSE VERFIL
$   DELETE/NOLOG/NOCONFIRM VERSION.DAT;*
$   VERS = F$EXTRACT(6,4,VERSION)
$   IC1 = F$INTEGER(F$EXTRACT(7,1,VERSION))
$   IC2 = F$INTEGER(F$EXTRACT(9,1,VERSION))
$   IF IC1 .LE. 5  
$   THEN
$     IF IC2 .LE. 1
$     THEN
$       SAY " "
$       SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$       SAY "          Can't build the XPilot 3.5.0 server !              "
$       SAY "   DEC C ''VERS' doesn't have the sigset family, *sigh* !    "
$       SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$       SAY " "
$       SAY " Trying @INSTALL_VMS_SERVER.COM "
$       IF F$SEARCH("INSTALL_VMS_SERVER.COM") .NES.""
$       THEN
$         @INSTALL_VMS_SERVER.COM
$       ELSE
$         SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$         SAY "  Either upgrade to VMS 7.x or install XPILOT342-VMS-SERVER  "
$         SAY "   from ftp://ttkax1.ciw.uni-karlsruhe.de/pub/xgames/xpilot/ "
$         SAY "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
$       ENDIF
$       GOTO EXIT 
$     ENDIF
$   ENDIF 
$ ENDIF
$!
$! ---------------------------------------------------------------------------
$! Compile the Server program
$! ---------------------------------------------------------------------------
$!
$ SET DEFAULT 'XPS_DIR'
$ @MAKEXPS 'COMPILER'
$ @MAKEXPS CLEAN
$ SET DEFAULT 'XPI_DIR'
$!
$ SET DEF [-.-.START]
$ DIR
$!
$EXIT:
$  IF SAVED_VERIFY .EQ. 1 THEN SET VERIFY
$  EXIT
$!
$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$EDIT_CONFIG: SUBROUTINE   !SUBROUTINE TO SET CONFIGURATION OPTIONS
$!
$ SET NOON
$ SAY " "
$ SAY " Creating REPLACE.TPU ..."
$!
$ tmp1_old = "'" + "lib_disk:[lib.xgames.xpilot341.lib" + "'"
$ tmp1_new = "'" + XP_DEV + F$EXTRACT(0,(LENGTH-12),XPI_DIR) +"LIB" + "'"
$!
$ SET DEFAULT 'XPS_DIR'
$ OPEN/WRITE DATEI REPLACE.TPU
$ WRITE DATEI   "procedure global_replace (str1,str2)"
$ WRITE DATEI   "local src_range;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "on_error"
$ WRITE DATEI   "  return;"
$ WRITE DATEI   "endon_error;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "loop"
$ WRITE DATEI   "  src_range := search (str1,forward);"
$ WRITE DATEI   "  erase (src_range);"
$ WRITE DATEI   "  position (end_of (src_range));"
$ WRITE DATEI   "  copy_text (str2);"
$ WRITE DATEI   "endloop;"
$ WRITE DATEI   "endprocedure;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "procedure global_delete (str1)"
$ WRITE DATEI   "local src_range;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "on_error"
$ WRITE DATEI   "  return;"
$ WRITE DATEI   "endon_error;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "loop"
$ WRITE DATEI   "  src_range := search (str1,forward);"
$ WRITE DATEI   "  erase (src_range);"
$ WRITE DATEI   "  position (end_of (src_range));"
$ WRITE DATEI   "endloop;"
$ WRITE DATEI   "endprocedure;"
$ WRITE DATEI   "!"
$ WRITE DATEI   "!"
$ WRITE DATEI   "input_file := get_info (command_line, 'file_name');"
$ WRITE DATEI   "!"
$ WRITE DATEI   "main_buffer := create_buffer ('main',input_file);"
$ WRITE DATEI   "lines := get_info (main_buffer,'record_count');"
$ WRITE DATEI   "set (insert,main_buffer);"
$ WRITE DATEI   "!"
$ WRITE DATEI   "position (beginning_of(main_buffer));"
$ WRITE DATEI   "message ("" *** Searching      :: "" + ""''tmp1_old'"");"
$ WRITE DATEI   "message ("" *** Replacing with :: "" + ""''tmp1_new'"");"
$ WRITE DATEI   "global_replace (''tmp1_old',''tmp1_new');"
$ WRITE DATEI   "position (beginning_of(main_buffer));"
$ WRITE DATEI   "write_file (main_buffer,'newfile.tmp');"
$ WRITE DATEI   "quit;"
$ CLOSE DATEI
$!
$ FILE = XPS_DIR + "CONFIG.H"
$ SET MESSAGE/NOTEXT/NOFACILITY/NOSEVERITY/NOIDENTIFICATION
$ WRITE SYS$OUTPUT " "
$ WRITE SYS$OUTPUT " *** Now in buffer :: ''FILE'"
$ EDIT/TPU/NOSECTION -
      /COMMAND=REPLACE.TPU -
      /NODISPLAY 'FILE'
$ SET MESSAGE/TEXT/FACILITY/SEVERITY/IDENTIFICATION
$ RENAME NEWFILE.TMP 'FILE'
$ DELETE REPLACE.TPU;*
$ SET DEFAULT 'XPI_DIR'
$ DELETE *.OBJ;*
$!
$ENDSUBROUTINE !SUBROUTINE EDIT_CONFIG
