/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2015-2016  Hercules Dev Team
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

#if !defined(HERCULES_CORE)
#ifdef COMMON_UTILS_H /* HCache */
struct HCache_interface *HCache;
#endif // COMMON_UTILS_H
#ifdef MAP_ATCOMMAND_H /* atcommand */
struct atcommand_interface *atcommand;
#endif // MAP_ATCOMMAND_H
#ifdef MAP_BATTLE_H /* battle */
struct battle_interface *battle;
#endif // MAP_BATTLE_H
#ifdef MAP_BATTLEGROUND_H /* bg */
struct battleground_interface *bg;
#endif // MAP_BATTLEGROUND_H
#ifdef MAP_BUYINGSTORE_H /* buyingstore */
struct buyingstore_interface *buyingstore;
#endif // MAP_BUYINGSTORE_H
#ifdef MAP_CHANNEL_H /* channel */
struct channel_interface *channel;
#endif // MAP_CHANNEL_H
#ifdef CHAR_CHAR_H /* chr */
struct char_interface *chr;
#endif // CHAR_CHAR_H
#ifdef MAP_CHAT_H /* chat */
struct chat_interface *chat;
#endif // MAP_CHAT_H
#ifdef MAP_CHRIF_H /* chrif */
struct chrif_interface *chrif;
#endif // MAP_CHRIF_H
#ifdef MAP_CLIF_H /* clif */
struct clif_interface *clif;
#endif // MAP_CLIF_H
#ifdef COMMON_CORE_H /* cmdline */
struct cmdline_interface *cmdline;
#endif // COMMON_CORE_H
#ifdef COMMON_CONSOLE_H /* console */
struct console_interface *console;
#endif // COMMON_CONSOLE_H
#ifdef COMMON_CORE_H /* core */
struct core_interface *core;
#endif // COMMON_CORE_H
#ifdef COMMON_DB_H /* DB */
struct db_interface *DB;
#endif // COMMON_DB_H
#ifdef MAP_DUEL_H /* duel */
struct duel_interface *duel;
#endif // MAP_DUEL_H
#ifdef MAP_ELEMENTAL_H /* elemental */
struct elemental_interface *elemental;
#endif // MAP_ELEMENTAL_H
#ifdef CHAR_GEOIP_H /* geoip */
struct geoip_interface *geoip;
#endif // CHAR_GEOIP_H
#ifdef MAP_GUILD_H /* guild */
struct guild_interface *guild;
#endif // MAP_GUILD_H
#ifdef MAP_STORAGE_H /* gstorage */
struct guild_storage_interface *gstorage;
#endif // MAP_STORAGE_H
#ifdef MAP_HOMUNCULUS_H /* homun */
struct homunculus_interface *homun;
#endif // MAP_HOMUNCULUS_H
#ifdef MAP_INSTANCE_H /* instance */
struct instance_interface *instance;
#endif // MAP_INSTANCE_H
#ifdef CHAR_INT_AUCTION_H /* inter_auction */
struct inter_auction_interface *inter_auction;
#endif // CHAR_INT_AUCTION_H
#ifdef CHAR_INT_ELEMENTAL_H /* inter_elemental */
struct inter_elemental_interface *inter_elemental;
#endif // CHAR_INT_ELEMENTAL_H
#ifdef CHAR_INT_GUILD_H /* inter_guild */
struct inter_guild_interface *inter_guild;
#endif // CHAR_INT_GUILD_H
#ifdef CHAR_INT_HOMUN_H /* inter_homunculus */
struct inter_homunculus_interface *inter_homunculus;
#endif // CHAR_INT_HOMUN_H
#ifdef CHAR_INTER_H /* inter */
struct inter_interface *inter;
#endif // CHAR_INTER_H
#ifdef CHAR_INT_MAIL_H /* inter_mail */
struct inter_mail_interface *inter_mail;
#endif // CHAR_INT_MAIL_H
#ifdef CHAR_INT_MERCENARY_H /* inter_mercenary */
struct inter_mercenary_interface *inter_mercenary;
#endif // CHAR_INT_MERCENARY_H
#ifdef CHAR_INT_PARTY_H /* inter_party */
struct inter_party_interface *inter_party;
#endif // CHAR_INT_PARTY_H
#ifdef CHAR_INT_PET_H /* inter_pet */
struct inter_pet_interface *inter_pet;
#endif // CHAR_INT_PET_H
#ifdef CHAR_INT_QUEST_H /* inter_quest */
struct inter_quest_interface *inter_quest;
#endif // CHAR_INT_QUEST_H
#ifdef CHAR_INT_STORAGE_H /* inter_storage */
struct inter_storage_interface *inter_storage;
#endif // CHAR_INT_STORAGE_H
#ifdef MAP_INTIF_H /* intif */
struct intif_interface *intif;
#endif // MAP_INTIF_H
#ifdef MAP_IRC_BOT_H /* ircbot */
struct irc_bot_interface *ircbot;
#endif // MAP_IRC_BOT_H
#ifdef MAP_ITEMDB_H /* itemdb */
struct itemdb_interface *itemdb;
#endif // MAP_ITEMDB_H
#ifdef LOGIN_LCLIF_H /* lclif */
struct lclif_interface *lclif;
#endif // LOGIN_LCLIF_H
#ifdef COMMON_CONF_H /* libconfig */
struct libconfig_interface *libconfig;
#endif // COMMON_CONF_H
#ifdef MAP_LOG_H /* logs */
struct log_interface *logs;
#endif // MAP_LOG_H
#ifdef LOGIN_LOGIN_H /* login */
struct login_interface *login;
#endif // LOGIN_LOGIN_H
#ifdef CHAR_LOGINIF_H /* loginif */
struct loginif_interface *loginif;
#endif // CHAR_LOGINIF_H
#ifdef MAP_MAIL_H /* mail */
struct mail_interface *mail;
#endif // MAP_MAIL_H
#ifdef COMMON_MEMMGR_H /* iMalloc */
struct malloc_interface *iMalloc;
#endif // COMMON_MEMMGR_H
#ifdef MAP_MAP_H /* map */
struct map_interface *map;
#endif // MAP_MAP_H
#ifdef CHAR_MAPIF_H /* mapif */
struct mapif_interface *mapif;
#endif // CHAR_MAPIF_H
#ifdef COMMON_MAPINDEX_H /* mapindex */
struct mapindex_interface *mapindex;
#endif // COMMON_MAPINDEX_H
#ifdef MAP_MAP_H /* mapit */
struct mapit_interface *mapit;
#endif // MAP_MAP_H
#ifdef MAP_MAPREG_H /* mapreg */
struct mapreg_interface *mapreg;
#endif // MAP_MAPREG_H
#ifdef MAP_MERCENARY_H /* mercenary */
struct mercenary_interface *mercenary;
#endif // MAP_MERCENARY_H
#ifdef MAP_MOB_H /* mob */
struct mob_interface *mob;
#endif // MAP_MOB_H
#ifdef MAP_NPC_H /* npc_chat */
struct npc_chat_interface *npc_chat;
#endif // MAP_NPC_H
#ifdef MAP_NPC_H /* npc */
struct npc_interface *npc;
#endif // MAP_NPC_H
#ifdef COMMON_NULLPO_H /* nullpo */
struct nullpo_interface *nullpo;
#endif // COMMON_NULLPO_H
#ifdef MAP_PARTY_H /* party */
struct party_interface *party;
#endif // MAP_PARTY_H
#ifdef MAP_PATH_H /* path */
struct path_interface *path;
#endif // MAP_PATH_H
#ifdef MAP_PC_GROUPS_H /* pcg */
struct pc_groups_interface *pcg;
#endif // MAP_PC_GROUPS_H
#ifdef MAP_PC_H /* pc */
struct pc_interface *pc;
#endif // MAP_PC_H
#ifdef MAP_NPC_H /* libpcre */
struct pcre_interface *libpcre;
#endif // MAP_NPC_H
#ifdef MAP_PET_H /* pet */
struct pet_interface *pet;
#endif // MAP_PET_H
#ifdef CHAR_PINCODE_H /* pincode */
struct pincode_interface *pincode;
#endif // CHAR_PINCODE_H
#ifdef MAP_QUEST_H /* quest */
struct quest_interface *quest;
#endif // MAP_QUEST_H
#ifdef MAP_SCRIPT_H /* script */
struct script_interface *script;
#endif // MAP_SCRIPT_H
#ifdef MAP_SEARCHSTORE_H /* searchstore */
struct searchstore_interface *searchstore;
#endif // MAP_SEARCHSTORE_H
#ifdef COMMON_SHOWMSG_H /* showmsg */
struct showmsg_interface *showmsg;
#endif // COMMON_SHOWMSG_H
#ifdef MAP_SKILL_H /* skill */
struct skill_interface *skill;
#endif // MAP_SKILL_H
#ifdef COMMON_SOCKET_H /* sockt */
struct socket_interface *sockt;
#endif // COMMON_SOCKET_H
#ifdef COMMON_SQL_H /* SQL */
struct sql_interface *SQL;
#endif // COMMON_SQL_H
#ifdef MAP_STATUS_H /* status */
struct status_interface *status;
#endif // MAP_STATUS_H
#ifdef MAP_STORAGE_H /* storage */
struct storage_interface *storage;
#endif // MAP_STORAGE_H
#ifdef COMMON_STRLIB_H /* StrBuf */
struct stringbuf_interface *StrBuf;
#endif // COMMON_STRLIB_H
#ifdef COMMON_STRLIB_H /* strlib */
struct strlib_interface *strlib;
#endif // COMMON_STRLIB_H
#ifdef COMMON_STRLIB_H /* sv */
struct sv_interface *sv;
#endif // COMMON_STRLIB_H
#ifdef COMMON_SYSINFO_H /* sysinfo */
struct sysinfo_interface *sysinfo;
#endif // COMMON_SYSINFO_H
#ifdef COMMON_TIMER_H /* timer */
struct timer_interface *timer;
#endif // COMMON_TIMER_H
#ifdef MAP_TRADE_H /* trade */
struct trade_interface *trade;
#endif // MAP_TRADE_H
#ifdef MAP_UNIT_H /* unit */
struct unit_interface *unit;
#endif // MAP_UNIT_H
#ifdef MAP_VENDING_H /* vending */
struct vending_interface *vending;
#endif // MAP_VENDING_H
#endif // ! HERCULES_CORE

