
/* File: variable.c */

/* Purpose: Angband variables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"


/*
 * Hack -- Link a copyright message into the executable
 */
cptr copyright[6] =
{
	"Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
	"also Copyright (c) 1997 Keldon L. Jones",
	"",
	"This software may be copied and distributed for educational, research,",
	"and not for profit purposes provided that this copyright and statement",
	"are included in all such copies."
};


/*
 * Hack -- Link the "version" into the executable
 */
byte version_major = VERSION_MAJOR;
byte version_minor = VERSION_MINOR;
byte version_patch = VERSION_PATCH;
byte version_extra = VERSION_EXTRA;


/*
 * Hack -- Savefile information
 */
u32b sf_xtra;			/* Operating system info */
u32b sf_when;			/* Time when savefile created */
u16b sf_lives;			/* Number of past "lives" with this file */
u16b sf_saves;			/* Number of "saves" during this life */

/*
 * Hack -- Run-time arguments
 */
bool arg_wizard;		/* Command arg -- Enter wizard mode */
bool arg_fiddle;		/* Command arg -- Enter fiddle mode */
bool arg_force_original;	/* Command arg -- Force original keyset */
bool arg_force_roguelike;	/* Command arg -- Force roguelike keyset */

bool server_generated;		/* The character exists */
bool server_dungeon;		/* The character has a dungeon */
bool server_state_loaded;	/* The server state was loaded from a savefile */
bool server_saved;		/* The character was just saved to a savefile */

bool character_loaded;		/* The character was loaded from a savefile */
bool character_icky;		/* The game is in an icky full screen mode */
bool character_xtra;		/* The game is in an icky startup mode */

u32b seed_flavor;		/* Hack -- consistent object colors */
u32b seed_town;			/* Hack -- consistent town layout */

s16b command_cmd;		/* Current "Angband Command" */

s16b command_arg;		/* Gives argument of current command */
s16b command_rep;		/* Gives repetition of current command */
s16b command_dir;		/* Gives direction of current command */

s16b command_see;		/* See "cmd1.c" */
s16b command_wrk;		/* See "cmd1.c" */

s16b command_gap = 50;		/* See "cmd1.c" */

s16b command_new;		/* Command chaining from inven/equip view */

s16b energy_use;		/* Energy use this turn */

bool create_up_stair;		/* Auto-create "up stairs" */
bool create_down_stair;		/* Auto-create "down stairs" */

bool msg_flag;			/* Used in msg_print() for "buffering" */

bool alive;			/* True if game is running */

bool death;			/* True if player has died */

s16b running;			/* Current counter for running, if any */
s16b resting;			/* Current counter for resting, if any */

/*s16b cur_hgt;	*/		/* Current dungeon height */
/*s16b cur_wid;	*/		/* Current dungeon width */
/*s16b dun_level;	*/		/* Current dungeon level */
s16b num_repro;			/* Current reproducer count */
s16b object_level;		/* Current object creation level */
s16b monster_level;		/* Current monster creation level */

byte trees_in_town;

byte level_up_y[MAX_DEPTH]; 	/* Where do players start if coming up? */
byte level_up_x[MAX_DEPTH];
byte level_down_y[MAX_DEPTH];	/* Where do players start if going down? */
byte level_down_x[MAX_DEPTH];
byte level_rand_y[MAX_DEPTH];	/* Where do players start if they tele level? */
byte level_rand_x[MAX_DEPTH];

s16b players_on_world[MAX_DEPTH + MAX_WILD];
s16b *players_on_depth=&(players_on_world[MAX_WILD]);  /* How many players are at each depth */

s16b special_levels[MAX_SPECIAL_LEVELS]; /* List of depths which are special static levels */

char summon_kin_type;		/* Hack -- See summon_specific() */

huge turn;			/* Current game turn */
huge old_turn;			/* Turn when level began (feelings) */

s32b player_id;			/* Current player ID */

bool wizard;			/* Is the player currently in Wizard mode? */
bool can_be_wizard;		/* Does the player have wizard permissions? */

u16b total_winner;		/* Semi-Hack -- Game has been won */

