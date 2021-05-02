/* $Id: pack.h,v 3.10 1993/08/02 12:55:19 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *	    Bert Gÿsbers (bert@mc.bio.uva.nl)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	PACK_H
#define	PACK_H

#include "socklib.h"
#include "const.h"
#include "types.h"

#define CAP_LETTER(c)	(c = (c>='a' && c<='z') ? c-'a'+'A' : c)

#define SERVER_PORT	15345		/* Port which server listens to. */
#define META_PORT	5500
#define META_HOST	"xpilot.cs.uit.no"

/*
 * Magic contact word.
 * The low 16 bits are the real magic word.
 * Bits 31-28 are the major version number.
 * Bits 27-24 are the minor version number.
 * Bits 23-20 are the patchlevel number.
 * Bits 19-16 are free to mean beta release or so.
 * These high bits only need to be changed when a new
 * client can't talk to an old server or vise versa.
 *
 * Reasons why it changed in the past:
 * 3.0.1: rewrite of contact pack protocol, because of
 * different structure layout rules on different architectures.
 * 3.0.2: rewrite of setup transmit from server to client to
 * make it possible for 64-bit machines and 32-bit machines
 * to join in the same game.  This was the last hardcoded
 * structure that was shared between client and server.
 * 3.0.3: implemented a version awareness system, so that
 * newer clients can join older servers and so that
 * newer servers can support older clients.
 * The client maintains a `version' variable indicating
 * the version of the server it has joined and the server
 * maintains for each connection a `connection_t->version'
 * and a `player->version' variable.
 */
#define	MAGIC		0x3030F4ED

#define MAGIC2VERSION(M)	(((M) >> 16) & 0xFFFF)
#define VERSION2MAGIC(V)	((((V) & 0xFFFF) << 16) | (MAGIC & 0xFFFF))
#define MY_VERSION		MAGIC2VERSION(MAGIC)

/*
 * Which client versions can join this server.
 */
#define MIN_CLIENT_VERSION	0x3020
#define MAX_CLIENT_VERSION	MY_VERSION

/*
 * Which server versions can this client join.
 */
#define MIN_SERVER_VERSION	0x3020
#define MAX_SERVER_VERSION	MY_VERSION

#define	MAX_STR_LEN	4096
#define	MAX_ARG_LEN	256
#define	MAX_DISP_LEN	80
#define	MAX_NAME_LEN	16

/*
 * Different contact pack types.
 */
#define	REPLY_pack		0x10
#define	ENTER_GAME_pack		0x00
#define	REPORT_STATUS_pack	0x21
#define	LOCK_GAME_pack		0x22
#define	MESSAGE_pack		0x23
#define	SHUTDOWN_pack		0x24
#define	KICK_PLAYER_pack	0x25
#define	CORE_pack		0x30
#define	CONTACT_pack		0x31

/*
 * Possible error codes returned.
 */
#define	SUCCESS		0x00		/* Operation successful */
#define	E_NOT_OWNER	0x01		/* Permission denied, not owner */
#define	E_GAME_FULL	0x02		/* Game is full, entry denied */
#define	E_TEAM_FULL	0x03		/* Team is full, entry denied */
#define	E_TEAM_NOT_SET	0x04		/* Team is full, entry denied */
#define	E_GAME_LOCKED	0x05		/* Game is locked, entry denied */
#define	E_DBUFF		0x06		/* Couldn't init. double buffering */
#define	E_NOT_FOUND	0x07		/* Player was not found */
#define	E_IN_USE	0x08		/* Name is already in use */
#define	E_SOCKET	0x09		/* Can't setup socket */
#define	E_INVAL		0x0A		/* Invalid input parameters */
#define	E_VERSION	0x0C		/* Incompatible version */

#endif
