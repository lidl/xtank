#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.c
*/

/*
$Author: lidl $
$Id: icounter.c,v 2.11 1992/08/31 01:51:30 lidl Exp $

$Log: icounter.c,v $
 * Revision 2.11  1992/08/31  01:51:30  lidl
 * changed to use tanktypes.h, instead of types.h
 *
 * Revision 2.10  1992/04/21  05:11:58  senft
 * Added support for no o and no delay options.
 *
 * Revision 2.9  1992/04/09  04:10:33  lidl
 * re-arranged to use #ifdef in a nicer manner, so that more code is
 * shared, and fewer sections are duplicated
 *
 * Revision 2.8  1991/12/27  01:40:08  lidl
 * added appropriate SVR4 defines
 *
 * Revision 2.7  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.6  1991/10/07  03:14:13  lidl
 * added multimax support (hopefully)
 *
 * Revision 2.5  1991/09/15  09:24:51  lidl
 * removed vestiges of config.h file, now all configuration is done in
 * the Imakefile, and propogated via compile-time -D flags
 *
 * Revision 2.4  1991/05/01  20:28:32  lidl
 * added Motorola SysVR3 unix support for both the m68k and the m88k
 *
 * Revision 2.3  1991/02/10  13:50:45  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:00  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:44  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:37  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:31  aahz
 * Initial revision
 * 
*/

#include "sysdep.h"
#include "icounter.h"
#include "common.h"
#include "tanktypes.h"
#include "clfkr.h"

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define INC_TIME 10000

int elapsed_time;

/*
** Virtual timer alarm handler.  Updates how much time has passed
** by incrementing elapsed_time by INC_TIME.
*/

#if defined(MOTOROLA) && defined(m68k)
int increment_time()
#else
void increment_time()
#endif
{
	elapsed_time += INC_TIME;
#ifdef mmax
	sigset(SIGVTALRM, increment_time);
#endif
}

#if defined(MOTOROLA) && defined(m68k)
#undef SIGVTALRM
#define SIGVTALRM SIGALRM
#endif

setup_counter()
{
#if defined(MOTOROLA) && defined(m88k) || defined(SVR4)
	if ((int) sigset(SIGVTALRM, increment_time) == -1)
#else
	if ((int) signal(SIGVTALRM, increment_time) == -1)
#endif
		rorre("Could not set up interval timer signal handler.");
}

#if defined(MOTOROLA) && defined(m68k)

static unsigned int start_val = INC_TIME / 1000;
static unsigned int stop_val = 0;

#else /* !defined(MOTOROLA) !! !definded(m68k) (ie everything else) */

static struct itimerval start_val = {{0, INC_TIME}, {0, INC_TIME}};
static struct itimerval stop_val = {{0, 0}, {0, 0}};

#endif /* !defined(MOTOROLA) !! !definded(m68k) (ie everything else) */

/*
** Start up interval timer to call increment_time every INC_TIME ms.
*/
start_counter()
{
#if defined(MOTOROLA) && defined(m68k)
	(void) alarm(start_val);
#else
	(void) setitimer(ITIMER_VIRTUAL, &start_val, (struct itimerval *) NULL);
#endif
}

/*
** Stop the interval timer.
*/
stop_counter()
{
#if defined(MOTOROLA) && defined(m68k)
	(void) alarm(stop_val);
#else
	(void) setitimer(ITIMER_VIRTUAL, &stop_val, (struct itimerval *) NULL);
#endif
}

/* Has enough real time passed since the last frame? */
static Boolean real_timer_expired = TRUE;

void sigalrm_handler()
{
	real_timer_expired = TRUE;
}

#if defined(MOTOROLA) && defined(m88k)
#define timerclear(tvp)	(tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#if defined(MOTOROLA) && defined(m68k)

start_real_counter(time)
int time;
{
}

wait_for_real_counter()
{
}

#else


extern struct CLFkr command_options; /* options for how xtank starts / exits */

start_real_counter(time)
int time;
{
	struct itimerval real_timer;

    if (!command_options.NoDelay)
	{
        /* Set up a real-time interval timer that expires every time useconds.
           Each time it expires, the variable real_timer_expired will be set to
	       true. */
	    timerclear(&real_timer.it_interval);
	    timerclear(&real_timer.it_value);
	    real_timer.it_interval.tv_usec = real_timer.it_value.tv_usec = time;
        (void) setitimer(ITIMER_REAL, &real_timer, (struct itimerval *)NULL);

	    /* Call the sigalrm_handler function every time the handler expires */
#if defined(MOTOROLA) && defined(m88k) || defined(SVR4)
	    sigset(SIGALRM, sigalrm_handler);
#else
	    signal(SIGALRM, sigalrm_handler);
#endif
    }
}

wait_for_real_counter()
{
    /* The variable real_timer_expires will be set to true when the ITIMER_REAL
       timer expires.  Until then, pause() is called, which gives up process
       control until an interval timer expires. This way we don't waste CPU
       time. */
    if (!command_options.NoDelay)
	{
	    while (!real_timer_expired)
		    pause();
    }
	real_timer_expired = FALSE;
}
#endif /* MOTOROLA && m68k */

#endif

#ifdef AMIGA
struct Custom *c = (struct Custom *) 0xDFF000;
extern void VB_count();
struct Interrupt intserver;
int count;

setup_counter()
{
	intserver.is_Node.ln_Type = NT_INTERRUPT;
	intserver.is_Node.ln_Pri = 120;
	intserver.is_Node.ln_Name = "VB-count";
	intserver.is_Data = (APTR) & count;
	intserver.is_Code = VB_count;
}

start_counter()
{
	AddIntServer(INTB_VERTB, &intserver);
}

stop_counter()
{
	RemIntServer(INTB_VERTB, &intserver);
}

start_real_counter(time)
int time;
{
}

wait_for_real_counter()
{
}

#endif
