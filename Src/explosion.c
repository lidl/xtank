/* explosion.c - part of XTank */

/*
 * $Id: explosion.c,v 2.4 1991/12/10 03:41:44 lidl Exp $
 *
 * $Log: explosion.c,v $
 * Revision 2.4  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
*/

#include "sysdep.h"
#include "bullet.h"
#include "graphics.h"
#include "loc.h"


#define EXP_SPREAD 15


/* Creates an explosion of the specified type at the specified location.  */

make_explosion(loc, type)
    Loc *loc;
    unsigned int type;
{
    extern int num_terminals;
    extern Object *exp_obj[];
    extern Eset *eset;
    Exp *e;
    int tnum;

    if (eset->number >= MAX_EXPS)
	return;
    e = eset->list[eset->number++];
    e->x = (int) loc->x;
    e->y = (int) loc->y;
    e->z = (int) loc->z;
    for (tnum = 0; tnum < num_terminals; tnum++) {
	e->old_screen_x[tnum] = e->screen_x[tnum] = loc->screen_x[tnum];
	e->old_screen_y[tnum] = e->screen_y[tnum] = loc->screen_y[tnum];
    }
    e->obj = exp_obj[type];
    e->life = e->obj->num_pics + 1;
    e->color = CUR_COLOR;
}


/* Makes the given number of explosions of the given type around the
   location.  */

explode_location(loc, num, type)
    Loc *loc;
    int num;
    unsigned int type;
{
    Loc exp_loc;
    int exp_dx, exp_dy;
    int i;

    for (i = 0; i < num; i++) {
	exp_loc = *loc;
	exp_dx = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
	exp_dy = rnd(EXP_SPREAD << 1) - EXP_SPREAD;
	update_loc(&exp_loc, &exp_loc, exp_dx, exp_dy);
	make_explosion(&exp_loc, type);
    }
}
