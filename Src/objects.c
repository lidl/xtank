/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** objects.c
*/

#include "xtank.h"
#include "gr.h"

#define MAX_VEHICLE_OBJS	10
#define MAX_TURRET_OBJS		1
#define MAX_EXP_OBJS		8
#define MAX_LANDMARK_OBJS	3
#define MAX_RANDOM_OBJS		2

#include "Objects/lightc.obj"
#include "Objects/hexo.obj"
#include "Objects/spider.obj"
#include "Objects/psycho.obj"
#include "Objects/tornado.obj"
#include "Objects/marauder.obj"
#include "Objects/tiger.obj"
#include "Objects/rhino.obj"
#include "Objects/medusa.obj"
#include "Objects/malice.obj"

#include "Objects/turret_sm.obj"

#include "Objects/shock.obj"
#include "Objects/gleam.obj"
#include "Objects/tink.obj"
#include "Objects/soft.obj"
#include "Objects/circle.obj"
#include "Objects/fiery.obj"
#include "Objects/double.obj"
#include "Objects/exhaust.obj"

#include "Objects/all_bullets.obj"

#include "Objects/anim_landmarks.obj"
#include "Objects/map_landmarks.obj"
#include "Objects/design_landmarks.obj"

#include "Objects/xtank.obj"
#include "Objects/team.obj"


int num_vehicle_objs;
Object *vehicle_obj[MAX_VEHICLE_OBJS];

int num_turret_objs;
Object *turret_obj[MAX_TURRET_OBJS];

int num_exp_objs;
Object *exp_obj[MAX_EXP_OBJS];

Object *bullet_obj;

int num_landmark_objs;
Object *landmark_obj[MAX_LANDMARK_OBJS];

int num_random_objs;
Object *random_obj[MAX_RANDOM_OBJS];

#ifdef X10
typedef unsigned short Bits;
#endif
#ifdef X11
typedef Byte Bits;
#endif
#ifdef AMIGA
typedef unsigned short Bits;
#endif
 
int object_error;

/*
** Sets up all of the pixmaps in each object.
** Returns 1 if a pixmap could not be made.
*/
make_objects()
{
  extern Object *make_object();

  /* Clear the error flag */
  object_error = 0;

  /* Make all of the vehicle objects */
  vehicle_obj[0] = make_object(&lightc_obj, lightc_bitmap);
  vehicle_obj[1] = make_object(&hexo_obj, hexo_bitmap);
  vehicle_obj[2] = make_object(&spider_obj, spider_bitmap);
  vehicle_obj[3] = make_object(&psycho_obj, psycho_bitmap);
  vehicle_obj[4] = make_object(&tornado_obj, tornado_bitmap);
  vehicle_obj[5] = make_object(&marauder_obj, marauder_bitmap);
  vehicle_obj[6] = make_object(&tiger_obj, tiger_bitmap);
  vehicle_obj[7] = make_object(&rhino_obj, rhino_bitmap);
  vehicle_obj[8] = make_object(&medusa_obj, medusa_bitmap);
  vehicle_obj[9] = make_object(&malice_obj, malice_bitmap);
  num_vehicle_objs = 10;

  /* Make all of the turret objects */
  turret_obj[0] = make_object(&turret_sm_obj,turret_sm_bitmap);
  num_turret_objs = 1;

  /* Make all of the explosion objects */
  exp_obj[0] = make_object(&shock_obj,shock_bitmap);
  exp_obj[1] = make_object(&gleam_obj,gleam_bitmap);
  exp_obj[2] = make_object(&tink_obj,tink_bitmap);
  exp_obj[3] = make_object(&soft_obj,soft_bitmap);
  exp_obj[4] = make_object(&circle_obj,circle_bitmap);
  exp_obj[5] = make_object(&fiery_obj,fiery_bitmap);
  exp_obj[6] = make_object(&double_obj,double_bitmap);
  exp_obj[7] = make_object(&exhaust_obj,exhaust_bitmap);
  num_exp_objs = 8;

  /* Make the bullet object */
  bullet_obj = make_object(&all_bullets_obj,all_bullets_bitmap);
  
  /* Make the landmark objects */
  landmark_obj[0] = make_object(&anim_landmarks_obj,anim_landmarks_bitmap);
  landmark_obj[1] = make_object(&map_landmarks_obj,map_landmarks_bitmap);
  landmark_obj[2] = make_object(&design_landmarks_obj,design_landmarks_bitmap);
  num_landmark_objs = 3;

  /* Make the random objects */
  random_obj[XTANK_OBJ] = make_object(&xtank_obj,xtank_bitmap);
  random_obj[TEAM_OBJ] = make_object(&team_obj,team_bitmap);
  num_random_objs = 2;

  return object_error;
}

