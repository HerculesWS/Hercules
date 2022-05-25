/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
 * Copyright (C) 2022 Andrei Karas (4144)
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

#ifndef CONFIG
#define CONFIG(a, b, c, d, e)
#endif
#ifndef CONFIGSTR
#define CONFIGSTR(a, b, c, d, e)
#endif

CONFIG(int, max_gif_guild_emblem_size, 51200, 0, 8388607)
CONFIG(int, max_bmp_guild_emblem_size, 1800,  0, 2000)
CONFIG(int, guild_emblem_width,        24,    1, 10000)
CONFIG(int, guild_emblem_height,       24,    1, 10000)
CONFIG(int, min_guild_emblem_frames,   1,     1, 10000)
CONFIG(int, max_guild_emblem_frames,   10000, 1, 10000)
