/* vehicle.c - part of XTank */

#include "xtank.h"
#include "xtanklib.h"
#include "vehicle.h"
#include "globals.h"
#include "thread.h"
#include "terminal.h"
#include "disc.h"
#include "vstructs.h"
#include "assert.h"


extern int team_color_bright[];
extern char team_char[];
extern Map real_map;
extern Settings settings;


/* create turrets for the given vehicle */

void make_turrets(v)
    Vehicle *v;
{
    extern Object *turret_obj[];
    Turret *t;
    Weapon *w;
    int turret_weight;
    int i, j;

    for (i = 0; i < v->num_turrets; i++) {
	t = &v->turret[i];

	turret_weight = TURRET_WEIGHT;

	/* Add to that weight, the weights of all weapons mounted on it */
	if (v->num_weapons > 0) {
	    for (j = 0; j < v->num_weapons; j++) {
		w = &v->weapon[j];
		if ((int) w->mount == i) {
		    turret_weight += weapon_stat[(int) w->type].weight;
		}
	    }
	}
	/* Make the turret turn rate inversely proportional to its weight */
	t->turn_rate = 600.0 / (float) turret_weight;

	t->obj = turret_obj[0];
    }
}


/* allocate the specials for a vehicle */

void make_specials(v, which)
    Vehicle *v;
    Flag which;
{
    extern int special_console(), special_mapper(), special_radar(),
        special_dummy(), special_repair();
    Special *s;
    int i;

    for (i = 0; i < MAX_SPECIALS; i++) {
	s = &v->special[i];

	/* make sure vehicle should have this special */
	if (! tstflag(which, (1 << i))) {
	    s->status = SP_nonexistent;
	    continue;
	}

	switch (i) {
	    case CONSOLE:
		s->proc = special_console;
		s->record = calloc((unsigned) 1, sizeof(Console));
		break;
	    case MAPPER:
		s->proc = special_mapper;
		s->record = calloc((unsigned) 1, sizeof(Mapper));
		break;
	    case RADAR:
		s->proc = special_radar;
		s->record = calloc((unsigned) 1, sizeof(Radar));
		break;
	    case REPAIR:
		s->proc = special_repair;
		s->record = calloc((unsigned) 1, 1);
		break;
	    default:
		s->proc = special_dummy;
		s->record = calloc((unsigned) 1, 1);
		break;
	}
    }
}


void unmake_specials(v)
    Vehicle *v;
{
    Special *s;
    int i;

    /* Free the special record pointers */
    for (i = 0; i < MAX_SPECIALS; i++) {
	s = &v->special[i];

	if (s->status == SP_nonexistent)
	    continue;

	free((char *) s->record);
    }
}


/* allocate a new vehicle */

Vehicle *make_vehicle(d, c)
    Vdesc *d;			/* what kind it should be */
    Combatant *c;		/* who's going to use it */
{
    extern Object *vehicle_obj[];
    static int turn_divider[MAX_SPEED] = {8,8,8,8,10,12,16,20,24,28,34,40,46,
					  52,60,68,78,88,100,114,130,146,162,
					  180,200};
    Vehicle *v;
    Weapon *w;
    int i;

    if (num_veh >= MAX_VEHICLES)
	return (Vehicle *) NULL;

    /* get next slot in array, and put into list of living vehicles */
    v = &actual_vehicles[num_veh++];

    bzero((char *) v, sizeof(*v));	/* start with a clean slate */

    v->vdesc = d;
    v->name = d->name;
    v->owner = c;
    v->number = c->number;
    v->flag = (VEHICLE_0 << v->number);
    v->team = c->team;
    v->color = team_color_bright[v->team];
    sprintf(v->disp, "%c%d %s", team_char[v->team], v->number, c->name);

    v->obj = vehicle_obj[d->body];
    v->num_turrets = v->obj->num_turrets;
    v->max_fuel = (float) engine_stat[d->engine].fuel_limit;

    v->num_weapons = d->num_weapons;

    for (i = 0; i < d->num_weapons; i++) {
	w = &v->weapon[i];
	w->type = d->weapon[i];
	w->mount = d->mount[i];
    }

    make_turrets(v);
    make_programs(v, c->num_programs, c->program);
    make_specials(v, d->specials);

    for (i = 0; i < MAX_SPECIALS; i++)
	v->special[i].status = (d->specials & (1 << i)) ? SP_off :
		SP_nonexistent;

    /* Turn rate for each speed */
    for (i = 0; i < MAX_SPEED; i++)
	v->turn_rate[i] = (float) d->handling / (float) turn_divider[i];

    /* Vehicle not restricted to making safe turns */
    v->safety = FALSE;

    return v;
}


