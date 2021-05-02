/* -*-C-*-
 *
 * Project :	 TRACE
 *
 * File    :	 socklib.h
 *
 * Description
 *
 * Copyright (C) 1991 by Arne Helme, The TRACE project
 *
 * Rights to use this source is granted for all non-commercial and research
 * uses. Creation of derivate forms of this software may be subject to
 * restriction. Please obtain written permission from the author.
 *
 * This software is provided "as is" without any express or implied warranty.
 *
 * RCS:      $Header: /zen/net/tromsoe/cvsroot/xpilot/src/socklib.h,v 3.11 1994/07/10 20:04:26 bert Exp $
 * Log:      $Log: socklib.h,v $
 * Revision 3.11  1994/07/10  20:04:26  bert
 * Several small changes to:
 *     - resolve compilation problems with very strict ANSI C or C++ compilers.
 *     - resolve potential problems for systems for which a long integer
 *       is bigger than a normal integer.
 *     - resolve problems for compilers for which enumeration constants
 *       are smaller than an integer.
 * Removed lots of superfluous whitespace.
 * More changes to the new help system.
 * Different key help list window.
 *
 * Revision 3.10  1994/04/10  13:21:24  bert
 * Extended the way sockets are made non-blocking by trying three different
 * possibilities depending upon which system include file constants are defined.
 * Now fcntl(O_NDELAY), fcntl(FNDELAY) and ioctl(FIONBIO) are all tried until
 * one of them succeeds.
 * Improved GetLocalHostName for suns.
 *
 * Log:      $Log: socklib.h,v $
 * Revision 3.9  1994/03/23  08:45:00  bert
 * Fixed GetLocalHostName by adding a size parameter.
 *
 * Revision 3.8  1994/01/27  22:28:16  bert
 * Added a new call GetLocalHostName() to get the fully qualified local hostname.
 *
 * Revision 3.7  1994/01/23  14:16:03  bert
 * Added a SetSocketBroadcast() function.
 *
 * Revision 3.6  1994/01/20  21:19:48  bert
 * Changes to get the prototypes when compiling with C++.
 * Small change for C++ functionprototypes and the select system call.
 *
 * Revision 3.5  1993/10/24  22:33:59  bert
 * Added prototypes for the new DgramReply() routine.
 *
 * Revision 3.4  1993/10/21  11:11:05  bert
 * VMS patch from Curt Hjorring.
 * Removed Optimize_map() from the server.
 * Made toggleShield a new client option.
 *
 * Revision 3.3  1993/08/19  07:35:26  kenrsc
 * Added patch from bert (3f4changes)
 *
 * Revision 3.2  1993/08/02  12:51:20  bjoerns
 * Patchlevel 2.
 *
 * Revision 3.1  1993/08/02  12:41:43  bjoerns
 * Patchlevel 1.
 *
 * Revision 3.0  1993/05/21  18:36:41  bjoerns
 * New client server release.
 *
 * Revision 1.2  1993/05/18  16:49:31  kenrsc
 * Berts few changes !
 *
 * Revision 1.1  1993/04/22  10:21:33  bjoerns
 * Moved socklib from lib to src.
 *
 * Revision 1.1  1993/03/09  14:33:27  kenrsc
 * Hopefully we won't have a corrupted CVS directory anymore.
 *
 * Revision 1.1.1.1  1993/02/27  14:47:46  bjoerns
 * XPilot v2.0
 *
 * Revision 1.1.1.1  1993/01/19  17:19:59  bjoerns
 * XPilot v1.4
 *
 * Revision 1.3  1992/09/11  22:50:24  bjoerns
 * Applied NCD2 patch.
 *
 * Revision 1.2  1992/08/26  19:36:36  bjoerns
 * Incorporated NCD patch.
 *
 * Revision 1.1.1.1  1992/05/11  12:32:34  bjoerns
 * XPilot v1.0
 *
 * Revision 1.2  91/10/02  08:38:20  08:38:20  arne (Arne Helme)
 * "ANSI C prototypes added."
 *
 * Revision 1.1  91/10/02  08:34:53  08:34:53  arne (Arne Helme)
 * Initial revision
 *
 */

