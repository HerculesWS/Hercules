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

#include "config/core.h" // CONSOLE_INPUT
#include "char.h"

#include "char/HPMchar.h"
#include "char/geoip.h"
#include "char/int_auction.h"
#include "char/int_elemental.h"
#include "char/int_guild.h"
#include "char/int_homun.h"
#include "char/int_mail.h"
#include "char/int_mercenary.h"
#include "char/int_party.h"
#include "char/int_pet.h"
#include "char/int_quest.h"
#include "char/int_storage.h"
#include "char/inter.h"
#include "char/loginif.h"
#include "char/mapif.h"
#include "char/pincode.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sql.h"
#include "common/timer.h"
#include "common/utils.h"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if MAX_MAP_SERVERS > 1
#	ifdef _MSC_VER
#		pragma message("WARNING: your settings allow more than one map server to connect, this is deprecated dangerous feature USE IT AT YOUR OWN RISK")
#	else
#		warning your settings allow more than one map server to connect, this is deprecated dangerous feature USE IT AT YOUR OWN RISK
#	endif
#endif

// private declarations
char char_db[256] = "char";
char scdata_db[256] = "sc_data";
char cart_db[256] = "cart_inventory";
char inventory_db[256] = "inventory";
char charlog_db[256] = "charlog";
char storage_db[256] = "storage";
char interlog_db[256] = "interlog";
char skill_db[256] = "skill";
char memo_db[256] = "memo";
char guild_db[256] = "guild";
char guild_alliance_db[256] = "guild_alliance";
char guild_castle_db[256] = "guild_castle";
char guild_expulsion_db[256] = "guild_expulsion";
char guild_member_db[256] = "guild_member";
char guild_position_db[256] = "guild_position";
char guild_skill_db[256] = "guild_skill";
char guild_storage_db[256] = "guild_storage";
char party_db[256] = "party";
char pet_db[256] = "pet";
char mail_db[256] = "mail"; // MAIL SYSTEM
char auction_db[256] = "auction"; // Auctions System
char friend_db[256] = "friends";
char hotkey_db[256] = "hotkey";
char quest_db[256] = "quest";
char homunculus_db[256] = "homunculus";
char skill_homunculus_db[256] = "skill_homunculus";
char mercenary_db[256] = "mercenary";
char mercenary_owner_db[256] = "mercenary_owner";
char ragsrvinfo_db[256] = "ragsrvinfo";
char elemental_db[256] = "elemental";
char account_data_db[256] = "account_data";
char acc_reg_num_db[32] = "acc_reg_num_db";
char acc_reg_str_db[32] = "acc_reg_str_db";
char char_reg_str_db[32] = "char_reg_str_db";
char char_reg_num_db[32] = "char_reg_num_db";

struct char_interface char_s;
struct char_interface *chr;

// show loading/saving messages
int save_log = 1;

char db_path[1024] = "db";

char wisp_server_name[NAME_LENGTH] = "Server";
char login_ip_str[128];
uint32 login_ip = 0;
uint16 login_port = 6900;
char char_ip_str[128];
char bind_ip_str[128];
uint32 bind_ip = INADDR_ANY;
int char_maintenance_min_group_id = 0;
bool char_new = true;

bool name_ignoring_case = false; // Allow or not identical name for characters but with a different case by [Yor]
int char_name_option = 0; // Option to know which letters/symbols are authorized in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
char unknown_char_name[NAME_LENGTH] = "Unknown"; // Name to use when the requested name cannot be determined
#define TRIM_CHARS "\255\xA0\032\t\x0A\x0D " //The following characters are trimmed regardless because they cause confusion and problems on the servers. [Skotlex]
char char_name_letters[1024] = ""; // list of letters/symbols allowed (or not) in a character name. by [Yor]

int char_del_level = 0; //From which level u can delete character [Lupus]
int char_del_delay = 86400;

int log_char = 1;  // logging char or not [devil]
int log_inter = 1; // logging inter or not [devil]

int char_aegis_delete = 0; // Verify if char is in guild/party or char and reacts as Aegis does (doesn't allow deletion), see chr->delete2_req for more information

int max_connect_user = -1;
int gm_allow_group = -1;
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int start_zeny = 0;
int start_items[MAX_START_ITEMS*3];
int guild_exp_rate = 100;

//Custom limits for the fame lists. [Skotlex]
int fame_list_size_chemist = MAX_FAME_LIST;
int fame_list_size_smith   = MAX_FAME_LIST;
int fame_list_size_taekwon = MAX_FAME_LIST;

// Char-server-side stored fame lists [DracoRPG]
struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

// Initial position (it's possible to set it in conf file)
#ifdef RENEWAL
	struct point start_point = { 0, 97, 90 };
#else
	struct point start_point = { 0, 53, 111 };
#endif

unsigned short skillid2idx[MAX_SKILL_ID];

//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
#define AUTH_TIMEOUT 30000

static struct DBMap *auth_db; // int account_id -> struct char_auth_node*

//-----------------------------------------------------
// Online User Database
//-----------------------------------------------------

/**
 * @see DBCreateData
 */
static struct DBData char_create_online_char_data(union DBKey key, va_list args)
{
	struct online_char_data* character;
	CREATE(character, struct online_char_data, 1);
	character->account_id = key.i;
	character->char_id = -1;
	character->server = -1;
	character->pincode_enable = -1;
	character->fd = -1;
	character->waiting_disconnect = INVALID_TIMER;
	return DB->ptr2data(character);
}

void char_set_account_online(int account_id)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x272b;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOSET(chr->login_fd,6);
}

void char_set_account_offline(int account_id)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x272c;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOSET(chr->login_fd,6);
}

void char_set_char_charselect(int account_id)
{
	struct online_char_data* character;

	character = (struct online_char_data*)idb_ensure(chr->online_char_db, account_id, chr->create_online_char_data);

	if( character->server > -1 )
		if( chr->server[character->server].users > 0 ) // Prevent this value from going negative.
			chr->server[character->server].users--;

	character->char_id = -1;
	character->server = -1;
	if(character->pincode_enable == -1)
		character->pincode_enable = pincode->charselect + pincode->enabled;

	if(character->waiting_disconnect != INVALID_TIMER) {
		timer->delete(character->waiting_disconnect, chr->waiting_disconnect);
		character->waiting_disconnect = INVALID_TIMER;
	}

	if (chr->login_fd > 0 && !sockt->session[chr->login_fd]->flag.eof)
		chr->set_account_online(account_id);
}

void char_set_char_online(int map_id, int char_id, int account_id)
{
	struct online_char_data* character;
	struct mmo_charstatus *cp;

	//Update DB
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `online`='1' WHERE `char_id`='%d' LIMIT 1", char_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	//Check to see for online conflicts
	character = (struct online_char_data*)idb_ensure(chr->online_char_db, account_id, chr->create_online_char_data);
	if( character->char_id != -1 && character->server > -1 && character->server != map_id )
	{
		ShowNotice("chr->set_char_online: Character %d:%d marked in map server %d, but map server %d claims to have (%d:%d) online!\n",
			character->account_id, character->char_id, character->server, map_id, account_id, char_id);
		mapif->disconnectplayer(chr->server[character->server].fd, character->account_id, character->char_id, 2); // 2: Already connected to server
	}

	//Update state data
	character->char_id = char_id;
	character->server = map_id;

	if( character->server > -1 )
		chr->server[character->server].users++;

	//Get rid of disconnect timer
	if(character->waiting_disconnect != INVALID_TIMER) {
		timer->delete(character->waiting_disconnect, chr->waiting_disconnect);
		character->waiting_disconnect = INVALID_TIMER;
	}

	//Set char online in guild cache. If char is in memory, use the guild id on it, otherwise seek it.
	cp = (struct mmo_charstatus*)idb_get(chr->char_db_,char_id);
	inter_guild->CharOnline(char_id, cp?cp->guild_id:-1);

	//Notify login server
	if (chr->login_fd > 0 && !sockt->session[chr->login_fd]->flag.eof)
		chr->set_account_online(account_id);
}

void char_set_char_offline(int char_id, int account_id)
{
	struct online_char_data* character;

	if ( char_id == -1 )
	{
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `online`='0' WHERE `account_id`='%d'", char_db, account_id) )
			Sql_ShowDebug(inter->sql_handle);
	}
	else
	{
		struct mmo_charstatus* cp = (struct mmo_charstatus*)idb_get(chr->char_db_,char_id);
		inter_guild->CharOffline(char_id, cp?cp->guild_id:-1);
		if (cp)
			idb_remove(chr->char_db_,char_id);

		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `online`='0' WHERE `char_id`='%d' LIMIT 1", char_db, char_id) )
			Sql_ShowDebug(inter->sql_handle);
	}

	if ((character = (struct online_char_data*)idb_get(chr->online_char_db, account_id)) != NULL) {
		//We don't free yet to avoid aCalloc/aFree spamming during char change. [Skotlex]
		if( character->server > -1 )
			if( chr->server[character->server].users > 0 ) // Prevent this value from going negative.
				chr->server[character->server].users--;

		if(character->waiting_disconnect != INVALID_TIMER){
			timer->delete(character->waiting_disconnect, chr->waiting_disconnect);
			character->waiting_disconnect = INVALID_TIMER;
		}

		if(character->char_id == char_id)
		{
			character->char_id = -1;
			character->server = -1;
			character->pincode_enable = -1;
		}

		//FIXME? Why Kevin free'd the online information when the char was effectively in the map-server?
	}

	//Remove char if 1- Set all offline, or 2- character is no longer connected to char-server.
	if (chr->login_fd > 0 && !sockt->session[chr->login_fd]->flag.eof && (char_id == -1 || character == NULL || character->fd == -1))
		chr->set_account_offline(account_id);
}

/**
 * @see DBApply
 */
static int char_db_setoffline(union DBKey key, struct DBData *data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)DB->data2ptr(data);
	int server_id = va_arg(ap, int);
	nullpo_ret(character);
	if (server_id == -1) {
		character->char_id = -1;
		character->server = -1;
		if(character->waiting_disconnect != INVALID_TIMER){
			timer->delete(character->waiting_disconnect, chr->waiting_disconnect);
			character->waiting_disconnect = INVALID_TIMER;
		}
	} else if (character->server == server_id)
		character->server = -2; //In some map server that we aren't connected to.
	return 0;
}

/**
 * @see DBApply
 */
static int char_db_kickoffline(union DBKey key, struct DBData *data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)DB->data2ptr(data);
	int server_id = va_arg(ap, int);
	nullpo_ret(character);

	if (server_id > -1 && character->server != server_id)
		return 0;

	//Kick out any connected characters, and set them offline as appropriate.
	if (character->server > -1 && character->server < MAX_MAP_SERVERS)
		mapif->disconnectplayer(chr->server[character->server].fd, character->account_id, character->char_id, 1); // 1: Server closed
	else if (character->waiting_disconnect == INVALID_TIMER)
		chr->set_char_offline(character->char_id, character->account_id);
	else
		return 0; // fail

	return 1;
}

void char_set_login_all_offline(void)
{
	//Tell login-server to also mark all our characters as offline.
	WFIFOHEAD(chr->login_fd,2);
	WFIFOW(chr->login_fd,0) = 0x2737;
	WFIFOSET(chr->login_fd,2);
}

void char_set_all_offline(int id)
{
	if (id < 0)
		ShowNotice("Sending all users offline.\n");
	else
		ShowNotice("Sending users of map-server %d offline.\n",id);
	chr->online_char_db->foreach(chr->online_char_db,chr->db_kickoffline,id);

	if (id >= 0 || chr->login_fd <= 0 || sockt->session[chr->login_fd]->flag.eof)
		return;
	chr->set_login_all_offline();
}

void char_set_all_offline_sql(void)
{
	//Set all players to 'OFFLINE'
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `online` = '0'", char_db) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `online` = '0'", guild_member_db) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `connect_member` = '0'", guild_db) )
		Sql_ShowDebug(inter->sql_handle);
}

/**
 * @see DBCreateData
 */
static struct DBData char_create_charstatus(union DBKey key, va_list args)
{
	struct mmo_charstatus *cp;
	cp = (struct mmo_charstatus *) aCalloc(1,sizeof(struct mmo_charstatus));
	cp->char_id = key.i;
	return DB->ptr2data(cp);
}

int char_mmo_char_tosql(int char_id, struct mmo_charstatus* p)
{
	int i = 0;
	int count = 0;
	int diff = 0;
	char save_status[128]; //For displaying save information. [Skotlex]
	struct mmo_charstatus *cp;
	int errors = 0; //If there are any errors while saving, "cp" will not be updated at the end.
	StringBuf buf;

	nullpo_ret(p);
	if (char_id != p->char_id) return 0;

	cp = idb_ensure(chr->char_db_, char_id, chr->create_charstatus);

	StrBuf->Init(&buf);
	memset(save_status, 0, sizeof(save_status));

	//map inventory data
	if( memcmp(p->inventory, cp->inventory, sizeof(p->inventory)) ) {
		if (!chr->memitemdata_to_sql(p->inventory, MAX_INVENTORY, p->char_id, TABLE_INVENTORY))
			strcat(save_status, " inventory");
		else
			errors++;
	}

	//map cart data
	if( memcmp(p->cart, cp->cart, sizeof(p->cart)) ) {
		if (!chr->memitemdata_to_sql(p->cart, MAX_CART, p->char_id, TABLE_CART))
			strcat(save_status, " cart");
		else
			errors++;
	}

	//map storage data
	if( memcmp(p->storage.items, cp->storage.items, sizeof(p->storage.items)) ) {
		if (!chr->memitemdata_to_sql(p->storage.items, MAX_STORAGE, p->account_id, TABLE_STORAGE))
			strcat(save_status, " storage");
		else
			errors++;
	}

	if (
		(p->base_exp != cp->base_exp) || (p->base_level != cp->base_level) ||
		(p->job_level != cp->job_level) || (p->job_exp != cp->job_exp) ||
		(p->zeny != cp->zeny) ||
		(p->last_point.map != cp->last_point.map) ||
		(p->last_point.x != cp->last_point.x) || (p->last_point.y != cp->last_point.y) ||
		(p->max_hp != cp->max_hp) || (p->hp != cp->hp) ||
		(p->max_sp != cp->max_sp) || (p->sp != cp->sp) ||
		(p->status_point != cp->status_point) || (p->skill_point != cp->skill_point) ||
		(p->str != cp->str) || (p->agi != cp->agi) || (p->vit != cp->vit) ||
		(p->int_ != cp->int_) || (p->dex != cp->dex) || (p->luk != cp->luk) ||
		(p->option != cp->option) ||
		(p->party_id != cp->party_id) || (p->guild_id != cp->guild_id) ||
		(p->pet_id != cp->pet_id) || (p->weapon != cp->weapon) || (p->hom_id != cp->hom_id) ||
		(p->ele_id != cp->ele_id) || (p->shield != cp->shield) || (p->head_top != cp->head_top) ||
		(p->head_mid != cp->head_mid) || (p->head_bottom != cp->head_bottom) || (p->delete_date != cp->delete_date) ||
		(p->rename != cp->rename) || (p->slotchange != cp->slotchange) || (p->robe != cp->robe) ||
		(p->show_equip != cp->show_equip) || (p->allow_party != cp->allow_party) || (p->font != cp->font) ||
		(p->uniqueitem_counter != cp->uniqueitem_counter) || (p->hotkey_rowshift != cp->hotkey_rowshift)
	) {
		//Save status
		unsigned int opt = 0;

		if( p->allow_party )
			opt |= OPT_ALLOW_PARTY;
		if( p->show_equip )
			opt |= OPT_SHOW_EQUIP;

		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `base_level`='%u', `job_level`='%u',"
			"`base_exp`='%u', `job_exp`='%u', `zeny`='%d',"
			"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%u',`skill_point`='%u',"
			"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
			"`option`='%u',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',`homun_id`='%d',`elemental_id`='%d',"
			"`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
			"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d', `rename`='%d',"
			"`delete_date`='%lu',`robe`='%d',`slotchange`='%d', `char_opt`='%u', `font`='%u', `uniqueitem_counter` ='%u',"
			"`hotkey_rowshift`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			char_db, p->base_level, p->job_level,
			p->base_exp, p->job_exp, p->zeny,
			p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
			p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
			p->option, p->party_id, p->guild_id, p->pet_id, p->hom_id, p->ele_id,
			p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
			mapindex_id2name(p->last_point.map), p->last_point.x, p->last_point.y,
			mapindex_id2name(p->save_point.map), p->save_point.x, p->save_point.y, p->rename,
			(unsigned long)p->delete_date,  // FIXME: platform-dependent size
			p->robe,p->slotchange,opt,p->font,p->uniqueitem_counter,
			p->hotkey_rowshift,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		} else
			strcat(save_status, " status");
	}

	if( p->bank_vault != cp->bank_vault || p->mod_exp != cp->mod_exp || p->mod_drop != cp->mod_drop || p->mod_death != cp->mod_death ) {
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`account_id`,`bank_vault`,`base_exp`,`base_drop`,`base_death`) VALUES ('%d','%d','%d','%d','%d')",account_data_db,p->account_id,p->bank_vault,p->mod_exp,p->mod_drop,p->mod_death) ) {
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		} else
			strcat(save_status, " accdata");
	}

	//Values that will seldom change (to speed up saving)
	if (
		(p->hair != cp->hair) || (p->hair_color != cp->hair_color) ||
		(p->clothes_color != cp->clothes_color) || (p->body != cp->body) ||
		(p->class_ != cp->class_) ||
		(p->partner_id != cp->partner_id) || (p->father != cp->father) ||
		(p->mother != cp->mother) || (p->child != cp->child) ||
		(p->karma != cp->karma) || (p->manner != cp->manner) ||
		(p->fame != cp->fame)
	)
	{
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `class`='%d',"
			"`hair`='%d', `hair_color`='%d', `clothes_color`='%d', `body`='%d',"
			"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d',"
			"`karma`='%d', `manner`='%d', `fame`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			char_db, p->class_,
			p->hair, p->hair_color, p->clothes_color, p->body,
			p->partner_id, p->father, p->mother, p->child,
			p->karma, p->manner, p->fame,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		} else
			strcat(save_status, " status2");
	}

	/* Mercenary Owner */
	if( (p->mer_id != cp->mer_id) ||
		(p->arch_calls != cp->arch_calls) || (p->arch_faith != cp->arch_faith) ||
		(p->spear_calls != cp->spear_calls) || (p->spear_faith != cp->spear_faith) ||
		(p->sword_calls != cp->sword_calls) || (p->sword_faith != cp->sword_faith) )
	{
		if (inter_mercenary->owner_tosql(char_id, p))
			strcat(save_status, " mercenary");
		else
			errors++;
	}

	//memo points
	if( memcmp(p->memo_point, cp->memo_point, sizeof(p->memo_point)) )
	{
		char esc_mapname[NAME_LENGTH*2+1];

		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", memo_db, p->char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		}

		//insert here.
		StrBuf->Clear(&buf);
		StrBuf->Printf(&buf, "INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ", memo_db);
		for( i = 0, count = 0; i < MAX_MEMOPOINTS; ++i )
		{
			if( p->memo_point[i].map )
			{
				if( count )
					StrBuf->AppendStr(&buf, ",");
				SQL->EscapeString(inter->sql_handle, esc_mapname, mapindex_id2name(p->memo_point[i].map));
				StrBuf->Printf(&buf, "('%d', '%s', '%d', '%d')", char_id, esc_mapname, p->memo_point[i].x, p->memo_point[i].y);
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
			{
				Sql_ShowDebug(inter->sql_handle);
				errors++;
			}
		}
		strcat(save_status, " memo");
	}

	//skills
	if( memcmp(p->skill, cp->skill, sizeof(p->skill)) ) {
		//`skill` (`char_id`, `id`, `lv`)
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", skill_db, p->char_id) ) {
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		}

		StrBuf->Clear(&buf);
		StrBuf->Printf(&buf, "INSERT INTO `%s`(`char_id`,`id`,`lv`,`flag`) VALUES ", skill_db);
		//insert here.
		for( i = 0, count = 0; i < MAX_SKILL; ++i ) {
			if( p->skill[i].id != 0 && p->skill[i].flag != SKILL_FLAG_TEMPORARY ) {
				if( p->skill[i].lv == 0 && ( p->skill[i].flag == SKILL_FLAG_PERM_GRANTED || p->skill[i].flag == SKILL_FLAG_PERMANENT ) )
					continue;
				if( p->skill[i].flag != SKILL_FLAG_PERMANENT && p->skill[i].flag != SKILL_FLAG_PERM_GRANTED && (p->skill[i].flag - SKILL_FLAG_REPLACED_LV_0) == 0 )
					continue;
				if( count )
					StrBuf->AppendStr(&buf, ",");
				StrBuf->Printf(&buf, "('%d','%d','%d','%d')", char_id, p->skill[i].id,
								 ( (p->skill[i].flag == SKILL_FLAG_PERMANENT || p->skill[i].flag == SKILL_FLAG_PERM_GRANTED) ? p->skill[i].lv : p->skill[i].flag - SKILL_FLAG_REPLACED_LV_0),
								 p->skill[i].flag == SKILL_FLAG_PERM_GRANTED ? p->skill[i].flag : 0);/* other flags do not need to be saved */
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
			{
				Sql_ShowDebug(inter->sql_handle);
				errors++;
			}
		}

		strcat(save_status, " skills");
	}

	diff = 0;
	for(i = 0; i < MAX_FRIENDS; i++){
		if(p->friends[i].char_id != cp->friends[i].char_id ||
			p->friends[i].account_id != cp->friends[i].account_id){
			diff = 1;
			break;
		}
	}

	if(diff == 1) {
		//Save friends
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", friend_db, char_id) )
		{
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		}

		StrBuf->Clear(&buf);
		StrBuf->Printf(&buf, "INSERT INTO `%s` (`char_id`, `friend_account`, `friend_id`) VALUES ", friend_db);
		for( i = 0, count = 0; i < MAX_FRIENDS; ++i )
		{
			if( p->friends[i].char_id > 0 )
			{
				if( count )
					StrBuf->AppendStr(&buf, ",");
				StrBuf->Printf(&buf, "('%d','%d','%d')", char_id, p->friends[i].account_id, p->friends[i].char_id);
				count++;
			}
		}
		if( count )
		{
			if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
			{
				Sql_ShowDebug(inter->sql_handle);
				errors++;
			}
		}
		strcat(save_status, " friends");
	}

#ifdef HOTKEY_SAVING
	// hotkeys
	StrBuf->Clear(&buf);
	StrBuf->Printf(&buf, "REPLACE INTO `%s` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`) VALUES ", hotkey_db);
	diff = 0;
	for(i = 0; i < ARRAYLENGTH(p->hotkeys); i++){
		if(memcmp(&p->hotkeys[i], &cp->hotkeys[i], sizeof(struct hotkey)))
		{
			if( diff )
				StrBuf->AppendStr(&buf, ",");// not the first hotkey
			StrBuf->Printf(&buf, "('%d','%u','%u','%u','%u')", char_id, (unsigned int)i, (unsigned int)p->hotkeys[i].type, p->hotkeys[i].id , (unsigned int)p->hotkeys[i].lv);
			diff = 1;
		}
	}
	if(diff) {
		if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
		{
			Sql_ShowDebug(inter->sql_handle);
			errors++;
		} else
			strcat(save_status, " hotkeys");
	}
#endif

	StrBuf->Destroy(&buf);
	if (save_status[0]!='\0' && save_log)
		ShowInfo("Saved char %d - %s:%s.\n", char_id, p->name, save_status);
	if (!errors)
		memcpy(cp, p, sizeof(struct mmo_charstatus));
	return 0;
}

/**
 * Saves an array of 'item' entries into the specified table.
 *
 * @param items       The items array.
 * @param max         The array size.
 * @param id          The character/account/guild ID (depending on tableswitch).
 * @param tableswitch The type of table (@see enum inventory_table_type).
 * @return Error code.
 * @retval 0 in case of success.
 */