u16b panic_save;		/* Track some special "conditions" */
u16b noscore;			/* Track various "cheating" conditions */

s16b signal_count = 0;		/* Hack -- Count interupts */

bool inkey_base;		/* See the "inkey()" function */
bool inkey_xtra;		/* See the "inkey()" function */
bool inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */

s16b coin_type;			/* Hack -- force coin type */

bool opening_chest;		/* Hack -- prevent chest generation */

bool use_graphics;		/* Hack -- Assume no graphics mapping */

bool use_sound;			/* Hack -- Assume no special sounds */

bool scan_monsters;		/* Hack -- optimize multi-hued code, etc */
bool scan_objects;		/* Hack -- optimize multi-hued code, etc */

/*s16b total_weight;*/		/* Total weight being carried */

s16b inven_nxt;			/* Hack -- unused */

/*s16b inven_cnt;*/			/* Number of items in inventory */
/*s16b equip_cnt;*/			/* Number of items in equipment */

s32b o_nxt = 1;			/* Object free scanner */
s32b m_nxt = 1;			/* Monster free scanner */

s32b o_max = 1;			/* Object heap size */
s32b m_max = 1;			/* Monster heap size */

s32b o_top = 0;			/* Object top size */
s32b m_top = 0;			/* Monster top size */


/*
 * Server options, set in mangband.cfg
 */

bool cfg_report_to_meta = 0;
char * cfg_meta_address;
char * cfg_report_address = NULL;
char * cfg_bind_name = NULL;
char * cfg_console_password = "change_me";
char * cfg_dungeon_master = "DungeonMaster";
bool cfg_secret_dungeon_master = 0;
s16b cfg_fps = 12;
s16b cfg_tcp_port = 18346;
bool cfg_mage_hp_bonus = 1;
bool cfg_no_steal = 0;
bool cfg_newbies_cannot_drop = 0;
bool cfg_ghost_diving = 0;
bool cfg_door_bump_open = 1;
s32b cfg_level_unstatic_chance = 60;
bool cfg_random_artifacts = 0; /* No randarts by default */
s16b cfg_preserve_artifacts = 3;
s32b cfg_retire_timer = -1;
bool cfg_ironman = 0;
bool cfg_town_wall = 0;
s32b cfg_unique_respawn_time = 300;
s32b cfg_unique_max_respawn_time = 50000;
s16b cfg_max_townies = 100;
s16b cfg_max_trees = 100;
s16b cfg_max_houses = 0;
bool cfg_chardump_color = FALSE;
bool cfg_instance_closed = FALSE;

/*
 * Software options (set via the '=' command).  See "tables.c"
 */


/* Option Set 1 -- User Interface */

bool rogue_like_commands;	/* Rogue-like commands */
bool quick_messages;		/* Activate quick messages */
bool other_query_flag;		/* Prompt for various information */
bool carry_query_flag;		/* Prompt before picking things up */
bool use_old_target;		/* Use old target by default */
bool always_pickup;			/* Pick things up by default */
bool always_repeat;			/* Repeat obvious commands */
bool depth_in_feet;			/* Show dungeon level in feet */

bool stack_force_notes;		/* Merge inscriptions when stacking */
bool stack_force_costs;		/* Merge discounts when stacking */

bool show_labels;			/* Show labels in object listings */
bool show_weights;			/* Show weights in object listings */
bool show_choices;			/* Show choices in certain sub-windows */
bool show_details;			/* Show details in certain sub-windows */

bool ring_bell;				/* Ring the bell (on errors, etc) */
bool use_color;				/* Use color if possible (slow) */


/* Option Set 2 -- Disturbance */

bool find_ignore_stairs;	/* Run past stairs */
bool find_ignore_doors;		/* Run through open doors */
bool find_cut;				/* Run past known corners */
bool find_examine;			/* Run into potential corners */

bool disturb_move;			/* Disturb whenever any monster moves */
bool disturb_near;			/* Disturb whenever viewable monster moves */
bool disturb_panel;			/* Disturb whenever map panel changes */
bool disturb_state;			/* Disturn whenever player state changes */
bool disturb_minor;			/* Disturb whenever boring things happen */
bool disturb_other;			/* Disturb whenever various things happen */

