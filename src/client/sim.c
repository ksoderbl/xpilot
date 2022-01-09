/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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

int simulating;

#ifdef SIMULATING_ONLY

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# include <X11/Xlib.h>
#endif

#include "xpconfig.h"
#include "const.h"
#include "setup.h"
#include "error.h"
#include "rules.h"
#include "bit.h"
#include "netclient.h"
#include "paint.h"
#include "xinit.h"
#include "pack.h"
#include "protoclient.h"
#include "portability.h"
#include "commonproto.h"

DFLOAT findDir(DFLOAT x, DFLOAT y);

typedef struct rpos_s {
	DFLOAT x, y;
	DFLOAT vx, vy;
	DFLOAT ax, ay;
	int last_dir_change;
} rpos_t;

Rpos_Update(rpos_t *rp, int loops) {
	rp->x += rp->vx;
	rp->y += rp->vy;
	while (rp->x < 0) rp->x += Setup->width;
	while (rp->y < 0) rp->y += Setup->height;
	while (rp->x > Setup->width) rp->x -= Setup->width;
	while (rp->y > Setup->height) rp->y -= Setup->height;
	if ((loops - rp->last_dir_change) > 30 + rfrac() * 30) {
		rp->ax = rfrac();
		rp->ay = rfrac();
		rp->last_dir_change = loops;
	}
	rp->vx += (rfrac() - rp->ax) * 20;
	rp->vy += (rfrac() - rp->ay) * 20;
	rp->vx *= 0.65;
	rp->vy *= 0.65;
}


/*
 * This should be a nice random map generator and it should go in common
 * and the server should use it.
 */
static int Random_map(void)
{
	int i;
	unsigned char *data;
	unsigned char typemap[] = {
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_SPACE, SETUP_SPACE, SETUP_SPACE, SETUP_SPACE,
		SETUP_FILLED, SETUP_FILLED, SETUP_FILLED,
		SETUP_DECOR_FILLED, SETUP_DECOR_LU,
		SETUP_REC_RU, SETUP_REC_RD,
		SETUP_CANNON_UP,
		SETUP_TREASURE,
	};

	for (i = Setup->x * Setup->y, data = &Setup->map_data[0] ; i-- > 0; ++data) {
		int type = (int) (rfrac() * sizeof(typemap));

		*data = typemap[type];
	}
}

#include "simbloods.h"

static int Fake_setup(void)
{
	const int x = 100, y = 100;

	if ((Setup = (setup_t *) malloc(sizeof(setup_t) + x * y)) == NULL) {
		error("No memory for setup data");
		return -1;
	}
	Setup->mode = WRAP_PLAY;
	Setup->lives = 1;
	Setup->x = x;
	Setup->y = y;
	Setup->width = x * BLOCK_SZ;
	Setup->height = y * BLOCK_SZ;
	Setup->frames_per_second = 12; /* unused? */
	strlcpy(Setup->name, "Simulated Map", sizeof(Setup->name));
	strlcpy(Setup->author, "Ben Jackson <ben@ben.com>",
		sizeof(Setup->author));
#if RANDOM_MAP
	Random_map();
#else
	memcpy(&Setup->map_data[0], bloods_music, Setup->x * Setup->y);
	Map_restore(0, 0, Setup->x, Setup->y);
#endif
}

#define N_FAKE_SHIPS 8

static rpos_t fake_ships[N_FAKE_SHIPS];

static void Fake_others(void)
{
	extern char name[];
	char fakename[15];
	int i;

	for (i = 0; i < N_FAKE_SHIPS; ++i) {
		if (i != 0) {
			int namelen;
			char *p;

			namelen = rfrac() * (sizeof(fakename) - 2) + 1;
			p = fakename;
			while (namelen--) {
				*p++ = 'a' + (int)(rfrac() * 26);
			}
			*p = '\0';
		} else {
			strlcpy(fakename, name, sizeof(fakename));
		}

		Handle_player(i, i > 3 ? 4 : 2, ' ', fakename,
				"fake", "fake.org", "");
	}
}

static int Simulate_init(void)
{
	simulating = 1;
	Client_init("simulator", MY_VERSION);
	Net_init(NULL, 0);
	Fake_setup();
	Fake_others();
	Client_setup();
	/* Net_start() */
	Client_start();
}

static alarmed = 0;

static int zot(void)
{
	alarmed = 1;
}

static int Simulate_frames(void)
{
	const int duration = 2;
	u_byte newitems[NUM_ITEMS];
	int i, old_i;
	int j;

	Client_score_table();
	Handle_message("Starting Test");
	Game_over_action(PLAYING);
	Send_display();

	bzero(newitems, sizeof(newitems));
	newitems[ITEM_AFTERBURNER] = 2;
	newitems[ITEM_AUTOPILOT] = 1;

	signal(SIGALRM, zot);

	alarm(duration);
	old_i = i = 0;
	while (1) {
		if (alarmed) {
			char buf[80];
			alarmed = 0;
			sprintf(buf, "%d frames/sec", (i - old_i) / duration);
			Handle_message(buf);
			alarm(duration);
			old_i = i;
		}
		Handle_start(i);
		Send_display();
		Handle_self((int)fake_ships[0].x, (int)fake_ships[0].y, (int)fake_ships[0].vx, (int)fake_ships[0].vy, 
			(int)findDir(fake_ships[0].vx, fake_ships[0].vy),
		0.0, 0.0, 0.0, 1, 0, 0, 0, 0, 
			newitems, 0, 
		1000 << FUEL_SCALE_BITS,
		MAX_PLAYER_FUEL,
			1400);
		for (j = 0; j < N_FAKE_SHIPS; ++j) {
			Handle_ship((int)fake_ships[j].x, (int)fake_ships[j].y, j, (int)findDir(fake_ships[j].vx, fake_ships[j].vy), 0, 0, 0, 0, 0);
			Handle_radar((int)fake_ships[j].x, (int)fake_ships[j].y, 3);
			Rpos_Update(&fake_ships[j], i);
		}
		Handle_end(i);
		if (Client_input(2) == -1) {
			xpilotShutdown();
			return;
		}
		Client_sync();
		++i;
	}
}

void Simulate(void)
{
    simulating = 1;
    Simulate_init();
    Simulate_frames();
    exit(0);
}

#else

void Simulate(void);

void Simulate(void)
{
    simulating = 0;
}

#endif /* SIMULATING_ONLY */
