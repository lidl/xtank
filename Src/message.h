/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** message.h
*/

/* Recipient for messages sent to every vehicle */
#define RECIPIENT_ALL 255

/* Sender for messages sent by the game commentator */
#define SENDER_COM 255

/* Sender for initial set of messages on before slots have been filled */
#define SENDER_NONE 254

/* Message opcodes */
#define OP_LOCATION	0
#define OP_GOTO		1
#define OP_FOLLOW	2
#define OP_HELP		3
#define OP_ATTACK	4
#define OP_OPEN		5
#define OP_THROW	6
#define OP_CAUGHT	7
#define OP_ACK		8
#define OP_TEXT		9
#define OP_DEATH        10
#define MAX_OPCODES	11

/* Message data types */
#define DATA_LOC	0
#define DATA_COMB	1
#define DATA_MISC	2

/* Message display string length is 9 more than the data length */
#define MAX_MESSAGE_LEN (MAX_DATA_LEN+9)

/* Font to display messages and the message menus */
#define MSG_FONT	S_FONT

#ifdef S1024x864
/* Menu locations */
#define RECIPIENT_X 5
#define RECIPIENT_Y 5
#define VEHICLE_X   80
#define VEHICLE_Y   5
#define OPCODE_X    190
#define OPCODE_Y    5
#define SEND_X      5
#define SEND_Y      110

/* Row of game window that the sending message is displayed */
#define SENDING_ROW 17
#endif
