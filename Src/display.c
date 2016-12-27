/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "xtank.h"
#include "bullet.h"
#include "vehicle.h"
#include "object.h"
#include "proto.h"
#include "vehicle.h"

#ifdef BATCH_LINES
#define USE_BATCHED_LINES
#endif

#ifdef BATCH_POINTS
#define USE_BATCHED_POINTS
#endif

#include "malloc.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "vstructs.h"
#include "message.h"
#include "bullet.h"
#include "terminal.h"
#include "globals.h"
#include "proto.h"


#if !defined(usleep)
#include <sys/time.h>
#define usleep(x) { \
    struct timeval st_delay; \
    st_delay.tv_usec = (x); \
    st_delay.tv_sec = 0; \
    select(0, NULL, NULL, NULL, &st_delay); \
}
#endif	/* defined(usleep) */


extern Terminal *term;
extern Weapon_stat weapon_stat[];
extern int team_color[];
extern int team_color_bright[];
extern Settings settings;
extern Map real_map;
extern int frame;


/*
** Displays everything on the current terminal.
*/
void
display_terminal(unsigned int status, int lastterm)
{
	/* Display all of the windows */
	display_anim(status, lastterm);
	display_cons(status);
	display_map(status);
	display_game(status);
	display_help(status);
	display_msg(status);
	display_status(status);
}

/*
** Displays the vehicles, walls, landmarks, bullets, and explosions in the
** animation window.
*/
void
display_anim(unsigned int status, int lastterm)
{
	Vehicle *v;
	int i;

#ifndef NO_HUD
	unsigned int action;
#endif /* NO_HUD */

	/* Check for being exposed */
	check_expose(ANIM_WIN, status);

	/* If we are turning this window on, clear it first */
	if (status == ON) {
		clear_window(ANIM_WIN);

		/* If paused, display the pause message */
		if (settings.game_speed == 0)
			display_pause_message();
	}
	/* If 3d mode is on, display in 3d */
	if (term->status & TS_3d) {
		display_anim_3d(status);
		return;
	}
	/* Display the walls and landmarks in the maze */
	display_maze(status);

	/* Display the vehicles in a manner dependent on their status */
	for (i = 0; i < num_veh_alive; i++) {
		v = live_vehicles[i];
		switch (status) {
		  case REDISPLAY:
			  if (tstflag(v->status, VS_is_alive) &&
				  tstflag(v->status, VS_was_alive)) {
				  display_vehicle(v, REDISPLAY);
			  } else if (tstflag(v->status, VS_is_alive)) {
				  display_vehicle(v, ON);
			  } else if (tstflag(v->status, VS_was_alive)) {
				  display_vehicle(v, OFF);
			  }
			  break;
		  case ON:
			  if (tstflag(v->status, VS_is_alive))
				  display_vehicle(v, ON);
			  break;
		  case OFF:
			  if (tstflag(v->status, VS_was_alive))
				  display_vehicle(v, OFF);
		}
	}

	display_bullets(status, lastterm);
	display_explosions(status);

#ifndef NO_HUD
	/*
     *  The test for NULL is a piece of SPAM.
     *
     *  Basically the reason this is needed is cause
     *  this routine is getting called with
     *  the current terminal set to a non-existant
     *  vehicle.  It's some bogosity for the
     *  death-delay so that the dead vehicle keeps getting
     *  updates
     *
     *  -ane
     */

	v = term->vehicle;

	if (v != NULL) {

		switch (status) {
		  case REDISPLAY:
			  if (tstflag(v->status, VS_is_alive))
				  do_special(v, HUD, SP_redisplay);
			  break;
		  case ON:
			  if (tstflag(v->status, VS_is_alive))
				  do_special(v, HUD, SP_draw);
			  break;
		  case OFF:
			  if (tstflag(v->status, VS_was_alive))
				  do_special(v, HUD, SP_erase);
			  break;
		}
	}
#endif /* !NO_HUD */

}


#define VEHICLE_NAME_Y 25

