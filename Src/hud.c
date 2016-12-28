/*
** Xtank
**
** Copyright 1991, 1992, 1992 by Aaron Nabil-Eastlund
**
** $Id$
*/

#include "xtank.h"
#include "xtanklib.h"
#include "vehicle.h"
#include "graphics.h"
#include "globals.h"
#include "gr.h"
#include "vstructs.h"
#include "bullet.h"
#include "terminal.h"
#include "proto.h"

#ifndef NO_HUD

extern int frame;
extern Weapon_stat weapon_stat[];
extern Settings settings;

#define norm_angle(angle) (angle - (2*PI) * floor(angle/(2*PI)))

#define BBOX 100

void
draw_armor(int armor, Angle ang)
{
	int start;

	switch (armor) {
	  case FRONT:
		  start = 45;
		  break;
	  case BACK:
		  start = 45 + 180;
		  break;
	  case LEFT:
		  start = 45 + 90;
		  break;
	  case RIGHT:
		  start = 45 + 90 + 180;
		  break;
	}

	start -= (ang / (2 * PI)) * 360 + 90;

	XDrawArc(vid->dpy, vid->win[ANIM_WIN].id,
			 vid->graph_gc[DRAW_XOR][RED],
	  (ANIM_WIN_WIDTH / 2) - (BBOX / 2), (ANIM_WIN_HEIGHT / 2) - (BBOX / 2),
			 BBOX, BBOX, (start) * 64, (90) * 64);
}

/*
 *
 */

