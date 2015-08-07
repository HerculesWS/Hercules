// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "config/core.h" // CONSOLE_INPUT, MAX_CONSOLE_INPUT
#include "console.h"

#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/showmsg.h"
#include "common/sysinfo.h"

#ifndef MINICORE
#	include "common/atomic.h"
#	include "common/ers.h"
#	include "common/malloc.h"
#	include "common/mutex.h"
#	include "common/spinlock.h"
#	include "common/sql.h"
#	include "common/strlib.h"
#	include "common/thread.h"
#	include "common/timer.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if !defined(WIN32)
#	include <sys/time.h>
#	include <unistd.h>
#else
#	include "common/winapi.h" // Console close event handling
#	ifdef CONSOLE_INPUT
#		include <conio.h> /* _kbhit() */
#	endif
#endif

struct console_interface console_s;
#ifdef CONSOLE_INPUT
struct console_input_interface console_input_s;

struct {
	char queue[CONSOLE_PARSE_SIZE][MAX_CONSOLE_INPUT];
	unsigned short count;
} cinput;
#endif

/*======================================
 * CORE : Display title
 *--------------------------------------*/
void display_title(void) {
	const char *vcstype = sysinfo->vcstype();

	ShowMessage("\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                                                                      "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                 Hercules Development Team presents                   "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                _   _                     _                           "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"               | | | |                   | |                          "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"               | |_| | ___ _ __ ___ _   _| | ___  ___                 "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"               |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|                "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"               | | | |  __/ | | (__| |_| | |  __/\\__ \\                "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"               \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/                "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                                                                      "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                    http://herc.ws/board/                         "CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED""CL_BT_WHITE"                                                                      "CL_CLL""CL_NORMAL"\n");

	ShowInfo("Hercules %d-bit for %s\n", sysinfo->is64bit() ? 64 : 32, sysinfo->platform());
	ShowInfo("%s revision (src): '"CL_WHITE"%s"CL_RESET"'\n", vcstype, sysinfo->vcsrevision_src());
	ShowInfo("%s revision (scripts): '"CL_WHITE"%s"CL_RESET"'\n", vcstype, sysinfo->vcsrevision_scripts());
	ShowInfo("OS version: '"CL_WHITE"%s"CL_RESET" [%s]'\n", sysinfo->osversion(), sysinfo->arch());
	ShowInfo("CPU: '"CL_WHITE"%s [%d]"CL_RESET"'\n", sysinfo->cpu(), sysinfo->cpucores());
	ShowInfo("Compiled with %s\n", sysinfo->compiler());
	ShowInfo("Compile Flags: %s\n", sysinfo->cflags());
}
#ifdef CONSOLE_INPUT
#if defined(WIN32)
int console_parse_key_pressed(void) {
	return _kbhit();
}
#else /* WIN32 */
int console_parse_key_pressed(void) {
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);

	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);

	return FD_ISSET(STDIN_FILENO, &fds);
}
#endif /* _WIN32 */

/*======================================
 * CORE: Console commands
 *--------------------------------------*/

/**
 * Stops server
 **/
CPCMD_C(exit,server) {
	runflag = 0;
}

/**
 * Displays ERS-related statistics (Entry Reusage System)
 **/
CPCMD_C(ers_report,server) {
	ers_report();
}

/**
 * Displays memory usage
 **/
CPCMD_C(mem_report,server) {
#ifdef USE_MEMMGR
	memmgr_report(line?atoi(line):0);
#endif
}

/**
 * Displays command list
 **/
CPCMD(help) {
	unsigned int i = 0;
	for ( i = 0; i < console->input->cmd_list_count; i++ ) {
		if( console->input->cmd_list[i]->next_count ) {
			ShowInfo("- '"CL_WHITE"%s"CL_RESET"' subs\n",console->input->cmd_list[i]->cmd);
			console->input->parse_list_subs(console->input->cmd_list[i],2);
		} else {
			ShowInfo("- '"CL_WHITE"%s"CL_RESET"'\n",console->input->cmd_list[i]->cmd);
		}
	}
}

/**
 * [Ind/Hercules]
 * Displays current malloc usage
 */
CPCMD_C(malloc_usage,server) {
	unsigned int val = (unsigned int)iMalloc->usage();
	ShowInfo("malloc_usage: %.2f MB\n",(double)(val)/1024);
}

/**
 * Skips an sql update
 * Usage: sql update skip UPDATE-FILE.sql
 **/
