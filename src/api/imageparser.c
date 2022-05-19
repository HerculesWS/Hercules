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
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/socket.h"

#include <stdlib.h>

static struct imageparser_interface imageparser_s;
struct imageparser_interface *imageparser;

static int do_init_imageparser(bool minimal)
{
	return 0;
}

static void do_final_imageparser(void)
{
}

static bool imageparser_validate_bmp_emblem(const char *emblem, unsigned long emblem_len)
{
	nullpo_retr(false, emblem);

	const uint8 *buf = (const uint8 *)emblem;

	enum e_bitmapconst {
		RGBTRIPLE_SIZE = 3,         // sizeof(RGBTRIPLE)
		RGBQUAD_SIZE = 4,           // sizeof(RGBQUAD)
		BITMAPFILEHEADER_SIZE = 14, // sizeof(BITMAPFILEHEADER)
		BITMAPINFOHEADER_SIZE = 40, // sizeof(BITMAPINFOHEADER)
		BITMAP_WIDTH = 24,
		BITMAP_HEIGHT = 24,
		MAX_BITMAP_SIZE = 1800
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
	if (emblem_len >= MAX_BITMAP_SIZE
	 || MAX_BITMAP_SIZE < BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE
	 || RBUFW(buf, 0) != 0x4d42 // BITMAPFILEHEADER.bfType (signature)
	 || RBUFL(buf, 2) != emblem_len // BITMAPFILEHEADER.bfSize (file size)
	 || RBUFL(buf, 14) != BITMAPINFOHEADER_SIZE // BITMAPINFOHEADER.biSize (other headers are not supported)
	 || RBUFL(buf, 18) != BITMAP_WIDTH // BITMAPINFOHEADER.biWidth
	 || RBUFL(buf, 22) != BITMAP_HEIGHT // BITMAPINFOHEADER.biHeight (top-down bitmaps (-24) are not supported)
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
			bitmap = BITMAP_WIDTH * BITMAP_HEIGHT;
			break;
		case 24:
			header = BITMAPFILEHEADER_SIZE + BITMAPINFOHEADER_SIZE;
			bitmap = BITMAP_WIDTH * BITMAP_HEIGHT * RGBTRIPLE_SIZE;
			break;
		default:
			return false;
	}

	// NOTE: This check gives a little freedom for bitmap-producing implementations,
	// that align the start of bitmap data, which is harmless but unnecessary.
	// If you want it paranoidly strict, change the first condition from < to !=.
	// This also allows files with trailing garbage at the end of the file.
	// If you want to avoid that, change the last condition to !=.
	if (offbits < header || MAX_BITMAP_SIZE <= bitmap || offbits > MAX_BITMAP_SIZE - bitmap) {
		return false;
	}

	return true;
}

static bool imageparser_validate_gif_emblem(const char *emblem, unsigned long emblem_len)
{
	return true;
}

void imageparser_defaults(void)
{
	imageparser = &imageparser_s;
	imageparser->init = do_init_imageparser;
	imageparser->final = do_final_imageparser;

	imageparser->validate_bmp_emblem = imageparser_validate_bmp_emblem;
	imageparser->validate_gif_emblem = imageparser_validate_gif_emblem;
}
