// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Sample Hercules Plugin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/HPMi.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/socket.h"

#if defined (HPMHOOKING_LOGIN)
#define HPM_SERVER_TYPE SERVER_TYPE_LOGIN
#define HPM_CORE_INCLUDE "../plugins/HPMHooking/HPMHooking_login.HPMHooksCore.inc"
#define HPM_SYMBOL_INCLUDE "../plugins/HPMHooking/HPMHooking_login.GetSymbol.inc"
#define HPM_HOOKS_INCLUDE "../plugins/HPMHooking/HPMHooking_login.Hooks.inc"
#define HPM_POINTS_INCLUDE "../plugins/HPMHooking/HPMHooking_login.HookingPoints.inc"
#define HPM_SOURCES_INCLUDE "../plugins/HPMHooking/HPMHooking_login.sources.inc"
#include "../login/login.h"
#elif defined (HPMHOOKING_CHAR)
#define HPM_SERVER_TYPE SERVER_TYPE_CHAR
#define HPM_CORE_INCLUDE "../plugins/HPMHooking/HPMHooking_char.HPMHooksCore.inc"
#define HPM_SYMBOL_INCLUDE "../plugins/HPMHooking/HPMHooking_char.GetSymbol.inc"
#define HPM_HOOKS_INCLUDE "../plugins/HPMHooking/HPMHooking_char.Hooks.inc"
#define HPM_POINTS_INCLUDE "../plugins/HPMHooking/HPMHooking_char.HookingPoints.inc"
#define HPM_SOURCES_INCLUDE "../plugins/HPMHooking/HPMHooking_char.sources.inc"
#include "../char/char.h"
#include "../char/geoip.h"
#include "../char/int_auction.h"
#include "../char/int_elemental.h"
#include "../char/int_guild.h"
#include "../char/int_homun.h"
#include "../char/int_mail.h"
#include "../char/int_mercenary.h"
#include "../char/int_party.h"
#include "../char/int_pet.h"
#include "../char/int_quest.h"
#include "../char/int_storage.h"
#include "../char/inter.h"
#include "../char/loginif.h"
#include "../char/mapif.h"
#include "../char/pincode.h"
#elif defined (HPMHOOKING_MAP)
#define HPM_SERVER_TYPE SERVER_TYPE_MAP
#define HPM_CORE_INCLUDE "../plugins/HPMHooking/HPMHooking_map.HPMHooksCore.inc"
#define HPM_SYMBOL_INCLUDE "../plugins/HPMHooking/HPMHooking_map.GetSymbol.inc"
#define HPM_HOOKS_INCLUDE "../plugins/HPMHooking/HPMHooking_map.Hooks.inc"
#define HPM_POINTS_INCLUDE "../plugins/HPMHooking/HPMHooking_map.HookingPoints.inc"
#define HPM_SOURCES_INCLUDE "../plugins/HPMHooking/HPMHooking_map.sources.inc"
#include "../map/atcommand.h"
#include "../map/battle.h"
#include "../map/battleground.h"
#include "../map/channel.h"
#include "../map/chat.h"
#include "../map/chrif.h"
#include "../map/clif.h"
#include "../map/duel.h"
#include "../map/elemental.h"
#include "../map/guild.h"
#include "../map/homunculus.h"
#include "../map/instance.h"
#include "../map/intif.h"
#include "../map/irc-bot.h"
#include "../map/itemdb.h"
#include "../map/log.h"
#include "../map/mail.h"
#include "../map/map.h"
#include "../map/mapreg.h"
#include "../map/mercenary.h"
#include "../map/mob.h"
#include "../map/npc.h"
#include "../map/npc.h"
#include "../map/party.h"
#include "../map/path.h"
#include "../map/pc.h"
#include "../map/pet.h"
#include "../map/quest.h"
#include "../map/script.h"
#include "../map/skill.h"
#include "../map/status.h"
#include "../map/storage.h"
#include "../map/trade.h"
#include "../map/unit.h"
#else
#define HPM_SERVER_TYPE SERVER_TYPE_UNKNOWN
#define HPM_CORE_INCLUDE "../plugins/HPMHooking/HPMHooking.HPMHooksCore.inc"
#define HPM_SYMBOL_INCLUDE "../plugins/HPMHooking/HPMHooking.GetSymbol.inc"
#define HPM_HOOKS_INCLUDE "../plugins/HPMHooking/HPMHooking.Hooks.inc"
#define HPM_POINTS_INCLUDE "../plugins/HPMHooking/HPMHooking.HookingPoints.inc"
#define HPM_SOURCES_INCLUDE "../plugins/HPMHooking/HPMHooking.sources.inc"
#error HPMHooking plugin needs to be compiled for a specific server type. Please make sure your Makefiles are up to date.
#endif

