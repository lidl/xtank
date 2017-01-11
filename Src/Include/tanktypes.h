/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
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
** Comment: simple typedefs that are used in many places
*/

#ifndef _TANKTYPES_H_
#define _TANKTYPES_H_

#include "sysdep.h"

typedef enum {
	False = 0,
	True = 1
} Boolean;

#define FALSE ((int) False)		/* lint likes these better :-) */
#define TRUE ((int) True)
typedef unsigned char Byte;
typedef unsigned int Flag;		/* a set of bits */
typedef FLOAT Angle;

#ifdef X10
typedef unsigned short Bits;
#endif

#ifdef X11
typedef Byte Bits;
#endif

#ifdef AMIGA
typedef unsigned short Bits;
#endif

typedef struct {
	short x, y;
} Coord;

typedef struct {
	long x, y;
} lCoord;

/* directions of rotation */
typedef enum {
	COUNTERCLOCKWISE = -1,
	NO_SPIN = 0,
	CLOCKWISE = 1,
	TOGGLE = 2
} Spin;

typedef enum {
	SP_nonexistent,
	SP_off,
	SP_on,
	SP_broken,
	real_MAX_SPEC_STATS
} SpecialStatus;

#define MAX_SPEC_STATS ((int)real_MAX_SPEC_STATS)

#endif /* _TANKTYPES_H_ */
