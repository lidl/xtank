/*
** Xtank
**
** Copyright 1992 by William T. Katz
**
** $Id$
*/

/*
     Gnat: A vehicle and bullet-dodging terminator.
     Version 5.0: February 5, 1992  
     Only 140,000 bytes long.  Why?  Because the man won't give him 
      anymore space.

     By William T. Katz, <wk5w@virginia.edu>
     Univ. of Virginia Medical Scientist Training Program.
     E-mail on Gnat performance (good or bad) is always welcome.

     ---------------------------------------------------------------------

                       ** IMPORTANT NOTICE **
                       ----------------------

     Gnat is a VERY big robot.  Therefore, you will have to declare
     CODE_SIZE (see the -D defines in Makefiles) of at least
     0x20000 (the default) and possibly more depending on your machine.
     You should use an optimizing flag (-O) when you compile gnat (and
     even the rest of xtank), because if you don't, gnat.o becomes
     unbearably long.  Check the size of gnat.o and if it is below
     approximately 128000 bytes, you should be safe.

     Gnat 5.0 does not use the features such as the new radar and TacLink
     because (1) they are not always compiled, (2) there's no documentation,
     and (3) there still might be bugs.  When these features are fully
     available, Gnat should be able to remove some extraneous code.  This
     also applies to simple tasks such as knowing which bullet is yours.
     The bottom line is that dodging eats CPU like the Washington Redskins
     ate the Buffalo Bills (hog heaven).


                                General
                                -------

     Gnat is an intelligent tank controller which uses hit-and-run tactics.
     It classifies a tank as either gnat-like (fast, good traction,
     distributed armor) or ram-like (fast, specialized for ramming so
     heavy front armor).  Gnat operation is like a gnat which buzzes around
     and annoys you to death.  Rammer operation is terminator-like, ramming 
     you at very high speeds and launching powerful short-range ammo at you 
     when closing.  Tanks are considered ram-like if its front armor is more 
     than twice as heavy as its back armor. 

     An additional distinction is made concerning the range of weapons.
     Gnats which drive tanks with long-distance (i.e. range >= 480)
     weaponry are designated "mortar-capable".  Mortar-capable tanks
     look for enemy sitting on depots through radar.  If a Gnat in a
     mortar-capable tank spots this kind of sitting duck, it tries to
     scout a good firing position beyond visible range.  It then fires
     away at the enemy from long-range, assuming it is on the depot.  To
     see this in action, use a mortar-capable gnat-like vehicle (e.g. Gnat2-3)
     in the maze "simple" against a tagman.  Eventually, it will find a
     reloading tagman and strike from a distance, and either (1) kill it
     and/or (2) drain it of money if it's on an armor depot.


	                      Tank Design
                              -----------

     The ideal tank for Gnat 5.0 is a fast turreted tank with good handling
     and traction. If there are multiple turrets, it would be best if weapons
     of one type are mounted on the same turret.  Good combat tanks are
     usually good racing tanks as long as speed doesn't fall below 17.  Front
     mounted weapons are used for destructable walls, so non-turreted racers
     can be used.  And as discussed above, mortar-capable tanks are very
     effective in the hands of Gnat drivers.


                              Capabilities
			      ------------

     1) Plays both combat and race.  Currently, Gnats do not work as a team
         when racing.  Racing teams have alot of potential for slowing down
	 the crowd when a Gnat has the lead.  Gnats can dodge when racing
         and this may slow down a racer; it is assumed that races are
	 generally slug-fests so some effort is made to make it to the goal
	 in one piece.

     2) Has an extensive targeting system which takes into account ammo
         speed, past and present target speed, our speed (for relative
         shooting), friendly tanks, heat and distance.  Multiple targets
	 can be acquired if there are multiple turrets.  Does not really
	 aim non-turret mounted weapons by turning vehicles, but will
	 fire them if they are in enemy's direction.

     3) Explores the maze when there is nothing else to do.

     4) Dodges bullets and vehicles which can cause damage.  Can actually
        use bouncing off walls as part of an escape.  Number of enemy
	also taken into account.

     5) Seeks out and loads resources from depots liberally.

     6) Uses extensive teamwork:

         (a) Will eventually regroup as a pack.
         (b) Teammates (if operational) will aid hurt tanks.
	 (c) Depot information is passed among the team, and relayed
	      whenever a team member reinitializes.
         (d) Friendly fire usually doesn't occur, especially when Gnats
	      are sitting.
	 (e) Friendly fire prevention is particularly extensive with regard
	      to heat seeker firings.  Heat seekers are usually not fired
	      towards visible or non-visible (far away) hot friends.  
	      The hotter a friend is, the less likely Gnat will fire 
	      anywhere near that direction. If they have no heat, targeting 
	      proceeds as usual.
         (f) Other information (e.g. visible enemies, heat) is relayed.

	 Currently, Gnat-specific text messages are used pending release
	 of documentation for new TacLink and general messages.

     7) Fast paths to a destination are usually available immediately
         while the optimal path (which considers scrolls, slow downs, etc.)
         is determined incrementally.  Paths can take into account
	 "hot spots" of enemy activity, and these areas are avoided in
	 choosing paths to replenish armor and ammo.

     8) Mine detection and avoidance as an integral part of the dodge
         bullet and mine routine.

     9) Shoots at outposts.  Because outposts which are just visible may
          be killed yet the map not reflect this fact, Gnat scrambles
	  outpost selection for targeting (if there are more than one)
	  every 100 frames.

    10) Prioritizes sitting on depot when there is more than two Gnats
         waiting for a depot.  Also prevents deadlock of two opposing Gnats
	 by having them look for friends in their way and checking their
	 intended action.

                                 Problems
				 --------

     1) Gnat is a real CPU-hog, and as such, its performance is dependent
         on the processing power of the host(s) it's running on.  So Gnats
         may be ineffectual on an Amiga, for example.  The current robot
         was tested using two Sun Sparcstations (one for display, one for
         running the Xtank kernel).  Extensive dodging routines which
	 attempted to dodge "every" visible bullet has been reworked to
	 consider the top "n" threats where "n" is NumDBullets.  Faster
	 CPUs can use larger NumDBullets.

     2) The neighborhood surrounding outposts should be considered as danger 
         zones, not just the outpost square itself.  This needs to be added
	 to the optimal path planner. 

     3) Dodge routine assumes fairly fast bullets which traverse vehicle
         size in one frame, or a very fast moving vehicle.  In other words,
         damage assessment is off if either condition doesn't hold.

     4) Sometimes it shuffles between two squares in a rocking motion,
         back-and-forth.  Still haven't been able to figure this one out.
	 And it lowers Gnat performance considerably in team play since
	 it effectively removes important team members.
        

                                Assumptions
				-----------

     1) clear_path() only looks at x, y, grid_x, and grid_y components
	  of a location structure.

     2) You cannot be hit with your own bullet.

     3) Probably alot of other implicit assumptions exist.

*/

#include <stdio.h>
#include <math.h>
#include <xtanklib.h>
#include <vstructs.h>

/*#define DEBUG_GNAT*/

/* #define DynamicLoad */
#ifdef DynamicLoad
#define ProgDesc  gnat5_prog
#define ProgName  "Gnat5"
#else
#define ProgDesc  gnat_prog
#define ProgName  "Gnat"
#endif 

static void gnat_main();
static Settings_info settings; 

Prog_desc ProgDesc =
{
  ProgName,
  "Gnat",
  "Seek out and kill the enemy from a distance.",
  "William T. Katz",
  PLAYS_COMBAT | PLAYS_RACE | 
    DOES_SHOOT | DOES_EXPLORE | DOES_DODGE | DOES_REPLENISH | 
    USES_TEAMS | USES_MESSAGES,
  8,  /* So why is tagman a 4? */
  gnat_main
};

/*** File specific definitions. ***/

#define CenterXofBox(x) ((x) * BOX_WIDTH + BOX_WIDTH / 2)
#define CenterYofBox(y) ((y) * BOX_HEIGHT + BOX_HEIGHT / 2)
#define LOG(x)          (float) (log((double) (x)))
#define DegToRad(x)     (FULL_CIRCLE * (x) / 360.0)
#define Deg15ToRad(x)   (FULL_CIRCLE * (x) / 24.0 + FULL_CIRCLE / 48.0)
#define RadToDeg(x)     ((int) (fixed_angle((x)) * 360.0 / FULL_CIRCLE) % 360)
#define RadTo4Deg(x)    ((int) (fixed_angle((x)) * 90.0 / FULL_CIRCLE) % 90)
#define RadTo15Deg(x)   ((int) (fixed_angle((x)) * 24.0 / FULL_CIRCLE) % 24)
#define OnLandmark(v) \
  map_landmark((v)->map, (v)->us.loc.grid_x, (v)->us.loc.grid_y)
#define OnMaze(x,y)     ((x)>=0 && (x)<GRID_WIDTH && (y)>=0 && (y)<GRID_HEIGHT)
#define OnWall(x,s) \
  (((x).box_x < (s) && \
    map_wall(vars->map, WEST, (x).grid_x, (x).grid_y) == MAP_WALL) || \
   ((x).box_x > BOX_WIDTH - (s) && \
    map_wall(vars->map, EAST, (x).grid_x, (x).grid_y) == MAP_WALL) || \
   ((x).box_y < (s) && \
    map_wall(vars->map, NORTH, (x).grid_x, (x).grid_y) == MAP_WALL) || \
   ((x).box_y > BOX_HEIGHT - (s) && \
    map_wall(vars->map, SOUTH, (x).grid_x, (x).grid_y) == MAP_WALL))
#define TAN(y,x) \
  ((x) == 0 && (y) == 0 ? 0.0 : fixed_angle(atan2((double)(y),(double)(x))))
#define ADiff(x) \
  (ABS((x)) > HALF_CIRCLE ? FULL_CIRCLE - ABS(x) : ABS(x))
#define ClearPath(a,b) \
  (!OnMaze((a).grid_x,(a).grid_y) || !OnMaze((b).grid_x,(b).grid_y) ? False : \
   clear_path(&(a), &(b)))

#define InfiniteNumber    99999999   /* Bigger than Texas. */
#define BadBox            99999999

#define MessageKey        200  /* Hopefully unique number to start message
			          texts.  Used to screen non-Gnat msgs. */
#define CryptKey            1
#define MaxDeadTime       100  /* If we don't hear from a friend in this
				  long, he must be gone for good.   */

#define MaxFastMoves      400  /* Used for fast find path queue.    */
#define MaxOptMoves      2000  /* Used for optimal find path queue. */
#define IterForPathPlan  1000  /* Path plan iterations per frame.   */
#define MaxDepots          10  

#define IEti         15   /* Look this many frames in future for dodging. */
#define MaxISize    100   /* Number of vsize's a vehicle can travel in IEti. */
#define NumDBullets  12   /* Number of bullets to consider dodging. */

#define RadarRange  7     /* Radius of radar sweep. */

/*** Structures used in the program. ***/

typedef enum {

  NoAid = 0,
  Regroup,
  MustAid

} AidTypes;

typedef enum {

  FriendStatus = 0,
  CheckingIn,
  DepotInfo

} MessageTypes;

typedef enum {

  NotWorking = 0,
  Working,
  DoneMapping

} MappingActions;

typedef enum {
  
  Critical = 0,
  Unstable,
  Operational,
  Maximum

} Status;

typedef enum {

  Default = 0,
  Evading,    /* I'll hit the brakes and they'll fly right by. */
  Panic,      /* Mav ... Come in mav ... they're all over me!  */
  Avoiding,
  LastStand   /* Rambo with a very large gun: "ARRRGGGHHHH" */

} MoveTypes;

typedef enum {

  Thinking = 0,
  Exploring,
  IdleSeeking,
  Combat,
  Aiding,
  Shelling,
  Seeking,
  Needing,
  Loading,
  Racing

} ActionTypes;  

typedef enum {

  NoAction = 0,
  Explore,
  CheckFuel, 
  CheckAmmo,
  CheckArmor,
  SeekEnemy,
  DoCombat,
  Pursue,
  Scout,
  Mortar,
  AidFriend,
  SeekFuel,  
  SeekAmmo,
  SeekArmor,
  NeedFuel,  
  NeedAmmo,
  NeedArmor,
  Repair,
  LoadFuel,
  LoadAmmo,
  LoadArmor,
  Race

} Actions;

#ifdef DEBUG_GNAT
static char *move_desc[] = {

  "Def",
  "Eva",
  "Pan",
  "Avo",
  "LS"

};
static char *action_desc[] = {

  "N/A",
  "Exp",
  "CFuel",
  "CAmmo",
  "CArmor",
  "SEnemy",
  "Combat",
  "Pursue",
  "Scout",
  "Mortar",
  "Aid",
  "SFuel",
  "SAmmo",
  "SArmor",
  "NFuel",
  "NAmmo",
  "NArmor",
  "Repair",
  "LFuel",
  "LAmmo",
  "LArmor",
  "Racing"

};
#endif

static ActionTypes action_type[] = {

  Thinking,
  Exploring,
  IdleSeeking,
  IdleSeeking,
  IdleSeeking,
  Combat,
  Combat,
  Combat,
  Shelling,
  Shelling,
  Aiding,
  Seeking,
  Seeking,
  Seeking,
  Needing,
  Needing,
  Needing,
  Loading,
  Loading,
  Loading,
  Loading,
  Racing

};

typedef struct {

  ID id;
  int fnum;
  int vsize;
  Location loc;
  float vx, vy;
  float dvx, dvy;
  float vspeed;
  float rspeed;
  Actions action;
  Coord dest;
  Status armor_cond;
  Status ammo_cond;
  Status fuel_cond;
  Status condition;
  int visible_enemies;
  int visible_friends;
  int heat;
  Boolean mortar_capable;
  Boolean in_sight;

} FriendInfo;

typedef struct {

  ID id;
  int vsize;
  int fnum;
  int dist;
  Angle pos_angle;
  Location loc;
  float vx, vy;
  float dvx, dvy;
  float ddvx, ddvy;
  float vspeed;
  float rspeed;
  Boolean in_sight;
  Boolean los;

} EnemyInfo;

typedef struct {

  int dx, dy, dist;
  int damage;
  int frames;
  Boolean hits;

} IVectInfo;


/*** Global variables.  Declared in struct so it is local to this file. ***/

typedef struct {

  /* Armor variables. */

  int max_armor[MAX_SIDES];
  int min_armor;       /* Least protected side armor initial value.  */
  int min_side;        /* Current value of most poorly armored side. */
  int max_side;
  int worst_side;
  int best_side;
  int armor[MAX_SIDES];
  int taking_damage;   /* If it's positive, then we have history of hits. */
  int critical_sides;
  int protection;
  Boolean repair_top;      /* Top can quickly be repaired. */
  Status armor_cond;

  /* Weapon variables. */

  int num_weapons;
  int ammo[MAX_WEAPONS];
  Weapon_info weapons[MAX_WEAPONS];
  int weapon_type[MAX_WEAPONS];      /* Maps weapons into weapon types. */
  Boolean weapon_ok[MAX_WEAPONS];    /* Whether we allow in to be used. */
  Boolean weapon_on[MAX_WEAPONS];    /* Whether gun is on for loading.  */
  Boolean mortar_capable;            /* Best mortar tank has range >= 480. */
  Boolean have_ammo;

  int num_wptypes;                   /* Group by speed, range, and mount.  */
  int wptype_speed[MAX_WEAPONS];    
  int wptype_range[MAX_WEAPONS];     /* The range in frames, not distance. */
  int wptype_count[MAX_WEAPONS];     /* The current ammo for this wp type. */
  int wptype_total[MAX_WEAPONS];     /* The maximum ammo for this wp type. */
  MountLocation wptype_mount[MAX_WEAPONS];

  int range;                         /* Good distance to shoot from. */
  Status ammo_cond;
  int weapon_heat;

  /* Fuel variables. */

  int max_fuel;
  Status fuel_cond;

  /* Vehicle information. */

  float max_speed; /* How fast we can go.          */
  float drive;     /* How fast we're trying to go. */
  float accel;
  float decel;
  int vsize;
  Boolean hover;
  Boolean rammer;  /* Two vehicle types: gnat or rammer. */

  Vehicle_info us;
  float speed;
  Angle vel_angle;
  Vehicle_info old_us;
  float old_speed;
  Byte team_code;

  /* Other variables giving game information. */

  Actions action;
  Boolean ram_him;      /* Just what it says. */
  MoveTypes movement;
  int money;
  int start_time;

  Boolean pos_changed;
  Boolean signal_friends;   /* When we should send a message to friends. */
  Boolean goal_changed;
  Coord goal;

  /* Intercepting objects: Vehicles and bullets. */

  int num_ivects;
  int igrid[IEti][MaxISize][MaxISize];
  int mine_field[MaxISize][MaxISize];  /* Record of which mines were hit. */
  int west_x, east_x, north_y, south_y;
  int ix0, iy0, ix1, iy1;

  Bullet_info bullets[MAX_BULLETS];
  IVectInfo ivects[MAX_BULLETS];   /* More info on bullets. */
  int b_orig_fnum[MAX_BULLETS];    /* Starting frame by id. */
  int b_fnum[MAX_BULLETS];         /* When id was last seen. */
  Boolean b_ours[MAX_BULLETS];
  int num_bullets;
  Boolean seeker_on_tail;
  Boolean mine_ahead;
  int possible_damage;

  int num_tanks;
  Vehicle_info tanks[MAX_VEHICLES];

  int outposts;
  int enemy_los;
  int visible_enemies;
  int close_enemies;   /* "Close" are enemies seen within last 30 ticks. */
  int fnum_enemy_sighting;
  Boolean withdraw;    /* Better "run" rather than "hit". */

  EnemyInfo enemies[MAX_VEHICLES];
  int num_enemies;
  int victim;               /* Current targeted victim.  Receives priority. */
  int closest_enemy;        /* The distance to closest enemy with los. */
  int default_enemy;
  Boolean boogie_too_fast;

  Location mortar_target;   /* Long-range target location. */
  Angle mortar_angle;
  int mortar_dist;

  /* Friend and long-distance target information. */

  FriendInfo friends[MAX_VEHICLES];
  int num_friends;
  int visible_friends;
  int active_friends;
  int last_friend_update;
  int vtof[MAX_VEHICLES];   /* Mapping of vehicle # to friend #. */
  int hot_spot[GRID_WIDTH][GRID_HEIGHT];
  Boolean close_friends;    /* Friends within one box distance. */

  int enemy_dist[24];
  int friend_dist[90];        /* Quick checking for friendly fire. */

  Blip_info blips[MAX_BLIPS];
  int blip_updated[MAX_BLIPS];
  int num_blips;

  /* Map and landmark information. */

  Box (*map)[GRID_HEIGHT];
  Boolean seen_box[GRID_WIDTH][GRID_HEIGHT];
  int unmapped_boxes;

  Coord armor_depots[MaxDepots];
  Boolean new_armor[MaxDepots];
  int num_armor_depots;
  int last_visited_armor[MaxDepots];

  Coord ammo_depots[MaxDepots];
  Boolean new_ammo[MaxDepots];
  int num_ammo_depots;

  Coord fuel_depots[MaxDepots];
  Boolean new_fuel[MaxDepots];
  int num_fuel_depots;

  /* Path information. */

  Coord dest;
  int cur_grad;  /* Which buffer holds the real fast_gradient. */
  int (*fast_gradient)[GRID_HEIGHT];
  int gradient_buffer[2][GRID_WIDTH][GRID_HEIGHT];
  int fnum_path_computed;
  Boolean path_ok;
  unsigned char qx[MaxFastMoves], qy[MaxFastMoves];
  Angle move_heading;

  int opt_gradient[GRID_WIDTH][GRID_HEIGHT];
  MappingActions opt_path_planning;
  unsigned char oqx[MaxOptMoves], oqy[MaxOptMoves];
  int q_bottom, q_top;

  Boolean working_on_destwall;

  /* Variables which should be local, but are here to conserve stack space. */

  Boolean bool_map[GRID_WIDTH][GRID_HEIGHT];
  int old_path_fnum;
  Coord old_dest;
  Boolean had_old_path;

  int last_x[2], last_y[2];

} GnatVars;

static Coord dir_offset[] = {
  {  0, -1  },    /* Should correspond to offsets for WallSides. */
  {  1,  0  },
  {  0,  1  },
  { -1,  0  }
};

/* Debugging variables. */

static char text[80];

/*** Check on our armor condition. */

