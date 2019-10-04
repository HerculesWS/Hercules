/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
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

#include "mapindex.h"

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdlib.h>

/* mapindex.c interface source */
static struct mapindex_interface mapindex_s;
struct mapindex_interface *mapindex;

/// Retrieves the map name from 'string' (removing .gat extension if present).
/// Result gets placed either into 'buf' or in a static local buffer.
static const char *mapindex_getmapname(const char *string, char *output)
{
	static char buf[MAP_NAME_LENGTH];
	char* dest = (output != NULL) ? output : buf;

	size_t len;
	nullpo_retr(buf, string);
	len = strnlen(string, MAP_NAME_LENGTH_EXT);
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
static const char *mapindex_getmapname_ext(const char *string, char *output)
{
	static char buf[MAP_NAME_LENGTH_EXT];
	char* dest = (output != NULL) ? output : buf;

	size_t len;

	nullpo_retr(buf, string);

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
static int mapindex_addmap(int index, const char *name)
{
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

static unsigned short mapindex_name2id(const char *name)
{
	int i;
	char map_name[MAP_NAME_LENGTH];

	mapindex->getmapname(name, map_name);

	if( (i = strdb_iget(mapindex->db, map_name)) )
		return i;

	ShowDebug("mapindex_name2id: Map \"%s\" not found in index list!\n", map_name);
	return 0;
}

static const char *mapindex_id2name_sub(uint16 id, const char *file, int line, const char *func)
{
	if (id >= MAX_MAPINDEX || !mapindex_exists(id)) {
		ShowDebug("mapindex_id2name: Requested name for non-existant map index [%d] in cache. %s:%s:%d\n", id,file,func,line);
		return mapindex->list[0].name; // dummy empty string so that the callee doesn't crash
	}
	return mapindex->list[id].name;
}

/**
 * Reads the db_path config of mapindex configuration file
 * @param filename File being read (used when displaying errors)
 * @param config Config structure being read
 * @returns true if it read the all the configs, false otherwise
 */
static bool mapindex_config_read_dbpath(const char *filename, const struct config_t *config)
{
	nullpo_retr(false, config);

	const struct config_setting_t *setting = NULL;

	if ((setting = libconfig->lookup(config, "mapindex_configuration")) == NULL) {
		ShowError("mapindex_config_read: mapindex_configuration was not found in %s!\n", filename);
		return false;
	}

	// mapindex_configuration/file_path
	if (libconfig->setting_lookup_mutable_string(setting, "file_path", mapindex->config_file, sizeof(mapindex->config_file)) == CONFIG_TRUE) {
		ShowInfo("map_index file %s\n", mapindex->config_file);
	} else {
		ShowInfo("Failed to load map_index path, defaulting to db/map_index.txt\n");
		safestrncpy(mapindex->config_file, "db/map_index.txt", sizeof(mapindex->config_file));
	}

	return true;
}

/**
 * Reads conf/common/map-index.conf config file
 * @returns true if it successfully read the file and configs, false otherwise
 */
static bool mapindex_config_read(void)
{
	struct config_t config;
	const char *filename = "conf/common/map-index.conf";
	
	if (!libconfig->load_file(&config, filename))
		return false; // Error message is already shown by libconfig->load_file

	if (!mapindex_config_read_dbpath(filename, &config))
		return false;

	ShowInfo("Done reading %s.\n", filename);
	return true;
}

static int mapindex_init(void)
{
	if (!mapindex_config_read())
		ShowError("Failed to load map_index configuration. Continuing with default values...\n");

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

static bool mapindex_check_default(void)
{
	if (!strdb_iget(mapindex->db, mapindex->default_map)) {
		ShowError("mapindex_init: MAP_DEFAULT '%s' not found in cache! update mapindex.h MAP_DEFAULT var!!!\n", mapindex->default_map);
		return false;
	}
	return true;
}

static void mapindex_removemap(int index)
{
	Assert_retv(index < MAX_MAPINDEX);
	strdb_remove(mapindex->db, mapindex->list[index].name);
	mapindex->list[index].name[0] = '\0';
}

static void mapindex_final(void)
{
	db_destroy(mapindex->db);
}

void mapindex_defaults(void)
{
	mapindex = &mapindex_s;

	/* TODO: place it in inter-server.conf? */
	snprintf(mapindex->config_file, sizeof(mapindex->config_file), "%s","db/map_index.txt");
	/* */
	mapindex->db = NULL;
	mapindex->num = 0;
	mapindex->default_map = MAP_DEFAULT;
	mapindex->default_x = MAP_DEFAULT_X;
	mapindex->default_y = MAP_DEFAULT_Y;
	memset (&mapindex->list, 0, sizeof (mapindex->list));

	/* */
	mapindex->config_read = mapindex_config_read;
	mapindex->config_read_dbpath = mapindex_config_read_dbpath;
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
