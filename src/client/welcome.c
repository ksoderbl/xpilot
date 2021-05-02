/* $Id: welcome.c,v 4.45 2001/04/01 10:30:08 bert Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# ifdef _AIX
#  include <sys/select.h>
# endif
# include <X11/Xlib.h>
#endif

#ifdef _WINDOWS
# include "../common/NT/winX.h"
# include "NT/winclient.h"
# include "NT/winXXPilot.h"
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "paint.h"
#include "xinit.h"
#include "list.h"
#include "widget.h"
#include "error.h"
#include "dbuff.h"
#include "socklib.h"
#include "net.h"
#include "pack.h"
#include "connectparam.h"
#include "protoclient.h"
#include "portability.h"
#include "socklib.h"
#include "commonproto.h"


char welcome_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: welcome.c,v 4.45 2001/04/01 10:30:08 bert Exp $";
#endif


/*
 * max number of servers we can find on the local network.
 */
#define MAX_LOCAL_SERVERS	10

/*
 * Some constants for describing access to the meta servers.
 * XXX These are also defined in some other file.
 */
#define NUM_METAS		2
#define META_HOST		"meta.xpilot.org"
#define META_HOST_TWO		"meta2.xpilot.org"
#define META_IP			"129.242.16.101"
#define META_IP_TWO		"132.235.197.27"
#define META_PROG_PORT		4401
#define NUM_META_DATA_FIELDS	18

/*
 * Access the data field of one of the servers
 * which is listed by the meta servers.
 */
#define SI_DATA(it)		((server_info_t *)LI_DATA(it))

/*
 * All the fields for a server in one line of meta output.
 */
struct ServerInfo {
    char		*version,
			*hostname,
			*users_str,
			*mapname,
			*mapsize,
			*author,
			*status,
			*bases_str,
			*fps_str,
			*playlist,
			*sound,
			*teambases_str,
			*timing,
			*ip_str,
			*freebases,
			*queue_str,
			*domain,
			pingtime_str[5];
    unsigned		port,
			ip,
			users,
			bases,
			fps,
			uptime,
			teambases,
			queue,
			pingtime;
    struct timeval	start;
    unsigned char	serial;
};
typedef struct ServerInfo	server_info_t;

#define PING_UNKNOWN	10000		/* never transmitted a ping to it */	
#define PING_NORESP	9999		/* never responded to our ping */
#define PING_SLOW	9998		/* responded to first ping after
					 * we had already retried (ie slow!) */

/*
 * Here we hold the servers which are listed by the meta servers.
 * We record the time we contacted Meta so as to not overload Meta.
 * server_it is an iterator pointing at the first server for the next page.
 */
static list_t		server_list;
static time_t		server_list_creation_time;
static list_iter_t	server_it;

/*
 * Are we in the process of quitting, or joining a game.
 */
extern int		quitting;
static int		joining;


/*
 * Some widgets.
 * form_widget is our toplevel widget.
 */
static int		form_widget = NO_WIDGET;
static int		subform_widget = NO_WIDGET;
static int		subform_label_widget = NO_WIDGET;

/*
 * An array of structures with information to join a local server.
 */
static Connect_param_t	*global_conpar;
static Connect_param_t	*localnet_conpars;


/*
 * States a connection to a meta server can be in.
 */
enum MetaState {
    MetaConnecting	= 0,
    MetaReadable	= 1,
    MetaReceiving	= 2
};

/*
 * Structure describing a meta server.
 * Name, address, and filedescriptor.
 */
struct Meta {
    char		name[MAX_HOST_LEN];
    char		addr[16];
    sock_t		sock;
    enum MetaState	state;	/* connecting, readable, receiving */
};
static struct Meta	metas[NUM_METAS] = {
			    { META_HOST,     META_IP,     { 0 }, MetaConnecting },
			    { META_HOST_TWO, META_IP_TWO, { 0 }, MetaConnecting }
			};

/*
 * Enum for different modes the welcome screen can be in.
 * We start out waiting for user to make a selection.
 * Then the screen can be active in any of the subfunctions.
 * When the user makes a selection again then we can call
 * certain cleanup handlers to cleanup the state from
 * the previous mode before setting up data structures
 * for to the new mode.
 */
enum Welcome_mode {
    ModeWaiting,
    ModeLocalnet,
    ModeInternet,
    ModeServer,
    ModeHelp,
    ModeQuit
};
static enum Welcome_mode	welcome_mode = ModeWaiting;
static void Welcome_set_mode(enum Welcome_mode new_welcome_mode);

/*
 * Other prototypes.
 */
static int Welcome_process_one_event(XEvent *event);
static int Welcome_show_server_list(Connect_param_t *conpar);


/*
 * Print a message that we do not have enough memory.
 * Also useful for debugging to set a breakpoint here.
 */
static void Not_enough_memory(void)
{
    error("Not enough memory.");
}

/*
 * Process only exposure events.
 */
static void Welcome_process_exposure_events()
{
    XEvent			event;

    while (XCheckMaskEvent(dpy, ExposureMask, &event)) {
	Welcome_process_one_event(&event);
    }
}

/*
 * Communicate a message to the user via a label widget.
 *
 * position:
 * 0 means top
 * 1 means middle
 * 2 means bottom.
 */
static int Welcome_create_label(int position, const char *label_text)
{
    int				label_x, label_y, label_width, label_height;
    int				subform_width = 0;
    int				subform_height = 0;

    Widget_destroy_children(subform_widget);	/*?*/
    subform_label_widget = NO_WIDGET;
    Widget_get_dimensions(subform_widget, &subform_width, &subform_height);
    label_width = XTextWidth(textFont, label_text, strlen(label_text));
    label_width += 40;
    label_height = textFont->ascent + textFont->descent;
    label_x = (subform_width - label_width) / 2;
    if (label_x < 0) {
	label_x = 0;
    }
    switch (position) {
    default:
    case 0:
	label_y = 10;
	label_height += 10;
	break;
    case 1:
	label_y = subform_height / 2 - textFont->ascent - 10;
	label_height += 20;
	break;
    case 2:
	label_y = subform_height - 10 - textFont->ascent - textFont->descent;
	label_height += 10;
	break;
    }
    subform_label_widget =
	Widget_create_label(subform_widget,
			    label_x, label_y,
			    label_width, label_height,
			    0, label_text);
    if (subform_label_widget != NO_WIDGET) {
	/* map children */
	Widget_map_sub(subform_widget);
	/* wait until mapped */
	XSync(dpy, False);
	/* draw widgets */
	Welcome_process_exposure_events();
    }

    return subform_label_widget;
}

/*
 * User clicked on a local server to join.
 */
static int Local_join_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t		*conpar = (Connect_param_t *) user_data;
    int				result;

    result = Connect_to_server(1, 0, 0, NULL, conpar);
    if (result) {
	joining = 1;
	/* structure copy. */
	*global_conpar = *conpar;
    }

    return 0;
}

/*
 * User asked for status on a local server.
 */
#if 0
static int Local_status_cb(int widget, void *user_data, const char **text)
{
    /* Connect_param_t		*conpar = (Connect_param_t *) user_data; */

    return 0;
}
#endif

/* 
 * Cleanup when leaving the mode ModeLocalnet.
 */
static void Localnet_cleanup(void)
{
    if (localnet_conpars) {
	free(localnet_conpars);
	localnet_conpars = NULL;
    }
}

/*
 * User wants us to search for servers on the local net.
 */