/*
** Displays the specified vehicle and its turrets in the animation window.
*/
void
display_vehicle(Vehicle *v, unsigned int status)
{
	Loc *loc, *old_loc;
	Picture *pic;

	/* Erase the old vehicle picture */
#ifdef NO_CAMO
	if (status != ON)
#else /* NO_CAMO */
	if (status != ON && !v->old_camod)
#endif /* NO_CAMO */
	{
		old_loc = v->old_loc;
		pic = &v->obj->pic[v->vector.old_rot];
		loc = v->loc;
		if (status != REDISPLAY || un_stingy ||
			(old_loc->screen_x[term->num] != loc->screen_x[term->num]) ||
#ifndef NO_CAMO
			(!v->old_camod && v->camod) ||
#endif /* !NO_CAMO */
			(old_loc->screen_y[term->num] != loc->screen_y[term->num]) ||
			(v->vector.old_rot != v->vector.rot))
			draw_picture(ANIM_WIN, old_loc->screen_x[term->num],
					 old_loc->screen_y[term->num], pic, DRAW_XOR, v->color);

		/* Erase the string showing name and team */
		if (!settings.si.no_nametags)
			if (status != REDISPLAY || un_stingy ||
				(old_loc->screen_x[term->num] != loc->screen_x[term->num]) ||
#ifndef NO_CAMO
				(!v->old_camod && v->camod) ||
#endif /* !NO_CAMO */
				(old_loc->screen_y[term->num] != loc->screen_y[term->num]))
				if (should_disp_name())
					draw_text(ANIM_WIN,
							  old_loc->screen_x[term->num],
							  old_loc->screen_y[term->num] + VEHICLE_NAME_Y,
							  v->disp, S_FONT, DRAW_XOR, v->color);
	}
	/* Draw the new vehicle picture */
#ifdef NO_CAMO
	if (status != OFF)
#else /* NO_CAMO */
	if (status != OFF && !v->camod)
#endif /* NO_CAMO */
	{
		loc = v->loc;
		pic = &v->obj->pic[v->vector.rot];
		old_loc = v->old_loc;
		if (status != REDISPLAY || un_stingy ||
			(old_loc->screen_x[term->num] != loc->screen_x[term->num]) ||
#ifndef NO_CAMO
			(v->old_camod && !v->camod) ||
#endif /* !NO_CAMO */
			(old_loc->screen_y[term->num] != loc->screen_y[term->num]) ||
			(v->vector.old_rot != v->vector.rot))
			draw_picture(ANIM_WIN, loc->screen_x[term->num],
						 loc->screen_y[term->num], pic, DRAW_XOR, v->color);

		/* Display a string showing name and team */
		if (!settings.si.no_nametags)
			if (status != REDISPLAY || un_stingy ||
				(old_loc->screen_x[term->num] != loc->screen_x[term->num]) ||
#ifndef NO_CAMO
				(v->old_camod && !v->camod) ||
#endif /* !NO_CAMO */
				(old_loc->screen_y[term->num] != loc->screen_y[term->num]))
				if (should_disp_name()) {
					draw_text(ANIM_WIN,
							  loc->screen_x[term->num],
							  loc->screen_y[term->num] + VEHICLE_NAME_Y,
							  v->disp, S_FONT, DRAW_XOR, v->color);
				}
	}
	/* Display the vehicle's turrets */
	display_turrets(v, status);
}

/*
** Displays all turrets for the specified vehicle in the animation window.
*/
void
display_turrets(Vehicle *v, unsigned int status)
{
	Picture *pic;
	Loc *loc, *old_loc;
	Object *obj;
	Coord *tcoord, *old_tcoord;
	Turret *t;
	int i;
	int mov_rot;

	loc = v->loc;
	old_loc = v->old_loc;
	mov_rot = (old_loc->screen_x[term->num] != loc->screen_x[term->num] || un_stingy ||
			   (old_loc->screen_y[term->num] != loc->screen_y[term->num]) ||
			   (v->vector.old_rot != v->vector.rot));
	for (i = 0; i < v->num_turrets; i++) {
		t = &v->turret[i];
		obj = t->obj;

		/* erase the old turret */
#ifdef NO_CAMO
		if (status != ON)
#else /* NO_CAMO */
		if ((status != ON) && !v->old_camod)
#endif /* NO_CAMO */
		{
			old_tcoord = &v->obj->picinfo[v->vector.old_rot].turret_coord[i];
			pic = &obj->pic[t->old_rot];
			if ((status != REDISPLAY) || mov_rot ||
#ifndef NO_CAMO
				(!v->old_camod && v->camod) ||
#endif /* !NO_CAMO */
#ifdef NEW_TURRETS
				(t->old_end.x != t->end.x) || (t->old_end.y != t->end.y) ||
#endif /* NEW_TURRETS */
				(t->old_rot != t->rot))
#ifndef TEST_TURRETS
				draw_picture(ANIM_WIN, old_loc->screen_x[term->num] + old_tcoord->x,
							 old_loc->screen_y[term->num] + old_tcoord->y,
							 pic, DRAW_XOR, v->color);
#else /* TEST_TURRETS */
				draw_line(ANIM_WIN, old_loc->screen_x[term->num] + old_tcoord->x,
						  old_loc->screen_y[term->num] + old_tcoord->y,
				old_loc->screen_x[term->num] + old_tcoord->x + t->old_end.x,
				old_loc->screen_y[term->num] + old_tcoord->y + t->old_end.y,
						  DRAW_XOR, v->color);
#endif /* TEST_TURRETS */
		}
		/* draw the new turret */
#ifdef NO_CAMO
		if (status != OFF)
#else /* NO_CAMO */
		if ((status != OFF) && !v->camod)
#endif /* NO_CAMO */
		{
			tcoord = &v->obj->picinfo[v->vector.rot].turret_coord[i];
			pic = &obj->pic[t->rot];
			if ((status != REDISPLAY) || mov_rot ||
#ifndef NO_CAMO
				(v->old_camod && !v->camod) ||
#endif /* !NO_CAMO */
#ifdef NEW_TURRETS
				(t->old_end.x != t->end.x) || (t->old_end.y != t->end.y) ||
#endif /* NEW_TURRETS */
				(t->old_rot != t->rot))
#ifndef TEST_TURRETS
				draw_picture(ANIM_WIN, loc->screen_x[term->num] + tcoord->x,
							 loc->screen_y[term->num] + tcoord->y,
							 pic, DRAW_XOR, v->color);
#else /* TEST_TURRETS */
				draw_line(ANIM_WIN, loc->screen_x[term->num] + tcoord->x,
						  loc->screen_y[term->num] + tcoord->y,
						  loc->screen_x[term->num] + tcoord->x + t->end.x,
						  loc->screen_y[term->num] + tcoord->y + t->end.y,
						  DRAW_XOR, v->color);
#endif /* TEST_TURRETS */
		}
	}
}