CPCMD_C(skip,update) {
	if( !line ) {
		ShowDebug("usage example: sql update skip 2013-02-14--16-15.sql\n");
		return;
	}
	Sql_HerculesUpdateSkip(console->input->SQL, line);
}

/**
 * Defines a main category.
 *
 * Categories can't be used as commands!
 * E.G.
 * - sql update skip
 *   'sql' is the main category
 * CP_DEF_C(category)
 **/
#define CP_DEF_C(x) { #x , NULL , NULL, NULL }
/**
 * Defines a sub-category.
 *
 * Sub-categories can't be used as commands!
 * E.G.
 * - sql update skip
 *   'update' is a sub-category
 * CP_DEF_C2(command, category)
 **/
#define CP_DEF_C2(x,y) { #x , NULL , #y, NULL }
/**
 * Defines a command that is inside a category or sub-category
 * CP_DEF_S(command, category/sub-category)
 **/
#define CP_DEF_S(x,y) { #x, CPCMD_C_A(x,y), #y, NULL }
/**
 * Defines a command that is _not_ inside any category
 * CP_DEF_S(command)
 **/
#define CP_DEF(x) { #x , CPCMD_A(x), NULL, NULL }

/**
 * Loads console commands list
 * See CP_DEF_C, CP_DEF_C2, CP_DEF_S, CP_DEF
 **/
