/*-
 * Copyright (c) 1988 Terry Donahue
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>		/* for lseek() */

#include "xtank.h"
#include "proto.h"

#ifndef UNIX
#undef DYNALOAD
#undef CHECKNET
#endif

#define MAX_CLIENTS 32

#ifdef UNIX
char *network_error_str[6] =
{
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

int
check_internet(int num_clients, char **client)
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
	if (host == (struct hostent *) 0)
		return 2;

	/* Get the network address of the server */
	(void) bcopy(host->h_addr, server_addr, sizeof(host->h_addr));

	/* Make sure all clients are on same subnet as the server */
	for (i = 0; i < num_clients; i++) {
		if (client[i][0] == '-') {
			if (client[i][1] == 's' || client[i][1] == 'F') {
				i++;
			}
			continue;
		}
		newlen = strlen(client[i]);

		/* Strip off the :x.y from each display name */
		if ((tmp = rindex(client[i], ':')) != (char *) 0)
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
		if (host == (struct hostent *) 0)
			return 4;

		/* Get the network address of the client */
		(void) bcopy(host->h_addr, client_addr[i], sizeof(host->h_addr));

		/* Compare subnet of server with subnet of client */
		if (server_addr[1] != client_addr[i][1])
			return 5;
	}
	return 0;
}

#else /* CHECKNET */

/*ARGSUSED*/
int
check_internet(int num_clients, char **client)
{
	return 0;
}

#endif /* CHECKNET */

/*
** UNIX Dynamic loading module
*/

#ifdef DYNALOAD
/* Dynamic loading includes */

#ifdef USE_DLOPEN
# include <dlfcn.h>
#else /* USE_DLOPEN */
# ifdef mips
#  include <sys/exec.h>
#  include <nlist.h>
# else
#  if defined(sun) && defined(SVR4)
#   include <sys/exechdr.h>
#   include <fcntl.h>
#   include <nlist.h>
#  else
#   include <a.out.h>
#  endif
# endif
#endif /* USE_DLOPEN */

#include <sys/param.h>

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

