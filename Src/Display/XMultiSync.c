/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#ifdef MULTI_SYNC

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
void
XMultiSync(Display *dpys[], int num_dpys, int discard)
{
	register Display *dpy;
	xGetInputFocusReply rep;
	register xReq *req;
	register int i;

	for (i = 0; i < num_dpys; ++i) {
		dpy = dpys[i];
		LockDisplay(dpy);
	}

	/* Send out all the input focus requests at once */
	for (i = 0; i < num_dpys; ++i) {
		dpy = dpys[i];
		GetEmptyReq(GetInputFocus, req);
		_XFlush(dpy);
	}

	/* Wait for all the replies to come back in */
	for (i = 0; i < num_dpys; ++i) {
		dpy = dpys[i];
		(void) _XReply(dpy, (xReply *) & rep, 0, xTrue);
	}

	if (discard) {
		for (i = 0; i < num_dpys; ++i) {
			dpy = dpys[i];
			if (dpy->head) {
				((_XQEvent *) dpy->tail)->next = _qfree;
				_qfree = (_XQEvent *) dpy->head;
				dpy->head = dpy->tail = NULL;
				dpy->qlen = 0;
			}
		}
	}
	for (i = 0; i < num_dpys; ++i) {
		dpy = dpys[i];
		UnlockDisplay(dpy);
	}
}

#endif /* MULTI_SYNC */
