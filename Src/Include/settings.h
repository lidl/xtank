/* settings.h - part of Xtank */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "maze.h"


/* game settings that are available to robot players */
typedef struct {
    Game game;				/* combat, war, ultimate, capture, race */
    Boolean ricochet;		/* whether bullets bounce off walls */
    Boolean rel_shoot;		/* whether shooter's speed added to bullet's */
    Boolean no_wear;		/* whether vehicles take damage & lose fuel */
    Boolean restart;		/* whether vehicles restart after death */
    Boolean full_map;		/* whether vehicles start out with full map */
    Boolean pay_to_play;	/* whether vehicles have to "pay to play" */
    Boolean no_nametags;	/* whether vehicles are anonymous */
    Boolean team_score;		/* whether to use team scoring */
    Boolean no_radar;		/* turns everyone's radar off */
    int winning_score;		/* score needed to win the game */
    int takeover_time;		/* how long you have to be in a square in order
				   to capture it (in War game) */
    int outpost_strength;	/* firepower of outposts (0-10) */
    int shocker_walls;		/* how much extra damage walls do (0-10) */
    FLOAT scroll_speed;		/* speed of scroll boxes (0-10) */
    FLOAT slip_friction;	/* friction in slip boxes (0-1) */
    FLOAT normal_friction;	/* friction in all other boxes (0-1) */
    FLOAT disc_friction;	/* friction factor applied to disc (0-1) */
    FLOAT box_slowdown;		/* slowdown caused by slow boxes (0-1) */
    FLOAT owner_slowdown;	/* how much to slow down disc owner (0-1) */
} Settings_info;

typedef struct {
    int   game_speed;		/* max. number of frames per second */
    Mdesc *mdesc;		/* pointer to maze description */
    int   maze_density;		/* % of walls kept in random maze (0 - 100) */
    Boolean point_bullets;	/* whether bullets are points or pictures */
    Boolean commentator;	/* whether commentator will comment on game */
    Boolean robots_dont_win;	/* whether robots can win a game */
    Boolean max_armor_scale;	/* scales armor to max instead of per side */
    int   difficulty;		/* difficulty of enemy robots (0 - 10) */
    Settings_info si;		/* lots of other info that is made available to
				   robot players */
} Settings;


#endif ndef _SETTINGS_H_
