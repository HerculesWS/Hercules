/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2026 Hercules Dev Team
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
#ifndef MAP_HPMI_MAP_H
#define MAP_HPMI_MAP_H

#include "map/instance.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "common/HPMi.h"

class hpm_plugin_imap : public hpm_plugin_i
{
  public:
	hpm_plugin_imap(const char *a_name, enum server_types a_type, const char *a_version, const char *the_req_version,
	                bool with_hpmhooking, const struct s_HPMDataCheck *the_HPMDataCheck,
	                unsigned int the_HPMDataCheckLen, int the_HPMDataCheckVer, const struct HPMi_interface *the_HPMi)
	    : hpm_plugin_i(a_name, a_type, a_version, the_req_version, with_hpmhooking, the_HPMDataCheck,
	                   the_HPMDataCheckLen, the_HPMDataCheckVer, the_HPMi)
	{
	}

	bool add_atcommand(const char *cname, bool (*func)(const int fd, struct map_session_data *sd, const char *command,
	                                                   const char *message, struct AtCommandInfo *info))
	{
		nullpo_retr(false, HPMi->addCommand);
		return HPMi->addCommand(cname, func);
	}

	bool add_script_command(const char *cname, const char *args, bool (*func)(struct script_state *st),
	                        bool is_deprecated)
	{
		nullpo_retr(false, HPMi->addScript);
		return HPMi->addScript(cname, args, func, is_deprecated);
	}

	bool add_battle_conf(const char *bcname, void (*parse_func)(const char *key, const char *val),
	                     int (*return_func)(const char *key), bool required)
	{
		return this->add_conf(HPCT_BATTLE, bcname, parse_func, return_func, required);
	}

	bool add_map_inter_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_MAP_INTER, bcname, parse_func, nullptr, false);
	}

	bool add_script_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_SCRIPT, bcname, parse_func, nullptr, false);
	}

	/* map_session_data */
	void add_to_map_session_data(struct map_session_data *sd, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_MSD, pid(), &sd->hdata, data, classid, autofree);
	}

	void *get_from_map_session_data(struct map_session_data *sd, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_MSD, pid(), sd->hdata, classid);
	}

	void remove_from_map_session_data(struct map_session_data *sd, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_MSD, pid(), sd->hdata, classid);
	}

	/* npc_data */
	void add_to_npc_data(struct npc_data *nd, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_NPCD, pid(), &nd->hdata, data, classid, autofree);
	}

	void *get_from_npc_data(struct npc_data *nd, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_NPCD, pid(), nd->hdata, classid);
	}

	void remove_from_npc_data(struct npc_data *nd, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_NPCD, pid(), nd->hdata, classid);
	}

	/* map_data */
	void add_to_map_data(struct map_data *m, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_MAP, pid(), &m->hdata, data, classid, autofree);
	}

	void *get_from_map_data(struct map_data *m, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_MAP, pid(), m->hdata, classid);
	}

	void remove_from_map_data(struct map_data *m, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_MAP, pid(), m->hdata, classid);
	}

	/* party_data */
	void add_to_party_data(struct party_data *p, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_PARTY, pid(), &p->hdata, data, classid, autofree);
	}

	void *get_from_party_data(struct party_data *p, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_PARTY, pid(), p->hdata, classid);
	}

	void remove_from_party_data(struct party_data *p, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_PARTY, pid(), p->hdata, classid);
	}

	/* guild */
	void add_to_guild_data(struct guild *g, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_GUILD, pid(), &g->hdata, data, classid, autofree);
	}

	void *get_from_guild_data(struct guild *g, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_GUILD, pid(), g->hdata, classid);
	}

	void remove_from_guild_data(struct guild *g, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_GUILD, pid(), g->hdata, classid);
	}

	/* instance_data */
	void add_to_instance_data(struct instance_data *inst, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_INSTANCE, pid(), &inst->hdata, data, classid, autofree);
	}

	void *get_from_instance_data(struct instance_data *inst, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_INSTANCE, pid(), inst->hdata, classid);
	}

	void remove_from_instance_data(struct instance_data *inst, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_INSTANCE, pid(), inst->hdata, classid);
	}

	/* mob_db */
	void add_to_mob_db(struct mob_db *mdb, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_MOBDB, pid(), &mdb->hdata, data, classid, autofree);
	}

	void *get_from_mob_db(struct mob_db *mdb, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_MOBDB, pid(), mdb->hdata, classid);
	}

	void remove_from_mob_db(struct mob_db *mdb, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_MOBDB, pid(), mdb->hdata, classid);
	}

	/* mob_data */
	void add_to_mob_data(struct mob_data *md, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_MOBDATA, pid(), &md->hdata, data, classid, autofree);
	}

	void *get_from_mob_data(struct mob_data *md, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_MOBDATA, pid(), md->hdata, classid);
	}

	void remove_from_mob_data(struct mob_data *md, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_MOBDATA, pid(), md->hdata, classid);
	}

	/* item_data */
	void add_to_item_data(struct item_data *itd, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_ITEMDATA, pid(), &itd->hdata, data, classid, autofree);
	}

	void *get_from_item_data(struct item_data *itd, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_ITEMDATA, pid(), itd->hdata, classid);
	}

	void remove_from_item_data(struct item_data *itd, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_ITEMDATA, pid(), itd->hdata, classid);
	}

	/* battleground_data */
	void add_to_battleground_data(struct battleground_data *bgd, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_BGDATA, pid(), &bgd->hdata, data, classid, autofree);
	}

	void *get_from_battleground_data(struct battleground_data *bgd, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_BGDATA, pid(), bgd->hdata, classid);
	}

	void remove_from_battleground_data(struct battleground_data *bgd, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_BGDATA, pid(), bgd->hdata, classid);
	}

	/* autotrade_vending */
	void add_to_autotrade_vending(struct autotrade_vending *atv, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_AUTOTRADE_VEND, pid(), &atv->hdata, data, classid, autofree);
	}

	void *get_from_autotrade_vending(struct autotrade_vending *atv, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_AUTOTRADE_VEND, pid(), atv->hdata, classid);
	}

	void remove_from_autotrade_vending(struct autotrade_vending *atv, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_AUTOTRADE_VEND, pid(), atv->hdata, classid);
	}

	/* clan */
	void add_to_clan(struct clan *c, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_CLAN, pid(), &c->hdata, data, classid, autofree);
	}

	void *get_from_clan(struct clan *c, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_CLAN, pid(), c->hdata, classid);
	}

	void remove_from_clan(struct clan *c, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_CLAN, pid(), c->hdata, classid);
	}

	/* unit parameters */
	void add_to_unit_params(struct s_unit_params *ud, void *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_UNIT_PARAMETER, pid(), &ud->hdata, data, classid, autofree);
	}

	void *get_from_unit_params(struct s_unit_params *ud, uint32 classid)
	{
		return HPMi->getFromHPData(HPDT_UNIT_PARAMETER, pid(), ud->hdata, classid);
	}

	void remove_from_unit_params(struct s_unit_params *ud, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_UNIT_PARAMETER, pid(), ud->hdata, classid);
	}
};

#endif /* MAP_HPMI_MAP_H */
