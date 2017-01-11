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

#ifndef _TEAM_H_
#define _TEAM_H_

#define MAX_TEAMS       7		/* must be < #of bits in an int for
                                   remove_vehicle() to work... */
#define NEUTRAL 0				/* The neutral team */
#define NO_TEAM 255				/* none of the teams */

typedef unsigned char Team;		/* team number */

  typedef struct {
	  int vehicle_count;		/* how many team members are still in the game
				   (i.e. not permanently dead) */
	  int treasury;				/* how much money the team as a whole has (for
                                   EcoWar) */
	  int spending;				/* caches small amounts of money teams spent on
                                   supplies, to avoid roundoff errors when
                                   distributing this money to the other teams
                                   */
  }
TeamData;

extern TeamData teamdata[];
extern Team winning_team;		/* team number of team that has just won */
extern int num_teams;			/* the number of teams with any vehicles left
                                   in play (NB: they are not necessarily the
                                   low-numbered teams!) */
extern Team last_team;			/* the highest-numbered team that's playing,
                                   useful for scanning through the teams */

#endif /* ndef _TEAM_H_ */
