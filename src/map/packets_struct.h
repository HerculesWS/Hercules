/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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
#else
	additemType = 0xa0c,
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
#else
	authokType = 0xa18,
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
#if PACKETVER > 20130000 /* not sure date */
	dropflooritemType = 0x84b,
#else
	dropflooritemType = 0x9e,
#endif
#if PACKETVER >= 20120925
	inventorylistnormalType = 0x991,
#elif PACKETVER >= 20080102
	inventorylistnormalType = 0x2e8,
#elif PACKETVER >= 20071002
	inventorylistnormalType = 0x1ee,
#else
	inventorylistnormalType = 0xa3,
#endif
#if PACKETVER >= 20150226
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
#if PACKETVER >= 20120925
	storagelistnormalType = 0x995,
#elif PACKETVER >= 20080102
	storagelistnormalType = 0x2ea,
#elif PACKETVER >= 20071002
	storagelistnormalType = 0x295,
#else
	storagelistnormalType = 0xa5,
#endif
#if PACKETVER >= 20150226
	storagelistequipType = 0xa10,
#elif PACKETVER >= 20120925
	storagelistequipType = 0x996,
#elif PACKETVER >= 20080102
	storagelistequipType = 0x2d1,
#elif PACKETVER >= 20071002
	storagelistequipType = 0x296,
#else
	storagelistequipType = 0xa6,
#endif
#if PACKETVER >= 20120925
	cartlistnormalType = 0x993,
#elif PACKETVER >= 20080102
	cartlistnormalType = 0x2e9,
#elif PACKETVER >= 20071002
	cartlistnormalType = 0x1ef,
#else
	cartlistnormalType = 0x123,
#endif
#if PACKETVER >= 20150226
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
#if PACKETVER >= 20150226
	viewequipackType = 0xa2d,
#elif PACKETVER >= 20120925
	viewequipackType = 0x997,
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
#if PACKETVER >= 20131223
	wisendType = 0x9df,
#else
	wisendType = 0x98,
#endif
	partyleaderchangedType = 0x7fc,
	rouletteinfoackType = 0xa1c,
	roulettgenerateackType = 0xa20,
	roulettercvitemackType = 0xa22,
#if 0 // Unknown
	questListType = 0x9f8, ///< ZC_ALL_QUEST_LIST3
#elif PACKETVER >= 20141022
	questListType = 0x97a, ///< ZC_ALL_QUEST_LIST2
#else // PACKETVER < 20141022
	questListType = 0x2b1, ///< ZC_ALL_QUEST_LIST
#endif // PACKETVER >= 20141022
};

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * structs for data
 */
struct EQUIPSLOTINFO {
	unsigned short card[4];
} __attribute__((packed));

struct NORMALITEM_INFO {
	short index;
	unsigned short ITID;
	unsigned char type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
	short count;
#if PACKETVER >= 20120925
	unsigned int WearState;
#else
	unsigned short WearState;
#endif
#if PACKETVER >= 5
	struct EQUIPSLOTINFO slot;
#endif
#if PACKETVER >= 20080102
	int HireExpireDate;
#endif
#if PACKETVER >= 20120925
	struct {
		unsigned char IsIdentified : 1;
		unsigned char PlaceETCTab : 1;
		unsigned char SpareBits : 6;
	} Flag;
#endif
} __attribute__((packed));

struct RndOptions {
	short index;
	short value;
	unsigned char param;
} __attribute__((packed));

struct EQUIPITEM_INFO {
	short index;
	unsigned short ITID;
	unsigned char type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
#if PACKETVER >= 20120925
	unsigned int location;
	unsigned int WearState;
#else
	unsigned short location;
	unsigned short WearState;
#endif
#if PACKETVER < 20120925
	uint8 IsDamaged;
#endif
	unsigned char RefiningLevel;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20071002
	int HireExpireDate;
#endif
#if PACKETVER >= 20080102
	unsigned short bindOnEquipType;
#endif
#if PACKETVER >= 20100629
	unsigned short wItemSpriteNumber;
#endif
#if PACKETVER >= 20150226
	unsigned char option_count;
	struct RndOptions option_data[5];
#endif
#if PACKETVER >= 20120925
	struct {
		unsigned char IsIdentified : 1;
		unsigned char IsDamaged : 1;
		unsigned char PlaceETCTab : 1;
		unsigned char SpareBits : 5;
	} Flag;
#endif
} __attribute__((packed));

