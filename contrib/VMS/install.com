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
$ SAY = "WRITE SYS$OUTPUT"
$ SAY " INSTALLING XPILOT (v 3.4.2)..."
$!
$ ON CONTROL_Y THEN GOTO EXIT
$ SET NOON
$!
$ XPI_DIR = F$DIRECTORY()
$ LEN = F$LENGTH(XPI_DIR)
$ TEST = F$EXTRACT((LEN-4),3,XPI_DIR)
$! 
$ IF TEST .NES. "VMS" 
$ THEN
$   SAY " "
$   SAY " Please start INSTALL in the directory [.CONTRIB.VMS]
$   SAY " "
$   GOTO EXIT
$ ENDIF
$!
$INTER1:
$ SAY " "
$ INQUIRE CONFIG " Do you want to edit the configuration file CONFIG.H (Y/N)"
$ IF CONFIG .EQS. "" THEN GOTO INTER1
$ IF CONFIG .EQS. "Y" .OR. CONFIG .EQS. "y" THEN GOTO EXIT
$INTER2:
$ SAY " "
$ INQUIRE COMPILER " DECC(D) or GCC(G) Compiler ?"
$ IF COMPILER .EQS. "" THEN GOTO INTER2
$ SAY " "
$!
$! Copy textures to [-.src.textures] 
$!  
$ CREATE/DIR                    [-.-.SRC.TEXTURES]
$ COPY [-.-.LIB.TEXTURES]*.*    [-.-.SRC.TEXTURES]*.*
$!
$! Copy VMS-Files to [-.start] 
$!  
$ CREATE/DIR                    [-.-.START]
$ COPY X*.COM                   [-.-.START]*.*
$ PURGE/NOLOG/NOCONFIRM         [-.-.START...]
$!
$! Copy VMS-Files to [-.src] 
$!  
$ COPY *.C                      [-.-.SRC]*.*
$ COPY *.H                      [-.-.SRC]*.*
$ COPY M*.COM                   [-.-.SRC]*.*
$ COPY *.OPT                    [-.-.SRC]*.*
$ COPY *.MMS                    [-.-.SRC]*.*
$!
$! Copy VMS-Files to [src.vms_include] (GCC only) 
$!  
$ IF COMPILER .EQS. "G" .OR. COMPILER .EQS. "g"
$ THEN
$   CREATE/DIR                  [-.-.SRC.VMS_INCLUDE]
$   COPY [.VMS_INCLUDE]*.H      [-.-.SRC.VMS_INCLUDE]*.*
$   COPY [.VMS_INCLUDE.SYS]*.H  [-.-.SRC.VMS_INCLUDE]*.*
$ ENDIF
$ PURGE/NOLOG/NOCONFIRM         [-.-.SRC...]
$!
$! Compile the Server program
$!
$ SET DEF                       [-.-.SRC]
$ @MAKEXPS 'COMPILER'
$!
$! Compile the Client program
$!
$ @MAKEXP  'COMPILER'
$ SET DEF                       [-.START]
$EXIT:
$ EXIT
