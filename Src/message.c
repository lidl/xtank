/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** message.c
*/

/*
$Author: lidl $
$Id: message.c,v 2.9 1992/04/09 04:15:28 lidl Exp $

$Log: message.c,v $
 * Revision 2.9  1992/04/09  04:15:28  lidl
 * changed to use system limits file to figure out INT_MAX, also
 * avoids a warning from ANSI compilers
 *
 * Revision 2.8  1992/03/31  21:45:50  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.7  1992/01/29  08:35:19  lidl
 * post-aaron patches, seems to mostly work now
 *
 * Revision 2.6  1992/01/26  04:58:41  stripes
 * most of the new message types (KJL)
 *
 * Revision 2.5  1992/01/06  07:07:02  stripes
 * Firse step in adding new messages.
 *
 * Revision 2.4  1991/11/22  03:36:47  aahz
 * fixed the print_num macro so numbers don't get munged!
 *
 * Revision 2.3  1991/02/10  13:51:22  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:40  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:35  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:14  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:54  aahz
 * Initial revision
 * 
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "menu.h"
#include "message.h"
#include "map.h"
#include "vehicle.h"
#include "terminal.h"
#include "globals.h"
#include "assert.h"
#include <limits.h>

#define DLEN	80


extern Terminal *term;


#ifdef S1024x864
/* Menu locations */
#define RECIPIENT_X 15
#define RECIPIENT_Y 22
#define VEHICLE_X   105
#define VEHICLE_Y   22
#define OPCODE_X    180
#define OPCODE_Y    22
#define SEND_X      15
#define SEND_Y      122

/* Message data types */
#define DATA_LOC	0
#define DATA_COMB	1
#define DATA_MISC	2
#define DATA_VINFO	3
#define DATA_LANDMK	4
#define DATA_LOCS	5

/* Row of game window that the sending message is displayed */
#define SENDING_ROW 17
#endif

/* Message display string length is 9 more than the data length */
#define MAX_MESSAGE_LEN (MAX_DATA_LEN+9)

/* Font to display messages and the message menus */
#define MSG_FONT	S_FONT


static char *op_str[MAX_OPCODES] = {
	"I'm at ",				/* 0 */
	"Go to ",				/* 1 */
	"Follow combatant ",	/* 2 */
	"Help me at ",			/* 3 */
	"Attack combatant ",	/* 4 */
	"I'm open at ",			/* 5 */
	"I'm throwing to ",		/* 6 */
	"I caught at ",			/* 7 */
	"Acknowledged",			/* 8 */
	"",						/* 9 Text */
	"",						/* 10 Death */
	"Where is ",			/* 11 */
	"Here are ",			/* 12 */
	"What's in...",			/* 13 */
	"Grid Contains...",		/* 14 */
	"Do you have...",		/* 15 */
	"Will you...",			/* 16 */
	"Affirmative...",		/* 17 */
	"Negative...",			/* 18 */
	"I don't understand...",	/* 19 */
	"I am ",				/* 20 */
	"Enemy",				/* 21 */
	"IFF key ",				/* 22 */
	"INCOMING!!!   ...",	/* 23 */
};

static char *opcode_entries[MAX_OPCODES] = {
	"Location", "Goto", "Follow", "Help", "Attack",
	"Open", "Throw", "Caught", "Ack", "Text", "Death",
	"Where is", "Here are", "What's in", "Grid has",
	"Do ya'", "Will you", "Affirmative",
	"Negative", "Clueless", "I am", "Enemy at",
	"IFF",
	"INCOMING!"};

/* Data types for each opcode */
static Byte data_type[MAX_OPCODES] = {
	DATA_LOC, DATA_LOC, DATA_COMB, DATA_LOC, DATA_COMB,
	DATA_LOC, DATA_LOC, DATA_LOC, DATA_MISC, DATA_MISC, DATA_MISC,
	DATA_LANDMK, DATA_LOCS, DATA_MISC, DATA_MISC,
	DATA_MISC, DATA_MISC, DATA_MISC,
	DATA_MISC, DATA_MISC, DATA_MISC, DATA_VINFO,
	DATA_MISC,
	DATA_VINFO
};

