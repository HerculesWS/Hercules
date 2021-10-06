/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2021 Hercules Dev Team
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
#ifndef MAP_MACRO_H
#define MAP_MACRO_H

#include "map/map.h"
#include "map/packets_struct.h"
#include "common/hercules.h"
#include "common/mmo.h"

#define CAPTCHA_BMP_SIZE (2 + 52 + (3 * 220 * 90)) // sizeof("BM") + sizeof(BITMAPV2INFOHEADER) + 24bits 220x90 BMP
#define CAPTCHA_REGISTERY_MAX_SIZE 0xFFF
#define MAX_CAPTCHA_CHUNK_SIZE 1024
#define SET_MACRO_BLOCK_ACTIONS(s, v) \
	do {                              \
		(s)->block_action.move =      \
		(s)->block_action.attack =    \
		(s)->block_action.skill =     \
		(s)->block_action.useitem =   \
		(s)->block_action.chat =      \
		(s)->block_action.immune =    \
		(s)->block_action.sitstand =  \
		(s)->block_action.commands =  \
		(s)->block_action.npc = (v);  \
	} while(0)

struct captcha_data {
	int upload_size;
	int image_size;
	char image_data[CAPTCHA_BMP_SIZE];
	char captcha_answer[16];
	char captcha_key[4];
};

struct macro_detect {
	const struct captcha_data *cd;
	int reporter_aid;
	int retry;
	int timer;
};

VECTOR_STRUCT_DECL(macroaidlist, int);

enum macro_detect_status {
	MCD_TIMEOUT = 0,
	MCD_INCORRECT = 1,
	MCD_GOOD = 2,
};

enum macro_report_status {
	MCR_MONITORING = 0,
	MCR_NO_DATA = 1,
	MCR_INPROGRESS = 2,
};

/**
 * macro.c Interface
 **/
struct macro_interface {
	/* vars */
	VECTOR_DECL(struct captcha_data) captcha_registery;

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);

	/* Captcha Register */
	void (*captcha_register) (struct map_session_data *sd, const int image_size, const char *captcha_answer);
	void (*captcha_register_upload) (struct map_session_data *sd, const char *captcha_key, const int upload_size, const char *upload_data);

	/* Captcha Preview */
	void (*captcha_preview) (struct map_session_data *sd, const int captcha_idx);

	/* Macro Detector */
	void (*detector_request) (struct map_session_data *sd);
	void (*detector_process_answer) (struct map_session_data *sd, const char *captcha_answer);
	int (*detector_timeout) (int tid, int64 tick, int id, intptr_t data);
	void (*detector_disconnect) (struct map_session_data *sd);

	/* Macro Reporter */
	void (*reporter_area_select) (struct map_session_data *sd, const int16 x, const int16 y, const int8 radius);
	int (*reporter_area_select_sub) (struct block_list *bl, va_list ap);
	void (*reporter_process) (struct map_session_data *ssd, struct map_session_data *tsd);

};

#ifdef HERCULES_CORE
void macro_defaults(void);
#endif // HERCULES_CORE

HPShared struct macro_interface *macro;

#endif /* MAP_MACRO_H */