static int Localnet_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t		*conpar = (Connect_param_t *) user_data;
    int				i;
    int				n = 0;
    int				label;
    int				label_y, label_height;
    int				subform_width = 0;
    int				subform_height = 0;
    char			*server_names;
    char			*server_addrs;
    char			*name_ptrs[MAX_LOCAL_SERVERS];
    char			*addr_ptrs[MAX_LOCAL_SERVERS];

    Welcome_set_mode(ModeLocalnet);

    label =
	Welcome_create_label(1,
	    "Searching for XPilot servers on your local network...");
    Widget_get_dimensions(subform_widget, &subform_width, &subform_height);

    server_names = (char *) malloc(MAX_LOCAL_SERVERS * MAX_HOST_LEN);
    server_addrs = (char *) malloc(MAX_LOCAL_SERVERS * MAX_HOST_LEN);
    if (!server_names || !server_addrs) {
	Not_enough_memory();
	quitting = 1;
	return 0;
    }
    for (i = 0; i < MAX_LOCAL_SERVERS; i++) {
	name_ptrs[i] = &server_names[i * MAX_HOST_LEN];
	addr_ptrs[i] = &server_addrs[i * MAX_HOST_LEN];
    }
    Contact_servers(0, NULL, 0, 2, 0, NULL,
		    MAX_LOCAL_SERVERS, &n,
		    addr_ptrs, name_ptrs,
		    conpar);
    LIMIT(n, 0, MAX_LOCAL_SERVERS);

    Widget_destroy_children(subform_widget);
    if (!n) {
	Welcome_create_label(1, "No servers were found on your local network.");
    } else {
	Welcome_create_label(0, "The following local XPilot servers were found:");
    }
    label_y = 10;
    label_height = textFont->ascent + textFont->descent;

    if (n > 0) {
	int	max_width = 0;
	int	button;
	int	button_width;
	int	button_height;
	int	button_x;
	int	button_y;
#if 0
	int	button2;
	int	button2_width;
	int	button2_height;
	int	button2_x;
	int	button2_y;
#endif
	int	button3;
	int	button3_width;
	int	button3_height;
	int	button3_x;
	int	button3_y;

	localnet_conpars = (Connect_param_t *) malloc(n * sizeof(Connect_param_t));
	if (!localnet_conpars) {
	    Not_enough_memory();
	    free(server_names);
	    free(server_addrs);
	    quitting = 1;
	    return 0;
	}
	for (i = 0; i < n; i++) {
	    int text_width = XTextWidth(textFont,
					name_ptrs[i],
					strlen(name_ptrs[i]));
	    if (text_width > max_width) {
		max_width = text_width;
	    }
	}
	for (i = 0; i < n; i++) {
	    localnet_conpars[i] = *conpar;
	    strcpy(localnet_conpars[i].server_name, name_ptrs[i]);
	    strcpy(localnet_conpars[i].server_addr, addr_ptrs[i]);
	    button_width = max_width + 20;
	    button_height = textFont->ascent + textFont->descent + 10;
	    button_x = 20;
	    button_y = label_y * 2 + label_height + i * (button_height + label_y);
	    button =
		Widget_create_label(subform_widget,
				    button_x, button_y,
				    button_width, button_height,
				    1, localnet_conpars[i].server_name);

#if 0
	    button2_x = button_x + button_width + button_x;
	    button2_y = button_y;
	    button2_width = XTextWidth(buttonFont, "Status", 6) + 40;
	    button2_height = buttonFont->ascent + buttonFont->descent + 10;
	    button2 =
		Widget_create_activate(subform_widget,
				       button2_x, button2_y,
				       button2_width, button2_height,
				       1, "Status",
				       Local_status_cb,
				       (void *) &localnet_conpars[i]);

	    button3_x = button2_x + button_width + button_x;
	    button3_y = button2_y;
#endif

	    button3_x = button_x + button_width + button_x;
	    button3_y = button_y;
	    button3_width = XTextWidth(buttonFont, "Join game", 7) + 40;
	    button3_height = buttonFont->ascent + buttonFont->descent + 10;
	    button3 =
		Widget_create_activate(subform_widget,
				       button3_x, button3_y,
				       button3_width, button3_height,
				       1, "Join game",
				       Local_join_cb,
				       (void *) &localnet_conpars[i]);
	}
    }
    Widget_map_sub(subform_widget);
    free(server_names);
    free(server_addrs);

    return 0;
}

/*
 * Deallocate a ServerInfo structure.
 */
static void Delete_server_info(server_info_t *sip)
{
    if (sip) {
	if (sip->version) {
	    free(sip->version);
	    sip->version = NULL;
	}
	free(sip);
    }
}

/*
 * Deallocate the server list.
 */
static void Delete_server_list(void)
{
    server_info_t	*sip;

    if (server_list) {
	while ((sip = (server_info_t *) List_pop_front(server_list)) != NULL) {
	    Delete_server_info(sip);
	}
	List_delete(server_list);
	server_list = NULL;
	server_list_creation_time = 0;
    }
    server_it = NULL;
}

/*
 * Convert a string to lowercase.
 */
static void string_to_lower(char *s)
{
    for (; *s; s++) {
	*s = tolower(*s);
    }
}

/*
 * From a hostname return the part after the last dot.
 * E.g.: Vincent.CS.Berkeley.EDU will return EDU.
 */
static char *Get_domain_from_hostname(char *hostname)
{
    static char		last_domain[] = "\x7E\x7E";
    char		*dom;

    if ((dom = strrchr(hostname, '.')) != NULL) {
	if (dom[1] == '\0') {
	    dom[0] = '\0';
	    dom = strrchr(hostname, '.');
	}
    }
    if (dom) {
	dom++;	/* skip dot */
	/* test toplevel domain for validity */
	if (!isdigit(*dom)) {
	    if (strlen(dom) >= 2 && strlen(dom) <= 3) {
		return dom;
	    }
	}
    }

    return last_domain;
}

/*
 * Sort servers based on:
 *	1) number of players.
 *	2) pingtime.
 *	3) country.
 *	4) hostname.
 */
static int Welcome_sort_server_list(void)
{
    list_t		old_list = server_list;
    list_t		new_list = List_new();
    list_iter_t		it;
    int			delta;
    void		*vp; 
    server_info_t	*sip_old;
    server_info_t	*sip_new;

    if (!new_list) {
	Not_enough_memory();
	return -1;
    }
    while ((vp = List_pop_front(old_list)) != NULL) {
	sip_old = (server_info_t *) vp;
	string_to_lower(sip_old->hostname);
	if (!strncmp(sip_old->hostname, "xpilot", 6)) {
	    sip_old->hostname[0] = 'X';
	    sip_old->hostname[1] = 'P';
	}
	sip_old->domain = Get_domain_from_hostname(sip_old->hostname);
	for (it = List_begin(new_list); it != List_end(new_list); LI_FORWARD(it)) {
	    sip_new = SI_DATA(it);
	    delta = sip_new->users - sip_old->users;
	    if (delta < 0) {
		/* old has more users */
		break;
	    }
	    else if (delta == 0) {
		delta = sip_old->pingtime - sip_new->pingtime;
		if (delta < 0) {
		    /* old has better ping time */
		    break;
		}
		else if (delta == 0) {
		    delta = strcmp(sip_old->domain, sip_new->domain);
		    if (delta < 0) {
			break;
		    }
		    else if (delta == 0) {
			delta = strcmp(sip_old->hostname, sip_new->hostname);
			if (delta < 0) {
			    break;
			}
		    }
		    else if (delta == 0) {
			if (sip_old->port < sip_new->port) {
			    break;
			}
		    }
		}
	    }
	}
	if (!List_insert(new_list, it, sip_old)) {
	    Not_enough_memory();
	    Delete_server_info(sip_old);
	}
    }

#if DEVELOPMENT
    if (getenv("XPILOTWELCOMEDEBUG") != NULL) {
	/* print for debugging */
	printf("\n");
	printf("Printing server list:\n");
	for (it = List_begin(new_list); it != List_end(new_list); LI_FORWARD(it)) {
	    sip_new = SI_DATA(it);
	    printf("%2d %5s %-31s %u", sip_new->users, sip_new->domain,
		    sip_new->hostname, sip_new->port);
	    if (sip_new->pingtime == PING_UNKNOWN) printf("%8s", "unknown");
	    else if (sip_new->pingtime == PING_NORESP) printf("%8s", "no resp");
	    else if (sip_new->pingtime == PING_SLOW) printf("%8s", "s-l-o-w");
	    else printf("%8u", sip_new->pingtime);
	    printf("\n");
	}
	printf("\n");
    }
#endif

    List_delete(old_list);
    server_list = new_list;

    return 0;
}

