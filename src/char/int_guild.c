/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
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
#define HERCULES_CORE

#include "config/core.h" // DBPATH
#include "int_guild.h"

#include "char/char.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
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

#define GS_MEMBER_UNMODIFIED 0x00
#define GS_MEMBER_MODIFIED 0x01
#define GS_MEMBER_NEW 0x02

#define GS_POSITION_UNMODIFIED 0x00
#define GS_POSITION_MODIFIED 0x01

// LSB = 0 => Alliance, LSB = 1 => Opposition
#define GUILD_ALLIANCE_TYPE_MASK 0x01
#define GUILD_ALLIANCE_REMOVE 0x08

struct inter_guild_interface inter_guild_s;
struct inter_guild_interface *inter_guild;

static const char dataToHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int inter_guild_save_timer(int tid, int64 tick, int id, intptr_t data) {
	static int last_id = 0; //To know in which guild we were.
	int state = 0; //0: Have not reached last guild. 1: Reached last guild, ready for save. 2: Some guild saved, don't do further saving.
	struct DBIterator *iter = db_iterator(inter_guild->guild_db);
	union DBKey key;
	struct guild* g;

	if( last_id == 0 ) //Save the first guild in the list.
		state = 1;

	for( g = DB->data2ptr(iter->first(iter, &key)); dbi_exists(iter); g = DB->data2ptr(iter->next(iter, &key)) )
	{
		if (!g)
			continue;
		if( state == 0 && g->guild_id == last_id )
			state++; //Save next guild in the list.
		else
		if( state == 1 && g->save_flag&GS_MASK )
		{
			inter_guild->tosql(g, g->save_flag&GS_MASK);
			g->save_flag &= ~GS_MASK;

			//Some guild saved.
			last_id = g->guild_id;
			state++;
		}

		if( g->save_flag == GS_REMOVE )
		{// Nothing to save, guild is ready for removal.
			if (save_log)
				ShowInfo("Guild Unloaded (%d - %s)\n", g->guild_id, g->name);
			db_remove(inter_guild->guild_db, key);
		}
	}
	dbi_destroy(iter);

	if( state != 2 ) //Reached the end of the guild db without saving.
		last_id = 0; //Reset guild saved, return to beginning.

	state = inter_guild->guild_db->size(inter_guild->guild_db);
	if( state < 1 ) state = 1; //Calculate the time slot for the next save.
	timer->add(tick + autosave_interval/state, inter_guild->save_timer, 0, 0);
	return 0;
}

int inter_guild_removemember_tosql(int account_id, int char_id)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE from `%s` where `account_id` = '%d' and `char_id` = '%d'", guild_member_db, account_id, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `guild_id` = '0' WHERE `char_id` = '%d'", char_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	return 0;
}

/**
 * Saves the requested guild information to SQL.
 *
 * @param g    The guild data to save.
 * @param flag Type of information to be saved (@see enum guild_save_types).
 * @retval true  in case of success.
 * @retval false in case of error.
 *
 * Fields saved depending on the flag type:
 *   Table guild (GS_BASIC_MASK)
 *   GS_EMBLEM `emblem_len`,`emblem_id`,`emblem_data`
 *   GS_CONNECT `connect_member`,`average_lv`
 *   GS_MES `mes1`,`mes2`
 *   GS_LEVEL `guild_lv`,`max_member`,`exp`,`next_exp`,`skill_point`,`max_storage`
 *   GS_BASIC `name`,`master`,`char_id`
 *   GS_MEMBER `guild_member` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name`)
 *   GS_POSITION `guild_position` (`guild_id`,`position`,`name`,`mode`,`exp_mode`)
 *   GS_ALLIANCE `guild_alliance` (`guild_id`,`opposition`,`alliance_id`,`name`)
 *   GS_EXPULSION `guild_expulsion` (`guild_id`,`account_id`,`name`,`mes`)
 *   GS_SKILL `guild_skill` (`guild_id`,`id`,`lv`)
 */
