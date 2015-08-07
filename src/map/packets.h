// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

//Included directly by clif.h in packet_loaddb()

#ifndef MAP_PACKETS_H
#define MAP_PACKETS_H

#ifndef packet
	#define packet(a,b,...)
#endif

#ifndef packetKeys
	#define packetKeys(a,b,c)
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

//2011-07-18aRagexe (Thanks to Yommy!)
#if PACKETVER >= 20110718
	packet(0x0844,2,clif->pCashShopOpen,2);/* tell server cashshop window is being open */
	packet(0x084a,2,clif->pCashShopClose,2);/* tell server cashshop window is being closed */
	packet(0x0846,4,clif->pCashShopReqTab,2);
	packet(0x08c9,2,clif->pCashShopSchedule,0);
	packet(0x0848,-1,clif->pCashShopBuy,2);
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
	packet(0x08d2,10);
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
	packet(0x08CF,10);//Amulet spirits
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
	packet(0x08d2,10);
	packet(0x0916,26,clif->pGuildInvite2,2);
#endif

#ifndef PACKETVER_RE
#if PACKETVER >= 20120604
	packet(0x0861,18,clif->pPartyRecruitRegisterReq,2,4,6);
#endif
#endif

//2012-06-18aRagexeRE
#if PACKETVER >= 20120618
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
	//packet(0x095A,8); // unknown usage
	packet(0x0868,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0888,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x086D,26,clif->pPartyInvite2,2);
	//packet(0x0890,4); // unknown usage
	packet(0x086F,26,clif->pFriendsListAdd,2);
	packet(0x093F,5,clif->pHomMenu,2,4);
	packet(0x0947,36,clif->pStoragePassword,0);
	// Shuffle End

	// New Packets
	packet(0x0998,8,clif->pEquipItem,2,4);
	packet(0x0447,2); // PACKET_CZ_BLOCKING_PLAY_CANCEL
	packet(0x099f,24);
	// New Packets End
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
	//packet(0x08AA,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0963,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0943,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0947,26,clif->pPartyInvite2,2);
	//packet(0x0862,4); // CZ_GANGSI_RANK
	packet(0x0962,26,clif->pFriendsListAdd,2);
	packet(0x0931,5,clif->pHomMenu,2,4);
	packet(0x093E,36,clif->pStoragePassword,0);
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
	//packet(0x0965,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x086A,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x08A9,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0950,26,clif->pPartyInvite2,2);
	//packet(0x08AC,4); // CZ_GANGSI_RANK
	packet(0x0362,26,clif->pFriendsListAdd,2);
	packet(0x0926,5,clif->pHomMenu,2,4);
	packet(0x088E,36,clif->pStoragePassword,0);
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
	// packet(0x088E,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0958,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0919,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x08A8,26,clif->pPartyInvite2,2);
	// packet(0x0888,4); // CZ_GANGSI_RANK
	packet(0x0877,26,clif->pFriendsListAdd,2);
	packet(0x023B,5,clif->pHomMenu,2,4);
	packet(0x0956,36,clif->pStoragePassword,0);
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0883,36,clif->pStoragePassword,0);
	packet(0x097C,4,clif->pRanklist);
#endif

//2013-06-12Ragexe (Shakto)
#if PACKETVER >= 20130612
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
	// packet(0x087A,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0942,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x095B,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0887,26,clif->pPartyInvite2,2);
	// packet(0x0878,4); // CZ_GANGSI_RANK
	packet(0x0953,26,clif->pFriendsListAdd,2);
	packet(0x02C4,5,clif->pHomMenu,2,4);
	packet(0x0864,36,clif->pStoragePassword,0);
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
	// packet(0x0860,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x08A5,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x088C,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0895,26,clif->pPartyInvite2,2);
	// packet(0x088F,4); // CZ_GANGSI_RANK
	packet(0x08AB,26,clif->pFriendsListAdd,2);
	packet(0x0960,5,clif->pHomMenu,2,4);
	packet(0x0930,36,clif->pStoragePassword,0);
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x0360,26,clif->pFriendsListAdd,2);
	packet(0x094A,5,clif->pHomMenu,2,4);
	packet(0x0873,36,clif->pStoragePassword,0);
#endif

