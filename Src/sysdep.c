/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp
**
** Comment: supplemental functions for lacking systems
**
** sysdep.c
*/

/*
$Author: lidl $
$Id: sysdep.c,v 1.1.1.2 1995/02/01 00:28:26 lidl Exp $
*/

/* these don't have strdup() in their c library, so here's a function */
#if defined(ultrix) || defined(sequent) || defined(hp300) || defined(sparc) || defined(NeXT)
#include <stdio.h>

char *strdup(s1)
char *s1;
{
	char *newstr = NULL;

	newstr = (char *) malloc(strlen(s1) + 1);
	if (newstr) {
		strcpy(newstr, s1);
		return (newstr);
	} else {
		return (NULL);
	}
}
#endif

#if defined(sequent)
double rint(double x) {
	if ((double) (x - (int) x) < 0.5) {
		return((int)x);
	} else {
		return((int)x + 1);
	}
}
#endif

/* stop the senseless input parsing */
int yywrap() {
	return(1);
}

int barf_o_rama()
{
	/* Some stupid C compilers complain when there is nothing */
	/* in a .c file, so here's a stupid function, just for    */
	/* extra-stupid compilers                                 */
	return 0;
}
