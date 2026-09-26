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

#include "md5calc.h"

#include "common/cbasetypes.h"
#include "common/nullpo.h"
#include "common/random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @file
 * Implementation of the md5 interface.
 */

static struct md5_interface md5_s;
struct md5_interface *md5;

/// Global variable
static unsigned int *pX;

/// String Table
static const unsigned int T[] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, // 0
    0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501, // 4
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, // 8
    0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821, // 12
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, // 16
    0xD62F105D, 0x2441453,  0xD8A1E681, 0xE7D3FBC8, // 20
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, // 24
    0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A, // 28
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, // 32
    0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70, // 36
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x4881D05,  // 40
    0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665, // 44
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, // 48
    0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1, // 52
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, // 56
    0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391  // 60
};

/// The left is made to rotate x [ n-bit ]. This is diverted as it is from RFC.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// The function used for other calculation
static unsigned int md5_F(unsigned int X, unsigned int Y, unsigned int Z)
{
	return (X & Y) | (~X & Z);
}

static unsigned int md5_G(unsigned int X, unsigned int Y, unsigned int Z)
{
	return (X & Z) | (Y & ~Z);
}

static unsigned int md5_H(unsigned int X, unsigned int Y, unsigned int Z)
{
	return X ^ Y ^ Z;
}

static unsigned int md5_I(unsigned int X, unsigned int Y, unsigned int Z)
{
	return Y ^ (X | ~Z);
}

static unsigned int
    md5_Round(unsigned int a, unsigned int b, unsigned int FGHI, unsigned int k, unsigned int s, unsigned int i)
{
	return b + ROTATE_LEFT(a + FGHI + pX[k] + T[i], s);
}

static void md5_Round1(
    unsigned int *a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int i
)
{
	*a = md5_Round(*a, b, md5_F(b, c, d), k, s, i);
}

static void md5_Round2(
    unsigned int *a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int i
)
{
	*a = md5_Round(*a, b, md5_G(b, c, d), k, s, i);
}

static void md5_Round3(
    unsigned int *a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int i
)
{
	*a = md5_Round(*a, b, md5_H(b, c, d), k, s, i);
}

static void md5_Round4(
    unsigned int *a, unsigned int b, unsigned int c, unsigned int d, unsigned int k, unsigned int s, unsigned int i
)
{
	*a = md5_Round(*a, b, md5_I(b, c, d), k, s, i);
}

