/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** structs.h
*/

typedef unsigned int Flag;

typedef struct {
  Flag flags;			/* bits for walls, inside maze */
  Byte type;			/* landmark, scroll, goal, outpost, etc. */
  Byte team;			/* number of team that owns the box */
  Byte strength;		/* strength of the scroll, outpost, etc. */
} Box;

typedef Box Map[GRID_WIDTH][GRID_HEIGHT];

typedef struct {
  short x;
  short y;
} Coord;

typedef struct {
  Byte type;
  char *name;
  char *designer;
  char *desc;
  Byte *data;
} Mdesc;

typedef struct {
  int num_starts[MAX_TEAMS];	/* number of start locs for each team */
  Coord start[MAX_TEAMS][MAX_VEHICLES];	/* coordinates of starting locations */
} Maze;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;			/* The rest of the stuff is for SPEED */
  int dx;			/* x2-x1 */
  int dy;			/* y2-y1 */
  float slope;			/* dy/dx */
  int intercept;		/* y1 - slope * x1 */
  int minx;			/* min(x1,x2) */
  int miny;			/* min(y1,y2) */
  int maxx;			/* max(x1,x2) */
  int maxy;			/* max(y1,y2) */
} Segment;

#ifndef _PICTURE_
#define _PICTURE_
typedef struct {
  int width;
  int height;
  int offset_x;
  int offset_y;
  int pixmap;
} Picture;
#endif _PICTURE_

typedef struct {
  Coord turret_coord[MAX_TURRETS]; /* relative to center */
  Segment segment[MAX_SEGMENTS]; /* polygon shaped to the picture */
} Picinfo;

typedef struct {
  char type[MAX_STRING];	/* type of object */
  int num_pics;			/* number of picture in the object */
  Picture *pic;			/* array of pictures of the object */
  int num_turrets;		/* number of turrets in object */
  int num_segs;			/* number of segments to represent object */
  Picinfo *picinfo;		/* array of info about pictures */
} Object;

typedef struct {
  float x;			/* absolute coordinates */
  float y;
  float z;
  float box_x;			/* coordinates relative to box */
  float box_y;
  int grid_x;			/* coordinates of the box in the grid */
  int grid_y;
  int screen_x[MAX_TERMINALS];	/* screen coords for each terminal */
  int screen_y[MAX_TERMINALS];
} Loc;

typedef struct {
  int x;
  int y;
  int grid_x;
  int grid_y;
} Intloc;

typedef struct _Vector {
  float speed;			/* actual ground speed */
  float angle;			/* actual ground angle */
  float heading;	       	/* direction vehicle is pointing */
  float drive;			/* speed the wheels are turning */
  float spin;			/* rotational velocity */
  float xspeed;			/* x component of speed and angle */
  float yspeed;			/* y component of speed and angle */
  float old_angle;
  float old_heading;
  float desired_drive;
  float desired_heading;
  Flag drive_flag;		/* indicates acceleration direction */
  Flag heading_flag;		/* indicates rotation direction */
  int rot;			/* index of picture (based on heading) */
  int old_rot;
} Vector;

typedef struct {
  float angle;			/* angle that the turret is pointing */
  float desired_angle;		/* angle driver wants turret to point */
  Flag angle_flag;		/* which way the turret is rotating */
  float turn_rate;		/* how fast the turret can rotate */
  int rot;			/* picture to show on the screen */
  int old_rot;			/* picture to erase from the screen */
  Object *obj;			/* pointer to object for the turret */
} Turret;

typedef struct {
  int type;			/* weapon type (determines bullet type) */
  int hits;			/* # hit points left in weapon */
  int mount;			/* location where weapon is mounted */
  int reload_counter;		/* # frames until next shot can be fired */
  int ammo;			/* number of ammo units left in weapon */
  Flag status;			/* status of weapon (on/off,no_ammo) */
} Weapon;

typedef struct {
  int type;
  int side[MAX_SIDES];
} Armor;

typedef struct {
  char name[MAX_STRING];
  char designer[MAX_STRING];
  int body;
  int engine;
  int num_weapons;
  int weapon[MAX_WEAPONS];
  int mount[MAX_WEAPONS];
  Armor armor;
  Flag specials;
  int heat_sinks;
  int suspension;
  int treads;
  int bumpers;
  float max_speed;
  float acc;
  int handling;
  int weight;
  int space;
  int cost;
} Vdesc;

typedef struct {
  char string[MAX_STRING];
  int value;
} Entry;

