/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2015  Hercules Dev Team
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
#ifndef COMMON_HERCULES_H
#define COMMON_HERCULES_H

#include "config/core.h"
#include "common/cbasetypes.h"

#ifdef WIN32
	#define HPExport __declspec(dllexport)
#else
	#define HPExport __attribute__((visibility("default")))
#endif

#define HPShared extern

#ifndef HERCULES_CORE
#include "common/HPMi.h"
#endif // HERCULES_CORE

#endif // COMMON_HERCULES_H
