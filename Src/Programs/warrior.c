/****************************************\
* copyright (c) 1988 by michael j zehr   *
* xtank battle program. first try        *
*                                        *
* algorithm: just moves and shoots a bit *
\****************************************/

/*
$Author: rpotter $
$Id: warrior.c,v 2.3 1991/02/10 13:52:09 rpotter Exp $

$Log: warrior.c,v $
 * Revision 2.3  1991/02/10  13:52:09  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:27  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:36  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:54  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:21  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtanklib.h"
#include <math.h>
#include "sysdep.h"

#define MAX_MAZE 3000

#define NOTHING 0
#define EXPLORING 1
#define NEED_HELP 2
#define COMBAT 3
#define RESTING 4
#define YELLOW 5

#define HALF_HEIGHT 96
#define HALF_WIDTH 96

#define VISITED 1
#define SCANNED 2
#define N_WALL 4
#define E_WALL 8
#define S_WALL 16
#define W_WALL 32
#define NULL_DIRECTION 255
#define STOP_SEARCH 254

#define CLOSE 400

#define AMMO_SPEED 20
#define RANGE 500

typedef struct
{
	int next_blip, next_vehicle, next_loc, next_move, fire;
	int next_bullet, next_mode, next_dest, fire_target;
} Timing;

typedef struct
{
	int x, y;
} Abs_Location;

typedef struct
{
    int num_blips, num_bullets, num_vehicles, target_id;
    int mode, frame, next_frame, crashed;
    WallSide last_dir;
    float last_speed, last_angle, desired_speed;
    Timing timing;
    Location loc, target, destination, old_loc, dest, final_dest;
    Blip_info blip[MAX_BLIPS];
    Vehicle_info vehicle[MAX_VEHICLES];
    Abs_Location future_v[MAX_VEHICLES], me;
    Bullet_info bullet[MAX_BULLETS];
    unsigned char maze[GRID_WIDTH + 1][GRID_HEIGHT + 1];
    unsigned char maze_search[GRID_WIDTH + 1][GRID_HEIGHT + 1];
    WallSide destination_dir[40];
    int destination_length, cur_weapon;
} Everything;

static void main();

Prog_desc warrior_prog = {
	"warrior",
	"zehr1",
	"explores, hunts, and attacks",
	"Michael J Zehr",
	PLAYS_COMBAT | DOES_SHOOT | DOES_EXPLORE,
	5,
	main
};

static void main()
{
    int frame;
    Everything all;

    /* set up tank -- get info and decide what mode to be in */

#ifdef WDEBUG
    printf("warrior_main\n");
