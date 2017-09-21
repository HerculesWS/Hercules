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

//Included directly by clif.c in packet_loaddb()

#ifndef MAP_PACKETS_H
#define MAP_PACKETS_H

#ifndef packet
	#define packet(a,b,...)
#endif

/*
 * packet syntax
 * - packet(packet_id,length)
 * OR
 * - packet(packet_id,length,function,offset ( specifies the offset of a packet field in bytes from the begin of the packet ),...)
 * - Example: packet(0x0072,19,clif->pWantToConnection,2,6,10,14,18);
 */

packet(0x0064,55);
packet(0x0065,17);
packet(0x0066,6);
packet(0x0067,37);
packet(0x0068,46);
packet(0x0069,-1);
packet(0x006a,23);
packet(0x006b,-1);
packet(0x006c,3);
packet(0x006d,108);
packet(0x006e,3);
packet(0x006f,2);
packet(0x0070,6);
packet(0x0071,28);
packet(0x0072,19,clif->pWantToConnection,2,6,10,14,18);
packet(0x0073,11);
packet(0x0074,3);
packet(0x0075,-1);
packet(0x0076,9);
packet(0x0077,5);
packet(0x0078,54);
packet(0x0079,53);
packet(0x007a,58);
packet(0x007b,60);
packet(0x007c,41);
packet(0x007d,2,clif->pLoadEndAck,0);
packet(0x007e,6,clif->pTickSend,2);
packet(0x007f,6);
packet(0x0080,7);
packet(0x0081,3);
packet(0x0082,2);
packet(0x0083,2);
packet(0x0084,2);
packet(0x0085,5,clif->pWalkToXY,2);
packet(0x0086,16);
packet(0x0087,12);
packet(0x0088,10);
packet(0x0089,7,clif->pActionRequest,2,6);
packet(0x008a,29);
packet(0x008b,2);
packet(0x008c,-1,clif->pGlobalMessage,2,4);
packet(0x008d,-1);
packet(0x008e,-1);
//packet(0x008f,-1);
packet(0x0090,7,clif->pNpcClicked,2);
packet(0x0091,22);
packet(0x0092,28);
packet(0x0093,2);
packet(0x0094,6,clif->pGetCharNameRequest,2);
packet(0x0095,30);
packet(0x0096,-1,clif->pWisMessage,2,4,28);
packet(0x0097,-1);
packet(0x0098,3);
packet(0x0099,-1,clif->pBroadcast,2,4);
packet(0x009a,-1);
packet(0x009b,5,clif->pChangeDir,2,4);
packet(0x009c,9);
packet(0x009d,17);
packet(0x009e,17);
packet(0x009f,6,clif->pTakeItem,2);
packet(0x00a0,23);
packet(0x00a1,6);
packet(0x00a2,6,clif->pDropItem,2,4);
packet(0x00a3,-1);
packet(0x00a4,-1);
packet(0x00a5,-1);
packet(0x00a6,-1);
packet(0x00a7,8,clif->pUseItem,2,4);
packet(0x00a8,7);
packet(0x00a9,6,clif->pEquipItem,2,4);
packet(0x00aa,7);
packet(0x00ab,4,clif->pUnequipItem,2);
packet(0x00ac,7);
//packet(0x00ad,-1);
packet(0x00ae,-1);
packet(0x00af,6);
packet(0x00b0,8);
packet(0x00b1,8);
packet(0x00b2,3,clif->pRestart,2);
packet(0x00b3,3);
packet(0x00b4,-1);
packet(0x00b5,6);
packet(0x00b6,6);
packet(0x00b7,-1);
packet(0x00b8,7,clif->pNpcSelectMenu,2,6);
packet(0x00b9,6,clif->pNpcNextClicked,2);
packet(0x00ba,2);
packet(0x00bb,5,clif->pStatusUp,2,4);
packet(0x00bc,6);
packet(0x00bd,44);
packet(0x00be,5);
packet(0x00bf,3,clif->pEmotion,2);
packet(0x00c0,7);
packet(0x00c1,2,clif->pHowManyConnections,0);
packet(0x00c2,6);
packet(0x00c3,8);
packet(0x00c4,6);
packet(0x00c5,7,clif->pNpcBuySellSelected,2,6);
packet(0x00c6,-1);
packet(0x00c7,-1);
packet(0x00c8,-1,clif->pNpcBuyListSend,2,4);
packet(0x00c9,-1,clif->pNpcSellListSend,2,4);
packet(0x00ca,3);
packet(0x00cb,3);
packet(0x00cc,6,clif->pGMKick,2);
packet(0x00cd,3);
packet(0x00ce,2,clif->pGMKickAll,0);
packet(0x00cf,27,clif->pPMIgnore,2,26);
packet(0x00d0,3,clif->pPMIgnoreAll,2);
packet(0x00d1,4);
packet(0x00d2,4);
packet(0x00d3,2,clif->pPMIgnoreList,0);
packet(0x00d4,-1);
packet(0x00d5,-1,clif->pCreateChatRoom,2,4,6,7,15);
packet(0x00d6,3);
packet(0x00d7,-1);
packet(0x00d8,6);
packet(0x00d9,14,clif->pChatAddMember,2,6);
packet(0x00da,3);
packet(0x00db,-1);
packet(0x00dc,28);
packet(0x00dd,29);
packet(0x00de,-1,clif->pChatRoomStatusChange,2,4,6,7,15);
packet(0x00df,-1);
packet(0x00e0,30,clif->pChangeChatOwner,2,6);
packet(0x00e1,30);
packet(0x00e2,26,clif->pKickFromChat,2);
packet(0x00e3,2,clif->pChatLeave,0);
packet(0x00e4,6,clif->pTradeRequest,2);
packet(0x00e5,26);
packet(0x00e6,3,clif->pTradeAck,2);
packet(0x00e7,3);
packet(0x00e8,8,clif->pTradeAddItem,2,4);
packet(0x00e9,19);
packet(0x00ea,5);
packet(0x00eb,2,clif->pTradeOk,0);
packet(0x00ec,3);
packet(0x00ed,2,clif->pTradeCancel,0);
packet(0x00ee,2);
packet(0x00ef,2,clif->pTradeCommit,0);
packet(0x00f0,3);
packet(0x00f1,2);
packet(0x00f2,6);
packet(0x00f3,8,clif->pMoveToKafra,2,4);
packet(0x00f4,21);
packet(0x00f5,8,clif->pMoveFromKafra,2,4);
packet(0x00f6,8);
packet(0x00f7,2,clif->pCloseKafra,0);
packet(0x00f8,2);
packet(0x00f9,26,clif->pCreateParty,2);
packet(0x00fa,3);
packet(0x00fb,-1);
packet(0x00fc,6,clif->pPartyInvite,2);
packet(0x00fd,27);
packet(0x00fe,30);
packet(0x00ff,10,clif->pReplyPartyInvite,2,6);
packet(0x0100,2,clif->pLeaveParty,0);
packet(0x0101,6);
packet(0x0102,6,clif->pPartyChangeOption,2);
packet(0x0103,30,clif->pRemovePartyMember,2,6);
packet(0x0104,79);
packet(0x0105,31);
packet(0x0106,10);
packet(0x0107,10);
packet(0x0108,-1,clif->pPartyMessage,2,4);
packet(0x0109,-1);
packet(0x010a,4);
packet(0x010b,6);
packet(0x010c,6);
packet(0x010d,2);
packet(0x010e,11);
packet(0x010f,-1);
packet(0x0110,10);
packet(0x0111,39);
packet(0x0112,4,clif->pSkillUp,2);
packet(0x0113,10,clif->pUseSkillToId,2,4,6);
packet(0x0114,31);
packet(0x0115,35);
packet(0x0116,10,clif->pUseSkillToPos,2,4,6,8);
packet(0x0117,18);
packet(0x0118,2,clif->pStopAttack,0);
packet(0x0119,13);
packet(0x011a,15);
packet(0x011b,20,clif->pUseSkillMap,2,4);
packet(0x011c,68);
packet(0x011d,2,clif->pRequestMemo,0);
packet(0x011e,3);
packet(0x011f,16);
packet(0x0120,6);
packet(0x0121,14);
packet(0x0122,-1);
packet(0x0123,-1);
packet(0x0124,21);
packet(0x0125,8);
packet(0x0126,8,clif->pPutItemToCart,2,4);
packet(0x0127,8,clif->pGetItemFromCart,2,4);
packet(0x0128,8,clif->pMoveFromKafraToCart,2,4);
packet(0x0129,8,clif->pMoveToKafraFromCart,2,4);
packet(0x012a,2,clif->pRemoveOption,0);
packet(0x012b,2);
packet(0x012c,3);
packet(0x012d,4);
packet(0x012e,2,clif->pCloseVending,0);
packet(0x012f,-1);
packet(0x0130,6,clif->pVendingListReq,2);
packet(0x0131,86);
packet(0x0132,6);
packet(0x0133,-1);
packet(0x0134,-1,clif->pPurchaseReq,2,4,8);
packet(0x0135,7);
packet(0x0136,-1);
packet(0x0137,6);
packet(0x0138,3);
packet(0x0139,16);
packet(0x013a,4);
packet(0x013b,4);
packet(0x013c,4);
packet(0x013d,6);
packet(0x013e,24);
packet(0x013f,26,clif->pGM_Monster_Item,2);
packet(0x0140,22,clif->pMapMove,2,18,20);
packet(0x0141,14);
packet(0x0142,6);
packet(0x0143,10,clif->pNpcAmountInput,2,6);
packet(0x0144,23);
packet(0x0145,19);
packet(0x0146,6,clif->pNpcCloseClicked,2);
packet(0x0147,39);
packet(0x0148,8);
packet(0x0149,9,clif->pGMReqNoChat,2,6,7);
packet(0x014a,6);
packet(0x014b,27);
packet(0x014c,-1);
packet(0x014d,2,clif->pGuildCheckMaster,0);
packet(0x014e,6);
packet(0x014f,6,clif->pGuildRequestInfo,2);
packet(0x0150,110);
packet(0x0151,6,clif->pGuildRequestEmblem,2);
packet(0x0152,-1);
packet(0x0153,-1,clif->pGuildChangeEmblem,2,4);
packet(0x0154,-1);
packet(0x0155,-1,clif->pGuildChangeMemberPosition,2);
packet(0x0156,-1);
packet(0x0157,6);
packet(0x0158,-1);
packet(0x0159,54,clif->pGuildLeave,2,6,10,14);
packet(0x015a,66);
packet(0x015b,54,clif->pGuildExpulsion,2,6,10,14);
packet(0x015c,90);
packet(0x015d,42,clif->pGuildBreak,2);
packet(0x015e,6);
packet(0x015f,42);
packet(0x0160,-1);
packet(0x0161,-1,clif->pGuildChangePositionInfo,2);
packet(0x0162,-1);
packet(0x0163,-1);
packet(0x0164,-1);
packet(0x0165,30,clif->pCreateGuild,6);
packet(0x0166,-1);
packet(0x0167,3);
packet(0x0168,14,clif->pGuildInvite,2);
packet(0x0169,3);
packet(0x016a,30);
packet(0x016b,10,clif->pGuildReplyInvite,2,6);
packet(0x016c,43);
packet(0x016d,14);
packet(0x016e,186,clif->pGuildChangeNotice,2,6,66);
packet(0x016f,182);
packet(0x0170,14,clif->pGuildRequestAlliance,2);
packet(0x0171,30);
packet(0x0172,10,clif->pGuildReplyAlliance,2,6);
packet(0x0173,3);
packet(0x0174,-1);
packet(0x0175,6);
packet(0x0176,106);
packet(0x0177,-1);
packet(0x0178,4,clif->pItemIdentify,2);
packet(0x0179,5);
packet(0x017a,4,clif->pUseCard,2);
packet(0x017b,-1);
packet(0x017c,6,clif->pInsertCard,2,4);
packet(0x017d,7);
packet(0x017e,-1,clif->pGuildMessage,2,4);
packet(0x017f,-1);
packet(0x0180,6,clif->pGuildOpposition,2);
packet(0x0181,3);
packet(0x0182,106);
packet(0x0183,10,clif->pGuildDelAlliance,2,6);
packet(0x0184,10);
packet(0x0185,34);
//packet(0x0186,-1);
packet(0x0187,6);
packet(0x0188,8);
packet(0x0189,4);
packet(0x018a,4,clif->pQuitGame,0);
packet(0x018b,4);
packet(0x018c,29);
packet(0x018d,-1);
packet(0x018e,10,clif->pProduceMix,2,4,6,8);
packet(0x018f,6);
packet(0x0190,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
packet(0x0191,86);
packet(0x0192,24);
packet(0x0193,6,clif->pSolveCharName,2);
packet(0x0194,30);
packet(0x0195,102);
packet(0x0196,9);
packet(0x0197,4,clif->pResetChar,2);
packet(0x0198,8,clif->pGMChangeMapType,2,4,6);
packet(0x0199,4);
packet(0x019a,14);
packet(0x019b,10);
packet(0x019c,-1,clif->pLocalBroadcast,2,4);
packet(0x019d,6,clif->pGMHide,0);
packet(0x019e,2);
packet(0x019f,6,clif->pCatchPet,2);
packet(0x01a0,3);
packet(0x01a1,3,clif->pPetMenu,2);
packet(0x01a2,35);
packet(0x01a3,5);
packet(0x01a4,11);
packet(0x01a5,26,clif->pChangePetName,2);
packet(0x01a6,-1);
packet(0x01a7,4,clif->pSelectEgg,2);
packet(0x01a8,4);
packet(0x01a9,6,clif->pSendEmotion,2);
packet(0x01aa,10);
packet(0x01ab,12);
packet(0x01ac,6);
packet(0x01ad,-1);
packet(0x01ae,4,clif->pSelectArrow,2);
packet(0x01af,4,clif->pChangeCart,2);
packet(0x01b0,11);
packet(0x01b1,7);
packet(0x01b2,-1,clif->pOpenVending,2,4,84,85);
packet(0x01b3,67);
packet(0x01b4,12);
packet(0x01b5,18);
packet(0x01b6,114);
packet(0x01b7,6);
packet(0x01b8,3);
packet(0x01b9,6);
packet(0x01ba,26,clif->pGMShift,2);
packet(0x01bb,26,clif->pGMShift,2);
packet(0x01bc,26,clif->pGMRecall,2);
packet(0x01bd,26,clif->pGMRecall,2);
packet(0x01be,2);
packet(0x01bf,3);
packet(0x01c0,2);
packet(0x01c1,14);
packet(0x01c2,10);
packet(0x01c3,-1);
packet(0x01c4,22);
packet(0x01c5,22);
packet(0x01c6,4);
packet(0x01c7,2);
packet(0x01c8,13);
packet(0x01c9,97);
//packet(0x01ca,-1);
packet(0x01cb,9);
packet(0x01cc,9);
packet(0x01cd,30);
packet(0x01ce,6,clif->pAutoSpell,2);
packet(0x01cf,28);
packet(0x01d0,8);
packet(0x01d1,14);
packet(0x01d2,10);
packet(0x01d3,35);
packet(0x01d4,6);
packet(0x01d5,-1,clif->pNpcStringInput,2,4,8);
packet(0x01d6,4);
packet(0x01d7,11);
packet(0x01d8,54);
packet(0x01d9,53);
packet(0x01da,60);
packet(0x01db,2);
packet(0x01dc,-1);
packet(0x01dd,47);
packet(0x01de,33);
packet(0x01df,6,clif->pGMReqAccountName,2);
packet(0x01e0,30);
packet(0x01e1,8);
packet(0x01e2,34);
packet(0x01e3,14);
packet(0x01e4,2);
packet(0x01e5,6);
packet(0x01e6,26);
packet(0x01e7,2,clif->pNoviceDoriDori,0);
packet(0x01e8,28,clif->pCreateParty2,2);
packet(0x01e9,81);
packet(0x01ea,6);
packet(0x01eb,10);
packet(0x01ec,26);
packet(0x01ed,2,clif->pNoviceExplosionSpirits,0);
packet(0x01ee,-1);
packet(0x01ef,-1);
packet(0x01f0,-1);
packet(0x01f1,-1);
packet(0x01f2,20);
packet(0x01f3,10);
packet(0x01f4,32);
packet(0x01f5,9);
packet(0x01f6,34);
packet(0x01f7,14,clif->pAdopt_reply,0);
packet(0x01f8,2);
packet(0x01f9,6,clif->pAdopt_request,0);
packet(0x01fa,48);
packet(0x01fb,56);
packet(0x01fc,-1);
packet(0x01fd,4,clif->pRepairItem,2);
packet(0x01fe,5);
packet(0x01ff,10);
packet(0x0200,26);
packet(0x0201,-1);
packet(0x0202,26,clif->pFriendsListAdd,2);
packet(0x0203,10,clif->pFriendsListRemove,2,6);
packet(0x0204,18);
packet(0x0205,26);
packet(0x0206,11);
packet(0x0207,34);
packet(0x0208,11,clif->pFriendsListReply,2,6,10);
packet(0x0209,36);
packet(0x020a,10);
//packet(0x020b,-1);
//packet(0x020c,-1);
packet(0x020d,-1);
packet(0x974,2,clif->cancelmergeitem);
packet(0x96e,-1,clif->ackmergeitems);

//2004-07-05aSakexe
#if PACKETVER >= 20040705
	packet(0x0072,22,clif->pWantToConnection,5,9,13,17,21);
	packet(0x0085,8,clif->pWalkToXY,5);
	packet(0x00a7,13,clif->pUseItem,5,9);
	packet(0x0113,15,clif->pUseSkillToId,4,9,11);
	packet(0x0116,15,clif->pUseSkillToPos,4,9,11,13);
	packet(0x0190,95,clif->pUseSkillToPosMoreInfo,4,9,11,13,15);
	packet(0x0208,14,clif->pFriendsListReply,2,6,10);
	packet(0x020e,24);
#endif

//2004-07-13aSakexe
#if PACKETVER >= 20040713
	packet(0x0072,39,clif->pWantToConnection,12,22,30,34,38);
	packet(0x0085,9,clif->pWalkToXY,6);
	packet(0x009b,13,clif->pChangeDir,5,12);
	packet(0x009f,10,clif->pTakeItem,6);
	packet(0x00a7,17,clif->pUseItem,6,13);
	packet(0x0113,19,clif->pUseSkillToId,7,9,15);
	packet(0x0116,19,clif->pUseSkillToPos,7,9,15,17);
	packet(0x0190,99,clif->pUseSkillToPosMoreInfo,7,9,15,17,19);
#endif

//2004-07-26aSakexe
#if PACKETVER >= 20040726
	packet(0x0072,14,clif->pDropItem,5,12);
	packet(0x007e,33,clif->pWantToConnection,12,18,24,28,32);
	packet(0x0085,20,clif->pUseSkillToId,7,12,16);
	packet(0x0089,15,clif->pGetCharNameRequest,11);
	packet(0x008c,23,clif->pUseSkillToPos,3,6,17,21);
	packet(0x0094,10,clif->pTakeItem,6);
	packet(0x009b,6,clif->pWalkToXY,3);
	packet(0x009f,13,clif->pChangeDir,5,12);
	packet(0x00a2,103,clif->pUseSkillToPosMoreInfo,3,6,17,21,23);
	packet(0x00a7,12,clif->pSolveCharName,8);
	packet(0x00f3,-1,clif->pGlobalMessage,2,4);
	packet(0x00f5,17,clif->pUseItem,6,12);
	packet(0x00f7,10,clif->pTickSend,6);
	packet(0x0113,16,clif->pMoveToKafra,5,12);
	packet(0x0116,2,clif->pCloseKafra,0);
	packet(0x0190,26,clif->pMoveFromKafra,10,22);
	packet(0x0193,9,clif->pActionRequest,3,8);
#endif

//2004-08-09aSakexe
#if PACKETVER >= 20040809
	packet(0x0072,17,clif->pDropItem,8,15);
	packet(0x007e,37,clif->pWantToConnection,9,21,28,32,36);
	packet(0x0085,26,clif->pUseSkillToId,11,18,22);
	packet(0x0089,12,clif->pGetCharNameRequest,8);
	packet(0x008c,40,clif->pUseSkillToPos,5,15,29,38);
	packet(0x0094,13,clif->pTakeItem,9);
	packet(0x009b,15,clif->pWalkToXY,12);
	packet(0x009f,12,clif->pChangeDir,7,11);
	packet(0x00a2,120,clif->pUseSkillToPosMoreInfo,5,15,29,38,40);
	packet(0x00a7,11,clif->pSolveCharName,7);
	packet(0x00f5,24,clif->pUseItem,9,20);
	packet(0x00f7,13,clif->pTickSend,9);
	packet(0x0113,23,clif->pMoveToKafra,5,19);
	packet(0x0190,26,clif->pMoveFromKafra,11,22);
	packet(0x0193,18,clif->pActionRequest,7,17);
#endif

//2004-08-16aSakexe
#if PACKETVER >= 20040816
	packet(0x0212,26,clif->pGMRc,2);
	packet(0x0213,26,clif->pCheck,2);
	packet(0x0214,42);
#endif

//2004-08-17aSakexe
#if PACKETVER >= 20040817
	packet(0x020f,10,clif->pPVPInfo,2,6);
	packet(0x0210,22);
#endif

//2004-09-06aSakexe
#if PACKETVER >= 20040906
	packet(0x0072,20,clif->pUseItem,9,20);
	packet(0x007e,19,clif->pMoveToKafra,3,15);
	packet(0x0085,23,clif->pActionRequest,9,22);
	packet(0x0089,9,clif->pWalkToXY,6);
	packet(0x008c,105,clif->pUseSkillToPosMoreInfo,10,14,18,23,25);
	packet(0x0094,17,clif->pDropItem,6,15);
	packet(0x009b,14,clif->pGetCharNameRequest,10);
	packet(0x009f,-1,clif->pGlobalMessage,2,4);
	packet(0x00a2,14,clif->pSolveCharName,10);
	packet(0x00a7,25,clif->pUseSkillToPos,10,14,18,23);
	packet(0x00f3,10,clif->pChangeDir,4,9);
	packet(0x00f5,34,clif->pWantToConnection,7,15,25,29,33);
	packet(0x00f7,2,clif->pCloseKafra,0);
	packet(0x0113,11,clif->pTakeItem,7);
	packet(0x0116,11,clif->pTickSend,7);
	packet(0x0190,22,clif->pUseSkillToId,9,15,18);
	packet(0x0193,17,clif->pMoveFromKafra,3,13);
#endif

//2004-09-20aSakexe
#if PACKETVER >= 20040920
	packet(0x0072,18,clif->pUseItem,10,14);
	packet(0x007e,25,clif->pMoveToKafra,6,21);
	packet(0x0085,9,clif->pActionRequest,3,8);
	packet(0x0089,14,clif->pWalkToXY,11);
	packet(0x008c,109,clif->pUseSkillToPosMoreInfo,16,20,23,27,29);
	packet(0x0094,19,clif->pDropItem,12,17);
	packet(0x009b,10,clif->pGetCharNameRequest,6);
	packet(0x00a2,10,clif->pSolveCharName,6);
	packet(0x00a7,29,clif->pUseSkillToPos,6,20,23,27);
	packet(0x00f3,18,clif->pChangeDir,8,17);
	packet(0x00f5,32,clif->pWantToConnection,10,17,23,27,31);
	packet(0x0113,14,clif->pTakeItem,10);
	packet(0x0116,14,clif->pTickSend,10);
	packet(0x0190,14,clif->pUseSkillToId,4,7,10);
	packet(0x0193,12,clif->pMoveFromKafra,4,8);
#endif

//2004-10-05aSakexe
#if PACKETVER >= 20041005
	packet(0x0072,17,clif->pUseItem,6,13);
	packet(0x007e,16,clif->pMoveToKafra,5,12);
	packet(0x0089,6,clif->pWalkToXY,3);
	packet(0x008c,103,clif->pUseSkillToPosMoreInfo,2,6,17,21,23);
	packet(0x0094,14,clif->pDropItem,5,12);
	packet(0x009b,15,clif->pGetCharNameRequest,11);
	packet(0x00a2,12,clif->pSolveCharName,8);
	packet(0x00a7,23,clif->pUseSkillToPos,3,6,17,21);
	packet(0x00f3,13,clif->pChangeDir,5,12);
	packet(0x00f5,33,clif->pWantToConnection,12,18,24,28,32);
	packet(0x0113,10,clif->pTakeItem,6);
	packet(0x0116,10,clif->pTickSend,6);
	packet(0x0190,20,clif->pUseSkillToId,7,12,16);
	packet(0x0193,26,clif->pMoveFromKafra,10,22);
#endif

//2004-10-25aSakexe
#if PACKETVER >= 20041025
	packet(0x0072,13,clif->pUseItem,5,9);
	packet(0x007e,13,clif->pMoveToKafra,6,9);
	packet(0x0085,15,clif->pActionRequest,4,14);
	packet(0x008c,108,clif->pUseSkillToPosMoreInfo,6,9,23,26,28);
	packet(0x0094,12,clif->pDropItem,6,10);
	packet(0x009b,10,clif->pGetCharNameRequest,6);
	packet(0x00a2,16,clif->pSolveCharName,12);
	packet(0x00a7,28,clif->pUseSkillToPos,6,9,23,26);
	packet(0x00f3,15,clif->pChangeDir,6,14);
	packet(0x00f5,29,clif->pWantToConnection,5,14,20,24,28);
	packet(0x0113,9,clif->pTakeItem,5);
	packet(0x0116,9,clif->pTickSend,5);
	packet(0x0190,26,clif->pUseSkillToId,4,10,22);
	packet(0x0193,22,clif->pMoveFromKafra,12,18);
#endif

//2004-11-01aSakexe
#if PACKETVER >= 20041101
	packet(0x0084,-1);
	packet(0x0215,6);
#endif

//2004-11-08aSakexe
#if PACKETVER >= 20041108
	packet(0x0084,2);
	packet(0x0216,6);
	packet(0x0217,2,clif->pBlacksmith,0);
	packet(0x0218,2,clif->pAlchemist,0);
	packet(0x0219,282);
	packet(0x021a,282);
	packet(0x021b,10);
	packet(0x021c,10);
#endif

//2004-11-15aSakexe
#if PACKETVER >= 20041115
	packet(0x021d,6,clif->pLessEffect,2);
#endif

//2004-11-29aSakexe
#if PACKETVER >= 20041129
	packet(0x0072,22,clif->pUseSkillToId,8,12,18);
	packet(0x007e,30,clif->pUseSkillToPos,4,9,22,28);
	packet(0x0085,-1,clif->pGlobalMessage,2,4);
	packet(0x0089,7,clif->pTickSend,3);
	packet(0x008c,13,clif->pGetCharNameRequest,9);
	packet(0x0094,14,clif->pMoveToKafra,4,10);
	packet(0x009b,2,clif->pCloseKafra,0);
	packet(0x009f,18,clif->pActionRequest,6,17);
	packet(0x00a2,7,clif->pTakeItem,3);
	packet(0x00a7,7,clif->pWalkToXY,4);
	packet(0x00f3,8,clif->pChangeDir,3,7);
	packet(0x00f5,29,clif->pWantToConnection,3,10,20,24,28);
	packet(0x00f7,14,clif->pSolveCharName,10);
	packet(0x0113,110,clif->pUseSkillToPosMoreInfo,4,9,22,28,30);
	packet(0x0116,12,clif->pDropItem,4,10);
	packet(0x0190,15,clif->pUseItem,3,11);
	packet(0x0193,21,clif->pMoveFromKafra,4,17);
	packet(0x0221,-1);
	packet(0x0222,6,clif->pWeaponRefine,2);
	packet(0x0223,8);
#endif

//2004-12-13aSakexe
#if PACKETVER >= 20041213
//skipped: many packets being set to -1
	packet(0x0066,3);
	packet(0x0070,3);
	packet(0x01ca,3);
	packet(0x021e,6);
	packet(0x021f,66);
	packet(0x0220,10);
#endif

//2005-01-10bSakexe
#if PACKETVER >= 20050110
	packet(0x0072,26,clif->pUseSkillToId,8,16,22);
	packet(0x007e,114,clif->pUseSkillToPosMoreInfo,10,18,22,32,34);
	packet(0x0085,23,clif->pChangeDir,12,22);
	packet(0x0089,9,clif->pTickSend,5);
	packet(0x008c,8,clif->pGetCharNameRequest,4);
	packet(0x0094,20,clif->pMoveToKafra,10,16);
	packet(0x009b,32,clif->pWantToConnection,3,12,23,27,31);
	packet(0x009f,17,clif->pUseItem,5,13);
	packet(0x00a2,11,clif->pSolveCharName,7);
	packet(0x00a7,13,clif->pWalkToXY,10);
	packet(0x00f3,-1,clif->pGlobalMessage,2,4);
	packet(0x00f5,9,clif->pTakeItem,5);
	packet(0x00f7,21,clif->pMoveFromKafra,11,17);
	packet(0x0113,34,clif->pUseSkillToPos,10,18,22,32);
	packet(0x0116,20,clif->pDropItem,15,18);
	packet(0x0190,20,clif->pActionRequest,9,19);
	packet(0x0193,2,clif->pCloseKafra,0);
#endif

//2005-03-28aSakexe
#if PACKETVER >= 20050328
	packet(0x0224,10);
	packet(0x0225,2,clif->pTaekwon,0);
	packet(0x0226,282);
#endif

//2005-04-04aSakexe
#if PACKETVER >= 20050404
	packet(0x0227,18);
	packet(0x0228,18);
#endif

//2005-04-11aSakexe
#if PACKETVER >= 20050411
	packet(0x0229,15);
	packet(0x022a,58);
	packet(0x022b,57);
	packet(0x022c,64);
#endif

//2005-04-25aSakexe
#if PACKETVER >= 20050425
	packet(0x022d,5,clif->pHomMenu,2,4);
	packet(0x0232,9,clif->pHomMoveTo,2,6);
	packet(0x0233,11,clif->pHomAttack,2,6,10);
	packet(0x0234,6,clif->pHomMoveToMaster,2);
#endif

//2005-05-09aSakexe
#if PACKETVER >= 20050509
	packet(0x0072,25,clif->pUseSkillToId,6,10,21);
	packet(0x007e,102,clif->pUseSkillToPosMoreInfo,5,9,12,20,22);
	packet(0x0085,11,clif->pChangeDir,7,10);
	packet(0x0089,8,clif->pTickSend,4);
	packet(0x008c,11,clif->pGetCharNameRequest,7);
	packet(0x0094,14,clif->pMoveToKafra,7,10);
	packet(0x009b,26,clif->pWantToConnection,4,9,17,21,25);
	packet(0x009f,14,clif->pUseItem,4,10);
	packet(0x00a2,15,clif->pSolveCharName,11);
	packet(0x00a7,8,clif->pWalkToXY,5);
	packet(0x00f5,8,clif->pTakeItem,4);
	packet(0x00f7,22,clif->pMoveFromKafra,14,18);
	packet(0x0113,22,clif->pUseSkillToPos,5,9,12,20);
	packet(0x0116,10,clif->pDropItem,5,8);
	packet(0x0190,19,clif->pActionRequest,5,18);
#endif

//2005-05-23aSakexe
#if PACKETVER >= 20050523
	packet(0x022e,69);
	packet(0x0230,12);
#endif

//2005-05-30aSakexe
#if PACKETVER >= 20050530
	packet(0x022e,71);
	packet(0x0235,-1);
	packet(0x0236,10);
	packet(0x0237,2,clif->pRankingPk,0);
	packet(0x0238,282);
#endif

//2005-05-31aSakexe
#if PACKETVER >= 20050531
	packet(0x0216,2);
	packet(0x0239,11);
#endif

//2005-06-08aSakexe
#if PACKETVER >= 20050608
	packet(0x0216,6);
	packet(0x0217,2,clif->pBlacksmith,0);
	packet(0x022f,5);
	packet(0x0231,26,clif->pChangeHomunculusName,0);
	packet(0x023a,4);
	packet(0x023b,36,clif->pStoragePassword,2,4,20);
	packet(0x023c,6);
#endif

//2005-06-22aSakexe
#if PACKETVER >= 20050622
	packet(0x022e,71);

#endif

//2005-06-28aSakexe
#if PACKETVER >= 20050628
	packet(0x0072,34,clif->pUseSkillToId,6,17,30);
	packet(0x007e,113,clif->pUseSkillToPosMoreInfo,12,15,18,31,33);
	packet(0x0085,17,clif->pChangeDir,8,16);
	packet(0x0089,13,clif->pTickSend,9);
	packet(0x008c,8,clif->pGetCharNameRequest,4);
	packet(0x0094,31,clif->pMoveToKafra,16,27);
	packet(0x009b,32,clif->pWantToConnection,9,15,23,27,31);
	packet(0x009f,19,clif->pUseItem,9,15);
	packet(0x00a2,9,clif->pSolveCharName,5);
	packet(0x00a7,11,clif->pWalkToXY,8);
	packet(0x00f5,13,clif->pTakeItem,9);
	packet(0x00f7,18,clif->pMoveFromKafra,11,14);
	packet(0x0113,33,clif->pUseSkillToPos,12,15,18,31);
	packet(0x0116,12,clif->pDropItem,3,10);
	packet(0x0190,24,clif->pActionRequest,11,23);
	packet(0x0216,-1);
	packet(0x023d,-1);
	packet(0x023e,4);
#endif

//2005-07-18aSakexe
#if PACKETVER >= 20050718
	packet(0x0072,19,clif->pUseSkillToId,5,11,15);
	packet(0x007e,110,clif->pUseSkillToPosMoreInfo,9,15,23,28,30);
	packet(0x0085,11,clif->pChangeDir,6,10);
	packet(0x0089,7,clif->pTickSend,3);
	packet(0x008c,11,clif->pGetCharNameRequest,7);
	packet(0x0094,21,clif->pMoveToKafra,12,17);
	packet(0x009b,31,clif->pWantToConnection,3,13,22,26,30);
	packet(0x009f,12,clif->pUseItem,3,8);
	packet(0x00a2,18,clif->pSolveCharName,14);
	packet(0x00a7,15,clif->pWalkToXY,12);
	packet(0x00f5,7,clif->pTakeItem,3);
	packet(0x00f7,13,clif->pMoveFromKafra,5,9);
	packet(0x0113,30,clif->pUseSkillToPos,9,15,23,28);
	packet(0x0116,12,clif->pDropItem,6,10);
	packet(0x0190,21,clif->pActionRequest,5,20);
	packet(0x0216,6);
	packet(0x023f,2,clif->pMail_refreshinbox,0);
	packet(0x0240,8);
	packet(0x0241,6,clif->pMail_read,2);
	packet(0x0242,-1);
	packet(0x0243,6,clif->pMail_delete,2);
	packet(0x0244,6,clif->pMail_getattach,2);
	packet(0x0245,7);
	packet(0x0246,4,clif->pMail_winopen,2);
	packet(0x0247,8,clif->pMail_setattach,2,4);
	packet(0x0248,68);
	packet(0x0249,3);
	packet(0x024a,70);
	packet(0x024b,4,clif->pAuction_cancelreg,0);
	packet(0x024c,8,clif->pAuction_setitem,0);
	packet(0x024d,14);
	packet(0x024e,6,clif->pAuction_cancel,0);
	packet(0x024f,10,clif->pAuction_bid,0);
	packet(0x0250,3);
	packet(0x0251,2);
	packet(0x0252,-1);
#endif

//2005-07-19bSakexe
#if PACKETVER >= 20050719
	packet(0x0072,34,clif->pUseSkillToId,6,17,30);
	packet(0x007e,113,clif->pUseSkillToPosMoreInfo,12,15,18,31,33);
	packet(0x0085,17,clif->pChangeDir,8,16);
	packet(0x0089,13,clif->pTickSend,9);
	packet(0x008c,8,clif->pGetCharNameRequest,4);
	packet(0x0094,31,clif->pMoveToKafra,16,27);
	packet(0x009b,32,clif->pWantToConnection,9,15,23,27,31);
	packet(0x009f,19,clif->pUseItem,9,15);
	packet(0x00a2,9,clif->pSolveCharName,5);
	packet(0x00a7,11,clif->pWalkToXY,8);
	packet(0x00f5,13,clif->pTakeItem,9);
	packet(0x00f7,18,clif->pMoveFromKafra,11,14);
	packet(0x0113,33,clif->pUseSkillToPos,12,15,18,31);
	packet(0x0116,12,clif->pDropItem,3,10);
	packet(0x0190,24,clif->pActionRequest,11,23);
#endif

//2005-08-01aSakexe
#if PACKETVER >= 20050801
	packet(0x0245,3);
	packet(0x0251,4);
#endif

//2005-08-08aSakexe
#if PACKETVER >= 20050808
	packet(0x024d,12,clif->pAuction_register,0);
	packet(0x024e,4);
#endif

//2005-08-17aSakexe
#if PACKETVER >= 20050817
	packet(0x0253,3);
	packet(0x0254,3,clif->pFeelSaveOk,0);
#endif

//2005-08-29aSakexe
#if PACKETVER >= 20050829
	packet(0x0240,-1);
	packet(0x0248,-1,clif->pMail_send,2,4,28,68);
	packet(0x0255,5);
	packet(0x0256,-1);
	packet(0x0257,8);
#endif

//2005-09-12bSakexe
#if PACKETVER >= 20050912
	packet(0x0256,5);
	packet(0x0258,2);
	packet(0x0259,3);
#endif

//2005-10-10aSakexe
#if PACKETVER >= 20051010
	packet(0x020e,32);
	packet(0x025a,-1);
	packet(0x025b,6,clif->pCooking,0);
#endif

//2005-10-13aSakexe
#if PACKETVER >= 20051013
	packet(0x007a,6);
	packet(0x0251,32);
	packet(0x025c,4,clif->pAuction_buysell,0);
#endif

//2005-10-17aSakexe
#if PACKETVER >= 20051017
	packet(0x007a,58);
	packet(0x025d,6,clif->pAuction_close,0);
	packet(0x025e,4);
#endif

//2005-10-24aSakexe
#if PACKETVER >= 20051024
	packet(0x025f,6);
	packet(0x0260,6);
#endif

//2005-11-07aSakexe
#if PACKETVER >= 20051107
	packet(0x024e,6,clif->pAuction_cancel,0);
	packet(0x0251,34,clif->pAuction_search,0);
#endif

//2006-01-09aSakexe
#if PACKETVER >= 20060109
	packet(0x0261,11);
	packet(0x0262,11);
	packet(0x0263,11);
	packet(0x0264,20);
	packet(0x0265,20);
	packet(0x0266,30);
	packet(0x0267,4);
	packet(0x0268,4);
	packet(0x0269,4);
	packet(0x026a,4);
	packet(0x026b,4);
	packet(0x026c,4);
	packet(0x026d,4);
	packet(0x026f,2);
	packet(0x0270,2);
	packet(0x0271,38);
	packet(0x0272,44);
#endif

//2006-01-26aSakexe
#if PACKETVER >= 20060126
	packet(0x0271,40);

#endif

//2006-03-06aSakexe
#if PACKETVER >= 20060306
	packet(0x0273,6);
	packet(0x0274,8);
#endif

//2006-03-13aSakexe
#if PACKETVER >= 20060313
	packet(0x0273,30,clif->pMail_return,2,6);
#endif

//2006-03-27aSakexe
#if PACKETVER >= 20060327
	packet(0x0072,26,clif->pUseSkillToId,11,18,22);
	packet(0x007e,120,clif->pUseSkillToPosMoreInfo,5,15,29,38,40);
	packet(0x0085,12,clif->pChangeDir,7,11);
	//packet(0x0089,13,clif->pTickSend,9);
	packet(0x008c,12,clif->pGetCharNameRequest,8);
	packet(0x0094,23,clif->pMoveToKafra,5,19);
	packet(0x009b,37,clif->pWantToConnection,9,21,28,32,36);
	packet(0x009f,24,clif->pUseItem,9,20);
	packet(0x00a2,11,clif->pSolveCharName,7);
	packet(0x00a7,15,clif->pWalkToXY,12);
	packet(0x00f5,13,clif->pTakeItem,9);
	packet(0x00f7,26,clif->pMoveFromKafra,11,22);
	packet(0x0113,40,clif->pUseSkillToPos,5,15,29,38);
	packet(0x0116,17,clif->pDropItem,8,15);
	packet(0x0190,18,clif->pActionRequest,7,17);
#endif

//2006-10-23aSakexe
#if PACKETVER >= 20061023
	packet(0x006d,110);
#endif

//2006-04-24aSakexe to 2007-01-02aSakexe
#if PACKETVER >= 20060424
	packet(0x023e,8);
	packet(0x0277,84);
	packet(0x0278,2);
	packet(0x0279,2);
	packet(0x027a,-1);
	packet(0x027b,14);
	packet(0x027c,60);
	packet(0x027d,62);
	packet(0x027e,-1);
	packet(0x027f,8);
	packet(0x0280,12);
	packet(0x0281,4);
	packet(0x0282,284);
	packet(0x0283,6);
	packet(0x0284,14);
	packet(0x0285,6);
	packet(0x0286,4);
	packet(0x0287,-1);
	packet(0x0288,6);
	packet(0x0289,8);
	packet(0x028a,18);
	packet(0x028b,-1);
	packet(0x028c,46);
	packet(0x028d,34);
	packet(0x028e,4);
	packet(0x028f,6);
	packet(0x0290,4);
	packet(0x0291,4);
	packet(0x0292,2,clif->pAutoRevive,0);
	packet(0x0293,70);
	packet(0x0294,10);
	packet(0x0295,-1);
	packet(0x0296,-1);
	packet(0x0297,-1);
	packet(0x0298,8);
	packet(0x0299,6);
	packet(0x029a,27);
	packet(0x029c,66);
	packet(0x029d,-1);
	packet(0x029e,11);
	packet(0x029f,3,clif->pmercenary_action,0);
	packet(0x02a0,-1);
	packet(0x02a1,-1);
	packet(0x02a2,8);
#endif

//2007-01-08aSakexe
#if PACKETVER >= 20070108
	packet(0x0072,30,clif->pUseSkillToId,10,14,26);
	packet(0x007e,120,clif->pUseSkillToPosMoreInfo,10,19,23,38,40);
	packet(0x0085,14,clif->pChangeDir,10,13);
	packet(0x0089,11,clif->pTickSend,7);
	packet(0x008c,17,clif->pGetCharNameRequest,13);
	packet(0x0094,17,clif->pMoveToKafra,4,13);
	packet(0x009b,35,clif->pWantToConnection,7,21,26,30,34);
	packet(0x009f,21,clif->pUseItem,7,17);
	packet(0x00a2,10,clif->pSolveCharName,6);
	packet(0x00a7,8,clif->pWalkToXY,5);
	packet(0x00f5,11,clif->pTakeItem,7);
	packet(0x00f7,15,clif->pMoveFromKafra,3,11);
	packet(0x0113,40,clif->pUseSkillToPos,10,19,23,38);
	packet(0x0116,19,clif->pDropItem,11,17);
	packet(0x0190,10,clif->pActionRequest,4,9);
#endif

//2007-01-22aSakexe
#if PACKETVER >= 20070122
	packet(0x02a3,18);
	packet(0x02a4,2);
#endif

//2007-01-29aSakexe
#if PACKETVER >= 20070129
	packet(0x029b,72);
	packet(0x02a3,-1);
	packet(0x02a4,-1);
	packet(0x02a5,8);

#endif

//2007-02-05aSakexe
#if PACKETVER >= 20070205
	packet(0x02aa,4);
	packet(0x02ab,36);
	packet(0x02ac,6);
#endif

//2007-02-12aSakexe
#if PACKETVER >= 20070212
	packet(0x0072,25,clif->pUseSkillToId,6,10,21);
	packet(0x007e,102,clif->pUseSkillToPosMoreInfo,5,9,12,20,22);
	packet(0x0085,11,clif->pChangeDir,7,10);
	packet(0x0089,8,clif->pTickSend,4);
	packet(0x008c,11,clif->pGetCharNameRequest,7);
	packet(0x0094,14,clif->pMoveToKafra,7,10);
	packet(0x009b,26,clif->pWantToConnection,4,9,17,21,25);
	packet(0x009f,14,clif->pUseItem,4,10);
	packet(0x00a2,15,clif->pSolveCharName,11);
	//packet(0x00a7,8,clif->pWalkToXY,5);
	packet(0x00f5,8,clif->pTakeItem,4);
	packet(0x00f7,22,clif->pMoveFromKafra,14,18);
	packet(0x0113,22,clif->pUseSkillToPos,5,9,12,20);
	packet(0x0116,10,clif->pDropItem,5,8);
	packet(0x0190,19,clif->pActionRequest,5,18);
#endif

//2007-05-07aSakexe
#if PACKETVER >= 20070507
	packet(0x01fd,15,clif->pRepairItem,2);
#endif

//2007-02-27aSakexe to 2007-10-02aSakexe
#if PACKETVER >= 20070227
	packet(0x0288,10,clif->pcashshop_buy,2,4,6);
	packet(0x0289,12);
	packet(0x02a6,22);
	packet(0x02a7,22);
	packet(0x02a8,162);
	packet(0x02a9,58);
	packet(0x02ad,8);
	packet(0x02b0,85);
	packet(0x02b1,-1);
	packet(0x02b2,-1);
	packet(0x02b3,107);
	packet(0x02b4,6);
	packet(0x02b5,-1);
	packet(0x02b6,7,clif->pquestStateAck,2,6);
	packet(0x02b7,7);
	packet(0x02b8,22);
	packet(0x02b9,191);
	packet(0x02ba,11,clif->pHotkey,2,4,5,9);
	packet(0x02bb,8);
	packet(0x02bc,6);
	packet(0x02bf,10);
	packet(0x02c0,2);
	packet(0x02c1,-1);
	packet(0x02c2,-1);
	packet(0x02c4,26,clif->pPartyInvite2,2);
	packet(0x02c5,30);
	packet(0x02c6,30);
	packet(0x02c7,7,clif->pReplyPartyInvite2,2,6);
	packet(0x02c8,3,clif->pPartyTick,2);
	packet(0x02c9,3);
	packet(0x02ca,3);
	packet(0x02cb,20);
	packet(0x02cc,4);
	packet(0x02cd,26);
	packet(0x02ce,10);
	packet(0x02cf,6);
	packet(0x02d0,-1);
	packet(0x02d1,-1);
	packet(0x02d2,-1);
	packet(0x02d3,4);
	packet(0x02d4,29);
	packet(0x02d5,2);
	packet(0x02d6,6,clif->pViewPlayerEquip,2);
	packet(0x02d7,-1);
	packet(0x02d8,10,clif->pEquipTick,6);
	packet(0x02d9,10);
	packet(0x02da,3);
	packet(0x02db,-1,clif->pBattleChat,2,4);
	packet(0x02dc,-1);
	packet(0x02dd,32);
	packet(0x02de,6);
	packet(0x02df,36);
	packet(0x02e0,34);
#endif

//2007-10-23aSakexe
#if PACKETVER >= 20071023
	packet(0x02cb,65);
	packet(0x02cd,71);
#endif

//2007-11-06aSakexe
#if PACKETVER >= 20071106
	packet(0x0078,55);
	packet(0x007c,42);
	packet(0x022c,65);
	packet(0x029b,80);
#endif

//2007-11-13aSakexe
#if PACKETVER >= 20071113
	packet(0x02e1,33);
#endif

//2007-11-20aSakexe
#if PACKETVER >= 20071120
	//packet(0x01df,10 <- ???);
	packet(0x02e2,14);
	packet(0x02e3,25);
	packet(0x02e4,8);
	packet(0x02e5,8);
	packet(0x02e6,6);
#endif

//2007-11-27aSakexe
#if PACKETVER >= 20071127
	packet(0x02e7,-1);
#endif

//2008-01-02aSakexe
#if PACKETVER >= 20080102
	packet(0x01df,6,clif->pGMReqAccountName,2);
	packet(0x02e8,-1);
	packet(0x02e9,-1);
	packet(0x02ea,-1);
	packet(0x02eb,13);
	packet(0x02ec,67);
	packet(0x02ed,59);
	packet(0x02ee,60);
	packet(0x02ef,8);
#endif

//2008-03-18aSakexe
#if PACKETVER >= 20080318
	packet(0x02bf,-1);
	packet(0x02c0,-1);
	packet(0x02f0,10);
	packet(0x02f1,2,clif->pProgressbar,0);
	packet(0x02f2,2);
#endif

//2008-03-25bSakexe
#if PACKETVER >= 20080325
	packet(0x02f3,-1);
	packet(0x02f4,-1);
	packet(0x02f5,-1);
	packet(0x02f6,-1);
	packet(0x02f7,-1);
	packet(0x02f8,-1);
	packet(0x02f9,-1);
	packet(0x02fa,-1);
	packet(0x02fb,-1);
	packet(0x02fc,-1);
	packet(0x02fd,-1);
	packet(0x02fe,-1);
	packet(0x02ff,-1);
	packet(0x0300,-1);
#endif

//2008-04-01aSakexe
#if PACKETVER >= 20080401
	packet(0x0301,-1);
	packet(0x0302,-1);
	packet(0x0303,-1);
	packet(0x0304,-1);
	packet(0x0305,-1);
	packet(0x0306,-1);
	packet(0x0307,-1);
	packet(0x0308,-1);
	packet(0x0309,-1);
	packet(0x030a,-1);
	packet(0x030b,-1);
	packet(0x030c,-1);
	packet(0x030d,-1);
	packet(0x030e,-1);
	packet(0x030f,-1);
	packet(0x0310,-1);
	packet(0x0311,-1);
	packet(0x0312,-1);
	packet(0x0313,-1);
	packet(0x0314,-1);
	packet(0x0315,-1);
	packet(0x0316,-1);
	packet(0x0317,-1);
	packet(0x0318,-1);
	packet(0x0319,-1);
	packet(0x031a,-1);
	packet(0x031b,-1);
	packet(0x031c,-1);
	packet(0x031d,-1);
	packet(0x031e,-1);
	packet(0x031f,-1);
	packet(0x0320,-1);
	packet(0x0321,-1);
	packet(0x0322,-1);
	packet(0x0323,-1);
	packet(0x0324,-1);
	packet(0x0325,-1);
	packet(0x0326,-1);
	packet(0x0327,-1);
	packet(0x0328,-1);
	packet(0x0329,-1);
	packet(0x032a,-1);
	packet(0x032b,-1);
	packet(0x032c,-1);
	packet(0x032d,-1);
	packet(0x032e,-1);
	packet(0x032f,-1);
	packet(0x0330,-1);
	packet(0x0331,-1);
	packet(0x0332,-1);
	packet(0x0333,-1);
	packet(0x0334,-1);
	packet(0x0335,-1);
	packet(0x0336,-1);
	packet(0x0337,-1);
	packet(0x0338,-1);
	packet(0x0339,-1);
	packet(0x033a,-1);
	packet(0x033b,-1);
	packet(0x033c,-1);
	packet(0x033d,-1);
	packet(0x033e,-1);
	packet(0x033f,-1);
	packet(0x0340,-1);
	packet(0x0341,-1);
	packet(0x0342,-1);
	packet(0x0343,-1);
	packet(0x0344,-1);
	packet(0x0345,-1);
	packet(0x0346,-1);
	packet(0x0347,-1);
	packet(0x0348,-1);
	packet(0x0349,-1);
	packet(0x034a,-1);
	packet(0x034b,-1);
	packet(0x034c,-1);
	packet(0x034d,-1);
	packet(0x034e,-1);
	packet(0x034f,-1);
	packet(0x0350,-1);
	packet(0x0351,-1);
	packet(0x0352,-1);
	packet(0x0353,-1);
	packet(0x0354,-1);
	packet(0x0355,-1);
	packet(0x0356,-1);
	packet(0x0357,-1);
	packet(0x0358,-1);
	packet(0x0359,-1);
	packet(0x035a,-1);
#endif

//2008-05-27aSakexe
#if PACKETVER >= 20080527
	packet(0x035b,-1);
	packet(0x035c,2);
	packet(0x035d,-1);
	packet(0x035e,2);
	packet(0x035f,-1);
	packet(0x0389,-1);
#endif

//2008-08-20aSakexe
#if PACKETVER >= 20080820
	packet(0x040c,-1);
	packet(0x040d,-1);
	packet(0x040e,-1);
	packet(0x040f,-1);
	packet(0x0410,-1);
	packet(0x0411,-1);
	packet(0x0412,-1);
	packet(0x0413,-1);
	packet(0x0414,-1);
	packet(0x0415,-1);
	packet(0x0416,-1);
	packet(0x0417,-1);
	packet(0x0418,-1);
	packet(0x0419,-1);
	packet(0x041a,-1);
	packet(0x041b,-1);
	packet(0x041c,-1);
	packet(0x041d,-1);
	packet(0x041e,-1);
	packet(0x041f,-1);
	packet(0x0420,-1);
	packet(0x0421,-1);
	packet(0x0422,-1);
	packet(0x0423,-1);
	packet(0x0424,-1);
	packet(0x0425,-1);
	packet(0x0426,-1);
	packet(0x0427,-1);
	packet(0x0428,-1);
	packet(0x0429,-1);
	packet(0x042a,-1);
	packet(0x042b,-1);
	packet(0x042c,-1);
	packet(0x042d,-1);
	packet(0x042e,-1);
	packet(0x042f,-1);
	packet(0x0430,-1);
	packet(0x0431,-1);
	packet(0x0432,-1);
	packet(0x0433,-1);
	packet(0x0434,-1);
	packet(0x0435,-1);
#endif

//2008-09-10aSakexe
#if PACKETVER >= 20080910
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0437,7,clif->pActionRequest,2,6);
	packet(0x0438,10,clif->pUseSkillToId,2,4,6);
	packet(0x0439,8,clif->pUseItem,2,4);