/*
** If point_bullets is on, all non-disc bullets are drawn as points.
** Otherwise, the bullet bitmaps are used.
*/
void
display_bullets(unsigned int status, int lastterm)
{
	extern char team_char[];
	extern Bset *bset;
	Bullet *b;
	Picture *pic;
	int i, bullet_color, obullet_color;
	char buf[2], obuf[2];

	/* blank the buffers */
	buf[1] = obuf[1] = '\0';
	for (i = 0; i < bset->number; i++) {

		b = bset->list[i];

		/* the old bullet color */
		if (b->type == DISC && b->thrower != -1) {
			if (b->thrower == -2) {
				obullet_color = WHITE;
				obuf[0] = '\0';
			} else {
				obullet_color = b->thrower;
				obuf[0] = team_char[obullet_color - 1];
			}
			if (lastterm)
				b->thrower = -1;
		} else if (b->owner != NULL) {
			obullet_color = b->owner->color;
			obuf[0] = team_char[b->owner->team];
		} else {
			obullet_color = WHITE;
			obuf[0] = '\0';
		}

		/* the new bullet color */
		if (b->owner != NULL) {
			bullet_color = b->owner->color;
			buf[0] = team_char[b->owner->team];
		} else {
			bullet_color = WHITE;
			buf[0] = '\0';
		}

		/* Erase the old picture of the bullet */
		if (status != ON) {
			pic = &(bullet_obj[(int)b->type])->pic[(int)b->old_rot];
			if (b->life < weapon_stat[(int) b->type].frames - 1 &&
				b->life != -2) {
				if (settings.point_bullets == TRUE
					&& b->type != DISC
					&& !(b->disp_flgs & F_NOPT)) {
					draw_point(ANIM_WIN, b->old_loc->screen_x[term->num],
							   b->old_loc->screen_y[term->num], DRAW_XOR,
							   obullet_color);
				} else {
					if (b->disp_flgs & F_BEAM) {
						draw_line(ANIM_WIN,
							b->old_old_loc.screen_x[term->num],
							b->old_old_loc.screen_y[term->num],
							b->old_loc->screen_x[term->num],
							b->old_loc->screen_y[term->num],
							DRAW_XOR, obullet_color);
					} else {
						draw_picture(ANIM_WIN, b->old_loc->screen_x[term->num],
								 b->old_loc->screen_y[term->num],
								 pic, DRAW_XOR,
								 obullet_color);
						if ((b->type == DISC) && (obuf[0] != '\0')) {
							draw_text(ANIM_WIN, b->old_loc->screen_x[term->num],
								  b->old_loc->screen_y[term->num] - 4,
								  obuf, S_FONT, DRAW_XOR, obullet_color);
						}
					}
				}
			}
		}
		/* Draw the new picture of the bullet */
		if (status != OFF) {
			pic = &(bullet_obj[(int)b->type])->pic[(int)b->rot];
			if (b->life > 0 && b->life < weapon_stat[(int) b->type].frames) {
				if (settings.point_bullets == TRUE
					&& b->type != DISC
					&& !(b->disp_flgs & F_NOPT)) {
					draw_point(ANIM_WIN, b->loc->screen_x[term->num],
							   b->loc->screen_y[term->num], DRAW_XOR,
							   bullet_color);
				} else {
					if (b->disp_flgs & F_BEAM) {
						draw_line(ANIM_WIN,
							b->old_loc->screen_x[term->num],
							b->old_loc->screen_y[term->num],
							b->loc->screen_x[term->num],
							b->loc->screen_y[term->num],
							DRAW_XOR, bullet_color);
					} else {
						draw_picture(ANIM_WIN, b->loc->screen_x[term->num],
									 b->loc->screen_y[term->num],
									 pic, DRAW_XOR,
									 bullet_color);
						if ((b->type == DISC) &&
							(buf[0] != '\0')) {
							draw_text(ANIM_WIN, b->loc->screen_x[term->num],
									  b->loc->screen_y[term->num] - 4,
									  buf, S_FONT, DRAW_XOR, bullet_color);
						}
					}
				}
			}
		}
	}
#ifdef USE_BATCHED_POINTS
	if (settings.point_bullets)
		flush_point_batch;
#endif
#ifdef USE_BATCHED_LINES
	flush_line_batch;
#endif
}