#endif

    all.timing.next_blip = all.timing.next_vehicle = all.timing.next_loc = 0;
    all.timing.next_move = all.timing.fire = all.timing.next_bullet = 0;
    all.timing.next_mode = 1;	/* cuz we start exploring and have no data */
    all.destination_length = all.next_frame = all.crashed = 0;
    all.num_blips = all.num_bullets = 0;
    all.num_vehicles = 0;	/* huh? This never gets initialized. -RDP */
    all.mode = EXPLORING;
    all.cur_weapon = 0;
    all.last_dir = NO_DIR;
    all.last_angle = heading();
    get_location(&all.loc);
    all.dest = all.loc;

    {
	int i, j;

	for (i = 0; i < GRID_WIDTH + 1; i++)
	    for (j = 0; j < GRID_HEIGHT + 1; j++)
	    {
		all.maze_search[i][j] = NULL_DIRECTION;
		all.maze[i][j] = 0;
	    }
    }

    while (1)
    {
	frame = frame_number();
	if (all.frame != frame)
	{
	    all.frame = frame;
	}
	else
	    done();		/* we've already been here this frame, so end */

	/* first come the *really* important things, like the mode checker
	   and the thing that handles each frame's move */

	if (frame >= all.timing.next_mode)
	{
	    warrior_mode(&all);
	    all.timing.next_mode = frame + 6;
	    /* should depend on current_mode */
	}
	if (frame >= all.next_frame)
	{
	    /* these are things that get called once per frame */
	    get_location(&all.loc);
	    all.me.x = all.loc.grid_x * BOX_WIDTH + all.loc.box_x;
	    all.me.y = all.loc.grid_y * BOX_HEIGHT + all.loc.box_y;
	    warrior_generic_move(&all);
	    all.next_frame = frame + 1;
	}
	switch (all.mode)
	{
	  case EXPLORING:
	    /* in each case, there should be some processing of data in
	       order to decide what the new timing values should be */
	    if (frame >= all.timing.next_blip)
	    {
		get_blips(&all.num_blips, all.blip);
		if (all.num_blips)
		{
		    /* switch mode and go back to top ... */
		    all.mode = YELLOW;
		}
		all.timing.next_blip = frame + 8;
	    }
	    if (frame >= all.timing.next_vehicle)
	    {
		warrior_get_vehicles(&all);
		all.timing.next_vehicle = frame + 8;
		if (all.num_vehicles)
		    all.mode = COMBAT;
	    }
	    if (frame >= all.timing.next_move)
	    {
		warrior_move(&all);
	    }
	    break;
	  case NEED_HELP:

	  case COMBAT:
	    if (frame >= all.timing.next_blip)
	    {
		get_blips(&all.num_blips, all.blip);
		all.timing.next_blip = frame + 10;
	    }
	    if (frame >= all.timing.next_vehicle)
	    {
		warrior_get_vehicles(&all);
		if (!(all.num_vehicles))
		    all.mode = YELLOW;
		warrior_combat_target(&all);
		all.timing.next_vehicle = frame + 4;
	    }
	    if (frame >= all.timing.fire)
	    {
		warrior_fire(&all);
		all.timing.fire = frame + 2;
	    }
	    if (frame >= all.timing.next_move)
	    {
		warrior_move(&all);
	    }
	    break;
	  case RESTING:

	  case YELLOW:
	    /* basic plan is to move towards the enemy */
	    if (frame >= all.timing.next_blip)
	    {
		get_blips(&all.num_blips, all.blip);
		if (!(all.num_blips))
		{
		    /* switch mode and go back to top ... */
		    all.mode = EXPLORING;
		    continue;
		}
		all.timing.next_blip = frame + 8;
	    }
	    if (frame >= all.timing.next_vehicle)
	    {
		warrior_get_vehicles(&all);
		if (all.num_vehicles)
		{
		    all.mode = COMBAT;
		    continue;
		}
		all.timing.next_vehicle = frame + 4;
	    }
	    if (frame >= all.timing.next_dest)
	    {
		warrior_yellow_target(&all);
		all.timing.next_dest = frame + 10;
	    }
	    if (frame >= all.timing.next_move)
	    {
		warrior_move(&all);
	    }
	    break;
	}
    }
}


WallSide warrior_explore_dir(all)
Everything *all;
{
    WallSide dir;
    int total, chances[4], x, y, choice, i;

	/* tries to make an intelligent choice of next box to go to while in
	   explore mode.  takes into account whether the box has been visited
	   before, how close it is to the edge of the maze, and other factors */

	if (all->destination_length)
        return all->destination_dir[--(all->destination_length)];

	x = all->loc.grid_x;
	y = all->loc.grid_y;

	chances[0] = chances[1] = chances[2] = chances[3] = 0;
	if (!(all->maze[x][y] & W_WALL))
	{
        chances[(int)WEST] = 1;
		if (!(all->maze[x - 1][y] & VISITED))
            chances[(int)WEST] += 100;
		if (all->last_dir != EAST)
            chances[(int)WEST] += 10;
	}
	if (!(all->maze[x][y] & E_WALL))
	{
        chances[(int)EAST] = 1;
		if (!(all->maze[x + 1][y] & VISITED))
            chances[(int)EAST] += 100;
		if (all->last_dir != WEST)
            chances[(int)EAST] += 10;
	}
	if (!(all->maze[x][y] & N_WALL))
	{
        chances[(int)NORTH] = 1;
		if (!(all->maze[x][y - 1] & VISITED))
            chances[(int)NORTH] += 100;
		if (all->last_dir != SOUTH)
            chances[(int)NORTH] += 10;
	}
	if (!(all->maze[x][y] & S_WALL))
	{
        chances[(int)SOUTH] = 1;
		if (!(all->maze[x][y + 1] & VISITED))
            chances[(int)SOUTH] += 100;
		if (all->last_dir != NORTH)
            chances[(int)SOUTH] += 10;
	}
	for (i = 0; i < all->num_blips; i++)
	{
		if (all->blip[i].x > x)
		{
            if (chances[(int)EAST])
                chances[(int)EAST] += 200;
		}
        else if (chances[(int)WEST])
            chances[(int)WEST] += 200;
		if (all->blip[i].y > y)
		{
            if (chances[(int)SOUTH])
                chances[(int)SOUTH] += 200;
		}
        else if (chances[(int)NORTH])
            chances[(int)NORTH] += 200;
	}

	/* pick a random direction based on weighting */
    total = chances[(int)NORTH] + chances[(int)SOUTH] +
        chances[(int)WEST] + chances[(int)EAST];
    dir = NORTH;
	choice = rand() % total;
    while (choice >= chances[(int)dir])
        choice -= chances[(int)(dir++)];
	return (dir);
}


