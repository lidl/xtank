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

/*****************************************************************************\
* weapon-defs.h - part of XTank						      *
* 									      *
* This file contains weapon definitions.				      *
* 									      *
* The definitions are presented in the form of a macro invokation with	      *
* several arguments belonging to various contexts.  This file is #included in *
* different places under differnt definitions of the macro, each of which     *
* selects the argument(s) relevant at that point.  Kind of obscure, but it    *
* keeps all the data in the same place.					      *
* 									      *
* The macro should have the general form:				      *
* 									      *
* #define QQ(sym,type,dam,rng,ammo,tm,spd,wgt,spc,mspc,fr,ht,ac,cost,safety) ...
* 									      *
* The args are: internal symbol used in the code, text name, bullet damage,   *
* bullet range, max ammo, reload time, bullet speed, weight, space, mount     *
* space, frames, bullet heat, bullet cost, weapon cost, refill speed, height, *
* safety                                                                      *
\*****************************************************************************/

/**** HAK MEL notes (3/24/93)
 *
 *  The RR macro behaves in the same way as the QQ.  It was created because
 *  the QQ macro would have been damn long....
 *
 *  mspc:  mount space (used to limit the number of weapons on a side)
 *  saftey:  How many frames before the bullet can do damage
 *  num_views:  How many bitmaps for the bullet
 *  mount:  Where the weapon can be mounted
 *  other_flg:  Misc. flags
 *  create_flg:  bullet creation
 *  disp_flg:  how the bullet is displayed
 *  move_flg:  how the bullet moves
 *  dam_flg:  damage type and side
 *  create_func, disp_func, move_func, dam_func:  functions handling special cases
 *
 *******/

#define BIG ((int) ((unsigned)(~0) >> 1))

#ifndef NO_GAME_BALANCE

/* sym       type                 dam rng ammo tm  spd  wgt   spc  mspc  fr  ht  a$   cost  refl safety hgt */
/*n_views mount o_flgs  creat_flgs      disp_flgs    move_flgs   hit_flgs cr_f di_f upd_f hit_f */

