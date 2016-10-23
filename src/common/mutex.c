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
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/timer.h"

#ifdef WIN32
#include "common/winapi.h"
#else
#include <pthread.h>
#include <sys/time.h>
#endif

/** @file
 * Implementation of the mutex interface.
 */

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

/* Mutex */

/// @copydoc mutex_interface::create()
struct mutex_data *mutex_create(void)
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
}

/// @copydoc mutex_interface::destroy()
void mutex_destroy(struct mutex_data *m)
{
	nullpo_retv(m);
#ifdef WIN32
	DeleteCriticalSection(&m->hMutex);
#else
	pthread_mutex_destroy(&m->hMutex);
#endif

	aFree(m);
}

/// @copydoc mutex_interface::lock()
void mutex_lock(struct mutex_data *m)
{
	nullpo_retv(m);
#ifdef WIN32
	EnterCriticalSection(&m->hMutex);
#else
	pthread_mutex_lock(&m->hMutex);
#endif
}

/// @copydoc mutex_interface::trylock()
bool mutex_trylock(struct mutex_data *m)
{
	nullpo_retr(false, m);
#ifdef WIN32
	if (TryEnterCriticalSection(&m->hMutex) != FALSE)
		return true;
#else
	if (pthread_mutex_trylock(&m->hMutex) == 0)
		return true;
#endif
	return false;
}

/// @copydoc mutex_interface::unlock()
void mutex_unlock(struct mutex_data *m)
{
	nullpo_retv(m);
#ifdef WIN32
	LeaveCriticalSection(&m->hMutex);
#else
	pthread_mutex_unlock(&m->hMutex);
#endif
}

/* Conditional variable */

/// @copydoc mutex_interface::cond_create()
struct cond_data *cond_create(void)
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
}

/// @copydoc mutex_interface::cond_destroy()
void cond_destroy(struct cond_data *c)
{
	nullpo_retv(c);
#ifdef WIN32
	CloseHandle(c->events[EVENT_COND_SIGNAL]);
	CloseHandle(c->events[EVENT_COND_BROADCAST]);
	DeleteCriticalSection(&c->waiters_lock);
#else
	pthread_cond_destroy(&c->hCond);
#endif

	aFree(c);
}

/// @copydoc mutex_interface::cond_wait()
void cond_wait(struct cond_data *c, struct mutex_data *m, sysint timeout_ticks)
{
#ifdef WIN32
	register DWORD ms;
	int result;
	bool is_last = false;

	nullpo_retv(c);
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
	nullpo_retv(m);
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
}

/// @copydoc mutex_interface::cond_signal()
void cond_signal(struct cond_data *c)
{
#ifdef WIN32
#	if 0
	bool has_waiters = false;
	nullpo_retv(c);
	EnterCriticalSection(&c->waiters_lock);
	if(c->nWaiters > 0)
		has_waiters = true;
	LeaveCriticalSection(&c->waiters_lock);

	if(has_waiters == true)
#	endif // 0
		SetEvent(c->events[EVENT_COND_SIGNAL]);
#else
	nullpo_retv(c);
	pthread_cond_signal(&c->hCond);
#endif
}

/// @copydoc mutex_interface::cond_broadcast()
void cond_broadcast(struct cond_data *c)
{
#ifdef WIN32
#	if 0
	bool has_waiters = false;
	nullpo_retv(c);
	EnterCriticalSection(&c->waiters_lock);
	if(c->nWaiters > 0)
		has_waiters = true;
	LeaveCriticalSection(&c->waiters_lock);

	if(has_waiters == true)
#	endif // 0
		SetEvent(c->events[EVENT_COND_BROADCAST]);
#else
	nullpo_retv(c);
	pthread_cond_broadcast(&c->hCond);
#endif
}

/**
 * Interface base initialization.
 */
void mutex_defaults(void)
{
	mutex = &mutex_s;
	mutex->create = mutex_create;
	mutex->destroy = mutex_destroy;
	mutex->lock = mutex_lock;
	mutex->trylock = mutex_trylock;
	mutex->unlock = mutex_unlock;

	mutex->cond_create = cond_create;
	mutex->cond_destroy = cond_destroy;
	mutex->cond_wait = cond_wait;
	mutex->cond_signal = cond_signal;
	mutex->cond_broadcast = cond_broadcast;
}
