/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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

#include "utils.h"

#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"

#ifdef WIN32
#	include "common/winapi.h"
#	ifndef F_OK
#		define F_OK   0x0
#	endif  /* F_OK */
#else
#	include <dirent.h>
#	include <unistd.h>
#endif

#include <math.h> // floor()
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // cache purposes [Ind/Hercules]

struct HCache_interface HCache_s;
struct HCache_interface *HCache;

/// Dumps given buffer into file pointed to by a handle.
void WriteDump(FILE* fp, const void* buffer, size_t length)
{
	size_t i;
	char hex[48+1], ascii[16+1];

	fprintf(fp, "--- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F   0123456789ABCDEF\n");
	ascii[16] = 0;

	for( i = 0; i < length; i++ )
	{
		char c = RBUFB(buffer,i);

		ascii[i%16] = ISCNTRL(c) ? '.' : c;
		sprintf(hex+(i%16)*3, "%02X ", RBUFB(buffer,i));

		if( (i%16) == 15 )
		{
			fprintf(fp, "%03X %s  %s\n", (unsigned int)(i/16), hex, ascii);
		}
	}

	if( (i%16) != 0 )
	{
		ascii[i%16] = 0;
		fprintf(fp, "%03X %-48s  %-16s\n", (unsigned int)(i/16), hex, ascii);
	}
}

/// Dumps given buffer on the console.
void ShowDump(const void *buffer, size_t length) {
	size_t i;
	char hex[48+1], ascii[16+1];

	ShowDebug("--- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F   0123456789ABCDEF\n");
	ascii[16] = 0;

	for (i = 0; i < length; i++) {
		char c = RBUFB(buffer,i);

		ascii[i%16] = ISCNTRL(c) ? '.' : c;
		sprintf(hex+(i%16)*3, "%02X ", RBUFB(buffer,i));

		if ((i%16) == 15) {
			ShowDebug("%03"PRIXS" %s  %s\n", i/16, hex, ascii);
		}
	}

	if ((i%16) != 0) {
		ascii[i%16] = 0;
		ShowDebug("%03"PRIXS" %-48s  %-16s\n", i/16, hex, ascii);
	}
}

#ifdef WIN32