/*
** Displays all the explosions visible in the animation window.
*/
void
display_explosions(unsigned int status)
{
	extern Eset *eset;
	Exp *e;
	Picture *pic;
	int i;

	for (i = 0; i < eset->number; i++) {
		e = eset->list[i];

		/* Compute the address of the old picture */
		pic = &e->obj->pic[e->obj->num_pics - e->life - 1];

		/* Erase the old picture of the explosion */
		if (status != ON)
			if (e->life < e->obj->num_pics)
				draw_picture(ANIM_WIN, e->old_screen_x[term->num],
					   e->old_screen_y[term->num], pic, DRAW_XOR, e->color);

		/* Draw the new picture of the explosion */
		if (status != OFF)
			if (e->life > 0) {
				pic++;
				draw_picture(ANIM_WIN, e->screen_x[term->num],
						   e->screen_y[term->num], pic, DRAW_XOR, e->color);
			}
	}
}

/* Draws a north wall */
#define draw_north_wall(b,x,y) \
  if(b->flags & NORTH_WALL) \
    draw_hor(ANIM_WIN,x,y,BOX_WIDTH,DRAW_XOR, \
	     (b->flags & NORTH_DEST) ? DEST_WALL : WHITE)

/* Draws a west wall */
#define draw_west_wall(b,x,y) \
  if(b->flags & WEST_WALL) \
    draw_vert(ANIM_WIN,x,y,BOX_HEIGHT,DRAW_XOR, \
	      (b->flags & WEST_DEST) ? DEST_WALL : WHITE)

/* Draws fuel, ammo, armor, and goal in center, outpost wherever it is */
#define draw_type(b,line_x,line_y,fr) \
  switch(b->type) { \
    case FUEL: case AMMO: case ARMOR: case GOAL: case PEACE: case TELEPORT: \
      pic = &landmark_obj[0]->pic[(int)b->type - 1]; \
      draw_picture(ANIM_WIN,line_x+BOX_WIDTH/2,line_y+BOX_HEIGHT/2, \
		   pic,DRAW_XOR,WHITE); \
      break; \
    case OUTPOST: \
      pic = &landmark_obj[0]->pic[(int)b->type - 1]; \
      oc = outpost_coordinate(b,fr); \
      draw_picture(ANIM_WIN,line_x+oc->x,line_y+oc->y,pic,DRAW_XOR, \
		   team_color_bright[b->team]); \
      break; \
    default: \
      break; \
  }

/* Draws team character in the center of the box */
#define draw_team(b,x,y) \
  if(b->team != NEUTRAL) { \
    buf[0] = team_char[b->team]; \
    draw_text(ANIM_WIN,x+BOX_WIDTH/2,y+BOX_HEIGHT/2-5, \
	      buf,S_FONT,DRAW_XOR,team_color[b->team]); \
  }

