#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef SUN40
#include <sys/filio.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef hpux
#include <time.h>
#else							/* hpux */
#include <sys/time.h>
#endif							/* hpux */

#include <signal.h>
#include <errno.h>
#include "defs.h"
#include "data.h"
#include "ipc.h"

#define RESSTR "Sorry, but playing XTREK is not permitted now.\nTry later\n"

typedef short day[48];
typedef day resth[7];

extern int peerdied;
extern resth hours;

extern void deadpeer();

int startagame(ns, ofdset, pno, pn)
int ns;							/* "new" socket */
fd_set *ofdset;
int pno, *pn;
{
	int addrlen;
	struct sockaddr addr;
	int si;
	int on;
	char sbuf[2048];
	int rtime;
	struct tm *ltime;
	char new_display[50], new_login[50], proto_str[50];
	char mode[5], pns[5];
	char *temp;
	int itmp;

	printf("They made it in... %d\n", ns);		/* DB */
	addrlen = sizeof(addr);
	errno = 0;
	if (ns == -1)
	{
		ns = accept(xtrek_socket, &addr, &addrlen);
		if (ns < 0)
		{
			perror("Startagame's accept");
			errno = 0;
			FD_CLR(xtrek_socket, ofdset);
			return (-1);
		}
		si = 0;
		peerdied = 0;
		signal(SIGPIPE, deadpeer);
		ioctl(ns, FIONBIO, &on);
		FD_SET(ns, ofdset);
	}
	si = read(ns, sbuf, 2048);
	if (si == 0)
	{
		peerdied = 1;
		return (-1);
	}
	if (si < 0)
	{
		perror("read, in startagame\n");
		return (-1);
	}
	if (!(si >= 2 && sbuf[si - 2] == '\015' && sbuf[si - 1] == '\012'))
	{
		perror("Bad shit in startgame...");
		sbuf[si] = '\0';
		printf("si = %d, sbuf = \"%s\"\n", si, sbuf);
		return (-1);
	}
	sbuf[si - 2] = '\0';
	rtime = time(0);
	ltime = localtime(&rtime);
	itmp = sscanf(sbuf,
				  "Version: %s Display: %s Login: %s Mode: %s PN: %s",
				  proto_str, new_display, new_login, mode, pns);
	if (itmp != 5)
	{
		write(ns, "Bad format\n", 11);
		FD_CLR(ns, ofdset);
		close(ns);
		ns = -1;
		return (-1);
	}
	if (atoi(proto_str) != PROTVER)
	{
		char tmp[BUFSIZE];

		sprintf("Wrong protocol version, useing %s need %d.  %s\n",
				proto_str, PROTVER, "Get a new front end!");
		write(ns, tmp, strlen(tmp) + 1);
		FD_CLR(ns, ofdset);
		close(ns);
		ns = -1;
		return (-1);
	}
	if ((hours[ltime->tm_wday][ltime->tm_hour * 2 + ltime->tm_min / 30] != 0) &&
			strcmp(new_login, "XTREKGOD"))
	{
		write(ns, RESSTR, strlen(RESSTR));
		FD_CLR(ns, ofdset);
		close(ns);
		ns = -1;
		return (-1);
	}
	*pn = pno = findslot();
	if (pno == MAXPLAYER)
	{
		write(ns, "No more room in game\n", 21);
		FD_CLR(ns, ofdset);
		close(ns);
		ns = -1;
		return (-1);
	}
	if (!atoi(mode))
		if (getships(players + pno, sbuf, ns, ofdset))
			return (-1);
	write(ns, "Creating connection on ", 23);
	write(ns, new_display, strlen(new_display));
	write(ns, "\015\012", 2);	/* just to make sure */
	printf("They are at the switch (mode=%s)\n", mode);	/* DB */
	switch (atoi(mode))
	{
		case (1):
			if (enter_watch(&players[pno], atoi(pns), ns))
			{
				FD_CLR(ns, ofdset);
				close(ns);
				ns = -1;
				players[pno].p_status = PFREE;
				return (-1);
			}
		case (2):
			if (enter_copilot(&players[pno], atoi(pns), ns))
			{
				FD_CLR(ns, ofdset);
				close(ns);
				ns = -1;
				players[pno].p_status = PFREE;
				return (-1);
			}
		default:
			players[pno].copilot = -1;
			players[pno].watch = -1;
	}
	if (ns != -1)
	{
		strcpy(players[pno].p_monitor, new_display);
		strcpy(players[pno].p_login, new_login);
		players[pno].p_status = PSETUP;
		sprintf(sbuf, " as player number %d\n", pno);
		write(ns, sbuf, strlen(sbuf));
		sprintf(sbuf, " with ship: %s\n", players[pno].p_shipchars[0]);
		for (temp = sbuf; *temp != '\0'; *temp++)
			if (*temp == '$')
				*temp = ' ';
		write(ns, sbuf, strlen(sbuf));
	}
	return (ns);
}

