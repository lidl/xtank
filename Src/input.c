/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** input.c
*/

#include "xtank.h"
#include "gr.h"
#include "disc.h"

#define MAX_EVENTS 100

/*
** Gets input events if there are any, dispatching them to the proper
** handler according to the window in which they occur.
** Returns one of GAME_RUNNING, GAME_QUIT.
*/
get_input()
{
  extern int anim_input(),game_input(),map_input();
  static int (*input_handler[MAX_WINDOWS])() = {
    anim_input, game_input, NULL, map_input, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL };
  int (*handler)();
  int num_events;
  Event event[MAX_EVENTS];
  int i;

  num_events = MAX_EVENTS;
  get_events(&num_events,event);

  for(i = 0 ; i < num_events ; i++) {
    handler = input_handler[event[i].win];
    if(handler != NULL)
      if((*handler)(&event[i]) == GAME_QUIT) return GAME_QUIT;
  }

  return GAME_RUNNING;
}

/*
** Handler for input events to animation window.
** Returns one of GAME_RUNNING, GAME_QUIT.
*/
anim_input(event)
     Event *event;
{
  extern int sync_rate;
  Vehicle *v;
  int dx,dy;

  /* Compute dx and dy from center of animation window to cursor */
  dx = event->x - ANIM_WIN_WIDTH/2;
  dy = event->y - ANIM_WIN_HEIGHT/2;
  if(term->status & TS_3d) transform_3d(&dx,&dy);

  /* Execute the proper command, depending on the mode */
  switch(settings.mode) {
    case DEMO_MODE:
    case BATTLE_MODE:
      switch(event->type) {
        case EVENT_LBUTTON:
        case EVENT_MBUTTON:
        case EVENT_RBUTTON:
	  move_view(dx,dy);
	  break;
	case EVENT_KEY:
	  switch(event->key) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8':
	    case '9': switch_view(event->key - '0'); break;
	    case ' ': pause_game(TRUE); break;
	    case 'w': map_battle_windows(); break;
	    case 'W': unmap_battle_windows(); break;
	    case 'Q': return GAME_QUIT;
	    }
	}
      break;
    case SINGLE_MODE:
    case MULTI_MODE:
      /* Let xtanklib functions know what vehicle we're talking about */
      v = term->vehicle;
      if(v == (Vehicle *) NULL) return GAME_RUNNING;
      set_current_vehicle(v);

      switch(event->type) {
        case EVENT_LBUTTON:
	  aim_all_turrets(dx,dy);
	  fire_all_weapons();
	  set_message_data(v,event);
	  break;
        case EVENT_MBUTTON:
	  aim_all_turrets(dx,dy);
	  set_message_data(v,event);
	  break;
        case EVENT_RBUTTON:
	  turn_vehicle(atan2((float) dy,(float) dx));
	  set_message_data(v,event);
	  break;
	case EVENT_KEY:
	  switch(event->key) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8':
	    case '9': set_rel_drive((float) (event->key - '0')); break;
	    case '-': set_rel_drive(-9.0); break;
	    case ' ': fire_all_weapons(); break;

	    case '!': weapon_on(0)?turn_off_weapon(0):turn_on_weapon(0); break;
	    case '@': weapon_on(1)?turn_off_weapon(1):turn_on_weapon(1); break;
	    case '#': weapon_on(2)?turn_off_weapon(2):turn_on_weapon(2); break;
	    case '$': weapon_on(3)?turn_off_weapon(3):turn_on_weapon(3); break;
	    case '%': weapon_on(4)?turn_off_weapon(4):turn_on_weapon(4); break;
	    case '^': weapon_on(5)?turn_off_weapon(5):turn_on_weapon(5); break;

	    case 't': aim_all_turrets(dx,dy); break;
	    case 'g': turn_vehicle(atan2((float) dy,(float) dx)); break;

	    case 's': set_disc_orbit(v,COUNTERCLOCKWISE); break;
	    case 'd': set_disc_orbit(v,TOGGLE); break;
	    case 'f': set_disc_orbit(v,CLOCKWISE); break;

	    case 'w': release_discs(v,DISC_SLOW_SPEED,TRUE); break;
	    case 'e': release_discs(v,DISC_MED_SPEED,TRUE); break;
	    case 'r': release_discs(v,DISC_FAST_SPEED,TRUE); break;

	    case 'z': v->safety ^= TRUE; break;
	    case 'x': set_abs_drive(v->vector.desired_drive - 1.0); break;
	    case 'c': set_abs_drive(0.0); break;
	    case 'v': set_abs_drive(v->vector.desired_drive + 1.0); break;

	    case 'C': do_special(v,CONSOLE,SP_toggle); break;
	    case 'M': do_special(v,MAPPER,SP_toggle); break;
	    case 'R': do_special(v,RADAR,SP_toggle); break;

	    case 'Q': return GAME_QUIT;
	    case 'P': pause_game(TRUE); break;
	    case '<': set_game_speed(settings.game_speed - 1); break;
	    case '>': set_game_speed(settings.game_speed + 1); break;
	    case '\r': send_message(v); break;

	    case 'T': toggle_3d(TS_3d); expose_win(ANIM_WIN,TRUE); break;
	    case 'W': toggle_3d(TS_wide); break;
	    case 'D': toggle_3d(TS_long); break;
	    case 'E': toggle_3d(TS_extend); break;
	    case 'L': toggle_3d(TS_clip); break;

	    case 'i': sync_rate = 1; break;
	    case 'o': sync_rate = 2; break;
	    case 'p': sync_rate = 4; break;
	    case '[': sync_rate = 8; break;
	    case ']': sync_rate = 16; break;
	    }
	  break;
	}
      break;
    }
  return GAME_RUNNING;
}

