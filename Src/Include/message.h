/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** message.h
*/

/*
$Author: lidl $
$Id: message.h,v 2.7 1992/08/31 01:50:45 lidl Exp $

$Log: message.h,v $
 * Revision 2.7  1992/08/31  01:50:45  lidl
 * changed to use tanktypes.h, instead of types.h
 *
 * Revision 2.6  1992/03/31  21:49:23  lidl
 * Post Aaron-3d patches, camo patches, march patches & misc PIX stuff
 *
 * Revision 2.5  1992/01/29  08:39:11  lidl
 * post aaron patches, seems to mostly work now
 *
 * Revision 2.4  1992/01/26  04:54:33  stripes
 * new message types to better support more intelligent robots
 *
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


#include "tanktypes.h"


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
    OP_LOCATION,			/* this is where I am */
    OP_GOTO,				/* go to this square */
    OP_FOLLOW,				/* follow this vehicle */
    OP_HELP,				/* help me! */
    OP_ATTACK,				/* attack this vehicle */
    OP_OPEN,				/* throw me the disc */
    OP_THROW,				/* I threw the disc */
    OP_CAUGHT,				/* I caught the disc */
    OP_ACK,					/* I got your message */
    OP_TEXT,				/* arbitrary text */
    OP_DEATH,				/* somebody died (sent only by xtank itself) */
	/* Gnat/Tagman inspired */
	OP_WHERE_IS,			/* Where is landmark foo */
	OP_HERE_ARE,			/* Foo is at 1+ given places */
	OP_WHATS_IN,
	OP_GRID_CONTAIN,		/* landmark/wall info 1+ positions */
	OP_DO_YOU_HAVE,			/* (some pheeture) */
	OP_WILL_YOU,			/* escort/guard/etc */
	OP_AFFIRMATIVE,			/* verbose yes */
	OP_NEGATIVE,			/* less verbose no */
	OP_CLUELESS,			/* You sent me something I don't understand */
	OP_I_AM,				/* "auto-bio" */
	OP_ENEMY_AT,			/* un-auth'ed bio */
    OP_IFF,					/* IFF key */
	OP_INCOMING,			/* FC's advice about long lived weapon */
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
