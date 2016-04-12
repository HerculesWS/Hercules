/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016  Hercules Dev Team
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
#define HERCULES_CORE

#include "lclif.h"

#include "login/ipban.h"
#include "login/login.h"
#include "login/loginlog.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/md5calc.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/utils.h"

struct lclif_interface lclif_s;
struct lclif_interface *lclif;

/* Definitions and macros */
/// Maximum amount of packets processed at once from the same client
#define MAX_PROCESSED_PACKETS (3)

// Packet DB
#define MIN_PACKET_DB 0x0064
#define MAX_PACKET_DB 0x08ff

/* Enums */

/// Packet IDs
enum login_packet_id {
	// CA (Client to Login)
	PACKET_ID_CA_LOGIN                = 0x0064,
	PACKET_ID_CA_LOGIN2               = 0x01dd,
	PACKET_ID_CA_LOGIN3               = 0x01fa,
	PACKET_ID_CA_CONNECT_INFO_CHANGED = 0x0200,
	PACKET_ID_CA_EXE_HASHCHECK        = 0x0204,
	PACKET_ID_CA_LOGIN_PCBANG         = 0x0277,
	PACKET_ID_CA_LOGIN4               = 0x027c,
	PACKET_ID_CA_LOGIN_HAN            = 0x02b0,
	PACKET_ID_CA_SSO_LOGIN_REQ        = 0x0825,
	PACKET_ID_CA_REQ_HASH             = 0x01db,
	PACKET_ID_CA_CHARSERVERCONNECT    = 0x2710, // Custom Hercules Packet
	//PACKET_ID_CA_SSO_LOGIN_REQa       = 0x825a, /* unused */

	// AC (Login to Client)
	PACKET_ID_AC_ACCEPT_LOGIN         = 0x0069,
	PACKET_ID_AC_REFUSE_LOGIN         = 0x006a,
	PACKET_ID_SC_NOTIFY_BAN           = 0x0081,
	PACKET_ID_AC_ACK_HASH             = 0x01dc,
	PACKET_ID_AC_REFUSE_LOGIN_R2      = 0x083e,
};

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * Packet structure for CA_LOGIN.
 */
struct packet_CA_LOGIN {
	int16 packet_id;   ///< Packet ID (#PACKET_ID_CA_LOGIN)
	uint32 version;    ///< Client Version
	char id[24];       ///< Username
	char password[24]; ///< Password
	uint8 clienttype;  ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN2.
 */
struct packet_CA_LOGIN2 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN2)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN3.
 */
struct packet_CA_LOGIN3 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN3)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	uint8 clientinfo;       ///< Index of the connection in the clientinfo file (+10 if the command-line contains "pc")
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN4.
 */
struct packet_CA_LOGIN4 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN4)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	char mac_address[13];   ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_PCBANG.
 */
struct packet_CA_LOGIN_PCBANG {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_LOGIN_PCBANG)
	uint32 version;	      ///< Client Version
	char id[24];          ///< Username
	char password[24];    ///< Password
	uint8 clienttype;     ///< Client Type
	char ip[16];          ///< IP Address
	char mac_address[13]; ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_HAN.
 */
struct packet_CA_LOGIN_HAN {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN_HAN)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	char password[24];      ///< Password
	uint8 clienttype;       ///< Client Type
	char ip[16];            ///< IP Address
	char mac_address[13];   ///< MAC Address
	uint8 is_han_game_user; ///< 'isGravityID'
} __attribute__((packed));

/**
 * Packet structure for CA_SSO_LOGIN_REQ.
 *
 * Variable-length packet.
 */
struct packet_CA_SSO_LOGIN_REQ {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_SSO_LOGIN_REQ)
	int16 packet_len;     ///< Length (variable length)
	uint32 version;       ///< Client Version
	uint8 clienttype;     ///< Client Type
	char id[24];          ///< Username
	char password[27];    ///< Password
	int8 mac_address[17]; ///< MAC Address
	char ip[15];          ///< IP Address
	char t1[];            ///< SSO Login Token (variable length)
} __attribute__((packed));

#if 0 // Unused
struct packet_CA_SSO_LOGIN_REQa {
	int16 packet_id;
	int16 packet_len;
	uint32 version;
	uint8 clienttype;
	char id[24];
	int8 mac_address[17];
	char ip[15];
	char t1[];
} __attribute__((packed));
#endif // unused

/**
 * Packet structure for CA_CONNECT_INFO_CHANGED.
 *
 * New alive packet. Used to verify if client is always alive.
 */