static char* checkpath(char *path, const char *srcpath)
{
	// just make sure the char*path is not const
	char *p = path;
	if (NULL == path || NULL == srcpath)
		return path;
	while(*srcpath) {
		if (*srcpath=='/') {
			*p++ = '\\';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;

	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");

	hFind = FindFirstFileA(tmppath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if (strcmp(FindFileData.cFileName, "..") == 0)
				continue;

			sprintf(tmppath,"%s%c%s",path,PATHSEP,FindFileData.cFileName);

			if (strstr(FindFileData.cFileName, pattern)) {
				func( tmppath );
			}

			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				findfile(tmppath, pat, func);
			}
		}while (FindNextFileA(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}
#else

#define MAX_DIR_PATH 2048

static char* checkpath(char *path, const char*srcpath)
{
	// just make sure the char*path is not const
	char *p=path;

	if(NULL!=path && NULL!=srcpath) {
		while(*srcpath) {
			if (*srcpath=='\\') {
				*p++ = '/';
				srcpath++;
			}
			else
				*p++ = *srcpath++;
		}
		*p = *srcpath; //EOS
	}
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{
	DIR* dir;             ///< pointer to the scanned directory.
	struct dirent* entry; ///< pointer to one directory entry.
	struct stat dir_stat; ///< used by stat().
	char tmppath[MAX_DIR_PATH+1];
	char path[MAX_DIR_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL) safestrncpy(path,p,sizeof(path));

	// open the directory for reading
	dir = opendir( checkpath(path, path) );
	if (!dir) {
		ShowError("Cannot read directory '%s'\n", path);
		return;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while ((entry = readdir(dir))) {
		// skip the "." and ".." entries.
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		sprintf(tmppath,"%s%c%s",path, PATHSEP, entry->d_name);

		// check if the pattern matches.
		if (strstr(entry->d_name, pattern)) {
			func( tmppath );
		}
		// check if it is a directory.
		if (stat(tmppath, &dir_stat) == -1) {
			ShowError("stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if (S_ISDIR(dir_stat.st_mode)) {
			// decent recursively
			findfile(tmppath, pat, func);
		}
	}//end while

	closedir(dir);
}
#endif

bool exists(const char* filename)
{
	return !access(filename, F_OK);
}

uint8 GetByte(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint8)( (val & 0x000000FF)         );
	case 1: return (uint8)( (val & 0x0000FF00) >> 0x08 );
	case 2: return (uint8)( (val & 0x00FF0000) >> 0x10 );
	case 3: return (uint8)( (val & 0xFF000000) >> 0x18 );
	default:
#if defined(DEBUG)
		ShowDebug("GetByte: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}

uint16 GetWord(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint16)( (val & 0x0000FFFF)         );
	case 1: return (uint16)( (val & 0xFFFF0000) >> 0x10 );
	default:
#if defined(DEBUG)
		ShowDebug("GetWord: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}
uint16 MakeWord(uint8 byte0, uint8 byte1)
{
	return byte0 | (byte1 << 0x08);
}

uint32 MakeDWord(uint16 word0, uint16 word1)
{
	return
		( (uint32)(word0        ) )|
		( (uint32)(word1 << 0x10) );
}

/*************************************
* Big-endian compatibility functions *
*************************************/

// Converts an int16 from current machine order to little-endian
int16 MakeShortLE(int16 val)
{
	unsigned char buf[2];
	buf[0] = (unsigned char)( (val & 0x00FF)         );
	buf[1] = (unsigned char)( (val & 0xFF00) >> 0x08 );
	return *((int16*)buf);
}

// Converts an int32 from current machine order to little-endian
int32 MakeLongLE(int32 val)
{
	unsigned char buf[4];
	buf[0] = (unsigned char)( (val & 0x000000FF)         );
	buf[1] = (unsigned char)( (val & 0x0000FF00) >> 0x08 );
	buf[2] = (unsigned char)( (val & 0x00FF0000) >> 0x10 );
	buf[3] = (unsigned char)( (val & 0xFF000000) >> 0x18 );
	return *((int32*)buf);
}

// Reads an uint16 in little-endian from the buffer
uint16 GetUShort(const unsigned char* buf)
{
	return ( ((uint16)(buf[0]))         )
	     | ( ((uint16)(buf[1])) << 0x08 );
}

// Reads an uint32 in little-endian from the buffer
uint32 GetULong(const unsigned char* buf)
{
	return ( ((uint32)(buf[0]))         )
	     | ( ((uint32)(buf[1])) << 0x08 )
	     | ( ((uint32)(buf[2])) << 0x10 )
	     | ( ((uint32)(buf[3])) << 0x18 );
}

// Reads an int32 in little-endian from the buffer
int32 GetLong(const unsigned char* buf)
{
	return (int32)GetULong(buf);
}

// Reads a float (32 bits) from the buffer
float GetFloat(const unsigned char* buf)
{
	uint32 val = GetULong(buf);
	return *((float*)(void*)&val);
}

/// calculates the value of A / B, in percent (rounded down)
unsigned int get_percentage(const unsigned int A, const unsigned int B)
{
	double result;

	if( B == 0 )
	{
		ShowError("get_percentage(): division by zero! (A=%u,B=%u)\n", A, B);
		return ~0U;
	}

	result = 100 * ((double)A / (double)B);

	if( result > UINT_MAX )
	{
		ShowError("get_percentage(): result percentage too high! (A=%u,B=%u,result=%g)\n", A, B, result);
		return UINT_MAX;
	}

	return (unsigned int)floor(result);
}

/**
 * Applies a percentual rate modifier.
 *
 * @param value The base value.
 * @param rate  The rate modifier to apply.
 * @param stdrate The rate modifier's divider (rate == stdrate => 100%).
 * @return The modified value.
 */
int64 apply_percentrate64(int64 value, int rate, int stdrate)
{
	Assert_ret(stdrate > 0);
	Assert_ret(rate >= 0);
	if (rate == stdrate)
		return value;
	if (rate == 0)
		return 0;
	if (INT64_MAX / rate < value) {
		// Give up some precision to prevent overflows
		return value / stdrate * rate;
	}
	return value * rate / stdrate;
}

/**
 * Applies a percentual rate modifier.
 *
 * @param value The base value.
 * @param rate  The rate modifier to apply. Must be <= maxrate.
 * @param maxrate The rate modifier's divider (maxrate = 100%).
 * @return The modified value.
 */
int apply_percentrate(int value, int rate, int maxrate)
{
	Assert_ret(maxrate > 0);
	Assert_ret(rate >= 0);
	if (rate == maxrate)
		return value;
	if (rate == 0)
		return 0;
	return (int)(value * (int64)rate / maxrate);
}

//-----------------------------------------------------
// custom timestamp formatting (from eApp)
//-----------------------------------------------------
const char* timestamp2string(char* str, size_t size, time_t timestamp, const char* format)
{
	size_t len = strftime(str, size, format, localtime(&timestamp));
	memset(str + len, '\0', size - len);
	return str;
}

/* [Ind/Hercules] Caching */
bool HCache_check(const char *file)
{
	struct stat bufa, bufb;
	FILE *first, *second;
	char s_path[255], dT[1];
	time_t rtime;

	if (!(first = fopen(file,"rb")))
		return false;

	if (file[0] == '.' && file[1] == '/')
		file += 2;
	else if (file[0] == '.')
		file++;

	snprintf(s_path, 255, "./cache/%s", file);

	if (!(second = fopen(s_path,"rb"))) {
		fclose(first);
		return false;
	}

	if (fread(dT,sizeof(dT),1,second) != 1
	 || fread(&rtime,sizeof(rtime),1,second) != 1
	 || dT[0] != HCACHE_KEY
	 || HCache->recompile_time > rtime) {
		fclose(first);
		fclose(second);
		return false;
	}

	if (fstat(fileno(first), &bufa) != 0) {
		fclose(first);
		fclose(second);
		return false;
	}
	fclose(first);

	if (fstat(fileno(second), &bufb) != 0) {
		fclose(second);
		return false;
	}
	fclose(second);

	if (bufa.st_mtime > bufb.st_mtime)
		return false;

	return true;
}

FILE *HCache_open(const char *file, const char *opt) {
	FILE *first;
	char s_path[255];

	if( file[0] == '.' && file[1] == '/' )
		file += 2;
	else if( file[0] == '.' )
		file++;

	snprintf(s_path, 255, "./cache/%s", file);

	if( !(first = fopen(s_path,opt)) ) {
		return NULL;
	}

	if( opt[0] != 'r' ) {
		char dT[1];/* 1-byte key to ensure our method is the latest, we can modify to ensure the method matches */
		dT[0] = HCACHE_KEY;
		hwrite(dT,sizeof(dT),1,first);
		hwrite(&HCache->recompile_time,sizeof(HCache->recompile_time),1,first);
	}
	if (fseek(first, 20, SEEK_SET) != 0) { // skip first 20, might wanna store something else later
		fclose(first);
		return NULL;
	}

	return first;
}

void HCache_init(void)
{
	struct stat buf;
	if (stat(SERVER_NAME, &buf) != 0) {
		ShowWarning("Unable to open '%s', caching capabilities have been disabled!\n",SERVER_NAME);
		return;
	}

	HCache->recompile_time = buf.st_mtime;
	HCache->enabled = true;
}

/* transit to fread, shields vs warn_unused_result */
size_t hread(void * ptr, size_t size, size_t count, FILE * stream) {
	return fread(ptr, size, count, stream);
}
/* transit to fwrite, shields vs warn_unused_result */
size_t hwrite(const void * ptr, size_t size, size_t count, FILE * stream) {
	return fwrite(ptr, size, count, stream);
}

void HCache_defaults(void) {
	HCache = &HCache_s;

	HCache->init = HCache_init;

	HCache->check = HCache_check;
	HCache->open = HCache_open;
	HCache->recompile_time = 0;
	HCache->enabled = false;
}