/*
 * Put server info on a sorted list.
 */
static int Add_server_info(server_info_t *sip)
{
    list_iter_t		it;
    server_info_t	*it_sip;

    if (!server_list) {
	server_list = List_new();
	if (!server_list) {
	    Not_enough_memory();
	    return -1;
	}
    }
    for (it = List_begin(server_list); it != List_end(server_list); LI_FORWARD(it)) {
	it_sip = SI_DATA(it);
	/* sort on IP. */
	if (it_sip->ip < sip->ip) {
	    continue;
	}
	if (it_sip->ip == sip->ip) {
	    /* same server when same IP + port. */
	    if (it_sip->port < sip->port) {
		continue;
	    }
	    if (it_sip->port == sip->port) {
		/* work around bug in meta: keep server with highest uptime. */
		if (it_sip->uptime > sip->uptime) {
		    /* printf("duplicate: not adding\n"); */
		    return -1;
		} else {
		    it = List_erase(server_list, it);
		    /* printf("duplicate: replacing\n"); */
		}
	    }
	}
	break;
    }
    if (!List_insert(server_list, it, sip)) {
	Not_enough_memory();
	return -1;
    }

#if 0
    /* print for debugging */
    printf("list size = %d after %08x, %d\n",
	    List_size(server_list), sip->ip, sip->port);
#endif

    return 0;
}

/*
 * Variant on strtok which does not skip empty fields.
 * Two delimiters after another returns the empty string ("").
 */
static char *my_strtok(char *buf, const char *sep)
{
    static char		*oldbuf;
    char		*ptr;
    char		*start;

    if (buf) {
	oldbuf = buf;
    }
    start = oldbuf;
    if (!start || !*start) {
	return NULL;
    }
    for (ptr = start; *ptr; ptr++) {
	if (strchr(sep, *ptr)) {
	    break;
	}
    }
    oldbuf = (*ptr) ? (ptr + 1) : (ptr);
    *ptr = '\0';
    return start;
}

/*
 * Parse one line of meta output and
 * put the fields in a structure.
 * The structure is put on a sorted list.
 */
