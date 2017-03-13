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
#ifndef CONFIG_RENEWAL_H
#define CONFIG_RENEWAL_H

/**
 * Hercules configuration file (http://herc.ws)
 * For detailed guidance on these check http://herc.ws/wiki/SRC/config/
 **/

/**
 * @INFO: This file holds general-purpose renewal settings, for class-specific ones check /src/config/classes folder
 **/

/**
 * Renewal full toggle switch.
 *
 * Uncomment this line to disable all of the below settings at once.
 * Note: in UNIX builds, this can be easily done without touching this
 * line, by passing --disable-renewal to the configure script:
 * ./configure --disable-renewal
 */
#define DISABLE_RENEWAL


#ifndef DISABLE_RENEWAL // Do not change this line

/// game renewal server mode
/// (disable by commenting the line)
///
/// leave this line to enable renewal specific support such as renewal formulas
#define RENEWAL

/// renewal cast time
/// (disable by commenting the line)
///
/// leave this line to enable renewal casting time algorithms
/// cast time is decreased by DEX * 2 + INT while 20% of the cast time is not reduced by stats.
/// example:
///  on a skill whos cast time is 10s, only 8s may be reduced. the other 2s are part of a
///  "fixed cast time" which can only be reduced by specialist items and skills
#define RENEWAL_CAST

/// renewal drop rate algorithms
/// (disable by commenting the line)
///
/// leave this line to enable renewal item drop rate algorithms
/// while enabled a special modified based on the difference between the player and monster level is applied
/// based on the http://irowiki.org/wiki/Drop_System#Level_Factor table
#define RENEWAL_DROP

/// renewal exp rate algorithms
/// (disable by commenting the line)
///
/// leave this line to enable renewal item exp rate algorithms
/// while enabled a special modified based on the difference between the player and monster level is applied
#define RENEWAL_EXP

/// renewal level modifier on damage
/// (disable by commenting the line)
///
// leave this line to enable renewal base level modifier on skill damage (selected skills only)
#define RENEWAL_LVDMG

/// renewal enchant deadly poison algorithm
///
/// leave this line to enable the renewed EDP algorithm
/// under renewal mode:
///  - damage is NOT increased by 400%
///  - it does NOT affect grimtooth
///  - weapon and status ATK are increased
///  - some skill's damage ratio has modified
#define RENEWAL_EDP

/// renewal ASPD [malufett]
///
/// leave this line to enable renewal ASPD
/// - shield penalty is applied
/// - AGI has a greater factor in ASPD increase
/// - there is a change in how skills/items give ASPD
/// - some skill/item ASPD bonuses won't stack
#define RENEWAL_ASPD

#endif // DISABLE_RENEWAL
#undef DISABLE_RENEWAL

#endif // CONFIG_RENEWAL_H