/*
** Displays all the walls and landmarks in the animation window.
*/
void
display_maze(unsigned int status)
{
	extern char team_char[];
	extern Object *landmark_obj[];
	extern Coord *outpost_coordinate();
	Coord *oc;
	Picture *pic;
	Intloc *sloc, *old_sloc;
	int right, bottom, i, j;
	int left_x, old_left_x, top_y, old_top_y;
	int line_x, old_line_x, line_y, old_line_y;
	int ox, oy;
	Box *b, *ob, temp;
	char buf[2];

	buf[1] = '\0';

	sloc = &term->loc;
	old_sloc = &term->old_loc;

	/* Draw just the north walls for the leftmost column of boxes. Draw just
       the west walls for the topmost row of boxes except for upperleftmost
       one, which isn't done at all.  For all the rest, do both the north and
       west walls. */
	switch (status) {
	  case REDISPLAY:
		  left_x = sloc->grid_x * BOX_WIDTH - sloc->x;
		  old_left_x = old_sloc->grid_x * BOX_WIDTH - old_sloc->x;

		  top_y = sloc->grid_y * BOX_HEIGHT - sloc->y;
		  old_top_y = old_sloc->grid_y * BOX_HEIGHT - old_sloc->y;

		  line_x = left_x;
		  old_line_x = old_left_x;
		  for (i = 0; i <= NUM_BOXES; i++) {
			  line_y = top_y;
			  old_line_y = old_top_y;
			  for (j = 0; j <= NUM_BOXES; j++) {
				  b = &real_map[sloc->grid_x + i][sloc->grid_y + j];
				  ox = old_sloc->grid_x + i;
				  oy = old_sloc->grid_y + j;
				  ob = &real_map[ox][oy];

				  /* If the old box has been changed, get the old value */
				  if (ob->flags & BOX_CHANGED)
					  if (old_box(&temp, ox, oy))
						  ob = &temp;

				  if (sloc->x != old_sloc->x || sloc->y != old_sloc->y || un_stingy) {

					  /* Redisplay walls */
					  if (j) {
						  draw_north_wall(ob, old_line_x, old_line_y);
						  draw_north_wall(b, line_x, line_y);
					  }
					  if (i) {
						  draw_west_wall(ob, old_line_x, old_line_y);
						  draw_west_wall(b, line_x, line_y);
					  }
				  } else {
					  if ((b->flags & NORTH_WALL) != (ob->flags & NORTH_WALL) || un_stingy) {

						  if (j) {
							  draw_north_wall(ob, old_line_x, old_line_y);
							  draw_north_wall(b, line_x, line_y);
						  }
					  }
					  if ((b->flags & WEST_WALL) != (ob->flags & WEST_WALL) || un_stingy) {
						  if (i) {
							  draw_west_wall(ob, old_line_x, old_line_y);
							  draw_west_wall(b, line_x, line_y);
						  }
					  }
				  }
				  if (sloc->x != old_sloc->x || sloc->y != old_sloc->y || ob->type == OUTPOST || un_stingy) {


					  /* Redisplay type */
					  draw_type(ob, old_line_x, old_line_y, frame - 1);
					  draw_type(b, line_x, line_y, frame);
				  }
				  /* Redisplay team */
				  if (sloc->x != old_sloc->x || sloc->y != old_sloc->y || b->team != ob->team || un_stingy) {

					  draw_team(ob, old_line_x, old_line_y);
					  draw_team(b, line_x, line_y);
				  }
				  line_y += BOX_HEIGHT;
				  old_line_y += BOX_HEIGHT;
			  }
			  line_x += BOX_WIDTH;
			  old_line_x += BOX_WIDTH;
		  }
		  break;
	  case ON:
		  right = sloc->grid_x + NUM_BOXES + 1;
		  bottom = sloc->grid_y + NUM_BOXES + 1;

		  left_x = sloc->grid_x * BOX_WIDTH - sloc->x;
		  top_y = sloc->grid_y * BOX_HEIGHT - sloc->y;

		  line_x = left_x;
		  for (i = sloc->grid_x; i < right; i++) {
			  line_y = top_y;
			  for (j = sloc->grid_y; j < bottom; j++) {
				  b = &real_map[i][j];

				  /* Draw walls */
				  if (j != sloc->grid_y)
					  draw_north_wall(b, line_x, line_y);
				  if (i != sloc->grid_x)
					  draw_west_wall(b, line_x, line_y);

				  /* Draw type */
				  draw_type(b, line_x, line_y, frame)
				  /* Draw team */
					draw_team(b, line_x, line_y);

				  line_y += BOX_HEIGHT;
			  }
			  line_x += BOX_WIDTH;
		  }
		  break;
	}
#ifdef USE_BATCHED_LINES
	flush_line_batch;
#endif
}

