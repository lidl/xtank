/*
 * mazedesc.c
 */

#include <stdio.h>
#define UNIX

char pathname[] = "/mit/games/src/vax/xtank/";
char mazesdir[] = "Mazes/";

  
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

/* Additional flags used in maze */
#define BOX_CHANGED	(1<<5)
#define VEHICLE_0	(1<<8)
#define ANY_VEHICLE	0x0fffff00

/* Types of boxes */  
#define NORMAL		0
#define FUEL		1
#define AMMO		2
#define ARMOR		3
#define GOAL		4
#define OUTPOST		5
#define SCROLL_N	6
#define SCROLL_NE	7
#define SCROLL_E	8
#define SCROLL_SE	9
#define SCROLL_S	10
#define SCROLL_SW	11
#define SCROLL_W	12
#define SCROLL_NW	13
#define SLIP		14
#define SLOW		15
#define START_POS	16

typedef unsigned char Byte;

typedef struct {
  Byte type;
  char *name;
  char *designer;
  char *desc;
  Byte *data;
} Mdesc;

typedef unsigned int Flag;

typedef struct {
  Flag flags;			/* bits for walls, inside maze */
  Byte type;			/* landmark, scroll, goal, outpost, etc. */
  Byte team;			/* number of team that owns the box */
} Box;

Box box[GRID_WIDTH][GRID_HEIGHT];

/*
** Loads the maze description from the file "[mazesdir]/[name].m".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
load_mdesc(d,name)
     Mdesc *d;
     char *name;
{
  FILE *file;
  char filename[80];
  int temp;

#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,mazesdir);
#endif UNIX
#ifdef AMIGA
   (void) strcpy(filename,"XVDIR:");
#endif AMIGA
  (void) strcat(filename,name);
  (void) strcat(filename,".m");
  if((file = fopen(filename,"r")) == NULL) return DESC_NOT_FOUND;

  if((temp = getc(file)) == EOF) return DESC_BAD_FORMAT;
  d->type = (Byte) temp;
  if(alloc_str(file,&d->name) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,&d->designer) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,&d->desc) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,(char **) &d->data) == DESC_BAD_FORMAT)
     return DESC_BAD_FORMAT;
  
  (void) fclose(file);

  return DESC_LOADED;
}

/* At most 3 bytes per box in the grid for a maze description */
#define MAX_DATA_BYTES    GRID_WIDTH * GRID_HEIGHT * 3 + 1

/*
** Reads in a string from the file.  Mallocs the appropriate amount
** of memory for str, and copies the string into str.
** Returns DESC_BAD_FORMAT if string is too long or EOF is reached.
** Otherwise, returns DESC_LOADED.
*/
alloc_str(file,strp)
     FILE *file;
     char **strp;
{
  extern char *malloc();
  int ret;
  char temp[MAX_DATA_BYTES];
  unsigned int len;

  /* Read chars from file into temp up to and including the first '\0' */
  len = 0;
  do {
    if((ret = getc(file)) == EOF || len >= MAX_DATA_BYTES)
      return DESC_BAD_FORMAT;
    temp[len++] = (char) ret;
  } while((char) ret != '\0');

  *strp = malloc(len);
  (void) strcpy(*strp,temp);

  return DESC_LOADED;
}

/*
** Makes a maze structure from the specified maze description.
*/
make_maze(d)
     Mdesc *d;
{
  static Box empty_box = {0, 0, 0};
  Box *b;
  Byte flags,*dptr;
  int num_empties;
  int i,j;

  /*
  ** For each box there is a byte that contains 8 flags.
  **
  ** If EMPTY_BOXES is set, the remaining 7 bits give the number of
  ** empty boxes (excluding this one) to make before reading the next byte.
  **
  ** Otherwise,
  **   If TYPE_EXISTS is set, the next byte is the box type.
  **   Otherwise, the type is 0.
  **
  **   If TEAM_EXISTS is set, the next byte is the box team.
  **   Otherwise, the team is 0.
  */
  num_empties = 0;
  dptr = d->data;
  for (i = 0 ; i < GRID_WIDTH ; i++)
    for (j = 0 ; j < GRID_HEIGHT ; j++) {
      b = &box[i][j];

      /* Are we making empty boxes? */
      if(num_empties > 0) {
	*b = empty_box;
	num_empties--;
	continue;
      }

      flags = *(dptr++);

      /* Check for empty box flag */
      if(flags & EMPTY_BOXES) {
	*b = empty_box;
	num_empties = (int) (flags & ~EMPTY_BOXES);
      }
      else {
	/* Get the box values */
	b->flags = flags & MAZE_FLAGS;
	b->type = (flags & TYPE_EXISTS) ? *(dptr++) : 0;
	b->team = (flags & TEAM_EXISTS) ? *(dptr++) : 0;
      }
    }
}
