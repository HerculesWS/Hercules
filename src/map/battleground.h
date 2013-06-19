// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _BATTLEGROUND_H_
#define _BATTLEGROUND_H_

#include "../common/mmo.h" // struct party
#include "guild.h"

/**
 * Defines
 **/
#define MAX_BG_MEMBERS 30
#define BG_DELAY_VAR_LENGTH 30

/**
 * Enumerations
 **/
enum bg_queue_types {
	BGQT_INVALID,
	BGQT_INDIVIDUAL,
	BGQT_PARTY,
	BGQT_GUILD
};

struct battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
};

struct battleground_data {
	unsigned int bg_id;
	unsigned char count;
	struct battleground_member_data members[MAX_BG_MEMBERS];
	// BG Cementery
	unsigned short mapindex, x, y;
	// Logout Event
	char logout_event[EVENT_NAME_LENGTH];
	char die_event[EVENT_NAME_LENGTH];
};

void do_init_battleground(void);
void do_final_battleground(void);

struct battleground_data* bg_team_search(int bg_id);
int bg_send_dot_remove(struct map_session_data *sd);
int bg_team_get_id(struct block_list *bl);
struct map_session_data* bg_getavailablesd(struct battleground_data *bg);

int bg_create(unsigned short mapindex, short rx, short ry, const char *ev, const char *dev);
int bg_team_join(int bg_id, struct map_session_data *sd);
int bg_team_delete(int bg_id);
int bg_team_leave(struct map_session_data *sd, int flag);
int bg_team_warp(int bg_id, unsigned short mapindex, short x, short y);
int bg_member_respawn(struct map_session_data *sd);
int bg_send_message(struct map_session_data *sd, const char *mes, int len);

struct bg_arena {
	char name[NAME_LENGTH];
	unsigned char id;
	char npc_event[EVENT_NAME_LENGTH];
	short min_level, max_level;
	short prize_win, prize_loss, prize_draw;
	short min_players;
	short max_players;
	short min_team_players;
	char cooldown_variable[NAME_LENGTH];
	char delay_var[NAME_LENGTH];
	unsigned short maxDuration;
	int queue_id;
	int begin_timer;
	int fillup_timer;
	int game_timer;
	unsigned short fillup_duration;
	unsigned short pregame_duration;
};

/* battleground.c interface (incomplete) */
struct battleground_interface {
	bool queue_on;
	/* */
	int mafksec;
	char gdelay_var[BG_DELAY_VAR_LENGTH];
	/* */
	struct bg_arena **arena;
	unsigned char arenas;
	/* */
	struct bg_arena *(*name2arena) (char *name);
	void (*queue_add) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	enum BATTLEGROUNDS_QUEUE_ACK (*can_queue) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	int (*id2pos) (int queue_id, int account_id);
	void (*queue_pc_cleanup) (struct map_session_data *sd);
	void (*begin) (struct bg_arena *arena);
	int (*begin_timer) (int tid, unsigned int tick, int id, intptr_t data);
	void (*queue_pregame) (struct bg_arena *arena);
	int (*fillup_timer) (int tid, unsigned int tick, int id, intptr_t data);
	/* */
	void (*config_read) (void);
} bg_s;

struct battleground_interface *bg;

void battleground_defaults(void);

#endif /* _BATTLEGROUND_H_ */
