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
#ifndef CONFIG_CORE_H
#define CONFIG_CORE_H

/// Max number of items on @autolootid list
#define AUTOLOOTITEM_SIZE 10

/// The maximum number of atcommand suggestions
#define MAX_SUGGESTIONS 10

/// Comment to disable the official walk path
/// The official walkpath disables users from taking non-clear walk paths,
/// e.g. if they want to get around an obstacle they have to walk around it,
/// while with OFFICIAL_WALKPATH disabled if they click to walk around a obstacle the server will do it automatically
#define OFFICIAL_WALKPATH

/// leave this line uncommented to enable callfunc checks when processing scripts.
/// while allowed, the script engine will attempt to match user-defined functions
/// in scripts allowing direct function callback (without the use of callfunc.)
/// this CAN affect performance, so if you find scripts running slower or find
/// your map-server using more resources while this is active, comment the line
#define SCRIPT_CALLFUNC_CHECK

/**
 * Strip linebreaks from `mes` dialogs.
 *
 * Leave this line commented out to keep the hard line-breaks (`\r`) in the
 * displayed `mes` dialogs (as in official servers).
 * Uncomment it to strip the line-breaks and replace them with spaces, letting
 * the client automatically wrap text in dialogs, depending on font size and
 * dialog window size (may work better for clients using a non-standard font).
 */
//#define SCRIPT_MES_STRIP_LINEBREAK

/// Comment to disable Hercules' console_parse
/// CONSOLE_INPUT allows you to type commands into the server's console,
/// Disabling it saves one thread.
#define CONSOLE_INPUT
/// Maximum number of characters 'CONSOLE_INPUT' will support per line.
#define MAX_CONSOLE_INPUT 150

/// Uncomment to disable Hercules' anonymous stat report
/// We kindly ask you to consider keeping it enabled, it helps us improve Hercules.
//#define STATS_OPT_OUT

/// Uncomment to enable the Cell Stack Limit mod.
/// It's only config is the battle_config custom_cell_stack_limit.
/// Only chars affected are those defined in BL_CHAR
//#define CELL_NOSTACK

/// Uncomment to enable circular area checks.
/// By default, most server-sided range checks in Aegis are of square shapes, so a monster
/// with a range of 4 can attack anything within a 9x9 area.
/// Client-sided range checks are, however, are always circular.
/// Enabling this changes all checks to circular checks, which is more realistic,
/// - but is not the official behaviour.
//#define CIRCULAR_AREA

//This is the distance at which @autoloot works,
//if the item drops farther from the player than this,
//it will not be autolooted. [Skotlex]
//Note: The range is unlimited unless this define is set.
//#define AUTOLOOT_DISTANCE AREA_SIZE

/// Uncomment to switch the way map zones' "skill_damage_cap" functions.
/// When commented the cap takes place before modifiers, as to have them be useful.
/// When uncommented the cap takes place after modifiers.
//#define HMAP_ZONE_DAMAGE_CAP_TYPE

/// Comment to disable Guild/Party Bound item system
#define GP_BOUND_ITEMS

/// Uncomment to enable real-time server stats (in and out data and ram usage). [Ai4rei]
//#define SHOW_SERVER_STATS

/// Comment to disable autotrade persistency (where autotrading merchants survive server restarts)
#define AUTOTRADE_PERSISTENCY

/**
 * No settings past this point
 **/
#include "./renewal.h"
#include "./secure.h"
#include "./classes/general.h"

/**
 * Constants come last; so they process anything that could've been modified in early includes
 **/
#include "./const.h"

#endif // CONFIG_CORE_H
