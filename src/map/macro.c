/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2021-2022 Hercules Dev Team
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

#include "macro.h"

#include "common/conf.h"
#include "common/db.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/pc.h"

#include <stdlib.h>

static struct macro_interface macro_s;
struct macro_interface *macro;
const char *macro_allowed_answer_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

static void macro_captcha_register(struct map_session_data *sd, const int image_size, const char *captcha_answer)
{
	nullpo_retv(sd);

	if (captcha_answer == NULL || strlen(captcha_answer) < 4
		|| (image_size < 0 || image_size > CAPTCHA_BMP_SIZE)
		|| VECTOR_LENGTH(macro->captcha_registery) >= CAPTCHA_REGISTERY_MAX_SIZE) {
		clif->captcha_upload_request(sd, "", 1); // Notify client of failure.
		return;
	}

	// Allocate a new captcha entry
	VECTOR_ENSURE(macro->captcha_registery, 1, 1);

	struct captcha_data cd = { 0 };
	cd.upload_size = 0;
	cd.image_size = image_size;
	safestrncpy(cd.captcha_answer, captcha_answer, sizeof(cd.captcha_answer));
	memset(cd.image_data, 0, CAPTCHA_BMP_SIZE);

	char captcha_key[50];
	snprintf(captcha_key, 50, "%X", (unsigned int)VECTOR_CAPACITY(macro->captcha_registery));
	safestrncpy(cd.captcha_key, captcha_key, sizeof(cd.captcha_key));

	VECTOR_PUSH(macro->captcha_registery, cd);

	// Request the image data from the client.
	clif->captcha_upload_request(sd, captcha_key, 0);
}

static void macro_captcha_register_upload(struct map_session_data *sd, const char *captcha_key, const int upload_size, const char *upload_data)
{
	nullpo_retv(sd);
	nullpo_retv(captcha_key);
	nullpo_retv(upload_data);
	Assert_retv(upload_size > 0 && upload_size <= MAX_CAPTCHA_CHUNK_SIZE);

	const int captcha_idx = (int)strtol(captcha_key, NULL, 16) - 1;
	Assert_retv(captcha_idx >= 0 && captcha_idx < VECTOR_LENGTH(macro->captcha_registery));

	struct captcha_data *cd = &VECTOR_INDEX(macro->captcha_registery, captcha_idx);
	Assert_retv((cd->upload_size + upload_size) <= cd->image_size);

	memcpy(&cd->image_data[cd->upload_size], upload_data, upload_size);
	cd->upload_size += upload_size;

	// Notify that the image finished uploading.
	if (cd->upload_size == cd->image_size)
		clif->captcha_upload_end(sd);
}

static void macro_captcha_preview(struct map_session_data *sd, const int captcha_idx)
{
	nullpo_retv(sd);

	// Send client error if captcha id is out of range.
	const int cr_len = VECTOR_LENGTH(macro->captcha_registery);
	if (cr_len == 0 || captcha_idx < 0 || captcha_idx > (cr_len - 1)) {
		clif->captcha_preview_request_init(sd, "", 0, 1);
		return;
	}

	const struct captcha_data *cd = &VECTOR_INDEX(macro->captcha_registery, captcha_idx);

	// Send preview initialization request to the client.
	clif->captcha_preview_request_init(sd, cd->captcha_key, cd->image_size, 0);

	// Send the image data in chunks.
	const int chunks = (cd->image_size / MAX_CAPTCHA_CHUNK_SIZE) +
						(cd->image_size % MAX_CAPTCHA_CHUNK_SIZE != 0);
	for (int i = 0, offset = 0; i < chunks; i++) {
		const int chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
		clif->captcha_preview_request_download(sd, cd->captcha_key, chunk_size, &cd->image_data[offset]);
		offset += chunk_size;
	}
}

