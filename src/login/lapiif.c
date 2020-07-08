/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "lapiif.h"

#include "common/cbasetypes.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"


static struct lapiif_interface lapiif_s;
struct lapiif_interface *lapiif;

static void lapiif_disconnect_user(int account_id)
{
}

static void lapiif_connect_user(struct login_session_data *sd, const unsigned char* auth_token)
{
}

static void lapiif_init(void)
{
}

static void lapiif_final(void)
{
}

void lapiif_defaults(void)
{
	lapiif = &lapiif_s;

	lapiif->init = lapiif_init;
	lapiif->final = lapiif_final;
	lapiif->connect_user = lapiif_connect_user;
	lapiif->disconnect_user = lapiif_disconnect_user;
}
