// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "common/cbasetypes.h"

#include <stdio.h> // FILE*

/* [HCache] 1-byte key to ensure our method is the latest, we can modify to ensure the method matches */
#define HCACHE_KEY 'k'

//Caps values to min/max
#define cap_value(a, min, max) (((a) >= (max)) ? (max) : ((a) <= (min)) ? (min) : (a))

#ifdef HERCULES_CORE
// generate a hex dump of the first 'length' bytes of 'buffer'
void WriteDump(FILE* fp, const void* buffer, size_t length);
void ShowDump(const void* buffer, size_t length);

void findfile(const char *p, const char *pat, void (func)(const char*));
bool exists(const char* filename);

/// calculates the value of A / B, in percent (rounded down)
unsigned int get_percentage(const unsigned int A, const unsigned int B);

const char* timestamp2string(char* str, size_t size, time_t timestamp, const char* format);

//////////////////////////////////////////////////////////////////////////
// byte word dword access [Shinomori]
//////////////////////////////////////////////////////////////////////////

extern uint8 GetByte(uint32 val, int idx);
extern uint16 GetWord(uint32 val, int idx);
extern uint16 MakeWord(uint8 byte0, uint8 byte1);
extern uint32 MakeDWord(uint16 word0, uint16 word1);

//////////////////////////////////////////////////////////////////////////
// Big-endian compatibility functions
//////////////////////////////////////////////////////////////////////////
extern int16 MakeShortLE(int16 val);
extern int32 MakeLongLE(int32 val);
extern uint16 GetUShort(const unsigned char* buf);
extern uint32 GetULong(const unsigned char* buf);
extern int32 GetLong(const unsigned char* buf);
extern float GetFloat(const unsigned char* buf);

size_t hread(void * ptr, size_t size, size_t count, FILE * stream);
size_t hwrite(const void * ptr, size_t size, size_t count, FILE * stream);
#endif // HERCULES_CORE

/* [Ind/Hercules] Caching */
struct HCache_interface {
	void (*init) (void);
	/* */
	bool (*check) (const char *file);
	FILE *(*open) (const char *file, const char *opt);
	/* */
	time_t recompile_time;
	bool enabled;
};

struct HCache_interface *HCache;

#ifdef HERCULES_CORE
void HCache_defaults(void);
#endif // HERCULES_CORE

#endif /* COMMON_UTILS_H */
