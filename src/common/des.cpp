/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2026 Hercules Dev Team
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

#include "des.h"

#include "common/cbasetypes.h"
#include "common/nullpo.h"

/** @file
 * Implementation of the des interface.
 */

static struct des_interface des_s;
struct des_interface *des;

/// Bitmask for accessing individual bits of a byte.
static const uint8_t mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

/**
 * Initial permutation (IP).
 */
static void des_IP(struct des_bit64 *src)
{
	static const uint8_t ip_table[64] = {
	    58, 50, 42, 34, 26, 18, 10, 2, //
	    60, 52, 44, 36, 28, 20, 12, 4, //
	    62, 54, 46, 38, 30, 22, 14, 6, //
	    64, 56, 48, 40, 32, 24, 16, 8, //
	    57, 49, 41, 33, 25, 17, 9,  1, //
	    59, 51, 43, 35, 27, 19, 11, 3, //
	    61, 53, 45, 37, 29, 21, 13, 5, //
	    63, 55, 47, 39, 31, 23, 15, 7, //
	};
	struct des_bit64 tmp = {{0}};
	int i;

	nullpo_retv(src);
	for (i = 0; i < ARRAYLENGTH(ip_table); ++i) {
		uint8_t j = ip_table[i] - 1;
		if (src->b[(j >> 3) & 7] & mask[j & 7])
			tmp.b[(i >> 3) & 7] |= mask[i & 7];
	}

	*src = tmp;
}

/**
 * Final permutation (IP^-1).
 */
static void des_FP(struct des_bit64 *src)
{
	static const uint8_t fp_table[64] = {
	    40, 8, 48, 16, 56, 24, 64, 32, //
	    39, 7, 47, 15, 55, 23, 63, 31, //
	    38, 6, 46, 14, 54, 22, 62, 30, //
	    37, 5, 45, 13, 53, 21, 61, 29, //
	    36, 4, 44, 12, 52, 20, 60, 28, //
	    35, 3, 43, 11, 51, 19, 59, 27, //
	    34, 2, 42, 10, 50, 18, 58, 26, //
	    33, 1, 41, 9,  49, 17, 57, 25, //
	};
	struct des_bit64 tmp = {{0}};
	int i;

	nullpo_retv(src);
	for (i = 0; i < ARRAYLENGTH(fp_table); ++i) {
		uint8_t j = fp_table[i] - 1;
		if (src->b[(j >> 3) & 7] & mask[j & 7])
			tmp.b[(i >> 3) & 7] |= mask[i & 7];
	}

	*src = tmp;
}

/**
 * Expansion (E).
 *
 * Expands upper four 8-bits (32b) into eight 6-bits (48b).
 */
static void des_E(struct des_bit64 *src)
{
	struct des_bit64 tmp = {{0}};

#if 0
	// original
	static const uint8_t expand_table[48] = {
		32,  1,  2,  3,  4,  5, //
		 4,  5,  6,  7,  8,  9, //
		 8,  9, 10, 11, 12, 13, //
		12, 13, 14, 15, 16, 17, //
		16, 17, 18, 19, 20, 21, //
		20, 21, 22, 23, 24, 25, //
		24, 25, 26, 27, 28, 29, //
		28, 29, 30, 31, 32,  1, //
	};
	int i;

	for (i = 0; i < ARRAYLENGTH(expand_table); ++i) {
		uint8_t j = expand_table[i] - 1;
		if (src->b[j / 8 + 4] &  mask[j % 8])
			tmp.b[i / 6 + 0] |= mask[i % 6];
	}
#endif
	nullpo_retv(src);
	// optimized
	tmp.b[0] = ((src->b[7] << 5) | (src->b[4] >> 3)) & 0x3F; // ..0 vutsr
	tmp.b[1] = ((src->b[4] << 1) | (src->b[5] >> 7)) & 0x3F; // ..srqpo n
	tmp.b[2] = ((src->b[4] << 5) | (src->b[5] >> 3)) & 0x3F; // ..o nmlkj
	tmp.b[3] = ((src->b[5] << 1) | (src->b[6] >> 7)) & 0x3F; // ..kjihg f
	tmp.b[4] = ((src->b[5] << 5) | (src->b[6] >> 3)) & 0x3F; // ..g fedcb
	tmp.b[5] = ((src->b[6] << 1) | (src->b[7] >> 7)) & 0x3F; // ..cba98 7
	tmp.b[6] = ((src->b[6] << 5) | (src->b[7] >> 3)) & 0x3F; // ..8 76543
	tmp.b[7] = ((src->b[7] << 1) | (src->b[4] >> 7)) & 0x3F; // ..43210 v

	*src = tmp;
}

/**
 * Transposition (P-BOX).
 */
static void des_TP(struct des_bit64 *src)
{
	static const uint8_t tp_table[32] = {
	    16, 7,  20, 21, //
	    29, 12, 28, 17, //
	    1,  15, 23, 26, //
	    5,  18, 31, 10, //
	    2,  8,  24, 14, //
	    32, 27, 3,  9,  //
	    19, 13, 30, 6,  //
	    22, 11, 4,  25, //
	};
	struct des_bit64 tmp = {{0}};
	int i;

	nullpo_retv(src);
	for (i = 0; i < ARRAYLENGTH(tp_table); ++i) {
		uint8_t j = tp_table[i] - 1;
		if (src->b[(j >> 3) + 0] & mask[j & 7])
			tmp.b[(i >> 3) + 4] |= mask[i & 7];
	}

	*src = tmp;
}