void console_load_defaults(void) {
	struct {
		char *name;
		CParseFunc func;
		char *connect;
		struct CParseEntry *self;
	} default_list[] = {
		CP_DEF(help),
		/**
		 * Server related commands
		 **/
		CP_DEF_C(server),
		CP_DEF_S(ers_report,server),
		CP_DEF_S(mem_report,server),
		CP_DEF_S(malloc_usage,server),
		CP_DEF_S(exit,server),
		/**
		 * Sql related commands
		 **/
		CP_DEF_C(sql),
		CP_DEF_C2(update,sql),
		CP_DEF_S(skip,update),
	};
	unsigned int i, len = ARRAYLENGTH(default_list);
	struct CParseEntry *cmd;

	RECREATE(console->input->cmds,struct CParseEntry *, len);

	for(i = 0; i < len; i++) {
		CREATE(cmd, struct CParseEntry, 1);

		safestrncpy(cmd->cmd, default_list[i].name, CP_CMD_LENGTH);

		if( default_list[i].func )
			cmd->u.func = default_list[i].func;
		else
			cmd->u.next = NULL;

		cmd->next_count = 0;

		console->input->cmd_count++;
		console->input->cmds[i] = cmd;
		default_list[i].self = cmd;
		if( !default_list[i].connect ) {
			RECREATE(console->input->cmd_list,struct CParseEntry *, ++console->input->cmd_list_count);
			console->input->cmd_list[console->input->cmd_list_count - 1] = cmd;
		}
	}

	for(i = 0; i < len; i++) {
		unsigned int k;
		if( !default_list[i].connect )
			continue;
		for(k = 0; k < console->input->cmd_count; k++) {
			if( strcmpi(default_list[i].connect,console->input->cmds[k]->cmd) == 0 ) {
				cmd = default_list[i].self;
				RECREATE(console->input->cmds[k]->u.next, struct CParseEntry *, ++console->input->cmds[k]->next_count);
				console->input->cmds[k]->u.next[console->input->cmds[k]->next_count - 1] = cmd;
				break;
			}
		}
	}
}
#undef CP_DEF_C
#undef CP_DEF_C2
#undef CP_DEF_S
#undef CP_DEF
void console_parse_create(char *name, CParseFunc func) {
	unsigned int i;
	char *tok;
	char sublist[CP_CMD_LENGTH * 5];
	struct CParseEntry *cmd;

	safestrncpy(sublist, name, CP_CMD_LENGTH * 5);
	tok = strtok(sublist,":");

	for ( i = 0; i < console->input->cmd_list_count; i++ ) {
		if( strcmpi(tok,console->input->cmd_list[i]->cmd) == 0 )
			break;
	}

	if( i == console->input->cmd_list_count ) {
		RECREATE(console->input->cmds,struct CParseEntry *, ++console->input->cmd_count);
		CREATE(cmd, struct CParseEntry, 1);
		safestrncpy(cmd->cmd, tok, CP_CMD_LENGTH);
		cmd->next_count = 0;
		console->input->cmds[console->input->cmd_count - 1] = cmd;
		RECREATE(console->input->cmd_list,struct CParseEntry *, ++console->input->cmd_list_count);
		console->input->cmd_list[console->input->cmd_list_count - 1] = cmd;
		i = console->input->cmd_list_count - 1;
	}

	cmd = console->input->cmd_list[i];
	while( ( tok = strtok(NULL, ":") ) != NULL ) {
		for(i = 0; i < cmd->next_count; i++) {
			if( strcmpi(cmd->u.next[i]->cmd,tok) == 0 )
				break;
		}

		if ( i == cmd->next_count ) {
			RECREATE(console->input->cmds,struct CParseEntry *, ++console->input->cmd_count);
			CREATE(console->input->cmds[console->input->cmd_count-1], struct CParseEntry, 1);
			safestrncpy(console->input->cmds[console->input->cmd_count-1]->cmd, tok, CP_CMD_LENGTH);
			console->input->cmds[console->input->cmd_count-1]->next_count = 0;
			RECREATE(cmd->u.next, struct CParseEntry *, ++cmd->next_count);
			cmd->u.next[cmd->next_count - 1] = console->input->cmds[console->input->cmd_count-1];
			cmd = console->input->cmds[console->input->cmd_count-1];
			continue;
		}
	}
	cmd->u.func = func;
}
void console_parse_list_subs(struct CParseEntry *cmd, unsigned char depth) {
	unsigned int i;
	char msg[CP_CMD_LENGTH * 2];
	for( i = 0; i < cmd->next_count; i++ ) {
		if( cmd->u.next[i]->next_count ) {
			memset(msg, '-', depth);
			snprintf(msg + depth,( CP_CMD_LENGTH * 2 ) - depth, " '"CL_WHITE"%s"CL_RESET"'",cmd->u.next[i]->cmd);
			ShowInfo("%s subs\n",msg);
			console->input->parse_list_subs(cmd->u.next[i],depth + 1);
		} else {
			memset(msg, '-', depth);
			snprintf(msg + depth,(CP_CMD_LENGTH * 2) - depth, " %s",cmd->u.next[i]->cmd);
			ShowInfo("%s\n",msg);
		}
	}
}
void console_parse_sub(char *line) {
	struct CParseEntry *cmd;
	char bline[200];
	char *tok;
	char sublist[CP_CMD_LENGTH * 5];
	unsigned int i, len = 0;

	memcpy(bline, line, 200);
	tok = strtok(line, " ");

	for ( i = 0; i < console->input->cmd_list_count; i++ ) {
		if( strcmpi(tok,console->input->cmd_list[i]->cmd) == 0 )
			break;
	}

	if( i == console->input->cmd_list_count ) {
		ShowError("'"CL_WHITE"%s"CL_RESET"' is not a known command, type '"CL_WHITE"help"CL_RESET"' to list all commands\n",line);
		return;
	}

	cmd = console->input->cmd_list[i];

	len += snprintf(sublist,CP_CMD_LENGTH * 5,"%s", cmd->cmd) + 1;

	if( cmd->next_count == 0 && console->input->cmd_list[i]->u.func ) {
		char *r = NULL;
		if( (tok = strtok(NULL, " ")) ) {
			r = bline;
			r += len + 1;
		}
		cmd->u.func(r);
	} else {
		while( ( tok = strtok(NULL, " ") ) != NULL ) {
			for( i = 0; i < cmd->next_count; i++ ) {
				if( strcmpi(cmd->u.next[i]->cmd,tok) == 0 )
					break;
			}
			if( i == cmd->next_count ) {
				if( strcmpi("help",tok) == 0 ) {
					if( cmd->next_count ) {
						ShowInfo("- '"CL_WHITE"%s"CL_RESET"' subs\n",sublist);
						console->input->parse_list_subs(cmd,2);
					} else {
						ShowError("'"CL_WHITE"%s"CL_RESET"' doesn't possess any subcommands\n",sublist);
					}
					return;
				}
				ShowError("'"CL_WHITE"%s"CL_RESET"' is not a known subcommand of '"CL_WHITE"%s"CL_RESET"'\n",tok,cmd->cmd);
				ShowError("type '"CL_WHITE"%s help"CL_RESET"' to list its subcommands\n",sublist);
				return;
			}
			if( cmd->u.next[i]->next_count == 0 && cmd->u.next[i]->u.func ) {
				char *r = NULL;
				if( (tok = strtok(NULL, " ")) ) {
					r = bline;
					r += len + strlen(cmd->u.next[i]->cmd) + 1;
				}
				cmd->u.next[i]->u.func(r);
				return;
			} else
				cmd = cmd->u.next[i];
			len += snprintf(sublist + len,(CP_CMD_LENGTH * 5) - len,":%s", cmd->cmd);
		}
		ShowError("Is only a category, type '"CL_WHITE"%s help"CL_RESET"' to list its subcommands\n",sublist);
	}
}
void console_parse(char* line) {
	int c, i = 0, len = MAX_CONSOLE_INPUT - 1;/* we leave room for the \0 :P */

	while( (c = fgetc(stdin)) != EOF ) {
		if( --len == 0 )
			break;
		if( (line[i++] = c) == '\n') {
			line[i-1] = '\0';/* clear, we skip the new line */
			break;/* new line~! we leave it for the next cycle */
		}
	}

	line[i++] = '\0';
}
void *cThread_main(void *x) {
	while( console->input->ptstate ) {/* loopx */
		if( console->input->key_pressed() ) {
			char input[MAX_CONSOLE_INPUT];

			console->input->parse(input);
			if( input[0] != '\0' ) {/* did we get something? */
				EnterSpinLock(&console->input->ptlock);

				if( cinput.count == CONSOLE_PARSE_SIZE ) {
					LeaveSpinLock(&console->input->ptlock);
					continue;/* drop */
				}

				safestrncpy(cinput.queue[cinput.count++],input,MAX_CONSOLE_INPUT);
				LeaveSpinLock(&console->input->ptlock);
			}
		}
		ramutex_lock( console->input->ptmutex );
		racond_wait( console->input->ptcond, console->input->ptmutex, -1 );
		ramutex_unlock( console->input->ptmutex );
	}

	return NULL;
}
int console_parse_timer(int tid, int64 tick, int id, intptr_t data) {
	int i;
	EnterSpinLock(&console->input->ptlock);
	for(i = 0; i < cinput.count; i++) {
		console->input->parse_sub(cinput.queue[i]);
	}
	cinput.count = 0;
	LeaveSpinLock(&console->input->ptlock);
	racond_signal(console->input->ptcond);
	return 0;
}
void console_parse_final(void) {
	if( console->input->ptstate ) {
		InterlockedDecrement(&console->input->ptstate);
		racond_signal(console->input->ptcond);

		/* wait for thread to close */
		rathread_wait(console->input->pthread, NULL);

		racond_destroy(console->input->ptcond);
		ramutex_destroy(console->input->ptmutex);
	}
}
void console_parse_init(void) {
	cinput.count = 0;

	console->input->ptstate = 1;

	InitializeSpinLock(&console->input->ptlock);

	console->input->ptmutex = ramutex_create();
	console->input->ptcond = racond_create();

	if( (console->input->pthread = rathread_create(console->input->pthread_main, NULL)) == NULL ){
		ShowFatalError("console_parse_init: failed to spawn console_parse thread.\n");
		exit(EXIT_FAILURE);
	}

	timer->add_func_list(console->input->parse_timer, "console_parse_timer");
	timer->add_interval(timer->gettick() + 1000, console->input->parse_timer, 0, 0, 500);/* start listening in 1s; re-try every 0.5s */
}
void console_setSQL(Sql *SQL_handle) {
	console->input->SQL = SQL_handle;
}
#endif /* CONSOLE_INPUT */

