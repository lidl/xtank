/*
** ftest.c
**
** To test the fixed point routines:
**
**     cc -o ftest ftest.c -lm
**     ftest | more
*/

#include <math.h>
#include "fixed.h"
#include "ftables.h"

#define PI 3.14159265358979323
#define DEG2RAD (PI/180)

#define INC 5

main()
{
  Fixed f,r,dx,dy;
  double ang,ang2;
  int i;

  printf("\ndegs\tfrot\tfsin\tsin\tfcos\tcos\n");
  printf("----\t----\t----\t---\t----\t---\n");
  for(i = 0 ; i < 360 ; i+=INC) {
    ang = i*DEG2RAD;
    f = angle2f(ang);
    printf("%d\t%d\t%.4f\t%.4f\t%.4f\t%.4f\n",
	   i,frot(f),f2float(fsin(f)),(float)sin(ang),
	   f2float(fcos(f)),(float)cos(ang));
  }

  printf("\nnum\tfsqrt\tsqrt\n");
  printf("---\t------\t----\n");
  for(i = 0 ; i < 30000 ; i+=INC*200) {
    f = int2f(i);
    fsqrt(f,r);
    printf("%d\t%.3f\t%.3f\n",i,f2float(r),(float)sqrt((double)i));
  }

  printf("\ndegs\tdx\tdy\tang\tfatan2\tfdist\n");
  printf("----\t--\t--\t---\t------\t-----\n");
  for(i = 0 ; i < 360 ; i+=INC) {
    ang = i*DEG2RAD;
    dx = float2f(100*cos(ang));
    dy = float2f(100*sin(ang));
    ang2 = f2angle(fatan2(dy,dx));
    printf("%d\t%.2f\t%.2f\t%.4f\t%.4f\t%.2f\n",
	   i,f2float(dx),f2float(dy),(float)ang,(float)ang2,
	   f2float(fdist(dx,dy)));
  }
}
