/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** message.h
*/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_


#include "types.h"
#include "limits.h"


/* Recipient for messages sent to every vehicle */
#define RECIPIENT_ALL 255

/* Sender for messages sent by the game commentator */
#define SENDER_COM 255

/* Sender for initial set of messages on before slots have been filled */
#define SENDER_NONE 254

/* Sender for messages sent by dead people */
#define SENDER_DEAD 253


/* the different kinds of messages */
typedef enum {
    OP_LOCATION,		/* this is where I am */
    OP_GOTO,			/* go to this square */
    OP_FOLLOW,			/* follow this vehicle */
    OP_HELP,			/* help me! */
    OP_ATTACK,			/* attack this vehicle */
    OP_OPEN,			/* throw me the disc */
    OP_THROW,			/* I threw the disc */
    OP_CAUGHT,			/* I caught the disc */
    OP_ACK,			/* I got your message */
    OP_TEXT,			/* arbitrary text */
    OP_DEATH			/* somebody died (sent only by xtank itself) */
} Opcode;
#define MAX_OPCODES 11		/* how many of them there are */

typedef struct {
    Byte  sender;	/* vehicle number of sender */
    Byte  sender_team;	/* team number of sender */
    Byte  recipient;	/* vehicle number of recipient */
    Opcode  opcode;		/* type of message */
    int   frame;		/* frame number when sent */
    Byte  data[MAX_DATA_LEN];	/* data of message */
} Message;


extern void compose_message();


#endif ndef _MESSAGE_H_
