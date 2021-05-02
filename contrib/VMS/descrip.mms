############################################################################
#
# Miscellaneous flags.
#
MATHLIB = 
SOCKLIB = [.lib]libsocklib.olb

############################################################################
# DECC Compiler for OpenVMS/AXP 
#
CC = cc/decc/standard=vaxc/precision=single
DEFINES = /define=(VMS)
CDEBUGFLAGS = /optimize=(level=4)
CFLAGS = $(DEFINES) $(CDEBUGFLAGS) 

############################################################################
# DECC Compiler for OpenVMS/VAX 
#
#CC = cc/decc/standard=vaxc/precision=single
#DEFINES = /define=(VMS,VAX)
#CDEBUGFLAGS = /optimize=(disjoint,inline)
#CFLAGS = $(DEFINES) $(CDEBUGFLAGS) 

############################################################################
# OpenVMS ansi C compiler GCC  
#
#CC = gcc/standard=ansi/nocasehack
#DEFINES = /define=(ALPHA,NO_X_GBLS)
#CDEBUGFLAGS = /opt
#CFLAGS = $(DEFINES) $(CDEBUGFLAGS) /include=([.vms_include])

SERVER_LFLAGS = xpilots.opt/opt
CLIENT_LFLAGS = xpilot.opt/opt

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
OBJS1 = server.obj, event.obj, map.obj, math.obj, cmdline.obj, net.obj, - 
        netserver.obj, play.obj, player.obj, rules.obj, update.obj, - 
        collision.obj, error.obj, frame.obj, robot.obj, option.obj, -
        socklib.obj, timer.obj, saudio.obj, ioctl.obj, username.obj, -
        $(USLEEP_OBJ), $(STRCASECMP_OBJ), $(STRDUP_OBJ), $(TIME_OBJ)
SRCS1 = server.c event.c map.c math.c cmdline.c net.c netserver.c -
        play.c player.c rules.c update.c collision.c error.c frame.c -
        robot.c option.c socklib.c timer.c saudio.c ioctl.c
#
# Client program, xpilot...
#
OBJS2 = xpilot.obj, client.obj, join.obj, net.obj, netclient.obj, paint.obj, -
        dbuff.obj, xinit.obj, default.obj, math.obj, xevent.obj, syslimit.obj, -
        error.obj, socklib.obj, caudio.obj, widget.obj, configure.obj, -
        username.obj, ioctl.obj, record.obj, texture.obj, xpmread.obj, -
        $(USLEEP_OBJ), $(STRCASECMP_OBJ), $(STRDUP_OBJ) $(AUDIOOBJ)
SRCS2 = xpilot.c client.c join.c net.c netclient.c paint.c dbuff.c xinit.c -
        default.c math.c xevent.c syslimit.c error.c socklib.c caudio.c -
        widget.c configure.c username.c ioctl.c record.c texture.c -
        xpmread.c $(AUDIOSRC)



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
	link/exec=$@ $(SERVER_LFLAGS)

xpilot.exe	: $(OBJS2)
	link/exec=$@ $(CLIENT_LFLAGS)

xpilot.obj      :       xpilot.c 
        set def [-.src]
        create/dir [-.src.vms_include]
        copy [-.contrib.vms]*.c  [-.src]*.c
        copy [-.contrib.vms]*.h  [-.src]*.h
        copy [-.contrib.vms.vms_include]*.h  [-.src.vms_include]*.h
        copy [-.contrib.vms.vms_include.sys]*.h  [-.src.vms_include]*.h
        purge/nolog/noconfirm [-.src...]
	$(CC) $(CFLAGS) xpilot.c

xpilots.obj             :       xpilots.c 
        set def [-.src]
        create/dir [-.src.vms_include]
        copy [-.contrib.vms]*.c  [-.src]*.c
        copy [-.contrib.vms]*.h  [-.src]*.h
        copy [-.contrib.vms.vms_include]*.h  [-.src.vms_include]*.h
        copy [-.contrib.vms.vms_include.sys]*.h  [-.src.vms_include]*.h
        purge/nolog/noconfirm [-.src...]
	$(CC) $(CFLAGS) xpilots.c

texture.obj             :       texture.c texture.h
        create/dir [-.src.textures]
        copy [-.lib.textures]*.*  [-.src.textures]*.*
        purge/nolog/noconfirm [-.src.textures]
	$(CC) $(CFLAGS) texture.c

# can't use gcc because of $'s in function names
username.obj	        :	username.c username.h
	$(CC) $(CFLAGS) username.c

ioctl.obj		:	ioctl.c
	$(CC) $(CFLAGS) ioctl.c

socklib.obj		:	socklib.c socklib.h
	$(CC) $(CFLAGS) socklib.c

strcasecmp.obj		:	strcasecmp.c strcasecmp.h
	$(CC) $(CFLAGS) strcasecmp.c

gettimeofday.obj	:	gettimeofday.c gettimeofday.h
	$(CC) $(CFLAGS) gettimeofday.c

usleep.obj		:	usleep.c
	$(CC) $(CFLAGS) usleep.c

