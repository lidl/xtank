/*
$Author: lidl $
$Id: XMultiSync.c,v 2.5 1991/09/24 14:06:33 lidl Exp $

$Log: XMultiSync.c,v $
 * Revision 2.5  1991/09/24  14:06:33  lidl
 * Made to work with X11R5, also, Xlibint.h is not longer distributed here
 *
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:50:01  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:12  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:10:43  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:08:54  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:01:50  aahz
 * Initial revision
 * 
*/

#ifdef MULTI_SYNC
#include "malloc.h"

#define NEED_REPLIES
#define NEED_EVENTS

#ifndef __GNUC__
#define UNIXCPP
#endif

#include <X11/Xlibint.h>

extern _XQEvent *_qfree;

/*
 * Synchronize multiple displays with errors and events, optionally
 * discarding pending events.
 */
XMultiSync(dpys, num_dpys, discard)
Display *dpys[];
int num_dpys;
int discard;

{
	register Display *dpy;
	xGetInputFocusReply rep;
	register xReq *req;
	register int i;

	for (i = 0; i < num_dpys; ++i)
	{
		dpy = dpys[i];
		LockDisplay(dpy);
	}

	/* Send out all the input focus requests at once */
	for (i = 0; i < num_dpys; ++i)
	{
		dpy = dpys[i];
		GetEmptyReq(GetInputFocus, req);
		_XFlush(dpy);
	}

	/* Wait for all the replies to come back in */
	for (i = 0; i < num_dpys; ++i)
	{
		dpy = dpys[i];
		(void) _XReply(dpy, (xReply *) & rep, 0, xTrue);
	}

	if (discard)
	{
		for (i = 0; i < num_dpys; ++i)
		{
			dpy = dpys[i];
			if (dpy->head)
			{
				((_XQEvent *) dpy->tail)->next = _qfree;
				_qfree = (_XQEvent *) dpy->head;
				dpy->head = dpy->tail = NULL;
				dpy->qlen = 0;
			}
		}
	}
	for (i = 0; i < num_dpys; ++i)
	{
		dpy = dpys[i];
		UnlockDisplay(dpy);
	}
}

#endif							/* MULTI_SYNC */