static void md5_Round_Calculate(
    const unsigned char *block, unsigned int *A2, unsigned int *B2, unsigned int *C2, unsigned int *D2
)
{
	// create X It is since it is required.
	unsigned int X[16]; // 512bit 64byte
	int j, k;

	// Save A as AA, B as BB, C as CC, and and D as DD (saving of A, B, C, and D)
	unsigned int A = *A2, B = *B2, C = *C2, D = *D2;
	unsigned int AA = A, BB = B, CC = C, DD = D;

	// It is a large region variable reluctantly because of calculation of a round. . . for Round1...4
	pX = X;

	// Copy block(padding_message) i into X
	for (j = 0, k = 0; j < 64; j += 4, k++) {
		X[k] = ((unsigned int)block[j])            // 8byte*4 -> 32byte conversion
		     | (((unsigned int)block[j + 1]) << 8) // A function called Decode as used in the field of RFC
		     | (((unsigned int)block[j + 2]) << 16)
		     | (((unsigned int)block[j + 3]) << 24);
	}

	// Round 1
	md5_Round1(&A, B, C, D, 0, 7, 0);
	md5_Round1(&D, A, B, C, 1, 12, 1);
	md5_Round1(&C, D, A, B, 2, 17, 2);
	md5_Round1(&B, C, D, A, 3, 22, 3);
	md5_Round1(&A, B, C, D, 4, 7, 4);
	md5_Round1(&D, A, B, C, 5, 12, 5);
	md5_Round1(&C, D, A, B, 6, 17, 6);
	md5_Round1(&B, C, D, A, 7, 22, 7);
	md5_Round1(&A, B, C, D, 8, 7, 8);
	md5_Round1(&D, A, B, C, 9, 12, 9);
	md5_Round1(&C, D, A, B, 10, 17, 10);
	md5_Round1(&B, C, D, A, 11, 22, 11);
	md5_Round1(&A, B, C, D, 12, 7, 12);
	md5_Round1(&D, A, B, C, 13, 12, 13);
	md5_Round1(&C, D, A, B, 14, 17, 14);
	md5_Round1(&B, C, D, A, 15, 22, 15);

	// Round 2
	md5_Round2(&A, B, C, D, 1, 5, 16);
	md5_Round2(&D, A, B, C, 6, 9, 17);
	md5_Round2(&C, D, A, B, 11, 14, 18);
	md5_Round2(&B, C, D, A, 0, 20, 19);
	md5_Round2(&A, B, C, D, 5, 5, 20);
	md5_Round2(&D, A, B, C, 10, 9, 21);
	md5_Round2(&C, D, A, B, 15, 14, 22);
	md5_Round2(&B, C, D, A, 4, 20, 23);
	md5_Round2(&A, B, C, D, 9, 5, 24);
	md5_Round2(&D, A, B, C, 14, 9, 25);
	md5_Round2(&C, D, A, B, 3, 14, 26);
	md5_Round2(&B, C, D, A, 8, 20, 27);
	md5_Round2(&A, B, C, D, 13, 5, 28);
	md5_Round2(&D, A, B, C, 2, 9, 29);
	md5_Round2(&C, D, A, B, 7, 14, 30);
	md5_Round2(&B, C, D, A, 12, 20, 31);

	// Round 3
	md5_Round3(&A, B, C, D, 5, 4, 32);
	md5_Round3(&D, A, B, C, 8, 11, 33);
	md5_Round3(&C, D, A, B, 11, 16, 34);
	md5_Round3(&B, C, D, A, 14, 23, 35);
	md5_Round3(&A, B, C, D, 1, 4, 36);
	md5_Round3(&D, A, B, C, 4, 11, 37);
	md5_Round3(&C, D, A, B, 7, 16, 38);
	md5_Round3(&B, C, D, A, 10, 23, 39);
	md5_Round3(&A, B, C, D, 13, 4, 40);
	md5_Round3(&D, A, B, C, 0, 11, 41);
	md5_Round3(&C, D, A, B, 3, 16, 42);
	md5_Round3(&B, C, D, A, 6, 23, 43);
	md5_Round3(&A, B, C, D, 9, 4, 44);
	md5_Round3(&D, A, B, C, 12, 11, 45);
	md5_Round3(&C, D, A, B, 15, 16, 46);
	md5_Round3(&B, C, D, A, 2, 23, 47);

	// Round 4
	md5_Round4(&A, B, C, D, 0, 6, 48);
	md5_Round4(&D, A, B, C, 7, 10, 49);
	md5_Round4(&C, D, A, B, 14, 15, 50);
	md5_Round4(&B, C, D, A, 5, 21, 51);
	md5_Round4(&A, B, C, D, 12, 6, 52);
	md5_Round4(&D, A, B, C, 3, 10, 53);
	md5_Round4(&C, D, A, B, 10, 15, 54);
	md5_Round4(&B, C, D, A, 1, 21, 55);
	md5_Round4(&A, B, C, D, 8, 6, 56);
	md5_Round4(&D, A, B, C, 15, 10, 57);
	md5_Round4(&C, D, A, B, 6, 15, 58);
	md5_Round4(&B, C, D, A, 13, 21, 59);
	md5_Round4(&A, B, C, D, 4, 6, 60);
	md5_Round4(&D, A, B, C, 11, 10, 61);
	md5_Round4(&C, D, A, B, 2, 15, 62);
	md5_Round4(&B, C, D, A, 9, 21, 63);

	// Then perform the following additions. (let's add)
	*A2 = A + AA;
	*B2 = B + BB;
	*C2 = C + CC;
	*D2 = D + DD;

	// The clearance of confidential information
	memset(pX, 0, sizeof(X));
}

