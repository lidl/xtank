/**********************************\
* dizzy.c - A simple XTank player. *
\**********************************/

#include <xtanklib.h>
#include <math.h>


static void main();		/* forward reference */

Prog_desc dizzy_prog = {
    "dizzy",
    "dizzy",			/* or dizzy2 */
    "Runs around in circles.",
    "Robert Potter",
    PLAYS_COMBAT | DOES_SHOOT,
    3,
    main
};


/* all my global data */
typedef struct {
    Location myloc;		/* where I am */
    Boolean have_target;	/* whether I have someone to attack */
    Vehicle_info target;	/* who I'm going to attack */
    int gun_range;		/* range of my gun */
} All;


static void look_around(allp)
    All *allp;
{
    get_location(&allp->myloc);	/* find out where I am */
    allp->have_target = get_closest_enemy(&allp->target);
}


static void do_something(allp)
    All *allp;
{
    /* If a bad guy's around, attack him */
    if (allp->have_target) {
	int dx, dy;
	double distance;

	/* Find out where he is with respect to me */
	dx = allp->target.loc.x - allp->myloc.x;
	dy = allp->target.loc.y - allp->myloc.y;
	distance = HYPOT(dx, dy);

	aim_all_turrets(dx, dy);
	if (distance <= allp->gun_range) {
	    fire_all_weapons();	/* blast him */
	}

	/* circle around him */
	set_rel_drive(9.0);
	turn_vehicle(ATAN2(dy, dx) +
		     FULL_CIRCLE * 0.23 *
		     (1 - (distance - allp->gun_range * 0.8) / 200));
    } else {
	set_rel_drive(0.0);	/* wait for next victim */
    }
}


static void cleanup(allp)
    All *allp;
{
    /* free all dynamically allocated memory */
    free((char *)allp);
}


static void give_up()
{
    /* I don't like my vehicle */
    printf("dizzy: giving up.\n");
    while (1) done();
}


static void main()
{
    All *allp;
    Weapon_info winfo;

    /* allocate the All structure dynamically, to save stack space */
    allp = (All *) calloc(1, sizeof(*allp));

    /* arrange for the All structure to get free()ed when I die */
    set_cleanup_func(cleanup, (void *) allp);

    if (get_weapon(0, &winfo) == BAD_VALUE) {
	give_up();
    }
    allp->gun_range = winfo.range;

    while (1) {
	look_around(allp);
	do_something(allp);
	done();
    }
}
