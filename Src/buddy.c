/*
** thatsBUDDYtoyoubuddy
** 
** A robot program for XTANK 0.90 + up
** 
** Authors: Dan Schmidt and Doug Church <lurk slink ooooof whoops splut>
** Goal   : A robot program
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
**      Chaser.c created, to separate the ultimate mode from the chase
**    mode.  No new functionality added, extra dead code that will be used
**    in ultimate only cut out.  Anyway, 0.02 will add vehicle dodge code
**    and such.
**      Also, now when it has the disc it holds it and trys to stutter-step.
**
**  ver 0.012: dschmidt, August 15, 1988
**      Wow, that move procedure was really heinous... let's try again...
*/

#include "xtanklib.h"
#include <math.h>

#include "buddy.h"		/* The constants, structs, etc. */

/* This is the main routine of the moose ultimate robot
*/
buddy_main()
{
  Binfo bi;
  int frame;

  buddy_initialize_maze(&bi);	/* get the maze set up */
  buddy_init_bi(&bi);		/* initialize the bi structure */
  set_safety(FALSE);		/* no safe turns here */
  bi.dir = heading();
  set_rel_drive(9.0);		/* KLUDGE */

  while (TRUE) {
    frame = frame_number();
    bi.frame = frame;
    if (frame >= bi.next_frame) {
      get_self(&bi.me);
      get_landmarks(&bi.numlandmarks,bi.linfo);
      bi.absloc.x   = bi.me.loc.grid_x*BOX_WIDTH + bi.me.loc.box_x;
      bi.absloc.y   = bi.me.loc.grid_y*BOX_WIDTH + bi.me.loc.box_y;
      bi.myspeed    = speed();
      bi.next_frame = frame+1;
      bi.discowned  = num_discs();
      bi.mode       = (bi.discowned)?(GOT_DISC):(CHAOS);
      get_vehicles(&bi.numvehicles, bi.vinfo);
      get_discs(&bi.numdiscs, bi.dinfo);
    }

    buddy_look_for_goals(&bi);

    switch(bi.mode) {
    case GOT_DISC:
      buddy_deal_with_disc(&bi);	/* Temporary */
      break;
    case CHAOS:
      if (bi.numdiscs)
	buddy_go_get_the_disc(&bi,0);
      else
	buddy_move(&bi);
      break;
    default:
      buddy_whatever(&bi);
      break;
    }
    
    done();
  }
}

buddy_look_for_goals(bi)
     Binfo *bi;
{
  int i;
  char buf[MAX_DATA_LEN];

  if (bi->numlandmarks > bi->lastnumlandmarks) {
    for (i = bi->lastnumlandmarks; i < bi->numlandmarks; ++i) {
      if (bi->linfo[i].type == GOAL && bi->linfo[i].team != bi->me.team) {
	bi->goalinfo[bi->numgoals].x = bi->linfo[i].x;
	bi->goalinfo[bi->numgoals].y = bi->linfo[i].y;
	bi->goalinfo[bi->numgoals].team = bi->linfo[i].team;
	++bi->numgoals;
      }
    }
    bi->lastnumlandmarks = bi->numlandmarks;
  }
}

buddy_move(bi)
   Binfo *bi;
{
  Angle newdir;

  newdir = buddy_turn(bi,bi->dir);

  if (newdir < NO_CHANGE-.1) bi->dir = newdir;
  turn_vehicle(bi->dir);
}

buddy_move2(bi)
   Binfo *bi;
{
  Angle newdir;

  newdir = buddy_manuever(bi,bi->dir);

  if (newdir < NO_CHANGE-.1) bi->dir = newdir;
  turn_vehicle(bi->dir);
}

/* This routine is only called in case of some internal error
*/
buddy_whatever(bi)
   Binfo *bi;
{
  char buf[MAX_DATA_LEN];

  strcpy(buf, "Hey, who is this anyway");
  send(RECIPIENT_ALL, OP_TEXT, buf);
  bi->mode=CHAOS;
}

