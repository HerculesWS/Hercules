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

struct rnd_interface {
	void (*init) (void);
	void (*final) (void);

	void (*seed) (uint32);

	int32 (*random) (void);// [0, SINT32_MAX]
	uint32 (*roll) (uint32 dice_faces);// [0, dice_faces)
	int32 (*value) (int32 min, int32 max);// [min, max]
	double (*uniform) (void);// [0.0, 1.0)
	double (*uniform53) (void);// [0.0, 1.0)
};

#define rnd() rnd->random()

#ifdef HERCULES_CORE
void rnd_defaults(void);
#endif // HERCULES_CORE

HPShared struct rnd_interface *rnd;

#endif /* COMMON_RANDOM_H */
