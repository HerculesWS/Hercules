/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H

#include "common/hercules.h"
#include "common/db.h"
#include "common/spinlock.h"

/* Forward Declarations */
struct Sql; // common/sql.h
struct cond_data;
struct mutex_data;
struct spin_lock;
struct thread_handle;

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

struct console_input_interface {
#ifdef CONSOLE_INPUT
	/* vars */
	struct spin_lock *ptlock;      ///< parse thread lock.
	struct thread_handle *pthread; ///< parse thread.
	volatile int32 ptstate;        ///< parse thread state.
	struct mutex_data *ptmutex;    ///< parse thread mutex.
	struct cond_data *ptcond;      ///< parse thread conditional variable.
	/* */
	VECTOR_DECL(struct CParseEntry *) command_list;
	VECTOR_DECL(struct CParseEntry *) commands;
	/* */
	struct Sql *SQL;
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
	void (*setSQL) (struct Sql *SQL_handle);
#else // not CONSOLE_INPUT
	UNAVAILABLE_STRUCT;
#endif
};

struct console_interface {
	void (*init) (void);
	void (*final) (void);
	void (*display_title) (void);
	void (*display_gplnotice) (void);

	struct console_input_interface *input;
};

#ifdef HERCULES_CORE
void console_defaults(void);
#endif // HERCULES_CORE

HPShared struct console_interface *console;

#endif /* COMMON_CONSOLE_H */
