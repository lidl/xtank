/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** setup.c
*/

#include "xtank.h"
#include "gr.h"
#include "vstructs.h"

extern int num_terminals;
extern Terminal *terminal[];
extern Engine_stat engine_stat[];

int num_combatants;
Combatant combatant[MAX_VEHICLES];

/*
** Sets up a combatant for each player and 5 + difficulty/2 robots.
*/
standard_combatants()
{
  Combatant *c;
  int i;

  num_combatants = num_terminals + 5 + settings.difficulty/2;
  for(i = 0 ; i < num_combatants ; i++) {
    c = &combatant[i];
    if(i < num_terminals) {
      /* Set up a vehicle for each player */
      c->num_players = 1;
      c->player[0] = i;
      c->num_programs = 0;
      c->team = (i+1) % MAX_TEAMS;
      c->vdesc = terminal[i]->vdesc;
    }
    else {
      /* Set up everyone else as neutral robots some fighters some talkers */
      c->num_players = 0;
      c->num_programs = 1;
      c->program[0] = choose_program();
      c->team = 0;
      c->vdesc = 0;
    }
  }
}

/*
** Chooses a program based on difficulty and game settings.
*/
choose_program()
{
  return 1;
}

/*
** Sets up 5 + difficulty robot combatants on different teams.
*/
robot_combatants()
{
  Combatant *c;
  int i;

  num_combatants = 5 + settings.difficulty;
  for(i = 0 ; i < num_combatants ; i++) {
    c = &combatant[i];
    c->num_players = 0;
    c->num_programs = 1;
    c->program[0] = choose_program();
    c->team = i%MAX_TEAMS;
    c->vdesc = 0;
  }
}

/*
** Sets up a combatant for each player.
*/
player_combatants()
{
  Combatant *c;
  int i;

  num_combatants = num_terminals;
  for(i = 0 ; i < num_combatants ; i++) {
    c = &combatant[i];
    c->num_players = 1;
    c->player[0] = i;
    c->num_programs = 0;
    c->team = (i+1) % MAX_TEAMS;
    c->vdesc = terminal[i]->vdesc;
  }
}

/*
** Sets up a customized combatants list from the combatants grid.
*/
customized_combatants()
{
  Combatant *c;
  int i;

  num_combatants = 0;
  for(i = 0 ; i < MAX_VEHICLES ; i++) {
    c = &combatant[num_combatants];
    if(!make_grid_combatant(c,i))
      num_combatants++;
  }
}
  
/*
** Initializes the number, score, kills, deaths, and money of all combatants.
** Initializes mode to single, battle, or multi depending on number of players.
*/
init_combatants()
{
  extern Vdesc vdesc[];
  extern Prog_desc *prog_desc[];
  extern int num_teams;
  Combatant *c;
  int max_cost,max_team,max_players,i;

  max_cost = 0;
  max_team = 0;
  max_players = 0;
  for(i = 0 ; i < num_combatants ; i++) {
    c = &combatant[i];
    if(c->num_players) {
      max_players++;
      (void) strncpy(c->name,terminal[c->player[0]]->player_name,MAX_STRING-1);
    }
    else
      (void) strncpy(c->name,prog_desc[c->program[0]]->name,MAX_STRING-1);
    max_cost = max(max_cost,vdesc[c->vdesc].cost);
    max_team = max(max_team,c->team);
    c->number = i;
    c->score = 0;
    c->kills = 0;
    c->deaths = 0;
  }

  switch(max_players) {
    case 0:  settings.mode = BATTLE_MODE; break;
    case 1:  settings.mode = SINGLE_MODE; break;
    default: settings.mode = MULTI_MODE; break;
  }

  /* Give everyone money equal to 1.5 * max_cost - cost of their vehicle */
  for(i = 0 ; i < num_combatants ; i++) {
    c = &combatant[i];
    c->money = max_cost * 1.5 - vdesc[c->vdesc].cost;
  }
  num_teams = max_team + 1;
}

/*
** Animates a game until it is finished, and then cleans up allocated stuff.
** Returns final status of the game.
*/
play_game()
{
  unsigned int status;
  int i;
  Boolean newgame;

  if(num_combatants == 0) return GAME_FAILED;
  init_combatants();
  newgame = TRUE;

  do {
     /* Setup game */
     if(setup_game(newgame) == GAME_FAILED) return GAME_FAILED;
     newgame = FALSE;

#ifdef X11
     button_up(ANIM_WIN,FALSE);
     follow_mouse(ANIM_WIN,FALSE);
#endif   

     /* Animate until game is finished */
     do {
	status = animate();
     } while(status == GAME_RUNNING);

     /* Display stats about the games state */
     status = display_game_stats(status);

     /* Free all allocated memory for game */
     free_game_mem();
  } while (status == GAME_RESET);

  /* Unmap battle windows and clear all other windows on all the terminals */
  for(i = 0 ; i < num_terminals ; i++) {
    set_terminal(i);
    unmap_battle_windows();
    clear_windows();
  }

  /* Return to terminal 0 for the user interface */
  set_terminal(0);

#ifdef X11
  button_up(ANIM_WIN,TRUE);
  follow_mouse(ANIM_WIN,TRUE);
#endif   

  return status;
}

