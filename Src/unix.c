#include "malloc.h"
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** unix.c
*/

#include <stdio.h>
#include <strings.h>
#include "xtank.h"
#include "config.h"
#include <sys/types.h>
#include <sys/file.h>

extern char *index(), *rindex();
extern off_t lseek();


#ifndef UNIX
#undef DYNALOAD
#undef CHECKNET
#endif


#define MAX_CLIENTS 32


#ifdef UNIX
char *network_error_str[6] = {
	"",
	"Cannot determine name of server",
	"Cannot determine location of server",
	"Client display name is too long",
	"Cannot determine location of client",
	"Client is too far away on the network"
};

#endif


#ifdef CHECKNET
/* Network includes */
#include <netdb.h>

/*
** Checks the internet addresses of each client name against the
** name of the server, the machine this process is running on.
**
** Return code  Meaning
** -----------  -------
**      0       all the addresses are on the same subnet
**      1       the server's name cannot be determined
**      2       the server's location cannot be determined
**      3       client's display name is too long
**      4       client's location cannot be determined
**      5       client is not on the same subnet as the server
*/

check_internet(num_clients, client)
int num_clients;
char **client;
{
	char server_addr[5];
	char client_addr[MAX_CLIENTS][5];
	char server_name[256];
	char client_name[256];
	struct hostent *host;
	char *tmp;
	int newlen;
	int i;

	/* Get the name of the server */
	if (gethostname(server_name, 256) != 0)
		return 1;

	/* Get the location of the server */
	host = gethostbyname(server_name);
	if (host == (struct hostent *) NULL)
		return 2;

	/* Get the network address of the server */
	(void) bcopy(host->h_addr, server_addr, sizeof(host->h_addr));

	/* Make sure all clients are on same subnet as the server */
	for (i = 0; i < num_clients; i++)
	{
		newlen = strlen(client[i]);

		/* Strip off the :x.y from each display name */
		if ((tmp = rindex(client[i], ':')) != (char *) NULL)
			newlen -= strlen(tmp);

		if (newlen > 225)
			return 3;

		(void) strncpy(client_name, client[i], newlen);
		client_name[newlen] = '\0';

		/* Client "unix" or "" means the server machine, so it's ok */
		if (!strcmp(client_name, "unix") || client_name[0] == '\0')
			continue;

		/* Get the host information about the client */
		host = gethostbyname(client_name);
		if (host == (struct hostent *) NULL)
			return 4;

		/* Get the network address of the client */
		(void) bcopy(host->h_addr, client_addr[i], sizeof(host->h_addr));

		/* Compare subnet of server with subnet of client */
		if (server_addr[1] != client_addr[i][1])
			return 5;
	}
	return 0;
}

#else							/* CHECKNET */

/*ARGSUSED*/
check_internet(num_clients, client)
int num_clients;
char **client;
{
	return 0;
}

#endif							/* CHECKNET */

/*
** UNIX Dynamic loading module
*/

#ifdef DYNALOAD
/* Dynamic loading includes */
#include <a.out.h>
#include <sys/param.h>

struct nlist nl[2];
extern char *malloc();

/*
** Compiles and/or dynamically loads the module with the given name.
** The module name should be a complete path to a .c or .o file.
** If the module is a .c file, it is compiled into a .o file in the
** same directory.  The .o file is then linked into xtank by forking
** an ld and frobbing with the symbol table.  The address of the prog_desc
** is put into pdesc.
**
** Return code  Meaning
** -----------  -------
**      0       module compiled and/or linked properly
**      1       module name is improperly formatted
**      2       .c file compiled incorrectly  (errors in /tmp/xtank.errors)
**      3       .o file linked incorrectly    (errors in /tmp/xtank.errors)
**      4       output file cannot be read
**      5       name list for the symbol table could not be generated
**      6       module does not have a program description structure
*/

