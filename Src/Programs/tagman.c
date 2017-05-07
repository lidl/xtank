/*-
 * Copyright (c) 1992 Matt Senft
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

#define MARGIN_OF_ERROR  59
#define ALWAYS_OLD_AGE   450
#define HOLD_TIME        4
#define MINE_RANGE       80
#define MIN_OPEN_SPACING 4
#define MIN_CLOSING_SPEED_REL 1.0
#define MIN_CLOSING_SPEED_ABS 4.0
#define DIRECT_SHOT_DISTANCE MARGIN_OF_ERROR
#define SPEED_BIAS 128
#define MAX_STILLS MAX_VEHICLES
#define LATE_FC          6

#define HAS_TURRET4

#include "xtanklib.h"
#include "special.h"
#include "xtank.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#include "assert.h"

#ifndef FLOAT
#define FLOAT float
#endif

#ifndef DOUBLE
#define DOUBLE double
#endif

#ifdef SECOND_COPY
#define PROGRAM_NAME  "Tagman2"
#define FILE_PROG     tagman2_prog
#else
#define PROGRAM_NAME  "Tagman"
#define FILE_PROG     tagman_prog
#endif


/*****************************************************************************/
/*                                                                           */
/* BAD THINGS WE ASSUME  (You now what they say about assuming things!?!)    */
/*                                                                           */
/* 1)  Vehicles numbers are from 0 to MAX_VEHICLES-1                         */
/* 2)  MOUNT_TURRETn == n-1 for (n = 1; n < MAX_TURRETS; n++)                */
/* 3)  The neutral team number is 0                                          */
/* 4)  You're reasonably familiar with C(just a jest :))                     */
/*                                                                           */
/*****************************************************************************/

typedef int Frame;
typedef int Coor;
typedef int Speed;

/* the maximum possible number of boxes in the map */
#define MAX_BOXES	(GRID_WIDTH * GRID_HEIGHT)

/* how deep to search for short cuts in the navigator's route */
#define SHORT_CUT_DEPTH	5

#define map_wall_north(x,y) map_north_result(pEnv->my_map[x][y].flags)
#define map_wall_south(x,y) map_north_result(pEnv->my_map[x][y+1].flags)
#define map_wall_east(x,y)  map_west_result(pEnv->my_map[x+1][y].flags)
#define map_wall_west(x,y)  map_west_result(pEnv->my_map[x][y].flags)


/* Xtank's enum of "WallSide" used to be no good, as it started */
/* with "NO_DIR" as -1.  And you can't use that as an array index. */
typedef enum
{
	North = NORTH,
	East  = EAST,
	South = SOUTH,
	West  = WEST,
	Nodir = (WEST+1)
}
Direction;

/* information I may want to keep on a on a map box */
typedef struct
{
	Boolean         seen;	/* iff we have ever seen this box */
	Direction       navdir;	/* direction (used by navigator)  */
}
BoxNotes;

/*********************************************************************/
/* The following variables are essentially constants, thus can be    */
/* shared between vehicles, thus can be global.  They are static so  */
/* that there are no namespace conflicts with the rest of Xtank.     */
/*********************************************************************/

static int  aiDamage[] = { 2, 3, 4, 2, 3, 4, 3, 4, 5,
			     4, 6, 8, 6, 3, 6, 8, 8, 8,
			     4, 6, 8, 6, 3, 6, 8, 8, 8};

static int PercentOfLandmark[] =
{
    25,    /* Lightcycle */
    30,    /* Hexo       */
    30,    /* Spider     */
    40,    /* Psycho     */
    40,    /* Tornado    */
    40,    /* Marauder   */
    50,    /* Tiger      */
    50,    /* Rhino      */
    50,    /* Medusa     */
    50,    /* Malice     */
    25,    /* Trike      */
   100,    /* Panzy      */
	50,
	50,
	50,
	50,
	50,
	50,
	50,
	50,
	50
};

static int MaxBodySide[] =
{
    32,    /* Lightcycle */
    39,    /* Hexo       */
    44,    /* Spider     */
    58,    /* Psycho     */
    57,    /* Tornado    */
    57,    /* Marauder   */
    73,    /* Tiger      */
    83,    /* Rhino      */
    64,    /* Medusa     */
    44,    /* Malice     */
    36,    /* Trike      */
   112,    /* Panzy      */
    50,
    50,
    50,
    50,
    50,
    50,
    50,
    50,
    50
};

#ifdef UNUSED
/* convert a direction into a heading angle */
static Angle    dir2angle[] =
{
	3 * PI / 2,		/* North */
	0,			/* East  */
	PI / 2,			/* South */
	PI,			/* West  */
	0,			/* Nodir */
};
#endif

/* convert a direction into differences in x and y coordinates */
static Coord    dir2delta[] =
{
	{ 0, -1},		/* North */
	{ 1,  0},		/* East  */
	{ 0,  1},		/* South */
	{-1,  0},		/* West  */
	{ 0,  0},		/* Nodir */
};

/*
 * convert a direction into the pixel coordinates of the side of the box that
 * will be the exit
 */
static Coord    dir2exit[] =
{
	{BOX_WIDTH / 2, 1},	                /* North (top center)    */
	{BOX_WIDTH - 1, BOX_HEIGHT / 2},	/* East  (right center)  */
	{BOX_WIDTH / 2, BOX_HEIGHT - 1},	/* South (bottom center) */
	{1,             BOX_HEIGHT / 2},	/* West  (left center)   */
	{BOX_WIDTH / 2, BOX_HEIGHT / 2},	/* Nodir (center center) */
};

/* convert a direction to its opposite */
static Direction dir2opposite[] =
{
	South,			/* [North] */
	West,			/* [East]  */
	North,			/* [South] */
	East,			/* [West]  */
	Nodir,			/* [Nodir] */
};


/* convert a direction to its adjacent direction */
static Direction dir2clockwise[] =
{
	East,			/* [North] */
	South,			/* [East]  */
	West,			/* [South] */
	North,			/* [West]  */
	Nodir,			/* [Nodir] */
};


/* convert a direction to its adjacent direction */
static Direction dir2counterclockwise[] =
{
	West,			/* [North] */
	North,			/* [East]  */
	East,			/* [South] */
	South,			/* [West]  */
	Nodir,			/* [Nodir] */
};

typedef enum
{
	CURRENT,
	CLOSEST
}
Search_mode;

typedef enum
{
	ENGAGE,
	DIS_ENGAGE,
	WANDER,
	REPLENISH,
	GOTO,
	SEEK,
	REPAIRING,
	RAMMING,
	RESTART,
	max_statuses
}
Status_code;

typedef struct
{
	Angle aRate;
	int	  iAvgAmmoSpeed;
	int	  iAvgAmmoLifetime;
	int   iNumWeapons;
	int   iMinReload;
#ifdef CHECK_LAST_FIRE
	int   iLastFire;
#endif
}
TurretInfo;


typedef struct
{
    Status_code   iStatus;
	int           iGoal;
#ifdef DEBUG_TARGET_SOLUTIONS
	int           iCalls;
	int           iZeros;
	int           iIterations;
	int           iClearPathCalls;
	int           iClearPaths;
	int           iBetters;
#endif
	Location      stPrevLoc;	/* previous location */
	Location      prev_loc;		/* previous location */
	Boolean       have_route;	/* iff the navigator found a route */
	Boolean       bGoalLocated;	/* iff we have ever seen our goal */
	Box (*my_map)[GRID_HEIGHT];	/* my personal map of the world (provided by
				                   Xtank itself) */
	BoxNotes boxnotes[GRID_WIDTH][GRID_HEIGHT];	/* my notes on each map box */
	Coord         dest_wall;    /* box coordinates of the first destructible
				                   wall in our path */
	Frame         frCurrent;
	Frame         frPrevious;
	Frame         frHoldAim;
	Frame         frLastOpen;
	Frame         frLastEnemyAt;
	int           iTimeOut;
	int           iFriendliesInTheArea;
	int           iWasCalled;
	int           iCallerId;
	Coor          iDestX;
	Coor          iDestY;
	int           iSpecTarget;
	int           iTargetVehicle;
	Settings_info stGameSettings;
	Vehicle_info  stMyVehicle;
	int           iNumWeapons;
	Weapon_info   stMyWeapons[MAX_WEAPONS];
	int           iNumVehicles;
	int           iTurnDir;
	float         fDriveSpeed;
	float         fDistance;
	int           iDistance;
	float         fClosePercent;
	Angle         aTurn;
	int           iStills;
	int           aiStills[MAX_STILLS][4];
	int           iAvgAmmoRange;
	TurretInfo    Turret[MAX_TURRETS];
	Vehicle_info  stVehicles[MAX_VEHICLES];
	Byte          aabIntendedLoc[MAX_VEHICLES][((MAX_DATA_LEN + 1)>>1)<<1];
	Frame         afFromFrame[MAX_VEHICLES];
	Boolean       bHaveTarget;    /* whether I have something to shoot */
	Boolean       bAlwaysRam;
	Boolean       bTargetIsVehicle; /* whether the target is a vehicle   */
	Boolean       bLayMines;
	Boolean       bHaveNewRadar;
	Boolean       bHaveRadar;
	Boolean       bHaveTacLink;
	Vehicle_info  target;	    /* who I'm going to attack */
	Frame         fTargetTime;
	Vehicle_info  prevTarget;
	Frame         fprevTime;
}
Environment;


#if defined(__STDC__) || defined(__cplusplus)
# define PROTO(s) s
#else
# define PROTO(s) ()
#endif

/* tagman.c */
static int HuntAndKill PROTO((Environment *pEnv));
static int Fight PROTO((Environment *pEnv, int bShootAlways));
static int TargetSolution PROTO((Environment *pEnv, Speed xvUs, Speed yvUs, Coor xUs, Coor yUs, Speed xvTarget, Speed yvTarget, Coor xTarget, Coor yTarget, int iErrorMargin, int iAmmoLifetime, int iAmmoSpeed, Angle aCurrent, Angle aRange, Angle *pBestAngle));
static void cleanup PROTO((Environment *pEnv));
static void StartGoto PROTO((Environment *pEnv));
static void StartSeek PROTO((Environment *pEnv));
static void StartEngage PROTO((Environment *pEnv));
static void StartWandering PROTO((Environment *pEnv));
static void GetBasics PROTO((Environment *pEnv));
static void Acquire PROTO((Environment *pEnv, Search_mode what));
static int IsApproaching PROTO((int xThem, int yThem, int xvThem, int yvThem, int xUs, int yUs));
static int IsClosing PROTO((int xUs, int yUs, int xvUs, int yvUs, int xThem, int yThem, int xvThem, int yvThem));
static void Move PROTO((Environment *pEnv));
static Location *DeltaLoc PROTO((Location *locp, int delta_x, int delta_y));
static int MyClearPath PROTO((Location *lStart, Location *locp));
static int InitializeVariables PROTO((Environment **ppEnvironment));
static int unseen_box PROTO((Environment *pEnv, Coor coX, Coor coY));
static FLOAT frand PROTO((FLOAT maxr));
static void TurnOffAllWeapons PROTO((void));
static void TurnOnAllWeapons PROTO((void));
static int NeedFuel PROTO((DOUBLE fraction));
static int NeedArmor PROTO((DOUBLE fraction));
static int NeedAmmo PROTO((DOUBLE fraction));
static int AtBoxCenter PROTO((Vehicle_info *selfp));
static int GotoBoxCenter PROTO((Environment *pEnv));
static int HandleOutpost PROTO((Environment *pEnv));
static int HandleLocalLandMark PROTO((Environment *pEnv));
static int VectorsCross PROTO((Coor x1, Coor y1, DOUBLE xs1, DOUBLE ys1, Coor x2, Coor y2, DOUBLE xs2, DOUBLE ys2));
static void ProcessMessages PROTO((Environment *pEnv, int iMaxProcess));
static int CanGo PROTO((Vehicle_info *pstMyVehicle, Coor xOff, Coor yOff));
static int self_clear_path PROTO((Location *start, int delta_x, int delta_y));
static int IsGoalBox PROTO((Environment *pEnv, int x, int y));
static void NoticeLandmark PROTO((Environment *pEnv, int x, int y));
static void NoteSurroundings PROTO((Environment *pEnv));
static int navigate PROTO((Environment *pEnv, int (*destfunc )(), int through_unseen));
static Angle recursive_short_cut PROTO((Environment *pEnv, int gx, int gy, int depth));
static void FollowRoute PROTO((Environment *pEnv));
#ifdef DEBUG_TIME
static void check_clock PROTO((Environment *pEnv));
#else
#define check_clock(A)
#endif
static void HandleWeapons PROTO((Environment *pEnv));
static void PositionTurrets PROTO((Environment *pEnv));
static void MainLoop PROTO((Environment *pEnv));
static void TagmanMain PROTO((void));
static int FriendliesReplenishing PROTO((Environment *pEnv, Location *pLoc));
static void PositionTurret PROTO((Environment *pEnv, int iTurret));
static void Frantic PROTO((Environment *pEnv));
static void BarfBarf PROTO((Byte, ...));
static void VehicleInfo2Message PROTO((Vehicle_info *pstVehicle, Byte *pbBuffer));
static void Message2VehicleInfo PROTO((Byte *pbBuffer, Vehicle_info *pstVehicle));
static FLOAT GetDodgeSpeed PROTO((Environment *pEnv, FLOAT fDefault));
static void SendEnemyAt PROTO((Environment *pEnv, Vehicle_info *pstVehicle, Boolean bSpace));
static int OutOfAmmo PROTO((Boolean bCountDroppedWeapons));
static int MyVehicleIsClosest PROTO((Environment *pEnv));
static void FindEnemy PROTO((Environment *pEnv));
static int MyGetBlips PROTO((Environment *pEnv, int *piNumBlips, Blip_info *abiBlips));

#undef PROTO

Prog_desc FILE_PROG =
{
    PROGRAM_NAME,
    "Disgorge",
    "Uses relative fire and team tactics to distribute quality poundage to \nits opponents. When it cannot fire upon opponents it uses RacerX modified code\ncare of RPotter.\nShould be placed in a vehicle with some turret-mounted weapons \nand a mapper. \nIt works better when all weapons on the turret are the same.\n",
    "Matthew T. Senft(senft@pix.net)",
    PLAYS_COMBAT | PLAYS_RACE | DOES_EXPLORE | DOES_SHOOT
	| USES_TEAMS | USES_MINES | USES_SLICKS | USES_SIDE_MOUNTS | USES_MESSAGES,
    4,
    TagmanMain
};


static int
HuntAndKill(Environment *pEnv)
{
    int iRetCode;

    if (!Fight(pEnv, FALSE))
	{
		iRetCode = FALSE;

        if (!MyClearPath(&pEnv->stMyVehicle.loc, &pEnv->target.loc))
		{
	        pEnv->iDestX = pEnv->target.loc.grid_x;
    	    pEnv->iDestY = pEnv->target.loc.grid_y;

	        StartGoto(pEnv);

			pEnv->iSpecTarget    = TRUE;
			pEnv->iTargetVehicle = pEnv->target.id;
			pEnv->iWasCalled     = TRUE;
			pEnv->iCallerId      = pEnv->stMyVehicle.id;
		}
	}
	else
	{
		iRetCode = TRUE;
	}

	return (iRetCode);
}