bool alert_hitpoint;		/* Alert user to critical hitpoints */
bool alert_failure;			/* Alert user to various failures */


/* Option Set 3 -- Game-Play */

bool no_ghost;				/* Death is permanent */

bool auto_scum;				/* Auto-scum for good levels */

bool stack_allow_items;		/* Allow weapons and armor to stack */
bool stack_allow_wands;		/* Allow wands/staffs/rods to stack */

bool expand_look;			/* Expand the power of the look command */
bool expand_list;			/* Expand the power of the list commands */

bool view_perma_grids;		/* Map remembers all perma-lit grids */
bool view_torch_grids;		/* Map remembers all torch-lit grids */

bool dungeon_align;			/* Generate dungeons with aligned rooms */
bool dungeon_stair;			/* Generate dungeons with connected stairs */

bool flow_by_sound;			/* Monsters track new player location */
bool flow_by_smell;			/* Monsters track old player location */

bool track_follow;			/* Monsters follow the player */
bool track_target;			/* Monsters target the player */

bool smart_learn;			/* Monsters learn from their mistakes */
bool smart_cheat;			/* Monsters exploit player weaknesses */


/* Option Set 4 -- Efficiency */

bool view_reduce_lite;		/* Reduce lite-radius when running */
bool view_reduce_view;		/* Reduce view-radius in town */

bool avoid_abort;			/* Avoid checking for user abort */
bool avoid_other;			/* Avoid processing special colors */

bool flush_failure;			/* Flush input on any failure */
bool flush_disturb;			/* Flush input on disturbance */
bool flush_command;			/* Flush input before every command */

bool fresh_before;			/* Flush output before normal commands */
bool fresh_after;			/* Flush output after normal commands */
bool fresh_message;			/* Flush output after all messages */

bool compress_savefile;		/* Compress messages in savefiles */

bool hilite_player;			/* Hilite the player with the cursor */

bool view_yellow_lite;		/* Use special colors for torch-lit grids */
bool view_bright_lite;		/* Use special colors for 'viewable' grids */

bool view_granite_lite;		/* Use special colors for wall grids (slow) */
bool view_special_lite;		/* Use special colors for floor grids (slow) */

/* Option Set 99 -- Obsolete but needed for savefile parsing */

bool old_mirror_debug;		/* Show debug messages in mirror window */
bool old_mirror_around;		/* Show overhead map in mirror window */
bool old_mirror_recent;		/* Show monster info in mirror window */
bool old_mirror_normal;		/* Show inven/equip in mirror window */
bool old_mirror_choose;		/* Show item choices in mirror window */
bool old_mirror_spells;		/* Show spell choices in mirror window */
bool old_recall_recent;		/* Show monster info in recall window */
bool old_choice_normal;		/* Show inven/equip in choice window */
bool old_choice_choose;		/* Show item choices in choice window */
bool old_choice_spells;		/* Show spell choices in choice window */


/* Cheating options */

bool cheat_peek;		/* Peek into object creation */
bool cheat_hear;		/* Peek into monster creation */
bool cheat_room;		/* Peek into dungeon creation */
bool cheat_xtra;		/* Peek into something else */
bool cheat_know;		/* Know complete monster info */
bool cheat_live;		/* Allow player to avoid death */


/* Special options */

s16b hitpoint_warn;		/* Hitpoint warning (0 to 9) */

s16b delay_factor;		/* Delay factor (0 to 9) */



s16b feeling;			/* Most recent feeling */
s16b rating;			/* Level's current rating */

bool good_item_flag;		/* True if "Artifact" on this level */

/*bool new_level_flag;*/		/* Start a new level */

bool closing_flag;		/* Dungeon is closing */


/* Dungeon size info */
s16b max_panel_rows, max_panel_cols;
s16b panel_row, panel_col;
/*s16b panel_row_min, panel_row_max;
s16b panel_col_min, panel_col_max;*/
/*s16b panel_col_prt, panel_row_prt;*/

/* Player location in dungeon */
/* This is now put in the player info --KLJ-- */
/*s16b py;
s16b px;*/

