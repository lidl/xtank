/*
** Xtank
**
** Copyright 1992 by C. Moore
**
** $Id$
*/

#define MAX_LENGTH 255

#include <stdio.h>
#include <sys/param.h>
#include "proto.h"

extern int number_of_machines;
extern char **machine_names, **machine_entries;
extern char pathname[];

init_players()
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