static int Fight(Environment *pEnv, int bShootAlways)
{
	int    bShotAtTarget = FALSE;
	int    bTurretCanHit[MAX_TURRETS];
	int    iCtr;
	int    iTurret;
	Coor   xUs      = pEnv->stMyVehicle.loc.x;
	Coor   yUs      = pEnv->stMyVehicle.loc.y;
	Speed  xvUs     = pEnv->stMyVehicle.xspeed;
	Speed  yvUs     = pEnv->stMyVehicle.yspeed;
	Coor   xTarget  = pEnv->target.loc.x;
	Coor   yTarget  = pEnv->target.loc.y;
	Speed  xvTarget = pEnv->target.xspeed;
	Speed  yvTarget = pEnv->target.yspeed;
	Coor   xTurret;
	Coor   yTurret;
    float  fNumFired[MAX_TURRETS];
	Angle  aBestAngle;
	float fDistance = HYPOT(xUs - xTarget, yUs - yTarget);

    if (!bShootAlways)
	{
        if (pEnv->stGameSettings.no_wear)
	    {
		    /* can't do any harm, so why fire? */
            check_clock(pEnv);
		    return (FALSE);
	    }

	    if (!xvTarget && !yvTarget)
	    {
            if (!MyClearPath(&pEnv->stMyVehicle.loc, &pEnv->target.loc))
		    {
                check_clock(pEnv);
		        return (FALSE);
		    }
	    }
	}

	if (!pEnv->stGameSettings.rel_shoot)
	{
		xvUs = 0;
		yvUs = 0;
	}

    pEnv->aTurn = fixed_angle(ATAN2(yTarget - yUs, xTarget - xUs));

    iTurret = pEnv->stMyVehicle.num_turrets;
	while (iTurret > 0)
	{
		iTurret--;

		if (!pEnv->Turret[iTurret].iNumWeapons)
		{
			continue;
		}

        fNumFired[iTurret]   = 0.0;

        turret_position(iTurret, &xTurret, &yTurret);

#ifdef CHECK_LAST_FIRE
		if (pEnv->frCurrent - pEnv->Turret[iTurret].iLastFire <
			pEnv->Turret[iTurret].iMinReload)
		{
			aBestAngle = ATAN2(yTarget - yUs - yTurret,
							   xTarget - xUs - xTurret);
			turn_turret(iTurret, aBestAngle);
			bTurretCanHit[iTurret] = FALSE;
			continue;
		}
#endif

        if (fDistance < DIRECT_SHOT_DISTANCE)
		{
			aBestAngle = ATAN2(yTarget - yUs - yTurret,
							   xTarget - xUs - xTurret);
			bTurretCanHit[iTurret] = TRUE;
			pEnv->bLayMines = TRUE;
		}
        else
		{
			Angle aTurretView = 0;
			int iErrorMargin = MARGIN_OF_ERROR;

#ifdef CHECK_ANGLE
			if (pEnv->stGameSettings.game == RACE_GAME ||
				fDistance < MINE_RANGE)
			{
				aTurretView = 1.9 * PI;
			}
			else
	        if (!xvTarget && !yvTarget)
			{
				aTurretView = pEnv->Turret[iTurret].aRate;
			}
			else
			{
				aTurretView = pEnv->Turret[iTurret].aRate + PI / 4.0;
			}
#endif
	        if (!xvTarget && !yvTarget)
	        {
			    iErrorMargin = MIN(pEnv->Turret[iTurret].iAvgAmmoSpeed / 2,
								   iErrorMargin);
			}

            bTurretCanHit[iTurret] =  TargetSolution(pEnv, xvUs, yvUs,
									  xUs + xTurret,
									  yUs + yTurret,
						              xvTarget, yvTarget, xTarget, yTarget,
				                      iErrorMargin,
						              pEnv->Turret[iTurret].iAvgAmmoLifetime,
						              pEnv->Turret[iTurret].iAvgAmmoSpeed,
					                  pEnv->stMyVehicle.turret_angle[iTurret],
									  aTurretView, &aBestAngle);
		}


		if (bTurretCanHit[iTurret])
		{
	        aBestAngle = fixed_angle(aBestAngle);
			turn_turret(iTurret, aBestAngle);
		}
		else
		{
			aBestAngle = ATAN2(yTarget - yUs - yTurret,
							   xTarget - xUs - xTurret);
			turn_turret(iTurret, aBestAngle);
		}
	}

	bShotAtTarget = FALSE;
    for (iCtr = 0; iCtr < pEnv->iNumWeapons; iCtr++)
    {
	    if (pEnv->stMyWeapons[iCtr].type != MINE &&
			pEnv->stMyWeapons[iCtr].type != SLICK)
	    {
			if ((pEnv->stMyWeapons[iCtr].mount == MOUNT_RIGHT &&
				 pEnv->iStatus == ENGAGE && pEnv->iTurnDir == -1) ||
			    (pEnv->stMyWeapons[iCtr].mount == MOUNT_LEFT &&
				 pEnv->iStatus == ENGAGE && pEnv->iTurnDir == 1))
			{
#ifdef OLD_WAY_SHOOT
	            bShotAtTarget = TRUE;
#endif
	            fire_weapon(iCtr);
			}
			else
			if (pEnv->stMyWeapons[iCtr].mount == MOUNT_TURRET1 ||
				pEnv->stMyWeapons[iCtr].mount == MOUNT_TURRET2 ||
				pEnv->stMyWeapons[iCtr].mount == MOUNT_TURRET3
#ifdef HAS_TURRET4
                || pEnv->stMyWeapons[iCtr].mount == MOUNT_TURRET4
#endif
				)
			{
				if (bTurretCanHit[pEnv->stMyWeapons[iCtr].mount])
				{
					int iMount = pEnv->stMyWeapons[iCtr].mount;
#ifdef CHECK_LAST_FIRE
					pEnv->Turret[iMount].iLastFire = pEnv->frCurrent;
#endif
                    if (fNumFired[iMount] / pEnv->Turret[iMount].iNumWeapons <
						1.0 / pEnv->Turret[iMount].iMinReload ||
                        fDistance < DIRECT_SHOT_DISTANCE ||
						pEnv->frCurrent != pEnv->frPrevious + 1 ||
                        (!xvTarget && !yvTarget))
					{
	                    bShotAtTarget = TRUE;
	                    if (fire_weapon(iCtr) == FIRED)
						{
							fNumFired[iMount] += 1.0;
						}
					}
				}
			}
			else
			if (pEnv->stMyWeapons[iCtr].mount == MOUNT_FRONT &&
				(pEnv->iStatus == RAMMING || bShootAlways))
			{
#ifdef OLD_WAY_SHOOT
	            bShotAtTarget = TRUE;
#endif
	            fire_weapon(iCtr);
			}
			else
			if (pEnv->stMyWeapons[iCtr].mount == MOUNT_BACK &&
				(pEnv->iStatus == DIS_ENGAGE))
			{
#ifdef OLD_WAY_SHOOT
	            bShotAtTarget = TRUE;
#endif
	            fire_weapon(iCtr);
			}
	    }
		else
		if (pEnv->iStatus == DIS_ENGAGE || pEnv->bLayMines)
		{
	        fire_weapon(iCtr);
		}
	}

    if (pEnv->iStatus == RAMMING)
	{
		int iMaxCount  = 4;
		int iSignX     = SIGN(xUs - xTarget);
		int iSignY     = SIGN(yUs - yTarget);
		int xNewUs     = xUs + xvUs;
		int yNewUs     = yUs + yvUs;
		int xNewTarget = xTarget + xvTarget;
		int yNewTarget = yTarget + yvTarget;

        do
		{
		    if (SIGN(xNewUs - xNewTarget) == iSignX &&
		        SIGN(yNewUs - yNewTarget) == iSignY)
			{
                pEnv->aTurn = fixed_angle(ATAN2(yNewTarget - yNewUs,
											    xNewTarget - xNewUs));
		        xNewUs     += xvUs;
		        yNewUs     += yvUs;
		        xNewTarget += xvTarget;
		        yNewTarget += yvTarget;
			}
			else
			{
				break;
			}
		}
		while (iMaxCount--);

		pEnv->fClosePercent = 0.0;
	}
	else
	if (IsApproaching(xTarget, yTarget, xvTarget, yvTarget, xUs, yUs))
	{
		int iAvgAmmoRange = pEnv->iAvgAmmoRange;

		if (fDistance < iAvgAmmoRange)
		{
			pEnv->iTimeOut = 0;
		}
		else
		{
#define PREEMPT_MOVE
#ifdef PREEMPT_MOVE
        pEnv->aTurn = fixed_angle(ATAN2((yTarget + yvTarget) - (yUs + yvUs),
    									(xTarget + xvTarget) - (xUs + xvUs)));
#endif
			pEnv->iTimeOut++;
		}

		if (fDistance < iAvgAmmoRange)
		{
		    pEnv->fClosePercent = 0.50;
		}
		else
		{
		    pEnv->fClosePercent = 0.30;
		}
	}
	else
    {
		int iAvgAmmoRange = pEnv->iAvgAmmoRange;

		if (fDistance < iAvgAmmoRange)
		{
			pEnv->iTimeOut = 0;
		}
		else
		{
			pEnv->iTimeOut++;
#define PREEMPT_MOVE
#ifdef PREEMPT_MOVE
        pEnv->aTurn = fixed_angle(ATAN2((yTarget + yvTarget) - (yUs + yvUs),
    									(xTarget + xvTarget) - (xUs + xvUs)));
#endif
		}

		if (fDistance < iAvgAmmoRange * 0.15)
		{
		    pEnv->fClosePercent = 0.50;
		}
		else
		if (fDistance < iAvgAmmoRange * 0.3)
		{
		    pEnv->fClosePercent = 0.43;
		}
		else
		if (fDistance < iAvgAmmoRange * 0.9
#define USE_TIMEOUTS
#ifdef USE_TIMEOUTS
			&& pEnv->iTimeOut > 2
#endif
			)
		{
		    pEnv->fClosePercent = 0.30;
		}
		else
		if (fDistance < iAvgAmmoRange * 1.1
#ifdef USE_TIMEOUTS
			&& pEnv->iTimeOut > 3
#endif
			)
		{
		    pEnv->fClosePercent = 0.15;
		}
		else
		if (fDistance >= iAvgAmmoRange * 1.1
#ifdef USE_TIMEOUTS
			&& pEnv->iTimeOut > 4
#endif
			)
		{
		    pEnv->fClosePercent = 0.05;
		}
		else
		{
		    pEnv->fClosePercent = 0.37;
		}
	}

	pEnv->aTurn += pEnv->iTurnDir * PI * pEnv->fClosePercent;
	pEnv->aTurn = fixed_angle(pEnv->aTurn);

#ifdef DEBUG_CLOSE_ANGLE
	printf("[%d]Dist %f, Close %f, TimeOut %d, AmmoRange %d\n",
		   pEnv->stMyVehicle.id, fDistance,
		   pEnv->fClosePercent, pEnv->iTimeOut, pEnv->iAvgAmmoRange);
#endif

	check_clock(pEnv);

	return (bShotAtTarget);
}

static int TargetSolution(pEnv, xvUs, yvUs, xUs, yUs,
	  xvTarget, yvTarget, xTarget, yTarget,
          iErrorMargin, iAmmoLifetime, iAmmoSpeed,
          aCurrent, aRange, pBestAngle)
Environment *pEnv;
Speed xvUs;
Speed yvUs;
Coor  xUs;
Coor  yUs;
Speed xvTarget;
Speed yvTarget;
Coor  xTarget;
Coor  yTarget;
int   iErrorMargin;
int   iAmmoLifetime;
int   iAmmoSpeed;
Angle aCurrent;
Angle aRange;
Angle *pBestAngle;
{
    int iCtr;
	int bFoundSolution = FALSE;
	int	bSetAimAngle = FALSE;
	int iBestError = iAmmoLifetime * iAmmoSpeed + iErrorMargin + 1;
	Coor xImpact   = xTarget;
	Coor yImpact   = yTarget;
#ifdef FOR_FUTURE_USE
	Coor xFutureUs = xUs;
	Coor yFutureUs = yUs;
#endif
	Coor xBullet;
	Coor yBullet;
	Angle aNewAngle = 0;
#ifdef CHECK_ANGLE
	Angle aDifference;
#endif
	int iError;
	int iErrorDiff;

#ifdef DEBUG_TARGET_SOLUTIONS
	pEnv->iCalls++;
#endif

	for (iCtr = 0; iCtr < iAmmoLifetime; iCtr++,
#ifdef FOR_FUTURE_USE
			 xFutureUs += xvUs,
	         yFutureUs += yvUs,
#endif
			 xImpact   += xvTarget - xvUs,
	         yImpact   += yvTarget - yvUs)

	{
#ifdef PREDICT_ACCELERATION
		if (pEnv->target.id == pEnv->prevTarget.id &&
			pEnv->fTargetTime < pEnv->fprevTime + 16 &&
			!iCtr)
		{
			int iTimeDiff = pEnv->fTargetTime - pEnv->fprevTime;

			xvTarget += (pEnv->target.xspeed - pEnv->prevTarget.xspeed) /
						iTimeDiff;
			yvTarget += (pEnv->target.yspeed - pEnv->prevTarget.yspeed) /
						iTimeDiff;
		}
#endif
#ifdef DEBUG_TARGET_SOLUTIONS
	    pEnv->iIterations++;
#endif
		aNewAngle = fixed_angle(ATAN2(yImpact - yUs, xImpact - xUs));

#ifdef CHECK_ANGLE
        aDifference = aNewAngle - aCurrent;
		if (aDifference < 0)
		{
			aDifference = -aDifference;
		}
		if (aDifference < aRange)
#endif
		{
			double trigCos, trigSin;

#ifdef HAS_SINCOS
			sincos(aNewAngle, &trigSin, &trigCos);
#else
			trigSin = SIN(aNewAngle);
			trigCos = COS(aNewAngle);
#endif

		    xBullet = xUs + iAmmoSpeed * trigCos * iCtr;
		    yBullet = yUs + iAmmoSpeed * trigSin * iCtr;

            iError = ABS(xBullet - xImpact);
            if (iError < ABS(yBullet - yImpact))
			{
                 iError = ABS(yBullet - yImpact);
			}

		    if (iError < iBestError)
		    {
				int bPrevFoundSolution = bFoundSolution;
#ifdef DEBUG_TARGET_SOLUTIONS
        	    pEnv->iBetters++;
#endif

			    iErrorDiff = iBestError - iError;

			    bFoundSolution = (iError < iErrorMargin);
				if (bFoundSolution)
				{
					Location lDestination;
#define CLEAR_CHECK
#ifdef CLEAR_CHECK
#ifdef DEBUG_TARGET_SOLUTIONS
	                pEnv->iClearPathCalls++;
#endif
                    memcpy(&lDestination, &pEnv->stMyVehicle.loc,
						   sizeof(Location));
                    DeltaLoc(&lDestination,
							 xBullet - xUs + xvUs * iCtr,
							 yBullet - yUs + yvUs * iCtr);
                    if (MyClearPath(&pEnv->stMyVehicle.loc, &lDestination))
#else
					if (1)
#endif
					{
#ifdef DEBUG_TARGET_SOLUTIONS
	                pEnv->iClearPaths++;
#endif
#define REDUCE_BUDDY_BUSTAGE
#ifdef REDUCE_BUDDY_BUSTAGE
                        {
							int iNumStill = pEnv->iStills;
							int bGoForIt = TRUE;

                            while (iNumStill)
							{
								int iTimeCtr;
								int iDiffX;
								int iDiffY;

                                iNumStill--;

								for (iTimeCtr = 1;
									 iTimeCtr <= iCtr && bGoForIt;
									 iTimeCtr++)
								{
									iDiffX = xUs +(xvUs + iAmmoSpeed * trigCos)
											 * iTimeCtr -
											 pEnv->aiStills[iNumStill][0];
									iDiffY = yUs +(yvUs + iAmmoSpeed * trigSin)
											 * iTimeCtr -
											 pEnv->aiStills[iNumStill][1];

									if (iDiffX * iDiffX + iDiffY * iDiffY <
										pEnv->aiStills[iNumStill][2])
                                    {
										bGoForIt = FALSE;
									}
								}
							}

							if (bGoForIt)
							{
			                    iBestError = (int)iError;
		                        *pBestAngle = aNewAngle;
							}
							else
							{
						        bFoundSolution = bPrevFoundSolution;
							}
						}

#else
			            iBestError = (int)iError;
		                *pBestAngle = aNewAngle;
#endif
					}
					else
					{
						bFoundSolution = bPrevFoundSolution;
					}
				}
				else
				{
					bFoundSolution = bPrevFoundSolution;
				}

				if (iErrorDiff * (iAmmoLifetime - iCtr) < iError)
				{
					break;
				}
#ifdef STOP_ON_FIRST_SOLUTION
				if (bFoundSolution)
				{
					break;
				}
#endif
		    }
			else
			{
			    if (!bFoundSolution && !bSetAimAngle)
			    {
				    *pBestAngle = aNewAngle;
				    bSetAimAngle = TRUE;
			    }
			    break;
			}
		}
	}

	return (bFoundSolution);
}