static void Add_meta_line(char *meta_line)
{
    char		*fields[NUM_META_DATA_FIELDS];
    int			i;
    int			num = 0;
    char		*p;
    unsigned		ip0, ip1, ip2, ip3;
    char		*text = xp_strdup(meta_line);
    server_info_t	*sip;

    if (!text) {
	Not_enough_memory();
	return;
    }

    /* split line into fields. */
    for (p = my_strtok(text, ":"); p; p = my_strtok(NULL, ":")) {
	if (num < NUM_META_DATA_FIELDS) {
	    fields[num++] = p;
	}
    }
    if (num < NUM_META_DATA_FIELDS) {
	/* should not happen, except maybe for last line. */
	free(text);
	return;
    }
    if (fields[0] != text) {
	/* sanity check, should not happen. */
	free(text);
	return;
    }

    if ((sip = (server_info_t *) malloc(sizeof(server_info_t))) == NULL) {
	Not_enough_memory();
	free(text);
	return;
    }
    memset(sip, 0, sizeof(*sip));
    sip->pingtime = PING_UNKNOWN;
    sip->version = fields[0];
    sip->hostname = fields[1];
    sip->users_str = fields[3];
    sip->mapname = fields[4];
    sip->mapsize = fields[5];
    sip->author = fields[6];
    sip->status = fields[7];
    sip->bases_str = fields[8];
    sip->fps_str = fields[9];
    sip->playlist = fields[10];
    sip->sound = fields[11];
    sip->teambases_str = fields[13];
    sip->timing = fields[14];
    sip->ip_str = fields[15];
    sip->freebases = fields[16];
    sip->queue_str = fields[17];
    if (sscanf(fields[i = 2], "%u", &sip->port) != 1 ||
	sscanf(fields[i = 3], "%u", &sip->users) != 1 ||
	sscanf(fields[i = 8], "%u", &sip->bases) != 1 ||
	sscanf(fields[i = 9], "%u", &sip->fps) != 1 ||
	sscanf(fields[i = 12], "%u", &sip->uptime) != 1 ||
	sscanf(fields[i = 13], "%u", &sip->teambases) != 1 ||
	sscanf(fields[i = 15], "%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3) != 4 ||
	(ip0 | ip1 | ip2 | ip3) > 255 ||
	sscanf(fields[i = 17], "%u", &sip->queue) != 1) {
	printf("error %d in: %s\n", i, meta_line);
	free(sip);
	free(text);
	return;
    } else {
	sip->ip = (ip0 << 24) | (ip1 << 16) | (ip2 << 8) | ip3;
	if (Add_server_info(sip) == -1) {
	    free(sip);
	    free(text);
	    return;
	}
    }
}

/*
 * Connect to the meta servers asynchronously.
 * Return the number of connections made,
 * and the highest fd.
 */
static void Meta_connect(int *connections_ptr, int *maxfd_ptr)
{
    int			i;
    int			status;
    int			connections = 0;
    int			max = -1;
    char		buf[MSG_LEN];

    for (i = 0; i < NUM_METAS; i++) {
	if (metas[i].sock.fd != SOCK_FD_INVALID) {
	    sock_close(&metas[i].sock);
	}
	status = sock_open_tcp_connected_non_blocking(&metas[i].sock,
						      metas[i].addr,
						      META_PROG_PORT);
	if (status == SOCK_IS_ERROR) {
	    sprintf(buf, "Could not establish connection with %s",
		    metas[i].name);
	    error(buf);
	    Welcome_create_label(1, buf);
	} else {
	    connections++;
	    if (metas[i].sock.fd > max) {
		max = metas[i].sock.fd;
	    }
	}
    }
    if (connections_ptr) {
	*connections_ptr = connections;
    }
    if (maxfd_ptr) {
	*maxfd_ptr = max;
    }
}

/*
 * Lookup the IPs of the metas.
 */
static void Meta_dns_lookup(void)
{
    int			i;
    char		*addr;
    char		buf[MSG_LEN];

    for (i = 0; i < NUM_METAS; i++) {
	if (metas[i].sock.fd == -2) {
	    metas[i].sock.fd = SOCK_FD_INVALID;
	    sprintf(buf, "Doing a DNS lookup on %s ... ", metas[i].name);
	    Welcome_create_label(1, buf);
	    addr = sock_get_addr_by_name(metas[i].name);
	    if (addr) {
		strcpy(metas[i].addr, addr);
	    }
	}
    }
}

static void Ping_servers()
{
    static int		serial;		/* mark pings to identify stale reply */
    const int		interval = 1000 / 14;	/* assumes we can do 14fps of pings */
    const int		tries = 1;	/* at least 1 ping for ever server.
					 * in practice we get several */
    int			maxwait = tries * interval * List_size(server_list);
    sock_t		sock;
    fd_set		input_mask, readmask;
    struct timeval	start, end, timeout;
    list_iter_t		it, that;
    server_info_t	*it_sip;
    sockbuf_t		sbuf, rbuf;
    int			ms;
    char		*reply_ip;
    int			reply_port;
    unsigned		reply_magic;
    unsigned char	reply_serial, reply_status;
    int			outstanding;
    char		buf[MSG_LEN];

    sprintf(buf, "Pinging servers (%d seconds)...", (maxwait + 500) / 1000);
    Welcome_create_label(1, buf);

    if (sock_open_udp(&sock, NULL, 0) == -1) {
	return;
    }
    if (sock_set_non_blocking(&sock, 1) == -1) {
	sock_close(&sock);
	return;
    }
    if (Sockbuf_init(&sbuf, &sock, CLIENT_RECV_SIZE,
		     SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1) {
	sock_close(&sock);
	return;
    }
    if (Sockbuf_init(&rbuf, &sock, CLIENT_RECV_SIZE,
		     SOCKBUF_READ | SOCKBUF_DGRAM) == -1) {
	Sockbuf_cleanup(&sbuf);
	sock_close(&sock);
	return;
    }

    FD_ZERO(&input_mask);
    FD_SET(sock.fd, &input_mask);

    it = List_end(server_list);
    outstanding = 0;
    ms = 0;
    gettimeofday(&start, NULL);
    do {
	while (outstanding < (ms / interval + 1)) {
	    if (it == List_end(server_list)) {
		++serial;
		serial &= 0xFF;
		if (serial == 0)
		    serial = 1;

		/*
		 * Send a packet to the contact port with
		 * a valid magic number but client version
		 * zero.  The server will reply to this
		 * so that the client can tell the user
		 * what version they need.
		 *
		 * Normally this would be a CONTACT_pack but
		 * we cheat and use the packet type field as
		 * a serial number, since the server is
		 * nice enough to send back whatever we send.
		 */
		Sockbuf_clear(&sbuf);
		Packet_printf(&sbuf, "%u%s%hu%c",
			      MAGIC & 0xffff, "p",
			      sock_get_port(&sock), serial);

		/*
		 * Assuming sort order is the most to least
		 * desirable servers, give the interesting
		 * servers first crack at more pings, making
		 * their results more accurate.
		 */
		Welcome_sort_server_list();
		it = List_begin(server_list);
	    }
	    it_sip = SI_DATA(it);
	    sock_send_dest(&sock, it_sip->ip_str, it_sip->port,
			   sbuf.buf, sbuf.len);
	    gettimeofday(&it_sip->start, NULL);
	    /* if it has never been pinged (pung?) mark it now
	     * as "not responding" instead of just blank.
	     */
	    if (it_sip->pingtime == PING_UNKNOWN) {
		it_sip->pingtime = PING_NORESP;
	    }
	    it_sip->serial = serial;
	    outstanding++;
	    LI_FORWARD(it);
	}
	timeout.tv_sec = 0;
	timeout.tv_usec = (interval - (ms % interval)) * 1000;
	readmask = input_mask;
	if (select(sock.fd + 1, &readmask, 0, 0, &timeout) == -1
	    && errno != EINTR) {
	    break;
	}
	gettimeofday(&end, NULL);
	ms = (end.tv_sec - start.tv_sec) * 1000 +
	    (end.tv_usec - start.tv_usec) / 1000;

	Sockbuf_clear(&rbuf);
	if ((rbuf.len = sock_receive_any(&sock, rbuf.buf, rbuf.size)) < 4) {
	    continue;
	}
	if (outstanding > 0) {
	    --outstanding;
	}
	if (Packet_scanf(&rbuf, "%u%c%c",
			 &reply_magic, &reply_serial, &reply_status) <= 0) {
	    continue;
	}
	reply_ip = sock_get_last_addr(&sock);
	reply_port = sock_get_last_port(&sock);
	for (that = List_begin(server_list);
	     that != List_end(server_list);
	     LI_FORWARD(that))
	{
	    it_sip = SI_DATA(that);
	    if (!strcmp(it_sip->ip_str, reply_ip)
		&& reply_port == it_sip->port)
	    {
		int n;

		if (reply_serial != it_sip->serial) {
		    /* replied to an old ping, alive but
		     * slower than `interval' at least
		     */
		    it_sip->pingtime = MIN(it_sip->pingtime, PING_SLOW);
		}
		else {
		    n = (end.tv_sec -
			 it_sip->start.tv_sec) * 1000 +
			(end.tv_usec - it_sip->start.tv_usec) / 1000;
		    it_sip->pingtime = MIN(it_sip->pingtime, n);
		}
		break;
	    }
	}
    } while (ms < maxwait);

    Sockbuf_cleanup(&sbuf);
    Sockbuf_cleanup(&rbuf);
    sock_close(&sock);
}

/*
 * User pressed the Internet button.
 */
static int Get_meta_data(void)
{
    int			i;
    int			max = -1;
    int			connections = 0;
    int			descriptor_count;
    int			readers = 0;
    int			senders = 0;
    int			bytes_read;
    int			buffer_space;
    int			total_bytes_read = 0;
    int			server_count;
    time_t		start, now;
    fd_set		rset_in, wset_in;
    fd_set		rset_out, wset_out;
    struct timeval	tv;
    char		*newline;
    char		buf[MSG_LEN];

    /*
     * Buffer to hold data from a socket connection to a Meta.
     * The ptr points to the first byte of the unprocessed data.
     * The end points to where the next new data should be loaded.
     */
    struct MetaData {
	char	*ptr;
	char	*end;
	char	buf[4096];
    };
    struct MetaData	md[NUM_METAS];

    /* lookup addresses. */
    Meta_dns_lookup();

    /* connect asynchronously. */
    Meta_connect(&connections, &max);
    if (!connections) {
	Welcome_create_label(1, "Could not establish connections with any metaserver");
	return -1;
    }

    sprintf(buf, "Establishing %s with %d metaserver%s ... ",
	    ((connections > 1) ? "connections" : "a connection"),
	    connections,
	    ((connections > 1) ? "s" : ""));
    Welcome_create_label(1, buf);

    /* setup select(2) structures. */
    FD_ZERO(&rset_in);
    FD_ZERO(&wset_in);
    for (i = 0; i < NUM_METAS; i++) {
	metas[i].state = MetaConnecting;
	if (metas[i].sock.fd != SOCK_FD_INVALID) {
	    FD_SET(metas[i].sock.fd, &wset_in);
	}
	md[i].ptr = NULL;
	md[i].end = NULL;
    }
    /*
     * First wait for the asynchronously connected sockets to become writable.
     * When a socket becomes writable it means that the connection attempt
     * has completed.  After that has happened we can test the socket for
     * readability.  When the connection attempt failed the read will
     * return -1 and probably set errno to ENOTCONN.
     *
     * We try to connect and read for a limited number of seconds.
     * Whenever a connection has succeeded we add another 5 seconds.
     * Whenever a read has succeeded we also add another 5 seconds.
     *
     * Keep administration of the number of sockects in the connected state,
     * the readability state, or the meta-is-sending-data state.
     */
    for (start = time(&now) + 5; connections > 0 && now < start + 5; time(&now)) {
	tv.tv_sec = start + 5 - now;
	tv.tv_usec = 0;
#if 0
	printf("select for %ld (con %d, read %d, send %d) at %ld\n",
	       tv.tv_sec, connections, readers, senders, time(0));
#endif
	rset_out = rset_in;
	wset_out = wset_in;
	descriptor_count = select(max + 1, &rset_out, &wset_out, NULL, &tv);
#if 0
	printf("select = %d at %ld\n", descriptor_count, time(0));
#endif
	if (descriptor_count <= 0) {
	    break;
	}
	for (i = 0; i < NUM_METAS; i++) {
	    if (metas[i].sock.fd == SOCK_FD_INVALID) {
		continue;
	    }
	    else if (FD_ISSET(metas[i].sock.fd, &wset_out)) {
		/* promote socket from writable to readable. */
		FD_CLR(metas[i].sock.fd, &wset_in);
		FD_SET(metas[i].sock.fd, &rset_in);
		metas[i].state = MetaReadable;
		readers++;
		if (!senders) {
		    sprintf(buf, "%d metaserver%s accepted a connection.",
			    readers, (readers > 1) ? "s have" : " has");
		    Welcome_create_label(1, buf);
		}
		time(&start);
	    }
	    else if (FD_ISSET(metas[i].sock.fd, &rset_out)) {
		if (md[i].ptr == NULL && md[i].end == NULL) {
		    md[i].ptr = md[i].buf;
		    md[i].end = md[i].buf;
		}
		buffer_space = &md[i].buf[sizeof(md[i].buf)] - md[i].end;
		bytes_read = read(metas[i].sock.fd, md[i].end, buffer_space);
		if (bytes_read <= 0) {
		    if (bytes_read == -1) {
			error("Error while reading data from meta %d\n", i + 1);
		    }
		    FD_CLR(metas[i].sock.fd, &rset_in);
		    close(metas[i].sock.fd);
		    metas[i].sock.fd = SOCK_FD_INVALID;
		    --connections;
		    --readers;
		    if (metas[i].state == MetaReceiving) {
			--senders;
			if (senders == 0) {
			    /*
			     * Assume that this meta has sent us all there is
			     * to know and close down all other connections.
			     */
			    for (i = 0; i < NUM_METAS; i++) {
				if (metas[i].sock.fd != SOCK_FD_INVALID) {
				    close(metas[i].sock.fd);
				    metas[i].sock.fd = SOCK_FD_INVALID;
				}
			    }
			    connections = 0;
			}
		    }
		    if (connections == 0) {
			break;
		    }
		}
		else {
		    /* Received some bytes from this connection. */
		    total_bytes_read += bytes_read;

		    /* If this connection wasn't marked as receiving do so now. */
		    if (metas[i].state != MetaReceiving) {
			metas[i].state = MetaReceiving;
			++senders;
		    }

		    sprintf(buf, "Received %d bytes from %d metaserver%s.",
			    total_bytes_read,
			    senders,
			    ((senders == 1) ? "" : "s")
			    );
		    Welcome_create_label(1, buf);

		    /* adjust buffer for newly read bytes. */
		    md[i].end += bytes_read;

		    /* process data for as far as we have lines ending in '\n'. */
		    while ((newline = (char *) memchr(md[i].ptr, '\n',
						      md[i].end - md[i].ptr))
			      != NULL) {

			*newline = '\0';
			if (newline > md[i].ptr && newline[-1] == '\r') {
			    newline[-1] = '\0';
			}
			Add_meta_line(md[i].ptr);
			md[i].ptr = newline + 1;
		    }
		    /* move partial data to the start of the buffer. */
		    if (md[i].ptr > md[i].buf) {
			int incomplete_data = (md[i].end - md[i].ptr);
			move_memory(md[i].buf, md[i].ptr, incomplete_data);
			md[i].ptr = md[i].buf;
			md[i].end = md[i].ptr + incomplete_data;
		    }
		    /* allow more time to receive more data */
		    time(&start);
		}
	    }
	}
    }

    server_count = 0;
    if (server_list) {
	server_count = List_size(server_list);
    }
    if (server_count > 0) {
	sprintf(buf, "Received information about %d Internet servers", server_count);
	server_list_creation_time = time(NULL);
    } else {
	sprintf(buf, "Could not contact any Internet Meta server");
    }
    Welcome_create_label(1, buf);

    return server_count;
}

/*
 * Copy a string up to a maximum number of bytes.
 * Similar to strncpy, but always terminates,
 * and writes only one zero byte.
 */
static char *string_max_copy(char *arg_dst, const char *arg_src, size_t n)
{
    char		*dst = arg_dst;
    const char		*src = arg_src;
    size_t		i;

    if (n > 0) {
	for (i = 1; i < n && *src != '\0'; i++) {
	    *dst++ = *src++;
	}
	/* always terminate */
	*dst = '\0';
    }

    return arg_dst;
}

/*
 * User wants to join a server.
 */
static int Internet_server_join_cb(int widget, void *user_data, const char **text)
{
    server_info_t		*sip = (server_info_t *) user_data;
    struct Connect_param	connect_param;
    struct Connect_param	*conpar = &connect_param;
    int				result;
    char			*server_addr_ptr = conpar->server_addr;

    /* structure copy */
    *conpar = *global_conpar;
    string_max_copy(conpar->server_name, sip->hostname, sizeof(conpar->server_name));
    string_max_copy(conpar->server_addr, sip->ip_str, sizeof(conpar->server_addr));
    conpar->contact_port = sip->port;
    result = Contact_servers(1, &server_addr_ptr, 1, 0, 0, NULL,
			     0, NULL,
			     NULL, NULL,
			     conpar);
    if (result) {
	/* structure copy */
	*global_conpar = *conpar;
	joining = 1;
    } else {
	printf("Server %s (%s) didn't respond on port %d\n",
		conpar->server_name, conpar->server_addr, conpar->contact_port);
    }

    return 0;
}

#if 0
/*
 * User selected a server on the Internet page.
 *
 * The idea is to show the characteristics to the user in more detail,
 * and choose team from this page, then click join.
 * Until work on this progresses stay with the simple Internet_server_join_cb().
 */
static int Internet_server_show_cb(int widget, void *user_data, const char **text)
{
    server_info_t		*sip = (server_info_t *) user_data;
    struct Connect_param	connect_param;
    struct Connect_param	*conpar = &connect_param;
    /* int				result; */
    /* char			*server_addr_ptr = conpar->server_addr; */
    int				subform_width = 0;
    int				subform_height = 0;
    int				i;
    int				label_x;
    int				label_y;
    int				label_x_offset;
    int				label_y_offset;
    int				label_width;
    int				label_height;
    int				label_space;
    int				label_border;
    int				max_label_width;
    struct Label {
	const char	*label;
	int		commas;
	int		yoff;
	int		height;
    };
    struct Label 		labels[] = {
				    /*  0 */ { "server hostname", 0, 0, 0 },
				    /*  1 */ { "xpilot version", 0, 0, 0 },
				    /*  2 */ { "users", 0, 0, 0 },
				    /*  3 */ { "map name", 0, 0, 0 },
				    /*  4 */ { "map size", 0, 0, 0 },
				    /*  5 */ { "map author", 0, 0, 0 },
				    /*  6 */ { "status", 0, 0, 0 },
				    /*  7 */ { "bases", 0, 0, 0 },
				    /*  8 */ { "teambases", 0, 0, 0 },
				    /*  9 */ { "free bases", 0, 0, 0 },
				    /* 10 */ { "queued players", 0, 0, 0 },
				    /* 11 */ { "FPS", 0, 0, 0 },
				    /* 12 */ { "sound", 0, 0, 0 },
				    /* 13 */ { "timing", 0, 0, 0 },
				    /* 14 */ { "playlist", 1, 0, 0 }
				};
    char			*s;

    Widget_destroy_children(subform_widget);

    /* structure copy */
    *conpar = *global_conpar;
    string_max_copy(conpar->server_name, sip->hostname, sizeof(conpar->server_name));
    string_max_copy(conpar->server_addr, sip->ip_str, sizeof(conpar->server_addr));
    conpar->contact_port = sip->port;
    /* structure copy */
    *global_conpar = *conpar;

    Widget_get_dimensions(subform_widget, &subform_width, &subform_height);

    button_x_offset = 10;

    Widget_create_activate(subform_widget,
			   label_x_offset,
			   label_y_offset,
			   server_width, label_height,
			   border ? border : 1, sip->hostname,
			   Internet_server_join_cb, (void *) sip);

    label_y_offset = 10;
    label_x_offset = 10;
    label_x = label_x_offset;
    label_y = label_y_offset;
    label_border = 0;
    label_space = 10;
    label_height = textFont->ascent + textFont->descent;
    max_label_width = 0;
    for (i = 0; i < NELEM(labels); i++) {
	label_width = XTextWidth(textFont,
				 labels[i].label,
				 strlen(labels[i].label));
	max_label_width = MAX(label_width, max_label_width);

	labels[i].yoff = label_y;
	label_y += label_height + label_space;
	if (labels[i].commas) {
	    labels[i].commas = 0;
	    for (s = sip->playlist; (s = strchr(s, ',')) != NULL; s++) {
		labels[i].commas++;
		label_y += label_height + label_space;
	    }
	}
	labels[i].height = (label_y - labels[i].yoff) - label_space;
    }

    label_width = max_label_width + 2 * label_space;
    for (i = 0; i < NELEM(labels); i++) {
	Widget_create_label(subform_widget,
			    label_x, labels[i].yoff,
			    label_width, labels[i].height,
			    label_border, labels[i].label);
    }

#if 0
    playlist_lines = 1;
    for (s = sip->playlist; (s = strchr(s, ',')) != NULL; s++) {
	playlist_lines++;
	Widget_create_label(subform_widget,
			    label_x, label_y,
			    label_width, label_height,
			    label_border, "");
	label_y += label_height + label_space;
    }
#endif

    Widget_map_sub(subform_widget);

    return 0;
}
#endif

/*
 * User pressed next page button on the Internet page.
 */
static int Internet_next_page_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t		*conpar = (Connect_param_t *) user_data;

    Welcome_show_server_list(conpar);

    return 0;
}

/*
 * User pressed first page button on the Internet page.
 */
static int Internet_first_page_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t		*conpar = (Connect_param_t *) user_data;

    server_it = List_begin(server_list);

    Welcome_show_server_list(conpar);

    return 0;
}