WallSide warrior_yellow_dir(all)
Everything *all;
{
    if (all->destination_length == 0)
    {
        return (warrior_explore_dir(all));
    }
    else
        return (all->destination_dir[--(all->destination_length)]);
}


WallSide warrior_combat_dir(all)
Everything *all;
{
    WallSide dir;
    int total, chances[4], x, y, choice, i, x1, y1, dx, dy;
    int dist, inc;

    /* tries to make an intelligent choice of next box to go to based on
       where our "target" is and where the other vehicles are, trying to
       maintain a constant distance from the enemy but not get trapped */

    x = all->loc.grid_x;
    y = all->loc.grid_y;

    chances[(int)WEST] = chances[(int)EAST] = chances[(int)NORTH] =
	chances[(int)SOUTH] = 0;
    if (!(all->maze[x][y] & W_WALL))
        chances[(int)WEST] = 1;
    if (!(all->maze[x][y] & E_WALL))
        chances[(int)EAST] = 1;
    if (!(all->maze[x][y] & N_WALL))
        chances[(int)NORTH] = 1;
    if (!(all->maze[x][y] & S_WALL))
        chances[(int)SOUTH] = 1;

    x1 = all->me.x;
    y1 = all->me.y;

    /* look at each of the visible or on-radar vehicles, and adjust the
       chance of going in a certain direction based on where that vehicle is
       and how fast it's moving, etc. */

    /* what it does now:  it tries to maintain a distance of one box from the
       target vehicle.  it attempts to move away from non-target vehicles */

    for (i = 0; i < all->num_vehicles; i++)
    {
	dx = all->future_v[i].x - x1;
	dy = all->future_v[i].y - y1;
	if (all->vehicle[i].id == all->target_id)
	{
	    if ((dx > BOX_WIDTH) || ((dx < 0) && (dx > -BOX_WIDTH)))
	    {
                if (chances[(int)EAST])
                    chances[(int)EAST] += 50;
	    }
            else if (chances[(int)WEST])
                chances[(int)WEST] += 50;

	    if ((dy > BOX_HEIGHT) || ((dy < 0) && (dy > -BOX_HEIGHT)))
	    {
                if (chances[(int)SOUTH])
                    chances[(int)SOUTH] += 50;
	    }
            else if (chances[(int)NORTH])
                chances[(int)NORTH] += 50;

	    dist = (int) sqrt((double) (dx * dx + dy * dy));
	    if (dist > BOX_WIDTH)
		all->desired_speed = 5.0;
	    else if (dist > BOX_WIDTH / 2)
		all->desired_speed = 6.0;
	    else if (dist > BOX_WIDTH / 4)
		all->desired_speed = 7.3;
	    else
		all->desired_speed = 9.0;
	}
	else
	{
	    dist = dx * dx + dy * dy;
	    if (dist < 1000)
		inc = 20;
	    else if (dist < 10000)
		inc = 10;
	    else
		inc = 4;
	    if (x > x1)
	    {
                if (chances[(int)WEST])
                    chances[(int)WEST] += inc;
	    }
            else if (chances[(int)EAST])
                chances[(int)EAST] += inc;
	    if (y > y1)
	    {
                if (chances[(int)NORTH])
                    chances[(int)SOUTH] += inc;
	    }
            else if (chances[(int)SOUTH])
                chances[(int)SOUTH] += inc;
	}
    }

    for (i = 0; i < all->num_blips; i++)
    {
	if (all->blip[i].x > x1)
	{
            if (chances[(int)WEST])
                chances[(int)WEST]++;
	}
        else if (chances[(int)EAST])
            chances[(int)EAST]++;
	if (all->blip[i].y > y1)
	{
            if (chances[(int)NORTH])
                chances[(int)SOUTH]++;
	}
        else if (chances[(int)SOUTH])
            chances[(int)SOUTH]++;
    }

    /* pick a random direction based on weighting */
    total = chances[(int)NORTH] + chances[(int)SOUTH] + chances[(int)WEST] +
	chances[(int)EAST];
    dir = NORTH;
    if (total)
	choice = rand() % total;
    else
	choice = 0;

    while (choice >= chances[(int)dir] && (int)dir <= (int)WEST)
        choice -= chances[(int)(dir++)];
    return (dir);
}