/* Targetting variables */
s32b target_who;
s16b target_col;
s16b target_row;

/* Health bar variable -DRS- */
s16b health_who;

/* Monster recall race */
s16b recent_idx;



/* User info */
int player_uid = 0;
int player_euid = 0;
int player_egid = 0;

/* The array of players */
player_type **Players;

/* The party information */
party_type parties[MAX_PARTIES];

/* The information about houses */
house_type houses[MAX_HOUSES];
int num_houses;

/* An array to access a Player's ID */
long GetInd[MAX_ID];

/* Player for doing text_out */
int player_textout = 0;

/* Current player's character name */
/* In the player info --KLJ-- */
/*char player_name[32];*/

/* Stripped version of "player_name" */
/* In the player info --KLJ-- */
/*char player_base[32];*/

/* What killed the player */
/* In the player info --KLJ-- */
/*char died_from[80];*/

/* Hack -- Textual "history" for the Player */
/* In the player info --KLJ-- */
/*char history[4][60];*/

/* Buffer to hold the current savefile name */
char savefile[1024];


/*
 * Array of grids lit by player lite (see "cave.c")
 */
/*s16b lite_n;
byte lite_y[LITE_MAX];
byte lite_x[LITE_MAX];*/

/*
 * Array of grids viewable to the player (see "cave.c")
 *
 * This is now in the player info, so that every player sees the cave
 * differently --KLJ--
 */
/*s16b view_n;
byte view_y[VIEW_MAX];
byte view_x[VIEW_MAX];*/

/*
 * Array of grids for use by various functions (see "cave.c")
 */
s16b temp_n;
byte temp_y[TEMP_MAX];
byte temp_x[TEMP_MAX];


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
cptr *macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
cptr *macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool *macro__cmd;

/*
 * Current macro action [1024]
 */
char *macro__buf;


/*
 * The number of quarks
 */
s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
cptr *quark__str;


/*
 * The next "free" index to use
 */
u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
u16b message__last;

/*
 * The next "free" offset
 */
u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
u16b message__tail;

/*
 * The array of offsets, by index [MESSAGE_MAX]
 */
u16b *message__ptr;

/*
 * The array of chars, by offset [MESSAGE_BUF]
 */
char *message__buf;


/*
 * The array of normal options
 */
u32b option_flag[8];
u32b option_mask[8];


/*
 * The array of window options
 */
u32b window_flag[8];
u32b window_mask[8];


/*
 * The array of window pointers
 */
/*term *ang_term[8];*/


/*
 * The array of indexes of "live" objects
 */
s16b o_fast[MAX_O_IDX];

/*
 * The array of indexes of "live" monsters
 */
s16b m_fast[MAX_M_IDX];


/*
 * The array of "cave grids" [MAX_WID][MAX_HGT].
 * Not completely allocated, that would be inefficient
 * Not completely hardcoded, that would overflow memory
 */
/* For wilderness, I am hacking this to extend in the negative direction.
   I currently have a huge number (4096?) of wilderness levels allocated.
   -APD-
*/ 
cave_type **world[MAX_DEPTH+MAX_WILD]; 
cave_type ***cave = &world[MAX_WILD];
wilderness_type world_info[MAX_WILD+1];
wilderness_type *wild_info=&(world_info[MAX_WILD]);


/*
 * The array of dungeon items [MAX_O_IDX]
 */
object_type *o_list;

/*
 * The array of dungeon monsters [MAX_M_IDX]
 */
monster_type *m_list;

/*
 * Hack -- Quest array
 */
quest q_list[MAX_Q_IDX];


/*
 * The stores [MAX_STORES]
 */
store_type *store;

/*
 * The player's inventory [INVEN_TOTAL]
 *
 * This is now in the player info --KLJ--
 */
/* object_type *inventory; */


/*
 * The size of "alloc_kind_table" (at most MAX_K_IDX * 4)
 */
s16b alloc_kind_size;

/*
 * The entries in the "kind allocator table"
 */
alloc_entry *alloc_kind_table;


/*
 * The size of "alloc_race_table" (at most MAX_R_IDX)
 */
s16b alloc_race_size;