int char_memitemdata_to_sql(const struct item items[], int max, int id, int tableswitch)
{
	StringBuf buf;
	struct SqlStmt *stmt = NULL;
	int i, j;
	const char *tablename = NULL;
	const char *selectoption = NULL;
	bool has_favorite = false;
	struct item item = { 0 }; // temp storage variable
	bool *flag = NULL; // bit array for inventory matching
	bool found;
	int errors = 0;

	nullpo_ret(items);

	switch (tableswitch) {
	case TABLE_INVENTORY:     tablename = inventory_db;     selectoption = "char_id";    has_favorite = true; break;
	case TABLE_CART:          tablename = cart_db;          selectoption = "char_id";    break;
	case TABLE_STORAGE:       tablename = storage_db;       selectoption = "account_id"; break;
	case TABLE_GUILD_STORAGE: tablename = guild_storage_db; selectoption = "guild_id";   break;
	default:
		ShowError("Invalid table name!\n");
		Assert_retr(1, tableswitch);
	}

	// The following code compares inventory with current database values
	// and performs modification/deletion/insertion only on relevant rows.
	// This approach is more complicated than a trivial delete&insert, but
	// it significantly reduces cpu load on the database server.

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for (j = 0; j < MAX_SLOTS; ++j)
		StrBuf->Printf(&buf, ", `card%d`", j);
	if (has_favorite)
		StrBuf->AppendStr(&buf, ", `favorite`");
	StrBuf->Printf(&buf, " FROM `%s` WHERE `%s`='%d'", tablename, selectoption, id);

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	 || SQL_ERROR == SQL->StmtExecute(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return 1;
	}

	SQL->StmtBindColumn(stmt, 0, SQLDT_INT,       &item.id,          0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_SHORT,     &item.nameid,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_SHORT,     &item.amount,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 3, SQLDT_UINT,      &item.equip,       0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 4, SQLDT_CHAR,      &item.identify,    0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 5, SQLDT_CHAR,      &item.refine,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 6, SQLDT_CHAR,      &item.attribute,   0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 7, SQLDT_UINT,      &item.expire_time, 0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 8, SQLDT_UCHAR,     &item.bound,       0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 9, SQLDT_UINT64,    &item.unique_id,   0, NULL, NULL);
	for (j = 0; j < MAX_SLOTS; ++j)
		SQL->StmtBindColumn(stmt, 10+j, SQLDT_SHORT, &item.card[j], 0, NULL, NULL);
	if (has_favorite)
		SQL->StmtBindColumn(stmt, 10+MAX_SLOTS, SQLDT_UCHAR, &item.favorite, 0, NULL, NULL);

	// bit array indicating which inventory items have already been matched
	flag = aCalloc(max, sizeof(bool));

	while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
		found = false;
		// search for the presence of the item in the char's inventory
		for (i = 0; i < max; ++i) {
			// skip empty and already matched entries
			if (items[i].nameid == 0 || flag[i])
				continue;

			if (items[i].nameid == item.nameid
			 && items[i].unique_id == item.unique_id
			 && items[i].card[0] == item.card[0]
			 && items[i].card[2] == item.card[2]
			 && items[i].card[3] == item.card[3]
			) {
				// They are the same item.
				ARR_FIND(0, MAX_SLOTS, j, items[i].card[j] != item.card[j]);
				if (j == MAX_SLOTS
				 && items[i].amount == item.amount
				 && items[i].equip == item.equip
				 && items[i].identify == item.identify
				 && items[i].refine == item.refine
				 && items[i].attribute == item.attribute
				 && items[i].expire_time == item.expire_time
				 && items[i].bound == item.bound
				 && (!has_favorite || items[i].favorite == item.favorite)
				) {
					; //Do nothing.
				} else {
					// update all fields.
					StrBuf->Clear(&buf);
					StrBuf->Printf(&buf, "UPDATE `%s` SET `amount`='%d', `equip`='%u', `identify`='%d', `refine`='%d',`attribute`='%d', `expire_time`='%u', `bound`='%d'",
						tablename, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time, items[i].bound);
					for (j = 0; j < MAX_SLOTS; ++j)
						StrBuf->Printf(&buf, ", `card%d`=%d", j, items[i].card[j]);
					if (has_favorite)
						StrBuf->Printf(&buf, ", `favorite`='%d'", items[i].favorite);
					StrBuf->Printf(&buf, " WHERE `id`='%d' LIMIT 1", item.id);

					if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
						Sql_ShowDebug(inter->sql_handle);
						errors++;
					}
				}

				found = flag[i] = true; //Item dealt with,
				break; //skip to next item in the db.
			}
		}
		if (!found) {
			// Item not present in inventory, remove it.
			if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE from `%s` where `id`='%d' LIMIT 1", tablename, item.id)) {
				Sql_ShowDebug(inter->sql_handle);
				errors++;
			}
		}
	}
	SQL->StmtFree(stmt);

	StrBuf->Clear(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`", tablename, selectoption);
	for (j = 0; j < MAX_SLOTS; ++j)
		StrBuf->Printf(&buf, ", `card%d`", j);
	if (has_favorite)
		StrBuf->AppendStr(&buf, ", `favorite`");
	StrBuf->AppendStr(&buf, ") VALUES ");

	found = false;
	// insert non-matched items into the db as new items
	for (i = 0; i < max; ++i) {
		// skip empty and already matched entries
		if (items[i].nameid == 0 || flag[i])
			continue;

		if (found)
			StrBuf->AppendStr(&buf, ",");
		else
			found = true;

		StrBuf->Printf(&buf, "('%d', '%d', '%d', '%u', '%d', '%d', '%d', '%u', '%d', '%"PRIu64"'",
			id, items[i].nameid, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time, items[i].bound, items[i].unique_id);
		for (j = 0; j < MAX_SLOTS; ++j)
			StrBuf->Printf(&buf, ", '%d'", items[i].card[j]);
		if (has_favorite)
			StrBuf->Printf(&buf, ", '%d'", items[i].favorite);
		StrBuf->AppendStr(&buf, ")");
	}

	if (found && SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		errors++;
	}

	StrBuf->Destroy(&buf);
	aFree(flag);

	return errors;
}

/**
 * Returns the correct gender ID for the given character and enum value.
 *
 * If the per-character sex is defined but not supported by the current packetver, the database entries are corrected.
 *
 * @param sd Character data, if available.
 * @param p  Character status.
 * @param sex Character sex (database enum)
 *
 * @retval SEX_MALE if the per-character sex is male
 * @retval SEX_FEMALE if the per-character sex is female
 * @retval 99 if the per-character sex is not defined or the current PACKETVER doesn't support it.
 */
int char_mmo_gender(const struct char_session_data *sd, const struct mmo_charstatus *p, char sex)
{
#if PACKETVER >= 20141016
	(void)sd; (void)p; // Unused
	switch (sex) {
		case 'M':
			return SEX_MALE;
		case 'F':
			return SEX_FEMALE;
		case 'U':
		default:
			return 99;
	}
#else
	if (sex == 'M' || sex == 'F') {
		if (!sd) {
			// sd is not available, there isn't much we can do. Just return and print a warning.
			ShowWarning("Character '%s' (CID: %d, AID: %d) has sex '%c', but PACKETVER does not support per-character sex. Defaulting to 'U'.\n",
					p->name, p->char_id, p->account_id, sex);
			return 99;
		}
		if ((sex == 'M' && sd->sex == SEX_FEMALE)
		 || (sex == 'F' && sd->sex == SEX_MALE)) {
			ShowWarning("Changing sex of character '%s' (CID: %d, AID: %d) to 'U' due to incompatible PACKETVER.\n", p->name, p->char_id, p->account_id);
			chr->changecharsex(p->char_id, sd->sex);
		} else {
			ShowInfo("Resetting sex of character '%s' (CID: %d, AID: %d) to 'U' due to incompatible PACKETVER.\n", p->name, p->char_id, p->account_id);
		}
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `sex` = 'U' WHERE `char_id` = '%d'", char_db, p->char_id)) {
			Sql_ShowDebug(inter->sql_handle);
		}
	}
	return 99;
#endif
}

//=====================================================================================================
// Loads the basic character rooster for the given account. Returns total buffer used.
int char_mmo_chars_fromsql(struct char_session_data* sd, uint8* buf)
{
	struct SqlStmt *stmt;
	struct mmo_charstatus p;
	int j = 0, i;
	char last_map[MAP_NAME_LENGTH_EXT];
	time_t unban_time = 0;
	char sex[2];

	nullpo_ret(sd);
	nullpo_ret(buf);

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if( stmt == NULL ) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}
	memset(&p, 0, sizeof(p));

	for(i = 0 ; i < MAX_CHARS; i++ ) {
		sd->found_char[i] = -1;
		sd->unban_time[i] = 0;
	}

	// read char data
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT "
		"`char_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
		"`status_point`,`skill_point`,`option`,`karma`,`manner`,`hair`,`hair_color`,"
		"`clothes_color`,`body`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`rename`,`delete_date`,"
		"`robe`,`slotchange`,`unban_time`,`sex`"
		" FROM `%s` WHERE `account_id`='%d' AND `char_num` < '%d'", char_db, sd->account_id, MAX_CHARS)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0,  SQLDT_INT,    &p.char_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1,  SQLDT_UCHAR,  &p.slot, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2,  SQLDT_STRING, &p.name, sizeof(p.name), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3,  SQLDT_SHORT,  &p.class_, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 4,  SQLDT_UINT,   &p.base_level, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 5,  SQLDT_UINT,   &p.job_level, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 6,  SQLDT_UINT,   &p.base_exp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 7,  SQLDT_UINT,   &p.job_exp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 8,  SQLDT_INT,    &p.zeny, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 9,  SQLDT_SHORT,  &p.str, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_SHORT,  &p.agi, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 11, SQLDT_SHORT,  &p.vit, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 12, SQLDT_SHORT,  &p.int_, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 13, SQLDT_SHORT,  &p.dex, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 14, SQLDT_SHORT,  &p.luk, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 15, SQLDT_INT,    &p.max_hp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 16, SQLDT_INT,    &p.hp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 17, SQLDT_INT,    &p.max_sp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 18, SQLDT_INT,    &p.sp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 19, SQLDT_UINT,   &p.status_point, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 20, SQLDT_UINT,   &p.skill_point, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 21, SQLDT_UINT,   &p.option, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 22, SQLDT_UCHAR,  &p.karma, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 23, SQLDT_SHORT,  &p.manner, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 24, SQLDT_SHORT,  &p.hair, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 25, SQLDT_SHORT,  &p.hair_color, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 26, SQLDT_SHORT,  &p.clothes_color, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 27, SQLDT_SHORT,  &p.body, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 28, SQLDT_SHORT,  &p.weapon, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 29, SQLDT_SHORT,  &p.shield, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 30, SQLDT_SHORT,  &p.head_top, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 31, SQLDT_SHORT,  &p.head_mid, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 32, SQLDT_SHORT,  &p.head_bottom, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 33, SQLDT_STRING, &last_map, sizeof(last_map), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 34, SQLDT_USHORT, &p.rename, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 35, SQLDT_UINT32, &p.delete_date, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 36, SQLDT_SHORT,  &p.robe, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 37, SQLDT_USHORT, &p.slotchange, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 38, SQLDT_LONG,   &unban_time, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 39, SQLDT_ENUM,   &sex, sizeof(sex), NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}

	for( i = 0; i < MAX_CHARS && SQL_SUCCESS == SQL->StmtNextRow(stmt); i++ ) {
		if (p.slot >= MAX_CHARS)
			continue;
		p.last_point.map = mapindex->name2id(last_map);
		sd->found_char[p.slot] = p.char_id;
		sd->unban_time[p.slot] = unban_time;
		p.sex = chr->mmo_gender(sd, &p, sex[0]);
		j += chr->mmo_char_tobuf(WBUFP(buf, j), &p);
	}

	memset(sd->new_name,0,sizeof(sd->new_name));

	SQL->StmtFree(stmt);
	return j;
}

//=====================================================================================================
int char_mmo_char_fromsql(int char_id, struct mmo_charstatus* p, bool load_everything)
{
	int i,j;
	char t_msg[128] = "";
	struct mmo_charstatus* cp;
	StringBuf buf;
	struct SqlStmt *stmt;
	char last_map[MAP_NAME_LENGTH_EXT];
	char save_map[MAP_NAME_LENGTH_EXT];
	char point_map[MAP_NAME_LENGTH_EXT];
	struct point tmp_point;
	struct item tmp_item;
	struct s_skill tmp_skill;
	struct s_friend tmp_friend;
#ifdef HOTKEY_SAVING
	struct hotkey tmp_hotkey;
	int hotkey_num = 0;
#endif
	unsigned int opt;
	int account_id;
	char sex[2];

	nullpo_ret(p);

	memset(p, 0, sizeof(struct mmo_charstatus));

	if (save_log) ShowInfo("Char load request (%d)\n", char_id);

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if( stmt == NULL )
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	// read char data
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT "
		"`char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
		"`status_point`,`skill_point`,`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`homun_id`,`elemental_id`,`hair`,"
		"`hair_color`,`clothes_color`,`body`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`last_x`,`last_y`,"
		"`save_map`,`save_x`,`save_y`,`partner_id`,`father`,`mother`,`child`,`fame`,`rename`,`delete_date`,`robe`,`slotchange`,"
		"`char_opt`,`font`,`uniqueitem_counter`,`sex`,`hotkey_rowshift`"
		" FROM `%s` WHERE `char_id`=? LIMIT 1", char_db)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0,  SQLDT_INT,    &p->char_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1,  SQLDT_INT,    &p->account_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2,  SQLDT_UCHAR,  &p->slot, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3,  SQLDT_STRING, &p->name, sizeof(p->name), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 4,  SQLDT_SHORT,  &p->class_, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 5,  SQLDT_UINT,   &p->base_level, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 6,  SQLDT_UINT,   &p->job_level, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 7,  SQLDT_UINT,   &p->base_exp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 8,  SQLDT_UINT,   &p->job_exp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 9,  SQLDT_INT,    &p->zeny, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_SHORT,  &p->str, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 11, SQLDT_SHORT,  &p->agi, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 12, SQLDT_SHORT,  &p->vit, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 13, SQLDT_SHORT,  &p->int_, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 14, SQLDT_SHORT,  &p->dex, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 15, SQLDT_SHORT,  &p->luk, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 16, SQLDT_INT,    &p->max_hp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 17, SQLDT_INT,    &p->hp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 18, SQLDT_INT,    &p->max_sp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 19, SQLDT_INT,    &p->sp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 20, SQLDT_UINT,   &p->status_point, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 21, SQLDT_UINT,   &p->skill_point, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 22, SQLDT_UINT,   &p->option, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 23, SQLDT_UCHAR,  &p->karma, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 24, SQLDT_SHORT,  &p->manner, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 25, SQLDT_INT,    &p->party_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 26, SQLDT_INT,    &p->guild_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 27, SQLDT_INT,    &p->pet_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 28, SQLDT_INT,    &p->hom_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 29, SQLDT_INT,    &p->ele_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 30, SQLDT_SHORT,  &p->hair, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 31, SQLDT_SHORT,  &p->hair_color, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 32, SQLDT_SHORT,  &p->clothes_color, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 33, SQLDT_SHORT,  &p->body, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 34, SQLDT_SHORT,  &p->weapon, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 35, SQLDT_SHORT,  &p->shield, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 36, SQLDT_SHORT,  &p->head_top, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 37, SQLDT_SHORT,  &p->head_mid, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 38, SQLDT_SHORT,  &p->head_bottom, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 39, SQLDT_STRING, &last_map, sizeof(last_map), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 40, SQLDT_SHORT,  &p->last_point.x, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 41, SQLDT_SHORT,  &p->last_point.y, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 42, SQLDT_STRING, &save_map, sizeof(save_map), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 43, SQLDT_SHORT,  &p->save_point.x, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 44, SQLDT_SHORT,  &p->save_point.y, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 45, SQLDT_INT,    &p->partner_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 46, SQLDT_INT,    &p->father, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 47, SQLDT_INT,    &p->mother, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 48, SQLDT_INT,    &p->child, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 49, SQLDT_INT,    &p->fame, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 50, SQLDT_USHORT, &p->rename, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 51, SQLDT_UINT32, &p->delete_date, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 52, SQLDT_SHORT,  &p->robe, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 53, SQLDT_USHORT, &p->slotchange, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 54, SQLDT_UINT,   &opt, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 55, SQLDT_UCHAR,  &p->font, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 56, SQLDT_UINT,   &p->uniqueitem_counter, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 57, SQLDT_ENUM,   &sex, sizeof(sex), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 58, SQLDT_UCHAR,  &p->hotkey_rowshift, 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}
	if (SQL_SUCCESS != SQL->StmtNextRow(stmt))
	{
		ShowError("Requested non-existant character id: %d!\n", char_id);
		SQL->StmtFree(stmt);
		return 0;
	}

	p->sex = chr->mmo_gender(NULL, p, sex[0]);

	account_id = p->account_id;

	p->last_point.map = mapindex->name2id(last_map);
	p->save_point.map = mapindex->name2id(save_map);

	if( p->last_point.map == 0 ) {
		p->last_point.map = (unsigned short)strdb_iget(mapindex->db, mapindex->default_map);
		p->last_point.x = mapindex->default_x;
		p->last_point.y = mapindex->default_y;
	}

	if( p->save_point.map == 0 ) {
		p->save_point.map = (unsigned short)strdb_iget(mapindex->db, mapindex->default_map);
		p->save_point.x = mapindex->default_x;
		p->save_point.y = mapindex->default_y;
	}

	strcat(t_msg, " status");

	if (!load_everything) // For quick selection of data when displaying the char menu
	{
		SQL->StmtFree(stmt);
		return 1;
	}

	//read memo data
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	memset(&tmp_point, 0, sizeof(tmp_point));
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`=? ORDER by `memo_id` LIMIT %d", memo_db, MAX_MEMOPOINTS)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &point_map, sizeof(point_map), NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_SHORT,  &tmp_point.x, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_SHORT,  &tmp_point.y, 0, NULL, NULL)
	)
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_MEMOPOINTS && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i ) {
		tmp_point.map = mapindex->name2id(point_map);
		memcpy(&p->memo_point[i], &tmp_point, sizeof(tmp_point));
	}
	strcat(t_msg, " memo");

	//read inventory
	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `expire_time`, `favorite`, `bound`, `unique_id`)
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `favorite`, `bound`, `unique_id`");
	for( i = 0; i < MAX_SLOTS; ++i )
		StrBuf->Printf(&buf, ", `card%d`", i);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `char_id`=? LIMIT %d", inventory_db, MAX_INVENTORY);

	memset(&tmp_item, 0, sizeof(tmp_item));
	if (SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  0, SQLDT_INT,       &tmp_item.id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  1, SQLDT_SHORT,     &tmp_item.nameid, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  2, SQLDT_SHORT,     &tmp_item.amount, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  3, SQLDT_UINT,      &tmp_item.equip, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  4, SQLDT_CHAR,      &tmp_item.identify, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  5, SQLDT_CHAR,      &tmp_item.refine, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  6, SQLDT_CHAR,      &tmp_item.attribute, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  7, SQLDT_UINT,      &tmp_item.expire_time, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  8, SQLDT_CHAR,      &tmp_item.favorite, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt,  9, SQLDT_UCHAR,     &tmp_item.bound, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_UINT64,    &tmp_item.unique_id, 0, NULL, NULL)
	)
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < MAX_SLOTS; ++i )
		if( SQL_ERROR == SQL->StmtBindColumn(stmt, 11+i, SQLDT_SHORT, &tmp_item.card[i], 0, NULL, NULL) )
			SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_INVENTORY && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i )
		memcpy(&p->inventory[i], &tmp_item, sizeof(tmp_item));

	strcat(t_msg, " inventory");

	//read cart
	//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, expire_time`, `bound`, `unique_id`)
	StrBuf->Clear(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StrBuf->Printf(&buf, ", `card%d`", j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `char_id`=? LIMIT %d", cart_db, MAX_CART);

	memset(&tmp_item, 0, sizeof(tmp_item));
	if (SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT,         &tmp_item.id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_SHORT,       &tmp_item.nameid, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_SHORT,       &tmp_item.amount, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_UINT,        &tmp_item.equip, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 4, SQLDT_CHAR,        &tmp_item.identify, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 5, SQLDT_CHAR,        &tmp_item.refine, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 6, SQLDT_CHAR,        &tmp_item.attribute, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 7, SQLDT_UINT,        &tmp_item.expire_time, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 8, SQLDT_UCHAR,       &tmp_item.bound, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 9, SQLDT_UINT64,      &tmp_item.unique_id, 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
	}
	for( i = 0; i < MAX_SLOTS; ++i )
		if( SQL_ERROR == SQL->StmtBindColumn(stmt, 10+i, SQLDT_SHORT, &tmp_item.card[i], 0, NULL, NULL) )
			SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_CART && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i )
		memcpy(&p->cart[i], &tmp_item, sizeof(tmp_item));
	strcat(t_msg, " cart");

	//read storage
	inter_storage->fromsql(p->account_id, &p->storage);
	strcat(t_msg, " storage");

	//read skill
	//`skill` (`char_id`, `id`, `lv`)
	memset(&tmp_skill, 0, sizeof(tmp_skill));
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `id`, `lv`,`flag` FROM `%s` WHERE `char_id`=? LIMIT %d", skill_db, MAX_SKILL)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_USHORT, &tmp_skill.id  , 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_UCHAR , &tmp_skill.lv  , 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_UCHAR , &tmp_skill.flag, 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
	}

	if( tmp_skill.flag != SKILL_FLAG_PERM_GRANTED )
		tmp_skill.flag = SKILL_FLAG_PERMANENT;

	for( i = 0; i < MAX_SKILL && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i ) {
		if( skillid2idx[tmp_skill.id] )
			memcpy(&p->skill[skillid2idx[tmp_skill.id]], &tmp_skill, sizeof(tmp_skill));
		else
			ShowWarning("chr->mmo_char_fromsql: ignoring invalid skill (id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", tmp_skill.id, tmp_skill.lv, p->name, p->account_id, p->char_id);
	}
	strcat(t_msg, " skills");

	//read friends
	//`friends` (`char_id`, `friend_account`, `friend_id`)
	memset(&tmp_friend, 0, sizeof(tmp_friend));
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT c.`account_id`, c.`char_id`, c.`name` FROM `%s` c LEFT JOIN `%s` f ON f.`friend_account` = c.`account_id` AND f.`friend_id` = c.`char_id` WHERE f.`char_id`=? LIMIT %d", char_db, friend_db, MAX_FRIENDS)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT,    &tmp_friend.account_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_INT,    &tmp_friend.char_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_STRING, &tmp_friend.name, sizeof(tmp_friend.name), NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
	}

	for( i = 0; i < MAX_FRIENDS && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i )
		memcpy(&p->friends[i], &tmp_friend, sizeof(tmp_friend));
	strcat(t_msg, " friends");

#ifdef HOTKEY_SAVING
	//read hotkeys
	//`hotkey` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`
	memset(&tmp_hotkey, 0, sizeof(tmp_hotkey));
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `hotkey`, `type`, `itemskill_id`, `skill_lvl` FROM `%s` WHERE `char_id`=?", hotkey_db)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT,    &hotkey_num, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_UCHAR,  &tmp_hotkey.type, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_UINT,   &tmp_hotkey.id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_USHORT, &tmp_hotkey.lv, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);

	while( SQL_SUCCESS == SQL->StmtNextRow(stmt) )
	{
		if( hotkey_num >= 0 && hotkey_num < MAX_HOTKEYS )
			memcpy(&p->hotkeys[hotkey_num], &tmp_hotkey, sizeof(tmp_hotkey));
		else
			ShowWarning("chr->mmo_char_fromsql: ignoring invalid hotkey (hotkey=%d,type=%u,id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", hotkey_num, tmp_hotkey.type, tmp_hotkey.id, tmp_hotkey.lv, p->name, p->account_id, p->char_id);
	}
	strcat(t_msg, " hotkeys");
#endif

	/* Mercenary Owner DataBase */
	inter_mercenary->owner_fromsql(char_id, p);
	strcat(t_msg, " mercenary");

	/* default */
	p->mod_exp = p->mod_drop = p->mod_death = 100;

	//`account_data` (`account_id`,`bank_vault`,`base_exp`,`base_drop`,`base_death`)
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `bank_vault`,`base_exp`,`base_drop`,`base_death` FROM `%s` WHERE `account_id`=? LIMIT 1", account_data_db)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &account_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT, &p->bank_vault, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_USHORT, &p->mod_exp, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_USHORT, &p->mod_drop, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_USHORT, &p->mod_death, 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
	}

	if( SQL_SUCCESS == SQL->StmtNextRow(stmt) )
		strcat(t_msg, " accdata");

	if (save_log) ShowInfo("Loaded char (%d - %s): %s\n", char_id, p->name, t_msg); //ok. all data load successfully!
	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);

	/* load options into proper vars */
	if( opt & OPT_ALLOW_PARTY )
		p->allow_party = true;
	if( opt & OPT_SHOW_EQUIP )
		p->show_equip = true;

	cp = idb_ensure(chr->char_db_, char_id, chr->create_charstatus);
	memcpy(cp, p, sizeof(struct mmo_charstatus));
	return 1;
}

//==========================================================================================================
int char_mmo_char_sql_init(void)
{
	chr->char_db_= idb_alloc(DB_OPT_RELEASE_DATA);

	//the 'set offline' part is now in check_login_conn ...
	//if the server connects to loginserver
	//it will dc all off players
	//and send the loginserver the new state....

	// Force all users offline in sql when starting char-server
	// (useful when servers crashes and don't clean the database)
	chr->set_all_offline_sql();

	return 0;
}

/* [Ind/Hercules] - special thanks to Yommy for providing the packet structure/data */
bool char_char_slotchange(struct char_session_data *sd, int fd, unsigned short from, unsigned short to) {
	struct mmo_charstatus char_dat;
	int from_id = 0;

	nullpo_ret(sd);
	if( from >= MAX_CHARS || to >= MAX_CHARS || ( sd->char_slots && to > sd->char_slots ) || sd->found_char[from] <= 0 )
		return false;

	if( !chr->mmo_char_fromsql(sd->found_char[from], &char_dat, false) ) // Only the short data is needed.
		return false;

	if( char_dat.slotchange == 0 )
		return false;

	from_id = sd->found_char[from];

	if( sd->found_char[to] > 0 ) {/* moving char to occupied slot */
		bool result = false;
		/* update both at once */
		if( SQL_SUCCESS != SQL->QueryStr(inter->sql_handle, "START TRANSACTION")
		   ||  SQL_SUCCESS != SQL->Query(inter->sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id`='%d' LIMIT 1", char_db, from, sd->found_char[to])
		   ||  SQL_SUCCESS != SQL->Query(inter->sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id`='%d' LIMIT 1", char_db, to, sd->found_char[from])
		)
			Sql_ShowDebug(inter->sql_handle);
		else
			result = true;

		if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") ) {
			Sql_ShowDebug(inter->sql_handle);
			result = false;
		}
		if( !result )
			return false;
	} else {/* slot is free. */
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id`='%d' LIMIT 1", char_db, to, sd->found_char[from] ) ) {
			Sql_ShowDebug(inter->sql_handle);
			return false;
		}
	}

	/* update count */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `slotchange`=`slotchange`-1 WHERE `char_id`='%d' LIMIT 1", char_db, from_id ) ) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

//-----------------------------------
// Function to change character's names
//-----------------------------------
int char_rename_char_sql(struct char_session_data *sd, int char_id)
{
	struct mmo_charstatus char_dat;
	char esc_name[NAME_LENGTH*2+1];

	nullpo_retr(2, sd);

	if( sd->new_name[0] == 0 ) // Not ready for rename
		return 2;

	if( !chr->mmo_char_fromsql(char_id, &char_dat, false) ) // Only the short data is needed.
		return 2;

	if (sd->account_id != char_dat.account_id) // Try rename not own char
		return 2;

	if( char_dat.rename == 0 )
		return 1;

	SQL->EscapeStringLen(inter->sql_handle, esc_name, sd->new_name, strnlen(sd->new_name, NAME_LENGTH));

	// check if the char exist
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT 1 FROM `%s` WHERE `name` LIKE '%s' LIMIT 1", char_db, esc_name) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return 4;
	}

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `name` = '%s', `rename` = '%d' WHERE `char_id` = '%d'", char_db, esc_name, --char_dat.rename, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return 3;
	}

	// Change character's name into guild_db.
	if( char_dat.guild_id )
		inter_guild->charname_changed(char_dat.guild_id, sd->account_id, char_id, sd->new_name);

	safestrncpy(char_dat.name, sd->new_name, NAME_LENGTH);
	memset(sd->new_name,0,sizeof(sd->new_name));

	// log change
	if( log_char )
	{
		if( SQL_ERROR == SQL->Query(inter->sql_handle,
			"INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%d', '%s', '0', '0', '0', '0', '0', '0', '0', '0')",
			charlog_db, "change char name", sd->account_id, char_dat.char_id, char_dat.slot, esc_name) )
			Sql_ShowDebug(inter->sql_handle);
	}

	return 0;
}

int char_check_char_name(char * name, char * esc_name)
{
	int i;

	nullpo_retr(-2, name);
	nullpo_retr(-2, esc_name);

	// check length of character name
	if (name[0] == '\0')
		return -2; // empty character name
	/**
	 * The client does not allow you to create names with less than 4 characters, however,
	 * the use of WPE can bypass this, and this fixes the exploit.
	 **/
	if( strlen( name ) < 4 )
		return -2;
	// check content of character name
	if( remove_control_chars(name) )
		return -2; // control chars in name

	// check for reserved names
	if( strcmpi(name, wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// Check Authorized letters/symbols in the name of the character
	if( char_name_option == 1 )
	{ // only letters/symbols in char_name_letters are authorized
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) == NULL )
				return -2;
	}
	else if( char_name_option == 2 )
	{ // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) != NULL )
				return -5;
	}
	if( name_ignoring_case ) {
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT 1 FROM `%s` WHERE BINARY `name` = '%s' LIMIT 1", char_db, esc_name) ) {
			Sql_ShowDebug(inter->sql_handle);
			return -2;
		}
	} else {
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT 1 FROM `%s` WHERE `name` = '%s' LIMIT 1", char_db, esc_name) ) {
			Sql_ShowDebug(inter->sql_handle);
			return -2;
		}
	}
	if( SQL->NumRows(inter->sql_handle) > 0 )
		return -1; // name already exists

	return 0;
}

/**
 * Creates a new character
 * Return values:
 *  -1: 'Charname already exists'
 *  -2: 'Char creation denied'/ Unknown error
 *  -3: 'You are underaged'
 *  -4: 'You are not eligible to open the Character Slot.'
 *  -5: 'Symbols in Character Names are forbidden'
 *  char_id: Success
 **/
int char_make_new_char_sql(struct char_session_data *sd, const char *name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];
	int char_id, flag, k, l;

	nullpo_retr(-2, sd);
	nullpo_retr(-2, name_);
	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name,TRIM_CHARS);
	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));

	flag = chr->check_char_name(name,esc_name);
	if( flag < 0 )
		return flag;

	//check other inputs
#if PACKETVER >= 20120307
	if(slot < 0 || slot >= sd->char_slots)
#else
	if((slot < 0 || slot >= sd->char_slots) // slots
	|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
	|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
	|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
#endif
#if PACKETVER >= 20100413
		return -4; // invalid slot
#else
		return -2; // invalid input
#endif

	// check char slot
	if( sd->found_char[slot] != -1 )
		return -2; /* character account limit exceeded */

#if PACKETVER >= 20120307
	//Insert the new char entry to the database
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`account_id`, `char_num`, `name`, `zeny`, `status_point`,`str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`,"
		"`max_sp`, `sp`, `hair`, `hair_color`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`) VALUES ("
		"'%d', '%d', '%s', '%d',  '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d','%d', '%d','%d', '%d', '%s', '%d', '%d', '%s', '%d', '%d')",
		char_db, sd->account_id , slot, esc_name, start_zeny, 48, str, agi, vit, int_, dex, luk,
		(40 * (100 + vit)/100) , (40 * (100 + vit)/100 ),  (11 * (100 + int_)/100), (11 * (100 + int_)/100), hair_style, hair_color,
		mapindex_id2name(start_point.map), start_point.x, start_point.y, mapindex_id2name(start_point.map), start_point.x, start_point.y) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return -2; //No, stop the procedure!
	}
