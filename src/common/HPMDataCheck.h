// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.
#ifndef HPM_DATA_CHECK_H
#define HPM_DATA_CHECK_H


HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
	#ifdef CHAR_CHAR_H
		{ "char_interface", sizeof(struct char_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_CHAR_H
	#endif // CHAR_CHAR_H
	#ifdef CHAR_GEOIP_H
		{ "geoip_interface", sizeof(struct geoip_interface), SERVER_TYPE_CHAR },
		{ "s_geoip", sizeof(struct s_geoip), SERVER_TYPE_CHAR },
	#else
		#define CHAR_GEOIP_H
	#endif // CHAR_GEOIP_H
	#ifdef CHAR_INTER_H
		{ "inter_interface", sizeof(struct inter_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INTER_H
	#endif // CHAR_INTER_H
	#ifdef CHAR_INT_AUCTION_H
		{ "inter_auction_interface", sizeof(struct inter_auction_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_AUCTION_H
	#endif // CHAR_INT_AUCTION_H
	#ifdef CHAR_INT_ELEMENTAL_H
		{ "inter_elemental_interface", sizeof(struct inter_elemental_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_ELEMENTAL_H
	#endif // CHAR_INT_ELEMENTAL_H
	#ifdef CHAR_INT_GUILD_H
		{ "inter_guild_interface", sizeof(struct inter_guild_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_GUILD_H
	#endif // CHAR_INT_GUILD_H
	#ifdef CHAR_INT_HOMUN_H
		{ "inter_homunculus_interface", sizeof(struct inter_homunculus_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_HOMUN_H
	#endif // CHAR_INT_HOMUN_H
	#ifdef CHAR_INT_MAIL_H
		{ "inter_mail_interface", sizeof(struct inter_mail_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_MAIL_H
	#endif // CHAR_INT_MAIL_H
	#ifdef CHAR_INT_MERCENARY_H
		{ "inter_mercenary_interface", sizeof(struct inter_mercenary_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_MERCENARY_H
	#endif // CHAR_INT_MERCENARY_H
	#ifdef CHAR_INT_PARTY_H
		{ "inter_party_interface", sizeof(struct inter_party_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_PARTY_H
	#endif // CHAR_INT_PARTY_H
	#ifdef CHAR_INT_PET_H
		{ "inter_pet_interface", sizeof(struct inter_pet_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_PET_H
	#endif // CHAR_INT_PET_H
	#ifdef CHAR_INT_QUEST_H
		{ "inter_quest_interface", sizeof(struct inter_quest_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_QUEST_H
	#endif // CHAR_INT_QUEST_H
	#ifdef CHAR_INT_STORAGE_H
		{ "inter_storage_interface", sizeof(struct inter_storage_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_INT_STORAGE_H
	#endif // CHAR_INT_STORAGE_H
	#ifdef CHAR_LOGINIF_H
		{ "loginif_interface", sizeof(struct loginif_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_LOGINIF_H
	#endif // CHAR_LOGINIF_H
	#ifdef CHAR_MAPIF_H
		{ "mapif_interface", sizeof(struct mapif_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_MAPIF_H
	#endif // CHAR_MAPIF_H
	#ifdef CHAR_PINCODE_H
		{ "pincode_interface", sizeof(struct pincode_interface), SERVER_TYPE_CHAR },
	#else
		#define CHAR_PINCODE_H
	#endif // CHAR_PINCODE_H
	#ifdef COMMON_CONF_H
		{ "libconfig_interface", sizeof(struct libconfig_interface), SERVER_TYPE_ALL },
	#else
		#define COMMON_CONF_H
	#endif // COMMON_CONF_H
	#ifdef COMMON_DB_H
		{ "DBData", sizeof(struct DBData), SERVER_TYPE_ALL },
		{ "DBIterator", sizeof(struct DBIterator), SERVER_TYPE_ALL },
		{ "DBMap", sizeof(struct DBMap), SERVER_TYPE_ALL },
	#else
		#define COMMON_DB_H
	#endif // COMMON_DB_H
	#ifdef COMMON_DES_H
		{ "BIT64", sizeof(struct BIT64), SERVER_TYPE_ALL },
	#else
		#define COMMON_DES_H
	#endif // COMMON_DES_H
	#ifdef COMMON_ERS_H
		{ "eri", sizeof(struct eri), SERVER_TYPE_ALL },
	#else
		#define COMMON_ERS_H
	#endif // COMMON_ERS_H
	#ifdef COMMON_MAPINDEX_H
		{ "mapindex_interface", sizeof(struct mapindex_interface), SERVER_TYPE_ALL },
	#else
		#define COMMON_MAPINDEX_H
	#endif // COMMON_MAPINDEX_H
	#ifdef COMMON_MMO_H
		{ "quest", sizeof(struct quest), SERVER_TYPE_ALL },
	#else
		#define COMMON_MMO_H
	#endif // COMMON_MMO_H
	#ifdef COMMON_SOCKET_H
		{ "socket_interface", sizeof(struct socket_interface), SERVER_TYPE_ALL },
	#else
		#define COMMON_SOCKET_H
	#endif // COMMON_SOCKET_H
	#ifdef COMMON_STRLIB_H
		{ "StringBuf", sizeof(struct StringBuf), SERVER_TYPE_ALL },
		{ "s_svstate", sizeof(struct s_svstate), SERVER_TYPE_ALL },
	#else
		#define COMMON_STRLIB_H
	#endif // COMMON_STRLIB_H
	#ifdef COMMON_SYSINFO_H
		{ "sysinfo_interface", sizeof(struct sysinfo_interface), SERVER_TYPE_ALL },
	#else
		#define COMMON_SYSINFO_H
	#endif // COMMON_SYSINFO_H
	#ifdef LOGIN_LOGIN_H
		{ "login_interface", sizeof(struct login_interface), SERVER_TYPE_LOGIN },
	#else
		#define LOGIN_LOGIN_H
	#endif // LOGIN_LOGIN_H
	#ifdef MAP_ATCOMMAND_H
		{ "AliasInfo", sizeof(struct AliasInfo), SERVER_TYPE_MAP },
		{ "atcommand_interface", sizeof(struct atcommand_interface), SERVER_TYPE_MAP },
	#else
		#define MAP_ATCOMMAND_H
	#endif // MAP_ATCOMMAND_H
	#ifdef MAP_BATTLE_H
		{ "Damage", sizeof(struct Damage), SERVER_TYPE_MAP },
		{ "battle_interface", sizeof(struct battle_interface), SERVER_TYPE_MAP },
	#else
		#define MAP_BATTLE_H
	#endif // MAP_BATTLE_H
	#ifdef MAP_BUYINGSTORE_H
		{ "buyingstore_interface", sizeof(struct buyingstore_interface), SERVER_TYPE_MAP },
		{ "s_buyingstore_item", sizeof(struct s_buyingstore_item), SERVER_TYPE_MAP },
	#else
		#define MAP_BUYINGSTORE_H
	#endif // MAP_BUYINGSTORE_H
	#ifdef MAP_CHANNEL_H
		{ "Channel_Config", sizeof(struct Channel_Config), SERVER_TYPE_MAP },
	#else
		#define MAP_CHANNEL_H
	#endif // MAP_CHANNEL_H
	#ifdef MAP_CHRIF_H
		{ "auth_node", sizeof(struct auth_node), SERVER_TYPE_MAP },
	#else
		#define MAP_CHRIF_H
	#endif // MAP_CHRIF_H
	#ifdef MAP_CLIF_H
		{ "clif_interface", sizeof(struct clif_interface), SERVER_TYPE_MAP },
	#else
		#define MAP_CLIF_H
	#endif // MAP_CLIF_H
	#ifdef MAP_ELEMENTAL_H
		{ "elemental_skill", sizeof(struct elemental_skill), SERVER_TYPE_MAP },
	#else
		#define MAP_ELEMENTAL_H
	#endif // MAP_ELEMENTAL_H
	#ifdef MAP_GUILD_H
		{ "eventlist", sizeof(struct eventlist), SERVER_TYPE_MAP },
		{ "guardian_data", sizeof(struct guardian_data), SERVER_TYPE_MAP },
	#else
		#define MAP_GUILD_H
	#endif // MAP_GUILD_H
	#ifdef MAP_MAPREG_H
		{ "mapreg_save", sizeof(struct mapreg_save), SERVER_TYPE_MAP },
	#else
		#define MAP_MAPREG_H
	#endif // MAP_MAPREG_H
	#ifdef MAP_MAP_H
		{ "map_data_other_server", sizeof(struct map_data_other_server), SERVER_TYPE_MAP },
	#else
		#define MAP_MAP_H
	#endif // MAP_MAP_H
	#ifdef MAP_PACKETS_STRUCT_H
		{ "EQUIPSLOTINFO", sizeof(struct EQUIPSLOTINFO), SERVER_TYPE_MAP },
	#else
		#define MAP_PACKETS_STRUCT_H
	#endif // MAP_PACKETS_STRUCT_H
	#ifdef MAP_PC_H
		{ "autotrade_vending", sizeof(struct autotrade_vending), SERVER_TYPE_MAP },
		{ "item_cd", sizeof(struct item_cd), SERVER_TYPE_MAP },
	#else
		#define MAP_PC_H
	#endif // MAP_PC_H
	#ifdef MAP_SCRIPT_H
		{ "Script_Config", sizeof(struct Script_Config), SERVER_TYPE_MAP },
		{ "reg_db", sizeof(struct reg_db), SERVER_TYPE_MAP },
		{ "script_interface", sizeof(struct script_interface), SERVER_TYPE_MAP },
	#else
		#define MAP_SCRIPT_H
	#endif // MAP_SCRIPT_H
	#ifdef MAP_SEARCHSTORE_H
		{ "searchstore_interface", sizeof(struct searchstore_interface), SERVER_TYPE_MAP },
	#else
		#define MAP_SEARCHSTORE_H
	#endif // MAP_SEARCHSTORE_H
	#ifdef MAP_SKILL_H
		{ "skill_cd", sizeof(struct skill_cd), SERVER_TYPE_MAP },
		{ "skill_condition", sizeof(struct skill_condition), SERVER_TYPE_MAP },
		{ "skill_interface", sizeof(struct skill_interface), SERVER_TYPE_MAP },
		{ "skill_unit_save", sizeof(struct skill_unit_save), SERVER_TYPE_MAP },
	#else
		#define MAP_SKILL_H
	#endif // MAP_SKILL_H
};
HPExport unsigned int HPMDataCheckLen = ARRAYLENGTH(HPMDataCheck);
HPExport int HPMDataCheckVer = 1;

#endif /* HPM_DATA_CHECK_H */