static void
cleanup(Environment *pEnv)
{
    /* free all dynamically allocated memory */

	if (pEnv)
	{
#ifdef DEBUG_TARGET_SOLUTIONS
        printf("[%d]Calls:%d Zeros:%d Iterations:%d Betters:%d ClearPathCalls:%d ClearPaths:%d\n",
		   pEnv->stMyVehicle.id,
	           pEnv->iCalls,
	           pEnv->iZeros,
	           pEnv->iIterations,
		   pEnv->iBetters,
		   pEnv->iClearPathCalls,
		   pEnv->iClearPaths);
#endif
        free((char *)pEnv);
	}
}

static void
TagmanMain(void)
{
    Environment *pEnv = NULL;
	int bInitialized;

    bInitialized = InitializeVariables(&pEnv);

    if (bInitialized)
	{
        while (TRUE)
        {
		    MainLoop(pEnv);
        }
	}
	else
	{
        while (TRUE)
        {
		    done();
        }
	}
}


static void
MainLoop(Environment *pEnv)
{
		pEnv->bHaveTarget = FALSE;

	    switch (pEnv->iStatus)
	    {
		    case RAMMING:
		    case ENGAGE:
                GetBasics(pEnv);
			    Acquire(pEnv, CLOSEST);
				if (pEnv->bHaveTarget)
				{
			        HuntAndKill(pEnv);
			        Move(pEnv);
				    ProcessMessages(pEnv, 4);
				}

                if (heat() > 90)
	            {
	                pEnv->iStatus = DIS_ENGAGE;
	            }

                if (pEnv->iStatus == ENGAGE || pEnv->stGameSettings.pay_to_play)
				{
				    if (NeedAmmo(0.1))
				    {
				        if (get_money() >= 50)
						{
				            pEnv->iGoal = AMMO;
				            StartSeek(pEnv);
						}
						else
						if (OutOfAmmo(FALSE))
						{
							pEnv->iStatus = RAMMING;
						}
				    }
					else
				    if (NeedArmor(
							pEnv->stGameSettings.pay_to_play ? 0.25 : 0.1) &&
						get_money() >= 490)
				    {
				          pEnv->iGoal = ARMOR;
				          StartSeek(pEnv);
				    }
					else
					if (NeedFuel(0.1) && get_money() >= 20)
					{
				          pEnv->iGoal = FUEL;
				          StartSeek(pEnv);
					}
				}

				if (!pEnv->bHaveTarget)
				{
				    if (pEnv->iStatus == ENGAGE ||
				        pEnv->iStatus == RAMMING)
				    {
	                    pEnv->iDestX = pEnv->target.loc.grid_x;
    	                pEnv->iDestY = pEnv->target.loc.grid_y;

	                    StartGoto(pEnv);

						if (pEnv->iStatus == GOTO)
						{
			                pEnv->iSpecTarget    = TRUE;
			                pEnv->iTargetVehicle = pEnv->target.id;
			                pEnv->iWasCalled     = TRUE;
			                pEnv->iCallerId      = pEnv->stMyVehicle.id;
						}
				    }
				}
			    break;

		    case DIS_ENGAGE:
                GetBasics(pEnv);
			    Acquire(pEnv, CLOSEST);
				if (!pEnv->bHaveTarget)
				{
				    ProcessMessages(pEnv, 10);
					StartWandering(pEnv);
					break;
				}
			    Fight(pEnv, FALSE);
			    Move(pEnv);
				ProcessMessages(pEnv, 4);
			    break;

			case SEEK:
		    case GOTO:
		    case WANDER:
                GetBasics(pEnv);
				/* new box? */
	            if (!pEnv->boxnotes[pEnv->stMyVehicle.loc.grid_x]
								   [pEnv->stMyVehicle.loc.grid_y].seen)
				{
	                NoteSurroundings(pEnv);
				    if (pEnv->iStatus == SEEK && !pEnv->bGoalLocated)
				    {
	                    /* either head for goal or explore */
	                    if (!navigate(pEnv, IsGoalBox, TRUE))
						{
		                    if (!navigate(pEnv, unseen_box, FALSE))
							{
								StartWandering(pEnv);
							}
						}
	                }
	            }
			    Acquire(pEnv, CLOSEST);
				if (pEnv->iStatus != SEEK)
				{
				    if (pEnv->bHaveTarget)
				    {
					    StartEngage(pEnv);
						if (pEnv->iStatus != ENGAGE)
						{
			                Move(pEnv);
						}
				    }
				    else
				    {
						if (HandleOutpost(pEnv))
						{
							if (pEnv->iStatus == WANDER)
							{
								pEnv->iStatus = ENGAGE;
			                    Move(pEnv);
								pEnv->iStatus = WANDER;
				                ProcessMessages(pEnv, 2);
							}
							else
							{
			                    Move(pEnv);
							}
						}
						else
						{
							if (pEnv->bHaveNewRadar &&
								pEnv->bHaveTacLink  &&
								pEnv->iStatus == WANDER)
                            {
								FindEnemy(pEnv);
			                    if (pEnv->iStatus == GOTO)
			                    {
				                    break;
			                    }
	                        }
			                Move(pEnv);
				            ProcessMessages(pEnv, 11);
					        PositionTurrets(pEnv);

                            if (HandleLocalLandMark(pEnv))
					        {
						        pEnv->iStatus = REPLENISH;
					        }
						}
					}
				}
				else
				{
				    if (pEnv->bHaveTarget)
				    {
					    Fight(pEnv, FALSE);

						SendEnemyAt(pEnv, &pEnv->target, TRUE);
				    }
                    if (HandleLocalLandMark(pEnv))
					{
						pEnv->iStatus = REPLENISH;
					}
					else
					{
			            Move(pEnv);
						if (!pEnv->bHaveTarget)
						{
					        HandleOutpost(pEnv);
						}
					}
				    ProcessMessages(pEnv, 8);
				}
				break;

		    case REPAIRING:
                GetBasics(pEnv);
			    Acquire(pEnv, CLOSEST);
				if (pEnv->bHaveTarget)
				{
					StartEngage(pEnv);
					if (pEnv->iStatus != ENGAGE)
					{
	                    set_rel_drive(0.0);
				        ProcessMessages(pEnv, 10);
						StartWandering(pEnv);
					}
				}
				else
				{
	                set_rel_drive(0.0);
				    ProcessMessages(pEnv, 10);
					StartWandering(pEnv);
				}
				break;

			case REPLENISH:
                GetBasics(pEnv);
			    Acquire(pEnv, CLOSEST);
                if (!HandleLocalLandMark(pEnv))
				{
				    TurnOnAllWeapons();
				    if (pEnv->bHaveTarget)
				    {
					    Fight(pEnv, FALSE);
						StartEngage(pEnv);
				    }
					else
					{
				        ProcessMessages(pEnv, 11);
					    PositionTurrets(pEnv);
						StartWandering(pEnv);
					}
				}
				else
				{
					if (pEnv->bHaveTarget)
					{
					    Frantic(pEnv);
                        TurnOnAllWeapons();
					    Fight(pEnv, FALSE);
                        TurnOffAllWeapons();

						SendEnemyAt(pEnv, &pEnv->target, TRUE);
					}
					else
					{
#ifdef FRANTIC_ON_BULLETS
						if (!pEnv->stMyVehicle.xspeed &&
						    !pEnv->stMyVehicle.yspeed)
						{
							Frantic(pEnv);
						}
#endif
					    PositionTurrets(pEnv);
					}
				}
				ProcessMessages(pEnv, 4);
				if (pEnv->iStatus != REPLENISH)
				{
                    TurnOnAllWeapons();
				}
			    break;
		    default:
				StartWandering(pEnv);
			    break;
	    }
	    done();
}


static void
Frantic(Environment *pEnv)
{
    int   aiArmor[6];
	int   iArmorSide;
    Angle aToTarget = fixed_angle(
		ATAN2(pEnv->target.loc.y - pEnv->stMyVehicle.loc.y,
              pEnv->target.loc.x - pEnv->stMyVehicle.loc.x));

	if (pEnv->bHaveTarget)
	{
        aToTarget = fixed_angle(
		    ATAN2(pEnv->target.loc.y - pEnv->stMyVehicle.loc.y,
            pEnv->target.loc.x - pEnv->stMyVehicle.loc.x));
    }
	else
	{
		int iNumBullets;
		int iCtr;
	    Bullet_info biBullets[MAX_BULLETS];

		get_bullets(&iNumBullets, biBullets);

		if (iNumBullets)
		{
			Bullet_info *pBullet = biBullets;
            int iProtection = protection();
			int OurX = pEnv->stMyVehicle.loc.x;
			int OurY = pEnv->stMyVehicle.loc.y;

			for (iCtr = 0; iCtr < iNumBullets; pBullet++, iCtr++)
			{
				int iDamage = aiDamage[pBullet->type] - iProtection;

				if ((pBullet->type == MINE || pBullet->type == SLICK)
					|| iDamage < 1)
				{
					continue;
				}

				if (IsApproaching(pBullet->loc.x, pBullet->loc.y,
								  pBullet->xspeed, pBullet->yspeed,
								  OurX, OurY))
				{
                    aToTarget = fixed_angle(
		                ATAN2(pBullet->loc.y - OurY,
                        pBullet->loc.x - OurX));
					break;
				}
			}

			if (iCtr >= iNumBullets)
			{
				return;
			}
		}
	}

	aiArmor[LEFT]  = armor(LEFT);
	aiArmor[RIGHT] = armor(RIGHT);
	aiArmor[FRONT] = armor(FRONT);
	aiArmor[BACK]  = armor(BACK);

#define BETTER_ARMOR(S1, S2) (aiArmor[(S1)] > aiArmor[(S2)] ? (S1) : (S2))

	iArmorSide =
		BETTER_ARMOR(BETTER_ARMOR(LEFT, RIGHT), BETTER_ARMOR(FRONT, BACK));

    switch (iArmorSide)
	{
		case LEFT:
			pEnv->aTurn = aToTarget + PI / 2.0;
			break;

		case RIGHT:
			pEnv->aTurn = aToTarget - PI / 2.0;
			break;

		case FRONT:
			pEnv->aTurn = aToTarget;
			break;

		default:
		case BACK:
			pEnv->aTurn = aToTarget + PI;
			break;

	}

	pEnv->aTurn = fixed_angle(pEnv->aTurn);

	if (pEnv->stMyVehicle.heading - pEnv->aTurn > PI * 0.75 ||
		pEnv->stMyVehicle.heading - pEnv->aTurn < -PI * 0.75)
	{
		if (aiArmor[dir2clockwise[iArmorSide]] >
			aiArmor[dir2counterclockwise[iArmorSide]])
		{
			pEnv->aTurn -= PI / 6.0;
		}
		else
		{
			pEnv->aTurn += PI / 6.0;
		}

		pEnv->aTurn = fixed_angle(pEnv->aTurn);
	}

	turn_vehicle(pEnv->aTurn);
}


static void
StartGoto(Environment *pEnv)
{
	pEnv->iStatus    = GOTO;
	pEnv->have_route = FALSE;

	if (!navigate(pEnv, IsGoalBox, TRUE))
	{
	    if (!navigate(pEnv, unseen_box, FALSE))
	    {
		    StartWandering(pEnv);
	    }
	}
}


static void
StartSeek(Environment *pEnv)
{
    Status_code iOrigStatus = pEnv->iStatus;

	pEnv->iStatus = SEEK;
	pEnv->have_route = FALSE;
	if (!navigate(pEnv, IsGoalBox, TRUE))
	{
		Byte abData[MAX_DATA_LEN];

	    abData[0] = (char)pEnv->iGoal;
	    abData[1] = 0;

	    send_msg(MAX_VEHICLES + pEnv->stMyVehicle.team, OP_WHERE_IS, abData);
	    if (!navigate(pEnv, unseen_box, FALSE))
	    {
			if (iOrigStatus == SEEK)
			{
	            StartWandering(pEnv);
			}
			else
			{
	            pEnv->iStatus = iOrigStatus;
			}
	    }
	}

	PositionTurrets(pEnv);
}

static void
StartEngage(Environment *pEnv)
{
	pEnv->iTurnDir   = (armor(LEFT) > armor(RIGHT)) ? 1 : -1;
	pEnv->have_route = FALSE;
	if (pEnv->bAlwaysRam)
	{
	    pEnv->iStatus = RAMMING;
	}
	else
	{
	    pEnv->iStatus = ENGAGE;
	}
	pEnv->iTimeOut   = 0;

	pEnv->fDriveSpeed = 9.0;
	set_rel_drive(pEnv->fDriveSpeed);

	HuntAndKill(pEnv);

	SendEnemyAt(pEnv, &pEnv->target, FALSE);
}

