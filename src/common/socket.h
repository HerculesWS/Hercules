// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef COMMON_SOCKET_H
#define COMMON_SOCKET_H

#include "common/hercules.h"
#include "common/conf.h"

#ifdef WIN32
#	include "common/winapi.h"
	typedef long in_addr_t;
#else
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#endif

struct hplugin_data_store;

#define FIFOSIZE_SERVERLINK 256*1024

// socket I/O macros
#define RFIFOHEAD(fd)
#define WFIFOHEAD(fd, size) \
	do{ \
		if ((fd) && sockt->session[fd]->wdata_size + (size) > sockt->session[fd]->max_wdata) \
			sockt->realloc_writefifo((fd), (size)); \
	} while(0)

#define RFIFOP(fd,pos) (sockt->session[fd]->rdata + sockt->session[fd]->rdata_pos + (pos))
#define WFIFOP(fd,pos) (sockt->session[fd]->wdata + sockt->session[fd]->wdata_size + (pos))

#define RFIFOB(fd,pos) (*(uint8*)RFIFOP((fd),(pos)))
#define WFIFOB(fd,pos) (*(uint8*)WFIFOP((fd),(pos)))
#define RFIFOW(fd,pos) (*(uint16*)RFIFOP((fd),(pos)))
#define WFIFOW(fd,pos) (*(uint16*)WFIFOP((fd),(pos)))
#define RFIFOL(fd,pos) (*(uint32*)RFIFOP((fd),(pos)))
#define WFIFOL(fd,pos) (*(uint32*)WFIFOP((fd),(pos)))
#define RFIFOQ(fd,pos) (*(uint64*)RFIFOP((fd),(pos)))
#define WFIFOQ(fd,pos) (*(uint64*)WFIFOP((fd),(pos)))
#define RFIFOSPACE(fd) (sockt->session[fd]->max_rdata - sockt->session[fd]->rdata_size)
#define WFIFOSPACE(fd) (sockt->session[fd]->max_wdata - sockt->session[fd]->wdata_size)

#define RFIFOREST(fd)  (sockt->session[fd]->flag.eof ? 0 : sockt->session[fd]->rdata_size - sockt->session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) \
	do { \
		if(sockt->session[fd]->rdata_size == sockt->session[fd]->rdata_pos){ \
			sockt->session[fd]->rdata_size = sockt->session[fd]->rdata_pos = 0; \
		} else { \
			sockt->session[fd]->rdata_size -= sockt->session[fd]->rdata_pos; \
			memmove(sockt->session[fd]->rdata, sockt->session[fd]->rdata+sockt->session[fd]->rdata_pos, sockt->session[fd]->rdata_size); \
			sockt->session[fd]->rdata_pos = 0; \
		} \
	} while(0)

#define WFIFOSET(fd, len)  (sockt->wfifoset(fd, len))
#define RFIFOSKIP(fd, len) (sockt->rfifoskip(fd, len))

/* [Ind/Hercules] */
#define RFIFO2PTR(fd) (void*)(sockt->session[fd]->rdata + sockt->session[fd]->rdata_pos)

// buffer I/O macros
#define RBUFP(p,pos) (((uint8*)(p)) + (pos))
#define RBUFB(p,pos) (*(uint8*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(uint16*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(uint32*)RBUFP((p),(pos)))
#define RBUFQ(p,pos) (*(uint64*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((uint8*)(p)) + (pos))
#define WBUFB(p,pos) (*(uint8*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(uint16*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(uint32*)WBUFP((p),(pos)))
#define WBUFQ(p,pos) (*(uint64*)WBUFP((p),(pos)))

#define TOB(n) ((uint8)((n)&UINT8_MAX))
#define TOW(n) ((uint16)((n)&UINT16_MAX))
#define TOL(n) ((uint32)((n)&UINT32_MAX))


// Struct declaration
typedef int (*RecvFunc)(int fd);
typedef int (*SendFunc)(int fd);
typedef int (*ParseFunc)(int fd);

struct socket_data {
	struct {
		unsigned char eof : 1;
		unsigned char server : 1;
		unsigned char ping : 2;
	} flag;

