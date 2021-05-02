
# Makefile for TkXpInterface-3.1

# Where is wish installed ( Tcl path and filename) ?
WISH = /usr/local/bin/wish

# Common prefix for installation.
PREFIX = /users/avatar

# Directory where TkXpInterface is to be installed.
BINDIR = $(PREFIX)/bin

# Where do you want the manpage?
# Assuming that someone is actually going to bother reading it. ;-)
MANDIR = $(PREFIX)/man/man1

# Where are the xpilot maps on this system?
MAPDIR = /usr/local/bin/xpilot/maps

# Directory to put the temporary file of list server  ?
TEMPDIR = /users/avatar

# Adress of the non-interactive meta-server (with port number)
METASERVER = xpilot.cs.uit.no 4401

# Path and filename of the xpilot default file
DEFAULT = /usr/local/bin/xpilot/defaults

# Map to charge by default (without extension)
MAP = globe

# motd file to charge by default 
MOTDFILE=

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

# Which flavour install program do we like best. If all else fails try cp.
INSTALL = cp

#######################  Don't change the lines below ####################

all: TkXpInterface TkXpInterface.1

TkXpInterface: src/TkXpInterface
	(sed -e 's;WISH;$(WISH);' \
	-e 's;MAPDIR;$(MAPDIR);' \
	-e 's;TEMPDIR;$(TEMPDIR);' \
	-e 's;DEFAULT;$(DEFAULT);' \
	-e 's;MAP;$(MAP);' \
	-e 's;MOTDFILE;$(MOTDFILE);' \
	-e 's;ZEXT;$(ZEXT);' \
	-e 's;ZCOMP;$(ZCOMP);' \
	-e 's;ZDCOMP;$(ZDCOMP);' \
	-e 's;METASERVER;$(METASERVER);' src/TkXpInterface > TkXpInterface;\
	chmod u+x TkXpInterface)

TkXpInterface.1: src/TkXpInterface.1
	(sed -e 's;WISH;$(WISH);' \
	-e 's;FRAMES;$(FRAMES);' \
	-e 's;MAPDIR;$(MAPDIR);' \
	-e 's;TEMPDIR;$(TEMPDIR);' \
	-e 's;DEFAULT;$(DEFAULT);' \
	-e 's;MAP;$(MAP);' \
	-e 's;MOTDFILE;$(MOTDFILE);' \
	-e 's;ZEXT;$(ZEXT);' \
	-e 's;ZCOMP;$(ZCOMP);' \
	-e 's;ZDCOMP;$(ZDCOMP);' \
	-e 's;METASERVER;$(METASERVER);' src/TkXpInterface.1 > TkXpInterface.1 )

install: install.TkXpInterface install.man

install.TkXpInterface: TkXpInterface
	$(INSTALL) TkXpInterface $(BINDIR) ; chmod 700 $(BINDIR)/TkXpInterface

install.man: TkXpInterface.1
	$(INSTALL) TkXpInterface.1 $(MANDIR)

clean:
	rm -f TkXpInterface TkXpInterface.1