/* Bank System [Yommy/Hercules] */
#if PACKETVER >= 20130724
	packet(0x09A6,12); // ZC_BANKING_CHECK
	packet(0x09A7,10,clif->pBankDeposit,2,4,6);
	packet(0x09A8,16); // ZC_ACK_BANKING_DEPOSIT
	packet(0x09A9,10,clif->pBankWithdraw,2,4,6);
	packet(0x09AA,16); // ZC_ACK_BANKING_WITHDRAW
	packet(0x09AB,6,clif->pBankCheck,2,4);
	////
	packet(0x09B6,6,clif->pBankOpen,2,4);
	packet(0x09B7,4); // ZC_ACK_OPEN_BANKING
	packet(0x09B8,6,clif->pBankClose,2,4);
	packet(0x09B9,4); // ZC_ACK_CLOSE_BANKING
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0887,36,clif->pStoragePassword,0);
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
	// packet(0x0896,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x08A4,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0368,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0927,26,clif->pPartyInvite2,2);
	// packet(0x0815,4); // CZ_GANGSI_RANK
	packet(0x0281,26,clif->pFriendsListAdd,2);
	packet(0x0958,5,clif->pHomMenu,2,4);
	packet(0x0885,36,clif->pStoragePassword,0);
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x092F,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x087B,4); // CZ_GANGSI_RANK
	packet(0x08AB,26,clif->pFriendsListAdd,2);
	packet(0x0811,5,clif->pHomMenu,2,4);
	packet(0x085C,36,clif->pStoragePassword,0);
	/* New */
	packet(0x09d4,2,clif->pNPCShopClosed);
	packet(0x09ce,102,clif->pGM_Monster_Item,2);
	/* NPC Market */
	packet(0x09d8,2,clif->pNPCMarketClosed);
	packet(0x09d6,-1,clif->pNPCMarketPurchase);
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022d,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x08A4,36,clif->pStoragePassword,0);
	packet(0x09df,7);
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
	// packet(0x093E,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x022D,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x089C,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x08A9,26,clif->pPartyInvite2,2);
	// packet(0x087E,4); // CZ_GANGSI_RANK
	packet(0x0943,26,clif->pFriendsListAdd,2);
	packet(0x0949,5,clif->pHomMenu,2,4);
	packet(0x091D,36,clif->pStoragePassword,0);
#endif

// 2014 Packet Data

// 2014-01-15eRagexe - YomRawr
#if PACKETVER >= 20140115
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x08A7,5,clif->pChangeDir,2,4);
	packet(0x0940,6,clif->pTakeItem,2);
	packet(0x0361,6,clif->pDropItem,2,4);
	packet(0x088E,8,clif->pMoveToKafra,2,4);
	packet(0x0367,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x0802,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0360,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0817,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0815,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x096A,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x088A,6,clif->pReqClickBuyingStore,2);
	packet(0x0965,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x096A,18,clif->pPartyBookingRegisterReq,2,4);
	// packet(0x088A,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0965,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0966,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x095D,26,clif->pPartyInvite2,2);
	// packet(0x095B,4); // CZ_GANGSI_RANK
	packet(0x089B,26,clif->pFriendsListAdd,2);
	packet(0x092D,5,clif->pHomMenu,2,4);
	packet(0x0865,36,clif->pStoragePassword,0);
#endif

// 2014-02-05bRagexe - Themon
#if PACKETVER >= 20140205
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x0938,36,clif->pStoragePassword,0);
	packet(0x09DF,7);
#endif

// 2014-03-05bRagexe - Themon
#if PACKETVER >= 20140305
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0815,5,clif->pChangeDir,2,4);
	packet(0x0202,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x0364,8,clif->pMoveFromKafra,2,4);
	packet(0x0436,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0361,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0365,18,clif->pPartyBookingRegisterReq,2,4);
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0438,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0878,4); // CZ_GANGSI_RANK
	packet(0x07E4,26,clif->pFriendsListAdd,2);
	packet(0x0934,5,clif->pHomMenu,2,4);
	packet(0x095e,36,clif->pStoragePassword,0);
	packet(0x09DF,7);
#endif