static void
StartWandering(Environment *pEnv)
{
	Byte abData[MAX_DATA_LEN];
	int x;
	int y;
	int iWeaponNum;
	int iWeaponNumOrig;
	float fAmmoPercent;
	Weapon_info winfo;

	abData[0] = pEnv->stMyVehicle.loc.grid_x;
	abData[1] = pEnv->stMyVehicle.loc.grid_y;
	abData[2] = 0;

    if (pEnv->frCurrent &&
		pEnv->frCurrent - pEnv->frLastOpen >= MIN_OPEN_SPACING)
	{
        fAmmoPercent = 0.0;

	    if (pEnv->iStatus != SEEK &&
			(iWeaponNum = iWeaponNumOrig = pEnv->iNumWeapons))
	    {
			if (get_money() >= 50)
			{
	            for (;iWeaponNum-- > 0;)
	            {
		            get_weapon(iWeaponNum, &winfo);
			        fAmmoPercent += ((float)weapon_ammo(iWeaponNum) /
							         (float)winfo.max_ammo);
	            }
			    if (fAmmoPercent / (float)iWeaponNumOrig < 0.50)
		        {
				    pEnv->iGoal = AMMO;
				    StartSeek(pEnv);
				    if (pEnv->iStatus == SEEK)
				    {
					    return;
				    }
		        }
		    }
	    }
		{
	        Side side;
			int bHaveRepair = has_special(REPAIR) && fuel() / max_fuel() > 0.1;

	        for (side = 0; side < MAX_SIDES; ++side)
	        {
				if (bHaveRepair)
				{
		            if (armor(side) < max_armor(side) * 0.90)
					{
						pEnv->iStatus = REPAIRING;
						return;
					}
				}
				else
				if (get_money() < 490)
				{
					continue;
				}
				else
		        if (armor(side) < max_armor(side) *
					(pEnv->stGameSettings.restart ?
					 (pEnv->stGameSettings.pay_to_play ? 0.5 : 0.25) : 0.85))
		        {
				    pEnv->iGoal = ARMOR;
				    StartSeek(pEnv);
				    if (pEnv->iStatus == SEEK)
				    {
					    return;
				    }
		        }
	        }
		}

		if (NeedFuel(0.25) && get_money() >= 20)
		{
			pEnv->iGoal = FUEL;
			StartSeek(pEnv);
			if (pEnv->iStatus == SEEK)
			{
				return;
			}
		}

		pEnv->frLastOpen = pEnv->frCurrent;
	    send_msg(MAX_VEHICLES + pEnv->stMyVehicle.team, OP_OPEN, abData);
	}

	pEnv->iStatus     = WANDER;
	pEnv->have_route  = FALSE;
	pEnv->iWasCalled  = FALSE;
	pEnv->iSpecTarget = FALSE;
	{
		int bChooseRandom = TRUE;
		int bAttemptArmor = FALSE;
		int iNumBlips;
		int side;
		Blip_info BlipInfo[MAX_BLIPS];

	    for (side = 0; side < MAX_SIDES; ++side)
	    {
		    /* Legal armor in lowlib.c needs to work correctly */
		    if (armor(side) < max_armor(side) * 0.90)
		    {
			    bAttemptArmor = TRUE;
				break;
		    }
	    }

		if (bAttemptArmor && get_money() > 490)
		{
	        for (x = 0; x < GRID_WIDTH && bChooseRandom; ++x)
	        {
		        for (y = 0; y < GRID_HEIGHT && bChooseRandom; ++y)
		        {
					if (pEnv->boxnotes[x][y].seen)
					{
	                    if (landmark(x, y) == ARMOR)
					    {
/* We need to go to the best armor */
/* plot a course here too          */
	                        pEnv->iDestX  = (int)x;
	                        pEnv->iDestY  = (int)y;
							bChooseRandom = FALSE;
					    }
					}
		        }
	        }
		}

        if (bChooseRandom)
		{
		    MyGetBlips(pEnv, &iNumBlips, BlipInfo);
			if (iNumBlips)
			{
				int iRandomIndex = frand((float)iNumBlips);

	            pEnv->iDestX = BlipInfo[iRandomIndex].x;
	            pEnv->iDestY = BlipInfo[iRandomIndex].y;

				bChooseRandom = (pEnv->iDestX ==
								 pEnv->stMyVehicle.loc.grid_x &&
				                 pEnv->iDestY ==
								 pEnv->stMyVehicle.loc.grid_y);
		    }
		}

		if (bChooseRandom)
		{
	        pEnv->iDestX = (int)frand((float)GRID_WIDTH);
	        pEnv->iDestY = (int)frand((float)GRID_HEIGHT);
		}
#ifdef DEBUG_WANDER_DESTINATION
	BarfBarf(RECIPIENT_ALL, "Lets go %s %d,%d",
			 (bChooseRandom) ? "Random" : "Blip",
	         pEnv->iDestX, pEnv->iDestY);
#endif
	}

	PositionTurrets(pEnv);
	pEnv->have_route = FALSE;
	if (!navigate(pEnv, IsGoalBox, TRUE))
	{
		if (!navigate(pEnv, unseen_box, FALSE))
		{
#if 0
/*
 * The call to StartWandering() is recursive.  On a fast system, this
 * will blow out the stack for the thread, and the entire games dies
 * when the end of the stack is encountered.  The above calls to
 * navigate() may alter internal state in pEnv, so leave them in place
 * but just remove execution of the recursive call.
*/
			StartWandering(pEnv);
#endif
		}
	}
#ifdef DEBUG_WANDER_NAVIGATION
	BarfBarf(pEnv->stMyVehicle.id, "Choose course for");
	if (pEnv->have_route)
	{
		BarfBarf(pEnv->stMyVehicle.id, "good nav ");
	}
	else
	{
		BarfBarf(pEnv->stMyVehicle.id, "bad nav ");
	}
#endif
}


static void
GetBasics(Environment *pEnv)
{
	pEnv->stPrevLoc = pEnv->stMyVehicle.loc;
	pEnv->frPrevious = pEnv->frCurrent;
	pEnv->frCurrent  = frame_number();
    get_self(&pEnv->stMyVehicle);
#ifdef DUMP_SAME_SQAURE
    if (pEnv->stMyVehicle.loc.grid_x == pEnv->stPrevLoc.grid_x &&
        pEnv->stMyVehicle.loc.grid_y == pEnv->stPrevLoc.grid_y)
	{
    	printf("[%d], mode=%d\n", pEnv->stMyVehicle.id, pEnv->iStatus);
	}
#endif
}


static void Acquire(Environment *pEnv, Search_mode what)
{
    int  iCtr;
	Coor xUs;
	Coor yUs;
	Coor xIt;
	Coor yIt;
	int  xvUs;
	int  yvUs;
	int  dx;
	int  dy;
	int  iTarget;
	int  bHaveTarget = FALSE;
	int  bClosing    = FALSE;
	int  iDistance;

    memcpy(&pEnv->prevTarget, &pEnv->target, sizeof(Vehicle_info));
	pEnv->fprevTime            = pEnv->fTargetTime;
	pEnv->fTargetTime          = pEnv->frCurrent;

	pEnv->aTurn                = pEnv->stMyVehicle.heading;
    pEnv->bLayMines            = FALSE;
	pEnv->iFriendliesInTheArea = 0;
	pEnv->bHaveTarget          = FALSE;
	pEnv->iStills              = 0;

    if (pEnv->stGameSettings.no_wear)
	{
		return;
	}

	xUs  = pEnv->stMyVehicle.loc.x;
	yUs  = pEnv->stMyVehicle.loc.y;
	xvUs = pEnv->stMyVehicle.xspeed;
	yvUs = pEnv->stMyVehicle.yspeed;

    get_vehicles(&pEnv->iNumVehicles, pEnv->stVehicles);

    if (pEnv->iNumVehicles)
    {
        for (iCtr = 0; iCtr < pEnv->iNumVehicles; iCtr++)
        {
            xIt = pEnv->stVehicles[iCtr].loc.x;
            yIt = pEnv->stVehicles[iCtr].loc.y;

            if (!pEnv->stVehicles[iCtr].team ||
                pEnv->stVehicles[iCtr].team != pEnv->stMyVehicle.team)
            {
                dx = xUs - xIt;
                dy = yUs - yIt;

				iDistance = dx * dx + dy * dy;

				if (iDistance < MINE_RANGE * MINE_RANGE)
				{
					pEnv->bLayMines = TRUE;
				}

				if (IsClosing(xUs, yUs, xvUs, yvUs,
                              xIt, yIt,
                              pEnv->stVehicles[iCtr].xspeed,
                              pEnv->stVehicles[iCtr].yspeed))
				{
					bClosing = TRUE;
				}

				switch (what)
				{
					case CLOSEST:
	                    if (bHaveTarget)
                        {
		                    if (iDistance < pEnv->iDistance)
		                    {
								pEnv->iDistance = iDistance;
								iTarget = iCtr;
		                    }
                        }
                        else
	                    {
		                    bHaveTarget = TRUE;
							pEnv->iDistance = iDistance;
							iTarget = iCtr;
				        }
						break;
                    case CURRENT:
						if (pEnv->target.id == pEnv->stVehicles[iCtr].id)
						{
		                    bHaveTarget = TRUE;
							iTarget = iCtr;
							pEnv->iDistance = iDistance;
						}
						break;
				}
				if (bHaveTarget && what == CURRENT)
				{
					break;
				}
            }
			else
			{
				pEnv->iFriendliesInTheArea++;
#define REDUCE_BUDDY_BUSTAGE
#ifdef REDUCE_BUDDY_BUSTAGE
				if (pEnv->iStills < MAX_STILLS &&
					!pEnv->stVehicles[iCtr].xspeed &&
					!pEnv->stVehicles[iCtr].yspeed)
				{
					int iNumStill = pEnv->iStills;
					int iSide = MaxBodySide[pEnv->stVehicles[iCtr].body];

					pEnv->aiStills[iNumStill][0] =
					    pEnv->stVehicles[iCtr].loc.x;
					pEnv->aiStills[iNumStill][1] =
						pEnv->stVehicles[iCtr].loc.y;
					pEnv->aiStills[iNumStill][2] =
						iSide * iSide / 4 + 1;
					pEnv->iStills++;
				}
#endif
			}
        }

	    if (bHaveTarget)
	    {
				Location lDestination;

                pEnv->bTargetIsVehicle = TRUE;
                if (VectorsCross(xUs, yUs, xvUs, yvUs,
                         pEnv->stVehicles[iCtr].loc.x,
						 pEnv->stVehicles[iCtr].loc.y,
						 pEnv->stVehicles[iCtr].xspeed,
						 pEnv->stVehicles[iCtr].yspeed))
		        {
			        pEnv->bLayMines = TRUE;
				}

			    memcpy(&lDestination, &(pEnv->stVehicles[iCtr].loc),
					   sizeof(Location));
                DeltaLoc(&lDestination, 0,
					      pEnv->stVehicles[iCtr].yspeed);
                if (!MyClearPath(&pEnv->stVehicles[iCtr].loc, &lDestination))
			    {
					pEnv->stVehicles[iCtr].yspeed = 0;
				}

			    memcpy(&lDestination, &(pEnv->stVehicles[iCtr].loc),
					   sizeof(Location));
                DeltaLoc(&lDestination,
					      pEnv->stVehicles[iCtr].xspeed, 0);
                if (!MyClearPath(&pEnv->stVehicles[iCtr].loc, &lDestination))
			    {
					pEnv->stVehicles[iCtr].xspeed = 0;
				}

assert(iTarget >= 0 && iTarget < pEnv->iNumVehicles);
		    memcpy(&pEnv->target, &pEnv->stVehicles[iTarget],
		           sizeof(Vehicle_info));
	    }


		if (bClosing && !pEnv->iFriendliesInTheArea && heat() < 10)
		{
			pEnv->bLayMines = TRUE;
		}
    }

    pEnv->bHaveTarget = bHaveTarget;
}

static int
IsApproaching(int xThem, int yThem, int xvThem, int yvThem, int xUs, int yUs)
{
    Angle aVelocity;
	Angle aSpace;

    aVelocity = fixed_angle(ATAN2(yvThem, xvThem));
	aSpace    = fixed_angle(ATAN2(yUs - yThem, xUs - xThem));

	return (aVelocity - aSpace <  PI / 4.0 &&
			aVelocity - aSpace > -PI / 4.0);
}


static int
IsClosing(int xUs, int yUs, int xvUs, int yvUs, int xThem, int yThem, int xvThem, int yvThem)
{
    int dx1 = xUs - xThem;
    int dy1 = yUs - yThem;
	int iDiff1 = dx1 * dx1 + dy1 * dy1;
    int dx2 = (xUs + xvUs) - (xThem + xvThem);
    int dy2 = (yUs + yvUs) - (yThem + yvThem);
	int iDiff2 = dx2 * dx2 + dy2 * dy2;

    return (iDiff2 < iDiff1);
}

static void Move(Environment *pEnv)
{
	Location lDestination;

#define DONT_ALWAYS_DODGE
#ifdef DONT_ALWAYS_DODGE
    if (pEnv->bHaveTarget)
	{
        pEnv->fDriveSpeed = 9.0;
	}
	else
#endif
	{
        pEnv->fDriveSpeed = GetDodgeSpeed(pEnv, 9.0);
	}

    switch (pEnv->iStatus)
	{
		case RAMMING:
		case ENGAGE:
	        turn_vehicle(pEnv->aTurn);
			memcpy(&lDestination, &pEnv->stMyVehicle.loc, sizeof(Location));
            DeltaLoc(&lDestination,
					  BOX_WIDTH * SIGN(COS(pEnv->aTurn)),
					  BOX_HEIGHT * SIGN(SIN(pEnv->aTurn)));
            if (MyClearPath(&pEnv->stMyVehicle.loc, &lDestination))
			{
	            set_rel_drive(pEnv->fDriveSpeed);
			}
			else
			{
				pEnv->iTurnDir = -pEnv->iTurnDir;
			}
			break;

		case DIS_ENGAGE:
	        turn_vehicle(pEnv->aTurn);
			if (CanGo(&pEnv->stMyVehicle,
					  BOX_WIDTH * SIGN(COS(pEnv->aTurn)),
					  BOX_HEIGHT * SIGN(SIN(pEnv->aTurn))))
			{
	            set_rel_drive(9.0);
			}
			else
			{
				pEnv->aTurn = fixed_angle(pEnv->aTurn +
										  PI / 3.0 * (float)pEnv->iTurnDir);
	            turn_vehicle(pEnv->aTurn);
			}
			break;

		case SEEK:
			if (IsGoalBox(pEnv, pEnv->stMyVehicle.loc.grid_x,
			    		  pEnv->stMyVehicle.loc.grid_y))
			{
	            set_rel_drive(0.0);
				pEnv->iStatus = REPLENISH;
			}
			else
			{
	            if (pEnv->have_route)
			    {
					int iGoalX = pEnv->iDestX;
					int iGoalY = pEnv->iDestY;
					int iUsX   = pEnv->stMyVehicle.loc.grid_x;
					int iUsY   = pEnv->stMyVehicle.loc.grid_y;
                    int iBoxes = MAX(ABS(iGoalX - iUsX), ABS(iGoalY - iUsY));
#ifdef SAVE
                    if (iBoxes < 3 && ABS(ATAN2(iGoalY - iUsY, iGoalX - iUsX) -
										  pEnv->stMyVehicle.heading) < PI / 4.0)
#endif
                    if (iBoxes < 3)
					{
						int      iDiffX;
						int      iDiffY;
						int      iDist;
						Location lDest;

						lDest.grid_x = iGoalX;
						lDest.grid_y = iGoalY;

						iDiffX = iGoalX * BOX_WIDTH + BOX_WIDTH / 2;
						iDiffY = iGoalY * BOX_HEIGHT + BOX_HEIGHT / 2;

						iDiffX -= pEnv->stMyVehicle.loc.x;
						iDiffY -= pEnv->stMyVehicle.loc.y;

						iDist = SQRT(iDiffX * iDiffX + iDiffY * iDiffY);

#ifdef OLDWAY
                        if (FriendliesReplenishing(pEnv, &lDest))
#endif
			            {
	                        pEnv->fDriveSpeed = SQRT(iDist * tread_acc() *
								pEnv->stGameSettings.normal_friction);
				            pEnv->fDriveSpeed = MAX(pEnv->fDriveSpeed,
										            2 * MIN_CLOSING_SPEED_ABS);

						    pEnv->fDriveSpeed = 9.0 * pEnv->fDriveSpeed /
											max_speed();
			            }
#ifdef OLDWAY
						else
						{
	                        pEnv->fDriveSpeed = SQRT(BOX_WIDTH / 2.0 *
						    tread_acc() * pEnv->stGameSettings.normal_friction);
				            pEnv->fDriveSpeed = MAX(pEnv->fDriveSpeed,
										            MIN_CLOSING_SPEED_ABS);
						    pEnv->fDriveSpeed = 9.0 * pEnv->fDriveSpeed /
											max_speed();
                        }
#endif
					}
					else
					{
	                    pEnv->fDriveSpeed = 9.0;
					}

                    pEnv->fDriveSpeed = MIN(9.0, pEnv->fDriveSpeed);
#ifdef DEBUG_SEEK_SPEED
						    printf("[%d]In calc speed %f\n",
								   pEnv->stMyVehicle.id, pEnv->fDriveSpeed);
#endif
		            FollowRoute(pEnv);
		            HandleWeapons(pEnv);
			    }
			    else
			    {
                    StartSeek(pEnv);
			    }
			}
			break;

		case WANDER:
		case GOTO:
	        if (pEnv->have_route &&
				!IsGoalBox(pEnv, pEnv->stMyVehicle.loc.grid_x,
			    			pEnv->stMyVehicle.loc.grid_y))
			{
	            pEnv->fDriveSpeed = 9.0;
		        FollowRoute(pEnv);
		        HandleWeapons(pEnv);
			}
			else
			{
				StartWandering(pEnv);
			}
			break;

		case REPLENISH:
		case REPAIRING:
		default:
			break;
	}
}


