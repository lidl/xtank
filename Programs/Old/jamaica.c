#include "/mit/games/src/vax/xtank/Contest/xtanklib.h"
#include <math.h>
#include <stdio.h>
  Vehicle_info *jamaica_closest();

jamaica_main()
{ 
  int fook;
  fook = 1;
  set_rel_speed(6.0);

  while (1) {

    if(fook == 1)
      fook = jamaica_north();
    
    if(fook == 2)
      fook = jamaica_south();

    if(fook == 3)
      fook = jamaica_east();

    if(fook == 4)
      fook = jamaica_west();

    if(fook == 5)
      fook = jamaica_side_decide();

    if(fook == 6)
      fook = jamaica_high_decide();

    if(fook == 7)
      fook = jamaica_suicide();
  }
}



jamaica_north()
{
  int xx;
  Location myloc;
  Vehicle_info *v;
  
  turn_vehicle(3*PI/2);
  
  while (1) {

    get_location(&myloc);
    xx = myloc.box_x - 96;

    if (xx > 10)
      turn_vehicle(3*PI/2 - PI/18.0);

    if (xx < -10)
      turn_vehicle(3*PI/2 + PI/18.0);

    if ((xx < 10) && (xx > -10))
      turn_vehicle(3*PI/2);

    if(wall(NORTH,myloc.grid_x,myloc.grid_y))
       return(5);

    if(speed() == 0)
      jamaica_free();

    v = jamaica_closest();
    if(v != NULL)
      if(jamaica_clear_path(&myloc,&v->loc) == 1)
	return(7);
  }
}

jamaica_south()
{
  int xx;
  Location myloc;
  Vehicle_info *v;

  turn_vehicle(PI/2);

  while (1) {

    get_location(&myloc);
    xx = myloc.box_x - 96;

    if (xx > 10)
      turn_vehicle(PI/2 + PI/18.0);

    if (xx < -10)
      turn_vehicle(PI/2 - PI/18.0);

    if ((xx < 10) && (xx > -10))
      turn_vehicle(PI/2);

    if(wall(SOUTH,myloc.grid_x,myloc.grid_y))
       return(5);

    if(speed() == 0)
      jamaica_free();
    
    v = jamaica_closest();
    if(v != NULL)
      if(jamaica_clear_path(&myloc,&v->loc) == 1)
	return(7);
  }
}


jamaica_east()
{
  int yy;
  Location myloc;
  Vehicle_info *v;

  turn_vehicle(0);

  while (1) {

    get_location(&myloc);
    yy = myloc.box_y - 96;

    if (yy > 10)
      turn_vehicle(0.0 - PI/18.0);

    if (yy < -10)
      turn_vehicle(0.0 + PI/18.0);

    if ((yy < 10) && (yy > -10))
      turn_vehicle(0.0);

    if(wall(EAST,myloc.grid_x,myloc.grid_y))
       return(6);

    if(speed() == 0)
      jamaica_free;
    
    v = jamaica_closest();
    if(v != NULL)
      if(jamaica_clear_path(&myloc,&v->loc) == 1)
	return(7);
  }
}


jamaica_west()
{
  int yy;
  Location myloc;
  Vehicle_info *v;

  turn_vehicle(PI);

  while (1) {

    get_location(&myloc);
    yy = myloc.box_y - 96;

    if (yy > 10)
      turn_vehicle(PI + PI/18.0);

    if (yy < 10)
      turn_vehicle(PI - PI/18.0);

    if ((yy < 10) && (yy > -10))
      turn_vehicle(PI);

    if(wall(WEST,myloc.grid_x,myloc.grid_y))
       return(6);

    if(speed() == 0)
      jamaica_free();

    v = jamaica_closest();
    if(v != NULL)
      if(jamaica_clear_path(&myloc,&v->loc) == 1)
	return(7);
  }      
}

jamaica_side_decide()
{
  Location myloc;
  get_location(&myloc);

  if(wall(WEST,myloc.grid_x,myloc.grid_y) == 0)
    return(4);
  if(wall(EAST,myloc.grid_x,myloc.grid_y) == 0)
    return(3);

  if(wall(NORTH,myloc.grid_x,myloc.grid_y) == 0)
    return(1);
  if(wall(SOUTH,myloc.grid_x,myloc.grid_y) == 0)
    return(2);
}

jamaica_high_decide()
{
  Location myloc;
  get_location(&myloc);

  if(wall(NORTH,myloc.grid_x,myloc.grid_y) == 0)
    return(1);
  if(wall(SOUTH,myloc.grid_x,myloc.grid_y) == 0)
    return(2);

  if(wall(WEST,myloc.grid_x,myloc.grid_y) == 0)
    return(4);
  if(wall(EAST,myloc.grid_x,myloc.grid_y) == 0)
    return(3);

}

jamaica_free()
{
  int framenum = frame_number();

  set_rel_speed(-3.0);

  while(frame_number() < framenum + 10);

  turn_vehicle(angle() + PI);
  set_rel_speed(8.0);
}


