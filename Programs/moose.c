/*
** Moose
** 
** A robot program for XTANK 0.90 + up
** 
** Authors: Dan Schmidt and Doug Church <lurk slink ooooof whoops splut>
** Goal   : A robot which plays a really good game of ultimate
**          it has several modes which enable it to play well (hopefully)
**
** Revision History:
**  ver 0.000: dschmidt, April 1988 
**      artful.c, which dodged walls and bullets
**  ver 0.001: dschmidt, July 7, 1988
**      bisfree.c, which could chase down and catch frisbees, but no
**    longer bullets since we figured for now they were a pain
**  ver 0.010: dachurch + dschmidt, July 13, 1988
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
*/

#include "../Src/xtanklib.h"
#include <math.h>
#include <stdio.h>

#include "moose.h"		/* The constants, structs, etc. */

/* This is the main routine of the moose ultimate robot
*/
moose_main()
{
   Binfo bi;
   int frame;

   moose_initialize_maze(&bi);	/* get the maze set up */
   moose_init_bi(&bi);		/* initialize the bi structure */
   moose_lock_on(&bi);
   set_safety(FALSE);		/* no safe turns here */
   bi.dir = heading();
   set_rel_drive(5.0);		/* KLUDGE */

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
	 if (bi.discowned && (bi.mode!=THROWIN))
	   bi.mode=GOT_DISC;	/* In case user takes over and catches */
      }				/* the disc */

      if (bi.mode!=GOT_DISC)    /* Toast this - it's just for show */
	bi.mode=CHAOS;

      if (frame%5)		/* For debugging, toast if you dont want */
	bi.dir = heading();	/* humans to munge the robot, allows a  */
				/* user to easily redirect moose...... */
      get_discs(&bi.numdiscs, bi.dinfo);

      switch(bi.mode) {
      case JUST_NOODLIN:
	 moose_find_something_to_do(&bi);
	 break;
      case GOT_DISC:
	 moose_wait_a_bit(&bi);	/* Temporary */
	 break;
      case THROWIN:
	 moose_throw_disc(&bi);	
	 break;
      case CUTTIN:
	 moose_get_open(&bi);
	 break;
      case CHASIN:
	 moose_go_get_the_disc(&bi,0);
	 break;
      case POINTIN:
	 moose_stand_around(&bi); /* Temporary */
	 break;
      case COVERIN:
	 moose_follow_man(&bi);
	 break;
      case CHAOS:
	 moose_go_get_the_disc(&bi,0);
	 break;
      default:
	 moose_what_the_fuck(&bi);
	 break;
      }
    
      done();
   }
}

moose_move(bi)
   Binfo *bi;
{
   double newdir;

   while ((newdir=moose_turn(bi,bi->dir))!=NO_CHANGE)
      bi->dir = newdir;
}

/* This routine sees who has the disc and then decides what mode to
** put moose the killer-who-looks-like-a-bunny in to winthe game....
*/
moose_find_something_to_do(bi)
   Binfo *bi;
{
   if (bi->dinfo[0].owner==NO_OWNER)
     bi->mode=CHAOS;		/* It's out there... we have no plan... */
   if (get_team(bi->dinfo[0].owner)==bi->us) /* we be offense */
     if (bi->dinfo[0].owner==my_id()) /* I have the disc, in fact */
       bi->mode = GOT_DISC;	/* lookin for the open man */
     else bi->mode = CUTTIN;	/* trying to get open */
   else				/* we be the defense */
     if (bi->dinfo[0].owner==bi->locked) /* my man's got the disc */
       bi->mode = POINTIN;	/* up there marking the thrower */
     else bi->mode = COVERIN;	/* covering up that receiver no mo no mo */
printf("Hi, I'm %d and I'm in mode %d\n",my_id(),bi->mode);   
}

/* This routine is only called in case of some internal error
*/
moose_what_the_fuck(bi)
   Binfo *bi;
{
   char buf[MAX_DATA_LEN];

   strcpy(buf, "Hey, who is this anyway");
   send(RECIPIENT_ALL, OP_TEXT, buf);
   moose_find_something_to_do(bi);
}

/* This routine stands stock still for a moment or two and then throws the disc
*/
moose_wait_a_bit(bi)
   Binfo *bi;
{
   if (bi->midstride) {
      if (!(bi->tdata--)) {
	 bi->midstride=FALSE;
	 throw_discs(25.0);
	 set_rel_drive(6.0);
	 bi->mode = CHAOS;	/* TEMPORARY... FOR TESTING */
      }
   } else {
      bi->midstride = TRUE;
      bi->tdata = 10+(random()%10);
      set_rel_drive(4.0);
   }
}
       
