############################################################################
#
# Miscellaneous flags.
#
MATHLIB = 
SOCKLIB = [.lib]libsocklib.olb

############################################################################
# DECC Compiler for OpenVMS/AXP 7.0 
#
CC = cc/decc/standard=vaxc/precision=single
DEFINES = /define=(VMS)
CDEBUGFLAGS = /optimize=(level=4)
CPREF = /prefix=(ALL,EXCEPT=(ioctl,strcasecmp,strncasecmp,strdup,gettimeofday))
CFLAGS = $(DEFINES) $(CDEBUGFLAGS) $(CPREF)

############################################################################
# DECC Compiler for OpenVMS/VAX 7.0
#
#CC = cc/decc/standard=vaxc/precision=single
#DEFINES = /define=(VMS,VAX)
#CDEBUGFLAGS = /optimize=(disjoint,inline)
#CPREF = /prefix=(ALL,EXCEPT=(ioctl,strcasecmp,strncasecmp,strdup,gettimeofday))
#CFLAGS = $(DEFINES) $(CDEBUGFLAGS) $(CPREF)

############################################################################
# OpenVMS ansi C compiler GCC  
#
#CC = gcc/standard=ansi/nocasehack
#DEFINES = /define=(ALPHA,NO_X_GBLS)
#CDEBUGFLAGS = /opt
#CFLAGS = $(DEFINES) $(CDEBUGFLAGS) 


.c.obj	:
	$(CC) $(CFLAGS) $*.c


############################################################################
#
# Source and object files.
#
USLEEP_OBJ = usleep.obj
STRCASECMP_OBJ = strcasecmp.obj
STRDUP_OBJ = strdup.obj
TIME_OBJ = gettimeofday.obj

SERVER_LFLAGS = $(STRCASECMP_OBJ), $(STRDUP_OBJ), xpilots.opt/opt

CLIENT_LFLAGS = xpilot.opt/opt

#
# 'Server' program, xpilots, object and source files
#
OBJS1 = server.obj, event.obj, map.obj, math.obj, cmdline.obj, net.obj, - 
        netserver.obj, play.obj, player.obj, rules.obj, update.obj, - 
        collision.obj, walls.obj, error.obj, frame.obj, robot.obj, robotdef.obj, -
        option.obj, socklib.obj, sched.obj, saudio.obj, username.obj, -
        trnlnm.obj, ioctl.obj, contact.obj, metaserver.obj, id.obj, objpos.obj, -
        $(STRCASECMP_OBJ), $(STRDUP_OBJ)
SRCS1 = server.c event.c map.c math.c cmdline.c net.c netserver.c -
        play.c player.c rules.c update.c collision.c walls.c error.c frame.c -
        robot.c robotdef.c option.c socklib.c sched.c saudio.c username.c -
        trnlnm.c  ioctl.c contact.c metaserver.c id.c objpos.c
#
# 'Client' program, xpilot...
#
OBJS2 = xpilot.obj, client.obj, join.obj, net.obj, netclient.obj, paint.obj, -
        paintdata.obj, painthud.obj, paintmap.obj, paintobjects.obj, paintradar.obj, -
        query.obj, dbuff.obj, xinit.obj, default.obj, math.obj, xevent.obj, syslimit.obj, -
        error.obj, socklib.obj, configure.obj, widget.obj, caudio.obj, -
        record.obj, texture.obj, xpmread.obj, username.obj, trnlnm.obj, -
        ioctl.obj, about.obj, colors.obj, talk.obj, -
        $(USLEEP_OBJ), $(STRCASECMP_OBJ), $(STRDUP_OBJ)
SCRS2 = xpilot.c client.c join.c net.c netclient.c paint.c -
        paintdata.c painthud.c paintmap.c paintobjects.c paintradar.c -
        query.c dbuff.c xinit.c default.c math.c xevent.c syslimit.c -
        error.c socklib.c configure.c widget.c caudio.c -
        record.c texture.c xpmread.c username.c trnlnm.c -
        ioctl.c about.c colors.c talk.c
#
# 'XP-Replay' program, xp-replay...
#
OBJS3 = xp-replay.obj, buttons.obj, $(TIME_OBJ)
SCRS3 = xp-replay.c buttons.c 


############################################################################
#
# Make subdirectories, socklib...
#

all :           xpilot xpilots xp-replay clean
                ! All Finished with the VMS build of XPilot (v3.6.3)

xpilots :       xpilots.exe
                @ Continue

xpilot :        xpilot.exe
                @ Continue

xp-replay :     xp-replay.exe
                @ Continue

gc :            descrip.mms
                set command gnu_cc:[000000]gcc
                @ Continue

xpilots.exe :   $(OBJS1)
                link/exec=$@ $(SERVER_LFLAGS)
                @ Continue

xpilot.exe :    $(OBJS2)
                link/exec=$@ $(CLIENT_LFLAGS)
                @ Continue

xp-replay.exe : $(OBJS3)
                link/exec=$@ $(CLIENT_LFLAGS)
                @ Continue

xpilot.obj :    xpilot.c
                set def [-.src]
                copy [-.contrib.vms]*.c  [-.src]*.c
                copy [-.contrib.vms]*.h  [-.src]*.h
                purge/nolog/noconfirm [-.src...]
                $(CC) $(CFLAGS) xpilot.c

xpilots.obj :   xpilots.c
                set def [-.src]
                copy [-.contrib.vms]*.c  [-.src]*.c
                copy [-.contrib.vms]*.h  [-.src]*.h
                purge/nolog/noconfirm [-.src...]
                $(CC) $(CFLAGS) xpilots.c

texture.obj :   texture.c texture.h
                create/dir [-.src.textures]
                copy [-.lib.textures]*.*  [-.src.textures]*.*
                purge/nolog/noconfirm [-.src.textures]
                $(CC) $(CFLAGS) texture.c

username.obj :  username.c username.h
                $(CC) $(CFLAGS) username.c

ioctl.obj :     ioctl.c
                $(CC) $(CFLAGS) ioctl.c

socklib.obj :   socklib.c socklib.h
                $(CC) $(CFLAGS) socklib.c

strcasecmp.obj : strcasecmp.c strcasecmp.h
                $(CC) $(CFLAGS) strcasecmp.c

gettimeofday.obj : gettimeofday.c gettimeofday.h
                $(CC) $(CFLAGS) gettimeofday.c

usleep.obj :    usleep.c
                $(CC) $(CFLAGS) usleep.c

clean :
                @- Set Protection = Owner:RWED *.obj,*.*;-1
                - Delete /NoConfirm /NoLog *.obj;*,*.log;*,*.lis;*,*.dia;*
                - Purge /NoConfirm /NoLog [-.src...]

