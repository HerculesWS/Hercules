// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Base Author: RenatoUtsch @ http://hercules.ws

#ifndef COMMON_CRYPTO_H
#define COMMON_CRYPTO_H

#include "cbasetypes.h"

/**
 * Generates num cryptographically strong pseudo-random bytes and puts them into
 * buf.
 * @param buf The output buffer of the random bytes. This output buffer must
 * already be allocated previously with num bytes. Note that a NULL character
 * will not be added, as this is NOT a string. It is an array of bytes.
 * @param num The number of bytes to generate.
 * @return true if it succeeded and false if it failed to generate the random
 * bytes.
 */
bool crypto_random_bytes(unsigned char *buf, int num);

/**
 * Generates a hash using the PBKDF2 algorithm together with HMAC-SHA512.
 * @return true if the hash was successfully generated and false if an error
 * occurred.
 * @param pass The password to be hashed.
 * @param passlen The length of the pass. If -1, this function will use
 * strlen() to calculate the length of the pass.
 * @param salt The salt used in the hashing process.
 * @param saltlen The length of the salt. strlen() may not work because salt
 * may not be a character string, but just a byte array.
 * @param iter The iteration count of the PBKDF2 algorithm. Should be at least
 * 1000.
 * @param outlen The length of the out buffer.
 * @param out The output buffer, that should already be allocated with outlen
 * bytes. Note that this is a byte array, not a string, so it will not (and
 * shouldn't be) NULL terminated.
 * @return true if the function succeeded and false if it failed to generate
 * the password hash.
 */
bool crypto_pbkdf2_hmac_sha512(const char *pass, int passlen,
	const unsigned char *salt, int saltlen, int iter, int outlen,
	unsigned char *out);

#endif /* COMMON_CRYPTO_H */
