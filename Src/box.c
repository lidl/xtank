/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "common.h"
#include "xtank.h"
#include "vstructs.h"
#include "sysdep.h"
#include "vehicle.h"
#include "outpost.h"
#include "globals.h"
#include "terminal.h"
#include "bullet.h"
#include "disc.h"
#include "graphics.h"			/* for DRAW_XOR and WHITE */
#include "gr.h"					/* for MAP_WIN */
#include "proto.h"

extern Boolean intersect_wall();

extern Map real_map;
extern int frame;
extern Settings settings;

/* Whether an outpost fires */
#define OUTPOST_FIRE  32		/* should be a power of 2 for the
				   outpost_shoot() macro to work */
#define outpost_shoot(b,fr) \
  ((b->strength >= 32) ? 1 : (outpost_fire[b->strength & (OUTPOST_FIRE-1)] \
			      >> (fr&31)) & 0x1)

/* When the nth bit in the integer is on, fire.  n is the frame mod 32. */
static unsigned int outpost_fire[OUTPOST_FIRE] =
{
	0x80808080, 0x80808080, 0x82081040, 0x82081040,
	0x84208420, 0x84208420, 0x88442210, 0x88442210,
	0x88888888, 0x88888888, 0x91244448, 0x91244448,
	0x92489248, 0x92489248, 0xa4924924, 0xa4924924,
	0xa4a4a4a4, 0xa4a4a4a4, 0xa52a5294, 0xa52a5294,
	0xaa54aa54, 0xaa54aa54, 0xaaaa5554, 0xaaaa5554,
	0xaaaaaaaa, 0xaaaaaaaa, 0xd555aaaa, 0xd555aaaa,
	0xd5aad5aa, 0xd5aad5aa, 0xd6b5ab5a, 0xd6b5ab5a
};


#if lint						/* lint hates those initializations */
Coord outpost_coord[OUTPOST_PATS][OUTPOST_FRAMES];

