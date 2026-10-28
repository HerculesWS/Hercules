/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2026 Hercules Dev Team
 * Copyright (C) hemagx
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

#ifndef COMMON_ERS_FWD_H
#define COMMON_ERS_FWD_H

#include <cstddef>

constexpr size_t ers_chunk_blocks_count = 2048; // Default blocks count

template<typename T, size_t blocks_per_chunk = ers_chunk_blocks_count>
class ERS;

#endif
