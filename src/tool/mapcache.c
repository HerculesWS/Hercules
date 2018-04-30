/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
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

#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#define NO_WATER 1000000

char *grf_list_file;
char *map_list_file;
char *map_cache_file;
int rebuild = 0;

FILE *map_cache_fp;

unsigned long file_size;

// Used internally, this structure contains the physical map cells
struct map_data {
	int16 xs;
	int16 ys;
	unsigned char *cells;
};

// This is the main header found at the very beginning of the file
struct main_header {
	uint32 file_size;
	uint16 map_count;
} header;

// This is the header appended before every compressed map cells info
struct map_info {
	char name[MAP_NAME_LENGTH];
	int16 xs;
	int16 ys;
	int32 len;
};

// Reads a map from GRF's GAT and RSW files
int read_map(char *name, struct map_data *m)
{
	char filename[256];
	unsigned char *gat, *rsw;
	int water_height;
	size_t xy, off, num_cells;

	// Open map GAT
	sprintf(filename,"data\\%s.gat", name);
	gat = grfio_read(filename);
	if (gat == NULL)
		return 0;

	// Open map RSW
	sprintf(filename,"data\\%s.rsw", name);
	rsw = grfio_read(filename);

	// Read water height
	if (rsw) {
		water_height = (int)GetFloat(rsw+166);
		aFree(rsw);
	} else
		water_height = NO_WATER;

	// Read map size and allocate needed memory
	m->xs = (int16)GetULong(gat+6);
	m->ys = (int16)GetULong(gat+10);
	if (m->xs <= 0 || m->ys <= 0) {
		aFree(gat);
		return 0;
	}
	num_cells = (size_t)m->xs*(size_t)m->ys;
	m->cells = (unsigned char *)aMalloc(num_cells);

	// Set cell properties
	off = 14;
	for (xy = 0; xy < num_cells; xy++) {
		// Height of the bottom-left corner
		float height = GetFloat(gat + off);
		// Type of cell
		uint32 type = GetULong(gat + off + 16);
		off += 20;

		if (type == 0 && water_height != NO_WATER && height > water_height)
			type = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)

		m->cells[xy] = (unsigned char)type;
	}

	aFree(gat);

	return 1;
}

/**
 * Adds a map to the cache.
 *
 * @param name The map name.
 * @param m    Map data to cache.
 * @retval true if the map was successfully added to the cache.
 */
bool cache_map(char *name, struct map_data *m)
{
	struct map_info info;
	unsigned long len;
	unsigned char *write_buf;

	// Create an output buffer twice as big as the uncompressed map... this way we're sure it fits
	len = (unsigned long)m->xs*(unsigned long)m->ys*2;
	write_buf = (unsigned char *)aMalloc(len);
	// Compress the cells and get the compressed length
	grfio->encode_zip(write_buf, &len, m->cells, m->xs*m->ys);

	// Fill the map header
	safestrncpy(info.name, name, MAP_NAME_LENGTH);
	if (strlen(name) > MAP_NAME_LENGTH) // It does not hurt to warn that there are maps with name longer than allowed.
		ShowWarning("Map name '%s' (length %"PRIuS") is too long. Truncating to '%s' (length %d).\n",
		            name, strlen(name), info.name, MAP_NAME_LENGTH);
	info.xs = MakeShortLE(m->xs);
	info.ys = MakeShortLE(m->ys);
	info.len = MakeLongLE((uint32)len);

	// Append map header then compressed cells at the end of the file
	if (fseek(map_cache_fp, header.file_size, SEEK_SET) != 0) {
		aFree(write_buf);
		aFree(m->cells);
		return false;
	}
	fwrite(&info, sizeof(struct map_info), 1, map_cache_fp);
	fwrite(write_buf, 1, len, map_cache_fp);
	header.file_size += sizeof(struct map_info) + len;
	header.map_count++;

	aFree(write_buf);
	aFree(m->cells);

	return true;
}

/**
 * Checks whether a map is already is the cache.
 *
 * @param name The map name.
 * @retval true if the map is already cached.
 */
bool find_map(char *name)
{
	int i;
	struct map_info info;

	if (fseek(map_cache_fp, sizeof(struct main_header), SEEK_SET) != 0)
		return false;

	for (i = 0; i < header.map_count; i++) {
		if (fread(&info, sizeof(info), 1, map_cache_fp) != 1)
			printf("An error as occured in fread while reading map_cache\n");
		if (strcmp(name, info.name) == 0) // Map found
			return true;
		// Map not found, jump to the beginning of the next map info header
		if (fseek(map_cache_fp, GetLong((unsigned char *)&(info.len)), SEEK_CUR) != 0)
			return false;
	}

	return false;
}

// Cuts the extension from a map name
char *remove_extension(char *mapname)
{
	char *ptr, *ptr2;
	ptr = strchr(mapname, '.');
	if (ptr) { //Check and remove extension.
		while (ptr[1] && (ptr2 = strchr(ptr+1, '.')) != NULL)
			ptr = ptr2; //Skip to the last dot.
		if (strcmp(ptr,".gat") == 0)
			*ptr = '\0'; //Remove extension.
	}
	return mapname;
}

/**
 * --grf-list handler
 *
 * Overrides the default grf list filename.
 * @see cmdline->exec
 */