static int
FriendliesReplenishing(Environment *pEnv, Location *pLoc)
{
    int iRetCode = 0;

    if (pEnv->iFriendliesInTheArea)
    {
        int iCtr = pEnv->iFriendliesInTheArea;
        int iCtr2;

        for (iCtr2 = 0; iCtr && iCtr2 < pEnv->iNumVehicles; iCtr2++)
        {
            if (pEnv->stVehicles[iCtr2].team == pEnv->stMyVehicle.team)
            {
				iCtr--;

				if (pEnv->stVehicles[iCtr2].loc.grid_x == pLoc->grid_x &&
					pEnv->stVehicles[iCtr2].loc.grid_y == pLoc->grid_y)
				{
#define MUST_TRULY_BE_REPLENISHING
#ifdef MUST_TRULY_BE_REPLENISHING
					if (AtBoxCenter(&pEnv->stVehicles[iCtr2]))
#endif
					{
						iRetCode +=
							PercentOfLandmark[pEnv->stVehicles[iCtr2].body];
					}
				}
			}
		}
	}

	return (iRetCode);
}

static Location *
DeltaLoc(Location *locp, int delta_x, int delta_y)
#if 0
Location *locp;		/* gets modified */
int delta_x, delta_y;	/* in pixels */
#endif
{
    locp->x += delta_x;
    locp->y += delta_y;
    locp->grid_x = locp->x / BOX_WIDTH;
    locp->box_x  = locp->x % BOX_WIDTH;
    locp->grid_y = locp->y / BOX_HEIGHT;
    locp->box_y  = locp->y % BOX_HEIGHT;

    return(locp);
}

static int
MyClearPath(Location *lStart, Location *locp)
{
	int bRetCode;

	if (locp->grid_x >= 0 && locp->grid_x < GRID_WIDTH &&
		locp->grid_y >= 0 && locp->grid_y < GRID_HEIGHT)
	{
		bRetCode = clear_path(lStart, locp);
	}
	else
	{
		bRetCode = FALSE;
	}

	return (bRetCode);
}


static int
InitializeVariables(Environment **ppEnvironment)
{
	register int    x, y;
	int bOk = FALSE;
	int iCtr;
	Environment *pEnv;


    /* allocate the Environment structure dynamically, to save stack space */
    pEnv = (Environment *) calloc(1, sizeof(Environment));
	*ppEnvironment = pEnv;

    /* arrange for the Environment structure to get free()ed when I die */
    set_cleanup_func(cleanup, (void *) pEnv);

	if (!pEnv)
	{
		BarfBarf(RECIPIENT_ALL, "Can't get memory!");
		goto error_exit;
	}

#ifdef DEBUG_TARGET_SOLUTIONS
	pEnv->iCalls               = 0;
	pEnv->iIterations          = 0;
	pEnv->iClearPaths          = 0;
	pEnv->iClearPathCalls      = 0;
	pEnv->iBetters             = 0;
	pEnv->iZeros               = 0;
#endif
    pEnv->frPrevious           = -1;
    pEnv->prevTarget.id        = -1;
	pEnv->fprevTime            = -1;
    pEnv->iTurnDir             = -1;
	pEnv->iGoal                = 99;
    pEnv->bHaveTarget          = FALSE;
    pEnv->bTargetIsVehicle     = FALSE;
    pEnv->iSpecTarget          = FALSE;
    pEnv->iWasCalled           = FALSE;
	pEnv->iFriendliesInTheArea = FALSE;
    pEnv->bHaveNewRadar        = FALSE;
    pEnv->bHaveRadar           = FALSE;
	pEnv->bHaveTacLink         = FALSE;
    pEnv->iStatus              = WANDER;
	pEnv->aTurn                = pEnv->stMyVehicle.heading;
	pEnv->bAlwaysRam           = TRUE;
	pEnv->iStills              = 0;
    pEnv->iNumVehicles         = 0;
	pEnv->frLastOpen           = 0;
	pEnv->frLastEnemyAt        = 0;
	pEnv->frHoldAim            = HOLD_TIME;
	pEnv->iTimeOut             = 0;
	pEnv->dest_wall.x          = -1;
	pEnv->stPrevLoc.grid_x = pEnv->stPrevLoc.grid_y = -1;


    pEnv->bHaveNewRadar        = has_special(NEW_RADAR);
    pEnv->bHaveRadar           = has_special(RADAR);
	pEnv->bHaveTacLink         = has_special(TACLINK);
	/* turn these on someday */

    get_settings(&pEnv->stGameSettings);

    if (pEnv->stGameSettings.game == RACE_GAME)
	{
		pEnv->iStatus = SEEK;
		pEnv->iGoal   = GOAL;
	}

    get_self(&pEnv->stMyVehicle);

    pEnv->iNumWeapons = num_weapons();
    pEnv->iAvgAmmoRange = 0;

    for (iCtr = 0; iCtr < pEnv->stMyVehicle.num_turrets; iCtr++)
	{
		pEnv->Turret[iCtr].iAvgAmmoSpeed    = 0;
		pEnv->Turret[iCtr].iAvgAmmoLifetime = 0;
		pEnv->Turret[iCtr].iNumWeapons      = 0;
		pEnv->Turret[iCtr].iMinReload       = 99;
#ifdef CHECK_LAST_FIRE
		pEnv->Turret[iCtr].iLastFire        = -99;
#endif
	}

    for (iCtr = 0; iCtr < pEnv->iNumWeapons; iCtr++)
    {
		MountLocation iMount;

        get_weapon(iCtr, &pEnv->stMyWeapons[iCtr]);

        turn_on_weapon(iCtr);

		iMount = pEnv->stMyWeapons[iCtr].mount;

        if (pEnv->stMyWeapons[iCtr].type == MINE ||
			pEnv->stMyWeapons[iCtr].type == SLICK)
        {
            ;
        }
		else
		if (iMount == MOUNT_TURRET1 ||
		    iMount == MOUNT_TURRET2 ||
		    iMount == MOUNT_TURRET3
#ifdef HAS_TURRET4
		    || iMount == MOUNT_TURRET4
#endif
			)
        {
			int iNumWeapons   = pEnv->Turret[iMount].iNumWeapons;
			int iAmmoSpeed    = pEnv->stMyWeapons[iCtr].ammo_speed;
			int iAmmoLifetime = pEnv->stMyWeapons[iCtr].range /
				    			pEnv->stMyWeapons[iCtr].ammo_speed;

            pEnv->bAlwaysRam = FALSE;
            pEnv->Turret[iMount].iMinReload =
				    MIN(pEnv->Turret[iMount].iMinReload,
				    	pEnv->stMyWeapons[iCtr].reload);

            pEnv->iAvgAmmoRange += pEnv->stMyWeapons[iCtr].range;
            if (iNumWeapons)
			{
				if ((pEnv->Turret[iMount].iAvgAmmoSpeed    / iNumWeapons
						!= iAmmoSpeed) ||
				    (pEnv->Turret[iMount].iAvgAmmoLifetime / iNumWeapons
						!= iAmmoLifetime))
			    {
				    BarfBarf(RECIPIENT_ALL, "Tagman would be better");
				    BarfBarf(RECIPIENT_ALL, "with the same weapons");
				    BarfBarf(RECIPIENT_ALL, "within a turret");
				}
			}

			pEnv->Turret[iMount].iAvgAmmoSpeed    += iAmmoSpeed;
			pEnv->Turret[iMount].iAvgAmmoLifetime += iAmmoLifetime;

			(pEnv->Turret[iMount].iNumWeapons)++;
        }
    }

    {
		int iNumWeapons = 0;

        for (iCtr = 0; iCtr < pEnv->stMyVehicle.num_turrets; iCtr++)
		{
			iNumWeapons += pEnv->Turret[iCtr].iNumWeapons;
		}

		if (iNumWeapons)
		{
            pEnv->iAvgAmmoRange /= iNumWeapons;
		}

		if (pEnv->iAvgAmmoRange > SCREEN_WIDTH / 2 - 2 * MARGIN_OF_ERROR)
		{
			pEnv->iAvgAmmoRange = SCREEN_WIDTH / 2 - 2 * MARGIN_OF_ERROR;
		}

		if (pEnv->iAvgAmmoRange <= 0)
		{
			pEnv->iAvgAmmoRange = SCREEN_WIDTH / 2 - 2 * MARGIN_OF_ERROR;
		}
	}

    for (iCtr = 0; iCtr < pEnv->stMyVehicle.num_turrets; iCtr++)
	{
		if (pEnv->Turret[iCtr].iNumWeapons)
		{
		    pEnv->Turret[iCtr].aRate = turret_turn_rate(iCtr);
			pEnv->Turret[iCtr].iAvgAmmoSpeed /=
				pEnv->Turret[iCtr].iNumWeapons;
		}
	}

    bOk = TRUE;

	pEnv->my_map = map_get();

	if (pEnv->stGameSettings.full_map)
	{
		for (x = 0; x < GRID_WIDTH; ++x)
		{
            if (pEnv->stGameSettings.game != RACE_GAME)
	        {
		        MainLoop(pEnv);
	        }

			for (y = 0; y < GRID_HEIGHT; ++y)
			{
				pEnv->boxnotes[x][y].seen = TRUE;
				NoticeLandmark(pEnv, x, y);
			}
		}
	}

error_exit:

	return (bOk);
}


/* return a random float in the range [0, max) */
static FLOAT frand(FLOAT maxr)
#if 0
FLOAT maxr;						/* maximum value */
#endif
{
	extern long random();

	return (random() & 0xffff) * maxr / 0x10000;
}


static void
TurnOffAllWeapons(void)
{
	WeaponNum wn;

	wn = num_weapons();
	while (wn--)
	{
		turn_off_weapon(wn);
	}
}


static void
TurnOnAllWeapons(void)
{
	WeaponNum wn;

	wn = num_weapons();
	while (wn--)
	{
		turn_on_weapon(wn);
	}
}


static int
NeedFuel(DOUBLE fraction)
#if 0
DOUBLE fraction;				/* I need it if below this */
#endif
{
	return (fuel() < max_fuel() * fraction);
}


