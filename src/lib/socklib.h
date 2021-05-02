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
 * RCS:      $Header: /users/staff/bjoerns/src/cvs/xpilot/src/lib/socklib.h,v 1.1 1993/03/09 14:33:27 kenrsc Exp $
 * Log:      $Log: socklib.h,v $
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

#if(hpux)
#pragma HP_ALIGN NATURAL
#endif

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
extern int
    sl_errno,
    sl_timeout_s,
    sl_timeout_us,
    sl_default_retries,
    sl_broadcast_enabled;
extern struct sockaddr_in
    sl_dgram_lastaddr;
#ifdef __STDC__
extern void	SetTimeout(int, int);
extern int	CreateServerSocket(int);
extern int	GetPortNum(int);
extern int	CreateClientSocket(char *, int);
extern int	SocketAccept(int);
extern int	SocketReadable(int);
extern int	SocketRead(int, char *, int);
extern int	SocketWrite(int, char *, int);
extern int	SocketClose(int);
extern int	CreateDgramSocket(int);
extern int	DgramSend(int, char *, int, char *, int);
extern int	DgramReceiveAny(int, char *, int);
extern int	DgramReceive(int, char *, char *, int);
extern int	DgramSendRec(int, char *, int, char *, int, char *, int);
extern char	*DgramLastaddr(void);
extern int	DgramLastport(void);
#else /* __STDC__ */
extern void	SetTimeout();
extern int	CreateServerSocket();
extern int	GetPortNum();
extern int	CreateClientSocket();
extern int	SocketAccept();
extern int	SocketReadable();
extern int	SocketRead();
extern int	SocketWrite();
extern int	SocketClose();
extern int	CreateDgramSocket();
extern int	DgramSend();
extern int	DgramReceiveAny();
extern int	DgramReceive();
extern int	DgramSendRec();
extern char	*DgramLastaddr();
extern int	DgramLastport();
#endif /* __STDC__ */
#endif /* _SOCKLIB_LIBSOURCE */
#endif /* _SOCKLIB_INCLUDED */
