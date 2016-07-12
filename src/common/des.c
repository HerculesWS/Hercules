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
#define HERCULES_CORE

#include "des.h"

#include "common/cbasetypes.h"

/** @file
 * Implementation of the des interface.
 */

struct des_interface des_s;
struct des_interface *des;

/// Bitmask for accessing individual bits of a byte.
static const uint8_t mask[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/**
 * Initial permutation (IP).
 */
static void des_IP(struct des_bit64 *src)
{
	static const uint8_t ip_table[64] = {
		58, 50, 42, 34, 26, 18, 10,  2,
		60, 52, 44, 36, 28, 20, 12,  4,
		62, 54, 46, 38, 30, 22, 14,  6,
		64, 56, 48, 40, 32, 24, 16,  8,
		57, 49, 41, 33, 25, 17,  9,  1,
		59, 51, 43, 35, 27, 19, 11,  3,
		61, 53, 45, 37, 29, 21, 13,  5,
		63, 55, 47, 39, 31, 23, 15,  7,
	};
	struct des_bit64 tmp = {{0}};
	int i;

	for(i = 0; i < ARRAYLENGTH(ip_table); ++i) {
		uint8_t j = ip_table[i] - 1;
		if (src->b[(j >> 3) & 7] &  mask[j & 7])
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
		40,  8, 48, 16, 56, 24, 64, 32,
		39,  7, 47, 15, 55, 23, 63, 31,
		38,  6, 46, 14, 54, 22, 62, 30,
		37,  5, 45, 13, 53, 21, 61, 29,
		36,  4, 44, 12, 52, 20, 60, 28,
		35,  3, 43, 11, 51, 19, 59, 27,
		34,  2, 42, 10, 50, 18, 58, 26,
		33,  1, 41,  9, 49, 17, 57, 25,
	};
	struct des_bit64 tmp = {{0}};
	int i;

	for (i = 0; i < ARRAYLENGTH(fp_table); ++i) {
		uint8_t j = fp_table[i] - 1;
		if (src->b[(j >> 3) & 7] &  mask[j & 7])
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
		32,  1,  2,  3,  4,  5,
		 4,  5,  6,  7,  8,  9,
		 8,  9, 10, 11, 12, 13,
		12, 13, 14, 15, 16, 17,
		16, 17, 18, 19, 20, 21,
		20, 21, 22, 23, 24, 25,
		24, 25, 26, 27, 28, 29,
		28, 29, 30, 31, 32,  1,
	};
	int i;

	for (i = 0; i < ARRAYLENGTH(expand_table); ++i) {
		uint8_t j = expand_table[i] - 1;
		if (src->b[j / 8 + 4] &  mask[j % 8])
			tmp.b[i / 6 + 0] |= mask[i % 6];
	}
#endif
	// optimized
	tmp.b[0] = ((src->b[7]<<5) | (src->b[4]>>3)) & 0x3f; // ..0 vutsr
	tmp.b[1] = ((src->b[4]<<1) | (src->b[5]>>7)) & 0x3f; // ..srqpo n
	tmp.b[2] = ((src->b[4]<<5) | (src->b[5]>>3)) & 0x3f; // ..o nmlkj
	tmp.b[3] = ((src->b[5]<<1) | (src->b[6]>>7)) & 0x3f; // ..kjihg f
	tmp.b[4] = ((src->b[5]<<5) | (src->b[6]>>3)) & 0x3f; // ..g fedcb
	tmp.b[5] = ((src->b[6]<<1) | (src->b[7]>>7)) & 0x3f; // ..cba98 7
	tmp.b[6] = ((src->b[6]<<5) | (src->b[7]>>3)) & 0x3f; // ..8 76543
	tmp.b[7] = ((src->b[7]<<1) | (src->b[4]>>7)) & 0x3f; // ..43210 v

	*src = tmp;
}

/**
 * Transposition (P-BOX).
 */
static void des_TP(struct des_bit64 *src)
{
	static const uint8_t tp_table[32] = {
		16,  7, 20, 21,
		29, 12, 28, 17,
		 1, 15, 23, 26,
		 5, 18, 31, 10,
		 2,  8, 24, 14,
		32, 27,  3,  9,
		19, 13, 30,  6,
		22, 11,  4, 25,
	};
	struct des_bit64 tmp = {{0}};
	int i;

	for (i = 0; i < ARRAYLENGTH(tp_table); ++i) {
		uint8_t j = tp_table[i] - 1;
		if (src->b[(j >> 3) + 0] &  mask[j & 7])
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
			0xef, 0x03, 0x41, 0xfd, 0xd8, 0x74, 0x1e, 0x47,  0x26, 0xef, 0xfb, 0x22, 0xb3, 0xd8, 0x84, 0x1e,
			0x39, 0xac, 0xa7, 0x60, 0x62, 0xc1, 0xcd, 0xba,  0x5c, 0x96, 0x90, 0x59, 0x05, 0x3b, 0x7a, 0x85,
			0x40, 0xfd, 0x1e, 0xc8, 0xe7, 0x8a, 0x8b, 0x21,  0xda, 0x43, 0x64, 0x9f, 0x2d, 0x14, 0xb1, 0x72,
			0xf5, 0x5b, 0xc8, 0xb6, 0x9c, 0x37, 0x76, 0xec,  0x39, 0xa0, 0xa3, 0x05, 0x52, 0x6e, 0x0f, 0xd9,
		},{
			0xa7, 0xdd, 0x0d, 0x78, 0x9e, 0x0b, 0xe3, 0x95,  0x60, 0x36, 0x36, 0x4f, 0xf9, 0x60, 0x5a, 0xa3,
			0x11, 0x24, 0xd2, 0x87, 0xc8, 0x52, 0x75, 0xec,  0xbb, 0xc1, 0x4c, 0xba, 0x24, 0xfe, 0x8f, 0x19,
			0xda, 0x13, 0x66, 0xaf, 0x49, 0xd0, 0x90, 0x06,  0x8c, 0x6a, 0xfb, 0x91, 0x37, 0x8d, 0x0d, 0x78,
			0xbf, 0x49, 0x11, 0xf4, 0x23, 0xe5, 0xce, 0x3b,  0x55, 0xbc, 0xa2, 0x57, 0xe8, 0x22, 0x74, 0xce,
		},{
			0x2c, 0xea, 0xc1, 0xbf, 0x4a, 0x24, 0x1f, 0xc2,  0x79, 0x47, 0xa2, 0x7c, 0xb6, 0xd9, 0x68, 0x15,
			0x80, 0x56, 0x5d, 0x01, 0x33, 0xfd, 0xf4, 0xae,  0xde, 0x30, 0x07, 0x9b, 0xe5, 0x83, 0x9b, 0x68,
			0x49, 0xb4, 0x2e, 0x83, 0x1f, 0xc2, 0xb5, 0x7c,  0xa2, 0x19, 0xd8, 0xe5, 0x7c, 0x2f, 0x83, 0xda,
			0xf7, 0x6b, 0x90, 0xfe, 0xc4, 0x01, 0x5a, 0x97,  0x61, 0xa6, 0x3d, 0x40, 0x0b, 0x58, 0xe6, 0x3d,
		},{
			0x4d, 0xd1, 0xb2, 0x0f, 0x28, 0xbd, 0xe4, 0x78,  0xf6, 0x4a, 0x0f, 0x93, 0x8b, 0x17, 0xd1, 0xa4,
			0x3a, 0xec, 0xc9, 0x35, 0x93, 0x56, 0x7e, 0xcb,  0x55, 0x20, 0xa0, 0xfe, 0x6c, 0x89, 0x17, 0x62,
			0x17, 0x62, 0x4b, 0xb1, 0xb4, 0xde, 0xd1, 0x87,  0xc9, 0x14, 0x3c, 0x4a, 0x7e, 0xa8, 0xe2, 0x7d,
			0xa0, 0x9f, 0xf6, 0x5c, 0x6a, 0x09, 0x8d, 0xf0,  0x0f, 0xe3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xcb,
		}
	};
	struct des_bit64 tmp = {{0}};
	int i;

	for (i = 0; i < ARRAYLENGTH(s_table); ++i) {
		tmp.b[i] = (s_table[i][src->b[i*2+0]] & 0xf0)
		         | (s_table[i][src->b[i*2+1]] & 0x0f);
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

	src->b[0] ^= tmp.b[4];
	src->b[1] ^= tmp.b[5];
	src->b[2] ^= tmp.b[6];
	src->b[3] ^= tmp.b[7];
}

/// @copydoc des_interface::decrypt_block()
void des_decrypt_block(struct des_bit64 *block)
{
	des_IP(block);
	des_RoundFunction(block);
	des_FP(block);
}

/// @copydoc des_interface::decrypt()
void des_decrypt(unsigned char *data, size_t size)
{
	struct des_bit64 *p = (struct des_bit64 *)data;
	size_t i;

	for (i = 0; i*8 < size; i += 8)
		des->decrypt_block(p);
}

/**
 * Interface base initialization.
 */
void des_defaults(void)
{
	des = &des_s;
	des->decrypt = des_decrypt;
	des->decrypt_block = des_decrypt_block;
}
