// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _COMMON_MAPINDEX_H_
#define _COMMON_MAPINDEX_H_

#include "../common/db.h"
#include "../common/mmo.h"

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

#define mapindex_id2name(n) mapindex->id2name((n),__FILE__, __LINE__, __func__)
#define mapindex_exists(n) ( mapindex->list[(n)].name[0] != '\0' )

/**
 * mapindex.c interface
 **/
struct mapindex_interface {
	char config_file[80];
	/* mapname (str) -> index (int) */
	DBMap *db;
	/* number of entries in the index table */
	int num;
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
	const char* (*id2name) (unsigned short,const char *file, int line, const char *func);
};

struct mapindex_interface *mapindex;

void mapindex_defaults(void);

#endif /* _COMMON_MAPINDEX_H_ */
