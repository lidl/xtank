/* lowlib.h - part of XTank */

#ifdef ANSI_PROTOTYPES
#define ANSI_PROTO(s) s
#else
#define ANSI_PROTO(s) ()
#endif


/* lowlib.c */
extern void get_location ANSI_PROTO((Location *loc));
extern FLOAT max_speed ANSI_PROTO((void));
extern FLOAT speed ANSI_PROTO((void));
extern Angle heading ANSI_PROTO((void));
extern FLOAT acc ANSI_PROTO((void));
extern FLOAT engine_acc ANSI_PROTO((void));
extern FLOAT tread_acc ANSI_PROTO((void));
extern void vehicle_size ANSI_PROTO((int *width, int *height));
extern void turn_vehicle ANSI_PROTO((Angle desired_heading));
extern void turn_vehicle_human ANSI_PROTO((Angle desired_heading));
extern Angle turn_rate ANSI_PROTO((double abs_speed));
extern void set_abs_drive ANSI_PROTO((double abs_speed));
extern void set_rel_drive ANSI_PROTO((double rel_drive));
extern void set_safety ANSI_PROTO((int status));
extern void set_teleport ANSI_PROTO((int status));
extern void turret_position ANSI_PROTO((TurretNum turret, int *xp, int *yp));
extern int num_turrets ANSI_PROTO((void));
extern Angle turret_angle ANSI_PROTO((TurretNum num));
extern Angle turret_turn_rate ANSI_PROTO((TurretNum num));
extern void turn_turret ANSI_PROTO((TurretNum num, Angle angle));
extern Angle aim_turret ANSI_PROTO((TurretNum num, int dx, int dy));
extern Boolean weapon_on ANSI_PROTO((WeaponNum num));
extern int turn_on_weapon ANSI_PROTO((WeaponNum num));
extern int turn_off_weapon ANSI_PROTO((WeaponNum num));
extern int toggle_weapon ANSI_PROTO((WeaponNum num));
extern WeaponStatus fire_weapon ANSI_PROTO((WeaponNum num));
extern int num_weapons ANSI_PROTO((void));
extern int get_weapon ANSI_PROTO((WeaponNum num, Weapon_info *winfo));
extern int weapon_time ANSI_PROTO((WeaponNum num));
extern int weapon_ammo ANSI_PROTO((WeaponNum num));
extern int armor ANSI_PROTO((Side num));
extern int max_armor ANSI_PROTO((Side num));
extern int protection ANSI_PROTO((void));
extern Box (*map_get())[GRID_HEIGHT];
extern WallType wall ANSI_PROTO((WallSide dir, int x, int y));
extern LandmarkType landmark ANSI_PROTO((int x, int y));
extern int get_landmarks ANSI_PROTO((int *num_landmark_infos, Landmark_info landmark_info[]));
extern void get_outpost_loc ANSI_PROTO((int x, int y, int frame_num, int *xret, int *yret));
extern int get_blips ANSI_PROTO((int *num_blip_infos, Blip_info blip_info[]));
extern void get_vehicles ANSI_PROTO((int *num_vehicle_infos, Vehicle_info vehicle_info[]));
extern void get_self ANSI_PROTO((Vehicle_info *v_info));
extern void get_bullets ANSI_PROTO((int *num_bullet_infos, Bullet_info bullet_info[]));
extern Team team ANSI_PROTO((ID vid));
extern int number_vehicles ANSI_PROTO((void));
extern void get_settings ANSI_PROTO((Settings_info *si));
extern int get_money ANSI_PROTO((void));
extern int get_fuel_cost ANSI_PROTO((void));
extern int get_armor_cost ANSI_PROTO((void));
extern int get_ammo_cost ANSI_PROTO((WeaponNum wn));
extern void throw_discs ANSI_PROTO((double dspeed, Boolean delay));
extern void spin_discs ANSI_PROTO((Spin dir));
extern int num_discs ANSI_PROTO((void));
extern void get_discs ANSI_PROTO((int *num_disc_infos, Disc_info disc_info[]));
extern int messages ANSI_PROTO((void));
extern void send_msg ANSI_PROTO((Byte recipient, Opcode opcode, Byte *data));
extern Boolean receive_msg ANSI_PROTO((Message *m));
extern FLOAT fuel ANSI_PROTO((void));
extern FLOAT max_fuel ANSI_PROTO((void));
extern int heat ANSI_PROTO((void));
extern int heat_sinks ANSI_PROTO((void));
extern Boolean has_special ANSI_PROTO((SpecialType st));
extern int frame_number ANSI_PROTO((void));
extern int num_kills ANSI_PROTO((void));
extern int score ANSI_PROTO((void));
extern void done ANSI_PROTO((void));
extern void set_cleanup_func ANSI_PROTO((void (*funcp )(), void *argp));

/* highlib.c */
extern void turn_all_turrets ANSI_PROTO((Angle angle));
extern void aim_all_turrets ANSI_PROTO((int dx, int dy));
extern int fire_all_weapons ANSI_PROTO((void));
extern Boolean clear_path ANSI_PROTO((Location *start, Location *finish));
extern Boolean get_closest_enemy ANSI_PROTO((Vehicle_info *enemy));

#undef ANSI_PROTO

