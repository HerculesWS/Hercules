/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2025  Hercules Dev Team
 * Copyright (C) 2019  OriginsRO
 * Copyright (C) 2019  Haru
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

#include "common/hercules.h"
#include "common/core.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/status.h"

#include "common/HPMDataCheck.h"

#include <stdlib.h>

HPExport struct hplugin_info pinfo = {
	"test_equippos", ///< Plugin name
	SERVER_TYPE_MAP, ///< Plugin type
	"0.1",           ///< Plugin version
	HPM_VERSION,     ///< HPM Version
};

#define TEST(name, function, ...) do { \
	const char *message = NULL; \
	ShowMessage("-------------------------------------------------------------------------------\n"); \
	ShowNotice("Testing %s...\n", (name)); \
	if ((message = (function)(__VA_ARGS__)) != NULL) { \
		ShowError("Failed. %s\n", message); \
		ShowMessage("===============================================================================\n"); \
		ShowFatalError("Failure. Aborting further tests.\n"); \
		exit(EXIT_FAILURE); \
	} \
	ShowInfo("Test passed.\n"); \
} while (false)

struct test_data {
	int view_sprite;
	int equip;
};

static struct item_data items[] = {
	{ .nameid = 0, .equip = 0 },
	{ .nameid = 1, .equip = EQP_HEAD_LOW },
	{ .nameid = 2, .equip = EQP_HEAD_MID },
	{ .nameid = 3, .equip = EQP_HEAD_TOP },
	{ .nameid = 4, .equip = EQP_HEAD_LOW | EQP_HEAD_MID },
	{ .nameid = 5, .equip = EQP_HEAD_LOW | EQP_HEAD_TOP },
	{ .nameid = 6, .equip = EQP_HEAD_MID | EQP_HEAD_TOP },
	{ .nameid = 7, .equip = EQP_HEAD_LOW | EQP_HEAD_MID | EQP_HEAD_TOP },
	{ .nameid = 8, .equip = EQP_COSTUME_HEAD_LOW },
	{ .nameid = 9, .equip = EQP_COSTUME_HEAD_MID },
	{ .nameid = 10, .equip = EQP_COSTUME_HEAD_TOP },
	{ .nameid = 11, .equip = EQP_COSTUME_HEAD_LOW | EQP_COSTUME_HEAD_MID },
	{ .nameid = 12, .equip = EQP_COSTUME_HEAD_LOW | EQP_COSTUME_HEAD_TOP },
	{ .nameid = 13, .equip = EQP_COSTUME_HEAD_MID | EQP_COSTUME_HEAD_TOP },
	{ .nameid = 14, .equip = EQP_COSTUME_HEAD_LOW | EQP_COSTUME_HEAD_MID | EQP_COSTUME_HEAD_TOP },
};

VECTOR_STRUCT_DECL(autorelease, struct map_session_data *);

#define EQUIP_ITEM(sd, id) do { pc->equipitem((sd), (id), items[(id)].equip); } while (0)
#define UNEQUIP_ITEM(sd, id) do { pc->unequipitem((sd), (id), PCUNEQUIPITEM_NONE); } while (0)

static bool checklook(const struct view_data *vd, enum look expected_bottom, enum look expected_top, enum look expected_mid)
{
	nullpo_retr(false, vd);
	if (vd->head_bottom != expected_bottom)
		return false;
	if (vd->head_top != expected_top)
		return false;
	if (vd->head_mid != expected_mid)
		return false;
	return true;
}

static char out_message[256];
static const char *check(struct map_session_data *sd, const enum look *expected)
{
	nullpo_retr("NULL pointer (expected)", expected);
	nullpo_retr("NULL pointer (sd)", sd);

	const struct view_data *vd = status->get_viewdata(&sd->bl);
	nullpo_retr("NULL pointer (vd)", vd);

	if (checklook(vd, expected[0], expected[1], expected[2]))
		return NULL;

	snprintf(out_message, sizeof out_message, "Current: %d %d %d; Status: %d %d %d; Expected: %d %d %d",
			vd->head_bottom, vd->head_top, vd->head_mid,
			sd->status.look.head_bottom, sd->status.look.head_top, sd->status.look.head_mid,
			(int)expected[0], (int)expected[1], (int)expected[2]
	);
	return out_message;
}
#define EQUIP_CHK(name, sd, id, exp) \
	do { \
		EQUIP_ITEM((sd), (id)); \
		TEST((name), check, (sd), (exp)); \
	} while (0);
#define UNEQUIP_CHK(name, sd, id, exp) \
	do { \
		UNEQUIP_ITEM((sd), (id)); \
		TEST((name), check, (sd), (exp)); \
	} while (0);

