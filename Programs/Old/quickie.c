#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"
#include <math.h>  

#define ATTACK_MODE 0
#define SEARCH_MODE 1
#define BODY_WIDTH 48
#define BODY_HEIGHT 48
#define HALF_BW (BODY_WIDTH / 2)
#define HALF_BH (BODY_HEIGHT / 2)
#define B_WIDTH (BOX_WIDTH - HALF_BW)
#define B_HEIGHT (BOX_HEIGHT - HALF_BH)
#define NO_DIR -1
#define MAX_WEAPONS 20

#define CRUISE_SPEED 8.0
#define TURN_SPEED 6.0
#define TURN_ANGLE 0.3
#define TURN_PERSISTENCE 7

#define BACKUP_FRAMES 15
#define WIGGLE_TIME 10

#define LEAD_FACTOR 0.0002
#define MAX_LEAD_ANGLE 0.5


/* assume grid boxes are square */
#define LOOKAHEAD  BOX_WIDTH

float angle_diff();

                   /* globals */

int quickie_mode;
int quickie_max_range_squared;
Vehicle_info quickie_vinfo[MAX_VEHICLES];
int quickie_nVehicles, quickie_badguy;
static char s[80];
static int weapon_range_squared[MAX_WEAPONS];
static int nWeapons;
static int lastTurnFrame;
static float lastTurnValue;

quickie_main()
{
  quickie_init();
  for (;;) {
    switch(quickie_mode) {
      case ATTACK_MODE: quickie_attack(); break;
      
      case SEARCH_MODE: quickie_search(); break;

      default: message("Illegal mode");
    }
  }
}

quickie_init()
{
  int w, range, max_range;

  quickie_mode = SEARCH_MODE;
  nWeapons = num_weapons();
  max_range = 0;
  for (w=0; w < nWeapons; w++) {
    range = weapon_range(w);
    weapon_range_squared[w] = range * range;
    if (range > max_range)
      max_range = range;
  }
  quickie_max_range_squared = max_range * max_range;
  lastTurnFrame = -10;
}

quickie_search()
{
  get_vehicles(&quickie_nVehicles, quickie_vinfo);
  if ((quickie_badguy = choose_target()) >= 0) {
    quickie_mode = ATTACK_MODE;
    return;
  } else {
    quickie_move();
  }
}

static choose_target()
{
  int i, closest_i, closest_d, dist_squared, target_d, target_i;
  Location loc;
  char s1[80];

  s1[0] = '\0';
  get_location(&loc);
  closest_i = -1;
  closest_d = quickie_max_range_squared+1;
  target_i = -1;
  target_d =  quickie_max_range_squared+1;
  for (i = 0; i < quickie_nVehicles; i++) {
    dist_squared = quickie_dist_squared(&quickie_vinfo[i].loc, &loc);
    if (dist_squared < closest_d) {
      closest_d = dist_squared;
      closest_i = i;
    }
    if (dist_squared < target_d &&
        line_of_sight_to(&loc, &quickie_vinfo[i].loc)) {
      target_d = dist_squared;
      target_i = i;
    }
  }
  if (target_i == -1 && closest_i != -1) {
    turn_turrets_towards(&quickie_vinfo[closest_i].loc);
  }
  return(target_i);
}
      


/************************ MOVING **********************/


quickie_move()
{
  float d_ang;

  if (speed() == 0.0) {
    quickie_get_unstuck();
    set_rel_speed(CRUISE_SPEED);
  } else if (heading_for_wall(&d_ang)) {
    set_rel_speed(TURN_SPEED);
    turn_vehicle(angle()+d_ang);
/*
  } else if (good_landmark()) {
    seek_landmark();
*/
  } else
    set_rel_speed(CRUISE_SPEED);
}

quickie_get_unstuck()
{
  int framenum = frame_number();

  set_rel_speed(-3.0);
  /* Wait until several frames have passed, so we back up enough */
  while(frame_number() < framenum + BACKUP_FRAMES)
    ;
  if (speed() != 0) {
    set_rel_speed(0.0);
    turn_vehicle(angle()+0.4);
  } else {
    message("I'm still stuck!");
  }
}


