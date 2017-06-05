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
#ifndef COMMON_ATOMIC_H
#define COMMON_ATOMIC_H

// Atomic Operations
// (Interlocked CompareExchange, Add .. and so on ..)
//
// Implementation varies / depends on:
// - Architecture
// - Compiler
// - Operating System
//
// our Abstraction is fully API-Compatible to Microsoft's implementation @ NT5.0+
//
#include "common/cbasetypes.h"

#if defined(_MSC_VER)
#include "common/winapi.h"

// This checks if C/C++ Compiler Version is 18.00
#if _MSC_VER < 1800

#if !defined(_M_X64)
// When compiling for windows 32bit, the 8byte interlocked operations are not provided by Microsoft
// (because they need at least i586 so its not generic enough.. ... )
forceinline int64 InterlockedCompareExchange64(volatile int64 *dest, int64 exch, int64 _cmp){
	_asm{
		lea esi,_cmp;
		lea edi,exch;

		mov eax,[esi];
		mov edx,4[esi];
		mov ebx,[edi];
		mov ecx,4[edi];
		mov esi,dest;

		lock CMPXCHG8B [esi];
	}
}

forceinline volatile int64 InterlockedIncrement64(volatile int64 *addend){
	__int64 old;
	do{
		old = *addend;
	}while(InterlockedCompareExchange64(addend, (old+1), old) != old);

	return (old + 1);
}

forceinline volatile int64 InterlockedDecrement64(volatile int64 *addend){
	__int64 old;

	do{
		old = *addend;
	}while(InterlockedCompareExchange64(addend, (old-1), old) != old);

	return (old - 1);
}

forceinline volatile int64 InterlockedExchangeAdd64(volatile int64 *addend, int64 increment){
	__int64 old;

	do{
		old = *addend;
	}while(InterlockedCompareExchange64(addend, (old + increment), old) != old);

	return old;
}

forceinline volatile int64 InterlockedExchange64(volatile int64 *target, int64 val){
	__int64 old;
	do{
		old = *target;
	}while(InterlockedCompareExchange64(target, val, old) != old);

	return old;
}

#endif //endif 32bit windows

#endif //endif _msc_ver check

#elif defined(__GNUC__)

// The __sync functions are available on x86 or ARMv6+
#if !defined(__x86_64__) && !defined(__i386__) \
	&& ( !defined(__ARM_ARCH_VERSION__) || __ARM_ARCH_VERSION__ < 6 )
#error Your Target Platfrom is not supported
#endif

static forceinline int64 InterlockedExchangeAdd64(volatile int64 *addend, int64 increment){
	return __sync_fetch_and_add(addend, increment);
}//end: InterlockedExchangeAdd64()

static forceinline int32 InterlockedExchangeAdd(volatile int32 *addend, int32 increment){
	return __sync_fetch_and_add(addend, increment);
}//end: InterlockedExchangeAdd()

static forceinline int64 InterlockedIncrement64(volatile int64 *addend){
	return __sync_add_and_fetch(addend, 1);
}//end: InterlockedIncrement64()

static forceinline int32 InterlockedIncrement(volatile int32 *addend){
	return __sync_add_and_fetch(addend, 1);
}//end: InterlockedIncrement()

static forceinline int64 InterlockedDecrement64(volatile int64 *addend){
	return __sync_sub_and_fetch(addend, 1);
}//end: InterlockedDecrement64()

static forceinline int32 InterlockedDecrement(volatile int32 *addend){
	return __sync_sub_and_fetch(addend, 1);
}//end: InterlockedDecrement()

static forceinline int64 InterlockedCompareExchange64(volatile int64 *dest, int64 exch, int64 cmp){
	return __sync_val_compare_and_swap(dest, cmp, exch);
}//end: InterlockedCompareExchange64()

static forceinline int32 InterlockedCompareExchange(volatile int32 *dest, int32 exch, int32 cmp){
	return __sync_val_compare_and_swap(dest, cmp, exch);
}//end: InterlockedCompareExchnage()

static forceinline int64 InterlockedExchange64(volatile int64 *target, int64 val){
	return __sync_lock_test_and_set(target, val);
}//end: InterlockedExchange64()

static forceinline int32 InterlockedExchange(volatile int32 *target, int32 val){
	return __sync_lock_test_and_set(target, val);
}//end: InterlockedExchange()

#endif //endif compiler decision

#endif /* COMMON_ATOMIC_H */
