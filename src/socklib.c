/* -*-C-*-
 *
 * Project :	 TRACE
 *
 * File    :	 socklib.c
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
 * 1993/05/22 bert
 * Changed SocketReadable to check for return value of select()
 * and changed unused fd sets to NULL pointers.
 *
 * Revision so-and-so, Id: bla-bla, Code: too.  Bert G�sbers April/Mai 93
 * Added several interface calls to get/setsockopt/ioctl calls for use
 * in the client/server version of XPilot.
 * Also, (Please Note!) I removed the linger stuff from the SocketAccept()
 * function into a separate function call.  When a socket is non-blocking
 * then lingering on close didn't seem like a good idea to me.
 *
 * RCS:      $Id: socklib.c,v 3.7 1993/08/02 12:55:37 bjoerns Exp $
 * Log:      $Log: socklib.c,v $
 * Revision 3.7  1993/08/02  12:55:37  bjoerns
 * Patchlevel 3.
 *
 * Revision 3.6  1993/08/02  12:51:18  bjoerns
 * Patchlevel 2.
 *
 * Revision 3.5  1993/08/02  12:41:41  bjoerns
 * Patchlevel 1.
 *
 * Revision 3.4  1993/06/28  20:54:22  bjoerns
 * Added an Imakefile and renamed Makefile to Makefile.std.
 * Made Bert's last name 8-bit.
 *
 * Revision 3.3  1993/06/28  11:43:36  bjoerns
 * Applied PL8 which was handled completely by Bert G�sbers.
 *
 *
 * SUMMARY OF CHANGES
 *
 * Andy Skinner contributed a new targetKillTeam option.
 * When turned on it will kill all players in the team that just
 * lost their target.  The default value is off/inactive.
 *
 * Any remaining problems  with the login sequence should be fixed
 * due to a thorough revision/rewrite of the receive-window-size code.
 * E.g., the `ufo-problem' reported by Andy should be gone.
 *
 * Player info is now transmitted and stored in a less bugprone way.
 * All problems with player names and radar visibility should
 * now be fixed.
 *
 * The flashing of the 'F' in fuelstations should now be gone.
 * The solution was to use the proper pre-xored pixel value as
 * foreground for xor operations.
 *
 * Drawing the new score objects now also work across edge wrap.
 *
 * Shots are now killed if they hit a cannon.
 *
 * In Move_smart_shot() `min' wasn't allways initialised before use.
 *
 * The PacketDrawMeter is removed (was overkill).
 *
 * The mineDraw resource is removed (was overkill).
 *
 * Mines are now drawn like they already appeared in the new
 * items and HUD instead of a small-blue-almost-invisible circle.
 *
 * Several functions have now a (void) prototype when no arguments
 * to shut up the compiler.
 *
 * The backgroundPointDist resource code is now checking for division
 * by zero.
 *
 * The printing of duplicate reliable data packet info is commented out.
 *
 * The drawing of fuelstations is buffered and optimised to reduce
 * changing of GCs.
 *
 * The '#ifdef SOUND' statement in sound.c is now on top of the sound.c file
 * like suggested in alt.games.xpilot.
 *
 * DrawShadowText() wasn't always returning a value.
 *
 * Small changes to pixmaps for Tank, Transporter and Cloak items.
 *
 * Revision 3.2  1993/06/12  20:35:37  bjoerns
 * Applied yet anoter patch from Bert, this time he has implemented clustering of objects (sparks for the moment).
 *
 * Revision 3.1  1993/05/28  18:04:02  bjoerns
 * Applied new giant patch from Bert.  Here's a summary I got from him:
 *
 * changes to Makefile:
 * 	some AIX flags
 * 	new include file dependencies
 * client.c:
 * 	Target test.
 * 	player info bug fix (already mailed to you).
 * 	More accurate RadarHeight calculation.
 * client.h:
 * 	"self_visible" boolean flag in client structure.
 * 	new prototype for Client_flush().
 * cmdline.c:
 * 	playerStartsShielded flag for PlayerShielding=false mode
 * 	playersOnRadar if players are visible on radar (default=true)
 * 	missilesOnRadar if missiles are visible on radar (default=true)
 * collision.c:
 * 	new interesting experiment with collision detection for objects,
 * 	now only test objects that have already passed the ObjectCollision
 * 	function this timeframe (simple change, should be an improvement I hope)
 * const.h:
 * 	Experiment with hypot() in LENGTH() macro, untested yet.
 * 	TARGET_DEAD_TIME is defined in some amount of FPS.
 * 	A little higher amount of TARGET_DAMAGE and TARGET_REPAIR_PER_FRAME.
 * event.c:
 * 	Changes to  Refuel() and Player_lock_closest(), I did these because
 * 	I hadn't your changes available.  Might be skipped.
 * 	Removed pl->control_count stuff as these are only needed in the client.
 * frame.c:
 * 	Added new visibility detection stuff for map data.
 * 	It didn't work in PL0 at the edges for cannons and fuelstations.
 * 	This one works perfect for cannons and fuelstations, but has a tough
 * 	to find bug for target info.  Target info is only correctly displayed
 * 	if the player ship is above the target, not below or equal.  Strange!
 * 	Changed radar stuff to incorporate the new playersOnRadar and
 * 	missilesOnRadar options.
 * global.h:
 * 	New declarations for the new commandline options.
 * join.c:
 * 	Flushing of display stuff, probably not necessary.
 * 	But David had some kind of strange problem.
 * 	May be removed.
 * net.c:
 * 	Changed ifdefs for memmove as AIX doesn't have that either.
 * 	Also, AIX does not use EWOULDBLOCK but EAGAIN instead.
 * netclient.c:
 * 	Removed Check_packet_type().  Not needed anymore.
 * 	Changed Kernel network buffer sizes ifdefs as AIX has smaller
 * 	socket buffers by default.
 * 	Added stuff for PKT_PLAY to also handle premature frames.
 * 	May evetually not be needed or in a different way.
 * netserver.c:
 * 	Big changes in states a new connection can be in.
 * 	Removed duplicate code.  Other cleanups.
 * 	A little better retransmission code + bug fix which you already have.
 * 	AIX doesn't have strdup().
 * 	More different types of packet dispatch tables.
 * 	Removed the CONN_ACCEPTED state.  Added CONN_DRAIN and CONN_READY
 * 	to wait until client has acked all reliable data.
 * 	Added war stuff for new players.
 * 	Made an upper limit of 5 for maximum input packet reads per frame.
 * 	Some functions now are static.
 * netserver.h:
 * 	See netserver.c.
 * object.h:
 * 	Removed control_count.
 * 	Added shield_time which gives the time a player still has shields left
 * 	after restart if mode is playerShielding=false and
 * 	playerStartsShielded=true.
 * paint.c:
 * 	Add drawing of transporter lines together with drawing of refuel lines
 * 	and connector lines.
 * 	Added proper control_count decrease.
 * 	New radar stuff.  Improved accuracy.  And better radar map drawing.
 * 	Changed drawing color of target damage, but I don't like it.  Damage lines
 * 	are now drawn in opposite color.  See if you dislike it as much as I do.
 * 	No more direction line of self on radar when dead, like Bjoern disliked.
 * 	I experimented somewhat with the drawing of a cross instead, but
 * 	removed it also again.  Now nothing is drawn when dead, just like 2.0.
 * play.c:
 * 	Small fix to remove AIX compiler complaint, it improves code readability
 * 	too I think.  Unimportant.
 * player.c:
 * 	New playerShielding code for playerStartsShielded option.
 * 	Bugfix for race mode when only one player is playing.
 * robot.c:
 * 	New playerShielding code for playerStartsShielded option.
 * server.c:
 * 	Removed RadarHeight stuff as this is only needed in the client.
 * 	Bugfix for Wait_for_new_players() to disable the real-time interval
 * 	timer when noone is playing.  Otherwise all selects() would have been
 * 	interrupted at a rate of FPS.
 * socklib.c:
 * 	Changed SocketReadable() to check for select returning -1.
 * timer.c:
 * 	Dummy routines if defined BUSYLOOP for block_timer() and allow_timer().
 * update.c:
 * 	New transport to home stuff after death.  I really like this and
 * 	am curious to know if you do too.  You now are transported back to
 * 	home with a constant acceleration and deacceleration.  Try it!
 * 	The old code is preserved with an ifdef.
 * xinit.c:
 * 	I think I fixed two syntax bugs, but please have a careful look yourself.
 * 	There were two statements like: if (a = b), instead of: if (a == b).
 *
 * That's all sofar.
 *
 * Bert
 *
 * Revision 3.0  1993/05/21  18:36:39  bjoerns
 * New client server release.
 *
 * Revision 1.3  1993/05/20  17:33:53  kenrsc
 * Bert changes from M -> R version !!!!!
 *
 * Revision 1.2  1993/05/18  16:49:30  kenrsc
 * Berts few changes !
 *
 * Revision 1.1  1993/04/22  10:21:32  bjoerns
 * Moved socklib from lib to src.
 *
 * Revision 1.3  1993/04/14  16:50:41  bjoerns
 * Applied two patches from Bert, one was really big and fixed all known
 * wrap-around problems, and added some timing and robot optimisations.
 * Also fixed other small bugs.
 *
 * Revision 1.2  1993/03/24  03:24:56  bjoerns
 * Patch by Bert, mainly casting to right type.
 *
 * Revision 1.1  1993/03/09  14:33:25  kenrsc
 * Hopefully we won't have a corrupted CVS directory anymore.
 *
 * Revision 1.1.1.1  1993/02/27  14:47:46  bjoerns
 * XPilot v2.0
 *
 * Revision 1.1.1.1  1993/01/19  17:19:59  bjoerns
 * XPilot v1.4
 *
 * Revision 1.4  1992/08/26  19:36:35  bjoerns
 * Incorporated NCD patch.
 *
 * Revision 1.3  1992/06/25  04:31:37  bjoerns
 * Added source id to socklib.
 *
 * Revision 1.2  1992/05/19  21:11:39  bjoerns
 * Minor.
 *
 * Revision 1.1.1.1  1992/05/11  12:32:34  bjoerns
 * XPilot v1.0
 *
 * Revision 1.2  91/10/02  08:38:01  08:38:01  arne (Arne Helme)
 * "ANSI C prototypes added.
 * Timeout interface changed."
 * 
 * Revision 1.1  91/10/02  08:34:45  08:34:45  arne (Arne Helme)
 * Initial revision
 * 
 */

