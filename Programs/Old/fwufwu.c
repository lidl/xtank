/*
** fwufwu.c
**
** Nice basic hack tank 
** Uses standard vehicle 14
**  Garth Zeglin
**  1/25/88
*/

#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"
#include <math.h>

void fwufwu_get_unstuck();
void fwufwu_move();
void fwufwu_shoot();
void fwufwu_wakeup();
void fwufwu_mesg();
int fwufwu_me[100]; 

fwufwu_main()
{
    int fwufwu_rev= 0;
    int bul_spd;
    char temp[40];
    fwufwu_wakeup();
    bul_spd= weapon_ammo_speed(0);
    while(1) {
      fwufwu_mesg();
    	
      if (speed() == 0) {
	  fwufwu_get_unstuck(bul_spd); fwufwu_rev= !fwufwu_rev;
      }
      fwufwu_move(fwufwu_rev);
  
      fwufwu_shoot(bul_spd);
    }
}
void fwufwu_mesg()
{
  static char fwufwu_curses[10][40] = {
"Die, Alien Scum!!",
"Eat Death",
"Swallow Lead, Foul Machine!",
"Take that, sucker!",
"Hah! Victory is mine.",
"Taste your defeat...",
"I drink to my winnings...",
"Blow a cog, stupid robot!",
"May your mother machine short out!",
"Run while you still roll!"};

  static int msg_timer = 20;

  if (--msg_timer < 0) {
    msg_timer = 20;
    message (fwufwu_curses[rand() % 10]);
  }
}
void fwufwu_move(rev)
int rev;
{
  Bullet_info bullet[MAX_BULLETS];
  int num_bullets;
  float xspeed,yspeed,ang,spd;
  Location loc;
  Bullet_info *b;
  float sensitivity;
  int dx, dy, dist;
  int i;

  /* Get information about the bullets, so we can avoid them */
  get_bullets(&num_bullets,bullet);

  /* If there are no bullets, don't bother dodging. */
  if(num_bullets == 0) return;

  /* Initialize vector components to zero */
  xspeed = 0;
  yspeed = 0;

  /* Get our location so we can tell find relative locations of the bullets */
  get_location(&loc);

  for(i = 0 ; i < num_bullets ; i++) {
    b = &bullet[i];
    
    dx = BOX_WIDTH * (b->loc.grid_x - loc.grid_x)
      + b->loc.box_x - loc.box_x;
    dy = BOX_HEIGHT * (b->loc.grid_y - loc.grid_y)
      + b->loc.box_y - loc.box_y;
    dist = dx*dx + dy*dy;

    /* Be more sensitive to the bullets that are close by
    ** If a bullet is going to hit, give up, and ignore it.
    */
    if(dist > 20*20)
      sensitivity = 1 / (float) dist;
    else
      sensitivity = 0;

    /* Add a vector 90 degrees to the line from me to the bullet */
    xspeed -= sensitivity * (float) dy;
    yspeed += sensitivity * (float) dx;
    if (rev) {
	xspeed= -xspeed;
	yspeed= -yspeed;
    }
  }

  /* Figure out the angle of the vector */
  ang = atan2(yspeed,xspeed);
  turn_vehicle(ang);

  /* Assume we always want to go about speed 4 */
}

void fwufwu_get_unstuck(bul_spd)
int bul_spd;
{
  int framenum = frame_number();
  float curangle = angle();

  /* First backup away from the obstacle */
  set_abs_speed(-4.1);

  while(frame_number() < framenum + 12)
    fwufwu_shoot(bul_spd);

  /* Turn around and get moving */
  turn_vehicle(curangle + PI);
  set_abs_speed(4.1);
  while(frame_number() < framenum + 24)
    fwufwu_shoot(bul_spd);
}


int fwufwu_range(l1,l2)
Location *l1, *l2;
{
    float dx, dy;
    dx = (float) (BOX_WIDTH * (l1->grid_x - l2->grid_x)
	 + l1->box_x - l2->box_x);
    dy = (float) (BOX_HEIGHT * (l1->grid_y - l2->grid_y)
         + l1->box_y - l2->box_y);
    return((int)(sqrt(dx*dx + dy*dy)));
}

float fwufwu_speed(v)
Vehicle_info *v;
{
    return(sqrt(v->xspeed * v->xspeed + v->yspeed * v->yspeed));
}

void fwufwu_wakeup()
{
    int bul_spd, i, framenum, num;
    char temp[40];
    Vehicle_info vehicles[MAX_VEHICLES];
    for(i= 0; i<100; i++) {
	fwufwu_me[i]= 0;
    }
    turn_vehicle(1.0);
    set_abs_speed(4.1);
    framenum=frame_number();
    bul_spd= weapon_ammo_speed(0);
    while(frame_number()<framenum+20) {
	get_vehicles(&num,vehicles);
	for (i=0; i<num; i++) {
	    if ((vehicles[i].angle>.99)&&(vehicles[i].angle<1.01)) {
		fwufwu_me[vehicles[i].id]++;
	    } else {
		fwufwu_me[vehicles[i].id]= 0;
	    }
	}
	fwufwu_shoot(bul_spd);
    }
    for (i=0; i<100; i++) if (fwufwu_me[i]== 1) fwufwu_me[i]= 0;
}    
    
void fwufwu_shoot(bul_spd)
int bul_spd;
{
    Vehicle_info vehicles[MAX_VEHICLES], *v;
    Location my_loc;
    int numvinfos, i;
    int bestrange= 9999999;
    int dx,dy;                      /* the distance between me and my target */
    
    char temp[40];
    int range, kills;
    float speed;

    kills= num_kills();

    get_vehicles(&numvinfos,vehicles);
    get_location(&my_loc);	/* find my location */
    
    /* if there is another vehicle on screen, shoot at it */
    v= 0;
    for (i=0; i< numvinfos; i++) {
	range= fwufwu_range(&vehicles[i].loc, &my_loc);
	speed= fwufwu_speed(&vehicles[i]);
	/* wouldn't it be embarrasing to shoot yourself? */
	if ((range < bestrange) && (kills || (!fwufwu_me[vehicles[i].id]))) {
	    v= &vehicles[i];
	    bestrange= range;
	}
    }
    if (v) {
	range /= (float) bul_spd;
	dx = BOX_WIDTH * (v->loc.grid_x - my_loc.grid_x) +
	     (v->loc.box_x - my_loc.box_x) +
	     (int)(v->xspeed * range);
	  
	dy = BOX_HEIGHT * (v->loc.grid_y - my_loc.grid_y) +
	     (v->loc.box_y - my_loc.box_y) +
	     (int)(v->yspeed * range);
    
	turn_all_turrets(dx,dy);     /* turn my turret(s) toward him */
	fire_all_weapons();            /* let him have it */
    }
}








