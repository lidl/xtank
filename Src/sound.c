#ifdef SOUND
#include "limits.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "xtanklib.h"
#include "vehicle.h"
#include "globals.h"
#include "terminal.h"
#include "assert.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include "sound.h"
#include "rplay.h"

extern Terminal	*terminal[];
extern int	num_terminals;
extern char	pathname[];

static RPLAY	*sound_table[MAX_SOUNDS];

/*
 * The sound_actions table used to make saving sound settings
 * much easier.  The order of this table must be consistent
 * with Include/sound.h and the names must be consistent with
 * the names in sload.l.
 */
static char	*sound_actions[] =
{
	"lmg",
	"mg",
	"hmg",
	"lcannon",
	"cannon",
	"hcannon",
	"lrocket",
	"rocket",
	"hrocket",
	"acid",
	"flame",
	"mine",
	"seeker",
	"blast",
	"laser",
	"slick",
	"procket",
	"umissle",
	"nuke",
	"harm",
	"disc",
	"vehicle_hit_wall",
	"vehicle_hit_vehicle",
	"tank_explosion",
	"gleam_explosion",
	"dam0_explosion",
	"dam1_explosion",
	"dam2_explosion",
	"dam3_explosion",
	"dam4_explosion",
	"exhaust_explosion",
	"electric_explosion",
	"nuke_explosion",
	"damage_explosion",
	"start",
	"end",
	"sonar",
	"blip",
	"disc_new_owner",
	"goal",
	"ricochet",
	"weapon_reloading",
	"weapon_no_ammo",
	"weapon_too_hot",
	"weapon_off",
	"ammo_warning",
	"armor_warning",
	"heat_warning",
	"fuel_warning",
	"ammo_ok",
	"armor_ok",
	"heat_ok",
	"fuel_ok",
	"killer",
};

/*
 * Initialize the sound table and load the sounds.
 */
init_sounds()
{
	char	name[MAXPATHLEN];
	int	i;

	draw_text_rc(ANIM_WIN, 0, 1, "Initializing Sounds...", M_FONT, WHITE);
	sync_output(TRUE);

	for (i = 0; i < MAX_SOUNDS; i++)
	{
		sound_table[i] = NULL;
	}

	strcpy(name, pathname);
	strcat(name, "/sounds");

	if (!load_sounds(name))
	{
		fprintf(stderr, "Cannot load sound settings file %s\n", name);
		exit(1);
	}
}

/*
 * Insert the given sound into the sound_table.
 * This can also be used to change sound attributes.
 */
sound_insert(index, name, volume)
int	index;
char	*name;
int	volume;
{
	int	val;

	if (index < 0 || index >= MAX_SOUNDS)
	{
		fprintf(stderr, "Bad sound index value %d\n", index);
		exit(1);
	}

	if (strcmp(name, NO_SOUND) == 0)
	{
		sound_table[index] = NULL;
		return;
	}

	sound_table[index] = rplay_create(RPLAY_PLAY);
	if (sound_table[index] == NULL)
	{
		rplay_perror("rplay_create");
		exit(1);
	}
	val = rplay_set(sound_table[index], RPLAY_INSERT, 0,
			RPLAY_SOUND,	(char *)strdup(name),
			RPLAY_VOLUME,	volume,
			NULL);
	if (val < 0)
	{
		rplay_perror("rplay_set");
		exit(1);
	}
}

/*
 * save the current sound table settings to filename
 */
int	save_sounds(filename)
char	*filename;
{
	FILE	*fp;
	int	i;

	fp = fopen(filename, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot save sound settings to file '%s'\n", filename);
		return 0;
	}

	for (i = 0; i < MAX_SOUNDS; i++)
	{
		if (sound_table[i] == NULL)
		{
			fprintf(fp, "%s: NONE, 0\n", sound_actions[i]);
		}
		else
		{
			fprintf(fp, "%s: %s, %d\n", sound_actions[i],
				(char *)rplay_get(sound_table[i], RPLAY_SOUND, 0),
				rplay_get(sound_table[i], RPLAY_VOLUME, 0));
		}
	}

	fclose(fp);

	return 1;
}

/*
 * initialize sound for the given terminal
 */
init_terminal_sound(t)
Terminal	*t;
{
	char	*p, *rindex();
	Video	*vidptr;
	char	hostname[MAXHOSTNAMELEN];
	int	i;

	/*
	 * get the display hostname 
	 */
	vidptr = (Video *)t->video;
	strcpy(hostname, vidptr->display_name);
	p = rindex(hostname, ':');
	*p = '\0';
	if (*hostname == '\0')
	{
		gethostname(hostname, sizeof(hostname));
	}

	/*
	 * initialize rplayd connection
	 */
	t->rplay_fd = rplay_open(hostname);
	if (t->rplay_fd < 0)
	{
		rplay_perror(hostname);
		return;
	}

	play_terminal(t, SONAR_SOUND);
}

/*
 * play a sound on a terminal
 */
play_terminal(t, index)
Terminal	*t;
SoundType	index;
{
	if (t->rplay_fd > 0 && sound_table[index] > 0)
		rplay(t->rplay_fd, sound_table[index]);
}

/*
 * play a sound on all terminals
 */
play_all(index)
SoundType	index;
{
	int	x;
	
	for(x = 0; x < num_terminals; x++)
		if (terminal[x]->rplay_fd > 0 && sound_table[index])
			rplay(terminal[x]->rplay_fd, sound_table[index]);
}

/*
 * play a sound on the terminal "owning" or watching this tank
 */
play_owner(vehicle, index)
Vehicle		*vehicle;
SoundType	index;
{
	int	x, y;

	for (x = 0; x < vehicle->owner->num_players; x++)
	{
		y = vehicle->owner->player[x];
		if (terminal[y]->rplay_fd > 0 && sound_table[index])
			rplay(terminal[y]->rplay_fd, sound_table[index]);
	}
}

/*
 * play a sound on the terminal that has this location in view
 */
play_in_view(loc, index)
Loc		*loc;
SoundType	index;
{
	int	x, dx, dy;
	int	t_x = loc->x;
	int	t_y = loc->y;

	for (x = 0; x < num_terminals; x++)
	{
		dx = t_x - terminal[x]->loc.x;
		dy = t_y - terminal[x]->loc.y;
		if (dx >= 0 && dx <= ANIM_WIN_WIDTH && dy >= 0 && dy <= ANIM_WIN_HEIGHT &&
			terminal[x]->rplay_fd > 0 && sound_table[index])
			rplay(terminal[x]->rplay_fd, sound_table[index]);
	}
}

/*
 * play a sound on the terminal that has this location in view
 */
play_in_view_x_y(t_x, t_y, index)
int		t_x;
int		t_y;
SoundType	index;
{
	int	x, dx, dy;

	for (x = 0; x < num_terminals; x++)
	{
		dx = t_x - terminal[x]->loc.x;
		dy = t_y - terminal[x]->loc.y;
		if (dx >= 0 && dx <= ANIM_WIN_WIDTH && dy >= 0 && dy <= ANIM_WIN_HEIGHT &&
			terminal[x]->rplay_fd > 0 && sound_table[index])
			rplay(terminal[x]->rplay_fd, sound_table[index]);
	}
}
#endif SOUND