#ifndef lint
static char sourceid[] =
    "@(#)$Id: ";
#endif

/* _SOCKLIB_LIBSOURCE must be defined int this file */
#define _SOCKLIB_LIBSOURCE

/* Include files */
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#if (SVR4)
#include <sys/filio.h>
#endif
#if (__hpux)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

/* Socklib Includes And Definitions */
#include "socklib.h"

/* Debug macro */
#ifdef DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

/* Default timeout value of socklib_timeout */
#define DEFAULT_S_TIMEOUT_VALUE		10
#define DEFAULT_US_TIMEOUT_VALUE	0

/* Default retry value of sl_default_retries */
#define DEFAULT_RETRIES			5

/* External references */
extern int 		errno;

/* Environment buffer for setjmp and longjmp */
static			jmp_buf env;

/* Global socklib errno variable */
int			sl_errno = 0;

/* Global timeout variable. May be modified by users */
int			sl_timeout_s = DEFAULT_S_TIMEOUT_VALUE;
int			sl_timeout_us = DEFAULT_US_TIMEOUT_VALUE;

/* Global default retries variable used by DgramSendRec */
int			sl_default_retries = DEFAULT_RETRIES;

/* Global variable containing the last address from DgramReceiveAny */
struct sockaddr_in	sl_dgram_lastaddr;