bool inter_guild_tosql(struct guild *g, int flag)
{
	// temporary storage for str conversion. They must be twice the size of the
	// original string to ensure no overflows will occur. [Skotlex]
	char t_info[256];
	char esc_name[NAME_LENGTH*2+1];
	char esc_master[NAME_LENGTH*2+1];
	char new_guild = 0;
	int i=0;

	nullpo_retr(false, g);
	Assert_retr(false, g->guild_id > 0 || g->guild_id == -1);

#ifdef NOISY
	ShowInfo("Save guild request ("CL_BOLD"%d"CL_RESET" - flag 0x%x).",g->guild_id, flag);
#endif

	SQL->EscapeStringLen(inter->sql_handle, esc_name, g->name, strnlen(g->name, NAME_LENGTH));
	SQL->EscapeStringLen(inter->sql_handle, esc_master, g->master, strnlen(g->master, NAME_LENGTH));
	*t_info = '\0';

	// Insert a new guild the guild
	if (flag&GS_BASIC && g->guild_id == -1) {
		strcat(t_info, " guild_create");

		// Create a new guild
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` "
				"(`name`,`master`,`guild_lv`,`max_member`,`average_lv`,`char_id`) "
				"VALUES ('%s', '%s', '%d', '%d', '%d', '%d')",
				guild_db, esc_name, esc_master, g->guild_lv, g->max_member, g->average_lv, g->member[0].char_id)) {
			Sql_ShowDebug(inter->sql_handle);
			return false; //Failed to create guild!
		} else {
			g->guild_id = (int)SQL->LastInsertId(inter->sql_handle);
			new_guild = 1;
		}
	}

	// If we need an update on an existing guild or more update on the new guild
	if (((flag & GS_BASIC_MASK) && !new_guild) || ((flag & (GS_BASIC_MASK & ~GS_BASIC)) && new_guild))
	{
		StringBuf buf;
		bool add_comma = false;

		StrBuf->Init(&buf);
		StrBuf->Printf(&buf, "UPDATE `%s` SET ", guild_db);

		if (flag & GS_EMBLEM)
		{
			char emblem_data[sizeof(g->emblem_data)*2+1];
			char* pData = emblem_data;

			strcat(t_info, " emblem");
			// Convert emblem_data to hex
			//TODO: why not use binary directly? [ultramage]
			for(i=0; i<g->emblem_len; i++){
				*pData++ = dataToHex[(g->emblem_data[i] >> 4) & 0x0F];
				*pData++ = dataToHex[g->emblem_data[i] & 0x0F];
			}
			*pData = 0;
			StrBuf->Printf(&buf, "`emblem_len`=%d, `emblem_id`=%d, `emblem_data`='%s'", g->emblem_len, g->emblem_id, emblem_data);
			add_comma = true;
		}
		if (flag & GS_BASIC)
		{
			strcat(t_info, " basic");
			if( add_comma )
				StrBuf->AppendStr(&buf, ", ");
			else
				add_comma = true;
			StrBuf->Printf(&buf, "`name`='%s', `master`='%s', `char_id`=%d",
				esc_name, esc_master, g->member[0].char_id);
		}
		if (flag & GS_CONNECT)
		{
			strcat(t_info, " connect");
			if( add_comma )
				StrBuf->AppendStr(&buf, ", ");
			else
				add_comma = true;
			StrBuf->Printf(&buf, "`connect_member`=%d, `average_lv`=%d", g->connect_member, g->average_lv);
		}
		if (flag & GS_MES)
		{
			char esc_mes1[sizeof(g->mes1)*2+1];
			char esc_mes2[sizeof(g->mes2)*2+1];

			strcat(t_info, " mes");
			if( add_comma )
				StrBuf->AppendStr(&buf, ", ");
			else
				add_comma = true;
			SQL->EscapeStringLen(inter->sql_handle, esc_mes1, g->mes1, strnlen(g->mes1, sizeof(g->mes1)));
			SQL->EscapeStringLen(inter->sql_handle, esc_mes2, g->mes2, strnlen(g->mes2, sizeof(g->mes2)));
			StrBuf->Printf(&buf, "`mes1`='%s', `mes2`='%s'", esc_mes1, esc_mes2);
		}
		if (flag & GS_LEVEL)
		{
			strcat(t_info, " level");
			if( add_comma )
				StrBuf->AppendStr(&buf, ", ");
#if 0
			else //last condition using add_coma setting
				add_comma = true;
#endif // 0
			StrBuf->Printf(&buf, "`guild_lv`=%d, `skill_point`=%d, `exp`=%"PRIu64", `next_exp`=%u, `max_member`=%d, `max_storage`=%hd",
				g->guild_lv, g->skill_point, g->exp, g->next_exp, g->max_member, g->max_storage);
		}
		StrBuf->Printf(&buf, " WHERE `guild_id`=%d", g->guild_id);
		if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
			Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
	}

	if (flag&GS_MEMBER) {

		strcat(t_info, " members");
		// Update only needed players
		for(i=0;i<g->max_member;i++){
			struct guild_member *m = &g->member[i];
			if (!m->modified)
				continue;
			if(m->account_id) {
				//Since nothing references guild member table as foreign keys, it's safe to use REPLACE INTO
				SQL->EscapeStringLen(inter->sql_handle, esc_name, m->name, strnlen(m->name, NAME_LENGTH));
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name`) "
					"VALUES ('%d','%d','%d','%d','%d','%d','%d','%d','%"PRIu64"','%d','%d','%d','%s')",
					guild_member_db, g->guild_id, m->account_id, m->char_id,
					m->hair, m->hair_color, m->gender,
					m->class_, m->lv, m->exp, m->exp_payper, m->online, m->position, esc_name) )
					Sql_ShowDebug(inter->sql_handle);
				if (m->modified&GS_MEMBER_NEW || new_guild == 1)
				{
					if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `guild_id` = '%d' WHERE `char_id` = '%d'",
						char_db, g->guild_id, m->char_id) )
						Sql_ShowDebug(inter->sql_handle);
				}
				m->modified = GS_MEMBER_UNMODIFIED;
			}
		}
	}

	if (flag&GS_POSITION){
		strcat(t_info, " positions");
		//printf("- Insert guild %d to guild_position\n",g->guild_id);
		for(i=0;i<MAX_GUILDPOSITION;i++){
			struct guild_position *p = &g->position[i];
			if (!p || !p->modified)
				continue;
			SQL->EscapeStringLen(inter->sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`guild_id`,`position`,`name`,`mode`,`exp_mode`) VALUES ('%d','%d','%s','%d','%d')",
				guild_position_db, g->guild_id, i, esc_name, p->mode, p->exp_mode) )
				Sql_ShowDebug(inter->sql_handle);
			p->modified = GS_POSITION_UNMODIFIED;
		}
	}

	if (flag&GS_ALLIANCE)
	{
		// Delete current alliances
		// NOTE: no need to do it on both sides since both guilds in memory had
		// their info changed, not to mention this would also mess up oppositions!
		// [Skotlex]
		//if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d' OR `alliance_id`='%d'", guild_alliance_db, g->guild_id, g->guild_id) )
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d'", guild_alliance_db, g->guild_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
		}
		else
		{
			//printf("- Insert guild %d to guild_alliance\n",g->guild_id);
			for(i=0;i<MAX_GUILDALLIANCE;i++)
			{
				struct guild_alliance *a=&g->alliance[i];
				if(a->guild_id>0)
				{
					SQL->EscapeStringLen(inter->sql_handle, esc_name, a->name, strnlen(a->name, NAME_LENGTH));
					if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`guild_id`,`opposition`,`alliance_id`,`name`) "
						"VALUES ('%d','%d','%d','%s')",
						guild_alliance_db, g->guild_id, a->opposition, a->guild_id, esc_name) )
						Sql_ShowDebug(inter->sql_handle);
				}
			}
		}
	}

	if (flag&GS_EXPULSION){
		strcat(t_info, " expulsions");
		//printf("- Insert guild %d to guild_expulsion\n",g->guild_id);
		for(i=0;i<MAX_GUILDEXPULSION;i++){
			struct guild_expulsion *e=&g->expulsion[i];
			if(e->account_id>0){
				char esc_mes[sizeof(e->mes)*2+1];

				SQL->EscapeStringLen(inter->sql_handle, esc_name, e->name, strnlen(e->name, NAME_LENGTH));
				SQL->EscapeStringLen(inter->sql_handle, esc_mes, e->mes, strnlen(e->mes, sizeof(e->mes)));
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`guild_id`,`account_id`,`name`,`mes`) "
					"VALUES ('%d','%d','%s','%s')", guild_expulsion_db, g->guild_id, e->account_id, esc_name, esc_mes) )
					Sql_ShowDebug(inter->sql_handle);
			}
		}
	}

	if (flag&GS_SKILL){
		strcat(t_info, " skills");
		//printf("- Insert guild %d to guild_skill\n",g->guild_id);
		for(i=0;i<MAX_GUILDSKILL;i++){
			if (g->skill[i].id>0 && g->skill[i].lv>0){
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`guild_id`,`id`,`lv`) VALUES ('%d','%d','%d')",
					guild_skill_db, g->guild_id, g->skill[i].id, g->skill[i].lv) )
					Sql_ShowDebug(inter->sql_handle);
			}
		}
	}

	if (save_log)
		ShowInfo("Saved guild (%d - %s):%s\n",g->guild_id,g->name,t_info);
	return true;
}

/**
 * Retrieves a guild's information from SQL.
 *
 * @param guild_id The guild ID to look up.
 * @return The guild data or NULL.
 */
struct guild *inter_guild_fromsql(int guild_id)
{
	struct guild *g;
	char* data;
	size_t len;
	char* p;
	int i;

	if( guild_id <= 0 )
		return NULL;

	g = (struct guild*)idb_get(inter_guild->guild_db, guild_id);
	if( g )
		return g;

#ifdef NOISY
	ShowInfo("Guild load request (%d)...\n", guild_id);
#endif

