
/*
** The Whole Common Thing
**
** Copyright 1988 by Terry Donahue
**
** common.h
*/

#ifndef _COMMON_H_
#define _COMMON_H_

/* constants useful in every day life */
#define COMPANY_NAME "The Company.  (C) 1988"

#ifndef FALSE
#define FALSE 0
#endif FALSE

#ifndef TRUE
#define TRUE 1
#endif TRUE

#ifndef PI
#define PI	3.1415926535
#endif PI

/* Useful macros */
#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif min

#ifndef max
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif max

#ifndef abs
#define abs(x) (((x) > 0) ? (x) : -(x))
#endif abs

#define rorre(dat) \
  {printf("error: %s\n", (char *)dat); \
  exit(17);}

/* all the types you WISHHHH C had built in */
typedef unsigned char Boolean;
typedef unsigned char Byte;

#include "config.h"
#endif _COMMON_H_
