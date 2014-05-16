// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/utils.h"

#ifdef WIN32
#	include "../common/winapi.h" // GetTickCount()
#else
#	include <sys/time.h> // struct timeval, gettimeofday()
#	include <unistd.h>
#endif

struct timer_interface timer_s;

// If the server can't handle processing thousands of monsters
// or many connected clients, please increase TIMER_MIN_INTERVAL.
#define TIMER_MIN_INTERVAL 50
#define TIMER_MAX_INTERVAL 1000

// timers (array)
static struct TimerData* timer_data = NULL;
static int timer_data_max = 0;
static int timer_data_num = 0;

// free timers (array)
static int* free_timer_list = NULL;
static int free_timer_list_max = 0;
static int free_timer_list_pos = 0;


/// Comparator for the timer heap. (minimum tick at top)
/// Returns negative if tid1's tick is smaller, positive if tid2's tick is smaller, 0 if equal.
///
/// @param tid1 First timer
/// @param tid2 Second timer
/// @return negative if tid1 is top, positive if tid2 is top, 0 if equal
#define DIFFTICK_MINTOPCMP(tid1,tid2) DIFF_TICK(timer_data[tid1].tick,timer_data[tid2].tick)

// timer heap (binary heap of tid's)
static BHEAP_VAR(int, timer_heap);


// server startup time
time_t start_time;


/*----------------------------
 * 	Timer debugging
 *----------------------------*/
struct timer_func_list {
	struct timer_func_list* next;
	TimerFunc func;
	char* name;
} *tfl_root = NULL;

/// Sets the name of a timer function.
int timer_add_func_list(TimerFunc func, char* name) {
	struct timer_func_list* tfl;

	if (name) {
		for( tfl=tfl_root; tfl != NULL; tfl=tfl->next )
		{// check suspicious cases
			if( func == tfl->func )
				ShowWarning("timer_add_func_list: duplicating function %p(%s) as %s.\n",tfl->func,tfl->name,name);
			else if( strcmp(name,tfl->name) == 0 )
				ShowWarning("timer_add_func_list: function %p has the same name as %p(%s)\n",func,tfl->func,tfl->name);
		}
		CREATE(tfl,struct timer_func_list,1);
		tfl->next = tfl_root;
		tfl->func = func;
		tfl->name = aStrdup(name);
		tfl_root = tfl;
	}
	return 0;
}

/// Returns the name of the timer function.
char* search_timer_func_list(TimerFunc func)
{
	struct timer_func_list* tfl;

	for( tfl=tfl_root; tfl != NULL; tfl=tfl->next )
		if (func == tfl->func)
			return tfl->name;

	return "unknown timer function";
}

/*----------------------------
 * 	Get tick time
 *----------------------------*/

#if defined(ENABLE_RDTSC)
static uint64 RDTSC_BEGINTICK = 0,   RDTSC_CLOCK = 0;

static __inline uint64 _rdtsc(){
	register union{
		uint64	qw;
		uint32 	dw[2];
	} t;

	asm volatile("rdtsc":"=a"(t.dw[0]), "=d"(t.dw[1]) );
	
	return t.qw;
}

static void rdtsc_calibrate(){
	uint64 t1, t2;
	int32 i;
	
	ShowStatus("Calibrating Timer Source, please wait... ");
	
	RDTSC_CLOCK = 0;
	
	for(i = 0; i < 5; i++){
		t1 = _rdtsc();
		usleep(1000000); //1000 MS
		t2 = _rdtsc();
		RDTSC_CLOCK += (t2 - t1) / 1000;
	}
	RDTSC_CLOCK /= 5;
	
	RDTSC_BEGINTICK = _rdtsc();
	
	ShowMessage(" done. (Frequency: %u Mhz)\n", (uint32)(RDTSC_CLOCK/1000) );
}

#endif

/**
 * platform-abstracted tick retrieval
 * @return server's current tick
 */