/*
 * The entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;


/*
 * Specify attr/char pairs for inventory items (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];
char tval_to_char[128];

/*
 * Simple keymap method, see "init.c" and "cmd6.c".
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte keymap_cmds[128];
byte keymap_dirs[128];


/*
 * Global table of color definitions
 * Be sure to use "index & 0xFF" to avoid illegal access
 */
byte color_table[256][4];


/*** Player information ***/

/*
 * Static player info record
 *
 * Don't need this anymore --KLJ--
 */
/*static player_type p_body;*/

/*
 * Pointer to the player info
 *
 * Or this --KLJ--
 */
/*player_type *p_ptr = &p_body;*/

/*
 * Pointer to the player tables (race, class, magic)
 *
 * This is in the player info now --KLJ--
 */
/*player_race *rp_ptr;
player_class *cp_ptr;
player_magic *mp_ptr;*/



/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 *
 * This is in the player info now --KLJ--
 */
/*s16b player_hp[PY_MAX_LEVEL];*/


/*
 * Structure (not array) of size limits
 */
maxima *z_info;

/*
 * The vault generation arrays
 */
header *v_head;
vault_type *v_info;
char *v_name;
char *v_text;

/*
 * The terrain feature arrays
 */
header *f_head;
feature_type *f_info;
char *f_name;
char *f_text;
char f_char_s[MAX_F_IDX];
char f_attr_s[MAX_F_IDX];

/*
 * The object kind arrays
 */
header *k_head;
object_kind *k_info;
char *k_name;
char *k_text;

/*
 * The artifact arrays
 */
header *a_head;
artifact_type *a_info;
char *a_name;
char *a_text;

/*
 * The ego-item arrays
 */
header *e_head;
ego_item_type *e_info;
char *e_name;
char *e_text;


/*
 * The monster race arrays
 */
header *r_head;
monster_race *r_info;
char *r_name;
char *r_text;
char r_char_s[MAX_R_IDX];
char r_attr_s[MAX_R_IDX];


/*
 * The player race arrays
 */
player_race *p_info;
char *p_name;
char *p_text;

/*
 * The player class arrays
 */
player_class *c_info;
char *c_name;
char *c_text;

/*
 * The shop owner arrays
 */
owner_type *b_info;
char *b_name;
char *b_text;

/*
 * The racial price adjustment arrays
 */
byte *g_info;
char *g_name;
char *g_text;

/*
 * The player history arrays
 */
hist_type *h_info;
char *h_text;

/*
 * The object flavor arrays
 */
flavor_type *flavor_info;
char *flavor_name;
char *flavor_text;

/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
cptr ANGBAND_DIR;


/*
 * Various data files for the game, such as the high score and
 * the mangband logs.
 */
cptr ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_GAME;
cptr ANGBAND_DIR_EDIT;

/*
 * Various user editable text files (ascii), such as the help and greeting
 * screen. These files may be portable between platforms
 */
cptr ANGBAND_DIR_TEXT;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_SAVE;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_USER;


/*
 * Total Hack -- allow all items to be listed (even empty ones)
 * This is only used by "do_cmd_inven_e()" and is cleared there.
 */
bool item_tester_full;


/*
 * Here is a "pseudo-hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
byte item_tester_tval;


/*
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
bool (*item_tester_hook)(object_type*);



/*
 * Current "comp" function for ang_sort()
 */
bool (*ang_sort_comp)(int Ind, vptr u, vptr v, int a, int b);


/*
 * Current "swap" function for ang_sort()
 */
void (*ang_sort_swap)(int Ind, vptr u, vptr v, int a, int b);



/*
 * Hack -- function hook to restrict "get_mon_num_prep()" function
 */
bool (*get_mon_num_hook)(int r_idx);



/*
 * Hack -- function hook to restrict "get_obj_num_prep()" function
 */
bool (*get_obj_num_hook)(int k_idx);

/* the dungeon master movement hook, is called whenever he moves
 * (to make building large buildings / summoning hoards of mosnters 
 * easier)
 */
//bool (*master_move_hook)(int Ind, char * args) = master_acquire;
bool (*master_move_hook)(int Ind, char * args) = NULL;
