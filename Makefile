all: game

game: src/Makefile
	cd src; make

src/Makefile: src/Imakefile
	cd src; xmkmf; make Makefiles

install: src/Makefile
	cd src; make install install.man

clean: src/Makefile
	cd src; make clean
	/bin/rm -f src/Makefile src/lib/Makefile

install.man:
	echo "Er, don't need to specify install.man :-)"
