// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAPINDEX_H_
#define _MAPINDEX_H_

#include "../common/db.h"

// When a map index search fails, return results from what map? default:prontera
#define MAP_DEFAULT "prontera"
#define MAP_DEFAULT_X 150
#define MAP_DEFAULT_Y 150
DBMap *mapindex_db;

//File in charge of assigning a numberic ID to each map in existance for space saving when passing map info between servers.
extern char mapindex_cfgfile[80];

#define MAX_MAPINDEX 2000

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

const char* mapindex_getmapname(const char* string, char* output);
const char* mapindex_getmapname_ext(const char* string, char* output);
unsigned short mapindex_name2id(const char*);
#define mapindex_id2name(n) mapindex_id2name_sub(n,__FILE__, __LINE__, __func__)
const char* mapindex_id2name_sub(unsigned short,const char *file, int line, const char *func);
void mapindex_init(void);
void mapindex_final(void);

int mapindex_addmap(int index, const char* name);
int mapindex_removemap(int index);

#endif /* _MAPINDEX_H_ */