// 2014-04-02gRagexe - Themon
#if PACKETVER >= 20140402
	packet(0x0946,7,clif->pActionRequest,2,6);
	packet(0x0868,10,clif->pUseSkillToId,2,4,6);
	packet(0x093F,5,clif->pWalkToXY,2);
	packet(0x0950,6,clif->pTickSend,2);
	packet(0x0360,5,clif->pChangeDir,2,4);
	packet(0x0958,6,clif->pTakeItem,2);
	packet(0x0882,6,clif->pDropItem,2,4);
	packet(0x095C,8,clif->pMoveToKafra,2,4);
	packet(0x085B,8,clif->pMoveFromKafra,2,4);
	packet(0x0364,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x092D,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x088A,6,clif->pGetCharNameRequest,2);
	packet(0x07EC,6,clif->pSolveCharName,2);
	packet(0x0965,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x085D,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0933,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x091F,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x023B,6,clif->pReqClickBuyingStore,2);
	packet(0x0867,2,clif->pReqCloseBuyingStore,0);
	packet(0x0944,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x08AC,18,clif->pPartyBookingRegisterReq,2,4);
	// packet(0x094C,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0883,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0920,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0890,26,clif->pPartyInvite2,2);
	// packet(0x088C,4); // CZ_GANGSI_RANK
	packet(0x089A,26,clif->pFriendsListAdd,2);
	packet(0x0896,5,clif->pHomMenu,2,4);
	packet(0x0926,36,clif->pStoragePassword,0);
	packet(0x09DF,7);
#endif

// 2014-04-16aRagexe - Themon
#if PACKETVER >= 20140416
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x022D,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0436,4); // CZ_GANGSI_RANK
	packet(0x023B,26,clif->pFriendsListAdd,2);
	packet(0x0361,5,clif->pHomMenu,2,4);
	packet(0x095C,36,clif->pStoragePassword,0);
	packet(0x09DF,7);
#endif

// 2014-10-16aRagexe - YomRawr
#if PACKETVER >= 20141016
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0967,5,clif->pChangeDir,2,4);
	packet(0x07E4,6,clif->pTakeItem,2);
	packet(0x0362,6,clif->pDropItem,2,4);
	packet(0x07EC,8,clif->pMoveToKafra,2,4);
	packet(0x022D,8,clif->pMoveFromKafra,2,4);
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
	// packet(0x0363,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x086E,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0802,26,clif->pPartyInvite2,2);
	// packet(0x0922,4); // CZ_GANGSI_RANK
	packet(0x094B,26,clif->pFriendsListAdd,2);
	packet(0x0364,5,clif->pHomMenu,2,4);
	packet(0x0936,36,clif->pStoragePassword,0);
	packet(0x09DF,7);
	packet(0x0a00,269);
#endif

/* Roulette System [Yommy/Hercules] */
#if PACKETVER >= 20141016
	packet(0x0A19,2,clif->pRouletteOpen,0);     // HEADER_CZ_REQ_OPEN_ROULETTE
	packet(0x0A1A,23);                          // HEADER_ZC_ACK_OPEN_ROULETTE
	packet(0x0A1B,2,clif->pRouletteInfo,0);     // HEADER_CZ_REQ_ROULETTE_INFO
	packet(0x0A1C,-1);                          // HEADER_ZC_ACK_ROULEITTE_INFO
	packet(0x0A1D,2,clif->pRouletteClose,0);    // HEADER_CZ_REQ_CLOSE_ROULETTE
	packet(0x0A1E,3);                           // HEADER_ZC_ACK_CLOSE_ROULETTE
	packet(0x0A1F,2,clif->pRouletteGenerate,0); // HEADER_CZ_REQ_GENERATE_ROULETTE
	packet(0x0A20,21);                          // HEADER_ZC_ACK_GENERATE_ROULETTE
	packet(0x0A21,3,clif->pRouletteRecvItem,2); // HEADER_CZ_RECV_ROULETTE_ITEM
	packet(0x0A22,5);                           // HEADER_ZC_RECV_ROULETTE_ITEM
#endif


// 2014-10-22bRagexe - YomRawr
#if PACKETVER >= 20141022
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x08AD,5,clif->pChangeDir,2,4);
	packet(0x094E,6,clif->pTakeItem,2);
	packet(0x087D,6,clif->pDropItem,2,4);
	packet(0x0878,8,clif->pMoveToKafra,2,4);
	packet(0x08AA,8,clif->pMoveFromKafra,2,4);
	packet(0x023B,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0835,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0940,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0955,18,clif->pPartyBookingRegisterReq,2,4);
	// packet(0x092B,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0281,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x093B,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x0896,26,clif->pPartyInvite2,2);
	// packet(0x08AB,4); // CZ_GANGSI_RANK
	packet(0x091A,26,clif->pFriendsListAdd,2);
	packet(0x0899,5,clif->pHomMenu,2,4);
	packet(0x0438,36,clif->pStoragePassword,0);
