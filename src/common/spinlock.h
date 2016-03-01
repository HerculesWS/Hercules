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
#ifndef COMMON_SPINLOCK_H
#define COMMON_SPINLOCK_H

// CAS based Spinlock Implementation
//
// CamelCase names are chosen to be consistent with Microsoft's WinAPI
// which implements CriticalSection by this naming...
//
// Author: Florian Wilkemeyer <fw@f-ws.de>

#include "common/atomic.h"
#include "common/cbasetypes.h"
#include "common/thread.h"

#ifdef WIN32
#include "common/winapi.h"
#endif

#ifdef WIN32

typedef struct __declspec( align(64) ) SPIN_LOCK{
	volatile LONG lock;
	volatile LONG nest;
	volatile LONG sync_lock;
}  SPIN_LOCK;
#else
typedef struct SPIN_LOCK{
		volatile int32 lock;
		volatile int32 nest; // nesting level.

		volatile int32 sync_lock;
} __attribute__((aligned(64))) SPIN_LOCK;
#endif


#ifdef HERCULES_CORE
static forceinline void InitializeSpinLock(SPIN_LOCK *lck)
{
	lck->lock = 0;
	lck->nest = 0;
	lck->sync_lock = 0;
}

static forceinline void FinalizeSpinLock(SPIN_LOCK *lck)
{
		return;
}


#define getsynclock(l) do { if(InterlockedCompareExchange((l), 1, 0) == 0) break; thread->yield(); } while(/*always*/1)
#define dropsynclock(l) do { InterlockedExchange((l), 0); } while(0)

static forceinline void EnterSpinLock(SPIN_LOCK *lck)
{
	int tid = thread->get_tid();

	// Get Sync Lock && Check if the requester thread already owns the lock.
	// if it owns, increase nesting level
	getsynclock(&lck->sync_lock);
	if (InterlockedCompareExchange(&lck->lock, tid, tid) == tid) {
		InterlockedIncrement(&lck->nest);
		dropsynclock(&lck->sync_lock);
		return; // Got Lock
	}
	// drop sync lock
	dropsynclock(&lck->sync_lock);

	// Spin until we've got it !
	while (true) {
		if (InterlockedCompareExchange(&lck->lock, tid, 0) == 0) {
			InterlockedIncrement(&lck->nest);
			return; // Got Lock
		}
		thread->yield(); // Force ctxswitch to another thread.
	}

}


static forceinline void LeaveSpinLock(SPIN_LOCK *lck)
{
	int tid = thread->get_tid();

	getsynclock(&lck->sync_lock);

	if (InterlockedCompareExchange(&lck->lock, tid, tid) == tid) { // this thread owns the lock.
		if (InterlockedDecrement(&lck->nest) == 0)
			InterlockedExchange(&lck->lock, 0); // Unlock!
	}

	dropsynclock(&lck->sync_lock);
}

#endif // HERCULES_CORE

#endif /* COMMON_SPINLOCK_H */
