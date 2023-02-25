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

#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/timer.h"
#include "common/utils.h"

#include "map/pc.h"

#include <stdlib.h>

static struct goldpc_interface goldpc_s;
struct goldpc_interface *goldpc;

// TODO: Replace with db
struct goldpc_mode default_goldpc_mode;

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

	sd->goldpc.mode = &default_goldpc_mode;
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

static int do_init_goldpc(bool minimal)
{
	default_goldpc_mode.id = 1;
	default_goldpc_mode.required_time = GOLDPC_MAX_TIME;
	default_goldpc_mode.points = 1;
	
	// Client always counts towards GOLDPC_MAX_TIME, so calculate an offset that should be added when sending to client
	default_goldpc_mode.time_offset = (GOLDPC_MAX_TIME - default_goldpc_mode.required_time);

	return 0;
}

static void do_final_goldpc(void)
{
	
}

void goldpc_defaults(void)
{
	goldpc = &goldpc_s;

	/* core */
	goldpc->init = do_init_goldpc;
	goldpc->final = do_final_goldpc;

	goldpc->addpoints = goldpc_addpoints;
	goldpc->load = goldpc_load;
	goldpc->start = goldpc_start;
	goldpc->timeout = goldpc_timeout;
	goldpc->stop = goldpc_stop;
}
