/* terminal.h - things describing a user's terminal */

/*
$Author: lidl $
$Id: terminal.h,v 2.5 1992/03/31 04:05:52 lidl Exp $

$Log: terminal.h,v $
 * Revision 2.5  1992/03/31  04:05:52  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.4  1991/12/10  01:21:04  lidl
 * change all occurances of "float" to "FLOAT"
 *
 * Revision 2.3  1991/02/10  13:51:47  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:59:07  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:13:09  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:37  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:03:10  aahz
 * Initial revision
 * 
*/

#include "vehicle.h"
#include "loc.h"


typedef struct {
    /* This should duplicate the definition for an XSegment */
    short x1, y1, x2, y2;
} Line;

typedef struct {
    int  num;
    char  player_name[MAX_STRING];
    int   vdesc;
    Vehicle *vehicle;		/* can be NULL */
    int   status;
    Intloc loc;				/* coordinates of ulc of screen in maze */
    Intloc old_loc;
    char *video;			/* video info specific to machine */
    /* Rest is for 3d mode */
    FLOAT heading;			/* direction of view */
    FLOAT view_angle;		/* the angle of view width */
    FLOAT aspect;			/* the aspect ratio of the view */
    int   view_dist;		/* the range of sight in pixels */
    int   num_lines;		/* number of lines drawn */
    Line  line[MAX_LINES];	/* lines drawn on the screen */
    Boolean observer;		/* True if this terminal is not controlling a
				   vehicle */
    int mouse_speed;		/* True if this terminal sets vehicle speed */
							/* by mouse clicks */
} Terminal;


extern Terminal *terminal[];
extern int num_terminals;
