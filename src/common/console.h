// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef	_CONSOLE_H_
#define	_CONSOLE_H_

#include "../common/thread.h"
#include "../common/mutex.h"
#include "../common/spinlock.h"
#include "../config/core.h"

/**
 * Queue Max
 * why is there a limit, why not make it dynamic? - I'm playing it safe, I'd rather not play with memory management between threads
 **/
#define CONSOLE_PARSE_SIZE 10
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
	void (*parse_init) (void);
	void (*parse_final) (void);
	int (*parse_timer) (int tid, unsigned int tick, int id, intptr_t data);
	void *(*pthread_main) (void *x);
	void (*parse) (char* line);
	int (*key_pressed) (void);
#endif
};

struct console_interface *console;

void console_defaults(void);

#endif /* _CONSOLE_H_ */
