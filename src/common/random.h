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
#ifndef COMMON_RANDOM_H
#define COMMON_RANDOM_H

#include "common/hercules.h"

/** @file
 * The random number generator interface.
 */

/// Random interface.
struct rnd_interface {
	/**
	 * Interface initialization.
	 *
	 * During initialization, the RNG is seeded with a random seed.
	 */
	void (*init) (void);

	/// Interface finalization.
	void (*final) (void);

	/**
	 * Re-seeds the random number generator.
	 *
	 * @param seed The new seed.
	 */
	void (*seed) (uint32 seed);

	/**
	 * Generates a random number in the interval [0, SINT32_MAX].
	 */
	int32 (*random) (void);

	/**
	 * Generates a random number in the interval [0, dice_faces).
	 *
	 * @remark
	 *  interval is open ended, so dice_faces is excluded (unless it's 0)
	 */
	uint32 (*roll) (uint32 dice_faces);

	/**
	 * Generates a random number in the interval [min, max].
	 *
	 * @retval min if range is invalid.
	 */
	int32 (*value) (int32 min, int32 max);

	/**
	 * Generates a random number in the interval [0.0, 1.0)
	 *
	 * @remark
	 *  interval is open ended, so 1.0 is excluded
	 */
	double (*uniform) (void);

	/**
	 * Generates a random number in the interval [0.0, 1.0) with 53-bit resolution.
	 *
	 * 53 bits is the maximum precision of a double.
	 *
	 * @remark
	 *   interval is open ended, so 1.0 is excluded
	 */
	double (*uniform53) (void);
};

/**
 * Utlity macro to call the frequently used rnd_interface#random().
 *
 * @related rnd_interface.
 */
#define rnd() rnd->random()

#ifdef HERCULES_CORE
void rnd_defaults(void);
#endif // HERCULES_CORE

HPShared struct rnd_interface *rnd; ///< Pointer to the random interface.

#endif /* COMMON_RANDOM_H */
