/* team.h - stuff having to do with teams */

#ifndef _TEAM_H_
#define _TEAM_H_

#define MAX_TEAMS       7       /* must be < #of bits in an int for
                                   remove_vehicle() to work... */
#define NEUTRAL 0               /* The neutral team */
#define NO_TEAM 255             /* none of the teams */

typedef unsigned char Team;     /* team number */

typedef struct {
    int vehicle_count;		/* how many team members are still in the game
				   (i.e. not permanently dead) */
    int treasury;               /* how much money the team as a whole has (for
                                   EcoWar) */
    int spending;               /* caches small amounts of money teams spent on
                                   supplies, to avoid roundoff errors when
                                   distributing this money to the other teams
                                   */
} TeamData;

extern TeamData teamdata[];
extern Team winning_team;       /* team number of team that has just won */
extern int num_teams;           /* the number of teams with any vehicles left
                                   in play (NB: they are not necessarily the
                                   low-numbered teams!) */
extern Team last_team;          /* the highest-numbered team that's playing,
                                   useful for scanning through the teams */

#endif ndef _TEAM_H_