/* Global broadcast enable variable (super-user only), default disabled */
int			sl_broadcast_enabled = 0;


/*
 *******************************************************************************
 *
 *	SetTimeout()
 *
 *******************************************************************************
 * Description
 *	Sets the global timout value to s + us.
 *
 * Input Parameters
 *	s			- Timeout value in seconds
 *	us			- Timeout value in useconds
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	void
 *
 * Globals Referenced
 *	sl_timeout_us		- Timeout value in useconds
 *	sl_timeout_s		- Timeout value in seconds
 *
 * External Calls
 *	None
 *
 * Called By
 *	User applications
 *
 * Originally coded by Arne Helme
 */
#ifdef __STDC__
void
SetTimeout(int s, int us)
#else
SetTimeout(s, us)
int s, us;
#endif /* __STDC__ */
{
    sl_timeout_us = us;
    sl_timeout_s = s;
} /* SetTimeout */


/*
 *******************************************************************************
 *
 *	CreateServerSocket()
 *
 *******************************************************************************
 * Description
 *	Creates a TCP/IP server socket in the Internet domain.
 *
 * Input Parameters
 *	port		- Server's listen port.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The function returns the socket descriptor, or -1 if 
 *	any errors occured. 
 *
 * Globals Referenced
 *	sl_errno	- if errors occured: SL_ESOCKET, SL_EBIND,
 *			  SL_ELISTEN
 *
 * External Calls
 *	socket
 *	bind
 *	listen
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
CreateServerSocket(int port)
#else
CreateServerSocket(port)
int	port;
#endif /* __STDC__ */
{
    struct sockaddr_in	addr_in;
    int			fd;
    int			retval;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
	sl_errno = SL_ESOCKET;
	return (-1);
    }
    memset((char *)&addr_in, 0, sizeof(struct sockaddr_in));
    addr_in.sin_family		= AF_INET;
    addr_in.sin_addr.s_addr	= INADDR_ANY;
    addr_in.sin_port		= htons(port);
    
    retval = bind(fd, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in));
    if (retval < 0)
    {
	sl_errno = SL_EBIND;
	(void) close(fd);
	return (-1);
    }

    retval = listen(fd, 5);
    if (retval < 0)
    {
	sl_errno = SL_ELISTEN;
	(void) close(fd);
	return (-1);
    }

    return (fd);
} /* CreateServerSocket */


/*
 *******************************************************************************
 *
 *	GetPortNum()
 *
 *******************************************************************************
 * Description
 *	Returns the port number of a socket connection.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The port number on host standard format.
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	getsockname
 *
 * Called By
 *	User applications
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
GetPortNum(int fd)
#else
GetPortNum(fd)
int	fd;
#endif /* __STDC__ */
{
    int			len;
    struct sockaddr_in	addr;

    len = sizeof(struct sockaddr_in);
    if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0)
	return (-1);

    return (ntohs(addr.sin_port));
} /* GetPortNum */


/*
 *******************************************************************************
 *
 *	GetPeerName()
 *
 *******************************************************************************
 * Description
 *	Returns the hostname of the peer of connected stream socket.
 *
 * Input Parameters
 *	fd		- The connected stream socket descriptor.
 *	namelen		- Maximum length of the peer name.
 *
 * Output Parameters
 *	The hostname of the peer in a byte array.
 *
 * Return Value
 *	-1 on failure, 0 on success.
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	getpeername
 *	gethostbyaddr
 *	inet_ntoa
 *
 * Called By
 *	User applications
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
GetPeerName(int fd, char *name, int namelen)
#else
GetPeerName(fd, name, namelen)
int	fd;
char	*name;
int	namelen;
#endif /* __STDC__ */
{
    int			len;
    struct sockaddr_in	addr;
    struct hostent	*hp;

    len = sizeof(struct sockaddr_in);
    if (getpeername(fd, (struct sockaddr *)&addr, &len) < 0)
	return (-1);

    hp = gethostbyaddr((char *)&addr.sin_addr.s_addr, 4, AF_INET);
    if (hp != NULL)
    {
	strncpy(name, hp->h_name, namelen);
    }
    else
    {
	strncpy(name, inet_ntoa(addr.sin_addr), namelen);
    }
    name[namelen - 1] = '\0';

    return (0);
} /* GetPeerName */


