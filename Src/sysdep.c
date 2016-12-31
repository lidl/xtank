/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp
**
** $Id$
*/

/*
** Comment: supplemental functions for lacking systems
*/

/* if you don't have strdup() in your c library, here's a function */
#ifdef NEED_STRDUP
#include <stdio.h>

char *
strdup(char *s1)
{
	char *newstr;

	newstr = (char *) malloc(strlen(s1) + 1);
	if (newstr) {
		strcpy(newstr, s1);
		return (newstr);
	} else {
		return (NULL);
	}
}
#endif /* NEED_STRDUP */

#ifdef NEED_RINT
double
rint(double x) {
	if ((double) (x - (int) x) < 0.5) {
		return((int)x);
	} else {
		return((int)x + 1);
	}
}
#endif /* NEED_RINT */

/* stop the senseless input parsing */
int
yywrap(void) {
	return(1);
}

int
barf_o_rama(void)
{
	/* Some stupid C compilers complain when there is nothing */
	/* in a .c file, so here's a stupid function, just for    */
	/* extra-stupid compilers                                 */
	return 0;
}