buddy_deal_with_disc(bi)
     Binfo *bi;
{
  int i;
  int enemy;
  int dx,dy;
  int dist,distsqr;
  int mindist = 999;
  int closestgoal = -1;
  Location goal_loc;
  char buf[MAX_DATA_LEN];

  if (!bi->discowned) {
    set_rel_drive(9.0);
    bi->midstride = FALSE;
    bi->mode = CHAOS;
  }
  else if (bi->numgoals) {
    for (i=0;i<bi->numgoals;++i) {
      bi->goalinfo[i].distance = abs(bi->goalinfo[i].x - bi->me.loc.grid_x) +
	                     abs(bi->goalinfo[i].y - bi->me.loc.grid_y);
      mindist = min(mindist,bi->goalinfo[i].distance);
      closestgoal = i;
    }
    if (closestgoal != -1) {
      goal_loc.grid_x = bi->goalinfo[closestgoal].x;
      goal_loc.grid_y = bi->goalinfo[closestgoal].y;
      goal_loc.box_x = BOX_WIDTH / 2;
      goal_loc.box_y = BOX_HEIGHT / 2;
      if (clear_path(&bi->dinfo[0].loc,&goal_loc)) {
	bi->midstride = FALSE;
	buddy_throw_towards(bi,&goal_loc,TRUE);
	return;
      }
      else {
	dx = bi->goalinfo[closestgoal].x * BOX_WIDTH + 
	     BOX_WIDTH/2 - bi->absloc.x;
	dy = bi->goalinfo[closestgoal].y * BOX_HEIGHT + 
	     BOX_HEIGHT/2 - bi->absloc.y;
	bi->dir = atanint(dy,dx);
	buddy_move2(bi);
      }
    }
  }

  if (bi->numvehicles) {
    for (i=0;i<bi->numvehicles;++i) {
      if (bi->vinfo[i].team == bi->me.team) {
	if (clear_path(&bi->dinfo[0].loc,&bi->vinfo[i].loc)) {
	  dx = bi->vinfo[i].loc.grid_x * BOX_WIDTH +
	       bi->vinfo[i].loc.box_x - bi->absloc.x;
	  dy = bi->vinfo[i].loc.grid_y * BOX_HEIGHT +
	       bi->vinfo[i].loc.box_y - bi->absloc.y;
	  distsqr = sqr(dx) + sqr(dy);
	  if (distsqr > BOX_WIDTH*BOX_WIDTH*2) {
	    bi->midstride = FALSE;
	    dist = sqrtint(distsqr);
	    bi->vinfo[i].loc.box_x += ((int)bi->vinfo[i].xspeed) * dist / 25;
	    bi->vinfo[i].loc.box_y += ((int)bi->vinfo[i].yspeed) * dist / 25;
	    buddy_throw_towards(bi,&bi->vinfo[i].loc,TRUE);
	  }
	}
      }
    }
  }
  buddy_play_keep_away(bi);
  buddy_move2(bi,bi->dir);
}

buddy_play_keep_away(bi)
     Binfo *bi;
{
  int i;
  int dx,dy;
  int distsqr;
  int cdx,cdy;
  int closestdistsqr = VEHICLE_TOO_CLOSE * VEHICLE_TOO_CLOSE;
  int closestvehicle = -1;
  Angle vangle,dangle,delta;
  char buf[MAX_DATA_LEN];

  if (bi->numvehicles)
    for (i=0;i<bi->numvehicles;++i) {
      if (bi->vinfo[i].team != bi->me.team) {
	dx = bi->vinfo[i].loc.grid_x * BOX_WIDTH +
	  bi->vinfo[i].loc.box_x +
	  (int) bi->vinfo[i].xspeed -
	  bi->absloc.x;
	dy = bi->vinfo[i].loc.grid_y * BOX_HEIGHT +
	  bi->vinfo[i].loc.box_y +
	  (int) bi->vinfo[i].yspeed - 
	  bi->absloc.y;
	distsqr = (sqr(dx) + sqr(dy));

	if (distsqr < closestdistsqr) {
	  closestdistsqr = distsqr;
	  closestvehicle = i;
	  cdx = dx;  cdy = dy;
	}
      }
    }
  if (distsqr < sqr(VEHICLE_TOO_CLOSE)) {
    vangle = atanint(cdy,cdx);
    dangle = bi->dinfo[0].angle;
    delta = dangle - vangle;
    fix_angle(delta);
    if (delta < PI) spin_discs(CLOCKWISE);
    else spin_discs(COUNTERCLOCKWISE);
    return(TRUE);
  }
  return(FALSE);
}