SpecialStatus
special_hud(Vehicle *v, char *record, int action)
{
	Hud *h;

	int i;
	Boolean anyonline;
	Boolean wstatus;
	Boolean vchanged;
	Boolean rotated;
	Boolean turreted;
	int middle, start;

	h = (Hud *) record;

	switch (action) {

	  case SP_update:

		  wstatus = FALSE;
		  anyonline = FALSE;
		  vchanged = FALSE;
		  rotated = FALSE;
		  turreted = FALSE;

		  if (v->vector.heading != v->vector.old_heading)
			  rotated = TRUE;

		  if (rotated)
			  h->need_redisplay_arm = TRUE;

		  for (i = FRONT; i < MAX_SIDES; i++) {
			  if (h->armor_draw[i]
				  != ((v->armor.side[i] * 5 <= v->vdesc->armor.side[i])
					  && v->vdesc->armor.side[i])) {
				  h->armor_draw[i]
					= ((v->armor.side[i] * 5 <= v->vdesc->armor.side[i])
					   && v->vdesc->armor.side[i]);
				  if (i < TOP)
					  h->need_redisplay_arm = TRUE;
			  }
		  }

		  if (h->need_redisplay_arm)
			  h->draw_arm_angle = norm_angle(v->vector.heading);

		  /*
	     * save old speed vector if we need it, and check
	     * if it has changed
	     */

		  if (settings.si.rel_shoot
			  && (h->old_xspeed != (int) v->vector.xspeed
				  || h->old_yspeed != (int) v->vector.yspeed)) {
			  h->old_xspeed = (int) v->vector.xspeed;
			  h->old_yspeed = (int) v->vector.yspeed;
			  vchanged = TRUE;
		  }
		  /*
	     * set wstatus if any of the weapons have
	     * changed status
	     *
	     * set a flag if any weapon is online
	     */

		  for (i = 0; i < v->num_weapons; i++) {
			  if (h->saved_status[i] != v->weapon[i].status) {
				  h->saved_status[i] = v->weapon[i].status;
				  wstatus = TRUE;
			  }
			  if ((v->weapon[i].status & WS_on)
				  && (v->weapon[i].status & WS_func)
				  && !(v->weapon[i].status & WS_no_ammo)) {
				  anyonline = TRUE;
			  }
		  }

		  /*
	     * If we lost an update or we just turned
	     * on rescan weapons
	     */

		  if (h->frame_updated + 1 != frame)
			  wstatus = TRUE;


		  /*
	     * if no weapons are online and they didn't just
	     * go offline, return early.
	     */

		  if (!anyonline && !wstatus) {
			  h->frame_updated = frame;
			  return SP_on;
		  }
		  /*
	     * If the weapon statuses are all the same, and
	     * our speed is the same, and we haven't
	     * spun, check
	     * if all of the turrets are
	     * the same, if they are return early
	     *
	     */

		  for (i = 0; i < v->num_turrets; i++) {
			  if (v->turret[i].angle != h->turret[i].old_angle) {
				  h->turret[i].old_angle = v->turret[i].angle;
				  turreted = TRUE;
			  }
		  }

		  if (!wstatus && !vchanged && !rotated && !turreted) {
			  h->frame_updated = frame;
			  return SP_on;
		  }
		  /*
	     * If a weapon status changed, search weapons again
	     * for longest range of on-line weapon per mount
	     *
	     * Exclude a few useless weapons
	     *
	     * compute range explictly
	     *
	     * Code does not take relative velocity
	     * into account.
	     *
	     */

		  if (wstatus) {

			  for (i = 0; i < NUM_MOUNTS; i++)
				  h->draw[i].weapon = -1;

			  for (i = 0; i < v->num_weapons; i++) {
				  if (!(weapon_stat[v->weapon[i].type].disp_flgs & F_NOHD)
					  && (v->weapon[i].status & WS_on)
					  && (v->weapon[i].status & WS_func)
					  && !(v->weapon[i].status & WS_no_ammo)
					  && (h->draw[v->weapon[i].mount].weapon == -1
					  || (weapon_stat[v->weapon[i].type].frames
							  * weapon_stat[v->weapon[i].type].ammo_speed) >
						  (weapon_stat[v->weapon[h->draw[v->weapon[i].mount].weapon].type].ammo_speed
						   * weapon_stat[v->weapon[h->draw[v->weapon[i].mount].weapon].type].frames))) {
					  h->draw[v->weapon[i].mount].weapon = i;
				  }
			  }
		  }
		  /*
	     * if we have gotten this far, either a weapon status
	     * changed, or a turret turreted or the vehicle turned
	     *
	     * loop thru all the possible mounts, if the mount has an
	     * active weapon set the line coords/color
	     */

		  for (i = 0; i < NUM_MOUNTS; i++) {
			  if (h->draw[i].weapon != -1) {

				  int x1, y1, x2, y2;
				  int color;
				  Angle a;
				  Coord *cp;

				  x1 = (SCREEN_WIDTH / 2);
				  y1 = (SCREEN_HEIGHT / 2);

				  switch (v->weapon[h->draw[i].weapon].mount) {
					case MOUNT_TURRET1:
					case MOUNT_TURRET2:
					case MOUNT_TURRET3:
					case MOUNT_TURRET4:
						a = norm_angle(v->turret[v->weapon[h->draw[i].weapon].mount].angle);
						cp = &(v->obj->picinfo[v->vector.rot].turret_coord[v->weapon[h->draw[i].weapon].mount]);
						x1 += cp->x;
						y1 += cp->y;
						break;
					case MOUNT_FRONT:
						a = norm_angle(v->vector.heading);
						break;
					case MOUNT_BACK:
						a = norm_angle(v->vector.heading) + HALF_CIRCLE;
						break;
					case MOUNT_LEFT:
						a = norm_angle(v->vector.heading) - QUAR_CIRCLE;
						break;
					case MOUNT_RIGHT:
						a = norm_angle(v->vector.heading) + QUAR_CIRCLE;
						break;
				  }

				  switch (weapon_stat[v->weapon[h->draw[i].weapon].type].disp_flgs & F_CM) {
					case F_BL:
						color = BLUE;
						break;
					case F_RE:
						color = RED;
						break;
					case F_OR:
						color = ORANGE;
						break;
					case F_YE:
						color = YELLOW;
						break;
					case F_GR:
						color = GREEN;
						break;
					case F_VI:
						color = VIOLET;
						break;
					default:
						color = GREY;
						break;
				  }

				  x2 = x1 + (int) (weapon_stat[v->weapon[h->draw[i].weapon].type].range * COS(a));
				  y2 = y1 + (int) (weapon_stat[v->weapon[h->draw[i].weapon].type].range * SIN(a));
				  if (!(weapon_stat[v->weapon[h->draw[i].weapon].type].creat_flgs & F_NREL)
					&& settings.si.rel_shoot) {
					  x2 += (int) v->vector.xspeed * weapon_stat[v->weapon[h->draw[i].weapon].type].frames;
					  y2 += (int) v->vector.yspeed * weapon_stat[v->weapon[h->draw[i].weapon].type].frames;
				  }
				  h->draw[i].x1 = x1;
				  h->draw[i].y1 = y1;
				  h->draw[i].x2 = x2;
				  h->draw[i].y2 = y2;
				  h->draw[i].color = color;
			  }
		  }

		  h->need_redisplay_weap = TRUE;
		  h->frame_updated = frame;
		  return SP_on;
		  break;


	  case SP_redisplay:
		  if (h->need_redisplay_arm) {
			  for (i = FRONT; i < TOP; i++) {
				  if (h->armor_drawn[i]) {
					  draw_armor(i, h->drawn_arm_angle);
					  h->armor_drawn[i] = FALSE;
				  }
				  if (h->armor_draw[i]) {
					  draw_armor(i, h->draw_arm_angle);
					  h->armor_drawn[i] = TRUE;
				  }
			  }
			  h->drawn_arm_angle = h->draw_arm_angle;
			  h->need_redisplay_arm = FALSE;
		  }
		  if (h->need_redisplay_weap) {
			  for (i = 0; i < NUM_MOUNTS; i++) {
				  if (h->drawn[i].weapon != -1) {
					  draw_line(ANIM_WIN, h->drawn[i].x1, h->drawn[i].y1,
								h->drawn[i].x2, h->drawn[i].y2, DRAW_XOR,
								h->drawn[i].color);
					  h->drawn[i].weapon = -1;
				  }
				  if (h->draw[i].weapon != -1) {
					  draw_line(ANIM_WIN, h->draw[i].x1, h->draw[i].y1,
								h->draw[i].x2, h->draw[i].y2, DRAW_XOR,
								h->draw[i].color);
					  h->drawn[i] = h->draw[i];
				  }
			  }
			  h->need_redisplay_weap = FALSE;
		  }
#define BSIZE 50
#define TSIZE 20
		  if (h->armor_draw[TOP] != h->armor_drawn[TOP]) {
			  draw_filled_square(ANIM_WIN, (ANIM_WIN_WIDTH / 2) - (TSIZE / 2),
				 (ANIM_WIN_HEIGHT / 2) - (TSIZE / 2), TSIZE, DRAW_XOR, RED);
			  h->armor_drawn[TOP] = h->armor_draw[TOP];
		  }
		  if (h->armor_draw[BOTTOM] != h->armor_drawn[BOTTOM]) {
			  draw_square(ANIM_WIN, (ANIM_WIN_WIDTH / 2) - (BSIZE / 2),
				 (ANIM_WIN_HEIGHT / 2) - (BSIZE / 2), BSIZE, DRAW_XOR, RED);
			  h->armor_drawn[BOTTOM] = h->armor_draw[BOTTOM];
		  }
		  return SP_on;
		  break;

	  case SP_draw:
	  case SP_erase:
		  if (h->armor_drawn[TOP]) {
			  draw_filled_square(ANIM_WIN, (ANIM_WIN_WIDTH / 2) - (TSIZE / 2),
				 (ANIM_WIN_HEIGHT / 2) - (TSIZE / 2), TSIZE, DRAW_XOR, RED);
			  if (action == SP_erase)
				  h->armor_drawn[TOP] = FALSE;
		  }
		  if (h->armor_drawn[BOTTOM]) {
			  draw_square(ANIM_WIN, (ANIM_WIN_WIDTH / 2) - (BSIZE / 2),
				 (ANIM_WIN_HEIGHT / 2) - (BSIZE / 2), BSIZE, DRAW_XOR, RED);
			  if (action == SP_erase)
				  h->armor_drawn[BOTTOM] = FALSE;
		  }
		  for (i = FRONT; i < TOP; i++) {
			  if (h->armor_drawn[i]) {
				  draw_armor(i, h->drawn_arm_angle);
				  if (action == SP_erase)
					  h->armor_drawn[i] = FALSE;
			  }
		  }
		  for (i = 0; i < NUM_MOUNTS; i++) {
			  if (h->drawn[i].weapon != -1) {
				  draw_line(ANIM_WIN, h->drawn[i].x1, h->drawn[i].y1,
							h->drawn[i].x2, h->drawn[i].y2, DRAW_XOR,
							h->drawn[i].color);
				  if (action == SP_erase)
					  h->drawn[i].weapon = -1;
			  }
		  }
		  return SP_on;
		  break;

	  case SP_activate:
		  h->need_redisplay_weap = FALSE;
		  h->need_redisplay_arm = FALSE;
		  h->frame_updated = -2;
		  for (i = 0; i < NUM_MOUNTS; i++) {
			  h->draw[i].weapon = -1;
			  h->drawn[i].weapon = -1;
		  }
		  for (i = FRONT; i < MAX_SIDES; i++) {
			  h->armor_draw[i] = FALSE;
			  h->armor_drawn[i] = FALSE;
		  }
		  return SP_on;
		  break;
	}
	return SP_on;
}


#endif /* !NO_HUD */