#else
	//Insert the new char entry to the database
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`account_id`, `char_num`, `name`, `zeny`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`,"
							   "`max_sp`, `sp`, `hair`, `hair_color`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`) VALUES ("
							   "'%d', '%d', '%s', '%d',  '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d','%d', '%d','%d', '%d', '%s', '%d', '%d', '%s', '%d', '%d')",
							   char_db, sd->account_id , slot, esc_name, start_zeny, str, agi, vit, int_, dex, luk,
							   (40 * (100 + vit)/100) , (40 * (100 + vit)/100 ),  (11 * (100 + int_)/100), (11 * (100 + int_)/100), hair_style, hair_color,
							   mapindex_id2name(start_point.map), start_point.x, start_point.y, mapindex_id2name(start_point.map), start_point.x, start_point.y) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return -2; //No, stop the procedure!
	}
#endif
	//Retrieve the newly auto-generated char id
	char_id = (int)SQL->LastInsertId(inter->sql_handle);

	if( !char_id )
		return -2;

	// Validation success, log result
	if (log_char) {
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			charlog_db, "make new char", sd->account_id, char_id, slot, esc_name, str, agi, vit, int_, dex, luk, hair_style, hair_color) )
			Sql_ShowDebug(inter->sql_handle);
	}

	//Give the char the default items
	for (k = 0; k < ARRAYLENGTH(start_items) && start_items[k] != 0; k += 3) {
		// FIXME: How to define if an item is stackable without having to lookup itemdb? [panikon]
		if( start_items[k+2] == 1 )
		{
			if( SQL_ERROR == SQL->Query(inter->sql_handle,
				"INSERT INTO `%s` (`char_id`,`nameid`, `amount`, `identify`) VALUES ('%d', '%d', '%d', '%d')",
				inventory_db, char_id, start_items[k], start_items[k + 1], 1) )
					Sql_ShowDebug(inter->sql_handle);
		}
		else if( start_items[k+2] == 0 )
		{
			// Non-stackable items should have their own entries (issue: 7279)
			for( l = 0; l < start_items[k+1]; l++ )
			{
				if( SQL_ERROR == SQL->Query(inter->sql_handle,
					"INSERT INTO `%s` (`char_id`,`nameid`, `amount`, `identify`) VALUES ('%d', '%d', '%d', '%d')",
					inventory_db, char_id, start_items[k], 1, 1)
					)
					Sql_ShowDebug(inter->sql_handle);
			}
		}
	}

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", sd->account_id, char_id, slot, name);
	return char_id;
}

/*----------------------------------------------------------------------------------------------------------*/
/* Divorce Players */
/*----------------------------------------------------------------------------------------------------------*/
int char_divorce_char_sql(int partner_id1, int partner_id2)
{
	unsigned char buf[64];

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `partner_id`='0' WHERE `char_id`='%d' OR `char_id`='%d' LIMIT 2", char_db, partner_id1, partner_id2) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE (`nameid`='%d' OR `nameid`='%d') AND (`char_id`='%d' OR `char_id`='%d') LIMIT 2", inventory_db, WEDDING_RING_M, WEDDING_RING_F, partner_id1, partner_id2) )
		Sql_ShowDebug(inter->sql_handle);

	WBUFW(buf,0) = 0x2b12;
	WBUFL(buf,2) = partner_id1;
	WBUFL(buf,6) = partner_id2;
	mapif->sendall(buf,10);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------*/
/* Delete char - davidsiaw */
/*----------------------------------------------------------------------------------------------------------*/
/* Returns 0 if successful
 * Returns < 0 for error
 */
int char_delete_char_sql(int char_id)
{
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1]; //Name needs be escaped.
	int account_id, party_id, guild_id, hom_id, base_level, partner_id, father_id, mother_id, elemental_id;
	char *data;
	size_t len;

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `name`,`account_id`,`party_id`,`guild_id`,`base_level`,`homun_id`,`partner_id`,`father`,`mother`,`elemental_id` FROM `%s` WHERE `char_id`='%d'", char_db, char_id))
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{
		ShowError("chr->delete_char_sql: Unable to fetch character data, deletion aborted.\n");
		SQL->FreeResult(inter->sql_handle);
		return -1;
	}

	SQL->GetData(inter->sql_handle, 0, &data, &len); safestrncpy(name, data, NAME_LENGTH);
	SQL->GetData(inter->sql_handle, 1, &data, NULL); account_id = atoi(data);
	SQL->GetData(inter->sql_handle, 2, &data, NULL); party_id = atoi(data);
	SQL->GetData(inter->sql_handle, 3, &data, NULL); guild_id = atoi(data);
	SQL->GetData(inter->sql_handle, 4, &data, NULL); base_level = atoi(data);
	SQL->GetData(inter->sql_handle, 5, &data, NULL); hom_id = atoi(data);
	SQL->GetData(inter->sql_handle, 6, &data, NULL); partner_id = atoi(data);
	SQL->GetData(inter->sql_handle, 7, &data, NULL); father_id = atoi(data);
	SQL->GetData(inter->sql_handle, 8, &data, NULL); mother_id = atoi(data);
	SQL->GetData(inter->sql_handle, 9, &data, NULL);
	elemental_id = atoi(data);

	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, min(len, NAME_LENGTH));
	SQL->FreeResult(inter->sql_handle);

	//check for config char del condition [Lupus]
	// TODO: Move this out to packet processing (0x68/0x1fb).
	if( ( char_del_level > 0 && base_level >= char_del_level )
	 || ( char_del_level < 0 && base_level <= -char_del_level )
	) {
			ShowInfo("Char deletion aborted: %s, BaseLevel: %i\n", name, base_level);
			return -1;
	}

	/* Divorce [Wizputer] */
	if( partner_id )
		chr->divorce_char_sql(char_id, partner_id);

	/* De-addopt [Zephyrus] */
	if( father_id || mother_id )
	{ // Char is Baby
		unsigned char buf[64];

		if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `child`='0' WHERE `char_id`='%d' OR `char_id`='%d'", char_db, father_id, mother_id) )
			Sql_ShowDebug(inter->sql_handle);
		if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `id` = '410'AND (`char_id`='%d' OR `char_id`='%d')", skill_db, father_id, mother_id) )
			Sql_ShowDebug(inter->sql_handle);

		WBUFW(buf,0) = 0x2b25;
		WBUFL(buf,2) = father_id;
		WBUFL(buf,6) = mother_id;
		WBUFL(buf,10) = char_id; // Baby
		mapif->sendall(buf,14);
	}

	//Make the character leave the party [Skotlex]
	if (party_id)
		inter_party->leave(party_id, account_id, char_id);

	/* delete char's pet */
	//Delete the hatched pet if you have one...
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d' AND `incubate` = '0'", pet_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	//Delete all pets that are stored in eggs (inventory + cart)
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` USING `%s` JOIN `%s` ON `pet_id` = `card1`|`card2`<<16 WHERE `%s`.char_id = '%d' AND card0 = -256", pet_db, pet_db, inventory_db, inventory_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` USING `%s` JOIN `%s` ON `pet_id` = `card1`|`card2`<<16 WHERE `%s`.char_id = '%d' AND card0 = -256", pet_db, pet_db, cart_db, cart_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* remove homunculus */
	if( hom_id )
		mapif->homunculus_delete(hom_id);

	/* remove elemental */
	if (elemental_id)
		mapif->elemental_delete(elemental_id);

	/* remove mercenary data */
	inter_mercenary->owner_delete(char_id);

	/* delete char's friends list */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", friend_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete char from other's friend list */
	//NOTE: Won't this cause problems for people who are already online? [Skotlex]
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `friend_id` = '%d'", friend_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

#ifdef HOTKEY_SAVING
	/* delete hotkeys */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", hotkey_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
#endif

	/* delete inventory */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", inventory_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete cart inventory */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", cart_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete memo areas */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", memo_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete character registry */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", char_reg_str_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", char_reg_num_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete skills */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", skill_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	/* delete mails (only received) */
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `dest_id`='%d'", mail_db, char_id))
		Sql_ShowDebug(inter->sql_handle);

#ifdef ENABLE_SC_SAVING
	/* status changes */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, account_id, char_id) )
		Sql_ShowDebug(inter->sql_handle);
#endif

	/* delete character */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	else if( log_char ) {
		if( SQL_ERROR == SQL->Query(inter->sql_handle,
			"INSERT INTO `%s`(`time`, `account_id`, `char_id`, `char_num`, `char_msg`, `name`)"
			" VALUES (NOW(), '%d', '%d', '%d', 'Deleted character', '%s')",
			charlog_db, account_id, char_id, 0, esc_name) )
			Sql_ShowDebug(inter->sql_handle);
	}

	/* No need as we used inter_guild->leave [Skotlex]
	// Also delete info from guildtables.
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", guild_member_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	*/

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `guild_id` FROM `%s` WHERE `char_id` = '%d'", guild_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	else if( SQL->NumRows(inter->sql_handle) > 0 )
		mapif->parse_BreakGuild(0,guild_id);
	else if( guild_id )
		inter_guild->leave(guild_id, account_id, char_id);// Leave your guild.
	return 0;
}

//---------------------------------------------------------------------
// This function return the number of online players in all map-servers
//---------------------------------------------------------------------
int char_count_users(void)
{
	int i, users;

	users = 0;
	for(i = 0; i < ARRAYLENGTH(chr->server); i++) {
		if (chr->server[i].fd > 0) {
			users += chr->server[i].users;
		}
	}
	return users;
}

// Writes char data to the buffer in the format used by the client.
// Used in packets 0x6b (chars info) and 0x6d (new char info)
// Returns the size
#define MAX_CHAR_BUF 150 //Max size (for WFIFOHEAD calls)
int char_mmo_char_tobuf(uint8* buffer, struct mmo_charstatus* p) {
	unsigned short offset = 0;
	uint8* buf;

	if( buffer == NULL || p == NULL )
		return 0;

	buf = WBUFP(buffer,0);
	WBUFL(buf,0) = p->char_id;
	WBUFL(buf,4) = min(p->base_exp, INT32_MAX);
	WBUFL(buf,8) = p->zeny;
	WBUFL(buf,12) = min(p->job_exp, INT32_MAX);
	WBUFL(buf,16) = p->job_level;
	WBUFL(buf,20) = 0; // probably opt1
	WBUFL(buf,24) = 0; // probably opt2
	WBUFL(buf,28) = (p->option &~ 0x40);
	WBUFL(buf,32) = p->karma;
	WBUFL(buf,36) = p->manner;
	WBUFW(buf,40) = min(p->status_point, INT16_MAX);
#if PACKETVER > 20081217
	WBUFL(buf,42) = p->hp;
	WBUFL(buf,46) = p->max_hp;
	offset+=4;
	buf = WBUFP(buffer,offset);
#else
	WBUFW(buf,42) = min(p->hp, INT16_MAX);
	WBUFW(buf,44) = min(p->max_hp, INT16_MAX);
#endif
	WBUFW(buf,46) = min(p->sp, INT16_MAX);
	WBUFW(buf,48) = min(p->max_sp, INT16_MAX);
	WBUFW(buf,50) = DEFAULT_WALK_SPEED; // p->speed;
	WBUFW(buf,52) = p->class_;
	WBUFW(buf,54) = p->hair;
#if PACKETVER >= 20141022
	WBUFW(buf,56) = p->body;
	offset+=2;
	buf = WBUFP(buffer,offset);
#endif

	//When the weapon is sent and your option is riding, the client crashes on login!?
	// FIXME[Haru]: is OPTION_HANBOK intended to be part of this list? And if it is, should the list also include other OPTION_ costumes?
	WBUFW(buf,56) = p->option&(OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_WUGRIDER|OPTION_MADOGEAR|OPTION_HANBOK) ? 0 : p->weapon;

	WBUFW(buf,58) = p->base_level;
	WBUFW(buf,60) = min(p->skill_point, INT16_MAX);
	WBUFW(buf,62) = p->head_bottom;
	WBUFW(buf,64) = p->shield;
	WBUFW(buf,66) = p->head_top;
	WBUFW(buf,68) = p->head_mid;
	WBUFW(buf,70) = p->hair_color;
	WBUFW(buf,72) = p->clothes_color;
	memcpy(WBUFP(buf,74), p->name, NAME_LENGTH);
	WBUFB(buf,98) = min(p->str, UINT8_MAX);
	WBUFB(buf,99) = min(p->agi, UINT8_MAX);
	WBUFB(buf,100) = min(p->vit, UINT8_MAX);
	WBUFB(buf,101) = min(p->int_, UINT8_MAX);
	WBUFB(buf,102) = min(p->dex, UINT8_MAX);
	WBUFB(buf,103) = min(p->luk, UINT8_MAX);
	WBUFW(buf,104) = p->slot;
#if PACKETVER >= 20061023
	WBUFW(buf,106) = ( p->rename > 0 ) ? 0 : 1;
	offset += 2;
#endif
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	mapindex->getmapname_ext(mapindex_id2name(p->last_point.map), WBUFP(buf,108));
	offset += MAP_NAME_LENGTH_EXT;
#endif
#if PACKETVER >= 20100803
	WBUFL(buf,124) = (int)p->delete_date;
	offset += 4;
#endif
#if PACKETVER >= 20110111
	WBUFL(buf,128) = p->robe;
	offset += 4;
#endif
#if PACKETVER != 20111116 //2011-11-16 wants 136, ask gravity.
	#if PACKETVER >= 20110928
		WBUFL(buf,132) = ( p->slotchange > 0 ) ? 1 : 0;  // change slot feature (0 = disabled, otherwise enabled)
		offset += 4;
	#endif
	#if PACKETVER >= 20111025
		WBUFL(buf,136) = ( p->rename > 0 ) ? 1 : 0;  // (0 = disabled, otherwise displays "Add-Ons" sidebar)
		offset += 4;
	#endif
	#if PACKETVER >= 20141016
		WBUFB(buf,140) = p->sex;// sex - (0 = female, 1 = male, 99 = logindefined)
		offset += 1;
	#endif
#endif

	return 106+offset;
}

/* Made Possible by Yommy~! <3 */
void char_mmo_char_send099d(int fd, struct char_session_data *sd) {
	WFIFOHEAD(fd,4 + (MAX_CHARS*MAX_CHAR_BUF));
	WFIFOW(fd,0) = 0x99d;
	WFIFOW(fd,2) = chr->mmo_chars_fromsql(sd, WFIFOP(fd,4)) + 4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

/* Sends character ban list */
/* Made Possible by Yommy~! <3 */
void char_mmo_char_send_ban_list(int fd, struct char_session_data *sd) {
	int i;
	time_t now = time(NULL);

	nullpo_retv(sd);
	ARR_FIND(0, MAX_CHARS, i, sd->unban_time[i]);
	if( i != MAX_CHARS ) {
		int c;

		WFIFOHEAD(fd, 4 + (MAX_CHARS*24));

		WFIFOW(fd, 0) = 0x20d;

		for(i = 0, c = 0; i < MAX_CHARS; i++) {
			if( sd->unban_time[i] ) {
				timestamp2string(WFIFOP(fd,8 + (28*c)), 20, sd->unban_time[i], "%Y-%m-%d %H:%M:%S");

				if( sd->unban_time[i] > now )
					WFIFOL(fd, 4 + (24*c)) = sd->found_char[i];
				else {
					/* reset -- client keeps this information even if you logout so we need to clear */
					WFIFOL(fd, 4 + (24*c)) = 0;
					/* also update on mysql */
					sd->unban_time[i] = 0;
					if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `unban_time`='0' WHERE `char_id`='%d' LIMIT 1", char_db, sd->found_char[i]) )
						Sql_ShowDebug(inter->sql_handle);
				}
				c++;
			}
		}

		WFIFOW(fd, 2) = 4 + (24*c);

		WFIFOSET(fd, WFIFOW(fd, 2));
	}
}

//----------------------------------------
// [Ind/Hercules] notify client about charselect window data
//----------------------------------------
void char_mmo_char_send_slots_info(int fd, struct char_session_data* sd) {
	nullpo_retv(sd);
	WFIFOHEAD(fd,29);
	WFIFOW(fd,0) = 0x82d;
	WFIFOW(fd,2) = 29;
	WFIFOB(fd,4) = sd->char_slots;
	WFIFOB(fd,5) = MAX_CHARS - sd->char_slots;
	WFIFOB(fd,6) = 0;
	WFIFOB(fd,7) = sd->char_slots;
	WFIFOB(fd,8) = sd->char_slots;
	memset(WFIFOP(fd,9), 0, 20); // unused bytes
	WFIFOSET(fd,29);
}
//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int char_mmo_char_send_characters(int fd, struct char_session_data* sd)
{
	int j, offset = 0;
	nullpo_ret(sd);
#if PACKETVER >= 20100413
	offset += 3;
#endif
	if (save_log)
		ShowInfo("Loading Char Data ("CL_BOLD"%d"CL_RESET")\n",sd->account_id);

	j = 24 + offset; // offset
	WFIFOHEAD(fd,j + MAX_CHARS*MAX_CHAR_BUF);
	WFIFOW(fd,0) = 0x6b;
#if PACKETVER >= 20100413
	WFIFOB(fd,4) = MAX_CHARS; // Max slots.
	WFIFOB(fd,5) = sd->char_slots; // Available slots. (aka PremiumStartSlot)
	WFIFOB(fd,6) = MAX_CHARS; // Premium slots. AKA any existent chars past sd->char_slots but within MAX_CHARS will show a 'Premium Service' in red
#endif
	memset(WFIFOP(fd,4 + offset), 0, 20); // unknown bytes
	j+=chr->mmo_chars_fromsql(sd, WFIFOP(fd,j));
	WFIFOW(fd,2) = j; // packet len
	WFIFOSET(fd,j);

	return 0;
}

int char_char_married(int pl1, int pl2)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `partner_id` FROM `%s` WHERE `char_id` = '%d'", char_db, pl1) )
		Sql_ShowDebug(inter->sql_handle);
	else if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		char* data;

		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		if( pl2 == atoi(data) )
		{
			SQL->FreeResult(inter->sql_handle);
			return 1;
		}
	}
	SQL->FreeResult(inter->sql_handle);
	return 0;
}

int char_char_child(int parent_id, int child_id)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `child` FROM `%s` WHERE `char_id` = '%d'", char_db, parent_id) )
		Sql_ShowDebug(inter->sql_handle);
	else if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		char* data;

		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		if( child_id == atoi(data) )
		{
			SQL->FreeResult(inter->sql_handle);
			return 1;
		}
	}
	SQL->FreeResult(inter->sql_handle);
	return 0;
}

