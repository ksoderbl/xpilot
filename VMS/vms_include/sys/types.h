/*
 *	This is the UNIX-compatible Types definition
 */

#ifndef _TYPES_
#define	_TYPES_
/*
 * Basic system types
 */
#ifndef  __SOCKET_TYPEDEFS
#define  __SOCKET_TYPEDEFS
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned long	u_long;
#endif
typedef  unsigned int   u_int;
typedef	unsigned short	ushort;		/* sys III compat */

typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
#ifndef CADDR_T
#define CADDR_T
typedef	char *	caddr_t;
#endif

typedef	u_short	ino_t;
typedef	char	*dev_t;
typedef	unsigned off_t;

typedef	long	swblk_t;
#ifndef _SIZE_T
#define _SIZE_T
typedef	unsigned int	size_t;
#endif
#ifndef	TYPES_H_DEFINED
#include <types.h>	/* This gets us time_t */
#endif	TYPES_H_DEFINED
typedef	u_short	uid_t;
typedef	u_short	gid_t;

#define	NBBY	8		/* number of bits in a byte */

#endif	_TYPES_