/*
** Makes all the pictures for the specified object.
*/
Object *make_object(obj,bitmap)
     Object *obj;
     Bits **bitmap;
{
  int i;

  for(i = 0 ; i < obj->num_pics ; i++)
    if(make_picture(&obj->pic[i],bitmap[i]))
      object_error = 1;
  return (obj);
}

/*
** Frees all the storage used by the objects.
*/
free_objects()
{
  int i;

  for(i = 0 ; i < num_vehicle_objs ; i++)
    free_object(vehicle_obj[i]);

  for(i = 0 ; i < num_turret_objs ; i++)
    free_object(turret_obj[i]);

  for(i = 0 ; i < num_exp_objs ; i++)
    free_object(exp_obj[i]);

  free_object(bullet_obj);

  for(i = 0 ; i < num_landmark_objs ; i++)
    free_object(landmark_obj[i]);

  for(i = 0 ; i < num_random_objs ; i++)
    free_object(random_obj[i]);
}

/*
** Frees all the pictures used by the object.
*/
free_object(obj)
     Object *obj;
{
  int i;

  for(i = 0 ; i < obj->num_pics ; i++)
    free_picture(&obj->pic[i]);
}

/*
** Rotates all the vehicle objects to so that all 16 rotations exist.
*/
rotate_objects()
{
  /* Rotate all of the vehicle objects */
  rotate_object(&lightc_obj,lightc_bitmap);
  rotate_object(&hexo_obj,hexo_bitmap);
  rotate_object(&spider_obj,spider_bitmap);
  rotate_object(&psycho_obj,psycho_bitmap);
  rotate_object(&tornado_obj,tornado_bitmap);
  rotate_object(&marauder_obj,marauder_bitmap);
  rotate_object(&tiger_obj,tiger_bitmap);
  rotate_object(&rhino_obj,rhino_bitmap);
  rotate_object(&medusa_obj,medusa_bitmap);
  rotate_object(&malice_obj,malice_bitmap);
}

/*
** Creates the 12 missing rotations from the first 4 rotations
** for every vehicle object.  This is done by 90 degree and 180
** degree rotations.
*/
rotate_object(obj,bitmap)
     Object *obj;
     Bits **bitmap;
{
  Picture *pic,*rot_pic;
  Bits *rotate_pic_90(),*rotate_pic_180();
  int dest,source;

  /* Create next 4 pictures by rotating the first 4 by 90 degrees */
  for(source = 0 ; source < 4 ; source++) {
    dest = source + 4;
    pic = &obj->pic[source];
    rot_pic = &obj->pic[dest];
    bitmap[dest] = (Bits *) rotate_pic_90(pic,rot_pic,bitmap[source]);
  }

  /* Create last 8 pictures by rotating the first 8 by 180 degrees */
  for(source = 0 ; source < 8 ; source++) {
    dest = source + 8;
    pic = &obj->pic[source];
    rot_pic = &obj->pic[dest];
    bitmap[dest] = (Bits *) rotate_pic_180(pic,rot_pic,bitmap[source]);
  }
}
