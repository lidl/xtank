/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "map.h"
#include "vehicle.h"
#include "terminal.h"
#include "proto.h"


extern Terminal *term;
extern Map real_map;
extern int team_color_bright[];


#ifdef OLD_MARKER
#define draw_marker(x,y,c) \
  draw_filled_square(MAP_WIN,x,y,MAP_BOX_SIZE-3,DRAW_XOR,WHITE)
#define draw_grid_marker(x,y) \
  draw_marker(grid2map(x+NUM_BOXES/2)+2, grid2map(y+NUM_BOXES/2)+2, CUR_COLOR)
#else
#define draw_marker(x,y,c) \
  { \
	draw_line(MAP_WIN, x, y, x+5, y+5, DRAW_XOR, c); \
    draw_line(MAP_WIN, x+5, y, x, y+5, DRAW_XOR, c); \
  }

#define draw_grid_marker(x,y) \
  { \
    int x1 = grid2map(x+NUM_BOXES/2)+2; \
    int y1 = grid2map(y+NUM_BOXES/2)+2; \
    draw_filled_square(MAP_WIN,x1,y1,MAP_BOX_SIZE-3,DRAW_XOR,WHITE); \
  }
#endif

/*
** Handles all mapper actions for the specified vehicle.
*/
SpecialStatus special_mapper(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Mapper *m = (Mapper *) record;
	Landmark_info *s;
	Box *b, *mb;
	int left, top, right, bottom;
	int left_x, top_y, line_x, line_y;
	unsigned int flags;
	int i, x, y;
	int veh;
	Vehicle *mv;

	switch (action) {
	  case SP_update:
		  /* If the vehicle hasn't move to a new box, do nothing */
		  if ((v->loc->grid_x == v->old_loc->grid_x) &&
			  (v->loc->grid_y == v->old_loc->grid_y) &&
			  (m->initial_update == FALSE) &&
			  (m->map_invalid == FALSE)) {
			  m->need_redisplay = FALSE;
			  break;
		  }
		  /* Otherwise, we need to redisplay */
		  m->initial_update = FALSE;
		  m->need_redisplay = TRUE;
		  m->map_invalid = FALSE;
		  m->num_symbols = 0;

		  /* Update the marker location */
		  m->old_marker = m->marker;
		  m->marker.x = grid2map(v->loc->grid_x) + 2;
		  m->marker.y = grid2map(v->loc->grid_y) + 2;

		  left = v->loc->grid_x - (NUM_BOXES + 1) / 2;
		  top = v->loc->grid_y - (NUM_BOXES + 1) / 2;
		  right = left + NUM_BOXES;
		  bottom = top + NUM_BOXES;

		  left_x = grid2map(left);
		  top_y = grid2map(top);

		  /* Find inconsistencies between the maze and the map */
		  for (x = left, line_x = left_x;
			   x <= right;
			   x++, line_x += MAP_BOX_SIZE)
			  for (y = top, line_y = top_y;
				   y <= bottom;
				   y++, line_y += MAP_BOX_SIZE) {
				  b = &real_map[x][y];
				  mb = &m->map[x][y];

				  /* mask off vehicle flags first -ane */
				  flags = (b->flags & MAZE_FLAGS) ^ mb->flags;	/* get flags difference */
				  if (flags & NORTH_WALL && y != top) {
					  /* add symbol to display list */
					  s = &m->symbol[m->num_symbols++];
					  s->type = ((b->flags) & NORTH_DEST) ? NORTH_DEST_SYM :
						NORTH_SYM;
					  s->x = line_x;
					  s->y = line_y;
					  mb->flags ^= NORTH_WALL;
					  mb->flags ^= flags & NORTH_DEST;
				  }
				  if (flags & WEST_WALL && x != left) {
					  s = &m->symbol[m->num_symbols++];
					  s->type = ((b->flags) & WEST_DEST) ? WEST_DEST_SYM : WEST_SYM;
					  s->x = line_x;
					  s->y = line_y;
					  mb->flags ^= WEST_WALL;
					  mb->flags ^= flags & WEST_DEST;
				  }
				  if (b->type != mb->type || b->team != mb->team) {
					  /* Change from one symbol to the next by xoring both
		       on */
					  if (b->type != NORMAL) {
						  s = &m->symbol[m->num_symbols++];
						  s->type = b->type;
						  s->team = b->team;
						  s->x = line_x;
						  s->y = line_y;
					  }
					  if (mb->type != NORMAL) {
						  s = &m->symbol[m->num_symbols++];
						  s->type = mb->type;
						  s->team = mb->team;
						  s->x = line_x;
						  s->y = line_y;
					  }
					  mb->type = b->type;
					  mb->team = b->team;

					  /* Put a symbol in the landmark array so xtank programs can
		       find the landmarks more easily (%%% urgh, this is wrong
		       if just the team changed) */
					  if (m->num_landmarks < MAX_LANDMARKS) {
						  s = &m->landmark[m->num_landmarks++];
						  s->type = mb->type;
						  s->team = mb->team;
						  s->x = x;
						  s->y = y;
					  }
				  }
			  }
		  break;
	  case SP_redisplay:
		  if (!m->need_redisplay)
			  break;

		  /* Redisplay the symbols that have changed */
		  for (i = 0; i < m->num_symbols; i++)
			  draw_symbol(&m->symbol[i]);

		  /* Redisplay the vehicle marker */
		  draw_marker(m->old_marker.x, m->old_marker.y, team_color_bright[v->team]);
		  draw_marker(m->marker.x, m->marker.y, team_color_bright[v->team]);
/*
	for (veh = 0; veh < MAX_VEHICLES; veh++) {
	    mv = &actual_vehicles[veh];
	    m = (Mapper *)(mv->special[(int) MAPPER].record)
	    if ( !tstflag(mv->status, VS_is_alive) ||
             ( !v->have_IFF_key[mv->number] && mv->team != v->team ) ||
	     ( ( mv->special[(int) TACLINK].status != SP_on ||
	     v->special[(int) TACLINK].status != SP_on ) && v != mv ) ||
	     !m->need_redisplay ||
	     mv->special[(int) MAPPER].status != SP_on);
		continue;
	    for (i = 0; i < m->num_symbols; i++)
		draw_symbol(&m->symbol[i]);
	}

        *m = (Mapper *) record;

	if (m->need_redisplay) {
	    draw_marker(m->old_marker.x, m->old_marker.y, team_color_bright[v->team]);
	    draw_marker(m->marker.x, m->marker.y, team_color_bright[v->team]);
	}
*/
		  break;
	  case SP_draw:
	  case SP_erase:
		  /* Draw/erase all the maze symbols that are on the map */
		  draw_full_map(m->map);

		  /* Draw/erase the vehicle marker */
		  draw_marker(m->marker.x, m->marker.y, team_color_bright[v->team]);
		  break;
	  case SP_activate:
		  /* compute initial location of the marker */
		  m->marker.x = grid2map(v->loc->grid_x) + 2;
		  m->marker.y = grid2map(v->loc->grid_y) + 2;

		  m->initial_update = TRUE;
		  m->map_invalid = FALSE;

		  return SP_on;

		  break;
	  case SP_deactivate:
		  break;
	}
}

