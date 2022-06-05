AM_CPPFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/client

bin_PROGRAMS = xpilot-replay

xpilot_replay_SOURCES = buttons.c xpilot-replay.c

xpilot_replay_LDADD = -lX11 -lm ../common/libxpcommon.a
