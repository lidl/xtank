/*
** Xtank
**
** $Id$
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

