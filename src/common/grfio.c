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

#include "grfio.h"

#include "common/cbasetypes.h"
#include "common/des.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <zlib.h>

/** @file
 * Implementation of the GRF I/O interface.
 */

/// File entry table struct.
struct grf_filelist {
	int srclen;         ///< compressed size
	int srclen_aligned;
	int declen;         ///< original size
	int srcpos;         ///< position of entry in grf
	int next;           ///< index of next filelist entry with same hash (-1: end of entry chain)
	char type;
	char fn[128-4*5];   ///< file name
	char *fnd;          ///< if the file was cloned, contains name of original file
	int8 gentry;        ///< read grf file select
};

enum grf_filelist_type {
	FILELIST_TYPE_FILE           = 0x01, ///< entry is a file
	FILELIST_TYPE_ENCRYPT_MIXED  = 0x02, ///< encryption mode 0 (header DES + periodic DES/shuffle)
	FILELIST_TYPE_ENCRYPT_HEADER = 0x04, ///< encryption mode 1 (header DES only)
};

//gentry ... > 0  : data read from a grf file (gentry_table[gentry-1])
//gentry ... 0    : data read from a local file (data directory)
//gentry ... < 0  : entry "-(gentry)" is marked for a local file check
//                  - if local file exists, gentry will be set to 0 (thus using the local file)
//                  - if local file doesn't exist, sign is inverted (thus using the original file inside a grf)
//                  (NOTE: this case is only used once (during startup) and only if GRFIO_LOCAL is enabled)
//                  (NOTE: probably meant to be used to override grf contents by files in the data directory)
//#define GRFIO_LOCAL

// stores info about every loaded file
struct grf_filelist *filelist = NULL;
int filelist_entrys   = 0;
int filelist_maxentry = 0;

// stores grf file names
char** gentry_table = NULL;
int gentry_entrys   = 0;
int gentry_maxentry = 0;

// the path to the data directory
char data_dir[1024] = "";

struct grfio_interface grfio_s;
struct grfio_interface *grfio;

// little endian char array to uint conversion
static unsigned int getlong(unsigned char *p)
{
	nullpo_ret(p);
	return (p[0] << 0 | p[1] << 8 | p[2] << 16 | p[3] << 24);
}

static void NibbleSwap(unsigned char *src, int len)
{
	nullpo_retv(src);
	while (len > 0) {
		*src = (*src >> 4) | (*src << 4);
		++src;
		--len;
	}
}

/**
 * (De)-obfuscates data.
 *
 * Substitutes some specific values for others, leaves rest intact.
 * NOTE: Operation is symmetric (calling it twice gives back the original input).
 */
static uint8_t grf_substitution(uint8_t in)
{
	uint8_t out;

	switch( in )
	{
	case 0x00: out = 0x2B; break;
	case 0x2B: out = 0x00; break;
	case 0x6C: out = 0x80; break;
	case 0x01: out = 0x68; break;
	case 0x68: out = 0x01; break;
	case 0x48: out = 0x77; break;
	case 0x60: out = 0xFF; break;
	case 0x77: out = 0x48; break;
	case 0xB9: out = 0xC0; break;
	case 0xC0: out = 0xB9; break;
	case 0xFE: out = 0xEB; break;
	case 0xEB: out = 0xFE; break;
	case 0x80: out = 0x6C; break;
	case 0xFF: out = 0x60; break;
	default:   out = in;   break;
	}

	return out;
}

#if 0 /* this is not used anywhere, is it ok to delete?  */
static void grf_shuffle_enc(struct des_bit64 *src)
{
	struct des_bit64 out;

	nullpo_retv(src);
	out.b[0] = src->b[3];
	out.b[1] = src->b[4];
	out.b[2] = src->b[5];
	out.b[3] = src->b[0];
	out.b[4] = src->b[1];
	out.b[5] = src->b[6];
	out.b[6] = src->b[2];
	out.b[7] = grf_substitution(src->b[7]);

	*src = out;
}
#endif // 0