#endif

//2008-11-13aSakexe
#if PACKETVER >= 20081113
	packet(0x043d,8);
	packet(0x043e,-1);
	packet(0x043f,8);
#endif

//2008-11-26aSakexe
#if PACKETVER >= 20081126
	packet(0x01a2,37);
	packet(0x0440,10);
	packet(0x0441,4);
#endif

//2008-12-10aSakexe
#if PACKETVER >= 20081210
	packet(0x0442,-1);
	packet(0x0443,8,clif->pSkillSelectMenu,2,6);
#endif

//2009-01-14aSakexe
#if PACKETVER >= 20090114
	packet(0x043f,25);
	packet(0x0444,-1);
	packet(0x0445,10);
#endif

//2009-02-18aSakexe
#if PACKETVER >= 20090218
	packet(0x0446,14);
#endif

//2009-02-25aSakexe
#if PACKETVER >= 20090225
	packet(0x0448,-1);
#endif

//2009-03-30aSakexe
#if PACKETVER >= 20090330
	packet(0x0449,4);
#endif

//2009-04-08aSakexe
#if PACKETVER >= 20090408
	packet(0x02a6,-1);
	packet(0x02a7,-1);
	packet(0x044a,6);
#endif

//2008-08-27aRagexeRE
#if PACKETVER >= 20080827
	packet(0x0072,22,clif->pUseSkillToId,9,15,18);
	packet(0x007c,44);
	packet(0x007e,105,clif->pUseSkillToPosMoreInfo,10,14,18,23,25);
	packet(0x0085,10,clif->pChangeDir,4,9);
	packet(0x0089,11,clif->pTickSend,7);
	packet(0x008c,14,clif->pGetCharNameRequest,10);
	packet(0x0094,19,clif->pMoveToKafra,3,15);
	packet(0x009b,34,clif->pWantToConnection,7,15,25,29,33);
	packet(0x009f,20,clif->pUseItem,7,20);
	packet(0x00a2,14,clif->pSolveCharName,10);
	packet(0x00a7,9,clif->pWalkToXY,6);
	packet(0x00f5,11,clif->pTakeItem,7);
	packet(0x00f7,17,clif->pMoveFromKafra,3,13);
	packet(0x0113,25,clif->pUseSkillToPos,10,14,18,23);
	packet(0x0116,17,clif->pDropItem,6,15);
	packet(0x0190,23,clif->pActionRequest,9,22);
	packet(0x02e2,20);
	packet(0x02e3,22);
	packet(0x02e4,11);
	packet(0x02e5,9);