	uint32 client_addr; // remote client address

	uint8 *rdata, *wdata;
	size_t max_rdata, max_wdata;
	size_t rdata_size, wdata_size;
	size_t rdata_pos;
	time_t rdata_tick; // time of last recv (for detecting timeouts); zero when timeout is disabled

	RecvFunc func_recv;
	SendFunc func_send;
	ParseFunc func_parse;

	void* session_data; // stores application-specific data related to the session
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store.
};

struct hSockOpt {
	unsigned int silent : 1;
	unsigned int setTimeo : 1;
};

/// Subnet/IP range in the IP/Mask format.
struct s_subnet {
	uint32 ip;
	uint32 mask;
};

/// Use a shortlist of sockets instead of iterating all sessions for sockets
/// that have data to send or need eof handling.
/// Adapted to use a static array instead of a linked list.
///
/// @author Buuyo-tama
#define SEND_SHORTLIST

// Note: purposely returns four comma-separated arguments
#define CONVIP(ip) ((ip)>>24)&0xFF,((ip)>>16)&0xFF,((ip)>>8)&0xFF,((ip)>>0)&0xFF
#define MAKEIP(a,b,c,d) ((uint32)( ( ( (a)&0xFF ) << 24 ) | ( ( (b)&0xFF ) << 16 ) | ( ( (c)&0xFF ) << 8 ) | ( ( (d)&0xFF ) << 0 ) ))

/**
 * Socket.c interface, mostly for reading however.
 **/
struct socket_interface {
	int fd_max;
	/* */
	time_t stall_time;
	time_t last_tick;
	/* */
	uint32 addr_[16];   // ip addresses of local host (host byte order)
	int naddr_;   // # of ip addresses

	struct socket_data **session;

	struct s_subnet *lan_subnet; ///< LAN subnets array
	int lan_subnet_count;        ///< LAN subnets count
	struct s_subnet *trusted_ip; ///< Trusted IP ranges array
	int trusted_ip_count;        ///< Trusted IP ranges count
	struct s_subnet *allowed_ip; ///< Allowed server IP ranges array
	int allowed_ip_count;        ///< Allowed server IP ranges count

	/* */
	void (*init) (void);
	void (*final) (void);
	/* */
	int (*perform) (int next);
	/* [Ind/Hercules] - socket_datasync */
	void (*datasync) (int fd, bool send);
	/* */
	int (*make_listen_bind) (uint32 ip, uint16 port);
	int (*make_connection) (uint32 ip, uint16 port, struct hSockOpt *opt);
	int (*realloc_fifo) (int fd, unsigned int rfifo_size, unsigned int wfifo_size);
	int (*realloc_writefifo) (int fd, size_t addition);
	int (*wfifoset) (int fd, size_t len);
	int (*rfifoskip) (int fd, size_t len);
	void (*close) (int fd);
	/* */
	bool (*session_is_valid) (int fd);
	bool (*session_is_active) (int fd);
	/* */
	void (*flush) (int fd);
	void (*flush_fifos) (void);
	void (*set_nonblocking) (int fd, unsigned long yes);
	void (*set_defaultparse) (ParseFunc defaultparse);
	/* hostname/ip conversion functions */
	uint32 (*host2ip) (const char* hostname);
	const char * (*ip2str) (uint32 ip, char *ip_str);
	uint32 (*str2ip) (const char* ip_str);
	/* */
	uint16 (*ntows) (uint16 netshort);
	/* */
	int (*getips) (uint32* ips, int max);
	/* */
	void (*eof) (int fd);

	uint32 (*lan_subnet_check) (uint32 ip, struct s_subnet *info);
	bool (*allowed_ip_check) (uint32 ip);
	bool (*trusted_ip_check) (uint32 ip);
	int (*net_config_read_sub) (config_setting_t *t, struct s_subnet **list, int *count, const char *filename, const char *groupname);
	void (*net_config_read) (const char *filename);
};

#ifdef HERCULES_CORE
void socket_defaults(void);
#endif // HERCULES_CORE

HPShared struct socket_interface *sockt;

#endif /* COMMON_SOCKET_H */