typedef struct {
  Entry _entry[MAX_ENTRIES];
  int num_changes;
  int change[MAX_ENTRIES]; /* indices of entries that changed */
} Console;

typedef struct {
  Flag type;
  Byte x;
  Byte y;
} Symbol;

typedef struct {
  Boolean need_redisplay;
  Boolean initial_update;
  int num_symbols;
  Symbol symbol[(2*NUM_BOXES+1)*(2*NUM_BOXES+1)]; /* new symbols to display */
  Coord marker;			/* show's vehicle location */
  Coord old_marker;
  Map map;
  int num_landmarks;
  Symbol landmark[MAX_LANDMARKS]; /* locations of recently passed landmarks */
} Mapper;

typedef struct {
  Byte x;
  Byte y;
  Byte life;
  Byte view;
  Byte old_view;
  Byte flags;
} Blip;

typedef struct {
  int num_blips;
  Blip blip[MAX_BLIPS];
  int pos;			/* current rotation of sweep */
  int start_x,start_y;		/* starting coordinate of sweep line */
  int end_x,end_y;		/* ending coordinate of sweep line */
  int old_start_x,old_start_y;
  int old_end_x,old_end_y;
} Radar;

typedef struct {
  Flag status;			/* status of the special */
  int (*proc)();		/* function to call for special */
  char *record;			/* pointer to special structure */
} Special;

typedef struct {
  char *name;			/* name of program */
  char *vehicle;		/* name of default vehicle */
  char *strategy;		/* description of strategy */
  char *author;			/* name of author */
  unsigned int abilities;	/* things the program does */
  int skill;			/* skill at doing these things */
  int (*func)();		/* main procedure of program */
  char *code;			/* pointer to code memory */
} Prog_desc;

typedef struct {
  Prog_desc *desc;		/* description of program */
  int status;			/* status of program */
  int next_message;		/* index of next message for program to read */
  int total_time;		/* execution time used by prog (in usec) */
  char *thread;			/* pointer to thread of execution */
} Program;

typedef struct {
  Byte sender;			/* vehicle number of sender */
  Byte sender_team;		/* team number of sender */
  Byte recipient;		/* vehicle number of recipient */
  Byte opcode;			/* type of message */
  Byte data[MAX_DATA_LEN];	/* data of message */
} Message;

typedef struct {
  struct _Combatant *owner;	/* owner of vehicle */
  char *name;			/* name of the vehicle */
  char disp[MAX_STRING];	/* string to display under vehicle */
  Flag flag;			/* unique flag for this vehicle */
  Flag status;			/* status of vehicle */
  Byte number;			/* number of vehicle (0 to MAX_VEHICLES-1) */
  int team;			/* team that the vehicle is on */
  int num_programs;		/* number of programs present in vehicle */
  Program program[MAX_PROGRAMS]; /* program to control vehicle */
  Program *current_prog;	/* current program being executed */
  int next_message;		/* index to slot for next message received */
  int new_messages;		/* number of messages received this frame */
  Message received[MAX_MESSAGES]; /* received messages storage */
  Message sending;		/* message to send */
  Loc *loc;			/* pointer to location info */
  Loc *old_loc;			/* pointer to old location info */
  Loc loc1;			/* 1st area for location info */
  Loc loc2;			/* 2nd area for location info */
  Vector vector;		/* orientation info about vehicle */
  int num_turrets;		/* number of turrets on tank */
  Turret *turret;		/* pointer to beginning of turret array */
  Object *obj;			/* pointer to screen object for the vehicle */
  Vdesc *vdesc;			/* description of vehicle */
  float max_fuel;		/* amount of fuel tank can hold */
  float fuel;			/* amount of fuel in tank */
  int heat;			/* amount of heat in vehicle */
  float turn_rate[MAX_SPEED];	/* safe turning rate for each speed */
  Armor armor;			/* present armor points on vehicle */
  int num_weapons;		/* number of weapons on vehicle */
  Weapon weapon[MAX_WEAPONS];	/* array of weapons */
  Special special[MAX_SPECIALS]; /* array of specials */
  Boolean safety;		/* TRUE means turn rate is limited */
  int num_discs;		/* number of discs owned by the vehicle */
  int color;			/* color for vehicle and bullets it owns */
} Vehicle;

