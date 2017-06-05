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
#ifndef MAP_PATH_H
#define MAP_PATH_H

#include "map/map.h" // enum cell_chk
#include "common/hercules.h"

#define MOVE_COST 10
#define MOVE_DIAGONAL_COST 14

#define MAX_WALKPATH 32

struct walkpath_data {
	unsigned char path_len,path_pos;
	unsigned char path[MAX_WALKPATH];
};

struct shootpath_data {
	int rx,ry,len;
	int x[MAX_WALKPATH];
	int y[MAX_WALKPATH];
};

#define check_distance_bl(bl1, bl2, distance)       (path->check_distance((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y, distance))
#define check_distance_blxy(bl, x1, y1, distance)   (path->check_distance((bl)->x - (x1), (bl)->y - (y1), distance))
#define check_distance_xy(x0, y0, x1, y1, distance) (path->check_distance((x0) - (x1), (y0) - (y1), distance))

#define distance_bl(bl1, bl2)       (path->distance((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y))
#define distance_blxy(bl, x1, y1)   (path->distance((bl)->x - (x1), (bl)->y - (y1)))
#define distance_xy(x0, y0, x1, y1) (path->distance((x0) - (x1), (y0) - (y1)))

#define check_distance_client_bl(bl1, bl2, distance) (path->check_distance_client((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y, distance))
#define check_distance_client_blxy(bl, x1, y1, distance) (path->check_distance_client((bl)->x-(x1), (bl)->y-(y1), distance))
#define check_distance_client_xy(x0, y0, x1, y1, distance) (path->check_distance_client((x0)-(x1), (y0)-(y1), distance))

#define distance_client_bl(bl1, bl2) (path->distance_client((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y))
#define distance_client_blxy(bl, x1, y1) (path->distance_client((bl)->x-(x1), (bl)->y-(y1)))
#define distance_client_xy(x0, y0, x1, y1) (path->distance_client((x0)-(x1), (y0)-(y1)))

struct path_interface {
	// calculates destination cell for knockback
	int (*blownpos) (struct block_list *bl, int16 m, int16 x0, int16 y0, int16 dx, int16 dy, int count);
	// tries to find a walkable path
	bool (*search) (struct walkpath_data *wpd, struct block_list *bl, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int flag, cell_chk cell);
	// tries to find a shootable path
	bool (*search_long) (struct shootpath_data *spd, struct block_list *bl, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, cell_chk cell);
	bool (*check_distance) (int dx, int dy, int distance);
	unsigned int (*distance) (int dx, int dy);
	bool (*check_distance_client) (int dx, int dy, int distance);
	int (*distance_client) (int dx, int dy);
};

#ifdef HERCULES_CORE
void path_defaults(void);
#endif // HERCULES_CORE

HPShared struct path_interface *path;

#endif /* MAP_PATH_H */
