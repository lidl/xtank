/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** file.c
*/

#include "xtank.h"
#include "vehicle.h"

#ifdef UNIX
char pathname[50], vehiclesdir[20], mazesdir[20], programsdir[20];
/* 
**in AMIGA version, ASSIGNs are used to declare directories XDIR: XVDIR: XMDIR:
*/ 
#endif UNIX

char username[MAX_STRING], displayname[120];

int num_vdescs = 0;
Vdesc vdesc[MAX_VDESCS];

int num_mdescs = 0;
Mdesc mdesc[MAX_MDESCS];

int num_sdescs = 0;
Sdesc sdesc[MAX_SDESCS];

/*
** Makes all vehicles listed in [vehiclesdir]/list and all mazes listed in
** [mazesdir]/list.
*/
load_desc_lists()
{
  FILE *file;
  char filename[80];
  char name[80];
  int num;

  /* Make all the vehicle descriptions in the vehicle list */
#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,vehiclesdir);
  (void) strcat(filename,"list");
#endif UNIX
#ifdef AMIGA
  (void) strcpy(filename,"XVDIR:list");
#endif AMIGA
  if((file = fopen(filename,"r")) != NULL) {
    while(fscanf(file,"%s",name) != EOF)
      make_vdesc(name,&num);

    (void) fclose(file);
  }

  /* Make all the maze descriptions in the maze list */
#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,mazesdir);
  (void) strcat(filename,"list");
#endif UNIX
#ifdef AMIGA
  (void) strcpy(filename,"XMDIR:list");
#endif AMIGA
  if((file = fopen(filename,"r")) != NULL) {
    while(fscanf(file,"%s",name) != EOF)
      make_mdesc(name,&num);

    (void) fclose(file);
  }
}

/*
** Loads the vdesc from the appropriate file into the next empty slot.
** Puts the number of the vehicle into num, if loaded.
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT, DESC_NO_ROOM.
*/
make_vdesc(name,num)
     char *name;
     int *num;
{
  Vdesc *d;
  int retval,i;

  if(num_vdescs >= MAX_VDESCS) return DESC_NO_ROOM;

  /* If name present, load into that slot, else load into next empty slot */
  for(i = 0 ; i < num_vdescs ; i++)
    if(!strcmp(name,vdesc[i].name)) break;
  d = &vdesc[i];
  retval = load_vdesc(d,name);

  /* Increment the number if a vdesc was loaded in the last slot */
  if(retval == DESC_LOADED && i == num_vdescs) num_vdescs++;
  *num = i;
  return retval;
}

#define check_range(val,mx) \
  if((val) < 0 || (val) >= (mx)) return DESC_BAD_FORMAT;

/*
** Loads the vehicle description from the file "[vehiclesdir]/[name].v".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
load_vdesc(d,name)
     Vdesc *d;
     char *name;
{
  extern int num_vehicle_objs;
  extern Object *vehicle_obj[];
  FILE *file;
  char filename[80];
  int num_turrets,weapon,mount;
  int i;

  /* Open the vehicle description file */
#ifdef UNIX 
  (void) strcpy(filename,pathname);
  (void) strcat(filename,vehiclesdir);
#endif UNIX
#ifdef AMIGA
  (void) strcpy(filename,"XVDIR:");
#endif AMIGA
  (void) strcat(filename,name);
  (void) strcat(filename,".v");
  if((file = fopen(filename,"r")) == NULL) return DESC_NOT_FOUND;

  /* Load the values into the vdesc structure, checking their validity */
  (void) fscanf(file,"%s",d->name);
  (void) fscanf(file,"%s",d->designer);

  (void) fscanf(file,"%d",&d->body);
  check_range(d->body,num_vehicle_objs);
  num_turrets = vehicle_obj[d->body]->num_turrets;

  (void) fscanf(file,"%d",&d->engine);
  check_range(d->engine,MAX_ENGINES);

  (void) fscanf(file,"%d",&d->num_weapons);
  check_range(d->num_weapons,MAX_WEAPONS+1);

  for(i = 0 ; i < d->num_weapons ; i++) {
    (void) fscanf(file,"%d %d",&weapon,&mount);
    check_range(weapon,MAX_WEAPON_STATS);

    if((mount == MOUNT_TURRET1 && num_turrets < 1) ||
       (mount == MOUNT_TURRET2 && num_turrets < 2) ||
       (mount == MOUNT_TURRET3 && num_turrets < 3) ||
       (mount < 0) ||
       (mount > MAX_MOUNT)) return DESC_BAD_FORMAT;
    
    d->weapon[i] = weapon;
    d->mount[i] = mount;
  }

  (void) fscanf(file,"%d",&d->armor.type);
  check_range(d->armor.type,MAX_ARMOR_TYPES);

  for(i = 0 ; i < MAX_SIDES ; i++)
    (void) fscanf(file,"%d",&d->armor.side[i]);

  (void) fscanf(file,"%d",&d->specials);
  (void) fscanf(file,"%d",&d->heat_sinks);

  (void) fscanf(file,"%d",&d->suspension);
  check_range(d->suspension,MAX_SUSPENSIONS);

  (void) fscanf(file,"%d",&d->treads);
  check_range(d->treads,MAX_TREADS);

  (void) fscanf(file,"%d",&d->bumpers);
  check_range(d->bumpers,MAX_BUMPERS);

  /* Compute parameters, and see if there are any problems */
  if(compute_vdesc(d)) return DESC_BAD_FORMAT;

  (void) fclose(file);

  return DESC_LOADED;
}