struct packet_CA_CONNECT_INFO_CHANGED {
	int16 packet_id; ///< Packet ID (#PACKET_ID_CA_CONNECT_INFO_CHANGED)
	char id[24];    ///< account.userid
} __attribute__((packed));

/**
 * Packet structure for CA_EXE_HASHCHECK.
 *
 * (kRO 2004-05-31aSakexe langtype 0 and 6)
 */
struct packet_CA_EXE_HASHCHECK {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_EXE_HASHCHECK)
	uint8 hash_value[16]; ///< Client MD5 hash
} __attribute__((packed));

/**
 * Packet structure for CA_REQ_HASH.
 */
struct packet_CA_REQ_HASH {
	int16 packet_id; ///< Packet ID (#PACKET_ID_CA_REQ_HASH)
} __attribute__((packed));

struct packet_CA_CHARSERVERCONNECT {
	int16 packet_id;
	char userid[24];
	char password[24];
	int32 unknown;
	int32 ip;
	int16 port;
	char name[20];
	int16 unknown2;
	int16 type;
	int16 new;
} __attribute__((packed));

struct packet_SC_NOTIFY_BAN {
	int16 packet_id;
	uint8 error_code;
} __attribute__((packed));

struct packet_AC_REFUSE_LOGIN {
	int16 packet_id;
	uint8 error_code;
	char block_date[20];
} __attribute__((packed));

struct packet_AC_REFUSE_LOGIN_R2 {
	int16 packet_id;
	uint32 error_code;
	char block_date[20];
} __attribute__((packed));

struct packet_AC_ACCEPT_LOGIN {
	int16 packet_id;
	int16 packet_len;
	int32 auth_code;
	uint32 aid;
	uint32 user_level;
	uint32 last_login_ip;
	char last_login_time[26];
	uint8 sex;
	struct {
		uint32 ip;
		int16 port;
		char name[20];
		uint16 usercount;
		uint16 state;
		uint16 property;
	} server_list[];
} __attribute__((packed));

struct packet_AC_ACK_HASH {
	int16 packet_id;
	int16 packet_len;
	uint8 secret[];
} __attribute__((packed));

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

struct login_packet_db packet_db[MAX_PACKET_DB + 1];

void lclif_connection_error(int fd, uint8 error)
{
	struct packet_SC_NOTIFY_BAN *packet = NULL;
	WFIFOHEAD(fd, sizeof(*packet));
	packet = WP2PTR(fd);
	packet->packet_id = PACKET_ID_SC_NOTIFY_BAN;
	packet->error_code = error;
	WFIFOSET(fd, sizeof(*packet));
}

