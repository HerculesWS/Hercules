// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams
#ifndef CONFIG_GENERAL_H
#define CONFIG_GENERAL_H

/**
 * Hercules configuration file (http://hercules.ws)
 **/

/**
 * Default Magical Reflection Behavior
 * - When reflecting, reflected damage depends on gears caster is wearing, not target (official)
 * - When disabled damage depends on gears target is wearing, not caster. (old/eathena)
 * @values 1 (enabled) or 0 (disabled)
 **/
#define MAGIC_REFLECTION_TYPE 1

/**
 * Spirit Sphere Limitation
 **/
#define MAX_SPIRITBALL 15

/**
 * when enabled, reflect damage doesn't bypass devotion (and thus damage is passed to crusader)
 * uncomment to enable
 **/
//#define DEVOTION_REFLECT_DAMAGE

/**
 * No settings past this point
 **/

#endif // CONFIG_GENERAL_H