/*
** Displays mapper and radar in the map window.
*/
void
display_map(unsigned int status)
{
	Vehicle *v = term->vehicle;

	/* Check for being exposed */
	check_expose(MAP_WIN, status);

	if (status == ON)
		clear_window(MAP_WIN);

	if (v != NULL) {
		unsigned int action;

		switch (status) {
		  case REDISPLAY:
			  action = SP_redisplay;
			  break;
		  case ON:
			  action = SP_draw;
			  break;
		  case OFF:
			  action = SP_erase;
			  break;
		}
		if (tstflag(v->status, VS_is_alive)) {
			do_special(v, MAPPER, action);
			do_special(v, RADAR, action);
			do_special(v, NEW_RADAR, action);
			do_special(v, TACLINK, action);
#ifndef NO_CAMO
			do_special(v, STEALTH, action);
			do_special(v, CAMO, action);
			do_special(v, RDF, action);
#endif /* !NO_CAMO */
		} else {
			if (v->death_timer == DEATH_DELAY - 1) {
				do_special(v, RADAR, SP_erase);
			}
		}
	} else if (term->observer) {
		full_mapper(status);
		full_radar(status);
	}
}

/*
** Displays the console information in the console window.
*/
void
display_cons(unsigned int status)
{
	unsigned int action;

	/* Check for being exposed */
	check_expose(CONS_WIN, status);

	switch (status) {
	  case REDISPLAY:
		  action = SP_redisplay;
		  break;
	  case ON:
		  clear_window(CONS_WIN);
		  action = SP_draw;
		  break;
	  case OFF:
		  action = SP_erase;
		  break;
	}

	if (term->vehicle != (Vehicle *) NULL)
		do_special(term->vehicle, CONSOLE, action);
}

/* these two message arrays must have the same size: */
char *help_normal[] =
{
	"BASIC FUNCTIONS                                       DISC CONTROL           GAME FUNCTIONS         REMOTE CTRL FUNCTIONS   SYNC CONTROL",
	"space  fire weapons (left)     C   toggle console     s   spin <=            Q   quit game          .   turn right          i   every frame",
	"t      turn turret (middle)    M   toggle mapper      d   spin toggle        P   pause game         ,   turn left           o   every 2 frames",
	"g      turn tank   (right)     R   toggle radar       f   spin =>            <   slow down game     /   detonate            p   every 4 frames",
	"0-9    set forward drive       z   toggle safety      w   throw slow         >   speed up game      V   toggle view         [   every 8 frames",
	"-      full reverse drive      +   toggle repair      e   throw medium                                                      ]   every 16 frames",
	"!@#$%^ toggle weapons 1-6      F   toggle teleport    r   throw fast",
	"=      toggle all weapons      c   stop",
	"return send message            v   speed up",
	"B      toggle beep             x   slow down",
/* Added 'toggle beep' (HAK 2/93) */
};
char *help_battle[] =
{
	"0-9     track vehicle",
	"button  move view",
	"space   pause game",
	"w       map battle windows",
	"W       unmap battle windows",
	"<       slow game down",
	">       speed game up",
	"Q       quit",
	"",
	""
};

/*
** Displays helpful information in the help window.
*/
void
display_help(unsigned int status)
{
	int i, lim;
	char **text;

	/* Check for being exposed */
	check_expose(HELP_WIN, status);

	if (status == ON) {
		clear_window(HELP_WIN);

		/* Determine which text to show in the help window */
		if (term->observer) {
			text = help_battle;
		} else {
			text = help_normal;
		}

		lim = (term->observer) ? sizeof(help_normal) / sizeof(help_normal[0]) :
		  sizeof(help_battle) / sizeof(help_battle[0]);
		for (i = 0; i < lim; i++)
			display_mesg(HELP_WIN, text[i], i, T_FONT);
	}
}

#ifdef S1024x864
#define VEH_X 1
#define VEH_Y 0
#define VEHICLE_H 46
#define VEHICLE_W 60

#define BULLET_X  180
#define BULLET_Y  0
#define BULLET_H  26

#define LAND_X    440
#define LAND_Y    0
#define LAND_H    27
#define LAND_W    85

#define EXP_X     160
#define EXP_Y     615
#define EXP_H     45
#define EXP_W     180

#define MSG_X     8
#define MSG_Y     2

#define PIC_X   30
#define PIC_Y   50
#define EXP_PIC_Y    -13
#define TEXT_OFFSET  48
#endif

