/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vstructs.h
*/

/*
$Author: lidl $
$Id: vstructs.h,v 1.1.1.1 1995/02/01 00:25:44 lidl Exp $
*/

#define MIN_ARMOR        0
#define MAX_ARMOR      999

#define OVER_WEIGHT      (1<<0)
#define OVER_SPACE       (1<<1)
#define NO_TURRETS	(1<<2)
#define MIS_MOUNT        (1<<3)
#define FRONT_FULL	(1<<4)
#define BACK_FULL	(1<<5)
#define LEFT_FULL	(1<<6)
#define RIGHT_FULL	(1<<7)
#define TURRET_FULL	(1<<8)

  typedef struct {
	  char *type;
	  int defense;
	  int weight;
	  int space;
	  int cost;
  }
Armor_stat;

/* Mount flags */
#define M_FRONT		(1<<0)
#define M_BACK		(1<<1)
#define M_LEFT		(1<<2)
#define M_RIGHT		(1<<3)
#define M_TURRET	(1<<4)
#define M_SIDES		M_FRONT|M_BACK|M_LEFT|M_RIGHT
#define M_LR		M_LEFT|M_RIGHT
#define M_ALL		M_SIDES|M_TURRET

  typedef struct {
	  char *type;
	  int damage;
	  int range;
	  int max_ammo;
	  int reload_time;
	  int ammo_speed;
	  int weight;
	  int space;
	  int mount_space;
	  int frames;
	  int heat;
	  int ammo_cost;
	  int cost;
	  int refill_time;
          int safety;
	  int height;
	  int num_views;
	  int mount;	/* which sides the weapon can be mounted on */
	  unsigned long other_flgs; /* see the Bullet definition (bullet.h) */
	  unsigned long creat_flgs; /* for a description of these... (HAK)  */
	  unsigned long disp_flgs;
	  unsigned long move_flgs;
	  unsigned long hit_flgs;
	  void (*creat_func)(void *v, void *bloc, Angle angle);
	  void (*disp_func)();
	  void (*upd_func)(void *b);
	  void (*hit_func)(int whatHit, void *b, int dx, int dy,
				void *parm1, void *parm2, void *parm3);
}
Weapon_stat;

  typedef struct {
	  char *type;
	  int power;
	  int weight;
	  int space;
	  int fuel_cost;
	  int fuel_limit;
	  int cost;
  }
Engine_stat;

  typedef struct {
	  char *type;
	  int size;
	  int weight;
	  int weight_limit;
	  int space;
	  FLOAT drag;
	  int handling_base;
	  int turrets;
	  int cost;
  }
Body_stat;

  typedef struct {
	  char *type;
	  int handling_adj;
	  int cost;
  }
Suspension_stat;

  typedef struct {
	  char *type;
	  FLOAT friction;
	  int cost;
  }
Tread_stat;

  typedef struct {
	  char *type;
	  FLOAT elasticity;
	  int cost;
  }
Bumper_stat;

  typedef struct {
	  char *type;
	  int cost;
  }
Special_stat;

  typedef struct {
	  int weight;
	  int space;
	  int cost;
  }
Heat_sink_stat;



extern Heat_sink_stat heat_sink_stat;
extern Weapon_stat weapon_stat[VMAX_WEAPONS];
extern Armor_stat armor_stat[MAX_ARMORS];
extern Engine_stat engine_stat[MAX_ENGINES];
extern Body_stat body_stat[MAX_BODIES];
extern Suspension_stat suspension_stat[MAX_SUSPENSIONS];
extern Tread_stat tread_stat[MAX_TREADS];
extern Bumper_stat bumper_stat[MAX_BUMPERS];
extern Special_stat special_stat[MAX_SPECIALS];