static void grf_shuffle_dec(struct des_bit64 *src)
{
	struct des_bit64 out;

	nullpo_retv(src);
	out.b[0] = src->b[3];
	out.b[1] = src->b[4];
	out.b[2] = src->b[6];
	out.b[3] = src->b[0];
	out.b[4] = src->b[1];
	out.b[5] = src->b[2];
	out.b[6] = src->b[5];
	out.b[7] = grf_substitution(src->b[7]);

	*src = out;
}

/**
 * Decodes header-encrypted grf data.
 *
 * @param[in,out] buf   Data to decode (in-place).
 * @param[in]     len   Length of the data.
 */
static void grf_decode_header(unsigned char *buf, size_t len)
{
	struct des_bit64 *p = (struct des_bit64 *)buf;
	size_t nblocks = len / sizeof(struct des_bit64);
	size_t i;
	nullpo_retv(buf);

	// first 20 blocks are all des-encrypted
	for (i = 0; i < 20 && i < nblocks; ++i)
		des->decrypt_block(&p[i]);

	// the rest is plaintext, done.
}

/**
 * Decodes fully encrypted grf data
 *
 * @param[in,out] buf   Data to decode (in-place).
 * @param[in]     len   Length of the data.
 * @param[in]     cycle The current decoding cycle.
 */
static void grf_decode_full(unsigned char *buf, size_t len, int cycle)
{
	struct des_bit64 *p = (struct des_bit64 *)buf;
	size_t nblocks = len / sizeof(struct des_bit64);
	int dcycle, scycle;
	size_t i, j;

	nullpo_retv(buf);
	// first 20 blocks are all des-encrypted
	for (i = 0; i < 20 && i < nblocks; ++i)
		des->decrypt_block(&p[i]);

	// after that only one of every 'dcycle' blocks is des-encrypted
	dcycle = cycle;

	// and one of every 'scycle' plaintext blocks is shuffled (starting from the 0th but skipping the 0th)
	scycle = 7;

	// so decrypt/de-shuffle periodically
	j = (size_t)-1; // 0, adjusted to fit the ++j step
	for (i = 20; i < nblocks; ++i) {
		if (i % dcycle == 0) {
			// decrypt block
			des->decrypt_block(&p[i]);
			continue;
		}

		++j;
		if (j % scycle == 0 && j != 0) {
			// de-shuffle block
			grf_shuffle_dec(&p[i]);
			continue;
		}

		// plaintext, do nothing.
	}
}

/**
 * Decodes grf data.
 *
 * @param[in,out] buf        Data to decode (in-place).
 * @param[in]     len        Length of the data
 * @param[in]     entry_type Flags associated with the data.
 * @param[in]     entry_len  True (unaligned) length of the data.
 */
static void grf_decode(unsigned char *buf, size_t len, char entry_type, int entry_len)
{
	if (entry_type & FILELIST_TYPE_ENCRYPT_MIXED) {
		// fully encrypted
		int digits;
		int cycle;
		int i;

		// compute number of digits of the entry length
		digits = 1;
		for (i = 10; i <= entry_len; i *= 10)
			++digits;

		// choose size of gap between two encrypted blocks
		// digits:  0  1  2  3  4  5  6  7  8  9 ...
		//  cycle:  1  1  1  4  5 14 15 22 23 24 ...
		cycle = ( digits < 3 ) ? 1
		      : ( digits < 5 ) ? digits + 1
		      : ( digits < 7 ) ? digits + 9
		      :                  digits + 15;

		grf_decode_full(buf, len, cycle);
	} else if (entry_type & FILELIST_TYPE_ENCRYPT_HEADER) {
		// header encrypted
		grf_decode_header(buf, len);
	}
	/* else plaintext */
}

/* Zlib Subroutines */

/// @copydoc grfio_interface::crc32()
unsigned long grfio_crc32(const unsigned char *buf, unsigned int len)
{
	return crc32(crc32(0L, Z_NULL, 0), buf, len);
}

/// @copydoc grfio_interface::decode_zip
int grfio_decode_zip(void *dest, unsigned long *dest_len, const void *source, unsigned long source_len)
{
	return uncompress(dest, dest_len, source, source_len);
}

