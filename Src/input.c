/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** input.c
*/

/*
$Author: lidl $
$Id: input.c,v 2.18 1992/09/13 07:01:59 lidl Exp $

$Log: input.c,v $
 * Revision 2.18  1992/09/13  07:01:59  lidl
 * removed NO_NEW_RADAR ifdefs
 *
 * Revision 2.17  1992/06/07  02:45:08  lidl
 * Post Adam Bryant patches and a manual merge of the rejects (ugh!)
 *
 * Revision 2.16  1992/03/31  21:45:50  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.15  1992/03/31  04:04:16  lidl
 * pre-aaron patches, post 1.3d release (ie mailing list patches)
 *
 * Revision 2.14  1992/01/29  08:37:01  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.13  1992/01/06  07:52:49  stripes
 * Changes for teleport
 *
 * Revision 2.12  1991/12/15  21:38:30  aahz
 * fixed typo
 *
 * Revision 2.11  1991/12/15  21:10:07  aahz
 * improved previous and next live_tank calls.
 *
 * Revision 2.10  1991/12/10  03:41:44  lidl
 * changed float to FLOAT, for portability reasons
 *
 * Revision 2.9  1991/11/22  07:01:20  stripes
 * (AAHZ) - added + and - to the view mode for incr/decr veh #'s
 *
 * Revision 2.8  1991/09/28  19:22:35  aahz
 * no change.
 *
 * Revision 2.7  1991/09/26  07:14:42  lbruck
 * added the ability to press the 'M'eg key in input_int().
 *
 * Revision 2.6  1991/09/23  06:07:40  lidl
 * hopefully fixes bug where you hit '9' and start going backwards
 *
 * Revision 2.5  1991/09/19  05:32:26  lidl
 * run through indent, added LOCK_GAME_CONTROLS & KEYPAD_DETECT ifdefs
 *
 * Revision 2.4  1991/09/17  16:59:46  lidl
 * caught a un-checked in change from rpotter
 *
 * Revision 2.3  91/02/10  13:50:49  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:04  rpotter
 * complete rewrite of vehicle death, other tweaks
 *
 * Revision 2.1  91/01/17  07:11:50  rpotter
 * lint warnings and a fix to update_vector()
 *
 * Revision 2.0  91/01/17  02:09:41  rpotter
 * small changes
 *
 * Revision 1.1  90/12/29  21:02:33  aahz
 * Initial revision
 *
*/

#include "malloc.h"
#include "xtank.h"
#include "screen.h"
#include "graphics.h"
#include "gr.h"
#include "disc.h"
#include "sysdep.h"
#include "vehicle.h"
#include "terminal.h"
#include "lowlib.h"


extern Terminal *term;
extern Settings settings;


#define MAX_EVENTS 100