int char_char_family(int cid1, int cid2, int cid3)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`,`partner_id`,`child` FROM `%s` WHERE `char_id` IN ('%d','%d','%d')", char_db, cid1, cid2, cid3) )
		Sql_ShowDebug(inter->sql_handle);
	else while( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		int charid;
		int partnerid;
		int childid;
		char* data;

		SQL->GetData(inter->sql_handle, 0, &data, NULL); charid = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); partnerid = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); childid = atoi(data);

		if( (cid1 == charid    && ((cid2 == partnerid && cid3 == childid  ) || (cid2 == childid   && cid3 == partnerid))) ||
			(cid1 == partnerid && ((cid2 == charid    && cid3 == childid  ) || (cid2 == childid   && cid3 == charid   ))) ||
			(cid1 == childid   && ((cid2 == charid    && cid3 == partnerid) || (cid2 == partnerid && cid3 == charid   ))) )
		{
			SQL->FreeResult(inter->sql_handle);
			return childid;
		}
	}
	SQL->FreeResult(inter->sql_handle);
	return 0;
}

//----------------------------------------------------------------------
// Force disconnection of an online player (with account value) by [Yor]
//----------------------------------------------------------------------
void char_disconnect_player(int account_id)
{
	int i;
	struct char_session_data* sd;

	// disconnect player if online on char-server
	ARR_FIND( 0, sockt->fd_max, i, sockt->session[i] && (sd = (struct char_session_data*)sockt->session[i]->session_data) && sd->account_id == account_id );
	if( i < sockt->fd_max )
		sockt->eof(i);
}

void char_authfail_fd(int fd, int type)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x81;
	WFIFOB(fd,2) = type;
	WFIFOSET(fd,3);
}

void char_request_account_data(int account_id)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x2716;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOSET(chr->login_fd,6);
}

static void char_auth_ok(int fd, struct char_session_data *sd)
{
	struct online_char_data* character;

	nullpo_retv(sd);

	if( (character = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id)) != NULL ) {
		// check if character is not online already. [Skotlex]
		if (character->server > -1) {
			//Character already online. KICK KICK KICK
			mapif->disconnectplayer(chr->server[character->server].fd, character->account_id, character->char_id, 2); // 2: Already connected to server
			if (character->waiting_disconnect == INVALID_TIMER)
				character->waiting_disconnect = timer->add(timer->gettick()+20000, chr->waiting_disconnect, character->account_id, 0);
			character->pincode_enable = -1;
			chr->authfail_fd(fd, 8);
			return;
		}
		if (character->fd >= 0 && character->fd != fd) {
			//There's already a connection from this account that hasn't picked a char yet.
			chr->authfail_fd(fd, 8);
			return;
		}
		character->fd = fd;
	}

	if (chr->login_fd > 0) {
		chr->request_account_data(sd->account_id);
	}

	// mark session as 'authed'
	sd->auth = true;

	// set char online on charserver
	chr->set_char_charselect(sd->account_id);

	// continues when account data is received...
}

void char_ping_login_server(int fd)
{
	WFIFOHEAD(fd,2);// sends a ping packet to login server (will receive pong 0x2718)
	WFIFOW(fd,0) = 0x2719;
	WFIFOSET(fd,2);
}

int char_parse_fromlogin_connection_state(int fd)
{
	if (RFIFOB(fd,2)) {
		//printf("connect login server error : %d\n", RFIFOB(fd,2));
		ShowError("Can not connect to login-server.\n");
		ShowError("The server communication passwords (default s1/p1) are probably invalid.\n");
		ShowError("Also, please make sure your login db has the correct communication username/passwords and the gender of the account is S.\n");
		ShowError("The communication passwords are set in /conf/map-server.conf and /conf/char-server.conf\n");
		sockt->eof(fd);
		return 1;
	} else {
		ShowStatus("Connected to login-server (connection #%d).\n", fd);
		loginif->on_ready();
	}
	RFIFOSKIP(fd,3);
	return 0;
}

// 0 - rejected from server
//
void char_auth_error(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x6c;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void char_parse_fromlogin_auth_state(int fd)
{
	struct char_session_data* sd = NULL;
	int account_id = RFIFOL(fd,2);
	uint32 login_id1 = RFIFOL(fd,6);
	uint32 login_id2 = RFIFOL(fd,10);
	uint8 sex = RFIFOB(fd,14);
	uint8 result = RFIFOB(fd,15);
	int request_id = RFIFOL(fd,16);
	uint32 version = RFIFOL(fd,20);
	uint8 clienttype = RFIFOB(fd,24);
	int group_id = RFIFOL(fd,25);
	unsigned int expiration_time = RFIFOL(fd, 29);
	RFIFOSKIP(fd,33);

	if (sockt->session_is_active(request_id) && (sd=(struct char_session_data*)sockt->session[request_id]->session_data) &&
		!sd->auth && sd->account_id == account_id && sd->login_id1 == login_id1 && sd->login_id2 == login_id2 && sd->sex == sex )
	{
		int client_fd = request_id;
		sd->version = version;
		sd->clienttype = clienttype;
		switch( result ) {
			case 0:// ok
				/* restrictions apply */
				if( chr->server_type == CST_MAINTENANCE && group_id < char_maintenance_min_group_id ) {
					chr->auth_error(client_fd, 0);
					break;
				}
				/* the client will already deny this request, this check is to avoid someone bypassing. */
				if( chr->server_type == CST_PAYING && (time_t)expiration_time < time(NULL) ) {
					chr->auth_error(client_fd, 0);
					break;
				}
				chr->auth_ok(client_fd, sd);
				break;
			case 1:// auth failed
				chr->auth_error(client_fd, 0);
				break;
		}
	}
}

void char_parse_fromlogin_account_data(int fd)
{
	struct char_session_data* sd = (struct char_session_data*)sockt->session[fd]->session_data;
	int i;
	// find the authenticated session with this account id
	ARR_FIND( 0, sockt->fd_max, i, sockt->session[i] && (sd = (struct char_session_data*)sockt->session[i]->session_data) && sd->auth && sd->account_id == RFIFOL(fd,2) );
	if( i < sockt->fd_max ) {
		memcpy(sd->email, RFIFOP(fd,6), 40);
		sd->expiration_time = (time_t)RFIFOL(fd,46);
		sd->group_id = RFIFOB(fd,50);
		sd->char_slots = RFIFOB(fd,51);
		if( sd->char_slots > MAX_CHARS ) {
			ShowError("Account '%d' `character_slots` column is higher than supported MAX_CHARS (%d), update MAX_CHARS in mmo.h! capping to MAX_CHARS...\n",sd->account_id,sd->char_slots);
			sd->char_slots = MAX_CHARS;/* cap to maximum */
		} else if ( sd->char_slots <= 0 )/* no value aka 0 in sql */
			sd->char_slots = MAX_CHARS;/* cap to maximum */
		safestrncpy(sd->birthdate, RFIFOP(fd,52), sizeof(sd->birthdate));
		safestrncpy(sd->pincode, RFIFOP(fd,63), sizeof(sd->pincode));
		sd->pincode_change = RFIFOL(fd,68);
		// continued from chr->auth_ok...
		if( (max_connect_user == 0 && sd->group_id != gm_allow_group) ||
			( max_connect_user > 0 && chr->count_users() >= max_connect_user && sd->group_id != gm_allow_group ) ) {
			// refuse connection (over populated)
			chr->auth_error(i, 0);
		} else {
			// send characters to player
	#if PACKETVER >= 20130000
			chr->mmo_char_send_slots_info(i, sd);
			chr->mmo_char_send_characters(i, sd);
	#else
			chr->mmo_char_send_characters(i, sd);
	#endif
	#if PACKETVER >= 20060819
			chr->mmo_char_send_ban_list(i, sd);
	#endif
	#if PACKETVER >= 20110309
			pincode->handle(i, sd);
	#endif
		}
	}
	RFIFOSKIP(fd,72);
}

void char_parse_fromlogin_login_pong(int fd)
{
	RFIFOSKIP(fd,2);
	if (sockt->session[fd])
		sockt->session[fd]->flag.ping = 0;
}

void char_changesex(int account_id, int sex)
{
	unsigned char buf[7];

	WBUFW(buf,0) = 0x2b0d;
	WBUFL(buf,2) = account_id;
	WBUFB(buf,6) = sex;
	mapif->sendall(buf, 7);
}

/**
 * Performs the necessary operations when changing a character's sex, such as
 * correcting the job class and unequipping items, and propagating the
 * information to the guild data.
 *
 * @param sex      The new sex (SEX_MALE or SEX_FEMALE).
 * @param acc      The character's account ID.
 * @param char_id  The character ID.
 * @param class_   The character's current job class.
 * @param guild_id The character's guild ID.
 */
void char_change_sex_sub(int sex, int acc, int char_id, int class_, int guild_id)
{
	// job modification
	if (class_ == JOB_BARD || class_ == JOB_DANCER)
		class_ = (sex == SEX_MALE ? JOB_BARD : JOB_DANCER);
	else if (class_ == JOB_CLOWN || class_ == JOB_GYPSY)
		class_ = (sex == SEX_MALE ? JOB_CLOWN : JOB_GYPSY);
	else if (class_ == JOB_BABY_BARD || class_ == JOB_BABY_DANCER)
		class_ = (sex == SEX_MALE ? JOB_BABY_BARD : JOB_BABY_DANCER);
	else if (class_ == JOB_MINSTREL || class_ == JOB_WANDERER)
		class_ = (sex == SEX_MALE ? JOB_MINSTREL : JOB_WANDERER);
	else if (class_ == JOB_MINSTREL_T || class_ == JOB_WANDERER_T)
		class_ = (sex == SEX_MALE ? JOB_MINSTREL_T : JOB_WANDERER_T);
	else if (class_ == JOB_BABY_MINSTREL || class_ == JOB_BABY_WANDERER)
		class_ = (sex == SEX_MALE ? JOB_BABY_MINSTREL : JOB_BABY_WANDERER);
	else if (class_ == JOB_KAGEROU || class_ == JOB_OBORO)
		class_ = (sex == SEX_MALE ? JOB_KAGEROU : JOB_OBORO);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `equip`='0' WHERE `char_id`='%d'", inventory_db, char_id))
		Sql_ShowDebug(inter->sql_handle);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `class`='%d', `weapon`='0', `shield`='0', "
				"`head_top`='0', `head_mid`='0', `head_bottom`='0' WHERE `char_id`='%d'",
				char_db, class_, char_id))
		Sql_ShowDebug(inter->sql_handle);
	if (guild_id) // If there is a guild, update the guild_member data [Skotlex]
		inter_guild->sex_changed(guild_id, acc, char_id, sex);
}

int char_parse_fromlogin_changesex_reply(int fd)
{
	int char_id = 0, class_ = 0, guild_id = 0;
	int i;
	struct char_auth_node *node;
	struct SqlStmt *stmt;

	int acc = RFIFOL(fd,2);
	int sex = RFIFOB(fd,6);

	RFIFOSKIP(fd,7);

	// This should _never_ happen
	if (acc <= 0) {
		ShowError("Received invalid account id from login server! (aid: %d)\n", acc);
		return 1;
	}

	node = (struct char_auth_node*)idb_get(auth_db, acc);
	if (node != NULL)
		node->sex = sex;

	// get characters
	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `char_id`,`class`,`guild_id` FROM `%s` WHERE `account_id` = '%d'", char_db, acc)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
	}
	SQL->StmtBindColumn(stmt, 0, SQLDT_INT, &char_id,  0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_INT, &class_,   0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_INT, &guild_id, 0, NULL, NULL);

	for (i = 0; i < MAX_CHARS && SQL_SUCCESS == SQL->StmtNextRow(stmt); ++i) {
		char_change_sex_sub(sex, acc, char_id, class_, guild_id);
	}
	SQL->StmtFree(stmt);

	// disconnect player if online on char-server
	chr->disconnect_player(acc);

	// notify all mapservers about this change
	chr->changesex(acc, sex);
	return 0;
}

void char_parse_fromlogin_account_reg2(int fd)
{
	//Receive account_reg2 registry, forward to map servers.
	mapif->sendall(RFIFOP(fd, 0), RFIFOW(fd,2));
	RFIFOSKIP(fd, RFIFOW(fd,2));
}

void mapif_ban(int id, unsigned int flag, int status)
{
	// send to all map-servers to disconnect the player
	unsigned char buf[11];
	WBUFW(buf,0) = 0x2b14;
	WBUFL(buf,2) = id;
	WBUFB(buf,6) = flag; // 0: change of status, 1: ban
	WBUFL(buf,7) = status; // status or final date of a banishment
	mapif->sendall(buf, 11);
}

void char_parse_fromlogin_ban(int fd)
{
	mapif->ban(RFIFOL(fd,2), RFIFOB(fd,6), RFIFOL(fd,7));
	// disconnect player if online on char-server
	chr->disconnect_player(RFIFOL(fd,2));
	RFIFOSKIP(fd,11);
}

void char_parse_fromlogin_kick(int fd)
{
	int aid = RFIFOL(fd,2);
	struct online_char_data* character = (struct online_char_data*)idb_get(chr->online_char_db, aid);
	RFIFOSKIP(fd,6);
	if( character != NULL )
	{// account is already marked as online!
		if( character->server > -1 ) {
			//Kick it from the map server it is on.
			mapif->disconnectplayer(chr->server[character->server].fd, character->account_id, character->char_id, 2); // 2: Already connected to server
			if (character->waiting_disconnect == INVALID_TIMER)
				character->waiting_disconnect = timer->add(timer->gettick()+AUTH_TIMEOUT, chr->waiting_disconnect, character->account_id, 0);
		}
		else
		{// Manual kick from char server.
			struct char_session_data *tsd;
			int i;
			ARR_FIND( 0, sockt->fd_max, i, sockt->session[i] && (tsd = (struct char_session_data*)sockt->session[i]->session_data) && tsd->account_id == aid );
			if( i < sockt->fd_max )
			{
				chr->authfail_fd(i, 2);
				sockt->eof(i);
			}
			else // still moving to the map-server
				chr->set_char_offline(-1, aid);
		}
	}
	idb_remove(auth_db, aid);// reject auth attempts from map-server
}

void char_update_ip(int fd)
{
	WFIFOHEAD(fd,6);
	WFIFOW(fd,0) = 0x2736;
	WFIFOL(fd,2) = htonl(chr->ip);
	WFIFOSET(fd,6);
}

void char_parse_fromlogin_update_ip(int fd)
{
	unsigned char buf[2];
	uint32 new_ip = 0;

	WBUFW(buf,0) = 0x2b1e;
	mapif->sendall(buf, 2);

	new_ip = sockt->host2ip(login_ip_str);
	if (new_ip && new_ip != login_ip)
		login_ip = new_ip; //Update login ip, too.

	new_ip = sockt->host2ip(char_ip_str);
	if (new_ip && new_ip != chr->ip) {
		//Update ip.
		chr->ip = new_ip;
		ShowInfo("Updating IP for [%s].\n", char_ip_str);
		// notify login server about the change
		chr->update_ip(fd);
	}
	RFIFOSKIP(fd,2);
}

void char_parse_fromlogin_accinfo2_failed(int fd)
{
	mapif->parse_accinfo2(false, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14),
	                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, 0, 0);
	RFIFOSKIP(fd,18);
}

void char_parse_fromlogin_accinfo2_ok(int fd)
{
	mapif->parse_accinfo2(true, RFIFOL(fd,167), RFIFOL(fd,171), RFIFOL(fd,175), RFIFOL(fd,179),
	                      RFIFOP(fd,2), RFIFOP(fd,26), RFIFOP(fd,59), RFIFOP(fd,99), RFIFOP(fd,119),
	                      RFIFOP(fd,151), RFIFOP(fd,156), RFIFOL(fd,115), RFIFOL(fd,143), RFIFOL(fd,147));
	RFIFOSKIP(fd,183);
}

int char_parse_fromlogin(int fd) {
	// only process data from the login-server
	if( fd != chr->login_fd ) {
		ShowDebug("chr->parse_fromlogin: Disconnecting invalid session #%d (is not the login-server)\n", fd);
		sockt->close(fd);
		return 0;
	}

	if( sockt->session[fd]->flag.eof ) {
		sockt->close(fd);
		chr->login_fd = -1;
		loginif->on_disconnect();
		return 0;
	} else if ( sockt->session[fd]->flag.ping ) {/* we've reached stall time */
		if( DIFF_TICK(sockt->last_tick, sockt->session[fd]->rdata_tick) > (sockt->stall_time * 2) ) {/* we can't wait any longer */
			sockt->eof(fd);
			return 0;
		} else if( sockt->session[fd]->flag.ping != 2 ) { /* we haven't sent ping out yet */
			chr->ping_login_server(fd);
			sockt->session[fd]->flag.ping = 2;
		}
	}

	while (RFIFOREST(fd) >= 2) {
		uint16 command = RFIFOW(fd,0);

		if (VECTOR_LENGTH(HPM->packets[hpParse_FromLogin]) > 0) {
			int result = HPM->parse_packets(fd,command,hpParse_FromLogin);
			if (result == 1)
				continue;
			if (result == 2)
				return 0;
		}

		switch (command) {
			// acknowledgment of connect-to-loginserver request
			case 0x2711:
				if (RFIFOREST(fd) < 3)
					return 0;
				if (chr->parse_fromlogin_connection_state(fd))
					return 0;
			break;

			// acknowledgment of account authentication request
			case 0x2713:
				if (RFIFOREST(fd) < 33)
					return 0;
			{
				chr->parse_fromlogin_auth_state(fd);
			}
			break;

			case 0x2717: // account data
			{
				if (RFIFOREST(fd) < 72)
					return 0;
				chr->parse_fromlogin_account_data(fd);
			}
			break;

			// login-server alive packet
			case 0x2718:
				if (RFIFOREST(fd) < 2)
					return 0;
				chr->parse_fromlogin_login_pong(fd);
			break;

			// changesex reply
			case 0x2723:
				if (RFIFOREST(fd) < 7)
					return 0;
			{
				if (chr->parse_fromlogin_changesex_reply(fd))
					return 0;
			}
			break;

			// reply to an account_reg2 registry request
			case 0x3804:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
					return 0;
				chr->parse_fromlogin_account_reg2(fd);
			break;

			// State change of account/ban notification (from login-server)
			case 0x2731:
				if (RFIFOREST(fd) < 11)
					return 0;
				chr->parse_fromlogin_ban(fd);
			break;

			// Login server request to kick a character out. [Skotlex]
			case 0x2734:
				if (RFIFOREST(fd) < 6)
					return 0;
			{
				chr->parse_fromlogin_kick(fd);
			}
			break;

			// ip address update signal from login server
			case 0x2735:
			{
				chr->parse_fromlogin_update_ip(fd);
			}
			break;

			case 0x2736: // Failed accinfo lookup to forward to mapserver
				if (RFIFOREST(fd) < 18)
					return 0;

				chr->parse_fromlogin_accinfo2_failed(fd);
			break;

			case 0x2737: // Successful accinfo lookup to forward to mapserver
				if (RFIFOREST(fd) < 183)
					return 0;

				chr->parse_fromlogin_accinfo2_ok(fd);
			break;

			default:
				ShowError("Unknown packet 0x%04x received from login-server, disconnecting.\n", command);
				sockt->eof(fd);
				return 0;
			}
	}

	RFIFOFLUSH(fd);
	return 0;
}

int char_request_accreg2(int account_id, int char_id)
{
	if (chr->login_fd > 0) {
		WFIFOHEAD(chr->login_fd,10);
		WFIFOW(chr->login_fd,0) = 0x272e;
		WFIFOL(chr->login_fd,2) = account_id;
		WFIFOL(chr->login_fd,6) = char_id;
		WFIFOSET(chr->login_fd,10);
		return 1;
	}
	return 0;
}

/**
 * Handles global account reg saving that continues with chr->global_accreg_to_login_add and global_accreg_to_send
 **/
void char_global_accreg_to_login_start (int account_id, int char_id) {
	WFIFOHEAD(chr->login_fd, 60000 + 300);
	WFIFOW(chr->login_fd,0)  = 0x2728;
	WFIFOW(chr->login_fd,2)  = 14;
	WFIFOL(chr->login_fd,4)  = account_id;
	WFIFOL(chr->login_fd,8)  = char_id;
	WFIFOW(chr->login_fd,12) = 0;/* count */
}

/**
 * Completes global account reg saving that starts chr->global_accreg_to_login_start and continues with chr->global_accreg_to_login_add
 **/
void char_global_accreg_to_login_send (void) {
	WFIFOSET(chr->login_fd, WFIFOW(chr->login_fd,2));
}

/**
 * Handles global account reg saving that starts chr->global_accreg_to_login_start and ends with global_accreg_to_send
 **/
void char_global_accreg_to_login_add (const char *key, unsigned int index, intptr_t val, bool is_string) {
	int nlen = WFIFOW(chr->login_fd, 2);
	size_t len = strlen(key)+1;

	WFIFOB(chr->login_fd, nlen) = (unsigned char)len;/* won't be higher; the column size is 32 */
	nlen += 1;

	safestrncpy(WFIFOP(chr->login_fd,nlen), key, len);
	nlen += len;

	WFIFOL(chr->login_fd, nlen) = index;
	nlen += 4;

	if( is_string ) {
		WFIFOB(chr->login_fd, nlen) = val ? 2 : 3;
		nlen += 1;

		if( val ) {
			char *sval = (char*)val;
			len = strlen(sval)+1;

			WFIFOB(chr->login_fd, nlen) = (unsigned char)len;/* won't be higher; the column size is 254 */
			nlen += 1;

			safestrncpy(WFIFOP(chr->login_fd,nlen), sval, len);
			nlen += len;
		}
	} else {
		WFIFOB(chr->login_fd, nlen) = val ? 0 : 1;
		nlen += 1;

		if( val ) {
			WFIFOL(chr->login_fd, nlen) = (int)val;
			nlen += 4;
		}
	}

	WFIFOW(chr->login_fd,12) += 1;

	WFIFOW(chr->login_fd, 2) = nlen;
	if( WFIFOW(chr->login_fd, 2) > 60000 ) {
		int account_id = WFIFOL(chr->login_fd,4), char_id = WFIFOL(chr->login_fd,8);
		chr->global_accreg_to_login_send();
		chr->global_accreg_to_login_start(account_id,char_id);/* prepare next */
	}
}

void char_read_fame_list(void) {
	int i;
	char* data;
	size_t len;

	// Empty ranking lists
	memset(smith_fame_list, 0, sizeof(smith_fame_list));
	memset(chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset(taekwon_fame_list, 0, sizeof(taekwon_fame_list));
	// Build Blacksmith ranking list
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_BLACKSMITH, JOB_WHITESMITH, JOB_BABY_BLACKSMITH, JOB_MECHANIC, JOB_MECHANIC_T, JOB_BABY_MECHANIC, fame_list_size_smith) )
		Sql_ShowDebug(inter->sql_handle);
	for( i = 0; i < fame_list_size_smith && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		// char_id
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		smith_fame_list[i].id = atoi(data);
		// fame
		SQL->GetData(inter->sql_handle, 1, &data, &len);
		smith_fame_list[i].fame = atoi(data);
		// name
		SQL->GetData(inter->sql_handle, 2, &data, &len);
		memcpy(smith_fame_list[i].name, data, min(len, NAME_LENGTH));
	}
	// Build Alchemist ranking list
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_ALCHEMIST, JOB_CREATOR, JOB_BABY_ALCHEMIST, JOB_GENETIC, JOB_GENETIC_T, JOB_BABY_GENETIC, fame_list_size_chemist) )
		Sql_ShowDebug(inter->sql_handle);
	for( i = 0; i < fame_list_size_chemist && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		// char_id
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		chemist_fame_list[i].id = atoi(data);
		// fame
		SQL->GetData(inter->sql_handle, 1, &data, &len);
		chemist_fame_list[i].fame = atoi(data);
		// name
		SQL->GetData(inter->sql_handle, 2, &data, &len);
		memcpy(chemist_fame_list[i].name, data, min(len, NAME_LENGTH));
	}
	// Build Taekwon ranking list
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_TAEKWON, fame_list_size_taekwon) )
		Sql_ShowDebug(inter->sql_handle);
	for( i = 0; i < fame_list_size_taekwon && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		// char_id
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		taekwon_fame_list[i].id = atoi(data);
		// fame
		SQL->GetData(inter->sql_handle, 1, &data, &len);
		taekwon_fame_list[i].fame = atoi(data);
		// name
		SQL->GetData(inter->sql_handle, 2, &data, &len);
		memcpy(taekwon_fame_list[i].name, data, min(len, NAME_LENGTH));
	}
	SQL->FreeResult(inter->sql_handle);
}

// Send map-servers the fame ranking lists
int char_send_fame_list(int fd) {
	int i, len = 8;
	unsigned char buf[32000];

	WBUFW(buf,0) = 0x2b1b;

	for(i = 0; i < fame_list_size_smith && smith_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &smith_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add blacksmith's block length
	WBUFW(buf, 6) = len;

	for(i = 0; i < fame_list_size_chemist && chemist_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &chemist_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add alchemist's block length
	WBUFW(buf, 4) = len;

	for(i = 0; i < fame_list_size_taekwon && taekwon_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &taekwon_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add total packet length
	WBUFW(buf, 2) = len;

	if (fd != -1)
		mapif->send(fd, buf, len);
	else
		mapif->sendall(buf, len);

	return 0;
}

void char_update_fame_list(int type, int index, int fame) {
	unsigned char buf[8];
	WBUFW(buf,0) = 0x2b22;
	WBUFB(buf,2) = type;
	WBUFB(buf,3) = index;
	WBUFL(buf,4) = fame;
	mapif->sendall(buf, 8);
}

//Loads a character's name and stores it in the buffer given (must be NAME_LENGTH in size) and not NULL
//Returns 1 on found, 0 on not found (buffer is filled with Unknown char name)
int char_loadName(int char_id, char* name)
{
	char* data;
	size_t len;

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `name` FROM `%s` WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);
	else if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		SQL->GetData(inter->sql_handle, 0, &data, &len);
		safestrncpy(name, data, NAME_LENGTH);
		return 1;
	}
	else
	{
		safestrncpy(name, unknown_char_name, NAME_LENGTH);
	}
	return 0;
}

/// Initializes a server structure.
void mapif_server_init(int id)
{
	//memset(&chr->server[id], 0, sizeof(server[id]));
	chr->server[id].fd = -1;
}

/// Destroys a server structure.
void mapif_server_destroy(int id)
{
	if( chr->server[id].fd == -1 )
	{
		sockt->close(chr->server[id].fd);
		chr->server[id].fd = -1;
	}
}


/// Resets all the data related to a server.
void mapif_server_reset(int id)
{
	int i,j;
	unsigned char buf[16384];
	int fd = chr->server[id].fd;
	//Notify other map servers that this one is gone. [Skotlex]
	WBUFW(buf,0) = 0x2b20;
	WBUFL(buf,4) = htonl(chr->server[id].ip);
	WBUFW(buf,8) = htons(chr->server[id].port);
	j = 0;
	for (i = 0; i < VECTOR_LENGTH(chr->server[id].maps); i++) {
		uint16 m = VECTOR_INDEX(chr->server[id].maps, i);
		if (m != 0)
			WBUFW(buf,10+(j++)*4) = m;
	}
	if (j > 0) {
		WBUFW(buf,2) = j * 4 + 10;
		mapif->sendallwos(fd, buf, WBUFW(buf,2));
	}
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `index`='%d'", ragsrvinfo_db, chr->server[id].fd) )
		Sql_ShowDebug(inter->sql_handle);
	chr->online_char_db->foreach(chr->online_char_db,chr->db_setoffline,id); //Tag relevant chars as 'in disconnected' server.
	mapif->server_destroy(id);
	mapif->server_init(id);
}

/// Called when the connection to a Map Server is disconnected.
void mapif_on_disconnect(int id)
{
	ShowStatus("Map-server #%d has disconnected.\n", id);
	mapif->server_reset(id);
}

void mapif_on_parse_accinfo(int account_id, int u_fd, int u_aid, int u_group, int map_fd) {
	Assert_retv(chr->login_fd > 0);
	WFIFOHEAD(chr->login_fd,22);
	WFIFOW(chr->login_fd,0) = 0x2740;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOL(chr->login_fd,6) = u_fd;
	WFIFOL(chr->login_fd,10) = u_aid;
	WFIFOL(chr->login_fd,14) = u_group;
	WFIFOL(chr->login_fd,18) = map_fd;
	WFIFOSET(chr->login_fd,22);
}

void char_parse_frommap_datasync(int fd)
{
	sockt->datasync(fd, false);
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void char_parse_frommap_skillid2idx(int fd)
{
	int i;
	int j = RFIFOW(fd, 2) - 4;

	memset(&skillid2idx, 0, sizeof(skillid2idx));
	if( j )
		j /= 4;
	for(i = 0; i < j; i++) {
		if( RFIFOW(fd, 4 + (i*4)) > MAX_SKILL_ID ) {
			ShowWarning("Error skillid2dx[%d] = %d failed, %d is higher than MAX_SKILL_ID (%d)\n",RFIFOW(fd, 4 + (i*4)), RFIFOW(fd, 6 + (i*4)),RFIFOW(fd, 4 + (i*4)),MAX_SKILL_ID);
			continue;
		}
		skillid2idx[RFIFOW(fd, 4 + (i*4))] = RFIFOW(fd, 6 + (i*4));
	}
	RFIFOSKIP(fd, RFIFOW(fd, 2));
}

void char_map_received_ok(int fd)
{
	WFIFOHEAD(fd, 3 + NAME_LENGTH);
	WFIFOW(fd,0) = 0x2afb;
	WFIFOB(fd,2) = 0;
	memcpy(WFIFOP(fd,3), wisp_server_name, NAME_LENGTH);
	WFIFOSET(fd,3+NAME_LENGTH);
}

void char_send_maps(int fd, int id, int j)
{
	int k,i;

	if (j == 0) {
		ShowWarning("Map-server %d has NO maps.\n", id);
	} else {
		unsigned char buf[16384];
		// Transmitting maps information to the other map-servers
		WBUFW(buf,0) = 0x2b04;
		WBUFW(buf,2) = j * 4 + 10;
		WBUFL(buf,4) = htonl(chr->server[id].ip);
		WBUFW(buf,8) = htons(chr->server[id].port);
		memcpy(WBUFP(buf,10), RFIFOP(fd,4), j * 4);
		mapif->sendallwos(fd, buf, WBUFW(buf,2));
	}
	// Transmitting the maps of the other map-servers to the new map-server
	for(k = 0; k < ARRAYLENGTH(chr->server); k++) {
		if (chr->server[k].fd > 0 && k != id) {
			WFIFOHEAD(fd,10 + 4 * VECTOR_LENGTH(chr->server[k].maps));
			WFIFOW(fd,0) = 0x2b04;
			WFIFOL(fd,4) = htonl(chr->server[k].ip);
			WFIFOW(fd,8) = htons(chr->server[k].port);
			j = 0;
			for(i = 0; i < VECTOR_LENGTH(chr->server[k].maps); i++) {
				uint16 m = VECTOR_INDEX(chr->server[k].maps, i);
				if (m != 0)
					WFIFOW(fd,10+(j++)*4) = m;
			}
			if (j > 0) {
				WFIFOW(fd,2) = j * 4 + 10;
				WFIFOSET(fd,WFIFOW(fd,2));
			}
		}
	}
}

void char_parse_frommap_map_names(int fd, int id)
{
	int i;

	VECTOR_CLEAR(chr->server[id].maps);
	VECTOR_ENSURE(chr->server[id].maps, (RFIFOW(fd, 2) - 4) / 4, 1);
	for (i = 4; i < RFIFOW(fd,2); i += 4) {
		VECTOR_PUSH(chr->server[id].maps, RFIFOW(fd,i));
	}

	ShowStatus("Map-Server %d connected: %d maps, from IP %u.%u.%u.%u port %d.\n",
			id, (int)VECTOR_LENGTH(chr->server[id].maps), CONVIP(chr->server[id].ip), chr->server[id].port);
	ShowStatus("Map-server %d loading complete.\n", id);

	// send name for wisp to player
	chr->map_received_ok(fd);
	chr->send_fame_list(fd); //Send fame list.
	chr->send_maps(fd, id, (int)VECTOR_LENGTH(chr->server[id].maps));
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void char_send_scdata(int fd, int aid, int cid)
{
	#ifdef ENABLE_SC_SAVING
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `type`, `tick`, `val1`, `val2`, `val3`, `val4` "
		"FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'",
		scdata_db, aid, cid) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return;
	}
	if( SQL->NumRows(inter->sql_handle) > 0 ) {
		struct status_change_data scdata;
		int count;
		char* data;

		memset(&scdata, 0, sizeof(scdata));
		WFIFOHEAD(fd,14+50*sizeof(struct status_change_data));
		WFIFOW(fd,0) = 0x2b1d;
		WFIFOL(fd,4) = aid;
		WFIFOL(fd,8) = cid;
		for( count = 0; count < 50 && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++count )
		{
			SQL->GetData(inter->sql_handle, 0, &data, NULL); scdata.type = atoi(data);
			SQL->GetData(inter->sql_handle, 1, &data, NULL); scdata.tick = atoi(data);
			SQL->GetData(inter->sql_handle, 2, &data, NULL); scdata.val1 = atoi(data);
			SQL->GetData(inter->sql_handle, 3, &data, NULL); scdata.val2 = atoi(data);
			SQL->GetData(inter->sql_handle, 4, &data, NULL); scdata.val3 = atoi(data);
			SQL->GetData(inter->sql_handle, 5, &data, NULL); scdata.val4 = atoi(data);
			memcpy(WFIFOP(fd, 14+count*sizeof(struct status_change_data)), &scdata, sizeof(struct status_change_data));
		}
		if (count >= 50)
			ShowWarning("Too many status changes for %d:%d, some of them were not loaded.\n", aid, cid);
		if (count > 0) {
			WFIFOW(fd,2) = 14 + count*sizeof(struct status_change_data);
			WFIFOW(fd,12) = count;
			WFIFOSET(fd,WFIFOW(fd,2));

			//Clear the data once loaded.
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, aid, cid) )
				Sql_ShowDebug(inter->sql_handle);
		}
	} else { //no sc (needs a response)
		WFIFOHEAD(fd,14);
		WFIFOW(fd,0) = 0x2b1d;
		WFIFOW(fd,2) = 14;
		WFIFOL(fd,4) = aid;
		WFIFOL(fd,8) = cid;
		WFIFOW(fd,12) = 0;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	SQL->FreeResult(inter->sql_handle);
	#endif
}

void char_parse_frommap_request_scdata(int fd)
{
	#ifdef ENABLE_SC_SAVING
	int aid = RFIFOL(fd,2);
	int cid = RFIFOL(fd,6);
	chr->send_scdata(fd, aid, cid);
	#endif
	RFIFOSKIP(fd, 10);
}

void char_parse_frommap_set_users_count(int fd, int id)
{
	if (RFIFOW(fd,2) != chr->server[id].users) {
		chr->server[id].users = RFIFOW(fd,2);
		ShowInfo("User Count: %d (Server: %d)\n", chr->server[id].users, id);
	}
	RFIFOSKIP(fd, 4);
}

void char_parse_frommap_set_users(int fd, int id)
{
	//TODO: When data mismatches memory, update guild/party online/offline states.
	int i;

	chr->server[id].users = RFIFOW(fd,4);
	chr->online_char_db->foreach(chr->online_char_db,chr->db_setoffline,id); //Set all chars from this server as 'unknown'
	for(i = 0; i < chr->server[id].users; i++) {
		int aid = RFIFOL(fd,6+i*8);
		int cid = RFIFOL(fd,6+i*8+4);
		struct online_char_data *character = idb_ensure(chr->online_char_db, aid, chr->create_online_char_data);
		if (character->server > -1 && character->server != id) {
			ShowNotice("Set map user: Character (%d:%d) marked on map server %d, but map server %d claims to have (%d:%d) online!\n",
				character->account_id, character->char_id, character->server, id, aid, cid);
			mapif->disconnectplayer(chr->server[character->server].fd, character->account_id, character->char_id, 2); // 2: Already connected to server
		}
		character->server = id;
		character->char_id = cid;
	}
	//If any chars remain in -2, they will be cleaned in the cleanup timer.
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void char_save_character_ack(int fd, int aid, int cid)
{
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x2b21; //Save ack only needed on final save.
	WFIFOL(fd,2) = aid;
	WFIFOL(fd,6) = cid;
	WFIFOSET(fd,10);
}

void char_parse_frommap_save_character(int fd, int id)
{
	int aid = RFIFOL(fd,4), cid = RFIFOL(fd,8), size = RFIFOW(fd,2);
	struct online_char_data* character;

	if (size - 13 != sizeof(struct mmo_charstatus)) {
		ShowError("parse_from_map (save-char): Size mismatch! %d != %"PRIuS"\n", size-13, sizeof(struct mmo_charstatus));
		RFIFOSKIP(fd,size);
		return;
	}
	//Check account only if this ain't final save. Final-save goes through because of the char-map reconnect
	if (RFIFOB(fd,12)
	 || ( (character = (struct online_char_data*)idb_get(chr->online_char_db, aid)) != NULL
	    && character->char_id == cid)
	) {
		struct mmo_charstatus char_dat;
		memcpy(&char_dat, RFIFOP(fd,13), sizeof(struct mmo_charstatus));
		chr->mmo_char_tosql(cid, &char_dat);
	} else {
		//This may be valid on char-server reconnection, when re-sending characters that already logged off.
		ShowError("parse_from_map (save-char): Received data for non-existing/offline character (%d:%d).\n", aid, cid);
		chr->set_char_online(id, cid, aid);
	}

	if (RFIFOB(fd,12)) {
		//Flag, set character offline after saving. [Skotlex]
		chr->set_char_offline(cid, aid);
		chr->save_character_ack(fd, aid, cid);
	}
	RFIFOSKIP(fd,size);
}

// 0 - not ok
// 1 - ok
void char_select_ack(int fd, int account_id, uint8 flag)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x2b03;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag;
	WFIFOSET(fd,7);
}

void char_parse_frommap_char_select_req(int fd)
{
	int account_id = RFIFOL(fd,2);
	uint32 login_id1 = RFIFOL(fd,6);
	uint32 login_id2 = RFIFOL(fd,10);
	uint32 ip = RFIFOL(fd,14);
	int32 group_id = RFIFOL(fd, 18);
	RFIFOSKIP(fd,22);

	if( core->runflag != CHARSERVER_ST_RUNNING )
	{
		chr->select_ack(fd, account_id, 0);
	}
	else
	{
		struct char_auth_node* node;

		// create temporary auth entry
		CREATE(node, struct char_auth_node, 1);
		node->account_id = account_id;
		node->char_id = 0;
		node->login_id1 = login_id1;
		node->login_id2 = login_id2;
		node->group_id = group_id;
		//node->sex = 0;
		node->ip = ntohl(ip);
		/* sounds troublesome. */
		//node->expiration_time = 0; // unlimited/unknown time by default (not display in map-server)
		//node->gmlevel = 0;
		idb_put(auth_db, account_id, node);

		//Set char to "@ char select" in online db [Kevin]
		chr->set_char_charselect(account_id);
		chr->select_ack(fd, account_id, 1);
	}
}

void char_change_map_server_ack(int fd, const uint8 *data, bool ok)
{
	WFIFOHEAD(fd,30);
	WFIFOW(fd,0) = 0x2b06;
	memcpy(WFIFOP(fd,2), data, 28);
	if (!ok)
		WFIFOL(fd,6) = 0; //Set login1 to 0.
	WFIFOSET(fd,30);
}

void char_parse_frommap_change_map_server(int fd)
{
	int map_id, map_fd = -1;
	struct mmo_charstatus* char_data;

	map_id = chr->search_mapserver(RFIFOW(fd,18), ntohl(RFIFOL(fd,24)), ntohs(RFIFOW(fd,28))); //Locate mapserver by ip and port.
	if (map_id >= 0)
		map_fd = chr->server[map_id].fd;
	//Char should just had been saved before this packet, so this should be safe. [Skotlex]
	char_data = (struct mmo_charstatus*)uidb_get(chr->char_db_,RFIFOL(fd,14));
	if (char_data == NULL) {
		//Really shouldn't happen.
		struct mmo_charstatus char_dat;
		chr->mmo_char_fromsql(RFIFOL(fd,14), &char_dat, true);
		char_data = (struct mmo_charstatus*)uidb_get(chr->char_db_,RFIFOL(fd,14));
	}

	if (core->runflag == CHARSERVER_ST_RUNNING && sockt->session_is_active(map_fd) && char_data) {
		//Send the map server the auth of this player.
		struct online_char_data* data;
		struct char_auth_node* node;

		//Update the "last map" as this is where the player must be spawned on the new map server.
		char_data->last_point.map = RFIFOW(fd,18);
		char_data->last_point.x = RFIFOW(fd,20);
		char_data->last_point.y = RFIFOW(fd,22);
		char_data->sex = RFIFOB(fd,30);

		// create temporary auth entry
		CREATE(node, struct char_auth_node, 1);
		node->account_id = RFIFOL(fd,2);
		node->char_id = RFIFOL(fd,14);
		node->login_id1 = RFIFOL(fd,6);
		node->login_id2 = RFIFOL(fd,10);
		node->sex = RFIFOB(fd,30);
		node->expiration_time = 0; // FIXME (this thing isn't really supported we could as well purge it instead of fixing)
		node->ip = ntohl(RFIFOL(fd,31));
		node->group_id = RFIFOL(fd,35);
		node->changing_mapservers = 1;
		idb_put(auth_db, RFIFOL(fd,2), node);

		data = idb_ensure(chr->online_char_db, RFIFOL(fd,2), chr->create_online_char_data);
		data->char_id = char_data->char_id;
		data->server = map_id; //Update server where char is.

		//Reply with an ack.
		chr->change_map_server_ack(fd, RFIFOP(fd,2), true);
	} else { //Reply with nak
		chr->change_map_server_ack(fd, RFIFOP(fd,2), false);
	}
	RFIFOSKIP(fd,39);
}

void char_parse_frommap_remove_friend(int fd)
{
	int char_id = RFIFOL(fd,2);
	int friend_id = RFIFOL(fd,6);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d' AND `friend_id`='%d' LIMIT 1",
		friend_db, char_id, friend_id) ) {
		Sql_ShowDebug(inter->sql_handle);
	}
	RFIFOSKIP(fd,10);
}

void char_char_name_ack(int fd, int char_id)
{
	WFIFOHEAD(fd,30);
	WFIFOW(fd,0) = 0x2b09;
	WFIFOL(fd,2) = char_id;
	chr->loadName(char_id, WFIFOP(fd,6));
	WFIFOSET(fd,30);
}

void char_parse_frommap_char_name_request(int fd)
{
	chr->char_name_ack(fd, RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
}

void char_parse_frommap_change_email(int fd)
{
	if (chr->login_fd > 0) { // don't send request if no login-server
		WFIFOHEAD(chr->login_fd,86);
		memcpy(WFIFOP(chr->login_fd,0), RFIFOP(fd,0),86); // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
		WFIFOW(chr->login_fd,0) = 0x2722;
		WFIFOSET(chr->login_fd,86);
	}
	RFIFOSKIP(fd, 86);
}

void mapif_char_ban(int char_id, time_t timestamp)
{
	unsigned char buf[11];
	WBUFW(buf,0) = 0x2b14;
	WBUFL(buf,2) = char_id;
	WBUFB(buf,6) = 2;
	WBUFL(buf,7) = (unsigned int)timestamp;
	mapif->sendall(buf, 11);
}

void char_ban(int account_id, int char_id, time_t *unban_time, short year, short month, short day, short hour, short minute, short second)
{
	time_t timestamp;
	struct tm *tmtime;
	struct SqlStmt *stmt = SQL->StmtMalloc(inter->sql_handle);

	nullpo_retv(unban_time);

	if (*unban_time == 0 || *unban_time < time(NULL))
		timestamp = time(NULL); // new ban
	else
		timestamp = *unban_time; // add to existing ban

	tmtime = localtime(&timestamp);
	tmtime->tm_year = tmtime->tm_year + year;
	tmtime->tm_mon  = tmtime->tm_mon + month;
	tmtime->tm_mday = tmtime->tm_mday + day;
	tmtime->tm_hour = tmtime->tm_hour + hour;
	tmtime->tm_min  = tmtime->tm_min + minute;
	tmtime->tm_sec  = tmtime->tm_sec + second;
	timestamp = mktime(tmtime);

	if( SQL_SUCCESS != SQL->StmtPrepare(stmt,
		"UPDATE `%s` SET `unban_time` = ? WHERE `char_id` = ? LIMIT 1",
		char_db)
	   || SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_LONG, &timestamp, sizeof(timestamp))
	   || SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_INT,  &char_id,   sizeof(char_id))
	   || SQL_SUCCESS != SQL->StmtExecute(stmt)
	) {
		SqlStmt_ShowDebug(stmt);
	}

	SQL->StmtFree(stmt);

	// condition applies; send to all map-servers to disconnect the player
	if( timestamp > time(NULL) ) {
		mapif->char_ban(char_id, timestamp);
		// disconnect player if online on char-server
		chr->disconnect_player(account_id);
	}
}

void char_unban(int char_id, int *result)
{
	/* handled by char server, so no redirection */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `unban_time` = '0' WHERE `char_id` = '%d' LIMIT 1", char_db, char_id) ) {
		Sql_ShowDebug(inter->sql_handle);
		if (result)
			*result = 1;
	}
}

void char_ask_name_ack(int fd, int acc, const char* name, int type, int result)
{
	nullpo_retv(name);
	WFIFOHEAD(fd,34);
	WFIFOW(fd, 0) = 0x2b0f;
	WFIFOL(fd, 2) = acc;
	safestrncpy(WFIFOP(fd,6), name, NAME_LENGTH);
	WFIFOW(fd,30) = type;
	WFIFOW(fd,32) = result;
	WFIFOSET(fd,34);
}

/**
 * Changes a character's sex.
 * The information is updated on database, and the character is kicked if it
 * currently is online.
 *
 * @param char_id The character's ID.
 * @param sex     The new sex.
 * @retval 0 in case of success.
 * @retval 1 in case of failure.
 */
int char_changecharsex(int char_id, int sex)
{
	int class_ = 0, guild_id = 0, account_id = 0;
	char *data;

	// get character data
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`,`class`,`guild_id` FROM `%s` WHERE `char_id` = '%d'", char_db, char_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return 1;
	}
	if (SQL->NumRows(inter->sql_handle) != 1 || SQL_ERROR == SQL->NextRow(inter->sql_handle)) {
		SQL->FreeResult(inter->sql_handle);
		return 1;
	}
	SQL->GetData(inter->sql_handle, 0, &data, NULL); account_id = atoi(data);
	SQL->GetData(inter->sql_handle, 1, &data, NULL); class_ = atoi(data);
	SQL->GetData(inter->sql_handle, 2, &data, NULL); guild_id = atoi(data);
	SQL->FreeResult(inter->sql_handle);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `sex` = '%c' WHERE `char_id` = '%d'", char_db, sex == SEX_MALE ? 'M' : 'F', char_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return 1;
	}
	char_change_sex_sub(sex, account_id, char_id, class_, guild_id);

	// disconnect player if online on char-server
	chr->disconnect_player(account_id);

	// notify all mapservers about this change
	chr->changesex(account_id, sex);
	return 0;
}

