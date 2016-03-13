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

// Basic Threading abstraction (for pthread / win32 based systems)
//
// Author: Florian Wilkemeyer <fw@f-ws.de>

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

struct thread_interface thread_s;
struct thread_interface *thread;

///
/// Subystem Code
///
static struct thread_handle l_threads[THREADS_MAX];

void rathread_init(void) {
	register unsigned int i;
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

}//end: rathread_init()

void rathread_final(void) {
	register unsigned int i;

	// Unterminated Threads Left?
	// Shouldn't happen ..
	// Kill 'em all!
	//
	for (i = 1; i < THREADS_MAX; i++) {
		if(l_threads[i].proc != NULL){
			ShowWarning("rAthread_final: unterminated Thread (tid %u entryPoint %p) - forcing to terminate (kill)\n", i, l_threads[i].proc);
			thread->destroy(&l_threads[i]);
		}
	}

}//end: rathread_final()

// gets called whenever a thread terminated ..
static void rat_thread_terminated(struct thread_handle *handle)
{
	// Preserve handle->myID and handle->hThread, set everything else to its default value
	handle->param = NULL;
	handle->proc = NULL;
	handle->prio = THREADPRIO_NORMAL;
}//end: rat_thread_terminated()

#ifdef WIN32
DWORD WINAPI raThreadMainRedirector(LPVOID p)
{
#else
static void *raThreadMainRedirector(void *p)
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

	rat_thread_terminated(self);
#ifdef WIN32
	return (DWORD)ret;
#else
	return ret;
#endif
}//end: raThreadMainRedirector()

///
/// API Level
///
struct thread_handle *rathread_create(threadFunc entryPoint, void *param)
{
	return thread->createEx(entryPoint, param,  (1<<23) /*8MB*/, THREADPRIO_NORMAL);
}//end: rathread_create()

struct thread_handle *rathread_createEx(threadFunc entryPoint, void *param, size_t szStack, enum thread_priority prio)
{
#ifndef WIN32
	pthread_attr_t attr;
#endif
	size_t tmp;
	unsigned int i;
	struct thread_handle *handle = NULL;

	// given stacksize aligned to systems pagesize?
	tmp = szStack % sysinfo->getpagesize();
	if(tmp != 0)
		szStack += tmp;

	// Get a free Thread Slot.
	for (i = 0; i < THREADS_MAX; i++) {
		if(l_threads[i].proc == NULL){
			handle = &l_threads[i];
			break;
		}
	}

	if(handle == NULL){
		ShowError("rAthread: cannot create new thread (entryPoint: %p) - no free thread slot found!", entryPoint);
		return NULL;
	}

	handle->proc = entryPoint;
	handle->param = param;

#ifdef WIN32
	handle->hThread = CreateThread(NULL, szStack, raThreadMainRedirector, (void*)handle, 0, NULL);
#else
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, szStack);

	if(pthread_create(&handle->hThread, &attr, raThreadMainRedirector, (void*)handle) != 0){
		handle->proc = NULL;
		handle->param = NULL;
		return NULL;
	}
	pthread_attr_destroy(&attr);
#endif

	thread->prio_set( handle,  prio );

	return handle;
}//end: rathread_createEx

void rathread_destroy(struct thread_handle *handle)
{
#ifdef WIN32
	if( TerminateThread(handle->hThread, 0) != FALSE){
		CloseHandle(handle->hThread);
		rat_thread_terminated(handle);
	}
#else
	if( pthread_cancel( handle->hThread ) == 0){
		// We have to join it, otherwise pthread wont re-cycle its internal resources assoc. with this thread.
		pthread_join( handle->hThread, NULL );

		// Tell our manager to release resources ;)
		rat_thread_terminated(handle);
	}
#endif
}//end: rathread_destroy()

struct thread_handle *rathread_self(void)
{
#ifdef HAS_TLS
	struct thread_handle *handle = &l_threads[g_rathread_ID];

	if(handle->proc != NULL) // entry point set, so its used!
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
		if(l_threads[i].hThread == hSelf  &&  l_threads[i].proc != NULL)
			return &l_threads[i];
	}
#endif

	return NULL;
}//end: rathread_self()

int rathread_get_tid(void) {

#ifdef HAS_TLS
	return g_rathread_ID;
#else
	// TODO
	#ifdef WIN32
		return (int)GetCurrentThreadId();
	#else
		return (int)pthread_self();
	#endif
#endif

}//end: rathread_get_tid()

bool rathread_wait(struct thread_handle *handle, void **out_exitCode)
{
	// Hint:
	// no thread data cleanup routine call here!
	// its managed by the callProxy itself..
	//
#ifdef WIN32
	WaitForSingleObject(handle->hThread, INFINITE);
	return true;
#else
	if(pthread_join(handle->hThread, out_exitCode) == 0)
		return true;
	return false;
#endif

}//end: rathread_wait()

void rathread_prio_set(struct thread_handle *handle, enum thread_priority prio)
{
	handle->prio = THREADPRIO_NORMAL;
	//@TODO
}//end: rathread_prio_set()

enum thread_priority rathread_prio_get(struct thread_handle *handle)
{
	return handle->prio;
}//end: rathread_prio_get()

void rathread_yield(void) {
#ifdef WIN32
	SwitchToThread();
#else
	sched_yield();
#endif
}//end: rathread_yield()

void thread_defaults(void)
{
	thread = &thread_s;
	thread->create = rathread_create;
	thread->createEx = rathread_createEx;
	thread->destroy = rathread_destroy;
	thread->self = rathread_self;
	thread->get_tid = rathread_get_tid;
	thread->wait = rathread_wait;
	thread->prio_set = rathread_prio_set;
	thread->prio_get = rathread_prio_get;
	thread->yield = rathread_yield;
	thread->init = rathread_init;
	thread->final = rathread_final;
}
