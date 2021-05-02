$ SAVE_VERIFY='F$VERIFY(0)
$ if p1 .Eqs. "CLEAN" then goto clean
$ if p1 .Eqs. "CLOBBER" then goto clobber
$!
$!	Compile the Xdaliclock Application, adapted for xpilots
$!
$ Set Symbol/Scope=NoGlobal
$!
$!  Get the compiler options
$!
$! define sys sys$library/nolog
$ define X11 decw$include/nolog
$ cc_options = "/standard=ansi/nocasehack/define=(ALPHA,NO_X_GBLS)/include=[.vms_include]"
$!
$!  Get the linker options
$!
$!  Compile the "C" files
$!
$! procedure target	command 			depends upon
$! CALL MAKE .OBJ	"CC ''cc_options' .C"	.C
$!
$ write sys$output "Compiling XPILOTS Server"
$ CALL MAKE SERVER.OBJ	"GCC ''cc_options' SERVER.C"	SERVER.C
$ CALL MAKE EVENT.OBJ	"GCC ''cc_options' EVENT.C"	EVENT.C
$ CALL MAKE MAP.OBJ	"GCC ''cc_options' MAP.C"	MAP.C
$ CALL MAKE CMDLINE.OBJ	"GCC ''cc_options' CMDLINE.C"	CMDLINE.C
$ CALL MAKE NETSERVER.OBJ	"GCC ''cc_options' NETSERVER.C" NETSERVER.C
$ CALL MAKE PLAY.OBJ	"GCC ''cc_options' PLAY.C"	PLAY.C
$ CALL MAKE PLAYER.OBJ	"GCC ''cc_options' PLAYER.C"	PLAYER.C
$ CALL MAKE RULES.OBJ	"GCC ''cc_options' RULES.C"	RULES.C
$ CALL MAKE UPDATE.OBJ	"GCC ''cc_options' UPDATE.C"	UPDATE.C
$ CALL MAKE COLLISION.OBJ	"GCC ''cc_options' COLLISION.C" COLLISION.C
$ CALL MAKE FRAME.OBJ	"GCC ''cc_options' FRAME.C"	FRAME.C
$ CALL MAKE ROBOT.OBJ	"GCC ''cc_options' ROBOT.C"	ROBOT.C
$ CALL MAKE OPTION.OBJ	"GCC ''cc_options' OPTION.C"	OPTION.C
$ CALL MAKE TIMER.OBJ	"GCC ''cc_options' TIMER.C"	TIMER.C
$ CALL MAKE SAUDIO.OBJ	"GCC ''cc_options' SAUDIO.C"	SAUDIO.C
$ CALL MAKE GETTIMEOFDAY.OBJ "CC GETTIMEOFDAY.C" GETTIMEOFDAY.C
$!
$ write sys$output "Building XPILOTS Server Image"
$ CALL MAKE XPILOTS.EXE "LINK/EXE=XPILOTS.exe/MAP/BRIEF XPILOTS_vax.opt/OPT" *.OBJ
$!
$ exit
$!
$ Clobber:	! Delete executables and clean up object files and listings
$ Delete/noconfirm/log *.'exe';*
$!
$ Clean:	! Purge directory, clean up object files and listings
$ Purge
$ Delete/noconfirm/log *.lis;*
$ Delete/noconfirm/log *.obj;*
$!
$ exit
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