buddy_throw_towards(bi,tloc,newthrow)
     Binfo *bi;
     Location *tloc;
     int newthrow;
{
  static Location loc;
  Angle alpha,beta,delta;
  char buf[MAX_DATA_LEN];

  if (newthrow) {
    loc.grid_x = tloc->grid_x;
    loc.grid_y = tloc->grid_y;
    loc.box_x = tloc->box_x;
    loc.box_y = tloc->box_y;
  }

  alpha = atanint(loc.grid_y * BOX_HEIGHT + loc.box_y - bi->absloc.y,
		  loc.grid_x * BOX_WIDTH + loc.box_x - bi->absloc.x);

  get_discs(&bi->numdiscs,bi->dinfo);
  beta = bi->dinfo[0].angle;
  delta = beta - alpha;
  fix_angle(delta);

  if (delta < PI/2 + .2 - THROW_THRESHOLD)
    spin_discs(CLOCKWISE);
  else if (delta < PI/2 + .2) {
    spin_discs(COUNTERCLOCKWISE);
    throw_discs(25.0,TRUE);
    bi->midstride = FALSE;
  }
  else if (delta < PI)
    spin_discs(COUNTERCLOCKWISE);
  else if (delta < 3*PI/2 - .2)
    spin_discs(CLOCKWISE);
  else if (delta < 3*PI/2 - .2 + THROW_THRESHOLD) {
    spin_discs(CLOCKWISE);
    throw_discs(25.0,TRUE);
    bi->midstride = FALSE;
  }
  else
    spin_discs(COUNTERCLOCKWISE);
}

buddy_go_get_the_disc(bi,discnum)
   Binfo *bi;
   int discnum;
{
  int dx,dy;
  Angle heading;
  int distsq;
  int i = 0;
  Coord discfuture;
  Location *discloc;
  int owner;

  owner = bi->dinfo[discnum].owner;

  if (owner == NO_OWNER || team(owner) != bi->me.team) {
    
    discloc = &bi->dinfo[discnum].loc;
    
    discfuture.x = discloc->grid_x * BOX_WIDTH + discloc->box_x;
    discfuture.y = discloc->grid_y * BOX_HEIGHT + discloc->box_y;
    
    while (i++ < DISC_FRAMES) {
      buddy_update_disc(bi,&discfuture,&bi->dinfo[discnum]);
      
      dx = discfuture.x - bi->absloc.x;
      dy = discfuture.y - bi->absloc.y;
      
      distsq = sqr(dx) + sqr(dy);
      
      if (distsq < sqr(((int)bi->maxspeed) * i)) {
	set_rel_drive(9.0);
	break;
      }
    }
    bi->dir = atanint(dy, dx);
    if (bi->dir<0.0) bi->dir+=2*PI;
    buddy_move(bi);
  }
}