/// @copydoc grfio_interface::encode_zip
int grfio_encode_zip(void *dest, unsigned long *dest_len, const void *source, unsigned long source_len)
{
	if (*dest_len == 0) /* [Ind/Hercules] */
		*dest_len = compressBound(source_len);
	if (dest == NULL) {
		/* [Ind/Hercules] */
		CREATE(dest, unsigned char, *dest_len);
	}
	return compress(dest, dest_len, source, source_len);
}

/* File List Subroutines */

/// File list hash table
int filelist_hash[256];

/**
 * Initializes the table that holds the first elements of all hash chains
 */
static void hashinit(void)
{
	int i;
	for (i = 0; i < 256; i++)
		filelist_hash[i] = -1;
}

/**
 * Hashes a filename string into a number from {0..255}
 *
 * @param fname The filename to hash.
 * @return The hash.
 */
static int grf_filehash(const char *fname)
{
	uint32 hash = 0;
	nullpo_ret(fname);
	while (*fname != '\0') {
		hash = (hash<<1) + (hash>>7)*9 + TOLOWER(*fname);
		fname++;
	}
	return hash & 255;
}

/**
 * Finds a grf_filelist entry with the specified file name
 *
 * @param fname The file to find.
 * @return The file entry.
 * @retval NULL if the file wasn't found.
 */
static struct grf_filelist *grfio_filelist_find(const char *fname)
{
	int hash, index;

	if (filelist == NULL)
		return NULL;

	hash = filelist_hash[grf_filehash(fname)];
	for (index = hash; index != -1; index = filelist[index].next)
		if(!strcmpi(filelist[index].fn, fname))
			break;

	return (index >= 0) ? &filelist[index] : NULL;
}

/// @copydoc grfio_interface::find_file()
const char *grfio_find_file(const char *fname)
{
	struct grf_filelist *flist = grfio_filelist_find(fname);
	if (flist == NULL)
		return NULL;
	return (!flist->fnd ? flist->fn : flist->fnd);
}

/**
 * Adds a grf_filelist entry into the list of loaded files.
 *
 * @param entry The entry to add.
 * @return A pointer to the added entry.
 */
static struct grf_filelist *grfio_filelist_add(struct grf_filelist *entry)
{
	int hash;
	nullpo_ret(entry);
#ifdef __clang_analyzer__
	// Make clang's static analyzer shut up about a possible NULL pointer in &filelist[filelist_entrys]
	nullpo_ret(&filelist[filelist_entrys]);
#endif // __clang_analyzer__

#define FILELIST_ADDS 1024 // number increment of file lists `

	if (filelist_entrys >= filelist_maxentry) {
		filelist = aRealloc(filelist, (filelist_maxentry + FILELIST_ADDS) * sizeof(struct grf_filelist));
		memset(filelist + filelist_maxentry, '\0', FILELIST_ADDS * sizeof(struct grf_filelist));
		filelist_maxentry += FILELIST_ADDS;
	}

#undef FILELIST_ADDS

	memcpy(&filelist[filelist_entrys], entry, sizeof(struct grf_filelist));

	hash = grf_filehash(entry->fn);
	filelist[filelist_entrys].next = filelist_hash[hash];
	filelist_hash[hash] = filelist_entrys;

	filelist_entrys++;

	return &filelist[filelist_entrys - 1];
}

/**
 * Adds a new grf_filelist entry or overwrites an existing one.
 *
 * @param entry The entry to add.
 * @return A pointer to the added entry.
 */
static struct grf_filelist *grfio_filelist_modify(struct grf_filelist *entry)
{
	struct grf_filelist *fentry;
	nullpo_retr(NULL, entry);
	fentry = grfio_filelist_find(entry->fn);
	if (fentry != NULL) {
		int tmp = fentry->next;
		memcpy(fentry, entry, sizeof(struct grf_filelist));
		fentry->next = tmp;
	} else {
		fentry = grfio_filelist_add(entry);
	}
	return fentry;
}

/// Shrinks the file list array if too long.
static void grfio_filelist_compact(void)
{
	if (filelist == NULL)
		return;

	if (filelist_entrys < filelist_maxentry) {
		filelist = aRealloc(filelist, filelist_entrys * sizeof(struct grf_filelist));
		filelist_maxentry = filelist_entrys;
	}
}

/* GRF I/O Subroutines */

