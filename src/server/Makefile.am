AM_CPPFLAGS = -I$(top_srcdir)/src/common

bin_PROGRAMS = xpilot-server

xpilot_server_SOURCES = alliance.c asteroid.c cannon.c cell.c cmdline.c collision.c command.c contact.c event.c fileparser.c frame.c id.c item.c laser.c map.c metaserver.c netserver.c object.c objpos.c option.c parser.c play.c player.c robot.c robotdef.c rules.c saudio.c sched.c score.c server.c ship.c shot.c showtime.c stratbot.c tuner.c update.c walls.c wildmap.c

xpilot_server_LDADD = -lm ../common/libxpcommon.a
