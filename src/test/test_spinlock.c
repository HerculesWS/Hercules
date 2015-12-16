/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
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
#define HERCULES_CORE

#include "common/atomic.h"
#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/thread.h"
#include "common/spinlock.h"
#include "common/showmsg.h"

#include <stdio.h>
#include <stdlib.h>

//
// Simple test for the spinlock implementation to see if it works properly..
//

#define THRC 32 //thread Count
#define PERINC 100000
#define LOOPS 47

static SPIN_LOCK lock;
static unsigned int val = 0;
static volatile int32 done_threads = 0;

static  void *worker(void *p){
	register int i;

	for(i = 0; i < PERINC; i++){
		EnterSpinLock(&lock);
		EnterSpinLock(&lock);

		val++;

		LeaveSpinLock(&lock);
		LeaveSpinLock(&lock);
	}

	InterlockedIncrement(&done_threads);

	return NULL;
}//end: worker()

int do_init(int argc, char **argv){
	rAthread *t[THRC];
	int j, i;
	int ok;

	ShowStatus("==========\n");
	ShowStatus("TEST: %u Runs,  (%u Threads)\n", LOOPS, THRC);
	ShowStatus("This can take a while\n");
	ShowStatus("\n\n");

	ok =0;
	for(j = 0; j < LOOPS; j++){
		val = 0;
		done_threads = 0;

		InitializeSpinLock(&lock);

		for(i =0; i < THRC; i++){
			t[i] = rathread_createEx( worker,  NULL,  1024*512,  RAT_PRIO_NORMAL);
		}

		while(1){
			if(InterlockedCompareExchange(&done_threads, THRC, THRC) == THRC)
				break;
			rathread_yield();
		}

		FinalizeSpinLock(&lock);

		// Everything fine?
		if (val != (THRC*PERINC)) {
			printf("FAILED! (Result: %u, Expected: %u)\n",  val,  (THRC*PERINC));
		} else {
			printf("OK! (Result: %u, Expected: %u)\n", val, (THRC*PERINC));
			ok++;
		}

	}

	if(ok != LOOPS){
		ShowFatalError("Test failed.\n");
		exit(1);
	}else{
		ShowStatus("Test passed.\n");
		exit(0);
	}
	return 0;
}//end: do_init()

void do_abort(void) {
}//end: do_abort()

void set_server_type(void) {
	SERVER_TYPE = SERVER_TYPE_UNKNOWN;
}//end: set_server_type()

int do_final(void) {
	return EXIT_SUCCESS;
}//end: do_final()

int parse_console(const char* command){
	return 0;
}//end: parse_console

void cmdline_args_init_local(void) { }
