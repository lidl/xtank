/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** animate.c
*/

#include "xtank.h"

/* Number of frames animation lasts after end of game */
#define QUIT_DELAY 17

/* # frames between display synchronizations */
int sync_rate = 1;

/*
** Removes dead vehicles, updates everything, checks for collisions,
** and displays all terminals, for one frame of animation.
**
** Returns one of GAME_RUNNING, GAME_QUIT, GAME_OVER, or GAME_RESET.
*/
animate()
{
  extern int num_terminals;
  extern Terminal *terminal[];
  extern int num_vehicles;
  extern Vehicle *vehicle[];
  Vehicle *v;
  unsigned int retval;
  int i;
  static int quit_frame;	/* frame that game ends */

  /* Check for paused or slowed game */
  check_game_speed();

  /* Increment frame counter */
  ++frame;

  /* Reset quit frame if we are starting a new game */
  if(frame <= 1) quit_frame = frame - 1;

  /* Check for end of game */
  if(frame == quit_frame) return(GAME_OVER);

  /* Update the was_alive and is_alive flags */
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];
    if(v->status & VS_is_alive) v->status |= VS_was_alive;
  }

  /* Delete any dead vehicles from the vehicle list */
  i = 0;
  while(i < num_vehicles) {
    v = vehicle[i];
    if(!(v->status & VS_is_alive)) {
      if(remove_vehicle(i) == GAME_OVER) quit_frame = frame + QUIT_DELAY;
    }
    else
      i++;
  }

  /* Initialize the changed boxes in the maze */
  init_changed_boxes();

  /* Update the old screen locations for all the terminals */
  for(i = 0 ; i < num_terminals ; i++)
    terminal[i]->old_loc = terminal[i]->loc;

  /* Clear the number of new messages for all the vehicles */
  for(i = 0 ; i < num_vehicles ; i++)
    vehicle[i]->new_messages = 0;

  /* Process input from all the programs */
  run_all_programs();

  /* Process input from all the terminals */
  for(i = 0 ; i < num_terminals ; i++) {
    set_terminal(i);
    if(get_input() == GAME_QUIT) {
      if(i != 0) {
	/*
        ** Kill the vehicle, decrement owner's num_players, remove the player.
	** Decrement counter since a new terminal is moved into old slot.
	*/
	kill_vehicle(terminal[i]->vehicle,(Vehicle *) NULL);
	terminal[i]->vehicle->owner->num_players--;
	remove_player(i);
	i--;
      }
      else
	return GAME_QUIT;
    }
  }

  /* Update locations and vectors of all vehicles, and bullets */
  for(i = 0 ; i < num_vehicles ; i++)
    update_vehicle(vehicle[i]);
  update_bullets();
  
  /* Check for collisions between all vehicles */
  coll_vehicles_vehicles();

  /* Check for collisions between each vehicles and the walls */
  for(i = 0 ; i < num_vehicles ; i++)
    coll_vehicle_walls(vehicle[i]);

  /* Check for bullet collisions against the maze and vehicles */
  coll_bullets_maze();
  coll_bullets_vehicles();

  /* Update vehicle rotations after collisions */
  for(i = 0 ; i < num_vehicles ; i++)
    update_rotation(vehicle[i]);

  /* Update explosions after bullet colls, since they might create some */
  update_explosions();

  /* Update maze flags after vehicle colls, to get new vehicle positions */
  update_maze_flags();

  /* Update specials after maze flags, since some specials use them */
  update_specials();

  /* Apply rules of the current game */
  if((retval = game_rules(FALSE)) != GAME_RUNNING) return retval;

  /* Update screen locations, and display everything on each terminal */
  for(i = 0 ; i < num_terminals ; i++) {
    set_terminal(i);
    update_screen_locs();
    display_terminal(REDISPLAY);
  }
  
  /* Synchronize all terminals every sync_rate frames */
  if(frame % sync_rate == 0) sync_terminals(FALSE);
  
  return GAME_RUNNING;
}