/* Whether or not to initialize the data for each opcode */
static Byte data_init[MAX_OPCODES] = {
	TRUE, FALSE, FALSE, TRUE, FALSE,
	TRUE, FALSE, TRUE, FALSE, FALSE, FALSE,
/*?*/
	FALSE, FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE, FALSE,
	FALSE,
	FALSE };

Menu_int msg_sys[MAX_TERMINALS];

/* Clears the text of a message in a window */
#define clear_message(w,num) \
  clear_text_rc(w,0,num,MAX_MESSAGE_LEN,1,MSG_FONT)

/* Prints the text of a message in a window */
#define print_message(w,num,disp) \
  clear_message(w,num); \
  display_mesg(w,disp,num,MSG_FONT)

/* Clears and highlights a line of text in a window */
#define hl_message(w,num) \
  clear_message(w,num); \
  draw_hor(w,5,num * font_height(MSG_FONT) + 7, \
	   font_string_width("M",MSG_FONT) * MAX_MESSAGE_LEN,DRAW_COPY,WHITE)

#define VEHICLE_MENU   0
#define RECIPIENT_MENU 1
#define OPCODE_MENU    2
#define SEND_MENU      3

/* Number of entries on recipients menu */
#define MAX_RECIPIENTS  MAX_TEAMS + 1

static char *vehicle_entries[MAX_VEHICLES];
static char *recipient_entries[MAX_RECIPIENTS];
static char *send_entries[] = {"Send"};

/*
** Initializes the entries arrays for the recipient and vehicle menus.
*/
init_msg_sys()
{
	extern char *teams_entries[];
	int i;

	/* Initialize the recipient entries from the team names */
	recipient_entries[0] = "All";
	for (i = 1; i < MAX_RECIPIENTS; i++)
		recipient_entries[i] = teams_entries[i - 1];
}

/*
** Initializes the message menu system and makes the recipient and opcode menus
** for the specified terminal number.
*/
init_msg_terminal(num)
    int num;
{
	Menu_int *sys;

	/* Make 3 non-holding menus in the game window */
	sys = &msg_sys[num];
	menu_sys_window(sys, GAME_WIN);
	menu_noho_make(sys, VEHICLE_MENU, "Vehicle", MAX_VEHICLES, 0,
				   VEHICLE_X, VEHICLE_Y, vehicle_entries, MSG_FONT, FALSE);
	menu_noho_make(sys, RECIPIENT_MENU, "Recipient", MAX_RECIPIENTS,
		   0, RECIPIENT_X, RECIPIENT_Y, recipient_entries, MSG_FONT, FALSE);
	menu_noho_make(sys, OPCODE_MENU, "Opcode", MAX_OPCODES - 1, 0,
				   OPCODE_X, OPCODE_Y, opcode_entries, MSG_FONT, FALSE);
	menu_simp_make(sys, SEND_MENU, "", 1, 0,
				   SEND_X, SEND_Y, send_entries, XL_FONT);
}

/*
** Sets all vehicle menu entries to be the names of the owners of the
** vehicles.  Resizes the vehicle menu to fit the new list.  Sets highlights
** on the menus to recipient "All", opcode "Text".
*/
init_msg_game()
{
    extern int num_terminals;
    extern Terminal *terminal[];
    Menu_int *sys;
    int i;

    for (i = 0; i < num_veh; i++)
	vehicle_entries[i] = actual_vehicles[i].owner->name;

    for (i = 0; i < num_terminals; i++)
    {
	sys = &msg_sys[terminal[i]->num];
	menu_resize(sys, VEHICLE_MENU, num_veh);
	menu_unhighlight(sys, VEHICLE_MENU);
	menu_unhighlight(sys, RECIPIENT_MENU);
	menu_unhighlight(sys, OPCODE_MENU);
	menu_set_hil(sys, RECIPIENT_MENU, 0);
	menu_set_hil(sys, OPCODE_MENU, OP_TEXT);
    }
}

/*
** Displays the menus and sending message in the game window.
*/
display_game(status)
unsigned int status;
{
	/* Check for being exposed */
	check_expose(GAME_WIN, status);

	/* If we are turning this window off or on, clear it */
	if (status != REDISPLAY)
		clear_window(GAME_WIN);

	if (status == ON)
	{
		menu_sys_display(&msg_sys[term->num]);

		/* Display the sending message if there is a vehicle */
		if (term->vehicle != (Vehicle *) NULL)
			display_sending();
	}
}