/*
** Saves the specified vehicle description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
save_vdesc(d)
     Vdesc *d;
{
  FILE *file;
  char filename[256];
  int i;

  /* Open the vehicle description file */
#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,vehiclesdir);
#endif UNIX
#ifdef AMIGA
  (void) strcpy(filename,"XVDIR:");
#endif AMIGA
  (void) strcat(filename,d->name);
  (void) strcat(filename,".v");
  if((file = fopen(filename,"w")) == NULL) return DESC_NOT_FOUND;

  /* Save all the fields in the vdesc structure */
  (void) fprintf(file,"%s\n",d->name);
  (void) fprintf(file,"%s\n",d->designer);
  (void) fprintf(file,"%d\n",d->body);
  (void) fprintf(file,"%d\n",d->engine);
  (void) fprintf(file,"%d\n",d->num_weapons);
  for(i = 0 ; i < d->num_weapons ; i++)
    (void) fprintf(file,"%d %d\n",d->weapon[i],d->mount[i]);

  (void) fprintf(file,"%d\n",d->armor.type);
  for(i = 0 ; i < MAX_SIDES ; i++)
    (void) fprintf(file,"%d\n",d->armor.side[i]);

  (void) fprintf(file,"%d\n",d->specials);
  (void) fprintf(file,"%d\n",d->heat_sinks);

  (void) fprintf(file,"%d\n",d->suspension);
  (void) fprintf(file,"%d\n",d->treads);
  (void) fprintf(file,"%d\n",d->bumpers);

  (void) fclose(file);
#ifdef UNIX
  /* Change protections so others can read the vehicle in */
  (void) chmod(filename,0644);
#endif
  return DESC_SAVED;
}

/*
** Loads the mdesc from the appropriate file into the next empty slot.
** Puts the number of the maze into num if loaded.
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT, DESC_NO_ROOM.
*/
make_mdesc(name,num)
     char *name;
     int *num;
{
  Mdesc *d;
  int retval,i;

  /* If name present, load into that slot, else load into next empty slot */
  for(i = 0 ; i < num_mdescs ; i++)
    if(!strcmp(name,mdesc[i].name)) break;
  d = &mdesc[i];
  retval = load_mdesc(d,name);

  /* Increment the number if a mdesc was loaded in the last slot */
  if(retval == DESC_LOADED && i == num_mdescs) num_mdescs++;
  *num = i;
  return retval;
}

/*
** Loads the maze description from the file "[mazesdir]/[name].m".
** Returns one of DESC_LOADED, DESC_NOT_FOUND, DESC_BAD_FORMAT.
*/
load_mdesc(d,name)
     Mdesc *d;
     char *name;
{
  FILE *file;
  char filename[80];
  int temp;

#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,mazesdir);
#endif UNIX
#ifdef AMIGA
   (void) strcpy(filename,"XVDIR:");
#endif AMIGA
  (void) strcat(filename,name);
  (void) strcat(filename,".m");
  if((file = fopen(filename,"r")) == NULL) return DESC_NOT_FOUND;

  if((temp = getc(file)) == EOF) return DESC_BAD_FORMAT;
  d->type = (Byte) temp;
  if(alloc_str(file,&d->name) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,&d->designer) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,&d->desc) == DESC_BAD_FORMAT) return DESC_BAD_FORMAT;
  if(alloc_str(file,(char **) &d->data) == DESC_BAD_FORMAT)
     return DESC_BAD_FORMAT;
  
  (void) fclose(file);

  return DESC_LOADED;
}

/* At most 3 bytes per box in the grid for a maze description */
#define MAX_DATA_BYTES    GRID_WIDTH * GRID_HEIGHT * 3 + 1