/*
 *******************************************************************************
 *
 *	CreateClientSocket()
 *
 *******************************************************************************
 * Description
 *	Creates a client TCP/IP socket in the Internet domain.
 *
 * Input Parameters
 *	host		- Pointer to string containing name of the peer
 *			  host on either dot-format or ascii-name format.
 *	port		- The requested port number.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	Returns the socket descriptor or the error value -1.
 *
 * Globals Referenced
 *	sl_errno	- If errors occured: SL_EHOSTNAME, SL_ESOCKET,
 *			  SL_ECONNECT.
 *
 * External Calls
 *	memset
 *	gethostbyname
 *	socket
 *	connect
 *	close
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
CreateClientSocket(char *host, int port)
#else
CreateClientSocket(host, port)
char	*host;
int	port;
#endif /* __STDC__ */
{
    struct sockaddr_in	peer;
    struct hostent	*hp;
    int			fd;
    
    memset((char *)&peer, 0, sizeof(struct sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_port   = htons(port);
    
    peer.sin_addr.s_addr = inet_addr(host);
    if (peer.sin_addr.s_addr == (int)-1)
    {
	hp = gethostbyname(host);
	if (hp == NULL)
	{
	    sl_errno = SL_EHOSTNAME;
	    return (-1);
	}
	else
	    peer.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
	sl_errno = SL_ESOCKET;
	return (-1);
    }

    if (connect(fd, (struct sockaddr *)&peer, sizeof(struct sockaddr_in)) < 0) 
    {
	sl_errno = SL_ECONNECT;
	(void) close(fd);
	return (-1);
    }

    return (fd);
} /* CreateClientSocket */


/*
 *******************************************************************************
 *
 *	SocketAccept()
 *
 *******************************************************************************
 * Description
 *	This function is called in a TCP/IP server to accept incoming calls.
 *	
 * Input Parameters
 *	fd		- The listen socket.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The functions returns a new descriptor which is used to the
 *	actual data transfer.
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	none
 *	
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme.
 */
int 
#ifdef __STDC__
SocketAccept(int fd)
#else
SocketAccept(fd)
int	fd;
#endif /* __STDC__ */
{
    return accept(fd, NULL, 0);
} /* SocketAccept */


/*
 *******************************************************************************
 *
 *	SocketLinger()
 *
 *******************************************************************************
 * Description
 *	This function is called on a stream socket to set the linger option.
 *	
 * Input Parameters
 *	fd		- The stream socket to set the linger option on.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success.
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	setsockopt
 *	
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme, but moved out of SocketAccept by Bert.
 */
int 
#ifdef __STDC__
SocketLinger(int fd)
#else
SocketLinger(fd)
int	fd;
#endif /* __STDC__ */
{
#ifdef LINUX
    /*
     * As of 0.99.9 Linux doesn't have LINGER stuff.
     * This is likely to be improved in later versions however.
     */
    return 0;
#else
#ifdef	__hp9000s300
    long			linger = 1;
    int				lsize  = sizeof(long);
#else
    static struct linger	linger = {1, 300};
    int				lsize  = sizeof(struct linger);
#endif
    return setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&linger, lsize);
#endif
} /* SocketLinger */


/*
 *******************************************************************************
 *
 *	SetSocketReceiveBufferSize()
 *
 *******************************************************************************
 * Description
 *	Set the receive buffer size for either a stream or a datagram socket.
 *
 * Input Parameters
 *	fd		- The socket descriptor to operate on.
 *	size		- The new buffer size to use by the kernel.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success
 *
 * Globals Referenced
 *	none
 *
 * External Calls
 *	setsockopt
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
SetSocketReceiveBufferSize(int fd, int size)
#else
SetSocketReceiveBufferSize(fd, size)
int	fd;
int	size;
#endif /* __STDC__ */
{
    return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
	(char *)&size, sizeof(size)));
} /* SetSocketReceiveBufferSize */


/*
 *******************************************************************************
 *
 *	SetSocketSendBufferSize()
 *
 *******************************************************************************
 * Description
 *	Set the send buffer size for either a stream or a datagram socket.
 *
 * Input Parameters
 *	fd		- The socket descriptor to operate on.
 *	size		- The new buffer size to use by the kernel.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success
 *
 * Globals Referenced
 *	none
 *
 * External Calls
 *	setsockopt
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
SetSocketSendBufferSize(int fd, int size)
#else
SetSocketSendBufferSize(fd, size)
int	fd;
int	size;
#endif /* __STDC__ */
{
    return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
	(char *)&size, sizeof(size)));
} /* SetSocketSendBufferSize */


