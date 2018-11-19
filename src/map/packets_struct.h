/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2018  Hercules Dev Team
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

/* Hercules Renewal: Phase Two http://herc.ws/board/topic/383-hercules-renewal-phase-two/ */

#ifndef MAP_PACKETS_STRUCT_H
#define MAP_PACKETS_STRUCT_H

#include "common/cbasetypes.h"
#include "common/mmo.h"
#include "common/packetsstatic_len.h"

// Packet DB
#define MAX_PACKET_POS 20

/**
 *
 **/
enum packet_headers {
	banking_withdraw_ackType = 0x9aa,
	banking_deposit_ackType = 0x9a8,
	banking_checkType = 0x9a6,
	cart_additem_ackType = 0x12c,
	sc_notickType = 0x196,
#if PACKETVER >= 20141022
	hotkeyType = 0xa00,
#elif PACKETVER >= 20090603
	hotkeyType = 0x7d9,
#else
	hotkeyType = 0x2b9,
#endif
#if PACKETVER >= 20150226
	cartaddType = 0xa0b,
#elif PACKETVER >= 5
	cartaddType = 0x1c5,
#else
	cartaddType = 0x124,
#endif
#if PACKETVER >= 20150226
	storageaddType = 0xa0a,
#elif PACKETVER >= 5
	storageaddType = 0x1c4,
#else
	storageaddType = 0xf4,
#endif
#if PACKETVER >= 20150226
	tradeaddType = 0xa09,
#elif PACKETVER >= 20100223
	tradeaddType = 0x80f,
#else
	tradeaddType = 0x0e9,
#endif
#if PACKETVER < 20061218
	additemType = 0x0a0,
#elif PACKETVER < 20071002
	additemType = 0x29a,
#elif PACKETVER < 20120925
	additemType = 0x2d4,
#elif PACKETVER < 20150226
	additemType = 0x990,
#elif PACKETVER < 20160921
	additemType = 0xa0c,
#else
	additemType = 0xa37,
#endif
#if PACKETVER < 4
	idle_unitType = 0x78,
#elif PACKETVER < 7
	idle_unitType = 0x1d8,
#elif PACKETVER < 20080102
	idle_unitType = 0x22a,
#elif PACKETVER < 20091103
	idle_unitType = 0x2ee,
#elif PACKETVER < 20101124
	idle_unitType = 0x7f9,
#elif PACKETVER < 20120221
	idle_unitType = 0x857,
#elif PACKETVER < 20131223
	idle_unitType = 0x915,
#elif PACKETVER < 20150513
	idle_unitType = 0x9dd,
#else
	idle_unitType = 0x9ff,
#endif
#if PACKETVER >= 20120618
	status_changeType = 0x983,
#elif PACKETVER >= 20090121
	status_changeType = 0x43f,
#else
	status_changeType = sc_notickType,/* 0x196 */
#endif
	status_change2Type = 0x43f,
	status_change_endType = 0x196,
#if PACKETVER < 20091103
	spawn_unit2Type = 0x7c,
	idle_unit2Type = 0x78,
#endif
#if PACKETVER < 20071113
	damageType = 0x8a,
#elif PACKETVER < 20131223
	damageType = 0x2e1,
#else
	damageType = 0x8c8,
#endif
#if PACKETVER < 4
	spawn_unitType = 0x79,
#elif PACKETVER < 7
	spawn_unitType = 0x1d9,
#elif PACKETVER < 20080102
	spawn_unitType = 0x22b,
#elif PACKETVER < 20091103
	spawn_unitType = 0x2ed,
#elif PACKETVER < 20101124
	spawn_unitType = 0x7f8,
#elif PACKETVER < 20120221
	spawn_unitType = 0x858,
#elif PACKETVER < 20131223
	spawn_unitType = 0x90f,
#elif PACKETVER < 20150513
	spawn_unitType = 0x9dc,
#else
	spawn_unitType = 0x9fe,
#endif
#if PACKETVER < 20080102
	authokType = 0x73,
#elif PACKETVER < 20141022
	authokType = 0x2eb,
// Some clients smaller than 20160330 cant be tested [4144]
#elif PACKETVER < 20160330
	authokType = 0xa18,
#else
	authokType = 0x2eb,
#endif
	script_clearType = 0x8d6,
	package_item_announceType = 0x7fd,
	item_drop_announceType = 0x7fd,
#if PACKETVER < 4
	unit_walkingType = 0x7b,
#elif PACKETVER < 7
	unit_walkingType = 0x1da,
#elif PACKETVER < 20080102
	unit_walkingType = 0x22c,
#elif PACKETVER < 20091103
	unit_walkingType = 0x2ec,
#elif PACKETVER < 20101124
	unit_walkingType = 0x7f7,
#elif PACKETVER < 20120221
	unit_walkingType = 0x856,
#elif PACKETVER < 20131223
	unit_walkingType = 0x914,
#elif PACKETVER < 20150513
	unit_walkingType = 0x9db,
#else
	unit_walkingType = 0x9fd,
#endif
	bgqueue_ackType = 0x8d8,
	bgqueue_notice_deleteType = 0x8db,
	bgqueue_registerType = 0x8d7,
	bgqueue_updateinfoType = 0x8d9,
	bgqueue_checkstateType = 0x90a,
	bgqueue_revokereqType = 0x8da,
	bgqueue_battlebeginackType = 0x8e0,
	bgqueue_notify_entryType = 0x8d9,
	bgqueue_battlebeginsType = 0x8df,
	notify_bounditemType = 0x2d3,
#if PACKETVER < 20110718
	skill_entryType = 0x11f,
#elif PACKETVER < 20121212
	skill_entryType = 0x8c7,
#elif PACKETVER < 20130731
	skill_entryType = 0x99f,
#else
	skill_entryType = 0x9ca,
#endif
	graffiti_entryType = 0x1c9,
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	dropflooritemType = 0xadd,
#elif PACKETVER > 20130000 /* not sure date */
	dropflooritemType = 0x84b,
#else
	dropflooritemType = 0x9e,
#endif
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	inventorylistnormalType = 0xb09,
#elif PACKETVER >= 20120925
	inventorylistnormalType = 0x991,
#elif PACKETVER >= 20080102
	inventorylistnormalType = 0x2e8,
#elif PACKETVER >= 20071002
	inventorylistnormalType = 0x1ee,
#else
	inventorylistnormalType = 0xa3,
#endif
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	inventorylistequipType = 0xb0a,
#elif PACKETVER >= 20150226
	inventorylistequipType = 0xa0d,
#elif PACKETVER >= 20120925
	inventorylistequipType = 0x992,
#elif PACKETVER >= 20080102
	inventorylistequipType = 0x2d0,
#elif PACKETVER >= 20071002
	inventorylistequipType = 0x295,
#else
	inventorylistequipType = 0xa4,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	storageListNormalType = 0xb09,
#elif PACKETVER >= 20120925
	storageListNormalType = 0x995,
#elif PACKETVER >= 20080102
	storageListNormalType = 0x2ea,
#elif PACKETVER >= 20071002
	storageListNormalType = 0x295,
#else
	storageListNormalType = 0xa5,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	storageListEquipType = 0xb0a,
#elif PACKETVER >= 20150226
	storageListEquipType = 0xa10,
#elif PACKETVER >= 20120925
	storageListEquipType = 0x996,
#elif PACKETVER >= 20080102
	storageListEquipType = 0x2d1,
#elif PACKETVER >= 20071002
	storageListEquipType = 0x296,
#else
	storageListEquipType = 0xa6,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	cartlistnormalType = 0xb09,
#elif PACKETVER >= 20120925
	cartlistnormalType = 0x993,
#elif PACKETVER >= 20080102
	cartlistnormalType = 0x2e9,
#elif PACKETVER >= 20071002
	cartlistnormalType = 0x1ef,
#else
	cartlistnormalType = 0x123,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	cartlistequipType = 0xb0a,
#elif PACKETVER >= 20150226
	cartlistequipType = 0xa0f,
#elif PACKETVER >= 20120925
	cartlistequipType = 0x994,
#elif PACKETVER >= 20080102
	cartlistequipType = 0x2d2,
#elif PACKETVER >= 20071002
	cartlistequipType = 0x297,
#else
	cartlistequipType = 0x122,
#endif
#if PACKETVER < 20100105
	vendinglistType = 0x133,
#else
	vendinglistType = 0x800,
#endif
	openvendingType = 0x136,
#if PACKETVER >= 20120925
	equipitemType = 0x998,
#else
	equipitemType = 0xa9,
#endif
#if PACKETVER >= 20120925
	equipitemackType = 0x999,
#else
	equipitemackType = 0xaa,
#endif
#if PACKETVER >= 20120925
	unequipitemackType = 0x99a,
#else
	unequipitemackType = 0xac,
#endif
#if PACKETVER_MAIN_NUM >= 20180801 || PACKETVER_RE_NUM >= 20180801 || PACKETVER_ZERO_NUM >= 20180808
	viewequipackType = 0xb03,
#elif PACKETVER >= 20150226
	viewequipackType = 0xa2d,
#elif PACKETVER >= 20120925
	viewequipackType = 0x997,
// [4144] not supported due other packets/structs not updated
//#elif (PACKETVER_MAIN_NUM >= 20111207) || (PACKETVER_RE_NUM >= 20111122)
//	viewequipackType = 0x906,
#elif PACKETVER >= 20101124
	viewequipackType = 0x859,
#else
	viewequipackType = 0x2d7,
#endif
	notifybindonequip = 0x2d3,
	monsterhpType = 0x977,
	maptypeproperty2Type = 0x99b,
	npcmarketresultackType = 0x9d7,
	npcmarketopenType = 0x9d5,
#if PACKETVER >= 20131223  // version probably can be 20131030 [4144]
	wisendType = 0x9df,
#else
	wisendType = 0x98,
#endif
	partyleaderchangedType = 0x7fc,
	rouletteinfoackType = 0xa1c,
	roulettgenerateackType = 0xa20,
	roulettercvitemackType = 0xa22,
#if PACKETVER >= 20141016
	achievementListType = 0xa23,
	achievementUpdateType = 0xa24,
	achievementRewardAckType = 0xa26,
#endif // PACKETVER >= 20141016
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questListType = 0xaff, ///< ZC_ALL_QUEST_LIST4
#elif PACKETVER >= 20150513  // [4144] 0x09f8 handling in client from 2014-10-29aRagexe and 2014-03-26cRagexeRE
	questListType = 0x9f8, ///< ZC_ALL_QUEST_LIST3
#elif PACKETVER >= 20141022
	questListType = 0x97a, ///< ZC_ALL_QUEST_LIST2
#else // PACKETVER < 20141022
	questListType = 0x2b1, ///< ZC_ALL_QUEST_LIST
#endif // PACKETVER >= 20141022
	/* Rodex */
	rodexicon = 0x09E7,
	rodexread = 0x09EB,
	rodexwriteresult = 0x09ED,
	rodexnextpage = 0x09F0,
	rodexgetzeny = 0x09F2,
	rodexgetitem = 0x09F4,
	rodexdelete = 0x09F6,
	rodexadditem = 0x0A05,
	rodexremoveitem = 0x0A07,
	rodexopenwrite = 0x0A12,
#if PACKETVER < 20160601
	rodexmailList = 0x09F0,
#elif PACKETVER < 20170419
	rodexmailList = 0x0A7D,
#else // PACKETVER >= 20170419
	rodexmailList = 0x0Ac2,
#endif
#if PACKETVER < 20160316
	rodexcheckplayer = 0x0A14,
#else // PACKETVER >= 20160316
	rodexcheckplayer = 0x0A51,
#endif
#if PACKETVER >= 20151223
	skillscale = 0xA41,
#endif
#if PACKETVER >= 20130821
	progressbarunit = 0x09D1,
#endif
#if PACKETVER >= 20171207
	partymemberinfo = 0x0ae4,
	partyinfo = 0x0ae5,
#elif PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	partymemberinfo = 0x0a43,
	partyinfo = 0x0a44,
#else
	partymemberinfo = 0x01e9,
	partyinfo = 0x00fb,
#endif
#if PACKETVER >= 20120716
	clanOnlineCount = 0x0988, ///< ZC_NOTIFY_CLAN_CONNECTINFO
	clanLeave = 0x0989, ///< ZC_ACK_CLAN_LEAVE
	clanMessage = 0x098E, ///< ZC_NOTIFY_CLAN_CHAT
#endif
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questAddType = 0xb0c,
#elif PACKETVER >= 20150513 // [4144] 0x09f9 handled in client from 2014-10-29aRagexe and 2014-03-26cRagexeRE
	questAddType = 0x9f9,
#else
	questAddType = 0x2b3,
#endif // PACKETVER < 20150513
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questUpdateType = 0xafe,
#elif PACKETVER >= 20150513
	questUpdateType = 0x9fa,
#else
	questUpdateType = 0x2b5,
#endif // PACKETVER < 20150513
	questUpdateType2 = 0x8fe,
#if PACKETVER >= 20171122
	openUiType = 0xAE2,
#elif PACKETVER >= 20150128
	openUiType = 0xA38,
#endif
#if PACKETVER >= 20180627
	authError = 0xb02,
#elif PACKETVER >= 20101123
	authError = 0x83e,
#else
	authError = 0x6a,
#endif
#if PACKETVER >= 3
	useItemAckType = 0x1c8,
#else
	useItemAckType = 0xa8,
#endif
#if PACKETVER >= 4
	sendLookType = 0x1d7,
#else
	sendLookType = 0xc3,
#endif
#if PACKETVER >= 20141016
	buyingStoreUpdateItemType = 0x9e6,
#else
	buyingStoreUpdateItemType = 0x81b,
#endif
// probably can works also for < 20141223, but in 3CeaM packet size defined only for 20150513
#if PACKETVER >= 20150513
	hominfoType = 0x9f7,
#else
	hominfoType = 0x22e,
#endif
	reqName = 0x95,
#if PACKETVER >= 20150503 // Confirm this?
	reqNameAllType = 0xA30,
#else
	reqNameAllType = 0x195,
#endif
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	skilWarpPointType = 0xabe,
#else
	skilWarpPointType = 0x11c,
#endif
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	guildExpulsion = 0xa82,
#elif PACKETVER >= 20100803
	guildExpulsion = 0x839,
#else
	guildExpulsion = 0x15c,
#endif
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	guildLeave = 0xa83,
#else
	guildLeave = 0x15a,
#endif
#if PACKETVER_RE_NUM >= 20181031
	autoSpellList = 0xafb,
#else
	autoSpellList = 0x1cd,
#endif
};

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * structs for data
 */
