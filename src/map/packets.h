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
	#define packet(a,...)
#endif

/*
 * packet syntax
 * - packet(packet_id)
 * OR
 * - packet(packet_id,function,offset ( specifies the offset of a packet field in bytes from the begin of the packet),...)
 * - Example: packet(0x0072,clif->pWantToConnection,2,6,10,14,18);
 */

packet(0x0072,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
packet(0x007d,clif->pLoadEndAck,0);
packet(0x007e,clif->pTickSend,2);  // CZ_REQUEST_TIME
packet(0x0085,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
packet(0x0089,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
packet(0x008c,clif->pGlobalMessage,2,4);
packet(0x0090,clif->pNpcClicked,2);
packet(0x0094,clif->pGetCharNameRequest,2);  // CZ_REQNAME
packet(0x0096,clif->pWisMessage,2,4,28);
packet(0x0099,clif->pBroadcast,2,4);
packet(0x009b,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
packet(0x009f,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
packet(0x00a2,clif->pDropItem,2,4);  // CZ_ITEM_THROW
packet(0x00a7,clif->pUseItem,2,4);
packet(0x00a9,clif->pEquipItem,2,4);
packet(0x00ab,clif->pUnequipItem,2);
packet(0x00b2,clif->pRestart,2);
packet(0x00b8,clif->pNpcSelectMenu,2,6);
packet(0x00b9,clif->pNpcNextClicked,2);
packet(0x00bb,clif->pStatusUp,2,4);
packet(0x00bf,clif->pEmotion,2);
packet(0x00c1,clif->pHowManyConnections,0);
packet(0x00c5,clif->pNpcBuySellSelected,2,6);
packet(0x00c8,clif->pNpcBuyListSend,2,4);
packet(0x00c9,clif->pNpcSellListSend,2,4);
packet(0x00cc,clif->pGMKick,2);
packet(0x00ce,clif->pGMKickAll,0);
packet(0x00cf,clif->pPMIgnore,2,26);
packet(0x00d0,clif->pPMIgnoreAll,2);
packet(0x00d3,clif->pPMIgnoreList,0);
packet(0x00d5,clif->pCreateChatRoom,2,4,6,7,15);
packet(0x00d9,clif->pChatAddMember,2,6);
packet(0x00de,clif->pChatRoomStatusChange,2,4,6,7,15);
packet(0x00e0,clif->pChangeChatOwner,2,6);
packet(0x00e2,clif->pKickFromChat,2);
packet(0x00e3,clif->pChatLeave,0);
packet(0x00e4,clif->pTradeRequest,2);
packet(0x00e6,clif->pTradeAck,2);
packet(0x00e8,clif->pTradeAddItem,2,4);
packet(0x00eb,clif->pTradeOk,0);
packet(0x00ed,clif->pTradeCancel,0);
packet(0x00ef,clif->pTradeCommit,0);
packet(0x00f3,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
packet(0x00f5,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
packet(0x00f7,clif->pCloseKafra,0);
packet(0x00f9,clif->pCreateParty,2);
packet(0x00fc,clif->pPartyInvite,2);
packet(0x00ff,clif->pReplyPartyInvite,2,6);
packet(0x0100,clif->pLeaveParty,0);
packet(0x0102,clif->pPartyChangeOption,2);
packet(0x0103,clif->pRemovePartyMember,2,6);
packet(0x0108,clif->pPartyMessage,2,4);
packet(0x0112,clif->pSkillUp,2);
packet(0x0113,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
packet(0x0116,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
packet(0x0118,clif->pStopAttack,0);
packet(0x011b,clif->pUseSkillMap,2,4);
packet(0x011d,clif->pRequestMemo,0);
packet(0x0126,clif->pPutItemToCart,2,4);
packet(0x0127,clif->pGetItemFromCart,2,4);
packet(0x0128,clif->pMoveFromKafraToCart,2,4);
packet(0x0129,clif->pMoveToKafraFromCart,2,4);
packet(0x012a,clif->pRemoveOption,0);
packet(0x012e,clif->pCloseVending,0);
packet(0x0130,clif->pVendingListReq,2);
packet(0x0134,clif->pPurchaseReq,2,4,8);
packet(0x013f,clif->pGM_Monster_Item,2);
packet(0x0140,clif->pMapMove,2,18,20);
packet(0x0143,clif->pNpcAmountInput,2,6);
packet(0x0146,clif->pNpcCloseClicked,2);
packet(0x0149,clif->pGMReqNoChat,2,6,7);
packet(0x014d,clif->pGuildCheckMaster,0);
packet(0x014f,clif->pGuildRequestInfo,2);
packet(0x0151,clif->pGuildRequestEmblem,2);
packet(0x0153,clif->pGuildChangeEmblem,2,4);
packet(0x0155,clif->pGuildChangeMemberPosition,2);
packet(0x0159,clif->pGuildLeave,2,6,10,14);
packet(0x015b,clif->pGuildExpulsion,2,6,10,14);
packet(0x015d,clif->pGuildBreak,2);
packet(0x0161,clif->pGuildChangePositionInfo,2);
packet(0x0165,clif->pCreateGuild,6);
packet(0x0168,clif->pGuildInvite,2);
packet(0x016b,clif->pGuildReplyInvite,2,6);
packet(0x016e,clif->pGuildChangeNotice,2,6,66);
packet(0x0170,clif->pGuildRequestAlliance,2);
packet(0x0172,clif->pGuildReplyAlliance,2,6);
packet(0x0178,clif->pItemIdentify,2);
packet(0x017a,clif->pUseCard,2);
packet(0x017c,clif->pInsertCard,2,4);
packet(0x017e,clif->pGuildMessage,2,4);
packet(0x0180,clif->pGuildOpposition,2);
packet(0x0183,clif->pGuildDelAlliance,2,6);
packet(0x018a,clif->pQuitGame,0);
packet(0x018e,clif->pProduceMix,2,4,6,8);
packet(0x0190,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
packet(0x0193,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
packet(0x0197,clif->pResetChar,2);
packet(0x0198,clif->pGMChangeMapType,2,4,6);
packet(0x019c,clif->pLocalBroadcast,2,4);
packet(0x019d,clif->pGMHide,0);
packet(0x019f,clif->pCatchPet,2);
packet(0x01a1,clif->pPetMenu,2);
packet(0x01a5,clif->pChangePetName,2);
packet(0x01a7,clif->pSelectEgg,2);
packet(0x01a9,clif->pSendEmotion,2);
packet(0x01ae,clif->pSelectArrow,2);
packet(0x01af,clif->pChangeCart,2);
packet(0x01b2,clif->pOpenVending,2,4,84,85);
packet(0x01ba,clif->pGMShift,2);
packet(0x01bb,clif->pGMShift,2);
packet(0x01bc,clif->pGMRecall,2);
packet(0x01bd,clif->pGMRecall,2);
packet(0x01c0,clif->pReqRemainTime);
packet(0x01ce,clif->pAutoSpell,2);
packet(0x01d5,clif->pNpcStringInput,2,4,8);
packet(0x01df,clif->pGMReqAccountName,2);
packet(0x01e7,clif->pNoviceDoriDori,0);
packet(0x01e8,clif->pCreateParty2,2);
packet(0x01ed,clif->pNoviceExplosionSpirits,0);
packet(0x01f7,clif->pAdopt_reply,0);
packet(0x01f9,clif->pAdopt_request,0);
packet(0x01fd,clif->pRepairItem,2);
packet(0x0202,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
packet(0x0203,clif->pFriendsListRemove,2,6);
packet(0x0208,clif->pFriendsListReply,2,6,10);
packet(0x974,clif->cancelmergeitem);
packet(0x96e,clif->ackmergeitems);

//2004-07-05aSakexe
#if PACKETVER >= 20040705
	packet(0x0072,clif->pWantToConnection,5,9,13,17,21);
	packet(0x0085,clif->pWalkToXY,5);
	packet(0x00a7,clif->pUseItem,5,9);
	packet(0x0113,clif->pUseSkillToId,4,9,11);
	packet(0x0116,clif->pUseSkillToPos,4,9,11,13);
	packet(0x0190,clif->pUseSkillToPosMoreInfo,4,9,11,13,15);
	packet(0x0208,clif->pFriendsListReply,2,6,10);
#endif

//2004-07-13aSakexe
#if PACKETVER >= 20040713
	packet(0x0072,clif->pWantToConnection,12,22,30,34,38);
	packet(0x0085,clif->pWalkToXY,6);
	packet(0x009b,clif->pChangeDir,5,12);
	packet(0x009f,clif->pTakeItem,6);
	packet(0x00a7,clif->pUseItem,6,13);
	packet(0x0113,clif->pUseSkillToId,7,9,15);
	packet(0x0116,clif->pUseSkillToPos,7,9,15,17);
	packet(0x0190,clif->pUseSkillToPosMoreInfo,7,9,15,17,19);
#endif

//2004-07-26aSakexe
#if PACKETVER >= 20040726
	packet(0x0072,clif->pDropItem,5,12);
	packet(0x007e,clif->pWantToConnection,12,18,24,28,32);
	packet(0x0085,clif->pUseSkillToId,7,12,16);
	packet(0x0089,clif->pGetCharNameRequest,11);
	packet(0x008c,clif->pUseSkillToPos,3,6,17,21);
	packet(0x0094,clif->pTakeItem,6);
	packet(0x009b,clif->pWalkToXY,3);
	packet(0x009f,clif->pChangeDir,5,12);
	packet(0x00a2,clif->pUseSkillToPosMoreInfo,3,6,17,21,23);
	packet(0x00a7,clif->pSolveCharName,8);
	packet(0x00f3,clif->pGlobalMessage,2,4);
	packet(0x00f5,clif->pUseItem,6,12);
	packet(0x00f7,clif->pTickSend,6);
	packet(0x0113,clif->pMoveToKafra,5,12);
	packet(0x0116,clif->pCloseKafra,0);
	packet(0x0190,clif->pMoveFromKafra,10,22);
	packet(0x0193,clif->pActionRequest,3,8);
#endif

//2004-08-09aSakexe
#if PACKETVER >= 20040809
	packet(0x0072,clif->pDropItem,8,15);
	packet(0x007e,clif->pWantToConnection,9,21,28,32,36);
	packet(0x0085,clif->pUseSkillToId,11,18,22);
	packet(0x0089,clif->pGetCharNameRequest,8);
	packet(0x008c,clif->pUseSkillToPos,5,15,29,38);
	packet(0x0094,clif->pTakeItem,9);
	packet(0x009b,clif->pWalkToXY,12);
	packet(0x009f,clif->pChangeDir,7,11);
	packet(0x00a2,clif->pUseSkillToPosMoreInfo,5,15,29,38,40);
	packet(0x00a7,clif->pSolveCharName,7);
	packet(0x00f5,clif->pUseItem,9,20);
	packet(0x00f7,clif->pTickSend,9);
	packet(0x0113,clif->pMoveToKafra,5,19);
	packet(0x0190,clif->pMoveFromKafra,11,22);
	packet(0x0193,clif->pActionRequest,7,17);
#endif

//2004-08-16aSakexe
#if PACKETVER >= 20040816
	packet(0x0212,clif->pGMRc,2);
	packet(0x0213,clif->pCheck,2);
#endif

//2004-08-17aSakexe
#if PACKETVER >= 20040817
	packet(0x020f,clif->pPVPInfo,2,6);
#endif

//2004-09-06aSakexe
#if PACKETVER >= 20040906
	packet(0x0072,clif->pUseItem,9,20);
	packet(0x007e,clif->pMoveToKafra,3,15);
	packet(0x0085,clif->pActionRequest,9,22);
	packet(0x0089,clif->pWalkToXY,6);
	packet(0x008c,clif->pUseSkillToPosMoreInfo,10,14,18,23,25);
	packet(0x0094,clif->pDropItem,6,15);
	packet(0x009b,clif->pGetCharNameRequest,10);
	packet(0x009f,clif->pGlobalMessage,2,4);
	packet(0x00a2,clif->pSolveCharName,10);
	packet(0x00a7,clif->pUseSkillToPos,10,14,18,23);
	packet(0x00f3,clif->pChangeDir,4,9);
	packet(0x00f5,clif->pWantToConnection,7,15,25,29,33);
	packet(0x00f7,clif->pCloseKafra,0);
	packet(0x0113,clif->pTakeItem,7);
	packet(0x0116,clif->pTickSend,7);
	packet(0x0190,clif->pUseSkillToId,9,15,18);
	packet(0x0193,clif->pMoveFromKafra,3,13);
#endif

//2004-09-20aSakexe
#if PACKETVER >= 20040920
	packet(0x0072,clif->pUseItem,10,14);
	packet(0x007e,clif->pMoveToKafra,6,21);
	packet(0x0085,clif->pActionRequest,3,8);
	packet(0x0089,clif->pWalkToXY,11);
	packet(0x008c,clif->pUseSkillToPosMoreInfo,16,20,23,27,29);
	packet(0x0094,clif->pDropItem,12,17);
	packet(0x009b,clif->pGetCharNameRequest,6);
	packet(0x00a2,clif->pSolveCharName,6);
	packet(0x00a7,clif->pUseSkillToPos,6,20,23,27);
	packet(0x00f3,clif->pChangeDir,8,17);
	packet(0x00f5,clif->pWantToConnection,10,17,23,27,31);
	packet(0x0113,clif->pTakeItem,10);
	packet(0x0116,clif->pTickSend,10);
	packet(0x0190,clif->pUseSkillToId,4,7,10);
	packet(0x0193,clif->pMoveFromKafra,4,8);
#endif

//2004-10-05aSakexe
#if PACKETVER >= 20041005
	packet(0x0072,clif->pUseItem,6,13);
	packet(0x007e,clif->pMoveToKafra,5,12);
	packet(0x0089,clif->pWalkToXY,3);
	packet(0x008c,clif->pUseSkillToPosMoreInfo,2,6,17,21,23);
	packet(0x0094,clif->pDropItem,5,12);
	packet(0x009b,clif->pGetCharNameRequest,11);
	packet(0x00a2,clif->pSolveCharName,8);
	packet(0x00a7,clif->pUseSkillToPos,3,6,17,21);
	packet(0x00f3,clif->pChangeDir,5,12);
	packet(0x00f5,clif->pWantToConnection,12,18,24,28,32);
	packet(0x0113,clif->pTakeItem,6);
	packet(0x0116,clif->pTickSend,6);
	packet(0x0190,clif->pUseSkillToId,7,12,16);
	packet(0x0193,clif->pMoveFromKafra,10,22);
#endif

//2004-10-25aSakexe
#if PACKETVER >= 20041025
	packet(0x0072,clif->pUseItem,5,9);
	packet(0x007e,clif->pMoveToKafra,6,9);
	packet(0x0085,clif->pActionRequest,4,14);
	packet(0x008c,clif->pUseSkillToPosMoreInfo,6,9,23,26,28);
	packet(0x0094,clif->pDropItem,6,10);
	packet(0x009b,clif->pGetCharNameRequest,6);
	packet(0x00a2,clif->pSolveCharName,12);
	packet(0x00a7,clif->pUseSkillToPos,6,9,23,26);
	packet(0x00f3,clif->pChangeDir,6,14);
	packet(0x00f5,clif->pWantToConnection,5,14,20,24,28);
	packet(0x0113,clif->pTakeItem,5);
	packet(0x0116,clif->pTickSend,5);
	packet(0x0190,clif->pUseSkillToId,4,10,22);
	packet(0x0193,clif->pMoveFromKafra,12,18);
#endif

//2004-11-08aSakexe
#if PACKETVER >= 20041108
	packet(0x0217,clif->pBlacksmith,0);
	packet(0x0218,clif->pAlchemist,0);
#endif

//2004-11-15aSakexe
#if PACKETVER >= 20041115
	packet(0x021d,clif->pLessEffect,2);
#endif

//2004-11-29aSakexe
#if PACKETVER >= 20041129
	packet(0x0072,clif->pUseSkillToId,8,12,18);
	packet(0x007e,clif->pUseSkillToPos,4,9,22,28);
	packet(0x0085,clif->pGlobalMessage,2,4);
	packet(0x0089,clif->pTickSend,3);
	packet(0x008c,clif->pGetCharNameRequest,9);
	packet(0x0094,clif->pMoveToKafra,4,10);
	packet(0x009b,clif->pCloseKafra,0);
	packet(0x009f,clif->pActionRequest,6,17);
	packet(0x00a2,clif->pTakeItem,3);
	packet(0x00a7,clif->pWalkToXY,4);
	packet(0x00f3,clif->pChangeDir,3,7);
	packet(0x00f5,clif->pWantToConnection,3,10,20,24,28);
	packet(0x00f7,clif->pSolveCharName,10);
	packet(0x0113,clif->pUseSkillToPosMoreInfo,4,9,22,28,30);
	packet(0x0116,clif->pDropItem,4,10);
	packet(0x0190,clif->pUseItem,3,11);
	packet(0x0193,clif->pMoveFromKafra,4,17);
	packet(0x0222,clif->pWeaponRefine,2);
#endif

//2005-01-10bSakexe
#if PACKETVER >= 20050110
	packet(0x0072,clif->pUseSkillToId,8,16,22);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,10,18,22,32,34);
	packet(0x0085,clif->pChangeDir,12,22);
	packet(0x0089,clif->pTickSend,5);
	packet(0x008c,clif->pGetCharNameRequest,4);
	packet(0x0094,clif->pMoveToKafra,10,16);
	packet(0x009b,clif->pWantToConnection,3,12,23,27,31);
	packet(0x009f,clif->pUseItem,5,13);
	packet(0x00a2,clif->pSolveCharName,7);
	packet(0x00a7,clif->pWalkToXY,10);
	packet(0x00f3,clif->pGlobalMessage,2,4);
	packet(0x00f5,clif->pTakeItem,5);
	packet(0x00f7,clif->pMoveFromKafra,11,17);
	packet(0x0113,clif->pUseSkillToPos,10,18,22,32);
	packet(0x0116,clif->pDropItem,15,18);
	packet(0x0190,clif->pActionRequest,9,19);
	packet(0x0193,clif->pCloseKafra,0);
#endif

//2005-03-28aSakexe
#if PACKETVER >= 20050328
	packet(0x0225,clif->pTaekwon,0);
#endif

//2005-04-25aSakexe
#if PACKETVER >= 20050425
	packet(0x022d,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0232,clif->pHomMoveTo,2,6);
	packet(0x0233,clif->pHomAttack,2,6,10);
	packet(0x0234,clif->pHomMoveToMaster,2);
#endif

//2005-05-09aSakexe
#if PACKETVER >= 20050509
	packet(0x0072,clif->pUseSkillToId,6,10,21);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,5,9,12,20,22);
	packet(0x0085,clif->pChangeDir,7,10);
	packet(0x0089,clif->pTickSend,4);
	packet(0x008c,clif->pGetCharNameRequest,7);
	packet(0x0094,clif->pMoveToKafra,7,10);
	packet(0x009b,clif->pWantToConnection,4,9,17,21,25);
	packet(0x009f,clif->pUseItem,4,10);
	packet(0x00a2,clif->pSolveCharName,11);
	packet(0x00a7,clif->pWalkToXY,5);
	packet(0x00f5,clif->pTakeItem,4);
	packet(0x00f7,clif->pMoveFromKafra,14,18);
	packet(0x0113,clif->pUseSkillToPos,5,9,12,20);
	packet(0x0116,clif->pDropItem,5,8);
	packet(0x0190,clif->pActionRequest,5,18);
#endif

//2005-05-30aSakexe
#if PACKETVER >= 20050530
	packet(0x0237,clif->pRankingPk,0);
#endif

//2005-06-08aSakexe
#if PACKETVER >= 20050608
	packet(0x0217,clif->pBlacksmith,0);
	packet(0x0231,clif->pChangeHomunculusName,0);
	packet(0x023b,clif->pStoragePassword,2,4,20);
#endif

//2005-06-28aSakexe
#if PACKETVER >= 20050628
	packet(0x0072,clif->pUseSkillToId,6,17,30);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,12,15,18,31,33);
	packet(0x0085,clif->pChangeDir,8,16);
	packet(0x0089,clif->pTickSend,9);
	packet(0x008c,clif->pGetCharNameRequest,4);
	packet(0x0094,clif->pMoveToKafra,16,27);
	packet(0x009b,clif->pWantToConnection,9,15,23,27,31);
	packet(0x009f,clif->pUseItem,9,15);
	packet(0x00a2,clif->pSolveCharName,5);
	packet(0x00a7,clif->pWalkToXY,8);
	packet(0x00f5,clif->pTakeItem,9);
	packet(0x00f7,clif->pMoveFromKafra,11,14);
	packet(0x0113,clif->pUseSkillToPos,12,15,18,31);
	packet(0x0116,clif->pDropItem,3,10);
	packet(0x0190,clif->pActionRequest,11,23);
#endif

//2005-07-18aSakexe
#if PACKETVER >= 20050718
	packet(0x0072,clif->pUseSkillToId,5,11,15);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,9,15,23,28,30);
	packet(0x0085,clif->pChangeDir,6,10);
	packet(0x0089,clif->pTickSend,3);
	packet(0x008c,clif->pGetCharNameRequest,7);
	packet(0x0094,clif->pMoveToKafra,12,17);
	packet(0x009b,clif->pWantToConnection,3,13,22,26,30);
	packet(0x009f,clif->pUseItem,3,8);
	packet(0x00a2,clif->pSolveCharName,14);
	packet(0x00a7,clif->pWalkToXY,12);
	packet(0x00f5,clif->pTakeItem,3);
	packet(0x00f7,clif->pMoveFromKafra,5,9);
	packet(0x0113,clif->pUseSkillToPos,9,15,23,28);
	packet(0x0116,clif->pDropItem,6,10);
	packet(0x0190,clif->pActionRequest,5,20);
	packet(0x023f,clif->pMail_refreshinbox,0);
	packet(0x0241,clif->pMail_read,2);
	packet(0x0243,clif->pMail_delete,2);
	packet(0x0244,clif->pMail_getattach,2);
	packet(0x0246,clif->pMail_winopen,2);
	packet(0x0247,clif->pMail_setattach,2,4);
	packet(0x024b,clif->pAuction_cancelreg,0);
	packet(0x024c,clif->pAuction_setitem,0);
	packet(0x024e,clif->pAuction_cancel,0);
	packet(0x024f,clif->pAuction_bid,0);
#endif

//2005-07-19bSakexe
#if PACKETVER >= 20050719
	packet(0x0072,clif->pUseSkillToId,6,17,30);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,12,15,18,31,33);
	packet(0x0085,clif->pChangeDir,8,16);
	packet(0x0089,clif->pTickSend,9);
	packet(0x008c,clif->pGetCharNameRequest,4);
	packet(0x0094,clif->pMoveToKafra,16,27);
	packet(0x009b,clif->pWantToConnection,9,15,23,27,31);
	packet(0x009f,clif->pUseItem,9,15);
	packet(0x00a2,clif->pSolveCharName,5);
	packet(0x00a7,clif->pWalkToXY,8);
	packet(0x00f5,clif->pTakeItem,9);
	packet(0x00f7,clif->pMoveFromKafra,11,14);
	packet(0x0113,clif->pUseSkillToPos,12,15,18,31);
	packet(0x0116,clif->pDropItem,3,10);
	packet(0x0190,clif->pActionRequest,11,23);
#endif

//2005-08-08aSakexe
#if PACKETVER >= 20050808
	packet(0x024d,clif->pAuction_register,0);
#endif

//2005-08-17aSakexe
#if PACKETVER >= 20050817
	packet(0x0254,clif->pFeelSaveOk,0);
#endif

//2005-08-29aSakexe
#if PACKETVER >= 20050829
	packet(0x0248,clif->pMail_send,2,4,28,68);
#endif

//2005-10-10aSakexe
#if PACKETVER >= 20051010
	packet(0x025b,clif->pCooking,0);
#endif

//2005-10-13aSakexe
#if PACKETVER >= 20051013
	packet(0x025c,clif->pAuction_buysell,0);
#endif

//2005-10-17aSakexe
#if PACKETVER >= 20051017
	packet(0x025d,clif->pAuction_close,0);
#endif

//2005-11-07aSakexe
#if PACKETVER >= 20051107
	packet(0x024e,clif->pAuction_cancel,0);
	packet(0x0251,clif->pAuction_search,0);
#endif

//2006-03-13aSakexe
#if PACKETVER >= 20060313
	packet(0x0273,clif->pMail_return,2,6);
#endif

//2006-03-27aSakexe
#if PACKETVER >= 20060327
	packet(0x0072,clif->pUseSkillToId,11,18,22);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,5,15,29,38,40);
	packet(0x0085,clif->pChangeDir,7,11);
	packet(0x008c,clif->pGetCharNameRequest,8);
	packet(0x0094,clif->pMoveToKafra,5,19);
	packet(0x009b,clif->pWantToConnection,9,21,28,32,36);
	packet(0x009f,clif->pUseItem,9,20);
	packet(0x00a2,clif->pSolveCharName,7);
	packet(0x00a7,clif->pWalkToXY,12);
	packet(0x00f5,clif->pTakeItem,9);
	packet(0x00f7,clif->pMoveFromKafra,11,22);
	packet(0x0113,clif->pUseSkillToPos,5,15,29,38);
	packet(0x0116,clif->pDropItem,8,15);
	packet(0x0190,clif->pActionRequest,7,17);
#endif

//2006-04-24aSakexe to 2007-01-02aSakexe
#if PACKETVER >= 20060424
	packet(0x0292,clif->pAutoRevive,0);
	packet(0x029f,clif->pmercenary_action,0);
#endif

//2007-01-08aSakexe
#if PACKETVER >= 20070108
	packet(0x0072,clif->pUseSkillToId,10,14,26);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,10,19,23,38,40);
	packet(0x0085,clif->pChangeDir,10,13);
	packet(0x0089,clif->pTickSend,7);
	packet(0x008c,clif->pGetCharNameRequest,13);
	packet(0x0094,clif->pMoveToKafra,4,13);
	packet(0x009b,clif->pWantToConnection,7,21,26,30,34);
	packet(0x009f,clif->pUseItem,7,17);
	packet(0x00a2,clif->pSolveCharName,6);
	packet(0x00a7,clif->pWalkToXY,5);
	packet(0x00f5,clif->pTakeItem,7);
	packet(0x00f7,clif->pMoveFromKafra,3,11);
	packet(0x0113,clif->pUseSkillToPos,10,19,23,38);
	packet(0x0116,clif->pDropItem,11,17);
	packet(0x0190,clif->pActionRequest,4,9);
#endif

//2007-02-12aSakexe
#if PACKETVER >= 20070212
	packet(0x0072,clif->pUseSkillToId,6,10,21);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,5,9,12,20,22);
	packet(0x0085,clif->pChangeDir,7,10);
	packet(0x0089,clif->pTickSend,4);
	packet(0x008c,clif->pGetCharNameRequest,7);
	packet(0x0094,clif->pMoveToKafra,7,10);
	packet(0x009b,clif->pWantToConnection,4,9,17,21,25);
	packet(0x009f,clif->pUseItem,4,10);
	packet(0x00a2,clif->pSolveCharName,11);
	packet(0x00f5,clif->pTakeItem,4);
	packet(0x00f7,clif->pMoveFromKafra,14,18);
	packet(0x0113,clif->pUseSkillToPos,5,9,12,20);
	packet(0x0116,clif->pDropItem,5,8);
	packet(0x0190,clif->pActionRequest,5,18);
#endif

//2007-05-07aSakexe
#if PACKETVER >= 20070507
	packet(0x01fd,clif->pRepairItem,2);
#endif

//2007-02-27aSakexe to 2007-10-02aSakexe
#if PACKETVER >= 20070227
	packet(0x0288,clif->pcashshop_buy,2,4,6);
	packet(0x02b6,clif->pquestStateAck,2,6);
	packet(0x02ba,clif->pHotkey,2,4,5,9);
	packet(0x02c4,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x02c7,clif->pReplyPartyInvite2,2,6);
	packet(0x02c8,clif->pPartyTick,2);
	packet(0x02cf,clif->pMemorialDungeonCommand);
	packet(0x02d6,clif->pViewPlayerEquip,2);
	packet(0x02d8,clif->p_cz_config,6);
	packet(0x02db,clif->pBattleChat,2,4);
#endif

//2008-01-02aSakexe
#if PACKETVER >= 20080102
	packet(0x01df,clif->pGMReqAccountName,2);
#endif

//2008-03-18aSakexe
#if PACKETVER >= 20080318
	packet(0x02f1,clif->pProgressbar,0);
#endif

//2008-09-10aSakexe
#if PACKETVER >= 20080910
	packet(0x0436,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0438,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0439,clif->pUseItem,2,4);
#endif

//2008-12-10aSakexe
#if PACKETVER >= 20081210
	packet(0x0443,clif->pSkillSelectMenu,2,6);
#endif

//2008-08-27aRagexeRE
#if PACKETVER >= 20080827
	packet(0x0072,clif->pUseSkillToId,9,15,18);
	packet(0x007e,clif->pUseSkillToPosMoreInfo,10,14,18,23,25);
	packet(0x0085,clif->pChangeDir,4,9);
	packet(0x0089,clif->pTickSend,7);
	packet(0x008c,clif->pGetCharNameRequest,10);
	packet(0x0094,clif->pMoveToKafra,3,15);
	packet(0x009b,clif->pWantToConnection,7,15,25,29,33);
	packet(0x009f,clif->pUseItem,7,20);
	packet(0x00a2,clif->pSolveCharName,10);
	packet(0x00a7,clif->pWalkToXY,6);
	packet(0x00f5,clif->pTakeItem,7);
	packet(0x00f7,clif->pMoveFromKafra,3,13);
	packet(0x0113,clif->pUseSkillToPos,10,14,18,23);
	packet(0x0116,clif->pDropItem,6,15);
	packet(0x0190,clif->pActionRequest,9,22);
#endif

//2008-09-10aRagexeRE
#if PACKETVER >= 20080910
	packet(0x0436,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0438,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0439,clif->pUseItem,2,4);
#endif

#if PACKETVER_MAIN_NUM >= 20090406 || PACKETVER_RE_NUM >= 20090408 || PACKETVER_SAK_NUM >= 20090408 || defined(PACKETVER_ZERO)
	packet(0x044a,clif->pClientVersion);
#endif

// 2009-05-20aRagexe, 2009-05-20aRagexeRE
#if PACKETVER >= 20090520
// new packets
	packet(0x0447,clif->p_cz_blocking_play_cancel); // PACKET_CZ_BLOCKING_PLAY_CANCEL
#endif

//2009-06-03aRagexeRE
#if PACKETVER >= 20090603
	packet(0x07d7,clif->pPartyChangeOption,2,6,7);
	packet(0x07da,clif->pPartyChangeLeader,2);
#endif

//2009-08-18aRagexeRE
#if PACKETVER >= 20090818
	packet(0x07e4,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
#endif

//2009-10-27aRagexeRE
#if PACKETVER >= 20091027
	packet(0x07f5,clif->pGMFullStrip,2);
#endif

//2009-12-22aRagexeRE
#if PACKETVER >= 20091222
	packet(0x0802,clif->pPartyBookingRegisterReq,2,4,6); // Booking System
	packet(0x0806,clif->pPartyBookingDeleteReq,2);// Booking System
#endif

	//2009-12-29aRagexeRE
#if PACKETVER >= 20091229
	packet(0x0804,clif->pPartyBookingSearchReq,2,4,6,8,12);// Booking System
	packet(0x0806,clif->pPartyBookingDeleteReq,0);// Booking System
	packet(0x0808,clif->pPartyBookingUpdateReq,2); // Booking System
#endif

	//2010-01-05aRagexeRE
#if PACKETVER >= 20100105
	packet(0x0801,clif->pPurchaseReq2,2,4,8,12);
#endif

//2010-03-03aRagexeRE
#if PACKETVER >= 20100303
	packet(0x0811,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

	//2010-04-20aRagexeRE
#if PACKETVER >= 20100420
	packet(0x0815,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0817,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0819,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
#endif

	//2010-06-01aRagexeRE
#if PACKETVER >= 20100601
	packet(0x0835,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
#endif

	//2010-06-08aRagexeRE
#if PACKETVER >= 20100608
	packet(0x0838,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x083B,clif->pCloseSearchStoreInfo,0);
	packet(0x083C,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
#endif

//2010-08-03aRagexeRE
#if PACKETVER >= 20100803
	packet(0x0842,clif->pGMRecall2,2);
	packet(0x0843,clif->pGMRemove2,2);
#endif

//2010-11-24aRagexeRE
#if PACKETVER >= 20101124
	packet(0x0288,clif->pcashshop_buy,4,8);
	packet(0x0436,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x035f,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0360,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0361,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0362,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0363,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0364,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0365,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0366,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0367,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0369,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
#endif

// 2011-01-25aRagexe
#if PACKETVER >= 20110125
// shuffle packets not added
// new packets
	packet(0x08b5,clif->pDull,2); // CZ_TRYCOLLECTION
#endif

// 2011-01-31aRagexe
#if PACKETVER >= 20110131
// shuffle packets not added
// new packets
	packet(0x02f3,clif->pDull); // CZ_IRMAIL_SEND
	packet(0x02f6,clif->pDull,2); // CZ_IRMAIL_LIST
#endif

// 2011-02-22aRagexe
#if PACKETVER >= 20110222
// shuffle packets not added
// new packets
	packet(0x08c1,clif->pDull); // CZ_MACRO_START
	packet(0x08c2,clif->pDull); // CZ_MACRO_STOP
#endif

// 2011-06-14aRagexe
#if PACKETVER >= 20110614
// shuffle packets not added
// new packets
	packet(0x08c9,clif->pCashShopSchedule,0);
#endif

//2011-07-18aRagexe (Thanks to Yommy!)
#if PACKETVER >= 20110718
// shuffle packets not added
	packet(0x0844,clif->pCashShopOpen,2);/* tell server cashshop window is being open */
	packet(0x084a,clif->pCashShopClose,2);/* tell server cashshop window is being closed */
	packet(0x0846,clif->pCashShopReqTab,2);
	packet(0x0848,clif->pCashShopBuy,2);
#endif

//2011-10-05aRagexeRE
#if PACKETVER >= 20111005
	packet(0x0364,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0817,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0366,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0815,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0885,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0893,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0897,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0369,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08ad,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x088a,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0838,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0439,clif->pUseItem,2,4);
	packet(0x08d7,clif->pBGQueueRegister,2);
	packet(0x090a,clif->pBGQueueCheckState,2);
	packet(0x08da,clif->pBGQueueRevokeReq,2);
	packet(0x08e0,clif->pBGQueueBattleBeginAck,2);
#endif

//2011-11-02aRagexe
#if PACKETVER >= 20111102
	packet(0x0436,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0898,clif->pHomMenu,4);
	packet(0x0281,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088d,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x083c,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08aa,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x02c4,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0811,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a5,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x0835,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089b,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a1,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x089e,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x08ab,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x088b,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x08a2,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	#ifndef PACKETVER_RE
		packet(0x0835,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
		packet(0x0892,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
		packet(0x0899,clif->pTickSend,2);  // CZ_REQUEST_TIME
	#endif
#endif

//2012-03-07fRagexeRE
#if PACKETVER >= 20120307
	packet(0x086A,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0887,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0890,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0865,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x02C4,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x093B,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0963,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0863,clif->pHomMenu,4);
	packet(0x0861,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0929,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0885,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0889,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0870,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0884,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0439,clif->pUseItem,2,4);
// changed packet sizes
#endif

//2012-04-10aRagexeRE
#if PACKETVER >= 20120410
	packet(0x01FD,clif->pRepairItem,2);
	packet(0x089C,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0885,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0961,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0288,clif->pcashshop_buy,4,8);
	packet(0x091C,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x094B,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0439,clif->pUseItem,2,4);
	packet(0x0945,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0886,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0871,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0938,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0891,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x086C,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08A6,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0889,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0884,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
#ifndef PACKETVER_RE
	packet(0x091D,clif->pPartyBookingRegisterReq,2,4,6);
#else
	packet(0x08E5,clif->pPartyRecruitRegisterReq,2,4);
#endif
	packet(0x08E7,clif->pPartyRecruitSearchReq,2);
	packet(0x08E9,clif->pPartyRecruitDeleteReq,2);
	packet(0x08EB,clif->pPartyRecruitUpdateReq,2);
#ifdef PARTY_RECRUIT
	packet(0x08EF,clif->pDull,2); //bookingignorereq
	packet(0x08F0,clif->pDull,2);
	packet(0x08F1,clif->pDull,2); //bookingjoinpartyreq
#endif
	packet(0x08F5,clif->pDull,2,4); //bookingsummonmember
#ifdef PARTY_RECRUIT
	packet(0x08F9,clif->pDull,2);
#endif
	packet(0x08FB,clif->pDull,2); //bookingcanceljoinparty
	packet(0x0907,clif->pMoveItem,2,4);
#endif

//2012-04-18aRagexeRE [Special Thanks to Judas!]
#if PACKETVER >= 20120418
	packet(0x023B,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0361,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08A8,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x022D,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0202,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x08E5,clif->pPartyRecruitRegisterReq,2,4);
	packet(0x0916,clif->pGuildInvite2,2);
#endif

// 2012-05-02aRagexeRE
#if PACKETVER >= 20120502
// shuffle packets not added
	packet(0x0980,clif->pSelectCart); // CZ_SELECTCART
#endif

#ifndef PACKETVER_RE
#if PACKETVER >= 20120604
// shuffle packets not added
	packet(0x0861,clif->pPartyRecruitRegisterReq,2,4,6);
#endif
#endif

// ========== 2012-07-02aRagexeRE  =============
// - 2012-07-02 is NOT STABLE.
// - The packets are kept here for reference, DONT USE THEM.
#if PACKETVER >= 20120702
	packet(0x0363,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0364,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x085a,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0861,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0862,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0863,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0886,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0889,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x089e,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089f,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08a0,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x094a,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0953,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0960,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
#endif

//2012-07-10
#if PACKETVER >= 20120710
	packet(0x0886,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
#endif

//2012-07-16aRagExe (special thanks to Yommy/Frost!)
#if PACKETVER >= 20120716
	packet(0x0879,clif->pPartyBookingRegisterReq,2,4,6);
	packet(0x023B,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0361,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0819,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x022D,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0439,clif->pUseItem,2,4);
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0940,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0811,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0202,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,clif->pDull); // CZ_GANGSI_RANK
#endif

//2012-07-16aRagExe
#if PACKETVER >= 20120716
// new packets
	packet(0x098d,clif->pClanMessage,2,4); // CZ_CLAN_CHAT
#endif

// 2012-09-25aRagexe
#if PACKETVER >= 20120925
// new packets (not all)
	packet(0x0998,clif->pEquipItem,2,4);
#endif

// 2013-03-06aRagexe
#if PACKETVER >= 20130306
// new packets
	packet(0x09a7,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_DEPOSIT
	packet(0x09a9,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_WITHDRAW
// changed packet sizes
#endif

// 2013-03-13aRagexe
#if PACKETVER >= 20130313
// new packets
	packet(0x09ab,clif->pDull/*,XXX*/); // CZ_REQ_BANKING_CHECK
	packet(0x09ac,clif->pDull/*,XXX*/); // CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ae,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
// changed packet sizes
#endif

//2013-03-20Ragexe (Judas + Yommy)
#if PACKETVER >= 20130320
	// Shuffle Start
	packet(0x088E,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x089B,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0881,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0363,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0897,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0933,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0438,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08AC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0874,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0959,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x085A,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0898,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x094C,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0365,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x092E,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x094E,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0922,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x035F,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0886,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0938,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x085D,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x085D,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0868,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0888,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x086D,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x086F,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093F,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0947,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0890,clif->pDull); // CZ_GANGSI_RANK
	packet(0x095a,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End

	// New Packets (wrong version or packet not exists)
	// New Packets End
#endif

#if PACKETVER >= 20130320
// new packets
// changed packet sizes
	packet(0x09a7,clif->pBankDeposit,2,4,6); // CZ_REQ_BANKING_DEPOSIT
	packet(0x09a9,clif->pBankWithdraw,2,4,6); // CZ_REQ_BANKING_WITHDRAW
	packet(0x09ab,clif->pBankCheck,2,4); // CZ_REQ_BANKING_CHECK
#endif

// 2013-03-27bRagexe
#if PACKETVER >= 20130327
// new packets
	packet(0x09ac,clif->pDull/*,XXX*/); // CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO
	packet(0x09ae,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
// changed packet sizes
#endif

//2013-05-15aRagexe (Shakto)
#if PACKETVER >= 20130515
	// Shuffle Start
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0362,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08A1,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0944,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0887,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x08AC,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x092D,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x092D,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0963,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0943,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0947,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0962,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0931,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x093E,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0862,clif->pDull); // CZ_GANGSI_RANK
	packet(0x08aa,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End
#endif

//2013-05-22Ragexe (Shakto)
#if PACKETVER >= 20130522
	// Shuffle Start
	packet(0x08A2,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x095C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0360,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x07EC,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0925,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x095E,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x089C,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x08A3,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x087E,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0811,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0964,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x08A6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0369,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x093E,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x08AA,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x095B,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0952,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0368,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x086E,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0874,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x089B,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x089B,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x086A,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08A9,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0950,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0362,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0926,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x088E,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08ac,clif->pDull); // CZ_GANGSI_RANK
	packet(0x0965,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	// Shuffle End
#endif

//2013-05-29Ragexe (Shakto)
#if PACKETVER >= 20130529
	packet(0x0890,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0438,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0876,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0897,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0951,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0895,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x08A7,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0938,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0957,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0917,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x085E,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0863,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0937,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x085A,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0918,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0936,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0892,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0964,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0869,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0874,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0874,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0958,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0919,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08A8,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0877,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x023B,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0956,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0888,clif->pDull); // CZ_GANGSI_RANK
	packet(0x088e,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

//2013-06-05Ragexe (Shakto)
#if PACKETVER >= 20130605
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0202,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0365,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x022D,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x023B,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0361,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0883,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x097C,clif->pRanklist);
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,clif->pDull); // CZ_GANGSI_RANK
#endif

//2013-06-12Ragexe (Shakto)
#if PACKETVER >= 20130612
// most shuffle packets used from 20130605
	packet(0x087E,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0919,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0940,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x093A,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0964,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
#endif

//2013-06-18Ragexe (Shakto)
#if PACKETVER >= 20130618
	packet(0x0889,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0951,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x088E,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0930,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x08A6,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0962,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0917,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0885,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0936,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x096A,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x094F,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0944,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0945,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0890,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0363,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0281,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0891,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0862,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x085A,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0932,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x08A7,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x08A7,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0942,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x095B,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0887,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0953,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x02C4,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0864,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0878,clif->pDull); // CZ_GANGSI_RANK
	packet(0x087a,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

//2013-06-26Ragexe (Shakto)
#if PACKETVER >= 20130626
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094D,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x088B,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0952,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0921,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0817,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0365,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0894,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0894,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x08A5,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088C,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0895,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08AB,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0960,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0930,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0860,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x088f,clif->pDull); // CZ_GANGSI_RANK
#endif

//2013-07-03Ragexe (Shakto)
#if PACKETVER >= 20130703
	packet(0x0930,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0202,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0365,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x022D,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0360,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x094A,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0873,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-04-17aRagexe
#if PACKETVER >= 20130417
// new packets
	packet(0x09b4,clif->pDull/*,XXX*/); // CZ_OPEN_BARGAIN_SALE_TOOL
	packet(0x09b6,clif->pBankOpen,2,4); // CZ_REQ_OPEN_BANKING
	packet(0x09b8,clif->pBankClose,2,4); // CZ_REQ_CLOSE_BANKING
// changed packet sizes
#endif

// 2013-04-24aRagexe
#if PACKETVER >= 20130424
// new packets
	packet(0x09ba,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_GUILD_STORAGE
	packet(0x09bc,clif->pDull/*,XXX*/); // CZ_CLOSE_BARGAIN_SALE_TOOL
// changed packet sizes
#endif

// 2013-05-02aRagexe
#if PACKETVER >= 20130502
// new packets
	packet(0x09be,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_GUILD_STORAGE
// changed packet sizes
#endif

// 2013-05-29Ragexe
#if PACKETVER >= 20130529
// new packets
	packet(0x09c3,clif->pDull/*,XXX*/); // CZ_REQ_COUNT_BARGAIN_SALE_ITEM
// changed packet sizes
#endif

//2013-08-07Ragexe (Shakto)
#if PACKETVER >= 20130807
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0202,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0365,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x022D,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x023B,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0361,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0887,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,clif->pDull); // CZ_GANGSI_RANK
#endif

//2013-08-14aRagexe - Themon
#if PACKETVER >= 20130814
	packet(0x0874,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0947,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x093A,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x088A,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x088C,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0926,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x095F,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0202,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0873,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0887,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0962,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0937,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0923,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0868,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0941,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0889,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0895,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094E,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0936,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#ifdef PACKETVER_RE
	packet(0x0365,clif->pPartyRecruitRegisterReq,2,4);
#else // not PACKETVER_RE
	packet(0x0959,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
#endif // PACKETVER_RE
	packet(0x08A4,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0368,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0927,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0281,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0958,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0885,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0815,clif->pDull); // CZ_GANGSI_RANK
	packet(0x0896,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

// 2013-08-14aRagexe
#if PACKETVER >= 20130814
// new packets
	packet(0x09ce,clif->pGM_Monster_Item,2); // CZ_ITEM_CREATE_EX
	packet(0x09d0,clif->pDull/*,XXX*/); // CZ_NPROTECTGAMEGUARDCSAUTH
// changed packet sizes
#endif

// 2013-08-28bRagexe
#if PACKETVER >= 20130828
// new packets
// changed packet sizes
	packet(0x09ba,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_GUILD_STORAGE
	packet(0x09be,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_GUILD_STORAGE
#endif

// 2013-09-11aRagexe
#if PACKETVER >= 20130911
// new packets
	packet(0x09d4,clif->pNPCShopClosed); // CZ_NPC_TRADE_QUIT
	packet(0x09d6,clif->pNPCMarketPurchase); // CZ_NPC_MARKET_PURCHASE
	packet(0x09d8,clif->pNPCMarketClosed); // CZ_NPC_MARKET_CLOSE
	packet(0x09d9,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
// changed packet sizes
#endif

// 2013-10-02aRagexe
#if PACKETVER >= 20131002
// new packets
// changed packet sizes
	packet(0x09d9,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
#endif

// 2013-10-16aRagexe
#if PACKETVER >= 20131016
// new packets
// changed packet sizes
	packet(0x09d9,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
#endif

// 2013-10-23aRagexe
#if PACKETVER >= 20131023
// new packets
// changed packet sizes
	packet(0x09d9,clif->pDull/*,XXX*/); // CZ_REQ_GUILDSTORAGE_LOG
#endif

// 2013-11-06aRagexe
#if PACKETVER >= 20131106
// new packets
	packet(0x09e1,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_BODY_TO_GUILDSTORAGE
	packet(0x09e2,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_GUILDSTORAGE_TO_BODY
	packet(0x09e3,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_CART_TO_GUILDSTORAGE
	packet(0x09e4,clif->pDull/*,XXX*/); // CZ_MOVE_ITEM_FROM_GUILDSTORAGE_TO_CART
// changed packet sizes
#endif

// 2013-12-11dRagexe
#if PACKETVER >= 20131211
// new packets
	packet(0x09e8,clif->pDull/*,XXX*/); // CZ_OPEN_RODEXBOX
	packet(0x09e9,clif->pRodexCloseMailbox); // CZ_CLOSE_RODEXBOX
	packet(0x09ee,clif->pDull/*,XXX*/); // CZ_REQ_NEXT_RODEX
// changed packet sizes
#endif

// 2013-12-18bRagexe - Yommy
#if PACKETVER >= 20131218
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0947,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x022D,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x092F,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08AB,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0811,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x085C,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x087b,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-12-18bRagexe
#if PACKETVER >= 20131218
// new packets
	packet(0x09ea,clif->pDull/*,XXX*/); // CZ_REQ_READ_RODEX
	packet(0x09ef,clif->pRodexRefreshMaillist); // CZ_REQ_REFRESH_RODEX
	packet(0x09f5,clif->pRodexDeleteMail); // CZ_REQ_DELETE_RODEX
// changed packet sizes
	packet(0x09e8,clif->pDull/*,XXX*/); // CZ_OPEN_RODEXBOX
	packet(0x09ee,clif->pRodexNextMaillist); // CZ_REQ_NEXT_RODEX
#endif

// 2013-12-23cRagexe - Yommy
#if PACKETVER >= 20131223
	packet(0x0369,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x083C,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0437,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x035F,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0202,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x07E4,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0362,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x07EC,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0364,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0438,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x096A,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0368,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0838,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0835,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0819,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0811,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0360,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0817,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0815,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0365,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0281,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x022d,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0802,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x023B,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0361,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08A4,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0363,clif->pDull); // CZ_JOIN_BATTLE_FIELD
	packet(0x0436,clif->pDull); // CZ_GANGSI_RANK
#endif

// 2013-12-23bRagexe
#if PACKETVER >= 20131223
// new packets
// changed packet sizes
	packet(0x09ea,clif->pRodexReadMail); // CZ_REQ_READ_RODEX
#endif

// 2013-12-30aRagexe - Yommy
#if PACKETVER >= 20131230
	packet(0x0871,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x02C4,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x035F,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094A,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x092A,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0860,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0968,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0895,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x091E,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x096A,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0926,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0898,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x087B,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0369,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x093D,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x087F,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0969,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x094C,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0365,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x091F,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x022D,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089C,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x08A9,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0943,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0949,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x091D,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x087e,clif->pDull); // CZ_GANGSI_RANK
	packet(0x093e,clif->pDull); // CZ_JOIN_BATTLE_FIELD
#endif

// 2013-12-30aRagexe
#if PACKETVER >= 20131230
// new packets
	packet(0x09ec,clif->pRodexSendMail); // CZ_REQ_SEND_RODEX
// changed packet sizes
#endif

// 2014 Packet Data

// 2014-01-15cRagexeRE
#if PACKETVER >= 20140115
// new packets
	packet(0x09f1,clif->pDull/*,XXX*/); // CZ_REQ_ZENY_FROM_RODEX
	packet(0x09f3,clif->pDull/*,XXX*/); // CZ_REQ_ITEM_FROM_RODEX
// changed packet sizes
#endif

// 2014-01-22aRagexeRE
#if PACKETVER >= 20140122
// new packets
	packet(0x09fb,clif->pPetEvolution); // CZ_PET_EVOLUTION
// changed packet sizes
#endif

// 2014-01-29bRagexeRE
#if PACKETVER >= 20140129
// new packets
	packet(0x0a01,clif->pHotkeyRowShift,2); // CZ_SHORTCUTKEYBAR_ROTATE
#endif

// 2014-02-12aRagexeRE
#if PACKETVER >= 20140212
// new packets
// changed packet sizes
	packet(0x09e8,clif->pRodexOpenMailbox); // CZ_OPEN_RODEXBOX
#endif

// 2014-02-26aRagexeRE
#if PACKETVER >= 20140226
// new packets
	packet(0x0a03,clif->pDull/*,XXX*/); // CZ_REQ_CANCEL_WRITE_RODEX
	packet(0x0a04,clif->pDull/*,XXX*/); // CZ_REQ_ADD_ITEM_RODEX
	packet(0x0a06,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_RODEX_ITEM
// changed packet sizes
#endif

// 2014-03-05aRagexeRE
#if PACKETVER >= 20140305
// new packets
	packet(0x0a08,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_WRITE_RODEX
// changed packet sizes
	packet(0x09f3,clif->pDull/*,XXX*/); // CZ_REQ_ITEM_FROM_RODEX
#endif

// 2014-03-26cRagexeRE
#if PACKETVER >= 20140326
// changed packet sizes
	packet(0x09f1,clif->pRodexRequestZeny); // CZ_REQ_ZENY_FROM_RODEX
	packet(0x09f3,clif->pRodexRequestItems); // CZ_REQ_ITEM_FROM_RODEX
	packet(0x0a03,clif->pRodexCancelWriteMail); // CZ_REQ_CANCEL_WRITE_RODEX
	packet(0x0a08,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_WRITE_RODEX
#endif

// 2014-04-16aRagexeRE
#if PACKETVER >= 20140416
// new packets
	packet(0x0a04,clif->pRodexAddItem); // CZ_REQ_ADD_ITEM_RODEX
	packet(0x0a13,clif->pRodexCheckName); // CZ_CHECK_RECEIVE_CHARACTER_NAME
// changed packet sizes
	packet(0x0a06,clif->pRodexRemoveItem); // CZ_REQ_REMOVE_RODEX_ITEM
	packet(0x0a08,clif->pRodexOpenWriteMail); // CZ_REQ_OPEN_WRITE_RODEX
#endif

// 2014-04-23aRagexeRE
#if PACKETVER >= 20140423
// new packets
// changed packet sizes
	packet(0x0a13,clif->pRodexCheckName); // CZ_CHECK_RECEIVE_CHARACTER_NAME
#endif

// 2014-04-30aRagexeRE
#if PACKETVER >= 20140430
// new packets
	packet(0x0a16,clif->pDull/*,XXX*/); // CZ_DYNAMICNPC_CREATE_REQUEST
#endif

/* Roulette System [Yommy/Hercules] */
// 2014-06-05aRagexe
#if PACKETVER >= 20140605
// new packets
	packet(0x0a19,clif->pDull/*,XXX*/); // CZ_REQ_OPEN_ROULETTE
	packet(0x0A1B,clif->pRouletteInfo,0);     // HEADER_CZ_REQ_ROULETTE_INFO
	packet(0x0a1d,clif->pDull/*,XXX*/); // CZ_REQ_CLOSE_ROULETTE
#endif

/* Roulette System [Yommy/Hercules] */
// 2014-06-11bRagexe / RE. moved by 4144
#if PACKETVER >= 20140611
// new packets
	packet(0x0a1f,clif->pRouletteGenerate,0); // CZ_REQ_GENERATE_ROULETTE
	packet(0x0a21,clif->pDull/*,XXX*/); // CZ_RECV_ROULETTE_ITEM
	packet(0x0a25,clif->pAchievementGetReward,2); // CZ_REQ_ACH_REWARD
// changed packet sizes
	packet(0x0a19,clif->pRouletteOpen,0); // CZ_REQ_OPEN_ROULETTE
	packet(0x0a1d,clif->pRouletteClose,0); // CZ_REQ_CLOSE_ROULETTE
#endif

// 2014-06-18cRagexeRE
#if PACKETVER >= 20140618
// changed packet sizes
	packet(0x0a21,clif->pRouletteRecvItem,2); // CZ_RECV_ROULETTE_ITEM
#endif

// 2014-07-02aRagexeRE
#if PACKETVER >= 20140702
// new packets
	packet(0x0a2a,clif->pDull/*,XXX*/); // CZ_ACK_AU_BOT
#endif

// 2014-09-03aRagexeRE
#if PACKETVER >= 20140903
// new packets
	packet(0x0a2e,clif->pChangeTitle); // CZ_REQ_CHANGE_TITLE
// changed packet sizes
#endif

// 2014-11-19bRagexeRE
#if PACKETVER >= 20141119
// new packets
	packet(0x0A35,clif->pOneClick_ItemIdentify,2);
// changed packet sizes
#endif

// 2015-11-04aRagexeRE
#if PACKETVER >= 20151104
// new packets
	packet(0x0a46,clif->pReqStyleChange);
#endif

// 2016-03-23aRagexeRE
#if PACKETVER >= 20160323
// new packets
	packet(0x0a68,clif->pOpenUIRequest);
// changed packet sizes
#endif

// 2016-03-30aRagexeRE
#if PACKETVER >= 20160330
// new packets
	packet(0x0a6e,clif->pRodexSendMail); // CZ_RODEX_SEND_MAIL
// changed packet sizes
#endif

// all 2016-05-25
#if PACKETVER >= 20160525
	packet(0x0a77,clif->pCameraInfo); // CZ_CAMERA_INFO
#endif

// all 20160622+
#if PACKETVER >= 20160622
	packet(0x0a88,clif->pResetCooldown);
#endif

// 2017-02-28aRagexeRE
#if PACKETVER >= 20170228
// new packets
	packet(0x0ac0,clif->pRodexOpenMailbox); // CZ_OPEN_RODEXBOX
	packet(0x0ac1,clif->pRodexRefreshMaillist); // CZ_REQ_REFRESH_RODEX
// changed packet sizes
#endif

// 2017-08-30aRagexeRE
#if PACKETVER >= 20170830
// new packets
// changed packet sizes
	packet(0x0a49,clif->pPrivateAirshipRequest);
#endif

#ifdef PACKETVER_ZERO
#if PACKETVER >= 20171214
// new packets
	packet(0x0ae8,clif->pChangeDress);
// changed packet sizes
#endif
#endif  // PACKETVER_ZERO

// 2017-12-20aRagexe
#if PACKETVER >= 20171220
// new packets
	packet(0x0ae8,clif->pChangeDress);
// changed packet sizes
#endif

#if PACKETVER >= 20180117
// new packets
	packet(0x0aef,clif->pAttendanceRewardRequest);
// changed packet sizes
#endif

#ifdef PACKETVER_ZERO
// 2018-01-31dRagexe_zero
#if PACKETVER >= 20180131
// new packets
	packet(0x0af4,clif->pUseSkillToPos,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND
// changed packet sizes
#endif
#endif  // PACKETVER_ZERO

#ifndef PACKETVER_ZERO
// 2018-02-07bRagexeRE, 2018-02-07bRagexe
#if PACKETVER >= 20180207
// new packets
	packet(0x0af4,clif->pUseSkillToPos,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND
// changed packet sizes
#endif
#endif  // PACKETVER_ZERO

#ifndef PACKETVER_ZERO
// 2018-05-16cRagexe, 2018-05-16cRagexeRE
#if PACKETVER >= 20180516
// new packets
	packet(0x0afc,clif->pReqStyleChange2);
// changed packet sizes
#endif
#endif  // PACKETVER_ZERO

#ifdef PACKETVER_ZERO
// 2018-05-23aRagexe_zero
#if PACKETVER >= 20180523
// new packets
	packet(0x0afc,clif->pReqStyleChange2);
// changed packet sizes
#endif
#endif  // PACKETVER_ZERO

// 2018-07-04aRagexeRE
#if PACKETVER_RE_NUM >= 20180704
// new packets
// changed packet sizes
	packet(0x018e,clif->pProduceMix); // CZ_REQMAKINGITEM
	packet(0x01ae,clif->pSelectArrow,2); // CZ_REQ_MAKINGARROW
	packet(0x01fd,clif->pRepairItem); // CZ_REQ_ITEMREPAIR
	packet(0x025b,clif->pCooking); // CZ_REQ_MAKINGITEM
	packet(0x0445,clif->pDull/*,XXX*/); // CZ_SIMPLE_BUY_CASH_POINT_ITEM
	packet(0x09ae,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09c3,clif->pDull/*,XXX*/); // CZ_REQ_COUNT_BARGAIN_SALE_ITEM
	packet(0x0a49,clif->pPrivateAirshipRequest); // CZ_PRIVATE_AIRSHIP_REQUEST
#endif

// 2018-11-14aRagexe_zero
#if PACKETVER_ZERO_NUM >= 20181114
// new packets
// changed packet sizes
	packet(0x018e,clif->pProduceMix); // CZ_REQMAKINGITEM
	packet(0x01ae,clif->pSelectArrow,2); // CZ_REQ_MAKINGARROW
	packet(0x01fd,clif->pRepairItem); // CZ_REQ_ITEMREPAIR
	packet(0x025b,clif->pCooking); // CZ_REQ_MAKINGITEM
	packet(0x0445,clif->pDull/*,XXX*/); // CZ_SIMPLE_BUY_CASH_POINT_ITEM
	packet(0x09ae,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09c3,clif->pDull/*,XXX*/); // CZ_REQ_COUNT_BARGAIN_SALE_ITEM
	packet(0x0a49,clif->pPrivateAirshipRequest); // CZ_PRIVATE_AIRSHIP_REQUEST
#endif

// 2018-11-21bRagexe
#if PACKETVER_ZERO_NUM >= 20181121
// new packets
// changed packet sizes
	packet(0x018e,clif->pProduceMix); // CZ_REQMAKINGITEM
	packet(0x01ae,clif->pSelectArrow,2); // CZ_REQ_MAKINGARROW
	packet(0x01fd,clif->pRepairItem); // CZ_REQ_ITEMREPAIR
	packet(0x025b,clif->pCooking); // CZ_REQ_MAKINGITEM
	packet(0x0445,clif->pDull/*,XXX*/); // CZ_SIMPLE_BUY_CASH_POINT_ITEM
	packet(0x09ae,clif->pDull/*,XXX*/); // CZ_REQ_APPLY_BARGAIN_SALE_ITEM
	packet(0x09b0,clif->pDull/*,XXX*/); // CZ_REQ_REMOVE_BARGAIN_SALE_ITEM
	packet(0x09c3,clif->pDull/*,XXX*/); // CZ_REQ_COUNT_BARGAIN_SALE_ITEM
	packet(0x0a49,clif->pPrivateAirshipRequest); // CZ_PRIVATE_AIRSHIP_REQUEST
#endif

#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	packet(0x0b10,clif->pStartUseSkillToId);
	packet(0x0b11,clif->pStopUseSkillToId);
#endif

#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
	packet(0x0b14,clif->pInventoryExpansion);
	packet(0x0b16,clif->pInventoryExpansionConfirmed);
	packet(0x0b19,clif->pInventoryExpansionRejected);
#endif

#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	packet(0x0b0f,clif->pNPCBarterPurchase);
	packet(0x0b12,clif->pNPCBarterClosed);
#endif

#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320
	packet(0x0b1c,clif->pPing);
#endif

#endif /* MAP_PACKETS_H */
