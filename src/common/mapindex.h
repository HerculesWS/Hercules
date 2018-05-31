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
#ifndef COMMON_MAPINDEX_H
#define COMMON_MAPINDEX_H

#include "common/hercules.h"
#include "common/mmo.h"

/* Forward Declarations */
struct DBMap; // common/db.h

#define MAX_MAPINDEX 2000

/* wohoo, someone look at all those |: map_default could (or *should*) be a char-server.conf */

// When a map index search fails, return results from what map? default:prontera
#define MAP_DEFAULT "prontera"
#define MAP_DEFAULT_X 150
#define MAP_DEFAULT_Y 150

//Some definitions for the mayor city maps.
#define MAP_PRONTERA "prontera"
#define MAP_GEFFEN "geffen"
#define MAP_MORROC "morocc"
#define MAP_ALBERTA "alberta"
#define MAP_PAYON "payon"
#define MAP_IZLUDE "izlude"
#define MAP_ALDEBARAN "aldebaran"
#define MAP_LUTIE "xmas"
#define MAP_COMODO "comodo"
#define MAP_YUNO "yuno"
#define MAP_AMATSU "amatsu"
#define MAP_GONRYUN "gonryun"
#define MAP_UMBALA "umbala"
#define MAP_NIFLHEIM "niflheim"
#define MAP_LOUYANG "louyang"
#define MAP_JAWAII "jawaii"
#define MAP_AYOTHAYA "ayothaya"
#define MAP_EINBROCH "einbroch"
#define MAP_LIGHTHALZEN "lighthalzen"
#define MAP_EINBECH "einbech"
#define MAP_HUGEL "hugel"
#define MAP_RACHEL "rachel"
#define MAP_VEINS "veins"
#define MAP_JAIL "sec_pri"
#define MAP_NOVICE "new_1-1"
#define MAP_MOSCOVIA "moscovia"
#define MAP_MIDCAMP "mid_camp"
#define MAP_MANUK "manuk"
#define MAP_SPLENDIDE "splendide"
#define MAP_BRASILIS "brasilis"
#define MAP_DICASTES "dicastes01"
#define MAP_MORA "mora"
#define MAP_DEWATA "dewata"
#define MAP_MALANGDO "malangdo"
#define MAP_MALAYA "malaya"
#define MAP_ECLAGE "eclage"
#define MAP_ECLAGE_IN "ecl_in01"

#define mapindex_id2name(n) mapindex->id2name((n),__FILE__, __LINE__, __func__)
#define mapindex_exists(n) ( mapindex->list[(n)].name[0] != '\0' )

/**
 * mapindex.c interface
 **/
struct mapindex_interface {
	char config_file[80];
	/* mapname (str) -> index (int) */
	struct DBMap *db;
	/* number of entries in the index table */
	int num;
	/* default map name */
	char *default_map;
	/* default x on map */
	int default_x;
	/* default y on map */
	int default_y;
	/* index list -- since map server map count is *unlimited* this should be too */
	struct {
		char name[MAP_NAME_LENGTH];
	} list[MAX_MAPINDEX];
	/* */
	int (*init) (void);
	void (*final) (void);
	/* */
	int (*addmap) (int index, const char* name);
	void (*removemap) (int index);
	const char* (*getmapname) (const char* string, char* output);
	/* TODO: server shouldn't be handling the extension, game client automatically adds .gat/.rsw/.whatever
	 * and there are official map names taking advantage of it that we cant support due to the .gat room being saved */
	const char* (*getmapname_ext) (const char* string, char* output);
	/* TODO: Hello World! make up your mind, this thing is int on some places and unsigned short on others */
	unsigned short (*name2id) (const char*);
	const char * (*id2name) (uint16 id, const char *file, int line, const char *func);
	bool (*check_default) (void);
};

#ifdef HERCULES_CORE
void mapindex_defaults(void);
#endif // HERCULES_CORE

HPShared struct mapindex_interface *mapindex;

#endif /* COMMON_MAPINDEX_H */
