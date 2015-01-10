// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.
#ifndef HPM_DATA_CHECK_H
#define HPM_DATA_CHECK_H


HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
	#ifdef COMMON_CONF_H
		{ "libconfig_interface", sizeof(struct libconfig_interface) },
	#else
		#define COMMON_CONF_H
	#endif // COMMON_CONF_H
	#ifdef COMMON_DB_H
		{ "DBData", sizeof(struct DBData) },
		{ "DBIterator", sizeof(struct DBIterator) },
		{ "DBMap", sizeof(struct DBMap) },
	#else
		#define COMMON_DB_H
	#endif // COMMON_DB_H
	#ifdef COMMON_DES_H
		{ "BIT64", sizeof(struct BIT64) },
	#else
		#define COMMON_DES_H
	#endif // COMMON_DES_H
	#ifdef COMMON_ERS_H
		{ "eri", sizeof(struct eri) },
	#else
		#define COMMON_ERS_H
	#endif // COMMON_ERS_H
	#ifdef COMMON_MAPINDEX_H
		{ "mapindex_interface", sizeof(struct mapindex_interface) },
	#else
		#define COMMON_MAPINDEX_H
	#endif // COMMON_MAPINDEX_H
	#ifdef COMMON_MMO_H
		{ "quest", sizeof(struct quest) },
	#else
		#define COMMON_MMO_H
	#endif // COMMON_MMO_H
	#ifdef COMMON_SOCKET_H
		{ "socket_interface", sizeof(struct socket_interface) },
	#else
		#define COMMON_SOCKET_H
	#endif // COMMON_SOCKET_H
	#ifdef COMMON_STRLIB_H
		{ "StringBuf", sizeof(struct StringBuf) },
		{ "s_svstate", sizeof(struct s_svstate) },
	#else
		#define COMMON_STRLIB_H
	#endif // COMMON_STRLIB_H
	#ifdef COMMON_SYSINFO_H
		{ "sysinfo_interface", sizeof(struct sysinfo_interface) },
	#else
		#define COMMON_SYSINFO_H
	#endif // COMMON_SYSINFO_H
	#ifdef MAP_ATCOMMAND_H
		{ "AliasInfo", sizeof(struct AliasInfo) },
		{ "atcommand_interface", sizeof(struct atcommand_interface) },
	#else
		#define MAP_ATCOMMAND_H
	#endif // MAP_ATCOMMAND_H
	#ifdef MAP_BATTLE_H
		{ "Damage", sizeof(struct Damage) },
		{ "battle_interface", sizeof(struct battle_interface) },
	#else
		#define MAP_BATTLE_H
	#endif // MAP_BATTLE_H
	#ifdef MAP_BUYINGSTORE_H
		{ "buyingstore_interface", sizeof(struct buyingstore_interface) },
		{ "s_buyingstore_item", sizeof(struct s_buyingstore_item) },
	#else
		#define MAP_BUYINGSTORE_H
	#endif // MAP_BUYINGSTORE_H
	#ifdef MAP_CHRIF_H
		{ "auth_node", sizeof(struct auth_node) },
	#else
		#define MAP_CHRIF_H
	#endif // MAP_CHRIF_H
	#ifdef MAP_CLIF_H
		{ "clif_interface", sizeof(struct clif_interface) },
	#else
		#define MAP_CLIF_H
	#endif // MAP_CLIF_H
	#ifdef MAP_ELEMENTAL_H
		{ "elemental_skill", sizeof(struct elemental_skill) },
	#else
		#define MAP_ELEMENTAL_H
	#endif // MAP_ELEMENTAL_H
	#ifdef MAP_GUILD_H
		{ "eventlist", sizeof(struct eventlist) },
		{ "guardian_data", sizeof(struct guardian_data) },
	#else
		#define MAP_GUILD_H
	#endif // MAP_GUILD_H
	#ifdef MAP_MAPREG_H
		{ "mapreg_save", sizeof(struct mapreg_save) },
	#else
		#define MAP_MAPREG_H
	#endif // MAP_MAPREG_H
	#ifdef MAP_MAP_H
		{ "map_data_other_server", sizeof(struct map_data_other_server) },
	#else
		#define MAP_MAP_H
	#endif // MAP_MAP_H
	#ifdef MAP_PACKETS_STRUCT_H
		{ "EQUIPSLOTINFO", sizeof(struct EQUIPSLOTINFO) },
	#else
		#define MAP_PACKETS_STRUCT_H
	#endif // MAP_PACKETS_STRUCT_H
	#ifdef MAP_PC_H
		{ "autotrade_vending", sizeof(struct autotrade_vending) },
		{ "item_cd", sizeof(struct item_cd) },
	#else
		#define MAP_PC_H
	#endif // MAP_PC_H
	#ifdef MAP_SCRIPT_H
		{ "Script_Config", sizeof(struct Script_Config) },
		{ "reg_db", sizeof(struct reg_db) },
		{ "script_interface", sizeof(struct script_interface) },
	#else
		#define MAP_SCRIPT_H
	#endif // MAP_SCRIPT_H
	#ifdef MAP_SEARCHSTORE_H
		{ "searchstore_interface", sizeof(struct searchstore_interface) },
	#else
		#define MAP_SEARCHSTORE_H
	#endif // MAP_SEARCHSTORE_H
	#ifdef MAP_SKILL_H
		{ "skill_cd", sizeof(struct skill_cd) },
		{ "skill_condition", sizeof(struct skill_condition) },
		{ "skill_interface", sizeof(struct skill_interface) },
		{ "skill_unit_save", sizeof(struct skill_unit_save) },
	#else
		#define MAP_SKILL_H
	#endif // MAP_SKILL_H
};
HPExport unsigned int HPMDataCheckLen = ARRAYLENGTH(HPMDataCheck);

#endif /* HPM_DATA_CHECK_H */
