$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!
$! A VMS Command procedure to make the foreign symbol definitions for
$! XPilot and start the XP-server program. Keep this command proc in 
$! the [.START] directory and it will get the pointers setup correctly.
$!
$! Written by Jens Koerber (koerber@ttkax1.ciw.uni-karlsruhe.de) 18-NOV-1995
$!
$!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$!
$  SAY = "WRITE SYS$OUTPUT"
$  SAY " "
$  SAY " SETTING UP XPILOT (v 3.6.3)..."
$!
$  SET NOON
$  SET PROCESS/PRIV=ALL
$!
$  XPS_DIR = F$DIRECTORY()
$  LEN = F$LENGTH(XPS_DIR)
$  TEST = F$EXTRACT((LEN-6),5,XPS_DIR)
$! 
$  IF TEST .NES. "START" 
$  THEN
$    SAY " "
$    SAY " Please start XP_STARTUP in the directory [.START]  
$    SAY " "
$    GOTO EXIT
$  ENDIF
$!
$! Put the help library into the next available help library slot
$!
$HLPLIB:
$  SET DEF [-]
$  XP_DIR  = F$PARSE("SRC.DIR",,,"DEVICE") + F$PARSE("SRC.DIR",,,"DIRECTORY")
$  XP_PATH = F$Element (0, "]", XP_DIR)
$!
$  HELP_PATH = "''XP_PATH'.CONTRIB.VMS]"
$  DEFINE/NOLOG XPILOT$MAIN   'XP_PATH'] 
$  DEFINE/NOLOG XPILOT$START  'XP_PATH'.START] 
$  DEFINE/NOLOG XPILOT$SRC    'XP_PATH'.SRC]
$  DEFINE/NOLOG XPILOT$HELP   'XP_PATH'.CONTRIB.VMS]
$!
$PRE_HLPLIB:
$  LIB = "HLP$LIBRARY"
$  X = F$TRNLNM (LIB, "Lnm$Process")
$  If X .EQS. "" Then GoTo INSERT
$  If X .EQS. "''HELP_PATH'XPILOT.HLB" Then GoTo PAST_HLPLIB
$  BASE = LIB + "_"
$  N = 1
$NEXTLIB:
$    LIB := 'BASE''N'
$    X = F$TRNLNM (LIB, "Lnm$Process")
$    If X .EQS. "" Then GoTo INSERT
$    If X .EQS. "''HELP_PATH'XPILOT.HLB" Then GoTo PAST_HLPLIB
$    N = N + 1
$    GoTo NEXTLIB
$INSERT:
$  DEFINE/NOLOG 'LIB' 'HELP_PATH'XPILOT.HLB
$!
$PAST_HLPLIB:
$  SET DEF XPILOT$START
$!
$  IF F$GETSYI("HW_MODEL") .GE. 1024
$  THEN
$   CPU="AXP"
$  ELSE
$   CPU="VAX"
$  ENDIF
$!
$SERVER:
$  SAY " "
$  INQUIRE SERV " Start the XPilot server (Y/N) ? "
$  IF SERV .EQS."N" .OR. SERV .EQS. "n" THEN GOTO CLIENT
$! 
$PROC:
$  I_XP=0
$PROC_CHECK:
$  PRC_STAT = "XP_SERVER"
$  CONTEXT = ""
$PROC_START:
$  PID = F$PID(CONTEXT)
$  PRC_NAME = F$GETJPI(PID,"PRCNAM")
$  IF PRC_NAME .NES. PRC_STAT THEN GOTO PROC_END
$  I_XP=I_XP+1
$PROC_END:
$  IF PID .EQS. "" THEN GOTO PROC_ON
$  GOTO PROC_START
$PROC_ON:
$!
$  IF I_XP .EQ. 0 
$  THEN
$    RUN  /DETACH /PRIORITY=6 -
          /ERROR=[-.START]XPILOT_'CPU'.ERR - 
          /PROCESS_NAME=XP_SERVER -
          /AST_LIMIT=4095 -
          /BUFFER_LIMIT=100000 -
          /IO_BUFFERED=255 -
          /IO_DIRECT=4096 -
          /ENQUEUE_LIMIT=2000 - 
          /WORKING_SET=20000 -
          /OUTPUT=[-.START]XPILOT_'CPU'.OUT -
          XPILOT$SRC:XPILOTS_'CPU'.EXE  
$  ELSE
$    SAY " "
$    SAY " The XPilot-Server has already been started ..."
$    SAY " "
$  ENDIF
$!
$CLIENT:
$  XP :== "$XPILOT$SRC:XPILOT_''CPU'"
$  SAY " "
$  SAY " prompt> XP invokes the XPilot client program. This command is"
$  SAY " described in detail in the Help-Library prompt> HELP @XPILOT "
$  SAY " "
$!
$REPLAY:
$  XP_REPLAY :== "$XPILOT$SRC:XP-REPLAY_''CPU'"
$  SAY " "
$  SAY " prompt> XP-REPLAY invokes the XPilot replay program. This command is"
$  SAY " described in detail in the Help-Library prompt> HELP @XPILOT"
$  SAY " "
$!
$PURGE:
$  PURGE/NOCONFIRM/NOLOG [-.START]
$!
$EXIT:
$  EXIT
