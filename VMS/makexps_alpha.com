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
$ cc_options = "/define=(VMS)/stand=vaxc"
$!
$!  Get the linker options
$!
$!  Compile the "C" files
$!
$! procedure target	command 			depends upon
$! CALL MAKE .OBJ	"CC ''cc_options' .C"	.C
$!
$ write sys$output "Compiling XPILOTS Server"
$ CALL MAKE SERVER.OBJ	"CC ''cc_options' SERVER.C"	SERVER.C
$ CALL MAKE EVENT.OBJ	"CC ''cc_options' EVENT.C"	EVENT.C
$ CALL MAKE MAP.OBJ	"CC ''cc_options' MAP.C"	MAP.C
$ CALL MAKE CMDLINE.OBJ	"CC ''cc_options' CMDLINE.C"	CMDLINE.C
$ CALL MAKE NETSERVER.OBJ	"CC ''cc_options' NETSERVER.C" NETSERVER.C
$ CALL MAKE PLAY.OBJ	"CC ''cc_options' PLAY.C"	PLAY.C
$ CALL MAKE PLAYER.OBJ	"CC ''cc_options' PLAYER.C"	PLAYER.C
$ CALL MAKE RULES.OBJ	"CC ''cc_options' RULES.C"	RULES.C
$ CALL MAKE UPDATE.OBJ	"CC ''cc_options' UPDATE.C"	UPDATE.C
$ CALL MAKE COLLISION.OBJ	"CC ''cc_options' COLLISION.C" COLLISION.C
$ CALL MAKE FRAME.OBJ	"CC ''cc_options' FRAME.C"	FRAME.C
$ CALL MAKE ROBOT.OBJ	"CC ''cc_options' ROBOT.C"	ROBOT.C
$ CALL MAKE OPTION.OBJ	"CC ''cc_options' OPTION.C"	OPTION.C
$ CALL MAKE TIMER.OBJ	"CC ''cc_options' TIMER.C"	TIMER.C
$ CALL MAKE SAUDIO.OBJ	"CC ''cc_options' SAUDIO.C"	SAUDIO.C
$ CALL MAKE GETTIMEOFDAY.OBJ "CC ''cc_options' GETTIMEOFDAY.C" GETTIMEOFDAY.C
$!
$ write sys$output "Building XPILOTS Server Image"
$ CALL MAKE XPILOTS.EXE "LINK/EXE=XPILOTS.exe/MAP/BRIEF XPILOTS_alpha.opt/OPT" *.OBJ
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