/**
 * Combines a resource path with the data folder location to create local
 * resource path.
 *
 * @param[out] buffer   The output buffer.
 * @param[in]  size     The size of the output buffer.
 * @param[in]  filename Resource path.
 */
static void grfio_localpath_create(char *buffer, size_t size, const char *filename)
{
	int i;
	size_t len;

	nullpo_retv(buffer);
	len = strlen(data_dir);

	if (data_dir[0] == '\0' || data_dir[len-1] == '/' || data_dir[len-1] == '\\')
		safesnprintf(buffer, size, "%s%s", data_dir, filename);
	else
		safesnprintf(buffer, size, "%s/%s", data_dir, filename);

	// normalize path
	for (i = 0; buffer[i] != '\0'; ++i)
		if (buffer[i] == '\\')
			buffer[i] = '/';
}

/// @copydoc grfio_interface::reads()
void *grfio_reads(const char *fname, int *size)
{
	struct grf_filelist *entry = grfio_filelist_find(fname);
	if (entry == NULL || entry->gentry <= 0) {
		// LocalFileCheck
		char lfname[256];
		FILE *in;
		grfio_localpath_create(lfname, sizeof(lfname), (entry && entry->fnd) ? entry->fnd : fname);

		in = fopen(lfname, "rb");
		if (in != NULL) {
			int declen;
			unsigned char *buf = NULL;
			fseek(in,0,SEEK_END);
			declen = (int)ftell(in);
			if (declen == -1) {
				ShowError("An error occurred in fread grfio_reads, fname=%s \n",fname);
				fclose(in);
				return NULL;
			}
			fseek(in,0,SEEK_SET);
			buf = aMalloc(declen+1); // +1 for resnametable zero-termination
			buf[declen] = '\0';
			if (fread(buf, 1, declen, in) != (size_t)declen) {
				ShowError("An error occurred in fread grfio_reads, fname=%s \n",fname);
				aFree(buf);
				fclose(in);
				return NULL;
			}
			fclose(in);

			if (size)
				*size = declen;
			return buf;
		}

		if (entry == NULL || entry->gentry >= 0) {
			ShowError("grfio_reads: %s not found (local file: %s)\n", fname, lfname);
			return NULL;
		}

		entry->gentry = -entry->gentry; // local file checked
	}

	if (entry != NULL && entry->gentry > 0) {
		// Archive[GRF] File Read
		char *grfname = gentry_table[entry->gentry - 1];
		FILE *in = fopen(grfname, "rb");

		if (in != NULL) {
			int fsize = entry->srclen_aligned;
			unsigned char *buf = aMalloc(fsize);
			unsigned char *buf2 = NULL;
			if (fseek(in, entry->srcpos, SEEK_SET) != 0
			 || fread(buf, 1, fsize, in) != (size_t)fsize) {
				ShowError("An error occurred in fread in grfio_reads, grfname=%s\n",grfname);
				aFree(buf);
				fclose(in);
				return NULL;
			}
			fclose(in);

			buf2 = aMalloc(entry->declen+1);  // +1 for resnametable zero-termination
			buf2[entry->declen] = '\0';
			if (entry->type & FILELIST_TYPE_FILE) {
				// file
				uLongf len;
				grf_decode(buf, fsize, entry->type, entry->srclen);
				len = entry->declen;
				grfio->decode_zip(buf2, &len, buf, entry->srclen);
				if (len != (uLong)entry->declen) {
					ShowError("decode_zip size mismatch err: %d != %d\n", (int)len, entry->declen);
					aFree(buf);
					aFree(buf2);
					return NULL;
				}
			} else {
				// directory?
				memcpy(buf2, buf, entry->declen);
			}

			if (size)
				*size = entry->declen;

			aFree(buf);
			return buf2;
		} else {
			ShowError("grfio_reads: %s not found (GRF file: %s)\n", fname, grfname);
			return NULL;
		}
	}

	return NULL;
}

/**
 * Decodes encrypted filename from a version 01xx grf index.
 *
 * @param[in,out] buf The encrypted filename (decrypted in-place).
 * @param[in]     len The filename length.
 * @return A pointer to the decrypted filename.
 */
