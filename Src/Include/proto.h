#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif


/* 3d.c */
int display_anim_3d P_((unsigned int status));
int init_terminal_3d P_((Terminal *t));
int transform_3d P_((int *dx, int *dy));
int toggle_3d P_((int mask));

/* XMultiSync.c */
int XMultiSync P_((Display *dpys[], int num_dpys, int discard));

/* actions.c */
int adjust_loc P_((Loc *loc, int dx, int dy));
int make_bullet P_((Vehicle *v, Loc *loc, WeaponType type, Angle angle));
int explode P_((Bullet *b, int damage));
int do_special P_((Vehicle *v, SpecialType special_num, unsigned int action));
int move_view P_((int dx, int dy));
int switch_view P_((int num));
int display_pause_message P_((void));
int pause_game P_((Boolean state));
int set_game_speed P_((int spd));
int check_game_speed P_((void));

/* animate.c */
int animate P_((void));

/* box.c */
int box_type_check P_((Vehicle *v, Box *b, FLOAT *xadj, FLOAT *yadj));
int box_landmark P_((Vehicle *v, Box *b));
int box_outpost P_((Vehicle *v, Box *b, int grid_x, int grid_y));
int closest_vehicle P_((Loc *loc, Vehicle *target));
int outpost_loc P_((Box *b, Loc *oloc, int grid_x, int grid_y));
int coll_outpost P_((Box *b, Loc *loc));
Coord *outpost_coordinate P_((Box *b, int fr));
int box_scroll P_((LandmarkType type, FLOAT *xadj, FLOAT *yadj));
int box_slow P_((Vehicle *v));
int init_changed_boxes P_((void));
int change_box P_((Box *b, int x, int y));
int old_box P_((Box *b, int x, int y));

/* collision.c */
int coll_vehicle_walls P_((Vehicle *v));
int coll_bullets_maze P_((void));
int coll_bullets_vehicles P_((void));
int coll_vehicles_vehicles P_((void));

/* console.c */
int special_dummy P_((Vehicle *v, char *record, unsigned int action));
int special_console P_((Vehicle *v, char *record, unsigned int action));
int display_bar P_((int w, int x, int y, int width, int height, int val, int *last_color, int mx, int mx2, Boolean vert, Boolean init, Boolean low_end));

/* cosell.c */
int comment P_((int op, int dat, Vehicle *vh1, Vehicle *vh2));

/* disc.c */
void disc_init_history P_((void));
void disc_new_owner P_((Bullet *b, Vehicle *vh1));
Vehicle *disc_cur_owner P_((Bullet *b));
Vehicle *disc_last_owner P_((Bullet *b));
Vehicle *disc_old_owner P_((Bullet *b));
int get_disc_team P_((Bullet *b));
void set_disc_team P_((Bullet *b, int teamnum));
int set_disc_owner P_((Bullet *b, Vehicle *v));
int release_discs P_((Vehicle *v, double dspeed, Boolean delay));
int set_disc_orbit P_((Vehicle *v, Spin dir));

/* display.c */
int display_terminal P_((unsigned int status, int lastterm));
int display_anim P_((unsigned int status, int lastterm));
int display_vehicle P_((Vehicle *v, unsigned int status));
int display_turrets P_((Vehicle *v, unsigned int status));
int display_bullets P_((unsigned int status, int lastterm));
int display_explosions P_((unsigned int status));
int display_maze P_((unsigned int status));
int display_map P_((unsigned int status));
int display_cons P_((unsigned int status));
int display_help P_((unsigned int status));
int display_pics P_((void));
int draw_objs P_((Object *obj[], Boolean text, int first, int last, int view, int x, int y, int height));
int draw_obj P_((Object *obj, int type, int x, int y, int height));
int draw_picture_string P_((Object *obj, int view, char *str, int x, int y, int adj));

/* escher.c */
int menu_frame P_((int win, int x, int y, int w, int h, int func, int color, int frame));

