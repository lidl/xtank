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

/*
** Note:  This program must be recompiled every time you want to view a
**        new object.  Use the correct object format, and include your
**        file with a line like:
**
**        #include "test.obj"
**
**        That .obj file must have the following variables defined within it:
**
**        Object test_obj
**        short *test_bitmap[]
**
**
**        Compile the program by typing:
**
**        make objview
**
**
**        Then run the program by typing:
**
**        objview
**
*/

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>

/* Window geometry information */
#define ANIM_WIN	0

#define ANIM_WIN_X	0
#define ANIM_WIN_Y	0
#define ANIM_WIN_WIDTH	200
#define ANIM_WIN_HEIGHT	200

#define BORDER          3
#define MAX_TURRETS	4
#define MAX_SEGMENTS	6

/* Useful constants */
#define FALSE	0
#define TRUE	1

typedef unsigned int Boolean;

typedef struct _Coord {
    int x;
    int y;
} Coord;

typedef struct _Segment {
    int x1;
    int y1;
    int x2;
    int y2;
    int dx;
    int dy;
    float slope;
    int intercept;
    int minx;                   /* min(x1,x2) */
    int miny;                   /* min(y1,y2) */
    int maxx;                   /* max(x1,x2) */
    int maxy;                   /* max(y1,y2) */
} Segment;

typedef struct _Picture {
    int width;
    int height;
    int offset_x;
    int offset_y;
    Pixmap pixmap;
} Picture;

typedef struct _Picinfo {
    Coord turret_coord[MAX_TURRETS];
    Segment segment[MAX_SEGMENTS];
} Picinfo;

typedef struct _Object {
    char type[12];		/* type of object */
    int num_pics;		/* number of picture in the object */
    Picture *pic;		/* array of pictures of the object */
    int num_turrets;		/* number of turrets in object */
    int num_segs;		/* number of segments to represent object */
    Picinfo *picinfo;		/* array of info about pictures */
} Object;

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

typedef struct _Terminal {
    char display_name[MAXHOSTNAMELEN+6];
    Display *display;
    Window anim;
    Window win[1];
    int num_pixids;
    Pixmap pixid[200];
    GC gc;
} Terminal;

/* put your .obj filename here */

#include "test.obj"

#define draw_picture(w,x,y,pic,func, color) \
    { \
    int tmp_x = x - pic->offset_x; \
    int tmp_y = y - pic->offset_y; \
	if(x > -(pic)->offset_x && tmp_x < ANIM_WIN_WIDTH && \
           y > -(pic)->offset_y && tmp_y < ANIM_WIN_HEIGHT) { \
           XCopyPlane(term->display, term->pixid[(pic)->pixmap], \
              term->win[w], term->gc, 0, 0, \
                   pic->width, pic->height, tmp_x,tmp_y, 1); \
        } \
    }

#define DELAY_INC 20

#define NORMAL 0
#define FREEZE 1
#define STEP_FORWARD 2
#define STEP_BACKWARD 3

Terminal *term;
int delay;
unsigned int status;
Object *obj;
unsigned long int whitepixel, blackpixel;
int myscreen;

main()
{
    extern Object *obj;
    extern int delay;
    extern unsigned int status;
    Picture *pic, *old_pic;
    int x, y;
    int i, j;
    int next;

    create_terminal("");
    myscreen = DefaultScreen(term->display);
    whitepixel = WhitePixel(term->display,myscreen);
    blackpixel = BlackPixel(term->display,myscreen);

    setup_display();
    make_objects();

    delay = 800;
    x = 100;
    y = 100;

    pic = &obj->pic[0];
    draw_picture(ANIM_WIN, x, y, pic, GXxor, whitepixel);

    /* Main loop */
    for (;;) {
	synchronize();
	j = delay;
	while (j > 0) {
	    get_input();
	    switch (status) {
		case NORMAL:
		    --j;
		    next = i + 1;
		    break;
		case STEP_FORWARD:
		    j = 0;
		    next = i + 1;
		    status = FREEZE;
		    break;
		case STEP_BACKWARD:
		    j = 0;
		    next = i - 1;
		    status = FREEZE;
		    break;
		case FREEZE:
		    break;
	    }
	}

	i = next;
	if (i >= obj->num_pics)
	    i = 0;
	else if (i < 0)
	    i = obj->num_pics - 1;

	old_pic = pic;
	pic = &obj->pic[i];
	draw_picture(ANIM_WIN, x, y, old_pic, GXxor, whitepixel);
	draw_picture(ANIM_WIN, x, y, pic, GXxor, blackpixel);
    }
}


