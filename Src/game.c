/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** game.c
*/

#include "xtank.h"
#include "gr.h"

extern Maze maze;
extern Vehicle *vehicle[];
extern int num_vehicles;
extern char *game_str[];
extern char *teams_entries[];
extern int num_terminals;

static Vehicle *winning_vehicle;
static int winning_team;
int num_teams;

/*
** Applies the rules of the game during play.
** Returns one of GAME_RUNNING, GAME_RESET, GAME_OVER.
*/
game_rules(init)
     Boolean init;
{
  switch(settings.game) {
    case COMBAT_GAME:   return combat_rules(init);
    case WAR_GAME:      return war_rules(init);
    case ULTIMATE_GAME: return ultimate_rules(init);
    case CAPTURE_GAME:  return capture_rules(init);
    case RACE_GAME:     return race_rules(init);
    }

  return GAME_RUNNING;
}

/*
** Vehicles fight each other for score.
** A team wins when a vehicle on that team gets the required score.
*/
combat_rules(init)
     Boolean init;
{
  Vehicle *v;
  int i;

  if(init) return GAME_RUNNING;
  
  for(i = 0 ; i < num_vehicles ; i++) {
    v = vehicle[i];
    
    if(v->owner->score >= settings.winning_score) {
      winning_vehicle = v;
      winning_team = v->team;
      return GAME_RESET;
    }
  }
  return GAME_RUNNING;
}
    
static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
static int dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

/*
** Teams take over boxes by remaining in them for a period of time.
** The time required decreases as the # neighbors on your team increases.
** The more vehicles in a box, the faster the takeover.
** A team wins the game if they control the required percentage of boxes.
*/
war_rules(init)
     Boolean init;
{
  static Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS];
  static int count[MAX_TEAMS];
  static int winning_count;
  Vehicle *v;
  Box *b;
  Byte *ptr;
  int total_count;
  int old_team,new_team;
  int x,y,i,j,k;

  if(init) {
    /* Clear total # boxes in play and # owned by each team */
    for(i = 0 ; i < num_teams ; i++)
      count[i] = 0;
    total_count = 0;

    for(i = 0 ; i < GRID_WIDTH ; i++)
      for(j = 0 ; j < GRID_HEIGHT ; j++) {
	b = &box[i][j];
	if(!(b->flags & INSIDE_MAZE)) continue;

	/* Count total # boxes in play and # owned by each team */
	count[b->team]++;
	total_count++;

	/* Initialize time based on neighboring teams */
	war_init_time(time,i,j);
      }
    
    /* Compute winning count from settings and total count */
    winning_count = total_count * settings.winning_score / 100;
  }
  else {
    for(i = 0 ; i < num_vehicles ; i++) {
      v = vehicle[i];

      /* Neutral vehicles don't affect anything */
      if(v->team == 0) continue;

      x = v->loc->grid_x;
      y = v->loc->grid_y;
      b = &box[x][y];

      /* Decrement time and check for takeover */
      if(--time[x][y][v->team] == 255) continue;

      /* If not already his, change ownership */
      if(b->team != v->team) {
	old_team = b->team;
	if(change_box(b,x,y)) b->team = b->team ? 0 : v->team;
	new_team = b->team;

	/* Fix counts */
	--count[old_team];
	if(++count[new_team] >= winning_count) {
	  winning_vehicle = (Vehicle *) NULL;
	  winning_team = new_team;
	  return GAME_RESET;
	}

	/* Fix times of surrounding boxes */
	for(k = 0 ; k < 8 ; k++) {
	  /* Increase time for old team, decrease time for new team */
	  ptr = time[x+dx[k]][y+dy[k]];
	  ptr[old_team] = (((int) ptr[old_team]) * 3) >> 1;   /*  *= 1.5  */
	  ptr[new_team] = (((int) ptr[new_team]) << 8) / 384; /*  /= 1.5  */
	}
      }

      /* In any case, reset times for all teams in this box */
      war_init_time(time,x,y);
    }
  }
  return GAME_RUNNING;
}

/*
** Initializes the time values for a square based on the neighboring teams.
*/
war_init_time(time,x,y)
     Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS];
     int x,y;
{
  static Byte begin[9] = { 200, 133, 89, 59, 40, 26, 18, 12, 8 };
  Byte *ptr;
  Box *b;
  int i;

  /* Initialize times for this box to beginning value for 0 neighbors */
  ptr = time[x][y];
  for(i = 0 ; i < num_teams ; i++)
    ptr[i] = begin[0];

  /* Decrease times for this box depending on neighboring teams */
  for(i = 0 ; i < 8 ; i++) {
    b = &box[x+dx[i]][y+dy[i]];
    if(!(b->flags & INSIDE_MAZE)) continue;
    
    ptr = &time[x][y][b->team];
    *ptr = (((int) *ptr) << 8) / 384; /*  *ptr /= 1.5   */
  }
}

