/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** $Id$
*/

#if 1 || defined(__STDC__) || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif

/* three-d.c */
int display_anim_3d P_((int));
int init_terminal_3d P_((Terminal *));
int transform_3d P_((int *, int *));
int toggle_3d P_((int));

/* XMultiSync.c */
#ifdef Display
int XMultiSync P_((Display *dpys[], int num_dpys, int discard));
#endif

/* actions.c */
void adjust_loc P_((Loc *loc, int dx, int dy));
Bullet *make_bullet P_((Vehicle *v, Loc *loc, WeaponType type, Angle angle, lCoord *target));

#ifdef _XTANKLIB_H_
#ifdef FLOAT
void adjust_speed P_((FLOAT *speedx, FLOAT *speedy, double adjust));
#endif
#ifdef Loc
#endif
#ifdef Vehicle
#ifdef Bullet
Boolean get_tele P_((Vehicle *v, Bullet **b));
#endif
int turn_tow P_((Vehicle *v, float direction));
int get_tow P_((Vehicle *v))l
#ifdef Bullet
#endif
#endif
#ifdef Bullet
#endif
#ifdef Vehicle
#endif
void do_special P_((Vehicle *, SpecialType, int));
void move_view P_((int dx, int dy));
int next_live_tank P_((void));
int previous_live_tank P_((void));
int IsVehicleAlive P_((int num));
void switch_view P_((int num));
void display_pause_message P_((void));
void pause_game P_((Boolean state));
void set_game_speed P_((int spd));
void check_game_speed P_((void));
#endif
void explode P_((Bullet *b, int damage));
void expl_area P_((Bullet *b));

/* animate.c */
int animate P_((void));

/* box.c */
void box_landmark P_((Vehicle *v, Box *b));
void box_slow P_((Vehicle *v));
void box_outpost P_((Vehicle *v, Box *b, int grid_x, int grid_y));
int closest_vehicle P_((Loc *loc, Vehicle *target));
void box_type_check P_((Vehicle *v, Box *b, FLOAT *xadj, FLOAT *yadj));
int coll_outpost P_((Box *b, Loc *loc));
Coord *outpost_coordinate P_((Box *b, int fr));
void box_scroll P_((LandmarkType type, FLOAT *xadj, FLOAT *yadj));
void init_changed_boxes P_((void));
int old_box P_((Box *b, int x, int y));
void outpost_loc P_((Box *b, Loc *oloc, int grid_x, int grid_y));
int change_box P_((Box *b, int x, int y));

/* collision.c */
#ifdef Vehicle
#endif
void coll_bullets_maze P_((void));
void coll_bullets_vehicles P_((void));
void coll_vehicles_vehicles P_((void));
void coll_vehicle_walls P_((Vehicle *v));

/* cosell.c */
int comment P_((int op, int dat, Vehicle *vh1, Vehicle *vh2, Bullet *db));

#ifdef _XTANKLIB_H_
/* disc.c */
void disc_init_history P_((void));
#ifdef Bullet
#ifdef Vehicle
void disc_new_owner P_((Bullet *b, Vehicle *vh1));
#endif
Vehicle *disc_cur_owner P_((Bullet *b));
Vehicle *disc_last_owner P_((Bullet *b));
Vehicle *disc_old_owner P_((Bullet *b));
#endif
#ifdef Vehicle
#ifdef Spin
int set_disc_orbit P_((Vehicle *v, Spin dir));
#endif
#endif
#endif
void set_disc_owner P_((Bullet *b, Vehicle *v));
int get_disc_team P_((Bullet *b));
void release_discs P_((Vehicle *v, double dspeed, Boolean delay));
void set_disc_team P_((Bullet *b, int teamnum));

/* display.c */
void display_terminal P_((unsigned int status, int lastterm));
void display_anim P_((unsigned int status, int lastterm));
void display_vehicle P_((Vehicle *v, unsigned int status));
#ifdef Vehicle
#endif
void display_turrets P_((Vehicle *v, unsigned int status));
void display_bullets P_((unsigned int status, int lastterm));
void display_explosions P_((unsigned int status));
void display_maze P_((unsigned int status));
void display_map P_((unsigned int status));
void display_cons P_((unsigned int status));
void display_help P_((unsigned int status));
void display_pics P_((void));
void init_box_names P_((void));
#ifdef Object
#endif
void draw_objs P_((Object **, Boolean, int, int, int, int, int, int));
void draw_obj P_((Object *, int, int, int, int));
void draw_picture_string P_((Object *, int, char *, int, int, int));

