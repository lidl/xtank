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
$Id: icounter.c,v 2.5 1991/09/15 09:24:51 lidl Exp $

$Log: icounter.c,v $
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

#include "icounter.h"
#include "common.h"
#include "types.h"

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define INC_TIME 10000

#if defined(MOTOROLA) && defined(m68k)
static unsigned int start_val = INC_TIME / 1000;
static unsigned int stop_val = 0;

int elapsed_time;

/*
** Virtual timer alarm handler.  Updates how much time has passed
** by incrementing elapsed_time by INC_TIME.
*/
increment_time()
{
	elapsed_time += INC_TIME;
}

setup_counter()
{
	if((int) sigset(SIGALRM, increment_time) == -1)
		rorre("Could not set up interval timer signal handler.");
}

/*
** Start up interval timer to call increment_time every INC_TIME ms.
*/
start_counter()
{
	(void) alarm(start_val);
}

/*
** Stop the interval timer.
*/
stop_counter()
{
	(void) alarm(stop_val);
}

#else /* MOTOROLA && 88k */

static struct itimerval start_val = {{0, INC_TIME}, {0, INC_TIME}};
static struct itimerval stop_val = {{0, 0}, {0, 0}};

int elapsed_time;

/*
** Virtual timer alarm handler.  Updates how much time has passed
** by incrementing elapsed_time by INC_TIME.
*/
void increment_time()
{
	elapsed_time += INC_TIME;
}

setup_counter()
{
#if defined(MOTOROLA) && defined(m88k)
	if ((int) sigset(SIGVTALRM, increment_time) == -1)
#else
	if ((int) signal(SIGVTALRM, increment_time) == -1)
#endif
		rorre("Could not set up interval timer signal handler.");
}

/*
** Start up interval timer to call increment_time every INC_TIME ms.
*/
start_counter()
{
	(void) setitimer(ITIMER_VIRTUAL, &start_val, (struct itimerval *) NULL);
}

/*
** Stop the interval timer.
*/
stop_counter()
{
	(void) setitimer(ITIMER_VIRTUAL, &stop_val, (struct itimerval *) NULL);
}
#endif /* MOTOROLA && m88k */


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

start_real_counter(time)
int time;
{
	struct itimerval real_timer;

    /* Set up a real-time interval timer that expires every time useconds.
       Each time it expires, the variable real_timer_expired will be set to
	   true. */
	timerclear(&real_timer.it_interval);
	timerclear(&real_timer.it_value);
	real_timer.it_interval.tv_usec = real_timer.it_value.tv_usec = time;
    (void) setitimer(ITIMER_REAL, &real_timer, (struct itimerval *)NULL);

	/* Call the sigalrm_handler function every time the handler expires */
#if defined(MOTOROLA) && defined(m88k)
	sigset(SIGALRM, sigalrm_handler);
#else
	signal(SIGALRM, sigalrm_handler);
#endif
}

wait_for_real_counter()
{
    /* The variable real_timer_expires will be set to true when the ITIMER_REAL
       timer expires.  Until then, pause() is called, which gives up process
       control until an interval timer expires. This way we don't waste CPU
       time. */
	while (!real_timer_expired)
		pause();
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
