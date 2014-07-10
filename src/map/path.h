// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef MAP_PATH_H
#define MAP_PATH_H

#include "map.h" // enum cell_chk
#include "../common/cbasetypes.h"

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

struct path_interface {
	// calculates destination cell for knockback
	int (*blownpos) (int16 m, int16 x0, int16 y0, int16 dx, int16 dy, int count);
	// tries to find a walkable path
	bool (*search) (struct walkpath_data *wpd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int flag, cell_chk cell);
	// tries to find a shootable path
	bool (*search_long) (struct shootpath_data *spd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, cell_chk cell);
	int (*check_distance) (int dx, int dy, int distance);
	unsigned int (*distance) (int dx, int dy);
};

struct path_interface *path;

void path_defaults(void);

#endif /* MAP_PATH_H */