#else
Coord outpost_coord[OUTPOST_PATS][OUTPOST_FRAMES] =
{
#define W BOX_WIDTH
#define H BOX_HEIGHT
	{							/* square */
		{W * .891, H * .500},
		{W * .891, H * .597},
		{W * .891, H * .696},
		{W * .891, H * .793},
		{W * .891, H * .891},
		{W * .793, H * .891},
		{W * .696, H * .891},
		{W * .597, H * .891},
		{W * .500, H * .891},
		{W * .403, H * .891},
		{W * .304, H * .891},
		{W * .207, H * .891},
		{W * .109, H * .891},
		{W * .109, H * .793},
		{W * .109, H * .696},
		{W * .109, H * .597},
		{W * .109, H * .500},
		{W * .109, H * .403},
		{W * .109, H * .304},
		{W * .109, H * .207},
		{W * .109, H * .109},
		{W * .207, H * .109},
		{W * .304, H * .109},
		{W * .403, H * .109},
		{W * .500, H * .109},
		{W * .597, H * .109},
		{W * .696, H * .109},
		{W * .793, H * .109},
		{W * .891, H * .109},
		{W * .891, H * .207},
		{W * .891, H * .304},
		{W * .891, H * .403},
	},
	{							/* diamond */
		{W * .891, H * .500},
		{W * .842, H * .549},
		{W * .793, H * .597},
		{W * .744, H * .647},
		{W * .696, H * .696},
		{W * .647, H * .744},
		{W * .597, H * .793},
		{W * .549, H * .842},
		{W * .500, H * .891},
		{W * .451, H * .842},
		{W * .403, H * .793},
		{W * .353, H * .744},
		{W * .304, H * .696},
		{W * .256, H * .647},
		{W * .207, H * .597},
		{W * .158, H * .549},
		{W * .109, H * .500},
		{W * .158, H * .451},
		{W * .207, H * .403},
		{W * .256, H * .353},
		{W * .304, H * .304},
		{W * .353, H * .256},
		{W * .403, H * .207},
		{W * .451, H * .158},
		{W * .500, H * .109},
		{W * .549, H * .158},
		{W * .597, H * .207},
		{W * .647, H * .256},
		{W * .696, H * .304},
		{W * .744, H * .353},
		{W * .793, H * .403},
		{W * .842, H * .451},
	},
	{							/* circle */
		{W * .891, H * .500},
		{W * .883, H * .576},
		{W * .861, H * .649},
		{W * .825, H * .717},
		{W * .776, H * .776},
		{W * .717, H * .825},
		{W * .649, H * .861},
		{W * .576, H * .883},
		{W * .500, H * .891},
		{W * .424, H * .883},
		{W * .351, H * .861},
		{W * .283, H * .825},
		{W * .224, H * .776},
		{W * .175, H * .717},
		{W * .139, H * .649},
		{W * .117, H * .576},
		{W * .109, H * .500},
		{W * .117, H * .424},
		{W * .139, H * .351},
		{W * .175, H * .283},
		{W * .224, H * .224},
		{W * .283, H * .175},
		{W * .351, H * .139},
		{W * .424, H * .117},
		{W * .500, H * .109},
		{W * .576, H * .117},
		{W * .649, H * .139},
		{W * .717, H * .175},
		{W * .776, H * .224},
		{W * .825, H * .283},
		{W * .861, H * .351},
		{W * .883, H * .424},
	},
	{							/* clover */
		{W * .891, H * .500},
		{W * .861, H * .578},
		{W * .793, H * .617},
		{W * .696, H * .637},
		{W * .617, H * .617},
		{W * .637, H * .696},
		{W * .617, H * .793},
		{W * .578, H * .861},
		{W * .500, H * .891},
		{W * .422, H * .861},
		{W * .383, H * .793},
		{W * .363, H * .696},
		{W * .383, H * .617},
		{W * .304, H * .637},
		{W * .207, H * .617},
		{W * .139, H * .578},
		{W * .109, H * .500},
		{W * .139, H * .422},
		{W * .207, H * .383},
		{W * .304, H * .363},
		{W * .383, H * .383},
		{W * .363, H * .304},
		{W * .383, H * .207},
		{W * .422, H * .139},
		{W * .500, H * .109},
		{W * .578, H * .139},
		{W * .617, H * .207},
		{W * .637, H * .304},
		{W * .617, H * .383},
		{W * .696, H * .363},
		{W * .793, H * .383},
		{W * .861, H * .422},
	},
#undef W
#undef H
};
#endif /* lint */



/*
** Checks for special box features and handles the different box types.
** Activates any outposts within 2 boxes of the vehicle.
** Puts adjustments of x and y components of speed into xadj and yadj.
*/
void
box_type_check(Vehicle *v, Box *b, FLOAT *xadj, FLOAT *yadj)
{
	int x, y;

	*xadj = 0;
	*yadj = 0;
	switch (b->type) {
	  case FUEL:
	  case AMMO:
	  case ARMOR:
	  case PEACE:
	  case TELEPORT:
		  box_landmark(v, b);
		  break;
	  case SCROLL_N:
	  case SCROLL_NE:
	  case SCROLL_E:
	  case SCROLL_SE:
	  case SCROLL_S:
	  case SCROLL_SW:
	  case SCROLL_W:
	  case SCROLL_NW:
		  box_scroll(b->type, xadj, yadj);
		  break;
	  case SLOW:
		  box_slow(v);
		  break;
	}

#ifndef NO_CAMO
	if (!v->camod)
#endif /* !NO_CAMO */

		/* Activate any outposts within 2 squares of the vehicle */
		for (x = v->loc->grid_x - 2; x <= v->loc->grid_x + 2; x++)
			for (y = v->loc->grid_y - 2; y <= v->loc->grid_y + 2; y++) {
				b = &real_map[x][y];
				if (b->type == OUTPOST)
					box_outpost(v, b, x, y);
			}
}

