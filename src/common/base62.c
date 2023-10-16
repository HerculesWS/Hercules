/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2023 Hercules Dev Team
 * Copyright (C) 2023 KirieZ
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

#include "base62.h"

#include "common/nullpo.h"
#include "common/utils.h"

static struct base62_interface base62_s;
struct base62_interface *base62;

/**
 * Base 62 conversion table
 */
static char base62tbl[62] = {
	'0','1','2','3','4','5','6','7','8',
	'9','a','b','c','d','e','f','g','h',
	'i','j','k','l','m','n','o','p','q',
	'r','s','t','u','v','w','x','y','z',
	'A','B','C','D','E','F','G','H','I',
	'J','K','L','M','N','O','P','Q','R',
	'S','T','U','V','W','X','Y','Z'
};

/**
 * base62-Encode `value` into `buf`, which has a size of `buf_len`.
 * If the base62 string is shorter than `min_len`, left pads it with 0 characters.
 *
 * @remark
 *   - `buf` will be NULL-terminated, so the maximum number of characters in buffer is always `buf_len` - 1.
 *
 * @param value value to be encoded
 * @param buf buffer where to write the values
 * @param min_len minimum length of the string (left padded with 0 when needed)
 * @param buf_len buffer size (including the NULL termination)
 * @returns true if value was fully encoded, false if something went wrong
 */
static bool base62_encode_int_padded(int value, char *buf, int min_len, int buf_len)
{
	nullpo_retr(false, buf);
	Assert_retr(false, min_len < buf_len);
	Assert_retr(false, buf_len >= 2);
	
	// if caller ignores an error, at least it gets a NULL-terminated string
	buf[0] = '\0';

	char temp_buf[BASE62_INT_BUFFER_LEN] = { 0 };
	int max_idx = cap_value(buf_len - 2, 0, BASE62_INT_BUFFER_LEN - 2);

	int idx = 0;
	do {
		temp_buf[idx] = base62tbl[value % 62];
		value /= 62;
		idx++;
	} while (idx <= max_idx && value > 0);

	Assert_retr(false, (value == 0));

	int final_buf_idx = 0;
	
	if (idx < min_len) {
		int pad_size = min_len - idx;
		for (int i = 0; i < pad_size; ++i) {
			buf[final_buf_idx] = base62tbl[0];
			final_buf_idx++;
		}
	}

	idx--;

	// reverse the string to get the base62-encoded value.
	while (idx >= 0) {
		buf[final_buf_idx] = temp_buf[idx];
		final_buf_idx++;
		idx--;
	}
	buf[final_buf_idx] = '\0';

	return (value == 0);
}

/**
 *
 **/
void base62_defaults(void)
{
	base62 = &base62_s;
	base62->encode_int_padded = base62_encode_int_padded;
}