static void check_armor(vars)
  GnatVars *vars;
{
  int side, fnum, value;
  int crit_type;
  float critical_val, unstable_val;
  Boolean damaged;

  vars->armor_cond = Maximum;
  vars->critical_sides = 0;
  vars->min_side = 10000;
  vars->max_side = -1;

  damaged = False;
  critical_val = 0.3;
  unstable_val = 0.6;
  crit_type = 0;

  for (side = 0; side < MAX_SIDES; side++) {
    if (side == BOTTOM && vars->hover)  /* If we're hover. */
      continue;
    value = armor(side);
    if (value < vars->armor[side]) {
      if (side != TOP && side != BOTTOM)
	damaged = True;
    }
    vars->armor[side] = value;

    if (side != BOTTOM && side != TOP) {
      if (value < vars->min_side) {
	vars->min_side = value;
	vars->worst_side = side;
      }
      if (value > vars->max_side) {
	vars->max_side = value;
	vars->best_side = side;
      }
    }

    if (value < vars->max_armor[side] * critical_val) {
      if (side != TOP && side != BOTTOM) 
	vars->critical_sides++;
      vars->armor_cond = Critical;
      if (side == FRONT)
	crit_type += 2;
      else if (side == BACK)
	crit_type -= 2;
      else if (side == LEFT)
	crit_type += 1;
      else if (side == RIGHT)
	crit_type -= 1;
    }
    else if (vars->armor_cond > Unstable && 
	     value < vars->max_armor[side] * unstable_val)
      vars->armor_cond = Unstable;
    else if (vars->armor_cond == Maximum && value < vars->max_armor[side])
      vars->armor_cond = Operational;
  }

  if (vars->armor[TOP] == vars->max_armor[TOP])
    vars->repair_top = False;
  else
    vars->repair_top = True;

  fnum = frame_number();
  if (damaged && vars->num_ivects)
    vars->taking_damage = fnum;
  else if (vars->taking_damage + 15 < fnum)
    vars->taking_damage = False;

  if (vars->critical_sides == 2 && crit_type == 0)
    vars->critical_sides = 3;
}

/*** Check on our ammunition condition by type. ***/

static void check_ammo(vars)
  GnatVars *vars;
{
  int wp;
  int value, total;

  total = 0;
  vars->range = 0;
  for (wp = 0; wp < vars->num_wptypes; wp++)
    vars->wptype_count[wp] = 0;

  /* Check on each weapon. */

  for (wp = 0; wp < vars->num_weapons; wp++) {
    value = weapon_ammo(wp);
    total += value;
    vars->ammo[wp] = value;
    vars->wptype_count[vars->weapon_type[wp]] += value;
    if (value > 0 && vars->weapons[wp].range > vars->range &&
	(vars->weapons[wp].heat + vars->weapon_heat < 100 ||
	 vars->min_side / (6 - vars->protection) < 25))
      vars->range = vars->weapons[wp].range;
  }

  /* Determine our condition. */

  vars->ammo_cond = Maximum;
  for (wp = 0; wp < vars->num_wptypes; wp++) {
    if (vars->wptype_count[wp] < MAX(1, vars->wptype_total[wp] / 10))
      vars->ammo_cond = Critical;
    else if (vars->ammo_cond > Unstable && 
	     vars->wptype_count[wp] < vars->wptype_total[wp] / 3)
      vars->ammo_cond = Unstable;
    else if (vars->ammo_cond > Operational && 
	     vars->wptype_count[wp] != vars->wptype_total[wp])
      vars->ammo_cond = Operational;
  }

  if (total == 0)
    vars->have_ammo = False;
  else
    vars->have_ammo = True;

  /* Establish a good distance to stay from enemy. */

  if (vars->range == 0)
    vars->range = 1500;    /* If no guns, stay FAR away if we can. */
  else {
    vars->range -= 100;
    vars->range = MIN(MAX(100, vars->range), 700);
  }
}

/*** Check on our fuel. ***/

static void check_fuel(vars)
  GnatVars *vars;
{
  int value;

  value = fuel();
  vars->fuel_cond = Maximum;
  if (value < vars->max_fuel * (has_special(REPAIR) ? 0.35 : 0.25))
    vars->fuel_cond = Critical;
  else if (value < vars->max_fuel * (has_special(REPAIR) ? 0.5 : 0.4))
    vars->fuel_cond = Unstable;
  else if (value < vars->max_fuel)
    vars->fuel_cond = Operational;
}

/*** Return True if we are moving backwards. ***/

static Boolean moving_backwards(us)
  Vehicle_info *us;
{
  float direction;

  direction = cos((double) (TAN(us->yspeed, us->xspeed) - us->heading));
  if (direction < 0.0)
    return(True);
  else
    return(False);
}

/*** Put the current path information in the old gradient buffer. ***/

static Boolean push_path(vars)
  GnatVars *vars;
{
  int i, j;

  if (!vars->path_ok)
    return(False);

  vars->had_old_path = vars->path_ok;
  vars->old_path_fnum = vars->fnum_path_computed;
  vars->old_dest = vars->dest;
  vars->cur_grad = (vars->cur_grad + 1) % 2;
  vars->fast_gradient = vars->gradient_buffer[vars->cur_grad];

  return(True);
}

/*** Pop the current path information in the old gradient buffer. ***/

static void pop_path(vars)
  GnatVars *vars;
{
  int i, j;

  vars->path_ok = vars->had_old_path;
  vars->fnum_path_computed = vars->old_path_fnum;
  vars->dest = vars->old_dest;
  vars->cur_grad = (vars->cur_grad + 1) % 2;
  vars->fast_gradient = vars->gradient_buffer[vars->cur_grad];
}

/*** Check if our vehicle can pass from point to point w/o hitting walls. ***/

static Boolean clear_tank_path(beg, end, size)
  Location *beg, *end;
  int size;
{
  Location b, e;

  b = *beg;
  e = *end;

  /* Go through each corner and check for clear path. */

  b.x -= (size + 1) / 2;
  b.y -= (size + 1) / 2;
  b.grid_x = b.x / BOX_WIDTH;
  b.grid_y = b.y / BOX_HEIGHT;
  e.x -= (size + 1) / 2;
  e.y -= (size + 1) / 2;
  e.grid_x = e.x / BOX_WIDTH;
  e.grid_y = e.y / BOX_HEIGHT;
  if (!ClearPath(b, e))
    return(False);

  b.x += size;
  e.x += size;
  b.grid_x = b.x / BOX_WIDTH;
  b.grid_y = b.y / BOX_HEIGHT;
  e.grid_x = e.x / BOX_WIDTH;
  e.grid_y = e.y / BOX_HEIGHT;
  if (!ClearPath(b, e))
    return(False);

  b.y += size;
  e.y += size;
  b.grid_x = b.x / BOX_WIDTH;
  b.grid_y = b.y / BOX_HEIGHT;
  e.grid_x = e.x / BOX_WIDTH;
  e.grid_y = e.y / BOX_HEIGHT;
  if (!ClearPath(b, e))
    return(False);

  b.x -= size;
  e.x -= size;
  b.grid_x = b.x / BOX_WIDTH;
  b.grid_y = b.y / BOX_HEIGHT;
  e.grid_x = e.x / BOX_WIDTH;
  e.grid_y = e.y / BOX_HEIGHT;
  if (!ClearPath(b, e))
    return(False);

  return(True);
}

/*** Weapon arming and firing procedures. ***/

static Boolean discharge_weapon(vars, wp)
  GnatVars *vars;
  int wp;
{
  Boolean fired;

  fired = False;
  if (!vars->weapon_on[wp]) {
    turn_on_weapon(wp);
    vars->weapon_on[wp] = True;
  }
  if (fire_weapon(wp) == FIRED)
    fired = True;

  return(fired);
}

/*** Toggle the armament to maximize reloading while readying for enemy. ***/

static void toggle_ammo(vars)
  GnatVars *vars;
{
  int wp;
  int max_ammo, max_num;

  if (vars->enemy_los) {         /* If enemy, keep some guns on. */
    max_ammo = -1;
    for (wp = 0; wp < vars->num_weapons; wp++)
      if (vars->ammo[wp] > max_ammo) {
	max_ammo = vars->ammo[wp];
	max_num = wp;
      }
    for (wp = 0; wp < vars->num_weapons; wp++)
      if (wp == max_num)
	vars->weapon_ok[wp] = True;
      else {
	if (vars->weapon_on[wp]) {
	  vars->weapon_on[wp] = False;
	  turn_off_weapon(wp);
	}
	vars->weapon_ok[wp] = False;
      }
  }
  else                            /* If no enemy, can afford all guns off. */
    for (wp = 0; wp < vars->num_weapons; wp++) {
      if (vars->weapon_on[wp]) {
	vars->weapon_on[wp] = False;
	turn_off_weapon(wp);
      }
      vars->weapon_ok[wp] = False;
    }
}

/*** Rethink our current action by making it N/A and resetting path. ***/

static void rethink(vars)
  GnatVars *vars;
{
  vars->action = NoAction;
  vars->path_ok = False;
  if (settings.game == RACE_GAME)
    vars->goal_changed = True;
}

/*** Update depot information. ***/

static Boolean update_depot_info(vars, type, x, y, discovered)
  GnatVars *vars;
  int type, x, y;
  Boolean discovered;   /* Not second hand info. */
{
  int i;
  Boolean new_info;

  new_info = True;
  switch (type) {
  case ARMOR:
    if (vars->num_armor_depots < MaxDepots) {
      for (i = 0; i < vars->num_armor_depots; i++)
	if (x == vars->armor_depots[i].x && y == vars->armor_depots[i].y) {
	  new_info = False;
	  break;
	}
      if (new_info) {
	i = vars->num_armor_depots++;
	vars->armor_depots[i].x = x;
	vars->armor_depots[i].y = y;
	vars->new_armor[i] = discovered;
	vars->last_visited_armor[i] = -100;

	if (vars->armor_cond < Operational && settings.game != RACE_GAME) {
#ifdef DEBUG_GNAT
	  send_msg(RECIPIENT_ALL, OP_TEXT, "New armor depot - rethink");
#endif
	  rethink(vars);
	}
      }
    }
    else
      new_info = False;
    break;
  case AMMO:
    if (vars->num_ammo_depots < MaxDepots) {
      for (i = 0; i < vars->num_ammo_depots; i++)
	if (x == vars->ammo_depots[i].x && y == vars->ammo_depots[i].y) {
	  new_info = False;
	  break;
	}
      if (new_info) {
	i = vars->num_ammo_depots++;
	vars->ammo_depots[i].x = x;
	vars->ammo_depots[i].y = y;
	vars->new_ammo[i] = discovered;

	if (vars->ammo_cond < Operational && settings.game != RACE_GAME) {
#ifdef DEBUG_GNAT
	  send_msg(RECIPIENT_ALL, OP_TEXT, "New ammo depot - rethink");
#endif
	  rethink(vars);
	}
      }
    }
    else
      new_info = False;
    break;
  case FUEL:
    if (vars->num_fuel_depots < MaxDepots) {
      for (i = 0; i < vars->num_fuel_depots; i++)
	if (x == vars->fuel_depots[i].x && y == vars->fuel_depots[i].y) {
	  new_info = False;
	  break;
	}
      if (new_info) {
	i = vars->num_fuel_depots++;
	vars->fuel_depots[i].x = x;
	vars->fuel_depots[i].y = y;
	vars->new_fuel[i] = discovered;

	if (vars->fuel_cond == Critical && settings.game != RACE_GAME) {
#ifdef DEBUG_GNAT
	  send_msg(RECIPIENT_ALL, OP_TEXT, "New fuel depot - rethink");
#endif
	  rethink(vars);
	}
      }
    }
    else
      new_info = False;
    break;
  default:
    new_info = False;
    break;
  }

  return(new_info);
}

/*** Update current map and landmark information. ***/

static void check_map(vars)
  GnatVars *vars;
{
  int x, y, dx, dy, min_dist, min_x, min_y;
  int type;
  Boolean new_landmark;

  min_x = -1;

  new_landmark = False;
  for (x = 0; x < GRID_WIDTH; x++)
    for (y = 0; y < GRID_HEIGHT; y++) {
      type = map_landmark(vars->map, x, y);
      if ((type == ARMOR || type == AMMO || type == FUEL) &&
	  update_depot_info(vars, type, x, y, True))
	new_landmark = True;
      if (type == GOAL) {
	dx = vars->us.loc.grid_x - x;
	dy = vars->us.loc.grid_y - y;
	if (min_x == -1 || min_dist > dx * dx + dy * dy) {
	  min_dist = dx * dx + dy * dy;
	  min_x = x;
	  min_y = y;
	}
      }
    }

  if (min_x != -1) {
    vars->goal_changed = False;
    if (min_x != vars->goal.x || min_y != vars->goal.y) {
      if (ABS(min_x - vars->goal.x) + ABS(min_y - vars->goal.y) > 10) {
	vars->goal_changed = True;
	vars->goal.x = min_x;
	vars->goal.y = min_y;
      }
    }
  }

  if (new_landmark) {
    vars->signal_friends = True;
    if (vars->action == Explore)
      rethink(vars);
  }
}

/*** Check the geography: Our position, map, etc. ***/

static void check_geography(vars)
  GnatVars *vars;
{
  int i, j, x, y;
  Location loc;
  Angle vel_angle;

  vars->old_us = vars->us;
  vars->old_speed = vars->speed;
  get_self(&(vars->us));
  vars->speed = speed();

  vel_angle = TAN(vars->us.yspeed, vars->us.xspeed);
  vars->vel_angle = vel_angle;

  if (vars->old_us.loc.grid_x != vars->us.loc.grid_x ||
      vars->old_us.loc.grid_y != vars->us.loc.grid_y) {
    vars->pos_changed = True;

    /* Are we rocking? Why it does this is currently unknown, but
       this is a kludge that might help to detect and fix it. */

    if (frame_number() > 100 && action_type[vars->action] != Combat &&
	vars->movement == Default && vars->path_ok &&
	vars->us.loc.grid_x == vars->last_x[0] &&
	vars->us.loc.grid_y == vars->last_y[0] &&
	vars->old_us.loc.grid_x == vars->last_x[1] &&
	vars->old_us.loc.grid_y == vars->last_y[1]) {
#ifdef DEBUG_GNAT
      send_msg(RECIPIENT_ALL, OP_TEXT, "Rocking.");
#endif DEBUG_GNAT
      rethink(vars);
    }

    vars->last_x[1] = vars->last_x[0];
    vars->last_y[1] = vars->last_y[0];
    vars->last_x[0] = vars->old_us.loc.grid_x;
    vars->last_y[0] = vars->old_us.loc.grid_y;

    if (vars->path_ok && vars->dest.x == vars->us.loc.grid_x &&
	vars->dest.y == vars->us.loc.grid_y)
      rethink(vars);
    check_map(vars);
    for (i = -1; i <= 1; i++)
      for (j = -1; j <= 1; j++) {
	x = vars->us.loc.grid_x + i;
	y = vars->us.loc.grid_y + j;
	if (OnMaze(x,y)) {
	  if (!settings.full_map && !vars->seen_box[x][y])
	    vars->unmapped_boxes--;
	  vars->seen_box[x][y]++;
	}
      }
  }
  else
    vars->pos_changed = False;

  vars->working_on_destwall = False;
}

/*** Update blip memory. ***/

static void check_radar(vars)
  GnatVars *vars;
{
  Blip_info blips[MAX_BLIPS];
  int blip_updated[MAX_BLIPS], num_blips;
  int i, j, dx, dy, fnum;
  Boolean got_it;

  /* Get current blips. */

  get_blips(&num_blips, blips);
  fnum = frame_number();
  for (i = 0; i < num_blips; i++)
    blip_updated[i] = fnum;
  
  /* Add unseen blips which are in short-term memory. */

  for (i = 0; i < vars->num_blips; i++)
    if (fnum - vars->blip_updated[i] <= 25) {
      got_it = False;
      for (j = 0; j < num_blips; j++)
	if (vars->blips[i].x == blips[j].x && vars->blips[i].y == blips[j].y) {
	  got_it = True;
	  break;
	}
      if (!got_it && num_blips < MAX_BLIPS) {
	blips[num_blips] = vars->blips[i];
	blip_updated[num_blips] = vars->blip_updated[i];
	num_blips++;
      }
    }

  /* Remove friendly blips. */

  vars->num_blips = 0;
  for (i = 0; i < num_blips; i++) {
    for (j = 0; j < vars->num_friends; j++) {
      if (vars->friends[j].visible_enemies)
	continue;
      dx = blips[i].x - vars->friends[j].loc.grid_x;
      dy = blips[i].y - vars->friends[j].loc.grid_y;
      if (ABS(dx) <= 1 && ABS(dy) <= 1) {
	blips[i].x = -1;
	break;
      }
    }
    if (blips[i].x != -1) {
      vars->blips[vars->num_blips] = blips[i];
      vars->blip_updated[vars->num_blips] = blip_updated[i];
      for (dx = blips[i].x - 3; dx <= blips[i].x + 3; dx++)
	for (dy = blips[i].y - 3; dy <= blips[i].y + 3; dy++)
	  if (OnMaze(dx,dy))
	    vars->hot_spot[dx][dy] = fnum - 10 * ABS(dx - blips[i].x)
	      - 10 * ABS(dy - blips[i].y);
      vars->num_blips++;
    }
  }

  /* Adjust hot spot map if there's nothing on radar. */

  if (vars->num_blips == 0)
    for (dx = vars->us.loc.grid_x - 3; dx <= vars->us.loc.grid_x + 3; dx++)
      for (dy = vars->us.loc.grid_y - 3; dy <= vars->us.loc.grid_y + 3; dy++)
	if (OnMaze(dx, dy))
	  vars->hot_spot[dx][dy] = -100;
}

/*** Determine maze area and unreachable outside areas; store in bool_map. ***/

static void fill_maze(vars, init_x, init_y)
  GnatVars *vars;
  int init_x, init_y;
{
  int x, y;
  int dx, dy;
  int bottom, top;
  WallSide dir;

#ifdef DEBUG_GNAT
  send_msg(RECIPIENT_ALL, OP_TEXT, "Filling maze.");
#endif DEBUG_GNAT

  top = 1;
  bottom = 0;
  x = init_x;
  y = init_y;
  if (!vars->seen_box[x][y])
    vars->unmapped_boxes--;
  vars->seen_box[x][y] = InfiniteNumber;
  vars->qx[bottom] = x;
  vars->qy[bottom] = y;

  /* Seed fill algorithm which works breadth-first using a queue. */

  do {

    /* Pull one off the queue. */

    x = vars->qx[bottom];
    y = vars->qy[bottom];
    bottom = (bottom + 1) % MaxFastMoves;

    /* Visit it's neighbors which have not been already queued. */

    for (dir = NORTH; dir <= WEST; dir++) {
      dx = x + dir_offset[dir].x;
      dy = y + dir_offset[dir].y;
      if (OnMaze(dx, dy) && top != bottom - 1 && 
	  vars->seen_box[dx][dy] != InfiniteNumber &&
	  map_wall(vars->map, dir, x, y) != MAP_WALL) {
	vars->qx[top] = dx;
	vars->qy[top] = dy;
	if (!vars->seen_box[dx][dy])
	  vars->unmapped_boxes--;
	vars->seen_box[dx][dy] = InfiniteNumber;
	top = (top + 1) % MaxFastMoves;
      }
    }
  } while (bottom != top);
}

/*** Initialization.  Find out what we have. ***/

static void gnat_init(vars)
  GnatVars *vars;
{
  int i, j;
  int vw, vh;
  int types;
  Boolean got_type;

  /* Vehicle information. */

  vars->max_speed = max_speed();
  set_abs_drive(vars->drive = 0.0);
  vars->accel = settings.normal_friction * MIN(acc(), MAX_ACCEL);
  vars->decel = settings.normal_friction * tread_acc();
  if (get_tread_type() == HOVER_TREAD)
    vars->hover = True;
  else
    vars->hover = False;
  turn_vehicle(PI / 4.0);
  done();
  vehicle_size(&vw, &vh);
  vars->vsize = MAX(vw, vh) + 10;    /* Maximum profile in any direction. */

  /* Armor variables. */

  vars->min_armor = InfiniteNumber;
  for (i = 0; i < MAX_SIDES; i++) {
    vars->max_armor[i] = max_armor(i);
    if (i < TOP && i != BOTTOM && vars->max_armor[i] < vars->min_armor)
      vars->min_armor = vars->max_armor[i];
  }
  vars->taking_damage = False;
  vars->protection = protection();
  vars->repair_top = False;
  check_armor(vars);

  if (vars->max_armor[FRONT] > 2 * vars->max_armor[BACK])
    vars->rammer = True;
  else
    vars->rammer = False;
  vars->ram_him = False;

  /* Weapon variables. */

  vars->num_weapons = num_weapons();
  vars->mortar_capable = False;
  vars->weapon_heat = 0;
  types = 0;
  for (i = 0; i < vars->num_weapons; i++) {
    get_weapon(i, vars->weapons + i);
    vars->weapon_ok[i] = True;
    vars->weapon_on[i] = False;
    turn_off_weapon(i);

    if (vars->weapons[i].range >= 480)
      vars->mortar_capable = True;

    got_type = False;
    for (j = 0; j < types; j++)
      if (vars->wptype_speed[j] == vars->weapons[i].ammo_speed &&
	  vars->wptype_mount[j] == vars->weapons[i].mount &&
	  vars->wptype_range[j] == vars->weapons[i].frames) {
	got_type = True;
	vars->weapon_type[i] = j;
	break;
      }
    if (!got_type) {
      vars->wptype_speed[types] = vars->weapons[i].ammo_speed;
      vars->wptype_mount[types] = vars->weapons[i].mount;
      vars->wptype_range[types] = vars->weapons[i].frames;
      vars->wptype_total[types] = 0;
      vars->weapon_type[i] = types;
      types++;
    }

    vars->wptype_total[vars->weapon_type[i]] += vars->weapons[i].max_ammo;
  }
  vars->num_wptypes = types;

  check_ammo(vars);

  /* Fuel variables. */

  vars->max_fuel = max_fuel();
  check_fuel(vars);

  /* Misc. */

  vars->action = NoAction;
  vars->movement = Default;
  vars->start_time = frame_number();
  vars->pos_changed = True;
  if (settings.game == RACE_GAME) {
    vars->goal_changed = False;
    vars->goal.x = -100;
    vars->goal.y = -100;
  }

  /* Intercepting objects. */

  for (i = 0; i < MAX_BULLETS; i++) {
    vars->b_fnum[i] = -100;
    vars->b_ours[i] = False;
    vars->ivects[i].hits = False;
  }
  vars->num_tanks = 0;
  vars->outposts = 0;
  vars->enemy_los = False;
  vars->visible_enemies = 0;
  vars->close_enemies = 0;
  vars->withdraw = False;
  vars->fnum_enemy_sighting = -100;
  vars->num_enemies = 0;
  vars->victim = -1;

  vars->num_ivects = 0;
  vars->num_bullets = 0;
  vars->seeker_on_tail = False;
  vars->mine_ahead = False;

  /* Friend and long-distance target info. */

  vars->num_friends = 0;
  vars->visible_friends = 0;
  vars->close_friends = 0;
  vars->active_friends = 8;
  vars->last_friend_update = -MaxDeadTime;
  for (i = 0; i < MAX_VEHICLES; i++)
    vars->vtof[i] = -1;
  for (i = 0; i < GRID_WIDTH; i++)
    for (j = 0; j < GRID_HEIGHT; j++)
      vars->hot_spot[i][j] = -100;

  vars->num_blips = 0;

  /* Map and landmark objects. */

  vars->num_armor_depots = 0;
  vars->num_ammo_depots = 0;
  vars->num_fuel_depots = 0;
  vars->map = map_get();
  check_map(vars);
  get_self(&(vars->us));
  vars->team_code = MAX_VEHICLES + vars->us.team;

  if (settings.full_map) {
    for (i = 0; i < GRID_WIDTH; i++)
      for (j = 0; j < GRID_HEIGHT; j++)
	vars->seen_box[i][j] = 0;
    fill_maze(vars, vars->us.loc.grid_x, vars->us.loc.grid_y);
    for (i = 0; i < GRID_WIDTH; i++)
      for (j = 0; j < GRID_HEIGHT; j++)
	if (vars->seen_box[i][j])
	  vars->seen_box[i][j] = 0;
	else
	  vars->seen_box[i][j] = InfiniteNumber;
    vars->unmapped_boxes = 0;
  }
  else {
    vars->unmapped_boxes = GRID_WIDTH * GRID_HEIGHT;
    for (i = 0; i < GRID_WIDTH; i++)
      for (j = 0; j < GRID_HEIGHT; j++)
	vars->seen_box[i][j] = 0;
  }

  check_geography(vars);

  /* Path variables. */

  vars->path_ok = False;
  vars->cur_grad = 0;
  vars->fast_gradient = vars->gradient_buffer[0];
  vars->opt_path_planning = NotWorking;
  vars->working_on_destwall = False;
}