static CMDLINEARG(grflist)
{
	aFree(grf_list_file);
	grf_list_file = aStrdup(params);
	return true;
}

/**
 * --map-list handler
 *
 * Overrides the default map list filename.
 * @see cmdline->exec
 */
static CMDLINEARG(maplist)
{
	aFree(map_list_file);
	map_list_file = aStrdup(params);
	return true;
}

/**
 * --map-cache handler
 *
 * Overrides the default map cache filename.
 * @see cmdline->exec
 */
static CMDLINEARG(mapcache)
{
	aFree(map_cache_file);
	map_cache_file = aStrdup(params);
	return true;
}

/**
 * --rebuild handler
 *
 * Forces a rebuild of the mapcache, rather than only adding missing maps.
 * @see cmdline->exec
 */
static CMDLINEARG(rebuild)
{
	rebuild = 1;
	return true;
}

/**
 * Defines the local command line arguments
 */
void cmdline_args_init_local(void)
{
	CMDLINEARG_DEF2(grf-list, grflist, "Alternative grf list file", CMDLINE_OPT_NORMAL|CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(map-list, maplist, "Alternative map list file", CMDLINE_OPT_NORMAL|CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(map-cache, mapcache, "Alternative map cache file", CMDLINE_OPT_NORMAL|CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(rebuild, rebuild, "Forces a rebuild of the map cache, rather than only adding missing maps", CMDLINE_OPT_NORMAL);
}

int do_init(int argc, char** argv)
{
	FILE *list;
	char line[1024];
	struct map_data map;
	char name[MAP_NAME_LENGTH_EXT];

	grf_list_file = aStrdup("conf/grf-files.txt");
	map_list_file = aStrdup("db/map_index.txt");
	/* setup pre-defined, #define-dependant */
	map_cache_file = aStrdup("db/"DBPATH"map_cache.dat");

	cmdline->exec(argc, argv, CMDLINE_OPT_PREINIT);
	cmdline->exec(argc, argv, CMDLINE_OPT_NORMAL);

	ShowStatus("Initializing grfio with %s\n", grf_list_file);
	grfio->init(grf_list_file);

	// Attempt to open the map cache file and force rebuild if not found
	ShowStatus("Opening map cache: %s\n", map_cache_file);
	if(!rebuild) {
		map_cache_fp = fopen(map_cache_file, "rb");
		if(map_cache_fp == NULL) {
			ShowNotice("Existing map cache not found, forcing rebuild mode\n");
			rebuild = 1;
		} else
			fclose(map_cache_fp);
	}
	if(rebuild)
		map_cache_fp = fopen(map_cache_file, "w+b");
	else
		map_cache_fp = fopen(map_cache_file, "r+b");
	if(map_cache_fp == NULL) {
		ShowError("Failure when opening map cache file %s\n", map_cache_file);
		exit(EXIT_FAILURE);
	}

	// Open the map list
	ShowStatus("Opening map list: %s\n", map_list_file);
	list = fopen(map_list_file, "r");
	if(list == NULL) {
		ShowError("Failure when opening maps list file %s\n", map_list_file);
		exit(EXIT_FAILURE);
	}

	// Initialize the main header
	if(rebuild) {
		header.file_size = sizeof(struct main_header);
		header.map_count = 0;
	} else {
		if(fread(&header, sizeof(struct main_header), 1, map_cache_fp) != 1){ printf("An error as occured while reading map_cache_fp \n"); }
		header.file_size = GetULong((unsigned char *)&(header.file_size));
		header.map_count = GetUShort((unsigned char *)&(header.map_count));
	}

	// Read and process the map list
	while(fgets(line, sizeof(line), list))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		if(sscanf(line, "%15s", name) < 1)
			continue;

		if(strcmp("map:", name) == 0 && sscanf(line, "%*s %15s", name) < 1)
			continue;

		name[MAP_NAME_LENGTH_EXT-1] = '\0';
		remove_extension(name);
		if (find_map(name)) {
			ShowInfo("Map '"CL_WHITE"%s"CL_RESET"' already in cache.\n", name);
		} else if(!read_map(name, &map)) {
			ShowError("Map '"CL_WHITE"%s"CL_RESET"' not found!\n", name);
		} else if (!cache_map(name, &map)) {
			ShowError("Map '"CL_WHITE"%s"CL_RESET"' failed to cache (write error).\n", name);
		} else {
			ShowInfo("Map '"CL_WHITE"%s"CL_RESET"' successfully cached.\n", name);
		}
	}

	ShowStatus("Closing map list: %s\n", map_list_file);
	fclose(list);

	// Write the main header and close the map cache
	ShowStatus("Closing map cache: %s\n", map_cache_file);
	fseek(map_cache_fp, 0, SEEK_SET);
	fwrite(&header, sizeof(struct main_header), 1, map_cache_fp);
	fclose(map_cache_fp);

	ShowStatus("Finalizing grfio\n");
	grfio->final();

	ShowInfo("%d maps now in cache\n", header.map_count);

	aFree(grf_list_file);
	aFree(map_list_file);
	aFree(map_cache_file);

	return 0;
}

int do_final(void)
{
	return EXIT_SUCCESS;
}
