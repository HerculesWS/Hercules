/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2026 Hercules Dev Team
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

/*
 * NOTE: This file was auto-generated and should never be manually edited,
 *       as it will get overwritten.
 */

/* GENERATED FILE DO NOT EDIT */

#if !defined(HERCULES_CORE)
  #ifdef MAP_ACHIEVEMENT_H /* achievement */
struct achievement_interface *achievement;
  #endif
  #ifdef MAP_ATCOMMAND_H /* atcommand */
struct atcommand_interface *atcommand;
  #endif
  #ifdef MAP_BATTLE_H /* battle */
struct battle_interface *battle;
  #endif
  #ifdef MAP_BATTLEGROUND_H /* bg */
struct battleground_interface *bg;
  #endif
  #ifdef MAP_BUYINGSTORE_H /* buyingstore */
struct buyingstore_interface *buyingstore;
  #endif
  #ifdef MAP_CHANNEL_H /* channel */
struct channel_interface *channel;
  #endif
  #ifdef MAP_CHAT_H /* chat */
struct chat_interface *chat;
  #endif
  #ifdef MAP_CHRIF_H /* chrif */
struct chrif_interface *chrif;
  #endif
  #ifdef MAP_CLAN_H /* clan */
struct clan_interface *clan;
  #endif
  #ifdef MAP_CLIF_H /* clif */
struct clif_interface *clif;
  #endif
  #ifdef MAP_DUEL_H /* duel */
struct duel_interface *duel;
  #endif
  #ifdef MAP_ELEMENTAL_H /* elemental */
struct elemental_interface *elemental;
  #endif
  #ifdef MAP_ENCHANTUI_H /* enchantui */
struct enchantui_interface *enchantui;
  #endif
  #ifdef MAP_GOLDPC_H /* goldpc */
struct goldpc_interface *goldpc;
  #endif
  #ifdef MAP_GRADER_H /* grader */
struct grader_interface *grader;
  #endif
  #ifdef MAP_GUILD_H /* guild */
struct guild_interface *guild;
  #endif
  #ifdef MAP_STORAGE_H /* gstorage */
struct guild_storage_interface *gstorage;
  #endif
  #ifdef MAP_HOMUNCULUS_H /* homun */
struct homunculus_interface *homun;
  #endif
  #ifdef MAP_INSTANCE_H /* instance */
struct instance_interface *instance;
  #endif
  #ifdef MAP_INTIF_H /* intif */
struct intif_interface *intif;
  #endif
  #ifdef MAP_IRC_BOT_H /* ircbot */
struct ircbot_interface *ircbot;
  #endif
  #ifdef MAP_ITEMDB_H /* itemdb */
struct itemdb_interface *itemdb;
  #endif
  #ifdef MAP_LOG_H /* logs */
struct log_interface *logs;
  #endif
  #ifdef MAP_MACRO_H /* macro */
struct macro_interface *macro;
  #endif
  #ifdef MAP_MAIL_H /* mail */
struct mail_interface *mail;
  #endif
  #ifdef MAP_MAP_H /* map */
struct map_interface *map;
  #endif
  #ifdef MAP_MAPIIF_H /* mapiif */
struct mapiif_interface *mapiif;
  #endif
  #ifdef MAP_MAP_H /* mapit */
struct mapit_interface *mapit;
  #endif
  #ifdef MAP_MAPREG_H /* mapreg */
struct mapreg_interface *mapreg;
  #endif
  #ifdef MAP_MERCENARY_H /* mercenary */
struct mercenary_interface *mercenary;
  #endif
  #ifdef MAP_MOB_H /* mob */
struct mob_interface *mob;
  #endif
  #ifdef MAP_NPC_H /* npc_chat */
struct npc_chat_interface *npc_chat;
  #endif
  #ifdef MAP_NPC_H /* npc */
struct npc_interface *npc;
  #endif
  #ifdef MAP_PARTY_H /* party */
struct party_interface *party;
  #endif
  #ifdef MAP_PATH_H /* path */
struct path_interface *path;
  #endif
  #ifdef MAP_PC_GROUPS_H /* pcg */
struct pc_groups_interface *pcg;
  #endif
  #ifdef MAP_PC_H /* pc */
struct pc_interface *pc;
  #endif
  #ifdef MAP_NPC_H /* libpcre */
struct pcre_interface *libpcre;
  #endif
  #ifdef MAP_PET_H /* pet */
struct pet_interface *pet;
  #endif
  #ifdef MAP_QUEST_H /* quest */
struct quest_interface *quest;
  #endif
  #ifdef MAP_REFINE_H /* refine */
struct refine_interface *refine;
  #endif
  #ifdef MAP_RODEX_H /* rodex */
struct rodex_interface *rodex;
  #endif
  #ifdef MAP_SCRIPT_H /* script */