/* explosion.c */
int make_explosion P_((Loc *loc, unsigned int type));
int explode_location P_((Loc *loc, int num, unsigned int type));

/* file.c */
int load_desc_lists P_((void));
int make_vdesc P_((char *name, int *num));
int load_vdesc P_((Vdesc *d, char *name));
int save_vdesc P_((Vdesc *d));
int make_mdesc P_((char *name, int *num));
int load_mdesc P_((Mdesc *d, char *name));
int alloc_str P_((FILE *file, char **strp));
int save_mdesc P_((Mdesc *d));
int load_sdesc P_((Sdesc *d, char *name));
int make_sdesc P_((char *name, int *num));
int get_environment P_((void));
char *read_file P_((char *filename));
int save_settings P_((char *filename));

/* game.c */
int game_rules P_((Boolean init));
int combat_rules P_((Boolean init));
int war_rules P_((Boolean init));
int war_init_time P_((Byte time[GRID_WIDTH ][GRID_HEIGHT ][MAX_TEAMS ], int x, int y));
int ultimate_rules P_((Boolean init));
int capture_rules P_((Boolean init));
int race_rules P_((Boolean init));
int display_game_stats P_((unsigned int status));

/* ghs.c */
static void trim P_((char *buf));
static char *tgets P_((char *buf, int len, FILE *fp));
static int GetNumber P_((FILE *fp, long *lVar));
static int GetFloat P_((FILE *fp, FLOAT *fVar));
static int GetTorF P_((FILE *fp, char *cVar));
static int GetGameType P_((FILE *fp, int *iVar));
int ReadSetupsFile P_((FILE *infile, struct LKJSDF *SetupStruct));

/* gr.c */
int set_terminal P_((int terminal_num));
int make_terminal P_((char *display_name));
int sync_terminals P_((Boolean discard));
int close_terminal P_((Terminal *t));
int open_windows P_((void));
int map_windows P_((void));
int clear_windows P_((void));
int map_battle_windows P_((void));
int unmap_battle_windows P_((void));

/* graphics.c */

/* highlib.c */
void turn_all_turrets P_((Angle angle));
void aim_all_turrets P_((int dx, int dy));
int fire_all_weapons P_((void));
Boolean clear_path P_((Location *start, Location *finish));
Boolean get_closest_enemy P_((Vehicle_info *enemy));

/* hit.c */
Side find_affected_side P_((Vehicle *v, double angle));
int vehicle_hit_vehicle P_((Vehicle *v1, Vehicle *v2, int width, int height, int shx, int shy));
int vehicle_hit_wall P_((Vehicle *v, int grid_x, int grid_y, WallSide dir));
int bounce_damage P_((double xspeed, double yspeed, double elast));
int bul_hit_vehicle P_((Vehicle *v, Bullet *b, int dx, int dy));
int bul_hit_outpost P_((Bullet *b, Box *bbox, int grid_x, int grid_y));
int bul_hit_wall P_((Bullet *b, int grid_x, int grid_y, WallSide dir));
int bounce_bullet P_((Bullet *b, WallSide dir, double dx, double dy));
int damage_wall P_((int x, int y, WallSide dir, int damage));
int damage_vehicle P_((Vehicle *v, Vehicle *damager, int damage, double angle, int height));
int bounce_vehicles P_((Vehicle *v1, Vehicle *v2, int dx, int dy, double elast));
int bounce_vehicle_wall P_((Vehicle *v, int dx, int dy, double elast));
int assign_speed P_((Vector *vec, double xsp, double ysp));

/* icounter.c */
void increment_time P_((void));
int setup_counter P_((void));
int start_counter P_((void));
int stop_counter P_((void));
void increment_time P_((void));
int setup_counter P_((void));
int start_counter P_((void));
int stop_counter P_((void));
void sigalrm_handler P_((void));
int start_real_counter P_((int time));
int wait_for_real_counter P_((void));
int start_real_counter P_((int time));
int wait_for_real_counter P_((void));
int setup_counter P_((void));
int start_counter P_((void));
int stop_counter P_((void));
int start_real_counter P_((int time));
int wait_for_real_counter P_((void));

