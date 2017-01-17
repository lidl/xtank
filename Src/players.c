/*-
 * Copyright (c) 1992 Chris Moore
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

#define MAX_LENGTH 255

#include <stdio.h>
#include <sys/param.h>
#include "proto.h"

extern int number_of_machines;
extern char **machine_names, **machine_entries;
extern char pathname[];

void
init_players(void)
{
	FILE *fp;
	char filename[MAXPATHLEN], buffer[MAX_LENGTH + 1];
	int i;
	char *p;

	(void) strcpy(filename, pathname);
	(void) strcat(filename, "/");
	(void) strcat(filename, "players");

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Cannot find players file %s\n", filename);
		exit(-1);
	}
	machine_names = (char **) malloc(sizeof(char *));
	machine_entries = (char **) malloc(sizeof(char *));

	/* Get the machine name. */
	while (fscanf(fp, "%s", buffer) == 1) {
		machine_names = (char **) realloc(machine_names,
										  sizeof(char *) *
										    (number_of_machines + 1));

		machine_entries = (char **) realloc(machine_entries,
											sizeof(char *) *
											  (number_of_machines + 1));

		machine_names[number_of_machines] = (char *) malloc(strlen(buffer) + 1);
		strcpy(machine_names[number_of_machines], buffer);

		/* Get the player name. */
		while ((buffer[0] = getc(fp)) == ' ' || buffer[0] == '\t')
			/* do nothing */
			;

		for (i = 1; i < MAX_LENGTH + 1; i++) {
			buffer[i] = getc(fp);

			if (buffer[i] == '\n')
				break;
		}

		if (i == MAX_LENGTH + 1) {
			while (getc(fp) != '\n')
				/* do nothing */
				;
			buffer[MAX_LENGTH] = 0;
		} else
			buffer[i] = 0;

		machine_entries[number_of_machines] = (char *) malloc(strlen(buffer) + 1);
		strcpy(machine_entries[number_of_machines], buffer);

		number_of_machines++;
	}

	fclose(fp);
}
