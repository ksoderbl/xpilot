############################################################################
#
# Miscellaneous flags.
#
MATHLIB = 
SOCKLIB = [.lib]libsocklib.olb

# VMS ansi C compiler
#CC = gcc/standard=ansi/nocasehack
# Alpha OpenVMS ansi C compiler
CC = cc

#DEFINES = /define=(ALPHA,NO_X_GBLS)
DEFINES =

#CDEBUGFLAGS = /opt
CDEBUGFLAGS =
# -Wl,-I first, then -Wl,-P

#CFLAGS = $(DEFINES) $(CDEBUGFLAGS) /include=[.vms_include]
CFLAGS = $(DEFINES) $(CDEBUGFLAGS)

SERVER_LFLAGS = ,link_ucx.opt/opt
CLIENT_LFLAGS = ,link_ucx.opt/opt,link_xlib.opt/opt

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


#
# 'Server' program, xpilots, object and source files
#
OBJS1 = server.obj, event.obj, map.obj, math.obj, cmdline.obj, net.obj, netserver.obj, -
        play.obj, player.obj, rules.obj, update.obj, collision.obj, error.obj, frame.obj, -
        robot.obj, option.obj, socklib.obj, timer.obj, saudio.obj, username.obj, -
        ioctl.obj, $(USLEEP_OBJ), $(STRCASECMP_OBJ), $(STRDUP_OBJ), $(TIME_OBJ)
SRCS1 = server.c event.c map.c math.c cmdline.c net.c netserver.c -
        play.c player.c rules.c update.c collision.c error.c frame.c -
        robot.c option.c socklib.c timer.c saudio.c ioctl.c
#
# Client program, xpilot...
#
OBJS2 = xpilot.obj, client.obj, join.obj, net.obj, netclient.obj, paint.obj, dbuff.obj, xinit.obj, -
        default.obj, math.obj, xevent.obj, syslimit.obj, error.obj, socklib.obj, caudio.obj, -
        widget.obj, configure.obj, username.obj, ioctl.obj, -
        $(USLEEP_OBJ), $(STRCASECMP_OBJ), $(STRDUP_OBJ) $(AUDIOOBJ)
SRCS2 = xpilot.c client.c join.c net.c netclient.c paint.c dbuff.c xinit.c -
        default.c math.c xevent.c syslimit.c error.c socklib.c caudio.c -
        widget.c configure.c username.c ioctl.c $(AUDIOSRC)



############################################################################
#
# Make subdirectories, socklib...
#
SUBDIRS = lib


all	:	xpilot.exe xpilots.exe

xpilots	:	xpilots.exe

xpilot	:	xpilot.exe

gc	:	descrip.mms
	set command gnu_cc:[000000]gcc

xpilots.exe	: $(OBJS1)
	link/exec=$@ $(OBJS1) $(SERVER_LFLAGS)

xpilot.exe	: $(OBJS2)
	link/exec=$@ $(OBJS2) $(CLIENT_LFLAGS)


username.obj	:	username.c username.h
# can't use gcc because of $'s in function names
	cc username

ioctl.obj		:	ioctl.c
	cc ioctl

socklib.obj		:	socklib.c socklib.h
	cc socklib

strcasecmp.obj		:	strcasecmp.c strcasecmp.h
	cc strcasecmp

gettimeofday.obj	:	gettimeofday.c gettimeofday.h
	cc gettimeofday

usleep.obj		:	usleep.c
	cc usleep

