/*
** bisfree.c
**
** A totally awesome ultimate-playing tank that kicks ass.
*/

#include "/mit/games/src/vax/xtank/Battle/xtanklib.h"

#define SE 0
#define S  1
#define SW 2
#define W  3
#define NW 4
#define N  5
#define NE 6
#define E  7

#define NOTURN -1

#define SAFETY 30
#define DELTA .1

#define NORTH_WALL (1<<0)
#define WEST_WALL  (1<<1)
#define SEEN       (1<<2)

#define abs(x) ((x)>0?(x):(-x))

extern bisfree_main();
Boolean bisfree_wall();

typedef struct {
  Location loc;
  Angle    dir;
  float    myspeed;
} Binfo;

Prog_desc bisfree_prog = {
  "bisfree",
  "ultimate",
  "bisfree",
"Bisfree plays a damn good game of ultimate, hopefully.  It looks for \
a frisbee and runs after it.  Once it gets it, it lets it go.  Then it \
looks for another frisbee.  And so on, and so on, and so on!",
  "Dan Schmidt",
  2,
  bisfree_main
  };

bisfree_main()
{
  int i = 0;
  int stuckframes = 0;

  Binfo bi;

  set_rel_speed(9.0);
  while(1) {
    get_location(&bi.loc);
    bi.dir = angle();
    myspeed = speed();
    if (bi.myspeed == 0) bisfree_get_unstuck(&bi,&stuckframes);
      else {
	if (stuckframes > 0) --stuckframes;
	else {
	  if ((++i)%2)bisfree_dodge_walls(&bi);
	  else bisfree_do_stuff(&bi);  
	}
      }
  }
}

bisfree_get_unstuck(bi,stuckframes)
     Binfo *bi;
     int *stuckframes;
{
  turn_vehicle(dir+PI);
  set_rel_speed(9.0);
  *stuckframes = 3;
}