typedef struct {
  Vehicle *owner;		/* pointer to vehicle that shot bullet */
  Loc *loc;			/* pointer to location info */
  Loc *old_loc;			/* pointer to previous location info */
  Loc loc1;			/* 1st area for location info */
  Loc loc2;			/* 2nd area for location info */
  float xspeed;			/* speed of travel in x direction */
  float yspeed;			/* speed of travel in y direction */
  int type;			/* type of bullet */
  int life;			/* number of frames left before bullet dies */
  Boolean hurt_owner:1;		/* whether bullet can hurt owner or not */
} Bullet;

typedef struct {
  int number;			/* number of bullets */
  Bullet *list[MAX_BULLETS];	/* array of pointers to bullets */
  Bullet array[MAX_BULLETS];	/* array of bullets */
} Bset;

typedef struct {
  int x;			/* x coord of explosion */
  int y;			/* y coord of explosion */
  int z;			/* z coord of explosion */
  int screen_x[MAX_TERMINALS];	/* x coord on screen */
  int screen_y[MAX_TERMINALS];	/* y coord on screen */
  int old_screen_x[MAX_TERMINALS]; /* previous x coord on screen */
  int old_screen_y[MAX_TERMINALS]; /* previous y coord on screen */
  int life;			/* # frames before explosion dies */
  Object *obj;			/* pointer to object for the explosion */
  int color;			/* color of the explosion */
} Exp;

typedef struct {
  int number;			/* number of explosions */
  Exp *list[MAX_EXPS];		/* array of pointers to explosions */
  Exp array[MAX_EXPS];		/* array of explosions */
} Eset;

typedef struct {
  int x;
  int y;
  int len;
  char *str;
} Word;

/* This should duplicate the definition for an XSegment */
typedef struct { short x1,y1,x2,y2; } Line;

typedef struct {
  Flag num;
  char player_name[MAX_STRING];
  int vdesc;
  Vehicle *vehicle;
  int status;
  Intloc loc;			/* coordinates of ulc of screen in maze */
  Intloc old_loc;
  char *video;			/* video info specific to machine */
				/* Rest is for 3d mode */
  float heading;		/* direction of view */
  float view_angle;		/* the angle of view width */
  float aspect;			/* the aspect ratio of the view */
  int view_dist;		/* the range of sight in pixels */
  int num_lines;		/* number of lines drawn */
  Line line[MAX_LINES];		/* lines drawn on the screen */
} Terminal;

typedef struct _Combatant {
  char name[MAX_STRING];
  int num_players;
  int player[MAX_TERMINALS];
  int num_programs;
  int program[MAX_PROGRAMS];
  int vdesc;
  int number;
  int team;
  int money;
  int score;
  int kills;
  int deaths;
  Vehicle *vehicle;
} Combatant;

typedef struct {
  int mode;			/* single, multi, demo, battle */
  int game;			/* combat, war, ultimate, capture, race */
  int game_speed;		/* max. number of frames per second */
  Mdesc *mdesc;			/* pointer to maze description */
  int maze_density;		/* % of walls kept in random maze (0 - 100) */
  Boolean point_bullets;	/* whether bullets are points or pictures */
  Boolean ricochet;		/* whether bullets bounce off walls */
  Boolean rel_shoot;		/* whether shooter's speed added to bullet's */
  Boolean no_wear;		/* whether vehicles take damage & lose fuel */
  Boolean restart;		/* whether vehicles restart after death */
  Boolean commentator;		/* whether commentator will comment on game */
  Boolean full_map;		/* whether vehicles start out with full map */
  int winning_score;		/* score needed to win the game */
  int outpost_strength;		/* firepower of outposts (0 - 10) */
  float scroll_speed;		/* speed of scroll boxes (0 - 10) */
  float box_slow;		/* slowdown caused by slow boxes (0 - 1) */
  float disc_friction;		/* friction factor applied to disc (0 - 1) */
  float disc_slow;		/* how much to slow down disc owner (0 - 1) */
  float slip_friction;		/* friction in slip boxes (0 - 1) */
  float normal_friction;	/* friction in all other boxes (0 - 1) */
  int difficulty;		/* difficulty of enemy robots (0 - 10) */
} Settings;

typedef struct {
  char vehicle_name[MAX_STRING];
  Byte player;
  Byte program;
  Byte team;
} Comb;

typedef struct {
  char name[MAX_STRING];	/* name of the setup */
  char maze_name[MAX_STRING];	/* name of the maze used in the setup */
  Comb comb[MAX_VEHICLES];	/* short descriptions of each combatant */
  Settings settings;		/* the settings for the game */
} Sdesc;
