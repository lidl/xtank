/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** graphics.c
*/

/*
$Author: lidl $
$Id: graphics.c,v 2.4 1991/09/15 09:24:51 lidl Exp $

$Log: graphics.c,v $
 * Revision 2.4  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.3  1991/02/10  13:50:37  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:52  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:32  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:30  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:24  aahz
 * Initial revision
 * 
*/

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
