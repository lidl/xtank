/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "tanklimits.h"
#include "tanktypes.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "proto.h"

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
#include "Objects/trike.obj"
#include "Objects/panzy.obj"
#include "Objects/disk.obj"
#include "Objects/delta.obj"

#include "Objects/turret_sm.obj"

#include "Objects/shock.obj"
#include "Objects/gleam.obj"
#include "Objects/tink.obj"
#include "Objects/soft.obj"
#include "Objects/circle.obj"
#include "Objects/fiery.obj"
#include "Objects/double.obj"
#include "Objects/exhaust.obj"
#include "Objects/electric.obj"
#include "Objects/bullets.obj"
#include "Objects/nuke.obj"

#include "Objects/anm_lmarks.obj"
#include "Objects/map_lmarks.obj"
#include "Objects/des_lmarks.obj"

#include "Objects/xtank.obj"
#include "Objects/team.obj"
#include "Objects/terp.obj"

int num_vehicle_objs;
Object *vehicle_obj[MAX_VEHICLE_OBJS];

int num_turret_objs;
Object *turret_obj[MAX_TURRET_OBJS];

int num_exp_objs;
Object *exp_obj[MAX_EXP_OBJS];

int num_bullet_objs;   /* NUM_BULLET_OBJS defined in bullets.obj */
Object *bullet_obj[NUM_BULLET_OBJS];

int num_landmark_objs;
Object *landmark_obj[MAX_LANDMARK_OBJS];

int num_random_objs;
Object *random_obj[MAX_RANDOM_OBJS];

int object_error;

/*
** Sets up all of the pixmaps in each object.
** Returns 1 if a pixmap could not be made.
*/
make_objects()
{
	int num;
	extern Object *make_object();

	/* Clear the error flag */
	object_error = 0;

	/* Rearranged the order of these: (HAK 2/93) */
	/* Make all of the vehicle objects */
	num = 0;
	vehicle_obj[num++] = make_object(&lightc_obj, lightc_bitmap);
	vehicle_obj[num++] = make_object(&trike_obj, trike_bitmap);
	vehicle_obj[num++] = make_object(&hexo_obj, hexo_bitmap);
	vehicle_obj[num++] = make_object(&spider_obj, spider_bitmap);
	vehicle_obj[num++] = make_object(&psycho_obj, psycho_bitmap);
	vehicle_obj[num++] = make_object(&tornado_obj, tornado_bitmap);
	vehicle_obj[num++] = make_object(&marauder_obj, marauder_bitmap);
	vehicle_obj[num++] = make_object(&tiger_obj, tiger_bitmap);
	vehicle_obj[num++] = make_object(&rhino_obj, rhino_bitmap);
	vehicle_obj[num++] = make_object(&medusa_obj, medusa_bitmap);
	vehicle_obj[num++] = make_object(&delta_obj, delta_bitmap);
	vehicle_obj[num++] = make_object(&disk_obj, disk_bitmap);
	vehicle_obj[num++] = make_object(&malice_obj, malice_bitmap);
	vehicle_obj[num++] = make_object(&panzy_obj, panzy_bitmap);
	num_vehicle_objs = num;


	/* Make all of the turret objects */
	turret_obj[0] = make_object(&turret_sm_obj, turret_sm_bitmap);
	num_turret_objs = 1;

	/* Make all of the explosion objects */
	num_exp_objs = 0;
	exp_obj[num_exp_objs++] = make_object(&shock_obj, shock_bitmap);
	exp_obj[num_exp_objs++] = make_object(&gleam_obj, gleam_bitmap);
	exp_obj[num_exp_objs++] = make_object(&tink_obj, tink_bitmap);
	exp_obj[num_exp_objs++] = make_object(&soft_obj, soft_bitmap);
	exp_obj[num_exp_objs++] = make_object(&circle_obj, circle_bitmap);
	exp_obj[num_exp_objs++] = make_object(&fiery_obj, fiery_bitmap);
	exp_obj[num_exp_objs++] = make_object(&double_obj, double_bitmap);
	exp_obj[num_exp_objs++] = make_object(&exhaust_obj, exhaust_bitmap);
	/*GHS*/
	exp_obj[num_exp_objs++] = make_object(&electric_obj, electric_bitmap);
	exp_obj[num_exp_objs++] = make_object(&nuke_obj, nuke_bitmap);

	if(num_exp_objs != MAX_EXP_OBJS)
		printf("Yikes!!!!  MAX_EXP_OBJS mismatched!\n");

	/* Make the bullet object */
/*
	bullet_obj = make_object(&all_bullets_obj, all_bullets_bitmap);
*/
	num_bullet_objs = 0;
	bullet_obj[num_bullet_objs++] = make_object(&lmg_obj, lmg_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&mg_obj, mg_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&hmg_obj, hmg_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&lac_obj, lac_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&ac_obj, ac_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&hac_obj, hac_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&lrl_obj, lrl_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&rl_obj, rl_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&hrl_obj, hrl_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&as_obj, as_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&ft_obj, ft_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&seek_obj, seek_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&pr_obj, pr_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&um_obj, um_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&tele_obj, tele_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&tow_obj, tow_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&ltorp_obj, ltorp_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&bc_obj, bc_bitmap);
	bullet_obj[num_bullet_objs++] = bullet_obj[0];/* pulse laser, no bitmap */
	bullet_obj[num_bullet_objs++] = make_object(&mine_obj, mine_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&slick_obj, slick_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&mort_obj, mort_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&tn_obj, tn_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&harm_obj, harm_bitmap);
	bullet_obj[num_bullet_objs++] = make_object(&disc_obj, disc_bitmap);

	if(num_bullet_objs != NUM_BULLET_OBJS)
		printf("Yikes!!!!  NUM_BULLET_OBJS mismatched!\n");

	/* Make the landmark objects */
	landmark_obj[0] = make_object(&anim_landmarks_obj, anim_landmarks_bitmap);
	landmark_obj[1] = make_object(&map_landmarks_obj, map_landmarks_bitmap);
	landmark_obj[2] = make_object(&design_landmarks_obj,
								  design_landmarks_bitmap);
	num_landmark_objs = 3;

	/* Make the random objects */
	random_obj[XTANK_OBJ] = make_object(&xtank_obj, xtank_bitmap);
	random_obj[TEAM_OBJ] = make_object(&team_obj, team_bitmap);
	random_obj[TERP_OBJ] = make_object(&terp_obj, terp_bitmap);
	num_random_objs = 3;

	return object_error;
}