/* the specified vehicle is dead forever, so deallocate it */

unmake_vehicle(v)
    Vehicle *v;
{
    int i;

    assert(v >= &actual_vehicles[0] && v < &actual_vehicles[num_veh]);

    /* Free the memory for the thread and buffer */
    for (i = 0; i < v->num_programs; i++) {
	assert(v->program[i].thread == NULL);
	if (v->program[i].thread_buf != NULL) {
	    free((char *) v->program[i].thread_buf);
	    v->program[i].thread_buf = NULL;
	}
    }

    unmake_specials(v);
}


/* bring the given vehicle (back) to life */

int activate_vehicle(v)
    Vehicle *v;
{
    Weapon *w;
    Combatant *c = v->owner;
    int i;

    setflag(v->status, VS_functioning);
    setflag(v->status, VS_is_alive);
    clrflag(v->status, VS_was_alive);

    live_vehicles[num_veh_alive++] = v;	/* add to living list */

    v->death_timer = 0;

    v->armor = v->vdesc->armor;
    v->fuel = v->max_fuel;
    v->heat = 0;
    v->num_discs = 0;

    for (i = 0; i < v->num_weapons; i++) {
	w = &v->weapon[i];
	w->status = (WS_on | WS_func);
	w->ammo = weapon_stat[(int) w->type].max_ammo;
	w->reload_counter = 0;
    }

    /* Set up each player's terminal */
    for (i = 0; i < c->num_players; i++)
	setup_terminal(c->player[i], v);

    /* Place vehicle in maze, must be done *before* initializing specials */
    if (place_vehicle(v) == -1) {
	return GAME_FAILED;
    }
    init_specials(v);
    init_messages(v);
    init_turrets(v);
    init_programs(v);

    init_vehicle_status(v);

    return GAME_RUNNING;
}


/* this does the internal bookkeeping to make the specified vehicle dead.  It
   doesn't actually get put on the dead list yet, we wait for the beginning of
   the next frame for that.  This gets called when a vehicle dies during the
   game, and also at the end of the game.  NOTE: this function must be
   idempotent: i.e you must be able to call it multiple times on the same
   vehicle without harm. */

void inactivate_vehicle(victim)
    Vehicle *victim;
{
    int i;

    clrflag(victim->status, VS_is_alive);

    /* kill the threads  */
    for (i = 0; i < victim->num_programs; i++) {
	Program *prog = &victim->program[i];

	if (prog->cleanup != NULL) {
	    /* call cleanup function with argument */
	    prog->cleanup(prog->cleanup_arg);
	    prog->cleanup = NULL;
	}
	if (prog->thread != NULL) {
	    thread_kill((Thread *) prog->thread);
	    prog->thread = NULL;
	}
    }

    /* Let the terminals know that they have no vehicle to track */
    for (i = 0; i < victim->owner->num_players; i++) {
	terminal[victim->owner->player[i]]->vehicle = (Vehicle *) NULL;
    }

    /* Remove victim's flag from the map */
    real_map[victim->old_loc->grid_x][victim->old_loc->grid_y].flags &=
	~victim->flag;
}