void char_parse_frommap_change_account(int fd)
{
	int result = 0; // 0-login-server request done, 1-player not found, 2-gm level too low, 3-login-server offline
	char esc_name[NAME_LENGTH*2+1];

	int acc = RFIFOL(fd,2); // account_id of who ask (-1 if server itself made this request)
	const char *name = RFIFOP(fd,6); // name of the target character
	int type = RFIFOW(fd,30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban, 5 changesex, 6 charban, 7 charunban
	short year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	int sex = SEX_MALE;
	if (type == 2 || type == 6) {
		year = RFIFOW(fd,32);
		month = RFIFOW(fd,34);
		day = RFIFOW(fd,36);
		hour = RFIFOW(fd,38);
		minute = RFIFOW(fd,40);
		second = RFIFOW(fd,42);
	} else if (type == 8) {
		sex = RFIFOB(fd, 32);
	}
	RFIFOSKIP(fd,44);

	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));

	if(SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`,`char_id`,`unban_time` FROM `%s` WHERE `name` = '%s'", char_db, esc_name)) {
		Sql_ShowDebug(inter->sql_handle);
	} else if (SQL->NumRows(inter->sql_handle) == 0) {
		SQL->FreeResult(inter->sql_handle);
		result = 1; // 1-player not found
	} else if (SQL_SUCCESS != SQL->NextRow(inter->sql_handle)) {
		Sql_ShowDebug(inter->sql_handle);
		SQL->FreeResult(inter->sql_handle);
		result = 1; // 1-player not found
	} else {
		int account_id, char_id;
		char *data;
		time_t unban_time;

		SQL->GetData(inter->sql_handle, 0, &data, NULL); account_id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); char_id = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); unban_time = atol(data);
		SQL->FreeResult(inter->sql_handle);

		if( chr->login_fd <= 0 ) {
			result = 3; // 3-login-server offline
#if 0 //FIXME: need to move this check to login server [ultramage]
		} else if( acc != -1 && isGM(acc) < isGM(account_id) ) {
			result = 2; // 2-gm level too low
#endif // 0
		} else {
			switch (type) {
			case CHAR_ASK_NAME_BLOCK:
				loginif->block_account(account_id, 5);
				break;
			case CHAR_ASK_NAME_BAN:
				loginif->ban_account(account_id, year, month, day, hour, minute, second);
				break;
			case CHAR_ASK_NAME_UNBLOCK:
				loginif->block_account(account_id, 0);
				break;
			case CHAR_ASK_NAME_UNBAN:
				loginif->unban_account(account_id);
				break;
			case CHAR_ASK_NAME_CHANGESEX:
				loginif->changesex(account_id);
				break;
			case CHAR_ASK_NAME_CHARBAN:
				/* handled by char server, so no redirection */
				chr->ban(account_id, char_id, &unban_time, year, month, day, hour, minute, second);
				break;
			case CHAR_ASK_NAME_CHARUNBAN:
				chr->unban(char_id, &result);
				break;
			case CHAR_ASK_NAME_CHANGECHARSEX:
				result = chr->changecharsex(char_id, sex);
				break;
			}
		}
	}

	// send answer if a player ask, not if the server ask
	if (acc != -1 && type != CHAR_ASK_NAME_CHANGESEX && type != CHAR_ASK_NAME_CHANGECHARSEX) { // Don't send answer for changesex
		chr->ask_name_ack(fd, acc, name, type, result);
	}
}

void char_parse_frommap_fame_list(int fd)
{
	int cid = RFIFOL(fd, 2);
	int fame = RFIFOL(fd, 6);
	char type = RFIFOB(fd, 10);
	int size;
	struct fame_list* list;
	int player_pos;
	int fame_pos;

	switch(type) {
		case RANKTYPE_BLACKSMITH: size = fame_list_size_smith;   list = smith_fame_list;   break;
		case RANKTYPE_ALCHEMIST:  size = fame_list_size_chemist; list = chemist_fame_list; break;
		case RANKTYPE_TAEKWON:    size = fame_list_size_taekwon; list = taekwon_fame_list; break;
		default:                  size = 0;                      list = NULL;              break;
	}

	if (!list) {
		RFIFOSKIP(fd, 11);
		return;
	}
	ARR_FIND(0, size, player_pos, list[player_pos].id == cid);// position of the player
	ARR_FIND(0, size, fame_pos, list[fame_pos].fame <= fame);// where the player should be

	if( player_pos == size && fame_pos == size )
		;// not on list and not enough fame to get on it
	else if( fame_pos == player_pos ) {
		// same position
		list[player_pos].fame = fame;
		chr->update_fame_list(type, player_pos, fame);
	} else {
		// move in the list
		if( player_pos == size ) {
			// new ranker - not in the list
			ARR_MOVE(size - 1, fame_pos, list, struct fame_list);
			list[fame_pos].id = cid;
			list[fame_pos].fame = fame;
			chr->loadName(cid, list[fame_pos].name);
		} else {
			// already in the list
			if( fame_pos == size )
				--fame_pos;// move to the end of the list
			ARR_MOVE(player_pos, fame_pos, list, struct fame_list);
			list[fame_pos].fame = fame;
		}
		chr->send_fame_list(-1);
	}

	RFIFOSKIP(fd,11);
}

void char_parse_frommap_divorce_char(int fd)
{
	chr->divorce_char_sql(RFIFOL(fd,2), RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
}

void char_parse_frommap_ragsrvinfo(int fd)
{
	char esc_server_name[sizeof(chr->server_name)*2+1];

	SQL->EscapeString(inter->sql_handle, esc_server_name, chr->server_name);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` SET `index`='%d',`name`='%s',`exp`='%u',`jexp`='%u',`drop`='%u'",
		ragsrvinfo_db, fd, esc_server_name, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)) )
	{
		Sql_ShowDebug(inter->sql_handle);
	}
	RFIFOSKIP(fd,14);
}

void char_parse_frommap_set_char_offline(int fd)
{
	chr->set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
}

void char_parse_frommap_set_all_offline(int fd, int id)
{
	chr->set_all_offline(id);
	RFIFOSKIP(fd,2);
}

void char_parse_frommap_set_char_online(int fd, int id)
{
	chr->set_char_online(id, RFIFOL(fd,2),RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
}

void char_parse_frommap_build_fame_list(int fd)
{
	chr->read_fame_list();
	chr->send_fame_list(-1);
	RFIFOSKIP(fd,2);
}

void char_parse_frommap_save_status_change_data(int fd)
{
	#ifdef ENABLE_SC_SAVING
	int aid = RFIFOL(fd, 4);
	int cid = RFIFOL(fd, 8);
	int count = RFIFOW(fd, 12);

	/* clear; ensure no left overs e.g. permanent */
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, aid, cid) )
		Sql_ShowDebug(inter->sql_handle);

	if( count > 0 )
	{
		struct status_change_data data;
		StringBuf buf;
		int i;

		StrBuf->Init(&buf);
		StrBuf->Printf(&buf, "INSERT INTO `%s` (`account_id`, `char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ", scdata_db);
		for( i = 0; i < count; ++i )
		{
			memcpy (&data, RFIFOP(fd, 14+i*sizeof(struct status_change_data)), sizeof(struct status_change_data));
			if( i > 0 )
				StrBuf->AppendStr(&buf, ", ");
			StrBuf->Printf(&buf, "('%d','%d','%hu','%d','%d','%d','%d','%d')", aid, cid,
				data.type, data.tick, data.val1, data.val2, data.val3, data.val4);
		}
		if( SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)) )
			Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
	}
	#endif
	RFIFOSKIP(fd, RFIFOW(fd, 2));
}

void char_send_pong(int fd)
{
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2b24;
	WFIFOSET(fd,2);
}

void char_parse_frommap_ping(int fd)
{
	chr->send_pong(fd);
	RFIFOSKIP(fd,2);
}

void char_map_auth_ok(int fd, int account_id, struct char_auth_node* node, struct mmo_charstatus* cd)
{
	nullpo_retv(cd);
	WFIFOHEAD(fd,25 + sizeof(struct mmo_charstatus));
	WFIFOW(fd,0) = 0x2afd;
	WFIFOW(fd,2) = 25 + sizeof(struct mmo_charstatus);
	WFIFOL(fd,4) = account_id;
	if (node)
	{
		WFIFOL(fd,8) = node->login_id1;
		WFIFOL(fd,12) = node->login_id2;
		WFIFOL(fd,16) = (uint32)node->expiration_time; // FIXME: will wrap to negative after "19-Jan-2038, 03:14:07 AM GMT"
		WFIFOL(fd,20) = node->group_id;
		WFIFOB(fd,24) = node->changing_mapservers;
	}
	else
	{
		WFIFOL(fd,8) = 0;
		WFIFOL(fd,12) = 0;
		WFIFOL(fd,16) = 0;
		WFIFOL(fd,20) = 0;
		WFIFOB(fd,24) = 0;
	}
	memcpy(WFIFOP(fd,25), cd, sizeof(struct mmo_charstatus));
	WFIFOSET(fd, WFIFOW(fd,2));
}

void char_map_auth_failed(int fd, int account_id, int char_id, int login_id1, char sex, uint32 ip)
{
	WFIFOHEAD(fd,19);
	WFIFOW(fd,0) = 0x2b27;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	WFIFOL(fd,10) = login_id1;
	WFIFOB(fd,14) = sex;
	WFIFOL(fd,15) = htonl(ip);
	WFIFOSET(fd,19);
}

void char_parse_frommap_auth_request(int fd, int id)
{
	struct mmo_charstatus char_dat;
	struct char_auth_node* node;
	struct mmo_charstatus* cd;

	int account_id  = RFIFOL(fd,2);
	int char_id     = RFIFOL(fd,6);
	int login_id1   = RFIFOL(fd,10);
	char sex        = RFIFOB(fd,14);
	uint32 ip       = ntohl(RFIFOL(fd,15));
	char standalone = RFIFOB(fd, 19);
	RFIFOSKIP(fd,20);

	node = (struct char_auth_node*)idb_get(auth_db, account_id);
	cd = (struct mmo_charstatus*)uidb_get(chr->char_db_,char_id);

	if( cd == NULL ) { //Really shouldn't happen.
		chr->mmo_char_fromsql(char_id, &char_dat, true);
		cd = (struct mmo_charstatus*)uidb_get(chr->char_db_,char_id);
	}

	if( core->runflag == CHARSERVER_ST_RUNNING && cd && standalone ) {
		cd->sex = sex;

		chr->map_auth_ok(fd, account_id, NULL, cd);
		chr->set_char_online(id, char_id, account_id);
		return;
	}

	if( core->runflag == CHARSERVER_ST_RUNNING &&
		cd != NULL &&
		node != NULL &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 /*&&
		node->sex == sex &&
		node->ip == ip*/ )
	{// auth ok
		if( cd->sex == 99 )
			cd->sex = sex;

		chr->map_auth_ok(fd, account_id, node, cd);
		// only use the auth once and mark user online
		idb_remove(auth_db, account_id);
		chr->set_char_online(id, char_id, account_id);
	}
	else
	{// auth failed
		chr->map_auth_failed(fd, account_id, char_id, login_id1, sex, ip);
	}
}

void char_parse_frommap_update_ip(int fd, int id)
{
	chr->server[id].ip = ntohl(RFIFOL(fd, 2));
	ShowInfo("Updated IP address of map-server #%d to %u.%u.%u.%u.\n", id, CONVIP(chr->server[id].ip));
	RFIFOSKIP(fd,6);
}