/*
** Makes all the pictures for the specified object.
*/
Object *make_object(obj, bitmap)
Object *obj;
Bits **bitmap;
{
	int i;

	for (i = 0; i < obj->num_pics; i++) {
		if (make_picture(&obj->pic[i], (char *) bitmap[i])) {
			object_error = 1;
		}
	}
	return (obj);
}

/*
** Frees all the storage used by the objects.
*/
free_objects()
{
	int i;

	for (i = 0; i < num_vehicle_objs; i++)
		free_object(vehicle_obj[i]);

	for (i = 0; i < num_turret_objs; i++)
		free_object(turret_obj[i]);

	for (i = 0; i < num_exp_objs; i++)
		free_object(exp_obj[i]);

	free_object(bullet_obj);

	for (i = 0; i < num_landmark_objs; i++)
		free_object(landmark_obj[i]);

	for (i = 0; i < num_random_objs; i++)
		free_object(random_obj[i]);
}

/*
** Frees all the pictures used by the object.
*/
free_object(obj)
Object *obj;
{
	int i;

	for (i = 0; i < obj->num_pics; i++)
		free_picture(&obj->pic[i]);
}

/*
** Rotates all the vehicle objects to so that all 16 rotations exist.
*/
rotate_objects()
{
	/* Rotate all of the vehicle objects */
	rotate_object(&lightc_obj, lightc_bitmap);
	rotate_object(&hexo_obj, hexo_bitmap);
	rotate_object(&spider_obj, spider_bitmap);
	rotate_object(&psycho_obj, psycho_bitmap);
	rotate_object(&tornado_obj, tornado_bitmap);
	rotate_object(&marauder_obj, marauder_bitmap);
	rotate_object(&tiger_obj, tiger_bitmap);
	rotate_object(&rhino_obj, rhino_bitmap);
	rotate_object(&medusa_obj, medusa_bitmap);
	rotate_object(&malice_obj, malice_bitmap);
	rotate_object(&trike_obj, trike_bitmap);
	rotate_object(&panzy_obj, panzy_bitmap);
	rotate_object(&disk_obj, disk_bitmap);
	rotate_object(&delta_obj, delta_bitmap);
}

/*
** Creates the 12 missing rotations from the first 4 rotations
** for every vehicle object.  This is done by 90 degree and 180
** degree rotations.
*/
rotate_object(obj, bitmap)
Object *obj;
Bits **bitmap;
{
	Picture *pic, *rot_pic;
	Bits *rotate_pic_90(), *rotate_pic_180();
	int dest, source;

	/* Create next 4 pictures by rotating the first 4 by 90 degrees */
	for (source = 0; source < 4; source++) {
		dest = source + 4;
		pic = &obj->pic[source];
		rot_pic = &obj->pic[dest];
		bitmap[dest] = (Bits *) rotate_pic_90(pic, rot_pic, bitmap[source]);
	}

	/* Create last 8 pictures by rotating the first 8 by 180 degrees */
	for (source = 0; source < 8; source++) {
		dest = source + 8;
		pic = &obj->pic[source];
		rot_pic = &obj->pic[dest];
		bitmap[dest] = (Bits *) rotate_pic_180(pic, rot_pic, bitmap[source]);
	}
}