struct script_interface *script;
  #endif
  #ifdef MAP_SEARCHSTORE_H /* searchstore */
struct searchstore_interface *searchstore;
  #endif
  #ifdef MAP_SKILL_H /* skill */
struct skill_interface *skill;
  #endif
  #ifdef MAP_STATUS_H /* status */
struct status_interface *status;
  #endif
  #ifdef MAP_STORAGE_H /* storage */
struct storage_interface *storage;
  #endif
  #ifdef MAP_STYLIST_H /* stylist */
struct stylist_interface *stylist;
  #endif
  #ifdef MAP_TRADE_H /* trade */
struct trade_interface *trade;
  #endif
  #ifdef MAP_UNIT_H /* unit */
struct unit_interface *unit;
  #endif
  #ifdef MAP_VENDING_H /* vending */
struct vending_interface *vending;
  #endif
#endif // ! HERCULES_CORE

HPExport const char *HPM_shared_symbols(int server_type)
{
#ifdef COMMON_UTILS_H /* HCache */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("HCache", HCache))
		return "HCache";
#endif
#ifdef MAP_ACHIEVEMENT_H /* achievement */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("achievement", achievement))
		return "achievement";
#endif
#ifdef MAP_ATCOMMAND_H /* atcommand */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("atcommand", atcommand))
		return "atcommand";
#endif
#ifdef COMMON_BASE62_H /* base62 */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("base62", base62))
		return "base62";
#endif
#ifdef MAP_BATTLE_H /* battle */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("battle", battle))
		return "battle";
#endif
#ifdef MAP_BATTLEGROUND_H /* bg */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("battlegrounds", bg))
		return "battlegrounds";
#endif
#ifdef MAP_BUYINGSTORE_H /* buyingstore */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("buyingstore", buyingstore))
		return "buyingstore";
#endif
#ifdef MAP_CHANNEL_H /* channel */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("channel", channel))
		return "channel";
#endif
#ifdef MAP_CHAT_H /* chat */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("chat", chat))
		return "chat";
#endif
#ifdef MAP_CHRIF_H /* chrif */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("chrif", chrif))
		return "chrif";
#endif
#ifdef MAP_CLAN_H /* clan */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("clan", clan))
		return "clan";
#endif
#ifdef MAP_CLIF_H /* clif */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("clif", clif))
		return "clif";
#endif
#ifdef COMMON_CORE_H /* cmdline */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("cmdline", cmdline))
		return "cmdline";
#endif
#ifdef COMMON_CONSOLE_H /* console */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("console", console))
		return "console";
#endif
#ifdef COMMON_CORE_H /* core */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("core", core))
		return "core";
#endif
#ifdef COMMON_DB_H /* DB */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("DB", DB))
		return "DB";
#endif
#ifdef COMMON_DES_H /* des */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("des", des))
		return "des";
#endif
#ifdef MAP_DUEL_H /* duel */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("duel", duel))
		return "duel";
#endif
#ifdef MAP_ELEMENTAL_H /* elemental */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("elemental", elemental))
		return "elemental";
#endif
#ifdef MAP_ENCHANTUI_H /* enchantui */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("enchantui", enchantui))
		return "enchantui";
#endif
#ifdef COMMON_EXTRACONF_H /* extraconf */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("extraconf", extraconf))
		return "extraconf";
#endif
#ifdef MAP_GOLDPC_H /* goldpc */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("goldpc", goldpc))
		return "goldpc";
#endif
#ifdef MAP_GRADER_H /* grader */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("grader", grader))
		return "grader";
#endif
#ifdef COMMON_GRFIO_H /* grfio */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("grfio", grfio))
		return "grfio";
#endif
#ifdef MAP_GUILD_H /* guild */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("guild", guild))
		return "guild";
#endif
#ifdef MAP_STORAGE_H /* gstorage */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("gstorage", gstorage))
		return "gstorage";
#endif
#ifdef MAP_HOMUNCULUS_H /* homun */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("homun", homun))
		return "homun";
#endif
#ifdef MAP_INSTANCE_H /* instance */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("instance", instance))
		return "instance";
#endif
#ifdef MAP_INTIF_H /* intif */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("intif", intif))
		return "intif";
#endif
#ifdef MAP_IRC_BOT_H /* ircbot */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("ircbot", ircbot))
		return "ircbot";
#endif
#ifdef MAP_ITEMDB_H /* itemdb */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("itemdb", itemdb))
		return "itemdb";
#endif
#ifdef COMMON_CONF_H /* libconfig */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("libconfig", libconfig))
		return "libconfig";
#endif
#ifdef MAP_LOG_H /* logs */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("logs", logs))
		return "logs";
#endif
#ifdef MAP_MACRO_H /* macro */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("macro", macro))
		return "macro";
#endif
#ifdef MAP_MAIL_H /* mail */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mail", mail))
		return "mail";
#endif
#ifdef MAP_MAP_H /* map */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("map", map))
		return "map";
#endif
#ifdef MAP_MAPIIF_H /* mapiif */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mapiif", mapiif))
		return "mapiif";
#endif
#ifdef COMMON_MAPINDEX_H /* mapindex */
	if ((server_type & (SERVER_TYPE_MAP|SERVER_TYPE_CHAR)) != 0 && !HPM_SYMBOL("mapindex", mapindex))
		return "mapindex";
#endif
#ifdef MAP_MAP_H /* mapit */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mapit", mapit))
		return "mapit";
#endif
#ifdef MAP_MAPREG_H /* mapreg */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mapreg", mapreg))
		return "mapreg";
#endif
#ifdef COMMON_MD5CALC_H /* md5 */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("md5", md5))
		return "md5";
#endif
#ifdef MAP_MERCENARY_H /* mercenary */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mercenary", mercenary))
		return "mercenary";
#endif
#ifdef MAP_MOB_H /* mob */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("mob", mob))
		return "mob";
#endif
#ifdef COMMON_MUTEX_H /* mutex */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("mutex", mutex))
		return "mutex";
#endif
#ifdef MAP_NPC_H /* npc_chat */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("npc_chat", npc_chat))
		return "npc_chat";
#endif
#ifdef MAP_NPC_H /* npc */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("npc", npc))
		return "npc";
#endif
#ifdef COMMON_NULLPO_H /* nullpo */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("nullpo", nullpo))
		return "nullpo";
#endif
#ifdef COMMON_PACKETS_H /* packets */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("packets", packets))
		return "packets";
#endif
#ifdef MAP_PARTY_H /* party */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("party", party))
		return "party";
#endif
#ifdef MAP_PATH_H /* path */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("path", path))
		return "path";
#endif
#ifdef MAP_PC_GROUPS_H /* pcg */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("pc_groups", pcg))
		return "pc_groups";
#endif
#ifdef MAP_PC_H /* pc */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("pc", pc))
		return "pc";
#endif
#ifdef MAP_NPC_H /* libpcre */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("libpcre", libpcre))
		return "libpcre";
#endif
#ifdef MAP_PET_H /* pet */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("pet", pet))
		return "pet";
#endif
#ifdef MAP_QUEST_H /* quest */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("quest", quest))
		return "quest";
#endif
#ifdef MAP_REFINE_H /* refine */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("refine", refine))
		return "refine";
#endif
#ifdef COMMON_RANDOM_H /* rnd */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("rnd", rnd))
		return "rnd";
#endif
#ifdef MAP_RODEX_H /* rodex */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("rodex", rodex))
		return "rodex";
#endif
#ifdef MAP_SCRIPT_H /* script */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("script", script))
		return "script";
#endif
#ifdef MAP_SEARCHSTORE_H /* searchstore */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("searchstore", searchstore))
		return "searchstore";
#endif
#ifdef COMMON_SHOWMSG_H /* showmsg */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("showmsg", showmsg))
		return "showmsg";
#endif
#ifdef MAP_SKILL_H /* skill */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("skill", skill))
		return "skill";
#endif
#ifdef COMMON_SOCKET_H /* sockt */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sockt", sockt))
		return "sockt";
#endif
#ifdef COMMON_SQL_H /* SQL */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("SQL", SQL))
		return "SQL";
#endif
#ifdef MAP_STATUS_H /* status */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("status", status))
		return "status";
#endif
#ifdef MAP_STORAGE_H /* storage */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("storage", storage))
		return "storage";
#endif
#ifdef COMMON_STRLIB_H /* StrBuf */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("StrBuf", StrBuf))
		return "StrBuf";
#endif
#ifdef COMMON_STRLIB_H /* strlib */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("strlib", strlib))
		return "strlib";
#endif
#ifdef MAP_STYLIST_H /* stylist */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("stylist", stylist))
		return "stylist";
#endif
#ifdef COMMON_STRLIB_H /* sv */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sv", sv))
		return "sv";
#endif
#ifdef COMMON_SYSINFO_H /* sysinfo */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("sysinfo", sysinfo))
		return "sysinfo";
#endif
#ifdef COMMON_THREAD_H /* thread */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("thread", thread))
		return "thread";
#endif
#ifdef COMMON_TIMER_H /* timer */
	if ((server_type & (SERVER_TYPE_ALL)) != 0 && !HPM_SYMBOL("timer", timer))
		return "timer";
#endif
#ifdef MAP_TRADE_H /* trade */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("trade", trade))
		return "trade";
#endif
#ifdef MAP_UNIT_H /* unit */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("unit", unit))
		return "unit";
#endif
#ifdef MAP_VENDING_H /* vending */
	if ((server_type & (SERVER_TYPE_MAP)) != 0 && !HPM_SYMBOL("vending", vending))
		return "vending";
#endif
	return NULL;
}