/*** Send message to friendly tanks. ***/

static void send_to_friends(vars, type, recipient)
  GnatVars *vars;
  MessageTypes type;
  Byte recipient;
{
  int i, b;
  int total_depots, depots_sent;
  Byte data[MAX_DATA_LEN];

  data[0] = (unsigned char) type + MessageKey;
  if (type != DepotInfo) {
    data[1] = (unsigned char) vars->us.loc.grid_x + CryptKey;
    data[2] = (unsigned char) vars->us.loc.grid_y + CryptKey;
    data[3] = (unsigned char) vars->action + CryptKey;
    if (vars->path_ok) {
      data[4] = (unsigned char) vars->dest.x + CryptKey;
      data[5] = (unsigned char) vars->dest.y + CryptKey;
    }
    else {
      data[4] = GRID_WIDTH + CryptKey;
      data[5] = CryptKey;
    }
    i = vars->armor_cond;
    i <<= 2;
    i |= vars->ammo_cond;
    i <<= 2;
    i |= vars->fuel_cond;
    i <<= 1;
    i |= vars->mortar_capable;
    data[6] = (unsigned char) i + CryptKey;
    data[7] = (unsigned char) vars->weapon_heat + CryptKey;
    i = MIN(vars->close_enemies, 7);
    i <<= 3;
    i |= MIN(vars->visible_friends, 7);
    data[8] = (unsigned char) i + CryptKey;
    b = 9;
    vars->last_friend_update = frame_number();
    
    /* Transmit depot information. */
    
    for (i = 0; i < vars->num_armor_depots; i++)
      if (vars->new_armor[i] && b < MAX_DATA_LEN - 3) {
	vars->new_armor[i] = False;
	data[b++] = (unsigned char) ARMOR + CryptKey;
	data[b++] = (unsigned char) vars->armor_depots[i].x + CryptKey;
	data[b++] = (unsigned char) vars->armor_depots[i].y + CryptKey;
      }
    for (i = 0; i < vars->num_ammo_depots; i++)
      if (vars->new_ammo[i] && b < MAX_DATA_LEN - 3) {
	vars->new_ammo[i] = False;
	data[b++] = (unsigned char) AMMO + CryptKey;
	data[b++] = (unsigned char) vars->ammo_depots[i].x + CryptKey;
	data[b++] = (unsigned char) vars->ammo_depots[i].y + CryptKey;
      }
    for (i = 0; i < vars->num_fuel_depots; i++)
      if (vars->new_fuel[i] && b < MAX_DATA_LEN - 3) {
	vars->new_fuel[i] = False;
	data[b++] = (unsigned char) FUEL + CryptKey;
	data[b++] = (unsigned char) vars->fuel_depots[i].x + CryptKey;
	data[b++] = (unsigned char) vars->fuel_depots[i].y + CryptKey;
      }
    data[b] = 0;
    send_msg(recipient, OP_TEXT, data);
  }
  else {      /* Send depot information. */
    total_depots = vars->num_armor_depots + vars->num_ammo_depots +
      vars->num_fuel_depots;
    depots_sent = 0;
    while (depots_sent < total_depots) {
      if (depots_sent != 0)
	data[0] = (unsigned char) type + MessageKey;
      b = 1;
      if (depots_sent < vars->num_armor_depots)
	for (i = depots_sent; i < vars->num_armor_depots; i++)
	  if (b < MAX_DATA_LEN - 3) {
	    depots_sent++;
	    data[b++] = (unsigned char) ARMOR + CryptKey;
	    data[b++] = (unsigned char) vars->armor_depots[i].x + CryptKey;
	    data[b++] = (unsigned char) vars->armor_depots[i].y + CryptKey;
	  }
      if (depots_sent < vars->num_armor_depots + vars->num_ammo_depots)
	for (i = 0; i < vars->num_ammo_depots; i++)
	  if (b < MAX_DATA_LEN - 3) {
	    depots_sent++;
	    data[b++] = (unsigned char) AMMO + CryptKey;
	    data[b++] = (unsigned char) vars->ammo_depots[i].x + CryptKey;
	    data[b++] = (unsigned char) vars->ammo_depots[i].y + CryptKey;
	  }
      if (depots_sent < total_depots)
	for (i = 0; i < vars->num_fuel_depots; i++)
	  if (b < MAX_DATA_LEN - 3) {
	    depots_sent++;
	    data[b++] = (unsigned char) FUEL + CryptKey;
	    data[b++] = (unsigned char) vars->fuel_depots[i].x + CryptKey;
	    data[b++] = (unsigned char) vars->fuel_depots[i].y + CryptKey;
	  }
      data[b] = 0;
      send_msg(recipient, OP_TEXT, data);
    }
  }
}

/*** Communicate with friendly tanks. ***/

static void check_friends(vars)
  GnatVars *vars;
{
  int i, j, b, type, x, y, friend;
  Boolean new_info;
  Message msg;

  while (receive_msg(&msg)) {
    if (msg.opcode == OP_TEXT &&
	(int)msg.data[0] >= (int)(MessageKey + FriendStatus) &&
	(int)msg.data[0] <= (int)(MessageKey + DepotInfo)) {
      if (msg.sender == vars->us.id)  /* No need to read our own messages. */
	continue;
      msg.data[0] -= MessageKey;
      if (msg.data[0] == DepotInfo) {
	if (frame_number() - vars->start_time > 200)
	  continue;
	b = 1;
      }
      else {
	if (msg.data[0] == CheckingIn)
	  send_to_friends(vars, DepotInfo, msg.sender);
	new_info = True;
	for (i = 0; i < vars->num_friends; i++)
	  if (vars->friends[i].id == msg.sender) {
	    friend = i;
	    new_info = False;
	    break;
	  }
	if (new_info) {    /* We have a new friend. */
	  friend = vars->num_friends++;
	  vars->friends[friend].id = msg.sender;
	  vars->vtof[msg.sender] = friend;
	}
	vars->friends[friend].loc.grid_x = msg.data[1] - CryptKey;
	vars->friends[friend].loc.grid_y = msg.data[2] - CryptKey;
	vars->friends[friend].action = msg.data[3] - CryptKey;
	vars->friends[friend].dest.x = msg.data[4] - CryptKey;
	vars->friends[friend].dest.y = msg.data[5] - CryptKey;
	i = msg.data[6] - CryptKey;
	vars->friends[friend].mortar_capable = i & 0x00000001;
	i >>= 1;
	vars->friends[friend].fuel_cond = i & 0x00000003;
	i >>= 2;
	vars->friends[friend].ammo_cond = i & 0x00000003;
	i >>= 2;
	vars->friends[friend].armor_cond = i & 0x00000003;
	vars->friends[friend].condition = MIN(vars->friends[friend].armor_cond,
					      vars->friends[friend].ammo_cond);
	vars->friends[friend].heat = msg.data[7] - CryptKey;
	i = msg.data[8] - CryptKey;
	vars->friends[friend].visible_friends = i & 0x00000007;
	i >>= 3;
	vars->friends[friend].visible_enemies = i & 0x00000007;
	vars->friends[friend].fnum = msg.frame;
	b = 9;

	/* Update hot spot information. */

	if (vars->friends[friend].visible_enemies)
	  for (x = -2; x <= 2; x++)
	    for (y = -2; y <= 2; y++) {
	      i = vars->friends[friend].loc.grid_x + x;
	      j = vars->friends[friend].loc.grid_y + y;
	      if (OnMaze(i,j) && vars->hot_spot[i][j] < msg.frame)
		vars->hot_spot[i][j] = msg.frame;
	    }
      }

      /* Check depot information against our memory. */

      while (b < MAX_DATA_LEN - 3 && msg.data[b] != 0) {
	type = msg.data[b++] - CryptKey;
	x = msg.data[b++] - CryptKey;
	y = msg.data[b++] - CryptKey;
	update_depot_info(vars, type, x, y, False);
      }
    }
  }
}

/*** Check priorities for depot use. ***/

static Boolean depot_priority(vars)
  GnatVars *vars;
{
  int i, dx, dy, dist;
  int fdist[MAX_VEHICLES];
  int fnum, space;
  LandmarkType type;

  if (vars->num_friends == 0)
    return(True);

  type = OnLandmark(vars);
  if (type < FUEL || type > ARMOR)
    return(True);

  fnum = frame_number();
  for (i = 0; i < vars->num_friends; i++)
    if (vars->friends[i].in_sight) {
      dx = CenterXofBox(vars->us.loc.grid_x) - vars->friends[i].loc.x;
      dy = CenterYofBox(vars->us.loc.grid_y) - vars->friends[i].loc.y;
      fdist[i] = dx * dx + dy * dy;
    }
    else
      fdist[i] = InfiniteNumber;
  
  dx = CenterXofBox(vars->us.loc.grid_x) - vars->us.loc.x;
  dy = CenterYofBox(vars->us.loc.grid_y) - vars->us.loc.y;
  dist = dx * dx + dy * dy;

  space = LANDMARK_WIDTH * LANDMARK_HEIGHT;
  for (i = 0; i < vars->num_friends; i++)
    if (fdist[i] <= dist)
      space -= vars->friends[i].vsize * vars->friends[i].vsize;

  if (space <= 0)
    return(False);

  return(True);
}

/*** Grows gradient map from our target coordinate quickly. ***/

typedef enum {

  AvoidScrolls = 0,
  AvoidDestWalls,
  AvoidEnemies,
  AllowAnything

} ToleranceAmount;

static Boolean found_path(vars, target_x, target_y, path_needs, severity)
  GnatVars *vars;
  int target_x, target_y;
  ToleranceAmount path_needs;
  int severity;  /* Of xenophobia that is. */
{
  int e, x, y, score;
  int dx, dy;
  int bottom, top;
  WallSide dir;

  for (x = 0; x < GRID_WIDTH; x++)
    for (y = 0; y < GRID_HEIGHT; y++) {
      vars->fast_gradient[x][y] = BadBox;
      vars->bool_map[x][y] = False;
    }

  /* Check for enemies if we're running. */

  if (path_needs == AvoidEnemies) {
    for (e = 0; e < vars->num_enemies; e++)
      if (vars->enemies[e].in_sight) {
	x = vars->enemies[e].loc.grid_x;
	y = vars->enemies[e].loc.grid_y;
	for (dx = x - severity; dx <= x + severity; dx++)
	  for (dy = y - severity; dy <= y + severity; dy++)
	    if (OnMaze(dx, dy) && 
		(dx != vars->us.loc.grid_x || dy != vars->us.loc.grid_y) &&
		(dx != target_x || dy != target_y))
	      vars->bool_map[dx][dy] = True;
      }
    for (e = 0; e < vars->num_blips; e++) {
      x = vars->blips[e].x;
      y = vars->blips[e].y;
      if (x == target_x && y == target_y)
	continue;
      for (dx = x - severity; dx <= x + severity; dx++)
	for (dy = y - severity; dy <= y + severity; dy++)
	  if (OnMaze(dx, dy) && 
	      (dx != vars->us.loc.grid_x || dy != vars->us.loc.grid_y) &&
	      (dx != target_x || dy != target_y))
	    vars->bool_map[dx][dy] = True;
    }
  }

  top = 1;
  bottom = 0;
  x = target_x;
  y = target_y;
  vars->fast_gradient[x][y] = 0;
  vars->qx[bottom] = x;
  vars->qy[bottom] = y;

  /* Seed fill algorithm which works breadth-first using a queue. */

  do {

    /* Pull one off the queue. */

    x = vars->qx[bottom];
    y = vars->qy[bottom];
    score = vars->fast_gradient[x][y];
    bottom = (bottom + 1) % MaxFastMoves;

    /* Visit it's neighbors which have not been already queued. */

    for (dir = NORTH; dir <= WEST; dir++) {
      dx = x + dir_offset[dir].x;
      dy = y + dir_offset[dir].y;
      if (OnMaze(dx, dy) && top != bottom - 1 && 
	  vars->fast_gradient[dx][dy] >= BadBox) {

	if (path_needs <= AvoidDestWalls) {
	  if (map_wall(vars->map, dir, x, y))
	    continue;
	}
	else if (map_wall(vars->map, dir, x, y) == MAP_WALL)
	  continue;

	if (path_needs == AvoidEnemies && vars->bool_map[dx][dy])
	  continue;

	if (path_needs != AllowAnything && settings.box_slowdown < 0.5 &&
	    vars->map[dx][dy].type == SLOW)
	  continue;

	if (path_needs == AvoidScrolls)
	  switch (vars->map[dx][dy].type) {
	  case SCROLL_S:
	    if (dir == SOUTH)
	      continue;
	    break;
	  case SCROLL_SE:
	    if (dir == SOUTH || dir == EAST)
	      continue;
	    break;
	  case SCROLL_SW:
	    if (dir == SOUTH || dir == WEST)
	      continue;
	    break;
	  case SCROLL_N:
	    if (dir == NORTH)
	      continue;
	    break;
	  case SCROLL_NE:
	    if (dir == NORTH || dir == EAST)
	      continue;
	    break;
	  case SCROLL_NW:
	    if (dir == NORTH || dir == WEST)
	      continue;
	    break;
	  case SCROLL_E:
	    if (dir == EAST)
	      continue;
	    break;
	  case SCROLL_W:
	    if (dir == WEST)
	      continue;
	    break;
	  case SLOW:
	    continue;
	    break;
	  }
	
	vars->qx[top] = dx;
	vars->qy[top] = dy;
	vars->fast_gradient[dx][dy] = score + 1;
	top = (top + 1) % MaxFastMoves;
      }
    }
  } while ((vars->us.loc.grid_x != x || vars->us.loc.grid_y != y) && 
	   bottom != top);

  if (vars->fast_gradient[vars->us.loc.grid_x][vars->us.loc.grid_y] >= BadBox)
    return(False);

  vars->fnum_path_computed = frame_number();
  vars->dest.x = target_x;
  vars->dest.y = target_y;
  vars->opt_path_planning = NotWorking;  /* Map or target has changed. */
  return(True);
}

/*** Go through difference tolerances under fast mapping. ***/

static Boolean found_fast_path(vars, target_x, target_y, avoid)
  GnatVars *vars;
  int target_x, target_y;
  Boolean avoid;
{
  Boolean got_path;
  int x, y;
  
  if (!OnMaze(target_x, target_y))
    return(vars->path_ok = False);

  got_path = True;
  if (settings.game == RACE_GAME) {
    if (!found_path(vars, target_x, target_y, AvoidScrolls, 0))
      if (!found_path(vars, target_x, target_y, AvoidDestWalls, 0))
	got_path = found_path(vars, target_x, target_y, AllowAnything, 0);
  }
  else if (avoid) {
    if (!vars->visible_enemies) {
      if (!found_path(vars, target_x, target_y, AvoidEnemies, 3))
	  if (!found_path(vars, target_x, target_y, AvoidEnemies, 0))
	    got_path = found_path(vars, target_x, target_y, AllowAnything,0);
    }
    else if (!found_path(vars, target_x, target_y, AvoidEnemies, 0))
      got_path = found_path(vars, target_x, target_y, AllowAnything, 0);
  }
  else
    got_path = found_path(vars, target_x, target_y, AllowAnything, 0);

  return(vars->path_ok = got_path);
}

/*** Grows gradient map from target coordinate optimally & incrementally. ***/

static void find_opt_path(vars)
  GnatVars *vars;
{
  int iterations;
  int e, x, y, score;
  int dx, dy, cost, fnum;
  int slow_factor, scroll_factor, slip_factor;
  int outpost_factor;
  WallSide dir;
  Boolean avoid_hotspots;

  if (vars->withdraw) {
    avoid_hotspots = True;
    for (x = 0; x < GRID_WIDTH; x++)
      for (y = 0; y < GRID_HEIGHT; y++)
	vars->bool_map[x][y] = False;
    for (e = 0; e < vars->num_enemies; e++)
      if (vars->enemies[e].in_sight) {
	x = vars->enemies[e].loc.grid_x;
	y = vars->enemies[e].loc.grid_y;
	if (x != vars->us.loc.grid_x || y != vars->us.loc.grid_y)
	  vars->bool_map[x][y] = True;
      }
  }
  else
    avoid_hotspots = False;
  fnum = frame_number();

  if (!vars->opt_path_planning) {      /* Initialize the mapping. */
    for (x = 0; x < GRID_WIDTH; x++)
      for (y = 0; y < GRID_HEIGHT; y++) 
	vars->opt_gradient[x][y] = BadBox;
    vars->q_bottom = 0;
    vars->q_top = 1;
    vars->opt_path_planning = Working;
    vars->opt_gradient[vars->dest.x][vars->dest.y] = 0;
    vars->oqx[0] = vars->dest.x;
    vars->oqy[0] = vars->dest.y;
  }

  iterations = 0;

  /* Rate the hazards by their settings. */

  slow_factor = (1.0 - settings.box_slowdown) * 200;
  scroll_factor = settings.scroll_speed * 10;
  slip_factor = (1.0 - settings.slip_friction) * 40;
  outpost_factor = settings.outpost_strength * 20;
  
  /* Seed fill algorithm which works breadth-first using a queue. */
  
  do {
    
    /* Pull one off the queue. */
    
    x = vars->oqx[vars->q_bottom];
    y = vars->oqy[vars->q_bottom];
    score = vars->opt_gradient[x][y];
    vars->q_bottom = (vars->q_bottom + 1) % MaxOptMoves;
    
    /* Visit it's neighbors which have not been already queued. */
    
    for (dir = NORTH; dir <= WEST; dir++) {
      dx = x + dir_offset[dir].x;
      dy = y + dir_offset[dir].y;
      if (OnMaze(dx, dy) && vars->q_top != vars->q_bottom - 1) {
	
	/* Add the cost of going through walls. */
	
	switch (map_wall(vars->map, dir, x, y)) {
	case MAP_NONE:
	  cost = 10;
	  break;
	case MAP_WALL:
	  cost = BadBox + 10;
	  break;
	case MAP_DEST:
	  cost = 50;
	  break;
	}

	/* Add the cost of being a hotspot. */

	if (avoid_hotspots) {
	  if (vars->hot_spot[dx][dy] >= 0) {
	    e = fnum - vars->hot_spot[dx][dy];
	    cost += 10 * (200 - MIN(200, e));
	  }
	  if (vars->bool_map[dx][dy])
	    cost += 10000;
	}

	/* Add the cost of going through scrolls. */
	
	switch (vars->map[dx][dy].type) {
	case OUTPOST:
	  cost += outpost_factor;
	  break;
	case SCROLL_S:
	  if (dir == NORTH)
	    cost -= MIN(9, scroll_factor / 10);
	  else if (dir == SOUTH)
	    cost += scroll_factor;
	  else
	    cost += scroll_factor / 2;
	  break;
	case SCROLL_SE:
	  if (dir == NORTH || dir == WEST)
	    cost -= MIN(4, scroll_factor / 10);
	  else
	    cost += scroll_factor;
	  break;
	case SCROLL_SW:
	  if (dir == NORTH || dir == EAST)
	    cost -= MIN(4, scroll_factor / 10);
	  else
	    cost += scroll_factor;
	  break;
	case SCROLL_N:
	  if (dir == SOUTH)
	    cost -= MIN(9, scroll_factor / 10);
	  else if (dir == NORTH)
	    cost += scroll_factor;
	  else
	    cost += scroll_factor / 2;
	  break;
	case SCROLL_NE:
	  if (dir == SOUTH || dir == WEST)
	    cost -= MIN(4, scroll_factor / 10);
	  else
	    cost += scroll_factor;
	  break;
	case SCROLL_NW:
	  if (dir == SOUTH || dir == EAST)
	    cost -= MIN(4, scroll_factor / 10);
	  else
	    cost += scroll_factor;
	  break;
	case SCROLL_E:
	  if (dir == WEST)
	    cost -= MIN(9, scroll_factor / 10);
	  else if (dir == EAST)
	    cost += scroll_factor;
	  else
	    cost += scroll_factor / 2;
	  break;
	case SCROLL_W:
	  if (dir == EAST)
	    cost -= MIN(9, scroll_factor / 10);
	  else if (dir == WEST)
	    cost += scroll_factor;
	  else
	    cost += scroll_factor / 2;
	  break;
	case SLIP:
	  cost += slip_factor;
	  break;
	case SLOW:
	  cost += slow_factor;
	  break;
	}
	
	/* If current score is cheaper than what's there, put on queue. */
	
	cost += score;
	if (vars->opt_gradient[dx][dy] > cost) {
	  vars->oqx[vars->q_top] = dx;
	  vars->oqy[vars->q_top] = dy;
	  vars->opt_gradient[dx][dy] = cost;
	  vars->q_top = (vars->q_top + 1) % MaxOptMoves;
	}
      }
    }
    iterations++;
    if (iterations > IterForPathPlan)
      break;
  } while ((settings.full_map || vars->us.loc.grid_x != x || 
	    vars->us.loc.grid_y != y) && vars->q_bottom != vars->q_top);
  
  if ((!settings.full_map && vars->us.loc.grid_x == x && 
       vars->us.loc.grid_y == y) || vars->q_bottom == vars->q_top)
    vars->opt_path_planning = DoneMapping;
}