void console_init (void) {
#ifdef CONSOLE_INPUT
	console->input->cmd_count = console->input->cmd_list_count = 0;
	console->input->load_defaults();
	console->input->parse_init();
#endif
}
void console_final(void) {
#ifdef CONSOLE_INPUT
	unsigned int i;
	console->input->parse_final();
	for( i = 0; i < console->input->cmd_count; i++ ) {
		if( console->input->cmds[i]->next_count )
			aFree(console->input->cmds[i]->u.next);
		aFree(console->input->cmds[i]);
	}
	aFree(console->input->cmds);
	aFree(console->input->cmd_list);
#endif
}
void console_defaults(void) {
	console = &console_s;
	console->init = console_init;
	console->final = console_final;
	console->display_title = display_title;
#ifdef CONSOLE_INPUT
	console->input = &console_input_s;
	console->input->parse_init = console_parse_init;
	console->input->parse_final = console_parse_final;
	console->input->parse_timer = console_parse_timer;
	console->input->pthread_main = cThread_main;
	console->input->parse = console_parse;
	console->input->parse_sub = console_parse_sub;
	console->input->key_pressed = console_parse_key_pressed;
	console->input->load_defaults = console_load_defaults;
	console->input->parse_list_subs = console_parse_list_subs;
	console->input->addCommand = console_parse_create;
	console->input->setSQL = console_setSQL;
	console->input->SQL = NULL;
#else
	console->input = NULL;
#endif
}
