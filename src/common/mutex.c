/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  rAthena Project (www.rathena.org)
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

#include "mutex.h"

#include "common/cbasetypes.h" // for WIN32
#include "common/memmgr.h"
#include "common/showmsg.h"
#include "common/timer.h"

#ifdef WIN32
#include "common/winapi.h"
#else
#include <pthread.h>
#include <sys/time.h>
#endif

struct mutex_interface mutex_s;
struct mutex_interface *mutex;

struct mutex_data {
#ifdef WIN32
	CRITICAL_SECTION hMutex;
#else
	pthread_mutex_t hMutex;
#endif
};

struct cond_data {
#ifdef WIN32
	HANDLE events[2];
	ra_align(8) volatile LONG nWaiters;
	CRITICAL_SECTION waiters_lock;
#define EVENT_COND_SIGNAL 0
#define EVENT_COND_BROADCAST 1
#else
	pthread_cond_t hCond;
#endif
};

////////////////////
// Mutex
//
// Implementation:
//

struct mutex_data *ramutex_create(void)
{
	struct mutex_data *m = aMalloc(sizeof(struct mutex_data));
	if (m == NULL) {
		ShowFatalError("ramutex_create: OOM while allocating %"PRIuS" bytes.\n", sizeof(struct mutex_data));
		return NULL;
	}

#ifdef WIN32
	InitializeCriticalSection(&m->hMutex);
#else
	pthread_mutex_init(&m->hMutex, NULL);
#endif

	return m;
}//end: ramutex_create()

void ramutex_destroy(struct mutex_data *m)
{
#ifdef WIN32
	DeleteCriticalSection(&m->hMutex);
#else
	pthread_mutex_destroy(&m->hMutex);
#endif

	aFree(m);
}//end: ramutex_destroy()

void ramutex_lock(struct mutex_data *m)
{
#ifdef WIN32
	EnterCriticalSection(&m->hMutex);
#else
	pthread_mutex_lock(&m->hMutex);
#endif
}//end: ramutex_lock

bool ramutex_trylock(struct mutex_data *m)
{
#ifdef WIN32
	if (TryEnterCriticalSection(&m->hMutex) != FALSE)
		return true;
#else
	if (pthread_mutex_trylock(&m->hMutex) == 0)
		return true;
#endif
	return false;
}//end: ramutex_trylock()

void ramutex_unlock(struct mutex_data *m)
{
#ifdef WIN32
	LeaveCriticalSection(&m->hMutex);
#else
	pthread_mutex_unlock(&m->hMutex);
#endif
}//end: ramutex_unlock()

///////////////
// Condition Variables
//
// Implementation:
//

struct cond_data *racond_create(void)
{
	struct cond_data *c = aMalloc(sizeof(struct cond_data));
	if (c == NULL) {
		ShowFatalError("racond_create: OOM while allocating %"PRIuS" bytes\n", sizeof(struct cond_data));
		return NULL;
	}

#ifdef WIN32
	c->nWaiters = 0;
	c->events[EVENT_COND_SIGNAL]    = CreateEvent(NULL, FALSE, FALSE, NULL);
	c->events[EVENT_COND_BROADCAST] = CreateEvent(NULL, TRUE,  FALSE, NULL);
	InitializeCriticalSection( &c->waiters_lock );
#else
	pthread_cond_init(&c->hCond, NULL);
#endif

	return c;
}//end: racond_create()

void racond_destroy(struct cond_data *c)
{
#ifdef WIN32
	CloseHandle(c->events[EVENT_COND_SIGNAL]);
	CloseHandle(c->events[EVENT_COND_BROADCAST]);
	DeleteCriticalSection(&c->waiters_lock);
#else
	pthread_cond_destroy(&c->hCond);
#endif

	aFree(c);
}//end: racond_destroy()

void racond_wait(struct cond_data *c, struct mutex_data *m, sysint timeout_ticks)
{
#ifdef WIN32
	register DWORD ms;
	int result;
	bool is_last = false;

	EnterCriticalSection(&c->waiters_lock);
	c->nWaiters++;
	LeaveCriticalSection(&c->waiters_lock);

	if (timeout_ticks < 0)
		ms = INFINITE;
	else
		ms = (timeout_ticks > MAXDWORD) ? (MAXDWORD - 1) : (DWORD)timeout_ticks;

	// we can release the mutex (m) here, cause win's
	// manual reset events maintain state when used with
	// SetEvent()
	mutex->unlock(m);

	result = WaitForMultipleObjects(2, c->events, FALSE, ms);

	EnterCriticalSection(&c->waiters_lock);
	c->nWaiters--;
	if ((result == WAIT_OBJECT_0 + EVENT_COND_BROADCAST) && (c->nWaiters == 0))
		is_last = true; // Broadcast called!
	LeaveCriticalSection(&c->waiters_lock);

	// we are the last waiter that has to be notified, or to stop waiting
	// so we have to do a manual reset
	if (is_last == true)
		ResetEvent( c->events[EVENT_COND_BROADCAST] );

	mutex->lock(m);

#else
	if (timeout_ticks < 0) {
		pthread_cond_wait(&c->hCond,  &m->hMutex);
	} else {
		struct timespec wtime;
		int64 exact_timeout = timer->gettick() + timeout_ticks;

		wtime.tv_sec = exact_timeout/1000;
		wtime.tv_nsec = (exact_timeout%1000)*1000000;

		pthread_cond_timedwait( &c->hCond,  &m->hMutex,  &wtime);
	}
#endif
}//end: racond_wait()

void racond_signal(struct cond_data *c)
{
#ifdef WIN32
#	if 0
	bool has_waiters = false;
	EnterCriticalSection(&c->waiters_lock);
	if(c->nWaiters > 0)
		has_waiters = true;
	LeaveCriticalSection(&c->waiters_lock);

	if(has_waiters == true)
#	endif // 0
		SetEvent( c->events[ EVENT_COND_SIGNAL ] );
#else
	pthread_cond_signal(&c->hCond);
#endif
}//end: racond_signal()

void racond_broadcast(struct cond_data *c)
{
#ifdef WIN32
#	if 0
	bool has_waiters = false;
	EnterCriticalSection(&c->waiters_lock);
	if(c->nWaiters > 0)
		has_waiters = true;
	LeaveCriticalSection(&c->waiters_lock);

	if(has_waiters == true)
#	endif // 0
		SetEvent( c->events[ EVENT_COND_BROADCAST ] );
#else
	pthread_cond_broadcast(&c->hCond);
#endif
}//end: racond_broadcast()

void mutex_defaults(void)
{
	mutex = &mutex_s;
	mutex->create = ramutex_create;
	mutex->destroy = ramutex_destroy;
	mutex->lock = ramutex_lock;
	mutex->trylock = ramutex_trylock;
	mutex->unlock = ramutex_unlock;

	mutex->cond_create = racond_create;
	mutex->cond_destroy = racond_destroy;
	mutex->cond_wait = racond_wait;
	mutex->cond_signal = racond_signal;
	mutex->cond_broadcast = racond_broadcast;
}