#ifndef _SOCKLIB_INCLUDED
#define _SOCKLIB_INCLUDED

/* Error values and their meanings */
#define SL_ESOCKET		0	/* socket system call error */
#define SL_EBIND		1	/* bind system call error */
#define SL_ELISTEN		2	/* listen system call error */
#define SL_EHOSTNAME		3	/* Invalid host name format */
#define SL_ECONNECT		5	/* connect system call error */
#define SL_ESHUTD		6	/* shutdown system call error */
#define SL_ECLOSE		7	/* close system call error */
#define SL_EWRONGHOST		8	/* message arrived from unspec. host */
#define SL_ENORESP		9	/* No response */
#define SL_ERECEIVE		10	/* Receive error */

#ifndef _SOCKLIB_LIBSOURCE
#ifdef VMS
#include <in.h>			/* for sockaddr_in */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN		64
#endif
#else
#include <netinet/in.h>			/* for sockaddr_in */
#endif
extern int
    sl_errno,
    sl_timeout_s,
    sl_timeout_us,
    sl_default_retries,
    sl_broadcast_enabled;
extern struct sockaddr_in
    sl_dgram_lastaddr;

#endif /* _SOCKLIB_LIBSOURCE */

#ifdef __cplusplus
#ifndef __STDC__
#define __STDC__	1
#endif
#endif

#ifdef __STDC__
extern void	SetTimeout(int, int);
extern int	CreateServerSocket(int);
extern int	GetPortNum(int);
extern char	*GetSockAddr(int);
extern int	GetPeerName(int, char *, int);
extern int	CreateClientSocket(char *, int);
extern int	SocketAccept(int);
extern int	SocketLinger(int);
extern int	SetSocketReceiveBufferSize(int, int);
extern int	SetSocketSendBufferSize(int, int);
extern int	SetSocketNoDelay(int, int);
extern int	SetSocketNonBlocking(int, int);
extern int	SetSocketBroadcast(int, int);
extern int	GetSocketError(int);
extern int	SocketReadable(int);
extern int	SocketRead(int, char *, int);
extern int	SocketWrite(int, char *, int);
extern int	SocketClose(int);
extern int	CreateDgramSocket(int);
extern int	DgramConnect(int, char *, int);
extern int	DgramSend(int, char *, int, char *, int);
extern int	DgramReceiveAny(int, char *, int);
extern int	DgramReceive(int, char *, char *, int);
extern int	DgramReply(int, char *, int);
extern int	DgramSendRec(int, char *, int, char *, int, char *, int);
extern char	*DgramLastaddr(void);
extern char	*DgramLastname(void);
extern int	DgramLastport(void);
extern void	GetLocalHostName(char *, unsigned);
#else /* __STDC__ */
extern void	SetTimeout();
extern int	CreateServerSocket();
extern int	GetPortNum();
extern char	*GetSockAddr();
extern int	GetPeerName();
extern int	CreateClientSocket();
extern int	SocketAccept();
extern int	SocketLinger();
extern int	SetSocketReceiveBufferSize();
extern int	SetSocketSendBufferSize();
extern int	SetSocketNoDelay();
extern int	SetSocketNonBlocking();
extern int	SetSocketBroadcast();
extern int	GetSocketError();
extern int	SocketReadable();
extern int	SocketRead();
extern int	SocketWrite();
extern int	SocketClose();
extern int	CreateDgramSocket();
extern int	DgramConnect();
extern int	DgramSend();
extern int	DgramReceiveAny();
extern int	DgramReceive();
extern int	DgramReply();
extern int	DgramSendRec();
extern char	*DgramLastaddr();
extern char	*DgramLastname();
extern int	DgramLastport();
extern void	GetLocalHostName();
#endif /* __STDC__ */

#if !defined(select) && defined(__linux__)
#define select(N, R, W, E, T)	select((N),		\
	(fd_set*)(R), (fd_set*)(W), (fd_set*)(E), (T))
#endif

#endif /* _SOCKLIB_INCLUDED */