/* escher.c */
void menu_frame P_((int, int, int, int, int, int, int, int));

/* explosion.c */
void make_explosion P_((Loc *, int, int));
void explode_location P_((Loc *, int, int));
#ifdef Loc
#endif

#if defined(FILE) && defined(Vdesc) && defined(Mdesc) && defined(Sdesc)
/* file.c */
int save_mdesc P_((Mdesc *d));
int load_sdesc P_((Sdesc *d, char *name));
int make_sdesc P_((char *name, int *num));
int get_environment P_((void));
char *read_file P_((char *filename));
int save_settings P_((char *filename));
#endif
int ReadVehicleFormat0 P_((FILE *, Vdesc *));
int ReadVehicleFormat1 P_((FILE *, Vdesc *));
int alloc_str P_((FILE *, char **));
void convert_maze P_((Mdesc *, int));
void load_desc_lists P_((void));
int load_mdesc P_((Mdesc *, char *));
int load_vdesc P_((Vdesc *, char *));
int make_mdesc P_((char *, int *));
int make_vdesc P_((char *, int *));
int save_vdesc P_((Vdesc *));
int SaveVehicleFormat1 P_((Vdesc *));

/* game.c */
int ScreenOut P_((char *, int, int));
int ScreenOutColor P_((char *, int, int, int));
void StandardOut P_((char *, int, int));
void StandardOutColor P_((char *, int, int, int));
int capture_rules P_((Boolean));
int combat_rules P_((Boolean));
int display_game_stats P_((int));
int display_game_stats_to_current P_((int, int));
int game_rules P_((Boolean));
int race_rules P_((Boolean));
int stq_rules P_((Boolean));
int ultimate_rules P_((Boolean));
void war_init_time P_((Byte time[GRID_WIDTH][GRID_HEIGHT][MAX_TEAMS], int, int));
int war_rules P_((Boolean));

/* gr.c */
void clear_windows P_((void));
void close_terminal P_((Terminal *));
int make_terminal P_((char *));
void map_battle_windows P_((void));
void map_windows P_((void));
int open_windows P_((void));
void set_terminal P_((int));
void sync_terminals P_((Boolean));
void unmap_battle_windows P_((void));

/* highlib.c */
/* highlib.c prototypes are in lowlib.h */

/* hit.c */
void vehicle_hit_vehicle P_((Vehicle *v1, Vehicle *v2, int width, int height, int shx, int shy));
void vehicle_hit_wall P_((Vehicle *v, int grid_x, int grid_y, WallSide dir));
int bounce_damage P_((double xspeed, double yspeed, double elast));
void invalidate_maps P_((void));
void bul_hit_vehicle P_((Vehicle *v, Bullet *b, int dx, int dy));
void bul_hit_outpost P_((Bullet *b, Box *bbox, int grid_x, int grid_y));
int bul_hit_wall P_((Bullet *b, int grid_x, int grid_y, WallSide dir));
void bounce_bullet P_((Bullet *b, WallSide dir, double dx, double dy));
int damage_vehicle P_((Vehicle *v, Vehicle *damager, int damage, double angle, int height));
#if defined(Vehicle) && defined(WallSide) && defined(Box)
#endif
void assign_speed P_((Vector *vec, double xsp, double ysp));
void bounce_vehicles P_((Vehicle *v1, Vehicle *v2, int dx, int dy, double elast));
void bounce_vehicle_wall P_((Vehicle *v, int dx, int dy, double elast));
int damage_wall P_((int x, int y, WallSide dir, int damage));
Side find_affected_side P_((Vehicle *v, double angle));

/* icounter.c */
#if defined(MOTOROLA) && defined(m68k)
int increment_time P_((void));
#else
void increment_time P_((void));
#endif
int setup_counter P_((void));
int start_counter P_((void));
int stop_counter P_((void));
void sigalrm_handler P_((void));
int start_real_counter P_((int time));
int wait_for_real_counter P_((void));

