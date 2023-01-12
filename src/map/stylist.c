/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2018-2023 Hercules Dev Team
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

#include "map/stylist.h"

#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"

#include "map/clif.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/pc.h"
#include "map/script.h"

static struct stylist_interface stylist_s;
struct stylist_interface *stylist;

static bool stylist_read_db_libconfig(void)
{
	struct config_t stylist_conf;
	struct config_setting_t *stylist_db = NULL, *it = NULL;
	char config_filename[256];
	libconfig->format_db_path("stylist_db.conf", config_filename, sizeof(config_filename));
	int i = 0;

	if (!libconfig->load_file(&stylist_conf, config_filename))
		return false;

	if ((stylist_db = libconfig->setting_get_member(stylist_conf.root, "stylist_db")) == NULL) {
		ShowError("can't read %s\n", config_filename);
		return false;
	}

	stylist->vector_clear();

	while ((it = libconfig->setting_get_elem(stylist_db, i++))) {
		stylist->read_db_libconfig_sub(it, i - 1, config_filename);
	}

	libconfig->destroy(&stylist_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, config_filename);
	return true;
}

static bool stylist_read_db_libconfig_sub(struct config_setting_t *it, int idx, const char *source)
{
	struct stylist_data_entry entry = { 0 };
	int i32 = 0, type = 0;
	int64 i64 = 0;

	nullpo_ret(it);
	nullpo_ret(source);

	if (!map->setting_lookup_const(it, "Type", &type) || type >= MAX_STYLIST_TYPE || type < 0) {
		ShowWarning("stylist_read_db_libconfig_sub: Invalid or missing Type (%d) in \"%s\", entry #%d, skipping.\n", type, source, idx);
		return false;
	}
	if (!map->setting_lookup_const(it, "Id", &i32) || i32 < 0) {
		ShowWarning("stylist_read_db_libconfig_sub: Invalid or missing Id (%d) in \"%s\", entry #%d, skipping.\n", i32, source, idx);
		return false;
	}
	entry.id = i32;

	if (libconfig->setting_lookup_int64(it, "Zeny", &i64)) {
		if (i64 > MAX_ZENY) {
			ShowWarning("stylist_read_db_libconfig_sub: zeny is too big in \"%s\", entry #%d, capping to MAX_ZENY.\n", source, idx);
			entry.zeny = MAX_ZENY;
		} else {
			entry.zeny = (int)i64;
		}
	}

	if (map->setting_lookup_const(it, "ItemID", &i32))
		entry.itemid = i32;

	if (map->setting_lookup_const(it, "BoxItemID", &i32))
		entry.boxid = i32;

	if (libconfig->setting_lookup_bool(it, "AllowDoram", &i32))
		entry.allow_doram = (i32 == 0) ? false : true;

	VECTOR_ENSURE(stylist->data[type], 1, 1);
	VECTOR_PUSH(stylist->data[type], entry);
	return true;
}

static bool stylist_validate_requirements(struct map_session_data *sd, int type, int16 idx)
{
	struct item it;
	struct stylist_data_entry *entry;

	nullpo_retr(false, sd);
	Assert_retr(false, type >= 0 && type < MAX_STYLIST_TYPE);
	Assert_retr(false, idx >= 0 && idx < VECTOR_LENGTH(stylist->data[type]));

	entry = &VECTOR_INDEX(stylist->data[type], idx);

	if (sd->status.class == JOB_SUMMONER && (entry->allow_doram == false))
		return false;

	if (entry->id >= 0) {
		if (entry->zeny != 0 && pc->payzeny(sd, entry->zeny, LOG_TYPE_STYLIST, NULL) != 0) {
			return false;
		} else if (entry->itemid != 0) {
			it.nameid = entry->itemid;
			it.amount = 1;
			return script->buildin_delitem_search(sd, &it, false);
		} else if (entry->boxid != 0) {
			it.nameid = entry->boxid;
			it.amount = 1;
			return script->buildin_delitem_search(sd, &it, false);
		}
		return true;
	}
	return false;
}

static void stylist_send_rodexitem(struct map_session_data *sd, int itemid)
{
	struct rodex_message msg = { 0 };

	nullpo_retv(sd);

	msg.receiver_id = sd->status.char_id;
	msg.items[0].item.nameid = itemid;
	msg.items[0].item.amount = 1;
	msg.items[0].item.identify = 1;
	msg.type = MAIL_TYPE_NPC | MAIL_TYPE_ITEM;

	safestrncpy(msg.sender_name, msg_txt(366), NAME_LENGTH);
	safestrncpy(msg.title, msg_txt(367), RODEX_TITLE_LENGTH);
	safestrncpy(msg.body, msg_txt(368), MAIL_BODY_LENGTH);
	msg.send_date = (int)time(NULL);
	msg.expire_date = (int)time(NULL) + RODEX_EXPIRE;

	intif->rodex_sendmail(&msg);
}

static void stylist_request_style_change(struct map_session_data *sd, int type, int16 idx, bool isitem)
{
	struct stylist_data_entry *entry;

	nullpo_retv(sd);
	Assert_retv(idx > 0);
	Assert_retv(type >= 0 && type < MAX_STYLIST_TYPE);

	if ((idx - 1) < VECTOR_LENGTH(stylist->data[type])) {
		entry = &VECTOR_INDEX(stylist->data[type], idx - 1);
		if (stylist->validate_requirements(sd, type, idx - 1)) {
			if (isitem == false)
				pc->changelook(sd, type, entry->id);
			else
				stylist->send_rodexitem(sd, entry->id);
		}
	}
}

static void stylist_vector_init(void)
{
	for (int i = 0; i < MAX_STYLIST_TYPE; i++)
		VECTOR_INIT(stylist->data[i]);
}
static void stylist_vector_clear(void)
{
	for (int i = 0; i < MAX_STYLIST_TYPE; i++)
		VECTOR_CLEAR(stylist->data[i]);
}

static void do_init_stylist(bool minimal)
{
	if (minimal)
		return;

	// Initialize the db
	stylist->vector_init();

	// Load the db
	stylist->read_db_libconfig();
}

static void do_final_stylist(void)
{
	// Clear the db
	stylist->vector_clear();
}

void stylist_defaults(void)
{
	stylist = &stylist_s;

	/* core */
	stylist->init = do_init_stylist;
	stylist->final = do_final_stylist;
	/* */
	stylist->vector_init = stylist_vector_init;
	stylist->vector_clear = stylist_vector_clear;
	/* database */
	stylist->read_db_libconfig = stylist_read_db_libconfig;
	stylist->read_db_libconfig_sub = stylist_read_db_libconfig_sub;
	/* */
	stylist->request_style_change = stylist_request_style_change;
	stylist->validate_requirements = stylist_validate_requirements;
	stylist->send_rodexitem = stylist_send_rodexitem;
}
