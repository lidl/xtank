/*
** thatsROGERtoyoubuddy
** 
** A robot program for XTANK 0.90 + up
** 
** Authors: Dan Schmidt and Doug Church <lurk slink ooooof whoops splut>
** Goal   : A robot program which plays a good game of disk chase
**          
**
** Revision History:
**  ver 0.000: dschmidt, April 1988 
**      artful.c, which dodged walls and bullets
**
**  ver 0.001: dschmidt, August 7, 1988
**      bisfree.c, which could chase down and catch frisbees, but no
**    longer bullets since we they're harder to catch
**
**  ver 0.010: dachurch + dschmidt, August 13, 1988
**      moose.c, due to a power failure and server losings, is started
**    since the games server is down for the whole weekend since
**    athena loses so much
**      This version started the new code.  It retained the structures
**    used in bisfree as well as the general structure.  It should, 
**    in theory, play 2on2 ultimate in the moosemaze.
**      It assumes that no bullets will be fired in the game, and that
**    there will be one disc, two teams, the moosemaze, disc owner
**    slowdown at maximum, and that its teammate understands moose's
**    message (at first his teammate will be a moose as well)
**
**  ver 0.011: dachurch, August 14, 1988
**      chaser.c created, to separate the ultimate mode from the chase
**    mode.  No new functionality added, extra dead code that will be used
**    in ultimate only cut out.  Anyway, 0.02 will add vehicle dodge code
**    and such.
**      Also, now when it has the disc it holds it and trys to stutter-step.
**
**  ver 0.012: dschmidt, August 15, 1988
**      Wow, that move procedure was really heinous... let's try again...
**      The birth of roger.c, since chaser is *in* xtank...
**
**  ver 0.013: dschmidt, August 20, 1988
**      Third try on the move procedure... and it works.  Incredible.
**    Roger now dodges walls fairly cleverly, clearing walls by just the
**    right amount and making cool diagonal turns instead of boring
**    straight ones.
**      Note that this is the first version to retain the same name
**    from an earlier version, another notable achievement.
*/

#include "/homes/osl/stripes/xtank/Programs/xtanklib.h"
#include <math.h>

#include "roger.h"		/* The constants, structs, etc. */

/* This is the main routine of the moose ultimate robot
*/

roger_main()
{
  Binfo bi;
  int frame;

  roger_initialize_maze(&bi);	/* get the maze set up */
  roger_init_bi(&bi);		/* initialize the bi structure */
  set_safety(FALSE);		/* no safe turns here */
  bi.dir = heading();
  set_rel_drive(9.0);

  get_location(&bi.loc);
  (void) roger_wall(&bi,NORTH,bi.loc.grid_x,bi.loc.grid_y);

  while (TRUE) {
    frame = frame_number();
    bi.frame = frame;
    if (frame >= bi.next_frame) {
      get_location(&bi.loc);
      bi.absloc.x   = bi.loc.grid_x*BOX_WIDTH + bi.loc.box_x;
      bi.absloc.y   = bi.loc.grid_y*BOX_WIDTH + bi.loc.box_y;
      bi.myspeed    = speed();
      bi.next_frame = frame+1;
      bi.discowned  = num_discs();
      bi.mode       = (bi.discowned)?(GOT_DISC):(CHAOS);
      bi.dir        = heading();
    }

/*  if (frame%5) */
      bi.desired_dir = bi.dir;

    get_discs(&bi.numdiscs, bi.dinfo);

    switch(bi.mode) {
    case GOT_DISC:
      roger_wait_a_bit(&bi);	/* Temporary */
      break;
    case CHAOS:
      if (bi.numdiscs)
	roger_go_get_the_disc(&bi,0);
      else
	roger_move(&bi);
      break;
    default:
      roger_what_the_fuck(&bi);
      break;
    }
    
    done();
  }
}

roger_move(bi)
   Binfo *bi;
{
  Angle newdir;

  newdir = roger_turn(bi,bi->desired_dir);

  if (newdir < NO_CHANGE-.1) bi->desired_dir = newdir;
  turn_vehicle(bi->desired_dir);
}

