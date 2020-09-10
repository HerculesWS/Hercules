/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
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
#define HERCULES_CORE

#include "config/core.h"
#include "api/jsonwriter.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/nullpo.h"

#include <cJSON/cJSON.h>

static struct jsonwriter_interface jsonwriter_s;
struct jsonwriter_interface *jsonwriter;

static int do_init_jsonwriter(bool minimal)
{
	return 0;
}

static void do_final_jsonwriter(void)
{
}

void jsonwriter_defaults(void)
{
	jsonwriter = &jsonwriter_s;
	/* core */
	jsonwriter->init = do_init_jsonwriter;
	jsonwriter->final = do_final_jsonwriter;
}
