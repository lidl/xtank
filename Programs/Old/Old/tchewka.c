
                  /***************************************/
                  /*                                     */
                  /* The name of this program is tchewka */
                  /*   It does a few stupid things now   */     
                  /*           Well, but slowly          */
                  /*                                     */
                  /***************************************/


#include <math.h>
#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"

typedef struct {
  int front;
  int back;
  int left;
  int right;
} Armor_on;

typedef struct {
  int north;
  int east;
  int south;
  int west;
} Boxes;

tchewka_main()
{
  int wall_frame_check = 0, tch_flag = 0, chpass = 0, hurt, figure = 99999;
  int killer = 0, homer = 0, diff = 999, chpan = 0, backing_up = 0, please = 0;
  int last_frame = frame_number(), tchumvinfos;
  Landmark_info lndmrk;
  Vehicle_info frumkin[MAX_VEHICLES];
  char temp[40];

  lndmrk.type = ARMOR;

  while (1) {
    if (wall_frame_check != 1 && tch_flag != 1)
      tch_cent(&chpass);
    if (wall_frame_check != 1)
      tch_kill(&killer,&homer,chpass,last_frame,tch_flag,&figure);   
    if (last_frame <= frame_number() + wall_frame_check) {
      last_frame = frame_number() + wall_frame_check;
      if (homer !=1)
	tch_wall(&wall_frame_check,&tch_flag,chpass,killer,&diff,please);
    }
    get_vehicles(&tchumvinfos,frumkin);
    if (tchumvinfos == 0) chpan = 1;
    else chpan = 0;

    if (chpan != 1) figure = 9999;
    please = 0;

    arm_chk(&hurt);

    if (((chpan == 1 && frame_number() < 350) || hurt ==1) && backing_up !=1) {
      tchewka_landmark(&lndmrk);
      if (lndmrk.x != -1)  
       tch_goes_to_landmark(lndmrk.x,lndmrk.y,diff,&please);
    }

    sprintf (temp,"If this number is one, I am hurt: %d",hurt);
    message(temp);

    if (speed() <= 0 && diff <= 35 && figure > 50) {
      set_rel_speed(-3.0);
      turn_vehicle(angle() + .4 * PI);
      sprintf(temp,"I tried to back up");
      message(temp);
      backing_up = 1;
    }        
    else backing_up = 0;
  }
}

/*
**  Finds the centroid of the enemy tank and runs away from that
*/

tch_cent(chpass)
      int *chpass;
{
  int tcount,numblipin,tcxdiff,tcydiff,numvinfos;
  float otang, tcang;
  Location tcloc;
  Vehicle_info tchew;
  int x = 0, y = 0;
  Vehicle_info enem[MAX_VEHICLES];
  Blip_info tcblips[MAX_BLIPS];
  char temp[40];
  static int tch_ftwo; 
/*
**  Gets the grid locations of the blips for x and y and adds them up
*/

  get_vehicles(&numvinfos,enem);
  if (frame_number() < 350 || numvinfos == 0) 
    {
      get_blips(&numblipin,tcblips);
      if (numblipin == 0 && numvinfos == 0) ++tch_ftwo;
      if (tch_ftwo > 15) { 
	*chpass = 1;
	tch_ftwo = 0;
      }

      if (numblipin != 0 || numvinfos != 0) {
	*chpass = 0;
	tch_ftwo = 0;
      }
  
      if (numblipin != 0) { 
	for (tcount=0; tcount<numblipin; ++tcount) {
	  x += tcblips[tcount].x;
	  y += tcblips[tcount].y;
	}
	
/*  Divide to find the centroid  */

	x = x/numblipin;
	y = y/numblipin;
	
/*	
**  Finds out where my tank is, then determines the angle to run 
**  at using the x and y differences between tanks
*/
    
	get_location(&tcloc);
	tcxdiff = x - tcloc.grid_x;
	tcydiff = y - tcloc.grid_y;
	if (frame_number() < 350)
	  tcang = PI + atan2((double) tcydiff,(double) tcxdiff);
	else otang = atan2((double) tcydiff,(double) tcxdiff);

/*  Run away!! (Or not...)  */

      if (frame_number() < 350) turn_vehicle(tcang);
      else turn_vehicle(otang);
      }
    }
}

