
/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** program.h
*/

/*
$Author: lidl $
$Id: program.h,v 1.1.1.1 1995/02/01 00:25:43 lidl Exp $
*/

#ifndef _PROGRAM_H_
#define _PROGRAM_H_


  typedef struct {
	  char *name;				/* name of program */
	  char *vehicle;			/* name of default vehicle */
	  char *strategy;			/* description of strategy */
	  char *author;				/* name of author */
	  unsigned int abilities;	/* things the program does */
	  int skill;				/* skill at doing these things (0-10) */
	  void (*func) ();			/* main procedure of program */
	  char *code;				/* pointer to code memory, used internally by
				   XTank */
	  char *filename;
  }
Prog_desc;

  typedef struct {
	  Prog_desc *desc;			/* description of program */
	  int status;				/* status of program */
	  int next_message;			/* index of next message for program to read */
	  int total_time;			/* execution time used by prog (in usec) */
	  void *thread;				/* pointer to thread of execution */
	  char *thread_buf;			/* buffer used to hold thread stack */
	  void (*cleanup) ();		/* function to be called to clean up after the
				   program (e.g.  free() some things) */
	  void *cleanup_arg;		/* this pointer is passed as an argument to the
				   cleanup function */
  }
Program;


#endif /* ndef _PROGRAM_H_ */