/*
** One disc in game, disc owned in enemy goal wins.
*/
ultimate_rules(init)
     Boolean init;
{
  Vehicle *v;
  Box *b;
  int i;

  if(init) {
    /* Start up 1 disc on a random vehicle */
    make_bullet((Vehicle *) NULL,vehicle[rnd(num_vehicles)]->loc,DISC,0.0);
  }
  else {
    /* When a vehicle is in an enemy goal, and has the disc, he wins */
    for(i = 0 ; i < num_vehicles ; i++) {
      v = vehicle[i];
      if(v->num_discs > 0) {
	b = &box[v->loc->grid_x][v->loc->grid_y];
	if(b->type == GOAL && b->team != v->team) {
	  winning_vehicle = v;
	  winning_team = v->team;
	  v->owner->score++;
	  if (settings.commentator)
	    comment(COS_GOAL_SCORED, 0, v, (Vehicle *) NULL);
	  return GAME_RESET;
	}
      }
    }
  }
  return GAME_RUNNING;
}

/*
** One disc per non-neutral team, all discs in own goal wins.
*/
capture_rules(init)
     Boolean init;
{
  static int total_discs;
  Vehicle *v;
  Box *b;
  int i,j;

  if(init) {
    /* Start up 1 disc in the first starting box for each team */
    total_discs = 0;
    for(i = 1 ; i < num_teams ; i++)
      for(j = 0 ; j < num_vehicles ; j++)
	if(vehicle[j]->team == i) {
	  make_bullet((Vehicle *) NULL,vehicle[j]->loc,DISC,0.0);
	  total_discs++;
	  break;
	}
  }
  else {
    /* When a vehicle is in its own goal, and has all the discs, he wins */
    for(i = 0 ; i < num_vehicles ; i++) {
      v = vehicle[i];
      if(v->num_discs == total_discs) {
	b = &box[v->loc->grid_x][v->loc->grid_y];
	if(b->type == GOAL && b->team == v->team) {
	  winning_vehicle = v;
	  winning_team = v->team;
	  return GAME_RESET;
	}
      }
    }
  }
  return GAME_RUNNING;
}

/*
** First one to a goal wins.
*/
race_rules(init)
     Boolean init;
{
  Vehicle *v;
  Box *b;
  int i;

  if(!init) {
    /* When a vehicle is a goal, he wins */
    for(i = 0 ; i < num_vehicles ; i++) {
      v = vehicle[i];
      b = &box[v->loc->grid_x][v->loc->grid_y];
      if(b->type == GOAL) {
	winning_vehicle = v;
	winning_team = v->team;
	return GAME_RESET;
      }
    }
  }
  return GAME_RUNNING;
}

#define mprint(str,x,y) \
  (display_mesg2(ANIM_WIN,str,x,y,M_FONT))

/*
** Displays the current state of the game, who scored, and the current score.
*/
display_game_stats(status)
     unsigned int status;
{
  char s[80];
  int i,j,k,l,m,n,reply;

  for (n=0; n<num_terminals; n++) {
     set_terminal(n);
     
     clear_window(ANIM_WIN);

     sprintf(s, "Game: %s     Frame: %d", game_str[settings.game],frame);
     mprint(s, 5, 2);

     /* If we just reset the game, say who scored what */
     if(status == GAME_RESET) {
       switch(settings.game) {
	 case CAPTURE_GAME:
	   sprintf(s, "All discs collected by the %s team",
		   teams_entries[winning_team]);
	   break;
	 case ULTIMATE_GAME:
	   sprintf(s, "A goal for the %s team scored by %s",
		   teams_entries[winning_team],winning_vehicle->disp);
	   break;
         case COMBAT_GAME:
	   sprintf(s, "Battle won by %s",winning_vehicle->disp);
	   break;
         case WAR_GAME:
	   sprintf(s, "War won by the %s team",teams_entries[winning_team]);
	   break; 
        case RACE_GAME:
	   sprintf(s, "Race won by %s",winning_vehicle->disp);
	   break;
       }
       mprint(s, 5, 4);
     }

     /* Print out the total scores for all the teams */
     mprint("Current score:",5,6);
     l = 8;
     for (i=0, k=0; i<num_teams; i++, k=0) {
       for (j=0, m=l+1; j<num_vehicles; j++)
	 if (vehicle[j]->team==i) {
	   sprintf(s, "     %s: %d points", vehicle[j]->owner->name,
		   vehicle[j]->owner->score);
	   mprint(s, 5, m++);	 
	   k+=vehicle[j]->owner->score;
	 }
       if (m!=l+1) {
	 sprintf(s, "  %s: %d points", teams_entries[i], k);
	 mprint(s, 5, l);
	 l=m+1;
       }
     }
     flush_output();
  }

  set_terminal(0);
  mprint("'q' to end game, 's' to start",20,40);
  do {
    reply = get_reply();
  } while(reply != 'q' && reply != 's' && reply != 'Q' && reply != 'S');
  return ((reply == 'q' || reply == 'Q') ? GAME_QUIT : GAME_RESET);
}