	if( SQL_ERROR == SQL->Query(inter->sql_handle,
		"SELECT g.`name`,c.`name`,g.`guild_lv`,g.`connect_member`,g.`max_member`,g.`max_storage`,"
		"g.`average_lv`,g.`exp`,g.`next_exp`,g.`skill_point`,g.`mes1`,g.`mes2`,g.`emblem_len`,g.`emblem_id`,g.`emblem_data` "
		"FROM `%s` g LEFT JOIN `%s` c ON c.`char_id` = g.`char_id` WHERE g.`guild_id`='%d'", guild_db, char_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return NULL;
	}

	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
		return NULL;// Guild does not exists.

	CREATE(g, struct guild, 1);

	g->guild_id = guild_id;
	SQL->GetData(inter->sql_handle,  0, &data, &len); memcpy(g->name, data, min(len, NAME_LENGTH));
	SQL->GetData(inter->sql_handle,  1, &data, &len); memcpy(g->master, data, min(len, NAME_LENGTH));
	SQL->GetData(inter->sql_handle,  2, &data, NULL); g->guild_lv = atoi(data);
	SQL->GetData(inter->sql_handle,  3, &data, NULL); g->connect_member = atoi(data);
	SQL->GetData(inter->sql_handle,  4, &data, NULL); g->max_member = atoi(data);
	if (g->max_member > MAX_GUILD) {
		// Fix reduction of MAX_GUILD [PoW]
		ShowWarning("Guild %d:%s specifies higher capacity (%d) than MAX_GUILD (%d)\n", guild_id, g->name, g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}
	SQL->GetData(inter->sql_handle,  5, &data, NULL); g->max_storage = atoi(data);
	SQL->GetData(inter->sql_handle,  6, &data, NULL); g->average_lv = atoi(data);
	SQL->GetData(inter->sql_handle,  7, &data, NULL); g->exp = strtoull(data, NULL, 10);
	SQL->GetData(inter->sql_handle,  8, &data, NULL); g->next_exp = (unsigned int)strtoul(data, NULL, 10);
	SQL->GetData(inter->sql_handle,  9, &data, NULL); g->skill_point = atoi(data);
	SQL->GetData(inter->sql_handle, 10, &data, &len); memcpy(g->mes1, data, min(len, sizeof(g->mes1)));
	SQL->GetData(inter->sql_handle, 11, &data, &len); memcpy(g->mes2, data, min(len, sizeof(g->mes2)));
	SQL->GetData(inter->sql_handle, 12, &data, &len); g->emblem_len = atoi(data);
	SQL->GetData(inter->sql_handle, 13, &data, &len); g->emblem_id = atoi(data);
	SQL->GetData(inter->sql_handle, 14, &data, &len);
	// convert emblem data from hexadecimal to binary
	//TODO: why not store it in the db as binary directly? [ultramage]
	for( i = 0, p = g->emblem_data; i < g->emblem_len; ++i, ++p )
	{
		if( *data >= '0' && *data <= '9' )
			*p = *data - '0';
		else if( *data >= 'a' && *data <= 'f' )
			*p = *data - 'a' + 10;
		else if( *data >= 'A' && *data <= 'F' )
			*p = *data - 'A' + 10;
		*p <<= 4;
		++data;

		if( *data >= '0' && *data <= '9' )
			*p |= *data - '0';
		else if( *data >= 'a' && *data <= 'f' )
			*p |= *data - 'a' + 10;
		else if( *data >= 'A' && *data <= 'F' )
			*p |= *data - 'A' + 10;
		++data;
	}

	// load guild member info
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name` "
		"FROM `%s` WHERE `guild_id`='%d' ORDER BY `position`", guild_member_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < g->max_member && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		struct guild_member* m = &g->member[i];

		SQL->GetData(inter->sql_handle,  0, &data, NULL); m->account_id = atoi(data);
		SQL->GetData(inter->sql_handle,  1, &data, NULL); m->char_id = atoi(data);
		SQL->GetData(inter->sql_handle,  2, &data, NULL); m->hair = atoi(data);
		SQL->GetData(inter->sql_handle,  3, &data, NULL); m->hair_color = atoi(data);
		SQL->GetData(inter->sql_handle,  4, &data, NULL); m->gender = atoi(data);
		SQL->GetData(inter->sql_handle,  5, &data, NULL); m->class_ = atoi(data);
		SQL->GetData(inter->sql_handle,  6, &data, NULL); m->lv = atoi(data);
		SQL->GetData(inter->sql_handle,  7, &data, NULL); m->exp = strtoull(data, NULL, 10);
		SQL->GetData(inter->sql_handle,  8, &data, NULL); m->exp_payper = (unsigned int)atoi(data);
		SQL->GetData(inter->sql_handle,  9, &data, NULL); m->online = atoi(data);
		SQL->GetData(inter->sql_handle, 10, &data, NULL); m->position = atoi(data);
		if( m->position >= MAX_GUILDPOSITION ) // Fix reduction of MAX_GUILDPOSITION [PoW]
			m->position = MAX_GUILDPOSITION - 1;
		SQL->GetData(inter->sql_handle, 11, &data, &len); memcpy(m->name, data, min(len, NAME_LENGTH));
		m->modified = GS_MEMBER_UNMODIFIED;
	}

	//printf("- Read guild_position %d from sql \n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `position`,`name`,`mode`,`exp_mode` FROM `%s` WHERE `guild_id`='%d'", guild_position_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		aFree(g);
		return NULL;
	}
	while( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		int position;
		struct guild_position *pos;

		SQL->GetData(inter->sql_handle, 0, &data, NULL); position = atoi(data);
		if( position < 0 || position >= MAX_GUILDPOSITION )
			continue;// invalid position
		pos = &g->position[position];
		SQL->GetData(inter->sql_handle, 1, &data, &len); memcpy(pos->name, data, min(len, NAME_LENGTH));
		SQL->GetData(inter->sql_handle, 2, &data, NULL); pos->mode = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL); pos->exp_mode = atoi(data);
		pos->modified = GS_POSITION_UNMODIFIED;
	}

	//printf("- Read guild_alliance %d from sql \n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `opposition`,`alliance_id`,`name` FROM `%s` WHERE `guild_id`='%d'", guild_alliance_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < MAX_GUILDALLIANCE && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		struct guild_alliance* a = &g->alliance[i];

		SQL->GetData(inter->sql_handle, 0, &data, NULL); a->opposition = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); a->guild_id = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, &len); memcpy(a->name, data, min(len, NAME_LENGTH));
	}

	//printf("- Read guild_expulsion %d from sql \n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`,`name`,`mes` FROM `%s` WHERE `guild_id`='%d'", guild_expulsion_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < MAX_GUILDEXPULSION && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		struct guild_expulsion *e = &g->expulsion[i];

		SQL->GetData(inter->sql_handle, 0, &data, NULL); e->account_id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, &len); memcpy(e->name, data, min(len, NAME_LENGTH));
		SQL->GetData(inter->sql_handle, 2, &data, &len); memcpy(e->mes, data, min(len, sizeof(e->mes)));
	}

	//printf("- Read guild_skill %d from sql \n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `id`,`lv` FROM `%s` WHERE `guild_id`='%d' ORDER BY `id`", guild_skill_db, guild_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		aFree(g);
		return NULL;
	}

	for (i = 0; i < MAX_GUILDSKILL; i++) {
		//Skill IDs must always be initialized. [Skotlex]
		g->skill[i].id = i + GD_SKILLBASE;
	}

	while( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		int id;
		SQL->GetData(inter->sql_handle, 0, &data, NULL); id = atoi(data) - GD_SKILLBASE;
		if( id < 0 || id >= MAX_GUILDSKILL )
			continue;// invalid guild skill
		SQL->GetData(inter->sql_handle, 1, &data, NULL); g->skill[id].lv = atoi(data);
	}
	SQL->FreeResult(inter->sql_handle);

	idb_put(inter_guild->guild_db, guild_id, g); //Add to cache
	g->save_flag |= GS_REMOVE; //But set it to be removed, in case it is not needed for long.

	if (save_log)
		ShowInfo("Guild loaded (%d - %s)\n", guild_id, g->name);

	return g;
}

// `guild_castle` (`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`)
int inter_guild_castle_tosql(struct guild_castle *gc)
{
	StringBuf buf;
	int i;

	nullpo_ret(gc);
	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "REPLACE INTO `%s` SET `castle_id`='%d', `guild_id`='%d', `economy`='%d', `defense`='%d', "
	                 "`triggerE`='%d', `triggerD`='%d', `nextTime`='%d', `payTime`='%d', `createTime`='%d', `visibleC`='%d'",
	                 guild_castle_db, gc->castle_id, gc->guild_id, gc->economy, gc->defense,
	                 gc->triggerE, gc->triggerD, gc->nextTime, gc->payTime, gc->createTime, gc->visibleC);
	for (i = 0; i < MAX_GUARDIANS; ++i)
		StrBuf->Printf(&buf, ", `visibleG%d`='%d'", i, gc->guardian[i].visible);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)))
		Sql_ShowDebug(inter->sql_handle);
	else if(save_log)
		ShowInfo("Saved guild castle (%d)\n", gc->castle_id);

	StrBuf->Destroy(&buf);
	return 0;
}

// Read guild_castle from SQL
struct guild_castle* inter_guild_castle_fromsql(int castle_id)
{
	char *data;
	int i;
	StringBuf buf;
	struct guild_castle *gc = idb_get(inter_guild->castle_db, castle_id);

	if (gc != NULL)
		return gc;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, "
	                    "`triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`");
	for (i = 0; i < MAX_GUARDIANS; ++i)
		StrBuf->Printf(&buf, ", `visibleG%d`", i);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `castle_id`='%d'", guild_castle_db, castle_id);
	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return NULL;
	}
	StrBuf->Destroy(&buf);

	CREATE(gc, struct guild_castle, 1);
	gc->castle_id = castle_id;

	if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
		SQL->GetData(inter->sql_handle, 1, &data, NULL); gc->guild_id =  atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); gc->economy = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL); gc->defense = atoi(data);
		SQL->GetData(inter->sql_handle, 4, &data, NULL); gc->triggerE = atoi(data);
		SQL->GetData(inter->sql_handle, 5, &data, NULL); gc->triggerD = atoi(data);
		SQL->GetData(inter->sql_handle, 6, &data, NULL); gc->nextTime = atoi(data);
		SQL->GetData(inter->sql_handle, 7, &data, NULL); gc->payTime = atoi(data);
		SQL->GetData(inter->sql_handle, 8, &data, NULL); gc->createTime = atoi(data);
		SQL->GetData(inter->sql_handle, 9, &data, NULL); gc->visibleC = atoi(data);
		for (i = 10; i < 10+MAX_GUARDIANS; i++) {
			SQL->GetData(inter->sql_handle, i, &data, NULL); gc->guardian[i-10].visible = atoi(data);
		}
	}
	SQL->FreeResult(inter->sql_handle);

	idb_put(inter_guild->castle_db, castle_id, gc);

	if (save_log)
		ShowInfo("Loaded guild castle (%d - guild %d)\n", castle_id, gc->guild_id);

	return gc;
}


// Read exp_guild.txt
bool inter_guild_exp_parse_row(char* split[], int column, int current) {
	int64 exp = strtoll(split[0], NULL, 10);
	nullpo_retr(true, split);

	if (exp < 0 || exp >= UINT_MAX) {
		ShowError("exp_guild: Invalid exp %"PRId64" (valid range: 0 - %u) at line %d\n", exp, UINT_MAX, current);
		return false;
	}

	inter_guild->exp[current] = (unsigned int)exp;
	return true;
}