warrior_move(all)
Everything *all;
{
    WallSide dir;

    /* if we're entering a new square, scan the area */
    if (!(all->maze[all->loc.grid_x][all->loc.grid_y] & VISITED))
        warrior_new_square(all);

    switch (all->mode)
    {
        case EXPLORING:
            dir = warrior_explore_dir(all);
            break;
        case NEED_HELP:
        case COMBAT:
            dir = warrior_combat_dir(all);
            break;
        case RESTING:
        case YELLOW:
            dir = warrior_yellow_dir(all);
            break;
    }

    warrior_set_dest(all, dir);

    /* don't come back until generic_move calls, except in combat */
    if (all->mode == COMBAT)
        all->timing.next_move = all->frame + 10;
    else
        all->timing.next_move = all->frame + 25;
}


warrior_yellow_target(all)
Everything *all;
{
	int max_dist, target, i, dist;

	/* find a "close" vehicle */
	max_dist = 100;
	target = 0;
	for (i = 0; i < all->num_blips; i++)
	{
		if (all->blip[i].x > all->loc.grid_x)
			dist = all->blip[i].x - all->loc.grid_x;
		else
			dist = all->loc.grid_x - all->blip[i].x;
		if (all->blip[i].y > all->loc.grid_y)
			dist += all->blip[i].y - all->loc.grid_y;
		else
			dist += all->loc.grid_y = all->blip[i].y;
		if (dist < max_dist)
		{
			max_dist = dist;
			target = i;
		}
	}

	/* determine a path to it */
	warrior_plot_course(all, all->loc.grid_x, all->loc.grid_y,
						all->blip[target].x, all->blip[target].y);
}


warrior_set_dest(all, dir)
Everything *all;
WallSide dir;
{
	all->dest = all->loc;
	all->last_dir = dir;
	switch (dir)
	{
		case NORTH:
			--(all->dest.grid_y);
			all->dest.box_x = HALF_WIDTH;
			all->dest.box_y = BOX_HEIGHT - 25;
			break;
		case EAST:
			++(all->dest.grid_x);
			all->dest.box_x = 25;
			all->dest.box_y = HALF_HEIGHT;
			break;
		case SOUTH:
			++(all->dest.grid_y);
			all->dest.box_x = HALF_WIDTH;
			all->dest.box_y = 25;
			break;
		case WEST:
			--(all->dest.grid_x);
			all->dest.box_x = BOX_WIDTH - 25;
			all->dest.box_y = HALF_HEIGHT;
	}
}