/*
 * User pressed "measure lag" button on the Internet page
 */
static int Internet_ping_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t		*conpar = (Connect_param_t *) user_data;

    Ping_servers();
    if (Welcome_sort_server_list() == -1) {
	Delete_server_list();
	Welcome_create_label(1, "Not enough memory.");
    }

    server_it = List_begin(server_list);
    Welcome_show_server_list(conpar);

    return 0;
}

/*
 * Create for each server a row on the subform_widget.
 */
static int Welcome_show_server_list(Connect_param_t *conpar)
{
    const int		border = 0;
    const int		extra_width = 6;
    const int		extra_height = 4;
    const int		space_width = 0 + 2 * border;
    const int		space_height = 4 + 2 * border;
    const int		max_map_length = 30;
    const int		max_version_length = 11;
    static const char	player_header[] = "Pl";
    static const char	queue_header[] = "Q";
    static const char	bases_header[] = "Ba";
    static const char	team_header[] = "Tm";
    static const char	fps_header[] = "FPS";
    static const char	status_header[] = "Stat";
    static const char	version_header[] = "Version";
    static const char	map_header[] = "Map";
    static const char	server_header[] = "Server                           ";
    static const char	ping_header[] = "Ping ";
    int			player_width = XTextWidth(textFont, "Pl", 2)
				    + extra_width + 2 * border;
    int			queue_width = XTextWidth(textFont, "99", 2)
				    + extra_width + 2 * border;
    int			bases_width = XTextWidth(textFont, "Ba", 2)
				    + extra_width + 2 * border;
    int			team_width = XTextWidth(textFont, "Tm", 2)
				    + extra_width + 2 * border;
    int			fps_width = XTextWidth(textFont, "WM", 2)
				    + extra_width + 2 * border;
    int			status_width = XTextWidth(textFont, "Stat", 4)
				    + extra_width + 2 * border;
    int			version_width = XTextWidth(textFont,
						   "4.2.0alpha7",
						   max_version_length)
				    + extra_width + 2 * border;
    int			map_width = XTextWidth(textFont,
					       "WMWMabcdefghijklmnopqrstuvwxyz",
					       max_map_length)
				    + extra_width + 2 * border;
    int			server_width = XTextWidth(buttonFont, server_header,
						  strlen(server_header));
    int			server_border_width = 2 * (border ? border : 1);
    int			ping_width = XTextWidth(textFont, ping_header,
						strlen(ping_header));
    int			xoff = space_width;
    int			yoff = space_height;
    int			text_height = textFont->ascent + textFont->descent;
    int			button_height = buttonFont->ascent + buttonFont->descent;
    int			label_height = MAX(text_height, button_height)
				     + extra_height + 2 * border;
    int			player_offset = xoff;
    int			queue_offset = player_offset + player_width + space_width;
    int			bases_offset = queue_offset + queue_width + space_width;
    int			team_offset = bases_offset + bases_width + space_width;
    int			fps_offset = team_offset + team_width + space_width;
    int			status_offset = fps_offset + fps_width + space_width;
    int			version_offset = status_offset + status_width + space_width;
    int			map_offset = version_offset + version_width + space_width;
    int			server_offset = map_offset + map_width + space_width;
    int			ping_offset;
    int			w;
    int			subform_width = 0;
    int			subform_height = 0;
    server_info_t	*sip;
    list_iter_t		start_server_it = server_it;

    Widget_get_dimensions(subform_widget, &subform_width, &subform_height);
    /*
    server_width = MAX(server_width, subform_width - server_offset - space_width);
    */
    server_width = MIN(server_width, subform_width - server_offset - space_width
				    - ping_width - space_width - server_border_width);
    ping_offset = server_offset + server_width + space_width + server_border_width;

    Widget_destroy_children(subform_widget);

    w = Widget_create_label(subform_widget,
			    player_offset, yoff,
			    player_width, label_height,
			    border, player_header);
    if (!w) {
	return -1;
    }
    Widget_create_label(subform_widget,
			queue_offset, yoff,
			queue_width, label_height,
			border, queue_header);
    Widget_create_label(subform_widget,
			bases_offset, yoff,
			bases_width, label_height,
			border, bases_header);
    Widget_create_label(subform_widget,
			team_offset, yoff,
			team_width, label_height,
			border, team_header);
    Widget_create_label(subform_widget,
			fps_offset, yoff,
			fps_width, label_height,
			border, fps_header);
    Widget_create_label(subform_widget,
			status_offset, yoff,
			status_width, label_height,
			border, status_header);
    Widget_create_label(subform_widget,
			version_offset, yoff,
			version_width, label_height,
			border, version_header);
    Widget_create_label(subform_widget,
			map_offset, yoff,
			map_width, label_height,
			border, map_header);
    Widget_create_label(subform_widget,
			server_offset, yoff,
			/* server_width, label_height, */
			server_width + server_border_width - 2*border, label_height,
			border, server_header);
    Widget_create_label(subform_widget,
			ping_offset, yoff,
			ping_width, label_height,
			border, ping_header);

    /* Widget_map_sub(subform_widget);
       Welcome_process_exposure_events(); */

    for (; server_it != List_end(server_list); LI_FORWARD(server_it)) {
	yoff += label_height + space_height;
	if (yoff + 2 * label_height + 3 * space_height >= subform_height) {
	    break;
	}
	sip = SI_DATA(server_it);
	Widget_create_label(subform_widget,
			    player_offset, yoff,
			    player_width, label_height,
			    border, sip->users ? sip->users_str : "");
	Widget_create_label(subform_widget,
			    queue_offset, yoff,
			    queue_width, label_height,
			    border, sip->queue ? sip->queue_str : "");
	Widget_create_label(subform_widget,
			    bases_offset, yoff,
			    bases_width, label_height,
			    border, sip->bases_str);
	Widget_create_label(subform_widget,
			    team_offset, yoff,
			    team_width, label_height,
			    border,
			    (sip->teambases > 0) ? sip->teambases_str : "");
	Widget_create_label(subform_widget,
			    fps_offset, yoff,
			    fps_width, label_height,
			    border, sip->fps_str);
	if (strlen(sip->status) > 4) {
	    sip->status[4] = '\0';
	}
	Widget_create_label(subform_widget,
			    status_offset, yoff,
			    status_width, label_height,
			    border,
			    strcmp(sip->status, "ok") ? sip->status : "");
	if (strlen(sip->version) > max_version_length) {
	    sip->version[max_version_length] = '\0';
	}
	string_to_lower(sip->version);
	Widget_create_label(subform_widget,
			    version_offset, yoff,
			    version_width, label_height,
			    border, sip->version);
	Widget_create_label(subform_widget,
			    map_offset, yoff,
			    map_width, label_height,
			    border, sip->mapname);
	Widget_create_activate(subform_widget,
			       server_offset,
			       yoff - (border == 0),
			       server_width, label_height,
			       border ? border : 1, sip->hostname,
			       Internet_server_join_cb, (void *) sip);
	sprintf(sip->pingtime_str, "%4d", sip->pingtime);
	Widget_create_label(subform_widget,
			    ping_offset, yoff,
			    ping_width, label_height,
			    border, (sip->pingtime == PING_NORESP)
				    ? "none"
				    : ((sip->pingtime == PING_SLOW)
					? "slow"
					: ((sip->pingtime == PING_UNKNOWN)
					    ? ""
					    : sip->pingtime_str)));
    }

    if (server_it != List_end(server_list)) {
	int		height_avail = subform_height - yoff;
	static char	next_text[] = "Next Page";
	static char	first_text[] = "First Page";
	int		next_border = border ? border : 1;
	int		first_border = next_border;
	int		next_width = XTextWidth(buttonFont,
						next_text,
						strlen(next_text))
				   + extra_width + 2 * next_border;
	int		first_width = XTextWidth(buttonFont,
						 first_text,
						 strlen(first_text))
				    + extra_width + 2 * first_border;
	int		next_height = label_height + 2 * (next_border - border);
	int		first_height = next_height;
	int		next_x_offset = (height_avail - next_height + 1) / 2;
	int		next_y_offset = yoff + (height_avail - next_height + 1) / 2;
	int		first_x_offset = next_x_offset + next_width + next_x_offset;
	int		first_y_offset = next_y_offset;

	Widget_create_activate(subform_widget,
			       next_x_offset, next_y_offset,
			       next_width, next_height,
			       next_border, next_text,
			       Internet_next_page_cb, (void *) conpar);
	if (start_server_it != List_begin(server_list)) {
	    Widget_create_activate(subform_widget,
				   first_x_offset, first_y_offset,
				   first_width, first_height,
				   first_border, first_text,
				   Internet_first_page_cb, (void *) conpar);
	}
    }
    else if (start_server_it != List_begin(server_list)) {
	static char	first_text[] = "First Page";
	int		first_border = border ? border : 1;
	int		first_width = XTextWidth(buttonFont,
						 first_text,
						 strlen(first_text))
				    + extra_width + 2 * first_border;
	int		first_height = label_height + 2 * (first_border - border);
	int		first_x_offset = (first_height + 1) / 2;
	int		first_y_offset = subform_height - first_x_offset - first_height;

	Widget_create_activate(subform_widget,
			       first_x_offset, first_y_offset,
			       first_width, first_height,
			       first_border, first_text,
			       Internet_first_page_cb, (void *) conpar);
    }
    {
	static char	ping_text[] = "Measure Lag";
	int		ping_border = border ? border : 1;
	int		ping_width = XTextWidth(buttonFont,
						 ping_text,
						 strlen(ping_text))
				    + extra_width + 2 * ping_border;
	int		ping_height = label_height + 2 * (ping_border - border);
	int		ping_pad = (ping_height + 1) / 2;
	int		ping_x_offset = subform_width - ping_width - ping_pad;
	int		ping_y_offset = subform_height - ping_height - ping_pad;

	Widget_create_activate(subform_widget,
			       ping_x_offset, ping_y_offset,
			       ping_width, ping_height,
			       ping_border, ping_text,
			       Internet_ping_cb, (void *) conpar);
    }

    Widget_map_sub(subform_widget);

    return -1;
}

