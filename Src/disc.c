/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "xtank.h"
#include "loc.h"
#include "disc.h"
#include "bullet.h"
#include "cosell.h"
#include "graphics.h"
#include "globals.h"
#include "terminal.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /* SOUND */

extern Settings settings;

/* local history for discs */
static Vehicle *cur_owners[MAX_TEAMS];	/* who has the disc */
static Vehicle *last_owners[MAX_TEAMS];	/* who had the disc */
static Vehicle *old_owners[MAX_TEAMS];	/* old time owner */
static int max_discs_out = 0;

/*
** Function to initialize the settings of the discs.
*/
void
disc_init_history(void)
{
	int i;

	for (i = 0; i < MAX_TEAMS; i++) {
		cur_owners[i] = (Vehicle *) NULL;
		last_owners[i] = (Vehicle *) NULL;
		old_owners[i] = (Vehicle *) NULL;
	}
	max_discs_out = 0;
}

/*
** Change the indicated disc owner... shifting history
*/
void
disc_new_owner(Bullet *b, Vehicle *vh1)
{
	int teamnum;

	if (!b)
		return;
	teamnum = get_disc_team(b);
	old_owners[teamnum] = last_owners[teamnum];
	last_owners[teamnum] = cur_owners[teamnum];
	cur_owners[teamnum] = vh1;
}

/*
** Return who currently owns the disc
*/
Vehicle *
disc_cur_owner(Bullet *b)
{
	return (cur_owners[get_disc_team(b)]);
}

/*
** Return who last owned the disc
*/
Vehicle *
disc_last_owner(Bullet *b)
{
	return (last_owners[get_disc_team(b)]);
}

/*
** Return who owned the disc a while ago.
*/
Vehicle *
disc_old_owner(Bullet *b)
{
	return (old_owners[get_disc_team(b)]);
}

/*
** Return the original team ownership of a disc
*/
int
get_disc_team(Bullet *b)
{
	if (!b) {
		if (max_discs_out > 1) {
			return (rnd(max_discs_out));
		}
		return (0);
	}
	return (b->life - 10);
}

/*
** Set the original team ownership of a disc
*/
void
set_disc_team(Bullet *b, int teamnum)
{
	if (!b)
		return;
	if (teamnum >= max_discs_out) {
		max_discs_out = teamnum + 1;
	}
	b->life = 10 + teamnum;
}

/*
** Sets the owner of the specified disc to the specified vehicle.
*/
void
set_disc_owner(Bullet *b, Vehicle *v)
{
	/* Take it away from previous owner */
	if (b->owner != (Vehicle *) NULL) {
		b->owner->num_discs--;
		if (b->thrower == -1)
			b->thrower = b->owner->color;
	} else {
		if (b->thrower == -1)
			b->thrower = -2;
	}

	/* Give it to new owner */
	/* display_bullets(OFF); */
#ifdef SOUND
	play_in_view(b->loc, DISC_NEW_OWNER_SOUND);
#endif /* SOUND */
	disc_new_owner(b, v);
	b->owner = v;
	/* display_bullets(OFF); */
	if (b->owner != (Vehicle *) NULL) {
		b->owner->num_discs++;
	}
}

/*
** Releases all discs owned by a vehicle.  Sets speed of discs to speed.
** Allows one update if delay is set.  This allows robots to throw to
** an accuracy of one frame.
*/
void
release_discs(Vehicle *v, double dspeed, Boolean delay)
{
	extern Bset *bset;
	Bullet *b;
	FLOAT ratio;
	FLOAT curspeed;
	int i;

	if (v->num_discs > 0) {
		dspeed *= (0.5 + 1.3 * settings.si.disc_speed);
		for (i = 0; i < bset->number; i++) {
			b = bset->list[i];
			if (b->type == DISC && b->owner == v) {
#ifdef SOUND
				play_in_view(b->loc, DISC_SOUND);
#endif /* SOUND */
				if (delay)
					update_disc(b);
				curspeed = sqrt(b->xspeed * b->xspeed + b->yspeed * b->yspeed);
				ratio = dspeed / curspeed;
				b->xspeed *= ratio;
				b->yspeed *= ratio;
				/* display_bullets(ON); */
				b->owner = (Vehicle *) NULL;
				if (b->thrower == -1) {
					b->thrower = v->color;
				}
				disc_new_owner(b, (Vehicle *) NULL);
				/* display_bullets(ON); */

				/* Inform the commentator about the throw */
				if (settings.commentator)
					comment(COS_OWNER_CHANGE, COS_IGNORE, (Vehicle *) NULL,
							(Vehicle *) NULL, b);
			}
		}
		v->num_discs = 0;
	}
}

/*
** Makes all discs owned by the vehicle spin in the specified direction.
*/
void
set_disc_orbit(Vehicle *v, Spin dir)
{
	switch (dir) {
	  case CLOCKWISE:
		  v->status &= ~VS_disc_spin;
		  break;
	  case COUNTERCLOCKWISE:
		  v->status |= VS_disc_spin;
		  break;
	  case TOGGLE:
		  v->status ^= VS_disc_spin;
		  break;
	  default:
		  /* silence compiler warning about unhandled cases */
		  break;
	}
}