/// @copydoc md5_interface::binary()
static void md5_buf2binary(const uint8 *buf, const int buf_size, uint8 *output)
{
	// var
	/*8bit*/
	unsigned char padding_message[64]; // Extended message   512bit 64byte
	const uint8 *pbuf;                 // The position of string in the present scanning notes is held.

	/*32bit*/
	unsigned int buf_bit_len,         // The bit length of string is held.
	    copy_len,                     // The number of bytes which is used by 1-3 and which remained
	    msg_digest[4];                // Message digest   128bit 4byte
	unsigned int *A = &msg_digest[0], // The message digest in accordance with RFC (reference)
	    *B = &msg_digest[1], *C = &msg_digest[2], *D = &msg_digest[3];
	int i;

	// prog
	// Step 3.Initialize MD Buffer (although it is the initialization; step 3 of A, B, C, and D -- unavoidable -- a
	// head)
	*A = 0x67452301;
	*B = 0xEFCDAB89;
	*C = 0x98BADCFE;
	*D = 0x10325476;

	// Step 1.Append Padding Bits (extension of a mark bit)
	// 1-1
	pbuf = buf; // The position of the present character sequence is set.

	// 1-2  Repeat calculation until length becomes less than 64 bytes.
	for (i = buf_size; 64 <= i; i -= 64, pbuf += 64)
		md5_Round_Calculate(pbuf, A, B, C, D);

	// 1-3
	copy_len = buf_size % 64; // The number of bytes which remained is computed.
	memcpy(
	    (char *)padding_message, (const char *)pbuf, copy_len
	);                                                    // A message is copied to an extended bit sequence.
	memset(padding_message + copy_len, 0, 64 - copy_len); // It buries by 0 until it becomes extended bit length.
	padding_message[copy_len] |= 0x80;                    // The next of a message is 1.

	// 1-4
	// If 56 bytes or more (less than 64 bytes) of remainder becomes, it will calculate by extending to 64 bytes.
	if (56 <= copy_len) {
		md5_Round_Calculate(padding_message, A, B, C, D);
		memset(padding_message, 0, 56); // 56 bytes is newly fill uped with 0.
	}

	// Step 2.Append Length (the information on length is added)
	buf_bit_len = buf_size * 8;                    // From the byte chief to bit length (32 bytes of low rank)
	memcpy(&padding_message[56], &buf_bit_len, 4); // 32 bytes of low rank is set.

	// When bit length cannot be expressed in 32 bytes of low rank, it is a beam raising to a higher rank.
	if (UINT_MAX / 8 < (unsigned int)buf_size) {
		unsigned int high = (buf_size - UINT_MAX / 8) * 8;
		memcpy(&padding_message[60], &high, 4);
	} else {
		memset(&padding_message[60], 0, 4); // In this case, it is good for a higher rank at 0.
	}

	// Step 4.Process Message in 16-Word Blocks (calculation of MD5)
	md5_Round_Calculate(padding_message, A, B, C, D);

	// Step 5.Output (output)
	memcpy(output, msg_digest, 16);
}

/// @copydoc md5_interface::string()
static void md5_string(const char *string, char *output)
{
	uint8 digest[16];

	nullpo_retv(string);
	nullpo_retv(output);

	md5->binary((const uint8 *)string, (int)strlen(string), digest);
	snprintf(
	    output, 33, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", digest[0], digest[1],
	    digest[2], digest[3], digest[4], digest[5], digest[6], digest[7], digest[8], digest[9], digest[10],
	    digest[11], digest[12], digest[13], digest[14], digest[15]
	);
}

/// @copydoc md5_interface::salt();
static void md5_salt(int len, char *output)
{
	int i;
	Assert_retv(len > 0);

	for (i = 0; i < len; ++i)
		output[i] = (char)(1 + rnd() % 255);
}

/**
 * Interface base initialization.
 */
void md5_defaults(void)
{
	md5         = &md5_s;
	md5->binary = md5_buf2binary;
	md5->string = md5_string;
	md5->salt   = md5_salt;
}