/*
**  Attempts to detect walls, then tells us if we should evade, also
**  returns where the wall is and the distance to it, which will be
**  used in the wall evader
*/

tch_wall(wall_frame_check,tch_flag,chpass,killer,diff,please)
     int *wall_frame_check, *tch_flag, chpass, killer;
     int *diff, please;
{
  Location tcloc;
  char atemp[40], btemp[40];
  int xdiff, ydiff;
  double ang, xpanic, ypanic, spd;
  Boxes tch_this_box;
  float turning;
  int randm = random()%2;

/*
**  This finds out if there are walls in my box, and also tells me if
**  there are walls in the adjacent box.  If there are three walls in 
**  a box next to me (i.e., it is an alcove), I treat the entrance to 
**  the box as if it were a wall
*/
  get_location(&tcloc);

  tch_this_box.north = wall(0, tcloc.grid_x, tcloc.grid_y);
  tch_this_box.east = wall(1, tcloc.grid_x, tcloc.grid_y);
  tch_this_box.south = wall(2, tcloc.grid_x, tcloc.grid_y);
  tch_this_box.west = wall(3, tcloc.grid_x, tcloc.grid_y);

/*
**  Executes only if there is a wall present and my tank is moving
*/
  *wall_frame_check = 3;
  if ((frame_number() > 350 || chpass == 0 || killer != 1) && please != 1)
    set_abs_speed(12.0);

  if (angle() < PI) {
    ydiff = 177 - tcloc.box_y;
    if (angle() < .5 * PI) xdiff = 177 - tcloc.box_x;
    else xdiff = tcloc.box_x - 15;
  } 
  else if (angle() <= 1.5 * PI) {
    xdiff = tcloc.box_x - 15;
    ydiff = tcloc.box_y - 15;
  }
  else {
    xdiff = 177 - tcloc.box_x;
    ydiff = tcloc.box_y - 15;
  }
  
  if (xdiff < 0) xdiff = -xdiff;
  if (ydiff < 0) ydiff = -ydiff;

  if (xdiff > ydiff) *diff = ydiff;
  else *diff = xdiff;

  *tch_flag = 0;

  if ((tch_this_box.north != 0 || tch_this_box.east != 0 || 
      tch_this_box.south != 0 || tch_this_box.west != 0) && speed() > 0) {
    get_location(&tcloc); 

    *tch_flag = 1;
/*
**  Checks to see if there is a wall around, and uses the angle to
**  find which way we are pointed.  Then it figures out if I need to
**  panic or not
*/

    spd = speed();
    ang = angle();
    
    xpanic = 11 * spd * (cos(ang)); 
    ypanic = 11 * spd * (sin(ang));
    
    if (xpanic < 0) xpanic = -xpanic;
    if (ypanic < 0) ypanic = -ypanic;


    if (tch_this_box.north == 1 && ang > PI) {
      if (ydiff < ypanic) {
	*wall_frame_check = 1;	
      }
      if (tch_this_box.west == 1) turning = 0.0;
      else if (tch_this_box.east == 1) turning = PI;
      else if (ang > 1.5 * PI) turning = 0.0;
      else if (ang < 1.5 * PI) turning = PI;
      else if (randm == 0) turning = 0.0;
      else turning = PI;
    }
    if (tch_this_box.east == 1 && (ang < PI*0.5 || ang > 1.502*PI)) {
      if (xdiff < xpanic) {
	*wall_frame_check = 1;
      }
      if (tch_this_box.north == 1) turning = .5 * PI;
      else if (tch_this_box.south == 1) turning = 1.5 * PI;
      else if (ang < .5 * PI) turning = .5 * PI;
      else if (ang > .5 * PI) turning = 1.5 * PI;
      else if (randm == 0) turning = 1.5 * PI;
      else turning = .5 * PI;
    }
    if (tch_this_box.south == 1 && ang < PI) { 
      if (ydiff < ypanic) {
	*wall_frame_check = 1;
      }
      if (tch_this_box.east == 1) turning = PI;
      else if (tch_this_box.west == 1) turning = 0.0;
      else if (ang > .5 * PI) turning = PI;
      else if (ang < .5 * PI) turning = 0.0;
      else if (randm == 0) turning = 0.0;
      else turning = PI;
    } 
    if (tch_this_box.west == 1 && (ang < 1.5*PI && ang > PI * .502)) {
      if (xdiff < xpanic) {
	*wall_frame_check = 1;
      }
      if (tch_this_box.south == 1) turning = 1.5 * PI;
      else if (tch_this_box.north == 1) turning = .5 * PI;
      else if (ang > PI) turning = 1.5 * PI;
      else if (ang > PI) turning = .5 * PI;
      else if (randm == 0) turning = 1.5 * PI;
      else turning = .5 * PI;
    }
  }
  if (*wall_frame_check == 1) {
    set_abs_speed(7.0);
    turn_vehicle(turning);
  }
}                  