/*
** Applies the effects of a landmark on the specified vehicle.
*/
void
box_landmark(Vehicle *v, Box *b)
{
	extern Engine_stat engine_stat[];
	extern Weapon_stat weapon_stat[];
	extern Armor_stat armor_stat[];
	extern Body_stat body_stat[];
	Loc *loc;
	Weapon *w;
	Weapon_stat *ws;
	int *side, *max_side;
	int cost, i;

	loc = v->loc;

	/* If we're moving, ignore it */
	if (v->vector.speed == 0.0 && b->type == TELEPORT ||
		v->vector.speed != 0.0 && b->type != TELEPORT)
		return;

	/* If we're not close, ignore it */
	if (loc->box_x < BOX_WIDTH / 2 - LANDMARK_WIDTH / 2 ||
		loc->box_x > BOX_WIDTH / 2 + LANDMARK_WIDTH / 2 ||
		loc->box_y < BOX_HEIGHT / 2 - LANDMARK_HEIGHT / 2 ||
		loc->box_y > BOX_HEIGHT / 2 + LANDMARK_HEIGHT / 2)
		return;

	switch (b->type) {
	  case FUEL:
		  /* Add one to fuel */
		  cost = engine_stat[v->vdesc->engine].fuel_cost;
		  if (v->owner->money >= cost && v->fuel < v->max_fuel) {
			  v->fuel++;
			  v->owner->money -= cost;
		  }
		  break;
	  case AMMO:
		  /* Add one to ammo of all weapons that are off but functioning */
		  for (i = 0; i < v->num_weapons; i++) {
			  w = &v->weapon[i];
			  if ((w->status & WS_func) && !(w->status & WS_on)) {
				  ws = &weapon_stat[(int) v->vdesc->weapon[i]];
				  if (v->owner->money >= ws->ammo_cost && w->ammo < ws->max_ammo)
/*
 * If the refill_time is 0, filled the weapon up to capacity
 * instantly
 *
 * 1 is the "normal" rate, ie 1 bullet per frame
 *
 * n requires n frames per bullet
 */

					  if (weapon_stat[(int) w->type].refill_time == 0) {
						  int needed;
						  int afford;
						  int get;

						  needed = ws->max_ammo - w->ammo;

						  if (ws->ammo_cost) {
							  afford = v->owner->money / ws->ammo_cost;
							  get = MIN(needed, afford);
						  } else {
							  get = needed;
						  }

						  if (get) {
							  w->status &= ~WS_no_ammo;
							  v->owner->money -= (ws->ammo_cost * get);
							  w->ammo += get;
						  }
					  } else if (w->refill_counter != 1)
						  --w->refill_counter;
					  else {
						  w->refill_counter = weapon_stat[(int) w->type].refill_time;

						  w->ammo++;
						  w->status &= ~WS_no_ammo;
						  v->owner->money -= ws->ammo_cost;
					  }
			  }
		  }
		  break;
	  case ARMOR:
		  /* Add one armor point to all sides every 3 frames */
		  if (frame % 3 != 0)
			  break;

		  side = v->armor.side;
		  max_side = v->vdesc->armor.side;

		  cost = armor_stat[v->vdesc->armor.type].cost *
			body_stat[v->vdesc->body].size;
		  for (i = 0; i < MAX_SIDES; i++) {
			  if (v->owner->money >= cost && side[i] < max_side[i]) {
				  side[i]++;
				  v->owner->money -= cost;

				  /* Make a gleam or two every now and then */
				  if (!rnd(30))
					  explode_location(v->loc, rnd(2), EXP_GLEAM);
			  }
		  }
		  break;
	  case TELEPORT:
		  {
			  extern Terminal *term;
			  extern Bset *bset;
			  Bullet *bullet;
			  int xd, yd, x, y, xmax = -1, ymax, xdmax, ydmax, found = 0, i;
			  Box *dest;
			  float ftemp, fmax;

			  /* if this teleport isn't neutral, or on our side, ignore it */
			  if (v->teleport == TRUE &&
				  ((b->team == v->team && settings.si.teleport_from_team) ||
				(b->team == NEUTRAL && settings.si.teleport_from_neutral) ||
				   (settings.si.teleport_any_to_any))) {
				  /* else search the maze for a different teleport */
				  for (x = 0; x < GRID_WIDTH; x++)
					  for (y = 0; y < GRID_HEIGHT; y++) {
						  if (x == v->loc->grid_x && y == v->loc->grid_y)
							  continue;

						  dest = &real_map[x][y];
						  if (dest->type == TELEPORT &&
							  dest->teleport_code == b->teleport_code &&
							  ((dest->team == v->team && settings.si.teleport_to_team) ||
							   (dest->team == NEUTRAL && settings.si.teleport_to_neutral) ||
							   (dest->team == b->team && settings.si.teleport_any_to_any))) {
							  xd = x - v->loc->grid_x;
							  yd = y - v->loc->grid_y;

							  if (xmax != -1 &&
								  xdmax * yd == xd * ydmax) {
								  /* found one on same line as best */
								  if ((ydmax == 0 && xd * xdmax > 0) ||
									  yd * ydmax > 0) {
									  /* it's in the same direction as the best */
									  if ((ydmax == 0 && ABS(xd) < ABS(xdmax)) ||
										  ABS(yd) < ABS(ydmax)) {
										  /* it's nearer than the best - use it */
										  xmax = x;
										  ymax = y;
										  xdmax = xd;
										  ydmax = yd;
									  }
									  /* else */
									  /* it's furthur away than the best */
								  } else if (fmax < 0) {
									  /* it's in the opposite direction - good */
									  fmax = -fmax;
									  xmax = x;
									  ymax = y;
									  xdmax = xd;
									  ydmax = yd;
								  }
								  /* else */
								  /* it's in the wrong direction */
							  } else {
								  /* this is a new direction - calculate how close to our
					     * current direction of travel this offset is */
								  ftemp = (v->vector.xspeed * xd +
										   v->vector.yspeed * yd) /
									SQRT((double) (xd * xd + yd * yd));

								  /* keep a note of the closest one */
								  if (xmax == -1 ||
									  ftemp > fmax) {
									  fmax = ftemp;
									  xmax = x;
									  ymax = y;
									  xdmax = xd;
									  ydmax = yd;
								  }
							  }
						  }
					  }

				  /* if we found a suitable place to jump to */
				  if (xmax != -1) {
					  /* calculate an offset to take us there ensuring that we */
					  /* land just past the teleport, so that we don't */
					  /* immediately re-enter it */
					  if (ABS(v->vector.xspeed) > ABS(v->vector.yspeed)) {
						  xd = (int) (BOX_WIDTH / 2 - v->old_loc->box_x +
									  (v->vector.xspeed < 0 ? -1 : 1) *
									  (LANDMARK_WIDTH + 4) / 2 +
								   (xmax - v->old_loc->grid_x) * BOX_WIDTH);
						  yd = (int) (BOX_HEIGHT / 2 - v->old_loc->box_y +
								  v->vector.yspeed / ABS(v->vector.xspeed) *
									  LANDMARK_HEIGHT / 2 +
								  (ymax - v->old_loc->grid_y) * BOX_HEIGHT);
					  } else {
						  xd = (int) (BOX_WIDTH / 2 - v->old_loc->box_x +
								  v->vector.xspeed / ABS(v->vector.yspeed) *
									  LANDMARK_WIDTH / 2 +
								   (xmax - v->old_loc->grid_x) * BOX_WIDTH);
						  yd = (int) (BOX_HEIGHT / 2 - v->old_loc->box_y +
									  (v->vector.yspeed < 0 ? -1 : 1) *
									  (LANDMARK_HEIGHT + 4) / 2 +
								  (ymax - v->old_loc->grid_y) * BOX_HEIGHT);
					  }

					  if (settings.si.player_teleport) {
						  adjust_loc(v->old_loc, xd, yd);
						  v->just_ported = TRUE;
					  }
					  if (settings.si.disc_teleport && v->num_discs > 0)
						  for (i = 0; i < bset->number; i++) {
							  bullet = bset->list[i];

							  if (bullet->type == DISC && bullet->owner == v)
								  adjust_loc(bullet->loc, xd, yd);

							  if (!settings.si.player_teleport)
								  release_discs(v, DISC_FAST_SPEED, TRUE);
						  }
				  }
			  }
			  break;
		  }
	}
}