/*
** Sets up the combatants, and the world for a game.
** Returns either GAME_FAILED or GAME_RUNNING.
*/
setup_game(newgame)
     Boolean newgame;
{
  extern int num_vehicles,num_terminals;
  extern Vehicle *vehicle[];
  int i;

  frame = 0;
  num_vehicles = 0;

  /* Setup maze */
  setup_maze();

  /* Initialize bullets and explosions */
  init_bset();
  init_eset();

  /* Initialize all terminals to vehicle 0 */
  for(i = 0 ; i < num_terminals ; i++)
    setup_terminal(i,vehicle[0]);
  
  /* Set up combatants */
  for(i = 0 ; i < num_combatants ; i++)
    if(setup_combatant(&combatant[i]) == GAME_FAILED) return GAME_FAILED;

  /* Initialize message menus for game */
  init_msg_game();

  /* Initialize status windows (based on vehicle information) */
  init_status();

  /* Initialize game rules */
  game_rules(TRUE);

  /* Initialize the commentator, passing*/
  if(settings.commentator)
    comment(COS_INIT_MOUTH,(int) newgame,(Vehicle *) NULL,(Vehicle *) NULL);

  /* Flush the input */
  sync_terminals(TRUE);

  return GAME_RUNNING;
}

/*
** Makes and initializes the vehicle for the specified combatant.
** Returns GAME_RUNNING if successful, GAME_FAILED if vehicle could not
** be set up.
*/
setup_combatant(c)
     Combatant *c;
{
  Mapper *m;
  extern char team_char[];
  extern Vdesc vdesc[];
  extern Vehicle *make_vehicle();
  Vehicle *v;
  int i;

  /* Make and initialize the vehicle */
  v = make_vehicle(&vdesc[c->vdesc]);
  if(v == (Vehicle *) NULL) return GAME_FAILED;

  /* Assign number to vehicle before init, so init_messages() will work */
  v->number = c->number;
  init_vehicle(v);

  /* Assign the combatant specific information to the vehicle */
  v->flag = VEHICLE_0 << v->number;
  v->team = c->team;
  sprintf(v->disp,"%c%d %s",team_char[c->team],c->number,c->name);
  v->owner = c;
  c->vehicle = v;

  /* Make and initialize the programs */
  make_programs(v,c->num_programs,c->program);
  init_programs(v);

  /* Set up each player's terminal */
  for(i = 0 ; i < c->num_players ; i++)
    setup_terminal(c->player[i],v);

  /* Place vehicle in maze */
  if(place_vehicle(v) == -1) return GAME_FAILED;

  /* Activate the specials */
  for(i = 0 ; i < MAX_SPECIALS ; i++)
    do_special(v,i,SP_activate);

  /* Copy maze into mapper if full_map is on */
  if(v->special[MAPPER].status != SP_nonexistent && settings.full_map) {
    m = (Mapper *) v->special[MAPPER].record;
    bcopy((char *) box,(char *) m->map,sizeof(Map));
  }
      
  return GAME_RUNNING;
}

/*
** Sets the vehicles and exposes the windows of the numbered terminal.
** Sets the number of lines drawn on the terminal to 0.
*/
setup_terminal(num,v)
     int num;
     Vehicle *v;
{
  int i;

  set_terminal(num);
  term->vehicle = v;
  for(i = 0 ; i < MAX_WINDOWS ; i++)
    expose_win(i,TRUE);
  term->num_lines = 0;
}

/*
** Makes a vehicle structure from the specified vehicle description.
** Returns NULL if no room in list, otherwise returns pointer to vehicle.
*/
Vehicle *make_vehicle(d)
     Vdesc *d;
{
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  extern Object *vehicle_obj[];
  static int turn_divider[MAX_SPEED] = {
    8, 8, 8, 8, 10, 12, 16, 20, 24, 28, 34, 40, 46, 52, 60, 68, 78, 88,
    100, 114, 130, 146, 162, 180, 200 };
  Vehicle *v;
  Weapon *w;
  int i;

  if(num_vehicles >= MAX_VEHICLES) return (Vehicle *) NULL;

  v = vehicle[num_vehicles++];

  v->vdesc = d;
  v->name = d->name;

  v->obj = vehicle_obj[d->body];
  v->num_turrets = v->obj->num_turrets;
  v->max_fuel = (float) engine_stat[d->engine].fuel_limit;
  
  v->num_weapons = d->num_weapons;

  for(i = 0 ; i < d->num_weapons ; i++) {
    w = &v->weapon[i];
    w->type = d->weapon[i];
    w->mount = d->mount[i];
  }

  for(i = 0 ; i < MAX_SPECIALS ; i++)
    v->special[i].status = (d->specials & (1<<i)) ? SP_off:SP_nonexistent;

  /* Turn rate for each speed */
  for(i = 0 ; i < MAX_SPEED ; i++)
    v->turn_rate[i] = (float) d->handling / (float) turn_divider[i];

  /* Vehicle not restricted to making safe turns */
  v->safety = FALSE;

  return v;
}