/*
** Displays pictures of all bodies, bullets, explosions, and landmarks.
*/
void
display_pics(void)
{
	int exp_view, v_view, max_pics, split;
	int xpos;

	display_mesg2(HELP_WIN, "Hit any key or button to continue",
				  MSG_X, MSG_Y, XL_FONT);

	clear_window(ANIM_WIN);

	/* Put in separator rectangles between the pictures */
	draw_filled_rect(ANIM_WIN, BULLET_X - 1, 0, 3, EXP_Y, DRAW_COPY, WHITE);
	draw_filled_rect(ANIM_WIN, LAND_X - 1, 0, 3, EXP_Y, DRAW_COPY, WHITE);
	draw_filled_rect(ANIM_WIN, 0, EXP_Y - 1, ANIM_WIN_WIDTH, 3, DRAW_COPY,
					 WHITE);

	/* Draw the vehicles in one column */
	draw_text(ANIM_WIN, BULLET_X / 2, VEH_Y + 5, "Vehicles", L_FONT, DRAW_COPY,
			  WHITE);
	draw_objs(vehicle_obj, TRUE, 0, num_vehicle_objs, 0, VEH_X, VEH_Y,
			  VEHICLE_H);

	/* Draw the bullets in a column */
	draw_text(ANIM_WIN, (BULLET_X + LAND_X) / 2, BULLET_Y + 5,
			  "Bullets", L_FONT, DRAW_COPY, WHITE);
	draw_objs(bullet_obj, TRUE, 0, num_bullet_objs, 0, BULLET_X, BULLET_Y, BULLET_H);

	/* Draw the landmarks in three columns */
	draw_text(ANIM_WIN, (LAND_X + ANIM_WIN_WIDTH) / 2, LAND_Y + 5,
			  "Landmarks", L_FONT, DRAW_COPY, WHITE);
	draw_text(ANIM_WIN, LAND_X + PIC_X + LAND_W, LAND_Y + 38,
			  "Game      Map   Design", M_FONT, DRAW_COPY, WHITE);
	draw_obj(landmark_obj[0], 1, LAND_X, LAND_Y + LAND_H, LAND_H);
	draw_obj(landmark_obj[1], 2, LAND_X + LAND_W, LAND_Y + LAND_H, LAND_H);
	draw_obj(landmark_obj[2], 3, LAND_X + 2 * LAND_W, LAND_Y + 3, LAND_H);

	/* Draw the explosions in 3 columns */
	split = (num_exp_objs + 1) / 3;
	draw_text_left(ANIM_WIN, 15, EXP_Y + 15, "Explosions", L_FONT, DRAW_COPY,
				   WHITE);
	xpos = EXP_X;
	draw_objs(exp_obj, TRUE, 0, split, 0, xpos, EXP_Y + EXP_PIC_Y, EXP_H);
	xpos += EXP_W;
	draw_objs(exp_obj, TRUE, split, 2 * split, 0, xpos, EXP_Y + EXP_PIC_Y,
			  EXP_H);
	xpos += EXP_W;
	draw_objs(exp_obj, TRUE, 2 * split, num_exp_objs, 0, xpos,
			  EXP_Y + EXP_PIC_Y, EXP_H);

	/* Animate the explosions and vehicles until a key or button is pressed */

	max_pics = vehicle_obj[0]->num_pics;	/* hack */
	exp_view = 0;
	v_view = 0;
	while (!scan_input()) {
		usleep(40000);
		if (++exp_view >= max_pics) {
			exp_view = 0;

			if (++v_view >= max_pics)
				v_view = 0;

			/* rotate vehicles one step */

#if 1
			draw_filled_rect(ANIM_WIN, VEH_X + PIC_X - VEHICLE_W / 2,
							 VEH_Y + PIC_Y - VEHICLE_H / 2,
							 VEHICLE_W, num_vehicle_objs * VEHICLE_H,
							 DRAW_COPY, BLACK);
			draw_objs(vehicle_obj, FALSE, 0, num_vehicle_objs, v_view,
					  VEH_X, VEH_Y, VEHICLE_H);
#else
			/* draw in 2 sets to reduce flicker */
			draw_filled_rect(ANIM_WIN, VEH_X + PIC_X - VEHICLE_W / 2,
							 VEH_Y + PIC_Y - VEHICLE_H / 2,
							 VEHICLE_W, vheight,
							 DRAW_COPY, BLACK);
			draw_objs(vehicle_obj, FALSE, 0, num_vehicle_objs / 2, v_view,
					  VEH_X, VEH_Y, VEHICLE_H);
			draw_filled_rect(ANIM_WIN, VEH_X + PIC_X - VEHICLE_W / 2,
							 VEH_Y + PIC_Y - VEHICLE_H / 2 + vheight,
							 VEHICLE_W, vheight,
							 DRAW_COPY, BLACK);
			draw_objs(vehicle_obj, FALSE, num_vehicle_objs / 2,
					  num_vehicle_objs, v_view, VEH_X, VEH_Y + vheight,
					  VEHICLE_H);
#endif
		}
		xpos = EXP_X + PIC_X - EXP_H / 2;
		draw_filled_rect(ANIM_WIN,
						 xpos, EXP_Y + EXP_PIC_Y + PIC_Y - EXP_H / 2,
						 EXP_H, split * EXP_H, DRAW_COPY, BLACK);
		xpos += EXP_W;
		draw_filled_rect(ANIM_WIN,
						 xpos, EXP_Y + EXP_PIC_Y + PIC_Y - EXP_H / 2,
						 EXP_H, (2 * split) * EXP_H,
						 DRAW_COPY, BLACK);
		xpos += EXP_W;
		draw_filled_rect(ANIM_WIN,
						 xpos, EXP_Y + EXP_PIC_Y + PIC_Y - EXP_H / 2,
						 EXP_H, (num_exp_objs - 2 * split) * EXP_H,
						 DRAW_COPY, BLACK);

		xpos = EXP_X;
		draw_objs(exp_obj, FALSE, 0, split, exp_view, xpos, EXP_Y + EXP_PIC_Y,
				  EXP_H);
		xpos += EXP_W;
		draw_objs(exp_obj, FALSE, split, 2 * split, exp_view, xpos,
				  EXP_Y + EXP_PIC_Y, EXP_H);
		xpos += EXP_W;
		draw_objs(exp_obj, FALSE, 2 * split, num_exp_objs, exp_view, xpos,
				  EXP_Y + EXP_PIC_Y, EXP_H);
		sync_output(FALSE);
	}

	clear_window(HELP_WIN);
}