enum parsefunc_rcode lclif_parse_CA_CONNECT_INFO_CHANGED(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_CONNECT_INFO_CHANGED(int fd, struct login_session_data *sd)
{
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_EXE_HASHCHECK(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_EXE_HASHCHECK(int fd, struct login_session_data *sd)
{
	const struct packet_CA_EXE_HASHCHECK *packet = RP2PTR(fd);
	sd->has_client_hash = 1;
	memcpy(sd->client_hash, packet->hash_value, 16);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	safestrncpy(sd->passwd, packet->password, PASSWD_LEN);

	if (login->config->use_md5_passwds)
		MD5_String(sd->passwd, sd->passwd);
	sd->passwdenc = PWENC_NONE;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN2(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN2(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN2 *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	bin2hex(sd->passwd, packet->password_md5, 16);
	sd->passwdenc = PASSWORDENC;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN3(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN3(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN3 *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	/* unused */
	/* sd->clientinfo = packet->clientinfo; */
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	bin2hex(sd->passwd, packet->password_md5, 16);
	sd->passwdenc = PASSWORDENC;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN4(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN4(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN4 *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	/* unused */
	/* safestrncpy(sd->mac_address, packet->mac_address, sizeof(sd->mac_address)); */
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	bin2hex(sd->passwd, packet->password_md5, 16);
	sd->passwdenc = PASSWORDENC;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN_PCBANG(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN_PCBANG(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN_PCBANG *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	/* unused */
	/* safestrncpy(sd->ip, packet->ip, sizeof(sd->ip)); */
	/* safestrncpy(sd->mac_address, packet->mac_address, sizeof(sd->mac_address)); */
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	safestrncpy(sd->passwd, packet->password, PASSWD_LEN);

	if (login->config->use_md5_passwds)
		MD5_String(sd->passwd, sd->passwd);
	sd->passwdenc = PWENC_NONE;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_LOGIN_HAN(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_LOGIN_HAN(int fd, struct login_session_data *sd)
{
	const struct packet_CA_LOGIN_HAN *packet = RP2PTR(fd);

	sd->version = packet->version;
	sd->clienttype = packet->clienttype;
	/* unused */
	/* safestrncpy(sd->ip, packet->ip, sizeof(sd->ip)); */
	/* safestrncpy(sd->mac_address, packet->mac_address, sizeof(sd->mac_address)); */
	/* sd->ishan = packet->is_han_game_user; */
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	safestrncpy(sd->passwd, packet->password, PASSWD_LEN);

	if (login->config->use_md5_passwds)
		MD5_String(sd->passwd, sd->passwd);
	sd->passwdenc = PWENC_NONE;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_SSO_LOGIN_REQ(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_SSO_LOGIN_REQ(int fd, struct login_session_data *sd)
{
	const struct packet_CA_SSO_LOGIN_REQ *packet = RP2PTR(fd);
	int tokenlen = (int)RFIFOREST(fd) - (int)sizeof(*packet);

	if (tokenlen > PASSWD_LEN || tokenlen < 1) {
		ShowError("packet_CA_SSO_LOGIN_REQ: Token length is not between allowed password length, kicking player ('%s')", packet->id);
		sockt->eof(fd);
		return PACKET_VALID;
	}

	sd->clienttype = packet->clienttype;
	sd->version = packet->version;
	safestrncpy(sd->userid, packet->id, NAME_LENGTH);
	safestrncpy(sd->passwd, packet->t1, min(tokenlen + 1, PASSWD_LEN)); // Variable-length field, don't copy more than necessary

	if (login->config->use_md5_passwds)
		MD5_String(sd->passwd, sd->passwd);
	sd->passwdenc = PWENC_NONE;

	login->client_login(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_REQ_HASH(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_REQ_HASH(int fd, struct login_session_data *sd)
{
	memset(sd->md5key, '\0', sizeof(sd->md5key));
	sd->md5keylen = (uint16)(12 + rnd() % 4);
	MD5_Salt(sd->md5keylen, sd->md5key);

	lclif->coding_key(fd, sd);
	return PACKET_VALID;
}

enum parsefunc_rcode lclif_parse_CA_CHARSERVERCONNECT(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
enum parsefunc_rcode lclif_parse_CA_CHARSERVERCONNECT(int fd, struct login_session_data *sd)
{
	char ip[16];
	uint32 ipl = sockt->session[fd]->client_addr;
	sockt->ip2str(ipl, ip);

	login->parse_request_connection(fd, sd, ip, ipl);

	return PACKET_STOPPARSE;
}

bool lclif_send_server_list(struct login_session_data *sd)
{
	int server_num = 0, i, n, length;
	uint32 ip;
	struct packet_AC_ACCEPT_LOGIN *packet = NULL;

	for (i = 0; i < ARRAYLENGTH(server); ++i) {
		if (sockt->session_is_active(server[i].fd))
			server_num++;
	}
	if (server_num == 0)
		return false;

	length = sizeof(*packet) + sizeof(packet->server_list[0]) * server_num;
	ip = sockt->session[sd->fd]->client_addr;

	// Allocate the packet
	WFIFOHEAD(sd->fd, length);
	packet = WP2PTR(sd->fd);

	packet->packet_id = PACKET_ID_AC_ACCEPT_LOGIN;
	packet->packet_len = length;
	packet->auth_code = sd->login_id1;
	packet->aid = sd->account_id;
	packet->user_level = sd->login_id2;
	packet->last_login_ip = 0; // Not used anymore
	memset(packet->last_login_time, '\0', sizeof(packet->last_login_time)); // Not used anymore
	packet->sex = sex_str2num(sd->sex);
	for (i = 0, n = 0;  i < ARRAYLENGTH(server); ++i) {
		uint32 subnet_char_ip;

		if (!sockt->session_is_valid(server[i].fd))
			continue;

		subnet_char_ip = login->lan_subnet_check(ip);
		packet->server_list[n].ip = htonl((subnet_char_ip) ? subnet_char_ip : server[i].ip);
		packet->server_list[n].port = sockt->ntows(htons(server[i].port)); // [!] LE byte order here [!]
		safestrncpy(packet->server_list[n].name, server[i].name, 20);
		packet->server_list[n].usercount = server[i].users;

		if (server[i].type == CST_PAYING && sd->expiration_time > time(NULL))
			packet->server_list[n].property = CST_NORMAL;
		else
			packet->server_list[n].property = server[i].type;

		packet->server_list[n].state = server[i].new_;
		++n;
	}
	WFIFOSET(sd->fd, length);

	return true;
}

void lclif_send_auth_failed(int fd, time_t ban, uint32 error)
{
#if PACKETVER >= 20120000 /* not sure when this started */
	struct packet_AC_REFUSE_LOGIN_R2 *packet = NULL;
	int packet_id = PACKET_ID_AC_REFUSE_LOGIN_R2;
#else
	struct packet_AC_REFUSE_LOGIN *packet = NULL;
	int packet_id = PACKET_ID_AC_REFUSE_LOGIN;
#endif
	WFIFOHEAD(fd, sizeof(*packet));
	packet = WP2PTR(fd);
	packet->packet_id = packet_id;
	packet->error_code = error;
	if (error == 6)
		timestamp2string(packet->block_date, sizeof(packet->block_date), ban, login->config->date_format);
	else
		memset(packet->block_date, '\0', sizeof(packet->block_date));
	WFIFOSET(fd, sizeof(*packet));
}

void lclif_send_login_error(int fd, uint8 error)
{
	struct packet_AC_REFUSE_LOGIN *packet = NULL;
	WFIFOHEAD(fd, sizeof(*packet));
	packet = WP2PTR(fd);
	packet->packet_id = PACKET_ID_AC_REFUSE_LOGIN;
	packet->error_code = error;
	memset(packet->block_date, '\0', sizeof(packet->block_date));
	WFIFOSET(fd, sizeof(*packet));
}

void lclif_send_coding_key(int fd, struct login_session_data *sd) __attribute__((nonnull (2)));
void lclif_send_coding_key(int fd, struct login_session_data *sd)
{
	struct packet_AC_ACK_HASH *packet = NULL;
	int16 size = sizeof(*packet) + sd->md5keylen;

	WFIFOHEAD(fd, size);
	packet = WP2PTR(fd);
	packet->packet_id = PACKET_ID_AC_ACK_HASH;
	packet->packet_len = size;
	memcpy(packet->secret, sd->md5key, sd->md5keylen);
	WFIFOSET(fd, size);
}

int lclif_parse(int fd)
{
	struct login_session_data *sd = NULL;
	int i;
	char ip[16];
	uint32 ipl = sockt->session[fd]->client_addr;
	sockt->ip2str(ipl, ip);

	if (sockt->session[fd]->flag.eof) {
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", ip);
		sockt->close(fd);
		return 0;
	}

	if ((sd = sockt->session[fd]->session_data) == NULL) {
		// Perform ip-ban check
		if (login->config->ipban && !sockt->trusted_ip_check(ipl) && ipban_check(ipl)) {
			ShowStatus("Connection refused: IP isn't authorized (deny/allow, ip: %s).\n", ip);
			login_log(ipl, "unknown", -3, "ip banned");
			lclif->login_error(fd, 3); // 3 = Rejected from Server
			sockt->eof(fd);
			return 0;
		}

		// create a session for this new connection
		CREATE(sockt->session[fd]->session_data, struct login_session_data, 1);
		sd = sockt->session[fd]->session_data;
		sd->fd = fd;
	}

	for (i = 0; i < MAX_PROCESSED_PACKETS; ++i) {
		enum parsefunc_rcode result;
		int16 packet_id = RFIFOW(fd, 0);
		int packet_len = (int)RFIFOREST(fd);

		if (packet_len < 2)
			return 0;

		result = lclif->parse_sub(fd, sd);

		switch (result) {
		case PACKET_SKIP:
			continue;
		case PACKET_INCOMPLETE:
		case PACKET_STOPPARSE:
			return 0;
		case PACKET_UNKNOWN:
			ShowWarning("lclif_parse: Received unsupported packet (packet 0x%04x, %d bytes received), disconnecting session #%d.\n", (unsigned int)packet_id, packet_len, fd);
#ifdef DUMP_INVALID_PACKET
			ShowDump(RFIFOP(fd, 0), RFIFOREST(fd));
#endif
			sockt->eof(fd);
			return 0;
		case PACKET_INVALIDLENGTH:
			ShowWarning("lclif_parse: Received packet 0x%04x specifies invalid packet_len (%d), disconnecting session #%d.\n", (unsigned int)packet_id, packet_len, fd);
#ifdef DUMP_INVALID_PACKET
			ShowDump(RFIFOP(fd, 0), RFIFOREST(fd));
#endif
			sockt->eof(fd);
			return 0;
		}
	}
	return 0;
}

enum parsefunc_rcode lclif_parse_sub(int fd, struct login_session_data *sd)
{
	int packet_len = (int)RFIFOREST(fd);
	int16 packet_id = RFIFOW(fd, 0);
	const struct login_packet_db *lpd;

	if (VECTOR_LENGTH(HPM->packets[hpParse_Login]) > 0) {
		int result = HPM->parse_packets(fd, packet_id, hpParse_Login);
		if (result == 1)
			return PACKET_VALID;
		if (result == 2)
			return PACKET_INCOMPLETE; // Packet not completed yet
	}

	lpd = lclif->packet(packet_id);

	if (lpd == NULL)
		return PACKET_UNKNOWN;

	if (lpd->len == 0)
		return PACKET_UNKNOWN;

	if (lpd->len > 0 && lpd->pFunc == NULL)
		return PACKET_UNKNOWN; //This Packet is defined for length purpose ? should never be sent from client ?

	if (lpd->len == -1) {
		uint16 packet_var_len = 0; //Max Variable Packet length is signed int16 size

		if (packet_len < 4)
			return PACKET_INCOMPLETE; //Packet incomplete

		packet_var_len = RFIFOW(fd, 2);

		if (packet_var_len < 4 || packet_var_len > SINT16_MAX)
			return PACKET_INVALIDLENGTH; //Something is wrong, close connection.

		if (RFIFOREST(fd) < packet_var_len)
			return PACKET_INCOMPLETE; //Packet incomplete again.

		return lclif->parse_packet(lpd, fd, sd);
	} else if (lpd->len <= packet_len) {
		return lclif->parse_packet(lpd, fd, sd);
	}

	return PACKET_VALID;
}

const struct login_packet_db *lclif_packet(int16 packet_id)
{
	if (packet_id == PACKET_ID_CA_CHARSERVERCONNECT)
		return &packet_db[0];

	if (packet_id > MAX_PACKET_DB || packet_id < MIN_PACKET_DB)
		return NULL;

	return &packet_db[packet_id];
}

enum parsefunc_rcode lclif_parse_packet(const struct login_packet_db *lpd, int fd, struct login_session_data *sd)
{
	int result;
	result = lpd->pFunc(fd, sd);
	RFIFOSKIP(fd, (lpd->len == -1) ? RFIFOW(fd, 2) : lpd->len);
	return result;
}

void packetdb_loaddb(void)
{
	int i;
	struct packet {
		int16 packet_id;
		int16 packet_len;
		int (*pFunc)(int, struct login_session_data *);
	} packet[] = {
#define packet_def(name) { PACKET_ID_ ## name, sizeof(struct packet_ ## name), lclif_parse_ ## name }
#define packet_def2(name, len) { PACKET_ID_ ## name, (len), lclif_parse_ ## name }
		packet_def(CA_CONNECT_INFO_CHANGED),
		packet_def(CA_EXE_HASHCHECK),
		packet_def(CA_LOGIN),
		packet_def(CA_LOGIN2),
		packet_def(CA_LOGIN3),
		packet_def(CA_LOGIN4),
		packet_def(CA_LOGIN_PCBANG),
		packet_def(CA_LOGIN_HAN),
		packet_def2(CA_SSO_LOGIN_REQ, -1),
		packet_def(CA_REQ_HASH),
#undef packet_def
#undef packet_def2
	};
	int length = ARRAYLENGTH(packet);

	memset(packet_db, '\0', sizeof(packet_db));

	for (i = 0; i < length; ++i) {
		int16 packet_id = packet[i].packet_id;
		Assert_retb(packet_id >= MIN_PACKET_DB && packet_id < MAX_PACKET_DB);
		packet_db[packet_id].len = packet[i].packet_len;
		packet_db[packet_id].pFunc = packet[i].pFunc;
	}

	//Explict case, we will save character login packet in position 0 which is unused and not valid by normal
	packet_db[0].len = sizeof(struct packet_CA_CHARSERVERCONNECT);
	packet_db[0].pFunc = lclif_parse_CA_CHARSERVERCONNECT;
}

void lclif_init(void)
{
	packetdb_loaddb();
}

void lclif_final(void)
{
}

void lclif_defaults(void)
{
	lclif = &lclif_s;

	lclif->init = lclif_init;
	lclif->final = lclif_final;

	lclif->connection_error = lclif_connection_error;
	lclif->server_list = lclif_send_server_list;
	lclif->auth_failed = lclif_send_auth_failed;
	lclif->login_error = lclif_send_login_error;
	lclif->coding_key = lclif_send_coding_key;

	lclif->packet = lclif_packet;
	lclif->parse_packet = lclif_parse_packet;
	lclif->parse = lclif_parse;
	lclif->parse_sub = lclif_parse_sub;
}