int inter_guild_CharOnline(int char_id, int guild_id) {
	struct guild *g;
	int i;

	if (guild_id == -1) {
		//Get guild_id from the database
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT guild_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			return 0;
		}

		if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
		{
			char* data;

			SQL->GetData(inter->sql_handle, 0, &data, NULL);
			guild_id = atoi(data);
		}
		else
		{
			guild_id = 0;
		}
		SQL->FreeResult(inter->sql_handle);
	}
	if (guild_id == 0)
		return 0; //No guild...

	g = inter_guild->fromsql(guild_id);
	if(!g) {
		ShowError("Character %d's guild %d not found!\n", char_id, guild_id);
		return 0;
	}

	//Member has logged in before saving, tell saver not to delete
	if(g->save_flag & GS_REMOVE)
		g->save_flag &= ~GS_REMOVE;

	//Set member online
	ARR_FIND( 0, g->max_member, i, g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = 1;
		g->member[i].modified = GS_MEMBER_MODIFIED;
	}

	return 1;
}

int inter_guild_CharOffline(int char_id, int guild_id)
{
	struct guild *g=NULL;
	int online_count, i;

	if (guild_id == -1)
	{
		//Get guild_id from the database
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT guild_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			return 0;
		}

		if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
		{
			char* data;

			SQL->GetData(inter->sql_handle, 0, &data, NULL);
			guild_id = atoi(data);
		}
		else
		{
			guild_id = 0;
		}
		SQL->FreeResult(inter->sql_handle);
	}
	if (guild_id == 0)
		return 0; //No guild...

	//Character has a guild, set character offline and check if they were the only member online
	g = inter_guild->fromsql(guild_id);
	if (g == NULL) //Guild not found?
		return 0;

	//Set member offline
	ARR_FIND( 0, g->max_member, i, g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = 0;
		g->member[i].modified = GS_MEMBER_MODIFIED;
	}

	online_count = 0;
	for( i = 0; i < g->max_member; i++ )
		if( g->member[i].online )
			online_count++;

	// Remove guild from memory if no players online
	if( online_count == 0 )
		g->save_flag |= GS_REMOVE;

	return 1;
}

// Initialize guild sql
int inter_guild_sql_init(void)
{
	//Initialize the guild cache
	inter_guild->guild_db= idb_alloc(DB_OPT_RELEASE_DATA);
	inter_guild->castle_db = idb_alloc(DB_OPT_RELEASE_DATA);

	//Read exp file
	sv->readdb("db", DBPATH"exp_guild.txt", ',', 1, 1, MAX_GUILDLEVEL, inter_guild->exp_parse_row);

	timer->add_func_list(inter_guild->save_timer, "inter_guild->save_timer");
	timer->add(timer->gettick() + 10000, inter_guild->save_timer, 0, 0);
	return 0;
}

/**
 * @see DBApply
 */
int inter_guild_db_final(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild *g = DB->data2ptr(data);
	nullpo_ret(g);
	if (g->save_flag&GS_MASK) {
		inter_guild->tosql(g, g->save_flag&GS_MASK);
		return 1;
	}
	return 0;
}

void inter_guild_sql_final(void)
{
	inter_guild->guild_db->destroy(inter_guild->guild_db, inter_guild->db_final);
	db_destroy(inter_guild->castle_db);
	return;
}

// Get guild_id by its name. Returns 0 if not found, -1 on error.
int inter_guild_search_guildname(const char *str)
{
	int guild_id;
	char esc_name[NAME_LENGTH*2+1];

	nullpo_retr(-1, str);
	SQL->EscapeStringLen(inter->sql_handle, esc_name, str, safestrnlen(str, NAME_LENGTH));
	//Lookup guilds with the same name
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT guild_id FROM `%s` WHERE name='%s'", guild_db, esc_name) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return -1;
	}

	if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		char* data;

		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		guild_id = atoi(data);
	}
	else
	{
		guild_id = 0;
	}
	SQL->FreeResult(inter->sql_handle);
	return guild_id;
}

// Check if guild is empty
static bool inter_guild_check_empty(struct guild *g)
{
	int i;
	nullpo_ret(g);
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id > 0 );
	if( i < g->max_member)
		return false; // not empty

	//Let the calling function handle the guild removal in case they need
	//to do something else with it before freeing the data. [Skotlex]
	return true;
}

unsigned int inter_guild_nextexp(int level) {
	if (level == 0)
		return 1;
	if (level <= 0 || level > MAX_GUILDLEVEL)
		return 0;

	return inter_guild->exp[level-1];
}

int inter_guild_checkskill(struct guild *g, int id)
{
	int idx = id - GD_SKILLBASE;

	nullpo_ret(g);
	if(idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	return g->skill[idx].lv;
}

int inter_guild_calcinfo(struct guild *g)
{
	int i,c;
	unsigned int nextexp;
	struct guild before = *g; // Save guild current values

	nullpo_ret(g);
	if(g->guild_lv<=0)
		g->guild_lv = 1;
	nextexp = inter_guild->nextexp(g->guild_lv);

	// Consume guild exp and increase guild level
	while(g->exp >= nextexp && nextexp > 0) { // nextexp would be 0 if g->guild_lv was >= MAX_GUILDLEVEL
		g->exp-=nextexp;
		g->guild_lv++;
		g->skill_point++;
		nextexp = inter_guild->nextexp(g->guild_lv);
	}

	// Save next exp step
	g->next_exp = nextexp;

	// Set the max storage size
#ifdef OFFICIAL_GUILD_STORAGE
	g->max_storage = inter_guild->checkskill(g, GD_GUILD_STORAGE)*100;
#else // ! OFFICIAL_GUILD_STORAGE
	g->max_storage = MAX_GUILD_STORAGE;
#endif // OFFICIAL_GUILD_STORAGE

	// Set the max number of members, Guild Extension skill - currently adds 6 to max per skill lv.
	g->max_member = BASE_GUILD_SIZE + inter_guild->checkskill(g, GD_EXTENSION) * 6;
	if(g->max_member > MAX_GUILD)
	{
		ShowError("Guild %d:%s has capacity for too many guild members (%d), max supported is %d\n", g->guild_id, g->name, g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}

	// Compute the guild average level level
	g->average_lv=0;
	g->connect_member=0;
	for(i=c=0;i<g->max_member;i++)
	{
		if(g->member[i].account_id>0)
		{
			if (g->member[i].lv >= 0)
			{
				g->average_lv+=g->member[i].lv;
				c++;
			}
			else
			{
				ShowWarning("Guild %d:%s, member %d:%s has an invalid level %d\n", g->guild_id, g->name, g->member[i].char_id, g->member[i].name, g->member[i].lv);
			}

			if(g->member[i].online)
				g->connect_member++;
		}
	}
	if(c)
		g->average_lv /= c;

	// Check if guild stats has change
	if (g->max_member != before.max_member
	 || g->guild_lv != before.guild_lv
	 || g->skill_point != before.skill_point
	 || g->max_storage != before.max_storage
	) {
		g->save_flag |= GS_LEVEL;
		mapif->guild_info(-1,g);
		return 1;
	}

	return 0;
}

//-------------------------------------------------------------------
// Packet sent to map server

int mapif_guild_created(int fd, int account_id, struct guild *g)
{
	WFIFOHEAD(fd, 10);
	WFIFOW(fd,0)=0x3830;
	WFIFOL(fd,2)=account_id;
	if(g != NULL)
	{
		WFIFOL(fd,6)=g->guild_id;
		ShowInfo("int_guild: Guild created (%d - %s)\n",g->guild_id,g->name);
	} else
		WFIFOL(fd,6)=0;

	WFIFOSET(fd,10);
	return 0;
}

// Guild not found
int mapif_guild_noinfo(int fd, int guild_id)
{
	unsigned char buf[12];
	WBUFW(buf,0)=0x3831;
	WBUFW(buf,2)=8;
	WBUFL(buf,4)=guild_id;
	ShowWarning("int_guild: info not found %d\n",guild_id);
	if(fd<0)
		mapif->sendall(buf,8);
	else
		mapif->send(fd,buf,8);
	return 0;
}

// Send guild info
int mapif_guild_info(int fd, struct guild *g)
{
	unsigned char buf[8+sizeof(struct guild)];
	nullpo_ret(g);
	WBUFW(buf,0)=0x3831;
	WBUFW(buf,2)=4+sizeof(struct guild);
	memcpy(buf+4,g,sizeof(struct guild));
	if(fd<0)
		mapif->sendall(buf,WBUFW(buf,2));
	else
		mapif->send(fd,buf,WBUFW(buf,2));
	return 0;
}

// ACK member add
int mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag)
{
	WFIFOHEAD(fd, 15);
	WFIFOW(fd,0)=0x3832;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=account_id;
	WFIFOL(fd,10)=char_id;
	WFIFOB(fd,14)=flag;
	WFIFOSET(fd,15);
	return 0;
}

// ACK member leave
int mapif_guild_withdraw(int guild_id,int account_id,int char_id,int flag, const char *name, const char *mes)
{
	unsigned char buf[55+NAME_LENGTH];

	nullpo_ret(name);
	nullpo_ret(mes);

	WBUFW(buf, 0)=0x3834;
	WBUFL(buf, 2)=guild_id;
	WBUFL(buf, 6)=account_id;
	WBUFL(buf,10)=char_id;
	WBUFB(buf,14)=flag;
	memcpy(WBUFP(buf,15),mes,40);
	memcpy(WBUFP(buf,55),name,NAME_LENGTH);
	mapif->sendall(buf,55+NAME_LENGTH);
	ShowInfo("int_guild: guild withdraw (%d - %d: %s - %s)\n",guild_id,account_id,name,mes);
	return 0;
}

// Send short member's info
int mapif_guild_memberinfoshort(struct guild *g, int idx)
{
	unsigned char buf[19];
	nullpo_ret(g);
	Assert_ret(idx >= 0 && idx < MAX_GUILD);
	WBUFW(buf, 0)=0x3835;
	WBUFL(buf, 2)=g->guild_id;
	WBUFL(buf, 6)=g->member[idx].account_id;
	WBUFL(buf,10)=g->member[idx].char_id;
	WBUFB(buf,14)=(unsigned char)g->member[idx].online;
	WBUFW(buf,15)=g->member[idx].lv;
	WBUFW(buf,17)=g->member[idx].class_;
	mapif->sendall(buf,19);
	return 0;
}

// Send guild broken
int mapif_guild_broken(int guild_id, int flag)
{
	unsigned char buf[7];
	WBUFW(buf,0)=0x3836;
	WBUFL(buf,2)=guild_id;
	WBUFB(buf,6)=flag;
	mapif->sendall(buf,7);
	ShowInfo("int_guild: Guild broken (%d)\n",guild_id);
	return 0;
}

// Send guild message
int mapif_guild_message(int guild_id, int account_id, const char *mes, int len, int sfd)
{
	unsigned char buf[512];
	nullpo_ret(mes);
	if (len > 500)
		len = 500;
	WBUFW(buf,0)=0x3837;
	WBUFW(buf,2)=len+12;
	WBUFL(buf,4)=guild_id;
	WBUFL(buf,8)=account_id;
	memcpy(WBUFP(buf,12),mes,len);
	mapif->sendallwos(sfd, buf,len+12);
	return 0;
}

// Send basic info
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len)
{
	unsigned char buf[2048];
	nullpo_ret(data);
	if (len > 2038)
		len = 2038;
	WBUFW(buf, 0)=0x3839;
	WBUFW(buf, 2)=len+10;
	WBUFL(buf, 4)=guild_id;
	WBUFW(buf, 8)=type;
	memcpy(WBUFP(buf,10),data,len);
	mapif->sendall(buf,len+10);
	return 0;
}