#endif

// 2015-05-13aRagexe
#if PACKETVER >= 20150513
	packet(0x0369,7,clif->pActionRequest,2,6);
	packet(0x083C,10,clif->pUseSkillToId,2,4,6);
	packet(0x0437,5,clif->pWalkToXY,2);
	packet(0x035F,6,clif->pTickSend,2);
	packet(0x0924,5,clif->pChangeDir,2,4);
	packet(0x0958,6,clif->pTakeItem,2);
	packet(0x0885,6,clif->pDropItem,2,4);
	packet(0x0879,8,clif->pMoveToKafra,2,4);
	packet(0x0864,8,clif->pMoveFromKafra,2,4);
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);
	packet(0x096A,6,clif->pGetCharNameRequest,2);
	packet(0x0368,6,clif->pSolveCharName,2);
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);
	packet(0x0360,6,clif->pReqClickBuyingStore,2);
	packet(0x022D,2,clif->pReqCloseBuyingStore,0);
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);
	packet(0x0883,18,clif->pPartyBookingRegisterReq,2,4);
	packet(0x02C4,8); // CZ_JOIN_BATTLE_FIELD
	packet(0x0960,-1,clif->pItemListWindowSelected,2,4,8);
	packet(0x0363,19,clif->pWantToConnection,2,6,10,14,18);
	packet(0x094A,26,clif->pPartyInvite2,2);
	packet(0x0927,4); // CZ_GANGSI_RANK
	packet(0x08A8,26,clif->pFriendsListAdd,2);
	packet(0x0817,5,clif->pHomMenu,2,4);
	packet(0x0923,36,clif->pStoragePassword,0);
	packet(0x09e8,11,clif->pDull);	//CZ_OPEN_MAILBOX
	packet(0x0a2e,6,clif->pDull);	//TITLE
#endif         

