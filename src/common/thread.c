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

#include "thread.h"

#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/showmsg.h"
#include "common/sysinfo.h" // sysinfo->getpagesize()

#ifdef WIN32
#	include "common/winapi.h"
#	define __thread __declspec( thread )
#else
#	include <pthread.h>
#	include <sched.h>
#	include <signal.h>
#	include <stdlib.h>
#	include <string.h>
#	include <unistd.h>
#endif

// When Compiling using MSC (on win32..) we know we have support in any case!
#ifdef _MSC_VER
#define HAS_TLS
#endif

/** @file
 * Thread interface implementation.
 * @author Florian Wilkemeyer <fw@f-ws.de>
 */

struct thread_interface thread_s;
struct thread_interface *thread;

/// The maximum amount of threads.
#define THREADS_MAX 64

struct thread_handle {
	unsigned int myID;

	enum thread_priority prio;
	threadFunc proc;
	void *param;

	#ifdef WIN32
	HANDLE hThread;
	#else
	pthread_t hThread;
	#endif
};

#ifdef HAS_TLS
__thread int g_rathread_ID = -1;
#endif

// Subystem Code

static struct thread_handle l_threads[THREADS_MAX];

/// @copydoc thread_interface::init()
void thread_init(void)
{
	register int i;
	memset(&l_threads, 0x00, THREADS_MAX * sizeof(struct thread_handle));

	for (i = 0; i < THREADS_MAX; i++) {
		l_threads[i].myID = i;
	}

	// now lets init thread id 0, which represents the main thread
#ifdef HAS_TLS
	g_rathread_ID = 0;
#endif
	l_threads[0].prio = THREADPRIO_NORMAL;
	l_threads[0].proc = (threadFunc)0xDEADCAFE;

}

/// @copydoc thread_interface::final()
void thread_final(void)
{
	register int i;

	// Unterminated Threads Left?
	// Shouldn't happen ... Kill 'em all!
	for (i = 1; i < THREADS_MAX; i++) {
		if (l_threads[i].proc != NULL){
			ShowWarning("thread_final: unterminated Thread (tid %d entry_point %p) - forcing to terminate (kill)\n", i, l_threads[i].proc);
			thread->destroy(&l_threads[i]);
		}
	}
}

/**
 * Gets called whenever a thread terminated.
 *
 * @param handle The terminated thread's handle.
 */
static void thread_terminated(struct thread_handle *handle)
{
	// Preserve handle->myID and handle->hThread, set everything else to its default value
	handle->param = NULL;
	handle->proc = NULL;
	handle->prio = THREADPRIO_NORMAL;
}

#ifdef WIN32
DWORD WINAPI thread_main_redirector(LPVOID p)
{
#else
static void *thread_main_redirector(void *p)
{
	sigset_t set; // on Posix Thread platforms
#endif
	void *ret;
	struct thread_handle *self = p;

	// Update myID @ TLS to right id.
#ifdef HAS_TLS
	g_rathread_ID = self->myID;
#endif

#ifndef WIN32
	// When using posix threads
	// the threads inherits the Signal mask from the thread which spawned
	// this thread
	// so we've to block everything we don't care about.
	(void)sigemptyset(&set);
	(void)sigaddset(&set, SIGINT);
	(void)sigaddset(&set, SIGTERM);
	(void)sigaddset(&set, SIGPIPE);

	pthread_sigmask(SIG_BLOCK, &set, NULL);
#endif

	ret = self->proc(self->param);

#ifdef WIN32
	CloseHandle(self->hThread);
#endif

	thread_terminated(self);
#ifdef WIN32
	return (DWORD)ret;
#else
	return ret;
#endif
}

// API Level

/// @copydoc thread_interface::create()
struct thread_handle *thread_create(threadFunc entry_point, void *param)
{
	return thread->create_opt(entry_point, param,  (1<<23) /*8MB*/, THREADPRIO_NORMAL);
}

/// @copydoc thread_interface::create_opt()
struct thread_handle *thread_create_opt(threadFunc entry_point, void *param, size_t stack_size, enum thread_priority prio)
{
#ifndef WIN32
	pthread_attr_t attr;
#endif
	size_t tmp;
	int i;
	struct thread_handle *handle = NULL;

	// given stacksize aligned to systems pagesize?
	tmp = stack_size % sysinfo->getpagesize();
	if (tmp != 0)
		stack_size += tmp;

	// Get a free Thread Slot.
	for (i = 0; i < THREADS_MAX; i++) {
		if(l_threads[i].proc == NULL){
			handle = &l_threads[i];
			break;
		}
	}

	if (handle == NULL) {
		ShowError("thread_create_opt: cannot create new thread (entry_point: %p) - no free thread slot found!", entry_point);
		return NULL;
	}

	handle->proc = entry_point;
	handle->param = param;

#ifdef WIN32
	handle->hThread = CreateThread(NULL, stack_size, thread_main_redirector, handle, 0, NULL);
#else
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, stack_size);

	if (pthread_create(&handle->hThread, &attr, thread_main_redirector, handle) != 0) {
		handle->proc = NULL;
		handle->param = NULL;
		return NULL;
	}
	pthread_attr_destroy(&attr);
#endif

	thread->prio_set(handle,  prio);

	return handle;
}