/*
** Tries to put the vehicle at each of its team starting locations.
** If those are all taken, random coordinates are used.
** Returns -1 if vehicle could not be placed.
*/
place_vehicle(v)
     Vehicle *v;
{
  extern Maze maze;
  Vector *vector;
  int views;
  Box *b;
  int num_starts;
  Coord *start;
  int grid_x,grid_y;
  int tries,num;

  /*
  ** Check if the desired box is in the maze, and unoccupied.
  ** If not, start picking random boxes until we find one that is.
  ** Once we find a "reasonable" box, try to get one that is the same
  ** team as the vehicle and normal.  If this doesn't seem possible, accept a
  ** neutral normal box.  If this doesn't seem possible, accept a box of any
  ** team that isn't an outpost.  If this doesn't seem possible, the vehicle
  ** cannot be placed.
  */
  num_starts = maze.num_starts[v->team];
  start = maze.start[v->team];
  tries = 0;
  num = 0;
  for(;;) {
    /* Take the next start location, or a random one if there are none */
    if(num < num_starts) {
      grid_x = start[num].x;
      grid_y = start[num].y;
      num++;
    }
    else {
      grid_x = rnd(GRID_WIDTH);
      grid_y = rnd(GRID_HEIGHT);
    }

    b = &box[grid_x][grid_y];

    if(b->flags & INSIDE_MAZE &&
       !(b->flags & ANY_VEHICLE)) {
      if(b->team == v->team && b->type == NORMAL) break;
      else if(tries > 500 && b->team == 0 && b->type == NORMAL) break;
      else if(tries > 1000 && b->type != OUTPOST) break;
    }

    /* Give up after 1500 tries */
    if(tries++ == 1500) return -1;
  }

  /* Put the vehicle in the center of the box */
  v->loc = &v->loc1;
  v->old_loc = &v->loc2;

  v->loc->grid_x = grid_x;
  v->loc->grid_y = grid_y;

  v->loc->box_x = BOX_WIDTH/2;
  v->loc->box_y = BOX_HEIGHT/2;

  v->loc->x = v->loc->grid_x*BOX_WIDTH + v->loc->box_x;
  v->loc->y = v->loc->grid_y*BOX_HEIGHT + v->loc->box_y;

  *v->old_loc = *v->loc;

  /* Orient the vehicle randomly */
  vector = &v->vector;

  vector->desired_drive = vector->speed = vector->drive = 0.0;
  vector->xspeed = vector->yspeed = 0.0;
  vector->spin = 0.0;

  vector->desired_heading = vector->old_angle = vector->angle =
    vector->heading = vector->old_heading = (float) rnd(100)*(2*PI)/100 - PI;
  views = v->obj->num_pics;
  vector->old_rot = vector->rot =
    ((int) ((vector->heading)/(2*PI)*views + views + .5))%views;
  vector->drive_flag = NO_ACCELERATION;
  vector->heading_flag = NO_ROTATION;

  /* Set flag in box to show that the vehicle is there */
  box[v->loc->grid_x][v->loc->grid_y].flags |= v->flag;

  return 0;
}

/*
** Removes ith vehicle from the list, trying to restart a new one.
** Returns GAME_OVER if the game should end, otherwise GAME_RUNNING.
*/
remove_vehicle(num,quitting)
     int num;
     Boolean quitting;
{
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  Vehicle *v;
  int i;

  /* Rearrange vehicles in array to shrink it by one */
  v = vehicle[num];
  if(num < num_vehicles - 1) {
    vehicle[num] = vehicle[num_vehicles - 1];
    vehicle[num_vehicles - 1] = v;
  }
  num_vehicles--;

  /* Let the terminals know that they have no vehicle to track */
  for(i = 0 ; i < v->owner->num_players ; i++)
    terminal[v->owner->player[i]]->vehicle = (Vehicle *) NULL;

  /* Free the memory allocated for the vehicle */
  free_vehicle_mem(v);
  v->owner->deaths++;

  /* If restart is on, restart the vehicle, otherwise check for game over */
  if(settings.restart) {
    setup_combatant(v->owner);
    init_vehicle_status(v);
  }
  else {
    if(settings.mode == SINGLE_MODE && v->owner->num_players > 0)
      return GAME_OVER;
    if(num_vehicles <= 1)
      return GAME_OVER;
  }

  return GAME_RUNNING;
}

/*
** Frees allocated memory for all remaining vehicles.
*/
free_game_mem()
{
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  int i;

  /* Free all the allocated memory for every vehicle */
  for(i = 0 ; i < num_vehicles ; i++)
    free_vehicle_mem(vehicle[i]);
}

/*
** Frees allocated memory for the specified vehicle.
*/
free_vehicle_mem(v)
     Vehicle *v;
{
  int i;

  /* Free the memory for the thread and buffer */
  for(i = 0 ; i < v->num_programs ; i++)
    free((char *) v->program[i].thread);

  /* Free the special record pointers */
  for(i = 0 ; i < MAX_SPECIALS ; i++)
    free((char *) v->special[i].record);

  /* Free the turret array */
  if(v->num_turrets > 0)
    free((char *) v->turret);
}