// Send member info
int mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len)
{
	unsigned char buf[2048];
	nullpo_ret(data);
	if (len > 2030)
		len = 2030;
	WBUFW(buf, 0)=0x383a;
	WBUFW(buf, 2)=len+18;
	WBUFL(buf, 4)=guild_id;
	WBUFL(buf, 8)=account_id;
	WBUFL(buf,12)=char_id;
	WBUFW(buf,16)=type;
	memcpy(WBUFP(buf,18),data,len);
	mapif->sendall(buf,len+18);
	return 0;
}

// ACK guild skill up
int mapif_guild_skillupack(int guild_id, uint16 skill_id, int account_id)
{
	unsigned char buf[14];
	WBUFW(buf, 0)=0x383c;
	WBUFL(buf, 2)=guild_id;
	WBUFL(buf, 6)=skill_id;
	WBUFL(buf,10)=account_id;
	mapif->sendall(buf,14);
	return 0;
}

// ACK guild alliance
int mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2)
{
	unsigned char buf[19+2*NAME_LENGTH];
	nullpo_ret(name1);
	nullpo_ret(name2);
	WBUFW(buf, 0)=0x383d;
	WBUFL(buf, 2)=guild_id1;
	WBUFL(buf, 6)=guild_id2;
	WBUFL(buf,10)=account_id1;
	WBUFL(buf,14)=account_id2;
	WBUFB(buf,18)=flag;
	memcpy(WBUFP(buf,19),name1,NAME_LENGTH);
	memcpy(WBUFP(buf,19+NAME_LENGTH),name2,NAME_LENGTH);
	mapif->sendall(buf,19+2*NAME_LENGTH);
	return 0;
}

// Send a guild position desc
int mapif_guild_position(struct guild *g, int idx)
{
	unsigned char buf[12 + sizeof(struct guild_position)];
	nullpo_ret(g);
	Assert_ret(idx >= 0 && idx < MAX_GUILDPOSITION);
	WBUFW(buf,0)=0x383b;
	WBUFW(buf,2)=sizeof(struct guild_position)+12;
	WBUFL(buf,4)=g->guild_id;
	WBUFL(buf,8)=idx;
	memcpy(WBUFP(buf,12),&g->position[idx],sizeof(struct guild_position));
	mapif->sendall(buf,WBUFW(buf,2));
	return 0;
}

// Send the guild notice
int mapif_guild_notice(struct guild *g)
{
	unsigned char buf[256];
	nullpo_ret(g);
	WBUFW(buf,0)=0x383e;
	WBUFL(buf,2)=g->guild_id;
	memcpy(WBUFP(buf,6),g->mes1,MAX_GUILDMES1);
	memcpy(WBUFP(buf,66),g->mes2,MAX_GUILDMES2);
	mapif->sendall(buf,186);
	return 0;
}

// Send emblem data
int mapif_guild_emblem(struct guild *g)
{
	unsigned char buf[12 + sizeof(g->emblem_data)];
	nullpo_ret(g);
	WBUFW(buf,0)=0x383f;
	WBUFW(buf,2)=g->emblem_len+12;
	WBUFL(buf,4)=g->guild_id;
	WBUFL(buf,8)=g->emblem_id;
	memcpy(WBUFP(buf,12),g->emblem_data,g->emblem_len);
	mapif->sendall(buf,WBUFW(buf,2));
	return 0;
}

int mapif_guild_master_changed(struct guild *g, int aid, int cid)
{
	unsigned char buf[14];
	nullpo_ret(g);
	WBUFW(buf,0)=0x3843;
	WBUFL(buf,2)=g->guild_id;
	WBUFL(buf,6)=aid;
	WBUFL(buf,10)=cid;
	mapif->sendall(buf,14);
	return 0;
}

int mapif_guild_castle_dataload(int fd, int sz, const int *castle_ids)
{
	struct guild_castle *gc = NULL;
	int num = (sz - 4) / sizeof(int);
	int len = 4 + num * sizeof(*gc);
	int i;

	nullpo_ret(castle_ids);
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x3840;
	WFIFOW(fd, 2) = len;
	for (i = 0; i < num; i++) {
		gc = inter_guild->castle_fromsql(*(castle_ids++));
		memcpy(WFIFOP(fd, 4 + i * sizeof(*gc)), gc, sizeof(*gc));
	}
	WFIFOSET(fd, len);
	return 0;
}

//-------------------------------------------------------------------
// Packet received from map server


// Guild creation request
int mapif_parse_CreateGuild(int fd, int account_id, const char *name, const struct guild_member *master)
{
	struct guild *g;
	int i=0;
#ifdef NOISY
	ShowInfo("Creating Guild (%s)\n", name);
#endif
	nullpo_ret(name);
	nullpo_ret(master);
	if(inter_guild->search_guildname(name) != 0){
		ShowInfo("int_guild: guild with same name exists [%s]\n",name);
		mapif->guild_created(fd,account_id,NULL);
		return 0;
	}
	// Check Authorized letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorized
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) == NULL) {
				mapif->guild_created(fd,account_id,NULL);
				return 0;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) != NULL) {
				mapif->guild_created(fd,account_id,NULL);
				return 0;
			}
	}

	g = (struct guild *)aMalloc(sizeof(struct guild));
	memset(g,0,sizeof(struct guild));

	memcpy(g->name,name,NAME_LENGTH);
	memcpy(g->master,master->name,NAME_LENGTH);
	memcpy(&g->member[0],master,sizeof(struct guild_member));
	g->member[0].modified = GS_MEMBER_MODIFIED;

	// Set default positions
	g->position[0].mode = GPERM_ALL;
	strcpy(g->position[0].name,"GuildMaster");
	strcpy(g->position[MAX_GUILDPOSITION-1].name,"Newbie");
	g->position[0].modified = g->position[MAX_GUILDPOSITION-1].modified = GS_POSITION_MODIFIED;
	for(i=1;i<MAX_GUILDPOSITION-1;i++) {
		sprintf(g->position[i].name,"Position %d",i+1);
		g->position[i].modified = GS_POSITION_MODIFIED;
	}

	// Initialize guild property
	g->max_member = BASE_GUILD_SIZE;
	g->average_lv = master->lv;
	g->connect_member = 1;
	g->guild_lv = 1;