/*
** If the vehicle is an enemy, the outpost shoots it with a leading fan.
*/
void
box_outpost(Vehicle *v, Box *b, int grid_x, int grid_y)
{
	Loc oloc;
	double ang, lead, dx, dy;
	WeaponType btype;

	/* If outpost isn't neutral, and the vehicle is on our team, ignore it */
	if (b->team != NEUTRAL && b->team == v->team)
		return;

	/* See if outpost is strong enough to shoot this frame */
	if (!outpost_shoot(b, frame))
		return;

	/* If there is no clear shooting path, ignore it */
	outpost_loc(b, &oloc, grid_x, grid_y);
	if (intersect_wall(&oloc, v->loc))
		return;

	/* If this vehicle is not the closest one to the outpost, ignore it */
	if (!closest_vehicle(&oloc, v))
		return;

	/* Compute angle to the vehicle with random fan lead */
	dx = v->loc->x - oloc.x;
	dy = v->loc->y - oloc.y;
	lead = rnd((int) b->strength) * SQRT(dx * dx + dy * dy) / 300;
	dx += v->vector.xspeed * lead;
	dy += v->vector.yspeed * lead;
	ang = ATAN2(dy, dx);

	/* Choose a bullet type based on outpost strength */

	btype = (WeaponType) ((b->strength >> 2) - 5 + rnd(11));
	if ((int) btype > (int) SEEKER)
		btype = SEEKER;
	if ((int) btype < (int) LMG)
		btype = LMG;

	make_bullet((Vehicle *) NULL, &oloc, btype, ang, NULL);
}