int bisfree_dodge_walls(bi)
     Binfo *bi;
{
  Boolean straightdir = FALSE;
  Boolean chosendir = FALSE;
  int dirtype;
  float fdirtype;
  int cturn = NOTURN;
  int x,y;
  char buffer[40];
  Boolean trouble = FALSE;

  x = bi->loc.grid_x;
  y = bi->loc.grid_y;

  fdirtype = (dir*4./PI);
  dirtype = (int) fdirtype;

  /* check for southness */
  if (S+1-DELTA < fdirtype && S+1+DELTA > fdirtype) {
        if (bi->loc.box_x < SAFETY && bisfree_wall(bi,WEST,x,y+1)) {
	  if (bisfree_wall(bi,SOUTH,x,y)) cturn = SW;
	  else if (bisfree_wall(bi,SOUTH,x-1,y)) cturn = SE;
	  choseturn = TRUE;
	}
	else if (bi->loc.box_x > BOX_WIDTH-SAFETY && 
		 bisfree_wall(bi,EAST,x,y+1)) {
	  if (bisfree_wall(bi,SOUTH,x,y)) cturn = SE;
	  else if (bisfree_wall(bi,SOUTH,x+1,y)) cturn = SW;
	  choseturn = TRUE;
	}
      }

  /* check for westness */
  if (W+1-DELTA < fdirtype && W+1+DELTA > fdirtype) {
	if (bi->loc.box_y < SAFETY && bisfree_wall(bi,NORTH,x-1,y)) {
	  if (bisfree_wall(bi,WEST,x,y)) cturn = NW;
	  else if (bisfree_wall(bi,WEST,x,y-1)) cturn = SW;
	  choseturn = TRUE;
	}
	else if (bi->loc.box_y > BOX_HEIGHT-SAFETY && 
		 bisfree_wall(bi,SOUTH,x-1,y)) {
	  if (bisfree_wall(bi,WEST,x,y)) cturn = SW;
	  else if (bisfree_wall(bi,WEST,x,y+1)) cturn = NW;
	  choseturn = TRUE;
	}
      }

  /* check for northness */
  if (N+1-DELTA < fdirtype && N+1+DELTA > fdirtype) {
      case N:
	if (bi->loc.box_x < SAFETY && bisfree_wall(bi,WEST,x,y-1)) {
	  if (bisfree_wall(bi,NORTH,x,y)) cturn = NW;
	  else if (bisfree_wall(bi,NORTH,x-1,y)) cturn = NE;
	  choseturn = TRUE;
	}
	else if (bi->loc.box_x > BOX_WIDTH-SAFETY && 
		 bisfree_wall(bi,EAST,x,y-1)) {
	  if (bisfree_wall(bi,NORTH,x,y)) cturn = NE;
	  else if (bisfree_wall(bi,NORTH,x+1,y)) cturn = NW;
	  choseturn = TRUE;
	}
      }
  
  /* check for eastness */
  if (E+1-DELTA < fdirtype || E+1-8+DELTA > fdirtype) {
	if (bi->loc.box_y < SAFETY && bisfree_wall(bi,NORTH,x+1,y)) {
	  if (bisfree_wall(bi,EAST,x,y)) cturn = NE;
	  else if (bisfree_wall(bi,EAST,x,y-1)) cturn = SE;
	  choseturn = TRUE;
	}
	else if (bi->loc.box_y > BOX_HEIGHT-SAFETY && 
		 bisfree_wall(bi,SOUTH,x+1,y)) {
	  if (bisfree_wall(bi,EAST,x,y)) cturn = SE;
	  else if (bisfree_wall(bi,EAST,x,y+1)) cturn = NE;
	  choseturn = TRUE;
	}
      }

  switch (dirtype) {
  case SE: 
    if (bisfree_wall(bi,SOUTH,x,y)) {
      if (bisfree_wall(bi,EAST,x,y)) {
	if (bisfree_wall(bi,NORTH,x,y)) cturn = W;
	else cturn = N;
      }
      else cturn = E;
    }
    else if (bisfree_wall(bi,EAST,x,y)) cturn = S;
    else if (bisfree_wall(bi,SOUTH,x+1,y)) cturn = E;
    else if (bisfree_wall(bi,EAST,y+1,x)) cturn = S;
    break;
  case S:
    if (bisfree_wall(bi,EAST,x,y)) {
      if (bisfree_wall(bi,SOUTH,x,y)) {
	if (bisfree_wall(bi,WEST,x,y)) cturn = N;
	else cturn = W;
	}     
	else cturn = S;
    }
    else if (bisfree_wall(bi,SOUTH,x,y)) cturn = E;
    else if (bisfree_wall(bi,EAST,x,y+1)) cturn = S;
    else if (bisfree_wall(bi,SOUTH,x+1,y)) cturn = E;
    break;
  case SW:
    if (bisfree_wall(bi,WEST,x,y)) {
      if (bisfree_wall(bi,SOUTH,x,y)) {
	if (bisfree_wall(bi,EAST,x,y)) cturn = N;
	else cturn = E;
      }
      else cturn = S;
    }
    else if (bisfree_wall(bi,SOUTH,x,y)) cturn = W;
    else if (bisfree_wall(bi,WEST,x,y+1)) cturn = S;
    else if (bisfree_wall(bi,SOUTH,x-1,y)) cturn = W;
    break;
  case W:
    if (bisfree_wall(bi,SOUTH,x,y)) {
      if (bisfree_wall(bi,WEST,x,y)) {
	if (bisfree_wall(bi,NORTH,x,y)) cturn = E;
	else cturn = N;
      }
      else cturn = W;
    }
    else if (bisfree_wall(bi,WEST,x,y)) cturn = S;
    else if (bisfree_wall(bi,SOUTH,x-1,y)) cturn = W;
    else if (bisfree_wall(bi,WEST,x,y+1)) cturn = S;
    break;
  case NW:
    if (bisfree_wall(bi,NORTH,x,y)) {
      if (bisfree_wall(bi,WEST,x,y)) {
	if (bisfree_wall(bi,SOUTH,x,y)) cturn = E;
	else cturn = S;
      }
      else cturn = W;
    }
    else if (bisfree_wall(bi,WEST,x,y)) cturn = N;
    else if (bisfree_wall(bi,NORTH,x-1,y)) cturn = W;
    else if (bisfree_wall(bi,WEST,x,y-1)) cturn = N;
    break;
  case N:
    if (bisfree_wall(bi,WEST,x,y)) {
      if (bisfree_wall(bi,NORTH,x,y)) {
	if (bisfree_wall(bi,EAST,x,y)) cturn = S;
	else cturn = E;
      }
      else cturn = N;
    }
    else if (bisfree_wall(bi,NORTH,x,y)) cturn = W;
    else if (bisfree_wall(bi,WEST,x,y-1)) cturn = N;
    else if (bisfree_wall(bi,NORTH,x-1,y)) cturn = W;
    break;
  case NE:
    if (bisfree_wall(bi,EAST,x,y)) {
      if (bisfree_wall(bi,NORTH,x,y)) {
	if (bisfree_wall(bi,WEST,x,y)) cturn = S;
	else cturn = W;
      }
      else cturn = N;
    }
    else if (bisfree_wall(bi,NORTH,x,y)) cturn = E;
    else if (bisfree_wall(bi,EAST,x,y-1)) cturn = N;
    else if (bisfree_wall(bi,NORTH,x+1,y)) cturn = E;
    break;
  case E:
    if (bisfree_wall(bi,NORTH,x,y)) {
      if (bisfree_wall(bi,EAST,x,y)) {
	if (bisfree_wall(bi,SOUTH,x,y)) cturn = W;
	else cturn = S;
      }
      else cturn = E;
    }
    else if (bisfree_wall(bi,EAST,x,y)) cturn = N;
    else if (bisfree_wall(bi,NORTH,x+1,y)) cturn = E;
    else if (bisfree_wall(bi,EAST,y-1,x)) cturn = N;
  }

  switch (cturn) {
  case N:
    turn_vehicle(3*PI/2);   break;
  case S:
    turn_vehicle(PI/2);     break;
  case W:
    turn_vehicle(PI);       break;
  case E:
    turn_vehicle(0.0);      break;
  case NOTURN:
    return(0);              break;
  }
  set_rel_speed(6.0);
  return(1);
}

