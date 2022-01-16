AM_CPPFLAGS = -I$(top_srcdir)/src/common

bin_PROGRAMS = xpilot-client

xpilot_client_SOURCES = about.c blockbitmaps.c client.c colors.c configure.c datagram.c dbuff.c default.c gfx2d.c gfx3d.c guimap.c guiobjects.c join.c ncdaudio.c netclient.c paint.c paintdata.c painthud.c paintmap.c paintobjects.c paintradar.c query.c record.c sim.c syslimit.c talk.c talkmacros.c textinterface.c texture.c usleep.c welcome.c widget.c xevent.c xeventhandlers.c xinit.c xpilot.c xpmread.c

xpilot_client_LDADD = -lX11 -lm ../common/libxpcommon.a

## compiled with CC=gcc CFLAGS=-DSOUND=1  ./configure; make clean; make
## needs libaudio-dev
## xpilot_client_LDADD = -lX11 -lm -laudio ../common/libxpcommon.a