/* 
 * Cleanup when leaving the mode ModeLocalnet.
 */
static void Internet_cleanup(void)
{
    Delete_server_list();
}

/*
 * User pressed the Internet button.
 */
static int Internet_cb(int widget, void *user_data, const char **text)
{
    Connect_param_t	*conpar = (Connect_param_t *) user_data;

    Welcome_set_mode(ModeInternet);

    if (!server_list ||
	List_size(server_list) < 10 ||
	server_list_creation_time + 5 < time(NULL)) {

	Delete_server_list();

	if (Get_meta_data() <= 0) {
	    return 0;
	}
	/* Ping_servers(); */

	if (Welcome_sort_server_list() == -1) {
	    Delete_server_list();
	    Welcome_create_label(1, "Not enough memory.");
	}
    }

    server_it = List_begin(server_list);
    Welcome_show_server_list(conpar);

    return 0;
}

/*
 * User pressed the Server button.
 */
#if 0
static int Server_cb(int widget, void *user_data, const char **text)
{
    /* Connect_param_t	*conpar = (Connect_param_t *) user_data; */

    Welcome_set_mode(ModeServer);

    return 0;
}
#endif

/*
 * User pressed the Help button.
 */
#if 0
static int Help_cb(int widget, void *user_data, const char **text)
{
    Welcome_set_mode(ModeHelp);

    /* Proper help about this welcome screen should be displayed.
     * For now just popup the about window.
     * Hmm, about buttons don't work.  They should become widgets.

    About_callback(0,0,0);
     */

    return 0;
}
#endif