static void macro_detector_request(struct map_session_data *sd)
{
	nullpo_retv(sd);

	const struct captcha_data *cd = sd->macro_detect.cd;
	nullpo_retv(cd);

	// Send preview initialization request to the client.
	clif->macro_detector_request_init(sd, cd->captcha_key, cd->image_size);

	// Send the image data in chunks.
	const int chunks = (cd->image_size / MAX_CAPTCHA_CHUNK_SIZE) +
						(cd->image_size % MAX_CAPTCHA_CHUNK_SIZE != 0);
	for (int i = 0, offset = 0; i < chunks; i++) {
		const int chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
		clif->macro_detector_request_download(sd, cd->captcha_key, chunk_size, &cd->image_data[offset]);
		offset += chunk_size;
	}
}

static int macro_detector_timeout(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = map->id2sd(id);
	nullpo_ret(sd);

	// Remove the current timer
	sd->macro_detect.timer = INVALID_TIMER;

	// Deduct an answering attempt
	sd->macro_detect.retry -= 1;

	if (sd->macro_detect.retry == 0) {
		// All attempts have been exhausted block the user
		clif->macro_detector_status(sd, MCD_TIMEOUT);
		chrif->char_ask_name(sd->macro_detect.reporter_aid, sd->status.name, CHAR_ASK_NAME_BLOCK, 0, 0, 0, 0, 0, 0);
	} else {
		// update the client
		clif->macro_detector_request_show(sd);

		// Start a new timer
		sd->macro_detect.timer = timer->add(timer->gettick() + battle->bc->macro_detect_timeout,
			macro->detector_timeout, sd->bl.id, 0);
	}
	return 0;
}

static void macro_detector_process_answer(struct map_session_data *sd, const char *captcha_answer)
{
	nullpo_retv(sd);

	const struct captcha_data *cd = sd->macro_detect.cd;

	// Correct answer
	if (captcha_answer != NULL && strcmp(captcha_answer, cd->captcha_answer) == 0) {
		// Delete the timer
		timer->delete(sd->macro_detect.timer, macro->detector_timeout);

		// Clear the macro detect data
		memset(&sd->macro_detect, 0, sizeof(sd->macro_detect));
		sd->macro_detect.timer = INVALID_TIMER;

		// Unblock all actions for the player
		SET_MACRO_BLOCK_ACTIONS(sd, 0);

		// Grant a small buff
		sc_start(NULL, &sd->bl, skill->get_sc_type(AL_INCAGI), 100, 10, 600000);
		sc_start(NULL, &sd->bl, skill->get_sc_type(AL_BLESSING), 100, 10, 600000);

		// Notify the client
		clif->macro_detector_status(sd, MCD_GOOD);
		return;
	}

	// Deduct an answering attempt
	sd->macro_detect.retry -= 1;

	// All attempts have been exhausted block the user
	if (sd->macro_detect.retry == 0) {
		clif->macro_detector_status(sd, MCD_INCORRECT);
		chrif->char_ask_name(sd->macro_detect.reporter_aid, sd->status.name, CHAR_ASK_NAME_BLOCK, 0, 0, 0, 0, 0, 0);
		return;
	}

	// Incorrect response, update the client
	clif->macro_detector_request_show(sd);

	// Reset the timer
	timer->settick(sd->macro_detect.timer, timer->gettick() + battle->bc->macro_detect_timeout);
}

static void macro_detector_disconnect(struct map_session_data *sd)
{
	nullpo_retv(sd);

	// Delete the timeout timer
	if (sd->macro_detect.timer != INVALID_TIMER) {
		timer->delete(sd->macro_detect.timer, macro->detector_timeout);
		sd->macro_detect.timer = INVALID_TIMER;
	}

	// If the player disconnects before clearing the challenge the account is banned.
	if (sd->macro_detect.retry != 0)
		chrif->char_ask_name(sd->macro_detect.reporter_aid, sd->status.name, CHAR_ASK_NAME_BLOCK, 0, 0, 0, 0, 0, 0);
}

