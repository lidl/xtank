/*
 ** thatsPRYNtoyoubuddy
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
 **    longer bullets since they're harder to catch
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
 **
 **  ver 0.020: dschmidt, late August, 1988
 **      Buddy.c, now plays a decent game of ultimate, hard to beat in
 **    one on one.  Features include 
 **    * throwing discs into enemy goals
 **    * keeping the disc from nearby enemies
 **    * passing to open teammates (pretty bad at this point)
 **
 **  Sept. 6 1988: buddy is put into xtank itself, this version 
 **                renamed to frog
 **
 **  Dec. 18 1988: first term is over, maybe now we can get some work done
 **                again.
 **
 **  Feb 1, 1989....
 **                How bout that IAP?
 **                Doug needs to get a clue and turns to trying to figure out
 **                frog (now pryn......)
 */

#include "/mit/games/src/vax/xtank/Programs/xtanklib.h"
#include <math.h>

#include "pryn.h"		/* The constants, structs, etc. */

/* pryn_main()
** The main procedure.
** Initializes stuff and then goes through an infinite loop until the
** game is reset for whatever reason.
*/
pryn_main()
{
  Binfo bi;			/* Everything you ever wanted to know */
  Message dummy;		/* for controlling it..... */
  
  pryn_initialize_maze(&bi);	/* Get the maze set up */
  pryn_init_bi(&bi);		/* Initialize the bi structure */
  set_safety(FALSE);		/* No safe turns here */
  set_rel_drive(9.0);		/* Let's cruise */
  
  while (TRUE) {
    bi.frame = frame_number();
    
    /* Are we on the next frame yet? */
    if (bi.frame >= bi.next_frame) {
      /* Get external information */
      get_self(&bi.me);
      get_landmarks(&bi.numlandmarks,bi.linfo);
      get_blips(&bi.numblips,bi.blinfo);
      bi.myspeed    = speed();
      bi.next_frame = bi.frame + 1;
      bi.discowned  = num_discs();
      bi.mode       = (bi.discowned)?(GOT_DISC):(CHAOS);
      bi.cur_tc     = bi.myspeed * TOO_CLOSE;
      get_vehicles(&bi.numvehicles, bi.vinfo);
      get_discs(&bi.numdiscs, bi.dinfo);
    }
    
    pryn_look_for_goals(&bi);

if (bi.midstride == FALSE)
  pryn_move(&bi,bi.desireddir);
if (messages()!=bi.lastmessages) {
  printf("In here heading %lf with %lf\n",
	 (double)bi.me.heading, (double)bi.cur_tc);
  bi.midstride = ((bi.midstride == FALSE)?(TRUE):(FALSE));
  bi.desireddir = bi.me.heading;
  bi.lastmessages = messages();
}


/*
    switch(bi.mode) {
    case GOT_DISC:
      pryn_deal_with_disc(&bi);
      break;
    case CHAOS:
      if (bi.numdiscs)
	pryn_go_get_the_disc(&bi,0);
      else
	pryn_find_the_action(&bi);
      break;
    default:
      pryn_what_the_fuck(&bi);
      break;
    }
*/
    done();

  }
}

/* pryn_look_for_goals(bi)
** Looks around to see if there are any goal locations that it hasn't
** seen yet.  If so, it puts the appropriate information in bi.ginfo.
*/
pryn_look_for_goals(bi)
     Binfo *bi;
{
  int i;
  
  if (bi->numlandmarks > bi->lastnumlandmarks) {
    for (i = bi->lastnumlandmarks; i < bi->numlandmarks; ++i) {
      if (bi->linfo[i].type == GOAL) {
	bi->ginfo[bi->numgoals].x = bi->linfo[i].x;
	bi->ginfo[bi->numgoals].y = bi->linfo[i].y;
	bi->ginfo[bi->numgoals].team = bi->linfo[i].team;
	if (bi->linfo[i].team == bi->me.team) 
	  bi->ginfo[bi->numgoals].enemy = FALSE;
	else
	  bi->ginfo[bi->numgoals].enemy = TRUE;
	++bi->numgoals;
      }
    }
  }
  bi->lastnumlandmarks = bi->numlandmarks;
}