#ifndef hp9000s800
compile_module(module_name, symbol, code, error_name, output_name)
char *module_name;
char **symbol, **code;
char *output_name, *error_name;
{
	extern char *executable_name, pathname[], programsdir[];
	extern char headersdir[];
	char *func, *func1, command[256], address[20], symbol_name[256], *p;
	register struct nlist *nlst, *nlp, *nlstend;
	struct nlist *namelist();
	int fd;
	struct exec hdr;

	/* Come up with some space for the module on a page boundary */
	func = (char *) malloc(0x10000);
	func1 = (char *) ((((int) func) + 2047) & ~0x7ff);
	sprintf(address, "%x", func1);

#ifdef DEBUG
	puts(address);
#endif

	/* Chop off everything from the . on to make the rest easier */
	if (p = rindex(module_name, '.'))
		*p = '\0';
	else
	{
		free(func);
		return 1;
	}

	/* Parse the module name to see if it is a .o or a .c */
	switch (*(p + 1))
	{
		case 'c':
			/* Do the compile */
            sprintf(command,
		    "cd %s/%s; /bin/cc -c -O -I%s -o %s.o %s.c > %s 2>&1",
					pathname, programsdir, headersdir, module_name,
					module_name, error_name);

#ifdef DEBUG
			printf("compile command: %s\n", command);
#endif

			if (system(command))
			{
				free(func);
				return 2;
			}
		case 'o':
			/* Do the link */
            sprintf(command,
		    "/bin/ld -x -N -A %s -T %s -o %s %s.o -lm -lc > %s 2>&1",
		    executable_name, address, output_name, module_name,
		    error_name);

#ifdef DEBUG
			printf("link command: %s\n", command);
#endif

			if (system(command))
			{
				free(func);
				return 3;
			}
			break;
		default:
			free(func);
			return 1;
	}

	/* Now find the symbol in the symbol table */
	if ((fd = open(output_name, O_RDONLY)) < 0)
	{
		free(func);
		return 4;
	}
	nlst = namelist(fd, &hdr);
	if (nlst == (struct nlist *) NULL)
	{
		free(func);
		return 5;
	}

#ifdef DEBUG
	printf("Magic number: %o\n", hdr.a_magic);
	printf("Text          %d\n", hdr.a_text);
	printf("Data          %d\n", hdr.a_data);
	printf("BSS           %d\n", hdr.a_bss);
	printf("Offset: %d\n", N_TXTOFF(hdr));
#endif

    lseek(fd, (off_t)N_TXTOFF(hdr), L_SET);

    read(fd, func1, (int)(hdr.a_text + hdr.a_data));

#ifdef DEBUGfag
	for (p = func1, c = hdr.a_text + hdr.a_data; c; --c, ++p)
		printf("%x %x\n", p, *p);
	printf("Found it at location %x\n", func1);
#endif

	/* Come up with the entry symbol for the Prog_desc structure */
	if ((p = rindex(module_name, '/') + 1) == (char *) 1)
		p = module_name;

	(void) strcpy(symbol_name, "_");
	(void) strcat(symbol_name, p);
	(void) strcat(symbol_name, "_prog");
	nlstend = nlst + (hdr.a_syms / sizeof(struct nlist));
	for (nlp = nlst; nlp < nlstend; nlp++)
	{
		if (!strcmp(symbol_name, nlp->n_un.n_name))
		{
			/* Fill in the symbol and code pointers */
			*symbol = (char *) nlp->n_value;
			*code = func;

			return 0;
		}
	}

	free(func);
	return 6;
}

#undef NULL
#define NULL ((char *) 0)

/*
** Returns the entire symbol table of the given executable.
** If anything goes wrong, NULL is returned.
*/
struct nlist *
 namelist(fd, hdr)