#if !defined(hp9000s800) && !defined(USE_DLOPEN)
compile_module(module_name, symbol, code, error_name, output_name)
char *module_name;
char **symbol, **code;
char *output_name, *error_name;
{
	extern char *executable_name, pathname[], programsdir[];
	extern char headersdir[];
	char *func, *func1, command[1024], address[20], symbol_name[256], *p;

#ifdef mips
	struct nlist nl[3];

#endif

#ifndef mips
	register struct nlist *nlst, *nlp, *nlstend;
	struct nlist *namelist();

#else
	char code_name[256];

#endif

#ifdef DEBUG
	unsigned long c;

#endif

	int fd;

#ifdef sgi
	  struct {
		  struct filehdr ex_f;
		  struct aouthdr ex_o;
	  } hdr;

#else
	struct exec hdr;

#endif

	/* Come up with some space for the module on a page boundary */
	func = (char *) malloc(CODE_SIZE);
	func1 = (char *) ((((int) func) + 2047) & ~0x7ff);
	sprintf(address, "%x", func1);

#ifdef DEBUG
	puts(address);
#endif

	/* Chop off everything from the . on to make the rest easier */
	if (p = rindex(module_name, '.'))
		*p = '\0';
	else {
		free(func);
		return 1;
	}

	/* Parse the module name to see if it is a .o or a .c */
	switch (*(p + 1)) {
	  case 'c':
		  /* Do the compile */
		  sprintf(command,
#ifdef mips
				  "cd %s/%s; cc %s -c -G 0 -O -I%s -o %s.o %s.c > %s 2>&1",
#else
				  "cd %s/%s; cc %s -c -O -I%s -o %s.o %s.c > %s 2>&1",
#endif
				  pathname, programsdir, ALLDEFINES, headersdir, module_name,
				  module_name, error_name);

#ifdef DEBUG
		  printf("compile command: %s\n", command);
#endif

		  if (system(command)) {
			  free(func);
			  return 2;
		  }
		  /* Note the lack of a break; here */
	  case 'o':
		  /* Do the link */
		  sprintf(command,
				  "ld -x -N -A %s -T %s -o %s %s.o -lm -lc > %s 2>&1",
				  executable_name, address, output_name, module_name,
				  error_name);

#ifdef DEBUG2
		  printf("link command: %s\n", command);
#endif

		  if (system(command)) {
			  free(func);
			  return 3;
		  }
		  break;
	  default:
		  free(func);
		  return 1;
	}

	/* Now find the symbol in the symbol table */
	if ((fd = open(output_name, O_RDONLY)) < 0) {
		free(func);
		return 4;
	}
#ifndef mips
	nlst = namelist(fd, &hdr);
	if (nlst == (struct nlist *) NULL)
#else
	/* Read in exec header from object file */
	lseek(fd, (long) 0, L_SET);
	if ((read(fd, &hdr, sizeof(hdr))) != sizeof(hdr))
#endif

	{
		free(func);
		return 5;
	}
#ifdef DEBUG
#ifndef mips
#ifdef linux
	printf("Magic number: %o\n", N_MAGIC(hdr));
#else
	printf("Magic number: %o\n", hdr.a_magic);
#endif
	printf("Text          %d\n", hdr.a_text);
	printf("Data          %d\n", hdr.a_data);
	printf("BSS           %d\n", hdr.a_bss);
	printf("Offset: %d\n", N_TXTOFF(hdr));
#else
	printf("Magic number: %o\n", hdr.ex_o.magic);
	printf("Text          %d\n", hdr.ex_o.tsize);
	printf("Data          %d\n", hdr.ex_o.dsize);
	printf("BSS           %d\n", hdr.ex_o.bsize);
	printf("Offset: %d\n", N_TXTOFF(hdr.ex_f, hdr.ex_o));
#endif
#endif

#ifndef mips
	lseek(fd, (off_t) N_TXTOFF(hdr), L_SET);
	read(fd, func1, (int) (hdr.a_text + hdr.a_data));
#else
	lseek(fd, (off_t) N_TXTOFF(hdr.ex_f, hdr.ex_o), L_SET);
	read(fd, func1, (int) (hdr.ex_o.tsize + hdr.ex_o.dsize));
#endif

#ifdef DEBUG
#ifndef mips
	for (p = func1, c = hdr.a_text + hdr.a_data; c; --c, ++p)
		printf("%x %x\n", p, *p);
	printf("Found it at location %x\n", func1);
#else
	for (p = func1, c = hdr.ex_o.tsize + hdr.ex_o.dsize; c; --c, ++p)
		printf("%x %x\n", p, *p);
	printf("Found it at location %x\n", func1);
#endif
#endif

	/* Come up with the entry symbol for the Prog_desc structure */
	if ((p = rindex(module_name, '/') + 1) == (char *) 1)
		p = module_name;

	(void) strcpy(symbol_name, "_");

#ifdef mips
	(void) strcpy(symbol_name, p);
#else
	(void) strcat(symbol_name, p);
#endif

	(void) strcat(symbol_name, "_prog");

#ifndef mips
	nlstend = nlst + (hdr.a_syms / sizeof(struct nlist));

	for (nlp = nlst; nlp < nlstend; nlp++) {
#if !defined(sun) || !defined(SVR4)
		if (!strcmp(symbol_name, nlp->n_un.n_name)) {
#else
		if (!strcmp(symbol_name, nlp->n_name)) {
#endif
			/* Fill in the symbol and code pointers */
			*symbol = (char *) nlp->n_value;
			*code = func;
			return 0;
		}
	}

#else
	(void) strcpy(code_name, p);
	(void) strcat(code_name, "_main");
	nl[0].n_name = symbol_name;
	nl[1].n_name = code_name;
	nl[2].n_name = NULL;
	nlist(output_name, nl);
	if (nl[0].n_type != 0 || nl[1].n_type != 0) {
		*symbol = (char *) nl[0].n_value;
		*code = (char *) nl[1].n_value;
		return 0;
	}
#endif


	free(func);
	return 6;
}

#ifndef mips
/*
** Returns the entire symbol table of the given executable.
** If anything goes wrong, NULL is returned.
*/
struct nlist *namelist(fd, hdr)
int fd;							/* (seekable) File descriptor of executable  */
struct exec *hdr;				/* Pointer to exec struct which is filled in */
{
	register struct nlist *nlst, *nlstend;	/* Name list */
	register struct nlist *nlp;	/* Pointer into list */
	register char *stbl;		/* String table */
	int size;					/* String table size */

	/* Snarf the header out of the file. */

	lseek(fd, (long) 0, L_SET);
	if ((read(fd, (char *) hdr, sizeof(*hdr))) != sizeof(*hdr))
		return ((struct nlist *) 0);

	/* Allocate a buffer for and read the symbol table */

	nlst = (struct nlist *) malloc((unsigned) hdr->a_syms);
	lseek(fd, (long) N_SYMOFF(*hdr), L_SET);
	if ((read(fd, (char *) nlst, (int) hdr->a_syms)) != hdr->a_syms) {
		free((char *) nlst);
		return ((struct nlist *) 0);
	}
	/* Now, read the string table size. */

	lseek(fd, (long) N_STROFF(*hdr), L_SET);
	if ((read(fd, (char *) &size, sizeof(int))) != sizeof(int)) {
		free((char *) nlst);
		return ((struct nlist *) 0);
	}
	/* Allocate a buffer and read the string table out of the file. */

	stbl = malloc((unsigned) size);
	lseek(fd, (long) N_STROFF(*hdr), L_SET);
	if ((read(fd, stbl, size)) != size) {
		free((char *) nlst);
		free(stbl);
		return ((struct nlist *) 0);
	}
	/* Rearrange the namelist to point at the character strings rather */
	/* than have offsets from the start of the string table. */
	nlstend = nlst + (hdr->a_syms / sizeof(struct nlist));

	for (nlp = nlst; nlp < nlstend; nlp++)
#if !defined(sun) || !defined(SVR4)
		nlp->n_un.n_name = nlp->n_un.n_strx + stbl;
#else
		nlp->n_name = nlp->n_numaux + stbl;
#endif

	/* Return the namelist we just constructed. */
	return (nlst);
}
#endif /* ifndef mips */

#elif defined(USE_DLOPEN)

int
compile_module(char *module_name, char **symbol, char **code, char *error_name, char *output_name)
{
	extern char *executable_name, pathname[], programsdir[];
	extern char headersdir[];
	char *func, *func1, command[1024], address[20], symbol_name[256], *p;
	char shlib_path[256];
	void *dl_handle;

	/* Chop off everything from the . on to make the rest easier */
	if ((p = rindex(module_name, '.'))) {
		*p = '\0';
	} else {
		return 1;
	}

	/* Parse the module name to see if it is a .o or a .c */
	switch (*(p + 1)) {
	case 'c':
		/* Do the compile */
		sprintf(command,
		"cd %s/%s; cc %s -c -O -w -I%s -o %s.o %s.c > %s 2>&1",
		pathname, programsdir, ALLDEFINES, headersdir, module_name,
		module_name, error_name);

#ifdef DEBUG
		printf("compile command: %s\n", command);
#endif

		if (system(command)) {
			return 2;
		}
		/* Note the lack of a break; here */
	case 'o':
		/* Do the link */
		sprintf(command,
		"ld -o %s -expect_unresolved '*' -shared -all %s.o -lm -lc > %s 2>&1",
		output_name, module_name,error_name);
#ifdef DEBUG
		printf("link command: %s\n", command);
#endif

		if (system(command)) {
			return 3;
		}
		break;
	default:
		return 1;
	}

	/* Now find the symbol in the symbol table */

	/* Add Prog dir to shlib search path */
	(void) strcpy(shlib_path,"LD_LIBRARY_PATH=");
	(void) strcat(shlib_path,pathname);
	(void) strcat(shlib_path,"/");
	(void) strcat(shlib_path,programsdir);
	putenv(shlib_path);

	if((dl_handle = dlopen(output_name,RTLD_NOW))==NULL) {
#ifdef DEBUG
		printf("dlopen: %s\n",dlerror());
#endif
		return 4;
	}
#ifdef DEBUG
	printf("dlopen called\n");
#endif
	/* Come up with the entry symbol for the Prog_desc structure */
	if ((p = rindex(module_name, '/') + 1) == (char *) 1) {
		p = module_name;
	}
	(void) strcpy(symbol_name, p);
	(void) strcat(symbol_name, "_prog");
	*symbol = (char *) dlsym(dl_handle,symbol_name);
	if (*symbol == NULL) {
#ifdef DEBUG
		printf("%s\n",dlerror());
#endif
		return 5;
	}
	*code = (char *)((Prog_desc *) *symbol)->func;

#ifdef DEBUG
	printf("*symbol = %x for %s\n",*symbol,symbol_name);
	printf("*code = %x for %s_main\n",*code,p);
#endif
	if (*code != NULL) {
		return 0;
	}
	dlclose(dl_handle);
	return 6;
}

#else /* hp9000s800 */

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
	func = (char *) malloc(CODE_SIZE);
	func1 = (char *) ((((int) func) + (NBPG - 1)) & ~(NBPG - 1));
	sprintf(address, "%x", func1);

#ifdef DEBUG
	puts(address);
#endif

	/* Parse the module name to see if it is a .o or a .c */
	switch (*(p + 1)) {
	  case 'c':
		  /* Do the compile */
		  sprintf(command, "cd %s/%s; cc -c -O -I. %s.c > %s 2>&1",
				  pathname, programsdir, module_name, error_name);

#ifdef DEBUG
		  printf("compile command: %s\n", command);
#endif

		  if (system(command)) {
			  free(func);
			  return 2;
		  }
	  case 'o':
		  /* Do the link */
#ifdef __hpux
		  sprintf(command, "/bin/ld -x -a archive -A %s -R %s -e %s_main -o %s %s.o -lm -lc > %s 2>&1",
#else
		  sprintf(command, "ld -x -N -A %s -R %s -e %s_main -o %s %s.o -lm -lc > %s 2>&1",
#endif
				  executable_name, address, sym, output_name, module_name, error_name);

#ifdef DEBUG
		  printf("link command: %s\n", command);
#endif

		  if (system(command)) {
			  free(func);
			  return 3;
		  }
		  break;
	  default:
		  free(func);
		  return 1;
	}

	/* Find the entry point (foo_main function) */
	if ((fd = open(output_name, O_RDONLY)) < 0) {
		free(func);
		return 4;
	}
	nlst = namelist(fd, &filhdr, &hdr);
	if (nlst == (struct nlist *) 0) {
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
	for (nlp = nlst; nlp < nlstend; nlp++) {
		if (!strcmp(symbol_name, nlp->name.n_name)) {
			/* Fill in the symbol and code pointers */
			*symbol = (Prog_desc *) nlp->symbol_value;
			/* hdr.exec_entry contains the address of the called stub for the
	       sym_main function. */
			(*symbol)->func = (int (*)()) hdr.exec_entry;
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
	if ((read(fd, filhdr, FILHSZ)) != FILHSZ) {
		return ((SYMENT *) 0);
	}
	lseek(fd, filhdr->aux_header_location, L_SET);
	if ((read(fd, hdr, sizeof(*hdr))) != sizeof(*hdr)) {
		return ((SYMENT *) 0);
	}
	/* Allocate a buffer for and read the symbol table */

	nlst = (SYMENT *) malloc(filhdr->symbol_total * SYMESZ);
	lseek(fd, filhdr->symbol_location, L_SET);
	if (read(fd, nlst, filhdr->symbol_total * SYMESZ) != filhdr->symbol_total * SYMESZ) {
		free((char *) nlst);
		return ((SYMENT *) 0);
	}
	size = filhdr->symbol_strings_size;

	/* Allocate a buffer and read the string table out of the file. */

	stbl = malloc(size);
	lseek(fd, filhdr->symbol_strings_location, L_SET);
	if ((read(fd, stbl, size)) != size) {
		free((char *) nlst);
		free(stbl);
		return ((SYMENT *) 0);
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

#else /* DYNALOAD */

int
compile_module(char *module_name, char **symbol, char **code, char *error_name, char *output_name)
{
	return 1;
}

#endif /* DYNALOAD */
