/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include "int_userconfig.h"

#include "char/char.h"
#include "char/int_mail.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
#include "common/apipackets.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

static struct inter_userconfig_interface inter_userconfig_s;
struct inter_userconfig_interface *inter_userconfig;

static int inter_userconfig_load_emotes(int account_id, struct userconfig_emotes *emotes)
{
	// should be used strncpy [4144]
	strncpy(emotes->emote[0], "/!", EMOTE_SIZE);
	strncpy(emotes->emote[1], "/?", EMOTE_SIZE);
	strncpy(emotes->emote[2], "/기쁨", EMOTE_SIZE);
	strncpy(emotes->emote[3], "/하트", EMOTE_SIZE);
	strncpy(emotes->emote[4], "/땀", EMOTE_SIZE);
	strncpy(emotes->emote[5], "/아하", EMOTE_SIZE);
	strncpy(emotes->emote[6], "/짜증", EMOTE_SIZE);
	strncpy(emotes->emote[7], "/화", EMOTE_SIZE);
	strncpy(emotes->emote[8], "/돈", EMOTE_SIZE);
	strncpy(emotes->emote[9], "/...", EMOTE_SIZE);

	// english emotes
//	"/!","/?","/ho","/lv","/swt","/ic","/an","/ag","/$","/..."
	return 0;
}

static int inter_userconfig_save_emotes(int account_id, const struct userconfig_emotes *emotes)
{
	for (int f = 0; f < MAX_EMOTES; f ++)
	{
		ShowWarning("save emote %d: %s\n", f, emotes->emote[f]);
	}
	return 0;
}

void inter_userconfig_defaults(void)
{
	inter_userconfig = &inter_userconfig_s;

	inter_userconfig->load_emotes = inter_userconfig_load_emotes;
	inter_userconfig->save_emotes = inter_userconfig_save_emotes;
}
