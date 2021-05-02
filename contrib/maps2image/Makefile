#
# Makefile for maps2image.
#
# Change the following line to reflect your ANSI C compiler
#
CC	=	gcc
CFLAGS	=	-O
OBJS	=	maps2image.o
LIBS	=	-lm
TARGET	=	maps2image

#
# Change the FONT macro to point to whatever font you want
# to use to display the map names below the images.
# You can choose between: helvR08.bdf, helvR10.bdf, helvR12.bdf or helvR14.bdf.
#
FONT	=	helvR08.bdf

all:		$(FONT) font.h maps2image bdf2c

$(TARGET):	$(OBJS) font.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

font.h:	bdf2c Makefile
	bdf2c $(FONT) >font.h

bdf2c:	bdf2c.o
	$(CC) $(CFLAGS) -o bdf2c bdf2c.o

install:
	@echo "Sorry, I am too lazy to install.  Do it yourself."

depend:
	makedepend -- $(CFLAGS) -- $(OBJS:%.o=%.c)

shar:
	shar -c -p README COPYRIGHT Makefile maps2image.c bdf2c.c -B helvR*.bdf >maps2image.shar

