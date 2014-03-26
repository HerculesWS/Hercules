#pragma once
#ifndef _COMMON_SPINLOCK_H_
#define _COMMON_SPINLOCK_H_

//
// CAS based Spinlock Implementation
//
// CamelCase names are choosen to be consistent with microsofts winapi
// which implements CriticalSection by this naming...
//
// Author: Florian Wilkemeyer <fw@f-ws.de>
//
// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
//
 
#ifdef WIN32
#include "../common/winapi.h"
#endif

#include "../common/cbasetypes.h"
#include "../common/atomic.h"
#include "../common/thread.h"

#ifdef WIN32

typedef struct __declspec( align(64) ) SPIN_LOCK{
	volatile LONG lock;
	volatile LONG nest;
	volatile LONG sync_lock;
}  SPIN_LOCK, *PSPIN_LOCK;
#else
typedef struct SPIN_LOCK{
		volatile int32 lock;
		volatile int32 nest; // nesting level.
		
		volatile int32 sync_lock;
} __attribute__((aligned(64))) SPIN_LOCK, *PSPIN_LOCK;
#endif



static forceinline void InitializeSpinLock(PSPIN_LOCK lck){
		lck->lock = 0;
		lck->nest = 0;
		lck->sync_lock = 0;
}

static forceinline void FinalizeSpinLock(PSPIN_LOCK lck){
		return;
}


#define getsynclock(l) do { if(InterlockedCompareExchange((l), 1, 0) == 0) break; rathread_yield(); } while(/*always*/1)
#define dropsynclock(l) do { InterlockedExchange((l), 0); } while(0)

static forceinline void EnterSpinLock(PSPIN_LOCK lck){
		int tid = rathread_get_tid();
		
		// Get Sync Lock && Check if the requester thread already owns the lock. 
		// if it owns, increase nesting level
		getsynclock(&lck->sync_lock);
		if(InterlockedCompareExchange(&lck->lock, tid, tid) == tid){
				InterlockedIncrement(&lck->nest);
				dropsynclock(&lck->sync_lock);
				return; // Got Lock
		}
		// drop sync lock
		dropsynclock(&lck->sync_lock);
		
		
		// Spin until we've got it ! 
		while(1){
				
				if(InterlockedCompareExchange(&lck->lock, tid, 0) == 0){
						
						InterlockedIncrement(&lck->nest);
						return; // Got Lock
				}
				
				rathread_yield(); // Force ctxswitch to another thread.
		}

}


static forceinline void LeaveSpinLock(PSPIN_LOCK lck){
		int tid = rathread_get_tid();

		getsynclock(&lck->sync_lock);
		
		if(InterlockedCompareExchange(&lck->lock, tid, tid) == tid){ // this thread owns the lock.
			if(InterlockedDecrement(&lck->nest) == 0)
					InterlockedExchange(&lck->lock, 0); // Unlock!
		}
		
		dropsynclock(&lck->sync_lock);
}




#endif /* _COMMON_SPINLOCK_H_ */