/* init.c */
int init_settings P_((void));
int init_turrets P_((Vehicle *v));
int init_bset P_((void));
int init_eset P_((void));

/* input.c */
int get_input P_((void));
int anim_input P_((Event *event));
int get_reply P_((void));
int wait_input P_((void));
int scan_input P_((void));
int input_int P_((int w, char *input_str, int col, int row, int deflt, int mn, int mx, int font));
int input_string P_((int w, char *prompt, char *response, int col, int row, int font, int max_length));
int confirm P_((int w, char *disp, int col, int row, int font));

/* interface.c */
int reset_dynamic_entries P_((void));
int init_interface P_((void));
int init_comb_menus P_((void));
int sub_interface_main P_((int choice));
void sub_interface_view P_((int choice));
void sub_interface_load P_((int choice));
void sub_interface_design P_((int choice));
void sub_interface_help P_((int choice));
void sub_interface_maze P_((int choice, EventType button));
void sub_interface_play P_((int choice));
void sub_interface_settings P_((int choice));
void sub_interface_flags P_((int choice));
int main_interface P_((void));
int erase_other_menus P_((int mu));
static int handle_comb_button P_((Event *evp, int mv));
int do_comb P_((void));
int make_grid_combatant P_((Combatant *c, int row));
int do_num P_((int num, Boolean init));
int display_settings P_((void));
int set_setting P_((int setting, int num));
int setting_num P_((int setting));
int interface_play P_((void));
int do_view P_((int menu, int choice));
int display_mdesc P_((Mdesc *d));
int display_program P_((Prog_desc *p));
int add_players P_((void));
int remove_player P_((int num));
int get_player_info P_((void));
int make_prog_desc P_((void));
int interface_load P_((int type));
int interface_set_desc P_((int type, char *name));
int fix_desc_menu P_((int type));
int ask_desc P_((int type, int row, int col));
int ask_winning_score P_((void));
int ask_maze_density P_((void));
int display_file P_((int w, char *filename));
int display_long_str P_((int w, char *str, int font));
int display_title P_((Boolean gleams));

/* intersect.c */
Boolean intersect_wall P_((Loc *start, Loc *finish));
Boolean seg_intersect_obj P_((Object *obj, Segment *seg2, int xoffs, int yoffs, Coord *ipt));
Boolean obj_overlap P_((Object *obj1, Object *obj2, int xoffs, int yoffs));
Boolean seg_intersect P_((Segment *seg1, Segment *seg2, int xoffs, int yoffs, Coord *ipt));
int make_segment P_((Segment *seg, int x1, int y1, int x2, int y2));
int point_in_vehicle P_((Vehicle *v, double x, double y));

