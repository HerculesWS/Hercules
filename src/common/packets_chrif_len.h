/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
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
#ifndef COMMON_PACKETS_CHRIF_LEN_H
#define COMMON_PACKETS_CHRIF_LEN_H

packetLen(0x2af8, 60)  /* M->H, chrif_connect -> 'connect to charserver / auth @ charserver' */
packetLen(0x2af9, 3)   /* H->M, chrif_connectack -> 'answer of the 2af8 login(ok / fail)' */
packetLen(0x2afa, -1)  /* M->H, chrif_sendmap -> 'sending our maps' */
packetLen(0x2afb, 27)  /* H->M, chrif_sendmapack -> 'Maps received successfully / or not ..' */
packetLen(0x2afc, 10)  /* M->H, chrif_scdata_request -> request sc_data for pc->authok'ed char. <- new command reuses previous one. */
packetLen(0x2afd, -1)  /* H->M, chrif_authok -> 'client authentication ok' */
packetLen(0x2afe, 6)   /* M->H, send_usercount_tochar -> 'sends player count of this map server to charserver' */
packetLen(0x2aff, -1)  /* M->H, chrif_send_users_tochar -> 'sends all actual connected character ids to charserver' */
packetLen(0x2b00, 6)   /* H->M, map_setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure) */
packetLen(0x2b01, -1)  /* M->H, chrif_save -> 'charsave of char XY account XY (complete struct)' */
packetLen(0x2b02, 18)  /* M->H, chrif_charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure) */
packetLen(0x2b03, 7)   /* H->M, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure) */
packetLen(0x2b04, 0)   /* FREE */
packetLen(0x2b05, 0)   /* FREE */
packetLen(0x2b06, 0)   /* FREE */
packetLen(0x2b07, 10)  /* M->H, chrif_removefriend -> 'Tell charserver to remove friend_id from char_id friend list' */
packetLen(0x2b08, 6)   /* M->H, chrif_searchcharid -> '...' */
packetLen(0x2b09, 30)  /* H->M, map_addchariddb -> 'Adds a name to the nick db' */
packetLen(0x2b0a, -1)  /* H->M/M->H, socket_datasync() */
packetLen(0x2b0b, 0)   /* M->H, update charserv skillid2idx */
packetLen(0x2b0c, 86)  /* M->H, chrif_changeemail -> 'change mail address ...' */
packetLen(0x2b0d, 7)   /* H->M, chrif_changedsex -> 'Change sex of acc XY' (or char) */
packetLen(0x2b0e, 44)  /* M->H, chrif_char_ask_name -> 'Do some operations (change sex, ban / unban etc)' */
packetLen(0x2b0f, 34)  /* H->M, chrif_char_ask_name_answer -> 'answer of the 2b0e' */
packetLen(0x2b10, 11)  /* M->H, chrif_updatefamelist -> 'Update the fame ranking lists and send them' */
packetLen(0x2b11, 10)  /* M->H, chrif_divorce -> 'tell the charserver to do divorce' */
packetLen(0x2b12, 10)  /* H->M, chrif_divorceack -> 'divorce chars */
packetLen(0x2b13, 0)   /* FREE */
packetLen(0x2b14, 11)  /* H->M, chrif_accountban -> 'not sure: kick the player with message XY' */
packetLen(0x2b15, 0)   /* FREE */
packetLen(0x2b16, 266) /* M->H, chrif_ragsrvinfo -> 'sends base / job / drop rates ....' */
packetLen(0x2b17, 10)  /* M->H, chrif_char_offline -> 'tell the charserver that the char is now offline' */
packetLen(0x2b18, 2)   /* M->H, chrif_char_reset_offline -> 'set all players OFF!' */
packetLen(0x2b19, 10)  /* M->H, chrif_char_online -> 'tell the charserver that the char .. is online' */
packetLen(0x2b1a, 2)   /* M->H, chrif_buildfamelist -> 'Build the fame ranking lists and send them' */
packetLen(0x2b1b, -1)  /* H->M, chrif_recvfamelist -> 'Receive fame ranking lists' */
packetLen(0x2b1c, -1)  /* M->H, chrif_save_scdata -> 'Send sc_data of player for saving.' */
packetLen(0x2b1d, -1)  /* H->M, chrif_load_scdata -> 'received sc_data of player for loading.' */
packetLen(0x2b1e, 2)   /* H->M, chrif_update_ip -> 'Request forwarded from char-server for interserver IP sync.' [Lance] */
packetLen(0x2b1f, 7)   /* H->M, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius] */
packetLen(0x2b20, 0)   /* FREE */
packetLen(0x2b21, 10)  /* H->M, chrif_save_ack. Returned after a character has been "final saved" on the char-server. [Skotlex] */
packetLen(0x2b22, 8)   /* H->M, chrif_updatefamelist_ack. Updated one position in the fame list. */
packetLen(0x2b23, 2)   /* M->H, chrif_keepalive. charserver ping. */
packetLen(0x2b24, 2)   /* H->M, chrif_keepalive_ack. charserver ping reply. */
packetLen(0x2b25, 14)  /* H->M, chrif_deadopt -> 'Removes baby from Father ID and Mother ID' */
packetLen(0x2b26, 19)  /* M->H, chrif_authreq -> 'client authentication request' */
packetLen(0x2b27, 19)  /* H->M, chrif_authfail -> 'client authentication failed' */
packetLen(0x2b28, 0)   /* FREE */
packetLen(0x2b29, 0)   /* FREE */
packetLen(0x2b2a, 0)   /* FREE */
packetLen(0x2b2b, 0)   /* FREE */
packetLen(0x2b2c, 0)   /* FREE */
packetLen(0x2b2d, 0)   /* FREE */
packetLen(0x2b2e, 0)   /* FREE */
packetLen(0x2b2f, 0)   /* FREE */
packetLen(0x2b30, 0)   /* FREE */
packetLen(0x2b31, 0)   /* FREE */
packetLen(0x2b32, 0)   /* FREE */
packetLen(0x2b33, 0)   /* FREE */
packetLen(0x2b34, 0)   /* FREE */

#endif /* COMMON_PACKETS_CHRIF_LEN_H */