static void macro_reporter_area_select(struct map_session_data *sd, const int16 x, const int16 y, const int8 radius)
{
	nullpo_retv(sd);

	struct macroaidlist aid_list;
	VECTOR_INIT(aid_list);

	map->foreachinarea(macro->reporter_area_select_sub, sd->bl.m,
					x - radius, y - radius, x + radius, y + radius,
					BL_PC, &aid_list);

	clif->macro_reporter_select(sd, &aid_list);
	VECTOR_CLEAR(aid_list);
}

static int macro_reporter_area_select_sub(struct block_list *bl, va_list ap)
{
	Assert_ret(bl->type == BL_PC);

	struct macroaidlist *aid_list = va_arg(ap, struct macroaidlist *);
	nullpo_ret(aid_list);

	VECTOR_ENSURE(*aid_list, 1, 1);
	VECTOR_PUSH(*aid_list, bl->id);
	return 0;
}

static void macro_reporter_process(struct map_session_data *ssd, struct map_session_data *tsd)
{
	nullpo_retv(ssd);
	nullpo_retv(tsd);
	Assert_retv(VECTOR_LENGTH(macro->captcha_registery) != 0);

	// pick a random image from the database.
	const int captcha_idx = rnd() % VECTOR_LENGTH(macro->captcha_registery);
	const struct captcha_data *cd = &VECTOR_INDEX(macro->captcha_registery, captcha_idx);

	// set macro detection data
	tsd->macro_detect.cd = cd;
	tsd->macro_detect.reporter_aid = ssd->status.account_id;
	tsd->macro_detect.retry = battle->bc->macro_detect_retry;

	// Block all actions for the target player
	SET_MACRO_BLOCK_ACTIONS(tsd, 1);

	// open macro detect client side
	macro->detector_request(tsd);

	// start the timeout timer.
	tsd->macro_detect.timer = timer->add(timer->gettick() + battle->bc->macro_detect_timeout,
										macro->detector_timeout, tsd->bl.id, 0);
}

static bool macro_read_captcha_db_libconfig(void)
{
	char filepath[280];
	snprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, "captcha_db.conf");

	struct config_t captcha_db_conf;
	if (libconfig->load_file(&captcha_db_conf, filepath) == CONFIG_FALSE) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		return false;
	}

	int i = 0;
	int count = 0;
	struct config_setting_t *it = NULL;
	struct config_setting_t *cdb = libconfig->lookup(&captcha_db_conf, "captcha_db");

	while ((it = libconfig->setting_get_elem(cdb, i++)) != NULL) {
		if (macro->read_captcha_db_libconfig_sub(it, i - 1, filepath))
			++count;
	}

	libconfig->destroy(&captcha_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
	return true;
}

static bool macro_read_captcha_db_libconfig_sub(const struct config_setting_t *it, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	const char *filename = libconfig->setting_get_string_elem(it, 0);

	if (filename == NULL) {
		ShowError("%s: Invalid filename for entry %d in %s\n", __func__, n, source);
		return false;
	}

	char filepath[270];
	snprintf(filepath, sizeof(filepath), "%s/captcha/%s", map->db_path, filename);

	if (!exists(filepath)) {
		ShowError("%s: File %s does not exist for entry %d in %s\n", __func__, filepath, n, source);
		return false;
	}

	const char *answer = libconfig->setting_get_string_elem(it, 1);

	if (answer == NULL) {
		ShowError("%s: Invalid answer for entry %d in %s\n", __func__, n, source);
		return false;
	}

	const unsigned long alen = strlen(answer);
	if (alen < 4 || alen > 15) {
		ShowError("%s: Answer \"%s\" must be between 4~15 chars in len for entry %d in %s\n", __func__, answer, n, source);
		return false;
	}

	for (int i = 0; i < alen; ++i) {
		if (strchr(macro_allowed_answer_chars, answer[i]) == NULL) {
			ShowError("%s: Answer \"%s\" have an invalid character \"%c\" for entry %d in %s\n", __func__, answer, answer[i], n, source);
			return false;
		}
	}

	struct captcha_data cd = { 0 };
	if (!macro->read_captcha_db_libconfig_sub_loadbmp(filepath, &cd))
		return false;

	VECTOR_ENSURE(macro->captcha_registery, 1, 1);

	cd.upload_size = cd.image_size;
	safestrncpy(cd.captcha_answer, answer, sizeof(cd.captcha_answer));

	char captcha_key[50];
	snprintf(captcha_key, 50, "%X", (unsigned int)VECTOR_CAPACITY(macro->captcha_registery));
	safestrncpy(cd.captcha_key, captcha_key, sizeof(cd.captcha_key));

	VECTOR_PUSH(macro->captcha_registery, cd);
	return true;
}

