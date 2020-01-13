/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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
#ifndef MAP_MAPDEFINES_H
#define MAP_MAPDEFINES_H

#include "common/mmo.h" // packet versions

#define MAX_NPC_PER_MAP 512
#define AREA_SIZE (battle->bc->area_size)
#define CHAT_AREA_SIZE (battle->bc->chat_area_size)
#define DEAD_AREA_SIZE (battle->bc->dead_area_size)
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_MOBSKILL 50
#define MAX_MOB_LIST_PER_MAP 100
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MIN_FLOORITEM 2
#define MAX_FLOORITEM START_ACCOUNT_NUM
#define MAX_IGNORE_LIST 20 // official is 14
#define MAX_VENDING 12
#define MAX_MAP_SIZE (512*512) // Wasn't there something like this already? Can't find it.. [Shinryo]

#define BLOCK_SIZE 8
#define block_free_max 1048576
#define BL_LIST_MAX 1048576

// The following system marks a different job ID system used by the map server,
// which makes a lot more sense than the normal one. [Skotlex]
// These marks the "level" of the job.
#define JOBL_2_1   0x0100
#define JOBL_2_2   0x0200
#define JOBL_2     0x0300 // JOBL_2_1 | JOBL_2_2
#define JOBL_UPPER 0x1000
#define JOBL_BABY  0x2000
#define JOBL_THIRD 0x4000

// For filtering and quick checking.
#define MAPID_BASEMASK 0x00ff
#define MAPID_UPPERMASK 0x0fff
#define MAPID_THIRDMASK (JOBL_THIRD|MAPID_UPPERMASK)

// Max size for inputs to Vending text prompts
#define MESSAGE_SIZE (79 + 1)
// Max size for inputs to Graffiti, Talkie Box text prompts
#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
#define TALKBOX_MESSAGE_SIZE 21
#else
#define TALKBOX_MESSAGE_SIZE (79 + 1)
#endif
// String length you can write in the 'talking box'
#define CHATBOX_SIZE (70 + 1)
// Chatroom-related string sizes
#define CHATROOM_TITLE_SIZE (36 + 1)
#define CHATROOM_PASS_SIZE (8 + 1)
// Max allowed chat text length
#define CHAT_SIZE_MAX (255 + 1)
// 24 for npc name + 24 for label + 2 for a "::" and 1 for EOS
#define EVENT_NAME_LENGTH ( NAME_LENGTH * 2 + 3 )
#define DEFAULT_AUTOSAVE_INTERVAL (5*60*1000)
// Specifies maps where players may hit each other
#define map_flag_vs(m) ( \
		map->list[m].flag.pvp \
		|| map->list[m].flag.gvg_dungeon \
		|| map->list[m].flag.gvg \
		|| ((map->agit_flag || map->agit2_flag) && map->list[m].flag.gvg_castle) \
		|| map->list[m].flag.battleground \
		|| map->list[m].flag.cvc \
		)
// Specifies maps that have special GvG/WoE restrictions
#define map_flag_gvg(m) (map->list[m].flag.gvg || ((map->agit_flag || map->agit2_flag) && map->list[m].flag.gvg_castle))
// Specifies if the map is tagged as GvG/WoE (regardless of map->agit_flag status)
#define map_flag_gvg2(m) (map->list[m].flag.gvg || map->list[m].flag.gvg_castle)
// No Kill Steal Protection
#define map_flag_ks(m) (map->list[m].flag.town || map->list[m].flag.pvp || map->list[m].flag.gvg || map->list[m].flag.battleground)
// No ViewID
#define map_no_view(m, view) (map->list[m].flag.noviewid & (view))

// For common mapforeach calls. Since pets cannot be affected, they aren't included here yet.
#define BL_CHAR (BL_PC|BL_MOB|BL_HOM|BL_MER|BL_ELEM)

#define MAP_ZONE_NAME_LENGTH 60
#define MAP_ZONE_ALL_NAME "All"
#define MAP_ZONE_NORMAL_NAME "Normal"
#define MAP_ZONE_PVP_NAME "PvP"
#define MAP_ZONE_GVG_NAME "GvG"
#define MAP_ZONE_BG_NAME "Battlegrounds"
#define MAP_ZONE_CVC_NAME "CvC"
#define MAP_ZONE_PK_NAME "PK Mode"
#define MAP_ZONE_MAPFLAG_LENGTH 65

#endif /* MAP_MAPDEFINES_H */
