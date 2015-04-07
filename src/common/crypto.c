// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Base Author: RenatoUtsch @ http://hercules.ws

#define HERCULES_CORE

#include "crypto.h"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

bool crypto_random_bytes(unsigned char *buf, int num)
{
	if(1 != RAND_bytes(buf, num))
		return false;

	return true;
}

bool crypto_pbkdf2_hmac_sha512(const char *pass, int passlen,
		const unsigned char *salt, int saltlen, int iter, int outlen,
		unsigned char *out)
{
	if(1 != PKCS5_PBKDF2_HMAC(pass, passlen, salt, saltlen, iter,
				EVP_sha512(), outlen, out))
		return false;

	return true;
}