#ifdef OFFICIAL_GUILD_STORAGE
	g->max_storage = 0;
#else // ! OFFICIAL_GUILD_STORAGE
	g->max_storage = MAX_GUILD_STORAGE;
#endif // OFFICIAL_GUILD_STORAGE

	for(i=0;i<MAX_GUILDSKILL;i++)
		g->skill[i].id=i + GD_SKILLBASE;
	g->guild_id= -1; //Request to create guild.

	// Create the guild
	if (!inter_guild->tosql(g,GS_BASIC|GS_POSITION|GS_SKILL|GS_MEMBER)) {
		//Failed to Create guild....
		ShowError("Failed to create Guild %s (Guild Master: %s)\n", g->name, g->master);
		mapif->guild_created(fd,account_id,NULL);
		aFree(g);
		return 0;
	}
	ShowInfo("Created Guild %d - %s (Guild Master: %s)\n", g->guild_id, g->name, g->master);

	//Add to cache
	idb_put(inter_guild->guild_db, g->guild_id, g);

	// Report to client
	mapif->guild_created(fd,account_id,g);
	mapif->guild_info(fd,g);

	if(log_inter)
		inter->log("guild %s (id=%d) created by master %s (id=%d)\n",
			name, g->guild_id, master->name, master->account_id );

	return 0;
}

// Return guild info to client
int mapif_parse_GuildInfo(int fd, int guild_id)
{
	struct guild * g = inter_guild->fromsql(guild_id); //We use this because on start-up the info of castle-owned guilds is required. [Skotlex]
	if(g)
	{
		if (!inter_guild->calcinfo(g))
			mapif->guild_info(fd,g);
	}
	else
		mapif->guild_noinfo(fd,guild_id); // Failed to load info
	return 0;
}

// Add member to guild
int mapif_parse_GuildAddMember(int fd, int guild_id, const struct guild_member *m)
{
	struct guild * g;
	int i;

	nullpo_ret(m);
	g = inter_guild->fromsql(guild_id);
	if(g==NULL){
		mapif->guild_memberadded(fd,guild_id,m->account_id,m->char_id,1); // 1: Failed to add
		return 0;
	}

	// Find an empty slot
	for(i=0;i<g->max_member;i++)
	{
		if(g->member[i].account_id==0)
		{
			memcpy(&g->member[i],m,sizeof(struct guild_member));
			g->member[i].modified = (GS_MEMBER_NEW | GS_MEMBER_MODIFIED);
			mapif->guild_memberadded(fd,guild_id,m->account_id,m->char_id,0); // 0: success
			if (!inter_guild->calcinfo(g)) //Send members if it was not invoked.
				mapif->guild_info(-1,g);

			g->save_flag |= GS_MEMBER;
			if (g->save_flag&GS_REMOVE)
				g->save_flag&=~GS_REMOVE;
			return 0;
		}
	}

	mapif->guild_memberadded(fd,guild_id,m->account_id,m->char_id,1); // 1: Failed to add
	return 0;
}

// Delete member from guild
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes)
{
	int i;

	struct guild* g = inter_guild->fromsql(guild_id);
	if( g == NULL )
	{
		// Unknown guild, just update the player
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `guild_id`='0' WHERE `account_id`='%d' AND `char_id`='%d'", char_db, account_id, char_id) )
			Sql_ShowDebug(inter->sql_handle);
		// mapif->guild_withdraw(guild_id,account_id,char_id,flag,g->member[i].name,mes);
		return 0;
	}

	nullpo_ret(mes);
	// Find the member
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i == g->max_member )
	{
		//TODO
		return 0;
	}

	if (flag) {
		// Write expulsion reason
		// Find an empty slot
		int j;
		ARR_FIND( 0, MAX_GUILDEXPULSION, j, g->expulsion[j].account_id == 0 );
		if( j == MAX_GUILDEXPULSION )
		{
			// Expulsion list is full, flush the oldest one
			for( j = 0; j < MAX_GUILDEXPULSION - 1; j++ )
				g->expulsion[j] = g->expulsion[j+1];
			j = MAX_GUILDEXPULSION-1;
		}
		// Save the expulsion entry
		g->expulsion[j].account_id = account_id;
		safestrncpy(g->expulsion[j].name, g->member[i].name, NAME_LENGTH);
		safestrncpy(g->expulsion[j].mes, mes, 40);
	}

	mapif->guild_withdraw(guild_id,account_id,char_id,flag,g->member[i].name,mes);
	inter_guild->removemember_tosql(g->member[i].account_id,g->member[i].char_id);

	memset(&g->member[i],0,sizeof(struct guild_member));

	if( inter_guild->check_empty(g) )
		mapif->parse_BreakGuild(-1,guild_id); //Break the guild.
	else {
		//Update member info.
		if (!inter_guild->calcinfo(g))
			mapif->guild_info(fd,g);
		g->save_flag |= GS_EXPULSION;
	}

	return 0;
}

// Change member info
int mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_)
{
	// Could speed up by manipulating only guild_member
	struct guild * g;
	int i,sum,c;
	int prev_count, prev_alv;

	g = inter_guild->fromsql(guild_id);
	if(g==NULL)
		return 0;

	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = online;
		g->member[i].lv = lv;
		g->member[i].class_ = class_;
		g->member[i].modified = GS_MEMBER_MODIFIED;
		mapif->guild_memberinfoshort(g,i);
	}

	prev_count = g->connect_member;
	prev_alv = g->average_lv;

	g->average_lv = 0;
	g->connect_member = 0;
	c = 0;
	sum = 0;

	for( i = 0; i < g->max_member; i++ )
	{
		if( g->member[i].account_id > 0 )
		{
			sum += g->member[i].lv;
			c++;
		}
		if( g->member[i].online )
			g->connect_member++;
	}

	if( c ) // this check should always succeed...
	{
		g->average_lv = sum / c;
		if( g->connect_member != prev_count || g->average_lv != prev_alv )
			g->save_flag |= GS_CONNECT;
		if( g->save_flag & GS_REMOVE )
			g->save_flag &= ~GS_REMOVE;
	}
	g->save_flag |= GS_MEMBER; //Update guild member data
	return 0;
}

// BreakGuild
int mapif_parse_BreakGuild(int fd, int guild_id)
{
	struct guild * g;

	g = inter_guild->fromsql(guild_id);
	if(g==NULL)
		return 0;

	// Delete guild from sql
	//printf("- Delete guild %d from guild\n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_member_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_castle_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_storage_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d' OR `alliance_id` = '%d'", guild_alliance_db, guild_id, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_position_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_skill_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_expulsion_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	//printf("- Update guild %d of char\n",guild_id);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `guild_id`='0' WHERE `guild_id`='%d'", char_db, guild_id) )
		Sql_ShowDebug(inter->sql_handle);

	mapif->guild_broken(guild_id,0);

	if(log_inter)
		inter->log("guild %s (id=%d) broken\n",g->name,guild_id);

	//Remove the guild from memory. [Skotlex]
	idb_remove(inter_guild->guild_db, guild_id);
	return 0;
}

// Forward Guild message to others map servers
int mapif_parse_GuildMessage(int fd, int guild_id, int account_id, const char *mes, int len)
{
	return mapif->guild_message(guild_id,account_id,mes,len, fd);
}

/**
 * Changes basic guild information
 * The types are available in mmo.h::guild_basic_info
 **/
int mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const void *data, int len) {
	struct guild *g;
	struct guild_skill gd_skill;
	short value;
	g = inter_guild->fromsql(guild_id);

	if( g == NULL )
		return 0;

	nullpo_ret(data);
	switch(type) {
		case GBI_EXP:
			value = *((const int16 *)data);
			if( value < 0 && abs(value) > g->exp )
				return 0;
			g->exp += value;
			inter_guild->calcinfo(g);
			break;

		case GBI_GUILDLV:
			value = *((const int16 *)data);
			if (value > 0 && g->guild_lv + value <= MAX_GUILDLEVEL) {
				g->guild_lv += value;
				g->skill_point += value;
			} else if (value < 0 && g->guild_lv + value >= 1)
				g->guild_lv += value;
			break;

		case GBI_SKILLPOINT:
			value = *((const int16 *)data);
			if( g->skill_point+value < 0 )
				return 0;
			g->skill_point += value;
			break;

		case GBI_SKILLLV:
			gd_skill = *((const struct guild_skill*)data);
			memcpy(&(g->skill[(gd_skill.id - GD_SKILLBASE)]), &gd_skill, sizeof(gd_skill));
			if( !inter_guild->calcinfo(g) )
				mapif->guild_info(-1,g);
			g->save_flag |= GS_SKILL;
			mapif->guild_skillupack(g->guild_id, gd_skill.id, 0);
			break;

		default:
			ShowError("int_guild: GuildBasicInfoChange: Unknown type %d, see mmo.h::guild_basic_info for more information\n",type);
			return 0;
	}
	mapif->guild_info(-1,g);
	g->save_flag |= GS_LEVEL;
	// Information is already sent in mapif->guild_info
	//mapif->guild_basicinfochanged(guild_id,type,data,len);
	return 0;
}

// Modification of the guild
int mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len)
{
	// Could make some improvement in speed, because only change guild_member
	int i;
	struct guild * g;

	nullpo_ret(data);
	g = inter_guild->fromsql(guild_id);
	if(g==NULL)
		return 0;

	// Search the member
	for (i = 0; i < g->max_member; i++) {
		if (g->member[i].account_id == account_id && g->member[i].char_id==char_id)
			break;
	}

	// Not Found
	if(i==g->max_member){
		ShowWarning("int_guild: GuildMemberChange: Not found %d,%d in guild (%d - %s)\n",
			account_id,char_id,guild_id,g->name);
		return 0;
	}

	switch(type)
	{
		case GMI_POSITION:
		{
			g->member[i].position = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER;
			break;
		}
		case GMI_EXP:
		{
			uint64 old_exp = g->member[i].exp;
			g->member[i].exp = *(const uint64 *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			if (g->member[i].exp > old_exp) {
				uint64 exp = g->member[i].exp - old_exp;

				// Compute gained exp
				if (guild_exp_rate != 100)
					exp = exp*guild_exp_rate/100;

				// Update guild exp
				if (exp > UINT64_MAX - g->exp)
					g->exp = UINT64_MAX;
				else
					g->exp+=exp;

				inter_guild->calcinfo(g);
				mapif->guild_basicinfochanged(guild_id,GBI_EXP,&g->exp,sizeof(g->exp));
				g->save_flag |= GS_LEVEL;
			}
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER;
			break;
		}
		case GMI_HAIR:
		{
			g->member[i].hair = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER; //Save new data.
			break;
		}
		case GMI_HAIR_COLOR:
		{
			g->member[i].hair_color = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER; //Save new data.
			break;
		}
		case GMI_GENDER:
		{
			g->member[i].gender = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER; //Save new data.
			break;
		}
		case GMI_CLASS:
		{
			g->member[i].class_ = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER; //Save new data.
			break;
		}
		case GMI_LEVEL:
		{
			g->member[i].lv = *(const short *)data;
			g->member[i].modified = GS_MEMBER_MODIFIED;
			mapif->guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
			g->save_flag |= GS_MEMBER; //Save new data.
			break;
		}
		default:
		  ShowError("int_guild: GuildMemberInfoChange: Unknown type %d\n",type);
		  break;
	}
	return 0;
}

int inter_guild_sex_changed(int guild_id, int account_id, int char_id, short gender)
{
	return mapif->parse_GuildMemberInfoChange(0, guild_id, account_id, char_id, GMI_GENDER, (const char*)&gender, sizeof(gender));
}

int inter_guild_charname_changed(int guild_id, int account_id, int char_id, char *name)
{
	struct guild *g;
	int i, flag = 0;

	nullpo_ret(name);
	g = inter_guild->fromsql(guild_id);
	if( g == NULL )
	{
		ShowError("inter_guild_charrenamed: Can't find guild %d.\n", guild_id);
		return 0;
	}

	ARR_FIND(0, g->max_member, i, g->member[i].char_id == char_id);
	if( i == g->max_member )
	{
		ShowError("inter_guild_charrenamed: Can't find character %d in the guild\n", char_id);
		return 0;
	}

	if( !strcmp(g->member[i].name, g->master) )
	{
		safestrncpy(g->master, name, NAME_LENGTH);
		flag |= GS_BASIC;
	}
	safestrncpy(g->member[i].name, name, NAME_LENGTH);
	g->member[i].modified = GS_MEMBER_MODIFIED;
	flag |= GS_MEMBER;

	if( !inter_guild->tosql(g, flag) )
		return 0;

	mapif->guild_info(-1,g);

	return 0;
}

// Change a position desc
int mapif_parse_GuildPosition(int fd, int guild_id, int idx, const struct guild_position *p)
{
	// Could make some improvement in speed, because only change guild_position
	struct guild * g;

	nullpo_ret(p);
	g = inter_guild->fromsql(guild_id);
	if(g==NULL || idx<0 || idx>=MAX_GUILDPOSITION)
		return 0;

	memcpy(&g->position[idx],p,sizeof(struct guild_position));
	mapif->guild_position(g,idx);
	g->position[idx].modified = GS_POSITION_MODIFIED;
	g->save_flag |= GS_POSITION; // Change guild_position
	return 0;
}

// Guild Skill UP
int mapif_parse_GuildSkillUp(int fd, int guild_id, uint16 skill_id, int account_id, int max)
{
	struct guild * g;
	int idx = skill_id - GD_SKILLBASE;

	g = inter_guild->fromsql(guild_id);
	if(g == NULL || idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	if(g->skill_point>0 && g->skill[idx].id>0 && g->skill[idx].lv<max )
	{
		g->skill[idx].lv++;
		g->skill_point--;
		if (!inter_guild->calcinfo(g))
			mapif->guild_info(-1,g);
		mapif->guild_skillupack(guild_id,skill_id,account_id);
		g->save_flag |= (GS_LEVEL|GS_SKILL); // Change guild & guild_skill
	}
	return 0;
}

//Manual deletion of an alliance when partnering guild does not exists. [Skotlex]
int mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag)
{
	int i;
	char name[NAME_LENGTH];

	nullpo_retr(-1, g);
	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->alliance[i].guild_id == guild_id );
	if( i == MAX_GUILDALLIANCE )
		return -1;

	strcpy(name, g->alliance[i].name);
	g->alliance[i].guild_id=0;

	mapif->guild_alliance(g->guild_id,guild_id,account_id1,account_id2,flag,g->name,name);
	g->save_flag |= GS_ALLIANCE;
	return 0;
}

// Alliance modification
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag)
{
	// Could speed up
	struct guild *g[2] = { NULL };
	int j,i;
	g[0] = inter_guild->fromsql(guild_id1);
	g[1] = inter_guild->fromsql(guild_id2);

	if(g[0] && g[1]==NULL && (flag & GUILD_ALLIANCE_REMOVE)) //Requested to remove an alliance with a not found guild.
		return mapif->parse_GuildDeleteAlliance(g[0], guild_id2, account_id1, account_id2, flag); //Try to do a manual removal of said guild.

	if(g[0]==NULL || g[1]==NULL)
		return 0;

	if( flag&GUILD_ALLIANCE_REMOVE ) {
		// Remove alliance/opposition, in case of alliance, remove on both side
		for( i = 0; i < ((flag&GUILD_ALLIANCE_TYPE_MASK) ? 1 : 2); i++ ) {
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == g[1-i]->guild_id && g[i]->alliance[j].opposition == (flag&GUILD_ALLIANCE_TYPE_MASK) );
			if( j < MAX_GUILDALLIANCE )
				g[i]->alliance[j].guild_id = 0;
		}
	} else {
		// Add alliance, in case of alliance, add on both side
		for( i = 0; i < ((flag&GUILD_ALLIANCE_TYPE_MASK) ? 1 : 2); i++ ) {
			// Search an empty slot
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == 0 );
			if( j < MAX_GUILDALLIANCE ) {
				g[i]->alliance[j].guild_id=g[1-i]->guild_id;
				memcpy(g[i]->alliance[j].name,g[1-i]->name,NAME_LENGTH);
				// Set alliance type
				g[i]->alliance[j].opposition = flag&GUILD_ALLIANCE_TYPE_MASK;
			}
		}
	}

	// Send on all map the new alliance/opposition
	mapif->guild_alliance(guild_id1,guild_id2,account_id1,account_id2,flag,g[0]->name,g[1]->name);

	// Mark the two guild to be saved
	g[0]->save_flag |= GS_ALLIANCE;
	g[1]->save_flag |= GS_ALLIANCE;
	return 0;
}

