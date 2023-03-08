/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2023-2023 Hercules Dev Team
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

#include "goldpc.h"

#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/timer.h"
#include "common/utils.h"

#include "map/pc.h"

#include <stdlib.h>

static struct goldpc_interface goldpc_s;
struct goldpc_interface *goldpc;

void goldpc_read_db_libconfig(void)
{
	char filepath[512];
	snprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, "goldpc_db.conf");

	struct config_t goldpc_conf;
	if (libconfig->load_file(&goldpc_conf, filepath) == CONFIG_FALSE) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		return;
	}

	struct config_setting_t *goldpc_db = NULL;
	if ((goldpc_db = libconfig->setting_get_member(goldpc_conf.root, "goldpc_db")) == NULL) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		libconfig->destroy(&goldpc_conf);
		return;
	}

	int i = 0;
	int count = 0;
	struct config_setting_t *it = NULL;

	while ((it = libconfig->setting_get_elem(goldpc_db, i++)) != NULL) {
		if (goldpc->read_db_libconfig_sub(it, i, filepath))
			++count;
	}

	libconfig->destroy(&goldpc_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
}

bool goldpc_read_db_libconfig_sub(const struct config_setting_t *it, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, source);

	struct goldpc_mode mode = { 0 };

	if (libconfig->setting_lookup_int(it, "Id", &mode.id) == CONFIG_FALSE) {
		ShowError("%s: Invalid GoldPC mode Id provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const char *str = NULL;
	if (!libconfig->setting_lookup_string(it, "Const", &str) || !*str) {
		ShowError("%s: Invalid GoldPC mode Const for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	char const_name[SCRIPT_VARNAME_LENGTH];
	safestrncpy(const_name, str, sizeof(const_name));

	if (libconfig->setting_lookup_int(it, "Time", &mode.required_time) == CONFIG_FALSE) {
		ShowError("%s: Invalid GoldPC mode Time provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	if (libconfig->setting_lookup_int(it, "Points", &mode.points) == CONFIG_FALSE) {
		ShowError("%s: Invalid GoldPC mode Points provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	if (!goldpc->read_db_validate(&mode, source))
		return false;

	// Client always counts towards GOLDPC_MAX_TIME, so calculate an offset that should be added when sending to client
	mode.time_offset = (GOLDPC_MAX_TIME - mode.required_time);

	struct goldpc_mode *mode_entry = aCalloc(1, sizeof(struct goldpc_mode));
	*mode_entry = mode;
	idb_put(goldpc->db, mode.id, mode_entry);

	script->set_constant2(const_name, mode.id, false, false);

	return true;
}

bool goldpc_read_db_validate(struct goldpc_mode *mode, const char *source)
{
	nullpo_retr(false, mode);
	nullpo_retr(false, source);

	if (mode->id == 0) {
		ShowError(
			"%s: Invalid GoldPC mode Id (%d) provided in '%s'. Id '0' is reserved for disabled state. Skipping...\n",
			__func__, mode->id, source);
		return false;
	}

	if (mode->required_time < 1 || mode->required_time > GOLDPC_MAX_TIME) {
		ShowError(
			"%s: Invalid GoldPC mode Time provided for ID %d in '%s'. Time must be between 1 and %d. Skipping...\n",
			__func__, mode->id, source, GOLDPC_MAX_TIME);
		return false;
	}

	if (mode->points < 0 || mode->points > GOLDPC_MAX_POINTS) {
		ShowError(
			"%s: Invalid GoldPC mode Points provided for ID %d in '%s'. Points must be between 0 and %d. Skipping...\n",
			__func__, mode->id, source, GOLDPC_MAX_POINTS);
		return false;
	}

	return true;
}

/**
 * Adds GoldPC points to sd. It also accepts negative values to remove points.
 */
static void goldpc_addpoints(struct map_session_data *sd, int points)
{
	nullpo_retv(sd);

	int final_balance;
	if (sd->goldpc.points < (GOLDPC_MAX_POINTS - points))
		final_balance = sd->goldpc.points + points;
	else
		final_balance = GOLDPC_MAX_POINTS;

	final_balance = cap_value(final_balance, 0, GOLDPC_MAX_POINTS);
	pc_setaccountreg(sd, script->add_variable(GOLDPC_POINTS_VAR), final_balance);
}

/**
 * Loads account's GoldPC data and start it.
 */
static void goldpc_load(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (!battle_config.feature_goldpc_enable)
		return;

	sd->goldpc.mode = goldpc->exists(battle_config.feature_goldpc_default_mode);
	sd->goldpc.points = pc_readaccountreg(sd,script->add_variable(GOLDPC_POINTS_VAR));
	sd->goldpc.play_time = pc_readaccountreg(sd,script->add_variable(GOLDPC_PLAYTIME_VAR));
	sd->goldpc.tid = INVALID_TIMER;
	sd->goldpc.loaded = true;

	if (sd->state.autotrade > 0 || sd->state.standalone > 0)
		return;

	goldpc->start(sd);
}

/**
 * Starts GoldPC timer
 */
static void goldpc_start(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (!battle_config.feature_goldpc_enable)
		return;

	if (!sd->goldpc.loaded)
		return;

	sd->goldpc.start_tick = 0;
	if (sd->goldpc.tid != INVALID_TIMER) {
		timer->delete(sd->goldpc.tid, goldpc->timeout);
		sd->goldpc.tid = INVALID_TIMER;
	}

	if (sd->goldpc.mode == NULL) {
		// We still want to send when NULL because it may be the case where GoldPC is being disabled
		clif->goldpc_info(sd);
		return;
	}

	if (sd->goldpc.points < GOLDPC_MAX_POINTS) {
		sd->goldpc.start_tick = timer->gettick();

		int remaining_time = sd->goldpc.mode->required_time - sd->goldpc.play_time;
		if (remaining_time < 0) {
			goldpc_addpoints(sd, sd->goldpc.mode->points);
			sd->goldpc.play_time = 0;

			goldpc->start(sd);
			return;
		}

		sd->goldpc.tid = timer->add(
			sd->goldpc.start_tick + remaining_time * 1000,
			goldpc->timeout,
			sd->bl.id,
			0
		);
	}

	clif->goldpc_info(sd);
}

/**
 * Timer called when GoldPC time is reached.
 * Process the point increment/restart
 */
static int goldpc_timeout(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = map->id2sd(id);
	if (sd == NULL)
		return 0; // Player logged out

	if (sd->goldpc.tid != tid) {
		// Should never happen unless something changed the timer without stopping the previous one.
		ShowWarning("%s: timer mismatch %d != %d\n", __func__, sd->goldpc.tid, tid);
		return 0;
	}

	sd->goldpc.play_time = 0;
	sd->goldpc.start_tick = 0;
	sd->goldpc.tid = INVALID_TIMER;

	if (sd->goldpc.mode == NULL || sd->goldpc.points >= GOLDPC_MAX_POINTS)
		return 0;

	goldpc->addpoints(sd, sd->goldpc.mode->points);

	goldpc->start(sd);
	return 0;
}

/**
 * Stops GoldPC timer, saving its current state.
 */
static void goldpc_stop(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (!sd->goldpc.loaded)
		return;

	pc_setaccountreg(sd, script->add_variable(GOLDPC_PLAYTIME_VAR), sd->goldpc.play_time);
	if (sd->goldpc.mode == NULL || sd->goldpc.tid == INVALID_TIMER)
		return;

	if (sd->goldpc.tid != INVALID_TIMER) {
		if (sd->goldpc.start_tick > 0) {
			int played_ticks = (int) ((timer->gettick() - sd->goldpc.start_tick) / 1000);
			int playtime = (int) cap_value(played_ticks + sd->goldpc.play_time, 0, GOLDPC_MAX_TIME);

			sd->goldpc.play_time = playtime;
			pc_setaccountreg(sd, script->add_variable(GOLDPC_PLAYTIME_VAR), playtime);
		}

		timer->delete(sd->goldpc.tid, goldpc_timeout);
		sd->goldpc.tid = INVALID_TIMER;
	}
}

/**
 * Checks if a goldpc with given id exists.
 * Returns NULL if it doesn't.
 */
static struct goldpc_mode * goldpc_db_exists(int id)
{
	return (struct goldpc_mode *) idb_get(goldpc->db, id);
}

static int do_init_goldpc(bool minimal)
{
	if (minimal)
		return 0;

	goldpc->db = idb_alloc(DB_OPT_RELEASE_DATA);
	goldpc->read_db_libconfig();

	return 0;
}

static void do_final_goldpc(void)
{
	goldpc->db->destroy(goldpc->db, NULL);
}

void goldpc_defaults(void)
{
	goldpc = &goldpc_s;

	/* core */
	goldpc->init = do_init_goldpc;
	goldpc->final = do_final_goldpc;
	goldpc->exists = goldpc_db_exists;

	/* database */
	goldpc->read_db_libconfig = goldpc_read_db_libconfig;
	goldpc->read_db_libconfig_sub = goldpc_read_db_libconfig_sub;
	goldpc->read_db_validate = goldpc_read_db_validate;

	/* process */
	goldpc->addpoints = goldpc_addpoints;
	goldpc->load = goldpc_load;
	goldpc->start = goldpc_start;
	goldpc->timeout = goldpc_timeout;
	goldpc->stop = goldpc_stop;
}