static heading_for_wall(d_ang)
float *d_ang;
{
  /* Treat the vehicle as a box, and do line-of-sight checks from the
     outside edges of the box.  Also need another check perpendicular to
     the other two to catch screw cases. */
  int point1x, point1y, point2x, point2y, dx, dy;
  float ang;
  Location loc;
  int x, y, exit1x, exit1y, exit2x, exit2y, exit3x, exit3y;
  int plusWall, minusWall, otherWall;

  get_location(&loc);
  absolute(&loc, &x, &y);
  ang = angle();
  dx = (int) (LOOKAHEAD * cos(ang));
  dy = (int) (LOOKAHEAD * sin(ang));
  /* set points so point1 is decreasing angle, point2 increasing */
  if (ang < PI/2) {
    point1x = x + HALF_BW;
    point1y = y - HALF_BH;
    point2x = x - HALF_BW;
    point2y = y + HALF_BH;
  } else if (ang < PI) {
    point1x = x + HALF_BW;
    point1y = y + HALF_BH;
    point2x = x - HALF_BW;
    point2y = y - HALF_BH;
  } else if (ang < 3*PI/2) {
    point2x = x + HALF_BW;
    point2y = y - HALF_BH;
    point1x = x - HALF_BW;
    point1y = y + HALF_BH;
  } else {
    point2x = x + HALF_BW;
    point2y = y + HALF_BH;
    point1x = x - HALF_BW;
    point1y = y - HALF_BH;
  }
  minusWall = !line_sight(point1x,point1y,point1x+dx,point1y+dy,
			  &exit1x,&exit1y);
  plusWall  = !line_sight(point2x,point2y,point2x+dx,point2y+dy,
			  &exit2x,&exit2y);
  otherWall = !line_sight(point1x+dx,point1y+dy,point2x+dx,point2y+dy,
			  &exit3x,&exit3y);
  if (minusWall || plusWall || otherWall) {
    if (!minusWall)
      *d_ang = -TURN_ANGLE;
    else if (!plusWall)
      *d_ang = TURN_ANGLE;
    else {  /* both plus and minus; compute distance and go for longer */
      /* if we've turned recently, keep the same direction */
      if (frame_number() - lastTurnFrame <= TURN_PERSISTENCE)
	*d_ang = lastTurnValue;
      else if (dist_sq(x, y, exit1x, exit1y) > dist_sq(x, y, exit2x, exit2y))
	*d_ang = -TURN_ANGLE;
      else
	*d_ang = TURN_ANGLE;
    }
    lastTurnFrame = frame_number();
    lastTurnValue = *d_ang;
    return(TRUE);
  } else
    return(FALSE);
}

/***************** LANDMARKS *********************/

static good_landmark()
{ 
  Location loc;
  int type;

  get_location(&loc);
  if ((type = landmark(loc.grid_x, loc.grid_y))) {
    switch(type) {
    case FUEL: return(fuel() < 30);
    case AMMO: return(need_ammo());
    case ARMOR: return(need_armor());
    }
  }
  return(FALSE);
}

static need_ammo()
{
  int i;
  
  for (i = 0; i < nWeapons ; i++) {
    if (weapon_ammo(i) < weapon_max_ammo(i))
      return(TRUE);
  }
  return(FALSE);
}

static need_armor()
{
  int i;

  for (i = 0; i <= 5; i++) {
    if (armor(i) < max_armor(i))
      return(TRUE);
  }
  return(FALSE);
}


static seek_landmark()
{
  Location loc;
  int dx,dy;
  float ang;
  float spd;
  int dist;

  /* find out where we are */
  get_location(&loc);

  /* compute dx and dy to center of target box */
  dx = BOX_WIDTH/2 - loc.box_x;
  dy = BOX_HEIGHT/2 - loc.box_y;

  if (dx*dx + dy*dy < 25*25) {
    set_rel_speed(0.0);
  } else {
    /* head towards the center of the target box */
    ang = (float) atan2((float) dy,(float) dx);
    turn_vehicle(ang);
  }
}



/******************* ATTACKING *****************/