synchronize()
{
    XSync(0);
}

create_terminal(display_name)
    char *display_name;
{
    Display *dpy;

    dpy = XOpenDisplay(display_name);

    if (dpy == NULL)
	fprintf(stderr, "Could not open display %s.\n", display_name);
    else {
	term = (Terminal *) malloc(sizeof(Terminal));
	(void) strncpy(term->display_name, display_name, 50);
	term->display = dpy;
    }
}


setup_display()
{
    open_window();
    make_objects();
    map_window();
    synchronize();
}


open_window()
{
    unsigned int eventmask;

    eventmask = ExposureMask | ButtonPressMask | KeyPressMask;

    term->anim = XCreateWindow(term->display,
		RootWindow(term->display,0),
		ANIM_WIN_X, ANIM_WIN_Y, ANIM_WIN_WIDTH, ANIM_WIN_HEIGHT, 0,
		whitepixel, blackpixel);
    if (term->anim == NULL) {
	fprintf(stderr, "Could not open animation window\n");
	exit(1);
    }
    XSelectInput(term->display, term->anim, eventmask);
    term->win[ANIM_WIN] = term->anim;
    term->gc = DefaultGC(term->display, myscreen);
    XSetBackground(term->display, term->gc, blackpixel);
    XSetForeground(term->display, term->gc, whitepixel);
}


map_window()
{
    XMapWindow(term->win[ANIM_WIN]);
}


make_objects()
{
    extern Object *obj;
    extern Object *make_object();

    /* Make the object */
    obj = make_object(&test_obj, test_bitmap);
}


Object *
 make_object(obj, bitmap)
    Object *obj;
    short **bitmap;
{
    extern Terminal *term;
    Pixmap temp;
    Picture *pic;
    int i;

    for (i = 0; i < obj->num_pics; i++) {
	pic = &obj->pic[i];
	pic->pixmap = (Pixmap) term->num_pixids++;
	temp = XCreatePixmapFromBitmapData(term->display,
		term->anim, bitmap, pic->width, pic->height,
		whitepixel, blackpixel, 1);

	if (temp == NULL) {
	    fprintf(stderr, "Could not store pixmap\n");
	    exit(1);
	}
	printf("pic->pixmap = %d\n", pic->pixmap);
	term->pixid[(int) pic->pixmap] = temp;
    }
    return (obj);
}


get_input()
{
    extern Terminal *term;
    XEvent event;
    int value;

    while (XPending()) {
	XNextEvent(term->display, &event);
	switch (event.type) {
	    case ButtonPress:
		if (event.xbutton.window == term->anim ) {
		    value = anim_input(&event);
		}
		break;
	    default:
		break;
	}
    }
    return (value);
}


anim_input(event)
    XEvent *event;
{
    extern unsigned int status;
    extern int delay;
    XButtonEvent *tmp;
    int num_chars;
    char *buffer;

    switch ((int) event->type) {
	case ButtonPress:
	    tmp = (XButtonEvent *) event;
	    switch (tmp->state & 0xf) {
		case Button1:
		    /* Slow down animation */
		    if (status == NORMAL) {
			delay += DELAY_INC;
			if (delay > 10000)
			    delay = 10000;
		    } else
			status = STEP_BACKWARD;
		    break;
		case Button2:
		    /* Freeze/restart animation */
		    if (status == FREEZE)
			status = NORMAL;
		    else
			status = FREEZE;
		    break;
		case Button3:
		    /* Speed up animation */
		    if (status == NORMAL) {
			delay -= DELAY_INC;
			if (delay < 1)
			    delay = 1;
		    } else
			status = STEP_FORWARD;
	    }
    }
    return (0);
}


#ifdef notdef
rotate_objects()
{
    /* Rotate all of the vehicle objects */
    rotate_object(&standard_obj, (unsigned short **) standard_bitmap);
    rotate_object(&wasp_obj, (unsigned short **) wasp_bitmap);
    rotate_object(&rhino_obj, (unsigned short **) rhino_bitmap);
    rotate_object(&tornado_obj, (unsigned short **) tornado_bitmap);
}