bisfree_chase_disc(bi)
     binfo *bi;
{
  /* Ha */
}

Boolean bisfree_wall(bi,side,x,y)
     binfo *bi;
     WallSide side;
     int x,y;
{
  static int maze[GRID_WIDTH][GRID_HEIGHT];
  Boolean initialized = FALSE;
  int hx,hy;
  WallSide hside;
  int *pi;

  if (!initialized) {
    for (i=0;i<GRID_WIDTH;++i)
      for (j=0;j<GRID_WIDTH:++j)
	maze[i][j] = 0;
    initialized = TRUE;
  }

  if (side == SOUTH) {
    hx = x;
    hy = y+1;
    hside = NORTH;
  }
  else if (side == EAST) {
    hx = x+1;
    hy = y;
    hside = WEST;
  }
  else {
    hx = x;
    hy = y;
    hside = side;
  }

  if (Maze[hx][hy] & SEEN) {
    if (hside = NORTH) {
      if (Maze[hx][hy] & NORTH_WALL) return (TRUE);
      else return (FALSE);
    }
    else {			/* hside = WEST */
      if (Maze[hx][hy] & WEST_WALL) return (TRUE);
      else return (FALSE);
    }
  }
  else {			/* not seen yet */
    boolean retval;

    if (hside = NORTH) {
      if (wall(NORTH,hx,hy)) {Maze[hx][hy] |= NORTH_WALL; retval = TRUE;}
      else {Maze[hx][hy] &= ~NORTH_WALL; retval = FALSE;}
    }
    else {			/* hside = WEST */
      if (wall(WEST,hx,hy)) {Maze[hx][hy] |= WEST_WALL; retval = TRUE;}
      else {Maze[hx][hy] &= ~WEST WALL; retval = FALSE;}
    }
    Maze[hx][hy] |= SEEN;
    return (retval);
  }
}
