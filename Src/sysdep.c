/*
**
** sysdep.c -- supplimental functions for lacking systems
**
*/

/*
$Author: lidl $
$Id: sysdep.c,v 1.5 1992/09/06 21:15:48 lidl Exp $

$Log: sysdep.c,v $
 * Revision 1.5  1992/09/06  21:15:48  lidl
 * fixed for bobcat machines (hp300 running 4.3BSD)
 *
 * Revision 1.4  1992/04/18  15:35:09  lidl
 * updated to work with sequents
 *
 * Revision 1.3  1992/04/09  04:12:41  lidl
 * remove warning from extra-stupid compilers
 *
 * Revision 1.2  1992/02/19  21:31:00  lidl
 * fixed up to work with ultirx :-)
 *
 * Revision 1.1  1991/12/07  07:00:34  lidl
 * Initial revision
 *
*/

/* these don't have strdup() in their c library, so here's a function */
#if defined(ultrix) || defined(sequent) || defined(hp300)
#include <stdio.h>

char * strdup(s1)
char *s1;
{
	char *newstr = NULL;

	newstr = (char *) malloc (strlen (s1) +1);
	if (newstr) {
		strcpy (newstr, s1);
		return (newstr);
	} else {
		return (NULL);
	}
}
#endif

int barf_o_rama()
{
	/* Some stupid C compilers complain when there is nothing */
	/* in a .c file, so here's a stupid function, just for    */
	/* extra-stupid compilers                                 */
	return 0;
}