static char *grfio_decode_filename(unsigned char *buf, int len)
{
	int i;
	nullpo_retr(NULL, buf);
	for (i = 0; i < len; i += 8) {
		NibbleSwap(&buf[i],8);
		des->decrypt(&buf[i],8);
	}
	return (char*)buf;
}

/**
 * Compares file extension against known large file types.
 *
 * @param fname The file name.
 * @return true if the file should undergo full mode 0 decryption, and true otherwise.
 */
static bool grfio_is_full_encrypt(const char *fname)
{
	const char *ext;
	nullpo_retr(false, fname);
	ext = strrchr(fname, '.');
	if (ext != NULL) {
		static const char *extensions[] = { ".gnd", ".gat", ".act", ".str" };
		int i;
		for (i = 0; i < ARRAYLENGTH(extensions); ++i)
			if (strcmpi(ext, extensions[i]) == 0)
				return false;
	}

	return true;
}

/**
 * Loads all entries in the specified grf file into the filelist.
 *
 * @param grfname Name of the grf file.
 * @param gentry  Index of the grf file name in the gentry_table.
 * @return Error code.
 * @retval 0 in case of success.
 */
static int grfio_entryread(const char *grfname, int gentry)
{
	long grf_size;
	unsigned char grf_header[0x2e] = { 0 };
	int entry,entrys,ofs,grf_version;
	unsigned char *grf_filelist;
	FILE *fp;

	nullpo_retr(1, grfname);
	fp = fopen(grfname, "rb");
	if (fp == NULL) {
		ShowWarning("GRF data file not found: '%s'\n", grfname);
		return 1; // 1:not found error
	}
	ShowInfo("GRF data file found: '%s'\n", grfname);

	fseek(fp,0,SEEK_END);
	grf_size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	if (fread(grf_header,1,0x2e,fp) != 0x2e) {
		ShowError("Couldn't read all grf_header element of %s \n", grfname);
		fclose(fp);
		return 2; // 2:file format error
	}
	if (strcmp((const char*)grf_header, "Master of Magic") != 0 || fseek(fp, getlong(grf_header+0x1e), SEEK_CUR) != 0) {
		fclose(fp);
		ShowError("GRF %s read error\n", grfname);
		return 2; // 2:file format error
	}

	grf_version = getlong(grf_header+0x2a) >> 8;

	if (grf_version == 0x01) {
		// ****** Grf version 01xx ******
		long list_size = grf_size - ftell(fp);
		grf_filelist = aMalloc(list_size);
		if (fread(grf_filelist,1,list_size,fp) != (size_t)list_size) {
			ShowError("Couldn't read all grf_filelist element of %s \n", grfname);
			aFree(grf_filelist);
			fclose(fp);
			return 2; // 2:file format error
		}
		fclose(fp);

		entrys = getlong(grf_header+0x26) - getlong(grf_header+0x22) - 7;

		// Get an entry
		for (entry = 0, ofs = 0; entry < entrys; ++entry) {
			struct grf_filelist aentry = { 0 };
			int ofs2 = ofs+getlong(grf_filelist+ofs)+4;
			unsigned char type = grf_filelist[ofs2+12];
			if (type&FILELIST_TYPE_FILE) {
				char *fname = grfio_decode_filename(grf_filelist+ofs+6, grf_filelist[ofs]-6);
				int srclen = getlong(grf_filelist+ofs2+0) - getlong(grf_filelist+ofs2+8) - 715;

				if (strlen(fname) > sizeof(aentry.fn) - 1) {
					ShowFatalError("GRF file name %s is too long\n", fname);
					aFree(grf_filelist);
					return 5; // 5: file name too long
				}

				type |= grfio_is_full_encrypt(fname) ? FILELIST_TYPE_ENCRYPT_MIXED : FILELIST_TYPE_ENCRYPT_HEADER;

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4)-37579;
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.type           = type;
				safestrncpy(aentry.fn, fname, sizeof(aentry.fn));
				aentry.fnd            = NULL;
#ifdef GRFIO_LOCAL
				aentry.gentry         = -(gentry+1); // As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = (char)(gentry+1); // With no first time LocalFileCheck
#endif
				grfio_filelist_modify(&aentry);
			}

			ofs = ofs2 + 17;
		}

		aFree(grf_filelist);
	} else if (grf_version == 0x02) {
		// ****** Grf version 02xx ******
		unsigned char eheader[8];
		unsigned char *rBuf;
		uLongf rSize, eSize;

		if (fread(eheader,1,8,fp) != 8) {
			ShowError("An error occurred in fread while reading header buffer\n");
			fclose(fp);
			return 4;
		}
		rSize = getlong(eheader); // Read Size
		eSize = getlong(eheader+4); // Extend Size

		if ((long)rSize > grf_size-ftell(fp)) {
			fclose(fp);
			ShowError("Illegal data format: GRF compress entry size\n");
			return 4;
		}

		rBuf = aMalloc(rSize); // Get a Read Size
		if (fread(rBuf,1,rSize,fp) != rSize) {
			ShowError("An error occurred in fread \n");
			fclose(fp);
			aFree(rBuf);
			return 4;
		}
		fclose(fp);
		grf_filelist = aMalloc(eSize); // Get a Extend Size
		grfio->decode_zip(grf_filelist, &eSize, rBuf, rSize); // Decode function
		aFree(rBuf);

		entrys = getlong(grf_header+0x26) - 7;
		Assert_retr(4, entrys >= 0);

		// Get an entry
		for (entry = 0, ofs = 0; entry < entrys; ++entry) {
			struct grf_filelist aentry;
			char *fname = (char*)(grf_filelist+ofs);
			int ofs2 = ofs + (int)strlen(fname)+1;
			int type = grf_filelist[ofs2+12];

			if (strlen(fname) > sizeof(aentry.fn)-1) {
				ShowFatalError("GRF file name %s is too long\n", fname);
				aFree(grf_filelist);
				return 5; // 5: file name too long
			}

			if (type&FILELIST_TYPE_FILE) {
				// file
				aentry.srclen         = getlong(grf_filelist+ofs2+0);
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4);
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.type           = (char)type;
				safestrncpy(aentry.fn, fname, sizeof(aentry.fn));
				aentry.fnd            = NULL;
#ifdef GRFIO_LOCAL
				aentry.gentry         = -(gentry+1); // As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = (char)(gentry+1); // With no first time LocalFileCheck
#endif
				grfio_filelist_modify(&aentry);
			}

			ofs = ofs2 + 17;
		}

		aFree(grf_filelist);
	} else {
		// ****** Grf Other version ******
		fclose(fp);
		ShowError("GRF version %04x not supported\n",getlong(grf_header+0x2a));
		return 4;
	}

	grfio_filelist_compact(); // Unnecessary area release of filelist

	return 0; // 0:no error
}