QQ(LMG,      "Light Machine Gun",  1, 360, 300, 2, 17,   20,  200,  200, 22,  0,  1,   1000,   1,   0, NORM)
RR(   1, M_ALL,  1,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(MG,       "Machine Gun",        2, 360, 250, 3, 17,   50,  225,  225, 22,  1,  2,   2200,   1,   0, NORM)
RR(   1, M_ALL,  1,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(HMG,      "Heavy Machine Gun",  3, 360, 200, 3, 17,  100,  250,  250, 22,  2,  3,   3000,   1,   0, NORM)
RR(   1, M_ALL,  2,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(LCANNON,  "Light Autocannon",   3, 400, 250, 3, 22,  200,  300,  300, 19,  3,  4,   3000,   1,   3, NORM)
RR(   1, M_ALL,  2,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(CANNON,   "Autocannon",         4, 400, 225, 3, 22,  300,  350,  350, 19,  4,  5,   6000,   1,   3, NORM)
RR(   1, M_ALL,  3,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(HCANNON,  "Heavy Autocannon",   5, 400, 200, 3, 22,  500,  400,  400, 19,  5,  6,  10000,   1,   3, NORM)
RR(   1, M_ALL,  4,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(LROCKET,  "Light Rkt Launcher", 6, 600, 150, 8, 40,  600,  800,  800, 15,  4,  8,   7000,   2,   3, NORM)
RR(   1, M_ALL,  5,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(ROCKET,   "Rkt Launcher",       7, 600, 125, 8, 40,  900, 1200, 1200, 15,  6, 10,  10000,   2,   3, NORM)
RR(   1, M_ALL,  6,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(HROCKET,  "Heavy Rkt Launcher", 8, 600, 100, 8, 40,  900, 1600, 1600, 15,  8, 12,  15000,   2,   3, NORM)
RR(   1, M_ALL,  7,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(ACID,     "Acid Sprayer",       4, 160, 100, 3, 10,  600,  700,  700, 17,  0, 10,  10000,   1,   0, NORM)
RR(   1, M_ALL,  9,      NORM,           F_GR,         NORM,       NORM,    0,   0,   0,   0)

QQ(FLAME,    "Flame Thrower",      3, 200, 300, 1, 12,  700,  500,  500, 17,  1,  2,   4000,   1,   0, NORM)
RR(   1, M_ALL,  9,      NORM,           F_GR,         NORM,       NORM,    0,   0,   0,   0)

QQ(SEEKER,   "Heat Seeker",        8, 1250, 15,15, 25, 1000, 1800, 1800, 51,  9, 50,  20000,  12,   3, HIGH)
RR(   1, M_ALL, 10,    F_NREL,        F_TRL|F_VI,      NORM,       NORM,   0, 0,update_seeker,0)

QQ(PROCKET,  "Pocket Rocket",      5, 1160, 24, 2, 40, 1200, 1900, 1900, 30,  9, 10,  17000,   2,   3, NORM)
RR(   1, M_ALL, 10,      NORM,          F_TRL,         NORM,       NORM,    0,   0,   0,   0)

QQ(UMISSLE,  "Unguided Missle",   10, 1960, 30, 8, 35, 1000, 1800, 1800, 57, 12, 25,  18000,   2,   3, NORM)
RR(   1, M_ALL,  0,      NORM,          F_TRL,         NORM,       NORM,    0,   0,   0,   0)

QQ(TELE,     "TeleGuided",        64, 6000, 1,  1, 15, 1500, 2000, 2000,400, 30,500,  30000, 200,   3, HIGH)
RR(   1,M_ALL,F_CHO,  F_NREL, F_TELE|F_TRL|F_RE|F_TAC, F_KEYB,    NORM,    0,   0,   0,   0)

QQ(TOW,      "TOW Missile",       64, 7500, 2,  1, 15, 1500, 2000, 2000,500, 30,100,  30000, 100,   3, HIGH)
RR(   1,M_ALL,F_CHO,   F_NREL,    F_TRL|F_RE|F_TAC,  F_KEYB,       NORM,    0,   0,   0,   0)

QQ(LTORP,    "Land Torpedo",      20, 5000, 2,  1, 10, 1500, 2000, 2000,500, 30,100,  30000, 100,   3, LOW)
RR(  16,M_SIDES,F_CHO,   F_NREL,           F_RE,       F_KEYB,       NORM,    0,   0,   0,   0)

QQ(BLAST,    "Blast Cannon",       1, 500,  20, 8, 25,  300,  350,  350, 20,  6, 20, 100000,   2,   3, NORM)
RR(   1, M_ALL,  0,      NORM,           F_RE,         NORM,       NORM,   0,  0,  0,hit_blast)

QQ(LASER,    "Pulse Laser",        3,1200, 500, 3, 60,  300,  250,  250, 20,  3,  3,  30000,   1,   0, NORM)
RR(   1, M_ALL,  0,    F_NREL,       F_BEAM|F_NOPT,    NORM,       NORM,    0,   0,   0,   0)

QQ(MINE,     "Mine Layer",         6,  50,  50, 2, 10, 1000, 1000, 1000, 70,  2, 10,   8000,   4,   0, LOW)
RR(  39, M_BACK,  0,      NORM, F_ROT|F_NOHD|F_NOPT,  F_MINE,    F_HOVER,    0,   0,   0,   0)

QQ(SLICK,    "Oil Slick",          0,  50,  50, 5, 10,  300,  500,  500, 70,  0, 10,   2000,   1,   0, LOW)
RR(   1, M_BACK,  0,     F_CR3,    F_NOHD|F_NOPT,     F_MINE,     F_SLICK,   0,   0,   0,   0)

QQ(MORTAR,   "Heavy Mortar",     170, 7500,  10,20, 50, 1400, 2000, 2000, 70, 30,500,  9000,  30,   0, FLY)
RR(   1,M_TURRET,0, F_MAP|F_NREL, F_NOHD|F_TAC|F_TRL, F_DET,       AREA, creat_mort,0,update_mortar,0)

QQ(NUKE,     "Tactical Nuke",    170,  50,  10,20, 10, 1600, 2200, 2200, 30,  6,500,  80000,  30,   0, NORM)
RR(   1,M_BACK, 0,      NORM,         F_NOHD,   F_MINE|F_DET, AREA|F_NOHIT,  0,  0,  0,  0)

QQ(HARM,     "Anti-Radiation",    64, 2500, 2, 192,25, 2000, 3400, 3400,100, 24,1000,100000, 192,   0, FLY)
RR(   1,M_LR,F_CHO, F_MAP|F_NREL,F_TRL|F_NOHD|F_TAC,   NORM,       NORM, creat_harm,0,update_harm,0)

/* If it is to show up in the vdesign menus, it must be before Disc Shooter */
QQ(DISC,     "Disc Shooter",       0,  BIG, 1, BIG, 0,   0,     0,    0,BIG,  0,   0,     0,    0,  0, 0)
RR(   1, M_ALL,  0,             0,          0,        0,          0,       0,    0,       0,     0)

#else /* no game balance */

/* sym       type                 dam rng ammo tm  spd  wgt   spc  mspc  fr  ht  a$   cost  refl safety hgt */
/*n_views mount o_flgs  creat_flgs      disp_flgs    move_flgs   hit_flgs cr_f di_f upd_f hit_f */

QQ(LMG,      "Light Machine Gun",  1, 360, 300, 2, 17,   20,  200,  200, 22,  0,  1,   1000,   1,   0, NORM)
RR(   1, M_ALL,  1,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(MG,       "Machine Gun",        2, 360, 250, 3, 17,   50,  225,  225, 22,  1,  2,   2200,   1,   0, NORM)
RR(   1, M_ALL,  1,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(HMG,      "Heavy Machine Gun",  3, 360, 200, 3, 17,  100,  250,  250, 22,  2,  3,   3000,   1,   0, NORM)
RR(   1, M_ALL,  2,      NORM,           F_BL,         NORM,       NORM,    0,   0,   0,   0)

QQ(LCANNON,  "Light Autocannon",   3, 400, 250, 3, 22,  200,  300,  300, 19,  3,  4,   3000,   1,   3, NORM)
RR(   1, M_ALL,  2,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(CANNON,   "Autocannon",         4, 400, 225, 3, 22,  300,  350,  350, 19,  4,  5,   6000,   1,   3, NORM)
RR(   1, M_ALL,  3,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(HCANNON,  "Heavy Autocannon",   5, 400, 200, 3, 22,  500,  400,  400, 19,  5,  6,  10000,   1,   3, NORM)
RR(   1, M_ALL,  4,      NORM,           F_OR,         NORM,       NORM,    0,   0,   0,   0)

QQ(LROCKET,  "Light Rkt Launcher", 6, 600, 150, 8, 40,  600,  800,  800, 15,  4,  8,   7000,   2,   3, NORM)
RR(   1, M_ALL,  5,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(ROCKET,   "Rkt Launcher",       7, 600, 125, 8, 40,  900, 1200, 1200, 15,  6, 10,  10000,   2,   3, NORM)
RR(   1, M_ALL,  6,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(HROCKET,  "Heavy Rkt Launcher", 8, 600, 100, 8, 40,  900, 1600, 1600, 15,  8, 12,  15000,   2,   3, NORM)
RR(   1, M_ALL,  7,      NORM,           F_YE,         NORM,       NORM,    0,   0,   0,   0)

QQ(ACID,     "Acid Sprayer",       4, 160, 100, 3, 10,  600,  700,  700, 17,  0, 10,  10000,   1,   0, NORM)
RR(   1, M_ALL,  9,      NORM,           F_GR,         NORM,       NORM,    0,   0,   0,   0)

QQ(FLAME,    "Flame Thrower",      3, 200, 300, 1, 12,  700,  500,  500, 17,  1,  2,   4000,   1,   0, NORM)
RR(   1, M_ALL,  9,      NORM,           F_GR,         NORM,       NORM,    0,   0,   0,   0)

QQ(SEEKER,   "Heat Seeker",        8, 1250, 15,15, 25, 1000, 1800, 1800, 51,  9, 50,  20000,  12,   3, HIGH)
RR(   1, M_ALL, 10,    F_NREL,        F_TRL|F_VI,      NORM,       NORM,   0, 0,update_seeker,0)

QQ(PROCKET,  "Pocket Rocket",      5, 1160, 24, 2, 40, 1200, 1900, 1900, 30,  9, 10,  17000,   2,   3, NORM)
RR(   1, M_ALL, 10,      NORM,          F_TRL,         NORM,       NORM,    0,   0,   0,   0)

QQ(UMISSLE,  "Unguided Missle",   10, 1960, 30, 8, 35, 1000, 1800, 1800, 57, 12, 25,  18000,   2,   3, NORM)
RR(   1, M_ALL,  0,      NORM,          F_TRL,         NORM,       NORM,    0,   0,   0,   0)

QQ(TELE,     "TeleGuided",        64, 6000, 1,  1, 15, 1500, 2000, 2000,400, 30,500,  30000, 200,   3, HIGH)
RR(   1,M_ALL,F_CHO,  F_NREL, F_TELE|F_TRL|F_RE|F_TAC, F_KEYB,    NORM,    0,   0,   0,   0)

QQ(TOW,      "TOW Missile",       64, 7500, 2,  1, 15, 1500, 2000, 2000,500, 30,100,  30000, 100,   3, HIGH)
RR(   1,M_ALL,F_CHO,   F_NREL,    F_TRL|F_RE|F_TAC,  F_KEYB,       NORM,    0,   0,   0,   0)

QQ(LTORP,    "Land Torpedo",      20, 5000, 2,  1, 10, 1500, 2000, 2000,500, 30,100,  30000, 100,   3, LOW)
RR(  16,M_SIDES,F_CHO,   F_NREL,           F_RE,       F_KEYB,       NORM,    0,   0,   0,   0)

QQ(BLAST,    "Blast Cannon",       1, 500,  20, 8, 25,  300,  350,  350, 20,  6, 20, 100000,   2,   3, NORM)
RR(   1, M_ALL,  0,      NORM,           F_RE,         NORM,       NORM,   0,  0,  0,hit_blast)

QQ(LASER,    "Pulse Laser",        3,1200, 500, 3, 60,  300,  250,  250, 20,  3,  3,  30000,   1,   0, NORM)
RR(   1, M_ALL,  0,    F_NREL,       F_BEAM|F_NOPT,    NORM,       NORM,    0,   0,   0,   0)

QQ(MINE,     "Mine Layer",         6,  50,  50, 2, 10, 1000, 1000, 1000, 70,  2, 10,   8000,   4,   0, LOW)
RR(  39, M_BACK,  0,      NORM, F_ROT|F_NOHD|F_NOPT,  F_MINE,    F_HOVER,    0,   0,   0,   0)

QQ(SLICK,    "Oil Slick",          0,  50,  50, 5, 10,  300,  500,  500, 70,  0, 10,   2000,   1,   0, LOW)
RR(   1, M_BACK,  0,     F_CR3,    F_NOHD|F_NOPT,     F_MINE,     F_SLICK,   0,   0,   0,   0)

QQ(MORTAR,   "Heavy Mortar",     170, 7500,  10,20, 50, 1400, 2000, 2000, 70, 30,500,  9000,  30,   0, FLY)
RR(   1,M_TURRET,0, F_MAP|F_NREL, F_NOHD|F_TAC|F_TRL, F_DET,       AREA, creat_mort,0,update_mortar,0)

QQ(NUKE,     "Tactical Nuke",    170, 500,  10,20, 25, 1600, 2200, 2200, 20,  6,500,  80000,  30,   0, NORM)
RR(   1,M_ALL, 0,      NORM,          NORM,          F_DET,       AREA,  0,  0,  0,  0)

QQ(HARM,     "Anti-Radiation",    64, 2500, 2, 192,25, 2000, 3400, 3400,100, 24,1000,100000, 192,   0, FLY)
RR(   1,M_LR,F_CHO, F_MAP|F_NREL,F_TRL|F_NOHD|F_TAC,   NORM,       NORM, creat_harm,0,update_harm,0)

/* If it is to show up in the vdesign menus, it must be before Disc Shooter */
QQ(DISC,     "Disc Shooter",       0,  BIG, 1, BIG, 0,   0,     0,    0,BIG,  0,   0,     0,    0,  0, 0)
RR(   1, M_ALL,  0,             0,          0,        0,          0,       0,    0,       0,     0)

#endif /* game balance */

#undef BIG