moose_throw_disc(bi)
   Binfo *bi;
{
   throw_discs(25.0);
   bi->mode = CUTTIN;
}

moose_get_open(bi)
   Binfo *bi;
{
   if (!(random()%49))		/* 2% chance of direction change */
     bi->dir=(((double)(random()%360))*2.0*PI)/360.0;
   if (!(random()%19))		/* 5% chance of spped change */
     set_rel_drive((double)(3+(random()%5)));
   moose_move(bi);
}

moose_stand_around(bi)
   Binfo *bi;
{
   if (bi->midstride) {
      if (bi->tdata!=bi->dinfo[0].owner) {
	 bi->midstride=FALSE;
	 set_rel_drive(5.0);
	 bi->mode = COVERIN;
      }
   } else {
      bi->midstride=TRUE;
      set_rel_drive(0.0);
      bi->tdata=bi->dinfo[0].owner;
   }
}

moose_follow_man(bi)
   Binfo *bi;
{
   moose_move(bi);
}

/* This routine goes and gets disc number discnum....
*/
moose_go_get_the_disc(bi,discnum)
   Binfo *bi;
   int discnum;
{
  int dx,dy;
  Angle heading;
  int distsq;
  int i = 0;
  Coord discfuture;
  Location *discloc;

  discloc = &bi->dinfo[discnum].loc;

  discfuture.x = discloc->grid_x * BOX_WIDTH + discloc->box_x;
  discfuture.y = discloc->grid_y * BOX_HEIGHT + discloc->box_y;

  while (i++ < DISC_FRAMES) {
    moose_update_disc(bi,&discfuture,&bi->dinfo[discnum]);
    
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
  moose_move(bi);
}

/* For use by the follow disc routines
*/
moose_update_disc(bi,discloc,discinfo)
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
    if (moose_wall(bi,EAST,oldgridloc.x,oldgridloc.y)) {
      discloc->x -= (discloc->x % BOX_WIDTH) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }
  else if (oldgridloc.x == discgridloc.x + 1) {
    if (moose_wall(bi,WEST,oldgridloc.x,oldgridloc.y)) {
      discloc->x += (BOX_WIDTH - (discloc->x % BOX_WIDTH)) * 2;
      discinfo->xspeed = -discinfo->xspeed;
    }
  }

  if (oldgridloc.y == discgridloc.y - 1) {
    if (moose_wall(bi,SOUTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y -= (discloc->y % BOX_HEIGHT) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
  else if (oldgridloc.y == discgridloc.y + 1) {
    if (moose_wall(bi,NORTH,oldgridloc.x,oldgridloc.y)) {
      discloc->y += (BOX_HEIGHT - (discloc->y % BOX_HEIGHT)) * 2;
      discinfo->yspeed = -discinfo->yspeed;
    }
  }
}

/* This silly routine initializes the Binfo structure for the tank...
*/
moose_init_bi(bi)
  Binfo *bi;
{
   bi->timing.bullet_check = 0;
   bi->timing.vehicle_check = 0;
   bi->timing.wall_check = 0;
   bi->timing.disc_check = 0;
   bi->frame = 0;
   bi->next_frame = -1;
   bi->maxspeed = max_speed();
   bi->mode = JUST_NOODLIN;
   bi->midstride = FALSE;
   bi->tox = -1;
   bi->toy = -1;
   bi->thead = 0;
   bi->us = get_team(my_id());
/* bi->locked = NO_OWNER;          /* Not now... this is for optimal teaming */
}

/* Locks our program onto some opponent...
** This is a kludge at the moment, assuming 2 teams, same number of people on
** both teams, and other heionousity (0-- numvehic's - 1 always...
** NEEDS REWRITE
*/
moose_lock_on(bi)
   Binfo *bi;
{
   int hisloc, ourloc, i;

   for (ourloc=0, i=0; i!=my_id(); i++)
     if (get_team(i)==bi->us) ourloc++;
   for (hisloc=-1, i=0; hisloc!=ourloc; i++)
     if (get_team(i)!=bi->us) hisloc++;
   bi->locked = i;
}

/* This initializes the internal maze representation
*/
moose_initialize_maze(bi)
  Binfo *bi;
{
   int i,j;
   
   for (i=0; i<GRID_WIDTH; ++i)
     for (j=0; j<GRID_HEIGHT; ++j)
       bi->maze[i][j] = 0;
}

/* moose_wall is our improved version of wall,....
** used by moose_turn, of course
*/
moose_wall(bi, side, x, y)
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
** moose_turn.  Yeah.
** 
** It gets passed in an Angle saying which way the vehicle wants to go.
** It turns that way if it's a good idea to do so.  Otherwise, it doesn't.
**
** Right now it sucks.
*/

double moose_turn(bi,dir)
     Binfo *bi;
     Angle dir;
{
   if (dir > PI/4 && dir < 3*PI/4) {	        /* goin' south */
      if (!moose_wall(bi,SOUTH,bi->loc.grid_x,bi->loc.grid_y)) {
	 turn_vehicle(dir);
	 return(NO_CHANGE);
      } 
      else if (!moose_wall(bi,WEST,bi->loc.grid_x,bi->loc.grid_y))
	return(PI);
      else if (!moose_wall(bi,EAST,bi->loc.grid_x,bi->loc.grid_y))
	return(0.0);
      else return(3*PI/2);
   }
   else if (dir >= 3*PI/4 && dir < 5*PI/4) {	/* goin' west */
      if (!moose_wall(bi,WEST,bi->loc.grid_x,bi->loc.grid_y)) {
	 turn_vehicle(dir);
	 return(NO_CHANGE);
      }
      else if (!moose_wall(bi,NORTH,bi->loc.grid_x,bi->loc.grid_y))
	return(3*PI/2);
      else if (!moose_wall(bi,SOUTH,bi->loc.grid_x,bi->loc.grid_y))
	return(PI/2);
      else return(0.0);
   }
   else if (dir >= 5*PI/4 && dir < 7*PI/4) { /* goin' north */
      if (!moose_wall(bi,NORTH,bi->loc.grid_x,bi->loc.grid_y)) {
	 turn_vehicle(dir);
	 return(NO_CHANGE);
      }
      else if (!moose_wall(bi,EAST,bi->loc.grid_x,bi->loc.grid_y))
	return(0.0);
      else if (!moose_wall(bi,WEST,bi->loc.grid_x,bi->loc.grid_y))
	return(PI);
      else return(PI/2);
   }
   else {			                /* goin' east */
      if (!moose_wall(bi,EAST,bi->loc.grid_x,bi->loc.grid_y)) {
	 turn_vehicle(dir);
	 return(NO_CHANGE);
      }
      else if (!moose_wall(bi,SOUTH,bi->loc.grid_x,bi->loc.grid_y))
	return(PI/2);
      else if (!moose_wall(bi,NORTH,bi->loc.grid_x,bi->loc.grid_y))
	return(3*PI/2);
      else return(PI);
   }
}

/* moose_new_turn is a different approach to turning....
** you give it an angle and such and then it tells you whether you can
** possibly continue... 
*/
double moose_new_turn(bi, dir)
   Binfo *bi;
   Angle dir;
{
   int mod,quelstep;		/* how much to change dir, and +/- */

   if (moose_angle_ok(bi,dir)) return(NO_CHANGE); /* Just keep cruising */

   quelstep=0;
   mod=0;
   do {
      mod=mod+(random()%30);
   } while (!moose_angle_ok(bi, ((++quelstep%2)*2-1)*(dir+mod)));

   return(dir+mod);
}

moose_angle_ok(bi,dir)
   Binfo *bi;
   int dir;
{
   if (dir==0.0)		/* due east */
     if (!moose_wall(bi,EAST,get_gx,get_gy)) return(TRUE); 
     else return(((BOX_WIDTH-get_bx)*25/BOX_WIDTH)< bi->myspeed);
   if (dir==PI)			/* due west */
     if (!moose_wall(bi,WEST,get_gx,get_gy)) return(TRUE); 
     else return((get_bx*25/BOX_WIDTH)< bi->myspeed);
   if (dir==3*PI/2)		/* due north */
     if (!moose_wall(bi,NORTH,get_gx,get_gy)) return(TRUE); 
     else return((get_by*25/BOX_HEIGHT)< bi->myspeed);
   if (dir==PI/2)		/* due south */
     if (!moose_wall(bi,SOUTH,get_gx,get_gy)) return(TRUE);
     else return(((BOX_HEIGHT-get_by)*25/BOX_HEIGHT)< bi->myspeed);

   if (dir>0.0 && dir<PI/2) {	/* SouthEast */
      if ((!moose_wall(bi,SOUTH,get_gx,get_gy))&&
	  (!moose_wall(bi,EAST,get_gx,get_gy))) return(TRUE); /* No walls */
      else {			/* There were some walls */
/* HEY! CLEVER SIN AND COSING GET CLOSE TO WALLS CODE HERE */
      }
   }

/* HEY! OTHER DIRECTIONS (besides SouthEast) HERE */
}
