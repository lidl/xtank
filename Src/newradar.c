/*
** Xtank
**
** Copyright 1988, 1989, 1990, 1991, 1992, 1993  by Aaron Nabil-Eastlund
**
** newradar.c
*/

/*
$Author: lidl $
$Id: newradar.c,v 1.1.1.1 1995/02/01 00:25:39 lidl Exp $
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "map.h"
#include "vehicle.h"
#include "globals.h"
#include "bullet.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif SOUND

extern Map real_map;

/*
 * The idea of the blip is somewhat perverted in that
 * a blip can be invisible, which means that it is out
 * of range. A lowlib function provides for copying
 * only visible blips into a robot readable blip
 * array.
 */

#define BLIP_SIZE 5				/* 7, 5, 3, 1 */
#define shft ((7 - BLIP_SIZE)/2)

#define THRESHOLD 0.00001

extern int frame;

/*
 * This is a combination of draw_number and draw_char
 * that reads out of the blip info instead of calculating from
 * the absolute vehicle position
 */

nr_draw_number(v, c)
Vehicle *v;
Coord *c;
{
	char buf[2];

	buf[0] = '0' + v->number;
	buf[1] = '\0';
	draw_text(MAP_WIN, c->x, c->y, buf, S_FONT, DRAW_XOR, v->color);
}

/* #define traceaction */

/*
 * note that if a draw follows an activate, there must not have been an
 * intervening redisplay, though an update is OK.
 */