/*
 *******************************************************************************
 *
 *	SetSocketNoDelay()
 *
 *******************************************************************************
 * Description
 *	Set the TCP_NODELAY option on a connected stream socket.
 *
 * Input Parameters
 *	fd		- The stream socket descriptor to operate on.
 *	flag		- One to turn it on, zero to turn it off.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success
 *
 * Globals Referenced
 *	none
 *
 * External Calls
 *	setsockopt
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
SetSocketNoDelay(int fd, int flag)
#else
SetSocketNoDelay(fd, flag)
int	fd;
int	flag;
#endif /* __STDC__ */
{
    return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
	(char *)&flag, sizeof(flag)));
} /* SetSocketNoDelay */


/*
 *******************************************************************************
 *
 *	SetSocketNonBlocking()
 *
 *******************************************************************************
 * Description
 *	Set the nonblocking option on a socket.
 *
 * Input Parameters
 *	fd		- The socket descriptor to operate on.
 *	flag		- One to turn it on, zero to turn it off.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success
 *
 * Globals Referenced
 *	none
 *
 * External Calls
 *	ioctl
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
SetSocketNonBlocking(int fd, int flag)
#else
SetSocketNonBlocking(fd, flag)
int	fd;
int	flag;
#endif /* __STDC__ */
{
#if (_SEQUENT_)
    return (fcntl(fd, F_SETFL, (flag * O_NDELAY)));
#else
    return (ioctl(fd, FIONBIO, &flag));
#endif
} /* SetSocketNonBlocking */


/*
 *******************************************************************************
 *
 *	GetSocketError()
 *
 *******************************************************************************
 * Description
 *	Clear the error status for the socket and return the error in errno.
 *
 * Input Parameters
 *	fd		- The socket descriptor to operate on.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on failure, 0 on success
 *
 * Globals Referenced
 *	errno
 *
 * External Calls
 *	getsockopt
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
GetSocketError(int fd)
#else
GetSocketError(fd)
int	fd;
#endif /* __STDC__ */
{
    int	error, size;

    size = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR,
	(char *)&error, &size) == -1) {
	return -1;
    }
    errno = error;
    return 0;
} /* GetSocketError */


/*
 *******************************************************************************
 *
 *	SocketReadable()
 *
 *******************************************************************************
 * Description
 *	Checks if data have arrived on the TCP/IP socket connection.
 *
 * Input Parameters
 *	fd		- The socket descriptor to be checked.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	TRUE (non-zero) or FALSE (zero) (or -1 if select() fails).
 *
 * Globals Referenced
 *	socket_timeout
 *
 * External Calls
 *	select
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
SocketReadable(int fd)
#else
SocketReadable(fd)
int	fd;
#endif /* __STDC__ */
{
    int			readfds;
    struct timeval	timeout;

    timerclear(&timeout); /* macro function */
    timeout.tv_sec = sl_timeout_s;
    timeout.tv_usec = sl_timeout_us;
    readfds = (1 << fd);

    if (select(fd + 1, &readfds, NULL, NULL, &timeout) == -1)
	return ((errno == EINTR) ? 0 : -1);
    
    if (readfds & (1 << fd))
	return (1);
    return (0);
} /* SocketReadable */


/*
 *******************************************************************************
 *
 *	inthandler()
 *
 *******************************************************************************
 * Description
 *	Library routine used to jump to a previous state.
 *
 * Input Parameters
 *	None
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	None
 *
 * Globals Referenced
 *	env
 *
 * External Calls
 *	longjmp
 *
 * Called By
 *	SocketRead
 *
 * Originally coded by Arne Helme
 */
#ifdef __STDC__
static void 
#else
static
#endif /* __STDC__ */
inthandler()
{
    DEB(fprintf(stderr, "Connection interrupted, timeout\n"));
    (void) longjmp(env, 1);
} /* inthandler */


/*
 *******************************************************************************
 *
 *	SocketRead()
 *
 *******************************************************************************
 * Description
 *	Receives <size> bytes and put them into buffer <buf> from
 *	socket <fd>.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	size		- The expected amount of data to receive.
 *
 * Output Parameters
 *	buf		- Pointer to a message buffer.
 *
 * Return Value
 *	The number of bytes received or -1 if any errors occured.
 *
 * Globals Referenced
 *	sl_timeout
 *
 * External Calls
 *	setjmp
 *	alarm
 *	signal
 *	read
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
SocketRead(int fd, char *buf, int size)
#else
SocketRead(fd, buf, size)
int	fd, size;
char	*buf;
#endif /* __STDC__ */
{
    int	ret, ret1;

    if (setjmp(env)) 
    {
	(void) alarm(0);
	(void) signal(SIGALRM, SIG_DFL);
	return (-1);
    }
    ret = 0;
    while (ret < size) 
    {
	(void) signal(SIGALRM, ((void (*) ())inthandler));
	(void) alarm(sl_timeout_s);
	ret1 = read(fd, &buf[ret], size - ret);
	DEB(fprintf(stderr, "Read %d bytes\n", ret1));
	(void) alarm(0);
	(void) signal(SIGALRM, SIG_DFL);
	ret += ret1;
	if (ret1 <= 0)
	    return (-1);
    }
    return (ret);
} /* SocketRead */


