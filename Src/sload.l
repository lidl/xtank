%{

#include <stdio.h>
#include <string.h>
#include "sound.h"

#define SEP	" ,\t"

char		*strtoke(), *sval();

%}

B		:[ \t]+
s		{B}.+
b		{B}[01]
i		{B}[0-9]+
f		{B}[0-9]+("."[0-9]*)?

%%
^lmg{s}/\n			{ do_sound(LMG_SOUND); }
^mg{s}/\n			{ do_sound(MG_SOUND); }
^hmg{s}/\n			{ do_sound(HMG_SOUND); }
^lcannon{s}/\n			{ do_sound(LCANNON_SOUND); }
^cannon{s}/\n			{ do_sound(CANNON_SOUND); }
^hcannon{s}/\n			{ do_sound(HCANNON_SOUND); }
^lrocket{s}/\n			{ do_sound(LROCKET_SOUND); }
^rocket{s}/\n			{ do_sound(ROCKET_SOUND); }
^hrocket{s}/\n			{ do_sound(HROCKET_SOUND); }
^acid{s}/\n			{ do_sound(ACID_SOUND); }
^flame{s}/\n			{ do_sound(FLAME_SOUND); }
^mine{s}/\n			{ do_sound(MINE_SOUND); }
^seeker{s}/\n			{ do_sound(SEEKER_SOUND); }
^blast{s}/\n			{ do_sound(BLAST_SOUND); }
^laser{s}/\n			{ do_sound(LASER_SOUND); }
^slick{s}/\n			{ do_sound(SLICK_SOUND); }
^procket{s}/\n			{ do_sound(PROCKET_SOUND); }
^umissle{s}/\n			{ do_sound(UMISSLE_SOUND); }
^nuke{s}/\n			{ do_sound(NUKE_SOUND); }
^harm{s}/\n			{ do_sound(HARM_SOUND); }
^disc{s}/\n			{ do_sound(DISC_SOUND); }
^vehicle_hit_wall{s}/\n		{ do_sound(VEHICLE_HIT_WALL_SOUND); }
^vehicle_hit_vehicle{s}/\n	{ do_sound(VEHICLE_HIT_VEHICLE_SOUND); }
^tank_explosion{s}/\n		{ do_sound(TANK_EXPLOSION_SOUND); }
^gleam_explosion{s}/\n		{ do_sound(GLEAM_EXPLOSION_SOUND); }
^dam0_explosion{s}/\n		{ do_sound(DAM0_EXPLOSION_SOUND); }
^dam1_explosion{s}/\n		{ do_sound(DAM1_EXPLOSION_SOUND); }
^dam2_explosion{s}/\n		{ do_sound(DAM2_EXPLOSION_SOUND); }
^dam3_explosion{s}/\n		{ do_sound(DAM3_EXPLOSION_SOUND); }
^dam4_explosion{s}/\n		{ do_sound(DAM4_EXPLOSION_SOUND); }
^exhaust_explosion{s}/\n	{ do_sound(EXHAUST_EXPLOSION_SOUND); }
^electric_explosion{s}/\n	{ do_sound(ELECTRIC_EXPLOSION_SOUND); }
^nuke_explosion{s}/\n		{ do_sound(NUKE_EXPLOSION_SOUND); }
^damage_explosion{s}/\n		{ do_sound(DAMAGE_EXPLOSION_SOUND); }
^start{s}/\n			{ do_sound(START_SOUND); }
^end{s}/\n			{ do_sound(END_SOUND); }
^sonar{s}/\n			{ do_sound(SONAR_SOUND); }
^blip{s}/\n			{ do_sound(BLIP_SOUND); }
^disc_new_owner{s}/\n		{ do_sound(DISC_NEW_OWNER_SOUND); }
^goal{s}/\n			{ do_sound(GOAL_SOUND); }
^ricochet{s}/\n			{ do_sound(RICOCHET_SOUND); }
^weapon_reloading{s}/\n		{ do_sound(WEAPON_RELOADING_SOUND); }
^weapon_no_ammo{s}/\n		{ do_sound(WEAPON_NO_AMMO_SOUND); }
^weapon_too_hot{s}/\n		{ do_sound(WEAPON_TOO_HOT_SOUND); }
^weapon_off{s}/\n		{ do_sound(WEAPON_OFF_SOUND); }
^ammo_warning{s}/\n		{ do_sound(AMMO_WARNING_SOUND); }
^armor_warning{s}/\n		{ do_sound(ARMOR_WARNING_SOUND); }
^heat_warning{s}/\n		{ do_sound(HEAT_WARNING_SOUND); }
^fuel_warning{s}/\n		{ do_sound(FUEL_WARNING_SOUND); }
^ammo_ok{s}/\n			{ do_sound(AMMO_OK_SOUND); }
^armor_ok{s}/\n			{ do_sound(ARMOR_OK_SOUND); }
^heat_ok{s}/\n			{ do_sound(HEAT_OK_SOUND); }
^fuel_ok{s}/\n			{ do_sound(FUEL_OK_SOUND); }
^killer{s}/\n			{ do_sound(KILLER_SOUND); }
\n				{ /* nothing */ }
.*				{
					fprintf(stderr, "Bad sound file entry '%s'\n", yytext);
					exit(2);
				}

%%

int	load_sounds(filename)
char	*filename;
{
	FILE	*fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot read sound settings file '%s'\n", filename);
		return 0;
	}

	yyin = fp;
	yylex();
	fclose(fp);

	return 1;
}

do_sound(index)
int	index;
{
	char	*p, *name;
	int	volume;

	p = sval();
	name = strtoke(p, SEP);
	volume = atoi(strtoke(NULL, SEP));

	sound_insert(index, name, volume);
}

char	*sval()
{
        char *cp;

	for(cp = yytext; *cp != ':'; cp++) ;
	cp++;
	while(*cp == ' ' || *cp == '\t') cp++;

	return cp;
}