/**
 * Substitution boxes (S-boxes).
 *
 * This implementation was optimized to process two nibbles in one step (twice
 * as fast).
 */
static void des_SBOX(struct des_bit64 *src)
{
	static const uint8_t s_table[4][64] = {
	    {
             0xEF, 0x03, 0x41, 0xFD, 0xD8, 0x74, 0x1E, 0x47, 0x26, 0xEF, 0xFB, 0x22, 0xB3, 0xD8, 0x84, 0x1E,
             0x39, 0xAC, 0xA7, 0x60, 0x62, 0xC1, 0xCD, 0xBA, 0x5C, 0x96, 0x90, 0x59, 0x05, 0x3B, 0x7A, 0x85,
             0x40, 0xFD, 0x1E, 0xC8, 0xE7, 0x8A, 0x8B, 0x21, 0xDA, 0x43, 0x64, 0x9F, 0x2D, 0x14, 0xB1, 0x72,
             0xF5, 0x5B, 0xC8, 0xB6, 0x9C, 0x37, 0x76, 0xEC, 0x39, 0xA0, 0xA3, 0x05, 0x52, 0x6E, 0x0F, 0xD9,
	     },
	    {
             0xA7, 0xDD, 0x0D, 0x78, 0x9E, 0x0B, 0xE3, 0x95, 0x60, 0x36, 0x36, 0x4F, 0xF9, 0x60, 0x5A, 0xA3,
             0x11, 0x24, 0xD2, 0x87, 0xC8, 0x52, 0x75, 0xEC, 0xBB, 0xC1, 0x4C, 0xBA, 0x24, 0xFE, 0x8F, 0x19,
             0xDA, 0x13, 0x66, 0xAF, 0x49, 0xD0, 0x90, 0x06, 0x8C, 0x6A, 0xFB, 0x91, 0x37, 0x8D, 0x0D, 0x78,
             0xBF, 0x49, 0x11, 0xF4, 0x23, 0xE5, 0xCE, 0x3B, 0x55, 0xBC, 0xA2, 0x57, 0xE8, 0x22, 0x74, 0xCE,
	     },
	    {
             0x2C, 0xEA, 0xC1, 0xBF, 0x4A, 0x24, 0x1F, 0xC2, 0x79, 0x47, 0xA2, 0x7C, 0xB6, 0xD9, 0x68, 0x15,
             0x80, 0x56, 0x5D, 0x01, 0x33, 0xFD, 0xF4, 0xAE, 0xDE, 0x30, 0x07, 0x9B, 0xE5, 0x83, 0x9B, 0x68,
             0x49, 0xB4, 0x2E, 0x83, 0x1F, 0xC2, 0xB5, 0x7C, 0xA2, 0x19, 0xD8, 0xE5, 0x7C, 0x2F, 0x83, 0xDA,
             0xF7, 0x6B, 0x90, 0xFE, 0xC4, 0x01, 0x5A, 0x97, 0x61, 0xA6, 0x3D, 0x40, 0x0B, 0x58, 0xE6, 0x3D,
	     },
	    {
             0x4D, 0xD1, 0xB2, 0x0F, 0x28, 0xBD, 0xE4, 0x78, 0xF6, 0x4A, 0x0F, 0x93, 0x8B, 0x17, 0xD1, 0xA4,
             0x3A, 0xEC, 0xC9, 0x35, 0x93, 0x56, 0x7E, 0xCB, 0x55, 0x20, 0xA0, 0xFE, 0x6C, 0x89, 0x17, 0x62,
             0x17, 0x62, 0x4B, 0xB1, 0xB4, 0xDE, 0xD1, 0x87, 0xC9, 0x14, 0x3C, 0x4A, 0x7E, 0xA8, 0xE2, 0x7D,
             0xA0, 0x9F, 0xF6, 0x5C, 0x6A, 0x09, 0x8D, 0xF0, 0x0F, 0xE3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xCB,
	     }
	};
	struct des_bit64 tmp = {{0}};
	int i;

	nullpo_retv(src);
	for (i = 0; i < ARRAYLENGTH(s_table); ++i) {
		tmp.b[i] = (s_table[i][src->b[i * 2 + 0]] & 0xF0) | (s_table[i][src->b[i * 2 + 1]] & 0x0F);
	}

	*src = tmp;
}

/**
 * DES round function.
 *
 * XORs src[0..3] with TP(SBOX(E(src[4..7]))).
 */
static void des_RoundFunction(struct des_bit64 *src)
{
	struct des_bit64 tmp = *src;
	des_E(&tmp);
	des_SBOX(&tmp);
	des_TP(&tmp);

	nullpo_retv(src);
	src->b[0] ^= tmp.b[4];
	src->b[1] ^= tmp.b[5];
	src->b[2] ^= tmp.b[6];
	src->b[3] ^= tmp.b[7];
}

/// @copydoc des_interface::decrypt_block()
static void des_decrypt_block(struct des_bit64 *block)
{
	des_IP(block);
	des_RoundFunction(block);
	des_FP(block);
}

/// @copydoc des_interface::decrypt()
static void des_decrypt(unsigned char *data, size_t size)
{
	struct des_bit64 *p = (struct des_bit64 *)data;
	size_t i;

	for (i = 0; i * 8 < size; i += 8)
		des->decrypt_block(p);
}

/**
 * Interface base initialization.
 */
void des_defaults(void)
{
	des                = &des_s;
	des->decrypt       = des_decrypt;
	des->decrypt_block = des_decrypt_block;
}