/*
** Handles input events in message window.  This includes clicks on menus
** and keystrokes for editing messages.
*/
message_input(event)
Event *event;
{
    Vehicle *v = term->vehicle;
    Message *m;
    char disp[DLEN];
    int menu, choice, len, i, itemp;
    Boolean fast_print, fast_erase;

    /* can't send messages in observation mode */
    if (v == NULL || term->observer)
	return GAME_RUNNING;

    fast_print = fast_erase = FALSE;
    m = &v->sending;
    switch (event->type)
    {
      case EVENT_LBUTTON:
      case EVENT_MBUTTON:
      case EVENT_RBUTTON:
	menu = menu_hit(&msg_sys[term->num], event->x, event->y);
	if (menu == MENU_NULL)
	    return GAME_RUNNING;

	/* Find out which choice on the menu was selected */
	menu_hit_p(&msg_sys[term->num], event, &menu, &choice, &itemp);

	switch (menu)
	{
	  case VEHICLE_MENU:
	    menu_unhighlight(&msg_sys[term->num], RECIPIENT_MENU);
	    {
		int j;

		for (j = 0; j < num_veh; j++)
		{
		    if (actual_vehicles[j].number == choice)
			break;
		}
		assert(j < num_veh);
		m->recipient = actual_vehicles[j].number;
	    }
	    break;
	  case RECIPIENT_MENU:
	    menu_unhighlight(&msg_sys[term->num], VEHICLE_MENU);
	    m->recipient = (choice == 0) ? RECIPIENT_ALL
		: MAX_VEHICLES + choice - 1;
	    break;
	  case OPCODE_MENU:
	    /* Set opcode and clear data, since it may be bad */
	    m->opcode = (Opcode) choice;
	    for (i = 0; i < MAX_DATA_LEN; i++)
		m->data[i] = '\0';

	    /* Initialize data with location if opcode desires it */
	    if (data_init[(int)m->opcode])
	    {
		m->data[0] = v->loc->grid_x;
		m->data[1] = v->loc->grid_y;
	    }
	    break;
	  case SEND_MENU:
	    /* Send the current message */
	    send_message(v);
	    break;
	}
	break;
      case EVENT_KEY:
	if (event->key == '\r')
	{
	    send_message(v);
	    break;
	}

	/* Ignore text if we are not writing a text message */
	if (m->opcode != OP_TEXT)
	    return GAME_RUNNING;
	len = strlen((char *) m->data);

	/* Check for backspace */
	if (event->key == 127)
	{
	    if (len > 0)
	    {
		m->data[len - 1] = '\0';
		fast_erase = TRUE;
	    }
	    else
		return GAME_RUNNING; /* to avoid prompt flicker */
	}
	else
	{
	    /* If the message is full, send it, else turn fast printing
	       on */
	    m->data[len] = event->key;
	    if (len == MAX_DATA_LEN - 1)
		send_message(v);
	    else
		fast_print = TRUE;
	}
	break;
    }

    /* Reformat message since we have changed some part of it */
    len = format_message(m, disp);

    /* Special cases for typing a character and erasing a character */
    if (fast_print)
	display_mesg2(GAME_WIN, disp + len - 1, len - 1, SENDING_ROW, MSG_FONT);
    else if (fast_erase)
	clear_text_rc(GAME_WIN, len, SENDING_ROW, 1, 1, MSG_FONT);
    else
    {
	print_message(GAME_WIN, SENDING_ROW, disp);
    }
    return GAME_RUNNING;
}

/*
** Interprets mouse clicks as locations if sending message opcode data
** type is a location. Sets data of sending message to clicked location.
*/
map_input(event)
Event *event;
{
    Vehicle *v = term->vehicle;


    /* can't mess with messages if just an observer */
    if (term->observer)
	return GAME_RUNNING;

/*
 * Ok, so it's not exactly the most elegant structure. Tries to copy
 * the format of the code in input.c
 */

        if (v == (Vehicle *) NULL)
            return GAME_RUNNING;

        set_current_vehicle(v);

#ifdef KEYPAD_DETECT

    if ( (event->type == EVENT_KEY) 
	 && (event->key >= '1') && (event->key <= '6') ) {


	v->target.y = (map2grid(event->y) * BOX_WIDTH) + (BOX_WIDTH/2);
	v->target.x = (map2grid(event->x) * BOX_HEIGHT) + (BOX_HEIGHT/2);

                switch (event->key) {
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':

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

	                  return GAME_RUNNING;


                        }
			break;


                    default:
                        beep_window();
	                return GAME_RUNNING;
                        break;
	}

    } else

