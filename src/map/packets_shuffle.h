/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2017  Hercules Dev Team
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

#ifndef MAP_PACKETS_SHUFFLE_H
#define MAP_PACKETS_SHUFFLE_H

#ifndef packet
	#define packet(a,b,...)
#endif

/*
 * packet syntax
 * - packet(packet_id,length,function,offset ( specifies the offset of a packet field in bytes from the begin of the packet ),...)
 * - Example: packet(0x0072,19,clif->pWantToConnection,2,6,10,14,18);
 */

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

// 2017-09-27bRagexeRE
#if PACKETVER == 20170927
// shuffle packets
	packet(0x02c4,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x035f,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0361,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0362,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0366,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x085c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0873,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0875,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x087d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x087e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x088b,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0899,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x089a,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x089b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08a3,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x08a5,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08a6,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08ad,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x091e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0922,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0923,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0927,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x093b,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0942,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0945,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x094d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0959,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x095a,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
#endif

// 2017-10-02cRagexeRE
#if PACKETVER == 20171002
// shuffle packets
	packet(0x022d,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0885,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0897,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0899,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x089d,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0928,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x092d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0934,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093b,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093d,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x093e,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0943,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x095f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-10-11bRagexeRE
#if PACKETVER == 20171011
// shuffle packets
	packet(0x023b,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
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
	packet(0x087b,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0882,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0950,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0954,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-10-18aRagexeRE
#if PACKETVER == 20171018
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0363,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0364,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086a,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x087a,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x087e,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0889,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089a,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x08a6,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0938,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0944,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x094a,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x094f,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-10-25eRagexeRE
#if PACKETVER == 20171025
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
	packet(0x08a2,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-01bRagexeRE
#if PACKETVER == 20171101
// shuffle packets
	packet(0x022d,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0368,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0369,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0438,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0835,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x085b,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0860,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0872,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0876,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0886,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088e,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0890,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0895,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0899,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x089b,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x089c,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a0,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x08ab,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x08ad,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x091b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0939,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x094a,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x094d,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0952,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0957,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x095a,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0962,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0966,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
#endif

// 2017-11-08bRagexeRE
#if PACKETVER == 20171108
// shuffle packets
	packet(0x0202,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0361,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x07e4,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0815,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0819,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0838,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x085d,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0863,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0878,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x087e,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0884,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0896,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0897,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x08a2,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x08a9,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08ad,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x091d,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x091f,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0940,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0941,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0945,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0947,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0949,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x094e,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0958,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x095a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0963,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0965,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0967,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
#endif

// 2017-11-15aRagexeRE
#if PACKETVER == 20171115
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0365,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0436,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0802,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0811,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x086d,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x086f,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x087e,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0883,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x088b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0890,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0898,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x08a4,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0926,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0958,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x095a,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-22bRagexeRE
#if PACKETVER == 20171122
// shuffle packets
	packet(0x0281,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x02c4,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x035f,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0838,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x083c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x085b,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x0862,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0867,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0877,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0885,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0890,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0891,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0893,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0897,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0898,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x089a,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x089e,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x08a6,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x08a9,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x091e,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0920,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0923,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0934,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x093b,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0945,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0946,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0947,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0962,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0968,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
#endif

// 2017-11-29aRagexeRE
#if PACKETVER == 20171129
// shuffle packets
	packet(0x02c4,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x035f,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0361,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0363,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0365,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
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
	packet(0x0838,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0862,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x086d,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0876,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0878,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088a,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x089c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x08a5,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0940,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x094b,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0953,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0966,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// kro zero clients
#ifdef PACKETVER_ZERO

// from 2017-10-19aRagexe_zero to 2017-11-13bRagexe_zero
#if PACKETVER == 20171019 || \
    PACKETVER == 20171023 || \
    PACKETVER == 20171024 || \
    PACKETVER == 20171025 || \
    PACKETVER == 20171027 || \
    PACKETVER == 20171030 || \
    PACKETVER == 20171031 || \
    PACKETVER == 20171109 || \
    PACKETVER == 20171113
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

// 2017-11-15bRagexe_zero to 2017-11-17aRagexe_zero
#if PACKETVER == 20171115 || \
    PACKETVER == 20171116 || \
    PACKETVER == 20171117
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
	packet(0x0860,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0881,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x091c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0922,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0959,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x0966,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-21aRagexe_zero
#if PACKETVER == 20171121 || \
    PACKETVER == 20171122
// shuffle packets
	packet(0x0202,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x022d,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0362,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0363,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0368,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0369,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0437,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0438,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0811,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x0815,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0817,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0819,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x0835,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0838,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x083c,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
	packet(0x0866,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0889,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0892,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x089e,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08ad,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0918,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x091f,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0928,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0943,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x0950,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-23dRagexe_zero
#if PACKETVER == 20171123
// shuffle packets
	packet(0x035f,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x0360,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0366,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x0367,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
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
	packet(0x085f,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0860,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0876,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0882,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x088c,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0896,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x089e,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x08a8,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x092b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0930,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0935,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x0947,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x0960,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-27cRagexe_zero to 2017-11-28aRagexe_zero
#if PACKETVER == 20171127 || \
    PACKETVER == 20171128
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
	packet(0x0893,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x096a,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
#endif

// 2017-11-30bRagexe_zero
#if PACKETVER == 20171130
// shuffle packets
	packet(0x0361,18,clif->pPartyBookingRegisterReq,2,4);  // CZ_PARTY_BOOKING_REQ_REGISTER
	packet(0x0864,6,clif->pGetCharNameRequest,2);  // CZ_REQNAME
	packet(0x086f,12,clif->pSearchStoreInfoListItemClick,2,6,10);  // CZ_SSILIST_ITEM_CLICK
	packet(0x0871,2,clif->pSearchStoreInfoNextPage,0);  // CZ_SEARCH_STORE_INFO_NEXT_PAGE
	packet(0x0872,6,clif->pReqClickBuyingStore,2);  // CZ_REQ_CLICK_TO_BUYING_STORE
	packet(0x0875,6,clif->pTakeItem,2);  // CZ_ITEM_PICKUP
	packet(0x0878,-1,clif->pReqTradeBuyingStore,2,4,8,12);  // CZ_REQ_TRADE_BUYING_STORE
	packet(0x0881,10,clif->pUseSkillToPos,2,4,6,8);  // CZ_USE_SKILL_TOGROUND
	packet(0x0884,6,clif->pDropItem,2,4);  // CZ_ITEM_THROW
	packet(0x0886,2,clif->pReqCloseBuyingStore,0);  // CZ_REQ_CLOSE_BUYING_STORE
	packet(0x0887,36,clif->pStoragePassword,0);  // CZ_ACK_STORE_PASSWORD
	packet(0x088b,8,clif->pMoveToKafra,2,4);  // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
	packet(0x0894,5,clif->pWalkToXY,2);  // CZ_REQUEST_MOVE
	packet(0x0899,8,clif->pDull/*,XXX*/);  // CZ_JOIN_BATTLE_FIELD
	packet(0x08a0,5,clif->pChangeDir,2,4);  // CZ_CHANGE_DIRECTION
	packet(0x08a7,-1,clif->pItemListWindowSelected,2,4,8);  // CZ_ITEMLISTWIN_RES
	packet(0x0925,-1,clif->pReqOpenBuyingStore,2,4,8,9,89);  // CZ_REQ_OPEN_BUYING_STORE
	packet(0x0928,4,clif->pDull/*,XXX*/);  // CZ_GANGSI_RANK
	packet(0x0930,19,clif->pWantToConnection,2,6,10,14,18);  // CZ_ENTER
	packet(0x0931,26,clif->pFriendsListAdd,2);  // CZ_ADD_FRIENDS
	packet(0x0935,8,clif->pMoveFromKafra,2,4);  // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
	packet(0x093a,7,clif->pActionRequest,2,6);  // CZ_REQUEST_ACT
	packet(0x0947,-1,clif->pSearchStoreInfo,2,4,5,9,13,14,15);  // CZ_SEARCH_STORE_INFO
	packet(0x094c,5,clif->pHomMenu,2,4);  // CZ_COMMAND_MER
	packet(0x094f,90,clif->pUseSkillToPosMoreInfo,2,4,6,8,10);  // CZ_USE_SKILL_TOGROUND_WITHTALKBOX
	packet(0x095b,6,clif->pTickSend,2);  // CZ_REQUEST_TIME
	packet(0x095f,26,clif->pPartyInvite2,2);  // CZ_PARTY_JOIN_REQ
	packet(0x0960,6,clif->pSolveCharName,2);  // CZ_REQNAME_BYGID
	packet(0x0965,10,clif->pUseSkillToId,2,4,6);  // CZ_USE_SKILL
#endif

#endif  // PACKETVER_ZERO

#endif /* MAP_PACKETS_SHUFFLE_H */
