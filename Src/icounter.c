/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.c
*/

#include "icounter.h"
#include "common.h"

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define INC_TIME 10000

static struct itimerval start_val = { {0, INC_TIME}, {0, INC_TIME} };
static struct itimerval stop_val  = { {0, 0}, {0, 0} };

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
   if((int) signal(SIGVTALRM, increment_time) == -1)
     rorre("Could not set up interval timer signal handler.");
}

/*
** Start up interval timer to call increment_time every INC_TIME ms.
*/
start_counter()
{
   (void) setitimer(ITIMER_VIRTUAL,&start_val,(struct itimerval *) NULL);
}

/*
** Stop the interval timer.
*/
stop_counter()
{
   (void) setitimer(ITIMER_VIRTUAL,&stop_val,(struct itimerval *) NULL);
}

#endif

#ifdef AMIGA
 
struct Custom *c = (struct Custom *) 0xDFF000;
extern void      VB_count();
struct Interrupt intserver;
int              count;

setup_counter() 
{
  intserver.is_Node.ln_Type = NT_INTERRUPT;
  intserver.is_Node.ln_Pri  = 120;
  intserver.is_Node.ln_Name = "VB-count";
  intserver.is_Data         = (APTR) &count;
  intserver.is_Code         = VB_count; 
}
  
start_counter()
{
  AddIntServer(INTB_VERTB,&intserver);     
}
  
stop_counter()
{ 
  RemIntServer(INTB_VERTB,&intserver);
}

#endif
