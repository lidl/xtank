/*
** cosell.h 
**
** This defines the constants for the commentator in xtank
*/


#ifndef _COSELL_
#define _COSELL_

#include "message.h"

#define TOO_MANY_SLICKS 6

#define SLICK_THRESH   100
#define CHANGE_THRESH    6
#define SMASH_THRESH    25
#define SLICKED_THRESH  20
#define BOBBLE_THRESH   50

#define WICKET_THRESH    4

#define HOWARD_SAYS_A_LITTLE(data) \
    compose_message(SENDER_COM, RECIPIENT_ALL, OP_TEXT, \
		    (Byte *)(sprintf data,buf))

#define HOWARD_SAYS(data) \
  return compose_message(SENDER_COM,RECIPIENT_ALL,OP_TEXT, \
			 (Byte *) (sprintf data,buf))

#define LAST_OWNER owners[(owner_index+9)%10]
#define LAST_NAME LAST_OWNER->disp

extern Bset *bset;
extern int num_vehicles;

#endif