/* init.c */
int init_settings P_((void));
#ifdef Vehicle
int init_turrets P_((Vehicle *v));
#endif
int init_bset P_((void));
int init_eset P_((void));

/* input.c */
int get_input P_((void));
#ifdef Event
int anim_input P_((Event *event));
#endif
int get_reply P_((void));
int wait_input P_((void));
int scan_input P_((void));
int input_int P_((int w, char *input_str, int col, int row, int deflt, int mn, int mx, int font));
int input_string P_((int w, char *prompt, char *response, int col, int row, int font, int max_length));
int confirm P_((int w, char *disp, int col, int row, int font));

/* interface.c */
int reset_dynamic_entries P_((void));
int init_flags_hil P_((void));
void MakeForceString P_((char *pcTemp, int iNum));
int init_interface P_((void));
int init_comb_menus P_((void));
int sub_interface_main P_((int choice));
int sub_interface_force P_((int choice));
void sub_interface_view P_((int choice));
void sub_interface_load P_((int choice));
void sub_interface_design P_((int choice));
void sub_interface_help P_((int choice));
#ifdef EventType
void sub_interface_maze P_((int choice, EventType button));
#endif
void sub_interface_play P_((int choice));
void sub_interface_machine P_((int choice));
void sub_interface_settings P_((int choice));
void sub_interface_flags P_((int choice));
int main_interface P_((void));
int erase_other_menus P_((int mu));
#ifdef Event
static int handle_comb_button P_((Event *evp, int mv));
#endif
int do_comb P_((void));
#ifdef Combatant
int make_grid_combatant P_((Combatant *c, int row));
int combatant_to_grid P_((Combatant *c, int row));
#endif
#ifdef Boolean
int do_num P_((int num, Boolean init));
#endif
int display_settings P_((void));
int set_setting P_((int setting, int num));
int setting_num P_((int setting));
int interface_play P_((void));
int do_view P_((int menu, int choice));
#ifdef Mdesc
int display_mdesc P_((Mdesc *d));
#endif
#ifdef Prog_desc
int display_program P_((Prog_desc *p));
#endif
int add_players P_((void));
int add_given_player P_((int choice));
int remove_player P_((int num));
int get_player_info P_((void));
int input_filename P_((int iWindow, char *pcPrevFileName, char *pcFileName, int iLineNum, int iFont, int iMaxLen));
int make_prog_desc P_((void));
int load_prog_desc P_((char *filename, int batch));
int interface_load P_((int type));
int interface_set_desc P_((int type, char *name));
int fix_desc_menu P_((int type));
int ask_desc P_((int type, int row, int col));
int ask_winning_score P_((void));
int ask_maze_density P_((void));
int display_file P_((int w, char *filename));
int display_long_str P_((int w, char *str, int font));
#ifdef Boolean
int display_title P_((Boolean gleams));
#endif
int fix_combantants P_((int nt));
int comb_load_v P_((void));
int comb_load_all P_((void));

/* intersect.c */
#if defined(Loc) && defined(Boolean) && defined(Segment) && defined(Vehicle) && defined(Object)
Boolean intersect_wall P_((Loc *start, Loc *finish));
Boolean seg_intersect_obj P_((Object *obj, Segment *seg2, int xoffs, int yoffs, Coord *ipt));
Boolean obj_overlap P_((Object *obj1, Object *obj2, int xoffs, int yoffs));
Boolean seg_intersect P_((Segment *seg1, Segment *seg2, int xoffs, int yoffs, Coord *ipt));
int make_segment P_((Segment *seg, int x1, int y1, int x2, int y2));
int point_in_vehicle P_((Vehicle *v, double x, double y));
#endif

/* lowlib.c */
/* lowlib.c prototypes are in lowlib.h */

/* main.c */
void debugger_break P_((void));
int main P_((int argc, char *argv[]));
#ifdef CLFkr
int InitConfigStruct P_((struct CLFkr *ConfigRunning));
#endif

/* malloc.c */
char *my_malloc P_((unsigned size));

