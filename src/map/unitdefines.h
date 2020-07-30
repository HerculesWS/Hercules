/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2019 Hercules Dev Team
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
#ifndef MAP_UNITDEFINES_H
#define MAP_UNITDEFINES_H

/**
 * Used for directions, @see unit_data.dir
 */
enum unit_dir {
	UNIT_DIR_UNDEFINED = -1,
	UNIT_DIR_FIRST     = 0,
	UNIT_DIR_NORTH     = 0,
	UNIT_DIR_NORTHWEST = 1,
	UNIT_DIR_WEST      = 2,
	UNIT_DIR_SOUTHWEST = 3,
	UNIT_DIR_SOUTH     = 4,
	UNIT_DIR_SOUTHEAST = 5,
	UNIT_DIR_EAST      = 6,
	UNIT_DIR_NORTHEAST = 7,
	UNIT_DIR_MAX       = 8,
	/* IMPORTANT: Changing the order would break the above macros
	 * and several usages of directions anywhere */
};

/* Returns the opposite of the facing direction */
#define unit_get_opposite_dir(dir) ( ((dir) + 4) % UNIT_DIR_MAX )

/* Returns true when direction is diagonal/combined (ex. UNIT_DIR_NORTHWEST, UNIT_DIR_SOUTHWEST, ...) */
#define unit_is_diagonal_dir(dir) ( ((dir) % 2) == UNIT_DIR_NORTHWEST )

/* Returns true if direction equals val or the opposite direction of val */
#define unit_is_dir_or_opposite(dir, val) ( ((dir) % 4) == (val) )

/* Returns the next direction after 90° CCW on a compass */
#define unit_get_ccw90_dir(dir) ( ((dir) + 2) % UNIT_DIR_MAX )

/* Returns a random diagonal direction */
#define unit_get_rnd_diagonal_dir() ( UNIT_DIR_NORTHWEST + 2 * (rnd() % 4) )

#endif /* MAP_UNITDEFINES_H */
