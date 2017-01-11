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
