// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

/* Hercules Renewal: Phase Two http://hercules.ws/board/topic/383-hercules-renewal-phase-two/ */

#ifndef _PACKETS_STRUCT_H_
#define _PACKETS_STRUCT_H_

#include "../common/mmo.h"

/**
 * structs for data
 */
struct EQUIPSLOTINFO {
	unsigned short card[4];
};
/**
 *
 **/
enum packet_headers {
	sc_notickType = 0x196,
#if PACKETVER < 20061218
	additemType = 0xa0,
#elif PACKETVER < 20071002
	additemType = 0x29a,
#elif PACKETVER < 20120925
	additemType = 0x2d4,
#else
	additemType = 0x990,
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
#elif PACKETVER < 20140000 //actual 20120221
	idle_unitType = 0x857,
#else
	idle_unitType = 0x915,
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
#elif PACKETVER < 20140000 //actual 20120221
	spawn_unitType = 0x858,
#else
	spawn_unitType = 0x90f,
#endif
#if PACKETVER < 20080102
	authokType = 0x73,
#else
	authokType = 0x2eb,
#endif
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
#elif PACKETVER < 20140000 //actual 20120221
	unit_walkingType = 0x856,
#else
	unit_walkingType = 0x914,
#endif
#if PACKETVER > 20130000 /* not sure date */
	dropflooritemType = 0x84b,
#else
	dropflooritemType = 0x9e,
#endif
	monsterhpType = 0x977,
	maptypeproperty2Type = 0x99b,
};

#pragma pack(push, 1)

struct packet_authok {
	short PacketType;
	unsigned int startTime;
	char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
#if PACKETVER >= 20080102
	short font;
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
	bool IsIdentified;
	bool IsDamaged;
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
} __attribute__((packed));

struct packet_dropflooritem {
	short PacketType;
	unsigned int ITAID;
	unsigned short ITID;
#if PACKETVER >= 20130000 /* not sure date */
	unsigned short type;
#endif
	bool IsIdentified;
	short xPos;
	short yPos;
	unsigned char subX;
	unsigned char subY;
	short count;
} __attribute__((packed));

struct packet_spawn_unit {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
	unsigned char objecttype;
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
#if PACKETVER >= 20091103
	int virtue;
#else
	short virtue;
#endif
	bool isPKModeON;
	unsigned char sex;
	char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20140000 //actual 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
#endif
} __attribute__((packed));

struct packet_unit_walking {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
	unsigned char objecttype;
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
#if PACKETVER >= 20091103
	int virtue;
#else
	short virtue;
#endif
	bool isPKModeON;
	unsigned char sex;
	char MoveData[6];
	unsigned char xSize;
	unsigned char ySize;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20140000 //actual 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
#endif
} __attribute__((packed));

struct packet_idle_unit {
	short PacketType;
#if PACKETVER >= 20091103
	short PacketLength;
	unsigned char objecttype;
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
#if PACKETVER >= 20091103
	int virtue;
#else
	short virtue;
#endif
	bool isPKModeON;
	unsigned char sex;
	char PosDir[3];
	unsigned char xSize;
	unsigned char ySize;
	unsigned char state;
	short clevel;
#if PACKETVER >= 20080102
	short font;
#endif
#if PACKETVER >= 20140000 //actual 20120221
	int maxHP;
	int HP;
	unsigned char isBoss;
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
		unsigned int party					: 1;
		unsigned int guild					: 1;
		unsigned int siege					: 1;
		unsigned int mineffect				: 1;
		unsigned int nolockon				: 1;
		unsigned int countpk				: 1;
		unsigned int nopartyformation		: 1;
		unsigned int bg						: 1;
		unsigned int noitemconsumption		: 1;
		unsigned int usecart				: 1;
		unsigned int summonstarmiracle		: 1;
		unsigned int SpareBits				: 15;
	} flag;
} __attribute__((packed));

#pragma pack(pop)

#endif /* _PACKETS_STRUCT_H_ */
