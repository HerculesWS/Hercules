// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

/**
 * @file Simple wrapper for the bcrypt password hashing algorithm implementation
 * as provided by Solar Designer at http://www.openwall.com/crypt/
 * Based on http://github.com/rg3/bcrypt/ (public domain).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _WIN32
	#include <unistd.h>
#else
	#include "../common/winapi.h"
#endif
#include <errno.h>

#include "../common/cbasetypes.h"
#include "../common/random.h"

#include "bcrypt.h"
#include "../../3rdparty/crypt_blowfish/crypt_blowfish.h"

#define RANDBYTES (16)

static int try_close(int fd)
{
	int ret;
	for (;;) {
		errno = 0;
		ret = close(fd);
		if (ret == -1 && errno == EINTR)
			continue;
		break;
	}
	return ret;
}

static int try_read(int fd, void *out, size_t count)
{
	size_t total;
	ssize_t partial;

	total = 0;
	do {
		for (;;) {
			errno = 0;
			partial = read(fd, (char*)out + total, count - total);
			if (partial == -1 && errno == EINTR)
				continue;
			break;
		}

		if (partial < 1)
			return partial;

		total += partial;
	} while (total < count);
	return 0;
}

int bcrypt_gensalt(int factor, char salt[BCRYPT_SALTSIZE])
{
	int fd;
	char input[RANDBYTES];
	char *aux;
	int success = 0;

	do {
		fd = open("/dev/urandom", O_RDONLY);
		if (fd == -1)
			break;

		if (try_read(fd, input, RANDBYTES) != 0) {
			if (try_close(fd) != 0)
				break;
			break;
		}

		if (try_close(fd) != 0)
			break;
		success = 1;
	} while (0);

	if (!success) { // Fallback to Mersenne-Twister
		int i;
		for (i = 0; i < ARRAYLENGTH(input); i++)
			input[i] = rnd()%256;
	}

	/* Generate salt. */
	factor = (factor < 4 || factor > 31) ? BCRYPT_DEFAULT_WORKFACTOR : factor;
	aux = _crypt_gensalt_blowfish_rn(BCRYPT_DEFAULT_PREFIX, factor, input, RANDBYTES, salt, BCRYPT_SALTSIZE);
	return (aux == NULL)?1:0;
}

int bcrypt_hashpw(const char *passwd, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE])
{
	char *aux;
	aux = _crypt_blowfish_rn(passwd, salt, hash, BCRYPT_HASHSIZE);
	return (aux == NULL)?1:0;
}

int bcrypt_hashnew(const char *passwd, char hash[BCRYPT_HASHSIZE], int workfactor)
{
	char salt[BCRYPT_SALTSIZE];
	if (bcrypt_gensalt(workfactor, salt) != 0)
		return 1;
	return bcrypt_hashpw(passwd, salt, hash);
}
