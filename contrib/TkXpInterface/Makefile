# Makefile for TkXpInterface-3.8.0

# Your TOP directory to install TkXpInterface
TOPDIR = /usr/local

# Where is wish installed ( Tcl path and filename) ?
WISH = /usr/local/bin/wish

# Directory where TkXpInterface is to be installed.
BINDIR = $(TOPDIR)/bin

# Where do you want the manpage?
# Assuming that someone is actually going to bother reading it. ;-)
MANDIR = $(TOPDIR)/man/man1

# Where are the xpilot maps on this system?
MAPDIR = $(TOPDIR)/lib/xpilot/maps

# Directory to put the temporary file of list server  ?
TEMPDIR = /tmp

# Directory to put the help and the shipshapes files (TkXpI.hlp,TkXpI.shp) ?
LIBDIR = $(TOPDIR)/lib/xpilot/tkxpi

# Path and filename of the xpilot default file
DEFAULT = $(TOPDIR)/lib/xpilot/defaults

# Map to charge by default (without extension)
MAP = globe

# motd file with its pathname to charge by default
MOTDFILE= $(TOPDIR)/lib/xpilot/servermotd

# file with its pathname where xpilots dumps the scores.
# You can set now this option BUT xpilots-3.2.6 and lower doesn't
# take it into account !
SCOREFILE=

# Adress of the non-interactive meta-server (with port number)
METASERVER = meta.xpilot.org 4401

# Filter of the remote servers list. You must set at least the 5 first
# characters of the Xpilots version (dots included).
# All remote servers whose have theire version lexically and strictly lower
# than METAFILTER will be rejected.
# This value must be the limit between 2 incompatible xpilots versions.
METAFILTER=3.1.0

# Value of the timeout or of the counter during a Ping test on a remote server ?
# (when you click with the middle-button on the remote host).
# see the man page of the ping command to have the right syntaxe !
# DON'T CHANGE the "$$remoteServer" word but move it in the right position
# in function of the position of the ping's options.
# Fix a value for the counter or the timeout to avoid an infinite loop !
#
# Examples with a couter equals to 3 (or timeout 3 secondes) and the packet size = 64
# Please, this value must be lower than 5 to reduce net and host load.
#PINGOPTIONS= -c 3 -s 64 $$remoteServer
#
# or personally my work-station uses :
PINGOPTIONS= $$remoteServer 64 3


# This (optional) option will be ALREADY added on the command line of
#the xpilots's client. Type "xpilot -help" to have the avalable options.
#Example :
#
#CLIENTOPTIONS= -geometry 100x100 -noLocalMotd -mono
#
# Don't forget if more than 1 player use TkXpInterface they will have all the
# same options ! so don't add for instance "-name Avatar"
# personally I use... nothing
CLIENTOPTIONS=


# Map's Compressor you want to use (extension, compressor, decompressor)
# The GNU gzip
ZEXT= .gz
ZCOMP= gzip -f
ZDCOMP= gzip -f -d
# or this standard UNIX compressor
#ZEXT= .Z
#ZCOMP= compress -f
#ZDCOMP= uncompress -f 
# or Nothing !
#ZEXT=
#ZCOMP=
#ZDCOMP=

####### And finally the optionnal options ####################
# Customize your own colors !
# If you haven't a color display TkXpInterface doesn't take into
#account the following values.
#Please, verify if the following colors are existing in your
# /usr/lib/X11/rgb.txt or /usr/local/lib/X11/rgb.txt or
# /X11/R5/lib/X11/rgb.txt or /X11/R4/lib/rgb/rgb.txt files.
#COLORBAK = general bacground color
#COLORACT = general foreground color (texts)
#COLORDEA = foreground color of deactivated scales and scale texts.
#COLORSCA = bacground color of scale.
#COLORBUT = color of the activated buttons (when there are selected)
COLORBAK= SteelBlue
COLORACT= white
COLORDEA= black
COLORSCA= Gray
COLORBUT= red