void explode_vehicle(victim)
    Vehicle *victim;
{
    extern float rnd_interval();
    Loc *loc = victim->loc;
    Vdesc *vdesc = victim->vdesc;
    int engine = vdesc->engine;
    int body = vdesc->body;
    int dam;
    int i;
    WeaponType wt;

    /* Make a tank explosion around the victim */
    explode_location(victim->loc, 1, EXP_TANK);

    /* blow up the body and its contents, producing shrapnel based on mass */
    for (dam = vdesc->weight / 500;
	    dam > 0;
	    dam -= weapon_stat[(int) wt].damage) {
	wt = (WeaponType) rnd((int) HCANNON + 1);	/* pick a bullet */
	make_bullet((Vehicle *) NULL, loc, wt,
		    (Angle) rnd_interval(-PI, PI));
    }
    /* maybe spill some lubricants */
    for (i = rnd(body_stat[body].size / 2 + 1); i-- > 0;)
	make_bullet((Vehicle *) NULL, loc, SLICK, rnd_interval(-PI, PI));

    /* blow up the ammo (some of it goes off) */
    for (i = victim->num_weapons; i-- > 0;) {
	Weapon *w;
	int j;

	w = &victim->weapon[i];
	for (j = rnd(w->ammo / 30 + 1); j-- > 0;)
	    make_bullet((Vehicle *) NULL, loc, w->type, rnd_interval(-PI, PI));
	/* perhaps we should have turned some SLICK into FLAME? */
    }

    /* blow up the fuel */
    switch (engine) {
	case 0: case 1: case 2: case 3:		/* electric */
	    /* battery acid (not dependent on how much "fuel" they had left) */
	    for (i = victim->max_fuel / 200; i-- > 0;)
		make_bullet((Vehicle *) NULL, loc, ACID,
			    rnd_interval(-PI, PI));
	    break;
	case 12:				/* fuel cell */
	case 8: case 9: case 10: case 11:	/* turbine */
	case 4: case 5: case 6: case 7:		/* combustion */
	    for (i = victim->fuel / 50; i-- > 0;)
		make_bullet((Vehicle *) NULL, loc, FLAME,
			    rnd_interval(-PI, PI));
	    break;
	default:
	    /* don't bother with fusion, the amount of deuterium it represents
	       is insignificant when burnt */
	    break;
    }

    /* blow up the engine (all right, so most of this is far-fetched) */
    switch (engine) {
	case 8: case 9: case 10: case 11:	/* turbine */
	    /* flying turbine blades */
	    for (i = engine_stat[engine].power / 200; i-- > 0;)
		make_bullet((Vehicle *) NULL, loc, HRIFLE,
			    rnd_interval(-PI, PI));
	    break;
	case 15:				/* fusion */
	    /* plasma */
	    for (i = 10; i-- > 0;)
		make_bullet((Vehicle *) NULL, loc, FLAME,
			    rnd_interval(-PI, PI));
	    break;
	case 13: case 14:			/* fission */
	    /* depleted uranium shrapnel */
	    for (i = engine_stat[engine].power / 300; i-- > 0;)
		make_bullet((Vehicle *) NULL, loc, HMG, rnd_interval(-PI, PI));
	    break;
    }
}


/* put the given terminal into "observer" mode */

void make_observer(trm)
    Terminal *trm;
{
    extern Terminal *term;
    Terminal *temp_term;

    trm->observer = True;
    trm->vehicle = NULL;

    temp_term = term;
    term = trm;			/* switch to this terminal temporarily */
    display_help(ON);		/* update the help window */
    term = temp_term;
}


/* kills the specified vehicle, complete with game effects.  This gets called
   as a result of vehicle damage. */

kill_vehicle(victim, killer)
    Vehicle *victim;
    Vehicle *killer;		/* can be NULL */
{
    extern float rnd_interval();

    if (!tstflag(victim->status, VS_is_alive)) {
	/* If vehicle isn't alive, don't kill it again.  I guess this happens
	   when two bullets hit simultaneously */
	return;
    }

    explode_vehicle(victim);

    /* Release at fast speed any discs the victim owned */
    release_discs(victim, DISC_FAST_SPEED, TRUE);

    victim->owner->deaths++;

    if (killer && !SAME_TEAM(killer, victim)) {
	int points = 1000.0 * victim->vdesc->cost / killer->vdesc->cost;

	killer->owner->score += points;
	killer->owner->money += points * 8;
	killer->owner->kills++;
    }
    /* Send out a message about the victim's death */
    send_death_message(victim, killer);

    /* check if we should let them resurrect */
    if (!settings.si.restart) {
	setflag(victim->status, VS_permanently_dead);
    } else {
	clrflag(victim->status, VS_permanently_dead);
	if (settings.si.pay_to_play) {
	    /* make 'em pay for a new tank */
	    victim->owner->money -= victim->vdesc->cost;
	    if (victim->owner->money < 0) {	/* can't afford it? */
		char str[32];

		sprintf(str, "%c is broke.", victim->number);
		compose_message(SENDER_DEAD, RECIPIENT_ALL, OP_TEXT,
				(Byte *) str);
		setflag(victim->status, VS_permanently_dead);
		all_terms(victim, make_observer);
	    }
	}
    }

    inactivate_vehicle(victim);
}
