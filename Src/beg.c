static char sccsid[] = "@(#)newwin.c	1.1";

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#ifdef hpux
#include <time.h>
#else							/* hpux */
#include <sys/time.h>
#endif							/* hpux */

#include "defs.h"
#include "data.h"
#include "bitmaps.h"

static char rvbuf[BUFSIZE];
static struct player *Pkludge;

#define SIZEOF(a)	(sizeof (a) / sizeof (*(a)))

#define WINSIDE		500
#define BOXSIDE		(WINSIDE / 5)
#define BORDER		4
#define TILESIDE	32
#define MESSAGESIZE	20
#define STATSIZE	(MESSAGESIZE * 2 + BORDER)
#define YOFF		100

Pixmap XCBitmap();
Pixmap CXCBitmap();

char *
 SetUpWCP(p, display)
register struct player *p;
Display *display;
{
	register int i;
	register char *str;
	register struct player *j;
	unsigned long mask;
	XGCValues values;
	XSizeHints wininfo;
	XWMHints winhints;
	int uspec, rootX, rootY, dummy1, dummy2;
	Cursor crosshair;
	Pixmap cur, curm;
	XColor curf, curb;
	XSetWindowAttributes attr;


#ifdef notdef
	for (i = 0, j = &players[i]; i < MAXPLAYER; i++, j++)
	{
		if (j->p_status != PFREE && (j != p) && (j->p_flags & PFROBOT) == 0 &&
				strcmp(j->p_monitor, p->p_monitor) == 0)
		{
			if (j->p_status == PALIVE)
				sprintf(rvbuf, "%s already playing on %s", j->p_login, p->p_monitor);
			else
				sprintf(rvbuf, "Somone already playing on %s", p->p_monitor);
			return rvbuf;
		}
	}
#endif


	Pkludge = p;
kludge:
	p->display = display;

#ifdef fullfe
	if ((p->display = XOpenDisplay(p->p_monitor)) == NULL)
	{
		perror(p->p_monitor);
		if (errno == EINTR)
			goto kludge;
		p->p_status = PFREE;
		sprintf(rvbuf, "Problems with display %s", p->p_monitor);
		return rvbuf;
	}
	cur = XCreateBitmapFromData(p->display, DefaultRootWindow(p->display), crossbits, crossw, crossh);
	curm = XCreateBitmapFromData(p->display, DefaultRootWindow(p->display), crossmask_bits, crossw, crossh);
	curf.pixel = p->myColor;
	curb.pixel = p->backColor;
/*    XQueryColor(p->display, DefaultColormap(p->display,0), &curf); */
/*    XQueryColor(p->display, DefaultColormap(p->display,0), &curb); */
	curf.red = curf.blue = curf.green = 0xffff;
	curb.red = curb.blue = curb.green = 0x0000;
	crosshair = XCreatePixmapCursor(p->display, cur, curm, &curf, &curb, 8, 8);
	XFreePixmap(p->display, cur);
	XFreePixmap(p->display, curm);
	attr.background_pixel = p->backColor;
	attr.border_pixmap = p->gTile;
	attr.cursor = crosshair;
#endif

	p->screen = DefaultScreen(p->display);
	p->mono = XDisplayCells(p->display, p->screen) <= 2;
	p->w = DefaultRootWindow(p->display);
	p->mapw = p->w;
	p->xcn = XConnectionNumber(p->display);

	getColorDefs(p, PROGRAM_NAME);

#ifdef fullfe
	rootX = 0;
	rootY = YOFF;
	uspec = 0;
	if ((str = XGetDefault(p->display, PROGRAM_NAME, "geometry")) != NULL)
	{
		uspec = XParseGeometry(str, &rootX, &rootY, &dummy1, &dummy2);
		if ((uspec & (XValue | XNegative)) == (XValue | XNegative))
			rootX = -rootX;
		if ((uspec & (YValue | YNegative)) == (YValue | YNegative))
			rootY = -rootY;
	}
	p->baseWin = XCreateWindow(p->display, RootWindow(p->display, p->screen), rootX, rootY,
							   (unsigned) WINSIDE * 2 + 1 * BORDER, (unsigned) WINSIDE + 2 * BORDER + 2 * MESSAGESIZE,
							   BORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	wininfo.x = rootX;
	wininfo.y = rootY;
	wininfo.width = WINSIDE * 2 + 1 * BORDER;
	wininfo.height = WINSIDE + 2 * BORDER + 2 * MESSAGESIZE;
	wininfo.min_width = WINSIDE * 2 + 1 * BORDER;
	wininfo.min_height = WINSIDE + 2 * BORDER + 2 * MESSAGESIZE;
	wininfo.max_width = WINSIDE * 2 + 1 * BORDER;
	wininfo.max_height = WINSIDE + 2 * BORDER + 2 * MESSAGESIZE;
	if (uspec & (XValue | YValue | XNegative | YNegative))
		wininfo.flags = USPosition | PSize | PMinSize | PMaxSize;
	else
		wininfo.flags = PPosition | PSize | PMinSize | PMaxSize;
	XSetWindowBackground(p->display, p->baseWin, p->backColor);
	p->ibm = XCBitmap(p->display, p->baseWin, icon_bits, icon_width, icon_height);
	XSetStandardProperties(p->display, p->baseWin, PROGRAM_NAME, PROGRAM_NAME,
						   p->ibm, (char **) NULL, 0, &wininfo);

	rootX = 0;
	rootY = YOFF;
	uspec = 0;
	if ((str = XGetDefault(p->display, PROGRAM_NAME, "icon.geometry")) != NULL)
	{
		uspec = XParseGeometry(str, &rootX, &rootY, &dummy1, &dummy2);
		if ((uspec & (XValue | XNegative)) == (XValue | XNegative))
			rootX = -rootX;
		if ((uspec & (YValue | YNegative)) == (YValue | YNegative))
			rootY = -rootY;
	}
	p->iconWin = XCreateWindow(p->display, RootWindow(p->display, p->screen), rootX, rootY, (unsigned) icon_width,
							   (unsigned) icon_height, BORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->iconWin, p->backColor);
	winhints.icon_window = p->iconWin;
	winhints.flags = IconWindowHint;
	XSetWMHints(p->display, p->baseWin, &winhints);
	wininfo.x = rootX;
	wininfo.y = rootY;
	if (uspec & (XValue | YValue | XNegative | YNegative))
		wininfo.flags = USPosition;
	else
		wininfo.flags = PPosition;
	XSetNormalHints(p->display, p->iconWin, &wininfo);

	if (p->backColor == XBlackPixel(p->display, p->screen))
		values.function = GXcopy;
	else
		values.function = GXcopyInverted;
	values.graphics_exposures = 0;
	p->bmgc = XCreateGC(p->display, p->baseWin, GCFunction | GCGraphicsExposures, &values);
	p->gc = XCreateGC(p->display, p->baseWin, (unsigned long) 0, NULL);
	values.function = GXcopy;
	values.foreground = p->backColor;
	p->cleargc = XCreateGC(p->display, p->baseWin, GCFunction | GCForeground, &values);
	p->w = XCreateWindow(p->display, p->baseWin, -BORDER, -BORDER, (unsigned) WINSIDE, (unsigned) WINSIDE,
						 BORDER, DefaultDepth(p->display, p->screen), InputOutput, CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->w, p->backColor);
	XSetWindowBorder(p->display, p->w, p->borderColor);

	if (getFonts(p, PROGRAM_NAME))
	{
		XCloseDisplay(p->display);
		sprintf(rvbuf, "Not all fonts available on %s\n", p->p_monitor);
		return rvbuf;
	}
	p->mapw = XCreateWindow(p->display, p->baseWin, WINSIDE, -BORDER, (unsigned) WINSIDE,
							(unsigned) WINSIDE, BORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->mapw, p->backColor);
	XSetWindowBorder(p->display, p->mapw, p->borderColor);

	p->tstatw = XCreateWindow(p->display, p->baseWin, -BORDER, WINSIDE, (unsigned) WINSIDE,
							  (unsigned) STATSIZE, BORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->tstatw, p->backColor);
	XSetWindowBorder(p->display, p->tstatw, p->borderColor);

	p->warnw = XCreateWindow(p->display, p->baseWin, WINSIDE, WINSIDE,
							 (unsigned) WINSIDE, (unsigned) MESSAGESIZE, BORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->warnw, p->backColor);
	XSetWindowBorder(p->display, p->warnw, p->borderColor);

	p->messagew = XCreateWindow(p->display, p->baseWin, WINSIDE,
								WINSIDE + BORDER + MESSAGESIZE, (unsigned) WINSIDE, (unsigned) MESSAGESIZE, BORDER,
								DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->messagew, p->backColor);
	XSetWindowBorder(p->display, p->messagew, p->borderColor);

	p->planetw = XCreateWindow(p->display, p->w, 3, 3, (unsigned) 47 * fontWidth(p->dfont),
							   (unsigned) (MAXPLANETS + 3) * fontHeight(p->dfont), 2, DefaultDepth(p->display, p->screen), InputOutput,
							   (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->planetw, p->backColor);
	XSetWindowBorder(p->display, p->planetw, p->borderColor);

	p->playerw = XCreateWindow(p->display, p->w, 3, 3, (unsigned) 66 * fontWidth(p->dfont),
							   (unsigned) (MAXPLAYER + 3) * fontHeight(p->dfont), 2, DefaultDepth(p->display, p->screen), InputOutput,
							   (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->playerw, p->backColor);
	XSetWindowBorder(p->display, p->playerw, p->borderColor);

	p->helpWin = XCreateWindow(p->display, RootWindow(p->display, p->screen),
						   0, YOFF + WINSIDE + 2 * BORDER + 2 * MESSAGESIZE,
							   (unsigned) WINSIDE * 2 + 1 * BORDER, (unsigned) 10 * fontHeight(p->dfont), BORDER,
							   DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	wininfo.x = 0;
	wininfo.y = YOFF + WINSIDE + 2 * BORDER + 2 * MESSAGESIZE;
	wininfo.width = WINSIDE * 2 + 1 * BORDER;
	wininfo.height = 10 * fontHeight(p->dfont);
	wininfo.min_width = WINSIDE * 2 + 1 * BORDER;
	wininfo.min_height = 10 * fontHeight(p->dfont);
	wininfo.max_width = WINSIDE * 2 + 1 * BORDER;
	wininfo.max_height = 20 * fontHeight(p->dfont);
	wininfo.flags = PPosition | PSize | PMinSize | PMaxSize;
	XSetNormalHints(p->display, p->helpWin, &wininfo);
	XStoreName(p->display, p->helpWin, "xtrek-help");
	XSetWindowBackground(p->display, p->helpWin, p->backColor);
	XSetWindowBorder(p->display, p->helpWin, p->borderColor);


/* These windows will be used for setting one's warlike stats */

#define WARHEIGHT (fontHeight(p->dfont) * 2)
#define WARWIDTH (fontWidth(p->dfont) * 20)
#define WARBORDER 2
	p->war = XCreateWindow(p->display, p->baseWin, WINSIDE + 10, -BORDER + 10, (unsigned) WARWIDTH,
						   (unsigned) WARHEIGHT * 6, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->war, p->backColor);
	XSetWindowBorder(p->display, p->war, p->borderColor);

	p->warf = XCreateWindow(p->display, p->war, 0, 0 * WARHEIGHT, (unsigned) WARWIDTH,
							(unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->warf, p->backColor);
	XSetWindowBorder(p->display, p->warf, p->borderColor);

	p->warr = XCreateWindow(p->display, p->war, 0, 1 * WARHEIGHT, (unsigned) WARWIDTH,
							(unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
	XSetWindowBackground(p->display, p->warr, p->backColor);
	XSetWindowBorder(p->display, p->warr, p->borderColor);

	p->wark = XCreateWindow(p->display, p->war, 0, 2 * WARHEIGHT, (unsigned) WARWIDTH,
							(unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->wark, p->backColor);
	XSetWindowBorder(p->display, p->wark, p->borderColor);

	p->waro = XCreateWindow(p->display, p->war, 0, 3 * WARHEIGHT, (unsigned) WARWIDTH,
							(unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->waro, p->backColor);
	XSetWindowBorder(p->display, p->waro, p->borderColor);

	p->wargo = XCreateWindow(p->display, p->war, 0, 4 * WARHEIGHT, (unsigned) WARWIDTH,
							 (unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->wargo, p->backColor);
	XSetWindowBorder(p->display, p->wargo, p->borderColor);

	p->warno = XCreateWindow(p->display, p->war, 0, 5 * WARHEIGHT, (unsigned) WARWIDTH,
							 (unsigned) WARHEIGHT, WARBORDER, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, CWBackPixel | CWBorderPixmap | CWCursor, &attr);
	XSetWindowBackground(p->display, p->warno, p->backColor);
	XSetWindowBorder(p->display, p->warno, p->borderColor);


	XMapWindow(p->display, p->warf);
	XMapWindow(p->display, p->warr);
	XMapWindow(p->display, p->wark);
	XMapWindow(p->display, p->waro);
	XMapWindow(p->display, p->wargo);
	XMapWindow(p->display, p->warno);

	getResources(p, PROGRAM_NAME);
	mask = GCForeground | GCBackground;
	values.foreground = p->borderColor;
	values.background = p->backColor;
	XChangeGC(p->display, p->gc, mask, &values);
	XSetBackground(p->display, p->gc, (unsigned long) p->backColor);
	if (!p->mono)
	{
		XSetWindowBorder(p->display, p->baseWin, p->gColor);
		XSetWindowBorder(p->display, p->iconWin, p->gColor);
	}
	else
	{
		XSetWindowBorderPixmap(p->display, p->baseWin, p->gTile);
		XSetWindowBorder(p->display, p->iconWin, p->gTile);
	}
	p->p_status = POUTFIT;
#endif

	savebitmaps(p);
	return (char *) NULL;
}

CGBitmapFromData(dpy, d, dat, pixmaps, w, h, fg)
Display *dpy;
Drawable d;
char *dat;
Pixmap pixmaps[];
unsigned int w, h;
unsigned long fg;

{
	unsigned int i, j;
	int mask = 0xaa;
	char *mydat, *p;
	extern char *malloc();

	/* XSynchronize(dpy, 1); */
	p = mydat = malloc(w * h);
	if (mydat == NULL)
	{
		fprintf("Shit fit!\n");
		exit(33);
	}
	bcopy(dat, mydat, w * h);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i += 8)
		{
			*p++ = *p & mask;
		}
		mask = (mask == 0xaa) ? 0x55 : 0xaa;
	}
	pixmaps[0] = CXCBitmap(dpy, d, mydat, w, h, fg);
	/* addShields(mydat, pixmaps, fg); */
	free(mydat);
}

savebitmaps(p)
register struct player *p;
{
	register int i;

	for (i = 0; i < VIEWS; i++)
	{
		p->fedview[i][0][0] = CXCBitmap(p->display, p->w, fed_bits[i],
									ship_width, ship_height, p->shipCol[1]);
		p->romview[i][0][0] = CXCBitmap(p->display, p->w, rom_bits[i],
									ship_width, ship_height, p->shipCol[2]);
		p->kliview[i][0][0] = CXCBitmap(p->display, p->w, kli_bits[i],
									ship_width, ship_height, p->shipCol[3]);
		p->oriview[i][0][0] = CXCBitmap(p->display, p->w, ori_bits[i],
									ship_width, ship_height, p->shipCol[4]);
		/* Now the grays... */
		CGBitmapFromData(p->display, p->w, fed_bits[i], p->fedview[i][1],
						 ship_width, ship_height, p->shipCol[1]);
		CGBitmapFromData(p->display, p->w, rom_bits[i], p->romview[i][1],
						 ship_width, ship_height, p->shipCol[2]);
		CGBitmapFromData(p->display, p->w, kli_bits[i], p->kliview[i][1],
						 ship_width, ship_height, p->shipCol[3]);
		CGBitmapFromData(p->display, p->w, ori_bits[i], p->oriview[i][1],
						 ship_width, ship_height, p->shipCol[4]);
		/* Now add the sheilds... */
		addShields(fed_bits[i], p->fedview[i][0], p->shipCol[1], 0);
		addShields(rom_bits[i], p->romview[i][0], p->shipCol[2], 0);
		addShields(kli_bits[i], p->kliview[i][0], p->shipCol[3], 0);
		addShields(ori_bits[i], p->oriview[i][0], p->shipCol[4], 0);
		addShields(fed_bits[i], p->fedview[i][1], p->shipCol[1], 1);
		addShields(rom_bits[i], p->romview[i][1], p->shipCol[2], 1);
		addShields(kli_bits[i], p->kliview[i][1], p->shipCol[3], 1);
		addShields(ori_bits[i], p->oriview[i][1], p->shipCol[4], 1);
	}
	p->cloud = XCBitmap(p->display, p->w, cloud_bits,
						cloud_width, cloud_height);
	p->etorp = XCBitmap(p->display, p->w, etorp_bits,
						etorp_width, etorp_height);
	p->mtorp = XCBitmap(p->display, p->w, mtorp_bits,
						mtorp_width, mtorp_height);
	p->bplanet[0][0] = XCBitmap(p->display, p->w, indplanet_bits,
								planet_width, planet_height);
	p->bplanet[0][1] = XCBitmap(p->display, p->w, fedplanet_bits,
								planet_width, planet_height);
	p->bplanet[0][2] = XCBitmap(p->display, p->w, romplanet_bits,
								planet_width, planet_height);
	p->bplanet[0][3] = XCBitmap(p->display, p->w, kliplanet_bits,
								planet_width, planet_height);
	p->bplanet[0][4] = XCBitmap(p->display, p->w, oriplanet_bits,
								planet_width, planet_height);
	p->mbplanet[0][0] = XCBitmap(p->display, p->mapw, indmplanet_bits,
								 mplanet_width, mplanet_height);
	p->mbplanet[0][1] = XCBitmap(p->display, p->mapw, fedmplanet_bits,
								 mplanet_width, mplanet_height);
	p->mbplanet[0][2] = XCBitmap(p->display, p->mapw, rommplanet_bits,
								 mplanet_width, mplanet_height);
	p->mbplanet[0][3] = XCBitmap(p->display, p->mapw, klimplanet_bits,
								 mplanet_width, mplanet_height);
	p->mbplanet[0][4] = XCBitmap(p->display, p->mapw, orimplanet_bits,
								 mplanet_width, mplanet_height);
	for (i = 0; i < EX_FRAMES; i++)
	{
		p->expview[i] = XCBitmap(p->display, p->w, ex_bits[i],
								 ex_width, ex_height);
	}
}

/*
 * this is separate from newwin.  It should; be called *after*
 * openmem.  If not, expose events will be eaten by the forked
 * process (daemon).
 */

#ifdef notfe
mapAll(p)
register struct player *p;
{
	initinput(p);
	XMapWindow(p->display, p->mapw);
	XMapWindow(p->display, p->tstatw);
	XMapWindow(p->display, p->warnw);
	XMapWindow(p->display, p->messagew);
	XMapWindow(p->display, p->w);
	XMapWindow(p->display, p->baseWin);
}

#endif

/* This routine throws up an entry window for the player. */

entrywindow(p)
register struct player *p;
{
	/* The following allows quick choosing of teams */
	if ((p->p_team & FED) && !p->mustexit)
	{
		p->fwin = XCreateWindow(p->display, p->w, 0 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->fwin, KeyPressMask | ButtonPressMask | ButtonReleaseMask |
					 ExposureMask);
		XSetWindowBackground(p->display, p->fwin, p->backColor);
		XMapWindow(p->display, p->fwin);
	}
	else
	{
		p->fwin = XCreateWindow(p->display, p->w, 0 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->fwin, ExposureMask);
		XSetWindowBackgroundPixmap(p->display, p->fwin, p->stippleTile);
		XMapWindow(p->display, p->fwin);
	}
	XSetWindowBorder(p->display, p->fwin, p->shipCol[1]);

	if ((p->p_team & ROM) && !p->mustexit)
	{
		p->rwin = XCreateWindow(p->display, p->w, 1 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE, 1,
								DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->rwin, KeyPressMask | ButtonPressMask | ButtonReleaseMask |
					 ExposureMask);
		XSetWindowBackground(p->display, p->rwin, p->backColor);
		XMapWindow(p->display, p->rwin);
	}
	else
	{
		p->rwin = XCreateWindow(p->display, p->w, 1 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->rwin, ExposureMask);
		XSetWindowBackgroundPixmap(p->display, p->rwin, p->stippleTile);
		XMapWindow(p->display, p->rwin);
	}
	XSetWindowBorder(p->display, p->rwin, p->shipCol[2]);

	if ((p->p_team & KLI) && !p->mustexit)
	{
		p->kwin = XCreateWindow(p->display, p->w, 2 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->kwin, KeyPressMask | ButtonPressMask | ButtonReleaseMask |
					 ExposureMask);
		XSetWindowBackground(p->display, p->kwin, p->backColor);
		XMapWindow(p->display, p->kwin);
	}
	else
	{
		p->kwin = XCreateWindow(p->display, p->w, 2 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->kwin, ExposureMask);
		XSetWindowBackgroundPixmap(p->display, p->kwin, p->stippleTile);
		XMapWindow(p->display, p->kwin);
	}
	XSetWindowBorder(p->display, p->kwin, p->shipCol[3]);

	if ((p->p_team & ORI) && !p->mustexit)
	{
		p->owin = XCreateWindow(p->display, p->w, 3 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->owin, KeyPressMask | ButtonPressMask | ButtonReleaseMask |
					 ExposureMask);
		XSetWindowBackground(p->display, p->owin, p->backColor);
		XMapWindow(p->display, p->owin);
	}
	else
	{
		p->owin = XCreateWindow(p->display, p->w, 3 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE,
								1, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
		XSelectInput(p->display, p->owin, ExposureMask);
		XSetWindowBackgroundPixmap(p->display, p->owin, p->stippleTile);
		XMapWindow(p->display, p->owin);
	}
	XSetWindowBorder(p->display, p->owin, p->shipCol[4]);

	p->qwin = XCreateWindow(p->display, p->w, 4 * BOXSIDE, 400, (unsigned) BOXSIDE, (unsigned) BOXSIDE, 1,
							DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
	XSetWindowBackground(p->display, p->qwin, p->backColor);
	XSetWindowBorder(p->display, p->qwin, p->textColor);

	XSelectInput(p->display, p->qwin, KeyPressMask | ButtonPressMask | ExposureMask);
	XMapWindow(p->display, p->qwin);
	XClearWindow(p->display, p->qwin);

	p->startTime = time(0);
	makeClock(p, p->qwin);
}

void del_entrywindow(p)
register struct player *p;
{
	destroyClock(p);
	XDestroyWindow(p->display, p->fwin);
	XDestroyWindow(p->display, p->rwin);
	XDestroyWindow(p->display, p->kwin);
	XDestroyWindow(p->display, p->owin);
	XDestroyWindow(p->display, p->qwin);
	p->fwin = p->rwin = p->kwin = p->owin = p->qwin = (Window) NULL;
}

numShips(owner)
{

#ifdef notfe
	int i, num = 0;
	struct player *p;

	for (i = 0, p = players; i < MAXPLAYER; i++, p++)
		if (p->p_status == PALIVE && p->p_team == owner)
			num++;
	return (num);
#endif
}
static char *AUTHOR[] = {
	"",
	"---  XTREK Release Version 4.0 ---",
	"",
	"By Chris Guthrie (chris@ic.berkeley.edu)",
	"And Ed James (edjames@ic.berkeley.edu)",
	"",
	" Later X11R3 Mods by Dan A. Dickey (ddickey@unix.eta.com)"
};

showMotd(p)
register struct player *p;
{
	char buf[BUFSIZ];
	FILE *motd, *fopen();
	int i, length, top, center;

	/* Author Gratification */
	XClearWindow(p->display, p->w);
	for (i = 0; i < SIZEOF(AUTHOR); i++)
	{
		length = strlen(AUTHOR[i]);
		center = WINSIDE / 2 - (length * fontWidth(p->dfont)) / 2;
		XDrawImageString(p->display, p->w, p->dfgc, center, i * fontHeight(p->dfont) + p->dfont->ascent, AUTHOR[i],
						 length);
	}
	top = SIZEOF(AUTHOR) + 2;

	/* the following will print a motd */
	if ((motd = fopen(MOTD, "r")) != NULL)
	{
		for (i = top; fgets(buf, sizeof(buf), motd) != NULL; i++)
		{
			length = strlen(buf);
			buf[length - 1] = NULL;
			if (length > 80)
				length = 80;
			XDrawImageString(p->display, p->w, p->dfgc, 20, i * fontHeight(p->dfont) + p->dfont->ascent,
							 buf, length);
		}
		(void) fclose(motd);
	}
}

getResources(p, prog)
register struct player *p;
char *prog;
{

#ifdef notfe
	getTiles(p, prog);
#endif

#ifdef notdef
	p->showShields = booleanDefault(p, prog, "showShields");
#endif

#ifdef notfe
	if (booleanDefault(p, prog, "showstats"))
		p->p_flags |= PFSHOWSTATS;
#endif
}

static char solid[TILESIDE] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static char gray[TILESIDE] = {
	0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
	0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
	0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
	0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
};
static char striped[TILESIDE] = {
	0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
	0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f,
	0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
	0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0,
};

#ifdef notfe
getTiles(p, prog)
register struct player *p;
char *prog;
{
	register int i;
	char rPatt[TILESIDE], yPatt[TILESIDE], gPatt[TILESIDE];
	int rSize = sizeof(rPatt) / 2;
	int ySize = sizeof(yPatt) / 2;
	int gSize = sizeof(gPatt) / 2;
	XImage *image;
	GC gc;
	XGCValues gcv;

	if (p->mono)
	{
		if (p->backColor == XWhitePixel(p->display, p->screen))
		{
			/* Reverse video on, so reverse the bitmaps. */
			for (i = 0; i < TILESIDE; i++)
				solid[i] = ~solid[i];
			for (i = 0; i < TILESIDE; i++)
				gray[i] = ~gray[i];
			for (i = 0; i < TILESIDE; i++)
				striped[i] = ~striped[i];
		}
		if (arrayDefault(p, prog, "RalertPattern", &rSize, rPatt) < 0)
		{
			rSize = TILESIDE / 2;
			bcopy(striped, rPatt, sizeof(rPatt));
		}
		if (arrayDefault(p, prog, "YalertPattern", &ySize, yPatt) < 0)
		{
			ySize = TILESIDE / 2;
			bcopy(gray, yPatt, sizeof(yPatt));
		}
		if (arrayDefault(p, prog, "GalertPattern", &gSize, gPatt) < 0)
		{
			gSize = TILESIDE / 2;
			bcopy(solid, gPatt, sizeof(gPatt));
		}
		p->rTile = XCBitmap(p->display, p->w, rPatt, rSize, rSize);
		p->yTile = XCBitmap(p->display, p->w, yPatt, ySize, ySize);
		p->gTile = XCBitmap(p->display, p->w, gPatt, gSize, gSize);
	}
	p->stippleTile = XCreatePixmap(p->display, p->baseWin,
		stipple_width, stipple_height, DefaultDepth(p->display, p->screen));
	gcv.foreground = p->textColor;
	gcv.background = p->backColor;
	gc = XCreateGC(p->display, p->stippleTile, GCForeground | GCBackground, &gcv);
	image = XCreateImage(p->display, DefaultVisual(p->display, p->screen),
		 1, XYBitmap, 0, stipple_bits, stipple_width, stipple_height, 8, 0);
	XPutImage(p->display, p->stippleTile, gc, image, 0, 0, 0, 0,
			  stipple_width, stipple_height);
	XFree(image);
	XFreeGC(p->display, gc);
}

#endif

getFonts(p, prog)
register struct player *p;
char *prog;
{
	char *font_name;
	XFontStruct *XLoadQueryFont();
	XGCValues gcv;

	if ((font_name = XGetDefault(p->display, prog, "font")) == NULL)
	{
		font_name = "serif10";
	}
try_font:
	if ((p->dfont = XLoadQueryFont(p->display, font_name))
			== (XFontStruct *) NULL)
	{
		if (!strcmp("serif10", font_name))
		{
			font_name = "-adobe-courier-medium-o-normal--10*";
			goto try_font;
		}
		perror(font_name);
		return (1);
	}
	gcv.font = p->dfont->fid;
	gcv.foreground = p->textColor;
	gcv.background = p->backColor;
	p->dfgc = XCreateGC(p->display, p->w, GCForeground | GCBackground | GCFont, &gcv);

	if (XGetDefault(p->display, PROGRAM_NAME, "boldFont") == NULL)
	{
		if (!strcmp(font_name, "serif10"))
		{
			font_name = "serifb10";
		}
		else
		{
			font_name = "-adobe-courier-bold-o-normal--10*";
		}
	}
	else
	{
		font_name = XGetDefault(p->display, PROGRAM_NAME, "boldFont");
	}
	if ((p->bfont = XLoadQueryFont(p->display, font_name))
			== (XFontStruct *) NULL)
		p->bfont = p->dfont;
	gcv.font = p->bfont->fid;
	p->bfgc = XCreateGC(p->display, p->w, GCForeground | GCBackground | GCFont, &gcv);

	if ((font_name = XGetDefault(p->display, PROGRAM_NAME, "italicFont")) == NULL)
		font_name = "serifi10";
	if ((p->ifont = XLoadQueryFont(p->display, font_name))
			== (XFontStruct *) NULL)
		p->ifont = p->dfont;
	gcv.font = p->ifont->fid;
	p->ifgc = XCreateGC(p->display, p->w, GCForeground | GCBackground | GCFont, &gcv);

	if ((font_name = XGetDefault(p->display, PROGRAM_NAME, "bigFont")) == NULL)
		font_name = "vg-40";
	if ((p->bigFont = XLoadQueryFont(p->display, font_name))
			== (XFontStruct *) NULL)
		p->bigFont = p->dfont;
	gcv.font = p->bigFont->fid;
	p->bFgc = XCreateGC(p->display, p->w, GCForeground | GCBackground | GCFont, &gcv);

	if ((font_name = XGetDefault(p->display, PROGRAM_NAME, "xtrekFont")) == NULL)
		font_name = "xtrek";
	if ((p->xfont = XLoadQueryFont(p->display, font_name))
			== (XFontStruct *) NULL)
	{
		return (1);
	}
	gcv.font = p->xfont->fid;
	p->xfgc = XCreateGC(p->display, p->w, GCForeground | GCBackground | GCFont, &gcv);
	return (0);
}

redrawFed(p, fwin, flg)
register struct player *p;
Window fwin;
{
	char buf[BUFSIZ];
	static int numfeds = -1;

	if (numfeds == -1 || flg || numfeds != numShips(FED))
	{
		XClearWindow(p->display, fwin);
		XDrawImageString(p->display, fwin, p->dfgc, p->dfont->ascent, p->dfont->ascent, "Federation", 10);
		(void) sprintf(buf, "%d", numfeds = numShips(FED));
		XDrawString(p->display, fwin, p->bFgc, 5, p->bigFont->ascent + p->dfont->ascent + 15, buf, strlen(buf));
	}
}

redrawRom(p, rwin, flg)
register struct player *p;
Window rwin;
{
	char buf[BUFSIZ];
	static int numroms = -1;

	if (numroms == -1 || flg || numroms != numShips(ROM))
	{
		XClearWindow(p->display, rwin);
		XDrawImageString(p->display, rwin, p->dfgc, p->dfont->ascent, p->dfont->ascent,
						 "Romulan", 7);
		(void) sprintf(buf, "%d", numroms = numShips(ROM));
		XDrawString(p->display, rwin, p->bFgc, 5, p->bigFont->ascent + p->dfont->ascent + 15, buf, strlen(buf));
	}
}

redrawKli(p, kwin, flg)
register struct player *p;
Window kwin;
{
	char buf[BUFSIZ];
	static int numklis = -1;

	if (numklis == -1 || flg || numklis != numShips(KLI))
	{
		XClearWindow(p->display, kwin);
		XDrawImageString(p->display, kwin, p->dfgc, p->dfont->ascent, p->dfont->ascent,
						 "Klingon", 7);
		(void) sprintf(buf, "%d", numklis = numShips(KLI));
		XDrawString(p->display, kwin, p->bFgc, 5, p->bigFont->ascent + p->dfont->ascent + 15, buf, strlen(buf));
	}
}

redrawOri(p, owin, flg)
register struct player *p;
Window owin;
{
	char buf[BUFSIZ];
	static int numoris = -1;

	if (numoris == -1 || flg || numoris != numShips(ORI))
	{
		XClearWindow(p->display, owin);
		XDrawImageString(p->display, owin, p->dfgc, p->dfont->ascent, p->dfont->ascent,
						 "Orion", 5);
		(void) sprintf(buf, "%d", numoris = numShips(ORI));
		XDrawString(p->display, owin, p->bFgc, 5, p->bigFont->ascent + p->dfont->ascent + 15, buf, strlen(buf));
	}
}

redrawQuit(p, qwin)
register struct player *p;
Window qwin;
{
	XDrawImageString(p->display, qwin, p->dfgc, p->dfont->ascent, p->dfont->ascent, "Quit xtrek", 10);
}

char *help_message[] = {
	"0-9  Set speed",
	"k    Set course",
	"p    Fire phaser",
	"t    Launch torp",
	"d    detonate other torps",
	"D    detonate your torps",
	"+    Put up screens",
	"-    Put down screens",
	"u    Toggle screens",
	"b    Bomb planet",
	"z    Beam up armies",
	"x    Beam down armies",
	"B    Beam armies to other ship",
	"R    Enter repair mode",
	"o    orbit planet",
	"Q    Quit",
	"?    Review messages",
	"c    Toggle cloak mode",
	"C    Coup a planet",
	"l    Lock on to player/planet",
	"@    (Dis)Allow copilots",
	"L    List players",
	"P    List planets",
	"S    List scores",
	"s    (Un)Map status window",
	"X    Add 10 to next warp change",
	"M    Turn on/off map window updating",
	"N    Turn on/off name mode",
	"i    Get info on player/planet",
	"h    (Un)Map this window",
	"w    (Un)Map war window",
	"*    start a hoser robot",
	"&    send in a harder robot",
	0,
};

#define MAXHELP 40

fillhelp(p)
register struct player *p;
{
	register int i = 0, row, column;

	for (column = 0; column < 3 /* 4 Emc2 */ ; column++)
	{
		for (row = 1; row < 11 /* 9 Emc2 */ ; row++)
		{
			if (help_message[i] == 0)
				break;
			else
			{
				XDrawImageString(p->display, p->helpWin, p->dfgc, fontWidth(p->dfont) * (MAXHELP * column + 1),
							  fontHeight(p->dfont) * row + p->dfont->ascent,
								 help_message[i], strlen(help_message[i]));
				i++;
			}
		}
		if (help_message[i] == 0)
			break;
	}
}

drawIcon(p)
register struct player *p;
{
	XSetForeground(p->display, p->bmgc, p->textColor);
	XCopyPlane(p->display, p->ibm, p->iconWin, p->bmgc, 0, 0, icon_width, icon_height,
			   0, 0, 1);
}

#include "clock.bitmap"
#define CLOCK_WID	(BOXSIDE * 9 / 10)
#define CLOCK_HEI	(BOXSIDE * 2 / 3)
#define CLOCK_BDR	0
#define CLOCK_X		(BOXSIDE / 2 - CLOCK_WID / 2)
#define CLOCK_Y		(BOXSIDE / 2 - CLOCK_HEI / 2)

makeClock(p, w)
register struct player *p;
Window w;
{
	p->once = 0;
	p->oldtime = -1;
	p->clockw = XCreateWindow(p->display, w, CLOCK_X, CLOCK_Y, (unsigned) CLOCK_WID,
							  (unsigned) CLOCK_HEI, CLOCK_BDR, DefaultDepth(p->display, p->screen), InputOutput, (Visual *) CopyFromParent, 0L, (XSetWindowAttributes *) 0);
	XSetWindowBackground(p->display, p->clockw, p->backColor);
	XSetWindowBorder(p->display, p->clockw, p->backColor);
	XMapWindow(p->display, p->clockw);
	p->tbm = XCBitmap(p->display, p->clockw, clock_bits, clock_width, clock_height);

	XClearWindow(p->display, p->clockw);
}

destroyClock(p)
register struct player *p;
{
	XFreePixmap(p->display, p->tbm);
	p->tbm = (Pixmap) NULL;
	XDestroyWindow(p->display, p->clockw);
	p->clockw = (Window) NULL;
}

#define PI		3.141592654

showTimeLeft(p, time, max, flg)
register struct player *p;
{
	char buf[BUFSIZ], *cp;
	int cx, cy, ex, ey, tx, ty;

	if (!p->once || flg)
	{
		p->once = 1;
		XClearWindow(p->display, p->clockw);
		p->oldtime = -1;
		cx = CLOCK_WID / 2;		/* 45 */
		cy = (CLOCK_HEI - fontHeight(p->dfont)) / 2;	/* 26 */
		ex = cx - clock_width / 2;		/* 21 */
		ey = cy - clock_height / 2;		/* 2 */

		XSetForeground(p->display, p->bmgc, p->textColor);
		XCopyArea(p->display, p->tbm, p->clockw, p->bmgc, 0, 0, clock_width, clock_height,
				  ex, ey);

		cp = "Auto Quit";
		tx = cx - fontWidth(p->dfont) * strlen(cp) / 2;
		ty = CLOCK_HEI - p->dfont->descent;
		XDrawImageString(p->display, p->clockw, p->dfgc, tx, ty, cp, strlen(cp));
	}
	XSetFunction(p->display, p->dfgc, GXinvert);
	if (p->oldtime != -1)
	{
		cx = CLOCK_WID / 2;		/* 45 */
		cy = (CLOCK_HEI - fontHeight(p->dfont)) / 2;	/* 26 */
		ex = cx - clock_width * sin(2 * PI * p->oldtime / max) / 2;
		ey = cy - clock_height * cos(2 * PI * p->oldtime / max) / 2;
		XDrawLine(p->display, p->clockw, p->dfgc, cx, cy, ex, ey);
	}
	p->oldtime = time;

	cx = CLOCK_WID / 2;			/* 45 */
	cy = (CLOCK_HEI - fontHeight(p->dfont)) / 2;		/* 26 */
	ex = cx - clock_width * sin(2 * PI * time / max) / 2;
	ey = cy - clock_height * cos(2 * PI * time / max) / 2;
	XDrawLine(p->display, p->clockw, p->dfgc, cx, cy, ex, ey);
	XSetFunction(p->display, p->dfgc, GXcopy);

	sprintf(buf, "%2.2d", max - time);
	tx = cx - fontWidth(p->dfont) * strlen(buf) / 2;
	ty = cy - fontHeight(p->dfont) / 2;
	XDrawImageString(p->display, p->clockw, p->dfgc, tx, ty, buf, strlen(buf));
}

Pixmap XCBitmap(dpy, d, dat, w, h)
Display *dpy;
Drawable d;
char *dat;
unsigned int w, h;
{
	int depth;
	unsigned long fg, bg;

	depth = DefaultDepth(dpy, Pkludge->screen);
	fg = WhitePixel(dpy, Pkludge->screen);
	bg = BlackPixel(dpy, Pkludge->screen);
	return (XCreatePixmapFromBitmapData(dpy, d, dat, w, h, fg, bg, depth));
}

Pixmap CXCBitmap(dpy, d, dat, w, h, fg)
Display *dpy;
Drawable d;
char *dat;
unsigned int w, h;
unsigned long fg;
{
	int depth;
	unsigned long bg;

	depth = DefaultDepth(dpy, Pkludge->screen);
	bg = BlackPixel(dpy, Pkludge->screen);
	return (XCreatePixmapFromBitmapData(dpy, d, dat, w, h, fg, bg, depth));
}

addShields(dat, pmaps, fg, gr)
char *dat;
Pixmap pmaps[4];
unsigned long fg;
int gr;

{
	char tmp[60];
	char tmp2[32];
	int i, j, x, y;
	int sby, sbi, dby, dbi;

	if (1 || DefaultDepth(Pkludge->display, Pkludge->screen) == 1)
	{
		/* Mono */
		/* Ships are 16x16, shields are 20x20 */
		for (i = 1; i < 4; i++)
		{
			switch (i)
			{
				case 1:
					bcopy(shield_bits, tmp, 60);
					break;
				case 2:
					bcopy(yshield_bits, tmp, 60);
					break;
				case 3:
					bcopy(rshield_bits, tmp, 60);
					break;
			}
			bcopy(dat, tmp2, 32);
			if (gr)
			{
				for (j = 0; j < 8; j++)
					((unsigned long *) tmp2)[j] &= 0xAAAA5555;
			}
			for (y = 2; y < 18; y++)
			{
				for (x = 2; x < 18; x++)
				{
					sby = (y - 2) * 2 + (x - 2) / 8;
					sbi = (x - 2) % 8;
					dby = y * 3 + x / 8;
					dbi = x % 8;
					/* Warning: negative bit shift ahead! */
					if ((sbi - dbi) >= 0)
						tmp[dby] |= (tmp2[sby] & (1 << sbi)) >> (sbi - dbi);
					else
						tmp[dby] |= (tmp2[sby] & (1 << sbi)) << (dbi - sbi);
				}
			}
			pmaps[i] = CXCBitmap(Pkludge->display, Pkludge->w, tmp, 20, 20, fg);
		}
	}
	else
	{
		/* BS */
		fprintf(stderr, "Fuck off and die!\n");
		exit(-7);
	}
}