/* lowlib.c */
void get_location P_((Location *loc));
FLOAT max_speed P_((void));
FLOAT speed P_((void));
Angle heading P_((void));
FLOAT acc P_((void));
FLOAT engine_acc P_((void));
FLOAT tread_acc P_((void));
void vehicle_size P_((int *width, int *height));
void turn_vehicle P_((Angle desired_heading));
void turn_vehicle_human P_((Angle desired_heading));
Angle turn_rate P_((double abs_speed));
void set_abs_drive P_((double abs_speed));
void set_rel_drive P_((double rel_drive));
void set_safety P_((int status));
void turret_position P_((TurretNum turret, int *xp, int *yp));
int num_turrets P_((void));
Angle turret_angle P_((TurretNum num));
Angle turret_turn_rate P_((TurretNum num));
void turn_turret P_((TurretNum num, Angle angle));
Angle aim_turret P_((TurretNum num, int dx, int dy));
Boolean weapon_on P_((WeaponNum num));
int turn_on_weapon P_((WeaponNum num));
int turn_off_weapon P_((WeaponNum num));
int toggle_weapon P_((WeaponNum num));
WeaponStatus fire_weapon P_((WeaponNum num));
int num_weapons P_((void));
int get_weapon P_((WeaponNum num, Weapon_info *winfo));
int weapon_time P_((WeaponNum num));
int weapon_ammo P_((WeaponNum num));
int get_tread_type P_((void));
int get_bumper_type P_((void));
int get_vehicle_cost P_((void));
int get_engine_type P_((void));
int get_handling P_((void));
int get_suspension_type P_((void));
int armor P_((Side num));
int max_armor P_((Side num));
int protection P_((void));
WallType wall P_((WallSide dir, int x, int y));
LandmarkType landmark P_((int x, int y));
int get_landmarks P_((int *num_landmark_infos, Landmark_info landmark_info[]));
void get_outpost_loc P_((int x, int y, int frame_num, int *xret, int *yret));
int get_blips P_((int *num_blip_infos, Blip_info blip_info[]));
void get_vehicles P_((int *num_vehicle_infos, Vehicle_info vehicle_info[]));
void get_self P_((Vehicle_info *v_info));
void get_bullets P_((int *num_bullet_infos, Bullet_info bullet_info[]));
Team team P_((ID vid));
int number_vehicles P_((void));
void get_settings P_((Settings_info *si));
int get_money P_((void));
int get_fuel_cost P_((void));
int get_armor_cost P_((void));
int get_ammo_cost P_((WeaponNum wn));
void throw_discs P_((double dspeed, Boolean delay));
void spin_discs P_((Spin dir));
int num_discs P_((void));
void get_discs P_((int *num_disc_infos, Disc_info disc_info[]));
int messages P_((void));
void send_msg P_((Byte recipient, Opcode opcode, Byte *data));
Boolean receive_msg P_((Message *m));
FLOAT fuel P_((void));
FLOAT max_fuel P_((void));
int heat P_((void));
int heat_sinks P_((void));
Boolean has_special P_((SpecialType st));
int frame_number P_((void));
int num_kills P_((void));
int score P_((void));
void done P_((void));
void set_cleanup_func P_((void (*funcp )(), void *argp));

/* main.c */
void debugger_break P_((void));
int main P_((int argc, char *argv[]));

/* malloc.c */
char *my_malloc P_((unsigned size));

/* mapper.c */
int special_mapper P_((Vehicle *v, char *record, unsigned int action));
int draw_full_map P_((Box map[GRID_WIDTH ][GRID_HEIGHT ]));
int draw_symbol P_((Landmark_info *s));
int full_mapper P_((unsigned int status));

/* maze.c */
int setup_maze P_((void));
int make_maze P_((Mdesc *d));
int build_mdesc P_((Mdesc *d, Game type, char *name, char *designer, char *desc));
int clear_mdesc P_((Mdesc *d));
int process_maze P_((void));
int make_random_maze_walls P_((void));
int remove_walls P_((int percent));
int make_dest_walls P_((int percent));
int set_box_types P_((int num_prob, FLOAT prob[]));

/* mazeconv.c */
int convert_maze P_((Mdesc *d, int convtype));

