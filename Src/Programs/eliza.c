/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
** This is an xtank program designed to talk with bored players.
*/

#include <stdio.h>
#include "xtanklib.h"

static void eliza_main(void);

Prog_desc eliza_prog = {
	"eliza",
	"Mauler",
	"Responds to messages sent from others.",
	"Terry Donahue",
	USES_MESSAGES,
	1,
	eliza_main
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

static void
eliza_main(void)
{
    Message m;
    int temp;

    for (;;)
    {
	while (messages())
	{
	    receive_msg(&m);
		if (m.sender != SENDER_COM && (int)m.opcode < MAX_OPCODES) {
			temp = max_responses[(int)m.opcode];
			if (temp > 0) {
				temp = random() % temp;
			} else {
				temp = 0;
			}
			send_msg(m.sender, OP_TEXT,
				(Byte *)response[(int)m.opcode][temp]);
		}
	}
	done();
    }
}
