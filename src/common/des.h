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
#ifndef COMMON_DES_H
#define COMMON_DES_H

#include "common/hercules.h"

/**
 * @file
 *
 * DES (Data Encryption Standard) algorithm, modified version.
 *
 * @see http://www.eathena.ws/board/index.php?autocom=bugtracker&showbug=5099
 * @see http://en.wikipedia.org/wiki/Data_Encryption_Standard
 * @see http://en.wikipedia.org/wiki/DES_supplementary_material
 */

/* Struct definitions */

/// One 64-bit block.
struct des_bit64 {
	uint8_t b[8];
};

/* Interface */

/// The des interface.
struct des_interface {
	/**
	 * Decrypts a block.
	 *
	 * @param[in,out] block The block to decrypt (in-place).
	 */
	void (*decrypt_block) (struct des_bit64 *block);

	/**
	 * Decrypts a buffer.
	 *
	 * @param [in,out] data The buffer to decrypt (in-place).
	 * @param [in]     size The size of the data.
	 */
	void (*decrypt) (unsigned char *data, size_t size);
};

#ifdef HERCULES_CORE
void des_defaults(void);
#endif // HERCULES_CORE

HPShared struct des_interface *des; ///< Pointer to the des interface implementation.

#endif // COMMON_DES_H