/* pryn_move(bi,dir)
** Turns in the direction dir, not worrying that much about whether there's
** a wall there or not.  If there is, avoid it a bit.
*/
pryn_move(bi,dir)
     Binfo *bi;
     Angle dir;
{
  Angle newdir;
  
  newdir = pryn_turn(bi,dir);
  
  if (newdir < NO_CHANGE-.1) bi->desireddir = newdir;
  else bi->desireddir = dir;
  turn_vehicle(bi->desireddir);
}

/* pryn_move_carefully(bi,dir)
** Turns in the direction dir.  If there's an obstacle nearby, avoid it
** by the minimum amount possible.
*/
pryn_move_carefully(bi,dir)
     Binfo *bi;
     Angle dir;
{
  Angle newdir;
  
  newdir = pryn_turn_carefully(bi,dir);
  
  if (newdir < NO_CHANGE-.1) bi->desireddir = newdir;
  else bi->desireddir = dir;
  turn_vehicle(bi->desireddir);
}

/* pryn_what_the_fuck(bi)
** This routine is only called in case of some internal error.
** And that never happens, right?
*/
pryn_what_the_fuck(bi)
     Binfo *bi;
{
  char buf[MAX_DATA_LEN];
  
  strcpy(buf, "Hey, who is this anyway");
  send(RECIPIENT_ALL, OP_TEXT, buf);
  bi->mode = CHAOS;
}

/* pryn_deal_with_disc(bi)
** This procedure is caled when the pryn has the disc and needs to know
** what to do with it.
*/
pryn_deal_with_disc(bi)
     Binfo *bi;
{
  if (!bi->discowned) {
    /* We must have lost it somewhere */
    set_rel_drive(9.0);
    bi->midstride = FALSE;
    bi->mode = CHAOS;
  }
  else {
    pryn_head_for_goal(bi);
    pryn_maybe_pass(bi,BOX_WIDTH*BOX_WIDTH*2);
    pryn_play_keep_away(bi);
  }
}

/* pryn_head_for_goal(bi)
** The pryn looks at the closest goal.
**   If it's an enemy goal, he moves toward it.
**   If it's his own goal, he moves away from it.
** If he doesn't know about any goals, he keeps on going straight, carefully
** since he has the disc.
*/
pryn_head_for_goal(bi)
     Binfo *bi;
{
  int i;
  int dx,dy;			/* deltas to the closest goal */
  int mindist = 999;		/* distance of closest goal */
  int closestgoal = -1;		/* number of closest goal */
  Boolean isenemygoal;
  Location goal_loc;
  
  if (bi->numgoals) {		/* only if we've found one */
    for (i=0;i<bi->numgoals;++i) {
      /* just use an approximation for distance, who cares about sqrts and
	 all that */
      bi->ginfo[i].distance = 
	abs(bi->ginfo[i].x - bi->me.loc.grid_x) +
	abs(bi->ginfo[i].y - bi->me.loc.grid_y);
      if (bi->ginfo[i].distance < mindist) {
	mindist = bi->ginfo[i].distance;
	closestgoal = i;
      }
    }

    if (bi->ginfo[closestgoal].enemy) isenemygoal = TRUE;
    else isenemygoal = FALSE;

    goal_loc.grid_x = bi->ginfo[closestgoal].x;
    goal_loc.grid_y = bi->ginfo[closestgoal].y;
    goal_loc.box_x = BOX_WIDTH / 2;
    goal_loc.box_y = BOX_HEIGHT / 2;
    goal_loc.x = goal_loc.grid_x * BOX_WIDTH + goal_loc.box_x;
    goal_loc.y = goal_loc.grid_y * BOX_HEIGHT + goal_loc.box_y;
    
    dx = goal_loc.x - bi->me.loc.x;
    dy = goal_loc.y - bi->me.loc.y;

    if (isenemygoal) {
      /* move towards it, carefully */
      bi->desireddir = atanint(dy,dx);
      pryn_move_carefully(bi,bi->desireddir);

      if (bi->discowned)
	pryn_throw_at_goal(bi,closestgoal);
    }
    else {
      /* move away from it, carefully */
      bi->desireddir = atanint(dy,dx) + PI;
      pryn_move_carefully(bi,bi->desireddir);
    }
  }
  else pryn_move_carefully(bi,bi->me.heading);
}

