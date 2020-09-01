/**
 * This file is part of Hercules.
 * https://herc.ws - https://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "duel.h"

#include "map/atcommand.h"  // msg_txt
#include "map/clif.h"
#include "map/pc.h"
#include "common/cbasetypes.h"
#include "common/nullpo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct duel_interface duel_s;
struct duel_interface *duel;

/*==========================================
 * Duel organizing functions [LuzZza]
 *------------------------------------------*/
static void duel_savetime(struct map_session_data *sd)
{
	pc_setglobalreg(sd, script->add_variable("PC_LAST_DUEL_TIME"), (int)time(NULL));
}

static int64 duel_difftime(struct map_session_data *sd)
{
	return (pc_readglobalreg(sd, script->add_variable("PC_LAST_DUEL_TIME")) + battle_config.duel_time_interval - (int)time(NULL));
}

static int duel_showinfo_sub(struct map_session_data *sd, va_list va)
{
	struct map_session_data *ssd = va_arg(va, struct map_session_data*);
	int *p = va_arg(va, int*);
	char output[256];

	nullpo_retr(1, sd);
	nullpo_retr(1, ssd);
	if (sd->duel_group != ssd->duel_group) return 0;

	sprintf(output, "      %d. %s", ++(*p), sd->status.name);
	clif_disp_onlyself(ssd, output);
	return 1;
}

static void duel_showinfo(const unsigned int did, struct map_session_data *sd)
{
	int p=0;
	char output[256];

	if(duel->list[did].max_players_limit > 0)
		sprintf(output, msg_sd(sd,370), //" -- Duels: %d/%d, Members: %d/%d, Max players: %d --"
			did, duel->count,
			duel->list[did].members_count,
			duel->list[did].members_count + duel->list[did].invites_count,
			duel->list[did].max_players_limit);
	else
		sprintf(output, msg_sd(sd,371), //" -- Duels: %d/%d, Members: %d/%d --"
			did, duel->count,
			duel->list[did].members_count,
			duel->list[did].members_count + duel->list[did].invites_count);

	clif_disp_onlyself(sd, output);
	map->foreachpc(duel_showinfo_sub, sd, &p);
}

static int duel_create(struct map_session_data *sd, const unsigned int maxpl)
{
	int i=1;
	char output[256];

	nullpo_ret(sd);

	while(i < MAX_DUEL && duel->list[i].members_count > 0) i++;
	if(i == MAX_DUEL) return 0;

	duel->count++;
	sd->duel_group = i;
	duel->list[i].members_count++;
	duel->list[i].invites_count = 0;
	duel->list[i].max_players_limit = maxpl;

	safestrncpy(output, msg_sd(sd,372), sizeof(output)); // " -- Duel has been created (@invite/@leave) --"
	clif_disp_onlyself(sd, output);

	clif->map_property(sd, MAPPROPERTY_FREEPVPZONE);
	clif->maptypeproperty2(&sd->bl,SELF);
	return i;
}

static void duel_invite(const unsigned int did, struct map_session_data *sd, struct map_session_data *target_sd)
{
	char output[256];

	nullpo_retv(sd);
	nullpo_retv(target_sd);
	// " -- Player %s invites %s to duel --"
	sprintf(output, msg_sd(sd,373), sd->status.name, target_sd->status.name);
	clif->disp_message(&sd->bl, output, DUEL_WOS);

	target_sd->duel_invite = did;
	duel->list[did].invites_count++;

	// "Blue -- Player %s invites you to PVP duel (@accept/@reject) --"
	sprintf(output, msg_sd(target_sd,374), sd->status.name);
	clif->broadcast(&target_sd->bl, output, (int)strlen(output)+1, BC_BLUE, SELF);
}

static int duel_leave_sub(struct map_session_data *sd, va_list va)
{
	int did = va_arg(va, int);
	nullpo_ret(sd);
	if (sd->duel_invite == did)
		sd->duel_invite = 0;
	return 0;
}

static void duel_leave(const unsigned int did, struct map_session_data *sd)
{
	char output[256];

	nullpo_retv(sd);
	// " <- Player %s has left duel --"
	sprintf(output, msg_sd(sd,375), sd->status.name);
	clif->disp_message(&sd->bl, output, DUEL_WOS);

	duel->list[did].members_count--;
	if(duel->list[did].members_count == 0) {
		map->foreachpc(duel_leave_sub, did);
		duel->count--;
	}

	sd->duel_group = 0;
	duel_savetime(sd);
	clif->map_property(sd, MAPPROPERTY_NOTHING);
	clif->maptypeproperty2(&sd->bl,SELF);
}

static void duel_accept(const unsigned int did, struct map_session_data *sd)
{
	char output[256];

	nullpo_retv(sd);
	duel->list[did].members_count++;
	sd->duel_group = sd->duel_invite;
	duel->list[did].invites_count--;
	sd->duel_invite = 0;

	// " -> Player %s has accepted duel --"
	sprintf(output, msg_sd(sd,376), sd->status.name);
	clif->disp_message(&sd->bl, output, DUEL_WOS);

	clif->map_property(sd, MAPPROPERTY_FREEPVPZONE);
	clif->maptypeproperty2(&sd->bl,SELF);
}

static void duel_reject(const unsigned int did, struct map_session_data *sd)
{
	char output[256];

	nullpo_retv(sd);
	// " -- Player %s has rejected duel --"
	sprintf(output, msg_sd(sd,377), sd->status.name);
	clif->disp_message(&sd->bl, output, DUEL_WOS);

	duel->list[did].invites_count--;
	sd->duel_invite = 0;
}

static void do_final_duel(void)
{
}

static void do_init_duel(bool minimal)
{
	if (minimal)
		return;

	memset(&duel->list[0], 0, sizeof(duel->list));
}

/*=====================================
 * Default Functions : duel.h
 * Generated by HerculesInterfaceMaker
 * created by Susu
 *-------------------------------------*/
void duel_defaults(void)
{
	duel = &duel_s;
	/* vars */
	duel->count = 0;
	/* funcs */
	//Duel functions // [LuzZza]
	duel->create = duel_create;
	duel->invite = duel_invite;
	duel->accept = duel_accept;
	duel->reject = duel_reject;
	duel->leave = duel_leave;
	duel->showinfo = duel_showinfo;
	duel->difftime = duel_difftime;

	duel->init = do_init_duel;
	duel->final = do_final_duel;
}