/*
 * User pressed the Quit button.
 */
static int Quit_cb(int widget, void *user_data, const char **text)
{
    Welcome_set_mode(ModeQuit);

    quitting = 1;

    return 0;
}

/*
 * Create toplevel widgets.
 */
static int Welcome_create_windows(Connect_param_t *conpar)
{
    int			i;
    int			form_border = 0;
    int			form_x = 0;
    int			form_y = 0;
    int			form_width = top_width - 2 * form_border;
    int			form_height = top_height - 2 * form_border;
    int			subform_x;
    int			subform_y;
    int			subform_width;
    int			subform_height;
    int			subform_border;
    int			button_border = 4;
    int			button_height = buttonFont->ascent
				      + buttonFont->descent
				      + 2 * button_border;
    int			max_width;
    int			text_width;
    int			button_x;
    int			button_y;
    int			button;
    int			min_height_needed;
    int			height_available;
    int			max_height_wanted;
    int			height_per_button;
    struct MyButton {
	const char	*text;
	int		(*callback)(int, void *, const char **);
    };
    struct MyButton my_buttons[] = {
	{ "Local", Localnet_cb },
	{ "Internet", Internet_cb },
#if 0
/* XXX TODO add server page to select a map and start a server. */
	{ "Server", Server_cb },
/* XXX TODO add help page . */
	{ "Help", Help_cb },
#endif
	{ "Quit", Quit_cb },
    };

    LIMIT(form_width, 400, 1282);
    LIMIT(form_height, 400, 1024);
    form_widget =
	Widget_create_form(0, top,
			   form_x, form_y,
			   form_width, form_height,
			   form_border);
    if (form_widget == NO_WIDGET) {
	return -1;
    }
    Widget_set_background(form_widget, BLACK);

    max_width = 0;
    for (i = 0; i < NELEM(my_buttons); i++) {
	text_width = XTextWidth(buttonFont,
				my_buttons[i].text,
				strlen(my_buttons[i].text));
	if (text_width > max_width) {
	    max_width = text_width;
	}
    }
    max_width += 20;

    min_height_needed = NELEM(my_buttons) * button_height;
    height_available = form_height;
    max_height_wanted = NELEM(my_buttons) * (button_height + 40);
    LIMIT(height_available, min_height_needed, max_height_wanted);
    height_per_button = height_available / NELEM(my_buttons);
    button_y = height_per_button - button_height;
    button_x = 20;
    for (i = 0; i < NELEM(my_buttons); i++) {
	button =
	    Widget_create_activate(form_widget,
				   button_x, button_y,
				   max_width, button_height + 20,
				   1, my_buttons[i].text,
				   my_buttons[i].callback,
				   (void *) conpar);
	if (button == NO_WIDGET) {
	    return -1;
	}
	button_y += height_per_button;
    }

    subform_x = 2 * button_x + max_width;
    subform_y = button_x;
    subform_border = 1;
    subform_width = form_width - subform_x - subform_y - 2 * subform_border;
    subform_height = form_height - 2 * subform_y - 2 * subform_border;
    subform_widget =
	Widget_create_form(form_widget, 0,
			   subform_x, subform_y,
			   subform_width, subform_height,
			   subform_border);
    if (subform_widget == NO_WIDGET) {
	return -1;
    }
    Widget_set_background(subform_widget, BLACK);

    Welcome_create_label(1, "Welcome to XPilot!");

    Widget_map_sub(form_widget);
    XMapSubwindows(dpy, top);

    return 0;
}