/*
 *******************************************************************************
 *
 *	SocketWrite()
 *
 *******************************************************************************
 * Description
 *	Writes <size> bytes from buffer <buf> onto socket <fd>.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	buf		- Pointer to a send buffer.
 *	size		- The amount of data to send.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The number of bytes sent or -1 if any errors occured.
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	write
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
SocketWrite(int fd, char *buf, int size)
#else
SocketWrite(fd, buf, size)
int	fd, size;
char	*buf;
#endif /* __STDC__ */
{
    /*
     * A SIGPIPE exception may occur if the peer entity has disconnected.
     */
    return (write(fd, buf, size));
} /* SocketWrite */


/*
 *******************************************************************************
 *
 *	SocketClose()
 *
 *******************************************************************************
 * Description
 *	performs a gracefule shutdown and close on a TCP/IP socket. May
 * 	cause errounous behaviour when used on the same connection from
 *	more than one process.
 *
 * Input Parameters
 *	fd		- The socket to be closed.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 if any errors occured, else 1.
 *
 * Globals Referenced
 *	sl_errno	- If any errors occured: SL_ESHUTD, SL_ECLOSE.
 *
 * External Calls
 *	shutdown
 *	close
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
SocketClose(int fd)
#else
SocketClose(fd)
int	fd;
#endif /* __STDC__ */
{
    if (shutdown(fd, 2) == -1)
    {
	sl_errno = SL_ESHUTD;
	/* return (-1);  ***BG: need close always */
    }

    if (close(fd) == -1)
    {
	sl_errno = SL_ECLOSE;
	return (-1);
    }
    return (1);
} /* SocketClose */


/*
 *******************************************************************************
 *
 *	CreateDgramSocket()
 *
 *******************************************************************************
 * Description
 *	Creates a UDP/IP datagram socket in the Internet domain.
 *
 * Input Parameters
 *	port		- The port number. A value of zero may be specified in
 *			  clients to assign any available port number.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	A UDP/IP datagram socket descriptor.
 *
 * Globals Referenced
 *	sl_errno	- If any errors occured: SL_ESOCKET, SL_EBIND.
 *
 * External Calls
 *	socket
 *	memset
 *	bind
 *	close
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int 
#ifdef __STDC__
CreateDgramSocket(int port)
#else
CreateDgramSocket(port)
int	port;
#endif /* __STDC__ */
{
    struct sockaddr_in	addr_in;
    int			fd;
    int			retval;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
	sl_errno = SL_ESOCKET;
	return (-1);
    }

    memset((char *)&addr_in, 0, sizeof(struct sockaddr_in));
    addr_in.sin_family		= AF_INET;
    addr_in.sin_addr.s_addr	= INADDR_ANY;
    addr_in.sin_port		= htons(port);
    retval = bind(fd, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in));
    if (retval < 0)
    {
	sl_errno = SL_EBIND;
	retval = errno;
	(void) close(fd);
	errno = retval;
	return (-1);
    }

    return (fd);
} /* CreateDgramSocket */


/*
 *******************************************************************************
 *
 *	DgramConnect()
 *
 *******************************************************************************
 * Description
 *	Associate a datagram socket with a peer.
 *
 * Input Parameters
 *	fd		- The socket to operate on.
 *	host		- The host name.
 *	port		- The port number. 
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	-1 on error, 0 on success
 *
 * Globals Referenced
 *	sl_errno	- If any errors occured: SL_EHOSTNAME, SL_ECONNECT.
 *
 * External Calls
 *	connect
 *	gethostbyname
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert G�sbers
 */
int 
#ifdef __STDC__
DgramConnect(int fd, char *host, int port)
#else
DgramConnect(fd, host, port)
int	fd;
char	*host;
int	port;
#endif /* __STDC__ */
{
    struct sockaddr_in	addr_in;
    struct hostent	*hp;
    int			retval;

    memset((char *)&addr_in, 0, sizeof(addr_in));
    addr_in.sin_addr.s_addr 	= inet_addr(host);
    if (addr_in.sin_addr.s_addr == (unsigned long)-1)
    {
	hp = gethostbyname(host);
	if (hp == NULL)
	{
	    sl_errno = SL_EHOSTNAME;
	    return (-1);
	}
	else
	    addr_in.sin_addr.s_addr = 
		((struct in_addr*)(hp->h_addr))->s_addr;
    }
    addr_in.sin_family		= AF_INET;
    addr_in.sin_port		= htons(port);
    retval = connect(fd, (struct sockaddr *)&addr_in, sizeof(addr_in));
    if (retval < 0)
    {
	sl_errno = SL_ECONNECT;
	return (-1);
    }

    return (0);
} /* DgramConnect */


