/* lowlib.h - part of XTank */

extern Angle aim_turret(), heading(), turn_rate(), turret_angle(),
    turret_turn_rate();
extern Boolean has_special(), weapon_on(), receive_msg();
extern Box (*map_get())[GRID_HEIGHT];
extern LandmarkType landmark();
extern Team team();
extern WallType wall();
extern WeaponStatus fire_weapon();
extern float fuel(), max_fuel(), max_speed(), speed(), acc(), engine_acc(),
    tread_acc();
extern int armor(), frame_number(), get_ammo_cost(), get_armor_cost(),
    get_blips(), get_fuel_cost(), get_landmarks(), get_money(), get_weapon(),
    heat(), heat_sinks(), max_armor(), messages(), num_discs(), num_kills(),
    num_turrets(), num_weapons(), number_vehicles(), protection(), score(),
    turn_off_weapon(), turn_on_weapon(), weapon_ammo(), weapon_time();
extern void done(), get_bullets(), get_discs(), get_location(),
    get_outpost_loc(), get_self(), get_settings(), get_vehicles(),
    send_msg(), set_abs_drive(), set_rel_drive(), set_safety(),
    spin_discs(), throw_discs(), turn_turret(), turn_vehicle(),
    turn_vehicle_human(), turret_position(), vehicle_size();

/* from highlib.c */
extern void aim_all_turrets(), turn_all_turrets();
extern int fire_all_weapons();
extern Boolean clear_path(), get_closest_enemy();