int
closest_vehicle(Loc *loc, Vehicle *target)
{
	Vehicle *v;
	int i, dx, dy, dist;

	dx = (int) target->loc->x - (int) loc->x;
	dy = (int) target->loc->y - (int) loc->y;
	dist = dx * dx + dy * dy;

	/* Check all vehicles near outpost and determine distances */
	for (i = 0; i < num_veh_alive; i++) {
		v = live_vehicles[i];
		if (v == target)
			continue;
		dx = v->loc->grid_x - loc->grid_x;
		if (dx < -2 || dx > 2)
			continue;
		dy = v->loc->grid_y - loc->grid_y;
		if (dy < -2 || dy > 2)
			continue;

		dx = (int) v->loc->x - (int) loc->x;
		dy = (int) v->loc->y - (int) loc->y;
		if (dx * dx + dy * dy < dist)
			return FALSE;
	}
	return TRUE;
}

/*
** Creates a location structure for an outpost at the given grid coordinates
** in the given box.
*/
void
outpost_loc(Box *b, Loc *oloc, int grid_x, int grid_y)
{
	Coord *oc;

	oc = get_outpost_coord(b, frame);
	oloc->grid_x = grid_x;
	oloc->grid_y = grid_y;
	oloc->box_x = oc->x;
	oloc->box_y = oc->y;
	oloc->x = BOX_WIDTH * grid_x + oc->x;
	oloc->y = BOX_HEIGHT * grid_y + oc->y;
	oloc->z = SIDE_MOUNT_Z;
}

/*
** Returns whether a location collides with an outpost in the box.
*/
int
coll_outpost(Box *b, Loc *loc)
{
	Coord *oc;
	int dx, dy;

	oc = get_outpost_coord(b, frame);
	dx = oc->x - loc->box_x;
	dy = oc->y - loc->box_y;
	return (dx > -OUTPOST_WIDTH / 2 && dx < OUTPOST_WIDTH / 2 &&
		 dy > -OUTPOST_HEIGHT / 2 && dy < OUTPOST_WIDTH / 2) ? TRUE : FALSE;
}

Coord *
outpost_coordinate(Box *b, int fr)
{
	return get_outpost_coord(b, fr);
}

