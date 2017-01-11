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

#include <string.h>

#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "disc.h"
#include "sysdep.h"
#include "lowlib.h"
#include "bullet.h"
#include "terminal.h"
#include "vehicle.h"
#include "proto.h"

extern Terminal *term;
extern Settings settings;
extern Video *vid;

/*
 * Non-linear (x*x) speed computation, gives greater control over vehicle
 * at slower speeds.
 */
#define SQR(x) ((x)*(x))
#define MAX_ABS_SPEED 9.0
#define SENSITIVITY_FACTOR 50.0 /* `Feel-good' value */
#define PRECISION 0.1 /* Needed to allow us to `stop' */
static FLOAT
compute_mouse_speed(int ix, int iy)
{
  FLOAT temp = ((term->mouse_speed * (SQR(ix) + SQR(iy))
		 * MAX_ABS_SPEED * SENSITIVITY_FACTOR)
		/ (SQR(ANIM_WIN_WIDTH) + SQR(ANIM_WIN_HEIGHT)));
  return ( temp > PRECISION ? temp : (FLOAT) 0.0);
}	

/*
 * Primative Keymaping function;
 * Search through 'MAP' (string of pairs of chars) looking for 'KEY', if we
 * find 'KEY' then return the next char; else return KEY if we don't.
 */

static char
lookup_key(char key)
{
  char k = '\0';
  char *map = term->keymap;

  if (map)
    while ((k = *map) != '\0')
      {
	if (k == key)
	  {
	    key=map[1];
	    break;
	  }
	else
	  map += 2;
      }
  return key;
}


#define MAX_EVENTS 100

/*
** Gets input events if there are any, dispatching them to the proper
** handler according to the window in which they occur.
** Returns one of GAME_RUNNING, GAME_QUIT.
*/
int
get_input(void)
{
	extern int anim_input(), message_input(), map_input();
	static int (*input_handler[MAX_WINDOWS]) () =
	{
		anim_input, message_input, NULL, map_input, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL};
	int (*handler) ();
	int num_events;
	Event event[MAX_EVENTS];
	int i;

	num_events = MAX_EVENTS;
	get_events(&num_events, event);

	for (i = 0; i < num_events; i++) {
		handler = input_handler[event[i].win];
		if (handler != NULL)
			if ((*handler) (&event[i]) == GAME_QUIT)
				return GAME_QUIT;
	}

	return GAME_RUNNING;
}