void char_parse_frommap_request_stats_report(int fd)
{
	int sfd;/* stat server fd */
	struct hSockOpt opt;
	RFIFOSKIP(fd, 2);/* we skip first 2 bytes which are the 0x3008, so we end up with a buffer equal to the one we send */

	opt.silent = 1;
	opt.setTimeo = 1;

	if ((sfd = sockt->make_connection(sockt->host2ip("stats.herc.ws"),(uint16)25427,&opt) ) == -1) {
		RFIFOSKIP(fd, RFIFOW(fd,2) );/* skip this packet */
		RFIFOFLUSH(fd);
		return;/* connection not possible, we drop the report */
	}

	sockt->session[sfd]->flag.server = 1;/* to ensure we won't drop our own packet */
	sockt->realloc_fifo(sfd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

	WFIFOHEAD(sfd, RFIFOW(fd,2) );

	memcpy(WFIFOP(sfd,0), RFIFOP(fd, 0), RFIFOW(fd,2));

	WFIFOSET(sfd, RFIFOW(fd,2) );

	do {
		sockt->flush(sfd);
		HSleep(1);
	} while( !sockt->session[sfd]->flag.eof && sockt->session[sfd]->wdata_size );

	sockt->close(sfd);

	RFIFOSKIP(fd, RFIFOW(fd,2) );/* skip this packet */
	RFIFOFLUSH(fd);
}

void char_parse_frommap_scdata_update(int fd)
{
	int account_id = RFIFOL(fd, 2);
	int char_id = RFIFOL(fd, 6);
	int val1 = RFIFOL(fd, 12);
	int val2 = RFIFOL(fd, 16);
	int val3 = RFIFOL(fd, 20);
	int val4 = RFIFOL(fd, 24);
	short type = RFIFOW(fd, 10);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s`"
			" (`account_id`,`char_id`,`type`,`tick`,`val1`,`val2`,`val3`,`val4`)"
			" VALUES ('%d','%d','%d','%d','%d','%d','%d','%d')",
			scdata_db, account_id, char_id, type, INFINITE_DURATION, val1, val2, val3, val4)
	) {
		Sql_ShowDebug(inter->sql_handle);
	}
	RFIFOSKIP(fd, 28);
}

void char_parse_frommap_scdata_delete(int fd)
{
	int account_id = RFIFOL(fd, 2);
	int char_id = RFIFOL(fd, 6);
	short type = RFIFOW(fd, 10);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id` = '%d' AND `type` = '%d' LIMIT 1",
								scdata_db, account_id, char_id, type) )
	{
		Sql_ShowDebug(inter->sql_handle);
	}
	RFIFOSKIP(fd, 12);
}

int char_parse_frommap(int fd)
{
	int id;

	ARR_FIND( 0, ARRAYLENGTH(chr->server), id, chr->server[id].fd == fd );
	if( id == ARRAYLENGTH(chr->server) ) {// not a map server
		ShowDebug("chr->parse_frommap: Disconnecting invalid session #%d (is not a map-server)\n", fd);
		sockt->close(fd);
		return 0;
	}
	if( sockt->session[fd]->flag.eof ) {
		sockt->close(fd);
		chr->server[id].fd = -1;
		mapif->on_disconnect(id);
		return 0;
	}

	while (RFIFOREST(fd) >= 2) {
		int packet_id = RFIFOW(fd,0);
		if (VECTOR_LENGTH(HPM->packets[hpParse_FromMap]) > 0) {
			int result = HPM->parse_packets(fd,packet_id,hpParse_FromMap);
			if (result == 1)
				continue;
			if (result == 2)
				return 0;
		}

		switch (packet_id) {
			case 0x2b0a:
				if( RFIFOREST(fd) < RFIFOW(fd, 2) )
					return 0;
				chr->parse_frommap_datasync(fd);
				break;

			case 0x2b0b:
				if( RFIFOREST(fd) < RFIFOW(fd, 2) )
					return 0;
				chr->parse_frommap_skillid2idx(fd);
				break;
			case 0x2afa: // Receiving map names list from the map-server
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
					return 0;
				chr->parse_frommap_map_names(fd, id);
			break;

			case 0x2afc: //Packet command is now used for sc_data request. [Skotlex]
				if (RFIFOREST(fd) < 10)
					return 0;
			{
				chr->parse_frommap_request_scdata(fd);
			}
			break;

			case 0x2afe: //set MAP user count
				if (RFIFOREST(fd) < 4)
					return 0;
				chr->parse_frommap_set_users_count(fd, id);
				break;

			case 0x2aff: //set MAP users
				if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
					return 0;
			{
				chr->parse_frommap_set_users(fd, id);
			}
			break;

			case 0x2b01: // Receive character data from map-server for saving
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
					return 0;
			{
				chr->parse_frommap_save_character(fd, id);

			}
			break;

			case 0x2b02: // req char selection
				if( RFIFOREST(fd) < 22 )
					return 0;
			{
				chr->parse_frommap_char_select_req(fd);
			}
			break;

			case 0x2b05: // request "change map server"
				if (RFIFOREST(fd) < 39)
					return 0;
			{
				chr->parse_frommap_change_map_server(fd);
			}
			break;

			case 0x2b07: // Remove RFIFOL(fd,6) (friend_id) from RFIFOL(fd,2) (char_id) friend list [Ind]
				if (RFIFOREST(fd) < 10)
					return 0;
				{
					chr->parse_frommap_remove_friend(fd);
				}
			break;

			case 0x2b08: // char name request
				if (RFIFOREST(fd) < 6)
					return 0;

				chr->parse_frommap_char_name_request(fd);
			break;

			case 0x2b0c: // Map server send information to change an email of an account -> login-server
				if (RFIFOREST(fd) < 86)
					return 0;
				chr->parse_frommap_change_email(fd);
			break;

			case 0x2b0e: // Request from map-server to change an account's or character's status (accounts will just be forwarded to login server)
				if (RFIFOREST(fd) < 44)
					return 0;
			{
				chr->parse_frommap_change_account(fd);
			}
			break;

			case 0x2b10: // Update and send fame ranking list
				if (RFIFOREST(fd) < 11)
					return 0;
			{
				chr->parse_frommap_fame_list(fd);
			}
			break;

			// Divorce chars
			case 0x2b11:
				if( RFIFOREST(fd) < 10 )
					return 0;

				chr->parse_frommap_divorce_char(fd);
			break;

			case 0x2b16: // Receive rates [Wizputer]
				if( RFIFOREST(fd) < 14 )
					return 0;
			{
				chr->parse_frommap_ragsrvinfo(fd);
			}
			break;

			case 0x2b17: // Character disconnected set online 0 [Wizputer]
				if (RFIFOREST(fd) < 6)
					return 0;
				chr->parse_frommap_set_char_offline(fd);
			break;

			case 0x2b18: // Reset all chars to offline [Wizputer]
				chr->parse_frommap_set_all_offline(fd, id);
			break;

			case 0x2b19: // Character set online [Wizputer]
				if (RFIFOREST(fd) < 10)
					return 0;
				chr->parse_frommap_set_char_online(fd, id);
			break;

			case 0x2b1a: // Build and send fame ranking lists [DracoRPG]
				if (RFIFOREST(fd) < 2)
					return 0;
				chr->parse_frommap_build_fame_list(fd);
			break;

			case 0x2b1c: //Request to save status change data. [Skotlex]
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
					return 0;
			{
				chr->parse_frommap_save_status_change_data(fd);
			}
			break;

			case 0x2b23: // map-server alive packet
				chr->parse_frommap_ping(fd);
			break;

			case 0x2b26: // auth request from map-server
				if (RFIFOREST(fd) < 20)
					return 0;

			{
				chr->parse_frommap_auth_request(fd, id);
			}
			break;

			case 0x2736: // ip address update
				if (RFIFOREST(fd) < 6) return 0;
				chr->parse_frommap_update_ip(fd, id);
			break;

			case 0x3008:
				if( RFIFOREST(fd) < RFIFOW(fd,4) )
					return 0;/* packet wasn't fully received yet (still fragmented) */
				else {
					chr->parse_frommap_request_stats_report(fd);
			}
			break;

			/* individual sc data insertion/update  */
			case 0x2740:
				if( RFIFOREST(fd) < 28 )
					return 0;
				else {
					chr->parse_frommap_scdata_update(fd);
				}
				break;

			/* individual sc data delete */
			case 0x2741:
				if( RFIFOREST(fd) < 12 )
					return 0;
				else {
					chr->parse_frommap_scdata_delete(fd);
				}
				break;

			default:
			{
				// inter server - packet
				int r = inter->parse_frommap(fd);
				if (r == 1) break;    // processed
				if (r == 2) return 0; // need more packet

				// no inter server packet. no char server packet -> disconnect
				ShowError("Unknown packet 0x%04x from map server, disconnecting.\n", RFIFOW(fd,0));
				sockt->eof(fd);
				return 0;
			}
		} // switch
	} // while

	return 0;
}

void do_init_mapif(void)
{
	int i;
	for( i = 0; i < ARRAYLENGTH(chr->server); ++i )
		mapif->server_init(i);
}

void do_final_mapif(void)
{
	int i;
	for( i = 0; i < ARRAYLENGTH(chr->server); ++i )
		mapif->server_destroy(i);
}

// Searches for the mapserver that has a given map (and optionally ip/port, if not -1).
// If found, returns the server's index in the 'server' array (otherwise returns -1).
int char_search_mapserver(unsigned short map, uint32 ip, uint16 port)
{
	int i, j;

	for(i = 0; i < ARRAYLENGTH(chr->server); i++)
	{
		if (chr->server[i].fd > 0
		&& (ip == (uint32)-1 || chr->server[i].ip == ip)
		&& (port == (uint16)-1 || chr->server[i].port == port)
		) {
			ARR_FIND(0, VECTOR_LENGTH(chr->server[i].maps), j, VECTOR_INDEX(chr->server[i].maps, j) == map);
			if (j != VECTOR_LENGTH(chr->server[i].maps))
				return i;
		}
	}

	return -1;
}

// Initialization process (currently only initialization inter_mapif)
static int char_mapif_init(int fd)
{
	return inter->mapif_init(fd);
}

/**
 * Checks whether the given IP comes from LAN or WAN.
 *
 * @param ip IP address to check.
 * @retval 0 if it is a WAN IP.
 * @return the appropriate LAN server address to send, if it is a LAN IP.
 */
uint32 char_lan_subnet_check(uint32 ip)
{
	struct s_subnet lan = {0};
	if (sockt->lan_subnet_check(ip, &lan)) {
		ShowInfo("Subnet check [%u.%u.%u.%u]: Matches "CL_CYAN"%u.%u.%u.%u/%u.%u.%u.%u"CL_RESET"\n", CONVIP(ip), CONVIP(lan.ip & lan.mask), CONVIP(lan.mask));
		return lan.ip;
	}
	ShowInfo("Subnet check [%u.%u.%u.%u]: "CL_CYAN"WAN"CL_RESET"\n", CONVIP(ip));
	return 0;
}


/// Answers to deletion request (HC_DELETE_CHAR3_RESERVED)
/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 3 (0x719): A database error occurred.
/// 4 (0x71a): To delete a character you must withdraw from the guild.
/// 5 (0x71b): To delete a character you must withdraw from the party.
/// Any (0x718): An unknown error has occurred.
void char_delete2_ack(int fd, int char_id, uint32 result, time_t delete_date)
{// HC: <0828>.W <char id>.L <Msg:0-5>.L <deleteDate>.L
	WFIFOHEAD(fd,14);
	WFIFOW(fd,0) = 0x828;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
#if PACKETVER >= 20130000
	WFIFOL(fd,10) = (int)(delete_date - time(NULL));
#else
	WFIFOL(fd,10) = (int)delete_date;

#endif
	WFIFOSET(fd,14);
}

void char_delete2_accept_actual_ack(int fd, int char_id, uint32 result)
{
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x82a;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
	WFIFOSET(fd,10);
}

/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 2 (0x71c): Due to system settings can not be deleted.
/// 3 (0x719): A database error occurred.
/// 4 (0x71d): Deleting not yet possible time.
/// 5 (0x71e): Date of birth do not match.
/// Any (0x718): An unknown error has occurred.
void char_delete2_accept_ack(int fd, int char_id, uint32 result)
{// HC: <082a>.W <char id>.L <Msg:0-5>.L
#if PACKETVER >= 20130000 /* not sure the exact date -- must refresh or client gets stuck */
	if( result == 1 ) {
		struct char_session_data* sd = (struct char_session_data*)sockt->session[fd]->session_data;
		chr->mmo_char_send099d(fd, sd);
	}
#endif
	chr->delete2_accept_actual_ack(fd, char_id, result);
}

/// @param result
/// 1 (0x718): none/success, (if char id not in deletion process): An unknown error has occurred.
/// 2 (0x719): A database error occurred.
/// Any (0x718): An unknown error has occurred.
void char_delete2_cancel_ack(int fd, int char_id, uint32 result)
{// HC: <082c>.W <char id>.L <Msg:1-2>.L
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x82c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
	WFIFOSET(fd,10);
}

static void char_delete2_req(int fd, struct char_session_data* sd)
{// CH: <0827>.W <char id>.L
	int char_id, i;
	char* data;
	time_t delete_date;

	char_id = RFIFOL(fd,2);
	nullpo_retv(sd);

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
	if( i == MAX_CHARS )
	{// character not found
		chr->delete2_ack(fd, char_id, 3, 0); // 3: A database error occurred
		return;
	}

	if( SQL_SUCCESS != SQL->Query(inter->sql_handle, "SELECT `delete_date` FROM `%s` WHERE `char_id`='%d'", char_db, char_id) || SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{
		Sql_ShowDebug(inter->sql_handle);
		chr->delete2_ack(fd, char_id, 3, 0); // 3: A database error occurred
		return;
	}

	SQL->GetData(inter->sql_handle, 0, &data, NULL); delete_date = strtoul(data, NULL, 10);

	if( delete_date ) {// character already queued for deletion
		chr->delete2_ack(fd, char_id, 0, 0); // 0: An unknown error occurred
		return;
	}

	// This check is imposed by Aegis to avoid dead entries in databases
	// _it is not needed_ as we clear data properly
	// see issue: 7338
	if (char_aegis_delete) {
		int party_id = 0, guild_id = 0;
		if( SQL_SUCCESS != SQL->Query(inter->sql_handle, "SELECT `party_id`, `guild_id` FROM `%s` WHERE `char_id`='%d'", char_db, char_id)
		 || SQL_SUCCESS != SQL->NextRow(inter->sql_handle)
		) {
			Sql_ShowDebug(inter->sql_handle);
			chr->delete2_ack(fd, char_id, 3, 0); // 3: A database error occurred
			return;
		}
		SQL->GetData(inter->sql_handle, 0, &data, NULL); party_id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); guild_id = atoi(data);

		if( guild_id )
		{
			chr->delete2_ack(fd, char_id, 4, 0); // 4: To delete a character you must withdraw from the guild
			return;
		}

		if( party_id )
		{
			chr->delete2_ack(fd, char_id, 5, 0); // 5: To delete a character you must withdraw from the party
			return;
		}
	}

	// success
	delete_date = time(NULL)+char_del_delay;

	if( SQL_SUCCESS != SQL->Query(inter->sql_handle, "UPDATE `%s` SET `delete_date`='%lu' WHERE `char_id`='%d'", char_db, (unsigned long)delete_date, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		chr->delete2_ack(fd, char_id, 3, 0); // 3: A database error occurred
		return;
	}

	chr->delete2_ack(fd, char_id, 1, delete_date); // 1: success
}

static void char_delete2_accept(int fd, struct char_session_data* sd)
{// CH: <0829>.W <char id>.L <birth date:YYMMDD>.6B
	char birthdate[8+1];
	int char_id, i;
	unsigned int base_level;
	char* data;
	time_t delete_date;

	nullpo_retv(sd);
	char_id = RFIFOL(fd,2);

	ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, char_id);

	// construct "YY-MM-DD"
	birthdate[0] = RFIFOB(fd,6);
	birthdate[1] = RFIFOB(fd,7);
	birthdate[2] = '-';
	birthdate[3] = RFIFOB(fd,8);
	birthdate[4] = RFIFOB(fd,9);
	birthdate[5] = '-';
	birthdate[6] = RFIFOB(fd,10);
	birthdate[7] = RFIFOB(fd,11);
	birthdate[8] = 0;

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
	if( i == MAX_CHARS )
	{// character not found
		chr->delete2_accept_ack(fd, char_id, 3); // 3: A database error occurred
		return;
	}

	if( SQL_SUCCESS != SQL->Query(inter->sql_handle, "SELECT `base_level`,`delete_date` FROM `%s` WHERE `char_id`='%d'", char_db, char_id) || SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{// data error
		Sql_ShowDebug(inter->sql_handle);
		chr->delete2_accept_ack(fd, char_id, 3); // 3: A database error occurred
		return;
	}

	SQL->GetData(inter->sql_handle, 0, &data, NULL); base_level = (unsigned int)strtoul(data, NULL, 10);
	SQL->GetData(inter->sql_handle, 1, &data, NULL); delete_date = strtoul(data, NULL, 10);

	if( !delete_date || delete_date>time(NULL) )
	{// not queued or delay not yet passed
		chr->delete2_accept_ack(fd, char_id, 4); // 4: Deleting not yet possible time
		return;
	}

	if( strcmp(sd->birthdate+2, birthdate) )  // +2 to cut off the century
	{// birth date is wrong
		chr->delete2_accept_ack(fd, char_id, 5); // 5: Date of birth do not match
		return;
	}

	if( ( char_del_level > 0 && base_level >= (unsigned int)char_del_level ) || ( char_del_level < 0 && base_level <= (unsigned int)(-char_del_level) ) )
	{// character level config restriction
		chr->delete2_accept_ack(fd, char_id, 2); // 2: Due to system settings can not be deleted
		return;
	}

	// success
	if( chr->delete_char_sql(char_id) < 0 )
	{
		chr->delete2_accept_ack(fd, char_id, 3); // 3: A database error occurred
		return;
	}

	// refresh character list cache
	sd->found_char[i] = -1;

	chr->delete2_accept_ack(fd, char_id, 1); // 1: success
}

static void char_delete2_cancel(int fd, struct char_session_data* sd)
{// CH: <082b>.W <char id>.L
	int char_id, i;

	nullpo_retv(sd);
	char_id = RFIFOL(fd,2);

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
	if( i == MAX_CHARS )
	{// character not found
		chr->delete2_cancel_ack(fd, char_id, 2); // 2: A database error occurred
		return;
	}

	// there is no need to check, whether or not the character was
	// queued for deletion, as the client prints an error message by
	// itself, if it was not the case (@see chr->delete2_cancel_ack)
	if( SQL_SUCCESS != SQL->Query(inter->sql_handle, "UPDATE `%s` SET `delete_date`='0' WHERE `char_id`='%d'", char_db, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		chr->delete2_cancel_ack(fd, char_id, 2); // 2: A database error occurred
		return;
	}

	chr->delete2_cancel_ack(fd, char_id, 1); // 1: success
}

void char_send_account_id(int fd, int account_id)
{
	WFIFOHEAD(fd,4);
	WFIFOL(fd,0) = account_id;
	WFIFOSET(fd,4);
}

void char_parse_char_connect(int fd, struct char_session_data* sd, uint32 ipl)
{
	int account_id = RFIFOL(fd,2);
	uint32 login_id1 = RFIFOL(fd,6);
	uint32 login_id2 = RFIFOL(fd,10);
	int sex = RFIFOB(fd,16);
	struct char_auth_node* node;

	RFIFOSKIP(fd,17);

	ShowInfo("request connect - account_id:%d/login_id1:%u/login_id2:%u\n", account_id, login_id1, login_id2);

	if (sd) {
		//Received again auth packet for already authenticated account?? Discard it.
		//TODO: Perhaps log this as a hack attempt?
		//TODO: and perhaps send back a reply?
		return;
	}

	CREATE(sockt->session[fd]->session_data, struct char_session_data, 1);
	sd = (struct char_session_data*)sockt->session[fd]->session_data;
	sd->account_id = account_id;
	sd->login_id1 = login_id1;
	sd->login_id2 = login_id2;
	sd->sex = sex;
	sd->auth = false; // not authed yet

	// send back account_id
	chr->send_account_id(fd, account_id);

	if( core->runflag != CHARSERVER_ST_RUNNING ) {
		chr->auth_error(fd, 0);
		return;
	}

	// search authentication
	node = (struct char_auth_node*)idb_get(auth_db, account_id);
	if( node != NULL &&
		node->account_id == account_id &&
		node->login_id1  == login_id1 &&
		node->login_id2  == login_id2 /*&&
		node->ip         == ipl*/ )
	{// authentication found (coming from map server)
		/* restrictions apply */
		if( chr->server_type == CST_MAINTENANCE && node->group_id < char_maintenance_min_group_id ) {
			chr->auth_error(fd, 0);
			return;
		}
		/* the client will already deny this request, this check is to avoid someone bypassing. */
		if( chr->server_type == CST_PAYING && (time_t)node->expiration_time < time(NULL) ) {
			chr->auth_error(fd, 0);
			return;
		}
		idb_remove(auth_db, account_id);
		chr->auth_ok(fd, sd);
	}
	else
	{// authentication not found (coming from login server)
		if (chr->login_fd > 0) { // don't send request if no login-server
			loginif->auth(fd, sd, ipl);
		} else { // if no login-server, we must refuse connection
			chr->auth_error(fd, 0);
		}
	}
}

void char_send_map_info(int fd, int i, uint32 subnet_map_ip, struct mmo_charstatus *cd)
{
	nullpo_retv(cd);
	WFIFOHEAD(fd,28);
	WFIFOW(fd,0) = 0x71;
	WFIFOL(fd,2) = cd->char_id;
	mapindex->getmapname_ext(mapindex_id2name(cd->last_point.map), WFIFOP(fd,6));
	WFIFOL(fd,22) = htonl((subnet_map_ip) ? subnet_map_ip : chr->server[i].ip);
	WFIFOW(fd,26) = sockt->ntows(htons(chr->server[i].port)); // [!] LE byte order here [!]
	WFIFOSET(fd,28);
}

void char_send_wait_char_server(int fd)
{
	WFIFOHEAD(fd, 24);
	WFIFOW(fd, 0) = 0x840;
	WFIFOW(fd, 2) = 24;
	safestrncpy(WFIFOP(fd,4), "0", 20);/* we can't send empty (otherwise the list will pop up) */
	WFIFOSET(fd, 24);
}

int char_search_default_maps_mapserver(struct mmo_charstatus *cd)
{
	int i;
	int j;
	nullpo_retr(-1, cd);
	if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_PRONTERA)),-1,-1)) >= 0) {
		cd->last_point.x = 273;
		cd->last_point.y = 354;
	} else if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_GEFFEN)),-1,-1)) >= 0) {
		cd->last_point.x = 120;
		cd->last_point.y = 100;
	} else if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_MORROC)),-1,-1)) >= 0) {
		cd->last_point.x = 160;
		cd->last_point.y = 94;
	} else if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_ALBERTA)),-1,-1)) >= 0) {
		cd->last_point.x = 116;
		cd->last_point.y = 57;
	} else if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_PAYON)),-1,-1)) >= 0) {
		cd->last_point.x = 87;
		cd->last_point.y = 117;
	} else if ((i = chr->search_mapserver((j=mapindex->name2id(MAP_IZLUDE)),-1,-1)) >= 0) {
		cd->last_point.x = 94;
		cd->last_point.y = 103;
	}
	if (i >= 0)
	{
		cd->last_point.map = j;
		ShowWarning("Unable to find map-server for '%s', sending to major city '%s'.\n", mapindex_id2name(cd->last_point.map), mapindex_id2name(j));
	}
	return i;
}

void char_parse_char_select(int fd, struct char_session_data* sd, uint32 ipl) __attribute__((nonnull (2)));
void char_parse_char_select(int fd, struct char_session_data* sd, uint32 ipl)
{
	struct mmo_charstatus char_dat;
	struct mmo_charstatus *cd;
	struct char_auth_node* node;
	char* data;
	int char_id;
	int server_id = 0;
	int i;
	int map_fd;
	uint32 subnet_map_ip;
	int slot = RFIFOB(fd,2);

	RFIFOSKIP(fd,3);

#if PACKETVER >= 20110309
	if( pincode->enabled ){ // hack check
		struct online_char_data* character;
		character = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id);
		if( character && character->pincode_enable == -1){
			chr->auth_error(fd, 0);
			return;
		}
	}
#endif

	ARR_FIND(0, ARRAYLENGTH(chr->server), server_id, chr->server[server_id].fd > 0 && VECTOR_LENGTH(chr->server[server_id].maps) > 0);
	/* not available, tell it to wait (client wont close; char select will respawn).
	 * magic response found by Ind thanks to Yommy <3 */
	if( server_id == ARRAYLENGTH(chr->server) ) {
		chr->send_wait_char_server(fd);
		return;
	}

	if (SQL_SUCCESS != SQL->Query(inter->sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d'", char_db, sd->account_id, slot)
	 || SQL_SUCCESS != SQL->NextRow(inter->sql_handle)
	 || SQL_SUCCESS != SQL->GetData(inter->sql_handle, 0, &data, NULL)
	) {
		//Not found?? May be forged packet.
		Sql_ShowDebug(inter->sql_handle);
		SQL->FreeResult(inter->sql_handle);
		chr->auth_error(fd, 0); // rejected from server
		return;
	}

	char_id = atoi(data);
	SQL->FreeResult(inter->sql_handle);

	/* client doesn't let it get to this point if you're banned, so its a forged packet */
	if( sd->found_char[slot] == char_id && sd->unban_time[slot] > time(NULL) ) {
		chr->auth_error(fd, 0); // rejected from server
		return;
	}

	/* set char as online prior to loading its data so 3rd party applications will realize the sql data is not reliable */
	chr->set_char_online(-2,char_id,sd->account_id);
	if( !chr->mmo_char_fromsql(char_id, &char_dat, true) ) { /* failed? set it back offline */
		chr->set_char_offline(char_id, sd->account_id);
		/* failed to load something. REJECT! */
		chr->auth_error(fd, 0); // rejected from server
		return;/* jump off this boat */
	}

	//Have to switch over to the DB instance otherwise data won't propagate [Kevin]
	cd = (struct mmo_charstatus *)idb_get(chr->char_db_, char_id);
	nullpo_retv(cd);
	if( cd->sex == 99 )
		cd->sex = sd->sex;

	if (log_char) {
		char esc_name[NAME_LENGTH*2+1];
		// FIXME: Why are we re-escaping the name if it was already escaped in rename/make_new_char? [Panikon]
		SQL->EscapeStringLen(inter->sql_handle, esc_name, char_dat.name, strnlen(char_dat.name, NAME_LENGTH));
		if( SQL_ERROR == SQL->Query(inter->sql_handle,
			"INSERT INTO `%s`(`time`, `account_id`, `char_id`, `char_num`, `name`) VALUES (NOW(), '%d', '%d', '%d', '%s')",
			charlog_db, sd->account_id, cd->char_id, slot, esc_name) )
			Sql_ShowDebug(inter->sql_handle);
	}
	ShowInfo("Selected char: (Account %d: %d - %s)\n", sd->account_id, slot, char_dat.name);

	// searching map server
	i = chr->search_mapserver(cd->last_point.map, -1, -1);

	// if map is not found, we check major cities
	if (i < 0 || !cd->last_point.map) {
		unsigned short j;
		//First check that there's actually a map server online.
		ARR_FIND(0, ARRAYLENGTH(chr->server), j, chr->server[j].fd >= 0 && VECTOR_LENGTH(chr->server[j].maps) > 0);
		if (j == ARRAYLENGTH(chr->server)) {
			ShowInfo("Connection Closed. No map servers available.\n");
			chr->authfail_fd(fd, 1); // 1 = Server closed
			return;
		}
		i = chr->search_default_maps_mapserver(cd);
		if (i < 0)
		{
			ShowInfo("Connection Closed. No map server available that has a major city, and unable to find map-server for '%s'.\n", mapindex_id2name(cd->last_point.map));
			chr->authfail_fd(fd, 1); // 1 = Server closed
			return;
		}
	}

	//Send NEW auth packet [Kevin]
	//FIXME: is this case even possible? [ultramage]
	if ((map_fd = chr->server[i].fd) < 1 || sockt->session[map_fd] == NULL)
	{
		ShowError("chr->parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, i);
		chr->server[i].fd = -1;
		memset(&chr->server[i], 0, sizeof(struct mmo_map_server));
		chr->authfail_fd(fd, 1); // 1 = Server closed
		return;
	}

	subnet_map_ip = chr->lan_subnet_check(ipl);
	//Send player to map
	chr->send_map_info(fd, i, subnet_map_ip, cd);

	// create temporary auth entry
	CREATE(node, struct char_auth_node, 1);
	node->account_id = sd->account_id;
	node->char_id = cd->char_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->sex;
	node->expiration_time = sd->expiration_time;
	node->group_id = sd->group_id;
	node->ip = ipl;
	idb_put(auth_db, sd->account_id, node);
}

void char_creation_failed(int fd, int result)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x6e;
	/* Others I found [Ind] */
	/* 0x02 = Symbols in Character Names are forbidden */
	/* 0x03 = You are not eligible to open the Character Slot. */
	/* 0x0B = This service is only available for premium users.  */
	switch (result) {
		case -1: WFIFOB(fd,2) = 0x00; break; // 'Charname already exists'
		case -2: WFIFOB(fd,2) = 0xFF; break; // 'Char creation denied'
		case -3: WFIFOB(fd,2) = 0x01; break; // 'You are underaged'
		case -4: WFIFOB(fd,2) = 0x03; break; // 'You are not eligible to open the Character Slot.'
		case -5: WFIFOB(fd,2) = 0x02; break; // 'Symbols in Character Names are forbidden'

		default:
			ShowWarning("chr->parse_char: Unknown result received from chr->make_new_char_sql!\n");
			WFIFOB(fd,2) = 0xFF;
			break;
	}
	WFIFOSET(fd,3);
}