HPExport const char *HPM_shared_symbols(int server_type)
{
#ifdef COMMON_UTILS_H /* HCache */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("HCache", HCache)) return "HCache";
#endif // COMMON_UTILS_H
#ifdef MAP_ATCOMMAND_H /* atcommand */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("atcommand", atcommand)) return "atcommand";
#endif // MAP_ATCOMMAND_H
#ifdef MAP_BATTLE_H /* battle */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("battle", battle)) return "battle";
#endif // MAP_BATTLE_H
#ifdef MAP_BATTLEGROUND_H /* bg */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("battlegrounds", bg)) return "battlegrounds";
#endif // MAP_BATTLEGROUND_H
#ifdef MAP_BUYINGSTORE_H /* buyingstore */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("buyingstore", buyingstore)) return "buyingstore";
#endif // MAP_BUYINGSTORE_H
#ifdef MAP_CHANNEL_H /* channel */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("channel", channel)) return "channel";
#endif // MAP_CHANNEL_H
#ifdef CHAR_CHAR_H /* chr */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("chr", chr)) return "chr";
#endif // CHAR_CHAR_H
#ifdef MAP_CHAT_H /* chat */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("chat", chat)) return "chat";
#endif // MAP_CHAT_H
#ifdef MAP_CHRIF_H /* chrif */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("chrif", chrif)) return "chrif";
#endif // MAP_CHRIF_H
#ifdef MAP_CLIF_H /* clif */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("clif", clif)) return "clif";
#endif // MAP_CLIF_H
#ifdef COMMON_CORE_H /* cmdline */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("cmdline", cmdline)) return "cmdline";
#endif // COMMON_CORE_H
#ifdef COMMON_CONSOLE_H /* console */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("console", console)) return "console";
#endif // COMMON_CONSOLE_H
#ifdef COMMON_CORE_H /* core */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("core", core)) return "core";
#endif // COMMON_CORE_H
#ifdef COMMON_DB_H /* DB */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("DB", DB)) return "DB";
#endif // COMMON_DB_H
#ifdef MAP_DUEL_H /* duel */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("duel", duel)) return "duel";
#endif // MAP_DUEL_H
#ifdef MAP_ELEMENTAL_H /* elemental */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("elemental", elemental)) return "elemental";
#endif // MAP_ELEMENTAL_H
#ifdef CHAR_GEOIP_H /* geoip */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("geoip", geoip)) return "geoip";
#endif // CHAR_GEOIP_H
#ifdef MAP_GUILD_H /* guild */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("guild", guild)) return "guild";
#endif // MAP_GUILD_H
#ifdef MAP_STORAGE_H /* gstorage */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("gstorage", gstorage)) return "gstorage";
#endif // MAP_STORAGE_H
#ifdef MAP_HOMUNCULUS_H /* homun */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("homun", homun)) return "homun";
#endif // MAP_HOMUNCULUS_H
#ifdef MAP_INSTANCE_H /* instance */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("instance", instance)) return "instance";
#endif // MAP_INSTANCE_H
#ifdef CHAR_INT_AUCTION_H /* inter_auction */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_auction", inter_auction)) return "inter_auction";
#endif // CHAR_INT_AUCTION_H
#ifdef CHAR_INT_ELEMENTAL_H /* inter_elemental */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_elemental", inter_elemental)) return "inter_elemental";
#endif // CHAR_INT_ELEMENTAL_H
#ifdef CHAR_INT_GUILD_H /* inter_guild */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_guild", inter_guild)) return "inter_guild";
#endif // CHAR_INT_GUILD_H
#ifdef CHAR_INT_HOMUN_H /* inter_homunculus */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_homunculus", inter_homunculus)) return "inter_homunculus";
#endif // CHAR_INT_HOMUN_H
#ifdef CHAR_INTER_H /* inter */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter", inter)) return "inter";
#endif // CHAR_INTER_H
#ifdef CHAR_INT_MAIL_H /* inter_mail */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_mail", inter_mail)) return "inter_mail";
#endif // CHAR_INT_MAIL_H
#ifdef CHAR_INT_MERCENARY_H /* inter_mercenary */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_mercenary", inter_mercenary)) return "inter_mercenary";
#endif // CHAR_INT_MERCENARY_H
#ifdef CHAR_INT_PARTY_H /* inter_party */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_party", inter_party)) return "inter_party";
#endif // CHAR_INT_PARTY_H
#ifdef CHAR_INT_PET_H /* inter_pet */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_pet", inter_pet)) return "inter_pet";
#endif // CHAR_INT_PET_H
#ifdef CHAR_INT_QUEST_H /* inter_quest */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_quest", inter_quest)) return "inter_quest";
#endif // CHAR_INT_QUEST_H
#ifdef CHAR_INT_STORAGE_H /* inter_storage */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("inter_storage", inter_storage)) return "inter_storage";
#endif // CHAR_INT_STORAGE_H
#ifdef MAP_INTIF_H /* intif */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("intif", intif)) return "intif";
#endif // MAP_INTIF_H
#ifdef MAP_IRC_BOT_H /* ircbot */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("ircbot", ircbot)) return "ircbot";
#endif // MAP_IRC_BOT_H
#ifdef MAP_ITEMDB_H /* itemdb */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("itemdb", itemdb)) return "itemdb";
#endif // MAP_ITEMDB_H
#ifdef LOGIN_LCLIF_H /* lclif */
if ((server_type&(SERVER_TYPE_LOGIN)) && !HPM_SYMBOL("lclif", lclif)) return "lclif";
#endif // LOGIN_LCLIF_H
#ifdef COMMON_CONF_H /* libconfig */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("libconfig", libconfig)) return "libconfig";
#endif // COMMON_CONF_H
#ifdef MAP_LOG_H /* logs */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("logs", logs)) return "logs";
#endif // MAP_LOG_H
#ifdef LOGIN_LOGIN_H /* login */
if ((server_type&(SERVER_TYPE_LOGIN)) && !HPM_SYMBOL("login", login)) return "login";
#endif // LOGIN_LOGIN_H
#ifdef CHAR_LOGINIF_H /* loginif */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("loginif", loginif)) return "loginif";
#endif // CHAR_LOGINIF_H
#ifdef MAP_MAIL_H /* mail */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("mail", mail)) return "mail";
#endif // MAP_MAIL_H
#ifdef COMMON_MEMMGR_H /* iMalloc */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("iMalloc", iMalloc)) return "iMalloc";
#endif // COMMON_MEMMGR_H
#ifdef MAP_MAP_H /* map */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("map", map)) return "map";
#endif // MAP_MAP_H
#ifdef CHAR_MAPIF_H /* mapif */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("mapif", mapif)) return "mapif";
#endif // CHAR_MAPIF_H
#ifdef COMMON_MAPINDEX_H /* mapindex */
if ((server_type&(SERVER_TYPE_MAP|SERVER_TYPE_CHAR)) && !HPM_SYMBOL("mapindex", mapindex)) return "mapindex";
#endif // COMMON_MAPINDEX_H
#ifdef MAP_MAP_H /* mapit */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("mapit", mapit)) return "mapit";
#endif // MAP_MAP_H
#ifdef MAP_MAPREG_H /* mapreg */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("mapreg", mapreg)) return "mapreg";
#endif // MAP_MAPREG_H
#ifdef MAP_MERCENARY_H /* mercenary */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("mercenary", mercenary)) return "mercenary";
#endif // MAP_MERCENARY_H
#ifdef MAP_MOB_H /* mob */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("mob", mob)) return "mob";
#endif // MAP_MOB_H
#ifdef MAP_NPC_H /* npc_chat */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("npc_chat", npc_chat)) return "npc_chat";
#endif // MAP_NPC_H
#ifdef MAP_NPC_H /* npc */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("npc", npc)) return "npc";
#endif // MAP_NPC_H
#ifdef COMMON_NULLPO_H /* nullpo */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("nullpo", nullpo)) return "nullpo";
#endif // COMMON_NULLPO_H
#ifdef MAP_PARTY_H /* party */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("party", party)) return "party";
#endif // MAP_PARTY_H
#ifdef MAP_PATH_H /* path */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("path", path)) return "path";
#endif // MAP_PATH_H
#ifdef MAP_PC_GROUPS_H /* pcg */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("pc_groups", pcg)) return "pc_groups";
#endif // MAP_PC_GROUPS_H
#ifdef MAP_PC_H /* pc */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("pc", pc)) return "pc";
#endif // MAP_PC_H
#ifdef MAP_NPC_H /* libpcre */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("libpcre", libpcre)) return "libpcre";
#endif // MAP_NPC_H
#ifdef MAP_PET_H /* pet */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("pet", pet)) return "pet";
#endif // MAP_PET_H
#ifdef CHAR_PINCODE_H /* pincode */
if ((server_type&(SERVER_TYPE_CHAR)) && !HPM_SYMBOL("pincode", pincode)) return "pincode";
#endif // CHAR_PINCODE_H
#ifdef MAP_QUEST_H /* quest */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("quest", quest)) return "quest";
#endif // MAP_QUEST_H
#ifdef MAP_SCRIPT_H /* script */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("script", script)) return "script";
#endif // MAP_SCRIPT_H
#ifdef MAP_SEARCHSTORE_H /* searchstore */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("searchstore", searchstore)) return "searchstore";
#endif // MAP_SEARCHSTORE_H
#ifdef COMMON_SHOWMSG_H /* showmsg */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("showmsg", showmsg)) return "showmsg";
#endif // COMMON_SHOWMSG_H
#ifdef MAP_SKILL_H /* skill */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("skill", skill)) return "skill";
#endif // MAP_SKILL_H
#ifdef COMMON_SOCKET_H /* sockt */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("sockt", sockt)) return "sockt";
#endif // COMMON_SOCKET_H
#ifdef COMMON_SQL_H /* SQL */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("SQL", SQL)) return "SQL";
#endif // COMMON_SQL_H
#ifdef MAP_STATUS_H /* status */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("status", status)) return "status";
#endif // MAP_STATUS_H
#ifdef MAP_STORAGE_H /* storage */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("storage", storage)) return "storage";
#endif // MAP_STORAGE_H
#ifdef COMMON_STRLIB_H /* StrBuf */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("StrBuf", StrBuf)) return "StrBuf";
#endif // COMMON_STRLIB_H
#ifdef COMMON_STRLIB_H /* strlib */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("strlib", strlib)) return "strlib";
#endif // COMMON_STRLIB_H
#ifdef COMMON_STRLIB_H /* sv */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("sv", sv)) return "sv";
#endif // COMMON_STRLIB_H
#ifdef COMMON_SYSINFO_H /* sysinfo */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("sysinfo", sysinfo)) return "sysinfo";
#endif // COMMON_SYSINFO_H
#ifdef COMMON_TIMER_H /* timer */
if ((server_type&(SERVER_TYPE_ALL)) && !HPM_SYMBOL("timer", timer)) return "timer";
#endif // COMMON_TIMER_H
#ifdef MAP_TRADE_H /* trade */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("trade", trade)) return "trade";
#endif // MAP_TRADE_H
#ifdef MAP_UNIT_H /* unit */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("unit", unit)) return "unit";
#endif // MAP_UNIT_H
#ifdef MAP_VENDING_H /* vending */
if ((server_type&(SERVER_TYPE_MAP)) && !HPM_SYMBOL("vending", vending)) return "vending";
#endif // MAP_VENDING_H
	return NULL;
}
