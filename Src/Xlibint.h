#include <X11/copyright.h>

/* $Header: /u2/rpotter/src/X/xtank1.2c/Src/RCS/Xlibint.h,v 1.2 90/08/06 23:43:05 rpotter Exp Locker: rpotter $ */
/* Copyright 1984, 1985, 1987  Massachusetts Institute of Technology */

/*
 *	XlibInternal.h - Header definition and support file for the internal
 *	support routines (XlibInternal) used by the C subroutine interface
 *	library (Xlib) to the X Window System.
 *
 *	Warning, there be dragons here....
 */

#ifndef NEED_EVENTS
#define _XEVENT_
#endif

#ifdef CRAY

#ifndef __TYPES__
#define __TYPES__
#include <sys/types.h>	/* forgot to protect it... */
#endif	/* __TYPES__ */

#else
#include <sys/types.h>
#endif	/* CRAY */

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include "Xlibos.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif

#define LOCKED 1
#define UNLOCKED 0

extern int errno;	/* Internal system error number. */
extern void bcopy();

extern (*_XIOErrorFunction) ();	/* X system error reporting routine. */
extern (*_XErrorFunction) ();	/* X_Error event reporting routine. */
extern char *_XAllocScratch();	/* fast memory allocator */
extern Visual *_XVIDtoVisual();	/* given visual id, find structure */

#ifndef BUFSIZE
#define BUFSIZE 2048	/* X output buffer size. */
#endif

#ifndef EPERBATCH
#define EPERBATCH 8	/* when batching, how many elements */
#endif

#ifndef CURSORFONT
#define CURSORFONT "cursor"	/* standard cursor fonts */
#endif

/*
 * X Protocol packetizing macros.
 */

/*   Need to start requests on 64 bit word boundries
 *   on a CRAY computer so add a NoOp (127) if needed.
 *   A character pointer on a CRAY computer will be non-zero
 *   after shifting right 61 bits of it is not pointing to
 *   a word boundary.
 */

#ifdef WORD64
#define WORD64ALIGN if ((long)dpy->bufptr >> 61) {\
           dpy->last_req = dpy->bufptr;\
           *(dpy->bufptr)   = X_NoOperation;\
           *(dpy->bufptr+1) =  0;\
           *(dpy->bufptr+2) =  0;\
           *(dpy->bufptr+3) =  1;\
             dpy->request += 1;\
             dpy->bufptr += 4;\
         }
#else	/* else does not require alignment on 64-bit boundaries */
#define WORD64ALIGN
#endif	/* WORD64 */


/*
 * GetReq - Get the next avilable X request packet in the buffer and
 * return it.
 *
 * "name" is the name of the request, e.g. CreatePixmap, OpenFont, etc.
 * "req" is the name of the request pointer.
 *
 */

#if (defined __STDC__) && (!defined UNIXCPP)
#define GetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(x/**/name/**/Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x##name##Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = (SIZEOF(x/**/name/**/Req))>>2;\
	dpy->bufptr += SIZEOF(x/**/name/**/Req);\
	dpy->request++

#else	/* non-ANSI C uses empty comment instead of "##" for token
	   concatenation */
#define GetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(x/**/name/**/Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = X_/**/name;\
	req->length = (SIZEOF(x/**/name/**/Req))>>2;\
	dpy->bufptr += SIZEOF(x/**/name/**/Req);\
	dpy->request++
#endif

/* GetReqExtra is the same as GetReq, but allocates "n" additional
   bytes after the request. "n" must be a multiple of 4!  */

#if (defined __STDC__) && (!defined UNIXCPP)
#define GetReqExtra(name, n, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(*req) + n) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x##name##Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = (SIZEOF(*req) + n)>>2;\
	dpy->bufptr += SIZEOF(*req) + n;\
	dpy->request++
#else
#define GetReqExtra(name, n, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(x/**/name/**/Req) + n) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = X_/**/name;\
	req->length = (SIZEOF(x/**/name/**/Req) + n)>>2;\
	dpy->bufptr += SIZEOF(x/**/name/**/Req) + n;\
	dpy->request++
#endif


/*
 * GetResReq is for those requests that have a resource ID
 * (Window, Pixmap, GContext, etc.) as their single argument.
 * "rid" is the name of the resource.
 */