RecieveLotsaPixmaps(ns, p)
int ns;
struct player *p;
{
	int i, j, k;
	long tmp;
	int ci = 0;
	long *xt;
	Atom atr;
	int afr;
	unsigned long nir, bar;

	printf("Getting lots of pixmaps\n");		/* DB */
	XGetWindowProperty(p->display, DefaultRootWindow(p->display), XInternAtom(p->display, "XTREK_PIXMAPS", 0),
					   0, XTNUMPM + 1, False, XA_PIXMAP, &atr, &afr, &nir,
					   &bar, &xt);
	if (xt[ci++] != PROTVER)
		printf("Yow, this shouldn't happen!\n. Pixmap proto mismatch\n");
	if (afr == 0)
		printf("Format Zero\n");
	else if (nir == 0)
		printf("Numitems Zero\n");
	for (i = 0; i < EX_FRAMES; i++)
	{
		p->expview[i] = xt[ci++];
	}
	for (i = 0; i < VIEWS; i++)
		for (j = 0; j < 2; j++)
			for (k = 0; k < 4; k++)
			{
				p->kliview[i][j][k] = xt[ci++];
				p->fedview[i][j][k] = xt[ci++];
				p->oriview[i][j][k] = xt[ci++];
				p->romview[i][j][k] = xt[ci++];
			}
	p->cloud = xt[ci++];
	p->etorp = xt[ci++];
	p->mtorp = xt[ci++];
	for (i = 0; i < 5; i++)
	{
		p->bplanet[0][i] = xt[ci++];
		p->mbplanet[0][i] = xt[ci++];
	}
	XFree(xt);
	printf("Server got Pixmaps\n");		/* DB */
}

int getships(p, sbuf, ns, ofdset)
int ns;
fd_set *ofdset;
struct player *p;
char *sbuf;
{
	int i;
	FILE *err;
	struct ship fake_shipp;
	char *cp, *sp;

	err = fdopen(ns, "w");
	setbuf(err, NULL);			/* Unbuffered! */
	for (sp = sbuf; *sp != '\01'; sp++)	/* None */
		;
	cp = ++sp;
	for (i = 0; i < 6; i++)
	{
		for (; *cp != ':'; cp++)/* None */
			;
		cp++;
		for (sp = ++cp; *cp != ' '; cp++)		/* None */
			;
		*cp++ = '\0';
		strcpy(p->p_shipname[i], sp);
		for (; *cp != ':'; cp++)/* None */
			;
		cp++;
		for (sp = ++cp; *cp != '\0' && *cp != ' '; cp++)		/* None */
			;
		*cp++ = '\0';
		strcpy(p->p_shipchars[i], sp);
		if (getship(&fake_shipp, sp, err))
		{
			printf("Kicking the bastard out\n");		/* DB */
			fprintf(err, "Ship %s is invalid\n", p->p_shipname[i]);
			fprintf(err, "Go away, you stupid cheating moron.\n");
			FD_CLR(ns, ofdset);
			close(ns);
			ns = -1;
			return (-1);
		}
	}
	return (0);
}
