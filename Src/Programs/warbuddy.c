#include "sysdep.h"
#include "xtanklib.h"
#include <math.h>

typedef struct {
   FLOAT xspeed, yspeed;
   ID    id;
   int   enemy, vnum;
   FLOAT xacc, yacc;
} Target_info;

typedef struct {
   int          mode, frame, lastframe, numvehicles;
   Vehicle_info vehicle[MAX_VEHICLES], me;
   Target_info  target;
   Bullet_info  bullet[MAX_BULLETS];
   Weapon_info  weapon[8];
   Message      buddytalk;
} Allinfo;

static void warbuddy_main();

Prog_desc warbuddy_prog = {
   "warbuddy",
   "Timebandit",
   "Follows team mate around and kills tanks",
   "Michael T Lee",
   PLAYS_COMBAT | DOES_SHOOT,
   4,
   warbuddy_main
};

static void warbuddy_main()
{
   Allinfo p;

   /* set_rel_speed(9); */
   get_self(&p.me);

   get_weapon(0,p.weapon);

   while (1) {
      p.frame = frame_number();
      if (p.lastframe != p.frame) {
         p.lastframe = p.frame;
         get_self(&p.me);
         /* Movement */
         /* Scan */
         get_vehicles(&p.numvehicles,p.vehicle);
         /* Combat */
         find_closest_enemy(&p);
         if (p.target.enemy) shoot_enemy(&p);
      } else done();
   }
}

find_closest_enemy(p)
Allinfo *p;
{
   int mindist, dist, i, dx, dy;

   p->target.enemy = FALSE;
   mindist = 100000000;
   for (i = 0; i < p->numvehicles; i++) {
      dx = p->vehicle[i].loc.x - p->me.loc.x;
      dy = p->vehicle[i].loc.y - p->me.loc.y;
      dist = dx * dx + dy * dy;
      if ((dist < mindist) && (p->vehicle[i].team != p->me.team)) {
         mindist = dist;
         p->target.enemy = TRUE;
         p->target.vnum = i;
      }
   }
   if (p->target.enemy) {
      if (p->target.id == p->vehicle[p->target.vnum].id) {
         p->target.xacc = p->vehicle[p->target.vnum].xspeed
                        - p->me.xspeed
                        - p->target.xspeed;
         p->target.yacc = p->vehicle[p->target.vnum].yspeed
                        - p->me.yspeed
                        - p->target.yspeed;
      } else {
         p->target.xacc = 0;
         p->target.yacc = 0;
         p->target.id = p->vehicle[p->target.vnum].id;
      }
      p->target.xspeed = p->vehicle[p->target.vnum].xspeed
                       - p->me.xspeed;
      p->target.yspeed = p->vehicle[p->target.vnum].yspeed
                       - p->me.yspeed;
   } else p->target.id = p->me.id;
}

shoot_enemy(p)
Allinfo *p;
{
   int       XVel, YVel, XLoc, YLoc, WeapSp, WeapRg, XAcc, YAcc;
   int       a, b, c, t;
   int       TX, TY;
   Location  TLoc;

   XVel = (int) p->target.xspeed - p->me.xspeed;
   YVel = (int) p->target.yspeed - p->me.yspeed;
   XLoc = (int) p->vehicle[p->target.vnum].loc.x - p->me.loc.x;
   YLoc = (int) p->vehicle[p->target.vnum].loc.y - p->me.loc.y;
   XAcc = (int) p->target.xacc;
   YAcc = (int) p->target.yacc;
   WeapSp = (int) p->weapon[0].ammo_speed;
   WeapRg = (int) p->weapon[0].range;
   a = XVel*XVel + YVel*YVel - WeapSp*WeapSp;
   b = (XVel*XLoc + YVel*YLoc) << 1;
   c = XLoc*XLoc + YLoc*YLoc;

   if (a != 0) {
      t = b*b - (a*c << 2);
      if (t >= 0) {
         t = (int) (sqrt((double) t));
         t = ((-b - t) / a) >> 1;
      } else return;
   } else if (b < 0) t = -c / b;
   else t = 0;

   XAcc = 0;
   YAcc = 0;
   TX = ( XVel + ((XAcc*t) >> 1) ) * t  +  XLoc;
   TY = ( YVel + ((YAcc*t) >> 1) ) * t  +  YLoc;

   TLoc.x = (FLOAT) TX + p->me.loc.x;
   TLoc.y = (FLOAT) TY + p->me.loc.y;

   TLoc.grid_x = TLoc.x / BOX_WIDTH;
   TLoc.grid_y = TLoc.y / BOX_HEIGHT;
   TLoc.box_x = TLoc.x % BOX_WIDTH;
   TLoc.box_y = TLoc.y % BOX_HEIGHT;

   if (clear_path(&p->me.loc, &TLoc)) {
      aim_all_turrets(TX, TY);
      if (TX*TX + TY*TY < WeapRg*WeapRg)
         fire_all_weapons();
   }
}

