// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef	_COMMON_CONSOLE_H_
#define	_COMMON_CONSOLE_H_

#include "../common/thread.h"
#include "../common/mutex.h"
#include "../common/spinlock.h"
#include "../common/sql.h"
#include "../config/core.h"

/**
 * Queue Max
 * why is there a limit, why not make it dynamic? - I'm playing it safe, I'd rather not play with memory management between threads
 **/
#define CONSOLE_PARSE_SIZE 10

typedef void (*CParseFunc)(char *line);
#define CPCMD(x) void console_parse_ ##x (char *line)
#define CPCMD_A(x) console_parse_ ##x

#define CP_CMD_LENGTH 20
struct CParseEntry {
	char cmd[CP_CMD_LENGTH];
	union {
		CParseFunc func;
		struct CParseEntry **next;
	} u;
	unsigned short next_count;
};

struct {
	char queue[CONSOLE_PARSE_SIZE][MAX_CONSOLE_INPUT];
	unsigned short count;
} cinput;

struct console_interface {
	void (*init) (void);
	void (*final) (void);
	void (*display_title) (void);
#ifdef CONSOLE_INPUT
	/* vars */
	SPIN_LOCK ptlock;/* parse thread lock */
	rAthread pthread;/* parse thread */
	volatile int32 ptstate;/* parse thread state */
	ramutex ptmutex;/* parse thread mutex */
	racond ptcond;/* parse thread cond */
	/* */
	struct CParseEntry **cmd_list;
	struct CParseEntry **cmds;
	unsigned int cmd_count;
	unsigned int cmd_list_count;
	/* */
	Sql *SQL;
	/* */
	void (*parse_init) (void);
	void (*parse_final) (void);
	int (*parse_timer) (int tid, int64 tick, int id, intptr_t data);
	void *(*pthread_main) (void *x);
	void (*parse) (char* line);
	void (*parse_sub) (char* line);
	int (*key_pressed) (void);
	void (*load_defaults) (void);
	void (*parse_list_subs) (struct CParseEntry *cmd, unsigned char depth);
	void (*addCommand) (char *name, CParseFunc func);
	void (*setSQL) (Sql *SQL_handle);
#endif
};

struct console_interface *console;

void console_defaults(void);

#endif /* _COMMON_CONSOLE_H_ */