#if (defined __STDC__) && (!defined UNIXCPP)
#define GetResReq(name, rid, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xResourceReq)) > dpy->bufmax)\
	    _XFlush(dpy);\
	req = (xResourceReq *) (dpy->last_req = dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = 2;\
	req->id = (rid);\
	dpy->bufptr += SIZEOF(xResourceReq);\
	dpy->request++
#else
#define GetResReq(name, rid, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xResourceReq)) > dpy->bufmax)\
	    _XFlush(dpy);\
	req = (xResourceReq *) (dpy->last_req = dpy->bufptr);\
	req->reqType = X_/**/name;\
	req->length = 2;\
	req->id = (rid);\
	dpy->bufptr += SIZEOF(xResourceReq);\
	dpy->request++
#endif

/*
 * GetEmptyReq is for those requests that have no arguments
 * at all.
 */

#if (defined __STDC__) && (!defined UNIXCPP)
#define GetEmptyReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xReq)) > dpy->bufmax)\
	    _XFlush(dpy);\
	req = (xReq *) (dpy->last_req = dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = 1;\
	dpy->bufptr += SIZEOF(xReq);\
	dpy->request++
#else
#define GetEmptyReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xReq)) > dpy->bufmax)\
	    _XFlush(dpy);\
	req = (xReq *) (dpy->last_req = dpy->bufptr);\
	req->reqType = X_/**/name;\
	req->length = 1;\
	dpy->bufptr += SIZEOF(xReq);\
	dpy->request++
#endif


#define SyncHandle() \
	if (dpy->synchandler) (*dpy->synchandler)(dpy)

#define FlushGC(dpy, gc) \
	if ((gc)->dirty) _XFlushGCCache((dpy), (gc))
/*
 * Data - Place data in the buffer and pad the end to provide
 * 32 bit word alignment.  Transmit if the buffer fills.
 *
 * "dpy" is a pointer to a Display.
 * "data" is a pinter to a data buffer.
 * "len" is the length of the data buffer.
 * we can presume buffer less than 2^16 bytes, so bcopy can be used safely.
 */
#define Data(dpy, data, len) \
	if (dpy->bufptr + (len) <= dpy->bufmax) {\
		bcopy(data, dpy->bufptr, (int)len);\
		dpy->bufptr += ((len) + 3) & ~3;\
	} else\
		_XSend(dpy, data, len)


/* Allocate bytes from the buffer.  No padding is done, so if
 * the length is not a multiple of 4, the caller must be
 * careful to leave the buffer aligned after sending the
 * current request.
 *
 * "type" is the type of the pointer being assigned to.
 * "ptr" is the pointer being assigned to.
 * "n" is the number of bytes to allocate.
 *
 * Example:
 *    xTextElt *elt;
 *    BufAlloc (xTextElt *, elt, nbytes)
 */

#define BufAlloc(type, ptr, n) \
    if (dpy->bufptr + (n) > dpy->bufmax) \
        _XFlush (dpy); \
    ptr = (type) dpy->bufptr; \
    dpy->bufptr += (n);

/*
 * provide emulation routines for smaller architectures
 */

#ifndef WORD64
#define Data16(dpy, data, len) Data((dpy), (char *)(data), (len))
#define Data32(dpy, data, len) Data((dpy), (char *)(data), (len))
#define _XRead16(dpy, data, len) _XRead((dpy), (char *)(data), (len))
#define _XRead32(dpy, data, len) _XRead((dpy), (char *)(data), (len))
#endif	/* not WORD64 */


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define	CI_NONEXISTCHAR	0x4000	/* required because QueryFont represents a
				   non-existant character with zero-value
				   metrics, but requires drivers to output
				   the default char in their place. */


/*
 * Stuff to handle large architecture machines; the constants were generated
 * on a 32-bit machine and must coorespond to the protocol.
 */

#ifdef WORD64
#define MUSTCOPY
#endif	/* WORD64 */


#ifdef MUSTCOPY

#if (defined __STDC__) && (!defined UNIXCPP)
#define SIZEOF(x) sizeof_##x
#else
#define SIZEOF(x) sizeof_/**/x
#endif	/* if ANSI C compiler else not */

#define NEXTPTR(p,t) (t *) (((char *) p) + SIZEOF(t))
#define INCPTR(p,t) p = (t *) (((char *) p) + SIZEOF(t))

/* a little bit of magic */
#define OneDataCard32(dpy,dstaddr,srcvar) \
  { dpy->bufptr -= 4; Data32 (dpy, (char *) &(srcvar), 4); }

#define sizeof_xSegment 8
#define sizeof_xPoint 4
#define sizeof_xRectangle 8
#define sizeof_xArc 12
#define sizeof_xConnClientPrefix 12
#define sizeof_xConnSetupPrefix 8
#define sizeof_xConnSetup 32
#define sizeof_xPixmapFormat 8
#define sizeof_xDepth 8
#define sizeof_xVisualType 24
#define sizeof_xWindowRoot 40
#define sizeof_xTimecoord 8
#define sizeof_xHostEntry 4
#define sizeof_xCharInfo 12
#define sizeof_xFontProp 8
#define sizeof_xTextElt 2
#define sizeof_xColorItem 12
#define sizeof_xrgb 8
#define sizeof_xGenericReply 32
#define sizeof_xGetWindowAttributesReply 44
#define sizeof_xGetGeometryReply 32
#define sizeof_xQueryTreeReply 32
#define sizeof_xInternAtomReply 32
#define sizeof_xGetAtomNameReply 32
#define sizeof_xGetPropertyReply 32
#define sizeof_xListPropertiesReply 32
#define sizeof_xGetSelectionOwnerReply 32
#define sizeof_xGrabPointerReply 32
#define sizeof_xQueryPointerReply 32
#define sizeof_xGetMotionEventsReply 32
#define sizeof_xTranslateCoordsReply 32
#define sizeof_xGetInputFocusReply 32
#define sizeof_xQueryKeymapReply 40
#define sizeof_xQueryFontReply 60
#define sizeof_xQueryTextExtentsReply 32
#define sizeof_xListFontsReply 32
#define sizeof_xGetFontPathReply 32
#define sizeof_xGetImageReply 32
#define sizeof_xListInstalledColormapsReply 32
#define sizeof_xAllocColorReply 32
#define sizeof_xAllocNamedColorReply 32
#define sizeof_xAllocColorCellsReply 32
#define sizeof_xAllocColorPlanesReply 32
#define sizeof_xQueryColorsReply 32
#define sizeof_xLookupColorReply 32
#define sizeof_xQueryBestSizeReply 32
#define sizeof_xQueryExtensionReply 32
#define sizeof_xListExtensionsReply 32
#define sizeof_xSetMappingReply 32
#define sizeof_xGetKeyboardControlReply 52
#define sizeof_xGetPointerControlReply 32
#define sizeof_xGetScreenSaverReply 32
#define sizeof_xListHostsReply 32
#define sizeof_xSetModifierMappingReply 32
#define sizeof_xError 32
#define sizeof_xEvent 32
#define sizeof_xKeymapEvent 32
#define sizeof_xReq 4
#define sizeof_xResourceReq 8
#define sizeof_xCreateWindowReq 32
#define sizeof_xChangeWindowAttributesReq 12
#define sizeof_xChangeSaveSetReq 8
#define sizeof_xReparentWindowReq 16
#define sizeof_xConfigureWindowReq 12
#define sizeof_xCirculateWindowReq 8
#define sizeof_xInternAtomReq 8
#define sizeof_xChangePropertyReq 24
#define sizeof_xDeletePropertyReq 12
#define sizeof_xGetPropertyReq 24
#define sizeof_xSetSelectionOwnerReq 16
#define sizeof_xConvertSelectionReq 24
#define sizeof_xSendEventReq 44
#define sizeof_xGrabPointerReq 24
#define sizeof_xGrabButtonReq 24
#define sizeof_xUngrabButtonReq 12
#define sizeof_xChangeActivePointerGrabReq 16
#define sizeof_xGrabKeyboardReq 16
#define sizeof_xGrabKeyReq 16
#define sizeof_xUngrabKeyReq 12
#define sizeof_xAllowEventsReq 8
#define sizeof_xGetMotionEventsReq 16
#define sizeof_xTranslateCoordsReq 16
#define sizeof_xWarpPointerReq 24
#define sizeof_xSetInputFocusReq 12
#define sizeof_xOpenFontReq 12
#define sizeof_xQueryTextExtentsReq 8
#define sizeof_xListFontsReq 8
#define sizeof_xSetFontPathReq 8
#define sizeof_xCreatePixmapReq 16
#define sizeof_xCreateGCReq 16
#define sizeof_xChangeGCReq 12
#define sizeof_xCopyGCReq 16
#define sizeof_xSetDashesReq 12
#define sizeof_xSetClipRectanglesReq 12
#define sizeof_xCopyAreaReq 28
#define sizeof_xCopyPlaneReq 32
#define sizeof_xPolyPointReq 12
#define sizeof_xPolySegmentReq 12
#define sizeof_xFillPolyReq 16
#define sizeof_xPutImageReq 24
#define sizeof_xGetImageReq 20
#define sizeof_xPolyTextReq 16
#define sizeof_xImageTextReq 16
#define sizeof_xCreateColormapReq 16
#define sizeof_xCopyColormapAndFreeReq 12
#define sizeof_xAllocColorReq 16
#define sizeof_xAllocNamedColorReq 12
#define sizeof_xAllocColorCellsReq 12
#define sizeof_xAllocColorPlanesReq 16
#define sizeof_xFreeColorsReq 12
#define sizeof_xStoreColorsReq 8
#define sizeof_xStoreNamedColorReq 16
#define sizeof_xQueryColorsReq 8
#define sizeof_xLookupColorReq 12
#define sizeof_xCreateCursorReq 32
#define sizeof_xCreateGlyphCursorReq 32
#define sizeof_xRecolorCursorReq 20
#define sizeof_xQueryBestSizeReq 12
#define sizeof_xQueryExtensionReq 8
#define sizeof_xChangeKeyboardControlReq 8
#define sizeof_xBellReq 4
#define sizeof_xChangePointerControlReq 12
#define sizeof_xSetScreenSaverReq 12
#define sizeof_xChangeHostsReq 8
#define sizeof_xListHostsReq 4
#define sizeof_xChangeModeReq 4
#define sizeof_xRotatePropertiesReq 12
#define sizeof_xReply 32
#define sizeof_xGrabKeyboardReply 32
#define sizeof_xListFontsWithInfoReply 60
#define sizeof_xSetPointerMappingReply 32
#define sizeof_xGetKeyboardMappingReply 32
#define sizeof_xGetPointerMappingReply 32
#define sizeof_xListFontsWithInfoReq 8
#define sizeof_xPolyLineReq 12
#define sizeof_xPolyArcReq 12
#define sizeof_xPolyRectangleReq 12
#define sizeof_xPolyFillRectangleReq 12
#define sizeof_xPolyFillArcReq 12
#define sizeof_xPolyText8Req 16
#define sizeof_xPolyText16Req 16
#define sizeof_xImageText8Req 16
#define sizeof_xImageText16Req 16
#define sizeof_xSetPointerMappingReq 4
#define sizeof_xForceScreenSaverReq 4
#define sizeof_xSetCloseDownModeReq 4
#define sizeof_xClearAreaReq 16
#define sizeof_xSetAccessControlReq 4
#define sizeof_xGetKeyboardMappingReq 8
#define sizeof_xSetModifierMappingReq 4
#define sizeof_xPropIconSize 24
#define sizeof_xChangeKeyboardMappingReq 8

#else	/* else not MUSTCOPY, this is used for 32-bit machines */

#ifndef SIZEOF
#define SIZEOF(x) sizeof(x)
#endif

#ifndef NEXTPTR
#define NEXTPTR(p,t) ((p)+1)
#endif

#ifndef INCPTR
#define INCPTR(p,t) (p++)
#endif

/* srcvar must be a variable for large architecture version */
#define OneDataCard32(dpy,dstaddr,srcvar) \
  { *(unsigned long *)(dstaddr) = (srcvar); }
#endif	/* MUSTCOPY - used machines whose C structs don't line up with proto */
