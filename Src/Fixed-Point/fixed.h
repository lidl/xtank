/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** fixed.h  --  An efficient fixed point macro library.
**
**                          Conversions
**
** f2int(f)        integer of fixed f
** int2f(i)        fixed of integer i
**
** f2float(f)      float of fixed f
** float2f(flt)    fixed of float flt
**
** f2angle(f)      angle (-PI to PI) of fixed f
** angle2f(ang)    fixed of angle ang (-PI to PI)
** badangle2f(ang) fixed of angle ang (any value)
**
** f2grid(f)       integer grid value (0-128) of fixed location f
** f2box(f)        integer box component (0-256) of fixed location f
** loc2f(g,b)      fixed location formed from integer grid and box values
**
**                          Operations
**
** fmul(f1,f2)     fixed f1*f2
** fdiv(f1,f2)     fixed f1/f2
** fsqrt(f,r)      fixed r becomes square root of fixed f
** fdist(dx,dy)    fixed distance to fixed (dx,dy)
** fadist(dx,dy)   fixed distance to (dx,dy) where dx>0,dy>0
** fsquare(i)      fixed square of integer i
**
** frot(f)         rotation (0-15) of fixed angle f
** fsin(f)         fixed sine of fixed angle f
** fcos(f)         fixed cosine of fixed angle f
** fatan2(dy,dx)   angle of vector to (dx,dy)
**
** frnd()          random fixed number from -FPI to FPI
** rnd(num)        random integer from 0 to num-1
** rndm(mask)      random set of bits masked by mask
** seedrnd(num)    sets seed for random number generator
*/

typedef int Fixed;

extern int _seed;
extern Fixed _fatan[];
extern Fixed _fsin[];
extern Fixed _fsq[];

#define FSHIFT   16
#define FHSHIFT  8
#define F2FLOAT  1.525878e-05
#define F2ANGLE  1.462918e-09
#define FPI      (-(1<<31))
#define FPI2     (1<<30)

#define f2int(f)     ((f)>>FSHIFT)
#define int2f(i)     ((i)<<FSHIFT)

#define f2float(f)   ((float)(f)*F2FLOAT)
#define float2f(flt) ((int)((flt)/F2FLOAT))

#define f2angle(f)   ((float)(f)*F2ANGLE)
#define angle2f(ang) ((int)((ang)/F2ANGLE))

#ifdef AMIGA
#define badangle2f(ang) \
  ((ang = fmod(ang, 2*PI)) > PI ? (int)((ang-2*PI)/F2ANGLE) \
                                : (int)(ang/F2ANGLE))
#else
#define badangle2f(ang) (int)(drem(ang,2*PI)/F2ANGLE)
#endif

#define f2box(f) (((f)>>FSHIFT)&0xff)
#define f2grid(f) ((f)>>(FSHIFT+8))
#define loc2f(g,b) (((g)<<(FSHIFT+8))+((b)<<FSHIFT))

#define fmul(f1,f2)  (((f1)>>FHSHIFT)*((f2)>>FHSHIFT))

#define fdiv(f1,f2)  (((f1)/((f2)>>FHSHIFT))<<FHSHIFT)

#define fsqrt(f,r) do { r = f>>4; if((r&~0xff) == 0) break;  \
  r = (r+fdiv(f,r))>>1; r = (r+fdiv(f,r))>>1; r = (r+fdiv(f,r))>>1; \
  r = (r+fdiv(f,r))>>1; r = (r+fdiv(f,r))>>1; r = (r+fdiv(f,r))>>1; } while(0)

#define fadist(dx,dy) \
   (dx>dy ? (dx>(dy<<1) ? dx+(dy>>2) : dx+(((dy<<3)-dx-dy)>>4)) \
	  : (dy>(dx<<1) ? dy+(dx>>2) : dy+(((dx<<3)-dx-dy)>>4)))

#define fdist(dx,dy) ((dx<0 ? dx= -dx :0), (dy<0 ? dy= -dy :0), fadist(dx,dy))

#define fsquare(i) _fsq[i]

#define frot(f) (((unsigned int)(f)+0x08000000)>>28)

#define fsin(f) \
  (f>=0 ? (f<FPI2 ? _fsin[(f>>22)&0xff] : _fsin[255-((f>>22)&0xff)]) \
        : (f<-FPI2 ? -_fsin[(f>>22)&0xff]: -_fsin[255-((f>>22)&0xff)]))

#define fcos(f) \
  (f>=0 ? (f<FPI2 ? _fsin[255-((f>>22)&0xff)] : -_fsin[(f>>22)&0xff]) \
        : (f<-FPI2 ? -_fsin[255-((f>>22)&0xff)] : _fsin[(f>>22)&0xff]))

#define fatan2(dy,dx) (((dx|dy)&~0xff)==0 ? 0 :\
(dy>=0 ? (dx>=0\
? (dx>dy ? _fatan[(dy/(dx>>8))&0x1ff]\
: FPI2-_fatan[(dx/(dy>>8))&0x1ff])\
: (-dx>dy ? FPI-_fatan[(dy/((-dx)>>8))&0x1ff]\
: FPI2+_fatan[((-dx)/(dy>>8))&0x1ff]))\
: (dx>=0\
? (dx>-dy ? -_fatan[((-dy)/(dx>>8))&0x1ff]\
: -FPI2+_fatan[(dx/((-dy)>>8))&0x1ff])\
: (-dx>-dy ? -FPI+_fatan[((-dy)/((-dx)>>8))&0x1ff]\
: -FPI2-_fatan[((-dx)/((-dy)>>8))&0x1ff]))))

#define seedrnd(num) _seed = (num)
#define frnd() (_seed=_seed*11109+13849)
#define rnd(num) (((frnd()&0x7fff)>>1)%(num))
#define rndm(mask) ((frnd()>>1)&(mask))