/* mdesign.c */
static int handle_key P_((Event *event));
int design_maze P_((void));
int handle_button P_((Event *event));
int mdesign_show_anim P_((void));
int display_mdesc_maze P_((void));
int display_mdesc_info P_((Mdesc *d));
int mdesign_show_help P_((void));
int mdesign_clear_input P_((void));
int change_maze P_((PixC *loc, int action));
int unmake_maze P_((void));
int mdesign_save P_((void));
int mdesign_load P_((void));
int make_landmark P_((BoxC *loc, LandmarkType type));
int show_landmark P_((BoxC *loc));
int set_team P_((BoxC *loc, int teamnum));
int show_box_fast P_((BoxC *loc));
int show_box P_((BoxC *loc));
int show_surrounding_walls P_((BoxC *loc));
int move_area P_((void));
int copy_area P_((void));
int erase_area P_((void));
int place_area_request P_((BoxC *loc));
int read_area P_((BoxC vertices[2 ], Box temp_maze[GRID_WIDTH ][GRID_HEIGHT ]));
int kill_area P_((BoxC vertices[2 ]));
int put_area P_((BoxC old_vertices[2 ], Box temp_maze[GRID_WIDTH ][GRID_HEIGHT ], BoxC *new_start));
int select_area P_((BoxC bvertices[2 ]));
int xor_rectangle P_((PixC *start, PixC *end));
int figure_insideness P_((void));
int get_inside_spot P_((BoxC *bloc));
int add_to_maze P_((int x, int y, BoxC *boxes, int *size));
int check_pixel P_((PixC *loc));
int check_pixel_extra P_((PixC *loc));
int check_box P_((BoxC *loc));
int box_to_pix P_((BoxC *bloc, PixC *ploc));
int pix_to_box P_((PixC *ploc, BoxC *bloc));
int check_wall P_((BoxC *loc, Wall wl));
int make_wall P_((BoxC *loc, Wall wl));
int make_destructible P_((BoxC *loc, Wall wl));
int unmake_wall P_((BoxC *loc, Wall wl));
int show_wall P_((BoxC *loc, Wall wl));
int unshow_wall P_((BoxC *loc, Wall wl));
int draw_wall P_((BoxC *loc, Wall wl, int color));
int show_dot P_((BoxC *loc));

/* menu.c */
int menu_sys_window P_((Menu_int *menuobj, int wdw));
int menu_bare_make P_((Menu_int *menuobj, int menuid, char *title, int size, int width, int xtop, int ytop, int fntcode, int has_bar, int use_escher));
int menu_set_fields P_((Menu_int *menuobj, int menuid, int val));
int menu_resize P_((Menu_int *menuobj, int menuid, int newsize));
int menu_new_width P_((Menu_int *menuobj, int menuid, int newwid));
int menu_display_internal P_((Menu_int *menuobj, int menuid));
int menu_display_frame P_((Menu_int *menuobj, int menuid));
int menu_display P_((Menu_int *menuobj, int menuid));
int menu_erase P_((Menu_int *menuobj, int menuid));
int menu_erase_internal P_((Menu_int *menuobj, int menuid));
int menu_redraw P_((Menu_int *menuobj, int menuid));
int in_sbar P_((Menu_int *menuobj, int menuid, int x, int y));
int menu_resolve_coord P_((Menu_int *menuobj, int menuid, int y));
int menu_adjust P_((Menu_int *menuobj, int menuid, Event *ev));
int menu_hit_in_border P_((Menu_int *menuobj, int menuid, int x, int y));
int menu_hit_p P_((Menu_int *menuobj, Event *ev, int *p_menuid, int *selec, int *just_scrolled));
int menu_track_mouse P_((Menu_int *menuobj, int menuid, int y));
int menu_highlight P_((int w, Menu *m, int item));
int menu_system_expose P_((Menu_int *menuobj));
int menu_unhighlight P_((Menu_int *menuobj, int menuid));
int menu_sys_display P_((Menu_int *menuobj));
int menu_sys_erase P_((Menu_int *menuobj));
int menu_hit P_((Menu_int *menuobj, int x, int y));

/* message.c */
int init_msg_sys P_((void));
int init_msg_terminal P_((int num));
int init_msg_game P_((void));
int display_game P_((unsigned int status));
int message_input P_((Event *event));
int map_input P_((Event *event));
int set_message_data P_((Vehicle *v, Event *event));
int display_sending P_((void));
int init_messages P_((Vehicle *v));
int send_message P_((Vehicle *v));
int send_death_message P_((Vehicle *victim, Vehicle *killer));
int dispatch_message P_((Message *m));
int receive_message P_((Vehicle *v, Message *m));
int display_msg P_((unsigned int status));
int format_message P_((Message *m, char *disp));
void compose_message P_((Byte sender, Byte sendee, Opcode opcode, Byte *data));