/* mapper.c */
#ifdef SpecialStatus
SpecialStatus special_mapper P_((Vehicle *v, char *record, unsigned int action));
#endif
#ifdef Box
int draw_full_map P_((Box map[GRID_WIDTH][GRID_HEIGHT]));
int draw_symbol P_((Landmark_info *s));
#endif
int full_mapper P_((unsigned int status));

/* maze.c */
int setup_maze P_((void));
#ifdef Mdesc
int make_maze P_((Mdesc *d));
int build_mdesc P_((Mdesc *d, Game type, char *name, char *designer, char *desc));
int clear_mdesc P_((Mdesc *d));
#endif
int process_maze P_((void));
int make_random_maze_walls P_((void));
int remove_walls P_((int percent));
int make_dest_walls P_((int percent));
#ifdef FLOAT
int set_box_types P_((int num_prob, FLOAT prob[]));
#endif


/* mdesign.c */
#ifdef Event
static int handle_key P_((Event *event));
int design_maze P_((void));
int handle_button P_((Event *event));
#endif
int mdesign_show_anim P_((void));
int display_mdesc_maze P_((void));
#ifdef Mdesc
int display_mdesc_info P_((Mdesc *d));
#endif
int mdesign_show_help P_((void));
int mdesign_clear_input P_((void));
#ifdef BoxC
int change_maze P_((PixC *loc, int action));
int unmake_maze P_((void));
int mdesign_save P_((void));
int mdesign_load P_((void));
int make_landmark P_((BoxC *loc, LandmarkType type));
int show_landmark P_((BoxC *loc));
int set_team P_((BoxC *loc, int teamnum));
int set_teleport_code P_((BoxC *loc));
int show_box_fast P_((BoxC *loc));
int show_box P_((BoxC *loc));
int show_surrounding_walls P_((BoxC *loc));
int move_area P_((void));
int copy_area P_((void));
int erase_area P_((void));
int place_area_request P_((BoxC *loc));
int read_area P_((BoxC vertices[2], Box temp_maze[GRID_WIDTH][GRID_HEIGHT]));
int kill_area P_((BoxC vertices[2]));
int put_area P_((BoxC old_vertices[2], Box temp_maze[GRID_WIDTH][GRID_HEIGHT], BoxC *new_start));
int select_area P_((BoxC bvertices[2]));
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
#endif

/* menu.c */
#ifdef Menu_int
int menu_sys_window P_((Menu_int *menuobj, int wdw));
int menu_bare_make P_((Menu_int *menuobj, int menuid, char *title, int size, int width, int xtop, int ytop, int fntcode, int has_bar, int use_escher, int center));
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
#endif

/* message.c */
int init_msg_sys P_((void));
int init_msg_terminal P_((int num));
int init_msg_game P_((void));
int display_game P_((unsigned int status));
#ifdef Event
int message_input P_((Event *event));
int map_input P_((Event *event));
#endif
#ifdef Vehicle
#ifdef Event
int set_message_data P_((Vehicle *v, Event *event));
#endif
int display_sending P_((void));
int init_messages P_((Vehicle *v));
int send_message P_((Vehicle *v));
int send_death_message P_((Vehicle *victim, Vehicle *killer));
#endif
int dispatch_message P_((Message *));
#ifdef Message
#ifdef Vehicle
int receive_message P_((Vehicle *v, Message *m));
#endif
int format_message P_((Message *m, char *disp));
#endif
#ifdef Opcode
void compose_message P_((Byte sender, Byte sendee, Opcode opcode, Byte * data));
#endif
int display_msg P_((unsigned int status));

/* newconsole.c */
int idx2armor P_((int idx, int *sidep));
#ifdef Vehicle
int con_init P_((Vehicle *v, char *record));
int special_dummy P_((Vehicle *v, char *record, unsigned int action));
SpecialStatus special_console P_((Vehicle *v, char *record, unsigned int action));
#endif

/* newfile.c */
#ifdef Vdesc
int new_load_vdesc P_((Vdesc *d, char *name));
#endif
char *abbrev_of P_((char *str));
void init_Wnames P_((void));
#ifdef Vdesc
int SaveVehicleFormat1 P_((Vdesc *d));
#endif

/* players.c */
int init_players P_((void));