// Change guild message
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	struct guild *g;

	nullpo_ret(mes1);
	nullpo_ret(mes2);
	g = inter_guild->fromsql(guild_id);
	if(g==NULL)
		return 0;

	memcpy(g->mes1,mes1,MAX_GUILDMES1);
	memcpy(g->mes2,mes2,MAX_GUILDMES2);
	g->save_flag |= GS_MES; //Change mes of guild
	return mapif->guild_notice(g);
}

int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	struct guild * g;

	nullpo_ret(data);
	g = inter_guild->fromsql(guild_id);
	if(g==NULL)
		return 0;

	if (len > sizeof(g->emblem_data))
		len = sizeof(g->emblem_data);

	memcpy(g->emblem_data,data,len);
	g->emblem_len=len;
	g->emblem_id++;
	g->save_flag |= GS_EMBLEM; //Change guild
	return mapif->guild_emblem(g);
}

int mapif_parse_GuildCastleDataLoad(int fd, int len, const int *castle_ids)
{
	return mapif->guild_castle_dataload(fd, len, castle_ids);
}

int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
	struct guild_castle *gc = inter_guild->castle_fromsql(castle_id);

	if (gc == NULL) {
		ShowError("mapif->parse_GuildCastleDataSave: castle id=%d not found\n", castle_id);
		return 0;
	}

	switch (index) {
		case 1:
			if (log_inter && gc->guild_id != value) {
				int gid = (value) ? value : gc->guild_id;
				struct guild *g = idb_get(inter_guild->guild_db, gid);
				inter->log("guild %s (id=%d) %s castle id=%d\n",
				          (g) ? g->name : "??", gid, (value) ? "occupy" : "abandon", castle_id);
			}
			gc->guild_id = value;
			break;
		case 2: gc->economy = value; break;
		case 3: gc->defense = value; break;
		case 4: gc->triggerE = value; break;
		case 5: gc->triggerD = value; break;
		case 6: gc->nextTime = value; break;
		case 7: gc->payTime = value; break;
		case 8: gc->createTime = value; break;
		case 9: gc->visibleC = value; break;
		default:
			if (index > 9 && index <= 9+MAX_GUARDIANS) {
				gc->guardian[index-10].visible = value;
				break;
			}
			ShowError("mapif->parse_GuildCastleDataSave: not found index=%d\n", index);
			return 0;
	}
	inter_guild->castle_tosql(gc);
	return 0;
}

int mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len)
{
	struct guild * g;
	struct guild_member gm;
	int pos;

	nullpo_ret(name);
	g = inter_guild->fromsql(guild_id);

	if(g==NULL || len > NAME_LENGTH)
		return 0;

	// Find member (name)
	for (pos = 0; pos < g->max_member && strncmp(g->member[pos].name, name, len); pos++);

	if (pos == g->max_member)
		return 0; //Character not found??

	// Switch current and old GM
	memcpy(&gm, &g->member[pos], sizeof (struct guild_member));
	memcpy(&g->member[pos], &g->member[0], sizeof(struct guild_member));
	memcpy(&g->member[0], &gm, sizeof(struct guild_member));

	// Switch positions
	g->member[pos].position = g->member[0].position;
	g->member[pos].modified = GS_MEMBER_MODIFIED;
	g->member[0].position = 0; //Position 0: guild Master.
	g->member[0].modified = GS_MEMBER_MODIFIED;

	safestrncpy(g->master, name, len);
	if (len < NAME_LENGTH)
		g->master[len] = '\0';

	ShowInfo("int_guild: Guildmaster Changed to %s (Guild %d - %s)\n",g->master, guild_id, g->name);
	g->save_flag |= (GS_BASIC|GS_MEMBER); //Save main data and member data.
	return mapif->guild_master_changed(g, g->member[0].account_id, g->member[0].char_id);
}

// Communication from the map server
// - Can analyzed only one by one packet
// Data packet length that you set to inter.c
//- Shouldn't do checking and packet length, RFIFOSKIP is done by the caller
// Must Return
//  1 : ok
//  0 : error
int inter_guild_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif->parse_CreateGuild(fd, RFIFOL(fd,4), RFIFOP(fd,8), RFIFOP(fd,32)); break;
	case 0x3031: mapif->parse_GuildInfo(fd,RFIFOL(fd,2)); break;
	case 0x3032: mapif->parse_GuildAddMember(fd, RFIFOL(fd,4), RFIFOP(fd,8)); break;
	case 0x3033: mapif->parse_GuildMasterChange(fd, RFIFOL(fd,4), RFIFOP(fd,8), RFIFOW(fd,2)-8); break;
	case 0x3034: mapif->parse_GuildLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOP(fd,15)); break;
	case 0x3035: mapif->parse_GuildChangeMemberInfoShort(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17)); break;
	case 0x3036: mapif->parse_BreakGuild(fd,RFIFOL(fd,2)); break;
	case 0x3037: mapif->parse_GuildMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3039: mapif->parse_GuildBasicInfoChange(fd, RFIFOL(fd,4), RFIFOW(fd,8), RFIFOP(fd,10), RFIFOW(fd,2)-10); break;
	case 0x303A: mapif->parse_GuildMemberInfoChange(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOL(fd,12), RFIFOW(fd,16), RFIFOP(fd,18), RFIFOW(fd,2)-18); break;
	case 0x303B: mapif->parse_GuildPosition(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12)); break;
	case 0x303C: mapif->parse_GuildSkillUp(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14)); break;
	case 0x303D: mapif->parse_GuildAlliance(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),RFIFOB(fd,18)); break;
	case 0x303E: mapif->parse_GuildNotice(fd, RFIFOL(fd,2), RFIFOP(fd,6), RFIFOP(fd,66)); break;
	case 0x303F: mapif->parse_GuildEmblem(fd, RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12)); break;
	case 0x3040: mapif->parse_GuildCastleDataLoad(fd, RFIFOW(fd,2), RFIFOP(fd,4)); break;
	case 0x3041: mapif->parse_GuildCastleDataSave(fd,RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5)); break;

	default:
		return 0;
	}

	return 1;
}

//Leave request from the server (for deleting character from guild)
int inter_guild_leave(int guild_id, int account_id, int char_id)
{
	return mapif->parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "** Character Deleted **");
}

int inter_guild_broken(int guild_id)
{
	return mapif->guild_broken(guild_id, 0);
}

void inter_guild_defaults(void)
{
	inter_guild = &inter_guild_s;

	inter_guild->guild_db = NULL;
	inter_guild->castle_db = NULL;
	memset(inter_guild->exp, 0, sizeof(inter_guild->exp));

	inter_guild->save_timer = inter_guild_save_timer;
	inter_guild->removemember_tosql = inter_guild_removemember_tosql;
	inter_guild->tosql = inter_guild_tosql;
	inter_guild->fromsql = inter_guild_fromsql;
	inter_guild->castle_tosql = inter_guild_castle_tosql;
	inter_guild->castle_fromsql = inter_guild_castle_fromsql;
	inter_guild->exp_parse_row = inter_guild_exp_parse_row;
	inter_guild->CharOnline = inter_guild_CharOnline;
	inter_guild->CharOffline = inter_guild_CharOffline;
	inter_guild->sql_init = inter_guild_sql_init;
	inter_guild->db_final = inter_guild_db_final;
	inter_guild->sql_final = inter_guild_sql_final;
	inter_guild->search_guildname = inter_guild_search_guildname;
	inter_guild->check_empty = inter_guild_check_empty;
	inter_guild->nextexp = inter_guild_nextexp;
	inter_guild->checkskill = inter_guild_checkskill;
	inter_guild->calcinfo = inter_guild_calcinfo;
	inter_guild->sex_changed = inter_guild_sex_changed;
	inter_guild->charname_changed = inter_guild_charname_changed;
	inter_guild->parse_frommap = inter_guild_parse_frommap;
	inter_guild->leave = inter_guild_leave;
	inter_guild->broken = inter_guild_broken;
}