/* objects.c */
int make_objects P_((void));
int free_objects P_((void));
int free_object P_((Object *obj));
int rotate_objects P_((void));
int rotate_object P_((Object *obj, Bits **bitmap));

/* program.c */
int set_current_vehicle P_((Vehicle *v));
int init_prog_descs P_((void));
int init_threader P_((void));
int init_specials P_((Vehicle *v));
int init_programs P_((Vehicle *v));
int run_all_programs P_((void));
int run_program P_((Program *prog));
int check_time P_((void));
int stop_program P_((void));
int make_programs P_((Vehicle *v, int num_progs, int prog_num[]));

#ifndef NO_CAMO
/* camo.c */
int special_camo P_((Vehicle *v, char *record, unsigned int action));
int special_stealth P_((Vehicle *v, char *record, unsigned int action));
int special_rdf P_((Vehicle *v, char *record, unsigned int action));
#endif /* !NO_CAMO */

#ifndef NO_HUD
/* hud.c */
int special_hud P_((Vehicle *v, char *record, unsigned int action));
#endif /* !NO_HUD */

/* radar.c */
int special_radar P_((Vehicle *v, char *record, unsigned int action));
int full_radar P_((unsigned int status));
int draw_char P_((Loc *loc, int c, int color));

/* repair.c */
int special_repair P_((Vehicle *v, char *record, unsigned int action));

/* scroll.c */
int draw_scrollbar P_((scrollbar *sbar));
int drag_scrollbar P_((scrollbar *sbar, int mx, int my, unsigned int button));

/* setup.c */
int standard_combatants P_((void));
int choose_program P_((void));
int robot_combatants P_((void));
int player_combatants P_((void));
int customized_combatants P_((void));
int init_terms P_((void));
int init_combatants P_((void));
int play_game P_((void));
int setup_game P_((Boolean newgame));
int setup_combatant P_((Combatant *c));
int setup_terminal P_((int num, Vehicle *v));
int place_vehicle P_((Vehicle *v));
int game_cleanup P_((void));
int all_terms P_((Vehicle *veh, void (*func )()));

/* status.c */
int init_status P_((void));
int init_vehicle_status P_((Vehicle *v));
int display_status P_((unsigned int disptype));
int display_status_win P_((int num, unsigned int disptype));
int draw_status_from_scratch P_((Vehicle *v));
int update P_((int section, int vnum, int *old, int new, unsigned int fromscratch, int color));
int draw_dead_symbol P_((int num));
int compute_minarmor P_((Vehicle *v));
int compute_totammo P_((Vehicle *v));

/* thread.c */
#if 0
Thread *thread_setup P_((void));
Thread *thread_init P_((char *buf, unsigned int bufsize, Thread *(*func )()));
Thread *thread_switch P_((Thread *newthd));
Thread *thread_kill P_((Thread *thd));
Thread *thread_setup P_((void));
Thread *thread_init P_((char *buf, unsigned int bufsize, Thread *(*func )()));
Thread *thread_switch P_((Thread *newthd));
Thread *thread_kill P_((Thread *thd));
Thread *thread_setup P_((void));
Thread *thread_init P_((char *buf, unsigned int bufsize, int (*func )()));
Thread *thread_switch P_((Thread *newthd));
Thread *thread_kill P_((Thread *thd));
#endif

/* unix.c */
int check_internet P_((int num_clients, char **client));
int check_internet P_((int num_clients, char **client));
int compile_module P_((char *module_name, char **symbol, char **code, char *error_name, char *output_name));
struct nlist *namelist P_((int fd, struct exec *hdr));
int compile_module P_((char *module_name, Prog_desc **symbol, char **code, char *error_name, char *output_name));
#if 0
SYMENT *namelist P_((int fd, FILHDR *filhdr, AOUTHDR *hdr));
#endif
int compile_module P_((char *module_name, char **symbol, char **code, char *error_name, char *output_name));