static bool macro_read_captcha_db_libconfig_sub_loadbmp(const char *filepath, struct captcha_data *cd)
{
	nullpo_retr(false, filepath);
	nullpo_retr(false, cd);

	FILE *fp = fopen(filepath, "rb");
	nullpo_retr(false, fp);

	// Get the file size
	fseek(fp, 0, SEEK_END);
	const unsigned int file_len = (unsigned int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_len != CAPTCHA_BMP_SIZE) {
		ShowError("%s: Invalid BMP file given at \"%s\"\n", __func__, filepath);
		fclose(fp);
		return false;
	}

	// Load the file data and verify magic
	char *bmp_data = aMalloc(CAPTCHA_BMP_SIZE);
	if (fread(bmp_data, CAPTCHA_BMP_SIZE, 1, fp) != 1) {
		ShowError("%s: Failed to read data from \"%s\"\n", __func__, filepath);
		fclose(fp);
		aFree(bmp_data);
		return false;
	}
	if (bmp_data[0] != 'B' || bmp_data[1] != 'M') {
		ShowError("%s: Invalid BMP file header given at \"%s\"\n", __func__, filepath);
		fclose(fp);
		aFree(bmp_data);
		return false;
	}

	// Initialize the destination buffer
	cd->image_size = 0;
	memset(cd->image_data, 0, CAPTCHA_BMP_SIZE);

	// Compress the data into the destination
	unsigned long com_size = 0;
	grfio->encode_zip(cd->image_data, &com_size, bmp_data, CAPTCHA_BMP_SIZE);
	cd->image_size = (int)com_size;

	fclose(fp);
	aFree(bmp_data);
	return true;
}

static int do_init_macro(bool minimal)
{
	if (minimal)
		return 0;

	VECTOR_INIT(macro->captcha_registery);
	timer->add_func_list(macro->detector_timeout, "macro_detector_timeout");
	macro->read_captcha_db_libconfig();
	return 0;
}

static void do_final_macro(void)
{
	VECTOR_CLEAR(macro->captcha_registery);
}

void macro_defaults(void)
{
	macro = &macro_s;

	/* core */
	macro->init = do_init_macro;
	macro->final = do_final_macro;

	macro->captcha_register = macro_captcha_register;
	macro->captcha_register_upload = macro_captcha_register_upload;
	macro->captcha_preview = macro_captcha_preview;
	macro->detector_request = macro_detector_request;
	macro->detector_process_answer = macro_detector_process_answer;
	macro->detector_timeout = macro_detector_timeout;
	macro->detector_disconnect = macro_detector_disconnect;
	macro->reporter_area_select = macro_reporter_area_select;
	macro->reporter_area_select_sub = macro_reporter_area_select_sub;
	macro->reporter_process = macro_reporter_process;
	macro->read_captcha_db_libconfig = macro_read_captcha_db_libconfig;
	macro->read_captcha_db_libconfig_sub = macro_read_captcha_db_libconfig_sub;
	macro->read_captcha_db_libconfig_sub_loadbmp = macro_read_captcha_db_libconfig_sub_loadbmp;
}