/*
 * Close all windows.
 */
static void Welcome_destroy_windows(void)
{
    Widget_destroy(form_widget);
    XFlush(dpy);
    form_widget = NO_WIDGET;
    subform_widget = NO_WIDGET;
}

/*
 * Cleanup everything.
 */
static void Welcome_cleanup(void)
{
    Welcome_destroy_windows();
    Delete_server_list();
}

/*
 * Change to a new subfunction mode.
 * Possibly call cleanup handlers for the mode
 * which we are leaving.
 */
static void Welcome_set_mode(enum Welcome_mode new_welcome_mode)
{
    int			old_welcome_mode = welcome_mode;

    Widget_destroy_children(subform_widget);

    switch (old_welcome_mode) {
    case ModeWaiting:
	break;
    case ModeLocalnet:
	Localnet_cleanup();
	break;
    case ModeInternet:
	if (new_welcome_mode != old_welcome_mode) {
	    Internet_cleanup();
	}
	break;
    case ModeServer:
	break;
    case ModeHelp:
	break;
    case ModeQuit:
	break;
    default:
	break;
    }

    welcome_mode = new_welcome_mode;
}

/*
 * Process one event.
 */
static int Welcome_process_one_event(XEvent *event)
{
    XClientMessageEvent	*cmev;
    XConfigureEvent	*conf;

    switch (event->type) {

    case ClientMessage:
	cmev = (XClientMessageEvent *) event;
	if (cmev->message_type == ProtocolAtom
	    && cmev->format == 32
	    && cmev->data.l[0] == KillAtom) {
	    /*
	     * On HP-UX 10.20 with CDE strange things happen
	     * sometimes when closing xpilot via the window
	     * manager.  Keypresses may result in funny characters
	     * after the client exits.  The remedy to this seems
	     * to be to explicitly destroy the top window with
	     * XDestroyWindow when the window manager asks the
	     * client to quit and then wait for the resulting
	     * DestroyNotify event before closing the connection
	     * with the X server.
	     */
	    XDestroyWindow(dpy, top);
	    XSync(dpy, True);
	    printf("Quit\n");
	    return -1;
	}
	break;

#ifndef _WINDOWS
    case MapNotify:
	if (ignoreWindowManager == 1) {
	    XSetInputFocus(dpy, top, RevertToParent, CurrentTime);
	    ignoreWindowManager = 2;
	}
	break;

	/* Back in play */
    case FocusIn:
	gotFocus = true;
	XAutoRepeatOff(dpy);
	break;

	/* Probably not playing now */
    case FocusOut:
    case UnmapNotify:
	gotFocus = false;
	XAutoRepeatOn(dpy);
	break;

    case MappingNotify:
	XRefreshKeyboardMapping(&event->xmapping);
	break;

    case ConfigureNotify:
	conf = &event->xconfigure;
	if (conf->window == top) {
	    top_width = conf->width;
	    top_height = conf->height;
	    LIMIT(top_width, MIN_TOP_WIDTH, MAX_TOP_WIDTH);
	    LIMIT(top_height, MIN_TOP_HEIGHT, MAX_TOP_HEIGHT);
#if 0
	    Welcome_destroy_windows();
	    if (Welcome_create_windows(conpar) == -1) {
		return -1;
	    }
#endif
	}
	else {
	    Widget_event(event);
	}
	break;
#endif

    default:
	Widget_event(event);
	break;
    }
    if (quitting) {
	quitting = 0;
	return -1;
    }
    if (joining) {
	return 1;
    }

    return 0;
}

/*
 * Process all events which are in the queue, but don't block.
 */
static int Welcome_process_pending_events(Connect_param_t *conpar)
{
    int			result;
    XEvent		event;

    while (XEventsQueued(dpy, QueuedAfterFlush) > 0) {
	XNextEvent(dpy, &event);
	result = Welcome_process_one_event(&event);
	if (result != 0) {
	    return result;
	}
    }
    return 0;
}

/*
 * Loop forever processing events.
 */
static int Welcome_input_loop(Connect_param_t *conpar)
{
    int			result;
    XEvent		event;

    while (!quitting && !joining) {
	XNextEvent(dpy, &event);
	result = Welcome_process_one_event(&event);
	if (result != 0) {
	    return result;
	}
    }

    return -1;
}

/*
 * Create the windows.
 */
static int Welcome_doit(Connect_param_t *conpar)
{
    int			result;

#if 0
    XSynchronize(dpy, True);
#endif
    if (Init_top() == -1) {
	return -1;
    }
    XMapSubwindows(dpy, top);
    XMapWindow(dpy, top);
    XSync(dpy, False);

    result = Welcome_process_pending_events(conpar);
    if (result) {
	return result;
    }

    if (Welcome_create_windows(conpar) == -1) {
	return -1;
    }

    result = Welcome_process_pending_events(conpar);
    if (result) {
	return result;
    }

    result = Welcome_input_loop(conpar);
    return result;
}

/*
 * The one and only entry point into this modules.
 */
int Welcome_screen(Connect_param_t *conpar)
{
    int			result;

    /* save pointer so that join callbacks can copy into it. */
    global_conpar = conpar;

    result = Welcome_doit(conpar);

    if (!quitting && joining) {
	Welcome_cleanup();
	result = Join(conpar->server_addr,
		      conpar->server_name,
		      conpar->login_port,
		      conpar->real_name,
		      conpar->nick_name,
		      conpar->team,
		      conpar->disp_name,
		      conpar->server_version);
    } else {
	Quit();
    }

    return result;
}

