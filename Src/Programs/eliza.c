#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** eliza.c
**
** This is an xtank program designed to talk with bored players.
*/

/*
$Author: rpotter $
$Id: eliza.c,v 2.3 1991/02/10 13:50:25 rpotter Exp $

$Log: eliza.c,v $
 * Revision 2.3  1991/02/10  13:50:25  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:57:40  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:11:17  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:09:22  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:20  aahz
 * Initial revision
 * 
*/

#include <stdio.h>
#include "xtanklib.h"

static void main();

Prog_desc eliza_prog = {
	"eliza",
	"Mauler",
	"Responds to messages sent from others.",
	"Terry Donahue",
	USES_MESSAGES,
	1,
	main
};

static int max_responses[MAX_OPCODES] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 17};

static char *response[MAX_OPCODES][17] = {
    {				/* Responses to location */
	"So that's where you are!"
	},
    {				/* Responses to goto */
	"Gonna make me?"
	},
    {				/* Responses to follow */
	"Doesn't look like a honda."
	},
    {				/* Responses to help */
	"You think you've got it bad?"
	},
    {				/* Responses to attack */
	"Are you kidding?"
	},
    {				/* Responses to open */
	"But you're miles away!"
	},
    {				/* Responses to throw */
	"Be there in a jiffy."
	},
    {				/* Responses to caught */
	"Impressive."
	},
    {				/* Responses to ack */
	"Roger Roger."
	},
    {				/* Responses to text */
	"Why are you so aggressive?",
	"Tell me about yourself.",
	"I see...",
	"Wars have started over less.",
	"We've got you surrounded.",
	"Come out with your hands up.",
	"Don't talk to me about life...",
	"The first million years were...",
	"Do you have a problem?",
	"Suddenly I was surrounded!",
	"Is anybody out there?",
	"Turn to the Dark Side, Luke",
	"Ground control to Major Tom.",
	"Piiigs iiiiin SPAAAAACE!",
	"Meet George Jetson!",
	"His boy Elroy!",
	"Space.  The final frontier"
	}
};

static void main()
{
    Message m;

    for (;;)
    {
	while (messages())
	{
	    receive_msg(&m);
            if (m.sender != SENDER_COM && (int)m.opcode < MAX_OPCODES)
		send_msg(m.sender, OP_TEXT,
			 (Byte *)response[(int)m.opcode]
			            [random() % max_responses[(int)m.opcode]]);
	}
	done();
    }
}