#To search for some sub-strings in the list of remote servers,
#it's useful if you want to look for a friend (with his nick 
#name or e-mail adress) and a map name, etc....
#-No difference between lower and uppercase letters.
#-Each word must have at least 3 caracters.
#-The words are in a decreased order.
#-You can change these words when you use TkXpInterface
WHOWHAT= 


# Which flavour install program do we like best. If all else fails try cp.
INSTALL= install

#######################  Don't change the lines below ####################

VERSION= 3.8.0

SEDOPT= -e 's;WISH;$(WISH);' \
	-e 's;MAPDIR;$(MAPDIR);' \
	-e 's;TEMPDIR;$(TEMPDIR);' \
	-e 's;DEFAULT;$(DEFAULT);' \
	-e 's;MAP;$(MAP);' \
	-e 's;MOTDFILE;$(MOTDFILE);' \
	-e 's;SCOREFILE;$(SCOREFILE);' \
	-e 's;ZEXT;$(ZEXT);' \
	-e 's;ZCOMP;$(ZCOMP);' \
	-e 's;ZDCOMP;$(ZDCOMP);' \
	-e 's;VERSION;$(VERSION);' \
	-e 's;LIBDIR;$(LIBDIR);' \
	-e 's;PINGOPTIONS;$(PINGOPTIONS);' \
	-e 's;CLIENTOPTIONS;$(CLIENTOPTIONS);' \
	-e 's;METAFILTER;$(METAFILTER);' \
	-e 's;METASERVER;$(METASERVER);' \
	-e 's;WHOWHAT;$(WHOWHAT);' \
	-e 's;COLORBAK;$(COLORBAK);' \
	-e 's;COLORACT;$(COLORACT);' \
	-e 's;COLORDEA;$(COLORDEA);' \
	-e 's;COLORSCA;$(COLORSCA);' \
	-e 's;COLORBUT;$(COLORBUT);' 


all: TkXpInterface TkXpInterface.1 TkXpI.hlp

TkXpInterface: src/TkXpInterface Makefile
	sed $(SEDOPT) src/TkXpInterface > TkXpInterface;\
	chmod u+x TkXpInterface

TkXpInterface.1: src/TkXpInterface.1 Makefile
	sed $(SEDOPT) src/TkXpInterface.1 > TkXpInterface.1

TkXpI.hlp: src/TkXpI.hlp Makefile
	sed $(SEDOPT) src/TkXpI.hlp > TkXpI.hlp
	@echo ""
	@echo "Now type : make install "

install: install.TkXpInterface install.man install.lib

install.TkXpInterface: TkXpInterface
	$(INSTALL) TkXpInterface $(BINDIR) ; chmod 755 $(BINDIR)/TkXpInterface

install.man: TkXpInterface.1
	$(INSTALL) TkXpInterface.1 $(MANDIR) ; chmod 644 $(MANDIR)/TkXpInterface.1

install.lib: TkXpI.hlp
	$(INSTALL) src/TkXpI.shp $(LIBDIR) ; chmod 644 $(LIBDIR)/TkXpI.shp
	$(INSTALL) TkXpI.hlp $(LIBDIR) ; chmod 644 $(LIBDIR)/TkXpI.hlp
	@echo ""
	@echo " Now type : make clean "

clean:
	rm -f TkXpInterface TkXpInterface.1 TkXpI.hlp
	@echo ""
	@echo " Excellent ! now type    : TkXpInterface "
	@echo " or to have a short help : TkXpInterface -info "
	@echo ""
	@echo " You must install the LAST version of maps2image !"
	@echo " If you type : maps2image -help   you must obtain : "
	@echo "   maps2image: illegal option -- h"
	@echo "   usage: maps2image [-s size][-x nmaps][-i][-v][-l] xpmap ..."
	@echo "  (The -l option MUST be available)
	@echo "  If not, install the ./contrib/maps2image.sh file"
	@echo ""
	@echo " type :   nroff -man $(MANDIR)/TkXpInterface.1 | more "
	@echo ""
	@echo "Enjoy !!!!!!!"