warrior_new_square(all)
Everything *all;
{
	int i, j, x, y;

	x = all->loc.grid_x;
	y = all->loc.grid_y;
	all->maze[x][y] |= VISITED;
	for (i = x - 1; i <= x + 1; i++)
		for (j = y - 1; j <= y + 1; j++)
		{
			if (all->maze[i][j] & SCANNED)
				continue;
			if (wall(EAST, i, j))
			{
				all->maze[i + 1][j] |= W_WALL;
				all->maze[i][j] |= E_WALL;
			}
			if (wall(SOUTH, i, j))
			{
				all->maze[i][j + 1] |= N_WALL;
				all->maze[i][j] |= S_WALL;
			}
			all->maze[i][j] |= SCANNED;
		}
}


warrior_generic_move(all)
Everything *all;
{
    /* this is what get's called every frame (I hope) it checks for
       collisions, and other "special cases" otherwise, it heads for the
       current all->dest the type of motion (straight, etc) depends on the
       current mode it assumes that there is a straight path to all->dest.
       all->dest will be kept up to date by a movement routine which is
       called periodically, and that will head for all->final_dest. this
       routine will schedule the next call to the short movement routine once
       again, mileage may vary ... */

    int dx, dy;
    float angle;

    if (all->crashed)
    {
	warrior_handle_crash(all);
	return;
    }
    /* if we're at destination, then we want to pick a new destination */
    if ((all->dest.grid_x == all->loc.grid_x) &&
	(all->dest.grid_y == all->loc.grid_y))
	warrior_move(all);

    dx = all->dest.grid_x * BOX_WIDTH + all->dest.box_x - all->me.x;
    dy = all->dest.grid_y * BOX_HEIGHT + all->dest.box_y - all->me.y;

    if (dx == 0)
	if (dy < 0)
	    angle = 1.5 * PI;
	else
	    angle = 0.5 * PI;
    else
        angle = (float) ATAN2(dy, dx);

    switch (all->mode)
    {
      case EXPLORING:
	warrior_try_to_go(all, angle, 8.0);
	break;
      case NEED_HELP:
      case COMBAT:
	warrior_try_to_go(all, angle + (rand() % 40) / 100 - 0.20,
			  all->desired_speed);
	break;
      case RESTING:
      case YELLOW:
	warrior_try_to_go(all, angle, 6.0);
	break;
    }
}


warrior_try_to_go(all, angl, spd)
Everything *all;
float angl, spd;
{
	/* theta is amount we need to turn */
	float theta;

	theta = angl - all->last_angle;
	if (theta < 0)
		theta = -theta;

	while (theta >= 2 * PI)
		theta -= 2 * PI;
	if (theta >= PI)
		theta = 2 * PI - theta;

	if (theta > 0.25 * PI)
		spd = 3.0;
	if (spd != all->last_speed)
		set_rel_drive(spd);

	if (theta > 0.1 * PI)
	{
		turn_vehicle(angl);
		all->last_angle = angl;
	}
	all->last_speed = spd;
}


warrior_mode(all)
Everything *all;
{
	/* this should be one of the "smarter" parts of the program, but right
	   now it's pretty dumb */
	if ((all->num_blips == 0) && (all->num_vehicles == 0))
		all->mode = EXPLORING;
}


warrior_handle_crash(all)
Everything *all;
{
	/* movement scheduler during a crash */
	switch (all->crashed)
	{
		case 1:
			all->dest = all->loc;
			break;
		case 2:
			turn_vehicle(heading() + PI);
			set_rel_drive(2.0);
			break;
		case 3:
			break;
		case 4:
			set_rel_drive(-3.0);
			break;
	}
	--(all->crashed);
}