/*
** Increases the xspeed and yspeed of the vehicle in the scroll direction.
*/
void
box_scroll(LandmarkType type, FLOAT *xadj, FLOAT *yadj)
{
	FLOAT ss;

	ss = settings.si.scroll_speed;

	switch (type) {
	  case SCROLL_N:
		  *xadj = 0.0;
		  *yadj = -ss;
		  break;
	  case SCROLL_NE:
		  *xadj = ss / SQRT_2;
		  *yadj = -ss / SQRT_2;
		  break;
	  case SCROLL_E:
		  *xadj = ss;
		  *yadj = 0.0;
		  break;
	  case SCROLL_SE:
		  *xadj = ss / SQRT_2;
		  *yadj = ss / SQRT_2;
		  break;
	  case SCROLL_S:
		  *xadj = 0.0;
		  *yadj = ss;
		  break;
	  case SCROLL_SW:
		  *xadj = -ss / SQRT_2;
		  *yadj = ss / SQRT_2;
		  break;
	  case SCROLL_W:
		  *xadj = -ss;
		  *yadj = 0.0;
		  break;
	  case SCROLL_NW:
		  *xadj = -ss / SQRT_2;
		  *yadj = -ss / SQRT_2;
		  break;
	}
}

/*
** Multiplies xspeed and yspeed of vehicle by box_slow.
*/
void
box_slow(Vehicle *v)
{
	v->vector.xspeed *= settings.si.box_slowdown;
	v->vector.yspeed *= settings.si.box_slowdown;
}

/*
** The remaining procedures support the changing of box information.
**
** At the beginning of each frame, init_changed_boxes() should be called
** to remove the old BOX_CHANGED flag from last frame's changed boxes,
** and to set the size of the changed array to 0.
**
** Before changing any aspect of a box, call the change_box() routine, which
** adds the box's to the changed array, and sets the box's BOX_CHANGED flag.
**
** To find out the old value of a box, call old_box().
*/

typedef struct {
	Box box;
	Byte x, y;
}
Changed_box;

#define MAX_CHANGED_BOXES 20

static Changed_box changed_box[MAX_CHANGED_BOXES];
static int num_changed_boxes = 0;

/*
** Sets all changed boxes back to unchanged and num_changed_boxes to 0.
*/
void
init_changed_boxes(void)
{
	extern Map real_map;
	Changed_box *cb;
	int i;

	/* Turn off all of the BOX_CHANGED flags */
	for (i = 0; i < num_changed_boxes; i++) {
		cb = &changed_box[i];
		real_map[cb->x][cb->y].flags &= ~BOX_CHANGED;
	}

	/* Set the size of the changed_box array to 0 */
	num_changed_boxes = 0;
}

/*
** Stores the state of the box into the changed_box array.
** Marks box as changed, to indicate that its old value is here.
** Returns 1 if successful, 0 if unsuccessful.
*/
int
change_box(Box *b, int x, int y)
{
	Changed_box *cb;

	/* See if we have room to store the old box */
	if (num_changed_boxes >= MAX_CHANGED_BOXES) {
		fprintf(stderr, "warning: changed_box[] is full...n\n");
		return 0;
	}
	/* Store the box in the changed_box array */
	cb = &changed_box[num_changed_boxes++];
	cb->box = *b;
	cb->x = (Byte) x;
	cb->y = (Byte) y;

	/* Mark the box as being changed */
	b->flags |= BOX_CHANGED;

	return 1;
}

/*
** Puts the old contents of the box at (x,y) into b.
** Returns 1 if successful, 0 if unsuccessful.
*/
int
old_box(Box *b, int x, int y)
{
	Changed_box *cb;
	int i;

	/* Search through the changed_box array looking for the right location.  If
       there are duplicate entries, we want the earliest one, which is the
       first one we'll find. */
	for (i = 0; i < num_changed_boxes; i++) {
		cb = &changed_box[i];
		if (cb->x == (Byte) x && cb->y == (Byte) y) {
			*b = cb->box;
			return 1;
		}
	}
	return 0;
}