#endif

//2008-09-10aRagexeRE
#if PACKETVER >= 20080910
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0437,7,clif->pActionRequest,2,6);
	packet(0x0438,10,clif->pUseSkillToId,2,4,6);
	packet(0x0439,8,clif->pUseItem,2,4);

#endif

//2008-11-12aRagexeRE
#if PACKETVER >= 20081112
	packet(0x043d,8);
	//packet(0x043e,-1);
	packet(0x043f,8);
#endif

//2008-12-17aRagexeRE
#if PACKETVER >= 20081217
	packet(0x01a2,37);
	//packet(0x0440,10);
	//packet(0x0441,4);
	//packet(0x0442,8);
	//packet(0x0443,8);
#endif

//2008-12-17bRagexeRE
#if PACKETVER >= 20081217
	packet(0x006d,114);

#endif

//2009-01-21aRagexeRE
#if PACKETVER >= 20090121
	packet(0x043f,25);
	//packet(0x0444,-1);
	//packet(0x0445,10);
#endif

//2009-02-18aRagexeRE
#if PACKETVER >= 20090218
	//packet(0x0446,14);
#endif

//2009-02-26cRagexeRE
#if PACKETVER >= 20090226
	//packet(0x0448,-1);
#endif

//2009-04-01aRagexeRE
#if PACKETVER >= 20090401
	//packet(0x0449,4);
#endif

//2009-05-14aRagexeRE
#if PACKETVER >= 20090514
	//packet(0x044b,2);
#endif

//2009-05-20aRagexeRE
#if PACKETVER >= 20090520
	//packet(0x07d0,6);
	//packet(0x07d1,2);
	//packet(0x07d2,-1);
	//packet(0x07d3,4);
	//packet(0x07d4,4);
	//packet(0x07d5,4);
	//packet(0x07d6,4);
	//packet(0x0447,2);
#endif

//2009-06-03aRagexeRE
#if PACKETVER >= 20090603
	packet(0x07d7,8,clif->pPartyChangeOption,2,6,7);
	packet(0x07d8,8);
	packet(0x07d9,254);
	packet(0x07da,6,clif->pPartyChangeLeader,2);
#endif

//2009-06-10aRagexeRE
#if PACKETVER >= 20090610
	//packet(0x07db,8);
#endif

//2009-06-17aRagexeRE
#if PACKETVER >= 20090617
	packet(0x07d9,268);
	//packet(0x07dc,6);
	//packet(0x07dd,54);
	//packet(0x07de,30);
	//packet(0x07df,54);
#endif

//2009-07-01aRagexeRE
#if PACKETVER >= 20090701
	//packet(0x0275,37);
	//packet(0x0276,-1);
#endif

//2009-07-08aRagexeRE
#if PACKETVER >= 20090708
	//packet(0x07e0,58);
#endif

//2009-07-15aRagexeRE
#if PACKETVER >= 20090715
	packet(0x07e1,15);
#endif

//2009-08-05aRagexeRE
#if PACKETVER >= 20090805
	packet(0x07e2,8);
#endif

//2009-08-18aRagexeRE
#if PACKETVER >= 20090818
	packet(0x07e3,6);
	packet(0x07e4,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x07e6,8);
#endif

//2009-08-25aRagexeRE
#if PACKETVER >= 20090825
	//packet(0x07e6,28);
	packet(0x07e7,5);
#endif

//2009-09-22aRagexeRE
#if PACKETVER >= 20090922
	packet(0x07e5,8);
	packet(0x07e6,8);
	packet(0x07e7,32);
	packet(0x07e8,-1);
	packet(0x07e9,5);
#endif

//2009-09-29aRagexeRE
#if PACKETVER >= 20090929
	//packet(0x07ea,2);
	//packet(0x07eb,-1);
	//packet(0x07ec,6);
	//packet(0x07ed,8);
	//packet(0x07ee,6);
	//packet(0x07ef,8);
	//packet(0x07f0,4);
	//packet(0x07f2,4);
	//packet(0x07f3,3);
#endif

//2009-10-06aRagexeRE
#if PACKETVER >= 20091006
	//packet(0x07ec,8);
	//packet(0x07ed,10);
	//packet(0x07f0,8);
	//packet(0x07f1,15);
	//packet(0x07f2,6);
	//packet(0x07f3,4);
	//packet(0x07f4,3);
#endif

//2009-10-27aRagexeRE
#if PACKETVER >= 20091027
	packet(0x07f5,6,clif->pGMFullStrip,2);
	packet(0x07f6,14);
#endif

//2009-11-03aRagexeRE
#if PACKETVER >= 20091103
	packet(0x07f7,-1);
	packet(0x07f8,-1);
	packet(0x07f9,-1);
#endif

//2009-11-17aRagexeRE
#if PACKETVER >= 20091117
	packet(0x07fa,8);

#endif

//2009-11-24aRagexeRE
#if PACKETVER >= 20091124
	packet(0x07fb,25);
#endif

//2009-12-01aRagexeRE
#if PACKETVER >= 20091201
	//packet(0x07fc,10);
	//packet(0x07fd,-1);
	packet(0x07fe,26);
	//packet(0x07ff,-1);
#endif

//2009-12-15aRagexeRE
#if PACKETVER >= 20091215
	packet(0x0800,-1);
	//packet(0x0801,-1);
#endif

//2009-12-22aRagexeRE
#if PACKETVER >= 20091222
	packet(0x0802,18,clif->pPartyBookingRegisterReq,2,4,6); // Booking System
	packet(0x0803,4);
	packet(0x0804,8); // Booking System
	packet(0x0805,-1);
	packet(0x0806,4,clif->pPartyBookingDeleteReq,2);// Booking System
	//packet(0x0807,2);
	packet(0x0808,4); // Booking System
	//packet(0x0809,14);
	//packet(0x080A,50);
	//packet(0x080B,18);
	//packet(0x080C,6);
#endif

	//2009-12-29aRagexeRE
#if PACKETVER >= 20091229
	packet(0x0804,14,clif->pPartyBookingSearchReq,2,4,6,8,12);// Booking System
	packet(0x0806,2,clif->pPartyBookingDeleteReq,0);// Booking System
	packet(0x0807,4);
	packet(0x0808,14,clif->pPartyBookingUpdateReq,2); // Booking System
	packet(0x0809,50);
	packet(0x080A,18);
	packet(0x080B,6);// Booking System
#endif

	//2010-01-05aRagexeRE
#if PACKETVER >= 20100105
	packet(0x0801,-1,clif->pPurchaseReq2,2,4,8,12);
#endif

	//2010-01-26aRagexeRE
#if PACKETVER >= 20100126
	//packet(0x080C,2);
	//packet(0x080D,3);
	packet(0x080E,14);
#endif

	//2010-02-09aRagexeRE
#if PACKETVER >= 20100209
	//packet(0x07F0,6);
#endif

	//2010-02-23aRagexeRE
#if PACKETVER >= 20100223
	packet(0x080F,20);
#endif

	//2010-03-03aRagexeRE
#if PACKETVER >= 20100303
	packet(0x0810,3);
	packet(0x0811,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	//packet(0x0812,86);
	//packet(0x0813,6);
	//packet(0x0814,6);
	//packet(0x0815,-1);
	//packet(0x0817,-1);
	//packet(0x0818,6);
	//packet(0x0819,4);
#endif

	//2010-03-09aRagexeRE
#if PACKETVER >= 20100309
	packet(0x0813,-1);
	//packet(0x0814,2);
	//packet(0x0815,6);
	packet(0x0816,6);
	packet(0x0818,-1);
	//packet(0x0819,10);
	//packet(0x081A,4);
	//packet(0x081B,4);
	//packet(0x081C,6);
	packet(0x081d,22);
	packet(0x081e,8);
#endif

	//2010-03-23aRagexeRE
#if PACKETVER >= 20100323
	//packet(0x081F,-1);
#endif

	//2010-04-06aRagexeRE
#if PACKETVER >= 20100406
	//packet(0x081A,6);
#endif

	//2010-04-13aRagexeRE
#if PACKETVER >= 20100413
	//packet(0x081A,10);
	packet(0x0820,11);
	//packet(0x0821,2);
	//packet(0x0822,9);
	//packet(0x0823,-1);
#endif

	//2010-04-14dRagexeRE
#if PACKETVER >= 20100414
	//packet(0x081B,8);
#endif

	//2010-04-20aRagexeRE
#if PACKETVER >= 20100420
	packet(0x0812,8);
	packet(0x0814,86);
	packet(0x0815,2,clif->pReqCloseBuyingStore,0);
	packet(0x0817,6,clif->pReqClickBuyingStore,2);
	packet(0x0819,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x081a,4);
	packet(0x081b,10);
	packet(0x081c,10);
	packet(0x0824,6);
#endif

	//2010-06-01aRagexeRE
#if PACKETVER >= 20100601
	//packet(0x0825,-1);
	//packet(0x0826,4);
	packet(0x0835,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0836,-1);
	packet(0x0837,3);
	//packet(0x0838,3);
#endif

	//2010-06-08aRagexeRE
#if PACKETVER >= 20100608
	packet(0x0838,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x083A,4); // Search Stalls Feature
	packet(0x083B,2,clif->pCloseSearchStoreInfo,0);
	packet(0x083C,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x083D,6);
#endif

//2010-06-15aRagexeRE
#if PACKETVER >= 20100615
	//packet(0x083E,26);
#endif

//2010-06-22aRagexeRE
#if PACKETVER >= 20100622
	//packet(0x083F,22);
#endif

//2010-06-29aRagexeRE
#if PACKETVER >= 20100629
	packet(0x00AA,9);
	//packet(0x07F1,18);
	//packet(0x07F2,8);
	//packet(0x07F3,6);
#endif

//2010-07-01aRagexeRE
#if PACKETVER >= 20100701
	packet(0x083A,5);// Search Stalls Feature
#endif

//2010-07-13aRagexeRE
#if PACKETVER >= 20100713
	//packet(0x0827,6);
	//packet(0x0828,14);
	//packet(0x0829,6);
	//packet(0x082A,10);
	//packet(0x082B,6);
	//packet(0x082C,14);
	//packet(0x0840,-1);
	//packet(0x0841,19);
#endif

//2010-07-14aRagexeRE
#if PACKETVER >= 20100714
	//packet(0x841,4);
#endif

//2010-08-03aRagexeRE
#if PACKETVER >= 20100803
	packet(0x0839,66);
	packet(0x0842,6,clif->pGMRecall2,2);
	packet(0x0843,6,clif->pGMRemove2,2);
#endif

//2010-11-24aRagexeRE
#if PACKETVER >= 20101124
	packet(0x0288,-1,clif->pcashshop_buy,4,8);
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x035f,5,clif->pWalkToXY,2);
	packet(0x0360,6,clif->pTickSend,2);
	packet(0x0361,5,clif->pChangeDir,2,4);
	packet(0x0362,6,clif->pTakeItem,2);
	packet(0x0363,6,clif->pDropItem,2,4);
	packet(0x0364,8,clif->pMoveToKafra,2,4);
	packet(0x0365,8,clif->pMoveFromKafra,2,4);
	packet(0x0366,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0367,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0368,6,clif->pGetCharNameRequest,2);
	packet(0x0369,6,clif->pSolveCharName,2);
	packet(0x0856,-1);
	packet(0x0857,-1);
	packet(0x0858,-1);
	packet(0x0859,-1);
#endif

// 2010-12-21aRagexe
#if PACKETVER >= 20101221
// shuffle packets not added
// new packets
	packet(0x08b1,-1); // ZC_MCSTORE_NOTMOVEITEM_LIST
#endif

// 2011-01-11aRagexe
#if PACKETVER >= 20110111
// shuffle packets not added
// new packets
	packet(0x08b3,-1); // ZC_SHOWSCRIPT
#endif

// 2011-01-25aRagexe
#if PACKETVER >= 20110125
// shuffle packets not added
// new packets
	packet(0x08b4,2); // ZC_START_COLLECTION
	packet(0x08b5,6,clif->pDull,2); // CZ_TRYCOLLECTION
	packet(0x08b6,3); // ZC_TRYCOLLECTION
#endif

// 2011-01-31aRagexe
#if PACKETVER >= 20110131
// shuffle packets not added
// new packets
	packet(0x02f3,-1,clif->pDull); // CZ_IRMAIL_SEND
	packet(0x02f4,3); // ZC_IRMAIL_SEND_RES
	packet(0x02f5,7); // ZC_IRMAIL_NOTIFY
	packet(0x02f6,7,clif->pDull,2); // CZ_IRMAIL_LIST
#endif

// 2011-02-22aRagexe
#if PACKETVER >= 20110222
// shuffle packets not added
// new packets
	packet(0x08c0,-1); // ZC_ACK_SE_CASH_ITEM_LIST2
	packet(0x08c1,2,clif->pDull); // CZ_MACRO_START
	packet(0x08c2,2,clif->pDull); // CZ_MACRO_STOP
#endif

// 2011-04-19aRagexe
#if PACKETVER >= 20110419
// shuffle packets not added
// new packets
	packet(0x08c7,-1); // ZC_SKILL_ENTRY3