static int64 sys_tick(void) {
#if defined(WIN32)
	// Windows: GetTickCount/GetTickCount64: Return the number of
	//   milliseconds that have elapsed since the system was started.

	// TODO: GetTickCount/GetTickCount64 has a resolution of only 10~15ms.
	//       Ai4rei recommends that we replace it with either performance
	//       counters or multimedia timers if we want it to be more accurate.
	//       I'm leaving this for a future follow-up patch.

	// GetTickCount64 is only available in Windows Vista / Windows Server
	//   2008 or newer. Since we still support older versions, this runtime
	//   check is required in order not to crash.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724411%28v=vs.85%29.aspx
	static bool first = true;
	static ULONGLONG (WINAPI *pGetTickCount64)(void) = NULL;

	if( first ) {
		HMODULE hlib = GetModuleHandle(TEXT("KERNEL32.DLL"));
		if( hlib != NULL )
			pGetTickCount64 = (ULONGLONG (WINAPI *)(void))GetProcAddress(hlib, "GetTickCount64");
		first = false;
	}
	if (pGetTickCount64)
		return (int64)pGetTickCount64();
	// 32-bit fallback. Note: This will wrap around every ~49 days since system startup!!!
	return (int64)GetTickCount();
#elif defined(ENABLE_RDTSC)
	// RDTSC: Returns the number of CPU cycles since reset. Unreliable if
	//   the CPU frequency is variable.
	return (int64)((_rdtsc() - RDTSC_BEGINTICK) / RDTSC_CLOCK);
#elif defined(HAVE_MONOTONIC_CLOCK)
	// Monotinic clock: Implementation-defined.
	//   Clock that cannot be set and represents monotonic time since some
	//   unspecified starting point.  This clock is not affected by
	//   discontinuous jumps in the system time (e.g., if the system
	//   administrator manually changes the  clock),  but  is  affected by
	//   the  incremental adjustments performed by adjtime(3) and NTP.
	struct timespec tval;
	clock_gettime(CLOCK_MONOTONIC, &tval);
	// int64 cast to avoid overflows on platforms where time_t is 32 bit
	return (int64)tval.tv_sec * 1000 + tval.tv_nsec / 1000000;
#else
	// Fallback, regular clock: Number of milliseconds since epoch.
	//   The time returned by gettimeofday() is affected by discontinuous
	//   jumps in the system time (e.g., if the system  administrator
	//   manually  changes  the system time).  If you need a monotonically
	//   increasing clock, see clock_gettime(2).
	struct timeval tval;
	gettimeofday(&tval, NULL);
	// int64 cast to avoid overflows on platforms where time_t is 32 bit
	return (int64)tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined(TICK_CACHE) && TICK_CACHE > 1
//////////////////////////////////////////////////////////////////////////
// tick is cached for TICK_CACHE calls
static int64 gettick_cache;
static int gettick_count = 1;

int64 timer_gettick_nocache(void) {
	gettick_count = TICK_CACHE;
	gettick_cache = sys_tick();
	return gettick_cache;
}

int64 timer_gettick(void) {
	return ( --gettick_count == 0 ) ? gettick_nocache() : gettick_cache;
}
//////////////////////////////
#else
//////////////////////////////
// tick doesn't get cached
int64 timer_gettick_nocache(void)
{
	return sys_tick();
}

int64 timer_gettick(void) {
	return sys_tick();
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------*/

/// Adds a timer to the timer_heap
static void push_timer_heap(int tid) {
	BHEAP_ENSURE(timer_heap, 1, 256);
	BHEAP_PUSH(timer_heap, tid, DIFFTICK_MINTOPCMP, swap);
}

/*==========================
 * 	Timer Management
 *--------------------------*/

/// Returns a free timer id.
static int acquire_timer(void) {
	int tid;

	// select a free timer
	if (free_timer_list_pos) {
		do {
			tid = free_timer_list[--free_timer_list_pos];
		} while(tid >= timer_data_num && free_timer_list_pos > 0);
	} else
		tid = timer_data_num;

	// check available space
	if( tid >= timer_data_num )
		for (tid = timer_data_num; tid < timer_data_max && timer_data[tid].type; tid++);
	if (tid >= timer_data_num && tid >= timer_data_max)
	{// expand timer array
		timer_data_max += 256;
		if( timer_data )
			RECREATE(timer_data, struct TimerData, timer_data_max);
		else
			CREATE(timer_data, struct TimerData, timer_data_max);
		memset(timer_data + (timer_data_max - 256), 0, sizeof(struct TimerData)*256);
	}

	if( tid >= timer_data_num )
		timer_data_num = tid + 1;

	return tid;
}

/// Starts a new timer that is deleted once it expires (single-use).
/// Returns the timer's id.
int timer_add(int64 tick, TimerFunc func, int id, intptr_t data) {
	int tid;
	
	tid = acquire_timer();
	timer_data[tid].tick     = tick;
	timer_data[tid].func     = func;
	timer_data[tid].id       = id;
	timer_data[tid].data     = data;
	timer_data[tid].type     = TIMER_ONCE_AUTODEL;
	timer_data[tid].interval = 1000;
	push_timer_heap(tid);

	return tid;
}

/// Starts a new timer that automatically restarts itself (infinite loop until manually removed).
/// Returns the timer's id, or INVALID_TIMER if it fails.
int timer_add_interval(int64 tick, TimerFunc func, int id, intptr_t data, int interval) {
	int tid;

	if( interval < 1 ) {
		ShowError("timer_add_interval: invalid interval (tick=%"PRId64" %p[%s] id=%d data=%d diff_tick=%"PRId64")\n", tick, func, search_timer_func_list(func), id, data, DIFF_TICK(tick, timer->gettick()));
		return INVALID_TIMER;
	}
	
	tid = acquire_timer();
	timer_data[tid].tick     = tick;
	timer_data[tid].func     = func;
	timer_data[tid].id       = id;
	timer_data[tid].data     = data;
	timer_data[tid].type     = TIMER_INTERVAL;
	timer_data[tid].interval = interval;
	push_timer_heap(tid);

	return tid;
}

/// Retrieves internal timer data
const struct TimerData* timer_get(int tid) {
	return ( tid >= 0 && tid < timer_data_num ) ? &timer_data[tid] : NULL;
}

/// Marks a timer specified by 'id' for immediate deletion once it expires.
/// Param 'func' is used for debug/verification purposes.
/// Returns 0 on success, < 0 on failure.
int timer_do_delete(int tid, TimerFunc func) {
	if( tid < 0 || tid >= timer_data_num ) {
		ShowError("timer_do_delete error : no such timer %d (%p(%s))\n", tid, func, search_timer_func_list(func));
		return -1;
	}
	if( timer_data[tid].func != func ) {
		ShowError("timer_do_delete error : function mismatch %p(%s) != %p(%s)\n", timer_data[tid].func, search_timer_func_list(timer_data[tid].func), func, search_timer_func_list(func));
		return -2;
	}

	timer_data[tid].func = NULL;
	timer_data[tid].type = TIMER_ONCE_AUTODEL;

	return 0;
}

/// Adjusts a timer's expiration time.
/// Returns the new tick value, or -1 if it fails.
int64 timer_addtick(int tid, int64 tick) {
	return timer->settick(tid, timer_data[tid].tick+tick);
}

/// Modifies a timer's expiration time (an alternative to deleting a timer and starting a new one).
/// Returns the new tick value, or -1 if it fails.
int64 timer_settick(int tid, int64 tick) {
	size_t i;
	
	// search timer position
	ARR_FIND(0, BHEAP_LENGTH(timer_heap), i, BHEAP_DATA(timer_heap)[i] == tid);
	if( i == BHEAP_LENGTH(timer_heap) ) {
		ShowError("timer_settick: no such timer %d (%p(%s))\n", tid, timer_data[tid].func, search_timer_func_list(timer_data[tid].func));
		return -1;
	}

	if( tick == -1 )
		tick = 0; // add 1ms to avoid the error value -1

	if( timer_data[tid].tick == tick )
		return tick; // nothing to do, already in proper position

	// pop and push adjusted timer
	BHEAP_POPINDEX(timer_heap, i, DIFFTICK_MINTOPCMP, swap);
	timer_data[tid].tick = tick;
	BHEAP_PUSH(timer_heap, tid, DIFFTICK_MINTOPCMP, swap);
	return tick;
}

/// Executes all expired timers.
/// Returns the value of the smallest non-expired timer (or 1 second if there aren't any).
int do_timer(int64 tick) {
	int64 diff = TIMER_MAX_INTERVAL; // return value

	// process all timers one by one
	while( BHEAP_LENGTH(timer_heap) ) {
		int tid = BHEAP_PEEK(timer_heap);// top element in heap (smallest tick)

		diff = DIFF_TICK(timer_data[tid].tick, tick);
		if( diff > 0 )
			break; // no more expired timers to process

		// remove timer
		BHEAP_POP(timer_heap, DIFFTICK_MINTOPCMP, swap);
		timer_data[tid].type |= TIMER_REMOVE_HEAP;

		if( timer_data[tid].func ) {
			if( diff < -1000 )
				// timer was delayed for more than 1 second, use current tick instead
				timer_data[tid].func(tid, tick, timer_data[tid].id, timer_data[tid].data);
			else
				timer_data[tid].func(tid, timer_data[tid].tick, timer_data[tid].id, timer_data[tid].data);
		}

		// in the case the function didn't change anything...
		if( timer_data[tid].type & TIMER_REMOVE_HEAP ) {
			timer_data[tid].type &= ~TIMER_REMOVE_HEAP;

			switch( timer_data[tid].type ) {
				default:
				case TIMER_ONCE_AUTODEL:
					timer_data[tid].type = 0;
					if (free_timer_list_pos >= free_timer_list_max) {
						free_timer_list_max += 256;
						RECREATE(free_timer_list,int,free_timer_list_max);
						memset(free_timer_list + (free_timer_list_max - 256), 0, 256 * sizeof(int));
					}
					free_timer_list[free_timer_list_pos++] = tid;
				break;
				case TIMER_INTERVAL:
					if( DIFF_TICK(timer_data[tid].tick, tick) < -1000 )
						timer_data[tid].tick = tick + timer_data[tid].interval;
					else
						timer_data[tid].tick += timer_data[tid].interval;
					push_timer_heap(tid);
				break;
			}
		}
	}

	return (int)cap_value(diff, TIMER_MIN_INTERVAL, TIMER_MAX_INTERVAL);
}

unsigned long timer_get_uptime(void) {
	return (unsigned long)difftime(time(NULL), start_time);
}

void timer_init(void)
{
#if defined(ENABLE_RDTSC)
	rdtsc_calibrate();
#endif

	time(&start_time);
}

void timer_final(void) {
	struct timer_func_list *tfl;
	struct timer_func_list *next;

	for( tfl=tfl_root; tfl != NULL; tfl = next ) {
		next = tfl->next;	// copy next pointer
		aFree(tfl->name);	// free structures
		aFree(tfl);
	}

	if (timer_data) aFree(timer_data);
	BHEAP_CLEAR(timer_heap);
	if (free_timer_list) aFree(free_timer_list);
}
/*=====================================
* Default Functions : timer.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void timer_defaults(void) {
	timer = &timer_s;

	/* funcs */
	timer->gettick = timer_gettick;
	timer->gettick_nocache = timer_gettick_nocache;
	timer->add = timer_add;
	timer->add_interval = timer_add_interval;
	timer->add_func_list = timer_add_func_list;
	timer->get = timer_get;
	timer->delete = timer_do_delete;
	timer->addtick = timer_addtick;
	timer->settick = timer_settick;
	timer->get_uptime = timer_get_uptime;
	timer->perform = do_timer;
	timer->init = timer_init;
	timer->final = timer_final;
}
