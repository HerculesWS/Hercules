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
#ifndef COMMON_GRFIO_H
#define COMMON_GRFIO_H

#include "common/hercules.h"

/** @file
 * GRF I/O library.
 */

/// The GRF I/O interface.
struct grfio_interface {
	/**
	 * Interface initialization.
	 *
	 * @param fname Name of the configuration file.
	 */
	void (*init) (const char *fname);

	/// Interface finalization.
	void (*final) (void);

	/**
	 * Reads a file into a newly allocated buffer (from grf or data directory).
	 *
	 * @param[in]  fname Name of the file to read.
	 * @param[out] size  Buffer to return the size of the read file (optional).
	 * @return The file data.
	 * @retval NULL in case of error.
	 */
	void *(*reads) (const char *fname, int *size);

	/**
	 * Finds a file in the grf or data directory
	 *
	 * @param fname The file to find.
	 * @return The original file name.
	 * @retval NULL if the file wasn't found.
	 */
	const char *(*find_file) (const char *fname);

	/**
	 * Calculates a CRC32 hash.
	 *
	 * @param buf The data to hash.
	 * @param len Data length.
	 *
	 * @return The CRC32 hash.
	 */
	unsigned long (*crc32) (const unsigned char *buf, unsigned int len);

	/**
	 * Decompresses ZIP data.
	 *
	 * Decompresses the source buffer into the destination buffer.
	 * source_len is the byte length of the source buffer.  Upon entry,
	 * dest_len is the total size of the destination buffer, which must be
	 * large enough to hold the entire uncompressed data.  (The size of the
	 * uncompressed data must have been saved previously by the compressor
	 * and transmitted to the decompressor by some mechanism outside the
	 * scope of this compression library.) Upon exit, dest_len is the
	 * actual size of the uncompressed buffer.
	 *
	 * @param[in,out] dest       The destination (uncompressed) buffer.
	 * @param[in,out] dest_len   Max length of the destination buffer, returns length of the decompressed data.
	 * @param[in]     source     The source (compressed) buffer.
	 * @param[in]     source_len Source data length.
	 * @return error code.
	 * @retval Z_OK in case of success.
	 */
	int (*decode_zip) (void *dest, unsigned long *dest_len, const void *source, unsigned long source_len);

	/**
	 * Compresses data to ZIP format.
	 *
	 * Compresses the source buffer into the destination buffer.
	 * source_len is the byte length of the source buffer.  Upon entry,
	 * dest_len is the total size of the destination buffer, which must be
	 * at least the value returned by compressBound(source_len).  Upon
	 * exit, dest_len is the actual size of the compressed buffer.
	 *
	 * @param[in,out] dest       The destination (compressed) buffer (if NULL, a new buffer will be created).
	 * @param[in,out] dest_len   Max length of the destination buffer (if 0, it will be calculated).
	 * @param[in]     source     The source (uncompressed) buffer.
	 * @param[in]     source_len Source data length.
	 */
	int (*encode_zip) (void *dest, unsigned long *dest_len, const void *source, unsigned long source_len);
};

/**
 * Reads a file into a newly allocated buffer (from grf or data directory)
 *
 * @param fn The filename to read.
 *
 * @see grfio_interface::reads()
 * @related grfio_interface
 */
#define grfio_read(fn) grfio->reads((fn), NULL)

#ifdef HERCULES_CORE
void grfio_defaults(void);
#endif // HERCULES_CORE

HPShared struct grfio_interface *grfio; ///< Pointer to the grfio interface.
#endif /* COMMON_GRFIO_H */