static int
NeedArmor(DOUBLE fraction)
#if 0
DOUBLE fraction;				/* I need it if below this */
#endif
{
	Side side;

	for (side = FRONT; side < MAX_SIDES; ++side)
	{
		if (armor(side) < max_armor(side) * fraction)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


static int NeedAmmo(DOUBLE fraction)
#if 0
DOUBLE fraction;				/* I need it if below this */
#endif
{
	int weapon;
	Weapon_info winfo;

	for (weapon = num_weapons(); weapon-- > 0;)
	{
		get_weapon(weapon, &winfo);
		if (weapon_ammo(weapon) < winfo.max_ammo * fraction)
			return (TRUE);
	}
	return (FALSE);
}


static int
AtBoxCenter(Vehicle_info *selfp)
{
	return (ABS(selfp->loc.box_x - BOX_WIDTH / 2) < LANDMARK_WIDTH / 2 &&
		ABS(selfp->loc.box_y - BOX_HEIGHT / 2) < LANDMARK_HEIGHT / 2);
}

/* moves to the center of the current box (presumably a store).  Returns true
   if I am already there. */

static int
GotoBoxCenter(Environment *pEnv)
{
    int iYoff;
    int iXoff;
    Vehicle_info *selfp = &pEnv->stMyVehicle;			/* self */

	if (!AtBoxCenter(selfp))
	{
		if (SIGN(selfp->loc.box_y - BOX_HEIGHT / 2) != SIGN(selfp->yspeed))
		{
			iYoff = -ABS(selfp->yspeed);
		}
		else
		{
			iYoff = ABS(selfp->yspeed);
		}

		if (SIGN(selfp->loc.box_x - BOX_WIDTH  / 2) != SIGN(selfp->xspeed))
		{
			iXoff = -ABS(selfp->xspeed);
		}
		else
		{
			iXoff = ABS(selfp->xspeed);
		}
        {
			float fDriveSpeed;
			int iPercentReplenishing =
				FriendliesReplenishing(pEnv, &pEnv->stMyVehicle.loc);

            if (iPercentReplenishing > 0)
			{
		turn_vehicle(ATAN2(BOX_HEIGHT / 2 - selfp->loc.box_y + iYoff,
						   BOX_WIDTH / 2 - selfp->loc.box_x + iXoff));
				if (iPercentReplenishing +
					PercentOfLandmark[pEnv->stMyVehicle.body] < 100
#define MUST_ALSO_BE_CLOSEST
#ifdef MUST_ALSO_BE_CLOSEST
					&& MyVehicleIsClosest(pEnv)
#endif
					)
				{
				    set_abs_drive(MIN_CLOSING_SPEED_ABS);
				}
				else
				{
				    set_abs_drive(0.0);
				}
				return (FALSE);
			}

	        fDriveSpeed = SQRT(
			    HYPOT(BOX_HEIGHT / 2.0 - selfp->loc.box_y,
                      BOX_WIDTH  / 2.0 - selfp->loc.box_x) *
		        tread_acc() * pEnv->stGameSettings.normal_friction);

				fDriveSpeed = MAX(fDriveSpeed, MIN_CLOSING_SPEED_ABS);

		        turn_vehicle(ATAN2(BOX_HEIGHT / 2 - selfp->loc.box_y + iYoff,
			        			   BOX_WIDTH  / 2 - selfp->loc.box_x + iXoff));

		        set_abs_drive(fDriveSpeed);
#ifdef DEBUG_SEEK_SPEED
				printf("[%d]IN calc speed %f\n", pEnv->stMyVehicle.id,
					   fDriveSpeed);
#endif
		}

		return (FALSE);
	}
	set_abs_drive(0.0);

	return (TRUE);
}

static int
MyVehicleIsClosest(Environment *pEnv)
{
    int iRetCode = TRUE;
    int iNumFound = 0;
	int iDiffX = pEnv->stMyVehicle.loc.box_x - BOX_WIDTH / 2;
	int iDiffY = pEnv->stMyVehicle.loc.box_y - BOX_HEIGHT / 2;
	int iDist = iDiffX * iDiffX + iDiffY * iDiffY;

    if (pEnv->iFriendliesInTheArea)
    {
        int iCtr = pEnv->iFriendliesInTheArea;
        int iCtr2;

        for (iCtr2 = 0; iCtr && iCtr2 < pEnv->iNumVehicles; iCtr2++)
        {
            if (pEnv->stVehicles[iCtr2].team == pEnv->stMyVehicle.team)
            {
				iCtr--;

				if (pEnv->stVehicles[iCtr2].loc.grid_x ==
						pEnv->stMyVehicle.loc.grid_x &&
					pEnv->stVehicles[iCtr2].loc.grid_y ==
						pEnv->stMyVehicle.loc.grid_y)
				{
					if (!AtBoxCenter(&pEnv->stVehicles[iCtr2]))
					{
	                    iDiffX = pEnv->stVehicles[iCtr2].loc.box_x -
									 BOX_WIDTH / 2;
	                    iDiffY = pEnv->stVehicles[iCtr2].loc.box_y -
									 BOX_HEIGHT / 2;
					    if (iDiffX * iDiffX + iDiffY *iDiffY < iDist)
					    {
                            iNumFound++;
							if (iNumFound > 1)
							{
						        iRetCode = FALSE;
						        break;
							}
					    }
					}
				}
			}
		}
	}

	return (iRetCode);
}

static int HandleOutpost(Environment *pEnv)
{
    int bHaveTarget = FALSE;
	int gridX, gridY;	/* current grid coords  */
	int nearX, nearY;	/* coords of nearby box */

	gridX = pEnv->stMyVehicle.loc.grid_x;
	gridY = pEnv->stMyVehicle.loc.grid_y;

	for (nearX = MAX(0, gridX - 2);
		 nearX <= MIN(GRID_WIDTH - 1, gridX + 2);
		 ++nearX)
	{
		for (nearY = MAX(0, gridY - 2);
			 nearY <= MIN(GRID_WIDTH - 1, gridY + 2);
			 ++nearY)
		{
	        if (landmark(nearX, nearY) == OUTPOST)
			{
		        pEnv->target.xspeed     = 0;
		        pEnv->target.yspeed     = 0;
		        pEnv->target.loc.grid_x = nearX;
		        pEnv->target.loc.grid_y = nearY;
		        pEnv->target.loc.box_x  = BOX_WIDTH  / 2;
		        pEnv->target.loc.box_y  = BOX_HEIGHT / 2;
		        pEnv->target.loc.x      = nearX * BOX_WIDTH  + BOX_WIDTH  / 2;
		        pEnv->target.loc.y      = nearY * BOX_HEIGHT + BOX_HEIGHT / 2;

				if (MyClearPath(&pEnv->stMyVehicle.loc, &pEnv->target.loc))
				{
			        bHaveTarget = TRUE;
				}
				break;
			}
		}
		if (bHaveTarget)
		{
			break;
		}
	}

	if (bHaveTarget)
	{
		pEnv->target.id        = -1;
        pEnv->bTargetIsVehicle = FALSE;
		bHaveTarget = Fight(pEnv, TRUE);
	}

	return (bHaveTarget);
}

/* do the appropriate thing with any landmark I might be on.  Returns true if
   something useful is being done. */
static int
HandleLocalLandMark(Environment *pEnv)
{
    Vehicle_info *selfp = &pEnv->stMyVehicle;			/* self */

	switch (landmark(selfp->loc.grid_x, selfp->loc.grid_y))
	{
		case FUEL:
			if (get_money() >= 20)
			{
			    if (NeedFuel(0.4) && AtBoxCenter(selfp))
			    {
				    set_abs_drive(0.0);
				    return (TRUE);
			    }
			    else
			    if (NeedFuel(0.3))
			    {
				    GotoBoxCenter(pEnv);
				    return (TRUE);
			    }
			}
			break;

		case AMMO:
			if (get_money() >= 50 && !NeedFuel(0.1))
			{
			    if (NeedAmmo(1.0) && AtBoxCenter(selfp))
			    {
				    set_abs_drive(0.0);
				    TurnOffAllWeapons();
				    return (TRUE);
			    }
			    else
			    if (NeedAmmo(0.9))
			    {
				    GotoBoxCenter(pEnv);
				    return (TRUE);
			    }
			}
			break;

		case ARMOR:
			if (get_money() >= 490 && !NeedFuel(0.1) &&
				(!OutOfAmmo(FALSE) || !pEnv->bHaveTarget))
			{
			    if (NeedArmor(1.0) && AtBoxCenter(selfp))
			    {
				    set_abs_drive(0.0);
				    return (TRUE);
			    }
			    else
			    if (NeedArmor(0.9))
			    {
				    GotoBoxCenter(pEnv);
				    return (TRUE);
			    }
			}
			break;

		case OUTPOST:
			if (pEnv->stGameSettings.outpost_strength)
			{
				return (FALSE);
			}
			break;

		case PEACE:
			if (!NeedArmor(0.1) && !OutOfAmmo(FALSE) && !NeedFuel(0.1) &&
			    AtBoxCenter(selfp))
			{
				set_abs_drive(0.0);
				return (TRUE);
			}
			else
			if (!NeedArmor(0.1) && !OutOfAmmo(FALSE) && !NeedFuel(0.1))
			{
				GotoBoxCenter(pEnv);
				return (TRUE);
			}
			break;

		default:
			break;
	}
	return (FALSE);				/* didn't find anything to do */
}


static int VectorsCross(x1, y1, xs1, ys1, x2, y2, xs2, ys2)
Coor   x1,  y1;				/* start of first vector */
DOUBLE xs1, ys1;			/* speeds of first vector */
Coor   x2,  y2;				/* start of second vector */
DOUBLE xs2, ys2;			/* speeds of second vector */
{
	double tx, ty;			/* times until "collision" in x and y */

	if (!xs1 && !ys1 && !xs2 && !ys2)
	{
		return (FALSE);
	}

	if (xs1 - xs2)
	{
	    tx = (x2 - x1) / (xs1 - xs2);
	}
	else
	{
		tx = (x1 == x2) ? 1 : 2;
	}

	if (ys1 - ys2)
	{
	    ty = (y2 - y1) / (ys1 - ys2);
	}
	else
	{
		tx = (y1 == y2) ? 1 : 2;
	}

	return (tx >= 0 && tx <= 1 && ty >= 0 && ty <= 1 &&
			ABS(tx - ty) < 0.2);
}


static void
BarfBarf(Byte recipent, ...)
{
	va_list args;
	char msg[88];
	char *fmt;

	va_start(args, recipent);
	fmt = va_arg(args, char *);
	(void) vsprintf(msg, fmt, args);
	send_msg(recipent, OP_TEXT, (Byte *) msg);
	va_end(args);
}


static void
ProcessMessages(Environment *pEnv, int iMaxProcess)
{
	Byte abData[MAX_DATA_LEN];
	int bHaveShot = FALSE;
	int iNumMessages = messages();
	Message mCurrent;
	Opcode  oCurrent;

    if (iNumMessages)
	{
		while (iNumMessages-- && iMaxProcess--)
		{
    		receive_msg(&mCurrent);

            if ((mCurrent.sender_team != pEnv->stMyVehicle.team &&
				 mCurrent.sender != SENDER_COM) ||
#ifdef PROCESS_ONLY_RECENT
			    pEnv->frCurrent - mCurrent.frame > ALWAYS_OLD_AGE ||
#endif
                mCurrent.sender == pEnv->stMyVehicle.id)
			{
				continue;
			}

            oCurrent = mCurrent.opcode;

            if (oCurrent == OP_WHERE_IS)
			{
				int iNumFound = 0;
				int x;
				int y;
				Byte abData[MAX_DATA_LEN];

				abData[0] = mCurrent.data[0];

	            for (x = 0;
					 x < GRID_WIDTH && iNumFound < (MAX_DATA_LEN - 1)/2; ++x)
	            {
		            for (y = 0;
						 y < GRID_HEIGHT && iNumFound < (MAX_DATA_LEN - 1)/2;
						 ++y)
		            {
					    if (pEnv->boxnotes[x][y].seen)
					    {
	                        if (landmark(x, y) == abData[0])
					        {
								abData[1 + iNumFound * 2 + 0] = x;
								abData[1 + iNumFound * 2 + 1] = y;
								iNumFound++;
							}
						}
		            }
	            }

				if (iNumFound)
				{
	                while (iNumFound < (MAX_DATA_LEN - 1)/2)
					{
						abData[1 + iNumFound * 2 + 0] = -1;
						abData[1 + iNumFound * 2 + 1] = -1;
						iNumFound++;
					}

                    send_msg(MAX_VEHICLES + pEnv->stMyVehicle.team,
				             OP_HERE_ARE, abData);
				}
			}
			else
			if (oCurrent == OP_HERE_ARE)
			{
				int iNumFound;
				int x;
				int y;

	            for (iNumFound = 0;
					 iNumFound < (MAX_DATA_LEN - 1)/2; ++iNumFound)
				{
					x = mCurrent.data[1 + iNumFound * 2 + 0];
					y = mCurrent.data[1 + iNumFound * 2 + 1];

				    if (x < 0 || x > GRID_WIDTH ||
						y < 0 || y > GRID_HEIGHT)
					{
						break;
					}

				    pEnv->boxnotes[x][y].seen = TRUE;
					pEnv->my_map[x][y].type = mCurrent.data[0];
				}

				if (pEnv->iStatus == SEEK && mCurrent.data[0] == pEnv->iGoal)
				{
				    StartSeek(pEnv);
				}
			}
			else
			if (oCurrent == OP_ATTACK && pEnv->iStatus == GOTO &&
				pEnv->iCallerId == mCurrent.sender)
			{
				if (mCurrent.data[0] == mCurrent.recipient)
				{
					 pEnv->iStatus = WANDER;
				}
				else
			    if (!pEnv->iSpecTarget)
				{
						pEnv->iSpecTarget = TRUE;
						pEnv->iTargetVehicle = mCurrent.data[0];
				}
			}
			else
			if (oCurrent == OP_OPEN &&
				pEnv->bHaveTarget && pEnv->bTargetIsVehicle)
			{
				SendEnemyAt(pEnv, &pEnv->target, FALSE);
			}
			else
			if (oCurrent == OP_ACK)
			{
			    if (pEnv->bHaveTarget)
			    {
					abData[0] = pEnv->target.id;
				}
				else
				{
					abData[0] = mCurrent.sender;
				}
	            abData[1] = 0;
	            send_msg(mCurrent.sender, OP_ATTACK, abData);
			}
			else
			if (oCurrent == OP_DEATH)
			{
				if (pEnv->iStatus == GOTO &&
				pEnv->iSpecTarget && pEnv->iTargetVehicle == mCurrent.data[0])
				{
					pEnv->iSpecTarget = FALSE;
					StartWandering(pEnv);
				}

				if (mCurrent.data[2] == pEnv->stMyVehicle.id &&
					team(mCurrent.data[0]) != pEnv->stMyVehicle.team)
				{
					static char *pcText[] =
					{
						"Fireworks? Great!",
						"Reach out and touch somebody.",
						"You're it!",
						"You suck rocks!",
						"It was great for me.",
						"Will you still respect me?",
						"Another one bites the dust.",
						"I'm always willing to talk.",
						"Next contestant.",
						"Ennnk, wrong answer.",
						"That was great, NOT!",
						"That was great, psych!",
						"You have a sense of loss.",
						"Join your brother!",
						"You are next.",
						"Mmmm, mmmmm good!",
						"Tagman needs new meat.",
						"Did that hurt?",
						"See'ya later.",
						"Give up, results the same.",
						"ORACLE said to shoot you.",
						"Kurt didn't get the movie!",
						"Leave'm while they're happy",
						"Barf Barf!",
						"8 bit brain, 1 bit bus.",
						"You look tired.",
						"I've fought better bugs.",
						"Shoot first, die last.",
						"Shoot last, die first.",
						"You weren't trying.",
						"Try bigger weapons.",
						"Maybe you're a Gnat?",
						"Why the 9, Katz?",
						"InTheEnd,ThereCanBOnly1",
						"One less road block.",
						"I'm sorry you are.",
						"Chocolate or Vanilla",
						"Let it dry first",
						"Where's Gordon?",
						"Was that it?",
						"Fifty bucks john."
					};
					int iRandomIndex =
						frand((float)sizeof(pcText)/sizeof(char *));

					send_msg(MAX_VEHICLES + team(mCurrent.data[0]), OP_TEXT,
						(Byte *)pcText[iRandomIndex]);
				}
			}
			else
			if ((oCurrent == OP_HELP     || oCurrent == OP_GOTO ||
				 oCurrent == OP_ENEMY_AT) &&
				(pEnv->iStatus == WANDER || pEnv->iStatus == GOTO ||
				 pEnv->iStatus == REPLENISH || pEnv->iStatus == SEEK))
			{
				int bRespond = FALSE;

				if (pEnv->iStatus == REPLENISH || pEnv->iStatus == SEEK)
				{
				    bRespond = FALSE;
                    if (oCurrent == OP_ENEMY_AT)
					{
                        if (!pEnv->bHaveTarget && !bHaveShot)
						{
							int iFramesOld = pEnv->frCurrent - mCurrent.frame;

							if (iFramesOld < LATE_FC)
							{
                            Message2VehicleInfo(mCurrent.data, &pEnv->target);

                            DeltaLoc(&(pEnv->target.loc),
									 iFramesOld * mCurrent.data[6],
									 iFramesOld * mCurrent.data[7]);
                            TurnOnAllWeapons();
						    Fight(pEnv, FALSE);
							if (pEnv->iStatus == REPLENISH)
							{
                                TurnOffAllWeapons();
							}
							bHaveShot = TRUE;
                            pEnv->frHoldAim = pEnv->frCurrent;
							}
						}
					}
				}
				else
				if (pEnv->iStatus == WANDER)
				{
				   bRespond = TRUE;
				}
				else
				{
				   int xUs      = pEnv->stMyVehicle.loc.grid_x;
				   int yUs      = pEnv->stMyVehicle.loc.grid_y;
				   int xDiffNew = mCurrent.data[0] - xUs;
				   int yDiffNew = mCurrent.data[1] - yUs;
				   int xDiffOld = pEnv->iDestX - xUs;
				   int yDiffOld = pEnv->iDestY - yUs;

				   bRespond = (xDiffNew * xDiffNew + yDiffNew * yDiffNew <
				               xDiffOld * xDiffOld + yDiffOld * yDiffOld);

                   if (pEnv->iStatus == GOTO && pEnv->iSpecTarget == TRUE &&
				       oCurrent == OP_ENEMY_AT)
				   {
						if (pEnv->iTargetVehicle == mCurrent.data[8])
						{
							bRespond = TRUE;
						}
				   }
				}

				if (bRespond)
				{
				    pEnv->iDestX     = mCurrent.data[0];
				    pEnv->iDestY     = mCurrent.data[1];
				    pEnv->iWasCalled = TRUE;
				    pEnv->iCallerId  = mCurrent.sender;

					pEnv->iSpecTarget = FALSE;

                    if (oCurrent == OP_ENEMY_AT)
					{
						pEnv->iSpecTarget = TRUE;
						pEnv->iTargetVehicle = mCurrent.data[8];

                        if (!pEnv->bHaveTarget && !bHaveShot)
						{
							int iFramesOld = pEnv->frCurrent - mCurrent.frame;

							if (iFramesOld < LATE_FC)
							{
                            Message2VehicleInfo(mCurrent.data, &pEnv->target);

                            DeltaLoc(&(pEnv->target.loc),
									 iFramesOld * mCurrent.data[6],
									 iFramesOld * mCurrent.data[7]);
						    Fight(pEnv, FALSE);
							bHaveShot = TRUE;
                            pEnv->frHoldAim = pEnv->frCurrent;
							}
						}
					}
					else
					{
                        abData[0] = oCurrent;
				        abData[1] = 0;

				        send_msg(mCurrent.sender, OP_ACK, abData);
					}

					pEnv->bGoalLocated = TRUE;

                    StartGoto(pEnv);
				}
			}

		}
	}
}

static int CanGo(Vehicle_info *pstMyVehicle, Coor xOff, Coor yOff)
{
	Location lDestination;

	memcpy(&lDestination, &pstMyVehicle->loc, sizeof(Location));
	DeltaLoc(&lDestination, xOff, yOff);
	return (MyClearPath(&pstMyVehicle->loc, &lDestination));
}



/* changes a location by the given x and y.  Returns locp. */
/* decides if I can make the specified move without hitting a wall, taking */
/* the vehicle size into account.  This is done by checking the paths of the */
/* vehicle's bounding-box's corners. */
static int
self_clear_path(Location *start, int delta_x, int delta_y)
#if 0
	Location       *start;	/* starting position of vehicle */
	int             delta_x;
	int             delta_y;	/* proposed move */
#endif
{
	Location        s, f;	/* start and finish of a vehicle corner */
	int             w, h;	/* width and height of this vehicle (changes
				 * with orientation) */

	vehicle_size(&w, &h);
	/*
	 * convert to difference from center of vehicle (assume that vehicle
	 * location is center of vehicle; it sucks that we can't get offset_x
	 * and offset_y)
	 */
	w = (w + 1) / 2;
	h = (h + 1) / 2;

	/* upper left */
	s = *start;
	DeltaLoc(&s, -w, -h);
	f = s;
	DeltaLoc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
	{
		return (FALSE);
	}

	/* upper right */
	s = *start;
	DeltaLoc(&s, w, -h);
	f = s;
	DeltaLoc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
	{
		return (FALSE);
	}

	/* lower right */
	s = *start;
	DeltaLoc(&s, w, h);
	f = s;
	DeltaLoc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
	{
		return (FALSE);
	}

	/* lower left */
	s = *start;
	DeltaLoc(&s, -w, h);
	f = s;
	DeltaLoc(&f, delta_x, delta_y);
	if (!clear_path(&s, &f))
	{
		return (FALSE);
	}

	return (TRUE);
}


/* Decides if a box is a goal of my team.  Suitable to pass to navigate(). */
static int
IsGoalBox(Environment *pEnv, int x, int y) /* x, y: grid coords of a box */
{
    int iRetCode = FALSE;

    switch (pEnv->iStatus)
	{
		case GOTO:
		case WANDER:
	        iRetCode = ((x == pEnv->iDestX) && (y == pEnv->iDestY));
			break;

		case SEEK:
	        iRetCode =  (map_landmark(pEnv->my_map, x, y) == pEnv->iGoal &&
		                (map_team(pEnv->my_map, x, y) ==
						 pEnv->stMyVehicle.team ||
		                 map_team(pEnv->my_map, x, y) == NEUTRAL));
			break;

		default:
			iRetCode = FALSE;
			break;
	}

	return (iRetCode);
}


static void
NoticeLandmark(Environment *pEnv, int x, int y) /* x, y: box coordinates */
{
	if (IsGoalBox(pEnv, x, y))
	{
		pEnv->bGoalLocated = TRUE;
	}
}


static void
NoteSurroundings(Environment *pEnv)
{
	int             gx, gy;	/* current grid coords */
	int             x, y;	/* coords of nearby box */

	if (!pEnv->stGameSettings.full_map)
	{			/* %% won't update outposts */
		gx = pEnv->stMyVehicle.loc.grid_x;
		gy = pEnv->stMyVehicle.loc.grid_y;

		for (x = MAX(0, gx - 2); x <= MIN(GRID_WIDTH - 1, gx + 2); ++x)
		{
			for (y = MAX(0, gy - 2); y <= MIN(GRID_WIDTH - 1, gy + 2); ++y)
			{
				pEnv->boxnotes[x][y].seen = TRUE;
				NoticeLandmark(pEnv, x, y);
			}
		}
	}
}


/*
 * find a path from the current location to one that fits destfunc().  Leaves
 * the result in the map and returns success in pEnv->have_route.
 */
static int
navigate(Environment *pEnv, int (*destfunc)(), int through_unseen)
/*
int (*destfunc)(); gets called with pEnv and x and y grid coordinates.
				   Returns true if that box is a destination.
int through_unseen; true if the search should proceed
				   even through boxes that have not been seen yet
*/
{
	/* used to implement breadth-first search through map */
	Coord           queue[MAX_BOXES];
	int             head, tail;	/* indexes into queue[] */
	Coord          *cp;	/* data on box we're currently looking at */
	register int    x, y;
	int             startx, starty;
	WallType        map_val;
	int             bOutOfAmmo = OutOfAmmo(FALSE);

	/* mark all boxes as unsearched */
    {
        BoxNotes *pbox;
	    for (x = 0; x < GRID_WIDTH; ++x)
	    {
			pbox = &pEnv->boxnotes[x][0];

		    for (y = 0; y < GRID_HEIGHT; ++y)
		    {
			    /* pEnv->boxnotes[x][y].navdir = Nodir; */
			    pbox->navdir = Nodir;
				pbox++;
		    }
	    }
	}

	/* current location is start of search */
	startx = pEnv->stMyVehicle.loc.grid_x;
	starty = pEnv->stMyVehicle.loc.grid_y;
	pEnv->boxnotes[startx][starty].navdir = North;	/* anything but Nodir */

	tail = 0;
	queue[0].x = startx;
	queue[0].y = starty;
	head = 1;

	while (tail < head)
	{			/* until queue is empty */
		cp = &queue[tail++];
		x = cp->x;
		y = cp->y;

		if (destfunc(pEnv, x, y))
		{		/* found a destination? */
			/* direction of path from this box */
			/* (i.e the direction from which the search found this box) */
			Direction dir;
			/* direction by which we came to this box on the retrace */
			Direction fromdir;

			pEnv->iDestX = x;
			pEnv->iDestY = y;

			/*
			 * now we are at the goal, with a path behind us
			 * leading back to the start, so we have to retrace
			 * the path, reversing the arrows as we go
			 */
			pEnv->boxnotes[startx][starty].navdir = Nodir;	/* terminates */
			fromdir = Nodir;

			do
			{
				dir = pEnv->boxnotes[x][y].navdir;

				pEnv->boxnotes[x][y].navdir = fromdir;	/* reverse it */

				x += dir2delta[dir].x;
				y += dir2delta[dir].y;

				fromdir = dir2opposite[dir];	/* for next box */
			}
			while (fromdir != Nodir);

			pEnv->have_route = TRUE;

			return (TRUE);
		}

		/* quit now if we shouldn't search further from here */
		if (!through_unseen && !pEnv->boxnotes[cp->x][cp->y].seen)
		{
			continue;
		}


		/*
		 * put all accessible unexplored adjacent boxes into queue
		 * (navdir is used to indicate where the search came _from_)
		 */

        map_val = map_wall_north(x, y);
		if (y > 0 && pEnv->boxnotes[x][y - 1].navdir == Nodir &&
		    map_val != MAP_WALL && (map_val != MAP_DEST || !bOutOfAmmo))
		{
			pEnv->boxnotes[x][y - 1].navdir = South;
			queue[head].x = x;
			queue[head].y = y - 1;
			++head;
		}
        map_val = map_wall_south(x, y);
		if (y < GRID_HEIGHT - 1 && pEnv->boxnotes[x][y + 1].navdir == Nodir &&
		    map_val != MAP_WALL && (map_val != MAP_DEST || !bOutOfAmmo))
		{
			pEnv->boxnotes[x][y + 1].navdir = North;
			queue[head].x = x;
			queue[head].y = y + 1;
			++head;
		}
        map_val = map_wall_west(x, y);
		if (x > 0 && pEnv->boxnotes[x - 1][y].navdir == Nodir &&
		    map_val != MAP_WALL && (map_val != MAP_DEST || !bOutOfAmmo))
		{
			pEnv->boxnotes[x - 1][y].navdir = East;
			queue[head].x = x - 1;
			queue[head].y = y;
			++head;
		}
        map_val = map_wall_east(x, y);
		if (x < GRID_WIDTH - 1 && pEnv->boxnotes[x + 1][y].navdir == Nodir &&
		    map_val != MAP_WALL && (map_val != MAP_DEST || !bOutOfAmmo))
		{
			pEnv->boxnotes[x + 1][y].navdir = West;
			queue[head].x = x + 1;
			queue[head].y = y;
			++head;
		}
	}
	pEnv->have_route = FALSE;

	return (FALSE);
}


/*
 * searches down the path the navigator found for short-cuts: clear lines
 * from my vehicle's current position to boxes later on in the route. Returns
 * the angle to go in (occaisonally BAD_ANGLE).
 */
static Angle
recursive_short_cut(Environment *pEnv, int gx, int gy, int depth)
#if 0
int   gx, gy;	/* box to check out (initially the box my vehicle is in) */
int   depth;	/* how many boxes down the path to search */
#endif
{
	Angle           a;
	Direction       dir;
	int             dx, dy;

	dir = pEnv->boxnotes[gx][gy].navdir;	/* direction navigator says
						 * to go from this box */
	if (dir == Nodir)
	{
		return BAD_ANGLE;
	}
	/* remember the first destructible wall along our path */
	/* so we can shoot at it */
	if (pEnv->dest_wall.x == -1)
	{
		switch (dir)
		{
		    case North:
			    if (map_wall_north(gx, gy) == MAP_DEST)
			    {
				    pEnv->dest_wall.x = gx;
				    pEnv->dest_wall.y = gy;
			    }
			    break;

		    case South:
			    if (map_wall_south(gx, gy) == MAP_DEST)
			    {
				    pEnv->dest_wall.x = gx;
				    pEnv->dest_wall.y = gy;
			    }
			    break;

		    case East:
			    if (map_wall_east(gx, gy) == MAP_DEST)
			    {
				    pEnv->dest_wall.x = gx;
				    pEnv->dest_wall.y = gy;
			    }
			    break;

		    case West:
			    if (map_wall_west(gx, gy) == MAP_DEST)
			    {
				    pEnv->dest_wall.x = gx;
				    pEnv->dest_wall.y = gy;
			    }
			    break;

		    default:
			    printf("direction bug!\n");
		}
	}
	/*
	 * if not at bottom of recursion, first check for shortcuts to the
	 * rest of the route (which will be better shortcuts than any
	 * shortcut from here)
	 */
	if (depth > 0)
	{
		a = recursive_short_cut(pEnv,
								gx + dir2delta[dir].x, gy + dir2delta[dir].y,
					            depth - 1);
		if (a != BAD_ANGLE)
		{
			return (a);
		}
	}
	/*
	 * check to see if there is a clear linear path between my vehicle
	 * and the part of the destination box where we want to be
	 */

	/* %% */
	dx = ((gx * BOX_WIDTH) + dir2exit[dir].x) - pEnv->stMyVehicle.loc.x;
	dy = ((gy * BOX_HEIGHT) + dir2exit[dir].y) - pEnv->stMyVehicle.loc.y;
	if (self_clear_path(&(pEnv->stMyVehicle.loc), dx, dy))
	{
		return (ATAN2(dy, dx));	/* found shortcut, so return its angle */
	}

	return (BAD_ANGLE);	/* didn't find the shortcut */
}


/* follow the route that navigate() produced */
static void
FollowRoute(Environment *pEnv)
{
	Angle           angle;
	Location      lDestination;

	if (!pEnv->have_route)
	{
		printf("I don't have a route!\n");
		return;
	}

	if (pEnv->iDestX == pEnv->stMyVehicle.loc.grid_x &&
		pEnv->iDestY == pEnv->stMyVehicle.loc.grid_y)
	{
		set_rel_drive(0.0);
		StartWandering(pEnv);

		return;
	}

	lDestination.grid_x = pEnv->iDestX;
	lDestination.grid_y = pEnv->iDestY;
	lDestination.box_x  = BOX_WIDTH  / 2;
	lDestination.box_y  = BOX_HEIGHT / 2;
	lDestination.x      = pEnv->iDestX * BOX_WIDTH  + BOX_WIDTH  / 2;
	lDestination.y      = pEnv->iDestY * BOX_HEIGHT + BOX_HEIGHT / 2;

    if (self_clear_path(&pEnv->stMyVehicle.loc,
		lDestination.x - pEnv->stMyVehicle.loc.x,
		lDestination.y - pEnv->stMyVehicle.loc.y))
	{
#ifdef DEBUG_DIRECT_DRIVE
		    printf("[%d] %d, %d\n", pEnv->stMyVehicle.id,
				pEnv->iDestX, pEnv->iDestY);
#endif
		turn_vehicle(ATAN2(lDestination.y - pEnv->stMyVehicle.loc.y,
		                   lDestination.x - pEnv->stMyVehicle.loc.x));
		set_rel_drive(pEnv->fDriveSpeed);
	    if (!pEnv->bHaveTarget)
	    {
		    turn_all_turrets(angle);	/* HandleWeapons() may override */
	    }
		return;
	}

	pEnv->dest_wall.x = -1;	/* no destructible wall yet */
	angle = recursive_short_cut(pEnv, pEnv->stMyVehicle.loc.grid_x,
			             	    pEnv->stMyVehicle.loc.grid_y, SHORT_CUT_DEPTH);
	if (angle == BAD_ANGLE)
	{
		/* assume destructible wall */
		int             gx = pEnv->stMyVehicle.loc.grid_x;
		int             gy = pEnv->stMyVehicle.loc.grid_y;
		Direction       navdir = pEnv->boxnotes[gx][gy].navdir;
		int             dx = dir2exit[navdir].x - pEnv->stMyVehicle.loc.box_x;
		int             dy = dir2exit[navdir].y - pEnv->stMyVehicle.loc.box_y;

        if (navdir == Nodir)
		{
		    set_rel_drive(0.0);
#ifdef DEBUG_FOLLOW_ROUTE
		    printf("[%d]I must have slid off course!\n",
				   pEnv->stMyVehicle.id);
#endif
	        if (!navigate(pEnv, IsGoalBox, TRUE))
			{
		        if (!navigate(pEnv, unseen_box, FALSE))
				{
					StartWandering(pEnv);
				}
			}
		}
		else
		{
			if (pEnv->have_route)
			{
#ifdef DEBUG_FOLLOW_ROUTE
			    printf("\n@%d,%d going %d,%d[%d] %d",
					   pEnv->stMyVehicle.loc.grid_x,
		               pEnv->stMyVehicle.loc.grid_y, dx, dy,
				       pEnv->stMyVehicle.id, navdir);
#endif
		        angle = ATAN2(dy, dx);
		        turn_vehicle(angle);
	            if (!navigate(pEnv, IsGoalBox, TRUE))
			    {
		            if (!navigate(pEnv, unseen_box, FALSE))
				    {
					    StartWandering(pEnv);
				    }
			    }
		        set_rel_drive(9.0);
			}
			else
			{
		        set_rel_drive(0.0);
	            if (!navigate(pEnv, IsGoalBox, TRUE))
			    {
		            if (!navigate(pEnv, unseen_box, FALSE))
				    {
					    StartWandering(pEnv);
				    }
			    }
			}
		}
	}
	else
	{
		turn_vehicle(angle);
		set_rel_drive(pEnv->fDriveSpeed);
	    if (!pEnv->bHaveTarget)
	    {
		    turn_all_turrets(angle);	/* HandleWeapons() may override */
	    }
	}
}


#ifdef DEBUG_TIME
static void
check_clock(Environment *pEnv)
{
	int             previous_frame;

	previous_frame  = pEnv->frCurrent;
	pEnv->frCurrent  = frame_number();

	if (pEnv->frCurrent != previous_frame)
	{
		char aBuf[20];
		int iFrames = pEnv->frCurrent - previous_frame;

		aBuf[0] = iFrames + '0';
		aBuf[1] = '\n';
		aBuf[2] = '\0';

		puts(aBuf);
	}
#ifdef DEBUG_TARGET_SOLUTIONS
	else
	{
	    pEnv->iZeros++;
	}
#endif
}
#endif


static void
HandleWeapons(Environment *pEnv)
{
	BoxNotes   *np;

	if (pEnv->dest_wall.x != -1)
	{
		/* destructible wall in our path? */
		/* target is middle of wall       */
		if (!OutOfAmmo(FALSE))
		{
		    np = &pEnv->boxnotes[pEnv->dest_wall.x][pEnv->dest_wall.y];
		    pEnv->target.loc.box_x  = dir2exit[np->navdir].x;
		    pEnv->target.loc.box_y  = dir2exit[np->navdir].y;
		    pEnv->target.loc.grid_x = pEnv->dest_wall.x;
		    pEnv->target.loc.grid_y = pEnv->dest_wall.y;
		    pEnv->target.loc.x      = pEnv->target.loc.box_x +
							          BOX_WIDTH * pEnv->target.loc.grid_x;
		    pEnv->target.loc.y      = pEnv->target.loc.box_y +
							          BOX_HEIGHT * pEnv->target.loc.grid_y;
            pEnv->target.xspeed     = 0;
            pEnv->target.yspeed     = 0;
		    pEnv->bTargetIsVehicle = FALSE;
		    /*****************************************************************/
		    /* NEED_WORK                                                     */
		    /* We're going to have problems when we use features of a target */
		    /* other than speed and position(size?, et al)                   */
		    /*****************************************************************/
		    Fight(pEnv, TRUE);
		    pEnv->frHoldAim         = pEnv->frCurrent;
		}
		else
		{
			/* check to see if we have mines and turn appropreately and */
			/* dump them on the wall, for now restart                   */
			pEnv->iStatus = RESTART;
		}
	}
}

static void
PositionTurrets(Environment *pEnv)
{
    Angle aAims[MAX_TURRETS];
	int iNumTurrets = pEnv->stMyVehicle.num_turrets;

    if (pEnv->frCurrent - pEnv->frHoldAim >= HOLD_TIME)
	{
	if (iNumTurrets)
	{
		int xUs = pEnv->stMyVehicle.loc.grid_x;
		int yUs = pEnv->stMyVehicle.loc.grid_y;
		int iNumBlips;
		Blip_info BlipInfo[MAX_BLIPS];

		MyGetBlips(pEnv, &iNumBlips, BlipInfo);
		{
			if (iNumBlips)
			{
				int iCtr;
				int iCtr2;

				for (iCtr2 = 3; iCtr2 < 8 && iNumTurrets; iCtr2++)
				{
				    for (iCtr = 0; iCtr < iNumBlips && iNumTurrets; iCtr++)
				    {
						if (pEnv->bHaveNewRadar && pEnv->bHaveTacLink &&
		                    BlipInfo[iNumBlips].friend)
						{
							continue;
						}
				       if (ABS(BlipInfo[iCtr].x - xUs) +
				           ABS(BlipInfo[iCtr].y - yUs) < iCtr2)
    			       {
					       iNumTurrets--;
                           aAims[iNumTurrets] =
								   fixed_angle(ATAN2(BlipInfo[iCtr].y - yUs,
								                     BlipInfo[iCtr].x - xUs));
			               turn_turret(iNumTurrets, aAims[iNumTurrets]);
				       }
				    }

				    if (iNumTurrets != pEnv->stMyVehicle.num_turrets)
				    {
		                while (iNumTurrets)
		                {
					       iNumTurrets--;
			               turn_turret(iNumTurrets, aAims[iNumTurrets + 1]);
		                }
				    }
				}
		    }
		}

		while (iNumTurrets)
		{
		   iNumTurrets--;

		   PositionTurret(pEnv, iNumTurrets);
		}
	}
	}
}


static void
PositionTurret(Environment *pEnv, int iTurret)
{
    int xvUs = pEnv->stMyVehicle.xspeed;
    int yvUs = pEnv->stMyVehicle.yspeed;
	Angle aOffset = (2.0 * PI - xvUs - yvUs);

    if (pEnv->stMyVehicle.num_turrets)
	{
	     aOffset /= pEnv->stMyVehicle.num_turrets;
	}
    switch (iTurret)
	{
		case 2:
			turn_turret(1, fixed_angle(pEnv->stMyVehicle.heading -
							   		   pEnv->iTurnDir * aOffset));
			break;


		case 1:
			   turn_turret(1, fixed_angle(pEnv->stMyVehicle.heading +
							   			  pEnv->iTurnDir * aOffset));
			   break;

		default:
		case 0:
		       turn_turret(iTurret, pEnv->stMyVehicle.heading);
			   break;
    }
}


static void
VehicleInfo2Message(Vehicle_info *pstVehicle, Byte *pbBuffer)
{
    pbBuffer[0]  = (int) pstVehicle->loc.grid_x;
    pbBuffer[1]  = (int) pstVehicle->loc.grid_y;
    pbBuffer[2]  = (int) pstVehicle->loc.box_x;
    pbBuffer[3]  = (int) pstVehicle->loc.box_y;
    pbBuffer[6]  = (int) pstVehicle->xspeed + SPEED_BIAS;
    pbBuffer[7]  = (int) pstVehicle->yspeed + SPEED_BIAS;
    pbBuffer[8]  = (int) pstVehicle->id;
    pbBuffer[9]  = (int) pstVehicle->team;
    pbBuffer[10] = (int) pstVehicle->body;
    pbBuffer[11] = (int) pstVehicle->num_turrets;
    pbBuffer[12] = 0x00;
}


static void
Message2VehicleInfo(Byte *pbBuffer, Vehicle_info *pstVehicle)
{
    pstVehicle->loc.grid_x  = (int)   pbBuffer[0];
    pstVehicle->loc.grid_y  = (int)   pbBuffer[1];
	pstVehicle->loc.box_x   = (int)   pbBuffer[2];
    pstVehicle->loc.box_y   = (int)   pbBuffer[3];
    pstVehicle->loc.x       = (int)   pstVehicle->loc.grid_x * BOX_WIDTH +
									  pstVehicle->loc.box_x;
    pstVehicle->loc.y       = (int)   pstVehicle->loc.grid_y * BOX_HEIGHT +
									  pstVehicle->loc.box_y;
    pstVehicle->xspeed      = (float) pbBuffer[6] - SPEED_BIAS;
    pstVehicle->yspeed      = (float) pbBuffer[7] - SPEED_BIAS;
    pstVehicle->id          = (ID)    pbBuffer[8];
    pstVehicle->team        = (Team)  pbBuffer[9];
    pstVehicle->body        = (int)   pbBuffer[10];
    pstVehicle->num_turrets = (int)   pbBuffer[11];
}



static FLOAT
GetDodgeSpeed(Environment *pEnv, FLOAT fDefault)
{
    int         iNumBullets;
	int         iCtr;
	int         iCtr2;
	int         iCtr3;
#define NUM_SPEEDS 3
	int         iDamage[NUM_SPEEDS];
	int         iErrorMargin;
	int         iProtection;
	int         iFinalDamage = 9999;
    int         xUs[NUM_SPEEDS];
    int         yUs[NUM_SPEEDS];
	int         xvUsNew[NUM_SPEEDS];
	int         yvUsNew[NUM_SPEEDS];
	Boolean     bAssumeHover;
    float       fSpeed = fDefault;
	float       fSpeeds[NUM_SPEEDS];
	Bullet_info biBullets[MAX_BULLETS];

	get_bullets(&iNumBullets, biBullets);

#define X /* printf("t%d\n", __LINE__); */

    if (iNumBullets)
	{
		int   xvUs = pEnv->stMyVehicle.xspeed;
		int   yvUs = pEnv->stMyVehicle.yspeed;
		float fCurSpeed = HYPOT(xvUs, yvUs);

X
		bAssumeHover    = (tread_acc() == 0.50);
X
		iErrorMargin    = pEnv->stMyVehicle.body * 8;
X /* */
		iErrorMargin    *= iErrorMargin;
X
        iProtection     = protection();
X

		for (iCtr2 = 0; iCtr2 < NUM_SPEEDS; iCtr2++)
		{
X
			xvUsNew[iCtr2] = xvUs;
/* X */
			yvUsNew[iCtr2] = yvUs;
X

			iDamage[iCtr2] = 0;

			switch (iCtr2)
			{
				case 0:
					fSpeeds[0] = fCurSpeed - tread_acc() *
							pEnv->stGameSettings.normal_friction;
					break;

				case 1:
					fSpeeds[1] = fCurSpeed;
					break;

				case 2:
					fSpeeds[2] = fCurSpeed + acc();
					break;
			}

X
			fSpeeds[iCtr2] = MAX(fSpeeds[iCtr2], -max_speed());
X
			fSpeeds[iCtr2] = MIN(fSpeeds[iCtr2],  max_speed());

			xvUsNew[iCtr2] = COS(pEnv->stMyVehicle.heading) * fSpeeds[iCtr2];
X /* */
			yvUsNew[iCtr2] = SIN(pEnv->stMyVehicle.heading) * fSpeeds[iCtr2];
X

			xUs[iCtr2] = pEnv->stMyVehicle.loc.x;
X
			yUs[iCtr2] = pEnv->stMyVehicle.loc.y;

			fSpeeds[iCtr2] = 9.0 * fSpeeds[iCtr2] / max_speed();
X
		}

        for (iCtr = 0; iCtr < iNumBullets; iCtr++)
		{
			int dx;
			int dy;
			int dis;
			Bullet_info *pBullet = &biBullets[iCtr];
			int iTakenDamage = MAX(aiDamage[pBullet->type] - iProtection, 0);

/* X */
			if (((pBullet->type == MINE || pBullet->type == SLICK)
				&& bAssumeHover) || !iTakenDamage)
			{
X
				continue;
			}

X
			dx = pBullet->loc.x - pEnv->stMyVehicle.loc.x;
X
			dy = pBullet->loc.y - pEnv->stMyVehicle.loc.y;
X /* */
			dis = dx * dx + dy * dy;
			if (dis > 500 && dis < 40000)
			{
X
			    for (iCtr3 = 0; iCtr3 < NUM_SPEEDS; iCtr3++)
				{
					float sx = pBullet->xspeed - xvUsNew[iCtr3];
					float sy = pBullet->yspeed - yvUsNew[iCtr3];
					float sumsq = sx * sx + sy * sy;

X
					if (dy * sy + dx * sx < 0.0)
					{
						float diff = dx * sy - dy * sx;
X
						if (diff * diff < sumsq * 400.0)
						{
/* X */
					        iDamage[iCtr3] += iTakenDamage;
						}
					}
				}
			}
		}

        iCtr2 = NUM_SPEEDS;
X

		while (iCtr2--)
		{
X
			if (iDamage[iCtr2] < iFinalDamage)
			{
X
				iFinalDamage = iDamage[iCtr2];
X /* */
		        fSpeed       = fSpeeds[iCtr2];
			}
		}
	}

	if (fSpeed == fSpeeds[0])
	{
		if (fDefault < fSpeed)
		{
X
			fSpeed = fDefault;
		}
		else
		{
			if (fSpeed == fSpeeds[2]) {
				if (fDefault > fSpeed)
				{
X
					fSpeed = fDefault;
				}
			}
		}
	}

	return (fSpeed);
}

static void
SendEnemyAt(Environment *pEnv, Vehicle_info *pstVehicle, Boolean bSpace)
{
    Byte abData[MAX_DATA_LEN];
	int  iSpacing = 1;

#ifdef DONT_SEND_EVERY_TIME
	if (bSpace)
	{
		iSpacing++;
	}
#endif

    if (pEnv->frCurrent - pEnv->frLastEnemyAt >= iSpacing)
	{
		pEnv->frLastEnemyAt = pEnv->frCurrent;

        VehicleInfo2Message(pstVehicle, abData);

        send_msg(MAX_VEHICLES + pEnv->stMyVehicle.team,
            OP_ENEMY_AT, abData);
    }
}

static int
OutOfAmmo(Boolean bCountDroppedWeapons)
{
    int weapon;
    Weapon_info winfo;

    for (weapon = num_weapons(); weapon-- > 0;)
    {
        get_weapon(weapon, &winfo);
        if (bCountDroppedWeapons ||
            (winfo.type != MINE && winfo.type != SLICK))
        {
            if (weapon_ammo(weapon))
            {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


static void
FindEnemy(Environment *pEnv)
{
	int iNumBlips;
	int iClosestDistance = 0;
	int iNewDistance = 0;
	Boolean bGoThere;
	Boolean bFoundOne = FALSE;
	Blip_info blip_info[MAX_BLIPS];
	int iOurX  = pEnv->stMyVehicle.loc.grid_x;
	int iOurY  = pEnv->stMyVehicle.loc.grid_y;

	MyGetBlips(pEnv, &iNumBlips, blip_info);
	while (iNumBlips--)
	{
		bGoThere = FALSE;
		if (!blip_info[iNumBlips].friend)
		{
			int iNewDX = iOurX - blip_info[iNumBlips].x;
			int iNewDY = iOurY - blip_info[iNumBlips].y;

			if (!iClosestDistance)
			{
				bGoThere = TRUE;
				iClosestDistance = iNewDX * iNewDX + iNewDY * iNewDY;
			}
			else
            {
                iNewDistance = iNewDX * iNewDX + iNewDY * iNewDY;
				if (iClosestDistance > iNewDistance)
				{
					iClosestDistance = iNewDistance;
					bGoThere         = TRUE;
				}
			}

			if (bGoThere)
			{
				bFoundOne    = TRUE;
			    pEnv->iDestX = blip_info[iNumBlips].x;
			    pEnv->iDestY = blip_info[iNumBlips].y;
		    }
	    }
	}

	if (bFoundOne)
	{
		StartGoto(pEnv);
	}
}

static int
MyGetBlips(Environment *pEnv, int *piNumBlips, Blip_info *abiBlips)
{
    int iTry = 0;

    *piNumBlips = 0;

    if (pEnv->bHaveNewRadar)
	{
        if (switch_special(NEW_RADAR, SP_on) == SP_on)
	    {
		    iTry = 1;
	    }
	}

    if (pEnv->bHaveRadar)
	{
        if (switch_special(RADAR, SP_on) == SP_on)
	    {
	    	iTry = 1;
	    }
	}

	if (iTry)
	{
        if (get_blips(piNumBlips, abiBlips) == BAD_VALUE)
	    {
			*piNumBlips = 0;
	    }

        if (pEnv->bHaveNewRadar)
	    {
            switch_special(NEW_RADAR, SP_off);
		}
        if (pEnv->bHaveRadar)
	    {
            switch_special(RADAR, SP_off);
		}
	}

	return (*piNumBlips);
}

static int
unseen_box(Environment *pEnv, Coor coX, Coor coY)
{
	return (!pEnv->boxnotes[coX][coY].seen);
}

