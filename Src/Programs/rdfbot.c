/*-
 * Copyright (c) 1992-1994 Aaron Nabil-Eastlund
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
 * Demo bot for rdf interface
 *
 * Set the "full map" flag, and use something
 * regular like djcmaze.
 *
 * Give the vehicle rdf and send it a ACK message for
 * it to dump state.
 *
 * Test vehicle must not have radar or newradar, as these
 * start up activated and will prevent RDF from working.
 */

#include <stdio.h>
#include "xtanklib.h"

static void rdfbot_main();

Prog_desc rdfbot_prog = {
	"rdfbot",
	"Any",
	"Demonstrator for RDF interface",
	"Aaron Nabil Eastlund",
	USES_MESSAGES,
	1,
	rdfbot_main
};

static void rdfbot_main() {

    Box (*my_map)[GRID_HEIGHT];
    int x,y;
    Message m;

    for (;;) {

    while (messages()) {

	receive_msg(&m);

	if (m.opcode == OP_ACK) {

	my_map = map_get();

/*
 * Erase the old RDF bits
 *
 * You can get a RDF pattern in the map, then
 * drive around, and get another WITHOUT clearing
 * the map, and the X bit will be set at the 
 * intersections.
 */

	for (y = 0; y < GRID_HEIGHT; y++) {
	    for (x = 0; x < GRID_WIDTH; x++) {
                my_map[x][y].flags &=
                  ~((unsigned int) 0 | ANY_RDF | X_RDF );
            }
        }

	rdf_map(my_map);

	printf("\n");
	for (y = 0; y < GRID_HEIGHT; y++) {
	    for (x = 0; x < GRID_WIDTH; x++) {
		if (my_map[x][y].flags & X_RDF)
		    printf("X");
		else if (my_map[x][y].flags & RED_RDF)
		    printf("R");
		else if (my_map[x][y].flags & GREEN_RDF)
		    printf("G");
		else if (my_map[x][y].flags & YELLOW_RDF)
		    printf("Y");
		else if (my_map[x][y].flags & INSIDE_MAZE)
		    printf("#");
		else
		    printf(" ");
	    }
	    printf("\n");
	}

	}
    }

    done();
}
}