struct packet_authok {
	short PacketType;
	unsigned int startTime;
	unsigned char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20141022
	unsigned char sex;
#endif
} __attribute__((packed));

struct packet_monster_hp {
	short PacketType;
	unsigned int GID;
	int HP;
	int MaxHP;
} __attribute__((packed));

struct packet_sc_notick {
	short PacketType;
	short index;
	unsigned int AID;
	unsigned char state;
} __attribute__((packed));

struct packet_additem {
	short PacketType;
	unsigned short Index;
	unsigned short count;
	unsigned short nameid;
	uint8 IsIdentified;
	uint8 IsDamaged;
	unsigned char refiningLevel;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20120925
	unsigned int location;
#else
	unsigned short location;
#endif
	unsigned char type;
	unsigned char result;
#if PACKETVER >= 20061218
	int HireExpireDate;
#endif
#if PACKETVER >= 20071002
	unsigned short bindOnEquipType;
#endif
#if PACKETVER >= 20150226
	struct RndOptions option_data[5];
#endif
} __attribute__((packed));

struct packet_dropflooritem {
	short PacketType;
	unsigned int ITAID;
	unsigned short ITID;
#if PACKETVER >= 20130000 /* not sure date */
	unsigned short type;
#endif
	uint8 IsIdentified;
	short xPos;
	short yPos;
	unsigned char subX;
	unsigned char subY;
	short count;
} __attribute__((packed));
struct packet_idle_unit2 {
#if PACKETVER < 20091103
	short PacketType;
#if PACKETVER >= 20071106
	unsigned char objecttype;
#endif
	unsigned int GID;
	short speed;
	short bodyState;
	short healthState;
	short effectState;
	short job;
	short head;
	short weapon;
	short accessory;
	short shield;
	short accessory2;
	short accessory3;
	short headpalette;
	short bodypalette;
	short headDir;
	unsigned int GUID;
	short GEmblemVer;
	short honor;
	short virtue;
	uint8 isPKModeON;
	unsigned char sex;
	unsigned char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
	unsigned char state;
	short clevel;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit2 {
#if PACKETVER < 20091103
	short PacketType;
#if PACKETVER >= 20071106
	unsigned char objecttype;
#endif
	unsigned int GID;
	short speed;
	short bodyState;
	short healthState;
	short effectState;
	short head;
	short weapon;
	short accessory;
	short job;
	short shield;
	short accessory2;
	short accessory3;
	short headpalette;
	short bodypalette;
	short headDir;
	uint8 isPKModeON;
	unsigned char sex;
	unsigned char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
	unsigned char objecttype;
#endif
#if PACKETVER >= 20131223
	unsigned int AID;
#endif
	unsigned int GID;
	short speed;
	short bodyState;
	short healthState;
#if PACKETVER < 20080102
	short effectState;
#else
	int effectState;
#endif
	short job;
	short head;
#if PACKETVER < 7
	short weapon;
#else
	int weapon;
#endif
	short accessory;
#if PACKETVER < 7
	short shield;
#endif
	short accessory2;
	short accessory3;
	short headpalette;
	short bodypalette;
	short headDir;
#if PACKETVER >= 20101124
	short robe;
#endif
	unsigned int GUID;
	short GEmblemVer;
	short honor;
#if PACKETVER > 7
	int virtue;
#else
	short virtue;
#endif
	uint8 isPKModeON;
	unsigned char sex;
	unsigned char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
	short body;
#endif
} __attribute__((packed));

struct packet_unit_walking {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
#endif
#if PACKETVER > 20071106
	unsigned char objecttype;
#endif
#if PACKETVER >= 20131223
	unsigned int AID;
#endif
	unsigned int GID;
	short speed;
	short bodyState;
	short healthState;
#if PACKETVER < 7
	short effectState;
#else
	int effectState;
#endif
	short job;
	short head;
#if PACKETVER < 7
	short weapon;
#else
	int weapon;
#endif
	short accessory;
	unsigned int moveStartTime;
#if PACKETVER < 7
	short shield;
#endif
	short accessory2;
	short accessory3;
	short headpalette;
	short bodypalette;
	short headDir;
#if PACKETVER >= 20101124
	short robe;
#endif
	unsigned int GUID;
	short GEmblemVer;
	short honor;
#if PACKETVER > 7
	int virtue;
#else
	short virtue;
#endif
	uint8 isPKModeON;
	unsigned char sex;
	unsigned char MoveData[6];
	unsigned char xSize;
	unsigned char ySize;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
	short body;
#endif
} __attribute__((packed));

struct packet_idle_unit {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
	unsigned char objecttype;
#endif
#if PACKETVER >= 20131223
	unsigned int AID;
#endif
	unsigned int GID;
	short speed;
	short bodyState;
	short healthState;
#if PACKETVER < 20080102
	short effectState;
#else
	int effectState;
#endif
	short job;
	short head;
#if PACKETVER < 7
	short weapon;
#else
	int weapon;
#endif
	short accessory;
#if PACKETVER < 7
	short shield;
#endif
	short accessory2;
	short accessory3;
	short headpalette;
	short bodypalette;
	short headDir;
#if PACKETVER >= 20101124
	short robe;
#endif
	unsigned int GUID;
	short GEmblemVer;
	short honor;
#if PACKETVER > 7
	int virtue;
#else
	short virtue;
#endif
	uint8 isPKModeON;
	unsigned char sex;
	unsigned char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
	unsigned char state;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
#endif
#if PACKETVER >= 20150513
	short body;
#endif
} __attribute__((packed));

struct packet_status_change {
	short PacketType;
	short index;
	unsigned int AID;
	unsigned char state;
#if PACKETVER >= 20120618
	unsigned int Total;
#endif
#if PACKETVER >= 20090121
	unsigned int Left;
	int val1;
	int val2;
	int val3;
#endif
} __attribute__((packed));

struct packet_status_change_end {
	short PacketType;
	short index;
	unsigned int AID;
	unsigned char state;
} __attribute__((packed));

struct packet_status_change2 {
	short PacketType;
	short index;
	unsigned int AID;
	unsigned char state;
	unsigned int Left;
	int val1;
	int val2;
	int val3;
} __attribute__((packed));

struct packet_maptypeproperty2 {
	short PacketType;
	short type;
	struct {
		unsigned int party             : 1;  // Show attack cursor on non-party members (PvP)
		unsigned int guild             : 1;  // Show attack cursor on non-guild members (GvG)
		unsigned int siege             : 1;  // Show emblem over characters' heads when in GvG (WoE castle)
		unsigned int mineffect         : 1;  // Automatically enable /mineffect
		unsigned int nolockon          : 1;  // TODO: What does this do? (shows attack cursor on non-party members)
		unsigned int countpk           : 1;  /// Show the PvP counter
		unsigned int nopartyformation  : 1;  /// Prevent party creation/modification
		unsigned int bg                : 1;  // TODO: What does this do? Probably related to Battlegrounds, but I'm not sure on the effect
		unsigned int nocostume         : 1;  /// Does not show costume sprite.
		unsigned int usecart           : 1;  /// Allow opening cart inventory
		unsigned int summonstarmiracle : 1;  // TODO: What does this do? Related to Taekwon Masters, but I have no idea.
		unsigned int SpareBits         : 15; /// Currently ignored, reserved for future updates
	} flag;
} __attribute__((packed));

struct packet_bgqueue_ack {
	short PacketType;
	unsigned char type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notice_delete {
	short PacketType;
	unsigned char type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_register {
	short PacketType;
	short type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_update_info {
	short PacketType;
	char bg_name[NAME_LENGTH];
	int position;
} __attribute__((packed));

struct packet_bgqueue_checkstate {
	short PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_revoke_req {
	short PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_battlebegin_ack {
	short PacketType;
	unsigned char result;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notify_entry {
	short PacketType;
	char name[NAME_LENGTH];
	int position;
} __attribute__((packed));

struct packet_bgqueue_battlebegins {
	short PacketType;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_script_clear {
	short PacketType;
	unsigned int NpcID;
} __attribute__((packed));

/* made possible thanks to Yommy!! */
struct packet_package_item_announce {
	short PacketType;
	short PacketLength;
	unsigned char type;
	unsigned short ItemID;
	char len;
	char Name[NAME_LENGTH];
	char unknown;
	unsigned short BoxItemID;
} __attribute__((packed));

/* made possible thanks to Yommy!! */
struct packet_item_drop_announce {
	short PacketType;
	short PacketLength;
	unsigned char type;
	unsigned short ItemID;
	char len;
	char Name[NAME_LENGTH];
	char monsterNameLen;
	char monsterName[NAME_LENGTH];
} __attribute__((packed));

struct packet_cart_additem_ack {
	short PacketType;
	char result;
} __attribute__((packed));

struct packet_banking_check {
	short PacketType;
	int64 Money;
	short Reason;
} __attribute__((packed));

struct packet_banking_deposit_req {
	short PacketType;
	unsigned int AID;
	int Money;
} __attribute__((packed));

struct packet_banking_withdraw_req {
	short PacketType;
	unsigned int AID;
	int Money;
} __attribute__((packed));

struct packet_banking_deposit_ack {
	short PacketType;
	short Reason;
	int64 Money;
	int Balance;
} __attribute__((packed));

struct packet_banking_withdraw_ack {
	short PacketType;
	short Reason;
	int64 Money;
	int Balance;
} __attribute__((packed));

/* Roulette System [Yommy/Hercules] */
struct packet_roulette_open_ack {
	short PacketType;
	char Result;
	int Serial;
	char Step;
	char Idx;
	short AdditionItemID;
	int GoldPoint;
	int SilverPoint;
	int BronzePoint;
} __attribute__((packed));

struct packet_roulette_info_ack {
	short PacketType;
	short PacketLength;
	unsigned int RouletteSerial;
	struct {
		unsigned short Row;
		unsigned short Position;
		unsigned short ItemId;
		unsigned short Count;
	} ItemInfo[42];
} __attribute__((packed));

struct packet_roulette_close_ack {
	short PacketType;
	unsigned char Result;
} __attribute__((packed));

struct packet_roulette_generate_ack {
	short PacketType;
	unsigned char Result;
	unsigned short Step;
	unsigned short Idx;
	unsigned short AdditionItemID;
	int RemainGold;
	int RemainSilver;
	int RemainBronze;
} __attribute__((packed));

struct packet_roulette_itemrecv_req {
	short PacketType;
	unsigned char Condition;
} __attribute__((packed));

struct packet_roulette_itemrecv_ack {
	short PacketType;
	unsigned char Result;
	unsigned short AdditionItemID;
} __attribute__((packed));

struct packet_itemlist_normal {
	short PacketType;
	short PacketLength;
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_itemlist_equip {
	short PacketType;
	short PacketLength;
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_storelist_normal {
	short PacketType;
	short PacketLength;
#if PACKETVER >= 20120925
	char name[NAME_LENGTH];
#endif
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_storelist_equip {
	short PacketType;
	short PacketLength;
#if PACKETVER >= 20120925
	char name[NAME_LENGTH];
#endif
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_equip_item {
	short PacketType;
	unsigned short index;
#if PACKETVER >= 20120925
	unsigned int wearLocation;
#else
	unsigned short wearLocation;
#endif
} __attribute__((packed));

struct packet_equipitem_ack {
	short PacketType;
	unsigned short index;
#if PACKETVER >= 20120925
	unsigned int wearLocation;
#else
	unsigned short wearLocation;
#endif
#if PACKETVER >= 20100629
	unsigned short wItemSpriteNumber;
#endif
	unsigned char result;
} __attribute__((packed));

struct packet_unequipitem_ack {
	short PacketType;
	unsigned short index;
#if PACKETVER >= 20120925
	unsigned int wearLocation;
#else
	unsigned short wearLocation;
#endif
	unsigned char result;
} __attribute__((packed));

struct packet_viewequip_ack {
	short PacketType;
	short PacketLength;
	char characterName[NAME_LENGTH];
	short job;
	short head;
	short accessory;
	short accessory2;
	short accessory3;
#if PACKETVER >= 20101124
	short robe;
#endif
	short headpalette;
	short bodypalette;
	unsigned char sex;
	struct EQUIPITEM_INFO list[MAX_INVENTORY];
} __attribute__((packed));

struct packet_notify_bounditem {
	short PacketType;
	unsigned short index;
} __attribute__((packed));

struct packet_skill_entry {
	short PacketType;
#if PACKETVER >= 20110718
	short PacketLength;
#endif
	unsigned int AID;
	unsigned int creatorAID;
	short xPos;
	short yPos;
#if PACKETVER >= 20121212
	int job;
#else
	unsigned char job;
#endif
#if PACKETVER >= 20110718
	char RadiusRange;
#endif
	unsigned char isVisible;
#if PACKETVER >= 20130731
	unsigned char level;
#endif
} __attribute__((packed));

struct packet_graffiti_entry {
	short PacketType;
	unsigned int AID;
	unsigned int creatorAID;
	short xPos;
	short yPos;
	unsigned char job;
	unsigned char isVisible;
	unsigned char isContens;
	char msg[80];
} __attribute__((packed));

struct packet_damage {
	short PacketType;
	unsigned int GID;
	unsigned int targetGID;
	unsigned int startTime;
	int attackMT;
	int attackedMT;
#if PACKETVER < 20071113
	short damage;
#else
	int damage;
#endif
#if PACKETVER >= 20131223
	unsigned char is_sp_damaged;
#endif
	short count;
	unsigned char action;
#if PACKETVER < 20071113
	short leftDamage;
#else
	int leftDamage;
#endif
} __attribute__((packed));

struct packet_gm_monster_item {
	short PacketType;
#if PACKETVER >= 20131218
	char str[100];
#else
	char str[24];
#endif
} __attribute__((packed));

struct packet_npc_market_purchase {
	short PacketType;
	short PacketLength;
	struct {
		unsigned short ITID;
		int qty;
	} list[MAX_INVENTORY];/* assuming MAX_INVENTORY is max since you can't hold more than MAX_INVENTORY items thus cant buy that many at once. */
	// TODO[Haru]: Change to a flexible array
} __attribute__((packed));

struct packet_npc_market_result_ack {
	short PacketType;
	short PacketLength;
	unsigned char result;
	struct {
		unsigned short ITID;
		unsigned short qty;
		unsigned int price;
	} list[MAX_INVENTORY];/* assuming MAX_INVENTORY is max since you can't hold more than MAX_INVENTORY items thus cant buy that many at once. */
} __attribute__((packed));

struct packet_npc_market_open {
	short PacketType;
	short PacketLength;
	/* inner struct figured by Ind after some annoying hour of debugging (data Thanks to Yommy) */
	struct {
		unsigned short nameid;
		unsigned char type;
		unsigned int price;
		unsigned int qty;
		unsigned short view;
	// It seems that the client doesn't have any hard-coded limit for this list
	// it's possible to send up to 1890 items without dropping a packet that's
	// too large [Panikon]
	} list[1000];/* TODO: whats the actual max of this? */
} __attribute__((packed));

struct packet_wis_end {
	short PacketType;
	char result;
#if PACKETVER >= 20131223
	unsigned int unknown;/* maybe AID, not sure what for (works sending as 0) */
#endif
} __attribute__((packed));


struct packet_party_leader_changed {
	short PacketType;
	unsigned int prev_leader_aid;
	unsigned int new_leader_aid;
} __attribute__((packed));

struct packet_hotkey {
#ifdef HOTKEY_SAVING
	short PacketType;
#if PACKETVER >= 20141022
	char Rotate;
#endif
	struct {
		char isSkill;    // 0: Item, 1:Skill
		unsigned int ID; // Item/Skill ID
		short count;     // Item Quantity/Skill Level
	} hotkey[MAX_HOTKEYS];
#else // not HOTKEY_SAVING
	UNAVAILABLE_STRUCT;
#endif // HOTKEY_SAVING
} __attribute__((packed));

/**
 * MISSION_HUNT_INFO
 */
struct packet_mission_info_sub {
	int32 mob_id;
	int16 huntCount;
	int16 maxCount;
	char mobName[NAME_LENGTH];
} __attribute__((packed));

/**
 * PACKET_ZC_ALL_QUEST_LIST2_INFO (PACKETVER >= 20141022)
 * PACKET_ZC_ALL_QUEST_LIST3_INFO (PACKETVER Unknown) / unused
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
 * PACKET_ZC_ALL_QUEST_LIST3 (PACKETVER Unknown) / unused
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

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* MAP_PACKETS_STRUCT_H */
