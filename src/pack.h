/* $Id: pack.h,v 1.7 1992/06/28 05:38:22 bjoerns Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#ifndef	PACK_H
#define	PACK_H

#include "lib/socklib.h"
#include "limits.h"
#include "types.h"

#define CAP_LETTER(c)	(c = (c>='a' && c<='z') ? c-'a'+'A' : c)

#define SERVER_PORT	15345		/* Port which server listens to. */
#define	MAGIC		0xABCDEF0UL	/* - Unique magic number - */

#define	MAX_STR_LEN	4096
#define	MAX_ARG_LEN	256
#define	MAX_DISP_LEN	80
#define	MAX_NAME_LEN	16


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Following are the different PACK types which the server
 * and the hand-shake program understands.  If any modification
 * is made to these, remember to change the MAGIC field.
 *
 */


/*
 * All common fields must be included in the following macro.
 */
#define	CORE	\
    					/* This is provided to prevent */    \
    u_long	magic;			/* corrupt and outdated packets */   \
    					/* from being interpreted. */	     \
    									     \
    u_byte	type;			/* Type of pack transmitted */	     \
    char	realname[MAX_NAME_LEN];	/* Real name of sender */            \
    u_long	port;			/* Port number to contact */         \
    u_byte	status;			/* Status of operation */            \


/*
 * REPLY pack, used to transfer large strings (info), or just as a general
 * reply pack.
 */
typedef struct {

    CORE

    char	str[MAX_STR_LEN];	/* Just a string, too large :) */
} reply_pack_t;
#define	REPLY_pack		0x10


/*
 * ENTER GAME pack, contains misc. info the server would like to know about.
 * Sent from the hand-shake program when a player wants to enter the game.
 */
typedef struct {

    CORE

    char	display[MAX_DISP_LEN];	/* Display of player */
    char	nick[MAX_NAME_LEN];	/* Nick name */
    u_short	team;			/* Team of player */
#define	TEAM_NOT_SET		0xff
} enter_game_pack_t;
#define	ENTER_GAME_pack		0x00


/*
 * COMMAND pack, sent from the hand-shake program to the server.  Contains
 * a command and argument.
 */
typedef	struct {

    CORE

    u_long	arg_int;
    char	arg_str[MAX_ARG_LEN];
} command_pack_t;
#define	REPORT_STATUS_pack	0x21
#define	LOCK_GAME_pack		0x22
#define	MESSAGE_pack		0x23
#define	SHUTDOWN_pack		0x24
#define	KICK_PLAYER_pack	0x25

#define	report_status_pack_t	command_pack_t
#define	lock_game_pack_t	command_pack_t
#define	message_pack_t		command_pack_t
#define	shutdown_pack_t		command_pack_t
#define	kick_player_pack_t	command_pack_t


/*
 * CORE pack, no information except the status field needed.
 */
typedef	struct {

    CORE

} core_pack_t;
#define	CORE_pack		0x30
#define	CONTACT_pack		0x31

#define	contact_pack_t		core_pack_t


/*
 *
 * Union which contains all the possible pack types.
 *
 */
typedef union {

    /*
     * Just the core pack, enables us to access fields in the CORE part of
     * all packs (without knowing what pack type it is).
     */
    core_pack_t		core;

    /*
     * Possible pack types sent by the hand-shake program.
     */
    enter_game_pack_t	enter;
    command_pack_t	command;

    /*
     * Packs going 'the other way' - that is, from the server to the
     * hand-shake program.
     */
    reply_pack_t	reply;

} pack_t;



/*
 * Possible error codes returned in the pack's status field.
 */
#define	SUCCESS		0x00		/* Operation successful */
#define	E_NOT_OWNER	0x01		/* Permission denied, not owner */
#define	E_GAME_FULL	0x02		/* Game is full, entry denied */
#define	E_GAME_LOCKED	0x03		/* Game is locked, entry denied */
#define	E_DISPLAY	0x04		/* Couldn't open display */
#define	E_DBUFF		0x05		/* Couldn't init. double buffering */
#define	E_NOT_FOUND	0x06		/* Player was not found */
#define	E_IN_USE	0x07		/* Name is already in use */

#endif