quickie_attack()
{
  char s[80];
  Vehicle_info *vi = &quickie_vinfo[quickie_badguy];
  float spd;
  int count;

  set_rel_speed(0.0);
  print_attack_msg();
  spd = 3.0;
  set_rel_speed(spd);
  count = 0;
  while (quickie_shoot_at(vi->id)) {
    if (speed() == 0) {
      spd = -spd;
      set_rel_speed(spd);
      count = -WIGGLE_TIME;
    } else {
      turn_vehicle(angle() + 0.3);
      count++;
    }
    if (count == WIGGLE_TIME) {
      spd = -spd;
      set_rel_speed(spd);
      count = 0;
    }
  }
  quickie_mode = SEARCH_MODE;
  message("Just cruisin'");
}

static print_attack_msg()
{
  message("Die, you gravy-sucking pig!");
}


/* 
** Moves all turrets at the vehicle whose id is vnum.
** Shoots all weapons at that vehicle, if in range; but
** regardless of whether the turrets are rotated yet.
*/
quickie_shoot_at(vnum)
int vnum;
{
  int num_vehicles;               /* the number of vehicles I can see */
  Vehicle_info vehicle[MAX_VEHICLES];  /* the array of vehicles I can see */
  Vehicle_info *v;
  Location my_loc;                /* my location */
  int i, w;                          /* the vehicle number I'm looking at */
  int dx,dy;                      /* the distance between me and my target */
  int range_squared;
  float enemy_angle, firing_angle;
  float compute_lead_angle();

  get_location(&my_loc);	/* find my location */
  get_vehicles(&num_vehicles,vehicle);  /* look around */

  /* Go through all the vehicles I can see and look for the one I want
  ** to shoot at.
  */
  for (i = 0 ; i < num_vehicles ; i++) {
    v = &vehicle[i];
    if (v->id == vnum) {                  /* Yup, that's the one */
      rel_loc(&my_loc, &v->loc, &dx, &dy);
      range_squared = dx*dx + dy*dy;
      if ((range_squared > quickie_max_range_squared) ||
	  (!line_of_sight_to(&my_loc, &v->loc)))
	return(FALSE);
      else {
	enemy_angle = atan2((float) dy, (float) dx);
	firing_angle = enemy_angle + compute_lead_angle(v, dx, dy);
	turn_turret(0, firing_angle);
	if (angle_diff(turret_angle(0), firing_angle) > 0.3)
	  return(TRUE);
	for (w = 0; w < nWeapons; w++) {
	  if (weapon_range_squared[w] >= range_squared)
	    fire_weapon(w);
	}
	return(TRUE);
      }
    }
  }
  return(FALSE);
}

static float compute_lead_angle(v, dx, dy)
Vehicle_info *v;
{
  float min();
  float max();

  return(min(max(LEAD_FACTOR * (dx * v->yspeed - dy * v->xspeed),
		   -MAX_LEAD_ANGLE),
	      MAX_LEAD_ANGLE));
}

static turn_turrets_towards(loc)
Location *loc;
{
  Location my_loc;
  int dx, dy;

  get_location(&my_loc);
  rel_loc(&my_loc, loc, &dx, &dy);
  turn_all_turrets(dx, dy);
}
  
/******************** UTILITIES ****************************/

line_of_sight_to(loc1, loc2)
Location *loc1, *loc2;
{
  int loc1x, loc1y, loc2x, loc2y, x, y;

  absolute(loc1, &loc1x, &loc1y);
  absolute(loc2, &loc2x, &loc2y);
  return(line_sight(loc1x, loc1y, loc2x, loc2y, &x, &y));
}

