/* special.h - part of Xtank */

#ifndef _SPECIAL_H_
#define _SPECIAL_H_

#include "types.h"
#include "xtanklib.h"

#ifndef NO_NEW_RADAR
#ifndef _XTANK_H_
#include "xtank.h"
#endif
#endif /* !NO_NEW_RADAR */

typedef struct {
    char  string[MAX_STRING];
    int   value;
    int   color;
} Entry;

typedef struct {
    Entry _entry[MAX_ENTRIES];
    int   num_changes;
    int   change[MAX_ENTRIES];	/* indices of entries that changed */
} Console;

typedef struct {
    Boolean need_redisplay;
    Boolean initial_update;
    Boolean map_invalid;
    Coord marker;		/* shows vehicle location */
    Coord old_marker;
    Map   map;
    /* new symbols to display: */
    int   num_symbols;
    Landmark_info symbol[SQR(2 * NUM_BOXES + 1)];
    /* list of some known landmarks: */
    int   num_landmarks;
    Landmark_info landmark[MAX_LANDMARKS];
} Mapper;

#ifndef NO_NEW_RADAR

typedef struct {
   Coord draw_loc;
   Coord drawn_loc;
   Coord draw_grid;
   Coord drawn_grid;
   Boolean draw_radar;
   Boolean drawn_radar;
   Boolean draw_friend;
   Boolean drawn_friend;
   Boolean draw_tactical;
   Boolean drawn_tactical;
} newBlip;

typedef struct {
   int rad_frame_updated;
   int tac_frame_updated;
   Boolean need_redisplay;
   newBlip blip[MAX_VEHICLES];
   newBlip *map[GRID_WIDTH][GRID_HEIGHT];
} newRadar;

#endif /* NO_NEW_RADAR */

typedef struct {
    Byte  x;
    Byte  y;
    Byte  life;
    Byte  view;
    Byte  old_view;
    Byte  flags;
} Blip;

typedef struct {
    int   num_blips;
    Blip  blip[MAX_BLIPS];
    int   pos;			/* current rotation of sweep */
    int   start_x, start_y;	/* starting coordinate of sweep line */
    int   end_x, end_y;		/* ending coordinate of sweep line */
    int   old_start_x, old_start_y;
    int   old_end_x, old_end_y;
} Radar;

typedef enum {
    SP_nonexistent, SP_off, SP_on, SP_broken
#ifndef NO_NEW_RADAR
    , real_MAX_SPEC_STATS
#endif /* !NO_NEW_RADAR */
} SpecialStatus;

#ifndef NO_NEW_RADAR
#define MAX_SPEC_STATS ((int)real_MAX_SPEC_STATS) 
#endif /* !NO_NEW_RADAR */

typedef struct {
    SpecialStatus status;	/* status of the special */
    int (*proc)();		/* function to call for special */
    void *record;		/* pointer to special structure */
    Boolean shared;		/* flag for shared record space */
} Special;


#endif ndef _SPECIAL_H_
