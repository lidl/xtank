/*
**
** sysdep.c -- supplimental functions for lacking systems
**
*/

/*
$Author: lidl $
$Id: sysdep.c,v 1.1 1991/12/07 07:00:34 lidl Exp $

$Log: sysdep.c,v $
 * Revision 1.1  1991/12/07  07:00:34  lidl
 * Initial revision
 *
*/

#if defined(ultrix)
/* ultrix doesn't have strdup() in its clib, so here's a macro */
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
