/* $Header: XMultiSync.c,v 11.12 88/08/13 08:52:42 tmdonahu Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#define NEED_EVENTS
#ifndef __GNUC__
#define UNIXCPP
#endif
#include "Xlibint.h"

extern _XQEvent *_qfree;

/*
 * Synchronize multiple displays with errors and events, optionally
 * discarding pending events.
 */
XMultiSync (dpys, num_dpys, discard)
    Display *dpys[];
    int num_dpys;
    int discard;
{
    register Display *dpy;
    xGetInputFocusReply rep;
    register xReq *req;
    register int i;

    for(i = 0 ; i < num_dpys ; ++i) {
       dpy = dpys[i];
       LockDisplay(dpy);
    }

    /* Send out all the input focus requests at once */
    for(i = 0 ; i < num_dpys ; ++i) {
       dpy = dpys[i];
       GetEmptyReq(GetInputFocus, req);
       _XFlush(dpy);
    }

    /* Wait for all the replies to come back in */
    for(i = 0 ; i < num_dpys ; ++i) {
       dpy = dpys[i];
       (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    }

    if (discard) {
       for(i = 0 ; i < num_dpys ; ++i) {
          dpy = dpys[i];
          if(dpy->head) {
	     ((_XQEvent *)dpy->tail)->next = _qfree;
             _qfree = (_XQEvent *)dpy->head;
             dpy->head = dpy->tail = NULL;
             dpy->qlen = 0;
	  }
       }
    }

    for(i = 0 ; i < num_dpys ; ++i) {
       dpy = dpys[i];
       UnlockDisplay(dpy);
    }
}