/**
 * Parses a Resource file table row.
 *
 * @param row The row data.
 * @return success state.
 * @retval true in case of success.
 */
static bool grfio_parse_restable_row(const char *row)
{
	char w1[256], w2[256];
	char src[256], dst[256];
	char local[256];
	struct grf_filelist *entry = NULL;

	nullpo_retr(false, row);
	if (sscanf(row, "%255[^#\r\n]#%255[^#\r\n]#", w1, w2) != 2)
		return false;

	if (strstr(w2, ".gat") == NULL && strstr(w2, ".rsw") == NULL)
		return false; // we only need the maps' GAT and RSW files

	sprintf(src, "data\\%s", w1);
	sprintf(dst, "data\\%s", w2);

	entry = grfio_filelist_find(dst);
	if (entry != NULL) {
		// alias for GRF resource
		struct grf_filelist fentry = *entry;
		safestrncpy(fentry.fn, src, sizeof(fentry.fn));
		fentry.fnd = aStrdup(dst);
		grfio_filelist_modify(&fentry);
		return true;
	}

	grfio_localpath_create(local, sizeof(local), dst);
	if (exists(local)) {
		// alias for local resource
		struct grf_filelist fentry = { 0 };
		safestrncpy(fentry.fn, src, sizeof(fentry.fn));
		fentry.fnd = aStrdup(dst);
		grfio_filelist_modify(&fentry);
		return true;
	}

	return false;
}

/**
 * Grfio Resource file check.
 */