/* This routine is only called in case of some internal error
*/
roger_what_the_fuck(bi)
   Binfo *bi;
{
  char buf[MAX_DATA_LEN];

  strcpy(buf, "Hey, who is this anyway");
  send(RECIPIENT_ALL, OP_TEXT, buf);
  bi->mode=CHAOS;
}

/* This routine stands stock still for a moment or two and then throws the disc
*/
roger_wait_a_bit(bi)
   Binfo *bi;
{
  if (!bi->discowned) {
    set_rel_drive(9.0);
    bi->mode= CHAOS;
  }
  else if (bi->midstride) {
    if (!(random()%50)) {
      bi->midstride=FALSE;
      throw_discs(25.0);
      set_rel_drive(9.0);
      bi->mode = CHAOS;		/* TEMPORARY... FOR TESTING */
    } else {
      if (!(random()%49))	/* 2% chance of direction change */
	bi->desired_dir=(((double)(random()%360))*2.0*PI)/360.0;
      if (!(random()%19))	/* 5% chance of speed change */
	set_rel_drive((double)(3+(random()%5)));
      if (!(random()%10))	/* 10% chance of disc direction change */
	spin_discs(TOGGLE);
      roger_move(bi);
    }
  } else {
    bi->midstride = TRUE;
    set_rel_drive(4.0);
  }
}

roger_go_get_the_disc(bi,discnum)
   Binfo *bi;
   int discnum;
{
  int dx,dy;
  int distsq;
  int i = 0;
  Coord discfuture;
  Location *discloc;
  char buf[MAX_DATA_LEN];

  discloc = &bi->dinfo[discnum].loc;

  discfuture.x = discloc->grid_x * BOX_WIDTH + discloc->box_x;
  discfuture.y = discloc->grid_y * BOX_HEIGHT + discloc->box_y;

  while (i++ < DISC_FRAMES) {
    roger_update_disc(bi,&discfuture,&bi->dinfo[discnum]);
    
    dx = discfuture.x - bi->absloc.x;
    dy = discfuture.y - bi->absloc.y;

    distsq = sqr(dx) + sqr(dy);

    if (distsq < sqr(((int)bi->maxspeed) * i)) {
      if (discfuture.x % BOX_WIDTH < 100 && 
	  roger_wall(bi,WEST,discfuture.x/BOX_WIDTH,discfuture.y/BOX_HEIGHT))
	continue;
      if (discfuture.y % BOX_HEIGHT < 100 && 
	  roger_wall(bi,NORTH,discfuture.y/BOX_WIDTH,discfuture.y/BOX_HEIGHT))
	continue;
      if (discfuture.x % BOX_WIDTH > BOX_WIDTH - 100 && 
	  roger_wall(bi,EAST,discfuture.x/BOX_WIDTH,discfuture.y/BOX_HEIGHT))
	continue;
      if (discfuture.y % BOX_HEIGHT < BOX_HEIGHT - 100 && 
	  roger_wall(bi,SOUTH,discfuture.x/BOX_WIDTH,discfuture.y/BOX_HEIGHT))
	continue;

      set_abs_drive((float) (sqrt((double)distsq)/(double)i));
      break;
    }
  }
  sprintf(buf,"Catch it in %d frames",i);
  send(RECIPIENT_ALL,OP_TEXT,buf);

  bi->desired_dir = atanint((float)dy, (float)dx);
  if (bi->desired_dir<0.0) bi->desired_dir+=2*PI;
  roger_move(bi);
}

