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
#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include "common/hercules.h"

typedef struct rAthread rAthread;
typedef void* (*rAthreadProc)(void*);

typedef enum RATHREAD_PRIO {
	RAT_PRIO_LOW = 0,
	RAT_PRIO_NORMAL,
	RAT_PRIO_HIGH
} RATHREAD_PRIO;

struct thread_interface {
	/**
	 * Creates a new Thread.
	 *
	 * @param entyPoint Thread's entry point.
	 * @param param     General purpose parameter, would be given as
	 *                  parameter to the thread's entry point.
	 *
	 * @return The created thread object.
	 * @retval NULL in vase of failure.
	 */
	rAthread *(*create) (rAthreadProc entryPoint, void *param);

	/**
	 * Creates a new Thread (with more creation options).
	 *
	 * @param entyPoint Thread's entry point.
	 * @param param     General purpose parameter, would be given as
	 *                  parameter to the thread's entry point.
	 * @param szStack   Stack Size in bytes.
	 * @param prio      Priority of the Thread in the OS scheduler.
	 *
	 * @return The created thread object.
	 * @retval NULL in case of failure.
	 */
	rAthread *(*createEx) (rAthreadProc entryPoint, void *param, size_t szStack, RATHREAD_PRIO prio);

	/**
	 * Destroys the given Thread immediately.
	 *
	 * @remark
	 *   The Handle gets invalid after call! don't use it afterwards.
	 *
	 * @param handle The thread to destroy.
	 */
	void (*destroy) (rAthread *handle);

	/**
	 * Returns the thread handle of the thread calling this function.
	 *
	 * @remark
	 *   This won't work in the program's main thread.
	 *
	 * @warning
	 *   The underlying implementation might not perform very well, cache
	 *   the value received!
	 *
	 * @return the thread handle.
	 * @retval NULL in case of failure.
	 */
	rAthread *(*self) (void);

	/**
	 * Returns own thread id (TID).
	 *
	 * @remark
	 *   This is an unique identifier for the calling thread, and depends
	 *   on platform/ compiler, and may not be the systems Thread ID!
	 *
	 * @return the thread ID.
	 * @retval -1 in case of failure.
	 */
	int (*get_tid) (void);

	/**
	 * Waits for the given thread to terminate.
	 *
	 * @param[in]  handle       The thread to wait (join) for.
	 * @param[out] out_Exitcode Pointer to return the exit code of the
	 *                          given thread after termination (optional).
	 *
	 * @retval true if the given thread has been terminated.
	 */
	bool (*wait) (rAthread *handle, void **out_exitCode);

	/**
	 * Sets the given priority in the OS scheduler.
	 *
	 * @param handle The thread to set the priority for.
	 * @param prio   The priority to set (@see enum RATHREAD_PRIO).
	 */
	void (*prio_set) (rAthread *handle, RATHREAD_PRIO prio);

	/**
	 * Gets the current priority of the given thread.
	 *
	 * @param handle The thread to get the priority for.
	 */
	RATHREAD_PRIO (*prio_get) (rAthread *handle);

	/**
	 * Tells the OS scheduler to yield the execution of the calling thread.
	 *
	 * @remark
	 *   This will not "pause" the thread, it just allows the OS to spend
	 *   the remaining time of the slice to another thread.
	 */
	void (*yield) (void);

	void (*init) (void);
	void (*final) (void);
};

#ifdef HERCULES_CORE
void thread_defaults(void);
#endif // HERCULES_CORE

HPShared struct thread_interface *thread;

#endif /* COMMON_THREAD_H */