warrior_plot_course(all, x, y, x1, y1)
Everything *all;
int x, y, x1, y1;
{
    /* plots a course from x,y to x1,y1, trying to find close to the shortest
       path, while staying within scanned territory */

    int dx, dy, absdx, absdy, max_distance, min_distance, i, j;
    int xloc, yloc, count, start, end, new_end, shortest_point;
    int data_x[250], data_y[250];
    int dist, shortest_dist, dir;

    dx = x - x1;
    dy = y - y1;
    if (dx < 0)
	absdx = -dx;
    else
	absdx = dx;
    if (dy < 0)
	absdy = -dy;
    else
	absdy = dy;

    /* decide what the max distance we'll search is ... */
    min_distance = absdx + absdy;
    if (min_distance < 4)
	max_distance = min_distance << 1;
    else
	max_distance = 8;

    shortest_dist = 100;
    shortest_point = -1;

    start = 0;
    new_end = end = 1;
    data_x[0] = x;
    data_y[0] = y;
    all->maze_search[x][y] = (unsigned char) STOP_SEARCH;

    for (i = 0; i < max_distance; i++)
    {
	for (j = start; j < end; j++)
	{
	    xloc = data_x[j];
	    yloc = data_y[j];

	    if ((xloc = x1) && (yloc = y1))
	    {
		shortest_point = j;
		break;
	    }
	    if ((all->maze[xloc][yloc - 1] & SCANNED) &&
		(!(all->maze[xloc][yloc - 1] & S_WALL)) &&
		(all->maze_search[xloc][yloc - 1] == NULL_DIRECTION))
	    {
		/* add x,y-1 to the list */
		data_x[new_end] = xloc;
		data_y[new_end] = yloc - 1;
		new_end++;
		all->maze_search[xloc][yloc - 1] = (unsigned short) SOUTH;
	    }
	    if ((all->maze[xloc][yloc + 1] & SCANNED) &&
		(!(all->maze[xloc][yloc + 1] & N_WALL)) &&
		(all->maze_search[xloc][yloc + 1] == NULL_DIRECTION))
	    {
		/* add x,y+1 to the list */
		data_x[new_end] = xloc;
		data_y[new_end] = yloc + 1;
		new_end++;
		all->maze_search[xloc][yloc + 1] = (unsigned short) NORTH;
	    }
	    if ((all->maze[xloc - 1][yloc] & SCANNED) &&
		(!(all->maze[xloc - 1][yloc] & E_WALL)) &&
		(all->maze_search[xloc - 1][yloc] == NULL_DIRECTION))
	    {
		/* add x-1,y to the list */
		data_x[new_end] = xloc - 1;
		data_y[new_end] = yloc;
		new_end++;
		all->maze_search[xloc - 1][yloc] = (unsigned short) EAST;
	    }
	    if
		((all->maze[xloc + 1][yloc] & SCANNED) &&
		 (!(all->maze[xloc + 1][yloc] & W_WALL)) &&
		 (all->maze_search[xloc + 1][yloc] == NULL_DIRECTION))
		{
		    /* add x+1,y to the list */
		    data_x[new_end] = xloc + 1;
		    data_y[new_end] = yloc;
		    new_end++;
		    all->maze_search[xloc + 1][yloc] = (unsigned short) WEST;
		}
	}
	if (shortest_point != -1)
	    break;
	start = end;
	end = new_end;
    }

    if (shortest_point == -1)
	for (i = 0; i < new_end; i++)
	{
	    if (x1 < data_x[i])
		dist = data_x[i] - x1;
	    else
		dist = x1 - data_x[i];

	    if (y1 < data_y[i])
		dist += data_y[i] - y1;
	    else
		dist += y1 - data_y[i];

	    if (dist < shortest_dist)
	    {
		shortest_dist = dist;
		shortest_point = i;
	    }
	}

    count = 0;
    xloc = data_x[shortest_point];
    yloc = data_y[shortest_point];
    while (count < 40)
    {
	dir = all->maze_search[xloc][yloc];

	switch (dir)
	{
	  case NORTH:
	    all->destination_dir[count++] = SOUTH;
	    --yloc;
	    break;
	  case SOUTH:
	    all->destination_dir[count++] = NORTH;
	    ++yloc;
	    break;
	  case EAST:
	    all->destination_dir[count++] = WEST;
	    ++xloc;
	    break;
	  case WEST:
	    all->destination_dir[count++] = EAST;
	    --xloc;
	    break;
	  default:
	    goto while_end;
	}
    }
  while_end:;

#ifdef WDEBUG
    printf("destination_length is %d, setting to %d\n",
	   all->destination_length, count);
#endif

    all->destination_length = count;

    /* erase maze_search */
    for (i = 0; i < new_end; i++)
	all->maze_search[data_x[i]][data_y[i]] = (unsigned char) NULL_DIRECTION;
}


