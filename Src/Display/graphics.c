/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** graphics.c
*/

/*
$Author: lidl $
$Id: graphics.c,v 1.1.1.1 1995/02/01 00:25:48 lidl Exp $
*/

#include "sysdep.h"
#include "malloc.h"

#ifdef X10
#include "x10.c"
#endif

#ifdef X11
#include "x11.c"
#endif

#ifdef AMIGA
#include "amigagfx.c"
#endif
