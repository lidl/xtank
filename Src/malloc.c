
/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** malloc.c
*/

/*
$Author: lidl $
$Id: malloc.c,v 1.1.1.1 1995/02/01 00:25:36 lidl Exp $
*/

#include <stdio.h>
#if !defined(_IBMR2)
extern char *malloc();

#endif

/* Get around malloc(0)'s */

char *my_malloc(size)
unsigned size;
{
	char *mem;

	if (size == 0) {
		fprintf(stderr, "malloc(0)\n");
		size = 1;
	}
	mem = malloc(size);
	if (mem == 0L)
		fprintf(stderr, "can't malloc %d bytes\n", size);
	return (mem);
}
