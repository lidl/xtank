#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.c
*/

#include "icounter.h"
#include "common.h"
#include "types.h"
#include "config.h"

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define INC_TIME 10000

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
	if ((int) signal(SIGVTALRM, increment_time) == -1)
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


/* Has enough real time passed since the last frame? */
static Boolean real_timer_expired = TRUE;

void sigalrm_handler()
{
	real_timer_expired = TRUE;
}

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
	signal(SIGALRM, sigalrm_handler);
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
