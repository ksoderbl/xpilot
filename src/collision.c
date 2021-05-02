/* $Id: collision.c,v 1.19 1993/04/02 20:34:52 kenrsc Exp $
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@staff.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 *
 * $NCDId: @(#)collision.c,v 1.21 1992/09/05 00:02:58 mellon Exp $
 */

#include "global.h"
#include "map.h"
#include "score.h"
#include "robot.h"

#ifndef	lint
static char sourceid[] =
    "@(#)$Id: collision.c,v 1.19 1993/04/02 20:34:52 kenrsc Exp $";
#endif

#define in_range(o1, o2, r)	\
    (ABS((o1)->intpos.x-(o2)->intpos.x)<(r) && \
     ABS((o1)->intpos.y-(o2)->intpos.y)<(r)) \


extern long KILLING_SHOTS;
static char msg[MSG_LEN];

static void WallCollide(object *obj, int x, int y,
			int count, int max, int axis);
static bool Landing(int ind, int point, int blockdir);
static int Rate(int winner, int looser);
static void ObjectCollision(void);
static void PlayerCollision(void);
static void PlayerObjectCollision(int ind);



static int Rate(int winner, int loser)
{
    int t;

    t = ((RATE_SIZE/2) * RATE_RANGE) / (ABS(loser-winner) + RATE_RANGE);
    if (loser > winner)
	t = RATE_SIZE - t;
    return (t);
}


void Check_collision(void)
{
    object *obj;
    int i;
    player *pl;

    for (i=0; i<NumObjs; i++) {
	obj = Obj[i];
	obj->intpos.x = obj->pos.x;
	obj->intpos.y = obj->pos.y;
    }
    for (i=0; i<NumPlayers; i++) {
	pl = Players[i];
	pl->intpos.x = pl->pos.x;
	pl->intpos.y = pl->pos.y;
    }
    ObjectCollision();
    PlayerCollision();
}


static void ObjectCollision(void)
{
    int		i, x, y, t;


    /* Shot - wall, and out of bounds */
    for (i=0; i<NumObjs; i++) {
	int sx, sy;
	int dx, dy;
	int placed = Obj[i]->placed;
	int wrapped = Obj[i]->wrapped;
	int start;

	x = (int)Obj[i]->pos.x / BLOCK_SZ;
	y = (int)Obj[i]->pos.y / BLOCK_SZ;
	sx = (int)(Obj[i]->prevpos.x / BLOCK_SZ);
	sy = (int)(Obj[i]->prevpos.y / BLOCK_SZ);
	dx = x - sx;
	dy = y - sy;

	if (dx == 0 && dy == 0) {
	    if (placed) {
		WallCollide(Obj[i], sx, sy, 0, 0, 0);
	    }
	} else {
	    if (ABS(dx) > ABS(dy)) {
		if (dx > 0) {
		    if (wrapped)
			start = dx;
		    else
			start = !placed;

		    for (t = start; t <= dx && Obj[i]->life > 0; t++) {
			WallCollide(Obj[i], sx + t, sy + dy * t/dx, t, dx, 1);
		    }
		} else {
		    if (wrapped)
			start = dx;
		    else
			start = -!placed;

		    for (t = start; t >= dx && Obj[i]->life > 0; t--) {
			WallCollide(Obj[i], sx + t, sy + dy * t/dx, t, dx, 1);
		    }
		}
	    } else {
		if (dy > 0) {
		    if (wrapped)
			start = dy;
		    else
			start = !placed;

		    for (t = start; t <= dy && Obj[i]->life > 0; t++) {
			WallCollide(Obj[i], sx + dx * t/dy, sy + t, t, dy, 0);
		    }
		} else {
		    if (wrapped)
			start = dy;
		    else
			start = -!placed;

		    for (t = start; t >= dy && Obj[i]->life > 0; t--) {
		        WallCollide(Obj[i], sx + dx * t/dy, sy + t, t, dy, 0);
		    }
		}
	    }
	}
    }
}


