/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // ANTI_MAYAP_CHEAT, RENEWAL, SECURE_NPCTIMEOUT
#include "clif.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/battleground.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clan.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/irc-bot.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/mail.h"
#include "map/map.h"
#include "map/mercenary.h"
#include "map/messages.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/rodex.h"
#include "map/refine.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/stylist.h"
#include "map/storage.h"
#include "map/trade.h"
#include "map/unit.h"
#include "map/vending.h"
#include "map/achievement.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/ers.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h" // NEW_CARTS, char_achievements
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static struct clif_interface clif_s;
struct clif_interface *clif;

static struct s_packet_db packet_db[MAX_PACKET_DB + 1];

/* re-usable */
static struct packet_itemlist_normal itemlist_normal;
static struct packet_itemlist_equip itemlist_equip;
static struct ZC_STORE_ITEMLIST_NORMAL storelist_normal;
static struct ZC_STORE_ITEMLIST_EQUIP storelist_equip;
static struct packet_viewequip_ack viewequip_list;
// temporart buffer for send big packets
char packet_buf[0xffff];
//#define DUMP_UNKNOWN_PACKET
//#define DUMP_INVALID_PACKET

//Converts item type in case of pet eggs.
static inline int itemtype(int type)
{
	switch( type ) {
#if PACKETVER >= 20080827
		case IT_WEAPON:
			return IT_ARMOR;
		case IT_ARMOR:
		case IT_PETARMOR:
#endif
		case IT_PETEGG:
			return IT_WEAPON;
		default:
			return type;
	}
}

static inline void WBUFPOS(uint8 *p, unsigned short pos, short x, short y, unsigned char dir)
{
	p += pos;
	p[0] = (uint8)(x>>2);
	p[1] = (uint8)((x<<6) | ((y>>4)&0x3f));
	p[2] = (uint8)((y<<4) | (dir&0xf));
}

// client-side: x0+=sx0*0.0625-0.5 and y0+=sy0*0.0625-0.5
static inline void WBUFPOS2(uint8 *p, unsigned short pos, short x0, short y0, short x1, short y1, unsigned char sx0, unsigned char sy0)
{
	p += pos;
	p[0] = (uint8)(x0>>2);
	p[1] = (uint8)((x0<<6) | ((y0>>4)&0x3f));
	p[2] = (uint8)((y0<<4) | ((x1>>6)&0x0f));
	p[3] = (uint8)((x1<<2) | ((y1>>8)&0x03));
	p[4] = (uint8)y1;
	p[5] = (uint8)((sx0<<4) | (sy0&0x0f));
}

#if 0 // Currently unused
static inline void WFIFOPOS(int fd, unsigned short pos, short x, short y, unsigned char dir)
{
	WBUFPOS(WFIFOP(fd,pos), 0, x, y, dir);
}
#endif // 0

static inline void WFIFOPOS2(int fd, unsigned short pos, short x0, short y0, short x1, short y1, unsigned char sx0, unsigned char sy0)
{
	WBUFPOS2(WFIFOP(fd,pos), 0, x0, y0, x1, y1, sx0, sy0);
}

static inline void RBUFPOS(const uint8 *p, unsigned short pos, short *x, short *y, unsigned char *dir)
{
	p += pos;

	if( x ) {
		x[0] = ( ( p[0] & 0xff ) << 2 ) | ( p[1] >> 6 );
	}

	if( y ) {
		y[0] = ( ( p[1] & 0x3f ) << 4 ) | ( p[2] >> 4 );
	}

	if( dir ) {
		dir[0] = ( p[2] & 0x0f );
	}
}

static inline void RFIFOPOS(int fd, unsigned short pos, short *x, short *y, unsigned char *dir)
{
	RBUFPOS(RFIFOP(fd,pos), 0, x, y, dir);
}

#if 0 // currently unused
static inline void RBUFPOS2(const uint8 *p, unsigned short pos, short *x0, short *y0, short *x1, short *y1, unsigned char *sx0, unsigned char *sy0)
{
	p += pos;

	if( x0 ) {
		x0[0] = ( ( p[0] & 0xff ) << 2 ) | ( p[1] >> 6 );
	}

	if( y0 ) {
		y0[0] = ( ( p[1] & 0x3f ) << 4 ) | ( p[2] >> 4 );
	}

	if( x1 ) {
		x1[0] = ( ( p[2] & 0x0f ) << 6 ) | ( p[3] >> 2 );
	}

	if( y1 ) {
		y1[0] = ( ( p[3] & 0x03 ) << 8 ) | ( p[4] >> 0 );
	}

	if( sx0 ) {
		sx0[0] = ( p[5] & 0xf0 ) >> 4;
	}

	if( sy0 ) {
		sy0[0] = ( p[5] & 0x0f ) >> 0;
	}
}

static inline void RFIFOPOS2(int fd, unsigned short pos, short *x0, short *y0, short *x1, short *y1, unsigned char *sx0, unsigned char *sy0)
{
	RBUFPOS2(WFIFOP(fd,pos), 0, x0, y0, x1, y1, sx0, sy0);
}
#endif // 0

//To identify disguised characters.
static bool clif_isdisguised(struct block_list *bl)
{
	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	if (sd == NULL || sd->disguise == -1)
		return false;
	return true;
}

/*==========================================
 * Ip setting of map-server
 *------------------------------------------*/
static bool clif_setip(const char *ip)
{
	char ip_str[16];
	nullpo_retr(false, ip);
	clif->map_ip = sockt->host2ip(ip);
	if ( !clif->map_ip ) {
		ShowWarning("Failed to Resolve Map Server Address! (%s)\n", ip);
		return false;
	}

	safestrncpy(clif->map_ip_str, ip, sizeof(clif->map_ip_str));
	ShowInfo("Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(clif->map_ip, ip_str));
	return true;
}

static bool clif_setbindip(const char *ip)
{
	nullpo_retr(false, ip);
	clif->bind_ip = sockt->host2ip(ip);
	if ( clif->bind_ip ) {
		char ip_str[16];
		ShowInfo("Map Server Bind IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(clif->bind_ip, ip_str));
		return true;
	}
	ShowWarning("Failed to Resolve Map Server Address! (%s)\n", ip);
	return false;
}

/*==========================================
 * Sets map port to 'port'
 * is run from map.c upon loading map server configuration
 *------------------------------------------*/
static void clif_setport(uint16 port)
{
	clif->map_port = port;
}

#if 0 // Unused function
/*==========================================
 * Returns map server IP
 *------------------------------------------*/
static uint32 clif_getip(void)
{
	return clif->map_ip;
}
#endif // 0

#if 0 // Unused function
/*==========================================
 * Returns map port which is set by clif_setport()
 *------------------------------------------*/
static uint16 clif_getport(void)
{
	return clif->map_port;
}
#endif // 0

/*==========================================
 * Updates server ip resolution and returns it
 *------------------------------------------*/
static uint32 clif_refresh_ip(void)
{
	uint32 new_ip = sockt->host2ip(clif->map_ip_str);
	if ( new_ip && new_ip != clif->map_ip ) {
		clif->map_ip = new_ip;
		ShowInfo("Updating IP resolution of [%s].\n", clif->map_ip_str);
		return clif->map_ip;
	}
	return 0;
}

static unsigned char clif_bl_type(struct block_list *bl)
{
#if PACKETVER >= 20071106
	struct view_data *vd;
	nullpo_retr(CLUT_NPC, bl);

	switch (bl->type) {
	case BL_PC:
		vd = status->get_viewdata(bl);
		nullpo_retr(CLUT_NPC, vd);

		if (clif->isdisguised(bl) && !pc->db_checkid(vd->class))
			return CLUT_NPC;
		return CLUT_PC;
	case BL_ITEM:
		return CLUT_ITEM;
	case BL_SKILL:
		return CLUT_SKILL;
	case BL_CHAT:
		return CLUT_UNKNOWN;
	case BL_MOB:
		vd = status->get_viewdata(bl);
		nullpo_retr(CLUT_NPC, vd);
		return pc->db_checkid(vd->class) ? CLUT_PC : CLUT_MOB;
	case BL_NPC:
		vd = status->get_viewdata(bl);
		nullpo_retr(CLUT_NPC, vd);
#if PACKETVER >= 20170726
		return CLUT_EVENT;
#else
		return pc->db_checkid(vd->class) ? CLUT_PC : CLUT_EVENT;
#endif
	case BL_PET:
		vd = status->get_viewdata(bl);
		nullpo_retr(CLUT_NPC, vd);
		return pc->db_checkid(vd->class) ? CLUT_PC : CLUT_PET;
	case BL_HOM:
		return CLUT_HOMNUCLUS;
	case BL_MER:
		return CLUT_MERCNARY;
	case BL_ELEM:
		return CLUT_ELEMENTAL;
	default:
		return CLUT_NPC;
	}
#endif
	return CLUT_UNKNOWN;
}

/*==========================================
 * sub process of clif_send
 * Called from a map_foreachinarea (grabs all players in specific area and subjects them to this function)
 * In order to send area-wise packets, such as:
 * - AREA : everyone nearby your area
 * - AREA_WOSC (AREA WITHOUT SAME CHAT) : Not run for people in the same chat as yours
 * - AREA_WOC (AREA WITHOUT CHAT) : Not run for people inside a chat
 * - AREA_WOS (AREA WITHOUT SELF) : Not run for self
 * - AREA_CHAT_WOC : Everyone in the area of your chat without a chat
 *------------------------------------------*/
static int clif_send_sub(struct block_list *bl, va_list ap)
{
	struct block_list *src_bl;
	struct map_session_data *sd;
	void *buf;
	int len, type, fd;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	fd = sd->fd;
	if (!fd || sockt->session[fd] == NULL) //Don't send to disconnected clients.
		return 0;

	buf = va_arg(ap,void*);
	len = va_arg(ap,int);
	nullpo_ret(src_bl = va_arg(ap,struct block_list*));
	type = va_arg(ap,int);

	switch(type) {
		case AREA_WOS:
			if (bl == src_bl)
				return 0;
		break;
		case AREA_WOC:
			if (sd->chat_id != 0 || bl == src_bl)
				return 0;
		break;
		case AREA_WOSC: {
			if (src_bl->type == BL_PC) {
				const struct map_session_data *ssd = BL_UCCAST(BL_PC, src_bl);
				if (ssd != NULL && sd->chat_id != 0 && (sd->chat_id == ssd->chat_id))
					return 0;
			} else if (src_bl->type == BL_NPC) {
				const struct npc_data *nd = BL_UCCAST(BL_NPC, src_bl);
				if (nd != NULL && sd->chat_id != 0 && (sd->chat_id == nd->chat_id))
					return 0;
			}
		}
		break;
/* 0x120 crashes the client when warping for this packetver range [Ind/Hercules], thanks to Judas! */
#if PACKETVER > 20120418 && PACKETVER < 20130000
		case AREA:
			if( WBUFW(buf, 0) == 0x120 && sd->state.warping )
				return 0;
			break;
#endif
	}

	/* unless visible, hold it here */
	if( clif->ally_only && !sd->sc.data[SC_CLAIRVOYANCE] && !sd->special_state.intravision && battle->check_target( src_bl, &sd->bl, BCT_ENEMY ) > 0 )
		return 0;

	return clif->send_actual(fd, buf, len);
}

static int clif_send_actual(int fd, void *buf, int len)
{
	nullpo_retr(0, buf);
	WFIFOHEAD(fd, len);
	if (WFIFOP(fd,0) == buf) {
		ShowError("WARNING: Invalid use of clif->send function\n");
		ShowError("         Packet x%4x use a WFIFO of a player instead of to use a buffer.\n", WBUFW(buf,0));
		ShowError("         Please correct your code.\n");
		// don't send to not move the pointer of the packet for next sessions in the loop
		//WFIFOSET(fd,0);//## TODO is this ok?
		//NO. It is not ok. There is the chance WFIFOSET actually sends the buffer data, and shifts elements around, which will corrupt the buffer.
		return 0;
	}

	memcpy(WFIFOP(fd,0), buf, len);
	WFIFOSET(fd,len);

	return 0;
}

/*==========================================
 * Packet Delegation (called on all packets that require data to be sent to more than one client)
 * functions that are sent solely to one use whose ID it posses use WFIFOSET
 *------------------------------------------*/
static bool clif_send(const void *buf, int len, struct block_list *bl, enum send_target type)
{
	if (type != ALL_CLIENT)
		nullpo_retr(false, bl);
	nullpo_retr(false, buf);
	Assert_retr(false, len > 0);

	int i;
	struct map_session_data *sd = BL_CAST(BL_PC, bl), *tsd;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	struct battleground_data *bgd = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, fd;
	struct s_mapiterator* iter;
	int area_size;

	if (sd != NULL && pc_isinvisible(sd)) {
		if (type == AREA || type == BG || type == BG_AREA)
			type = SELF;
		else if (type == AREA_WOS || type == BG_WOS || type == BG_AREA_WOS)
			return true;
	}

	switch(type) {
		case ALL_CLIENT: //All player clients.
			iter = mapit_getallusers();
			while ((tsd = BL_UCAST(BL_PC, mapit->next(iter))) != NULL) {
				WFIFOHEAD(tsd->fd, len);
				memcpy(WFIFOP(tsd->fd,0), buf, len);
				WFIFOSET(tsd->fd,len);
			}
			mapit->free(iter);
			break;

		case ALL_SAMEMAP: //All players on the same map
			iter = mapit_getallusers();
			while ((tsd = BL_UCAST(BL_PC, mapit->next(iter))) != NULL) {
				if (bl && bl->m == tsd->bl.m) {
					WFIFOHEAD(tsd->fd, len);
					memcpy(WFIFOP(tsd->fd,0), buf, len);
					WFIFOSET(tsd->fd,len);
				}
			}
			mapit->free(iter);
			break;

		case AREA:
		case AREA_WOSC:
		case AREA_DEAD:
			if (sd && bl->prev == NULL) //Otherwise source misses the packet.[Skotlex]
				clif->send (buf, len, bl, SELF);
			/* Fall through */
		case AREA_WOC:
		case AREA_WOS:
			if (type == AREA_DEAD)
				area_size = DEAD_AREA_SIZE;
			else
				area_size = AREA_SIZE;
			nullpo_retr(true, bl);
			map->foreachinarea(clif->send_sub, bl->m, bl->x - area_size, bl->y - area_size, bl->x + area_size, bl->y + area_size,
				BL_PC, buf, len, bl, type);
			break;
		case AREA_CHAT_WOC:
			nullpo_retr(true, bl);
			map->foreachinarea(clif->send_sub, bl->m, bl->x-CHAT_AREA_SIZE, bl->y-CHAT_AREA_SIZE,
			                   bl->x+CHAT_AREA_SIZE, bl->y+CHAT_AREA_SIZE, BL_PC, buf, len, bl, AREA_WOC);
			break;

		case CHAT:
		case CHAT_WOS:
			nullpo_retr(true, bl);
			{
				const struct chat_data *cd = NULL;
				if (sd != NULL) {
					cd = map->id2cd(sd->chat_id);
				} else {
					cd = BL_CCAST(BL_CHAT, bl);
				}
				if (cd == NULL)
					break;
				for(i = 0; i < cd->users; i++) {
					if (type == CHAT_WOS && cd->usersd[i] == sd)
						continue;
					if ((fd=cd->usersd[i]->fd) >0 && sockt->session[fd]) { // Added check to see if session exists [PoW]
						WFIFOHEAD(fd,len);
						memcpy(WFIFOP(fd,0), buf, len);
						WFIFOSET(fd,len);
					}
				}
			}
			break;

		case PARTY_AREA:
		case PARTY_AREA_WOS:
			nullpo_retr(true, bl);
			x0 = bl->x - AREA_SIZE;
			y0 = bl->y - AREA_SIZE;
			x1 = bl->x + AREA_SIZE;
			y1 = bl->y + AREA_SIZE;
			/* Fall through */
		case PARTY:
		case PARTY_WOS:
		case PARTY_SAMEMAP:
		case PARTY_SAMEMAP_WOS:
			if (sd && sd->status.party_id)
				p = party->search(sd->status.party_id);

			if (p) {
				for(i=0;i<MAX_PARTY;i++){
					if( (sd = p->data[i].sd) == NULL )
						continue;

					if( !(fd=sd->fd) )
						continue;

					if( sd->bl.id == bl->id && (type == PARTY_WOS || type == PARTY_SAMEMAP_WOS || type == PARTY_AREA_WOS) )
						continue;

					if( type != PARTY && type != PARTY_WOS && bl->m != sd->bl.m )
						continue;

					if( (type == PARTY_AREA || type == PARTY_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
						continue;

					WFIFOHEAD(fd,len);
					memcpy(WFIFOP(fd,0), buf, len);
					WFIFOSET(fd,len);
				}
				if (!map->enable_spy) //Skip unnecessary parsing. [Skotlex]
					break;

				iter = mapit_getallusers();
				while ((tsd = BL_UCAST(BL_PC, mapit->next(iter))) != NULL) {
					if( tsd->partyspy == p->party.party_id ) {
						WFIFOHEAD(tsd->fd, len);
						memcpy(WFIFOP(tsd->fd,0), buf, len);
						WFIFOSET(tsd->fd,len);
					}
				}
				mapit->free(iter);
			}
			break;

		case DUEL:
		case DUEL_WOS:
			if (!sd || !sd->duel_group) break; //Invalid usage.

			iter = mapit_getallusers();
			while ((tsd = BL_UCAST(BL_PC, mapit->next(iter))) != NULL) {
				if( type == DUEL_WOS && bl->id == tsd->bl.id )
					continue;
				if( sd->duel_group == tsd->duel_group ) {
					WFIFOHEAD(tsd->fd, len);
					memcpy(WFIFOP(tsd->fd,0), buf, len);
					WFIFOSET(tsd->fd,len);
				}
			}
			mapit->free(iter);
			break;

		case SELF:
			if (sd && (fd=sd->fd) != 0) {
				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
			}
			break;

		// New definitions for guilds [Valaris] - Cleaned up and reorganized by [Skotlex]
		case GUILD_AREA:
		case GUILD_AREA_WOS:
			nullpo_retr(true, bl);
			x0 = bl->x - AREA_SIZE;
			y0 = bl->y - AREA_SIZE;
			x1 = bl->x + AREA_SIZE;
			y1 = bl->y + AREA_SIZE;
			/* Fall through */
		case GUILD_SAMEMAP:
		case GUILD_SAMEMAP_WOS:
		case GUILD:
		case GUILD_WOS:
		case GUILD_NOBG:
			if (sd && sd->status.guild_id)
				g = sd->guild;

			if (g) {
				for(i = 0; i < g->max_member; i++) {
					if( (sd = g->member[i].sd) != NULL ) {
						if( !(fd=sd->fd) )
							continue;

						if( type == GUILD_NOBG && sd->bg_id )
							continue;

						if( sd->bl.id == bl->id && (type == GUILD_WOS || type == GUILD_SAMEMAP_WOS || type == GUILD_AREA_WOS) )
							continue;

						if( type != GUILD && type != GUILD_NOBG && type != GUILD_WOS && sd->bl.m != bl->m )
							continue;

						if( (type == GUILD_AREA || type == GUILD_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
							continue;
						WFIFOHEAD(fd,len);
						memcpy(WFIFOP(fd,0), buf, len);
						WFIFOSET(fd,len);
					}
				}
				if (!map->enable_spy) //Skip unnecessary parsing. [Skotlex]
					break;

				iter = mapit_getallusers();
				while ((tsd = BL_UCAST(BL_PC, mapit->next(iter))) != NULL) {
					if( tsd->guildspy == g->guild_id ) {
						WFIFOHEAD(tsd->fd, len);
						memcpy(WFIFOP(tsd->fd,0), buf, len);
						WFIFOSET(tsd->fd,len);
					}
				}
				mapit->free(iter);
			}
			break;

		case BG_AREA:
		case BG_AREA_WOS:
			nullpo_retr(true, bl);
			x0 = bl->x - AREA_SIZE;
			y0 = bl->y - AREA_SIZE;
			x1 = bl->x + AREA_SIZE;
			y1 = bl->y + AREA_SIZE;
			/* Fall through */
		case BG_SAMEMAP:
		case BG_SAMEMAP_WOS:
		case BG:
		case BG_WOS:
			if( sd && sd->bg_id && (bgd = bg->team_search(sd->bg_id)) != NULL ) {
				for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
					if( (sd = bgd->members[i].sd) == NULL || !(fd = sd->fd) )
						continue;
					if( sd->bl.id == bl->id && (type == BG_WOS || type == BG_SAMEMAP_WOS || type == BG_AREA_WOS) )
						continue;
					if( type != BG && type != BG_WOS && sd->bl.m != bl->m )
						continue;
					if( (type == BG_AREA || type == BG_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
						continue;
					WFIFOHEAD(fd,len);
					memcpy(WFIFOP(fd,0), buf, len);
					WFIFOSET(fd,len);
				}
			}
			break;

		case BG_QUEUE:
			if( sd && sd->bg_queue.arena ) {
				struct script_queue *queue = script->queue(sd->bg_queue.arena->queue_id);

				for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
					struct map_session_data *qsd = map->id2sd(VECTOR_INDEX(queue->entries, i));

					if (qsd != NULL) {
						WFIFOHEAD(qsd->fd,len);
						memcpy(WFIFOP(qsd->fd,0), buf, len);
						WFIFOSET(qsd->fd,len);
					}
				}
			}
			break;

		case CLAN:
			if (sd && sd->status.clan_id) {
				struct clan *c = clan->search(sd->status.clan_id);

				nullpo_retr(false, c);

				for (i = 0; i < VECTOR_LENGTH(c->members); i++) {
					if (VECTOR_INDEX(c->members, i).online == 0 || (sd = VECTOR_INDEX(c->members, i).sd) == NULL || (fd = sd->fd) <= 0)
						continue;
					WFIFOHEAD(fd, len);
					memcpy(WFIFOP(fd, 0), buf, len);
					WFIFOSET(fd, len);
				}
			}
			break;

		default:
			ShowError("clif_send: Unrecognized type %u\n", type);
			return false;
	}

	return true;
}

/// Notifies the client, that it's connection attempt was accepted.
/// 0073 <start time>.L <position>.3B <x size>.B <y size>.B (ZC_ACCEPT_ENTER)
/// 02eb <start time>.L <position>.3B <x size>.B <y size>.B <font>.W (ZC_ACCEPT_ENTER2)
static void clif_authok(struct map_session_data *sd)
{
	struct packet_authok p;

	nullpo_retv(sd);
	p.PacketType = authokType;
	p.startTime = (unsigned int)timer->gettick();
	WBUFPOS(&p.PosDir[0],0,sd->bl.x,sd->bl.y,sd->ud.dir); /* do the stupid client math */
	p.xSize = p.ySize = 5; /* not-used */
#if PACKETVER >= 20080102
	p.font = sd->status.font;
#endif
// Some clients smaller than 20160330 cant be tested [4144]
#if PACKETVER >= 20141022 && PACKETVER < 20160330
	p.sex = sd->status.sex;
#endif
	clif->send(&p,sizeof(p),&sd->bl,SELF);
}

/// [4144] Packet not using error_code anymore. Works for fixed error only (MsgString: 9 - Rejected from Server)
/// Notifies the client, that it's connection attempt was refused (ZC_REFUSE_ENTER).
/// 0074 <error code>.B
/// error code:
///     0 = client type mismatch
///     1 = ID mismatch
///     2 = mobile - out of available time
///     3 = mobile - already logged in
///     4 = mobile - waiting state
static void clif_authrefuse(int fd, uint8 error_code)
{
	WFIFOHEAD(fd,packet_len(0x74));
	WFIFOW(fd,0) = 0x74;
	WFIFOB(fd,2) = error_code;
	WFIFOSET(fd,packet_len(0x74));
}

/// Notifies the client of a ban or forced disconnect (SC_NOTIFY_BAN).
/// 0081 <error code>.B
/// error code:
///     0 = BAN_UNFAIR -> "disconnected from server" -> MsgStringTable[3]
///     1 = server closed -> MsgStringTable[4]
///     2 = ID already logged in -> MsgStringTable[5]
///     3 = timeout/too much lag -> MsgStringTable[241]
///     4 = server full -> MsgStringTable[264]
///     5 = underaged -> MsgStringTable[305]
///     8 = Server sill recognizes last connection -> MsgStringTable[441]
///     9 = too many connections from this ip -> MsgStringTable[529]
///     10 = out of available time paid for -> MsgStringTable[530]
///     11 = BAN_PAY_SUSPEND
///     12 = BAN_PAY_CHANGE
///     13 = BAN_PAY_WRONGIP
///     14 = BAN_PAY_PNGAMEROOM
///     15 = disconnected by a GM -> if( servicetype == taiwan ) MsgStringTable[579]
///     16 = BAN_JAPAN_REFUSE1
///     17 = BAN_JAPAN_REFUSE2
///     18 = BAN_INFORMATION_REMAINED_ANOTHER_ACCOUNT
///     100 = BAN_PC_IP_UNFAIR
///     101 = BAN_PC_IP_COUNT_ALL
///     102 = BAN_PC_IP_COUNT
///     103 = BAN_GRAVITY_MEM_AGREE
///     104 = BAN_GAME_MEM_AGREE
///     105 = BAN_HAN_VALID
///     106 = BAN_PC_IP_LIMIT_ACCESS
///     107 = BAN_OVER_CHARACTER_LIST
///     108 = BAN_IP_BLOCK
///     109 = BAN_INVALID_PWD_CNT
///     110 = BAN_NOT_ALLOWED_JOBCLASS
///     113 = access is restricted between the hours of midnight to 6:00am.
///     115 = You are in game connection ban period.
///     ? = disconnected -> MsgStringTable[3]
// TODO: type enum
static void clif_authfail_fd(int fd, int type)
{
	if (!fd || !sockt->session[fd] || sockt->session[fd]->func_parse != clif->parse) //clif_authfail should only be invoked on players!
		return;

	WFIFOHEAD(fd, packet_len(0x81));
	WFIFOW(fd,0) = 0x81;
	WFIFOB(fd,2) = type;
	WFIFOSET(fd,packet_len(0x81));
	sockt->eof(fd);

}

/// Notifies the client, whether it can disconnect and change servers (ZC_RESTART_ACK).
/// 00b3 <type>.B
/// type:
///     1 = disconnect, char-select
///     ? = nothing
static void clif_charselectok(int id, uint8 ok)
{
	struct map_session_data* sd;
	int fd;

	if ((sd = map->id2sd(id)) == NULL || !sd->fd)
		return;

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xb3));
	WFIFOW(fd,0) = 0xb3;
	WFIFOB(fd,2) = ok;
	WFIFOSET(fd,packet_len(0xb3));
}

/// Makes an item appear on the ground.
/// 009e <id>.L <name id>.W <identified>.B <x>.W <y>.W <subX>.B <subY>.B <amount>.W (ZC_ITEM_FALL_ENTRY)
/// 084b (ZC_ITEM_FALL_ENTRY4)
static void clif_dropflooritem(struct flooritem_data *fitem)
{
	struct packet_dropflooritem p;
	int view;

	nullpo_retv(fitem);

	if (fitem->item_data.nameid <= 0)
		return;

	p.PacketType = dropflooritemType;
	p.ITAID = fitem->bl.id;
	p.ITID = ((view = itemdb_viewid(fitem->item_data.nameid)) > 0) ? view : fitem->item_data.nameid;
#if PACKETVER >= 20130000 /* not sure date */
	p.type = itemtype(itemdb_type(fitem->item_data.nameid));
#endif
	p.IsIdentified = fitem->item_data.identify ? 1 : 0;
	p.xPos = fitem->bl.x;
	p.yPos = fitem->bl.y;
	p.subX = fitem->subx;
	p.subY = fitem->suby;
	p.count = fitem->item_data.amount;
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	if (fitem->showdropeffect) {
		p.showdropeffect = itemdb_showdropeffect(fitem->item_data.nameid);
		p.dropeffectmode = itemdb_dropeffectmode(fitem->item_data.nameid);
	} else {
		p.showdropeffect = 0;
		p.dropeffectmode = 0;
	}
#endif
	clif->send(&p, sizeof(p), &fitem->bl, AREA);
}

/// Makes an item disappear from the ground.
/// 00a1 <id>.L (ZC_ITEM_DISAPPEAR)
static void clif_clearflooritem(struct flooritem_data *fitem, int fd)
{
	unsigned char buf[16];

	nullpo_retv(fitem);

	WBUFW(buf,0) = 0xa1;
	WBUFL(buf,2) = fitem->bl.id;

	if (fd == 0) {
		clif->send(buf, packet_len(0xa1), &fitem->bl, AREA);
	} else {
		WFIFOHEAD(fd,packet_len(0xa1));
		memcpy(WFIFOP(fd,0), buf, packet_len(0xa1));
		WFIFOSET(fd,packet_len(0xa1));
	}
}

/// Makes a unit (char, npc, mob, homun) disappear to one client (ZC_NOTIFY_VANISH).
/// 0080 <id>.L <type>.B
/// type:
///     0 = out of sight
///     1 = died
///     2 = logged out
///     3 = teleport
///     4 = trickdead
static void clif_clearunit_single(int id, enum clr_type type, int fd)
{
	WFIFOHEAD(fd, packet_len(0x80));
	WFIFOW(fd,0) = 0x80;
	WFIFOL(fd,2) = id;
	WFIFOB(fd,6) = type;
	WFIFOSET(fd, packet_len(0x80));
}

/// Makes a unit (char, npc, mob, homun) disappear to all clients in area (ZC_NOTIFY_VANISH).
/// 0080 <id>.L <type>.B
/// type:
///     0 = out of sight
///     1 = died
///     2 = logged out
///     3 = teleport
///     4 = trickdead
static void clif_clearunit_area(struct block_list *bl, enum clr_type type)
{
	unsigned char buf[8];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl->id;
	WBUFB(buf,6) = type;

	/**
	 * When monster dies, there's a delay before the packet is sent,
	 * so we send it to a bigger area to avoid clients at the edge
	 * walking out of the area and missing it [KirieZ]
	 */
	clif->send(buf, packet_len(0x80), bl, type == CLR_DEAD ? AREA_DEAD : AREA_WOS);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2) = -bl->id;
		clif->send(buf, packet_len(0x80), bl, SELF);
	}
}

/// Used to make monsters with player-sprites disappear after dying
/// like normal monsters, because the client does not remove those
/// automatically.
static int clif_clearunit_delayed_sub(int tid, int64 tick, int id, intptr_t data)
{
	struct block_list *bl = (struct block_list *)data;
	nullpo_ret(bl);
	Assert_ret(bl->m >= 0 && bl->m < map->count);
	if (map->list[bl->m].block == NULL) {
		// avoid error report for missing/removed map
		ers_free(clif->delay_clearunit_ers, bl);
		return 0;
	}
	clif->clearunit_area(bl, (enum clr_type) id);
	ers_free(clif->delay_clearunit_ers, bl);
	return 0;
}

static void clif_clearunit_delayed(struct block_list *bl, enum clr_type type, int64 tick)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MOB);
	struct mob_data *md = ers_alloc(clif->delay_clearunit_ers, struct mob_data);
	memcpy (md, bl, sizeof (struct mob_data));
	timer->add(tick, clif->clearunit_delayed_sub, (int)type, (intptr_t)md);
}

/// Gets weapon view info from sd's inventory_data and points (*rhand,*lhand)
static void clif_get_weapon_view(struct map_session_data *sd, int *rhand, int *lhand)
{
	nullpo_retv(sd);
	nullpo_retv(rhand);
	nullpo_retv(lhand);
	if(sd->sc.option&OPTION_COSTUME) {
		*rhand = *lhand = 0;
		return;
	}

#if PACKETVER < 4
	*rhand = sd->status.look.weapon;
	*lhand = sd->status.look.shield;
#else
	if (sd->equip_index[EQI_HAND_R] >= 0 &&
		sd->inventory_data[sd->equip_index[EQI_HAND_R]])
	{
		struct item_data* id = sd->inventory_data[sd->equip_index[EQI_HAND_R]];
		if (id->view_id > 0)
			*rhand = id->view_id;
		else
			*rhand = id->nameid;
	} else
		*rhand = 0;

	if (sd->equip_index[EQI_HAND_L] >= 0 &&
		sd->equip_index[EQI_HAND_L] != sd->equip_index[EQI_HAND_R] &&
		sd->inventory_data[sd->equip_index[EQI_HAND_L]])
	{
		struct item_data* id = sd->inventory_data[sd->equip_index[EQI_HAND_L]];
		if (id->view_id > 0)
			*lhand = id->view_id;
		else
			*lhand = id->nameid;
	} else
		*lhand = 0;
#endif
}

//To make the assignation of the level based on limits clearer/easier. [Skotlex]
static int clif_setlevel_sub(int lv)
{
	if( lv < battle_config.max_lv ) {
		;
	} else if( lv < battle_config.aura_lv ) {
		lv = battle_config.max_lv - 1;
	} else {
		lv = battle_config.max_lv;
	}

	return lv;
}

static int clif_setlevel(struct block_list *bl)
{
	int lv = status->get_lv(bl);
	nullpo_retr(0, bl);
	if( battle_config.client_limit_unit_lv&bl->type )
		return clif_setlevel_sub(lv);
	switch( bl->type ) {
		case BL_NPC:
		case BL_PET:
			// npcs and pets do not have level
			return 0;
	}
	return lv;
}

/* for 'packetver < 20091103' 0x78 non-pc-looking unit handling */
static void clif_set_unit_idle2(struct block_list *bl, struct map_session_data *tsd, enum send_target target)
{
#if PACKETVER < 20091103
	struct map_session_data* sd;
	struct status_change* sc = status->get_sc(bl);
	struct view_data* vd = status->get_viewdata(bl);
	struct packet_idle_unit2 p;
	int g_id = status->get_guild_id(bl);

	nullpo_retv(bl);
	nullpo_retv(vd);
	sd = BL_CAST(BL_PC, bl);

	p.PacketType = idle_unit2Type;
#if PACKETVER >= 20071106
	p.objecttype = clif->bl_type(bl);
#endif
	p.GID = bl->id;
	p.speed = status->get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	p.job = vd->class;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
	p.shield = vd->shield;
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	if (bl->type == BL_NPC && vd->class == FLAG_CLASS) {
		// The hell, why flags work like this?
		p.shield = status->get_emblem_id(bl);
		p.accessory2 = GetWord(g_id, 1);
		p.accessory3 = GetWord(g_id, 0);
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
	p.GUID = g_id;
	p.GEmblemVer = status->get_emblem_id(bl);
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.state = vd->dead_sit;
	p.clevel = clif_setlevel(bl);

	clif->send(&p,sizeof(p),tsd?&tsd->bl:bl,target);
#else
	return;
#endif
}

/*==========================================
 * Prepares 'unit standing' packet
 *------------------------------------------*/
static void clif_set_unit_idle(struct block_list *bl, struct map_session_data *tsd, enum send_target target)
{
	struct map_session_data* sd;
	struct status_change* sc = status->get_sc(bl);
	struct view_data* vd = status->get_viewdata(bl);
	struct packet_idle_unit p;
	int g_id = status->get_guild_id(bl);

	nullpo_retv(bl);
	nullpo_retv(vd);

#if PACKETVER < 20091103
	if (!pc->db_checkid(vd->class)) {
		clif->set_unit_idle2(bl,tsd,target);
		return;
	}
#endif

	sd = BL_CAST(BL_PC, bl);

	p.PacketType = idle_unitType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
	p.objecttype = clif->bl_type(bl);
#endif
#if PACKETVER >= 20131223
	p.AID = bl->id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl->id;
#endif
	p.speed = status->get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	p.job = vd->class;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	if (bl->type == BL_NPC && vd->class == FLAG_CLASS) {
		// The hell, why flags work like this?
		p.accessory = status->get_emblem_id(bl);
		p.accessory2 = GetWord(g_id, 1);
		p.accessory3 = GetWord(g_id, 0);
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = g_id;
	p.GEmblemVer = status->get_emblem_id(bl);
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.state = vd->dead_sit;
	p.clevel = clif_setlevel(bl);
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if (clif->show_monster_hp_bar(bl)) {
		p.maxHP = status_get_max_hp(bl);
		p.HP = status_get_hp(bl);
	} else {
		p.maxHP = -1;
		p.HP = -1;
	}
	if (bl->type == BL_MOB) {
		const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
		p.isBoss = (md->spawn != NULL) ? md->spawn->state.boss : BTYPE_NONE;
	} else {
		p.isBoss = BTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy(p.name, clif->get_bl_name(bl), NAME_LENGTH);
#endif
	clif->send(&p,sizeof(p),tsd?&tsd->bl:bl,target);

	if (clif->isdisguised(bl)) {
#if PACKETVER >= 20091103
		p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = -bl->id;
#else
		p.GID = -bl->id;
#endif
#else
		p.GID = -bl->id;
#endif
		clif->send(&p,sizeof(p),bl,SELF);
	}

}

/* for 'packetver < 20091103' 0x7c non-pc-looking unit handling */
static void clif_spawn_unit2(struct block_list *bl, enum send_target target)
{
#if PACKETVER < 20091103
	struct map_session_data* sd;
	struct status_change* sc = status->get_sc(bl);
	struct view_data* vd = status->get_viewdata(bl);
	struct packet_spawn_unit2 p;
	int g_id = status->get_guild_id(bl);

	nullpo_retv(bl);
	nullpo_retv(vd);
	sd = BL_CAST(BL_PC, bl);

	p.PacketType = spawn_unit2Type;
#if PACKETVER >= 20071106
	p.objecttype = clif->bl_type(bl);
#endif
	p.GID = bl->id;
	p.speed = status->get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
	p.job = vd->class;
	p.shield = vd->shield;
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	if (bl->type == BL_NPC && vd->class == FLAG_CLASS) {
		// The hell, why flags work like this?
		p.shield = status->get_emblem_id(bl);
		p.accessory2 = GetWord(g_id, 1);
		p.accessory3 = GetWord(g_id, 0);
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
	p.xSize = p.ySize = (sd) ? 5 : 0;

	clif->send(&p,sizeof(p),bl,target);
#else
	return;
#endif
}

static void clif_spawn_unit(struct block_list *bl, enum send_target target)
{
	struct map_session_data* sd;
	struct status_change* sc = status->get_sc(bl);
	struct view_data* vd = status->get_viewdata(bl);
	struct packet_spawn_unit p;
	int g_id = status->get_guild_id(bl);

	nullpo_retv(bl);
	nullpo_retv(vd);

#if PACKETVER < 20091103
	if (!pc->db_checkid(vd->class)) {
		clif->spawn_unit2(bl,target);
		return;
	}
#endif

	sd = BL_CAST(BL_PC, bl);

	p.PacketType = spawn_unitType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
	p.objecttype = clif->bl_type(bl);
#endif
#if PACKETVER >= 20131223
	p.AID = bl->id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl->id;
#endif
	p.speed = status->get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	p.job = vd->class;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	if (bl->type == BL_NPC && vd->class == FLAG_CLASS) {
		// The hell, why flags work like this?
		p.accessory = status->get_emblem_id(bl);
		p.accessory2 = GetWord(g_id, 1);
		p.accessory3 = GetWord(g_id, 0);
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = g_id;
	p.GEmblemVer = status->get_emblem_id(bl);
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS(&p.PosDir[0],0,bl->x,bl->y,unit->getdir(bl));
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.clevel = clif_setlevel(bl);
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if (clif->show_monster_hp_bar(bl)) {
		p.maxHP = status_get_max_hp(bl);
		p.HP = status_get_hp(bl);
	} else {
		p.maxHP = -1;
		p.HP = -1;
	}
	if (bl->type == BL_MOB) {
		const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
		p.isBoss = (md->spawn != NULL) ? md->spawn->state.boss : BTYPE_NONE;
	} else {
		p.isBoss = BTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy(p.name, clif->get_bl_name(bl), NAME_LENGTH);
#endif
	if (clif->isdisguised(bl)) {
		nullpo_retv(sd);
		if (sd->status.class != sd->disguise)
			clif->send(&p,sizeof(p),bl,target);
#if PACKETVER >= 20091103
		p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = -bl->id;
#else
		p.GID = -bl->id;
#endif
#else
		p.GID = -bl->id;
#endif
		clif->send(&p,sizeof(p),bl,SELF);
	} else
		clif->send(&p,sizeof(p),bl,target);

}

/*==========================================
 * Prepares 'unit walking' packet
 *------------------------------------------*/
static void clif_set_unit_walking(struct block_list *bl, struct map_session_data *tsd, struct unit_data *ud, enum send_target target)
{
	struct map_session_data* sd;
	struct status_change* sc = status->get_sc(bl);
	struct view_data* vd = status->get_viewdata(bl);
	struct packet_unit_walking p;
	int g_id = status->get_guild_id(bl);

	nullpo_retv(bl);
	nullpo_retv(ud);
	nullpo_retv(vd);

	sd = BL_CAST(BL_PC, bl);

	p.PacketType = unit_walkingType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
#endif
#if PACKETVER >= 20071106
	p.objecttype = clif->bl_type(bl);
#endif
#if PACKETVER >= 20131223
	p.AID = bl->id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl->id;
#endif
	p.speed = status->get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	p.job = vd->class;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
	p.moveStartTime = (unsigned int)timer->gettick();
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = g_id;
	p.GEmblemVer = status->get_emblem_id(bl);
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS2(&p.MoveData[0],0,bl->x,bl->y,ud->to_x,ud->to_y,8,8);
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.clevel = clif_setlevel(bl);
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if (clif->show_monster_hp_bar(bl)) {
		p.maxHP = status_get_max_hp(bl);
		p.HP = status_get_hp(bl);
	} else {
		p.maxHP = -1;
		p.HP = -1;
	}
	if (bl->type == BL_MOB) {
		const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
		p.isBoss = (md->spawn != NULL) ? md->spawn->state.boss : BTYPE_NONE;
	} else {
		p.isBoss = BTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy(p.name, clif->get_bl_name(bl), NAME_LENGTH);
#endif

	clif->send(&p,sizeof(p),tsd?&tsd->bl:bl,target);

	if (clif->isdisguised(bl)) {
#if PACKETVER >= 20091103
		p.objecttype = pc->db_checkid(status->get_viewdata(bl)->class) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = -bl->id;
#else
		p.GID = -bl->id;
#endif
#else
		p.GID = -bl->id;
#endif
		clif->send(&p,sizeof(p),bl,SELF);
	}
}

/// Changes sprite of an NPC object (ZC_NPCSPRITE_CHANGE).
/// 01b0 <id>.L <type>.B <value>.L
/// type:
///     unused
static void clif_class_change(struct block_list *bl, int class_, int type, struct map_session_data *sd)
{
	nullpo_retv(bl);

	if(!pc->db_checkid(class_))
	{// player classes yield missing sprites
		unsigned char buf[16];
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=bl->id;
		WBUFB(buf,6)=type;
		WBUFL(buf,7)=class_;

		if (sd == NULL)
			clif->send(buf, packet_len(0x1b0), bl, AREA);
		else
			clif->send(buf, packet_len(0x1b0), &sd->bl, SELF);
	}
}

/// Notifies the client of an object's spirits.
/// 01d0 <id>.L <amount>.W (ZC_SPIRITS)
/// 01e1 <id>.L <amount>.W (ZC_SPIRITS2)
static void clif_spiritball_single(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	WFIFOHEAD(fd, packet_len(0x1e1));
	WFIFOW(fd,0)=0x1e1;
	WFIFOL(fd,2)=sd->bl.id;
	WFIFOW(fd,6)=sd->spiritball;
	WFIFOSET(fd, packet_len(0x1e1));
}

/*==========================================
 * Kagerou/Oboro amulet spirit
 *------------------------------------------*/
static void clif_charm_single(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20110809
	nullpo_retv(sd);
	WFIFOHEAD(fd, packet_len(0x08cf));
	WFIFOW(fd,0) = 0x08cf;
	WFIFOL(fd,2) = sd->bl.id;
	WFIFOW(fd,6) = sd->charm_type;
	WFIFOW(fd,8) = sd->charm_count;
	WFIFOSET(fd, packet_len(0x08cf));
#endif
}

/*==========================================
 * Run when player changes map / refreshes
 * Tells its client to display all weather settings being used by this map
 *------------------------------------------*/
static void clif_weather_check(struct map_session_data *sd)
{
	int16 m;
	int fd;

	nullpo_retv(sd);
	m = sd->bl.m;
	fd = sd->fd;
	if (map->list[m].flag.snow)
		clif->specialeffect_single(&sd->bl, 162, fd);
	if (map->list[m].flag.clouds)
		clif->specialeffect_single(&sd->bl, 233, fd);
	if (map->list[m].flag.clouds2)
		clif->specialeffect_single(&sd->bl, 516, fd);
	if (map->list[m].flag.fog)
		clif->specialeffect_single(&sd->bl, 515, fd);
	if (map->list[m].flag.fireworks) {
		clif->specialeffect_single(&sd->bl, 297, fd);
		clif->specialeffect_single(&sd->bl, 299, fd);
		clif->specialeffect_single(&sd->bl, 301, fd);
	}
	if (map->list[m].flag.sakura)
		clif->specialeffect_single(&sd->bl, 163, fd);
	if (map->list[m].flag.leaves)
		clif->specialeffect_single(&sd->bl, 333, fd);
}

/**
 * Run when the weather on a map changes, throws all players in map id 'm' to clif_weather_check function
 **/
static void clif_weather(int16 m)
{
	struct s_mapiterator* iter;
	struct map_session_data *sd=NULL;

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if( sd->bl.m == m )
			clif->weather_check(sd);
	}
	mapit->free(iter);
}

/**
 * Main function to spawn a unit on the client (player/mob/pet/etc)
 **/
static bool clif_spawn(struct block_list *bl)
{
	struct view_data *vd;

	nullpo_retr(false, bl);
	vd = status->get_viewdata(bl);
	if( !vd )
		return false;

	if (vd->class == INVISIBLE_CLASS)
		return true; // Doesn't need to be spawned, so everything is alright

	if (bl->type == BL_NPC) {
		struct npc_data *nd = BL_UCAST(BL_NPC, bl);
		if (nd->chat_id == 0 && (nd->option&OPTION_INVISIBLE)) // Hide NPC from maya purple card.
			return true; // Doesn't need to be spawned, so everything is alright
	}

	clif->spawn_unit(bl,AREA_WOS);

	if (vd->cloth_color)
		clif->refreshlook(bl,bl->id,LOOK_CLOTHES_COLOR,vd->cloth_color,AREA_WOS);
	if (vd->body_style)
		clif->refreshlook(bl,bl->id,LOOK_BODY2,vd->body_style,AREA_WOS);

	switch (bl->type) {
		case BL_PC:
		{
			struct map_session_data *sd = BL_UCAST(BL_PC, bl);
			int i;
			if (sd->spiritball > 0)
				clif->spiritball(&sd->bl);
			if (sd->state.size == SZ_BIG) // tiny/big players [Valaris]
				clif->specialeffect(bl,423,AREA);
			else if (sd->state.size == SZ_MEDIUM)
				clif->specialeffect(bl,421,AREA);
			if (sd->bg_id != 0 && map->list[sd->bl.m].flag.battleground)
				clif->sendbgemblem_area(sd);

			struct status_change *sc = status->get_sc(bl);

			if (sd->sc_display_count > 0 && sc != NULL) {
				for (i = 0; i < sd->sc_display_count; i++) {
					enum sc_type type = sd->sc_display[i]->type;

					if (sc->data[type] == NULL)
						continue;

					int tick = 0;
					int tid = sc->data[type]->timer;
					const struct TimerData *td = (tid > 0) ? timer->get(tid) : NULL;

					if (td != NULL)
						tick = DIFF_TICK32(td->tick, timer->gettick());

					int sc_icon = status->get_sc_icon(type);
					int sc_types = status->get_sc_relevant_bl_types(type);
					int val1 = sd->sc_display[i]->val1;
					int val2 = sd->sc_display[i]->val2;
					int val3 = sd->sc_display[i]->val3;

					clif->status_change(&sd->bl, sc_icon, sc_types, 1, tick, val1, val2, val3);
				}
			}

			if (sd->charm_type != CHARM_TYPE_NONE && sd->charm_count > 0)
				clif->spiritcharm(sd);
			if (sd->status.look.robe != 0)
				clif->refreshlook(bl, bl->id, LOOK_ROBE, sd->status.look.robe, AREA);
			clif->hat_effect(bl, NULL, AREA);
		}
			break;
		case BL_MOB:
		{
			struct mob_data *md = BL_UCAST(BL_MOB, bl);
			if (md->special_state.size==SZ_BIG) // tiny/big mobs [Valaris]
				clif->specialeffect(&md->bl,423,AREA);
			else if (md->special_state.size==SZ_MEDIUM)
				clif->specialeffect(&md->bl,421,AREA);
		}
			break;
		case BL_NPC:
		{
			struct npc_data *nd = BL_UCAST(BL_NPC, bl);
			if (nd->size == SZ_BIG)
				clif->specialeffect(&nd->bl,423,AREA);
			else if (nd->size == SZ_MEDIUM)
				clif->specialeffect(&nd->bl,421,AREA);
			if (nd->clan_id > 0)
				clif->sc_load(&nd->bl, nd->bl.id, AREA, status->get_sc_icon(SC_CLAN_INFO), 0, nd->clan_id, 0);
		}
			break;
		case BL_PET:
			if (vd->head_bottom)
				clif->send_petdata(NULL, BL_UCAST(BL_PET, bl), 3, vd->head_bottom); // needed to display pet equip properly
			break;
	}
	return true;
}

/// Sends information about owned homunculus to the client (ZC_PROPERTY_HOMUN). [orn]
/// 022e <name>.24B <modified>.B <level>.W <hunger>.W <intimacy>.W <equip id>.W <atk>.W <matk>.W <hit>.W <crit>.W <def>.W <mdef>.W <flee>.W <aspd>.W <hp>.W <max hp>.W <sp>.W <max sp>.W <exp>.L <max exp>.L <skill points>.W <atk range>.W
static void clif_hominfo(struct map_session_data *sd, struct homun_data *hd, int flag)
{
#if PACKETVER_MAIN_NUM >= 20101005 || PACKETVER_RE_NUM >= 20080827 || defined(PACKETVER_ZERO)
	struct status_data *hstatus;
	enum homun_type htype;
	struct PACKET_ZC_PROPERTY_HOMUN p;

	nullpo_retv(sd);
	nullpo_retv(hd);

	hstatus  = &hd->battle_status;
	htype = homun->class2type(hd->homunculus.class_);

	memset(&p, 0, sizeof(p));
	p.packetType = HEADER_ZC_PROPERTY_HOMUN;
	memcpy(p.name, hd->homunculus.name, NAME_LENGTH);
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	p.flags = (!battle_config.hom_rename && hd->homunculus.rename_flag ? 0x1 : 0x0) | (hd->homunculus.vaporize == HOM_ST_REST ? 0x2 : 0) | (hd->homunculus.hp > 0 ? 0x4 : 0);
	p.level = hd->homunculus.level;
	p.hunger = hd->homunculus.hunger;
	p.intimacy = hd->homunculus.intimacy / 100;
#if !(PACKETVER_MAIN_NUM >= 20190619 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190626)
	p.itemId = 0; // equip id
#endif
#ifdef RENEWAL
	p.atk2 = cap_value(hstatus->rhw.atk2, 0, INT16_MAX);
#else
	p.atk2 = cap_value(hstatus->rhw.atk2 + hstatus->batk, 0, INT16_MAX);
#endif
	p.matk = cap_value(hstatus->matk_max, 0, INT16_MAX);
	p.hit = hstatus->hit;
	if (battle_config.hom_setting&0x10)
		p.crit = hstatus->luk / 3 + 1; //crit is a +1 decimal value! Just display purpose.[Vicious]
	else
		p.crit = hstatus->cri / 10;
#ifdef RENEWAL
	p.def = hstatus->def + hstatus->def2;
	p.mdef = hstatus->mdef + hstatus->mdef2;
#else
	p.def = hstatus->def + hstatus->vit ;
	p.mdef = hstatus->mdef;
#endif
	p.flee = hstatus->flee;
	p.amotion = (flag) ? 0 : hstatus->amotion;

// probably can works also for < 20141223, but in 3CeaM packet size defined only for 20150513
#if PACKETVER < 20150513
	if (hstatus->max_hp > INT16_MAX) {
		p.hp = hstatus->hp / (hstatus->max_hp / 100);
		p.maxHp = 100;
	} else {
		p.hp = hstatus->hp;
		p.maxHp = hstatus->max_hp;
	}
#else
	p.hp = hstatus->hp;
	p.maxHp = hstatus->max_hp;
#endif

	if (hstatus->max_sp > INT16_MAX) {
		p.sp = hstatus->sp / (hstatus->max_sp / 100);
		p.maxSp = 100;
	} else {
		p.sp = hstatus->sp;
		p.maxSp = hstatus->max_sp;
	}
	p.exp = hd->homunculus.exp;
	p.expNext = hd->exp_next;
	switch (htype) {
		case HT_REG:
		case HT_EVO:
			if (hd->homunculus.level >= battle_config.hom_max_level)
				p.expNext = 0;
			break;
		case HT_S:
			if (hd->homunculus.level >= battle_config.hom_S_max_level)
				p.expNext = 0;
			break;
	}
	p.skillPoints = hd->homunculus.skillpts;
	p.range = status_get_range(&hd->bl);
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/// Notification about a change in homunuculus' state (ZC_CHANGESTATE_MER).
/// 0230 <type>.B <state>.B <id>.L <data>.L
/// type:
///     unused
/// state:
///     0 = pre-init
///     1 = intimacy
///     2 = hunger
///     3 = accessory?
///     ? = ignored
static void clif_send_homdata(struct map_session_data *sd, int state, int param)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(sd->hd);

	fd = sd->fd;
	if ( (state == SP_INTIMATE) && (param >= 910) && (sd->hd->homunculus.class_ == sd->hd->homunculusDB->evo_class) )
		homun->calc_skilltree(sd->hd, 0);

	WFIFOHEAD(fd, packet_len(0x230));
	WFIFOW(fd,0)=0x230;
	WFIFOB(fd,2)=0;
	WFIFOB(fd,3)=state;
	WFIFOL(fd,4)=sd->hd->bl.id;
	WFIFOL(fd,8)=param;
	WFIFOSET(fd,packet_len(0x230));
}

/// Prepares and sends homun related information [orn]
static void clif_homskillinfoblock(struct map_session_data *sd)
{
	struct homun_data *hd;
	int fd;
	int i,j;
	int len=4;
	nullpo_retv(sd);

	fd = sd->fd;
	hd = sd->hd;

	if ( !hd )
		return;

	WFIFOHEAD(fd, 4+37*MAX_HOMUNSKILL);
	WFIFOW(fd,0)=0x235;

	for ( i = 0; i < MAX_HOMUNSKILL; i++ ) {
		int id = hd->homunculus.hskill[i].id;
		if ( id != 0 ) {
			j = id - HM_SKILLBASE;
			WFIFOW(fd, len) = id;
			WFIFOL(fd, len + 2) = skill->get_inf(id);
			WFIFOW(fd, len + 6) = hd->homunculus.hskill[j].lv;
			if ( hd->homunculus.hskill[j].lv ) {
				WFIFOW(fd, len + 8) = skill->get_sp(id, hd->homunculus.hskill[j].lv);
				WFIFOW(fd, len + 10) = skill->get_range2(&sd->hd->bl, id, hd->homunculus.hskill[j].lv);
			} else {
				WFIFOW(fd, len + 8) = 0;
				WFIFOW(fd, len + 10) = 0;
			}
			safestrncpy(WFIFOP(fd, len + 12), skill->get_name(id), NAME_LENGTH);
			WFIFOB(fd, len + 36) = (hd->homunculus.hskill[j].lv < homun->skill_tree_get_max(id, hd->homunculus.class_)) ? 1 : 0;
			len += 37;
		}
	}
	WFIFOW(fd,2)=len;
	WFIFOSET(fd,len);

	return;
}

//[orn]
static void clif_homskillup(struct map_session_data *sd, uint16 skill_id)
{
	struct homun_data *hd;
	int fd, idx;
	nullpo_retv(sd);
	nullpo_retv(sd->hd);
	idx = skill_id - HM_SKILLBASE;

	fd=sd->fd;
	hd=sd->hd;

	WFIFOHEAD(fd, packet_len(0x239));
	WFIFOW(fd,0) = 0x239;
	WFIFOW(fd,2) = skill_id;
	WFIFOW(fd,4) = hd->homunculus.hskill[idx].lv;
	WFIFOW(fd,6) = skill->get_sp(skill_id,hd->homunculus.hskill[idx].lv);
	WFIFOW(fd,8) = skill->get_range2(&hd->bl, skill_id,hd->homunculus.hskill[idx].lv);
	WFIFOB(fd,10) = (hd->homunculus.hskill[idx].lv < skill->get_max(hd->homunculus.hskill[idx].id)) ? 1 : 0;
	WFIFOSET(fd,packet_len(0x239));
}

/// Result of request to feed a homun/merc (ZC_FEED_MER).
/// 022f <result>.B <name id>.W
/// result:
///     0 = failure
///     1 = success
static void clif_hom_food(struct map_session_data *sd, int foodid, int fail)
{
	int fd;
	struct PACKET_ZC_FEED_MER p;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x22f;
	p.result = fail;
	p.itemId = foodid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Notifies the client, that it is walking (ZC_NOTIFY_PLAYERMOVE).
/// 0087 <walk start time>.L <walk data>.6B
static void clif_walkok(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x87));
	WFIFOW(fd,0)=0x87;
	WFIFOL(fd,2)=(unsigned int)timer->gettick();
	WFIFOPOS2(fd,6,sd->bl.x,sd->bl.y,sd->ud.to_x,sd->ud.to_y,8,8);
	WFIFOSET(fd,packet_len(0x87));
}

static void clif_move2(struct block_list *bl, struct view_data *vd, struct unit_data *ud)
{
#ifdef ANTI_MAYAP_CHEAT
	struct status_change *sc = NULL;
#endif

	nullpo_retv(bl);
	nullpo_retv(vd);
	nullpo_retv(ud);

#ifdef ANTI_MAYAP_CHEAT
	if( (sc = status->get_sc(bl)) && sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_INVISIBLE|OPTION_CHASEWALK) )
		clif->ally_only = true;
#endif

	clif->set_unit_walking(bl,NULL,ud,AREA_WOS);

	if(vd->cloth_color)
		clif->refreshlook(bl,bl->id,LOOK_CLOTHES_COLOR,vd->cloth_color,AREA_WOS);
	if (vd->body_style)
		clif->refreshlook(bl,bl->id,LOOK_BODY2,vd->body_style,AREA_WOS);

	switch(bl->type) {
		case BL_PC:
		{
			struct map_session_data *sd = BL_UCAST(BL_PC, bl);
			//clif_movepc(sd);
			if(sd->state.size==SZ_BIG) // tiny/big players [Valaris]
				clif->specialeffect(&sd->bl,423,AREA);
			else if(sd->state.size==SZ_MEDIUM)
				clif->specialeffect(&sd->bl,421,AREA);
		}
			break;
		case BL_MOB:
		{
			struct mob_data *md = BL_UCAST(BL_MOB, bl);
			if (md->special_state.size == SZ_BIG) // tiny/big mobs [Valaris]
				clif->specialeffect(&md->bl,423,AREA);
			else if (md->special_state.size == SZ_MEDIUM)
				clif->specialeffect(&md->bl,421,AREA);
		}
			break;
		case BL_PET:
			if( vd->head_bottom ) // needed to display pet equip properly
				clif->send_petdata(NULL, BL_UCAST(BL_PET, bl), 3, vd->head_bottom);
			break;
	}
#ifdef ANTI_MAYAP_CHEAT
	clif->ally_only = false;
#endif
}

/// Notifies clients in an area, that an other visible object is walking (ZC_NOTIFY_PLAYERMOVE).
/// 0086 <id>.L <walk data>.6B <walk start time>.L
/// Note: unit must not be self
static void clif_move(struct unit_data *ud)
{
	unsigned char buf[16];
	struct view_data *vd;
	struct block_list *bl;
#ifdef ANTI_MAYAP_CHEAT
	struct status_change *sc = NULL;
#endif

	nullpo_retv(ud);
	bl = ud->bl;
	nullpo_retv(bl);
	vd = status->get_viewdata(bl);
	if (vd == NULL || vd->class == INVISIBLE_CLASS)
		return; //This performance check is needed to keep GM-hidden objects from being notified to bots.

	if (bl->type == BL_NPC) {
		// Hide NPC from maya purple card.
		struct npc_data *nd = BL_UCAST(BL_NPC, bl);
		if (nd->chat_id == 0 && (nd->option&OPTION_INVISIBLE))
			return;
	}

	if (ud->state.speed_changed) {
		// Since we don't know how to update the speed of other objects,
		// use the old walk packet to update the data.
		ud->state.speed_changed = 0;
		clif->move2(bl, vd, ud);
		return;
	}
#ifdef ANTI_MAYAP_CHEAT
	if( (sc = status->get_sc(bl)) && sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_INVISIBLE) )
		clif->ally_only = true;
#endif

	WBUFW(buf,0)=0x86;
	WBUFL(buf,2)=bl->id;
	WBUFPOS2(buf,6,bl->x,bl->y,ud->to_x,ud->to_y,8,8);
	WBUFL(buf,12)=(unsigned int)timer->gettick();

	clif->send(buf, packet_len(0x86), bl, AREA_WOS);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2)=-bl->id;
		clif->send(buf, packet_len(0x86), bl, SELF);
	}
#ifdef ANTI_MAYAP_CHEAT
	clif->ally_only = false;
#endif
}

/*==========================================
 * Delays the map->quit of a player after they are disconnected. [Skotlex]
 *------------------------------------------*/
static int clif_delayquit(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = NULL;

	//Remove player from map server
	if ((sd = map->id2sd(id)) != NULL && sd->fd == 0) //Should be a disconnected player.
		map->quit(sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static void clif_quitsave(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	if (!battle_config.prevent_logout ||
		DIFF_TICK(timer->gettick(), sd->canlog_tick) > battle_config.prevent_logout)
		map->quit(sd);
	else if (sd->fd) {
		//Disassociate session from player (session is deleted after this function was called)
		//And set a timer to make him quit later.
		sockt->session[sd->fd]->session_data = NULL;
		sd->fd = 0;
		timer->add(timer->gettick() + 10000, clif->delayquit, sd->bl.id, 0);
	}
}

/// Notifies the client of a position change to coordinates on given map (ZC_NPCACK_MAPMOVE).
/// 0091 <map name>.16B <x>.W <y>.W
static void clif_changemap(struct map_session_data *sd, short m, int x, int y)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x91));
	WFIFOW(fd,0) = 0x91;
	mapindex->getmapname_ext(map->list[m].custom_name ? map->list[map->list[m].instance_src_map].name : map->list[m].name, WFIFOP(fd,2));
	WFIFOW(fd,18) = x;
	WFIFOW(fd,20) = y;
	WFIFOSET(fd,packet_len(0x91));
}

/// Notifies the client of a position change (on air ship) to coordinates on given map (ZC_AIRSHIP_MAPMOVE).
/// 0A4B <map name>.16B <x>.W <y>.W
static void clif_changemap_airship(struct map_session_data *sd, short m, int x, int y)
{
#if PACKETVER_MAIN_NUM >= 20180620 || PACKETVER_RE_NUM >= 20180321 || PACKETVER_ZERO_NUM >= 20171027
	// [4144] this packet is not used yet by kro, but it here
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa4b));
	WFIFOW(fd, 0) = 0xa4b;
	mapindex->getmapname_ext(map->list[m].custom_name ? map->list[map->list[m].instance_src_map].name : map->list[m].name, WFIFOP(fd,2));
	WFIFOW(fd, 18) = x;
	WFIFOW(fd, 20) = y;
	WFIFOSET(fd, packet_len(0xa4b));
#endif
}

/// Notifies the client of a position change to coordinates on given map, which is on another map-server (ZC_NPCACK_SERVERMOVE).
/// 0092 <map name>.16B <x>.W <y>.W <ip>.L <port>.W
/// 0ac7 <map name>.16B <x>.W <y>.W <ip>.L <port>.W <dns host>.128B
static void clif_changemapserver(struct map_session_data *sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port, char *dnsHost)
{
	int fd;
#if PACKETVER >= 20170315
	const int cmd = 0xac7;
#else
	const int cmd = 0x92;
#endif
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd, 0) = cmd;
	mapindex->getmapname_ext(mapindex_id2name(map_index), WFIFOP(fd, 2));
	WFIFOW(fd, 18) = x;
	WFIFOW(fd, 20) = y;
	WFIFOL(fd, 22) = htonl(ip);
	WFIFOW(fd, 26) = sockt->ntows(htons(port)); // [!] LE byte order here [!]

#if PACKETVER >= 20170315
	if (dnsHost != NULL) {
		safestrncpy(WFIFOP(fd, 28), dnsHost, 128);
	} else {
		memset(WFIFOP(fd, 28), 0, 128);
	}
#endif

	WFIFOSET(fd, packet_len(cmd));
}

/// Notifies the client of a position change (with air ship) to coordinates on given map, which is on another map-server (ZC_NPCACK_SERVERMOVE).
/// 0a4c <map name>.16B <x>.W <y>.W <ip>.L <port>.W
static void clif_changemapserver_airship(struct map_session_data *sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port)
{
#if (PACKETVER_MAIN_NUM >= 20180620) || (PACKETVER_RE_NUM >= 20180321) || (PACKETVER_ZERO_NUM >= 20171027)
	// [4144] this packet is not used yet by kro, but it here
	int fd;
	const int cmd = 0xa4c;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd, 0) = cmd;
	mapindex->getmapname_ext(mapindex_id2name(map_index), WFIFOP(fd, 2));
	WFIFOW(fd, 18) = x;
	WFIFOW(fd, 20) = y;
	WFIFOL(fd, 22) = htonl(ip);
	WFIFOW(fd, 26) = sockt->ntows(htons(port)); // [!] LE byte order here [!]
	WFIFOSET(fd, packet_len(cmd));
#endif
}

static void clif_blown(struct block_list *bl)
{
//Aegis packets says fixpos, but it's unsure whether slide works better or not.
	nullpo_retv(bl);
	clif->fixpos(bl);
	clif->slide(bl, bl->x, bl->y);
}

/// Visually moves(slides) a character to x,y. If the target cell
/// isn't walkable, the char doesn't move at all. If the char is
/// sitting it will stand up (ZC_STOPMOVE).
/// 0088 <id>.L <x>.W <y>.W
static void clif_fixpos(struct block_list *bl)
{
	unsigned char buf[10];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x88;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = bl->x;
	WBUFW(buf,8) = bl->y;
	clif->send(buf, packet_len(0x88), bl, AREA);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2) = -bl->id;
		clif->send(buf, packet_len(0x88), bl, SELF);
	}
}

/// Displays the buy/sell dialog of an NPC shop (ZC_SELECT_DEALTYPE).
/// 00c4 <shop id>.L
static void clif_npcbuysell(struct map_session_data *sd, int id)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0xc4));
	WFIFOW(fd,0)=0xc4;
	WFIFOL(fd,2)=id;
	WFIFOSET(fd,packet_len(0xc4));
}

/// Presents list of items, that can be bought in an NPC shop (ZC_PC_PURCHASE_ITEMLIST).
/// 00c6 <packet len>.W { <price>.L <discount price>.L <item type>.B <name id>.W }*
static void clif_buylist(struct map_session_data *sd, struct npc_data *nd)
{
	struct npc_item_list *shop = NULL;
	unsigned short shop_size = 0;
	int fd, i, c, len;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST *p;

	nullpo_retv(sd);
	nullpo_retv(nd);

	if (nd->subtype == SCRIPT) {
		shop = nd->u.scr.shop->item;
		shop_size = nd->u.scr.shop->items;
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}

	fd = sd->fd;

	len = sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST) + shop_size * sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0xc6;

	c = 0;
	for (i = 0; i < shop_size; i++) {
		if (shop[i].nameid) {
			struct item_data* id = itemdb->exists(shop[i].nameid);
			int val = shop[i].value;
			if (id == NULL)
				continue;
			p->items[c].price = val;
			p->items[c].discountPrice = pc->modifybuyvalue(sd, val);
			p->items[c].itemType = itemtype(id->type);
			p->items[c].itemId = (id->view_id > 0) ? id->view_id : id->nameid;
			c++;
		}
	}

	len = sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST) + c * sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub);
	p->packetLength = len;
	WFIFOSET(fd, len);
}

/// Presents list of items, that can be sold to an NPC shop (ZC_PC_SELL_ITEMLIST).
/// 00c7 <packet len>.W { <index>.W <price>.L <overcharge price>.L }*
static void clif_selllist(struct map_session_data *sd)
{
	int c = 0, val;

	nullpo_retv(sd);

	int fd = sd->fd;
	WFIFOHEAD(fd, sd->status.inventorySize * 10 + 4);
	WFIFOW(fd,0)=0xc7;
	for (int i = 0; i < sd->status.inventorySize; i++)
	{
		if( sd->status.inventory[i].nameid > 0 && sd->inventory_data[i] )
		{
			if( !itemdb_cansell(&sd->status.inventory[i], pc_get_group_level(sd)) )
				continue;

			if (sd->status.inventory[i].favorite != 0)
				continue; // Cannot Sell Favorite item

			if( sd->status.inventory[i].expire_time )
				continue; // Cannot Sell Rental Items

			if( sd->status.inventory[i].bound && !pc_can_give_bound_items(sd))
				continue; // Don't allow sale of bound items

			val=sd->inventory_data[i]->value_sell;
			if( val < 0 )
				continue;
			WFIFOW(fd,4+c*10)=i+2;
			WFIFOL(fd,6+c*10)=val;
			WFIFOL(fd,10+c*10)=pc->modifysellvalue(sd,val);
			c++;
		}
	}
	WFIFOW(fd,2)=c*10+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Displays an NPC dialog message (ZC_SAY_DIALOG).
/// 00b4 <packet len>.W <npc id>.L <message>.?B
/// Client behavior (dialog window):
/// - disable mouse targeting
/// - open the dialog window
/// - set npcid of dialog window (0 by default)
/// - if set to clear on next mes, clear contents
/// - append this text
static void clif_scriptmes(struct map_session_data *sd, int npcid, const char *mes)
{
	int fd, slen;
#ifdef SCRIPT_MES_STRIP_LINEBREAK
	char *stripmes = NULL;
	int i;
#endif

	nullpo_retv(sd);
	nullpo_retv(mes);

	fd = sd->fd;
	slen = (int)strlen(mes) + 9;
	Assert_retv(slen <= INT16_MAX);

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	sd->state.dialog = 1;

	WFIFOHEAD(fd, slen);
	WFIFOW(fd,0) = 0xb4;
	WFIFOW(fd,2) = slen;
	WFIFOL(fd,4) = npcid;
#ifdef SCRIPT_MES_STRIP_LINEBREAK
	stripmes = aStrdup(mes);
	for (i = 0; stripmes[i] != '\0'; ++i) {
		if (stripmes[i] == '\r')
			stripmes[i] = ' ';
	}
	memcpy(WFIFOP(fd,8), stripmes, slen-8);
	aFree(stripmes);
#else // ! SCRIPT_MES_STRIP_LINEBREAK
	memcpy(WFIFOP(fd,8), mes, slen-8);
#endif // SCRIPT_MES_STRIP_LINEBREAK
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Adds a 'next' button to an NPC dialog (ZC_WAIT_DIALOG).
/// 00b5 <npc id>.L
/// Client behavior (dialog window):
/// - disable mouse targeting
/// - open the dialog window
/// - add 'next' button
/// When 'next' is pressed:
/// - 00B9 <npcid of dialog window>.L
/// - set to clear on next mes
/// - remove 'next' button
static void clif_scriptnext(struct map_session_data *sd, int npcid)
{
	int fd;

	nullpo_retv(sd);

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0xb5));
	WFIFOW(fd,0)=0xb5;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len(0xb5));
}

/// Adds a 'close' button to an NPC dialog (ZC_CLOSE_DIALOG).
/// 00b6 <npc id>.L
/// Client behavior:
/// - if dialog window is open:
///   - remove 'next' button
///   - add 'close' button
/// - else:
///   - enable mouse targeting
///   - close the dialog window
///   - close the menu window
/// When 'close' is pressed:
/// - enable mouse targeting
/// - close the dialog window
/// - close the menu window
/// - 0146 <npcid of dialog window>.L
static void clif_scriptclose(struct map_session_data *sd, int npcid)
{
	int fd;

	nullpo_retv(sd);

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0xb6));
	WFIFOW(fd,0)=0xb6;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len(0xb6));
}

/*==========================================
 *
 *------------------------------------------*/
static void clif_sendfakenpc(struct map_session_data *sd, int npcid)
{
	unsigned char *buf;
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	sd->state.using_fake_npc = 1;
	WFIFOHEAD(fd, packet_len(0x78));
	buf = WFIFOP(fd,0);
	memset(WBUFP(buf,0), 0, packet_len(0x78));
	WBUFW(buf,0)=0x78;
#if PACKETVER >= 20071106
	WBUFB(buf,2) = 0; // object type
	buf = WFIFOP(fd,1);
#endif
	WBUFL(buf,2)=npcid;
	WBUFW(buf,14)=111;
	WBUFPOS(buf,46,sd->bl.x,sd->bl.y,sd->ud.dir);
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WFIFOSET(fd, packet_len(0x78));
}

/// Displays an NPC dialog menu (ZC_MENU_LIST).
/// 00b7 <packet len>.W <npc id>.L <menu items>.?B
/// Client behavior:
/// - disable mouse targeting
/// - close the menu window
/// - open the menu window
/// - add options to the menu (separated in the text by ":")
/// - set npcid of menu window
/// - if dialog window is open:
///   - remove 'next' button
/// When 'ok' is pressed:
/// - 00B8 <npcid of menu window>.L <selected option>.B
/// - close the menu window
/// When 'cancel' is pressed:
/// - 00B8 <npcid of menu window>.L <-1>.B
/// - enable mouse targeting
/// - close a bunch of windows...
/// WARNING: the 'cancel' button closes other windows besides the dialog window and the menu window.
///    Which suggests their have intertwined behavior. (probably the mouse targeting)
/// TODO investigate behavior of other windows [FlavioJS]
static void clif_scriptmenu(struct map_session_data *sd, int npcid, const char *mes)
{
	int fd, slen;
	struct block_list *bl = NULL;

	nullpo_retv(sd);
	nullpo_retv(mes);

	fd = sd->fd;
	slen = (int)strlen(mes) + 9;
	Assert_retv(slen <= INT16_MAX);

	if (!sd->state.using_fake_npc && (npcid == npc->fake_nd->bl.id || ((bl = map->id2bl(npcid)) != NULL && (bl->m!=sd->bl.m ||
						bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
						bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1))))
		clif->sendfakenpc(sd, npcid);

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	WFIFOHEAD(fd, slen);
	WFIFOW(fd,0) = 0xb7;
	WFIFOW(fd,2) = slen;
	WFIFOL(fd,4) = npcid;
	memcpy(WFIFOP(fd,8), mes, slen-8);
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Displays an NPC dialog input box for numbers (ZC_OPEN_EDITDLG).
/// 0142 <npc id>.L
/// Client behavior (inputnum window):
/// - if npcid exists in the client:
///   - open the inputnum window
///   - set npcid of inputnum window
/// When 'ok' is pressed:
/// - if inputnum window has text:
///   - if npcid exists in the client:
///     - 0143 <npcid of inputnum window>.L <atoi(text)>.L
///   - close inputnum window
static void clif_scriptinput(struct map_session_data *sd, int npcid)
{
	int fd;
	struct block_list *bl = NULL;

	nullpo_retv(sd);

	if (!sd->state.using_fake_npc && (npcid == npc->fake_nd->bl.id || ((bl = map->id2bl(npcid)) != NULL && (bl->m!=sd->bl.m ||
						bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
						bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1))))
		clif->sendfakenpc(sd, npcid);

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0x142));
	WFIFOW(fd,0)=0x142;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len(0x142));
}

/// Displays an NPC dialog input box for numbers (ZC_OPEN_EDITDLGSTR).
/// 01d4 <npc id>.L
/// Client behavior (inputstr window):
/// - if npcid is 0 or npcid exists in the client:
///   - open the inputstr window
///   - set npcid of inputstr window
/// When 'ok' is pressed:
/// - if inputstr window has text and isn't an insult(manner.txt):
///   - if npcid is 0 or npcid exists in the client:
///     - 01d5 <packetlen>.W <npcid of inputstr window>.L <text>.?B
///   - close inputstr window
static void clif_scriptinputstr(struct map_session_data *sd, int npcid)
{
	nullpo_retv(sd);

	struct block_list *bl = map->id2bl(npcid);
	int x1 = sd->bl.x - AREA_SIZE - 1;
	int x2 = sd->bl.x + AREA_SIZE + 1;
	int y1 = sd->bl.y - AREA_SIZE - 1;
	int y2 = sd->bl.y + AREA_SIZE + 1;
	bool out_of_sight = (bl != NULL && (bl->m != sd->bl.m || bl->x < x1 || bl->x > x2 || bl->y < y1 || bl->y > y2));

	if (sd->state.using_fake_npc == 0 && sd->state.using_megaphone == 0
	    && (npcid == npc->fake_nd->bl.id || out_of_sight)) {
		clif->sendfakenpc(sd, npcid);
	}

	pc->update_idle_time(sd, BCIDLE_SCRIPT);

	WFIFOHEAD(sd->fd, packet_len(0x1d4));
	WFIFOW(sd->fd, 0) = 0x1d4;
	WFIFOL(sd->fd, 2) = (sd->state.using_megaphone == 0) ? npcid : 0;
	WFIFOSET(sd->fd, packet_len(0x1d4));
}

/// Marks a position on client's minimap (ZC_COMPASS).
/// 0144 <npc id>.L <type>.L <x>.L <y>.L <id>.B <color>.L
/// npc id:
///     is ignored in the client
/// type:
///     0 = display mark for 15 seconds
///     1 = display mark until dead or teleported
///     2 = remove mark
/// color:
///     0x00RRGGBB
static void clif_viewpoint(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0x144));
	WFIFOW(fd,0)=0x144;
	WFIFOL(fd,2)=npc_id;
	WFIFOL(fd,6)=type;
	WFIFOL(fd,10)=x;
	WFIFOL(fd,14)=y;
	WFIFOB(fd,18)=id;
	WFIFOL(fd,19)=color;
	WFIFOSET(fd,packet_len(0x144));
}

/// Displays an illustration image.
/// 0145 <image name>.16B <type>.B (ZC_SHOW_IMAGE)
/// 01b3 <image name>.64B <type>.B (ZC_SHOW_IMAGE2)
/// type:
///     0 = bottom left corner
///     1 = bottom middle
///     2 = bottom right corner
///     3 = middle of screen, inside a movable window
///     4 = middle of screen, movable with a close button, chrome-less
///   255 = hide
static void clif_cutin(struct map_session_data *sd, const char *image, int type)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0x1b3));
	WFIFOW(fd,0)=0x1b3;
	strncpy(WFIFOP(fd,2),image,64);
	WFIFOB(fd,66)=type;
	WFIFOSET(fd,packet_len(0x1b3));
}

/*==========================================
 * Fills in card data from the given item and into the buffer. [Skotlex]
 *------------------------------------------*/
static void clif_addcards(struct EQUIPSLOTINFO *buf, struct item *item)
{
	int i = 0, j;
	nullpo_retv(buf);
	if (item == NULL) {  //Blank data
		buf->card[0] = 0;
		buf->card[1] = 0;
		buf->card[2] = 0;
		buf->card[3] = 0;
		return;
	}
	if (item->card[0] == CARD0_PET) { //pet eggs
		buf->card[0] = 0;
		buf->card[1] = 0;
		buf->card[2] = (item->card[3] >> 1); // Pet intimacy level.
		buf->card[3] = (item->card[3] & 1); // Pet renamed flag.
		return;
	}
	if (item->card[0] == CARD0_FORGE || item->card[0] == CARD0_CREATE) { //Forged/created items
		buf->card[0] = item->card[0];
		buf->card[1] = item->card[1];
		buf->card[2] = item->card[2];
		buf->card[3] = item->card[3];
		return;
	}
	//Client only receives four cards.. so randomly send them a set of cards. [Skotlex]
	if (MAX_SLOTS > 4 && (j = itemdb_slot(item->nameid)) > 4)
		i = rnd() % (j - 3); //eg: 6 slots, possible i values: 0->3, 1->4, 2->5 => i = rnd()%3;

	//Normal items.
	if (item->card[i] > 0 && (j = itemdb_viewid(item->card[i])) > 0)
		buf->card[0] = j;
	else
		buf->card[0] = item->card[i];

	if (item->card[++i] > 0 && (j = itemdb_viewid(item->card[i])) > 0)
		buf->card[1] = j;
	else
		buf->card[1] = item->card[i];

	if (item->card[++i] > 0 && (j = itemdb_viewid(item->card[i])) > 0)
		buf->card[2] = j;
	else
		buf->card[2] = item->card[i];

	if (item->card[++i] > 0 && (j = itemdb_viewid(item->card[i])) > 0)
		buf->card[3] = j;
	else
		buf->card[3] = item->card[i];
}

/**
 * Fills in ItemOptions(Bonuses) of items into the buffer
 *
 * A maximum of 5 item options can be supported.
 *
 * @param buf[in,out] The buffer to write to. The pointer must be valid and initialized.
 * @param item[in]    The source item.
 */
static int clif_add_item_options(struct ItemOptions *buf, const struct item *it)
{
	int i = 0, j = 0, total_options = 0;

	nullpo_ret(buf);

	// Append the buffer with existing options first.
	for (i = 0; i < MAX_ITEM_OPTIONS; i++) {
		if (it->option[i].index) {
			WBUFW(buf, j * 5 + 0) = it->option[i].index; // OptIndex
			WBUFW(buf, j * 5 + 2) = it->option[i].value; // Value
			WBUFB(buf, j * 5 + 4) = it->option[i].param; // Param1
			total_options++;
			j++;
		}
	}
	// Append the remaining buffer with no values;
	for (; j < MAX_ITEM_OPTIONS || j < 5; j++) {
		WBUFW(buf, j * 5 + 0) = 0;
		WBUFW(buf, j * 5 + 2) = 0;
		WBUFB(buf, j * 5 + 4) = 0;
	}

	return total_options;
}

/// Notifies the client, about a received inventory item or the result of a pick-up request.
/// 00a0 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B (ZC_ITEM_PICKUP_ACK)
/// 029a <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B <expire time>.L (ZC_ITEM_PICKUP_ACK2)
/// 02d4 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W (ZC_ITEM_PICKUP_ACK3)
/// 0990 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W (ZC_ITEM_PICKUP_ACK_V5)
/// 0a0c <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W (ZC_ITEM_PICKUP_ACK_V6)
/// 0a37 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W <favorite>.B <view id>.W (ZC_ITEM_PICKUP_ACK_V7)
static void clif_additem(struct map_session_data *sd, int n, int amount, int fail)
{
	struct packet_additem p;
	nullpo_retv(sd);

	if (!sockt->session_is_active(sd->fd))  //Sasuke-
		return;

	if( fail )
		memset(&p, 0, sizeof(p));

	p.PacketType = additemType;
	p.Index = n+2;
	p.count = amount;

	if( !fail ) {
		if (n < 0 || n >= sd->status.inventorySize || sd->status.inventory[n].nameid <= 0 || sd->inventory_data[n] == NULL)
			return;

		if (sd->inventory_data[n]->view_id > 0)
			p.nameid = sd->inventory_data[n]->view_id;
		else
			p.nameid = sd->status.inventory[n].nameid;

		p.IsIdentified = sd->status.inventory[n].identify ? 1 : 0;
		p.IsDamaged = (sd->status.inventory[n].attribute & ATTR_BROKEN) != 0 ? 1 : 0;
		p.refiningLevel =sd->status.inventory[n].refine;
		clif->addcards(&p.slot, &sd->status.inventory[n]);
		p.location = pc->equippoint(sd,n);
		p.type = itemtype(sd->inventory_data[n]->type);
#if PACKETVER >= 20061218
		p.HireExpireDate = sd->status.inventory[n].expire_time;
#endif
#if PACKETVER >= 20071002
		/* why restrict the flag to non-stackable? because this is the only packet allows stackable to,
		 * show the color, and therefore it'd be inconsistent with the rest (aka it'd show yellow, you relog/refresh and boom its gone)
		 */
		p.bindOnEquipType = sd->status.inventory[n].bound && !itemdb->isstackable2(sd->inventory_data[n]) ? 2 : sd->inventory_data[n]->flag.bindonequip ? 1 : 0;
#endif
#if PACKETVER >= 20150226
		clif->add_item_options(&p.option_data[0], &sd->status.inventory[n]);
#endif
#if PACKETVER >= 20160921
		p.favorite = sd->status.inventory[n].favorite;
		p.look = sd->inventory_data[n]->view_sprite;
#endif
	}
	p.result = (unsigned char)fail;

	clif->send(&p,sizeof(p),&sd->bl,SELF);
}

/// Notifies the client, that an inventory item was deleted or dropped (ZC_ITEM_THROW_ACK).
/// 00af <index>.W <amount>.W
static void clif_dropitem(struct map_session_data *sd, int n, int amount)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0xaf));
	WFIFOW(fd,0)=0xaf;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=amount;
	WFIFOSET(fd,packet_len(0xaf));
}

static void clif_item_movefailed(struct map_session_data *sd, int n)
{
#if PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)
	int fd = sd->fd;
	const int len = sizeof(struct PACKET_ZC_INVENTORY_MOVE_FAILED);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_INVENTORY_MOVE_FAILED *p = WFIFOP(fd, 0);
	p->packetType = 0xaa7;
	p->index = n + 2;
	p->unknown = 1;
	WFIFOSET(fd, len);
#else
	clif->dropitem(sd, n, 0);
#endif
}

/// Notifies the client, that an inventory item was deleted (ZC_DELETE_ITEM_FROM_BODY).
/// 07fa <delete type>.W <index>.W <amount>.W
/// delete type: @see enum delitem_reason
static void clif_delitem(struct map_session_data *sd, int n, int amount, short reason)
{
#if PACKETVER < 20091117
	clif->dropitem(sd,n,amount);
#else
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;

	WFIFOHEAD(fd, packet_len(0x7fa));
	WFIFOW(fd,0)=0x7fa;
	WFIFOW(fd,2)=reason;
	WFIFOW(fd,4)=n+2;
	WFIFOW(fd,6)=amount;
	WFIFOSET(fd,packet_len(0x7fa));
#endif
}

// Simplifies inventory/cart/storage packets by handling the packet section relevant to items. [Skotlex]
// Equip is >= 0 for equippable items (holds the equip-point, is 0 for pet
// armor/egg) -1 for stackable items, -2 for stackable items where arrows must send in the equip-point.
// look like unused, not adding checks
static void clif_item_sub(unsigned char *buf, int n, struct item *i, struct item_data *id, int equip)
{
	if (id->view_id > 0)
		WBUFW(buf,n)=id->view_id;
	else
		WBUFW(buf,n)=i->nameid;
	WBUFB(buf,n+2)=itemtype(id->type);
	WBUFB(buf,n+3)=i->identify;
	if (equip >= 0) { //Equippable item
		WBUFW(buf,n+4)=equip;
		WBUFW(buf,n+6)=i->equip;
		WBUFB(buf,n+8)=i->attribute;
		WBUFB(buf,n+9)=i->refine;
	} else { //Stackable item.
		WBUFW(buf,n+4)=i->amount;
		if (equip == -2 && id->equip == EQP_AMMO)
			WBUFW(buf,n+6)=EQP_AMMO;
		else
			WBUFW(buf,n+6)=0;
	}

}

static void clif_item_equip(short idx, struct EQUIPITEM_INFO *p, struct item *it, struct item_data *id, int eqp_pos)
{
	nullpo_retv(p);
	nullpo_retv(it);
	nullpo_retv(id);
	p->index = idx;

	if (id->view_id > 0)
		p->ITID = id->view_id;
	else
		p->ITID = it->nameid;

	p->type = itemtype(id->type);

#if PACKETVER < 20120925
	p->IsIdentified = it->identify ? 1 : 0;
#endif

	p->location = eqp_pos;
	p->WearState = it->equip;
#if PACKETVER < 20120925
	p->IsDamaged = (it->attribute & ATTR_BROKEN) != 0 ? 1 : 0;
#endif
	p->RefiningLevel = it->refine;

	clif->addcards(&p->slot, it);

#if PACKETVER >= 20071002
	p->HireExpireDate = it->expire_time;
#endif

#if PACKETVER >= 20080102
	p->bindOnEquipType = it->bound ? 2 : id->flag.bindonequip ? 1 : 0;
#endif

#if PACKETVER >= 20100629
	p->wItemSpriteNumber = (id->equip&EQP_VISIBLE) ? id->view_sprite : 0;
#endif

#if PACKETVER >= 20120925
	p->Flag.IsIdentified = it->identify ? 1 : 0;
	p->Flag.IsDamaged    = (it->attribute & ATTR_BROKEN) != 0 ? 1 : 0;
	p->Flag.PlaceETCTab  = it->favorite ? 1 : 0;
	p->Flag.SpareBits    = 0;
#endif

#if PACKETVER >= 20150226
	p->option_count = clif->add_item_options(p->option_data, it);
#endif
}

static void clif_item_normal(short idx, struct NORMALITEM_INFO *p, struct item *i, struct item_data *id)
{
	nullpo_retv(p);
	nullpo_retv(i);
	nullpo_retv(id);

	p->index = idx;

	if (id->view_id > 0)
		p->ITID = id->view_id;
	else
		p->ITID = i->nameid;

	p->type = itemtype(id->type);

#if PACKETVER < 20120925
	p->IsIdentified = i->identify ? 1 : 0;
#endif

	p->count = i->amount;
	p->WearState = id->equip;

#if PACKETVER >= 5
	clif->addcards(&p->slot, i);
#endif

#if PACKETVER >= 20080102
	p->HireExpireDate = i->expire_time;
#endif

#if PACKETVER >= 20120925
	p->Flag.IsIdentified = i->identify ? 1 : 0;
	p->Flag.PlaceETCTab  = i->favorite ? 1 : 0;
	p->Flag.SpareBits    = 0;
#endif
}

static void clif_inventoryList(struct map_session_data *sd)
{
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif->inventoryStart(sd, INVTYPE_INVENTORY, "");
#endif
	clif->inventoryItems(sd, INVTYPE_INVENTORY);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif->inventoryEnd(sd, INVTYPE_INVENTORY);
#endif
}

static void clif_inventoryItems(struct map_session_data *sd, enum inventory_type type)
{
	int i, normal = 0, equip = 0;

	nullpo_retv(sd);
	for (i = 0; i < sd->status.inventorySize; i++) {

		if( sd->status.inventory[i].nameid <= 0 || sd->inventory_data[i] == NULL )
			continue;
		if( !itemdb->isstackable2(sd->inventory_data[i]) ) //Non-stackable (Equippable)
			clif->item_equip(i+2,&itemlist_equip.list[equip++],&sd->status.inventory[i],sd->inventory_data[i],pc->equippoint(sd,i));
		else //Stackable (Normal)
			clif->item_normal(i+2,&itemlist_normal.list[normal++],&sd->status.inventory[i],sd->inventory_data[i]);
	}

	if (normal) {
		itemlist_normal.PacketType = inventorylistnormalType;
		itemlist_normal.PacketLength = (sizeof(itemlist_normal) - sizeof(itemlist_normal.list)) + (sizeof(struct NORMALITEM_INFO) * normal);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_normal.invType = type;
#endif

		clif->send(&itemlist_normal, itemlist_normal.PacketLength, &sd->bl, SELF);
	}

	if( sd->equip_index[EQI_AMMO] >= 0 )
		clif->arrowequip(sd,sd->equip_index[EQI_AMMO]);

	if( equip ) {
		itemlist_equip.PacketType  = inventorylistequipType;
		itemlist_equip.PacketLength = (sizeof(itemlist_equip) - sizeof(itemlist_equip.list)) + (sizeof(struct EQUIPITEM_INFO) * equip);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_equip.invType = type;
#endif

		clif->send(&itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF);
	}
/* on 20120925 onwards this is a field on clif_item_equip/normal */
#if PACKETVER >= 20111122 && PACKETVER < 20120925
	for (i = 0; i < sd->status.inventorySize; i++) {
		if( sd->status.inventory[i].nameid <= 0 || sd->inventory_data[i] == NULL )
			continue;

		if ( sd->status.inventory[i].favorite )
			clif->favorite_item(sd, i);
	}
#endif
}

static void clif_equipList(struct map_session_data *sd)
{
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif->inventoryStart(sd, INVTYPE_INVENTORY, "");
	clif->inventoryItems(sd, INVTYPE_INVENTORY);
	clif->inventoryEnd(sd, INVTYPE_INVENTORY);
#else
	// [4144] for old packet version it send only equipment. this is bug?
	clif->equipItems(sd, INVTYPE_INVENTORY);
#endif
}

//Required when items break/get-repaired. Only sends equippable item list.
static void clif_equipItems(struct map_session_data *sd, enum inventory_type type)
{
	int i, equip = 0;

	nullpo_retv(sd);
	for (i = 0; i < sd->status.inventorySize; i++) {

		if( sd->status.inventory[i].nameid <= 0 || sd->inventory_data[i] == NULL )
			continue;
		if( !itemdb->isstackable2(sd->inventory_data[i]) ) //Non-stackable (Equippable)
			clif->item_equip(i+2,&itemlist_equip.list[equip++],&sd->status.inventory[i],sd->inventory_data[i],pc->equippoint(sd,i));
	}

	if (equip) {
		itemlist_equip.PacketType = inventorylistequipType;
		itemlist_equip.PacketLength = (sizeof(itemlist_equip) - sizeof(itemlist_equip.list)) + (sizeof(struct EQUIPITEM_INFO) * equip);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_equip.invType = type;
#endif

		clif->send(&itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF);
	}

	/* on 20120925 onwards this is a field on clif_item_equip */
#if PACKETVER >= 20111122 && PACKETVER < 20120925
	for (i = 0; i < sd->status.inventorySize; i++) {
		if( sd->status.inventory[i].nameid <= 0 || sd->inventory_data[i] == NULL )
			continue;

		if ( sd->status.inventory[i].favorite )
			clif->favorite_item(sd, i);
	}
#endif
}

static void clif_storageList(struct map_session_data *sd, struct item *items, int items_length)
{
	nullpo_retv(sd);

	clif->inventoryStart(sd, INVTYPE_STORAGE, "Storage");
	if (sd->storage.aggregate > 0)
		clif->storageItems(sd, INVTYPE_STORAGE, items, items_length);
	clif->inventoryEnd(sd, INVTYPE_STORAGE);
}

static void clif_guildStorageList(struct map_session_data *sd, struct item *items, int items_length)
{
	clif->inventoryStart(sd, INVTYPE_GUILD_STORAGE, "Guild storage");
	clif->storageItems(sd, INVTYPE_GUILD_STORAGE, items, items_length);
	clif->inventoryEnd(sd, INVTYPE_GUILD_STORAGE);
}

static void clif_inventoryStart(struct map_session_data *sd, enum inventory_type type, const char *name)
{
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	nullpo_retv(sd);
	nullpo_retv(name);

	char buf[sizeof(struct PACKET_ZC_INVENTORY_START) + 24];
	memset(buf, 0, sizeof(buf));
	struct PACKET_ZC_INVENTORY_START *p = (struct PACKET_ZC_INVENTORY_START *)buf;
	p->packetType = HEADER_ZC_INVENTORY_START;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	p->invType = type;
#endif
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	int strLen = (int)safestrnlen(name, 24) + 1;
	if (strLen > 24)
		strLen = 24;
	const int len = sizeof(struct PACKET_ZC_INVENTORY_START) + strLen;
	p->packetLength = len;
	safestrncpy(p->name, name, strLen);
#else
	const int len = sizeof(struct PACKET_ZC_INVENTORY_START);
	safestrncpy(p->name, name, NAME_LENGTH);
#endif
	clif->send(p, len, &sd->bl, SELF);
#endif
}

static void clif_inventoryEnd(struct map_session_data *sd, enum inventory_type type)
{
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	nullpo_retv(sd);

	struct PACKET_ZC_INVENTORY_END p;
	p.packetType = HEADER_ZC_INVENTORY_END;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	p.invType = type;
#endif
	p.flag = 0;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_storageItems(struct map_session_data *sd, enum inventory_type type, struct item *items, int items_length)
{
	nullpo_retv(sd);
	nullpo_retv(items);

	int normal_count = 0, equip_count = 0;
	for (int i = 0; i < items_length; ++i) {
		if (items[i].nameid == 0)
			continue;

		struct item_data *itd = itemdb->search(items[i].nameid);

		if (!itemdb->isstackable2(itd))
			clif->item_equip(i + 1, &storelist_equip.list[equip_count++], &items[i], itd, itd->equip);
		else
			clif->item_normal(i + 1, &storelist_normal.list[normal_count++], &items[i], itd);

		if (normal_count > 0 && (normal_count == MAX_STORAGE_ITEM_PACKET_NORMAL || i + 1 == items_length)) {
			storelist_normal.PacketType = storageListNormalType;
			storelist_normal.PacketLength = (sizeof(storelist_normal) - sizeof(storelist_normal.list)) + (sizeof(struct NORMALITEM_INFO) * normal_count);

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
			storelist_normal.invType = type;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
			safestrncpy(storelist_normal.name, "Storage", NAME_LENGTH);
#endif

			clif->send(&storelist_normal, storelist_normal.PacketLength, &sd->bl, SELF);
			normal_count = 0;
		}

		if (equip_count > 0 && (equip_count == MAX_STORAGE_ITEM_PACKET_EQUIP || i + 1 == items_length)) {
			storelist_equip.PacketType = storageListEquipType;
			storelist_equip.PacketLength = (sizeof(storelist_equip) - sizeof(storelist_equip.list)) + (sizeof(struct EQUIPITEM_INFO) * equip_count);

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
			storelist_equip.invType = type;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
			safestrncpy(storelist_equip.name, "Storage", NAME_LENGTH);
#endif

			clif->send(&storelist_equip, storelist_equip.PacketLength, &sd->bl, SELF);
			equip_count = 0;
		}
	}

	if (normal_count > 0) {
		storelist_normal.PacketType = storageListNormalType;
		storelist_normal.PacketLength = (sizeof(storelist_normal) - sizeof(storelist_normal.list)) + (sizeof(struct NORMALITEM_INFO) * normal_count);

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		storelist_normal.invType = type;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
		safestrncpy(storelist_normal.name, "Storage", NAME_LENGTH);
#endif

		clif->send(&storelist_normal, storelist_normal.PacketLength, &sd->bl, SELF);
		normal_count = 0;
	}

	if (equip_count > 0) {
		storelist_equip.PacketType = storageListEquipType;
		storelist_equip.PacketLength = (sizeof(storelist_equip) - sizeof(storelist_equip.list)) + (sizeof(struct EQUIPITEM_INFO) * equip_count);

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		storelist_equip.invType = type;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
		safestrncpy(storelist_equip.name, "Storage", NAME_LENGTH);
#endif

		clif->send(&storelist_equip, storelist_equip.PacketLength, &sd->bl, SELF);
		equip_count = 0;
	}
}

static void clif_cartList(struct map_session_data *sd)
{
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif->inventoryStart(sd, INVTYPE_CART, "");
#endif
	clif->cartItems(sd, INVTYPE_CART);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif->inventoryEnd(sd, INVTYPE_CART);
#endif
}

static void clif_cartItems(struct map_session_data *sd, enum inventory_type type)
{
	int i, normal = 0, equip = 0;
	struct item_data *id;

	nullpo_retv(sd);
	for( i = 0; i < MAX_CART; i++ ) {

		if( sd->status.cart[i].nameid <= 0 )
			continue;

		id = itemdb->search(sd->status.cart[i].nameid);
		if( !itemdb->isstackable2(id) ) //Non-stackable (Equippable)
			clif->item_equip(i+2,&itemlist_equip.list[equip++],&sd->status.cart[i],id,id->equip);
		else //Stackable (Normal)
			clif->item_normal(i+2,&itemlist_normal.list[normal++],&sd->status.cart[i],id);
	}

	if (normal) {
		itemlist_normal.PacketType = cartlistnormalType;
		itemlist_normal.PacketLength = (sizeof(itemlist_normal) - sizeof(itemlist_normal.list)) + (sizeof(struct NORMALITEM_INFO) * normal);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_normal.invType = type;
#endif

		clif->send(&itemlist_normal, itemlist_normal.PacketLength, &sd->bl, SELF);
	}

	if (equip) {
		itemlist_equip.PacketType = cartlistequipType;
		itemlist_equip.PacketLength = (sizeof(itemlist_equip) - sizeof(itemlist_equip.list)) + (sizeof(struct EQUIPITEM_INFO) * equip);
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_equip.invType = type;
#endif

		clif->send(&itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF);
	}
}

static void clif_inventoryExpansionInfo(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv(sd);

	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_INVENTORY_EXPANSION_INFO));
	struct PACKET_ZC_INVENTORY_EXPANSION_INFO *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_INVENTORY_EXPANSION_INFO;
	p->expansionSize = sd->status.inventorySize - FIXED_INVENTORY_SIZE;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_INVENTORY_EXPANSION_INFO));
#endif
}

static void clif_inventoryExpandAck(struct map_session_data *sd, enum expand_inventory result, int itemId)
{
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv(sd);

	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_ACK_INVENTORY_EXPAND));
	struct PACKET_ZC_ACK_INVENTORY_EXPAND *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_ACK_INVENTORY_EXPAND;
	p->result = result;
	p->itemId = itemId;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_ACK_INVENTORY_EXPAND));
#endif
}

static void clif_inventoryExpandResult(struct map_session_data *sd, enum expand_inventory_result result)
{
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv(sd);

	const int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_ACK_INVENTORY_EXPAND_RESULT));
	struct PACKET_ZC_ACK_INVENTORY_EXPAND_RESULT *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_ACK_INVENTORY_EXPAND_RESULT;
	p->result = result;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_ACK_INVENTORY_EXPAND_RESULT));
#endif
}

static void clif_parse_inventoryExpansion(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_inventoryExpansion(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
	if (pc_isdead(sd) || pc_cant_act(sd)) {
		clif->inventoryExpandAck(sd, EXPAND_INVENTORY_OTHER_WORK, 0);
		return;
	}
	if (sd->status.inventorySize == MAX_INVENTORY) {
		clif->inventoryExpandAck(sd, EXPAND_INVENTORY_MAX_SIZE, 0);
		return;
	}

	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;

	safestrncpy(evname, "inventory_expansion::OnInvExpandRequest", EVENT_NAME_LENGTH);
	if ((ev = strdb_get(npc->ev_db, evname))) {
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else {
		ShowError("clif_parse_inventoryExpansion: event '%s' not found, operation failed.\n", evname);
	}
#endif
}

static void clif_parse_inventoryExpansionConfirmed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_inventoryExpansionConfirmed(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
	if (pc_isdead(sd) || pc_cant_act(sd)) {
		clif->inventoryExpandResult(sd, EXPAND_INVENTORY_RESULT_OTHER_WORK);
		return;
	}
	if (sd->status.inventorySize == MAX_INVENTORY) {
		clif->inventoryExpandResult(sd, EXPAND_INVENTORY_RESULT_MAX_SIZE);
		return;
	}

	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;

	safestrncpy(evname, "inventory_expansion::OnInvExpandConfirmed", EVENT_NAME_LENGTH);
	if ((ev = strdb_get(npc->ev_db, evname))) {
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else {
		ShowError("clif_parse_inventoryExpansionConfirmed: event '%s' not found, operation failed.\n", evname);
	}
#endif
}

static void clif_parse_inventoryExpansionRejected(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_inventoryExpansionRejected(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;

	safestrncpy(evname, "inventory_expansion::OnInvExpandRejected", EVENT_NAME_LENGTH);
	if ((ev = strdb_get(npc->ev_db, evname))) {
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else {
		ShowError("clif_parse_inventoryExpansionRejected: event '%s' not found, operation failed.\n", evname);
	}
#endif
}

// CZ_REQ_REMAINTIME
static void clif_parse_reqRemainTime(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_reqRemainTime(int fd, struct map_session_data *sd)
{
}

/// Removes cart (ZC_CARTOFF).
/// 012b
/// Client behavior:
/// Closes the cart storage and removes all it's items from memory.
/// The Num & Weight values of the cart are left untouched and the cart is NOT removed.
static void clif_clearcart(int fd)
{
	WFIFOHEAD(fd, packet_len(0x12b));
	WFIFOW(fd,0) = 0x12b;
	WFIFOSET(fd, packet_len(0x12b));

}

/// Guild XY locators (ZC_NOTIFY_POSITION_TO_GUILDM) [Valaris]
/// 01eb <account id>.L <x>.W <y>.W
static void clif_guild_xy(struct map_session_data *sd)
{
	unsigned char buf[10];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=sd->bl.x;
	WBUFW(buf,8)=sd->bl.y;
	clif->send(buf,packet_len(0x1eb),&sd->bl,GUILD_SAMEMAP_WOS);
}

/*==========================================
 * Sends x/y dot to a single fd. [Skotlex]
 *------------------------------------------*/
static void clif_guild_xy_single(int fd, struct map_session_data *sd)
{
	if( sd->bg_id )
		return;

	nullpo_retv(sd);
	WFIFOHEAD(fd,packet_len(0x1eb));
	WFIFOW(fd,0)=0x1eb;
	WFIFOL(fd,2)=sd->status.account_id;
	WFIFOW(fd,6)=sd->bl.x;
	WFIFOW(fd,8)=sd->bl.y;
	WFIFOSET(fd,packet_len(0x1eb));
}

// Guild XY locators [Valaris]
static void clif_guild_xy_remove(struct map_session_data *sd)
{
	unsigned char buf[10];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=-1;
	WBUFW(buf,8)=-1;
	clif->send(buf,packet_len(0x1eb),&sd->bl,GUILD_SAMEMAP_WOS);
}

/*==========================================
 *
 *------------------------------------------*/
static int clif_hpmeter_sub(struct block_list *bl, va_list ap)
{
#if PACKETVER < 20100126
	const int cmd = 0x106;
#else
	const int cmd = 0x80e;
#endif
	struct map_session_data *sd = va_arg(ap, struct map_session_data *);
	struct map_session_data *tsd = BL_CAST(BL_PC, bl);

	nullpo_ret(sd);
	nullpo_ret(tsd);

	if( !tsd->fd || tsd == sd )
		return 0;

	if( !pc_has_permission(tsd, PC_PERM_VIEW_HPMETER) )
		return 0;
	WFIFOHEAD(tsd->fd,packet_len(cmd));
	WFIFOW(tsd->fd,0) = cmd;
	WFIFOL(tsd->fd,2) = sd->status.account_id;
#if PACKETVER < 20100126
	if( sd->battle_status.max_hp > INT16_MAX )
	{ //To correctly display the %hp bar. [Skotlex]
		WFIFOW(tsd->fd,6) = sd->battle_status.hp/(sd->battle_status.max_hp/100);
		WFIFOW(tsd->fd,8) = 100;
	} else {
		WFIFOW(tsd->fd,6) = sd->battle_status.hp;
		WFIFOW(tsd->fd,8) = sd->battle_status.max_hp;
	}
#else
	WFIFOL(tsd->fd,6) = sd->battle_status.hp;
	WFIFOL(tsd->fd,10) = sd->battle_status.max_hp;
#endif
	WFIFOSET(tsd->fd,packet_len(cmd));
	return 0;
}

/*==========================================
 * Server tells all players that are allowed to view HP bars
 * and are nearby 'sd' that 'sd' hp bar was updated.
 *------------------------------------------*/
static int clif_hpmeter(struct map_session_data *sd)
{
	nullpo_ret(sd);
	map->foreachinarea(clif->hpmeter_sub, sd->bl.m, sd->bl.x-AREA_SIZE, sd->bl.y-AREA_SIZE, sd->bl.x+AREA_SIZE, sd->bl.y+AREA_SIZE, BL_PC, sd);
	return 0;
}

/// Notifies client of a character parameter change.
/// 00b0 <var id>.W <value>.L (ZC_PAR_CHANGE)
/// 00b1 <var id>.W <value>.L (ZC_LONGPAR_CHANGE)
/// 00be <status id>.W <value>.B (ZC_STATUS_CHANGE)
/// 0121 <current count>.W <max count>.W <current weight>.L <max weight>.L (ZC_NOTIFY_CARTITEM_COUNTINFO)
/// 013a <atk range>.W (ZC_ATTACK_RANGE)
/// 0141 <status id>.L <base status>.L <plus status>.L (ZC_COUPLESTATUS)
/// TODO: Extract individual packets.
static void clif_updatestatus(struct map_session_data *sd, int type)
{
	int fd,len;

	nullpo_retv(sd);

	fd=sd->fd;

	if (!sockt->session_is_active(fd)) // Invalid pointer fix, by sasuke [Kevin]
		return;

	WFIFOHEAD(fd, 14);
	WFIFOW(fd,0)=0xb0;
	WFIFOW(fd,2)=type;
	len = packet_len(0xb0);
	switch(type){
			// 00b0
		case SP_WEIGHT:
			pc->updateweightstatus(sd);
			WFIFOHEAD(fd,14);
			WFIFOW(fd,0)=0xb0; //Need to re-set as pc->updateweightstatus can alter the buffer. [Skotlex]
			WFIFOW(fd,2)=type;
			WFIFOL(fd,4)=sd->weight;
			break;
		case SP_MAXWEIGHT:
			WFIFOL(fd,4)=sd->max_weight;
			break;
		case SP_SPEED:
			WFIFOL(fd,4)=sd->battle_status.speed;
			break;
		case SP_BASELEVEL:
			WFIFOL(fd,4)=sd->status.base_level;
			break;
		case SP_JOBLEVEL:
			WFIFOL(fd,4)=sd->status.job_level;
			break;
		case SP_KARMA: // Adding this back, I wonder if the client intercepts this - [Lance]
			WFIFOL(fd,4)=sd->status.karma;
			break;
		case SP_MANNER:
			WFIFOL(fd,4)=sd->status.manner;
			break;
		case SP_STATUSPOINT:
			WFIFOL(fd,4)=sd->status.status_point;
			break;
		case SP_SKILLPOINT:
			WFIFOL(fd,4)=sd->status.skill_point;
			break;
		case SP_HIT:
			WFIFOL(fd,4)=sd->battle_status.hit;
			break;
		case SP_FLEE1:
			WFIFOL(fd,4)=sd->battle_status.flee;
			break;
		case SP_FLEE2:
			WFIFOL(fd,4)=sd->battle_status.flee2/10;
			break;
		case SP_MAXHP:
			WFIFOL(fd,4)=sd->battle_status.max_hp;
			break;
		case SP_MAXSP:
			WFIFOL(fd,4)=sd->battle_status.max_sp;
			break;
		case SP_HP:
			if (sd->battle_status.hp == 0 && battle_config.display_fake_hp_when_dead) {
				// On official servers, the HP displayed when dead is the HP that the character will have at respawn.
				WFIFOL(fd, 4) = status->get_restart_hp(sd, &sd->battle_status);
			} else {
				WFIFOL(fd, 4) = sd->battle_status.hp;
			}
			break;
		case SP_SP:
			WFIFOL(fd,4)=sd->battle_status.sp;
			break;
		case SP_ASPD:
			WFIFOL(fd,4)=sd->battle_status.amotion;
			break;
		case SP_ATK1:
			WFIFOL(fd,4)=pc_leftside_atk(sd);
			break;
		case SP_DEF1:
			WFIFOL(fd,4)=pc_leftside_def(sd);
			break;
		case SP_MDEF1:
			WFIFOL(fd,4)=pc_leftside_mdef(sd);
			break;
		case SP_ATK2:
			WFIFOL(fd,4)=pc_rightside_atk(sd);
			break;
		case SP_DEF2:
			WFIFOL(fd,4)=pc_rightside_def(sd);
			break;
		case SP_MDEF2: {
				//negative check (in case you have something like Berserk active)
				int mdef2 = pc_rightside_mdef(sd);

				WFIFOL(fd,4)=
	#ifndef RENEWAL
				( mdef2 < 0 ) ? 0 :
	#endif
				mdef2;

			}
			break;
		case SP_CRITICAL:
			WFIFOL(fd,4)=sd->battle_status.cri/10;
			break;
		case SP_MATK1:
			WFIFOL(fd,4)=pc_rightside_matk(sd);
			break;
		case SP_MATK2:
			WFIFOL(fd,4)=pc_leftside_matk(sd);
			break;
		case SP_ZENY:
// [4144] possible send 64 bit value from PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
// but kro sending 0xb1 packet only.
			WFIFOW(fd,0)=0xb1;
			WFIFOL(fd,4)=sd->status.zeny;
			len = packet_len(0xb1);
			break;
#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
		case SP_BASEEXP:
			WFIFOW(fd, 0) = 0xacb;
			WFIFOQ(fd, 4) = sd->status.base_exp;
			len = packet_len(0xacb);
			break;
		case SP_JOBEXP:
			WFIFOW(fd, 0) = 0xacb;
			WFIFOQ(fd, 4) = sd->status.job_exp;
			len = packet_len(0xacb);
			break;
		case SP_NEXTBASEEXP:
			WFIFOW(fd, 0) = 0xacb;
			WFIFOQ(fd, 4) = pc->nextbaseexp(sd);
			len = packet_len(0xacb);
			break;
		case SP_NEXTJOBEXP:
			WFIFOW(fd, 0) = 0xacb;
			WFIFOQ(fd, 4) = pc->nextjobexp(sd);
			len = packet_len(0xacb);
			break;
#else
		case SP_BASEEXP:
			WFIFOW(fd, 0) = 0xb1;
			WFIFOL(fd, 4) = (uint32)(sd->status.base_exp);
			len = packet_len(0xb1);
			break;
		case SP_JOBEXP:
			WFIFOW(fd, 0) = 0xb1;
			WFIFOL(fd, 4) = (uint32)(sd->status.job_exp);
			len = packet_len(0xb1);
			break;
		case SP_NEXTBASEEXP:
			WFIFOW(fd, 0) = 0xb1;
			WFIFOL(fd, 4) = (uint32)pc->nextbaseexp(sd);
			len = packet_len(0xb1);
			break;
		case SP_NEXTJOBEXP:
			WFIFOW(fd, 0) = 0xb1;
			WFIFOL(fd, 4) = (uint32)pc->nextjobexp(sd);
			len = packet_len(0xb1);
			break;
#endif
		/**
		 * SP_U<STAT> are used to update the amount of points necessary to increase that stat
		 **/
		case SP_USTR:
		case SP_UAGI:
		case SP_UVIT:
		case SP_UINT:
		case SP_UDEX:
		case SP_ULUK:
			WFIFOW(fd,0)=0xbe;
			WFIFOB(fd,4)=pc->need_status_point(sd,type-SP_USTR+SP_STR,1);
			len = packet_len(0xbe);
			break;

		/**
		 * Tells the client how far it is allowed to attack (weapon range)
		 **/
		case SP_ATTACKRANGE:
			WFIFOW(fd,0)=0x13a;
			WFIFOW(fd,2)=sd->battle_status.rhw.range;
			len = packet_len(0x13a);
			break;

		case SP_STR:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.str;
			WFIFOL(fd,10)=sd->battle_status.str - sd->status.str;
			len = packet_len(0x141);
			break;
		case SP_AGI:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.agi;
			WFIFOL(fd,10)=sd->battle_status.agi - sd->status.agi;
			len = packet_len(0x141);
			break;
		case SP_VIT:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.vit;
			WFIFOL(fd,10)=sd->battle_status.vit - sd->status.vit;
			len = packet_len(0x141);
			break;
		case SP_INT:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.int_;
			WFIFOL(fd,10)=sd->battle_status.int_ - sd->status.int_;
			len = packet_len(0x141);
			break;
		case SP_DEX:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.dex;
			WFIFOL(fd,10)=sd->battle_status.dex - sd->status.dex;
			len = packet_len(0x141);
			break;
		case SP_LUK:
			WFIFOW(fd,0)=0x141;
			WFIFOL(fd,2)=type;
			WFIFOL(fd,6)=sd->status.luk;
			WFIFOL(fd,10)=sd->battle_status.luk - sd->status.luk;
			len = packet_len(0x141);
			break;

		case SP_CARTINFO:
			WFIFOW(fd,0)=0x121;
			WFIFOW(fd,2)=sd->cart_num;
			WFIFOW(fd,4)=MAX_CART;
			WFIFOL(fd,6)=sd->cart_weight;
			WFIFOL(fd,10)=sd->cart_weight_max;
			len = packet_len(0x121);
			break;

		default:
			ShowError("clif->updatestatus : unrecognized type %d\n",type);
			return;
	}
	WFIFOSET(fd,len);

	// Additional update packets that should be sent right after
	switch (type) {
		case SP_BASELEVEL:
			pc->update_job_and_level(sd);
			break;
		case SP_HP:
			if (map->list[sd->bl.m].hpmeter_visible)
				clif->hpmeter(sd);
			if (!battle_config.party_hp_mode && sd->status.party_id)
				clif->party_hp(sd);
			if (sd->bg_id)
				clif->bg_hp(sd);
			break;
	}
}

/// Notifies client of a parameter change of an another player (ZC_PAR_CHANGE_USER).
/// 01ab <account id>.L <var id>.W <value>.L
static void clif_changestatus(struct map_session_data *sd, int type, int val)
{
	unsigned char buf[12];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x1ab;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=type;

	switch(type)
	{
		case SP_MANNER:
			WBUFL(buf,8)=val;
			break;
		default:
			ShowError("clif_changestatus : unrecognized type %d.\n",type);
			return;
	}

	clif->send(buf,packet_len(0x1ab),&sd->bl,AREA_WOS);
}

/// Updates sprite/style properties of an object.
static void clif_changelook(struct block_list *bl, int type, int val)
{
	struct map_session_data* sd;
	struct status_change* sc;
	struct view_data* vd;
	enum send_target target = AREA;
#if PACKETVER >= 4
	int val2 = 0;
#endif
	nullpo_retv(bl);

	sd = BL_CAST(BL_PC, bl);
	sc = status->get_sc(bl);
	vd = status->get_viewdata(bl);

	if( vd ) //temp hack to let Warp Portal change appearance
		switch(type) {
			case LOOK_WEAPON:
				if (sd) {
					clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
					val = vd->weapon;
				}
				else
					vd->weapon = val;
			break;
			case LOOK_SHIELD:
				if (sd) {
					clif->get_weapon_view(sd, &vd->weapon, &vd->shield);
					val = vd->shield;
				}
				else
					vd->shield = val;
			break;
			case LOOK_BASE:
				if (sd == NULL)
					break;

				if (val == INVISIBLE_CLASS) /* nothing to change look */
					return;

				if (sd->sc.option & OPTION_COSTUME)
					vd->weapon = vd->shield = 0;

				if (!vd->cloth_color)
					break;

				if ((sd->sc.option & OPTION_WEDDING) != 0 && battle_config.wedding_ignorepalette == true)
					vd->cloth_color = 0;
				if ((sd->sc.option & OPTION_XMAS) != 0 && battle_config.xmas_ignorepalette == true)
					vd->cloth_color = 0;
				if ((sd->sc.option & OPTION_SUMMER) != 0 && battle_config.summer_ignorepalette == true)
					vd->cloth_color = 0;
				if ((sd->sc.option & OPTION_HANBOK) != 0 && battle_config.hanbok_ignorepalette == true)
					vd->cloth_color = 0;
				if ((sd->sc.option & OPTION_OKTOBERFEST) != 0 && battle_config.oktoberfest_ignorepalette == true)
					vd->cloth_color = 0;
				if ((sd->sc.option & OPTION_SUMMER2) != 0 && battle_config.summer2_ignorepalette == true)
					vd->cloth_color = 0;
				if (vd->body_style != 0 && (sd->sc.option & OPTION_COSTUME) != 0)
					vd->body_style = 0;
			break;
			case LOOK_HAIR:
				vd->hair_style = val;
			break;
			case LOOK_HEAD_BOTTOM:
				vd->head_bottom = val;
			break;
			case LOOK_HEAD_TOP:
				vd->head_top = val;
			break;
			case LOOK_HEAD_MID:
				vd->head_mid = val;
			break;
			case LOOK_HAIR_COLOR:
				vd->hair_color = val;
			break;
			case LOOK_CLOTHES_COLOR:
				if (val && sd != NULL) {
					if ((sd->sc.option & OPTION_WEDDING) != 0 && battle_config.wedding_ignorepalette == true)
						val = 0;
					if ((sd->sc.option & OPTION_XMAS) != 0 && battle_config.xmas_ignorepalette == true)
						val = 0;
					if ((sd->sc.option & OPTION_SUMMER) != 0 && battle_config.summer_ignorepalette == true)
						val = 0;
					if ((sd->sc.option & OPTION_HANBOK) != 0 && battle_config.hanbok_ignorepalette == true)
						val = 0;
					if ((sd->sc.option & OPTION_OKTOBERFEST) != 0 && battle_config.oktoberfest_ignorepalette == true)
						val = 0;
					if ((sd->sc.option & OPTION_SUMMER2) != 0 && battle_config.summer2_ignorepalette == true)
						val = 0;
				}
				vd->cloth_color = val;
			break;
			case LOOK_SHOES:
		#if PACKETVER > 3
				if (sd) {
					int n;
					if((n = sd->equip_index[2]) >= 0 && sd->inventory_data[n]) {
						if(sd->inventory_data[n]->view_id > 0)
							val = sd->inventory_data[n]->view_id;
						else
							val = sd->status.inventory[n].nameid;
					} else
						val = 0;
				}
		#endif
				//Shoes? No packet uses this....
			break;
			case LOOK_BODY:
			case LOOK_FLOOR:
				// unknown purpose
			break;
			case LOOK_ROBE:
		#if PACKETVER < 20110111
				return;
		#else
				vd->robe = val;
		#endif
			break;
			case LOOK_BODY2:
				if (sd != NULL && (sd->sc.option&OPTION_COSTUME) != OPTION_NOTHING)
					val = 0;
				vd->body_style = val;
			break;
	}

	// prevent leaking the presence of GM-hidden objects
	if (sc && sc->option&OPTION_INVISIBLE && !clif->isdisguised(bl))
		target = SELF;
#if PACKETVER < 4
	clif->sendlook(bl, bl->id, type, val, 0, target);
#else
	if (bl->type != BL_NPC) {
		if(type == LOOK_WEAPON || type == LOOK_SHIELD) {
			nullpo_retv(vd);
			type = LOOK_WEAPON;
			val = vd->weapon;
			val2 = vd->shield;
		}
		if (clif->isdisguised(bl)) {
			clif->sendlook(bl, bl->id, type, val, val2, AREA_WOS);
			clif->sendlook(bl, -bl->id, type, val, val2, SELF);
		} else {
			clif->sendlook(bl, bl->id, type, val, val2, target);
		}
	} else {
		struct npc_data *nd = BL_UCAST(BL_NPC, bl);
		npc->refresh(nd);
	}
#endif
}

//Sends a change-base-look packet required for traps as they are triggered.
static void clif_changetraplook(struct block_list *bl, int val)
{
	clif->sendlook(bl, bl->id, LOOK_BASE, val, 0, AREA);
}

/// 00c3 <id>.L <type>.B <value>.B (ZC_SPRITE_CHANGE)
/// 01d7 <id>.L <type>.B <value>.L (ZC_SPRITE_CHANGE2)
static void clif_sendlook(struct block_list *bl, int id, int type, int val, int val2, enum send_target target)
{
	struct PACKET_ZC_SPRITE_CHANGE p;
	p.packetType = sendLookType;
	p.AID = id;
	p.type = type;
	p.val = val;
#if PACKETVER >= 4
	p.val2 = val2;
#endif
	clif->send(&p, sizeof(p), bl, target);
}

//For the stupid cloth-dye bug. Resends the given view data to the area specified by bl.
static void clif_refreshlook(struct block_list *bl, int id, int type, int val, enum send_target target)
{
	clif->sendlook(bl, id, type, val, 0, target);
}

/// Character status (ZC_STATUS).
/// 00bd <stpoint>.W <str>.B <need str>.B <agi>.B <need agi>.B <vit>.B <need vit>.B
///     <int>.B <need int>.B <dex>.B <need dex>.B <luk>.B <need luk>.B <atk>.W <atk2>.W
///     <matk min>.W <matk max>.W <def>.W <def2>.W <mdef>.W <mdef2>.W <hit>.W
///     <flee>.W <flee2>.W <crit>.W <aspd>.W <aspd2>.W
static void clif_initialstatus(struct map_session_data *sd)
{
	int fd, mdef2;
	unsigned char *buf;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xbd));
	buf=WFIFOP(fd,0);

	WBUFW(buf,0)=0xbd;
	WBUFW(buf,2)=min(sd->status.status_point, INT16_MAX);
	WBUFB(buf,4)=min(sd->status.str, UINT8_MAX);
	WBUFB(buf,5)=pc->need_status_point(sd,SP_STR,1);
	WBUFB(buf,6)=min(sd->status.agi, UINT8_MAX);
	WBUFB(buf,7)=pc->need_status_point(sd,SP_AGI,1);
	WBUFB(buf,8)=min(sd->status.vit, UINT8_MAX);
	WBUFB(buf,9)=pc->need_status_point(sd,SP_VIT,1);
	WBUFB(buf,10)=min(sd->status.int_, UINT8_MAX);
	WBUFB(buf,11)=pc->need_status_point(sd,SP_INT,1);
	WBUFB(buf,12)=min(sd->status.dex, UINT8_MAX);
	WBUFB(buf,13)=pc->need_status_point(sd,SP_DEX,1);
	WBUFB(buf,14)=min(sd->status.luk, UINT8_MAX);
	WBUFB(buf,15)=pc->need_status_point(sd,SP_LUK,1);

	WBUFW(buf,16) = pc_leftside_atk(sd);
	WBUFW(buf,18) = pc_rightside_atk(sd);
	WBUFW(buf,20) = pc_rightside_matk(sd);
	WBUFW(buf,22) = pc_leftside_matk(sd);
	WBUFW(buf,24) = pc_leftside_def(sd);
	WBUFW(buf,26) = pc_rightside_def(sd);
	WBUFW(buf,28) = pc_leftside_mdef(sd);
	mdef2 = pc_rightside_mdef(sd);
	WBUFW(buf,30) =
#ifndef RENEWAL
		( mdef2 < 0 ) ? 0 : //Negative check for Frenzy'ed characters.
#endif
		mdef2;
	WBUFW(buf,32) = sd->battle_status.hit;
	WBUFW(buf,34) = sd->battle_status.flee;
	WBUFW(buf,36) = sd->battle_status.flee2/10;
	WBUFW(buf,38) = sd->battle_status.cri/10;
	WBUFW(buf,40) = sd->battle_status.amotion; // aspd
	WBUFW(buf,42) = 0;  // always 0 (plusASPD)

	WFIFOSET(fd,packet_len(0xbd));

	clif->updatestatus(sd,SP_STR);
	clif->updatestatus(sd,SP_AGI);
	clif->updatestatus(sd,SP_VIT);
	clif->updatestatus(sd,SP_INT);
	clif->updatestatus(sd,SP_DEX);
	clif->updatestatus(sd,SP_LUK);

	clif->updatestatus(sd,SP_ATTACKRANGE);
	clif->updatestatus(sd,SP_ASPD);
}

/// Marks an ammunition item in inventory as equipped (ZC_EQUIP_ARROW).
/// 013c <index>.W
static void clif_arrowequip(struct map_session_data *sd, int val)
{
	int fd;

	nullpo_retv(sd);

#if PACKETVER >= 20121128
	clif->status_change(&sd->bl, status->get_sc_icon(SC_CLIENT_ONLY_EQUIP_ARROW), status->get_sc_relevant_bl_types(SC_CLIENT_ONLY_EQUIP_ARROW), 1, INVALID_TIMER, 0, 0, 0);
#endif
	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0x013c));
	WFIFOW(fd,0)=0x013c;
	WFIFOW(fd,2)=val+2; //Item ID of the arrow
	WFIFOSET(fd,packet_len(0x013c));
}

/// Ammunition action message (ZC_ACTION_FAILURE).
/// 013b <type>.W
/// type:
///     0 = MsgStringTable[242]="Please equip the proper ammunition first."
///     1 = MsgStringTable[243]="You can't Attack or use Skills because your Weight Limit has been exceeded."
///     2 = MsgStringTable[244]="You can't use Skills because Weight Limit has been exceeded."
///     3 = assassin, baby_assassin, assassin_cross => MsgStringTable[1040]="You have equipped throwing daggers."
///         gunslinger => MsgStringTable[1175]="Bullets have been equipped."
///         NOT ninja => MsgStringTable[245]="Ammunition has been equipped."
static void clif_arrow_fail(struct map_session_data *sd, int type)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(0x013b));
	WFIFOW(fd,0)=0x013b;
	WFIFOW(fd,2)=type;
	WFIFOSET(fd,packet_len(0x013b));
}

/// Presents a list of items, that can be processed by Arrow Crafting (ZC_MAKINGARROW_LIST).
/// 01ad <packet len>.W { <name id>.W }*
static void clif_arrow_create_list(struct map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	int len = MAX_SKILL_ARROW_DB * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MAKINGARROW_LIST *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGARROW_LIST;

	int c = 0;
	for (int i = 0; i < MAX_SKILL_ARROW_DB; i++) {
		int j;
		if (skill->dbs->arrow_db[i].nameid > 0
		 && (j = pc->search_inventory(sd, skill->dbs->arrow_db[i].nameid)) != INDEX_NOT_FOUND
		 && !sd->status.inventory[j].equip && sd->status.inventory[j].identify
		) {
			if ((j = itemdb_viewid(skill->dbs->arrow_db[i].nameid)) > 0)
				p->items[c].itemId = j;
			else
				p->items[c].itemId = skill->dbs->arrow_db[i].nameid;
			c++;
		}
	}
	len = c * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	p->packetLength = len;
	WFIFOSET(fd, len);
	if (c > 0) {
		sd->menuskill_id = AC_MAKINGARROW;
		sd->menuskill_val = c;
	}
}

/// Notifies the client, about the result of an status change request (ZC_STATUS_CHANGE_ACK).
/// 00bc <status id>.W <result>.B <value>.B
/// status id:
///     SP_STR ~ SP_LUK
/// result:
///     0 = failure
///     1 = success
static void clif_statusupack(struct map_session_data *sd, int type, int ok, int val)
{
	nullpo_retv(sd);
	int fd = sd->fd;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_STATUS_CHANGE_ACK));
	struct PACKET_ZC_STATUS_CHANGE_ACK *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_STATUS_CHANGE_ACK;
	p->sp = type;
	p->ok = ok;
	p->value = cap_value(val, 0, UINT8_MAX);
	WFIFOSET(fd, sizeof(struct PACKET_ZC_STATUS_CHANGE_ACK));
}

/// Notifies the client about the result of a request to equip an item (ZC_REQ_WEAR_EQUIP_ACK).
/// 00aa <index>.W <equip location>.W <result>.B
/// 00aa <index>.W <equip location>.W <view id>.W <result>.B (PACKETVER >= 20100629)
static void clif_equipitemack(struct map_session_data *sd, int n, int pos, enum e_EQUIP_ITEM_ACK result)
{
	struct packet_equipitem_ack p;

	nullpo_retv(sd);

	p.PacketType = equipitemackType;
	p.index = n+2;
	p.wearLocation = pos;
#if PACKETVER >= 20100629
	Assert_retv(n >= 0 && n < sd->status.inventorySize);
	if (result == EIA_SUCCESS && sd->inventory_data[n]->equip&EQP_VISIBLE)
		p.wItemSpriteNumber = sd->inventory_data[n]->view_sprite;
	else
		p.wItemSpriteNumber = 0;
#endif
	p.result = (unsigned char)result;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
}

/// Notifies the client about the result of a request to take off an item (ZC_REQ_TAKEOFF_EQUIP_ACK).
/// 00ac <index>.W <equip location>.W <result>.B
static void clif_unequipitemack(struct map_session_data *sd, int n, int pos, enum e_UNEQUIP_ITEM_ACK result)
{
	struct packet_unequipitem_ack p;

	nullpo_retv(sd);

	p.PacketType = unequipitemackType;
	p.index = n+2;
	p.wearLocation = pos;
	p.result = (unsigned char)result;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
}

/// Notifies clients in the area about an special/visual effect (ZC_NOTIFY_EFFECT).
/// 019b <id>.L <effect id>.L
/// effect id:
///     0 = base level up
///     1 = job level up
///     2 = refine failure
///     3 = refine success
///     4 = game over
///     5 = pharmacy success
///     6 = pharmacy failure
///     7 = base level up (super novice)
///     8 = job level up (super novice)
///     9 = base level up (taekwon)
static void clif_misceffect(struct block_list *bl, int type)
{
	unsigned char buf[32];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x19b;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	clif->send(buf,packet_len(0x19b),bl,AREA);
}

/// Notifies clients in the area of a state change.
/// 0119 <id>.L <body state>.W <health state>.W <effect state>.W <pk mode>.B (ZC_STATE_CHANGE)
/// 0229 <id>.L <body state>.W <health state>.W <effect state>.L <pk mode>.B (ZC_STATE_CHANGE3)
static void clif_changeoption(struct block_list *bl)
{
	nullpo_retv(bl);

	struct status_change *sc = status->get_sc(bl);

	if (sc == NULL && bl->type != BL_NPC) // How can an option change if there's no sc?
		return;

	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	struct PACKET_ZC_STATE_CHANGE p;
	p.packetType = HEADER_ZC_STATE_CHANGE;
	p.AID = bl->id;
	p.bodyState = (sc != NULL) ? sc->opt1 : 0;
	p.healthState = (sc != NULL) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : BL_UCCAST(BL_NPC, bl)->option;
	p.isPKModeON = (sd != NULL) ? sd->status.karma : 0;
	if (clif->isdisguised(bl)) {
		clif->send(&p, sizeof(p), bl, AREA_WOS);
		p.AID = -bl->id;
		clif->send(&p, sizeof(p), bl, SELF);
		p.AID = bl->id;
		p.effectState = OPTION_INVISIBLE;
		clif->send(&p, sizeof(p), bl, SELF);
	} else {
		clif->send(&p, sizeof(p), bl, AREA);
	}
}

static void clif_changeoption_target(struct block_list *bl, struct block_list *target_bl, enum send_target target)
{
	nullpo_retv(bl);
	nullpo_retv(target_bl);

	struct status_change *sc = status->get_sc(bl);

	if (sc == NULL && bl->type != BL_NPC) // How can an option change if there's no sc?
		return;

	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	struct PACKET_ZC_STATE_CHANGE p;
	p.packetType = HEADER_ZC_STATE_CHANGE;
	p.AID = bl->id;
	p.bodyState = (sc != NULL) ? sc->opt1 : 0;
	p.healthState = (sc != NULL) ? sc->opt2 : 0;
	p.effectState = (sc != NULL) ? sc->option : BL_UCCAST(BL_NPC, bl)->option;
	p.isPKModeON = (sd != NULL) ? sd->status.karma : 0;
	if (clif->isdisguised(bl)) {
		p.AID = -bl->id;
		clif->send(&p, sizeof(p), target_bl, target);
		p.AID = bl->id;
		p.effectState = OPTION_INVISIBLE;
	}
	clif->send(&p, sizeof(p), target_bl, target);
}

/// Displays status change effects on NPCs/monsters (ZC_NPC_SHOWEFST_UPDATE).
/// 028a <id>.L <effect state>.L <level>.L <showEFST>.L
static void clif_changeoption2(struct block_list *bl)
{
	unsigned char buf[20];
	struct status_change *sc;

	nullpo_retv(bl);
	if ( !(sc = status->get_sc(bl)) && bl->type != BL_NPC ) return; //How can an option change if there's no sc?

	WBUFW(buf,0) = 0x28a;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = (sc != NULL) ? sc->option : ((bl->type == BL_NPC) ? BL_UCCAST(BL_NPC, bl)->option : 0);
	WBUFL(buf,10) = clif_setlevel(bl);
	WBUFL(buf,14) = (sc) ? sc->opt3 : 0;
	if (clif->isdisguised(bl)) {
		clif->send(buf,packet_len(0x28a),bl,AREA_WOS);
		WBUFL(buf,2) = -bl->id;
		clif->send(buf,packet_len(0x28a),bl,SELF);
		WBUFL(buf,2) = bl->id;
		WBUFL(buf,6) = OPTION_INVISIBLE;
		clif->send(buf,packet_len(0x28a),bl,SELF);
	} else {
		clif->send(buf,packet_len(0x28a),bl,AREA);
	}
}

/// Notifies the client about the result of an item use request.
/// 00a8 <index>.W <amount>.W <result>.B (ZC_USE_ITEM_ACK)
/// 01c8 <index>.W <name id>.W <id>.L <amount>.W <result>.B (ZC_USE_ITEM_ACK2)
static void clif_useitemack(struct map_session_data *sd, int index, int amount, bool ok)
{
	struct PACKET_ZC_USE_ITEM_ACK p;
	int fd;

	nullpo_retv(sd);

	if (index < 0 || index >= sd->status.inventorySize)
		return;

	fd = sd->fd;
	p.packetType = useItemAckType;
	p.index = index + 2;
#if PACKETVER > 3
	if (sd->inventory_data[index] && sd->inventory_data[index]->view_id > 0)
		p.itemId = sd->inventory_data[index]->view_id;
	else
		p.itemId = sd->status.inventory[index].nameid;
	p.AID = sd->bl.id;
#endif
	p.amount = amount;
	p.result = ok;

	if (!ok) {
		WFIFOHEAD(fd, sizeof(p));
		memcpy(WFIFOP(fd, 0), &p, sizeof(p));
		WFIFOSET(fd, sizeof(p));
	} else {
		clif->send(&p, sizeof(p), &sd->bl, AREA);
	}
}

/// Inform client whether chatroom creation was successful or not (ZC_ACK_CREATE_CHATROOM).
/// 00d6 <flag>.B
/// flag:
///     0 = Room has been successfully created (opens chat room)
///     1 = Room limit exceeded
///     2 = Same room already exists
// TODO: Flag enum
static void clif_createchat(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xd6));
	WFIFOW(fd,0) = 0xd6;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_len(0xd6));
}

/// Display a chat above the owner (ZC_ROOM_NEWENTRY).
/// 00d7 <packet len>.W <owner id>.L <char id>.L <limit>.W <users>.W <type>.B <title>.?B
/// type:
///     0 = private (password protected)
///     1 = public
///     2 = arena (npc waiting room)
///     3 = PK zone (non-clickable)
static void clif_dispchat(struct chat_data *cd, int fd)
{
	unsigned char buf[128];
	uint8 type;
	int len;

	if (cd == NULL || cd->owner == NULL)
		return;

	type = (cd->owner->type == BL_PC ) ? (cd->pub) ? 1 : 0
	     : (cd->owner->type == BL_NPC) ? (cd->limit) ? 2 : 3
	     : 1;
	len = (int)strlen(cd->title);
	Assert_retv(len <= INT16_MAX - 17);

	WBUFW(buf, 0) = 0xd7;
	WBUFW(buf, 2) = 17 + len;
	WBUFL(buf, 4) = cd->owner->id;
	WBUFL(buf, 8) = cd->bl.id;
	WBUFW(buf,12) = cd->limit;
	WBUFW(buf,14) = (cd->owner->type == BL_NPC) ? cd->users+1 : cd->users;
	WBUFB(buf,16) = type;
	memcpy(WBUFP(buf,17), cd->title, len); // not zero-terminated

	if( fd ) {
		WFIFOHEAD(fd,WBUFW(buf,2));
		memcpy(WFIFOP(fd,0),buf,WBUFW(buf,2));
		WFIFOSET(fd,WBUFW(buf,2));
	} else {
		clif->send(buf,WBUFW(buf,2),cd->owner,AREA_WOSC);
	}
}

/// Chatroom properties adjustment (ZC_CHANGE_CHATROOM).
/// 00df <packet len>.W <owner id>.L <chat id>.L <limit>.W <users>.W <type>.B <title>.?B
/// type:
///     0 = private (password protected)
///     1 = public
///     2 = arena (npc waiting room)
///     3 = PK zone (non-clickable)
static void clif_changechatstatus(struct chat_data *cd)
{
	unsigned char buf[128];
	uint8 type;
	int len;

	if( cd == NULL || cd->usersd[0] == NULL )
		return;

	type = (cd->owner->type == BL_PC ) ? (cd->pub) ? 1 : 0
	     : (cd->owner->type == BL_NPC) ? (cd->limit) ? 2 : 3
	     : 1;
	len = (int)strlen(cd->title);
	Assert_retv(len <= INT16_MAX - 17);

	WBUFW(buf, 0) = 0xdf;
	WBUFW(buf, 2) = 17 + len;
	WBUFL(buf, 4) = cd->owner->id;
	WBUFL(buf, 8) = cd->bl.id;
	WBUFW(buf,12) = cd->limit;
	WBUFW(buf,14) = (cd->owner->type == BL_NPC) ? cd->users+1 : cd->users;
	WBUFB(buf,16) = type;
	memcpy(WBUFP(buf,17), cd->title, len); // not zero-terminated

	clif->send(buf,WBUFW(buf,2),cd->owner,CHAT);
}

/// Removes the chatroom (ZC_DESTROY_ROOM).
/// 00d8 <chat id>.L
static void clif_clearchat(struct chat_data *cd, int fd)
{
	unsigned char buf[32];

	nullpo_retv(cd);

	WBUFW(buf,0) = 0xd8;
	WBUFL(buf,2) = cd->bl.id;
	if( fd ) {
		WFIFOHEAD(fd,packet_len(0xd8));
		memcpy(WFIFOP(fd,0),buf,packet_len(0xd8));
		WFIFOSET(fd,packet_len(0xd8));
	} else {
		clif->send(buf,packet_len(0xd8),cd->owner,AREA_WOSC);
	}
}

/// Displays messages regarding join chat failures (ZC_REFUSE_ENTER_ROOM).
/// 00da <result>.B
/// result:
///     0 = room full
///     1 = wrong password
///     2 = kicked
///     3 = success (no message)
///     4 = no enough zeny
///     5 = too low level
///     6 = too high level
///     7 = unsuitable job class
static void clif_joinchatfail(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0xda));
	WFIFOW(fd,0) = 0xda;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_len(0xda));
}

/// Notifies the client about entering a chatroom (ZC_ENTER_ROOM).
/// 00db <packet len>.W <chat id>.L { <role>.L <name>.24B }*
/// role:
///     0 = owner (menu)
///     1 = normal
static void clif_joinchatok(struct map_session_data *sd, struct chat_data *cd)
{
	int fd;
	int i,t;

	nullpo_retv(sd);
	nullpo_retv(cd);

	fd = sd->fd;
	if (!sockt->session_is_active(fd))
		return;
	t = (int)(cd->owner->type == BL_NPC);
	WFIFOHEAD(fd, 8 + (28*(cd->users+t)));
	WFIFOW(fd, 0) = 0xdb;
	WFIFOW(fd, 2) = 8 + (28*(cd->users+t));
	WFIFOL(fd, 4) = cd->bl.id;

	if(cd->owner->type == BL_NPC) {
		const struct npc_data *nd = BL_UCCAST(BL_NPC, cd->owner);
		WFIFOL(fd, 30) = 1;
		WFIFOL(fd, 8) = 0;
		memcpy(WFIFOP(fd, 12), nd->name, NAME_LENGTH);
		for (i = 0; i < cd->users; i++) {
			WFIFOL(fd, 8+(i+1)*28) = 1;
			memcpy(WFIFOP(fd, 8+(i+t)*28+4), cd->usersd[i]->status.name, NAME_LENGTH);
		}
	} else
	for (i = 0; i < cd->users; i++) {
		WFIFOL(fd, 8+i*28) = (i != 0 || cd->owner->type == BL_NPC);
		memcpy(WFIFOP(fd, 8+(i+t)*28+4), cd->usersd[i]->status.name, NAME_LENGTH);
	}
	WFIFOSET(fd, WFIFOW(fd, 2));
}

/// Notifies clients in a chat about a new member (ZC_MEMBER_NEWENTRY).
/// 00dc <users>.W <name>.24B
static void clif_addchat(struct chat_data *cd, struct map_session_data *sd)
{
	unsigned char buf[32];

	nullpo_retv(sd);
	nullpo_retv(cd);

	WBUFW(buf, 0) = 0xdc;
	WBUFW(buf, 2) = cd->users;
	memcpy(WBUFP(buf, 4),sd->status.name,NAME_LENGTH);
	clif->send(buf,packet_len(0xdc),&sd->bl,CHAT_WOS);
}

/// Announce the new owner (ZC_ROLE_CHANGE).
/// 00e1 <role>.L <nick>.24B
/// role:
///     0 = owner (menu)
///     1 = normal
static void clif_chatRoleChange(struct chat_data *cd, struct map_session_data *sd, struct block_list* bl, int isNotOwner)
{
	nullpo_retv(sd);
	nullpo_retv(bl);
	struct PACKET_ZC_ROLE_CHANGE p;

	p.packetType = HEADER_ZC_ROLE_CHANGE;
	p.flag = isNotOwner;
	memcpy(&p.name, sd->status.name, NAME_LENGTH);
	clif->send(&p, sizeof(struct PACKET_ZC_ROLE_CHANGE), bl, CHAT);
}

static void clif_changechatowner(struct chat_data *cd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	nullpo_retv(cd);

	clif->chatRoleChange(cd, cd->usersd[0], &sd->bl, 1);
	clif->chatRoleChange(cd, sd, &sd->bl, 0);
}

/// Notify about user leaving the chatroom (ZC_MEMBER_EXIT).
/// 00dd <users>.W <nick>.24B <flag>.B
/// flag:
///     0 = left
///     1 = kicked
static void clif_leavechat(struct chat_data *cd, struct map_session_data *sd, bool flag)
{
	unsigned char buf[32];

	nullpo_retv(sd);
	nullpo_retv(cd);

	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = cd->users-1;
	memcpy(WBUFP(buf,4),sd->status.name,NAME_LENGTH);
	WBUFB(buf,28) = flag;

	clif->send(buf,packet_len(0xdd),&sd->bl,CHAT);
}

/// Opens a trade request window from char 'name'.
/// 00e5 <nick>.24B (ZC_REQ_EXCHANGE_ITEM)
/// 01f4 <nick>.24B <charid>.L <baselvl>.W (ZC_REQ_EXCHANGE_ITEM2)
static void clif_traderequest(struct map_session_data *sd, const char *name)
{
	int fd;
#if PACKETVER >= 6
	struct map_session_data* tsd = NULL;
#endif // PACKETVER >= 6
	nullpo_retv(sd);
	nullpo_retv(name);

	fd = sd->fd;
#if PACKETVER < 6
	WFIFOHEAD(fd,packet_len(0xe5));
	WFIFOW(fd,0) = 0xe5;
	safestrncpy(WFIFOP(fd,2), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0xe5));
#else // PACKETVER >= 6
	tsd = map->id2sd(sd->trade_partner);
	if (!tsd)
		return;

	WFIFOHEAD(fd,packet_len(0x1f4));
	WFIFOW(fd,0) = 0x1f4;
	safestrncpy(WFIFOP(fd,2), name, NAME_LENGTH);
	WFIFOL(fd,26) = tsd->status.char_id;
	WFIFOW(fd,30) = tsd->status.base_level;
	WFIFOSET(fd,packet_len(0x1f4));
#endif // PACKETVER < 6
}

/// Reply to a trade-request.
/// 00e7 <result>.B (ZC_ACK_EXCHANGE_ITEM)
/// 01f5 <result>.B <charid>.L <baselvl>.W (ZC_ACK_EXCHANGE_ITEM2)
/// result:
///     0 = Char is too far
///     1 = Character does not exist
///     2 = Trade failed
///     3 = Accept
///     4 = Cancel
///     5 = Busy
static void clif_tradestart(struct map_session_data *sd, uint8 type)
{
	int fd;
#if PACKETVER >= 20090406
	struct map_session_data *tsd = NULL;
#endif // PACKETVER >= 6
	nullpo_retv(sd);

	fd = sd->fd;
#if PACKETVER >= 20090406
	tsd = map->id2sd(sd->trade_partner);
	if (tsd) {
		WFIFOHEAD(fd,packet_len(0x1f5));
		WFIFOW(fd,0) = 0x1f5;
		WFIFOB(fd,2) = type;
		WFIFOL(fd,3) = tsd->status.char_id;
		WFIFOW(fd,7) = tsd->status.base_level;
		WFIFOSET(fd,packet_len(0x1f5));
		return;
	}
#else
	WFIFOHEAD(fd,packet_len(0xe7));
	WFIFOW(fd,0) = 0xe7;
	WFIFOB(fd,2) = type;
	WFIFOSET(fd,packet_len(0xe7));
#endif // PACKETVER >= 6
}

/// Notifies the client about an item from other player in current trade.
/// 00e9 <amount>.L <nameid>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_EXCHANGE_ITEM)
/// 080f <nameid>.W <item type>.B <amount>.L <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_EXCHANGE_ITEM2)
static void clif_tradeadditem(struct map_session_data *sd, struct map_session_data *tsd, int index, int amount)
{
	int fd;
	struct PACKET_ZC_ADD_EXCHANGE_ITEM p;

	nullpo_retv(sd);
	nullpo_retv(tsd);

	fd = tsd->fd;
	WFIFOHEAD(fd, sizeof(p));
	memset(&p, 0, sizeof(p));
	p.packetType = tradeaddType;
	p.amount = amount;
	if (index != 0)
	{
		index -= 2; //index fix
		Assert_retv(index >= 0 && index < sd->status.inventorySize);
		if(sd->inventory_data[index] && sd->inventory_data[index]->view_id > 0)
			p.itemId = sd->inventory_data[index]->view_id;
		else
			p.itemId = sd->status.inventory[index].nameid;
#if PACKETVER >= 20100223
		p.itemType = sd->inventory_data[index]->type;
#endif
		p.identified = sd->status.inventory[index].identify;
		p.damaged = sd->status.inventory[index].attribute;
		p.refine = sd->status.inventory[index].refine;
		clif->addcards(&p.slot, &sd->status.inventory[index]);
#if PACKETVER >= 20150226
		clif->add_item_options(&p.option_data[0], &sd->status.inventory[index]);
#endif
	}
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Notifies the client about the result of request to add an item to the current trade (ZC_ACK_ADD_EXCHANGE_ITEM).
/// 00ea <index>.W <result>.B
/// result:
///     0 = success
///     1 = overweight
///     2 = trade canceled
///     3 = amount is exceeded. message 0x792
///     4 = other amount is exceeded. message 0x793
static void clif_tradeitemok(struct map_session_data *sd, int index, int fail)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xea));
	WFIFOW(fd,0) = 0xea;
	WFIFOW(fd,2) = index;
	WFIFOB(fd,4) = fail;
	WFIFOSET(fd,packet_len(0xea));
}

/// Notifies the client about finishing one side of the current trade (ZC_CONCLUDE_EXCHANGE_ITEM).
/// 00ec <who>.B
/// who:
///     0 = self
///     1 = other player
static void clif_tradedeal_lock(struct map_session_data *sd, int fail)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xec));
	WFIFOW(fd,0) = 0xec;
	WFIFOB(fd,2) = fail;
	WFIFOSET(fd,packet_len(0xec));
}

/// Notifies the client about the trade being canceled (ZC_CANCEL_EXCHANGE_ITEM).
/// 00ee
static void clif_tradecancelled(struct map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xee));
	WFIFOW(fd,0) = 0xee;
	WFIFOSET(fd,packet_len(0xee));
}

/// Result of a trade (ZC_EXEC_EXCHANGE_ITEM).
/// 00f0 <result>.B
/// result:
///     0 = success
///     1 = failure
static void clif_tradecompleted(struct map_session_data *sd, int fail)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xf0));
	WFIFOW(fd,0) = 0xf0;
	WFIFOB(fd,2) = fail;
	WFIFOSET(fd,packet_len(0xf0));
}

/// Resets the trade window on the send side (ZC_EXCHANGEITEM_UNDO).
/// 00f1
/// NOTE: Unknown purpose. Items are not removed until the window is
///       refreshed (ex. by putting another item in there).
/// unused
static void clif_tradeundo(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xf1));
	WFIFOW(fd,0) = 0xf1;
	WFIFOSET(fd,packet_len(0xf1));
}

/// Updates storage total amount (ZC_NOTIFY_STOREITEM_COUNTINFO).
/// 00f2 <current count>.W <max count>.W
static void clif_updatestorageamount(struct map_session_data *sd, int amount, int max_amount)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xf2));
	WFIFOW(fd,0) = 0xf2;
	WFIFOW(fd,2) = amount;
	WFIFOW(fd,4) = max_amount;
	WFIFOSET(fd,packet_len(0xf2));
}

/// Notifies the client of an item being added to the storage.
/// 00f4 <index>.W <amount>.L <nameid>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_STORE)
/// 01c4 <index>.W <amount>.L <nameid>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_STORE2)
static void clif_storageitemadded(struct map_session_data *sd, struct item *i, int index, int amount)
{
	int view, fd;
	struct PACKET_ZC_ADD_ITEM_TO_STORE p;

	nullpo_retv(sd);
	nullpo_retv(i);

	fd = sd->fd;
	view = itemdb_viewid(i->nameid);

	WFIFOHEAD(fd, sizeof(p));
	p.packetType = storageaddType; // Storage item added
	p.index = index + 1; // index
	p.amount = amount; // amount
	p.itemId = (view > 0) ? view : i->nameid; // id
#if PACKETVER >= 5
	p.itemType = itemtype(itemdb_type(i->nameid)); //type
#endif
	p.identified = i->identify; //identify flag
	p.damaged = i->attribute; // attribute
	p.refine = i->refine; //refine
	clif->addcards(&p.slot, i);
#if PACKETVER >= 20150226
	clif->add_item_options(&p.option_data[0], i);
#endif
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Notifies the client of an item being deleted from the storage (ZC_DELETE_ITEM_FROM_STORE).
/// 00f6 <index>.W <amount>.L
static void clif_storageitemremoved(struct map_session_data *sd, int index, int amount)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xf6));
	WFIFOW(fd, 0) = 0xf6; // Storage item removed
	WFIFOW(fd, 2) = index + 1;
	WFIFOL(fd, 4) = amount;
	WFIFOSET(fd, packet_len(0xf6));
}

/// Closes storage (ZC_CLOSE_STORE).
/// 00f8
static void clif_storageclose(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xf8));
	WFIFOW(fd,0) = 0xf8; // Storage Closed
	WFIFOSET(fd,packet_len(0xf8));
}

/*==========================================
 * Server tells 'sd' player client the abouts of 'dstsd' player
 *------------------------------------------*/
static void clif_getareachar_pc(struct map_session_data *sd, struct map_session_data *dstsd)
{
	struct block_list *d_bl;
	int i;

	nullpo_retv(sd);
	nullpo_retv(dstsd);
	if (dstsd->chat_id != 0) {
		struct chat_data *cd = map->id2cd(dstsd->chat_id);
		if (cd != NULL && cd->usersd[0] == dstsd)
			clif->dispchat(cd,sd->fd);
	} else if( dstsd->state.vending )
		clif->showvendingboard(&dstsd->bl,dstsd->message,sd->fd);
	else if( dstsd->state.buyingstore )
		clif->buyingstore_entry_single(sd, dstsd);

	if(dstsd->spiritball > 0)
		clif->spiritball_single(sd->fd, dstsd);
	if (dstsd->charm_type != CHARM_TYPE_NONE && dstsd->charm_count > 0)
		clif->charm_single(sd->fd, dstsd);

	for( i = 0; i < dstsd->sc_display_count; i++ ) {
		clif->sc_continue(&sd->bl, dstsd->bl.id, SELF, status->get_sc_icon(dstsd->sc_display[i]->type), dstsd->sc_display[i]->val1, dstsd->sc_display[i]->val2, dstsd->sc_display[i]->val3);
	}
	if( (sd->status.party_id && dstsd->status.party_id == sd->status.party_id) || //Party-mate, or hpdisp setting.
		(sd->bg_id && sd->bg_id == dstsd->bg_id) || //BattleGround
		pc_has_permission(sd, PC_PERM_VIEW_HPMETER)
	)
		clif->hpmeter_single(sd->fd, dstsd->bl.id, dstsd->battle_status.hp, dstsd->battle_status.max_hp);

	// display link (sd - dstsd) to sd
	ARR_FIND( 0, MAX_PC_DEVOTION, i, sd->devotion[i] == dstsd->bl.id );
	if( i < MAX_PC_DEVOTION ) clif->devotion(&sd->bl, sd);
	// display links (dstsd - devotees) to sd
	ARR_FIND( 0, MAX_PC_DEVOTION, i, dstsd->devotion[i] > 0 );
	if( i < MAX_PC_DEVOTION ) clif->devotion(&dstsd->bl, sd);
	// display link (dstsd - crusader) to sd
	if( dstsd->sc.data[SC_DEVOTION] && (d_bl = map->id2bl(dstsd->sc.data[SC_DEVOTION]->val1)) != NULL )
		clif->devotion(d_bl, sd);
}

static void clif_getareachar_unit(struct map_session_data *sd, struct block_list *bl)
{
	struct unit_data *ud;
	struct view_data *vd;

	nullpo_retv(sd);
	nullpo_retv(bl);

	vd = status->get_viewdata(bl);
	if (vd == NULL || vd->class == INVISIBLE_CLASS)
		return;

	if (bl->type == BL_NPC) {
		// Hide NPC from maya purple card.
		struct npc_data *nd = BL_UCAST(BL_NPC, bl);
		if (nd->chat_id == 0 && (nd->option&OPTION_INVISIBLE))
			return;
	}

	if ( ( ud = unit->bl2ud(bl) ) && ud->walktimer != INVALID_TIMER )
		clif->set_unit_walking(bl,sd,ud,SELF);
	else
		clif->set_unit_idle(bl,sd,SELF);

	if (vd->cloth_color)
		clif->refreshlook(&sd->bl,bl->id,LOOK_CLOTHES_COLOR,vd->cloth_color,SELF);
	if (vd->body_style)
		clif->refreshlook(&sd->bl,bl->id,LOOK_BODY2,vd->body_style,SELF);

	switch (bl->type) {
		case BL_PC:
		{
			struct map_session_data *tsd = BL_UCAST(BL_PC, bl);
			clif->getareachar_pc(sd, tsd);
			if (tsd->state.size == SZ_BIG) // tiny/big players [Valaris]
				clif->specialeffect_single(bl,423,sd->fd);
			else if (tsd->state.size == SZ_MEDIUM)
				clif->specialeffect_single(bl,421,sd->fd);
			if (tsd->bg_id != 0 && map->list[tsd->bl.m].flag.battleground)
				clif->sendbgemblem_single(sd->fd,tsd);
			if (tsd->status.look.robe != 0)
				clif->refreshlook(&sd->bl, bl->id, LOOK_ROBE, tsd->status.look.robe, SELF);
			clif->hat_effect(bl, &sd->bl, SELF);
		}
			break;
		case BL_MER: // Devotion Effects
		{
			struct mercenary_data *md = BL_UCAST(BL_MER, bl);
			if (md->devotion_flag)
				clif->devotion(bl, sd);
		}
			break;
		case BL_NPC:
		{
			struct npc_data *nd = BL_UCAST(BL_NPC, bl);
			if (nd->chat_id != 0)
				clif->dispchat(map->id2cd(nd->chat_id), sd->fd);
			if (nd->size == SZ_BIG)
				clif->specialeffect_single(bl,423,sd->fd);
			else if (nd->size == SZ_MEDIUM)
				clif->specialeffect_single(bl,421,sd->fd);
			if (nd->clan_id > 0)
				clif->sc_load(&nd->bl, nd->bl.id, AREA, status->get_sc_icon(SC_CLAN_INFO), 0, nd->clan_id, 0);
		}
			break;
		case BL_MOB:
		{
			struct mob_data *md = BL_UCAST(BL_MOB, bl);
			if (md->special_state.size == SZ_BIG) // tiny/big mobs [Valaris]
				clif->specialeffect_single(bl,423,sd->fd);
			else if (md->special_state.size == SZ_MEDIUM)
				clif->specialeffect_single(bl,421,sd->fd);
#if (PACKETVER >= 20120404 && PACKETVER < 20131223)
			if (battle_config.show_monster_hp_bar && !(md->status.mode&MD_BOSS)) {
				int i;
				for (i = 0; i < DAMAGELOG_SIZE; i++) {// must show hp bar to all char who already hit the mob.
					if (md->dmglog[i].id == sd->status.char_id) {
						clif->monster_hp_bar(md, sd);
						break;
					}
				}
			}
#endif
		}
			break;
		case BL_PET:
			if (vd->head_bottom)
				clif->send_petdata(NULL, BL_UCAST(BL_PET, bl), 3, vd->head_bottom); // needed to display pet equip properly
			break;
	}
}

//Modifies the type of damage according to status changes [Skotlex]
//Aegis data specifies that: 4 endure against single hit sources, 9 against multi-hit.
static inline enum battle_dmg_type clif_calc_delay(enum battle_dmg_type type, int div, int damage, int delay)
{
	return ( delay == 0 && damage > 0 ) ? ( div > 1 ? BDT_MULTIENDURE : BDT_ENDURE ) : type;
}

/*==========================================
 * Estimates walk delay based on the damage criteria. [Skotlex]
 *------------------------------------------*/
static int clif_calc_walkdelay(struct block_list *bl, int delay, int type, int damage, int div_)
{
	if (type == 4 || type == 9 || damage <=0)
		return 0;

	nullpo_retr(delay, bl);
	if (bl->type == BL_PC) {
		if (battle_config.pc_walk_delay_rate != 100)
			delay = delay*battle_config.pc_walk_delay_rate/100;
	} else
		if (battle_config.walk_delay_rate != 100)
			delay = delay*battle_config.walk_delay_rate/100;

	if (div_ > 1) //Multi-hit skills mean higher delays.
		delay += battle_config.multihit_delay*(div_-1);

	return delay>0?delay:1; //Return 1 to specify there should be no noticeable delay, but you should stop walking.
}

/// Sends a 'damage' packet (src performs action on dst)
/// 008a <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.W <div>.W <type>.B <damage2>.W (ZC_NOTIFY_ACT)
/// 02e1 <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.L <div>.W <type>.B <damage2>.L (ZC_NOTIFY_ACT2)
/// 08c8 <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.L <IsSPDamage>.B <div>.W <type>.B <damage2>.L (ZC_NOTIFY_ACT2)
/// type: @see enum battle_dmg_type
///     for BDT_NORMAL: [ damage: total damage, div: amount of hits, damage2: assassin dual-wield damage ]
static int clif_damage(struct block_list *src, struct block_list *dst, int sdelay, int ddelay, int64 in_damage, short div, enum battle_dmg_type type, int64 in_damage2)
{
	struct packet_damage p;
	struct status_change *sc;
#if PACKETVER < 20071113
	short damage,damage2;
#else
	int damage,damage2;
#endif

	nullpo_ret(src);
	nullpo_ret(dst);

	sc = status->get_sc(dst);

	if(sc && sc->count && sc->data[SC_ILLUSION]) {
		if(in_damage) in_damage = in_damage*(sc->data[SC_ILLUSION]->val2) + rnd()%100;
		if(in_damage2) in_damage2 = in_damage2*(sc->data[SC_ILLUSION]->val2) + rnd()%100;
	}

#if PACKETVER < 20071113
	damage = (short)min(in_damage,INT16_MAX);
	damage2 = (short)min(in_damage2,INT16_MAX);
#else
	damage = (int)min(in_damage,INT_MAX);
	damage2 = (int)min(in_damage2,INT_MAX);
#endif

	type = clif_calc_delay(type,div,damage+damage2,ddelay);

	p.PacketType = damageType;
	p.GID = src->id;
	p.targetGID = dst->id;
	p.startTime = (uint32)timer->gettick();
	p.attackMT = sdelay;
	p.attackedMT = ddelay;
	p.count = div;
	p.action = type;

	if( battle_config.hide_woe_damage && map_flag_gvg2(src->m) ) {
		p.damage = damage?div:0;
		p.leftDamage = damage2?div:0;
	} else {
		p.damage = damage;
		p.leftDamage = damage2;
	}
#if PACKETVER >= 20131223
	p.is_sp_damaged = 0; // TODO: IsSPDamage - Displays blue digits.
#endif

	if (clif->isdisguised(dst)) {
		clif->send(&p,sizeof(p),dst,AREA_WOS);
		p.targetGID = -dst->id;
		clif->send(&p,sizeof(p),dst,SELF);
	} else {
		clif->send(&p,sizeof(p),dst,AREA);
	}

	if (clif->isdisguised(src)) {
		p.GID = -src->id;
		if (clif->isdisguised(dst))
			p.targetGID = dst->id;

		if(damage > 0) p.damage = -1;
		if(damage2 > 0) p.leftDamage = -1;

		clif->send(&p,sizeof(p),src,SELF);
	}

	if(src == dst) {
		unit->set_dir(src, unit->getdir(src));
	}

	//Return adjusted can't walk delay for further processing.
	return clif->calc_walkdelay(dst,ddelay,type,damage+damage2,div);
}

/*==========================================
 * src picks up dst
 *------------------------------------------*/
static void clif_takeitem(struct block_list *src, struct block_list *dst)
{
	//clif->damage(src,dst,0,0,0,0,BDT_PICKUP,0);
	unsigned char buf[32];

	nullpo_retv(src);
	nullpo_retv(dst);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = src->id;
	WBUFL(buf, 6) = dst->id;
	WBUFB(buf,26) = 1;
	clif->send(buf, packet_len(0x8a), src, AREA);

}

/*==========================================
 * inform clients in area that `bl` is sitting
 *------------------------------------------*/
static void clif_sitting(struct block_list *bl)
{
	unsigned char buf[32];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = bl->id;
	WBUFB(buf,26) = 2;
	clif->send(buf, packet_len(0x8a), bl, AREA);

	if (clif->isdisguised(bl)) {
		WBUFL(buf, 2) = - bl->id;
		clif->send(buf, packet_len(0x8a), bl, SELF);
	}
}

/*==========================================
 * inform clients in area that `bl` is standing
 *------------------------------------------*/
static void clif_standing(struct block_list *bl)
{
	unsigned char buf[32];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = bl->id;
	WBUFB(buf,26) = 3;
	clif->send(buf, packet_len(0x8a), bl, AREA);

	if (clif->isdisguised(bl)) {
		WBUFL(buf, 2) = - bl->id;
		clif->send(buf, packet_len(0x8a), bl, SELF);
	}
}

/// Inform client(s) about a map-cell change (ZC_UPDATE_MAPINFO).
/// 0192 <x>.W <y>.W <type>.W <map name>.16B
static void clif_changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target)
{
	unsigned char buf[32];

	WBUFW(buf,0) = 0x192;
	WBUFW(buf,2) = x;
	WBUFW(buf,4) = y;
	WBUFW(buf,6) = type;
	mapindex->getmapname_ext(map->list[m].custom_name ? map->list[map->list[m].instance_src_map].name : map->list[m].name, WBUFP(buf,8));

	if( fd ) {
		WFIFOHEAD(fd,packet_len(0x192));
		memcpy(WFIFOP(fd,0), buf, packet_len(0x192));
		WFIFOSET(fd,packet_len(0x192));
	} else {
		struct block_list dummy_bl;
		dummy_bl.type = BL_NUL;
		dummy_bl.x = x;
		dummy_bl.y = y;
		dummy_bl.m = m;
		clif->send(buf,packet_len(0x192),&dummy_bl,target);
	}
}

/// Notifies the client about an item on floor (ZC_ITEM_ENTRY).
/// 009d <id>.L <name id>.W <identified>.B <x>.W <y>.W <amount>.W <subX>.B <subY>.B
static void clif_getareachar_item(struct map_session_data *sd, struct flooritem_data *fitem)
{
	int view, fd;
	struct PACKET_ZC_ITEM_ENTRY p;

	nullpo_retv(sd);
	nullpo_retv(fitem);
	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x9d;
	p.AID = fitem->bl.id;
	if ((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
		p.itemId = view;
	else
		p.itemId = fitem->item_data.nameid;
	p.identify = fitem->item_data.identify;
	p.x = fitem->bl.x;
	p.y = fitem->bl.y;
	p.amount = fitem->item_data.amount;
	p.subX = fitem->subx;
	p.subY = fitem->suby;

	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

static void clif_graffiti_entry(struct block_list *bl, struct skill_unit *su, enum send_target target)
{
	struct packet_graffiti_entry p;

	nullpo_retv(bl);
	nullpo_retv(su);
	nullpo_retv(su->group);
	p.PacketType = graffiti_entryType;
	p.AID = su->bl.id;
	p.creatorAID = su->group->src_id;
	p.xPos = su->bl.x;
	p.yPos = su->bl.y;
	p.job = su->group->unit_id;
	p.isContens = 1;
	p.isVisible = 1;
	safestrncpy(p.msg, su->group->valstr, 80);

	clif->send(&p,sizeof(p),bl,target);
}

/// Notifies the client of a skill unit.
/// 011f <id>.L <creator id>.L <x>.W <y>.W <unit id>.B <visible>.B (ZC_SKILL_ENTRY)
/// 01c9 <id>.L <creator id>.L <x>.W <y>.W <unit id>.B <visible>.B <has msg>.B <msg>.80B (ZC_SKILL_ENTRY2)
/// 08c7 <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.B <range>.W <visible>.B (ZC_SKILL_ENTRY3)
/// 099f <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.L <range>.W <visible>.B (ZC_SKILL_ENTRY4)
static void clif_getareachar_skillunit(struct block_list *bl, struct skill_unit *su, enum send_target target)
{
	struct packet_skill_entry p;
	nullpo_retv(bl);
	nullpo_retv(su);
	nullpo_retv(su->group);

	if( su->group->state.guildaura )
		return;

#if PACKETVER >= 3
	if(su->group->unit_id == UNT_GRAFFITI) {
		clif->graffiti_entry(bl,su,target);
		return;
	}
#endif

	p.PacketType = skill_entryType;
#if PACKETVER >= 20110718
	p.PacketLength = sizeof(p);
#endif

	p.AID = su->bl.id;
	p.creatorAID = su->group->src_id;
	p.xPos = su->bl.x;
	p.yPos = su->bl.y;

	// Use invisible unit id for some ground skills.
	if (skill->get_unit_flag(su->group->skill_id) & UF_RANGEDSINGLEUNIT && !(su->val2 & UF_RANGEDSINGLEUNIT))
		p.job = UNT_DUMMYSKILL;
	else
		p.job = su->group->unit_id;

#if PACKETVER >= 20110718
	p.RadiusRange = (unsigned char)su->range;
#endif

	p.isVisible = su->visible;

#if PACKETVER >= 20130731
	p.level = (unsigned char)su->group->skill_lv;
#endif

	clif->send(&p,sizeof(p),bl,target);

	if (su->group->skill_id == WZ_ICEWALL) {
		struct map_session_data *sd = BL_CAST(BL_PC, bl);
		clif->changemapcell(sd != NULL ? sd->fd : 0, su->bl.m, su->bl.x, su->bl.y, 5, SELF);
	}
}

/*==========================================
 * Server tells client to remove unit of id 'unit->bl.id'
 *------------------------------------------*/
static void clif_clearchar_skillunit(struct skill_unit *su, int fd)
{
	nullpo_retv(su);

	WFIFOHEAD(fd,packet_len(0x120));
	WFIFOW(fd, 0)=0x120;
	WFIFOL(fd, 2)=su->bl.id;
	WFIFOSET(fd,packet_len(0x120));

	if(su->group && su->group->skill_id == WZ_ICEWALL)
		clif->changemapcell(fd,su->bl.m,su->bl.x,su->bl.y,su->val2,SELF);
}

/// Removes a skill unit (ZC_SKILL_DISAPPEAR).
/// 0120 <id>.L
static void clif_skill_delunit(struct skill_unit *su)
{
	unsigned char buf[16];

	nullpo_retv(su);

	WBUFW(buf, 0)=0x120;
	WBUFL(buf, 2)=su->bl.id;
	clif->send(buf,packet_len(0x120),&su->bl,AREA);
}

/// Sent when an object gets ankle-snared (ZC_SKILL_UPDATE).
/// 01ac <id>.L
/// Only affects units with class [139,153] client-side.
static void clif_skillunit_update(struct block_list *bl)
{
	unsigned char buf[6];
	nullpo_retv(bl);

	WBUFW(buf,0) = 0x1ac;
	WBUFL(buf,2) = bl->id;

	clif->send(buf,packet_len(0x1ac),bl,AREA);
}

/*==========================================
 *
 *------------------------------------------*/
static int clif_getareachar(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;

	nullpo_ret(bl);

	sd=va_arg(ap,struct map_session_data*);

	if (sd == NULL || !sd->fd)
		return 0;

	switch(bl->type){
		case BL_ITEM:
			clif->getareachar_item(sd, BL_UCAST(BL_ITEM, bl));
			break;
		case BL_SKILL:
			clif->getareachar_skillunit(&sd->bl, BL_UCAST(BL_SKILL, bl), SELF);
			break;
		default:
			if(&sd->bl == bl)
				break;
			clif->getareachar_unit(sd,bl);
			break;
	}
	return 0;
}

/*==========================================
 * tbl has gone out of view-size of bl
 *------------------------------------------*/
static int clif_outsight(struct block_list *bl, va_list ap)
{
	struct block_list *tbl;
	struct view_data *vd;
	struct map_session_data *sd, *tsd;
	tbl=va_arg(ap,struct block_list*);
	if(bl == tbl) return 0;
	// bl can be null pointer? and after if BL_PC, sd will be null pointer too
	sd = BL_CAST(BL_PC, bl);
	tsd = BL_CAST(BL_PC, tbl);

	if (tsd && tsd->fd) { //tsd has lost sight of the bl object.
		nullpo_ret(bl);
		switch(bl->type){
			case BL_PC:
				if (sd->vd.class != INVISIBLE_CLASS)
					clif->clearunit_single(bl->id,CLR_OUTSIGHT,tsd->fd);
				if (sd->chat_id != 0) {
					struct chat_data *cd = map->id2cd(sd->chat_id);
					if (cd != NULL && cd->usersd[0] == sd)
						clif->dispchat(cd,tsd->fd);
				}
				if( sd->state.vending )
					clif->closevendingboard(bl,tsd->fd);
				if( sd->state.buyingstore )
					clif->buyingstore_disappear_entry_single(tsd, sd);
				break;
			case BL_ITEM:
				clif->clearflooritem(BL_UCAST(BL_ITEM, bl), tsd->fd);
				break;
			case BL_SKILL:
				clif->clearchar_skillunit(BL_UCAST(BL_SKILL, bl), tsd->fd);
				break;
			case BL_NPC:
				if (!(BL_UCAST(BL_NPC, bl)->option&OPTION_INVISIBLE))
					clif->clearunit_single(bl->id,CLR_OUTSIGHT,tsd->fd);
				break;
			default:
				if ((vd=status->get_viewdata(bl)) && vd->class != INVISIBLE_CLASS)
					clif->clearunit_single(bl->id,CLR_OUTSIGHT,tsd->fd);
				break;
			}
	}
	if (sd && sd->fd) { //sd is watching tbl go out of view.
		nullpo_ret(tbl);
		if (tbl->type == BL_SKILL) //Trap knocked out of sight
			clif->clearchar_skillunit(BL_UCAST(BL_SKILL, tbl), sd->fd);
		else if ((vd = status->get_viewdata(tbl)) != NULL && vd->class != INVISIBLE_CLASS
		      && !(tbl->type == BL_NPC && (BL_UCAST(BL_NPC, tbl)->option&OPTION_INVISIBLE)))
			clif->clearunit_single(tbl->id,CLR_OUTSIGHT,sd->fd);
	}
	return 0;
}

/*==========================================
 * tbl has come into view of bl
 *------------------------------------------*/
static int clif_insight(struct block_list *bl, va_list ap)
{
	struct block_list *tbl;
	struct map_session_data *sd, *tsd;
	tbl=va_arg(ap,struct block_list*);

	if (bl == tbl) return 0;

	sd = BL_CAST(BL_PC, bl);
	tsd = BL_CAST(BL_PC, tbl);

	if (tsd && tsd->fd) { //Tell tsd that bl entered into his view
		nullpo_ret(bl);
		switch(bl->type) {
			case BL_ITEM:
				clif->getareachar_item(tsd, BL_UCAST(BL_ITEM, bl));
				break;
			case BL_SKILL:
				clif->getareachar_skillunit(&tsd->bl, BL_UCAST(BL_SKILL, bl), SELF);
				break;
			default:
				clif->getareachar_unit(tsd,bl);
				break;
		}
	}
	if (sd && sd->fd) { //Tell sd that tbl walked into his view
		clif->getareachar_unit(sd,tbl);
	}
	return 0;
}

static void clif_playerSkillToPacket(struct map_session_data *sd, struct SKILLDATA *skillData, int skillId, int idx, bool newSkill)
{
	nullpo_retv(sd);
	nullpo_retv(skillData);
	Assert_retv(idx >= 0 && idx < MAX_SKILL_DB);

	int skill_lv = sd->status.skill[idx].lv;
	skillData->id = skillId;
	skillData->inf = skill->get_inf(skillId);
	skillData->level = skill_lv;
	if (skill_lv > 0) {
		skillData->sp = skill->get_sp(skillId, skill_lv);
		skillData->range2 = skill->get_range2(&sd->bl, skillId, skill_lv);
	} else {
		skillData->sp = 0;
		skillData->range2 = 0;
	}
#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
	if (newSkill)
		skillData->level2 = 0;
	else
		skillData->level2 = skill_lv;
#else
	safestrncpy(skillData->name, skill->get_name(skillId), NAME_LENGTH);
#endif
	if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
		skillData->upFlag = (skill_lv < skill->tree_get_max(skillId, sd->status.class)) ? 1 : 0;
	else
		skillData->upFlag = 0;
}

/// Updates whole skill tree (ZC_SKILLINFO_LIST).
/// 010f <packet len>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradable>.B }*
static void clif_skillinfoblock(struct map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	if (!fd)
		return;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_SKILLINFO_LIST) + MAX_SKILL_DB * sizeof(struct SKILLDATA));
	struct PACKET_ZC_SKILLINFO_LIST *p = WFIFOP(fd, 0);

	p->packetType = HEADER_ZC_SKILLINFO_LIST;
	int skillIndex = 0;
	int len = sizeof(struct PACKET_ZC_SKILLINFO_LIST);
	int i;
	for (i = 0; i < MAX_SKILL_DB; i++) {
		int id = sd->status.skill[i].id;
		if (id != 0) {
			// workaround for bugreport:5348
			if (len + sizeof(struct SKILLDATA) > 8192)
				break;

			clif->playerSkillToPacket(sd, &p->skills[skillIndex], id, i, false);
			len += sizeof(struct SKILLDATA);
			skillIndex++;
		}
	}
	p->packetLength = len;
	WFIFOSET(fd, len);

	// workaround for bugreport:5348; send the remaining skills one by one to bypass packet size limit
	for (; i < MAX_SKILL_DB; i++) {
		int id = sd->status.skill[i].id;
		if (id != 0) {
			clif->addskill(sd, id);
			clif->skillinfo(sd, id, 0);
		}
	}
}

/**
 * Server tells client 'sd' to add skill of id 'id' to it's skill tree (e.g. with Ice Falcion item)
 **/

/// Adds new skill to the skill tree (ZC_ADD_SKILL).
/// 0111 <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradable>.B
static void clif_addskill(struct map_session_data *sd, int id)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	if (!fd)
		return;

	int idx = skill->get_index(id);
	if (sd->status.skill[idx].id <= 0)
		return;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_ADD_SKILL));
	struct PACKET_ZC_ADD_SKILL *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_ADD_SKILL;
	clif->playerSkillToPacket(sd, &p->skill, id, idx, true);
	WFIFOSET(fd, sizeof(struct PACKET_ZC_ADD_SKILL));
}

/// Deletes a skill from the skill tree (ZC_SKILLINFO_DELETE).
/// 0441 <skill id>.W
static void clif_deleteskill(struct map_session_data *sd, int id)
{
#if PACKETVER >= 20081217
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	if( !fd ) return;

	WFIFOHEAD(fd,packet_len(0x441));
	WFIFOW(fd,0) = 0x441;
	WFIFOW(fd,2) = id;
	WFIFOSET(fd,packet_len(0x441));
#endif
	clif->skillinfoblock(sd);
}

/// Updates a skill in the skill tree (ZC_SKILLINFO_UPDATE).
/// 010e <skill id>.W <level>.W <sp cost>.W <attack range>.W <upgradable>.B
/// Merged clif_skillup and clif_guild_skillup, same packet was used [panikon]
/// flag:
///  0: guild call
///  1: player call
static void clif_skillup(struct map_session_data *sd, uint16 skill_id, int skill_lv, int flag)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x10e));
	WFIFOW(fd, 0) = 0x10e;
	WFIFOW(fd, 2) = skill_id;
	WFIFOW(fd, 4) = skill_lv;
	WFIFOW(fd, 6) = skill->get_sp(skill_id, skill_lv);
	WFIFOW(fd, 8) = (flag)?skill->get_range2(&sd->bl, skill_id, skill_lv) : skill->get_range(skill_id, skill_lv);
	if( flag )
		WFIFOB(fd,10) = (skill_lv < skill->tree_get_max(skill_id, sd->status.class)) ? 1 : 0;
	else
		WFIFOB(fd,10) = 1;

	WFIFOSET(fd, packet_len(0x10e));
}

/// Updates a skill in the skill tree (ZC_SKILLINFO_UPDATE2).
/// 07e1 <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <upgradable>.B
static void clif_skillinfo(struct map_session_data *sd, int skill_id, int inf)
{
	nullpo_retv(sd);

	const int fd = sd->fd;
	int idx = skill->get_index(skill_id);
	Assert_retv(idx >= 0 && idx < MAX_SKILL_DB);

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_SKILLINFO_UPDATE2));
	struct PACKET_ZC_SKILLINFO_UPDATE2 *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_SKILLINFO_UPDATE2;
	int skill_lv = sd->status.skill[idx].lv;
	p->id = skill_id;
	p->inf = inf?inf:skill->get_inf(skill_id);
	p->level = skill_lv;
	if (skill_lv > 0) {
		p->sp = skill->get_sp(skill_id, skill_lv);
		p->range2 = skill->get_range2(&sd->bl, skill_id, skill_lv);
	} else {
		p->sp = 0;
		p->range2 = 0;
	}
#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
	p->level2 = skill_lv;
#endif
	if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
		p->upFlag = (skill_lv < skill->tree_get_max(skill_id, sd->status.class)) ? 1 : 0;
	else
		p->upFlag = 0;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_SKILLINFO_UPDATE2));
}

/// Notifies clients in area, that an object is about to use a skill.
/// 013e <src id>.L <dst id>.L <x>.W <y>.W <skill id>.W <property>.L <delaytime>.L (ZC_USESKILL_ACK)
/// 07fb <src id>.L <dst id>.L <x>.W <y>.W <skill id>.W <property>.L <delaytime>.L <is disposable>.B (ZC_USESKILL_ACK2)
/// property:
///     0 = Yellow cast aura
///     1 = Water elemental cast aura
///     2 = Earth elemental cast aura
///     3 = Fire elemental cast aura
///     4 = Wind elemental cast aura
///     5 = Poison elemental cast aura
///     6 = Holy elemental cast aura
///     ? = like 0
/// is disposable:
///     0 = yellow chat text "[src name] will use skill [skill name]."
///     1 = no text
static void clif_useskill(struct block_list *bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, uint16 skill_lv, int casttime)
{
	nullpo_retv(bl);

	const int element = skill->get_ele(skill_id, skill_lv);
	struct PACKET_ZC_USESKILL_ACK p;
	p.packetType = HEADER_ZC_USESKILL_ACK;
	p.srcId = src_id;
	p.dstId = dst_id;
	p.x = dst_x;
	p.y = dst_y;
	p.skillId = skill_id;
	p.element = element < 0 ? 0 : element; //Avoid sending negatives as element [Skotlex]
	p.delayTime = casttime;
#if PACKETVER_MAIN_NUM >= 20091124 || PACKETVER_RE_NUM >= 20091124 || defined(PACKETVER_ZERO)
	p.disposable = 0;
#endif
#if PACKETVER_MAIN_NUM >= 20181212 || PACKETVER_RE_NUM >= 20181212 || PACKETVER_ZERO_NUM >= 20190130
	p.unknown = 0;
#endif

	if (clif->isdisguised(bl)) {
		clif->send(&p, sizeof(p), bl, AREA_WOS);
		p.srcId = -src_id;
		clif->send(&p, sizeof(p), bl, SELF);
	} else {
		clif->send(&p, sizeof(p), bl, AREA);
	}
#if PACKETVER >= 20151223
	if ((skill->get_inf2(skill_id) & INF2_SHOW_SKILL_SCALE) != 0)
		clif->skill_scale(bl, src_id, bl->x, bl->y, skill_id, skill_lv, casttime);
#endif
}

/// Notifies clients in area, that an object canceled casting (ZC_DISPEL).
/// 01b9 <id>.L
static void clif_skillcastcancel(struct block_list *bl)
{
	unsigned char buf[16];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x1b9;
	WBUFL(buf,2) = bl->id;
	clif->send(buf,packet_len(0x1b9), bl, AREA);
}

/// Notifies the client about the result of a skill use request (ZC_ACK_TOUSESKILL).
/// 0110 <skill id>.W <num>.L <result>.B <cause>.B
/// num (only used when skill id = NV_BASIC and cause = 0):
///     0 = "skill failed" MsgStringTable[159]
///     1 = "no emotions" MsgStringTable[160]
///     2 = "no sit" MsgStringTable[161]
///     3 = "no chat" MsgStringTable[162]
///     4 = "no party" MsgStringTable[163]
///     5 = "no shout" MsgStringTable[164]
///     6 = "no PKing" MsgStringTable[165]
///     7 = "no aligning" MsgStringTable[383]
///     ? = ignored
/// cause:
///     0 = "not enough skill level" MsgStringTable[214] (AL_WARP)
///         "steal failed" MsgStringTable[205] (TF_STEAL)
///         "envenom failed" MsgStringTable[207] (TF_POISON)
///         "skill failed" MsgStringTable[204] (otherwise)
///   ... = @see enum useskill_fail_cause
///     ? = ignored
///
/// if(result!=0) doesn't display any of the previous messages
/// Note: when this packet is received an unknown flag is always set to 0,
/// suggesting this is an ACK packet for the UseSkill packets and should be sent on success too [FlavioJS]
static void clif_skill_fail(struct map_session_data *sd, uint16 skill_id, enum useskill_fail_cause cause, int btype, int32 item_id)
{
	int fd;
	struct PACKET_ZC_ACK_TOUSESKILL p;

	if (!sd) {
		//Since this is the most common nullpo....
		ShowDebug("clif_skill_fail: Error, received NULL sd for skill %d\n", skill_id);
		return;
	}

	fd = sd->fd;
	if (!fd) return;

	if (battle_config.display_skill_fail&1)
		return; //Disable all skill failed messages

	if (cause == USESKILL_FAIL_SKILLINTERVAL && !sd->state.showdelay)
		return; //Disable delay failed messages

	if (skill_id == RG_SNATCHER && battle_config.display_skill_fail & 4)
		return;

	if (skill_id == TF_POISON && battle_config.display_skill_fail & 8)
		return;

	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x110;
	p.skillId = skill_id;
	p.btype = btype;
	p.itemId = item_id;
	p.flag = 0; // 0 - failed
	p.cause = cause;

	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Skill cooldown display icon (ZC_SKILL_POSTDELAY).
/// 043d <skill ID>.W <tick>.L
static void clif_skill_cooldown(struct map_session_data *sd, uint16 skill_id, unsigned int duration)
{
#if PACKETVER_MAIN_NUM >= 20081112 || PACKETVER_RE_NUM >= 20081111 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	int fd = sd->fd;
	struct PACKET_ZC_SKILL_POSTDELAY p = { 0 };

	WFIFOHEAD(fd, sizeof(p));
	p.PacketType = HEADER_ZC_SKILL_POSTDELAY;
	p.SKID = skill_id;
	p.DelayTM = duration;

	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
#endif
}

/// Skill attack effect and damage.
/// 0114 <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <damage>.W <level>.W <div>.W <type>.B (ZC_NOTIFY_SKILL)
/// 01de <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <damage>.L <level>.W <div>.W <type>.B (ZC_NOTIFY_SKILL2)
static int clif_skill_damage(struct block_list *src, struct block_list *dst, int64 tick, int sdelay, int ddelay, int64 in_damage, int div, uint16 skill_id, uint16 skill_lv, enum battle_dmg_type type)
{
	nullpo_ret(src);
	nullpo_ret(dst);

	int damage = (int)cap_value(in_damage, INT_MIN, INT_MAX);
	type = clif_calc_delay(type, div, damage, ddelay);

#if PACKETVER >= 20131223
	// For some reason, late 2013 and newer clients have
	// a issue that causes players and monsters to endure
	// type 6 (ACTION_SKILL) skills. So we have to do a small
	// hack to set all type 6 to be sent as type 8 ACTION_ATTACK_MULTIPLE
	if (type == BDT_SKILL)
		type = BDT_MULTIHIT; //bugreport:8263
#endif

	const struct status_change *sc = status->get_sc(dst);
	if (sc != NULL && sc->count) {
		if (sc->data[SC_ILLUSION] != NULL && damage != 0)
			damage = damage * (sc->data[SC_ILLUSION]->val2) + rnd() % 100;
	}

	struct PACKET_ZC_NOTIFY_SKILL p = { 0 };
	p.PacketType = HEADER_ZC_NOTIFY_SKILL;
	p.SKID = skill_id;
	p.AID = src->id;
	p.targetID = dst->id;
	p.startTime = (uint32)tick;
	p.attackMT = sdelay;
	p.attackedMT = ddelay;
	if (battle_config.hide_woe_damage && map_flag_gvg2(src->m)) {
		p.damage = damage ? div : 0;
	} else {
		p.damage = damage;
	}
	p.level = skill_lv;
	p.count = div;
	p.action = type;

	if (clif->isdisguised(dst)) {
		clif->send(&p, sizeof(p), dst, AREA_WOS);
		p.targetID = -dst->id;
		clif->send(&p, sizeof(p), dst, SELF);
	} else {
		clif->send(&p, sizeof(p), dst, AREA);
	}

	if (clif->isdisguised(src)) {
		p.AID = -src->id;
		if (clif->isdisguised(dst))
			p.targetID = dst->id;
		if (damage > 0)
			p.damage = -1;
		clif->send(&p, sizeof(p), src, SELF);
	}

	//Because the damage delay must be synced with the client, here is where the can-walk tick must be updated. [Skotlex]
	return clif->calc_walkdelay(dst, ddelay, type, damage, div);
}

/// Ground skill attack effect and damage (ZC_NOTIFY_SKILL_POSITION).
/// 0115 <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <x>.W <y>.W <damage>.W <level>.W <div>.W <type>.B
#if 0
static int clif_skill_damage2(struct block_list *src, struct block_list *dst, int64 tick, int sdelay, int ddelay, int damage, int div, uint16 skill_id, uint16 skill_lv, enum battle_dmg_type type)
{
	nullpo_ret(src);
	nullpo_ret(dst);

	type = (type > 0) ? type : skill->get_hit(skill_id, skill_lv);
	type = clif_calc_delay(type, div, damage, ddelay);

	const struct status_change *sc = status->get_sc(dst);
	if (sc != NULL && sc->count) {
		if (sc->data[SC_ILLUSION] != NULL && damage != 0)
			damage = damage * (sc->data[SC_ILLUSION]->val2) + rnd() % 100;
	}

	struct PACKET_ZC_NOTIFY_SKILL_POSITION p = { 0 };
	p.PacketType = HEADER_ZC_NOTIFY_SKILL_POSITION;
	p.SKID = skill_id;
	p.AID = src->id;
	p.targetID = dst->id;
	p.startTime = (uint32)tick;
	p.attackMT = sdelay;
	p.attackedMT = ddelay;
	p.xPos = dst->x;
	p.yPos = dst->y;
	if (battle_config.hide_woe_damage && map_flag_gvg(src->m)) {
		p.damage = damage ? div : 0;
	} else {
		p.damage = damage;
	}
	p.level = skill_lv;
	p.count = div;
	p.action = type;
	clif->send(&p, sizeof(p), src, AREA);
	if (clif->isdisguised(src)) {
		p.AID = -src->id;
		if (damage > 0)
			p.damage = -1;
		clif->send(&p, sizeof(p), src, SELF);
	}
	if (clif->isdisguised(dst)) {
		p.targetID = -dst->id;
		if (clif->isdisguised(src))
			p.AID = src->id;
		else if (damage > 0)
			p.damage = -1;
		clif->send(&p, sizeof(p), dst, SELF);
	}

	//Because the damage delay must be synced with the client, here is where the can-walk tick must be updated. [Skotlex]
	return clif->calc_walkdelay(dst,ddelay,type,damage,div);
}
#endif // 0

/// Non-damaging skill effect.
/// 011a <skill id>.W <skill lv>.W <dst id>.L <src id>.L <result>.B (ZC_USE_SKILL)
/// 09cb <skill id>.W <skill lv>.L <dst id>.L <src id>.L <result>.B (ZC_USE_SKILL2)
static int clif_skill_nodamage(struct block_list *src, struct block_list *dst, uint16 skill_id, int heal, int fail)
{
	nullpo_ret(dst);

	struct PACKET_ZC_USE_SKILL p = { 0 };
	p.PacketType = HEADER_ZC_USE_SKILL;
	p.SKID = skill_id;
#if PACKETVER_MAIN_NUM >= 20130731 || PACKETVER_RE_NUM >= 20130724 || defined(PACKETVER_ZERO)
	p.level = min(heal, INT_MAX);
#else
	p.level = min(heal, INT16_MAX);
#endif
	p.targetAID = dst->id;
	p.srcAID = src ? src->id : 0;
	p.result = fail;

	if (clif->isdisguised(dst)) {
		clif->send(&p, sizeof(p), dst, AREA_WOS);
		p.targetAID = -dst->id;
		clif->send(&p, sizeof(p), dst, SELF);
	} else
		clif->send(&p, sizeof(p), dst, AREA);

	if (src && clif->isdisguised(src)) {
		p.srcAID = -src->id;
		if (clif->isdisguised(dst))
			p.targetAID = dst->id;
		clif->send(&p, sizeof(p), src, SELF);
	}

	return fail;
}

/// Non-damaging ground skill effect (ZC_NOTIFY_GROUNDSKILL).
/// 0117 <skill id>.W <src id>.L <level>.W <x>.W <y>.W <tick>.L
static void clif_skill_poseffect(struct block_list *src, uint16 skill_id, int val, int x, int y, int64 tick)
{
	nullpo_retv(src);

	struct PACKET_ZC_NOTIFY_GROUNDSKILL p = { 0 };
	p.PacketType = HEADER_ZC_NOTIFY_GROUNDSKILL;
	p.SKID = skill_id;
	p.AID = src->id;
	p.level = val;
	p.xPos = x;
	p.yPos = y;
	p.startTime = (uint32)tick;
	if (clif->isdisguised(src)) {
		clif->send(&p, sizeof(p), src, AREA_WOS);
		p.AID = -src->id;
		clif->send(&p, sizeof(p), src, SELF);
	} else {
		clif->send(&p, sizeof(p), src, AREA);
	}
}

/// Presents a list of available warp destinations (ZC_WARPLIST).
/// 011c <skill id>.W { <map name>.16B }*4
static void clif_skill_warppoint(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4)
{
	int fd;
	int len;
	int mapsCount = 0;
	struct PACKET_ZC_WARPLIST *p;

	nullpo_retv(sd);
	fd = sd->fd;
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	len = sizeof(struct PACKET_ZC_WARPLIST) + sizeof(struct PACKET_ZC_WARPLIST_sub) * 6;
#else
	len = sizeof(struct PACKET_ZC_WARPLIST);
#endif

	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	memset(p, 0, len);
	p->packetType = skilWarpPointType;
	p->skillId = skill_id;
	if (map1 == (unsigned short)-1) {
		strcpy(p->maps[mapsCount++].map, "Random");
	} else { // normal map name
		if (map1 > 0) mapindex->getmapname_ext(mapindex_id2name(map1), p->maps[mapsCount++].map);
	}
	if (map2 > 0) mapindex->getmapname_ext(mapindex_id2name(map2), p->maps[mapsCount++].map);
	if (map3 > 0) mapindex->getmapname_ext(mapindex_id2name(map3), p->maps[mapsCount++].map);
	if (map4 > 0) mapindex->getmapname_ext(mapindex_id2name(map4), p->maps[mapsCount++].map);

#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	len = sizeof(struct PACKET_ZC_WARPLIST) + sizeof(struct PACKET_ZC_WARPLIST_sub) * mapsCount;
	p->packetLength = len;
#endif

	WFIFOSET(fd, len);

	sd->menuskill_id = skill_id;
	if (skill_id == AL_WARP) {
		sd->menuskill_val = (sd->ud.skillx << 16) | sd->ud.skilly; //Store warp position here.
		sd->state.workinprogress = 3;
	} else {
		sd->menuskill_val = skill_lv;
	}
}

/// Memo message (ZC_ACK_REMEMBER_WARPPOINT).
/// 011e <type>.B
/// type:
///     0 = "Saved location as a Memo Point for Warp skill." in color 0xFFFF00 (cyan)
///     1 = "Skill Level is not high enough." in color 0x0000FF (red)
///     2 = "You haven't learned Warp." in color 0x0000FF (red)
///     3 = "Cannot save location as a Memo Point at current location." in color 0x0000FF (red)
///
/// @param sd Who receives the message
/// @param type What message
static void clif_skill_memomessage(struct map_session_data *sd, int type)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x11e));
	WFIFOW(fd,0)=0x11e;
	WFIFOB(fd,2)=type;
	WFIFOSET(fd,packet_len(0x11e));
}

/// Teleport message (ZC_NOTIFY_MAPINFO).
/// 0189 <type>.W
/// type:
///     0 = "Unable to Teleport in this area" in color 0xFFFF00 (cyan)
///     1 = "Saved point cannot be memorized." in color 0x0000FF (red)
///     2 = "This skill cannot be used within this area." in color 0xFFFF00 (cyan)
///     3 = "This item cannot be used within this area." in color 0xFFFF00 (cyan)
///
/// @param sd Who receives the message
/// @param type What message
static void clif_skill_mapinfomessage(struct map_session_data *sd, int type)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x189));
	WFIFOW(fd,0)=0x189;
	WFIFOW(fd,2)=type;
	WFIFOSET(fd,packet_len(0x189));
}

/// Displays Sense (WZ_ESTIMATION) information window (ZC_MONSTER_INFO).
/// 018c <class>.W <level>.W <size>.W <hp>.L <def>.W <race>.W <mdef>.W <element>.W
///     <water%>.B <earth%>.B <fire%>.B <wind%>.B <poison%>.B <holy%>.B <shadow%>.B <ghost%>.B <undead%>.B
static void clif_skill_estimation(struct map_session_data *sd, struct block_list *dst)
{
	struct status_data *dstatus;
	unsigned char buf[64];
	int i, fix;

	nullpo_retv(sd);
	nullpo_retv(dst);

	if( dst->type != BL_MOB )
		return;

	dstatus = status->get_status_data(dst);

	WBUFW(buf, 0) = 0x18c;
	WBUFW(buf, 2) = status->get_class(dst);
	WBUFW(buf, 4) = status->get_lv(dst);
	WBUFW(buf, 6) = dstatus->size;
	WBUFL(buf, 8) = dstatus->hp;
	WBUFW(buf,12) = ((battle_config.estimation_type&1) ? dstatus->def : 0)
	              + ((battle_config.estimation_type&2) ? dstatus->def2 : 0);
	WBUFW(buf,14) = dstatus->race;
	WBUFW(buf,16) = ((battle_config.estimation_type&1) ? dstatus->mdef : 0)
	              + ((battle_config.estimation_type&2) ? dstatus->mdef2 : 0);
	WBUFW(buf,18) = dstatus->def_ele;
	for(i=0;i<9;i++) {
		// WBUFB(buf,20+i)= (unsigned char)battle->attr_ratio(i+1,dstatus->def_ele, dstatus->ele_lv);
		// The following caps negative attributes to 0 since the client displays them as 255-fix. [Skotlex]
		WBUFB(buf,20+i)= (unsigned char)((fix=battle->attr_ratio(i+1,dstatus->def_ele, dstatus->ele_lv))<0?0:fix);
	}

	clif->send(buf,packet_len(0x18c),&sd->bl,sd->status.party_id>0?PARTY_SAMEMAP:SELF);
}

/// Presents a textual list of producible items (ZC_MAKABLEITEMLIST).
/// 018d <packet len>.W { <name id>.W { <material id>.W }*3 }*
/// material id:
///     unused by the client
static void clif_skill_produce_mix_list(struct map_session_data *sd, int skill_id, int trigger)
{
	int i, c, view, fd;
	int len;
	struct PACKET_ZC_MAKABLEITEMLIST *p;

	nullpo_retv(sd);

	if (sd->menuskill_id == skill_id)
		return; //Avoid resending the menu twice or more times...
	if (skill_id == GC_CREATENEWPOISON)
		skill_id = GC_RESEARCHNEWPOISON;

	fd = sd->fd;
	len = MAX_SKILL_PRODUCE_DB * sizeof(struct PACKET_ZC_MAKABLEITEMLIST_sub) + sizeof(struct PACKET_ZC_MAKABLEITEMLIST);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x18d;

	for (i = 0, c = 0; i < MAX_SKILL_PRODUCE_DB; i++) {
		if (skill->can_produce_mix(sd, skill->dbs->produce_db[i].nameid, trigger, 1) &&
		    ((skill_id > 0 && skill->dbs->produce_db[i].req_skill == skill_id) || skill_id < 0)) {
			if ((view = itemdb_viewid(skill->dbs->produce_db[i].nameid)) > 0)
				p->items[c].itemId = view;
			else
				p->items[c].itemId = skill->dbs->produce_db[i].nameid;
			p->items[c].material[0] = 0;
			p->items[c].material[1] = 0;
			p->items[c].material[2] = 0;
			c++;
		}
	}
	len = c * sizeof(struct PACKET_ZC_MAKABLEITEMLIST_sub) + sizeof(struct PACKET_ZC_MAKABLEITEMLIST);
	p->packetLength = len;
	WFIFOSET(fd, len);
	if (c > 0) {
		sd->menuskill_id = skill_id;
		sd->menuskill_val = trigger;
		return;
	}
}

/// Present a list of producible items (ZC_MAKINGITEM_LIST).
/// 025a <packet len>.W <mk type>.W { <name id>.W }*
/// mk type:
///     1 = cooking
///     2 = arrow
///     3 = elemental
///     4 = GN_MIX_COOKING
///     5 = GN_MAKEBOMB
///     6 = GN_S_PHARMACY
static void clif_cooking_list(struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type)
{
	int fd;
	int i, c;
	int view;
	int len;
	struct PACKET_ZC_MAKINGITEM_LIST *p;

	nullpo_retv(sd);
	fd = sd->fd;

	len = sizeof(struct PACKET_ZC_MAKINGITEM_LIST) + MAX_SKILL_PRODUCE_DB * sizeof(struct PACKET_ZC_MAKINGITEM_LIST_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGITEM_LIST;
	p->makeItem = list_type; // list type

	c = 0;
	for (i = 0; i < MAX_SKILL_PRODUCE_DB; i++) {
		if (!skill->can_produce_mix(sd,skill->dbs->produce_db[i].nameid,trigger, qty))
			continue;

		if ((view = itemdb_viewid(skill->dbs->produce_db[i].nameid)) > 0)
			p->items[c].itemId = view;
		else
			p->items[c].itemId = skill->dbs->produce_db[i].nameid;

		c++;
	}

	len = sizeof(struct PACKET_ZC_MAKINGITEM_LIST) + c * sizeof(struct PACKET_ZC_MAKINGITEM_LIST_sub);
	p->packetLength = len;
	if (skill_id == AM_PHARMACY) {
		// Only send it while Cooking else check for c.
		WFIFOSET(fd, len);
	}

	if (c > 0) {
		sd->menuskill_id = skill_id;
		sd->menuskill_val = trigger;
		if (skill_id != AM_PHARMACY) {
			sd->menuskill_val2 = qty; // amount.
			WFIFOSET(fd, len);
		}
	} else {
		clif_menuskill_clear(sd);
		if (skill_id != AM_PHARMACY) { // AM_PHARMACY is used to Cooking.
			// It fails.
#if PACKETVER >= 20091013
			clif->msgtable_skill(sd, skill_id, MSG_SKILL_MATERIAL_FAIL);
#else
			WFIFOSET(fd, len);
#endif
		}
	}
}

static void clif_status_change_notick(struct block_list *bl, int type, int relevant_bl, int flag, int tick, int total_tick, int val1, int val2, int val3)
{
	struct packet_sc_notick p;
	struct map_session_data *sd;

	nullpo_retv(bl);

	if (type == SI_BLANK)  //It shows nothing on the client...
		return;

	if (!(relevant_bl & bl->type)) // only send status changes that actually matter to the client
		return;

	sd = BL_CAST(BL_PC, bl);

	p.PacketType = sc_notickType;
	p.index = type;
	p.AID = bl->id;
	p.state = (unsigned char)flag;

	clif->send(&p,packet_len(p.PacketType), bl, (sd && sd->status.option&OPTION_INVISIBLE) ? SELF : AREA);
}

/// Notifies clients of a status change.
/// 0196 <index>.W <id>.L <state>.B (ZC_MSG_STATE_CHANGE) [used for ending status changes and starting them on non-pc units (when needed)]
/// 043f <index>.W <id>.L <state>.B <remain msec>.L { <val>.L }*3 (ZC_MSG_STATE_CHANGE2) [used exclusively for starting statuses on pcs]
/// 08ff <id>.L <index>.W <remain msec>.L { <val>.L }*3  (PACKETVER >= 20111108)
/// 0983 <index>.W <id>.L <state>.B <total msec>.L <remain msec>.L { <val>.L }*3 (PACKETVER >= 20120618)
/// 0984 <id>.L <index>.W <total msec>.L <remain msec>.L { <val>.L }*3 (PACKETVER >= 20120618)
static void clif_status_change_sub(struct block_list *bl, int type, int relevant_bl, int flag, int tick, int total_tick, int val1, int val2, int val3)
{
	struct packet_status_change p;
	struct map_session_data *sd;

	if (type == SI_BLANK)  //It shows nothing on the client...
		return;

	nullpo_retv(bl);

	if (!(relevant_bl & bl->type)) // only send status changes that actually matter to the client
		return;

	if ( tick < 0 )
		tick = 9999;

	sd = BL_CAST(BL_PC, bl);

	p.PacketType = status_changeType;
	p.index = type;
	p.AID = bl->id;
	p.state = (unsigned char)flag;

#if PACKETVER >= 20120618
	p.Total = total_tick;
#endif
#if PACKETVER >= 20090121
	p.Left = tick;
	p.val1 = val1;
	p.val2 = val2;
	p.val3 = val3;
#endif
	clif->send(&p,sizeof(p), bl, (sd && sd->status.option&OPTION_INVISIBLE) ? SELF : AREA);
}

/// Notifies clients of a status change.
/// @see clif_status_change_sub
static void clif_status_change(struct block_list *bl, int type, int relevant_bl, int flag, int total_tick, int val1, int val2, int val3)
{
	clif->status_change_sub(bl, type, relevant_bl, flag, total_tick, total_tick, val1, val2, val3);
}

/// Send message (modified by [Yor]) (ZC_NOTIFY_PLAYERCHAT).
/// 008e <packet len>.W <message>.?B
static void clif_displaymessage(const int fd, const char *mes)
{
	nullpo_retv(mes);

	if (map->cpsd_active && fd == 0) {
		ShowInfo("HCP: %s\n",mes);
	} else if (fd > 0) {
	#if PACKETVER == 20141022
		/** for some reason game client crashes depending on message pattern (only for this packet) **/
		/** so we redirect to ZC_NPC_CHAT **/
		clif->messagecolor_self(fd, COLOR_DEFAULT, mes);
	#else
		int len = (int)strnlen(mes, 255);

		if (len > 0) { // don't send a void message (it's not displaying on the client chat). @help can send void line.
			WFIFOHEAD(fd, 5 + len);
			WFIFOW(fd,0) = 0x8e;
			WFIFOW(fd,2) = 5 + len; // 4 + len + NUL terminate
			safestrncpy(WFIFOP(fd,4), mes, len + 1);
			WFIFOSET(fd, 5 + len);
		}
	#endif
	}
}

static void clif_displaymessage2(const int fd, const char *mes)
{
	nullpo_retv(mes);

	//Scrapped, as these are shared by disconnected players =X [Skotlex]
	if (fd == 0 && !map->cpsd_active)
		;
	else {
		// Limit message to 255+1 characters (otherwise it causes a buffer overflow in the client)
		char *message, *line;

		message = aStrdup(mes);
		line = strtok(message, "\n");
		while(line != NULL) {
			// Limit message to 255+1 characters (otherwise it causes a buffer overflow in the client)
			int len = (int)strnlen(line, 255);

			if (len > 0) { // don't send a void message (it's not displaying on the client chat). @help can send void line.
				if( map->cpsd_active && fd == 0 ) {
					ShowInfo("HCP: %s\n",line);
				} else {
					WFIFOHEAD(fd, 5 + len);
					WFIFOW(fd,0) = 0x8e;
					WFIFOW(fd,2) = 5 + len; // 4 + len + NULL terminate
					safestrncpy(WFIFOP(fd,4), line, len + 1);
					WFIFOSET(fd, 5 + len);
				}
			}
			line = strtok(NULL, "\n");
		}
		aFree(message);
	}
}

/* oh noo! another version of 0x8e! */
static void clif_displaymessage_sprintf(const int fd, const char *mes, ...) __attribute__((format(printf, 2, 3)));
static void clif_displaymessage_sprintf(const int fd, const char *mes, ...)
{
	va_list ap;

	nullpo_retv(mes);
	if (map->cpsd_active && fd == 0) {
		ShowInfo("HCP: ");
		va_start(ap,mes);
		vShowMessage(mes,ap);
		va_end(ap);
		ShowMessage("\n");
	} else if (fd > 0) {
		int len = 1;
		char *ptr;

		WFIFOHEAD(fd, 5 + 255);/* ensure the maximum */

		/* process */
		va_start(ap,mes);
		len += vsnprintf(WFIFOP(fd,4), 255, mes, ap);
		va_end(ap);

		/* adjusting */
		ptr = WFIFOP(fd,4);
		ptr[len - 1] = '\0';

		/* */
		WFIFOW(fd,0) = 0x8e;
		WFIFOW(fd,2) = 5 + len; // 4 + len + NULL terminate

		WFIFOSET(fd, 5 + len);
	}
}

/// Send broadcast message in yellow or blue without font formatting (ZC_BROADCAST).
/// 009a <packet len>.W <message>.?B
static void clif_broadcast(struct block_list *bl, const char *mes, int len, int type, enum send_target target)
{
	int lp = ((type & BC_COLOR_MASK) != 0 || (type & BC_MEGAPHONE) != 0) ? 4 : 0;
	unsigned char *buf = NULL;
	nullpo_retv(mes);

	buf = aMalloc((4 + lp + len)*sizeof(unsigned char));

	WBUFW(buf,0) = 0x9a;
	WBUFW(buf,2) = 4 + lp + len;
	if( type&BC_BLUE )
		WBUFL(buf,4) = 0x65756c62; //If there's "blue" at the beginning of the message, game client will display it in blue instead of yellow.
	else if( type&BC_WOE )
		WBUFL(buf,4) = 0x73737373; //If there's "ssss", game client will recognize message as 'WoE broadcast'.
	else if ((type & BC_MEGAPHONE) != 0)
		WBUFL(buf, 4) = 0x6363696d; // If there's "micc" at the beginning of the message, the game client will recognize message as 'Megaphone shout'.
	memcpy(WBUFP(buf, 4 + lp), mes, len);
	clif->send(buf, WBUFW(buf,2), bl, target);

	aFree(buf);
}

/*==========================================
 * Displays a message on a 'bl' to all it's nearby clients
 * Used by npc_globalmessage
 *------------------------------------------*/
static void clif_GlobalMessage(struct block_list *bl, const char *message)
{
	char buf[256];
	int len;
	nullpo_retv(bl);

	if (message == NULL)
		return;

	len = (int)strlen(message)+1;

	if (len > (int)sizeof(buf)-8) {
		ShowWarning("clif_GlobalMessage: Truncating too long message '%s' (len=%d).\n", message, len);
		len = (int)sizeof(buf)-8;
	}

	WBUFW(buf,0) = 0x8d;
	WBUFW(buf,2) = len+8;
	WBUFL(buf,4) = bl->id;
	safestrncpy(WBUFP(buf,8),message,len);
	clif->send(buf,WBUFW(buf,2),bl,ALL_CLIENT);
}

/// Send broadcast message with font formatting (ZC_BROADCAST2).
/// 01c3 <packet len>.W <fontColor>.L <fontType>.W <fontSize>.W <fontAlign>.W <fontY>.W <message>.?B
static void clif_broadcast2(struct block_list *bl, const char *mes, int len, unsigned int fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target)
{
	unsigned char *buf;

	nullpo_retv(mes);

	buf = aMalloc((16 + len)*sizeof(unsigned char));
	WBUFW(buf,0)  = 0x1c3;
	WBUFW(buf,2)  = len + 16;
	WBUFL(buf,4)  = fontColor;
	WBUFW(buf,8)  = fontType;
	WBUFW(buf,10) = fontSize;
	WBUFW(buf,12) = fontAlign;
	WBUFW(buf,14) = fontY;
	memcpy(WBUFP(buf,16), mes, len);
	clif->send(buf, WBUFW(buf,2), bl, target);

	aFree(buf);
}

/// Displays heal effect (ZC_RECOVERY).
/// 013d <var id>.W <amount>.W
/// var id:
///     5 = HP (SP_HP)
///     7 = SP (SP_SP)
///     ? = ignored
static void clif_heal(int fd, int type, int val)
{
#if PACKETVER < 20150513
	short cmd = 0x13d;
#else
	short cmd = 0xa27;
#endif
	int len = packet_len(cmd);

	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = cmd;
	WFIFOW(fd, 2) = type;
#if PACKETVER < 20150513
	WFIFOW(fd, 4) = cap_value(val, 0, INT16_MAX);
#else
	WFIFOL(fd, 4) = cap_value(val, 0, INT_MAX);
#endif
	WFIFOSET(fd, len);
}

/// Displays resurrection effect (ZC_RESURRECTION).
/// 0148 <id>.L <type>.W
/// type:
///     ignored
static void clif_resurrection(struct block_list *bl, int type)
{
	unsigned char buf[16];

	nullpo_retv(bl);

	WBUFW(buf,0)=0x148;
	WBUFL(buf,2)=bl->id;
	WBUFW(buf,6)=0;

	clif->send(buf,packet_len(0x148),bl, type == 1 ? AREA : AREA_WOS);
	if (clif->isdisguised(bl)) {
		struct map_session_data *sd = BL_UCAST(BL_PC, bl);
		if (sd->fontcolor) {
			WBUFL(buf,2)=-bl->id;
			clif->send(buf,packet_len(0x148),bl, SELF);
		} else {
			clif->spawn(bl);
		}
	}
}

/// Sets the map property (ZC_NOTIFY_MAPPROPERTY).
/// 0199 <type>.W
static void clif_map_property(struct map_session_data *sd, enum map_property property)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x199));
	WFIFOW(fd,0)=0x199;
	WFIFOW(fd,2)=property;
	WFIFOSET(fd,packet_len(0x199));
}

/// Set the map type (ZC_NOTIFY_MAPPROPERTY2).
/// 01d6 <type>.W
static void clif_map_type(struct map_session_data *sd, enum map_type type)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x1D6));
	WFIFOW(fd,0)=0x1D6;
	WFIFOW(fd,2)=type;
	WFIFOSET(fd,packet_len(0x1D6));
}

/// Updates PvP ranking (ZC_NOTIFY_RANKING).
/// 019a <id>.L <ranking>.L <total>.L
// FIXME: missing documentation for the 'type' parameter
static void clif_pvpset(struct map_session_data *sd, int pvprank, int pvpnum, int type)
{
	nullpo_retv(sd);

	if(type == 2) {
		int fd = sd->fd;
		WFIFOHEAD(fd,packet_len(0x19a));
		WFIFOW(fd,0) = 0x19a;
		WFIFOL(fd,2) = sd->bl.id;
		WFIFOL(fd,6) = pvprank;
		WFIFOL(fd,10) = pvpnum;
		WFIFOSET(fd,packet_len(0x19a));
	} else {
		unsigned char buf[32];
		WBUFW(buf,0) = 0x19a;
		WBUFL(buf,2) = sd->bl.id;
		if (sd->sc.option&(OPTION_HIDE|OPTION_CLOAK)) // TODO[Haru] Should this be pc_ishiding(sd)? (i.e. include Chase Walk and any new options)
			WBUFL(buf,6) = UINT32_MAX; //On client displays as --
		else
			WBUFL(buf,6) = pvprank;
		WBUFL(buf,10) = pvpnum;
		if (pc_isinvisible(sd) || sd->disguise != -1) //Causes crashes when a 'mob' with pvp info dies.
			clif->send(buf,packet_len(0x19a),&sd->bl,SELF);
		else if(!type)
			clif->send(buf,packet_len(0x19a),&sd->bl,AREA);
		else
			clif->send(buf,packet_len(0x19a),&sd->bl,ALL_SAMEMAP);
	}
}

/*==========================================
 *
 *------------------------------------------*/
static void clif_map_property_mapall(int mapid, enum map_property property)
{
	struct block_list bl;
	unsigned char buf[16];

	memset(&bl, 0, sizeof(bl));
	bl.id = 0;
	bl.type = BL_NUL;
	bl.m = mapid;
	WBUFW(buf,0)=0x199;
	WBUFW(buf,2)=property;
	clif->send(buf,packet_len(0x199),&bl,ALL_SAMEMAP);
}

/// Notifies the client about the result of a refine attempt (ZC_ACK_ITEMREFINING).
/// 0188 <result>.W <index>.W <refine>.W
/// result:
///     0 = success
///     1 = failure
///     2 = downgrade
static void clif_refine(int fd, int fail, int index, int val)
{
	WFIFOHEAD(fd,packet_len(0x188));
	WFIFOW(fd,0)=0x188;
	WFIFOW(fd,2)=fail;
	WFIFOW(fd,4)=index+2;
	WFIFOW(fd,6)=val;
	WFIFOSET(fd,packet_len(0x188));
}

/// Notifies the client about the result of a weapon refine attempt (ZC_ACK_WEAPONREFINE).
/// 0223 <result>.L <nameid>.W
/// result:
///     0 = "weapon upgraded: %s" MsgStringTable[911] in rgb(0,255,255)
///     1 = "weapon upgraded: %s" MsgStringTable[912] in rgb(0,205,205)
///     2 = "cannot upgrade %s until you level up the upgrade weapon skill" MsgStringTable[913] in rgb(255,200,200)
///     3 = "you lack the item %s to upgrade the weapon" MsgStringTable[914] in rgb(255,200,200)
static void clif_upgrademessage(int fd, int result, int item_id)
{
	struct PACKET_ZC_ACK_WEAPONREFINE p;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x223;
	p.result = result;
	p.itemId = item_id;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Whisper is transmitted to the destination player (ZC_WHISPER).
/// 0097 <packet len>.W <nick>.24B <message>.?B
/// 0097 <packet len>.W <nick>.24B <isAdmin>.L <message>.?B (PACKETVER >= 20091104)
static void clif_wis_message(int fd, const char *nick, const char *mes, int mes_len)
{
#if PACKETVER >= 20091104
	struct map_session_data *ssd = NULL;
#endif // PACKETVER >= 20091104
	nullpo_retv(nick);
	nullpo_retv(mes);

#if PACKETVER < 20091104
	WFIFOHEAD(fd, mes_len + NAME_LENGTH + 5);
	WFIFOW(fd,0) = 0x97;
	WFIFOW(fd,2) = mes_len + NAME_LENGTH + 5;
	safestrncpy(WFIFOP(fd,4), nick, NAME_LENGTH);
	safestrncpy(WFIFOP(fd,28), mes, mes_len + 1);
	WFIFOSET(fd,WFIFOW(fd,2));
#else
	ssd = map->nick2sd(nick, false);

	WFIFOHEAD(fd, mes_len + NAME_LENGTH + 9);
	WFIFOW(fd,0) = 0x97;
	WFIFOW(fd,2) = mes_len + NAME_LENGTH + 9;
	safestrncpy(WFIFOP(fd,4), nick, NAME_LENGTH);
	WFIFOL(fd,28) = (ssd && pc_get_group_level(ssd) == 99) ? 1 : 0; // isAdmin; if nonzero, also displays text above char
	safestrncpy(WFIFOP(fd,32), mes, mes_len + 1);
	WFIFOSET(fd,WFIFOW(fd,2));
#endif
}

/// Inform the player about the result of his whisper action (ZC_ACK_WHISPER).
/// 0098 <result>.B
/// result:
///     0 = success to send whisper
///     1 = target character is not logged in
///     2 = ignored by target
///     3 = everyone ignored by target
///     other = target character is not logged in
static void clif_wis_end(int fd, int flag)
{
	struct map_session_data *sd = sockt->session_is_valid(fd) ? sockt->session[fd]->session_data : NULL;
	struct packet_wis_end p;

	if( !sd )
		return;

	p.PacketType = wisendType;
	p.result = (char)flag;
#if PACKETVER >= 20131223
	p.AID = sd->bl.id;
#endif

	clif->send(&p, sizeof(p), &sd->bl, SELF);
}

/// Returns character name requested by char_id (ZC_ACK_REQNAME_BYGID).
/// 0194 <char id>.L <name>.24B
/// 0af7 <flag>.W <char id>.L <name>.24B
static void clif_solved_charname(int fd, int charid, const char *name)
{
	nullpo_retv(name);
#if PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328
	WFIFOHEAD(fd, packet_len(0x0af7));
	WFIFOW(fd, 0) = 0xaf7;
	if (*name == 0) {
		WFIFOW(fd, 2) = 2;
		memset(WFIFOP(fd, 8), 0, NAME_LENGTH);
	} else {
		WFIFOW(fd, 2) = 3;
		safestrncpy(WFIFOP(fd, 8), name, NAME_LENGTH);
	}
	WFIFOL(fd, 4) = charid;
	WFIFOSET(fd, packet_len(0x0af7));
#else
	WFIFOHEAD(fd, packet_len(0x194));
	WFIFOW(fd, 0) = 0x194;
	WFIFOL(fd, 2) = charid;
	safestrncpy(WFIFOP(fd, 6), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x194));
#endif
}

/// Presents a list of items that can be carded/composed (ZC_ITEMCOMPOSITION_LIST).
/// 017b <packet len>.W { <index>.W }*
static void clif_use_card(struct map_session_data *sd, int idx)
{
	int i, c;
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	if (sd->state.trading != 0)
		return;
	if (!pc->can_insert_card(sd, idx))
		return;

	WFIFOHEAD(fd, sd->status.inventorySize * 2 + 4);
	WFIFOW(fd, 0) = 0x17b;

	for (i = c = 0; i < sd->status.inventorySize; i++) {
		if (!pc->can_insert_card_into(sd, idx, i))
			continue;
		WFIFOW(fd, 4 + c * 2) = i + 2;
		c++;
	}

	if (!c) return; // no item is available for card insertion

	WFIFOW(fd, 2) = 4 + c * 2;
	WFIFOSET(fd, WFIFOW(fd, 2));
}

/// Notifies the client about the result of item carding/composition (ZC_ACK_ITEMCOMPOSITION).
/// 017d <equip index>.W <card index>.W <result>.B
/// result:
///     0 = success
///     1 = failure
static void clif_insert_card(struct map_session_data *sd, int idx_equip, int idx_card, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x17d));
	WFIFOW(fd,0)=0x17d;
	WFIFOW(fd,2)=idx_equip+2;
	WFIFOW(fd,4)=idx_card+2;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,packet_len(0x17d));
}

/// Presents a list of items that can be identified (ZC_ITEMIDENTIFY_LIST).
/// 0177 <packet len>.W { <index>.W }*
static void clif_item_identify_list(struct map_session_data *sd)
{
	int i,c;
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;

	WFIFOHEAD(fd, sd->status.inventorySize * 2 + 4);
	WFIFOW(fd,0)=0x177;
	for (i = c = 0; i < sd->status.inventorySize; i++) {
		if(sd->status.inventory[i].nameid > 0 && !sd->status.inventory[i].identify){
			WFIFOW(fd,c*2+4)=i+2;
			c++;
		}
	}
	if(c > 0) {
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,WFIFOW(fd,2));
		sd->menuskill_id = MC_IDENTIFY;
		sd->menuskill_val = c;
		sd->state.workinprogress = 3;
	}
}

/// Notifies the client about the result of a item identify request (ZC_ACK_ITEMIDENTIFY).
/// 0179 <index>.W <result>.B
static void clif_item_identified(struct map_session_data *sd, int idx, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x179));
	WFIFOW(fd, 0)=0x179;
	WFIFOW(fd, 2)=idx+2;
	WFIFOB(fd, 4)=flag;
	WFIFOSET(fd,packet_len(0x179));
}

/// Presents a list of items that can be repaired (ZC_REPAIRITEMLIST).
/// 01fc <packet len>.W { <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }*
static void clif_item_repair_list(struct map_session_data *sd, struct map_session_data *dstsd, int lv)
{
	int i,c;
	int fd;
	int len;
	struct PACKET_ZC_REPAIRITEMLIST *p;

	nullpo_retv(sd);
	nullpo_retv(dstsd);

	fd = sd->fd;

	len = dstsd->status.inventorySize * sizeof(struct PACKET_ZC_REPAIRITEMLIST_sub) + sizeof(struct PACKET_ZC_REPAIRITEMLIST);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x1fc;
	for (i = c = 0; i < sd->status.inventorySize; i++) {
		int nameid = dstsd->status.inventory[i].nameid;
		if (nameid > 0 && (dstsd->status.inventory[i].attribute & ATTR_BROKEN) != 0) { // && skill_can_repair(sd,nameid)) {
			p->items[c].index = i;
			p->items[c].itemId = nameid;
			p->items[c].refine = dstsd->status.inventory[i].refine;
			clif->addcards(&p->items[c].slot, &dstsd->status.inventory[i]);
			c++;
		}
	}
	if (c > 0) {
		len = c * sizeof(struct PACKET_ZC_REPAIRITEMLIST_sub) + sizeof(struct PACKET_ZC_REPAIRITEMLIST);
		p->packetLength = len;
		WFIFOSET(fd, len);
		sd->menuskill_id = BS_REPAIRWEAPON;
		sd->menuskill_val = dstsd->bl.id;
		sd->menuskill_val2 = lv;
	} else {
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
	}
}

/// Notifies the client about the result of a item repair request (ZC_ACK_ITEMREPAIR).
/// 01fe <index>.W <result>.B
/// index:
///     ignored (inventory index)
/// result:
///     0 = Item repair success.
///     1 = Item repair failure.
static void clif_item_repaireffect(struct map_session_data *sd, int idx, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x1fe));
	WFIFOW(fd, 0)=0x1fe;
	WFIFOW(fd, 2)=idx+2;
	WFIFOB(fd, 4)=flag;
	WFIFOSET(fd,packet_len(0x1fe));

}

/// Displays a message, that an equipment got damaged (ZC_EQUIPITEM_DAMAGED).
/// 02bb <equip location>.W <account id>.L
static void clif_item_damaged(struct map_session_data *sd, unsigned short position)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2bb));
	WFIFOW(fd,0) = 0x2bb;
	WFIFOW(fd,2) = position;
	WFIFOL(fd,4) = sd->bl.id;  // TODO: the packet seems to be sent to other people as well, probably party and/or guild.
	WFIFOSET(fd,packet_len(0x2bb));
}

/// Presents a list of weapon items that can be refined [Taken from jAthena] (ZC_NOTIFY_WEAPONITEMLIST).
/// 0221 <packet len>.W { <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }*
static void clif_item_refine_list(struct map_session_data *sd)
{
	int i,c;
	int fd;
	int len;
	struct PACKET_ZC_NOTIFY_WEAPONITEMLIST *p;
	uint16 skill_lv;

	nullpo_retv(sd);

	skill_lv = pc->checkskill(sd, WS_WEAPONREFINE);

	fd = sd->fd;
	len = sd->status.inventorySize * sizeof(struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub) + sizeof(struct PACKET_ZC_NOTIFY_WEAPONITEMLIST);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x221;
	for (i = c = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify
			&& itemdb_wlv(sd->status.inventory[i].nameid) >= 1
			&& !sd->inventory_data[i]->flag.no_refine
			&& !(sd->status.inventory[i].equip & EQP_ARMS)) {
			p->items[c].index = i + 2;
			p->items[c].itemId = sd->status.inventory[i].nameid;
			p->items[c].refine = sd->status.inventory[i].refine;
			clif->addcards(&p->items[c].slot, &sd->status.inventory[i]);
			c++;
		}
	}
	len = c * sizeof(struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub) + sizeof(struct PACKET_ZC_NOTIFY_WEAPONITEMLIST);
	p->packetLength = len;
	WFIFOSET(fd, len);
	if (c > 0) {
		sd->menuskill_id = WS_WEAPONREFINE;
		sd->menuskill_val = skill_lv;
	}
}

/// Notification of an auto-casted skill (ZC_AUTORUN_SKILL).
/// 0147 <skill id>.W <type>.L <level>.W <sp cost>.W <atk range>.W <skill name>.24B <upgradeable>.B
static void clif_item_skill(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_AUTORUN_SKILL));

	struct PACKET_ZC_AUTORUN_SKILL *p = WFIFOP(fd, 0);
	int type = skill->get_inf(skill_id);

	if (sd->auto_cast_current.itemskill_cast_on_self && sd->auto_cast_current.type == AUTOCAST_ITEM)
		type = INF_SELF_SKILL;

	p->packetType = HEADER_ZC_AUTORUN_SKILL;
	p->skill_id = skill_id;
	p->skill_type = type;
	p->skill_lv = skill_lv;
	p->skill_sp = skill->get_sp(skill_id, skill_lv);
	p->skill_range = skill->get_range2(&sd->bl, skill_id, skill_lv);
	safestrncpy(p->skill_name, skill->get_name(skill_id), NAME_LENGTH);
	p->up_flag = 0;

	WFIFOSET(fd, sizeof(struct PACKET_ZC_AUTORUN_SKILL));
}

/// Adds an item to character's cart.
/// 0124 <index>.W <amount>.L <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_CART)
/// 01c5 <index>.W <amount>.L <name id>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_CART2)
static void clif_cart_additem(struct map_session_data *sd, int n, int amount, int fail)
{
	int view, fd;
	struct PACKET_ZC_ADD_ITEM_TO_CART p;

	nullpo_retv(sd);

	fd = sd->fd;
	if (n < 0 || n >= MAX_CART || sd->status.cart[n].nameid <= 0)
		return;

	WFIFOHEAD(fd, sizeof(p));
	p.packetType = cartaddType;
	p.index = n + 2;
	p.amount = amount;
	if ((view = itemdb_viewid(sd->status.cart[n].nameid)) > 0)
		p.itemId = view;
	else
		p.itemId = sd->status.cart[n].nameid;
#if PACKETVER >= 5
	p.itemType = itemdb_type(sd->status.cart[n].nameid);
#endif
	p.identified = sd->status.cart[n].identify;
	p.damaged  = sd->status.cart[n].attribute;
	p.refine = sd->status.cart[n].refine;
	clif->addcards(&p.slot, &sd->status.cart[n]);
#if PACKETVER >= 20150226
	clif->add_item_options(&p.option_data[0], &sd->status.cart[n]);
#endif
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Deletes an item from character's cart (ZC_DELETE_ITEM_FROM_CART).
/// 0125 <index>.W <amount>.L
static void clif_cart_delitem(struct map_session_data *sd, int n, int amount)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;

	WFIFOHEAD(fd,packet_len(0x125));
	WFIFOW(fd,0)=0x125;
	WFIFOW(fd,2)=n+2;
	WFIFOL(fd,4)=amount;
	WFIFOSET(fd,packet_len(0x125));
}

/// Opens the shop creation menu (ZC_OPENSTORE).
/// 012d <num>.W
/// num:
///     number of allowed item slots
static void clif_openvendingreq(struct map_session_data *sd, int num)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x12d));
	WFIFOW(fd,0) = 0x12d;
	WFIFOW(fd,2) = num;
	WFIFOSET(fd,packet_len(0x12d));
}

/// Displays a vending board to target/area (ZC_STORE_ENTRY).
/// 0131 <owner id>.L <message>.80B
static void clif_showvendingboard(struct block_list *bl, const char *message, int fd)
{
	unsigned char buf[128];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x131;
	WBUFL(buf,2) = bl->id;
	safestrncpy(WBUFP(buf,6), message, 80);

	if( fd ) {
		WFIFOHEAD(fd,packet_len(0x131));
		memcpy(WFIFOP(fd,0),buf,packet_len(0x131));
		WFIFOSET(fd,packet_len(0x131));
	} else {
		clif->send(buf,packet_len(0x131),bl,AREA_WOS);
	}
}

/// Removes a vending board from screen (ZC_DISAPPEAR_ENTRY).
/// 0132 <owner id>.L
static void clif_closevendingboard(struct block_list *bl, int fd)
{
	unsigned char buf[16];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x132;
	WBUFL(buf,2) = bl->id;
	if( fd ) {
		WFIFOHEAD(fd,packet_len(0x132));
		memcpy(WFIFOP(fd,0),buf,packet_len(0x132));
		WFIFOSET(fd,packet_len(0x132));
	} else {
		clif->send(buf,packet_len(0x132),bl,AREA_WOS);
	}
}

/// Sends a list of items in a shop.
/// R 0133 <packet len>.W <owner id>.L { <price>.L <amount>.W <index>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_PC_PURCHASE_ITEMLIST_FROMMC)
/// R 0800 <packet len>.W <owner id>.L <unique id>.L { <price>.L <amount>.W <index>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_PC_PURCHASE_ITEMLIST_FROMMC2)
static void clif_vendinglist(struct map_session_data *sd, unsigned int id, struct s_vending *vending_items)
{
	int i, fd;
	int count;
	struct map_session_data* vsd;
	int len;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC *p;

	nullpo_retv(sd);
	nullpo_retv(vending_items);
	nullpo_retv(vsd=map->id2sd(id));

	fd = sd->fd;
	count = vsd->vend_num;
	len = sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC) + count * sizeof(struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub);

	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = vendinglistType;
	p->packetLength = len;
	p->AID = id;
#if PACKETVER >= 20100105
	p->venderId = vsd->vender_id;
#endif

	for (i = 0; i < count; i++) {
		int index = vending_items[i].index;
		struct item_data* data = itemdb->search(vsd->status.cart[index].nameid);
		p->items[i].price = vending_items[i].value;
		p->items[i].amount = vending_items[i].amount;
		p->items[i].index = vending_items[i].index + 2;
		p->items[i].itemType = itemtype(data->type);
		p->items[i].itemId = (data->view_id > 0) ? data->view_id : vsd->status.cart[index].nameid;
		p->items[i].identified = vsd->status.cart[index].identify;
		p->items[i].damaged = vsd->status.cart[index].attribute;
		p->items[i].refine = vsd->status.cart[index].refine;
		clif->addcards(&p->items[i].slot, &vsd->status.cart[index]);
#if PACKETVER >= 20150226
		clif->add_item_options(&p->items[i].option_data[0], &vsd->status.cart[index]);
#endif
// [4144] date 20160921 not confirmed. Can be bigger or smaller
#if PACKETVER >= 20160921
		p->items[i].location = pc->item_equippoint(sd, data);
		p->items[i].viewSprite = data->view_sprite;
#endif
	}
	WFIFOSET(fd, len);
}

/// Shop purchase failure (ZC_PC_PURCHASE_RESULT_FROMMC).
/// 0135 <index>.W <amount>.W <result>.B
/// result:
///     0 = success
///     1 = not enough zeny
///     2 = overweight
///     4 = out of stock
///     5 = "cannot use an npc shop while in a trade"
///     6 = Because the store information was incorrect the item was not purchased.
///     7 = No sales information.
static void clif_buyvending(struct map_session_data *sd, int index, int amount, int fail)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x135));
	WFIFOW(fd,0) = 0x135;
	WFIFOW(fd,2) = index+2;
	WFIFOW(fd,4) = amount;
	WFIFOB(fd,6) = fail;
	WFIFOSET(fd,packet_len(0x135));
}

/// Shop creation success (ZC_PC_PURCHASE_MYITEMLIST).
/// 0136 <packet len>.W <owner id>.L { <price>.L <index>.W <amount>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }*
static void clif_openvending(struct map_session_data *sd, int id, struct s_vending *vending_items)
{
	int i, fd;
	int count;

	struct PACKET_ZC_PC_PURCHASE_MYITEMLIST *p;
	int len;

	nullpo_retv(sd);
	nullpo_retv(vending_items);

	fd = sd->fd;
	count = sd->vend_num;
	len = sizeof(struct PACKET_ZC_PC_PURCHASE_MYITEMLIST) + count * sizeof(struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x136;
	p->packetLength = len;
	p->AID = id;
	for (i = 0; i < count; i++) {
		int index = vending_items[i].index;
		struct item_data* data = itemdb->search(sd->status.cart[index].nameid);
		p->items[i].price = vending_items[i].value;
		p->items[i].index = vending_items[i].index + 2;
		p->items[i].amount = vending_items[i].amount;
		p->items[i].itemType = itemtype(data->type);
		p->items[i].itemId = (data->view_id > 0) ? data->view_id : sd->status.cart[index].nameid;
		p->items[i].identified = sd->status.cart[index].identify;
		p->items[i].damaged = sd->status.cart[index].attribute;
		p->items[i].refine = sd->status.cart[index].refine;
		clif->addcards(&p->items[i].slot, &sd->status.cart[index]);
#if PACKETVER >= 20150226
		clif->add_item_options(&p->items[i].option_data[0], &sd->status.cart[index]);
#endif
	}
	WFIFOSET(fd, len);

	clif->openvendingAck(fd, 0);
}

// 0 - open vending success
// 1 - message MSG_MERCHANTSHOP_MAKING_FAIL
// 2 - silent ignore
// 3 - message MSG_ID_C9D (You can not open a stall at the current location)
static void clif_openvendingAck(int fd, int result)
{
#if PACKETVER >= 20140625
	WFIFOHEAD(fd, packet_len(0xa28));
	WFIFOW(fd, 0) = 0xa28;  // ZC_ACK_OPENSTORE2
	WFIFOB(fd, 2) = result;
	WFIFOSET(fd, packet_len(0xa28));
#endif
}

/// Inform merchant that someone has bought an item.
/// 0137 <index>.W <amount>.W (ZC_DELETEITEM_FROM_MCSTORE).
/// 09e5 <index>.W <amount>.W <GID>.L <Date>.L <zeny>.L (ZC_DELETEITEM_FROM_MCSTORE2).
static void clif_vendingreport(struct map_session_data *sd, int index, int amount, uint32 char_id, int zeny)
{
	int fd;
#if PACKETVER < 20141016  // TODO : not sure for client date [Napster]
	const int cmd = 0x137;
#else
	const int cmd = 0x9e5;
#endif
	const int len = packet_len(cmd);

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = cmd;
	WFIFOW(fd, 2) = index + 2;
	WFIFOW(fd, 4) = amount;
#if PACKETVER >= 20141016
	WFIFOL(fd,6) = char_id; // GID
	WFIFOL(fd,10) = (int)time(NULL); // Date
	WFIFOL(fd,14) = zeny;   // zeny
#endif
	WFIFOSET(fd, len);
}

/// Result of organizing a party (ZC_ACK_MAKE_GROUP).
/// 00fa <result>.B
/// result:
///     0 = opens party window and shows MsgStringTable[77]="party successfully organized"
///     1 = MsgStringTable[78]="party name already exists"
///     2 = MsgStringTable[79]="already in a party"
///     3 = cannot organize parties on this map
///     ? = nothing
static void clif_party_created(struct map_session_data *sd, int result)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xfa));
	WFIFOW(fd,0)=0xfa;
	WFIFOB(fd,2)=result;
	WFIFOSET(fd,packet_len(0xfa));
}

/// Adds new member to a party.
/// role:
///     0 = leader
///     1 = normal
/// state:
///     0 = connected
///     1 = disconnected
static void clif_party_member_info(struct party_data *p, struct map_session_data *sd)
{
	int i;
	struct PACKET_ZC_ADD_MEMBER_TO_GROUP packet;

	nullpo_retv(p);
	nullpo_retv(sd);

	memset(&packet, 0, sizeof(packet));
	if (!sd) { //Pick any party member (this call is used when changing item share rules)
		ARR_FIND(0, MAX_PARTY, i, p->data[i].sd != 0);
	} else {
		ARR_FIND(0, MAX_PARTY, i, p->data[i].sd == sd);
	}
	if (i >= MAX_PARTY)
		return; //Should never happen...
	sd = p->data[i].sd;

	packet.packetType = partymemberinfo;
	packet.AID = sd->status.account_id;
#if PACKETVER >= 20171207
	packet.GID = sd->status.char_id;
#endif
	packet.leader = (p->party.member[i].leader) ? 0 : 1;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	packet.class = sd->status.class;
	packet.baseLevel = sd->status.base_level;
#endif
	packet.x = sd->bl.x;
	packet.y = sd->bl.y;
	packet.offline = (p->party.member[i].online) ? 0 : 1;
	memcpy(packet.partyName, p->party.name, NAME_LENGTH);
	memcpy(packet.playerName, sd->status.name, NAME_LENGTH);
	mapindex->getmapname_ext(map->list[sd->bl.m].custom_name ? map->list[map->list[sd->bl.m].instance_src_map].name : map->list[sd->bl.m].name, packet.mapName);
	packet.sharePickup = (p->party.item & 1) ? 1 : 0;
	packet.shareLoot = (p->party.item & 2) ? 1 : 0;
	clif->send(&packet, sizeof(packet), &sd->bl, PARTY);
}

/// Sends party information (ZC_GROUP_LIST).
/// role:
///     0 = leader
///     1 = normal
/// state:
///     0 = connected
///     1 = disconnected
static void clif_party_info(struct party_data *p, struct map_session_data *sd)
{
	struct PACKET_ZC_GROUP_LIST *packet;
	struct map_session_data* party_sd = NULL;
	int i, c;
	unsigned char buf[sizeof(*packet) + sizeof(struct PACKET_ZC_GROUP_LIST_SUB) * MAX_PARTY];
	nullpo_retv(p);

	memset(buf, 0, sizeof(buf));
	packet = (struct PACKET_ZC_GROUP_LIST *)buf;
	packet->packetType = partyinfo;
	memcpy(packet->partyName, p->party.name, NAME_LENGTH);
	for(i = 0, c = 0; i < MAX_PARTY; i++)
	{
		struct party_member *m = &p->party.member[i];
		if (!m->account_id)
			continue;

		if (party_sd == NULL)
			party_sd = p->data[i].sd;

		packet->members[c].AID = m->account_id;
#if PACKETVER >= 20171207
		packet->members[c].GID = m->char_id;
#endif
		memcpy(packet->members[c].playerName, m->name, NAME_LENGTH);
		mapindex->getmapname_ext(mapindex_id2name(m->map), packet->members[c].mapName);
		packet->members[c].leader = (m->leader) ? 0 : 1;
		packet->members[c].offline = (m->online) ? 0 : 1;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
		packet->members[c].class = m->class;
		packet->members[c].baseLevel = m->lv;
#endif
		c++;
	}
	packet->packetLen = sizeof(*packet) + c * sizeof(struct PACKET_ZC_GROUP_LIST_SUB);

	if (sd) { // send only to self
		clif->send(buf, packet->packetLen, &sd->bl, SELF);
	} else if (party_sd) { // send to whole party
		clif->send(buf, packet->packetLen, &party_sd->bl, PARTY);
	}
}

/// Updates the job and level of a party member
/// 0abd <account id>.L <job>.W <level>.W
static void clif_party_job_and_level(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	unsigned char buf[10];

	nullpo_retv(sd);

	WBUFW(buf, 0) = 0xabd;
	WBUFL(buf, 2) = sd->status.account_id;
	WBUFW(buf, 6) = sd->status.class;
	WBUFW(buf, 8) = sd->status.base_level;

	clif->send(buf, packet_len(0xabd), &sd->bl, PARTY);
#endif
}

/// The player's 'party invite' state, sent during login (ZC_PARTY_CONFIG).
/// 02c9 <flag>.B
/// flag:
///     0 = allow party invites
///     1 = auto-deny party invites
static void clif_partyinvitationstate(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20070911 || defined(PACKETVER_RE) || PACKETVER_AD_NUM >= 20070911 || PACKETVER_SAK_NUM >= 20070904 || defined(PACKETVER_ZERO)
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_PARTY_CONFIG));
	struct PACKET_ZC_PARTY_CONFIG *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_PARTY_CONFIG;
	p->denyPartyInvites = sd->status.allow_party ? 1 : 0;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_PARTY_CONFIG));
#endif
}

/// Party invitation request.
/// 00fe <party id>.L <party name>.24B (ZC_REQ_JOIN_GROUP)
/// 02c6 <party id>.L <party name>.24B (ZC_PARTY_JOIN_REQ)
static void clif_party_invite(struct map_session_data *sd, struct map_session_data *tsd)
{
#if PACKETVER < 20070821
	const int cmd = 0xfe;
#else
	const int cmd = 0x2c6;
#endif
	int fd;
	struct party_data *p;

	nullpo_retv(sd);
	nullpo_retv(tsd);

	fd=tsd->fd;

	if( (p=party->search(sd->status.party_id))==NULL )
		return;

	WFIFOHEAD(fd,packet_len(cmd));
	WFIFOW(fd,0)=cmd;
	WFIFOL(fd,2)=sd->status.party_id;
	memcpy(WFIFOP(fd,6),p->party.name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(cmd));
}

/// Party invite result.
/// 00fd <nick>.24S <result>.B (ZC_ACK_REQ_JOIN_GROUP)
/// 02c5 <nick>.24S <result>.L (ZC_PARTY_JOIN_REQ_ACK)
/// result=0  : char is already in a party -> MsgStringTable[80]
/// result=1  : party invite was rejected -> MsgStringTable[81]
/// result=2  : party invite was accepted -> MsgStringTable[82]
/// result=3  : party is full -> MsgStringTable[83]
/// result=4  : char of the same account already joined the party -> MsgStringTable[608]
/// result=5  : char blocked party invite -> MsgStringTable[1324] (since 20070904)
/// result=7  : char is not online or doesn't exist -> MsgStringTable[71] (since 20070904)
/// result=8  : (%s) are currently in restricted map to join a party. -> MsgStringTable[1388] (since 20080527)
/// result=9  : Cannot join a party in this map. -> MsgStringTable[1871] (since 20110215)
/// result=10 : You cannot invite or withdraw while in memorial dungeon -> message: MSG_ID_BD3 (since 20161130)
/// result=11 : The character is a level that can not join the party -> message: MSG_ID_C9A (since 20170412)
static void clif_party_inviteack(struct map_session_data *sd, const char *nick, int result)
{
	int fd;
	nullpo_retv(sd);
	nullpo_retv(nick);
	fd=sd->fd;

#if PACKETVER < 20070904
	if( result == 7 ) {
		clif->message(fd, msg_sd(sd,3)); // Character not found.
		return;
	}
#endif

#if PACKETVER < 20070821
	WFIFOHEAD(fd,packet_len(0xfd));
	WFIFOW(fd,0) = 0xfd;
	safestrncpy(WFIFOP(fd,2),nick,NAME_LENGTH);
	WFIFOB(fd,26) = result;
	WFIFOSET(fd,packet_len(0xfd));
#else
	WFIFOHEAD(fd,packet_len(0x2c5));
	WFIFOW(fd,0) = 0x2c5;
	safestrncpy(WFIFOP(fd,2),nick,NAME_LENGTH);
	WFIFOL(fd,26) = result;
	WFIFOSET(fd,packet_len(0x2c5));
#endif
}

/**
 * Sends party settings to the client.
 *
 * 0101 <exp option>.L (ZC_GROUPINFO_CHANGE)
 * 07d8 <exp option>.L <item pick rule>.B <item share rule>.B (ZC_REQ_GROUPINFO_CHANGE_V2)
 * <exp option>:
 *	0 = EXP sharing disabled.
 *	1 = EXP sharing enabled.
 *	2 = Cannot change EXP sharing.
 *
 * @param p The related party.
 * @param sd The related character.
 * @param flag Reason for sending.
 * @parblock
 * Possible flags:
 *	0x01 = Cannot change EXP sharing. (Only set when tried to change options manually.)
 *	0x02 = Options changed manually.
 *	0x04 = Options changed automatically.
 *	0x08 = Member added.
 *	0x10 = Member removed.
 *	0x20 = Character logged in.
 *	0x40 = Character changed map.
 *	0x80 = Character teleported.
 * @endparblock
 *
 **/
static void clif_party_option(struct party_data *p, struct map_session_data *sd, int flag)
{
	nullpo_retv(p);

	if (sd == NULL && (flag & 0x01) == 0) {
		for (int i = 0; i < MAX_PARTY; i++) {
			if (p->data[i].sd != NULL) {
				sd = p->data[i].sd;
				break;
			}
		}
	}

	if (sd == NULL)
		return;

	int conf = battle_config.send_party_options;

	if (((flag & 0x01) != 0 && (conf & 0x10) == 0)
	    || ((flag & 0x02) != 0 && (conf & 0x08) == 0)
	    || ((flag & 0x04) != 0 && (conf & 0x20) == 0)
	    || ((flag & 0x08) != 0 && (conf & 0x40) == 0)
	    || ((flag & 0x10) != 0 && (conf & 0x80) == 0)
	    || ((flag & 0x20) != 0 && (conf & 0x01) == 0)
	    || ((flag & 0x40) != 0 && (conf & 0x02) == 0)
	    || ((flag & 0x80) != 0 && (conf & 0x04) == 0)) {
		return;
	}

	enum send_target target = SELF;

	if (((flag & 0x01) != 0 && (conf & 0x100) != 0)
	    || ((flag & 0x01) == 0 && (flag & 0x02) != 0)
	    || (flag & 0x04) != 0) {
		target = PARTY;
	}

	int cmd = 0x101;

	if (((flag & 0x01) != 0 && (conf & 0x02000) != 0)
	    || ((flag & 0x02) != 0 && (conf & 0x01000) != 0)
	    || ((flag & 0x04) != 0 && (conf & 0x04000) != 0)
	    || ((flag & 0x08) != 0 && (conf & 0x08000) != 0)
	    || ((flag & 0x10) != 0 && (conf & 0x10000) != 0)
	    || ((flag & 0x20) != 0 && (conf & 0x00200) != 0)
	    || ((flag & 0x40) != 0 && (conf & 0x00400) != 0)
	    || ((flag & 0x80) != 0 && (conf & 0x00800) != 0)) {
		cmd = 0x7d8;
	}

#if PACKETVER < 20090603
	if (cmd == 0x7d8)
		cmd = 0x101;
#endif

	unsigned char buf[16];

	WBUFW(buf, 0) = cmd;
	WBUFL(buf, 2) = ((flag & 0x10) != 0) ? 0 : (((flag & 0x01) != 0) ? 2 : p->party.exp);

	if (cmd == 0x7d8) {
		WBUFB(buf, 6) = ((flag & 0x10) != 0) ? 0 : (((p->party.item & 1) != 0) ? 1 : 0);
		WBUFB(buf, 7) = ((flag & 0x10) != 0) ? 0 : (((p->party.item & 2) != 0) ? 1 : 0);
	}

	clif->send(buf, packet_len(cmd), &sd->bl, target);

	if ((flag & 0x04) != 0)
		p->state.option_auto_changed = 0;
}

/// 0105 <account id>.L <char name>.24B <result>.B (ZC_DELETE_MEMBER_FROM_GROUP).
/// result:
///     0 = leave
///     1 = expel
///     2 = cannot leave party on this map
///     3 = cannot expel from party on this map
static void clif_party_withdraw(struct party_data *p, struct map_session_data *sd, int account_id, const char *name, int flag)
{
	unsigned char buf[64];

	nullpo_retv(p);
	nullpo_retv(name);

	if(!sd && (flag&0xf0)==0) { // TODO: Document this flag
		int i;
		// Search for any online party member
		ARR_FIND(0, MAX_PARTY, i, p->data[i].sd != NULL);
		if (i != MAX_PARTY)
			sd = p->data[i].sd;
	}

	if (!sd)
		return;

	WBUFW(buf,0)=0x105;
	WBUFL(buf,2)=account_id;
	memcpy(WBUFP(buf,6),name,NAME_LENGTH);
	WBUFB(buf,30)=flag&0x0f;
	if((flag&0xf0)==0)
		clif->send(buf,packet_len(0x105),&sd->bl,PARTY);
	else
		clif->send(buf,packet_len(0x105),&sd->bl,SELF);
}

/// Party chat message (ZC_NOTIFY_CHAT_PARTY).
/// 0109 <packet len>.W <account id>.L <message>.?B
static void clif_party_message(struct party_data *p, int account_id, const char *mes, int len)
{
	struct map_session_data *sd;
	int i;

	nullpo_retv(p);
	nullpo_retv(mes);

	ARR_FIND(0, MAX_PARTY, i, p->data[i].sd != NULL);

	if (i < MAX_PARTY) {
		unsigned char buf[1024];
		int maxlen = (int)sizeof(buf) - 9;

		if (len > maxlen) {
			ShowWarning("clif_party_message: Truncated message '%s' (len=%d, max=%d, party_id=%d).\n",
			            mes, len, maxlen, p->party.party_id);
			len = maxlen;
		}

		sd = p->data[i].sd;
		WBUFW(buf,0) = 0x109;
		WBUFW(buf,2) = len+9;
		WBUFL(buf,4) = account_id;
		safestrncpy(WBUFP(buf,8), mes, len+1);
		clif->send(buf, len+9, &sd->bl, PARTY);
	}
}

/// Updates the position of a party member on the minimap (ZC_NOTIFY_POSITION_TO_GROUPM).
/// 0107 <account id>.L <x>.W <y>.W
static void clif_party_xy(struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=sd->bl.x;
	WBUFW(buf,8)=sd->bl.y;
	clif->send(buf,packet_len(0x107),&sd->bl,PARTY_SAMEMAP_WOS);
}

/*==========================================
 * Sends x/y dot to a single fd. [Skotlex]
 *------------------------------------------*/
static void clif_party_xy_single(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	WFIFOHEAD(fd,packet_len(0x107));
	WFIFOW(fd,0)=0x107;
	WFIFOL(fd,2)=sd->status.account_id;
	WFIFOW(fd,6)=sd->bl.x;
	WFIFOW(fd,8)=sd->bl.y;
	WFIFOSET(fd,packet_len(0x107));
}

/// Updates HP bar of a party member.
/// 0106 <account id>.L <hp>.W <max hp>.W (ZC_NOTIFY_HP_TO_GROUPM)
/// 080e <account id>.L <hp>.L <max hp>.L (ZC_NOTIFY_HP_TO_GROUPM_R2)
static void clif_party_hp(struct map_session_data *sd)
{
	unsigned char buf[16];
#if PACKETVER < 20100126
	const int cmd = 0x106;
#else
	const int cmd = 0x80e;
#endif

	nullpo_retv(sd);

	WBUFW(buf,0)=cmd;
	WBUFL(buf,2)=sd->status.account_id;
#if PACKETVER < 20100126
	if (sd->battle_status.max_hp > INT16_MAX) { //To correctly display the %hp bar. [Skotlex]
		WBUFW(buf,6) = sd->battle_status.hp/(sd->battle_status.max_hp/100);
		WBUFW(buf,8) = 100;
	} else {
		WBUFW(buf,6) = sd->battle_status.hp;
		WBUFW(buf,8) = sd->battle_status.max_hp;
	}
#else
	WBUFL(buf,6) = sd->battle_status.hp;
	WBUFL(buf,10) = sd->battle_status.max_hp;
#endif
	clif->send(buf,packet_len(cmd),&sd->bl,PARTY_AREA_WOS);
}

/*==========================================
 * Sends HP bar to a single fd. [Skotlex]
 *------------------------------------------*/
static void clif_hpmeter_single(int fd, int id, unsigned int hp, unsigned int maxhp)
{
#if PACKETVER < 20100126
	const int cmd = 0x106;
#else
	const int cmd = 0x80e;
#endif
	WFIFOHEAD(fd,packet_len(cmd));
	WFIFOW(fd,0) = cmd;
	WFIFOL(fd,2) = id;
#if PACKETVER < 20100126
	if( maxhp > INT16_MAX )
	{// To correctly display the %hp bar. [Skotlex]
		WFIFOW(fd,6) = hp/(maxhp/100);
		WFIFOW(fd,8) = 100;
	} else {
		WFIFOW(fd,6) = hp;
		WFIFOW(fd,8) = maxhp;
	}
#else
	WFIFOL(fd,6) = hp;
	WFIFOL(fd,10) = maxhp;
#endif
	WFIFOSET(fd, packet_len(cmd));
}

/// Notifies the client, that it's attack target is too far (ZC_ATTACK_FAILURE_FOR_DISTANCE).
/// 0139 <target id>.L <target x>.W <target y>.W <x>.W <y>.W <atk range>.W
static void clif_movetoattack(struct map_session_data *sd, struct block_list *bl)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(bl);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x139));
	WFIFOW(fd, 0)=0x139;
	WFIFOL(fd, 2)=bl->id;
	WFIFOW(fd, 6)=bl->x;
	WFIFOW(fd, 8)=bl->y;
	WFIFOW(fd,10)=sd->bl.x;
	WFIFOW(fd,12)=sd->bl.y;
	WFIFOW(fd,14)=sd->battle_status.rhw.range;
	WFIFOSET(fd,packet_len(0x139));
}

/// Notifies the client about the result of an item produce request (ZC_ACK_REQMAKINGITEM).
/// 018f <result>.W <name id>.W
/// result:
///     0 = success
///     1 = failure
///     2 = success (alchemist)
///     3 = failure (alchemist)
///     4 = success (???)
///     5 = failure (???)
///     6 = failure (???)
///     7 = failure (???)
static void clif_produceeffect(struct map_session_data *sd, int flag, int nameid)
{
	int view, fd;
	struct PACKET_ZC_ACK_REQMAKINGITEM p;

	nullpo_retv(sd);

	fd = sd->fd;
	clif->solved_charname(fd, sd->status.char_id, sd->status.name);
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x18f;
	p.result = flag;
	if ((view = itemdb_viewid(nameid)) > 0)
		p.itemId = view;
	else
		p.itemId = nameid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Initiates the pet taming process (ZC_START_CAPTURE).
/// 019e
static void clif_catch_process(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x19e));
	WFIFOW(fd,0)=0x19e;
	WFIFOSET(fd,packet_len(0x19e));
}

/// Displays the result of a pet taming attempt (ZC_TRYCAPTURE_MONSTER).
/// 01a0 <result>.B
///     0 = failure
///     1 = success
static void clif_pet_roulette(struct map_session_data *sd, int data)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x1a0));
	WFIFOW(fd,0)=0x1a0;
	WFIFOB(fd,2)=data;
	WFIFOSET(fd,packet_len(0x1a0));
}

/// Presents a list of pet eggs that can be hatched (ZC_PETEGG_LIST).
/// 01a6 <packet len>.W { <index>.W }*
static void clif_sendegg(struct map_session_data *sd)
{
	int i, n, fd;

	nullpo_retv(sd);

	fd = sd->fd;
	if (map->list[sd->bl.m].flag.nopet != 0) {
		clif->message(fd, msg_sd(sd, 866)); // "Pets are disabled in this map."
		return;
	}

	WFIFOHEAD(fd, sd->status.inventorySize * 2 + 4);
	WFIFOW(fd,0) = 0x1a6;
	for (i = n = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].nameid <= 0 || sd->inventory_data[i] == NULL || sd->inventory_data[i]->type!=IT_PETEGG || sd->status.inventory[i].amount <= 0)
			continue;
		WFIFOW(fd, n * 2 + 4) = i + 2;
		n++;
	}

	if (!n) return;

	WFIFOW(fd, 2) = 4 + n * 2;
	WFIFOSET(fd, WFIFOW(fd, 2));

	sd->menuskill_id = SA_TAMINGMONSTER;
	sd->menuskill_val = -1;
}

/// Sends a specific pet data update (ZC_CHANGESTATE_PET).
/// 01a4 <type>.B <id>.L <data>.L
/// type:
///     0 = pre-init (data = 0)
///     1 = intimacy (data = 0~4)
///     2 = hunger (data = 0~4)
///     3 = accessory
///     4 = performance (data = 1~3: normal, 4: special)
///     5 = hairstyle
///     6 = close egg selection ui and update egg in inventory (PACKETVER >= 20180704)
///
/// If sd is null, the update is sent to nearby objects, otherwise it is sent only to that player.
static void clif_send_petdata(struct map_session_data *sd, struct pet_data *pd, int type, int param)
{
	uint8 buf[16];
	nullpo_retv(pd);

	WBUFW(buf,0) = 0x1a4;
	WBUFB(buf,2) = type;
	WBUFL(buf,3) = pd->bl.id;
	WBUFL(buf,7) = param;
	if (sd)
		clif->send(buf, packet_len(0x1a4), &sd->bl, SELF);
	else
		clif->send(buf, packet_len(0x1a4), &pd->bl, AREA);
}

/// Pet's base data (ZC_PROPERTY_PET).
/// 01a2 <name>.24B <renamed>.B <level>.W <hunger>.W <intimacy>.W <accessory id>.W <class>.W
static void clif_send_petstatus(struct map_session_data *sd)
{
	int fd;
	struct s_pet *p;

	nullpo_retv(sd);
	nullpo_retv(sd->pd);

	fd=sd->fd;
	p = &sd->pd->pet;
	WFIFOHEAD(fd,packet_len(0x1a2));
	WFIFOW(fd,0)=0x1a2;
	memcpy(WFIFOP(fd,2),p->name,NAME_LENGTH);
	WFIFOB(fd,26)=battle_config.pet_rename?0:p->rename_flag;
	WFIFOW(fd,27)=p->level;
	WFIFOW(fd,29)=p->hungry;
	WFIFOW(fd,31)=p->intimate;
	WFIFOW(fd,33)=p->equip;
#if PACKETVER >= 20081126
	WFIFOW(fd,35)=p->class_;
#endif
	WFIFOSET(fd,packet_len(0x1a2));
}

/// Notification about a pet's emotion/talk (ZC_PET_ACT).
/// 01aa <id>.L <data>.L
/// data:
///     @see CZ_PET_ACT.
static void clif_pet_emotion(struct pet_data *pd, int param)
{
	unsigned char buf[16];

	nullpo_retv(pd);

	memset(buf,0,packet_len(0x1aa));

	WBUFW(buf,0)=0x1aa;
	WBUFL(buf,2)=pd->bl.id;
	if(param >= 100 && pd->petDB->talk_convert_class) {
		if(pd->petDB->talk_convert_class < 0)
			return;
		else if(pd->petDB->talk_convert_class > 0) {
			// replace mob_id component of talk/act data
			param -= (pd->pet.class_ - 100)*100;
			param += (pd->petDB->talk_convert_class - 100)*100;
		}
	}
	WBUFL(buf,6)=param;

	clif->send(buf,packet_len(0x1aa),&pd->bl,AREA);
}

/// Result of request to feed a pet (ZC_FEED_PET).
/// 01a3 <result>.B <name id>.W
/// result:
///     0 = failure
///     1 = success
static void clif_pet_food(struct map_session_data *sd, int foodid, int fail)
{
	int fd;
	struct PACKET_ZC_FEED_PET p;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x1a3;
	p.result = fail;
	p.itemId = foodid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Presents a list of skills that can be auto-spelled (ZC_AUTOSPELLLIST).
/// 01cd { <skill id>.L }*7
static void clif_autospell(struct map_session_data *sd, uint16 skill_lv)
{
#if PACKETVER_MAIN_NUM >= 20090406 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_SAK_NUM >= 20080618
	nullpo_retv(sd);

	int fd = sd->fd;
#if PACKETVER_MAIN_NUM >= 20181128 || PACKETVER_RE_NUM >= 20181031
	// reserve space for 7 skills
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_AUTOSPELLLIST) + 4 * 7);
#else
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_AUTOSPELLLIST));
#endif
	struct PACKET_ZC_AUTOSPELLLIST *p = WFIFOP(fd, 0);
	memset(p, 0, sizeof(struct PACKET_ZC_AUTOSPELLLIST));
	p->packetType = HEADER_ZC_AUTOSPELLLIST;
	int index = 0;

	if (skill_lv > 0 && pc->checkskill(sd, MG_NAPALMBEAT) > 0)
		p->skills[index++] = MG_NAPALMBEAT;
	if (skill_lv > 1 && pc->checkskill(sd, MG_COLDBOLT) > 0)
		p->skills[index++] = MG_COLDBOLT;
	if (skill_lv > 1 && pc->checkskill(sd, MG_FIREBOLT) > 0)
		p->skills[index++] = MG_FIREBOLT;
	if (skill_lv > 1 && pc->checkskill(sd, MG_LIGHTNINGBOLT) > 0)
		p->skills[index++] = MG_LIGHTNINGBOLT;
	if (skill_lv > 4 && pc->checkskill(sd, MG_SOULSTRIKE) > 0)
		p->skills[index++] = MG_SOULSTRIKE;
	if (skill_lv > 7 && pc->checkskill(sd, MG_FIREBALL) > 0)
		p->skills[index++] = MG_FIREBALL;
	if (skill_lv > 9 && pc->checkskill(sd, MG_FROSTDIVER) > 0)
		p->skills[index++] = MG_FROSTDIVER;

#if PACKETVER_MAIN_NUM >= 20181128 || PACKETVER_RE_NUM >= 20181031
	const int len = sizeof(struct PACKET_ZC_AUTOSPELLLIST) + index * 4;
	p->packetLength = len;
#else
	const int len = sizeof(struct PACKET_ZC_AUTOSPELLLIST);
#endif
	WFIFOSET(fd, len);

	sd->menuskill_id = SA_AUTOSPELL;
	sd->menuskill_val = skill_lv;
#endif
}

/// Devotion's visual effect (ZC_DEVOTIONLIST).
/// 01cf <devoter id>.L { <devotee id>.L }*5 <max distance>.W
static void clif_devotion(struct block_list *src, struct map_session_data *tsd)
{
	unsigned char buf[56];

	nullpo_retv(src);
	memset(buf,0,packet_len(0x1cf));

	WBUFW(buf,0) = 0x1cf;
	WBUFL(buf,2) = src->id;
	if( src->type == BL_MER )
	{
		struct mercenary_data *md = BL_CAST(BL_MER,src);
		int skill_lvl;
		if( md && md->master && md->devotion_flag )
			WBUFL(buf,6) = md->master->bl.id;

		skill_lvl = mercenary->checkskill(md, ML_DEVOTION);
		if (skill_lvl > 0)
			WBUFW(buf, 26) = skill->get_range2(src, ML_DEVOTION, skill_lvl);
		else
			WBUFW(buf, 26) = 0;
	}
	else
	{
		int i;
		struct map_session_data *sd = BL_CAST(BL_PC,src);
		int skill_lvl;
		if( sd == NULL )
			return;

		for( i = 0; i < MAX_PC_DEVOTION; i++ )
			WBUFL(buf,6+4*i) = sd->devotion[i];
		skill_lvl = pc->checkskill(sd, CR_DEVOTION);
		if (skill_lvl > 0)
			WBUFW(buf, 26) = skill->get_range2(src, CR_DEVOTION, skill_lvl);
		else
			WBUFW(buf, 26) = 0;
	}

	if( tsd )
		clif->send(buf, packet_len(0x1cf), &tsd->bl, SELF);
	else
		clif->send(buf, packet_len(0x1cf), src, AREA);
}

/*==========================================
 * Server tells clients nearby 'sd' (and himself) to display 'sd->spiritball' number of spiritballs on 'sd'
 * Notifies clients in an area of an object's spirits.
 * 01d0 <id>.L <amount>.W (ZC_SPIRITS)
 * 01e1 <id>.L <amount>.W (ZC_SPIRITS2)
 *------------------------------------------*/
static void clif_spiritball(struct block_list *bl)
{
	unsigned char buf[16];

	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x1d0;
	WBUFL(buf, 2) = bl->id;
	WBUFW(buf, 6) = 0; //init to 0
	switch (bl->type) {
		case BL_PC:
		{
			struct map_session_data *sd = BL_CAST(BL_PC, bl);
			nullpo_retv(sd);
			WBUFW(buf, 6) = sd->spiritball;
			break;
		}
		case BL_HOM:
		{
			struct homun_data *hd = BL_CAST(BL_HOM, bl);
			nullpo_retv(hd);
			WBUFW(buf, 6) = hd->homunculus.spiritball;
			break;
		}
	}
	clif->send(buf, packet_len(0x1d0), bl, AREA);
}

/// Notifies clients in area of a character's combo delay (ZC_COMBODELAY).
/// 01d2 <account id>.L <delay>.L
static void clif_combo_delay(struct block_list *bl, int wait)
{
	unsigned char buf[32];

	nullpo_retv(bl);

	WBUFW(buf,0)=0x1d2;
	WBUFL(buf,2)=bl->id;
	WBUFL(buf,6)=wait;
	clif->send(buf,packet_len(0x1d2),bl,AREA);
}

/// Notifies clients in area that a character has blade-stopped another (ZC_BLADESTOP).
/// 01d1 <src id>.L <dst id>.L <flag>.L
/// flag:
///     0 = inactive
///     1 = active
static void clif_bladestop(struct block_list *src, int dst_id, int active)
{
	unsigned char buf[32];

	nullpo_retv(src);

	WBUFW(buf,0)=0x1d1;
	WBUFL(buf,2)=src->id;
	WBUFL(buf,6)=dst_id;
	WBUFL(buf,10)=active;

	clif->send(buf,packet_len(0x1d1),src,AREA);
}

/// MVP effect (ZC_MVP).
/// 010c <account id>.L
static void clif_mvp_effect(struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x10c;
	WBUFL(buf,2)=sd->bl.id;
	clif->send(buf,packet_len(0x10c),&sd->bl,AREA);
}

/// MVP item reward message (ZC_MVP_GETTING_ITEM).
/// 010a <name id>.W
/// 010a <name id>.L
static void clif_mvp_item(struct map_session_data *sd, int nameid)
{
	int view, fd;
	struct PACKET_ZC_MVP_GETTING_ITEM p;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x10a;
	if ((view = itemdb_viewid(nameid)) > 0)
		p.itemId = view;
	else
		p.itemId = nameid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// MVP EXP reward message (ZC_MVP_GETTING_SPECIAL_EXP).
/// 010b <exp>.L
static void clif_mvp_exp(struct map_session_data *sd, unsigned int exp)
{
#if PACKETVER_RE_NUM >= 20080827 || PACKETVER_MAIN_NUM >= 20090401 || defined(PACKETVER_ZERO)
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x10b));
	WFIFOW(fd, 0) = 0x10b;
	WFIFOL(fd, 2) = cap_value(exp, 0, INT32_MAX);
	WFIFOSET(fd, packet_len(0x10b));
#endif
}

/// Dropped MVP item reward message (ZC_THROW_MVPITEM).
/// 010d
///
/// "You are the MVP, but cannot obtain the reward because
///     you are overweight."
static void clif_mvp_noitem(struct map_session_data *sd)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x10d));
	WFIFOW(fd,0) = 0x10d;
	WFIFOSET(fd,packet_len(0x10d));
}

/// Guild creation result (ZC_RESULT_MAKE_GUILD).
/// 0167 <result>.B
/// result:
///     0 = "Guild has been created."
///     1 = "You are already in a Guild."
///     2 = "That Guild Name already exists."
///     3 = "You need the necessary item to create a Guild."
///     4 = "Can't create a Guild in this area."
static void clif_guild_created(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x167));
	WFIFOW(fd,0)=0x167;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x167));
}

/// Notifies the client that it is belonging to a guild (ZC_UPDATE_GDID).
/// 016c <guild id>.L <emblem id>.L <mode>.L <ismaster>.B <inter sid>.L <guild name>.24B
/// mode: @see enum guild_permission
static void clif_guild_belonginfo(struct map_session_data *sd, struct guild *g)
{
	nullpo_retv(sd);
	nullpo_retv(g);

	int fd = sd->fd;
	int ps = guild->getposition(g, sd);
	Assert_retv(ps != -1);

	WFIFOHEAD(fd,packet_len(0x16c));
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=g->guild_id;
	WFIFOL(fd,6)=g->emblem_id;
	WFIFOL(fd,10)=g->position[ps].mode;
	WFIFOB(fd,14)=(bool)(sd->state.gmaster_flag == 1);
	WFIFOL(fd,15)=0;  // InterSID (unknown purpose)
	memcpy(WFIFOP(fd,19),g->name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x16c));
}

/// Guild member login notice.
/// 016d <account id>.L <char id>.L <status>.L (ZC_UPDATE_CHARSTAT)
/// 01f2 <account id>.L <char id>.L <status>.L <gender>.W <hair style>.W <hair color>.W (ZC_UPDATE_CHARSTAT2)
/// status:
///     0 = offline
///     1 = online
static void clif_guild_memberlogin_notice(struct guild *g, int idx, int flag)
{
	unsigned char buf[64];
	struct map_session_data* sd;

	nullpo_retv(g);

	WBUFW(buf, 0)=0x1f2;
	WBUFL(buf, 2)=g->member[idx].account_id;
	WBUFL(buf, 6)=g->member[idx].char_id;
	WBUFL(buf,10)=flag;

	if( ( sd = g->member[idx].sd ) != NULL )
	{
		WBUFW(buf,14) = sd->status.sex;
		WBUFW(buf,16) = sd->status.hair;
		WBUFW(buf,18) = sd->status.hair_color;
		clif->send(buf,packet_len(0x1f2),&sd->bl,GUILD_WOS);
	}
	else if( ( sd = guild->getavailablesd(g) ) != NULL )
	{
		WBUFW(buf,14) = 0;
		WBUFW(buf,16) = 0;
		WBUFW(buf,18) = 0;
		clif->send(buf,packet_len(0x1f2),&sd->bl,GUILD);
	}
}

// Function `clif_guild_memberlogin_notice` sends info about
// logins and logouts of a guild member to the rest members.
// But at the 1st time (after a player login or map changing)
// the client won't show the message.
// So I suggest use this function for sending "first-time-info"
// to some player on entering the game or changing location.
// At next time the client would always show the message.
// The function sends all the statuses in the single packet
// to economize traffic. [LuzZza]
static void clif_guild_send_onlineinfo(struct map_session_data *sd)
{
	struct guild *g;
	unsigned char buf[14*128];
	int i, count=0, p_len;

	nullpo_retv(sd);

	p_len = packet_len(0x16d);

	if(!(g = sd->guild))
		return;

	for(i=0; i<g->max_member; i++) {

		if(g->member[i].account_id > 0 &&
			g->member[i].account_id != sd->status.account_id) {

			WBUFW(buf,count*p_len) = 0x16d;
			WBUFL(buf,count*p_len+2) = g->member[i].account_id;
			WBUFL(buf,count*p_len+6) = g->member[i].char_id;
			WBUFL(buf,count*p_len+10) = g->member[i].online;
			count++;
		}
	}

	clif->send(buf, p_len*count, &sd->bl, SELF);
}

/// Bitmask of enabled guild window tabs (ZC_ACK_GUILD_MENUINTERFACE).
/// 014e <menu flag>.L
/// menu flag:
///      0x00 = Basic Info (always on)
///     &0x01 = Member manager
///     &0x02 = Positions
///     &0x04 = Skills
///     &0x10 = Expulsion list
///     &0x40 = Unknown (GMENUFLAG_ALLGUILDLIST)
///     &0x80 = Notice
static void clif_guild_masterormember(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x14e));
	WFIFOW(fd,0) = 0x14e;
	WFIFOL(fd,2) = (sd->state.gmaster_flag) ? 0xd7 : 0x57;
	WFIFOSET(fd,packet_len(0x14e));
}

/// Guild basic information (Territories [Valaris])
/// 0150 <guild id>.L <level>.L <member num>.L <member max>.L <exp>.L <max exp>.L <points>.L <honor>.L <virtue>.L <emblem id>.L <name>.24B <master name>.24B <manage land>.16B (ZC_GUILD_INFO)
/// 01b6 <guild id>.L <level>.L <member num>.L <member max>.L <exp>.L <max exp>.L <points>.L <honor>.L <virtue>.L <emblem id>.L <name>.24B <master name>.24B <manage land>.16B <zeny>.L (ZC_GUILD_INFO2)
static void clif_guild_basicinfo(struct map_session_data *sd)
{
	int fd;
	struct guild *g;

#if PACKETVER < 20160622
	const int cmd = 0x1b6;  //0x150; [4144] this is packet for older versions?
#else
	const int cmd = 0xa84;
#endif

	nullpo_retv(sd);
	fd = sd->fd;

	if ((g = sd->guild) == NULL)
		return;

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd, 0) = cmd;
	WFIFOL(fd, 2) = g->guild_id;
	WFIFOL(fd, 6) = g->guild_lv;
	WFIFOL(fd, 10) = g->connect_member;
	WFIFOL(fd, 14) = g->max_member;
	WFIFOL(fd, 18) = g->average_lv;
	WFIFOL(fd, 22) = (uint32)cap_value(g->exp, 0, INT32_MAX);
	WFIFOL(fd, 26) = g->next_exp;
	WFIFOL(fd, 30) = 0;  // Tax Points
	WFIFOL(fd, 34) = 0;  // Honor: (left) Vulgar [-100,100] Famed (right)
	WFIFOL(fd, 38) = 0;  // Virtue: (down) Wicked [-100,100] Righteous (up)
	WFIFOL(fd, 42) = g->emblem_id;
	memcpy(WFIFOP(fd, 46), g->name, NAME_LENGTH);
#if PACKETVER < 20160622
	memcpy(WFIFOP(fd, 70), g->master, NAME_LENGTH);
	safestrncpy(WFIFOP(fd, 94), msg_sd(sd, 300 + guild->checkcastles(g)), 16);  // "'N' castles"
	WFIFOL(fd, 110) = 0;  // zeny
#else
	safestrncpy(WFIFOP(fd, 70), msg_sd(sd, 300 + guild->checkcastles(g)), 16);  // "'N' castles"
	WFIFOL(fd, 86) = 0;  // zeny
	WFIFOL(fd, 90) = g->member[0].char_id;  // leader
#endif

	WFIFOSET(fd, packet_len(cmd));
}

/// Guild alliance and opposition list (ZC_MYGUILD_BASIC_INFO).
/// 014c <packet len>.W { <relation>.L <guild id>.L <guild name>.24B }*
static void clif_guild_allianceinfo(struct map_session_data *sd)
{
	int fd,i,c;
	struct guild *g;

	nullpo_retv(sd);
	if( (g = sd->guild) == NULL )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDALLIANCE * 32 + 4);
	WFIFOW(fd, 0)=0x14c;
	for(i=c=0;i<MAX_GUILDALLIANCE;i++){
		struct guild_alliance *a=&g->alliance[i];
		if(a->guild_id>0){
			WFIFOL(fd,c*32+4)=a->opposition;
			WFIFOL(fd,c*32+8)=a->guild_id;
			memcpy(WFIFOP(fd,c*32+12),a->name,NAME_LENGTH);
			c++;
		}
	}
	WFIFOW(fd, 2)=c*32+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

static void clif_guild_castlelist(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
	nullpo_retv(sd);

	struct guild *g = sd->guild;
	if (g == NULL)
		return;

	int castle_count = guild->checkcastles(g);
	if (castle_count > 0) {
		int len = sizeof(struct PACKET_ZC_GUILD_CASTLE_LIST) + castle_count;
		struct PACKET_ZC_GUILD_CASTLE_LIST *p = aMalloc(len);
		p->packetType = HEADER_ZC_GUILD_CASTLE_LIST;
		p->packetLength = len;

		int i = 0;
		struct DBIterator *iter = db_iterator(guild->castle_db);
		for (struct guild_castle *gc = dbi_first(iter); dbi_exists(iter); gc = dbi_next(iter)) {
			if (gc->guild_id == g->guild_id) {
				p->castle_list[i] = gc->castle_id;
				++i;
			}
		}
		dbi_destroy(iter);

		clif->send(p, len, &sd->bl, SELF);
		aFree(p);
	}
#endif
}

static void clif_guild_castleinfo(struct map_session_data *sd, struct guild_castle *gc)
{
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814

	nullpo_retv(sd);
	nullpo_retv(gc);

	struct PACKET_ZC_CASTLE_INFO p = { 0 };
	p.packetType = HEADER_ZC_CASTLE_INFO;
	p.castle_id = gc->castle_id;
	p.economy = gc->economy;
	p.defense = gc->defense;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/// Guild member manager information (ZC_MEMBERMGR_INFO).
/// 0154 <packet len>.W { <account>.L <char id>.L <hair style>.W <hair color>.W <gender>.W <class>.W <level>.W <contrib exp>.L <state>.L <position>.L <memo>.50B <name>.24B }*
/// state:
///     0 = offline
///     1 = online
/// memo:
///     probably member's self-introduction (unused, no client UI/packets for editing it)
static void clif_guild_memberlist(struct map_session_data *sd)
{
	int fd;
	int i,c;
	struct guild *g;
#if PACKETVER < 20161026
	const int cmd = 0x154;
	const int size = 104;
#else
	const int cmd = 0xaa5;
	const int size = 34;
#endif

	nullpo_retv(sd);

	if ((fd = sd->fd) == 0)
		return;
	if ((g = sd->guild) == NULL)
		return;

	WFIFOHEAD(fd, g->max_member * size + 4);
	WFIFOW(fd, 0) = cmd;
	for (i = 0, c = 0; i < g->max_member; i++) {
		struct guild_member *m = &g->member[i];
		if (m->account_id == 0)
			continue;
		WFIFOL(fd, c * size + 4) = m->account_id;
		WFIFOL(fd, c * size + 8) = m->char_id;
		WFIFOW(fd, c * size + 12) = m->hair;
		WFIFOW(fd, c * size + 14) = m->hair_color;
		WFIFOW(fd, c * size + 16) = m->gender;
		WFIFOW(fd, c * size + 18) = m->class;
		WFIFOW(fd, c * size + 20) = m->lv;
		WFIFOL(fd, c * size + 22) = (int)cap_value(m->exp, 0, INT32_MAX);
		WFIFOL(fd, c * size + 26) = m->online;
		WFIFOL(fd, c * size + 30) = m->position;
#if PACKETVER < 20161026
		memset(WFIFOP(fd, c * size + 34), 0, 50);  //[Ind] - This is displayed in the 'note' column but being you can't edit it it's sent empty.
		memcpy(WFIFOP(fd, c * size + 84), m->name, NAME_LENGTH);
#else
		WFIFOL(fd, c * size + 34) = m->last_login;  // [Megasantos] - Shows last date online
#endif
		c++;
	}
	WFIFOW(fd, 2) = c * size + 4;
	WFIFOSET(fd, WFIFOW(fd, 2));
}

/// Guild position name information (ZC_POSITION_ID_NAME_INFO).
/// 0166 <packet len>.W { <position id>.L <position name>.24B }*
static void clif_guild_positionnamelist(struct map_session_data *sd)
{
	int i,fd;
	struct guild *g;

	nullpo_retv(sd);
	if( (g = sd->guild) == NULL )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDPOSITION * 28 + 4);
	WFIFOW(fd, 0)=0x166;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		WFIFOL(fd,i*28+4)=i;
		memcpy(WFIFOP(fd,i*28+8),g->position[i].name,NAME_LENGTH);
	}
	WFIFOW(fd,2)=i*28+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Guild position information (ZC_POSITION_INFO).
/// 0160 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L }*
/// mode: @see enum guild_permission
/// ranking:
///     TODO
static void clif_guild_positioninfolist(struct map_session_data *sd)
{
	int i,fd;
	struct guild *g;

	nullpo_retv(sd);
	if( (g = sd->guild) == NULL )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDPOSITION * 16 + 4);
	WFIFOW(fd, 0)=0x160;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		struct guild_position *p=&g->position[i];
		WFIFOL(fd,i*16+ 4)=i;
		WFIFOL(fd,i*16+ 8)=p->mode;
		WFIFOL(fd,i*16+12)=i;
		WFIFOL(fd,i*16+16)=p->exp_mode;
	}
	WFIFOW(fd, 2)=i*16+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Notifies clients in a guild about updated position information (ZC_ACK_CHANGE_GUILD_POSITIONINFO).
/// 0174 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L <position name>.24B }*
/// mode: @see enum guild_permission
/// ranking:
///     TODO
static void clif_guild_positionchanged(struct guild *g, int idx)
{
	// FIXME: This packet is intended to update the clients after a
	// commit of position info changes, not sending one packet per
	// position.
	struct map_session_data *sd;
	unsigned char buf[128];

	nullpo_retv(g);

	WBUFW(buf, 0)=0x174;
	WBUFW(buf, 2)=44;  // packet len
	// GUILD_REG_POSITION_INFO{
	WBUFL(buf, 4)=idx;
	WBUFL(buf, 8)=g->position[idx].mode;
	WBUFL(buf,12)=idx;
	WBUFL(buf,16)=g->position[idx].exp_mode;
	memcpy(WBUFP(buf,20),g->position[idx].name,NAME_LENGTH);
	// }*
	if( (sd=guild->getavailablesd(g))!=NULL )
		clif->send(buf,WBUFW(buf,2),&sd->bl,GUILD);
}

/// Notifies clients in a guild about updated member position assignments (ZC_ACK_REQ_CHANGE_MEMBERS).
/// 0156 <packet len>.W { <account id>.L <char id>.L <position id>.L }*
static void clif_guild_memberpositionchanged(struct guild *g, int idx)
{
	// FIXME: This packet is intended to update the clients after a
	// commit of member position assignment changes, not sending one
	// packet per position.
	struct map_session_data *sd;
	unsigned char buf[64];

	nullpo_retv(g);

	WBUFW(buf, 0)=0x156;
	WBUFW(buf, 2)=16;  // packet len
	// MEMBER_POSITION_INFO{
	WBUFL(buf, 4)=g->member[idx].account_id;
	WBUFL(buf, 8)=g->member[idx].char_id;
	WBUFL(buf,12)=g->member[idx].position;
	// }*
	if( (sd=guild->getavailablesd(g))!=NULL )
		clif->send(buf,WBUFW(buf,2),&sd->bl,GUILD);
}

/// Sends emblems bitmap data to the client that requested it (ZC_GUILD_EMBLEM_IMG).
/// 0152 <packet len>.W <guild id>.L <emblem id>.L <emblem data>.?B
static void clif_guild_emblem(struct map_session_data *sd, struct guild *g)
{
	int fd;
	nullpo_retv(sd);
	nullpo_retv(g);

	fd = sd->fd;
	if( g->emblem_len <= 0 )
		return;

	WFIFOHEAD(fd,g->emblem_len+12);
	WFIFOW(fd,0)=0x152;
	WFIFOW(fd,2)=g->emblem_len+12;
	WFIFOL(fd,4)=g->guild_id;
	WFIFOL(fd,8)=g->emblem_id;
	memcpy(WFIFOP(fd,12),g->emblem_data,g->emblem_len);
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Sends update of the guild id/emblem id to everyone in the area (ZC_CHANGE_GUILD).
/// 01b4 <id>.L <guild id>.L <emblem id>.W
static void clif_guild_emblem_area(struct block_list *bl)
{
	uint8 buf[12];

	nullpo_retv(bl);

	// TODO this packet doesn't force the update of ui components that have the emblem visible
	//      (emblem in the flag npcs and emblem over the head in agit maps) [FlavioJS]
	WBUFW(buf,0) = 0x1b4;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = status->get_guild_id(bl);
	WBUFW(buf,10) = status->get_emblem_id(bl);
	clif->send(buf, 12, bl, AREA_WOS);
}

/// Sends guild skills (ZC_GUILD_SKILLINFO).
/// 0162 <packet len>.W <skill points>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <atk range>.W <skill name>.24B <upgradeable>.B }*
static void clif_guild_skillinfo(struct map_session_data *sd)
{
	int fd;
	struct guild* g;
	int i,c;

	nullpo_retv(sd);
	if( (g = sd->guild) == NULL )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, 6 + MAX_GUILDSKILL*37);
	WFIFOW(fd,0) = 0x0162;
	WFIFOW(fd,4) = g->skill_point;
	for(i = 0, c = 0; i < MAX_GUILDSKILL; i++) {
		if(g->skill[i].id > 0 && guild->check_skill_require(g, g->skill[i].id)) {
			int id = g->skill[i].id;
			int p = 6 + c*37;
			WFIFOW(fd,p+0) = id;
			WFIFOL(fd,p+2) = skill->get_inf(id);
			WFIFOW(fd,p+6) = g->skill[i].lv;
			if ( g->skill[i].lv ) {
				WFIFOW(fd, p + 8) = skill->get_sp(id, g->skill[i].lv);
				WFIFOW(fd, p + 10) = skill->get_range(id, g->skill[i].lv);
			} else {
				WFIFOW(fd, p + 8) = 0;
				WFIFOW(fd, p + 10) = 0;
			}
			safestrncpy(WFIFOP(fd,p+12), skill->get_name(id), NAME_LENGTH);
			WFIFOB(fd,p+36)= (g->skill[i].lv < guild->skill_get_max(id) && sd == g->member[0].sd) ? 1 : 0;
			c++;
		}
	}
	WFIFOW(fd,2) = 6 + c*37;
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Sends guild notice to client (ZC_GUILD_NOTICE).
/// 016f <subject>.60B <notice>.120B
static void clif_guild_notice(struct map_session_data *sd, struct guild *g)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(g);

	fd = sd->fd;

	if (!sockt->session_is_active(fd))
		return;

	if(g->mes1[0] == '\0' && g->mes2[0] == '\0')
		return;

	WFIFOHEAD(fd,packet_len(0x16f));
	WFIFOW(fd,0) = 0x16f;
	memcpy(WFIFOP(fd,2), g->mes1, MAX_GUILDMES1);
	memcpy(WFIFOP(fd,62), g->mes2, MAX_GUILDMES2);
	WFIFOSET(fd,packet_len(0x16f));
}

/// Guild invite (ZC_REQ_JOIN_GUILD).
/// 016a <guild id>.L <guild name>.24B
static void clif_guild_invite(struct map_session_data *sd, struct guild *g)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(g);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x16a));
	WFIFOW(fd,0)=0x16a;
	WFIFOL(fd,2)=g->guild_id;
	memcpy(WFIFOP(fd,6),g->name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x16a));
}

/// Reply to invite request (ZC_ACK_REQ_JOIN_GUILD).
/// 0169 <answer>.B
/// answer:
///     0 = Already in guild.
///     1 = Offer rejected.
///     2 = Offer accepted.
///     3 = Guild full.
///     4 = Offline or not exists
static void clif_guild_inviteack(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x169));
	WFIFOW(fd,0)=0x169;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x169));
}

/// Notifies clients of a guild of a leaving member (ZC_ACK_LEAVE_GUILD).
/// 015a <char name>.24B <reason>.40B
static void clif_guild_leave(struct map_session_data *sd, const char *name, int char_id, const char *mes)
{
	nullpo_retv(sd);
	nullpo_retv(name);
	nullpo_retv(mes);

	struct PACKET_ZC_ACK_LEAVE_GUILD p;
	p.packetType = guildLeave;
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	p.GID = char_id;
#else
	safestrncpy(&p.name[0], name, NAME_LENGTH);
#endif
	safestrncpy(&p.reason[0], mes, 40);
	clif->send(&p, sizeof(p), &sd->bl, GUILD_NOBG);
}

/// Notifies clients of a guild of an expelled member.
/// 015c <char name>.24B <reason>.40B <account name>.24B (ZC_ACK_BAN_GUILD)
/// 0839 <char name>.24B <reason>.40B (ZC_ACK_BAN_GUILD_SSO)
static void clif_guild_expulsion(struct map_session_data *sd, const char *name, int char_id, const char *mes, int account_id)
{
	nullpo_retv(sd);
	nullpo_retv(name);
	nullpo_retv(mes);

	struct PACKET_ZC_ACK_BAN_GUILD p;
	p.packetType = guildExpulsion;
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	p.GID = char_id;
#else
	safestrncpy(&p.name[0], name, NAME_LENGTH);
#endif
	safestrncpy(&p.reason[0], mes, 40);

// version unconfirmed
#if PACKETVER < 20100803
	memset(&p.account_name, 0, NAME_LENGTH); // account name (not used for security reasons)
#endif
	clif->send(&p, sizeof(p), &sd->bl, GUILD_NOBG);
}

/// Guild expulsion list (ZC_BAN_LIST).
/// 0163 <packet len>.W { <char name>.24B <account name>.24B <reason>.40B }*
/// 0163 <packet len>.W { <char name>.24B <reason>.40B }* (PACKETVER >= 20100803)
static void clif_guild_expulsionlist(struct map_session_data *sd)
{
	nullpo_retv(sd);

	int c = 0;

	struct guild* g;
	if ((g = sd->guild) == NULL)
		return;

	int fd = sd->fd;

	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_BAN_LIST) + MAX_GUILDEXPULSION * sizeof(struct PACKET_ZC_BAN_LIST_sub));
	struct PACKET_ZC_BAN_LIST *packet = WFIFOP(fd, 0);
	packet->packetType = HEADER_ZC_BAN_LIST;

	for (int i = 0; i < MAX_GUILDEXPULSION; i++)
	{
		struct guild_expulsion* e = &g->expulsion[i];

		if (e->account_id > 0)
		{
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
			packet->chars[c].char_id = e->char_id;
// version unconfirmed
#elif PACKETVER >= 20100803
			memcpy(packet->chars[c].char_name, e->name, NAME_LENGTH);

#else
			memcpy(packet->chars[c].char_name, e->name, NAME_LENGTH);
			memset(packet->chars[c].account_name, 0, NAME_LENGTH); // account name (not used for security reasons)

#endif
			memcpy(packet->chars[c].message, e->mes, 40);

			c ++;
		}
	}
	packet->packetLen = sizeof(struct PACKET_ZC_BAN_LIST) + c * sizeof(struct PACKET_ZC_BAN_LIST_sub);
	WFIFOSET(fd, packet->packetLen);
}

/// Guild chat message (ZC_GUILD_CHAT).
/// 017f <packet len>.W <message>.?B
static void clif_guild_message(struct guild *g, int account_id, const char *mes, int len)
{// TODO: account_id is not used, candidate for deletion? [Ai4rei]
	struct map_session_data *sd;
	uint8 buf[256];

	nullpo_retv(mes);
	if (len == 0)
		return;

	if (len > sizeof(buf)-5) {
		ShowWarning("clif_guild_message: Truncated message '%s' (len=%d, max=%"PRIuS", guild_id=%d).\n", mes, len, sizeof(buf)-5, g->guild_id);
		len = sizeof(buf)-5;
	}

	WBUFW(buf, 0) = 0x17f;
	WBUFW(buf, 2) = len + 5;
	safestrncpy(WBUFP(buf,4), mes, len+1);

	if ((sd = guild->getavailablesd(g)) != NULL)
		clif->send(buf, WBUFW(buf,2), &sd->bl, GUILD_NOBG);
}

/// Request for guild alliance (ZC_REQ_ALLY_GUILD).
/// 0171 <inviter account id>.L <guild name>.24B
static void clif_guild_reqalliance(struct map_session_data *sd, int account_id, const char *name)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(name);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x171));
	WFIFOW(fd,0)=0x171;
	WFIFOL(fd,2)=account_id;
	memcpy(WFIFOP(fd,6),name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x171));
}

/// Notifies the client about the result of a alliance request (ZC_ACK_REQ_ALLY_GUILD).
/// 0173 <answer>.B
/// answer:
///     0 = Already allied.
///     1 = You rejected the offer.
///     2 = You accepted the offer.
///     3 = They have too any alliances.
///     4 = You have too many alliances.
///     5 = Alliances are disabled.
static void clif_guild_allianceack(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x173));
	WFIFOW(fd,0)=0x173;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x173));
}

/// Notifies the client that a alliance or opposition has been removed (ZC_DELETE_RELATED_GUILD).
/// 0184 <other guild id>.L <relation>.L
/// relation:
///     0 = Ally
///     1 = Enemy
static void clif_guild_delalliance(struct map_session_data *sd, int guild_id, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	if (fd <= 0)
		return;
	WFIFOHEAD(fd,packet_len(0x184));
	WFIFOW(fd,0)=0x184;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=flag;
	WFIFOSET(fd,packet_len(0x184));
}

/// Notifies the client about the result of a opposition request (ZC_ACK_REQ_HOSTILE_GUILD).
/// 0181 <result>.B
/// result:
///     0 = Antagonist has been set.
///     1 = Guild has too many Antagonists.
///     2 = Already set as an Antagonist.
///     3 = Antagonists are disabled.
static void clif_guild_oppositionack(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x181));
	WFIFOW(fd,0)=0x181;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x181));
}

/// Adds alliance or opposition (ZC_ADD_RELATED_GUILD).
/// 0185 <relation>.L <guild id>.L <guild name>.24B
#if 0
static void clif_guild_allianceadded(struct guild *g, int idx)
{
	unsigned char buf[64];
	WBUFW(buf,0)=0x185;
	WBUFL(buf,2)=g->alliance[idx].opposition;
	WBUFL(buf,6)=g->alliance[idx].guild_id;
	memcpy(WBUFP(buf,10),g->alliance[idx].name,NAME_LENGTH);
	clif->send(buf,packet_len(0x185),guild->getavailablesd(g),GUILD);
}
#endif // 0

/// Notifies the client about the result of a guild break (ZC_ACK_DISORGANIZE_GUILD_RESULT).
/// 015e <reason>.L
///     0 = success
///     1 = invalid key (guild name, @see clif_parse_GuildBreak)
///     2 = there are still members in the guild
static void clif_guild_broken(struct map_session_data *sd, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x15e));
	WFIFOW(fd,0)=0x15e;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x15e));
}

static void clif_guild_position_selected(struct map_session_data *sd)
{
#if PACKETVER >= 20180801
	clif->guild_set_position(sd);
#else
	clif->charnameupdate(sd);
#endif
}

static void clif_guild_set_position(struct map_session_data *sd)
{
	nullpo_retv(sd);

	int len = sizeof(struct PACKET_ZC_GUILD_POSITION);
	const char *name = NULL;
	if (sd->status.guild_id > 0) {
		struct guild *g = sd->guild;

		nullpo_retv(g);

		int i = 0;
		int ps = -1;
		ARR_FIND(0, g->max_member, i, g->member[i].account_id == sd->status.account_id && g->member[i].char_id == sd->status.char_id);
		if (i < g->max_member)
			ps = g->member[i].position;

		if (ps >= 0 && ps < MAX_GUILDPOSITION) {
			len += 24;
			name = g->position[ps].name;
		}
	}

	unsigned char buf[sizeof(struct PACKET_ZC_GUILD_POSITION) + NAME_LENGTH];
	struct PACKET_ZC_GUILD_POSITION *p = WBUFP(buf, 0);
	p->packetType = 0xafd;
	p->packetLength = len;
	p->AID = sd->bl.id;
	if (name != NULL)
		memcpy(&p->position, name, 24);

	clif->send(buf, len, &sd->bl, AREA);
}

/// Displays emotion on an object (ZC_EMOTION).
/// 00c0 <id>.L <type>.B
/// type:
///     enum emotion_type
static void clif_emotion(struct block_list *bl, int type)
{
	unsigned char buf[8];

	nullpo_retv(bl);

	WBUFW(buf,0)=0xc0;
	WBUFL(buf,2)=bl->id;
	WBUFB(buf,6)=type;
	clif->send(buf,packet_len(0xc0),bl,AREA);
}

/// Displays the contents of a talkiebox trap (ZC_TALKBOX_CHATCONTENTS).
/// 0191 <id>.L <contents>.80B
static void clif_talkiebox(struct block_list *bl, const char *talkie)
{
	nullpo_retv(bl);
	nullpo_retv(talkie);
	struct PACKET_ZC_TALKBOX_CHATCONTENTS p;

	p.PacketType = HEADER_ZC_TALKBOX_CHATCONTENTS;
	p.aid = bl->id;
	safestrncpy(&p.message[0], talkie, TALKBOX_MESSAGE_SIZE);
	clif->send(&p, sizeof(struct PACKET_ZC_TALKBOX_CHATCONTENTS), bl, AREA);
}

/// Displays wedding effect centered on an object (ZC_CONGRATULATION).
/// 01ea <id>.L
static void clif_wedding_effect(struct block_list *bl)
{
	unsigned char buf[6];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x1ea;
	WBUFL(buf,2) = bl->id;
	clif->send(buf, packet_len(0x1ea), bl, AREA);
}

/// Notifies the client of the name of the partner character (ZC_COUPLENAME).
/// 01e6 <partner name>.24B
static void clif_callpartner(struct map_session_data *sd)
{
	unsigned char buf[26];

	nullpo_retv(sd);

	WBUFW(buf,0) = 0x1e6;

	if( sd->status.partner_id ) {
		const char *p;
		if( ( p = map->charid2nick(sd->status.partner_id) ) != NULL ) {
			memcpy(WBUFP(buf,2), p, NAME_LENGTH);
		} else {
			WBUFB(buf,2) = 0;
		}
	} else {
		// Send zero-length name if no partner, to initialize the client buffer.
		WBUFB(buf,2) = 0;
	}

	clif->send(buf, packet_len(0x1e6), &sd->bl, AREA);
}

/// Initiates the partner "taming" process [DracoRPG] (ZC_START_COUPLE).
/// 01e4
/// This packet while still implemented by the client is no longer being officially used.
#if 0
static void clif_marriage_process(struct map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x1e4));
	WFIFOW(fd,0)=0x1e4;
	WFIFOSET(fd,packet_len(0x1e4));
}
#endif // 0

/// Notice of divorce (ZC_DIVORCE).
/// 0205 <partner name>.24B
static void clif_divorced(struct map_session_data *sd, const char *name)
{
	int fd;
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x205));
	WFIFOW(fd,0)=0x205;
	memcpy(WFIFOP(fd,2), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x205));
}

/// Marriage proposal (ZC_REQ_COUPLE).
/// 01e2 <account id>.L <char id>.L <char name>.24B
/// This packet while still implemented by the client is no longer being officially used.
#if 0
static void clif_marriage_proposal(int fd, struct map_session_data *sd, struct map_session_data *ssd)
{
	nullpo_retv(sd);

	WFIFOHEAD(fd,packet_len(0x1e2));
	WFIFOW(fd,0) = 0x1e2;
	WFIFOL(fd,2) = ssd->status.account_id;
	WFIFOL(fd,6) = ssd->status.char_id;
	safestrncpy(WFIFOP(fd,10), ssd->status.name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x1e2));
}
#endif // 0

/*==========================================
 * Displays a message using the guild-chat colors to the specified targets. [Skotlex]
 *------------------------------------------*/
static void clif_disp_message(struct block_list *src, const char *mes, enum send_target target)
{
	unsigned char buf[256];
	int len;

	nullpo_retv(mes);
	nullpo_retv(src);

	len = (int)strlen(mes);
	if (len == 0)
		return;

	if (len > (int)sizeof(buf)-5) {
		ShowWarning("clif_disp_message: Truncated message '%s' (len=%d, max=%"PRIuS", aid=%d).\n", mes, len, sizeof(buf)-5, src->id);
		len = (int)sizeof(buf)-5;
	}

	WBUFW(buf, 0) = 0x17f;
	WBUFW(buf, 2) = len + 5;
	safestrncpy(WBUFP(buf,4), mes, len+1);
	clif->send(buf, WBUFW(buf,2), src, target);
}

/// Notifies the client about the result of a request to disconnect another player (ZC_ACK_DISCONNECT_CHARACTER).
/// 00cd <result>.L (unknown packet version or invalid information at packet_len_table)
/// 00cd <result>.B
/// result:
///     0 = failure
///     1 = success
static void clif_GM_kickack(struct map_session_data *sd, int result)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xcd));
	WFIFOW(fd,0) = 0xcd;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd, packet_len(0xcd));
}

static void clif_GM_kick(struct map_session_data *sd, struct map_session_data *tsd)
{
	int fd;

	nullpo_retv(tsd);
	fd = tsd->fd;

	if (fd > 0)
		clif->authfail_fd(fd, 15);
	else
		map->quit(tsd);

	if (sd)
		clif->GM_kickack(sd, 1);
}

/// Displays various manner-related status messages (ZC_ACK_GIVE_MANNER_POINT).
/// 014a <result>.L
/// result:
///     0 = "A manner point has been successfully aligned."
///     1 = MP_FAILURE_EXHAUST
///     2 = MP_FAILURE_ALREADY_GIVING
///     3 = "Chat Block has been applied by GM due to your ill-mannerous action."
///     4 = "Automated Chat Block has been applied due to Anti-Spam System."
///     5 = "You got a good point from %s."
static void clif_manner_message(struct map_session_data *sd, uint32 type)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x14a));
	WFIFOW(fd,0) = 0x14a;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd, packet_len(0x14a));
}

/// Follow-up to 0x14a type 3/5, informs who did the manner adjustment action (ZC_NOTIFY_MANNER_POINT_GIVEN).
/// 014b <type>.B <GM name>.24B
/// type:
///     0 = positive (unmute)
///     1 = negative (mute)
static void clif_GM_silence(struct map_session_data *sd, struct map_session_data *tsd, uint8 type)
{
	int fd;
	nullpo_retv(sd);
	nullpo_retv(tsd);

	fd = tsd->fd;
	WFIFOHEAD(fd,packet_len(0x14b));
	WFIFOW(fd,0) = 0x14b;
	WFIFOB(fd,2) = type;
	safestrncpy(WFIFOP(fd,3), sd->status.name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x14b));
}

/// Notifies the client about the result of a request to allow/deny whispers from a player (ZC_SETTING_WHISPER_PC).
/// 00d1 <type>.B <result>.B
/// type:
///     0 = /ex (deny)
///     1 = /in (allow)
/// result:
///     0 = success
///     1 = failure
///     2 = too many blocks
static void clif_wisexin(struct map_session_data *sd, int type, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xd1));
	WFIFOW(fd,0)=0xd1;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len(0xd1));
}

/// Notifies the client about the result of a request to allow/deny whispers from anyone (ZC_SETTING_WHISPER_STATE).
/// 00d2 <type>.B <result>.B
/// type:
///     0 = /exall (deny)
///     1 = /inall (allow)
/// result:
///     0 = success
///     1 = failure
static void clif_wisall(struct map_session_data *sd, int type, int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xd2));
	WFIFOW(fd,0)=0xd2;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len(0xd2));
}

/// Play a BGM! [Rikter/Yommy] (ZC_PLAY_NPC_BGM).
/// 07fe <bgm>.24B
static void clif_playBGM(struct map_session_data *sd, const char *name)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x7fe));
	WFIFOW(fd,0) = 0x7fe;
	safestrncpy(WFIFOP(fd,2), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x7fe));
}

/// Plays/stops a wave sound (ZC_SOUND).
/// 01d3 <file name>.24B <act>.B <term>.L <npc id>.L
/// file name:
///     relative to data\wav
/// act:
///     0 = play (once)
///     1 = play (repeat, does not work)
///     2 = stops all sound instances of file name (does not work)
/// term:
///     unknown purpose, only relevant to act = 1
/// npc id:
///     The acoustic direction of the sound is determined by the
///     relative position of the NPC to the player (3D sound).
static void clif_soundeffect(struct map_session_data *sd, struct block_list *bl, const char *name, int type)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(bl);
	nullpo_retv(name);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x1d3));
	WFIFOW(fd,0) = 0x1d3;
	safestrncpy(WFIFOP(fd,2), name, NAME_LENGTH);
	WFIFOB(fd,26) = type;
	WFIFOL(fd,27) = 0;
	WFIFOL(fd,31) = bl->id;
	WFIFOSET(fd,packet_len(0x1d3));
}

static void clif_soundeffectall(struct block_list *bl, const char *name, int type, enum send_target coverage)
{
	unsigned char buf[40];

	nullpo_retv(bl);
	nullpo_retv(name);

	WBUFW(buf,0) = 0x1d3;
	safestrncpy(WBUFP(buf,2), name, NAME_LENGTH);
	WBUFB(buf,26) = type;
	WBUFL(buf,27) = 0;
	WBUFL(buf,31) = bl->id;
	clif->send(buf, packet_len(0x1d3), bl, coverage);
}

/// Displays special effects (npcs, weather, etc) [Valaris] (ZC_NOTIFY_EFFECT2).
/// 01f3 <id>.L <effect id>.L
/// effect id:
///     @see doc/effect_list.txt
static void clif_specialeffect(struct block_list *bl, int type, enum send_target target)
{
	unsigned char buf[24];

	nullpo_retv(bl);

	memset(buf, 0, packet_len(0x1f3));

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	clif->send(buf, packet_len(0x1f3), bl, target);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2) = -bl->id;
		clif->send(buf, packet_len(0x1f3), bl, SELF);
	}
}

static void clif_specialeffect_single(struct block_list *bl, int type, int fd)
{
	nullpo_retv(bl);
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x1f3;
	WFIFOL(fd,2) = bl->id;
	WFIFOL(fd,6) = type;
	WFIFOSET(fd,10);
}

/// Notifies clients of an special/visual effect that accepts an value (ZC_NOTIFY_EFFECT3).
/// 0284 <id>.L <effect id>.L <num data>.L
/// effect id:
///     @see doc/effect_list.txt
/// num data:
///     effect-dependent value
static void clif_specialeffect_value(struct block_list *bl, int effect_id, uint64 num, send_target target)
{
#if PACKETVER_MAIN_NUM >= 20060911 || PACKETVER_AD_NUM >= 20060911 || PACKETVER_SAK_NUM >= 20060911 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO)
	struct PACKET_ZC_NOTIFY_EFFECT3 packet;
	packet.packetType = HEADER_ZC_NOTIFY_EFFECT3;
	packet.aid = bl->id;
	packet.effectId = effect_id;
#if PACKETVER >= 20191127
	packet.num = num;
#else
	packet.num = (uint32)num;
#endif

	clif->send(&packet, sizeof(struct PACKET_ZC_NOTIFY_EFFECT3), bl, target);

	if (clif->isdisguised(bl)) {
		packet.aid = -bl->id;
		clif->send(&packet, sizeof(struct PACKET_ZC_NOTIFY_EFFECT3), bl, SELF);
	}
#endif
}

static void clif_specialeffect_value_single(struct block_list *bl, int effect_id, uint64 num, int fd)
{
#if PACKETVER_MAIN_NUM >= 20060911 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO)
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_NOTIFY_EFFECT3));

	struct PACKET_ZC_NOTIFY_EFFECT3 *packet = WFIFOP(fd, 0);
	packet->packetType = HEADER_ZC_NOTIFY_EFFECT3;
	packet->aid = bl->id;
	packet->effectId = effect_id;
#if PACKETVER >= 20191127
	packet->num = num;
#else
	packet->num = (uint32)num;
#endif
	WFIFOSET(fd, sizeof(struct PACKET_ZC_NOTIFY_EFFECT3));
#endif
}

/// Remove special effects (ZC_REMOVE_EFFECT).
/// 0b0d <id>.L <effect id>.L
/// effect id:
///     @see doc/effect_list.txt
static void clif_removeSpecialEffect(struct block_list *bl, int effectId, enum send_target target)
{
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	nullpo_retv(bl);

	struct PACKET_ZC_REMOVE_EFFECT p;
	p.packetType = 0xb0d;
	p.aid = bl->id;
	p.effectId = effectId;

	clif->send(&p, sizeof(p), bl, target);

	if (clif->isdisguised(bl)) {
		p.aid = -bl->id;
		clif->send(&p, sizeof(p), bl, SELF);
	}
#endif
}

static void clif_removeSpecialEffect_single(struct block_list *bl, int effectId, struct block_list *targetBl)
{
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	nullpo_retv(bl);
	nullpo_retv(targetBl);

	struct PACKET_ZC_REMOVE_EFFECT p;
	p.packetType = 0xb0d;
	p.aid = bl->id;
	p.effectId = effectId;

	clif->send(&p, sizeof(p), targetBl, SELF);
#endif
}

/**
 * Modification of clif_messagecolor to send colored messages to players to chat log only (doesn't display overhead).
 *
 * 02c1 <packet len>.W <id>.L <color>.L <message>.?B
 *
 * @param fd    Target fd to send the message to
 * @param color Message color (RGB format: 0xRRGGBB)
 * @param msg   Message text
 */
static void clif_messagecolor_self(int fd, uint32 color, const char *msg)
{
	int msg_len;

	nullpo_retv(msg);
	msg_len = (int)strlen(msg) + 1;
	Assert_retv(msg_len <= INT16_MAX - 12);

	WFIFOHEAD(fd,msg_len + 12);
	WFIFOW(fd,0) = 0x2C1;
	WFIFOW(fd,2) = msg_len + 12;
	WFIFOL(fd,4) = 0;
	WFIFOL(fd,8) = RGB2BGR(color);
	safestrncpy(WFIFOP(fd,12), msg, msg_len);
	WFIFOSET(fd, msg_len + 12);
}

/**
 * Monster/NPC color chat [SnakeDrak] (ZC_NPC_CHAT).
 *
 * 02c1 <packet len>.W <id>.L <color>.L <message>.?B
 *
 * @param bl    Source block list.
 * @param color Message color (RGB format: 0xRRGGBB)
 * @param msg   Message text
 */
static void clif_messagecolor(struct block_list *bl, uint32 color, const char *msg)
{
	int msg_len;
	uint8 buf[256];

	nullpo_retv(bl);
	nullpo_retv(msg);

	msg_len = (int)strlen(msg) + 1;

	if (msg_len > (int)sizeof(buf)-12) {
		ShowWarning("clif_messagecolor: Truncating too long message '%s' (len=%d).\n", msg, msg_len);
		msg_len = (int)sizeof(buf)-12;
	}

	WBUFW(buf,0) = 0x2C1;
	WBUFW(buf,2) = msg_len + 12;
	WBUFL(buf,4) = bl->id;
	WBUFL(buf,8) = RGB2BGR(color);
	memcpy(WBUFP(buf,12), msg, msg_len);

	clif->send(buf, WBUFW(buf,2), bl, AREA_CHAT_WOC);
}

// Message without owner, not logged in chat
static void clif_serviceMessageColor(struct map_session_data *sd, uint32 color, const char *msg)
{
#if PACKETVER_MAIN_NUM >= 20170830 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	nullpo_retv(msg);

	int msg_len = (int)strlen(msg) + 1;

	if (msg_len > 512) {
		ShowWarning("clif_serviceMessageColor: Truncating too long message '%s' (len=%d).\n", msg, msg_len);
		msg_len = 512;
	}

	const int len = sizeof(struct PACKET_ZC_SERVICE_MESSAGE_COLOR) + msg_len;
	const int fd = sd->fd;
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_SERVICE_MESSAGE_COLOR *p = WFIFOP(fd, 0);

	p->packetType = HEADER_ZC_SERVICE_MESSAGE_COLOR;
	p->packetLength = len;
	p->color = RGB2BGR(color);
	safestrncpy(p->message, msg, msg_len);

	WFIFOSET(fd, len);
#endif
}

/**
 * Notifies the client that the storage window is still open
 *
 * Should only be used in cases where the client closed the
 * storage window without server's consent
 **/
static void clif_refresh_storagewindow(struct map_session_data *sd)
{
	nullpo_retv(sd);
	// Notify the client that the storage is open
	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL) {
		if (sd->storage.aggregate > 0) {
			storage->sortitem(VECTOR_DATA(sd->storage.item), VECTOR_LENGTH(sd->storage.item));
		}
		clif->storageList(sd, VECTOR_DATA(sd->storage.item), VECTOR_LENGTH(sd->storage.item));
		clif->updatestorageamount(sd, sd->storage.aggregate, MAX_STORAGE);
	}
	// Notify the client that the gstorage is open otherwise it will
	// remain locked forever and nobody will be able to access it
	if (sd->state.storage_flag == STORAGE_FLAG_GUILD) {
		struct guild_storage *gstor;
		if( (gstor = idb_get(gstorage->db,sd->status.guild_id)) == NULL) {
			// Shouldn't happen... The information should already be at the map-server
			intif->request_guild_storage(sd->status.account_id,sd->status.guild_id);
		} else {
			storage->sortitem(gstor->items, ARRAYLENGTH(gstor->items));
			clif->guildStorageList(sd, gstor->items, ARRAYLENGTH(gstor->items));
			clif->updatestorageamount(sd, gstor->storage_amount, MAX_GUILD_STORAGE);
		}
	}
}

// refresh the client's screen, getting rid of any effects
static void clif_refresh(struct map_session_data *sd)
{
	nullpo_retv(sd);

	clif->changemap(sd,sd->bl.m,sd->bl.x,sd->bl.y);
	clif->inventoryList(sd);
	if(pc_iscarton(sd)) {
		clif->cartList(sd);
		clif->updatestatus(sd,SP_CARTINFO);
	}
	clif->updatestatus(sd,SP_WEIGHT);
	clif->updatestatus(sd,SP_MAXWEIGHT);
	clif->updatestatus(sd,SP_STR);
	clif->updatestatus(sd,SP_AGI);
	clif->updatestatus(sd,SP_VIT);
	clif->updatestatus(sd,SP_INT);
	clif->updatestatus(sd,SP_DEX);
	clif->updatestatus(sd,SP_LUK);
	if (sd->spiritball)
		clif->spiritball_single(sd->fd, sd);
	if (sd->charm_type != CHARM_TYPE_NONE && sd->charm_count > 0)
		clif->charm_single(sd->fd, sd);

	if (sd->vd.cloth_color)
		clif->refreshlook(&sd->bl,sd->bl.id,LOOK_CLOTHES_COLOR,sd->vd.cloth_color,SELF);
	if (sd->vd.body_style)
		clif->refreshlook(&sd->bl,sd->bl.id,LOOK_BODY2,sd->vd.body_style,SELF);
	if(homun_alive(sd->hd))
		clif->send_homdata(sd,SP_ACK,0);
	if( sd->md ) {
		clif->mercenary_info(sd);
		clif->mercenary_skillblock(sd);
	}
	if( sd->ed )
		clif->elemental_info(sd);
	map->foreachinrange(clif->getareachar,&sd->bl,AREA_SIZE,BL_ALL,sd);
	clif->weather_check(sd);
	if (sd->chat_id != 0)
		chat->leave(sd, false);
	if( sd->state.vending )
		clif->openvending(sd, sd->bl.id, sd->vending);
	if( pc_issit(sd) )
		clif->sitting(&sd->bl); // FIXME: just send to self, not area
	if( pc_isdead(sd) ) // When you refresh, resend the death packet.
		clif->clearunit_single(sd->bl.id,CLR_DEAD,sd->fd);
	else
		clif->changed_dir(&sd->bl, SELF);

	// unlike vending, resuming buyingstore crashes the client.
	buyingstore->close(sd);

	mail->clear(sd);

	clif->loadConfirm(sd);

	if (clif->isdisguised(&sd->bl)) {/* refresh-da */
		short disguise = sd->disguise;
		pc->disguise(sd, -1);
		pc->disguise(sd, disguise);
	}

	clif->refresh_storagewindow(sd);
}

static void clif_send_selforarea(int fd, struct block_list *bl, const void *buf, int len)
{
	// if no recipient specified just update nearby clients
	if (fd == 0) {
		clif->send(buf, len, bl, AREA);
	} else {
		struct map_session_data *sd = sockt->session_is_valid(fd) ? sockt->session[fd]->session_data : NULL;
		if (sd != NULL) {
			clif->send(buf, len, &sd->bl, SELF);
		} else {
			clif->send(buf, len, bl, SELF);
		}
	}
}

/**
 * Updates a character's name on client.
 *
 * @code
 *	0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
 *	0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
 *	0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param bl The related character's block list.
 *
 **/
static void clif_pcname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_PC);

	struct PACKET_ZC_ACK_REQNAMEALL packet = {0};
	packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
	packet.gid = bl->id;

	const struct map_session_data *ssd = BL_UCCAST(BL_PC, bl);

	if (ssd->fd == fd && ssd->disguise != -1) // Requesting your own "shadow" name.
		packet.gid = -bl->id;

	if (ssd->fakename[0] != '\0')
		memcpy(packet.name, ssd->fakename, NAME_LENGTH);
	else
		memcpy(packet.name, ssd->status.name, NAME_LENGTH);

	const struct party_data *p = NULL;

	if (ssd->status.party_id != 0)
		p = party->search(ssd->status.party_id);

	const struct guild *g = NULL;
	int pos_idx = INDEX_NOT_FOUND;

	if (ssd->status.guild_id != 0 && (g = ssd->guild) != NULL) {
		int i;
		int acc_id = ssd->status.account_id;
		int chr_id = ssd->status.char_id;

		ARR_FIND(0, g->max_member, i, g->member[i].account_id == acc_id && g->member[i].char_id == chr_id);

		if (i < g->max_member)
			pos_idx = g->member[i].position;
	}

	if (battle_config.display_party_name == 0 && g == NULL)
		p = NULL; // Do not display party name, unless the character is also in a guild.

	if (p != NULL) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_PARTYNAME) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.party_name, p->party.name, NAME_LENGTH);
		}
	}

	if (g != NULL && pos_idx >= 0 && pos_idx < MAX_GUILDPOSITION) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_GUILDNAME) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.guild_name, g->name,NAME_LENGTH);
		}

		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_GUILDPOSITION) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.position_name, g->position[pos_idx].name, NAME_LENGTH);
		}
	} else if (ssd->status.clan_id != 0) {
		struct clan *c = clan->search(ssd->status.clan_id);

		if (c != 0) {
			if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_CLANPOSITION) != 0)
			    || ssd->fakename[0] == '\0') {
				memcpy(packet.position_name, c->name, NAME_LENGTH);
			}
		}
	}

#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO) // Title system.
	if (ssd->status.title_id > 0) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_TITLE) != 0)
		    || ssd->fakename[0] == '\0') {
			packet.title_id = ssd->status.title_id;
		}
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAMEALL));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_homname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_HOM);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	memcpy(packet.name, BL_UCCAST(BL_HOM, bl)->homunculus.name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_mername_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MER);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	memcpy(packet.name, BL_UCCAST(BL_MER, bl)->db->name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_petname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_PET);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	memcpy(packet.name, BL_UCCAST(BL_PET, bl)->pet.name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_npcname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_NPC);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	memcpy(packet.name, BL_UCCAST(BL_NPC, bl)->name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_mobname_guardian_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MOB);
	const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
	Assert_retv(md->guardian_data && md->guardian_data->g);

	struct PACKET_ZC_ACK_REQNAMEALL packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
	packet.gid = bl->id;
	memcpy(packet.name, md->name, NAME_LENGTH);
	memcpy(packet.guild_name, md->guardian_data->g->name, NAME_LENGTH);
	memcpy(packet.position_name, md->guardian_data->castle->castle_name, NAME_LENGTH);
	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAMEALL));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_mobname_normal_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MOB);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
	memcpy(packet.name, md->name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_mobname_additional_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MOB);

	struct PACKET_ZC_ACK_REQNAMEALL packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
	packet.gid = bl->id;

	const struct mob_data *md = BL_UCCAST(BL_MOB, bl);

	memcpy(packet.name, md->name, NAME_LENGTH);
	char mobhp[100];
	char *str_p = mobhp;
	if (battle_config.show_mob_info&4)
		str_p += sprintf(str_p, "Lv. %d | ", md->level);
	if (battle_config.show_mob_info&1)
		str_p += sprintf(str_p, "HP: %u/%u | ", md->status.hp, md->status.max_hp);
	if (battle_config.show_mob_info&2)
		str_p += sprintf(str_p, "HP: %u%% | ", get_percentage(md->status.hp, md->status.max_hp));
	//Even thought mobhp ain't a name, we send it as one so the client
	//can parse it. [Skotlex]
	if (str_p != mobhp) {
		*(str_p-3) = '\0'; //Remove trailing space + pipe.
		memcpy(packet.party_name, mobhp, NAME_LENGTH);
	}

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAMEALL));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_mobname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_MOB);

	const struct mob_data *md = BL_UCCAST(BL_MOB, bl);

	if (md->guardian_data && md->guardian_data->g) {
		clif->mobname_guardian_ack(fd, bl);
	} else if (battle_config.show_mob_info) {
		clif->mobname_additional_ack(fd, bl);
	} else {
		clif->mobname_normal_ack(fd, bl);
	}
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_chatname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_CHAT);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;

#if 0 // Clients DO request this... what should be done about it? The chat's title may not fit... [Skotlex]
	memcpy(packet.name, BL_UCCAST(BL_CHAT, bl)->title, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

/// Updates the object's (bl) name on client.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
static void clif_elemname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	Assert_retv(bl->type == BL_ELEM);

	struct PACKET_ZC_ACK_REQNAME_TITLE packet = { 0 };
	packet.packet_id = HEADER_ZC_ACK_REQNAME_TITLE;
	packet.gid = bl->id;
	memcpy(packet.name, BL_UCCAST(BL_ELEM, bl)->db->name, NAME_LENGTH);
#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
	struct unit_data *ud = unit->bl2ud(bl);
	if (ud != NULL) {
		memcpy(packet.title, ud->title, NAME_LENGTH);
		packet.groupId = ud->groupId;
	}
#endif

	clif->send_selforarea(fd, bl, &packet, sizeof(struct PACKET_ZC_ACK_REQNAME_TITLE));
}

static void clif_skillname_ack(int fd, struct block_list *bl)
{
}

static void clif_itemname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	ShowError("clif_itemname_ack: bad type %u(%d)\n", bl->type, bl->id);
	Assert_retv(0);
}

static void clif_unknownname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);
	ShowError("clif_blname_ack: bad type %u(%d)\n", bl->type, bl->id);
	Assert_retv(0);
}

static void clif_blname_ack(int fd, struct block_list *bl)
{
	nullpo_retv(bl);

	switch(bl->type) {
		case BL_PC:
			clif->pcname_ack(fd, bl);
			break;
		case BL_HOM:
			clif->homname_ack(fd, bl);
			break;
		case BL_MER:
			clif->mername_ack(fd, bl);
			break;
		case BL_PET:
			clif->petname_ack(fd, bl);
			break;
		case BL_NPC:
			clif->npcname_ack(fd, bl);
			break;
		case BL_MOB:
			clif->mobname_ack(fd, bl);
			break;
		case BL_CHAT:
			clif->chatname_ack(fd, bl);
			break;
		case BL_ELEM:
			clif->elemname_ack(fd, bl);
			break;
		case BL_ITEM:
			clif->itemname_ack(fd, bl);
			break;
		case BL_SKILL:
			clif->skillname_ack(fd, bl);
			break;
		default:
			clif->unknownname_ack(fd, bl);
			break;
	}
}

/**
 * Updates a character's name on client when leaving a party/guild.
 *
 * @code
 *	0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
 *	0A30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title id>.L (ZC_ACK_REQNAMEALL2)
 * @endcode
 *
 * @param ssd The related character.
 *
 **/
static void clif_charnameupdate(struct map_session_data *ssd)
{
	nullpo_retv(ssd);

	struct PACKET_ZC_ACK_REQNAMEALL packet = {0};
	packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
	packet.gid = ssd->bl.id;

	if (ssd->fakename[0] != '\0')
		memcpy(packet.name, ssd->fakename, NAME_LENGTH);
	else
		memcpy(packet.name, ssd->status.name, NAME_LENGTH);

	struct party_data *p = NULL;

	if (ssd->status.party_id != 0)
		p = party->search(ssd->status.party_id);

	struct guild *g = NULL;
	int pos_idx = INDEX_NOT_FOUND;

	if (ssd->status.guild_id != 0 && (g = ssd->guild) != NULL) {
		int i;
		int acc_id = ssd->status.account_id;
		int chr_id = ssd->status.char_id;

		ARR_FIND(0, g->max_member, i, g->member[i].account_id == acc_id && g->member[i].char_id == chr_id);

		if (i < g->max_member)
			pos_idx = g->member[i].position;
	}

	if (battle_config.display_party_name == 0 && g == NULL)
		p = NULL; // Do not display party name, unless the character is also in a guild.

	if (p != NULL) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_PARTYNAME) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.party_name, p->party.name, NAME_LENGTH);
		}
	}

	if (g != NULL && pos_idx >= 0 && pos_idx < MAX_GUILDPOSITION) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_GUILDNAME) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.guild_name, g->name,NAME_LENGTH);
		}

		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_GUILDPOSITION) != 0)
		    || ssd->fakename[0] == '\0') {
			memcpy(packet.position_name, g->position[pos_idx].name, NAME_LENGTH);
		}
	} else if (ssd->status.clan_id != 0) {
		struct clan *c = clan->search(ssd->status.clan_id);

		if (c != 0) {
			if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_CLANPOSITION) != 0)
			    || ssd->fakename[0] == '\0') {
				memcpy(packet.position_name, c->name, NAME_LENGTH);
			}
		}
	}

#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO) // Title system.
	if (ssd->status.title_id > 0) {
		if ((ssd->fakename[0] != '\0' && (ssd->fakename_options & FAKENAME_OPTION_SHOW_TITLE) != 0)
		    || ssd->fakename[0] == '\0') {
			packet.title_id = ssd->status.title_id;
		}
	}
#endif

	clif->send(&packet, sizeof(packet), &ssd->bl, AREA); // Update nearby clients.
}

/// Taekwon Jump (TK_HIGHJUMP) effect (ZC_HIGHJUMP).
/// 01ff <id>.L <x>.W <y>.W
///
/// Visually moves(instant) a character to x,y. The char moves even
/// when the target cell isn't walkable. If the char is sitting it
/// stays that way.
static void clif_slide(struct block_list *bl, int x, int y)
{
	unsigned char buf[10];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x01ff;
	WBUFL(buf, 2) = bl->id;
	WBUFW(buf, 6) = x;
	WBUFW(buf, 8) = y;
	clif->send(buf, packet_len(0x1ff), bl, AREA);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2) = -bl->id;
		clif->send(buf, packet_len(0x1ff), bl, SELF);
	}
}

/// Public chat message (ZC_NOTIFY_CHAT). lordalfa/Skotlex - used by @me as well
/// 008d <packet len>.W <id>.L <message>.?B
static void clif_disp_overhead(struct block_list *bl, const char *mes, enum send_target target, struct block_list *target_bl)
{
	char buf[CHAT_SIZE_MAX + (int)sizeof(struct PACKET_ZC_NOTIFY_CHAT)];
	int max_len = CHAT_SIZE_MAX - (int)sizeof(struct PACKET_ZC_NOTIFY_CHAT);
	struct PACKET_ZC_NOTIFY_CHAT *p = (struct PACKET_ZC_NOTIFY_CHAT *)&buf;
	int mes_len;

	nullpo_retv(bl);
	nullpo_retv(mes);

	mes_len = (int)strlen(mes) + 1; //Account for \0
	if (mes_len > max_len) {
		ShowError("clif_disp_overhead: Truncated message '%s' (len=%d, max=%d).\n", mes, mes_len, max_len);
		mes_len = max_len; //Trunk it to avoid problems.
	}

	// send message to others
	p->PacketType = 0x8d;
	p->PacketLength = mes_len + (int)sizeof(struct PACKET_ZC_NOTIFY_CHAT); // len of message + 8 (command+len+id)
	p->GID = bl->id;
	safestrncpy(p->Message, mes, mes_len);
	if (target == SELF && target_bl != NULL) {
		clif->send(p, p->PacketLength, target_bl, SELF);
	} else {
		clif->send(p, p->PacketLength, bl, AREA_CHAT_WOC);

		// send back message to the speaker
		if (bl->type == BL_PC)
			clif->notify_playerchat(bl, mes);
	}
}

static void clif_notify_playerchat(struct block_list *bl, const char *mes)
{
	char buf[CHAT_SIZE_MAX + (int)sizeof(struct PACKET_ZC_NOTIFY_PLAYERCHAT)];
	int max_len = CHAT_SIZE_MAX - (int)sizeof(struct PACKET_ZC_NOTIFY_PLAYERCHAT);
	struct PACKET_ZC_NOTIFY_PLAYERCHAT *p = (struct PACKET_ZC_NOTIFY_PLAYERCHAT *)&buf;
	int mes_len;

	nullpo_retv(bl);
	nullpo_retv(mes);

	mes_len = (int)strlen(mes) + 1; // Account for \0
	if (mes_len > max_len) {
		ShowError("clif_notify_playerchat: Truncated message '%s' (len=%d, max=%d).\n", mes, mes_len, max_len);
		mes_len = max_len; // Truncate to avoid problems.
	}

	p->PacketType = 0x8e;
	p->PacketLength = mes_len + (int)sizeof(struct PACKET_ZC_NOTIFY_PLAYERCHAT);
	safestrncpy(p->Message, mes, mes_len);
	clif->send(p, p->PacketLength, bl, SELF);
}

/*==========================
 * Minimap fix [Kevin]
 * Remove dot from minimap
 *--------------------------*/
static void clif_party_xy_remove(struct map_session_data *sd)
{
	unsigned char buf[16];
	nullpo_retv(sd);
	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=-1;
	WBUFW(buf,8)=-1;
	clif->send(buf,packet_len(0x107),&sd->bl,PARTY_SAMEMAP_WOS);
}

/// Displays a skill message (thanks to Rayce) (ZC_SKILLMSG).
/// 0215 <msg id>.L
/// msg id:
///     0x15 = End all negative status (PA_GOSPEL)
///     0x16 = Immunity to all status (PA_GOSPEL)
///     0x17 = MaxHP +100% (PA_GOSPEL)
///     0x18 = MaxSP +100% (PA_GOSPEL)
///     0x19 = All stats +20 (PA_GOSPEL)
///     0x1c = Enchant weapon with Holy element (PA_GOSPEL)
///     0x1d = Enchant armor with Holy element (PA_GOSPEL)
///     0x1e = DEF +25% (PA_GOSPEL)
///     0x1f = ATK +100% (PA_GOSPEL)
///     0x20 = HIT/Flee +50 (PA_GOSPEL)
///     0x28 = Full strip failed because of coating (ST_FULLSTRIP)
///     ? = nothing
static void clif_gospel_info(struct map_session_data *sd, int type)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x215));
	WFIFOW(fd,0)=0x215;
	WFIFOL(fd,2)=type;
	WFIFOSET(fd, packet_len(0x215));

}

/// Multi-purpose mission information packet (ZC_STARSKILL).
/// 020e <mapname>.24B <monster_id>.L <star>.B <result>.B
/// result:
///      0 = Star Gladiator %s has designed <mapname>'s as the %s.
///      star:
///          0 = Place of the Sun
///          1 = Place of the Moon
///          2 = Place of the Stars
///      1 = Star Gladiator %s's %s: <mapname>
///      star:
///          0 = Place of the Sun
///          1 = Place of the Moon
///          2 = Place of the Stars
///      10 = Star Gladiator %s has designed <mapname>'s as the %s.
///      star:
///          0 = Target of the Sun
///          1 = Target of the Moon
///          2 = Target of the Stars
///      11 = Star Gladiator %s's %s: <mapname used as monster name>
///      star:
///          0 = Monster of the Sun
///          1 = Monster of the Moon
///          2 = Monster of the Stars
///      20 = [TaeKwon Mission] Target Monster : <mapname used as monster name> (<star>%)
///      21 = [Taming Mission] Target Monster : <mapname used as monster name>
///      22 = [Collector Rank] Target Item : <monster_id used as item id>
///      30 = [Sun, Moon and Stars Angel] Designed places and monsters have been reset.
///      40 = Target HP : <monster_id used as HP>
static void clif_starskill(struct map_session_data *sd, const char *mapname, int monster_id, unsigned char star, unsigned char result)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(mapname);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x20e));
	WFIFOW(fd,0) = 0x20e;
	safestrncpy(WFIFOP(fd,2), mapname, NAME_LENGTH);
	WFIFOL(fd,26) = monster_id;
	WFIFOB(fd,30) = star;
	WFIFOB(fd,31) = result;
	WFIFOSET(fd,packet_len(0x20e));
}

/*==========================================
 * Info about Star Gladiator save map [Komurka]
 * type: 1: Information, 0: Map registered
 *------------------------------------------*/
static void clif_feel_info(struct map_session_data *sd, unsigned char feel_level, unsigned char type)
{
	char mapname[MAP_NAME_LENGTH_EXT];

	nullpo_retv(sd);
	Assert_retv(feel_level < MAX_PC_FEELHATE);
	mapindex->getmapname_ext(mapindex_id2name(sd->feel_map[feel_level].index), mapname);
	clif->starskill(sd, mapname, 0, feel_level, type ? 1 : 0);
}

/*==========================================
 * Info about Star Gladiator hate mob [Komurka]
 * type: 1: Register mob, 0: Information.
 *------------------------------------------*/
static void clif_hate_info(struct map_session_data *sd, unsigned char hate_level, int class_, unsigned char type)
{
	if( pc->db_checkid(class_) ) {
		clif->starskill(sd, pc->job_name(class_), class_, hate_level, type ? 10 : 11);
	} else if( mob->db_checkid(class_) ) {
		clif->starskill(sd, mob->db(class_)->jname, class_, hate_level, type ? 10 : 11);
	} else {
		ShowWarning("clif_hate_info: Received invalid class %d for this packet (char_id=%d, hate_level=%u, type=%u).\n", class_, sd->status.char_id, (unsigned int)hate_level, (unsigned int)type);
	}
}

/*==========================================
 * Info about TaeKwon Do TK_MISSION mob [Skotlex]
 *------------------------------------------*/
static void clif_mission_info(struct map_session_data *sd, int mob_id, unsigned char progress)
{
	clif->starskill(sd, mob->db(mob_id)->jname, mob_id, progress, 20);
}

/*==========================================
 * Feel/Hate reset (thanks to Rayce) [Skotlex]
 *------------------------------------------*/
static void clif_feel_hate_reset(struct map_session_data *sd)
{
	clif->starskill(sd, "", 0, 0, 30);
}

/// Send configurations (ZC_CONFIG).
/// 02d9 <type>.L <value>.L
/// type:
///     0 = open equip window
///     3 = homunculus autofeeding
///     value:
///         0 = disabled
///         1 = enabled
static void clif_zc_config(struct map_session_data *sd, enum CZ_CONFIG type, int flag)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2d9));
	WFIFOW(fd, 0) = 0x2d9;
	WFIFOL(fd, 2) = type;
	WFIFOL(fd, 6) = flag;
	WFIFOSET(fd, packet_len(0x2d9));
}

/// The player's 'view equip' state, sent during login (ZC_CONFIG_NOTIFY).
/// 02da <open equip window>.B
/// open equip window:
///     0 = disabled
///     1 = enabled
static void clif_equpcheckbox(struct map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2da));
	WFIFOW(fd, 0) = 0x2da;
	WFIFOB(fd, 2) = (sd->status.show_equip ? 1 : 0);
	WFIFOSET(fd, packet_len(0x2da));
}

/// Sends info about a player's equipped items.
/// 02d7 <packet len>.W <name>.24B <class>.W <hairstyle>.W <up-viewid>.W <mid-viewid>.W <low-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.26B* (ZC_EQUIPWIN_MICROSCOPE)
/// 02d7 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE, PACKETVER >= 20100629)
/// 0859 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE2, PACKETVER >= 20101124)
/// 0859 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <robe>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE2, PACKETVER >= 20110111)
static void clif_viewequip_ack(struct map_session_data *sd, struct map_session_data *tsd)
{
	int i, equip = 0;

	nullpo_retv(sd);
	nullpo_retv(tsd);

	for (i = 0; i < EQI_MAX; i++) {
		int k = tsd->equip_index[i];
		if (k >= 0) {
			if (tsd->status.inventory[k].nameid <= 0 || tsd->inventory_data[k] == NULL) // Item doesn't exist
				continue;

			clif->item_equip(k+2,&viewequip_list.list[equip++],&tsd->status.inventory[k],tsd->inventory_data[k],pc->equippoint(tsd,k));
		}
	}

	viewequip_list.PacketType = viewequipackType;
	viewequip_list.PacketLength = ( sizeof( viewequip_list ) - sizeof( viewequip_list.list ) ) + ( sizeof(struct EQUIPITEM_INFO) * equip );

	safestrncpy(viewequip_list.characterName, tsd->status.name, NAME_LENGTH);

	viewequip_list.job         = tsd->status.class;
	viewequip_list.head        = tsd->vd.hair_style;
	viewequip_list.accessory   = tsd->vd.head_bottom;
	viewequip_list.accessory2  = tsd->vd.head_mid;
	viewequip_list.accessory3  = tsd->vd.head_top;
#if PACKETVER >= 20110111
	viewequip_list.robe        = tsd->vd.robe;
#endif
	viewequip_list.headpalette = tsd->vd.hair_color;
	viewequip_list.bodypalette = tsd->vd.cloth_color;
#if PACKETVER_MAIN_NUM >= 20180801 || PACKETVER_RE_NUM >= 20180801 || PACKETVER_ZERO_NUM >= 20180808
	viewequip_list.body2       = tsd->vd.body_style;
#endif
	viewequip_list.sex         = tsd->vd.sex;

	clif->send(&viewequip_list, viewequip_list.PacketLength, &sd->bl, SELF);
}

/**
 * Displays a string from msgstringtable.txt (ZC_MSG).
 *
 * 0291 <msg id>.W
 *
 * @param sd     The target character.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 */
static void clif_msgtable(struct map_session_data *sd, enum clif_messages msg_id)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x291));
	WFIFOW(fd, 0) = 0x291;
	WFIFOW(fd, 2) = msg_id;  // zero-based msgstringtable.txt index
	WFIFOSET(fd, packet_len(0x291));
}

/**
 * Displays a format string from msgstringtable.txt with a %d value (ZC_MSG_VALUE).
 *
 * 0x7e2 <msg id>.W <value>.L
 *
 * @param sd     The target character.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 * @param value  The value to fill %d.
 */
static void clif_msgtable_num(struct map_session_data *sd, enum clif_messages msg_id, int value)
{
#if PACKETVER >= 20090805
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x7e2));
	WFIFOW(fd, 0) = 0x7e2;
	WFIFOW(fd, 2) = msg_id;
	WFIFOL(fd, 4) = value;
	WFIFOSET(fd, packet_len(0x7e2));
#endif
}

/**
 * Displays a string from msgstringtable.txt, prefixed with a skill name (ZC_MSG_SKILL).
 *
 * 07e6 <skill id>.W <msg id>.L
 *
 * NOTE: Message has following format and is printed in color 0xCDCDFF (purple):
 * "[SkillName] Message"
 *
 * @param sd       The target character.
 * @param skill_id ID of the skill to display.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 */
static void clif_msgtable_skill(struct map_session_data *sd, uint16 skill_id, enum clif_messages msg_id)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x7e6));
	WFIFOW(fd,0) = 0x7e6;
	WFIFOW(fd,2) = skill_id;
	WFIFOL(fd,4) = msg_id;
	WFIFOSET(fd, packet_len(0x7e6));
}

/**
 * Displays a format string from msgstringtable.txt with a %s value (ZC_FORMATSTRING_MSG).
 *
 * @param sd     The target character.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 * @param value  The value to fill %s.
 */
static void clif_msgtable_str(struct map_session_data *sd, enum clif_messages msg_id, const char *value)
{
	int message_len;
	int len;
	struct PACKET_ZC_FORMATSTRING_MSG *p;

	nullpo_retv(sd);
	nullpo_retv(value);

	message_len = (int)strlen(value) + 1;
	len = sizeof(*p) + message_len + 1;

	p = (struct PACKET_ZC_FORMATSTRING_MSG *)aMalloc(len);
	p->PacketType = 0x2c2;
	p->PacketLength = len;
	p->MessageId = msg_id;
	safestrncpy(p->MessageString, value, message_len);
	p->MessageString[message_len] = 0;

	clif->send(p, p->PacketLength, &sd->bl, SELF);
	aFree(p);
}

/**
 * Displays a format string from msgstringtable.txt with a %s value and color (ZC_FORMATSTRING_MSG).
 *
 * @param sd     The target character.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 * @param value  The value to fill %s.
 * @param color  The color to use
 */
static void clif_msgtable_str_color(struct map_session_data *sd, enum clif_messages msg_id, const char *value, uint32 color)
{
#if PACKETVER >= 20160330
	nullpo_retv(sd);
	nullpo_retv(value);

	int message_len = (int)strlen(value) + 1;
	const int len = sizeof(struct PACKET_ZC_FORMATSTRING_MSG_COLOR) + message_len + 1;
	struct PACKET_ZC_FORMATSTRING_MSG_COLOR *p = (struct PACKET_ZC_FORMATSTRING_MSG_COLOR *)aMalloc(len);

	p->PacketType = 0xa6f;
	p->PacketLength = len;
	p->messageId = msg_id;
#if PACKETVER >= 20160406
	p->color = color;
#endif
	safestrncpy(p->messageString, value, message_len);
	p->messageString[message_len] = 0;

	clif->send(p, p->PacketLength, &sd->bl, SELF);
	aFree(p);
#endif
}

/**
 * Displays a format string from msgstringtable.txt with a color (ZC_MSG_COLOR).
 *
 * @param sd     The target character.
 * @param msg_id msgstringtable message index, 0-based (@see enum clif_messages)
 * @param color  The color to use
 */
static void clif_msgtable_color(struct map_session_data *sd, enum clif_messages msg_id, uint32 color)
{
	struct PACKET_ZC_MSG_COLOR p;

	nullpo_retv(sd);

	p.PacketType = 0x9cd;
	p.MessageId = msg_id;
	p.MessageColor = RGB2BGR(color);

	clif->send(&p, sizeof(p), &sd->bl, SELF);
}

/**
 * Validates and processes a global/guild/party message packet.
 *
 * @param[in]  sd         The source character.
 * @param[in]  packet     The packet data.
 * @param[out] out_buf    The output buffer (must be a valid buffer), that will
 *                        be filled with "Name : Message".
 * @param[in]  out_buflen The size of out_buf (including the NUL terminator).
 * @return a pointer to the "Message" part of out_buf.
 * @retval NULL if the validation failed, the messages was a command or the
 *              character can't send chat messages. out_buf shan't be used.
 */
static const char *clif_process_chat_message(struct map_session_data *sd, const struct packet_chat_message *packet, char *out_buf, int out_buflen)
{
	const char *srcname = NULL, *srcmessage = NULL, *message = NULL;
	int textlen = 0, namelen = 0, messagelen = 0;

	nullpo_ret(sd);
	nullpo_ret(packet);
	nullpo_ret(out_buf);

	if (packet->packet_len < 4 + 1) {
		// 4-byte header and at least an empty string is expected
		ShowWarning("clif_process_chat_message: Received malformed packet from player '%s' (no message data)!\n", sd->status.name);
		return NULL;
	}

#if PACKETVER >= 20151001
	// Packet doesn't include a NUL terminator
	textlen = packet->packet_len - 4;
#else // PACKETVER < 20151001
	// Packet includes a NUL terminator
	textlen = packet->packet_len - 4 - 1;
#endif // PACKETVER > 20151001

	// name and message are separated by ' : '
	srcname = packet->message;
	namelen = (int)strnlen(sd->status.name, NAME_LENGTH-1); // name length (w/o zero byte)

	if (strncmp(srcname, sd->status.name, namelen) != 0 // the text must start with the speaker's name
	 || srcname[namelen] != ' ' || srcname[namelen+1] != ':' || srcname[namelen+2] != ' ' // followed by ' : '
	 ) {
		//Hacked message, or infamous "client desynchronize" issue where they pick one char while loading another.
		ShowWarning("clif_process_chat_message: Player '%s' sent a message using an incorrect name! Forcing a relog...\n", sd->status.name);
		sockt->eof(sd->fd); // Just kick them out to correct it.
		return NULL;
	}

	srcmessage = packet->message + namelen + 3; // <name> " : " <message>
	messagelen = textlen - namelen - 3;

	if (messagelen >= CHAT_SIZE_MAX || textlen >= out_buflen) {
		// messages mustn't be too long
		// Normally you can only enter CHATBOX_SIZE-1 letters into the chat box, but Frost Joke / Dazzler's text can be longer.
		// Also, the physical size of strings that use multibyte encoding can go multiple times over the chatbox capacity.
		// Neither the official client nor server place any restriction on the length of the data in the packet,
		// but we'll only allow reasonably long strings here. This also makes sure that they fit into the `chatlog` table.
		ShowWarning("clif_process_chat_message: Player '%s' sent a message too long ('%.*s')!\n", sd->status.name, CHATBOX_SIZE-1, srcmessage);
		return NULL;
	}

	safestrncpy(out_buf, packet->message, textlen+1); // [!] packet->message is not necessarily NUL terminated
	message = out_buf + namelen + 3;

	if (!pc->process_chat_message(sd, message))
		return NULL;
	return message;
}

/**
 * Validates and processes a whisper message packet.
 *
 * @param[in]  sd             The source character.
 * @param[in]  packet         The packet data.
 * @param[out] out_name       The parsed target name buffer (must be a valid
 *                            buffer of size NAME_LENGTH + 1 because the client
 *                            can send 24 characters without NULL terminator).
 * @param[out] out_message    The output message buffer (must be a valid buffer).
 * @param[in]  out_messagelen The size of out_message.
 * @retval true  if the validation succeeded and the message is a chat message.
 * @retval false if the validation failed, the messages was a command or the
 *              character can't send chat messages. out_name and out_message
 *              shan't be used.
 */
static bool clif_process_whisper_message(struct map_session_data *sd, const struct packet_whisper_message *packet, char *out_name, char *out_message, int out_messagelen)
{
	int messagelen = 0;

	nullpo_retr(false, sd);
	nullpo_retr(false, packet);
	nullpo_retr(false, out_name);
	nullpo_retr(false, out_message);

	if (packet->packet_len < NAME_LENGTH + 4 + 1) {
		// 4-byte header and at least an empty string is expected
		ShowWarning("clif_process_whisper_message: Received malformed packet from player '%s' (packet length is incorrect)!\n", sd->status.name);
		return false;
	}

#if PACKETVER >= 20151001
	// Packet doesn't include a NUL terminator
	messagelen = packet->packet_len - NAME_LENGTH - 4;
#else // PACKETVER < 20151001
	// Packet includes a NUL terminator
	messagelen = packet->packet_len - NAME_LENGTH - 4 - 1;
#endif // PACKETVER > 20151001

	if (messagelen >= CHAT_SIZE_MAX || messagelen >= out_messagelen) {
		// messages mustn't be too long
		// Normally you can only enter CHATBOX_SIZE-1 letters into the chat box, but Frost Joke / Dazzler's text can be longer.
		// Also, the physical size of strings that use multibyte encoding can go multiple times over the chatbox capacity.
		// Neither the official client nor server place any restriction on the length of the data in the packet,
		// but we'll only allow reasonably long strings here. This also makes sure that they fit into the `chatlog` table.
		ShowWarning("clif_process_whisper_message: Player '%s' sent a message too long ('%.*s')!\n", sd->status.name, CHAT_SIZE_MAX-1, packet->message);
		return false;
	}

	safestrncpy(out_name, packet->name, NAME_LENGTH + 1); // [!] packet->name is not NUL terminated
	safestrncpy(out_message, packet->message, messagelen+1); // [!] packet->message is not necessarily NUL terminated

	if (!pc->process_chat_message(sd, out_message))
		return false;

	return true;
}

static void clif_channel_msg(struct channel_data *chan, struct map_session_data *sd, char *msg)
{
	struct DBIterator *iter;
	struct map_session_data *user;
	int msg_len;
	uint32 color;

	nullpo_retv(chan);
	nullpo_retv(sd);
	nullpo_retv(msg);
	iter = db_iterator(chan->users);
	msg_len = (int)strlen(msg) + 1;
	Assert_retv(msg_len <= INT16_MAX - 12);
	color = channel->config->colors[chan->color];

	WFIFOHEAD(sd->fd,msg_len + 12);
	WFIFOW(sd->fd,0) = 0x2C1;
	WFIFOW(sd->fd,2) = msg_len + 12;
	WFIFOL(sd->fd,4) = 0;
	WFIFOL(sd->fd,8) = RGB2BGR(color);
	safestrncpy(WFIFOP(sd->fd,12), msg, msg_len);

	for (user = dbi_first(iter); dbi_exists(iter); user = dbi_next(iter)) {
		if( user->fd == sd->fd )
			continue;
		WFIFOHEAD(user->fd,msg_len + 12);
		memcpy(WFIFOP(user->fd,0), WFIFOP(sd->fd,0), msg_len + 12);
		WFIFOSET(user->fd, msg_len + 12);
	}

	WFIFOSET(sd->fd, msg_len + 12);

	dbi_destroy(iter);
}

static void clif_channel_msg2(struct channel_data *chan, char *msg)
{
	struct DBIterator *iter;
	struct map_session_data *user;
	unsigned char buf[210];
	int msg_len;
	uint32 color;

	nullpo_retv(chan);
	nullpo_retv(msg);
	iter = db_iterator(chan->users);
	msg_len = (int)strlen(msg) + 1;
	Assert_retv(msg_len <= INT16_MAX - 12);
	color = channel->config->colors[chan->color];

	WBUFW(buf,0) = 0x2C1;
	WBUFW(buf,2) = msg_len + 12;
	WBUFL(buf,4) = 0;
	WBUFL(buf,8) = RGB2BGR(color);
	safestrncpy(WBUFP(buf,12), msg, msg_len);

	for (user = dbi_first(iter); dbi_exists(iter); user = dbi_next(iter)) {
		WFIFOHEAD(user->fd,msg_len + 12);
		memcpy(WFIFOP(user->fd,0), WBUFP(buf,0), msg_len + 12);
		WFIFOSET(user->fd, msg_len + 12);
	}

	dbi_destroy(iter);
}

// TODO: [4144] same packet with login server. need somehow use one function for both servers
// 3 - Rejected by server
static void clif_auth_error(int fd, int errorCode)
{
	struct packet_ZC_REFUSE_LOGIN p;
	const int len = sizeof(p);

	p.PacketType = authError;
	p.error_code = errorCode;
	p.block_date[0] = '\0';

	WFIFOHEAD(fd, len);
	memcpy(WFIFOP(fd, 0), &p, len);
	WFIFOSET(fd, len);
}

// ------------
// clif_parse_*
// ------------
// Parses incoming (player) connection

/// Request to connect to map-server.
/// 0072 <account id>.L <char id>.L <auth code>.L <client time>.L <gender>.B (CZ_ENTER)
/// 0436 <account id>.L <char id>.L <auth code>.L <client time>.L <gender>.B (CZ_ENTER2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_WantToConnection(int fd, struct map_session_data *sd)
{
	struct block_list* bl;
	struct auth_node* node;
	int cmd, account_id, char_id, login_id1, sex;
	unsigned int client_tick; //The client tick is a tick, therefore it needs be unsigned. [Skotlex]

	if (sd) {
		ShowError("clif_parse_WantToConnection : invalid request (character already logged in)\n");
		return;
	}

	// Only valid packet version get here

	cmd = RFIFOW(fd,0);
	account_id  = RFIFOL(fd, packet_db[cmd].pos[0]);
	char_id     = RFIFOL(fd, packet_db[cmd].pos[1]);
	login_id1   = RFIFOL(fd, packet_db[cmd].pos[2]);
	client_tick = RFIFOL(fd, packet_db[cmd].pos[3]);
	sex         = RFIFOB(fd, packet_db[cmd].pos[4]);

	if( core->runflag != MAPSERVER_ST_RUNNING ) { // not allowed
		clif->authfail_fd(fd,1);// server closed
		return;
	}

	//Check for double login.
	bl = map->id2bl(account_id);
	if(bl && bl->type != BL_PC) {
		ShowError("clif_parse_WantToConnection: a non-player object already has id %d, please increase the starting account number\n", account_id);
		clif->auth_error(fd, 3);  // Rejected by server
		sockt->eof(fd);

		return;
	}

	if (bl ||
		((node=chrif->search(account_id)) && //An already existing node is valid only if it is for this login.
			!(node->account_id == account_id && node->char_id == char_id && node->state == ST_LOGIN)))
	{
		clif->authfail_fd(fd, 8); //Still recognizes last connection
		return;
	}

	CREATE(sd, struct map_session_data, 1);
	sd->fd = fd;

	sd->cryptKey = (( ((( clif->cryptKey[0] * clif->cryptKey[1] ) + clif->cryptKey[2]) & 0xFFFFFFFF)
						* clif->cryptKey[1] ) + clif->cryptKey[2]) & 0xFFFFFFFF;
	sd->parse_cmd_func = clif->parse_cmd;

	sockt->session[fd]->session_data = sd;

	pc->setnewpc(sd, account_id, char_id, login_id1, client_tick, sex, fd);

#if PACKETVER < 20070521
	WFIFOHEAD(fd,4);
	WFIFOL(fd,0) = sd->bl.id;
	WFIFOSET(fd,4);
#else
	WFIFOHEAD(fd,packet_len(0x283));
	WFIFOW(fd,0) = 0x283;
	WFIFOL(fd,2) = sd->bl.id;
	WFIFOSET(fd,packet_len(0x283));
#endif

	chrif->authreq(sd,false);
}

/**
 * Displays the common server messages upon login, chaning maps or teleporting to a character.
 *
 * @param sd The character who should receive the messages.
 * @param connect_new Whether the character is logging in.
 * @param change_map Whether the character is changing maps.
 *
 **/
static void clif_load_end_ack_sub_messages(struct map_session_data *sd, bool connect_new, bool change_map)
{
	nullpo_retv(sd);

	/** Display overweight messages. **/
	if (((battle_config.display_overweight_messages & 0x1) != 0 && connect_new)
	    || ((battle_config.display_overweight_messages & 0x2) != 0 && !connect_new && change_map)) {
		// Send the character's weight to the client. (With displaying overweight messages.)
		clif->updatestatus(sd, SP_MAXWEIGHT);
		clif->updatestatus(sd, SP_WEIGHT);
	} else {
		// Send the character's weight to the client. (Without displaying overweight messages.)
		clif->updatestatus(sd, SP_WEIGHT);
		clif->updatestatus(sd, SP_MAXWEIGHT);
	}

	/** Display configuration messages. **/
	if (((battle_config.display_config_messages & 0x1) != 0 && connect_new)
	    || ((battle_config.display_config_messages & 0x2) != 0 && !connect_new && change_map)
	    || (battle_config.display_config_messages & 0x4) != 0) {
#if PACKETVER >= 20070918
		if ((battle_config.display_config_messages & 0x10) != 0)
			clif->partyinvitationstate(sd);

		if ((battle_config.display_config_messages & 0x20) != 0)
			clif->equpcheckbox(sd);
#endif

#if PACKETVER_MAIN_NUM >= 20171025 || PACKETVER_RE_NUM >= 20170920
		if ((battle_config.display_config_messages & 0x40) != 0)
			clif->zc_config(sd, CZ_CONFIG_CALL, sd->status.allow_call);

		if ((battle_config.display_config_messages & 0x80) != 0) {
			if (sd->pd != NULL)
				clif->zc_config(sd, CZ_CONFIG_PET_AUTOFEEDING, sd->pd->pet.autofeed);
			else
				clif->zc_config(sd, CZ_CONFIG_PET_AUTOFEEDING, false);
		}

		if ((battle_config.display_config_messages & 0x100) != 0) {
			if (sd->hd != NULL)
				clif->zc_config(sd, CZ_CONFIG_HOMUNCULUS_AUTOFEEDING, sd->hd->homunculus.autofeed);
			else
				clif->zc_config(sd, CZ_CONFIG_HOMUNCULUS_AUTOFEEDING, false);
		}
#endif
	}

	/** Display party options. **/
	struct party_data *p = NULL;

	if (sd->status.party_id != 0 && (p = party->search(sd->status.party_id)) != NULL) {
		int flag;

		if (p->state.option_auto_changed != 0)
			flag = 0x04;
		else if (connect_new)
			flag = 0x20;
		else if (change_map)
			flag = 0x40;
		else
			flag = 0x80;

		clif->party_option(p, sd, flag);
	}

	/** Display rate modifier messages. **/
	if (((battle_config.display_rate_messages & 0x1) != 0 && connect_new)
	    || ((battle_config.display_rate_messages & 0x2) != 0 && !connect_new && change_map)
	    || (battle_config.display_rate_messages & 0x4) != 0) {
		clif->show_modifiers(sd);
	}

	/** Display guild notice. **/
	if (sd->guild != NULL) {
		if (((battle_config.guild_notice_changemap & 0x1) != 0 && connect_new)
		    || ((battle_config.guild_notice_changemap & 0x2) != 0 && !connect_new && change_map)
		    || (battle_config.guild_notice_changemap & 0x4) != 0) {
			clif->guild_notice(sd, sd->guild);
		}
	}
}

/**
 * Notification from the client, that it has finished map loading and is about to display player's character. (CZ_NOTIFY_ACTORINIT)
 *
 * @code
 *	007d
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 *
 **/
static void clif_parse_LoadEndAck(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_LoadEndAck(int fd, struct map_session_data *sd)
{
	if (sd->state.using_megaphone != 0)
		sd->state.using_megaphone = 0;

	if (sd->bl.prev != NULL)
		return;

	if (sd->state.active == 0) { // Character loading is not complete yet! Let pc->reg_received reinvoke this when ready.
		sd->state.connect_new = 0;
		return;
	}

	if (sd->state.rewarp != 0) { // Rewarp character.
		sd->state.rewarp = 0;
		clif->changemap(sd, sd->bl.m, sd->bl.x, sd->bl.y);
		return;
	}

	sd->state.warping = 0;
	sd->state.dialog = 0; // Reset when warping. Client dialog will go missing.

	// Character looks.
#if PACKETVER < 4
	clif->changelook(&sd->bl, LOOK_WEAPON, sd->status.look.weapon);
	clif->changelook(&sd->bl, LOOK_SHIELD, sd->status.look.shield);
#else
	clif->changelook(&sd->bl, LOOK_WEAPON, 0);
#endif

	if (sd->vd.cloth_color != 0)
		clif->refreshlook(&sd->bl, sd->bl.id, LOOK_CLOTHES_COLOR, sd->vd.cloth_color, SELF);

	if (sd->vd.body_style != 0)
		clif->refreshlook(&sd->bl, sd->bl.id, LOOK_BODY2, sd->vd.body_style, SELF);

	/**
	 * Send character inventory to the client.
	 * Call this before pc->checkitem() so that the client isn't called to delete a non-existent items.
	 *
	 **/
	clif->inventoryList(sd);

	// Send the cart inventory, counts & weight to the client.
	if (pc_iscarton(sd)) {
		clif->cartList(sd);
		clif->updatestatus(sd, SP_CARTINFO);
	}

	/**
	 * In official servers, an item's unequip script is executed when entering a zone where the item is restricted,
	 * even if the item won't be unequipped.
	 *
	 **/
	if (map->list[sd->bl.m].zone != NULL && map->list[sd->bl.m].zone->disabled_items_count != 0) {
		struct map_zone_data *zone = map->list[sd->bl.m].zone;
		int dis_items_cnt = zone->disabled_items_count;
		int handled_equip = 0x00000000;

		for (int i = 0; i < EQI_MAX; i++) {
			if (sd->equip_index[i] == INDEX_NOT_FOUND)
				continue;

			int inv_idx = sd->equip_index[i];
			struct item_data *equip_data = sd->inventory_data[inv_idx];

			if (equip_data == NULL)
				continue;

			if ((handled_equip & equip_data->equip) != 0)
				continue; // Equipment takes multiple slots and was already handled.

			handled_equip |= equip_data->equip;

			if (equip_data->unequip_script != NULL) {
				int idx;

				ARR_FIND(0, dis_items_cnt, idx, zone->disabled_items[idx] == equip_data->nameid);

				if (idx < dis_items_cnt)
					script->run_item_unequip_script(sd, equip_data, npc->fake_nd->bl.id);
			}

			if (inv_idx != sd->equip_index[i])
				continue; // Unequip script execution corrupted the inventory index.

			struct item *equip = &sd->status.inventory[inv_idx];

			if (equip != NULL && !itemdb_isspecial(equip->card[0])) {
				for (int slot = 0; slot < equip_data->slot; slot++) {
					if (equip->card[slot] == 0)
						continue;

					struct item_data *card_data = itemdb->exists(equip->card[slot]);

					if (card_data != NULL && card_data->unequip_script != NULL) {
						int idx;

						ARR_FIND(0, dis_items_cnt, idx, zone->disabled_items[idx] == card_data->nameid);

						if (idx < dis_items_cnt)
							script->run_item_unequip_script(sd, card_data, npc->fake_nd->bl.id);
					}
				}
			}
		}
	}

	// Check for and delete unavailable/disabled items.
	pc->checkitem(sd);

	// Send character's guild info to the client. Call this before clif->spawn() to show guild emblems correctly.
	if (sd->status.guild_id != 0)
		guild->send_memberinfoshort(sd, 1);

	if (battle_config.pc_invincible_time > 0)
		pc->setinvincibletimer(sd, battle_config.pc_invincible_time);

	if (map->list[sd->bl.m].users++ == 0 && battle_config.dynamic_mobs != 0)
		map->spawnmobs(sd->bl.m);

	if (map->list[sd->bl.m].instance_id >= 0) {
		instance->list[map->list[sd->bl.m].instance_id].users++;
		instance->check_idle(map->list[sd->bl.m].instance_id);
	}

	if (pc_has_permission(sd, PC_PERM_VIEW_HPMETER)) {
		map->list[sd->bl.m].hpmeter_visible++;
		sd->state.hpmeter_visible = 1;
	}

	if (!pc_isinvisible(sd)) // Increment the number of pvp players on the map.
		map->list[sd->bl.m].users_pvp++;

	sd->state.debug_remove_map = 0; // Temporary state to track double calls of unit->remove_map(). [FlavioJS]
	sd->state.callshop = 0; // Reset the callshop flag if the character changes map.
	map->addblock(&sd->bl); // Add the character to the map.
	clif->spawn(&sd->bl); // Spawn character client side.

	clif_load_end_ack_sub_messages(sd, (sd->state.connect_new != 0), (sd->state.changemap != 0));

	struct party_data *p = NULL;

	if (sd->status.party_id != 0)
		p = party->search(sd->status.party_id);

	// Send character's party info to the client. Call this after clif->spawn() to show HP bars correctly.
	if (p != NULL) {
		if (sd->state.connect_new == 0) // Login is handled in party_member_joined().
			party->send_movemap(sd);

		clif->party_hp(sd); // Show HP after displacement. [LuzZza]
	}

	if (sd->bg_id != 0)
		clif->bg_hp(sd); // BattleGround system.

	if (map->list[sd->bl.m].flag.pvp != 0 && !pc_isinvisible(sd)) {
		if (battle_config.pk_mode == 0) { // Remove PVP stuff for pk_mode. [Valaris]
			if (map->list[sd->bl.m].flag.pvp_nocalcrank == 0)
				sd->pvp_timer = timer->add(timer->gettick() + 200, pc->calc_pvprank_timer, sd->bl.id, 0);

			sd->pvp_rank = 0;
			sd->pvp_lastusers = 0;
			sd->pvp_point = 5;
			sd->pvp_won = 0;
			sd->pvp_lost = 0;
		}

		clif->map_property(sd, MAPPROPERTY_FREEPVPZONE);
	} else if(sd->duel_group != 0) { // Set flag, if it's a duel. [LuzZza]
		clif->map_property(sd, MAPPROPERTY_FREEPVPZONE);
	}

	if (map->list[sd->bl.m].flag.gvg_dungeon != 0)
		clif->map_property(sd, MAPPROPERTY_FREEPVPZONE); //TODO: Figure out the real packet to send here.

	if (map_flag_gvg2(sd->bl.m))
		clif->map_property(sd, MAPPROPERTY_AGITZONE);

	// Info about nearby objects. Must use map->foreachinarea(). (CIRCULAR_AREA interferes with map->foreachinrange().)
	map->foreachinarea(clif->getareachar, sd->bl.m, sd->bl.x - AREA_SIZE, sd->bl.y - AREA_SIZE,
			   sd->bl.x + AREA_SIZE, sd->bl.y + AREA_SIZE, BL_ALL, sd);

	// Spawn pet.
	if (sd->pd != NULL) {
		if (map->list[sd->bl.m].flag.nopet != 0) { // Return the pet to egg. [Skotlex]
			clif->message(sd->fd, msg_sd(sd, 866)); // Pets are not allowed in Guild Wars.
			pet->menu(sd, 3); // Option 3 is return to egg.
		} else {
			pet->spawn(sd, false);
		}
	}

	// Spawn homunculus. [blackhole89]
	if (homun_alive(sd->hd)) {
		map->addblock(&sd->hd->bl);
		clif->spawn(&sd->hd->bl);
		clif->send_homdata(sd, SP_ACK, 0);
		clif->hominfo(sd,sd->hd, 1);
		clif->hominfo(sd,sd->hd, 0); // For some reason, at least older clients want this sent twice.
		clif->homskillinfoblock(sd);

		if ((battle_config.hom_setting & 0x8) != 0)
			status_calc_bl(&sd->hd->bl, SCB_SPEED); // Homunculi mimic their master's speed on each map change.

		if ((battle_config.hom_setting & 0x2) == 0)
			skill->unit_move(&sd->hd->bl, timer->gettick(), 1); // Apply land skills immediately.
	}

	// Spawn mercenary.
	if (sd->md != NULL) {
		map->addblock(&sd->md->bl);
		clif->spawn(&sd->md->bl);
		clif->mercenary_info(sd);
		clif->mercenary_skillblock(sd);
		status_calc_bl(&sd->md->bl, SCB_SPEED); // Mercenaries mimic their master's speed on each map change.
	}

	// Spawn elemental.
	if (sd->ed != NULL) {
		map->addblock(&sd->ed->bl);
		clif->spawn(&sd->ed->bl);
		clif->elemental_info(sd);
		clif->elemental_updatestatus(sd, SP_HP);
		clif->hpmeter_single(sd->fd, sd->ed->bl.id, sd->ed->battle_status.hp, sd->ed->battle_status.max_hp);
		clif->elemental_updatestatus(sd, SP_SP);
		status_calc_bl(&sd->ed->bl, SCB_SPEED); // Elementals mimic their master's speed on each map change.
	}

	bool first_time = false;

	if (sd->state.connect_new != 0) {
		first_time = true;
		sd->state.connect_new = 0;
		clif->skillinfoblock(sd);
		clif->hotkeysAll(sd);
		clif->updatestatus(sd, SP_BASEEXP);
		clif->updatestatus(sd, SP_NEXTBASEEXP);
		clif->updatestatus(sd, SP_JOBEXP);
		clif->updatestatus(sd, SP_NEXTJOBEXP);
		clif->updatestatus(sd, SP_SKILLPOINT);
		clif->initialstatus(sd);

		// Unequip items which can't be equipped by the character.
		for (int i = 0; i < EQI_MAX; i++) {
			if (sd->equip_index[i] >= 0 && pc->isequip(sd , sd->equip_index[i]) == 0)
				pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_FORCE);
		}

		if (pc_isfalcon(sd)) {
			int sc_icn = status->get_sc_icon(SC_FALCON);
			int sc_typ = status->get_sc_relevant_bl_types(SC_FALCON);
			clif->status_change(&sd->bl, sc_icn, sc_typ, 1, 0, 0, 0, 0);
		}

		if (pc_isridingpeco(sd) || pc_isridingdragon(sd)) {
			int sc_icn = status->get_sc_icon(SC_RIDING);
			int sc_typ = status->get_sc_relevant_bl_types(SC_RIDING);
			clif->status_change(&sd->bl, sc_icn, sc_typ, 1, 0, 0, 0, 0);
		} else if (pc_isridingwug(sd)) {
			int sc_icn = status->get_sc_icon(SC_WUGRIDER);
			int sc_typ = status->get_sc_relevant_bl_types(SC_WUGRIDER);
			clif->status_change(&sd->bl, sc_icn, sc_typ, 1, 0, 0, 0, 0);
		}

		if (sd->status.manner < 0)
			sc_start(NULL, &sd->bl, SC_NOCHAT, 100, 0, 0);

		int lv = pc->checkskill(sd,SG_KNOWLEDGE);

		// Auron reported that this skill only triggers when you logon on the map. [Skotlex]
		if (lv > 0) {
			for (int i = 0; i < MAX_PC_FEELHATE; i++) {
				if (sd->bl.m == sd->feel_map[i].m) {
					sc_start(NULL, &sd->bl, SC_KNOWLEDGE, 100, lv, skill->get_time(SG_KNOWLEDGE, lv));
					break;
				}
			}
		}

		if (sd->pd != NULL && sd->pd->pet.intimate > PET_INTIMACY_LOYAL)
			clif->pet_emotion(sd->pd, (sd->pd->pet.class_ - 100) * 100 + 50 + pet->hungry_val(sd->pd));

		if (homun_alive(sd->hd))
			homun->init_timers(sd->hd);

		if (map->night_flag != 0 && map->list[sd->bl.m].flag.nightenabled != 0) {
			int sc_icn = status->get_sc_icon(SC_SKE);
			int sc_typ = status->get_sc_relevant_bl_types(SC_SKE);

			sd->state.night = 1;
			clif->status_change(&sd->bl, sc_icn, sc_typ, 1, 0, 0, 0, 0);
		}

		// Notify everyone that this character logged in. [Skotlex]
		map->foreachpc(clif->friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 1);

#if PACKETVER >= 20171122
		if (battle_config.show_tip_window != 0)
			clif->open_ui_send(sd, ZC_TIPBOX_UI);
#endif
#if PACKETVER_RE_NUM >= 20180307 || PACKETVER_MAIN_NUM >= 20180404 || PACKETVER_ZERO_NUM >= 20180411
		if (battle_config.feature_enable_attendance_system != 0 && battle_config.show_attendance_window != 0 && clif->attendance_timediff(sd) == true)
			clif->open_ui_send(sd, ZC_ATTENDANCE_UI);
#endif

		// Run OnPCLoginEvent labels.
		npc->script_event(sd, NPCE_LOGIN);
	} else {
		// For some reason the client "loses" these on warp/map-change.
		clif->updatestatus(sd, SP_STR);
		clif->updatestatus(sd, SP_AGI);
		clif->updatestatus(sd, SP_VIT);
		clif->updatestatus(sd, SP_INT);
		clif->updatestatus(sd, SP_DEX);
		clif->updatestatus(sd, SP_LUK);

		if (sd->state.warp_clean != 0) { // Abort currently running script.
			sd->state.using_fake_npc = 0;
			sd->state.menu_or_input = 0;
			sd->npc_menu = 0;

			if (sd->npc_id != 0)
				npc->event_dequeue(sd);
		} else {
			sd->state.warp_clean = 1;
		}
	}

	if (sd->state.changemap != 0) { // Restore information that gets lost on map-change.
		bool flee_penalty = (battle_config.bg_flee_penalty != 100 || battle_config.gvg_flee_penalty != 100);
		bool is_gvg = (map_flag_gvg2(sd->state.pmap) || map_flag_gvg2(sd->bl.m));
		bool is_bg = (map->list[sd->state.pmap].flag.battleground != 0 || map->list[sd->bl.m].flag.battleground != 0);

		if (flee_penalty && (is_gvg || is_bg))
			status_calc_bl(&sd->bl, SCB_FLEE); // Refresh flee penalty.

		if (map->night_flag != 0 && map->list[sd->bl.m].flag.nightenabled != 0) {
			if (sd->state.night == 0) { // Display night.
				int sc_icn = status->get_sc_icon(SC_SKE);
				int sc_typ = status->get_sc_relevant_bl_types(SC_SKE);

				sd->state.night = 1;
				clif->status_change(&sd->bl, sc_icn, sc_typ, 1, 0, 0, 0, 0);
			}
		} else if (sd->state.night != 0) { // Clear night display.
			sd->state.night = 0;
			clif->sc_end(&sd->bl, sd->bl.id, SELF, status->get_sc_icon(SC_SKE));
		}

		if (map->list[sd->bl.m].flag.battleground != 0) {
			clif->map_type(sd, MAPTYPE_BATTLEFIELD); // Battleground mode.

			if (map->list[sd->bl.m].flag.battleground == 2)
				clif->bg_updatescore_single(sd);
		}

		if (map->list[sd->bl.m].flag.allowks != 0 && !map_flag_ks(sd->bl.m)) {
			char output[128];

			sprintf(output, "%s", msg_sd(sd, 893)); // [ Kill Steal Protection Disabled. KS is allowed in this map ]
			clif->broadcast(&sd->bl, output, (int)strlen(output) + 1, BC_BLUE, SELF);
		}

		map->iwall_get(sd); // Updates walls info on this map to client.
		status_calc_pc(sd, SCO_NONE); // Some conditions are map-dependent so we must recalculate.
		sd->state.changemap = false;

		if (channel->config->local && channel->config->local_autojoin)
			channel->map_join(sd);

		if (channel->config->irc && channel->config->irc_autojoin)
			channel->irc_join(sd);
	}

	mail->clear(sd);
	clif->maptypeproperty2(&sd->bl, SELF);

	if (sd->guild != NULL) {
		// Init guild aura.
		if (sd->state.gmaster_flag != 0) {
			guild->aura_refresh(sd, GD_LEADERSHIP, guild->checkskill(sd->guild, GD_LEADERSHIP));
			guild->aura_refresh(sd, GD_GLORYWOUNDS, guild->checkskill(sd->guild, GD_GLORYWOUNDS));
			guild->aura_refresh(sd, GD_SOULCOLD, guild->checkskill(sd->guild, GD_SOULCOLD));
			guild->aura_refresh(sd, GD_HAWKEYES, guild->checkskill(sd->guild, GD_HAWKEYES));
		}
	}

	if (sd->state.vending != 0) { // Character is vending.
		clif->openvending(sd, sd->bl.id, sd->vending);
		clif->showvendingboard(&sd->bl, sd->message, 0);
	}

	if (map->list[sd->bl.m].flag.loadevent != 0) // Run OnPCLoadMapEvent labels. [Lance]
		npc->script_event(sd, NPCE_LOADMAP);

	if (pc->checkskill(sd, SG_DEVIL) > 0 && pc->nextjobexp(sd) == 0) // Blindness. [Komurka]
		clif->sc_end(&sd->bl, sd->bl.id, SELF, status->get_sc_icon(SC_DEVIL1));

	if (sd->sc.opt2 != 0) // Client loses these on warp.
		clif->changeoption(&sd->bl);

	if (sd->sc.data[SC_MONSTER_TRANSFORM] != NULL && battle_config.mon_trans_disable_in_gvg != 0
	    && map_flag_gvg2(sd->bl.m)) {
		status_change_end(&sd->bl, SC_MONSTER_TRANSFORM, INVALID_TIMER);
		clif->message(sd->fd, msg_sd(sd, 1488)); // Transforming into monster is not allowed in Guild Wars.
	}

	clif->weather_check(sd);

	// For automatic triggering of NPCs after map loading. (So you don't need to walk 1 step first.)
	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNPC) != 0)
		npc->touch_areanpc(sd, sd->bl.m, sd->bl.x, sd->bl.y);
	else
		npc->untouch_areanpc(sd, sd->bl.m, sd->bl.x, sd->bl.y);

	// It broke at some point (e.g. during a crash), so we make it visibly dead again.
	if (sd->status.hp == 0 && !pc_isdead(sd) && status->isdead(&sd->bl) != 0)
		pc_setdead(sd);

	// Send death packet, if character is dead and is spawned (such as @refresh). [Valaris]
	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl, CLR_DEAD);
	} else {
		skill->usave_trigger(sd);

		if (battle_config.player_warp_keep_direction == 1)
			clif->changed_dir(&sd->bl, SELF); // Visually updates player facing direction
	}

	// Trigger skill effects if you appear standing on them.
	if (battle_config.pc_invincible_time == 0)
		skill->unit_move(&sd->bl, timer->gettick(), 1);

#if PACKETVER >= 20090218
	quest->questinfo_refresh(sd); // NPC quest/event icon check. [Kisuka]
#endif

	if (first_time) {
		int i;

		ARR_FIND(0, instance->instances, i, instance->list[i].owner_type == IOT_CHAR && instance->list[i].owner_id == sd->status.account_id);

		if (i < instance->instances) {
			sd->instances = 1;
			CREATE(sd->instance, short, 1);
			sd->instance[0] = instance->list[i].id;
			clif->instance_join(sd->fd, instance->list[i].id);
		}
	}
}

/// Server's tick (ZC_NOTIFY_TIME).
/// 007f <time>.L
static void clif_notify_time(struct map_session_data *sd, int64 time)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x7f));
	WFIFOW(fd,0) = 0x7f;
	WFIFOL(fd,2) = (uint32)time;
	WFIFOSET(fd,packet_len(0x7f));
}

static void clif_parse_TickSend(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request for server's tick.
/// 007e <client tick>.L (CZ_REQUEST_TIME)
/// 0360 <client tick>.L (CZ_REQUEST_TIME2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_TickSend(int fd, struct map_session_data *sd)
{
	sd->client_tick = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	clif->notify_time(sd, timer->gettick());
}

static void clif_hotkeysAll_send(struct map_session_data *sd)
{
#ifdef HOTKEY_SAVING
	clif->hotkeys(sd, 0);
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	// send second tab only if data exists
	for (int i = MAX_HOTKEYS; i < MAX_HOTKEYS * 2; i++) {
		if (sd->status.hotkeys[i].type != 0 || sd->status.hotkeys[i].id != 0 || sd->status.hotkeys[i].lv != 0) {
			clif->hotkeys(sd, 1);
			return;
		}
	}
#endif
#endif
}

/// Sends hotkey bar.
/// 02b9 { <is skill>.B <id>.L <count>.W }*27 (ZC_SHORTCUT_KEY_LIST)
/// 07d9 { <is skill>.B <id>.L <count>.W }*36 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090603)
/// 07d9 { <is skill>.B <id>.L <count>.W }*38 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090617)
/// 0a00 <rotate>.B { <is skill>.B <id>.L <count>.W }*38 (ZC_SHORTCUT_KEY_LIST_V3, PACKETVER >= 20141022)
static void clif_hotkeys_send(struct map_session_data *sd, int tab)
{
#ifdef HOTKEY_SAVING
	nullpo_retv(sd);
	struct PACKET_ZC_SHORTCUT_KEY_LIST p;
	p.packetType = HEADER_ZC_SHORTCUT_KEY_LIST;
#if PACKETVER_MAIN_NUM >= 20141022 || PACKETVER_RE_NUM >= 20141015 || defined(PACKETVER_ZERO)
	if (tab == 0)
		p.rotate = sd->status.hotkey_rowshift;
	else
		p.rotate = sd->status.hotkey_rowshift2;
#endif
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	p.tab = tab;
#endif
	const int offset = tab * MAX_HOTKEYS;
	for (int i = 0; i < MAX_HOTKEYS_PACKET; i++) {
		p.hotkey[i].isSkill = sd->status.hotkeys[i + offset].type;
		p.hotkey[i].id = sd->status.hotkeys[i + offset].id;
		p.hotkey[i].count = sd->status.hotkeys[i + offset].lv;
	}
	clif->send(&p, sizeof(struct PACKET_ZC_SHORTCUT_KEY_LIST), &sd->bl, SELF);
#endif
}

static void clif_parse_HotkeyRowShift1(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_HotkeyRowShift1(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20140129 || PACKETVER_RE_NUM >= 20140129 || defined(PACKETVER_ZERO)
	const struct PACKET_CZ_SHORTCUTKEYBAR_ROTATE1 *p = RFIFOP(fd, 0);
	sd->status.hotkey_rowshift = p->rowshift;
#endif
}

static void clif_parse_HotkeyRowShift2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_HotkeyRowShift2(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	const struct PACKET_CZ_SHORTCUTKEYBAR_ROTATE2 *p = RFIFOP(fd, 0);
	if (p->tab == 0)
		sd->status.hotkey_rowshift = p->rowshift;
	else
		sd->status.hotkey_rowshift2 = p->rowshift;
#endif
}

static void clif_parse_Hotkey1(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update a position on the hotkey bar (CZ_SHORTCUT_KEY_CHANGE1).
/// 02ba <index>.W <is skill>.B <id>.L <count>.W
static void clif_parse_Hotkey1(int fd, struct map_session_data *sd)
{
#ifdef HOTKEY_SAVING
#if PACKETVER_MAIN_NUM >= 20070618 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_AD_NUM >= 20070618 || PACKETVER_SAK_NUM >= 20070618
	const struct PACKET_CZ_SHORTCUT_KEY_CHANGE1 *p = RFIFOP(fd, 0);
	const unsigned short idx = p->index;
	Assert_retv(idx < MAX_HOTKEYS);

	sd->status.hotkeys[idx].type = p->hotkey.isSkill;
	sd->status.hotkeys[idx].id = p->hotkey.id;
	sd->status.hotkeys[idx].lv = p->hotkey.count;
#endif
#endif
}

static void clif_parse_Hotkey2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update a position on the hotkey bar (CZ_SHORTCUT_KEY_CHANGE2).
static void clif_parse_Hotkey2(int fd, struct map_session_data *sd)
{
#ifdef HOTKEY_SAVING
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	const struct PACKET_CZ_SHORTCUT_KEY_CHANGE2 *p = RFIFOP(fd, 0);
	const unsigned short idx = p->index + p->tab * MAX_HOTKEYS;
	Assert_retv(idx < MAX_HOTKEYS_DB);

	sd->status.hotkeys[idx].type = p->hotkey.isSkill;
	sd->status.hotkeys[idx].id = p->hotkey.id;
	sd->status.hotkeys[idx].lv = p->hotkey.count;
#endif
#endif
}

/// Displays cast-like progress bar (ZC_PROGRESS).
/// 02f0 <color>.L <time>.L
static void clif_progressbar(struct map_session_data *sd, unsigned int color, unsigned int second)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2f0));
	WFIFOW(fd,0) = 0x2f0;
	WFIFOL(fd,2) = color;
	WFIFOL(fd,6) = second;
	WFIFOSET(fd,packet_len(0x2f0));
}

/// Removes an ongoing progress bar (ZC_PROGRESS_CANCEL).
/// 02f2
static void clif_progressbar_abort(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2f2));
	WFIFOW(fd,0) = 0x2f2;
	WFIFOSET(fd,packet_len(0x2f2));
}

/**
 * Displays cast-like progress bar on a unit.
 * 09d1 <id>.L <color>.L <time>.L
 *
 * @param bl       Source block list.
 * @param color    Message color (RGB format: 0xRRGGBB).
 * @param time   Time in seconds.
 */
static void clif_progressbar_unit(struct block_list *bl, uint32 color, uint32 time)
{
#if PACKETVER >= 20130821
	struct ZC_PROGRESS_ACTOR p;
	nullpo_retv(bl);

	p.PacketType = progressbarunit;
	p.GID = bl->id;
	p.color = color;
	p.time = time;
	clif->send(&p, sizeof(p), bl, AREA);
#else
	ShowWarning("clif_progressbar_unit: Using progressbar with units available for PACKETVER >= 20130821 only.");
#endif
}

static void clif_parse_progressbar(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Notification from the client, that the progress bar has reached 100% (CZ_PROGRESS).
/// 02f1
static void clif_parse_progressbar(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int npc_id = sd->progressbar.npc_id;
	Assert_retv(npc_id != 0);

	if (timer->gettick() < sd->progressbar.timeout && sd->st)
		sd->st->state = END;

	sd->progressbar.timeout = sd->state.workinprogress = sd->progressbar.npc_id = 0;
	npc->scriptcont(sd, npc_id, false);
}

static void clif_parse_WalkToXY(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to walk to a certain position on the current map.
/// 0085 <dest>.3B (CZ_REQUEST_MOVE)
/// 035f <dest>.3B (CZ_REQUEST_MOVE2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_WalkToXY(int fd, struct map_session_data *sd)
{
	short x, y;

	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl, CLR_DEAD);
		return;
	}

	if (sd->sc.opt1 && ( sd->sc.opt1 == OPT1_STONEWAIT || sd->sc.opt1 == OPT1_BURNING ))
		; //You CAN walk on this OPT1 value.
	/*else if( sd->progressbar.npc_id )
		clif->progressbar_abort(sd);*/
	else if (pc_cant_act_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if(sd->sc.data[SC_RUN] || sd->sc.data[SC_WUGDASH])
		return;

	pc->delinvincibletimer(sd);

	RFIFOPOS(fd, packet_db[RFIFOW(fd,0)].pos[0], &x, &y, NULL);

	//Set last idle time... [Skotlex]
	pc->update_idle_time(sd, BCIDLE_WALK);

	unit->walk_toxy(&sd->bl, x, y, 4);
}

/// Notification about the result of a disconnect request (ZC_ACK_REQ_DISCONNECT).
/// 018b <result>.W
/// result:
///     0 = disconnect (quit)
///     1 = cannot disconnect (wait 10 seconds)
///     ? = ignored
static void clif_disconnect_ack(struct map_session_data *sd, short result)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x18b));
	WFIFOW(fd,0) = 0x18b;
	WFIFOW(fd,2) = result;
	WFIFOSET(fd,packet_len(0x18b));
}

static void clif_parse_QuitGame(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to disconnect from server (CZ_REQ_DISCONNECT).
/// 018a <type>.W
/// type:
///     0 = quit
static void clif_parse_QuitGame(int fd, struct map_session_data *sd)
{
	/* Rovert's prevent logout option fixed [Valaris] */
	if (!sd->sc.data[SC_CLOAKING] && !sd->sc.data[SC_HIDING] && !sd->sc.data[SC_CHASEWALK] && !sd->sc.data[SC_CLOAKINGEXCEED] && !sd->sc.data[SC__INVISIBILITY] && !sd->sc.data[SC_SUHIDE] &&
		(!battle_config.prevent_logout || DIFF_TICK(timer->gettick(), sd->canlog_tick) > battle_config.prevent_logout)) {
		clif->disconnect_ack(sd, 0);
		sockt->flush(fd);
		if (battle_config.drop_connection_on_quit)
			sockt->eof(fd);
	} else {
		clif->disconnect_ack(sd, 1);
	}
}

static void clif_parse_GetCharNameRequest(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Requesting unit's name.
/// 0094 <id>.L (CZ_REQNAME)
/// 0368 <id>.L (CZ_REQNAME2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_GetCharNameRequest(int fd, struct map_session_data *sd)
{
	int id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	struct block_list* bl;
	//struct status_change *sc;

	if( id < 0 && -id == sd->bl.id ) // for disguises [Valaris]
		id = sd->bl.id;

	bl = map->id2bl(id);
	if( bl == NULL )
		return; // Lagged clients could request names of already gone mobs/players. [Skotlex]

	if( sd->bl.m != bl->m || !check_distance_bl(&sd->bl, bl, AREA_SIZE) )
		return; // Block namerequests past view range

	// 'see people in GM hide' cheat detection
#if 0 /* disabled due to false positives (network lag + request name of char that's about to hide = race condition) */
	sc = status->get_sc(bl);
	if (sc && sc->option&OPTION_INVISIBLE && !clif->isdisguised(bl) &&
		bl->type != BL_NPC && //Skip hidden NPCs which can be seen using Maya Purple
		!pc_has_permission(sd, PC_PERM_RECEIVE_HACK_INFO)
	) {
		char gm_msg[256];
		sprintf(gm_msg, "Hack on NameRequest: character '%s' (account: %d) requested the name of an invisible target (id: %d).\n", sd->status.name, sd->status.account_id, id);
		ShowWarning(gm_msg);
		// information is sent to all online GMs
		pc->wis_message_to_gm(map->wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, gm_msg);
		return;
	}
#endif // 0

	clif->blname_ack(fd, bl);
}
static int clif_undisguise_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data * sd;
	if( (sd = map->id2sd(id)) ) {
		sd->fontcolor_tid = INVALID_TIMER;
		if (sd->fontcolor && sd->disguise == sd->status.class)
			pc->disguise(sd,-1);
	}
	return 0;
}

/**
 * Validates and processed global messages.
 *
 * There are various variants of this packet.
 *
 * @code
 * 008c <packet len>.W <text>.?B (<name> : <message>) 00 (CZ_REQUEST_CHAT)
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 */
static void clif_parse_GlobalMessage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_GlobalMessage(int fd, struct map_session_data *sd)
{
	const struct packet_chat_message *packet = NULL;
	char full_message[CHAT_SIZE_MAX + NAME_LENGTH + 3 + 1];
	const char *message = NULL;
	bool is_fakename = false;
	int outlen = 0;

	packet = RP2PTR(fd);
	message = clif->process_chat_message(sd, packet, full_message, sizeof full_message);
	if (message == NULL)
		return;

	pc->check_supernovice_call(sd, message);

	if (sd->gcbind != NULL) {
		channel->send(sd->gcbind, sd, message);
		return;
	}

	if (sd->fakename[0] != '\0') {
		is_fakename = true;
		outlen = (int)strlen(sd->fakename) + (int)strlen(message) + 3 + 1;
	} else {
		outlen = (int)strlen(full_message) + 1;
	}

	if (sd->fontcolor != 0 && sd->chat_id == 0) {
		uint32 color = 0;

		if (sd->disguise == -1) {
			sd->fontcolor_tid = timer->add(timer->gettick()+5000, clif->undisguise_timer, sd->bl.id, 0);
			pc->disguise(sd,sd->status.class);
			if (pc_isdead(sd))
				clif->clearunit_single(-sd->bl.id, CLR_DEAD, sd->fd);
			if (unit->is_walking(&sd->bl))
				clif->move(&sd->ud);
		} else if (sd->disguise == sd->status.class && sd->fontcolor_tid != INVALID_TIMER) {
			const struct TimerData *td;
			if ((td = timer->get(sd->fontcolor_tid)) != NULL)
				timer->settick(sd->fontcolor_tid, td->tick+5000);
		}

		int fontColor = sd->fontcolor - 1;
		if (fontColor < 0 || fontColor >= channel->config->colors_count)
			fontColor = 0;
		color = channel->config->colors[fontColor];
		WFIFOHEAD(fd, outlen + 12);
		WFIFOW(fd,0) = 0x2C1;
		WFIFOW(fd,2) = outlen + 12;
		WFIFOL(fd,4) = sd->bl.id;
		WFIFOL(fd,8) = RGB2BGR(color);
		if (is_fakename)
			safesnprintf(WFIFOP(fd, 12), outlen, "%s : %s", sd->fakename, message);
		else
			safestrncpy(WFIFOP(fd, 12), full_message, outlen);
		clif->send(WFIFOP(fd,0), WFIFOW(fd,2), &sd->bl, AREA_WOS);
		WFIFOL(fd,4) = -sd->bl.id;
		WFIFOSET(fd, outlen + 12);
		return;
	}

	{
		// send message to others
		void *buf = aMalloc(8 + outlen);
		WBUFW(buf, 0) = 0x8d;
		WBUFW(buf, 2) = 8 + outlen;
		WBUFL(buf, 4) = sd->bl.id;
		if (is_fakename)
			safesnprintf(WBUFP(buf, 8), outlen, "%s : %s", sd->fakename, message);
		else
			safestrncpy(WBUFP(buf, 8), full_message, outlen);
		//FIXME: chat has range of 9 only
		clif->send(buf, WBUFW(buf, 2), &sd->bl, sd->chat_id != 0 ? CHAT_WOS : AREA_CHAT_WOC);
		aFree(buf);
	}

	// send back message to the speaker
	WFIFOHEAD(fd, 4 + outlen);
	WFIFOW(fd, 0) = 0x8e;
	WFIFOW(fd, 2) = 4 + outlen;
	if (is_fakename)
		safesnprintf(WFIFOP(fd, 4), outlen, "%s : %s", sd->fakename, message);
	else
		safestrncpy(WFIFOP(fd, 4), full_message, outlen);
	WFIFOSET(fd, WFIFOW(fd,2));

	// Chat logging type 'O' / Global Chat
	logs->chat(LOG_CHAT_GLOBAL, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, message);

	// trigger listening npcs
	map->foreachinrange(npc_chat->sub, &sd->bl, AREA_SIZE, BL_NPC, full_message, strlen(full_message), &sd->bl);
}

static void clif_parse_MapMove(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /mm /mapmove (as @rura GM command) (CZ_MOVETO_MAP).
/// Request to warp to a map on given coordinates.
/// 0140 <map name>.16B <x>.W <y>.W
static void clif_parse_MapMove(int fd, struct map_session_data *sd)
{
	char command[MAP_NAME_LENGTH_EXT+25];
	char map_name[MAP_NAME_LENGTH_EXT];

	safestrncpy(map_name, RFIFOP(fd,2), MAP_NAME_LENGTH_EXT);
	sprintf(command, "%cmapmove %s %d %d", atcommand->at_symbol, map_name, RFIFOW(fd,18), RFIFOW(fd,20));
	atcommand->exec(fd, sd, command, true);
}

/// Updates body and head direction of an object (ZC_CHANGE_DIRECTION).
/// 009c <id>.L <head dir>.W <dir>.B
/// head dir:
///     0 = straight
///     1 = turned CW
///     2 = turned CCW
/// dir: @see enum unit_dir
static void clif_changed_dir(struct block_list *bl, enum send_target target)
{
	unsigned char buf[64];

	nullpo_retv(bl);
	WBUFW(buf,0) = 0x9c;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = bl->type == BL_PC ? BL_UCCAST(BL_PC, bl)->head_dir : 0;
	WBUFB(buf,8) = unit->getdir(bl);

	clif->send(buf, packet_len(0x9c), bl, target);

	if (clif->isdisguised(bl)) {
		WBUFL(buf,2) = -bl->id;
		WBUFW(buf,6) = 0;
		clif->send(buf, packet_len(0x9c), bl, SELF);
	}
}

static void clif_parse_ChangeDir(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change own body and head direction.
/// 009b <head dir>.W <dir>.B (CZ_CHANGE_DIRECTION)
/// 0361 <head dir>.W <dir>.B (CZ_CHANGE_DIRECTION2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_ChangeDir(int fd, struct map_session_data *sd)
{
	unsigned char headdir, dir;

	headdir = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	dir = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	pc_setdir(sd, dir, headdir);

	clif->changed_dir(&sd->bl, AREA_WOS);
}

static void clif_parse_Emotion(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to show an emotion (CZ_REQ_EMOTION).
/// 00bf <type>.B
/// type:
///     @see enum emotion_type
static void clif_parse_Emotion(int fd, struct map_session_data *sd)
{
	int emoticon = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	if (battle_config.basic_skill_check == 0 || pc->check_basicskill(sd, 2)) {
		if (emoticon == E_MUTE) {// prevent use of the mute emote [Valaris]
			clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 1, 0);
			return;
		}
		// fix flood of emotion icon (ro-proxy): flood only the hacker player
		if (sd->emotionlasttime + 1 >= time(NULL)) { // not more than 1 per second
			sd->emotionlasttime = time(NULL);
			clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 1, 0);
			return;
		}
		sd->emotionlasttime = time(NULL);

		pc->update_idle_time(sd, BCIDLE_EMOTION);

		if(battle_config.client_reshuffle_dice && emoticon>=E_DICE1 && emoticon<=E_DICE6) {// re-roll dice
			emoticon = rnd()%6+E_DICE1;
		}

		clif->emotion(&sd->bl, emoticon);
	} else
		clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 1, 0);
}

/// Amount of currently online players, reply to /w /who (ZC_USER_COUNT).
/// 00c2 <count>.L
static void clif_user_count(struct map_session_data *sd, int count)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0xc2));
	WFIFOW(fd,0) = 0xc2;
	WFIFOL(fd,2) = count;
	WFIFOSET(fd,packet_len(0xc2));
}

static void clif_parse_HowManyConnections(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /w /who (CZ_REQ_USER_COUNT).
/// Request to display amount of currently connected players.
/// 00c1
static void clif_parse_HowManyConnections(int fd, struct map_session_data *sd)
{
	clif->user_count(sd, map->getusers());
}

static void clif_parse_ActionRequest_sub(struct map_session_data *sd, enum action_type action_type, int target_id, int64 tick)
{
	nullpo_retv(sd);
	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl, CLR_DEAD);
		return;
	}

	// Statuses that don't let the player sit / attack / talk with NPCs(targeted)
	// (not all are included in pc_can_attack)
	if( sd->sc.count && (
			sd->sc.data[SC_TRICKDEAD] ||
			(sd->sc.data[SC_AUTOCOUNTER] && action_type != ACT_ATTACK_REPEAT) ||
			 sd->sc.data[SC_BLADESTOP] ||
			 sd->sc.data[SC_DEEP_SLEEP] ||
			 sd->sc.data[SC_SUHIDE] )
			 )
		return;

	if (action_type != ACT_ATTACK && action_type != ACT_ATTACK_REPEAT)
		pc_stop_walking(sd, STOPWALKING_FLAG_FIXPOS);
	pc_stop_attack(sd);

	if(target_id<0 && -target_id == sd->bl.id) // for disguises [Valaris]
		target_id = sd->bl.id;

	switch(action_type) {
		case ACT_ATTACK: // once attack
		case ACT_ATTACK_REPEAT: // continuous attack
		{
			struct npc_data *nd = map->id2nd(target_id);
			if (nd != NULL) {
				if (sd->block_action.npc == 0) { // *pcblock script command
					npc->click(sd, nd);
				}
				return;
			}

			if (pc_cant_act_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0)
			    || pc_issit(sd) || (sd->sc.option & OPTION_HIDE) != 0 || pc_isvending(sd)) {
				return;
			}

			if (sd->sc.option & OPTION_COSTUME)
				return;

			if (!battle_config.sdelay_attack_enable && pc->checkskill(sd, SA_FREECAST) <= 0 && (skill->get_inf2(sd->ud.skill_id) & (INF2_FREE_CAST_REDUCED | INF2_FREE_CAST_NORMAL)) == 0) {
				if (DIFF_TICK(tick, sd->ud.canact_tick) < 0) {
					clif->skill_fail(sd, 1, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
					return;
				}
			}

			pc->delinvincibletimer(sd);
			pc->update_idle_time(sd, BCIDLE_ATTACK);
			unit->attack(&sd->bl, target_id, action_type != ACT_ATTACK);
		}
		break;
		case ACT_SIT: // sitdown
			if (battle_config.basic_skill_check && !pc->check_basicskill(sd, 3)) {
				clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 2, 0);
				break;
			}

			if (sd->sc.data[SC_SITDOWN_FORCE] || sd->sc.data[SC_BANANA_BOMB_SITDOWN_POSTDELAY])
				return;

			if(pc_issit(sd)) {
				//Bugged client? Just refresh them.
				clif->sitting(&sd->bl);
				return;
			}

			if (sd->block_action.sitstand) // *pcblock script command
				break;

			if (sd->ud.skilltimer != INVALID_TIMER || (sd->sc.opt1 && sd->sc.opt1 != OPT1_BURNING ))
				break;

			if (sd->sc.count && (
				sd->sc.data[SC_DANCING] ||
				sd->sc.data[SC_ANKLESNARE] ||
				(sd->sc.data[SC_GRAVITATION] && sd->sc.data[SC_GRAVITATION]->val3 == BCT_SELF)
			)) //No sitting during these states either.
				break;

			pc->update_idle_time(sd, BCIDLE_SIT);

			pc_setsit(sd);
			skill->sit(sd,1);
			clif->sitting(&sd->bl);
		break;
		case ACT_STAND: // standup

			if (sd->sc.data[SC_SITDOWN_FORCE] || sd->sc.data[SC_BANANA_BOMB_SITDOWN_POSTDELAY])
				return;

			if (!pc_issit(sd)) {
				//Bugged client? Just refresh them.
				clif->standing(&sd->bl);
				return;
			}

			if (sd->block_action.sitstand) // *pcblock script command
				break;

			pc->update_idle_time(sd, BCIDLE_SIT);

			pc->setstand(sd);
			skill->sit(sd,0);
			clif->standing(&sd->bl);
		break;
	}
}

static void clif_parse_ActionRequest(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request for an action.
/// 0089 <target id>.L <action>.B (CZ_REQUEST_ACT)
/// 0437 <target id>.L <action>.B (CZ_REQUEST_ACT2)
/// action:
///     0 = attack
///     1 = pick up item
///     2 = sit down
///     3 = stand up
///     7 = continuous attack
///     12 = (touch skill?)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_ActionRequest(int fd, struct map_session_data *sd)
{
	clif->pActionRequest_sub(sd,
		(enum action_type)RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[1]),
		RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]),
		timer->gettick()
	);
}

static void clif_parse_Restart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Response to the death/system menu (CZ_RESTART).
/// 00b2 <type>.B
/// type:
///     0 = restart (respawn)
///     1 = char-select (disconnect)
static void clif_parse_Restart(int fd, struct map_session_data *sd)
{
	switch(RFIFOB(fd,2)) {
		case 0x00:
			pc->respawn(sd,CLR_OUTSIGHT);
			break;
		case 0x01:
			/* Rovert's Prevent logout option - Fixed [Valaris] */
			if (!sd->sc.data[SC_CLOAKING] && !sd->sc.data[SC_HIDING] && !sd->sc.data[SC_CHASEWALK]
			 && !sd->sc.data[SC_CLOAKINGEXCEED] && !sd->sc.data[SC__INVISIBILITY] && !sd->sc.data[SC_SUHIDE]
			 && (!battle_config.prevent_logout || DIFF_TICK(timer->gettick(), sd->canlog_tick) > battle_config.prevent_logout)
			) {
				//Send to char-server for character selection.
				chrif->charselectreq(sd, sockt->session[fd]->client_addr);
			} else {
				clif->disconnect_ack(sd, 1);
			}
			break;
	}
}

/**
 * Validates and processes whispered messages (CZ_WHISPER).
 *
 * @code
 * 0096 <packet len>.W <nick>.24B <message>.?B
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 */
static void clif_parse_WisMessage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_WisMessage(int fd, struct map_session_data *sd)
{
	struct map_session_data* dstsd;
	int i;

	char target[NAME_LENGTH + 1]; // Client can send 24 characters without NULL terminator.
	char message[CHAT_SIZE_MAX + 1];
	const struct packet_whisper_message *packet = RP2PTR(fd);

	if (!clif->process_whisper_message(sd, packet, target, message, sizeof message))
		return;

	// Chat logging type 'W' / Whisper
	logs->chat(LOG_CHAT_WHISPER, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, target, message);

	//-------------------------------------------------------//
	//   Lordalfa - Paperboy - To whisper NPC commands       //
	//-------------------------------------------------------//
	if (target[0] && (strncasecmp(target,"NPC:",4) == 0) && (strlen(target) > 4)) {
		char *str = target + 4; // Skip the NPC: string part.
		struct npc_data *nd;
		if ((nd = npc->name2id(str))) {
			char split_data[NUM_WHISPER_VAR][SCRIPT_STRING_VAR_LENGTH + 1];
			char *split;
			char output[256];

			str = message;
			// skip codepage indicator, if detected
			if( str[0] == '|' && strlen(str) >= 4 )
				str += 3;
			for( i = 0; i < NUM_WHISPER_VAR; ++i ) {// Splits the message using '#' as separators
				split = strchr(str,'#');
				if( split == NULL ) { // use the remaining string
					safestrncpy(split_data[i], str, ARRAYLENGTH(split_data[i]));
					for( ++i; i < NUM_WHISPER_VAR; ++i )
						split_data[i][0] = '\0';
					break;
				}
				*split = '\0';
				safestrncpy(split_data[i], str, ARRAYLENGTH(split_data[i]));
				str = split+1;
			}

			for( i = 0; i < NUM_WHISPER_VAR; ++i ) {
				sprintf(output, "@whispervar%d$", i);
				script->set_var(sd,output,(char *) split_data[i]);
			}

			safesnprintf(output, 255, "%s::OnWhisperGlobal", nd->exname);
			npc->event(sd,output, 0); // Calls the NPC label

			return;
		}
	} else if( target[0] == '#' ) {
		const char *chname = target;
		struct channel_data *chan = channel->search(chname, sd);

		if (chan) {
			int k;
			ARR_FIND(0, VECTOR_LENGTH(sd->channels), k, VECTOR_INDEX(sd->channels, k) == chan);
			if (k < VECTOR_LENGTH(sd->channels) || channel->join(chan, sd, "", true) == HCS_STATUS_OK) {
				channel->send(chan,sd,message);
			} else {
				clif->message(fd, msg_fd(fd,1402)); //You're not in that channel, type '@join <#channel_name>'
			}
			return;
		} else if (strcmpi(&chname[1], channel->config->ally_name) == 0) {
			clif->message(fd, msg_fd(fd,1294)); // You're not allowed to talk on this channel
			return;
		}
	}

	// searching destination character
	dstsd = map->nick2sd(target, false);

	if (dstsd == NULL) {
		// Character not found (or found through partial match).
		clif->wis_end(sd->fd, 1);
		return;
	}

	// if player ignores everyone
	if (dstsd->state.ignoreAll && pc_get_group_level(sd) <= pc_get_group_level(dstsd)) {
		if (pc_isinvisible(dstsd) && pc_get_group_level(sd) < pc_get_group_level(dstsd))
			clif->wis_end(fd, 1); // 1: target character is not logged in
		else
			clif->wis_end(fd, 3); // 3: everyone ignored by target
		return;
	}

	// if player is autotrading
	if (dstsd->state.autotrade) {
		char output[256];
		sprintf(output, msg_fd(fd, 894), dstsd->status.name); // %s is in autotrade mode and cannot receive whispered messages.
		clif->wis_message(fd, map->wisp_server_name, output, (int)strlen(output));
		return;
	}

	if( pc_get_group_level(sd) <= pc_get_group_level(dstsd) ) {
		// if player ignores the source character
		ARR_FIND(0, MAX_IGNORE_LIST, i, dstsd->ignore[i].name[0] == '\0' || strcmp(dstsd->ignore[i].name, sd->status.name) == 0);
		if(i < MAX_IGNORE_LIST && dstsd->ignore[i].name[0] != '\0') { // source char present in ignore list
			clif->wis_end(fd, 2); // 2: ignored by target
			return;
		}
	}

	// notify sender of success
	clif->wis_end(fd, 0); // 0: success to send wisper

	// Normal message
	clif->wis_message(dstsd->fd, sd->status.name, message, (int)strlen(message));
}

static void clif_parse_Broadcast(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /b /nb (CZ_BROADCAST).
/// Request to broadcast a message on whole server.
/// 0099 <packet len>.W <text>.?B 00
static void clif_parse_Broadcast(int fd, struct map_session_data *sd)
{
	const char commandname[] = "kami";
	char command[sizeof commandname + 2 + CHAT_SIZE_MAX] = ""; // '@' command + ' ' + message + NUL
	int len = (int)RFIFOW(fd,2) - 4;

	if (len <= 0)
		return;

	sprintf(command, "%c%s ", atcommand->at_symbol, commandname);

	// as the length varies depending on the command used, truncate unreasonably long strings
	if (len >= (int)(sizeof command - strlen(command)))
		len = (int)(sizeof command - strlen(command)) - 1;

	strncat(command, RFIFOP(fd,4), len);
	atcommand->exec(fd, sd, command, true);
}

static void clif_parse_TakeItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to pick up an item.
/// 009f <id>.L (CZ_ITEM_PICKUP)
/// 0362 <id>.L (CZ_ITEM_PICKUP2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_TakeItem(int fd, struct map_session_data *sd)
{
	int map_object_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	struct flooritem_data *fitem = map->id2fi(map_object_id);

	do {
		if (pc_isdead(sd)) {
			clif->clearunit_area(&sd->bl, CLR_DEAD);
			break;
		}

		if (fitem == NULL || fitem->bl.m != sd->bl.m)
			break;

		if( sd->sc.count && (
				 sd->sc.data[SC_HIDING] ||
				 sd->sc.data[SC_CLOAKING] ||
				 sd->sc.data[SC_TRICKDEAD] ||
				 sd->sc.data[SC_BLADESTOP] ||
				 sd->sc.data[SC_CLOAKINGEXCEED] ||
				 sd->sc.data[SC_SUHIDE] ||
				 pc_ismuted(&sd->sc, MANNER_NOITEM)
			) )
			break;

		if (pc_cant_act_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0))
			break;

		if (!pc->takeitem(sd, fitem))
			break;

		return;
	} while (0);
	// Client REQUIRES a fail packet or you can no longer pick items.
	clif->additem(sd,0,0,6);
}

static void clif_parse_DropItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to drop an item.
/// 00a2 <index>.W <amount>.W (CZ_ITEM_THROW)
/// 0363 <index>.W <amount>.W (CZ_ITEM_THROW2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_DropItem(int fd, struct map_session_data *sd)
{
	int item_index = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2;
	int item_amount = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[1]);

	for(;;) {
		if (pc_isdead(sd))
			break;

		if (pc_cant_act_except_npc_chat(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0))
			break;

		if (sd->sc.count && (
			sd->sc.data[SC_AUTOCOUNTER] ||
			sd->sc.data[SC_BLADESTOP] ||
			pc_ismuted(&sd->sc, MANNER_NOITEM)
		))
			break;

		if (!pc->dropitem(sd, item_index, item_amount))
			break;

		pc->update_idle_time(sd, BCIDLE_DROPITEM);

		return;
	}

	//Because the client does not like being ignored.
	clif->dropitem(sd, item_index, 0);
}

static void clif_parse_UseItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to use an item.
/// 00a7 <index>.W <account id>.L (CZ_USE_ITEM)
/// 0439 <index>.W <account id>.L (CZ_USE_ITEM2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_UseItem(int fd, struct map_session_data *sd)
{
	int n;

	if (pc_isvending(sd))
		return;

	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl, CLR_DEAD);
		return;
	}

	if ((!sd->npc_id && pc_istrading(sd)) || sd->chat_id != 0)
		return;

	//Whether the item is used or not is irrelevant, the char ain't idle. [Skotlex]
	pc->update_idle_time(sd, BCIDLE_USEITEM);
	n = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2;

	if (n < 0 || n >= sd->status.inventorySize)
		return;
	if (!pc->useitem(sd,n))
		clif->useitemack(sd,n,0,false); //Send an empty ack packet or the client gets stuck.
}

static void clif_parse_EquipItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to equip an item (CZ_REQ_WEAR_EQUIP).
/// 00a9 <index>.W <position>.W
/// 0998 <index>.W <position>.L
static void clif_parse_EquipItem(int fd, struct map_session_data *sd)
{
	const struct packet_equip_item *p = RP2PTR(fd);

	if (pc_isvending(sd))
		return;

	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl,CLR_DEAD);
		return;
	}

	int index = p->index - 2;
	if (index < 0 || index >= sd->status.inventorySize)
		return; //Out of bounds check.

	if( sd->npc_id ) {
		if ((sd->npc_item_flag & ITEMENABLEDNPC_EQUIP) == 0 && sd->state.using_megaphone == 0)
			return;
	} else if (sd->state.storage_flag != STORAGE_FLAG_CLOSED || sd->sc.opt1)
		; //You can equip/unequip stuff while storage is open/under status changes
	else if ( pc_cant_act2(sd) || sd->state.prerefining )
		return;

	if (!sd->status.inventory[index].identify) {
		clif->equipitemack(sd, index, 0, EIA_FAIL);  // fail
		return;
	}

	if (!sd->inventory_data[index])
		return;

	if (sd->inventory_data[index]->type == IT_PETARMOR) {
		pet->equipitem(sd, index);
		return;
	}

	pc->update_idle_time(sd, BCIDLE_USEITEM);

	//Client doesn't send the position for ammo.
	if (sd->inventory_data[index]->type == IT_AMMO)
		pc->equipitem(sd, index, EQP_AMMO);
	else
		pc->equipitem(sd, index, p->wearLocation);
}

static void clif_parse_UnequipItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to take off an equip (CZ_REQ_TAKEOFF_EQUIP).
/// 00ab <index>.W
static void clif_parse_UnequipItem(int fd, struct map_session_data *sd)
{
	int index;

	if (pc_isvending(sd))
		return;

	if(pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl,CLR_DEAD);
		return;
	}

	if( sd->npc_id ) {
		if ((sd->npc_item_flag & ITEMENABLEDNPC_EQUIP) == 0 && sd->state.using_megaphone == 0)
			return;
	} else if (sd->state.storage_flag != STORAGE_FLAG_CLOSED || sd->sc.opt1)
		; //You can equip/unequip stuff while storage is open/under status changes
	else if ( pc_cant_act2(sd) || sd->state.prerefining )
		return;

	index = RFIFOW(fd,2)-2;

	pc->update_idle_time(sd, BCIDLE_USEITEM);

	pc->unequipitem(sd,index, PCUNEQUIPITEM_RECALC);
}

static void clif_parse_NpcClicked(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to start a conversation with an NPC (CZ_CONTACTNPC).
/// 0090 <id>.L <type>.B
/// type:
///     1 = click
static void clif_parse_NpcClicked(int fd, struct map_session_data *sd)
{
	struct block_list *bl;

	if( pc_isdead(sd) ) {
		clif->clearunit_area(&sd->bl,CLR_DEAD);
		return;
	}
	if (sd->npc_id > 0 || (sd->state.workinprogress & 2) == 2 || sd->block_action.npc == 1) {	// *pcblock script command
#if PACKETVER >= 20110308
		clif->msgtable(sd, MSG_BUSY);
#else
		clif->messagecolor_self(fd, COLOR_WHITE, msg_fd(fd, 48));
#endif
		return;
	}
	if (pc_cant_act2(sd) || !(bl = map->id2bl(RFIFOL(fd,2))) || sd->state.vending || sd->state.prevend)
		return;

	switch (bl->type) {
		case BL_MOB:
		case BL_PC:
			clif->pActionRequest_sub(sd, ACT_ATTACK_REPEAT, bl->id, timer->gettick());
			break;
		case BL_NPC:
			if (sd->ud.skill_id < RK_ENCHANTBLADE && sd->ud.skilltimer != INVALID_TIMER) { // TODO: should only work with none 3rd job skills
#if PACKETVER >= 20110308
				clif->msgtable(sd, MSG_BUSY);
#else
				clif->messagecolor_self(fd, COLOR_WHITE, msg_fd(fd, 48));
#endif
				break;
			}
			if( bl->m != -1 )// the user can't click floating npcs directly (hack attempt)
				npc->click(sd, BL_UCAST(BL_NPC, bl));
			break;
	}
}

static void clif_parse_NpcBuySellSelected(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Selection between buy/sell was made (CZ_ACK_SELECT_DEALTYPE).
/// 00c5 <id>.L <type>.B
/// type:
///     0 = buy
///     1 = sell
static void clif_parse_NpcBuySellSelected(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;
	npc->buysellsel(sd, RFIFOL(fd,2), RFIFOB(fd,6));
}

/// Notification about the result of a purchase attempt from an NPC shop (ZC_PC_PURCHASE_RESULT).
/// 00ca <result>.B
/// result:
///     0 = "The deal has successfully completed."
///     1 = "You do not have enough zeny."
///     2 = "You are over your Weight Limit."
///     3 = "Out of the maximum capacity, you have too many items."
///     9 = "Amounts are exceeded the possession of the item is not available for purchase."
///    10 = "Props open-air store sales will be traded in RODEX"
///    11 = "The exchange failed."
///    12 = "The exchange was well done."
///    13 = "The item is already sold and out of stock."
///    14 = "There is not enough goods to exchange."
static void clif_npc_buy_result(struct map_session_data *sd, unsigned char result)
{
	int fd;

	nullpo_retv(sd);
	pc->update_idle_time(sd, BCIDLE_SCRIPT);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xca));
	WFIFOW(fd,0) = 0xca;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,packet_len(0xca));
}

static void clif_parse_NpcBuyListSend(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to buy chosen items from npc shop (CZ_PC_PURCHASE_ITEMLIST).
/// 00c8 <packet len>.W { <amount>.W <name id>.W }*
static void clif_parse_NpcBuyListSend(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_PC_PURCHASE_ITEMLIST *p = RFIFOP(fd, 0);
	int n = ((int)p->packetLength - sizeof(struct PACKET_CZ_PC_PURCHASE_ITEMLIST)) / sizeof(struct PACKET_CZ_PC_PURCHASE_ITEMLIST_sub);
	int result;

	Assert_retv(n >= 0);

	if (sd->state.trading || !sd->npc_shopid || pc_has_permission(sd, PC_PERM_DISABLE_STORE)) {
		result = 1;
	} else {
		struct itemlist item_list = { 0 };
		int i;

		VECTOR_INIT(item_list);
		VECTOR_ENSURE(item_list, n, 1);
		for (i = 0; i < n; i++) {
			struct itemlist_entry entry = { 0 };

			entry.amount = p->items[i].amount;
			entry.id =     p->items[i].itemId;

			VECTOR_PUSH(item_list, entry);
		}
		result = npc->buylist(sd, &item_list);
		VECTOR_CLEAR(item_list);
	}

	sd->npc_shopid = 0; //Clear shop data.

	clif->npc_buy_result(sd, result);
}

/// Notification about the result of a sell attempt to an NPC shop (ZC_PC_SELL_RESULT).
/// 00cb <result>.B
/// result:
///     0 = "The deal has successfully completed."
///     1 = "The deal has failed."
static void clif_npc_sell_result(struct map_session_data *sd, unsigned char result)
{
	int fd;

	nullpo_retv(sd);
	pc->update_idle_time(sd, BCIDLE_SCRIPT);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xcb));
	WFIFOW(fd,0) = 0xcb;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,packet_len(0xcb));
}

static void clif_parse_NpcSellListSend(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to sell chosen items to npc shop (CZ_PC_SELL_ITEMLIST).
/// 00c9 <packet len>.W { <index>.W <amount>.W }*
static void clif_parse_NpcSellListSend(int fd, struct map_session_data *sd)
{
	int fail=0,n;

	n = ((int)RFIFOW(fd,2)-4) /4;

	Assert_retv(n >= 0);

	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd) || !sd->npc_shopid) {
		fail = 1;
	} else {
		struct itemlist item_list = { 0 };
		int i;

		VECTOR_INIT(item_list);
		VECTOR_ENSURE(item_list, n, 1);

		for (i = 0; i < n; i++) {
			struct itemlist_entry entry = { 0 };

			entry.id = (int)RFIFOW(fd, 4 + 4 * i) - 2;
			entry.amount =  RFIFOW(fd, 4 + 4 * i + 2);

			VECTOR_PUSH(item_list, entry);
		}
		fail = npc->selllist(sd, &item_list);

		VECTOR_CLEAR(item_list);
	}

	sd->npc_shopid = 0; //Clear shop data.

	clif->npc_sell_result(sd, fail);
}

static void clif_parse_CreateChatRoom(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Chatroom creation request (CZ_CREATE_CHATROOM).
/// 00d5 <packet len>.W <limit>.W <type>.B <passwd>.8B <title>.?B
/// type:
///     0 = private
///     1 = public
static void clif_parse_CreateChatRoom(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	int len = (int)RFIFOW(fd, 2) - 15;
	int limit;
	bool pub;
	const char *password; //not zero-terminated
	const char *title; // not zero-terminated
	char s_password[CHATROOM_PASS_SIZE];
	char s_title[CHATROOM_TITLE_SIZE];

	if (len < 1)
		return;

	limit = RFIFOW(fd, 4);
	pub = (RFIFOB(fd, 6) != 0);
	password = RFIFOP(fd, 7); //not zero-terminated
	title = RFIFOP(fd, 15); // not zero-terminated

	if (limit < 0)
		return;

	if (pc_ismuted(&sd->sc, MANNER_NOROOM))
		return;
	if(battle_config.basic_skill_check && !pc->check_basicskill(sd, 4)) {
		clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 3, 0);
		return;
	}
	if( npc->isnear(&sd->bl) ) {
		// uncomment for more verbose message.
		//char output[150];
		//sprintf(output, msg_txt(862), battle_config.min_npc_vendchat_distance); // "You're too close to a NPC, you must be at least %d cells away from any NPC."
		//clif_displaymessage(sd->fd, output);
		clif->skill_fail(sd, 1, USESKILL_FAIL_THERE_ARE_NPC_AROUND, 0, 0);
		return;
	}

	safestrncpy(s_password, password, CHATROOM_PASS_SIZE);
	safestrncpy(s_title, title, min(len+1,CHATROOM_TITLE_SIZE)); //NOTE: assumes that safestrncpy will not access the len+1'th byte

	chat->create_pc_chat(sd, s_title, s_password, limit, pub);
}

static void clif_parse_ChatAddMember(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Chatroom join request (CZ_REQ_ENTER_ROOM).
/// 00d9 <chat ID>.L <passwd>.8B
static void clif_parse_ChatAddMember(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	int chatid = RFIFOL(fd,2);
	const char *password = RFIFOP(fd,6); // not zero-terminated

	chat->join(sd,chatid,password);
}

static void clif_parse_ChatRoomStatusChange(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Chatroom properties adjustment request (CZ_CHANGE_CHATROOM).
/// 00de <packet len>.W <limit>.W <type>.B <passwd>.8B <title>.?B
/// type:
///     0 = private
///     1 = public
static void clif_parse_ChatRoomStatusChange(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	int len = (int)RFIFOW(fd, 2) - 15;
	int limit;
	bool pub;
	const char *password; // not zero-terminated
	const char *title; // not zero-terminated
	char s_password[CHATROOM_PASS_SIZE];
	char s_title[CHATROOM_TITLE_SIZE];

	if (len < 1)
		return;

	limit = RFIFOW(fd, 4);
	if (limit < 0)
		return;
	pub = (RFIFOB(fd, 6) != 0);
	password = RFIFOP(fd, 7); // not zero-terminated
	title = RFIFOP(fd, 15); // not zero-terminated

	safestrncpy(s_password, password, CHATROOM_PASS_SIZE);
	safestrncpy(s_title, title, min(len+1,CHATROOM_TITLE_SIZE)); //NOTE: assumes that safestrncpy will not access the len+1'th byte

	chat->change_status(sd, s_title, s_password, limit, pub);
}

static void clif_parse_ChangeChatOwner(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change the chat room ownership (CZ_REQ_ROLE_CHANGE).
/// 00e0 <role>.L <nick>.24B
/// role:
///     0 = owner
///     1 = normal
static void clif_parse_ChangeChatOwner(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	chat->change_owner(sd, RFIFOP(fd,6)); // non null terminated
}

static void clif_parse_KickFromChat(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to expel a player from chat room (CZ_REQ_EXPEL_MEMBER).
/// 00e2 <name>.24B
static void clif_parse_KickFromChat(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	chat->kick(sd, RFIFOP(fd,2)); // non null terminated
}

static void clif_parse_ChatLeave(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to leave the current chatroom (CZ_EXIT_ROOM).
/// 00e3
static void clif_parse_ChatLeave(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_CHAT) == 0) || pc_isvending(sd))
		return;

	chat->leave(sd, false);
}

//Handles notifying asker and rejecter of what has just occurred.
//Type is used to determine the correct msg_txt to use:
//0:
static void clif_noask_sub(struct map_session_data *src, struct map_session_data *target, int type)
{
	const char* msg;
	char output[256];
	nullpo_retv(src);
	// Your request has been rejected by autoreject option.
	msg = msg_sd(src,392);
	clif_disp_onlyself(src, msg);
	//Notice that a request was rejected.
	snprintf(output, 256, msg_sd(target,393+type), src->status.name, 256);
	clif_disp_onlyself(target, output);
}

static void clif_parse_TradeRequest(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to begin a trade (CZ_REQ_EXCHANGE_ITEM).
/// 00e4 <account id>.L
static void clif_parse_TradeRequest(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd = map->id2sd(RFIFOL(fd, 2));

	if (pc_cant_act_except_npc_chat(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0))
		return; //You can trade while in a chatroom.

	// @noask [LuzZza]
	if(t_sd && t_sd->state.noask) {
		clif->noask_sub(sd, t_sd, 0);
		return;
	}

	if( battle_config.basic_skill_check && !pc->check_basicskill(sd, 1)) {
		clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 0, 0);
		return;
	}

	trade->request(sd,t_sd);
}

static void clif_parse_TradeAck(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to a trade request (CZ_ACK_EXCHANGE_ITEM).
/// 00e6 <result>.B
/// result:
///     3 = accepted
///     4 = rejected
static void clif_parse_TradeAck(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	trade->ack(sd,RFIFOB(fd,2));
}

static void clif_parse_TradeAddItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add an item to current trade (CZ_ADD_EXCHANGE_ITEM).
/// 00e8 <index>.W <amount>.L
static void clif_parse_TradeAddItem(int fd, struct map_session_data *sd)
{
	if (sd->state.trading == 0 || (pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	short index = RFIFOW(fd,2);
	int amount = RFIFOL(fd,4);

	if( index == 0 )
		trade->addzeny(sd, amount);
	else
		trade->additem(sd, index, (short)amount);
}

static void clif_parse_TradeOk(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to lock items in current trade (CZ_CONCLUDE_EXCHANGE_ITEM).
/// 00eb
static void clif_parse_TradeOk(int fd, struct map_session_data *sd)
{
	if ((pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	trade->ok(sd);
}

static void clif_parse_TradeCancel(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to cancel current trade (CZ_CANCEL_EXCHANGE_ITEM).
/// 00ed
static void clif_parse_TradeCancel(int fd, struct map_session_data *sd)
{
	if ((pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	trade->cancel(sd);
}

static void clif_parse_TradeCommit(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to commit current trade (CZ_EXEC_EXCHANGE_ITEM).
/// 00ef
static void clif_parse_TradeCommit(int fd, struct map_session_data *sd)
{
	if ((pc_isdead(sd) && (battle_config.allowed_actions_when_dead & PCALLOWACTION_TRADE) == 0) || pc_isvending(sd))
		return;

	trade->commit(sd);
}

static void clif_parse_StopAttack(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to stop chasing/attacking an unit (CZ_CANCEL_LOCKON).
/// 0118
static void clif_parse_StopAttack(int fd, struct map_session_data *sd)
{
	pc_stop_attack(sd);
}

static void clif_parse_PutItemToCart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from inventory to cart (CZ_MOVE_ITEM_FROM_BODY_TO_CART).
/// 0126 <index>.W <amount>.L
static void clif_parse_PutItemToCart(int fd, struct map_session_data *sd)
{
	int flag = 0;

	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || sd->state.prevend != 0)
		return;

	if (!pc_iscarton(sd))
		return;
	if ( (flag = pc->putitemtocart(sd,RFIFOW(fd,2)-2,RFIFOL(fd,4))) ) {
		clif->item_movefailed(sd, RFIFOW(fd,2)-2);
		clif->cart_additem_ack(sd,flag == 1?0x0:0x1);
	}
}

static void clif_parse_GetItemFromCart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from cart to inventory (CZ_MOVE_ITEM_FROM_CART_TO_BODY).
/// 0127 <index>.W <amount>.L
static void clif_parse_GetItemFromCart(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || sd->state.prevend != 0)
		return;

	if (!pc_iscarton(sd))
		return;
	pc->getitemfromcart(sd,RFIFOW(fd,2)-2,RFIFOL(fd,4));
}

static void clif_parse_RemoveOption(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to remove cart/falcon/peco/dragon (CZ_REQ_CARTOFF).
/// 012a
static void clif_parse_RemoveOption(int fd, struct map_session_data *sd)
{
	if (pc_isridingpeco(sd) || pc_isfalcon(sd) || pc_isridingdragon(sd) || pc_ismadogear(sd)) {
		// priority to remove this option before we can clear cart
		pc->setoption(sd,sd->sc.option&~(OPTION_RIDING|OPTION_FALCON|OPTION_DRAGON|OPTION_MADOGEAR));
	} else {
#ifdef NEW_CARTS
		if (sd->sc.data[SC_PUSH_CART])
			pc->setcart(sd,0);
#else // not NEW_CARTS
		pc->setoption(sd,sd->sc.option&~OPTION_CART);
#endif // NEW_CARTS
	}
}

static void clif_parse_reqGearOff(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_reqGearOff(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190703 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190709
	const struct PACKET_CZ_REQ_MOUNTOFF *p = RFIFOP(fd, 0);
	switch (p->action) {
	case REMOVE_MOUNT_DRAGON:
		if (pc_isridingdragon(sd))
			pc->setoption(sd, sd->sc.option &~ OPTION_DRAGON);
		break;
	case REMOVE_MOUNT_MADO:
		if (pc_ismadogear(sd))
			pc->setoption(sd, sd->sc.option &~ OPTION_MADOGEAR);
		break;
	case REMOVE_MOUNT_PECO:
		if (pc_isridingpeco(sd))
			pc->setoption(sd, sd->sc.option &~ OPTION_RIDING);
		break;
	case REMOVE_MOUNT_FALCON:
		if (pc_isfalcon(sd))
			pc->setoption(sd, sd->sc.option &~ OPTION_FALCON);
		break;
	case REMOVE_MOUNT_CART:
		// this packet exists in clients with only new carts [4144]
		if (sd->sc.data[SC_PUSH_CART])
			pc->setcart(sd, 0);
		break;
	case REMOVE_MOUNT_0:
	case REMOVE_MOUNT_2:
	default:
		ShowError("Unknown action in remove mount packet: %d\n", p->action);
		break;
	}
#endif
}

static void clif_parse_ChangeCart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change cart's visual look (CZ_REQ_CHANGECART).
/// 01af <num>.W
static void clif_parse_ChangeCart(int fd, struct map_session_data *sd)
{// TODO: State tracking?
	int type;

	if (pc->checkskill(sd, MC_CHANGECART) == 0)
		return;

	if ((sd->npc_id != 0 && sd->state.using_megaphone == 0) || (sd->state.workinprogress & 1) != 0) {
#if PACKETVER >= 20110308
		clif->msgtable(sd, MSG_BUSY);
#else
		clif->messagecolor_self(fd, COLOR_WHITE, msg_fd(fd, 48));
#endif
		return;
	}

	type = RFIFOW(fd, 2);

	if (
#ifdef NEW_CARTS
		(type == 9 && sd->status.base_level > 130) ||
		(type == 8 && sd->status.base_level > 120) ||
		(type == 7 && sd->status.base_level > 110) ||
		(type == 6 && sd->status.base_level > 100) ||
#endif
		(type == 5 && sd->status.base_level >  90) ||
		(type == 4 && sd->status.base_level >  80) ||
		(type == 3 && sd->status.base_level >  65) ||
		(type == 2 && sd->status.base_level >  40) ||
		(type == 1))

		pc->setcart(sd, type);
}

/// Request to select cart's visual look for new cart design (CZ_SELECTCART).
/// 0980 <identity>.L <type>.B
static void clif_parse_SelectCart(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20150805 // RagexeRE
	int type;

	if (!sd || !pc->checkskill(sd, MC_CARTDECORATE) || RFIFOL(fd, 2) != sd->status.account_id)
		return;

	type = RFIFOB(fd, 6);

	if (type <= MAX_BASE_CARTS || type > MAX_CARTS)
		return;

	pc->setcart(sd, type);
#endif
}

static void clif_parse_StatusUp(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to increase status (CZ_STATUS_CHANGE).
/// 00bb <status id>.W <amount>.B
/// status id:
///     SP_STR ~ SP_LUK
/// amount:
///     Old clients send always 1 for this, even when using /str+ and the like.
///     Newer clients (2013-12-23 and newer) send the correct amount.
static void clif_parse_StatusUp(int fd, struct map_session_data *sd)
{
	int increase_amount;

	increase_amount = RFIFOB(fd,4);
	if( increase_amount < 0 )
	{
		ShowDebug("clif_parse_StatusUp: Negative 'increase' value sent by client! (fd: %d, value: %d)\n",
			fd, increase_amount);
	}
	pc->statusup(sd, RFIFOW(fd,2), increase_amount);
}

static void clif_parse_SkillUp(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to increase level of a skill (CZ_UPGRADE_SKILLLEVEL).
/// 0112 <skill id>.W
static void clif_parse_SkillUp(int fd, struct map_session_data *sd)
{
	pc->skillup(sd,RFIFOW(fd,2));
}

static void clif_parse_UseSkillToId_homun(struct homun_data *hd, struct map_session_data *sd, int64 tick, uint16 skill_id, uint16 skill_lv, int target_id)
{
	int lv;

	nullpo_retv(sd);
	if( !hd )
		return;
	if (skill->not_ok_hom(skill_id, hd)){
		clif->emotion(&hd->bl, E_DOTS);
		return;
	}
	if (hd->bl.id != target_id && skill->get_inf(skill_id)&INF_SELF_SKILL)
		target_id = hd->bl.id;
	if (hd->ud.skilltimer != INVALID_TIMER) {
		if (skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST) return;
	}
	else if (DIFF_TICK(tick, hd->ud.canact_tick) < 0){
		clif->emotion(&hd->bl, E_DOTS);
		if (hd->master)
			clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
		return;

	}

	lv = homun->checkskill(hd, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit->skilluse_id(&hd->bl, target_id, skill_id, skill_lv);
}

static void clif_parse_UseSkillToPos_homun(struct homun_data *hd, struct map_session_data *sd, int64 tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
{
	int lv;
	nullpo_retv(sd);
	if( !hd )
		return;
	if (skill->not_ok_hom(skill_id, hd)){
		clif->emotion(&hd->bl, E_DOTS);
		return;
	}
	if ( hd->ud.skilltimer != INVALID_TIMER ) {
		if ( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST ) return;

	} else if ( DIFF_TICK(tick, hd->ud.canact_tick) < 0 ) {
		clif->emotion(&hd->bl, E_DOTS);
		if ( hd->master )
			clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
		return;
	}

	if( hd->sc.data[SC_BASILICA] )
		return;
	lv = homun->checkskill(hd, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit->skilluse_pos(&hd->bl, x, y, skill_id, skill_lv);
}

static void clif_parse_UseSkillToId_mercenary(struct mercenary_data *md, struct map_session_data *sd, int64 tick, uint16 skill_id, uint16 skill_lv, int target_id)
{
	int lv;

	nullpo_retv(sd);
	if( !md )
		return;
	if( skill->not_ok_mercenary(skill_id, md) )
		return;
	if( md->bl.id != target_id && skill->get_inf(skill_id)&INF_SELF_SKILL )
		target_id = md->bl.id;
	if( md->ud.skilltimer != INVALID_TIMER ) {
		if( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST ) return;
	} else if( DIFF_TICK(tick, md->ud.canact_tick) < 0 )
		return;

	lv = mercenary->checkskill(md, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit->skilluse_id(&md->bl, target_id, skill_id, skill_lv);
}

static void clif_parse_UseSkillToPos_mercenary(struct mercenary_data *md, struct map_session_data *sd, int64 tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
{
	int lv;
	nullpo_retv(sd);
	if( !md )
		return;
	if( skill->not_ok_mercenary(skill_id, md) )
		return;
	if( md->ud.skilltimer != INVALID_TIMER )
		return;
	if( DIFF_TICK(tick, md->ud.canact_tick) < 0 ) {
		clif->skill_fail(md->master, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
		return;
	}

	if( md->sc.data[SC_BASILICA] )
		return;
	lv = mercenary->checkskill(md, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit->skilluse_pos(&md->bl, x, y, skill_id, skill_lv);
}

static void clif_useSkillToIdReal(int fd, struct map_session_data *sd, int skill_id, int skill_lv, int target_id) __attribute__((nonnull (2)));
static void clif_useSkillToIdReal(int fd, struct map_session_data *sd, int skill_id, int skill_lv, int target_id)
{
	int64 tick = timer->gettick();

	pc->autocast_set_current(sd, skill_id);

	/**
	 * According to Skotlex' comment below, the client sometimes passes 0 for the skill level.
	 * Even though this seems to only affect guild skills, sd->autocast.skill_lv is used
	 * for the auto-cast data validation if skill_lv is 0.
	 *
	 **/
	skill->validate_autocast_data(sd, skill_id, (skill_lv == 0) ? sd->auto_cast_current.skill_lv : skill_lv);

	if (skill_lv < 1)
		skill_lv = 1; //No clue, I have seen the client do this with guild skills :/ [Skotlex]

	int tmp = skill->get_inf(skill_id);
	if (tmp & INF_GROUND_SKILL || !tmp)
		return; //Using a ground/passive skill on a target? WRONG.

	if (skill_id >= HM_SKILLBASE && skill_id < HM_SKILLBASE + MAX_HOMUNSKILL) {
		clif->pUseSkillToId_homun(sd->hd, sd, tick, skill_id, skill_lv, target_id);
		return;
	}

	if (skill_id >= MC_SKILLBASE && skill_id < MC_SKILLBASE + MAX_MERCSKILL) {
		clif->pUseSkillToId_mercenary(sd->md, sd, tick, skill_id, skill_lv, target_id);
		return;
	}

	// Whether skill fails or not is irrelevant, the char ain't idle. [Skotlex]
	pc->update_idle_time(sd, BCIDLE_USESKILLTOID);

	bool allow_self_skill = ((tmp & INF_SELF_SKILL) != 0 && (skill->get_nk(skill_id) & NK_NO_DAMAGE) != 0);
	allow_self_skill = (allow_self_skill && battle_config.skill_enabled_npc == SKILLENABLEDNPC_SELF);

	if ((sd->npc_id != 0 && sd->state.using_megaphone == 0 && !allow_self_skill && battle_config.skill_enabled_npc != SKILLENABLEDNPC_ALL)
	    || (sd->state.workinprogress & 1) != 0) {
#if PACKETVER >= 20110308
		clif->msgtable(sd, MSG_BUSY);
#else
		clif->messagecolor_self(fd, COLOR_WHITE, msg_fd(fd, 48));
#endif
		return;
	}

	if (pc_cant_act_except_npc(sd)
		&& skill_id != RK_REFRESH
		&& !(skill_id == SR_GENTLETOUCH_CURE && (sd->sc.opt1 == OPT1_STONE || sd->sc.opt1 == OPT1_FREEZE || sd->sc.opt1 == OPT1_STUN))
		&& (sd->state.storage_flag != STORAGE_FLAG_CLOSED && !(tmp&INF_SELF_SKILL)) // SELF skills can be used with the storage open, issue: 8027
	) {
		return;
	}

	if (pc_issit(sd))
		return;

	if (skill->not_ok(skill_id, sd))
		return;

	if (sd->bl.id != target_id && tmp & INF_SELF_SKILL)
		target_id = sd->bl.id; // never trust the client

	if (target_id < 0 && -target_id == sd->bl.id) // for disguises [Valaris]
		target_id = sd->bl.id;

	if (sd->ud.skilltimer != INVALID_TIMER) {
		if (skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST && sd->auto_cast_current.type == AUTOCAST_NONE)
			return;
	} else if (DIFF_TICK(tick, sd->ud.canact_tick) < 0) {
		if (sd->auto_cast_current.type == AUTOCAST_NONE) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
			return;
		}
	}

	if (sd->sc.option & OPTION_COSTUME)
		return;

	if (sd->sc.data[SC_BASILICA] && (skill_id != HP_BASILICA || sd->sc.data[SC_BASILICA]->val4 != sd->bl.id))
		return; // On basilica only caster can use Basilica again to stop it.

	if (sd->menuskill_id) {
		if (sd->menuskill_id == SA_TAMINGMONSTER) {
			clif_menuskill_clear(sd); //Cancel pet capture.
		} else if (sd->menuskill_id != SA_AUTOSPELL)
			return; //Can't use skills while a menu is open.
	}
	if (sd->auto_cast_current.type != AUTOCAST_NONE) {
		if (skill_lv != sd->auto_cast_current.skill_lv)
			skill_lv = sd->auto_cast_current.skill_lv;
		if (!(tmp&INF_SELF_SKILL))
			pc->delinvincibletimer(sd); // Target skills through items cancel invincibility. [Inkfish]
		unit->skilluse_id(&sd->bl, target_id, skill_id, skill_lv);
		return;
	}

	pc->autocast_clear(sd);

	if (skill_id >= GD_SKILLBASE && skill_id < GD_MAX) {
		if (sd->state.gmaster_flag)
			skill_lv = guild->checkskill(sd->guild, skill_id);
		else
			skill_lv = 0;
	} else {
		tmp = pc->checkskill(sd, skill_id);
		if (skill_lv > tmp)
			skill_lv = tmp;
	}

	pc->delinvincibletimer(sd);

	if (skill_lv)
		unit->skilluse_id(&sd->bl, target_id, skill_id, skill_lv);
}

static void clif_parse_UseSkillToId(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to use a targeted skill.
/// 0113 <skill lv>.W <skill id>.W <target id>.L (CZ_USE_SKILL)
/// 0438 <skill lv>.W <skill id>.W <target id>.L (CZ_USE_SKILL2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_UseSkillToId(int fd, struct map_session_data *sd)
{
	clif->useSkillToIdReal(fd,
		sd,
		RFIFOW(fd, packet_db[RFIFOW(fd, 0)].pos[1]),
		RFIFOW(fd, packet_db[RFIFOW(fd, 0)].pos[0]),
		RFIFOL(fd, packet_db[RFIFOW(fd, 0)].pos[2]));
}

static void clif_parse_startUseSkillToId(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_startUseSkillToId(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	const struct PACKET_CZ_START_USE_SKILL *p = RFIFOP(fd, 0);
	clif->useSkillToIdReal(fd, sd, p->skillId, p->skillLv, p->targetId);
#endif
}

static void clif_parse_stopUseSkillToId(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_stopUseSkillToId(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	const struct PACKET_CZ_STOP_USE_SKILL *p = RFIFOP(fd, 0);
	if (p->skillId != GC_ROLLINGCUTTER) {
		ShowWarning("Packet CZ_STOP_USE_SKILL usage for unknown skill: %d\n", p->skillId);
	}
#endif
}

/*==========================================
 * Client tells server he'd like to use AoE skill id 'skill_id' of level 'skill_lv' on 'x','y' location
 *------------------------------------------*/
static void clif_parse_UseSkillToPosSub(int fd, struct map_session_data *sd, uint16 skill_lv, uint16 skill_id, short x, short y, int skillmoreinfo)
{
	int64 tick = timer->gettick();

	nullpo_retv(sd);

	pc->autocast_set_current(sd, skill_id);

	/**
	 * When using clif_item_skill() to initiate the execution of ground skills,
	 * the client sometimes passes 0 for the skill level in packet 0x0af4.
	 * In that case sd->autocast.skill_lv is used for the auto-cast data validation,
	 * since clif_item_skill() is only used for auto-cast skills.
	 *
	 **/
	skill->validate_autocast_data(sd, skill_id, (skill_lv == 0) ? sd->auto_cast_current.skill_lv : skill_lv);

	if( !(skill->get_inf(skill_id)&INF_GROUND_SKILL) )
		return; //Using a target skill on the ground? WRONG.

	if( skill_id >= HM_SKILLBASE && skill_id < HM_SKILLBASE + MAX_HOMUNSKILL ) {
		clif->pUseSkillToPos_homun(sd->hd, sd, tick, skill_id, skill_lv, x, y, skillmoreinfo);
		return;
	}

	if( skill_id >= MC_SKILLBASE && skill_id < MC_SKILLBASE + MAX_MERCSKILL ) {
		clif->pUseSkillToPos_mercenary(sd->md, sd, tick, skill_id, skill_lv, x, y, skillmoreinfo);
		return;
	}

	if ((sd->npc_id != 0 && sd->state.using_megaphone == 0 && battle_config.skill_enabled_npc != SKILLENABLEDNPC_ALL)
	    || (sd->state.workinprogress & 1) != 0) {
#if PACKETVER >= 20110308
		clif->msgtable(sd, MSG_BUSY);
#else
		clif->messagecolor_self(fd, COLOR_WHITE, msg_fd(fd, 48));
#endif
		return;
	}

	//Whether skill fails or not is irrelevant, the char ain't idle. [Skotlex]
	pc->update_idle_time(sd, BCIDLE_USESKILLTOPOS);

	if( skill->not_ok(skill_id, sd) )
		return;
	if( skillmoreinfo != -1 ) {
		if( pc_issit(sd) ) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			return;
		}
		//You can't use Graffiti/TalkieBox AND have a vending open, so this is safe.
		safestrncpy(sd->message, RFIFOP(fd, skillmoreinfo), TALKBOX_MESSAGE_SIZE);
	}

	if (sd->ud.skilltimer != INVALID_TIMER && sd->auto_cast_current.type == AUTOCAST_NONE)
		return;

	if( DIFF_TICK(tick, sd->ud.canact_tick) < 0 ) {
		if (sd->auto_cast_current.type == AUTOCAST_NONE) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
			return;
		}
	}

	if( sd->sc.option&OPTION_COSTUME )
		return;

	if( sd->sc.data[SC_BASILICA] && (skill_id != HP_BASILICA || sd->sc.data[SC_BASILICA]->val4 != sd->bl.id) )
		return; // On basilica only caster can use Basilica again to stop it.

	if( sd->menuskill_id ) {
		if( sd->menuskill_id == SA_TAMINGMONSTER ) {
			clif_menuskill_clear(sd); //Cancel pet capture.
		} else if( sd->menuskill_id != SA_AUTOSPELL )
			return; //Can't use skills while a menu is open.
	}

	pc->delinvincibletimer(sd);

	if (sd->auto_cast_current.type != AUTOCAST_NONE) {
		if (skill_lv != sd->auto_cast_current.skill_lv)
			skill_lv = sd->auto_cast_current.skill_lv;
		unit->skilluse_pos(&sd->bl, x, y, skill_id, skill_lv);
	} else {
		int lv;
		pc->autocast_clear(sd);
		if( (lv = pc->checkskill(sd, skill_id)) > 0 ) {
			if( skill_lv > lv )
				skill_lv = lv;
			unit->skilluse_pos(&sd->bl, x, y, skill_id,skill_lv);
		}
	}
}

static void clif_parse_UseSkillToPos(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to use a ground skill.
/// 0116 <skill lv>.W <skill id>.W <x>.W <y>.W (CZ_USE_SKILL_TOGROUND)
/// 0366 <skill lv>.W <skill id>.W <x>.W <y>.W (CZ_USE_SKILL_TOGROUND2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_UseSkillToPos(int fd, struct map_session_data *sd)
{
	if (pc_cant_act_except_npc(sd))
		return;
	if (pc_issit(sd))
		return;

	clif->pUseSkillToPosSub(fd, sd,
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]), //skill lv
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[1]), //skill num
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[2]), //pos x
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[3]), //pos y
		-1 //Skill more info.
	);
}

static void clif_parse_UseSkillToPosMoreInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to use a ground skill with text.
/// 0190 <skill lv>.W <skill id>.W <x>.W <y>.W <contents>.80B (CZ_USE_SKILL_TOGROUND_WITHTALKBOX)
/// 0367 <skill lv>.W <skill id>.W <x>.W <y>.W <contents>.80B (CZ_USE_SKILL_TOGROUND_WITHTALKBOX2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_UseSkillToPosMoreInfo(int fd, struct map_session_data *sd)
{
	if (pc_cant_act_except_npc(sd))
		return;
	if (pc_issit(sd))
		return;

	clif->pUseSkillToPosSub(fd, sd,
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]), //Skill lv
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[1]), //Skill num
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[2]), //pos x
		RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[3]), //pos y
		packet_db[RFIFOW(fd,0)].pos[4] //skill more info
	);
}

static void clif_parse_UseSkillMap(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to map selection dialog (CZ_SELECT_WARPPOINT).
/// 011b <skill id>.W <map name>.16B
static void clif_parse_UseSkillMap(int fd, struct map_session_data *sd)
{
	uint16 skill_id = RFIFOW(fd,2);
	char map_name[MAP_NAME_LENGTH];

	mapindex->getmapname(RFIFOP(fd,4), map_name);
	sd->state.workinprogress = 0;

	if(skill_id != sd->menuskill_id)
		return;

	// It is possible to use teleport with the storage window open issue:8027
	if ((pc_cant_act_except_npc(sd) && sd->state.storage_flag == STORAGE_FLAG_CLOSED && skill_id != AL_TELEPORT)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0 && battle_config.skill_enabled_npc != SKILLENABLEDNPC_ALL)) {
		clif_menuskill_clear(sd);
		return;
	}

	pc->autocast_set_current(sd, skill_id);

	/**
	 * Since no skill level was passed use 0 to notify skill_validate_autocast_data() of this special case.
	 *
	 **/
	skill->validate_autocast_data(sd, skill_id, 0);

	pc->delinvincibletimer(sd);
	skill->castend_map(sd,skill_id,map_name);
	pc->autocast_clear(sd);
}

static void clif_parse_RequestMemo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to set a memo on current map (CZ_REMEMBER_WARPPOINT).
/// 011d
static void clif_parse_RequestMemo(int fd, struct map_session_data *sd)
{
	if (!pc_isdead(sd))
		pc->memo(sd,-1);
}

static void clif_parse_ProduceMix(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to pharmacy item selection dialog (CZ_REQMAKINGITEM).
/// 018e <name id>.W { <material id>.W }*3
static void clif_parse_ProduceMix(int fd, struct map_session_data *sd)
{
	const struct PACKET_CZ_REQMAKINGITEM *p = RFIFOP(fd, 0);

	switch (sd->menuskill_id) {
		case -1:
		case AM_PHARMACY:
		case RK_RUNEMASTERY:
		case GC_RESEARCHNEWPOISON:
			break;
		default:
			return;
	}
	if (pc_istrading_except_npc(sd) || pc_isdead(sd) || pc_isvending(sd)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}

	if (skill->can_produce_mix(sd, p->itemId, sd->menuskill_val, 1))
		skill->produce_mix(sd, 0, p->itemId, p->material[0], p->material[1], p->material[2], 1);
	clif_menuskill_clear(sd);
}

static void clif_parse_Cooking(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to mixing item selection dialog (CZ_REQ_MAKINGITEM).
/// 025b <mk type>.W <name id>.W
/// mk type:
///     1 = cooking
///     2 = arrow
///     3 = elemental
///     4 = GN_MIX_COOKING
///     5 = GN_MAKEBOMB
///     6 = GN_S_PHARMACY
static void clif_parse_Cooking(int fd, struct map_session_data *sd)
{
	const struct PACKET_CZ_REQ_MAKINGITEM *p = RFIFOP(fd, 0);
	int type = p->type;
	int nameid = p->itemId;
	int amount = sd->menuskill_val2 ? sd->menuskill_val2 : 1;
	if (type == 6 && sd->menuskill_id != GN_MIX_COOKING && sd->menuskill_id != GN_S_PHARMACY)
		return;

	if (pc_istrading_except_npc(sd) || pc_isdead(sd) || pc_isvending(sd)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}
	if (skill->can_produce_mix(sd, nameid, sd->menuskill_val, amount))
		skill->produce_mix(sd, (type > 1 ? sd->menuskill_id : 0), nameid, 0, 0, 0, amount);
	clif_menuskill_clear(sd);
}

static void clif_parse_RepairItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to repair weapon item selection dialog (CZ_REQ_ITEMREPAIR).
/// 01fd <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W
static void clif_parse_RepairItem(int fd, struct map_session_data *sd)
{
	const struct PACKET_CZ_REQ_ITEMREPAIR *p = RFIFOP(fd, 0);

	if (sd->menuskill_id != BS_REPAIRWEAPON)
		return;
	if (pc_istrading_except_npc(sd) || pc_isdead(sd) || pc_isvending(sd)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}
	skill->repairweapon(sd, p->index);
	clif_menuskill_clear(sd);
}

static void clif_parse_WeaponRefine(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to refine weapon item selection dialog (CZ_REQ_WEAPONREFINE).
/// 0222 <index>.L
static void clif_parse_WeaponRefine(int fd, struct map_session_data *sd)
{
	sd->state.prerefining = 0;

	if (sd->menuskill_id != WS_WEAPONREFINE) //Packet exploit?
		return;
	if (pc_istrading_except_npc(sd) || pc_isdead(sd) || pc_isvending(sd)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}
	int idx = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	skill->weaponrefine(sd, idx-2);
	clif_menuskill_clear(sd);
}

static void clif_parse_NpcSelectMenu(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to script menu dialog (CZ_CHOOSE_MENU).
/// 00b8 <npc id>.L <choice>.B
/// choice:
///     1~254 = menu item
///     255   = cancel
/// NOTE: If there were more than 254 items in the list, choice
///     overflows to choice%256.
static void clif_parse_NpcSelectMenu(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int npc_id = RFIFOL(fd,2);
	uint8 select = RFIFOB(fd,6);

	if( (select > sd->npc_menu && select != MAX_MENU_OPTIONS) || select == 0 ) {
#ifdef SECURE_NPCTIMEOUT
		if( sd->npc_idle_timer != INVALID_TIMER ) {
#endif
			struct npc_data *nd = map->id2nd(npc_id);
			ShowWarning("Invalid menu selection on npc %d:'%s' - got %d, valid range is [%d..%d] (player AID:%d, CID:%d, name:'%s')!\n", npc_id, (nd)?nd->name:"invalid npc id", select, 1, sd->npc_menu, sd->bl.id, sd->status.char_id, sd->status.name);
			clif->GM_kick(NULL,sd);
#ifdef SECURE_NPCTIMEOUT
		}
#endif
		return;
	}

	sd->npc_menu = select;
	npc->scriptcont(sd,npc_id, false);
}

static void clif_parse_NpcNextClicked(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// NPC dialog 'next' click (CZ_REQ_NEXT_SCRIPT).
/// 00b9 <npc id>.L
static void clif_parse_NpcNextClicked(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	npc->scriptcont(sd,RFIFOL(fd,2), false);
}

static void clif_parse_NpcAmountInput(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// NPC numeric input dialog value (CZ_INPUT_EDITDLG).
/// 0143 <npc id>.L <value>.L
static void clif_parse_NpcAmountInput(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int npcid = RFIFOL(fd,2);
	int amount = RFIFOL(fd,6);

	if (amount < sd->npc_amount_min) {
		sd->npc_amount = sd->npc_amount_min;
		sd->npc_input_capped_range = -1;
	}
	else if (amount > sd->npc_amount_max) {
		sd->npc_amount = sd->npc_amount_max;
		sd->npc_input_capped_range = 1;
	}
	else {
		sd->npc_amount = amount;
		sd->npc_input_capped_range = 0;
	}

	npc->scriptcont(sd, npcid, false);
}

static void clif_parse_NpcStringInput(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// NPC text input dialog value (CZ_INPUT_EDITDLGSTR).
/// 01d5 <packet len>.W <npc id>.L <string>.?B
static void clif_parse_NpcStringInput(int fd, struct map_session_data *sd)
{
	if ((sd->state.trading != 0 || pc_isvending(sd) || pc_isdead(sd)) && sd->state.using_megaphone == 0)
		return;

	int len = RFIFOW(fd, 2);
// [4144] can't confirm exact client version. At least >= correct for 20150513
#if PACKETVER >= 20151029
	int message_len = len - 7;
#else
	int message_len = len - 8;
#endif
	int npcid;
	const char *message;

	if (len < 9)
		return;

	npcid = (sd->state.using_megaphone == 0) ? RFIFOL(fd, 4) : sd->npc_id;
	message = RFIFOP(fd, 8);

	safestrncpy(sd->npc_str, message, min(message_len,CHATBOX_SIZE));
	npc->scriptcont(sd, npcid, false);
}

static void clif_parse_NpcCloseClicked(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// NPC dialog 'close' click (CZ_CLOSE_DIALOG).
/// 0146 <npc id>.L
static void clif_parse_NpcCloseClicked(int fd, struct map_session_data *sd)
{
	if (!sd->npc_id) //Avoid parsing anything when the script was done with. [Skotlex]
		return;
	if (sd->state.trading || pc_isvending(sd))
		return;
	sd->state.dialog = 0;
	npc->scriptcont(sd, RFIFOL(fd,2), true);
}

static void clif_parse_ItemIdentify(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to identify item selection dialog (CZ_REQ_ITEMIDENTIFY).
/// 0178 <index>.W
/// index:
///     -1 = cancel
static void clif_parse_ItemIdentify(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	short idx = RFIFOW(fd,2);

	if (sd->menuskill_id != MC_IDENTIFY)
		return;
	if( idx == -1 ) {// cancel pressed
		sd->state.workinprogress = 0;
		clif->item_identified(sd,idx-2,1);
		clif_menuskill_clear(sd);
		return;
	}
	skill->identify(sd,idx-2);
	clif_menuskill_clear(sd);
}

/// Identifying item with right-click (CZ_REQ_ONECLICK_ITEMIDENTIFY).
/// 0A35 <index>.W
static void clif_parse_OneClick_ItemIdentify(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int cmd = RFIFOW(fd,0);
	short idx = RFIFOW(fd, packet_db[cmd].pos[0]) - 2;
	int n;

	if (idx < 0 || idx >= sd->status.inventorySize || sd->inventory_data[idx] == NULL || sd->status.inventory[idx].nameid <= 0)
		return;

	if ((n = pc->have_magnifier(sd)) != INDEX_NOT_FOUND &&
		pc->delitem(sd, n, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME) == 0)
		skill->identify(sd, idx);
}

static void clif_parse_SelectArrow(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to arrow crafting item selection dialog (CZ_REQ_MAKINGARROW).
/// 01ae <name id>.W
static void clif_parse_SelectArrow(int fd, struct map_session_data *sd)
{
	int itemId;
	if (pc_istrading_except_npc(sd) || pc_isdead(sd) || pc_isvending(sd)
	    || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif->skill_fail(sd, sd->ud.skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	itemId = RFIFOL(fd, 2);
#else
	itemId = RFIFOW(fd, 2);
#endif
	switch (sd->menuskill_id) {
		case AC_MAKINGARROW:
			skill->arrow_create(sd, itemId);
			break;
		case SA_CREATECON:
			skill->produce_mix(sd, SA_CREATECON, itemId, 0, 0, 0, 1);
			break;
		case WL_READING_SB:
			skill->spellbook(sd, itemId);
			break;
		case GC_POISONINGWEAPON:
			skill->poisoningweapon(sd, itemId);
			break;
		case NC_MAGICDECOY:
			skill->magicdecoy(sd, itemId);
			break;
	}

	clif_menuskill_clear(sd);
}

static void clif_parse_AutoSpell(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to SA_AUTOSPELL skill selection dialog (CZ_SELECTAUTOSPELL).
/// 01ce <skill id>.L
static void clif_parse_AutoSpell(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	uint16 skill_id = RFIFOL(fd,2);

	sd->state.workinprogress = 0;

	if( sd->menuskill_id != SA_AUTOSPELL )
		return;

	if( !skill_id )
		return;

	skill->autospell(sd, skill_id);
	clif_menuskill_clear(sd);
}

static void clif_parse_UseCard(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to display item carding/composition list (CZ_REQ_ITEMCOMPOSITION_LIST).
/// 017a <card index>.W
static void clif_parse_UseCard(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	clif->use_card(sd,RFIFOW(fd,2)-2);
}

static void clif_parse_InsertCard(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to carding/composing item selection dialog (CZ_REQ_ITEMCOMPOSITION).
/// 017c <card index>.W <equip index>.W
static void clif_parse_InsertCard(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	pc->insert_card(sd,RFIFOW(fd,2)-2,RFIFOW(fd,4)-2);
}

static void clif_parse_SolveCharName(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request of character's name by char ID.
/// 0193 <char id>.L (CZ_REQNAME_BYGID)
/// 0369 <char id>.L (CZ_REQNAME_BYGID2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_SolveCharName(int fd, struct map_session_data *sd)
{
	int charid;

	charid = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	map->reqnickdb(sd, charid);
}

static void clif_parse_ResetChar(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /resetskill /resetstate (CZ_RESET).
/// Request to reset stats or skills.
/// 0197 <type>.W
/// type:
///     0 = state
///     1 = skill
static void clif_parse_ResetChar(int fd, struct map_session_data *sd)
{
	if (pc_istrading(sd) || pc_isvending(sd))
		return;

	char cmd[15];

	if( RFIFOW(fd,2) )
		sprintf(cmd,"%cskreset",atcommand->at_symbol);
	else
		sprintf(cmd,"%cstreset",atcommand->at_symbol);

	atcommand->exec(fd, sd, cmd, true);
}

static void clif_parse_LocalBroadcast(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /lb /nlb (CZ_LOCALBROADCAST).
/// Request to broadcast a message on current map.
/// 019c <packet len>.W <text>.?B
static void clif_parse_LocalBroadcast(int fd, struct map_session_data *sd)
{
	const char commandname[] = "lkami";
	char command[sizeof commandname + 2 + CHAT_SIZE_MAX] = ""; // '@' + command + ' ' + message + NUL
	int len = (int)RFIFOW(fd,2) - 4;

	if (len < 0)
		return;

	sprintf(command, "%c%s ", atcommand->at_symbol, commandname);

	// as the length varies depending on the command used, truncate unreasonably long strings
	if (len >= (int)(sizeof command - strlen(command)))
		len = (int)(sizeof command - strlen(command)) - 1;

	strncat(command, RFIFOP(fd,4), len);
	atcommand->exec(fd, sd, command, true);
}

static void clif_parse_MoveToKafra(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from inventory to storage.
/// 00f3 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_BODY_TO_STORE)
/// 0364 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_BODY_TO_STORE2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_MoveToKafra(int fd, struct map_session_data *sd)
{
	int item_index, item_amount;

	if (pc_istrading(sd) || sd->state.prevend)
		return;

	item_index = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2;
	item_amount = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	if (item_index < 0 || item_index >= sd->status.inventorySize || item_amount < 1)
		return;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
		storage->add(sd, item_index, item_amount);
	else if (sd->state.storage_flag == STORAGE_FLAG_GUILD)
		gstorage->add(sd, item_index, item_amount);
}

static void clif_parse_MoveFromKafra(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from storage to inventory.
/// 00f5 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_STORE_TO_BODY)
/// 0365 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_STORE_TO_BODY2)
/// There are various variants of this packet, some of them have padding between fields.
static void clif_parse_MoveFromKafra(int fd, struct map_session_data *sd)
{
	if (pc_istrading(sd) || sd->state.prevend)
		return;

	int item_index, item_amount;

	item_index = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-1;
	item_amount = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[1]);

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
		storage->get(sd, item_index, item_amount);
	else if(sd->state.storage_flag == STORAGE_FLAG_GUILD)
		gstorage->get(sd, item_index, item_amount);
}

static void clif_parse_MoveToKafraFromCart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from cart to storage (CZ_MOVE_ITEM_FROM_CART_TO_STORE).
/// 0129 <index>.W <amount>.L
static void clif_parse_MoveToKafraFromCart(int fd, struct map_session_data *sd)
{
	if (pc_istrading(sd) || sd->state.prevend)
		return;
	if (!pc_iscarton(sd))
		return;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
		storage->addfromcart(sd, RFIFOW(fd,2) - 2, RFIFOL(fd,4));
	else if (sd->state.storage_flag == STORAGE_FLAG_GUILD)
		gstorage->addfromcart(sd, RFIFOW(fd,2) - 2, RFIFOL(fd,4));
}

static void clif_parse_MoveFromKafraToCart(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move an item from storage to cart (CZ_MOVE_ITEM_FROM_STORE_TO_CART).
/// 0128 <index>.W <amount>.L
static void clif_parse_MoveFromKafraToCart(int fd, struct map_session_data *sd)
{
	if (pc_istrading(sd) || sd->state.prevend)
		return;
	if (!pc_iscarton(sd))
		return;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
		storage->gettocart(sd, RFIFOW(fd,2)-1, RFIFOL(fd,4));
	else if (sd->state.storage_flag == STORAGE_FLAG_GUILD)
		gstorage->gettocart(sd, RFIFOW(fd,2)-1, RFIFOL(fd,4));
}

static void clif_parse_CloseKafra(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to close storage (CZ_CLOSE_STORE).
/// 00f7
static void clif_parse_CloseKafra(int fd, struct map_session_data *sd)
{
	if( sd->state.storage_flag == STORAGE_FLAG_NORMAL )
		storage->close(sd);
	else if( sd->state.storage_flag == STORAGE_FLAG_GUILD )
		gstorage->close(sd);
}

#if 0 // Unused function
/// Displays kafra storage password dialog (ZC_REQ_STORE_PASSWORD).
/// 023a <info>.W
/// info:
///     0 = password has not been set yet
///     1 = storage is password-protected
///     8 = too many wrong passwords
///     ? = ignored
/// NOTE: This packet is only available on certain non-kRO clients.
static void clif_storagepassword(struct map_session_data *sd, short info)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x23a));
	WFIFOW(fd,0) = 0x23a;
	WFIFOW(fd,2) = info;
	WFIFOSET(fd,packet_len(0x23a));
}
#endif // 0

static void clif_parse_StoragePassword(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to the kafra storage password dialog (CZ_ACK_STORE_PASSWORD).
/// 023b <type>.W <password>.16B <new password>.16B
/// type:
///     2 = change password
///     3 = check password
/// NOTE: This packet is only available on certain non-kRO clients.
static void clif_parse_StoragePassword(int fd, struct map_session_data *sd)
{
	//TODO
}

#if 0 // Unused function
/// Result of kafra storage password validation (ZC_RESULT_STORE_PASSWORD).
/// 023c <result>.W <error count>.W
/// result:
///     4 = password change success
///     5 = password change failure
///     6 = password check success
///     7 = password check failure
///     8 = too many wrong passwords
///     ? = ignored
/// NOTE: This packet is only available on certain non-kRO clients.
static void clif_storagepassword_result(struct map_session_data *sd, short result, short error_count)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x23c));
	WFIFOW(fd,0) = 0x23c;
	WFIFOW(fd,2) = result;
	WFIFOW(fd,4) = error_count;
	WFIFOSET(fd,packet_len(0x23c));
}
#endif // 0

static void clif_parse_CreateParty(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Party creation request
/// 00f9 <party name>.24B (CZ_MAKE_GROUP)
/// 01e8 <party name>.24B <item pickup rule>.B <item share rule>.B (CZ_MAKE_GROUP2)
static void clif_parse_CreateParty(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOP(fd,2), NAME_LENGTH);

	if( map->list[sd->bl.m].flag.partylock ) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}
	if (battle_config.basic_skill_check && !pc->check_basicskill(sd, 7)) {
		clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 4, 0);
		return;
	}

	party->create(sd,name,0,0);
}

static void clif_parse_CreateParty2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_CreateParty2(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	char name[NAME_LENGTH];
	int item1 = RFIFOB(fd,26);
	int item2 = RFIFOB(fd,27);

	safestrncpy(name, RFIFOP(fd,2), NAME_LENGTH);

	if( map->list[sd->bl.m].flag.partylock ) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}
	if (battle_config.basic_skill_check && !pc->check_basicskill(sd, 7)) {
		clif->skill_fail(sd, 1, USESKILL_FAIL_LEVEL, 4, 0);
		return;
	}

	party->create(sd,name,item1,item2);
}

static void clif_parse_PartyInvite(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Party invitation request
/// 00fc <account id>.L (CZ_REQ_JOIN_GROUP)
/// 02c4 <char name>.24B (CZ_PARTY_JOIN_REQ)
static void clif_parse_PartyInvite(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd;

	if(map->list[sd->bl.m].flag.partylock) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}

	t_sd = map->id2sd(RFIFOL(fd,2));

	if(t_sd && t_sd->state.noask) {// @noask [LuzZza]
		clif->noask_sub(sd, t_sd, 1);
		return;
	}

	party->invite(sd, t_sd);
}

static void clif_parse_PartyInvite2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_PartyInvite2(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd;
	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOP(fd,2), NAME_LENGTH);

	if(map->list[sd->bl.m].flag.partylock) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}

	t_sd = map->nick2sd(name, true);

	if(t_sd && t_sd->state.noask) { // @noask [LuzZza]
		clif->noask_sub(sd, t_sd, 1);
		return;
	}

	party->invite(sd, t_sd);
}

static void clif_parse_ReplyPartyInvite(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Party invitation reply
/// 00ff <party id>.L <flag>.L (CZ_JOIN_GROUP)
/// 02c7 <party id>.L <flag>.B (CZ_PARTY_JOIN_REQ_ACK)
/// flag:
///     0 = reject
///     1 = accept
static void clif_parse_ReplyPartyInvite(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd)) {
		party->reply_invite(sd, RFIFOL(fd, 2), 0);
		return;
	}

	party->reply_invite(sd, RFIFOL(fd, 2), RFIFOL(fd, 6));
}

static void clif_parse_ReplyPartyInvite2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_ReplyPartyInvite2(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd)) {
		party->reply_invite(sd, RFIFOL(fd, 2), 0);
		return;
	}

	party->reply_invite(sd, RFIFOL(fd, 2), RFIFOB(fd, 6));
}

static void clif_parse_LeaveParty(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to leave party (CZ_REQ_LEAVE_GROUP).
/// 0100
static void clif_parse_LeaveParty(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.partylock) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}
	party->leave(sd);
}

static void clif_parse_RemovePartyMember(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to expel a party member (CZ_REQ_EXPEL_GROUP_MEMBER).
/// 0103 <account id>.L <char name>.24B
static void clif_parse_RemovePartyMember(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.partylock) {
		// Party locked.
		clif->message(fd, msg_fd(fd,227)); // Party modification is disabled in this map.
		return;
	}
	party->removemember(sd, RFIFOL(fd, 2), RFIFOP(fd, 6));
}

static void clif_parse_PartyChangeOption(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change party options.
/// 0102 <exp share rule>.L (CZ_CHANGE_GROUPEXPOPTION)
/// 07d7 <exp share rule>.L <item pickup rule>.B <item share rule>.B (CZ_GROUPINFO_CHANGE_V2)
static void clif_parse_PartyChangeOption(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	struct party_data *p;
	int i;

	if( !sd->status.party_id )
		return;

	p = party->search(sd->status.party_id);
	if( p == NULL )
		return;

	ARR_FIND( 0, MAX_PARTY, i, p->data[i].sd == sd );
	if( i == MAX_PARTY )
		return; //Shouldn't happen

	if( !p->party.member[i].leader )
		return;

#if PACKETVER < 20090603
	//Client can't change the item-field
	party->changeoption(sd, RFIFOL(fd,2), p->party.item);
#else
	party->changeoption(sd, RFIFOL(fd,2), ((RFIFOB(fd,6)?1:0)|(RFIFOB(fd,7)?2:0)));
#endif
}

/**
 * Validates and processes party messages (CZ_REQUEST_CHAT_PARTY).
 *
 * @code
 * 0108 <packet len>.W <text>.?B (<name> : <message>) 00
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 */
static void clif_parse_PartyMessage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_PartyMessage(int fd, struct map_session_data *sd)
{
	const struct packet_chat_message *packet = RP2PTR(fd);
	char message[CHAT_SIZE_MAX + NAME_LENGTH + 3 + 1];

	if (clif->process_chat_message(sd, packet, message, sizeof message) == NULL)
		return;

	party->send_message(sd, message);
}

static void clif_parse_PartyChangeLeader(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Changes Party Leader (CZ_CHANGE_GROUP_MASTER).
/// 07da <account id>.L
static void clif_parse_PartyChangeLeader(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	party->changeleader(sd, map->id2sd(RFIFOL(fd,2)));
}

static void clif_parse_PartyBookingRegisterReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Party Booking in KRO [Spiria]
///

/// Request to register a party booking advertisement (CZ_PARTY_BOOKING_REQ_REGISTER).
/// 0802 <level>.W <map id>.W { <job>.W }*6
static void clif_parse_PartyBookingRegisterReq(int fd, struct map_session_data *sd)
{
#ifndef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	short level = RFIFOW(fd, 2);
	short mapid = RFIFOW(fd, 4);
	short job[MAX_PARTY_BOOKING_JOBS];

	for (int i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		job[i] = RFIFOB(fd, 6 + i * 2);

	party->booking_register(sd, level, mapid, job);
#else
	return;
#endif
}

/// Result of request to register a party booking advertisement (ZC_PARTY_BOOKING_ACK_REGISTER).
/// 0803 <result>.W
/// result:
///     0 = success
///     1 = failure
///     2 = already registered
static void clif_PartyBookingRegisterAck(struct map_session_data *sd, int flag)
{
#ifndef PARTY_RECRUIT
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x803));
	WFIFOW(fd,0) = 0x803;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x803));
#else
	return;
#endif
}

static void clif_parse_PartyBookingSearchReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to search for party booking advertisement (CZ_PARTY_BOOKING_REQ_SEARCH).
/// 0804 <level>.W <map id>.W <job>.W <last index>.L <result count>.W
static void clif_parse_PartyBookingSearchReq(int fd, struct map_session_data *sd)
{
#ifndef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	short level = RFIFOW(fd,2);
	short mapid = RFIFOW(fd,4);
	short job = RFIFOW(fd,6);
	unsigned long lastindex = RFIFOL(fd,8);
	short resultcount = RFIFOW(fd,12);

	party->booking_search(sd, level, mapid, job, lastindex, resultcount);
#else
	return;
#endif
}

/// Party booking search results (ZC_PARTY_BOOKING_ACK_SEARCH).
/// 0805 <packet len>.W <more results>.B { <index>.L <char name>.24B <expire time>.L <level>.W <map id>.W { <job>.W }*6 }*
/// more results:
///     0 = no
///     1 = yes
static void clif_PartyBookingSearchAck(int fd, struct party_booking_ad_info **results, int count, bool more_result)
{
#ifndef PARTY_RECRUIT
	int size = sizeof(struct party_booking_ad_info); // structure size (48)
	struct party_booking_ad_info *pb_ad;
	nullpo_retv(results);
	WFIFOHEAD(fd, size * count + 5);
	WFIFOW(fd, 0) = 0x805;
	WFIFOW(fd, 2) = size * count + 5;
	WFIFOB(fd, 4) = more_result;

	for (int i = 0; i < count; i++) {
		pb_ad = results[i];
		WFIFOL(fd, i * size + 5) = pb_ad->index;
		memcpy(WFIFOP(fd, i * size + 9), pb_ad->charname, NAME_LENGTH);
		WFIFOL(fd, i * size + 33) = pb_ad->expiretime;
		WFIFOW(fd, i * size + 37) = pb_ad->p_detail.level;
		WFIFOW(fd, i * size + 39) = pb_ad->p_detail.mapid;
		for (int j = 0; j < MAX_PARTY_BOOKING_JOBS; j++)
			WFIFOW(fd, i * size + 41 + j * 2) = pb_ad->p_detail.job[j];
	}
	WFIFOSET(fd, WFIFOW(fd, 2));
#else
	return;
#endif
}

static void clif_parse_PartyBookingDeleteReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete own party booking advertisement (CZ_PARTY_BOOKING_REQ_DELETE).
/// 0806
static void clif_parse_PartyBookingDeleteReq(int fd, struct map_session_data *sd)
{
#ifndef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if (party->booking_delete(sd))
		clif->PartyBookingDeleteAck(sd, 0);
#else
	return;
#endif
}

/// Result of request to delete own party booking advertisement (ZC_PARTY_BOOKING_ACK_DELETE).
/// 0807 <result>.W
/// result:
///     0 = success
///     1 = success (auto-removed expired ad)
///     2 = failure
///     3 = nothing registered
static void clif_PartyBookingDeleteAck(struct map_session_data *sd, int flag)
{
#ifndef PARTY_RECRUIT
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x807));
	WFIFOW(fd,0) = 0x807;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x807));
#else
	return;
#endif
}

static void clif_parse_PartyBookingUpdateReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update party booking advertisement (CZ_PARTY_BOOKING_REQ_UPDATE).
/// 0808 { <job>.W }*6
static void clif_parse_PartyBookingUpdateReq(int fd, struct map_session_data *sd)
{
#ifndef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	short job[MAX_PARTY_BOOKING_JOBS];

	for (int i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		job[i] = RFIFOW(fd, 2 + i * 2);

	party->booking_update(sd, job);
#else
	return;
#endif
}

/// Notification about new party booking advertisement (ZC_PARTY_BOOKING_NOTIFY_INSERT).
/// 0809 <index>.L <char name>.24B <expire time>.L <level>.W <map id>.W { <job>.W }*6
static void clif_PartyBookingInsertNotify(struct map_session_data *sd, struct party_booking_ad_info *pb_ad)
{
	nullpo_retv(sd);
	nullpo_retv(pb_ad);

#ifndef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	uint8 buf[38 + MAX_PARTY_BOOKING_JOBS * 2];

	WBUFW(buf, 0) = 0x809;
	WBUFL(buf, 2) = pb_ad->index;
	memcpy(WBUFP(buf, 6), pb_ad->charname, NAME_LENGTH);
	WBUFL(buf, 30) = pb_ad->expiretime;
	WBUFW(buf, 34) = pb_ad->p_detail.level;
	WBUFW(buf, 36) = pb_ad->p_detail.mapid;

	for (int i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		WBUFW(buf, 38 + i * 2) = pb_ad->p_detail.job[i];

	clif->send(buf, packet_len(0x809), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

/// Notification about updated party booking advertisement (ZC_PARTY_BOOKING_NOTIFY_UPDATE).
/// 080a <index>.L { <job>.W }*6
static void clif_PartyBookingUpdateNotify(struct map_session_data *sd, struct party_booking_ad_info *pb_ad)
{
	nullpo_retv(sd);
	nullpo_retv(pb_ad);

#ifndef PARTY_RECRUIT
	uint8 buf[6 + MAX_PARTY_BOOKING_JOBS * 2];

	WBUFW(buf, 0) = 0x80a;
	WBUFL(buf, 2) = pb_ad->index;

	for (int i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		WBUFW(buf, 6 + i * 2) = pb_ad->p_detail.job[i];
	clif->send(buf, packet_len(0x80a), &sd->bl, ALL_CLIENT); // Now UPDATE all client.
#else
	return;
#endif
}

/// Notification about deleted party booking advertisement (ZC_PARTY_BOOKING_NOTIFY_DELETE).
/// 080b <index>.L
static void clif_PartyBookingDeleteNotify(struct map_session_data *sd, int index)
{
#ifndef PARTY_RECRUIT
	uint8 buf[6];

	nullpo_retv(sd);
	WBUFW(buf,0) = 0x80b;
	WBUFL(buf,2) = index;

	clif->send(buf, packet_len(0x80b), &sd->bl, ALL_CLIENT); // Now UPDATE all client.
#else
	return;
#endif
}

static void clif_parse_PartyRecruitRegisterReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Modified version of Party Booking System for 2012-04-10 or 2012-04-18 (RagexeRE).
/// Code written by mkbu95, Spiria, Yommy and Ind

/// Request to register a party booking advertisement (CZ_PARTY_RECRUIT_REQ_REGISTER).
/// 08e5 <level>.W <notice>.37B
static void clif_parse_PartyRecruitRegisterReq(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	short level = RFIFOW(fd, 2);
	const char *notice = RFIFOP(fd, 4);

	party->recruit_register(sd, level, notice);
#else
	return;
#endif
}

/// Party booking search results (ZC_PARTY_RECRUIT_ACK_SEARCH).
/// 08e8 <packet len>.W <more results>.B { <index>.L <char name>.24B <expire time>.L <level>.W <notice>.37B }*
/// more results:
///     0 = no
///     1 = yes
static void clif_PartyRecruitSearchAck(int fd, struct party_booking_ad_info **results, int count, bool more_result)
{
#ifdef PARTY_RECRUIT
	int i;
	int size = sizeof(struct party_booking_ad_info);
	struct party_booking_ad_info *pb_ad;

	nullpo_retv(results);
	WFIFOHEAD(fd, (size * count) + 5);
	WFIFOW(fd, 0) = 0x8e8;
	WFIFOW(fd, 2) = (size * count) + 5;
	WFIFOB(fd, 4) = more_result;

	for (i = 0; i < count; ++i) {
		pb_ad = results[i];

		WFIFOL(fd, (i * size) + 5) = pb_ad->index;
		WFIFOL(fd, (i * size) + 9) = pb_ad->expiretime;
		memcpy(WFIFOP(fd, (i * size) + 13), pb_ad->charname, NAME_LENGTH);
		WFIFOW(fd, (i * size) + 13 + NAME_LENGTH) = pb_ad->p_detail.level;
		memcpy(WFIFOP(fd, (i * size) + 13 + NAME_LENGTH + 2), pb_ad->p_detail.notice, PB_NOTICE_LENGTH);
	}

	WFIFOSET(fd,WFIFOW(fd,2));
#else
	return;
#endif
}

/// Result of request to register a party booking advertisement (ZC_PARTY_RECRUIT_ACK_REGISTER).
/// 08e6 <result>.W
/// result:
///     0 = success
///     1 = failure
///     2 = already registered
static void clif_PartyRecruitRegisterAck(struct map_session_data *sd, int flag)
{
#ifdef PARTY_RECRUIT
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x8e6));
	WFIFOW(fd, 0) = 0x8e6;
	WFIFOW(fd, 2) = flag;
	WFIFOSET(fd, packet_len(0x8e6));
#else
	return;
#endif
}

static void clif_parse_PartyRecruitSearchReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to search for party booking advertisement (CZ_PARTY_RECRUIT_REQ_SEARCH).
/// 08e7 <level>.W <map id>.W <last index>.L <result count>.W
static void clif_parse_PartyRecruitSearchReq(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	short level = RFIFOW(fd, 2);
	short mapid = RFIFOW(fd, 4);
	unsigned long lastindex = RFIFOL(fd, 6);
	short resultcount = RFIFOW(fd, 10);

	party->recruit_search(sd, level, mapid, lastindex, resultcount);
#else
	return;
#endif
}

static void clif_parse_PartyRecruitDeleteReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete own party booking advertisement (CZ_PARTY_RECRUIT_REQ_DELETE).
/// 08e9
static void clif_parse_PartyRecruitDeleteReq(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if (party->booking_delete(sd))
		clif->PartyRecruitDeleteAck(sd, 0);
#else
	return;
#endif
}

/// Result of request to delete own party booking advertisement (ZC_PARTY_RECRUIT_ACK_DELETE).
/// 08ea <result>.W
/// result:
///     0 = success
///     1 = success (auto-removed expired ad)
///     2 = failure
///     3 = nothing registered
static void clif_PartyRecruitDeleteAck(struct map_session_data *sd, int flag)
{
#ifdef PARTY_RECRUIT
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x8ea));
	WFIFOW(fd, 0) = 0x8ea;
	WFIFOW(fd, 2) = flag;
	WFIFOSET(fd, packet_len(0x8ea));
#else
	return;
#endif
}

static void clif_parse_PartyRecruitUpdateReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update party booking advertisement (CZ_PARTY_RECRUIT_REQ_UPDATE).
/// 08eb <notice>.37B
static void clif_parse_PartyRecruitUpdateReq(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	const char *notice = RFIFOP(fd, 2);

	party->recruit_update(sd, notice);
#else
	return;
#endif
}

/// Notification about new party booking advertisement (ZC_PARTY_RECRUIT_NOTIFY_INSERT).
/// 08ec <index>.L <expire time>.L <char name>.24B <level>.W <notice>.37B
static void clif_PartyRecruitInsertNotify(struct map_session_data *sd, struct party_booking_ad_info *pb_ad)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+6+24+4+37+1];

	nullpo_retv(sd);
	if (pb_ad == NULL)
		return;

	WBUFW(buf, 0) = 0x8ec;
	WBUFL(buf, 2) = pb_ad->index;
	WBUFL(buf, 6) = pb_ad->expiretime;
	memcpy(WBUFP(buf, 10), pb_ad->charname, NAME_LENGTH);
	WBUFW(buf,34) = pb_ad->p_detail.level;
	memcpy(WBUFP(buf, 36), pb_ad->p_detail.notice, PB_NOTICE_LENGTH);
	clif->send(buf, packet_len(0x8ec), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

/// Notification about updated party booking advertisement (ZC_PARTY_RECRUIT_NOTIFY_UPDATE).
/// 08ed <index>.L <notice>.37B
static void clif_PartyRecruitUpdateNotify(struct map_session_data *sd, struct party_booking_ad_info *pb_ad)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+37+1];

	nullpo_retv(sd);
	nullpo_retv(pb_ad);
	WBUFW(buf, 0) = 0x8ed;
	WBUFL(buf, 2) = pb_ad->index;
	memcpy(WBUFP(buf, 6), pb_ad->p_detail.notice, PB_NOTICE_LENGTH);

	clif->send(buf, packet_len(0x8ed), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

/// Notification about deleted party booking advertisement (ZC_PARTY_RECRUIT_NOTIFY_DELETE).
/// 08ee <index>.L
static void clif_PartyRecruitDeleteNotify(struct map_session_data *sd, int index)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+1];

	nullpo_retv(sd);
	WBUFW(buf, 0) = 0x8ee;
	WBUFL(buf, 2) = index;

	clif->send(buf, packet_len(0x8ee), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

static void clif_parse_PartyBookingAddFilteringList(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add to filtering list (PARTY_RECRUIT_ADD_FILTERLINGLIST).
/// 08ef <index>.L
static void clif_parse_PartyBookingAddFilteringList(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	int index = RFIFOL(fd, 2);

	clif->PartyBookingAddFilteringList(index, sd);
#else
	return;
#endif
}

static void clif_parse_PartyBookingSubFilteringList(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to remove from filtering list (PARTY_RECRUIT_SUB_FILTERLINGLIST).
/// 08f0 <GID>.L
static void clif_parse_PartyBookingSubFilteringList(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	int gid = RFIFOL(fd, 2);

	clif->PartyBookingSubFilteringList(gid, sd);
#else
	return;
#endif
}

static void clif_parse_PartyBookingReqVolunteer(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to recruit volunteer (PARTY_RECRUIT_REQ_VOLUNTEER).
/// 08f1 <index>.L
static void clif_parse_PartyBookingReqVolunteer(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	int index = RFIFOL(fd, 2);

	clif->PartyBookingVolunteerInfo(index, sd);
#else
	return;
#endif
}

/// Request volunteer information (PARTY_RECRUIT_VOLUNTEER_INFO).
/// 08f2 <AID>.L <job>.L <level>.W <char name>.24B
static void clif_PartyBookingVolunteerInfo(int index, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+4+4+2+24+1];

	nullpo_retv(sd);
	WBUFW(buf, 0) = 0x8f2;
	WBUFL(buf, 2) = sd->status.account_id;
	WBUFL(buf, 6) = sd->status.class;
	WBUFW(buf, 10) = sd->status.base_level;
	memcpy(WBUFP(buf, 12), sd->status.name, NAME_LENGTH);

	clif->send(buf, packet_len(0x8f2), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

#if 0 //Disabled for now. Needs more info.
/// 08f3 <packet type>.W <cost>.L
static void clif_PartyBookingPersonalSetting(int fd, struct map_session_data *sd)
{
}

static void clif_parse_PartyBookingShowEquipment(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08f4 <target GID>.L
static void clif_parse_PartyBookingShowEquipment(int fd, struct map_session_data *sd)
{
}

static void clif_parse_PartyBookingReqRecall(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08f5 <packet len>.W
static void clif_parse_PartyBookingReqRecall(int fd, struct map_session_data *sd)
{
}

static void clif_PartyBookingRecallCost(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08f6 <money>.L <map name>.16B
static void clif_PartyBookingRecallCost(int fd, struct map_session_data *sd)
{
}

static void clif_parse_PartyBookingAckRecall(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08f7 <result>.B
static void clif_parse_PartyBookingAckRecall(int fd, struct map_session_data *sd)
{
}

/// 08f8 <caller AID>.L <reason>.B
/// <reason>:
///    REASON_PROHIBITION =  0x0
///    REASON_MASTER_IN_PROHIBITION_MAP =  0x1
///    REASON_REFUSE =  0x2
///    REASON_NOT_PARTY_MEMBER =  0x3
///    REASON_ETC =  0x4
static void clif_PartyBookingFailedRecall(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_PartyBookingFailedRecall(int fd, struct map_session_data *sd)
{
}
#endif //if 0

static void clif_parse_PartyBookingRefuseVolunteer(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08f9 <refuse AID>.L
static void clif_parse_PartyBookingRefuseVolunteer(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	unsigned int aid = RFIFOL(fd, 2);

	clif->PartyBookingRefuseVolunteer(aid, sd);
#else
	return;
#endif
}

static void clif_PartyBookingRefuseVolunteer(unsigned int aid, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08fa <index>.L
static void clif_PartyBookingRefuseVolunteer(unsigned int aid, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6];

	WBUFW(buf, 0) = 0x8fa;
	WBUFL(buf, 2) = aid;

	clif->send(buf, packet_len(0x8fa), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

static void clif_parse_PartyBookingCancelVolunteer(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// 08fb <index>.L
static void clif_parse_PartyBookingCancelVolunteer(int fd, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	int index = RFIFOL(fd, 2);

	clif->PartyBookingCancelVolunteer(index, sd);
#else
	return;
#endif
}

/// 0909 <index>.L
static void clif_PartyBookingCancelVolunteer(int index, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+1];

	nullpo_retv(sd);
	WBUFW(buf, 0) = 0x909;
	WBUFL(buf, 2) = index;

	clif->send(buf, packet_len(0x909), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

/// 090b <gid>.L <char name>.24B
static void clif_PartyBookingAddFilteringList(int index, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+24+1];

	nullpo_retv(sd);
	WBUFW(buf, 0) = 0x90b;
	WBUFL(buf, 2) = sd->bl.id;
	memcpy(WBUFP(buf, 6), sd->status.name, NAME_LENGTH);

	clif->send(buf, packet_len(0x90b), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

/// 090c <gid>.L <char name>.24B
static void clif_PartyBookingSubFilteringList(int gid, struct map_session_data *sd)
{
#ifdef PARTY_RECRUIT
	unsigned char buf[2+6+24+1];

	nullpo_retv(sd);
	WBUFW(buf, 0) = 0x90c;
	WBUFL(buf, 2) = gid;
	memcpy(WBUFP(buf, 6), sd->status.name, NAME_LENGTH);

	clif->send(buf, packet_len(0x90c), &sd->bl, ALL_CLIENT);
#else
	return;
#endif
}

#if 0
/// 091c <aid>.L
static void clif_PartyBookingCancelVolunteerToPM(struct map_session_data *sd)
{
}

/// 0971 <pm_aid>.L
static void clif_PartyBookingRefuseVolunteerToPM(struct map_session_data *sd)
{
}
#endif //if 0

static void clif_parse_CloseVending(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to close own vending (CZ_REQ_CLOSESTORE).
/// 012e
static void clif_parse_CloseVending(int fd, struct map_session_data *sd)
{
	if ((sd->npc_id != 0 && sd->state.using_megaphone == 0) || sd->state.buyingstore != 0 || sd->state.trading != 0)
		return;

	vending->close(sd);
}

static void clif_parse_VendingListReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to open a vending shop (CZ_REQ_BUY_FROMMC).
/// 0130 <account id>.L
static void clif_parse_VendingListReq(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isdead(sd))
		return;

	vending->list(sd,RFIFOL(fd,2));
}

static void clif_parse_PurchaseReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Shop item(s) purchase request (CZ_PC_PURCHASE_ITEMLIST_FROMMC).
/// 0134 <packet len>.W <account id>.L { <amount>.W <index>.W }*
static void clif_parse_PurchaseReq(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int len = (int)RFIFOW(fd, 2) - 8;
	int id;
	const uint8 *data;

	if (len < 1)
		return;

	id = RFIFOL(fd, 4);
	data = RFIFOP(fd, 8);

	vending->purchase(sd, id, sd->vended_id, data, len/4);

	// whether it fails or not, the buy window is closed
	sd->vended_id = 0;
}

static void clif_parse_PurchaseReq2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Shop item(s) purchase request (CZ_PC_PURCHASE_ITEMLIST_FROMMC2).
/// 0801 <packet len>.W <account id>.L <unique id>.L { <amount>.W <index>.W }*
static void clif_parse_PurchaseReq2(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int len = (int)RFIFOW(fd, 2) - 12;
	int aid;
	int uid;
	const uint8 *data;

	if (len < 1)
		return;
	aid = RFIFOL(fd, 4);
	uid = RFIFOL(fd, 8);
	data = RFIFOP(fd, 12);

	vending->purchase(sd, aid, uid, data, len/4);

	// whether it fails or not, the buy window is closed
	sd->vended_id = 0;
}

static void clif_parse_OpenVending(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Confirm or cancel the shop preparation window.
/// 012f <packet len>.W <shop name>.80B { <index>.W <amount>.W <price>.L }* (CZ_REQ_OPENSTORE)
/// 01b2 <packet len>.W <shop name>.80B <result>.B { <index>.W <amount>.W <price>.L }* (CZ_REQ_OPENSTORE2)
/// result:
///     0 = canceled
///     1 = open
static void clif_parse_OpenVending(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0)
	    || pc_isdead(sd) || sd->state.vending != 0 || sd->state.buyingstore != 0) {
		return;
	}

	int len = (int)RFIFOW(fd, 2) - 85;

	if (len < 0)
		return;

	const char *message = RFIFOP(fd, 4);
	bool flag = (RFIFOB(fd, 84) != 0) ? true : false;
	const uint8 *data = RFIFOP(fd, 85);

	if (!flag)
		sd->state.prevend = sd->state.workinprogress = 0;

	if (pc_ismuted(&sd->sc, MANNER_NOROOM))
		return;
	if (map->list[sd->bl.m].flag.novending) {
		clif->message (sd->fd, msg_sd(sd, 276)); // "You can't open a shop on this map"
		return;
	}
	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING)) {
		clif->message (sd->fd, msg_sd(sd,204)); // "You can't open a shop on this cell."
		return;
	}

	if( message[0] == '\0' ) // invalid input
		return;

	vending->open(sd, message, data, len/8);
}

static void clif_parse_CreateGuild(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Guild creation request (CZ_REQ_MAKE_GUILD).
/// 0165 <char id>.L <guild name>.24B
static void clif_parse_CreateGuild(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	char name[NAME_LENGTH];
	safestrncpy(name, RFIFOP(fd,6), NAME_LENGTH);

	if (map->list[sd->bl.m].flag.guildlock) {
		clif->message(fd, msg_fd(fd, 228)); // Guild modification is disabled in this map.
		return;
	}

	guild->create(sd, name);
}

static void clif_parse_GuildCheckMaster(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request for guild window interface permissions (CZ_REQ_GUILD_MENUINTERFACE).
/// 014d
static void clif_parse_GuildCheckMaster(int fd, struct map_session_data *sd)
{
	clif->guild_masterormember(sd);
}

static void clif_parse_GuildRequestInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request for guild window information (CZ_REQ_GUILD_MENU).
/// 014f <type>.L
/// type:
///     0 = basic info
///     1 = member manager
///     2 = positions
///     3 = skills
///     4 = expulsion list
///     5 = unknown (GM_ALLGUILDLIST)
///     6 = notice
static void clif_parse_GuildRequestInfo(int fd, struct map_session_data *sd)
{
	if( !sd->status.guild_id && !sd->bg_id )
		return;

	switch( RFIFOL(fd,2) ) {
		case 0: // Basic Information Guild, hostile alliance information
			clif->guild_basicinfo(sd);
			clif->guild_allianceinfo(sd);
			clif->guild_castlelist(sd);
			break;
		case 1: // Members list, list job title
			clif->guild_positionnamelist(sd);
			clif->guild_memberlist(sd);
			break;
		case 2: // List job title, title information list
			clif->guild_positionnamelist(sd);
			clif->guild_positioninfolist(sd);
			break;
		case 3: // Skill list
			clif->guild_skillinfo(sd);
			break;
		case 4: // Expulsion list
			clif->guild_expulsionlist(sd);
			break;
		default:
			ShowError("clif: guild request info: unknown type %u\n", RFIFOL(fd,2));
			break;
	}
}

static void clif_parse_GuildChangePositionInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update guild positions (CZ_REG_CHANGE_GUILD_POSITIONINFO).
/// 0161 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L <name>.24B }*
static void clif_parse_GuildChangePositionInfo(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int i;
	int count = (RFIFOW(fd, 2) - 4) / 40;

	if (!sd->state.gmaster_flag)
		return;

	for (i = 0; i < count; i ++ ) {
		int idx = i * 40 + 4;
		guild->change_position(sd->status.guild_id, RFIFOL(fd, idx), RFIFOL(fd, idx + 4), RFIFOL(fd, idx + 12), RFIFOP(fd, idx + 16));
	}
}

static void clif_parse_GuildChangeMemberPosition(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update the position of guild members (CZ_REQ_CHANGE_MEMBERPOS).
/// 0155 <packet len>.W { <account id>.L <char id>.L <position id>.L }*
static void clif_parse_GuildChangeMemberPosition(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int i;
	int len = RFIFOW(fd, 2);
	int count = (len - 4) / 12;

	if(!sd->state.gmaster_flag)
		return;

	// Guild leadership change
	if (len == 16 && RFIFOL(fd, 12) == 0) {
		guild->gm_change(sd->status.guild_id, RFIFOL(fd, 8));
		return;
	}

	for (i = 0; i < count; i++) {
		int idx = i * 12 + 4;
		int position = RFIFOL(fd, idx + 8);
		if (position > 0 && position < MAX_GUILDPOSITION) {
			guild->change_memberposition(sd->status.guild_id, RFIFOL(fd, idx), RFIFOL(fd, idx + 4), position);
		}
	}
}

static void clif_parse_GuildRequestEmblem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request for guild emblem data (CZ_REQ_GUILD_EMBLEM_IMG).
/// 0151 <guild id>.L
static void clif_parse_GuildRequestEmblem(int fd, struct map_session_data *sd)
{
	struct guild* g;
	int guild_id = RFIFOL(fd,2);

	if( (g = guild->search(guild_id)) != NULL )
		clif->guild_emblem(sd,g);
}

/// Validates data of a guild emblem (compressed bitmap)
static bool clif_validate_emblem(const uint8 *emblem, unsigned long emblem_len)
{
	enum e_bitmapconst {
		RGBTRIPLE_SIZE = 3,         // sizeof(RGBTRIPLE)
		RGBQUAD_SIZE = 4,           // sizeof(RGBQUAD)
		BITMAPFILEHEADER_SIZE = 14, // sizeof(BITMAPFILEHEADER)
		BITMAPINFOHEADER_SIZE = 40, // sizeof(BITMAPINFOHEADER)
		BITMAP_WIDTH = 24,
		BITMAP_HEIGHT = 24,
	};
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris
	struct s_bitmaptripple {
		//uint8 b;
		//uint8 g;
		//uint8 r;
		uint32 rgb:24;
	} __attribute__((packed));
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris
	uint8 buf[1800]; // no well-formed emblem bitmap is larger than 1782 (24 bit) / 1654 (8 bit) bytes
	unsigned long buf_len = sizeof(buf);
	int header = 0, bitmap = 0, offbits = 0, palettesize = 0;

	nullpo_retr(false, emblem);
	if (grfio->decode_zip(buf, &buf_len, emblem, emblem_len) != 0
	 || buf_len < BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE
	 || RBUFW(buf,0) != 0x4d42 // BITMAPFILEHEADER.bfType (signature)
	 || RBUFL(buf,2) != buf_len // BITMAPFILEHEADER.bfSize (file size)
	 || RBUFL(buf,14) != BITMAPINFOHEADER_SIZE // BITMAPINFOHEADER.biSize (other headers are not supported)
	 || RBUFL(buf,18) != BITMAP_WIDTH // BITMAPINFOHEADER.biWidth
	 || RBUFL(buf,22) != BITMAP_HEIGHT // BITMAPINFOHEADER.biHeight (top-down bitmaps (-24) are not supported)
	 || RBUFL(buf,30) != 0 // BITMAPINFOHEADER.biCompression == BI_RGB (compression not supported)
	 ) {
		// Invalid data
		return false;
	}

	offbits = RBUFL(buf,10); // BITMAPFILEHEADER.bfOffBits (offset to bitmap bits)

	switch( RBUFW(buf,28) ) { // BITMAPINFOHEADER.biBitCount
		case 8:
			palettesize = RBUFL(buf,46); // BITMAPINFOHEADER.biClrUsed (number of colors in the palette)
			if( palettesize == 0 )
				palettesize = 256; // Defaults to 2^n if set to zero
			else if( palettesize > 256 )
				return false;
			header = BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE + RGBQUAD_SIZE * palettesize; // headers + palette
			bitmap = BITMAP_WIDTH * BITMAP_HEIGHT;
			break;
		case 24:
			header = BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE;
			bitmap = BITMAP_WIDTH * BITMAP_HEIGHT * RGBTRIPLE_SIZE;
			break;
		default:
			return false;
	}

	// NOTE: This check gives a little freedom for bitmap-producing implementations,
	// that align the start of bitmap data, which is harmless but unnecessary.
	// If you want it paranoidly strict, change the first condition from < to !=.
	// This also allows files with trailing garbage at the end of the file.
	// If you want to avoid that, change the last condition to !=.
	if( offbits < header || buf_len <= bitmap || offbits > buf_len - bitmap ) {
		return false;
	}

	if( battle_config.client_emblem_max_blank_percent < 100 ) {
		int required_pixels = BITMAP_WIDTH * BITMAP_HEIGHT * (100 - battle_config.client_emblem_max_blank_percent) / 100;
		int found_pixels = 0;
		int i;
		/// Checks what percentage of a guild emblem is blank. A blank emblem
		/// consists solely of magenta pixels. Since the client uses 16-bit
		/// colors, any magenta shade that reduces to #ff00ff passes off as
		/// transparent color as well (down to #f807f8).
		///
		/// Unlike real magenta, reduced magenta causes the guild window to
		/// become see-through in the transparent parts of the emblem
		/// background (glitch).
		switch( RBUFW(buf,28) ) {
			case 8: // palette indexes
			{
				const uint8 *indexes = RBUFP(buf,offbits);
				const uint32 *palette = RBUFP(buf,BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE);

				for (i = 0; i < BITMAP_WIDTH * BITMAP_HEIGHT; i++) {
					if( indexes[i] >= palettesize ) // Invalid color
						return false;

					// if( color->r < 0xF8 || color->g > 0x07 || color->b < 0xF8 )
					if( ( palette[indexes[i]]&0x00F8F8F8 ) != 0x00F800F8 ) {
						if( ++found_pixels >= required_pixels ) {
							// Enough valid pixels were found
							return true;
						}
					}
				}
				break;
			}
			case 24: // full colors
			{
				const struct s_bitmaptripple *pixels = RBUFP(buf,offbits);

				for (i = 0; i < BITMAP_WIDTH * BITMAP_HEIGHT; i++) {
					// if( pixels[i].r < 0xF8 || pixels[i].g > 0x07 || pixels[i].b < 0xF8 )
					if( ( pixels[i].rgb&0xF8F8F8 ) != 0xF800F8 ) {
						if( ++found_pixels >= required_pixels ) {
							// Enough valid pixels were found
							return true;
						}
					}
				}
				break;
			}
		}

		// Not enough non-blank pixels found
		return false;
	}

	return true;
}

static void clif_parse_GuildChangeEmblem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to update the guild emblem (CZ_REGISTER_GUILD_EMBLEM_IMG).
/// 0153 <packet len>.W <emblem data>.?B
static void clif_parse_GuildChangeEmblem(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	unsigned int emblem_len = RFIFOW(fd, 2) - 4;
	const uint8* emblem = RFIFOP(fd, 4);

	if (!emblem_len || !sd->state.gmaster_flag)
		return;

	if (!clif->validate_emblem(emblem, emblem_len)) {
		ShowWarning("clif_parse_GuildChangeEmblem: Rejected malformed guild emblem (size=%u, accound_id=%d, char_id=%d, guild_id=%d).\n",
		            emblem_len, sd->status.account_id, sd->status.char_id, sd->status.guild_id);
		return;
	}

	guild->change_emblem(sd, emblem_len, (const char*)emblem);
}

static void clif_parse_GuildChangeNotice(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Guild notice update request (CZ_GUILD_NOTICE).
/// 016e <guild id>.L <msg1>.60B <msg2>.120B
static void clif_parse_GuildChangeNotice(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int guild_id = RFIFOL(fd, 2);
	char *msg1 = NULL, *msg2 = NULL;

	if (!sd->state.gmaster_flag)
		return;

	msg1 = aStrndup(RFIFOP(fd,6), MAX_GUILDMES1-1);
	msg2 = aStrndup(RFIFOP(fd,66), MAX_GUILDMES2-1);

	// compensate for some client defects when using multilingual mode
	if (msg1[0] == '|' && msg1[3] == '|') msg1+= 3; // skip duplicate marker
	if (msg2[0] == '|' && msg2[3] == '|') msg2+= 3; // skip duplicate marker
	if (msg2[0] == '|') msg2[strnlen(msg2, MAX_GUILDMES2)-1] = '\0'; // delete extra space at the end of string

	guild->change_notice(sd, guild_id, msg1, msg2);
	aFree(msg1);
	aFree(msg2);
}

// Helper function for guild invite functions
static bool clif_sub_guild_invite(int fd, struct map_session_data *sd, struct map_session_data *t_sd)
{
	if ( t_sd == NULL )// not online or does not exist
		return false;

	nullpo_retr(false, sd);
	nullpo_retr(false, t_sd);
	if ( map->list[sd->bl.m].flag.guildlock ) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return false;
	}

	if (t_sd->state.noask) {// @noask [LuzZza]
		clif->noask_sub(sd, t_sd, 2);
		return false;
	}

	// Players in a clan can't join a guild
	if (t_sd->clan != NULL) {
		clif->message(fd, msg_fd(fd, 140)); // You can't join in a clan if you're in a guild.
		return false;
	}

	guild->invite(sd,t_sd);
	return true;
}

static void clif_parse_GuildInvite(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Guild invite request (CZ_REQ_JOIN_GUILD).
/// 0168 <account id>.L <inviter account id>.L <inviter char id>.L
static void clif_parse_GuildInvite(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd = map->id2sd(RFIFOL(fd,2));

	if (!clif_sub_guild_invite(fd, sd, t_sd))
		return;
}

static void clif_parse_GuildInvite2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Guild invite request (/guildinvite) (CZ_REQ_JOIN_GUILD2).
/// 0916 <char name>.24B
static void clif_parse_GuildInvite2(int fd, struct map_session_data *sd)
{
	char nick[NAME_LENGTH];
	struct map_session_data *t_sd = NULL;

	safestrncpy(nick, RFIFOP(fd, 2), NAME_LENGTH);
	t_sd = map->nick2sd(nick, true);

	clif_sub_guild_invite(fd, sd, t_sd);
}

static void clif_parse_GuildReplyInvite(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to guild invitation (CZ_JOIN_GUILD).
/// 016b <guild id>.L <answer>.L
/// answer:
///     0 = refuse
///     1 = accept
static void clif_parse_GuildReplyInvite(int fd, struct map_session_data *sd)
{
	guild->reply_invite(sd, RFIFOL(fd, 2), RFIFOL(fd, 6));
}

static void clif_parse_GuildLeave(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to leave guild (CZ_REQ_LEAVE_GUILD).
/// 0159 <guild id>.L <account id>.L <char id>.L <reason>.40B
static void clif_parse_GuildLeave(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.guildlock) {
		clif->message(fd, msg_fd(fd, 228)); // Guild modification is disabled in this map.
		return;
	}
	if (sd->bg_id) {
		clif->message(fd, msg_fd(fd, 870)); //"You can't leave battleground guilds."
		return;
	}

	guild->leave(sd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10), RFIFOP(fd, 14));
}

static void clif_parse_GuildExpulsion(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to expel a member of a guild (CZ_REQ_BAN_GUILD).
/// 015b <guild id>.L <account id>.L <char id>.L <reason>.40B
static void clif_parse_GuildExpulsion(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.guildlock || sd->bg_id) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return;
	}
	guild->expulsion(sd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10), RFIFOP(fd, 14));
}

/**
 * Validates and processes guild messages (CZ_GUILD_CHAT).
 *
 * @code
 * 017e <packet len>.W <text>.?B (<name> : <message>) 00
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 */
static void clif_parse_GuildMessage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_GuildMessage(int fd, struct map_session_data *sd)
{
	const struct packet_chat_message *packet = RP2PTR(fd);
	char message[CHAT_SIZE_MAX + NAME_LENGTH + 3 + 1];

	if (clif->process_chat_message(sd, packet, message, sizeof message) == NULL)
		return;

	if (sd->bg_id)
		bg->send_message(sd, message);
	else
		guild->send_message(sd, message);
}

static void clif_parse_GuildRequestAlliance(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Guild alliance request (CZ_REQ_ALLY_GUILD).
/// 0170 <account id>.L <inviter account id>.L <inviter char id>.L
static void clif_parse_GuildRequestAlliance(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd;

	if(!sd->state.gmaster_flag)
		return;

	if(map->list[sd->bl.m].flag.guildlock) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return;
	}

	t_sd = map->id2sd(RFIFOL(fd,2));

	// @noask [LuzZza]
	if(t_sd && t_sd->state.noask) {
		clif->noask_sub(sd, t_sd, 3);
		return;
	}

	guild->reqalliance(sd,t_sd);
}

static void clif_parse_GuildReplyAlliance(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to a guild alliance request (CZ_ALLY_GUILD).
/// 0172 <inviter account id>.L <answer>.L
/// answer:
///     0 = refuse
///     1 = accept
static void clif_parse_GuildReplyAlliance(int fd, struct map_session_data *sd)
{
	guild->reply_reqalliance(sd,RFIFOL(fd,2),RFIFOL(fd,6));
}

static void clif_parse_GuildDelAlliance(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete a guild alliance or opposition (CZ_REQ_DELETE_RELATED_GUILD).
/// 0183 <opponent guild id>.L <relation>.L
/// relation:
///     0 = Ally
///     1 = Enemy
static void clif_parse_GuildDelAlliance(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if(!sd->state.gmaster_flag)
		return;

	if(map->list[sd->bl.m].flag.guildlock) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return;
	}
	guild->delalliance(sd,RFIFOL(fd,2),RFIFOL(fd,6));
}

static void clif_parse_GuildOpposition(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to set a guild as opposition (CZ_REQ_HOSTILE_GUILD).
/// 0180 <account id>.L
static void clif_parse_GuildOpposition(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *t_sd;

	if(!sd->state.gmaster_flag)
		return;

	if(map->list[sd->bl.m].flag.guildlock) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return;
	}

	t_sd = map->id2sd(RFIFOL(fd,2));

	// @noask [LuzZza]
	if(t_sd && t_sd->state.noask) {
		clif->noask_sub(sd, t_sd, 4);
		return;
	}

	guild->opposition(sd,t_sd);
}

static void clif_parse_GuildBreak(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete own guild (CZ_REQ_DISORGANIZE_GUILD).
/// 015d <key>.40B
/// key:
///     now guild name; might have been (intended) email, since the
///     field name and size is same as the one in CH_DELETE_CHAR.
static void clif_parse_GuildBreak(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	char key[40];
	if( map->list[sd->bl.m].flag.guildlock ) {
		clif->message(fd, msg_fd(fd,228)); // Guild modification is disabled in this map.
		return;
	}
	safestrncpy(key, RFIFOP(fd, 2), 40);
	guild->dobreak(sd, key);
}

/// Pet
///

static void clif_parse_PetMenu(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke a pet menu action (CZ_COMMAND_PET).
/// 01a1 <type>.B
/// type:
///     0 = pet information
///     1 = feed
///     2 = performance
///     3 = return to egg
///     4 = unequip accessory
static void clif_parse_PetMenu(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	pet->menu(sd, RFIFOB(fd, 2));
}

static void clif_parse_CatchPet(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Attempt to tame a monster (CZ_TRYCAPTURE_MONSTER).
/// 019f <id>.L
static void clif_parse_CatchPet(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	pet->catch_process2(sd, RFIFOL(fd, 2));
}

static void clif_parse_SelectEgg(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to pet incubator egg selection dialog (CZ_SELECT_PETEGG).
/// 01a7 <index>.W
static void clif_parse_SelectEgg(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (sd->menuskill_id != SA_TAMINGMONSTER || sd->menuskill_val != -1) {
		return;
	}
	pet->select_egg(sd, RFIFOW(fd, 2) - 2);
	clif_menuskill_clear(sd);
}

static void clif_parse_SendEmotion(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to display pet's emotion/talk (CZ_PET_ACT).
/// 01a9 <data>.L
/// data:
///     is either emotion (@see enum emotion_type) or a compound value
///     (((mob id)-100)*100+(act id)*10+(hunger)) that describes an
///     entry (given in parentheses) in data\pettalktable.xml
///     act id:
///         0 = feeding
///         1 = hunting
///         2 = danger
///         3 = dead
///         4 = normal (stand)
///         5 = special performance (perfor_s)
///         6 = level up (levelup)
///         7 = performance 1 (perfor_1)
///         8 = performance 2 (perfor_2)
///         9 = performance 3 (perfor_3)
///        10 = log-in greeting (connect)
///     hungry value:
///         0 = very hungry (hungry)
///         1 = hungry (bit_hungry)
///         2 = satisfied (noting)
///         3 = stuffed (full)
///         4 = full (so_full)
static void clif_parse_SendEmotion(int fd, struct map_session_data *sd)
{
	if(sd->pd)
		clif->pet_emotion(sd->pd,RFIFOL(fd,2));
}

static void clif_parse_ChangePetName(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change pet's name (CZ_RENAME_PET).
/// 01a5 <name>.24B
static void clif_parse_ChangePetName(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	pet->change_name(sd, RFIFOP(fd,2));
}

/**
 * Request to evolve the pet. (CZ_PET_EVOLUTION)
 *
 * @code
 *	09fb <Length>.W <EvolvedPetEggID>.W {<index>.W <amount>.W}*items
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 *
 **/
static void clif_parse_pet_evolution(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_pet_evolution(int fd, struct map_session_data *sd)
{
	if (sd->state.trading != 0 || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (sd->pd == NULL || sd->status.pet_id == 0) { // No pet.
		clif->petEvolutionResult(fd, PET_EVOL_NO_CALLPET);
		return;
	}

	int inv_index;

	ARR_FIND(0, sd->status.inventorySize, inv_index, sd->status.inventory[inv_index].card[0] == CARD0_PET
		 && sd->status.pet_id == MakeDWord(sd->status.inventory[inv_index].card[1],
						   sd->status.inventory[inv_index].card[2]));

	if (inv_index == sd->status.inventorySize) { // No pet egg.
		clif->petEvolutionResult(fd, PET_EVOL_NO_PETEGG);
		return;
	}

	if (sd->pd->pet.intimate < PET_INTIMACY_LOYAL) { // Pet isn't loyal.
		clif->petEvolutionResult(fd, PET_EVOL_RG_FAMILIAR);
		return;
	}

	int pet_index;

	ARR_FIND(0, MAX_PET_DB, pet_index, pet->db[pet_index].class_ == sd->pd->pet.class_);

	if (pet_index == MAX_PET_DB) {
		clif->petEvolutionResult(fd, PET_EVOL_UNKNOWN); // Which error?
		return;
	}

	const struct PACKET_CZ_PET_EVOLUTION *p = RP2PTR(fd);

	Assert_retv(p->PacketLength >= (uint16)sizeof(struct PACKET_CZ_PET_EVOLUTION));

	// Client side validation is not done as it is insecure.
	for (int i = 0; i < VECTOR_LENGTH(pet->db[pet_index].evolve_data); i++) {
		struct pet_evolve_data *ped = &VECTOR_INDEX(pet->db[pet_index].evolve_data, i);

		if (ped->petEggId == p->EvolvedPetEggID) {
			if (VECTOR_LENGTH(ped->items) == 0) {
				clif->petEvolutionResult(fd, PET_EVOL_NO_RECIPE);
				return;
			}

			for (int j = 0; j < VECTOR_LENGTH(ped->items); j++) {
				struct itemlist_entry *list = &VECTOR_INDEX(ped->items, j);
				int n = pc->search_inventory(sd, list->id);

				if (n == INDEX_NOT_FOUND) {
					clif->petEvolutionResult(fd, PET_EVOL_NO_MATERIAL);
					return;
				}
			}

			for (int j = 0; j < VECTOR_LENGTH(ped->items); j++) {
				struct itemlist_entry *list = &VECTOR_INDEX(ped->items, j);
				int n = pc->search_inventory(sd, list->id);

				if (pc->delitem(sd, n, list->amount, 0, DELITEM_NORMAL, LOG_TYPE_EGG) == 1) {
					clif->petEvolutionResult(fd, PET_EVOL_NO_MATERIAL);
					return;
				}
			}

			pet->return_egg(sd, sd->pd); // Return pet to egg.

			if (pc->delitem(sd, inv_index, 1, 0, DELITEM_NORMAL, LOG_TYPE_EGG) == 1) {
				clif->petEvolutionResult(fd, PET_EVOL_NO_PETEGG);
				return;
			}

			int pet_id = pet->search_petDB_index(ped->petEggId, PET_EGG);

			if (pet_id != INDEX_NOT_FOUND) {
				sd->catch_target_class = pet->db[pet_id].class_;
				intif->create_pet(sd->status.account_id, sd->status.char_id, pet->db[pet_id].class_,
						  mob->db(pet->db[pet_id].class_)->lv, pet->db[pet_id].EggID,
						  0, (short)pet->db[pet_id].intimate, PET_HUNGER_STUFFED,
						  0, 1, pet->db[pet_id].jname);
				clif->petEvolutionResult(fd, PET_EVOL_SUCCESS);
			} else {
				clif->petEvolutionResult(fd, PET_EVOL_UNKNOWN);
			}

			return;
		}
	}

	clif->petEvolutionResult(fd, PET_EVOL_UNKNOWN);
}

/**
 * Result of Pet Evolution (ZC_PET_EVOLUTION_RESULT)
 * 0x9fc <Result>.L
 */
static void clif_pet_evolution_result(int fd, enum pet_evolution_result result)
{
#if PACKETVER >= 20140122
	WFIFOHEAD(fd, packet_len(0x9fc));
	WFIFOW(fd, 0) = 0x9fc;
	WFIFOL(fd, 2) = result;
	WFIFOSET(fd, packet_len(0x9fc));
#endif
}

static void clif_parse_GMKick(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /kill (CZ_DISCONNECT_CHARACTER).
/// Request to disconnect a character.
/// 00cc <account id>.L
/// NOTE: Also sent when using GM right click menu "(name) force to quit"
static void clif_parse_GMKick(int fd, struct map_session_data *sd)
{
	struct block_list *target;
	int tid;

	tid = RFIFOL(fd,2);
	target = map->id2bl(tid);
	if (!target) {
		clif->GM_kickack(sd, 0);
		return;
	}

	switch (target->type) {
		case BL_PC:
		{
			char command[NAME_LENGTH+6];
			sprintf(command, "%ckick %s", atcommand->at_symbol, clif->get_bl_name(target));
			atcommand->exec(fd, sd, command, true);
		}
		break;

		/**
		 * This one does not invoke any atcommand, so we need to check for permissions.
		 */
		case BL_MOB:
		{
			char command[100];
			if( !pc->can_use_command(sd, "@killmonster")) {
				clif->GM_kickack(sd, 0);
				return;
			}
			sprintf(command, "/kick %s (%d)", clif->get_bl_name(target), status->get_class(target));
			logs->atcommand(sd, command);
			status_percent_damage(&sd->bl, target, 100, 0, true); // can invalidate 'target'
		}
		break;

		case BL_NPC:
		{
			struct npc_data *nd = BL_UCAST(BL_NPC, target);
			if( !pc->can_use_command(sd, "@unloadnpc")) {
				clif->GM_kickack(sd, 0);
				return;
			}
			npc->unload_duplicates(nd, true);
			npc->unload(nd, true, true);
			npc->motd = npc->name2id("HerculesMOTD");
			npc->read_event_script();
		}
		break;

		default:
			clif->GM_kickack(sd, 0);
	}
}

static void clif_parse_GMKickAll(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /killall (CZ_DISCONNECT_ALL_CHARACTER).
/// Request to disconnect all characters.
/// 00ce
static void clif_parse_GMKickAll(int fd, struct map_session_data *sd)
{
	char cmd[15];
	sprintf(cmd,"%ckickall",atcommand->at_symbol);
	atcommand->exec(fd, sd, cmd, true);
}

static void clif_parse_GMShift(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /remove (CZ_REMOVE_AID).
/// Request to warp to a character with given login ID.
/// 01ba <account name>.24B

/// /shift (CZ_SHIFT).
/// Request to warp to a character with given name.
/// 01bb <char name>.24B
static void clif_parse_GMShift(int fd, struct map_session_data *sd)
{
	// FIXME: remove is supposed to receive account name for clients prior 20100803RE
	char player_name[NAME_LENGTH];
	char command[NAME_LENGTH + 20];

	safestrncpy(player_name, RFIFOP(fd,2), NAME_LENGTH);

	sprintf(command, "%cjumpto %s", atcommand->at_symbol, player_name);
	atcommand->exec(fd, sd, command, true);
}

static void clif_parse_GMRemove2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /remove (CZ_REMOVE_AID_SSO).
/// Request to warp to a character with given account ID.
/// 0843 <account id>.L
static void clif_parse_GMRemove2(int fd, struct map_session_data *sd)
{
	int account_id;
	struct map_session_data* pl_sd;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( (pl_sd = map->id2sd(account_id)) != NULL ) {
		char command[NAME_LENGTH + 20];
		sprintf(command, "%cjumpto %s", atcommand->at_symbol, pl_sd->status.name);
		atcommand->exec(fd, sd, command, true);
	}
}

static void clif_parse_GMRecall(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /recall (CZ_RECALL).
/// Request to summon a player with given login ID to own position.
/// 01bc <account name>.24B

/// /summon (CZ_RECALL_GID).
/// Request to summon a player with given name to own position.
/// 01bd <char name>.24B
static void clif_parse_GMRecall(int fd, struct map_session_data *sd)
{
	// FIXME: recall is supposed to receive account name for clients prior 20100803RE
	char player_name[NAME_LENGTH];
	char command[NAME_LENGTH+8];

	safestrncpy(player_name, RFIFOP(fd,2), NAME_LENGTH);

	sprintf(command, "%crecall %s", atcommand->at_symbol, player_name);
	atcommand->exec(fd, sd, command, true);
}

static void clif_parse_GMRecall2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /recall (CZ_RECALL_SSO).
/// Request to summon a player with given account ID to own position.
/// 0842 <account id>.L
static void clif_parse_GMRecall2(int fd, struct map_session_data *sd)
{
	int account_id;
	struct map_session_data* pl_sd;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( (pl_sd = map->id2sd(account_id)) != NULL ) {
		char command[NAME_LENGTH + 20];
		sprintf(command, "%crecall %s", atcommand->at_symbol, pl_sd->status.name);
		atcommand->exec(fd, sd, command, true);
	}
}

static void clif_parse_GM_Monster_Item(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /item /monster (CZ_ITEM_CREATE).
/// Request to execute GM commands.
/// usage:
/// /item n - summon n monster or acquire n item/s
/// /item money - grants 2147483647 Zeny
/// /item whereisboss - locate boss mob in current map.(not yet implemented)
/// /item regenboss_n t - regenerate n boss monster by t millisecond.(not yet implemented)
/// /item onekillmonster - toggle an ability to kill mobs in one hit.(not yet implemented)
/// /item bossinfo - display the information of a boss monster in current map.(not yet implemented)
/// /item cap_n - capture n monster as pet.(not yet implemented)
/// /item agitinvest - reset current global agit investments.(not yet implemented)
/// 013f <item/mob name>.24B
/// 09ce <item/mob name>.100B [Ind/Yommy<3]
static void clif_parse_GM_Monster_Item(int fd, struct map_session_data *sd)
{
	const struct packet_gm_monster_item *p = RP2PTR(fd);
	int i, count;
	char item_monster_name[sizeof p->str];
	struct item_data *item_array[10];
	struct mob_db *mob_array[10];
	char command[256];

	safestrncpy(item_monster_name, p->str, sizeof(item_monster_name));

	if ( (count=itemdb->search_name_array(item_array, 10, item_monster_name, IT_SEARCH_NAME_EXACT)) > 0 ) {
		for(i = 0; i < count; i++) {
			if( !item_array[i] )
				continue;
			// It only accepts aegis name
			if( battle_config.case_sensitive_aegisnames && strcmp(item_array[i]->name, item_monster_name) == 0 )
				break;
			if( !battle_config.case_sensitive_aegisnames && strcasecmp(item_array[i]->name, item_monster_name) == 0 )
				break;
		}

		if( i < count ) {
			if( item_array[i]->type == IT_WEAPON || item_array[i]->type == IT_ARMOR ) // nonstackable
				snprintf(command, sizeof(command)-1, "%citem2 %d 1 0 0 0 0 0 0 0", atcommand->at_symbol, item_array[i]->nameid);
			else
				snprintf(command, sizeof(command)-1, "%citem %d 20", atcommand->at_symbol, item_array[i]->nameid);
			atcommand->exec(fd, sd, command, true);
			return;
		}
	}

	if( strcmp("money", item_monster_name) == 0 ){
		snprintf(command, sizeof(command)-1, "%czeny %d", atcommand->at_symbol, INT_MAX);
		atcommand->exec(fd, sd, command, true);
		return;
	}

	if( (count=mob->db_searchname_array(mob_array, 10, item_monster_name, 1)) > 0) {
		for(i = 0; i < count; i++) {
			if( !mob_array[i] )
				continue;
			// It only accepts sprite name
			if( battle_config.case_sensitive_aegisnames && strcmp(mob_array[i]->sprite, item_monster_name) == 0 )
				break;
			if( !battle_config.case_sensitive_aegisnames && strcasecmp(mob_array[i]->sprite, item_monster_name) == 0 )
				break;
		}

		if( i < count ){
			snprintf(command, sizeof(command)-1, "%cmonster %s", atcommand->at_symbol, mob_array[i]->sprite);
			atcommand->exec(fd, sd, command, true);
		}
	}
}

static void clif_parse_GMHide(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /hide (CZ_CHANGE_EFFECTSTATE).
/// 019d <effect state>.L
/// effect state:
///     TODO: Any OPTION_* ?
static void clif_parse_GMHide(int fd, struct map_session_data *sd)
{
	char cmd[6];

	sprintf(cmd,"%chide",atcommand->at_symbol);

	atcommand->exec(fd, sd, cmd, true);
}

static void clif_parse_GMReqNoChat(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to adjust player's manner points (CZ_REQ_GIVE_MANNER_POINT).
/// 0149 <account id>.L <type>.B <value>.W
/// type:
///     0 = positive points
///     1 = negative points
///     2 = self mute (+10 minutes)
static void clif_parse_GMReqNoChat(int fd, struct map_session_data *sd)
{
	int id, type, value;
	struct map_session_data *dstsd;
	char command[NAME_LENGTH+15];

	id = RFIFOL(fd,2);
	type = RFIFOB(fd,6);
	value = RFIFOW(fd,7);

	if( type == 0 )
		value = -value;

	if (type == 2) {
		if (!battle_config.client_accept_chatdori)
			return;
		if (pc_get_group_level(sd) > 0 || sd->bl.id != id)
			return;

		value = battle_config.client_accept_chatdori;
		dstsd = sd;
	} else {
		dstsd = map->id2sd(id);
		if( dstsd == NULL )
			return;
	}

	if (type == 2 || ( (pc_get_group_level(sd)) > pc_get_group_level(dstsd) && !pc->can_use_command(sd, "@mute"))) {
		clif->manner_message(sd, 0);
		clif->manner_message(dstsd, 5);

		if (dstsd->status.manner < value) {
			dstsd->status.manner -= value;
			sc_start(NULL,&dstsd->bl,SC_NOCHAT,100,0,0);

		} else {
			dstsd->status.manner = 0;
			status_change_end(&dstsd->bl, SC_NOCHAT, INVALID_TIMER);
		}

		if( type != 2 )
			clif->GM_silence(sd, dstsd, type);
	}

	sprintf(command, "%cmute %d %s", atcommand->at_symbol, value, dstsd->status.name);
	atcommand->exec(fd, sd, command, true);
}

static void clif_parse_GMRc(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /rc (CZ_REQ_GIVE_MANNER_BYNAME).
/// GM adjustment of a player's manner value by -60.
/// 0212 <char name>.24B
static void clif_parse_GMRc(int fd, struct map_session_data *sd)
{
	char command[NAME_LENGTH+15];
	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOP(fd,2), NAME_LENGTH);

	sprintf(command, "%cmute %d %s", atcommand->at_symbol, 60, name);
	atcommand->exec(fd, sd, command, true);
}

/// Result of request to resolve account name (ZC_ACK_ACCOUNTNAME).
/// 01e0 <account id>.L <account name>.24B
static void clif_account_name(struct map_session_data *sd, int account_id, const char *accname)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x1e0));
	WFIFOW(fd,0) = 0x1e0;
	WFIFOL(fd,2) = account_id;
	safestrncpy(WFIFOP(fd,6), accname, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x1e0));
}

static void clif_parse_GMReqAccountName(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// GM requesting account name (for right-click gm menu) (CZ_REQ_ACCOUNTNAME).
/// 01df <account id>.L
static void clif_parse_GMReqAccountName(int fd, struct map_session_data *sd)
{
	int account_id = RFIFOL(fd,2);

	//TODO: find out if this works for any player or only for authorized GMs
	clif->account_name(sd, account_id, ""); // insert account name here >_<
}

static void clif_parse_GMChangeMapType(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /changemaptype <x> <y> <type> (CZ_CHANGE_MAPTYPE).
/// GM single cell type change request.
/// 0198 <x>.W <y>.W <type>.W
/// type:
///     0 = not walkable
///     1 = walkable
static void clif_parse_GMChangeMapType(int fd, struct map_session_data *sd)
{
	int x,y,type;

	if (!pc_has_permission(sd, PC_PERM_USE_CHANGEMAPTYPE))
		return;

	x = RFIFOW(fd,2);
	y = RFIFOW(fd,4);
	type = RFIFOW(fd,6);

	map->setgatcell(sd->bl.m,x,y,type);
	clif->changemapcell(0,sd->bl.m,x,y,type,ALL_SAMEMAP);
	//FIXME: once players leave the map, the client 'forgets' this information.
}

static void clif_parse_PMIgnore(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /in /ex (CZ_SETTING_WHISPER_PC).
/// Request to allow/deny whispers from a nick.
/// 00cf <nick>.24B <type>.B
/// type:
///     0 = (/ex nick) deny speech from nick
///     1 = (/in nick) allow speech from nick
static void clif_parse_PMIgnore(int fd, struct map_session_data *sd)
{
	char nick[NAME_LENGTH];
	uint8 type;
	int i;

	safestrncpy(nick, RFIFOP(fd,2), NAME_LENGTH);

	type = RFIFOB(fd,26);

	if( type == 0 ) { // Add name to ignore list (block)
		if (strcmp(map->wisp_server_name, nick) == 0) {
			clif->wisexin(sd, type, 1); // fail
			return;
		}

		// try to find a free spot, while checking for duplicates at the same time
		ARR_FIND( 0, MAX_IGNORE_LIST, i, sd->ignore[i].name[0] == '\0' || strcmp(sd->ignore[i].name, nick) == 0 );
		if( i == MAX_IGNORE_LIST ) {// no space for new entry
			clif->wisexin(sd, type, 2); // too many blocks
			return;
		}
		if( sd->ignore[i].name[0] != '\0' ) { // name already exists
			clif->wisexin(sd, type, 0); // Aegis reports success.
			return;
		}

		//Insert in position i
		safestrncpy(sd->ignore[i].name, nick, NAME_LENGTH);
	} else { // Remove name from ignore list (unblock)

		// find entry
		ARR_FIND( 0, MAX_IGNORE_LIST, i, sd->ignore[i].name[0] == '\0' || strcmp(sd->ignore[i].name, nick) == 0 );
		if( i == MAX_IGNORE_LIST || sd->ignore[i].name[i] == '\0' ) { //Not found
			clif->wisexin(sd, type, 1); // fail
			return;
		}
		// move everything one place down to overwrite removed entry
		if( i != MAX_IGNORE_LIST - 1 )
			memmove(&sd->ignore[i], &sd->ignore[i+1], (MAX_IGNORE_LIST-i-1)*sizeof(sd->ignore[0]));
		// wipe last entry
		memset(sd->ignore[MAX_IGNORE_LIST-1].name, 0, sizeof(sd->ignore[0].name));
	}

	clif->wisexin(sd, type, 0); // success
}

static void clif_parse_PMIgnoreAll(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /inall /exall (CZ_SETTING_WHISPER_STATE).
/// Request to allow/deny all whispers.
/// 00d0 <type>.B
/// type:
///     0 = (/exall) deny all speech
///     1 = (/inall) allow all speech
static void clif_parse_PMIgnoreAll(int fd, struct map_session_data *sd)
{
	int type = RFIFOB(fd,2), flag;

	if( type == 0 ) {// Deny all
		if( sd->state.ignoreAll ) {
			flag = 1; // fail
		} else {
			sd->state.ignoreAll = 1;
			flag = 0; // success
		}
	} else {//Unblock everyone
		if( sd->state.ignoreAll ) {
			sd->state.ignoreAll = 0;
			flag = 0; // success
		} else {
			if (sd->ignore[0].name[0] != '\0')
			{  //Wipe the ignore list.
				memset(sd->ignore, 0, sizeof(sd->ignore));
				flag = 0; // success
			} else {
				flag = 1; // fail
			}
		}
	}

	clif->wisall(sd, type, flag);
}

/// Whisper ignore list (ZC_WHISPER_LIST).
/// 00d4 <packet len>.W { <char name>.24B }*
static void clif_PMIgnoreList(struct map_session_data *sd)
{
	int i, fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,4+ARRAYLENGTH(sd->ignore)*NAME_LENGTH);
	WFIFOW(fd,0) = 0xd4;

	for( i = 0; i < ARRAYLENGTH(sd->ignore) && sd->ignore[i].name[0]; i++ ) {
		memcpy(WFIFOP(fd,4+i*NAME_LENGTH), sd->ignore[i].name, NAME_LENGTH);
	}

	WFIFOW(fd,2) = 4+i*NAME_LENGTH;
	WFIFOSET(fd,WFIFOW(fd,2));
}

static void clif_parse_PMIgnoreList(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Whisper ignore list request (CZ_REQ_WHISPER_LIST).
/// 00d3
static void clif_parse_PMIgnoreList(int fd, struct map_session_data *sd)
{
	clif->PMIgnoreList(sd);
}

static void clif_parse_NoviceDoriDori(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke the /doridori recovery bonus (CZ_DORIDORI).
/// 01e7
static void clif_parse_NoviceDoriDori(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (sd->state.doridori) return;

	switch (sd->job & MAPID_UPPERMASK) {
		case MAPID_SOUL_LINKER:
		case MAPID_STAR_GLADIATOR:
		case MAPID_TAEKWON:
			if (!sd->state.rest)
				break;
			FALLTHROUGH
		case MAPID_SUPER_NOVICE:
			sd->state.doridori=1;
			break;
	}
}

static void clif_parse_NoviceExplosionSpirits(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke the effect of super novice's guardian angel prayer (CZ_CHOPOKGI).
/// 01ed
/// Note: This packet is caused by 7 lines of any text, followed by
///       the prayer and an another line of any text. The prayer is
///       defined by lines 790~793 in data\msgstringtable.txt
///       "Dear angel, can you hear my voice?"
///       "I am" (space separated player name) "Super Novice~"
///       "Help me out~ Please~ T_T"
static void clif_parse_NoviceExplosionSpirits(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	/* [Ind/Hercules] */
	/* game client is currently broken on this (not sure the packetver range) */
	/* it sends the request when the criteria doesn't match (and of course we let it fail) */
	/* so restoring the old parse_globalmes method. */
	if ((sd->job & MAPID_UPPERMASK) == MAPID_SUPER_NOVICE) {
		uint64 next = pc->nextbaseexp(sd);
		if( next == 0 ) next = pc->thisbaseexp(sd);
		if( next ) {
			int percent = (int)( ( (float)sd->status.base_exp/(float)next )*1000. );

			if( percent && ( percent%100 ) == 0 ) {// 10.0%, 20.0%, ..., 90.0%
				sc_start(NULL,&sd->bl, status->skill2sc(MO_EXPLOSIONSPIRITS), 100, 17, skill->get_time(MO_EXPLOSIONSPIRITS, 5)); //Lv17-> +50 critical (noted by Poki) [Skotlex]
				clif->skill_nodamage(&sd->bl, &sd->bl, MO_EXPLOSIONSPIRITS, 5, 1);  // prayer always shows successful Lv5 cast and disregards noskill restrictions
			}
		}
	}
}

/// Friends List
///

/// Toggles a single friend online/offline [Skotlex] (ZC_FRIENDS_STATE).
/// 0206 <account id>.L <char id>.L <state>.B
/// 0206 <account id>.L <char id>.L <state>.B <name>.24B
/// state:
///     0 = online
///     1 = offline
static void clif_friendslist_toggle(struct map_session_data *sd, int account_id, int char_id, int online)
{
	int i, fd;

	nullpo_retv(sd);
	fd = sd->fd;
	//Seek friend.
	for (i = 0; i < MAX_FRIENDS && sd->status.friends[i].char_id &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);

	if(i == MAX_FRIENDS || sd->status.friends[i].char_id == 0)
		return; //Not found

	WFIFOHEAD(fd,packet_len(0x206));
	WFIFOW(fd, 0) = 0x206;
	WFIFOL(fd, 2) = sd->status.friends[i].account_id;
	WFIFOL(fd, 6) = sd->status.friends[i].char_id;
	WFIFOB(fd, 10) = !online; //Yeah, a 1 here means "logged off", go figure...
#if PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328
	memcpy(WFIFOP(fd, 11), sd->status.friends[i].name, NAME_LENGTH);
#endif  // PACKETVER_ZERO

	WFIFOSET(fd, packet_len(0x206));
}

//Sub-function called from clif_foreachclient to toggle friends on/off [Skotlex]
static int clif_friendslist_toggle_sub(struct map_session_data *sd, va_list ap)
{
	int account_id, char_id, online;
	account_id = va_arg(ap, int);
	char_id = va_arg(ap, int);
	online = va_arg(ap, int);
	clif->friendslist_toggle(sd, account_id, char_id, online);
	return 0;
}

/// Sends the whole friends list (ZC_FRIENDS_LIST).
/// 0201 <packet len>.W { <account id>.L <char id>.L <name>.24B }*
/// 0201 <packet len>.W { <account id>.L <char id>.L }*
static void clif_friendslist_send(struct map_session_data *sd)
{
	int i = 0, n, fd = sd->fd;

#if PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328
	const int offset = 8;
#else
	const int offset = 32;
#endif
	nullpo_retv(sd);
	// Send friends list
	WFIFOHEAD(fd, MAX_FRIENDS * offset + 4);
	WFIFOW(fd, 0) = 0x201;
	for(i = 0; i < MAX_FRIENDS && sd->status.friends[i].char_id; i++) {
		WFIFOL(fd, 4 + offset * i + 0) = sd->status.friends[i].account_id;
		WFIFOL(fd, 4 + offset * i + 4) = sd->status.friends[i].char_id;
#if !(PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328)
		memcpy(WFIFOP(fd, 4 + offset * i + 8), &sd->status.friends[i].name, NAME_LENGTH);
#endif
	}

	if (i) {
		WFIFOW(fd,2) = 4 + offset * i;
		WFIFOSET(fd, WFIFOW(fd,2));
	}

	for (n = 0; n < i; n++) { //Sending the online players
		if (map->charid2sd(sd->status.friends[n].char_id))
			clif->friendslist_toggle(sd, sd->status.friends[n].account_id, sd->status.friends[n].char_id, 1);
	}
}

/// Notification about the result of a friend add request (ZC_ADD_FRIENDS_LIST).
/// 0209 <result>.W <account id>.L <char id>.L <name>.24B
/// result:
///     0 = MsgStringTable[821]="You have become friends with (%s)."
///     1 = MsgStringTable[822]="(%s) does not want to be friends with you."
///     2 = MsgStringTable[819]="Your Friend List is full."
///     3 = MsgStringTable[820]="(%s)'s Friend List is full."
static void clif_friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x209));
	WFIFOW(fd,0) = 0x209;
	WFIFOW(fd,2) = type;
	if (f_sd) {
		WFIFOL(fd,4) = f_sd->status.account_id;
		WFIFOL(fd,8) = f_sd->status.char_id;
		memcpy(WFIFOP(fd, 12), f_sd->status.name,NAME_LENGTH);
	}
	WFIFOSET(fd, packet_len(0x209));
}

/// Asks a player for permission to be added as friend (ZC_REQ_ADD_FRIENDS).
/// 0207 <req account id>.L <req char id>.L <req char name>.24B
static void clif_friendlist_req(struct map_session_data *sd, int account_id, int char_id, const char *name)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x207));
	WFIFOW(fd,0) = 0x207;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	memcpy(WFIFOP(fd,10), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x207));
}

static void clif_parse_FriendsListAdd(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add a player as friend (CZ_ADD_FRIENDS).
/// 0202 <name>.24B
static void clif_parse_FriendsListAdd(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *f_sd;
	int i;
	char nick[NAME_LENGTH];

	safestrncpy(nick, RFIFOP(fd,2), NAME_LENGTH);

	f_sd = map->nick2sd(nick, true);

	// ensure that the request player's friend list is not full
	ARR_FIND(0, MAX_FRIENDS, i, sd->status.friends[i].char_id == 0);

	if( i == MAX_FRIENDS ) {
		clif->friendslist_reqack(sd, f_sd, 2);
		return;
	}

	// Friend doesn't exist (no player with this name)
	if (f_sd == NULL) {
		clif->message(fd, msg_fd(fd,3)); // "Character not found."
		return;
	}

	if( sd->bl.id == f_sd->bl.id ) { // adding oneself as friend
		return;
	}

	// @noask [LuzZza]
	if(f_sd->state.noask) {
		clif->noask_sub(sd, f_sd, 5);
		return;
	}

	// Friend already exists
	for (i = 0; i < MAX_FRIENDS && sd->status.friends[i].char_id != 0; i++) {
		if (sd->status.friends[i].char_id == f_sd->status.char_id) {
			clif->message(fd, msg_fd(fd,871)); //"Friend already exists."
			return;
		}
	}

	f_sd->friend_req = sd->status.char_id;
	sd->friend_req   = f_sd->status.char_id;

	clif->friendlist_req(f_sd, sd->status.account_id, sd->status.char_id, sd->status.name);
}

static void clif_parse_FriendsListReply(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to a friend add request (CZ_ACK_REQ_ADD_FRIENDS).
/// 0208 <inviter account id>.L <inviter char id>.L <result>.B
/// 0208 <inviter account id>.L <inviter char id>.L <result>.L (PACKETVER >= 6)
/// result:
///     0 = rejected
///     1 = accepted
static void clif_parse_FriendsListReply(int fd, struct map_session_data *sd)
{
	struct map_session_data *f_sd;
	int account_id;
	char reply;

	account_id = RFIFOL(fd,2);
	//char_id = RFIFOL(fd,6);
#if PACKETVER < 6
	reply = RFIFOB(fd,10);
#else
	reply = RFIFOL(fd,10);
#endif

	if( sd->bl.id == account_id ) { // adding oneself as friend
		return;
	}

	f_sd = map->id2sd(account_id); //The account id is the same as the bl.id of players.
	if (f_sd == NULL)
		return;

	if (reply == 0 || !( sd->friend_req == f_sd->status.char_id && f_sd->friend_req == sd->status.char_id ) )
		clif->friendslist_reqack(f_sd, sd, 1);
	else {
		int i;
		// Find an empty slot
		for (i = 0; i < MAX_FRIENDS; i++)
			if (f_sd->status.friends[i].char_id == 0)
				break;
		if (i == MAX_FRIENDS) {
			clif->friendslist_reqack(f_sd, sd, 2);
			return;
		}

		f_sd->status.friends[i].account_id = sd->status.account_id;
		f_sd->status.friends[i].char_id = sd->status.char_id;
		memcpy(f_sd->status.friends[i].name, sd->status.name, NAME_LENGTH);
		clif->friendslist_reqack(f_sd, sd, 0);
		achievement->validate_friend_add(f_sd); // Achievements [Smokexyz/Hercules]

		if (battle_config.friend_auto_add) {
			// Also add f_sd to sd's friendlist.
			for (i = 0; i < MAX_FRIENDS; i++) {
				if (sd->status.friends[i].char_id == f_sd->status.char_id)
					return; //No need to add anything.
				if (sd->status.friends[i].char_id == 0)
					break;
			}
			if (i == MAX_FRIENDS) {
				clif->friendslist_reqack(sd, f_sd, 2);
				return;
			}

			sd->status.friends[i].account_id = f_sd->status.account_id;
			sd->status.friends[i].char_id = f_sd->status.char_id;
			memcpy(sd->status.friends[i].name, f_sd->status.name, NAME_LENGTH);
			clif->friendslist_reqack(sd, f_sd, 0);
			achievement->validate_friend_add(sd); // Achievements [Smokexyz/Hercules]
		}
	}
}

static void clif_parse_FriendsListRemove(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete a friend (CZ_DELETE_FRIENDS).
/// 0203 <account id>.L <char id>.L
static void clif_parse_FriendsListRemove(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *f_sd = NULL;
	int account_id, char_id;
	int i, j;

	account_id = RFIFOL(fd,2);
	char_id = RFIFOL(fd,6);

	// Search friend
	for (i = 0; i < MAX_FRIENDS &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);

	if (i == MAX_FRIENDS) {
		clif->message(fd, msg_fd(fd,872)); //"Name not found in list."
		return;
	}

	//remove from friend's list first
	if( (f_sd = map->id2sd(account_id)) && f_sd->status.char_id == char_id) {
		for (i = 0; i < MAX_FRIENDS &&
			(f_sd->status.friends[i].char_id != sd->status.char_id || f_sd->status.friends[i].account_id != sd->status.account_id); i++);

		if (i != MAX_FRIENDS) {
			// move all chars up
			for(j = i + 1; j < MAX_FRIENDS; j++)
				memcpy(&f_sd->status.friends[j-1], &f_sd->status.friends[j], sizeof(f_sd->status.friends[0]));

			memset(&f_sd->status.friends[MAX_FRIENDS-1], 0, sizeof(f_sd->status.friends[MAX_FRIENDS-1]));
			//should the guy be notified of some message? we should add it here if so
			WFIFOHEAD(f_sd->fd,packet_len(0x20a));
			WFIFOW(f_sd->fd,0) = 0x20a;
			WFIFOL(f_sd->fd,2) = sd->status.account_id;
			WFIFOL(f_sd->fd,6) = sd->status.char_id;
			WFIFOSET(f_sd->fd, packet_len(0x20a));
		}

	} else { //friend not online -- ask char server to delete from his friendlist
		if(!chrif->removefriend(char_id,sd->status.char_id)) { // char-server offline, abort
			clif->message(fd, msg_fd(fd,873)); //"This action can't be performed at the moment. Please try again later."
			return;
		}
	}

	// We can now delete from original requester
	for (i = 0; i < MAX_FRIENDS &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);
	// move all chars up
	for(j = i + 1; j < MAX_FRIENDS; j++)
		memcpy(&sd->status.friends[j-1], &sd->status.friends[j], sizeof(sd->status.friends[0]));

	memset(&sd->status.friends[MAX_FRIENDS-1], 0, sizeof(sd->status.friends[MAX_FRIENDS-1]));
	clif->message(fd, msg_fd(fd,874)); //"Friend removed"

	WFIFOHEAD(fd,packet_len(0x20a));
	WFIFOW(fd,0) = 0x20a;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	WFIFOSET(fd, packet_len(0x20a));
}

/// /pvpinfo list (ZC_ACK_PVPPOINT).
/// 0210 <char id>.L <account id>.L <win point>.L <lose point>.L <point>.L
static void clif_PVPInfo(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x210));
	WFIFOW(fd,0) = 0x210;
	WFIFOL(fd,2) = sd->status.char_id;
	WFIFOL(fd,6) = sd->status.account_id;
	WFIFOL(fd,10) = sd->pvp_won;  // times won
	WFIFOL(fd,14) = sd->pvp_lost; // times lost
	WFIFOL(fd,18) = sd->pvp_point;
	WFIFOSET(fd, packet_len(0x210));
}

static void clif_parse_PVPInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /pvpinfo (CZ_REQ_PVPPOINT).
/// 020f <char id>.L <account id>.L
static void clif_parse_PVPInfo(int fd, struct map_session_data *sd)
{
	// TODO: Is there a way to use this on an another player (char/acc id)?
	clif->PVPInfo(sd);
}

/// Ranking list

/// ranking pointlist  { <name>.24B <point>.L }*10
static void clif_ranklist_sub(struct PACKET_ZC_ACK_RANKING_sub *ranks, enum fame_list_type type)
{
#if !(PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190724)
	nullpo_retv(ranks);

	struct fame_list* list;
	switch (type) {
		case RANKTYPE_BLACKSMITH: list = pc->smith_fame_list; break;
		case RANKTYPE_ALCHEMIST:  list = pc->chemist_fame_list; break;
		case RANKTYPE_TAEKWON:    list = pc->taekwon_fame_list; break;
		default: return; // Unsupported
	}

	int i;
	// Packet size limits this list to 10 elements. [Skotlex]
	for (i = 0; i < 10 && i < MAX_FAME_LIST; i++) {
		if (list[i].id > 0) {
			const char* name;
			if (strcmp(list[i].name, "-") == 0 && (name = map->charid2nick(list[i].id)) != NULL) {
				strncpy(ranks->names[i].name, name, NAME_LENGTH);
			} else {
				strncpy(ranks->names[i].name, list[i].name, NAME_LENGTH);
			}
		} else {
			strncpy(ranks->names[i].name, "None", 5);
		}
		ranks->points[i].points = list[i].fame; //points
	}
	for (;i < 10; i++) { // In case the MAX is less than 10.
		strncpy(ranks->names[i].name, "Unavailable", 12);
		ranks->points[i].points = 0;
	}
#endif
}

static void clif_ranklist_sub2(uint32 *chars, uint32 *points, enum fame_list_type type)
{
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190724
	nullpo_retv(chars);
	nullpo_retv(points);

	struct fame_list* list;
	switch (type) {
		case RANKTYPE_BLACKSMITH: list = pc->smith_fame_list; break;
		case RANKTYPE_ALCHEMIST:  list = pc->chemist_fame_list; break;
		case RANKTYPE_TAEKWON:    list = pc->taekwon_fame_list; break;
		default: return; // Unsupported
	}

	int i;
	// Packet size limits this list to 10 elements. [Skotlex]
	for (i = 0; i < 10 && i < MAX_FAME_LIST; i++) {
		if (list[i].id > 0) {
			chars[i] = list[i].id;
		} else {
			chars[i] = 0;
		}
		points[i] = list[i].fame; //points
	}
	for (;i < 10; i++) { // In case the MAX is less than 10.
		chars[i] = 0;
		points[i] = 0;
	}
#endif
}

/// 097d <RankingType>.W {<CharName>.24B <point>L}*10 <mypoint>L (ZC_ACK_RANKING)
static void clif_ranklist(struct map_session_data *sd, enum fame_list_type type)
{
#if PACKETVER_MAIN_NUM >= 20130605 || PACKETVER_RE_NUM >= 20130529 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	int fd = sd->fd;
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_ACK_RANKING));
	struct PACKET_ZC_ACK_RANKING *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_ACK_RANKING;
	p->rankType = type;
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190724
PRAGMA_GCC9(GCC diagnostic push)
PRAGMA_GCC9(GCC diagnostic ignored "-Waddress-of-packed-member")
	clif->ranklist_sub2(p->chars, p->points, type);
PRAGMA_GCC9(GCC diagnostic pop)
#else
	clif->ranklist_sub(&p->ranks, type);
#endif

	if (pc->famelist_type(sd->job) == type) {
		p->myPoints = sd->status.fame; //mypoint
	} else {
		p->myPoints = 0; //mypoint
	}

	WFIFOSET(fd, sizeof(struct PACKET_ZC_ACK_RANKING));
#endif
}

static void clif_parse_ranklist(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/*
 *  097c <type> (CZ_REQ_RANKING)
 * */
static void clif_parse_ranklist(int fd, struct map_session_data *sd)
{
	int16 type = RFIFOW(fd, 2); //type

	switch( type ) {
		case RANKTYPE_BLACKSMITH:
		case RANKTYPE_ALCHEMIST:
		case RANKTYPE_TAEKWON:
			clif->ranklist(sd, type); // pk_list unsupported atm
			break;
	}
}

// 097e <RankingType>.W <point>.L <TotalPoint>.L (ZC_UPDATE_RANKING_POINT)
static void clif_update_rankingpoint(struct map_session_data *sd, enum fame_list_type type, int points)
{
#if PACKETVER < 20120502
	switch( type ) {
		case RANKTYPE_BLACKSMITH: clif->fame_blacksmith(sd,points); break;
		case RANKTYPE_ALCHEMIST:  clif->fame_alchemist(sd,points);  break;
		case RANKTYPE_TAEKWON:    clif->fame_taekwon(sd,points);    break;
	}
#else

	int fd;
	int len = packet_len(0x97e);

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x97e;
	WFIFOW(fd, 2) = type;
	WFIFOL(fd, 4) = points;
	WFIFOL(fd, 8) = sd->status.fame;
	WFIFOSET(fd, len);
#endif
}

/// /blacksmith list (ZC_BLACKSMITH_RANK).
/// 0219 { <name>.24B }*10 { <point>.L }*10
static void clif_blacksmith(struct map_session_data *sd)
{
#if !(PACKETVER_MAIN_NUM >= 20130605 || PACKETVER_RE_NUM >= 20130529 || defined(PACKETVER_ZERO))
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x219));
	WFIFOW(fd,0) = 0x219;
	clif->ranklist_sub(WFIFOP(fd, 2), RANKTYPE_BLACKSMITH);
	WFIFOSET(fd, packet_len(0x219));
#endif
}

static void clif_parse_Blacksmith(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /blacksmith (CZ_BLACKSMITH_RANK).
/// 0217
static void clif_parse_Blacksmith(int fd, struct map_session_data *sd)
{
	clif->blacksmith(sd);
}

/// Notification about backsmith points (ZC_BLACKSMITH_POINT).
/// 021b <points>.L <total points>.L
static void clif_fame_blacksmith(struct map_session_data *sd, int points)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x21b));
	WFIFOW(fd,0) = 0x21b;
	WFIFOL(fd,2) = points;
	WFIFOL(fd,6) = sd->status.fame;
	WFIFOSET(fd, packet_len(0x21b));
}

/// /alchemist list (ZC_ALCHEMIST_RANK).
/// 021a { <name>.24B }*10 { <point>.L }*10
static void clif_alchemist(struct map_session_data *sd)
{
#if !(PACKETVER_MAIN_NUM >= 20130605 || PACKETVER_RE_NUM >= 20130529 || defined(PACKETVER_ZERO))
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x21a));
	WFIFOW(fd,0) = 0x21a;
	clif->ranklist_sub(WFIFOP(fd,2), RANKTYPE_ALCHEMIST);
	WFIFOSET(fd, packet_len(0x21a));
#endif
}

static void clif_parse_Alchemist(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /alchemist (CZ_ALCHEMIST_RANK).
/// 0218
static void clif_parse_Alchemist(int fd, struct map_session_data *sd)
{
	clif->alchemist(sd);
}

/// Notification about alchemist points (ZC_ALCHEMIST_POINT).
/// 021c <points>.L <total points>.L
static void clif_fame_alchemist(struct map_session_data *sd, int points)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x21c));
	WFIFOW(fd,0) = 0x21c;
	WFIFOL(fd,2) = points;
	WFIFOL(fd,6) = sd->status.fame;
	WFIFOSET(fd, packet_len(0x21c));
}

/// /taekwon list (ZC_TAEKWON_RANK).
/// 0226 { <name>.24B }*10 { <point>.L }*10
static void clif_taekwon(struct map_session_data *sd)
{
#if !(PACKETVER_MAIN_NUM >= 20130605 || PACKETVER_RE_NUM >= 20130529 || defined(PACKETVER_ZERO))
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x226));
	WFIFOW(fd,0) = 0x226;
	clif->ranklist_sub(WFIFOP(fd,2), RANKTYPE_TAEKWON);
	WFIFOSET(fd, packet_len(0x226));
#endif
}

static void clif_parse_Taekwon(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /taekwon (CZ_TAEKWON_RANK).
/// 0225
static void clif_parse_Taekwon(int fd, struct map_session_data *sd)
{
	clif->taekwon(sd);
}

/// Notification about taekwon points (ZC_TAEKWON_POINT).
/// 0224 <points>.L <total points>.L
static void clif_fame_taekwon(struct map_session_data *sd, int points)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x224));
	WFIFOW(fd,0) = 0x224;
	WFIFOL(fd,2) = points;
	WFIFOL(fd,6) = sd->status.fame;
	WFIFOSET(fd, packet_len(0x224));
}

/// /pk list (ZC_KILLER_RANK).
/// 0238 { <name>.24B }*10 { <point>.L }*10
static void clif_ranking_pk(struct map_session_data *sd)
{
	int i, fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x238));
	WFIFOW(fd,0) = 0x238;
	for (i = 0; i < 10;i ++) {
		strncpy(WFIFOP(fd, i * 24 + 2), "Unknown", NAME_LENGTH);
		WFIFOL(fd,i*4+242) = 0;
	}
	WFIFOSET(fd, packet_len(0x238));
}

static void clif_parse_RankingPk(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /pk (CZ_KILLER_RANK).
/// 0237
static void clif_parse_RankingPk(int fd, struct map_session_data *sd)
{
	clif->ranking_pk(sd);
}

static void clif_parse_FeelSaveOk(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// SG Feel save OK [Komurka] (CZ_AGREE_STARPLACE).
/// 0254 <which>.B
/// which:
///     0 = sun
///     1 = moon
///     2 = star
static void clif_parse_FeelSaveOk(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int i;
	if (sd->menuskill_id != SG_FEEL)
		return;
	i = sd->menuskill_val-1;
	if (i<0 || i >= MAX_PC_FEELHATE) return; //Bug?

	sd->feel_map[i].index = map_id2index(sd->bl.m);
	sd->feel_map[i].m = sd->bl.m;
	pc_setglobalreg(sd,script->add_variable(pc->sg_info[i].feel_var),sd->feel_map[i].index);

#if 0 // Are these really needed? Shouldn't they show up automatically from the feel save packet?
	clif_misceffect2(&sd->bl, 0x1b0);
	clif_misceffect2(&sd->bl, 0x21f);
#endif // 0
	clif->feel_info(sd, i, 0);
	clif_menuskill_clear(sd);
}

/// Star Gladiator's Feeling map confirmation prompt (ZC_STARPLACE).
/// 0253 <which>.B
/// which:
///     0 = sun
///     1 = moon
///     2 = star
///     10 = Do you agree to cast the magic spell that consumes 1 Black Gemstone and 1,000,000 Zeny?
static void clif_feel_req(int fd, struct map_session_data *sd, uint16 skill_lv)
{
	nullpo_retv(sd);
	WFIFOHEAD(fd,packet_len(0x253));
	WFIFOW(fd,0)=0x253;
	WFIFOB(fd,2)=TOB(skill_lv-1);
	WFIFOSET(fd, packet_len(0x253));
	sd->menuskill_id = SG_FEEL;
	sd->menuskill_val = skill_lv;
}

static void clif_parse_ChangeHomunculusName(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change homunculus' name (CZ_RENAME_MER).
/// 0231 <name>.24B
static void clif_parse_ChangeHomunculusName(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	homun->change_name(sd, RFIFOP(fd,2));
}

static void clif_parse_HomMoveToMaster(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to warp/move homunculus/mercenary to it's owner (CZ_REQUEST_MOVETOOWNER).
/// 0234 <id>.L
static void clif_parse_HomMoveToMaster(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int id = RFIFOL(fd,2); // Mercenary or Homunculus
	struct block_list *bl = NULL;
	struct unit_data *ud = NULL;

	if (sd->md && sd->md->bl.id == id)
		bl = &sd->md->bl;
	else if (homun_alive(sd->hd) && sd->hd->bl.id == id)
		bl = &sd->hd->bl; // Moving Homunculus
	else
		return;

	unit->calc_pos(bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	ud = unit->bl2ud(bl);
	unit->walk_toxy(bl, ud->to_x, ud->to_y, 4);
}

static void clif_parse_HomMoveTo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to move homunculus/mercenary (CZ_REQUEST_MOVENPC).
/// 0232 <id>.L <position data>.3B
static void clif_parse_HomMoveTo(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int id = RFIFOL(fd,2); // Mercenary or Homunculus
	struct block_list *bl = NULL;
	short x, y;

	RFIFOPOS(fd, packet_db[RFIFOW(fd,0)].pos[1], &x, &y, NULL);

	if( sd->md && sd->md->bl.id == id )
		bl = &sd->md->bl; // Moving Mercenary
	else if( homun_alive(sd->hd) && sd->hd->bl.id == id )
		bl = &sd->hd->bl; // Moving Homunculus
	else
		return;

	unit->walk_toxy(bl, x, y, 4);
}

static void clif_parse_HomAttack(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to do an action with homunculus/mercenary (CZ_REQUEST_ACTNPC).
/// 0233 <id>.L <target id>.L <action>.B
/// action:
///     always 0
static void clif_parse_HomAttack(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct block_list *bl = NULL;
	const struct PACKET_CZ_REQUEST_ACTNPC *p = RP2PTR(fd);

	if (homun_alive(sd->hd) && sd->hd->bl.id == p->GID)
		bl = &sd->hd->bl;
	else if (sd->md && sd->md->bl.id == p->GID)
		bl = &sd->md->bl;
	else
		return;

	unit->stop_attack(bl);
	unit->attack(bl, p->targetGID, p->action != ACT_ATTACK);
}

static void clif_parse_HomMenu(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke a homunculus menu action (CZ_COMMAND_MER).
/// 022d <type>.W <command>.B
/// type:
///     always 0
/// command:
///     0 = homunculus information
///     1 = feed
///     2 = delete
static void clif_parse_HomMenu(int fd, struct map_session_data *sd)
{ //[orn]
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int cmd;

	cmd = RFIFOW(fd,0);

	if(!homun_alive(sd->hd))
		return;

	homun->menu(sd,RFIFOB(fd,packet_db[cmd].pos[1]));
}

static void clif_parse_AutoRevive(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to resurrect oneself using Token of Siegfried (CZ_STANDING_RESURRECTION).
/// 0292
static void clif_parse_AutoRevive(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isvending(sd))
		return;

	if (!pc_isdead(sd))
		return;

	if (sd->sc.data[SC_HELLPOWER]) //Cannot res while under the effect of SC_HELLPOWER.
		return;

	int item_position = pc->have_item_chain(sd, ECC_SIEGFRIED);
	int hpsp = 100;

	if (item_position == INDEX_NOT_FOUND) {
		if (sd->sc.data[SC_LIGHT_OF_REGENE])
			hpsp = 20 * sd->sc.data[SC_LIGHT_OF_REGENE]->val1;
		else
			return;
	}

	if (!status->revive(&sd->bl, hpsp, hpsp))
		return;

	if (item_position == INDEX_NOT_FOUND)
		status_change_end(&sd->bl, SC_LIGHT_OF_REGENE, INVALID_TIMER);
	else
		pc->delitem(sd, item_position, 1, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);

	clif->skill_nodamage(&sd->bl, &sd->bl, ALL_RESURRECTION, 4, 1);
}

/// Information about character's status values (ZC_ACK_STATUS_GM).
/// 0214 <str>.B <standardStr>.B <agi>.B <standardAgi>.B <vit>.B <standardVit>.B
///      <int>.B <standardInt>.B <dex>.B <standardDex>.B <luk>.B <standardLuk>.B
///      <attPower>.W <refiningPower>.W <max_mattPower>.W <min_mattPower>.W
///      <itemdefPower>.W <plusdefPower>.W <mdefPower>.W <plusmdefPower>.W
///      <hitSuccessValue>.W <avoidSuccessValue>.W <plusAvoidSuccessValue>.W
///      <criticalSuccessValue>.W <ASPD>.W <plusASPD>.W
static void clif_check(int fd, struct map_session_data *pl_sd)
{
	nullpo_retv(pl_sd);
	WFIFOHEAD(fd,packet_len(0x214));
	WFIFOW(fd, 0) = 0x214;
	WFIFOB(fd, 2) = min(pl_sd->status.str, UINT8_MAX);
	WFIFOB(fd, 3) = pc->need_status_point(pl_sd, SP_STR, 1);
	WFIFOB(fd, 4) = min(pl_sd->status.agi, UINT8_MAX);
	WFIFOB(fd, 5) = pc->need_status_point(pl_sd, SP_AGI, 1);
	WFIFOB(fd, 6) = min(pl_sd->status.vit, UINT8_MAX);
	WFIFOB(fd, 7) = pc->need_status_point(pl_sd, SP_VIT, 1);
	WFIFOB(fd, 8) = min(pl_sd->status.int_, UINT8_MAX);
	WFIFOB(fd, 9) = pc->need_status_point(pl_sd, SP_INT, 1);
	WFIFOB(fd,10) = min(pl_sd->status.dex, UINT8_MAX);
	WFIFOB(fd,11) = pc->need_status_point(pl_sd, SP_DEX, 1);
	WFIFOB(fd,12) = min(pl_sd->status.luk, UINT8_MAX);
	WFIFOB(fd,13) = pc->need_status_point(pl_sd, SP_LUK, 1);
	WFIFOW(fd,14) = pl_sd->battle_status.batk+pl_sd->battle_status.rhw.atk+pl_sd->battle_status.lhw.atk;
	WFIFOW(fd,16) = pl_sd->battle_status.rhw.atk2+pl_sd->battle_status.lhw.atk2;
	WFIFOW(fd,18) = pl_sd->battle_status.matk_max;
	WFIFOW(fd,20) = pl_sd->battle_status.matk_min;
	WFIFOW(fd,22) = pl_sd->battle_status.def;
	WFIFOW(fd,24) = pl_sd->battle_status.def2;
	WFIFOW(fd,26) = pl_sd->battle_status.mdef;
	WFIFOW(fd,28) = pl_sd->battle_status.mdef2;
	WFIFOW(fd,30) = pl_sd->battle_status.hit;
	WFIFOW(fd,32) = pl_sd->battle_status.flee;
	WFIFOW(fd,34) = pl_sd->battle_status.flee2/10;
	WFIFOW(fd,36) = pl_sd->battle_status.cri/10;
	WFIFOW(fd,38) = (2000-pl_sd->battle_status.amotion)/10;  // aspd
	WFIFOW(fd,40) = 0;  // FIXME: What is 'plusASPD' supposed to be? Maybe a delay?
	WFIFOSET(fd,packet_len(0x214));
}

static void clif_parse_Check(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// /check (CZ_REQ_STATUS_GM).
/// Request character's status values.
/// 0213 <char name>.24B
static void clif_parse_Check(int fd, struct map_session_data *sd)
{
	if (pc_istrading(sd) || pc_isvending(sd))
		return;

	char charname[NAME_LENGTH];
	struct map_session_data* pl_sd;

	if(!pc_has_permission(sd, PC_PERM_USE_CHECK))
		return;

	safestrncpy(charname, RFIFOP(fd,packet_db[RFIFOW(fd,0)].pos[0]), sizeof(charname));

	if ((pl_sd = map->nick2sd(charname, true)) == NULL || pc_get_group_level(sd) < pc_get_group_level(pl_sd)) {
		return;
	}

	clif->check(fd, pl_sd);
}

/// MAIL SYSTEM
/// By Zephyrus
///

/// Notification about the result of adding an item to mail (ZC_ACK_MAIL_ADD_ITEM).
/// 0255 <index>.W <result>.B
/// result:
///     0 = success
///     1 = failure
static void clif_Mail_setattachment(int fd, int index, uint8 flag)
{
	WFIFOHEAD(fd,packet_len(0x255));
	WFIFOW(fd,0) = 0x255;
	WFIFOW(fd,2) = index;
	WFIFOB(fd,4) = flag;
	WFIFOSET(fd,packet_len(0x255));
}

/// Notification about the result of retrieving a mail attachment (ZC_MAIL_REQ_GET_ITEM).
/// 0245 <result>.B
/// result:
///     0 = success
///     1 = failure
///     2 = too many items
static void clif_Mail_getattachment(int fd, uint8 flag)
{
	WFIFOHEAD(fd,packet_len(0x245));
	WFIFOW(fd,0) = 0x245;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x245));
}

/// Notification about the result of sending a mail (ZC_MAIL_REQ_SEND).
/// 0249 <result>.B
/// result:
///     0 = success
///     1 = recipient does not exist
static void clif_Mail_send(int fd, bool fail)
{
	WFIFOHEAD(fd,packet_len(0x249));
	WFIFOW(fd,0) = 0x249;
	WFIFOB(fd,2) = fail;
	WFIFOSET(fd,packet_len(0x249));
}

/// Notification about the result of deleting a mail (ZC_ACK_MAIL_DELETE).
/// 0257 <mail id>.L <result>.W
/// result:
///     0 = success
///     1 = failure
static void clif_Mail_delete(int fd, int mail_id, short fail)
{
	WFIFOHEAD(fd, packet_len(0x257));
	WFIFOW(fd,0) = 0x257;
	WFIFOL(fd,2) = mail_id;
	WFIFOW(fd,6) = fail;
	WFIFOSET(fd, packet_len(0x257));
}

/// Notification about the result of returning a mail (ZC_ACK_MAIL_RETURN).
/// 0274 <mail id>.L <result>.W
/// result:
///     0 = success
///     1 = failure
static void clif_Mail_return(int fd, int mail_id, short fail)
{
	WFIFOHEAD(fd,packet_len(0x274));
	WFIFOW(fd,0) = 0x274;
	WFIFOL(fd,2) = mail_id;
	WFIFOW(fd,6) = fail;
	WFIFOSET(fd,packet_len(0x274));
}

/// Notification about new mail (ZC_MAIL_RECEIVE).
/// 024a <mail id>.L <title>.40B <sender>.24B
static void clif_Mail_new(int fd, int mail_id, const char *sender, const char *title)
{
	nullpo_retv(sender);
	nullpo_retv(title);
	WFIFOHEAD(fd,packet_len(0x24a));
	WFIFOW(fd,0) = 0x24a;
	WFIFOL(fd,2) = mail_id;
	safestrncpy(WFIFOP(fd,6), title, MAIL_TITLE_LENGTH);
	safestrncpy(WFIFOP(fd,46), sender, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x24a));
}

/// Opens/closes the mail window (ZC_MAIL_WINDOWS).
/// 0260 <type>.L
/// type:
///     0 = open
///     1 = close
static void clif_Mail_window(int fd, int flag)
{
	WFIFOHEAD(fd,packet_len(0x260));
	WFIFOW(fd,0) = 0x260;
	WFIFOL(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x260));
}

/// Lists mails stored in inbox (ZC_MAIL_REQ_GET_LIST).
/// 0240 <packet len>.W <amount>.L { <mail id>.L <title>.40B <read>.B <sender>.24B <time>.L }*amount
/// read:
///     0 = unread
///     1 = read
static void clif_Mail_refreshinbox(struct map_session_data *sd)
{
	int fd = sd->fd;
	struct mail_data *md;
	struct mail_message *msg;
	int len, i, j;

	nullpo_retv(sd);
	md = &sd->mail.inbox;
	len = 8 + (73 * md->amount);

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x240;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = md->amount;
	for( i = j = 0; i < MAIL_MAX_INBOX && j < md->amount; i++ )
	{
		msg = &md->msg[i];
		if (msg->id < 1)
			continue;

		WFIFOL(fd,8+73*j) = msg->id;
		memcpy(WFIFOP(fd,12+73*j), msg->title, MAIL_TITLE_LENGTH);
		WFIFOB(fd,52+73*j) = (msg->status != MAIL_UNREAD);
		memcpy(WFIFOP(fd,53+73*j), msg->send_name, NAME_LENGTH);
		WFIFOL(fd,77+73*j) = (uint32)msg->timestamp;
		j++;
	}
	WFIFOSET(fd,len);

	if( md->full ) {// TODO: is this official?
		char output[100];
		sprintf(output, msg_sd(sd, 511), MAIL_MAX_INBOX); // Inbox is full (Max %d). Delete some mails.
		clif_disp_onlyself(sd, output);
	}
}

static void clif_parse_Mail_refreshinbox(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Mail inbox list request (CZ_MAIL_GET_LIST).
/// 023f
static void clif_parse_Mail_refreshinbox(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct mail_data* md = &sd->mail.inbox;

	if( md->amount < MAIL_MAX_INBOX && (md->full || sd->mail.changed) )
		intif->Mail_requestinbox(sd->status.char_id, 1);
	else
		clif->mail_refreshinbox(sd);

	mail->removeitem(sd, 0);
	mail->removezeny(sd, 0);
}

/// Opens a mail (ZC_MAIL_REQ_OPEN).
/// 0242 <packet len>.W <mail id>.L <title>.40B <sender>.24B <time>.L <zeny>.L
///     <amount>.L <name id>.W <item type>.W <identified>.B <damaged>.B <refine>.B
///     <card1>.W <card2>.W <card3>.W <card4>.W <message>.?B
static void clif_Mail_read(struct map_session_data *sd, int mail_id)
{
	int i, fd = sd->fd;

	nullpo_retv(sd);
	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if( i == MAIL_MAX_INBOX ) {
		clif->mail_return(sd->fd, mail_id, 1); // Mail doesn't exist
		ShowWarning("clif_parse_Mail_read: char '%s' trying to read a message not the inbox.\n", sd->status.name);
		return;
	} else {
		struct mail_message *msg = &sd->mail.inbox.msg[i];
		struct item *item = &msg->item;
		struct item_data *data;
		int msg_len = (int)strlen(msg->body), len;

		if (msg_len == 0) {
			strcpy(msg->body, "(no message)");
			msg_len = (int)strlen(msg->body);
		}

		if (msg_len > UINT8_MAX) {
			Assert_report(msg_len > UINT8_MAX);
			msg_len = UINT8_MAX;
		}

		len = 101 + msg_len;

		WFIFOHEAD(fd,len);
		WFIFOW(fd,0) = 0x242;
		WFIFOW(fd,2) = len;
		WFIFOL(fd,4) = msg->id;
		safestrncpy(WFIFOP(fd,8), msg->title, MAIL_TITLE_LENGTH + 1);
		safestrncpy(WFIFOP(fd,48), msg->send_name, NAME_LENGTH + 1);
		WFIFOL(fd,72) = 0;
		WFIFOL(fd,76) = msg->zeny;

		if( item->nameid && (data = itemdb->exists(item->nameid)) != NULL ) {
			WFIFOL(fd,80) = item->amount;
			WFIFOW(fd,84) = (data->view_id)?data->view_id:item->nameid;
			WFIFOW(fd,86) = data->type;
			WFIFOB(fd,88) = item->identify;
			WFIFOB(fd,89) = item->attribute;
			WFIFOB(fd,90) = item->refine;
			WFIFOW(fd,91) = item->card[0];
			WFIFOW(fd,93) = item->card[1];
			WFIFOW(fd,95) = item->card[2];
			WFIFOW(fd,97) = item->card[3];
		} else // no item, set all to zero
			memset(WFIFOP(fd,80), 0x00, 19);

		WFIFOB(fd,99) = (uint8)msg_len;
		safestrncpy(WFIFOP(fd,100), msg->body, msg_len + 1);
		WFIFOSET(fd,len);

		if (msg->status == MAIL_UNREAD) {
			msg->status = MAIL_READ;
			intif->Mail_read(mail_id);
			clif->pMail_refreshinbox(fd, sd);
		}
	}
}

static void clif_parse_Mail_read(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to open a mail (CZ_MAIL_OPEN).
/// 0241 <mail id>.L
static void clif_parse_Mail_read(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int mail_id = RFIFOL(fd,2);

	if( mail_id <= 0 )
		return;
	if( mail->invalid_operation(sd) )
		return;

	clif->mail_read(sd, RFIFOL(fd,2));
}

static void clif_parse_Mail_getattach(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to receive mail's attachment (CZ_MAIL_GET_ITEM).
/// 0244 <mail id>.L
static void clif_parse_Mail_getattach(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int mail_id = RFIFOL(fd,2);
	int i;

	if( !chrif->isconnected() )
		return;
	if( mail_id <= 0 )
		return;
	if( mail->invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if( i == MAIL_MAX_INBOX )
		return;

	if( sd->mail.inbox.msg[i].zeny < 1 && (sd->mail.inbox.msg[i].item.nameid < 1 || sd->mail.inbox.msg[i].item.amount < 1) )
		return;

	if( sd->mail.inbox.msg[i].zeny > MAX_ZENY - sd->status.zeny ) {
		clif->mail_getattachment(fd, 1);
		return;
	}

	if( sd->mail.inbox.msg[i].item.nameid > 0 ) {
		struct item_data *data;
		unsigned int weight;
		bool fail = false;

		if ((data = itemdb->exists(sd->mail.inbox.msg[i].item.nameid)) == NULL)
			return;

		if( pc_is90overweight(sd) ) {
			clif->mail_getattachment(fd, 2);
			return;
		}

		switch( pc->checkadditem(sd, data->nameid, sd->mail.inbox.msg[i].item.amount) ) {
			case ADDITEM_NEW:
				fail = ( pc->inventoryblank(sd) == 0 );
				break;
			case ADDITEM_OVERAMOUNT:
				fail = true;
		}

		if( fail ) {
			clif->mail_getattachment(fd, 1);
			return;
		}

		weight = data->weight * sd->mail.inbox.msg[i].item.amount;
		if( sd->weight + weight > sd->max_weight ) {
			clif->mail_getattachment(fd, 2);
			return;
		}
	}

	sd->mail.inbox.msg[i].zeny = 0;
	memset(&sd->mail.inbox.msg[i].item, 0, sizeof(struct item));
	mail->clear(sd);

	clif->mail_read(sd, mail_id);
	intif->Mail_getattach(sd->status.char_id, mail_id);
}

static void clif_parse_Mail_delete(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to delete a mail (CZ_MAIL_DELETE).
/// 0243 <mail id>.L
static void clif_parse_Mail_delete(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int mail_id = RFIFOL(fd,2);
	int i;

	if( !chrif->isconnected() )
		return;
	if( mail_id <= 0 )
		return;
	if( mail->invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if (i < MAIL_MAX_INBOX) {
		struct mail_message *msg = &sd->mail.inbox.msg[i];

		if( (msg->item.nameid > 0 && msg->item.amount > 0) || msg->zeny > 0 ) {// can't delete mail without removing attachment first
			clif->mail_delete(sd->fd, mail_id, 1);
			return;
		}

		sd->mail.inbox.msg[i].zeny = 0;
		memset(&sd->mail.inbox.msg[i].item, 0, sizeof(struct item));
		mail->clear(sd);
		intif->Mail_delete(sd->status.char_id, mail_id);
	}
}

static void clif_parse_Mail_return(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to return a mail (CZ_REQ_MAIL_RETURN).
/// 0273 <mail id>.L <receive name>.24B
static void clif_parse_Mail_return(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int mail_id = RFIFOL(fd,2);
	int i;

	if( mail_id <= 0 )
		return;
	if( mail->invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if (i < MAIL_MAX_INBOX && sd->mail.inbox.msg[i].send_id != 0) {
		sd->mail.inbox.msg[i].zeny = 0;
		memset(&sd->mail.inbox.msg[i].item, 0, sizeof(struct item));
		mail->clear(sd);
		intif->Mail_return(sd->status.char_id, mail_id);
	} else {
		clif->mail_return(sd->fd, mail_id, 1);
	}
}

static void clif_parse_Mail_setattach(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add an item or Zeny to mail (CZ_MAIL_ADD_ITEM).
/// 0247 <index>.W <amount>.L
static void clif_parse_Mail_setattach(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int idx = RFIFOW(fd,2);
	int amount = RFIFOL(fd,4);
	unsigned char flag;

	if( !chrif->isconnected() )
		return;
	if (idx < 0 || amount < 0)
		return;

	flag = mail->setitem(sd, idx, amount);
	clif->mail_setattachment(fd,idx,flag);
}

static void clif_parse_Mail_winopen(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to reset mail item and/or Zeny (CZ_MAIL_RESET_ITEM).
/// 0246 <type>.W
/// type:
///     0 = reset all
///     1 = remove item
///     2 = remove zeny
static void clif_parse_Mail_winopen(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int flag = RFIFOW(fd,2);

	if (flag == 0 || flag == 1)
		mail->removeitem(sd, 0);
	if (flag == 0 || flag == 2)
		mail->removezeny(sd, 0);
}

static void clif_parse_Mail_send(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to send mail (CZ_MAIL_SEND).
/// 0248 <packet len>.W <recipient>.24B <title>.40B <body len>.B <body>.?B

static void clif_parse_Mail_send(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct mail_message msg;
	int body_len;
	int len = RFIFOW(fd, 2);

	if( !chrif->isconnected() )
		return;
	if( sd->state.trading )
		return;

	if (len < 69) {
		ShowWarning("Invalid Msg Len from account %d.\n", sd->status.account_id);
		return;
	}

	if( DIFF_TICK(sd->cansendmail_tick, timer->gettick()) > 0 ) {
		clif->message(sd->fd,msg_sd(sd,875)); //"Cannot send mails too fast!!."
		clif->mail_send(fd, true); // fail
		return;
	}

	body_len = RFIFOB(fd,68);

	if (body_len > MAIL_BODY_LENGTH)
		body_len = MAIL_BODY_LENGTH;

	if (body_len + 69 > len) {
		ShowWarning("Invalid Msg Len from account %d.\n", sd->status.account_id);
		return;
	}

	memset(&msg, 0, sizeof(msg));
	if (!mail->setattachment(sd, &msg)) { // Invalid Append condition
		clif->mail_send(sd->fd, true); // fail
		mail->removeitem(sd,0);
		mail->removezeny(sd,0);
		return;
	}

	msg.id = 0; // id will be assigned by charserver
	msg.send_id = sd->status.char_id;
	msg.dest_id = 0; // will attempt to resolve name
	safestrncpy(msg.send_name, sd->status.name, NAME_LENGTH);
	safestrncpy(msg.dest_name, RFIFOP(fd,4), NAME_LENGTH);
	safestrncpy(msg.title, RFIFOP(fd,28), MAIL_TITLE_LENGTH);

	if (msg.title[0] == '\0') {
		return; // Message has no length and somehow client verification was skipped.
	}

	if (body_len)
		safestrncpy(msg.body, RFIFOP(fd,69), body_len + 1);
	else
		memset(msg.body, 0x00, MAIL_BODY_LENGTH);

	msg.timestamp = time(NULL);
	if( !intif->Mail_send(sd->status.account_id, &msg) )
		mail->deliveryfail(sd, &msg);

	sd->cansendmail_tick = timer->gettick() + 1000; // 1 Second flood Protection
}

/// AUCTION SYSTEM
/// By Zephyrus
///

/// Opens/closes the auction window (ZC_AUCTION_WINDOWS).
/// 025f <type>.L
/// type:
///     0 = open
///     1 = close
static void clif_Auction_openwindow(struct map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	if (sd->state.storage_flag != STORAGE_FLAG_CLOSED || sd->state.vending || sd->state.prevend || sd->state.buyingstore || sd->state.trading)
		return;

	if( !battle_config.feature_auction )
		return;

	WFIFOHEAD(fd,packet_len(0x25f));
	WFIFOW(fd,0) = 0x25f;
	WFIFOL(fd,2) = 0;
	WFIFOSET(fd,packet_len(0x25f));
}

/// Returns auction item search results (ZC_AUCTION_ITEM_REQ_SEARCH).
/// 0252 <packet len>.W <pages>.L <count>.L { <auction id>.L <seller name>.24B <name id>.W <type>.L <amount>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <now price>.L <max price>.L <buyer name>.24B <delete time>.L }*
static void clif_Auction_results(struct map_session_data *sd, short count, short pages, const uint8 *buf)
{
	int i, fd, len = sizeof(struct auction_data);
	struct auction_data auction;
	struct item_data *item;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,12 + (count * 83));
	WFIFOW(fd,0) = 0x252;
	WFIFOW(fd,2) = 12 + (count * 83);
	WFIFOL(fd,4) = pages;
	WFIFOL(fd,8) = count;

	for( i = 0; i < count; i++ ) {
		int k = 12 + (i * 83);
		memcpy(&auction, RBUFP(buf,i * len), len);

		WFIFOL(fd,k) = auction.auction_id;
		safestrncpy(WFIFOP(fd,4+k), auction.seller_name, NAME_LENGTH);

		if( (item = itemdb->exists(auction.item.nameid)) != NULL && item->view_id > 0 )
			WFIFOW(fd,28+k) = item->view_id;
		else
			WFIFOW(fd,28+k) = auction.item.nameid;

		WFIFOL(fd,30+k) = auction.type;
		WFIFOW(fd,34+k) = auction.item.amount; // Always 1
		WFIFOB(fd,36+k) = auction.item.identify;
		WFIFOB(fd,37+k) = auction.item.attribute;
		WFIFOB(fd,38+k) = auction.item.refine;
		WFIFOW(fd,39+k) = auction.item.card[0];
		WFIFOW(fd,41+k) = auction.item.card[1];
		WFIFOW(fd,43+k) = auction.item.card[2];
		WFIFOW(fd,45+k) = auction.item.card[3];
		WFIFOL(fd,47+k) = auction.price;
		WFIFOL(fd,51+k) = auction.buynow;
		safestrncpy(WFIFOP(fd,55+k), auction.buyer_name, NAME_LENGTH);
		WFIFOL(fd,79+k) = (uint32)auction.timestamp;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
}

/// Result from request to add an item (ZC_ACK_AUCTION_ADD_ITEM).
/// 0256 <index>.W <result>.B
/// result:
///     0 = success
///     1 = failure
static void clif_Auction_setitem(int fd, int index, bool fail)
{
	WFIFOHEAD(fd,packet_len(0x256));
	WFIFOW(fd,0) = 0x256;
	WFIFOW(fd,2) = index;
	WFIFOB(fd,4) = fail;
	WFIFOSET(fd,packet_len(0x256));
}

static void clif_parse_Auction_cancelreg(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to initialize 'new auction' data (CZ_AUCTION_CREATE).
/// 024b <type>.W
/// type:
///     0 = create (any other action in auction window)
///     1 = cancel (cancel pressed on register tab)
///     ? = junk, uninitialized value (ex. when switching between list filters)
static void clif_parse_Auction_cancelreg(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (sd->auction.amount > 0)
		clif->additem(sd, sd->auction.index, sd->auction.amount, 0);

	sd->auction.amount = 0;
}

static void clif_parse_Auction_setitem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add an item to the action (CZ_AUCTION_ADD_ITEM).
/// 024c <index>.W <count>.L
static void clif_parse_Auction_setitem(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int idx = RFIFOW(fd, 2) - 2;
	int amount = RFIFOL(fd, 4); // Always 1
	struct item_data *item;

	if( !battle_config.feature_auction )
		return;

	if( sd->auction.amount > 0 )
		sd->auction.amount = 0;

	if (idx < 0 || idx >= sd->status.inventorySize) {
		ShowWarning("Character %s trying to set invalid item index in auctions.\n", sd->status.name);
		return;
	}

	if( amount != 1 || amount > sd->status.inventory[idx].amount ) { // By client, amount is always set to 1. Maybe this is a future implementation.
		ShowWarning("Character %s trying to set invalid amount in auctions.\n", sd->status.name);
		return;
	}

	if( (item = itemdb->exists(sd->status.inventory[idx].nameid)) != NULL && !(item->type == IT_ARMOR || item->type == IT_PETARMOR || item->type == IT_WEAPON || item->type == IT_CARD || item->type == IT_ETC) )
	{ // Consumable or pets are not allowed
		clif->auction_setitem(sd->fd, idx, true);
		return;
	}

	if( !pc_can_give_items(sd) || sd->status.inventory[idx].expire_time ||
			!sd->status.inventory[idx].identify ||
				!itemdb_canauction(&sd->status.inventory[idx],pc_get_group_level(sd)) || // Quest Item or something else
					(sd->status.inventory[idx].bound && !pc_can_give_bound_items(sd)) ) {
		clif->auction_setitem(sd->fd, idx, true);
		return;
	}

	sd->auction.index = idx;
	sd->auction.amount = amount;
	clif->auction_setitem(fd, idx + 2, false);
}

/// Result from an auction action (ZC_AUCTION_RESULT).
/// 0250 <result>.B
/// result:
///     0 = You have failed to bid into the auction
///     1 = You have successfully bid in the auction
///     2 = The auction has been canceled
///     3 = An auction with at least one bidder cannot be canceled
///     4 = You cannot register more than 5 items in an auction at a time
///     5 = You do not have enough Zeny to pay the Auction Fee
///     6 = You have won the auction
///     7 = You have failed to win the auction
///     8 = You do not have enough Zeny
///     9 = You cannot place more than 5 bids at a time
static void clif_Auction_message(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,packet_len(0x250));
	WFIFOW(fd,0) = 0x250;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x250));
}

/// Result of the auction close request (ZC_AUCTION_ACK_MY_SELL_STOP).
/// 025e <result>.W
/// result:
///     0 = You have ended the auction
///     1 = You cannot end the auction
///     2 = Auction ID is incorrect
static void clif_Auction_close(int fd, unsigned char flag)
{
	WFIFOHEAD(fd, 4);
	WFIFOW(fd,0) = 0x25d;  // BUG: The client identifies this packet as 0x25d (CZ_AUCTION_REQ_MY_SELL_STOP)
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd, 4);
}

static void clif_parse_Auction_register(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to add an auction (CZ_AUCTION_ADD).
/// 024d <now money>.L <max money>.L <delete hour>.W
static void clif_parse_Auction_register(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct auction_data auction;
	struct item_data *item;

	if (!battle_config.feature_auction)
		return;

	Assert_retv(sd->auction.index >= 0 && sd->auction.index < sd->status.inventorySize);

	memset(&auction, 0, sizeof(auction));
	auction.price = RFIFOL(fd,2);
	auction.buynow = RFIFOL(fd,6);
	auction.hours = RFIFOW(fd,10);

	// Invalid Situations...
	if (auction.price <= 0 || auction.buynow <= 0) {
		ShowWarning("Character %s trying to register auction wit wrong price.\n", sd->status.name);
		return;
	}

	if( sd->auction.amount < 1 ) {
		ShowWarning("Character %s trying to register auction without item.\n", sd->status.name);
		return;
	}

	if( auction.price >= auction.buynow ) {
		ShowWarning("Character %s trying to alter auction prices.\n", sd->status.name);
		return;
	}

	if( auction.hours < 1 || auction.hours > 48 ) {
		ShowWarning("Character %s trying to enter an invalid time for auction.\n", sd->status.name);
		return;
	}

	if( sd->status.zeny < (auction.hours * battle_config.auction_feeperhour) ) {
		clif_Auction_message(fd, 5); // You do not have enough zeny to pay the Auction Fee.
		return;
	}

	if( auction.buynow > battle_config.auction_maximumprice )
	{ // Zeny Limits
		auction.buynow = battle_config.auction_maximumprice;
		if( auction.price >= auction.buynow )
			auction.price = auction.buynow - 1;
	}

	auction.auction_id = 0;
	auction.seller_id = sd->status.char_id;
	safestrncpy(auction.seller_name, sd->status.name, sizeof(auction.seller_name));
	auction.buyer_id = 0;
	memset(auction.buyer_name, '\0', sizeof(auction.buyer_name));

	if( sd->status.inventory[sd->auction.index].nameid == 0 || sd->status.inventory[sd->auction.index].amount < sd->auction.amount )
	{
		clif->auction_message(fd, 2); // The auction has been canceled
		return;
	}

	if( (item = itemdb->exists(sd->status.inventory[sd->auction.index].nameid)) == NULL )
	{ // Just in case
		clif->auction_message(fd, 2); // The auction has been canceled
		return;
	}

	// Auction checks...
	if( sd->status.inventory[sd->auction.index].bound && !pc_can_give_bound_items(sd) ) {
		clif->message(sd->fd, msg_sd(sd,293)); // This bound item cannot be traded to that character.
		clif->auction_message(fd, 2); // The auction has been canceled
		return;
	}

	safestrncpy(auction.item_name, item->jname, sizeof(auction.item_name));
	auction.type = item->type;
	memcpy(&auction.item, &sd->status.inventory[sd->auction.index], sizeof(struct item));
	auction.item.amount = 1;
	auction.timestamp = 0;

	if( !intif->Auction_register(&auction) )
		clif->auction_message(fd, 4); // No Char Server? lets say something to the client
	else
	{
		int zeny = auction.hours*battle_config.auction_feeperhour;

		pc->delitem(sd, sd->auction.index, sd->auction.amount, 1, DELITEM_SOLD, LOG_TYPE_AUCTION);
		sd->auction.amount = 0;

		pc->payzeny(sd, zeny, LOG_TYPE_AUCTION, NULL);
	}
}

static void clif_parse_Auction_cancel(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Cancels an auction (CZ_AUCTION_ADD_CANCEL).
/// 024e <auction id>.L
static void clif_parse_Auction_cancel(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	unsigned int auction_id = RFIFOL(fd, 2);

	intif->Auction_cancel(sd->status.char_id, auction_id);
}

static void clif_parse_Auction_close(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Closes an auction (CZ_AUCTION_REQ_MY_SELL_STOP).
/// 025d <auction id>.L
static void clif_parse_Auction_close(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	unsigned int auction_id = RFIFOL(fd,2);

	intif->Auction_close(sd->status.char_id, auction_id);
}

static void clif_parse_Auction_bid(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Places a bid on an auction (CZ_AUCTION_BUY).
/// 024f <auction id>.L <money>.L
static void clif_parse_Auction_bid(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	unsigned int auction_id = RFIFOL(fd, 2);
	int bid = RFIFOL(fd, 6);

	if( !pc_can_give_items(sd) ) { //They aren't supposed to give zeny [Inkfish]
		clif->message(sd->fd, msg_sd(sd,246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	if( bid <= 0 )
		clif->auction_message(fd, 0); // You have failed to bid into the auction
	else if( bid > sd->status.zeny )
		clif->auction_message(fd, 8); // You do not have enough zeny
	else if ( intif->CheckForCharServer() ) // char server is down (bugreport:1138)
		clif->auction_message(fd, 0); // You have failed to bid into the auction
	else {
		pc->payzeny(sd, bid, LOG_TYPE_AUCTION, NULL);
		intif->Auction_bid(sd->status.char_id, sd->status.name, auction_id, bid);
	}
}

static void clif_parse_Auction_search(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Auction Search (CZ_AUCTION_ITEM_SEARCH).
/// 0251 <search type>.W <auction id>.L <search text>.24B <page number>.W
/// search type:
///     0 = armor
///     1 = weapon
///     2 = card
///     3 = misc
///     4 = name search
///     5 = auction id search
static void clif_parse_Auction_search(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	char search_text[NAME_LENGTH];
	short type = RFIFOW(fd, 2), page = RFIFOW(fd, 32);
	int price = RFIFOL(fd, 4);  // FIXME: bug #5071

	if( !battle_config.feature_auction )
		return;

	clif->pAuction_cancelreg(fd, sd);

	safestrncpy(search_text, RFIFOP(fd,8), sizeof(search_text));
	intif->Auction_requestlist(sd->status.char_id, type, price, search_text, page);
}

static void clif_parse_Auction_buysell(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Requests list of own currently active bids or auctions (CZ_AUCTION_REQ_MY_INFO).
/// 025c <type>.W
/// type:
///     0 = sell (own auctions)
///     1 = buy (own bids)
static void clif_parse_Auction_buysell(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	short type = RFIFOW(fd, 2) + 6;

	if( !battle_config.feature_auction )
		return;

	clif->pAuction_cancelreg(fd, sd);

	intif->Auction_requestlist(sd->status.char_id, type, 0, "", 1);
}

/// CASH/POINT SHOP
///

/// List of items offered in a cash shop (ZC_PC_CASH_POINT_ITEMLIST).
/// 0287 <packet len>.W <cash point>.L { <sell price>.L <discount price>.L <item type>.B <name id>.W }*
/// 0287 <packet len>.W <cash point>.L <kafra point>.L { <sell price>.L <discount price>.L <item type>.B <name id>.W }* (PACKETVER >= 20070711)
static void clif_cashshop_show(struct map_session_data *sd, struct npc_data *nd)
{
	struct npc_item_list *shop = NULL;
	unsigned short shop_size = 0;
	int fd, i, c = 0;
	int currency[2] = { 0,0 };
	int len;
	struct PACKET_ZC_PC_CASH_POINT_ITEMLIST *p;

	nullpo_retv(sd);
	nullpo_retv(nd);

	if( nd->subtype == SCRIPT ) {
		shop = nd->u.scr.shop->item;
		shop_size = nd->u.scr.shop->items;

		npc->trader_count_funds(nd, sd);

		currency[0] = npc->trader_funds[0];
		currency[1] = npc->trader_funds[1];
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;

		currency[0] = sd->cashPoints;
		currency[1] = sd->kafraPoints;
	}

	fd = sd->fd;
	sd->npc_shopid = nd->bl.id;
	len = sizeof(struct PACKET_ZC_PC_CASH_POINT_ITEMLIST) + shop_size * sizeof(struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x287;
	p->cashPoints = currency[0];
#if PACKETVER >= 20070711
	p->kafraPoints = currency[1];
#endif

	for (i = 0; i < shop_size; i++) {
		if (shop[i].nameid) {
			struct item_data* id = itemdb->search(shop[i].nameid);
			p->items[c].price = shop[i].value;
			p->items[c].discountPrice = shop[i].value;
			p->items[c].itemType = itemtype(id->type);
			p->items[c].itemId = (id->view_id > 0) ? id->view_id : id->nameid;
			c++;
		}
	}
	len = sizeof(struct PACKET_ZC_PC_CASH_POINT_ITEMLIST) + c * sizeof(struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub);
	p->packetLength = len;
	WFIFOSET(fd, len);
}

/// Cashshop Buy Ack (ZC_PC_CASH_POINT_UPDATE).
/// 0289 <cash point>.L <error>.W
/// 0289 <cash point>.L <kafra point>.L <error>.W (PACKETVER >= 20070711)
/// For error return codes see enum cashshop_error@clif.h
static void clif_cashshop_ack(struct map_session_data *sd, int error)
{
	struct npc_data *nd;
	int fd;
	int currency[2] = { 0,0 };

	nullpo_retv(sd);
	fd = sd->fd;
	if( (nd = map->id2nd(sd->npc_shopid)) && nd->subtype == SCRIPT ) {
		npc->trader_count_funds(nd,sd);
		currency[0] = npc->trader_funds[0];
		currency[1] = npc->trader_funds[1];
	} else {
		currency[0] = sd->cashPoints;
		currency[1] = sd->kafraPoints;
	}

	WFIFOHEAD(fd, packet_len(0x289));
	WFIFOW(fd,0) = 0x289;
	WFIFOL(fd,2) = currency[0];
#if PACKETVER < 20070711
	WFIFOW(fd,6) = TOW(error);
#else
	WFIFOL(fd,6) = currency[1];
	WFIFOW(fd,10) = TOW(error);
#endif
	WFIFOSET(fd, packet_len(0x289));
}

static void clif_parse_cashshop_buy(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to buy item(s) from cash shop (CZ_PC_BUY_CASH_POINT_ITEM).
/// 0288 <name id>.W <amount>.W
/// 0288 <name id>.W <amount>.W <kafra points>.L (PACKETVER >= 20070711)
/// 0288 <packet len>.W <kafra points>.L <count>.W { <amount>.W <name id>.W }.4B*count (PACKETVER >= 20100803)
static void clif_parse_cashshop_buy(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int fail = 0;
	const struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM *p = RFIFOP(fd, 0);

	if (sd->state.trading || !sd->npc_shopid || pc_has_permission(sd,PC_PERM_DISABLE_STORE)) {
		fail = 1;
	} else {
#if PACKETVER < 20070711
		fail = npc->cashshop_buy(sd, p->itemId, p->amount, 0);
#elif PACKETVER < 20101116
		fail = npc->cashshop_buy(sd, p->itemId, p->amount, p->kafraPoints);
#else
		int len = p->packetLength;
		int needLen;
		int points;
		int count;
		struct itemlist item_list = { 0 };
		int i;

		if (len < sizeof(struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM)) {
			ShowWarning("Player %d sent incorrect cash shop buy packet (len %d)!\n", sd->status.char_id, len);
			return;
		}

		points = p->kafraPoints;
		count = p->count;
		needLen = sizeof(struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM) + count * sizeof(struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM_sub);
		if (len != needLen) {
			ShowWarning("Player %d sent incorrect cash shop buy packet (len %d:%d)!\n", sd->status.char_id, len, needLen);
			return;
		}

		VECTOR_INIT(item_list);
		VECTOR_ENSURE(item_list, count, 1);
		for (i = 0; i < count; i++) {
			struct itemlist_entry entry = { 0 };

			entry.amount = p->items[i].amount;
			entry.id =     p->items[i].itemId;

			VECTOR_PUSH(item_list, entry);
		}
		fail = npc->cashshop_buylist(sd, points, &item_list);
		VECTOR_CLEAR(item_list);
#endif
	}

	clif->cashshop_ack(sd,fail);
}

/// Adoption System
///

/// Adoption message (ZC_BABYMSG).
/// 0216 <msg>.L
/// msg:
///     0 = "You cannot adopt more than 1 child."
///     1 = "You must be at least character level 70 in order to adopt someone."
///     2 = "You cannot adopt a married person."
static void clif_Adopt_reply(struct map_session_data *sd, int type)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,6);
	WFIFOW(fd,0) = 0x216;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd,6);
}

/// Adoption confirmation (ZC_REQ_BABY).
/// 01f6 <account id>.L <char id>.L <name>.B
static void clif_Adopt_request(struct map_session_data *sd, struct map_session_data *src, int p_id)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(src);
	fd = sd->fd;
	WFIFOHEAD(fd,34);
	WFIFOW(fd,0) = 0x1f6;
	WFIFOL(fd,2) = src->status.account_id;
	WFIFOL(fd,6) = p_id;
	memcpy(WFIFOP(fd,10), src->status.name, NAME_LENGTH);
	WFIFOSET(fd,34);
}

static void clif_parse_Adopt_request(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to adopt a player (CZ_REQ_JOIN_BABY).
/// 01f9 <account id>.L
static void clif_parse_Adopt_request(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct map_session_data *tsd = map->id2sd(RFIFOL(fd,2)), *p_sd = map->charid2sd(sd->status.partner_id);

	if( pc->can_Adopt(sd, p_sd, tsd) ) {
		tsd->adopt_invite = sd->status.account_id;
		clif->adopt_request(tsd, sd, p_sd->status.account_id);
	}
}

static void clif_parse_Adopt_reply(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Answer to adopt confirmation (CZ_JOIN_BABY).
/// 01f7 <account id>.L <char id>.L <answer>.L
/// answer:
///     0 = rejected
///     1 = accepted
static void clif_parse_Adopt_reply(int fd, struct map_session_data *sd)
{
	if (pc_isdead(sd))
		return;

	int p1_id = RFIFOL(fd,2);
	int p2_id = RFIFOL(fd,6);
	int result = RFIFOL(fd,10);
	struct map_session_data* p1_sd = map->id2sd(p1_id);
	struct map_session_data* p2_sd = map->id2sd(p2_id);

	int pid = sd->adopt_invite;
	sd->adopt_invite = 0;

	if( p1_sd == NULL || p2_sd == NULL )
		return; // Both players need to be online

	if( pid != p1_sd->status.account_id )
		return; // Incorrect values

	if( result == 0 )
		return; // Rejected

	pc->adoption(p1_sd, p2_sd, sd);
}

/// Convex Mirror (ZC_BOSS_INFO).
/// 0293 <infoType>.B <x>.L <y>.L <minHours>.W <minMinutes>.W <maxHours>.W <maxMinutes>.W <monster name>.51B
/// infoType:
///     BOSS_INFO_NONE  = No boss on the map
///     BOSS_INFO_ALIVE = Boss is alive (Update monster position)
///     BOSS_INFO_ALIVE_WITHMSG = Boss is alive (Initial announce)
///     BOSS_INFO_DEAD = Boss on the map is dead (Display respawn time)
static void clif_bossmapinfo(int fd, struct mob_data *md, enum bossmap_info_type flag)
{
	WFIFOHEAD(fd, 70);
	memset(WFIFOP(fd, 0), 0, 70);
	WFIFOW(fd, 0) = 0x293;
	WFIFOB(fd, 2) = flag;

	switch (flag) {
	case BOSS_INFO_NONE:
		break; 
	case BOSS_INFO_ALIVE:
	case BOSS_INFO_ALIVE_WITHMSG:
		if (md != NULL) {
			WFIFOL(fd, 3) = md->bl.x;
			WFIFOL(fd, 7) = md->bl.y;
		}
		break;
	case BOSS_INFO_DEAD:
		if (md != NULL) {
			const struct TimerData *timer_data = timer->get(md->spawn_timer);
			unsigned int seconds;
			int hours;
			int minutes;

			seconds = (unsigned int)(DIFF_TICK(timer_data->tick, timer->gettick()) / 1000 + 60);
			hours = seconds / (60 * 60);
			seconds = seconds - (60 * 60 * hours);
			minutes = seconds / 60;

			WFIFOW(fd, 11) = hours;
			WFIFOW(fd, 13) = minutes;
		}
		break;
	}

	if (md != NULL)
		safestrncpy(WFIFOP(fd, 19), md->db->jname, NAME_LENGTH);

	WFIFOSET(fd, 70);
}

static void clif_parse_ViewPlayerEquip(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Requesting equip of a player (CZ_EQUIPWIN_MICROSCOPE).
/// 02d6 <account id>.L
static void clif_parse_ViewPlayerEquip(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int charid = RFIFOL(fd, 2);
	struct map_session_data* tsd = map->id2sd(charid);

	if (!tsd)
		return;

	if (tsd->status.show_equip || pc_has_permission(sd, PC_PERM_VIEW_EQUIPMENT)) {
		clif->viewequip_ack(sd, tsd);
	} else {
#if PACKETVER >= 20070918
		clif->msgtable(sd, MSG_OPEN_EQUIPEDITEM_REFUSED);
#endif
	}
}

static void clif_parse_cz_config(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Receive configurations (CZ_CONFIG).
/// 02d8 <type>.L <value>.L
/// type:
///     0 = open equip window
///     2 = pet autofeeding
///     3 = homunculus autofeeding
///     value:
///         0 = disabled
///         1 = enabled
static void clif_parse_cz_config(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	enum CZ_CONFIG type = RFIFOL(fd, 2);
	int flag = RFIFOL(fd, 6);

	switch (type) {
	case CZ_CONFIG_OPEN_EQUIPMENT_WINDOW:
		sd->status.show_equip = flag;
		break;
	case CZ_CONFIG_PET_AUTOFEEDING: {
		struct pet_data *pd = sd->pd;
		nullpo_retv(pd);
		if (pd->petDB->autofeed == 0) {
			clif->message(fd, msg_sd(sd, 189)); // "Autofeed is disabled for this pet."
			return;
		}
		pd->pet.autofeed = flag;
		break;
	}
	case CZ_CONFIG_HOMUNCULUS_AUTOFEEDING: {
		struct homun_data *hd = sd->hd;
		nullpo_retv(hd);
		hd->homunculus.autofeed = flag;
		break;
	}
	case CZ_CONFIG_CALL:
		sd->status.allow_call = flag;
		break;
	default:
		ShowWarning("clif_parse_cz_config: Unsupported type has been received (%u).\n", type);
		return;
	}
	clif->zc_config(sd, type, flag);
}

static void clif_parse_PartyTick(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change party invitation tick.
///     value:
///         0 = enabled
///         1 = disabled
static void clif_parse_PartyTick(int fd, struct map_session_data *sd)
{
	const struct PACKET_CZ_PARTY_CONFIG *const p = RFIFOP(fd, 0);
	const bool newAllowParty = p->refuseInvite ? true : false;
	if (newAllowParty != sd->status.allow_party) {
		sd->status.allow_party = newAllowParty;
		if ((map->save_settings & 512) != 0)
			chrif->save(sd, 0); // send to char server
	} else {
		sd->status.allow_party = newAllowParty;
	}
	clif->partytickack(sd, sd->status.allow_party);
}

/// Questlog System [Kevin] [Inkfish]
///

/// Sends list of all quest states (ZC_ALL_QUEST_LIST).
/// 02b1 <packet len>.W <num>.L { <quest id>.L <active>.B }*num
/// 097a <packet len>.W <num>.L { <quest id>.L <active>.B <remaining time>.L <time>.L <count>.W { <mob_id>.L <killed>.W <total>.W <mob name>.24B }*count }*num
/// 09f8 <packet len>.W <num>.L { <quest id>.L <active>.B <remaining time>.L <time>.L <count>.W { <hunt identification>.L <mob type>.L <mob_id>.L <min level>.L <max level>.L <killed>.W <total>.W <mob name>.24B }*count }*num
static void clif_quest_send_list(struct map_session_data *sd)
{
	int i, len, real_len;
	uint8 *buf = NULL;
	struct packet_quest_list_header *packet = NULL;
	nullpo_retv(sd);

	len = sizeof(struct packet_quest_list_header)
	    + sd->avail_quests * (sizeof(struct packet_quest_list_info)
	                         + MAX_QUEST_OBJECTIVES * sizeof(struct packet_mission_info_sub)); // >= than the actual length
	buf = aMalloc(len);
	packet = WBUFP(buf, 0);
	real_len = sizeof(*packet);

	packet->PacketType = questListType;
	packet->questCount = sd->avail_quests;

	for (i = 0; i < sd->avail_quests; i++) {
		struct packet_quest_list_info *info = (struct packet_quest_list_info *)(buf+real_len);
#if PACKETVER >= 20141022
		struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
		int j;
#endif // PACKETVER >= 20141022
		real_len += sizeof(*info);

		info->questID = sd->quest_log[i].quest_id;
		info->active = sd->quest_log[i].state;
#if PACKETVER >= 20141022
		info->quest_svrTime = sd->quest_log[i].time - qi->time;
		info->quest_endTime = sd->quest_log[i].time;
		info->hunting_count = qi->objectives_count;

		for (j = 0; j < qi->objectives_count; j++) {
			Assert_retb(j < MAX_QUEST_OBJECTIVES);

			int mob_id = qi->objectives[j].mob;
			if (mob_id == 0) {
				if (quest_mobtypeisenabled(qi->objectives[j].mobtype)) {
					mob_id = QUEST_MOBTYPE_ID;
				} else if (qi->objectives[j].mapid >= 0) {
					mob_id = QUEST_MAPWIDE_ID;
				}
			}
			real_len += sizeof(info->objectives[j]);

#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
			info->objectives[j].huntIdent = sd->quest_log[i].quest_id;
			info->objectives[j].huntIdent2 = j;
			info->objectives[j].mobType = quest_mobtype2client(qi->objectives[j].mobtype);
#elif PACKETVER >= 20150513
			info->objectives[j].huntIdent = (sd->quest_log[i].quest_id * 1000) + j;
			info->objectives[j].mobType = quest_mobtype2client(qi->objectives[j].mobtype);
#endif
			info->objectives[j].mob_id = mob_id;
#if PACKETVER >= 20150513
			info->objectives[j].levelMin = qi->objectives[j].level.min;
			info->objectives[j].levelMax = qi->objectives[j].level.max;
#endif
			info->objectives[j].huntCount = sd->quest_log[i].count[j];
			info->objectives[j].maxCount = qi->objectives[j].count;
			if (mob_id == QUEST_MAPWIDE_ID) {
				safestrncpy(info->objectives[j].mobName, map->list[qi->objectives[j].mapid].name, NAME_LENGTH);
			} else if (mob_id == QUEST_MOBTYPE_ID) {
				safestrncpy(info->objectives[j].mobName, "", NAME_LENGTH);
			} else {
				const struct mob_db *monster = mob->db(qi->objectives[j].mob);
				safestrncpy(info->objectives[j].mobName, monster->jname, NAME_LENGTH);
			}
		}
#endif // PACKETVER >= 20141022
	}
	packet->PacketLength = real_len;
	clif->send(buf, real_len, &sd->bl, SELF);
	aFree(buf);
}

/// Sends list of all quest missions (ZC_ALL_QUEST_MISSION).
/// 02b2 <packet len>.W <num>.L { <quest id>.L <start time>.L <expire time>.L <mobs>.W { <mob id>.L <mob count>.W <mob name>.24B }*3 }*num
static void clif_quest_send_mission(struct map_session_data *sd)
{
	int fd = sd->fd;
	int i, j;
	int len;
	struct mob_db *monster;

	nullpo_retv(sd);
	len = sd->avail_quests*104+8;
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x2b2;
	WFIFOW(fd, 2) = len;
	WFIFOL(fd, 4) = sd->avail_quests;

	for (i = 0; i < sd->avail_quests; i++) {
		struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
		WFIFOL(fd, i*104+8) = sd->quest_log[i].quest_id;
		WFIFOL(fd, i*104+12) = sd->quest_log[i].time - qi->time;
		WFIFOL(fd, i*104+16) = sd->quest_log[i].time;
		WFIFOW(fd, i*104+20) = qi->objectives_count;

		for (j = 0 ; j < qi->objectives_count; j++) {
			WFIFOL(fd, i*104+22+j*30) = qi->objectives[j].mob;
			WFIFOW(fd, i*104+26+j*30) = sd->quest_log[i].count[j];
			monster = mob->db(qi->objectives[j].mob);
			memcpy(WFIFOP(fd, i*104+28+j*30), monster->jname, NAME_LENGTH);
		}
	}

	WFIFOSET(fd, len);
}

/// Notification about a new quest (ZC_ADD_QUEST).
/// 02b3 <quest id>.L <active>.B <start time>.L <expire time>.L <mobs>.W { <mob id>.L <mob count>.W <mob name>.24B }*3
/// 09f9 <quest id>.L <active>.B <start time>.L <expire time>.L <mobs>.W { <hunt identification>.L <mob type>.L <mob id>.L <min level>.L <max level>.L <mob count>.W <mob name>.24B }*3
static void clif_quest_add(struct map_session_data *sd, struct quest *qd)
{
	int i, len;
	uint8 *buf = NULL;
	struct packet_quest_add_header *packet = NULL;
	struct quest_db *qi;

	nullpo_retv(sd);
	nullpo_retv(qd);

	qi = quest->db(qd->quest_id);
	Assert_retv(qi->objectives_count < MAX_QUEST_OBJECTIVES);

	len = sizeof(struct packet_quest_add_header)
	            + MAX_QUEST_OBJECTIVES * sizeof(struct packet_quest_hunt_sub); // >= than the actual length

	buf = aCalloc(1, len);
	packet = (struct packet_quest_add_header *)WBUFP(buf, 0);

	packet->PacketType = questAddType;
	packet->questID = qd->quest_id;
	packet->active = qd->state;
	packet->quest_svrTime = qd->time - qi->time;
	packet->quest_endTime = qd->time;
	packet->count = qi->objectives_count;

	for (i = 0; i < qi->objectives_count; i++) {
		int mob_id = qi->objectives[i].mob;
		if (mob_id == 0) {
			if (quest_mobtypeisenabled(qi->objectives[i].mobtype)) {
				mob_id = QUEST_MOBTYPE_ID;
			} else if (qi->objectives[i].mapid >= 0) {
				mob_id = QUEST_MAPWIDE_ID;
			}
		}

#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
		packet->objectives[i].huntIdent = qd->quest_id;
		packet->objectives[i].huntIdent2 = i;
		packet->objectives[i].mobType = quest_mobtype2client(qi->objectives[i].mobtype);
#elif PACKETVER >= 20150513
		packet->objectives[i].huntIdent = (qd->quest_id * 1000) + i;
		packet->objectives[i].mobType = quest_mobtype2client(qi->objectives[i].mobtype);
#endif
		packet->objectives[i].mob_id = mob_id;
#if PACKETVER >= 20150513
		packet->objectives[i].levelMin = qi->objectives[i].level.min;
		packet->objectives[i].levelMax = qi->objectives[i].level.max;
#endif
		packet->objectives[i].huntCount = qd->count[i];

		if (mob_id == QUEST_MAPWIDE_ID) {
			safestrncpy(packet->objectives[i].mobName, map->list[qi->objectives[i].mapid].name, NAME_LENGTH);
		} else if (mob_id == QUEST_MOBTYPE_ID) {
			safestrncpy(packet->objectives[i].mobName, "", NAME_LENGTH);
		} else {
			const struct mob_db *monster = mob->db(qi->objectives[i].mob);
			safestrncpy(packet->objectives[i].mobName, monster->jname, NAME_LENGTH);
		}
	}
	clif->send(buf, len, &sd->bl, SELF);
	aFree(buf);
}

/// Notification about a quest being removed (ZC_DEL_QUEST).
/// 02b4 <quest id>.L
static void clif_quest_delete(struct map_session_data *sd, int quest_id)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x2b4));
	WFIFOW(fd, 0) = 0x2b4;
	WFIFOL(fd, 2) = quest_id;
	WFIFOSET(fd, packet_len(0x2b4));
}

/// Notification of an update to the hunting mission counter (ZC_UPDATE_MISSION_HUNT).
/// 02b5 <packet len>.W <mobs>.W { <quest id>.L <mob id>.L <total count>.W <current count>.W }*3
/// 09fa <packet len>.W <mobs>.W { <quest id>.L <hunt identification>.L <total count>.W <current count>.W }*3
static void clif_quest_update_objective(struct map_session_data *sd, struct quest *qd)
{
	int i, len, real_len;
	uint8 *buf = NULL;
	struct packet_quest_update_header *packet = NULL;
	struct quest_db *qi;

	nullpo_retv(sd);
	nullpo_retv(qd);

	qi = quest->db(qd->quest_id);
	Assert_retv(qi->objectives_count < MAX_QUEST_OBJECTIVES);

	len = sizeof(struct packet_quest_update_header)
	            + MAX_QUEST_OBJECTIVES * sizeof(struct packet_quest_update_hunt); // >= than the actual length

	buf = aCalloc(1, len);
	packet = (struct packet_quest_update_header *)WBUFP(buf, 0);
	real_len = sizeof(*packet);

	packet->PacketType = questUpdateType;
	packet->count = qi->objectives_count;

	for (i = 0; i < qi->objectives_count; i++) {
		real_len += sizeof(packet->objectives[i]);

		packet->objectives[i].questID = qd->quest_id;
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
		packet->objectives[i].huntIdent = qd->quest_id;
		packet->objectives[i].huntIdent2 = i;
#elif PACKETVER >= 20150513
		packet->objectives[i].huntIdent = (qd->quest_id * 1000) + i;
#else
		packet->objectives[i].mob_id = qi->objectives[i].mob;
#endif
		packet->objectives[i].maxCount = qi->objectives[i].count;
		packet->objectives[i].count = qd->count[i];
	}
	packet->PacketLength = real_len;
	clif->send(buf, real_len, &sd->bl, SELF);
	aFree(buf);
}

/// Notification of an hunting mission counter just after quest is added (ZC_HUNTING_QUEST_INFO).
/// 08fe <packet len>.W  { <quest id>.L <mob id>.L <total count>.W <current count>.W }*3
static void clif_quest_notify_objective(struct map_session_data *sd, struct quest *qd)
{
#if PACKETVER >= 20150513
	int i, len, real_len;
	uint8 *buf = NULL;
	struct packet_quest_hunt_info *packet = NULL;
	struct quest_db *qi;

	nullpo_retv(sd);
	nullpo_retv(qd);

	qi = quest->db(qd->quest_id);
	Assert_retv(qi->objectives_count < MAX_QUEST_OBJECTIVES);

	len = sizeof(struct packet_quest_hunt_info)
	            + MAX_QUEST_OBJECTIVES * sizeof(struct packet_quest_hunt_info_sub); // >= than the actual length

	buf = aCalloc(1, len);
	packet = (struct packet_quest_hunt_info *)WBUFP(buf, 0);
	real_len = sizeof(*packet);

	packet->PacketType = questUpdateType2;

	for (i = 0; i < qi->objectives_count; i++) {
		real_len += sizeof(packet->info[i]);

		packet->info[i].questID = qd->quest_id;
		packet->info[i].mob_id = qi->objectives[i].mob;
		packet->info[i].maxCount = qi->objectives[i].count;
		packet->info[i].count = qd->count[i];
	}
	packet->PacketLength = real_len;
	clif->send(buf, real_len, &sd->bl, SELF);
	aFree(buf);
#endif
}

static void clif_parse_questStateAck(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to change the state of a quest (CZ_ACTIVE_QUEST).
/// 02b6 <quest id>.L <active>.B
static void clif_parse_questStateAck(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	quest->update_status(sd, RFIFOL(fd,2), RFIFOB(fd,6)?Q_ACTIVE:Q_INACTIVE);
}

/// Notification about the change of a quest state (ZC_ACTIVE_QUEST).
/// 02b7 <quest id>.L <active>.B
static void clif_quest_update_status(struct map_session_data *sd, int quest_id, bool active)
{
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x2b7));
	WFIFOW(fd, 0) = 0x2b7;
	WFIFOL(fd, 2) = quest_id;
	WFIFOB(fd, 6) = active;
	WFIFOSET(fd, packet_len(0x2b7));
}

/// Notification about an NPC's quest state (ZC_QUEST_NOTIFY_EFFECT).
/// 0446 <npc id>.L <x>.W <y>.W <effect>.W <type>.W
/// effect:
///     0 = none
///     1 = exclamation mark icon
///     2 = question mark icon
/// type:
///     0 = yellow
///     1 = orange
///     2 = green
///     3 = purple
static void clif_quest_show_event(struct map_session_data *sd, struct block_list *bl, short state, short color)
{
#if PACKETVER >= 20090218
	int fd;

	nullpo_retv(sd);
	nullpo_retv(bl);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x446));
	WFIFOW(fd, 0) = 0x446;
	WFIFOL(fd, 2) = bl->id;
	WFIFOW(fd, 6) = bl->x;
	WFIFOW(fd, 8) = bl->y;
	WFIFOW(fd, 10) = state;
	WFIFOW(fd, 12) = color;
	WFIFOSET(fd, packet_len(0x446));
#endif
}

/// Mercenary System
///

/// Notification about a mercenary status parameter change (ZC_MER_PAR_CHANGE).
/// 02a2 <var id>.W <value>.L
static void clif_mercenary_updatestatus(struct map_session_data *sd, int type)
{
	struct mercenary_data *md;
	struct status_data *mstatus;
	int fd;
	if( sd == NULL || (md = sd->md) == NULL )
		return;

	fd = sd->fd;
	mstatus = &md->battle_status;
	WFIFOHEAD(fd,packet_len(0x2a2));
	WFIFOW(fd,0) = 0x2a2;
	WFIFOW(fd,2) = type;
	switch( type ) {
		case SP_ATK1:
		{
			int atk = rnd()%(mstatus->rhw.atk2 - mstatus->rhw.atk + 1) + mstatus->rhw.atk;
			WFIFOL(fd,4) = cap_value(atk, 0, INT16_MAX);
		}
			break;
		case SP_MATK1:
			WFIFOL(fd,4) = cap_value(mstatus->matk_max, 0, INT16_MAX);
			break;
		case SP_HIT:
			WFIFOL(fd,4) = mstatus->hit;
			break;
		case SP_CRITICAL:
			WFIFOL(fd,4) = mstatus->cri/10;
			break;
		case SP_DEF1:
			WFIFOL(fd,4) = mstatus->def;
			break;
		case SP_MDEF1:
			WFIFOL(fd,4) = mstatus->mdef;
			break;
		case SP_MERCFLEE:
			WFIFOL(fd,4) = mstatus->flee;
			break;
		case SP_ASPD:
			WFIFOL(fd,4) = mstatus->amotion;
			break;
		case SP_HP:
			WFIFOL(fd,4) = mstatus->hp;
			break;
		case SP_MAXHP:
			WFIFOL(fd,4) = mstatus->max_hp;
			break;
		case SP_SP:
			WFIFOL(fd,4) = mstatus->sp;
			break;
		case SP_MAXSP:
			WFIFOL(fd,4) = mstatus->max_sp;
			break;
		case SP_MERCKILLS:
			WFIFOL(fd,4) = md->mercenary.kill_count;
			break;
		case SP_MERCFAITH:
			WFIFOL(fd,4) = mercenary->get_faith(md);
			break;
	}
	WFIFOSET(fd,packet_len(0x2a2));
}

/// Mercenary base status data (ZC_MER_INIT).
/// 029b <id>.L <atk>.W <matk>.W <hit>.W <crit>.W <def>.W <mdef>.W <flee>.W <aspd>.W
///     <name>.24B <level>.W <hp>.L <maxhp>.L <sp>.L <maxsp>.L <expire time>.L <faith>.W
///     <calls>.L <kills>.L <atk range>.W
static void clif_mercenary_info(struct map_session_data *sd)
{
	int fd;
	struct mercenary_data *md;
	struct status_data *mstatus;
	int atk;

	if( sd == NULL || (md = sd->md) == NULL )
		return;

	fd = sd->fd;
	mstatus = &md->battle_status;

	WFIFOHEAD(fd,packet_len(0x29b));
	WFIFOW(fd,0) = 0x29b;
	WFIFOL(fd,2) = md->bl.id;

	// Mercenary shows ATK as a random value between ATK ~ ATK2
#ifdef RENEWAL
	atk = status->get_weapon_atk(&md->bl, &mstatus->rhw, 0);
#else
	atk = rnd() % (mstatus->rhw.atk2 - mstatus->rhw.atk + 1) + mstatus->rhw.atk;
#endif
	WFIFOW(fd, 6) = cap_value(atk, 0, INT16_MAX);
#ifdef RENEWAL
	atk = status->base_matk(&md->bl, mstatus, status->get_lv(&md->bl));
	WFIFOW(fd,8) = cap_value(atk, 0, INT16_MAX);
#else
	WFIFOW(fd,8) = cap_value(mstatus->matk_max, 0, INT16_MAX);
#endif
	WFIFOW(fd,10) = mstatus->hit;
	WFIFOW(fd,12) = mstatus->cri/10;
#ifdef RENEWAL
	WFIFOW(fd, 14) = mstatus->def2;
	WFIFOW(fd, 16) = mstatus->mdef2;
#else
	WFIFOW(fd,14) = mstatus->def;
	WFIFOW(fd,16) = mstatus->mdef;
#endif
	WFIFOW(fd,18) = mstatus->flee;
	WFIFOW(fd,20) = mstatus->amotion;
	safestrncpy(WFIFOP(fd,22), md->db->name, NAME_LENGTH);
	WFIFOW(fd,46) = md->db->lv;
	WFIFOL(fd,48) = mstatus->hp;
	WFIFOL(fd,52) = mstatus->max_hp;
	WFIFOL(fd,56) = mstatus->sp;
	WFIFOL(fd,60) = mstatus->max_sp;
	WFIFOL(fd,64) = (int)time(NULL) + (mercenary->get_lifetime(md) / 1000);
	WFIFOW(fd,68) = mercenary->get_faith(md);
	WFIFOL(fd,70) = mercenary->get_calls(md);
	WFIFOL(fd,74) = md->mercenary.kill_count;
	WFIFOW(fd,78) = md->battle_status.rhw.range;
	WFIFOSET(fd,packet_len(0x29b));
}

/// Mercenary skill tree (ZC_MER_SKILLINFO_LIST).
/// 029d <packet len>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradeable>.B }*
static void clif_mercenary_skillblock(struct map_session_data *sd)
{
	struct mercenary_data *md;
	int fd, i, len = 4, j;

	if( sd == NULL || (md = sd->md) == NULL )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd,4+37*MAX_MERCSKILL);
	WFIFOW(fd,0) = 0x29d;
	for (i = 0; i < MAX_MERCSKILL; i++) {
		int id = md->db->skill[i].id;
		if (id == 0)
			continue;
		j = id - MC_SKILLBASE;
		WFIFOW(fd,len) = id;
		WFIFOL(fd,len+2) = skill->get_inf(id);
		WFIFOW(fd,len+6) = md->db->skill[j].lv;
		if ( md->db->skill[j].lv ) {
			WFIFOW(fd, len + 8) = skill->get_sp(id, md->db->skill[j].lv);
			WFIFOW(fd, len + 10) = skill->get_range2(&md->bl, id, md->db->skill[j].lv);
		} else {
			WFIFOW(fd, len + 8) = 0;
			WFIFOW(fd, len + 10) = 0;
		}
		safestrncpy(WFIFOP(fd,len+12), skill->get_name(id), NAME_LENGTH);
		WFIFOB(fd,len+36) = 0; // Skillable for Mercenary?
		len += 37;
	}

	WFIFOW(fd,2) = len;
	WFIFOSET(fd,len);
}

static void clif_parse_mercenary_action(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke a mercenary menu action (CZ_MER_COMMAND).
/// 029f <command>.B
///     1 = mercenary information
///     2 = delete
static void clif_parse_mercenary_action(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int option = RFIFOB(fd,2);
	if (sd->md == NULL)
		return;

	if (option == 2)
		mercenary->delete(sd->md, MERC_DELETE_REMOVED);
}

/// Mercenary Message
/// message:
///     0 = Mercenary soldier's duty hour is over.
///     1 = Your mercenary soldier has been killed.
///     2 = Your mercenary soldier has been fired.
///     3 = Your mercenary soldier has ran away.
static void clif_mercenary_message(struct map_session_data *sd, int message)
{
#if PACKETVER >= 20070227
	clif->msgtable(sd, MSG_MER_FINISH + message);
#endif
}

/// Notification about the remaining time of a rental item (ZC_CASH_TIME_COUNTER).
/// 0298 <name id>.W <seconds>.L
static void clif_rental_time(int fd, int nameid, int seconds)
{ // '<ItemName>' item will disappear in <seconds/60> minutes.
	struct PACKET_ZC_CASH_TIME_COUNTER p;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x298;
	p.itemId = nameid;
	p.seconds = seconds;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Deletes a rental item from client's inventory (ZC_CASH_ITEM_DELETE).
/// 0299 <index>.W <name id>.W
static void clif_rental_expired(int fd, int index, int nameid)
{ // '<ItemName>' item has been deleted from the Inventory
	struct PACKET_ZC_CASH_ITEM_DELETE p;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x299;
	p.index = index + 2;
	p.itemId = nameid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Book Reading (ZC_READ_BOOK).
/// 0294 <book id>.L <page>.L
static void clif_readbook(int fd, int book_id, int page)
{
	WFIFOHEAD(fd,packet_len(0x294));
	WFIFOW(fd,0) = 0x294;
	WFIFOL(fd,2) = book_id;
	WFIFOL(fd,6) = page;
	WFIFOSET(fd,packet_len(0x294));
}

/// Battlegrounds
///

/// Updates HP bar of a camp member.
/// 02e0 <account id>.L <name>.24B <hp>.W <max hp>.W (ZC_BATTLEFIELD_NOTIFY_HP).
/// 0a0e <account id>.L <hp>.L <max hp>.L (ZC_BATTLEFIELD_NOTIFY_HP2)
static void clif_bg_hp(struct map_session_data *sd)
{
	unsigned char buf[34];

// packet version can be wrong, because inconsistend data in other servers. From packets table it start from 20140312 [4144]
#if PACKETVER < 20140613
	const int cmd = 0x2e0;
	nullpo_retv(sd);

	WBUFW(buf, 0) = cmd;
	WBUFL(buf, 2) = sd->status.account_id;
	memcpy(WBUFP(buf, 6), sd->status.name, NAME_LENGTH);

	if (sd->battle_status.max_hp > INT16_MAX)
	{ // To correctly display the %hp bar. [Skotlex]
		WBUFW(buf, 30) = sd->battle_status.hp / (sd->battle_status.max_hp / 100);
		WBUFW(buf, 32) = 100;
	}
	else
	{
		WBUFW(buf, 30) = sd->battle_status.hp;
		WBUFW(buf, 32) = sd->battle_status.max_hp;
	}
#else
	const int cmd = 0xa0e;
	nullpo_retv(sd);

	WBUFW(buf, 0) = cmd;
	WBUFL(buf, 2) = sd->status.account_id;
	if (sd->battle_status.max_hp > INT32_MAX) {
		WBUFL(buf, 6) = sd->battle_status.hp / (sd->battle_status.max_hp / 100);
		WBUFL(buf, 10) = 100;
	} else {
		WBUFL(buf, 6) = sd->battle_status.hp;
		WBUFL(buf, 10) = sd->battle_status.max_hp;
	}
#endif

	clif->send(buf, packet_len(cmd), &sd->bl, BG_AREA_WOS);
}

/// Updates the position of a camp member on the minimap (ZC_BATTLEFIELD_NOTIFY_POSITION).
/// 02df <account id>.L <name>.24B <class>.W <x>.W <y>.W
static void clif_bg_xy(struct map_session_data *sd)
{
	unsigned char buf[36];
	nullpo_retv(sd);

	WBUFW(buf,0)=0x2df;
	WBUFL(buf,2)=sd->status.account_id;
	memcpy(WBUFP(buf,6), sd->status.name, NAME_LENGTH);
	WBUFW(buf,30)=sd->status.class;
	WBUFW(buf,32)=sd->bl.x;
	WBUFW(buf,34)=sd->bl.y;

	clif->send(buf, packet_len(0x2df), &sd->bl, BG_SAMEMAP_WOS);
}

static void clif_bg_xy_remove(struct map_session_data *sd)
{
	unsigned char buf[36];
	nullpo_retv(sd);

	WBUFW(buf,0)=0x2df;
	WBUFL(buf,2)=sd->status.account_id;
	memset(WBUFP(buf,6), 0, NAME_LENGTH);
	WBUFW(buf,30)=0;
	WBUFW(buf,32)=-1;
	WBUFW(buf,34)=-1;

	clif->send(buf, packet_len(0x2df), &sd->bl, BG_SAMEMAP_WOS);
}

/// Notifies clients of a battleground message (ZC_BATTLEFIELD_CHAT).
/// 02dc <packet len>.W <account id>.L <name>.24B <message>.?B
static void clif_bg_message(struct battleground_data *bgd, int src_id, const char *name, const char *mes)
{
	struct map_session_data *sd;
	unsigned char *buf;
	int len;

	nullpo_retv(bgd);
	nullpo_retv(name);
	nullpo_retv(mes);

	if (!bgd->count || (sd = bg->getavailablesd(bgd)) == NULL)
		return;

	len = (int)strlen(mes);
	Assert_retv(len <= INT16_MAX - NAME_LENGTH - 9);
	buf = (unsigned char *)aCalloc(len + NAME_LENGTH + 9, sizeof(unsigned char));

	WBUFW(buf, 0) = 0x2dc;
	WBUFW(buf, 2) = len + NAME_LENGTH + 9;
	WBUFL(buf, 4) = src_id;
	safestrncpy(WBUFP(buf, 8), name, NAME_LENGTH);
	safestrncpy(WBUFP(buf, 32), mes, len + 1);
	clif->send(buf, WBUFW(buf, 2), &sd->bl, BG);

	aFree(buf);
}

/**
 * Validates and processes battlechat messages [pakpil] (CZ_BATTLEFIELD_CHAT).
 *
 * @code
 * 0x2db <packet len>.W <text>.?B (<name> : <message>) 00
 * @endcode
 *
 * @param fd The incoming file descriptor.
 * @param sd The related character.
 */
static void clif_parse_BattleChat(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BattleChat(int fd, struct map_session_data *sd)
{
	const struct packet_chat_message *packet = RP2PTR(fd);
	char message[CHAT_SIZE_MAX + NAME_LENGTH + 3 + 1];

	if (clif->process_chat_message(sd, packet, message, sizeof message) == NULL)
		return;

	bg->send_message(sd, message);
}

/// Notifies client of a battleground score change (ZC_BATTLEFIELD_NOTIFY_POINT).
/// 02de <camp A points>.W <camp B points>.W
static void clif_bg_updatescore(int16 m)
{
	struct block_list bl;
	unsigned char buf[6];

	memset(&bl, 0, sizeof(bl));
	bl.type = BL_NUL;
	bl.m = m;

	WBUFW(buf,0) = 0x2de;
	WBUFW(buf,2) = map->list[m].bgscore_lion;
	WBUFW(buf,4) = map->list[m].bgscore_eagle;
	clif->send(buf,packet_len(0x2de),&bl,ALL_SAMEMAP);
}

static void clif_bg_updatescore_single(struct map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2de));
	WFIFOW(fd,0) = 0x2de;
	WFIFOW(fd,2) = map->list[sd->bl.m].bgscore_lion;
	WFIFOW(fd,4) = map->list[sd->bl.m].bgscore_eagle;
	WFIFOSET(fd,packet_len(0x2de));
}

/// Battleground camp belong-information (ZC_BATTLEFIELD_NOTIFY_CAMPINFO).
/// 02dd <account id>.L <name>.24B <camp>.W
static void clif_sendbgemblem_area(struct map_session_data *sd)
{
	unsigned char buf[33];
	nullpo_retv(sd);

	WBUFW(buf, 0) = 0x2dd;
	WBUFL(buf,2) = sd->bl.id;
	safestrncpy(WBUFP(buf,6), sd->status.name, NAME_LENGTH); // name don't show in screen.
	WBUFW(buf,30) = sd->bg_id;
	clif->send(buf,packet_len(0x2dd), &sd->bl, AREA);
}

static void clif_sendbgemblem_single(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	WFIFOHEAD(fd,32);
	WFIFOW(fd,0) = 0x2dd;
	WFIFOL(fd,2) = sd->bl.id;
	safestrncpy(WFIFOP(fd,6), sd->status.name, NAME_LENGTH);
	WFIFOW(fd,30) = sd->bg_id;
	WFIFOSET(fd,packet_len(0x2dd));
}

/// Custom Fonts (ZC_NOTIFY_FONT).
/// 02ef <account_id>.L <font id>.W
static void clif_font(struct map_session_data *sd)
{
#if PACKETVER >= 20080102
	unsigned char buf[8];
	nullpo_retv(sd);
	WBUFW(buf,0) = 0x2ef;
	WBUFL(buf,2) = sd->bl.id;
	WBUFW(buf,6) = sd->status.font;
	clif->send(buf, packet_len(0x2ef), &sd->bl, AREA);
#endif
}

/*==========================================
 * Instancing Window
 *------------------------------------------*/
static int clif_instance(int instance_id, int type, int flag)
{
	struct map_session_data *sd = NULL;
	unsigned char buf[255];
	enum send_target target = PARTY;

	switch( instance->list[instance_id].owner_type ) {
		case IOT_NONE:
			return 0;
		case IOT_GUILD:
			target = GUILD;
			sd = guild->getavailablesd(guild->search(instance->list[instance_id].owner_id));
			break;
		case IOT_PARTY:
			/* default is already PARTY */
			sd = party->getavailablesd(party->search(instance->list[instance_id].owner_id));
			break;
		case IOT_CHAR:
			target = SELF;
			sd = map->id2sd(instance->list[instance_id].owner_id);
			break;
	}

	if( !sd )
		return 0;

	switch( type ) {
		case 1:
			// S 0x2cb <Instance name>.61B <Standby Position>.W
			// Required to start the instancing information window on Client
			// This window re-appear each "refresh" of client automatically until type 4 is send to client.
			WBUFW(buf,0) = 0x02CB;
			memcpy(WBUFP(buf,2),instance->list[instance_id].name,INSTANCE_NAME_LENGTH);
			WBUFW(buf,63) = flag;
			clif->send(buf,packet_len(0x02CB),&sd->bl,target);
			break;
		case 2:
			// S 0x2cc <Standby Position>.W
			// To announce Instancing queue creation if no maps available
			// flag is priority, negative value mean cancel reservation
			WBUFW(buf,0) = 0x02CC;
			WBUFW(buf,2) = flag;
			clif->send(buf,packet_len(0x02CC),&sd->bl,target);
			break;
		case 3:
		case 4:
			// S 0x2cd <Instance Name>.61B <Instance Remaining Time>.L <Instance Noplayers close time>.L
			WBUFW(buf,0) = 0x02CD;
			memcpy(WBUFP(buf,2),instance->list[instance_id].name,61);
			if( type == 3 ) {
				WBUFL(buf,63) = instance->list[instance_id].progress_timeout;
				WBUFL(buf,67) = 0;
			} else {
				WBUFL(buf,63) = 0;
				WBUFL(buf,67) = instance->list[instance_id].idle_timeout;
			}
			clif->send(buf,packet_len(0x02CD),&sd->bl,target);
			break;
		case 5:
			// S 0x2ce <Message ID>.L
			// 0 = Notification (EnterLimitDate update?)
			// 1 = The Memorial Dungeon expired; it has been destroyed
			// 2 = The Memorial Dungeon's entry time limit expired; it has been destroyed
			// 3 = The Memorial Dungeon has been removed.
			// 4 = Create failure (removes the instance window)
			WBUFW(buf,0) = 0x02CE;
			WBUFL(buf,2) = flag;
			//WBUFL(buf,6) = EnterLimitDate;
			clif->send(buf,packet_len(0x02CE),&sd->bl,target);
			break;
	}
	return 0;
}

static void clif_instance_join(int fd, int instance_id)
{
	if( instance->list[instance_id].idle_timer != INVALID_TIMER ) {
		WFIFOHEAD(fd,packet_len(0x02CD));
		WFIFOW(fd,0) = 0x02CD;
		memcpy(WFIFOP(fd,2),instance->list[instance_id].name,61);
		WFIFOL(fd,63) = 0;
		WFIFOL(fd,67) = instance->list[instance_id].idle_timeout;
		WFIFOSET(fd,packet_len(0x02CD));
	} else if( instance->list[instance_id].progress_timer != INVALID_TIMER ) {
		WFIFOHEAD(fd,packet_len(0x02CD));
		WFIFOW(fd,0) = 0x02CD;
		memcpy(WFIFOP(fd,2),instance->list[instance_id].name,61);
		WFIFOL(fd,63) = instance->list[instance_id].progress_timeout;
		WFIFOL(fd,67) = 0;
		WFIFOSET(fd,packet_len(0x02CD));
	} else {
		WFIFOHEAD(fd,packet_len(0x02CB));
		WFIFOW(fd,0) = 0x02CB;
		memcpy(WFIFOP(fd,2),instance->list[instance_id].name,61);
		WFIFOW(fd,63) = 0;
		WFIFOSET(fd,packet_len(0x02CB));
	}
}

static void clif_instance_leave(int fd)
{
	WFIFOHEAD(fd,packet_len(0x02CE));
	WFIFOW(fd,0) = 0x02ce;
	WFIFOL(fd,2) = 4;
	WFIFOSET(fd,packet_len(0x02CE));
}

/// Notifies clients about item picked up by a party member (ZC_ITEM_PICKUP_PARTY).
/// 02b8 <account id>.L <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B
static void clif_party_show_picker(struct map_session_data *sd, struct item *item_data)
{
#if PACKETVER >= 20071002
	struct item_data* id;
	struct PACKET_ZC_ITEM_PICKUP_PARTY p;

	nullpo_retv(sd);
	nullpo_retv(item_data);
	id = itemdb->search(item_data->nameid);
	p.packetType = 0x2b8;
	p.AID = sd->status.account_id;
	p.itemId = item_data->nameid;
	p.identified = item_data->identify;
	p.damaged = item_data->attribute;
	p.refine = item_data->refine;
	clif->addcards(&p.slot, item_data);
	p.location = id->equip; // equip location
	p.itemType = itemtype(id->type); // item type
	clif->send(&p, sizeof(p), &sd->bl, PARTY_SAMEMAP_WOS);
#endif
}

/// Display gained exp (ZC_NOTIFY_EXP).
/// 07f6 <account id>.L <amount>.L <var id>.W <exp type>.W
/// var id:
///     SP_BASEEXP, SP_JOBEXP
/// exp type:
///     0 = normal exp gain/loss
///     1 = quest exp gain/loss
static void clif_displayexp(struct map_session_data *sd, uint64 exp, char type, bool is_quest)
{
	int fd;

#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	const int cmd = 0xacc;
#else
	const int cmd = 0x7f6;
#endif
	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd, 0) = cmd;
	WFIFOL(fd, 2) = sd->bl.id;
#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	WFIFOQ(fd, 6) = exp;
	WFIFOW(fd, 14) = type;
	WFIFOW(fd, 16) = is_quest ? 1 : 0; // Normal exp is shown in yellow, quest exp is shown in purple.
#else
	WFIFOL(fd, 6) = (uint32)exp;
	WFIFOW(fd, 10) = type;
	WFIFOW(fd, 12) = is_quest ? 1 : 0; // Normal exp is shown in yellow, quest exp is shown in purple.
#endif
	WFIFOSET(fd, packet_len(cmd));
}

/// Displays digital clock digits on top of the screen (ZC_SHOWDIGIT).
/// type:
///     0 = Displays 'value' for 5 seconds.
///     1 = Incremental counter (1 tick/second), negated 'value' specifies start value (e.g. using -10 lets the counter start at 10).
///     2 = Decremental counter (1 tick/second), negated 'value' specifies start value (does not stop when reaching 0, but overflows).
///     3 = Decremental counter (1 tick/second), 'value' specifies start value (stops when reaching 0, displays at most 2 digits).
/// value:
///     Except for type 3 it is interpreted as seconds for displaying as DD:HH:MM:SS, HH:MM:SS, MM:SS or SS (leftmost '00' is not displayed).
static void clif_showdigit(struct map_session_data *sd, unsigned char type, int value)
{
	nullpo_retv(sd);
	WFIFOHEAD(sd->fd, packet_len(0x1b1));
	WFIFOW(sd->fd,0) = 0x1b1;
	WFIFOB(sd->fd,2) = type;
	WFIFOL(sd->fd,3) = value;
	WFIFOSET(sd->fd, packet_len(0x1b1));
}

static void clif_parse_LessEffect(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Notification of the state of client command /effect (CZ_LESSEFFECT).
/// 021d <state>.L
/// state:
///     0 = Full effects
///     1 = Reduced effects
///
/// NOTE:   The state is used on Aegis for sending skill unit packet
///         0x11f (ZC_SKILL_ENTRY) instead of 0x1c9 (ZC_SKILL_ENTRY2)
///         whenever possible. Due to the way the decision check is
///         constructed, this state tracking was rendered useless,
///         as the only skill unit, that is sent with 0x1c9 is
///         Graffiti.
static void clif_parse_LessEffect(int fd, struct map_session_data *sd)
{
	int isLess = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	sd->state.lesseffect = ( isLess != 0 );
}

static void clif_parse_ItemListWindowSelected(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// S 07e4 <length>.w <option>.l <val>.l {<index>.w <amount>.w).4b*
static void clif_parse_ItemListWindowSelected(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int n = ((int)RFIFOW(fd, 2) - 12) / 4;
	int type = RFIFOL(fd,4);
	int flag = RFIFOL(fd,8); // Button clicked: 0 = Cancel, 1 = OK
	struct itemlist item_list = { 0 };
	int i;

	if( sd->state.trading || sd->npc_shopid )
		return;

	if (flag == 0 || n <= 0) {
		clif_menuskill_clear(sd);
		return; // Canceled by player.
	}

	if (n > sd->status.inventorySize)
		n = sd->status.inventorySize; // It should be impossible to have more than that.

	if (sd->menuskill_id != SO_EL_ANALYSIS && sd->menuskill_id != GN_CHANGEMATERIAL) {
		clif_menuskill_clear(sd);
		return; // Prevent hacking.
	}

	VECTOR_INIT(item_list);
	VECTOR_ENSURE(item_list, n, 1);
	for (i = 0; i < n; i++) {
		struct itemlist_entry entry = { 0 };
		entry.id = (int)RFIFOW(fd, 12 + 4 * i) - 2; // Inventory index
		entry.amount =  RFIFOW(fd, 12 + 4 * i + 2);
		VECTOR_PUSH(item_list, entry);
	}

	switch( type ) {
		case 0: // Change Material
			skill->changematerial(sd, &item_list);
			break;
		case 1: // Level 1: Pure to Rough
		case 2: // Level 2: Rough to Pure
			skill->elementalanalysis(sd, type, &item_list);
			break;
	}
	VECTOR_CLEAR(item_list);
	clif_menuskill_clear(sd);

	return;
}

/*==========================================
 * Elemental System
 *==========================================*/
static void clif_elemental_updatestatus(struct map_session_data *sd, int type)
{
	struct elemental_data *ed;
	struct status_data *estatus;
	int fd;

	if( sd == NULL || (ed = sd->ed) == NULL )
		return;

	fd = sd->fd;
	estatus = &ed->battle_status;
	WFIFOHEAD(fd,8);
	WFIFOW(fd,0) = 0x81e;
	WFIFOW(fd,2) = type;
	switch( type ) {
		case SP_HP:
			WFIFOL(fd,4) = estatus->hp;
			break;
		case SP_MAXHP:
			WFIFOL(fd,4) = estatus->max_hp;
			break;
		case SP_SP:
			WFIFOL(fd,4) = estatus->sp;
			break;
		case SP_MAXSP:
			WFIFOL(fd,4) = estatus->max_sp;
			break;
	}
	WFIFOSET(fd,8);
}

static void clif_elemental_info(struct map_session_data *sd)
{
	int fd;
	struct elemental_data *ed;
	struct status_data *estatus;

	if( sd == NULL || (ed = sd->ed) == NULL )
		return;

	fd = sd->fd;
	estatus = &ed->battle_status;

	WFIFOHEAD(fd,22);
	WFIFOW(fd, 0) = 0x81d;
	WFIFOL(fd, 2) = ed->bl.id;
	WFIFOL(fd, 6) = estatus->hp;
	WFIFOL(fd,10) = estatus->max_hp;
	WFIFOL(fd,14) = estatus->sp;
	WFIFOL(fd,18) = estatus->max_sp;
	WFIFOSET(fd,22);
}

/// Buying Store System
///

/// Opens preparation window for buying store (ZC_OPEN_BUYING_STORE).
/// 0810 <slots>.B
static void clif_buyingstore_open(struct map_session_data *sd)
{
#if PACKETVER >= 20100303
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x810));
	WFIFOW(fd,0) = 0x810;
	WFIFOB(fd,2) = sd->buyingstore.slots;
	WFIFOSET(fd,packet_len(0x810));
#endif
}

static void clif_parse_ReqOpenBuyingStore(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to create a buying store (CZ_REQ_OPEN_BUYING_STORE).
/// 0811 <packet len>.W <limit zeny>.L <result>.B <store name>.80B { <name id>.W <amount>.W <price>.L }*
/// result:
///     0 = cancel
///     1 = open
static void clif_parse_ReqOpenBuyingStore(int fd, struct map_session_data *sd)
{
	if (pc_istrading_except_npc(sd) || (sd->npc_id != 0 && sd->state.using_megaphone == 0) || pc_isdead(sd))
		return;

	const unsigned int blocksize = sizeof(struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub);
	const struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub *itemlist;
	char storename[MESSAGE_SIZE];
	unsigned char result;
	int zenylimit;
	int count, packet_len;
	const struct PACKET_CZ_REQ_OPEN_BUYING_STORE *p = RFIFOP(fd, 0);

	packet_len = p->packetLength;

	// TODO: Make this check global for all variable length packets.
	if (packet_len < sizeof(struct PACKET_CZ_REQ_OPEN_BUYING_STORE))
	{// minimum packet length
		ShowError("clif_parse_ReqOpenBuyingStore: Malformed packet (expected length=%u, length=%d, account_id=%d).\n", (uint32)sizeof(struct PACKET_CZ_REQ_OPEN_BUYING_STORE), packet_len, sd->bl.id);
		return;
	}

	zenylimit = p->zenyLimit;
	result    = p->result;
	safestrncpy(storename, p->storeName, sizeof(storename));
	itemlist  = &p->items[0];

	// so that buyingstore_create knows, how many elements it has access to
	packet_len -= sizeof(struct PACKET_CZ_REQ_OPEN_BUYING_STORE);

	if (packet_len < 0)
		return;

	if (packet_len % blocksize)
	{
		ShowError("clif_parse_ReqOpenBuyingStore: Unexpected item list size %d (account_id=%d, block size=%u)\n", packet_len, sd->bl.id, blocksize);
		return;
	}
	count = packet_len / blocksize;

	buyingstore->create(sd, zenylimit, result, storename, itemlist, count);
}

/// Notification, that the requested buying store could not be created (ZC_FAILED_OPEN_BUYING_STORE_TO_BUYER).
/// 0812 <result>.W <total weight>.L
/// result:
///     1 = "Failed to open buying store." (0x6cd, MSI_BUYINGSTORE_OPEN_FAILED)
///     2 = "Total amount of then possessed items exceeds the weight limit by <weight/10-maxweight*90%>. Please re-enter." (0x6ce, MSI_BUYINGSTORE_OVERWEIGHT)
///     9 = "No sale (purchase) information available." (0x705)
///    10 = "Cant open store at this location." (0xC9D)
///     ? = nothing
static void clif_buyingstore_open_failed(struct map_session_data *sd, unsigned short result, unsigned int weight)
{
#if PACKETVER >= 20100420
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x812));
	WFIFOW(fd,0) = 0x812;
	WFIFOW(fd,2) = result;
	WFIFOL(fd,4) = weight;
	WFIFOSET(fd,packet_len(0x812));
#endif
}

/// Notification, that the requested buying store was created (ZC_MYITEMLIST_BUYING_STORE).
/// 0813 <packet len>.W <account id>.L <limit zeny>.L { <price>.L <count>.W <type>.B <name id>.W }*
static void clif_buyingstore_myitemlist(struct map_session_data *sd)
{
	int fd;
	unsigned int i;
	struct PACKET_ZC_MYITEMLIST_BUYING_STORE *p;
	int len;

	nullpo_retv(sd);
	fd = sd->fd;
	len = sizeof(struct PACKET_ZC_MYITEMLIST_BUYING_STORE) + sd->buyingstore.slots * sizeof(struct PACKET_ZC_MYITEMLIST_BUYING_STORE_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x813;
	p->packetLength = len;
	p->AID = sd->bl.id;
	p->zenyLimit = sd->buyingstore.zenylimit;

	for (i = 0; i < sd->buyingstore.slots; i++)
	{
		p->items[i].price = sd->buyingstore.items[i].price;
		p->items[i].amount = sd->buyingstore.items[i].amount;
		p->items[i].itemType = itemtype(itemdb_type(sd->buyingstore.items[i].nameid));
		p->items[i].itemId = sd->buyingstore.items[i].nameid;
	}

	WFIFOSET(fd, len);
}

/// Notifies clients in area of a buying store (ZC_BUYING_STORE_ENTRY).
/// 0814 <account id>.L <store name>.80B
static void clif_buyingstore_entry(struct map_session_data *sd)
{
#if PACKETVER >= 20100420
	uint8 buf[86];

	nullpo_retv(sd);
	WBUFW(buf,0) = 0x814;
	WBUFL(buf,2) = sd->bl.id;
	memcpy(WBUFP(buf,6), sd->message, MESSAGE_SIZE);

	clif->send(buf, packet_len(0x814), &sd->bl, AREA_WOS);
#endif
}
static void clif_buyingstore_entry_single(struct map_session_data *sd, struct map_session_data *pl_sd)
{
#if PACKETVER >= 20100420
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x814));
	WFIFOW(fd,0) = 0x814;
	WFIFOL(fd,2) = pl_sd->bl.id;
	memcpy(WFIFOP(fd,6), pl_sd->message, MESSAGE_SIZE);
	WFIFOSET(fd,packet_len(0x814));
#endif
}

static void clif_parse_ReqCloseBuyingStore(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to close own buying store (CZ_REQ_CLOSE_BUYING_STORE).
/// 0815
static void clif_parse_ReqCloseBuyingStore(int fd, struct map_session_data *sd)
{
	buyingstore->close(sd);
}

/// Notifies clients in area that a buying store was closed (ZC_DISAPPEAR_BUYING_STORE_ENTRY).
/// 0816 <account id>.L
static void clif_buyingstore_disappear_entry(struct map_session_data *sd)
{
#if PACKETVER >= 20100309
	uint8 buf[6];

	nullpo_retv(sd);
	WBUFW(buf,0) = 0x816;
	WBUFL(buf,2) = sd->bl.id;

	clif->send(buf, packet_len(0x816), &sd->bl, AREA_WOS);
#endif
}

static void clif_buyingstore_disappear_entry_single(struct map_session_data *sd, struct map_session_data *pl_sd)
{
#if PACKETVER >= 20100309
	int fd;

	nullpo_retv(sd);
	nullpo_retv(pl_sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x816));
	WFIFOW(fd,0) = 0x816;
	WFIFOL(fd,2) = pl_sd->bl.id;
	WFIFOSET(fd,packet_len(0x816));
#endif
}

/// Request to open someone else's buying store (CZ_REQ_CLICK_TO_BUYING_STORE).
/// 0817 <account id>.L
static void clif_parse_ReqClickBuyingStore(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int account_id;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	buyingstore->open(sd, account_id);
}

/// Sends buying store item list (ZC_ACK_ITEMLIST_BUYING_STORE).
/// 0818 <packet len>.W <account id>.L <store id>.L <limit zeny>.L { <price>.L <amount>.W <type>.B <name id>.W }*
static void clif_buyingstore_itemlist(struct map_session_data *sd, struct map_session_data *pl_sd)
{
	int fd;
	unsigned int i;
	struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE *p;
	int len;

	nullpo_retv(sd);
	nullpo_retv(pl_sd);
	fd = sd->fd;
	len = sizeof(struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE) + pl_sd->buyingstore.slots * sizeof(struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub);
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x818;
	p->packetLength = len;
	p->AID = pl_sd->bl.id;
	p->storeId = pl_sd->buyer_id;
	p->zenyLimit = pl_sd->buyingstore.zenylimit;

	for (i = 0; i < pl_sd->buyingstore.slots; i++)
	{
		p->items[i].price = pl_sd->buyingstore.items[i].price;
		p->items[i].amount = pl_sd->buyingstore.items[i].amount;  // TODO: Figure out, if no longer needed items (amount == 0) are listed on official.
		p->items[i].itemType = itemtype(itemdb_type(pl_sd->buyingstore.items[i].nameid));
		p->items[i].itemId = pl_sd->buyingstore.items[i].nameid;
	}

	WFIFOSET(fd, len);
}

static void clif_parse_ReqTradeBuyingStore(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to sell items to a buying store (CZ_REQ_TRADE_BUYING_STORE).
/// 0819 <packet len>.W <account id>.L <store id>.L { <index>.W <name id>.W <amount>.W }*
static void clif_parse_ReqTradeBuyingStore(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const unsigned int blocksize = sizeof(struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub);
	const struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub *itemlist;
	int account_id;
	unsigned int buyer_id;
	int count, packet_len;
	const struct PACKET_CZ_REQ_TRADE_BUYING_STORE *p = RFIFOP(fd, 0);
	packet_len = p->packetLength;

	if (packet_len < sizeof(struct PACKET_CZ_REQ_TRADE_BUYING_STORE))
	{// minimum packet length
		ShowError("clif_parse_ReqTradeBuyingStore: Malformed packet (expected length=%u, length=%d, account_id=%d).\n", (uint32)sizeof(struct PACKET_CZ_REQ_TRADE_BUYING_STORE), packet_len, sd->bl.id);
		return;
	}

	account_id = p->AID;
	buyer_id   = p->storeId;
	itemlist   = &p->items[0];

	// so that buyingstore_trade knows, how many elements it has access to
	packet_len -= sizeof(struct PACKET_CZ_REQ_TRADE_BUYING_STORE);
	if (packet_len < 0)
		return;

	if (packet_len % blocksize)
	{
		ShowError("clif_parse_ReqTradeBuyingStore: Unexpected item list size %d (account_id=%d, buyer_id=%d, block size=%u)\n", packet_len, sd->bl.id, account_id, blocksize);
		return;
	}
	count = packet_len / blocksize;

	buyingstore->trade(sd, account_id, buyer_id, itemlist, count);
}

/// Notifies the buyer, that the buying store has been closed due to a post-trade condition (ZC_FAILED_TRADE_BUYING_STORE_TO_BUYER).
/// 081a <result>.W
/// result:
///     3 = "All items within the buy limit were purchased." (0x6cf, MSI_BUYINGSTORE_TRADE_OVERLIMITZENY)
///     4 = "All items were purchased." (0x6d0, MSI_BUYINGSTORE_TRADE_BUYCOMPLETE)
///     ? = nothing
static void clif_buyingstore_trade_failed_buyer(struct map_session_data *sd, short result)
{
#if PACKETVER >= 20100420
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x81a));
	WFIFOW(fd,0) = 0x81a;
	WFIFOW(fd,2) = result;
	WFIFOSET(fd,packet_len(0x81a));
#endif
}

/// Updates the zeny limit and an item in the buying store item list (ZC_UPDATE_ITEM_FROM_BUYING_STORE).
/// 081b <name id>.W <amount>.W <limit zeny>.L
static void clif_buyingstore_update_item(struct map_session_data *sd, int nameid, unsigned short amount, uint32 char_id, int zeny)
{
	int fd;
	struct PACKET_ZC_UPDATE_ITEM_FROM_BUYING_STORE p;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = buyingStoreUpdateItemType;
	p.itemId = nameid;
	p.amount = amount;
	p.zenyLimit = sd->buyingstore.zenylimit;
#if PACKETVER >= 20141016
	p.zeny = zeny;
	p.charId = char_id;  // GID
	p.updateTime = (int)time(NULL);
#endif
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

/// Deletes item from inventory, that was sold to a buying store (ZC_ITEM_DELETE_BUYING_STORE).
/// 081c <index>.W <amount>.W <price>.L
/// message:
///     "%s (%d) were sold at %dz." (0x6d2, MSI_BUYINGSTORE_TRADE_SELLCOMPLETE)
///
/// NOTE:   This function has to be called _instead_ of clif_delitem/clif_dropitem.
static void clif_buyingstore_delete_item(struct map_session_data *sd, short index, unsigned short amount, int price)
{
#if PACKETVER >= 20100420
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x81c));
	WFIFOW(fd,0) = 0x81c;
	WFIFOW(fd,2) = index+2;
	WFIFOW(fd,4) = amount;
	WFIFOL(fd,6) = price;  // price per item, client calculates total Zeny by itself
	WFIFOSET(fd,packet_len(0x81c));
#endif
}

/// Notifies the seller, that a buying store trade failed (ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER).
/// 0824 <result>.W <name id>.W
/// result:
///     5 = "The deal has failed." (0x39, MSI_DEAL_FAIL)
///     6 = "The trade failed, because the entered amount of item %s is higher, than the buyer is willing to buy." (0x6d3, MSI_BUYINGSTORE_TRADE_OVERCOUNT)
///     7 = "The trade failed, because the buyer is lacking required balance." (0x6d1, MSI_BUYINGSTORE_TRADE_LACKBUYERZENY)
///     ? = nothing
static void clif_buyingstore_trade_failed_seller(struct map_session_data *sd, short result, int nameid)
{
#if PACKETVER >= 20100420
	int fd;
	struct PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER p;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(p));
	p.packetType = 0x824;
	p.result = result;
	p.itemId = nameid;
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
#endif
}

static void clif_parse_SearchStoreInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Search Store Info System
///

/// Request to search for stores (CZ_SEARCH_STORE_INFO).
/// 0835 <packet len>.W <type>.B <max price>.L <min price>.L <name id count>.B <card count>.B { <name id>.W }* { <card>.W }*
/// type:
///     0 = Vending
///     1 = Buying Store
///
/// NOTE:   The client determines the item ids by specifying a name and optionally,
///         amount of card slots. If the client does not know about the item it
///         cannot be searched.
static void clif_parse_SearchStoreInfo(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const unsigned int blocksize = sizeof(struct PACKET_CZ_SEARCH_STORE_INFO_item);
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist;
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist;
	unsigned char type;
	unsigned int min_price, max_price;
	int packet_len, count, item_count, card_count;
	int i;
	const struct PACKET_CZ_SEARCH_STORE_INFO *p = RFIFOP(fd, 0);
	uint32 *items_list;
	uint32 *cards_list;

	packet_len = p->packetLength;

	if (packet_len < sizeof(struct PACKET_CZ_SEARCH_STORE_INFO))
	{// minimum packet length
		ShowError("clif_parse_SearchStoreInfo: Malformed packet (expected length=%u, length=%d, account_id=%d).\n", (uint32)sizeof(struct PACKET_CZ_SEARCH_STORE_INFO), packet_len, sd->bl.id);
		return;
	}

	if (p->itemsCount < 1)
		return; // Should never happen

	type       = p->searchType;
	max_price  = p->maxPrice;
	min_price  = p->minPrice;
	item_count = p->itemsCount;
	card_count = p->cardsCount;
	itemlist   = &p->items[0];

	// check, if there is enough data for the claimed count of items
	packet_len -= sizeof(struct PACKET_CZ_SEARCH_STORE_INFO);

	if (packet_len < 0)
		return;

	if (packet_len % blocksize)
	{
		ShowError("clif_parse_SearchStoreInfo: Unexpected item list size %d (account_id=%d, block size=%u)\n", packet_len, sd->bl.id, blocksize);
		return;
	}
	count = packet_len / blocksize;

	if (count < item_count + card_count)
	{
		ShowError("clif_parse_SearchStoreInfo: Malformed packet (expected count=%d, count=%d, account_id=%d).\n", item_count + card_count, count, sd->bl.id);
		return;
	}

	cardlist = RFIFOP(fd, sizeof(struct PACKET_CZ_SEARCH_STORE_INFO) + blocksize * item_count);

	items_list = aMalloc(sizeof(uint32) * item_count);
	cards_list = aMalloc(sizeof(uint32) * card_count);
	for (i = 0; i < item_count; i ++) {
		items_list[i] = itemlist[i].itemId;
	}
	for (i = 0; i < card_count; i ++) {
		cards_list[i] = cardlist[i].itemId;
	}
	// TODO: add search by item options.
	searchstore->query(sd, type, min_price, max_price, items_list, item_count, cards_list, card_count);
	aFree(items_list);
	aFree(cards_list);
}

/// Results for a store search request (ZC_SEARCH_STORE_INFO_ACK).
/// 0836 <packet len>.W <is first page>.B <is next page>.B <remaining uses>.B { <store id>.L <account id>.L <shop name>.80B <nameid>.W <item type>.B <price>.L <amount>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }*
/// is first page:
///     0 = appends to existing results
///     1 = clears previous results before displaying this result set
/// is next page:
///     0 = no "next" label
///     1 = "next" label to retrieve more results
static void clif_search_store_info_ack(struct map_session_data *sd)
{
	const unsigned int blocksize = sizeof(struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub);
	int fd;
	unsigned int i, start, end;
	struct PACKET_ZC_SEARCH_STORE_INFO_ACK *p;
	int len;

	nullpo_retv(sd);
	fd = sd->fd;
	start = sd->searchstore.pages * SEARCHSTORE_RESULTS_PER_PAGE;
	end   = min(sd->searchstore.count, start + SEARCHSTORE_RESULTS_PER_PAGE);

	len = sizeof(struct PACKET_ZC_SEARCH_STORE_INFO_ACK) + (end - start) * blocksize;
	WFIFOHEAD(fd, len);
	p = WFIFOP(fd, 0);
	p->packetType = 0x836;
	p->packetLength = len;
	p->firstPage = !sd->searchstore.pages;
	p->nextPage = searchstore->querynext(sd);
	p->usesCount = (unsigned char)min(sd->searchstore.uses, UINT8_MAX);

	for (i = start; i < end; i++) {
		struct s_search_store_info_item* ssitem = &sd->searchstore.items[i];
		struct item it;

		p->items[i].storeId = ssitem->store_id;
		p->items[i].AID = ssitem->account_id;
		memcpy(&p->items[i].shopName, ssitem->store_name, MESSAGE_SIZE);
		p->items[i].itemId = ssitem->nameid;
		p->items[i].itemType = itemtype(itemdb_type(ssitem->nameid));
		p->items[i].price = ssitem->price;
		p->items[i].amount = ssitem->amount;
		p->items[i].refine = ssitem->refine;

		// make-up an item for clif_addcards
		memset(&it, 0, sizeof(it));
		memcpy(&it.card, &ssitem->card, sizeof(it.card));
		it.nameid = ssitem->nameid;
		it.amount = ssitem->amount;

		clif->addcards(&p->items[i].slot, &it);
#if PACKETVER >= 20150226
		memcpy(&it.option, &ssitem->option, sizeof(it.option));
		clif->add_item_options(&p->items[i].option_data[0], &it);
#endif
	}

	WFIFOSET(fd, len);
}

/// Notification of failure when searching for stores (ZC_SEARCH_STORE_INFO_FAILED).
/// 0837 <reason>.B
/// reason:
///     0 = "No matching stores were found." (0x70b)
///     1 = "There are too many results. Please enter more detailed search term." (0x6f8)
///     2 = "You cannot search anymore." (0x706)
///     3 = "You cannot search yet." (0x708)
///     4 = "No sale (purchase) information available." (0x705)
///     362 = silent error
static void clif_search_store_info_failed(struct map_session_data *sd, unsigned char reason)
{
#if PACKETVER >= 20100601
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x837));
	WFIFOW(fd,0) = 0x837;
	WFIFOB(fd,2) = reason;
	WFIFOSET(fd,packet_len(0x837));
#endif
}

static void clif_parse_SearchStoreInfoNextPage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to display next page of results (CZ_SEARCH_STORE_INFO_NEXT_PAGE).
/// 0838
static void clif_parse_SearchStoreInfoNextPage(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	searchstore->next(sd);
}

/// Opens the search store window (ZC_OPEN_SEARCH_STORE_INFO).
/// 083a <type>.W <remaining uses>.B
/// type:
///     0 = Search Stores
///     1 = Search Stores (Cash), asks for confirmation, when clicking a store
static void clif_open_search_store_info(struct map_session_data *sd)
{
#if PACKETVER >= 20100608
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x83a));
	WFIFOW(fd,0) = 0x83a;
	WFIFOW(fd,2) = sd->searchstore.effect;
#if PACKETVER > 20100701
	WFIFOB(fd,4) = (unsigned char)min(sd->searchstore.uses, UINT8_MAX);
#endif
	WFIFOSET(fd,packet_len(0x83a));
#endif
}

static void clif_parse_CloseSearchStoreInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to close the store search window (CZ_CLOSE_SEARCH_STORE_INFO).
/// 083b
static void clif_parse_CloseSearchStoreInfo(int fd, struct map_session_data *sd)
{
	searchstore->close(sd);
}

static void clif_parse_SearchStoreInfoListItemClick(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Request to invoke catalog effect on a store from search results (CZ_SSILIST_ITEM_CLICK).
/// 083c <account id>.L <store id>.L <nameid>.W
static void clif_parse_SearchStoreInfoListItemClick(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_SSILIST_ITEM_CLICK *p = RFIFOP(fd, 0);
	searchstore->click(sd, p->AID, p->storeId, p->itemId);
}

/// Notification of the store position on current map (ZC_SSILIST_ITEM_CLICK_ACK).
/// 083d <xPos>.W <yPos>.W
static void clif_search_store_info_click_ack(struct map_session_data *sd, short x, short y)
{
#if PACKETVER >= 20100608
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x83d));
	WFIFOW(fd,0) = 0x83d;
	WFIFOW(fd,2) = x;
	WFIFOW(fd,4) = y;
	WFIFOSET(fd,packet_len(0x83d));
#endif
}

/// Parse function for packet debugging.
static void clif_parse_debug(int fd, struct map_session_data *sd)
{
	int cmd, packet_len;

	// clif_parse ensures, that there is at least 2 bytes of data
	cmd = RFIFOW(fd,0);

	if( sd ) {
		packet_len = packets->db[cmd];

		if( packet_len == -1 ) {// variable length
			packet_len = RFIFOW(fd,2);  // clif_parse ensures, that this amount of data is already received
		}
		ShowDebug("Packet debug of 0x%04X (length %d), %s session #%d, %d/%d (AID/CID)\n", (unsigned int)cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id);
	} else {
		packet_len = (int)RFIFOREST(fd);
		ShowDebug("Packet debug of 0x%04X (length %d), session #%d\n", (unsigned int)cmd, packet_len, fd);
	}

	ShowDump(RFIFOP(fd,0), packet_len);
}
/*==========================================
 * Server tells client to display a window similar to Magnifier (item) one
 * Server populates the window with available elemental converter options according to player's inventory
 *------------------------------------------*/
static int clif_elementalconverter_list(struct map_session_data *sd)
{
	nullpo_ret(sd);

/// Main client packet processing function
	int fd = sd->fd;
	int len = MAX_SKILL_ARROW_DB * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MAKINGARROW_LIST *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGARROW_LIST;

	int c = 0;
	for (int i = 0; i < MAX_SKILL_PRODUCE_DB; i++) {
		if (skill->can_produce_mix(sd,skill->dbs->produce_db[i].nameid,23, 1) ) {
			int view = itemdb_viewid(skill->dbs->produce_db[i].nameid);
			if (view > 0)
				p->items[c].itemId = view;
			else
				p->items[c].itemId = skill->dbs->produce_db[i].nameid;
			c++;
		}
	}
	if (c > 0) {
		len = c * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
		p->packetLength = len;
		WFIFOSET(fd, len);
		sd->menuskill_id = SA_CREATECON;
		sd->menuskill_val = c;
	}

	return 0;
}
/**
 * Rune Knight
 **/
static void clif_millenniumshield(struct block_list *bl, short shields)
{
#if PACKETVER >= 20081217
	unsigned char buf[10];

	nullpo_retv(bl);
	WBUFW(buf,0) = 0x440;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = shields;
	WBUFW(buf,8) = 0;
	clif->send(buf,packet_len(0x440),bl,AREA);
#endif
}
/**
 * Warlock
 **/
/*==========================================
 * Spellbook list [LimitLine/3CeAM]
 *------------------------------------------*/
static int clif_spellbook_list(struct map_session_data *sd)
{
	nullpo_ret(sd);

	int fd = sd->fd;
	int len = MAX_SKILL_ARROW_DB * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MAKINGARROW_LIST *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGARROW_LIST;

	int c = 0;
	for (int i = 0; i < sd->status.inventorySize; i ++ )
	{
		if (itemdb_is_spellbook(sd->status.inventory[i].nameid))
		{
			p->items[c].itemId = sd->status.inventory[i].nameid;
			c++;
		}
	}

	if (c > 0)
	{
		len = c * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
		p->packetLength = len;
		WFIFOSET(fd, len);
		sd->menuskill_id = WL_READING_SB;
		sd->menuskill_val = c;
	} else {
		status_change_end(&sd->bl, SC_STOP, INVALID_TIMER);
		clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK, 0, 0);
	}

	return 1;
}
/**
 * Mechanic
 **/
/*==========================================
 * Magic Decoy Material List
 *------------------------------------------*/
static int clif_magicdecoy_list(struct map_session_data *sd, uint16 skill_lv, short x, short y)
{
	int i, c;

	nullpo_ret(sd);

	int fd = sd->fd;
	int len = MAX_SKILL_ARROW_DB * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MAKINGARROW_LIST *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGARROW_LIST;

	for (i = 0, c = 0; i < sd->status.inventorySize; i ++) {
		if (itemdb_is_element(sd->status.inventory[i].nameid)) {
			p->items[c].itemId = sd->status.inventory[i].nameid;
			c ++;
		}
	}
	if( c > 0 ) {
		sd->menuskill_id = NC_MAGICDECOY;
		sd->menuskill_val = skill_lv;
		sd->sc.comet_x = x;
		sd->sc.comet_y = y;

		len = c * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
		p->packetLength = len;
		WFIFOSET(fd, len);
	} else {
		clif->skill_fail(sd, NC_MAGICDECOY, USESKILL_FAIL_LEVEL, 0, 0);
		return 0;
	}

	return 1;
}
/**
 * Guilotine Cross
 **/
/*==========================================
 * Guillotine Cross Poisons List
 *------------------------------------------*/
static int clif_poison_list(struct map_session_data *sd, uint16 skill_lv)
{
	int i, c;

	nullpo_ret(sd);

	int fd = sd->fd;
	int len = MAX_SKILL_ARROW_DB * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MAKINGARROW_LIST *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MAKINGARROW_LIST;

	for (i = 0, c = 0; i < sd->status.inventorySize; i ++) {
		if( itemdb_is_poison(sd->status.inventory[i].nameid) ) {
			p->items[c].itemId = sd->status.inventory[i].nameid;
			c ++;
		}
	}
	if (c > 0) {
		sd->menuskill_id = GC_POISONINGWEAPON;
		sd->menuskill_val = skill_lv;

		len = c * sizeof(struct PACKET_ZC_MAKINGARROW_LIST_sub) + sizeof(struct PACKET_ZC_MAKINGARROW_LIST);
		p->packetLength = len;
		WFIFOSET(fd, len);
	} else {
		clif->skill_fail(sd, GC_POISONINGWEAPON, USESKILL_FAIL_GUILLONTINE_POISON, 0, 0);
		return 0;
	}

	return 1;
}
static int clif_autoshadowspell_list(struct map_session_data *sd)
{
	int fd, i, c;
	nullpo_ret(sd);
	fd = sd->fd;
	if( !fd ) return 0;

	if( sd->menuskill_id == SC_AUTOSHADOWSPELL )
		return 0;

	WFIFOHEAD(fd, 2 * 6 + 4);
	WFIFOW(fd,0) = 0x442;
	for (i = 0, c = 0; i < MAX_SKILL_DB; i++)
		if (sd->status.skill[i].flag == SKILL_FLAG_PLAGIARIZED && sd->status.skill[i].id > 0 && sd->status.skill[i].id < GS_GLITTERING
		    && skill->get_type(sd->status.skill[i].id, sd->status.skill[i].lv) == BF_MAGIC) {
			// Can't auto cast both Extended class and 3rd class skills.
			WFIFOW(fd,8+c*2) = sd->status.skill[i].id;
			c++;
		}

	if( c > 0 ) {
		WFIFOW(fd,2) = 8 + c * 2;
		WFIFOL(fd,4) = c;
		WFIFOSET(fd,WFIFOW(fd,2));
		sd->menuskill_id = SC_AUTOSHADOWSPELL;
		sd->menuskill_val = c;
	} else {
		status_change_end(&sd->bl,SC_STOP,INVALID_TIMER);
		clif->skill_fail(sd, SC_AUTOSHADOWSPELL, USESKILL_FAIL_IMITATION_SKILL_NONE, 0, 0);
	}

	return 1;
}
/*===========================================
 * Skill list for Four Elemental Analysis
 * and Change Material skills.
 *------------------------------------------*/
static int clif_skill_itemlistwindow(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
#if PACKETVER >= 20090922
	int fd;

	nullpo_ret(sd);

	sd->menuskill_id = skill_id; // To prevent hacking.
	sd->menuskill_val = skill_lv;

	if( skill_id == GN_CHANGEMATERIAL )
		skill_lv = 0; // Changematerial

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x7e3));
	WFIFOW(fd,0) = 0x7e3;
	WFIFOL(fd,2) = skill_lv;
	WFIFOL(fd,4) = 0;
	WFIFOSET(fd,packet_len(0x7e3));

#endif

	return 1;

}

static void clif_parse_SkillSelectMenu(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/*==========================================
 * used by SC_AUTOSHADOWSPELL
 * RFIFOL(fd,2) - flag (currently not used)
 *------------------------------------------*/
static void clif_parse_SkillSelectMenu(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (sd->menuskill_id != SC_AUTOSHADOWSPELL)
		return;

	if (pc_istrading_except_npc(sd) || sd->state.prevend != 0 || (sd->npc_id != 0 && sd->state.using_megaphone == 0)) {
		clif->skill_fail(sd, sd->ud.skill_id, 0, 0, 0);
		clif_menuskill_clear(sd);
		return;
	}

	skill->select_menu(sd,RFIFOW(fd,6));

	clif_menuskill_clear(sd);
}

/*==========================================
 * Kagerou/Oboro amulet spirit
 *------------------------------------------*/
static void clif_charm(struct map_session_data *sd)
{
#if PACKETVER >= 20110809
	unsigned char buf[10];

	nullpo_retv(sd);

	WBUFW(buf,0) = 0x08cf;
	WBUFL(buf,2) = sd->bl.id;
	WBUFW(buf,6) = sd->charm_type;
	WBUFW(buf,8) = sd->charm_count;
	clif->send(buf,packet_len(0x08cf),&sd->bl,AREA);
#endif
}

static void clif_parse_MoveItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/// Move Item from or to Personal Tab (CZ_WHATSOEVER) [FE]
/// 0907 <index>.W
///
/// R 0908 <index>.w <type>.b
/// type:
///   0 = move item to personal tab
///   1 = move item to normal tab
static void clif_parse_MoveItem(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20111122
	int index;

	/* can't move while dead. */
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	index = RFIFOW(fd,2)-2;

	if (index < 0 || index >= sd->status.inventorySize)
		return;

	if ( sd->status.inventory[index].favorite && RFIFOB(fd, 4) == 1 )
		sd->status.inventory[index].favorite = 0;
	else if( RFIFOB(fd, 4) == 0 )
		sd->status.inventory[index].favorite = 1;
	else
		return;/* nothing to do. */

	clif->favorite_item(sd, index);
#endif
}

/* [Ind/Hercules] */
static void clif_cashshop_db(void)
{
	struct config_t cashshop_conf;
	struct config_setting_t *cashshop = NULL, *cats = NULL;
	char config_filename[256];
	libconfig->format_db_path("cashshop_db.conf", config_filename, sizeof(config_filename));
	int i, item_count_t = 0;
	for( i = 0; i < CASHSHOP_TAB_MAX; i++ ) {
		CREATE(clif->cs.data[i], struct hCSData *, 1);
		clif->cs.item_count[i] = 0;
	}

	if (!libconfig->load_file(&cashshop_conf, config_filename))
		return;

	cashshop = libconfig->lookup(&cashshop_conf, "cash_shop");

	if( cashshop != NULL && (cats = libconfig->setting_get_elem(cashshop, 0)) != NULL ) {
		for(i = 0; i < CASHSHOP_TAB_MAX; i++) {
			struct config_setting_t *cat;
			char entry_name[15];

			sprintf(entry_name,"cat_%d",i);

			if( (cat = libconfig->setting_get_member(cats, entry_name)) != NULL ) {
				int k, item_count = libconfig->setting_length(cat);

				for(k = 0; k < item_count; k++) {
					struct config_setting_t *entry = libconfig->setting_get_elem(cat,k);
					const char *name = config_setting_name(entry);
					int price = libconfig->setting_get_int(entry);
					struct item_data * data = NULL;

					if( price < 1 ) {
						ShowWarning("cashshop_db: unsupported price '%d' for entry named '%s' in category '%s'\n", price, name, entry_name);
						continue;
					}

					if (name[0] == 'I' && name[1] == 'D' && strlen(name) <= 12) {
						if( !( data = itemdb->exists(atoi(name+2))) ) {
							ShowWarning("cashshop_db: unknown item id '%s' in category '%s'\n", name+2, entry_name);
							continue;
						}
					} else {
						if( !( data = itemdb->search_name(name) ) ) {
							ShowWarning("cashshop_db: unknown item name '%s' in category '%s'\n", name, entry_name);
							continue;
						}
					}

					RECREATE(clif->cs.data[i], struct hCSData *, ++clif->cs.item_count[i]);
					CREATE(clif->cs.data[i][ clif->cs.item_count[i] - 1 ], struct hCSData , 1);

					clif->cs.data[i][ clif->cs.item_count[i] - 1 ]->id = data->nameid;
					clif->cs.data[i][ clif->cs.item_count[i] - 1 ]->price = price;
					item_count_t++;
				}
			}
		}

	}
	libconfig->destroy(&cashshop_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", item_count_t, config_filename);
}
/// Items that are in favorite tab of inventory (ZC_ITEM_FAVORITE).
/// 0900 <index>.W <favorite>.B
static void clif_favorite_item(struct map_session_data *sd, unsigned short index)
{
#if PACKETVER >= 20120410
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x908));
	WFIFOW(fd,0) = 0x908;
	WFIFOW(fd,2) = index+2;
	WFIFOB(fd,4) = (sd->status.inventory[index].favorite == 1) ? 0 : 1;
	WFIFOSET(fd,packet_len(0x908));
#endif
}

static void clif_snap(struct block_list *bl, short x, short y)
{
#if PACKETVER >= 20110809
	unsigned char buf[10];

	nullpo_retv(bl);
	WBUFW(buf,0) = 0x8d2;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = x;
	WBUFW(buf,8) = y;

	clif->send(buf,packet_len(0x8d2),bl,AREA);
#endif
}

static void clif_monster_hp_bar(struct mob_data *md, struct map_session_data *sd)
{
#if PACKETVER >= 20120228
	struct packet_monster_hp p;

	nullpo_retv(md);
	nullpo_retv(sd);
	p.PacketType = monsterhpType;
	p.GID = md->bl.id;
	p.HP = md->status.hp;
	p.MaxHP = md->status.max_hp;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static bool clif_show_monster_hp_bar(struct block_list *bl)
{
	nullpo_ret(bl);

	if (bl->type != BL_MOB)
		return false;

	const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
	if (status_get_hp(bl) < status_get_max_hp(bl)) {
		if ((battle->bc->show_monster_hp_bar & 1) != 0 && md->class_ != MOBID_EMPELIUM && (md->status.mode & MD_BOSS) == 0)
			return true;
		if ((battle->bc->show_monster_hp_bar & 2) != 0 && md->class_ == MOBID_EMPELIUM)
			return true;
		if ((battle->bc->show_monster_hp_bar & 4) != 0 && (md->status.mode & MD_BOSS) != 0 && md->class_ != MOBID_EMPELIUM)
			return true;
	}
	return false;
}

/* [Ind/Hercules] placeholder for unsupported incoming packets (avoids server disconnecting client) */
static void clif_parse_dull(int fd, struct map_session_data *sd)
{
	const int cmd = clif->cmd;
	Assert_retv(cmd <= MAX_PACKET_DB && cmd >= MIN_PACKET_DB);

	int packet_len = packets->db[cmd];
	if (packet_len == -1) { // variable-length packet
		packet_len = RFIFOW(fd, 2);
	}
	if (sd) {
		ShowWarning("Unhandled packet 0x%04X (length %d), %s session #%d, %d/%d (AID/CID)\n", (uint32)cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id);
	} else {
		ShowWarning("Unhandled packet 0x%04X (length %d), session #%d\n", (uint32)cmd, packet_len, fd);
	}
	return;
}

static void clif_parse_cashShopOpen1(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopOpen1(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.nocashshop) {
		clif->messagecolor_self(fd, COLOR_RED, msg_fd(fd, 1489)); //Cash Shop is disabled in this map
		return;
	}

	clif->cashShopOpen(fd, sd, 0);
}

static void clif_parse_cashShopLimitedReq(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopLimitedReq(int fd, struct map_session_data *sd)
{
}

static void clif_parse_cashShopOpen2(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopOpen2(int fd, struct map_session_data *sd)
{
	if (sd->state.trading != 0 || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (map->list[sd->bl.m].flag.nocashshop != 0) {
		clif->messagecolor_self(fd, COLOR_RED, msg_fd(fd, 1489)); //Cash Shop is disabled in this map
		return;
	}

#if PACKETVER >= 20191224
	const struct PACKET_CZ_SE_CASHSHOP_OPEN2 *p = RFIFOP(fd, 0);
	clif->cashShopOpen(fd, sd, p->tab);
#endif
}

static void clif_cashShopOpen(int fd, struct map_session_data *sd, int tab)
{
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_SE_CASHSHOP_OPEN));
	struct PACKET_ZC_SE_CASHSHOP_OPEN *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_SE_CASHSHOP_OPEN;
	p->cashPoints = sd->cashPoints;  //[Ryuuzaki] - switched positions to reflect proper values
	p->kafraPoints = sd->kafraPoints;
#if PACKETVER_MAIN_NUM >= 20200129 || PACKETVER_RE_NUM >= 20200205 || PACKETVER_ZERO_NUM >= 20191224
	p->tab = tab;
#endif
	WFIFOSET(fd, sizeof(struct PACKET_ZC_SE_CASHSHOP_OPEN));
#endif
}

static void clif_parse_cashShopClose(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopClose(int fd, struct map_session_data *sd)
{
	/* TODO apply some state tracking */
}

static void clif_parse_cashShopSchedule(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopSchedule(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	clif->cashShopSchedule(fd, sd);
}

void clif_cashShopSchedule(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20110614
	int i = 0;

	nullpo_retv(sd);
	for (i = 0; i < CASHSHOP_TAB_MAX; i++) {
		const int len = sizeof(struct PACKET_ZC_ACK_SCHEDULER_CASHITEM) + (clif->cs.item_count[i] * sizeof(struct PACKET_ZC_ACK_SCHEDULER_CASHITEM_sub));
		int j = 0;
		struct PACKET_ZC_ACK_SCHEDULER_CASHITEM *p;
		if (clif->cs.item_count[i] == 0)
			continue; // Skip empty tabs, the client only expects filled ones

		WFIFOHEAD(fd, len);
		p = WFIFOP(fd, 0);
		p->packetType = 0x8ca;
		p->packetLength = len;
		p->count = clif->cs.item_count[i];
		p->tabNum = i;

		for (j = 0; j < clif->cs.item_count[i]; j++) {
			const int itemId = clif->cs.data[i][j]->id;
			p->items[j].itemId = itemId;
			p->items[j].price = clif->cs.data[i][j]->price;
#ifdef ENABLE_CASHSHOP_PREVIEW_PATCH
			if (itemId == 0) {
				p->items[j].location = 0;
				p->items[j].viewSprite = 0;
				continue;
			}
			struct item_data *data = itemdb->search(itemId);
			if (data == NULL) {
				p->items[j].location = 0;
				p->items[j].viewSprite = 0;
				continue;
			}
			p->items[j].location = pc->item_equippoint(sd, data);
			p->items[j].viewSprite = data->view_sprite;
#endif  // ENABLE_CASHSHOP_PREVIEW_PATCH
		}

		WFIFOSET(fd, len);
	}
#endif
}

/// R 0848 <len>.W <limit>.W <kafra pay>.L (<item id>.L <amount>.L <tab>.W)*
static void clif_parse_cashShopBuy(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cashShopBuy(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int len = RFIFOW(fd, 2);
	unsigned short limit, i, j;
	unsigned int kafra_pay;
	int count;

	if (map->list[sd->bl.m].flag.nocashshop) {
		clif->messagecolor_self(fd, COLOR_RED, msg_fd(fd,1489)); //Cash Shop is disabled in this map
		return;
	}

	if (len < 10)
		return;

	limit = RFIFOW(fd, 4);
	kafra_pay = RFIFOL(fd, 6); // [Ryuuzaki] - These are free cash points (strangely #CASH = main cash currently for us, confusing)
	count = (len - 10) / 10;
	if (count != limit) {
		ShowError("Wrong cash shop limit: %d\n", limit);
		return;
	}

	for(i = 0; i < limit; i++) {
		int qty = RFIFOL(fd, 14 + ( i * 10 ));
		int id = RFIFOL(fd, 10 + ( i * 10 ));
		short tab = RFIFOW(fd, 18 + ( i * 10 ));
		enum CASH_SHOP_BUY_RESULT result = CSBR_UNKNOWN;

		if(tab < 0 || tab >= CASHSHOP_TAB_MAX)
			continue;

		for(j = 0; j < clif->cs.item_count[tab]; j++) {
			if( clif->cs.data[tab][j]->id == id )
				break;
		}
		if(j < clif->cs.item_count[tab]) {
			struct item_data *data;
			if(sd->kafraPoints < kafra_pay) {
				result = CSBR_SHORTTAGE_CASH;
			} else if((sd->cashPoints+kafra_pay) < (clif->cs.data[tab][j]->price * qty)) {
				result = CSBR_SHORTTAGE_CASH;
			} else if (!(data = itemdb->exists(clif->cs.data[tab][j]->id))) {
				result = CSBR_UNKONWN_ITEM;
			} else {
				struct item item_tmp;
				int k, get_count;
				int ret = 0;

				get_count = qty;

				if (!itemdb->isstackable2(data))
					get_count = 1;

				ret = pc->paycash(sd, clif->cs.data[tab][j]->price * qty, kafra_pay);// [Ryuuzaki] //changed Kafrapoints calculation. [Normynator]
				if (ret < 0) {
					ShowError("clif_parse_cashShopBuy: The return from pc->paycash was negative which is not allowed.\n");
					break; //This should never happen.
				}
				kafra_pay = ret;
				for (k = 0; k < qty; k += get_count) {
					if (!pet->create_egg(sd, data->nameid)) {
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = data->nameid;
						item_tmp.identify = 1;

						switch (pc->additem(sd, &item_tmp, get_count, LOG_TYPE_NPC)) {
							case 0:
								result = CSBR_SUCCESS;
								break;
							case 1:
								result = CSBR_EACHITEM_OVERCOUNT;
								break;
							case 2:
								result = CSBR_INVENTORY_WEIGHT;
								break;
							case 4:
								result = CSBR_INVENTORY_ITEMCNT;
								break;
							case 5:
								result = CSBR_EACHITEM_OVERCOUNT;
								break;
							case 7:
								result = CSBR_RUNE_OVERCOUNT;
								break;
						}

						if( result != CSBR_SUCCESS )
							pc->getcash(sd, clif->cs.data[tab][j]->price * get_count,0);
					} else /* create_egg succeeded so mark as success */
						result = CSBR_SUCCESS;
				}
			}
		} else {
			result = CSBR_UNKONWN_ITEM;
		}
		clif->cashShopBuyAck(fd, sd, id, result);

	}
}

static void clif_cashShopBuyAck(int fd, struct map_session_data *sd, int itemId, enum CASH_SHOP_BUY_RESULT result)
{
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT));
	struct PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT *p = WFIFOP(fd, 0);
	p->packetType = 0x849;
	p->itemId = itemId;
	p->result = result;
	p->cashPoints = sd->cashPoints;
	p->kafraPoints = sd->kafraPoints;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT));
#endif
}

static void clif_parse_cashShopReqTab(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/* [Ind/Hercules] */
static void clif_parse_cashShopReqTab(int fd, struct map_session_data *sd)
{
// [4144] packet exists only in 2011 and was dropped after
#if PACKETVER >= 20110222 && PACKETVER < 20120000
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	short tab = RFIFOW(fd, 2);
	int j;

	if( tab < 0 || tab >= CASHSHOP_TAB_MAX || clif->cs.item_count[tab] == 0 )
		return;

	WFIFOHEAD(fd, 10 + ( clif->cs.item_count[tab] * 6 ) );
	WFIFOW(fd, 0) = 0x8c0;
	WFIFOW(fd, 2) = 10 + ( clif->cs.item_count[tab] * 6 );
	WFIFOL(fd, 4) = tab;
	WFIFOW(fd, 8) = clif->cs.item_count[tab];

	for( j = 0; j < clif->cs.item_count[tab]; j++ ) {
		WFIFOW(fd, 10 + ( 6 * j ) ) = clif->cs.data[tab][j]->id;
		WFIFOL(fd, 12 + ( 6 * j ) ) = clif->cs.data[tab][j]->price;
	}

	WFIFOSET(fd, 10 + ( clif->cs.item_count[tab] * 6 ));
#endif
}

/* [Ind/Hercules] */
static void clif_maptypeproperty2(struct block_list *bl, enum send_target t)
{
#if PACKETVER >= 20121010
	struct packet_maptypeproperty2 p;
	nullpo_retv(bl);

	p.PacketType = maptypeproperty2Type;
	p.type = 0x28;
	p.flag.party = map->list[bl->m].flag.pvp ? 1 : 0; //PARTY
	p.flag.guild = (map->list[bl->m].flag.battleground || map_flag_gvg(bl->m)) ? 1 : 0; // GUILD
	p.flag.siege = (map->list[bl->m].flag.battleground || map_flag_gvg2(bl->m)) ? 1: 0; // SIEGE
	p.flag.mineffect = map_flag_gvg2(bl->m) ? 1 : 0; // USE_SIMPLE_EFFECT - Automatically enable /mineffect in guild arenas and castles.
	p.flag.nolockon = 0; // DISABLE_LOCKON - TODO
	p.flag.countpk = map->list[bl->m].flag.pvp ? 1 : 0; // COUNT_PK
	p.flag.nopartyformation = map->list[bl->m].flag.partylock ? 1 : 0; // NO_PARTY_FORMATION
	p.flag.bg = map->list[bl->m].flag.battleground ? 1 : 0; // BATTLEFIELD
	p.flag.nocostume = (map->list[bl->m].flag.noviewid & EQP_COSTUME) ? 1 : 0; // DISABLE_COSTUMEITEM - Disables Costume Sprite
	p.flag.usecart = 1; // USECART - TODO
	p.flag.summonstarmiracle = 0; // SUNMOONSTAR_MIRACLE - TODO
	p.flag.SpareBits = 0; // UNUSED

	clif->send(&p,sizeof(p),bl,t);
#endif
}

static void clif_status_change2(struct block_list *bl, int tid, enum send_target target, int type, int val1, int val2, int val3)
{
	struct packet_status_change2 p;

	p.PacketType = status_change2Type;
	p.index = type;
	p.AID = tid;
	p.state = 1;
	p.Left = 9999;
	p.val1 = val1;
	p.val2 = val2;
	p.val3 = val3;

	clif->send(&p,sizeof(p), bl, target);
}

static void clif_partytickack(struct map_session_data *sd, bool flag)
{
#if PACKETVER_MAIN_NUM >= 20070911 || defined(PACKETVER_RE) || PACKETVER_AD_NUM >= 20070911 || PACKETVER_SAK_NUM >= 20070904 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	WFIFOHEAD(sd->fd, sizeof(struct PACKET_ZC_PARTY_CONFIG));
	struct PACKET_ZC_PARTY_CONFIG *p = WFIFOP(sd->fd, 0);
	p->packetType = HEADER_ZC_PARTY_CONFIG;
	p->denyPartyInvites = flag;
	WFIFOSET(sd->fd, sizeof(struct PACKET_ZC_PARTY_CONFIG));
#endif
}

static void clif_ShowScript(struct block_list *bl, const char *message, enum send_target target)
{
#if PACKETVER >= 20110111
	char buf[256];
	int len;
	nullpo_retv(bl);

	if (message == NULL)
		return;

	len = (int)strlen(message)+1;

	if (len > (int)sizeof(buf)-8) {
		ShowWarning("clif_ShowScript: Truncating too long message '%s' (len=%d).\n", message, len);
		len = (int)sizeof(buf)-8;
	}

	WBUFW(buf,0) = 0x8b3;
	WBUFW(buf,2) = len+8;
	WBUFL(buf,4) = bl->id;
	safestrncpy(WBUFP(buf,8),message,len);
	clif->send(buf, WBUFW(buf,2), bl, target);
#endif
}

static void clif_status_change_end(struct block_list *bl, int tid, enum send_target target, int type)
{
	struct packet_status_change_end p;

	nullpo_retv(bl);

	if (bl->type == BL_PC && !BL_UCAST(BL_PC, bl)->state.active)
		return;

	p.PacketType = status_change_endType;
	p.index = type;
	p.AID = tid;
	p.state = 0;

	clif->send(&p,sizeof(p), bl, target);
}

static void clif_bgqueue_ack(struct map_session_data *sd, enum BATTLEGROUNDS_QUEUE_ACK response, unsigned char arena_id)
{
	switch (response) {
		case BGQA_FAIL_COOLDOWN:
		case BGQA_FAIL_DESERTER:
		case BGQA_FAIL_TEAM_COUNT:
			break;
		default: {
			struct packet_bgqueue_ack p;

			nullpo_retv(sd);
			p.PacketType = bgqueue_ackType;
			p.type = response;
			safestrncpy(p.bg_name, bg->arena[arena_id]->name, sizeof(p.bg_name));

			clif->send(&p,sizeof(p), &sd->bl, SELF);
		}
			break;
	}
}

static void clif_bgqueue_notice_delete(struct map_session_data *sd, enum BATTLEGROUNDS_QUEUE_NOTICE_DELETED response, const char *name)
{
	struct packet_bgqueue_notice_delete p;

	nullpo_retv(sd);
	p.PacketType = bgqueue_notice_deleteType;
	p.type = response;
	safestrncpy(p.bg_name, name, sizeof(p.bg_name));

	clif->send(&p,sizeof(p), &sd->bl, SELF);
}

static void clif_parse_bgqueue_register(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_bgqueue_register(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_bgqueue_register *p = RP2PTR(fd);
	struct bg_arena *arena = NULL;
	if( !bg->queue_on ) return; /* temp, until feature is complete */

	if( !(arena = bg->name2arena(p->bg_name)) ) {
		clif->bgqueue_ack(sd,BGQA_FAIL_BGNAME_INVALID,0);
		return;
	}

	switch( (enum bg_queue_types)p->type ) {
		case BGQT_INDIVIDUAL:
		case BGQT_PARTY:
		case BGQT_GUILD:
			break;
		default:
			clif->bgqueue_ack(sd,BGQA_FAIL_TYPE_INVALID, arena->id);
			return;
	}

	bg->queue_add(sd, arena, (enum bg_queue_types)p->type);
}

static void clif_bgqueue_update_info(struct map_session_data *sd, unsigned char arena_id, int position)
{
	struct packet_bgqueue_update_info p;

	nullpo_retv(sd);
	Assert_retv(arena_id < bg->arenas);
	p.PacketType = bgqueue_updateinfoType;
	safestrncpy(p.bg_name, bg->arena[arena_id]->name, sizeof(p.bg_name));
	p.position = position;

	sd->bg_queue.client_has_bg_data = true; // Client creates bg data when this packet arrives

	clif->send(&p,sizeof(p), &sd->bl, SELF);
}

static void clif_parse_bgqueue_checkstate(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_bgqueue_checkstate(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_bgqueue_checkstate *p = RP2PTR(fd);

	if (sd->bg_queue.arena && sd->bg_queue.type) {
		clif->bgqueue_update_info(sd,sd->bg_queue.arena->id,bg->id2pos(sd->bg_queue.arena->queue_id,sd->status.account_id));
	} else {
		clif->bgqueue_notice_delete(sd, BGQND_FAIL_NOT_QUEUING,p->bg_name);
	}
}

static void clif_parse_bgqueue_revoke_req(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_bgqueue_revoke_req(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_bgqueue_revoke_req *p = RP2PTR(fd);

	if( sd->bg_queue.arena )
		bg->queue_pc_cleanup(sd);
	else
		clif->bgqueue_notice_delete(sd, BGQND_FAIL_NOT_QUEUING,p->bg_name);
}

static void clif_parse_bgqueue_battlebegin_ack(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_bgqueue_battlebegin_ack(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_bgqueue_battlebegin_ack *p = RP2PTR(fd);
	struct bg_arena *arena;

	if( !bg->queue_on ) return; /* temp, until feature is complete */

	if( ( arena = bg->name2arena(p->bg_name) )  ) {
		bg->queue_ready_ack(arena,sd, ( p->result == 1 ) ? true : false);
	} else {
		clif->bgqueue_ack(sd,BGQA_FAIL_BGNAME_INVALID, 0);
	}
}

static void clif_bgqueue_joined(struct map_session_data *sd, int pos)
{
	struct packet_bgqueue_notify_entry p;

	nullpo_retv(sd);
	p.PacketType = bgqueue_notify_entryType;
	safestrncpy(p.name,sd->status.name,sizeof(p.name));
	p.position = pos;

	clif->send(&p,sizeof(p), &sd->bl, BG_QUEUE);
}

static void clif_bgqueue_pcleft(struct map_session_data *sd)
{
	/* no idea */
	return;
}

// Sends BG ready req to all with same bg arena/type as sd
static void clif_bgqueue_battlebegins(struct map_session_data *sd, unsigned char arena_id, enum send_target target)
{
	struct packet_bgqueue_battlebegins p;

	nullpo_retv(sd);
	Assert_retv(arena_id < bg->arenas);
	p.PacketType = bgqueue_battlebeginsType;
	safestrncpy(p.bg_name, bg->arena[arena_id]->name, sizeof(p.bg_name));
	safestrncpy(p.game_name, bg->arena[arena_id]->name, sizeof(p.game_name));

	clif->send(&p,sizeof(p), &sd->bl, target);
}

static void clif_scriptclear(struct map_session_data *sd, int npcid)
{
#if PACKETVER >= 20110928
	struct packet_script_clear p;

	nullpo_retv(sd);
	p.PacketType = script_clearType;
	p.NpcID = npcid;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/* Made Possible Thanks to Yommy! */
static void clif_package_item_announce(struct map_session_data *sd, int nameid, int containerid)
{
	struct packet_package_item_announce p;

	nullpo_retv(sd);
	p.PacketType = package_item_announceType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.PacketLength = 7 + 4 + 4 + NAME_LENGTH;
#else
	p.PacketLength = 7 + 2 + 2 + NAME_LENGTH;
#endif
	p.type = 0x0;
	p.ItemID = nameid;
	p.len = NAME_LENGTH;
	safestrncpy(p.Name, sd->status.name, sizeof(p.Name));
	p.unknown = 0x2; // some strange byte, IDA shows.. BYTE3(BoxItemIDLength) = 2;
	p.BoxItemID = containerid;

	clif->send(&p, p.PacketLength, &sd->bl, ALL_CLIENT);
}

/* Made Possible Thanks to Yommy! */
static void clif_item_drop_announce(struct map_session_data *sd, int nameid, char *monsterName)
{
	struct packet_item_drop_announce p;

	nullpo_retv(sd);
	p.PacketType = item_drop_announceType;
	p.PacketLength = sizeof(p);
	p.ItemID = nameid;
	p.monsterNameLen = NAME_LENGTH;
	p.len = NAME_LENGTH;
	safestrncpy(p.Name, sd->status.name, sizeof(p.Name));
	if (monsterName == NULL) {
		// message: MSG_BROADCASTING_SPECIAL_ITEM_OBTAIN2
		p.type = 0x2;
		p.PacketLength -= NAME_LENGTH;
	} else {
		// message: MSG_BROADCASTING_SPECIAL_ITEM_OBTAIN
		p.type = 0x1;
		safestrncpy(p.monsterName, monsterName, sizeof(p.monsterName));
	}
	clif->send(&p, p.PacketLength, &sd->bl, ALL_CLIENT);
}

/* [Ind/Hercules] special thanks to Yommy~! */
static void clif_skill_cooldown_list(int fd, struct skill_cd *cd)
{
#if PACKETVER >= 20120604
	const int offset = 10;
#else
	const int offset = 6;
#endif
	int i, count = 0;

	nullpo_retv(cd);

	WFIFOHEAD(fd,4+(offset*cd->cursor));

#if PACKETVER >= 20120604
	WFIFOW(fd,0) = 0x985;
#else
	WFIFOW(fd,0) = 0x43e;
#endif

	for( i = 0; i < cd->cursor; i++ ) {
		if( cd->entry[i]->duration < 1 ) continue;

		WFIFOW(fd, 4  + (count*offset)) = cd->entry[i]->skill_id;
#if PACKETVER >= 20120604
		WFIFOL(fd, 6  + (count*offset)) = cd->entry[i]->total;
		WFIFOL(fd, 10 + (count*offset)) = cd->entry[i]->duration;
#else
		WFIFOL(fd, 6  + (count*offset)) = cd->entry[i]->duration;
#endif
		count++;
	}

	WFIFOW(fd,2) = 4+(offset*count);

	WFIFOSET(fd,4+(offset*count));
}

/* [Ind/Hercules] - Data Thanks to Yommy
 * - ADDITEM_TO_CART_FAIL_WEIGHT = 0x0
 * - ADDITEM_TO_CART_FAIL_COUNT  = 0x1
 */
static void clif_cart_additem_ack(struct map_session_data *sd, int flag)
{
	struct packet_cart_additem_ack p;

	nullpo_retv(sd);
	p.PacketType = cart_additem_ackType;
	p.result = (char)flag;

	clif->send(&p,sizeof(p), &sd->bl, SELF);
}

/* Bank System [Yommy/Hercules] */
static void clif_parse_BankDeposit(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BankDeposit(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_banking_deposit_req *p = RP2PTR(fd);
	int money;

	if (!battle_config.feature_banking) {
		clif->messagecolor_self(fd, COLOR_RED, msg_fd(fd,1483));
		return;
	}

	money = (int)cap_value(p->Money,0,INT_MAX);

	pc->bank_deposit(sd,money);
}

static void clif_parse_BankWithdraw(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BankWithdraw(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_banking_withdraw_req *p = RP2PTR(fd);
	int money;

	if (!battle_config.feature_banking) {
		clif->messagecolor_self(fd, COLOR_RED, msg_fd(fd,1483));
		return;
	}

	money = (int)cap_value(p->Money,0,INT_MAX);

	pc->bank_withdraw(sd,money);
}

static void clif_parse_BankCheck(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BankCheck(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20130320
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct packet_banking_check p;

	p.PacketType = banking_checkType;
	if (!battle_config.feature_banking) {
		p.Money = 0;
		p.Reason = (short)1;
	} else {
		p.Money = (int)sd->status.bank_vault;
		p.Reason = (short)0;
	}

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_parse_BankOpen(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BankOpen(int fd, struct map_session_data *sd)
{
	return;
}

static void clif_parse_BankClose(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_BankClose(int fd, struct map_session_data *sd)
{
	return;
}

static void clif_bank_deposit(struct map_session_data *sd, enum e_BANKING_DEPOSIT_ACK reason)
{
#if PACKETVER >= 20130313
	struct packet_banking_deposit_ack p;

	nullpo_retv(sd);
	p.PacketType = banking_deposit_ackType;
	p.Balance = sd->status.zeny;/* how much zeny char has after operation */
	p.Money = (int64)sd->status.bank_vault;/* money in the bank */
	p.Reason = (short)reason;

	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_bank_withdraw(struct map_session_data *sd, enum e_BANKING_WITHDRAW_ACK reason)
{
#if PACKETVER >= 20130313
	struct packet_banking_withdraw_ack p;

	nullpo_retv(sd);
	p.PacketType = banking_withdraw_ackType;
	p.Balance = sd->status.zeny;/* how much zeny char has after operation */
	p.Money = (int64)sd->status.bank_vault;/* money in the bank */
	p.Reason = (short)reason;

	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

/**
 * Sends EXP, drop and death-penalty rates.
 * 0x097b <packet len>.W <exp>.L <death>.L <drop>.L <DETAIL_EXP_INFO>13B (ZC_PERSONAL_INFOMATION2)
 * <InfoType>.B <Exp>.L <Death>.L <Drop>.L (DETAIL_EXP_INFO)
 *
 * @param sd The character which should receive the messages.
 *
 **/
static void clif_show_modifiers(struct map_session_data *sd)
{
	nullpo_retv(sd);

#if PACKETVER_MAIN_NUM >= 20120503 || PACKETVER_RE_NUM >= 20120502 || defined(PACKETVER_ZERO)
	int length = sizeof(struct PACKET_ZC_PERSONAL_INFOMATION) + 4 * sizeof(struct PACKET_ZC_PERSONAL_INFOMATION_SUB);
	WFIFOHEAD(sd->fd, length);
	struct PACKET_ZC_PERSONAL_INFOMATION *p = WFIFOP(sd->fd, 0);

	p->packetType = HEADER_ZC_PERSONAL_INFOMATION;
	p->length = length;
	// Single values.
	p->details[0].type = PC_EXP_INFO;
	p->details[0].exp = 0;
	p->details[0].death = 0;
	p->details[0].drop = 0;
	p->details[1].type = TPLUS_EXP_INFO;
	p->details[1].exp = 0;
	p->details[1].death = 0;
	p->details[1].drop = 0;
	p->details[2].type = PREMIUM_EXP_INFO;
	p->details[2].exp = (sd->status.mod_exp - 100) * 1000;
	p->details[2].death = (sd->status.mod_death - 100) * 1000;
	p->details[2].drop = (sd->status.mod_drop - 100) * 1000;
	p->details[3].type = SERVER_EXP_INFO;
	p->details[3].exp = battle_config.base_exp_rate * 1000;
	p->details[3].death = battle_config.death_penalty_base * 10;
	p->details[3].drop = battle_config.item_rate_common * 1000;
	// Total values.
	p->total_exp = (battle_config.base_exp_rate * sd->status.mod_exp / 100) * 1000;
	p->total_death = (battle_config.base_exp_rate * sd->status.mod_death / 100) * 10;
	p->total_drop = (battle_config.base_exp_rate * sd->status.mod_drop / 100) * 1000;

	WFIFOSET(sd->fd, length);
#else
	if (sd->status.mod_exp != 100 || sd->status.mod_drop != 100 || sd->status.mod_death != 100) {
		char output[128];

		// Base EXP : %d%% | Base Drop: %d%% | Base Death Penalty: %d%%
		safesnprintf(output, sizeof(output), msg_sd(sd, 896), sd->status.mod_exp, sd->status.mod_drop, sd->status.mod_death);
		clif->broadcast2(&sd->bl, output, (int)strlen(output) + 1, 0xffbc90, 0x190, 12, 0, 0, SELF);
	}
#endif  // PACKETVER_MAIN_NUM >= 20120503 || PACKETVER_RE_NUM >= 20120502 || defined(PACKETVER_ZERO)
}

static void clif_notify_bounditem(struct map_session_data *sd, unsigned short index)
{
	struct packet_notify_bounditem p;

	nullpo_retv(sd);
	p.PacketType = notify_bounditemType;
	p.index = index+2;

	clif->send(&p,sizeof(p), &sd->bl, SELF);
}

static void clif_parse_GMFullStrip(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/**
 * Parses the (GM) right click option 'remove all equipment'
 **/
static void clif_parse_GMFullStrip(int fd, struct map_session_data *sd)
{
	struct map_session_data *tsd = map->id2sd(RFIFOL(fd,2));
	int i;

	/* TODO maybe this could be a new permission? using gm level in the meantime */
	if( !tsd || pc_get_group_level(tsd) >= pc_get_group_level(sd) )
		return;

	for( i = 0; i < EQI_MAX; i++ ) {
		if( tsd->equip_index[ i ] >= 0 )
			pc->unequipitem(tsd, tsd->equip_index[i], PCUNEQUIPITEM_FORCE);
	}
}

/**
 * clif_delay_damage timer, sends the stored data and clears the memory afterwards
 **/
static int clif_delay_damage_sub(int tid, int64 tick, int id, intptr_t data)
{
	struct cdelayed_damage *dd = (struct cdelayed_damage *)data;

	clif->send(&dd->p,sizeof(struct packet_damage),&dd->bl,AREA_WOS);

	ers_free(clif->delayed_damage_ers,dd);

	return 0;
}

/**
 * Delays sending a damage packet in order to avoid the visual display to overlap
 *
 * @param tick when to trigger the timer (e.g. timer->gettick() + 500)
 * @param src block_list pointer of the src
 * @param dst block_list pointer of the damage source
 * @param sdelay attack motion usually status_get_amotion()
 * @param ddelay damage motion usually status_get_dmotion()
 * @param in_damage total damage to be sent
 * @param div amount of hits
 * @param type action type
 *
 * @return clif->calc_walkdelay used in further processing
 **/
static int clif_delay_damage(int64 tick, struct block_list *src, struct block_list *dst, int sdelay, int ddelay, int64 in_damage, short div, enum battle_dmg_type type)
{
	struct cdelayed_damage *dd;
	struct status_change *sc;
#if PACKETVER < 20071113
	short damage;
#else
	int damage;
#endif

	nullpo_ret(src);
	nullpo_ret(dst);

	sc = status->get_sc(dst);

	if(sc && sc->count && sc->data[SC_ILLUSION]) {
		if(in_damage) in_damage = in_damage*(sc->data[SC_ILLUSION]->val2) + rnd()%100;
	}

#if PACKETVER < 20071113
	damage = (short)min(in_damage,INT16_MAX);
#else
	damage = (int)min(in_damage,INT_MAX);
#endif

	type = clif_calc_delay(type,div,damage,ddelay);

	dd = ers_alloc(clif->delayed_damage_ers, struct cdelayed_damage);

	dd->p.PacketType = damageType;
	dd->p.GID = src->id;
	dd->p.targetGID = dst->id;
	dd->p.startTime = (uint32)timer->gettick();
	dd->p.attackMT = sdelay;
	dd->p.attackedMT = ddelay;
	dd->p.count = div;
	dd->p.action = type;
	dd->p.leftDamage = 0;

	if( battle_config.hide_woe_damage && map_flag_gvg2(src->m) )
		dd->p.damage = damage?div:0;
	else
		dd->p.damage = damage;

	dd->bl.m = dst->m;
	dd->bl.x = dst->x;
	dd->bl.y = dst->y;
	dd->bl.type = BL_NUL;

	if( tick > timer->gettick() )
		timer->add(tick,clif->delay_damage_sub,0,(intptr_t)dd);
	else {
		clif->send(&dd->p,sizeof(struct packet_damage),&dd->bl,AREA_WOS);

		ers_free(clif->delayed_damage_ers,dd);
	}

	return clif->calc_walkdelay(dst,ddelay,type,damage,div);
}

static void clif_parse_NPCShopClosed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/* Thanks to Yommy */
static void clif_parse_NPCShopClosed(int fd, struct map_session_data *sd)
{
	/* TODO track the state <3~ */
	sd->npc_shopid = 0;
}

/* NPC Market (by Ind after an extensive debugging of the packet, only possible thanks to Yommy <3) */
static void clif_npc_market_open(struct map_session_data *sd, struct npc_data *nd)
{
#if PACKETVER_MAIN_NUM >= 20131120 || PACKETVER_RE_NUM >= 20131106 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	nullpo_retv(nd);
	struct npc_item_list *shop = nd->u.scr.shop->item;
	const int shop_size = nd->u.scr.shop->items;

	int c = 0;
	int maxCount = (sizeof(packet_buf) - sizeof(struct PACKET_ZC_NPC_MARKET_OPEN)) / sizeof(struct PACKET_ZC_NPC_MARKET_OPEN_sub);
	struct PACKET_ZC_NPC_MARKET_OPEN *packet = (struct PACKET_ZC_NPC_MARKET_OPEN*)&packet_buf[0];
	packet->packetType = HEADER_ZC_NPC_MARKET_OPEN;

	for (int i = 0; i < shop_size && c < maxCount; i++) {
		struct item_data *id = NULL;
		if (shop[i].nameid && (id = itemdb->exists(shop[i].nameid)) != NULL) {
			packet->list[c].nameid = shop[i].nameid;
			packet->list[c].price  = shop[i].value;
			packet->list[c].qty    = shop[i].qty;
			packet->list[c].type   = itemtype(id->type);
			packet->list[c].weight = id->weight * 10;
			c++;
		}
	}

	packet->packetLength = sizeof(struct PACKET_ZC_NPC_MARKET_OPEN) + sizeof(struct PACKET_ZC_NPC_MARKET_OPEN_sub) * c;
	clif->send(packet, packet->packetLength, &sd->bl, SELF);
#endif
}

static void clif_parse_NPCMarketClosed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_NPCMarketClosed(int fd, struct map_session_data *sd)
{
	/* TODO track the state <3~ */
	sd->npc_shopid = 0;
}

static void clif_parse_NPCBarterClosed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_NPCBarterClosed(int fd, struct map_session_data *sd)
{
	sd->npc_shopid = 0;
}

static void clif_npc_market_purchase_ack(struct map_session_data *sd, const struct itemlist *item_list, enum market_buy_result response)
{
#if PACKETVER_MAIN_NUM >= 20131120 || PACKETVER_RE_NUM >= 20130911 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	nullpo_retv(item_list);
	struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT *p = (struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT *)packet_buf;
	p->PacketType = HEADER_ZC_NPC_MARKET_PURCHASE_RESULT;
	p->result = response;

	unsigned short c = 0;
	if (response == MARKET_BUY_RESULT_SUCCESS) {
		int vectorLen = VECTOR_LENGTH(*item_list);
		int maxCount = (sizeof(packet_buf) - sizeof(struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT)) / sizeof(struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT_sub);
		if (maxCount > vectorLen)
			maxCount = vectorLen;
		struct npc_data *nd = map->id2nd(sd->npc_shopid);
		struct npc_item_list *shop = nd->u.scr.shop->item;
		unsigned short shop_size = nd->u.scr.shop->items;

		for (int i = 0; i < maxCount; i++) {
			const struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
			int j;

			p->list[i].ITID = entry->id;
			p->list[i].qty  = entry->amount;

			ARR_FIND(0, shop_size, j, entry->id == shop[j].nameid);

			p->list[i].price = (j != shop_size) ? shop[j].value : 0;

			c++;
		}
	}

	p->PacketLength = sizeof(struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT) + (sizeof(struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT_sub) * c);

	clif->send(p, p->PacketLength, &sd->bl, SELF);
#endif
}

static void clif_parse_NPCMarketPurchase(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_NPCMarketPurchase(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20131223
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct packet_npc_market_purchase *p = RP2PTR(fd);
	int count = (p->PacketLength - 4) / sizeof p->list[0];
	struct itemlist item_list;

	Assert_retv(count >= 0 && count <= sd->status.inventorySize);

	VECTOR_INIT(item_list);
	VECTOR_ENSURE(item_list, count, 1);

	for (int i = 0; i < count; i++) {
		struct itemlist_entry entry = { 0 };

		entry.id = p->list[i].ITID;
		entry.amount = p->list[i].qty;

		VECTOR_PUSH(item_list, entry);
	}

	enum market_buy_result response = npc->market_buylist(sd, &item_list);
	clif->npc_market_purchase_ack(sd, &item_list, response);

	VECTOR_CLEAR(item_list);
#endif
}

static void clif_PartyLeaderChanged(struct map_session_data *sd, int prev_leader_aid, int new_leader_aid)
{
	struct packet_party_leader_changed p;

	nullpo_retv(sd);
	p.PacketType = partyleaderchangedType;

	p.prev_leader_aid = prev_leader_aid;
	p.new_leader_aid = new_leader_aid;

	clif->send(&p,sizeof(p),&sd->bl,PARTY);
}

static void clif_parse_RouletteOpen(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/* Roulette System [Yommy/Hercules] */
static void clif_parse_RouletteOpen(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20140612
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct packet_roulette_open_ack p;

	if( !battle_config.feature_roulette ) {
		clif->message(fd, msg_fd(fd,82)); // Roulette is disabled
		return;
	}

	p.PacketType = 0xa1a;
	p.Result = OPEN_ROULETTE_SUCCESS;
	p.Serial = 0;
	p.Step = sd->roulette.stage - 1;
	p.Idx = (char)sd->roulette.prizeIdx;
	p.AdditionItemID = -1; /** TODO **/
	p.BronzePoint = pc_readglobalreg(sd, script->add_variable("TmpRouletteBronze"));
	p.GoldPoint = pc_readglobalreg(sd, script->add_variable("TmpRouletteGold"));
	p.SilverPoint = pc_readglobalreg(sd, script->add_variable("TmpRouletteSilver"));

	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_parse_RouletteInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_RouletteInfo(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20140612
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct packet_roulette_info_ack p;
	unsigned short i, j, count = 0;

	if( !battle_config.feature_roulette ) {
		clif->message(fd, msg_fd(fd,82)); // Roulette is disabled
		return;
	}

	p.PacketType = rouletteinfoackType;
	p.PacketLength = sizeof(p);
	p.RouletteSerial = 1;

	for(i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		for(j = 0; j < MAX_ROULETTE_COLUMNS-i; j++) {
			p.ItemInfo[count].Row = i;
			p.ItemInfo[count].Position = j;
			p.ItemInfo[count].ItemId = clif->rd.nameid[i][j];
			p.ItemInfo[count].Count = clif->rd.qty[i][j];
#if PACKETVER >= 20180511
			p.ItemInfo[count].unused = 0;
#endif
			count++;
		}
	}
	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_parse_RouletteClose(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_RouletteClose(int fd, struct map_session_data *sd)
{
	if( !battle_config.feature_roulette ) {
		clif->message(fd, msg_fd(fd,82)); // Roulette is disabled
		return;
	}

	/** What do we need this for? (other than state tracking), game client closes the window without our response. **/
	//ShowDebug("clif_parse_RouletteClose\n");

	return;
}

static void clif_parse_RouletteGenerate(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_RouletteGenerate(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	unsigned char result = GENERATE_ROULETTE_SUCCESS;
	short stage = sd->roulette.stage;

	if( !battle_config.feature_roulette ) {
		clif->message(fd, msg_fd(fd,82)); // Roulette is disabled
		return;
	}

	if( sd->roulette.stage >= MAX_ROULETTE_LEVEL )
		stage = sd->roulette.stage = 0;

	if( stage == 0 ) {
		if( pc_readglobalreg(sd, script->add_variable("TmpRouletteBronze")) <= 0 &&
		    pc_readglobalreg(sd, script->add_variable("TmpRouletteSilver")) < 10 &&
		    pc_readglobalreg(sd, script->add_variable("TmpRouletteGold")) < 10 )
			result = GENERATE_ROULETTE_NO_ENOUGH_POINT;
	}

	if( result == GENERATE_ROULETTE_SUCCESS ) {
		if( stage == 0 ) {
			if( pc_readglobalreg(sd, script->add_variable("TmpRouletteBronze")) > 0 ) {
				pc_setglobalreg(sd, script->add_variable("TmpRouletteBronze"), pc_readglobalreg(sd, script->add_variable("TmpRouletteBronze")) - 1);
			} else if( pc_readglobalreg(sd, script->add_variable("TmpRouletteSilver")) > 9 ) {
				pc_setglobalreg(sd, script->add_variable("TmpRouletteSilver"), pc_readglobalreg(sd, script->add_variable("TmpRouletteSilver")) - 10);
				stage = sd->roulette.stage = 2;
			} else if( pc_readglobalreg(sd, script->add_variable("TmpRouletteGold")) > 9 ) {
				pc_setglobalreg(sd, script->add_variable("TmpRouletteGold"), pc_readglobalreg(sd, script->add_variable("TmpRouletteGold")) - 10);
				stage = sd->roulette.stage = 4;
			}
		}
		sd->roulette.prizeStage = stage;
		sd->roulette.prizeIdx = rnd()%clif->rd.items[stage];
		if( sd->roulette.prizeIdx == 0 ) {
			struct item it;
			memset(&it, 0, sizeof(it));

			it.nameid = clif->rd.nameid[stage][0];
			it.identify = 1;

			pc->additem(sd, &it, clif->rd.qty[stage][0], LOG_TYPE_ROULETTE);/** TODO maybe a new log type for roulette items? **/

			sd->roulette.stage = 0;
			result = GENERATE_ROULETTE_LOSING;
		} else
			sd->roulette.claimPrize = true;
	}

	clif->roulette_generate_ack(sd,result,stage,sd->roulette.prizeIdx,0);
	if( result == GENERATE_ROULETTE_SUCCESS )
		sd->roulette.stage++;
}

static void clif_parse_RouletteRecvItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/**
 * Request to cash in!
 **/
static void clif_parse_RouletteRecvItem(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20140612
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;
	struct packet_roulette_itemrecv_ack p;

	if( !battle_config.feature_roulette ) {
		clif->message(fd, msg_fd(fd,82)); // Roulette is disabled
		return;
	}

	p.PacketType = roulettercvitemackType;
	p.AdditionItemID = 0;/** TODO **/

	if( sd->roulette.claimPrize ) {
		struct item it;
		memset(&it, 0, sizeof(it));

		it.nameid = clif->rd.nameid[sd->roulette.prizeStage][sd->roulette.prizeIdx];
		it.identify = 1;

		switch (pc->additem(sd, &it, clif->rd.qty[sd->roulette.prizeStage][sd->roulette.prizeIdx], LOG_TYPE_ROULETTE)) {
			case 0:
				p.Result = RECV_ITEM_SUCCESS;
				sd->roulette.claimPrize = false;
				sd->roulette.prizeStage = 0;
				sd->roulette.prizeIdx = 0;
				sd->roulette.stage = 0;
				break;
			case 1:
			case 4:
			case 5:
				p.Result = RECV_ITEM_OVERCOUNT;
				break;
			case 2:
				p.Result = RECV_ITEM_OVERWEIGHT;
				break;
			default:
			case 7:
				p.Result = RECV_ITEM_FAILED;
				break;
		}
	} else
		p.Result = RECV_ITEM_FAILED;

	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

static bool clif_parse_roulette_db(void)
{
	struct config_t roulette_conf;
	struct config_setting_t *roulette = NULL, *levels = NULL;
	char config_filename[256];
	libconfig->format_db_path("roulette_db.conf", config_filename, sizeof(config_filename));
	int i, j, item_count_t = 0;

	for( i = 0; i < MAX_ROULETTE_LEVEL; i++ ) {
		clif->rd.items[i] = 0;
	}

	if (!libconfig->load_file(&roulette_conf, config_filename))
		return false;
	roulette = libconfig->lookup(&roulette_conf, "roulette");

	if( roulette != NULL && (levels = libconfig->setting_get_elem(roulette, 0)) != NULL ) {
		for(i = 0; i < MAX_ROULETTE_LEVEL; i++) {
			struct config_setting_t *level;
			char entry_name[10];

			sprintf(entry_name,"level_%d",i+1);

			if( (level = libconfig->setting_get_member(levels, entry_name)) != NULL ) {
				int k, item_count = libconfig->setting_length(level);

				for(k = 0; k < item_count; k++) {
					struct config_setting_t *entry = libconfig->setting_get_elem(level,k);
					const char *name = config_setting_name(entry);
					int qty = libconfig->setting_get_int(entry);
					struct item_data * data = NULL;

					if( qty < 1 ) {
						ShowWarning("roulette_db: unsupported qty '%d' for entry named '%s' in category '%s'\n", qty, name, entry_name);
						continue;
					}

					if (name[0] == 'I' && name[1] == 'D' && strlen(name) <= 12) {
						if( !( data = itemdb->exists(atoi(name+2))) ) {
							ShowWarning("roulette_db: unknown item id '%s' in category '%s'\n", name+2, entry_name);
							continue;
						}
					} else {
						if( !( data = itemdb->search_name(name) ) ) {
							ShowWarning("roulette_db: unknown item name '%s' in category '%s'\n", name, entry_name);
							continue;
						}
					}

					j = clif->rd.items[i];
					RECREATE(clif->rd.nameid[i],int,++clif->rd.items[i]);
					RECREATE(clif->rd.qty[i],int,clif->rd.items[i]);

					clif->rd.nameid[i][j] = data->nameid;
					clif->rd.qty[i][j] = qty;

					item_count_t++;
				}
			}
		}
	}
	libconfig->destroy(&roulette_conf);

	for(i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		int limit = MAX_ROULETTE_COLUMNS-i;
		if( clif->rd.items[i] == limit ) continue;

		if( clif->rd.items[i] > limit ) {
			ShowWarning("roulette_db: level %d has %d items, only %d supported, capping...\n",i+1,clif->rd.items[i],limit);
			clif->rd.items[i] = limit;
			continue;
		}
		/** this scenario = clif->rd.items[i] < limit **/
		ShowWarning("roulette_db: level %d has %d items, %d are required. filling with apples\n",i+1,clif->rd.items[i],limit);

		clif->rd.items[i] = limit;
		RECREATE(clif->rd.nameid[i],int,clif->rd.items[i]);
		RECREATE(clif->rd.qty[i],int,clif->rd.items[i]);

		for(j = 0; j < MAX_ROULETTE_COLUMNS-i; j++) {
			if (clif->rd.qty[i][j])
				continue;
			clif->rd.nameid[i][j] = ITEMID_APPLE;
			clif->rd.qty[i][j] = 1;
		}
	}
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", item_count_t, config_filename);

	return true;
}

/**
 *
 **/
static void clif_roulette_generate_ack(struct map_session_data *sd, enum GENERATE_ROULETTE_ACK result, short stage, short prizeIdx, int bonusItemID)
{
#if PACKETVER >= 20140612
	struct packet_roulette_generate_ack p;

	nullpo_retv(sd);
	p.PacketType = roulettgenerateackType;
	p.Result = result;
	p.Step = stage;
	p.Idx = prizeIdx;
	p.AdditionItemID = bonusItemID;
	p.RemainBronze = pc_readglobalreg(sd, script->add_variable("TmpRouletteBronze"));
	p.RemainGold = pc_readglobalreg(sd, script->add_variable("TmpRouletteGold"));
	p.RemainSilver = pc_readglobalreg(sd, script->add_variable("TmpRouletteSilver"));

	clif->send(&p,sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_roulette_close(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20141008 || PACKETVER_RE_NUM >= 20140903 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	struct PACKET_ZC_ACK_CLOSE_ROULETTE p;
	p.packetType = HEADER_ZC_ACK_CLOSE_ROULETTE;
	p.result = 0;  // close window

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/**
 * Stackable items merger
 */
static void clif_openmergeitem(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	int n = 0, j = 0;
	struct merge_item merge_items[MAX_INVENTORY];
	struct merge_item *merge_items_[MAX_INVENTORY] = {0};

	memset(&merge_items, '\0', sizeof(merge_items));

	for (int i = 0; i < sd->status.inventorySize; i++) {
		struct item *item_data = &sd->status.inventory[i];

		if (item_data->nameid == 0 || !itemdb->isstackable(item_data->nameid) || item_data->bound != IBT_NONE)
			continue;

		merge_items[n].nameid = item_data->nameid;
		merge_items[n].position = i + 2;
		n++;
	}

	qsort(merge_items, n, sizeof(struct merge_item), clif->comparemergeitem);

	j = 0;
	for (int i = 0; i < n; i++) {
		if (i > 0 && merge_items[i].nameid == merge_items[i - 1].nameid)
		{
			merge_items_[j] = &merge_items[i];
			j++;
			continue;
		}

		if (i < n - 1 && merge_items[i].nameid == merge_items[i + 1].nameid)
		{
			merge_items_[j] = &merge_items[i];
			j++;
			continue;
		}
	}

	const int len = sizeof(struct PACKET_ZC_MERGE_ITEM_OPEN) + j * sizeof(struct PACKET_ZC_MERGE_ITEM_OPEN_sub);
	WFIFOHEAD(fd, len);
	struct PACKET_ZC_MERGE_ITEM_OPEN *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_MERGE_ITEM_OPEN;
	p->packetLen = len;
	for (int i = 0; i < j; i++)
		p->items[i].index = merge_items_[i]->position;
	WFIFOSET(fd, len);
#endif
}

static int clif_comparemergeitem(const void *a, const void *b)
{
	const struct merge_item *a_ = a;
	const struct merge_item *b_ = b;

	nullpo_ret(a);
	nullpo_ret(b);
	if (a_->nameid == b_->nameid)
		return 0;
	return a_->nameid > b_->nameid ? -1 : 1;
}

static void clif_mergeitems(int fd, struct map_session_data *sd, int index, int amount, enum mergeitem_reason reason)
{
#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
	WFIFOHEAD(fd, sizeof(struct PACKET_ZC_ACK_MERGE_ITEM));
	struct PACKET_ZC_ACK_MERGE_ITEM *p = WFIFOP(fd, 0);
	p->packetType = HEADER_ZC_ACK_MERGE_ITEM;
	p->index = index + 2;
	p->amount = amount;
	p->reason = reason;
	WFIFOSET(fd, sizeof(struct PACKET_ZC_ACK_MERGE_ITEM));
#endif
}

static void clif_ackmergeitems(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	int i = 0, n = 0, count = 0;
	int nameid = 0;
	int16 indexes[MAX_INVENTORY] = {0}, amounts[MAX_INVENTORY] = {0};

	int length = (RFIFOW(fd, 2) - 4) / 2;

	if (length >= sd->status.inventorySize || length < 2) {
		clif->mergeitems(fd, sd, 0, 0, MERGEITEM_FAILD);
		return;
	}

	for (i = 0, n = 0; i < length; i++) {
		int16 idx = RFIFOW(fd, i * 2 + 4) - 2;

		if (idx < 0 || idx >= sd->status.inventorySize)
			continue;

		struct item *it = &sd->status.inventory[idx];

		if (it->nameid == 0 || !itemdb->isstackable(it->nameid) || it->bound != IBT_NONE)
			continue;

		if (nameid == 0)
			nameid = it->nameid;

		if (nameid != it->nameid)
			continue;

		count += it->amount;
		indexes[n] = idx;
		amounts[n] = it->amount;
		n++;
	}

	if (n < 2 || count == 0) {
		clif->mergeitems(fd, sd, 0, 0, MERGEITEM_FAILD);
		return;
	}

	if (count > MAX_AMOUNT) {
		clif->mergeitems(fd, sd, 0, 0, MERGEITEM_MAXCOUNTFAILD);
		return;
	}

	for (i = 0; i < n; i++)
		pc->delitem(sd, indexes[i], amounts[i], 0, DELITEM_NORMAL, LOG_TYPE_NPC);

	struct item item_data;
	memset(&item_data, '\0', sizeof(item_data));

	item_data.nameid = nameid;
	item_data.identify = 1;
	item_data.unique_id = itemdb->unique_id(sd);
	pc->additem(sd, &item_data, count, LOG_TYPE_NPC);

	ARR_FIND(0, sd->status.inventorySize, i, item_data.unique_id == sd->status.inventory[i].unique_id);

	clif->mergeitems(fd, sd, i, count, MERGEITEM_SUCCESS);
#endif
}

static void clif_cancelmergeitem(int fd, struct map_session_data *sd)
{
	//Track The merge item cancelation ?
	return;
}

static void clif_dressroom_open(struct map_session_data *sd, int view)
{
#if PACKETVER >= 20150513
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xa02));
	WFIFOW(fd,0)=0xa02;
	WFIFOW(fd,2)=view;
	WFIFOSET(fd,packet_len(0xa02));
#endif
}

/// Request to select cart's visual look for new cart design (ZC_SELECTCART).
/// 097f <Length>.W <identity>.L <type>.B
static void clif_selectcart(struct map_session_data *sd)
{
#if PACKETVER >= 20150805
	int i = 0, fd;

	fd = sd->fd;

	WFIFOHEAD(fd, 8 + MAX_CARTDECORATION_CARTS);
	WFIFOW(fd, 0) = 0x97f;
	WFIFOW(fd, 2) = 8 + MAX_CARTDECORATION_CARTS;
	WFIFOL(fd, 4) = sd->status.account_id;

	for (i = 0; i < MAX_CARTDECORATION_CARTS; i++) {
		WFIFOB(fd, 8 + i) = MAX_BASE_CARTS + 1 + i;
	}

	WFIFOSET(fd, 8 + MAX_CARTDECORATION_CARTS);
#endif
}

/// Starts navigation to the given target on client side
static void clif_navigate_to(struct map_session_data *sd, const char *mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id)
{
// probably this packet with other fields present in older packet versions
#if PACKETVER >= 20120307
	int fd;

	nullpo_retv(sd);
	nullpo_retv(mapname);
	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x8e2));
	WFIFOW(fd, 0) = 0x8e2;

	// How detailed will our navigation be?
	if (mob_id > 0) {
		x = 0;
		y = 0;
		WFIFOB(fd, 2) = 3; // monster with destination field
	} else if (x > 0 && y > 0) {
		WFIFOB(fd, 2) = 0; // with coordinates
	} else {
		x = 0;
		y = 0;
		WFIFOB(fd, 2) = 1; // without coordinates(will fail if you are already on the map)
	}

	// Which services can be used for transportation?
	WFIFOB(fd, 3) = flag;
	// If this flag is set, the navigation window will not be opened up
	WFIFOB(fd, 4) = hideWindow;
	// Target map
	safestrncpy((char*)WFIFOP(fd, 5), mapname, MAP_NAME_LENGTH_EXT);
	// Target x
	WFIFOW(fd, 21) = x;
	// Target y
	WFIFOW(fd, 23) = y;
	// Target monster ID
	WFIFOW(fd, 25) = mob_id;
	WFIFOSET(fd, packet_len(0x8e2));
#endif
}

/**
 * Returns the name of the given bl, in a client-friendly format.
 *
 * @param bl The requested bl.
 * @return The bl's name (guaranteed to be non-NULL).
 */
static const char *clif_get_bl_name(const struct block_list *bl)
{
	const char *name = status->get_name(bl);

	if (name == NULL)
		return "Unknown";

	return name;
}

/**
 * Clan System: Sends the basic clan informations to client.
 * 098a <length>.W <clan id>.L <clan name>.24B <clan master>.24B <clan map>.16B <alliance count>.B
 *      <antagonist count>.B { <alliance>.24B } * alliance count { <antagonist>.24B } * antagonist count (ZC_CLANINFO)
 */
static void clif_clan_basicinfo(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20130626 || PACKETVER_RE_NUM >= 20130605 || defined(PACKETVER_ZERO)
	int len, i, fd;
	struct clan *c, *ally, *antagonist;
	struct PACKET_ZC_CLANINFO *packet = NULL;
	nullpo_retv(sd);
	nullpo_retv(c = sd->clan);

	len = sizeof(struct PACKET_ZC_CLANINFO);
	fd = sd->fd;

	const int maxEntries = 100;  // max entries with clan names
	WFIFOHEAD(fd, len + maxEntries * 24);
	packet = WFIFOP(fd, 0);

	packet->PacketType = HEADER_ZC_CLANINFO;
	packet->ClanID = c->clan_id;

	safestrncpy(packet->ClanName, c->name, NAME_LENGTH);
	safestrncpy(packet->MasterName, c->master, NAME_LENGTH);

	mapindex->getmapname_ext(c->map, packet->Map);

	packet->AllyCount = VECTOR_LENGTH(c->allies);
	packet->AntagonistCount = VECTOR_LENGTH(c->antagonists);

	int cnt = 0;
	// All allies and antagonists are assumed as valid entries
	// since it only gets inside the vector after the validation
	// on clan->config_read
	for (i = 0; i < VECTOR_LENGTH(c->allies) && cnt < maxEntries; i++) {
		struct clan_relationship *al = &VECTOR_INDEX(c->allies, i);

		if ((ally = clan->search(al->clan_id)) != NULL) {
			safestrncpy(WFIFOP(fd, len), ally->name, NAME_LENGTH);
			len += NAME_LENGTH;
			cnt ++;
		}
	}

	for (i = 0; i < VECTOR_LENGTH(c->antagonists) && cnt < maxEntries; i++) {
		struct clan_relationship *an = &VECTOR_INDEX(c->antagonists, i);

		if ((antagonist = clan->search(an->clan_id)) != NULL) {
			safestrncpy(WFIFOP(fd, len), antagonist->name, NAME_LENGTH);
			len += NAME_LENGTH;
			cnt ++;
		}
	}

	packet->PacketLength = len;
	WFIFOSET(fd, len);
#endif
}

/**
 * Clan System: Updates the online and maximum player count of a clan.
 * 0988 <online count>.W <maximum member amount>.W (ZC_NOTIFY_CLAN_CONNECTINFO)
 */
static void clif_clan_onlinecount(struct clan *c)
{
#if PACKETVER >= 20120716
	struct map_session_data *sd;
	struct PACKET_ZC_NOTIFY_CLAN_CONNECTINFO p;

	nullpo_retv(c);

	p.PacketType = clanOnlineCount;
	p.NumConnect = c->connect_member;
	p.NumTotal = c->max_member;

	if ((sd = clan->getonlinesd(c)) != NULL) {
		clif->send(&p, sizeof(p), &sd->bl, CLAN);
	}
#endif
}

/**
 * Clan System: Notifies the client that the player has left his clan.
 * 0989 (ZC_ACK_CLAN_LEAVE)
 */
static void clif_clan_leave(struct map_session_data *sd)
{
#if PACKETVER >= 20131223
	struct PACKET_ZC_ACK_CLAN_LEAVE p;

	nullpo_retv(sd);

	p.PacketType = clanLeave;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/**
 * Clan System: Sends a clan message to a player
 * 098e <length>.W <name>.24B <message>.?B (ZC_NOTIFY_CLAN_CHAT)
 */
static void clif_clan_message(struct clan *c, const char *mes, int len)
{
#if PACKETVER >= 20120716
	struct map_session_data *sd;
	struct PACKET_ZC_NOTIFY_CLAN_CHAT *p;
	unsigned int max_len = CHAT_SIZE_MAX - 5 - NAME_LENGTH;
	int packet_length;

	nullpo_retv(c);
	nullpo_retv(mes);

	if (len == 0) {
		return;
	} else if (len > max_len) {
		ShowWarning("clif_clan_message: Truncated message '%s' (len=%d, max=%u, clan_id=%d).\n", mes, len, max_len, c->clan_id);
		len = max_len;
	}

	packet_length = sizeof(*p) + len + 1;
	p = (struct PACKET_ZC_NOTIFY_CLAN_CHAT *)aMalloc(packet_length);
	p->PacketType = clanMessage;
	p->PacketLength = packet_length;
	// p->MemberName is being ignored on the client side.
	safestrncpy(p->Message, mes, len + 1);

	if ((sd = clan->getonlinesd(c)) != NULL)
		clif->send(p, packet_length, &sd->bl, CLAN);
	aFree(p);
#endif
}

static void clif_parse_ClanMessage(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
/**
 * Clan System: Parses a clan message from a player.
 * 098d <length>.W <text>.?B (<name> : <message>) (CZ_CLAN_CHAT)
 */
static void clif_parse_ClanMessage(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20120716
	const struct packet_chat_message *packet = RP2PTR(fd);
	char message[CHAT_SIZE_MAX + NAME_LENGTH + 3 + 1];

	if (clif->process_chat_message(sd, packet, message, sizeof(message)) == NULL)
		return;

	clan->send_message(sd, message);
#endif
}

/* */
static unsigned short clif_decrypt_cmd(int cmd, struct map_session_data *sd)
{
	if( sd ) {
		return (cmd ^ ((sd->cryptKey >> 16) & 0x7FFF));
	}
	return (cmd ^ (((( clif->cryptKey[0] * clif->cryptKey[1] ) + clif->cryptKey[2]) >> 16) & 0x7FFF));
}

static unsigned short clif_parse_cmd_normal(int fd, struct map_session_data *sd)
{
	unsigned short cmd = RFIFOW(fd,0);

	return cmd;
}

static unsigned short clif_parse_cmd_decrypt(int fd, struct map_session_data *sd)
{
	unsigned short cmd = RFIFOW(fd,0);

	cmd = clif->decrypt_cmd(cmd, sd);

	return cmd;
}

static unsigned short clif_parse_cmd_optional(int fd, struct map_session_data *sd)
{
	unsigned short cmd = RFIFOW(fd,0);

	// filter out invalid / unsupported packets
	if( cmd > MAX_PACKET_DB || cmd < MIN_PACKET_DB || packets->db[cmd] == 0 ) {
		if( sd )
			sd->parse_cmd_func = clif_parse_cmd_decrypt;
		return clif_parse_cmd_decrypt(fd, sd);
	} else if( sd ) {
		sd->parse_cmd_func = clif_parse_cmd_normal;
	}

	return cmd;
}

/**
 * Send all of a session's achievement information to client.
 * Called only once on login/char-loading. (PACKET_ZC_ALL_ACH_LIST)
 * @packet [out] 0x0A23 <ID>.W <Length>.W <ach_count>.L <total_points>.L <rank>.W <current_rank_points>.L <next_rank_points>.L <struct ach_list_info *[]>.P
 * @param fd   socket descriptor
 * @param sd   pointer to map_session_data
 */
static void clif_achievement_send_list(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO)
	int i = 0, count = 0, curr_exp_tmp = 0;
	struct packet_achievement_list p = { 0 };

	nullpo_retv(sd);

	/* Browse through the session's achievement list and gather their values. */
	for (i = 0; i < VECTOR_LENGTH(sd->achievement); i++) {
		int j = 0;
		struct achievement *a = &VECTOR_INDEX(sd->achievement, i);
		const struct achievement_data *ad = NULL;

		/* Sanity check for nonull pointers. */
		if (a == NULL || (ad = achievement->get(a->id)) == NULL)
			continue;

		p.ach[count].ach_id = a->id;

		for (j = 0; j < VECTOR_LENGTH(ad->objective); j++)
			p.ach[count].objective[j] = a->objective[j];

		if (a->completed_at) {
			p.ach[count].completed = 1;
			p.ach[count].completed_at = (uint32) a->completed_at;
			p.ach[count].reward = a->rewarded_at ? 1 : 0;
			p.total_points += ad->points;
		}

		count++;
	}

	p.packet_id = achievementListType;
	p.packet_len = sizeof(struct ach_list_info) * count + 22;
	p.total_achievements = count;
	/* Determine Achievement Rank and current exp */
	curr_exp_tmp = p.total_points;
	for (i = 0; curr_exp_tmp && i < MAX_ACHIEVEMENT_RANKS && i < VECTOR_LENGTH(achievement->rank_exp); i++) {
		if (curr_exp_tmp >= VECTOR_INDEX(achievement->rank_exp, i)) {
			curr_exp_tmp -= VECTOR_INDEX(achievement->rank_exp, i);
			p.rank++;

			// Validate achievement rank type achievements.
			achievement->validate_achievement_rank(sd, p.rank);
		}
	}
	/* Determine Achievement Rank Exp */
	p.current_rank_points = curr_exp_tmp;
	p.next_rank_points = VECTOR_INDEX(achievement->rank_exp, p.rank);
	/* Send payload */
	clif->send(&p, p.packet_len, &sd->bl, SELF);
#endif // PACKETVER >= 20141016
}

/**
 * Sends achievement information for a single achievement.
 * Called on objective progress updates/completion. (PACKET_ZC_ACH_UPDATE)
 * @packet [out] 0x0A24 <ID>.W <total_points>.L <rank>.W <current_rank_points>.L <next_rank_points>.L <struct ach_list_info *>.P
 * @param fd    socket descriptor
 * @param sd    pointer to struct map_session_data
 * @param ad    const pointer to struct achievement_data from the achievement db.
 */
static void clif_achievement_send_update(int fd, struct map_session_data *sd, const struct achievement_data *ad)
{
#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO)
	struct packet_achievement_update p = { 0 };
	struct achievement *a = NULL;
	int i = 0, points = 0, rank = 0, curr_rank_points = 0;

	nullpo_retv(sd);
	nullpo_retv(ad);

	/* Get Session Achievement Data */
	if ((a = achievement->ensure(sd, ad)) == NULL)
		return;

	/* Get total points, current rank and current rank points from the session. */
	achievement->calculate_totals(sd, &points, NULL, &rank, &curr_rank_points);

	p.packet_id = achievementUpdateType;

	/* Determine Total Achievement Points */
	p.total_points = points;

	/* Determine Achievement Rank */
	p.rank = rank;

	/* Determine Achievement Rank Exp */
	p.current_rank_points = curr_rank_points;
	p.next_rank_points = VECTOR_INDEX(achievement->rank_exp, p.rank);

	p.ach.ach_id = ad->id;
	p.ach.completed = (uint8) achievement->check_complete(sd, ad);

	for (i = 0; i < VECTOR_LENGTH(ad->objective); i++)
		p.ach.objective[i] = a->objective[i];

	p.ach.completed_at = (uint32) a->completed_at;

	p.ach.reward = a->rewarded_at ? 1 : 0;

	clif->send(&p, packet_len(achievementUpdateType), &sd->bl, SELF);

	/* Validate rank-related achievements */
	achievement->validate_achievement_rank(sd, rank);

#endif // PACKETVER >= 20141016
}

/**
 * Parses achievement reward packet from session.
 * @packet [in] 0x0A25 <ach_id>.L
 * @param fd   socket descriptor.
 * @param sd   ptr to session data.
 */
static void clif_parse_achievement_get_reward(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_achievement_get_reward(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int ach_id = RFIFOL(fd, 2);
	const struct achievement_data *ad = NULL;
	struct achievement *ach = NULL;

	if (ach_id <= 0 || (ad = achievement->get(ach_id)) == NULL)
		return;

	if ((ach = achievement->ensure(sd, ad)) == NULL)
		return;

	if (achievement->check_complete(sd, ad) && ach->completed_at && ach->rewarded_at == 0) {
		achievement->get_rewards(sd, ad);
	}
}

/**
 * Sends achievement reward collection acknowledgement to the client.
 * @packet [out] 0x0A26 <packet_id>.W <received
 */
static void clif_achievement_reward_ack(int fd, struct map_session_data *sd, const struct achievement_data *ad)
{
#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO)
	struct packet_achievement_reward_ack p = { 0 };

	nullpo_retv(sd);
	nullpo_retv(ad);

	p.packet_id = achievementRewardAckType;
	p.failed = 0;
	p.ach_id = ad->id;

	clif->send(&p, packet_len(achievementRewardAckType), &sd->bl, SELF);
#endif // PACKETVER >= 20141016
}

/**
 * Sends achievement reward collection acknowledgement to the client.
 * @packet[in] 0x0A2E <packet_id>.W <title_id>.L
 */
static void clif_parse_change_title(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_change_title(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	int title_id = RFIFOL(fd, 2);

	if (title_id == sd->status.title_id) { // Same Title
		return;
	} else if (title_id < 0) {
		title_id = 0;
	}

	clif->change_title_ack(fd, sd, title_id);
}

/**
 * [clif_change_title_ack description]
 * @packet [out] 0x0A2F <packet_id>.W <Result>.B <title_id>.L
 */
static void clif_change_title_ack(int fd, struct map_session_data *sd, int title_id)
{
#if PACKETVER >= 20141016
	unsigned char failed = 0;

	if (!achievement->check_title(sd, title_id)) {
		clif->message(fd, msg_sd(sd, 1485)); // "Title is not yet earned."
		failed = 1;
	}

	sd->status.title_id = title_id;

	WFIFOHEAD(fd, packet_len(0xa2f));
	WFIFOW(fd, 0) = 0xa2f;
	WFIFOB(fd, 2) = failed;
	WFIFOL(fd, 3) = sd->status.title_id;
	WFIFOSET(fd, packet_len(0xa2f));

	// Update names
	clif->blname_ack(fd, &sd->bl);
	clif->blname_ack(0, &sd->bl);
#endif
}
// End of Achievement System

/*==========================================
 * RoDEX
 *------------------------------------------*/

static void clif_parse_rodex_open_write_mail(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_open_write_mail(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_OPEN_WRITE_MAIL *rPacket = RFIFOP(fd, 0);
	int8 result = (rodex->isenabled() && (sd->npc_id == 0 || sd->state.using_megaphone != 0)) ? 1 : 0;

	if (result == 1)
		sd->state.workinprogress |= 2;

	clif->rodex_open_write_mail(fd, rPacket->receiveName, result);
}

static void clif_rodex_open_write_mail(int fd, const char *receiver_name, int8 result)
{
#if PACKETVER >= 20140416
	struct PACKET_ZC_ACK_OPEN_WRITE_MAIL *sPacket = NULL;

	nullpo_retv(receiver_name);

	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexopenwrite;
	safestrncpy(sPacket->receiveName, receiver_name, NAME_LENGTH);
	sPacket->result = result;
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_parse_rodex_add_item(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_add_item(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_ADD_ITEM_TO_MAIL *rPacket = RFIFOP(fd, 0);
	int16 idx = rPacket->index - 2;

	rodex->add_item(sd, idx, (int16)rPacket->count);
}

static void clif_rodex_add_item_result(struct map_session_data *sd, int16 idx, int16 amount, enum rodex_add_item result)
{
#if PACKETVER >= 20141119
	struct PACKET_ZC_ADD_ITEM_TO_MAIL *packet;
	int fd, j;

	nullpo_retv(sd);
	if (idx < 0 || idx >= sd->status.inventorySize)
		return;

	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(*packet));
	packet = WFIFOP(fd, 0);
	memset(packet, 0x0, sizeof(*packet));
	packet->PacketType = rodexadditem;
	packet->result = result;

	if (result != RODEX_ADD_ITEM_SUCCESS) { //No need to continue building the packet if it failed
		WFIFOSET(fd, sizeof(*packet));
		return;
	}

	packet->index = idx + 2;
	packet->count = amount;
	packet->itemId = sd->status.inventory[idx].nameid;
	packet->type = itemtype(sd->inventory_data[idx]->type);
	packet->IsIdentified = sd->status.inventory[idx].identify ? 1 : 0;
	packet->IsDamaged = (sd->status.inventory[idx].attribute & ATTR_BROKEN) != 0 ? 1 : 0;
	packet->refiningLevel = sd->status.inventory[idx].refine;
	for (j = 0; j < ARRAYLENGTH(packet->slot.card); ++j)
		packet->slot.card[j] = sd->status.inventory[idx].card[j];
	for (j = 0; j < MAX_ITEM_OPTIONS; ++j) {
		packet->optionData[j].index = sd->status.inventory[idx].option[j].index;
		packet->optionData[j].param = sd->status.inventory[idx].option[j].param;
		packet->optionData[j].value = sd->status.inventory[idx].option[j].value;
	}
	packet->weight = sd->rodex.tmp.weight / 10;
	packet->favorite = sd->status.inventory[idx].favorite;
	packet->location = pc->equippoint(sd, idx);
	WFIFOSET(fd, sizeof(*packet));
#endif
}

static void clif_parse_rodex_remove_item(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_remove_item(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_REMOVE_ITEM_MAIL *rPacket = RFIFOP(fd, 0);
	int16 idx = rPacket->index - 2;

	rodex->remove_item(sd, idx, (int16)rPacket->cnt);
}

static void clif_rodex_remove_item_result(struct map_session_data *sd, int16 idx, int16 amount)
{
#if PACKETVER >= 20140521
	struct PACKET_ZC_ACK_REMOVE_ITEM_MAIL *packet;
	int fd;

	nullpo_retv(sd);
	Assert_retv(idx >= 0 && idx < sd->status.inventorySize);

	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(*packet));
	packet = WFIFOP(fd, 0);
	packet->PacketType = rodexremoveitem;
	packet->result = (amount < 0) ? 0 : 1;
	packet->cnt = (amount < 0) ? 0 : sd->status.inventory[idx].amount - amount;
	packet->index = idx + 2;
	packet->weight = sd->rodex.tmp.weight / 10;
	WFIFOSET(fd, sizeof(*packet));
#endif
}

static void clif_parse_rodex_checkname(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_checkname(int fd, struct map_session_data *sd)
{
	const struct PACKET_CZ_CHECKNAME *rPacket = RFIFOP(fd, 0);
	int char_id = 0, base_level = 0;
	int class = 0;
	char name[NAME_LENGTH];

	safestrncpy(name, rPacket->Name, NAME_LENGTH);

	rodex->check_player(sd, name, &base_level, &char_id, &class);
}

static void clif_rodex_checkname_result(struct map_session_data *sd, int char_id, int class_, int base_level, const char *name)
{
#if PACKETVER >= 20140521
	struct PACKET_ZC_CHECKNAME *sPacket;
	int fd;

	nullpo_retv(sd);
	nullpo_retv(name);

	fd = sd->fd;
	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexcheckplayer;
	if (char_id == 0) {
		sPacket->CharId = 0;
		WFIFOSET(fd, sizeof(*sPacket));
		return;
	}
	sPacket->CharId = char_id;
	sPacket->Class = class_;
	sPacket->BaseLevel = base_level;
#if PACKETVER >= 20160316
	safestrncpy(sPacket->Name, name, NAME_LENGTH);
#endif
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_parse_rodex_send_mail(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_send_mail(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_SEND_MAIL *rPacket = RFIFOP(fd, 0);
	int8 result;

	if (rPacket->TextcontentsLength + rPacket->Titlelength > rPacket->PacketLength - sizeof(*rPacket)) {
		result = RODEX_SEND_MAIL_FATAL_ERROR;
	} else if (rPacket->TextcontentsLength > RODEX_BODY_LENGTH || rPacket->Titlelength > RODEX_TITLE_LENGTH) {
		result = RODEX_SEND_MAIL_FATAL_ERROR;
	} else {
		char rname[NAME_LENGTH] = "";
		char title[RODEX_TITLE_LENGTH] = "";
		char body[RODEX_BODY_LENGTH] = "";

		safestrncpy(rname, rPacket->receiveName, NAME_LENGTH);
		safestrncpy(title, rPacket->string, RODEX_TITLE_LENGTH);
		safestrncpy(body, &rPacket->string[rPacket->Titlelength], RODEX_BODY_LENGTH);

		result = rodex->send_mail(sd, rname, body, title, rPacket->zeny);
	}

	if (result != RODEX_SEND_MAIL_SUCCESS)
		clif->rodex_send_mail_result(fd, sd, result);
	rodex->clean(sd, 1);
}

static void clif_rodex_send_mail_result(int fd, struct map_session_data *sd, int8 result)
{
#if PACKETVER >= 20131230
	struct PACKET_ZC_WRITE_MAIL_RESULT *sPacket;

	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexwriteresult;
	sPacket->result = result;
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_rodex_send_maillist(int fd, struct map_session_data *sd, int8 open_type, int64 page_start)
{
#if PACKETVER >= 20131218
	struct PACKET_ZC_MAIL_LIST *packet;
	struct maillistinfo *inner;
	int16 size = sizeof(*packet);
	int8 count = 0;

	nullpo_retv(sd);

	WFIFOHEAD(fd, sizeof(*packet) + (sizeof(*inner) + RODEX_TITLE_LENGTH) * RODEX_MAIL_PER_PAGE);
	packet = WFIFOP(fd, 0);
	packet->PacketType = ((page_start == (VECTOR_LENGTH(sd->rodex.messages) - 1)) ? rodexmailList : rodexnextpage);
#if PACKETVER < 20170419
	packet->opentype = open_type;
#endif
	inner = WFIFOP(fd, size);

	while (page_start >= 0 && count < RODEX_MAIL_PER_PAGE) {
		struct rodex_message *msg = &VECTOR_INDEX(sd->rodex.messages, page_start);
		--page_start;

		if (msg->is_deleted)
			continue;

		inner->MailID = msg->id;
		inner->Isread = (msg->is_read == true || msg->sender_read == true) ? 1 : 0;
		inner->type = msg->type;
#if PACKETVER >= 20170419
		inner->openType = msg->opentype;
#else
		inner->regDateTime = (int)time(NULL) - msg->send_date;
#endif
		inner->expireDateTime = msg->expire_date - (int)time(NULL);
		if (open_type == RODEX_OPENTYPE_RETURN) {
			inner->expireDateTime += RODEX_EXPIRE;
		}
		inner->Titlelength = (int16)strlen(msg->title) + 1;
		if (open_type != RODEX_OPENTYPE_RETURN) {
			strncpy(inner->SenderName, msg->sender_name, sizeof(inner->SenderName));
		} else {
			strncpy(inner->SenderName, msg->receiver_name, sizeof(inner->SenderName));
		}
		strncpy(inner->title, msg->title, inner->Titlelength);
		size += sizeof(*inner) + inner->Titlelength;
		inner = WFIFOP(fd, size);
		++count;
	}

	packet->PacketLength = size;
#if PACKETVER < 20170419
	packet->cnt = count;
#endif
	packet->IsEnd = page_start > 0 ? 0 : 1;
	WFIFOSET(fd, size);
#endif
}

static void clif_rodex_send_mails_all(int fd, struct map_session_data *sd, int64 mail_id)
{
#if PACKETVER >= 20170419
	struct PACKET_ZC_MAIL_LIST *packet;
	struct maillistinfo *inner;
	int16 size = sizeof(*packet);
	int packetMailCount = 0;
	int mailListCount = 0;
	int mailsSize, i;
	int j = -1;

	nullpo_retv(sd);

	mailsSize = VECTOR_LENGTH(sd->rodex.messages);

	if (mail_id > 0)
		ARR_FIND(0, VECTOR_LENGTH(sd->rodex.messages), j, (VECTOR_INDEX(sd->rodex.messages, j)).id == mail_id);

	WFIFOHEAD(fd, sizeof(*packet) + (sizeof(*inner) + RODEX_TITLE_LENGTH) * RODEX_MAIL_PER_PAGE);
	packet = WFIFOP(fd, 0);
	packet->PacketType = rodexmailList;
	inner = WFIFOP(fd, size);

	i = mailsSize - 1;
	mailsSize -= (j + 1);
	while (i > j) {
		struct rodex_message *msg = &VECTOR_INDEX(sd->rodex.messages, i);
		--i;

		if (msg->is_deleted)
			continue;

		inner->MailID = msg->id;
		inner->Isread = (msg->is_read == true || msg->sender_read == true) ? 1 : 0;
		inner->type = msg->type;
		inner->openType = msg->opentype;
		inner->expireDateTime = msg->expire_date - (int)time(NULL);
		if (msg->opentype == RODEX_OPENTYPE_RETURN) {
			inner->expireDateTime += RODEX_EXPIRE;
		}
		inner->Titlelength = (int16)strlen(msg->title) + 1;
		if (msg->opentype != RODEX_OPENTYPE_RETURN) {
			strncpy(inner->SenderName, msg->sender_name, sizeof(inner->SenderName));
		} else {
			strncpy(inner->SenderName, msg->receiver_name, sizeof(inner->SenderName));
		}
		strncpy(inner->title, msg->title, inner->Titlelength);
		size += sizeof(*inner) + inner->Titlelength;
		inner = WFIFOP(fd, size);
		packetMailCount ++;
		mailListCount ++;
		if (packetMailCount == RODEX_MAIL_PER_PAGE) {
			packet->PacketLength = size;
			packet->IsEnd = mailListCount > mailsSize ? 1 : 0;
			WFIFOSET(fd, size);
			WFIFOHEAD(fd, sizeof(*packet) + (sizeof(*inner) + RODEX_TITLE_LENGTH) * RODEX_MAIL_PER_PAGE);
			packet = WFIFOP(fd, 0);
			packet->PacketType = rodexmailList;
			size = sizeof(*packet);
			inner = WFIFOP(fd, size);
			packetMailCount = 0;
		}
	}

	if (packetMailCount > 0 || mailListCount == 0) {
		packet->PacketLength = size;
		packet->IsEnd = 1;
		WFIFOSET(fd, size);
	}
#endif
}

static void clif_rodex_send_refresh(int fd, struct map_session_data *sd, int8 open_type, int count)
{
#if PACKETVER >= 20131218
	struct PACKET_ZC_MAIL_LIST *packet;
	struct maillistinfo *inner;
	int16 size = sizeof(*packet);
	int i, j;

	nullpo_retv(sd);

	WFIFOHEAD(fd, sizeof(*packet) + (sizeof(*inner) + RODEX_TITLE_LENGTH) * RODEX_MAIL_PER_PAGE);
	packet = WFIFOP(fd, 0);
	packet->PacketType = rodexmailList;
#if PACKETVER < 20170419
	packet->opentype = open_type;
#endif
	inner = WFIFOP(fd, size);

	i = VECTOR_LENGTH(sd->rodex.messages) - 1;
	j = count;
	while (i >= 0 && j > 0) {
		struct rodex_message *msg = &VECTOR_INDEX(sd->rodex.messages, i);
		--i;

		if (msg->is_deleted)
			continue;

		inner->MailID = msg->id;
		inner->Isread = (msg->is_read == true || msg->sender_read == true) ? 1 : 0;
		inner->type = msg->type;
#if PACKETVER >= 20170419
		inner->openType = msg->opentype;
#else
		inner->regDateTime = (int)time(NULL) - msg->send_date;
#endif
		inner->expireDateTime = msg->expire_date - (int)time(NULL);
		if (open_type == RODEX_OPENTYPE_RETURN) {
			inner->expireDateTime += RODEX_EXPIRE;
		}
		inner->Titlelength = (int16)strlen(msg->title) + 1;
		if (open_type != RODEX_OPENTYPE_RETURN) {
			strncpy(inner->SenderName, msg->sender_name, sizeof(inner->SenderName));
		} else {
			strncpy(inner->SenderName, msg->receiver_name, sizeof(inner->SenderName));
		}
		strncpy(inner->title, msg->title, inner->Titlelength);
		size += sizeof(*inner) + inner->Titlelength;
		inner = WFIFOP(fd, size);
		--j;
	}

	packet->PacketLength = size;
#if PACKETVER < 20170419
	packet->cnt = count;
#endif
	packet->IsEnd = 1;
	WFIFOSET(fd, size);
#endif
}

static void clif_parse_rodex_next_maillist(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_next_maillist(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_NEXT_MAIL_LIST *packet = RFIFOP(fd, 0);

	rodex->next_page(sd, packet->opentype, packet->Lower_MailID);
}

static void clif_parse_rodex_read_mail(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_read_mail(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_READ_MAIL *rPacket = RFIFOP(fd, 0);

	rodex->read_mail(sd, rPacket->MailID);
}

static void clif_rodex_read_mail(struct map_session_data *sd, int8 opentype, struct rodex_message *msg)
{
#if PACKETVER >= 20140115
	struct PACKET_ZC_READ_MAIL *sPacket;
	struct mail_item *item;
	int fd, i, body_len, size;

	nullpo_retv(sd);
	nullpo_retv(msg);

	fd = sd->fd;
	body_len = (int)strlen(msg->body) + 1;
	size = sizeof(*sPacket);

	WFIFOHEAD(fd, sizeof(*sPacket) + body_len + (sizeof(*item) * RODEX_MAX_ITEM));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexread;
	sPacket->opentype = opentype;
	sPacket->MailID = msg->id;
	sPacket->TextcontentsLength = body_len;
	sPacket->zeny = msg->zeny;
	sPacket->ItemCnt = msg->items_count;
	strncpy(WFIFOP(fd, size), msg->body, body_len);
	size += body_len;
	for (i = 0; i < RODEX_MAX_ITEM; ++i) {
		struct item *it = &msg->items[i].item;
		struct item_data *data;
		int j, k;

		if (it->nameid == 0)
			continue;
		data = itemdb->search(it->nameid);
		if (data == NULL)
			continue;

		item = WFIFOP(fd, size);
		memset(item, 0x0, sizeof(*item));
		item->ITID = it->nameid;
		item->count = it->amount;
		item->type = itemtype(itemdb->search(it->nameid)->type);
		item->IsIdentified = it->identify ? 1 : 0;
		item->IsDamaged = (it->attribute & ATTR_BROKEN) != 0 ? 1 : 0;
		item->refiningLevel = it->refine;
		item->location = pc->item_equippoint(sd, data);
		item->viewSprite = data->view_sprite;
		item->bindOnEquip = it->bound ? 2 : data->flag.bindonequip ? 1 : 0;
		for (k = 0; k < MAX_SLOTS; ++k) {
			item->slot.card[k] = it->card[k];
		}
		for (j = 0; j < MAX_ITEM_OPTIONS; ++j) {
			item->optionData[j].index = it->option[j].index;
			item->optionData[j].value = it->option[j].value;
		}

		size += sizeof(*item);
	}
	sPacket->PacketLength = size;
	WFIFOSET(fd, size);
#endif
}

static void clif_parse_rodex_delete_mail(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_delete_mail(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_DELETE_MAIL *rPacket = RFIFOP(fd, 0);

	rodex->delete_mail(sd, rPacket->MailID);
}

static void clif_rodex_delete_mail(struct map_session_data *sd, int8 opentype, int64 mail_id)
{
#if PACKETVER >= 20131218
	struct PACKET_ZC_ACK_DELETE_MAIL *sPacket;
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexdelete;
	sPacket->opentype = opentype;
	sPacket->MailID = mail_id;
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_parse_rodex_request_zeny(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_request_zeny(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_ZENY_FROM_MAIL *rPacket = RFIFOP(fd, 0);

	rodex->get_zeny(sd, rPacket->opentype, rPacket->MailID);
}

static void clif_rodex_request_zeny(struct map_session_data *sd, int8 opentype, int64 mail_id, enum rodex_get_zeny result)
{
#if PACKETVER >= 20140409
	struct PACKET_ZC_ACK_ZENY_FROM_MAIL *sPacket;
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexgetzeny;
	sPacket->MailID = mail_id;
	sPacket->opentype = opentype;
	sPacket->result = result;
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_parse_rodex_request_items(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_request_items(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_ITEM_FROM_MAIL *rPacket = RFIFOP(fd, 0);

	rodex->get_items(sd, rPacket->opentype, rPacket->MailID);
}

static void clif_rodex_request_items(struct map_session_data *sd, int8 opentype, int64 mail_id, enum rodex_get_items result)
{
#if PACKETVER >= 20140409
	struct PACKET_ZC_ACK_ITEM_FROM_MAIL *sPacket;
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, sizeof(*sPacket));
	sPacket = WFIFOP(fd, 0);
	sPacket->PacketType = rodexgetitem;
	sPacket->MailID = mail_id;
	sPacket->opentype = opentype;
	sPacket->result = result;
	WFIFOSET(fd, sizeof(*sPacket));
#endif
}

static void clif_rodex_icon(int fd, bool show)
{
// packet add date is 20140716, but from players reports it wrong. Using closer known correct version.
#if PACKETVER >= 20141112
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) = rodexicon;
	WFIFOB(fd, 2) = (show == true ? 1 : 0);
	WFIFOSET(fd, 3);
#endif
}

static void clif_parse_rodex_refresh_maillist(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_refresh_maillist(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_REFRESH_MAIL_LIST *packet = RFIFOP(fd, 0);
#if PACKETVER >= 20170419
	rodex->refresh(sd, RODEX_OPENTYPE_UNSET, packet->Upper_MailID);
#else
	rodex->refresh(sd, packet->opentype, packet->Upper_MailID);
#endif
}

static void clif_parse_rodex_open_mailbox(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_open_mailbox(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_OPEN_MAIL *packet = RFIFOP(fd, 0);
#if PACKETVER >= 20170419
	rodex->open(sd, RODEX_OPENTYPE_UNSET, packet->char_Upper_MailID);
#else
	rodex->open(sd, packet->opentype, packet->Upper_MailID);
#endif
	rodex->clean(sd, 1);
}

static void clif_parse_rodex_close_mailbox(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_close_mailbox(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	rodex->clean(sd, 0);
	intif->rodex_checkhasnew(sd);
}

static void clif_parse_rodex_cancel_write_mail(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_rodex_cancel_write_mail(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	rodex->clean(sd, 1);
}

static void clif_skill_scale(struct block_list *bl, int src_id, int x, int y, uint16 skill_id, uint16 skill_lv, int casttime)
{
#if PACKETVER >= 20151223
	struct PACKET_ZC_SKILL_SCALE p;

	p.PacketType = skillscale;
	p.AID = src_id;
	p.skill_id = skill_id;
	p.skill_lv = skill_lv;
	p.x = x;
	p.y = y;
	p.casttime = casttime;

	if (clif->isdisguised(bl)) {
		clif->send(&p, sizeof(p), bl, AREA_WOS);
		p.AID = -src_id;
		clif->send(&p, sizeof(p), bl, SELF);
	} else {
		clif->send(&p, sizeof(p), bl, AREA);
	}
#else
	ShowWarning("clif_skill_scale: showing skill scale available only for clients >= 20151223.");
	return;
#endif
}

/// Send hat effects to the client (ZC_HAT_EFFECT).
/// 0A3B <Length>.W <AID>.L <Status>.B { <HatEffectId>.W }
static void clif_hat_effect(struct block_list *bl, struct block_list *tbl, enum send_target target)
{
#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	nullpo_retv(bl);
	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	nullpo_retv(sd);

	const int len = sizeof(struct PACKET_ZC_HAT_EFFECT) + VECTOR_LENGTH(sd->hatEffectId) * 2;
	struct PACKET_ZC_HAT_EFFECT *p = aMalloc(len);

	p->packetType = HEADER_ZC_HAT_EFFECT;
	p->packetLength = len;
	p->aid = bl->id;
	p->status = 1;

	for (int i = 0; i < VECTOR_LENGTH(sd->hatEffectId); i++) {
		p->effects[i] = VECTOR_INDEX(sd->hatEffectId, i);
	}

	if (tbl != NULL) {
		clif->send(p, len, tbl, target);
	} else {
		clif->send(p, len, bl, target);
	}
	aFree(p);
#endif
}

static void clif_hat_effect_single(struct block_list *bl, uint16 effectId, bool enable){
#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	nullpo_retv(bl);

	const int len = sizeof(struct PACKET_ZC_HAT_EFFECT) + 2;
	struct PACKET_ZC_HAT_EFFECT *p = aMalloc(len);

	p->packetType = HEADER_ZC_HAT_EFFECT;
	p->packetLength = len;
	p->aid = bl->id;
	p->status = enable;
	p->effects[0] = effectId;

	clif->send(p, len, bl, AREA);
	aFree(p);
#endif
}

static bool clif_parse_attendance_db(void)
{
	struct config_t attendance_conf;
	struct config_setting_t *attendance = NULL, *it = NULL;
	char config_filename[256];
	libconfig->format_db_path("attendance_db.conf", config_filename, sizeof(config_filename));
	int i = 0;

	if (!libconfig->load_file(&attendance_conf, config_filename))
		return false;
	attendance = libconfig->lookup(&attendance_conf, "attendance_db");

	VECTOR_CLEAR(clif->attendance_data);

	while ((it = libconfig->setting_get_elem(attendance, i++))) {
		clif->attendancedb_libconfig_sub(it, i, config_filename);
	}

	libconfig->destroy(&attendance_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, config_filename);
	return true;
}

static bool clif_attendancedb_libconfig_sub(struct config_setting_t *it, int n, const char *source)
{
	struct attendance_entry entry = { 0 };
	int i32 = 0;

	nullpo_ret(it);
	nullpo_ret(source);

	if (!itemdb->lookup_const(it, "ItemID", &i32) || i32 < 0) {
		ShowWarning("clif_attendancedb_libconfig_sub: unknown item %d, entry #%d, skipping.\n", i32, n);
		return false;
	}
	entry.nameid = i32;

	if (!libconfig->setting_lookup_int(it, "Amount", &i32) || i32 < 1) {
		ShowWarning("clif_attendancedb_libconfig_sub: invalid amount %d, entry #%d, skipping.\n", i32, n);
		return false;
	}
	entry.qty = i32;

	VECTOR_ENSURE(clif->attendance_data, 1, 1);
	VECTOR_PUSH(clif->attendance_data, entry);
	return true;
}

static bool clif_attendance_timediff(struct map_session_data *sd)
{
	int64 timediff;

	nullpo_retr(false, sd);

	timediff = (time(NULL) / (60 * 60 * 24)) - (sd->status.attendance_timer / (60 * 60 * 24));

	if (timediff <= 0)
		return false;
	return true;
}
static time_t clif_attendance_getendtime(void)
{
	time_t timestamp;
	struct tm tmtime = { 0 };
	int year = 0, month = 0, day = 0;
	char timestring[9];

	sprintf(timestring, "%8d", battle_config.feature_attendance_endtime);
	sscanf(timestring, "%4d%2d%2d", &year, &month, &day);

	tmtime.tm_year = year - 1900;
	tmtime.tm_mon = month - 1;
	tmtime.tm_mday = day;

	timestamp = mktime(&tmtime);

	return timestamp;
}

static void clif_parse_open_ui_request(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_open_ui_request(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_OPEN_UI *p = RP2PTR(fd);

	clif->open_ui(sd, p->UIType);
}

/**
 * Does the actual packet sending for clif_open_ui().
 *
 * @param sd The character who opens the UI.
 * @param ui_type The UI which should be opened.
 *
 **/
static void clif_open_ui_send(struct map_session_data *sd, enum zc_ui_types ui_type)
{
	nullpo_retv(sd);

#if PACKETVER >= 20150128
	struct PACKET_ZC_OPEN_UI p;

	p.PacketType = openUiType;
	p.UIType = ui_type;

	switch (ui_type) {
	case ZC_BANK_UI:
	case ZC_STYLIST_UI:
	case ZC_CAPTCHA_UI:
	case ZC_MACRO_UI:
#if PACKETVER >= 20171122
		p.data = 0;
#endif
		break;
#if PACKETVER >= 20171122
	case ZC_TIPBOX_UI:
	case ZC_RENEWQUEST_UI:
		p.data = 0;
		break;
	case ZC_ATTENDANCE_UI:
		if (battle_config.feature_enable_attendance_system == 0)
			return;

		if (clif->attendance_getendtime() < time(NULL)) {
#if PACKETVER >= 20180207
			clif->msgtable_color(sd, MSG_ATTENDANCE_UNAVAILABLE, COLOR_RED);
#endif
			return;
		}

#if PACKETVER_RE_NUM >= 20180307 || PACKETVER_MAIN_NUM >= 20180404 || PACKETVER_ZERO_NUM >= 20180411
		int claimed = 0;

		if (!clif->attendance_timediff(sd))
			++claimed;
		else if (sd->status.attendance_count >= VECTOR_LENGTH(clif->attendance_data))
			sd->status.attendance_count = 0;

		p.data = sd->status.attendance_count * 10 + claimed;
#else
		ShowWarning("Attendance System available only for PACKETVER_RE_NUM >= 20180307 || PACKETVER_MAIN_NUM >= 20180404 || PACKETVER_ZERO_NUM >= 20180411.\n");
		return;
#endif
		break;
#endif
	default:
		ShowWarning("clif_open_ui_send: Requested UI (%u) is not implemented yet.\n", ui_type);
		return;
	}

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_open_ui(struct map_session_data *sd, enum cz_ui_types uiType)
{
	nullpo_retv(sd);

	enum zc_ui_types send_ui_type;

	switch (uiType) {
#if PACKETVER >= 20150128
	case CZ_STYLIST_UI:
		send_ui_type = ZC_STYLIST_UI;
		break;
	case CZ_MACRO_REGISTER_UI:
		send_ui_type = ZC_CAPTCHA_UI;
		break;
	case CZ_MACRO_DETECTOR_UI:
		send_ui_type = ZC_MACRO_UI;
		break;
#endif
#if PACKETVER >= 20171122
	case CZ_ATTENDANCE_UI:
		send_ui_type = ZC_ATTENDANCE_UI;
		break;
#endif
	default:
		ShowWarning("clif_open_ui: Requested UI (%u) is not implemented yet.\n", uiType);
		return;
	}

	clif->open_ui_send(sd, send_ui_type);
}

static void clif_parse_attendance_reward_request(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_attendance_reward_request(int fd, struct map_session_data *sd)
{
#if PACKETVER_RE_NUM >= 20180307 || PACKETVER_MAIN_NUM >= 20180404 || PACKETVER_ZERO_NUM >= 20180411
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	struct rodex_message msg = { 0 };
	struct attendance_entry *entry;
	int attendance_count;
	char title[RODEX_TITLE_LENGTH], body[MAIL_BODY_LENGTH];

	if (clif->attendance_getendtime() < time(NULL)) {
		clif->msgtable_color(sd, MSG_ATTENDANCE_UNAVAILABLE, COLOR_RED);
		return;
	}

	if (battle_config.feature_enable_attendance_system != 1)
		return;

	if (clif->attendance_timediff(sd) != true)
		return;

	if (sd->status.attendance_count >= VECTOR_LENGTH(clif->attendance_data))
		sd->status.attendance_count = 0;

	attendance_count = sd->status.attendance_count;
	++sd->status.attendance_count;
	sd->status.attendance_timer = time(NULL);

	msg.receiver_id = sd->status.char_id;
	sprintf(title, msg_txt(545), attendance_count + 1);
	sprintf(body, msg_txt(545), attendance_count + 1);

	entry = &VECTOR_INDEX(clif->attendance_data, attendance_count);
	msg.items[0].item.nameid = entry->nameid;
	msg.items[0].item.amount = entry->qty;
	msg.items[0].item.identify = 1;
	msg.type = MAIL_TYPE_NPC | MAIL_TYPE_ITEM;

	safestrncpy(msg.sender_name, msg_txt(544), NAME_LENGTH);
	safestrncpy(msg.title, title, RODEX_TITLE_LENGTH);
	safestrncpy(msg.body, body, MAIL_BODY_LENGTH);
	msg.send_date = (int)time(NULL);
	msg.expire_date = (int)time(NULL) + RODEX_EXPIRE;

	intif->rodex_sendmail(&msg);
	clif->ui_action(sd, 0, sd->status.attendance_count);
#else
	ShowWarning("Attendance System available only for PACKETVER_RE_NUM >= 20180307 || PACKETVER_MAIN_NUM >= 20180404 || PACKETVER_ZERO_NUM >= 20180411.\n");
#endif
}

static void clif_parse_cz_blocking_play_cancel(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_cz_blocking_play_cancel(int fd, struct map_session_data *sd)
{
	clif->loadConfirm(sd);
}

static void clif_loadConfirm(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320 || PACKETVER_ZERO_NUM >= 20190410
	nullpo_retv(sd);
	struct PACKET_ZC_LOAD_CONFIRM p;
	p.packetType = HEADER_ZC_LOAD_CONFIRM;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_ui_action(struct map_session_data *sd, int32 UIType, int32 data)
{

	struct PACKET_ZC_UI_ACTION p;

	nullpo_retv(sd);

	p.PacketType = 0xAF0;
	p.UIType = UIType;
	p.data = data;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
}

static void clif_parse_private_airship_request(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_private_airship_request(int fd, struct map_session_data *sd)
{
#if PACKETVER_RE_NUM >= 20180321 || PACKETVER_MAIN_NUM >= 20180620 || defined(PACKETVER_ZERO)
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;
	const struct PACKET_CZ_PRIVATE_AIRSHIP_REQUEST *p = RP2PTR(fd);

	safestrncpy(evname, "private_airship::OnAirShipRequest", EVENT_NAME_LENGTH);
	if ((ev = strdb_get(npc->ev_db, evname))) {
		pc->setregstr(sd, script->add_variable("@mapname$"), p->mapName);
		pc->setreg(sd, script->add_variable("@itemid"), p->ItemID);
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else {
		ShowError("clif_parse_private_airship_request: event '%s' not found, operation failed.\n", evname);
	}
#else
	ShowWarning("clif_parse_private_airship_request: private airship is not supported in this client version, possible packet manipulation.\n");
#endif
}

static void clif_private_airship_response(struct map_session_data *sd, uint32 flag)
{
#if PACKETVER_RE_NUM >= 20180321 || PACKETVER_MAIN_NUM >= 20180620 || defined(PACKETVER_ZERO)
	struct PACKET_ZC_PRIVATE_AIRSHIP_RESPONSE p;

	nullpo_retv(sd);

	if (flag > P_AIRSHIP_ITEM_INVALID) {
		ShowError("clif_private_airship_response: invalid flag given '%u', defaulting to 0.\n", flag);
		flag = 0;
	}

	p.PacketType = 0xA4A;
	p.flag = flag;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#else
	ShowWarning("clif_private_airship_response: private airship works only for clients PACKETVER_RE_NUM >= 20180321 || PACKETVER_MAIN_NUM >= 20180620.\n");
#endif
}

static void clif_parse_cz_req_style_change(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_cz_req_style_change(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_STYLE_CHANGE *p = RP2PTR(fd);

	if (p->HeadStyle > 0)
		stylist->request_style_change(sd, LOOK_HAIR, p->HeadStyle, false);
	if (p->HeadPalette > 0)
		stylist->request_style_change(sd, LOOK_HAIR_COLOR, p->HeadPalette, false);
	if (p->BodyPalette > 0)
		stylist->request_style_change(sd, LOOK_CLOTHES_COLOR, p->BodyPalette, false);
	if (p->TopAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_TOP, p->TopAccessory, true);
	if (p->MidAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_MID, p->MidAccessory, true);
	if (p->BottomAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_BOTTOM, p->BottomAccessory, true);
	clif->style_change_response(sd, STYLIST_SHOP_SUCCESS);
	return;
}

static void clif_parse_cz_req_style_change2(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_cz_req_style_change2(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_REQ_STYLE_CHANGE2 *p = RP2PTR(fd);

	if (p->HeadStyle > 0)
		stylist->request_style_change(sd, LOOK_HAIR, p->HeadStyle, false);
	if (p->HeadPalette > 0)
		stylist->request_style_change(sd, LOOK_HAIR_COLOR, p->HeadPalette, false);
	if (p->BodyPalette > 0)
		stylist->request_style_change(sd, LOOK_CLOTHES_COLOR, p->BodyPalette, false);
	if (p->TopAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_TOP, p->TopAccessory, true);
	if (p->MidAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_MID, p->MidAccessory, true);
	if (p->BottomAccessory > 0)
		stylist->request_style_change(sd, LOOK_HEAD_BOTTOM, p->BottomAccessory, true);
	if (p->BodyStyle > 0) {
		if (pc->has_second_costume(sd)) {
			stylist->request_style_change(sd, LOOK_BODY2, p->BodyStyle, false);
		}
	}
	clif->style_change_response(sd, STYLIST_SHOP_SUCCESS);
	return;
}

static void clif_parse_cz_style_close(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_cz_style_close(int fd, struct map_session_data *sd)
{
	// do nothing
}

static void clif_style_change_response(struct map_session_data *sd, enum stylist_shop flag)
{
#if PACKETVER >= 20151104
	struct PACKET_ZC_STYLE_CHANGE_RES p;

	nullpo_retv(sd);

	p.PacketType = 0x0a47;
	p.flag = flag;

	clif->send(&p, sizeof(p), &sd->bl, SELF);
#else
	ShowWarning("clif_style_change_response: unsupported client version is being used.");
#endif
}

static void clif_overweight_percent(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20171108 || PACKETVER_RE_NUM >= 20171025 || PACKETVER_ZERO_NUM >= 20171019
	struct PACKET_ZC_OVERWEIGHT_PERCENT p;

	nullpo_retv(sd);

	p.packetType = 0xade;
	p.percent = battle_config.natural_heal_weight_rate;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_parse_changeDress(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
/// 0ae8 <packet len>.W
static void clif_parse_changeDress(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const char commandname[] = "changedress";
	char command[sizeof commandname + 3] = ""; // '@' command + ' ' + NUL

	sprintf(command, "%c%s ", atcommand->at_symbol, commandname);
	atcommand->exec(fd, sd, command, true);
}

static void clif_party_dead_notification(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	struct PACKET_ZC_GROUP_ISALIVE p;

	nullpo_retv(sd);

	p.packetType = 0xab2;
	p.AID = sd->bl.id;
	p.isDead = pc_isdead(sd);
	clif->send(&p, sizeof(p), &sd->bl, PARTY_WOS);
#endif
}

static void clif_parse_memorial_dungeon_command(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_memorial_dungeon_command(int fd, struct map_session_data *sd)
{
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_MEMORIALDUNGEON_COMMAND *p = RP2PTR(fd);

	switch (p->command) {
	case COMMAND_MEMORIALDUNGEON_DESTROY_FORCE:
		instance->force_destroy(sd);
	}
}

static void clif_camera_showWindow(struct map_session_data *sd)
{
#if PACKETVER >= 20160525
	nullpo_retv(sd);
	struct PACKET_ZC_CAMERA_INFO p;
	p.packetType = HEADER_ZC_CAMERA_INFO;
	p.action = 1;
	p.range = 0;
	p.rotation = 0;
	p.latitude = 0;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static void clif_camera_change(struct map_session_data *sd, float range, float rotation, float latitude, enum send_target target)
{
#if PACKETVER >= 20160525
	nullpo_retv(sd);
	struct PACKET_ZC_CAMERA_INFO p;
	p.packetType = HEADER_ZC_CAMERA_INFO;
	p.action = 0;
	p.range = range;
	p.rotation = rotation;
	p.latitude = latitude;
	clif->send(&p, sizeof(p), &sd->bl, target);
#endif
}

static void clif_parse_cameraInfo(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_cameraInfo(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20160525
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_CAMERA_INFO *const p = RFIFOP(fd, 0);
	char command[100];
	if (p->action == 1) {
		sprintf(command, "%ccamerainfo", atcommand->at_symbol);
	} else {
		sprintf(command, "%ccamerainfo %15f %15f %15f", atcommand->at_symbol, p->range, p->rotation, p->latitude);
	}
	atcommand->exec(fd, sd, command, true);
#endif
}

// show item preview in already opened preview window
static void clif_item_preview(struct map_session_data *sd, int n)
{
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	Assert_retv(n >= 0 && n < sd->status.inventorySize);

	struct PACKET_ZC_ITEM_PREVIEW p;
	p.packetType = HEADER_ZC_ITEM_PREVIEW;
	p.index = n + 2;
#if PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024
	p.isDamaged = (sd->status.inventory[n].attribute & ATTR_BROKEN) != 0 ? 1 : 0;
#endif
	p.refiningLevel = sd->status.inventory[n].refine;
	clif->addcards(&p.slot, &sd->status.inventory[n]);
	clif->add_item_options(&p.option_data[0], &sd->status.inventory[n]);
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

// insert cardId into equipped item in pos equipment slot into slot cardSlot.
static bool clif_enchant_equipment(struct map_session_data *sd, enum equip_pos pos, int cardSlot, int cardId)
{
#if PACKETVER_MAIN_NUM >= 20160831 || PACKETVER_RE_NUM >= 20151118 || defined(PACKETVER_ZERO)
	nullpo_ret(sd);
	Assert_ret(cardSlot >= 0 && cardSlot < MAX_SLOTS);
	struct PACKET_ZC_ENCHANT_EQUIPMENT p;
	p.packetType = HEADER_ZC_ENCHANT_EQUIPMENT;
	p.wearState = pos;
	p.cardSlot = cardSlot;
	p.itemId = cardId;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
	return true;
#else
	return false;
#endif
}

static void clif_npc_barter_open(struct map_session_data *sd, struct npc_data *nd)
{
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	nullpo_retv(sd);
	nullpo_retv(nd);
	struct npc_item_list *shop = nd->u.scr.shop->item;
	const int shop_size = nd->u.scr.shop->items;

	int c = 0;
	int maxCount = (sizeof(packet_buf) - sizeof(struct PACKET_ZC_NPC_BARTER_OPEN)) / sizeof(struct PACKET_ZC_NPC_BARTER_OPEN_sub);
	struct PACKET_ZC_NPC_BARTER_OPEN *packet = (struct PACKET_ZC_NPC_BARTER_OPEN*)&packet_buf[0];
	packet->packetType = HEADER_ZC_NPC_BARTER_OPEN;

	for (int i = 0; i < shop_size && c < maxCount; i++) {
		if (shop[i].nameid) {
			struct item_data *id = itemdb->exists(shop[i].nameid);
			if (id == NULL)
				continue;

			packet->list[c].nameid = shop[i].nameid;
			packet->list[c].type   = itemtype(id->type);
			packet->list[c].amount = shop[i].qty;
			packet->list[c].currencyNameid = shop[i].value;
			packet->list[c].currencyAmount = shop[i].value2;
			packet->list[c].weight = id->weight * 10;
			packet->list[c].index = i;
			c++;
		}
	}

	packet->packetLength = sizeof(struct PACKET_ZC_NPC_BARTER_OPEN) + sizeof(struct PACKET_ZC_NPC_BARTER_OPEN_sub) * c;
	clif->send(packet, packet->packetLength, &sd->bl, SELF);
#endif
}

static void clif_parse_NPCBarterPurchase(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_NPCBarterPurchase(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_NPC_BARTER_PURCHASE *p = RP2PTR(fd);
	int count = (p->packetLength - sizeof(struct PACKET_CZ_NPC_BARTER_PURCHASE)) / sizeof p->list[0];
	struct barteritemlist item_list;

	Assert_retv(count >= 0 && count <= sd->status.inventorySize);

	VECTOR_INIT(item_list);
	VECTOR_ENSURE(item_list, count, 1);

	for (int i = 0; i < count; i++) {
		struct barter_itemlist_entry entry = { 0 };
		entry.addId = p->list[i].itemId;
		entry.addAmount = p->list[i].amount;
		entry.removeIndex = p->list[i].invIndex - 2;
		entry.shopIndex = p->list[i].shopIndex;

		VECTOR_PUSH(item_list, entry);
	}

	int response = npc->barter_buylist(sd, &item_list);
	clif->npc_buy_result(sd, response);

	VECTOR_CLEAR(item_list);
#endif
}

static void clif_parse_npc_expanded_barter_closed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_npc_expanded_barter_closed(int fd, struct map_session_data *sd)
{
}

#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
#define NEXT_EXPANDED_BARTER_ITEM(var, count) \
	var = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *)((char*)item + \
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub) - \
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) + \
		count * sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2))
#endif

static void clif_npc_expanded_barter_open(struct map_session_data *sd, struct npc_data *nd)
{
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
	nullpo_retv(sd);
	nullpo_retv(nd);
	struct npc_item_list *shop = nd->u.scr.shop->item;
	const int shop_size = nd->u.scr.shop->items;

	int items_count = 0;
	int currencies_count = 0;
	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN *packet = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN*)&packet_buf[0];
	STATIC_ASSERT(sizeof(packet_buf) > sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN), "packet_buf size too small");
	int buf_left = sizeof(packet_buf) - sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN);
	packet->packetType = HEADER_ZC_NPC_EXPANDED_BARTER_OPEN;
	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *item = &packet->items[0];

	// Workaround for fix Visual Studio bug (error C2233)
	// Here should be sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub)
	const int ptr_size = sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub) -
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2);
	for (int i = 0; i < shop_size && buf_left >= ptr_size; i++) {
		if (shop[i].nameid) {
			struct item_data *id = itemdb->exists(shop[i].nameid);
			if (id == NULL)
				continue;

			item->nameid = shop[i].nameid;
			item->type   = itemtype(id->type);
			item->amount = shop[i].qty;
			item->weight = id->weight * 10;
			item->index  = i;
			item->zeny   = shop[i].value;
			item->currency_count = 0;
			buf_left -= ptr_size;
			items_count ++;
			int count = shop[i].value2;
			if (buf_left < sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) * count) {
				NEXT_EXPANDED_BARTER_ITEM(item, 0);
				break;
			}
			for (int j = 0; j < count; j ++) {
				struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2 *packet_currency = &item->currencies[j];
				struct npc_barter_currency *currency = &shop[i].currency[j];
				struct item_data *id2 = itemdb->exists(currency->nameid);
				if (id2 == NULL)
					continue;
				packet_currency->nameid = currency->nameid;
				if (currency->refine == -1)
					packet_currency->refine_level = 0;
				else
					packet_currency->refine_level = currency->refine;
				packet_currency->amount = currency->amount;
				packet_currency->type = itemtype(id2->type);
				currencies_count ++;
				item->currency_count ++;
			}
			NEXT_EXPANDED_BARTER_ITEM(item, item->currency_count);
		}
	}

	packet->items_count = items_count;
	packet->packetLength = sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN) +
		ptr_size * items_count +
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) * currencies_count;
	clif->send(packet, packet->packetLength, &sd->bl, SELF);
#endif
}

#undef NEXT_EXPANDED_BARTER_ITEM

static void clif_parse_npc_expanded_barter_purchase(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_npc_expanded_barter_purchase(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE *const p = RP2PTR(fd);
	int count = (p->packetLength - sizeof(struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE)) / sizeof p->list[0];
	struct barteritemlist item_list;

	Assert_retv(count >= 0 && count <= sd->status.inventorySize);

	VECTOR_INIT(item_list);
	VECTOR_ENSURE(item_list, count, 1);

	for (int i = 0; i < count; i++) {
		struct barter_itemlist_entry entry = { 0 };
		entry.addId = p->list[i].itemId;
		entry.addAmount = p->list[i].amount;
		entry.removeIndex = -1;
		entry.shopIndex = p->list[i].shopIndex;
		VECTOR_PUSH(item_list, entry);
	}

	int response = npc->expanded_barter_buylist(sd, &item_list);
	clif->npc_buy_result(sd, response);

	VECTOR_CLEAR(item_list);
#endif
}

static void clif_parse_clientVersion(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_clientVersion(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20090406 || PACKETVER_RE_NUM >= 20090408 || PACKETVER_SAK_NUM >= 20090408 || defined(PACKETVER_ZERO)
	// TODO: show or store client version
#endif
}

static void clif_parse_ping(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_ping(int fd, struct map_session_data *sd)
{
	// do nothing, any packet update client tick
}

static void clif_ping(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190227 || PACKETVER_RE_NUM >= 20190220 || PACKETVER_ZERO_NUM >= 20190220
	nullpo_retv(sd);
	struct PACKET_ZC_PING p;
	p.packetType = HEADER_ZC_PING;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

static int clif_pingTimer(int tid, int64 tick, int id, intptr_t data)
{
	map->foreachpc(clif->pingTimerSub, time(NULL));
	return 0;
}

static int clif_pingTimerSub(struct map_session_data *sd, va_list ap)
{
	nullpo_ret(sd);
	const int fd = sd->fd;

	if (!sockt->session_is_active(fd))
        {
		return 0;
        }

	time_t tick = va_arg(ap, time_t);

	if (sockt->session[fd]->wdata_tick + battle_config.ping_time < tick)
        {
		clif->ping(sd);
        }
	return 0;
}

static void clif_parse_ResetCooldown(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_ResetCooldown(int fd, struct map_session_data *sd)
{
	char cmd[15];
	sprintf(cmd,"%ccddebug reset", atcommand->at_symbol);
	atcommand->exec(fd, sd, cmd, true);
}

static void clif_OpenRefineryUI(struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20161130 || PACKETVER_RE_NUM >= 20161109 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	if (battle_config.enable_refinery_ui == 0)
		return;

	struct PACKET_ZC_REFINE_OPEN_WINDOW p;
	p.packetType = HEADER_ZC_REFINE_OPEN_WINDOW;
	clif->send(&p, sizeof(p), &sd->bl, SELF);

	sd->state.refine_ui = 1;
#endif
}

static void clif_parse_AddItemRefineryUI(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_AddItemRefineryUI(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20161005 || PACKETVER_RE_NUM >= 20161005 || defined(PACKETVER_ZERO)
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (battle_config.enable_refinery_ui == 0)
		return;

	const struct PACKET_CZ_REFINE_ADD_ITEM *p = RFIFO2PTR(fd);
	refine->refinery_add_item(sd, p->index - 2);
#endif
}

static void clif_AddItemRefineryUIAck(struct map_session_data *sd, int item_index, struct s_refine_requirement *req)
{
#if PACKETVER_MAIN_NUM >= 20161130 || PACKETVER_RE_NUM >= 20161109 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);
	nullpo_retv(req);
	Assert_retv(item_index >= 0 && item_index < sd->status.inventorySize);

	if (battle_config.enable_refinery_ui == 0)
		return;

	char buf[sizeof(struct PACKET_ZC_REFINE_ADD_ITEM) + sizeof(struct PACKET_ZC_REFINE_ADD_ITEM_SUB) * MAX_REFINE_REQUIREMENTS];
	struct PACKET_ZC_REFINE_ADD_ITEM *p = (struct PACKET_ZC_REFINE_ADD_ITEM *)buf;

	p->packetType = HEADER_ZC_REFINE_ADD_ITEM;
	p->packtLength = sizeof(*p) + sizeof(p->req[0]) * req->req_count;
	p->itemIndex = item_index + 2;
	p->blacksmithBlessing = req->blacksmith_blessing;

	int weapon_level = itemdb_wlv(sd->status.inventory[item_index].nameid);
	for (int i = 0; i < req->req_count; ++i) {
		p->req[i].chance = refine->get_refine_chance(weapon_level, sd->status.inventory[item_index].refine, req->req[i].type);
		p->req[i].itemId = req->req[i].nameid;
		p->req[i].zeny = req->req[i].cost;
	}

	clif->send(p, p->packtLength, &sd->bl, SELF);
#endif
}

static void clif_parse_RefineryUIRefine(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_RefineryUIRefine(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20161005 || PACKETVER_RE_NUM >= 20161005 || defined(PACKETVER_ZERO)
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (battle_config.enable_refinery_ui == 0)
		return;

	const struct PACKET_CZ_REFINE_ITEM_REQUEST *p = RFIFO2PTR(fd);
	refine->refinery_refine_request(sd, p->index - 2, p->itemId, (p->blacksmithBlessing == 1) ? true : false);
#endif
}

static void clif_parse_RefineryUIClose(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_RefineryUIClose(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20161130 || PACKETVER_RE_NUM >= 20161109 || defined(PACKETVER_ZERO)
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	if (battle_config.enable_refinery_ui == 0)
		return;

	sd->state.refine_ui = 0;
	return;
#endif
}

static void clif_announce_refine_status(struct map_session_data *sd, int item_id, int refine_level, bool success, enum send_target target)
{
#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	nullpo_retv(sd);

	Assert_retv(refine_level > 0 && refine_level <= INT8_MAX);

	struct PACKET_ZC_REFINE_STATUS p;
	p.packetType = HEADER_ZC_REFINE_STATUS;
	safestrncpy(p.name, sd->status.name, NAME_LENGTH);
	p.itemId = item_id;
	p.refine_level = refine_level;
	p.status = (success) ? true : false;
	clif->send(&p, sizeof(p), &sd->bl, target);
#endif
}

static void clif_parse_GuildCastleTeleportRequest(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_GuildCastleTeleportRequest(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
	const struct PACKET_CZ_CASTLE_TELEPORT_REQUEST *p = RFIFO2PTR(fd);
	struct guild *g = sd->guild;

	if (g == NULL)
		return;

	struct guild_castle *gc = guild->castle_search(p->castle_id);
	if (gc == NULL)
		return;
	if (gc->enable_client_warp == false)
		return;
	if (gc->guild_id != g->guild_id)
		return;

	if (map->list[sd->bl.m].flag.gvg_castle == 1)
		return;

	int zeny = gc->client_warp.zeny;
	if (gc->siege_type == SIEGE_TYPE_FE && map->agit_flag == 1) {
		zeny = gc->client_warp.zeny_siege;
	} else if (gc->siege_type == SIEGE_TYPE_SE && map->agit2_flag == 1) {
		zeny = gc->client_warp.zeny_siege;
	} else if (gc->siege_type == SIEGE_TYPE_TE) {
		clif->guild_castleteleport_res(sd, SIEGE_TP_INVALID_MODE);
		return;
	}

	if (sd->status.zeny < zeny) {
		clif->guild_castleteleport_res(sd, SIEGE_TP_NOT_ENOUGH_ZENY);
		return;
	}
	sd->status.zeny -= zeny;
	clif->updatestatus(sd, SP_ZENY);
	pc->setpos(sd, gc->mapindex, gc->client_warp.x, gc->client_warp.y, CLR_OUTSIGHT);
#endif
}

static void clif_guild_castleteleport_res(struct map_session_data *sd, enum siege_teleport_result result)
{
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814

	nullpo_retv(sd);

	struct PACKET_ZC_CASTLE_TELEPORT_RESPONSE p = { 0 };
	p.packetType = HEADER_ZC_CASTLE_TELEPORT_RESPONSE;
	p.result = (int16)result;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
#endif
}
static void clif_parse_GuildCastleInfoRequest(int fd, struct map_session_data *sd) __attribute__((nonnull(2)));
static void clif_parse_GuildCastleInfoRequest(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
	const struct PACKET_CZ_CASTLE_INFO_REQUEST *p = RFIFO2PTR(fd);
	struct guild *g = sd->guild;

	if (g == NULL)
		return;

	struct guild_castle *gc = guild->castle_search(p->castle_id);
	if (gc == NULL)
		return;
	if (gc->guild_id != g->guild_id)
		return;
	clif->guild_castleinfo(sd, gc);
#endif
}

static bool clif_lapineDdukDdak_open(struct map_session_data *sd, int item_id)
{
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	nullpo_retr(false, sd);
	nullpo_retr(false, itemdb->exists(item_id));
	struct PACKET_ZC_LAPINEDDUKDDAK_OPEN p;

	p.packetType = HEADER_ZC_LAPINEDDUKDDAK_OPEN;
	p.itemId = item_id;
	clif->send(&p, sizeof(p), &sd->bl, SELF);

	sd->state.lapine_ui = 1;
	return true;
#else
	return false;
#endif // PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
}

static bool clif_lapineDdukDdak_result(struct map_session_data *sd, enum lapineddukddak_result result)
{
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	nullpo_retr(false, sd);
	struct PACKET_ZC_LAPINEDDUKDDAK_RESULT p;

	p.packetType = HEADER_ZC_LAPINEDDUKDDAK_RESULT;
	p.result = result;
	clif->send(&p, sizeof(p), &sd->bl, SELF);
	return true;
#else
	return false;
#endif // PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
}

static void clif_parse_lapineDdukDdak_ack(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_lapineDdukDdak_ack(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20160302
	if (sd->state.lapine_ui == 0)
		return;

	const struct PACKET_CZ_LAPINEDDUKDDAK_ACK *p = RP2PTR(fd);
	struct item_data *it = itemdb->exists(p->itemId);

	if (it == NULL || it->lapineddukddak == NULL)
		return;
	if (pc_cant_act_except_lapine(sd))
		return;
	if (pc->search_inventory(sd, it->nameid) == INDEX_NOT_FOUND)
		return;

	if (((p->packetLength - sizeof(struct PACKET_CZ_LAPINEDDUKDDAK_ACK)) / sizeof(struct PACKET_CZ_LAPINEDDUKDDAK_ACK_sub)) != it->lapineddukddak->NeedCount)
		return;

	for (int i = 0; i < it->lapineddukddak->NeedCount; ++i) {
		int16 idx = p->items[i].index - 2;
		Assert_retv(idx >= 0 && idx < sd->status.inventorySize);

		struct item itr = sd->status.inventory[idx];
		int j = 0;
		for (j = 0; j < VECTOR_LENGTH(it->lapineddukddak->SourceItems); ++j) {
			if (itr.nameid == VECTOR_INDEX(it->lapineddukddak->SourceItems, j).id) {
				// Validate that the amount sent in the packet is matching the database
				if (p->items[i].count != VECTOR_INDEX(it->lapineddukddak->SourceItems, j).amount) {
					clif->lapineDdukDdak_result(sd, LAPINEDDKUKDDAK_INSUFFICIENT_AMOUNT);
					return;
				}

				// Validate that the player have enough of the item
				if (itr.amount < VECTOR_INDEX(it->lapineddukddak->SourceItems, j).amount) {
					clif->lapineDdukDdak_result(sd, LAPINEDDKUKDDAK_INSUFFICIENT_AMOUNT);
					return;
				}

				// Validate refine rate requirement
				if ((itemdb_type(itr.nameid) == IT_ARMOR || itemdb_type(itr.nameid) == IT_WEAPON)
					&& (itr.refine < it->lapineddukddak->NeedRefineMin || itr.refine > it->lapineddukddak->NeedRefineMax))
					return;

				// All requirements are met, move to the next one
				break;
			}
		}
		// The item is not in sources list
		if (j == VECTOR_LENGTH(it->lapineddukddak->SourceItems)) {
			clif->lapineDdukDdak_result(sd, LAPINEDDKUKDDAK_INVALID_ITEM);
			return;
		}
	}

	for (int i = 0; i < it->lapineddukddak->NeedCount; ++i)
		pc->delitem(sd, p->items[i].index - 2, p->items[i].count, 0, DELITEM_NORMAL, LOG_TYPE_SCRIPT);
	if (it->lapineddukddak->script != NULL)
		script->run_item_lapineddukddak_script(sd, it, npc->fake_nd->bl.id);
	clif->lapineDdukDdak_result(sd, LAPINEDDKUKDDAK_SUCCESS);
	return;
#endif // PACKETVER >= 20160302
}

static void clif_parse_lapineDdukDdak_close(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_lapineDdukDdak_close(int fd, struct map_session_data *sd)
{
#if PACKETVER >= 20160504
	sd->state.lapine_ui = 0;
#endif // PACKETVER >= 20160504
}

static bool clif_lapineUpgrade_open(struct map_session_data *sd, int item_id)
{
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	nullpo_retr(false, sd);
	nullpo_retr(false, itemdb->exists(item_id));
	struct PACKET_ZC_LAPINEUPGRADE_OPEN p;

	p.packetType = HEADER_ZC_LAPINEUPGRADE_OPEN;
	p.itemId = item_id;
	clif->send(&p, sizeof(p), &sd->bl, SELF);

	return true;
#else
	return false;
#endif  // PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
}

static void clif_parse_lapineUpgrade_close(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_lapineUpgrade_close(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)
#endif  // PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)
}

static void clif_parse_lapineUpgrade_makeItem(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
static void clif_parse_lapineUpgrade_makeItem(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)
	ShowError("Lapin upgrade not implemented yet.");
	clif->lapineUpgrade_result(sd, LAPINE_UPGRADE_FAILED);
#endif  // PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)
}

static bool clif_lapineUpgrade_result(struct map_session_data *sd, enum lapineUpgrade_result result)
{
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	nullpo_retr(false, sd);
	struct PACKET_ZC_LAPINEUPGRADE_RESULT p;

	p.packetType = HEADER_ZC_LAPINEUPGRADE_RESULT;
	p.result = result;
	clif->send(&p, sizeof(p), &sd->bl, SELF);

	return true;
#else
	return false;
#endif  // PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
}

/*==========================================
 * Main client packet processing function
 *------------------------------------------*/
static int clif_parse(int fd)
{
	int cmd, packet_len;
	struct map_session_data *sd;
	int pnum;

	//TODO apply delays or disconnect based on packet throughput [FlavioJS]
	// Note: "click masters" can do 80+ clicks in 10 seconds

	for( pnum = 0; pnum < 3; ++pnum ) { // Limit max packets per cycle to 3 (delay packet spammers) [FlavioJS]  -- This actually aids packet spammers, but stuff like /str+ gets slow without it [Ai4rei]
		unsigned short (*parse_cmd_func)(int fd, struct map_session_data *sd);
		// begin main client packet processing loop

		sd = sockt->session[fd]->session_data;

		if (sockt->session[fd]->flag.eof) {
			if (sd) {
				if (sd->state.autotrade) {
					//Disassociate character from the socket connection.
					sockt->session[fd]->session_data = NULL;
					sd->fd = 0;
					ShowInfo("Character '"CL_WHITE"%s"CL_RESET"' logged off (using @autotrade).\n", sd->status.name);
				} else
					if (sd->state.active) {
						// Player logout display [Valaris]
						ShowInfo("Character '"CL_WHITE"%s"CL_RESET"' logged off.\n", sd->status.name);
						clif->quitsave(fd, sd);
					} else {
						//Unusual logout (during log on/off/map-changer procedure)
						ShowInfo("Player AID:%d/CID:%d logged off.\n", sd->status.account_id, sd->status.char_id);
						map->quit(sd);
					}
			} else {
				ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", sockt->ip2str(sockt->session[fd]->client_addr, NULL));
			}
			sockt->close(fd);
			return 0;
		}

		if (RFIFOREST(fd) < 2)
			return 0;

		if (sd)
			parse_cmd_func = sd->parse_cmd_func;
		else
			parse_cmd_func = clif->parse_cmd;

		cmd = parse_cmd_func(fd,sd);
		clif->cmd = cmd;

		if (VECTOR_LENGTH(HPM->packets[hpClif_Parse]) > 0) {
			int result = HPM->parse_packets(fd,cmd,hpClif_Parse);
			if (result == 1)
				continue;
			if (result == 2)
				return 0;
		}

		// filter out invalid / unsupported packets
		if (cmd > MAX_PACKET_DB || cmd < MIN_PACKET_DB || packets->db[cmd] == 0) {
			ShowWarning("clif_parse: Received unsupported packet (packet 0x%04x (0x%04x), %"PRIuS" bytes received), disconnecting session #%d.\n",
			            (unsigned int)cmd, RFIFOW(fd,0), RFIFOREST(fd), fd);
#ifdef DUMP_INVALID_PACKET
			ShowDump(RFIFOP(fd,0), RFIFOREST(fd));
#endif
			sockt->eof(fd);
			return 0;
		}

		// determine real packet length
		if ((packet_len = packets->db[cmd]) == -1) { // variable-length packet

			if (RFIFOREST(fd) < 4)
				return 0;

			packet_len = RFIFOW(fd,2);
			if (packet_len < 4 || packet_len > 32768) {
				ShowWarning("clif_parse: Received packet 0x%04x specifies invalid packet_len (%d), disconnecting session #%d.\n", (unsigned int)cmd, packet_len, fd);
#ifdef DUMP_INVALID_PACKET
				ShowDump(RFIFOP(fd,0), RFIFOREST(fd));
#endif
				sockt->eof(fd);

				return 0;
			}
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0; // not enough data received to form the packet

		if( battle_config.packet_obfuscation == 2 || cmd != RFIFOW(fd, 0) || (sd && sd->parse_cmd_func == clif_parse_cmd_decrypt) ) {
			// Note: Overriding const qualifier to re-inject the decoded packet ID.
#define RFIFOP_mutable(fd, pos) ((void *)(sockt->session[fd]->rdata + sockt->session[fd]->rdata_pos + (pos)))
			int16 *packet_id = RFIFOP_mutable(fd, 0);
#undef RFIFOP_mutable
			*packet_id = cmd;
			if( sd ) {
				sd->cryptKey = (( sd->cryptKey * clif->cryptKey[1] ) + clif->cryptKey[2]) & 0xFFFFFFFF; // Update key for the next packet
			}
		}

		if( packet_db[cmd].func == clif->pDebug )
			packet_db[cmd].func(fd, sd);
		else if( packet_db[cmd].func != NULL ) {
			if( !sd && packet_db[cmd].func != clif->pWantToConnection )
				; //Only valid packet when there is no session
			else
				if( sd && sd->bl.prev == NULL && packet_db[cmd].func != clif->pLoadEndAck )
					; //Only valid packet when player is not on a map
				else
					packet_db[cmd].func(fd, sd);
		}
		else {
#ifdef DUMP_UNKNOWN_PACKET
			const char* packet_txt = "save/packet.txt";
			FILE* fp;

			if( ( fp = fopen( packet_txt , "a" ) ) != NULL ) {
				if( sd ) {
					fprintf(fp, "Unknown packet 0x%04X (length %d), %s session #%d, %d/%d (AID/CID)\n", cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id);
				} else {
					fprintf(fp, "Unknown packet 0x%04X (length %d), session #%d\n", cmd, packet_len, fd);
				}

				WriteDump(fp, RFIFOP(fd,0), packet_len);
				fprintf(fp, "\n");
				fclose(fp);
			} else {
				ShowError("Failed to write '%s'.\n", packet_txt);

				// Dump on console instead
				if( sd ) {
					ShowDebug("Unknown packet 0x%04X (length %d), %s session #%d, %d/%d (AID/CID)\n", cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id);
				} else {
					ShowDebug("Unknown packet 0x%04X (length %d), session #%d\n", cmd, packet_len, fd);
				}

				ShowDump(RFIFOP(fd,0), packet_len);
			}
#else
			clif->pDull(fd, sd);
#endif
		}

		RFIFOSKIP(fd, packet_len);

	}; // main loop end

	return 0;
}

/**
 * Returns information about the given packet ID.
 *
 * @param packet_id The packet ID.
 * @return The corresponding packet_db entry, if any.
 */
static const struct s_packet_db *clif_packet(int packet_id)
{
	if (packet_id < MIN_PACKET_DB || packet_id > MAX_PACKET_DB || packets->db[packet_id] == 0)
		return NULL;
	return &packet_db[packet_id];
}

static void __attribute__ ((unused)) packetdb_addpacket(int cmd, ...)
{
	va_list va;
	int i;
	int pos;
	pFunc func;

	if (cmd > MAX_PACKET_DB) {
		ShowError("Packet Error: packet 0x%x is greater than the maximum allowed (0x%x), skipping...\n", (unsigned int)cmd, (unsigned int)MAX_PACKET_DB);
		return;
	}

	if (cmd < MIN_PACKET_DB) {
		ShowError("Packet Error: packet 0x%x is lower than the minimum allowed (0x%x), skipping...\n", (unsigned int)cmd, (unsigned int)MIN_PACKET_DB);
		return;
	}

	va_start(va, cmd);

	pos = va_arg(va, int);

	va_end(va);

	if (pos == 0xFFFF) { /* nothing more to do */
		return;
	}

	va_start(va, cmd);

	func = va_arg(va, pFunc);

	packet_db[cmd].func = func;

	for (i = 0; i < MAX_PACKET_POS; i++) {
		pos = va_arg(va, int);

		if (pos == 0xFFFF)
			break;

		packet_db[cmd].pos[i] = pos;
	}
	va_end(va);
}

static void packetdb_loaddb(void)
{
	memset(packet_db,0,sizeof(packet_db));

#define packet(id, ...) packetdb_addpacket((id), ##__VA_ARGS__, 0xFFFF)
#include "map/packets.h" /* load structure data */
#ifdef PACKETVER_ZERO
#include "map/packets_shuffle_zero.h"
#elif defined(PACKETVER_RE)
#include "map/packets_shuffle_re.h"
#else  // PACKETVER_ZERO
#include "map/packets_shuffle_main.h"
#endif  // PACKETVER_ZERO
#undef packet
#define packetKeys(a,b,c) do { clif->cryptKey[0] = (a); clif->cryptKey[1] = (b); clif->cryptKey[2] = (c); } while(0)
#if defined(OBFUSCATIONKEY1) && defined(OBFUSCATIONKEY2) && defined(OBFUSCATIONKEY3)
	packetKeys(OBFUSCATIONKEY1,OBFUSCATIONKEY2,OBFUSCATIONKEY3);
#else  // defined(OBFUSCATIONKEY1) && defined(OBFUSCATIONKEY2) && defined(OBFUSCATIONKEY3)
#ifdef PACKETVER_ZERO
#include "map/packets_keys_zero.h"
#else  // PACKETVER_ZERO
#include "map/packets_keys_main.h"
#endif  // PACKETVER_ZERO
#endif  // defined(OBFUSCATIONKEY1) && defined(OBFUSCATIONKEY2) && defined(OBFUSCATIONKEY3)
#undef packetKeys
}

static void clif_bc_ready(void)
{
	if( battle_config.display_status_timers )
		clif->status_change_sub = clif_status_change_sub;
	else
		clif->status_change_sub = clif_status_change_notick;

	switch( battle_config.packet_obfuscation ) {
		case 0:
			clif->parse_cmd = clif_parse_cmd_normal;
			break;
		default:
		case 1:
			clif->parse_cmd = clif_parse_cmd_optional;
			break;
		case 2:
			clif->parse_cmd = clif_parse_cmd_decrypt;
			break;
	}
}
/*==========================================
 *
 *------------------------------------------*/
static int do_init_clif(bool minimal)
{
	if (minimal)
		return 0;

	packetdb_loaddb();

	sockt->set_defaultparse(clif->parse);
	sockt->validate = true;
	if (sockt->make_listen_bind(clif->bind_ip,clif->map_port) == -1) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n",clif->map_port);
		exit(EXIT_FAILURE);
	}

	timer->add_func_list(clif->clearunit_delayed_sub, "clif_clearunit_delayed_sub");
	timer->add_func_list(clif->delayquit, "clif_delayquit");

	clif->delay_clearunit_ers = ers_new(sizeof(struct mob_data), "clif.c::delay_clearunit_ers", ERS_OPT_CLEAR);
	clif->delayed_damage_ers = ers_new(sizeof(struct cdelayed_damage),"clif.c::delayed_damage_ers",ERS_OPT_CLEAR);

#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320
	timer->add_func_list(clif->pingTimer, "clif_pingTimer");
	if (battle_config.ping_timer_interval != 0)
		timer->add_interval(timer->gettick() + battle_config.ping_timer_interval * 1000, clif->pingTimer, 0, 0, battle_config.ping_timer_interval * 1000);
#endif

	return 0;
}

static void do_final_clif(void)
{
	unsigned char i;

	ers_destroy(clif->delay_clearunit_ers);
	ers_destroy(clif->delayed_damage_ers);

	for(i = 0; i < CASHSHOP_TAB_MAX; i++) {
		int k;
		for( k = 0; k < clif->cs.item_count[i]; k++ ) {
			aFree(clif->cs.data[i][k]);
		}
		aFree(clif->cs.data[i]);
	}

	for(i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		if( clif->rd.nameid[i] )
			aFree(clif->rd.nameid[i]);
		if( clif->rd.qty[i] )
			aFree(clif->rd.qty[i]);
	}

}

void clif_defaults(void)
{
	clif = &clif_s;
	/* vars */
	clif->bind_ip = INADDR_ANY;
	clif->map_port = 5121;
	clif->ally_only = false;
	clif->delayed_damage_ers = NULL;
	clif->cmd = -1;
	/* core */
	clif->init = do_init_clif;
	clif->final = do_final_clif;
	clif->setip = clif_setip;
	clif->setbindip = clif_setbindip;
	clif->setport = clif_setport;
	clif->refresh_ip = clif_refresh_ip;
	clif->send = clif_send;
	clif->send_sub = clif_send_sub;
	clif->send_actual = clif_send_actual;
	clif->parse = clif_parse;
	clif->parse_cmd = clif_parse_cmd_optional;
	clif->decrypt_cmd = clif_decrypt_cmd;
	clif->packet = clif_packet;
	/* auth */
	clif->authok = clif_authok;
	clif->auth_error = clif_auth_error;
	clif->authrefuse = clif_authrefuse;
	clif->authfail_fd = clif_authfail_fd;
	clif->charselectok = clif_charselectok;
	/* item-related */
	clif->dropflooritem = clif_dropflooritem;
	clif->clearflooritem = clif_clearflooritem;
	clif->additem = clif_additem;
	clif->dropitem = clif_dropitem;
	clif->delitem = clif_delitem;
	clif->takeitem = clif_takeitem;
	clif->item_movefailed = clif_item_movefailed;
	clif->item_equip = clif_item_equip;
	clif->item_normal = clif_item_normal;
	clif->arrowequip = clif_arrowequip;
	clif->arrow_fail = clif_arrow_fail;
	clif->use_card = clif_use_card;
	clif->cart_additem = clif_cart_additem;
	clif->cart_delitem = clif_cart_delitem;
	clif->equipitemack = clif_equipitemack;
	clif->unequipitemack = clif_unequipitemack;
	clif->useitemack = clif_useitemack;
	clif->addcards = clif_addcards;
	clif->item_sub = clif_item_sub;  // look like unused
	clif->getareachar_item = clif_getareachar_item;
	clif->cart_additem_ack = clif_cart_additem_ack;
	clif->cashshop_load = clif_cashshop_db;
	clif->cashShopSchedule = clif_cashShopSchedule;
	clif->package_announce = clif_package_item_announce;
	clif->item_drop_announce = clif_item_drop_announce;
	/* unit-related */
	clif->clearunit_single = clif_clearunit_single;
	clif->clearunit_area = clif_clearunit_area;
	clif->clearunit_delayed = clif_clearunit_delayed;
	clif->walkok = clif_walkok;
	clif->move = clif_move;
	clif->move2 = clif_move2;
	clif->blown = clif_blown;
	clif->slide = clif_slide;
	clif->fixpos = clif_fixpos;
	clif->changelook = clif_changelook;
	clif->changetraplook = clif_changetraplook;
	clif->refreshlook = clif_refreshlook;
	clif->sendlook = clif_sendlook;
	clif->class_change = clif_class_change;
	clif->skill_delunit = clif_skill_delunit;
	clif->skillunit_update = clif_skillunit_update;
	clif->clearunit_delayed_sub = clif_clearunit_delayed_sub;
	clif->set_unit_idle = clif_set_unit_idle;
	clif->spawn_unit = clif_spawn_unit;
	clif->spawn_unit2 = clif_spawn_unit2;
	clif->set_unit_idle2 = clif_set_unit_idle2;
	clif->set_unit_walking = clif_set_unit_walking;
	clif->calc_walkdelay = clif_calc_walkdelay;
	clif->getareachar_skillunit = clif_getareachar_skillunit;
	clif->getareachar_unit = clif_getareachar_unit;
	clif->clearchar_skillunit = clif_clearchar_skillunit;
	clif->getareachar = clif_getareachar;
	clif->graffiti_entry = clif_graffiti_entry;
	/* main unit spawn */
	clif->spawn = clif_spawn;
	/* map-related */
	clif->changemap = clif_changemap;
	clif->changemap_airship = clif_changemap_airship;
	clif->changemapcell = clif_changemapcell;
	clif->map_property = clif_map_property;
	clif->pvpset = clif_pvpset;
	clif->map_property_mapall = clif_map_property_mapall;
	clif->bossmapinfo = clif_bossmapinfo;
	clif->map_type = clif_map_type;
	clif->maptypeproperty2 = clif_maptypeproperty2;
	/* multi-map-server */
	clif->changemapserver = clif_changemapserver;
	clif->changemapserver_airship = clif_changemapserver_airship;
	/* npc-shop-related */
	clif->npcbuysell = clif_npcbuysell;
	clif->buylist = clif_buylist;
	clif->selllist = clif_selllist;
	clif->cashshop_show = clif_cashshop_show;
	clif->npc_buy_result = clif_npc_buy_result;
	clif->npc_sell_result = clif_npc_sell_result;
	clif->cashshop_ack = clif_cashshop_ack;
	/* npc-script-related */
	clif->scriptmes = clif_scriptmes;
	clif->scriptnext = clif_scriptnext;
	clif->scriptclose = clif_scriptclose;
	clif->scriptmenu = clif_scriptmenu;
	clif->scriptinput = clif_scriptinput;
	clif->scriptinputstr = clif_scriptinputstr;
	clif->cutin = clif_cutin;
	clif->sendfakenpc = clif_sendfakenpc;
	clif->scriptclear = clif_scriptclear;
	/* client-user-interface-related */
	clif->viewpoint = clif_viewpoint;
	clif->damage = clif_damage;
	clif->sitting = clif_sitting;
	clif->standing = clif_standing;
	clif->arrow_create_list = clif_arrow_create_list;
	clif->refresh_storagewindow = clif_refresh_storagewindow;
	clif->refresh = clif_refresh;
	clif->fame_blacksmith = clif_fame_blacksmith;
	clif->fame_alchemist = clif_fame_alchemist;
	clif->fame_taekwon = clif_fame_taekwon;
	clif->ranklist = clif_ranklist;
	clif->ranklist_sub = clif_ranklist_sub;
	clif->ranklist_sub2 = clif_ranklist_sub2;
	clif->pRanklist = clif_parse_ranklist;
	clif->update_rankingpoint = clif_update_rankingpoint;
	clif->hotkeys = clif_hotkeys_send;
	clif->hotkeysAll = clif_hotkeysAll_send;
	clif->insight = clif_insight;
	clif->outsight = clif_outsight;
	clif->skillcastcancel = clif_skillcastcancel;
	clif->skill_fail = clif_skill_fail;
	clif->skill_cooldown = clif_skill_cooldown;
	clif->skill_memomessage = clif_skill_memomessage;
	clif->skill_mapinfomessage = clif_skill_mapinfomessage;
	clif->skill_produce_mix_list = clif_skill_produce_mix_list;
	clif->cooking_list = clif_cooking_list;
	clif->autospell = clif_autospell;
	clif->combo_delay = clif_combo_delay;
	clif->status_change = clif_status_change;
	clif->status_change_sub = clif_status_change_sub;
	clif->insert_card = clif_insert_card;
	clif->inventoryList = clif_inventoryList;
	clif->inventoryItems = clif_inventoryItems;
	clif->equipList = clif_equipList;
	clif->equipItems = clif_equipItems;
	clif->cartList = clif_cartList;
	clif->cartItems = clif_cartItems;
	clif->inventoryExpansionInfo = clif_inventoryExpansionInfo;
	clif->inventoryExpandAck = clif_inventoryExpandAck;
	clif->inventoryExpandResult = clif_inventoryExpandResult;
	clif->pInventoryExpansion = clif_parse_inventoryExpansion;
	clif->pInventoryExpansionConfirmed = clif_parse_inventoryExpansionConfirmed;
	clif->pInventoryExpansionRejected = clif_parse_inventoryExpansionRejected;
	clif->favorite_item = clif_favorite_item;
	clif->clearcart = clif_clearcart;
	clif->item_identify_list = clif_item_identify_list;
	clif->item_identified = clif_item_identified;
	clif->item_repair_list = clif_item_repair_list;
	clif->item_repaireffect = clif_item_repaireffect;
	clif->item_damaged = clif_item_damaged;
	clif->item_refine_list = clif_item_refine_list;
	clif->item_skill = clif_item_skill;
	clif->mvp_item = clif_mvp_item;
	clif->mvp_exp = clif_mvp_exp;
	clif->mvp_noitem = clif_mvp_noitem;
	clif->changed_dir = clif_changed_dir;
	clif->blname_ack = clif_blname_ack;
	clif->pcname_ack = clif_pcname_ack;
	clif->homname_ack = clif_homname_ack;
	clif->mername_ack = clif_mername_ack;
	clif->petname_ack = clif_petname_ack;
	clif->npcname_ack = clif_npcname_ack;
	clif->mobname_ack = clif_mobname_ack;
	clif->mobname_guardian_ack = clif_mobname_guardian_ack;
	clif->mobname_additional_ack = clif_mobname_additional_ack;
	clif->mobname_normal_ack = clif_mobname_normal_ack;
	clif->chatname_ack = clif_chatname_ack;
	clif->elemname_ack = clif_elemname_ack;
	clif->skillname_ack = clif_skillname_ack;
	clif->itemname_ack = clif_itemname_ack;
	clif->unknownname_ack = clif_unknownname_ack;
	clif->monster_hp_bar = clif_monster_hp_bar;
	clif->show_monster_hp_bar = clif_show_monster_hp_bar;
	clif->hpmeter = clif_hpmeter;
	clif->hpmeter_single = clif_hpmeter_single;
	clif->hpmeter_sub = clif_hpmeter_sub;
	clif->upgrademessage = clif_upgrademessage;
	clif->get_weapon_view = clif_get_weapon_view;
	clif->gospel_info = clif_gospel_info;
	clif->feel_req = clif_feel_req;
	clif->starskill = clif_starskill;
	clif->feel_info = clif_feel_info;
	clif->hate_info = clif_hate_info;
	clif->mission_info = clif_mission_info;
	clif->feel_hate_reset = clif_feel_hate_reset;
	clif->partytickack = clif_partytickack;
	clif->zc_config = clif_zc_config;
	clif->viewequip_ack = clif_viewequip_ack;
	clif->equpcheckbox = clif_equpcheckbox;
	clif->displayexp = clif_displayexp;
	clif->font = clif_font;
	clif->progressbar = clif_progressbar;
	clif->progressbar_abort = clif_progressbar_abort;
	clif->progressbar_unit = clif_progressbar_unit;
	clif->showdigit = clif_showdigit;
	clif->elementalconverter_list = clif_elementalconverter_list;
	clif->spellbook_list = clif_spellbook_list;
	clif->magicdecoy_list = clif_magicdecoy_list;
	clif->poison_list = clif_poison_list;
	clif->autoshadowspell_list = clif_autoshadowspell_list;
	clif->skill_itemlistwindow = clif_skill_itemlistwindow;
	clif->sc_load = clif_status_change2;
	clif->sc_continue = clif_status_change2;
	clif->sc_end = clif_status_change_end;
	clif->initialstatus = clif_initialstatus;
	clif->cooldown_list = clif_skill_cooldown_list;
	/* player-unit-specific-related */
	clif->updatestatus = clif_updatestatus;
	clif->changestatus = clif_changestatus;
	clif->statusupack = clif_statusupack;
	clif->movetoattack = clif_movetoattack;
	clif->solved_charname = clif_solved_charname;
	clif->charnameupdate = clif_charnameupdate;
	clif->delayquit = clif_delayquit;
	clif->getareachar_pc = clif_getareachar_pc;
	clif->disconnect_ack = clif_disconnect_ack;
	clif->PVPInfo = clif_PVPInfo;
	clif->blacksmith = clif_blacksmith;
	clif->alchemist = clif_alchemist;
	clif->taekwon = clif_taekwon;
	clif->ranking_pk = clif_ranking_pk;
	clif->quitsave = clif_quitsave;
	/* visual effects client-side */
	clif->misceffect = clif_misceffect;
	clif->changeoption = clif_changeoption;
	clif->changeoption_target = clif_changeoption_target;
	clif->changeoption2 = clif_changeoption2;
	clif->emotion = clif_emotion;
	clif->talkiebox = clif_talkiebox;
	clif->wedding_effect = clif_wedding_effect;
	clif->divorced = clif_divorced;
	clif->callpartner = clif_callpartner;
	clif->skill_damage = clif_skill_damage;
	clif->skill_nodamage = clif_skill_nodamage;
	clif->skill_poseffect = clif_skill_poseffect;
	clif->skill_estimation = clif_skill_estimation;
	clif->skill_warppoint = clif_skill_warppoint;
	clif->useskill = clif_useskill;
	clif->produce_effect = clif_produceeffect;
	clif->devotion = clif_devotion;
	clif->spiritball = clif_spiritball;
	clif->spiritball_single = clif_spiritball_single;
	clif->bladestop = clif_bladestop;
	clif->mvp_effect = clif_mvp_effect;
	clif->heal = clif_heal;
	clif->resurrection = clif_resurrection;
	clif->refine = clif_refine;
	clif->weather = clif_weather;
	clif->specialeffect = clif_specialeffect;
	clif->specialeffect_single = clif_specialeffect_single;
	clif->specialeffect_value = clif_specialeffect_value;
	clif->specialeffect_value_single = clif_specialeffect_value_single;
	clif->removeSpecialEffect = clif_removeSpecialEffect;
	clif->removeSpecialEffect_single = clif_removeSpecialEffect_single;
	clif->millenniumshield = clif_millenniumshield;
	clif->spiritcharm = clif_charm;
	clif->charm_single = clif_charm_single;
	clif->snap = clif_snap;
	clif->weather_check = clif_weather_check;
	/* sound effects client-side */
	clif->playBGM = clif_playBGM;
	clif->soundeffect = clif_soundeffect;
	clif->soundeffectall = clif_soundeffectall;
	/* chat/message-related */
	clif->GlobalMessage = clif_GlobalMessage;
	clif->createchat = clif_createchat;
	clif->dispchat = clif_dispchat;
	clif->joinchatfail = clif_joinchatfail;
	clif->joinchatok = clif_joinchatok;
	clif->addchat = clif_addchat;
	clif->changechatowner = clif_changechatowner;
	clif->chatRoleChange = clif_chatRoleChange;
	clif->clearchat = clif_clearchat;
	clif->leavechat = clif_leavechat;
	clif->changechatstatus = clif_changechatstatus;
	clif->wis_message = clif_wis_message;
	clif->wis_end = clif_wis_end;
	clif->disp_message = clif_disp_message;
	clif->broadcast = clif_broadcast;
	clif->broadcast2 = clif_broadcast2;
	clif->messagecolor_self = clif_messagecolor_self;
	clif->messagecolor = clif_messagecolor;
	clif->serviceMessageColor = clif_serviceMessageColor;
	clif->disp_overhead = clif_disp_overhead;
	clif->notify_playerchat = clif_notify_playerchat;
	clif->msgtable_skill = clif_msgtable_skill;
	clif->msgtable = clif_msgtable;
	clif->msgtable_num = clif_msgtable_num;
	clif->msgtable_str = clif_msgtable_str;
	clif->msgtable_str_color = clif_msgtable_str_color;
	clif->msgtable_color = clif_msgtable_color;
	clif->message = clif_displaymessage;
	clif->messageln = clif_displaymessage2;
	clif->messages = clif_displaymessage_sprintf;
	clif->process_chat_message = clif_process_chat_message;
	clif->process_whisper_message = clif_process_whisper_message;
	clif->wisexin = clif_wisexin;
	clif->wisall = clif_wisall;
	clif->PMIgnoreList = clif_PMIgnoreList;
	clif->ShowScript = clif_ShowScript;
	/* trade handling */
	clif->traderequest = clif_traderequest;
	clif->tradestart = clif_tradestart;
	clif->tradeadditem = clif_tradeadditem;
	clif->tradeitemok = clif_tradeitemok;
	clif->tradedeal_lock = clif_tradedeal_lock;
	clif->tradecancelled = clif_tradecancelled;
	clif->tradecompleted = clif_tradecompleted;
	clif->tradeundo = clif_tradeundo;  // unused
	/* vending handling */
	clif->openvendingreq = clif_openvendingreq;
	clif->showvendingboard = clif_showvendingboard;
	clif->closevendingboard = clif_closevendingboard;
	clif->vendinglist = clif_vendinglist;
	clif->buyvending = clif_buyvending;
	clif->openvending = clif_openvending;
	clif->openvendingAck = clif_openvendingAck;
	clif->vendingreport = clif_vendingreport;
	/* storage handling */
	clif->storageList = clif_storageList;
	clif->guildStorageList = clif_guildStorageList;
	clif->storageItems = clif_storageItems;
	clif->inventoryStart = clif_inventoryStart;
	clif->inventoryEnd = clif_inventoryEnd;
	clif->updatestorageamount = clif_updatestorageamount;
	clif->storageitemadded = clif_storageitemadded;
	clif->storageitemremoved = clif_storageitemremoved;
	clif->storageclose = clif_storageclose;
	/* skill-list handling */
	clif->skillinfoblock = clif_skillinfoblock;
	clif->skillup = clif_skillup;
	clif->skillinfo = clif_skillinfo;
	clif->addskill = clif_addskill;
	clif->deleteskill = clif_deleteskill;
	clif->playerSkillToPacket = clif_playerSkillToPacket;
	/* party-specific */
	clif->party_created = clif_party_created;
	clif->party_member_info = clif_party_member_info;
	clif->party_info = clif_party_info;
	clif->party_job_and_level = clif_party_job_and_level;
	clif->party_invite = clif_party_invite;
	clif->party_inviteack = clif_party_inviteack;
	clif->party_option = clif_party_option;
	clif->party_withdraw = clif_party_withdraw;
	clif->party_message = clif_party_message;
	clif->party_xy = clif_party_xy;
	clif->party_xy_single = clif_party_xy_single;
	clif->party_hp = clif_party_hp;
	clif->party_xy_remove = clif_party_xy_remove;
	clif->party_show_picker = clif_party_show_picker;
	clif->partyinvitationstate = clif_partyinvitationstate;
	clif->PartyLeaderChanged = clif_PartyLeaderChanged;
	/* guild-specific */
	clif->guild_created = clif_guild_created;
	clif->guild_belonginfo = clif_guild_belonginfo;
	clif->guild_masterormember = clif_guild_masterormember;
	clif->guild_basicinfo = clif_guild_basicinfo;
	clif->guild_allianceinfo = clif_guild_allianceinfo;
	clif->guild_castlelist = clif_guild_castlelist;
	clif->guild_castleinfo = clif_guild_castleinfo;
	clif->guild_memberlist = clif_guild_memberlist;
	clif->guild_skillinfo = clif_guild_skillinfo;
	clif->guild_send_onlineinfo = clif_guild_send_onlineinfo;
	clif->guild_memberlogin_notice = clif_guild_memberlogin_notice;
	clif->guild_invite = clif_guild_invite;
	clif->guild_inviteack = clif_guild_inviteack;
	clif->guild_leave = clif_guild_leave;
	clif->guild_expulsion = clif_guild_expulsion;
	clif->guild_positionchanged = clif_guild_positionchanged;
	clif->guild_memberpositionchanged = clif_guild_memberpositionchanged;
	clif->guild_emblem = clif_guild_emblem;
	clif->guild_emblem_area = clif_guild_emblem_area;
	clif->guild_notice = clif_guild_notice;
	clif->guild_message = clif_guild_message;
	clif->guild_reqalliance = clif_guild_reqalliance;
	clif->guild_allianceack = clif_guild_allianceack;
	clif->guild_delalliance = clif_guild_delalliance;
	clif->guild_oppositionack = clif_guild_oppositionack;
	clif->guild_broken = clif_guild_broken;
	clif->guild_xy = clif_guild_xy;
	clif->guild_xy_single = clif_guild_xy_single;
	clif->guild_xy_remove = clif_guild_xy_remove;
	clif->guild_positionnamelist = clif_guild_positionnamelist;
	clif->guild_positioninfolist = clif_guild_positioninfolist;
	clif->guild_expulsionlist = clif_guild_expulsionlist;
	clif->guild_set_position = clif_guild_set_position;
	clif->guild_position_selected = clif_guild_position_selected;
	clif->validate_emblem = clif_validate_emblem;
	/* battleground-specific */
	clif->bg_hp = clif_bg_hp;
	clif->bg_xy = clif_bg_xy;
	clif->bg_xy_remove = clif_bg_xy_remove;
	clif->bg_message = clif_bg_message;
	clif->bg_updatescore = clif_bg_updatescore;
	clif->bg_updatescore_single = clif_bg_updatescore_single;
	clif->sendbgemblem_area = clif_sendbgemblem_area;
	clif->sendbgemblem_single = clif_sendbgemblem_single;
	/* instance-related */
	clif->instance = clif_instance;
	clif->instance_join = clif_instance_join;
	clif->instance_leave = clif_instance_leave;
	/* pet-related */
	clif->catch_process = clif_catch_process;
	clif->pet_roulette = clif_pet_roulette;
	clif->sendegg = clif_sendegg;
	clif->send_petstatus = clif_send_petstatus;
	clif->send_petdata = clif_send_petdata;
	clif->pet_emotion = clif_pet_emotion;
	clif->pet_food = clif_pet_food;
	/* friend-related */
	clif->friendslist_toggle_sub = clif_friendslist_toggle_sub;
	clif->friendslist_send = clif_friendslist_send;
	clif->friendslist_reqack = clif_friendslist_reqack;
	clif->friendslist_toggle = clif_friendslist_toggle;
	clif->friendlist_req = clif_friendlist_req;
	/* gm-related */
	clif->GM_kickack = clif_GM_kickack;
	clif->GM_kick = clif_GM_kick;
	clif->manner_message = clif_manner_message;
	clif->GM_silence = clif_GM_silence;
	clif->account_name = clif_account_name;
	clif->check = clif_check;
	/* hom-related */
	clif->hominfo = clif_hominfo;
	clif->homskillinfoblock = clif_homskillinfoblock;
	clif->homskillup = clif_homskillup;
	clif->hom_food = clif_hom_food;
	clif->send_homdata = clif_send_homdata;
	/* questlog-related */
	clif->quest_send_list = clif_quest_send_list;
	clif->quest_send_mission = clif_quest_send_mission;
	clif->quest_add = clif_quest_add;
	clif->quest_delete = clif_quest_delete;
	clif->quest_update_status = clif_quest_update_status;
	clif->quest_update_objective = clif_quest_update_objective;
	clif->quest_notify_objective = clif_quest_notify_objective;
	clif->quest_show_event = clif_quest_show_event;
	/* mail-related */
	clif->mail_window = clif_Mail_window;
	clif->mail_read = clif_Mail_read;
	clif->mail_delete = clif_Mail_delete;
	clif->mail_return = clif_Mail_return;
	clif->mail_send = clif_Mail_send;
	clif->mail_new = clif_Mail_new;
	clif->mail_refreshinbox = clif_Mail_refreshinbox;
	clif->mail_getattachment = clif_Mail_getattachment;
	clif->mail_setattachment = clif_Mail_setattachment;
	/* auction-related */
	clif->auction_openwindow = clif_Auction_openwindow;
	clif->auction_results = clif_Auction_results;
	clif->auction_message = clif_Auction_message;
	clif->auction_close = clif_Auction_close;
	clif->auction_setitem = clif_Auction_setitem;
	/* mercenary-related */
	clif->mercenary_info = clif_mercenary_info;
	clif->mercenary_skillblock = clif_mercenary_skillblock;
	clif->mercenary_message = clif_mercenary_message;
	clif->mercenary_updatestatus = clif_mercenary_updatestatus;
	/* item rental */
	clif->rental_time = clif_rental_time;
	clif->rental_expired = clif_rental_expired;
	/* party booking related */
	clif->PartyBookingRegisterAck = clif_PartyBookingRegisterAck;
	clif->PartyBookingDeleteAck = clif_PartyBookingDeleteAck;
	clif->PartyBookingSearchAck = clif_PartyBookingSearchAck;
	clif->PartyBookingUpdateNotify = clif_PartyBookingUpdateNotify;
	clif->PartyBookingDeleteNotify = clif_PartyBookingDeleteNotify;
	clif->PartyBookingInsertNotify = clif_PartyBookingInsertNotify;
	clif->PartyRecruitRegisterAck = clif_PartyRecruitRegisterAck;
	clif->PartyRecruitDeleteAck = clif_PartyRecruitDeleteAck;
	clif->PartyRecruitSearchAck = clif_PartyRecruitSearchAck;
	clif->PartyRecruitUpdateNotify = clif_PartyRecruitUpdateNotify;
	clif->PartyRecruitDeleteNotify = clif_PartyRecruitDeleteNotify;
	clif->PartyRecruitInsertNotify = clif_PartyRecruitInsertNotify;
	/* Group Search System Update */
	clif->PartyBookingVolunteerInfo = clif_PartyBookingVolunteerInfo;
	clif->PartyBookingRefuseVolunteer = clif_PartyBookingRefuseVolunteer;
	clif->PartyBookingCancelVolunteer = clif_PartyBookingCancelVolunteer;
	clif->PartyBookingAddFilteringList = clif_PartyBookingAddFilteringList;
	clif->PartyBookingSubFilteringList = clif_PartyBookingSubFilteringList;
	/* buying store-related */
	clif->buyingstore_open = clif_buyingstore_open;
	clif->buyingstore_open_failed = clif_buyingstore_open_failed;
	clif->buyingstore_myitemlist = clif_buyingstore_myitemlist;
	clif->buyingstore_entry = clif_buyingstore_entry;
	clif->buyingstore_entry_single = clif_buyingstore_entry_single;
	clif->buyingstore_disappear_entry = clif_buyingstore_disappear_entry;
	clif->buyingstore_disappear_entry_single = clif_buyingstore_disappear_entry_single;
	clif->buyingstore_itemlist = clif_buyingstore_itemlist;
	clif->buyingstore_trade_failed_buyer = clif_buyingstore_trade_failed_buyer;
	clif->buyingstore_update_item = clif_buyingstore_update_item;
	clif->buyingstore_delete_item = clif_buyingstore_delete_item;
	clif->buyingstore_trade_failed_seller = clif_buyingstore_trade_failed_seller;
	/* search store-related */
	clif->search_store_info_ack = clif_search_store_info_ack;
	clif->search_store_info_failed = clif_search_store_info_failed;
	clif->open_search_store_info = clif_open_search_store_info;
	clif->search_store_info_click_ack = clif_search_store_info_click_ack;
	/* elemental-related */
	clif->elemental_info = clif_elemental_info;
	clif->elemental_updatestatus = clif_elemental_updatestatus;
	/* bgqueue */
	clif->bgqueue_ack = clif_bgqueue_ack;
	clif->bgqueue_notice_delete = clif_bgqueue_notice_delete;
	clif->bgqueue_update_info = clif_bgqueue_update_info;
	clif->bgqueue_joined = clif_bgqueue_joined;
	clif->bgqueue_pcleft = clif_bgqueue_pcleft;
	clif->bgqueue_battlebegins = clif_bgqueue_battlebegins;
	/* misc-handling */
	clif->adopt_reply = clif_Adopt_reply;
	clif->adopt_request = clif_Adopt_request;
	clif->readbook = clif_readbook;
	clif->notify_time = clif_notify_time;
	clif->user_count = clif_user_count;
	clif->noask_sub = clif_noask_sub;
	clif->bc_ready = clif_bc_ready;
	/* Hercules Channel System */
	clif->channel_msg = clif_channel_msg;
	clif->channel_msg2 = clif_channel_msg2;
	/* */
	clif->undisguise_timer = clif_undisguise_timer;
	/* Bank System [Yommy/Hercules] */
	clif->bank_deposit = clif_bank_deposit;
	clif->bank_withdraw = clif_bank_withdraw;
	/* */
	clif->show_modifiers = clif_show_modifiers;
	/* */
	clif->notify_bounditem = clif_notify_bounditem;
	/* */
	clif->delay_damage = clif_delay_damage;
	clif->delay_damage_sub = clif_delay_damage_sub;
	/* NPC Market */
	clif->npc_market_open = clif_npc_market_open;
	clif->npc_market_purchase_ack = clif_npc_market_purchase_ack;
	/* */
	clif->parse_roulette_db = clif_parse_roulette_db;
	clif->roulette_generate_ack = clif_roulette_generate_ack;
	clif->roulette_close = clif_roulette_close;
	/* Merge Items */
	clif->openmergeitem = clif_openmergeitem;
	clif->cancelmergeitem = clif_cancelmergeitem;
	clif->comparemergeitem = clif_comparemergeitem;
	clif->ackmergeitems = clif_ackmergeitems;
	clif->mergeitems = clif_mergeitems;
	/* Cart Deco */
	clif->selectcart = clif_selectcart;
	/* */
	clif->isdisguised = clif_isdisguised;
	clif->navigate_to = clif_navigate_to;
	clif->bl_type = clif_bl_type;
	/* Achievement System */
	clif->achievement_send_list = clif_achievement_send_list;
	clif->achievement_send_update = clif_achievement_send_update;
	clif->pAchievementGetReward = clif_parse_achievement_get_reward;
	clif->achievement_reward_ack = clif_achievement_reward_ack;
	/* Title */
	clif->change_title_ack = clif_change_title_ack;
	clif->pChangeTitle = clif_parse_change_title;

	/*------------------------
	 *- Parse Incoming Packet
	 *------------------------*/
	clif->pWantToConnection = clif_parse_WantToConnection;
	clif->pLoadEndAck = clif_parse_LoadEndAck;
	clif->pTickSend = clif_parse_TickSend;
	clif->pHotkey1 = clif_parse_Hotkey1;
	clif->pHotkey2 = clif_parse_Hotkey2;
	clif->pProgressbar = clif_parse_progressbar;
	clif->pWalkToXY = clif_parse_WalkToXY;
	clif->pQuitGame = clif_parse_QuitGame;
	clif->pGetCharNameRequest = clif_parse_GetCharNameRequest;
	clif->pGlobalMessage = clif_parse_GlobalMessage;
	clif->pMapMove = clif_parse_MapMove;
	clif->pChangeDir = clif_parse_ChangeDir;
	clif->pEmotion = clif_parse_Emotion;
	clif->pHowManyConnections = clif_parse_HowManyConnections;
	clif->pActionRequest = clif_parse_ActionRequest;
	clif->pActionRequest_sub = clif_parse_ActionRequest_sub;
	clif->pRestart = clif_parse_Restart;
	clif->pWisMessage = clif_parse_WisMessage;
	clif->pBroadcast = clif_parse_Broadcast;
	clif->pTakeItem = clif_parse_TakeItem;
	clif->pDropItem = clif_parse_DropItem;
	clif->pUseItem = clif_parse_UseItem;
	clif->pEquipItem = clif_parse_EquipItem;
	clif->pUnequipItem = clif_parse_UnequipItem;
	clif->pNpcClicked = clif_parse_NpcClicked;
	clif->pNpcBuySellSelected = clif_parse_NpcBuySellSelected;
	clif->pNpcBuyListSend = clif_parse_NpcBuyListSend;
	clif->pNpcSellListSend = clif_parse_NpcSellListSend;
	clif->pCreateChatRoom = clif_parse_CreateChatRoom;
	clif->pChatAddMember = clif_parse_ChatAddMember;
	clif->pChatRoomStatusChange = clif_parse_ChatRoomStatusChange;
	clif->pChangeChatOwner = clif_parse_ChangeChatOwner;
	clif->pKickFromChat = clif_parse_KickFromChat;
	clif->pChatLeave = clif_parse_ChatLeave;
	clif->pTradeRequest = clif_parse_TradeRequest;
	clif->pTradeAck = clif_parse_TradeAck;
	clif->pTradeAddItem = clif_parse_TradeAddItem;
	clif->pTradeOk = clif_parse_TradeOk;
	clif->pTradeCancel = clif_parse_TradeCancel;
	clif->pTradeCommit = clif_parse_TradeCommit;
	clif->pStopAttack = clif_parse_StopAttack;
	clif->pPutItemToCart = clif_parse_PutItemToCart;
	clif->pGetItemFromCart = clif_parse_GetItemFromCart;
	clif->pRemoveOption = clif_parse_RemoveOption;
	clif->pChangeCart = clif_parse_ChangeCart;
	clif->pSelectCart = clif_parse_SelectCart;
	clif->pStatusUp = clif_parse_StatusUp;
	clif->pSkillUp = clif_parse_SkillUp;
	clif->pUseSkillToId = clif_parse_UseSkillToId;
	clif->pUseSkillToId_homun = clif_parse_UseSkillToId_homun;
	clif->pUseSkillToId_mercenary = clif_parse_UseSkillToId_mercenary;
	clif->pStartUseSkillToId = clif_parse_startUseSkillToId;
	clif->pStopUseSkillToId = clif_parse_stopUseSkillToId;
	clif->useSkillToIdReal = clif_useSkillToIdReal;
	clif->pUseSkillToPos = clif_parse_UseSkillToPos;
	clif->pUseSkillToPosSub = clif_parse_UseSkillToPosSub;
	clif->pUseSkillToPos_homun = clif_parse_UseSkillToPos_homun;
	clif->pUseSkillToPos_mercenary = clif_parse_UseSkillToPos_mercenary;
	clif->pUseSkillToPosMoreInfo = clif_parse_UseSkillToPosMoreInfo;
	clif->pUseSkillMap = clif_parse_UseSkillMap;
	clif->pRequestMemo = clif_parse_RequestMemo;
	clif->pProduceMix = clif_parse_ProduceMix;
	clif->pCooking = clif_parse_Cooking;
	clif->pRepairItem = clif_parse_RepairItem;
	clif->pWeaponRefine = clif_parse_WeaponRefine;
	clif->pNpcSelectMenu = clif_parse_NpcSelectMenu;
	clif->pNpcNextClicked = clif_parse_NpcNextClicked;
	clif->pNpcAmountInput = clif_parse_NpcAmountInput;
	clif->pNpcStringInput = clif_parse_NpcStringInput;
	clif->pNpcCloseClicked = clif_parse_NpcCloseClicked;
	clif->pItemIdentify = clif_parse_ItemIdentify;
	clif->pSelectArrow = clif_parse_SelectArrow;
	clif->pAutoSpell = clif_parse_AutoSpell;
	clif->pUseCard = clif_parse_UseCard;
	clif->pInsertCard = clif_parse_InsertCard;
	clif->pSolveCharName = clif_parse_SolveCharName;
	clif->pResetChar = clif_parse_ResetChar;
	clif->pLocalBroadcast = clif_parse_LocalBroadcast;
	clif->pMoveToKafra = clif_parse_MoveToKafra;
	clif->pMoveFromKafra = clif_parse_MoveFromKafra;
	clif->pMoveToKafraFromCart = clif_parse_MoveToKafraFromCart;
	clif->pMoveFromKafraToCart = clif_parse_MoveFromKafraToCart;
	clif->pCloseKafra = clif_parse_CloseKafra;
	clif->pStoragePassword = clif_parse_StoragePassword;
	clif->pCreateParty = clif_parse_CreateParty;
	clif->pCreateParty2 = clif_parse_CreateParty2;
	clif->pPartyInvite = clif_parse_PartyInvite;
	clif->pPartyInvite2 = clif_parse_PartyInvite2;
	clif->pReplyPartyInvite = clif_parse_ReplyPartyInvite;
	clif->pReplyPartyInvite2 = clif_parse_ReplyPartyInvite2;
	clif->pLeaveParty = clif_parse_LeaveParty;
	clif->pRemovePartyMember = clif_parse_RemovePartyMember;
	clif->pPartyChangeOption = clif_parse_PartyChangeOption;
	clif->pPartyMessage = clif_parse_PartyMessage;
	clif->pPartyChangeLeader = clif_parse_PartyChangeLeader;
	clif->pPartyBookingRegisterReq = clif_parse_PartyBookingRegisterReq;
	clif->pPartyBookingSearchReq = clif_parse_PartyBookingSearchReq;
	clif->pPartyBookingDeleteReq = clif_parse_PartyBookingDeleteReq;
	clif->pPartyBookingUpdateReq = clif_parse_PartyBookingUpdateReq;
	clif->pPartyRecruitRegisterReq = clif_parse_PartyRecruitRegisterReq;
	clif->pPartyRecruitSearchReq = clif_parse_PartyRecruitSearchReq;
	clif->pPartyRecruitDeleteReq = clif_parse_PartyRecruitDeleteReq;
	clif->pPartyRecruitUpdateReq = clif_parse_PartyRecruitUpdateReq;
	clif->pCloseVending = clif_parse_CloseVending;
	clif->pVendingListReq = clif_parse_VendingListReq;
	clif->pPurchaseReq = clif_parse_PurchaseReq;
	clif->pPurchaseReq2 = clif_parse_PurchaseReq2;
	clif->pOpenVending = clif_parse_OpenVending;
	clif->pCreateGuild = clif_parse_CreateGuild;
	clif->pGuildCheckMaster = clif_parse_GuildCheckMaster;
	clif->pGuildRequestInfo = clif_parse_GuildRequestInfo;
	clif->pGuildChangePositionInfo = clif_parse_GuildChangePositionInfo;
	clif->pGuildChangeMemberPosition = clif_parse_GuildChangeMemberPosition;
	clif->pGuildRequestEmblem = clif_parse_GuildRequestEmblem;
	clif->pGuildChangeEmblem = clif_parse_GuildChangeEmblem;
	clif->pGuildChangeNotice = clif_parse_GuildChangeNotice;
	clif->pGuildInvite = clif_parse_GuildInvite;
	clif->pGuildReplyInvite = clif_parse_GuildReplyInvite;
	clif->pGuildLeave = clif_parse_GuildLeave;
	clif->pGuildExpulsion = clif_parse_GuildExpulsion;
	clif->pGuildMessage = clif_parse_GuildMessage;
	clif->pGuildRequestAlliance = clif_parse_GuildRequestAlliance;
	clif->pGuildReplyAlliance = clif_parse_GuildReplyAlliance;
	clif->pGuildDelAlliance = clif_parse_GuildDelAlliance;
	clif->pGuildOpposition = clif_parse_GuildOpposition;
	clif->pGuildBreak = clif_parse_GuildBreak;
	clif->pPetMenu = clif_parse_PetMenu;
	clif->pCatchPet = clif_parse_CatchPet;
	clif->pSelectEgg = clif_parse_SelectEgg;
	clif->pSendEmotion = clif_parse_SendEmotion;
	clif->pChangePetName = clif_parse_ChangePetName;
	clif->pGMKick = clif_parse_GMKick;
	clif->pGMKickAll = clif_parse_GMKickAll;
	clif->pGMShift = clif_parse_GMShift;
	clif->pGMRemove2 = clif_parse_GMRemove2;
	clif->pGMRecall = clif_parse_GMRecall;
	clif->pGMRecall2 = clif_parse_GMRecall2;
	clif->pGM_Monster_Item = clif_parse_GM_Monster_Item;
	clif->pGMHide = clif_parse_GMHide;
	clif->pGMReqNoChat = clif_parse_GMReqNoChat;
	clif->pGMRc = clif_parse_GMRc;
	clif->pGMReqAccountName = clif_parse_GMReqAccountName;
	clif->pGMChangeMapType = clif_parse_GMChangeMapType;
	clif->pGMFullStrip = clif_parse_GMFullStrip;
	clif->pPMIgnore = clif_parse_PMIgnore;
	clif->pPMIgnoreAll = clif_parse_PMIgnoreAll;
	clif->pPMIgnoreList = clif_parse_PMIgnoreList;
	clif->pNoviceDoriDori = clif_parse_NoviceDoriDori;
	clif->pNoviceExplosionSpirits = clif_parse_NoviceExplosionSpirits;
	clif->pFriendsListAdd = clif_parse_FriendsListAdd;
	clif->pFriendsListReply = clif_parse_FriendsListReply;
	clif->pFriendsListRemove = clif_parse_FriendsListRemove;
	clif->pPVPInfo = clif_parse_PVPInfo;
	clif->pBlacksmith = clif_parse_Blacksmith;
	clif->pAlchemist = clif_parse_Alchemist;
	clif->pTaekwon = clif_parse_Taekwon;
	clif->pRankingPk = clif_parse_RankingPk;
	clif->pFeelSaveOk = clif_parse_FeelSaveOk;
	clif->pChangeHomunculusName = clif_parse_ChangeHomunculusName;
	clif->pHomMoveToMaster = clif_parse_HomMoveToMaster;
	clif->pHomMoveTo = clif_parse_HomMoveTo;
	clif->pHomAttack = clif_parse_HomAttack;
	clif->pHomMenu = clif_parse_HomMenu;
	clif->pAutoRevive = clif_parse_AutoRevive;
	clif->pCheck = clif_parse_Check;
	clif->pMail_refreshinbox = clif_parse_Mail_refreshinbox;
	clif->pMail_read = clif_parse_Mail_read;
	clif->pMail_getattach = clif_parse_Mail_getattach;
	clif->pMail_delete = clif_parse_Mail_delete;
	clif->pMail_return = clif_parse_Mail_return;
	clif->pMail_setattach = clif_parse_Mail_setattach;
	clif->pMail_winopen = clif_parse_Mail_winopen;
	clif->pMail_send = clif_parse_Mail_send;
	clif->pAuction_cancelreg = clif_parse_Auction_cancelreg;
	clif->pAuction_setitem = clif_parse_Auction_setitem;
	clif->pAuction_register = clif_parse_Auction_register;
	clif->pAuction_cancel = clif_parse_Auction_cancel;
	clif->pAuction_close = clif_parse_Auction_close;
	clif->pAuction_bid = clif_parse_Auction_bid;
	clif->pAuction_search = clif_parse_Auction_search;
	clif->pAuction_buysell = clif_parse_Auction_buysell;
	clif->pcashshop_buy = clif_parse_cashshop_buy;
	clif->pAdopt_request = clif_parse_Adopt_request;
	clif->pAdopt_reply = clif_parse_Adopt_reply;
	clif->pViewPlayerEquip = clif_parse_ViewPlayerEquip;
	clif->p_cz_config = clif_parse_cz_config;
	clif->pquestStateAck = clif_parse_questStateAck;
	clif->pmercenary_action = clif_parse_mercenary_action;
	clif->pBattleChat = clif_parse_BattleChat;
	clif->pLessEffect = clif_parse_LessEffect;
	clif->pItemListWindowSelected = clif_parse_ItemListWindowSelected;
	clif->pReqOpenBuyingStore = clif_parse_ReqOpenBuyingStore;
	clif->pReqCloseBuyingStore = clif_parse_ReqCloseBuyingStore;
	clif->pReqClickBuyingStore = clif_parse_ReqClickBuyingStore;
	clif->pReqTradeBuyingStore = clif_parse_ReqTradeBuyingStore;
	clif->pSearchStoreInfo = clif_parse_SearchStoreInfo;
	clif->pSearchStoreInfoNextPage = clif_parse_SearchStoreInfoNextPage;
	clif->pCloseSearchStoreInfo = clif_parse_CloseSearchStoreInfo;
	clif->pSearchStoreInfoListItemClick = clif_parse_SearchStoreInfoListItemClick;
	clif->pDebug = clif_parse_debug;
	clif->pSkillSelectMenu = clif_parse_SkillSelectMenu;
	clif->pMoveItem = clif_parse_MoveItem;
	clif->p_cz_blocking_play_cancel = clif_parse_cz_blocking_play_cancel;
	clif->overweight_percent = clif_overweight_percent;
	clif->pChangeDress = clif_parse_changeDress;
	/* dull */
	clif->pDull = clif_parse_dull;
	/* BGQueue */
	clif->pBGQueueRegister = clif_parse_bgqueue_register;
	clif->pBGQueueCheckState = clif_parse_bgqueue_checkstate;
	clif->pBGQueueRevokeReq = clif_parse_bgqueue_revoke_req;
	clif->pBGQueueBattleBeginAck = clif_parse_bgqueue_battlebegin_ack;
	/* RagExe Cash Shop [Ind/Hercules] */
	clif->pCashShopOpen1 = clif_parse_cashShopOpen1;
	clif->pCashShopOpen2 = clif_parse_cashShopOpen2;
	clif->pCashShopLimitedReq = clif_parse_cashShopLimitedReq;
	clif->pCashShopClose = clif_parse_cashShopClose;
	clif->pCashShopReqTab = clif_parse_cashShopReqTab;
	clif->pCashShopSchedule = clif_parse_cashShopSchedule;
	clif->pCashShopBuy = clif_parse_cashShopBuy;
	clif->cashShopBuyAck = clif_cashShopBuyAck;
	clif->cashShopOpen = clif_cashShopOpen;
	/*  */
	clif->pPartyTick = clif_parse_PartyTick;
	clif->pGuildInvite2 = clif_parse_GuildInvite2;
	/* Group Search System Update */
	clif->pPartyBookingAddFilter = clif_parse_PartyBookingAddFilteringList;
	clif->pPartyBookingSubFilter = clif_parse_PartyBookingSubFilteringList;
	clif->pPartyBookingReqVolunteer = clif_parse_PartyBookingReqVolunteer;
	clif->pPartyBookingRefuseVolunteer = clif_parse_PartyBookingRefuseVolunteer;
	clif->pPartyBookingCancelVolunteer = clif_parse_PartyBookingCancelVolunteer;
	/* Bank System [Yommy/Hercules] */
	clif->pBankDeposit = clif_parse_BankDeposit;
	clif->pBankWithdraw = clif_parse_BankWithdraw;
	clif->pBankCheck = clif_parse_BankCheck;
	clif->pBankOpen = clif_parse_BankOpen;
	clif->pBankClose = clif_parse_BankClose;
	/* Roulette System [Yommy/Hercules] */
	clif->pRouletteOpen = clif_parse_RouletteOpen;
	clif->pRouletteInfo = clif_parse_RouletteInfo;
	clif->pRouletteClose = clif_parse_RouletteClose;
	clif->pRouletteGenerate = clif_parse_RouletteGenerate;
	clif->pRouletteRecvItem = clif_parse_RouletteRecvItem;
	/* */
	clif->pNPCShopClosed = clif_parse_NPCShopClosed;
	/* NPC Market */
	clif->pNPCMarketClosed = clif_parse_NPCMarketClosed;
	clif->pNPCMarketPurchase = clif_parse_NPCMarketPurchase;
	/* */
	clif->add_item_options = clif_add_item_options;
	clif->pHotkeyRowShift1 = clif_parse_HotkeyRowShift1;
	clif->pHotkeyRowShift2 = clif_parse_HotkeyRowShift2;
	clif->dressroom_open = clif_dressroom_open;
	clif->pOneClick_ItemIdentify = clif_parse_OneClick_ItemIdentify;
	/* Achievements [Smokexyz/Hercules] */
	clif->get_bl_name = clif_get_bl_name;
	/* RODEX */
	clif->pRodexOpenWriteMail = clif_parse_rodex_open_write_mail;
	clif->rodex_open_write_mail = clif_rodex_open_write_mail;
	clif->pRodexAddItem = clif_parse_rodex_add_item;
	clif->rodex_add_item_result = clif_rodex_add_item_result;
	clif->pRodexRemoveItem = clif_parse_rodex_remove_item;
	clif->rodex_remove_item_result = clif_rodex_remove_item_result;
	clif->pRodexSendMail = clif_parse_rodex_send_mail;
	clif->rodex_send_mail_result = clif_rodex_send_mail_result;
	clif->rodex_send_maillist = clif_rodex_send_maillist;
	clif->rodex_send_refresh = clif_rodex_send_refresh;
	clif->pRodexReadMail = clif_parse_rodex_read_mail;
	clif->rodex_read_mail = clif_rodex_read_mail;
	clif->pRodexNextMaillist = clif_parse_rodex_next_maillist;
	clif->pRodexCloseMailbox = clif_parse_rodex_close_mailbox;
	clif->pRodexCancelWriteMail = clif_parse_rodex_cancel_write_mail;
	clif->pRodexOpenMailbox = clif_parse_rodex_open_mailbox;
	clif->pRodexCheckName = clif_parse_rodex_checkname;
	clif->rodex_checkname_result = clif_rodex_checkname_result;
	clif->pRodexDeleteMail = clif_parse_rodex_delete_mail;
	clif->rodex_delete_mail = clif_rodex_delete_mail;
	clif->pRodexRefreshMaillist = clif_parse_rodex_refresh_maillist;
	clif->pRodexRequestZeny = clif_parse_rodex_request_zeny;
	clif->rodex_request_zeny = clif_rodex_request_zeny;
	clif->pRodexRequestItems = clif_parse_rodex_request_items;
	clif->rodex_request_items = clif_rodex_request_items;
	clif->rodex_icon = clif_rodex_icon;
	clif->rodex_send_mails_all = clif_rodex_send_mails_all;
	clif->skill_scale = clif_skill_scale;
	// -- Clan system
	clif->clan_basicinfo = clif_clan_basicinfo;
	clif->clan_onlinecount = clif_clan_onlinecount;
	clif->clan_leave = clif_clan_leave;
	clif->clan_message = clif_clan_message;
	clif->pClanMessage = clif_parse_ClanMessage;
	// -- Hat Effect
	clif->hat_effect = clif_hat_effect;
	clif->hat_effect_single = clif_hat_effect_single;
	clif->party_dead_notification = clif_party_dead_notification;

	clif->pAttendanceDB = clif_parse_attendance_db;
	clif->attendancedb_libconfig_sub = clif_attendancedb_libconfig_sub;
	clif->attendance_timediff = clif_attendance_timediff;
	clif->attendance_getendtime = clif_attendance_getendtime;
	clif->pOpenUIRequest = clif_parse_open_ui_request;
	clif->open_ui_send = clif_open_ui_send;
	clif->open_ui = clif_open_ui;
	clif->pAttendanceRewardRequest = clif_parse_attendance_reward_request;
	clif->ui_action = clif_ui_action;
	clif->pPrivateAirshipRequest = clif_parse_private_airship_request;
	clif->PrivateAirshipResponse = clif_private_airship_response;

	clif->pReqStyleChange = clif_parse_cz_req_style_change;
	clif->pReqStyleChange2 = clif_parse_cz_req_style_change2;
	clif->pStyleClose = clif_parse_cz_style_close;
	clif->style_change_response = clif_style_change_response;

	clif->camera_showWindow = clif_camera_showWindow;
	clif->camera_change = clif_camera_change;
	clif->pCameraInfo = clif_parse_cameraInfo;
	clif->item_preview = clif_item_preview;
	clif->enchant_equipment = clif_enchant_equipment;

	// -- Pet Evolution
	clif->pPetEvolution = clif_parse_pet_evolution;
	clif->petEvolutionResult = clif_pet_evolution_result;

	clif->pMemorialDungeonCommand = clif_parse_memorial_dungeon_command;
	clif->pReqRemainTime = clif_parse_reqRemainTime;

	clif->npc_barter_open = clif_npc_barter_open;
	clif->pNPCBarterClosed = clif_parse_NPCBarterClosed;
	clif->pNPCBarterPurchase = clif_parse_NPCBarterPurchase;
	clif->npc_expanded_barter_open = clif_npc_expanded_barter_open;
	clif->pNPCExpandedBarterPurchase = clif_parse_npc_expanded_barter_purchase;
	clif->pNPCExpandedBarterClosed = clif_parse_npc_expanded_barter_closed;
	clif->pClientVersion = clif_parse_clientVersion;
	clif->pPing = clif_parse_ping;
	clif->ping = clif_ping;
	clif->pingTimer = clif_pingTimer;
	clif->pingTimerSub = clif_pingTimerSub;
	clif->pResetCooldown = clif_parse_ResetCooldown;
	clif->loadConfirm = clif_loadConfirm;
	clif->send_selforarea = clif_send_selforarea;
	clif->OpenRefineryUI = clif_OpenRefineryUI;
	clif->pAddItemRefineryUI = clif_parse_AddItemRefineryUI;
	clif->AddItemRefineryUIAck = clif_AddItemRefineryUIAck;
	clif->pRefineryUIClose = clif_parse_RefineryUIClose;
	clif->pRefineryUIRefine = clif_parse_RefineryUIRefine;
	clif->announce_refine_status = clif_announce_refine_status;
	clif->pGuildCastleTeleportRequest = clif_parse_GuildCastleTeleportRequest;
	clif->pGuildCastleInfoRequest = clif_parse_GuildCastleInfoRequest;
	clif->guild_castleteleport_res = clif_guild_castleteleport_res;
	clif->lapineDdukDdak_open = clif_lapineDdukDdak_open;
	clif->lapineDdukDdak_result = clif_lapineDdukDdak_result;
	clif->plapineDdukDdak_ack = clif_parse_lapineDdukDdak_ack;
	clif->plapineDdukDdak_close = clif_parse_lapineDdukDdak_close;
	clif->lapineUpgrade_open = clif_lapineUpgrade_open;
	clif->lapineUpgrade_result = clif_lapineUpgrade_result;
	clif->pLapineUpgrade_close = clif_parse_lapineUpgrade_close;
	clif->pLapineUpgrade_makeItem = clif_parse_lapineUpgrade_makeItem;
	clif->pReqGearOff = clif_parse_reqGearOff;
}