#endif /* KEYPAD_DETECT */

    if (event->type == EVENT_LBUTTON) {
        int i;
    

	v->target.y = (map2grid(event->y) * BOX_WIDTH) + (BOX_WIDTH/2);
	v->target.x = (map2grid(event->x) * BOX_HEIGHT) + (BOX_HEIGHT/2);

	for (i = 0; i < v->num_weapons; i++)
	    if (v->weapon[i].type == HARM) 
		fire_weapon(i);
	
	return GAME_RUNNING;

    } else {

    if (event->type == EVENT_KEY && event->key == '\r')
	send_message(v);

    if (data_type[(int)v->sending.opcode] != DATA_LOC)
	return GAME_RUNNING;

    if (event->type == EVENT_LBUTTON || event->type == EVENT_MBUTTON ||
	event->type == EVENT_RBUTTON)
    {
	v->sending.data[0] = map2grid(event->x);
	v->sending.data[1] = map2grid(event->y);
	display_sending();
    }
    return GAME_RUNNING;
}

}

/*
** Requires that the vehicle sent is from the current terminal.
** If the sending message of that vehicle is combatant, then the data
** is set to the vehicle number of the nearest vehicle to the event's
** coordinates on the screen.
*/
set_message_data(v, event)
Vehicle *v;
Event *event;
{
    int dx, dy, dist, min_dist, min_num, x, y, dtype, i;

    dtype = data_type[(int)v->sending.opcode];
    if (dtype == DATA_COMB)
    {
        /* Find the vehicle on screen with shortest distance to that location
	 */
	min_dist = INT_MAX;
	for (i = 0; i < num_veh_alive; i++)
	{
	    x = live_vehicles[i]->loc->screen_x[term->num];
	    y = live_vehicles[i]->loc->screen_y[term->num];
	    if (x < 0 || x >= ANIM_WIN_WIDTH || y < 0 || y >= ANIM_WIN_HEIGHT)
		continue;
	    dx = x - event->x;
	    dy = y - event->y;
	    dist = dx * dx + dy * dy;
	    if (dist < min_dist)
	    {
		min_dist = dist;
		min_num = live_vehicles[i]->number;
	    }
	}
	v->sending.data[0] = min_num;
    }
    else if (dtype == DATA_LOC)
    {
	v->sending.data[0] = (term->loc.x + event->x) / BOX_WIDTH;
	v->sending.data[1] = (term->loc.y + event->y) / BOX_HEIGHT;
    }
    display_sending();
}

/*
** Displays the sending message of the current terminal.
*/
display_sending()
{
	char disp[DLEN];

	format_message(&term->vehicle->sending, disp);
	print_message(GAME_WIN, SENDING_ROW, disp);
}

/*
** Initializes the message system of the specified vehicle.
*/
init_messages(v)
Vehicle *v;
{
	int i;

	v->next_message = 0;
	v->new_messages = 0;
	for (i = 0; i < MAX_MESSAGES; i++)
		v->received[i].sender = SENDER_NONE;
	v->sending.sender = v->number;
	v->sending.recipient = RECIPIENT_ALL;
	v->sending.opcode = OP_TEXT;
}

/*
** Sends the current message for the specified vehicle, clears the data in it.
*/
send_message(v)
Vehicle *v;
{
	int i;

	dispatch_message(&v->sending);
	for (i = 0; i < MAX_DATA_LEN; i++)
		v->sending.data[i] = '\0';
}

