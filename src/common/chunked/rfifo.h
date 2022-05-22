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
#ifndef COMMON_CHUNKED_RFIFO_H
#define COMMON_CHUNKED_RFIFO_H

struct fifo_chunk_buf {
	char *data;
	int data_size;
};

#define fifo_chunk_buf_init(dataVar) \
	(dataVar).data = NULL; \
	(dataVar).data_size = 0

#define fifo_chunk_buf_clear(dataVar) \
	aFree((dataVar).data); \
	(dataVar).data = NULL; \
	(dataVar).data_size = 0


#define RFIFO_CHUNKED_INIT(p, src_data_size, dst_data) \
	const int p ## _flag = (p)->flag; \
	char **p ## _dst_data_ptr = &((dst_data).data); \
	int *p ## _dst_data_size_ptr = &((dst_data).data_size); \
	const char *p ## _src_data = (p)->data; \
	const size_t p ## _src_data_size = src_data_size

#define RFIFO_CHUNKED_ERROR(p) \
	if (p ## _flag > 2 || p ## _flag < 0 || (p ## _flag == 0 && *p ## _dst_data_ptr != NULL) || (p ## _flag == 1 && *p ## _dst_data_ptr == NULL))

#define RFIFO_CHUNKED_COMPLETE(p) \
	if (p ## _flag == 0 || (p ## _flag == 2 && *p ## _dst_data_ptr == NULL)) { \
		*p ## _dst_data_ptr = aMalloc(p ## _src_data_size); \
		memcpy(*p ## _dst_data_ptr, p ## _src_data, p ## _src_data_size); \
		*p ## _dst_data_size_ptr = p ## _src_data_size; \
	} else if (p ## _flag == 1 || p ## _flag == 2) { \
		*p ## _dst_data_ptr = aRealloc(*p ## _dst_data_ptr, *p ## _dst_data_size_ptr + p ## _src_data_size); \
		memcpy(*p ## _dst_data_ptr + *p ## _dst_data_size_ptr, p ## _src_data, p ## _src_data_size); \
		*p ## _dst_data_size_ptr += p ## _src_data_size; \
	} \
	if (p ## _flag == 2)

#define RFIFO_CHUNKED_FREE(p) \
	aFree(*p ## _dst_data_ptr); \
	*p ## _dst_data_ptr = NULL


#define GET_RFIFO_PACKET_CHUNKED_SIZE(fd, pname) (RFIFOW(fd, 2) - sizeof(struct pname))
#define GET_RBUF_PACKET_CHUNKED_SIZE(fd, pname) (RBUFW(fd, 2) - sizeof(struct pname))
#define GET_RFIFO_API_PROXY_PACKET_CHUNKED_SIZE(fd) GET_RFIFO_PACKET_CHUNKED_SIZE(fd, PACKET_API_PROXY_CHUNKED)

#endif /* COMMON_CHUNKED_RFIFO_H */