int fd;							/* (seekable) File descriptor of executable  */
struct exec *hdr;				/* Pointer to exec struct which is filled in */
{
	register struct nlist *nlst, *nlstend;		/* Name list */
	register struct nlist *nlp;	/* Pointer into list */
	register char *stbl;		/* String table */
	int size;					/* String table size */

	/* Snarf the header out of the file. */

	lseek(fd, (long) 0, L_SET);
    if ((read(fd, (char *)hdr, sizeof(*hdr))) != sizeof(*hdr))
		return ((struct nlist *) NULL);

	/* Allocate a buffer for and read the symbol table */

    nlst = (struct nlist *) malloc((unsigned)hdr->a_syms);
	lseek(fd, (long) N_SYMOFF(*hdr), L_SET);
    if ((read(fd, (char *)nlst, (int)hdr->a_syms)) != hdr->a_syms)
	{
		free((char *) nlst);
		return ((struct nlist *) NULL);
	}
	/* Now, read the string table size. */

	lseek(fd, (long) N_STROFF(*hdr), L_SET);
    if ((read(fd, (char *)&size, sizeof(int))) != sizeof(int))
	{
		free((char *) nlst);
		return ((struct nlist *) NULL);
	}
	/* Allocate a buffer and read the string table out of the file. */

    stbl = malloc((unsigned)size);
	lseek(fd, (long) N_STROFF(*hdr), L_SET);
	if ((read(fd, stbl, size)) != size)
	{
		free((char *) nlst);
		free(stbl);
		return ((struct nlist *) NULL);
	}
	/* Rearrange the namelist to point at the character strings rather than
	   have offsets from the start of the string table. */
	nlstend = nlst + (hdr->a_syms / sizeof(struct nlist));
	for (nlp = nlst; nlp < nlstend; nlp++)
		nlp->n_un.n_name = nlp->n_un.n_strx + stbl;

	/* Return the namelist we just constructed. */
	return (nlst);
}

#else							/* hp9000s800 */

compile_module(module_name, symbol, code, error_name, output_name)
char *module_name;
Prog_desc **symbol;
char **code;
char *output_name, *error_name;
{
	extern char *executable_name, pathname[], programsdir[];
	char *func, *func1, command[256], address[20], symbol_name[256], *p;
	char *sym;
	SYMENT *nlst, *nlp, *nlstend;
	SYMENT *namelist();
	FILHDR filhdr;
	AOUTHDR hdr;
	int fd;

    /* Chop off everything from the last . on to make the rest easier */
	if (p = rindex(module_name, '.'))
		*p = '\0';
	else
		return 1;

	if (sym = rindex(module_name, '/'))
		sym++;
	else
		sym = module_name;

	/* Come up with some space for the module on a page boundary */
	func = (char *) malloc(0x10000);
	func1 = (char *) ((((int) func) + (NBPG - 1)) & ~(NBPG - 1));
	sprintf(address, "%x", func1);

#ifdef DEBUG
	puts(address);
#endif

	/* Parse the module name to see if it is a .o or a .c */
	switch (*(p + 1))
	{
		case 'c':
			/* Do the compile */
			sprintf(command, "cd %s/%s; /bin/cc -c -O -I. %s.c > %s 2>&1",
					pathname, programsdir, module_name, error_name);

#ifdef DEBUG
			printf("compile command: %s\n", command);
#endif

			if (system(command))
			{
				free(func);
				return 2;
			}
		case 'o':
			/* Do the link */
			sprintf(command, "/bin/ld -x -N -A %s -R %s -e %s_main -o %s %s.o -lm -lc > %s 2>&1",
					executable_name, address, sym, output_name, module_name, error_name);

#ifdef DEBUG
			printf("link command: %s\n", command);
#endif

			if (system(command))
			{
				free(func);
				return 3;
			}
			break;
		default:
			free(func);
			return 1;
	}

    /* Find the entry point (foo_main function) */
	if ((fd = open(output_name, O_RDONLY)) < 0)
	{
		free(func);
		return 4;
	}
	nlst = namelist(fd, &filhdr, &hdr);
	if (nlst == NULL)
	{
		free(func);
		return 5;
	}

#ifdef DEBUG
	printf("Text          %d\n", hdr.exec_tsize);
	printf("Data          %d\n", hdr.exec_dsize);
	printf("BSS           %d\n", hdr.exec_bsize);
	printf("Offset: %d\n", hdr.exec_tfile);
#endif

	/* Read the text */
	lseek(fd, hdr.exec_tfile, L_SET);
	read(fd, hdr.exec_tmem, hdr.exec_tsize);

	/* Flush the cache for the text region */
	flush_cache((char *) hdr.exec_tmem, hdr.exec_tsize);

	/* Read the initialized data */
	lseek(fd, hdr.exec_dfile, L_SET);
	read(fd, hdr.exec_dmem, hdr.exec_dsize);

	/* Zero bss */
	memset(hdr.exec_dmem + hdr.exec_dsize, 0, hdr.exec_bsize);

	/* Come up with the entry symbol for the Prog_desc structure */
	(void) strcpy(symbol_name, sym);
	(void) strcat(symbol_name, "_prog");
	nlstend = nlst + filhdr.symbol_total;
	for (nlp = nlst; nlp < nlstend; nlp++)
	{
		if (!strcmp(symbol_name, nlp->name.n_name))
		{
			/* Fill in the symbol and code pointers */
			*symbol = (Prog_desc *) nlp->symbol_value;
			/* hdr.exec_entry contains the address of the called stub for the
			   sym_main function. */
			(*symbol)->func = (int (*) ()) hdr.exec_entry;
			*code = func;

			free(nlst);
			return 0;
		}
	}

	free(func);
	return 6;
}