/* objects.c */
int make_objects P_((void));
#ifdef Object
Object *make_object P_((Object *obj, Bits **bitmap));
int free_objects P_((void));
int free_object P_((Object *obj));
int rotate_objects P_((void));
int rotate_object P_((Object *obj, Bits **bitmap));
#endif

#if defined(Vehicle) && defined(Program)
/* program.c */
int set_current_vehicle P_((Vehicle *v));
int init_prog_descs P_((void));
int init_specials P_((Vehicle *v));
int zap_specials P_((Vehicle *v));
int init_programs P_((Vehicle *v));
int run_program P_((Program *prog));
int stop_program P_((void));
int make_programs P_((Vehicle *v, int num_progs, int prog_num[]));
int find_pdesc P_((char *prog_name, int *index_return));
#endif
int check_time P_((void));
int run_all_programs P_((void));

#ifndef NO_CAMO
#ifdef Vehicle
/* camo.c */
SpecialStatus special_stealth P_((Vehicle *v, char *record, unsigned int action));
SpecialStatus special_camo P_((Vehicle *v, char *record, unsigned int action));
SpecialStatus special_rdf P_((Vehicle *v, char *record, unsigned int action));
#endif
#endif /* !NO_CAMO */

#ifndef NO_HUD
/* hud.c */
#ifdef Angle
voide draw_armor P_((int armor, Angle ang));
#endif
#ifdef Vehicle
SpecialStatus special_hud P_((Vehicle *v, char *record, unsigned int action));
#endif

#endif /* !NO_HUD */

/* radar.c */
#if defined(SpecialStatus) && defined(Vehicle)
SpecialStatus special_radar P_((Vehicle *v, char *record, unsigned int action));
#endif
int full_radar P_((unsigned int status));
#ifdef Loc
int draw_char P_((Loc *loc, int c, int color));
#endif
#ifdef Vehicle
#ifdef Coord
int nr_draw_number P_((Vehicle *v, Coord *c));
#endif
SpecialStatus special_new_radar P_((Vehicle *v, char *record, unsigned int action));
int special_taclink P_((Vehicle *v, char *record, unsigned int action));
#endif
#ifdef newRadar
int nr_t_redisplay P_((newRadar *r));
#endif

/* repair.c */
#if defined(SpecialStatus) && defined(Vehicle)
SpecialStatus special_repair P_((Vehicle *v, char *record, unsigned int action));
#endif

/* scroll.c */
#ifdef scrollbar
int draw_scrollbar P_((scrollbar *sbar));
int drag_scrollbar P_((scrollbar *sbar, int mx, int my, unsigned int button));
#endif

/* setup.c */
int standard_combatants P_((void));
int choose_program P_((void));
int robot_combatants P_((void));
int player_combatants P_((void));
int customized_combatants P_((void));
int init_terms P_((void));
int init_combatants P_((void));
int play_game P_((void));
int setup_game P_((Boolean));
#ifdef Combatant
int setup_combatant P_((Combatant *c));
#endif
#ifdef Vehicle
int setup_terminal P_((int num, Vehicle *v));
int place_vehicle P_((Vehicle *v));
int game_cleanup P_((void));
int all_terms P_((Vehicle *veh, void (*func) ()));
#endif

/* status.c */
int init_status P_((void));
#ifdef Vehicle
int init_vehicle_status P_((Vehicle *v));
#endif
int display_status P_((unsigned int disptype));
int display_status_win P_((int num, unsigned int disptype));
#ifdef Vehicle
int draw_status_from_scratch P_((Vehicle *v));
#endif
int update P_((int section, int vnum, int *old, int new, unsigned int fromscratch, int color));
int draw_dead_symbol P_((int num));
#ifdef Vehicle
int compute_minarmor P_((Vehicle *v));
int compute_totammo P_((Vehicle *v));
#endif

/* thread.c */
#ifdef Thread
Thread *thread_setup P_((void));
Thread *thread_init P_((char *buf, unsigned int bufsize, Thread *(*func) ()));
#endif

/* threadglue.c */
int init_threader P_((void));

