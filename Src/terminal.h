/* terminal.h - things describing a user's terminal */

#include "vehicle.h"



typedef struct {
    /* This should duplicate the definition for an XSegment */
    short x1, y1, x2, y2;
} Line;

typedef struct {
    int  num;
    char  player_name[MAX_STRING];
    int   vdesc;
    Vehicle *vehicle;
    int   status;
    Intloc loc;			/* coordinates of ulc of screen in maze */
    Intloc old_loc;
    char *video;		/* video info specific to machine */
    /* Rest is for 3d mode */
    float heading;		/* direction of view */
    float view_angle;		/* the angle of view width */
    float aspect;		/* the aspect ratio of the view */
    int   view_dist;		/* the range of sight in pixels */
    int   num_lines;		/* number of lines drawn */
    Line  line[MAX_LINES];	/* lines drawn on the screen */
    int   is_dead;
} Terminal;