void char_creation_ok(int fd, struct mmo_charstatus *char_dat)
{
	int len;

	// send to player
	WFIFOHEAD(fd,2+MAX_CHAR_BUF);
	WFIFOW(fd,0) = 0x6d;
	len = 2 + chr->mmo_char_tobuf(WFIFOP(fd,2), char_dat);
	WFIFOSET(fd,len);
}

void char_parse_char_create_new_char(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_create_new_char(int fd, struct char_session_data* sd)
{
	int result;
	if( !char_new ) {
		//turn character creation on/off [Kevin]
		result = -2;
	} else {
	#if PACKETVER >= 20120307
		result = chr->make_new_char_sql(sd, RFIFOP(fd,2), 1, 1, 1, 1, 1, 1, RFIFOB(fd,26),RFIFOW(fd,27),RFIFOW(fd,29));
	#else
		result = chr->make_new_char_sql(sd, RFIFOP(fd,2),RFIFOB(fd,26),RFIFOB(fd,27),RFIFOB(fd,28),RFIFOB(fd,29),RFIFOB(fd,30),RFIFOB(fd,31),RFIFOB(fd,32),RFIFOW(fd,33),RFIFOW(fd,35));
	#endif
	}

	//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
	if (result < 0) {
		chr->creation_failed(fd, result);
	} else {
		// retrieve data
		struct mmo_charstatus char_dat;
		chr->mmo_char_fromsql(result, &char_dat, false); //Only the short data is needed.
		chr->creation_ok(fd, &char_dat);

		// add new entry to the chars list
		sd->found_char[char_dat.slot] = result; // the char_id of the new char
	}
	#if PACKETVER >= 20120307
	RFIFOSKIP(fd,31);
	#else
	RFIFOSKIP(fd,37);
	#endif
}

// flag:
// 0 = Incorrect Email address
void char_delete_char_failed(int fd, int flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x70;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void char_delete_char_ok(int fd)
{
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x6f;
	WFIFOSET(fd,2);
}

void char_parse_char_delete_char(int fd, struct char_session_data* sd, unsigned short cmd) __attribute__((nonnull (2)));
void char_parse_char_delete_char(int fd, struct char_session_data* sd, unsigned short cmd)
{
	char email[40];
	int cid = RFIFOL(fd,2);
	int i;

#if PACKETVER >= 20110309
	if (pincode->enabled) { // hack check
		struct online_char_data* character;
		character = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id);
		if( character && character->pincode_enable == -1 ){
			chr->auth_error(fd, 0);
			RFIFOSKIP(fd,( cmd == 0x68) ? 46 : 56);
			return;
		}
	}
#endif
	ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, cid);
	memcpy(email, RFIFOP(fd,6), 40);
	RFIFOSKIP(fd,( cmd == 0x68) ? 46 : 56);

	// Check if e-mail is correct
	if (strcmpi(email, sd->email) != 0  /* emails don't match */
	 && ( strcmp("a@a.com", sd->email) != 0 /* it's not the default email */
	  || (strcmp("a@a.com", email) != 0 && strcmp("", email) != 0) /* sent email isn't the default */
	)) {
		//Fail
		chr->delete_char_failed(fd, 0);
		return;
	}

	// check if this char exists
	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
	if( i == MAX_CHARS )
	{ // Such a character does not exist in the account
		chr->delete_char_failed(fd, 0);
		return;
	}

	// remove char from list and compact it
	sd->found_char[i] = -1;

	/* Delete character */
	if(chr->delete_char_sql(cid)<0){
		//can't delete the char
		//either SQL error or can't delete by some CONFIG conditions
		//del fail
		chr->delete_char_failed(fd, 0);
		return;
	}
	/* Char successfully deleted.*/
	chr->delete_char_ok(fd);
}

void char_parse_char_ping(int fd)
{
	RFIFOSKIP(fd,6);
}

void char_allow_rename(int fd, int flag)
{
	WFIFOHEAD(fd, 4);
	WFIFOW(fd,0) = 0x28e;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,4);
}

void char_parse_char_rename_char(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_rename_char(int fd, struct char_session_data* sd)
{
	int i, cid =RFIFOL(fd,2);
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];
	safestrncpy(name, RFIFOP(fd,6), NAME_LENGTH);
	RFIFOSKIP(fd,30);

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
	if( i == MAX_CHARS )
		return;

	normalize_name(name,TRIM_CHARS);
	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( !chr->check_char_name(name,esc_name) ) {
		i = 1;
		safestrncpy(sd->new_name, name, NAME_LENGTH);
	} else {
		i = 0;
	}

	chr->allow_rename(fd, i);
}

void char_parse_char_rename_char2(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_rename_char2(int fd, struct char_session_data* sd)
{
	int i, aid = RFIFOL(fd,2), cid =RFIFOL(fd,6);
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];
	safestrncpy(name, RFIFOP(fd,10), NAME_LENGTH);
	RFIFOSKIP(fd,34);

	if( aid != sd->account_id )
		return;
	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
	if( i == MAX_CHARS )
		return;

	normalize_name(name,TRIM_CHARS);
	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( !chr->check_char_name(name,esc_name) )
	{
		i = 1;
		safestrncpy(sd->new_name, name, NAME_LENGTH);
	}
	else
		i = 0;

	chr->allow_rename(fd, i);
}

void char_rename_char_ack(int fd, int flag)
{
	WFIFOHEAD(fd, 4);
	WFIFOW(fd,0) = 0x290;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,4);
}