/*
** Draws all the objects in the array in a vertical column, starting
** at the specified location and working downwards in jumps of height.
** The specified view is used for each object, provided it exists.
*/
void
draw_objs(Object *obj[], Boolean text, int first, int last, int view, int x, int y, int height)
{
	int i;

	for (i = first; i < last; i++)
		if (view < obj[i]->num_pics)
			draw_picture_string(obj[i], view, (text ? obj[i]->type : ""),
								x + PIC_X,
								y + PIC_Y + height * (i - first), 0);
}

/* Keep names of landmarks in a global where others can see 'em */

char *box_type_name[NUM_LANDMARK_TYPES];

void
init_box_names(void)
{
	int i;

	for (i = 0; i < NUM_LANDMARK_TYPES; i++) {
		box_type_name[i] = "???";
	}

	box_type_name[FUEL] = "fuel";
	box_type_name[AMMO] = "ammo";
	box_type_name[ARMOR] = "armor";
	box_type_name[GOAL] = "goal";
	box_type_name[OUTPOST] = "outpost";
	box_type_name[PEACE] = "peace";
	box_type_name[TELEPORT] = "teleport";
	box_type_name[SCROLL_N] = "scroll";
	box_type_name[SCROLL_NE] = "scroll";
	box_type_name[SCROLL_E] = "scroll";
	box_type_name[SCROLL_SE] = "scroll";
	box_type_name[SCROLL_S] = "scroll";
	box_type_name[SCROLL_SW] = "scroll";
	box_type_name[SCROLL_W] = "scroll";
	box_type_name[SCROLL_NW] = "scroll";
	box_type_name[SLIP] = "slip";
	box_type_name[SLOW] = "slow";
	box_type_name[START_POS] = "start";
}

/*
** Draws all the views of a given object in a vertical column, starting
** at the specified location and working downwards in jumps of height.
*/
void
draw_obj(Object *obj, int type, int x, int y, int height)
{
	extern Weapon_stat weapon_stat[];
	char *str;
	int adj, i;

	for (i = 0; i < obj->num_pics; i++) {
		adj = 0;
		str = "";
		if (type == 0)
			str = weapon_stat[i].type;
		else if (type == 2)
			adj = -4;
		else if (type == 3) {
			/* Skip drawing the empty pixmap for normal landmark */
			if (i == 0)
				continue;
			str = box_type_name[i];
			adj = -13;
		}
		draw_picture_string(obj, i, str, x + PIC_X, y + PIC_Y + height * i,
							adj);
	}
}


/*
** Draws the specified view of the object with the string written beneath
** at the specified location in the animation window.  The adj parameter
** is added to the picture coordinates but not the text coordinates.
*/
void
draw_picture_string(Object *obj, int view, char *str, int x, int y, int adj)
{
	draw_picture(ANIM_WIN, x + adj, y + adj, &obj->pic[view], DRAW_COPY,
				 WHITE);
	if (str[0] != '\0')
		draw_text_left(ANIM_WIN, x + TEXT_OFFSET, y - font_height(M_FONT) / 2,
					   str, M_FONT, DRAW_COPY, WHITE);
}
