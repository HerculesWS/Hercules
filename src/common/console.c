// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/showmsg.h"
#include "../common/core.h"
#include "../config/core.h"
#include "console.h"

#ifndef MINICORE
	#include "../common/atomic.h"
	#include "../common/spinlock.h"
	#include "../common/thread.h"
	#include "../common/mutex.h"
	#include "../common/timer.h"
	#include "../common/strlib.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
	#include <unistd.h>
#else
	#include "../common/winapi.h" // Console close event handling
#endif

#ifdef CONSOLE_INPUT
	#ifdef _WIN32
		#include <conio.h> /* _kbhit() */
	#endif
#endif

struct console_interface console_s;

/*======================================
 *	CORE : Display title
 *--------------------------------------*/
void display_title(void) {
	const char* svn = get_svn_revision();
	const char* git = get_git_hash();

	ShowMessage("\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"                                                                 "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"       "CL_BT_WHITE"                Hercules Development Team presents                  "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"                _   _                     _           "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"               | | | |                   | |          "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"               | |_| | ___ _ __ ___ _   _| | ___  ___ "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"               |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|"CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"               | | | |  __/ | | (__| |_| | |  __/\\__ \\"CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"               \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/"CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"                                                                 "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"       "CL_BT_WHITE"                   http://hercules.ws/board/                        "CL_BG_RED""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_BG_RED"     "CL_BT_WHITE"                                                                 "CL_BG_RED""CL_CLL""CL_NORMAL"\n");

	if( git[0] != HERC_UNKNOWN_VER )
		ShowInfo("Git Hash: '"CL_WHITE"%s"CL_RESET"'\n", git);
	else if( svn[0] != HERC_UNKNOWN_VER )
		ShowInfo("SVN Revision: '"CL_WHITE"%s"CL_RESET"'\n", svn);
}
#ifdef CONSOLE_INPUT
#ifdef _WIN32
int console_parse_key_pressed(void) {
	return _kbhit();
}
#else /* _WIN32 */
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
		
	while( console->ptstate ) {/* loopx */
		if( console->key_pressed() ) {
			char input[MAX_CONSOLE_INPUT];
					
			console->parse(input);
			if( input[0] != '\0' ) {/* did we get something? */
				EnterSpinLock(&console->ptlock);
				
				if( cinput.count == CONSOLE_PARSE_SIZE ) {
					LeaveSpinLock(&console->ptlock);
					continue;/* drop */
				}
				
				safestrncpy(cinput.queue[cinput.count++],input,MAX_CONSOLE_INPUT);
				LeaveSpinLock(&console->ptlock);
			}
		}
		ramutex_lock( console->ptmutex );
		racond_wait( console->ptcond,	console->ptmutex,  -1 );
		ramutex_unlock( console->ptmutex );
	}
		
	return NULL;
}
int console_parse_timer(int tid, unsigned int tick, int id, intptr_t data) {
	int i;
	EnterSpinLock(&console->ptlock);
	for(i = 0; i < cinput.count; i++) {
		parse_console(cinput.queue[i]);
	}
	cinput.count = 0;
	LeaveSpinLock(&console->ptlock);
	racond_signal(console->ptcond);
	return 0;
}
void console_parse_final(void) {
	InterlockedDecrement(&console->ptstate);
	racond_signal(console->ptcond);
	
	/* wait for thread to close */
	rathread_wait(console->pthread, NULL);
	
	racond_destroy(console->ptcond);
	ramutex_destroy(console->ptmutex);
	
}
void console_parse_init(void) {
	cinput.count = 0;
	
	console->ptstate = 1;

	InitializeSpinLock(&console->ptlock);
	
	console->ptmutex = ramutex_create();
	console->ptcond = racond_create();
	
	if( (console->pthread = rathread_create(console->pthread_main, NULL)) == NULL ){
		ShowFatalError("console_parse_init: failed to spawn console_parse thread.\n");
		exit(EXIT_FAILURE);
	}
	
	add_timer_func_list(console->parse_timer, "console_parse_timer");
	add_timer_interval(gettick() + 1000, console->parse_timer, 0, 0, 500);/* start listening in 1s; re-try every 0.5s */
	
}
#endif /* CONSOLE_INPUT */

void console_init (void) {
#ifdef CONSOLE_INPUT
	console->parse_init();
#endif
}
void console_final(void) {
#ifdef CONSOLE_INPUT
	console->parse_final();
#endif
}
void console_defaults(void) {
	console = &console_s;
	console->init = console_init;
	console->final = console_final;
	console->display_title = display_title;
#ifdef CONSOLE_INPUT
	console->parse_init = console_parse_init;
	console->parse_final = console_parse_final;
	console->parse_timer = console_parse_timer;
	console->pthread_main = cThread_main;
	console->parse = console_parse;
	console->key_pressed = console_parse_key_pressed;
#endif
}