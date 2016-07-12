/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef COMMON_MD5CALC_H
#define COMMON_MD5CALC_H

#include "common/hercules.h"

/** @file
 * md5 calculation algorithm.
 *
 * The source code referred to the following URL.
 * http://www.geocities.co.jp/SiliconValley-Oakland/8878/lab17/lab17.html
 */

/// The md5 interface
struct md5_interface {
	/**
	 * Hashes a string, returning the hash in string format.
	 *
	 * @param[in]  string The source string (NUL terminated).
	 * @param[out] output Output buffer (at least 33 bytes available).
	 */
	void (*string) (const char *string, char *output);

	/**
	 * Hashes a string, returning the buffer in binary format.
	 *
	 * @param[in]  string The source string.
	 * @param[out] output Output buffer (at least 16 bytes available).
	 */
	void (*binary) (const char *string, unsigned char *output);

	/**
	 * Generates a random salt.
	 *
	 * @param[in]  len    The desired salt length.
	 * @param[out] output The output buffer (at least len bytes available).
	 */
	void (*salt) (int len, char *output);
};

#ifdef HERCULES_CORE
void md5_defaults(void);
#endif // HERCULES_CORE

HPShared struct md5_interface *md5; ///< Pointer to the md5 interface.

#endif /* COMMON_MD5CALC_H */