#define ADD(ar, base, id, exp) \
	({ \
		struct map_session_data *temp_ = make_autoreleased_sd(ar); \
		*temp_ = *(base); \
		EQUIP_CHK(#base " + " #id, temp_, (id), (exp)); \
		temp_; \
	})

#define DEL(ar, base, id, exp) \
	do { \
		struct map_session_data *temp_ = make_sd(); \
		*temp_ = *(base); \
		UNEQUIP_CHK(#base " - " #id, temp_, (id), (exp)); \
		aFree(temp_); \
	} while (0)

#define TOGGLE(ar, base, id, exp1, exp2) \
	do { \
		struct map_session_data *temp_ = make_sd(); \
		*temp_ = *(base); \
		EQUIP_CHK(#base " + " #id, temp_, (id), (exp1)); \
		UNEQUIP_CHK(#base " +/- " #id, temp_, (id), (exp2)); \
		aFree(temp_); \
	} while (0)

static struct map_session_data *make_sd(void)
{
	struct map_session_data *dummy = pc->get_dummy_sd();
	dummy->bl.id = 150000;
	dummy->bl.type = BL_PC;
	dummy->status.account_id = 150000;
	dummy->status.char_id = 150000;
	dummy->status.base_level = 1;
	dummy->vd.class = JOB_NOVICE;
	dummy->status.inventorySize = FIXED_INVENTORY_SIZE;
	for (int i = 0; i < ARRAYLENGTH(items); i++) {
		dummy->inventory_data[i] = &items[i];
		dummy->status.inventory[i].nameid = items[i].nameid;
	}
	for (int i = 0; i < EQI_MAX; i++) {
		dummy->equip_index[i] = -1;
	}
	return dummy;
}

static struct map_session_data *make_autoreleased_sd(struct autorelease *autorelease)
{
	struct map_session_data *dummy = make_sd();
	VECTOR_ENSURE(*autorelease, 1, 1);
	VECTOR_PUSH(*autorelease, dummy);
	return dummy;
}

static void my_clif_equipitemack(struct map_session_data *sd, int n, int pos, enum e_EQUIP_ITEM_ACK result)
{
}

static void my_status_calc_bl_(struct block_list *bl, enum scb_flag flag, enum e_status_calc_opt opt)
{
}

HPExport void plugin_init(void)
{
	for (int i = 0; i < ARRAYLENGTH(items); i++) {
		items[i].type = IT_ARMOR;
		itemdb->jobmask2mapid(items[i].class_base, UINT64_MAX);
		items[i].class_upper = ITEMUPPER_ALL;
		items[i].view_sprite = i;
	}
}

HPExport void server_preinit(void)
{
	clif->equipitemack = my_clif_equipitemack;
	status->calc_bl_ = my_status_calc_bl_;
}

HPExport void server_online(void)
{
	ShowMessage("===============================================================================\n");
	ShowStatus("Starting tests.\n");

	struct autorelease autorelease;
	VECTOR_INIT(autorelease);

	enum {
		ID_EXT  = 0,
		ID_0    = 0,
		ID_B    = 1,
		ID_M    = 2,
		ID_T    = 3,
		ID_MB   = 4,
		ID_TB   = 5,
		ID_TM   = 6,
		ID_TMB  = 7,
		ID_CB   = 8,
		ID_CM   = 9,
		ID_CT   = 10,
		ID_CMB  = 11,
		ID_CTB  = 12,
		ID_CTM  = 13,
		ID_CTMB = 14,
	};

	// Zero
	const enum look exp_empty[] = { ID_0, ID_0, ID_0 };

	// One
	const enum look exp_bottom[] = { ID_B, ID_0, ID_0 };
	const enum look exp_mid[] = { ID_0, ID_0, ID_M };
	const enum look exp_top[] = { ID_0, ID_T, ID_0 };
	const enum look exp_midbottom[] = { ID_EXT, ID_0, ID_MB };
	const enum look exp_topbottom[] = { ID_EXT, ID_TB, ID_0 };
	const enum look exp_topmid[] = { ID_0, ID_TM, ID_EXT };
	const enum look exp_topmidbottom[] = { ID_EXT, ID_TMB, ID_EXT };
	const enum look exp_cbottom[] = { ID_CB, ID_0, ID_0 };
	const enum look exp_cmid[] = { ID_0, ID_0, ID_CM };
	const enum look exp_ctop[] = { ID_0, ID_CT, ID_0 };
	const enum look exp_cmidbottom[] = { ID_EXT, ID_0, ID_CMB };
	const enum look exp_ctopbottom[] = { ID_EXT, ID_CTB, ID_0 };
	const enum look exp_ctopmid[] = { ID_0, ID_CTM, ID_EXT };
	const enum look exp_ctopmidbottom[] = { ID_EXT, ID_CTMB, ID_EXT };

	// Two
	const enum look exp_mid_bottom[] = { ID_B, ID_0, ID_M };
	const enum look exp_top_bottom[] = { ID_B, ID_T, ID_0 };
	const enum look exp_top_mid[] = { ID_0, ID_T, ID_M };
	const enum look exp_topmid_bottom[] = { ID_B, ID_TM, ID_EXT };
	const enum look exp_topbottom_mid[] = { ID_EXT, ID_TB, ID_M };
	const enum look exp_top_midbottom[] = { ID_EXT, ID_T, ID_MB };
	const enum look exp_cmid_bottom[] = { ID_B, ID_0, ID_CM };
	const enum look exp_cmid_topbottom[] = { ID_EXT, ID_TB, ID_CM };
	const enum look exp_cmid_top[] = { ID_0, ID_T, ID_CM };
	const enum look exp_ctop_bottom[] = { ID_B, ID_CT, ID_0 };
	const enum look exp_ctop_mid[] = { ID_0, ID_CT, ID_M };
	const enum look exp_ctop_midbottom[] = { ID_0, ID_CT, ID_MB };
	const enum look exp_ctopmid_bottom[] = { ID_B, ID_CTM, ID_EXT };
	const enum look exp_cbottom_mid[] = { ID_CB, ID_0, ID_M };
	const enum look exp_cbottom_top[] = { ID_CB, ID_T, ID_0 };
	const enum look exp_cbottom_topmid[] = { ID_CB, ID_TM, ID_EXT };
	const enum look exp_ctopbottom_mid[] = { ID_EXT, ID_CTB, ID_M };
	const enum look exp_cmidbottom_top[] = { ID_EXT, ID_T, ID_CMB };
	const enum look exp_cmid_cbottom[] = { ID_CB, ID_0, ID_CM };
	const enum look exp_ctop_cbottom[] = { ID_CB, ID_CT, ID_0 };
	const enum look exp_ctopmid_cbottom[] = { ID_CB, ID_CTM, ID_EXT };
	const enum look exp_ctop_cmid[] = { ID_0, ID_CT, ID_CM };
	const enum look exp_ctopbottom_cmid[] = { ID_EXT, ID_CTB, ID_CM };
	const enum look exp_ctop_cmidbottom[] = { ID_EXT, ID_CT, ID_CMB };

	// Three
	const enum look exp_top_mid_bottom[] = { ID_B, ID_T, ID_M };
	const enum look exp_ctop_mid_bottom[] = { ID_B, ID_CT, ID_M };
	const enum look exp_cmid_top_bottom[] = { ID_B, ID_T, ID_CM };
	const enum look exp_ctop_cmid_bottom[] = { ID_B, ID_CT, ID_CM };
	const enum look exp_cbottom_top_mid[] = { ID_CB, ID_T, ID_M };
	const enum look exp_ctop_cbottom_mid[] = { ID_CB, ID_CT, ID_M };
	const enum look exp_cmid_cbottom_top[] = { ID_CB, ID_T, ID_CM };
	const enum look exp_ctop_cmid_cbottom[] = { ID_CB, ID_CT, ID_CM };
	//{ LOOK_HEAD_BOTTOM, LOOK_HEAD_TOP, LOOK_HEAD_MID }

	// Zero: (0)
	struct map_session_data *zero_sd = make_autoreleased_sd(&autorelease);
	TEST("No headgears", check, zero_sd, exp_empty);

	// One: b m t mb tb tm tmb cb cm ct cmb ctb ctm ctmb (14)
	struct map_session_data *sd_b = ADD(&autorelease, zero_sd, ID_B, exp_bottom);
	struct map_session_data *sd_m = ADD(&autorelease, zero_sd, ID_M, exp_mid);
	struct map_session_data *sd_t = ADD(&autorelease, zero_sd, ID_T, exp_top);
	struct map_session_data *sd_mb = ADD(&autorelease, zero_sd, ID_MB, exp_midbottom);
	struct map_session_data *sd_tb = ADD(&autorelease, zero_sd, ID_TB, exp_topbottom);
	struct map_session_data *sd_tm = ADD(&autorelease, zero_sd, ID_TM, exp_topmid);
	struct map_session_data *sd_tmb = ADD(&autorelease, zero_sd, ID_TMB, exp_topmidbottom);
	struct map_session_data *sd_cb = ADD(&autorelease, zero_sd, ID_CB, exp_cbottom);
	struct map_session_data *sd_cm = ADD(&autorelease, zero_sd, ID_CM, exp_cmid);
	struct map_session_data *sd_ct = ADD(&autorelease, zero_sd, ID_CT, exp_ctop);
	struct map_session_data *sd_cmb = ADD(&autorelease, zero_sd, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_ctb = ADD(&autorelease, zero_sd, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_ctm = ADD(&autorelease, zero_sd, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_ctmb = ADD(&autorelease, zero_sd, ID_CTMB, exp_ctopmidbottom);

	// Two (61)
	// b+x: b+m b+t b+tm b+cb b+cm b+ct b+cmb b+ctb b+ctm b+ctmb (10)
	ADD(&autorelease, sd_b, ID_B, exp_bottom); DEL(&autorelease, sd_b, ID_B, exp_empty);
	struct map_session_data *sd_b_m = ADD(&autorelease, sd_b, ID_M, exp_mid_bottom);
	struct map_session_data *sd_b_t = ADD(&autorelease, sd_b, ID_T, exp_top_bottom);
	TOGGLE(&autorelease, sd_b, ID_MB, exp_midbottom, exp_empty);
	TOGGLE(&autorelease, sd_b, ID_TB, exp_topbottom, exp_empty);
	struct map_session_data *sd_b_tm = ADD(&autorelease, sd_b, ID_TM, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_b_cb = ADD(&autorelease, sd_b, ID_CB, exp_cbottom);
	struct map_session_data *sd_b_cm = ADD(&autorelease, sd_b, ID_CM, exp_cmid_bottom);
	struct map_session_data *sd_b_ct = ADD(&autorelease, sd_b, ID_CT, exp_ctop_bottom);
	struct map_session_data *sd_b_cmb = ADD(&autorelease, sd_b, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_b_ctb = ADD(&autorelease, sd_b, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_b_ctm = ADD(&autorelease, sd_b, ID_CTM, exp_ctopmid_bottom);
	struct map_session_data *sd_b_ctmb = ADD(&autorelease, sd_b, ID_CTMB, exp_ctopmidbottom);
	// m+x: m+t m+tb m+cb m+cm m+ct m+ctb m+ctm m+cmb m+ctmb (9)
	ADD(&autorelease, sd_m, ID_B, exp_mid_bottom);
	ADD(&autorelease, sd_m, ID_M, exp_mid); DEL(&autorelease, sd_m, ID_M, exp_empty);
	struct map_session_data *sd_m_t = ADD(&autorelease, sd_m, ID_T, exp_top_mid);
	TOGGLE(&autorelease, sd_m, ID_MB, exp_midbottom, exp_empty);
	struct map_session_data *sd_m_tb = ADD(&autorelease, sd_m, ID_TB, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_m, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_m_cb = ADD(&autorelease, sd_m, ID_CB, exp_cbottom_mid);
	struct map_session_data *sd_m_cm = ADD(&autorelease, sd_m, ID_CM, exp_cmid);
	struct map_session_data *sd_m_ct = ADD(&autorelease, sd_m, ID_CT, exp_ctop_mid);
	struct map_session_data *sd_m_cmb = ADD(&autorelease, sd_m, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_m_ctb = ADD(&autorelease, sd_m, ID_CTB, exp_ctopbottom_mid);
	struct map_session_data *sd_m_ctm = ADD(&autorelease, sd_m, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_m_ctmb = ADD(&autorelease, sd_m, ID_CTMB, exp_ctopmidbottom);
	// t+x: t+mb t+cb t+cm t+ct t+cmb t+ctb t+ctm t+ctmb (8)
	ADD(&autorelease, sd_t, ID_B, exp_top_bottom);
	ADD(&autorelease, sd_t, ID_M, exp_top_mid);
	ADD(&autorelease, sd_t, ID_T, exp_top); DEL(&autorelease, sd_t, ID_T, exp_empty);
	struct map_session_data *sd_t_mb = ADD(&autorelease, sd_t, ID_MB, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t, ID_TB, exp_topbottom, exp_empty);
	TOGGLE(&autorelease, sd_t, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_t, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_t_cb = ADD(&autorelease, sd_t, ID_CB, exp_cbottom_top);
	struct map_session_data *sd_t_cm = ADD(&autorelease, sd_t, ID_CM, exp_cmid_top);
	struct map_session_data *sd_t_ct = ADD(&autorelease, sd_t, ID_CT, exp_ctop);
	struct map_session_data *sd_t_cmb = ADD(&autorelease, sd_t, ID_CMB, exp_cmidbottom_top);
	struct map_session_data *sd_t_ctb = ADD(&autorelease, sd_t, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_t_ctm = ADD(&autorelease, sd_t, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_t_ctmb = ADD(&autorelease, sd_t, ID_CTMB, exp_ctopmidbottom);
	// mb+x: mb+cb mb+cm mb+ct mb+cmb mb+ctb mb+ctm mb+ctmb (7)
	TOGGLE(&autorelease, sd_mb, ID_B, exp_bottom, exp_empty);
	TOGGLE(&autorelease, sd_mb, ID_M, exp_mid, exp_empty);
	ADD(&autorelease, sd_mb, ID_T, exp_top_midbottom);
	ADD(&autorelease, sd_mb, ID_MB, exp_midbottom); DEL(&autorelease, sd_mb, ID_MB, exp_empty);
	TOGGLE(&autorelease, sd_mb, ID_TB, exp_topbottom, exp_empty);
	TOGGLE(&autorelease, sd_mb, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_mb, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_mb_cb = ADD(&autorelease, sd_mb, ID_CB, exp_cbottom);
	struct map_session_data *sd_mb_cm = ADD(&autorelease, sd_mb, ID_CM, exp_cmid);
	struct map_session_data *sd_mb_ct = ADD(&autorelease, sd_mb, ID_CT, exp_ctop_midbottom);
	struct map_session_data *sd_mb_cmb = ADD(&autorelease, sd_mb, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_mb_ctb = ADD(&autorelease, sd_mb, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_mb_ctm = ADD(&autorelease, sd_mb, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_mb_ctmb = ADD(&autorelease, sd_mb, ID_CTMB, exp_ctopmidbottom);
	// tb+x: tb+cb tb+cm tb+ct tb+cmb tb+ctb tb+ctm tb+ctmb (7)
	TOGGLE(&autorelease, sd_tb, ID_B, exp_bottom, exp_empty);
	ADD(&autorelease, sd_tb, ID_M, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_tb, ID_T, exp_top, exp_empty);
	TOGGLE(&autorelease, sd_tb, ID_MB, exp_midbottom, exp_empty);
	ADD(&autorelease, sd_tb, ID_TB, exp_topbottom); DEL(&autorelease, sd_tb, ID_TB, exp_empty);
	TOGGLE(&autorelease, sd_tb, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_tb, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_tb_cb = ADD(&autorelease, sd_tb, ID_CB, exp_cbottom);
	struct map_session_data *sd_tb_cm = ADD(&autorelease, sd_tb, ID_CM, exp_cmid_topbottom);
	struct map_session_data *sd_tb_ct = ADD(&autorelease, sd_tb, ID_CT, exp_ctop);
	struct map_session_data *sd_tb_cmb = ADD(&autorelease, sd_tb, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_tb_ctb = ADD(&autorelease, sd_tb, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_tb_ctm = ADD(&autorelease, sd_tb, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_tb_ctmb = ADD(&autorelease, sd_tb, ID_CTMB, exp_ctopmidbottom);
	// tm+x: tm+cb tm+cm tm+ct tm+cmb tm+ctb tm+ctm tm+ctmb (7)
	ADD(&autorelease, sd_tm, ID_B, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_tm, ID_M, exp_mid, exp_empty);
	TOGGLE(&autorelease, sd_tm, ID_T, exp_top, exp_empty);
	TOGGLE(&autorelease, sd_tm, ID_MB, exp_midbottom, exp_empty);
	TOGGLE(&autorelease, sd_tm, ID_TB, exp_topbottom, exp_empty);
	ADD(&autorelease, sd_tm, ID_TM, exp_topmid); DEL(&autorelease, sd_tm, ID_TM, exp_empty);
	TOGGLE(&autorelease, sd_tm, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_tm_cb = ADD(&autorelease, sd_tm, ID_CB, exp_cbottom_topmid);
	struct map_session_data *sd_tm_cm = ADD(&autorelease, sd_tm, ID_CM, exp_cmid);
	struct map_session_data *sd_tm_ct = ADD(&autorelease, sd_tm, ID_CT, exp_ctop);
	struct map_session_data *sd_tm_cmb = ADD(&autorelease, sd_tm, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_tm_ctb = ADD(&autorelease, sd_tm, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_tm_ctm = ADD(&autorelease, sd_tm, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_tm_ctmb = ADD(&autorelease, sd_tm, ID_CTMB, exp_ctopmidbottom);
	// tmb+x: tmb+cb tmb+ct tmb+cm tmb+ctb tmb+ctm tmb+cmb tmb+ctmb (7)
	TOGGLE(&autorelease, sd_tmb, ID_B, exp_bottom, exp_empty);
	TOGGLE(&autorelease, sd_tmb, ID_M, exp_mid, exp_empty);
	TOGGLE(&autorelease, sd_tmb, ID_T, exp_top, exp_empty);
	TOGGLE(&autorelease, sd_tmb, ID_MB, exp_midbottom, exp_empty);
	TOGGLE(&autorelease, sd_tmb, ID_TB, exp_topbottom, exp_empty);
	TOGGLE(&autorelease, sd_tmb, ID_TM, exp_topmid, exp_empty);
	ADD(&autorelease, sd_tmb, ID_TMB, exp_topmidbottom); DEL(&autorelease, sd_tmb, ID_TMB, exp_empty);
	struct map_session_data *sd_tmb_cb = ADD(&autorelease, sd_tmb, ID_CB, exp_cbottom);
	struct map_session_data *sd_tmb_cm = ADD(&autorelease, sd_tmb, ID_CM, exp_cmid);
	struct map_session_data *sd_tmb_ct = ADD(&autorelease, sd_tmb, ID_CT, exp_ctop);
	struct map_session_data *sd_tmb_cmb = ADD(&autorelease, sd_tmb, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_tmb_ctb = ADD(&autorelease, sd_tmb, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_tmb_ctm = ADD(&autorelease, sd_tmb, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_tmb_ctmb = ADD(&autorelease, sd_tmb, ID_CTMB, exp_ctopmidbottom);
	// cb+x: cb+cm cb+ct cb+ctm (3)
	ADD(&autorelease, sd_cb, ID_B, exp_cbottom);
	ADD(&autorelease, sd_cb, ID_M, exp_cbottom_mid);
	ADD(&autorelease, sd_cb, ID_T, exp_cbottom_top);
	ADD(&autorelease, sd_cb, ID_MB, exp_cbottom);
	ADD(&autorelease, sd_cb, ID_TB, exp_cbottom);
	ADD(&autorelease, sd_cb, ID_TM, exp_cbottom_topmid);
	ADD(&autorelease, sd_cb, ID_TMB, exp_cbottom);
	ADD(&autorelease, sd_cb, ID_CB, exp_cbottom); DEL(&autorelease, sd_cb, ID_CB, exp_empty);
	struct map_session_data *sd_cb_cm = ADD(&autorelease, sd_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_cb_ct = ADD(&autorelease, sd_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_cb, ID_CMB, exp_cmidbottom, exp_empty);
	TOGGLE(&autorelease, sd_cb, ID_CTB, exp_ctopbottom, exp_empty);
	struct map_session_data *sd_cb_ctm = ADD(&autorelease, sd_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_cb, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// cm+x: cm+ct cm+ctb (2)
	ADD(&autorelease, sd_cm, ID_B, exp_cmid_bottom);
	ADD(&autorelease, sd_cm, ID_M, exp_cmid);
	ADD(&autorelease, sd_cm, ID_T, exp_cmid_top);
	ADD(&autorelease, sd_cm, ID_MB, exp_cmid);
	ADD(&autorelease, sd_cm, ID_TB, exp_cmid_topbottom);
	ADD(&autorelease, sd_cm, ID_TM, exp_cmid);
	ADD(&autorelease, sd_cm, ID_TMB, exp_cmid);
	ADD(&autorelease, sd_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_cm, ID_CM, exp_cmid); DEL(&autorelease, sd_cm, ID_CM, exp_empty);
	struct map_session_data *sd_cm_ct = ADD(&autorelease, sd_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_cm, ID_CMB, exp_cmidbottom, exp_empty);
	struct map_session_data *sd_cm_ctb = ADD(&autorelease, sd_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_cm, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_cm, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// ct+x: ct+cmb (1)
	ADD(&autorelease, sd_ct, ID_B, exp_ctop_bottom);
	ADD(&autorelease, sd_ct, ID_M, exp_ctop_mid);
	ADD(&autorelease, sd_ct, ID_T, exp_ctop);
	ADD(&autorelease, sd_ct, ID_MB, exp_ctop_midbottom);
	ADD(&autorelease, sd_ct, ID_TB, exp_ctop);
	ADD(&autorelease, sd_ct, ID_TM, exp_ctop);
	ADD(&autorelease, sd_ct, ID_TMB, exp_ctop);
	ADD(&autorelease, sd_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_ct, ID_CT, exp_ctop); DEL(&autorelease, sd_ct, ID_CT, exp_empty);
	struct map_session_data *sd_ct_cmb = ADD(&autorelease, sd_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_ct, ID_CTB, exp_ctopbottom, exp_empty);
	TOGGLE(&autorelease, sd_ct, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_ct, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// cmb+x
	ADD(&autorelease, sd_cmb, ID_B, exp_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_M, exp_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_T, exp_cmidbottom_top);
	ADD(&autorelease, sd_cmb, ID_MB, exp_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_TB, exp_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_TM, exp_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_TMB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_cmb, ID_CB, exp_cbottom, exp_empty);
	TOGGLE(&autorelease, sd_cmb, ID_CM, exp_cmid, exp_empty);
	ADD(&autorelease, sd_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_cmb, ID_CMB, exp_empty);
	TOGGLE(&autorelease, sd_cmb, ID_CTB, exp_ctopbottom, exp_empty);
	TOGGLE(&autorelease, sd_cmb, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_cmb, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// ctb+x
	ADD(&autorelease, sd_ctb, ID_B, exp_ctopbottom);
	ADD(&autorelease, sd_ctb, ID_M, exp_ctopbottom_mid);
	ADD(&autorelease, sd_ctb, ID_T, exp_ctopbottom);
	ADD(&autorelease, sd_ctb, ID_MB, exp_ctopbottom);
	ADD(&autorelease, sd_ctb, ID_TB, exp_ctopbottom);
	ADD(&autorelease, sd_ctb, ID_TM, exp_ctopbottom);
	ADD(&autorelease, sd_ctb, ID_TMB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_ctb, ID_CB, exp_cbottom, exp_empty);
	ADD(&autorelease, sd_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_ctb, ID_CT, exp_ctop, exp_empty);
	TOGGLE(&autorelease, sd_ctb, ID_CMB, exp_cmidbottom, exp_empty);
	ADD(&autorelease, sd_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_ctb, ID_CTB, exp_empty);
	TOGGLE(&autorelease, sd_ctb, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_ctb, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// ctm+x
	ADD(&autorelease, sd_ctm, ID_B, exp_ctopmid_bottom);
	ADD(&autorelease, sd_ctm, ID_M, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_T, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_MB, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_TB, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_TM, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_TMB, exp_ctopmid);
	ADD(&autorelease, sd_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_ctm, ID_CM, exp_cmid, exp_empty);
	TOGGLE(&autorelease, sd_ctm, ID_CT, exp_ctop, exp_empty);
	TOGGLE(&autorelease, sd_ctm, ID_CMB, exp_cmidbottom, exp_empty);
	TOGGLE(&autorelease, sd_ctm, ID_CTB, exp_ctopbottom, exp_empty);
	ADD(&autorelease, sd_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_ctm, ID_CTM, exp_empty);
	TOGGLE(&autorelease, sd_ctm, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// ctmb+x
	ADD(&autorelease, sd_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_T, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_MB, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_TB, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_TM, exp_ctopmidbottom);
	ADD(&autorelease, sd_ctmb, ID_TMB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_ctmb, ID_CB, exp_cbottom, exp_empty);
	TOGGLE(&autorelease, sd_ctmb, ID_CM, exp_cmid, exp_empty);
	TOGGLE(&autorelease, sd_ctmb, ID_CT, exp_ctop, exp_empty);
	TOGGLE(&autorelease, sd_ctmb, ID_CMB, exp_cmidbottom, exp_empty);
	TOGGLE(&autorelease, sd_ctmb, ID_CTB, exp_ctopbottom, exp_empty);
	TOGGLE(&autorelease, sd_ctmb, ID_CTM, exp_ctopmid, exp_empty);
	ADD(&autorelease, sd_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_ctmb, ID_CTMB, exp_empty);

	// Three: (86)
	// b+m+x: b+m+t b+m+cb b+m+ct b+m+cm b+m+ctb b+m+ctm b+m+cmb b+m+ctmb [8]
	ADD(&autorelease, sd_b_m, ID_B, exp_mid_bottom); DEL(&autorelease, sd_b_m, ID_B, exp_mid);
	ADD(&autorelease, sd_b_m, ID_M, exp_mid_bottom); DEL(&autorelease, sd_b_m, ID_M, exp_bottom);
	struct map_session_data *sd_b_m_t = ADD(&autorelease, sd_b_m, ID_T, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m, ID_MB, exp_midbottom, exp_empty);
	TOGGLE(&autorelease, sd_b_m, ID_TB, exp_topbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_b_m, ID_TM, exp_topmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_m, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_b_m_cb = ADD(&autorelease, sd_b_m, ID_CB, exp_cbottom_mid);
	struct map_session_data *sd_b_m_cm = ADD(&autorelease, sd_b_m, ID_CM, exp_cmid_bottom);
	struct map_session_data *sd_b_m_ct = ADD(&autorelease, sd_b_m, ID_CT, exp_ctop_mid_bottom);
	struct map_session_data *sd_b_m_cmb = ADD(&autorelease, sd_b_m, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_b_m_ctb = ADD(&autorelease, sd_b_m, ID_CTB, exp_ctopbottom_mid);
	struct map_session_data *sd_b_m_ctm = ADD(&autorelease, sd_b_m, ID_CTM, exp_ctopmid_bottom);
	struct map_session_data *sd_b_m_ctmb = ADD(&autorelease, sd_b_m, ID_CTMB, exp_ctopmidbottom);
	// b+t+x: b+t+cb b+t+ct b+t+cm b+t+ctb b+t+ctm b+t+cmb b+t+ctmb [7]
	ADD(&autorelease, sd_b_t, ID_B, exp_top_bottom); DEL(&autorelease, sd_b_t, ID_B, exp_top);
	ADD(&autorelease, sd_b_t, ID_M, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_t, ID_T, exp_top_bottom); DEL(&autorelease, sd_b_t, ID_T, exp_bottom);
	TOGGLE(&autorelease, sd_b_t, ID_MB, exp_top_midbottom, exp_top);
	TOGGLE(&autorelease, sd_b_t, ID_TB, exp_topbottom, exp_empty);
	TOGGLE(&autorelease, sd_b_t, ID_TM, exp_topmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_t, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_b_t_cb = ADD(&autorelease, sd_b_t, ID_CB, exp_cbottom_top);
	struct map_session_data *sd_b_t_cm = ADD(&autorelease, sd_b_t, ID_CM, exp_cmid_top_bottom);
	struct map_session_data *sd_b_t_ct = ADD(&autorelease, sd_b_t, ID_CT, exp_ctop_bottom);
	struct map_session_data *sd_b_t_cmb = ADD(&autorelease, sd_b_t, ID_CMB, exp_cmidbottom_top);
	struct map_session_data *sd_b_t_ctb = ADD(&autorelease, sd_b_t, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_b_t_ctm = ADD(&autorelease, sd_b_t, ID_CTM, exp_ctopmid_bottom);
	struct map_session_data *sd_b_t_ctmb = ADD(&autorelease, sd_b_t, ID_CTMB, exp_ctopmidbottom);
	// b+tm+x: b+tm+cb b+tm+ct b+tm+cm b+tm+ctb b+tm+ctm b+tm+cmb b+tm+ctmb [7]
	ADD(&autorelease, sd_b_tm, ID_B, exp_topmid_bottom); DEL(&autorelease, sd_b_tm, ID_B, exp_topmid);
	TOGGLE(&autorelease, sd_b_tm, ID_M, exp_mid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_tm, ID_T, exp_top_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_tm, ID_MB, exp_midbottom, exp_empty);
	TOGGLE(&autorelease, sd_b_tm, ID_TB, exp_topbottom, exp_empty);
	ADD(&autorelease, sd_b_tm, ID_TM, exp_topmid_bottom); DEL(&autorelease, sd_b_tm, ID_TM, exp_bottom);
	TOGGLE(&autorelease, sd_b_tm, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_b_tm_cb = ADD(&autorelease, sd_b_tm, ID_CB, exp_cbottom_topmid);
	struct map_session_data *sd_b_tm_cm = ADD(&autorelease, sd_b_tm, ID_CM, exp_cmid_bottom);
	struct map_session_data *sd_b_tm_ct = ADD(&autorelease, sd_b_tm, ID_CT, exp_ctop_bottom);
	struct map_session_data *sd_b_tm_cmb = ADD(&autorelease, sd_b_tm, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_b_tm_ctb = ADD(&autorelease, sd_b_tm, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_b_tm_ctm = ADD(&autorelease, sd_b_tm, ID_CTM, exp_ctopmid_bottom);
	struct map_session_data *sd_b_tm_ctmb = ADD(&autorelease, sd_b_tm, ID_CTMB, exp_ctopmidbottom);
	// b+cb+x: b+cb+cm b+cb+ct b+cb+ctm [3]
	ADD(&autorelease, sd_b_cb, ID_B, exp_cbottom); DEL(&autorelease, sd_b_cb, ID_B, exp_cbottom);
	ADD(&autorelease, sd_b_cb, ID_M, exp_cbottom_mid);
	ADD(&autorelease, sd_b_cb, ID_T, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_cb, ID_MB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb, ID_TB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_cb, ID_TM, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_cb, ID_CB, exp_cbottom); DEL(&autorelease, sd_b_cb, ID_CB, exp_bottom);
	struct map_session_data *sd_b_cb_cm = ADD(&autorelease, sd_b_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_b_cb_ct = ADD(&autorelease, sd_b_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_cb, ID_CMB, exp_cmidbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cb, ID_CTB, exp_ctopbottom, exp_bottom);
	struct map_session_data *sd_b_cb_ctm = ADD(&autorelease, sd_b_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+cm+x: b+cm+ct b+cm+ctb [2]
	ADD(&autorelease, sd_b_cm, ID_B, exp_cmid_bottom); DEL(&autorelease, sd_b_cm, ID_B, exp_cmid);
	ADD(&autorelease, sd_b_cm, ID_M, exp_cmid_bottom);
	ADD(&autorelease, sd_b_cm, ID_T, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_cm, ID_MB, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_b_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	ADD(&autorelease, sd_b_cm, ID_TM, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_b_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_cm, ID_CM, exp_cmid_bottom); DEL(&autorelease, sd_b_cm, ID_CM, exp_bottom);
	struct map_session_data *sd_b_cm_ct = ADD(&autorelease, sd_b_cm, ID_CT, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm, ID_CMB, exp_cmidbottom, exp_bottom);
	struct map_session_data *sd_b_cm_ctb = ADD(&autorelease, sd_b_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_cm, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cm, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+ct+x: b+ct+cmb [1]
	ADD(&autorelease, sd_b_ct, ID_B, exp_ctop_bottom); DEL(&autorelease, sd_b_ct, ID_B, exp_ctop);
	ADD(&autorelease, sd_b_ct, ID_M, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_ct, ID_T, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_b_ct, ID_TB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_ct, ID_TM, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_ct, ID_CM, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_ct, ID_CT, exp_ctop_bottom); DEL(&autorelease, sd_b_ct, ID_CT, exp_bottom);
	struct map_session_data *sd_b_ct_cmb = ADD(&autorelease, sd_b_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_ct, ID_CTB, exp_ctopbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ct, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ct, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// m+t+x: m+t+cb m+t+cm m+t+ct m+t+cmb m+t+cmt m+t+ctb m+t+cmtb [7]
	ADD(&autorelease, sd_m_t, ID_B, exp_top_mid_bottom);
	ADD(&autorelease, sd_m_t, ID_M, exp_top_mid); DEL(&autorelease, sd_m_t, ID_M, exp_top);
	ADD(&autorelease, sd_m_t, ID_T, exp_top_mid); DEL(&autorelease, sd_m_t, ID_T, exp_mid);
	TOGGLE(&autorelease, sd_m_t, ID_MB, exp_top_midbottom, exp_top);
	TOGGLE(&autorelease, sd_m_t, ID_TB, exp_topbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_t, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_m_t, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_m_t_cb = ADD(&autorelease, sd_m_t, ID_CB, exp_cbottom_top_mid);
	struct map_session_data *sd_m_t_cm = ADD(&autorelease, sd_m_t, ID_CM, exp_cmid_top);
	struct map_session_data *sd_m_t_ct = ADD(&autorelease, sd_m_t, ID_CT, exp_ctop_mid);
	struct map_session_data *sd_m_t_cmb = ADD(&autorelease, sd_m_t, ID_CMB, exp_cmidbottom_top);
	struct map_session_data *sd_m_t_ctb = ADD(&autorelease, sd_m_t, ID_CTB, exp_ctopbottom_mid);
	struct map_session_data *sd_m_t_ctm = ADD(&autorelease, sd_m_t, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_m_t_ctmb = ADD(&autorelease, sd_m_t, ID_CTMB, exp_ctopmidbottom);
	// m+tb+x: m+tb+cb m+tb+cm m+tb+ct m+tb+cmb m+tb+cmt m+tb+ctb m+tb+cmtb [7]
	TOGGLE(&autorelease, sd_m_tb, ID_B, exp_mid_bottom, exp_mid);
	ADD(&autorelease, sd_m_tb, ID_M, exp_topbottom_mid); DEL(&autorelease, sd_m_tb, ID_M, exp_topbottom);
	TOGGLE(&autorelease, sd_m_tb, ID_T, exp_top_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_tb, ID_MB, exp_midbottom, exp_empty);
	ADD(&autorelease, sd_m_tb, ID_TB, exp_topbottom_mid); DEL(&autorelease, sd_m_tb, ID_TB, exp_mid);
	TOGGLE(&autorelease, sd_m_tb, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_m_tb, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_m_tb_cb = ADD(&autorelease, sd_m_tb, ID_CB, exp_cbottom_mid);
	struct map_session_data *sd_m_tb_cm = ADD(&autorelease, sd_m_tb, ID_CM, exp_cmid_topbottom);
	struct map_session_data *sd_m_tb_ct = ADD(&autorelease, sd_m_tb, ID_CT, exp_ctop_mid);
	struct map_session_data *sd_m_tb_cmb = ADD(&autorelease, sd_m_tb, ID_CMB, exp_cmidbottom);
	struct map_session_data *sd_m_tb_ctb = ADD(&autorelease, sd_m_tb, ID_CTB, exp_ctopbottom_mid);
	struct map_session_data *sd_m_tb_ctm = ADD(&autorelease, sd_m_tb, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_m_tb_ctmb = ADD(&autorelease, sd_m_tb, ID_CTMB, exp_ctopmidbottom);
	// m+cb+x: m+cb+cm m+cb+ct m+cb+cmt [3]
	ADD(&autorelease, sd_m_cb, ID_B, exp_cbottom_mid);
	ADD(&autorelease, sd_m_cb, ID_M, exp_cbottom_mid); DEL(&autorelease, sd_m_cb, ID_M, exp_cbottom);
	ADD(&autorelease, sd_m_cb, ID_T, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_cb, ID_MB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_m_cb, ID_TB, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_m_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_m_cb, ID_CB, exp_cbottom_mid); DEL(&autorelease, sd_m_cb, ID_CB, exp_mid);
	struct map_session_data *sd_m_cb_cm = ADD(&autorelease, sd_m_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_m_cb_ct = ADD(&autorelease, sd_m_cb, ID_CT, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb, ID_CMB, exp_cmidbottom, exp_mid);
	TOGGLE(&autorelease, sd_m_cb, ID_CTB, exp_ctopbottom_mid, exp_mid);
	struct map_session_data *sd_m_cb_ctm = ADD(&autorelease, sd_m_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+cm+x: m+cm+ct m+cm+ctb [2]
	ADD(&autorelease, sd_m_cm, ID_B, exp_cmid_bottom);
	ADD(&autorelease, sd_m_cm, ID_M, exp_cmid); DEL(&autorelease, sd_m_cm, ID_M, exp_cmid);
	ADD(&autorelease, sd_m_cm, ID_T, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_cm, ID_MB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_m_cm, ID_TB, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_m_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_cm, ID_CM, exp_cmid); DEL(&autorelease, sd_m_cm, ID_CM, exp_mid);
	struct map_session_data *sd_m_cm_ct = ADD(&autorelease, sd_m_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_cm, ID_CMB, exp_cmidbottom, exp_mid);
	struct map_session_data *sd_m_cm_ctb = ADD(&autorelease, sd_m_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_cm, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_cm, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+ct+x: m+ct+cmb [1]
	ADD(&autorelease, sd_m_ct, ID_B, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_m_ct, ID_M, exp_ctop_mid); DEL(&autorelease, sd_m_ct, ID_M, exp_ctop);
	ADD(&autorelease, sd_m_ct, ID_T, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	ADD(&autorelease, sd_m_ct, ID_TB, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_m_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_m_ct, ID_CB, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_m_ct, ID_CT, exp_ctop_mid); DEL(&autorelease, sd_m_ct, ID_CT, exp_mid);
	struct map_session_data *sd_m_ct_cmb = ADD(&autorelease, sd_m_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_ct, ID_CTB, exp_ctopbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ct, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_ct, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// t+mb+x: t+mb+cb t+mb+cm t+mb+ct t+mb+cmb t+mb+cmt t+mb+ctb t+mb+cmtb [7]
	TOGGLE(&autorelease, sd_t_mb, ID_B, exp_top_bottom, exp_top);
	TOGGLE(&autorelease, sd_t_mb, ID_M, exp_top_mid, exp_top);
	ADD(&autorelease, sd_t_mb, ID_T, exp_top_midbottom); DEL(&autorelease, sd_t_mb, ID_T, exp_midbottom);
	ADD(&autorelease, sd_t_mb, ID_MB, exp_top_midbottom); DEL(&autorelease, sd_t_mb, ID_MB, exp_top);
	TOGGLE(&autorelease, sd_t_mb, ID_TB, exp_topbottom, exp_empty);
	TOGGLE(&autorelease, sd_t_mb, ID_TM, exp_topmid, exp_empty);
	TOGGLE(&autorelease, sd_t_mb, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_t_mb_cb = ADD(&autorelease, sd_t_mb, ID_CB, exp_cbottom_top);
	struct map_session_data *sd_t_mb_cm = ADD(&autorelease, sd_t_mb, ID_CM, exp_cmid_top);
	struct map_session_data *sd_t_mb_ct = ADD(&autorelease, sd_t_mb, ID_CT, exp_ctop_midbottom);
	struct map_session_data *sd_t_mb_cmb = ADD(&autorelease, sd_t_mb, ID_CMB, exp_cmidbottom_top);
	struct map_session_data *sd_t_mb_ctb = ADD(&autorelease, sd_t_mb, ID_CTB, exp_ctopbottom);
	struct map_session_data *sd_t_mb_ctm = ADD(&autorelease, sd_t_mb, ID_CTM, exp_ctopmid);
	struct map_session_data *sd_t_mb_ctmb = ADD(&autorelease, sd_t_mb, ID_CTMB, exp_ctopmidbottom);
	// t+cb+x: t+cb+cm t+cb+ct t+cb+cmt [3]
	ADD(&autorelease, sd_t_cb, ID_B, exp_cbottom_top);
	ADD(&autorelease, sd_t_cb, ID_M, exp_cbottom_top_mid);
	ADD(&autorelease, sd_t_cb, ID_T, exp_cbottom_top); DEL(&autorelease, sd_t_cb, ID_T, exp_cbottom);
	ADD(&autorelease, sd_t_cb, ID_MB, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb, ID_TB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_t_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_t_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_t_cb, ID_CB, exp_cbottom_top); DEL(&autorelease, sd_t_cb, ID_CB, exp_top);
	struct map_session_data *sd_t_cb_cm = ADD(&autorelease, sd_t_cb, ID_CM, exp_cmid_cbottom_top);
	struct map_session_data *sd_t_cb_ct = ADD(&autorelease, sd_t_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_cb, ID_CMB, exp_cmidbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_cb, ID_CTB, exp_ctopbottom, exp_top);
	struct map_session_data *sd_t_cb_ctm = ADD(&autorelease, sd_t_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+cm+x: t+cm+ct t+cm+ctb [2]
	ADD(&autorelease, sd_t_cm, ID_B, exp_cmid_top_bottom);
	ADD(&autorelease, sd_t_cm, ID_M, exp_cmid_top);
	ADD(&autorelease, sd_t_cm, ID_T, exp_cmid_top); DEL(&autorelease, sd_t_cm, ID_T, exp_cmid);
	ADD(&autorelease, sd_t_cm, ID_MB, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_t_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_t_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_t_cm, ID_CB, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_t_cm, ID_CM, exp_cmid_top); DEL(&autorelease, sd_t_cm, ID_CM, exp_top);
	struct map_session_data *sd_t_cm_ct = ADD(&autorelease, sd_t_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_cm, ID_CMB, exp_cmidbottom_top, exp_top);
	struct map_session_data *sd_t_cm_ctb = ADD(&autorelease, sd_t_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_cm, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_cm, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+ct+x: t+ct+cmb [1]
	ADD(&autorelease, sd_t_ct, ID_B, exp_ctop_bottom);
	ADD(&autorelease, sd_t_ct, ID_M, exp_ctop_mid);
	ADD(&autorelease, sd_t_ct, ID_T, exp_ctop); DEL(&autorelease, sd_t_ct, ID_T, exp_ctop);
	ADD(&autorelease, sd_t_ct, ID_MB, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_ct, ID_TB, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_t_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_t_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_t_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_t_ct, ID_CT, exp_ctop); DEL(&autorelease, sd_t_ct, ID_CT, exp_top);
	struct map_session_data *sd_t_ct_cmb = ADD(&autorelease, sd_t_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_ct, ID_CTB, exp_ctopbottom, exp_top);
	TOGGLE(&autorelease, sd_t_ct, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_ct, ID_CTMB, exp_ctopmidbottom, exp_top);
	// mb+cb+x: mb+cb+cm mb+cb+ct mb+cb+ctm [3]
	TOGGLE(&autorelease, sd_mb_cb, ID_B, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_M, exp_cbottom_mid, exp_cbottom);
	ADD(&autorelease, sd_mb_cb, ID_T, exp_cbottom_top);
	ADD(&autorelease, sd_mb_cb, ID_MB, exp_cbottom); DEL(&autorelease, sd_mb_cb, ID_MB, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_TB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_mb_cb, ID_CB, exp_cbottom); DEL(&autorelease, sd_mb_cb, ID_CB, exp_midbottom);
	struct map_session_data *sd_mb_cb_cm = ADD(&autorelease, sd_mb_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_mb_cb_ct = ADD(&autorelease, sd_mb_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_CMB, exp_cmidbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_CTB, exp_ctopbottom, exp_midbottom);
	struct map_session_data *sd_mb_cb_ctm = ADD(&autorelease, sd_mb_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+cm+x: mb+cm+ct mb+cm+ctb [2]
	TOGGLE(&autorelease, sd_mb_cm, ID_B, exp_cmid_bottom, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_M, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_mb_cm, ID_T, exp_cmid_top);
	ADD(&autorelease, sd_mb_cm, ID_MB, exp_cmid); DEL(&autorelease, sd_mb_cm, ID_MB, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_mb_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_mb_cm, ID_CM, exp_cmid); DEL(&autorelease, sd_mb_cm, ID_CM, exp_midbottom);
	struct map_session_data *sd_mb_cm_ct = ADD(&autorelease, sd_mb_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_CMB, exp_cmidbottom, exp_midbottom);
	struct map_session_data *sd_mb_cm_ctb = ADD(&autorelease, sd_mb_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cm, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+ct+x: mb+ct+cmb [1]
	TOGGLE(&autorelease, sd_mb_ct, ID_B, exp_ctop_bottom, exp_ctop);
	TOGGLE(&autorelease, sd_mb_ct, ID_M, exp_ctop_mid, exp_ctop);
	ADD(&autorelease, sd_mb_ct, ID_T, exp_ctop_midbottom);
	ADD(&autorelease, sd_mb_ct, ID_MB, exp_ctop_midbottom); DEL(&autorelease, sd_mb_ct, ID_MB, exp_ctop);
	TOGGLE(&autorelease, sd_mb_ct, ID_TB, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_mb_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_mb_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_mb_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_mb_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_mb_ct, ID_CT, exp_ctop_midbottom); DEL(&autorelease, sd_mb_ct, ID_CT, exp_midbottom);
	struct map_session_data *sd_mb_ct_cmb = ADD(&autorelease, sd_mb_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct, ID_CTB, exp_ctopbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ct, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ct, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// tb+cb+x: tb+cb+cm tb+cb+ct tb+cb+ctm [3]
	TOGGLE(&autorelease, sd_tb_cb, ID_B, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_tb_cb, ID_M, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_tb_cb, ID_T, exp_cbottom_top, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_MB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_tb_cb, ID_TB, exp_cbottom); DEL(&autorelease, sd_tb_cb, ID_TB, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_tb_cb, ID_CB, exp_cbottom); DEL(&autorelease, sd_tb_cb, ID_CB, exp_topbottom);
	struct map_session_data *sd_tb_cb_cm = ADD(&autorelease, sd_tb_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_tb_cb_ct = ADD(&autorelease, sd_tb_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_CMB, exp_cmidbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_CTB, exp_ctopbottom, exp_topbottom);
	struct map_session_data *sd_tb_cb_ctm = ADD(&autorelease, sd_tb_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+cm+x: tb+cm+ct tb+cm+ctb [2]
	TOGGLE(&autorelease, sd_tb_cm, ID_B, exp_cmid_bottom, exp_cmid);
	ADD(&autorelease, sd_tb_cm, ID_M, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cm, ID_T, exp_cmid_top, exp_cmid);
	TOGGLE(&autorelease, sd_tb_cm, ID_MB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_tb_cm, ID_TB, exp_cmid_topbottom); DEL(&autorelease, sd_tb_cm, ID_TB, exp_cmid);
	TOGGLE(&autorelease, sd_tb_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tb_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_tb_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_tb_cm, ID_CM, exp_cmid_topbottom); DEL(&autorelease, sd_tb_cm, ID_CM, exp_topbottom);
	struct map_session_data *sd_tb_cm_ct = ADD(&autorelease, sd_tb_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tb_cm, ID_CMB, exp_cmidbottom, exp_topbottom);
	struct map_session_data *sd_tb_cm_ctb = ADD(&autorelease, sd_tb_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cm, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+ct+x: tb+ct+cmb [1]
	TOGGLE(&autorelease, sd_tb_ct, ID_B, exp_ctop_bottom, exp_ctop);
	ADD(&autorelease, sd_tb_ct, ID_M, exp_ctop_mid);
	TOGGLE(&autorelease, sd_tb_ct, ID_T, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_tb_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	ADD(&autorelease, sd_tb_ct, ID_TB, exp_ctop); DEL(&autorelease, sd_tb_ct, ID_TB, exp_ctop);
	TOGGLE(&autorelease, sd_tb_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_tb_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_tb_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_tb_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_tb_ct, ID_CT, exp_ctop); DEL(&autorelease, sd_tb_ct, ID_CT, exp_topbottom);
	struct map_session_data *sd_tb_ct_cmb = ADD(&autorelease, sd_tb_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct, ID_CTB, exp_ctopbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ct, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tm+cb+x: tm+cb+cm tm+cb+ct tm+cb+ctm [3]
	ADD(&autorelease, sd_tm_cb, ID_B, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb, ID_M, exp_cbottom_mid, exp_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_T, exp_cbottom_top, exp_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_MB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_TB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_tm_cb, ID_TM, exp_cbottom_topmid); DEL(&autorelease, sd_tm_cb, ID_TM, exp_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_tm_cb, ID_CB, exp_cbottom_topmid); DEL(&autorelease, sd_tm_cb, ID_CB, exp_topmid);
	struct map_session_data *sd_tm_cb_cm = ADD(&autorelease, sd_tm_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_tm_cb_ct = ADD(&autorelease, sd_tm_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_CMB, exp_cmidbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cb, ID_CTB, exp_ctopbottom, exp_topmid);
	struct map_session_data *sd_tm_cb_ctm = ADD(&autorelease, sd_tm_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+cm+x: tm+cm+ct tm+cm+ctb [2]
	ADD(&autorelease, sd_tm_cm, ID_B, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_tm_cm, ID_M, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_T, exp_cmid_top, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_MB, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	ADD(&autorelease, sd_tm_cm, ID_TM, exp_cmid); DEL(&autorelease, sd_tm_cm, ID_TM, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_tm_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_tm_cm, ID_CM, exp_cmid); DEL(&autorelease, sd_tm_cm, ID_CM, exp_topmid);
	struct map_session_data *sd_tm_cm_ct = ADD(&autorelease, sd_tm_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_CMB, exp_cmidbottom, exp_topmid);
	struct map_session_data *sd_tm_cm_ctb = ADD(&autorelease, sd_tm_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cm, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+ct+x: tm+ct+cmb [1]
	ADD(&autorelease, sd_tm_ct, ID_B, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_tm_ct, ID_M, exp_ctop_mid, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct, ID_T, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct, ID_TB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_tm_ct, ID_TM, exp_ctop); DEL(&autorelease, sd_tm_ct, ID_TM, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_tm_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_tm_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_tm_ct, ID_CT, exp_ctop); DEL(&autorelease, sd_tm_ct, ID_CT, exp_topmid);
	struct map_session_data *sd_tm_ct_cmb = ADD(&autorelease, sd_tm_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct, ID_CTB, exp_ctopbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ct, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tmb+cb+x: tmb+cb+ct tmb+cb+cm tmb+cb+ctm [3]
	TOGGLE(&autorelease, sd_tmb_cb, ID_B, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_M, exp_cbottom_mid, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_T, exp_cbottom_top, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_MB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_TB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	ADD(&autorelease, sd_tmb_cb, ID_TMB, exp_cbottom); DEL(&autorelease, sd_tmb_cb, ID_TMB, exp_cbottom);
	ADD(&autorelease, sd_tmb_cb, ID_CB, exp_cbottom); DEL(&autorelease, sd_tmb_cb, ID_CB, exp_topmidbottom);
	struct map_session_data *sd_tmb_cb_cm = ADD(&autorelease, sd_tmb_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_tmb_cb_ct = ADD(&autorelease, sd_tmb_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	struct map_session_data *sd_tmb_cb_ctm = ADD(&autorelease, sd_tmb_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+cm+x: tmb+cm+ct tmb+cm+ctb [2]
	TOGGLE(&autorelease, sd_tmb_cm, ID_B, exp_cmid_bottom, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_M, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_T, exp_cmid_top, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_MB, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_TM, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_tmb_cm, ID_TMB, exp_cmid); DEL(&autorelease, sd_tmb_cm, ID_TMB, exp_cmid);
	ADD(&autorelease, sd_tmb_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cm, ID_CM, exp_cmid); DEL(&autorelease, sd_tmb_cm, ID_CM, exp_topmidbottom);
	struct map_session_data *sd_tmb_cm_ct = ADD(&autorelease, sd_tmb_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	struct map_session_data *sd_tmb_cm_ctb = ADD(&autorelease, sd_tmb_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cm, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+ct:x tmb+ct+cmb [1]
	TOGGLE(&autorelease, sd_tmb_ct, ID_B, exp_ctop_bottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct, ID_M, exp_ctop_mid, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct, ID_T, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct, ID_TB, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct, ID_TM, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_tmb_ct, ID_TMB, exp_ctop); DEL(&autorelease, sd_tmb_ct, ID_TMB, exp_ctop);
	ADD(&autorelease, sd_tmb_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_tmb_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_tmb_ct, ID_CT, exp_ctop); DEL(&autorelease, sd_tmb_ct, ID_CT, exp_topmidbottom);
	struct map_session_data *sd_tmb_ct_cmb = ADD(&autorelease, sd_tmb_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// cb+cm+x: cb+cm+ct [1]
	ADD(&autorelease, sd_cb_cm, ID_B, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_M, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_T, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_cb_cm, ID_MB, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_TB, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_TM, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_TMB, exp_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_cb_cm, ID_CB, exp_cmid);
	ADD(&autorelease, sd_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_cb_cm, ID_CM, exp_cbottom);
	struct map_session_data *sd_cb_cm_ct = ADD(&autorelease, sd_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_cb_cm, ID_CMB, exp_cmidbottom, exp_empty);
	TOGGLE(&autorelease, sd_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// b+cmb+x
	ADD(&autorelease, sd_b_cmb, ID_B, exp_cmidbottom); DEL(&autorelease, sd_b_cmb, ID_B, exp_cmidbottom);
	ADD(&autorelease, sd_b_cmb, ID_M, exp_cmidbottom);
	ADD(&autorelease, sd_b_cmb, ID_T, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_b_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_b_cmb, ID_TM, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_CB, exp_cbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_CM, exp_cmid_bottom, exp_bottom);
	ADD(&autorelease, sd_b_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_b_cmb, ID_CMB, exp_bottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_CTB, exp_ctopbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cmb, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+ctb+x
	ADD(&autorelease, sd_b_ctb, ID_B, exp_ctopbottom); DEL(&autorelease, sd_b_ctb, ID_B, exp_ctopbottom);
	ADD(&autorelease, sd_b_ctb, ID_M, exp_ctopbottom_mid);
	ADD(&autorelease, sd_b_ctb, ID_T, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_b_ctb, ID_TM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_CB, exp_cbottom, exp_bottom);
	ADD(&autorelease, sd_b_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_ctb, ID_CT, exp_ctop_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_CMB, exp_cmidbottom, exp_bottom);
	ADD(&autorelease, sd_b_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_b_ctb, ID_CTB, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctb, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+ctm+x
	ADD(&autorelease, sd_b_ctm, ID_B, exp_ctopmid_bottom); DEL(&autorelease, sd_b_ctm, ID_B, exp_ctopmid);
	ADD(&autorelease, sd_b_ctm, ID_M, exp_ctopmid_bottom);
	ADD(&autorelease, sd_b_ctm, ID_T, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_ctm, ID_TM, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_CM, exp_cmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_CT, exp_ctop_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_CMB, exp_cmidbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_CTB, exp_ctopbottom, exp_bottom);
	ADD(&autorelease, sd_b_ctm, ID_CTM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_ctm, ID_CTM, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctm, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+ctmb+x
	ADD(&autorelease, sd_b_ctmb, ID_B, exp_ctopmidbottom); DEL(&autorelease, sd_b_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_ctmb, ID_TM, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CB, exp_cbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CM, exp_cmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CT, exp_ctop_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CMB, exp_cmidbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CTB, exp_ctopbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ctmb, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	ADD(&autorelease, sd_b_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_b_ctmb, ID_CTMB, exp_bottom);
	// m+cmb+x
	ADD(&autorelease, sd_m_cmb, ID_B, exp_cmidbottom);
	ADD(&autorelease, sd_m_cmb, ID_M, exp_cmidbottom); DEL(&autorelease, sd_m_cmb, ID_M, exp_cmidbottom);
	ADD(&autorelease, sd_m_cmb, ID_T, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_m_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_m_cmb, ID_TB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_cmb, ID_CB, exp_cbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_cmb, ID_CM, exp_cmid, exp_mid);
	ADD(&autorelease, sd_m_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_m_cmb, ID_CMB, exp_mid);
	TOGGLE(&autorelease, sd_m_cmb, ID_CTB, exp_ctopbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_cmb, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_cmb, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+ctb+x
	ADD(&autorelease, sd_m_ctb, ID_B, exp_ctopbottom_mid);
	ADD(&autorelease, sd_m_ctb, ID_M, exp_ctopbottom_mid); DEL(&autorelease, sd_m_ctb, ID_M, exp_ctopbottom);
	ADD(&autorelease, sd_m_ctb, ID_T, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_m_ctb, ID_TB, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_ctb, ID_CB, exp_cbottom_mid, exp_mid);
	ADD(&autorelease, sd_m_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_ctb, ID_CT, exp_ctop_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctb, ID_CMB, exp_cmidbottom, exp_mid);
	ADD(&autorelease, sd_m_ctb, ID_CTB, exp_ctopbottom_mid); DEL(&autorelease, sd_m_ctb, ID_CTB, exp_mid);
	TOGGLE(&autorelease, sd_m_ctb, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctb, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+ctm+x
	ADD(&autorelease, sd_m_ctm, ID_B, exp_ctopmid_bottom);
	ADD(&autorelease, sd_m_ctm, ID_M, exp_ctopmid); DEL(&autorelease, sd_m_ctm, ID_M, exp_ctopmid);
	ADD(&autorelease, sd_m_ctm, ID_T, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_m_ctm, ID_TB, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_m_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_ctm, ID_CM, exp_cmid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctm, ID_CT, exp_ctop_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctm, ID_CMB, exp_cmidbottom, exp_mid);
	TOGGLE(&autorelease, sd_m_ctm, ID_CTB, exp_ctopbottom_mid, exp_mid);
	ADD(&autorelease, sd_m_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_m_ctm, ID_CTM, exp_mid);
	TOGGLE(&autorelease, sd_m_ctm, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+ctmb+x
	ADD(&autorelease, sd_m_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_ctmb, ID_M, exp_ctopmidbottom); DEL(&autorelease, sd_m_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_ctmb, ID_TB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CB, exp_cbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CM, exp_cmid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CT, exp_ctop_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CMB, exp_cmidbottom, exp_mid);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CTB, exp_ctopbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ctmb, ID_CTM, exp_ctopmid, exp_mid);
	ADD(&autorelease, sd_m_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_m_ctmb, ID_CTMB, exp_mid);
	// t+cmb+x
	ADD(&autorelease, sd_t_cmb, ID_B, exp_cmidbottom_top);
	ADD(&autorelease, sd_t_cmb, ID_M, exp_cmidbottom_top);
	ADD(&autorelease, sd_t_cmb, ID_T, exp_cmidbottom_top); DEL(&autorelease, sd_t_cmb, ID_T, exp_cmidbottom);
	ADD(&autorelease, sd_t_cmb, ID_MB, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_t_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_cmb, ID_CB, exp_cbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_cmb, ID_CM, exp_cmid_top, exp_top);
	ADD(&autorelease, sd_t_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_cmb, ID_CMB, exp_cmidbottom_top); DEL(&autorelease, sd_t_cmb, ID_CMB, exp_top);
	TOGGLE(&autorelease, sd_t_cmb, ID_CTB, exp_ctopbottom, exp_top);
	TOGGLE(&autorelease, sd_t_cmb, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_cmb, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+ctb+x
	ADD(&autorelease, sd_t_ctb, ID_B, exp_ctopbottom);
	ADD(&autorelease, sd_t_ctb, ID_M, exp_ctopbottom_mid);
	ADD(&autorelease, sd_t_ctb, ID_T, exp_ctopbottom); DEL(&autorelease, sd_t_ctb, ID_T, exp_ctopbottom);
	ADD(&autorelease, sd_t_ctb, ID_MB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_ctb, ID_CB, exp_cbottom_top, exp_top);
	ADD(&autorelease, sd_t_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_ctb, ID_CT, exp_ctop, exp_top);
	TOGGLE(&autorelease, sd_t_ctb, ID_CMB, exp_cmidbottom_top, exp_top);
	ADD(&autorelease, sd_t_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_t_ctb, ID_CTB, exp_top);
	TOGGLE(&autorelease, sd_t_ctb, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_ctb, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+ctm+x
	ADD(&autorelease, sd_t_ctm, ID_B, exp_ctopmid_bottom);
	ADD(&autorelease, sd_t_ctm, ID_M, exp_ctopmid);
	ADD(&autorelease, sd_t_ctm, ID_T, exp_ctopmid); DEL(&autorelease, sd_t_ctm, ID_T, exp_ctopmid);
	ADD(&autorelease, sd_t_ctm, ID_MB, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_t_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_ctm, ID_CM, exp_cmid_top, exp_top);
	TOGGLE(&autorelease, sd_t_ctm, ID_CT, exp_ctop, exp_top);
	TOGGLE(&autorelease, sd_t_ctm, ID_CMB, exp_cmidbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_ctm, ID_CTB, exp_ctopbottom, exp_top);
	ADD(&autorelease, sd_t_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_t_ctm, ID_CTM, exp_top);
	TOGGLE(&autorelease, sd_t_ctm, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+ctmb+x
	ADD(&autorelease, sd_t_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_t_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_t_ctmb, ID_T, exp_ctopmidbottom); DEL(&autorelease, sd_t_ctmb, ID_T, exp_ctopmidbottom);
	ADD(&autorelease, sd_t_ctmb, ID_MB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CB, exp_cbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CM, exp_cmid_top, exp_top);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CT, exp_ctop, exp_top);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CMB, exp_cmidbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CTB, exp_ctopbottom, exp_top);
	TOGGLE(&autorelease, sd_t_ctmb, ID_CTM, exp_ctopmid, exp_top);
	ADD(&autorelease, sd_t_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_t_ctmb, ID_CTMB, exp_top);
	// mb+cmb+x
	TOGGLE(&autorelease, sd_mb_cmb, ID_B, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_M, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_mb_cmb, ID_T, exp_cmidbottom_top);
	ADD(&autorelease, sd_mb_cmb, ID_MB, exp_cmidbottom); DEL(&autorelease, sd_mb_cmb, ID_MB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_CB, exp_cbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_CM, exp_cmid, exp_midbottom);
	ADD(&autorelease, sd_mb_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_mb_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_mb_cmb, ID_CMB, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_CTB, exp_ctopbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cmb, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+ctb+x
	TOGGLE(&autorelease, sd_mb_ctb, ID_B, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_M, exp_ctopbottom_mid, exp_ctopbottom);
	ADD(&autorelease, sd_mb_ctb, ID_T, exp_ctopbottom);
	ADD(&autorelease, sd_mb_ctb, ID_MB, exp_ctopbottom); DEL(&autorelease, sd_mb_ctb, ID_MB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_CB, exp_cbottom, exp_midbottom);
	ADD(&autorelease, sd_mb_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_ctb, ID_CT, exp_ctop_midbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_CMB, exp_cmidbottom, exp_midbottom);
	ADD(&autorelease, sd_mb_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_mb_ctb, ID_CTB, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctb, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+ctm+x
	TOGGLE(&autorelease, sd_mb_ctm, ID_B, exp_ctopmid_bottom, exp_ctopmid);
	TOGGLE(&autorelease, sd_mb_ctm, ID_M, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_mb_ctm, ID_T, exp_ctopmid);
	ADD(&autorelease, sd_mb_ctm, ID_MB, exp_ctopmid); DEL(&autorelease, sd_mb_ctm, ID_MB, exp_ctopmid);
	TOGGLE(&autorelease, sd_mb_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_mb_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_mb_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_mb_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_ctm, ID_CM, exp_cmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctm, ID_CT, exp_ctop_midbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctm, ID_CMB, exp_cmidbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctm, ID_CTB, exp_ctopbottom, exp_midbottom);
	ADD(&autorelease, sd_mb_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_mb_ctm, ID_CTM, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctm, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+ctmb+x
	TOGGLE(&autorelease, sd_mb_ctmb, ID_B, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_M, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_mb_ctmb, ID_T, exp_ctopmidbottom);
	ADD(&autorelease, sd_mb_ctmb, ID_MB, exp_ctopmidbottom); DEL(&autorelease, sd_mb_ctmb, ID_MB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CB, exp_cbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CM, exp_cmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CT, exp_ctop_midbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CMB, exp_cmidbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CTB, exp_ctopbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ctmb, ID_CTM, exp_ctopmid, exp_midbottom);
	ADD(&autorelease, sd_mb_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_mb_ctmb, ID_CTMB, exp_midbottom);
	// tb+cmb+x
	TOGGLE(&autorelease, sd_tb_cmb, ID_B, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_tb_cmb, ID_M, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_T, exp_cmidbottom_top, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_tb_cmb, ID_TB, exp_cmidbottom); DEL(&autorelease, sd_tb_cmb, ID_TB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_CB, exp_cbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_CM, exp_cmid_topbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tb_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_tb_cmb, ID_CMB, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_CTB, exp_ctopbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cmb, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+ctb+x
	TOGGLE(&autorelease, sd_tb_ctb, ID_B, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_tb_ctb, ID_M, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_tb_ctb, ID_T, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_tb_ctb, ID_TB, exp_ctopbottom); DEL(&autorelease, sd_tb_ctb, ID_TB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_CB, exp_cbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_ctb, ID_CT, exp_ctop, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_CMB, exp_cmidbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_tb_ctb, ID_CTB, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctb, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+ctm+x
	TOGGLE(&autorelease, sd_tb_ctm, ID_B, exp_ctopmid_bottom, exp_ctopmid);
	ADD(&autorelease, sd_tb_ctm, ID_M, exp_ctopmid);
	TOGGLE(&autorelease, sd_tb_ctm, ID_T, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tb_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_tb_ctm, ID_TB, exp_ctopmid); DEL(&autorelease, sd_tb_ctm, ID_TB, exp_ctopmid);
	TOGGLE(&autorelease, sd_tb_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tb_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_tb_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_ctm, ID_CM, exp_cmid_topbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctm, ID_CT, exp_ctop, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctm, ID_CMB, exp_cmidbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctm, ID_CTB, exp_ctopbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_tb_ctm, ID_CTM, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+ctmb
	TOGGLE(&autorelease, sd_tb_ctmb, ID_B, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_tb_ctmb, ID_M, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_T, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_tb_ctmb, ID_TB, exp_ctopmidbottom); DEL(&autorelease, sd_tb_ctmb, ID_TB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CB, exp_cbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CM, exp_cmid_topbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CT, exp_ctop, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CMB, exp_cmidbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CTB, exp_ctopbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ctmb, ID_CTM, exp_ctopmid, exp_topbottom);
	ADD(&autorelease, sd_tb_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_tb_ctmb, ID_CTMB, exp_topbottom);
	// tm+cmb+x
	ADD(&autorelease, sd_tm_cmb, ID_B, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_M, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_T, exp_cmidbottom_top, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_tm_cmb, ID_TM, exp_cmidbottom); DEL(&autorelease, sd_tm_cmb, ID_TM, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tm_cmb, ID_CB, exp_cbottom_topmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cmb, ID_CM, exp_cmid, exp_topmid);
	ADD(&autorelease, sd_tm_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tm_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_tm_cmb, ID_CMB, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cmb, ID_CTB, exp_ctopbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cmb, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+ctb+x
	ADD(&autorelease, sd_tm_ctb, ID_B, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_M, exp_ctopbottom_mid, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_T, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_tm_ctb, ID_TM, exp_ctopbottom); DEL(&autorelease, sd_tm_ctb, ID_TM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_ctb, ID_CB, exp_cbottom_topmid, exp_topmid);
	ADD(&autorelease, sd_tm_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_ctb, ID_CT, exp_ctop, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctb, ID_CMB, exp_cmidbottom, exp_topmid);
	ADD(&autorelease, sd_tm_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_tm_ctb, ID_CTB, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctb, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+ctm+x
	ADD(&autorelease, sd_tm_ctm, ID_B, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_tm_ctm, ID_M, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_T, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_tm_ctm, ID_TM, exp_ctopmid); DEL(&autorelease, sd_tm_ctm, ID_TM, exp_ctopmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_tm_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_ctm, ID_CM, exp_cmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_CT, exp_ctop, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_CMB, exp_cmidbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_CTB, exp_ctopbottom, exp_topmid);
	ADD(&autorelease, sd_tm_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_tm_ctm, ID_CTM, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+ctmb+x
	ADD(&autorelease, sd_tm_ctmb, ID_B, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_M, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_T, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_tm_ctmb, ID_TM, exp_ctopmidbottom); DEL(&autorelease, sd_tm_ctmb, ID_TM, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CB, exp_cbottom_topmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CM, exp_cmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CT, exp_ctop, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CMB, exp_cmidbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CTB, exp_ctopbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ctmb, ID_CTM, exp_ctopmid, exp_topmid);
	ADD(&autorelease, sd_tm_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_tm_ctmb, ID_CTMB, exp_topmid);
	// tmb+cmb+x
	TOGGLE(&autorelease, sd_tmb_cmb, ID_B, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_M, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_T, exp_cmidbottom_top, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_tmb_cmb, ID_TMB, exp_cmidbottom); DEL(&autorelease, sd_tmb_cmb, ID_TMB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_CB, exp_cbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_CM, exp_cmid, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tmb_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_tmb_cmb, ID_CMB, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+ctb+x
	TOGGLE(&autorelease, sd_tmb_ctb, ID_B, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_M, exp_ctopbottom_mid, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_T, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_tmb_ctb, ID_TMB, exp_ctopbottom); DEL(&autorelease, sd_tmb_ctb, ID_TMB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_CB, exp_cbottom, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_CT, exp_ctop, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_tmb_ctb, ID_CTB, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+ctm+x
	TOGGLE(&autorelease, sd_tmb_ctm, ID_B, exp_ctopmid_bottom, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_M, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_T, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_tmb_ctm, ID_TMB, exp_ctopmid); DEL(&autorelease, sd_tmb_ctm, ID_TMB, exp_ctopmid);
	ADD(&autorelease, sd_tmb_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_CM, exp_cmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_CT, exp_ctop, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_tmb_ctm, ID_CTM, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+ctmb+x
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_B, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_M, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_T, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_tmb_ctmb, ID_TMB, exp_ctopmidbottom); DEL(&autorelease, sd_tmb_ctmb, ID_TMB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CB, exp_cbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CM, exp_cmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CT, exp_ctop, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ctmb, ID_CTM, exp_ctopmid, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_tmb_ctmb, ID_CTMB, exp_topmidbottom);
	// cb+ct
	ADD(&autorelease, sd_cb_ct, ID_B, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_M, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_cb_ct, ID_T, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_MB, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_TB, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_TM, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_TMB, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_cb_ct, ID_CB, exp_ctop);
	ADD(&autorelease, sd_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_cb_ct, ID_CT, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_cb_ct, ID_CTB, exp_ctopbottom, exp_empty);
	TOGGLE(&autorelease, sd_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// cb+ctm
	ADD(&autorelease, sd_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_T, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_MB, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_TB, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_TM, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_TMB, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_ctm, ID_CMB, exp_cmidbottom, exp_empty);
	TOGGLE(&autorelease, sd_cb_ctm, ID_CTB, exp_ctopbottom, exp_empty);
	ADD(&autorelease, sd_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_cb_ctm, ID_CTM, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// cm+ct+x
	ADD(&autorelease, sd_cm_ct, ID_B, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_cm_ct, ID_M, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_T, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_MB, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_TB, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_TM, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_TMB, exp_ctop_cmid);
	ADD(&autorelease, sd_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_cm_ct, ID_CM, exp_ctop);
	ADD(&autorelease, sd_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_cm_ct, ID_CT, exp_cmid);
	TOGGLE(&autorelease, sd_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_cm_ct, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// cm+ctb+x
	ADD(&autorelease, sd_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_T, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_MB, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_TB, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_TM, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_TMB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid);
	ADD(&autorelease, sd_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_cm_ctb, ID_CMB, exp_cmidbottom, exp_empty);
	ADD(&autorelease, sd_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_cm_ctb, ID_CTB, exp_cmid);
	TOGGLE(&autorelease, sd_cm_ctb, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_empty);
	// ct+cmb+x
	ADD(&autorelease, sd_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_T, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_MB, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_TB, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_TM, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_TMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop);
	TOGGLE(&autorelease, sd_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop);
	ADD(&autorelease, sd_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_ct_cmb, ID_CMB, exp_ctop);
	TOGGLE(&autorelease, sd_ct_cmb, ID_CTB, exp_ctopbottom, exp_empty);
	TOGGLE(&autorelease, sd_ct_cmb, ID_CTM, exp_ctopmid, exp_empty);
	TOGGLE(&autorelease, sd_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_empty);

	// Four: (46)
	// b+m+t+x: b+m+t+cb b+m+t+cm b+m+t+ct b+m+t+cmb b+m+t+ctm b+m+t+ctb b+m+t+ctmb [7]
	ADD(&autorelease, sd_b_m_t, ID_B, exp_top_mid_bottom); DEL(&autorelease, sd_b_m_t, ID_B, exp_top_mid);
	ADD(&autorelease, sd_b_m_t, ID_M, exp_top_mid_bottom); DEL(&autorelease, sd_b_m_t, ID_M, exp_top_bottom);
	ADD(&autorelease, sd_b_m_t, ID_T, exp_top_mid_bottom); DEL(&autorelease, sd_b_m_t, ID_T, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t, ID_MB, exp_top_midbottom, exp_top);
	TOGGLE(&autorelease, sd_b_m_t, ID_TB, exp_topbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_b_m_t, ID_TM, exp_topmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_m_t, ID_TMB, exp_topmidbottom, exp_empty);
	struct map_session_data *sd_b_m_t_cb = ADD(&autorelease, sd_b_m_t, ID_CB, exp_cbottom_top_mid);
	struct map_session_data *sd_b_m_t_cm = ADD(&autorelease, sd_b_m_t, ID_CM, exp_cmid_top_bottom);
	struct map_session_data *sd_b_m_t_ct = ADD(&autorelease, sd_b_m_t, ID_CT, exp_ctop_mid_bottom);
	struct map_session_data *sd_b_m_t_cmb = ADD(&autorelease, sd_b_m_t, ID_CMB, exp_cmidbottom_top);
	struct map_session_data *sd_b_m_t_ctb = ADD(&autorelease, sd_b_m_t, ID_CTB, exp_ctopbottom_mid);
	struct map_session_data *sd_b_m_t_ctm = ADD(&autorelease, sd_b_m_t, ID_CTM, exp_ctopmid_bottom);
	struct map_session_data *sd_b_m_t_ctmb = ADD(&autorelease, sd_b_m_t, ID_CTMB, exp_ctopmidbottom);
	// b+m+cb+x: b+m+cb+cm b+m+cb+ct b+m+cb+ctm [3]
	ADD(&autorelease, sd_b_m_cb, ID_B, exp_cbottom_mid); DEL(&autorelease, sd_b_m_cb, ID_B, exp_cbottom_mid);
	ADD(&autorelease, sd_b_m_cb, ID_M, exp_cbottom_mid); DEL(&autorelease, sd_b_m_cb, ID_M, exp_cbottom);
	ADD(&autorelease, sd_b_m_cb, ID_T, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_cb, ID_MB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb, ID_TB, exp_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_m_cb, ID_CB, exp_cbottom_mid); DEL(&autorelease, sd_b_m_cb, ID_CB, exp_mid_bottom);
	struct map_session_data *sd_b_m_cb_cm = ADD(&autorelease, sd_b_m_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_b_m_cb_ct = ADD(&autorelease, sd_b_m_cb, ID_CT, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_cb, ID_CMB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_cb, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	struct map_session_data *sd_b_m_cb_ctm = ADD(&autorelease, sd_b_m_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cm+x: b+m+cm+ct b+m+cm+ctb [2]
	ADD(&autorelease, sd_b_m_cm, ID_B, exp_cmid_bottom); DEL(&autorelease, sd_b_m_cm, ID_B, exp_cmid);
	ADD(&autorelease, sd_b_m_cm, ID_M, exp_cmid_bottom); DEL(&autorelease, sd_b_m_cm, ID_M, exp_cmid_bottom);
	ADD(&autorelease, sd_b_m_cm, ID_T, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_cm, ID_MB, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_b_m_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_b_m_cm, ID_TM, exp_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_b_m_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cm, ID_CM, exp_cmid_bottom); DEL(&autorelease, sd_b_m_cm, ID_CM, exp_mid_bottom);
	struct map_session_data *sd_b_m_cm_ct = ADD(&autorelease, sd_b_m_cm, ID_CT, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	struct map_session_data *sd_b_m_cm_ctb = ADD(&autorelease, sd_b_m_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+ct+x: b+m+ct+cmb [1]
	ADD(&autorelease, sd_b_m_ct, ID_B, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_ct, ID_B, exp_ctop_mid);
	ADD(&autorelease, sd_b_m_ct, ID_M, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_ct, ID_M, exp_ctop_bottom);
	ADD(&autorelease, sd_b_m_ct, ID_T, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_b_m_ct, ID_TB, exp_ctop_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_b_m_ct, ID_TM, exp_ctop_bottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_m_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_m_ct, ID_CB, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_ct, ID_CM, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_ct, ID_CT, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_ct, ID_CT, exp_mid_bottom);
	struct map_session_data *sd_b_m_ct_cmb = ADD(&autorelease, sd_b_m_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cmb+x
	ADD(&autorelease, sd_b_m_cmb, ID_B, exp_cmidbottom); DEL(&autorelease, sd_b_m_cmb, ID_B, exp_cmidbottom);
	ADD(&autorelease, sd_b_m_cmb, ID_M, exp_cmidbottom); DEL(&autorelease, sd_b_m_cmb, ID_M, exp_cmidbottom);
	ADD(&autorelease, sd_b_m_cmb, ID_T, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_CB, exp_cbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_CM, exp_cmid_bottom, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_b_m_cmb, ID_CMB, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cmb, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+ctb+x
	ADD(&autorelease, sd_b_m_ctb, ID_B, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_ctb, ID_B, exp_ctopbottom_mid);
	ADD(&autorelease, sd_b_m_ctb, ID_M, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_ctb, ID_M, exp_ctopbottom);
	ADD(&autorelease, sd_b_m_ctb, ID_T, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_TB, exp_ctopbottom_mid, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_CB, exp_cbottom_mid, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_CT, exp_ctop_mid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_ctb, ID_CTB, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_ctb, ID_CTB, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctb, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+ctm+x
	ADD(&autorelease, sd_b_m_ctm, ID_B, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_ctm, ID_B, exp_ctopmid);
	ADD(&autorelease, sd_b_m_ctm, ID_M, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_ctm, ID_M, exp_ctopmid_bottom);
	ADD(&autorelease, sd_b_m_ctm, ID_T, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_TM, exp_ctopmid_bottom, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_m_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_CM, exp_cmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_CT, exp_ctop_mid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_ctm, ID_CTM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_ctm, ID_CTM, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctm, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+ctmb+x
	ADD(&autorelease, sd_b_m_ctmb, ID_B, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_m_ctmb, ID_M, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_m_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CB, exp_cbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CM, exp_cmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CT, exp_ctop_mid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ctmb, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_ctmb, ID_CTMB, exp_mid_bottom);
	// b+t+cb+x: b+t+cb+cm b+t+cb+ct b+t+cb+ctm [3]
	ADD(&autorelease, sd_b_t_cb, ID_B, exp_cbottom_top); DEL(&autorelease, sd_b_t_cb, ID_B, exp_cbottom_top);
	ADD(&autorelease, sd_b_t_cb, ID_M, exp_cbottom_top_mid);
	ADD(&autorelease, sd_b_t_cb, ID_T, exp_cbottom_top); DEL(&autorelease, sd_b_t_cb, ID_T, exp_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_MB, exp_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb, ID_TB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_t_cb, ID_CB, exp_cbottom_top); DEL(&autorelease, sd_b_t_cb, ID_CB, exp_top_bottom);
	struct map_session_data *sd_b_t_cb_cm = ADD(&autorelease, sd_b_t_cb, ID_CM, exp_cmid_cbottom_top);
	struct map_session_data *sd_b_t_cb_ct = ADD(&autorelease, sd_b_t_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_CTB, exp_ctopbottom, exp_top_bottom);
	struct map_session_data *sd_b_t_cb_ctm = ADD(&autorelease, sd_b_t_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cm+x: b+t+cm+ct b+t+cm+ctb [2]
	ADD(&autorelease, sd_b_t_cm, ID_B, exp_cmid_top_bottom); DEL(&autorelease, sd_b_t_cm, ID_B, exp_cmid_top);
	ADD(&autorelease, sd_b_t_cm, ID_M, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_t_cm, ID_T, exp_cmid_top_bottom); DEL(&autorelease, sd_b_t_cm, ID_T, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cm, ID_MB, exp_cmid_top, exp_cmid_top);
	TOGGLE(&autorelease, sd_b_t_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_b_t_cm, ID_TM, exp_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_b_t_cm, ID_CB, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_t_cm, ID_CM, exp_cmid_top_bottom); DEL(&autorelease, sd_b_t_cm, ID_CM, exp_top_bottom);
	struct map_session_data *sd_b_t_cm_ct = ADD(&autorelease, sd_b_t_cm, ID_CT, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cm, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	struct map_session_data *sd_b_t_cm_ctb = ADD(&autorelease, sd_b_t_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+ct+x: b+t+ct+cmb [1]
	ADD(&autorelease, sd_b_t_ct, ID_B, exp_ctop_bottom); DEL(&autorelease, sd_b_t_ct, ID_B, exp_ctop);
	ADD(&autorelease, sd_b_t_ct, ID_M, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_t_ct, ID_T, exp_ctop_bottom); DEL(&autorelease, sd_b_t_ct, ID_T, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_b_t_ct, ID_TB, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_b_t_ct, ID_TM, exp_ctop_bottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_t_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_t_ct, ID_CM, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_t_ct, ID_CT, exp_ctop_bottom); DEL(&autorelease, sd_b_t_ct, ID_CT, exp_top_bottom);
	struct map_session_data *sd_b_t_ct_cmb = ADD(&autorelease, sd_b_t_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct, ID_CTB, exp_ctopbottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ct, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ct, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cmb+x
	ADD(&autorelease, sd_b_t_cmb, ID_B, exp_cmidbottom_top); DEL(&autorelease, sd_b_t_cmb, ID_B, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_t_cmb, ID_M, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_t_cmb, ID_T, exp_cmidbottom_top); DEL(&autorelease, sd_b_t_cmb, ID_T, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_MB, exp_cmidbottom_top, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_CB, exp_cbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_CM, exp_cmid_top_bottom, exp_top_bottom);
	ADD(&autorelease, sd_b_t_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_t_cmb, ID_CMB, exp_cmidbottom_top); DEL(&autorelease, sd_b_t_cmb, ID_CMB, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_CTB, exp_ctopbottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+ctb+x
	ADD(&autorelease, sd_b_t_ctb, ID_B, exp_ctopbottom); DEL(&autorelease, sd_b_t_ctb, ID_B, exp_ctopbottom);
	ADD(&autorelease, sd_b_t_ctb, ID_M, exp_ctopbottom_mid);
	ADD(&autorelease, sd_b_t_ctb, ID_T, exp_ctopbottom); DEL(&autorelease, sd_b_t_ctb, ID_T, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_CB, exp_cbottom_top, exp_top_bottom);
	ADD(&autorelease, sd_b_t_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_CT, exp_ctop_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	ADD(&autorelease, sd_b_t_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_b_t_ctb, ID_CTB, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+ctm+x
	ADD(&autorelease, sd_b_t_ctm, ID_B, exp_ctopmid_bottom); DEL(&autorelease, sd_b_t_ctm, ID_B, exp_ctopmid);
	ADD(&autorelease, sd_b_t_ctm, ID_M, exp_ctopmid_bottom);
	ADD(&autorelease, sd_b_t_ctm, ID_T, exp_ctopmid_bottom); DEL(&autorelease, sd_b_t_ctm, ID_T, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_TM, exp_ctopmid_bottom, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_t_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_CM, exp_cmid_top_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_CT, exp_ctop_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_CTB, exp_ctopbottom, exp_top_bottom);
	ADD(&autorelease, sd_b_t_ctm, ID_CTM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_t_ctm, ID_CTM, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+ctmb+x
	ADD(&autorelease, sd_b_t_ctmb, ID_B, exp_ctopmidbottom); DEL(&autorelease, sd_b_t_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_t_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_t_ctmb, ID_T, exp_ctopmidbottom); DEL(&autorelease, sd_b_t_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CB, exp_cbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CM, exp_cmid_top_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CT, exp_ctop_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CTB, exp_ctopbottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ctmb, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	ADD(&autorelease, sd_b_t_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_b_t_ctmb, ID_CTMB, exp_top_bottom);
	// b+tm+cb+x: b+tm+cb+cm b+tm+cb+ct b+tm+cb+ctm [3]
	ADD(&autorelease, sd_b_tm_cb, ID_B, exp_cbottom_topmid); DEL(&autorelease, sd_b_tm_cb, ID_B, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_M, exp_cbottom_mid, exp_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_T, exp_cbottom_top, exp_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_MB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_TB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_tm_cb, ID_TM, exp_cbottom_topmid); DEL(&autorelease, sd_b_tm_cb, ID_TM, exp_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_tm_cb, ID_CB, exp_cbottom_topmid); DEL(&autorelease, sd_b_tm_cb, ID_CB, exp_topmid_bottom);
	struct map_session_data *sd_b_tm_cb_cm = ADD(&autorelease, sd_b_tm_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_b_tm_cb_ct = ADD(&autorelease, sd_b_tm_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	struct map_session_data *sd_b_tm_cb_ctm = ADD(&autorelease, sd_b_tm_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cm+x: b+tm+cm+ct b+tm+cm+ctb [2]
	ADD(&autorelease, sd_b_tm_cm, ID_B, exp_cmid_bottom); DEL(&autorelease, sd_b_tm_cm, ID_B, exp_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_M, exp_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_T, exp_cmid_top_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_MB, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	ADD(&autorelease, sd_b_tm_cm, ID_TM, exp_cmid_bottom); DEL(&autorelease, sd_b_tm_cm, ID_TM, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_b_tm_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cm, ID_CM, exp_cmid_bottom); DEL(&autorelease, sd_b_tm_cm, ID_CM, exp_topmid_bottom);
	struct map_session_data *sd_b_tm_cm_ct = ADD(&autorelease, sd_b_tm_cm, ID_CT, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	struct map_session_data *sd_b_tm_cm_ctb = ADD(&autorelease, sd_b_tm_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+ct+x: b+tm+ct+cmb [1]
	ADD(&autorelease, sd_b_tm_ct, ID_B, exp_ctop_bottom); DEL(&autorelease, sd_b_tm_ct, ID_B, exp_ctop);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_M, exp_ctop_mid_bottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_T, exp_ctop_bottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_TB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_tm_ct, ID_TM, exp_ctop_bottom); DEL(&autorelease, sd_b_tm_ct, ID_TM, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_tm_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_tm_ct, ID_CM, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_tm_ct, ID_CT, exp_ctop_bottom); DEL(&autorelease, sd_b_tm_ct, ID_CT, exp_topmid_bottom);
	struct map_session_data *sd_b_tm_ct_cmb = ADD(&autorelease, sd_b_tm_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cmb+x
	ADD(&autorelease, sd_b_tm_cmb, ID_B, exp_cmidbottom); DEL(&autorelease, sd_b_tm_cmb, ID_B, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_M, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_T, exp_cmidbottom_top, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_b_tm_cmb, ID_TM, exp_cmidbottom); DEL(&autorelease, sd_b_tm_cmb, ID_TM, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_CB, exp_cbottom_topmid, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_CM, exp_cmid_bottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_tm_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_b_tm_cmb, ID_CMB, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+ctb+x
	ADD(&autorelease, sd_b_tm_ctb, ID_B, exp_ctopbottom); DEL(&autorelease, sd_b_tm_ctb, ID_B, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_M, exp_ctopbottom_mid, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_T, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_b_tm_ctb, ID_TM, exp_ctopbottom); DEL(&autorelease, sd_b_tm_ctb, ID_TM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_CB, exp_cbottom_topmid, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_CT, exp_ctop_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_b_tm_ctb, ID_CTB, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+ctm+x
	ADD(&autorelease, sd_b_tm_ctm, ID_B, exp_ctopmid_bottom); DEL(&autorelease, sd_b_tm_ctm, ID_B, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_M, exp_ctopmid_bottom, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_T, exp_ctopmid_bottom, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_tm_ctm, ID_TM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_tm_ctm, ID_TM, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_tm_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_CM, exp_cmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_CT, exp_ctop_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_ctm, ID_CTM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_tm_ctm, ID_CTM, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+ctmb+x
	ADD(&autorelease, sd_b_tm_ctmb, ID_B, exp_ctopmidbottom); DEL(&autorelease, sd_b_tm_ctmb, ID_B, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_M, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_T, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_tm_ctmb, ID_TM, exp_ctopmidbottom); DEL(&autorelease, sd_b_tm_ctmb, ID_TM, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CB, exp_cbottom_topmid, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CM, exp_cmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CT, exp_ctop_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ctmb, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_b_tm_ctmb, ID_CTMB, exp_topmid_bottom);
	// b+cb+cm+x: b+cb+cm+ct [1]
	ADD(&autorelease, sd_b_cb_cm, ID_B, exp_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm, ID_B, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm, ID_M, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm, ID_T, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm, ID_TM, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm, ID_CB, exp_cmid_bottom);
	ADD(&autorelease, sd_b_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm, ID_CM, exp_cbottom);
	struct map_session_data *sd_b_cb_cm_ct = ADD(&autorelease, sd_b_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_CMB, exp_cmidbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+cb+ct+x
	ADD(&autorelease, sd_b_cb_ct, ID_B, exp_ctop_cbottom); DEL(&autorelease, sd_b_cb_ct, ID_B, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_cb_ct, ID_M, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_cb_ct, ID_T, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_cb_ct, ID_TM, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_b_cb_ct, ID_CB, exp_ctop_bottom);
	ADD(&autorelease, sd_b_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_b_cb_ct, ID_CT, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_CTB, exp_ctopbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+cb+ctm+x
	ADD(&autorelease, sd_b_cb_ctm, ID_B, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_cb_ctm, ID_TM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_cb_ctm, ID_CB, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_CMB, exp_cmidbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_CTB, exp_ctopbottom, exp_bottom);
	ADD(&autorelease, sd_b_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_cb_ctm, ID_CTM, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+cm+ct+x
	ADD(&autorelease, sd_b_cm_ct, ID_B, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_cm_ct, ID_B, exp_ctop_cmid);
	ADD(&autorelease, sd_b_cm_ct, ID_M, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_cm_ct, ID_T, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_cm_ct, ID_TM, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cm_ct, ID_CM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_cm_ct, ID_CM, exp_ctop_bottom);
	ADD(&autorelease, sd_b_cm_ct, ID_CT, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_cm_ct, ID_CT, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+cm+ctb+x
	ADD(&autorelease, sd_b_cm_ctb, ID_B, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_cm_ctb, ID_TM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid_bottom);
	ADD(&autorelease, sd_b_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_CT, exp_ctop_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_CMB, exp_cmidbottom, exp_bottom);
	ADD(&autorelease, sd_b_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_cm_ctb, ID_CTB, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// b+ct+cmb+x
	ADD(&autorelease, sd_b_ct_cmb, ID_B, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_ct_cmb, ID_TM, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_CM, exp_ctop_cmid_bottom, exp_ctop_bottom);
	ADD(&autorelease, sd_b_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_b_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_ct_cmb, ID_CMB, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_CTB, exp_ctopbottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_CTM, exp_ctopmid_bottom, exp_bottom);
	TOGGLE(&autorelease, sd_b_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// m+t+cb+x: m+t+cb+cm m+t+cb+ct m+t+cb+ctm [3]
	ADD(&autorelease, sd_m_t_cb, ID_B, exp_cbottom_top_mid);
	ADD(&autorelease, sd_m_t_cb, ID_M, exp_cbottom_top_mid); DEL(&autorelease, sd_m_t_cb, ID_M, exp_cbottom_top);
	ADD(&autorelease, sd_m_t_cb, ID_T, exp_cbottom_top_mid); DEL(&autorelease, sd_m_t_cb, ID_T, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cb, ID_MB, exp_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_m_t_cb, ID_TB, exp_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_m_t_cb, ID_CB, exp_cbottom_top_mid); DEL(&autorelease, sd_m_t_cb, ID_CB, exp_top_mid);
	struct map_session_data *sd_m_t_cb_cm = ADD(&autorelease, sd_m_t_cb, ID_CM, exp_cmid_cbottom_top);
	struct map_session_data *sd_m_t_cb_ct = ADD(&autorelease, sd_m_t_cb, ID_CT, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cb, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	struct map_session_data *sd_m_t_cb_ctm = ADD(&autorelease, sd_m_t_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cm+x: m+t+cm+ct m+t+cm+ctb [2]
	ADD(&autorelease, sd_m_t_cm, ID_B, exp_cmid_top_bottom);
	ADD(&autorelease, sd_m_t_cm, ID_M, exp_cmid_top); DEL(&autorelease, sd_m_t_cm, ID_M, exp_cmid_top);
	ADD(&autorelease, sd_m_t_cm, ID_T, exp_cmid_top); DEL(&autorelease, sd_m_t_cm, ID_T, exp_cmid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_MB, exp_cmid_top, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_m_t_cm, ID_CB, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_m_t_cm, ID_CM, exp_cmid_top); DEL(&autorelease, sd_m_t_cm, ID_CM, exp_top_mid);
	struct map_session_data *sd_m_t_cm_ct = ADD(&autorelease, sd_m_t_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	struct map_session_data *sd_m_t_cm_ctb = ADD(&autorelease, sd_m_t_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cm, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+ct+x: m+t+ct+cmb [1]
	ADD(&autorelease, sd_m_t_ct, ID_B, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_m_t_ct, ID_M, exp_ctop_mid); DEL(&autorelease, sd_m_t_ct, ID_M, exp_ctop);
	ADD(&autorelease, sd_m_t_ct, ID_T, exp_ctop_mid); DEL(&autorelease, sd_m_t_ct, ID_T, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_m_t_ct, ID_TB, exp_ctop_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_m_t_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_m_t_ct, ID_CB, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_t_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_m_t_ct, ID_CT, exp_ctop_mid); DEL(&autorelease, sd_m_t_ct, ID_CT, exp_top_mid);
	struct map_session_data *sd_m_t_ct_cmb = ADD(&autorelease, sd_m_t_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ct, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cmb+x
	ADD(&autorelease, sd_m_t_cmb, ID_B, exp_cmidbottom_top);
	ADD(&autorelease, sd_m_t_cmb, ID_M, exp_cmidbottom_top); DEL(&autorelease, sd_m_t_cmb, ID_M, exp_cmidbottom_top);
	ADD(&autorelease, sd_m_t_cmb, ID_T, exp_cmidbottom_top); DEL(&autorelease, sd_m_t_cmb, ID_T, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_MB, exp_cmidbottom_top, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_CB, exp_cbottom_top_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_CM, exp_cmid_top, exp_top_mid);
	ADD(&autorelease, sd_m_t_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_t_cmb, ID_CMB, exp_cmidbottom_top); DEL(&autorelease, sd_m_t_cmb, ID_CMB, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+ctb+x
	ADD(&autorelease, sd_m_t_ctb, ID_B, exp_ctopbottom_mid);
	ADD(&autorelease, sd_m_t_ctb, ID_M, exp_ctopbottom_mid); DEL(&autorelease, sd_m_t_ctb, ID_M, exp_ctopbottom);
	ADD(&autorelease, sd_m_t_ctb, ID_T, exp_ctopbottom_mid); DEL(&autorelease, sd_m_t_ctb, ID_T, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_TB, exp_ctopbottom_mid, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_CB, exp_cbottom_top_mid, exp_top_mid);
	ADD(&autorelease, sd_m_t_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_CT, exp_ctop_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	ADD(&autorelease, sd_m_t_ctb, ID_CTB, exp_ctopbottom_mid); DEL(&autorelease, sd_m_t_ctb, ID_CTB, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+ctm+x
	ADD(&autorelease, sd_m_t_ctm, ID_B, exp_ctopmid_bottom);
	ADD(&autorelease, sd_m_t_ctm, ID_M, exp_ctopmid); DEL(&autorelease, sd_m_t_ctm, ID_M, exp_ctopmid);
	ADD(&autorelease, sd_m_t_ctm, ID_T, exp_ctopmid); DEL(&autorelease, sd_m_t_ctm, ID_T, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_m_t_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_CM, exp_cmid_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_CT, exp_ctop_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	ADD(&autorelease, sd_m_t_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_m_t_ctm, ID_CTM, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+ctmb+x
	ADD(&autorelease, sd_m_t_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_t_ctmb, ID_M, exp_ctopmidbottom); DEL(&autorelease, sd_m_t_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_t_ctmb, ID_T, exp_ctopmidbottom); DEL(&autorelease, sd_m_t_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CB, exp_cbottom_top_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CM, exp_cmid_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CT, exp_ctop_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ctmb, ID_CTM, exp_ctopmid, exp_top_mid);
	ADD(&autorelease, sd_m_t_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_m_t_ctmb, ID_CTMB, exp_top_mid);
	// m+tb+cb+x: m+tb+cb+cm m+tb+cb+ct m+tb+cb+ctm [3]
	TOGGLE(&autorelease, sd_m_tb_cb, ID_B, exp_cbottom_mid, exp_cbottom_mid);
	ADD(&autorelease, sd_m_tb_cb, ID_M, exp_cbottom_mid); DEL(&autorelease, sd_m_tb_cb, ID_M, exp_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_T, exp_cbottom_top_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_MB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_m_tb_cb, ID_TB, exp_cbottom_mid); DEL(&autorelease, sd_m_tb_cb, ID_TB, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_m_tb_cb, ID_CB, exp_cbottom_mid); DEL(&autorelease, sd_m_tb_cb, ID_CB, exp_topbottom_mid);
	struct map_session_data *sd_m_tb_cb_cm = ADD(&autorelease, sd_m_tb_cb, ID_CM, exp_cmid_cbottom);
	struct map_session_data *sd_m_tb_cb_ct = ADD(&autorelease, sd_m_tb_cb, ID_CT, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	struct map_session_data *sd_m_tb_cb_ctm = ADD(&autorelease, sd_m_tb_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cm+x: m+tb+cm+ct m+tb+cm+ctb [2]
	TOGGLE(&autorelease, sd_m_tb_cm, ID_B, exp_cmid_bottom, exp_cmid);
	ADD(&autorelease, sd_m_tb_cm, ID_M, exp_cmid_topbottom); DEL(&autorelease, sd_m_tb_cm, ID_M, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_T, exp_cmid_top, exp_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_MB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_m_tb_cm, ID_TB, exp_cmid_topbottom); DEL(&autorelease, sd_m_tb_cm, ID_TB, exp_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_m_tb_cm, ID_CB, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cm, ID_CM, exp_cmid_topbottom); DEL(&autorelease, sd_m_tb_cm, ID_CM, exp_topbottom_mid);
	struct map_session_data *sd_m_tb_cm_ct = ADD(&autorelease, sd_m_tb_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	struct map_session_data *sd_m_tb_cm_ctb = ADD(&autorelease, sd_m_tb_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cm, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+ct+x: m+tb+ct+cmb [1]
	TOGGLE(&autorelease, sd_m_tb_ct, ID_B, exp_ctop_mid_bottom, exp_ctop_mid);
	ADD(&autorelease, sd_m_tb_ct, ID_M, exp_ctop_mid); DEL(&autorelease, sd_m_tb_ct, ID_M, exp_ctop);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_T, exp_ctop_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	ADD(&autorelease, sd_m_tb_ct, ID_TB, exp_ctop_mid); DEL(&autorelease, sd_m_tb_ct, ID_TB, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_m_tb_ct, ID_CB, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_tb_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_m_tb_ct, ID_CT, exp_ctop_mid); DEL(&autorelease, sd_m_tb_ct, ID_CT, exp_topbottom_mid);
	struct map_session_data *sd_m_tb_ct_cmb = ADD(&autorelease, sd_m_tb_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cmb+x
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_B, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_m_tb_cmb, ID_M, exp_cmidbottom); DEL(&autorelease, sd_m_tb_cmb, ID_M, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_T, exp_cmidbottom_top, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_MB, exp_cmidbottom, exp_cmidbottom);
	ADD(&autorelease, sd_m_tb_cmb, ID_TB, exp_cmidbottom); DEL(&autorelease, sd_m_tb_cmb, ID_TB, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_CB, exp_cbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_CM, exp_cmid_topbottom, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_tb_cmb, ID_CMB, exp_cmidbottom); DEL(&autorelease, sd_m_tb_cmb, ID_CMB, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cmb, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+ctb+x
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_B, exp_ctopbottom_mid, exp_ctopbottom_mid);
	ADD(&autorelease, sd_m_tb_ctb, ID_M, exp_ctopbottom_mid); DEL(&autorelease, sd_m_tb_ctb, ID_M, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_T, exp_ctopbottom_mid, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	ADD(&autorelease, sd_m_tb_ctb, ID_TB, exp_ctopbottom_mid); DEL(&autorelease, sd_m_tb_ctb, ID_TB, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_CB, exp_cbottom_mid, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_CT, exp_ctop_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_ctb, ID_CTB, exp_ctopbottom_mid); DEL(&autorelease, sd_m_tb_ctb, ID_CTB, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctb, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+ctm+x
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_B, exp_ctopmid_bottom, exp_ctopmid);
	ADD(&autorelease, sd_m_tb_ctm, ID_M, exp_ctopmid); DEL(&autorelease, sd_m_tb_ctm, ID_M, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_T, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_m_tb_ctm, ID_TB, exp_ctopmid); DEL(&autorelease, sd_m_tb_ctm, ID_TB, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_m_tb_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_CM, exp_cmid_topbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_CT, exp_ctop_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_m_tb_ctm, ID_CTM, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+ctmb+x
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_B, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_tb_ctmb, ID_M, exp_ctopmidbottom); DEL(&autorelease, sd_m_tb_ctmb, ID_M, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_T, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_m_tb_ctmb, ID_TB, exp_ctopmidbottom); DEL(&autorelease, sd_m_tb_ctmb, ID_TB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CB, exp_cbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CM, exp_cmid_topbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CT, exp_ctop_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ctmb, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_m_tb_ctmb, ID_CTMB, exp_topbottom_mid);
	// m+cb+cm+x: m+cb+cm+ct [1]
	ADD(&autorelease, sd_m_cb_cm, ID_B, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm, ID_M, exp_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm, ID_M, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm, ID_T, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm, ID_TB, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm, ID_CB, exp_cmid);
	ADD(&autorelease, sd_m_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm, ID_CM, exp_cbottom_mid);
	struct map_session_data *sd_m_cb_cm_ct = ADD(&autorelease, sd_m_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_CMB, exp_cmidbottom, exp_mid);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+cb+ct+x
	ADD(&autorelease, sd_m_cb_ct, ID_B, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_cb_ct, ID_M, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_cb_ct, ID_M, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_cb_ct, ID_T, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_cb_ct, ID_TB, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_cb_ct, ID_CB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_cb_ct, ID_CB, exp_ctop_mid);
	ADD(&autorelease, sd_m_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_ct, ID_CT, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_cb_ct, ID_CT, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_CTB, exp_ctopbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+cb+ctm+x
	ADD(&autorelease, sd_m_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_cb_ctm, ID_M, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_cb_ctm, ID_TB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_CT, exp_ctop_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_CMB, exp_cmidbottom, exp_mid);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_CTB, exp_ctopbottom_mid, exp_mid);
	ADD(&autorelease, sd_m_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_cb_ctm, ID_CTM, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+cm+ct+x
	ADD(&autorelease, sd_m_cm_ct, ID_B, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_m_cm_ct, ID_M, exp_ctop_cmid); DEL(&autorelease, sd_m_cm_ct, ID_M, exp_ctop_cmid);
	ADD(&autorelease, sd_m_cm_ct, ID_T, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_m_cm_ct, ID_TB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_m_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_m_cm_ct, ID_CM, exp_ctop_mid);
	ADD(&autorelease, sd_m_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_m_cm_ct, ID_CT, exp_cmid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+cm+ctb+x
	ADD(&autorelease, sd_m_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_cm_ctb, ID_M, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_cm_ctb, ID_TB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid);
	ADD(&autorelease, sd_m_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_cm_ctb, ID_CM, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_CMB, exp_cmidbottom, exp_mid);
	ADD(&autorelease, sd_m_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_cm_ctb, ID_CTB, exp_cmid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// m+ct+cmb+x
	ADD(&autorelease, sd_m_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_ct_cmb, ID_M, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_ct_cmb, ID_TB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_CB, exp_ctop_cbottom_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop_mid);
	ADD(&autorelease, sd_m_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_m_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_ct_cmb, ID_CMB, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_CTB, exp_ctopbottom_mid, exp_mid);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_CTM, exp_ctopmid, exp_mid);
	TOGGLE(&autorelease, sd_m_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// t+mb+cb+x: t+mb+cb+cm t+mb+cb+ct t+mb+cb+ctm [3]
	TOGGLE(&autorelease, sd_t_mb_cb, ID_B, exp_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_M, exp_cbottom_top_mid, exp_cbottom_top);
	ADD(&autorelease, sd_t_mb_cb, ID_T, exp_cbottom_top); DEL(&autorelease, sd_t_mb_cb, ID_T, exp_cbottom);
	ADD(&autorelease, sd_t_mb_cb, ID_MB, exp_cbottom_top); DEL(&autorelease, sd_t_mb_cb, ID_MB, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_TB, exp_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_t_mb_cb, ID_CB, exp_cbottom_top); DEL(&autorelease, sd_t_mb_cb, ID_CB, exp_top_midbottom);
	struct map_session_data *sd_t_mb_cb_cm = ADD(&autorelease, sd_t_mb_cb, ID_CM, exp_cmid_cbottom_top);
	struct map_session_data *sd_t_mb_cb_ct = ADD(&autorelease, sd_t_mb_cb, ID_CT, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	struct map_session_data *sd_t_mb_cb_ctm = ADD(&autorelease, sd_t_mb_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cm+x: t+mb+cm+ct t+mb+cm+ctb [2]
	TOGGLE(&autorelease, sd_t_mb_cm, ID_B, exp_cmid_top_bottom, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_M, exp_cmid_top, exp_cmid_top);
	ADD(&autorelease, sd_t_mb_cm, ID_T, exp_cmid_top); DEL(&autorelease, sd_t_mb_cm, ID_T, exp_cmid);
	ADD(&autorelease, sd_t_mb_cm, ID_MB, exp_cmid_top); DEL(&autorelease, sd_t_mb_cm, ID_MB, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_TM, exp_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_t_mb_cm, ID_CB, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_t_mb_cm, ID_CM, exp_cmid_top); DEL(&autorelease, sd_t_mb_cm, ID_CM, exp_top_midbottom);
	struct map_session_data *sd_t_mb_cm_ct = ADD(&autorelease, sd_t_mb_cm, ID_CT, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	struct map_session_data *sd_t_mb_cm_ctb = ADD(&autorelease, sd_t_mb_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cm, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+ct+x: t+mb+ct+cmb [1]
	TOGGLE(&autorelease, sd_t_mb_ct, ID_B, exp_ctop_bottom, exp_ctop);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_M, exp_ctop_mid, exp_ctop);
	ADD(&autorelease, sd_t_mb_ct, ID_T, exp_ctop_midbottom); DEL(&autorelease, sd_t_mb_ct, ID_T, exp_ctop_midbottom);
	ADD(&autorelease, sd_t_mb_ct, ID_MB, exp_ctop_midbottom); DEL(&autorelease, sd_t_mb_ct, ID_MB, exp_ctop);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_TB, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_TM, exp_ctop, exp_ctop);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_t_mb_ct, ID_CB, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_mb_ct, ID_CM, exp_ctop_cmid);
	ADD(&autorelease, sd_t_mb_ct, ID_CT, exp_ctop_midbottom); DEL(&autorelease, sd_t_mb_ct, ID_CT, exp_top_midbottom);
	struct map_session_data *sd_t_mb_ct_cmb = ADD(&autorelease, sd_t_mb_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cmb+x
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_B, exp_cmidbottom_top, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_M, exp_cmidbottom_top, exp_cmidbottom_top);
	ADD(&autorelease, sd_t_mb_cmb, ID_T, exp_cmidbottom_top); DEL(&autorelease, sd_t_mb_cmb, ID_T, exp_cmidbottom);
	ADD(&autorelease, sd_t_mb_cmb, ID_MB, exp_cmidbottom_top); DEL(&autorelease, sd_t_mb_cmb, ID_MB, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_CB, exp_cbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_CM, exp_cmid_top, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_mb_cmb, ID_CMB, exp_cmidbottom_top); DEL(&autorelease, sd_t_mb_cmb, ID_CMB, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+ctb+x
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_B, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_M, exp_ctopbottom_mid, exp_ctopbottom);
	ADD(&autorelease, sd_t_mb_ctb, ID_T, exp_ctopbottom); DEL(&autorelease, sd_t_mb_ctb, ID_T, exp_ctopbottom);
	ADD(&autorelease, sd_t_mb_ctb, ID_MB, exp_ctopbottom); DEL(&autorelease, sd_t_mb_ctb, ID_MB, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_TB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_CB, exp_cbottom_top, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_CT, exp_ctop_midbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_ctb, ID_CTB, exp_ctopbottom); DEL(&autorelease, sd_t_mb_ctb, ID_CTB, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+ctm+x
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_B, exp_ctopmid_bottom, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_M, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_t_mb_ctm, ID_T, exp_ctopmid); DEL(&autorelease, sd_t_mb_ctm, ID_T, exp_ctopmid);
	ADD(&autorelease, sd_t_mb_ctm, ID_MB, exp_ctopmid); DEL(&autorelease, sd_t_mb_ctm, ID_MB, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_TM, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_t_mb_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_CM, exp_cmid_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_CT, exp_ctop_midbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_ctm, ID_CTM, exp_ctopmid); DEL(&autorelease, sd_t_mb_ctm, ID_CTM, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+ctmb+x
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_B, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_M, exp_ctopmidbottom, exp_ctopmidbottom);
	ADD(&autorelease, sd_t_mb_ctmb, ID_T, exp_ctopmidbottom); DEL(&autorelease, sd_t_mb_ctmb, ID_T, exp_ctopmidbottom);
	ADD(&autorelease, sd_t_mb_ctmb, ID_MB, exp_ctopmidbottom); DEL(&autorelease, sd_t_mb_ctmb, ID_MB, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CB, exp_cbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CM, exp_cmid_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CT, exp_ctop_midbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ctmb, ID_CTM, exp_ctopmid, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_t_mb_ctmb, ID_CTMB, exp_top_midbottom);
	// t+cb+cm+x: t+cb+cm+ct [1]
	ADD(&autorelease, sd_t_cb_cm, ID_B, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_t_cb_cm, ID_M, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_t_cb_cm, ID_T, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_cb_cm, ID_T, exp_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm, ID_MB, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm, ID_CB, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_cb_cm, ID_CB, exp_cmid_top);
	ADD(&autorelease, sd_t_cb_cm, ID_CM, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_cb_cm, ID_CM, exp_cbottom_top);
	struct map_session_data *sd_t_cb_cm_ct = ADD(&autorelease, sd_t_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_CMB, exp_cmidbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+cb+ct+x
	ADD(&autorelease, sd_t_cb_ct, ID_B, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_cb_ct, ID_M, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_t_cb_ct, ID_T, exp_ctop_cbottom); DEL(&autorelease, sd_t_cb_ct, ID_T, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_cb_ct, ID_MB, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_t_cb_ct, ID_CB, exp_ctop);
	ADD(&autorelease, sd_t_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_t_cb_ct, ID_CT, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_CTB, exp_ctopbottom, exp_top);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+cb+ctm+x
	ADD(&autorelease, sd_t_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_cb_ctm, ID_T, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_cb_ctm, ID_T, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_cb_ctm, ID_MB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_CM, exp_cmid_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_CMB, exp_cmidbottom_top, exp_top);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_CTB, exp_ctopbottom, exp_top);
	ADD(&autorelease, sd_t_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_cb_ctm, ID_CTM, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+cm+ct+x
	ADD(&autorelease, sd_t_cm_ct, ID_B, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_t_cm_ct, ID_M, exp_ctop_cmid);
	ADD(&autorelease, sd_t_cm_ct, ID_T, exp_ctop_cmid); DEL(&autorelease, sd_t_cm_ct, ID_T, exp_ctop_cmid);
	ADD(&autorelease, sd_t_cm_ct, ID_MB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_t_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_t_cm_ct, ID_CM, exp_ctop);
	ADD(&autorelease, sd_t_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_t_cm_ct, ID_CT, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+cm+ctb+x
	ADD(&autorelease, sd_t_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_t_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_t_cm_ctb, ID_T, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_cm_ctb, ID_T, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_t_cm_ctb, ID_MB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_CB, exp_cmid_cbottom_top, exp_cmid_top);
	ADD(&autorelease, sd_t_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_CMB, exp_cmidbottom_top, exp_top);
	ADD(&autorelease, sd_t_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_cm_ctb, ID_CTB, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_top);
	// t+ct+cmb+x
	ADD(&autorelease, sd_t_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_ct_cmb, ID_T, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_ct_cmb, ID_T, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_ct_cmb, ID_MB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop);
	ADD(&autorelease, sd_t_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_ct_cmb, ID_CT, exp_cmidbottom_top);
	ADD(&autorelease, sd_t_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_ct_cmb, ID_CMB, exp_ctop);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_CTB, exp_ctopbottom, exp_top);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_CTM, exp_ctopmid, exp_top);
	TOGGLE(&autorelease, sd_t_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_top);
	// mb+cb+cm+x: mb+cb+cm+ct [1]
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_B, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_M, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_cm, ID_T, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_mb_cb_cm, ID_MB, exp_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm, ID_MB, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm, ID_CB, exp_cmid);
	ADD(&autorelease, sd_mb_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm, ID_CM, exp_cbottom);
	struct map_session_data *sd_mb_cb_cm_ct = ADD(&autorelease, sd_mb_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_CMB, exp_cmidbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+cb+ct+x
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_B, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_M, exp_ctop_cbottom_mid, exp_ctop_cbottom);
	ADD(&autorelease, sd_mb_cb_ct, ID_T, exp_ctop_cbottom);
	ADD(&autorelease, sd_mb_cb_ct, ID_MB, exp_ctop_cbottom); DEL(&autorelease, sd_mb_cb_ct, ID_MB, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_mb_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_mb_cb_ct, ID_CB, exp_ctop_midbottom);
	ADD(&autorelease, sd_mb_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_mb_cb_ct, ID_CT, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_CTB, exp_ctopbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+cb+ctm+x
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_B, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_M, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_mb_cb_ctm, ID_T, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_mb_cb_ctm, ID_MB, exp_ctopmid_cbottom); DEL(&autorelease, sd_mb_cb_ctm, ID_MB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_mb_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_mb_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_CMB, exp_cmidbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_CTB, exp_ctopbottom, exp_midbottom);
	ADD(&autorelease, sd_mb_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_mb_cb_ctm, ID_CTM, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+cm+ct+x
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_B, exp_ctop_cmid_bottom, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_M, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_mb_cm_ct, ID_T, exp_ctop_cmid);
	ADD(&autorelease, sd_mb_cm_ct, ID_MB, exp_ctop_cmid); DEL(&autorelease, sd_mb_cm_ct, ID_MB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_mb_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_mb_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_mb_cm_ct, ID_CM, exp_ctop_midbottom);
	ADD(&autorelease, sd_mb_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_mb_cm_ct, ID_CT, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+cm+ctb+x
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_B, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_M, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_mb_cm_ctb, ID_T, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_mb_cm_ctb, ID_MB, exp_ctopbottom_cmid); DEL(&autorelease, sd_mb_cm_ctb, ID_MB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid);
	ADD(&autorelease, sd_mb_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_mb_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_CMB, exp_cmidbottom, exp_midbottom);
	ADD(&autorelease, sd_mb_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_mb_cm_ctb, ID_CTB, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// mb+ct+cmb+x
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_B, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_M, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_mb_ct_cmb, ID_T, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_mb_ct_cmb, ID_MB, exp_ctop_cmidbottom); DEL(&autorelease, sd_mb_ct_cmb, ID_MB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop_midbottom);
	ADD(&autorelease, sd_mb_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_mb_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_mb_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_mb_ct_cmb, ID_CMB, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_CTB, exp_ctopbottom, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_CTM, exp_ctopmid, exp_midbottom);
	TOGGLE(&autorelease, sd_mb_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// tb+cb+cm+x: tb+cb+cm+ct [1]
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_B, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm, ID_M, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_T, exp_cmid_cbottom_top, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm, ID_TB, exp_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm, ID_TB, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm, ID_CB, exp_cmid_topbottom);
	ADD(&autorelease, sd_tb_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm, ID_CM, exp_cbottom);
	struct map_session_data *sd_tb_cb_cm_ct = ADD(&autorelease, sd_tb_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_CMB, exp_cmidbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+cb+ct+x
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_B, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tb_cb_ct, ID_M, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_T, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tb_cb_ct, ID_TB, exp_ctop_cbottom); DEL(&autorelease, sd_tb_cb_ct, ID_TB, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tb_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_tb_cb_ct, ID_CB, exp_ctop);
	ADD(&autorelease, sd_tb_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_tb_cb_ct, ID_CT, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_CTB, exp_ctopbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+cb+ctm+x
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_B, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tb_cb_ctm, ID_M, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_T, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tb_cb_ctm, ID_TB, exp_ctopmid_cbottom); DEL(&autorelease, sd_tb_cb_ctm, ID_TB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tb_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_tb_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_CMB, exp_cmidbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_CTB, exp_ctopbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_tb_cb_ctm, ID_CTM, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+cm+ct+x
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_B, exp_ctop_cmid_bottom, exp_ctop_cmid);
	ADD(&autorelease, sd_tb_cm_ct, ID_M, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_T, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_tb_cm_ct, ID_TB, exp_ctop_cmid); DEL(&autorelease, sd_tb_cm_ct, ID_TB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_tb_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tb_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_tb_cm_ct, ID_CM, exp_ctop);
	ADD(&autorelease, sd_tb_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_tb_cm_ct, ID_CT, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+cm+ctb+x
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_B, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_tb_cm_ctb, ID_M, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_T, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_tb_cm_ctb, ID_TB, exp_ctopbottom_cmid); DEL(&autorelease, sd_tb_cm_ctb, ID_TB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid_topbottom);
	ADD(&autorelease, sd_tb_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_tb_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_CMB, exp_cmidbottom, exp_topbottom);
	ADD(&autorelease, sd_tb_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_tb_cm_ctb, ID_CTB, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tb+ct+cmb
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_B, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tb_ct_cmb, ID_M, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_T, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tb_ct_cmb, ID_TB, exp_ctop_cmidbottom); DEL(&autorelease, sd_tb_ct_cmb, ID_TB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop);
	ADD(&autorelease, sd_tb_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_tb_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_tb_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_tb_ct_cmb, ID_CMB, exp_ctop);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_CTB, exp_ctopbottom, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_CTM, exp_ctopmid, exp_topbottom);
	TOGGLE(&autorelease, sd_tb_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tm+cb+cm+x: tm+cb+cm+ct [1]
	ADD(&autorelease, sd_tm_cb_cm, ID_B, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_M, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_T, exp_cmid_cbottom_top, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tm_cb_cm, ID_TM, exp_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm, ID_TM, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tm_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm, ID_CB, exp_cmid);
	ADD(&autorelease, sd_tm_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm, ID_CM, exp_cbottom_topmid);
	struct map_session_data *sd_tm_cb_cm_ct = ADD(&autorelease, sd_tm_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_CMB, exp_cmidbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+cb+ct+x
	ADD(&autorelease, sd_tm_cb_ct, ID_B, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_M, exp_ctop_cbottom_mid, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_T, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tm_cb_ct, ID_TM, exp_ctop_cbottom); DEL(&autorelease, sd_tm_cb_ct, ID_TM, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tm_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_tm_cb_ct, ID_CB, exp_ctop);
	ADD(&autorelease, sd_tm_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tm_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_tm_cb_ct, ID_CT, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_CTB, exp_ctopbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+cb+ctm+x
	ADD(&autorelease, sd_tm_cb_ctm, ID_B, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_M, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_T, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tm_cb_ctm, ID_TM, exp_ctopmid_cbottom); DEL(&autorelease, sd_tm_cb_ctm, ID_TM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tm_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_tm_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_CMB, exp_cmidbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_CTB, exp_ctopbottom, exp_topmid);
	ADD(&autorelease, sd_tm_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_tm_cb_ctm, ID_CTM, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+cm+ct+x
	ADD(&autorelease, sd_tm_cm_ct, ID_B, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_M, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_T, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_tm_cm_ct, ID_TM, exp_ctop_cmid); DEL(&autorelease, sd_tm_cm_ct, ID_TM, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_tm_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tm_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_tm_cm_ct, ID_CM, exp_ctop);
	ADD(&autorelease, sd_tm_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_tm_cm_ct, ID_CT, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+cm+ctb+x
	ADD(&autorelease, sd_tm_cm_ctb, ID_B, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_M, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_T, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_tm_cm_ctb, ID_TM, exp_ctopbottom_cmid); DEL(&autorelease, sd_tm_cm_ctb, ID_TM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid);
	ADD(&autorelease, sd_tm_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_tm_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_CMB, exp_cmidbottom, exp_topmid);
	ADD(&autorelease, sd_tm_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_tm_cm_ctb, ID_CTB, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tm+ct+cmb+x
	ADD(&autorelease, sd_tm_ct_cmb, ID_B, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_M, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_T, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tm_ct_cmb, ID_TM, exp_ctop_cmidbottom); DEL(&autorelease, sd_tm_ct_cmb, ID_TM, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop);
	ADD(&autorelease, sd_tm_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_tm_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_tm_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_tm_ct_cmb, ID_CMB, exp_ctop);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_CTB, exp_ctopbottom, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_CTM, exp_ctopmid, exp_topmid);
	TOGGLE(&autorelease, sd_tm_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tmb+cb+cm+x: tmb+cb+cm+ct [1]
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_B, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_M, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_T, exp_cmid_cbottom_top, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_cm, ID_TMB, exp_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm, ID_TMB, exp_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm, ID_CB, exp_cmid);
	ADD(&autorelease, sd_tmb_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm, ID_CM, exp_cbottom);
	struct map_session_data *sd_tmb_cb_cm_ct = ADD(&autorelease, sd_tmb_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+cb+ct+x
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_B, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_M, exp_ctop_cbottom_mid, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_T, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_tmb_cb_ct, ID_TMB, exp_ctop_cbottom); DEL(&autorelease, sd_tmb_cb_ct, ID_TMB, exp_ctop_cbottom);
	ADD(&autorelease, sd_tmb_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_tmb_cb_ct, ID_CB, exp_ctop);
	ADD(&autorelease, sd_tmb_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_tmb_cb_ct, ID_CT, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+cb+ctm+x
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_B, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_M, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_T, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_ctm, ID_TMB, exp_ctopmid_cbottom); DEL(&autorelease, sd_tmb_cb_ctm, ID_TMB, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_tmb_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_tmb_cb_ctm, ID_CTM, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+cm+ct+x
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_B, exp_ctop_cmid_bottom, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_M, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_T, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_tmb_cm_ct, ID_TMB, exp_ctop_cmid); DEL(&autorelease, sd_tmb_cm_ct, ID_TMB, exp_ctop_cmid);
	ADD(&autorelease, sd_tmb_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_tmb_cm_ct, ID_CM, exp_ctop);
	ADD(&autorelease, sd_tmb_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_tmb_cm_ct, ID_CT, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+cm+ctb+x
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_B, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_M, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_T, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_tmb_cm_ctb, ID_TMB, exp_ctopbottom_cmid); DEL(&autorelease, sd_tmb_cm_ctb, ID_TMB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid);
	ADD(&autorelease, sd_tmb_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_tmb_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_CMB, exp_cmidbottom, exp_topmidbottom);
	ADD(&autorelease, sd_tmb_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_tmb_cm_ctb, ID_CTB, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// tmb+ct+cmb+x
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_B, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_M, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_T, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_tmb_ct_cmb, ID_TMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_tmb_ct_cmb, ID_TMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop);
	ADD(&autorelease, sd_tmb_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_tmb_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_tmb_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_tmb_ct_cmb, ID_CMB, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_CTB, exp_ctopbottom, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_CTM, exp_ctopmid, exp_topmidbottom);
	TOGGLE(&autorelease, sd_tmb_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);
	// cb+cm+ct+x
	ADD(&autorelease, sd_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_empty);

	// Five: (12)
	// b+m+t+cb+x: b+m+t+cb+cm b+m+t+cb+ct b+m+t+cb+ctm [3]
	ADD(&autorelease, sd_b_m_t_cb, ID_B, exp_cbottom_top_mid); DEL(&autorelease, sd_b_m_t_cb, ID_B, exp_cbottom_top_mid);
	ADD(&autorelease, sd_b_m_t_cb, ID_M, exp_cbottom_top_mid); DEL(&autorelease, sd_b_m_t_cb, ID_M, exp_cbottom_top);
	ADD(&autorelease, sd_b_m_t_cb, ID_T, exp_cbottom_top_mid); DEL(&autorelease, sd_b_m_t_cb, ID_T, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_MB, exp_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_TB, exp_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_TM, exp_cbottom_topmid, exp_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_TMB, exp_cbottom, exp_cbottom);
	ADD(&autorelease, sd_b_m_t_cb, ID_CB, exp_cbottom_top_mid); DEL(&autorelease, sd_b_m_t_cb, ID_CB, exp_top_mid_bottom);
	struct map_session_data *sd_b_m_t_cb_cm = ADD(&autorelease, sd_b_m_t_cb, ID_CM, exp_cmid_cbottom_top);
	struct map_session_data *sd_b_m_t_cb_ct = ADD(&autorelease, sd_b_m_t_cb, ID_CT, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	struct map_session_data *sd_b_m_t_cb_ctm = ADD(&autorelease, sd_b_m_t_cb, ID_CTM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cm+x: b+m+t+cm+ct b+m+t+cm+ctb [2]
	ADD(&autorelease, sd_b_m_t_cm, ID_B, exp_cmid_top_bottom); DEL(&autorelease, sd_b_m_t_cm, ID_B, exp_cmid_top);
	ADD(&autorelease, sd_b_m_t_cm, ID_M, exp_cmid_top_bottom); DEL(&autorelease, sd_b_m_t_cm, ID_M, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_m_t_cm, ID_T, exp_cmid_top_bottom); DEL(&autorelease, sd_b_m_t_cm, ID_T, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_MB, exp_cmid_top, exp_cmid_top);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_TB, exp_cmid_topbottom, exp_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_TM, exp_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_TMB, exp_cmid, exp_cmid);
	ADD(&autorelease, sd_b_m_t_cm, ID_CB, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_m_t_cm, ID_CM, exp_cmid_top_bottom); DEL(&autorelease, sd_b_m_t_cm, ID_CM, exp_top_mid_bottom);
	struct map_session_data *sd_b_m_t_cm_ct = ADD(&autorelease, sd_b_m_t_cm, ID_CT, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	struct map_session_data *sd_b_m_t_cm_ctb = ADD(&autorelease, sd_b_m_t_cm, ID_CTB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+ct+x: b+m+t+ct+cmb [1]
	ADD(&autorelease, sd_b_m_t_ct, ID_B, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_t_ct, ID_B, exp_ctop_mid);
	ADD(&autorelease, sd_b_m_t_ct, ID_M, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_t_ct, ID_M, exp_ctop_bottom);
	ADD(&autorelease, sd_b_m_t_ct, ID_T, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_t_ct, ID_T, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_MB, exp_ctop_midbottom, exp_ctop);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_TB, exp_ctop_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_TM, exp_ctop_bottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_TMB, exp_ctop, exp_ctop);
	ADD(&autorelease, sd_b_m_t_ct, ID_CB, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_t_ct, ID_CM, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_t_ct, ID_CT, exp_ctop_mid_bottom); DEL(&autorelease, sd_b_m_t_ct, ID_CT, exp_top_mid_bottom);
	struct map_session_data *sd_b_m_t_ct_cmb = ADD(&autorelease, sd_b_m_t_ct, ID_CMB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cmb+x
	ADD(&autorelease, sd_b_m_t_cmb, ID_B, exp_cmidbottom_top); DEL(&autorelease, sd_b_m_t_cmb, ID_B, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_m_t_cmb, ID_M, exp_cmidbottom_top); DEL(&autorelease, sd_b_m_t_cmb, ID_M, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_m_t_cmb, ID_T, exp_cmidbottom_top); DEL(&autorelease, sd_b_m_t_cmb, ID_T, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_MB, exp_cmidbottom_top, exp_cmidbottom_top);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_TB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_TM, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_TMB, exp_cmidbottom, exp_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_CB, exp_cbottom_top_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_CM, exp_cmid_top_bottom, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_cmb, ID_CT, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_t_cmb, ID_CMB, exp_cmidbottom_top); DEL(&autorelease, sd_b_m_t_cmb, ID_CMB, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+ctb+x
	ADD(&autorelease, sd_b_m_t_ctb, ID_B, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_t_ctb, ID_B, exp_ctopbottom_mid);
	ADD(&autorelease, sd_b_m_t_ctb, ID_M, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_t_ctb, ID_M, exp_ctopbottom);
	ADD(&autorelease, sd_b_m_t_ctb, ID_T, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_t_ctb, ID_T, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_MB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_TB, exp_ctopbottom_mid, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_TM, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_TMB, exp_ctopbottom, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_CB, exp_cbottom_top_mid, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_ctb, ID_CM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_CT, exp_ctop_mid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_ctb, ID_CTB, exp_ctopbottom_mid); DEL(&autorelease, sd_b_m_t_ctb, ID_CTB, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+ctm+x
	ADD(&autorelease, sd_b_m_t_ctm, ID_B, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_t_ctm, ID_B, exp_ctopmid);
	ADD(&autorelease, sd_b_m_t_ctm, ID_M, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_t_ctm, ID_M, exp_ctopmid_bottom);
	ADD(&autorelease, sd_b_m_t_ctm, ID_T, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_t_ctm, ID_T, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_MB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_TB, exp_ctopmid, exp_ctopmid);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_TM, exp_ctopmid_bottom, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_TMB, exp_ctopmid, exp_ctopmid);
	ADD(&autorelease, sd_b_m_t_ctm, ID_CB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_CM, exp_cmid_top_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_CT, exp_ctop_mid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_ctm, ID_CTM, exp_ctopmid_bottom); DEL(&autorelease, sd_b_m_t_ctm, ID_CTM, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+ctmb+x
	ADD(&autorelease, sd_b_m_t_ctmb, ID_B, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_t_ctmb, ID_B, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_m_t_ctmb, ID_M, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_t_ctmb, ID_M, exp_ctopmidbottom);
	ADD(&autorelease, sd_b_m_t_ctmb, ID_T, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_t_ctmb, ID_T, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_MB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_TB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_TM, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_TMB, exp_ctopmidbottom, exp_ctopmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CB, exp_cbottom_top_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CM, exp_cmid_top_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CT, exp_ctop_mid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ctmb, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_ctmb, ID_CTMB, exp_ctopmidbottom); DEL(&autorelease, sd_b_m_t_ctmb, ID_CTMB, exp_top_mid_bottom);
	// b+m+cb+cm+x: b+m+cb+cm+ct [1]
	ADD(&autorelease, sd_b_m_cb_cm, ID_B, exp_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm, ID_B, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm, ID_M, exp_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm, ID_M, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm, ID_T, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm, ID_CB, exp_cmid_bottom);
	ADD(&autorelease, sd_b_m_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm, ID_CM, exp_cbottom_mid);
	struct map_session_data *sd_b_m_cb_cm_ct = ADD(&autorelease, sd_b_m_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cb+ct+x
	ADD(&autorelease, sd_b_m_cb_ct, ID_B, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_cb_ct, ID_B, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_cb_ct, ID_M, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_cb_ct, ID_M, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_m_cb_ct, ID_T, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_TB, exp_ctop_cbottom_mid, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_m_cb_ct, ID_CB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_cb_ct, ID_CB, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_ct, ID_CT, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_cb_ct, ID_CT, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cb+ctm+x
	ADD(&autorelease, sd_b_m_cb_ctm, ID_B, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_ctm, ID_M, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_cb_ctm, ID_CB, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_CT, exp_ctop_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_cb_ctm, ID_CTM, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cm+ct+x
	ADD(&autorelease, sd_b_m_cm_ct, ID_B, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_cm_ct, ID_B, exp_ctop_cmid);
	ADD(&autorelease, sd_b_m_cm_ct, ID_M, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_cm_ct, ID_M, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_cm_ct, ID_T, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_TM, exp_ctop_cmid_bottom, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_m_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cm_ct, ID_CM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_cm_ct, ID_CM, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_cm_ct, ID_CT, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_cm_ct, ID_CT, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+cm+ctb+x
	ADD(&autorelease, sd_b_m_cm_ctb, ID_B, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_m_cm_ctb, ID_M, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_m_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid_bottom);
	ADD(&autorelease, sd_b_m_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_cm_ctb, ID_CM, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_CT, exp_ctop_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_CMB, exp_cmidbottom, exp_mid_bottom);
	ADD(&autorelease, sd_b_m_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_cm_ctb, ID_CTB, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+m+ct+cmb+x
	ADD(&autorelease, sd_b_m_ct_cmb, ID_B, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_ct_cmb, ID_M, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_CB, exp_ctop_cbottom_mid, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_CM, exp_ctop_cmid_bottom, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_b_m_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_ct_cmb, ID_CMB, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_CTB, exp_ctopbottom_mid, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_CTM, exp_ctopmid_bottom, exp_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+t+cb+cm+x: b+t+cb+cm+ct [1]
	ADD(&autorelease, sd_b_t_cb_cm, ID_B, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_t_cb_cm, ID_B, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_t_cb_cm, ID_M, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_t_cb_cm, ID_T, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_t_cb_cm, ID_T, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_MB, exp_cmid_cbottom_top, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_cm, ID_CB, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_t_cb_cm, ID_CB, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_t_cb_cm, ID_CM, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_t_cb_cm, ID_CM, exp_cbottom_top);
	struct map_session_data *sd_b_t_cb_cm_ct = ADD(&autorelease, sd_b_t_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cb+ct+x
	ADD(&autorelease, sd_b_t_cb_ct, ID_B, exp_ctop_cbottom); DEL(&autorelease, sd_b_t_cb_ct, ID_B, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_t_cb_ct, ID_M, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_t_cb_ct, ID_T, exp_ctop_cbottom); DEL(&autorelease, sd_b_t_cb_ct, ID_T, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_t_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_b_t_cb_ct, ID_CB, exp_ctop_bottom);
	ADD(&autorelease, sd_b_t_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_b_t_cb_ct, ID_CT, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_CTB, exp_ctopbottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cb+ctm+x
	ADD(&autorelease, sd_b_t_cb_ctm, ID_B, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_t_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_ctm, ID_T, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_t_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_t_cb_ctm, ID_CB, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_CM, exp_cmid_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_CTB, exp_ctopbottom, exp_top_bottom);
	ADD(&autorelease, sd_b_t_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_t_cb_ctm, ID_CTM, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cm+ct+x
	ADD(&autorelease, sd_b_t_cm_ct, ID_B, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_t_cm_ct, ID_B, exp_ctop_cmid);
	ADD(&autorelease, sd_b_t_cm_ct, ID_M, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_t_cm_ct, ID_T, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_t_cm_ct, ID_T, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_TM, exp_ctop_cmid_bottom, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_t_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_t_cm_ct, ID_CM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_t_cm_ct, ID_CM, exp_ctop_bottom);
	ADD(&autorelease, sd_b_t_cm_ct, ID_CT, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_t_cm_ct, ID_CT, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+cm+ctb+x
	ADD(&autorelease, sd_b_t_cm_ctb, ID_B, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_t_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_t_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_t_cm_ctb, ID_T, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_t_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_CB, exp_cmid_cbottom_top, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_t_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_t_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_CT, exp_ctop_cmid_bottom, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_CMB, exp_cmidbottom_top, exp_top_bottom);
	ADD(&autorelease, sd_b_t_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_t_cm_ctb, ID_CTB, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+t+ct+cmb+x
	ADD(&autorelease, sd_b_t_ct_cmb, ID_B, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_t_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_t_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_t_ct_cmb, ID_T, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_t_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_CM, exp_ctop_cmid_bottom, exp_ctop_bottom);
	ADD(&autorelease, sd_b_t_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_t_ct_cmb, ID_CT, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_t_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_t_ct_cmb, ID_CMB, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_CTB, exp_ctopbottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_CTM, exp_ctopmid_bottom, exp_top_bottom);
	TOGGLE(&autorelease, sd_b_t_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+tm+cb+cm+x: b+tm+cb+cm+ct [1]
	ADD(&autorelease, sd_b_tm_cb_cm, ID_B, exp_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm, ID_B, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_M, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_T, exp_cmid_cbottom_top, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_cm, ID_TM, exp_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm, ID_TM, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm, ID_CB, exp_cmid_bottom);
	ADD(&autorelease, sd_b_tm_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm, ID_CM, exp_cbottom_topmid);
	struct map_session_data *sd_b_tm_cb_cm_ct = ADD(&autorelease, sd_b_tm_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cb+ct+x
	ADD(&autorelease, sd_b_tm_cb_ct, ID_B, exp_ctop_cbottom); DEL(&autorelease, sd_b_tm_cb_ct, ID_B, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_M, exp_ctop_cbottom_mid, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_T, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_tm_cb_ct, ID_TM, exp_ctop_cbottom); DEL(&autorelease, sd_b_tm_cb_ct, ID_TM, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_tm_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_b_tm_cb_ct, ID_CB, exp_ctop_bottom);
	ADD(&autorelease, sd_b_tm_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_b_tm_cb_ct, ID_CT, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cb+ctm+x
	ADD(&autorelease, sd_b_tm_cb_ctm, ID_B, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_tm_cb_ctm, ID_B, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_M, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_T, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_ctm, ID_TM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_tm_cb_ctm, ID_TM, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_tm_cb_ctm, ID_CB, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_tm_cb_ctm, ID_CTM, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cm+ct+x
	ADD(&autorelease, sd_b_tm_cm_ct, ID_B, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_tm_cm_ct, ID_B, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_M, exp_ctop_cmid_bottom, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_T, exp_ctop_cmid_bottom, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_tm_cm_ct, ID_TM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_tm_cm_ct, ID_TM, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_tm_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cm_ct, ID_CM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_tm_cm_ct, ID_CM, exp_ctop_bottom);
	ADD(&autorelease, sd_b_tm_cm_ct, ID_CT, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_tm_cm_ct, ID_CT, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+cm+ctb+x
	ADD(&autorelease, sd_b_tm_cm_ctb, ID_B, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_tm_cm_ctb, ID_B, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_M, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_T, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_tm_cm_ctb, ID_TM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_tm_cm_ctb, ID_TM, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid_bottom);
	ADD(&autorelease, sd_b_tm_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_tm_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_CT, exp_ctop_cmid_bottom, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_CMB, exp_cmidbottom, exp_topmid_bottom);
	ADD(&autorelease, sd_b_tm_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_tm_cm_ctb, ID_CTB, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+tm+ct+cmb+x
	ADD(&autorelease, sd_b_tm_ct_cmb, ID_B, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_tm_ct_cmb, ID_B, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_M, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_T, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_tm_ct_cmb, ID_TM, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_tm_ct_cmb, ID_TM, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_CM, exp_ctop_cmid_bottom, exp_ctop_bottom);
	ADD(&autorelease, sd_b_tm_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_tm_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_b_tm_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_tm_ct_cmb, ID_CMB, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_CTB, exp_ctopbottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_CTM, exp_ctopmid_bottom, exp_topmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// b+cb+cm+ct+x
	ADD(&autorelease, sd_b_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm_ct, ID_CB, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_b_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_bottom);
	// m+t+cb+cm+x: m+t+cb+cm+ct [1]
	ADD(&autorelease, sd_m_t_cb_cm, ID_B, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_m_t_cb_cm, ID_M, exp_cmid_cbottom_top); DEL(&autorelease, sd_m_t_cb_cm, ID_M, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_m_t_cb_cm, ID_T, exp_cmid_cbottom_top); DEL(&autorelease, sd_m_t_cb_cm, ID_T, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_MB, exp_cmid_cbottom_top, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_cm, ID_CB, exp_cmid_cbottom_top); DEL(&autorelease, sd_m_t_cb_cm, ID_CB, exp_cmid_top);
	ADD(&autorelease, sd_m_t_cb_cm, ID_CM, exp_cmid_cbottom_top); DEL(&autorelease, sd_m_t_cb_cm, ID_CM, exp_cbottom_top_mid);
	struct map_session_data *sd_m_t_cb_cm_ct = ADD(&autorelease, sd_m_t_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cb+ct+x
	ADD(&autorelease, sd_m_t_cb_ct, ID_B, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_t_cb_ct, ID_M, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_t_cb_ct, ID_M, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_t_cb_ct, ID_T, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_t_cb_ct, ID_T, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_TB, exp_ctop_cbottom_mid, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_t_cb_ct, ID_CB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_t_cb_ct, ID_CB, exp_ctop_mid);
	ADD(&autorelease, sd_m_t_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_ct, ID_CT, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_t_cb_ct, ID_CT, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cb+ctm+x
	ADD(&autorelease, sd_m_t_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_ctm, ID_M, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_t_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_ctm, ID_T, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_t_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_t_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_CM, exp_cmid_cbottom_top, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_CT, exp_ctop_cbottom_mid, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	ADD(&autorelease, sd_m_t_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_t_cb_ctm, ID_CTM, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cm+ct+x
	ADD(&autorelease, sd_m_t_cm_ct, ID_B, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_m_t_cm_ct, ID_M, exp_ctop_cmid); DEL(&autorelease, sd_m_t_cm_ct, ID_M, exp_ctop_cmid);
	ADD(&autorelease, sd_m_t_cm_ct, ID_T, exp_ctop_cmid); DEL(&autorelease, sd_m_t_cm_ct, ID_T, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_m_t_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_m_t_cm_ct, ID_CM, exp_ctop_mid);
	ADD(&autorelease, sd_m_t_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_m_t_cm_ct, ID_CT, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+cm+ctb+x
	ADD(&autorelease, sd_m_t_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_t_cm_ctb, ID_M, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_t_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_t_cm_ctb, ID_T, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_t_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_CB, exp_cmid_cbottom_top, exp_cmid_top);
	ADD(&autorelease, sd_m_t_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_t_cm_ctb, ID_CM, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_CMB, exp_cmidbottom_top, exp_top_mid);
	ADD(&autorelease, sd_m_t_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_t_cm_ctb, ID_CTB, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+t+ct+cmb+x
	ADD(&autorelease, sd_m_t_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_t_ct_cmb, ID_M, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_t_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_t_ct_cmb, ID_T, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_t_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_CB, exp_ctop_cbottom_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop_mid);
	ADD(&autorelease, sd_m_t_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_t_ct_cmb, ID_CT, exp_cmidbottom_top);
	ADD(&autorelease, sd_m_t_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_t_ct_cmb, ID_CMB, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_CTM, exp_ctopmid, exp_top_mid);
	TOGGLE(&autorelease, sd_m_t_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+tb+cb+cm+x: m+tb+cb+cm+ct [1]
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_B, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm, ID_M, exp_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm, ID_M, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_T, exp_cmid_cbottom_top, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_MB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm, ID_TB, exp_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm, ID_TB, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm, ID_CB, exp_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm, ID_CB, exp_cmid_topbottom);
	ADD(&autorelease, sd_m_tb_cb_cm, ID_CM, exp_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm, ID_CM, exp_cbottom_mid);
	struct map_session_data *sd_m_tb_cb_cm_ct = ADD(&autorelease, sd_m_tb_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cb+ct+x
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_B, exp_ctop_cbottom_mid, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_tb_cb_ct, ID_M, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_tb_cb_ct, ID_M, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_T, exp_ctop_cbottom_mid, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ct, ID_TB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_tb_cb_ct, ID_TB, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ct, ID_CB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_tb_cb_ct, ID_CB, exp_ctop_mid);
	ADD(&autorelease, sd_m_tb_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ct, ID_CT, exp_ctop_cbottom_mid); DEL(&autorelease, sd_m_tb_cb_ct, ID_CT, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cb+ctm+x
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_B, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ctm, ID_M, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_tb_cb_ctm, ID_M, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_T, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ctm, ID_TB, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_tb_cb_ctm, ID_TB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_tb_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_CM, exp_cmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_CT, exp_ctop_cbottom_mid, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_m_tb_cb_ctm, ID_CTM, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cm+ct+x
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_B, exp_ctop_cmid_bottom, exp_ctop_cmid);
	ADD(&autorelease, sd_m_tb_cm_ct, ID_M, exp_ctop_cmid); DEL(&autorelease, sd_m_tb_cm_ct, ID_M, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_T, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_m_tb_cm_ct, ID_TB, exp_ctop_cmid); DEL(&autorelease, sd_m_tb_cm_ct, ID_TB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_m_tb_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_m_tb_cm_ct, ID_CM, exp_ctop_mid);
	ADD(&autorelease, sd_m_tb_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_m_tb_cm_ct, ID_CT, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+cm+ctb+x
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_B, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_tb_cm_ctb, ID_M, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_tb_cm_ctb, ID_M, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_T, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_m_tb_cm_ctb, ID_TB, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_tb_cm_ctb, ID_TB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_CB, exp_cmid_cbottom, exp_cmid_topbottom);
	ADD(&autorelease, sd_m_tb_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_tb_cm_ctb, ID_CM, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_CMB, exp_cmidbottom, exp_topbottom_mid);
	ADD(&autorelease, sd_m_tb_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_m_tb_cm_ctb, ID_CTB, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+tb+ct+cmb+x
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_B, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_tb_ct_cmb, ID_M, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_tb_ct_cmb, ID_M, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_T, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_m_tb_ct_cmb, ID_TB, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_tb_ct_cmb, ID_TB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_CB, exp_ctop_cbottom_mid, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop_mid);
	ADD(&autorelease, sd_m_tb_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_tb_ct_cmb, ID_CT, exp_cmidbottom);
	ADD(&autorelease, sd_m_tb_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_m_tb_ct_cmb, ID_CMB, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_CTB, exp_ctopbottom_mid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_CTM, exp_ctopmid, exp_topbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// m+cb+cm+ct+x
	ADD(&autorelease, sd_m_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm_ct, ID_CM, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_mid);
	// t+mb+cb+cm+x: t+mb+cb+cm+ct [1]
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_B, exp_cmid_cbottom_top, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_M, exp_cmid_cbottom_top, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_t_mb_cb_cm, ID_T, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_mb_cb_cm, ID_T, exp_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm, ID_MB, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_mb_cb_cm, ID_MB, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm, ID_CB, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_mb_cb_cm, ID_CB, exp_cmid_top);
	ADD(&autorelease, sd_t_mb_cb_cm, ID_CM, exp_cmid_cbottom_top); DEL(&autorelease, sd_t_mb_cb_cm, ID_CM, exp_cbottom_top);
	struct map_session_data *sd_t_mb_cb_cm_ct = ADD(&autorelease, sd_t_mb_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cb+ct+x
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_B, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_M, exp_ctop_cbottom_mid, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ct, ID_T, exp_ctop_cbottom); DEL(&autorelease, sd_t_mb_cb_ct, ID_T, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ct, ID_MB, exp_ctop_cbottom); DEL(&autorelease, sd_t_mb_cb_ct, ID_MB, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_TB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ct, ID_CB, exp_ctop_cbottom); DEL(&autorelease, sd_t_mb_cb_ct, ID_CB, exp_ctop_midbottom);
	ADD(&autorelease, sd_t_mb_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ct, ID_CT, exp_ctop_cbottom); DEL(&autorelease, sd_t_mb_cb_ct, ID_CT, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cb+ctm+x
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_B, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_M, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ctm, ID_T, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_mb_cb_ctm, ID_T, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ctm, ID_MB, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_mb_cb_ctm, ID_MB, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_mb_cb_ctm, ID_CB, exp_ctopmid);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_CM, exp_cmid_cbottom_top, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_CT, exp_ctop_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_t_mb_cb_ctm, ID_CTM, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cm+ct+x
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_B, exp_ctop_cmid_bottom, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_M, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_t_mb_cm_ct, ID_T, exp_ctop_cmid); DEL(&autorelease, sd_t_mb_cm_ct, ID_T, exp_ctop_cmid);
	ADD(&autorelease, sd_t_mb_cm_ct, ID_MB, exp_ctop_cmid); DEL(&autorelease, sd_t_mb_cm_ct, ID_MB, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_TM, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_t_mb_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cm_ct, ID_CM, exp_ctop_cmid); DEL(&autorelease, sd_t_mb_cm_ct, ID_CM, exp_ctop_midbottom);
	ADD(&autorelease, sd_t_mb_cm_ct, ID_CT, exp_ctop_cmid); DEL(&autorelease, sd_t_mb_cm_ct, ID_CT, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+cm+ctb+x
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_B, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_M, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_t_mb_cm_ctb, ID_T, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_mb_cm_ctb, ID_T, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_t_mb_cm_ctb, ID_MB, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_mb_cm_ctb, ID_MB, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_CB, exp_cmid_cbottom_top, exp_cmid_top);
	ADD(&autorelease, sd_t_mb_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_mb_cm_ctb, ID_CM, exp_ctopbottom);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_CT, exp_ctop_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_CMB, exp_cmidbottom_top, exp_top_midbottom);
	ADD(&autorelease, sd_t_mb_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_t_mb_cm_ctb, ID_CTB, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+mb+ct+cmb+x
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_B, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_M, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_mb_ct_cmb, ID_T, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_mb_ct_cmb, ID_T, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_t_mb_ct_cmb, ID_MB, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_mb_ct_cmb, ID_MB, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_CB, exp_ctop_cbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_CM, exp_ctop_cmid, exp_ctop_midbottom);
	ADD(&autorelease, sd_t_mb_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_mb_ct_cmb, ID_CT, exp_cmidbottom_top);
	ADD(&autorelease, sd_t_mb_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_t_mb_ct_cmb, ID_CMB, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_CTB, exp_ctopbottom, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_CTM, exp_ctopmid, exp_top_midbottom);
	TOGGLE(&autorelease, sd_t_mb_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);
	// t+cb+cm+ct+x
	ADD(&autorelease, sd_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_cb_cm_ct, ID_CT, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top);
	// mb+cb+cm+ct+x
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_mb_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_mb_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_mb_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_mb_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_mb_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_midbottom);
	// tb+cb+cm+ct+x
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tb_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_tb_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_tb_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tb_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tb_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom);
	// tm+cb+cm+ct+x
	ADD(&autorelease, sd_tm_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tm_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tm_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_tm_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_tm_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tm_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_tm_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid);
	// tmb+cb+cm+ct+x
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_tmb_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_tmb_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_tmb_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_tmb_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom);
	TOGGLE(&autorelease, sd_tmb_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmidbottom);

	// Six: (1)
	// b+m+t+cb+cm+x: b+m+t+cb+cm+ct [1]
	ADD(&autorelease, sd_b_m_t_cb_cm, ID_B, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_m_t_cb_cm, ID_B, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_m_t_cb_cm, ID_M, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_m_t_cb_cm, ID_M, exp_cmid_cbottom_top);
	ADD(&autorelease, sd_b_m_t_cb_cm, ID_T, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_m_t_cb_cm, ID_T, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_MB, exp_cmid_cbottom_top, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_TB, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_TM, exp_cmid_cbottom, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_TMB, exp_cmid_cbottom, exp_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_cm, ID_CB, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_m_t_cb_cm, ID_CB, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_m_t_cb_cm, ID_CM, exp_cmid_cbottom_top); DEL(&autorelease, sd_b_m_t_cb_cm, ID_CM, exp_cbottom_top_mid);
	struct map_session_data *sd_b_m_t_cb_cm_ct = ADD(&autorelease, sd_b_m_t_cb_cm, ID_CT, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cb+ct+x
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_B, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_t_cb_ct, ID_B, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_M, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_t_cb_ct, ID_M, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_T, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_t_cb_ct, ID_T, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_MB, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_TB, exp_ctop_cbottom_mid, exp_ctop_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_TM, exp_ctop_cbottom, exp_ctop_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_TMB, exp_ctop_cbottom, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_CB, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_t_cb_ct, ID_CB, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_CM, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ct, ID_CT, exp_ctop_cbottom_mid); DEL(&autorelease, sd_b_m_t_cb_ct, ID_CT, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cb+ctm+x
	ADD(&autorelease, sd_b_m_t_cb_ctm, ID_B, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_ctm, ID_B, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ctm, ID_M, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_ctm, ID_M, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ctm, ID_T, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_ctm, ID_T, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_MB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_TB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_TM, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_TMB, exp_ctopmid_cbottom, exp_ctopmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_ctm, ID_CB, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_ctm, ID_CB, exp_ctopmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_CM, exp_cmid_cbottom_top, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_CT, exp_ctop_cbottom_mid, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_cb_ctm, ID_CTM, exp_ctopmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_ctm, ID_CTM, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_ctm, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cm+ct+x
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_B, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_t_cm_ct, ID_B, exp_ctop_cmid);
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_M, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_t_cm_ct, ID_M, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_T, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_t_cm_ct, ID_T, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_MB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_TB, exp_ctop_cmid, exp_ctop_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_TM, exp_ctop_cmid_bottom, exp_ctop_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_TMB, exp_ctop_cmid, exp_ctop_cmid);
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_CB, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_CM, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_t_cm_ct, ID_CM, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_t_cm_ct, ID_CT, exp_ctop_cmid_bottom); DEL(&autorelease, sd_b_m_t_cm_ct, ID_CT, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+cm+ctb+x
	ADD(&autorelease, sd_b_m_t_cm_ctb, ID_B, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_t_cm_ctb, ID_B, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_m_t_cm_ctb, ID_M, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_t_cm_ctb, ID_M, exp_ctopbottom_cmid);
	ADD(&autorelease, sd_b_m_t_cm_ctb, ID_T, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_t_cm_ctb, ID_T, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_MB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_TB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_TM, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_TMB, exp_ctopbottom_cmid, exp_ctopbottom_cmid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_CB, exp_cmid_cbottom_top, exp_cmid_top_bottom);
	ADD(&autorelease, sd_b_m_t_cm_ctb, ID_CM, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_t_cm_ctb, ID_CM, exp_ctopbottom_mid);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_CT, exp_ctop_cmid_bottom, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_CMB, exp_cmidbottom_top, exp_top_mid_bottom);
	ADD(&autorelease, sd_b_m_t_cm_ctb, ID_CTB, exp_ctopbottom_cmid); DEL(&autorelease, sd_b_m_t_cm_ctb, ID_CTB, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cm_ctb, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+t+ct+cmb+x
	ADD(&autorelease, sd_b_m_t_ct_cmb, ID_B, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_t_ct_cmb, ID_B, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_t_ct_cmb, ID_M, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_t_ct_cmb, ID_M, exp_ctop_cmidbottom);
	ADD(&autorelease, sd_b_m_t_ct_cmb, ID_T, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_t_ct_cmb, ID_T, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_MB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_TB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_TM, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_TMB, exp_ctop_cmidbottom, exp_ctop_cmidbottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_CB, exp_ctop_cbottom_mid, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_CM, exp_ctop_cmid_bottom, exp_ctop_mid_bottom);
	ADD(&autorelease, sd_b_m_t_ct_cmb, ID_CT, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_t_ct_cmb, ID_CT, exp_cmidbottom_top);
	ADD(&autorelease, sd_b_m_t_ct_cmb, ID_CMB, exp_ctop_cmidbottom); DEL(&autorelease, sd_b_m_t_ct_cmb, ID_CMB, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_CTB, exp_ctopbottom_mid, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_CTM, exp_ctopmid_bottom, exp_top_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_ct_cmb, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);
	// b+m+cb+cm+ct
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm_ct, ID_CB, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm_ct, ID_CM, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_b_m_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_mid_bottom);
	// b+t+cb+cm+ct+x
	ADD(&autorelease, sd_b_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_t_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_t_cb_cm_ct, ID_CB, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_t_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_t_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_t_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_t_cb_cm_ct, ID_CT, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_b_t_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_bottom);
	// b+tm+cb+cm+ct+x
	ADD(&autorelease, sd_b_tm_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_tm_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm_ct, ID_CB, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_tm_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_b_tm_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_tm_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_bottom);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_topmid);
	TOGGLE(&autorelease, sd_b_tm_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topmid_bottom);
	// m+t+cb+cm+ct+x
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_t_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_t_cb_cm_ct, ID_CM, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_t_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_t_cb_cm_ct, ID_CT, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_m_t_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid);
	// m+tb+cb+cm+ct+x
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_m_tb_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_m_tb_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm_ct, ID_CM, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_m_tb_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_m_tb_cb_cm_ct, ID_CT, exp_cmid_cbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_topbottom);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_mid);
	TOGGLE(&autorelease, sd_m_tb_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_topbottom_mid);
	// t+mb+cb+cm+ct+x
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_mb_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_mb_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_mb_cb_cm_ct, ID_CB, exp_ctop_cmid);
	ADD(&autorelease, sd_t_mb_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_mb_cb_cm_ct, ID_CM, exp_ctop_cbottom);
	ADD(&autorelease, sd_t_mb_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_t_mb_cb_cm_ct, ID_CT, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_midbottom);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top);
	TOGGLE(&autorelease, sd_t_mb_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_midbottom);

	// Seven (0)
	// b+m+t+cb+cm+ct+x
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_B, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_M, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_T, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_MB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_TB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_TM, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_TMB, exp_ctop_cmid_cbottom, exp_ctop_cmid_cbottom);
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_CB, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_CB, exp_ctop_cmid_bottom);
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_CM, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_CM, exp_ctop_cbottom_mid);
	ADD(&autorelease, sd_b_m_t_cb_cm_ct, ID_CT, exp_ctop_cmid_cbottom); DEL(&autorelease, sd_b_m_t_cb_cm_ct, ID_CT, exp_cmid_cbottom_top);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_CMB, exp_ctop_cmidbottom, exp_ctop_mid_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_CTB, exp_ctopbottom_cmid, exp_cmid_top_bottom);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_CTM, exp_ctopmid_cbottom, exp_cbottom_top_mid);
	TOGGLE(&autorelease, sd_b_m_t_cb_cm_ct, ID_CTMB, exp_ctopmidbottom, exp_top_mid_bottom);

	// 220 * 14 -> 3080

	while (VECTOR_LENGTH(autorelease) > 0) {
		aFree(VECTOR_POP(autorelease));
	}
	VECTOR_CLEAR(autorelease);

	ShowMessage("===============================================================================\n");
	ShowStatus("All tests passed.\n");
	map->do_shutdown();
}

HPExport void plugin_final(void)
{
}