/* For use by the follow disc routines
*/
roger_update_disc(bi,discloc,discinfo)
     Binfo *bi;
     Coord *discloc;
     Disc_info *discinfo;
{
  Coord oldloc, oldgridloc, discgridloc;

  oldloc.x = discloc->x;
  oldloc.y = discloc->y;

  oldgridloc.x = oldloc.x / BOX_WIDTH;
  oldgridloc.y = oldloc.y / BOX_HEIGHT;

  discloc->x += (int) discinfo->xspeed;
  discloc->y += (int) discinfo->yspeed;

  discgridloc.x = discloc->x / BOX_WIDTH;
  discgridloc.y = discloc->y / BOX_HEIGHT;

  if (oldgridloc.x == discgridloc.x - 1) {
    if (roger_wall(bi,EAST,oldgridloc.x,oldgridloc.y)) {
      discloc->x -= (discloc->x % BOX_WIDTH) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }
  else if (oldgridloc.x == discgridloc.x + 1) {
    if (roger_wall(bi,WEST,oldgridloc.x,oldgridloc.y)) {
      discloc->x += (BOX_WIDTH - (discloc->x % BOX_WIDTH)) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }

  if (oldgridloc.y == discgridloc.y - 1) {
    if (roger_wall(bi,SOUTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y -= (discloc->y % BOX_HEIGHT) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
  else if (oldgridloc.y == discgridloc.y + 1) {
    if (roger_wall(bi,NORTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y += (BOX_HEIGHT - (discloc->y % BOX_HEIGHT)) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
}

/* This silly routine initializes the Binfo structure for the tank...
*/
roger_init_bi(bi)
  Binfo *bi;
{
  bi->timing.bullet_check = 0;
  bi->timing.vehicle_check = 0;
  bi->timing.wall_check = 0;
  bi->timing.disc_check = 0;
  bi->frame = 0;
  bi->next_frame = -1;
  bi->maxspeed = max_speed();
  bi->midstride = FALSE;
}

/* This initializes the internal maze representation
*/
roger_initialize_maze(bi)
  Binfo *bi;
{
  int i,j;
   
  for (i=0; i<GRID_WIDTH; ++i)
    for (j=0; j<GRID_HEIGHT; ++j)
      bi->maze[i][j] = 0;
}

/* roger_wall is our improved version of wall,....
** used by roger_turn, of course
*/
roger_wall(bi, side, x, y)
   Binfo *bi;
   WallSide side;
   int x,y;
{
  int hx, hy;
  WallSide hside;


  if (side == SOUTH) {
    hx = x;
    hy = y + 1;
    hside = NORTH;
  }
  else if (side == EAST) {
    hx = x + 1;
    hy = y;
    hside = WEST;
  }
  else {
    hx = x;
    hy = y;
    hside = side;
  }

  if (hx<0 || hy<0 || hx>GRID_WIDTH-1 || hy>GRID_HEIGHT-1)
    return(TRUE);
   
  if (!(bi->maze[hx][hy] & SEEN)) {
    if (wall(NORTH,hx,hy)) bi->maze[hx][hy] |= NORTH_WALL;
    else bi->maze[hx][hy] &= ~NORTH_WALL;
    if (wall(WEST,hx,hy)) bi->maze[hx][hy] |= WEST_WALL;
    else bi->maze[hx][hy] &= ~WEST_WALL;
    bi->maze[hx][hy] |= SEEN;
  }

  if (hside == NORTH) {
    if (bi->maze[hx][hy] & NORTH_WALL) {
       return(TRUE); 
    }
    else {
       return(FALSE);
    }
 }
  else {			
    if (bi->maze[hx][hy] & WEST_WALL)  {
       return(TRUE); 
    }
    else {
       return(FALSE);
    }
  }
}

/* roger_turn.
**
** Now this is what I call a turn procedure.
*/

Angle roger_turn(bi,dir)
     Binfo *bi;
     Angle dir;
{
  int corner;
  int bx, by;
  int gx, gy;
  Angle delta;
  Angle idir;
  Angle min_ang = 100.0, max_ang = -100.0;
  Angle foo,bar;

  bx = bi->loc.box_x;   by = bi->loc.box_y;
  gx = bi->loc.grid_x;  gy = bi->loc.grid_y;

  idir = dir;
  fix_angle(idir);

  if (idir < PI/2) corner = SE;
  else if (idir < PI) corner = SW;
  else if (idir < 3*PI/2) corner = NW;
  else if (idir < 2*PI) corner = NE;

  switch (corner) {
  case NW:
    if (roger_wall(bi,NORTH,gx,gy)) {
      if (roger_wall(bi,WEST,gx,gy)) 
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      else {
	if (roger_wall(bi,NORTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
	  else
	    doturnstuff(atanint(bx, by), 3*PI/2, -1,
			atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      }
    }
    else if (roger_wall(bi,WEST,gx,gy)) {
      if (roger_wall(bi,WEST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(bx, by), 3*PI/2, -1);
    }
    else if (roger_wall(bi,WEST,gx,gy - 1)) {
      if (roger_wall(bi,NORTH,gx - 1,gy)) 
	doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else 
	doturnstuff(atanint(bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
    }
    else if (roger_wall(bi,NORTH,gx - 1,gy)) {
      doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		  atanint(bx, by), 3*PI/2, -1);
    }
    break;
  case SW:
    if (roger_wall(bi,SOUTH,gx,gy)) {
      if (roger_wall(bi,WEST,gx,gy))
	doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(by,bx), PI, 1);
      else {
	if (roger_wall(bi,SOUTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (roger_wall(bi,WEST,gx,gy)) {
      if (roger_wall(bi,WEST,gx,gy + 1))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(by,bx), PI, 1);
      else
	doturnstuff(atanint(bx,BOX_HEIGHT - by), PI/2, 1,
		    atanint(by,bx), PI, 1);
    }
    else if (roger_wall(bi,WEST,gx,gy + 1)) {
      if (roger_wall(bi,SOUTH,gx - 1,gy))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(bx,BOX_HEIGHT - by), PI/2, 1);
    }
    else if (roger_wall(bi,SOUTH,gx - 1, gy)) {
      doturnstuff(atanint(bx,BOX_HEIGHT - by),PI/2,1,
		  atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
    }
    break;
  case NE:
    if (roger_wall(bi,NORTH,gx,gy)) {
      if (roger_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(bx,by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else {
	if (roger_wall(bi,NORTH,gx + 1,gy))
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
	else
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
      }
    }
    else if (roger_wall(bi,EAST,gx,gy)) {
      if (roger_wall(bi,EAST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH -bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (roger_wall(bi,EAST,gx,gy - 1)) {
      if (roger_wall(bi,NORTH,gx + 1,gy))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
    }
    else if (roger_wall(bi,NORTH,gx + 1,gy))
      doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		  atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
    break;
  case SE:
    if (roger_wall(bi,SOUTH,gx,gy)) {
      if (roger_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(bx,BOX_HEIGHT - by), 5*PI/2, 1);
      else {
	if (roger_wall(bi,SOUTH,gx + 1,gy))
	  doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (roger_wall(bi,EAST,gx,gy)) {
      if (roger_wall(bi,EAST,gx,gy + 1))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (roger_wall(bi,EAST,gx,gy + 1)) {
      if (roger_wall(bi,SOUTH,gx + 1,gy))
	doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
    }
    else if (roger_wall(bi,SOUTH,gx + 1,gy))
      doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		  atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
    break;
    
  }

  if (min_ang == 100.0) {
    set_rel_drive(9.0);
    return(NO_CHANGE);
  }
  else {
    min_ang -= .3;
    max_ang += .3;
    if (min_ang < 0) {
      min_ang += 2*PI;
      max_ang += 2*PI;
    }
    if ((min_ang > 2*PI || max_ang > 2*PI) && idir < min_ang) idir += 2*PI;
    if (idir < min_ang || idir > max_ang) {
      set_rel_drive(9.0);
      return(NO_CHANGE);
    }
    else {
      if (idir - min_ang < max_ang - idir)
	idir = min_ang - .2;
      else idir = max_ang + .2;
      delta = abs(idir - bi->dir);
      if (delta > PI) delta = 2*PI - delta;
      set_rel_drive(6.0 - 4.0 * delta / PI);
      return(idir);
    }
  }
}
