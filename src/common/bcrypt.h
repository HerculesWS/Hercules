// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

/**
 * @file Simple wrapper for the bcrypt password hashing algorithm implementation
 * as provided by Solar Designer at http://www.openwall.com/crypt/
 * Based on http://github.com/rg3/bcrypt/ (public domain).
 */

#ifndef _BCRYPT_H_
#define _BCRYPT_H_

/// 7 characters for prefix & work factor "$2x$nn$"
/// + 22 characters for actual salt
/// + 1 EOS
#define BCRYPT_SALTSIZE (7 + 22 + 1)

/// Like BCRYPT_SALTSIZE
/// + 31 characters for actual password hash
#define BCRYPT_HASHSIZE (7 + 22 + 31 + 1)

/// Default prefix
#define BCRYPT_DEFAULT_PREFIX "$2y$"

/// Default work factor (log2(number of rounds))
#define BCRYPT_DEFAULT_WORKFACTOR 12

/**
 * Generates new random salt.
 * Internally, the code uses /dev/urandom to read a few random bytes needed
 * for the salt. If reading from /dev/urandom is not successful, code falls
 * back to Mersenne-Twister random generator.
 * @param workfactor Work factor between 4 and 31. If work factor is not in
 * the range, it will default to BCRYPT_DEFAULT_WORKFACTOR
 * @param salt Char array to store the generated salt (should typically have
 * BCRYPT_SALTSIZE bytes at least)
 * @returns zero if the salt could be correctly generated and nonzero otherwise
 */
int bcrypt_gensalt(int workfactor, char salt[BCRYPT_SALTSIZE]);

/**
 * Hashes password with salt.
 * It can also be used to verify a hashed password. In that case, provide the
 * expected hash in the salt parameter and verify the output hash is the same
 * as the input hash.
 * Both the salt and the hash parameters should have room for BCRYPT_HASHSIZE
 * characters at least.
 * @param passwd Password to be hashed
 * @param salt Salt to hash the password
 * @param hash Char array to store the generated hash
 * @returns zero if the password could be hashed and nonzero otherwise.
 */
int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE]);

/**
 * Hashes password with new randomly generated salt.
 * @param passwd Password to be hashed
 * @param hash Char array to store the generated hash
 * @param workfactor Work factor between 4 and 31. If work factor is not in
 * the range, it will default to BCRYPT_DEFAULT_WORKFACTOR
 * @returns zero if the password could be hashed and nonzero otherwise.
 */
int bcrypt_hashnew(const char *passwd, char hash[BCRYPT_HASHSIZE], int workfactor);

#endif /* _BCRYPT_H_ */
