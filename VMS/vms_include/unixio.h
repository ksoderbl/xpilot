#ifndef __UNIXIO_LOADED
#define __UNIXIO_LOADED	1

/*	UNIXIO - V3.0 - Prototypes for UNIX emulation functions	*/

/* Changed for XPilot 3.0 */

#pragma nostandard
#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#pragma standard

/*
int	access	(char *file_spec, int mode);
int	close	(int file_desc);
int	creat	(char *file_spec, unsigned int mode, ...);
int	delete	(const char *file_spec);
int	dup	(int file_desc);
int	dup2	(int file_desc1, int file_desc2);
int	fstat	(int file_desc, stat_t *buffer);
char   *getname	(int file_desc, char *buffer, ...);
int	isapipe	(int file_desc);
int	isatty	(int file_desc);
int	lseek	(int file_desc, int offset, int direction);
char   *mktemp	(char *template);
int	read	(int file_desc, void *buffer, int nbytes);
int	rename	(const char *old_file_spec, const char *new_file_spec);
int	open	(char *file_spec, int flags, unsigned int mode, ...);
int	stat	(char *file_spec, stat_t *buffer);
char   *ttyname	(void);
int	write	(int file_desc, void *buffer, int nbytes);
*/

#endif					/* __UNIXIO_LOADED */