#include "../common/HPMDataCheck.h"

HPExport struct hplugin_info pinfo = {
	"HPMHooking",   // Plugin name
	HPM_SERVER_TYPE,// Which server types this plugin works with?
	"0.2",          // Plugin version
	HPM_VERSION,    // HPM Version (don't change, macro is automatically updated)
};

#define HP_POP(x,y) #x , (void**)(&x) , (void*)y , 0
DBMap *hp_db;/* hooking points db -- for quick lookup */

struct HookingPointData {
	char* name;
	void **sref;
	void *tref;
	int idx;
};

struct HPMHookPoint {
	void *func;
	unsigned int pID;
};

struct HPMHooksCore {
	#include HPM_CORE_INCLUDE
	struct {
		int total;
	} data;
} HPMHooks;

bool *HPMforce_return;

void HPM_HP_final(void);
void HPM_HP_load(void);

HPExport void server_post_final (void) {
	HPM_HP_final();
}

HPExport bool Hooked (bool *fr) {
	HPMforce_return = fr;
	DB = GET_SYMBOL("DB");
	iMalloc = GET_SYMBOL("iMalloc");
#include HPM_SYMBOL_INCLUDE
	HPM_HP_load();
	return true;
}


HPExport bool HPM_Plugin_AddHook(enum HPluginHookType type, const char *target, void *hook, unsigned int pID) {
	struct HookingPointData *hpd;

	if( hp_db && (hpd = strdb_get(hp_db,target)) ) {
		struct HPMHookPoint **hp = NULL;
		int *count = NULL;

		if( type == HOOK_TYPE_PRE ) {
			hp = (struct HPMHookPoint **)((char *)&HPMHooks.list + (sizeof(struct HPMHookPoint *)*hpd->idx));
			count = (int *)((char *)&HPMHooks.count + (sizeof(int)*hpd->idx));
		} else {
			hp = (struct HPMHookPoint **)((char *)&HPMHooks.list + (sizeof(struct HPMHookPoint *)*(hpd->idx+1)));
			count = (int *)((char *)&HPMHooks.count + (sizeof(int)*(hpd->idx+1)));
		}

		if( hp ) {
			*count += 1;

			RECREATE(*hp, struct HPMHookPoint, *count);

			(*hp)[*count - 1].func = hook;
			(*hp)[*count - 1].pID = pID;

			*(hpd->sref) = hpd->tref;

			return true;
		}
	}

	return false;
}

#include HPM_HOOKS_INCLUDE

void HPM_HP_final(void) {
	int i, len = HPMHooks.data.total * 2;

	if( hp_db )
		db_destroy(hp_db);

	for(i = 0; i < len; i++) {
		int *count = (int *)((char *)&HPMHooks.count + (sizeof(int)*(i)));

		if( count && *count ) {
			struct HPMHookPoint **hp = (struct HPMHookPoint **)((char *)&HPMHooks.list + (sizeof(struct HPMHookPoint *)*(i)));

			if( hp && *hp )
				aFree(*hp);
		}
	}
}

void HPM_HP_load(void) {
	#include HPM_POINTS_INCLUDE
	int i, len = ARRAYLENGTH(HookingPoints), idx = 0;

	memset(&HPMHooks,0,sizeof(struct HPMHooksCore));

	hp_db = strdb_alloc(DB_OPT_BASE|DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, HookingPointsLenMax);

	for(i = 0; i < len; i++) {
		struct HookingPointData *hpd = NULL;

		CREATE(hpd, struct HookingPointData, 1);

		memcpy(hpd, &HookingPoints[i], sizeof(struct HookingPointData));

		hpd->idx = idx;
		idx += 2;

		strdb_put(hp_db, HookingPoints[i].name, hpd);

		HPMHooks.data.total++;
	}

	#include HPM_SOURCES_INCLUDE
}

