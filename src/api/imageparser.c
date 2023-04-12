/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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

#include "config/core.h"
#include "api/imageparser.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/extraconf.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/socket.h"

#include <stdlib.h>

#define DEBUG_ERRORS

static struct imageparser_interface imageparser_s;
struct imageparser_interface *imageparser;

static int do_init_imageparser(bool minimal)
{
	return 0;
}

static void do_final_imageparser(void)
{
}

static bool imageparser_validate_bmp_emblem(const char *emblem, uint64 emblem_len)
{
	nullpo_retr(false, emblem);

	// basic check for bmp format
	if (emblem_len < 10 || strncmp(emblem, "BM", 2) != 0)
		return false;

	const uint8 *buf = (const uint8 *)emblem;

	enum e_bitmapconst {
		RGBTRIPLE_SIZE = 3,         // sizeof(RGBTRIPLE)
		RGBQUAD_SIZE = 4,           // sizeof(RGBQUAD)
		BITMAPFILEHEADER_SIZE = 14, // sizeof(BITMAPFILEHEADER)
		BITMAPINFOHEADER_SIZE = 40, // sizeof(BITMAPINFOHEADER)
	};
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris
	struct s_bitmaptripple {
		//uint8 b;
		//uint8 g;
		//uint8 r;
		uint32 rgb:24;
	} __attribute__((packed));
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris
	if (emblem_len > extraconf->emblems->max_bmp_guild_emblem_size
	 || extraconf->emblems->max_bmp_guild_emblem_size < BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE
	 || RBUFW(buf, 0) != 0x4d42 // BITMAPFILEHEADER.bfType (signature)
	 || RBUFL(buf, 2) != emblem_len // BITMAPFILEHEADER.bfSize (file size)
	 || RBUFL(buf, 14) != BITMAPINFOHEADER_SIZE // BITMAPINFOHEADER.biSize (other headers are not supported)
	 || RBUFL(buf, 18) != extraconf->emblems->guild_emblem_width // BITMAPINFOHEADER.biWidth
	 || RBUFL(buf, 22) != extraconf->emblems->guild_emblem_height // BITMAPINFOHEADER.biHeight (top-down bitmaps (-24) are not supported)
	 || RBUFL(buf, 30) != 0 // BITMAPINFOHEADER.biCompression == BI_RGB (compression not supported)
	 ) {
		// Invalid data
		return false;
	}

	const int offbits = RBUFL(buf, 10); // BITMAPFILEHEADER.bfOffBits (offset to bitmap bits)
	int header = 0;
	int bitmap = 0;
	int palettesize = 0;

	switch (RBUFW(buf, 28)) { // BITMAPINFOHEADER.biBitCount
		case 8:
			palettesize = RBUFL(buf, 46); // BITMAPINFOHEADER.biClrUsed (number of colors in the palette)
			if (palettesize == 0)
				palettesize = 256; // Defaults to 2^n if set to zero
			else if (palettesize > 256)
				return false;
			header = BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE + RGBQUAD_SIZE * palettesize; // headers + palette
			bitmap = extraconf->emblems->guild_emblem_width * extraconf->emblems->guild_emblem_height;
			break;
		case 24:
			header = BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE;
			bitmap = extraconf->emblems->guild_emblem_width * extraconf->emblems->guild_emblem_height * RGBTRIPLE_SIZE;
			break;
		default:
			return false;
	}

	// NOTE: This check gives a little freedom for bitmap-producing implementations,
	// that align the start of bitmap data, which is harmless but unnecessary.
	// If you want it paranoidly strict, change the first condition from < to !=.
	// This also allows files with trailing garbage at the end of the file.
	// If you want to avoid that, change the last condition to !=.
	if (offbits < header || extraconf->emblems->max_bmp_guild_emblem_size < bitmap || offbits > extraconf->emblems->max_bmp_guild_emblem_size - bitmap) {
		return false;
	}

	return true;
}

// emulate fread like function
static int imageparser_read_gif_func(GifFileType *gif, GifByteType *buf, int len)
{
	nullpo_ret(gif);
	nullpo_ret(buf);

	if (len < 0)
		return 0;
	struct gif_user_data *userData = gif->UserData;
	nullpo_ret(userData);
	const uint64 read_pos = userData->read_pos;
	const uint64 emblem_len = userData->emblem_len;
	if (read_pos >= emblem_len)
		return 0;
	// for real fread here need return possible number of bytes to read
	if (read_pos + len > emblem_len)
		return 0;
	memcpy(buf, userData->emblem + read_pos, len);
	userData->read_pos += len;
	return len;
}

static bool imageparser_validate_gif_emblem(const char *emblem, uint64 emblem_len)
{
	nullpo_retr(false, emblem);

	if (emblem_len > extraconf->emblems->max_gif_guild_emblem_size) {
#ifdef DEBUG_ERRORS
		ShowError("Error: gif image file size too big: %"PRIu64"\n", emblem_len);
#endif
		return false;
	}

	// basic check for gif format
	if (emblem_len < 10 ||
	    strncmp(emblem, "GIF", 3) != 0 ||
	    (memcmp(emblem + 3, "87a", 3) != 0 && memcmp(emblem + 3, "89a", 3) != 0)) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Unknown gif image header\n");
#endif
		return false;
	}

	int error = D_GIF_SUCCEEDED;
	struct gif_user_data userData;
	userData.emblem = emblem;
	userData.emblem_len = emblem_len;
	userData.read_pos = 0;

	GifFileType *image = DGifOpen(&userData, imageparser->read_gif_func, &error);
	if (image == NULL) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif open error\n");
#endif
		return false;
	}
	const int ret = DGifSlurp(image);
	if (ret != GIF_OK) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif parse error\n");
#endif
		DGifCloseFile(image, &error);
		return false;
	}

// last frame resolution check disabled because client support other frames resolution [4144]
/*
	if (image->Image.Width != extraconf->emblems->guild_emblem_width ||
	    image->Image.Height != extraconf->emblems->guild_emblem_height) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif image resolution error: %d, %d\n", image->Image.Width, image->Image.Height);
#endif
		DGifCloseFile(image, &error);
		return false;
	}
*/
	// check image resolution and images count
	if (image->SWidth != extraconf->emblems->guild_emblem_width ||
	    image->SHeight != extraconf->emblems->guild_emblem_height) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif canvas resolution error: %d, %d\n", image->SWidth, image->SHeight);
#endif
		DGifCloseFile(image, &error);
		return false;
	}
	if (image->ImageCount < extraconf->emblems->min_guild_emblem_frames) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif frames count too small: %d\n", image->ImageCount);
#endif
		DGifCloseFile(image, &error);
		return false;
	}
	if (image->ImageCount > extraconf->emblems->max_guild_emblem_frames) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif frames count too big: %d\n", image->ImageCount);
#endif
		DGifCloseFile(image, &error);
		return false;
	}
	DGifCloseFile(image, &error);

	// check used bytes from image buffer
	if (userData.read_pos != emblem_len) {
#ifdef DEBUG_ERRORS
		ShowError("Error: Gif extra bytes found\n");
#endif
		return false;
	}
	return true;
}

void imageparser_defaults(void)
{
	imageparser = &imageparser_s;
	imageparser->init = do_init_imageparser;
	imageparser->final = do_final_imageparser;

	imageparser->validate_bmp_emblem = imageparser_validate_bmp_emblem;
	imageparser->validate_gif_emblem = imageparser_validate_gif_emblem;
	imageparser->read_gif_func = imageparser_read_gif_func;
}
