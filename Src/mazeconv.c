
/*
$Author: stripes $
$Id: mazeconv.c,v 2.5 1992/01/06 07:52:49 stripes Exp $

$Log: mazeconv.c,v $
 * Revision 2.5  1992/01/06  07:52:49  stripes
 * Changes for teleport
 *
 * Revision 2.4  1991/12/03  19:52:51  stripes
 * changed comment spacing
 *
 * Revision 2.3  1991/02/10  13:51:14  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:32  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:24  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:06  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:47  aahz
 * Initial revision
 * 
*/

static int external_type[] =
{
    NORMAL,
    FUEL,
    AMMO,
    ARMOR,
    GOAL,
    OUTPOST,
    SCROLL_N,
    SCROLL_NE,
    SCROLL_E,
    SCROLL_SE,
    SCROLL_S,
    SCROLL_SW,
    SCROLL_W,
    SCROLL_NW,
    SLIP,
    SLOW,
    START_POS,
    NORTH_SYM,
    WEST_SYM,
    NORTH_DEST_SYM,
    WEST_DEST_SYM,
    PEACE,
    TELEPORT,
	-1
};

#define TO_INTERNAL_TYPE       0
#define TO_EXTERNAL_TYPE       1

convert_maze(d, convtype)
	Mdesc *d;
	int convtype;
{
    Byte flags, *dptr;
    int ctr;

    /* For each box there is a byte that contains 8 flags.  If EMPTY_BOXES is
       set, the remaining 7 bits give the number of empty boxes (excluding this
       one) to make before reading the next byte.  Otherwise, If TYPE_EXISTS is
       set, the next byte is the box type.  Otherwise, the type is 0. If
       TEAM_EXISTS is set, the next byte is the box team.  Otherwise, the team
       is 0. */
    dptr = d->data;
    while (*dptr)
    {
	flags = *(dptr++);

	/* Check for empty box flag */
	if (! (flags & EMPTY_BOXES))
	{
	    if (flags & TYPE_EXISTS)
	    {
		if (convtype == TO_INTERNAL_TYPE)
		    *dptr = external_type[(LandmarkType) *dptr];
		else
		{
		    ctr = 0;
		    while (external_type[ctr] != -1)
		    {
			if (external_type[ctr] == (int)*dptr)
			{
			    *dptr = ctr;
			    break;
			}
			ctr++;
		    }
		    dptr++;
		}

		if (flags & TEAM_EXISTS)
		{
		    dptr++;
		}
	    }
	}
    }
}