/*
** Reads in a string from the file.  Mallocs the appropriate amount
** of memory for str, and copies the string into str.
** Returns DESC_BAD_FORMAT if string is too long or EOF is reached.
** Otherwise, returns DESC_LOADED.
*/
alloc_str(file,strp)
     FILE *file;
     char **strp;
{
  extern char *malloc();
  int ret;
  char temp[MAX_DATA_BYTES];
  unsigned int len;

  /* Read chars from file into temp up to and including the first '\0' */
  len = 0;
  do {
    if((ret = getc(file)) == EOF || len >= MAX_DATA_BYTES)
      return DESC_BAD_FORMAT;
    temp[len++] = (char) ret;
  } while((char) ret != '\0');

  *strp = malloc(len);
  (void) strcpy(*strp,temp);

  return DESC_LOADED;
}

/*
** Saves the specified maze description.
** Returns one of DESC_NOT_FOUND, DESC_SAVED.
*/
save_mdesc(d)
     Mdesc *d;
{
  FILE *file;
  char filename[256];

#ifdef UNIX
  (void) strcpy(filename,pathname);
  (void) strcat(filename,mazesdir);
#endif UNIX
#ifdef AMIGA
   (void) strcpy(filename,"XVDIR:");
#endif AMIGA
  (void) strcat(filename,d->name);
  (void) strcat(filename,".m");
  if((file = fopen(filename,"w")) == NULL) return DESC_NOT_FOUND;

  (void) fputc((char) d->type,file);
  fputs(d->name,file);          (void) fputc('\0',file);
  fputs(d->designer,file);      (void) fputc('\0',file);
  fputs(d->desc,file);          (void) fputc('\0',file);
  fputs((char *) d->data,file); (void) fputc('\0',file);
  
  (void) fclose(file);
#ifdef UNIX
  /* Change protections so others can read in the maze */
  (void) chmod(filename,0644);
#endif
  return DESC_SAVED;
}

#ifdef UNIX

/*
** Reads environment variables for the pathname, username,
** vehicles directory, mazes directory, and programs directory.
*/
get_environment()
{
  extern char *getenv();

  /* Read in variables, providing appropriate defaults if not found */
  (void) strcpy(username,getenv("USER"));
  if(strlen(username) == 0)
    (void) strcpy(username,"user");

  (void) strcpy(pathname,getenv("XTANKDIR"));
  if(strlen(pathname) == 0)
    (void) strcpy(pathname,"/mit/games/src/vax/xtank/");

  (void) strcpy(vehiclesdir,getenv("VEHICLESDIR"));
  if(strlen(vehiclesdir) == 0)
    (void) strcpy(vehiclesdir,"Vehicles/");

  (void) strcpy(mazesdir,getenv("MAZESDIR"));
  if(strlen(mazesdir) == 0)
    (void) strcpy(mazesdir,"Mazes/");

  (void) strcpy(programsdir,getenv("PROGRAMSDIR"));
  if(strlen(programsdir) == 0)
    (void) strcpy(programsdir,"Programs/");

  (void) strcpy(displayname,getenv("DISPLAY"));
  if(strlen(displayname) == 0)
    (void) strcpy(displayname,"unix:0");
}
#endif UNIX

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif 

#ifdef AMIGA
#include <dos.h>
#endif

/*
** Reads a file, returning its contents in a null-terminated string.
** Returns an empty string if the file is not found.
*/
char *read_file(filename)
     char *filename;
{
  extern char *malloc();
  FILE *file;
  char *string,temp[256];
  int size;
  
#ifdef AMIGA
  strcpy(temp,"XVDIR:");
#endif
#ifdef UNIX
  if(filename[0] != '/') strcpy(temp,pathname);
  else temp[0] = '\0';
#endif
  strcat(temp,filename);

  file = fopen(temp,"r");
  if(file == (FILE *) NULL)
    return((char *) NULL);

  /* Get file size */
#ifdef AMIGA
  {
   struct FILEINFO finfo;

   if (dfind(&finfo,temp,0))
      return((char *) NULL);
   size = finfo.fib_Size;
  }   
#endif AMIGA

#ifdef UNIX
  {
   struct stat fileinfo;

   if (fstat(fileno(file), &fileinfo))
     return((char *) NULL);
   size = fileinfo.st_size;
  }
#endif UNIX

  /* Leave space for the NULL */
  string = malloc((unsigned int) (size + 1));

  /* Copy the file into memory and close the file */
  (void) fread(string,sizeof(char),size,file);
  (void) fclose(file);

  /* Put NULL at end of string */
  *(string + size) = '\0';

  return(string);
}