/*
** Returns next character typed.  Blocks until character entered.
** Clicking any button returns a '\r'.
*/
char get_reply()
{
  int num_events;
  Event event;
  
  for(;;) {
    num_events = 1;
    get_events(&num_events,&event);
    if(num_events > 0)
      switch(event.type) {
	case EVENT_KEY: return (event.key);
	case EVENT_LBUTTON:
	case EVENT_MBUTTON:
	case EVENT_RBUTTON: return '\r';
      }
  }
}

/*
** Waits for next key or button press.
*/
wait_input()
{
  int num_events;
  Event event;
  
  for(;;) {
    num_events = 1;
    get_events(&num_events,&event);
    if(num_events > 0 &&
       (event.type == EVENT_KEY || event.type == EVENT_LBUTTON ||
	event.type == EVENT_MBUTTON || event.type == EVENT_RBUTTON))
      break;
  }
}

/*
** Returns character typed if any.  Returns NULL if no character typed.
*/
scan_input()
{
  int num_events;
  Event event;
  
  num_events = 1;
  get_events(&num_events,&event);
  if(num_events > 0 &&
     (event.type == EVENT_KEY || event.type == EVENT_LBUTTON ||
      event.type == EVENT_MBUTTON || event.type == EVENT_RBUTTON))
    return TRUE;
  return FALSE;
}

/*
** Inputs an integer from the user.
*/
input_int(w,input_str,col,row,deflt,mn,mx,font)
     int w;
     char *input_str;
     int col,row,deflt,mn,mx,font;
{
  Word disp;
  char next_char,response[12],disp_response[80],temp[80];
  int value,max_length,rlen;

  (void) sprintf(temp,"%s (%d-%d)[%d]:",input_str,mn,mx,deflt);
  display_mesg2(w,temp,col,row,font);
  sync_output(FALSE);

  max_length = 11;
  value = 0;
  response[0] = '\0';

  disp.x = col+1 + strlen(temp);
  disp.y = row;
  disp.len = max_length+1;
  disp.str = disp_response;

  while(1) {
    (void) sprintf(disp_response,"%s_ ",response);
    draw_text_rc(w,disp.x,disp.y,disp.str,font,WHITE);
    sync_output(FALSE);
    next_char = get_reply();
    switch(next_char) {
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9': case '0':
	if(strlen(response) < max_length) {
	  value *= 10;
	  value += next_char - '0';
	  rlen = strlen(response);
	  *(response+rlen) = next_char;
	  *(response+rlen+1) = '\0';
	}
	break;
      case 127:
	rlen = strlen(response);
	if(rlen > 0) {
	  value /= 10;
	  *(response+rlen-1) = '\0';
	}
	break;
      case '\r':
	(void) sprintf(disp_response,"%s ",response);
	draw_text_rc(w,disp.x,disp.y,disp.str,font,WHITE);
	sync_output(FALSE);
	if(strlen(response) == 0 || value < mn || value > mx)
	  return(deflt);
	else
	  return(value);
      }
  }
}

/*
** Inputs a string from the user.
*/
input_string(w,prompt,response,col,row,font)
     int w;
     char *prompt,*response;
     int col,row,font;
{
  Word disp;
  char next_char,disp_response[300];
  int max_length,rlen;

  display_mesg2(w,prompt,col,row,font);
  sync_output(FALSE);

  max_length = 256;
  response[0] = '\0';

  disp.x = col+1 + strlen(prompt);
  disp.y = row;
  disp.len = max_length+1;
  disp.str = disp_response;

  while(1) {
    (void) strcpy(disp_response,response);
    (void) strcat(disp_response,"_ ");
    draw_text_rc(w,disp.x,disp.y,disp.str,font,WHITE);
    sync_output(FALSE);
    next_char = get_reply();
    switch(next_char) {
      case 127:
	rlen = strlen(response);
	if(rlen > 0) {
	  *(response+rlen-1) = '\0';
	}
	break;
      case '\r':
	(void) sprintf(disp_response,"%s ",response);
	draw_text_rc(w,disp.x,disp.y,disp.str,font,WHITE);
	sync_output(FALSE);
	return;
      default:
	if(strlen(response) < max_length) {
	  rlen = strlen(response);
	  *(response+rlen) = next_char;
	  *(response+rlen+1) = '\0';
	}
      }
  }
}

/*
** Prompts the user to confirm a given action.
*/
confirm(w,disp,col,row,font)
     int w;
     char *disp;
     int col,row,font;
{
  char prompt[80],reply;
  
  (void) strcpy(prompt,"Are you sure you want to ");
  (void) strcat(prompt,disp);
  (void) strcat(prompt," (y/n)?");
  display_mesg2(w,prompt,col,row,font);
  reply = get_reply();

  if (reply == 'y' || reply == 'Y') return TRUE;
  else return FALSE;
}