/* pryn_throw-at_goal(bi,goalnum)
** Throw the disc at the goal numbered goalnum, if there's a clear path.
*/
pryn_throw_at_goal(bi,goalnum)
     Binfo *bi;
     int goalnum;
{
  Location goal_loc;

  goal_loc.grid_x = bi->ginfo[goalnum].x;
  goal_loc.grid_y = bi->ginfo[goalnum].y;
  goal_loc.box_x = BOX_WIDTH / 2;
  goal_loc.box_y = BOX_HEIGHT / 2;
  goal_loc.x = goal_loc.grid_x * BOX_WIDTH + goal_loc.box_x;
  goal_loc.y = goal_loc.grid_y * BOX_HEIGHT + goal_loc.box_y;

  if (clear_path(&bi->dinfo[0].loc,&goal_loc)) {
    bi->midstride = FALSE;
    pryn_throw_at_loc(bi,&goal_loc,TRUE);
    return;
  }
}

/* pryn_maybe_pass(bi,mindistsqr)
** Passes the disc to a teammate if it seems like a good idea.
** Current criteria are:
**   Must be a clear path to the teammate.
**   The teammate's distance squared must be > mindistsqr.
** There should probably be rules about
**   Am I double-teamed?
**   Is my teammate open? (no enemies near him)
**   Is he in a better position than me?
*/
pryn_maybe_pass(bi,mindistsqr)
     Binfo *bi;
     int mindistsqr;
{
  int i, distsqr;
  int perhapspass;
  int dx,dy,dist;
  
  if (bi->numvehicles) {
    for (i=0;i<bi->numvehicles;++i) {
      if (bi->vinfo[i].team == bi->me.team) {
	if (clear_path(&bi->dinfo[0].loc,&bi->vinfo[i].loc)) {
	  dx = bi->vinfo[i].loc.x - bi->me.loc.x;
	  dy = bi->vinfo[i].loc.y - bi->me.loc.y;
	  distsqr = sqr(dx) + sqr(dy);
	  
	  if (distsqr > mindistsqr) {
	    bi->midstride = FALSE;
	    dist = sqrtint(distsqr);
	    bi->vinfo[i].loc.box_x += ((int)bi->vinfo[i].xspeed) * dist / 25;
	    bi->vinfo[i].loc.box_y += ((int)bi->vinfo[i].yspeed) * dist / 25;
	    pryn_throw_at_loc(bi,&bi->vinfo[i].loc,TRUE);
	  }
	}
      }
    }
  }
}

