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
#ifndef CONFIG_SECURE_H
#define CONFIG_SECURE_H

/**
 * Hercules configuration file (http://herc.ws)
 * For detailed guidance on these check http://herc.ws/wiki/SRC/config/
 **/

/**
 * @INFO: This file holds optional security settings
 **/

/**
 * Optional NPC Dialog Timer
 * When enabled all npcs dialog will 'timeout' if user is on idle for longer than the amount of seconds allowed
 * - On 'timeout' the npc dialog window changes its next/menu to a 'close' button
 * Uncomment to enable
 **/
//#define SECURE_NPCTIMEOUT

/**
 * number of seconds to 'timeout' if the user is on idle for longer than the value allowed after a 'input' field is displayed.
 * default: 180
 **/
#define NPC_SECURE_TIMEOUT_INPUT 180

/**
 * number of seconds to 'timeout' if the user is on idle for longer than the value allowed after a 'menu' is displayed.
 * default: 60
 **/
#define NPC_SECURE_TIMEOUT_MENU 60

/**
 * number of seconds to 'timeout' if the user is on idle for longer than the value allowed after a 'next' button is displayed.
 * default: 60
 **/
#define NPC_SECURE_TIMEOUT_NEXT 60

/**
 * (Secure) Optional NPC Dialog Timer
 * @requirement : SECURE_NPCTIMEOUT must be enabled
 * Minimum Interval Between timeout checks in seconds
 * Default: 1s
 **/
#define SECURE_NPCTIMEOUT_INTERVAL 1

/**
 * Uncomment to disable
 * while enabled, movement of invisible (cloaking, hide, etc [not chase walk]) units is not informed to nearby foes,
 * rendering any client-side cheat, that would otherwise make these units visible, to not function.
 * - "Why is this a setting?" because theres a cost, while enabled if a hidden character uses a skill with cast time,
 * - for example "cloaking -> walk a bit -> soul break another player" the character display will be momentarily abrupted
 * - on the action of unhiding (its a quick effect, ~0.007s in duration), and due to the nature of the skill cast on the client
 * - it was not possible to work around this, and thus why it is optional, comment the line to disable.
 **/
#define ANTI_MAYAP_CHEAT

#endif // CONFIG_SECURE_H