/*
** Sends a death message from commentator saying that the victim was killed
** by the killer.
*/
send_death_message(victim, killer)
Vehicle *victim, *killer;
{
	Message m;

	m.sender = SENDER_COM;
	m.recipient = RECIPIENT_ALL;
	m.opcode = OP_DEATH;
	m.data[0] = victim->number;
	if (killer != (Vehicle *) NULL) {
		m.data[1] = killer->owner->kills;
		m.data[2] = killer->owner->number;
	} else {
		m.data[1] = 0;
		m.data[2] = SENDER_NONE;
	}
	dispatch_message(&m);
}

/*
** Dispatches a message to the recipient vehicles.
*/
dispatch_message(m)
Message *m;
{
    int rec, i;
    int sender_dead;

    /* Sender is either a vehicle number, a number indicating anonymous, or a
       number indicating no sender.  Commentator sender is neutral.  No sender
       means a bad message. */
    if (m->sender == SENDER_COM)
	m->sender_team = 0;
    else if (m->sender == SENDER_DEAD)
    {

#ifdef WEIRD_BUG
	printf("sender dead\n");
#endif

	sender_dead = m->data[0];
	m->sender_team = SENDER_NONE;
	for (i = 0; i < num_veh; i++)
	{
	    if (sender_dead == actual_vehicles[i].number)
		m->sender_team = actual_vehicles[i].team;
	}
    }
    else
    {
	m->sender_team = SENDER_NONE;
	for (i = 0; i < num_veh; i++)
	{
	    if (m->sender == actual_vehicles[i].number)
		m->sender_team = actual_vehicles[i].team;
	}
    }
    if (m->sender_team == SENDER_NONE)
	return;

#ifdef WEIRD_BUG
    printf("didn't quit\n");
#endif

    /* Recipient is either a vehicle number, a special number indicating all
       vehicles, or a team number + MAX_VEHICLES. */
    rec = m->recipient;
    if (rec >= 0 && rec < num_veh)
    {
	for (i = 0; i < num_veh_alive; i++)
	    if (rec == live_vehicles[i]->number)
	    {
		receive_message(live_vehicles[i], m);
		break;
	    }
    }
    else if (rec == RECIPIENT_ALL)
    {
	sender_dead = m->sender;
	if (sender_dead == SENDER_DEAD)
	{
	    m->sender = m->data[0];
            memcpy((char *)m->data, (char *)(m->data + 1), MAX_DATA_LEN - 1);
	}

	for (i = 0; i < num_veh_alive; i++)
	{

#ifdef WEIRD_BUG
	    printf("sending a %d message to #%d from #%d\n", m->opcode, i, m->sender);
#endif

	    if (sender_dead == SENDER_DEAD)
	    {
		m->recipient = live_vehicles[i]->number;
	    }

	    receive_message(live_vehicles[i], m);
	}
    }
    else
    {
	/* Convert recipient to a team number */
	rec -= MAX_VEHICLES;
	for (i = 0; i < num_veh_alive; i++)
	    if (rec == live_vehicles[i]->team)
		receive_message(live_vehicles[i], m);
    }
}

#define inc_index(i) { if(++(i) == MAX_MESSAGES) (i) = 0; }

/*
** Receives a the specified message for the specified vehicle.
*/
receive_message(v, m)
Vehicle *v;
Message *m;
{
	/* * Increment the number of new messages received this frame, copy * the
	   message onto the list, and increment the next message index. * All
	   indexing is done modulo MAX_MESSAGES. */

/*
 * If this is a IFF key...
 *    has recp offered IFF key exchange to sender?
 *       yes: set both vehicles as having each others keys
 *       no:  set sender vehicle to reflect his offer to exchange.
 */
	if (( m->sender < MAX_VEHICLES) && ( m->opcode == OP_IFF) )
		if (v->offered_IFF_key[m->sender]) {
			actual_vehicles[m->sender].have_IFF_key[v->number] = TRUE;
			v->have_IFF_key[m->sender] = TRUE;
		} else {
			actual_vehicles[m->sender].offered_IFF_key[v->number] = TRUE;
		}

#ifdef TRACE_MESSAGE
	printf("before incr new_messages = %d\n", v->new_messages);
#endif

	inc_index(v->new_messages);

#ifdef TRACE_MESSAGE
	printf("after incr new_messages = %d\n\n", v->new_messages);
#endif

	STRUCT_ASSIGN(v->received[v->next_message], *m, Message);

#ifdef TRACE_MESSAGE
	printf("before incr next_message = %d\n", v->next_message);
#endif

	inc_index(v->next_message);

#ifdef TRACE_MESSAGE
	printf("after incr next_message = %d\n\n", v->next_message);
#endif
}