/*
** Gets input events if there are any, dispatching them to the proper
** handler according to the window in which they occur.
** Returns one of GAME_RUNNING, GAME_QUIT.
*/
get_input()
{
    extern int anim_input(), message_input(), map_input();
    static int (*input_handler[MAX_WINDOWS]) () = {
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
anim_input(event)
Event *event;
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

    if (term->observer) {	/* snoop around */
	switch (event->type) {
	    case EVENT_LBUTTON:
	    case EVENT_MBUTTON:
	    case EVENT_RBUTTON:
		move_view(dx, dy);
		break;
	    case EVENT_KEY:
		switch (event->key) {
			case '-':
                num = previous_live_tank();
                if (num >= 0)
                {
                    switch_view(num);
                }
                break;
			case '+':
                num = next_live_tank();
                if (num >= 0)
                {
                    switch_view(num);
                }
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
		fire_all_weapons();
		set_message_data(v, event);
		break;
	    case EVENT_MBUTTON:
		aim_all_turrets(dx, dy);
		set_message_data(v, event);
		if (v -> mouse_speed)
		  set_rel_drive(v -> mouse_speed *
				(float) (50 * (dx * dx + dy * dy) * 9 /
					 (ANIM_WIN_WIDTH * ANIM_WIN_WIDTH +
					  ANIM_WIN_HEIGHT * ANIM_WIN_HEIGHT)));
		break;
	    case EVENT_RBUTTON:
		turn_vehicle_human(ATAN2(dy, dx));
		set_message_data(v, event);
		if (v -> mouse_speed) {
			set_rel_drive((float) v -> mouse_speed *
				(50 * (dx * dx + dy * dy) * 9 /
					 (ANIM_WIN_WIDTH * ANIM_WIN_WIDTH +
					  ANIM_WIN_HEIGHT * ANIM_WIN_HEIGHT)));
		}
		break;
	    case EVENT_KEY:
		switch (event->key) {
		    case 'Q':
			return GAME_QUIT;

#ifndef LOCK_GAME_CONTROLS
		    case 'P':
			pause_game(TRUE);
			break;
#endif

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
			if (event->keypad) {
			    int weapon, status;

			    weapon = event->key - '1';
			    if ((status = weapon_on(weapon)) == TRUE) {
				fire_weapon(weapon);
			    } else if (status == FALSE) {
				turn_on_weapon(weapon);
				fire_weapon(weapon);
				turn_off_weapon(weapon);
			    }
			} else {
#endif
			    set_rel_drive((FLOAT) (event->key - '0'));
#ifdef KEYPAD_DETECT
			}
#endif
			break;

		    case '-':
			if (v -> mouse_speed)
			  v -> mouse_speed = -v -> mouse_speed;
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
		    case 'I': /* for Invisible */
			do_special(v, CAMO, SP_toggle);
			break;
#endif /* NO_CAMO */
#ifndef NO_HUD
		    case 'H': /* for HUD */
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
			if (v->vdesc->treads != HOVER_TREAD) v->safety ^= TRUE;
			break;
		    case '\r':
			send_message(v);
			break;
		    case 't':
			aim_all_turrets(dx, dy);
			break;
		    case 'g':
			turn_vehicle(ATAN2(dy, dx));
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
get_reply()
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
    }
}

/*
** Waits for next key or button press.
*/
wait_input()
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
scan_input()
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
input_int(w, input_str, col, row, deflt, mn, mx, font)
int w;
char *input_str;
int col, row, deflt, mn, mx, font;
{
    Word disp;
    char next_char, response[12], disp_response[80], temp[80];
    int value, max_length, rlen;

    (void) sprintf(temp, "%s (%d-%d)[%d]:", input_str, mn, mx, deflt);
    display_mesg2(w, temp, col, row, font);
    sync_output(FALSE);

    max_length = 11;
    value = 0;
    response[0] = '\0';

    disp.x = col + 1 + strlen(temp);
    disp.y = row;
    disp.len = max_length + 1;
    disp.str = disp_response;

    while (1) {
	(void) sprintf(disp_response, "%s_ ", response);
	draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
	sync_output(FALSE);
	next_char = get_reply();
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
	    case 127:
		rlen = strlen(response);
		if (rlen > 0) {
		    value /= 10;
		    *(response + rlen - 1) = '\0';
		}
		break;
	    case 'm':
	    case 'M':
		value <<= 10;	/* * 1024 */
		/* Missing Break Important */
	    case 'k':
	    case 'K':
		value <<= 10;	/* * 1024 */
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
input_string(w, prompt, response, col, row, font, max_length)
int w;
char *prompt, *response;
int col, row, font, max_length;
{
    Word disp;
    char next_char, disp_response[300];
    int rlen;

    display_mesg2(w, prompt, col, row, font);
    sync_output(FALSE);

    response[0] = '\0';

    disp.x = col + 1 + strlen(prompt);
    disp.y = row;
    disp.len = max_length + 1;
    disp.str = disp_response;

    while (1) {
	(void) strcpy(disp_response, response);
	(void) strcat(disp_response, "_ ");
	draw_text_rc(w, disp.x, disp.y, disp.str, font, WHITE);
	sync_output(FALSE);
	next_char = get_reply();
	switch (next_char) {
	    case 127:
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
confirm(w, disp, col, row, font)
int w;
char *disp;
int col, row, font;
{
    char prompt[80], reply;

    (void) strcpy(prompt, "Are you sure you want to ");
    (void) strcat(prompt, disp);
    (void) strcat(prompt, " (y/n)?");
    display_mesg2(w, prompt, col, row, font);
    reply = get_reply();

    if (reply == 'y' || reply == 'Y')
	return TRUE;
    else
	return FALSE;
}