/*
** Returns the entire symbol table of the given executable.
** If anything goes wrong, NULL is returned.
*/
SYMENT *
 namelist(fd, filhdr, hdr)
int fd;
FILHDR *filhdr;
AOUTHDR *hdr;
{
	register SYMENT *nlst, *nlstend;	/* Name list */
	register SYMENT *nlp;		/* Pointer into list */
	register char *stbl;		/* String table */
	int size;					/* String table size */

	/* Snarf the headers out of the file. */

	lseek(fd, (long) 0, L_SET);
	if ((read(fd, filhdr, FILHSZ)) != FILHSZ)
		return (NULL);

	lseek(fd, filhdr->aux_header_location, L_SET);
	if ((read(fd, hdr, sizeof(*hdr))) != sizeof(*hdr))
		return (NULL);

	/* Allocate a buffer for and read the symbol table */

	nlst = (SYMENT *) malloc(filhdr->symbol_total * SYMESZ);
	lseek(fd, filhdr->symbol_location, L_SET);
	if (read(fd, nlst, filhdr->symbol_total * SYMESZ) != filhdr->symbol_total * SYMESZ)
	{
		free((char *) nlst);
		return (NULL);
	}
	size = filhdr->symbol_strings_size;

	/* Allocate a buffer and read the string table out of the file. */

	stbl = malloc(size);
	lseek(fd, filhdr->symbol_strings_location, L_SET);
	if ((read(fd, stbl, size)) != size)
	{
		free((char *) nlst);
		free(stbl);
		return (NULL);
	}
	/* Rearrange the namelist to point at the character strings rather than
	   have offsets from the start of the string table. */
	nlstend = nlst + filhdr->symbol_total;
	for (nlp = nlst; nlp < nlstend; nlp++)
		nlp->name.n_name = nlp->name.n_strx + stbl;

	/* Return the namelist we just constructed. */
	return (nlst);
}

#endif

#else							/* DYNALOAD */

compile_module(module_name, symbol, code, error_name, output_name)
char *module_name;
char **symbol, **code;
char *output_name, *error_name;
{
	return 1;
}

#endif							/* DYNALOAD */