/* pryn_play_keep_away(bi)
** Makes sure no one steals the disc.
** Returns TRUE if there is a vehicle close enough that it has to worry.
*/
pryn_play_keep_away(bi)
     Binfo *bi;
{
  int i;
  int dx,dy;
  int distsqr;
  int cdx,cdy;			/* deltas of closest vehicle */
  int closestdistsqr = VEHICLE_TOO_CLOSE * VEHICLE_TOO_CLOSE;
  int closestvehicle = -1;
  int numclosevehicles = 0;
  Angle vangle,dangle,delta;
  char buf[MAX_DATA_LEN];
  
  if (bi->numvehicles)
    for (i=0;i<bi->numvehicles;++i) {
      if (bi->vinfo[i].team != bi->me.team) {
	dx = bi->vinfo[i].loc.x + (int) bi->vinfo[i].xspeed - bi->me.loc.x;
	dy = bi->vinfo[i].loc.y + (int) bi->vinfo[i].yspeed - bi->me.loc.y;
	distsqr = (sqr(dx) + sqr(dy));
	
	if (distsqr < closestdistsqr) {
	  numclosevehicles++;
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

/* pryn_throw_at_loc(bi,tloc,newthrow)
** Throws the disc at the location tloc.
** newthrow specifies whether we are attempting a new throw (TRUE) or
** continuing an old throw (FALSE).
*/
pryn_throw_at_loc(bi,tloc,newthrow)
     Binfo *bi;
     Location *tloc;
     int newthrow;
{
  static Location loc;
  Angle alpha,beta;
  char buf[MAX_DATA_LEN];
  
  if (newthrow) {
    loc.grid_x = tloc->grid_x;
    loc.grid_y = tloc->grid_y;
    loc.box_x = tloc->box_x;
    loc.box_y = tloc->box_y;
  }
  
  alpha = atanint(loc.grid_y * BOX_HEIGHT + loc.box_y - bi->me.loc.y,
		  loc.grid_x * BOX_WIDTH + loc.box_x - bi->me.loc.x);
  
  beta = bi->dinfo[0].angle;
  pryn_throw(bi,beta - alpha);
}

/* pryn_throw_in_dir(bi,dir)
** Throws the disc in the direction dir.
*/
pryn_throw_in_dir(bi,dir)
     Binfo *bi;
     Angle dir;
{
  pryn_throw(bi,dir - bi->dinfo[0].angle);
}

/* pryn_throw(bi,delta)
** Throws the disc according to delta, which is some strange variable
** specifying where the disc is in relation to where it should be.
** I'll sit down and figure out all out again some day.
*/
pryn_throw(bi,delta)
     Binfo *bi;
     Angle delta;
{
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

/* pryn_find_the_action(bi)
** This procedure is called if the pryn does not have the disc and doesn't
** see a disc.  It causes the pryn to move towards the center of mass of
** all other vehicles.
*/
pryn_find_the_action(bi)
     Binfo *bi;
{
  int i;
  int x,y;
  int blips;			
  int dist;
  int maxdist = 0;
  Location action;
  Angle dir;
  char buf[MAX_DATA_LEN];

  if (bi->numblips) {
    x = 0; y = 0; blips = 0;

    for (i=0;i<bi->numblips;++i) {
      /* only look at ones out of view */
      if (bi->blinfo[i].x < bi->me.loc.grid_x - 2 ||
	  bi->blinfo[i].x > bi->me.loc.grid_x + 2 ||
	  bi->blinfo[i].y < bi->me.loc.grid_y - 2 ||
	  bi->blinfo[i].y > bi->me.loc.grid_y + 2)
	{
	  x += bi->blinfo[i].x;
	  y += bi->blinfo[i].y;
	  blips++;
	}
    }

    if (blips) {
      action.grid_x = x / blips;
      action.grid_y = y / blips;
/*
      action.box_x = BOX_WIDTH / 2;
      action.box_y = BOX_HEIGHT / 2;
      action.x = action.grid_x * BOX_WIDTH + action.box_x;
      action.y = action.grid_y * BOX_HEIGHT + action.box_y;
*/
      sprintf(buf,"Go to %2d %2d",action.grid_x, action.grid_y);
      send(bi->me.id,OP_TEXT,buf);
      dir = atanint(action.grid_y - bi->me.loc.grid_y,
		    action.grid_x - bi->me.loc.grid_x);

      pryn_move_carefully(bi,dir);
    }
    else {
      pryn_move_carefully(bi,bi->me.heading);
    }
  }
  else {
    pryn_move_carefully(bi,bi->me.heading);
  }
}

/* pryn_head_for_loc(bi,loc)
** Causes the pryn to head for loc, avoiding obstacles the tricky way.
*/
pryn_head_for_loc(bi,loc)
     Binfo *bi;
     Location *loc;
{
  char buf[MAX_DATA_LEN];
  int dx,dy;

  dx = loc->x - bi->me.loc.x;
  dy = loc->y - bi->me.loc.y;

  pryn_move_carefully(bi,atanint(dy,dx));
}

/* pryn_go_get_the_disc(bi,discnum)
** Causes the pryn to chase the disc, only if it is unowned or 
** enemy controlled.  Otherwise, it heads towards an enemy goal
** (or away from its own)
*/
pryn_go_get_the_disc(bi,discnum)
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
    
    discfuture.x = discloc->x;
    discfuture.y = discloc->y;
    
    while (i++ < DISC_FRAMES) {
      pryn_update_disc(bi,&discfuture,&bi->dinfo[discnum]);
      
      dx = discfuture.x - bi->me.loc.x;
      dy = discfuture.y - bi->me.loc.y;
      
      distsq = sqr(dx) + sqr(dy);
      
      if (distsq < sqr(((int)bi->maxspeed) * i)) {
	set_abs_drive((float)(sqrtint(distsq) / i));
	break;
      }
    }
    discloc->x = discfuture.x;
    discloc->y = discfuture.y;
    discloc->grid_x = discfuture.x / BOX_WIDTH;
    discloc->grid_y = discfuture.y / BOX_HEIGHT;
    discloc->box_x = discfuture.x % BOX_WIDTH;
    discloc->box_y = discfuture.y % BOX_HEIGHT;

    bi->desireddir = atanint(dy, dx);
    if (bi->desireddir<0.0) bi->desireddir+=2*PI;

    if (clear_path(&bi->me.loc,discloc))
      pryn_move(bi,bi->desireddir);
    else
      pryn_move_carefully(bi,bi->desireddir);
  }
  
  else				/* a friend owns it */
    pryn_head_for_goal(bi);
}

/* For use by the follow disc routines
 */
pryn_update_disc(bi,discloc,discinfo)
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
    if (pryn_wall(bi,EAST,oldgridloc.x,oldgridloc.y)) {
      discloc->x -= (discloc->x % BOX_WIDTH) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }
  else if (oldgridloc.x == discgridloc.x + 1) {
    if (pryn_wall(bi,WEST,oldgridloc.x,oldgridloc.y)) {
      discloc->x += (BOX_WIDTH - (discloc->x % BOX_WIDTH)) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }
  
  if (oldgridloc.y == discgridloc.y - 1) {
    if (pryn_wall(bi,SOUTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y -= (discloc->y % BOX_HEIGHT) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
  else if (oldgridloc.y == discgridloc.y + 1) {
    if (pryn_wall(bi,NORTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y += (BOX_HEIGHT - (discloc->y % BOX_HEIGHT)) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }

  discinfo->xspeed *= bi->settings.disc_friction;
  discinfo->yspeed *= bi->settings.disc_friction;
}

/* This silly routine initializes the Binfo structure for the tank...
 */
pryn_init_bi(bi)
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
  bi->lastmessages = messages();
  get_self(&bi->me);
  get_settings(&bi->settings);
}

/* This initializes the internal maze representation
 */
pryn_initialize_maze(bi)
     Binfo *bi;
{
  int i,j;
  
  for (i=0; i<GRID_WIDTH; ++i)
    for (j=0; j<GRID_HEIGHT; ++j)
      bi->maze[i][j] = 0;
}

/* pryn_wall is our improved version of wall,....
 ** used by pryn_turn, of course
 */
pryn_wall(bi, side, x, y)
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
    if (bi->maze[hx][hy] & NORTH_WALL) return(TRUE); else return(FALSE);
  } else {			/* hside == WEST */
    if (bi->maze[hx][hy] & WEST_WALL)  return(TRUE); else return(FALSE);
  }
}

/*
 ** pryn_turn, take 2.
 ** 
 ** It gets passed in an Angle saying which way the vehicle wants to go.
 ** It turns that way if it's a good idea to do so.  Otherwise, it doesn't.
 **
 ** Right now it sucks.  Let's hope it's better than take 1 anyways.
 */

Angle pryn_turn(bi,dir)
     Binfo *bi;
     Angle dir;
{
  int gx,gy;
  
  gx = bi->me.loc.grid_x;
  gy = bi->me.loc.grid_y;
  
  if (dir > PI/4 && dir < 3*PI/4) {	        /* goin' south */
    if (!pryn_wall(bi,SOUTH,gx,gy)) {
      return(NO_CHANGE);
    } 
    else if (bi->me.loc.box_y > BOX_HEIGHT - bi->cur_tc) {
      if (pryn_wall(bi,WEST,gx,gy)) return(0.0);
      else if (pryn_wall(bi,EAST,gx,gy)) return(PI);
      else {
	if (dir < PI/2) return(0.0);
	else return(PI);
      }
    }
    else return(NO_CHANGE);
  }
  else if (dir >= 3*PI/4 && dir < 5*PI/4) {	/* goin' west */
    if (!pryn_wall(bi,WEST,gx,gy)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_x < bi->cur_tc) {
      if (pryn_wall(bi,NORTH,gx,gy)) return(PI/2);
      else if (pryn_wall(bi,SOUTH,gx,gy)) return(3*PI/2);
      else {
	if (dir < PI) return(PI/2);
	else return(3*PI/2);
      }
    }
    else return(NO_CHANGE);
  }
  else if (dir >= 5*PI/4 && dir < 7*PI/4) { /* goin' north */
    if (!pryn_wall(bi,NORTH,gx,gy)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_y < bi->cur_tc) {
      if (pryn_wall(bi,EAST,gx,gy)) return(PI);
      else if (pryn_wall(bi,WEST,gx,gy)) return(0.0);
      else {
	if (dir < 3*PI/2) return(PI);
	else return(0.0);
      }
    }
    else return(NO_CHANGE);
  }
  else {			                /* goin' east */
    if (!pryn_wall(bi,EAST,gx,gy)) {
      return(NO_CHANGE);
    }
    else if (bi->me.loc.box_x > BOX_WIDTH - bi->cur_tc) {
      if (pryn_wall(bi,SOUTH,gx,gy)) return(3*PI/2);
      else if (pryn_wall(bi,NORTH,gx,gy)) return(PI/2);
      else {
	if (dir > PI) return(3*PI/2);
	else return(PI/2);
      }
    }
    else return(NO_CHANGE);
  }
}

/* pryn_turn_carefully()
 **
 ** This is used when we want to avoid an obstacle.  We give it a direction
 ** we want to go.  Repeatedly calling it with the direction to the goal
 ** should eventually get it there.
 */

Angle pryn_turn_carefully(bi,dir)
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
  char buf[MAX_DATA_LEN];
  
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
    if (pryn_wall(bi,NORTH,gx,gy)) {
      if (pryn_wall(bi,WEST,gx,gy)) 
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      else {
	if (pryn_wall(bi,NORTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
	else
	  doturnstuff(atanint(bx, by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx, by), 3*PI/2, 1);
      }
    }
    else if (pryn_wall(bi,WEST,gx,gy)) {
      if (pryn_wall(bi,WEST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by, bx), PI, -1,
		    atanint(bx, by), 3*PI/2, -1);
    }
    else if (pryn_wall(bi,WEST,gx,gy - 1)) {
      if (pryn_wall(bi,NORTH,gx - 1,gy)) 
	doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
      else 
	doturnstuff(atanint(bx, by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT + by, bx), PI, 1);
    }
    else if (pryn_wall(bi,NORTH,gx - 1,gy)) {
      doturnstuff(atanint(BOX_WIDTH + bx, by), 3*PI/2, -1,
		  atanint(bx, by), 3*PI/2, -1);
    }
    break;
  case SW:
    if (pryn_wall(bi,SOUTH,gx,gy)) {
      if (pryn_wall(bi,WEST,gx,gy))
	doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(by,bx), PI, 1);
      else {
	if (pryn_wall(bi,SOUTH,gx - 1,gy))
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (pryn_wall(bi,WEST,gx,gy)) {
      if (pryn_wall(bi,WEST,gx,gy + 1))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(by,bx), PI, 1);
      else
	doturnstuff(atanint(bx,BOX_HEIGHT - by), PI/2, 1,
		    atanint(by,bx), PI, 1);
    }
    else if (pryn_wall(bi,WEST,gx,gy + 1)) {
      if (pryn_wall(bi,SOUTH,gx - 1,gy))
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT * 2 - by,bx), PI, -1,
		    atanint(bx,BOX_HEIGHT - by), PI/2, 1);
    }
    else if (pryn_wall(bi,SOUTH,gx - 1, gy)) {
      doturnstuff(atanint(bx,BOX_HEIGHT - by),PI/2,1,
		  atanint(BOX_WIDTH + bx,BOX_HEIGHT - by), PI/2, 1);
    }
    break;
  case NE:
    if (pryn_wall(bi,NORTH,gx,gy)) {
      if (pryn_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(bx,by), 3*PI/2, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else {
	if (pryn_wall(bi,NORTH,gx + 1,gy))
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
	else
	  doturnstuff(atanint(bx,by), 3*PI/2, -1,
		      atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
      }
    }
    else if (pryn_wall(bi,EAST,gx,gy)) {
      if (pryn_wall(bi,EAST,gx,gy - 1))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH -bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (pryn_wall(bi,EAST,gx,gy - 1)) {
      if (pryn_wall(bi,NORTH,gx + 1,gy))
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT + by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_WIDTH - bx,by), 3*PI/2, 1);
    }
    else if (pryn_wall(bi,NORTH,gx + 1,gy))
      doturnstuff(atanint(BOX_WIDTH - bx,by), 3*PI/2, 1,
		  atanint(2*BOX_WIDTH - bx,by), 3*PI/2, 1);
    break;
  case SE:
    if (pryn_wall(bi,SOUTH,gx,gy)) {
      if (pryn_wall(bi,EAST,gx,gy))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(bx,BOX_HEIGHT - by), 5*PI/2, 1);
      else {
	if (pryn_wall(bi,SOUTH,gx + 1,gy))
	  doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
	else
	  doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		      atanint(bx,BOX_HEIGHT - by), PI/2, 1);
      }
    }
    else if (pryn_wall(bi,EAST,gx,gy)) {
      if (pryn_wall(bi,EAST,gx,gy + 1))
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
      else
	doturnstuff(atanint(by,BOX_WIDTH - bx), 2*PI, -1,
		    atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 2*PI, 1);
    }
    else if (pryn_wall(bi,EAST,gx,gy + 1)) {
      if (pryn_wall(bi,SOUTH,gx + 1,gy))
	doturnstuff(atanint(2*BOX_WIDTH - bx,BOX_HEIGHT - by), PI/2, -1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
      else
	doturnstuff(atanint(BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1,
		    atanint(2*BOX_HEIGHT - by,BOX_WIDTH - bx), 0, 1);
    }
    else if (pryn_wall(bi,SOUTH,gx + 1,gy))
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
      delta = abs(idir - bi->me.heading);
      if (delta > PI) delta = 2*PI - delta;
      set_rel_drive(6.0 - 4.0 * delta / PI);
      return(idir);
    }
  }
}
