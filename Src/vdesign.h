/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** vdesign.h
*/

#define BIG (1<<31 - 1)

Weapon_stat weapon_stat[VMAX_WEAPONS] = {
    /* type             dam  rng ammo  tm spd  wgt  spc  fr  bn ht  a$    $  */
    {"Light Machine Gun", 2, 360, 300,  2, 17,  20, 200, 22,  0, 0,  1, 1000},
    {"Machine Gun",       3, 360, 250,  3, 17,  50, 225, 22,  1, 1,  2, 2200},
    {"Heavy Machine Gun", 4, 360, 200,  3, 17, 100, 250, 22,  2, 2,  3, 3000},

    {"Light Pulse Rifle", 2, 480, 250, 4, 20, 50, 200, 25, 3, 2, 1, 1500},
    {"Pulse Rifle", 3, 480, 225, 4, 20, 125, 225, 25, 4, 3, 2, 3300},
    {"Heavy Pulse Rifle", 4, 480, 200, 4, 20, 300, 250, 25, 5, 4, 3, 4500},

    {"Light Autocannon", 3, 400, 250, 3, 22, 200, 300, 21, 6, 3, 4, 3000},
    {"Autocannon", 4, 400, 225, 3, 22, 300, 350, 21, 7, 4, 5, 6000},
    {"Heavy Autocannon", 5, 400, 200, 3, 22, 500, 400, 21, 8, 5, 6, 10000},

    {"Light Rkt Launcher", 4, 600, 150, 7, 25, 600, 800, 25, 9, 4, 8, 7000},
    {"Rkt Launcher", 6, 600, 125, 7, 25, 900, 1200, 25, 10, 6, 10, 10000},
    {"Heavy Rkt Launcher", 8, 600, 100, 7, 25, 900, 1600, 25, 11, 8, 12, 15000},

    {"Acid Sprayer", 6, 160, 100, 3, 10, 300, 500, 17, 12, 0, 10, 3000},
    {"Flame Thrower", 3, 200, 300, 1, 12, 700, 500, 17, 13, 1, 2, 4000},

    {"Mine Layer", 6, 50, 50, 2, 10, 1000, 1000, 50, 14, 2, 10, 8000},
    {"Heat Seeker", 8, 1250, 15, 15, 25, 1000, 1800, 50, 15, 9, 50, 20000},
    {"Oil Slick", 0, 50, 50, 5, 10, 300, 500, 50, 16, 0, 10, 2000},

    {"Disc Shooter", 0, BIG, 1, BIG, 0, 0, 0, BIG, 17, 0, 0, 0}
};

Armor_stat armor_stat[MAX_ARMORS] = {
    {"Steel", 0, 8, 3, 10},
    {"Kevlar", 0, 3, 3, 20},
    {"Hardened Steel", 1, 8, 3, 20},
    {"Composite", 1, 4, 3, 30},
    {"Compound Steel", 2, 8, 3, 40},
    {"Titanium", 2, 5, 3, 70},
    {"Tungsten", 3, 20, 3, 100}
};

Engine_stat engine_stat[MAX_ENGINES] = {
    {"Small Electric", 50, 100, 20, 5, 200, 1500},
    {"Medium Electric", 100, 150, 30, 5, 300, 2200},
    {"Large Electric", 200, 200, 40, 5, 400, 3000},
    {"Super Electric", 300, 250, 50, 5, 500, 6000},
    {"Small Combustion", 300, 400, 200, 8, 200, 2000},
    {"Medium Combustion", 400, 500, 300, 8, 300, 2500},
    {"Large Combustion", 500, 600, 400, 8, 400, 3000},
    {"Super Combustion", 600, 1000, 600, 8, 500, 4000},
    {"Small Turbine", 600, 1000, 800, 10, 350, 4000},
    {"Medium Turbine", 700, 1200, 1000, 10, 450, 5000},
    {"Large Turbine", 800, 1500, 1500, 10, 550, 7000},
    {"Turbojet Turbine", 1000, 2000, 2000, 10, 750, 10000},
    {"Fuel Cell", 1200, 1000, 400, 20, 600, 15000},
    {"Fission", 1500, 3000, 3500, 15, 1000, 20000},
    {"Breeder Fission", 1800, 3500, 4000, 15, 1250, 25000},
    {"Fusion", 2250, 4000, 2500, 5, 1500, 40000}
};

Body_stat body_stat[MAX_BODIES] = {
    /* type       sz  wgt   wlim   space    drag ha tur cost */
    {"Lightcycle", 2, 200,   800,   600,     .10, 8, 0, 3000},
    {"Hexo",       3, 1500,  5000,  4000,    .25, 6, 1, 4000},
    {"Spider",     3, 2500,  8000,  3000,    .40, 7, 1, 5000},
    {"Psycho",     4, 5000,  18000, 8000,    .60, 4, 1, 5000},
    {"Tornado",    4, 7000,  22000, 12000,   .80, 4, 1, 7000},
    {"Marauder",   5, 9000,  28000, 18000,  1.00, 3, 2, 10000},
    {"Tiger",      6, 11000, 35000, 22000,  1.50, 5, 1, 12000},
    {"Rhino",      7, 12000, 40000, 30000,  2.00, 3, 2, 10000},
    {"Medusa",     7, 14000, 40000, 25000,  1.20, 4, 3, 15000},
    {"Malice",     5, 4000,  20000, 15000,   .40, 7, 1, 17000},
    {"Trike",      2, 400,   1600,  1200,    .15, 6, 0, 4000}
};

Suspension_stat suspension_stat[MAX_SUSPENSIONS] = {
    {"Light", -1, 100},
    {"Normal", 0, 200},
    {"Heavy", 1, 400},
    {"Active", 2, 1000}
};

Tread_stat tread_stat[MAX_TREADS] = {
    /* type   friction  cost */
    {"Smooth", .70, 100},
    {"Normal", .80, 200},
    {"Chained", .90, 400},
    {"Spiked", 1.00, 1000},
    {"Hover", .20, 5000}
};

Bumper_stat bumper_stat[MAX_BUMPERS] = {
    {"None", 0.0, 0},
    {"Normal", 0.07, 200},
    {"Rubber", 0.15, 400},
    {"Retro", 0.25, 1000}
};

Special_stat special_stat[MAX_SPECIALS] = {
    /* type         cost */
    {"Console", 250},
    {"Mapper", 500},
    {"Radar", 1000},
    {"Repair",     50000},
    {"Ram Plate",   2000},
    {"Deep Radar",  8000},
    {"Stealth",    20000},
    {"Navigation",   200}
};

Heat_sink_stat heat_sink_stat = {500, 1000, 500};
