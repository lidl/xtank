/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** common.h
*/

#ifndef _COMMON_H_
#define _COMMON_H_


#define VERSION "1.2e-UMDENG"


/* Useful constants */
#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif FALSE

#ifndef TRUE
#define TRUE 1
#endif TRUE

#ifndef PI
#define PI	3.14159265358979323846
#endif

#define BAD_VALUE       (-1)


/* Useful macros */

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) > 0) ? (x) : -(x))
#define SIGN(x)	((x)<0?-1:(x)==0?0:1)
#define SQR(x) ((x)*(x))

#define rorre(dat) \
  {printf("error: %s\n", (char *)dat); \
  exit(17);}

/* avoid some lint warnings */
extern char *malloc(), *calloc(), *sprintf(), *strcpy(), *memset(), *memcpy();
extern long random();


#endif ndef _COMMON_H_
