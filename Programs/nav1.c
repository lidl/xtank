/* 




   nav1.c 

This is a feeble attempt at an Xtank robot program.  If it works as well as I 
hope, it will be a dudely navigation/wall avoidance algoritm

*/
#include "/mit/games/src/vax/xtank/Programs/xtanklib.h"
#include <math.h>

extern int nav1_main();

Prog_desc nav1_prog = {
   "nav1",
   "nav",
   "Moves to box (0,0) avoiding walls",
   "Marc LeBlanc",
    USES_MESSAGES + PLAYS_RACE,
    6,
    nav1_main
    };

#define noaim (Location *) 0
static Location Dest, Target, Approach;
static int xdirs[4] = { 0, 1, 0, -1};
static int ydirs[4] = { -1, 0, 1, 0};
static Boolean Debug = FALSE;
static int targeted;
#define nav1_xdir(c) ((c)>-1 ? xdirs[c] : 0)
#define nav1_ydir(c) ((c)>-1 ? ydirs[c] : 0)
#define nav1_sxdir(c,s) (nav1_ydir(c) ? s : 0)
#define nav1_sydir(c,s) (nav1_xdir(c) ? s : 0) 

nav1_main()
{
  double angle,diff;
  Vehicle_info me;
  Location *aim;Dest.grid_x = 0;
  Dest.grid_y = 0;
  Dest.box_x = Dest.x = BOX_WIDTH / 2;
  Dest.box_y = Dest.y = BOX_HEIGHT / 2;
  targeted = FALSE;
  set_rel_drive(9.0);
  for(;;)
    {
      int dx,dy;
      get_self(&me);
      nav1_handle_messages();
      aim = noaim;
      if ((me.loc.grid_x == Dest.grid_x) && 
	  (me.loc.grid_y == Dest.grid_y))
	aim = &Dest;
      else if (((me.loc.grid_x == Target.grid_x) &&
	       (me.loc.grid_y == Target.grid_y)) 
	       || !targeted)
	{
	  nav1_newbox(&me);
	  targeted = TRUE;
	}
      if (aim == noaim)
      {
	aim = clear_path(&me.loc,&Target) ? &Target : &Approach;
        if ((aim == &Approach) && !clear_path(&me.loc,aim))
	  {
	    targeted = FALSE;
	    continue;
	  }
/*	if (aim == &Target) if (Debug) printf("Target\n"); 
	else if (Debug) printf("Approach\n");  */
      }
      dx = aim->x - me.loc.x;
      dy = aim->y - me.loc.y;
      angle = atan2((double) dy, (double) dx);
      if (angle < 0) angle += 3.14159*2;
      diff = angle - me.heading;
      angle += diff/4*sin(diff > 0 ? diff : -diff);
      turn_vehicle(angle);
      done();
    }
}

