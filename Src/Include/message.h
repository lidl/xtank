/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** message.h
*/

/*
$Author: rpotter $
$Id: message.h,v 2.3 1991/02/10 13:51:24 rpotter Exp $

$Log: message.h,v $
 * Revision 2.3  1991/02/10  13:51:24  rpotter
 * bug fixes, display tweaks, non-restart fixes, header reorg.
 *
 * Revision 2.2  91/01/20  09:58:42  rpotter
 * complete rewrite of vehicle death, other tweaks
 * 
 * Revision 2.1  91/01/17  07:12:37  rpotter
 * lint warnings and a fix to update_vector()
 * 
 * Revision 2.0  91/01/17  02:10:16  rpotter
 * small changes
 * 
 * Revision 1.1  90/12/29  21:02:55  aahz
 * Initial revision
 * 
*/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_


#include "types.h"


#define MAX_DATA_LEN    31	/* number of bytes that can fit in a message */

#define RECIPIENT_ALL 255	/* Recipient for messages sent to every vehicle
				   */
#define SENDER_COM 255		/* Sender for messages sent by the game
				   commentator */
#define SENDER_NONE 254		/* Sender for initial set of messages on before
				   slots have been filled */
#define SENDER_DEAD 253		/* Sender for messages sent by dead people */


typedef Byte ID;		/* vehicle identification */

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
    OP_DEATH,			/* somebody died (sent only by xtank itself) */
    real_MAX_OPCODES
} Opcode;
#define MAX_OPCODES ((int)real_MAX_OPCODES)

typedef struct {
    ID sender;			/* vehicle number of sender */
    Byte sender_team;		/* team number of sender */
    ID recipient;		/* vehicle number of recipient */
    Opcode opcode;		/* type of message */
    int frame;			/* frame number when sent */
    Byte data[MAX_DATA_LEN];	/* data of message */
} Message;


extern void compose_message();


#endif ndef _MESSAGE_H_