line_sight(loc1x, loc1y, loc2x, loc2y, exitx, exity)
int loc1x, loc1y, loc2x, loc2y;
int *exitx, *exity;
{
  float ang, tan_ang;
  int dx, dy;
  int top, left, right, bottom;
  int dir, x, y, gridx, gridy;

  dx = loc2x - loc1x;
  dy = loc2y - loc1y;
  ang = atan2((float) dy, (float) dx);
  norm_angle(&ang);
  tan_ang = tan(ang);
  gridx = loc1x / BOX_WIDTH;
  gridy = loc1y / BOX_HEIGHT;
  top = gridy * BOX_HEIGHT;
  bottom = top + BOX_HEIGHT;
  left = gridx * BOX_WIDTH;
  right = left + BOX_WIDTH;
  x = loc1x;
  y = loc1y;
  s[0] = '\0';
  while (!point_in_box(loc2x, loc2y, top, left, bottom, right)) {
    dir = dir_exit_rect(x, y, ang, tan_ang, top,left,bottom,right, &x, &y);
    add_dir_to_string(s, dir, gridx, gridy);
    if (wall(dir, gridx, gridy)) {
      *exitx = x;
      *exity = y;
      return(FALSE);
    }
    switch(dir) {
    case NORTH: top -= BOX_HEIGHT;
                bottom -= BOX_HEIGHT;
                gridy--;
                break;

    case SOUTH: top += BOX_HEIGHT;
                bottom += BOX_HEIGHT;
                gridy++;
                break;

    case EAST:  left += BOX_WIDTH;
                right += BOX_WIDTH;
                gridx++;
                break;

    case WEST:  left -= BOX_WIDTH;
                right -= BOX_WIDTH;
                gridx--;
                break;

    default: message("Whoops");
    }
  }
  *exitx = x;
  *exity = y;
  return(TRUE);
}

static point_in_box(x, y, top, left, bottom, right)
{
  return((x >= left) && (x <= right) && (y >= top) && (y <= bottom));
}

static dir_exit_rect(locx, locy, ang, tan_ang, top, left, bottom, right, x, y)
int locx, locy, top, left, bottom, right;
float ang, tan_ang;
int *x, *y;
{
  int west, north, result;
  
  west = (ang > PI / 2) && (ang < 3 * PI / 2);
  *x = (west)? left: right;
  *y = locy + (*x - locx) * tan_ang;
  if (*y >= top && *y <= bottom)
    return((west)? WEST:EAST);

  north = (ang > PI) && (ang < 2 * PI);
  *y = (north)? top:bottom;
  *x = locx + (*y - locy) / tan_ang;
  if (*x >= left && *x <= right)
    return((north)? NORTH:SOUTH);
}

static absolute(loc, x, y)
Location *loc;
int *x, *y;
{
  *x = loc->grid_x * BOX_WIDTH + loc->box_x;
  *y = loc->grid_y * BOX_HEIGHT + loc->box_y;
}

/* 
** Returns the lowest armor value of front, back, left, and right sides.
*/
quickie_lowest_armor()
{
  int low_so_far = 100;   /* lowest armor value found so far */
  int side;               /* current side I'm looking at */

  /* go through all the sides and check if the armor on that side is
     less than the lowest we've found so far.  If it is, I set low_so_far
     to that value. */

  for (side = 0 ; side < 4 ; ++side)
    if (low_so_far>armor(side)) low_so_far = armor(side);

  return (low_so_far);
}
  


static rel_loc(loc1, loc2, dx, dy)
Location *loc1, *loc2;
int *dx, *dy;
{
  *dx = BOX_WIDTH * (loc2->grid_x - loc1->grid_x) + loc2->box_x - loc1->box_x;
  *dy = BOX_HEIGHT * (loc2->grid_y - loc1->grid_y) + loc2->box_y - loc1->box_y;
}

quickie_dist_squared(loc1, loc2)
Location *loc1, *loc2;
{
  int dx, dy;

  rel_loc(loc1, loc2, &dx, &dy);
  return(dx*dx + dy*dy);
}

static dist_sq(x1, y1, x2, y2)
{
  int dx = x2-x1;
  int dy = y2-y1;
  
  return(dx*dx + dy*dy);
}

quickie_wait(n)
{
  int dummy;

  while (n--)
    dummy = n / (13+n);
}

static float angle_diff(a1, a2)
float a1, a2;
{
  float diff;

  norm_angle(&a1);
  norm_angle(&a2);
  diff = a1 - a2;
  if (diff < 0)
    diff = -diff;
  if (diff > PI)
    diff = 2*PI - diff;
  return(diff);
}