/*
** Handler for input events to animation window.
** Returns one of GAME_RUNNING, GAME_QUIT.
*/
int
anim_input(Event *event)
{
	extern int sync_rate;
	Vehicle *v;
	int dx, dy;
	int num;

	/* Compute dx and dy from center of animation window to cursor */
	dx = event->x - ANIM_WIN_WIDTH / 2;
	dy = event->y - ANIM_WIN_HEIGHT / 2;
	if (term->status & TS_3d)
		transform_3d(&dx, &dy);

	if (term->observer) {		/* snoop around */
		switch (event->type) {
		case EVENT_RBUTTONUP:
		  if (term->mouse_drive != 0)
		    term->mouse_drive_active ^= TRUE;
		  break;
		case EVENT_RBUTTON:
		  if (term->mouse_drive != 0)
		    term->mouse_drive_active ^= TRUE;
		  else
		    move_view(dx, dy);
		  break;
		case EVENT_MOVED:
		  if (!(term->mouse_drive_active))
		    break;
		case EVENT_LBUTTON:
		case EVENT_MBUTTON:
		          move_view(dx, dy);
			  break;
		case EVENT_KEY:
			  switch (lookup_key(event->key)) {
				case '-':
					num = previous_live_tank();
					if (num >= 0) {
						switch_view(num);
					}
					break;
				case '+':
					num = next_live_tank();
					if (num >= 0) {
						switch_view(num);
					}
					break;
				case 'B':  /* toggle beeps (HAK 2/93) */
					if (vid->beep_flag = !vid->beep_flag)
						beep_window();
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					switch_view(event->key - '0');
					break;
				case ' ':
					pause_game(TRUE);
					break;
				case '\t':
					if (term->mouse_drive != 0)
					  term->mouse_drive_active ^= TRUE;
					break;
				case 'w':
					map_battle_windows();
					break;
				case 'W':
					unmap_battle_windows();
					break;
				case '<':
					set_game_speed(settings.game_speed - 1);
					break;
				case '>':
					set_game_speed(settings.game_speed + 1);
					break;
				case 'Q':
					return GAME_QUIT;
			  }
		}
	} else if (term->vehicle != NULL) {	/* control a vehicle */
		/* Let xtanklib functions know what vehicle we're talking about */
		if ((v = term->vehicle) == (Vehicle *) NULL)
			return GAME_RUNNING;
		set_current_vehicle(v);
		switch (event->type) {
		  case EVENT_LBUTTON:
			  aim_all_turrets(dx, dy);
			  if (term->mouse_heat != 0)
			    {
			      if (term->mouse_heat > 0)
				fire_hot_weapons(term->mouse_heat);
			      else
				fire_cool_weapons(-(term->mouse_heat));
			    }
			  else
			    fire_all_weapons();
			  set_message_data(v, event);
			  break;
		  case EVENT_MBUTTON:
			  aim_all_turrets(dx, dy);
			  set_message_data(v, event);
			  break;
		  case EVENT_RBUTTON:
			  turn_vehicle_human(ATAN2(dy, dx));
			  set_message_data(v, event);
			  if (term->mouse_speed)
			    set_rel_drive(compute_mouse_speed(dx, dy));
			  if (term->mouse_drive != 0)
			    term->mouse_drive_active ^= TRUE;
			  break;
		   case EVENT_RBUTTONUP:
			  if (term->mouse_drive != 0)
			    term->mouse_drive_active ^= TRUE;
			  break;
		   case EVENT_MOVED:
			  if (term->mouse_drive_active)
			    {
			      turn_vehicle_human(ATAN2(dy, dx));
			      if (term->mouse_speed != 0)
				set_rel_drive(compute_mouse_speed(dx, dy));
			    }
			  break;
		   case EVENT_KEY:
			  switch (lookup_key(event->key)) {
				case 'Q':
					return GAME_QUIT;

#ifndef LOCK_GAME_CONTROLS
				case 'P':
					pause_game(TRUE);
					break;
#endif

				case 'B':
					if (vid->beep_flag = !vid->beep_flag)
						beep_window();
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
#ifdef KEYPAD_DETECT
					/*
					 * If keypad is != then decode
					 * numeric keypad key as a weapon
					 * toggling/firing command.
					 */
					if (event->keypad)
					  {
					    int weapon = event->key - '1';
					    /*
					     * If keypad is < 0 then use
					     * the 1.3[df] style keypad
					     * control method - numeric
					     * keys toggle on/off weapons,
					     * KP_0 fires all weapons (with
					     * appropriate toggling.
					     */
					    if (event->keypad < 0)
					      {
						if (weapon >=0)
						  {
						    toggle_weapon(weapon);
						  }
						else
						  {
						    int wn;
						    for (wn = 0;
							 wn < MAX_WEAPONS;
							 wn++)
						      {
							switch(weapon_on(wn)) {
							case TRUE:
							  anim_fire_weapon(wn);
							  break;
							case FALSE:
							  turn_on_weapon(wn);
							  anim_fire_weapon(wn);
							  turn_off_weapon(wn);
							  break;
							}
						      }
						  }
					      }
					    /*
					     * Use the `new' kepad decoding,
					     * which fires the
					     * corresponding weapon.
					     */
					    else
					      {
						switch(weapon_on(weapon)) {
						case TRUE:
						  anim_fire_weapon(weapon);
						  break;
						case FALSE:
						  turn_on_weapon(weapon);
						  anim_fire_weapon(weapon);
						  turn_off_weapon(weapon);
						  break;
						}
					      }
					} else {
#endif
						set_rel_drive((FLOAT) (event->key - '0'));
#ifdef KEYPAD_DETECT
					}
#endif
					break;

				case '-':
					if (term->mouse_speed)
						term->mouse_speed = -term->mouse_speed;
					else
						set_rel_drive(-9.0);
					break;
				case 'x':
					set_abs_drive(v->vector.drive - 1);
					break;
				case 'c':
					set_abs_drive(0.0);
					break;
				case 'v':
					set_abs_drive(v->vector.drive + 1);
					break;

				case ' ':
					if (term->mouse_heat != 0)
					  {
					    if (term->mouse_heat > 0)
					      fire_cool_weapons((term->mouse_heat));
					    else
					      fire_hot_weapons(-(term->mouse_heat));
					  }
					else
					  fire_all_weapons();
					break;
				case '!':
					toggle_weapon(0);
					break;
				case '@':
					toggle_weapon(1);
					break;
				case '#':
					toggle_weapon(2);
					break;
				case '$':
					toggle_weapon(3);
					break;
				case '%':
					toggle_weapon(4);
					break;
				case '^':
					toggle_weapon(5);
					break;
				case '=':
				case 'a':
					{
						int wn;

						for (wn = 0; wn < MAX_WEAPONS; ++wn)
							toggle_weapon(wn);
					}
					break;

				case 's':
					set_disc_orbit(v, COUNTERCLOCKWISE);
					break;
				case 'd':
					set_disc_orbit(v, TOGGLE);
					break;
				case 'f':
					set_disc_orbit(v, CLOCKWISE);
					break;
				case 'w':
					release_discs(v, DISC_SLOW_SPEED, TRUE);
					break;
				case 'e':
					release_discs(v, DISC_MED_SPEED, TRUE);
					break;
				case 'r':
					release_discs(v, DISC_FAST_SPEED, TRUE);
					break;
#ifndef NO_CAMO
				case 'I':		/* for Invisible */
					do_special(v, CAMO, SP_toggle);
					break;
#endif /* NO_CAMO */
#ifndef NO_HUD
				case 'H':		/* for HUD */
					do_special(v, HUD, SP_toggle);
					break;
#endif /* !NO_HUD */
				case 'C':
					do_special(v, CONSOLE, SP_toggle);
					break;
				case 'M':
					do_special(v, MAPPER, SP_toggle);
					break;
				case 'N':
					do_special(v, NEW_RADAR, SP_toggle);
					break;
				case 'T':
					do_special(v, TACLINK, SP_toggle);
					break;
				case 'R':
					do_special(v, RADAR, SP_toggle);
					break;
				case '+':
					do_special(v, REPAIR, SP_toggle);
					break;

#ifndef LOCK_GAME_CONTROLS
				case '<':
					set_game_speed(settings.game_speed - 1);
					break;
				case '>':
					set_game_speed(settings.game_speed + 1);
					break;

				case 'i':
					sync_rate = 1;
					break;
				case 'o':
					sync_rate = 2;
					break;
				case 'p':
					sync_rate = 4;
					break;
				case '[':
					sync_rate = 8;
					break;
				case ']':
					sync_rate = 16;
					break;
#endif
				case 'W':
					toggle_3d(TS_wide);
					break;
				case 'D':
					toggle_3d(TS_long);
					break;
				case 'E':
					toggle_3d(TS_extend);
					break;
				case 'L':
					toggle_3d(TS_clip);
					break;
				case 'F':
					v->teleport ^= TRUE;
					break;

				case 'z':
					if (v->vdesc->treads != HOVER_TREAD)
						v->safety ^= TRUE;
					break;
				case '\r':
					send_message(v);
					break;
				case '\t':
					if (term->mouse_drive != 0)
					  term->mouse_drive_active ^= TRUE;
					break;
				case 't':
					aim_all_turrets(dx, dy);
					break;
				case 'g':
					turn_vehicle(ATAN2(dy, dx));
					break;
				case 'V':
					term->teleview = ~term->teleview;
					break;
				case ',':
					turn_tow(v,-1.0);
					break;
				case '.':
					turn_tow(v,1.0);
					break;
				case '/':
					det_tow(v);
					break;

				default:
					beep_window();
			  }
			  break;
		}
	}
	return GAME_RUNNING;
}