/* PacketKeys: http://herc.ws/board/topic/1105-hercules-wpe-free-june-14th-patch/ */
#if PACKETVER >= 20110817
	packetKeys(0x053D5CED,0x3DED6DED,0x6DED6DED); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20110824
	packetKeys(0x35C91401,0x262A5556,0x28FA03AA); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20110831
	packetKeys(0x3AD67ED0,0x44703C69,0x6F876809); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20110906
	packetKeys(0x3AD67ED0,0x44703C69,0x6F876809); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111005
	packetKeys(0x291E6762,0x77CD391A,0x60AC2F16); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111012
	packetKeys(0x7F3C2D29,0x59B01DE6,0x1DBB44CA); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111021
	packetKeys(0x357D55DC,0x5A8D759F,0x245C30F5); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111025
	packetKeys(0x50AE1A63,0x3CE579B5,0x29C10406); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111102
	packetKeys(0x5324329D,0x5D545D52,0x06137269); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111109
	packetKeys(0x0B642BDA,0x6ECB1D1C,0x61C7454B); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111122
	packetKeys(0x3B550F07,0x1F666C7C,0x60304EF5); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111207
	packetKeys(0x2A610886,0x3E09165E,0x57C11888); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111214
	packetKeys(0x5151306B,0x7AE32886,0x53060628); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111220
	packetKeys(0x05D53871,0x7D0027B4,0x29975333); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20111228
	packetKeys(0x0FF87E93,0x6CFF7860,0x3A3D1DEC); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120104
	packetKeys(0x262034A1,0x674542A5,0x73A50BA5); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120111
	packetKeys(0x2B412AFC,0x4FF94487,0x6705339D); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120120
	packetKeys(0x504345D0,0x3D427B1B,0x794C2DCC); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120202
	packetKeys(0x2CFC0A71,0x2BA91D8D,0x087E39E0); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120207
	packetKeys(0x1D373F5D,0x5ACD604D,0x1C4D7C4D); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120214
	packetKeys(0x7A255EFA,0x30977276,0x2D4A0448); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120229
	packetKeys(0x520B4C64,0x2800407D,0x47651458); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120307
	packetKeys(0x382A6DEF,0x5CBE7202,0x61F46637); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120314
	packetKeys(0x689C1729,0x11812639,0x60F82967); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120321
	packetKeys(0x21F9683F,0x710C5CA5,0x1FD910E9); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120328
	packetKeys(0x75B8553B,0x37F20B12,0x385C2B40); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120404
	packetKeys(0x0036310C,0x2DCD0BED,0x1EE62A78); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120410
	packetKeys(0x01581359,0x452D6FFA,0x6AFB6E2E); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120418
	packetKeys(0x01540E48,0x13041224,0x31247924); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120424
	packetKeys(0x411D1DBB,0x4CBA4848,0x1A432FC4); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120509
	packetKeys(0x16CF3301,0x1F472B9B,0x0B4A3CD2); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120515
	packetKeys(0x4A715EF9,0x79103E4F,0x405C1238); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120525
	packetKeys(0x70EB4CCB,0x0487713C,0x398D4B08); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120605
	packetKeys(0x68CA3080,0x31B74BDD,0x505208F1); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120612
	packetKeys(0x32E45D64,0x35643564,0x35643564); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120618
	packetKeys(0x261F261F,0x261F261F,0x261F261F); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120702
	packetKeys(0x25733B31,0x53486CFD,0x398649BD); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20120716
	packetKeys(0x76052205,0x22052205,0x22052205); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130320
	packetKeys(0x3F094C49,0x55F86C1E,0x58AA359A); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130514
	packetKeys(0x75794A38,0x58A96BC1,0x296E6FB8); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130522
	packetKeys(0x6948050B,0x06511D9D,0x725D4DF1); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130529
	packetKeys(0x023A6C87,0x14BF1F1E,0x5CC70CC9); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130605
	packetKeys(0x646E08D9,0x5F153AB5,0x61B509B5); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130612
	packetKeys(0x6D166F66,0x3C000FCF,0x295B0FCB); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130618
	packetKeys(0x434115DE,0x34A10FE9,0x6791428E); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130626
	packetKeys(0x38F453EF,0x6A040FD8,0X65BD6668); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130703
	packetKeys(0x4FF90E23,0x0F1432F2,0x4CFA1EDA); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130807
	packetKeys(0x7E241DE0,0x5E805580,0x3D807D80); /* Thanks to Shakto */
#endif

#if PACKETVER >= 20130814
	packetKeys(0x23A23148,0x0C41420E,0x53785AD7); /* Themon */
#endif

#if PACKETVER >= 20131218
	packetKeys(0x6A596301,0x76866D0E,0x32294A45);
#endif

#if PACKETVER >= 20131223
	packetKeys(0x631C511C,0x111C111C,0x111C111C);
#endif

#if PACKETVER >= 20131230
	packetKeys(0x611B7097,0x01F957A1,0x768A0FCB);
#endif

// 2014 Packet Keys

#if PACKETVER >= 20140115
	packetKeys(0x63224335,0x0F3A1F27,0x6D217B24); /* Thanks to Yommy */
#endif

#if PACKETVER >= 20140205
	packetKeys(0x63DC7BDC,0x7BDC7BDC,0x7BDC7BDC); /* Themon */
#endif

#if PACKETVER >= 20140305
	packetKeys(0x116763F2,0x41117DAC,0x7FD13C45); /* Themon */
#endif

#if PACKETVER >= 20140402
	packetKeys(0x15D3271C,0x004D725B,0x111A3A37); /* Themon */
#endif

#if PACKETVER >= 20140416
	packetKeys(0x04810281,0x42814281,0x42814281); /* Themon */
#endif

#if PACKETVER >= 20141016
	packetKeys(0x2DFF467C,0x444B37EE,0x2C1B634F); /* YomRawr */
#endif

#if PACKETVER >= 20141022
	packetKeys(0x290551EA,0x2B952C75,0x2D67669B); /* YomRawr */
#endif

// 2015 Packet Keys

#if PACKETVER >= 20150513
	packetKeys(0x62C86D09,0x75944F17,0x112C133D); /* Dastgir */
#endif

#if defined(OBFUSCATIONKEY1) && defined(OBFUSCATIONKEY2) && defined(OBFUSCATIONKEY3)
	packetKeys(OBFUSCATIONKEY1,OBFUSCATIONKEY2,OBFUSCATIONKEY3);
#endif

#endif /* MAP_PACKETS_H */
