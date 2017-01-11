/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
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
	Prog_desc *desc;	/* description of program */
	int status;		/* status of program */
	int next_message;	/* index of next message for program to read */
	int total_time;		/* execution time used by prog (in usec) */
	void *thread;		/* pointer to thread of execution */
	char *thread_buf;	/* buffer used to hold thread, maybe stack */
	char *stack_buf;	/* buffer used to hold thread stack */
	void (*cleanup) ();	/* function to be called to clean up after
				   the program (e.g.  free() some things) */
	void *cleanup_arg;	/* this pointer is passed as an argument
				   to the cleanup function */
} Program;

#endif /* ndef _PROGRAM_H_ */
