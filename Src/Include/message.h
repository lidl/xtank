/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** $Id$
*/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "tanktypes.h"

#define MAX_DATA_LEN    31		/* number of bytes that can fit in a message */

#define RECIPIENT_ALL 255		/* Recipient for messages sent to every vehicle
				   */
#define SENDER_COM 255			/* Sender for messages sent by the game
				   commentator */
#define SENDER_NONE 254			/* Sender for initial set of messages on before
				   slots have been filled */
#define SENDER_DEAD 253			/* Sender for messages sent by dead people */

#ifndef NO_DAMAGE
#define SENDER_R2D2  252		/* Sender is damage control robot */
#endif


typedef Byte ID;				/* vehicle identification */

/* the different kinds of messages */
  typedef enum {
	  OP_LOCATION,				/*  1 this is where I am */
	  OP_GOTO,				/*  2 go to this square */
	  OP_FOLLOW,				/*  3 follow this vehicle */
	  OP_HELP,				/*  4 help me! */
	  OP_ATTACK,				/*  5 attack this vehicle */
	  OP_OPEN,				/*  6 throw me the disc */
	  OP_THROW,				/*  7 I threw the disc */
	  OP_CAUGHT,				/*  8 I caught the disc */
	  OP_ACK,				/*  9 I got your message */
	  OP_TEXT,				/* 10 arbitrary text */
	  OP_IFF,				/* 11 IFF key */
	  OP_WHERE_IS,				/* 12 Where is landmark foo */ /* Gnat/Tagman inspired */
	  OP_HERE_ARE,				/* 13 Foo is at 1+ given places */
	  OP_WHATS_IN,				/* 14 */
	  OP_GRID_CONTAIN,			/* 15 landmark/wall info 1+ positions */
	  OP_DO_YOU_HAVE,			/* 16 (some pheeture) */
	  OP_WILL_YOU,				/* 17 escort/guard/etc */
	  OP_AFFIRMATIVE,			/* 18 verbose yes */
	  OP_NEGATIVE,				/* 19 less verbose no */
	  OP_CLUELESS,				/* 20 You sent me something I don't understand */
	  OP_I_AM,				/* 21 "auto-bio" */
	  OP_ENEMY_AT,				/* 22 un-auth'ed bio */
	  OP_INCOMING,				/* 23 FC's advice about long lived weapon */
	  OP_DEATH,				/* 24 somebody died (sent only by xtank itself) */
	  real_MAX_OPCODES
  } Opcode;

#define MAX_OPCODES ((int)real_MAX_OPCODES)

  typedef struct {
	  ID sender;				/* vehicle number of sender */
	  Byte sender_team;			/* team number of sender */
	  ID recipient;				/* vehicle number of recipient */
	  Opcode opcode;			/* type of message */
	  int frame;				/* frame number when sent */
	  Byte data[MAX_DATA_LEN];	/* data of message */
  }
Message;


extern void compose_message();


#endif /* ndef _MESSAGE_H_ */
