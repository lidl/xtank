/*
** To test the speed of integer, fixed point, and floating point math:
**
**     cc -O -o speed speed.c -lm
**     speed
**
*/

#include <stdio.h>
#include <math.h>
#include "fixed.h"
#include "ftables.h"

#ifndef AMIGA
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/resource.h>
#endif

empty_proc(n)
     int n;
{
  return n;
}

empty_test(n)
     int n;
{
  do { } while(--n >= 0);
  return n;
}

int_add_test(n)
     int n;
{
  int x=3,y=59;
  do { x += y; } while(--n >= 0);
  return x;
}

int_shift_test(n)
     int n;
{
  int x=3,y=5;
  do { x >>= y; } while(--n >= 0);
  return x;
}

int_mul_test(n)
     int n;
{
  int x=3,y=0x555555;
  do { x *= y; } while(--n >= 0);
  return x;
}

int_div_test(n)
     int n;
{
  int x=3,y=59;
  do { x /= y; } while(--n >= 0);
  return x;
}

float_add_test(n)
     int n;
{
  float x=3,y=5;
  do { x += y; } while(--n >= 0);
  return x;
}

float_mul_test(n)
     int n;
{
  float x=3,y=.5;
  do { x *= y; } while(--n >= 0);
  return x;
}

float_div_test(n)
     int n;
{
  float x=3,y=59;
  do { x /= y; } while(--n >= 0);
  return x;
}

double_add_test(n)
     int n;
{
  double x=3,y=5;
  do { x += y; } while(--n >= 0);
  return x;
}

double_mul_test(n)
     int n;
{
  double x=3,y=.5;
  do { x *= y; } while(--n >= 0);
  return x;
}

double_div_test(n)
     int n;
{
  double x=3,y=59;
  do { x /= y; } while(--n >= 0);
  return x;
}

sqrt_test(n)
     int n;
{
  float x=3;
  do { x = sqrt((double)x); } while(--n >= 0);
  return (int)x;
}

sin_test(n)
     int n;
{
  float x=3,y=59;
  do { x = sin((double)x); } while(--n >= 0);
  return (int)x;
}

atan2_test(n)
     int n;
{
  float x=3,y=59,z;
  do { z = atan2((double)y,(double)x); x=y; y=x; } while(--n >= 0);
  return (int)z;
}

dist_test(n)
     int n;
{
  float x=3,y=59,z;
  do { z = sqrt((double)(x*x+y*y)); x=y; y=x; } while(--n >= 0);
  return (int)z;
}

fsqrt_test(n)
     int n;
{
  Fixed x=int2f(3),y=int2f(59),z;
  do { fsqrt(x,z); x = y; y = x; } while(--n >= 0);
  return (int)z;
}

fsin_test(n)
     int n;
{
  Fixed x=int2f(3),y=int2f(59);
  do { x = fsin(x); } while(--n >= 0);
  return (int)x;
}

fatan2_test(n)
     int n;
{
  Fixed x=int2f(3),y=int2f(59),z;
  do { z = fatan2(y,x); x=y; y = x;} while(--n >= 0);
  return (int)z;
}

fdist_test(n)
     int n;
{
  int n1;
  Fixed x=int2f(3),y=int2f(3),z;

  n1 = n>>1;
  do { z=fdist(x,y); x=y; y=x;} while(--n1 >= 0);

  n1 = n>>1;
  x=int2f(-3); y=int2f(-3);
  do { z=fdist(x,y); x=y; y=x;} while(--n1 >= 0);
  return (int)z;
}

proc_test(n)
     int n;
{
  int x;

  do {x = empty_proc(n);} while(--n >= 0);
  return n;
}

f2float_test(n)
      int n;
{
   float y;
   
  do {y = f2float(n);} while(--n >=0);
  return (int) y;
}

float2f_test(n)
      int n;
{
  Fixed x;
  float y = 86.435;

  do {x = float2f(y += .001);} while(--n >= 0);
  return x;
}

/* returns CPU usage in seconds */
float gettime()
{
#ifdef AMIGA
    unsigned int clock[2];
    static unsigned int start_time=0;
    
    if (start_time == 0) {
      timer(clock);
      start_time = clock[0];
    }
    timer(clock);
    return (float)(clock[0]-start_time) + (float)clock[1]/1000000.;
#else
    static struct rusage val = { 0, 0 };
    getrusage(RUSAGE_SELF,&val);
    return (float)val.ru_utime.tv_sec + (float)val.ru_utime.tv_usec / 1000000;
#endif
}

char *test_name[] = {
  "empty loop","int +\t","int >>\t","int *\t","int /\t",
  "float +\t","float *\t","float /\t","double +","double *","double /",
  "sqrt\t","sin\t","atan2\t","dist\t","fsqrt\t","fsin\t","fatan2\t",
  "fdist\t", "subroutine", "f2float ", "float2f "
};

int (*test_func[])() = {
  empty_test, int_add_test, int_shift_test, int_mul_test, int_div_test,
  float_add_test, float_mul_test, float_div_test,
  double_add_test, double_mul_test, double_div_test,
  sqrt_test, sin_test, atan2_test, dist_test,
  fsqrt_test, fsin_test, fatan2_test, fdist_test,
  proc_test, f2float_test, float2f_test
};

int test_iter[] = {
  50,50,50,10,10,
  10,10,10,
  10,10,10,
  1,1,1,1,
  2,10,10,10,
  10,1,1
};

/* Returns # microseconds to perform the test */
float timed_test(i,iter)
     int i,iter;
{
  int n;
  float t;

  n = iter * test_iter[i];

  t = gettime();
  (*test_func[i])(n);
  t = gettime() - t;
  return t*1000000/(float)n;
}

main(argc,argv)
     int argc;
     char **argv;
{
  int n,iter,i,start,end;
  float t,micro,micro_empty;

#ifdef AMIGA
  iter = 50000;
#else
  iter = 100000;
#endif

  start = 1;
  end = 21; 
  if(argc > 1) iter *= atoi(argv[1]);
  if(argc > 2) start = atoi(argv[2]);
  if(argc > 3) end = atoi(argv[3]);

  micro_empty = timed_test(0,iter);

  puts("\n\nNum\tOperation\tTime (usec)");
  puts("---\t---------\t----");
  for(i = start ; i <= end ; i++) {
    micro = timed_test(i,iter) - micro_empty;
    printf("%2d.\t%s\t%6.2f\n",i,test_name[i],micro);
  }
}