static void PlayerCollision(void)
{
    int		i, j, x, y, sc, t;
    player	*pl;
    int		xd, yd;


    /* Player - player, checkpoint, treasure, object and wall */
    for (i=0; i<NumPlayers; i++) {
	pl = Players[i];
	if (!BIT(pl->status, PLAYING) || BIT(pl->status, GAME_OVER))
	    continue;

	if (pl->pos.x < 0 || pl->pos.y < 0
	    || pl->pos.x >= World.x * BLOCK_SZ
	    || pl->pos.y >= World.y * BLOCK_SZ) {
	    SET_BIT(pl->status, KILLED);
	    sprintf(msg, "%s left the known universe.", pl->name);
	    Set_message(msg);
	    SCORE(i, -Rate(WALL_RATING, pl->score));
	    continue;
	}

	/* Player - player */
	if (BIT(World.rules->mode, CRASH_WITH_PLAYER)) {
	    for (j=i+1; j<NumPlayers; j++)
		if (BIT(Players[j]->status, PLAYING)
		    && !BIT(Players[j]->status, GAME_OVER)
		    && in_range((object *)pl, (object *)Players[j],
				2*SHIP_SZ-6)) {
		    if (!TEAM(i, j) && !PSEUDO_TEAM(i,j)) {
			Add_fuel(&(pl->fuel), ED_PL_CRASH);
			Add_fuel(&(Players[j]->fuel), ED_PL_CRASH);
			Item_damage(i);
			Item_damage(j);
			pl->forceVisible = 20;
			Players[j]->forceVisible = 20;
		/*	Delta_mv((object *)pl, (object *)Players[j]);	*/
			Obj_repel((object *)pl, (object *)Players[j],
				  2*SHIP_SZ);
			if (pl->fuel.sum <= 0 || !BIT(pl->used, OBJ_SHIELD))
			    SET_BIT(pl->status, KILLED);
			if (Players[j]->fuel.sum <= 0
			    || !BIT(Players[j]->used, OBJ_SHIELD))
			    SET_BIT(Players[j]->status, KILLED);

			if (BIT(Players[j]->status, KILLED)) {
			    if (BIT(pl->status, KILLED)) {
				sprintf(msg, "%s and %s crashed.",
					pl->name, Players[j]->name);
				sc = Rate(pl->score, Players[j]->score) / 3;
				SCORE(i, -sc); SCORE(j, -sc);
			    } else {
				sprintf(msg, "%s ran over %s.",
					pl->name, Players[j]->name);
				sc = Rate(pl->score, Players[j]->score) / 3;
				SCORE(i, sc); SCORE(j, -sc);
			    }
			    Set_message(msg);
			} else {
			    if (BIT(pl->status, KILLED)) {
				sprintf(msg, "%s ran over %s.",
				        Players[j]->name, pl->name);
				sc = Rate(Players[j]->score, pl->score) / 3;
				SCORE(j, sc); SCORE(i, -sc);
				Set_message(msg);
			    }
			}

			if (Players[j]->robot_mode != RM_NOT_ROBOT
			    && BIT(Players[j]->status, KILLED)
			    && Players[j]->robot_lock == LOCK_PLAYER
			    && Players[j]->robot_lock_id == pl->id)
			    Players[j]->robot_lock = LOCK_NONE;

			if (pl->robot_mode != RM_NOT_ROBOT
			    && BIT(pl->status, KILLED)
			    && pl->robot_lock == LOCK_PLAYER
			    && pl->robot_lock_id == Players[j]->id)
			    pl->robot_lock = LOCK_NONE;
		    }
		}
	}

	/* Player checkpoint */
	if (BIT(World.rules->mode, TIMING))
	    if (LENGTH(pl->pos.x - World.check[pl->check].x*BLOCK_SZ,
		       pl->pos.y - World.check[pl->check].y*BLOCK_SZ) < 200) {

		if (pl->check == 0) {
		    pl->round++;
		    if (((pl->best_lap > pl->time - pl->last_lap)
			 || (pl->best_lap == 0))
			&& (pl->time != 0)) {
			pl->best_lap = pl->time - pl->last_lap;
		    }
		    pl->last_lap_time = pl->time - pl->last_lap;
		    pl->last_lap = pl->time;
		}

		pl->check++;

		if (pl->check == World.NumChecks)
		    pl->check = 0;
	    }

	/* Player picking up ball/treasure */
	if (!BIT(pl->used, OBJ_CONNECTOR)) {
	    pl->ball = NULL;
	} else {
	    if (pl->ball != NULL) {
		if ((LENGTH(pl->pos.x - pl->ball->pos.x,
			    pl->pos.y - pl->ball->pos.y) 
		     > BALL_STRING_LENGTH)) {
		    pl->ball->id = pl->id;
		    pl->ball->owner = pl->id;
		    pl->ball->status = GRAVITY;
		    if (pl->ball->treasure != -1)
			World.treasures[pl->ball->treasure].have = false;
		    SET_BIT(pl->have, OBJ_BALL);
		    pl->ball = NULL;
		}
	    } else if (!BIT(pl->have, OBJ_BALL)) {
		for (j=0 ; j < NumObjs; j++) {
		    if (BIT(Obj[j]->type, OBJ_BALL) && Obj[j]->id == -1) {
			if (((LENGTH(pl->pos.x - Obj[j]->pos.x, 
				     pl->pos.y - Obj[j]->pos.y) 
			      < BALL_STRING_LENGTH))) {
			    pl->ball = Obj[j];
			    break;
			}
		    }
		}
	    }
	}
  
	PlayerObjectCollision(i);

	/* Player - wall */
	if (!(BIT(pl->used, OBJ_TRAINER) || BIT(pl->status, KILLED))) {
	    for(j=0; j<3 && !BIT(pl->status, KILLED) &&
		!BIT(pl->status, WARPING); j++) {
		x = (int)((pl->pos.x + ships[pl->dir].pts[j].x) / BLOCK_SZ);
		y = (int)((pl->pos.y + ships[pl->dir].pts[j].y) / BLOCK_SZ);
		if (x < 0 || x >= World.x || y < 0 || y >= World.y) {
#ifdef FOO
		    sprintf(msg, "%s left the known universe.", pl->name);
		    Set_message(msg);
		    SET_BIT(pl->status, KILLED);
		    SCORE(i, -Rate (WALL_RATING, pl->score));
#endif
		}
		else
		    switch (World.block[x][y]) {
			/* NOTE:
			 * These should be modified so that it is called with
			 * the direction (outward normal) of the side crossed
			 * by the player.  Also, should be called for every
			 * block the fighter has been into this tick.
			 */
		    case FUEL:
		    case FILLED:
		    case FILLED_NO_DRAW:
			if (!Landing(i, j, 0)) {
			    SET_BIT(pl->status, KILLED);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			}
			break;
		    case TREASURE:
			{
			    int t;

			    for(t = 0; t < World.NumTreasures; t++) {
				if (World.treasures[t].pos.x == x
				    && World.treasures[t].pos.y == y) {
				    SET_BIT(pl->status, KILLED);
				    sprintf(msg,
					    "%s crashed with a treasure.",
					    pl->name);
				    Set_message(msg);
				    SCORE(i, -Rate(WALL_RATING,pl->score));
				}
			    }
			}
			break;
		    case REC_LU:
			if ((((int)(pl->pos.x
				    +ships[pl->dir].pts[j].x))
			     % BLOCK_SZ)
			    <= (((int)(pl->pos.y
				       +ships[pl->dir].pts[j].y))
				% BLOCK_SZ)) {
			    if (!Landing(i, j, 7*RES/8)) {
				SET_BIT(pl->status, KILLED);
				SCORE(i, -Rate(WALL_RATING, pl->score));
			    }
			}
			break;
		    case REC_RU:
			if ((((int)(pl->pos.x
				    +ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			    >= BLOCK_SZ - (((int)(pl->pos.y
						  +ships[pl->dir].pts[j].y))
					   % BLOCK_SZ)) {
			    if (!Landing(i, j, 5*RES/8)) {
				SET_BIT(pl->status, KILLED);
				SCORE(i, -Rate(WALL_RATING, pl->score));
			    }
			}
			break;
		    case REC_LD:
			if ((((int)(pl->pos.x
				    +ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			    <= BLOCK_SZ - (((int)(pl->pos.y
						  +ships[pl->dir].pts[j].y))
					   % BLOCK_SZ)) {
			    if (!Landing(i, j, RES/8)) {
				SET_BIT(pl->status, KILLED);
				SCORE(i, -Rate(WALL_RATING, pl->score));
			    }
			}
			break;
		    case REC_RD:
			if ((((int)(pl->pos.x
				    +ships[pl->dir].pts[j].x)) % BLOCK_SZ)
			    >= (((int)(pl->pos.y
				       +ships[pl->dir].pts[j].y))
				% BLOCK_SZ)) {
			    if (!Landing(i, j, 3*RES/8)) {
				SET_BIT(pl->status, KILLED);
				SCORE(i, -Rate(WALL_RATING, pl->score));
			    }
			}
			break;
		    case CANNON:
			xd = pl->pos.x + ships[pl->dir].pts[j].x;
			yd = pl->pos.y + ships[pl->dir].pts[j].y;

			for(t = 0;
			    World.cannon[t].pos.x != x
			    || World.cannon[t].pos.y != y;
			    t++);

			if (World.cannon[t].dead_time > 0)
			    break;

			if ((World.cannon[t].dir == DIR_UP
			     && yd%BLOCK_SZ < BLOCK_SZ/3)
			    || (World.cannon[t].dir == DIR_DOWN
				&& yd%BLOCK_SZ > 2*BLOCK_SZ/3)
			    || (World.cannon[t].dir == DIR_RIGHT
				&& xd%BLOCK_SZ < BLOCK_SZ/3)
			    || (World.cannon[t].dir == DIR_LEFT
				&& xd%BLOCK_SZ > 2*BLOCK_SZ/3)) {

			    SET_BIT(pl->status, KILLED);
			    sprintf(msg, "%s crashed with a cannon.",
				    pl->name);
			    SCORE(i, -Rate(WALL_RATING, pl->score));
			    Set_message(msg);

			    World.cannon[t].dead_time = CANNON_DEAD_TIME;
			    World.block
				[World.cannon[t].pos.x]
				    [World.cannon[t].pos.y] = SPACE;
			    World.cannon[t].active = false;
			    Explode_object((float)(x*BLOCK_SZ),
					   (float)(y*BLOCK_SZ),
					   World.cannon[t].dir, RES*0.4,
					   120);
			}

			break;
		    case WORMHOLE:
			{
			    int hole = wormXY(x, y);

			    if (World.wormHoles[hole].type != WORM_OUT) {
				SET_BIT(pl->status, WARPING);
				pl->wormHoleHit = hole;
			    }
			    break;
			}
		    default:
			break;
		    }
	    }
	    
	    /*
	     * Don't re-warp us if we've just warped.
	     * Let us get clear of the wormhole first.
	     */
	    if (BIT(pl->status, WARPED))
		if (BIT(pl->status, WARPING) &&
		    pl->wormHoleDest == pl->wormHoleHit) {
		    CLR_BIT(pl->status, WARPING);
		} else {
		    CLR_BIT(pl->status, WARPED);
		}
	    
	    if (BIT(pl->status, KILLED)
		&& pl->score < 0
		&& pl->robot_mode != RM_NOT_ROBOT 
                && pl->robot_mode != RM_OBJECT) {
		pl->home_base = -1;
		Pick_startpos(i);
	    }
	}
    }
}


static void PlayerObjectCollision(int i)
{
    int		j, killer, range, sc;
    player	*pl;
    char	*type;
    object	*obj;


    /*
     * Collision between a player and an object.
     */
    pl = Players[i]; 
    if (BIT(pl->status, PLAYING|GAME_OVER|KILLED) != PLAYING)
	return;

    for (j=0; j<NumObjs; j++) {
	obj = Obj[j];
	if (obj->life <= 0)
	    continue;

	if (BIT(obj->type, OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_MINE))
	    switch (obj->type) {
	    case OBJ_TORPEDO:
		if (pl->id == obj->id && obj->info < 8)
		    continue;
		else
		    range = SHIP_SZ + TORPEDO_RANGE;
		break;
	    case OBJ_SMART_SHOT:
	    case OBJ_HEAT_SHOT:
		range = SHIP_SZ + MISSILE_RANGE;
		break;
	    case OBJ_MINE:
		range = SHIP_SZ + MINE_RANGE;
		break;
	    }
	else
	    range = SHIP_SZ;
	

	if (!in_range((object *)pl, obj, range))
	    continue;

	if ((obj->type == OBJ_SPARK && obj->id == pl->id)
	    || (obj->type == OBJ_MINE && (obj->id != -1)
		&& (obj->id == pl->id || TEAM(GetInd[obj->id], i))))
	    continue;

	if ((obj->id != -1)
	    && (TEAM(i, GetInd[obj->id]))
	    && (pl->id != obj->id))
	    continue;
	else
	    obj->life = 0;
	
	Delta_mv((object *)pl, (object *)obj);

	/*
	 * Object collision.
	 */
	switch (obj->type) {
	case OBJ_WIDEANGLE_SHOT:
	    pl->extra_shots++;
	    break;
	case OBJ_BALL_PACK:
	    if (!BIT(pl->have, OBJ_BALL)) {
		SET_BIT(pl->have, OBJ_BALL);
		Make_ball(i, pl->pos.x, pl->pos.y-BALL_STRING_LENGTH,
			  true, -1);
	    }
	    break;
	case OBJ_ECM:
	    pl->ecms++;
	    break;
	case OBJ_SENSOR_PACK:
	    pl->sensors++;
	    pl->updateVisibility = 1;
	    break;
	case OBJ_AFTER_BURNER:
	    SET_BIT(pl->have, OBJ_AFTER_BURNER);
	    if (++pl->after_burners > MAX_AFTER_BURNER)
		pl->after_burners = MAX_AFTER_BURNER;
	    break;
	case OBJ_REAR_SHOT:
	    SET_BIT(pl->have, OBJ_REAR_SHOT);
	    pl->rear_shots++;
	    break;
	case OBJ_SMART_SHOT_PACK:
	    pl->missiles += 4;
	    break;
	case OBJ_CLOAKING_DEVICE:
	    SET_BIT(pl->have, OBJ_CLOAKING_DEVICE);
	    pl->cloaks++;
	    pl->updateVisibility = 1;
	    break;
	case OBJ_ENERGY_PACK:
	    Add_fuel(&(pl->fuel), ENERGY_PACK_FUEL);
	    pl->fuel.count = FUEL_NOTIFY;
	    break;
	case OBJ_MINE_PACK:
	    pl->mines += 1 + (rand()&1);
	    break;
	case OBJ_TANK: {
	    int c = pl->fuel.current;

	    if (pl->fuel.num_tanks < MAX_TANKS) {
		/*
		 * Set a new, empty tank in the list.
		 * update max-fuel
		 */
		int no = ++(pl->fuel.num_tanks);

		SET_BIT(pl->have, OBJ_TANK);
		pl->fuel.current = no;
		pl->fuel.max += TANK_CAP(no);
		pl->fuel.tank[no] = 0;
		pl->emptymass += TANK_MASS;
	    }
	    Add_fuel(&(pl->fuel), TANK_FUEL(pl->fuel.current));
	    pl->fuel.count = FUEL_NOTIFY;
	    pl->fuel.current = c;
	    break;
	}
	case OBJ_MINE:
	    if (obj->id == -1)
		sprintf(msg, "%s hit a %smine.",
			pl->name, obj->status ? "moving " : "");
	    else {
		sprintf(msg, "%s hit mine %s by %s.", pl->name,
			obj->status ? "thrown" : "dropped",
			Players[killer=GetInd[obj->id]]->name);
		sc = Rate(Players[killer]->score, pl->score) / 6;
		SCORE(killer, sc);
		SCORE(i, -sc);
	    }
	    Set_message(msg);
	    break;
	default:
	    break;
	}

	if (!BIT(obj->type, KILLING_SHOTS))
	    continue;

	type = NULL;

	if (BIT(pl->used, OBJ_SHIELD)) {
	    switch (obj->type) {

	    case OBJ_TORPEDO:
		type = "torpedo";
	    case OBJ_HEAT_SHOT:
		if (!type) type = "heat shot";
	    case OBJ_SMART_SHOT:
		if (!type) type = "smart shot";

		if (obj->id == -1)
		    sprintf(msg, "%s ate a %s.", pl->name, type);
		else
		    sprintf(msg, "%s ate a %s from %s.", pl->name, type,
			    Players[ killer=GetInd[obj->id] ]->name);

		Add_fuel(&(pl->fuel), ED_SMART_SHOT_HIT);
		pl->forceVisible += 2;
		Set_message(msg);
		break;

	    case OBJ_SHOT:
	    case OBJ_CANNON_SHOT:
		Add_fuel(&(pl->fuel), ED_SHOT_HIT);
		pl->forceVisible += 1;
		break;

	    default:
		printf("You were hit by what?\n");
		break;
	    }
	} else {
	    switch (obj->type) {
	    case OBJ_TORPEDO:
		Add_fuel(&(pl->fuel),ED_SMART_SHOT_HIT);
		if (rand()&3)
		    break;
	    case OBJ_SHOT:
	    case OBJ_SMART_SHOT:
	    case OBJ_HEAT_SHOT:
		sprintf(msg, "%s was shot down by %s.", pl->name,
			Players[killer=obj->id == -1
				? i :GetInd[obj->id]]->name);
		SET_BIT(pl->status, KILLED);

		if (killer == i) {
		    strcat(msg, " How strange!");
		    SCORE(i, PTS_PR_PL_SHOT);
		} else {
		    sc = Rate(Players[killer]->score, pl->score);
		    SCORE(killer, sc);
		    SCORE(i, -sc);
		}
		Set_message(msg);

		if (pl->robot_mode != RM_NOT_ROBOT
		    && killer != i
		    && rand()%100 < Players[killer]->score-pl->score) {
			/* Give fuel for offensive */
		    pl->fuel.sum = MAX_PLAYER_FUEL;

		    if (pl->robot_lock != LOCK_PLAYER
			|| pl->robot_lock_id != Players[killer]->id) {
			sprintf(msg, "%s declares war on %s.",
				pl->name, Players[killer]->name);
			pl->robot_lock_id = Players[killer]->id;
			pl->robot_lock = LOCK_PLAYER;
			Set_message(msg);
		    }
		}
		break;
	    case OBJ_CANNON_SHOT:
		SET_BIT(pl->status, KILLED);

		sprintf(msg, "%s was hit by cannonfire.", pl->name);
		Set_message(msg);
		SCORE(i, -Rate(CANNON_RATING, pl->score)/4);
		break;
	    default:
		break;
	    }
	}
	if (BIT(pl->status, KILLED))
	    break;
    }
}


static void WallCollide(object *obj, int x, int y,
			int count, int max, int axis)
{
    player *pl;
    int t, killer;

    if (x < 0 || x >= World.x || y < 0 || y >= World.y)
	obj->life = 0;
    else {
	int type = World.block[x][y];
	
	/* Simple and common, so compute first... */
	if (type == SPACE || type == BASE)
	    ;
	/* Simple and almost as common, so compute second... */
	else if (type == TREASURE) {
	    obj->life = 0;
	    if (obj->type == OBJ_BALL) {
		int t;
		for (t = 0; t < World.NumTreasures; t++) {
		    if (World.treasures[t].pos.x == x
			&& World.treasures[t].pos.y == y) {
			if (t == obj->treasure)
			    obj->life = LONG_MAX;
			else if (obj->owner != -1 
				 && World.treasures[t].team == 
				 Players[GetInd[obj->owner]]->team) {
			    /* Treasure have been stolen and brought back */
			    /* too home treasure. The team should be */
			    /* punished */
			    Punish_team(GetInd[obj->owner], obj->treasure);
			    World.treasures[t].count = 10;
			    obj->count = 0;   			
			}
		    }	
		}
	    }
	} else if (type == FUEL || type == FILLED || type == FILLED_NO_DRAW)
	    obj->life = 0;
	else {
	    /* These cases are more complicated, because they don't
	       entirely fill the square they're in.   The algorithm
	       that got us here basically took the line segment that
	       connects the object's previous position with its
	       current one and then calls WallCollide for every square
	       that that segment passes through.   WallCollide must
	       now examine the subsegment of that segment which is
	       contained within the square, and see if that segment
	       intersects with the shape that is in this square.
	       
	       While that's not horribly complicated to do,
	       it is a bit complicated, and quite slow, so what
	       we do instead is to compute the two endpoints of the
	       subsegment, and see if they are contained within the
	       shape in this square.   Since all of the possible
	       shapes lie to one side or the other of the square,
	       this is fairly safe; the only time it will fail
	       is if an object passes entirely through the square
	       nearly parallel to the long side of a cannon; in that
	       case, neither endpoint would be within the cannon.
	       However, this isn't a very likely case, so it seems
	       safe to discount it. */
	    
	    int ex, ey;                 /* Enter coordinates... */
	    int lx, ly;                 /* Leave coordinates... */
	    
	    /* We can skip the complicated calculations if the
	       motion was entirely within this square. */
	    if (!count && !max) {
		ex = obj->prevpos.x;
		ey = obj->prevpos.y;
		lx = obj->pos.x;
		ly = obj->pos.y;
	    }
	    /* Otherwise, we have to do some thinking... */
	    else {
		float ddx, ddy;
		
		/* Compute the total distance travelled... */
		ddx = obj->pos.x - obj->prevpos.x;
		ddy = obj->pos.y - obj->prevpos.y;
		
		/* Did we start in this square? */
		if (!count) {
		    ex = obj->prevpos.x;
		    ey = obj->prevpos.y;
		}
		/* Nope (groan) */
		else {
		    if (max > 0) {	/* Increasing on major axis */
			if (axis) {	/* Major axis is X axis */
			    ex = x * BLOCK_SZ;
			    ey = obj->prevpos.y
				+ ddy * (((float)ex - obj->prevpos.x) / ddx);
			}
			else {		/* Major axis is Y axis */
			    ey = y * BLOCK_SZ;
			    ex = obj->prevpos.x
				+ ddx * ((ey - obj->prevpos.y) / ddy);
			}
		    }
		    else {		/* Decreasing on major axis */
			if (axis) {	/* Major axis is X axis */
			    ex = x * BLOCK_SZ + BLOCK_SZ - 1;
			    ey = obj->prevpos.y
				+ ddy * ((ex - obj->prevpos.x) / ddx);
			}
			else {  	/* Major axis is Y axis */
			    ey = y * BLOCK_SZ + BLOCK_SZ - 1;
			    ex = obj->prevpos.x
				+ ddx * ((ey - obj->prevpos.y) / ddy);
			}
		    }
		}
		
		/* Did we end in this square? */
		if (count == max) {
		    lx = obj->pos.x;
		    ly = obj->pos.y;
		}
		/* Nope (groan) */
		else {
		    if (max > 0) {		/* Increasing on major axis */
			if (axis) {		/* Major axis is X axis */
			    lx = x * BLOCK_SZ + BLOCK_SZ - 1;
			    ly = obj->prevpos.y
				+ ddy * ((lx - obj->prevpos.x) / ddx);
			}
			else {    		/* Major axis is Y axis */
			    ly = y * BLOCK_SZ + BLOCK_SZ - 1;
			    lx = obj->prevpos.x
				+ ddx * ((ly - obj->prevpos.y) / ddy);
			}
		    }
		    else  {         		/* Decreasing on major axis */
			if (axis) {		/* Major axis is X axis */
			    lx = x * BLOCK_SZ;
			    ly = obj->prevpos.y
				+ ddy * ((lx - obj->prevpos.x) / ddx);
			}
			else {     		/* Major axis is Y axis */
			    ly = y * BLOCK_SZ;
			    lx = obj->prevpos.x
				+ ddx * ((ly - obj->prevpos.y) / ddy);
			}
		    }
		}
		
	    }

#ifdef DEBUG
	    /*
	     * If debug, draw something to clarify things
	     */
	    player *dr = Players[0];	/* This better not be a robot */
#define X(co)  ((int)((co) - dr->world.x))
#define Y(co)  ((int)(FULL - ((co) - dr->world.y)))
	    XSetForeground(dr->disp, dr->gc, dr->colors[WHITE].pixel);
	    XDrawLine(dr->disp, dr->p_draw, dr->gc,
		      X(ex), Y(ey), X(lx), Y(ly));
	    XFlush(dr->disp);
#undef X
#undef Y
	    XSync(dr->disp, False);
#endif	/* DEBUG */

	    /*
	     * By default, we have to keep recomputing for oddly-
	     * shaped objects.
	     */
	    if (count == max)
		obj->placed = 1;
	    switch (type) {
	    case REC_LU:
		if ((ex % BLOCK_SZ) < (ey % BLOCK_SZ)
		    || (lx % BLOCK_SZ) < (ly % BLOCK_SZ)) {
		    obj->life = 0;
		}
		break;
		
	    case REC_RU:
		if ((ex % BLOCK_SZ) >= BLOCK_SZ - (ey % BLOCK_SZ)
		    || (lx % BLOCK_SZ) >= BLOCK_SZ - (ly % BLOCK_SZ)) {
		    obj->life = 0;
		}
		break;
		
	    case REC_LD:
		if ((ex % BLOCK_SZ) <= BLOCK_SZ - (ey % BLOCK_SZ)
		    || (lx % BLOCK_SZ) <= BLOCK_SZ - (ly % BLOCK_SZ)) {
		    obj->life = 0;
		}
		break;
		
	    case REC_RD:
		if ((ex % BLOCK_SZ) > (ey % BLOCK_SZ)
		    || (lx % BLOCK_SZ) > (ly % BLOCK_SZ)) {
		    obj->life = 0;
		}
		break;
		
	    case CANNON:
		if (!BIT(obj->type, KILLING_SHOTS&(~OBJ_CANNON_SHOT)))
		    break;
		
		for(t=0; World.cannon[t].pos.x!=x
		    || World.cannon[t].pos.y!=y; t++);
		
		if (World.cannon[t].dead_time > 0)
		    break;
		
		if ((World.cannon[t].dir == DIR_UP
		     && (ey%BLOCK_SZ <= BLOCK_SZ/3
			 || ly%BLOCK_SZ <= BLOCK_SZ/3))
		    || (World.cannon[t].dir == DIR_DOWN
			&& (ey%BLOCK_SZ >= 2*BLOCK_SZ/3
			    || ly%BLOCK_SZ >= 2*BLOCK_SZ/3))
		    || (World.cannon[t].dir == DIR_RIGHT
			&& (ex%BLOCK_SZ <= BLOCK_SZ/3
			    || lx%BLOCK_SZ <= BLOCK_SZ/3))
		    || (World.cannon[t].dir == DIR_LEFT
			&& (ex%BLOCK_SZ >= 2*BLOCK_SZ/3
			    || lx%BLOCK_SZ >= 2*BLOCK_SZ/3))) {

		    World.cannon[t].dead_time		= CANNON_DEAD_TIME;
		    World.block[World.cannon[t].pos.x]
			       [World.cannon[t].pos.y]	= SPACE;
		    World.cannon[t].active    		= false;
		    Explode_object(x * BLOCK_SZ + BLOCK_SZ/2,
				   y * BLOCK_SZ + BLOCK_SZ/2,
				   World.cannon[t].dir, RES * 0.4, 80);

		    if (obj->id >= 0) {
			killer = GetInd[obj->id];
			pl = Players[killer];
			SCORE(killer, Rate(pl->score, CANNON_RATING)/4);
		    }
		}
		
		break;
		
	    default:
		/*
		 * As a special case, we don't have to recompute
		 * for this one.
		 */
		if (count == max)
		    obj->placed = 0;
		break;
	    }
	}
    }
}



static bool Landing(int ind, int point, int blockdir)
{
    int	x, y, depth;
    int landdir;
    bool diagonal;
    player *pl = Players[ind];


    /* Head first? */
    if (point == 0) {
	sprintf(msg, "%s smashed head first into a wall.", Players[ind]->name);
	Set_message(msg);
	return False;
    }
    
    /* Way too fast? */
    if (LENGTH(pl->vel.y, pl->vel.x) > 12.0) {
	sprintf(msg, "%s smashed into a wall.", Players[ind]->name);
	Set_message(msg);
	return False;
    }
    
    x = pl->pos.x + ships[pl->dir].pts[point].x;
    y = pl->pos.y + ships[pl->dir].pts[point].y;

    landdir = ((pl->dir + RES/16) / (RES/8)) * (RES/8);
    diagonal = (landdir % (RES/4) != 0);
 
    if (diagonal && blockdir != landdir) { /* Wrong angle? */
	sprintf(msg, "%s crashed into a wall.", Players[ind]->name);
	Set_message(msg);
        return False;
    }

    /* Wrong angle? */
    if (!diagonal && blockdir != 0
	&& (landdir+(RES-blockdir)+RES/4) % RES <= RES/2) {

	sprintf(msg, "%s crashed into a wall.", Players[ind]->name);
	Set_message(msg);
        return False;
    }

    depth = (diagonal && blockdir==0 ? 2*BLOCK_SZ : BLOCK_SZ);
    if (landdir < RES/4 || landdir > 3*RES/4)
	depth -= x % BLOCK_SZ;
    if (landdir > RES/4 && landdir < 3*RES/4)
	depth -= BLOCK_SZ - 1 - (x % BLOCK_SZ);
    if (landdir > 0 && landdir < RES/2)
	depth -= y % BLOCK_SZ;
    if (landdir > RES/2 && landdir < RES)
	depth -= BLOCK_SZ - 1 - (y % BLOCK_SZ);
    if (diagonal)
	depth = (depth+1)/2;

    if (depth > 20) {
	sprintf(msg, "%s crashed into a wall.", Players[ind]->name);
	Set_message(msg);
	return False;
    }

    if (depth <= 0)
	return True;
 
    /*
     * Update velocity, position, etc
     */
    if (diagonal) {
	float tmp = pl->vel.x;
	pl->vel.x = pl->vel.y;
	pl->vel.y = tmp;
    }
 
    if (landdir < RES/4 || landdir > 3*RES/4) {
	pl->pos.x = x + depth - ships[pl->dir].pts[point].x;
	pl->vel.x = ABS(pl->vel.x);
    }
    if (landdir > RES/4 && landdir < 3*RES/4) {
	pl->pos.x = x - depth - ships[pl->dir].pts[point].x;
	pl->vel.x = -ABS(pl->vel.x);
    }
    if (landdir > 0 && landdir < RES/2) {
	pl->pos.y = y + depth - ships[pl->dir].pts[point].y;
	pl->vel.y = ABS(pl->vel.y);
    }
    if (landdir > RES/2 && landdir < RES) {
	pl->pos.y = y - depth - ships[pl->dir].pts[point].y;
	pl->vel.y = -ABS(pl->vel.y);
    }
 
    pl->vel.x *= 0.90;
    pl->vel.y *= 0.90;
    pl->dir = pl->float_dir -= ((pl->dir - landdir)*0.2);

    return True;
}



int wormXY(int x, int y)
{
    int i;
    
    for (i = 0; i < World.NumWormholes; i++)
	if (World.wormHoles[i].pos.x == x &&
	    World.wormHoles[i].pos.y == y)
	    break;

    return i;
}
