/*
 * dmaz - dumps an xtank maze out to a terminal.
 */

/* Return values for description loading */
#define DESC_LOADED     0
#define DESC_SAVED	1
#define DESC_NOT_FOUND  2
#define DESC_BAD_FORMAT 3
#define DESC_NO_ROOM    4

#define GRID_HEIGHT	30
#define GRID_WIDTH	30
#define MAZE_HEIGHT	26
#define MAZE_WIDTH	26
#define MAZE_TOP	2
#define MAZE_BOTTOM	27
#define MAZE_LEFT	2
#define MAZE_RIGHT	27

/* Flags for boxes in a maze description */
#define INSIDE_MAZE	(1<<0)
#define NORTH_WALL	(1<<1)
#define WEST_WALL	(1<<2)
#define NORTH_DEST	(1<<3)
#define WEST_DEST	(1<<4)
#define TYPE_EXISTS	(1<<5)
#define TEAM_EXISTS	(1<<6)
#define EMPTY_BOXES	(1<<7)
#define MAZE_FLAGS	(INSIDE_MAZE|NORTH_WALL|WEST_WALL|NORTH_DEST|WEST_DEST)

#include <stdio.h>
#include <sys/types.h>
typedef unsigned char Byte;

typedef struct {
    Byte type;
    char *name;
    char *designer;
    char *desc;
    Byte *data;
} Mdesc;

char *mtypes[] = {
    "Combat",
    "War",
    "Ultimate",
    "Capture",
    "Race",
};

char *derrs[] = {
    "none",
    "saved???",
    "Maze not found",
    "Maze bad format",
    "No room to store maze",
};

main(argc, argv)
    int argc;
    char **argv;
{
    Mdesc d;
    int er;
    char *ri, *rindex();

    if (ri = rindex(argv[1], '.'))
	*ri = '\0';

    if (er = load_mdesc(&d, argv[1])) {
	fputs("dmaz: ", stderr);
	fputs(derrs[er], stderr);
	putc('\n', stderr);
	exit(1);
    }
    fputs("Maze name: ", stdout);
    puts(d.name);
    fputs("Maze type: ", stdout);
    puts(mtypes[d.type]);
    fputs("Designer: ", stdout);
    puts(d.designer);
    fputs("Description: ", stdout);
    puts(d.desc);
    dumpmaze(&d);
    return 0;
}

/*
 * text layout of the maze:
 * north and west walls, real/dest, team, thing:
 *
 * === real wall --- dest wall
 * #             |
 * #             |
 *
 * ===           ===
 * # A           #1A
 * #A            #A1
 *
 */

char pics[17][4] =
{
    "    ",
    "FUEL",
    "AMMO",
    "ARMR",
    "GOAL",
    "OUCH",
    "/\\||",
    "-+/|",
    "->->",
    "\\|-+",
    "||\\/",
    "|/+-",
    "<-<-",
    "+-|\\",
    "SLIP",
    "SLOW",
    "\\//\\",
};


typedef unsigned int Flag;

typedef struct {
    Flag flags;			/* bits for walls, inside maze */
    Byte type;			/* landmark, scroll, goal, outpost, etc. */
    Byte team;			/* number of team that owns the box */
} Box;

extern Box box[GRID_WIDTH][GRID_HEIGHT];


dumpmaze(md)
    Mdesc *md;
{
    int i, j, k;
    Box bp;

    make_maze(md);

    for (i = MAZE_TOP; i <= MAZE_BOTTOM + 1; i++) {
	for (j = MAZE_LEFT; j <= MAZE_RIGHT; j++) {
	    int bpt = (box[j][i].flags & WEST_WALL)
	    || (box[j - 1][i].flags & NORTH_WALL);

	    bp = box[j][i];
	    if (bp.flags & NORTH_WALL) {
		if (bp.flags & NORTH_DEST) {
		    putchar(bpt ? '+' : '-');
		    putchar('-');
		    putchar('-');
		} else {
		    putchar(bpt ? '#' : '=');
		    putchar('=');
		    putchar('=');
		}
	    } else {
		putchar(bpt ? '#' : ' ');
		putchar(' ');
		putchar(' ');
	    }
	}
	putchar('\n');

	if (i <= MAZE_BOTTOM)
	    for (k = 0; k < 4; k += 2) {
		for (j = MAZE_LEFT; j <= MAZE_RIGHT; j++) {
		    bp = box[j][i];
		    if (bp.flags & WEST_WALL) {
			if (bp.flags & WEST_DEST)
			    putchar('|');
			else
			    putchar('#');
		    } else
			putchar(' ');

		    putchar(pics[bp.type][k]);
		    putchar(pics[bp.type][k + 1]);
		}
		putchar('\n');
	    }
    }
}
