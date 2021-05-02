$ SAVE_VERIFY='F$VERIFY(0)
$ on control_y then goto exit
$!
$ if p1 .eqs. "CLEAN" then goto clean
$ if p1 .eqs. "CLOBBER" then goto clobber
$ if p1 .eqs. "" then inquire p1 "DECC(D) or GCC(G) Compiler ?"
$!
$!      Compile the Xdaliclock Application, adapted for xpilot
$!
$ set Symbol/Scope=NoGlobal
$ define X11 decw$include/nolog
$!
$!      CPU type (VAX, Alpha)
$!
$ if f$getsyi("HW_MODEL") .ge. 1024
$ then
$   cpu="AXP"
$ else
$   cpu="VAX"
$ endif
$!
$!      Get the compiler options
$!
$ if p1.eqs."D".or.p1.eqs."d"
$ then
$   options="CC/decc/standard=vaxc/precision=single"
$   if cpu .eqs. "VAX" 
$   then
$     defines="/define=(VMS,VAX)"
$     include=""
$     debug="/optimize=(disjoint,inline)"
$!    debug="/debug/nooptimize"
$   else
$     defines="/define=(VMS)"
$     include=""
$     debug="/optimize=(level=4)"
$!    debug="/debug/nooptimize"
$   endif
$ else
$   options="GCC/standard=ansi/nocasehack"
$   defines="/define=(ALPHA,NO_X_GBLS)"
$   include="/include=[.vms_include]"
$   debug="/optimize"
$!  debug="/debug"
$ endif
$ cc_options=options+defines+include+debug
$!
$!      Compile the "C" files
$!
$! procedure target	   command 			   depends upon
$! CALL MAKE .OBJ	   "CC ''cc_options' .C"	   .C
$!
$ write sys$output " "
$ write sys$output "Compiling XPILOT Client"
$ CALL MAKE XPILOT.OBJ 	   "''cc_options' XPILOT.C"	   XPILOT.C
$ CALL MAKE CLIENT.OBJ	   "''cc_options' CLIENT.C" 	   CLIENT.C
$ CALL MAKE JOIN.OBJ	   "''cc_options' JOIN.C"	   JOIN.C
$ CALL MAKE NET.OBJ	   "''cc_options' NET.C"	   NET.C
$ CALL MAKE NETCLIENT.OBJ  "''cc_options' NETCLIENT.C"     NETCLIENT.C
$ CALL MAKE PAINT.OBJ	   "''cc_options' PAINT.C"	   PAINT.C
$ CALL MAKE DBUFF.OBJ	   "''cc_options' DBUFF.C"	   DBUFF.C
$ CALL MAKE XINIT.OBJ	   "''cc_options' XINIT.C"	   XINIT.C
$ CALL MAKE DEFAULT.OBJ	   "''cc_options' DEFAULT.C"	   DEFAULT.C
$ CALL MAKE MATH.OBJ	   "''cc_options' MATH.C"	   MATH.C
$ CALL MAKE XEVENT.OBJ	   "''cc_options' XEVENT.C"	   XEVENT.C
$ CALL MAKE SYSLIMIT.OBJ   "''cc_options' SYSLIMIT.C"      SYSLIMIT.C
$ CALL MAKE ERROR.OBJ	   "''cc_options' ERROR.C"	   ERROR.C
$ CALL MAKE SOCKLIB.OBJ	   "''cc_options' SOCKLIB.C"	   SOCKLIB.C
$ CALL MAKE CONFIGURE.OBJ  "''cc_options' CONFIGURE.C"     CONFIGURE.C
$ CALL MAKE WIDGET.OBJ	   "''cc_options' WIDGET.C"	   WIDGET.C
$ CALL MAKE CAUDIO.OBJ	   "''cc_options' CAUDIO.C"	   CAUDIO.C
$ CALL MAKE USLEEP.OBJ	   "''cc_options' USLEEP.C"	   USLEEP.C
$ CALL MAKE RECORD.OBJ	   "''cc_options' RECORD.C"	   RECORD.C
$ CALL MAKE TEXTURE.OBJ	   "''cc_options' TEXTURE.C"	   TEXTURE.C
$ CALL MAKE XPMREAD.OBJ	   "''cc_options' XPMREAD.C"	   XPMREAD.C
$ CALL MAKE USERNAME.OBJ   "''cc_options' USERNAME.C"      USERNAME.C
$ CALL MAKE TRNLNM.OBJ     "''cc_options' TRNLNM.C"        TRNLNM.C
$ CALL MAKE IOCTL.OBJ	   "''cc_options' IOCTL.C"	   IOCTL.C
$ CALL MAKE STRCASECMP.OBJ "''cc_options' STRCASECMP.C"    STRCASECMP.C
$ CALL MAKE STRDUP.OBJ	   "''cc_options' STRDUP.C"	   STRDUP.C
$!
$ write sys$output "Building XPILOT Client Image"
$ CALL MAKE XPILOT.EXE "LINK/EXE=XPILOT.exe XPILOT.opt/OPT" *.OBJ
$ rename xpilot.exe xpilot_'cpu'.exe
$!
$ write sys$output "Compiling XPILOT Replay"
$ CALL MAKE XP-REPLAY.OBJ    "''cc_options' XP-REPLAY.C"     XP-REPLAY.C
$ CALL MAKE BUTTONS.OBJ      "''cc_options' BUTTONS.C"       BUTTONS.C
$ CALL MAKE GETTIMEOFDAY.OBJ "''cc_options' GETTIMEOFDAY.C"  GETTIMEOFDAY.C
$!
$ write sys$output "Building XPILOT Replay Image"
$ set message/notext/nofacility/noseverity/noidentification
$ CALL MAKE XP-REPLAY.EXE "LINK/EXE=XP-REPLAY XP-REPLAY.opt/OPT" *.OBJ
$ set message/text/facility/severity/identification
$ rename xp-replay.exe xp-replay_'cpu'.exe
$!
$ purge/noconfirm/nolog
$ set noon
$ set prot=(s:rwed,o:rwed,w:re,g:re) *.exe
$ exit
$!
$ Clobber:	! Delete executables, Purge dir and clean up obj and lis
$ delete/noconfirm/log *.'exe';*
$!
$ Clean:	! Purge directory, clean up object files and listings
$ purge/noconfirm/nolog
$ set noon
$ delete/noconfirm/log *.obj;*
$ delete/noconfirm/log *.map;*
$ set message/notext/nofacility/noseverity/noidentification
$ delete/noconfirm/nolog *.lis;*
$ delete/noconfirm/nolog *.dia;*
$ set message/text/facility/severity/identification
$ set prot=(s:rwed,o:rwed,w:re,g:re) *.exe
$!
$exit:
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$  V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$	Argument = P'arg
$	If Argument .Eqs. "" Then Goto Exit
$	El=0
$Loop2:
$	File = F$Element(El," ",Argument)
$	If File .Eqs. " " Then Goto Endl
$	AFile = ""
$Loop3:
$	OFile = AFile
$	AFile = F$Search(File)
$	If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$	If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$	Goto Loop3
$NextEL:
$	El = El + 1
$	Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ Set Verify
$ 'P2
$ VV='F$Verify(0)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