SpecialStatus special_new_radar(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	int veh, veh2, x, y;
	newRadar *r;
	newBlip *b;
	Vehicle *bv, *tv;
	long dist_2;

	r = (newRadar *) record;

	switch (action) {

	  case SP_redisplay:
#ifdef traceaction
		  if (v->number == 0)
			  printf("snr: %i redisplay ", frame);
#endif
		  if (r->need_redisplay) {
			  nr_t_redisplay(r);
#ifdef traceaction
			  if (v->number == 0)
				  printf("-- redisplayed");
#endif
		  }
#ifdef traceaction
		  if (v->number == 0)
			  printf("\n");
#endif
		  break;

	  case SP_update:
#ifdef traceaction
		  if (v->number == 0)
			  printf("snr: %i update    ", frame);
#endif
		  if (frame - r->frame_updated >= RAD_UPDATE_INTERVAL) {
			  r->frame_updated = frame;
			  for (veh = 0; veh < num_veh_alive; veh++) {
				  bv = live_vehicles[veh];
				  b = &r->blip[bv->number];
				  b->draw_radar = FALSE;
				  if (bv == v)
					  continue;

				  /* sort of illegally scribbling on this, change in future */
				  b->draw_friend = v->have_IFF_key[bv->number] || (bv->team == v->team && v->team != NEUTRAL);

				  dist_2 = idist((long) bv->loc->x, (long) bv->loc->y,
								 (long) v->loc->x, (long) v->loc->y);
				  dist_2 *= dist_2;

				  if ((bv->rcs / dist_2) > THRESHOLD) {
					  b->draw_radar = TRUE;
					  b->draw_loc.x = grid2map(bv->loc->grid_x) + shft + MAP_BOX_SIZE / 4;
					  b->draw_loc.y = grid2map(bv->loc->grid_y) + shft + MAP_BOX_SIZE / 4;
					  b->draw_grid.x = bv->loc->grid_x;
					  b->draw_grid.y = bv->loc->grid_y;
#ifndef NO_CAMO
					  if (!b->draw_friend) {
						  if (v->loc->grid_x != bv->loc->grid_x
							  || v->loc->grid_y != bv->loc->grid_y) {
							  bv->illum[v->number].gx = v->loc->grid_x;
							  bv->illum[v->number].gy = v->loc->grid_y;
							  bv->illum[v->number].color = RED;
						  }
					  }
					  /* want him to be able to see emmiters from 2x illiuminated distance. */
				  } else if ((bv->normal_rcs / (dist_2 * .25)) > THRESHOLD) {
					  if (!b->draw_friend) {
						  if (v->loc->grid_x != bv->loc->grid_x
							  || v->loc->grid_y != bv->loc->grid_y) {
							  bv->illum[v->number].gx = v->loc->grid_x;
							  bv->illum[v->number].gy = v->loc->grid_y;
							  bv->illum[v->number].color = GREEN;
						  }
					  }
#endif /* !NO_CAMO */
				  }
			  }
			  r->need_redisplay = TRUE;
#ifdef traceaction
			  if (v->number == 0)
				  printf("-- passed");
#endif
		  }
#ifdef traceaction
		  if (v->number == 0)
			  printf("\n");
#endif
		  break;

	  case SP_draw:
	  case SP_erase:
#ifdef traceaction
		  if (v->number == 0)
			  printf("snr: %i draw/erase\n", frame);
#endif
		  for (veh = 0; veh < MAX_VEHICLES; veh++) {
			  b = &r->blip[veh];
			  bv = &actual_vehicles[veh];
			  if (b->drawn_radar)
				  if (b->drawn_friend)
					  nr_draw_number(bv, &b->drawn_loc);
				  else {
					  draw_filled_square(MAP_WIN, b->drawn_loc.x, b->drawn_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					  if (action == SP_erase)
						  r->map[b->drawn_grid.x][b->drawn_grid.y] = NULL;
				  }
			  if (action == SP_erase)
				  b->drawn_radar = FALSE;
		  }
		  break;

	  case SP_activate:
#ifdef traceaction
		  if (v->number == 0)
			  printf("snr: %i activate\n", frame);
#endif

		  if (v->special[(SpecialType) RADAR].status == SP_on)
			  return SP_off;

		  r->need_redisplay = TRUE;

		  r->frame_updated = frame;

		  for (veh = 0; veh < MAX_VEHICLES; veh++) {
			  b = &r->blip[veh];
			  b->drawn_radar = FALSE;
			  b->draw_radar = FALSE;
			  if (v->special[(SpecialType) TACLINK].status != SP_on) {
				  b->draw_tactical = FALSE;
				  b->drawn_tactical = FALSE;
			  }
		  }

		  if (v->special[(SpecialType) TACLINK].status != SP_on)
			  for (x = 0; x < GRID_WIDTH; x++)
				  for (y = 0; y < GRID_WIDTH; y++)
					  r->map[x][y] = NULL;
		  else if (v->special[(SpecialType) TACLINK].status == SP_on)
			  for (x = 0; x < GRID_WIDTH; x++)
				  for (y = 0; y < GRID_WIDTH; y++)
					  if (r->map[x][y] && !(r->map[x][y])->drawn_radar)
						  r->map[x][y] = NULL;
		  return SP_on;
		  break;

	  case SP_deactivate:
#ifdef traceaction
		  if (v->number == 0)
			  printf("snr: %i deactivate\n", frame);
#endif
		  for (veh = 0; veh < MAX_VEHICLES; veh++) {
			  b = &r->blip[veh];
			  b->draw_radar = FALSE;
		  }
		  break;

	  default:
		  break;
	}
}

special_taclink(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	int veh, veh2, x, y;
	newRadar *r;
	newBlip *b;
	Vehicle *bv, *tv;
	float dx, dy, dist_2;
	Taclink *t;
	Rdf *rdf;
	int i, vi;
	Trace *tr;
	Bullet *h;
	TacCoord *dr_h, *drn_h;

	t = (Taclink *) record;
	r = (newRadar *) (v->special[(SpecialType) NEW_RADAR].record);
	rdf = (Rdf *) (v->special[(SpecialType) RDF].record);

	switch (action) {

	  case SP_redisplay:
#ifdef traceaction
		  if (v->number == 0)
			  printf(" st: %i redisplay  ", frame);
#endif
		  for (i = 0; i < HARM_TRACKING_SLOTS; i++) {
			  dr_h = &t->draw_harm[i];
			  drn_h = &t->drawn_harm[i];
			  if (dr_h->color != drn_h->color
				  || dr_h->x != drn_h->x
				  || dr_h->y != drn_h->y) {
				  if (drn_h->color != -1) {
					  draw_filled_square(MAP_WIN, drn_h->x, drn_h->y, 3, DRAW_XOR, drn_h->color);
					  drn_h->color = -1;
				  }
				  if (dr_h->color != -1) {
					  draw_filled_square(MAP_WIN, dr_h->x, dr_h->y, 3, DRAW_XOR, dr_h->color);
					  t->drawn_harm[i] = t->draw_harm[i];
				  }
			  }
		  }

		  if (v->special[(SpecialType) RDF].status != SP_nonexistent) {
			  for (vi = 0; vi < MAX_VEHICLES; vi++) {
				  if (vi == v->number)
					  continue;
				  for (i = 0; i < MAX_VEHICLES; i++) {
					  tr = &rdf->trace[vi][i];
					  if (tr->is_drawn && (tr->to_draw == PERSIST || !tr->to_draw)) {
						  draw_line(MAP_WIN, grid2map(tr->drawn.start_x) + MAP_OFF,
									grid2map(tr->drawn.start_y) + MAP_OFF,
									grid2map(tr->drawn.end_x) + MAP_OFF,
									grid2map(tr->drawn.end_y) + MAP_OFF,
									DRAW_XOR, YELLOW);
						  tr->is_drawn = FALSE;
					  }
					  if (tr->to_draw == PERSIST) {
						  draw_line(MAP_WIN, grid2map(tr->draw.start_x) + MAP_OFF,
									grid2map(tr->draw.start_y) + MAP_OFF,
									grid2map(tr->draw.end_x) + MAP_OFF,
									grid2map(tr->draw.end_y) + MAP_OFF,
									DRAW_XOR, YELLOW);
						  tr->is_drawn = TRUE;
						  tr->drawn = tr->draw;
					  }
				  }
			  }
		  }
		  if (v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent) {
			  if (r->need_redisplay) {
				  nr_t_redisplay(r);
#ifdef traceaction
				  if (v->number == 0)
					  printf("-- redisplayed");
#endif
			  }
		  }
#ifdef traceaction
		  if (v->number == 0)
			  printf("\n");
#endif
		  break;

	  case SP_update:
#ifdef traceaction
		  if (v->number == 0)
			  printf(" st: %i update    ", frame);
#endif
		  /*
	     * Look thru harm tracking slots, if there is something in it
	     * copy it then mark it is availiable again, else zap the last
	     * copy as it must have blown up
	     */
		  for (i = 0; i < HARM_TRACKING_SLOTS; i++) {
			  dr_h = &t->draw_harm[i];
			  if (t->harm[i]) {
				  h = (Bullet *) t->harm[i];
				  dr_h->color = h->state;
				  dr_h->grid_x = h->loc->grid_x;
				  dr_h->grid_y = h->loc->grid_y;
				  dr_h->x = grid2map(h->loc->grid_x) + ((7 - 3) / 2) + MAP_BOX_SIZE / 4;
				  dr_h->y = grid2map(h->loc->grid_y) + ((7 - 3) / 2) + MAP_BOX_SIZE / 4;
				  t->harm[i] = NULL;
			  } else {
				  dr_h->color = -1;
			  }
		  }

		  if (v->special[(SpecialType) RDF].status != SP_nonexistent) {
			  for (vi = 0; vi < num_veh_alive; vi++) {
				  tv = live_vehicles[vi];
				  for (i = 0; i < MAX_VEHICLES; i++) {
					  if (tv == v)
						  continue;
					  if (rdf->trace[tv->number][i].to_draw)
						  --rdf->trace[tv->number][i].to_draw;
					  if (!tstflag(tv->status, VS_is_alive)
						  || tv->special[(SpecialType) RDF].status != SP_on
					   || tv->special[(SpecialType) TACLINK].status != SP_on
						  || !(v->have_IFF_key[tv->number]
							|| (tv->team == v->team && v->team != NEUTRAL)))
						  continue;
					  if (((Rdf *) (tv->special[(SpecialType) RDF].record))->trace[tv->number][i].to_draw == PERSIST) {
						  rdf->trace[tv->number][i].to_draw = PERSIST;
						  rdf->trace[tv->number][i].draw = ((Rdf *) (tv->special[(SpecialType) RDF].record))->trace[tv->number][i].draw;
						  /*
			    printf("got a tac rdf trace v: %i i: %i\n", tv->number, i);
			    */
					  }
				  }
			  }
		  }
		  if (v->special[(SpecialType) TACLINK].status == SP_on &&
			  v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent) {
			  if (frame - t->frame_updated >= TAC_UPDATE_INTERVAL) {
				  t->frame_updated = frame;
				  for (veh = 0; veh < num_veh_alive; veh++) {
					  bv = live_vehicles[veh];
					  b = &r->blip[bv->number];
					  b->draw_tactical = FALSE;

					  if (bv == v)
						  continue;

					  /* illegally scribbling on this too, make a local variable */
					  b->draw_friend = v->have_IFF_key[bv->number] || (bv->team == v->team && v->team != NEUTRAL);
					  /*
			 * My taclink is on (else I wouldn't be here)
			 * and his is on so mark him as a friend via taclink
			 */
					  if (b->draw_friend && bv->special[(SpecialType) TACLINK].status == SP_on)
						  b->draw_tactical = TRUE;
					  /*
			 * He wasn't a friend or his Taclink was off.
			 * If I have newRadar capability, look for newRadar info
			 * coming down the link and scan all of my IFF friends
			 * NR memory for this guy
			 */
					  else if (v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent
							   && v->special[(SpecialType) NEW_RADAR].status != SP_broken) {
						  /*
			     * Looks thru all of the vehicles (veh2) until
			     * I find one that is an IFF friend and has
			     * tac & nr running and sees this guy
			     */
						  for (veh2 = 0; (veh2 < num_veh_alive) && !b->draw_tactical; veh2++) {
							  tv = live_vehicles[veh2];
							  b->draw_tactical = (tstflag(tv->status, VS_is_alive) &&
												  (v->have_IFF_key[tv->number] || (tv->team == v->team && v->team != NEUTRAL)) &&
												  tv != v &&
												  tv->special[(SpecialType) TACLINK].status == SP_on &&
												  tv->special[(SpecialType) NEW_RADAR].status == SP_on &&
												  ((newRadar *) (tv->special[(SpecialType) NEW_RADAR].record))->blip[veh].draw_radar);
						  }
					  }
					  if (b->draw_tactical) {
						  /*
		     * clear the draw_radar flag if we have tactical data on it and it's moved
		     * from the radar position (ie, we can have more up-to-date tac info than
		     * our radar is displaying)
		     *
		     * code depends on updates all happening before any displays, as this modifies
		     * a blip that is as of yet undrawn, and isn't well tested.
		     *
		     * note that we have already updated friend even if it is a radar blip
		     *
		     * kindof silly only updating the draw_ stuff if it is different as it could
		     * be moved out of the tests cause if it's the same it won't matter anyway!
		     *
		     */
						  if (b->draw_radar) {
							  if (b->draw_grid.x != bv->loc->grid_x || b->draw_grid.y != bv->loc->grid_y) {
								  b->draw_radar = FALSE;
								  b->draw_loc.x = grid2map(bv->loc->grid_x) + shft + MAP_BOX_SIZE / 4;
								  b->draw_loc.y = grid2map(bv->loc->grid_y) + shft + MAP_BOX_SIZE / 4;
								  b->draw_grid.x = bv->loc->grid_x;
								  b->draw_grid.y = bv->loc->grid_y;
							  } else {
								  ;
							  }
						  } else {
							  b->draw_loc.x = grid2map(bv->loc->grid_x) + shft + MAP_BOX_SIZE / 4;
							  b->draw_loc.y = grid2map(bv->loc->grid_y) + shft + MAP_BOX_SIZE / 4;
							  b->draw_grid.x = bv->loc->grid_x;
							  b->draw_grid.y = bv->loc->grid_y;
						  }
					  }
				  }
				  r->need_redisplay = TRUE;
			  }
#ifdef traceaction
			  if (v->number == 0)
				  printf("-- passed");
#endif
		  }
#ifdef traceaction
		  if (v->number == 0)
			  printf("\n");
#endif
		  break;

		  /*
	 * Draw/undraw anything that is already on the screen
	 *
	 * if this is an erase, zap those objects too.
	 */

	  case SP_draw:
	  case SP_erase:
#ifdef traceaction
		  if (v->number == 0)
			  printf(" st: %i draw/erase\n", frame);
#endif
		  for (i = 0; i < HARM_TRACKING_SLOTS; i++) {
			  if (t->drawn_harm[i].color != -1) {
				  draw_filled_square(MAP_WIN, t->drawn_harm[i].x, t->drawn_harm[i].y, 3, DRAW_XOR, t->drawn_harm[i].color);
				  if (action == SP_erase)
					  t->drawn_harm[i].color = -1;
			  }
		  }

		  if (v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent) {
			  for (veh = 0; veh < MAX_VEHICLES; veh++) {
				  b = &r->blip[veh];
				  bv = &actual_vehicles[veh];
				  if (b->drawn_tactical)
					  if (b->drawn_friend)
						  nr_draw_number(bv, &b->drawn_loc);
					  else {
						  draw_square(MAP_WIN, b->drawn_loc.x, b->drawn_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
						  if (action = SP_erase)
							  r->map[b->drawn_grid.x][b->drawn_grid.y] = NULL;
					  }
				  if (action = SP_erase)
					  b->drawn_tactical = FALSE;
			  }
		  }
		  if (v->special[(SpecialType) RDF].status != SP_nonexistent) {
			  /*
		* Erase all the drawn tactical traces
		*/
			  for (vi = 0; vi < MAX_VEHICLES; vi++) {
				  if (vi == v->number)
					  continue;
				  for (i = 0; i < MAX_VEHICLES; i++) {
					  tr = &rdf->trace[vi][i];
					  if (tr->is_drawn) {
						  draw_line(MAP_WIN, grid2map(tr->drawn.start_x) + MAP_OFF,
									grid2map(tr->drawn.start_y) + MAP_OFF,
									grid2map(tr->drawn.end_x) + MAP_OFF,
									grid2map(tr->drawn.end_y) + MAP_OFF,
									DRAW_XOR, YELLOW);
						  if (action == SP_erase)
							  tr->is_drawn = FALSE;
					  }
				  }
			  }
		  }
		  break;

	  case SP_activate:
#ifdef traceaction
		  if (v->number == 0)
			  printf(" st: %i activate\n", frame);
#endif

		  for (i = 0; i < HARM_TRACKING_SLOTS; i++) {
			  t->draw_harm[i].color = -1;
			  t->drawn_harm[i].color = -1;
		  }

		  if (v->special[(SpecialType) RDF].status != SP_nonexistent) {
			  for (vi = 0; vi < MAX_VEHICLES; vi++) {
				  if (vi == v->number)
					  continue;
				  for (i = 0; i < MAX_VEHICLES; i++) {
					  rdf->trace[vi][i].is_drawn = FALSE;
					  rdf->trace[vi][i].to_draw = FALSE;
				  }
			  }
		  }
		  t->frame_updated = frame - TAC_UPDATE_INTERVAL;

		  /*
	     * clear out the tactical state flags
	     *
	     * and if my base special isn't running, init his too.
	     */

		  if (v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent) {
			  r->need_redisplay = TRUE;
			  for (veh = 0; veh < MAX_VEHICLES; veh++) {
				  b = &r->blip[veh];
				  b->drawn_tactical = FALSE;
				  b->draw_tactical = FALSE;
				  if (v->special[(SpecialType) NEW_RADAR].status != SP_on) {
					  b->draw_radar = FALSE;
					  b->drawn_radar = FALSE;
				  }
			  }
			  /*
		 * if my base special isn't running, initialize the
		 * usage map so I can use it.
		 *
		 * if it is already running, just initialize my bits.
		 *
		 * I expect him to do the same for me!
		 */
			  if (v->special[(SpecialType) NEW_RADAR].status != SP_on)
				  for (x = 0; x < GRID_WIDTH; x++)
					  for (y = 0; y < GRID_WIDTH; y++)
						  r->map[x][y] = NULL;
			  else if (v->special[(SpecialType) NEW_RADAR].status == SP_on)
				  for (x = 0; x < GRID_WIDTH; x++)
					  for (y = 0; y < GRID_WIDTH; y++)
						  if (r->map[x][y] && !(r->map[x][y])->drawn_radar)
							  r->map[x][y] = NULL;
		  }
		  return SP_on;

		  break;

	  case SP_deactivate:
#ifdef traceaction
		  if (v->number == 0)
			  printf(" st: %i deactivate\n", frame);
#endif
		  for (i = 0; i < HARM_TRACKING_SLOTS; i++) {
			  t->draw_harm[i].color = -1;
			  t->drawn_harm[i].color = -1;
		  }
		  if (v->special[(SpecialType) NEW_RADAR].status != SP_nonexistent) {
			  for (veh = 0; veh < MAX_VEHICLES; veh++) {
				  b = &r->blip[veh];
				  b->draw_tactical = FALSE;
			  }
		  }
		  break;

	  default:
		  break;
	}
}

nr_t_redisplay(r)
newRadar *r;
{
	int veh;
	newBlip *b;
	Vehicle *bv;

/*
 * erases anything drawn not scheduled to be drawn OR that is different
 *
 * note that draw_tactical can != drawn_tactical even though nothing has
 * changed if it is a radar blip too, as radar draws preempt tactical
 * draws
 */

	for (veh = 0; veh < MAX_VEHICLES; veh++) {
		b = &r->blip[veh];
		bv = &actual_vehicles[veh];
		if (b->draw_loc.x != b->drawn_loc.x
			|| b->draw_loc.y != b->drawn_loc.y
			|| b->draw_friend != b->drawn_friend
			|| b->draw_radar != b->drawn_radar
			|| ((b->draw_tactical != b->drawn_tactical) && !b->draw_radar))
			if (b->drawn_radar) {
				if (b->drawn_friend)
					nr_draw_number(bv, &b->drawn_loc);
				else {
					draw_filled_square(MAP_WIN, b->drawn_loc.x, b->drawn_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					r->map[b->drawn_grid.x][b->drawn_grid.y] = NULL;
				}
				b->drawn_radar = FALSE;
			} else if (b->drawn_tactical) {
				if (b->drawn_friend)
					nr_draw_number(bv, &b->drawn_loc);
				else {
					draw_square(MAP_WIN, b->drawn_loc.x, b->drawn_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					r->map[b->drawn_grid.x][b->drawn_grid.y] = NULL;
				}
				b->drawn_tactical = FALSE;
			}
	}

/*
 *    draws everything supposed to be drawn that's not drawn
 */

/*
 *    first, draw all of the radar blips
 *
 *    draw any blip to be drawn_radar and
 *    not drawn already (blips with new data had
 *           been erased above, and drawn is clear now)
 */

	for (veh = 0; veh < MAX_VEHICLES; veh++) {
		b = &r->blip[veh];
		bv = &actual_vehicles[veh];
		if (b->draw_radar && !b->drawn_radar) {
			if (b->draw_friend) {
				nr_draw_number(bv, &b->draw_loc);
				b->drawn_radar = TRUE;
			} else {
				/*
		 * Check to see if another blip is using this space
		 *   if not, grab it.
		 *   if a tactical blip is using it, GRAB IT ANYWAY! HA HA HA!
		 *      undraw it for him and clear his drawn flag
		 *   otherwise, don't draw it as it's in use.
		 */
				if (r->map[b->draw_grid.x][b->draw_grid.y] == NULL) {
					draw_filled_square(MAP_WIN, b->draw_loc.x, b->draw_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					r->map[b->draw_grid.x][b->draw_grid.y] = b;
					b->drawn_radar = TRUE;
				} else if ((r->map[b->draw_grid.x][b->draw_grid.y])->drawn_tactical) {
					(r->map[b->draw_grid.x][b->draw_grid.y])->drawn_tactical = FALSE;
					draw_square(MAP_WIN, (r->map[b->draw_grid.x][b->draw_grid.y])->draw_loc.x,
								(r->map[b->draw_grid.x][b->draw_grid.y])->draw_loc.y, BLIP_SIZE, DRAW_XOR,
								CUR_COLOR);
					draw_filled_square(MAP_WIN, b->draw_loc.x, b->draw_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					r->map[b->draw_grid.x][b->draw_grid.y] = b;
					b->drawn_radar = TRUE;
				}
			}
			if (b->drawn_radar) {
				b->drawn_loc = b->draw_loc;
				b->drawn_friend = b->draw_friend;
				b->drawn_grid = b->draw_grid;
			}
		}
	}


/*
 *    next, draw all of the tactical blips that
 *    weren't drawn as radar blips
 *
 *    Only draw a blip if
 *    1) it's marked for drawing tactical
 *    2) it's not already draw tactical
 *       (note, that this is cleared above if new data
 *        was put into the blip)
 *    3) it's drawn as a radar blip (from above)
 *    4) it's not GOING TO BE drawn as a radar blip
 */


	for (veh = 0; veh < MAX_VEHICLES; veh++) {
		b = &r->blip[veh];
		bv = &actual_vehicles[veh];
		if (b->draw_tactical && !b->drawn_tactical && !b->drawn_radar && !b->draw_radar) {
			if (b->draw_friend) {
				nr_draw_number(bv, &b->draw_loc);
				b->drawn_tactical = TRUE;
			} else {
				/*
		 * if nobodies using this square, grab it
		 */
				if (r->map[b->draw_grid.x][b->draw_grid.y] == NULL) {
					draw_square(MAP_WIN, b->draw_loc.x, b->draw_loc.y, BLIP_SIZE, DRAW_XOR, CUR_COLOR);
					r->map[b->draw_grid.x][b->draw_grid.y] = b;
					b->drawn_tactical = TRUE;
				}
			}
			if (b->drawn_tactical) {
				b->drawn_loc = b->draw_loc;
				b->drawn_friend = b->draw_friend;
				b->drawn_grid = b->draw_grid;
			}
		}
	}

	/* We've just redisplayed, set a flag to that effect */
	r->need_redisplay = FALSE;
}