struct EQUIPSLOTINFO {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 card[4];
#else
	uint16 card[4];
#endif
} __attribute__((packed));

struct NORMALITEM_INFO {
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	uint8 type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
	int16 count;
#if PACKETVER >= 20120925
	uint32 WearState;
#else
	uint16 WearState;
#endif
#if PACKETVER >= 5
	struct EQUIPSLOTINFO slot;
#endif
#if PACKETVER >= 20080102
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20120925
	struct {
		uint8 IsIdentified : 1;
		uint8 PlaceETCTab : 1;
		uint8 SpareBits : 6;
	} Flag;
#endif
} __attribute__((packed));

struct ItemOptions {
	int16 index;
	int16 value;
	uint8 param;
} __attribute__((packed));

struct EQUIPITEM_INFO {
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	uint8 type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
#if PACKETVER >= 20120925
	uint32 location;
	uint32 WearState;
#else
	uint16 location;
	uint16 WearState;
#endif
#if PACKETVER < 20120925
	uint8 IsDamaged;
#endif
	uint8 RefiningLevel;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20071002
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20080102
	uint16 bindOnEquipType;
#endif
#if PACKETVER >= 20100629
	uint16 wItemSpriteNumber;
#endif
#if PACKETVER >= 20150226
	uint8 option_count;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
#if PACKETVER >= 20120925
	struct {
		uint8 IsIdentified : 1;
		uint8 IsDamaged : 1;
		uint8 PlaceETCTab : 1;
		uint8 SpareBits : 5;
	} Flag;
#endif
} __attribute__((packed));