/// @copydoc thread_interface::destroy()
void thread_destroy(struct thread_handle *handle)
{
#ifdef WIN32
	if (TerminateThread(handle->hThread, 0) != FALSE) {
		CloseHandle(handle->hThread);
		thread_terminated(handle);
	}
#else
	if (pthread_cancel(handle->hThread) == 0) {
		// We have to join it, otherwise pthread wont re-cycle its internal resources assoc. with this thread.
		pthread_join(handle->hThread, NULL);

		// Tell our manager to release resources ;)
		thread_terminated(handle);
	}
#endif
}

struct thread_handle *thread_self(void)
{
#ifdef HAS_TLS
	struct thread_handle *handle = &l_threads[g_rathread_ID];

	if (handle->proc != NULL) // entry point set, so its used!
		return handle;
#else
	// .. so no tls means we have to search the thread by its api-handle ..
	int i;

#ifdef WIN32
	HANDLE hSelf;
	hSelf = GetCurrent = GetCurrentThread();
#else
	pthread_t hSelf;
	hSelf = pthread_self();
#endif

	for (i = 0; i < THREADS_MAX; i++) {
		if (l_threads[i].hThread == hSelf  &&  l_threads[i].proc != NULL)
			return &l_threads[i];
	}
#endif

	return NULL;
}

/// @copydoc thread_interface::get_tid()
int thread_get_tid(void)
{
#if defined(HAS_TLS)
	return g_rathread_ID;
#elif defined(WIN32)
	return (int)GetCurrentThreadId();
#else
	return (int)pthread_self();
#endif
}

/// @copydoc thread_interface::wait()
bool thread_wait(struct thread_handle *handle, void **out_exit_code)
{
	// Hint:
	// no thread data cleanup routine call here!
	// its managed by the callProxy itself..
#ifdef WIN32
	WaitForSingleObject(handle->hThread, INFINITE);
	return true;
#else
	if (pthread_join(handle->hThread, out_exit_code) == 0)
		return true;
	return false;
#endif

}

/// @copydoc thread_interface::prio_set()
void thread_prio_set(struct thread_handle *handle, enum thread_priority prio)
{
	handle->prio = THREADPRIO_NORMAL;
	//@TODO
}

/// @copydoc thread_interface::prio_get()
enum thread_priority thread_prio_get(struct thread_handle *handle)
{
	return handle->prio;
}

/// @copydoc thread_interface::yield()
void thread_yield(void) {
#ifdef WIN32
	SwitchToThread();
#else
	sched_yield();
#endif
}

/// Interface base initialization.
void thread_defaults(void)
{
	thread = &thread_s;
	thread->init = thread_init;
	thread->final = thread_final;
	thread->create = thread_create;
	thread->create_opt = thread_create_opt;
	thread->destroy = thread_destroy;
	thread->self = thread_self;
	thread->get_tid = thread_get_tid;
	thread->wait = thread_wait;
	thread->prio_set = thread_prio_set;
	thread->prio_get = thread_prio_get;
	thread->yield = thread_yield;
}
