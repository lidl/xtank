/*****************************************************************************\
* spider.c - An example XTank player.					      *
* 									      *
* Note the use of the All structure to simulate global variables, and the use *
* of set_cleanup_func().						      *
\*****************************************************************************/

#include <xtanklib.h>
#include <math.h>

void spider_main();		/* forward reference */

Prog_desc spider_prog = {
    "spider",
    "Red_tie",
    "Waits for an enemy to appear, then charges in guns blazing.",
    "Robert Potter (after Terry Donahue)",
    PLAYS_COMBAT | DOES_SHOOT,
    2,
    spider_main
};

/* all my global data */
typedef struct {
    Location myloc;		/* where I am */
    Boolean have_target;	/* whether I have someone to attack */
    Vehicle_info target;	/* who I'm going to attack */
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

	/* Find out where he is with respect to me */
	dx = allp->target.loc.x - allp->myloc.x;
	dy = allp->target.loc.y - allp->myloc.y;

	aim_all_turrets(dx, dy);
	fire_all_weapons();	/* blast him */

	turn_vehicle(ATAN2(dy, dx));
	set_rel_drive(9.0);	/* give chase */
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

static void spider_main()
{
    All *allp;

    /* allocate the All structure dynamically, to save stack space */
    allp = (All *) calloc(1, sizeof(*allp));

    /* arrange for the All structure to get free()ed when I die */
    set_cleanup_func(cleanup, (void *) allp);

    while (1) {
	look_around(allp);
	do_something(allp);
	done();
    }
}