/*
** Returns next character typed.  Blocks until character entered.
** Clicking any button returns a '\r'.
*/
int
get_reply(void)
{
	int num_events;
	Event event;

	for (;;) {
		num_events = 1;
		get_events(&num_events, &event);
		if (num_events > 0)
			switch (event.type) {
			  case EVENT_KEY:
				  return (event.key);
			  case EVENT_LBUTTON:
			  case EVENT_MBUTTON:
			  case EVENT_RBUTTON:
				  return '\r';
			}
		else
			if(vid->win[ANIM_WIN].flags | WIN_exposed)
				return -1;  /* window needs refresh */
	}
}

/*
** Waits for next key or button press.
*/
void
wait_input(void)
{
	int num_events;
	Event event;

	for (;;) {
		num_events = 1;
		get_events(&num_events, &event);
		if (num_events > 0 &&
			(event.type == EVENT_KEY || event.type == EVENT_LBUTTON ||
			 event.type == EVENT_MBUTTON || event.type == EVENT_RBUTTON))
			break;
	}
}

/*
** Returns character typed if any.  Returns NULL if no character typed.
*/
int
scan_input(void)
{
	int num_events;
	Event event;

	num_events = 1;
	get_events(&num_events, &event);
	if (num_events > 0 &&
		(event.type == EVENT_KEY || event.type == EVENT_LBUTTON ||
		 event.type == EVENT_MBUTTON || event.type == EVENT_RBUTTON))
		return TRUE;
	return FALSE;
}

