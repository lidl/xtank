/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** graphics.c
*/

#include "malloc.h"
#include "config.h"

#ifdef X10
#include "x10.c"
#endif

#ifdef X11
#include "x11.c"
#endif

#ifdef AMIGA
#include "amigagfx.c"
#endif