static void grfio_resourcecheck(void)
{
	char restable[256];
	char *buf = NULL;
	FILE *fp = NULL;
	int size = 0, i = 0;

	// read resnametable from data directory and return if successful
	grfio_localpath_create(restable, sizeof(restable), "data\\resnametable.txt");

	fp = fopen(restable, "rb");
	if (fp != NULL) {
		char line[256];
		while (fgets(line, sizeof(line), fp)) {
			if (grfio_parse_restable_row(line))
				++i;
		}

		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, "resnametable.txt");
		return; // we're done here!
	}

	// read resnametable from loaded GRF's, only if it cannot be loaded from the data directory
	buf = grfio->reads("data\\resnametable.txt", &size);
	if (buf != NULL) {
		char *ptr = NULL;
		buf[size] = '\0';

		ptr = buf;
		while (ptr - buf < size) {
			if (grfio_parse_restable_row(ptr))
				++i;

			ptr = strchr(ptr, '\n');
			if (ptr == NULL)
				break;
			ptr++;
		}

		aFree(buf);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, "data\\resnametable.txt");
		return;
	}
}

/**
 * Reads a grf file and adds it to the list.
 *
 * @param fname The file name to read.
 * @return Error code.
 * @retval 0 in case of success.
 */
static int grfio_add(const char *fname)
{
	nullpo_retr(1, fname);
	if (gentry_entrys >= gentry_maxentry) {
#define GENTRY_ADDS 4 // The number increment of gentry_table entries
		gentry_maxentry += GENTRY_ADDS;
		gentry_table = aRealloc(gentry_table, gentry_maxentry * sizeof(char*));
		memset(gentry_table + (gentry_maxentry - GENTRY_ADDS), 0, sizeof(char*) * GENTRY_ADDS);
#undef GENTRY_ADDS
	}

	gentry_table[gentry_entrys++] = aStrdup(fname);

	return grfio_entryread(fname, gentry_entrys - 1);
}

/// @copydoc grfio_interface::final()
void grfio_final(void)
{
	if (filelist != NULL) {
		int i;
		for (i = 0; i < filelist_entrys; i++)
			if (filelist[i].fnd != NULL)
				aFree(filelist[i].fnd);

		aFree(filelist);
		filelist = NULL;
	}
	filelist_entrys = filelist_maxentry = 0;

	if (gentry_table != NULL) {
		int i;
		for (i = 0; i < gentry_entrys; i++)
			if (gentry_table[i] != NULL)
				aFree(gentry_table[i]);

		aFree(gentry_table);
		gentry_table = NULL;
	}
	gentry_entrys = gentry_maxentry = 0;
}

/// @copydoc grfio_interface::init()
void grfio_init(const char *fname)
{
	FILE *data_conf;
	int grf_num = 0;

	nullpo_retv(fname);
	hashinit(); // hash table initialization

	data_conf = fopen(fname, "r");
	if (data_conf != NULL) {
		char line[1024];
		while (fgets(line, sizeof(line), data_conf)) {
			char w1[1024], w2[1024];

			if (line[0] == '/' && line[1] == '/')
				continue; // skip comments

			if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
				continue; // skip unrecognized lines

			// Entry table reading
			if (strcmp(w1, "grf") == 0) {
				// GRF file
				if (grfio_add(w2) == 0)
					++grf_num;
			} else if (strcmp(w1,"data_dir") == 0) {
				// Data directory
				safestrncpy(data_dir, w2, sizeof(data_dir));
			}
		}

		fclose(data_conf);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", fname);
	}

	if (grf_num == 0)
		ShowInfo("No GRF loaded, using default data directory\n");

	// Unnecessary area release of filelist
	grfio_filelist_compact();

	// Resource check
	grfio_resourcecheck();
}

/// Interface base initialization.
void grfio_defaults(void)
{
	grfio = &grfio_s;
	grfio->init = grfio_init;
	grfio->final = grfio_final;
	grfio->reads = grfio_reads;
	grfio->find_file = grfio_find_file;
	grfio->crc32 = grfio_crc32;
	grfio->decode_zip = grfio_decode_zip;
	grfio->encode_zip = grfio_encode_zip;
}