/*
** Inputs an integer from the user.
*/
int
input_int(int w, char *input_str, int col, int row, int deflt, int mn, int mx, int font)
{
	Word disp;
	char next_char, response[12], disp_response[80], temp[80];
	int value, max_length, rlen;

	(void) sprintf(temp, "%s (%d-%d)[%d]:", input_str, mn, mx, deflt);
/*
	display_mesg2(w, temp, col, row, font);
	sync_output(FALSE);
*/

	max_length = 11;
	value = 0;
	response[0] = '\0';

	disp.x = col + 1 + strlen(temp);
	disp.y = row;
	disp.len = max_length + 1;
	disp.str = disp_response;

	while (1) {
/*
		(void) sprintf(temp, "%s (%d-%d)[%d]:", input_str, mn, mx, deflt);
*/
		display_mesg2(w, temp, col, row, font); /* redraw the prompt */
		sync_output(FALSE);

		(void) sprintf(disp_response, "%s_ ", response);
		draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
		sync_output(FALSE);
		next_char = get_reply();
		if (next_char == -1) continue; /* window exposed, redraw */
		switch (next_char) {
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
		  case '0':
			  if (strlen(response) < (unsigned int) max_length) {
				  value *= 10;
				  value += next_char - '0';
				  rlen = strlen(response);
				  *(response + rlen) = next_char;
				  *(response + rlen + 1) = '\0';
			  }
			  break;
		  case 0x08:
		  case 0x7f:
			  rlen = strlen(response);
			  if (rlen > 0) {
				  value /= 10;
				  *(response + rlen - 1) = '\0';
			  }
			  break;
		  case 'm':
		  case 'M':
			  value <<= 10;		/* * 1024 */
			  /* Missing Break Important */
		  case 'k':
		  case 'K':
			  value <<= 10;		/* * 1024 */
			  (void) sprintf(response, "%d", value);
			  /* Missing Break Important */
		  case '\r':
			  (void) sprintf(disp_response, "%s ", response);
			  draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
			  sync_output(FALSE);
			  if (strlen(response) == 0 || value < mn || value > mx)
				  return (deflt);
			  else
				  return (value);
		}
	}
}

/*
** Inputs a string from the user.
*/
void
input_string(int w, char *prompt, char *response, int col, int row, int font, int max_length)
{
	Word disp;
	char next_char, disp_response[300];
	int rlen;
/*
	display_mesg2(w, prompt, col, row, font);
	sync_output(FALSE);
*/
	response[0] = '\0';

	disp.x = col + 1 + strlen(prompt);
	disp.y = row;
	disp.len = max_length + 1;
	disp.str = disp_response;

	while (1) {
		display_mesg2(w, prompt, col, row, font);
		sync_output(FALSE);
		(void) strcpy(disp_response, response);
		(void) strcat(disp_response, "_ ");
		draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
		sync_output(FALSE);
		next_char = get_reply();
		if(next_char == -1) continue;
		switch (next_char) {
		  case 0x08:
		  case 0x7f:
			  rlen = strlen(response);
			  if (rlen > 0) {
				  *(response + rlen - 1) = '\0';
			  }
			  break;
		  case '\r':
			  (void) sprintf(disp_response, "%s ", response);
			  draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
			  sync_output(FALSE);
			  return;
		  default:
			  if (strlen(response) < (unsigned int) max_length) {
				  rlen = strlen(response);
				  *(response + rlen) = next_char;
				  *(response + rlen + 1) = '\0';
			  }
		}
	}
}

/*
** Prompts the user to confirm a given action.
*/
int
confirm(int w, char *disp, int col, int row, int font)
{
	char prompt[80], reply;

	do {
		(void) strcpy(prompt, "Are you sure you want to ");
		(void) strcat(prompt, disp);
		(void) strcat(prompt, " (y/n)?");
		display_mesg2(w, prompt, col, row, font);
		reply = get_reply();
	} while(reply == -1);

	if (reply == 'y' || reply == 'Y')
		return TRUE;
	else
		return FALSE;
}