/*
** Displays the messages in the message window.
** Messages are displayed in a wraparound fashion.
** The most recently received message is highlighted and is followed by a
** blank line.
*/
display_msg(status)
unsigned int status;
{
    Vehicle *v = term->vehicle;
    char disp[DLEN];
    int num;

    /* Check for being exposed */
    check_expose(MSG_WIN, status);

    /* If we are turning this window off or on, clear it */
    if (status != REDISPLAY)
	clear_window(MSG_WIN);

    /* no messages if no vehicle being tracked */
    if (v == (Vehicle *) NULL)
	return;

    switch (status)
    {
      case ON:
	/* Display all the messages and highlight the next open line */
	for (num = 0; num < MAX_MESSAGES; num++)
	{
	    if (num == v->next_message)
	    {
		hl_message(MSG_WIN, num);
	    }
	    else
	    {
		format_message(&v->received[num], disp);
		display_mesg(MSG_WIN, disp, num, MSG_FONT);
	    }
	}
	hl_message(MSG_WIN, v->next_message);
	break;
      case REDISPLAY:
	/* If there are new messages, display them and highlight the next
	   line */
	if (v->new_messages > 0 && !v->death_timer)
	{
	    num = v->next_message - v->new_messages;
	    if (num < 0)
		num = MAX_MESSAGES + num;

#ifdef TRACE_MESSAGE
	    printf("\nprinting %d messages starting at %d\n\n", v->new_messages, num);
#endif

	    while (num != v->next_message)
	    {
		if (v->received[num].recipient != RECIPIENT_ALL)
		{
		    beep_window();
		}
		format_message(&v->received[num], disp);
		print_message(MSG_WIN, num, disp);
		inc_index(num);
	    }
	    hl_message(MSG_WIN, num);
	}
	break;
    }				/* end switch */
}

/* Macros for adding strings and numbers to the display string */
#define print_num(rnum) { \
   int num = rnum; \
\
   if ((num) < 0) { \
	  disp[i++] = '-'; \
	  num = -num; \
   } \
   if ((num) < 10) { \
      disp[i++] = '0' + (char) (num); \
   } else { \
       if ((num) < 100) { \
          disp[i++] = (char) ((num) / 10) + '0';  \
          disp[i++] = (char) ((num) % 10) + '0'; \
       } else { \
           if ((num) < 255) { \
              disp[i++] = '0' + (char) ((num) / 100); \
              disp[i++] = '0' + (char) ((num) / 10) % 10; \
              disp[i++] = '0' + (char) ((num) % 10); \
           } else { \
              disp[i++] = '+'; \
              disp[i++] = '+'; \
              disp[i++] = '+'; \
           } \
		} \
	}  \
}

#define print_xy(x, y) { \
		disp[i++] = '('; \
		print_num(x); \
		disp[i++] = ','; \
		print_num(y); \
		disp[i++] = ')'; \
	}

#define print_str(str) \
  j = 0; \
  while(j < DLEN && str[j] != '\0') \
    disp[i++] = (char) str[j++];

#define print_combatant(num) \
{ \
    int fooj; \
 \
    for (fooj = 0; fooj < num_veh; fooj++) { \
	if (actual_vehicles[fooj].number == num) \
	    break; \
    } \
    if (fooj == num_veh) {  \
	printf("assert failed on num = %d %c\n",num,num);  \
	printf("%s\n", m->data); \
	printf("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n", \
	       *(m->data), *(m->data+1), *(m->data+2), *(m->data+3), \
	       *(m->data+4), *(m->data+5), *(m->data+6), *(m->data+7), \
	       *(m->data+8), *(m->data+9), *(m->data+10)); \
	printf("m->opcode %d, sender %d, team %d, recp %d\n",  \
	       m->opcode, m->sender, m->sender_team, m->recipient);  \
	printf("Failure in print_combatant, attempting to continue\n"); \
	disp[i++] = 'A';  \
	disp[i++] = 'F';  \
	disp[i++] = 'U';  \
    } else { \
	disp[i++] = team_char[actual_vehicles[fooj].team];  \
	print_num(num);  \
    }  \
}