warrior_combat_target(all)
Everything *all;
{
	int min_dist, dist, i, dx, dy;

	min_dist = 1000000;
	for (i = 0; i < all->num_vehicles; i++)
	{
		dx = all->future_v[i].x - all->me.x;
		dy = all->future_v[i].y - all->me.y;
		dist = dx * dx + dy * dy;
		if (dist < min_dist)
		{
			min_dist = dist;
			all->target_id = all->vehicle[i].id;
		}
	}
}


warrior_get_vehicles(all)
Everything *all;
{
	int i;

	get_vehicles(&all->num_vehicles, all->vehicle);
	for (i = 0; i < all->num_vehicles; i++)
	{
		all->future_v[i].x = all->vehicle[i].loc.grid_x * BOX_WIDTH +
			all->vehicle[i].loc.box_x;
		all->future_v[i].y = all->vehicle[i].loc.grid_y * BOX_HEIGHT +
			all->vehicle[i].loc.box_y;
	}
}


warrior_fire(all)
Everything *all;
{
	int target, i;
	int dx, dy, temp, vtx, vty, vt, a, b, c, t;
	Location tloc;

	target = 0;
	for (i = 0; i < all->num_vehicles; i++)
		if (all->vehicle[i].id == all->target_id)
			target = i;

	vtx = all->vehicle[target].xspeed;
	vty = all->vehicle[target].yspeed;
	vt = (int) sqrt((double) (vtx * vtx + vty * vty));
	a = vt * vt - AMMO_SPEED * AMMO_SPEED;
	dx = all->future_v[target].x - all->me.x;
	dy = all->future_v[target].y - all->me.y;
	b = (dx * vtx + dy * vty) << 1;
	c = dx * dx + dy * dy;
	temp = b * b - (a * c << 2);

	if ((temp < 0) || (a == 0))
		return;

	temp = (int) (sqrt((double) temp));
	t = ((-b - temp) / a) >> 1;
	dx += t * vtx;
	dy += t * vty;

	if (all->cur_weapon > 2)
		all->cur_weapon = 0;
	else
		++(all->cur_weapon);

	tloc = all->loc;
	tloc.box_x += dx;
	tloc.box_y += dy;

	if (tloc.box_x < 0)
		do
		{
			--tloc.grid_x;
			tloc.box_x += BOX_WIDTH;
		} while (tloc.box_x < 0);
	else
	while (tloc.box_x >= BOX_WIDTH)
	{
		++tloc.grid_x;
		tloc.box_x -= BOX_WIDTH;
	}

	if (tloc.box_y < 0)
		do
		{
			--tloc.grid_y;
			tloc.box_y += BOX_HEIGHT;
		} while (tloc.box_y < 0);
	else
	while (tloc.box_y >= BOX_HEIGHT)
	{
		++tloc.grid_y;
		tloc.box_y -= BOX_HEIGHT;
	}


	if (warrior_clear_path(all, &all->loc, &tloc))
	{
		aim_all_turrets(dx, dy);
		if (dx * dx + dy * dy < RANGE * RANGE)
			fire_all_weapons();
	}
}