rotate_object(obj, bitmap)
    Object *obj;
    unsigned short **bitmap;
{
    Picture *pic, *rot_pic;
    unsigned short *rotate_pic_90(), *rotate_pic_180();
    int dest, source;

    /* Create next 4 pictures by rotating the first 4 by 90 degrees */
    for (source = 0; source < 4; source++) {
	dest = source + 4;
	pic = &obj->pic[source];
	rot_pic = &obj->pic[dest];
	bitmap[dest] = rotate_pic_90(pic, rot_pic, bitmap[source]);
    }

    /* Create last 8 pictures by rotating the first 8 by 180 degrees */
    for (source = 0; source < 8; source++) {
	dest = source + 8;
	pic = &obj->pic[source];
	rot_pic = &obj->pic[dest];
	bitmap[dest] = rotate_pic_180(pic, rot_pic, bitmap[source]);
    }
}

#define SHORT_MASK_0 0x0001
#define SHORT_MASK_15 0x8000

unsigned int short_mask[16] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};

unsigned short *rotate_pic_90(pic, rot_pic, bitmap)
    Picture *pic, *rot_pic;
    unsigned short *bitmap;
{
    unsigned short *rot_bitmap;
    int sh_line, rot_sh_line, shorts, rot_shorts;
    register unsigned short *s_ptr, *d_ptr, *s_ptr_begin, *d_ptr_begin;
    register unsigned short s_mask, d_mask, s_mask_begin;

    rot_pic->width = pic->height;
    rot_pic->height = pic->width;
    rot_pic->offset_x = rot_pic->width - pic->offset_y - 1;
    rot_pic->offset_y = pic->offset_x;

    sh_line = (pic->width + 15) >> 4;
    shorts = sh_line * pic->height;
    rot_sh_line = (rot_pic->width + 15) >> 4;
    rot_shorts = rot_sh_line * rot_pic->height;
    rot_bitmap = (unsigned short *) calloc((unsigned) rot_shorts,
					   sizeof(unsigned short));

    if (rot_bitmap == NULL) {
	fprintf(stderr, "Could not allocate memory for some random bitmap.\n");
	exit(1);
    }
    /* Scan across each source scanline in the bitmap starting */
    /* from the bottom right corner and working left and up */
    s_ptr = s_ptr_begin = bitmap + shorts - 1;
    s_mask = s_mask_begin = short_mask[(pic->width - 1) & 0xf];
    d_ptr = d_ptr_begin = rot_bitmap + rot_shorts - sh_line;
    d_mask = SHORT_MASK_0;

    do {
	s_ptr_begin -= sh_line;

	do {
	    if (*s_ptr & s_mask)
		*d_ptr |= d_mask;

	    if ((s_mask >>= 1) == 0) {
		s_mask = SHORT_MASK_15;
		s_ptr--;
	    }
	    d_ptr -= rot_sh_line;
	} while (s_ptr > s_ptr_begin);

	if ((d_mask <<= 1) == 0) {
	    d_mask = SHORT_MASK_0;
	    d_ptr_begin++;
	}
	d_ptr = d_ptr_begin;

	s_mask = s_mask_begin;

    } while (s_ptr >= bitmap);

    return rot_bitmap;
}


/* Lookup table to palindrome a byte (patent pending) */
unsigned char reverse_byte[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


unsigned short *rotate_pic_180(pic, rot_pic, bitmap)
    Picture *pic, *rot_pic;
    unsigned short *bitmap;
{
    unsigned short *rot_bitmap;
    int sh_line, shorts;
    register unsigned short *s_ptr, *d_ptr;
    register unsigned char left_byte, right_byte;
    register unsigned short source_short, dest_short;
    register unsigned int dest_int;
    register int shift;

    *rot_pic = *pic;

    sh_line = (pic->width + 15) >> 4;
    shorts = sh_line * pic->height;
    rot_bitmap = (unsigned short *) calloc((unsigned) shorts,
					   sizeof(unsigned short));

    if (rot_bitmap == NULL) {
	fprintf(stderr, "Could not allocate memory for some random bitmap.\n");
	exit(1);
    }
    /* Scan across each source scanline in the bitmap starting */
    /* from the bottom right corner and working left and up */
    shift = pic->width & 0xf;
    s_ptr = bitmap + shorts - 1;
    d_ptr = rot_bitmap - 1;

    do {
	/* First swap first and second byte */
	source_short = *s_ptr;
	left_byte = (char) (source_short >> 8);
	right_byte = (char) source_short;
	dest_short = reverse_byte[left_byte] | (reverse_byte[right_byte] << 8);

	/* now shift the appropriate amount and or it into the destination */
	dest_int = (unsigned int) dest_short << shift;
	*((int *) d_ptr) |= dest_int;

	d_ptr++;
	s_ptr--;
    } while (s_ptr >= bitmap);

    return rot_bitmap;
}

#endif
