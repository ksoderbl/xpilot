/* $Id: net.h,v 3.6 1993/08/02 12:41:18 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	NET_H
#define	NET_H

#include "socklib.h"
#include "const.h"

/*
 * We want to give the client the possibility to tune the performance
 * of the connection a little.  Therefore a few connection parameters
 * are configurable by the client, but within a reasonable range.
 */
#define MIN_SOCKBUF_SIZE	1024
#define MAX_SOCKBUF_SIZE	(50*1024)
#define SERVER_RECV_SIZE	MIN_SOCKBUF_SIZE
#ifdef LINUX
#define SERVER_SEND_SIZE	(3*512 + 3*128)
#else
#define SERVER_SEND_SIZE	(4*1024)
#endif
#define CLIENT_SEND_SIZE	SERVER_RECV_SIZE
#define CLIENT_RECV_SIZE	SERVER_SEND_SIZE

/*
 * Definitions for the states a socket buffer can be in.
 */
#define SOCKBUF_READ		0x01	/* if readable */
#define SOCKBUF_WRITE		0x02	/* if writeable */
#define SOCKBUF_LOCK		0x04	/* if locked against kernel i/o */
#define SOCKBUF_ERROR		0x08	/* if i/o error occurred */
#define SOCKBUF_DGRAM		0x10	/* if datagram socket */

/*
 * Hack: leave some spare room for the last terminating packet
 * of a frame update.
 */
#define SOCKBUF_WRITE_SPARE	8

/*
 * Maximum number of socket i/o retries if datagram socket.
 */
#define MAX_SOCKBUF_RETRIES	2

/*
 * Currently biggest packet size.
 */
#define	MAX_PACKET_SIZE		(1 + MSG_LEN)

/*
 * A buffer to reduce the number of system calls made and to reduce
 * the number of network packets.
 */
typedef struct {
    int		sock;		/* socket filedescriptor */
    char	*buf;		/* i/o buffer */
    int		size;		/* size of buffer */
    int		len;		/* amount of data in buffer */
    char	*ptr;		/* current position in buffer */
    int		state;		/* read/write/locked/error */
} sockbuf_t;

int Sockbuf_init(sockbuf_t *sbuf, int sock, int size, int state);
int Sockbuf_cleanup(sockbuf_t *sbuf);
int Sockbuf_clear(sockbuf_t *sbuf);
int Sockbuf_advance(sockbuf_t *sbuf, int len);
int Sockbuf_flush(sockbuf_t *sbuf);
int Sockbuf_write(sockbuf_t *sbuf, char *buf, int len);
int Sockbuf_read(sockbuf_t *sbuf);
int Sockbuf_copy(sockbuf_t *dest, sockbuf_t *src, int len);

#if !defined(STDVA)
#   if defined(__STDC__) && !defined(__sun__)
#	define STDVA	1		/* has ANSI stdarg stuff */
#   else
#	define STDVA	0		/* nope, still the K&R way */
#   endif
#endif

#if STDVA
    int Packet_printf(sockbuf_t *, char *fmt, ...);
    int Packet_scanf(sockbuf_t *, char *fmt, ...);
#else
    int Packet_printf();
    int Packet_scanf();
#endif

#endif

