/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vstructs.h
*/

#define VMAX_ENGINES     16
#define VMAX_WEAPONS     18
#define VMAX_ARMORS       7
#define VMAX_BODIES      10
#define VMAX_SPECIALS     3
#define VMAX_SUSPENSIONS  4
#define VMAX_TREADS       4
#define VMAX_BUMPERS      4

#define MIN_ARMOR        0
#define MAX_ARMOR      999

#define OVER_WEIGHT      (1<<0)
#define OVER_SPACE       (1<<1)
#define BAD_MOUNT        (1<<2)

typedef struct {
  char *type;
  int defense;
  int weight;
  int space;
  int cost;
} Armor_stat;

typedef struct {
  char *type;
  int damage;
  int range;
  int max_ammo;
  int reload_time;
  int ammo_speed;
  int weight;
  int space;
  int frames;
  int bullet_num;
  int heat;
  int ammo_cost;
  int cost;
} Weapon_stat;

typedef struct {
  char *type;
  int power;
  int weight;
  int space;
  int fuel_cost;
  int fuel_limit;
  int cost;
} Engine_stat;

typedef struct {
  char *type;
  int size;
  int weight;
  int weight_limit;
  int space;
  float drag;
  int handling_base;
  int turrets;
  int cost;
} Body_stat;

typedef struct {
  char *type;
  int handling_adj;
  int cost;
} Suspension_stat;

typedef struct {
  char *type;
  float friction;
  int cost;
} Tread_stat;

typedef struct {
  char *type;
  float elasticity;
  int cost;
} Bumper_stat;

typedef struct {
  char *type;
  int cost;
} Special_stat;

typedef struct {
  int weight;
  int space;
  int cost;
} Heat_sink_stat;