/*
**  Slams straight into enemies, fires, and kills them (hopefully)
*/

tch_kill(killer,homer,chpass,last_frame,tch_flag,figure,please)
     int *killer, *homer, chpass, last_frame;
     int tch_flag,*figure,please;
{
  Vehicle_info enem[MAX_VEHICLES];
  Vehicle_info *en;
  Location my_loc;
  Location *start;
  Location *finish;
  int numvinfos, tcount;
  int dx, dy, real_dx, real_dy, chkone, chktwo, well;
  float final_dis = 999999, cur_dis = 0, desired_ang = 0, figure;
  float ang = angle(), spd = speed(), frm = frame_number(), chkfor;
  char atemp[40], btemp[40];

  get_location(&my_loc);
  get_vehicles(&numvinfos,enem);

  for (tcount = 0; tcount < numvinfos; ++tcount) {
    en = &enem[tcount];

    dx = (en->loc.grid_x - my_loc.grid_x) * 192 + 
      en->loc.box_x - my_loc.box_x;
    dy = (en->loc.grid_y - my_loc.grid_y) * 192 + 
      en->loc.box_y - my_loc.box_y;

    cur_dis = dy*dy + dx*dx;

    if (cur_dis < final_dis) {
      start = &my_loc;
      finish = &en->loc;
      chkone = en->loc.grid_x;
      chktwo = en->loc.grid_y;
      final_dis = cur_dis; 
      real_dx = dx;
      real_dy = dy;
    }
  }

  figure = sqrt((float)final_dis);

  desired_ang = atan2((float) dy,(float) dx);

  chkfor = desired_ang - angle();
  if (chkfor < 0) chkfor = -chkfor;

  if (figure < 500 && frm > 350 && chpass == 0)
    set_abs_speed(15.0);

  if ((figure < 60 || chpass == 0) && frm > 350) {
    if (speed() != 0 && chpass != 1 && tch_flag !=1) turn_vehicle(desired_ang);
    if (figure < 80) *killer = 1;
  }
  else *killer = 0;

  well = 1;
  turn_all_turrets(real_dx,real_dy);

  if (figure < 150 &&  chkone != my_loc.grid_x && chktwo != my_loc.grid_y) {
    well = tch_clear_path(start,finish);
    sprintf(atemp,"I got to clear path");
    message(atemp);
  }
  
  if (figure < 150 && well != 0) fire_all_weapons();
  if (figure < 90 && frm > 350 && chkone == my_loc.grid_x
      && chktwo == my_loc.grid_y && chkfor < .1) *homer = 1;
  else *homer = 0;

  figure = 0;
}