void char_parse_char_rename_char_confirm(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_rename_char_confirm(int fd, struct char_session_data* sd)
{
	int i;
	int cid = RFIFOL(fd,2);
	RFIFOSKIP(fd,6);

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
	if( i == MAX_CHARS )
		return;
	i = chr->rename_char_sql(sd, cid);

	chr->rename_char_ack(fd, i);
}

void char_captcha_notsupported(int fd)
{
	WFIFOHEAD(fd,5);
	WFIFOW(fd,0) = 0x7e9;
	WFIFOW(fd,2) = 5;
	WFIFOB(fd,4) = 1;
	WFIFOSET(fd,5);
}

void char_parse_char_request_captcha(int fd)
{
	chr->captcha_notsupported(fd);
	RFIFOSKIP(fd,8);
}

void char_parse_char_check_captcha(int fd)
{
	chr->captcha_notsupported(fd);
	RFIFOSKIP(fd,32);
}

void char_parse_char_delete2_req(int fd, struct char_session_data* sd)
{
	chr->delete2_req(fd, sd);
	RFIFOSKIP(fd,6);
}

void char_parse_char_delete2_accept(int fd, struct char_session_data* sd)
{
	chr->delete2_accept(fd, sd);
	RFIFOSKIP(fd,12);
}

void char_parse_char_delete2_cancel(int fd, struct char_session_data* sd)
{
	chr->delete2_cancel(fd, sd);
	RFIFOSKIP(fd,6);
}

// flag:
// 0 - ok
// 3 - error
void char_login_map_server_ack(int fd, uint8 flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x2af9;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void char_parse_char_login_map_server(int fd, uint32 ipl)
{
	char l_user[24], l_pass[24];
	int i;
	safestrncpy(l_user, RFIFOP(fd,2), 24);
	safestrncpy(l_pass, RFIFOP(fd,26), 24);

	ARR_FIND( 0, ARRAYLENGTH(chr->server), i, chr->server[i].fd <= 0 );
	if (core->runflag != CHARSERVER_ST_RUNNING ||
		i == ARRAYLENGTH(chr->server) ||
		strcmp(l_user, chr->userid) != 0 ||
		strcmp(l_pass, chr->passwd) != 0 ||
		!sockt->allowed_ip_check(ipl))
	{
		chr->login_map_server_ack(fd, 3); // Failure
	} else {
		chr->login_map_server_ack(fd, 0); // Success

		chr->server[i].fd = fd;
		chr->server[i].ip = ntohl(RFIFOL(fd,54));
		chr->server[i].port = ntohs(RFIFOW(fd,58));
		chr->server[i].users = 0;
		sockt->session[fd]->func_parse = chr->parse_frommap;
		sockt->session[fd]->flag.server = 1;
		sockt->realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
		chr->mapif_init(fd);
	}
	sockt->datasync(fd, true);

	RFIFOSKIP(fd,60);
}

void char_parse_char_pincode_check(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_pincode_check(int fd, struct char_session_data* sd)
{
	if (RFIFOL(fd,2) == sd->account_id)
		pincode->check(fd, sd);

	RFIFOSKIP(fd, 10);
}

void char_parse_char_pincode_window(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_pincode_window(int fd, struct char_session_data* sd)
{
	if (RFIFOL(fd,2) == sd->account_id)
		pincode->sendstate(fd, sd, PINCODE_NOTSET);

	RFIFOSKIP(fd, 6);
}

void char_parse_char_pincode_change(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_pincode_change(int fd, struct char_session_data* sd)
{
	if (RFIFOL(fd,2) == sd->account_id)
		pincode->change(fd, sd);

	RFIFOSKIP(fd, 14);
}

void char_parse_char_pincode_first_pin(int fd, struct char_session_data* sd) __attribute__((nonnull (2)));
void char_parse_char_pincode_first_pin(int fd, struct char_session_data* sd)
{
	if (RFIFOL(fd,2) == sd->account_id)
		pincode->setnew (fd, sd);
	RFIFOSKIP(fd, 10);
}

void char_parse_char_request_chars(int fd, struct char_session_data* sd)
{
	chr->mmo_char_send099d(fd, sd);
	RFIFOSKIP(fd,2);
}

void char_change_character_slot_ack(int fd, bool ret)
{
	WFIFOHEAD(fd, 8);
	WFIFOW(fd, 0) = 0x8d5;
	WFIFOW(fd, 2) = 8;
	WFIFOW(fd, 4) = ret?0:1;
	WFIFOW(fd, 6) = 0;/* we enforce it elsewhere, go 0 */
	WFIFOSET(fd, 8);
}

void char_parse_char_move_character(int fd, struct char_session_data* sd)
{
	bool ret = chr->char_slotchange(sd, fd, RFIFOW(fd, 2), RFIFOW(fd, 4));
	chr->change_character_slot_ack(fd, ret);
	/* for some stupid reason it requires the char data again (gravity -_-) */
	if( ret )
#if PACKETVER >= 20130000
		chr->mmo_char_send099d(fd, sd);
#else
		chr->mmo_char_send_characters(fd, sd);
#endif
	RFIFOSKIP(fd, 8);
}

int char_parse_char_unknown_packet(int fd, uint32 ipl)
{
	ShowError("chr->parse_char: Received unknown packet "CL_WHITE"0x%x"CL_RESET" from ip '"CL_WHITE"%s"CL_RESET"'! Disconnecting!\n", RFIFOW(fd,0), sockt->ip2str(ipl, NULL));
	sockt->eof(fd);
	return 1;
}

int char_parse_char(int fd)
{
	unsigned short cmd;
	struct char_session_data* sd;
	uint32 ipl = sockt->session[fd]->client_addr;

	sd = (struct char_session_data*)sockt->session[fd]->session_data;

	// disconnect any player if no login-server.
	if(chr->login_fd < 0)
		sockt->eof(fd);

	if(sockt->session[fd]->flag.eof)
	{
		if( sd != NULL && sd->auth ) {
			// already authed client
			struct online_char_data* data = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id);
			if( data != NULL && data->fd == fd)
				data->fd = -1;
			if( data == NULL || data->server == -1) //If it is not in any server, send it offline. [Skotlex]
				chr->set_char_offline(-1,sd->account_id);
		}
		sockt->close(fd);
		return 0;
	}

	while (RFIFOREST(fd) >= 2) {
		cmd = RFIFOW(fd,0);

//For use in packets that depend on an sd being present [Skotlex]
#define FIFOSD_CHECK(rest) do { if(RFIFOREST(fd) < (rest)) return 0; if (sd==NULL || !sd->auth) { RFIFOSKIP(fd,(rest)); return 0; } } while (0)

		if (VECTOR_LENGTH(HPM->packets[hpParse_Char]) > 0) {
			int result = HPM->parse_packets(fd,cmd,hpParse_Char);
			if (result == 1)
				continue;
			if (result == 2)
				return 0;
		}

		switch (cmd) {
			// request to connect
			// 0065 <account id>.L <login id1>.L <login id2>.L <???>.W <sex>.B
			case 0x65:
				if( RFIFOREST(fd) < 17 )
					return 0;
			{
				chr->parse_char_connect(fd, sd, ipl);
			}
			break;

			// char select
			case 0x66:
				FIFOSD_CHECK(3);
			{
				chr->parse_char_select(fd, sd, ipl);
			}
			break;

			// create new char
	#if PACKETVER >= 20120307
			// S 0970 <name>.24B <slot>.B <hair color>.W <hair style>.W
			case 0x970:
			{
				FIFOSD_CHECK(31);
	#else
			// S 0067 <name>.24B <str>.B <agi>.B <vit>.B <int>.B <dex>.B <luk>.B <slot>.B <hair color>.W <hair style>.W
			case 0x67:
			{
				FIFOSD_CHECK(37);
	#endif

				chr->parse_char_create_new_char(fd, sd);
			}
			break;

			// delete char
			case 0x68:
			// 2004-04-19aSakexe+ langtype 12 char deletion packet
			case 0x1fb:
				if (cmd == 0x68) FIFOSD_CHECK(46);
				if (cmd == 0x1fb) FIFOSD_CHECK(56);
			{
				chr->parse_char_delete_char(fd, sd, cmd);
			}
			break;

			// client keep-alive packet (every 12 seconds)
			// R 0187 <account ID>.l
			case 0x187:
				if (RFIFOREST(fd) < 6)
					return 0;
				chr->parse_char_ping(fd);
			break;
			// char rename request
			// R 08fc <char ID>.l <new name>.24B
			case 0x8fc:
				FIFOSD_CHECK(30);
				{
					chr->parse_char_rename_char(fd, sd);
				}
				break;

			// char rename request
			// R 028d <account ID>.l <char ID>.l <new name>.24B
			case 0x28d:
				FIFOSD_CHECK(34);
				{
					chr->parse_char_rename_char2(fd, sd);
				}
				break;
			//Confirm change name.
			// 0x28f <char_id>.L
			case 0x28f:
				// 0: Successful
				// 1: This character's name has already been changed. You cannot change a character's name more than once.
				// 2: User information is not correct.
				// 3: You have failed to change this character's name.
				// 4: Another user is using this character name, so please select another one.
				FIFOSD_CHECK(6);
				{
					chr->parse_char_rename_char_confirm(fd, sd);
				}
				break;

			// captcha code request (not implemented)
			// R 07e5 <?>.w <aid>.l
			case 0x7e5:
				chr->parse_char_request_captcha(fd);
				break;

			// captcha code check (not implemented)
			// R 07e7 <len>.w <aid>.l <code>.b10 <?>.b14
			case 0x7e7:
				chr->parse_char_check_captcha(fd);
			break;

			// deletion timer request
			case 0x827:
				FIFOSD_CHECK(6);
				chr->parse_char_delete2_req(fd, sd);
			break;

			// deletion accept request
			case 0x829:
				FIFOSD_CHECK(12);
				chr->parse_char_delete2_accept(fd, sd);
			break;

			// deletion cancel request
			case 0x82b:
				FIFOSD_CHECK(6);
				chr->parse_char_delete2_cancel(fd, sd);
			break;

			// login as map-server
			case 0x2af8:
				if (RFIFOREST(fd) < 60)
					return 0;
			{
				chr->parse_char_login_map_server(fd, ipl);
			}
			return 0; // avoid processing of follow-up packets here

			// checks the entered pin
			case 0x8b8:
				FIFOSD_CHECK(10);
				chr->parse_char_pincode_check(fd, sd);
			break;

			// request for PIN window
			case 0x8c5:
				FIFOSD_CHECK(6);
				chr->parse_char_pincode_window(fd, sd);
			break;

			// pincode change request
			case 0x8be:
				FIFOSD_CHECK(14);
				chr->parse_char_pincode_change(fd, sd);
			break;

			// activate PIN system and set first PIN
			case 0x8ba:
				FIFOSD_CHECK(10);
				chr->parse_char_pincode_first_pin(fd, sd);
			break;

			case 0x9a1:
				FIFOSD_CHECK(2);
				chr->parse_char_request_chars(fd, sd);
			break;

			/* 0x8d4 <from>.W <to>.W <unused>.W (2+2+2+2) */
			case 0x8d4:
				FIFOSD_CHECK(8);
				{
					chr->parse_char_move_character(fd, sd);
				}
			break;

			// unknown packet received
			default:
				if (chr->parse_char_unknown_packet(fd, ipl))
					return 0;
			}
	}

	RFIFOFLUSH(fd);
	return 0;
}

int mapif_sendall(const unsigned char *buf, unsigned int len)
{
	int i, c;

	nullpo_ret(buf);
	c = 0;
	for(i = 0; i < ARRAYLENGTH(chr->server); i++) {
		int fd;
		if ((fd = chr->server[i].fd) > 0) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len)
{
	int i, c;

	nullpo_ret(buf);
	c = 0;
	for(i = 0; i < ARRAYLENGTH(chr->server); i++) {
		int fd;
		if ((fd = chr->server[i].fd) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

int mapif_send(int fd, unsigned char *buf, unsigned int len)
{
	nullpo_ret(buf);
	if (fd >= 0) {
		int i;
		ARR_FIND( 0, ARRAYLENGTH(chr->server), i, fd == chr->server[i].fd );
		if( i < ARRAYLENGTH(chr->server) )
		{
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			return 1;
		}
	}
	return 0;
}

void mapif_send_users_count(int users)
{
	uint8 buf[6];
	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif->sendall(buf,6);
}

int char_broadcast_user_count(int tid, int64 tick, int id, intptr_t data) {
	int users = chr->count_users();

	// only send an update when needed
	static int prev_users = 0;
	if( prev_users == users )
		return 0;
	prev_users = users;

	if( chr->login_fd > 0 && sockt->session[chr->login_fd] )
	{
		// send number of user to login server
		loginif->send_users_count(users);
	}

	mapif->send_users_count(users);

	return 0;
}

/**
 * Load this character's account id into the 'online accounts' packet
 * @see DBApply
 */
static int char_send_accounts_tologin_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct online_char_data* character = DB->data2ptr(data);
	int* i = va_arg(ap, int*);

	nullpo_ret(character);
	if(character->server > -1)
	{
		WFIFOL(chr->login_fd,8+(*i)*4) = character->account_id;
		(*i)++;
		return 1;
	}
	return 0;
}

int char_send_accounts_tologin(int tid, int64 tick, int id, intptr_t data) {
	if (chr->login_fd > 0 && sockt->session[chr->login_fd])
	{
		// send account list to login server
		int users = chr->online_char_db->size(chr->online_char_db);
		int i = 0;

		WFIFOHEAD(chr->login_fd,8+users*4);
		WFIFOW(chr->login_fd,0) = 0x272d;
		chr->online_char_db->foreach(chr->online_char_db, chr->send_accounts_tologin_sub, &i, users);
		WFIFOW(chr->login_fd,2) = 8+ i*4;
		WFIFOL(chr->login_fd,4) = i;
		WFIFOSET(chr->login_fd,WFIFOW(chr->login_fd,2));
	}
	return 0;
}

int char_check_connect_login_server(int tid, int64 tick, int id, intptr_t data) {
	if (chr->login_fd > 0 && sockt->session[chr->login_fd] != NULL)
		return 0;

	ShowInfo("Attempt to connect to login-server...\n");

	if ((chr->login_fd = sockt->make_connection(login_ip, login_port, NULL)) == -1) { //Try again later. [Skotlex]
		chr->login_fd = 0;
		return 0;
	}

	sockt->session[chr->login_fd]->func_parse = chr->parse_fromlogin;
	sockt->session[chr->login_fd]->flag.server = 1;
	sockt->realloc_fifo(chr->login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

	loginif->connect_to_server();

	return 1;
}

//------------------------------------------------
//Invoked 15 seconds after mapif->disconnectplayer in case the map server doesn't
//replies/disconnect the player we tried to kick. [Skotlex]
//------------------------------------------------
static int char_waiting_disconnect(int tid, int64 tick, int id, intptr_t data) {
	struct online_char_data* character;
	if ((character = (struct online_char_data*)idb_get(chr->online_char_db, id)) != NULL && character->waiting_disconnect == tid) {
		//Mark it offline due to timeout.
		character->waiting_disconnect = INVALID_TIMER;
		chr->set_char_offline(character->char_id, character->account_id);
	}
	return 0;
}

/**
 * @see DBApply
 */
static int char_online_data_cleanup_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct online_char_data *character= DB->data2ptr(data);
	nullpo_ret(character);
	if (character->fd != -1)
		return 0; //Character still connected
	if (character->server == -2) //Unknown server.. set them offline
		chr->set_char_offline(character->char_id, character->account_id);
	if (character->server < 0)
		//Free data from players that have not been online for a while.
		db_remove(chr->online_char_db, key);
	return 0;
}

static int char_online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	chr->online_char_db->foreach(chr->online_char_db, chr->online_data_cleanup_sub);
	return 0;
}

void char_sql_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if(!strcmpi(w1,"char_db"))
			safestrncpy(char_db, w2, sizeof(char_db));
		else if(!strcmpi(w1,"scdata_db"))
			safestrncpy(scdata_db, w2, sizeof(scdata_db));
		else if(!strcmpi(w1,"cart_db"))
			safestrncpy(cart_db, w2, sizeof(cart_db));
		else if(!strcmpi(w1,"inventory_db"))
			safestrncpy(inventory_db, w2, sizeof(inventory_db));
		else if(!strcmpi(w1,"charlog_db"))
			safestrncpy(charlog_db, w2, sizeof(charlog_db));
		else if(!strcmpi(w1,"storage_db"))
			safestrncpy(storage_db, w2, sizeof(storage_db));
		else if(!strcmpi(w1,"skill_db"))
			safestrncpy(skill_db, w2, sizeof(skill_db));
		else if(!strcmpi(w1,"interlog_db"))
			safestrncpy(interlog_db, w2, sizeof(interlog_db));
		else if(!strcmpi(w1,"memo_db"))
			safestrncpy(memo_db, w2, sizeof(memo_db));
		else if(!strcmpi(w1,"guild_db"))
			safestrncpy(guild_db, w2, sizeof(guild_db));
		else if(!strcmpi(w1,"guild_alliance_db"))
			safestrncpy(guild_alliance_db, w2, sizeof(guild_alliance_db));
		else if(!strcmpi(w1,"guild_castle_db"))
			safestrncpy(guild_castle_db, w2, sizeof(guild_castle_db));
		else if(!strcmpi(w1,"guild_expulsion_db"))
			safestrncpy(guild_expulsion_db, w2, sizeof(guild_expulsion_db));
		else if(!strcmpi(w1,"guild_member_db"))
			safestrncpy(guild_member_db, w2, sizeof(guild_member_db));
		else if(!strcmpi(w1,"guild_skill_db"))
			safestrncpy(guild_skill_db, w2, sizeof(guild_skill_db));
		else if(!strcmpi(w1,"guild_position_db"))
			safestrncpy(guild_position_db, w2, sizeof(guild_position_db));
		else if(!strcmpi(w1,"guild_storage_db"))
			safestrncpy(guild_storage_db, w2, sizeof(guild_storage_db));
		else if(!strcmpi(w1,"party_db"))
			safestrncpy(party_db, w2, sizeof(party_db));
		else if(!strcmpi(w1,"pet_db"))
			safestrncpy(pet_db, w2, sizeof(pet_db));
		else if(!strcmpi(w1,"mail_db"))
			safestrncpy(mail_db, w2, sizeof(mail_db));
		else if(!strcmpi(w1,"auction_db"))
			safestrncpy(auction_db, w2, sizeof(auction_db));
		else if(!strcmpi(w1,"friend_db"))
			safestrncpy(friend_db, w2, sizeof(friend_db));
		else if(!strcmpi(w1,"hotkey_db"))
			safestrncpy(hotkey_db, w2, sizeof(hotkey_db));
		else if(!strcmpi(w1,"quest_db"))
			safestrncpy(quest_db,w2,sizeof(quest_db));
		else if(!strcmpi(w1,"homunculus_db"))
			safestrncpy(homunculus_db,w2,sizeof(homunculus_db));
		else if(!strcmpi(w1,"skill_homunculus_db"))
			safestrncpy(skill_homunculus_db,w2,sizeof(skill_homunculus_db));
		else if(!strcmpi(w1,"mercenary_db"))
			safestrncpy(mercenary_db,w2,sizeof(mercenary_db));
		else if(!strcmpi(w1,"mercenary_owner_db"))
			safestrncpy(mercenary_owner_db,w2,sizeof(mercenary_owner_db));
		else if(!strcmpi(w1,"ragsrvinfo_db"))
			safestrncpy(ragsrvinfo_db,w2,sizeof(ragsrvinfo_db));
		else if(!strcmpi(w1,"elemental_db"))
			safestrncpy(elemental_db,w2,sizeof(elemental_db));
		else if(!strcmpi(w1,"account_data_db"))
			safestrncpy(account_data_db,w2,sizeof(account_data_db));
		else if(!strcmpi(w1,"char_reg_num_db"))
			safestrncpy(char_reg_num_db, w2, sizeof(char_reg_num_db));
		else if(!strcmpi(w1,"char_reg_str_db"))
			safestrncpy(char_reg_str_db, w2, sizeof(char_reg_str_db));
		else if(!strcmpi(w1,"acc_reg_str_db"))
			safestrncpy(acc_reg_str_db, w2, sizeof(acc_reg_str_db));
		else if(!strcmpi(w1,"acc_reg_num_db"))
			safestrncpy(acc_reg_num_db, w2, sizeof(acc_reg_num_db));
		//support the import command, just like any other config
		else if(!strcmpi(w1,"import"))
			chr->sql_config_read(w2);
		else
			HPM->parseConf(w1, w2, HPCT_CHAR_INTER);
	}
	fclose(fp);
	ShowInfo("Done reading %s.\n", cfgName);
}

void char_config_dispatch(char *w1, char *w2) {
	bool (*dispatch_to[]) (char *w1, char *w2) = {
		/* as many as it needs */
		pincode->config_read
	};
	int i, len = ARRAYLENGTH(dispatch_to);
	for(i = 0; i < len; i++) {
		if( (*dispatch_to[i])(w1,w2) )
			break;/* we found who this belongs to, can stop */
	}
	if (i == len)
		HPM->parseConf(w1, w2, HPCT_CHAR);
}

int char_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");

	if (fp == NULL) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);
		if(strcmpi(w1,"timestamp_format") == 0) {
			safestrncpy(showmsg->timestamp_format, w2, sizeof(showmsg->timestamp_format));
		} else if(strcmpi(w1,"console_silent")==0){
			showmsg->silent = atoi(w2);
			if (showmsg->silent) /* only bother if its actually enabled */
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
			showmsg->stdout_with_ansisequence = config_switch(w2) ? true : false;
		} else if (strcmpi(w1, "userid") == 0) {
			safestrncpy(chr->userid, w2, sizeof(chr->userid));
		} else if (strcmpi(w1, "passwd") == 0) {
			safestrncpy(chr->passwd, w2, sizeof(chr->passwd));
		} else if (strcmpi(w1, "server_name") == 0) {
			safestrncpy(chr->server_name, w2, sizeof(chr->server_name));
		} else if (strcmpi(w1, "wisp_server_name") == 0) {
			if (strlen(w2) >= 4) {
				safestrncpy(wisp_server_name, w2, sizeof(wisp_server_name));
			}
		} else if (strcmpi(w1, "login_ip") == 0) {
			login_ip = sockt->host2ip(w2);
			if (login_ip) {
				char ip_str[16];
				safestrncpy(login_ip_str, w2, sizeof(login_ip_str));
				ShowStatus("Login server IP address : %s -> %s\n", w2, sockt->ip2str(login_ip, ip_str));
			}
		} else if (strcmpi(w1, "login_port") == 0) {
			login_port = atoi(w2);
		} else if (strcmpi(w1, "char_ip") == 0) {
			chr->ip = sockt->host2ip(w2);
			if (chr->ip) {
				char ip_str[16];
				safestrncpy(char_ip_str, w2, sizeof(char_ip_str));
				ShowStatus("Character server IP address : %s -> %s\n", w2, sockt->ip2str(chr->ip, ip_str));
			}
		} else if (strcmpi(w1, "bind_ip") == 0) {
			bind_ip = sockt->host2ip(w2);
			if (bind_ip) {
				char ip_str[16];
				safestrncpy(bind_ip_str, w2, sizeof(bind_ip_str));
				ShowStatus("Character server binding IP address : %s -> %s\n", w2, sockt->ip2str(bind_ip, ip_str));
			}
		} else if (strcmpi(w1, "char_port") == 0) {
			chr->port = atoi(w2);
		} else if (strcmpi(w1, "char_server_type") == 0) {
			chr->server_type = atoi(w2);
		} else if (strcmpi(w1, "char_new") == 0) {
			char_new = (bool)atoi(w2);
		} else if (strcmpi(w1, "char_new_display") == 0) {
			chr->new_display = atoi(w2);
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			max_connect_user = atoi(w2);
			if (max_connect_user < -1)
				max_connect_user = -1; // unlimited online players
		} else if(strcmpi(w1, "gm_allow_group") == 0) {
			gm_allow_group = atoi(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2) * 1000;
			if (autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if (strcmpi(w1, "save_log") == 0) {
			save_log = config_switch(w2);
		}
		#ifdef RENEWAL
			else if (strcmpi(w1, "start_point_re") == 0) {
				char map[MAP_NAME_LENGTH_EXT];
				int x, y;
				if (sscanf(w2, "%15[^,],%d,%d", map, &x, &y) < 3)
					continue;
				start_point.map = mapindex->name2id(map);
				if (!start_point.map)
					ShowError("Specified start_point_re '%s' not found in map-index cache.\n", map);
				start_point.x = x;
				start_point.y = y;
			}
		#else
			else if (strcmpi(w1, "start_point_pre") == 0) {
				char map[MAP_NAME_LENGTH_EXT];
				int x, y;
				if (sscanf(w2, "%15[^,],%d,%d", map, &x, &y) < 3)
					continue;
				start_point.map = mapindex->name2id(map);
				if (!start_point.map)
					ShowError("Specified start_point_pre '%s' not found in map-index cache.\n", map);
				start_point.x = x;
				start_point.y = y;
			}
		#endif
		else if (strcmpi(w1, "start_items") == 0) {
			int i;
			char *split;

			i = 0;
			split = strtok(w2, ",");
			while (split != NULL && i < MAX_START_ITEMS * 3) {
				char *split2 = split;
				split = strtok(NULL, ",");
				start_items[i] = atoi(split2);

				if (start_items[i] < 0)
					start_items[i] = 0;

				++i;
			}

			// Format is: id1,quantity1,stackable1,idN,quantityN,stackableN
			if( i%3 )
			{
				ShowWarning("chr->config_read: There are not enough parameters in start_items, ignoring last item...\n");
				if( i%3 == 1 )
					start_items[i-1] = 0;
				else
					start_items[i-2] = 0;
			}
		} else if (strcmpi(w1, "start_zeny") == 0) {
			start_zeny = atoi(w2);
			if (start_zeny < 0)
				start_zeny = 0;
		} else if(strcmpi(w1,"log_char")==0) {
			log_char = atoi(w2); //log char or not [devil]
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			safestrncpy(unknown_char_name, w2, sizeof(unknown_char_name));
			unknown_char_name[NAME_LENGTH-1] = '\0';
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			name_ignoring_case = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			safestrncpy(char_name_letters, w2, sizeof(char_name_letters));
		} else if (strcmpi(w1, "char_del_level") == 0) { //disable/enable char deletion by its level condition [Lupus]
			char_del_level = atoi(w2);
		} else if (strcmpi(w1, "char_del_delay") == 0) {
			char_del_delay = atoi(w2);
		} else if (strcmpi(w1, "char_aegis_delete") == 0) {
			char_aegis_delete = atoi(w2);
		} else if(strcmpi(w1,"db_path")==0) {
			safestrncpy(db_path, w2, sizeof(db_path));
		} else if (strcmpi(w1, "fame_list_alchemist") == 0) {
			fame_list_size_chemist = atoi(w2);
			if (fame_list_size_chemist > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_alchemist)\n", MAX_FAME_LIST);
				fame_list_size_chemist = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_blacksmith") == 0) {
			fame_list_size_smith = atoi(w2);
			if (fame_list_size_smith > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_blacksmith)\n", MAX_FAME_LIST);
				fame_list_size_smith = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_taekwon") == 0) {
			fame_list_size_taekwon = atoi(w2);
			if (fame_list_size_taekwon > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_taekwon)\n", MAX_FAME_LIST);
				fame_list_size_taekwon = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "guild_exp_rate") == 0) {
			guild_exp_rate = atoi(w2);
		} else if (strcmpi(w1, "char_maintenance_min_group_id") == 0) {
			char_maintenance_min_group_id = atoi(w2);
		} else if (strcmpi(w1, "import") == 0) {
			chr->config_read(w2);
		} else
			chr->config_dispatch(w1,w2);
	}
	fclose(fp);

	ShowInfo("Done reading %s.\n", cfgName);
	return 0;
}

int do_final(void) {
	int i;

	ShowStatus("Terminating...\n");

	HPM->event(HPET_FINAL);

	chr->set_all_offline(-1);
	chr->set_all_offline_sql();

	inter->final();

	sockt->flush_fifos();

	do_final_mapif();
	loginif->final();

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s`", ragsrvinfo_db) )
		Sql_ShowDebug(inter->sql_handle);

	chr->char_db_->destroy(chr->char_db_, NULL);
	chr->online_char_db->destroy(chr->online_char_db, NULL);
	auth_db->destroy(auth_db, NULL);

	if( chr->char_fd != -1 ) {
		sockt->close(chr->char_fd);
		chr->char_fd = -1;
	}

	HPM_char_do_final();

	SQL->Free(inter->sql_handle);
	mapindex->final();

	for (i = 0; i < MAX_MAP_SERVERS; i++)
		VECTOR_CLEAR(chr->server[i].maps);

	aFree(chr->CHAR_CONF_NAME);
	aFree(chr->NET_CONF_NAME);
	aFree(chr->SQL_CONF_NAME);
	aFree(chr->INTER_CONF_NAME);

	HPM->event(HPET_POST_FINAL);

	ShowStatus("Finished.\n");
	return EXIT_SUCCESS;
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

void set_server_type(void) {
	SERVER_TYPE = SERVER_TYPE_CHAR;
}

/// Called when a terminate signal is received.
void do_shutdown(void)
{
	if( core->runflag != CHARSERVER_ST_SHUTDOWN )
	{
		int id;
		core->runflag = CHARSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		// TODO proper shutdown procedure; wait for acks?, kick all characters, ... [FlavoJS]
		for( id = 0; id < ARRAYLENGTH(chr->server); ++id )
			mapif->server_reset(id);
		loginif->check_shutdown();
		sockt->flush_fifos();
		core->runflag = CORE_ST_STOP;
	}
}

/**
 * --char-config handler
 *
 * Overrides the default char configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(charconfig)
{
	aFree(chr->CHAR_CONF_NAME);
	chr->CHAR_CONF_NAME = aStrdup(params);
	return true;
}
/**
 * --inter-config handler
 *
 * Overrides the default inter-server configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(interconfig)
{
	aFree(chr->INTER_CONF_NAME);
	chr->INTER_CONF_NAME = aStrdup(params);
	return true;
}
/**
 * --net-config handler
 *
 * Overrides the default network configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(netconfig)
{
	aFree(chr->NET_CONF_NAME);
	chr->NET_CONF_NAME = aStrdup(params);
	return true;
}
/**
 * Initializes the command line arguments handlers.
 */
void cmdline_args_init_local(void)
{
	CMDLINEARG_DEF2(char-config, charconfig, "Alternative char-server configuration.", CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(inter-config, interconfig, "Alternative inter-server configuration.", CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(net-config, netconfig, "Alternative network configuration.", CMDLINE_OPT_PARAM);
}

int do_init(int argc, char **argv) {
	int i;
	memset(&skillid2idx, 0, sizeof(skillid2idx));

	char_load_defaults();

	chr->CHAR_CONF_NAME = aStrdup("conf/char-server.conf");
	chr->NET_CONF_NAME = aStrdup("conf/network.conf");
	chr->SQL_CONF_NAME = aStrdup("conf/inter-server.conf");
	chr->INTER_CONF_NAME = aStrdup("conf/inter-server.conf");

	for (i = 0; i < MAX_MAP_SERVERS; i++)
		VECTOR_INIT(chr->server[i].maps);

	HPM_char_do_init();
	cmdline->exec(argc, argv, CMDLINE_OPT_PREINIT);
	HPM->config_read();
	HPM->event(HPET_PRE_INIT);

	//Read map indexes
	mapindex->init();

	#ifdef RENEWAL
		start_point.map = mapindex->name2id("iz_int");
	#else
		start_point.map = mapindex->name2id("new_1-1");
	#endif

	cmdline->exec(argc, argv, CMDLINE_OPT_NORMAL);
	chr->config_read(chr->CHAR_CONF_NAME);
	sockt->net_config_read(chr->NET_CONF_NAME);
	chr->sql_config_read(chr->SQL_CONF_NAME);

	if (strcmp(chr->userid, "s1")==0 && strcmp(chr->passwd, "p1")==0) {
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("And then change the user/password to use in conf/char-server.conf (or conf/import/char_conf.txt)\n");
	}

	inter->init_sql(chr->INTER_CONF_NAME); // inter server configuration

	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);
	chr->online_char_db = idb_alloc(DB_OPT_RELEASE_DATA);

	HPM->event(HPET_INIT);

	chr->mmo_char_sql_init();
	chr->read_fame_list(); //Read fame lists.

	if ((sockt->naddr_ != 0) && (!login_ip || !chr->ip)) {
		char ip_str[16];
		sockt->ip2str(sockt->addr_[0], ip_str);

		if (sockt->naddr_ > 1)
			ShowStatus("Multiple interfaces detected..  using %s as our IP address\n", ip_str);
		else
			ShowStatus("Defaulting to %s as our IP address\n", ip_str);
		if (!login_ip) {
			safestrncpy(login_ip_str, ip_str, sizeof(login_ip_str));
			login_ip = sockt->str2ip(login_ip_str);
		}
		if (!chr->ip) {
			safestrncpy(char_ip_str, ip_str, sizeof(char_ip_str));
			chr->ip = sockt->str2ip(char_ip_str);
		}
	}

	loginif->init();
	do_init_mapif();

	// periodically update the overall user count on all mapservers + login server
	timer->add_func_list(chr->broadcast_user_count, "chr->broadcast_user_count");
	timer->add_interval(timer->gettick() + 1000, chr->broadcast_user_count, 0, 0, 5 * 1000);

	// Timer to clear (chr->online_char_db)
	timer->add_func_list(chr->waiting_disconnect, "chr->waiting_disconnect");

	// Online Data timers (checking if char still connected)
	timer->add_func_list(chr->online_data_cleanup, "chr->online_data_cleanup");
	timer->add_interval(timer->gettick() + 1000, chr->online_data_cleanup, 0, 0, 600 * 1000);

	//Cleaning the tables for NULL entries @ startup [Sirius]
	//Chardb clean
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '0'", char_db) )
		Sql_ShowDebug(inter->sql_handle);

	//guilddb clean
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_lv` = '0' AND `max_member` = '0' AND `exp` = '0' AND `next_exp` = '0' AND `average_lv` = '0'", guild_db) )
		Sql_ShowDebug(inter->sql_handle);

	//guildmemberdb clean
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '0' AND `account_id` = '0' AND `char_id` = '0'", guild_member_db) )
		Sql_ShowDebug(inter->sql_handle);

	sockt->set_defaultparse(chr->parse_char);

	if ((chr->char_fd = sockt->make_listen_bind(bind_ip,chr->port)) == -1) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n",chr->port);
		exit(EXIT_FAILURE);
	}

	Sql_HerculesUpdateCheck(inter->sql_handle);
#ifdef CONSOLE_INPUT
	console->input->setSQL(inter->sql_handle);
	console->display_gplnotice();
#endif
	ShowStatus("The char-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", chr->port);

	if( core->runflag != CORE_ST_STOP )
	{
		core->shutdown_callback = do_shutdown;
		core->runflag = CHARSERVER_ST_RUNNING;
	}

	HPM->event(HPET_READY);

	return 0;
}

void char_load_defaults(void)
{
	mapindex_defaults();
	pincode_defaults();
	char_defaults();
	loginif_defaults();
	mapif_defaults();
	inter_auction_defaults();
	inter_elemental_defaults();
	inter_guild_defaults();
	inter_homunculus_defaults();
	inter_mail_defaults();
	inter_mercenary_defaults();
	inter_party_defaults();
	inter_pet_defaults();
	inter_quest_defaults();
	inter_storage_defaults();
	inter_defaults();
	geoip_defaults();
}

void char_defaults(void)
{
	chr = &char_s;

	memset(chr->server, 0, sizeof(chr->server));

	chr->login_fd = 0;
	chr->char_fd = -1;
	chr->online_char_db = NULL;
	chr->char_db_ = NULL;

	memset(chr->userid, 0, sizeof(chr->userid));
	memset(chr->passwd, 0, sizeof(chr->passwd));
	memset(chr->server_name, 0, sizeof(chr->server_name));

	chr->ip = 0;
	chr->port = 6121;
	chr->server_type = 0;
	chr->new_display = 0;

	chr->waiting_disconnect = char_waiting_disconnect;
	chr->delete_char_sql = char_delete_char_sql;
	chr->create_online_char_data = char_create_online_char_data;
	chr->set_account_online = char_set_account_online;
	chr->set_account_offline = char_set_account_offline;
	chr->set_char_charselect = char_set_char_charselect;
	chr->set_char_online = char_set_char_online;
	chr->set_char_offline = char_set_char_offline;
	chr->db_setoffline = char_db_setoffline;
	chr->db_kickoffline = char_db_kickoffline;
	chr->set_login_all_offline = char_set_login_all_offline;
	chr->set_all_offline = char_set_all_offline;
	chr->set_all_offline_sql = char_set_all_offline_sql;
	chr->create_charstatus = char_create_charstatus;
	chr->mmo_char_tosql = char_mmo_char_tosql;
	chr->memitemdata_to_sql = char_memitemdata_to_sql;
	chr->mmo_gender = char_mmo_gender;
	chr->mmo_chars_fromsql = char_mmo_chars_fromsql;
	chr->mmo_char_fromsql = char_mmo_char_fromsql;
	chr->mmo_char_sql_init = char_mmo_char_sql_init;
	chr->char_slotchange = char_char_slotchange;
	chr->rename_char_sql = char_rename_char_sql;
	chr->check_char_name = char_check_char_name;
	chr->make_new_char_sql = char_make_new_char_sql;
	chr->divorce_char_sql = char_divorce_char_sql;
	chr->count_users = char_count_users;
	chr->mmo_char_tobuf = char_mmo_char_tobuf;
	chr->mmo_char_send099d = char_mmo_char_send099d;
	chr->mmo_char_send_ban_list = char_mmo_char_send_ban_list;
	chr->mmo_char_send_slots_info = char_mmo_char_send_slots_info;
	chr->mmo_char_send_characters = char_mmo_char_send_characters;
	chr->char_married = char_char_married;
	chr->char_child = char_char_child;
	chr->char_family = char_char_family;
	chr->disconnect_player = char_disconnect_player;
	chr->authfail_fd = char_authfail_fd;
	chr->request_account_data = char_request_account_data;
	chr->auth_ok = char_auth_ok;
	chr->ping_login_server = char_ping_login_server;
	chr->parse_fromlogin_connection_state = char_parse_fromlogin_connection_state;
	chr->auth_error = char_auth_error;
	chr->parse_fromlogin_auth_state = char_parse_fromlogin_auth_state;
	chr->parse_fromlogin_account_data = char_parse_fromlogin_account_data;
	chr->parse_fromlogin_login_pong = char_parse_fromlogin_login_pong;
	chr->changesex = char_changesex;
	chr->parse_fromlogin_changesex_reply = char_parse_fromlogin_changesex_reply;
	chr->parse_fromlogin_account_reg2 = char_parse_fromlogin_account_reg2;
	chr->parse_fromlogin_ban = char_parse_fromlogin_ban;
	chr->parse_fromlogin_kick = char_parse_fromlogin_kick;
	chr->update_ip = char_update_ip;
	chr->parse_fromlogin_update_ip = char_parse_fromlogin_update_ip;
	chr->parse_fromlogin_accinfo2_failed = char_parse_fromlogin_accinfo2_failed;
	chr->parse_fromlogin_accinfo2_ok = char_parse_fromlogin_accinfo2_ok;
	chr->parse_fromlogin = char_parse_fromlogin;
	chr->request_accreg2 = char_request_accreg2;
	chr->global_accreg_to_login_start = char_global_accreg_to_login_start;
	chr->global_accreg_to_login_send = char_global_accreg_to_login_send;
	chr->global_accreg_to_login_add = char_global_accreg_to_login_add;
	chr->read_fame_list = char_read_fame_list;
	chr->send_fame_list = char_send_fame_list;
	chr->update_fame_list = char_update_fame_list;
	chr->loadName = char_loadName;
	chr->parse_frommap_datasync = char_parse_frommap_datasync;
	chr->parse_frommap_skillid2idx = char_parse_frommap_skillid2idx;
	chr->map_received_ok = char_map_received_ok;
	chr->send_maps = char_send_maps;
	chr->parse_frommap_map_names = char_parse_frommap_map_names;
	chr->send_scdata = char_send_scdata;
	chr->parse_frommap_request_scdata = char_parse_frommap_request_scdata;
	chr->parse_frommap_set_users_count = char_parse_frommap_set_users_count;
	chr->parse_frommap_set_users = char_parse_frommap_set_users;
	chr->save_character_ack = char_save_character_ack;
	chr->parse_frommap_save_character = char_parse_frommap_save_character;
	chr->select_ack = char_select_ack;
	chr->parse_frommap_char_select_req = char_parse_frommap_char_select_req;
	chr->change_map_server_ack = char_change_map_server_ack;
	chr->parse_frommap_change_map_server = char_parse_frommap_change_map_server;
	chr->parse_frommap_remove_friend = char_parse_frommap_remove_friend;
	chr->char_name_ack = char_char_name_ack;
	chr->parse_frommap_char_name_request = char_parse_frommap_char_name_request;
	chr->parse_frommap_change_email = char_parse_frommap_change_email;
	chr->ban = char_ban;
	chr->unban = char_unban;
	chr->ask_name_ack = char_ask_name_ack;
	chr->changecharsex = char_changecharsex;
	chr->parse_frommap_change_account = char_parse_frommap_change_account;
	chr->parse_frommap_fame_list = char_parse_frommap_fame_list;
	chr->parse_frommap_divorce_char = char_parse_frommap_divorce_char;
	chr->parse_frommap_ragsrvinfo = char_parse_frommap_ragsrvinfo;
	chr->parse_frommap_set_char_offline = char_parse_frommap_set_char_offline;
	chr->parse_frommap_set_all_offline = char_parse_frommap_set_all_offline;
	chr->parse_frommap_set_char_online = char_parse_frommap_set_char_online;
	chr->parse_frommap_build_fame_list = char_parse_frommap_build_fame_list;
	chr->parse_frommap_save_status_change_data = char_parse_frommap_save_status_change_data;
	chr->send_pong = char_send_pong;
	chr->parse_frommap_ping = char_parse_frommap_ping;
	chr->map_auth_ok = char_map_auth_ok;
	chr->map_auth_failed = char_map_auth_failed;
	chr->parse_frommap_auth_request = char_parse_frommap_auth_request;
	chr->parse_frommap_update_ip = char_parse_frommap_update_ip;
	chr->parse_frommap_request_stats_report = char_parse_frommap_request_stats_report;
	chr->parse_frommap_scdata_update = char_parse_frommap_scdata_update;
	chr->parse_frommap_scdata_delete = char_parse_frommap_scdata_delete;
	chr->parse_frommap = char_parse_frommap;
	chr->search_mapserver = char_search_mapserver;
	chr->mapif_init = char_mapif_init;
	chr->lan_subnet_check = char_lan_subnet_check;
	chr->delete2_ack = char_delete2_ack;
	chr->delete2_accept_actual_ack = char_delete2_accept_actual_ack;
	chr->delete2_accept_ack = char_delete2_accept_ack;
	chr->delete2_cancel_ack = char_delete2_cancel_ack;
	chr->delete2_req = char_delete2_req;
	chr->delete2_accept = char_delete2_accept;
	chr->delete2_cancel = char_delete2_cancel;
	chr->send_account_id = char_send_account_id;
	chr->parse_char_connect = char_parse_char_connect;
	chr->send_map_info = char_send_map_info;
	chr->send_wait_char_server = char_send_wait_char_server;
	chr->search_default_maps_mapserver = char_search_default_maps_mapserver;
	chr->parse_char_select = char_parse_char_select;
	chr->creation_failed = char_creation_failed;
	chr->creation_ok = char_creation_ok;
	chr->parse_char_create_new_char = char_parse_char_create_new_char;
	chr->delete_char_failed = char_delete_char_failed;
	chr->delete_char_ok = char_delete_char_ok;
	chr->parse_char_delete_char = char_parse_char_delete_char;
	chr->parse_char_ping = char_parse_char_ping;
	chr->allow_rename = char_allow_rename;
	chr->parse_char_rename_char = char_parse_char_rename_char;
	chr->parse_char_rename_char2 = char_parse_char_rename_char2;
	chr->rename_char_ack = char_rename_char_ack;
	chr->parse_char_rename_char_confirm = char_parse_char_rename_char_confirm;
	chr->captcha_notsupported = char_captcha_notsupported;
	chr->parse_char_request_captcha = char_parse_char_request_captcha;
	chr->parse_char_check_captcha = char_parse_char_check_captcha;
	chr->parse_char_delete2_req = char_parse_char_delete2_req;
	chr->parse_char_delete2_accept = char_parse_char_delete2_accept;
	chr->parse_char_delete2_cancel = char_parse_char_delete2_cancel;
	chr->login_map_server_ack = char_login_map_server_ack;
	chr->parse_char_login_map_server = char_parse_char_login_map_server;
	chr->parse_char_pincode_check = char_parse_char_pincode_check;
	chr->parse_char_pincode_window = char_parse_char_pincode_window;
	chr->parse_char_pincode_change = char_parse_char_pincode_change;
	chr->parse_char_pincode_first_pin = char_parse_char_pincode_first_pin;
	chr->parse_char_request_chars = char_parse_char_request_chars;
	chr->change_character_slot_ack = char_change_character_slot_ack;
	chr->parse_char_move_character = char_parse_char_move_character;
	chr->parse_char_unknown_packet = char_parse_char_unknown_packet;
	chr->parse_char = char_parse_char;
	chr->broadcast_user_count = char_broadcast_user_count;
	chr->send_accounts_tologin_sub = char_send_accounts_tologin_sub;
	chr->send_accounts_tologin = char_send_accounts_tologin;
	chr->check_connect_login_server = char_check_connect_login_server;
	chr->online_data_cleanup_sub = char_online_data_cleanup_sub;
	chr->online_data_cleanup = char_online_data_cleanup;
	chr->sql_config_read = char_sql_config_read;
	chr->config_dispatch = char_config_dispatch;
	chr->config_read = char_config_read;
}