/*** Return best movement heading based on current gradient map. ***/

static Boolean best_move_heading(vars, allow_dest, move_heading, through_dest)
  GnatVars *vars;
  Boolean allow_dest;      /* Whether we can move through DEST_WALL.     */
  Angle *move_heading;     /* The best heading which is on our path.     */
  Boolean *through_dest;   /* If returns TRUE, moving through DEST_WALL. */
{
  Location tloc;
  int i, j, k, x, y, dx, dy, dist;
  int lowest_x, lowest_y, low_score;
  int cur_x, cur_y;
  Boolean eligible[3][3], got_clear, got_dest, work_on_wall;
  int (*gradient)[GRID_HEIGHT];
  int clear_score, dest_score, diff1, diff2;
  Angle angle, clear_angle, dest_angle;
  WallSide dir;

  if (!vars->path_ok || (vars->us.loc.grid_x == vars->dest.x &&
			 vars->us.loc.grid_y == vars->dest.y))
    return(False);

  if (vars->opt_path_planning == DoneMapping) {
    gradient = vars->opt_gradient;
    diff1 = 10;  /* Difference between two plain boxes next to each other. */
    diff2 = 50;  /* Difference across a destructable wall. */
  }
  else {
    gradient = vars->fast_gradient;
    diff1 = 1;
    diff2 = -60;
  }

  for (x = 0; x <= 2; x++)
    for (y = 0; y <= 2; y++)
      eligible[x][y] = True;
  eligible[1][1] = False;

  /* Calculate where our current velocity vector will take us. */

  if (vars->speed > 0.0) {
    angle = floor(fixed_angle(vars->vel_angle + PI/8.0) / (PI/4.0)) * PI/4.0;
    cur_x = SIGN(COS(angle)) + 1;
    cur_y = SIGN(SIN(angle)) + 1;
  }

  /* Determine our best move. */

  got_clear = got_dest = False;

  for (i = 0; i < 8; i++) {

    /* Determine the lowest gradient in an eligible box. */

    low_score = BadBox;
    for (j = 0; j <= 2; j++)
      for (k = 0; k <= 2; k++) 
	if (eligible[j][k]) {
	  x = vars->us.loc.grid_x + j - 1;
	  y = vars->us.loc.grid_y + k - 1;
	  if (OnMaze(x,y) && gradient[x][y] < low_score) {
	    low_score = gradient[x][y];
	    lowest_x = j;
	    lowest_y = k;
	  }
	}

    /* Favor our current velocity vector if it is just as good as "best". */

    if (vars->speed > 0.0 && eligible[cur_x][cur_y] && 
	ABS(cur_x - lowest_x) + ABS(cur_y - lowest_y) <= 1) {
      x = vars->us.loc.grid_x + cur_x - 1;
      y = vars->us.loc.grid_y + cur_y - 1;
      if (OnMaze(x,y) && gradient[x][y] <= low_score) {
	lowest_x = cur_x;
	lowest_y = cur_y;
      }
    }

    if (low_score >= BadBox || (got_dest && dest_score + diff2 < low_score))
      break;
    
    dir = NO_DIR;
    if (lowest_x == 1) {
      if (lowest_y == 0)
	dir = NORTH;
      else if (lowest_y == 2)
	dir = SOUTH;
    }
    else if (lowest_y == 1) {
      if (lowest_x == 0)
	dir = WEST;
      else if (lowest_x == 2)
	dir = EAST;
    }
    lowest_x += vars->us.loc.grid_x - 1;
    lowest_y += vars->us.loc.grid_y - 1;

    /* Try to get through to our selected box. */

    tloc.grid_x = lowest_x;
    tloc.grid_y = lowest_y;
    tloc.x = CenterXofBox(lowest_x);
    tloc.y = CenterXofBox(lowest_y);
    dx = tloc.x - vars->us.loc.x;
    dy = tloc.y - vars->us.loc.y;
    angle = TAN(dy, dx);

    if (!vars->num_friends || vars->friend_dist[RadTo4Deg(vars->vel_angle)] >= 
	vars->speed * vars->speed / vars->decel) {
      if (clear_tank_path(&tloc, &(vars->us.loc), vars->vsize)) {
	got_clear = True;
	clear_angle = angle;
	clear_score = low_score;
	break;
      }
      else if (dir != NO_DIR && allow_dest && vars->have_ammo &&
	       map_wall(vars->map, dir, vars->us.loc.grid_x,
			vars->us.loc.grid_y) == MAP_DEST) {
	got_dest = True;
	dest_angle = angle;
	dest_score = low_score;
      }
    }

    lowest_x -= vars->us.loc.grid_x - 1;
    lowest_y -= vars->us.loc.grid_y - 1;
    eligible[lowest_x][lowest_y] = False;
  }

  /* Return (1) whether we got a heading, and (2) if it's destructable. */

  if (got_dest && (!got_clear || dest_score + diff2 < clear_score)) {
    *move_heading = dest_angle;
    *through_dest = True;
    return(True);
  }
  else if (got_clear) {
    *move_heading = clear_angle;
    *through_dest = False;
    return(True);
  }

  vars->path_ok = False;    /* Gradient not usable at this point. */
  return(False);
}

/*** See if there is a friend in need, and go to him. ***/

static Boolean aid_friend(vars)
  GnatVars *vars;
{
  int i, dx, dy, fnum;
  int neediest, score, best_score;
  Boolean had_path;

  fnum = frame_number();
  had_path = push_path(vars);

  /* Go through the list of friends and check for those needing aid. */

  neediest = -1;
  for (i = 0; i < vars->num_friends; i++)
    if (fnum - vars->friends[i].fnum < MaxDeadTime &&
	(vars->friends[i].visible_enemies ||
	 action_type[vars->friends[i].action] != Loading)) {
      score = 10 * vars->friends[i].visible_enemies;
      score -= 3 * vars->friends[i].visible_friends;
      score -= 3 * vars->friends[i].condition;
      dx = vars->friends[i].loc.grid_x - vars->us.loc.grid_x;
      dy = vars->friends[i].loc.grid_y - vars->us.loc.grid_y;
      score -= (ABS(dx) + ABS(dy)) / 8;
      if (neediest == -1 || score > best_score) {
	best_score = score;
	neediest = i;
      }
    }

  /* Try to move to neediest colleague. */

  if (neediest != -1 && best_score >= -5) {
    if (vars->friends[neediest].in_sight) {
      vars->path_ok = False;
      if (best_score < 0)
	return(Regroup);
      return(MustAid);
    }
    else if (found_fast_path(vars, vars->friends[neediest].loc.grid_x,
			     vars->friends[neediest].loc.grid_y, False)) {
      if (best_score < 0)
	return(Regroup);
      return(MustAid);
    }
  }

  /* Restore old gradient map if no help needed. */

  if (had_path)
    pop_path(vars);

  return(NoAid);
}

/*** Protect our weakest sides from incoming fire. ***/

static void protect_sides(vars)
  GnatVars *vars;
{
  int i, dx, dy;
  int left, right, front, back;
  int left_damage, right_damage, front_damage, back_damage;
  int damage, total, best, min_total;

  vars->movement = LastStand;
  if (vars->drive != 0.0)
    set_abs_drive(vars->drive = 0.0);

  /* Determine damage to various sides of a vehicle pointing north. */

  left_damage = right_damage = front_damage = back_damage = 0;
  for (i = 0; i < vars->num_bullets; i++)
    if (vars->bullets[i].type != MINE && vars->ivects[i].dist <= 300) {
      dx = vars->ivects[i].dx;
      dy = vars->ivects[i].dy;
      if (dy < 0 && ABS(dy) > ABS(dx))
	front_damage += vars->ivects[i].damage;
      if (dx > 0 && ABS(dx) > ABS(dy))
	right_damage += vars->ivects[i].damage;
      if (dx < 0 && ABS(dx) > ABS(dy))
	left_damage += vars->ivects[i].damage;
      if (dy > 0 && ABS(dy) > ABS(dx))
	back_damage += vars->ivects[i].damage;
    }
  
  /* Pointing North */

  total = vars->armor[LEFT] - left_damage;
  total = MIN(total, vars->armor[RIGHT] - right_damage);
  total = MIN(total, vars->armor[BACK] - back_damage);
  total = MIN(total, vars->armor[FRONT] - front_damage);
  min_total = total;
  best = NORTH;

  /* Pointing South. */

  left = right_damage;
  right = left_damage;
  back = front_damage;
  front = back_damage;

  total = vars->armor[LEFT] - left;
  total = MIN(total, vars->armor[RIGHT] - right);
  total = MIN(total, vars->armor[BACK] - back);
  total = MIN(total, vars->armor[FRONT] - front);
  if (total > min_total) {
    min_total = total;
    best = SOUTH;
  }

  /* Pointing East. */

  front = right_damage;
  back = left_damage;
  right = back_damage;
  left = front_damage;

  total = vars->armor[LEFT] - left;
  total = MIN(total, vars->armor[RIGHT] - right);
  total = MIN(total, vars->armor[BACK] - back);
  total = MIN(total, vars->armor[FRONT] - front);
  if (total > min_total) {
    min_total = total;
    best = EAST;
  }

  /* Pointing West. */

  front = left_damage;
  back = right_damage;
  right = front_damage;
  left = back_damage;

  total = vars->armor[LEFT] - left;
  total = MIN(total, vars->armor[RIGHT] - right);
  total = MIN(total, vars->armor[BACK] - back);
  total = MIN(total, vars->armor[FRONT] - front);
  if (total > min_total) {
    min_total = total;
    best = WEST;
  }

  /* Turn to the best direction. */

  switch (best) {
  case NORTH:
    turn_vehicle(HALF_CIRCLE + QUAR_CIRCLE);
    break;
  case SOUTH:
    turn_vehicle(QUAR_CIRCLE);
    break;
  case EAST:
    turn_vehicle(0.0);
    break;
  case WEST:
    turn_vehicle(HALF_CIRCLE);
    break;
  }
}

/*** Avoid both hostile and friendly intercepting objects. ***/

#define MaxDodges 8
static Angle dodge_angles[8] = {   0.0,  -45.0,   45.0,  -90.0,  90.0,
				-135.0,  135.0,  180.0 };

