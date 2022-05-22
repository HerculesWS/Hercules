/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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
#ifndef COMMON_CHUNKED_WFIFO_H
#define COMMON_CHUNKED_WFIFO_H

#ifndef WFIFO_CHUNK_SIZE
#define WFIFO_CHUNK_SIZE 65500
#endif  // WFIFO_CHUNK_SIZE

#ifndef WFIFO_CLIENT_CHUNK_SIZE
#define WFIFO_CLIENT_CHUNK_SIZE 32000
#endif  // WFIFO_CLIENT_CHUNK_SIZE

// base WFIFO_CHUNKED defines
#define WFIFO_CHUNKED_INIT_RAW(p, fd, header, pname, pdata, pdata_len, chunk_size) \
	struct pname *p = NULL; \
	int p ## _len = 0; \
	int p ## _offset = 0; \
	const int p ## _fd = fd; \
	const int p ## _fixed_len = sizeof(struct pname); \
	const int p ## _header_id = header; \
	const char *p ## data = pdata; \
	const int p ## data_len = pdata_len; \
	const int p ## _full_chunks_count = p ## data_len / (chunk_size); \
	for (int p ## _cnt = 0; p ## _cnt < p ## _full_chunks_count; p ## _cnt ++, p ## _offset += (chunk_size))

#define WFIFO_CHUNKED_BLOCK_START_RAW1(p, _dataField, chunk_size) \
		p ## _len = p ## _fixed_len + (chunk_size); \
		WFIFOHEAD(p ## _fd, p ## _len); \
		p = WFIFOP(p ## _fd, 0); \
		WFIFOW(p ## _fd, 0) = p ## _header_id; \
		WFIFOW(p ## _fd, 2) = p ## _len; \
		memcpy((p)->_dataField, p ## data + p ## _offset, (chunk_size))
#define WFIFO_CHUNKED_BLOCK_START_RAW_FLAG(p) \
		if (p ## _cnt == 0) \
			(p)->flag = 0; \
		else \
			(p)->flag = 1

#define WFIFO_CHUNKED_BLOCK_END_RAW() \
		WFIFOSET(p ## _fd, p ## _len)

#define WFIFO_CHUNKED_FINAL_START_RAW1(p, _dataField, chunk_size) \
	const uint32 p ## _left_size = p ## data_len - p ## _full_chunks_count * (chunk_size); \
	p ## _len = p ## _fixed_len + p ## _left_size; \
	WFIFOHEAD(p ## _fd, p ## _len); \
	p = WFIFOP(p ## _fd, 0); \
	WFIFOW(p ## _fd, 0) = p ## _header_id; \
	WFIFOW(p ## _fd, 2) = p ## _len; \
	if (p ## _left_size > 0) \
		memcpy((p)->_dataField, p ## data + p ## _offset, p ## _left_size)

#define WFIFO_CHUNKED_FINAL_START_RAW_FLAG(p) \
	(p)->flag = 2

#define WFIFO_CHUNKED_FINAL_END_RAW() \
		WFIFOSET(p ## _fd, p ## _len)

// inter server WFIFO_CHUNKED defines
#define WFIFO_CHUNKED_INIT(p, fd, header, pname, pdata, pdata_len) \
	WFIFO_CHUNKED_INIT_RAW(p, fd, header, pname, pdata, pdata_len, WFIFO_CHUNK_SIZE)

#define WFIFO_CHUNKED_BLOCK_START(p) \
	WFIFO_CHUNKED_BLOCK_START_RAW1(p, data, WFIFO_CHUNK_SIZE); \
	WFIFO_CHUNKED_BLOCK_START_RAW_FLAG(p)

#define WFIFO_CHUNKED_BLOCK_END() \
	WFIFO_CHUNKED_BLOCK_END_RAW()

#define WFIFO_CHUNKED_FINAL_START(p) \
	WFIFO_CHUNKED_FINAL_START_RAW1(p, data, WFIFO_CHUNK_SIZE); \
	WFIFO_CHUNKED_FINAL_START_RAW_FLAG(p)

#define WFIFO_CHUNKED_FINAL_END() \
	WFIFO_CHUNKED_FINAL_END_RAW()

// server to client WFIFO_CHUNKED defines
#define WFIFO_CLIENT_CHUNKED_INIT(p, fd, header, pname, pdata, pdata_len) \
	WFIFO_CHUNKED_INIT_RAW(p, fd, header, pname, pdata, pdata_len, WFIFO_CLIENT_CHUNK_SIZE)

#define WFIFO_CLIENT_CHUNKED_BLOCK_START(p, _dataField) \
	WFIFO_CHUNKED_BLOCK_START_RAW1(p, _dataField, WFIFO_CLIENT_CHUNK_SIZE)

#define WFIFO_CLIENT_CHUNKED_BLOCK_END() \
	WFIFO_CHUNKED_BLOCK_END_RAW()

#define WFIFO_CLIENT_CHUNKED_FINAL_START(p, _dataField) \
	WFIFO_CHUNKED_FINAL_START_RAW1(p, _dataField, WFIFO_CLIENT_CHUNK_SIZE)

#define WFIFO_CLIENT_CHUNKED_FINAL_END() \
	WFIFO_CHUNKED_FINAL_END_RAW()

#endif /* COMMON_CHUNKED_WFIFO_H */
