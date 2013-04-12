// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

/* Hercules Renewal: Phase Two http://hercules.ws/board/topic/383-hercules-renewal-phase-two/ */

#ifndef _PACKETS_STRUCT_H_
#define _PACKETS_STRUCT_H_

enum packet_headers {
#if PACKETVER < 20080102
	authokType = 0x73,
#else
	authokType = 0x2eb,
#endif
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