static void dodge_ivectors(vars)
  GnatVars *vars;
{
  int try, max_tries;
  int min_try, min_damage, damage, mine_damage;
  int i, j, x, y, t;
  int dx, dy, gx, gy;
  Angle angle, opt_angle;
  float vx, vy, max_vx, max_vy, x_acc, y_acc;
  float cos_angle, sin_angle;
  float hit_ratio;
  int hit_wall, hit_mines;
  int deg, opt_dir[24];
  Boolean dest_wall, brake;

  if (vars->hover)
    hit_ratio = 0.0;
  else
    hit_ratio = (float) vars->min_side / (float) vars->armor[BOTTOM];

  /* Determine the default direction and number of headings to consider. */

  if (settings.game == RACE_GAME) {
    if (best_move_heading(vars, True, &opt_angle, &dest_wall)) {
      max_tries = 2;
      vars->working_on_destwall = dest_wall;
    }
    else {
      vars->movement = Default;
      return;
    }
  }
  else if (vars->movement != Panic &&
	   best_move_heading(vars, False, &opt_angle, &dest_wall)) {
#ifdef DEBUG_GNAT
    send_msg(RECIPIENT_ALL, OP_TEXT, "Dodging using gradient");
#endif
    if (vars->movement == Evading)
      max_tries = 3;
    else
      max_tries = 6;
    vars->working_on_destwall = dest_wall;
  }
  else {

#ifdef DEBUG_GNAT
    send_msg(RECIPIENT_ALL, OP_TEXT, "Free for all dodging");
#endif
    max_tries = MaxDodges;
    for (i = 0; i < 24; i++)
      opt_dir[i] = 0;

    /* Favor heading in accordance with current velocity vector. */

    deg = (RadTo15Deg(vars->vel_angle) - 3) % 24;
    for (i = 0; i < 7; i++) {
      opt_dir[deg]--;
      deg = (deg + 1) % 24;
    }

    /* Move away from any enemies except victim. */

    for (i = 0; i < vars->num_enemies; i++)
      if (vars->enemies[i].in_sight && 
	  (i != vars->victim || vars->withdraw)) {
	deg = (RadTo15Deg(vars->enemies[i].pos_angle) - 5) % 24;
	for (j = 0; j < 11; j++) {
	  opt_dir[deg] += 3;
	  deg = (deg + 1) % 24;
	}
	if (vars->enemies[i].rspeed < 0) {   /* Boogie is gaining. */
#ifdef DEBUG_GNAT
	  send_msg(RECIPIENT_ALL, OP_TEXT, "Going round boogie.");
#endif
	  deg = (RadTo15Deg(vars->enemies[i].pos_angle + PI/2) - 1) % 24;
	  for (j = 0; j < 3; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	  deg = (RadTo15Deg(vars->enemies[i].pos_angle - PI/2) - 1) % 24;
	  for (j = 0; j < 3; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	}
	else {
#ifdef DEBUG_GNAT
	  send_msg(RECIPIENT_ALL, OP_TEXT, "Backing from boogie.");
#endif
	  deg = (RadTo15Deg(vars->enemies[i].pos_angle + PI) - 2) % 24;
	  for (j = 0; j < 5; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	}
      }

    /* Adjust heading if we have a victim. */

    if (vars->victim != -1 && !vars->withdraw) {
      t = vars->victim;
      if (vars->armor_cond >= Operational && vars->max_side > 50 &&
	  (!vars->enemies[t].in_sight || 
	   vars->enemies[t].dist >= vars->range)) {

	/* Move in on victim. */

	if (vars->enemies[t].in_sight) {
	  if (map_landmark(vars->map, vars->enemies[t].loc.grid_x,
			   vars->enemies[t].loc.grid_y) == ARMOR) {
	    max_tries = 2;
	    deg = (RadTo15Deg(vars->enemies[t].pos_angle) - 1) % 24;
	    for (i = 0; i < 3; i++) {
	      opt_dir[deg] -= 100;
	      deg = (deg + 1) % 24;
	    }
	  }
	  else {
	    deg = (RadTo15Deg(vars->enemies[t].pos_angle) - 5) % 24;
	    for (i = 0; i < 11; i++) {
	      if (i < 4 || i > 6)
		opt_dir[deg]--;
	      deg = (deg + 1) % 24;
	    }
	  }
	}
	else {
	  dx = vars->enemies[t].loc.x - vars->us.loc.x;
	  dy = vars->enemies[t].loc.y - vars->us.loc.y;
	  deg = (RadTo15Deg(TAN(dy, dx)) - 3) % 24;
	  for (i = 0; i < 7; i++) {
	    opt_dir[deg] -= 2;
	    deg = (deg + 1) % 24;
	  }
	}
      }
      else if (vars->enemies[vars->victim].in_sight) { 

	/* Move away. */

	deg = (RadTo15Deg(vars->enemies[vars->victim].pos_angle) - 5) % 24;
	for (i = 0; i < 11; i++) {
	  opt_dir[deg] += 3;
	  deg = (deg + 1) % 24;
	}
	if (vars->enemies[i].rspeed < 0) {  /* Boogie is gaining. */
	  deg = (RadTo15Deg(vars->enemies[vars->victim].pos_angle + 
			    PI/2) - 1) % 24;
	  for (j = 0; j < 3; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	  deg = (RadTo15Deg(vars->enemies[vars->victim].pos_angle - 
			    PI/2) - 1) % 24;
	  for (j = 0; j < 3; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	}
	else {
	  deg = (RadTo15Deg(vars->enemies[vars->victim].pos_angle + 
			    PI) - 2) % 24;
	  for (j = 0; j < 5; j++) {
	    opt_dir[deg]--;
	    deg = (deg + 1) % 24;
	  }
	}
      }
    }

    /* Select best direction as our optimal heading. */

    min_try = try = RadTo15Deg(vars->vel_angle);
    min_damage = opt_dir[try];
    for (i = 0; i < 23; i++) {
      try = (try + 1) % 24;
      if (opt_dir[try] < min_damage) {
	min_try = try;
	min_damage = opt_dir[try];
      }
    }
    opt_angle = Deg15ToRad(min_try);
  }

  /* Try different headings in an effort to avoid bullets/mines. */

  hit_wall = 0;
  min_try = -1;
  for (try = 0; try <= max_tries; try++) {

    if (vars->mine_ahead)
      for (i = 0; i <= vars->east_x + vars->west_x; i++)
	for (j = 0; j <= vars->north_y + vars->south_y; j++)
	  vars->mine_field[i][j] = 0;

    vx = vars->us.xspeed;
    vy = vars->us.yspeed;

    if (try == MaxDodges || (max_tries == 3 && try == 3)) {
      if (vars->movement == Avoiding)
	continue;
      brake = True;
      x_acc = -SIGN(vars->us.xspeed) * vars->decel;
      y_acc = -SIGN(vars->us.yspeed) * vars->decel;
    }
    else {
      brake = False;
      angle = opt_angle + DegToRad(dodge_angles[try]);
      cos_angle = COS(angle);
      sin_angle = SIN(angle);
      x_acc = cos_angle * vars->accel;
      y_acc = sin_angle * vars->accel;
      if (vars->movement != Avoiding) {
	max_vx = cos_angle * vars->max_speed;
	max_vy = sin_angle * vars->max_speed;
      }
      else {
	max_vx = cos_angle * 6.0;
	max_vy = sin_angle * 6.0;
      }
    }

    damage = mine_damage = 0;
    x = vars->us.loc.x;
    y = vars->us.loc.y;
    for (t = 0; t < IEti; t++) {
      dx = x - vars->us.loc.x;
      dy = y - vars->us.loc.y;
      i = vars->west_x + (dx + SIGN(dx) * vars->vsize / 2) / vars->vsize;
      j = vars->north_y + (dy + SIGN(dy) * vars->vsize / 2) / vars->vsize;
      if (i >= 0 && i <= vars->east_x + vars->west_x &&
	  j >= 0 && j <= vars->north_y + vars->south_y) {
	if (vars->igrid[t][i][j] == InfiniteNumber) {           /* Hit tank. */
	  damage += 5;
	  vx = -vx / 2.0;
	  vy = -vy / 2.0;
	}
	else if (vars->igrid[t][i][j] >= 1000) {
	  hit_mines = vars->igrid[t][i][j] / 1000 - vars->mine_field[i][j];
	  mine_damage += hit_mines * (6 - vars->protection);
	  if (mine_damage >= vars->armor[BOTTOM]) {
	    damage = InfiniteNumber;
	    break;
	  }
	  vars->mine_field[i][j] += hit_mines;
	  damage += (vars->igrid[t][i][j] % 1000) + 
	    hit_ratio * hit_mines * 6;
	}
	else 
	  damage += vars->igrid[t][i][j];
      }
      gx = x / BOX_WIDTH;
      gy = y / BOX_HEIGHT;
      if (map_landmark(vars->map, gx, gy) == SLOW)
	damage += (1.0 - settings.box_slowdown) * 100;
      x += vx;
      y += vy;
      if ((gx != x / BOX_WIDTH && 
	   ((vx < 0 && map_wall(vars->map, WEST, gx, gy)) ||
	    (vx > 0 && map_wall(vars->map, EAST, gx, gy)))) ||
	  (gy != y / BOX_HEIGHT &&
	   ((vy < 0 && map_wall(vars->map, NORTH, gx, gy)) ||
	    (vy > 0 && map_wall(vars->map, SOUTH, gx, gy))))) {  /* Hit wall */
	x -= vx;
	y -= vy;
	vx = -vx / 1.5;
	vy = -vy / 1.5;
	damage += 5 + settings.shocker_walls;
	hit_wall++;
      }
      vx += x_acc;
      vy += y_acc;
      if (brake) {
	if (SIGN(vx) == SIGN(x_acc))
	  vx = 0.0;
	if (SIGN(vy) == SIGN(y_acc))
	  vy = 0.0;
      }
      else {
	if (ABS(vx) > ABS(max_vx))
	  vx = max_vx;
	if (ABS(vy) > ABS(max_vy))
	  vy = max_vy;
      }
    }

    if (min_try == -1 || damage < min_damage) {
      min_try = try;
      min_damage = damage;
      if (damage == 0)
	break;
    }
  }

  /* If we're really hurting and all dodges are dangerous, just stop and
     protect sides. */

  if (settings.game != RACE_GAME && min_try == -1 && 
      vars->movement == Evading) {
    protect_sides(vars);
    return;
  }

  /* Take the evading angle. */

  if (min_try == -1) {
#ifdef DEBUG_GNAT
    sprintf(text, "%d: Can't dodge (%d)", frame_number(), try);
    send_msg(RECIPIENT_ALL, OP_TEXT, text);
#endif
  }
  else if (min_try == MaxDodges || (min_try == 3 && max_tries == 3))
    set_abs_drive(vars->drive = 0.0);
  else if (min_try >= 0) {
    angle = fixed_angle(opt_angle + DegToRad(dodge_angles[min_try]));
    if (vars->movement == Avoiding)
      vx = 6.0;
    else
      vx = vars->max_speed;
    if (ADiff(angle - fixed_angle(heading())) > QUAR_CIRCLE) {
      turn_vehicle(angle + HALF_CIRCLE);
      set_abs_drive(vars->drive = -vx);
    }
    else {
      turn_vehicle(angle);
      set_abs_drive(vars->drive = vx);
    }
  }
}

/*** Initialize the intercept grid and boundaries. ***/

static void init_igrid(vars)
  GnatVars *vars;
{
  int i, x, y;
  int f, eti;
  float d;

  /* Initialize intercept grid. */

  f = (vars->us.xspeed + vars->max_speed) / vars->accel;
  eti = IEti - f;
  d = f * vars->us.xspeed - f * f * vars->accel / 2.0;
  if (eti > 0)
    d -= eti * vars->max_speed;
  x = - ((int) d - vars->vsize / 2);
  vars->west_x = x / vars->vsize;
  for (i = 0; i < (BOX_WIDTH + x - vars->us.loc.box_x) / BOX_WIDTH; i++)
    if (map_wall(vars->map, WEST, vars->us.loc.grid_x - i,
		 vars->us.loc.grid_y) == MAP_WALL) {
      vars->west_x = MIN(vars->us.loc.box_x + i*BOX_WIDTH, x) / vars->vsize;
      break;
    }

  f = (vars->max_speed - vars->us.xspeed) / vars->accel;
  eti = IEti - f;
  d = f * vars->us.xspeed + f * f * vars->accel / 2.0;
  if (eti > 0)
    d += eti * vars->max_speed;
  x = (int) d + vars->vsize / 2;
  vars->east_x = x / vars->vsize;
  for (i = 0; i < (x + vars->us.loc.box_x) / BOX_WIDTH; i++)
    if (map_wall(vars->map, EAST, vars->us.loc.grid_x + i,
		 vars->us.loc.grid_y) == MAP_WALL) {
      vars->east_x = MIN(i*BOX_WIDTH - vars->us.loc.box_x, x) / vars->vsize;
      break;
    }

  f = (vars->us.yspeed + vars->max_speed) / vars->accel;
  eti = IEti - f;
  d = f * vars->us.yspeed - f * f * vars->accel / 2.0;
  if (eti > 0)
    d -= eti * vars->max_speed;
  y = -((int) d - vars->vsize / 2);
  vars->north_y = y / vars->vsize;
  for (i = 0; i < (BOX_HEIGHT + y - vars->us.loc.box_y) / BOX_HEIGHT; i++)
    if (map_wall(vars->map, NORTH, vars->us.loc.grid_x,
		 vars->us.loc.grid_y - i) == MAP_WALL) {
      vars->north_y = MIN(vars->us.loc.box_y + i*BOX_HEIGHT, y) / vars->vsize;
      break;
    }
	
  f = (vars->max_speed - vars->us.yspeed) / vars->accel;
  eti = IEti - f;
  d = f * vars->us.yspeed + f * f * vars->accel / 2.0;
  if (eti > 0)
    d += eti * vars->max_speed;
  y = (int) d + vars->vsize / 2;
  vars->south_y = y / vars->vsize;
  for (i = 0; i < (y + vars->us.loc.box_y) / BOX_HEIGHT; i++)
    if (map_wall(vars->map, SOUTH, vars->us.loc.grid_x,
		 vars->us.loc.grid_y + i) == MAP_WALL) {
      vars->south_y = MIN(i*BOX_HEIGHT - vars->us.loc.box_y, y)/vars->vsize;
      break;
    }

  vars->east_x = MAX(1, vars->east_x);
  vars->west_x = MAX(1, vars->west_x);
  vars->north_y = MAX(1, vars->north_y);
  vars->south_y = MAX(1, vars->south_y);

  if (vars->east_x + vars->west_x + 1 >= MaxISize) {
    fprintf(stderr, "**** EAST + WEST >= MaxISize ****\n");
    fflush(stderr);
    send_msg(RECIPIENT_ALL, OP_TEXT, "EAST + WEST >= MaxISize");
  }
  if (vars->south_y + vars->north_y + 1 >= MaxISize) {
    fprintf(stderr, "**** SOUTH + NORTH >= MaxISize ****\n");
    fflush(stderr);
    send_msg(RECIPIENT_ALL, OP_TEXT, "SOUTH + NORTH >= MaxISize");
  }

  for (i = 0; i < IEti; i++)
    for (x = 0; x < vars->east_x + vars->west_x + 1; x++)
      for (y = 0; y < vars->south_y + vars->north_y + 1; y++)
	vars->igrid[i][x][y] = 0;

  vars->ix0 = vars->us.loc.x - (vars->west_x+1)*vars->vsize - vars->vsize/2;
  vars->ix1 = vars->us.loc.x + vars->east_x*vars->vsize + vars->vsize/2;
  vars->iy0 = vars->us.loc.y - (vars->north_y+1)*vars->vsize - vars->vsize/2;
  vars->iy1 = vars->us.loc.y + vars->south_y*vars->vsize + vars->vsize/2;
}

/*** Add an intercepting vector to the intercept grid. ***/

static Boolean add_ivector(vars, x, y, vx, vy, dvx, dvy, damage, allowed_time)
  GnatVars *vars;
  int x, y;
  float vx, vy;
  float dvx, dvy;   /* Only tanks will have acceleration. */
  int damage;
  int allowed_time; /* If < 0, its a tank. */
{
  int i, j;
  int dx, dy;
  int dx0, dy0, dx1, dy1;
  int lx0, ly0, lx1, ly1;
  int t, start_time, max_time;
  float max_v;
  Angle vel_angle, lim1, lim2, lim_diff;
  Boolean in_grid, within_x, within_y;
  Boolean is_tank;
  Location loc0, loc1;

  if (allowed_time < 0) {
    is_tank = True;
    max_time = IEti;
  }
  else {
    is_tank = False;
    max_time = MIN(allowed_time, IEti);
  }

  /* See if this bullet can possibly pass through intercept grid. */

  in_grid = within_x = within_y = False;
  if (x >= vars->ix0 && x <= vars->ix1) {
    within_x = True;
    if (y >= vars->iy0 && y <= vars->iy1) {
      in_grid = True;
      within_y = True;
    }
    else if (y < vars->iy0) {
      lx0 = vars->ix0;
      ly0 = vars->iy0;
      lx1 = vars->ix1;
      ly1 = vars->iy0;
    }
    else {
      lx0 = vars->ix0;
      ly0 = vars->iy1;
      lx1 = vars->ix1;
      ly1 = vars->iy1;
    }
  }
  else if (x < vars->ix0) {
    if (y < vars->iy0) {
      lx0 = vars->ix0;
      ly0 = vars->iy1;
      lx1 = vars->ix1;
      ly1 = vars->iy0;
    }
    else if (y > vars->iy1) {
      lx0 = vars->ix0;
      ly0 = vars->iy0;
      lx1 = vars->ix1;
      ly1 = vars->iy1;
    }
    else {
      within_y = True;
      lx0 = vars->ix0;
      ly0 = vars->iy0;
      lx1 = vars->ix0;
      ly1 = vars->iy1;
    }
  }
  else {
    if (y < vars->iy0) {
      lx0 = vars->ix0;
      ly0 = vars->iy0;
      lx1 = vars->ix1;
      ly1 = vars->iy1;
    }
    else if (y > vars->iy1) {
      lx0 = vars->ix0;
      ly0 = vars->iy1;
      lx1 = vars->ix1;
      ly1 = vars->iy0;
    }
    else {
      within_y = True;
      lx0 = vars->ix1;
      ly0 = vars->iy0;
      lx1 = vars->ix1;
      ly1 = vars->iy1;
    }
  }

  if (!in_grid) {

    if (vx == 0.0 && vy == 0.0)
      return(False);

    /* Make sure the trajectory crosses our intercept grid. */

    vel_angle = TAN((vy + dvy), (vx + dvx));  /* Will improve this in future */
    dx0 = lx0 - x;
    dy0 = ly0 - y;
    dx1 = lx1 - x;
    dy1 = ly1 - y;
    lim1 = TAN(dy0, dx0);
    lim2 = TAN(dy1, dx1);
    lim_diff = ADiff(lim1 - lim2);
    if (ADiff(vel_angle - lim1) >= lim_diff || 
	ADiff(vel_angle - lim2) >= lim_diff)
      return(False);

    /* Make sure the bullet has the range to reach our grid. */

    if (within_x)
      t = MIN(ABS(dy0), ABS(dy1)) / ABS(vy);
    else if (within_y)
      t = MIN(ABS(dx0), ABS(dx1)) / ABS(vx);
    else
      t = MAX(MIN(ABS(dx0),ABS(dx1))/ABS(vx), MIN(ABS(dy0),ABS(dy1))/ABS(vy));

    if (t > max_time)
      return(False);

    /* Make sure there are no walls blocking the bullet. */

    loc0.x = x;
    loc0.y = y;
    loc0.grid_x = x / BOX_WIDTH;
    loc0.grid_y = y / BOX_HEIGHT;
    loc1.x = x + t * vx;
    loc1.y = y + t * vy;
    loc1.grid_x = loc1.x / BOX_WIDTH;
    loc1.grid_y = loc1.y / BOX_HEIGHT;
    if (!ClearPath(loc0, loc1))
      return(False);

    start_time = t;  /* Start bullet tracking when it hits intercept grid. */
    x = loc1.x;
    y = loc1.y;
  }
  else
    start_time = 0;

  /* Check where the bullet goes. */

  max_v = MAX(20.0, MAX(ABS(vx), ABS(vy)));
  for (t = 0; t < max_time - start_time; t++) {
    dx = x - vars->us.loc.x;
    dy = y - vars->us.loc.y;
    i = vars->west_x + (dx + SIGN(dx) * vars->vsize / 2) / vars->vsize;
    j = vars->north_y + (dy + SIGN(dy) * vars->vsize / 2) / vars->vsize;
    if (i >= 0 && i <= vars->east_x + vars->west_x &&
	j >= 0 && j <= vars->north_y + vars->south_y) {
      in_grid = True;
      if (is_tank)
	vars->igrid[t + start_time][i][j] = InfiniteNumber;
      else if (vars->igrid[t + start_time][i][j] == InfiniteNumber)
	break;   /* Bullet will hit this tank. */
      else
	vars->igrid[t + start_time][i][j] += damage;
    }
    else if (in_grid)  /* Bullet has passed through grid. */
      break;
    x += vx;
    y += vy;
    vx += dvx;
    vy += dvy;
    if (is_tank) {
      if (ABS(vx) > max_v)
	vx = SIGN(vx) * max_v;
      if (ABS(vy) > max_v)
	vy = SIGN(vy) * max_v;
    }
  }
  
  return(True);
}

/*** Process battlefield info: Look for possible intercepting bullets. */

static void process_bullets(vars)
  GnatVars *vars;
{
  int i, fnum, id,  damage;
  int t, dx, dy, dist;
  int num_hits;
  Boolean old_bullet;
  float bvx, bvy, bspeed;

  vars->seeker_on_tail = False;
  vars->mine_ahead = False;
  vars->possible_damage = 0;

  /* Process each bullet that can pass through the intercept grid. */

  fnum = frame_number();

  num_hits = 0;
  for (i = 0; i < vars->num_bullets; i++) {

    vars->ivects[i].hits = False;

    /* Don't worry about mines if we're hovercraft. */

    if (vars->hover && vars->bullets[i].type == MINE)
      continue;

    /* See if we've seen this within last 10 frames. */

    id = vars->bullets[i].id;
    old_bullet = (fnum - vars->b_fnum[id] < 10);
    vars->b_fnum[id] = fnum;
    vars->ivects[i].dist = InfiniteNumber;

    if (old_bullet && vars->b_ours[id])
      continue;

    if (vars->bullets[i].type == MINE)
      damage = 1000;
    else
      damage = weapon_stat[vars->bullets[i].type].damage - vars->protection;
    vars->ivects[i].damage = damage;

    /* Compute current bullet statistics. */

    if (damage > 0) {

      dx = vars->bullets[i].loc.x - vars->us.loc.x;
      dy = vars->bullets[i].loc.y - vars->us.loc.y;
      vars->ivects[i].dx = dx;
      vars->ivects[i].dy = dy;
      vars->ivects[i].dist = SQRT(dx * dx + dy * dy);

      /* Try to rule out our own bullets. */

      if (!vars->visible_enemies && !vars->outposts && !vars->taking_damage &&
	  vars->bullets[i].type != MINE && vars->ivects[i].dist < BOX_WIDTH) {
	vars->b_ours[id] = True;
	continue;
      }

      /* Rule out bullets which are heading in wrong direction. */
      
      bvx = vars->bullets[i].xspeed - vars->us.xspeed;
      bvy = vars->bullets[i].yspeed - vars->us.yspeed;
      if ((SIGN(bvx) == SIGN(dx) && (ABS(dx) > 200 || ABS(bvx) > 5.0)) ||
	  (SIGN(bvy) == SIGN(dy) && (ABS(dy) > 200 || ABS(bvy) > 5.0)))
	continue;
      bspeed = HYPOT(bvx, bvy) + 2.0;

      if (!old_bullet) {
	vars->b_orig_fnum[id] = fnum;
	if (vars->ivects[i].dist < vars->vsize && 
	    vars->bullets[i].type != MINE) {
	  vars->b_ours[id] = True;
	  continue;
	}
	else
	  vars->b_ours[id] = False;
      }
      t = weapon_stat[vars->bullets[i].type].frames;
      if (old_bullet)
	t -= fnum - vars->b_orig_fnum[id];  /* how long it's existed. */

      if (vars->bullets[i].type != MINE &&
	  t < vars->ivects[i].dist / bspeed)  /* t to int. */
	continue;

      vars->ivects[i].frames = t;
      vars->ivects[i].hits = True;
      if (vars->bullets[i].type != MINE) {
	num_hits++;
	vars->possible_damage += damage;
      }
      else
	vars->mine_ahead = True;
      if (vars->bullets[i].type == SEEKER)
	vars->seeker_on_tail = True;
    }
  }

  /* Process all mines. */

  if (vars->mine_ahead) {
    vars->mine_ahead = False;
    for (i = 0; i < vars->num_bullets; i++)
      if (vars->ivects[i].hits && vars->bullets[i].type == MINE &&
	  add_ivector(vars, vars->bullets[i].loc.x, vars->bullets[i].loc.y,
		      vars->bullets[i].xspeed, vars->bullets[i].yspeed,
		      0.0, 0.0, vars->ivects[i].damage, 
		      vars->ivects[i].frames)) {
	vars->mine_ahead = True;
	vars->num_ivects++;
      }
  }

  /* Take upto NumDBullets intercepting bullets ranked on distance. */

  if (NumDBullets && num_hits)
    for (dist = 750; dist >= 200; dist -= 150) {
      t = 0;
      for (i = 0; i < vars->num_bullets; i++)
	if (vars->ivects[i].hits && vars->ivects[i].dist <= dist &&
	    vars->bullets[i].type != MINE)
	  t++;
	else
	  vars->ivects[i].hits = False;
      if (t <= NumDBullets) {
	for (i = 0; i < vars->num_bullets; i++)
	  if (vars->ivects[i].hits && 
	      add_ivector(vars, vars->bullets[i].loc.x, vars->bullets[i].loc.y,
			  vars->bullets[i].xspeed, vars->bullets[i].yspeed,
			  0.0, 0.0, vars->ivects[i].damage, 
			  vars->ivects[i].frames))
	    vars->num_ivects++;
	  else
	    vars->ivects[i].hits = False;
	break;
      }
    }
}

/*** Process battlefield info: Look for visible/intercepting tanks. */

static void process_tanks(vars)
  GnatVars *vars;
{
  int i, j, dx, dy, dist;
  int fnum, damage;
  int deg, angle_deg, safety_deg;
  int enemy, friend;
  float vspeed, rspeed, eti;
  float dvx, dvy;
  Angle delta, pos_angle, vel_angle, avoid_angle;
  Boolean line_of_sight, hostile, see_enemy;
  Location loc;

  /* Deal with friend-related arrays. */

  for (deg = 0; deg < 90; deg++)
    vars->friend_dist[deg] = InfiniteNumber;
  for (i = 0; i < vars->num_friends; i++)
    vars->friends[i].in_sight = False;

  /* Initialize our active enemy list. */

  for (i = 0; i < vars->num_enemies; i++)
    vars->enemies[i].in_sight = False;
  for (deg = 0; deg < 24; deg++)
    vars->enemy_dist[deg] = InfiniteNumber;
  vars->default_enemy = -1;

  /* Process each tank for hostility and possibility of interception. */

  fnum = frame_number();
  vars->boogie_too_fast = False;
  vars->closest_enemy = InfiniteNumber;
  vars->visible_enemies = vars->visible_friends = vars->close_friends = 0;
  vars->enemy_los = 0;

  for (i = 0; i < vars->num_tanks; i++) {
    line_of_sight = ClearPath(vars->us.loc, vars->tanks[i].loc);
    hostile = (vars->tanks[i].team != vars->us.team || vars->us.team == 0);
    dx = vars->tanks[i].loc.x - vars->us.loc.x;
    dy = vars->tanks[i].loc.y - vars->us.loc.y;
    dist = HYPOT(dx, dy);
    pos_angle = TAN(dy, dx);

    /* Determine tank's and overall velocity relative to our position. */

    vel_angle = TAN(vars->tanks[i].yspeed - vars->us.yspeed, 
		    vars->tanks[i].xspeed - vars->us.xspeed);
    delta = fixed_angle(pos_angle + HALF_CIRCLE) - vel_angle;
    rspeed = -COS(ADiff(delta)) * 
      HYPOT(vars->tanks[i].yspeed - vars->us.yspeed, 
	    vars->tanks[i].xspeed - vars->us.xspeed);

    if (vars->us.xspeed != 0.0 || vars->us.yspeed != 0.0) {
      vel_angle = TAN(vars->tanks[i].yspeed, vars->tanks[i].xspeed);
      delta = fixed_angle(pos_angle + HALF_CIRCLE) - vel_angle;
      vspeed = -COS(ADiff(delta)) * 
	HYPOT(vars->tanks[i].xspeed, vars->tanks[i].yspeed);
    }
    else
      vspeed = rspeed;

    /* Look for enemies. */

    if (hostile) {
      vars->visible_enemies++;
      enemy = -1;
      for (j = 0; j < vars->num_enemies; j++)
	if (vars->tanks[i].id == vars->enemies[j].id) {
	  enemy = j;
	  break;
	}
      if (enemy == -1) {    /* New enemy. */
	enemy = vars->num_enemies++;
	vars->enemies[enemy].id = vars->tanks[i].id;
	vars->enemies[enemy].fnum = -100;
      }

      vars->enemies[enemy].dist = dist;
      vars->enemies[enemy].loc = vars->tanks[i].loc;
      j = fnum - vars->enemies[enemy].fnum;
      if (j < 10) {   /* Update tank info. */
	dvx = (vars->tanks[i].xspeed - vars->enemies[enemy].vx) / j;
	vars->enemies[enemy].ddvx = dvx - vars->enemies[enemy].dvx;
	vars->enemies[enemy].dvx = dvx;
	dvy = (vars->tanks[i].yspeed - vars->enemies[enemy].vy) / j;
	vars->enemies[enemy].ddvy = dvy - vars->enemies[enemy].dvy;
	vars->enemies[enemy].dvy = dvy;
      }
      else {
	dvx = dvy = 0.0;
	vars->enemies[enemy].dvx = vars->enemies[enemy].dvy = 0.0;
	vars->enemies[enemy].ddvx = vars->enemies[enemy].ddvy = 0.0;
      }
      vars->enemies[enemy].vsize = MAX(vars->tanks[i].bwidth,
				       vars->tanks[i].bheight);
      vars->enemies[enemy].vx = vars->tanks[i].xspeed;
      vars->enemies[enemy].vy = vars->tanks[i].yspeed;
      vars->enemies[enemy].fnum = fnum;
      vars->enemies[enemy].in_sight = True;
      vars->enemies[enemy].pos_angle = pos_angle;
      vars->enemies[enemy].vspeed = vspeed;
      vars->enemies[enemy].rspeed = rspeed;
      if (line_of_sight) {
	angle_deg = (RadTo15Deg(pos_angle) - 3) % 24;
	for (deg = 0; deg < 7; deg++) {
	  if (dist < vars->enemy_dist[angle_deg])
	    vars->enemy_dist[angle_deg] = dist;
	  angle_deg = (angle_deg + 1) % 24;
	}
	if (rspeed < vars->speed - vars->max_speed)
	  vars->boogie_too_fast = True;
	vars->enemy_los++;
	vars->enemies[enemy].los = True;
	if (dist < vars->closest_enemy) {
	  vars->closest_enemy = dist;
	  vars->default_enemy = enemy;
	}
      }
      else
	vars->enemies[enemy].los = False;
    }

    /* Handle friends. */

    else {

      /* Register the exact position of this friend. */

      if (vars->vtof[vars->tanks[i].id] >= 0) {
	friend = vars->vtof[vars->tanks[i].id];
	if (vars->friends[friend].condition >= Operational)
	  vars->visible_friends++;
      }
      else {      /* Must be non-gnat since we've received no messages. */
	friend = vars->num_friends++;
	vars->friends[friend].id = vars->tanks[i].id;
	vars->vtof[vars->tanks[i].id] = friend;
	vars->visible_friends++;
	vars->friends[friend].condition = Operational;   /* Assume this. */
      }
      if (dist <= MAX(BOX_WIDTH / 2, BOX_HEIGHT / 2))
	vars->close_friends++;
      vars->friends[friend].vsize = MAX(vars->tanks[i].bwidth,
					vars->tanks[i].bheight);
      vars->friends[friend].loc = vars->tanks[i].loc;
      vars->friends[friend].vspeed = vspeed;
      vars->friends[friend].rspeed = rspeed;
      vars->friends[friend].in_sight = True;
      j = fnum - vars->friends[friend].fnum;
      if (j < 10) {   /* Update tank info. */
	dvx = (vars->tanks[i].xspeed - vars->friends[friend].vx) / j;
	dvy = (vars->tanks[i].yspeed - vars->friends[friend].vy) / j;
	vars->friends[friend].dvx = dvx;
	vars->friends[friend].dvy = dvy;
      }
      else {
	dvx = dvy = 0.0;
	vars->friends[friend].dvx = vars->friends[friend].dvy = 0.0;
      }
      vars->friends[friend].vx = vars->tanks[i].xspeed;
      vars->friends[friend].vy = vars->tanks[i].yspeed;
      vars->friends[friend].fnum = fnum;

      /* Add this friend to the friendly fire array. */

      angle_deg = RadTo4Deg(pos_angle);
      if (delta < 0) {    /* Friend is travelling clock-wise to us. */
	safety_deg = RadTo4Deg(QUAR_CIRCLE - 
	     TAN(dist, vars->friends[friend].vsize + 10));
	angle_deg += safety_deg;
	angle_deg %= 90;
	safety_deg *= 2;
	safety_deg += RadTo4Deg(QUAR_CIRCLE - TAN(dist, 3 * rspeed));
	for (deg = 0; deg <= safety_deg; deg++) {
	  if (vars->friend_dist[angle_deg] > dist)
	    vars->friend_dist[angle_deg] = dist;
	  angle_deg = (angle_deg - 1) % 90;
	}
      }
      else {
	safety_deg = RadTo4Deg(QUAR_CIRCLE - 
             TAN(dist, vars->friends[friend].vsize + 10));
	angle_deg -= safety_deg;
	angle_deg %= 90;
	safety_deg *= 2;
	safety_deg += RadTo4Deg(QUAR_CIRCLE - TAN(dist, 3 * rspeed));
	for (deg = 0; deg <= safety_deg; deg++) {
	  if (vars->friend_dist[angle_deg] > dist)
	    vars->friend_dist[angle_deg] = dist;
	  angle_deg = (angle_deg + 1) % 90;
	}
      }
    } /* End of friend processing. */

    /* Look for vehicles which might be on an intercept course. */

    if (add_ivector(vars, vars->tanks[i].loc.x, vars->tanks[i].loc.y,
		    vars->tanks[i].xspeed, vars->tanks[i].yspeed, dvx, dvy,
		    hostile, -1) && 
	hostile && action_type[vars->action] != Loading)
      vars->num_ivects++;

  }  /* Iterate through all tanks. */

  if (vars->default_enemy == -1)
    for (i = 0; i < vars->num_enemies; i++)
      if (vars->enemies[i].in_sight && (vars->default_enemy == -1 ||
				      vars->enemies[i].dist < j)) {
	vars->default_enemy = i;
	j = vars->enemies[i].dist;
      }

  if (vars->visible_enemies)
    vars->fnum_enemy_sighting = fnum;
}

/*** Check battlefield for hostile or friendly objects. ***/

static void check_battlefield(vars)
  GnatVars *vars;
{
  int i, j, x, y, e, fnum;

  vars->seeker_on_tail = False;
  fnum = frame_number();

  /* Check battlefield for tanks. */

  vars->num_ivects = 0;
  init_igrid(vars);
  get_vehicles(&(vars->num_tanks), vars->tanks);
  process_tanks(vars);

  if (settings.game != RACE_GAME) {

    /* Count recently seen boogies. */
    
    vars->close_enemies = 0;
    for (e = 0; e < vars->num_enemies; e++)
      if (vars->enemies[e].in_sight || fnum - vars->enemies[e].fnum < 30)
	vars->close_enemies++;
    
    /* Should we hit or run? */
    
    if (vars->armor_cond <= Unstable || vars->ammo_cond <= Unstable ||
	vars->fuel_cond == Critical ||
	(vars->close_enemies > 1 && vars->action != AidFriend &&
	 (vars->armor_cond != Maximum || vars->close_enemies >= 3)))
      vars->withdraw = True;
    else
      vars->withdraw = False;
    
    /* To ram or not to ram? */
    
    if (vars->rammer && vars->armor[FRONT] > vars->max_armor[FRONT] / 2 && 
	vars->armor[LEFT] > vars->max_armor[LEFT] / 2 &&
	vars->armor[RIGHT] > vars->max_armor[RIGHT] / 2 &&
	vars->ammo_cond > Critical && vars->enemy_los)
      vars->ram_him = True;
    else
      vars->ram_him = False;
    
    /* Process hot spots, etc. */
    
    if (vars->visible_enemies) {
      for (e = 0; e < vars->num_enemies; e++)
	if (vars->enemies[e].in_sight) {
	  i = vars->enemies[e].loc.grid_x;
	  j = vars->enemies[e].loc.grid_y;
	  vars->hot_spot[i][j] = fnum;
	  if (vars->path_ok && vars->withdraw && ABS(i - vars->dest.x) <= 2 &&
	      ABS(j - vars->dest.y) <= 2)
	    vars->path_ok = False;   /* Don't go to depot which is "hot". */
	}
    }
    else
      for (x = -1; x <= 1; x++)
	for (y = -1; y <= 1; y++) {
	  i = vars->us.loc.grid_x + x;
	  j = vars->us.loc.grid_y + y;
	  if (OnMaze(i,j))
	    vars->hot_spot[i][j] = -100;
	}

    /* Just do combat if we're relatively idle or have found victim. */

    if (vars->enemy_los && (vars->action < DoCombat ||
			    (vars->action == Pursue && vars->victim != -1 &&
			     vars->enemies[vars->victim].los))) { 
      vars->action = DoCombat;
      vars->path_ok = False;
    }
  }
  
  /* Look for intercepting bullets. */

  get_bullets(&(vars->num_bullets), vars->bullets);
  process_bullets(vars);

  /* Dodge. */

  if (vars->num_ivects && settings.game != RACE_GAME && 
      (vars->enemy_los < 2 || vars->possible_damage <= vars->min_side / 4) &&
      vars->have_ammo && vars->closest_enemy <= vars->range &&
      vars->max_side >= (vars->num_ivects * 6) &&
      (vars->critical_sides == 1 || vars->critical_sides == 2 ||
       (vars->min_side < MIN(20, vars->max_armor[vars->worst_side]) &&
	vars->critical_sides == 0)))
    protect_sides(vars);
  else if (vars->num_ivects && !vars->ram_him &&
	   (settings.game != RACE_GAME || vars->armor_cond != Maximum ||
	    vars->mine_ahead)) {
    if (settings.game != RACE_GAME && 
	(vars->possible_damage > vars->min_side / 4 ||
	 (vars->boogie_too_fast && vars->withdraw)))
      vars->movement = Panic;
    else
      vars->movement = Evading;
    dodge_ivectors(vars);
  }
  else if (vars->movement == Evading || vars->movement == LastStand)
    vars->movement = Default;
}

/*** Check if any external (scroll) force is keeping us from moving. ***/

static void check_for_scrolls(vars)
  GnatVars *vars;
{
  int i;
  LandmarkType type;
  Angle angle;
  WallSide move, dir[4];

  type = map_landmark(vars->map, vars->us.loc.grid_x, vars->us.loc.grid_y);
  if (type < SCROLL_N || type > SCROLL_NW)
    return;

  if (vars->hover || vars->speed > 1.0 || vars->drive == 0.0 ||
      !OnWall(vars->us.loc, vars->vsize + 10))
    return;
  
  switch(type) {
  case SCROLL_S:
    dir[0] = WEST;
    dir[1] = EAST;
    dir[2] = NORTH;
    dir[3] = SOUTH;
    break;
  case SCROLL_N:
    dir[0] = WEST;
    dir[1] = EAST;
    dir[2] = SOUTH;
    dir[3] = NORTH;
    break;
  case SCROLL_E:
    dir[0] = NORTH;
    dir[1] = SOUTH;
    dir[2] = WEST;
    dir[3] = EAST;
    break;
  case SCROLL_W:
    dir[0] = NORTH;
    dir[1] = SOUTH;
    dir[2] = EAST;
    dir[3] = WEST;
    break;
  case SCROLL_NW:
    dir[0] = SOUTH;
    dir[1] = EAST;
    dir[2] = NORTH;
    dir[3] = WEST;
    break;
  case SCROLL_NE:
    dir[0] = SOUTH;
    dir[1] = WEST;
    dir[2] = NORTH;
    dir[3] = EAST;
    break;
  case SCROLL_SW:
    dir[0] = NORTH;
    dir[1] = EAST;
    dir[2] = SOUTH;
    dir[3] = WEST;
    break;
  case SCROLL_SE:
    dir[0] = NORTH;
    dir[1] = WEST;
    dir[2] = SOUTH;
    dir[3] = EAST;
    break;
  }

  for (i = 0; i < 4; i++)
    if (map_wall(vars->map, dir[i], vars->us.loc.grid_x, 
		 vars->us.loc.grid_y) != MAP_WALL) {
      move = dir[i];
      break;
    }

  switch (move) {
  case NORTH:
    angle = HALF_CIRCLE + QUAR_CIRCLE;
    break;
  case SOUTH:
    angle = QUAR_CIRCLE;
    break;
  case WEST:
    angle = HALF_CIRCLE;
    break;
  case EAST:
    angle = 0;
    break;
  }

  turn_vehicle(angle);
  set_abs_drive(vars->drive = vars->max_speed);
  vars->movement = Avoiding;
}

/*** Heat seeker friendly fire prevention procedure. ***/

static Boolean bad_missile_shot(vars, missile_angle)
  GnatVars *vars;
  Angle missile_angle;
{
  int f, frame_num;
  int dx, dy;
  int dist;
  Angle pos_angle, safety_angle;
  Boolean bad_shot;

  bad_shot = False;
  frame_num = frame_number();
  for (f = 0; f < vars->num_friends; f++)
    if (frame_num - vars->friends[f].fnum < MaxDeadTime &&
	vars->friends[f].heat > 0) {
      if (vars->friends[f].in_sight) {
	dx = vars->friends[f].loc.x - vars->us.loc.x;
	dy = vars->friends[f].loc.y - vars->us.loc.y;
      }
      else {
	dx = CenterXofBox(vars->friends[f].loc.grid_x) - vars->us.loc.x;
	dy = CenterYofBox(vars->friends[f].loc.grid_y) - vars->us.loc.y;
      }
      dist = HYPOT(dy, dx);
      if (dist > 1250 + BOX_WIDTH)
	continue;
      pos_angle = TAN(dy, dx);
      safety_angle = 2 * (QUAR_CIRCLE - TAN(120, vars->friends[f].heat + 20));
      if (ADiff(pos_angle - missile_angle) < safety_angle) {
	bad_shot = True;
	break;
      }
    }
  return(bad_shot);
}

/*** Spot the nearest landmark of a given type. ***/

static found_depot(vars, type, flee)
  GnatVars *vars;
  LandmarkType type;
  Boolean flee;
{
  int i, j, dx, dy, dist;
  int best_depot, best_dist;
  int msign, fnum, cost;
  Coord *depots;
  int num_depots;
  Boolean eligible[MaxDepots];
  Boolean had_path;
  Boolean avoid_hotspots;

  if (flee)
    msign = -1;
  else
    msign = 1;
  avoid_hotspots = False;
  fnum = frame_number();

  if (vars->armor_cond <= Unstable || vars->ammo_cond <= Critical) {
    avoid_hotspots = True;
    for (dx = 0; dx < GRID_WIDTH; dx++)
      for (dy = 0; dy < GRID_HEIGHT; dy++)
	vars->bool_map[dx][dy] = False;
    for (i = 0; i < vars->num_enemies; i++)
      if (vars->enemies[i].in_sight) {
	dx = vars->enemies[i].loc.grid_x;
	dy = vars->enemies[i].loc.grid_y;
	vars->bool_map[dx][dy] = True;
      }
    for (i = 0; i < vars->num_blips; i++)
      vars->bool_map[vars->blips[i].x][vars->blips[i].y] = True;
    if (vars->taking_damage)
      vars->bool_map[vars->us.loc.grid_x][vars->us.loc.grid_y] = True;
  }

  switch(type) {
  case ARMOR:
    depots = vars->armor_depots;
    num_depots = vars->num_armor_depots;
    break;
  case AMMO:
    depots = vars->ammo_depots;
    num_depots = vars->num_ammo_depots;
    break;
  case FUEL:
    depots = vars->fuel_depots;
    num_depots = vars->num_fuel_depots;
    break;
  }
  had_path = push_path(vars);
  for (i = 0; i < num_depots; i++)
    if (vars->bool_map[depots[i].x][depots[i].y])
      eligible[i] = False;
    else
      eligible[i] = True;

  do {

    /* Check for closest/farthest eligible depot. */
    
    best_depot = -1;
    for (i = 0; i < num_depots; i++)
      if (eligible[i]) {
	dx = depots[i].x - vars->us.loc.grid_x;
	dy = depots[i].y - vars->us.loc.grid_y;
	dist = dx * dx + dy * dy;
	if (avoid_hotspots) {
	  if (vars->hot_spot[depots[i].x][depots[i].y] >= 0) {
	    j = fnum - vars->hot_spot[depots[i].x][depots[i].y];
	    cost = 200 - MIN(200, j);
	  }
	  else
	    cost = 0;
	  dist += msign * cost * cost / 4;
	}
	if (best_depot == -1 || msign * best_dist > msign * dist) {
	  best_dist = dist;
	  best_depot = i;
	}
      }

    if (best_depot == -1)
      break;


    if (found_fast_path(vars, depots[best_depot].x, depots[best_depot].y,
			vars->withdraw))
      return(True);
 
    eligible[best_depot] = False;

  } while (True);

  if (had_path)
    pop_path(vars);

  return(False);
}

/*** Load from a depot. ***/

static void load_from_depot(vars)
  GnatVars *vars;
{
  int dx, dy, dist;
  LandmarkType type;
  Angle angle;

  type = OnLandmark(vars);
  if (type != ARMOR && type != AMMO && type != FUEL) {
    rethink(vars);
    return;
  }

  /* Move to center of box if we have priority. */

  dx = CenterXofBox(vars->us.loc.grid_x) - vars->us.loc.x;
  dy = CenterYofBox(vars->us.loc.grid_y) - vars->us.loc.y;

  if (depot_priority(vars)) {

    /* Feast on the depot if we're in position. */

    if (ABS(dx) < LANDMARK_WIDTH / 2 && ABS(dy) < LANDMARK_HEIGHT / 2) {
      if (vars->drive != 0.0)
	set_abs_drive(vars->drive = 0.0);
      if (type == AMMO)
	toggle_ammo(vars);
      return;
    }

    /* Else move to loading position. */

    angle = TAN(dy, dx);
    if (vars->speed < vars->drive / 2 && vars->visible_friends)
      turn_vehicle(fixed_angle(angle + DegToRad(rand() % 90)));
    else
      turn_vehicle(angle);
    vars->move_heading = angle;
      
    /* Regulate speed to stop at edge of landmark. */

    dx -= SIGN(dx) * LANDMARK_WIDTH + vars->vsize;
    dy -= SIGN(dy) * LANDMARK_HEIGHT + vars->vsize;
    dist = SQRT(dx * dx + dy * dy);
    if (vars->speed != 0 && 
	dist/(vars->speed + 0.1) <= vars->speed/vars->decel) {
      vars->drive /= 2.0;
      set_abs_drive(vars->drive);
    }
    else 
      set_abs_drive(vars->drive = 6.0);
  }
  else {   

    /* Move to side to allow others to sit on depot. */

    if ((vars->us.loc.box_x <= vars->vsize + 10 ||
	 vars->us.loc.box_y >= BOX_WIDTH - vars->vsize - 10) &&
	(vars->us.loc.box_y <= vars->vsize + 10 ||
	 vars->us.loc.box_y >= BOX_HEIGHT - vars->vsize - 10))
      set_abs_drive(vars->drive = 0.0);
    else {
      angle = TAN(-dy, -dx);
      turn_vehicle(fixed_angle(angle + DegToRad(rand() % 90)));
      set_abs_drive(vars->drive = 4.0);
    }
  }
}

/*** Move tank using the precomputed gradient map. ***/

static void move_to_dest(vars)
  GnatVars *vars;
{
  int dx, dy;
  Angle angle, opt_angle;
  Boolean dest_wall;

  if (!vars->path_ok) {
    rethink(vars);
    return;
  }

  /* Find best heading and take the required action. */
  
  if (best_move_heading(vars, True, &opt_angle, &dest_wall)) {
    vars->move_heading = opt_angle;
    turn_vehicle(opt_angle);
    if (dest_wall) {
      if (vars->speed < 14.0)
	set_abs_drive(vars->drive = vars->max_speed);
      else
	set_abs_drive(vars->drive = MIN(14.0, vars->max_speed));
      vars->working_on_destwall = True;
    }
    else if (vars->hover) {
      angle = ADiff(opt_angle - fixed_angle(heading()));
      if (angle >= PI / 10)
	set_abs_drive(vars->drive = 0.0);
      else if (angle >= PI / 36)
	set_abs_drive(vars->drive = 6.0);
      else
	set_abs_drive(vars->drive = MIN(12.0, vars->max_speed));
    }
    else 
      set_abs_drive(vars->drive = vars->max_speed);

  }
  else {
#ifdef DEBUG_GNAT
    send_msg(RECIPIENT_ALL, OP_TEXT, "Moving off wall.");
#endif
    dx = CenterXofBox(vars->us.loc.grid_x) - vars->us.loc.x;
    dy = CenterYofBox(vars->us.loc.grid_y) - vars->us.loc.y;
    vars->move_heading = opt_angle = TAN(dy, dx);
    turn_vehicle(opt_angle);
    set_abs_drive(vars->drive = vars->max_speed);
    if (!OnWall(vars->us.loc, vars->vsize + 10)) {
#ifdef DEBUG_GNAT
      send_msg(RECIPIENT_ALL, OP_TEXT, "Could not move correctly!");
#endif
      rethink(vars);
    }
  }
}

/*** Explore the maze. ***/

static void set_to_explore(vars)
  GnatVars *vars;
{
  int x, y, dx, dy;
  int heading_x, heading_y;
  int score, best_score, min_x, min_y;
  Angle cur_heading;

  vars->action = NoAction;

  cur_heading = floor(fixed_angle(heading() + PI / 8.0) / (PI / 4.0));
  cur_heading *= PI / 4;
  heading_x = vars->us.loc.grid_x + 2 * SIGN(COS(cur_heading));
  heading_y = vars->us.loc.grid_y + 2 * SIGN(SIN(cur_heading));

  /* Pick the square we have seen the least. */

  min_x = -1;
  for (x = vars->us.loc.grid_x - 2; x <= vars->us.loc.grid_x + 2; x++)
    for (y = vars->us.loc.grid_y - 2; y <= vars->us.loc.grid_y + 2; y++)
      if ((x != vars->us.loc.grid_x || y != vars->us.loc.grid_y) &&
	  OnMaze(x,y) && vars->seen_box[x][y] < InfiniteNumber) {
	dx = ABS(x - heading_x);
	dy = ABS(y - heading_y);
	score = vars->seen_box[x][y] + dx + dy;
	if (min_x == -1 || score < best_score) {
	  best_score = score;
	  min_x = x;
	  min_y = y;
	}
      }
    
  if (min_x == -1) {
#ifdef DEBUG_GNAT
    send_msg(RECIPIENT_ALL, OP_TEXT, "Can't find explore square!");
#endif
  }
  else {

    /* Check if we can move to this square. */
      
    if (found_fast_path(vars, min_x, min_y, vars->withdraw)) {
      vars->action = Explore;
      return;
    }
    else if (vars->unmapped_boxes > 0) {
#ifdef DEBUG_GNAT
      sprintf(text, "Filling (%d,%d)", min_x, min_y);
      send_msg(RECIPIENT_ALL, OP_TEXT, text);
#endif
      set_abs_drive(vars->drive = 0.0);  /* This could be long. */
      fill_maze(vars, min_x, min_y); 
      set_to_explore(vars);   /* Recur to fill in unavailable areas. */
    }
#ifdef DEBUG_GNAT
    else {
      sprintf(text, "Couldn't explore (%d,%d)", min_x, min_y);
      send_msg(RECIPIENT_ALL, OP_TEXT, text);
    }
#endif
  }

  if (vars->action == NoAction)
    rethink(vars);
}

/*** See if we can pummel enemy from a distance. ***/

static Boolean can_mortar(vars)
  GnatVars *vars;
{
  int i;
  int dx, dy, dist;
  Angle angle;
  Location loc;
  LandmarkType type;
  Boolean had_path;
  Boolean scout_correct;

  if (vars->taking_damage || vars->visible_enemies)
    return(False);

  /* Will include spotting by friends in later versions.  Now just blips. */

  scout_correct = False;
  for (i = 0; i < vars->num_blips; i++) 
    if (vars->blips[i].x != vars->us.loc.grid_x || 
	vars->blips[i].y != vars->us.loc.grid_y) {
      type = map_landmark(vars->map, vars->blips[i].x, vars->blips[i].y);
      if (type == ARMOR || type == AMMO || type == FUEL) {
	loc.x = CenterXofBox(vars->blips[i].x);
	loc.y = CenterYofBox(vars->blips[i].y);
	loc.grid_x = vars->blips[i].x;
	loc.grid_y = vars->blips[i].y;
	dx = loc.x - vars->us.loc.x;
	dy = loc.y - vars->us.loc.y;
	dist = HYPOT(dx, dy);
	if (dist <= 1.75 * MIN(BOX_WIDTH, BOX_HEIGHT) && 
	    ClearPath(loc, vars->us.loc))  /* We can see this blip! */
	  continue;
	angle = TAN(dy, dx);
	if (vars->action == Scout && loc.grid_x == vars->dest.x &&
	    loc.grid_y == vars->dest.y)
	  scout_correct = True;
	if (dist <= vars->range + 200 && ClearPath(loc, vars->us.loc) &&
	    vars->friend_dist[RadTo4Deg(angle)] >= InfiniteNumber) {
	  vars->mortar_target = loc;
	  vars->mortar_angle = angle;
	  vars->mortar_dist = dist;
	  turn_vehicle(vars->move_heading = angle);
	  
	  vars->action = Mortar;
	  vars->path_ok = False;
	  return(True);
	}
	else if (vars->armor_cond > Critical && vars->action != Scout && 
		 vars->mortar_capable) {
	  had_path = push_path(vars);
	  if (found_fast_path(vars, loc.grid_x, loc.grid_y, True)) {
	    vars->action = Scout;
	    return(True);
	  }
	  pop_path(vars);
	}
      }
    }

  /* If we're scouting and there's nothing within radar range, 
     there's no need to go any further. */

  if (vars->action == Scout && vars->path_ok &&
      HYPOT(vars->dest.x - vars->us.loc.grid_x, 
	    vars->dest.y - vars->us.loc.grid_y) < RadarRange) {
    rethink(vars);
    for (i = 0; i < vars->num_armor_depots; i++)
      if (vars->armor_depots[i].x == vars->dest.x &&
	  vars->armor_depots[i].y == vars->dest.y) {
	vars->last_visited_armor[i] = frame_number();
	break;
      }
  }

  return(False);
}

/*** Lay in a course for enemy if one is there. ***/

static Boolean pursue_enemy(vars)
  GnatVars *vars;
{
  int i, j, dx, dy, dist, target_proximity;
  int min, min_dist;
  int tried[MAX_BLIPS], left, tries;
  int priority_x, priority_y;
  int target_x, target_y;
  float eti, close_speed;
  Angle angle;
  Boolean had_path;
  EnemyInfo *target;
  Location loc;
  LandmarkType type;

  had_path = push_path(vars);
  for (i = 0; i < vars->num_blips; i++)
    tried[i] = False;
  tries = vars->num_blips;

  /* Look for our previous victim. */

  if (vars->victim != -1 && 
      frame_number() - vars->enemies[vars->victim].fnum < 100) {

    target = vars->enemies + vars->victim;
    if (target->in_sight && !target->los) {
      if (found_fast_path(vars, target->loc.grid_x, target->loc.grid_y,
			  vars->withdraw)) {
	vars->action = Pursue;
	return(True);
      }
      else {
	rethink(vars);
#ifdef DEBUG_GNAT
	send_msg(RECIPIENT_ALL, OP_TEXT, "No action in pursue_enemy.");
#endif
	return(False);   /* When would this happen?  Can see but not reach? */
      }
    }
    if (target->in_sight) {    /* Must have line of sight. */
      vars->action = DoCombat;
      vars->path_ok = False;
      close_speed = MAX(5.0, vars->max_speed - target->rspeed);
      eti = MIN(10.0, (float) target->dist / close_speed);
      dx = target->loc.x - vars->us.loc.x;
      dx += eti * target->vx;
      dx += eti * eti * target->dvx / 2;
      dy = target->loc.y - vars->us.loc.y;
      dy += eti * target->vy;
      dy += eti * eti * target->dvy / 2;
      loc = target->loc;
      loc.x += dx;
      loc.y += dy;
      loc.grid_x = loc.x / BOX_WIDTH;
      loc.grid_y = loc.y / BOX_HEIGHT;
      if (!ClearPath(target->loc, loc))
	vars->move_heading = target->pos_angle;
      else
	vars->move_heading = TAN(dy, dx);
      if (vars->movement == Default) {
	turn_vehicle(vars->move_heading);
	if (target->dist > vars->range || vars->ram_him)
	  set_abs_drive(vars->drive = vars->max_speed);
	else if (target->dist < MAX(120, vars->range - 100))
	  set_abs_drive(vars->drive = -vars->max_speed);
	else
	  set_abs_drive(vars->drive = target->vspeed); /* Match their speed. */
      }
      return(True);
    }

    /* We have a victim which is no longer visible: make blip priority. */

    if (target->rspeed > 0) {  /* When last seen was moving away. */
      priority_x = (target->loc.x + 10 * target->vx) / BOX_WIDTH;
      priority_y = (target->loc.y + 10 * target->vy) / BOX_HEIGHT;
    }
    else {
      priority_x = target->loc.grid_x;
      priority_y = target->loc.grid_y;
    }
  }
  else
    priority_x = -1;

  /* Check the blips, selecting the closest one which we can get to. */

  for (left = tries; left > 0; left--) {
    min = -1;
    for (i = 0; i < vars->num_blips; i++)
      if (!tried[i]) {
	if (priority_x != -1) {
	  dx = vars->blips[i].x - priority_x;
	  dy = vars->blips[i].y - priority_y;
	  target_proximity = InfiniteNumber - dx * dx + dy * dy;
	}
	else
	  target_proximity = 0;

	dx = vars->blips[i].x - vars->us.loc.grid_x;
	dy = vars->blips[i].y - vars->us.loc.grid_y;
	dist = dx * dx + dy * dy - target_proximity;
	if (min == -1 || min_dist > dist) {
	  min_dist = dist;
	  min = i;
	}
      }

    if (min == -1)
      break;

    target_x = vars->blips[min].x;
    target_y = vars->blips[min].y;

    /* Try to find a path to target. */
    
    if (found_fast_path(vars, target_x, target_y, vars->withdraw)) {
      if (priority_x != -1)
	vars->action = Pursue;
      else
	vars->action = SeekEnemy;
      return(True);
    }

    tried[min] = True;    /* Try another blip. */
    priority_x = -1;     
  }

  /* Restore old gradient map to non-enemy destination if no enemy found. */

  if (had_path)
    pop_path(vars);

  return(False);
}

/*** Weapon targeting and pursuit procedure. ***/

static void fire_control(vars)
  GnatVars *vars;
{
  Location us;               /* Can actually vary with turret position. */
  Boolean targeting_outpost;

  EnemyInfo target, outposts[10];
  EnemyInfo *targets[MAX_VEHICLES];
  int num_targets;
  int target_to_enemy[MAX_VEHICLES];
  int target_num, priority, fired_on, turned_to;
  int fnum;
  Boolean same_victim;

  int wp, type, tnum, tmount;
  Angle angle, best_angle, safety_angle, bullet_angle;
  Boolean can_turn[MAX_TURRETS];
  Boolean can_fire[MAX_WEAPONS];
  Boolean did_fire[MAX_WEAPONS];

  float eti_tolerance;
  float eti_err[MAX_VEHICLES];
  Angle angle_err[MAX_VEHICLES];
  Angle aim_angle[MAX_VEHICLES];
  Boolean not_targeted[MAX_VEHICLES];
  Boolean type_ok, bad_spot;

  int i, j, dx, dy, tdx, tdy;
  int dist, min_dist;
  int best, deg;
  int total_heat;
  float t, eti, bvx, bvy, min_err;
  Location loc;

  if (vars->num_weapons == 0)
    return;

  /* Long-distance "mortar"-like fire control. */

  if (vars->action == Mortar) {
    turn_all_turrets(vars->mortar_angle);
    for (wp = 0; wp < vars->num_weapons; wp++)
      if (vars->weapons[wp].range >= vars->mortar_dist) {
	if (IS_TURRET(vars->weapons[wp].mount))
	  tmount = vars->weapons[wp].mount - MOUNT_TURRET1;
	else
	  tmount = -1;
      
	if (tmount >= 0) {
	  angle = fixed_angle(turret_angle(tmount));
	  if (ADiff(vars->mortar_angle - angle) > 0.01)
	    continue;
	}
	else if (vars->weapons[wp].mount != MOUNT_FRONT ||
		 ADiff(vars->mortar_angle - fixed_angle(heading())) > 0.01)
	  continue;

	discharge_weapon(vars, wp);
      }
    return;
  }

  /* Use weapons for destructable wall if we're trying to get through. */

  if (vars->working_on_destwall) {
    turn_all_turrets(heading());
    for (wp = 0; wp < vars->num_weapons; wp++)
      if (vars->weapons[wp].type != SEEKER && 
	  vars->weapons[wp].type != MINE &&
	  vars->weapons[wp].type != SLICK &&
	  vars->weapons[wp].type != UMISSLE) {
	if (IS_TURRET(vars->weapons[wp].mount))
	  tmount = vars->weapons[wp].mount - MOUNT_TURRET1;
	else
	  tmount = -1;
      
	if (tmount >= 0) {
	  angle = fixed_angle(turret_angle(tmount));
	  if (ADiff(angle - fixed_angle(heading())) > PI / 8.0)
	    continue;
	}
	else if (vars->weapons[wp].mount == MOUNT_FRONT) 
	  angle = heading();
	else
	  continue;

	if (vars->friend_dist[RadTo4Deg(angle)] >= InfiniteNumber)
	  discharge_weapon(vars, wp);
      }
    return;
  }

  /* If no enemy around but we are pursuing, try a preemptive firing. */

  fnum = frame_number();
  vars->outposts = 0;

  if (!vars->enemy_los && !vars->visible_enemies) {

    vars->ram_him = False;

    if (vars->action == SeekEnemy || vars->action == Pursue) {
      dx = vars->us.loc.grid_x - vars->dest.x;
      dy = vars->us.loc.grid_y - vars->dest.y;
      if (ABS(dx) <= 3 && ABS(dy) <= 3) {
	tdx = CenterXofBox(vars->dest.x) - vars->us.loc.x - 10*vars->us.xspeed;
	tdy = CenterYofBox(vars->dest.y) - vars->us.loc.y - 10*vars->us.yspeed;
	angle = TAN(tdy, tdx);
	dist = HYPOT(tdx, tdy);
	turn_all_turrets(angle);
	if (vars->num_friends == 0 && vars->ammo_cond >= Operational &&
	    ABS(dx) <= 2 && ABS(dy) <= 2 && vars->speed == vars->max_speed) {
	  
	  loc.x = CenterXofBox(vars->dest.x);
	  loc.y = CenterXofBox(vars->dest.y);
	  loc.grid_x = vars->dest.x;
	  loc.grid_y = vars->dest.y;
	  if (ClearPath(vars->us.loc, loc)) {

	    Boolean preemptive_fire;

	    preemptive_fire = True;
	    for (i = 0; i < vars->num_friends; i++) {
	      tdx = vars->us.loc.grid_x - vars->friends[i].loc.grid_x;
	      tdy = vars->us.loc.grid_y - vars->friends[i].loc.grid_y;
	      if (SIGN(tdx) == SIGN(dx) && SIGN(tdy) == SIGN(dy) &&
		  ABS(tdx) <= 2 && ABS(tdy) <= 2) {
		preemptive_fire = False;
		break;
	      }
	    }
	    if (preemptive_fire)
	      for (wp = 0; wp < vars->num_weapons; wp++) {
		if (vars->weapons[wp].type == SEEKER ||
		    vars->weapons[wp].type == MINE ||
		    vars->weapons[wp].type == SLICK ||
		    vars->ammo[wp] < vars->weapons[wp].max_ammo / 2 ||
		    dist / (vars->weapons[wp].ammo_speed + vars->speed) <
		    vars->weapons[wp].frames + 3)
		  continue;

		if (IS_TURRET(vars->weapons[wp].mount))
		  tmount = vars->weapons[wp].mount - MOUNT_TURRET1;
		else
		  tmount = -1;
      
		if (tmount >= 0) {
		  if (ADiff(fixed_angle(turret_angle(tmount)) - angle) > PI/4)
		    continue;
		}
		else if (vars->weapons[wp].mount != MOUNT_FRONT ||
			 ADiff(fixed_angle(heading()) - angle) > PI/4)
		  continue;

		discharge_weapon(vars, wp);
	      }
	  }
	}
	return;
      }
    }
  }

  /* If no enemy around, check for outposts. */

  targeting_outpost = False;
  if (!vars->enemy_los && 
      (!vars->visible_enemies || (vars->taking_damage &&
                                  vars->armor_cond < Operational))) {
    tnum = 0;
    for (dx = -2; dx <= 2; dx++)
      for (dy = -2; dy <= 2; dy++) {
	i = vars->us.loc.grid_x + dx;
	j = vars->us.loc.grid_y + dy;
	if (OnMaze(i,j) && map_landmark(vars->map, i, j) == OUTPOST) {
	  if (tnum == 10)
	    continue;
	  tdx = CenterXofBox(i) - vars->us.loc.x;
	  tdy = CenterYofBox(j) - vars->us.loc.y;
	  dist = HYPOT(tdx, tdy);
	  angle = TAN(tdy, tdx);
	  outposts[tnum].dist = dist;
	  outposts[tnum].loc.grid_x = i;
	  outposts[tnum].loc.grid_y = j;
	  outposts[tnum].loc.x = CenterXofBox(i);
	  outposts[tnum].loc.y = CenterYofBox(j);
	  outposts[tnum].pos_angle = angle;
	  outposts[tnum].rspeed = -COS(ADiff(angle - vars->vel_angle)) *
	    vars->speed;
	  outposts[tnum].vx = outposts[tnum].vy = 0.0;
	  outposts[tnum].dvx = outposts[tnum].dvy = 0.0;
	  outposts[tnum].ddvx = outposts[tnum].ddvy = 0.0;
	  outposts[tnum].in_sight = True;
	  tnum++;
	}
      }
    if (tnum) {
      vars->outposts = tnum;
      targeting_outpost = True;
    }
    if (!vars->taking_damage)
      targeting_outpost = False;
    if (!targeting_outpost) {
      if (settings.game == RACE_GAME)
	turn_all_turrets(heading());
      else {
	best = -1;
	for (i = 0; i < vars->num_blips; i++) {
	  dx = vars->blips[i].x - vars->us.loc.grid_x;
	  dy = vars->blips[i].y - vars->us.loc.grid_y;
	  dist = MIN(dx, dy);
	  if (i == 0 || dist < min_dist) {
	    min_dist = dist;
	    best = i;
	  }
	}
	if (best != -1) {
	  dx = vars->blips[best].x - vars->us.loc.grid_x;
	  dy = vars->blips[best].y - vars->us.loc.grid_y;
	  angle = TAN(dy, dx);
	  turn_all_turrets(angle);
	}
      }
      return;
    }

    num_targets = vars->outposts;
    for (tnum = 0; tnum < num_targets; tnum++)
      targets[tnum] = outposts + tnum;
    priority = ((fnum / 100) % vars->outposts);
  }
  else {
    priority = -1;
    num_targets = 0;
    for (tnum = 0; tnum < vars->num_enemies; tnum++)
      if (vars->enemies[tnum].in_sight) {
	if (vars->victim == tnum)
	  priority = num_targets;
        targets[num_targets] = vars->enemies + tnum;
	target_to_enemy[num_targets] = tnum;
	num_targets++;
      }
  }

  /* Allow turning of all turrets. */

  for (tnum = 0; tnum < MAX_TURRETS; tnum++)
    can_turn[tnum] = True;

  /* Initialize firing array. */

  for (wp = 0; wp < vars->num_weapons; wp++)
    did_fire[wp] = False;

  /* Cycle through weapon types and possible targets for targeting. */

  same_victim = False;
  fired_on = turned_to = -1;
  total_heat = vars->weapon_heat;

  for (type = 0; type < vars->num_wptypes; type++) {

    /* Allow targeting of all visible (or spotted) tanks or outposts. */

    for (tnum = 0; tnum < num_targets; tnum++) {
      not_targeted[tnum] = True;
      eti_err[tnum] = 10000.0;
      angle_err[tnum] = FULL_CIRCLE;
    }
    type_ok = False;
    if (IS_TURRET(vars->wptype_mount[type]))
      tmount = vars->wptype_mount[type] - MOUNT_TURRET1;
    else
      tmount = -1;

    while (True) {
   
      /* Select the victim. Our last target gets priority. */

      if (priority != -1 && not_targeted[priority])
	target_num = priority;
      else {
	target_num = -1;
	for (tnum = 0; tnum < num_targets; tnum++)
	  if (not_targeted[tnum] && 
	      (target_num == -1 || targets[tnum]->dist < min_dist)) {
	    target_num = tnum;
	    min_dist = targets[tnum]->dist;
	  }
      }
      
      /* If no target available, turn to last best target. */
      
      if (target_num == -1) {
	if (!type_ok) {
	  if (tmount >= 0 && can_turn[tmount]) {
	    dx = vars->enemies[vars->default_enemy].loc.x - vars->us.loc.x;
	    dx += 2 * vars->enemies[vars->default_enemy].vx;
	    dx += 2 * vars->enemies[vars->default_enemy].dvx;
	    dy = vars->enemies[vars->default_enemy].loc.y - vars->us.loc.y;
	    dy += 2 * vars->enemies[vars->default_enemy].vy;
	    dy += 2 * vars->enemies[vars->default_enemy].dvy;
	    angle = TAN(dy, dx);
	    turn_turret(tmount, angle);
	  }
	  break;
	}
	best = -1;
	min_err = FULL_CIRCLE;
	for (tnum = 0; tnum < num_targets; tnum++)
	  if (best == -1 || angle_err[tnum] < min_err) {
	    best = tnum;
	    min_err = angle_err[tnum];
	  }
	
	if (best != -1 && tmount >= 0 && can_turn[tmount]) {
	  turn_turret(tmount, aim_angle[best]);
	  turned_to = best;
	}
	break;
      }

      not_targeted[target_num] = False;
      target = *targets[target_num];

      /* Set our exact location. */
      
      us = vars->us.loc;
      if (tmount >= 0) {
	turret_position(vars->wptype_mount[type], &dx, &dy);
	us.x += dx;
	us.y += dy;
	us.grid_x = us.x / BOX_WIDTH;
	us.grid_y = us.y / BOX_HEIGHT;
      }

      /* Try to estimate time to intercept and location of vehicle. */

      dist = target.dist;

      /* Get Estimated Time to Intercept for this weapon's ammo. */
      
      t = vars->wptype_speed[type] - target.rspeed;
      if (t <= 0.0)
	continue;
      eti = (float) target.dist / t;
      if (eti > vars->wptype_range[type] + 3.0)
	continue;
      
      /* Predict target location based on estimated time to intercept. */

      loc.x = target.loc.x;
      if (ABS(target.dvx) > MAX_ACCEL) 
	loc.x += eti * target.vx;
      else {
	loc.x += eti * target.vx;
	loc.x += MIN(10, eti) * MIN(10, eti) * target.dvx / 2;
      }
      loc.grid_x = loc.x / BOX_WIDTH;
      
      loc.y = target.loc.y;
      if (ABS(target.dvy) > MAX_ACCEL)
	loc.y += eti * target.vy;
      else {
	loc.y += eti * target.vy;
	loc.y += MIN(10, eti) * MIN(10, eti) * target.dvy / 2;
      }
      loc.grid_y = loc.y / BOX_HEIGHT;

      if (!clear_tank_path(&loc, &(target.loc), 30))
	continue;

      /* Get intercept angle. */

      bvx = 2 * target.vx + 2 * target.loc.x / eti;
      bvx += eti * target.dvx - 2 * vars->us.loc.x / eti;
      bvy = 2 * target.vy + 2 * target.loc.y / eti;
      bvy += eti * target.dvy - 2 * vars->us.loc.y / eti;
      bullet_angle = best_angle = TAN(bvy, bvx);
      if (settings.rel_shoot) {
	bvx -= 2 * vars->us.xspeed;
	bvy -= 2 * vars->us.yspeed;
	best_angle = TAN(bvy, bvx);
      }
      aim_angle[target_num] = best_angle;

      /* Check range. */
      
      if (targeting_outpost)
	eti -= BOX_WIDTH / (2 * vars->wptype_speed[type]);
      else
	eti -= 30.0 / t;   /* Compensate for enemy tank size. */

      eti_err[target_num] = eti - vars->wptype_range[type];

      /* Determine by how much turret angle is off. */

      if (targeting_outpost)
	safety_angle = QUAR_CIRCLE - TAN(dist, 200.0);
      else
	safety_angle = QUAR_CIRCLE - TAN(dist, target.vsize);

      if (tmount >= 0) {
	angle = fixed_angle(turret_angle(tmount));
	angle_err[target_num] = ADiff(angle - best_angle);
	if (can_turn[tmount])
	  angle_err[target_num] = MAX(0, angle_err[target_num] -  
				      turret_turn_rate(tmount));
      }
      else {
	switch (vars->wptype_mount[type]) {
	case MOUNT_FRONT:
	  angle = 0.0;
	  break;
	case MOUNT_LEFT:
	  angle = -QUAR_CIRCLE;
	  break;
	case MOUNT_RIGHT:
	  angle = QUAR_CIRCLE;
	  break;
	case MOUNT_BACK:
	  angle = HALF_CIRCLE;
	  break;
	}
	angle += heading();
	angle = fixed_angle(angle);
	angle_err[target_num] = ADiff(angle - best_angle);
      }

      /* Don't fire and turn turret if bullet won't get there. */

      type_ok = True;
      if (vars->wptype_count[type] > vars->wptype_total[type] / 4) {
	eti_tolerance = 3.0;
	safety_angle += QUAR_CIRCLE / 3;
      }
      else 
	eti_tolerance = 1.0;

      if (!ClearPath(us, loc) || eti_err[target_num] > eti_tolerance ||
	  angle_err[target_num] > safety_angle)
	continue;
      
      /* Check for friendly fire. */

      deg = RadTo4Deg(bullet_angle);
      if (vars->friend_dist[deg] <= dist)
	continue;
      
      /* Go through the weapons that constitute this class type and see if
	 at least one of them can fire. */
    
      type_ok = False;
      for (wp = 0; wp < vars->num_weapons; wp++)
	if (!did_fire[wp] &&
	    vars->weapon_type[wp] == type && vars->weapon_ok[wp] &&
	    vars->ammo[wp] && total_heat + vars->weapons[wp].heat < 100 &&
	    !(vars->weapons[wp].type == SEEKER &&
	      (targeting_outpost || bad_missile_shot(vars, bullet_angle))) &&
	    !(vars->weapons[wp].type == MINE && targeting_outpost) &&
	    !(vars->seeker_on_tail && vars->weapons[wp].heat)) {
	  if ((vars->weapons[wp].type == MINE || 
	       vars->weapons[wp].type == SLICK) && 
	      ADiff(best_angle - vars->vel_angle) <= QUAR_CIRCLE)
	    can_fire[wp] = False;
	  else {
	    type_ok = True;
	    can_fire[wp] = True;
	    total_heat += vars->weapons[wp].heat;
	  }
	}
	else
	  can_fire[wp] = False;
      
      if (!type_ok) {
	type_ok = True;
	continue;
      }

      /* Rotate the turret to the desired heading. */
      
      if (tmount >= 0 && can_turn[tmount]) {
	turn_turret(tmount, best_angle);
	can_turn[tmount] = False;
	turned_to = target_num;
      }

      /* Fire away. */

      for (wp = 0; wp < vars->num_weapons; wp++)
	if (can_fire[wp]) {
	  discharge_weapon(vars, wp);
	  did_fire[wp] = True;
	  if (priority == target_num)
	    same_victim = True;
	  fired_on = target_num;
	}
    }
  }

  /* Hail mary weapon firing if no lock and we have ammo. */

  for (wp = 0; wp < vars->num_weapons; wp++)
    if (can_fire[wp] && vars->weapon_ok[wp] && !targeting_outpost &&
	total_heat + vars->weapons[wp].heat < 100 &&
	(vars->movement == LastStand ||
	 (vars->weapons[wp].type != SEEKER &&
	  (vars->ammo[wp] > vars->weapons[wp].max_ammo / 2 ||
	   (vars->ammo[wp] > vars->weapons[wp].max_ammo / 10 &&
	    vars->closest_enemy < 150)) &&
	  !(vars->seeker_on_tail && vars->weapons[wp].heat) &&
	  vars->weapons[wp].reload <= 4))) {

      if (IS_TURRET(vars->weapons[wp].mount))
	tmount = vars->weapons[wp].mount - MOUNT_TURRET1;
      else
	tmount = -1;
      
      if (tmount >= 0)
	angle = fixed_angle(turret_angle(tmount));
      else {
	switch (vars->weapons[wp].mount) {
	case MOUNT_FRONT:
	  angle = 0.0;
	  break;
	case MOUNT_LEFT:
	  angle = -QUAR_CIRCLE;
	  break;
	case MOUNT_RIGHT:
	  angle = QUAR_CIRCLE;
	  break;
	case MOUNT_BACK:
	  angle = HALF_CIRCLE;
	  break;
	}
	angle += heading();
	angle = fixed_angle(angle);
      }
  
      if (settings.rel_shoot) {
	bvx = COS(angle) * vars->weapons[wp].ammo_speed + vars->us.xspeed;
	bvy = SIN(angle) * vars->weapons[wp].ammo_speed + vars->us.yspeed;
	bullet_angle = TAN(bvy, bvx);
	t = HYPOT(bvx, bvy);
      }
      else {
	t = vars->weapons[wp].ammo_speed;
	bullet_angle = angle;
      }

      if ((vars->weapons[wp].type == MINE || 
	   vars->weapons[wp].type == SLICK) && 
	  ADiff(angle - vars->vel_angle) <= QUAR_CIRCLE)
	continue;
      deg = RadTo4Deg(bullet_angle);
      if (vars->friend_dist[deg] < InfiniteNumber)
	continue;
      deg = RadTo15Deg(bullet_angle);
      if (vars->enemy_dist[deg] < t * (vars->weapons[wp].frames + 3.0) ||
	  vars->closest_enemy < 200) {
	discharge_weapon(vars, wp);
	total_heat += vars->weapons[wp].heat;
      }
    }
  
  if (!targeting_outpost) {
    if (same_victim)
      vars->victim = target_to_enemy[priority];
    else if (fired_on != -1)
      vars->victim = target_to_enemy[fired_on];
    else if (turned_to != -1)
      vars->victim = target_to_enemy[turned_to];
  }
}

/*** COMBAT COMMAND CENTRAL. ***/

static void combat_control(vars)
  GnatVars *vars;
{
  int i, flee;
  int fnum;
  AidTypes aid;

  do {

    /* Go through the combat checklist. */

    fnum = frame_number();
    vars->money = get_money();
    vars->weapon_heat = heat();
    check_armor(vars);
    check_ammo(vars);
    check_fuel(vars);
    check_geography(vars);

    check_battlefield(vars);
    if (vars->movement == Panic)
      goto perform_action;

    check_for_scrolls(vars);
    flee = (vars->taking_damage && vars->enemy_los);

    if (action_type[vars->action] == Shelling &&
	(!vars->have_ammo || vars->visible_enemies || vars->taking_damage))
      rethink(vars);

    if ((vars->num_friends || fnum - vars->start_time < 100) && 
	!vars->enemy_los)
      check_friends(vars);

    check_radar(vars);
    
    /* Inform our teammates if anything has changed. */

    if (vars->us.team != 0) {
      if (fnum - vars->start_time < 20)
	send_to_friends(vars, CheckingIn, vars->team_code);
      else if (vars->num_friends) {
	if (fnum - vars->last_friend_update + 10 > MaxDeadTime ||
	    vars->visible_enemies || vars->pos_changed ||
	    (vars->num_blips && vars->withdraw))
	  send_to_friends(vars, FriendStatus, vars->team_code);
      }
    }
    
    /* Check for deadlock with friend. */

    if (vars->close_friends && vars->movement == Default && vars->path_ok) {
      vars->movement = Avoiding;
      dodge_ivectors(vars);
      goto perform_action;
    }

    /* If we're on a depot, see if we can use it. */

    if (!flee && action_type[vars->action] != Loading && 
	vars->action != AidFriend && depot_priority(vars))
      switch(OnLandmark(vars)) {
      case ARMOR:
	if (vars->armor_cond < Maximum && vars->money >= 5000) {
	  vars->action = LoadArmor;
	  goto perform_action;
	}
	break;
      case AMMO:
	if (vars->ammo_cond < Maximum && vars->money >= 50) {
	  vars->action = LoadAmmo;
	  goto perform_action;
	}
	break;
      case FUEL:
	if (vars->fuel_cond < Operational && vars->money >= 20) {
	  vars->action = LoadFuel;
	  goto perform_action;
	}
	break;
      }

    /* See if it's time to stop loading. */

    if (action_type[vars->action] == Loading) {
      if ((vars->num_ivects || vars->movement == Evading) &&
	  (vars->num_ivects > vars->visible_enemies ||
	   vars->action != LoadAmmo))
	rethink(vars);
      else
	switch (vars->action) {
	case Repair:
	  if ((!vars->repair_top && vars->armor_cond >= Operational) || flee)
	    rethink(vars);
	  else
	    goto perform_action;
	  break;
	case LoadArmor:
	  if (vars->armor_cond == Maximum || flee || vars->money < 1000 ||
	      (vars->close_friends >= 2 && 
		   vars->min_side >= 4*vars->max_armor[vars->worst_side]/5))
	    rethink(vars);
	  else
	    goto perform_action;
	  break;
	case LoadAmmo:
	  if (vars->ammo_cond == Maximum || vars->money < 50)
	    rethink(vars);
	  else
	    goto perform_action;
	  break;
	case LoadFuel:
	  if (vars->fuel_cond == Maximum || flee || vars->money < 20)
	    rethink(vars);
	  else
	    goto perform_action;
	  break;
	}
    }

    /* See if we should reevaluate our path to avoid enemies. */

    if ((vars->withdraw || vars->action == Scout || vars->action == Pursue ||
	 vars->action == SeekEnemy) && vars->path_ok &&
	vars->movement == Default && !vars->visible_enemies &&
	vars->num_blips && vars->fnum_path_computed - fnum > 24)
      vars->path_ok = False;

    /* Check if we need resources (critical readings). */

    if (vars->action != Mortar && 
	(action_type[vars->action] != Needing || !vars->path_ok)) {
      if (vars->armor_cond == Critical) {
	if (vars->money >= 5000 && found_depot(vars, ARMOR, flee)) {
	  vars->action = NeedArmor;
	  goto perform_action;
	}
	else if (has_special(REPAIR) && !vars->enemy_los && 
		 vars->fuel_cond != Critical) {
	  vars->action = Repair;
	  goto perform_action;
	}
      }
      if (vars->ammo_cond == Critical && vars->money >= 50 &&
	  found_depot(vars, AMMO, False)) {
	vars->action = NeedAmmo;
	goto perform_action;
      }
      if (vars->fuel_cond == Critical && vars->money >= 20 &&
	  found_depot(vars, FUEL, flee)) {
	vars->action = NeedFuel;
	goto perform_action;
      }
      if ((vars->armor_cond == Critical || vars->ammo_cond == Critical) &&
	  vars->money >= 5000 && vars->unmapped_boxes) {
	vars->action = Explore;
	vars->path_ok = False;
	goto perform_action;
      }
    }

    /* Check if we should seek resources (unstable readings). */

    if (action_type[vars->action] != Shelling && 
	(action_type[vars->action] < Seeking || !vars->path_ok)) {
      if (vars->armor_cond == Unstable) {
	if (vars->money >= 5000 && found_depot(vars, ARMOR, flee)) {
	  vars->action = SeekArmor;
	  goto perform_action;
	}
	else if (has_special(REPAIR) && !vars->enemy_los &&
		 vars->fuel_cond != Critical) {
	  vars->action = Repair;
	  goto perform_action;
	}
      }
      if (vars->ammo_cond == Unstable && vars->money >= 50 &&
	  found_depot(vars, AMMO, False)) {
	vars->action = SeekAmmo;
	goto perform_action;
      }
      if (vars->fuel_cond == Unstable && vars->money >= 20 &&
	  found_depot(vars, FUEL, flee)) {
	vars->action = SeekFuel;
	goto perform_action;
      }
      if (vars->repair_top && has_special(REPAIR) && !vars->enemy_los) {
	vars->action = Repair;   /* Quick top repair. */
	goto perform_action;
      }
      if ((vars->armor_cond == Unstable || vars->ammo_cond == Unstable) &&
	  vars->money >= 5000 && vars->unmapped_boxes) {
	vars->action = Explore;
	vars->path_ok = False;
	goto perform_action;
      }
    }

    /* See if we can strike from a distance. */

    if (vars->range > 200 && vars->have_ammo && can_mortar(vars))
      goto perform_action;
    else if (vars->action == Mortar)
      rethink(vars);

    /* Check if a teammate needs aid. */

    aid = NoAid;
    if (vars->us.team != 0 && vars->num_friends && 
	(action_type[vars->action] < Aiding ||
	 (vars->action == AidFriend && !vars->path_ok))) {

      aid = aid_friend(vars);
      if (aid == MustAid) {
	vars->action = AidFriend;
	goto perform_action;
      }
      else if (vars->action == AidFriend && !aid)
	rethink(vars);
    }

    /* If no immediate aid requested, try to engage enemies. */

    if ((action_type[vars->action] < Combat ||
	(action_type[vars->action] == Combat && !vars->path_ok)) &&
	vars->armor_cond >= Operational && vars->ammo_cond >= Operational &&
	pursue_enemy(vars))
      goto perform_action;

    /* If no enemies around, idly seek depots if we're not at maximum. */

    if (action_type[vars->action] <= Exploring || !vars->path_ok) {
      if (aid == Regroup) {
	vars->action = AidFriend;
	goto perform_action;
      }
      if (vars->armor_cond == Operational && vars->money >= 5000 &&
	  found_depot(vars, ARMOR, False)) {
	if (vars->fast_gradient[vars->us.loc.grid_x][vars->us.loc.grid_y] > 5)
	  pop_path(vars);
	else {
	  vars->action = CheckArmor;
	  goto perform_action;
	}
      }
      if (vars->ammo_cond == Operational && vars->money >= 50 &&
	  found_depot(vars, AMMO, False)) {
	if (vars->fast_gradient[vars->us.loc.grid_x][vars->us.loc.grid_y] > 5)
	  pop_path(vars);
	else {
	  vars->action = CheckAmmo;
	  goto perform_action;
	}
      }
    }

    /* Perform our current action. */

  perform_action:

    if (vars->action == NoAction || 
	(vars->action == Explore && !vars->path_ok))
      set_to_explore(vars);

#ifdef DEBUG_GNAT
    if (vars->path_ok)
      sprintf(text, "%d(%d): %s %s (%d,%d)", 
	      frame_number(), vars->num_ivects,
	      action_desc[vars->action], move_desc[vars->movement],
	      vars->dest.x, vars->dest.y);
    else
      sprintf(text, "%d(%d): %s %s", frame_number(), vars->num_ivects,
	      action_desc[vars->action], move_desc[vars->movement]);
    send_msg(RECIPIENT_ALL, OP_TEXT, text);
#endif

    switch (vars->movement) {
    case Default:
      if (vars->action == Mortar) {
	if (vars->mortar_dist <= vars->range)
	  set_abs_drive(vars->drive = 0.0);
	else if (vars->speed != 0 && vars->mortar_dist / (vars->speed+0.1) <= 
		 vars->speed / vars->decel) {
	  vars->drive /= 2.0;
	  set_abs_drive(vars->drive);
	}
	else 
	  set_abs_drive(vars->drive = vars->max_speed);
      }
      else if (action_type[vars->action] == Loading) {
	if (vars->action == Repair)
	  set_abs_drive(vars->drive = 0.0);
	else
	  load_from_depot(vars);
      }
      else if (vars->path_ok)
	move_to_dest(vars);
      else if (!vars->enemy_los || action_type[vars->action] != Combat)
	rethink(vars);
      break;
    case Evading:
    case LastStand:
      break;
    case Panic:
    case Avoiding:
      vars->movement = Default;
      break;
    }

    fire_control(vars);

    for (i = 0; i < vars->num_weapons; i++)
      vars->weapon_ok[i] = True;

    /* Done with this cycle. */

    if (vars->num_friends && 
	vars->friend_dist[RadTo4Deg(vars->vel_angle)] < 
	3 * vars->speed * vars->speed / vars->decel)
      set_abs_drive(vars->drive = 0.0);

    done();

#ifdef DEBUG_GNAT
    if ((i = frame_number() - fnum) > 2) {
      sprintf(text, "Lost %d cycles.", i - 1);
      send_msg(RECIPIENT_ALL, OP_TEXT, text);
    }
#endif

  } while (True);
}

/*** RACE COMMAND CENTRAL ***/

static void race_control(vars)
  GnatVars *vars;
{
  do {

    /* Go through the race checklist. */

    vars->weapon_heat = heat();
    check_armor(vars);
    check_ammo(vars);
    check_fuel(vars);
    check_geography(vars);

    check_battlefield(vars);
    check_for_scrolls(vars);

    if (settings.full_map && vars->path_ok && 
	vars->opt_path_planning != DoneMapping)
      find_opt_path(vars);
    
    /* If we're on a depot, see if we can use it. */

    if (!vars->taking_damage)
      switch(OnLandmark(vars)) {
      case ARMOR:
	if (vars->armor_cond == Critical && vars->money >= 5000) {
	  vars->action = LoadArmor;
	  goto do_the_race;
	}
	break;
      case AMMO:
	if (vars->ammo_cond == Critical && vars->money >= 50) {
	  vars->action = LoadAmmo;
	  goto do_the_race;
	}
	break;
      default:
	if (vars->action == LoadArmor || vars->action == LoadAmmo)
	  rethink(vars);
	if (vars->armor_cond == Critical && has_special(REPAIR) && 
	    vars->fuel_cond > Unstable && 
	    frame_number() - vars->fnum_enemy_sighting > 30) {
	  vars->action = Repair;
	  goto do_the_race;
	}
	else if (vars->action == Repair)
	  rethink(vars);
      }

    /* Race !! */

    if (vars->action == NoAction || vars->goal_changed) {
      vars->action = Explore;
      if (OnMaze(vars->goal.x, vars->goal.y)) {
	if (found_fast_path(vars, vars->goal.x, vars->goal.y, False))
	  vars->action = Race;
      }
      if (vars->action == Explore)
	set_to_explore(vars);
    }

    /* Do movement and firing. */

  do_the_race:

    switch (vars->movement) {
    case Default:
      if (action_type[vars->action] == Loading) {
	if (vars->action == Repair)
	  set_abs_drive(vars->drive = 0.0);
	else
	  load_from_depot(vars);
      }
      else if (vars->path_ok)
	move_to_dest(vars);
      else
	rethink(vars);
      break;
    case Evading:
    case LastStand:
      break;
    case Avoiding:
      vars->movement = Default;
      break;
    }

    fire_control(vars);

    /* Done with this cycle. */

#ifdef DEBUG_GNAT
    if (vars->path_ok)
      sprintf(text, "%d(%d)-%d: %s %s (%d,%d)", 
	      frame_number(), vars->num_ivects, vars->opt_path_planning,
	      action_desc[vars->action], move_desc[vars->movement],
	      vars->dest.x, vars->dest.y);
    else
      sprintf(text, "%d(%d)-%d: %s %s", frame_number(), vars->num_ivects,
	      vars->opt_path_planning, action_desc[vars->action], 
	      move_desc[vars->movement]);
    send_msg(RECIPIENT_ALL, OP_TEXT, text);
#endif

    done();

  } while (True);
}


/*** Cleanup function. ***/

static void clean_up(vars)
  GnatVars *vars;
{
  free((char *) vars);
}

/*** The main loop accessable to the xtank program. ***/

static void gnat_main()
{
  GnatVars *vars;

  /* Initialize our variables. */

  get_settings(&settings);
  if ((vars = (GnatVars *) malloc(sizeof(GnatVars))) == NULL) {
    fprintf(stderr, "*** COULD NOT ALLOCATE GnatVars! ***\n");
    fflush(stderr);
    do
      done();
    while (True);
  }
  gnat_init(vars);

  /* Pass control to main loop. */

  set_cleanup_func(clean_up, vars);
  if (settings.game == RACE_GAME)
    race_control(vars);
  else
    combat_control(vars);
}  
