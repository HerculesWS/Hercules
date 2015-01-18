// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "mapindex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"

/* mapindex.c interface source */
struct mapindex_interface mapindex_s;

/// Retrieves the map name from 'string' (removing .gat extension if present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname(const char* string, char* output) {
	static char buf[MAP_NAME_LENGTH];
	char* dest = (output != NULL) ? output : buf;

	size_t len = strnlen(string, MAP_NAME_LENGTH_EXT);
	if (len == MAP_NAME_LENGTH_EXT) {
		ShowWarning("(mapindex_normalize_name) Map name '%*s' is too long!\n", 2*MAP_NAME_LENGTH_EXT, string);
		len--;
	}
	if (len >= 4 && stricmp(&string[len-4], ".gat") == 0)
		len -= 4; // strip .gat extension

	len = min(len, MAP_NAME_LENGTH-1);
	safestrncpy(dest, string, len+1);
	memset(&dest[len], '\0', MAP_NAME_LENGTH-len);

	return dest;
}

/// Retrieves the map name from 'string' (adding .gat extension if not already present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname_ext(const char* string, char* output) {
	static char buf[MAP_NAME_LENGTH_EXT];
	char* dest = (output != NULL) ? output : buf;

	size_t len;

	safestrncpy(buf,string, sizeof(buf));
	sscanf(string, "%*[^#]%*[#]%15s", buf);

	len = safestrnlen(buf, MAP_NAME_LENGTH);

	if (len == MAP_NAME_LENGTH) {
		ShowWarning("(mapindex_normalize_name) Map name '%s' is too long!\n", buf);
		len--;
	}
	safestrncpy(dest, buf, len+1);

	if (len < 4 || stricmp(&dest[len-4], ".gat") != 0) {
		strcpy(&dest[len], ".gat");
		len += 4; // add .gat extension
	}

	memset(&dest[len], '\0', MAP_NAME_LENGTH_EXT-len);

	return dest;
}

/// Adds a map to the specified index
/// Returns 1 if successful, 0 otherwise
int mapindex_addmap(int index, const char* name) {
	char map_name[MAP_NAME_LENGTH];

	if (index == -1){
		for (index = 1; index < mapindex->num; index++) {
			if (mapindex->list[index].name[0] == '\0')
				break;
		}
	}

	if (index < 0 || index >= MAX_MAPINDEX) {
		ShowError("(mapindex_add) Map index (%d) for \"%s\" out of range (max is %d)\n", index, name, MAX_MAPINDEX);
		return 0;
	}

	mapindex->getmapname(name, map_name);

	if (map_name[0] == '\0') {
		ShowError("(mapindex_add) Cannot add maps with no name.\n");
		return 0;
	}

	if (strlen(map_name) >= MAP_NAME_LENGTH) {
		ShowError("(mapindex_add) Map name %s is too long. Maps are limited to %d characters.\n", map_name, MAP_NAME_LENGTH);
		return 0;
	}

	if (mapindex_exists(index)) {
		ShowWarning("(mapindex_add) Overriding index %d: map \"%s\" -> \"%s\"\n", index, mapindex->list[index].name, map_name);
		strdb_remove(mapindex->db, mapindex->list[index].name);
	}

	safestrncpy(mapindex->list[index].name, map_name, MAP_NAME_LENGTH);
	strdb_iput(mapindex->db, map_name, index);

	if (mapindex->num <= index)
		mapindex->num = index+1;

	return index;
}

unsigned short mapindex_name2id(const char* name) {
	int i;
	char map_name[MAP_NAME_LENGTH];

	mapindex->getmapname(name, map_name);

	if( (i = strdb_iget(mapindex->db, map_name)) )
		return i;

	ShowDebug("mapindex_name2id: Map \"%s\" not found in index list!\n", map_name);
	return 0;
}

const char* mapindex_id2name_sub(unsigned short id,const char *file, int line, const char *func) {
	if (id >= MAX_MAPINDEX || !mapindex_exists(id)) {
		ShowDebug("mapindex_id2name: Requested name for non-existant map index [%d] in cache. %s:%s:%d\n", id,file,func,line);
		return mapindex->list[0].name; // dummy empty string so that the callee doesn't crash
	}
	return mapindex->list[id].name;
}

int mapindex_init(void) {
	FILE *fp;
	char line[1024];
	int last_index = -1;
	int index, total = 0;
	char map_name[13];

	if( ( fp = fopen(mapindex->config_file,"r") ) == NULL ){
		ShowFatalError("Unable to read mapindex config file %s!\n", mapindex->config_file);
		exit(EXIT_FAILURE); //Server can't really run without this file.
	}

	mapindex->db = strdb_alloc(DB_OPT_DUP_KEY, MAP_NAME_LENGTH);

	while(fgets(line, sizeof(line), fp)) {
		if(line[0] == '/' && line[1] == '/')
			continue;

		switch (sscanf(line, "%12s\t%d", map_name, &index)) {
			case 1: //Map with no ID given, auto-assign
				index = last_index+1;
				/* Fall through */
			case 2: //Map with ID given
				mapindex->addmap(index,map_name);
				total++;
				break;
			default:
				continue;
		}
		last_index = index;
	}
	fclose(fp);

	mapindex->check_default();

	return total;
}

bool mapindex_check_default(void)
{
	if (!strdb_iget(mapindex->db, mapindex->default_map)) {
		ShowError("mapindex_init: MAP_DEFAULT '%s' not found in cache! update mapindex.h MAP_DEFAULT var!!!\n", mapindex->default_map);
		return false;
	}
	return true;
}

void mapindex_removemap(int index){
	strdb_remove(mapindex->db, mapindex->list[index].name);
	mapindex->list[index].name[0] = '\0';
}

void mapindex_final(void) {
	db_destroy(mapindex->db);
}

void mapindex_defaults(void) {
	mapindex = &mapindex_s;

	/* TODO: place it in inter-server.conf? */
	snprintf(mapindex->config_file, 80, "%s","db/map_index.txt");
	/* */
	mapindex->db = NULL;
	mapindex->num = 0;
	mapindex->default_map = MAP_DEFAULT;
	mapindex->default_x = MAP_DEFAULT_X;
	mapindex->default_y = MAP_DEFAULT_Y;
	memset (&mapindex->list, 0, sizeof (mapindex->list));

	/* */
	mapindex->init = mapindex_init;
	mapindex->final = mapindex_final;
	/* */
	mapindex->addmap = mapindex_addmap;
	mapindex->removemap = mapindex_removemap;
	mapindex->getmapname = mapindex_getmapname;
	mapindex->getmapname_ext = mapindex_getmapname_ext;
	mapindex->name2id = mapindex_name2id;
	mapindex->id2name = mapindex_id2name_sub;
	mapindex->check_default = mapindex_check_default;
}
