// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef	_SOCKET_H_
#define _SOCKET_H_

#include "../common/cbasetypes.h"

#ifdef WIN32
	#include "../common/winapi.h"
	typedef long in_addr_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#include <time.h>

struct HPluginData;

#define FIFOSIZE_SERVERLINK 256*1024

// socket I/O macros
#define RFIFOHEAD(fd)
#define WFIFOHEAD(fd, size) do{ if((fd) && session[fd]->wdata_size + (size) > session[fd]->max_wdata ) realloc_writefifo((fd), (size)); }while(0)
#define RFIFOP(fd,pos) (session[fd]->rdata + session[fd]->rdata_pos + (pos))
#define WFIFOP(fd,pos) (session[fd]->wdata + session[fd]->wdata_size + (pos))

#define RFIFOB(fd,pos) (*(uint8*)RFIFOP((fd),(pos)))
#define WFIFOB(fd,pos) (*(uint8*)WFIFOP((fd),(pos)))
#define RFIFOW(fd,pos) (*(uint16*)RFIFOP((fd),(pos)))
#define WFIFOW(fd,pos) (*(uint16*)WFIFOP((fd),(pos)))
#define RFIFOL(fd,pos) (*(uint32*)RFIFOP((fd),(pos)))
#define WFIFOL(fd,pos) (*(uint32*)WFIFOP((fd),(pos)))
#define RFIFOQ(fd,pos) (*(uint64*)RFIFOP((fd),(pos)))
#define WFIFOQ(fd,pos) (*(uint64*)WFIFOP((fd),(pos)))
#define RFIFOSPACE(fd) (session[fd]->max_rdata - session[fd]->rdata_size)
#define WFIFOSPACE(fd) (session[fd]->max_wdata - session[fd]->wdata_size)

#define RFIFOREST(fd)  (session[fd]->flag.eof ? 0 : session[fd]->rdata_size - session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) \
	do { \
		if(session[fd]->rdata_size == session[fd]->rdata_pos){ \
			session[fd]->rdata_size = session[fd]->rdata_pos = 0; \
		} else { \
			session[fd]->rdata_size -= session[fd]->rdata_pos; \
			memmove(session[fd]->rdata, session[fd]->rdata+session[fd]->rdata_pos, session[fd]->rdata_size); \
			session[fd]->rdata_pos = 0; \
		} \
	} while(0)

/* [Ind/Hercules] */
#define RFIFO2PTR(fd) (void*)(session[fd]->rdata + session[fd]->rdata_pos)

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
	
	struct HPluginData **hdata;
	unsigned int hdatac;
};

struct hSockOpt {
	unsigned int silent : 1;
	unsigned int setTimeo : 1;
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
 * This stays out of the interface.
 **/
struct socket_data **session;

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
	int (*WFIFOSET) (int fd, size_t len);
	int (*RFIFOSKIP) (int fd, size_t len);
	void (*close) (int fd);
	/* */
	bool (*session_isValid) (int fd);
	bool (*session_isActive) (int fd);
	/* */
	void (*flush_fifo) (int fd);
	void (*flush_fifos) (void);
	void (*set_nonblocking) (int fd, unsigned long yes);
	void (*set_defaultparse) (ParseFunc defaultparse);
	/* hostname/ip conversion functions */
	uint32 (*host2ip) (const char* hostname);
	const char * (*ip2str) (uint32 ip, char ip_str[16]);
	uint32 (*str2ip) (const char* ip_str);
	/* */
	uint16 (*ntows) (uint16 netshort);
	/* */
	int (*getips) (uint32* ips, int max);
	/* */
	void (*set_eof) (int fd);
};

struct socket_interface *sockt;

void socket_defaults(void);

/* the purpose of these macros is simply to not make calling them be an annoyance */
#ifndef _H_SOCKET_C_
	#define make_listen_bind(ip, port) ( sockt->make_listen_bind(ip, port) )
	#define make_connection(ip, port, opt) ( sockt->make_connection(ip, port, opt) )
	#define realloc_fifo(fd, rfifo_size, wfifo_size) ( sockt->realloc_fifo(fd, rfifo_size, wfifo_size) )
	#define realloc_writefifo(fd, addition) ( sockt->realloc_writefifo(fd, addition) )
	#define WFIFOSET(fd, len) ( sockt->WFIFOSET(fd, len) )
	#define RFIFOSKIP(fd, len) ( sockt->RFIFOSKIP(fd, len) )
	#define do_close(fd) ( sockt->close(fd) )
	#define session_isValid(fd) ( sockt->session_isValid(fd) )
	#define session_isActive(fd) ( sockt->session_isActive(fd) )
	#define flush_fifo(fd) ( sockt->flush_fifo(fd) )
	#define flush_fifos() ( sockt->flush_fifos() )
	#define set_nonblocking(fd, yes) ( sockt->set_nonblocking(fd, yes) )
	#define set_defaultparse(defaultparse) ( sockt->set_defaultparse(defaultparse) )
	#define host2ip(hostname) ( sockt->host2ip(hostname) )
	#define ip2str(ip, ip_str) ( sockt->ip2str(ip, ip_str) )
	#define str2ip(ip_str) ( sockt->str2ip(ip_str) )
	#define ntows(netshort) ( sockt->ntows(netshort) )
	#define getips(ips, max) ( sockt->getips(ips, max) )
	#define set_eof(fd) ( sockt->set_eof(fd) )
#endif /* _H_SOCKET_C_ */

#endif /* _SOCKET_H_ */
