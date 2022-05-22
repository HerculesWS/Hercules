/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include "common/cbasetypes.h"
#include "common/chunked/rfifo.h"
#include "common/chunked/wfifo.h"
#include "common/core.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef WFIFOHEAD
#undef WFIFOP
#undef WFIFOSET

#define WFIFOHEAD(fd, size) fake_WFIFOHEAD(fd, size)
#define WFIFOP(fd, pos) fake_WFIFOP(fd, pos)
#define WFIFOSET(fd, size) fake_WFIFOSET(fd, size)

#undef RFIFOHEAD
#undef RFIFOP
#undef RFIFOSET

#define RFIFOHEAD(fd, size) fake_RFIFOHEAD(fd, size)
#define RFIFOP(fd, pos) fake_RFIFOP(fd, pos)
#define RFIFOSET(fd, size) fake_RFIFOSET(fd, size)

#undef WFIFO_CHUNK_SIZE
#define WFIFO_CHUNK_SIZE fake_GET_WCHUNK_SIZE()

#define SHOW_TEST_ERROR(...) ShowError("  failed: " #__VA_ARGS__ "\n");

#define TEST(...) \
	if (!(__VA_ARGS__)) { \
		ShowError("  failed: " #__VA_ARGS__ "\n"); \
		exit(1); \
	} else if (show_success) { \
		ShowStatus("  passed: " #__VA_ARGS__ "\n"); \
	}

#define TEST_INT(a, b) \
	if ((a) != (b)) { \
		ShowError("  failed: " #a " == " #b "  ->  %d == %d\n", a, b); \
		exit(1); \
	} else if (show_success) { \
		ShowStatus("  passed: " #a " == " #b "  ->  %d\n", a); \
	}

#define TEST_UINT(a, b) \
	if ((a) != (b)) { \
		ShowError("  failed: " #a " == " #b "  ->  %u == %u\n", a, b); \
		exit(1); \
	} else if (show_success) { \
		ShowStatus("  passed: " #a " == " #b "  ->  %u\n", a); \
	}

#define TEST_BUF(a, b, size) \
	if (memcmp(a, b, size) != 0) { \
		ShowError("  failed: " #a " == " #b "  ->\n"); \
		ShowBuf("   a   ", a, size); \
		ShowBuf("   b   ", b, size); \
		ShowBufDiff("   diff", a, b, size); \
		exit(1); \
	} else if (show_success) { \
		ShowStatus("  passed: " #a " == " #b "  ->  "); \
		ShowBuf("", a, size); \
	}

//#define DEBUGLOG

struct PACKET_TEST_CHUNKED {
	int16 packet_id;
	int16 packet_len;
	int16 msg_id;  // some persistent data field
	uint8 flag;
	char data[];
} __attribute__((packed));

//
// Simple test for chunked packets transfer
//

static bool show_success = true;

static int fake_wfd = 0;
static int fake_wsize = 0;
static uint8 *fake_wbuf = NULL;
static void (*pWFIFOSET) (int fd, int size) = NULL;
static int fake_wchunk_size = 5;

static int fake_rfd = 0;
static struct fifo_chunk_buf fake_rbuf;
static char *fake_rflags = NULL;
static int fake_rflags_ptr = 0;
static void (*pRecv) (char *buf, int size) = NULL;
static int recv_cnt = 0;
static bool recv_complete = false;

static int fake_GET_WCHUNK_SIZE(void)
{
	return fake_wchunk_size;
}

static void fake_WFIFOHEAD(int fd, int size)
{
#ifdef DEBUGLOG
	ShowInfo("    WFIFIFOHEAD(%d, %d)\n", fd, size);
#endif
	Assert_retv(fake_wfd == 0);
	Assert_retv(fake_wsize == 0);
	Assert_retv(fake_wbuf == NULL);

	fake_wfd = fd;
	fake_wsize = size;
	fake_wbuf = aCalloc(1, size);
}

static void fake_WFIFOSET(int fd, int size)
{
#ifdef DEBUGLOG
	ShowInfo("    WFIFIFOSET(%d, %d)\n", fd, size);
#endif
	Assert_retv(fake_wfd == fd);
	Assert_retv(fake_wsize == size);
	Assert_retv(fake_wbuf != NULL);

	if (pWFIFOSET != NULL)
		pWFIFOSET(fd, size);

	fake_wfd = 0;
	fake_wsize = 0;
	aFree(fake_wbuf);
	fake_wbuf = NULL;
}

static void *fake_WFIFOP(int fd, int pos)
{
#ifdef DEBUGLOG
	ShowInfo("    WFIFIFOP(%d, %d)\n", fd, pos);
#endif
	Assert_ret(fake_wfd == fd);
	Assert_ret(fake_wbuf != NULL);
	Assert_ret(fake_wsize != 0);
	return (void*)(fake_wbuf + pos);
}

static void recv_clear(void)
{
	fake_rfd = 0;
	fifo_chunk_buf_clear(fake_rbuf);
	aFree(fake_rflags);
	fake_rflags = NULL;
	fake_rflags_ptr = 0;
	recv_cnt = 0;
	recv_complete = false;
}

static void write_clear(void)
{
	fake_wfd = 0;
	fake_wsize = 0;
	aFree(fake_wbuf);
	fake_wbuf = NULL;
}

void ShowBuf(const char *msg, void *buf, const size_t size)
{
	printf("%s: ", msg);
	for (size_t f = 0; f < size; f ++) {
		printf("%02x", ((uint8*)buf)[f]);
	}
	printf("  ");
	for (size_t f = 0; f < size; f ++) {
		uint8 chr = ((uint8*)buf)[f];
		if (chr >= 0x20 && chr < 0x80)
			printf("%c", chr);
		else
			printf("_");
	}
	printf("\n");
}

void ShowBufDiff(const char *msg, void *buf1, void *buf2, const size_t size)
{
	printf("%s: ", msg);
	for (size_t f = 0; f < size; f ++) {
		if (((uint8*)buf1)[f] == ((uint8*)buf2)[f])
			printf("  ");
		else
			printf("XX");
	}
	printf("  ");
	for (size_t f = 0; f < size; f ++) {
		if (((uint8*)buf1)[f] == ((uint8*)buf2)[f])
			printf(" ");
		else
			printf("X");
	}
	printf("\n");
}

static void testMacro(void)
{
	ShowStatus("Test fake fifo macroses\n");

	int fd = 1;
	WFIFOHEAD(fd, 100);

	WFIFOL(fd, 0) = 2;
	TEST_UINT(WFIFOL(fd, 0), 2U);
	TEST_UINT(WFIFOB(fd, 0), 2U);
	TEST_UINT(WFIFOW(fd, 0), 2U);
	TEST_UINT(WFIFOB(fd, 1), 0U);
	TEST_UINT(WFIFOW(fd, 1), 0U);

	WFIFOB(fd, 2) = 1;
	TEST_UINT(WFIFOL(fd, 0), 0x10002U);
	TEST_UINT(WFIFOB(fd, 0), 2U);
	TEST_UINT(WFIFOW(fd, 0), 2U);
	TEST_UINT(WFIFOB(fd, 1), 0U);
	TEST_UINT(WFIFOW(fd, 1), 0x100U);
	TEST_UINT(WFIFOW(fd, 2), 1U);

	WFIFOSET(fd, 100);
}

static void testChunked1Send(int fd, int size)
{
#ifdef DEBUGLOG
	ShowBuf("test send called: ", fake_wbuf, fake_wsize);
#endif
	// reallocate buffer always for detect overflow
	char *buf = aCalloc(1, size);
	memcpy(buf, fake_wbuf, size);
	if (pRecv != NULL)
		pRecv(buf, size);
	aFree(buf);
}

static void testChunked1Recv(char *buf, int size)
{
#ifdef DEBUGLOG
	ShowBuf("test recv called: ", buf, size);
#endif

	nullpo_retv(buf);
	Assert_retv(size > 0);

	struct PACKET_TEST_CHUNKED *p = (struct PACKET_TEST_CHUNKED*)buf;

	recv_cnt++;
	const size_t src_size = GET_RBUF_PACKET_CHUNKED_SIZE(buf, PACKET_TEST_CHUNKED);

	TEST_INT(p->msg_id, 10);

	RFIFO_CHUNKED_INIT(p, src_size, fake_rbuf);

	RFIFO_CHUNKED_ERROR(p) {
		ShowError("Error in testChunked1Recv\n");
		exit(1);
	}

#ifdef DEBUGLOG
	ShowBuf("recv buffer before: ", fake_rbuf.data, fake_rbuf.data_size);
#endif

	RFIFO_CHUNKED_COMPLETE(p) {
#ifdef DEBUGLOG
		ShowStatus("test recv complete\n");
#endif
		recv_complete = true;
	}

	fake_rflags[fake_rflags_ptr++] = p->flag;
#ifdef DEBUGLOG
	ShowBuf("recv buffer after: ", fake_rbuf.data, fake_rbuf.data_size);
#endif
}

static void testChunkedBuf2(char *data, int sz)
{
	if (show_success)
		ShowStatus("Test chunked: size: %d, '%.*s'\n", fake_wchunk_size, sz, data);

	int fd = 2;
	int data_len = sz;
	int msg_id = 10;
	int cnt = (data_len + 1) / WFIFO_CHUNK_SIZE;

	if ((data_len + 1) % WFIFO_CHUNK_SIZE != 0)
		cnt ++;
	if (cnt == 0)
		cnt = 1;

	recv_clear();
	write_clear();

	// reallocate buffer always for detect overflow
	fake_rflags = aCalloc(1, cnt);

	WFIFO_CHUNKED_INIT(p, fd, 0x1234, PACKET_TEST_CHUNKED, data, data_len) {
		WFIFO_CHUNKED_BLOCK_START(p);
		p->msg_id = msg_id;
		WFIFO_CHUNKED_BLOCK_END();
	}
	WFIFO_CHUNKED_FINAL_START(p);
	p->msg_id = msg_id;
	WFIFO_CHUNKED_FINAL_END();

	TEST_INT(recv_complete, true);
	TEST_BUF(data, fake_rbuf.data, fake_rbuf.data_size);
	TEST_INT(recv_cnt, cnt);

	if (recv_cnt == 1) {
		if (fake_rflags[0] != 2) {
			SHOW_TEST_ERROR("packet flag is not 2");
			ShowBuf("flags", fake_rflags, recv_cnt);
			exit(1);
		}
	} else {
		if (fake_rflags[0] != 0) {
			SHOW_TEST_ERROR("first packet flag is not 0");
			ShowBuf("flags", fake_rflags, recv_cnt);
			exit(1);
		}
		for (int f = 1; f < recv_cnt - 1; f ++) {
			TEST_INT((int)fake_rflags[f], 1);
		}
		if (fake_rflags[recv_cnt - 1] != 2) {
			SHOW_TEST_ERROR("last packet flag is not 2");
			ShowBuf("flags", fake_rflags, recv_cnt);
			exit(1);
		}
	}
	aFree(fake_rflags);
	fake_rflags = NULL;
}

static void testChunkedBuf(char *data, int sz)
{
	if (sz == 0)
		sz = strlen(data);
	for (int f = 1; f < 30; f ++) {
		fake_wchunk_size = f;
		testChunkedBuf2(data, sz);
	}
}

static void testChunked1(void)
{
	ShowStatus("Test chunked\n");
	pWFIFOSET = testChunked1Send;
	pRecv = testChunked1Recv;
	testChunkedBuf("test line", 0);
	testChunkedBuf("test", 0);
	testChunkedBuf("this is very long data line for chunked packets data.", 0);
	testChunkedBuf("", 0);
	show_success = false;
	ShowStatus("Test long chunked\n");
	for (int f = 0; f < 10000; f += 100) {
		// reallocate buffer always for detect overflow
		char *buf = aCalloc(1, f);
		for (int i = 0; i < f; i ++) {
			buf[i] = '0' + (i % 10);
		}
		testChunkedBuf(buf, f);
		aFree(buf);
	}
}

int do_init(int argc, char **argv)
{
	ShowStatus("Testing chunked packets.\n");

	testMacro();
	testChunked1();

	core->runflag = CORE_ST_STOP;
	return EXIT_SUCCESS;
}

void do_abort(void)
{
}

void set_server_type(void)
{
	SERVER_TYPE = SERVER_TYPE_UNKNOWN;
}

int do_final(void)
{
	ShowStatus("Tests passed.\n");

	return EXIT_SUCCESS;
}

int parse_console(const char* command)
{
	return 0;
}

void cmdline_args_init_local(void) { }