/* For use by the follow disc routines
*/
buddy_update_disc(bi,discloc,discinfo)
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
    if (buddy_wall(bi,EAST,oldgridloc.x,oldgridloc.y)) {
      discloc->x -= (discloc->x % BOX_WIDTH) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }
  else if (oldgridloc.x == discgridloc.x + 1) {
    if (buddy_wall(bi,WEST,oldgridloc.x,oldgridloc.y)) {
      discloc->x += (BOX_WIDTH - (discloc->x % BOX_WIDTH)) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }

  if (oldgridloc.y == discgridloc.y - 1) {
    if (buddy_wall(bi,SOUTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y -= (discloc->y % BOX_HEIGHT) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
  else if (oldgridloc.y == discgridloc.y + 1) {
    if (buddy_wall(bi,NORTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y += (BOX_HEIGHT - (discloc->y % BOX_HEIGHT)) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
}

/* This silly routine initializes the Binfo structure for the tank...
*/
buddy_init_bi(bi)
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
  bi->lastnumlandmarks = 0;
  bi->numgoals = 0;
  get_self(&bi->me);
}

/* This initializes the internal maze representation
*/
buddy_initialize_maze(bi)
  Binfo *bi;
{
  int i,j;
   
  for (i=0; i<GRID_WIDTH; ++i)
    for (j=0; j<GRID_HEIGHT; ++j)
      bi->maze[i][j] = 0;
}

/* buddy_wall is our improved version of wall,....
** used by buddy_turn, of course
*/
buddy_wall(bi, side, x, y)
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
    if (wall(NORTH,hx,hy)) bi->maze[hx][hy] |= BUDDY_NORTH_WALL;
    else bi->maze[hx][hy] &= ~BUDDY_NORTH_WALL;
    if (wall(WEST,hx,hy)) bi->maze[hx][hy] |= BUDDY_WEST_WALL;
    else bi->maze[hx][hy] &= ~BUDDY_WEST_WALL;
    bi->maze[hx][hy] |= SEEN;
  }

  if (hside == NORTH) {
    if (bi->maze[hx][hy] & BUDDY_NORTH_WALL) return(TRUE); else return(FALSE);
  } else {			/* hside == WEST */
    if (bi->maze[hx][hy] & BUDDY_WEST_WALL)  return(TRUE); else return(FALSE);
  }
}

/*
** buddy_turn, take 2.
** 
** It gets passed in an Angle saying which way the vehicle wants to go.
** It turns that way if it's a good idea to do so.  Otherwise, it doesn't.
**
** Right now it sucks.  Let's hope it's better than take 1 anyways.
*/

Angle buddy_turn(bi,dir)
     Binfo *bi;
     Angle dir;
{
  if (dir > PI/4 && dir < 3*PI/4) {	        /* goin' south */
    if (!buddy_wall(bi,SOUTH,bi->me.loc.grid_x,bi->me.loc.grid_y)) {
      return(NO_CHANGE);
    } 
    else if (bi->me.loc.box_y > BOX_HEIGHT - TOO_CLOSE) {
      if (!buddy_wall(bi,WEST,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(PI);
      else if (!buddy_wall(bi,EAST,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(0.0);
      else return(3*PI/2);
    }
    else return(NO_CHANGE);
  }
  else if (dir >= 3*PI/4 && dir < 5*PI/4) {	/* goin' west */
    if (!buddy_wall(bi,WEST,bi->me.loc.grid_x,bi->me.loc.grid_y)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_x < TOO_CLOSE) {
      if (!buddy_wall(bi,NORTH,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(3*PI/2);
      else if (!buddy_wall(bi,SOUTH,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(PI/2);
      else return(0.0);
    }
    else return(NO_CHANGE);
  }
  else if (dir >= 5*PI/4 && dir < 7*PI/4) { /* goin' north */
    if (!buddy_wall(bi,NORTH,bi->me.loc.grid_x,bi->me.loc.grid_y)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_y < TOO_CLOSE) {
      if (!buddy_wall(bi,EAST,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(0.0);
      else if (!buddy_wall(bi,WEST,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(PI);
      else return(PI/2);
    }
    else return(NO_CHANGE);
  }
  else {			                /* goin' east */
    if (!buddy_wall(bi,EAST,bi->me.loc.grid_x,bi->me.loc.grid_y)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_x > BOX_WIDTH - TOO_CLOSE) {
      if (!buddy_wall(bi,SOUTH,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(PI/2);
      else if (!buddy_wall(bi,NORTH,bi->me.loc.grid_x,bi->me.loc.grid_y))
	return(3*PI/2);
      else return(PI);
    }
    else return(NO_CHANGE);
  }
}

/* buddy_manuever()
**
** This is used when we want to avoid an obstacle.  We give it a direction
** we want to go.  Repeatedly calling it with the direction to the goal
** should eventually get it there.
*/

Angle buddy_manuever(bi,dir)
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

  bx = bi->me.loc.box_x;   by = bi->me.loc.box_y;
  gx = bi->me.loc.grid_x;  gy = bi->me.loc.grid_y;

  idir = dir;
  fix_angle(idir);

  if (idir < PI/2) corner = SE;
  else if (idir < PI) corner = SW;
  else if (idir < 3*PI/2) corner = NW;
  else if (idir < 2*PI) corner = NE;

  switch (corner) {
  case NW:
    if (buddy_wall(bi,NORTH,gx,gy)) {
      if (buddy_wall(bi,WEST,gx,gy)) 
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      else {
	if (buddy_wall(bi,NORTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
	  else
	    doturnstuff(atanint(bx, by), 3*PI/2, -1,
			atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      }
    }
    else if (buddy_wall(bi,WEST,gx,gy)) {
      if (buddy_wall(bi,WEST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(bx, by), 3*PI/2, -1);
    }
    else if (buddy_wall(bi,WEST,gx,gy - 1)) {
      if (buddy_wall(bi,NORTH,gx - 1,gy)) 
	doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else 
	doturnstuff(atanint(bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
    }
    else if (buddy_wall(bi,NORTH,gx - 1,gy)) {
      doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		  atanint(bx, by), 3*PI/2, -1);
    }
    break;
  case SW:
    if (buddy_wall(bi,SOUTH,gx,gy)) {
      if (buddy_wall(bi,WEST,gx,gy))
	doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(by,bx), PI, 1);
      else {
	if (buddy_wall(bi,SOUTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (buddy_wall(bi,WEST,gx,gy)) {
      if (buddy_wall(bi,WEST,gx,gy + 1))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(by,bx), PI, 1);
      else
	doturnstuff(atanint(bx,BOX_HEIGHT - by), PI/2, 1,
		    atanint(by,bx), PI, 1);
    }
    else if (buddy_wall(bi,WEST,gx,gy + 1)) {
      if (buddy_wall(bi,SOUTH,gx - 1,gy))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(bx,BOX_HEIGHT - by), PI/2, 1);
    }
    else if (buddy_wall(bi,SOUTH,gx - 1, gy)) {
      doturnstuff(atanint(bx,BOX_HEIGHT - by),PI/2,1,
		  atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
    }
    break;
  case NE:
    if (buddy_wall(bi,NORTH,gx,gy)) {
      if (buddy_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(bx,by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else {
	if (buddy_wall(bi,NORTH,gx + 1,gy))
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
	else
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
      }
    }
    else if (buddy_wall(bi,EAST,gx,gy)) {
      if (buddy_wall(bi,EAST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH -bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (buddy_wall(bi,EAST,gx,gy - 1)) {
      if (buddy_wall(bi,NORTH,gx + 1,gy))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
    }
    else if (buddy_wall(bi,NORTH,gx + 1,gy))
      doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		  atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
    break;
  case SE:
    if (buddy_wall(bi,SOUTH,gx,gy)) {
      if (buddy_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(bx,BOX_HEIGHT - by), 5*PI/2, 1);
      else {
	if (buddy_wall(bi,SOUTH,gx + 1,gy))
	  doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (buddy_wall(bi,EAST,gx,gy)) {
      if (buddy_wall(bi,EAST,gx,gy + 1))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (buddy_wall(bi,EAST,gx,gy + 1)) {
      if (buddy_wall(bi,SOUTH,gx + 1,gy))
	doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
    }
    else if (buddy_wall(bi,SOUTH,gx + 1,gy))
      doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		  atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
    break;
    
  }

  if (min_ang == 100.0) {
    set_rel_drive(9.0);
    return(NO_CHANGE);
  }
  else {
    min_ang -= .5;
    max_ang += .5;
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