/*
** Returns 1 if there are no walls blocking the path from start to finish,
** otherwise returns 0.  Keep in mind that the path has 0 width, so
** a bullet would make it through, but a vehicle might not.
**
** This code is optimized, but I'm sure someone could improve on it.
*/
warrior_clear_path(all, start, finish)
Everything *all;
Location *start, *finish;
{
	int start_x, start_y, finish_x, finish_y;
	int dx, dy, lattice_dx, lattice_dy;
	int tgrid_x, tgrid_y, fgrid_x, fgrid_y;

	/* Compute absolute x coordinate in maze */
	start_x = start->grid_x * BOX_WIDTH + start->box_x;
	start_y = start->grid_y * BOX_HEIGHT + start->box_y;
	finish_x = finish->grid_x * BOX_WIDTH + finish->box_x;
	finish_y = finish->grid_y * BOX_HEIGHT + finish->box_y;

	/* Computed x and y differences from start to finish */
	dx = finish_x - start_x;
	dy = finish_y - start_y;

	/* Set up temporary and final box coordinates */
	tgrid_x = start->grid_x;
	tgrid_y = start->grid_y;
	fgrid_x = finish->grid_x;
	fgrid_y = finish->grid_y;

	/* Figure out the general direction that the line is travelling in * so
	   that we can write specific code for each case. *
	
	In the NE, SE, NW, and SW cases, * lattice_dx and lattice_dy are the
	   deltas from the starting * location to the lattice point that the path
	   is heading towards. * The slope of the line is compared to the slope
	   to the lattice point * This determines which wall the path intersects. *
	   Instead of comparing dx/dy with lattice_dx/lattice_dy, I multiply *
	   both sides by dy * lattice_dy, which lets me do 2 multiplies instead *
	   of 2 divides. */
	if (fgrid_x > tgrid_x)
		if (fgrid_y > tgrid_y)
		{						/* Southeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx < dy * lattice_dx)
				{
					if (all->maze[tgrid_x][tgrid_y] & S_WALL)
						return (0);
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				}
				else
				{
					if (all->maze[tgrid_x][tgrid_y] & E_WALL)
						return (0);
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		}
		else if (fgrid_y < tgrid_y)
		{						/* Northeast */
			lattice_dx = (tgrid_x + 1) * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx > dy * lattice_dx)
				{
					if (all->maze[tgrid_x][tgrid_y] & N_WALL)
						return (0);
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				}
				else
				{
					if (all->maze[tgrid_x][tgrid_y] & E_WALL)
						return (0);
					tgrid_x++;
					lattice_dx += BOX_WIDTH;
				}
			}
		}
		else
		{						/* East */
			for (; tgrid_x < fgrid_x; tgrid_x++)
				if (all->maze[tgrid_x][tgrid_y] & E_WALL)
					return (0);
		}

	else if (fgrid_x < tgrid_x)
		if (fgrid_y > tgrid_y)
		{						/* Southwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = (tgrid_y + 1) * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx >= dy * lattice_dx)
				{
					if (all->maze[tgrid_x][tgrid_y] & S_WALL)
						return (0);
					tgrid_y++;
					lattice_dy += BOX_HEIGHT;
				}
				else
				{
					if (all->maze[tgrid_x][tgrid_y] & W_WALL)
						return (0);
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		}
		else if (fgrid_y < tgrid_y)
		{						/* Northwest */
			lattice_dx = tgrid_x * BOX_WIDTH - start_x;
			lattice_dy = tgrid_y * BOX_HEIGHT - start_y;
			while (tgrid_x != fgrid_x || tgrid_y != fgrid_y)
			{
				if (lattice_dy * dx < dy * lattice_dx)
				{
					if (all->maze[tgrid_x][tgrid_y] & N_WALL)
						return (0);
					tgrid_y--;
					lattice_dy -= BOX_HEIGHT;
				}
				else
				{
					if (all->maze[tgrid_x][tgrid_y] & W_WALL)
						return (0);
					tgrid_x--;
					lattice_dx -= BOX_WIDTH;
				}
			}
		}
		else
		{						/* West */
			for (; tgrid_x > fgrid_x; tgrid_x--)
				if (all->maze[tgrid_x][tgrid_y] & W_WALL)
					return (0);
		}

	else if (fgrid_y > tgrid_y)
	{							/* South */
		for (; tgrid_y < fgrid_y; tgrid_y++)
			if (all->maze[tgrid_x][tgrid_y] & S_WALL)
				return (0);
	}
	else if (fgrid_y < tgrid_y)
	{							/* North */
		for (; tgrid_y > fgrid_y; tgrid_y--)
			if (all->maze[tgrid_x][tgrid_y] & N_WALL)
				return (0);
	}
	return (1);
}