static norm_angle(ang)
float *ang;
{
  while (*ang < 0)
    *ang += 2*PI;
  while (*ang > 2*PI)
    *ang -= 2*PI;
} 

static float min(f1, f2)
float f1, f2;
{
  if (f1 > f2)
    return(f2);
  else
    return(f1);
}

static float max(f1, f2)
float f1, f2;
{
  if (f1 > f2)
    return(f1);
  else
    return(f2);
}

/************** DEBUGGING ***********************/

static add_dir_to_string(s, dir, gridx, gridy)
char *s;
int dir;
{
  char s1[80];

  switch(dir) {
  case NORTH: strcat(s, "N"); break;
  case SOUTH: strcat(s, "S"); break;
  case EAST: strcat(s, "E"); break;
  case WEST: strcat(s, "W"); break;
  }
  sprintf(s1, "(%d,%d)", gridx, gridy);
  strcat(s, s1);
}

/****************** TRASH *******************/

#ifdef TRASH

static heading_for_wall()
{
  float ang, tan_ang;
  Location loc;
  int x, y, exitx, exity;
  int top, bottom, left, right;
  int gridx, gridy;
  int absx, absy;
  int xtag, ytag;
  int dir, dirx, diry, dx, dy;

  ang = angle();
  tan_ang = tan(ang);
  get_location(&loc);
  x = loc.box_x;
  y = loc.box_y;
  gridx = loc.grid_x * BOX_WIDTH;
  gridy = loc.grid_y * BOX_HEIGHT;
  /* see what kind of rectangle we're in */
  if (x <= HALF_BW) {
    left = gridx - HALF_BW;
    right = gridx + HALF_BW;
    xtag = -1;
  } else if (x <= B_WIDTH) {
    left = gridx + HALF_BW;
    right = gridx + B_WIDTH;
    xtag = 0;
  } else {
    left = gridx + B_WIDTH;
    right = left + BODY_WIDTH;
    xtag = 1;
  }
  if (y <= HALF_BH) {
    top = gridy - HALF_BH;
    bottom = gridy + HALF_BW;
    ytag = -1;
  } else if (y <= B_HEIGHT) {
    top = gridy + HALF_BH;
    bottom = gridy + B_HEIGHT;
    ytag = 0;
  } else {
    top = gridy + B_HEIGHT;
    bottom = top + BODY_HEIGHT;
    ytag = 1;
  }

  absolute(&loc, &absx, &absy);
  dir = dir_exit_rect(absx, absy, ang, tan_ang, top, left, bottom, right,
		      &exitx, &exity);
  if (xtag == 0 && ytag == 0)
    return(WALL(dir, 0, 0));
  else if (xtag == 0) {
    if (dir == EAST || dir == WEST) {
      diry = (ytag == -1)? SOUTH:NORTH;
      dirx = OPP_DIR(dir);
      dx = (dir == EAST)? 1:-1;
      return(WALL(dir,0,0) || WALL(diry,dx,ytag) || WALL(dirx,dx,ytag));
    }
  } if (ytag == 0) {
    if (dir == NORTH || dir == SOUTH) {
      dirx = (xtag == -1)? EAST:WEST;
      diry = OPP_DIR(dir);
      dy = (dir == SOUTH)? 1:-1;
      return(WALL(dir,0,0) || WALL(dirx,xtag,dy) || WALL(diry,xtag,dy));
    }
  } else
    return(TRUE);
}

quickie_turn_to_avoid_wall()
{
  Location loc;
  int dir;
  Boolean wall_left, wall_right;
  float wallAng;

  get_location(&loc);
  dir = quickie_facing_wall_dir(quickie_angle, &loc);
  if (wall(dir, loc.grid_x, loc.grid_y)) {
    wall_left = wall((dir+3)%4, loc.grid_x, loc.grid_y);
    wall_right = wall((dir+1)%4, loc.grid_x, loc.grid_y);
    if (wall_left && wall_right) {
      set_rel_speed(0.0);
      quickie_angle += PI;
    } else {
      wallAng = quickie_dir_to_angle(dir);
      if (wall_left) {
	quickie_angle = wallAng 
      }
    }
  }
}
    
#endif

/* End quickie.c */
