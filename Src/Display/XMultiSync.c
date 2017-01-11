/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