/* unix.c */
int check_internet P_((int num_clients, char **client));
#ifndef hp9000s800
int compile_module P_((char *module_name, char **symbol, char **code, char *error_name, char *output_name));
#ifndef mips
/*struct nlist *namelist P_((int fd, struct exec * hdr)); */
#endif
#else /* hp9000s800 */
int compile_module P_((char *module_name, Prog_desc **symbol, char **code, char *error_name, char *output_name));
#endif /* hp9000s800 */

/* update.c */
#ifdef Vehicle
int update_vector P_((Vehicle *v));
#endif
#ifdef Turret
int update_turret P_((Turret *t));
#endif
int update_bullets P_((void));
#ifdef Bullet
#endif
int update_disc P_((Bullet *b));
int update_vehicle P_((Vehicle *v));
int update_explosions P_((void));
int update_maze_flags P_((void));
int update_rotation P_((Vehicle *v));
int update_specials P_((void));
int update_screen_locs P_((void));

/* util.c */
int init_random P_((void));
int rnd P_((int mx));
#ifdef FLOAT
FLOAT rnd_interval P_((double mn, double mx));
#endif
int display_mesg P_((int w, char *string, int row, int font));
int display_mesg2 P_((int w, char *string, int column, int row, int font));
int display_mesg1 P_((int w, char *string, int column, int row, int font, int color));
int free_everything P_((void));
long idist P_((long x1, long y1, long x2, long y2));

/* vdesign.c */
int design_vehicle P_((void));
int init_vdesign_interface P_((void));
#ifdef Vdesc
int vdesign_specials_hil P_((Vdesc *d));
void vdesign_load P_((Vdesc *d));
void vdesign_save P_((Vdesc *d));
int vdesign_interface P_((Vdesc *d));
int erase_vdesign_menus P_((int mu));
int init_vdesc P_((Vdesc *d));
int display_vdesc P_((Vdesc *d, unsigned int status));
#endif
int compute_vdesc P_((Vdesc *));
int init_vdesign P_((void));

/* vehicle.c */
#ifdef Vehicle
void make_turrets P_((Vehicle *v));
void make_specials P_((Vehicle *v, Flag which));
void unmake_specials P_((Vehicle *v));
Vehicle *make_vehicle P_((Vdesc *d, Combatant *c));
void inactivate_vehicle P_((Vehicle *victim));
void explode_vehicle P_((Vehicle *victim));
#endif
#ifdef Terminal
void make_observer P_((Terminal *trm));
#endif
int kill_vehicle P_((Vehicle *victim, Vehicle *killer));
int unmake_vehicle P_((Vehicle *v));
int activate_vehicle P_((Vehicle *v));

/* x11.c */
int open_graphics P_((void));
int close_graphics P_((void));
int reset_video P_((void));
int set_video P_((Video *video));
#ifdef Video
Video *make_video P_((char *name));
#endif
void close_video P_((Video *));
int make_parent P_((void));
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
#ifdef Boolean
#endif
int sync_output P_((Boolean discard));
#if defined(Video) && defined(Boolean)
int multi_sync P_((Video *video[], int num_videos, Boolean discard));
#endif
#ifdef Event
#endif
int get_events P_((int *, Event *));
#ifdef Boolean
int follow_mouse P_((int w, Boolean status));
int button_up P_((int w, Boolean status));
#endif
int make_gcs P_((void));
int set_font_path P_((char *fontdir));
int make_cursors P_((void));
int make_cursor P_((int c, int width, int height, int xhot, int yhot, char bits[], char mask[]));
int set_cursor P_((int c));
int font_height P_((int font));
int font_string_width P_((char *str, int font));
#ifdef Picture
int make_picture P_((Picture *pic, char *bitmap));
int free_picture P_((Picture *pic));
#ifdef Byte
Byte *rotate_pic_90 P_((Picture *pic, Picture *rot_pic, Byte * bitmap));
Byte *rotate_pic_180 P_((Picture *pic, Picture *rot_pic, Byte * bitmap));
#endif
#endif
#if defined(Display) && defined(XErrorEvent)
int liteXerror P_((Display *dpy, XErrorEvent *err));
#endif
char *get_default P_((char *itemname, char *itemclass, char *defaultstr));
int get_num_default P_((char *itemname, char *itemclass, int defaultnum));

#undef P_
