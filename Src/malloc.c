#include <stdio.h>

/*
$Author: stripes $
$Id: malloc.c,v 2.4 1991/03/25 00:42:11 stripes Exp $

$Log: malloc.c,v $
 * Revision 2.4  1991/03/25  00:42:11  stripes
 * RS6K Patches (IBM is a rock sucker)
 *
 * Revision 2.3  1991/02/10  13:51:07  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:24  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:14  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:59  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:43  aahz
 * Initial revision
 * 
*/

#if !defined(_IBMR2)
extern char *malloc();
#endif

/* Get around malloc(0)'s */

char *my_malloc(size)
unsigned size;
{
	char *mem;

	if (size == 0)
	{
		fprintf(stderr, "malloc(0)\n");
		size = 1;
	}
	mem = malloc(size);
	if (mem == 0L)
		fprintf(stderr, "can't malloc %d bytes\n", size);
	return (mem);
}
