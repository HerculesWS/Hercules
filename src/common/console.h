// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H

#include "common/hercules.h"
#include "common/db.h"
#include "common/mutex.h"
#include "common/spinlock.h"
#include "common/sql.h"
#include "common/thread.h"

/**
 * Queue Max
 * why is there a limit, why not make it dynamic? - I'm playing it safe, I'd rather not play with memory management between threads
 **/
#define CONSOLE_PARSE_SIZE 10

/**
 * Default parsing function abstract prototype
 **/
typedef void (*CParseFunc)(char *line);

/**
 * Console parsing function prototypes
 * CPCMD: Console Parsing CoMmand
 * x - command
 * y - category
 **/
#define CPCMD(x) void console_parse_ ##x (char *line)
#define CPCMD_A(x) console_parse_ ##x
#define CPCMD_C(x,y) void console_parse_ ##y ##x (char *line)
#define CPCMD_C_A(x,y) console_parse_ ##y ##x

#define CP_CMD_LENGTH 20

enum CONSOLE_PARSE_ENTRY_TYPE {
	CPET_UNKNOWN,
	CPET_FUNCTION,
	CPET_CATEGORY,
};

struct CParseEntry {
	char cmd[CP_CMD_LENGTH];
	int type; ///< Entry type (@see enum CONSOLE_PARSE_ENTRY_TYPE)
	union {
		CParseFunc func;
		VECTOR_DECL(struct CParseEntry *) children;
	} u;
};

#ifdef CONSOLE_INPUT
struct console_input_interface {
	/* vars */
	SPIN_LOCK ptlock;/* parse thread lock */
	rAthread *pthread;/* parse thread */
	volatile int32 ptstate;/* parse thread state */
	ramutex *ptmutex;/* parse thread mutex */
	racond *ptcond;/* parse thread cond */
	/* */
	VECTOR_DECL(struct CParseEntry *) command_list;
	VECTOR_DECL(struct CParseEntry *) commands;
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
};
#else
struct console_input_interface;
#endif

struct console_interface {
	void (*init) (void);
	void (*final) (void);
	void (*display_title) (void);

	struct console_input_interface *input;
};

#ifdef HERCULES_CORE
void console_defaults(void);
#endif // HERCULES_CORE

HPShared struct console_interface *console;

#endif /* COMMON_CONSOLE_H */