/*
 *******************************************************************************
 *
 *	DgramSend()
 *
 *******************************************************************************
 * Description
 *	Transmits a UDP/IP datagram.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	host		- Pointer to string containing destination host name.
 *	port		- Destination port.
 *	sbuf		- Pointer to the message to be sent.
 *	size		- Message size.
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The number of bytes sent or -1 if any errors occured.
 *
 * Globals Referenced
 *	sl_broadcast_enabled
 *	sl_errno	- If any errors occured: SL_EHOSTNAME.
 *
 * External Calls
 *	memset
 *	inet_addr
 *	gethostbyname
 *	sendto
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
DgramSend(int fd, char *host, int port,
	  char *sbuf, int size)
#else
DgramSend(fd, host, port, sbuf, size)
int	fd, port, size;
char	*host, *sbuf;
#endif /* __STDC__ */
{
    struct sockaddr_in	the_addr;
    struct hostent	*hp;

    (void) memset((char *)&the_addr, 0, sizeof(struct sockaddr_in));
    the_addr.sin_family		= AF_INET;
    the_addr.sin_port		= htons(port);
    if (sl_broadcast_enabled)
	the_addr.sin_addr.s_addr	= INADDR_BROADCAST;
    else
    {
	the_addr.sin_addr.s_addr 	= inet_addr(host);
	if (the_addr.sin_addr.s_addr == (int)-1)
	{
	    hp = gethostbyname(host);
	    if (hp == NULL)
	    {
		sl_errno = SL_EHOSTNAME;
		return (-1);
	    }
	    else
		the_addr.sin_addr.s_addr = 
		    ((struct in_addr*)(hp->h_addr))->s_addr;
	}
    }
    return (sendto(fd, sbuf, size, 0, (struct sockaddr *)&the_addr,
		   sizeof(struct sockaddr_in)));
} /* DgramSend */


/*
 *******************************************************************************
 *
 *	DgramReceiveAny()
 *
 *******************************************************************************
 * Description
 *	Receives a datagram from any sender.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	size		- Expected message size.
 *
 * Output Parameters
 *	rbuf		- Pointer to a message buffer.
 *
 * Return Value
 *	The number of bytes received or -1 if any errors occured.
 *
 * Globals Referenced
 *	sl_dgram_lastaddr
 *
 * External Calls
 *	memset
 *
 * Called By
 *	User applications
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
DgramReceiveAny(int fd, char *rbuf, int size)
#else
DgramReceiveAny(fd, rbuf, size)
int	fd;
char	*rbuf;
int	size;
#endif /* __STDC__ */
{
    int		addrlen = sizeof(struct sockaddr_in);

    (void) memset((char *)&sl_dgram_lastaddr, 0, addrlen);
    return (recvfrom(fd, rbuf, size, 0, (struct sockaddr *)&sl_dgram_lastaddr,
	&addrlen));
} /* DgramReceiveAny */


/*
 *******************************************************************************
 *
 *	DgramReceive()
 *
 *******************************************************************************
 * Description
 *	Receive a datagram from a specifc host. If a message from another
 *	host arrives, an error value is returned.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	from		- Pointer to the specified hostname.
 *	size		- Expected message size.
 *
 * Output Parameters
 *	rbuf		- Pointer to message buffer.
 *
 * Return Value
 *	The number of bytes received or -1 if any errors occured.
 *
 * Globals Referenced
 *	sl_dgram_lastaddr
 *	sl_errno	- If any errors occured: SL_EHOSTNAME, SL_EWRONGHOST.
 *
 * External Calls
 *	inet_addr
 *	gethostbyname
 *	DgramReceiveAny
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
DgramReceive(int fd, char *from, char *rbuf, int size)
#else
DgramReceive(fd, from, rbuf, size)
int	fd, size;
char	*from, *rbuf;
#endif /* __STDC__ */
{
    struct sockaddr_in	tmp_addr;
    struct hostent	*hp;
    int			retval;

    tmp_addr.sin_addr.s_addr = inet_addr(from);
    if (tmp_addr.sin_addr.s_addr == (int)-1)
    {
	hp = gethostbyname(from);
	if (hp == NULL)
	{
	    sl_errno = SL_EHOSTNAME;
	    return (-1);
	}
	else
	    tmp_addr.sin_addr.s_addr =
		((struct in_addr*)(hp->h_addr))->s_addr;
    }
    retval = DgramReceiveAny(fd, rbuf, size);
    if (retval == -1 || 
	tmp_addr.sin_addr.s_addr != sl_dgram_lastaddr.sin_addr.s_addr)
    {
	sl_errno = SL_EWRONGHOST;
	return (-1);
    }
    return (retval);
} /* DgramReceive */


/*
 *******************************************************************************
 *
 *	DgramInthandler()
 *
 *******************************************************************************
 * Description
 *	Library routine used by DgramSendRec to handle alarm interrupts.
 *
 * Input Parameters
 *	None
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	None
 *
 * Globals Referenced
 *	None
 *
 * External Calls
 *	signal
 *
 * Called By
 *	DgramSendRec
 *
 * Originally coded by Arne Helme
 */
#ifdef __STDC__
static void
DgramInthandler(void)
#else
static
DgramInthandler()
#endif /* __STDC__ */
{
    (void) signal(SIGALRM, ((void (*) ())DgramInthandler));
} /* DgramInthandler */