struct packet_authok {
	int16 PacketType;
	uint32 startTime;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
#if PACKETVER >= 20080102
	int16 font;
#endif
// Some clients smaller than 20160330 cant be tested [4144]
#if PACKETVER >= 20141022 && PACKETVER < 20160330
	uint8 sex;
#endif
} __attribute__((packed));

struct packet_monster_hp {
	int16 PacketType;
	uint32 GID;
	int32 HP;
	int32 MaxHP;
} __attribute__((packed));

struct packet_sc_notick {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
} __attribute__((packed));

struct packet_additem {
	int16 PacketType;
	uint16 Index;
	uint16 count;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 IsIdentified;
	uint8 IsDamaged;
	uint8 refiningLevel;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20120925
	uint32 location;
#else
	uint16 location;
#endif
	uint8 type;
	uint8 result;
#if PACKETVER >= 20061218
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20071002
	uint16 bindOnEquipType;
#endif
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
#if PACKETVER >= 20160921
	uint8 favorite;
	uint16 look;
#endif
} __attribute__((packed));

struct packet_dropflooritem {
	int16 PacketType;
	uint32 ITAID;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
#if PACKETVER >= 20130000 /* not sure date */
	uint16 type;
#endif
	uint8 IsIdentified;
	int16 xPos;
	int16 yPos;
	uint8 subX;
	uint8 subY;
	int16 count;
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	int8 showdropeffect;
	int16 dropeffectmode;
#endif
} __attribute__((packed));
struct packet_idle_unit2 {
#if PACKETVER < 20091103
	int16 PacketType;
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
	int16 effectState;
	int16 job;
	uint16 head;
	uint16 weapon;
	uint16 accessory;
	uint16 shield;
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
	int16 virtue;
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	uint8 state;
	int16 clevel;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit2 {
#if PACKETVER < 20091103
	int16 PacketType;
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
	int16 effectState;
	uint16 head;
	uint16 weapon;
	uint16 accessory;
	int16 job;
	uint16 shield;
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 20080102
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	int16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_unit_walking {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
#endif
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 7
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
	uint32 moveStartTime;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 MoveData[6];
	uint8 xSize;
	uint8 ySize;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	uint16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_idle_unit {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 20080102
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	uint8 state;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	uint16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_status_change {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
#if PACKETVER >= 20120618
	uint32 Total;
#endif
#if PACKETVER >= 20090121
	uint32 Left;
	int32 val1;
	int32 val2;
	int32 val3;
#endif
} __attribute__((packed));

struct packet_status_change_end {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
} __attribute__((packed));

struct packet_status_change2 {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
	uint32 Left;
	int32 val1;
	int32 val2;
	int32 val3;
} __attribute__((packed));

struct packet_maptypeproperty2 {
	int16 PacketType;
	int16 type;
	struct {
		uint32 party             : 1;  // Show attack cursor on non-party members (PvP)
		uint32 guild             : 1;  // Show attack cursor on non-guild members (GvG)
		uint32 siege             : 1;  // Show emblem over characters' heads when in GvG (WoE castle)
		uint32 mineffect         : 1;  // Automatically enable /mineffect
		uint32 nolockon          : 1;  // TODO: What does this do? (shows attack cursor on non-party members)
		uint32 countpk           : 1;  /// Show the PvP counter
		uint32 nopartyformation  : 1;  /// Prevent party creation/modification
		uint32 bg                : 1;  // TODO: What does this do? Probably related to Battlegrounds, but I'm not sure on the effect
		uint32 nocostume         : 1;  /// Does not show costume sprite.
		uint32 usecart           : 1;  /// Allow opening cart inventory
		uint32 summonstarmiracle : 1;  // TODO: What does this do? Related to Taekwon Masters, but I have no idea.
		uint32 SpareBits         : 21; /// Currently ignored, reserved for future updates
	} flag;
} __attribute__((packed));

struct packet_bgqueue_ack {
	int16 PacketType;
	uint8 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notice_delete {
	int16 PacketType;
	uint8 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_register {
	int16 PacketType;
	int16 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_update_info {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
	int32 position;
} __attribute__((packed));

struct packet_bgqueue_checkstate {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_revoke_req {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_battlebegin_ack {
	int16 PacketType;
	uint8 result;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notify_entry {
	int16 PacketType;
	char name[NAME_LENGTH];
	int32 position;
} __attribute__((packed));

struct packet_bgqueue_battlebegins {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_script_clear {
	int16 PacketType;
	uint32 NpcID;
} __attribute__((packed));

/* made possible thanks to Yommy!! */
struct packet_package_item_announce {
	int16 PacketType;
	int16 PacketLength;
	uint8 type;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
	int8 len;
	char Name[NAME_LENGTH];
	int8 unknown;  // probably unused
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 BoxItemID;
#else
	uint16 BoxItemID;
#endif
} __attribute__((packed));

/* made possible thanks to Yommy!! */
struct packet_item_drop_announce {
	int16 PacketType;
	int16 PacketLength;
	uint8 type;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
	int8 len;
	char Name[NAME_LENGTH];
	char monsterNameLen;
	char monsterName[NAME_LENGTH];
} __attribute__((packed));

struct packet_cart_additem_ack {
	int16 PacketType;
	int8 result;
} __attribute__((packed));

struct packet_banking_check {
	int16 PacketType;
	int64 Money;
	int16 Reason;
} __attribute__((packed));

struct packet_banking_deposit_req {
	int16 PacketType;
	uint32 AID;
	int32 Money;
} __attribute__((packed));

struct packet_banking_withdraw_req {
	int16 PacketType;
	uint32 AID;
	int32 Money;
} __attribute__((packed));

struct packet_banking_deposit_ack {
	int16 PacketType;
	int16 Reason;
	int64 Money;
	int32 Balance;
} __attribute__((packed));

struct packet_banking_withdraw_ack {
	int16 PacketType;
	int16 Reason;
	int64 Money;
	int32 Balance;
} __attribute__((packed));

/* Roulette System [Yommy/Hercules] */
struct packet_roulette_open_ack {
	int16 PacketType;
	int8 Result;
	int32 Serial;
	int8 Step;
	int8 Idx;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
	int32 GoldPoint;
	int32 SilverPoint;
	int32 BronzePoint;
} __attribute__((packed));

struct packet_roulette_info_ack {
	int16 PacketType;
	int16 PacketLength;
	uint32 RouletteSerial;
	struct {
		uint16 Row;
		uint16 Position;
#if PACKETVER >= 20180511
		uint32 ItemId;
		uint16 Count;
		uint16 unused;
#else
		uint16 ItemId;
		uint16 Count;
#endif
	} ItemInfo[42];
} __attribute__((packed));

struct packet_roulette_close_ack {
	int16 PacketType;
	uint8 Result;
} __attribute__((packed));

struct packet_roulette_generate_ack {
	int16 PacketType;
	uint8 Result;
	uint16 Step;
	uint16 Idx;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
	int32 RemainGold;
	int32 RemainSilver;
	int32 RemainBronze;
} __attribute__((packed));

struct packet_roulette_itemrecv_req {
	int16 PacketType;
	uint8 Condition;
} __attribute__((packed));

struct packet_roulette_itemrecv_ack {
	int16 PacketType;
	uint8 Result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
} __attribute__((packed));

struct packet_itemlist_normal {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_itemlist_equip {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct ZC_STORE_ITEMLIST_NORMAL {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
	char name[NAME_LENGTH];
#endif
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct ZC_INVENTORY_START {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	int16 packetLength;
#endif
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	char name[];
#else
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct ZC_INVENTORY_END {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	char flag;
} __attribute__((packed));

struct ZC_STORE_ITEMLIST_EQUIP {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
	char name[NAME_LENGTH];
#endif
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_equip_item {
	int16 PacketType;
	uint16 index;
#if PACKETVER >= 20120925
	uint32 wearLocation;
#else
	uint16 wearLocation;
#endif
} __attribute__((packed));

struct packet_equipitem_ack {
	int16 PacketType;
	uint16 index;
#if PACKETVER >= 20120925
	uint32 wearLocation;
#else
	uint16 wearLocation;
#endif
#if PACKETVER >= 20100629
	uint16 wItemSpriteNumber;
#endif
	uint8 result;
} __attribute__((packed));

struct packet_unequipitem_ack {
	int16 PacketType;
	uint16 index;
#if PACKETVER >= 20120925
	uint32 wearLocation;
#else
	uint16 wearLocation;
#endif
	uint8 result;
} __attribute__((packed));

struct packet_viewequip_ack {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
#if PACKETVER >= 20101124
	int16 robe;
#endif
	int16 headpalette;
	int16 bodypalette;
#if PACKETVER_MAIN_NUM >= 20180801 || PACKETVER_RE_NUM >= 20180801 || PACKETVER_ZERO_NUM >= 20180808
	int16 body2;
#endif
	uint8 sex;
	struct EQUIPITEM_INFO list[MAX_INVENTORY];
} __attribute__((packed));

struct packet_notify_bounditem {
	int16 PacketType;
	uint16 index;
} __attribute__((packed));

struct packet_skill_entry {
	int16 PacketType;
#if PACKETVER >= 20110718
	int16 PacketLength;
#endif
	uint32 AID;
	uint32 creatorAID;
	int16 xPos;
	int16 yPos;
#if PACKETVER >= 20121212
	int32 job;
#else
	uint8 job;
#endif
#if PACKETVER >= 20110718
	int8 RadiusRange;
#endif
	uint8 isVisible;
#if PACKETVER >= 20130731
	uint8 level;
#endif
} __attribute__((packed));

struct packet_graffiti_entry {
	int16 PacketType;
	uint32 AID;
	uint32 creatorAID;
	int16 xPos;
	int16 yPos;
	uint8 job;
	uint8 isVisible;
	uint8 isContens;
	char msg[80];
} __attribute__((packed));

struct packet_damage {
	int16 PacketType;
	uint32 GID;
	uint32 targetGID;
	uint32 startTime;
	int32 attackMT;
	int32 attackedMT;
#if PACKETVER < 20071113
	int16 damage;
#else
	int32 damage;
#endif
#if PACKETVER >= 20131223
	uint8 is_sp_damaged;
#endif
	int16 count;
	uint8 action;
#if PACKETVER < 20071113
	int16 leftDamage;
#else
	int32 leftDamage;
#endif
} __attribute__((packed));

struct packet_gm_monster_item {
	int16 PacketType;
#if PACKETVER >= 20131218
	char str[100];
#else
	char str[24];
#endif
} __attribute__((packed));

struct packet_npc_market_purchase {
	int16 PacketType;
	int16 PacketLength;
	struct {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
		uint32 ITID;
#else
		uint16 ITID;
#endif
		int32 qty;
	} list[]; // Note: We assume this should be <= MAX_INVENTORY (since you can't hold more than MAX_INVENTORY items thus cant buy that many at once).
} __attribute__((packed));

struct packet_npc_market_result_ack {
	int16 PacketType;
	int16 PacketLength;
	uint8 result;
	struct {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
		uint32 ITID;
#else
		uint16 ITID;
#endif
		uint16 qty;
		uint32 price;
	} list[MAX_INVENTORY];/* assuming MAX_INVENTORY is max since you can't hold more than MAX_INVENTORY items thus cant buy that many at once. */
} __attribute__((packed));

struct packet_npc_market_open {
	int16 PacketType;
	int16 PacketLength;
	/* inner struct figured by Ind after some annoying hour of debugging (data Thanks to Yommy) */
	struct {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
		uint32 nameid;
#else
		uint16 nameid;
#endif
		uint8 type;
		uint32 price;
		uint32 qty;
		uint16 view;
	// It seems that the client doesn't have any hard-coded limit for this list
	// it's possible to send up to 1890 items without dropping a packet that's
	// too large [Panikon]
	} list[1000];/* TODO: whats the actual max of this? */
} __attribute__((packed));

struct packet_wis_end {
	int16 PacketType;
	int8 result;
#if PACKETVER >= 20131223
	uint32 AID;
#endif
} __attribute__((packed));


struct packet_party_leader_changed {
	int16 PacketType;
	uint32 prev_leader_aid;
	uint32 new_leader_aid;
} __attribute__((packed));

struct packet_hotkey {
#ifdef HOTKEY_SAVING
	int16 PacketType;
#if PACKETVER >= 20141022
	int8 Rotate;
#endif
	struct {
		int8 isSkill; // 0: Item, 1:Skill
		uint32 ID;    // Item/Skill ID
		int16 count;  // Item Quantity/Skill Level
	} hotkey[MAX_HOTKEYS];
#else // not HOTKEY_SAVING
	UNAVAILABLE_STRUCT;
#endif // HOTKEY_SAVING
} __attribute__((packed));

/**
 * MISSION_HUNT_INFO (PACKETVER >= 20141022)
 * MISSION_HUNT_INFO_EX (PACKETVER >= 20150513)
 */
struct packet_mission_info_sub {
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
	uint32 mobType;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
	uint32 mobType;
#endif
	uint32 mob_id;
#if PACKETVER >= 20150513
	int16 levelMin;
	int16 levelMax;
#endif
	int16 huntCount;
	int16 maxCount;
	char mobName[NAME_LENGTH];
} __attribute__((packed));

/**
 * PACKET_ZC_ALL_QUEST_LIST2_INFO (PACKETVER >= 20141022)
 * PACKET_ZC_ALL_QUEST_LIST3_INFO (PACKETVER >= 20150513)
 */
struct packet_quest_list_info {
	int32 questID;
	int8 active;
#if PACKETVER >= 20141022
	int32 quest_svrTime;
	int32 quest_endTime;
	int16 hunting_count;
	struct packet_mission_info_sub objectives[]; // Note: This will be < MAX_QUEST_OBJECTIVES
#endif // PACKETVER >= 20141022
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_ALL_QUEST_LIST (PACKETVER < 20141022)
 * PACKET_ZC_ALL_QUEST_LIST2 (PACKETVER >= 20141022)
 * PACKET_ZC_ALL_QUEST_LIST3 (PACKETVER >= 20150513)
 *
 * @remark
 *     Contains (is followed by) a variable-length array of packet_quest_list_info
 */
struct packet_quest_list_header {
	uint16 PacketType;
	uint16 PacketLength;
	int32 questCount;
	//struct packet_quest_list_info list[]; // Variable-length
} __attribute__((packed));

struct packet_chat_message {
	uint16 packet_id;
	int16 packet_len;
	char message[];
} __attribute__((packed));

struct packet_whisper_message {
	uint16 packet_id;
	int16 packet_len;
	char name[NAME_LENGTH];
	char message[];
} __attribute__((packed));

/* RoDEX */
struct PACKET_CZ_ADD_ITEM_TO_MAIL {
	int16 PacketType;
	int16 index;
	int16 count;
} __attribute__((packed));

struct PACKET_ZC_ADD_ITEM_TO_MAIL {
	int16 PacketType;
	int8 result;
	int16 index;
	int16 count;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 type;
	int8 IsIdentified;
	int8 IsDamaged;
	int8 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions optionData[MAX_ITEM_OPTIONS];
	int16 weight;
	uint8 favorite;
	uint32 location;
} __attribute__((packed));

struct mail_item {
	int16 count;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	int8 IsIdentified;
	int8 IsDamaged;
	int8 refiningLevel;
	struct EQUIPSLOTINFO slot;
	uint32 location;
	uint8 type;
	uint16 viewSprite;
	uint16 bindOnEquip;
	struct ItemOptions optionData[MAX_ITEM_OPTIONS];
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_WRITE_MAIL {
	int16 PacketType;
	char receiveName[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ACK_OPEN_WRITE_MAIL {
	int16 PacketType;
	char receiveName[NAME_LENGTH];
	int8 result;
} __attribute__((packed));

struct PACKET_CZ_REQ_REMOVE_ITEM_MAIL {
	int16 PacketType;
	int16 index;
	uint16 cnt;
} __attribute__((packed));

struct PACKET_ZC_ACK_REMOVE_ITEM_MAIL {
	int16 PacketType;
	int8 result;
	int16 index;
	uint16 cnt;
	int16 weight;
} __attribute__((packed));

struct PACKET_CZ_SEND_MAIL {
	int16 PacketType;
	int16 PacketLength;
	char receiveName[24];
	char senderName[24];
	int64 zeny;
	int16 Titlelength;
	int16 TextcontentsLength;
#if PACKETVER > 20160600
	int32 receiver_char_id;
#endif // PACKETVER > 20160600
	char string[];
} __attribute__((packed));

struct PACKET_ZC_WRITE_MAIL_RESULT {
	int16 PacketType;
	int8 result;
} __attribute__((packed));

struct PACKET_CZ_CHECKNAME {
	int16 PacketType;
	char Name[24];
} __attribute__((packed));

struct PACKET_ZC_CHECKNAME {
	int16 PacketType;
	int32 CharId;
	int16 Class;
	int16 BaseLevel;
#if PACKETVER >= 20160316
	char Name[24];
#endif
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_UNREADMAIL {
	int16 PacketType;
	char result;
} __attribute__((packed));

struct maillistinfo {
#if PACKETVER >= 20170419
	uint8 openType;
#endif
	int64 MailID;
	int8 Isread;
	uint8 type;
	char SenderName[24];
#if PACKETVER < 20170419
	int32 regDateTime;
#endif
	int32 expireDateTime;
	int16 Titlelength;
	char title[];
} __attribute__((packed));

struct PACKET_ZC_MAIL_LIST {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER < 20170419
	int8 opentype;
	int8 cnt;
#endif
	int8 IsEnd;
} __attribute__((packed));

struct PACKET_CZ_REQ_NEXT_MAIL_LIST {
	int16 PacketType;
	int8 opentype;
	int64 Lower_MailID;
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_MAIL {
	int16 PacketType;
#if PACKETVER >= 20170419
	int64 char_Upper_MailID;
	int64 return_Upper_MailID;
	int64 account_Upper_MailID;
#else
	int8 opentype;
	int64 Upper_MailID;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_READ_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

struct PACKET_ZC_READ_MAIL {
	int16 PacketType;
	int16 PacketLength;
	int8 opentype;
	int64 MailID;
	int16 TextcontentsLength;
	int64 zeny;
	int8 ItemCnt;
} __attribute__((packed));

struct PACKET_CZ_REQ_DELETE_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

struct PACKET_ZC_ACK_DELETE_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

struct PACKET_CZ_REQ_REFRESH_MAIL_LIST {
	int16 PacketType;
#if PACKETVER >= 20170419
	int64 Upper_MailID;
	int8 unknown[16];
#else
	int8 opentype;
	int64 Upper_MailID;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_ZENY_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
} __attribute__((packed));

struct PACKET_ZC_ACK_ZENY_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
	int8 result;
} __attribute__((packed));

struct PACKET_CZ_REQ_ITEM_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
} __attribute__((packed));

struct PACKET_ZC_ACK_ITEM_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
	int8 result;
} __attribute__((packed));

struct PACKET_ZC_SKILL_SCALE {
	int16 PacketType;
	uint32 AID;
	int16 skill_id;
	int16 skill_lv;
	int16 x;
	int16 y;
	uint32 casttime;
} __attribute__((packed));

struct ZC_PROGRESS_ACTOR {
	int16 PacketType;
	int32 GID;
	int32 color;
	uint32 time;
} __attribute__((packed));

struct PACKET_ZC_ADD_MEMBER_TO_GROUP {
	int16 packetType;
	uint32 AID;
#if PACKETVER >= 20171207
	uint32 GID;
#endif
	uint32 leader;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	int16 class;
	int16 baseLevel;
#endif
	int16 x;
	int16 y;
	uint8 offline;
	char partyName[NAME_LENGTH];
	char playerName[NAME_LENGTH];
	char mapName[MAP_NAME_LENGTH_EXT];
	int8 sharePickup;
	int8 shareLoot;
} __attribute__((packed));

struct PACKET_ZC_GROUP_LIST_SUB {
	uint32 AID;
#if PACKETVER >= 20171207
	uint32 GID;
#endif
	char playerName[NAME_LENGTH];
	char mapName[MAP_NAME_LENGTH_EXT];
	uint8 leader;
	uint8 offline;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	int16 class;
	int16 baseLevel;
#endif
} __attribute__((packed));

struct PACKET_ZC_GROUP_LIST {
	int16 packetType;
	int16 packetLen;
	char partyName[NAME_LENGTH];
	struct PACKET_ZC_GROUP_LIST_SUB members[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20130626 || PACKETVER_RE_NUM >= 20130605 || defined(PACKETVER_ZERO)
struct PACKET_ZC_CLANINFO {
	int16 PacketType;
	int16 PacketLength;
	uint32 ClanID;
	char ClanName[NAME_LENGTH];
	char MasterName[NAME_LENGTH];
	char Map[MAP_NAME_LENGTH_EXT];
	uint8 AllyCount;
	uint8 AntagonistCount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLANINFO, 0x098a);
#endif

struct PACKET_ZC_NOTIFY_CLAN_CONNECTINFO {
	int16 PacketType;
	int16 NumConnect;
	int16 NumTotal;
} __attribute__((packed));

struct PACKET_ZC_ACK_CLAN_LEAVE {
	int16 PacketType;
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_CLAN_CHAT {
	int16 PacketType;
	int16 PacketLength;
	char MemberName[NAME_LENGTH];
	char Message[];
} __attribute__((packed));

/**
 * PACKET_ZC_MISSION_HUNT (PACKETVER < 20150513)
 * PACKET_ZC_MISSION_HUNT_EX (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_sub {
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
	uint32 mobType;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
	uint32 mobType;
#endif
	uint32 mob_id;
#if PACKETVER >= 20150513
	int16 levelMin;
	int16 levelMax;
#endif
	int16 huntCount;
	char mobName[NAME_LENGTH];
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_ADD_QUEST (PACKETVER < 20150513)
 * PACKET_ZC_ADD_QUEST_EX (PACKETVER >= 20150513)
 */
struct packet_quest_add_header {
	uint16 PacketType;
	uint32 questID;
	uint8 active;
	int32 quest_svrTime;
	int32 quest_endTime;
	int16 count;
	struct packet_quest_hunt_sub objectives[];
} __attribute__((packed));

/**
 * PACKET_MOB_HUNTING (PACKETVER < 20150513)
 * PACKET_MOB_HUNTING_EX (PACKETVER >= 20150513)
 */
struct packet_quest_update_hunt {
	uint32 questID;
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
#else
	uint32 mob_id;
#endif // PACKETVER < 20150513
	int16 maxCount;
	int16 count;
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_UPDATE_MISSION_HUNT (PACKETVER < 20150513)
 * PACKET_ZC_UPDATE_MISSION_HUNT_EX (PACKETVER >= 20150513)
 */
struct packet_quest_update_header {
	uint16 PacketType;
	uint16 PacketLength;
	int16 count;
	struct packet_quest_update_hunt objectives[];
} __attribute__((packed));

/**
 * Header for:
 * PACKET_MOB_HUNTING (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_info_sub {
	uint32 questID;
	uint32 mob_id;
	int16 maxCount;
	int16 count;
} __attribute__((packed));

/**
 * Header for:
 * ZC_HUNTING_QUEST_INFO (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_info {
	uint16 PacketType;
	uint16 PacketLength;
	struct packet_quest_hunt_info_sub info[];
} __attribute__((packed));

struct PACKET_ZC_FORMATSTRING_MSG {
	uint16 PacketType;
	uint16 PacketLength;
	uint16 MessageId;
	char MessageString[];
} __attribute__((packed));

struct PACKET_ZC_FORMATSTRING_MSG_COLOR {
	uint16 PacketType;
	uint16 PacketLength;
	uint16 messageId;
#if PACKETVER >= 20160406
	uint32 color;
#endif
	char messageString[];
} __attribute__((packed));

struct PACKET_ZC_MSG_COLOR {
	uint16 PacketType;
	uint16 MessageId;
	uint32 MessageColor;
} __attribute__((packed));

struct PACKET_CZ_OPEN_UI {
	int16 PacketType;
	int8 UIType;
} __attribute__((packed));

struct PACKET_ZC_OPEN_UI {
	int16 PacketType;
	int8 UIType;
#if PACKETVER >= 20171122
	int32 data;
#endif
} __attribute__((packed));

struct PACKET_ZC_UI_ACTION {
	int16 PacketType;
	int32 UIType;
	int32 data;
} __attribute__((packed));

struct PACKET_CZ_PRIVATE_AIRSHIP_REQUEST {
	int16 PacketType;
	char mapName[MAP_NAME_LENGTH_EXT];
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
} __attribute__((packed));

struct PACKET_ZC_PRIVATE_AIRSHIP_RESPONSE {
	int16 PacketType;
	uint32 flag;
} __attribute__((packed));

struct PACKET_CZ_REQ_STYLE_CHANGE {
	int16 PacketType;
	int16 HeadPalette;
	int16 HeadStyle;
	int16 BodyPalette;
	int16 TopAccessory;
	int16 MidAccessory;
	int16 BottomAccessory;
} __attribute__((packed));

struct PACKET_CZ_REQ_STYLE_CHANGE2 {
	int16 PacketType;
	int16 HeadPalette;
	int16 HeadStyle;
	int16 BodyPalette;
	int16 TopAccessory;
	int16 MidAccessory;
	int16 BottomAccessory;
	int16 BodyStyle;
} __attribute__((packed));

struct PACKET_ZC_STYLE_CHANGE_RES {
	int16 PacketType;
	int8 flag;
} __attribute__((packed));

struct pet_evolution_items {
	int16 index;
	int16 amount;
} __attribute__((packed));

struct PACKET_CZ_PET_EVOLUTION {
	int16 PacketType;
	uint16 PacketLength;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 EvolvedPetEggID;
#else
	uint16 EvolvedPetEggID;
#endif
	// struct pet_evolution_items items[];
} __attribute__((packed));

struct packet_ZC_REFUSE_LOGIN {
	int16 PacketType;
#if PACKETVER >= 20101123
	uint32 error_code;
#else
	uint8 error_code;
#endif
	char block_date[20];
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_CHAT {
	int16 PacketType;
	int16 PacketLength;
	uint32 GID;
	char Message[];
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_PLAYERCHAT {
	int16 PacketType;
	int16 PacketLength;
	char Message[];
} __attribute__((packed));

struct PACKET_ZC_ITEM_ENTRY {
	int16 packetType;
	uint32 AID;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identify;
	uint16 x;
	uint16 y;
	uint16 amount;
	uint8 subX;
	uint8 subY;
} __attribute__((packed));

struct PACKET_ZC_ADD_ITEM_TO_STORE {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
#if PACKETVER >= 5
	uint8 itemType;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_ZC_MVP_GETTING_ITEM {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_TOUSESKILL {
	int16 packetType;
	uint16 skillId;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	int32 btype;
	uint32 itemId;
#else
	int16 btype;
	uint16 itemId;
#endif
	uint8 flag;
	uint8 cause;
} __attribute__((packed));

struct PACKET_ZC_ADD_ITEM_TO_CART {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
#if PACKETVER >= 5
	uint8 itemType;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_CZ_REQMAKINGITEM {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 material[3];
#else
	uint16 itemId;
	uint16 material[3];
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_REQMAKINGITEM {
	int16 packetType;
	int16 result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_FEED_PET {
	int16 packetType;
	uint8 result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_FEED_MER {
	int16 packetType;
	uint8 result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_USE_ITEM_ACK {
	int16 packetType;
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 AID;
#elif PACKETVER >= 3
	uint16 itemId;
	uint32 AID;
#endif
	int16 amount;
	uint8 result;
} __attribute__((packed));

struct PACKET_ZC_SPRITE_CHANGE {
	int16 packetType;
	uint32 AID;
	uint8 type;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 val;
	uint32 val2;
#elif PACKETVER >= 4
	uint16 val;
	uint16 val2;
#else
	uint8 val;
#endif
} __attribute__((packed));

struct PACKET_ZC_ADD_EXCHANGE_ITEM {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint8 itemType;
	int32 amount;
#elif PACKETVER >= 20100223
	uint16 itemId;
	uint8 itemType;
	int32 amount;
#else
	int32 amount;
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_ZC_CASH_TIME_COUNTER {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 seconds;
} __attribute__((packed));

struct PACKET_ZC_CASH_ITEM_DELETE {
	int16 packetType;
	uint16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ITEM_PICKUP_PARTY {
	int16 packetType;
	uint32 AID;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
	uint16 location;
	uint8 itemType;
} __attribute__((packed));

struct PACKET_ZC_UPDATE_ITEM_FROM_BUYING_STORE {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
#if PACKETVER >= 20141016
	uint32 zeny;
	uint32 zenyLimit;
	uint32 charId;
	uint32 updateTime;
#else
	uint32 zenyLimit;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_WEAPONREFINE {
	int16 packetType;
	int32 result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
#if PACKETVER < 20150513
	uint16 hp;
	uint16 maxHp;
#else
	uint32 hp;
	uint32 maxHp;
#endif
	uint16 sp;
	uint16 maxSp;
	uint32 exp;
	uint32 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));

struct PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER {
	int16 packetType;
	uint16 result;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_ITEMREPAIR {
	int16 packetType;
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));

struct PACKET_CZ_REQ_MAKINGITEM {
	int16 packetType;
	int16 type;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_SSILIST_ITEM_CLICK {
	int16 packetType;
	uint32 AID;
	uint32 storeId;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_SCHEDULER_CASHITEM_sub {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 price;
} __attribute__((packed));

struct PACKET_ZC_ACK_SCHEDULER_CASHITEM {
	int16 packetType;
	int16 packetLength;
	int16 count;
	int16 tabNum;
	struct PACKET_ZC_ACK_SCHEDULER_CASHITEM_sub items[];
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub {
	uint32 price;
	int16 index;
	int16 amount;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_MYITEMLIST {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub {
	uint32 price;
	uint32 discountPrice;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_PC_PURCHASE_ITEMLIST_sub {
	uint16 amount;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_PC_PURCHASE_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_CZ_PC_PURCHASE_ITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
	uint32 price;
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 zenyLimit;
	uint8 result;
	char storeName[MESSAGE_SIZE];
	struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MYITEMLIST_BUYING_STORE_sub {
	uint32 price;
	uint16 amount;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MYITEMLIST_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 zenyLimit;
	struct PACKET_ZC_MYITEMLIST_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub {
	uint32 price;
	uint16 amount;
	int16 index;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
// [4144] date 20160921 not confirmed. Can be bigger or smaller
#if PACKETVER >= 20160921
	uint32 location;
	uint16 viewSprite;
#endif
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
#if PACKETVER >= 20100105
	uint32 venderId;
#endif
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub items[];
} __attribute__((packed));

struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub {
	uint32 price;
	uint16 amount;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 storeId;
	uint32 zenyLimit;
	struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub {
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
} __attribute__((packed));

struct PACKET_CZ_REQ_TRADE_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 storeId;
	struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKABLEITEMLIST_sub {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 material[3];
#else
	uint16 itemId;
	uint16 material[3];
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKABLEITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_MAKABLEITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKINGARROW_LIST_sub {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKINGARROW_LIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_MAKINGARROW_LIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_REPAIRITEMLIST_sub {
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;  // unused?
	struct EQUIPSLOTINFO slot;  // unused?
} __attribute__((packed));

struct PACKET_ZC_REPAIRITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_REPAIRITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub {
	int16 index;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;  // unused?
	struct EQUIPSLOTINFO slot;  // unused?
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_WEAPONITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKINGITEM_LIST_sub {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKINGITEM_LIST {
	int16 packetType;
	int16 packetLength;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 makeItem;
#else
	uint16 makeItem;
#endif
	struct PACKET_ZC_MAKINGITEM_LIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub {
	uint32 price;
	uint32 discountPrice;
	uint8 itemType;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_PC_CASH_POINT_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	uint32 cashPoints;
#if PACKETVER >= 20070711
	uint32 kafraPoints;
#endif
	struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM_sub {
	uint16 amount;
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM {
	int16 packetType;
#if PACKETVER >= 20101116
	int16 packetLength;
	uint32 kafraPoints;
	uint16 count;
	struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM_sub items[];
#else
	uint16 itemId;
	uint16 amount;
#if PACKETVER >= 20070711
	uint32 kafraPoints;
#endif
#endif
} __attribute__((packed));

struct PACKET_CZ_SEARCH_STORE_INFO_item {
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_SEARCH_STORE_INFO {
	int16 packetType;
	int16 packetLength;
	uint8 searchType;
	uint32 maxPrice;
	uint32 minPrice;
	uint8 itemsCount;
	uint8 cardsCount;
	struct PACKET_CZ_SEARCH_STORE_INFO_item items[];  // items[itemCount]
/*
	struct PACKET_CZ_SEARCH_STORE_INFO_item cards[cardCount];
*/
} __attribute__((packed));

struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub {
	uint32 storeId;
	uint32 AID;
	char shopName[MESSAGE_SIZE];
#if PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint32 price;
	uint16 amount;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_ZC_SEARCH_STORE_INFO_ACK {
	int16 packetType;
	int16 packetLength;
	uint8 firstPage;
	uint8 nextPage;
	uint8 usesCount;
	struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub items[];
} __attribute__((packed));

/* Achievement System */
struct ach_list_info {
	uint32 ach_id;
	uint8 completed;
	uint32 objective[MAX_ACHIEVEMENT_OBJECTIVES];
	uint32 completed_at;
	uint8 reward;
} __attribute__((packed));

struct packet_achievement_list {
	uint16 packet_id;
	uint16 packet_len;
	uint32 total_achievements;
	uint32 total_points;
	uint16 rank;
	uint32 current_rank_points;
	uint32 next_rank_points;
	struct ach_list_info ach[MAX_ACHIEVEMENT_DB];
} __attribute__((packed));

struct packet_achievement_update {
	uint16 packet_id;
	uint32 total_points;
	uint16 rank;
	uint32 current_rank_points;
	uint32 next_rank_points;
	struct ach_list_info ach;
} __attribute__((packed));

struct packet_achievement_reward_ack {
	uint16 packet_id;
	uint8 failed;
	uint32 ach_id;
} __attribute__((packed));

// Name Packet ZC_ACK_REQNAME
struct packet_reqname_ack {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
} __attribute__((packed));

// ZC_ACK_REQNAMEALL / ZC_ACK_REQNAMEALL2
struct packet_reqnameall_ack {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	char guild_name[NAME_LENGTH];
	char position_name[NAME_LENGTH];
#if PACKETVER >= 20150503 // Confirm this?
	int32 title_id; // Achievement Title
#endif
} __attribute__((packed));

struct PACKET_ZC_OVERWEIGHT_PERCENT {
	int16 packetType;
	uint32 percent;
} __attribute__((packed));

struct PACKET_ZC_WARPLIST_sub {
	char map[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));

struct PACKET_ZC_WARPLIST {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	int16 packetLength;
	uint16 skillId;
	struct PACKET_ZC_WARPLIST_sub maps[];
#else
	uint16 skillId;
	struct PACKET_ZC_WARPLIST_sub maps[4];
#endif
} __attribute__((packed));

struct PACKET_ZC_GROUP_ISALIVE {
	int16 packetType;
	uint32 AID;
	uint8 isDead;
} __attribute__((packed));

struct PACKET_ZC_GUILD_POSITION {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	char position[];
} __attribute__((packed));

struct PACKET_ZC_INVENTORY_MOVE_FAILED {
	int16 packetType;
	int16 index;
	int16 unknown;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD3
#elif PACKETVER >= 20100803
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD2
#else
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD1
#endif

struct PACKET_ZC_ACK_BAN_GUILD1 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
	char account_name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ACK_BAN_GUILD2 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
} __attribute__((packed));

struct PACKET_ZC_ACK_BAN_GUILD3 {
	int16 packetType;
	char reason[40];
	uint32 GID;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
#define PACKET_ZC_ACK_LEAVE_GUILD PACKET_ZC_ACK_LEAVE_GUILD2
#else
#define PACKET_ZC_ACK_LEAVE_GUILD PACKET_ZC_ACK_LEAVE_GUILD1
#endif

struct PACKET_ZC_ACK_LEAVE_GUILD1 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
} __attribute__((packed));

struct PACKET_ZC_ACK_LEAVE_GUILD2 {
	int16 packetType;
	uint32 GID;
	char reason[40];
} __attribute__((packed));

struct PACKET_CZ_MEMORIALDUNGEON_COMMAND {
	int16 packetType;
	int32 command;
} __attribute__((packed));

struct PACKET_ZC_REMOVE_EFFECT {
	int16 packetType;
	uint32 aid;
	uint32 effectId;
} __attribute__((packed));

struct PACKET_ZC_CAMERA_INFO {
	int16 packetType;
	int8 action;
	float range;
	float rotation;
	float latitude;
} __attribute__((packed));

#if PACKETVER_RE_NUM >= 20181031
#define PACKET_ZC_AUTOSPELLLIST PACKET_ZC_AUTOSPELLLIST2
#else
#define PACKET_ZC_AUTOSPELLLIST PACKET_ZC_AUTOSPELLLIST1
#endif

struct PACKET_ZC_AUTOSPELLLIST1 {
	int16 packetType;
	int skills[7];
} __attribute__((packed));

struct PACKET_ZC_AUTOSPELLLIST2 {
	int16 packetType;
	int16 packetLength;
	int skills[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
#if PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024
struct PACKET_ZC_ITEM_PREVIEW {
	int16 packetType;
	int16 index;
	int8 isDamaged;
	int16 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_PREVIEW, 0x0b13);
#else  // PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024

struct PACKET_ZC_ITEM_PREVIEW {
	int16 packetType;
	int16 index;
	int16 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_PREVIEW, 0x0ab9);
#endif  // PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024
#endif  // PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* MAP_PACKETS_STRUCT_H */