/*
** Draws all walls and landmarks in specified map onto the map window.
*/
draw_full_map(map)
Box map[GRID_WIDTH][GRID_HEIGHT];
{
	int x, y, line_x, line_y;
	Landmark_info sym;
	Box *b;

	for (x = 0, line_x = MAP_BORDER; x < GRID_WIDTH;
		 x++, line_x += MAP_BOX_SIZE)
		for (y = 0, line_y = MAP_BORDER; y < GRID_HEIGHT;
			 y++, line_y += MAP_BOX_SIZE) {
			b = &map[x][y];
			sym.x = line_x;
			sym.y = line_y;

			if (b->flags & NORTH_WALL) {
				sym.type = (b->flags & NORTH_DEST) ? NORTH_DEST_SYM :
				  NORTH_SYM;
				draw_symbol(&sym);
			}
			if (b->flags & WEST_WALL) {
				sym.type = (b->flags & WEST_DEST) ? WEST_DEST_SYM :
				  WEST_SYM;
				draw_symbol(&sym);
			}
			if (b->type != NORMAL) {
				sym.type = b->type;
				sym.team = b->team;
				draw_symbol(&sym);
			}
		}
}

/*
** XORs the symbol s onto the map.
*/
draw_symbol(s)
Landmark_info *s;
{
	extern Object *landmark_obj[];
	Picture *pic;

	switch (s->type) {
	  case NORTH_SYM:
		  draw_hor(MAP_WIN, s->x, s->y, MAP_BOX_SIZE, DRAW_XOR, WHITE);
		  break;
	  case WEST_SYM:
		  draw_vert(MAP_WIN, s->x, s->y, MAP_BOX_SIZE, DRAW_XOR, WHITE);
		  break;
	  case NORTH_DEST_SYM:
		  draw_hor(MAP_WIN, s->x, s->y, MAP_BOX_SIZE, DRAW_XOR, DEST_WALL);
		  break;
	  case WEST_DEST_SYM:
		  draw_vert(MAP_WIN, s->x, s->y, MAP_BOX_SIZE, DRAW_XOR, DEST_WALL);
		  break;
	  default:
		  pic = &landmark_obj[1]->pic[(int) s->type - 1];
		  draw_picture(MAP_WIN, s->x, s->y, pic, DRAW_XOR,
					   team_color_bright[(int) s->team]);
	}
}

/*
** Displays the entire map. Highlights the location of the terminal.
*/
full_mapper(status)
unsigned int status;
{
	if (status == ON) {
		draw_full_map(real_map);
		draw_grid_marker(term->loc.grid_x, term->loc.grid_y);
	} else if (status == REDISPLAY) {
		if (term->loc.grid_x != term->old_loc.grid_x ||
			term->loc.grid_y != term->old_loc.grid_y) {
			draw_grid_marker(term->old_loc.grid_x, term->old_loc.grid_y);
			draw_grid_marker(term->loc.grid_x, term->loc.grid_y);
		}
	}
}