tch_clear_path(start,finish)
     Location *start, *finish;
{
  int start_x,start_y,finish_x,finish_y;
  int dx,dy,lattice_dx,lattice_dy;
  int tgrid_x,tgrid_y,fgrid_x,fgrid_y;

  /* Compute absolute x coordinate in maze */
  start_x = start->grid_x * BOX_WIDTH + start->box_x;
  start_y = start->grid_y * BOX_HEIGHT + start->box_y;
  finish_x = finish->grid_x * BOX_WIDTH + finish->box_x;
  finish_y = finish->grid_y * BOX_HEIGHT + finish->box_y;

  /* Computed x and y differences from start to finish */
  dx = finish_x - start_x;
  dy = finish_y - start_y;

  /* Set up temporary and final box coordinates */
  tgrid_x = start->grid_x;
  tgrid_y = start->grid_y;
  fgrid_x = finish->grid_x;
  fgrid_y = finish->grid_y;

  /* Figure out the general direction that the line is travelling in
  ** so that we can write specific code for each case.
  **
  ** In the NE, SE, NW, and SW cases, 
  ** lattice_dx and lattice_dy are the deltas from the starting
  ** location to the lattice point that the path is heading towards.
  ** The slope of the line is compared to the slope to the lattice point
  ** This determines which wall the path intersects.
  ** Instead of comparing dx/dy with lattice_dx/lattice_dy, I multiply
  ** both sides by dy * lattice_dy, which lets me do 2 multiplies instead
  ** of 2 divides.
  */
  if(fgrid_x > tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else {			/* East */
      for(; tgrid_x < fgrid_x ; tgrid_x++)
	if(wall(EAST,tgrid_x,tgrid_y)) return(0);
    }

  else if(fgrid_x < tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	  }
      }
    }
    else {			/* West */
      for(; tgrid_x > fgrid_x ; tgrid_x--)
	if(wall(WEST,tgrid_x,tgrid_y)) return(0);
    }

  else
    if(fgrid_y > tgrid_y) {	/* South */
      for(; tgrid_y < fgrid_y ; tgrid_y++)
	if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
    }
    else if(fgrid_y < tgrid_y) { /* North */
      for(; tgrid_y > fgrid_y ; tgrid_y--)
	if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
    }
  return(1);
}


tchewka_landmark(lndmrk)
     Landmark_info *lndmrk;
{
  Location myloc;
  int num_lmarks;
  Landmark_info lmark[MAX_LANDMARKS];
  int mindist = 9999;		/* distance to nearest landmark so far */
  int dx, dy, i, curdist;

  /* find out where the vehicle is */
  get_location(&myloc);

  /* find out where all the landmarks are */
  get_landmarks(&num_lmarks,lmark);

  /* set lndmrk coordinates to (-1,-1) in case we find no landmarks */
  lndmrk->x = -1;
  lndmrk->y = -1;

  /* look through the array of landmarks, looking for the specified
     type, and remembering the closest one */
  for (i = 0 ; i < num_lmarks ; ++i)
    /* Check for landmark of the right type */
    if (lmark[i].type == lndmrk->type) {
      /* find out how far away it is */
      dx = lmark[i].x - myloc.grid_x;
      dy = lmark[i].y - myloc.grid_y;
      curdist = dx*dx + dy*dy;

      /* Check to see if it is closer than the closest one so far */
      if (curdist < mindist) {
	/* copy the coordinates and the distance */
	lndmrk->x = lmark[i].x;
	lndmrk->y = lmark[i].y;
	mindist = curdist;
      }
    }
}


tch_goes_to_landmark(x,y,diff,please)
     int x,y,diff,*please;
{
  Location loc;
  int dx,dy,dist;
  float spd = speed(), lm_des_ang;
  char temp[40];
  
  
    /* find out where we are */
    get_location(&loc);

    /* compute dx and dy to center of target box */
    dx = (x - loc.grid_x) * BOX_WIDTH + (BOX_WIDTH/2 - loc.box_x);
    dy = (y - loc.grid_y) * BOX_HEIGHT + (BOX_HEIGHT/2 - loc.box_y);

    /* head towards the center of the target box */
    lm_des_ang = (float) atan2((float) dy,(float) dx);
    dist = dx*dx + dy*dy;
    if (dist < 70*70) {
      if (dist >= 25*25 && spd!= 0) set_abs_speed(4.0);
      else if (dist <= 25*25) set_rel_speed(0.0);
      *please = 1;
    }
  if (diff > 70 && dist < 600*600 && dist > 25*25)
    turn_vehicle(lm_des_ang);
  sprintf (temp,"The distance flag is:",*please);
  message(temp);

}


arm_chk(hurt)
     int *hurt;
{
  Armor_on arm;

  arm.front = armor(0);
  arm.back = armor(1);
  arm.left = armor(2);
  arm.right = armor(3);

  if (arm.front < 25 || arm.back < 15 || arm.left < 20
      || arm.right < 20) *hurt = 1;
  else *hurt = 0;
}