nav1_newbox(me)
  Vehicle_info *me;
{
  int maj, min, c, s, Delx, Dely, bestc, bests, bestw;
  static int lastc = -1, lasts = 0,last2c = -1;
  Delx = Dest.grid_x - me->loc.grid_x;
  Dely = Dest.grid_y - me->loc.grid_y;
  {
    int ew, ns;

    ew = (Delx > 0) ? EAST : WEST; 
    if (Delx == 0) ew = -1;
    ns = (Dely < 0) ? NORTH : SOUTH; 
    if (Dely == 0) ns = -1;
    if (abs(Delx) > abs(Dely)) maj = ew, min = ns;
    else maj = ns, min = ew;
  }
  if (Debug) printf("maj: %d min: %d\n",maj,min);
  bestw = -100; bestc = 0; bests = 0;
  for(c = 0; c < 4; ++c)
    {
      if (Debug) printf("c = %d\n", c);
    if (!wall(c,me->loc.grid_x,me->loc.grid_y))
      for(s = -1; s < 2; ++s)
      {
	int w,bx,by,xd,yd,sxd,syd;
	
	if (Debug) printf("trying c: %d s: %d\n",c,s);
	xd = nav1_xdir(c);
	yd = nav1_ydir(c);
	sxd = nav1_sxdir(c,s);
	syd = nav1_sydir(c,s);
/*	if (Debug) printf("xd: %d yd: %d sxd: %d syd: %d\n",xd,yd,sxd,syd);*/
	bx = me->loc.grid_x + xd + sxd;
	by = me->loc.grid_y + yd + syd;
	w = 0;
	if (s == 0) w+=1;
/*	if (Debug) printf("bx: %d by: %d\n",bx,by);*/

	if (bx != Dest.grid_x || by != Dest.grid_y) 
	  if (wall(NORTH,bx,by) + wall(SOUTH,bx,by) + wall(EAST,bx,by) 
	      + wall(WEST,bx,by) > 2) 
	    continue;
       	if (s != 0)
	  {
	    int d;
	    if (yd) d = WEST*(s==1) + EAST*(s==-1);
	    if (xd) d = NORTH*(s==1) + SOUTH*(s==-1);
	    if (wall(d,bx,by)) continue;
	    if (d == lastc) w-= 20;
	  }
	if (c == maj) w+=5;
	if ((xd == - nav1_xdir(maj)) && (yd == - nav1_ydir(maj)))
	  w-=5;
	if ((sxd == - nav1_xdir(maj)) && (syd == - nav1_ydir(maj)))
	  w-=5;	
	if ((sxd ==  nav1_xdir(maj)) && (syd == nav1_ydir(maj)))
	  w+=5;

	if (((xd == nav1_xdir(min)) && yd == nav1_ydir(min)) ||
	    ((sxd == nav1_xdir(min)) && (syd == nav1_ydir(min))))
	  w+=3;
	if (((xd == -nav1_xdir(min)) && yd == -nav1_ydir(min)) ||
	    ((sxd == -nav1_xdir(min)) && (syd == -nav1_ydir(min))))
	  w-=3;
	if (!wall(maj,bx,by)) w+=2; else w-=1;
	if (!wall(min,bx,by)) w+=1;
	if (lastc != -1 && c == (lastc + 2) % 4) w-=40;
	if (lasts != 0)
	  {
	    int d;
	    if (nav1_ydir(lastc)) d = WEST*(lasts==1) 
	                            + EAST*(lasts==-1);
	    if (nav1_xdir(lastc)) d = NORTH*(lasts==1) 
	                            + SOUTH*(lasts==-1);
	    if (d == c) w-=40;
	  }
	if (last2c != -1 && c == (last2c + 2) % 4) w-=5;
	if (w > bestw) bestw = w, bestc = c, bests = s;
	if (Debug) printf("weight: %d\n",w);
      }
    }
  if (bestc != lastc) last2c = lastc;
  lastc = bestc;
  lasts = bests;
  Target.grid_x = me->loc.grid_x + nav1_xdir(bestc) + nav1_sxdir(bestc,bests);
  Target.grid_y = me->loc.grid_y + nav1_ydir(bestc) + nav1_sydir(bestc,bests);
  Target.box_x = BOX_WIDTH * (0.5 - 0.35 * nav1_sxdir(bestc,bests));
  Target.box_y = BOX_HEIGHT * (0.5 - 0.35  * nav1_sydir(bestc,bests));
  nav1_fillxy(&Target);
  Approach.grid_x = me->loc.grid_x;
  Approach.grid_y = me->loc.grid_y;
  Approach.box_x = BOX_WIDTH * (0.5 + nav1_xdir(bestc)*0.5
				+ nav1_sxdir(bestc,bests)*0.35);
  Approach.box_y = BOX_HEIGHT * (0.5 + nav1_ydir(bestc)*0.5
				+ nav1_sydir(bestc,bests)*0.35);
  nav1_fillxy(&Approach);
  if (Debug) printf("picked c: %d, s: %d w: %d\n",bestc,bests,bestw);
/*  if (Debug) printf("tx: %d ty: %d\n",Target.grid_x, Target.grid_y);
  if (Debug) printf("ax: %d ay: %d\n",Approach.box_x,Approach.box_y);*/
}

nav1_fillxy(loc)
 Location *loc;
{
  loc->x = loc->box_x + BOX_WIDTH * loc->grid_x;
  loc->y = loc->box_y + BOX_HEIGHT * loc->grid_y;
}
  
nav1_handle_messages()
{
  Message m;

  while (messages())
    {
      receive(&m);
      if (m.opcode == OP_GOTO)
	{
	  Dest.grid_x = m.data[0];
          Dest.grid_y = m.data[1];
	  Dest.box_x = m.data[2];
	  Dest.box_y = m.data[3];
	  nav1_fillxy(&Dest);
	  targeted = FALSE;
	}
     if (m.opcode == OP_ACK) Debug = !Debug;
    }
}





