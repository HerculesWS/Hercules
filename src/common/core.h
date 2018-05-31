/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef COMMON_CORE_H
#define COMMON_CORE_H

#include "common/hercules.h"
#include "common/db.h"

/* so that developers with --enable-debug can raise signals from any section of the code they'd like */
#ifdef DEBUG
#	include <signal.h>
#endif

#define HERC_UNKNOWN_VER '\x02'

enum server_types {
	SERVER_TYPE_UNKNOWN = 0x0,
	SERVER_TYPE_LOGIN   = 0x1,
	SERVER_TYPE_CHAR    = 0x2,
	SERVER_TYPE_MAP     = 0x4,
};

/// The main loop continues until runflag is CORE_ST_STOP
enum E_CORE_ST {
	CORE_ST_STOP = 0,
	CORE_ST_RUN,
	CORE_ST_LAST
};

/// Options for command line argument handlers.
enum cmdline_options {
	CMDLINE_OPT_NORMAL         = 0x0, ///< No special options.
	CMDLINE_OPT_PARAM          = 0x1, ///< An additional value parameter is expected.
	CMDLINE_OPT_SILENT         = 0x2, ///< If this command-line argument is passed, the server won't print any messages.
	CMDLINE_OPT_PREINIT        = 0x4, ///< This command-line argument is executed before initializing the HPM.
};
typedef bool (*CmdlineExecFunc)(const char *name, const char *params);
struct CmdlineArgData {
	unsigned int pluginID; ///< Plugin ID (HPM_PID_CORE if used by the core)
	unsigned int options;  ///< Command line argument options (@see enum cmdline_options)
	char *name;            ///< Command-line argument (i.e. "--my-arg", "--version", "--whatever")
	char shortname;        ///< Short form (i.e. "-v") - only store the 'v' part.
	CmdlineExecFunc func;  ///< Function to call
	char *help;            ///< Help message
};

struct cmdline_interface {
	VECTOR_DECL(struct CmdlineArgData) args_data;

	void (*init) (void);
	void (*final) (void);
	bool (*arg_add) (unsigned int pluginID, const char *name, char shortname, CmdlineExecFunc func, const char *help, unsigned int options);
	int (*exec) (int argc, char **argv, unsigned int options);
	bool (*arg_next_value) (const char *name, int current_arg, int argc);
	const char *(*arg_source) (struct CmdlineArgData *arg);
};

struct core_interface {
	int arg_c;
	char **arg_v;
	/// @see E_CORE_ST
	int runflag;
	char *server_name;
	enum server_types server_type;

	/// Called when a terminate signal is received. (Ctrl+C pressed)
	/// If NULL, runflag is set to CORE_ST_STOP instead.
	void (*shutdown_callback)(void);
};

#define CMDLINEARG(x) bool cmdline_arg_ ## x (const char *name, const char *params)
#define SERVER_NAME (core->server_name)
#define SERVER_TYPE (core->server_type)

#ifdef HERCULES_CORE
extern void cmdline_args_init_local(void);
extern int do_init(int,char**);
extern void set_server_type(void);
extern void do_abort(void);
extern int do_final(void);

/// Special plugin ID assigned to the Hercules core
#define HPM_PID_CORE ((unsigned int)-1)

#define CMDLINEARG_DEF(name, shortname, help, options) cmdline->arg_add(HPM_PID_CORE, "--" EXPAND_AND_QUOTE(name), shortname, cmdline_arg_ ## name, help, options)
#define CMDLINEARG_DEF2(name, funcname, help, options) cmdline->arg_add(HPM_PID_CORE, "--" EXPAND_AND_QUOTE(name), '\0', cmdline_arg_ ## funcname, help, options)

void cmdline_defaults(void);
#endif // HERCULES_CORE

HPShared struct core_interface *core;
HPShared struct cmdline_interface *cmdline;

#endif /* COMMON_CORE_H */
