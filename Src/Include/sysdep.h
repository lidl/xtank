/*-
 * Copyright (c) 1988 Terry Donahue
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

#ifndef SYSDEP_H
#define SYSDEP_H

#ifdef i860
#define STRUCT_ASSIGN(a,b,c) { memcpy(&(a), &(b), sizeof(c)); }
#else
#define STRUCT_ASSIGN(a,b,c) { a = b; }
#endif

#if defined(sun) || defined(hpux) || defined(apollo) || defined(mips) || defined(MOTOROLA) || defined(i860) || defined(mmax) || defined(__alpha) || defined(__bsdi__)
/* Avoid domain errors when both x and y are 0 */
#define ATAN2(_Y,_X) ((_X)==0 && (_Y)==0 ? 0.0 : atan2((double)_Y, (double)_X))
#else
#define ATAN2(Y,X) atan2((double)Y, (double)X)
#endif

/*
** Ultrix doesn't have a prototype for this, so we must.  Typical.
*/
#if defined(ultrix) || defined(sequent)
#if defined(__STDC__) || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif
double rint P_((double x));
#undef P_
#endif

/*
** Some ANSI compilers bitch and moan about float != double, and then
** also have different arguement promotion rules.
*/

#if 1 || defined(i860) || defined(sparc)
#define FLOAT double
#else
#define FLOAT float
#endif

#if defined(SYSV) || defined(SVR4)
#ifndef sgi
#define random lrand48
#endif
#if !defined(sun) || !defined(SVR4)
#define srandom srand48
#endif
#if defined(__hpux) || defined(__STDC__) || defined(STDC_LIBRARIES)
#define bcopy(s,d,n) memmove(d,s,n)
#else
#define bcopy(s,d,n) memcpy(d,s,n)
#endif
#define bzero(d,n) memset(d,0,n)
#define bcmp(a,b,n) memcmp(a,b,n)
#define index(s,c) strchr(s,c)
#ifndef sgi
#define rindex(s,c) strrchr(s,c)
#endif
#endif


#endif /* SYSDEP_H */
