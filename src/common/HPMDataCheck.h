// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.
#ifndef _HPM_DATA_CHECK_H_
#define _HPM_DATA_CHECK_H_


HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
	#ifdef _COMMON_CONF_H_
		{ "libconfig_interface", sizeof(struct libconfig_interface) },
	#else
		#define _COMMON_CONF_H_
	#endif // _COMMON_CONF_H_
	#ifdef _COMMON_DB_H_
		{ "DBData", sizeof(struct DBData) },
		{ "DBIterator", sizeof(struct DBIterator) },
		{ "DBMap", sizeof(struct DBMap) },
	#else
		#define _COMMON_DB_H_
	#endif // _COMMON_DB_H_
	#ifdef _COMMON_DES_H_
		{ "BIT64", sizeof(struct BIT64) },
	#else
		#define _COMMON_DES_H_
	#endif // _COMMON_DES_H_
	#ifdef _COMMON_ERS_H_
		{ "eri", sizeof(struct eri) },
	#else
		#define _COMMON_ERS_H_
	#endif // _COMMON_ERS_H_
	#ifdef _COMMON_MAPINDEX_H_
		{ "mapindex_interface", sizeof(struct mapindex_interface) },
	#else
		#define _COMMON_MAPINDEX_H_
	#endif // _COMMON_MAPINDEX_H_
	#ifdef _COMMON_MMO_H_
		{ "quest", sizeof(struct quest) },
	#else
		#define _COMMON_MMO_H_
	#endif // _COMMON_MMO_H_
	#ifdef _COMMON_SOCKET_H_
		{ "socket_interface", sizeof(struct socket_interface) },
	#else
		#define _COMMON_SOCKET_H_
	#endif // _COMMON_SOCKET_H_
	#ifdef _COMMON_STRLIB_H_
		{ "StringBuf", sizeof(struct StringBuf) },
		{ "s_svstate", sizeof(struct s_svstate) },
	#else
		#define _COMMON_STRLIB_H_
	#endif // _COMMON_STRLIB_H_
	#ifdef _MAP_ATCOMMAND_H_
		{ "AliasInfo", sizeof(struct AliasInfo) },
		{ "atcommand_interface", sizeof(struct atcommand_interface) },
	#else
		#define _MAP_ATCOMMAND_H_
	#endif // _MAP_ATCOMMAND_H_
	#ifdef _MAP_BATTLE_H_
		{ "Damage", sizeof(struct Damage) },
		{ "battle_interface", sizeof(struct battle_interface) },
	#else
		#define _MAP_BATTLE_H_
	#endif // _MAP_BATTLE_H_
	#ifdef _MAP_BUYINGSTORE_H_
		{ "buyingstore_interface", sizeof(struct buyingstore_interface) },
		{ "s_buyingstore_item", sizeof(struct s_buyingstore_item) },
	#else
		#define _MAP_BUYINGSTORE_H_
	#endif // _MAP_BUYINGSTORE_H_
	#ifdef _MAP_CHRIF_H_
		{ "auth_node", sizeof(struct auth_node) },
	#else
		#define _MAP_CHRIF_H_
	#endif // _MAP_CHRIF_H_
	#ifdef _MAP_CLIF_H_
		{ "clif_interface", sizeof(struct clif_interface) },
	#else
		#define _MAP_CLIF_H_
	#endif // _MAP_CLIF_H_
	#ifdef _MAP_ELEMENTAL_H_
		{ "elemental_skill", sizeof(struct elemental_skill) },
	#else
		#define _MAP_ELEMENTAL_H_
	#endif // _MAP_ELEMENTAL_H_
	#ifdef _MAP_GUILD_H_
		{ "eventlist", sizeof(struct eventlist) },
	#else
		#define _MAP_GUILD_H_
	#endif // _MAP_GUILD_H_
	#ifdef _MAP_MAP_H_
		{ "map_data_other_server", sizeof(struct map_data_other_server) },
	#else
		#define _MAP_MAP_H_
	#endif // _MAP_MAP_H_
	#ifdef _MAP_PACKETS_STRUCT_H_
		{ "EQUIPSLOTINFO", sizeof(struct EQUIPSLOTINFO) },
	#else
		#define _MAP_PACKETS_STRUCT_H_
	#endif // _MAP_PACKETS_STRUCT_H_
	#ifdef _MAP_PC_H_
		{ "autotrade_vending", sizeof(struct autotrade_vending) },
		{ "item_cd", sizeof(struct item_cd) },
	#else
		#define _MAP_PC_H_
	#endif // _MAP_PC_H_
	#ifdef _MAP_SCRIPT_H_
		{ "Script_Config", sizeof(struct Script_Config) },
		{ "script_interface", sizeof(struct script_interface) },
	#else
		#define _MAP_SCRIPT_H_
	#endif // _MAP_SCRIPT_H_
	#ifdef _MAP_SEARCHSTORE_H_
		{ "searchstore_interface", sizeof(struct searchstore_interface) },
	#else
		#define _MAP_SEARCHSTORE_H_
	#endif // _MAP_SEARCHSTORE_H_
	#ifdef _MAP_SKILL_H_
		{ "skill_cd", sizeof(struct skill_cd) },
		{ "skill_condition", sizeof(struct skill_condition) },
		{ "skill_interface", sizeof(struct skill_interface) },
		{ "skill_unit_save", sizeof(struct skill_unit_save) },
	#else
		#define _MAP_SKILL_H_
	#endif // _MAP_SKILL_H_
};
HPExport unsigned int HPMDataCheckLen = ARRAYLENGTH(HPMDataCheck);

#endif /* _HPM_DATA_CHECK_H_ */
