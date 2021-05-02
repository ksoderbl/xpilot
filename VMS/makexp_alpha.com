$ SAVE_VERIFY='F$VERIFY(0)
$ if p1 .Eqs. "CLEAN" then goto clean
$ if p1 .Eqs. "CLOBBER" then goto clobber
$!
$!	Compile the Xdaliclock Application, adapted for xpilot
$!
$ Set Symbol/Scope=NoGlobal
$!
$!  Get the compiler options
$!
$! define sys sys$library/nolog
$ define X11 decw$include/nolog
$ cc_options = "/define=(VMS)/stand=vaxc"
$!
$!  Get the linker options
$!
$!  Compile the "C" files
$!
$! procedure target	command 			depends upon
$! CALL MAKE .OBJ	"CC ''cc_options' .C"	.C
$!
$ write sys$output "Compiling XPILOT Client"
$ CALL MAKE XPILOT.OBJ	"CC ''cc_options' XPILOT.C"	XPILOT.C
$ CALL MAKE CLIENT.OBJ	"CC ''cc_options' CLIENT.C"	CLIENT.C
$ CALL MAKE JOIN.OBJ	"CC ''cc_options' JOIN.C"	JOIN.C
$ CALL MAKE NET.OBJ	"CC ''cc_options' NET.C"	NET.C
$ CALL MAKE NETCLIENT.OBJ	"CC ''cc_options' NETCLIENT.C"	NETCLIENT.C
$ CALL MAKE PAINT.OBJ	"CC ''cc_options' PAINT.C"	PAINT.C
$ CALL MAKE DBUFF.OBJ	"CC ''cc_options' DBUFF.C"	DBUFF.C
$ CALL MAKE XINIT.OBJ	"CC ''cc_options' XINIT.C"	XINIT.C
$ CALL MAKE DEFAULT.OBJ	"CC ''cc_options' DEFAULT.C"	DEFAULT.C
$ CALL MAKE MATH.OBJ	"CC ''cc_options' MATH.C"	MATH.C
$ CALL MAKE XEVENT.OBJ	"CC ''cc_options' XEVENT.C"	XEVENT.C
$ CALL MAKE SYSLIMIT.OBJ	"CC ''cc_options' SYSLIMIT.C"	SYSLIMIT.C
$ CALL MAKE ERROR.OBJ	"CC ''cc_options' ERROR.C"	ERROR.C
$ CALL MAKE SOCKLIB.OBJ	"CC ''cc_options' SOCKLIB.C"	SOCKLIB.C
$! commented out this as it produces no code
$! CALL MAKE CAUDIO.OBJ	"CC ''cc_options' CAUDIO.C"	CAUDIO.C
$ CALL MAKE WIDGET.OBJ	"CC ''cc_options' WIDGET.C"	WIDGET.C
$ CALL MAKE CONFIGURE.OBJ	"CC ''cc_options' CONFIGURE.C"	CONFIGURE.C
$ CALL MAKE USERNAME.OBJ	"CC ''cc_options' USERNAME.C"	USERNAME.C
$ CALL MAKE IOCTL.OBJ	"CC ''cc_options' IOCTL.C"	IOCTL.C
$ CALL MAKE USLEEP.OBJ	"CC ''cc_options' USLEEP.C"	USLEEP.C
$ CALL MAKE STRCASECMP.OBJ	"CC ''cc_options' STRCASECMP.C"	STRCASECMP.C
$ CALL MAKE STRDUP.OBJ	"CC ''cc_options' STRDUP.C"	STRDUP.C
$!
$ write sys$output "Building XPILOT Client Image"
$ CALL MAKE XPILOT.EXE "LINK/EXE=XPILOT.exe/MAP/BRIEF XPILOT_alpha.opt/OPT" *.OBJ
$!
$ exit
$!
$ Clobber:	! Delete executables, Purge dir and clean up obj and lis
$ Delete/noconfirm/log *.'exe';*
$!
$ Clean:	! Purge directory, clean up object files and listings
$ Purge
$ Delete/noconfirm/log *.lis;*
$ Delete/noconfirm/log *.obj;*
$!
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
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
