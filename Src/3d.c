#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** 3d.c
*/

/*
$Author: rpotter $
$Id: 3d.c,v 2.3 1991/02/10 13:49:50 rpotter Exp $

$Log: 3d.c,v $
 * Revision 2.3  1991/02/10  13:49:50  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:00  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:10:26  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:08:44  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:34  aahz
 * Initial revision
 * 
*/

#include "terminal.h"


/*
** Displays the walls, bullets and vehicles in the animation window.
*/
/*ARGSUSED*/
display_anim_3d(status)
unsigned int status;
{
}


/*
** Initializes the 3d aspect ratio and view distance of the specified terminal.
*/
/*ARGSUSED*/
init_terminal_3d(t)
Terminal *t;
{
}

/*
** Transforms a (dx,dy) from screen coordinates to maze coordinates.
*/
/*ARGSUSED*/
transform_3d(dx, dy)
int *dx, *dy;
{
}

/*
** Toggles 3d, wide view, distance view, extend walls, and clipping.
*/
/*ARGSUSED*/
toggle_3d(mask)
int mask;
{
}