Vehicle_info *jamaica_closest()
{
  int num_vehicles;            
  Vehicle_info vehicle[MAX_VEHICLES]; 
  Vehicle_info *v,*closest_v;
  Location my_loc;              
  int i;                      
  int dx,dy;                    
  int dist;
  int min_dist;		
 
  get_vehicles(&num_vehicles,vehicle);

   if(num_vehicles == 0) return(NULL);

  get_location(&my_loc);	
  min_dist = 9999999;		

  closest_v = NULL;

  for (i = 0 ; i < num_vehicles ; i++) {
    v = &vehicle[i];

    dx = BOX_WIDTH * (v->loc.grid_x - my_loc.grid_x)
      + v->loc.box_x - my_loc.box_x;
    dy = BOX_HEIGHT * (v->loc.grid_y - my_loc.grid_y)
      + v->loc.box_y - my_loc.box_y;

    dist = dx*dx + dy*dy;
    if(dist < min_dist) {
      closest_v = v;
      min_dist = dist;
    }
  }

  return(closest_v);
}

jamaica_clear_path(start,finish)
     Location *start, *finish;
{
  int start_x,start_y,finish_x,finish_y;
  int dx,dy,lattice_dx,lattice_dy;
  int tgrid_x,tgrid_y,fgrid_x,fgrid_y;

  /* Compute absolute x coordinate in maze */
  start_x = start->grid_x * BOX_WIDTH + start->box_x;
  start_y = start->grid_y * BOX_HEIGHT + start->box_y;
  finish_x = finish->grid_x * BOX_WIDTH + finish->box_x;
  finish_y = finish->grid_y * BOX_HEIGHT + finish->box_y;

  /* Computed x and y differences from start to finish */
  dx = finish_x - start_x;
  dy = finish_y - start_y;

  /* Set up temporary and final box coordinates */
  tgrid_x = start->grid_x;
  tgrid_y = start->grid_y;
  fgrid_x = finish->grid_x;
  fgrid_y = finish->grid_y;

  /* Figure out the general direction that the line is travelling in
  ** so that we can write specific code for each case.
  **
  ** In the NE, SE, NW, and SW cases, 
  ** lattice_dx and lattice_dy are the deltas from the starting
  ** location to the lattice point that the path is heading towards.
  ** The slope of the line is compared to the slope to the lattice point
  ** This determines which wall the path intersects.
  ** Instead of comparing dx/dy with lattice_dx/lattice_dy, I multiply
  ** both sides by dy * lattice_dy, which lets me do 2 multiplies instead
  ** of 2 divides.
  */
  if(fgrid_x > tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northeast */
      lattice_dx = (tgrid_x + 1)*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(EAST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x++;
	  lattice_dx += BOX_WIDTH;
	}
      }
    }
    else {			/* East */
      for(; tgrid_x < fgrid_x ; tgrid_x++)
	if(wall(EAST,tgrid_x,tgrid_y)) return(0);
    }

  else if(fgrid_x < tgrid_x)
    if(fgrid_y > tgrid_y) {	/* Southwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = (tgrid_y + 1)*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx > dy * lattice_dx) {
	  if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y++;
	  lattice_dy += BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	}
      }
    }
    else if(fgrid_y < tgrid_y) { /* Northwest */
      lattice_dx = tgrid_x*BOX_WIDTH - start_x;
      lattice_dy = tgrid_y*BOX_HEIGHT - start_y;
      while(tgrid_x != fgrid_x || tgrid_y != fgrid_y) {
	if(lattice_dy * dx < dy * lattice_dx) {
	  if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
	  tgrid_y--;
	  lattice_dy -= BOX_HEIGHT;
	}
	else {
	  if(wall(WEST,tgrid_x,tgrid_y)) return(0);
	  tgrid_x--;
	  lattice_dx -= BOX_WIDTH;
	  }
      }
    }
    else {			/* West */
      for(; tgrid_x > fgrid_x ; tgrid_x--)
	if(wall(WEST,tgrid_x,tgrid_y)) return(0);
    }

  else
    if(fgrid_y > tgrid_y) {	/* South */
      for(; tgrid_y < fgrid_y ; tgrid_y++)
	if(wall(SOUTH,tgrid_x,tgrid_y)) return(0);
    }
    else if(fgrid_y < tgrid_y) { /* North */
      for(; tgrid_y > fgrid_y ; tgrid_y--)
	if(wall(NORTH,tgrid_x,tgrid_y)) return(0);
    }
  return(1);
}

jamaica_suicide()
{
  Vehicle_info *him;
  Location myloc;
  int i,dx,dy;
  float ang;

  for(i = 0 ; i < 20 ; i++) {

    him = jamaica_closest();
    if(him == NULL) return(1);

    get_location(&myloc);

    dx = BOX_WIDTH * (him->loc.grid_x - myloc.grid_x)
      + him->loc.box_x - myloc.box_x;
    dy = BOX_HEIGHT * (him->loc.grid_y - myloc.grid_y)
      + him->loc.box_y - myloc.box_y;

    turn_all_turrets(dx,dy);
    if ((dx*dx + dy*dy) < 70000)
      fire_all_weapons();
    ang = atan2((float) dy,(float) dx);
    turn_vehicle(ang + PI/3.5);
    set_rel_speed(9.0);

    if (speed() == 0.0) 
      jamaica_free();

  }
  
  return(1);
  
}