/*
** Formats message into a string    {sender}->{recipient} {opcode} {data}
** Returns length of string.
*/
format_message(m, disp)
Message *m;
char *disp;
{
	extern char team_char[];
	Byte *data;
	int rec, sen, i, j, ictr, jctr;
	Opcode op;
	static box_names_valid = FALSE;
	extern char *box_type_name[];

	i = 0;
	/* Add sender id characters */
	sen = m->sender;
	if (sen == SENDER_NONE) {
		/* No sender, message doesn't exist */
		disp[i] = '\0';
		return 0;
	}
	if (sen == SENDER_COM) {
		/* Commentator sender */
		disp[i++] = 'C';
		disp[i++] = 'O';
		disp[i++] = 'M';
	} else {
		/* Vehicle sender */
		print_combatant(sen);
	}

	/* Add arrow */
	disp[i++] = '-';
	disp[i++] = '>';

	/* Add receiver id characters */
	rec = m->recipient;
	if (rec == RECIPIENT_ALL) {
		/* All vehicles received message */
		disp[i++] = 'A';
		disp[i++] = 'L';
		disp[i++] = 'L';
	} else {
		if (rec >= MAX_VEHICLES) {
			/* Team received message */
			disp[i++] = team_char[rec - MAX_VEHICLES];
		} else {
			/* Vehicle received message */
			print_combatant(rec);
		}
	}

	disp[i++] = ' ';

	/* Add body of message, constructed from opcode and data */
	op = m->opcode;
	data = m->data;
	print_str(op_str[(int)op]);
	switch (data_type[(int)op]) {
	  case DATA_VINFO:
		disp[i++] = ' ';
		print_combatant(data[8]);
		print_str(" at ");
		print_xy(data[0], data[1]);
		print_str(" > ");
		print_num(-128 + (int)data[6]);
		disp[i++] = ',';
		print_num(-128 + (int)data[7]);
		break;
	  case DATA_LOC:
		print_xy(data[0], data[1]);
		break;
	  case DATA_COMB:
		/* Data is a combatant number */
		print_combatant(data[0]);
		break;
	  case DATA_LOCS:
		for(ictr = 1; ictr < 28; ictr += 2) {
			if (data[ictr] < GRID_WIDTH || data[ictr] < 0) {
				ictr -= 2;
				break;
			}
		}
		print_num(1+ ictr / 2);
		disp[i++] = ' ';
		print_str(box_type_name[data[0]]);
		disp[i++] = ' ';
		assert(GRID_WIDTH < 100 && GRID_HEIGHT < 100);
		jctr = 1;
		while(i < DLEN -8 && jctr < ictr) {
			print_xy(data[jctr], data[jctr+1]);
			jctr += 2;
		}
		if (jctr < ictr && i < DLEN -4) print_str("...");
		break;
	  case DATA_LANDMK:
		if (!box_names_valid) {
			init_box_names();
			box_names_valid = TRUE;
		}

		print_str(box_type_name[data[0]]);
		break;
	  case DATA_MISC:
		if (op == OP_DEATH) {
			print_combatant(data[0]);
			if (data[2] == SENDER_NONE) {
				print_str(" died");
			} else {
				print_str(" was kill ");
				print_num(data[1]);
				print_str(" for ");
				print_combatant(data[2]);
			}
		} else {
			/* Data is a string */
			print_str(data)
		}
		break;
	}

	if (i >= DLEN) {
		printf("op %d %s, type %d\n", m->opcode, op_str[m->opcode], data_type[m->opcode]);
		printf("%d chars\n", i);
		disp[i] = '\0';
		printf("'%s'\n", disp);
		assert(i < DLEN);
	}
	disp[i] = '\0';
	return i;
}

/*
** Composes a message with the given information and dispatches it.
*/
void compose_message(sender, sendee, opcode, data)
Byte sender, sendee;
Opcode opcode;
Byte *data;
{
	Message m;
	int i;

	m.sender = sender;
	m.recipient = sendee;
	m.opcode = opcode;
	for (i = 0; i < MAX_DATA_LEN - 1; i++)
		m.data[i] = data[i];
	dispatch_message(&m);
}