#endif

// 2011-06-14aRagexe
#if PACKETVER >= 20110614
// shuffle packets not added
// new packets
	packet(0x08c8,34); // ZC_NOTIFY_ACT3
	packet(0x08c9,2,clif->pCashShopSchedule,0);
	packet(0x08ca,-1); // ZC_ACK_SCHEDULER_CASHITEM
#endif

// 2011-06-27aRagexe
#if PACKETVER >= 20110627
// shuffle packets not added
// new packets
	packet(0x08cb,-1); // ZC_PERSONAL_INFOMATION
#endif

//2011-07-18aRagexe (Thanks to Yommy!)
#if PACKETVER >= 20110718
// shuffle packets not added
	packet(0x0844,2,clif->pCashShopOpen,2);/* tell server cashshop window is being open */
	packet(0x084a,2,clif->pCashShopClose,2);/* tell server cashshop window is being closed */
	packet(0x0846,4,clif->pCashShopReqTab,2);
	packet(0x0848,-1,clif->pCashShopBuy,2);
#endif

// 2011-08-02aRagexe
#if PACKETVER >= 20110802
// shuffle packets not added
// new packets
	packet(0x09dc,2); // unknown
#endif

// 2011-08-09aRagexe
#if PACKETVER >= 20110809
// shuffle packets not added
// new packets
	packet(0x08cf,10); // ZC_SPIRITS_ATTRIBUTE
	packet(0x08d0,9); // ZC_REQ_WEAR_EQUIP_ACK2
	packet(0x08d1,7); // ZC_REQ_TAKEOFF_EQUIP_ACK2
	packet(0x08d2,10); // ZC_FASTMOVE
#endif

// 2011-08-16aRagexe
#if PACKETVER >= 20110816
// shuffle packets not added
// new packets
	packet(0x08d3,10); // ZC_SE_CASHSHOP_UPDATE
#endif

// 2011-09-28aRagexe
#if PACKETVER >= 20110928
// shuffle packets not added
// new packets
	packet(0x08d6,6); // ZC_CLEAR_DIALOG
#endif

//2011-10-05aRagexeRE
#if PACKETVER >= 20111005
	packet(0x0364,5,clif->pWalkToXY,2);
	packet(0x0817,6,clif->pTickSend,2);
	packet(0x0366,5,clif->pChangeDir,2,4);
	packet(0x0815,6,clif->pTakeItem,2);
	packet(0x0885,6,clif->pDropItem,2,4);
	packet(0x0893,8,clif->pMoveToKafra,2,4);
	packet(0x0897,8,clif->pMoveFromKafra,2,4);
	packet(0x0369,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x08ad,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x088a,6,clif->pGetCharNameRequest,2);
	packet(0x0838,6,clif->pSolveCharName,2);
	packet(0x0439,8,clif->pUseItem,2,4);
	packet(0x08d7,28,clif->pBGQueueRegister,2);
	packet(0x090a,26,clif->pBGQueueCheckState,2);
	packet(0x08da,26,clif->pBGQueueRevokeReq,2);
	packet(0x08e0,51,clif->pBGQueueBattleBeginAck,2);
#endif

//2011-11-02aRagexe
#if PACKETVER >= 20111102
	packet(0x0436,26,clif->pFriendsListAdd,2);
	packet(0x0898,5,clif->pHomMenu,4);
	packet(0x0281,36,clif->pStoragePassword,0);
	packet(0x088d,26,clif->pPartyInvite2,2);
	packet(0x083c,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x08aa,7,clif->pActionRequest,2,6);
	packet(0x02c4,10,clif->pUseSkillToId,2,4,6);
	packet(0x0811,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x890,8);
	packet(0x08a5,18,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x0835,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x089b,2,clif->pReqCloseBuyingStore,0);
	packet(0x08a1,6,clif->pReqClickBuyingStore,2);
	packet(0x089e,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x08ab,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x088b,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x08a2,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	#ifndef PACKETVER_RE
		packet(0x0835,19,clif->pWantToConnection,2,6,10,14,18);
		packet(0x0892,5,clif->pWalkToXY,2);
		packet(0x0899,6,clif->pTickSend,2);
	#endif
#endif

//2012-03-07fRagexeRE
#if PACKETVER >= 20120307
	packet(0x086A,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x0887,6,clif->pTickSend,2);
	packet(0x0890,5,clif->pChangeDir,2,4);
	packet(0x0865,6,clif->pTakeItem,2);
	packet(0x02C4,6,clif->pDropItem,2,4);
	packet(0x093B,8,clif->pMoveToKafra,2,4);
	packet(0x0963,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0369,26,clif->pFriendsListAdd,2);
	packet(0x0863,5,clif->pHomMenu,4);
	packet(0x0861,36,clif->pStoragePassword,0);
	packet(0x0929,26,clif->pPartyInvite2,2);
	packet(0x0885,7,clif->pActionRequest,2,6);
	packet(0x0889,10,clif->pUseSkillToId,2,4,6);
	packet(0x0870,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0884,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0439,8,clif->pUseItem,2,4);
// changed packet sizes
	packet(0x08e2,27); // ZC_NAVIGATION_ACTIVE
#endif

//2012-04-10aRagexeRE
#if PACKETVER >= 20120410
	packet(0x01FD,15,clif->pRepairItem,2);
	packet(0x089C,26,clif->pFriendsListAdd,2);
	packet(0x0885,5,clif->pHomMenu,2,4);
	packet(0x0961,36,clif->pStoragePassword,0);
	packet(0x0288,-1,clif->pcashshop_buy,4,8);
	packet(0x091C,26,clif->pPartyInvite2,2);
	packet(0x094B,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0439,8,clif->pUseItem,2,4);
	packet(0x0945,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x0886,6,clif->pTickSend,2);
	packet(0x0871,5,clif->pChangeDir,2,4);
	packet(0x0938,6,clif->pTakeItem,2);
	packet(0x0891,6,clif->pDropItem,2,4);
	packet(0x086C,8,clif->pMoveToKafra,2,4);
	packet(0x08A6,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0889,6,clif->pGetCharNameRequest,2);
	packet(0x0884,6,clif->pSolveCharName,2);
#ifndef PACKETVER_RE
	packet(0x091D,18,clif->pPartyBookingRegisterReq,2,4,6);
#else
	packet(0x08E5,41,clif->pPartyRecruitRegisterReq,2,4);
#endif
	packet(0x08E6,4);
	packet(0x08E7,10,clif->pPartyRecruitSearchReq,2);
	packet(0x08E8,-1);
	packet(0x08E9,2,clif->pPartyRecruitDeleteReq,2);
	packet(0x08EA,4);
	packet(0x08EB,39,clif->pPartyRecruitUpdateReq,2);
	packet(0x08EC,73);
	packet(0x08ED,43);
	packet(0x08EE,6);
#ifdef PARTY_RECRUIT
	packet(0x08EF,6,clif->pDull,2); //bookingignorereq
	packet(0x08F0,6,clif->pDull,2);
	packet(0x08F1,6,clif->pDull,2); //bookingjoinpartyreq
#endif
	packet(0x08F2,36);
	packet(0x08F3,-1);
	packet(0x08F4,6);
	packet(0x08F5,-1,clif->pDull,2,4); //bookingsummonmember
	packet(0x08F6,22);
	packet(0x08F7,3);
	packet(0x08F8,7);
	packet(0x08F9,6);
#ifdef PARTY_RECRUIT
	packet(0x08F9,6,clif->pDull,2);
#endif
	packet(0x08FA,6);
	packet(0x08FB,6,clif->pDull,2); //bookingcanceljoinparty
	packet(0x0907,5,clif->pMoveItem,2,4);
	packet(0x0908,5);
	packet(0x0977,14);//Monster HP Bar
#endif

//2012-04-18aRagexeRE [Special Thanks to Judas!]
#if PACKETVER >= 20120418
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x08A8,36,clif->pStoragePassword,0);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0202,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x08E5,41,clif->pPartyRecruitRegisterReq,2,4);
	packet(0x0916,26,clif->pGuildInvite2,2);
#endif

// 2012-05-02aRagexeRE
#if PACKETVER >= 20120502
// shuffle packets not added
	packet(0x097d,288); // ZC_ACK_RANKING
	packet(0x097e,12); // ZC_UPDATE_RANKING_POINT
	packet(0x097f,-1);  // ZC_SELECTCART
	packet(0x0980,7,clif->pSelectCart); // CZ_SELECTCART
#endif

#ifndef PACKETVER_RE
#if PACKETVER >= 20120604
// shuffle packets not added
	packet(0x0861,18,clif->pPartyRecruitRegisterReq,2,4,6);
#endif
#endif

//2012-06-18aRagexeRE
#if PACKETVER >= 20120618
// shuffle packets not added
	packet(0x0983,29);
#endif

// ========== 2012-07-02aRagexeRE  =============
// - 2012-07-02 is NOT STABLE.
// - The packets are kept here for reference, DONT USE THEM.
#if PACKETVER >= 20120702
	packet(0x0363,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0364,6,clif->pTickSend,2);
	packet(0x085a,7,clif->pActionRequest,2,6);
	packet(0x0861,8,clif->pMoveFromKafra,2,4);
	packet(0x0862,10,clif->pUseSkillToId,2,4,6);
	packet(0x0863,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0886,6,clif->pSolveCharName,2);
	packet(0x0889,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x089e,6,clif->pDropItem,2,4);
	packet(0x089f,6,clif->pTakeItem,2);
	packet(0x08a0,8,clif->pMoveToKafra,2,4);
	packet(0x094a,6,clif->pGetCharNameRequest,2);
	packet(0x0953,5,clif->pWalkToXY,2);
	packet(0x0960,5,clif->pChangeDir,2,4);
#endif

//2012-07-10
#if PACKETVER >= 20120710
	packet(0x0886,2,clif->pReqCloseBuyingStore,0);
#endif

//2012-07-16aRagExe (special thanks to Yommy/Frost!)
#if PACKETVER >= 20120716
	packet(0x0879,18,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0819,36,clif->pStoragePassword,0);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0439,8,clif->pUseItem,2,4);
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0940,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0811,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0202,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,4,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2012-09-25aRagexe
#if PACKETVER >= 20120925
// new packets (not all)
	packet(0x0998,8,clif->pEquipItem,2,4);
#endif

// 2013-02-06aRagexe
#if PACKETVER >= 20130206
// new packets
	packet(0x09a4,18); // ZC_DISPATCH_TIMING_INFO_CHN
// changed packet sizes
#endif