/*
 *******************************************************************************
 *
 *	DgramSendRec()
 *
 *******************************************************************************
 * Description
 *	Sends a message to a specified host and receives a reply from the
 *	same host. Messages arriving from other hosts when this routine is
 *	called will be discarded. Timeouts and retries can be modified
 * 	by setting the global variables sl_timeout and sl_default_retries.
 *
 * Input Parameters
 *	fd		- The socket descriptor.
 *	host		- Pointer to string contaning a hostname.
 *	port		- The specified port.
 *	sbuf		- Pointer to buffer containing message to be sent.
 *	sbuf_size	- The size of the outgoing message.
 *	rbuf_size	- Expected size of incoming message.
 *
 * Output Parameters
 *	rbuf		- Pointer to message buffer.
 *
 * Return Value
 *	The number of bytes received from the specified host or -1 if any
 *	errors occured.
 *
 * Globals Referenced
 *	errno
 *	sl_errno
 *	sl_timeout
 *	sl_default_retries
 *
 * External Calls
 *	alarm
 *	signal
 *	DgramSend
 *	DgramReceive
 *	
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
DgramSendRec(int fd, char *host, int port, char *sbuf,
	     int sbuf_size, char *rbuf, int rbuf_size)
#else
DgramSendRec(fd, host, port, sbuf, sbuf_size, rbuf, rbuf_size)
int	fd, port, sbuf_size, rbuf_size;
char	*host, *sbuf, *rbuf;
#endif /* __STDC__ */
{
    int		retval;
    int		retry = sl_default_retries;
    
    (void) signal(SIGALRM, ((void (*) ())DgramInthandler));
    while (retry)
    {
	if (DgramSend(fd, host, port, sbuf, sbuf_size) == -1)
	    return (-1);

	(void) alarm(sl_timeout_s);
	retval = DgramReceive(fd, host, rbuf, rbuf_size);
	if (retval == -1)
	    if (errno == EINTR || sl_errno == SL_EWRONGHOST)
		/* We have a timeout or a message from wrong host */
		if (--retry)
		    continue;	/* Try one more time */
		else
		{
		    sl_errno = SL_ENORESP;
		    break;	/* Unable to get response */
		}
	    else
	    {
		sl_errno = SL_ERECEIVE;
		break;		/* Unable to receive response */
	    }
	else
	    break;		/* Datagram from <host> arrived */
    }
    (void) alarm(0);
    (void) signal(SIGALRM, SIG_DFL);
    return (retval);
} /* DgramInthandler */


/*
 *******************************************************************************
 *
 *	DgramLastaddr()
 *
 *******************************************************************************
 * Description
 *	Extracts the last host address from the global variable
 *	sl_dgram_lastaddr.
 *
 * Input Parameters
 *	None
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	Pointer to string containing the host address. Warning, the string
 *	resides in static memory area.
 *
 * Globals Referenced
 *	sl_dgram_lastaddr
 *
 * External Calls
 *	inet_ntoa
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
char *
#ifdef __STDC__
DgramLastaddr(void)
#else
DgramLastaddr()
#endif /* __STDC__ */
{
    return (inet_ntoa(sl_dgram_lastaddr.sin_addr));
} /* DgramLastaddr */


/*
 *******************************************************************************
 *
 *	DgramLastname()
 *
 *******************************************************************************
 * Description
 *	Does a name lookup for the last host address from the
 *	global variable sl_dgram_lastaddr.  If this nameserver
 *	query fails then it resorts to DgramLastaddr().
 *
 * Input Parameters
 *	None
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	Pointer to string containing the hostname. Warning, the string
 *	resides in static memory area.
 *
 * Globals Referenced
 *	sl_dgram_lastaddr
 *
 * External Calls
 *	inet_ntoa
 *	gethostbyaddr
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Bert Gijsbers
 */
char *
#ifdef __STDC__
DgramLastname(void)
#else
DgramLastname()
#endif /* __STDC__ */
{
    struct hostent	*he;
    char		*str;

    he = gethostbyaddr((char *)&sl_dgram_lastaddr.sin_addr,
		       sizeof(struct in_addr), AF_INET);
    if (he == NULL) {
	str = inet_ntoa(sl_dgram_lastaddr.sin_addr);
    } else {
	str = (char *) he->h_name;
    }
    return str;
} /* DgramLastname */


/*
 *******************************************************************************
 *
 *	DgramLastport()
 *
 *******************************************************************************
 * Description
 *	Extracts the last host port from the global variable sl_dgram_lastaddr.
 *
 * Input Parameters
 *	None
 *
 * Output Parameters
 *	None
 *
 * Return Value
 *	The last port number on host standard format.
 *
 * Globals Referenced
 *	sl_dgram_lastaddr
 *
 * External Calls
 *	None
 *
 * Called By
 *	User applications.
 *
 * Originally coded by Arne Helme
 */
int
#ifdef __STDC__
DgramLastport(void)
#else
DgramLastport()
#endif /* __STDC__ */
{
    return (ntohs((int)sl_dgram_lastaddr.sin_port));
} /* DgramLastport */


#if defined(__sun__)
/*
 * There seems to be a strange bug in inet_ntoa() on
 * some Suns running a particular version of SunOS causing a
 * segmentation violation.  This version of inet_ntoa() seems
 * to be a workaround for it.
 * Eventually we need to come up with something better.
 */
char *inet_ntoa (struct in_addr in)
{
	unsigned long addr = ntohl (in.s_addr);
	static char ascii[16];

	sprintf (ascii, "%d.%d.%d.%d",
		addr >> 24 & 0xFF,
		addr >> 16 & 0xFF,
		addr >> 8 & 0xFF,
		addr & 0xFF);

	return ascii;
}
#endif