/* update.c */
int update_loc P_((Loc *old_loc, Loc *loc, int dx, int dy));
int update_vehicle P_((Vehicle *v));
int update_vector P_((Vehicle *v));
int update_rotation P_((Vehicle *v));
int update_turret P_((Turret *t));
int update_bullets P_((void));
int update_mine P_((Bullet *b));
int update_seeker P_((Bullet *b));
int update_disc P_((Bullet *b));
int update_explosions P_((void));
int update_maze_flags P_((void));
int update_specials P_((void));
int update_screen_locs P_((void));

/* util.c */
int init_random P_((void));
int rnd P_((int mx));
FLOAT rnd_interval P_((double mn, double mx));
int display_mesg P_((int w, char *string, int row, int font));
int display_mesg2 P_((int w, char *string, int column, int row, int font));
int display_mesg1 P_((int w, char *string, int column, int row, int font, int color));
int free_everything P_((void));

/* vdesign.c */
int design_vehicle P_((void));
int init_vdesign_interface P_((void));
int vdesign_specials_hil P_((Vdesc *d));
void vdesign_load P_((Vdesc *d));
void vdesign_save P_((Vdesc *d));
int vdesign_interface P_((Vdesc *d));
int erase_vdesign_menus P_((int mu));
int init_vdesc P_((Vdesc *d));
int compute_vdesc P_((Vdesc *d));
int display_vdesc P_((Vdesc *d, unsigned int status));
int init_vdesign P_((void));

/* vehicle.c */
void make_turrets P_((Vehicle *v));
void make_specials P_((Vehicle *v, Flag which));
void unmake_specials P_((Vehicle *v));
#if 0
Vehicle *make_vehicle P_((Vdesc *d, Combatant *c));
#endif
int unmake_vehicle P_((Vehicle *v));
int activate_vehicle P_((Vehicle *v));
void inactivate_vehicle P_((Vehicle *victim));
void explode_vehicle P_((Vehicle *victim));
void make_observer P_((Terminal *trm));
int kill_vehicle P_((Vehicle *victim, Vehicle *killer));

/* x11.c */
int open_graphics P_((void));
int close_graphics P_((void));
int reset_video P_((void));
int set_video P_((Video *video));
#if 0
Video *make_video P_((char *name));
#endif
int make_parent P_((void));
int close_video P_((Video *video));
int make_window P_((int w, int x, int y, int width, int height, int border));
int beep_window P_((void));
int map_window P_((int w));
int unmap_window P_((int w));
int clear_window P_((int w));
int draw_text_left P_((int window, int x, int y, char *str, int font, int func, int color));
int draw_text P_((int w, int x, int y, char *str, int font, int func, int color));
int should_disp_name P_((void));
int draw_text_rc P_((int w, int x, int y, char *str, int font, int color));
int clear_text_rc P_((int w, int x, int y, int width, int height, int font));
int flush_output P_((void));
int sync_output P_((Boolean discard));
int multi_sync P_((Video *video[], int num_videos, Boolean discard));
int get_events P_((int *num_events, Event event[]));
int follow_mouse P_((int w, Boolean status));
int button_up P_((int w, Boolean status));
int make_gcs P_((void));
int set_font_path P_((char *fontdir));
int make_cursors P_((void));
int make_cursor P_((int c, int width, int height, int xhot, int yhot, char bits[], char mask[]));
int set_cursor P_((int c));
int font_height P_((int font));
int font_string_width P_((char *str, int font));
int make_picture P_((Picture *pic, char *bitmap));
int free_picture P_((Picture *pic));
Byte *rotate_pic_90 P_((Picture *pic, Picture *rot_pic, Byte *bitmap));
Byte *rotate_pic_180 P_((Picture *pic, Picture *rot_pic, Byte *bitmap));
int liteXerror P_((Display *dpy, XErrorEvent *err));
char *get_default P_((char *itemname, char *itemclass, char *defaultstr));
int get_num_default P_((char *itemname, char *itemclass, int defaultnum));

#undef P_