// 2013-03-06aRagexe
#if PACKETVER >= 20130306
// new packets
	packet(0x09a6,12); // ZC_BANKING_CHECK
	packet(0x09a7,14,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_DEPOSIT
	packet(0x09a8,4); // ZC_ACK_BANKING_DEPOSIT
	packet(0x09a9,14,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_WITHDRAW
	packet(0x09aa,4); // ZC_ACK_BANKING_WITHDRAW
// changed packet sizes
#endif

// 2013-03-13aRagexe
#if PACKETVER >= 20130313
// new packets
	packet(0x09ab,-1,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_CHECK
	packet(0x09ac,20,clif->pDull/*,XXX*/); // CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ad,6); // ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ae,-1,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09af,-1); // ZC_ACK_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,8,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09b1,6); // ZC_ACK_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09b2,-1); // ZC_NOTIFY_BARGAIN_SALE_SELLING
// changed packet sizes
#endif

//2013-03-20Ragexe (Judas + Yommy)
#if PACKETVER >= 20130320
	// Shuffle Start
	packet(0x088E,7,clif->pActionRequest,2,6);
	packet(0x089B,10,clif->pUseSkillToId,2,4,6);
	packet(0x0881,5,clif->pWalkToXY,2);
	packet(0x0363,6,clif->pTickSend,2);
	packet(0x0897,5,clif->pChangeDir,2,4);
	packet(0x0933,6,clif->pTakeItem,2);
	packet(0x0438,6,clif->pDropItem,2,4);
	packet(0x08AC,8,clif->pMoveToKafra,2,4);
	packet(0x0874,8,clif->pMoveFromKafra,2,4);
	packet(0x0959,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x085A,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0898,6,clif->pGetCharNameRequest,2);
	packet(0x094C,6,clif->pSolveCharName,2);
	packet(0x0365,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x092E,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x094E,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0922,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x035F,6,clif->pReqClickBuyingStore,2);
	packet(0x0886,2,clif->pReqCloseBuyingStore,0);
	packet(0x0938,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x085D,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x085D,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0868,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0888,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x086D,26,clif->pPartyInvite2,2);
	packet(0x086F,26,clif->pFriendsListAdd,2);
	packet(0x093F,5,clif->pHomMenu,2,4);
	packet(0x0947,36,clif->pStoragePassword,0);
	packet(0x0890,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x095a,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End

	// New Packets (wrong version or packet not exists)
	packet(0x0447,2); // PACKET_CZ_BLOCKING_PLAY_CANCEL
	packet(0x099f,24);
	// New Packets End
#endif

#if PACKETVER >= 20130320
// new packets
// changed packet sizes
	packet(0x09a7,10,clif->pBankDeposit,2,4,6); // CZ_REQ_BANKING_DEPOSIT
	packet(0x09a8,12); // ZC_ACK_BANKING_DEPOSIT
	packet(0x09a9,10,clif->pBankWithdraw,2,4,6); // CZ_REQ_BANKING_WITHDRAW
	packet(0x09aa,12); // ZC_ACK_BANKING_WITHDRAW
	packet(0x09ab,6,clif->pBankCheck,2,4); // CZ_REQ_BANKING_CHECK
#endif

// 2013-03-27bRagexe
#if PACKETVER >= 20130327
// new packets
	packet(0x09ac,-1,clif->pDull/*,XXX*/); // CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ad,10); // ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ae,17,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09af,4); // ZC_ACK_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,8,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09b1,4); // ZC_ACK_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09b2,6); // ZC_NOTIFY_BARGAIN_SALE_SELLING
	packet(0x09b3,6); // ZC_NOTIFY_BARGAIN_SALE_CLOSE
// changed packet sizes
#endif

//2013-05-15aRagexe (Shakto)
#if PACKETVER >= 20130515
	// Shuffle Start
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0362,5,clif->pChangeDir,2,4);
	packet(0x08A1,6,clif->pTakeItem,2);
	packet(0x0944,6,clif->pDropItem,2,4);
	packet(0x0887,8,clif->pMoveToKafra,2,4);
	packet(0x08AC,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x092D,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x092D,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0963,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0943,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0947,26,clif->pPartyInvite2,2);
	packet(0x0962,26,clif->pFriendsListAdd,2);
	packet(0x0931,5,clif->pHomMenu,2,4);
	packet(0x093E,36,clif->pStoragePassword,0);
	packet(0x0862,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x08aa,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End
#endif

//2013-05-22Ragexe (Shakto)
#if PACKETVER >= 20130522
	// Shuffle Start
	packet(0x08A2,7,clif->pActionRequest,2,6);
	packet(0x095C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0360,5,clif->pWalkToXY,2);
	packet(0x07EC,6,clif->pTickSend,2);
	packet(0x0925,5,clif->pChangeDir,2,4);
	packet(0x095E,6,clif->pTakeItem,2);
	packet(0x089C,6,clif->pDropItem,2,4);
	packet(0x08A3,8,clif->pMoveToKafra,2,4);
	packet(0x087E,8,clif->pMoveFromKafra,2,4);
	packet(0x0811,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0964,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x08A6,6,clif->pGetCharNameRequest,2);
	packet(0x0369,6,clif->pSolveCharName,2);
	packet(0x093E,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x08AA,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x095B,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0952,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0368,6,clif->pReqClickBuyingStore,2);
	packet(0x086E,2,clif->pReqCloseBuyingStore,0);
	packet(0x0874,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x089B,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x089B,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x086A,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x08A9,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0950,26,clif->pPartyInvite2,2);
	packet(0x0362,26,clif->pFriendsListAdd,2);
	packet(0x0926,5,clif->pHomMenu,2,4);
	packet(0x088E,36,clif->pStoragePassword,0);
	packet(0x08ac,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x0965,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End
#endif

//2013-05-29Ragexe (Shakto)
#if PACKETVER >= 20130529
	packet(0x0890,7,clif->pActionRequest,2,6);
	packet(0x0438,10,clif->pUseSkillToId,2,4,6);
	packet(0x0876,5,clif->pWalkToXY,2);
	packet(0x0897,6,clif->pTickSend,2);
	packet(0x0951,5,clif->pChangeDir,2,4);
	packet(0x0895,6,clif->pTakeItem,2);
	packet(0x08A7,6,clif->pDropItem,2,4);
	packet(0x0938,8,clif->pMoveToKafra,2,4);
	packet(0x0957,8,clif->pMoveFromKafra,2,4);
	packet(0x0917,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x085E,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0863,6,clif->pGetCharNameRequest,2);
	packet(0x0937,6,clif->pSolveCharName,2);
	packet(0x085A,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0941,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0918,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0936,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0892,6,clif->pReqClickBuyingStore,2);
	packet(0x0964,2,clif->pReqCloseBuyingStore,0);
	packet(0x0869,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0874,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0874,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0958,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0919,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x08A8,26,clif->pPartyInvite2,2);
	packet(0x0877,26,clif->pFriendsListAdd,2);
	packet(0x023B,5,clif->pHomMenu,2,4);
	packet(0x0956,36,clif->pStoragePassword,0);
	packet(0x0888,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x088e,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

//2013-06-05Ragexe (Shakto)
#if PACKETVER >= 20130605
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0202,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0365,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0883,36,clif->pStoragePassword,0);
	packet(0x097C,4,clif->pRanklist);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,4,clif->pDull); // CZ_GANGSI_RANK
#endif

//2013-06-12Ragexe (Shakto)
#if PACKETVER >= 20130612
// most shuffle packets used from 20130605
	packet(0x087E,5,clif->pChangeDir,2,4);
	packet(0x0919,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0940,26,clif->pFriendsListAdd,2);
	packet(0x093A,5,clif->pHomMenu,2,4);
	packet(0x0964,36,clif->pStoragePassword,0);
#endif

//2013-06-18Ragexe (Shakto)
#if PACKETVER >= 20130618
	packet(0x0889,7,clif->pActionRequest,2,6);
	packet(0x0951,10,clif->pUseSkillToId,2,4,6);
	packet(0x088E,5,clif->pWalkToXY,2);
	packet(0x0930,6,clif->pTickSend,2);
	packet(0x08A6,5,clif->pChangeDir,2,4);
	packet(0x0962,6,clif->pTakeItem,2);
	packet(0x0917,6,clif->pDropItem,2,4);
	packet(0x0885,8,clif->pMoveToKafra,2,4);
	packet(0x0936,8,clif->pMoveFromKafra,2,4);
	packet(0x096A,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x094F,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0944,6,clif->pGetCharNameRequest,2);
	packet(0x0945,6,clif->pSolveCharName,2);
	packet(0x0890,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0363,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0281,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0891,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0862,6,clif->pReqClickBuyingStore,2);
	packet(0x085A,2,clif->pReqCloseBuyingStore,0);
	packet(0x0932,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x08A7,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x08A7,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0942,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x095B,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0887,26,clif->pPartyInvite2,2);
	packet(0x0953,26,clif->pFriendsListAdd,2);
	packet(0x02C4,5,clif->pHomMenu,2,4);
	packet(0x0864,36,clif->pStoragePassword,0);
	packet(0x0878,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x087a,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

//2013-06-26Ragexe (Shakto)
#if PACKETVER >= 20130626
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x094D,5,clif->pChangeDir,2,4);
	packet(0x088B,6,clif->pTakeItem,2);
	packet(0x0952,6,clif->pDropItem,2,4);
	packet(0x0921,8,clif->pMoveToKafra,2,4);
	packet(0x0817,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0365,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0894,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0894,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x08A5,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x088C,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0895,26,clif->pPartyInvite2,2);
	packet(0x08AB,26,clif->pFriendsListAdd,2);
	packet(0x0960,5,clif->pHomMenu,2,4);
	packet(0x0930,36,clif->pStoragePassword,0);
	packet(0x0860,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x088f,4,clif->pDull); // CZ_GANGSI_RANK
#endif

//2013-07-03Ragexe (Shakto)
#if PACKETVER >= 20130703
	packet(0x0930,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0202,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0365,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x0360,26,clif->pFriendsListAdd,2);
	packet(0x094A,5,clif->pHomMenu,2,4);
	packet(0x0873,36,clif->pStoragePassword,0);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,4,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-04-17aRagexe
#if PACKETVER >= 20130417
// new packets
	packet(0x09b4,6,clif->pDull/*,XXX*/); // CZ_OPEN_BARGAIN_SALE_TOOL
	packet(0x09b5,2); // ZC_OPEN_BARGAIN_SALE_TOOL
	packet(0x09b6,6,clif->pBankOpen,2,4); // CZ_REQ_OPEN_BANKING
	packet(0x09b7,4); // ZC_ACK_OPEN_BANKING
	packet(0x09b8,6,clif->pBankClose,2,4); // CZ_REQ_CLOSE_BANKING
	packet(0x09b9,4); // ZC_ACK_CLOSE_BANKING
// changed packet sizes
#endif

// 2013-04-24aRagexe
#if PACKETVER >= 20130424
// new packets
	packet(0x09ba,6,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_GUILD_STORAGE
	packet(0x09bb,4); // ZC_ACK_OPEN_GUILD_STORAGE
	packet(0x09bc,6,clif->pDull/*,XXX*/); // CZ_CLOSE_BARGAIN_SALE_TOOL
	packet(0x09bd,2); // ZC_CLOSE_BARGAIN_SALE_TOOL
// changed packet sizes
#endif

// 2013-05-02aRagexe
#if PACKETVER >= 20130502
// new packets
	packet(0x09be,6,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_GUILD_STORAGE
	packet(0x09bf,4); // ZC_ACK_CLOSE_GUILD_STORAGE
// changed packet sizes
	packet(0x09bb,6); // ZC_ACK_OPEN_GUILD_STORAGE
#endif

// 2013-05-15aRagexe
#if PACKETVER >= 20130515
// new packets
	packet(0x09c0,11); // ZC_ACTION_MOVE
	packet(0x09c1,11); // ZC_C_MARKERINFO
// changed packet sizes
	packet(0x09a8,16); // ZC_ACK_BANKING_DEPOSIT
	packet(0x09aa,16); // ZC_ACK_BANKING_WITHDRAW
#endif

// 2013-05-29Ragexe
#if PACKETVER >= 20130529
// new packets
	packet(0x09c3,8,clif->pDull/*,XXX*/); // CZ_REQ_COUNT_BARGAIN_SALE_ITEM
// changed packet sizes
#endif

// 2013-06-05Ragexe
#if PACKETVER >= 20130605
// new packets
	packet(0x09c4,8); // ZC_ACK_COUNT_BARGAIN_SALE_ITEM
#endif

// 2013-06-18aRagexe
#if PACKETVER >= 20130618
// new packets
	packet(0x09ca,23); // ZC_SKILL_ENTRY5
// changed packet sizes
#endif

// 2013-07-17cRagexe
#if PACKETVER >= 20130717
// new packets
	packet(0x09cb,17); // ZC_USE_SKILL2
	packet(0x09cc,-1); // ZC_SECRETSCAN_DATA
// changed packet sizes
	packet(0x09c1,10); // ZC_C_MARKERINFO
#endif

//2013-08-07Ragexe (Shakto)
#if PACKETVER >= 20130807
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0202,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0365,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0887,36,clif->pStoragePassword,0);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,4,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-08-07aRagexe
#if PACKETVER >= 20130807
// new packets
	packet(0x09cd,8); // ZC_MSG_COLOR
// changed packet sizes
#endif

//2013-08-14aRagexe - Themon
#if PACKETVER >= 20130814
	packet(0x0874,7,clif->pActionRequest,2,6);
	packet(0x0947,10,clif->pUseSkillToId,2,4,6);
	packet(0x093A,5,clif->pWalkToXY,2);
	packet(0x088A,6,clif->pTickSend,2);
	packet(0x088C,5,clif->pChangeDir,2,4);
	packet(0x0926,6,clif->pTakeItem,2);
	packet(0x095F,6,clif->pDropItem,2,4);
	packet(0x0202,8,clif->pMoveToKafra,2,4);
	packet(0x0873,8,clif->pMoveFromKafra,2,4);
	packet(0x0887,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0962,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0937,6,clif->pGetCharNameRequest,2);
	packet(0x0923,6,clif->pSolveCharName,2);
	packet(0x0868,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0941,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0889,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0835,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0895,6,clif->pReqClickBuyingStore,2);
	packet(0x094E,2,clif->pReqCloseBuyingStore,0);
	packet(0x0936,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
#ifdef PACKETVER_RE
	packet(0x0365,41,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0959,18,clif->pPartyBookingRegisterReq,2,4);
#endif // PACKETVER_RE
	packet(0x08A4,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0368,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0927,26,clif->pPartyInvite2,2);
	packet(0x0281,26,clif->pFriendsListAdd,2);
	packet(0x0958,5,clif->pHomMenu,2,4);
	packet(0x0885,36,clif->pStoragePassword,0);
	packet(0x0815,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x0896,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

// 2013-08-14aRagexe
#if PACKETVER >= 20130814
// new packets
	packet(0x09ce,102,clif->pGM_Monster_Item,2); // CZ_ITEM_CREATE_EX
	packet(0x09cf,-1); // ZC_NPROTECTGAMEGUARDCSAUTH
	packet(0x09d0,-1,clif->pDull/*,XXX*/); // CZ_NPROTECTGAMEGUARDCSAUTH
// changed packet sizes
#endif

// 2013-08-21bRagexe
#if PACKETVER >= 20130821
// new packets
	packet(0x09d1,14); // ZC_PROGRESS_ACTOR
// changed packet sizes
#endif

// 2013-08-28bRagexe
#if PACKETVER >= 20130828
// new packets
	packet(0x09d2,-1); // ZC_GUILDSTORAGE_ITEMLIST_NORMAL_V5
	packet(0x09d3,-1); // ZC_GUILDSTORAGE_ITEMLIST_EQUIP_V5
// changed packet sizes
	packet(0x09ba,2,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_GUILD_STORAGE
	packet(0x09be,2,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_GUILD_STORAGE
#endif

// 2013-09-04aRagexe
#if PACKETVER >= 20130904
// new packets
// changed packet sizes
	packet(0x09ca,-1); // ZC_SKILL_ENTRY5
#endif

// 2013-09-11aRagexe
#if PACKETVER >= 20130911
// new packets
	packet(0x09d4,2,clif->pNPCShopClosed); // CZ_NPC_TRADE_QUIT
	packet(0x09d5,-1); // ZC_NPC_MARKET_OPEN
	packet(0x09d6,-1,clif->pNPCMarketPurchase); // CZ_NPC_MARKET_PURCHASE
	packet(0x09d7,-1); // ZC_NPC_MARKET_PURCHASE_RESULT
	packet(0x09d8,2,clif->pNPCMarketClosed); // CZ_NPC_MARKET_CLOSE
	packet(0x09d9,2,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
	packet(0x09da,2); // ZC_ACK_GUILDSTORAGE_LOG
// changed packet sizes
#endif

// 2013-09-25aRagexe
#if PACKETVER >= 20130925
// new packets
// changed packet sizes
	packet(0x09da,10); // ZC_ACK_GUILDSTORAGE_LOG
#endif

// 2013-10-02aRagexe
#if PACKETVER >= 20131002
// new packets
// changed packet sizes
	packet(0x09d9,4,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
	packet(0x09da,-1); // ZC_ACK_GUILDSTORAGE_LOG
#endif

// 2013-10-16aRagexe
#if PACKETVER >= 20131016
// new packets
// changed packet sizes
	packet(0x09d9,6,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
#endif

// 2013-10-23aRagexe
#if PACKETVER >= 20131023
// new packets
	packet(0x09db,-1); // ZC_NOTIFY_MOVEENTRY10
	packet(0x09dc,-1); // ZC_NOTIFY_NEWENTRY10
	packet(0x09dd,-1); // ZC_NOTIFY_STANDENTRY10
// changed packet sizes
	packet(0x09d9,4,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
#endif

// 2013-10-30aRagexe
#if PACKETVER >= 20131030
// new packets
	packet(0x09de,-1); // ZC_WHISPER02
	packet(0x09df,7); // ZC_ACK_WHISPER02
	packet(0x09e0,-1); // SC_LOGIN_ANSWER_WITH_ID
#endif

// 2013-11-06aRagexe
#if PACKETVER >= 20131106
// new packets
	packet(0x09e1,8,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_BODY_TO_GUILDSTORAGE
	packet(0x09e2,8,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_GUILDSTORAGE_TO_BODY
	packet(0x09e3,8,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_CART_TO_GUILDSTORAGE
	packet(0x09e4,8,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_GUILDSTORAGE_TO_CART
// changed packet sizes
#endif

// 2013-11-20dRagexe
#if PACKETVER >= 20131120
// new packets
	packet(0x09e5,14); // ZC_DELETEITEM_FROM_MCSTORE2
	packet(0x09e6,18); // ZC_UPDATE_ITEM_FROM_BUYING_STORE2
// changed packet sizes
#endif

// 2013-11-27bRagexe
#if PACKETVER >= 20131127
// new packets
// changed packet sizes
	packet(0x09e5,18); // ZC_DELETEITEM_FROM_MCSTORE2
	packet(0x09e6,22); // ZC_UPDATE_ITEM_FROM_BUYING_STORE2
#endif

// 2013-12-11dRagexe
#if PACKETVER >= 20131211
// new packets
	packet(0x09e7,2); // ZC_NOTIFY_UNREAD_RODEX
	packet(0x09e8,18,clif->pDull/*,XXX*/); // CZ_OPEN_RODEXBOX
	packet(0x09e9,2,clif->pRodexCloseMailbox); // CZ_CLOSE_RODEXBOX
	packet(0x09ed,-1); // ZC_ACK_SEND_RODEX
	packet(0x09ee,-1,clif->pDull/*,XXX*/); // CZ_REQ_NEXT_RODEX
// changed packet sizes
#endif

// 2013-12-18bRagexe - Yommy
#if PACKETVER >= 20131218
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0947,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x022D,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x092F,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x08AB,26,clif->pFriendsListAdd,2);
	packet(0x0811,5,clif->pHomMenu,2,4);
	packet(0x085C,36,clif->pStoragePassword,0);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x087b,4,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-12-18bRagexe
#if PACKETVER >= 20131218
// new packets
	packet(0x09ea,10,clif->pDull/*,XXX*/); // CZ_REQ_READ_RODEX
	packet(0x09eb,14); // ZC_ACK_READ_RODEX
	packet(0x09ef,11,clif->pRodexRefreshMaillist); // CZ_REQ_REFRESH_RODEX
	packet(0x09f0,-1); // ZC_ACK_RODEX_LIST
	packet(0x09f5,11,clif->pRodexDeleteMail); // CZ_REQ_DELETE_RODEX
	packet(0x09f6,11); // ZC_ACK_DELETE_RODEX
// changed packet sizes
	packet(0x09e8,10,clif->pDull/*,XXX*/); // CZ_OPEN_RODEXBOX
	packet(0x09ee,11,clif->pRodexNextMaillist); // CZ_REQ_NEXT_RODEX
#endif

// 2013-12-23cRagexe - Yommy
#if PACKETVER >= 20131223
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0202,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x08A4,36,clif->pStoragePassword,0);
	packet(0x0363,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,4,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-12-23bRagexe
#if PACKETVER >= 20131223
// new packets
// changed packet sizes
	packet(0x09ea,11,clif->pRodexReadMail); // CZ_REQ_READ_RODEX
	packet(0x09eb,24); // ZC_ACK_READ_RODEX
#endif

// 2013-12-30aRagexe - Yommy
#if PACKETVER >= 20131230
	packet(0x0871,7,clif->pActionRequest,2,6);
	packet(0x02C4,10,clif->pUseSkillToId,2,4,6);
	packet(0x035F,5,clif->pWalkToXY,2);
	packet(0x0438,6,clif->pTickSend,2);
	packet(0x094A,5,clif->pChangeDir,2,4);
	packet(0x092A,6,clif->pTakeItem,2);
	packet(0x0860,6,clif->pDropItem,2,4);
	packet(0x0968,8,clif->pMoveToKafra,2,4);
	packet(0x0895,8,clif->pMoveFromKafra,2,4);
	packet(0x091E,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x096A,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0926,6,clif->pGetCharNameRequest,2);
	packet(0x0898,6,clif->pSolveCharName,2);
	packet(0x087B,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0369,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x093D,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x087F,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0969,6,clif->pReqClickBuyingStore,2);
	packet(0x094C,2,clif->pReqCloseBuyingStore,0);
	packet(0x0365,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x091F,18,clif->pPartyBookingRegisterReq,2,4);
	packet(0x022D,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x089C,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x08A9,26,clif->pPartyInvite2,2);
	packet(0x0943,26,clif->pFriendsListAdd,2);
	packet(0x0949,5,clif->pHomMenu,2,4);
	packet(0x091D,36,clif->pStoragePassword,0);
	packet(0x087e,4,clif->pDull); // CZ_GANGSI_RANK
	packet(0x093e,8,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

// 2013-12-30aRagexe
#if PACKETVER >= 20131230
// new packets
	packet(0x09ec,-1,clif->pRodexSendMail); // CZ_REQ_SEND_RODEX
	packet(0x09ed,3); // ZC_ACK_SEND_RODEX
	packet(0x09f7,75); // ZC_PROPERTY_HOMUN_2
// changed packet sizes
	packet(0x09eb,23); // ZC_ACK_READ_RODEX
#endif

// 2014 Packet Data

// 2014-01-08cRagexe
#if PACKETVER == 20140108
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0936,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-01-15eRagexe
#if PACKETVER == 20140115
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2); // CZ_REQUEST_MOVE2
	packet(0x0360,6,clif->pReqClickBuyingStore,2); // CZ_REQUEST_TIME2
	packet(0x0361,6,clif->pDropItem,2,4); // CZ_CHANGE_DIRECTION2
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10); // CZ_USE_SKILL_TOGROUND2
	packet(0x0367,8,clif->pMoveFromKafra,2,4); // CZ_USE_SKILL_TOGROUND_WITHTALKBOX2
	packet(0x0368,6,clif->pSolveCharName,2); // CZ_REQNAME2
	packet(0x0369,7,clif->pActionRequest,2,6); // CZ_REQNAME_BYGID2
	packet(0x0437,5,clif->pWalkToXY,2); // CZ_REQUEST_ACT2
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8); // CZ_USE_SKILL2
	packet(0x0802,6,clif->pGetCharNameRequest,2); // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89); // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0); // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15); // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0); // CZ_SEARCH_STORE_INFO
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10); // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083c,10,clif->pUseSkillToId,2,4,6); // CZ_SSILIST_ITEM_CLICK
	packet(0x0865,36,clif->pStoragePassword,0); // ZC_REASSEMBLY_AUTH12
	packet(0x0887,-1,clif->pReqTradeBuyingStore,2,4,8,12); // CZ_REASSEMBLY_AUTH04
	packet(0x088a,8,clif->pDull/*,XXX*/); // CZ_REASSEMBLY_AUTH07
	packet(0x088e,8,clif->pMoveToKafra,2,4); // CZ_REASSEMBLY_AUTH11
	packet(0x089b,26,clif->pFriendsListAdd,2); // CZ_REASSEMBLY_AUTH24
	packet(0x08a7,5,clif->pChangeDir,2,4); // CZ_REASSEMBLY_AUTH36
	packet(0x092d,5,clif->pHomMenu,2,4); // ZC_REASSEMBLY_AUTH65
	packet(0x0940,6,clif->pTakeItem,2); // ZC_REASSEMBLY_AUTH84
	packet(0x095b,4,clif->pDull/*,XXX*/); // CZ_REASSEMBLY_AUTH69
	packet(0x095d,26,clif->pPartyInvite2,2); // CZ_REASSEMBLY_AUTH71
	packet(0x0965,-1,clif->pItemListWindowSelected,2,4,8); // CZ_REASSEMBLY_AUTH79
	packet(0x0966,19,clif->pWantToConnection,2,6,10,14,18); // CZ_REASSEMBLY_AUTH80
	packet(0x096a,18,clif->pPartyBookingRegisterReq,2,4); // CZ_REASSEMBLY_AUTH84
#endif

// 2014-01-15cRagexeRE
#if PACKETVER >= 20140115
// new packets
	packet(0x09f1,10,clif->pDull/*,XXX*/); // CZ_REQ_ZENY_FROM_RODEX
	packet(0x09f2,3); // ZC_ACK_ZENY_FROM_RODEX
	packet(0x09f3,15,clif->pDull/*,XXX*/); // CZ_REQ_ITEM_FROM_RODEX
	packet(0x09f4,12); // ZC_ACK_ITEM_FROM_RODEX
	packet(0x09f8,-1); // ZC_ALL_QUEST_LIST3
	packet(0x09f9,131); // ZC_ADD_QUEST_EX
	packet(0x09fa,-1); // ZC_UPDATE_MISSION_HUNT_EX
// changed packet sizes
	packet(0x09eb,-1); // ZC_ACK_READ_RODEX
#endif

// 2014-01-22aRagexe
#if PACKETVER == 20140122
// shuffle packets
	packet(0x0360,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x07ec,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0811,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0863,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0870,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0871,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0872,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088c,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0890,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0893,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0899,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x089d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08a2,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08aa,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0917,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0925,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x092f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0940,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0941,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0942,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x094c,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0950,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0952,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0955,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0957,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x095d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

// 2014-01-22aRagexeRE
#if PACKETVER >= 20140122
// new packets
	packet(0x09fb,-1,clif->pDull/*,XXX*/); // CZ_PET_EVOLUTION
	packet(0x09fc,6); // ZC_PET_EVOLUTION_RESULT
	packet(0x09fd,-1); // ZC_NOTIFY_MOVEENTRY11
	packet(0x09fe,-1); // ZC_NOTIFY_NEWENTRY11
	packet(0x09ff,-1); // ZC_NOTIFY_STANDENTRY11
// changed packet sizes
	packet(0x09f9,143); // ZC_ADD_QUEST_EX
#endif

// 2014-01-29aRagexe
#if PACKETVER == 20140129
// shuffle packets
	packet(0x0281,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0367,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0802,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0884,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0885,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0889,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0921,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0924,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x092c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0958,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0961,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-01-29bRagexeRE
#if PACKETVER >= 20140129
// new packets
	packet(0x0a00,269); // ZC_SHORTCUT_KEY_LIST_V3
	packet(0x0a01,3,clif->pHotkeyRowShift,2); // CZ_SHORTCUTKEYBAR_ROTATE
// Warning hercules using this packets for items manipulation. In RagexeRE from 20140129 and before 20140305, this actions broken.
#ifdef PACKETVER_RE
// changed packet sizes
	packet(0x01c4,43); // ZC_ADD_ITEM_TO_STORE2
	packet(0x01c5,43); // ZC_ADD_ITEM_TO_CART2
	packet(0x080f,41); // ZC_ADD_EXCHANGE_ITEM2
	packet(0x0990,52); // ZC_ITEM_PICKUP_ACK_V5
#endif  // PACKETVER_RE
#endif

// 2014-02-05bRagexe
#if PACKETVER == 20140205
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0938,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-02-12aRagexe
#if PACKETVER == 20140212
// shuffle packets
	packet(0x02c4,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0369,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0438,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x086e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0874,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0877,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0878,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x087e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0888,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x088c,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x08a0,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a1,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a7,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08ac,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08ad,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0919,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0928,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0930,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0934,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0936,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0944,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0952,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0953,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0960,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
#endif

// 2014-02-12aRagexeRE
#if PACKETVER >= 20140212
// new packets
	packet(0x0a02,4); // ZC_DRESSROOM_OPEN
// changed packet sizes
	packet(0x09e8,11,clif->pRodexOpenMailbox); // CZ_OPEN_RODEXBOX
#endif

// 2014-02-19aRagexe
#if PACKETVER == 20140219
// shuffle packets
	packet(0x0202,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0360,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0364,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0802,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0838,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x085d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x085f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0860,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0868,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x086f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0889,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0897,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0898,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x089f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a6,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08aa,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08ac,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0921,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0927,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0939,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0946,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0949,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0953,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x095a,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0961,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
#endif

// 2014-02-19aRagexeRE
#if PACKETVER >= 20140219
// Warning hercules using this packets for items manipulation. In RagexeRE from 20140129 and before 20140305, this actions broken.
#ifdef PACKETVER_RE
// changed packet sizes
	packet(0x01c4,53); // ZC_ADD_ITEM_TO_STORE2
	packet(0x01c5,53); // ZC_ADD_ITEM_TO_CART2
	packet(0x080f,51); // ZC_ADD_EXCHANGE_ITEM2
	packet(0x0990,62); // ZC_ITEM_PICKUP_ACK_V5
#endif  // PACKETVER_RE
#endif

// 2014-02-26aRagexe
#if PACKETVER == 20140226
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0362,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0364,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0877,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0887,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0894,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0895,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x091a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0921,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0931,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0941,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0962,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0964,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0969,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-02-26aRagexeRE
#if PACKETVER >= 20140226
// new packets
	packet(0x0a03,14,clif->pDull/*,XXX*/); // CZ_REQ_CANCEL_WRITE_RODEX
	packet(0x0a04,11,clif->pDull/*,XXX*/); // CZ_REQ_ADD_ITEM_RODEX
	packet(0x0a05,6); // ZC_ACK_ADD_ITEM_RODEX
	packet(0x0a06,5,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_RODEX_ITEM
// changed packet sizes
#endif

// 2014-03-05aRagexe
#if PACKETVER == 20140305
// shuffle packets
	packet(0x0202,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x07e4,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0878,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0934,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095e,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-03-05aRagexeRE
#if PACKETVER >= 20140305
// new packets
	packet(0x0a07,4); // ZC_ACK_REMOVE_RODEX_ITEM
	packet(0x0a08,5,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_WRITE_RODEX
	packet(0x0a09,50); // ZC_ADD_EXCHANGE_ITEM3
	packet(0x0a0a,52); // ZC_ADD_ITEM_TO_STORE3
	packet(0x0a0b,52); // ZC_ADD_ITEM_TO_CART3
	packet(0x0a0c,61); // ZC_ITEM_PICKUP_ACK_V6
	packet(0x0a0d,4); // ZC_INVENTORY_ITEMLIST_EQUIP_V6
// changed packet sizes
#ifdef PACKETVER_RE
	packet(0x01c4,22); // ZC_ADD_ITEM_TO_STORE2
	packet(0x01c5,22); // ZC_ADD_ITEM_TO_CART2
	packet(0x080f,20); // ZC_ADD_EXCHANGE_ITEM2
	packet(0x0990,31); // ZC_ITEM_PICKUP_ACK_V5
#endif  // PACKETVER_RE
	packet(0x09f3,10,clif->pDull/*,XXX*/); // CZ_REQ_ITEM_FROM_RODEX
	packet(0x09f4,3); // ZC_ACK_ITEM_FROM_RODEX
#endif

// 2014-03-12dRagexe
#if PACKETVER == 20140312
// shuffle packets
	packet(0x0202,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x023b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0366,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x086f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0889,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x088d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0891,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0894,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x089b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x089e,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a6,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a9,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08ad,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x091b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x091c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x091e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x092a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0948,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x094b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x094c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0957,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x095d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x095e,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0966,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2014-03-12bRagexeRE
#if PACKETVER >= 20140312
// new packets
	packet(0x0a0e,14); // ZC_BATTLEFIELD_NOTIFY_HP2
// changed packet sizes
	packet(0x0a09,45); // ZC_ADD_EXCHANGE_ITEM3
	packet(0x0a0a,47); // ZC_ADD_ITEM_TO_STORE3
	packet(0x0a0b,47); // ZC_ADD_ITEM_TO_CART3
	packet(0x0a0c,56); // ZC_ITEM_PICKUP_ACK_V6
	packet(0x0a0d,-1); // ZC_INVENTORY_ITEMLIST_EQUIP_V6
#endif

// 2014-03-26aRagexe
#if PACKETVER == 20140326
// shuffle packets
	packet(0x0362,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0365,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x07ec,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x083c,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0865,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0867,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0869,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x086b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x087c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x087e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0887,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0898,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08aa,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08ac,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08ad,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0918,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0928,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x092a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093d,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0942,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0945,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0946,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0956,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0959,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x095a,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x095c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0969,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

// 2014-03-26cRagexeRE
#if PACKETVER >= 20140326
// changed packet sizes
	packet(0x09f1,11,clif->pRodexRequestZeny); // CZ_REQ_ZENY_FROM_RODEX
	packet(0x09f2,4); // ZC_ACK_ZENY_FROM_RODEX
	packet(0x09f3,11,clif->pRodexRequestItems); // CZ_REQ_ITEM_FROM_RODEX
	packet(0x09f4,4); // ZC_ACK_ITEM_FROM_RODEX
	packet(0x0a03,2,clif->pRodexCancelWriteMail); // CZ_REQ_CANCEL_WRITE_RODEX
	packet(0x0a07,6); // ZC_ACK_REMOVE_RODEX_ITEM
	packet(0x0a08,7,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_WRITE_RODEX
#endif

// 2014-04-02fRagexe
#if PACKETVER == 20140402
// shuffle packets
	packet(0x023b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0360,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0364,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x085b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x085d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0867,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0868,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0882,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0883,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x088c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0890,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0896,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x089a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08ac,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0920,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0926,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0933,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x093f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0944,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0946,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x094c,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0950,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0958,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0965,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

// 2014-04-02eRagexeRE
#if PACKETVER >= 20140402
// new packets
	packet(0x0a0f,-1); // ZC_CART_ITEMLIST_EQUIP_V6
	packet(0x0a10,-1); // ZC_STORE_ITEMLIST_EQUIP_V6
	packet(0x0a11,-1); // ZC_GUILDSTORAGE_ITEMLIST_EQUIP_V6
// changed packet sizes
#endif

// 2014-04-09aRagexe
#if PACKETVER == 20140409
// shuffle packets
	packet(0x0819,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x085b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0868,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x086a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x086d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0873,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0875,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x087e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0883,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0884,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x088a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0890,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0893,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0896,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0897,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0899,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a2,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a4,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08a6,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x08a7,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a9,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0918,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x092e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0942,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0947,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x095a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x095e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
#endif

// 2014-04-09aRagexeRE
#if PACKETVER >= 20140409
// changed packet sizes
	packet(0x09f2,12); // ZC_ACK_ZENY_FROM_RODEX
	packet(0x09f4,12); // ZC_ACK_ITEM_FROM_RODEX
#endif

// 2014-04-16aRagexe
#if PACKETVER == 20140416
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x095c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-04-16aRagexeRE
#if PACKETVER >= 20140416
// new packets
	packet(0x0a04,6,clif->pRodexAddItem); // CZ_REQ_ADD_ITEM_RODEX
	packet(0x0a12,27); // ZC_ACK_OPEN_WRITE_RODEX
	packet(0x0a13,2,clif->pRodexCheckName); // CZ_CHECK_RECEIVE_CHARACTER_NAME
// changed packet sizes
	packet(0x0a05,48); // ZC_ACK_ADD_ITEM_RODEX
	packet(0x0a06,6,clif->pRodexRemoveItem); // CZ_REQ_REMOVE_RODEX_ITEM
	packet(0x0a07,7); // ZC_ACK_REMOVE_RODEX_ITEM
	packet(0x0a08,26,clif->pRodexOpenWriteMail); // CZ_REQ_OPEN_WRITE_RODEX
#endif

// 2014-04-23aRagexe
#if PACKETVER == 20140423
// shuffle packets
	packet(0x022d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0360,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0436,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0811,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x083c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x085a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x085b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0862,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0863,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0866,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x086f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0873,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0890,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0895,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0896,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0897,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0898,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x089d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a8,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08ad,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x091a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0920,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x094f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x095e,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
#endif

// 2014-04-23aRagexeRE
#if PACKETVER >= 20140423
// new packets
	packet(0x0a14,6); // ZC_CHECK_RECEIVE_CHARACTER_NAME
// changed packet sizes
	packet(0x0a13,26,clif->pRodexCheckName); // CZ_CHECK_RECEIVE_CHARACTER_NAME
#endif

// 2014-04-30aRagexeRE
#if PACKETVER >= 20140430
// new packets
	packet(0x0a15,11); // ZC_GOLDPCCAFE_POINT
	packet(0x0a16,26,clif->pDull/*,XXX*/); // CZ_DYNAMICNPC_CREATE_REQUEST
	packet(0x0a17,6); // ZC_DYNAMICNPC_CREATE_RESULT
#endif

// 2014-05-08aRagexe
#if PACKETVER == 20140508
// shuffle packets
	packet(0x0202,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x022d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x023b,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0281,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x02c4,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x035f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0360,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0361,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0362,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0363,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0364,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0365,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0366,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0367,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0369,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0438,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x07e4,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x07ec,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0802,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0811,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0815,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0817,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0819,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0835,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0838,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083c,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

// 2014-05-08bRagexeRE
#if PACKETVER >= 20140508
// changed packet sizes
	packet(0x0a15,12); // ZC_GOLDPCCAFE_POINT
#endif

// 2014-05-14bRagexe
#if PACKETVER == 20140514
// shuffle packets
	packet(0x0437,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0817,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0865,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0867,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0868,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0876,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0877,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0885,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0886,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x088b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0895,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089a,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x089c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a5,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0918,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0921,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0925,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x092c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x092f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x094d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0958,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0962,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0965,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x096a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
#endif

// 2014-05-21bRagexe
#if PACKETVER == 20140521
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x088d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x089c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08ac,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0968,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-05-21aRagexeRE
#if PACKETVER >= 20140521
// changed packet sizes
	packet(0x0a07,9); // ZC_ACK_REMOVE_RODEX_ITEM
	packet(0x0a14,10); // ZC_CHECK_RECEIVE_CHARACTER_NAME
#endif

// 2014-05-28aRagexe
#if PACKETVER == 20140528
// shuffle packets
	packet(0x0202,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0360,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x085f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0862,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0872,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0875,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0877,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0879,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x087e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088a,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x088f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0894,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0896,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a4,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08a8,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x08ab,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x091d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0929,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0930,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0938,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x093a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x093f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x094a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x095f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0963,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0964,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0966,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
#endif

// 2014-06-05aRagexe
#if PACKETVER == 20140605
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0369,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0921,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0931,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x094c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

/* Roulette System [Yommy/Hercules] */
// 2014-06-05aRagexe
#if PACKETVER >= 20140605
// new packets
	packet(0x0a18,2); // ZC_ACCEPT_ENTER3
	packet(0x0a19,-1,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_ROULETTE
	packet(0x0a1a,10); // ZC_ACK_OPEN_ROULETTE
	packet(0x0A1B,2,clif->pRouletteInfo,0);     // HEADER_CZ_REQ_ROULETTE_INFO
	packet(0x0a1c,6); // ZC_ACK_ROULEITTE_INFO
	packet(0x0a1d,14,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_ROULETTE
#endif

// 2014-06-11cRagexe
#if PACKETVER == 20140611
// shuffle packets
	packet(0x0364,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0438,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0838,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0864,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0867,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x086c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0874,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0878,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x088c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0891,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0893,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0894,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a1,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a2,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0924,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0936,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x094f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0950,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0951,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0952,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0957,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0958,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0963,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0965,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0969,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

/* Roulette System [Yommy/Hercules] */
// 2014-06-11bRagexe / RE. moved by 4144
#if PACKETVER >= 20140611
// new packets
	packet(0x0a1e,3); // ZC_ACK_CLOSE_ROULETTE
	packet(0x0a1f,2,clif->pRouletteGenerate,0); // CZ_REQ_GENERATE_ROULETTE
	packet(0x0a20,21); // ZC_ACK_GENERATE_ROULETTE
	packet(0x0a21,6,clif->pDull/*,XXX*/); // CZ_RECV_ROULETTE_ITEM
	packet(0x0a22,3); // ZC_RECV_ROULETTE_ITEM
	packet(0x0a23,-1); // ZC_ALL_ACH_LIST
	packet(0x0a24,35); // ZC_ACH_UPDATE
	packet(0x0a25,6,clif->pDull/*,XXX*/); // CZ_REQ_ACH_REWARD
	packet(0x0a26,7); // ZC_REQ_ACH_REWARD_ACK
// changed packet sizes
	packet(0x0a18,14); // ZC_ACCEPT_ENTER3
	packet(0x0a19,2,clif->pRouletteOpen,0); // CZ_REQ_OPEN_ROULETTE
	packet(0x0a1a,23); // ZC_ACK_OPEN_ROULETTE
	packet(0x0a1c,-1); // ZC_ACK_ROULEITTE_INFO
	packet(0x0a1d,2,clif->pRouletteClose,0); // CZ_REQ_CLOSE_ROULETTE
#endif

// 2014-06-12aRagexe
#if PACKETVER == 20140612
// shuffle packets
	packet(0x0364,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0438,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0838,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0864,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0867,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x086c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0874,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0878,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x088c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0891,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0893,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0894,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a1,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a2,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0924,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0936,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x094f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0950,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0951,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0952,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0957,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0958,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0963,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0965,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0969,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

// 2014-06-13aRagexe
#if PACKETVER == 20140613
// shuffle packets
	packet(0x0364,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0438,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0838,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0864,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0867,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x086c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0874,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0878,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x088c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0891,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0893,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0894,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a1,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a2,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0924,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0936,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x094f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0950,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0951,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0952,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0957,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0958,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0963,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0965,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0969,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

// 2014-06-18aRagexe
#if PACKETVER == 20140618
// shuffle packets
	packet(0x085d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0860,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0861,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x086c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0878,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0884,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0885,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0886,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0890,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0892,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a6,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a7,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08ac,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0917,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x091f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0929,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0935,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0938,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0939,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x093b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0945,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0954,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0957,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x095e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0962,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0967,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
#endif

// 2014-06-18cRagexeRE
#if PACKETVER >= 20140618
// changed packet sizes
	packet(0x0a21,3,clif->pRouletteRecvItem,2); // CZ_RECV_ROULETTE_ITEM
	packet(0x0a22,5); // ZC_RECV_ROULETTE_ITEM
#endif

// 2014-06-25aRagexe
#if PACKETVER == 20140625
// shuffle packets
	packet(0x0202,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x023b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0815,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0817,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0835,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x085a,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0861,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0875,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0885,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0886,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0888,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x088a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0897,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a1,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a2,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0923,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0928,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0940,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0946,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0959,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0960,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0968,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0969,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-06-25aRagexeRE
#if PACKETVER >= 20140625
// new packets
	packet(0x0a27,8); // ZC_RECOVERY2
	packet(0x0a28,3); // ZC_ACK_OPENSTORE2
// changed packet sizes
	packet(0x0a24,36); // ZC_ACH_UPDATE
#endif

// 2014-07-02aRagexe
#if PACKETVER == 20140702
// shuffle packets
	packet(0x022d,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x023b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0364,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0438,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x07e4,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0835,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x086c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0887,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0892,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0895,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08a0,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a2,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0925,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0933,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0940,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-07-02aRagexeRE
#if PACKETVER >= 20140702
// new packets
	packet(0x0a29,6); // ZC_REQ_AU_BOT
	packet(0x0a2a,6,clif->pDull/*,XXX*/); // CZ_ACK_AU_BOT
#endif

// 2014-07-09aRagexe
#if PACKETVER == 20140709
// shuffle packets
	packet(0x0364,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0437,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0860,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0866,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0869,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0875,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0877,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0879,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x087a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0887,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0888,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0894,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0897,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0898,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08ad,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0925,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x092f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0931,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0934,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0939,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x093f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0940,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x094d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x094f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0961,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
#endif

// 2014-07-16aRagexe
#if PACKETVER == 20140716
// shuffle packets
	packet(0x0362,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0811,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085c,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0868,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0871,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0881,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x088d,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0896,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x089a,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x089f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08a2,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a4,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x08ac,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0918,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x091f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0926,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x092c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0938,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0947,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0952,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0958,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0959,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0969,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
#endif

// 2014-07-16aRagexeRE
#if PACKETVER >= 20140716
// changed packet sizes
	packet(0x09e7,3); // ZC_NOTIFY_UNREAD_RODEX
#endif

// 2014-07-23aRagexe
#if PACKETVER == 20140723
// shuffle packets
	packet(0x02c4,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0364,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0368,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0436,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0819,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0838,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x085f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0869,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086d,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x087d,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0888,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0891,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0896,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0898,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x089e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08a2,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08ad,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0927,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x092f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0934,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0935,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0939,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x093d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0945,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0947,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0948,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0960,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2014-07-23aRagexeRE
#if PACKETVER >= 20140723
// new packets
	packet(0x0a2b,14); // ZC_SE_CASHSHOP_OPEN2
	packet(0x0a2c,12); // ZC_SE_PC_BUY_TAIWANCASHITEM_RESULT
// changed packet sizes
	packet(0x0a24,56); // ZC_ACH_UPDATE
#endif

// 2014-07-30aRagexe
#if PACKETVER == 20140730
// shuffle packets
	packet(0x022d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0364,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0366,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0367,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0437,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x07ec,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0802,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0815,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0817,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x085f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x087d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x087e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x087f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0889,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x088b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0892,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a0,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a6,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a7,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a9,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08ad,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0924,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x092a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0934,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0940,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0946,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
#endif

// 2014-08-06aRagexe
#if PACKETVER == 20140806
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0948,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-08-13aRagexe
#if PACKETVER == 20140813
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0802,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0868,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0878,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0882,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0895,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0897,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0899,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a3,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a7,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08ab,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0967,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-08-14aRagexe
#if PACKETVER == 20140814
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0802,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0868,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0878,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0882,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0895,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0897,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0899,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a3,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a7,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08ab,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0967,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-08-20aRagexe
#if PACKETVER == 20140820
// shuffle packets
	packet(0x035f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0835,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0861,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0864,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0869,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x086c,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x086e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0872,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0876,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0891,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0899,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089a,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x089b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a3,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a7,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x091d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x092f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0936,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0937,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x093a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0951,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0952,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0956,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0958,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0961,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2014-08-20aRagexeRE
#if PACKETVER >= 20140820
// new packets
	packet(0x0a2d,-1); // ZC_EQUIPWIN_MICROSCOPE_V6
#endif

// 2014-08-27aRagexe
#if PACKETVER == 20140827
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0943,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-09-03aRagexe
#if PACKETVER == 20140903
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x089b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0931,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0941,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0943,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0945,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-09-03aRagexeRE
#if PACKETVER >= 20140903
// new packets
	packet(0x0a2e,6,clif->pDull/*,XXX*/); // CZ_REQ_CHANGE_TITLE
	packet(0x0a2f,7); // ZC_ACK_CHANGE_TITLE
// changed packet sizes
#endif

// 2014-09-17aRagexe
#if PACKETVER == 20140917
// shuffle packets
	packet(0x022d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0364,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0365,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0366,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0367,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0369,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0864,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086d,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0889,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0895,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0897,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0898,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089c,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a8,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0919,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x091e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x092a,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0930,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0949,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x094f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0951,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0955,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0956,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0957,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x095a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x095c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x095e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0966,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
#endif

// 2014-09-24cRagexe
#if PACKETVER == 20140924
// shuffle packets
	packet(0x0366,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0367,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x07e4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0802,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0815,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0862,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0864,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0865,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0867,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x086b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x086d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0886,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x088b,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0894,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0898,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x089c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a5,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a7,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0918,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0925,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0926,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0928,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x092d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0934,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0949,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0952,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
#endif

// 2014-09-24bRagexeRE
#if PACKETVER >= 20140924
// new packets
	packet(0x0a30,106); // ZC_ACK_REQNAMEALL2
	packet(0x0a31,-1); // ZC_RESULT_PACKAGE_ITEM_TEST
	packet(0x0a32,2); // ZC_OPEN_RODEX_THROUGH_NPC_ONLY
	packet(0x0a33,7); // ZC_UPDATE_ROULETTE_COIN
	packet(0x0a34,6); // ZC_UPDATE_TAIWANCASH
#endif

// 2014-10-01aRagexe
#if PACKETVER == 20141001
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0365,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0884,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0885,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x089d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08ad,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x092a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0937,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0939,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093f,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0952,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
#endif

// 2014-10-01bRagexeRE
#if PACKETVER >= 20141001
// changed packet sizes
	packet(0x0a24,66); // ZC_ACH_UPDATE
#endif

// 2014-10-08aRagexe
#if PACKETVER == 20141008
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0942,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-10-08bRagexeRE
#if PACKETVER >= 20141008
// changed packet sizes
	packet(0x0a05,49); // ZC_ACK_ADD_ITEM_RODEX
#endif

// 2014-10-15bRagexe
#if PACKETVER == 20141015
// shuffle packets
	packet(0x022d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0922,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0936,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0967,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-10-16aRagexe
#if PACKETVER == 20141016
// shuffle packets
	packet(0x022d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0922,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0936,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0967,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-10-22bRagexe
#if PACKETVER == 20141022
// shuffle packets
	packet(0x023b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0878,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x087d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0896,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0899,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08aa,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08ab,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08ad,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x094e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0955,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-10-29aRagexe
#if PACKETVER == 20141029
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0940,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-11-05aRagexe
#if PACKETVER == 20141105
// shuffle packets
	packet(0x022d,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x035f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0360,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x085c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0863,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0864,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0865,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0871,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0874,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0875,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0877,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0879,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0887,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0892,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0898,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a0,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08a5,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a7,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08ad,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x091e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x092b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x093e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0944,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0948,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0950,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0957,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x095f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0968,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2014-11-12cRagexe
#if PACKETVER == 20141112
// shuffle packets
	packet(0x0362,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0438,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x07e4,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0835,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0838,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085f,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0863,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0871,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0885,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0886,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0887,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088d,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08a0,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a1,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08ab,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0919,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0926,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0929,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0943,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0955,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x095d,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0960,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0962,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
#endif

// 2014-11-19dRagexe
#if PACKETVER == 20141119
// shuffle packets
	packet(0x0202,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0861,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0865,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0866,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0872,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0873,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0875,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x087c,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0885,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0887,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0888,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x088d,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0895,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a8,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08aa,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0918,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0920,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0921,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0929,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0933,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0938,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0940,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0941,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0942,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0948,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x094c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0963,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
#endif

// 2014-11-19bRagexeRE
#if PACKETVER >= 20141119
// new packets
	packet(0x0A35,4,clif->pOneClick_ItemIdentify,2);
// changed packet sizes
	packet(0x0a05,53); // ZC_ACK_ADD_ITEM_RODEX
#endif

// 2014-11-26aRagexe
#if PACKETVER == 20141126
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0367,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0871,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0884,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0896,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a4,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08ad,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0920,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0942,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0965,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-11-26aRagexeRE
#if PACKETVER >= 20141126
// new packets
	packet(0x0a36,7); // ZC_HP_INFO_TINY
	packet(0x0a37,57); // ZC_ITEM_PICKUP_ACK_V7
#endif

// 2014-12-03aRagexe
#if PACKETVER == 20141203
// shuffle packets
	packet(0x0202,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0281,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0362,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0367,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0802,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0861,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x086d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x086e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x087b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x087e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0880,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0889,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0898,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08a5,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08aa,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0917,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x091c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0928,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x092a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0936,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0952,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0957,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x095c,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0962,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

// 2014-12-10bRagexe
#if PACKETVER == 20141210
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0885,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08ac,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0917,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0927,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x092b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0947,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0954,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0955,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0958,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0961,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0963,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0967,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2014-12-24aRagexe
#if PACKETVER == 20141224
// shuffle packets
	packet(0x0361,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0438,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0835,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x085a,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0865,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0867,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x086c,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0870,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x087b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x089b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a3,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a8,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08ac,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0930,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0932,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0945,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0946,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0949,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x094f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0950,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0953,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0956,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x095b,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x095f,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
#endif

// 2014-12-31aRagexe
#if PACKETVER == 20141231
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-01-07aRagexeRE
#if PACKETVER == 20150107
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0895,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0943,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0947,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-01-14aRagexe
#if PACKETVER == 20150114
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0868,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0899,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0946,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0955,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0957,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-01-21aRagexe
#if PACKETVER == 20150121
// shuffle packets
	packet(0x0281,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x088b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x089e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08ab,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0918,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0919,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0955,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0959,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0963,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0967,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-01-28aRagexe
#if PACKETVER == 20150128
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x023b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x035f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0368,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0838,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085a,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0864,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x086d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0870,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0874,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0875,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0876,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0888,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ab,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0927,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0929,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0938,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0944,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0952,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0963,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0968,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
#endif

// 2015-01-28aRagexeRE
#if PACKETVER >= 20150128
// new packets
	packet(0x0a38,3);
#endif

// 2015-01-29aRagexe
#if PACKETVER == 20150129
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x023b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x035f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0368,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0838,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085a,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0864,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x086d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0870,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0874,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0875,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0876,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0888,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ab,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0927,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0929,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0938,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0944,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0952,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0963,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0968,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
#endif

// 2015-01-30aRagexe
#if PACKETVER == 20150130
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x023b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x035f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0368,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0838,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085a,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0864,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x086d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0870,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0874,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0875,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0876,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0888,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ab,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0927,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0929,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0938,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0944,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0952,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0963,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0968,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
#endif

// 2015-02-04aRagexe
#if PACKETVER == 20150204
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0966,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-02-11aRagexe
#if PACKETVER == 20150211
// shuffle packets
	packet(0x023b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0368,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0369,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0436,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0437,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x07e4,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0817,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0819,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0835,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0862,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0863,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0870,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0873,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x087b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x087f,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0882,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0883,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0885,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0886,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x089c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a0,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a4,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08aa,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0919,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0920,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0944,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0951,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0957,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0958,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2015-02-17aRagexe
#if PACKETVER == 20150217
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085b,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-02-25aRagexeRE
#if PACKETVER == 20150225
// shuffle packets
	packet(0x02c4,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0362,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0819,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0885,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0896,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x089c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0940,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0946,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0948,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0952,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0955,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
#endif

// 2015-02-26aRagexeRE
#if PACKETVER == 20150226
// shuffle packets
	packet(0x02c4,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0362,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0819,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0885,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0896,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x089c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0940,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0946,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0948,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0952,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0955,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
#endif

// 2015-03-04aRagexe
#if PACKETVER == 20150304
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0862,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x086d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0879,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0892,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x093a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0947,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x095d,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0960,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0961,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-03-11aRagexeRE
#if PACKETVER == 20150311
// shuffle packets
	packet(0x023b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0360,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0436,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0438,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0838,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x086a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x087b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0883,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0886,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0888,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0896,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a1,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a3,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a5,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08a6,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0928,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x092e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0943,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0946,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0957,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0958,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x095b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0963,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0964,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2015-03-11aRagexeRE
#if PACKETVER >= 20150311
// new packets
	packet(0x0a3a,12);
// changed packet sizes
#endif

// 2015-03-18aRagexe
#if PACKETVER == 20150318
// shuffle packets
	packet(0x0202,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x023b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0281,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0367,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x07e4,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0802,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0811,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0862,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0863,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0873,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0885,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0889,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x088c,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x089c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a4,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x091d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0920,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0927,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0928,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0936,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0937,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0938,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x093a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093c,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x094c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0951,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0958,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0959,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0960,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2015-03-25aRagexe
#if PACKETVER == 20150325
// shuffle packets
	packet(0x0202,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0363,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0365,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0438,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0802,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0819,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x086f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x087c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x087e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0883,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0885,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0891,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0893,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0897,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0899,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a1,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a7,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0919,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0931,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0932,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0938,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0947,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0950,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0954,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0969,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
#endif

// 2015-04-01aRagexe
#if PACKETVER == 20150401
// shuffle packets
	packet(0x0362,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0367,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0437,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x083c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x086f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0875,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x087e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x088f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0895,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0898,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x089c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a5,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x091c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0922,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0924,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0938,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0939,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x093a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x093b,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x093e,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0946,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0949,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094b,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0953,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0964,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2015-04-08aRagexe
#if PACKETVER == 20150408
// shuffle packets
	packet(0x0819,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x085a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0865,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0868,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x086b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x086e,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0878,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x087f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0888,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0889,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0891,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0898,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a2,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a4,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x091e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0922,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0946,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0955,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0957,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0959,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0963,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

// 2015-04-15aRagexe
#if PACKETVER == 20150415
// shuffle packets
	packet(0x0361,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0366,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0368,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0802,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0817,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0835,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x085e,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0863,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0867,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0868,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0869,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x086c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0880,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0891,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0898,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a0,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0922,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092e,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x093c,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x093e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0946,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0953,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x095c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0960,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0961,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2015-04-15aRagexeRE
#if PACKETVER >= 20150415
// changed packet sizes
	packet(0x0a39,36);  // CH_UNKNOWN_MAKE_CHAR  // in char server used from 20151001. is this correct?
#endif

// 2015-04-22aRagexeRE
#if PACKETVER == 20150422
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0955,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-04-22aRagexeRE
#if PACKETVER >= 20150422
// new packets
	packet(0x0a3b,-1);
// changed packet sizes
#endif

// 2015-04-29aRagexe
#if PACKETVER == 20150429
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x086a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0886,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0894,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0899,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x089f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a6,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08a8,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08ad,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0929,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0943,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-05-07bRagexe
#if PACKETVER == 20150507
// shuffle packets
	packet(0x023b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0864,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0887,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0889,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0924,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x092e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x093b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0941,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0942,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0953,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0955,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0958,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-05-13aRagexe
#if PACKETVER == 20150513
// shuffle packets
	packet(0x022d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x02c4,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0864,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0879,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0883,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0885,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a8,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0923,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0924,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0927,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x094a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0958,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0960,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-05-20aRagexe
#if PACKETVER == 20150520
// shuffle packets
	packet(0x0202,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0361,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0835,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x085e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0865,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0868,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x087d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0880,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0882,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x089c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089e,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a2,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08ad,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091d,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0924,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0931,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0936,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x093d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0940,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0945,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x094e,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x095b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0960,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0961,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2015-05-20aRagexeRE
#if PACKETVER >= 20150520
// new packets
	packet(0x0a3c,-1);
	packet(0x0a3d,18,clif->pDull/*,XXX*/);
#endif

// 2015-05-27aRagexe
#if PACKETVER == 20150527
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x083c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0940,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-06-03aRagexe
#if PACKETVER == 20150603
// shuffle packets
	packet(0x0361,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0437,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0811,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0819,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0860,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0864,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0867,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0873,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0877,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0881,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0884,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x088b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0897,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x089a,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x089d,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a1,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08ad,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0922,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x093b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x093f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0955,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0956,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0960,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0969,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x096a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
#endif

// 2015-06-03bRagexeRE
#if PACKETVER >= 20150603
// new packets
	packet(0x0a3e,-1);
#endif

// 2015-06-10aRagexe
#if PACKETVER == 20150610
// shuffle packets
	packet(0x022d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0438,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x07e4,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0835,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0870,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0872,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0877,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x087e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0884,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0885,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0888,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x088d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x088f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0897,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a0,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08ac,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0925,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092b,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x092c,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x092e,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0932,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x093e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0940,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0946,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0949,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0957,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x095d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0964,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2015-06-17aRagexeRE
#if PACKETVER == 20150617
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0365,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0811,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x086a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x086b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0870,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0886,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0894,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-06-18aRagexeRE
#if PACKETVER == 20150618
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0365,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0811,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x086a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x086b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0870,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0886,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0894,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-06-24aRagexe
#if PACKETVER == 20150624
// shuffle packets
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0365,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0870,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0940,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0941,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0966,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-06-24aRagexeRE
#if PACKETVER >= 20150624
// new packets
	packet(0x0a3f,9);
#endif

// 2015-07-02aRagexe
#if PACKETVER == 20150702
// shuffle packets
	packet(0x023b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x07e4,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0802,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x087d,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x087e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0883,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0893,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a0,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a4,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a5,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a6,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08ad,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0919,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0923,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0928,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x092c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093e,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x093f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0946,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094e,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0954,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0956,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0958,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x095f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0960,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0968,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2015-07-08cRagexe
#if PACKETVER == 20150708
// shuffle packets
	packet(0x022d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x02c4,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0368,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085e,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0872,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x087f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0884,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x089d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a5,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08ad,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x092a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0962,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-07-15aRagexe
#if PACKETVER == 20150715
// shuffle packets
	packet(0x023b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0362,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0364,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0436,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0437,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0438,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0835,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x083c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085c,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086f,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0873,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0879,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x087c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x087f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0886,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0895,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0896,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0897,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0899,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x089a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a4,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08ac,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0917,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x093e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0944,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0950,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0956,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0961,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0965,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2015-07-29aRagexe
#if PACKETVER == 20150729
// shuffle packets
	packet(0x0437,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0438,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x085b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0860,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x086e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x086f,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0870,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0880,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0881,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0886,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x089a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a3,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a4,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08ac,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08ad,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0920,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x092b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x092f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x093a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x093f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0955,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0961,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
#endif

// 2015-08-05aRagexe
#if PACKETVER == 20150805
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-08-12aRagexe
#if PACKETVER == 20150812
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087f,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-08-12aRagexeRE
#if PACKETVER >= 20150812
// new packets
	packet(0x0a40,11);
#endif

// 2015-08-19aRagexeRE
#if PACKETVER == 20150819
// shuffle packets
	packet(0x0202,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x022d,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0281,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x085d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0862,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0865,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0871,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0888,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0919,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091e,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0927,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0961,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0967,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-08-26aRagexeRE
#if PACKETVER == 20150826
// shuffle packets
	packet(0x0362,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0368,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0436,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x07ec,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0819,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0861,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0865,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x086b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0870,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x087b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x088b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0890,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0891,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a0,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08a1,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a8,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0924,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0928,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x092e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x093b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0945,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094f,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0951,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0959,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0964,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0968,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0969,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
#endif

// 2015-09-02aRagexe
#if PACKETVER == 20150902
// shuffle packets
	packet(0x023b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0360,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0367,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0802,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x083c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0863,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x087b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x087f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0886,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0887,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0889,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x088d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0892,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0897,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0899,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08a9,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0923,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0928,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092a,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x092d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0941,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0947,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x094f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0953,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x095b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0960,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

// 2015-09-09aRagexe
#if PACKETVER == 20150909
// shuffle packets
	packet(0x023b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0361,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0365,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0437,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0861,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0871,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x087b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0883,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0886,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0895,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0928,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0940,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0941,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095e,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0962,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2015-09-09aRagexeRE
#if PACKETVER >= 20150909
// new packets
	packet(0x0a41,18);
#endif

// 2015-09-16aRagexe
#if PACKETVER == 20150916
// shuffle packets
	packet(0x022d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0817,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0835,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0869,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0873,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0877,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x087f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0881,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x089b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x089c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08ac,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0920,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0924,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x092e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x092f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0934,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0936,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0938,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0941,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0942,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0948,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x095a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0960,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0961,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0969,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
#endif

// 2015-09-16aRagexeRE
#if PACKETVER >= 20150916
// new packets
	packet(0x0a42,43);
#endif

// 2015-09-23bRagexe
#if PACKETVER == 20150923
// shuffle packets
	packet(0x0361,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0817,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x085c,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x085d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0864,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x086e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x086f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0870,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0879,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x087f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0886,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x088e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0892,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0895,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a0,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a2,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08a5,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08a6,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x092b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0930,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0936,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x093b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0951,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0961,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
#endif

// 2015-10-01aRagexe
#if PACKETVER == 20151001
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0960,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-10-07aRagexeRE
#if PACKETVER == 20151007
// shuffle packets
	packet(0x0202,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0862,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0961,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0967,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-10-07aRagexeRE
#if PACKETVER >= 20151007
// new packets
	packet(0x0a43,85);
	packet(0x0a44,-1);
#endif

// 2015-10-14bRagexeRE
#if PACKETVER == 20151014
// shuffle packets
	packet(0x0202,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0817,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0838,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x085a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0860,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0863,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0867,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0872,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0874,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0881,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0883,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0884,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0889,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x088e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x089a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x089f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08aa,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x091d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0930,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0934,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0944,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x094f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0956,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x095e,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0961,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0964,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
#endif

// 2015-10-21aRagexe
#if PACKETVER == 20151021
// shuffle packets
	packet(0x023b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x02c4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-10-22aRagexe
#if PACKETVER == 20151022
// shuffle packets
	packet(0x023b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x02c4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-10-28cRagexeRE
#if PACKETVER == 20151028
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0860,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-10-28cRagexeRE
#if PACKETVER >= 20151028
// new packets
	packet(0x0a45,-1);
#endif

// 2015-10-29aRagexe
#if PACKETVER == 20151029
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0860,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-11-04aRagexe
#if PACKETVER == 20151104
// shuffle packets
	packet(0x023b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0360,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0363,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0437,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0811,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0815,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0886,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0887,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x088b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x088d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a3,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a5,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0928,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0939,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093a,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x0940,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x0964,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
#endif

// 2015-11-11aRagexe
#if PACKETVER == 20151111
// shuffle packets
	packet(0x02c4,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085d,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x0862,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0871,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0885,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x089c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0942,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x094a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0958,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0966,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0967,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0969,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-11-18aRagexeRE
#if PACKETVER == 20151118
// shuffle packets
	packet(0x022d,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x035f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x088b,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x08ab,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0921,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0925,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x092e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0943,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0946,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x0957,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095c,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-11-18aRagexeRE
#if PACKETVER >= 20151118
// new packets
	packet(0x0a49,22);
	packet(0x0a4a,6);
	packet(0x0a4b,22);
	packet(0x0a4c,28);
#endif

// 2015-11-25bRagexe
#if PACKETVER == 20151125
// shuffle packets
	packet(0x0361,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0365,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0366,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0368,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x0438,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0802,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0838,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0863,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0883,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0884,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x0885,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x088c,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0899,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x089f,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x08a9,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08ad,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0920,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x092a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x092e,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0939,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x093e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0951,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0956,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0957,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0959,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
#endif

// 2015-12-02bRagexeRE
#if PACKETVER == 20151202
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0870,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2015-12-09aRagexe
#if PACKETVER == 20151209
// shuffle packets
	packet(0x0365,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0369,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x07e4,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x07ec,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0811,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0819,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x085b,36,clif->pStoragePassword,2,4,20);  // CZ_ACK_STORE_PASSWORD
	packet(0x085d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085e,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0861,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0866,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0875,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x087a,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x087f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x088e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0894,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08a1,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0920,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x092d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0930,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0932,-1,clif->pItemListWindowSelected,2,4,8,12);  // CZ_ITEMLISTWIN_RES
	packet(0x093b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0948,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094a,18,clif->pPartyBookingRegisterReq,2,4,6);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0956,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x095c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0961,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0964,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
#endif

// 2015-12-16aRagexe
#if PACKETVER == 20151216
// shuffle packets
	packet(0x022d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0361,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0364,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0436,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x085b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0864,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0865,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x086a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x086e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0870,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0874,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0885,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089d,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x089e,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a2,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a9,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08ac,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0944,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0947,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0949,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0954,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0960,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0966,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0968,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

// 2015-12-23bRagexeRE
#if PACKETVER == 20151223
// shuffle packets
	packet(0x02c4,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0362,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0802,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0815,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0864,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0866,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086e,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0872,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0875,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0876,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0881,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0884,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0886,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x088d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0890,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0891,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0898,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08aa,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0918,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x091a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x091b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0920,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0923,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0924,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x095e,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x095f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0965,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0967,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

// 2015-12-30aRagexe
#if PACKETVER == 20151230
// shuffle packets
	packet(0x02c4,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x07ec,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085b,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0861,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0869,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0886,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x088e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0897,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0923,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0949,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-01-06aRagexeRE
#if PACKETVER == 20160106
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0861,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x086c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0878,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x087a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x087f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0885,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0889,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x088a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0891,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a0,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0940,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-01-13aRagexeRE
#if PACKETVER == 20160113
// shuffle packets
	packet(0x022d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x023b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0815,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x085b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0864,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x086d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0873,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0875,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0888,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088c,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0892,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0893,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0899,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a0,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a6,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08aa,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0919,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0924,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0930,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0932,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x093c,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0941,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x094d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0967,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

// 2016-01-20aRagexeRE
#if PACKETVER == 20160120
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0865,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-01-27aRagexeRE
#if PACKETVER == 20160127
// shuffle packets
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0922,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0961,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-01-27aRagexeRE
#if PACKETVER >= 20160127
// new packets
	packet(0x0a4d,-1);
// changed packet sizes
#endif

// 2016-02-03aRagexeRE
#if PACKETVER == 20160203
// shuffle packets
	packet(0x0202,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0437,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0811,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0835,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0872,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0873,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0918,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0940,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0947,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0954,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x095a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-02-11aRagexeRE
#if PACKETVER == 20160211
// shuffle packets
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0870,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0886,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-02-17cRagexeRE
#if PACKETVER == 20160217
// shuffle packets
	packet(0x0202,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x023b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0362,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0365,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0864,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0870,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0873,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x087a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0888,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x088f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0899,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a0,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a9,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08ac,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ad,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x091d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0920,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0926,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x092e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x093b,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x093e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0941,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x095e,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0966,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0967,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0969,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

// 2016-02-24bRagexeRE
#if PACKETVER == 20160224
// shuffle packets
	packet(0x022d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0364,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0436,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0861,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0884,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0885,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0888,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a9,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0920,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0929,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0936,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0938,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0961,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-03-02bRagexeRE
#if PACKETVER == 20160302
// shuffle packets
	packet(0x022d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0367,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0802,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0819,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x085b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0864,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0865,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0867,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0868,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0873,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0875,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x087a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x087d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0883,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a6,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a9,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x091a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0927,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092d,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x092f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0945,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x094e,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0950,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0957,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0960,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0961,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0967,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0968,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
#endif

// 2016-03-02bRagexeRE
#if PACKETVER >= 20160302
// new packets
	packet(0x0a4e,4);
	packet(0x0a4f,-1,clif->pDull/*,XXX*/);
	packet(0x0a50,6);
	packet(0x0a51,34);
// changed packet sizes
#endif

// 2016-03-09aRagexeRE
#if PACKETVER == 20160309
// shuffle packets
	packet(0x023b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0281,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0364,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0819,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0838,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x083c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x085a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0866,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086a,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0873,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x087e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x089b,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a7,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0920,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0922,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0929,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x092a,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x092e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0932,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094f,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0956,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x095e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x096a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
#endif

// 2016-03-16aRagexeRE
#if PACKETVER == 20160316
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0922,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-03-16aRagexeRE
#if PACKETVER >= 20160316
// new packets
	packet(0x0a52,20,clif->pDull/*,XXX*/);
	packet(0x0a53,10);
	packet(0x0a54,-1);
	packet(0x0a55,2);
	packet(0x0a56,6,clif->pDull/*,XXX*/);
	packet(0x0a57,6);
	packet(0x0a58,8);
	packet(0x0a59,-1);
	packet(0x0a5a,2,clif->pDull/*,XXX*/);
	packet(0x0a5b,7);
	packet(0x0a5c,18,clif->pDull/*,XXX*/);
	packet(0x0a5d,6);
// changed packet sizes
#endif

// 2016-03-23aRagexeRE
#if PACKETVER == 20160323
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0869,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0872,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0878,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0883,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0896,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089a,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x091b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0926,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0927,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0933,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x093c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-03-23aRagexeRE
#if PACKETVER >= 20160323
// new packets
	packet(0x0a68,3);
	packet(0x0a69,6);
	packet(0x0a6a,12);
	packet(0x0a6b,-1);
// changed packet sizes
#endif

// 2016-03-30aRagexeRE
#if PACKETVER == 20160330
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x086d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0878,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087f,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0889,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x088d,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0918,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0925,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x092a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x092c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0930,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0939,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x093b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-03-30aRagexeRE
#if PACKETVER >= 20160330
// new packets
	packet(0x0a6c,7,clif->pDull/*,XXX*/);
	packet(0x0a6d,-1);
	packet(0x0a6e,-1,clif->pRodexSendMail); // CZ_RODEX_SEND_MAIL
	packet(0x0a6f,-1);
// changed packet sizes
#endif

// 2016-04-06aRagexeRE
#if PACKETVER == 20160406
// shuffle packets
	packet(0x0364,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x07e4,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0819,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x085a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0869,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0877,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0878,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0879,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0884,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0892,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0895,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0898,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a1,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a9,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08ac,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0927,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x092d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0933,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0934,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0940,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0949,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x094d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0953,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0962,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
#endif

// 2016-04-14bRagexeRE
#if PACKETVER == 20160414
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0363,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0862,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x087a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0880,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0885,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0918,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0922,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0927,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0931,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0934,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0945,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0953,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-04-20aRagexeRE
#if PACKETVER == 20160420
// shuffle packets
	packet(0x022d,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x02c4,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x035f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0864,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0870,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0872,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0874,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0884,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0888,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a5,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0935,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x094e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095c,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-04-27aRagexeRE
#if PACKETVER == 20160427
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0835,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0940,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-04-27aRagexeRE
#if PACKETVER >= 20160427
// new packets
// changed packet sizes
	packet(0x0a50,4);
#endif

// 2016-05-04aRagexeRE
#if PACKETVER == 20160504
// shuffle packets
	packet(0x0202,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0363,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0365,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x083c,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x085f,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x086b,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x087f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0884,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0886,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0887,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x088d,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0890,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0893,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0898,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x089d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08ad,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0918,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0921,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0922,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0924,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0940,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0941,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0948,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0952,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0969,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
#endif

// 2016-05-04aRagexeRE
#if PACKETVER >= 20160504
// new packets
	packet(0x0a70,2,clif->pDull/*,XXX*/);
	packet(0x0a71,-1);
	packet(0x0a72,61);
// changed packet sizes
#endif

// 2016-05-11aRagexeRE
#if PACKETVER == 20160511
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0894,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0918,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0920,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-05-11aRagexeRE
#if PACKETVER >= 20160511
// new packets
	packet(0x0a73,6);
	packet(0x0a74,8);
// changed packet sizes
#endif

// 2016-05-18aRagexeRE
#if PACKETVER == 20160518
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0874,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x089a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a9,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0928,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-05-18aRagexeRE
#if PACKETVER >= 20160518
// new packets
	packet(0x0a76,80);
// changed packet sizes
	packet(0x0a73,2);
#endif

// 2016-05-25aRagexeRE
#if PACKETVER == 20160525
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x085e,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0867,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x086a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0899,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x092c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0937,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0945,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x094a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x094e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0951,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0956,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-05-25aRagexeRE
#if PACKETVER >= 20160525
// new packets
	packet(0x0a77,15);
	packet(0x0a78,15);
// changed packet sizes
#endif

// 2016-06-01aRagexeRE
#if PACKETVER == 20160601
// shuffle packets
	packet(0x0202,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x02c4,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0863,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0870,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x087d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x088f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0895,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a7,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08ac,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0924,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x095f,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0961,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-06-01aRagexeRE
#if PACKETVER >= 20160601
// new packets
	packet(0x0a79,-1);
	packet(0x0a7b,-1);
	packet(0x0a7c,-1);
	packet(0x0a7d,-1); // ZC_RODEX_MAILLIST
// changed packet sizes
#endif

// 2016-06-08aRagexeRE
#if PACKETVER == 20160608
// shuffle packets
	packet(0x022d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x02c4,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x035f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0437,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07ec,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0802,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085c,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0885,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0889,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0899,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a6,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x094d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0958,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0969,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-06-15aRagexeRE
#if PACKETVER == 20160615
// shuffle packets
	packet(0x0281,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0363,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0364,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0369,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x083c,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0866,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0870,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x087f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0884,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0887,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0888,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0891,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0898,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x092f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x093e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0947,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0948,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x094a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094b,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0954,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0957,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0958,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x095c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x095e,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0961,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

// 2016-06-15aRagexeRE
#if PACKETVER >= 20160615
// new packets
	packet(0x0a7e,4);
	packet(0x0a7f,-1);
	packet(0x0a80,2);
	packet(0x0a81,3);
// changed packet sizes
#endif

// 2016-06-22aRagexeRE
#if PACKETVER == 20160622
// shuffle packets
	packet(0x023b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x035f,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0361,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x07e4,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0861,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0865,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0867,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0880,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0887,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0890,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0891,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0892,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x089a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x089e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a2,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a8,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092d,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x092f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0936,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0937,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x093b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x093f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0946,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0959,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0965,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0969,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
#endif

// 2016-06-22aRagexeRE
#if PACKETVER >= 20160622
// new packets
	packet(0x0a82,46);
	packet(0x0a83,46);
	packet(0x0a84,94);
	packet(0x0a85,82);
	packet(0x0a86,-1);
	packet(0x0a87,4);
	packet(0x0a88,2);
// changed packet sizes
#endif

// 2016-06-29aRagexeRE
#if PACKETVER == 20160629
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x022d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0363,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0368,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x085c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x085e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0860,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0861,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0863,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0867,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x086b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0881,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0885,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x088e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0893,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091e,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0922,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0925,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0926,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x093e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0946,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0948,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x094a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0957,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x095a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0968,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0969,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x096a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2016-06-29aRagexeRE
#if PACKETVER >= 20160629
// new packets
	packet(0x0a89,32);
	packet(0x0a8a,6);
	packet(0x0a8b,2);
	packet(0x0a8c,2);
	packet(0x0a8d,-1);
// changed packet sizes
	packet(0x0a80,6);
#endif

// 2016-06-30aRagexeRE
#if PACKETVER == 20160630
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x022d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0363,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0368,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x085c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x085e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0860,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0861,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0863,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0867,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x086b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0881,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0885,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x088e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0893,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091e,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0922,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0925,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0926,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x093e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0946,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0948,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x094a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0957,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x095a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0968,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0969,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x096a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2016-07-06cRagexeRE
#if PACKETVER == 20160706
// shuffle packets
	packet(0x0362,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0436,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0860,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0869,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x086b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0884,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0886,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0889,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0892,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0899,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a4,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a5,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08a8,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0918,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x091b,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0924,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0926,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0927,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0929,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0939,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x093d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0944,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0945,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x094c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0952,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0957,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0958,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
#endif

// 2016-07-06cRagexeRE
#if PACKETVER >= 20160706
// new packets
	packet(0x0a81,3);
// changed packet sizes
	packet(0x0a7e,-1);
	packet(0x0a89,57);
#endif

// 2016-07-13bRagexeRE
#if PACKETVER == 20160713
// shuffle packets
	packet(0x022d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0363,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0364,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0838,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0860,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0865,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0869,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0875,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0877,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x087b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0883,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0892,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x089a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a2,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a4,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x091d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0921,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0922,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x092c,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0931,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0939,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0944,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0945,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0947,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0957,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x095b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
#endif

// 2016-07-13aRagexeRE
#if PACKETVER >= 20160713
// new packets
// changed packet sizes
	packet(0x0a87,-1);
#endif

// 2016-07-20aRagexeRE
#if PACKETVER == 20160720
// shuffle packets
	packet(0x0362,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0363,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0365,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x07e4,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0819,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0838,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x085b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x086a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x087f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0883,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0887,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0897,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x089a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x089e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a0,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08aa,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0917,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x091c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093b,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x093e,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0946,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0953,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x095b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0960,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0969,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
#endif

// 2016-07-20aRagexeRE
#if PACKETVER >= 20160720
// new packets
	packet(0x0a8e,2);
	packet(0x0a8f,2);
	packet(0x0a90,3);
// changed packet sizes
#endif

// 2016-07-27bRagexeRE
#if PACKETVER == 20160727
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x023b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0362,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0363,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0436,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0438,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x07ec,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0866,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0868,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0869,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0874,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0877,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0883,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0887,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0891,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x089f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08a2,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a4,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08a7,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0936,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0941,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0946,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0949,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0951,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x095f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0966,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0969,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
#endif

// 2016-07-27aRagexeRE
#if PACKETVER >= 20160727
// new packets
	packet(0x0a91,-1);
	packet(0x0a92,-1);
	packet(0x0a93,3);
// changed packet sizes
#endif

// 2016-08-03bRagexeRE
#if PACKETVER == 20160803
// shuffle packets
	packet(0x0364,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x085d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0878,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0881,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0886,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0887,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0888,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0891,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0895,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089e,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x08a1,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x091b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0929,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0930,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0932,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0934,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0937,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x093a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x093e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x093f,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0952,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0955,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0956,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0959,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x095a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
#endif

// 2016-08-03bRagexeRE
#if PACKETVER >= 20160803
// new packets
	packet(0x0a94,2);
// changed packet sizes
	packet(0x0a81,4);
#endif

// 2016-08-10aRagexeRE
#if PACKETVER == 20160810
// shuffle packets
	packet(0x0361,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0819,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0838,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x085d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x085e,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x085f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0860,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x086f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0875,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0879,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x087a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0885,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0888,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0890,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x089d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x089f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a9,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x091c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0926,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x092b,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x092d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0935,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0943,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0959,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x095b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0967,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
#endif

// 2016-08-31bRagexeRE
#if PACKETVER == 20160831
// shuffle packets
	packet(0x022d,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0366,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07ec,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0835,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0865,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x086d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0870,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0874,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0876,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0878,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x087c,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08a8,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08a9,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0917,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x092c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0938,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x093a,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0946,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x094a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x094f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0950,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0954,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0957,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x095e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0960,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0964,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0967,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

// 2016-09-07aRagexeRE
#if PACKETVER == 20160907
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x091c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-09-07aRagexeRE
#if PACKETVER >= 20160907
// new packets
	packet(0x0a95,4);
// changed packet sizes
#endif

// 2016-09-13aRagexeRE
#if PACKETVER == 20160913
// shuffle packets
	packet(0x0361,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0817,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085b,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0865,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0874,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0875,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0879,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x087a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0887,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0889,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x088e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0891,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0892,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x089b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a5,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0928,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0935,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x093a,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0949,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x094a,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0950,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0952,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0954,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0962,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0963,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0968,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
#endif

// 2016-09-21bRagexeRE
#if PACKETVER == 20160921
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x094a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-09-21bRagexeRE
#if PACKETVER >= 20160921
// new packets
	packet(0x0a96,51);
// changed packet sizes
	packet(0x0a37,59); // ZC_ITEM_PICKUP_ACK_V7
#endif

// 2016-09-28dRagexeRE
#if PACKETVER == 20160928
// shuffle packets
	packet(0x0202,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0366,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0436,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0811,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0838,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0864,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0866,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0872,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0878,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0889,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x088e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0897,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x089a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a2,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a9,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0919,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x091e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0927,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x092d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0944,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x094d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094e,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0953,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0955,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0957,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
#endif

// 2016-09-28cRagexeRE
#if PACKETVER >= 20160928
// new packets
	packet(0x0a97,8);
	packet(0x0a98,12);
	packet(0x0a99,8);
	packet(0x0a9a,10);
	packet(0x0a9b,-1);
	packet(0x0a9c,2);
	packet(0x0a9d,4);
	packet(0x0a9e,2);
	packet(0x0a9f,2);
// changed packet sizes
#endif

// 2016-10-05aRagexeRE
#if PACKETVER == 20161005
// shuffle packets
	packet(0x0202,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0838,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0863,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0886,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0891,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0892,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x089b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a0,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08ac,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08ad,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0918,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0919,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x091e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x092b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0931,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0932,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x093b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0942,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0944,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0945,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x094a,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x094d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0952,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x095b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0967,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

// 2016-10-05aRagexeRE
#if PACKETVER >= 20161005
// new packets
	packet(0x0aa0,2,clif->pDull/*,XXX*/);
	packet(0x0aa1,4);
	packet(0x0aa2,-1);
	packet(0x0aa3,7);
	packet(0x0aa4,2);
// changed packet sizes
#endif

// 2016-10-12aRagexeRE
#if PACKETVER == 20161012
// shuffle packets
	packet(0x023b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0364,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0365,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0369,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x07ec,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0819,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x085b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x085e,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0863,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0868,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0872,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0875,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0880,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0893,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a0,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0936,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0937,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0939,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0943,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0944,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0951,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x095c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0962,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0966,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0967,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
#endif

// 2016-10-19aRagexeRE
#if PACKETVER == 20161019
// shuffle packets
	packet(0x022d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0361,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0889,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0892,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0946,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0963,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-10-26bRagexeRE
#if PACKETVER == 20161026
// shuffle packets
	packet(0x0363,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0438,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0802,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085f,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0861,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0862,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x086a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086c,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x086e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087a,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x087c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0886,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0891,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0894,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0898,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0926,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x092c,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x092e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x092f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0930,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x094b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0953,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x095c,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0962,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2016-10-26bRagexeRE
#if PACKETVER >= 20161026
// new packets
	packet(0x0aa5,-1);
	packet(0x0aa6,36);
// changed packet sizes
#endif

// 2016-11-02aRagexeRE
#if PACKETVER == 20161102
// shuffle packets
	packet(0x0361,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0367,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0436,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0802,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0838,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x083c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086c,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0874,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0886,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0890,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a2,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08aa,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x091b,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0922,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0925,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0928,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0936,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0946,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0949,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x095e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0964,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0965,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0966,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
#endif

// 2016-11-03aRagexeRE
#if PACKETVER == 20161103
// shuffle packets
	packet(0x0361,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0367,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0436,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0802,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0838,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x083c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0869,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086c,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0874,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0886,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0890,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a2,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08aa,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x091b,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0922,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0925,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0928,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0936,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0946,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0949,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x095e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0964,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0965,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0966,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
#endif

// 2016-11-09bRagexeRE
#if PACKETVER == 20161109
// shuffle packets
	packet(0x02c4,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0361,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0362,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0366,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x085d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x085e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0865,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x086a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x086d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0870,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0876,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x087a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0881,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x088e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0891,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0898,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089d,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a7,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08ad,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0927,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0937,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0954,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0956,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2016-11-16cRagexeRE
#if PACKETVER == 20161116
// shuffle packets
	packet(0x0368,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0369,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0835,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x085f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0864,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x086f,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0885,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x088d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x088f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0890,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0892,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0893,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a1,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a2,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08aa,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08ac,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0920,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0925,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x092a,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0931,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x093c,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x094a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0952,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0957,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x095b,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095d,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0967,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
#endif

// 2016-11-23aRagexeRE
#if PACKETVER == 20161123
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0362,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0437,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085c,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0861,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0862,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0866,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x086f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0871,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0880,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0882,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x088b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089c,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a9,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08aa,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0926,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x092a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x092f,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0930,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0941,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x094d,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x094f,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x095a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x095b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0962,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x096a,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
#endif

// 2016-11-30bRagexeRE
#if PACKETVER == 20161130
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0931,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0943,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0954,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0959,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-11-30aRagexeRE
#if PACKETVER >= 20161130
// new packets
	packet(0x0aa7,6);
	packet(0x0aa8,5);
	packet(0x0aa9,-1);
	packet(0x0aaa,-1);
	packet(0x0aab,-1);
// changed packet sizes
#endif

// 2016-12-07eRagexeRE
#if PACKETVER == 20161207
// shuffle packets
	packet(0x023b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0867,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0868,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0875,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x087e,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0886,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a1,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a2,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08ad,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0918,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0943,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0965,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-12-07cRagexeRE
#if PACKETVER >= 20161207
// new packets
	packet(0x0aac,67);
// changed packet sizes
#endif

// 2016-12-14bRagexeRE
#if PACKETVER == 20161214
// shuffle packets
	packet(0x022d,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0281,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x02c4,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0364,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0436,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0862,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x086d,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0887,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0895,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0899,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a6,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x092e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x093d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2016-12-21aRagexeRE
#if PACKETVER == 20161221
// shuffle packets
	packet(0x035f,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0366,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0438,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0817,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x085b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0866,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0876,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0881,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0884,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0885,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088c,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0890,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0899,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08aa,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091e,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0926,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0928,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092c,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x092e,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0930,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0943,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0946,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x094b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x095a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0964,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0965,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

// 2016-12-21aRagexeRE
#if PACKETVER >= 20161221
// new packets
	packet(0x0aad,47);
	packet(0x0aae,2);
	packet(0x0aaf,6);
	packet(0x0ab0,6);
	packet(0x0ab1,10);
// changed packet sizes
#endif

// 2016-12-28aRagexeRE
#if PACKETVER == 20161228
// shuffle packets
	packet(0x0362,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x085a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0865,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x086a,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x086c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0870,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0871,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0875,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x087f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0886,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0889,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0893,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a2,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a3,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a5,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ab,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08ac,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08ad,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0929,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x092c,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0934,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0935,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0938,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x093d,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0944,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
#endif

// 2016-12-28aRagexeRE
#if PACKETVER >= 20161228
// new packets
// changed packet sizes
	packet(0x0ab1,14);
#endif

// 2017-01-04bRagexeRE
#if PACKETVER == 20170104
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x085a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087f,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0896,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-01-04bRagexeRE
#if PACKETVER >= 20170104
// new packets
	packet(0x0ab2,7);
	packet(0x0ab3,15);
// changed packet sizes
#endif

// 2017-01-11aRagexeRE
#if PACKETVER == 20170111
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0877,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x088a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a1,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a3,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a6,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x091b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094c,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0961,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0969,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-01-11aRagexeRE
#if PACKETVER >= 20170111
// new packets
	packet(0x0ab4,4);
	packet(0x0ab5,2);
	packet(0x0ab6,6);
	packet(0x0ab7,4);
	packet(0x0ab8,2);
	packet(0x0ab9,39);
// changed packet sizes
#endif

// 2017-01-18aRagexeRE
#if PACKETVER == 20170118
// shuffle packets
	packet(0x022d,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0364,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0862,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0865,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x086f,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0873,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08ad,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x091f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0927,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0933,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0958,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0962,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2017-01-18aRagexeRE
#if PACKETVER >= 20170118
// new packets
	packet(0x0aba,2);
	packet(0x0abb,2);
// changed packet sizes
	packet(0x0aad,51);
	packet(0x0ab3,19);
#endif

// 2017-01-25aRagexeRE
#if PACKETVER == 20170125
// shuffle packets
	packet(0x0438,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0811,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0876,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0877,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0879,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087b,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x087d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0881,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0884,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0893,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0894,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0895,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0898,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x089b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08a5,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x091b,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x091c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0920,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0929,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x092b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0930,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0943,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0944,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x095c,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0965,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0968,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

// 2017-02-01aRagexeRE
#if PACKETVER == 20170201
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0815,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x085e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0875,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0879,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0881,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0884,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0885,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0886,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a4,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0919,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0920,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0938,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0940,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x094c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0966,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0969,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-02-01aRagexeRE
#if PACKETVER >= 20170201
// new packets
	packet(0x0abc,-1);
// changed packet sizes
#endif

// 2017-02-08aRagexeRE
#if PACKETVER == 20170208
// shuffle packets
	packet(0x02c4,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0367,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0860,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0892,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a1,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ac,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0921,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0923,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0932,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0937,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-02-15aRagexeRE
#if PACKETVER == 20170215
// shuffle packets
	packet(0x02c4,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x035f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0811,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x083c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0876,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087c,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x087d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087e,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0883,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0884,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088a,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x088c,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0890,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0896,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a2,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08a8,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091c,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0925,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092b,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x092d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0942,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x094e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x095f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0962,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0969,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
#endif

// 2017-02-15aRagexeRE
#if PACKETVER >= 20170215
// new packets
	packet(0x0abd,10);
// changed packet sizes
#endif

// 2017-02-22aRagexeRE
#if PACKETVER == 20170222
// shuffle packets
	packet(0x0202,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0866,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0870,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0871,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0877,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0889,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0894,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a3,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a8,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0937,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0939,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0943,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0962,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-02-22aRagexeRE
#if PACKETVER >= 20170222
// new packets
	packet(0x0abe,116);
	packet(0x0abf,114);
// changed packet sizes
#endif

// 2017-02-28aRagexeRE
#if PACKETVER == 20170228
// shuffle packets
	packet(0x022d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0360,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0362,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0819,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x085e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0863,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x086b,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0873,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0874,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0876,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0883,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0884,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0889,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0893,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x089e,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a0,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a2,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a6,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08a7,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x091f,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x092a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0937,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x093e,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0944,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0947,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0948,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0952,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0955,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif

// 2017-02-28aRagexeRE
#if PACKETVER >= 20170228
// new packets
	packet(0x0ac0,26);
	packet(0x0ac1,26);
	packet(0x0ac2,-1);
	packet(0x0ac3,2);
	packet(0x0ac4,-1);
	packet(0x0ac5,156,clif->pDull/*,XXX*/);
	packet(0x0ac6,156);
	packet(0x0ac7,156);
// changed packet sizes
	packet(0x0abe,-1);
	packet(0x0abf,-1);
#endif

// 2017-03-08bRagexeRE
#if PACKETVER == 20170308
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-03-08bRagexeRE
#if PACKETVER >= 20170308
// new packets
	packet(0x0ac8,2);
	packet(0x0ac9,-1);
// changed packet sizes
#endif

// 2017-03-15cRagexeRE
#if PACKETVER == 20170315
// shuffle packets
	packet(0x02c4,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x035f,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0360,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0366,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0367,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0436,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x07ec,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085c,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0863,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x086a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0872,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087b,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0884,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x088b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x088d,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088f,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0892,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x089c,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08aa,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x091d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0920,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0922,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0944,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x094a,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0950,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0952,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
#endif

// 2017-03-22aRagexeRE
#if PACKETVER == 20170322
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x091a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-03-22aRagexeRE
#if PACKETVER >= 20170322
// new packets
	packet(0x0aca,3);
// changed packet sizes
#endif

// 2017-03-29dRagexeRE
#if PACKETVER == 20170329
// shuffle packets
	packet(0x0281,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0363,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x087a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0888,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x08a8,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0917,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0926,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0929,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x092e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0937,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0939,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0949,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x095f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-03-29cRagexeRE
#if PACKETVER >= 20170329
// new packets
// changed packet sizes
	packet(0x0aac,69);
#endif

// 2017-04-05bRagexeRE
#if PACKETVER == 20170405
// shuffle packets
	packet(0x022d,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0281,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0363,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085f,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0860,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0864,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0865,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x086f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0893,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a5,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094c,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0964,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-04-05bRagexeRE
#if PACKETVER >= 20170405
// new packets
	packet(0x0acb,12);
	packet(0x0acc,18);
// changed packet sizes
#endif

// 2017-04-12aRagexeRE
#if PACKETVER == 20170412
// shuffle packets
	packet(0x023b,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0365,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0863,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0869,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x086d,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0878,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0879,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087b,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088b,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0890,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0893,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0898,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x089a,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x089c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08a1,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x091a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x091e,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0929,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x092e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0938,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0942,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0945,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0949,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x094f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0952,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0959,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x095c,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x095d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
#endif

// 2017-04-19bRagexeRE
#if PACKETVER == 20170419
// shuffle packets
	packet(0x0811,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0819,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0838,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0862,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0868,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x086a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0872,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0881,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0897,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0898,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x089d,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x08aa,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0920,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0922,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0930,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0931,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0935,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x093a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093f,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0942,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x095c,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0963,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0965,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
#endif

// 2017-04-19bRagexeRE
#if PACKETVER >= 20170419
// new packets
	packet(0x0acd,23);
// changed packet sizes
	packet(0x0a99,4);
#endif

// 2017-04-26dRagexeRE
#if PACKETVER == 20170426
// shuffle packets
	packet(0x0281,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0866,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x086f,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087a,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0887,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0899,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a2,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08a4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0927,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0940,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0958,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0963,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-04-26dRagexeRE
#if PACKETVER >= 20170426
// new packets
// changed packet sizes
	packet(0x0a98,10);
#endif

// 2017-05-02dRagexeRE
#if PACKETVER == 20170502
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0875,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0894,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x089c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x093c,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0950,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-05-02dRagexeRE
#if PACKETVER >= 20170502
// new packets
	packet(0x0ace,4);
// changed packet sizes
#endif

// 2017-05-17aRagexeRE
#if PACKETVER == 20170517
// shuffle packets
	packet(0x0364,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0367,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0437,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0802,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0815,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0817,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0868,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0875,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x087b,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087d,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088c,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x088d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0894,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0896,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0899,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089e,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x089f,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a2,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a8,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08aa,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x091b,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0923,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x093b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0945,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0946,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0947,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0958,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0960,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0964,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
#endif

// 2017-05-24aRagexeRE
#if PACKETVER == 20170524
// shuffle packets
	packet(0x0364,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0368,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0802,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x085e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0860,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0864,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0866,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0868,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x086d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0873,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0874,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x087d,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0882,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x088d,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0894,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089c,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a1,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0923,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0925,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0934,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0946,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0958,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x095b,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0964,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0967,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0968,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
#endif

// 2017-05-31aRagexeRE
#if PACKETVER == 20170531
// shuffle packets
	packet(0x0361,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0369,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x07e4,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x07ec,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0819,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x085f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0861,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0868,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0873,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0875,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0878,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0885,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x088d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0894,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x089a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a2,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x08ac,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08ad,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x092d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0933,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0937,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0940,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0945,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0963,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0968,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
#endif

// 2017-06-07bRagexeRE
#if PACKETVER == 20170607
// shuffle packets
	packet(0x0361,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0364,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x07e4,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x085e,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0862,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0863,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0864,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0871,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0873,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0875,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0885,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x088a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0897,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089d,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a9,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08ab,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0917,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0918,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0919,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0925,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0927,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0931,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0934,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0938,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x093d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0942,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0944,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0949,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

// 2017-06-14bRagexeRE
#if PACKETVER == 20170614
// shuffle packets
	packet(0x023b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0361,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0364,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0367,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0437,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0838,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x083c,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0860,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0865,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0866,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0867,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x086b,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x086c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0877,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0879,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x087d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0889,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0899,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089d,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a2,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x08ad,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091b,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0928,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x092f,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0936,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0944,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0957,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0963,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

// 2017-06-14bRagexeRE
#if PACKETVER >= 20170614
// new packets
	packet(0x0acf,52);
	packet(0x0ad0,11);
	packet(0x0ad1,-1);
#endif

// 2017-06-21aRagexeRE
#if PACKETVER == 20170621
// shuffle packets
	packet(0x0202,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0365,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0366,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0802,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x085d,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x087d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0885,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0889,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a8,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0956,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0957,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x095b,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x095c,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0961,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-06-21aRagexeRE
#if PACKETVER >= 20170621
// changed packet sizes
	packet(0x0acf,57);
#endif

// 2017-06-28bRagexeRE
#if PACKETVER == 20170628
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0863,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-07-05aRagexeRE
#if PACKETVER == 20170705
// shuffle packets
	packet(0x0202,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x02c4,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0879,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0886,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x088d,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x089d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091a,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0930,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0932,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0934,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x094c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-07-05aRagexeRE
#if PACKETVER >= 20170705
// changed packet sizes
	packet(0x0acf,64);
#endif

// 2017-07-12bRagexeRE
#if PACKETVER == 20170712
// shuffle packets
	packet(0x0202,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x023b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0944,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-07-19aRagexeRE
#if PACKETVER == 20170719
// shuffle packets
	packet(0x022d,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0367,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0368,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0369,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x07e4,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x085a,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085e,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0863,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x086e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x087d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0881,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0882,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0885,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0891,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0898,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x089a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089d,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x08a6,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a8,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091b,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091f,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x092c,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x092e,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x092f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x093d,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0944,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0946,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0966,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
#endif

// 2017-07-19aRagexeRE
#if PACKETVER >= 20170719
// new packets
	packet(0x0ad2,30);
	packet(0x0ad3,-1);
	packet(0x0ad4,-1);
	packet(0x0ad5,2);
	packet(0x0ad6,2);
	packet(0x0ad7,8);
	packet(0x0ad8,8);
	packet(0x0ad9,-1);
// changed packet sizes
#endif

// 2017-07-26cRagexeRE
#if PACKETVER == 20170726
// shuffle packets
	packet(0x0363,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0364,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0366,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0369,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0438,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0838,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0873,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0874,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0878,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0881,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0888,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x088e,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08a3,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08a7,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08aa,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x08ab,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08ac,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091d,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x091e,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091f,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0921,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0923,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0943,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x094f,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0950,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0952,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0954,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x095a,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0963,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
#endif

// 2017-07-26cRagexeRE
#if PACKETVER >= 20170726
// new packets
	packet(0x0ada,30);
#endif

// 2017-08-01aRagexeRE
#if PACKETVER == 20170801
// shuffle packets
	packet(0x022d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0281,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0362,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x087d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a6,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x094f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095a,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-08-16cRagexeRE
#if PACKETVER == 20170816
// shuffle packets
	packet(0x022d,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x035f,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0361,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0362,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0438,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x085a,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0862,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0864,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x087e,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0881,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0882,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0884,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0888,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0889,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08a3,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a7,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08a9,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x08ac,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x091c,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0921,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0925,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x092c,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x093a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x093d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0941,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0950,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0959,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0960,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
#endif

// 2017-08-23aRagexeRE
#if PACKETVER == 20170823
// shuffle packets
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0364,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x07e4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x07ec,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0802,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086c,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08ac,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x095b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-08-30bRagexeRE
#if PACKETVER == 20170830
// shuffle packets
	packet(0x0281,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x02c4,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0363,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0364,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0860,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0865,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x086a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0875,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0884,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0885,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0888,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0897,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0899,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089e,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a2,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a8,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x091e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0921,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0925,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x092e,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0939,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0940,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0942,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0943,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0947,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0951,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0959,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
#endif

// 2017-08-30aRagexeRE
#if PACKETVER >= 20170830
// new packets
	packet(0x0adb,-1);
// changed packet sizes
	packet(0x006d,157); // HC_ACCEPT_MAKECHAR
	packet(0x08e3,157); // HC_UPDATE_CHARINFO
	packet(0x0a49,20);
#endif

// 2017-09-06cRagexeRE
#if PACKETVER == 20170906
// shuffle packets
	packet(0x0202,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0281,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x02c4,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0860,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0866,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x086c,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08a2,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a3,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x08a7,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x091a,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0953,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-09-06cRagexeRE
#if PACKETVER >= 20170906
// new packets
	packet(0x0adc,6);
#endif

// 2017-09-13bRagexeRE
#if PACKETVER == 20170913
// shuffle packets
	packet(0x0281,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x035f,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0437,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x07e4,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0817,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0835,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x085a,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0860,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0865,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0866,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x088c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0890,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0891,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0892,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a6,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x08a7,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x08aa,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08ab,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08ac,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08ad,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x091b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x091d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x091e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0920,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0923,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0925,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0927,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x095a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x095c,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
#endif

// 2017-09-13bRagexeRE
#if PACKETVER >= 20170913
// new packets
	packet(0x0add,22);
#endif

// 2017-09-20bRagexeRE
#if PACKETVER == 20170920
// shuffle packets
	packet(0x0369,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0436,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x07ec,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x085a,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0861,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0862,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0864,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0865,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x086a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x086c,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0874,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0875,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0889,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x088e,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x089b,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0919,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x091e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0921,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0923,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0926,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x092e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0937,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0939,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0945,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x094c,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x095d,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0961,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0966,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096a,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
#endif

#if PACKETVER >= 20170920
// new packets
	packet(0x0ade,6);
	packet(0x0adf,58);
#endif

#endif /* MAP_PACKETS_H */
