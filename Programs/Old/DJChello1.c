/********************************************
* DJChello.c - my first xtank robot
* This robot isn't meant to be original.
* It's just my attempt to make something
* which works.
********************************************/

#include <stdio.h>
#include <math.h>
#include "/mit/games/src/vax/xtank/Src/xtanklib.h"

#define NOT_HERE (-1)
#define UNKNOWN_ID 255
#define MAX_WEAPONS 6

extern int DJChello1_main();

Prog_desc DJChello1_prog =
{
  "DJChello1",     /* program name */
  "Vanguard",     /* default tank */
  "martyrdom",    /* strategy */
  "Dan Connelly", /* author */
  USES_MESSAGES,  /* skills */
  1,              /* skill level */
  DJChello1_main   /* main procedure */
};




/*************************************************************************
* DJCHELLO1_MAIN : main procedure; initializes and calls other procedures
*/

DJChello1_main()
{

  Vehicle_info	*my_info;               /* info about me */
  Vehicle_info  vehicle[MAX_VEHICLES];  /* info about all visible vehicles */
  int		num_vehicles;           /* number of vehicles seen */
  int           i,j;                    /* counting variables */
  int           num;                    /* temporary storage variable */
  struct Player_Info
    {
      ID  id;                           /* player identification */
      int status;                       /* number in array of visible vehicles or NOT_HERE */
    }           player_info[MAX_VEHICLES+1];


  /*---Initialize player information array---*/

  get_self(&my_info);

  player_info[0].id = my_info->id;               /* I'm accounted for */
  player_info[0].status = NOT_HERE;

  for (i=1; i<=MAX_VEHICLES; i++)
    {
      player_info[i].status = NOT_HERE;              /* mark that nobody's here */
      player_info[i].id     = UNKNOWN_ID;       /* nobody else is known */
    }

  while(1) {
    get_vehicles(&num_vehicles, vehicle);

    /* say hello to new friends */
    for (i=0; i<num_vehicles; i++)
      {
	for (j=0; (player_info[j].id != vehicle[i].id) && (player_info[j].id != UNKNOWN_ID); j++){};
	if (player_info[j].status == NOT_HERE)
	  {
	    send(vehicle[i].id, OP_TEXT, "Hello!");  /* send a hearty hello */
	    /* send(my_info->id, OP_TEXT, "hello sent");  /* Debug line */
	    player_info[j].id = vehicle[i].id;      /* remember who he is */
	  }
	player_info[j].status = i;                  /* remember that he is here */
      }
    
    /* say goodbye to departed loved ones */
    for (i=0; player_info[i].id != UNKNOWN_ID; i++)
      {
	if ( (player_info[i].id != my_info->id) && (num = player_info[i].status) != NOT_HERE &&
	    ( (num >= num_vehicles) || vehicle[num].id != player_info[i].id ))
	  {
	    send(player_info[i].id, OP_TEXT, "Goodbye!");     /* wave goodbye */
	    /* send(my_info->id, OP_TEXT, "goodbye sent"); /* Debug line */
	    player_info[i].status = NOT_HERE;  /* remember that he's gone */
	  }
      }

    done();
  }
}


  
