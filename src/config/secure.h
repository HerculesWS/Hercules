// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams
#ifndef _CONFIG_SECURE_H_
#define _CONFIG_SECURE_H_

/**
 * Hercules configuration file (http://hercules.ws)
 * For detailed guidance on these check http://hercules.ws/wiki/SRC/config/
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

#endif // _CONFIG_SECURE_H_
