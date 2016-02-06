/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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

#include "npc.h" // struct npc_data

#include "map/mob.h" // struct mob_data
#include "map/pc.h" // struct map_session_data
#include "map/script.h" // set_var()
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <pcre.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * interface sources
 **/
struct npc_chat_interface npc_chat_s;
struct pcre_interface libpcre_s;

struct npc_chat_interface *npc_chat;
struct pcre_interface *libpcre;

/**
 *  Written by MouseJstr in a vision... (2/21/2005)
 *
 *  This allows you to make npc listen for spoken text (global
 *  messages) and pattern match against that spoken text using perl
 *  regular expressions.
 *
 *  Please feel free to copy this code into your own personal ragnarok
 *  servers or distributions but please leave my name.  Also, please
 *  wait until I've put it into the main eA branch which means I
 *  believe it is ready for distribution.
 *
 *  So, how do people use this?
 *
 *  The first and most important function is defpattern
 *
 *    defpattern 1, "[^:]+: (.*) loves (.*)", "label";
 *
 *  this defines a new pattern in set 1 using perl syntax
 *    (http://www.troubleshooters.com/codecorn/littperl/perlreg.htm)
 *  and tells it to jump to the supplied label when the pattern
 *  is matched.
 *
 *  each of the matched Groups will result in a variable being
 *  set ($@p1$ through $@p9$  with $@p0$ being the entire string)
 *  before the script gets executed.
 *
 *    activatepset 1;
 *
 *  This activates a set of patterns.. You can have many pattern
 *  sets defined and many active all at once.  This feature allows
 *  you to set up "conversations" and ever changing expectations of
 *  the pattern matcher
 *
 *    deactivatepset 1;
 *
 *  turns off a pattern set;
 *
 *    deactivatepset -1;
 *
 *  turns off ALL pattern sets;
 *
 *    deletepset 1;
 *
 *  deletes a pset
 */

/**
 * delete everything associated with a entry
 *
 * This does NOT do the list management
 */
void finalize_pcrematch_entry(struct pcrematch_entry* e)
{
	libpcre->free(e->pcre_);
	libpcre->free(e->pcre_extra_);
	aFree(e->pattern);
	aFree(e->label);
}

/**
 * Lookup (and possibly create) a new set of patterns by the set id
 */
struct pcrematch_set* lookup_pcreset(struct npc_data* nd, int setid) {
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = nd->chatdb;
	if (npcParse == NULL)
		nd->chatdb = npcParse = (struct npc_parse *)aCalloc(sizeof(struct npc_parse), 1);

	pcreset = npcParse->active;

	while (pcreset != NULL) {
		if (pcreset->setid == setid)
		break;
		pcreset = pcreset->next;
	}
	if (pcreset == NULL)
		pcreset = npcParse->inactive;

	while (pcreset != NULL) {
		if (pcreset->setid == setid)
		break;
		pcreset = pcreset->next;
	}

	if (pcreset == NULL) {
		pcreset = (struct pcrematch_set *)aCalloc(sizeof(struct pcrematch_set), 1);
		pcreset->next = npcParse->inactive;
		if (pcreset->next != NULL)
			pcreset->next->prev = pcreset;
		pcreset->prev = 0;
		npcParse->inactive = pcreset;
		pcreset->setid = setid;
	}
	return pcreset;
}

/**
 * activate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */
void activate_pcreset(struct npc_data* nd, int setid)
{
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = nd->chatdb;
	if (npcParse == NULL)
		return; // Nothing to activate...
	pcreset = npcParse->inactive;
	while (pcreset != NULL) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == NULL)
		return; // not in inactive list
	if (pcreset->next != NULL)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != NULL)
		pcreset->prev->next = pcreset->next;
	else
		npcParse->inactive = pcreset->next;

	pcreset->prev = NULL;
	pcreset->next = npcParse->active;
	if (pcreset->next != NULL)
		pcreset->next->prev = pcreset;
	npcParse->active = pcreset;
}

/**
 * deactivate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */
void deactivate_pcreset(struct npc_data* nd, int setid)
{
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = nd->chatdb;
	if (npcParse == NULL)
		return; // Nothing to deactivate...
	if (setid == -1) {
		while(npcParse->active != NULL)
			npc_chat->deactivate_pcreset(nd, npcParse->active->setid);
		return;
	}
	pcreset = npcParse->active;
	while (pcreset != NULL) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == NULL)
		return; // not in active list
	if (pcreset->next != NULL)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != NULL)
		pcreset->prev->next = pcreset->next;
	else
		npcParse->active = pcreset->next;

	pcreset->prev = NULL;
	pcreset->next = npcParse->inactive;
	if (pcreset->next != NULL)
		pcreset->next->prev = pcreset;
	npcParse->inactive = pcreset;
}

/**
 * delete a set of patterns.
 */
void delete_pcreset(struct npc_data* nd, int setid)
{
	int active = 1;
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = nd->chatdb;
	if (npcParse == NULL)
		return; // Nothing to deactivate...
	pcreset = npcParse->active;
	while (pcreset != NULL) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == NULL) {
		active = 0;
		pcreset = npcParse->inactive;
		while (pcreset != NULL) {
			if (pcreset->setid == setid)
				break;
			pcreset = pcreset->next;
		}
	}
	if (pcreset == NULL)
		return;

	if (pcreset->next != NULL)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != NULL)
		pcreset->prev->next = pcreset->next;

	if(active)
		npcParse->active = pcreset->next;
	else
		npcParse->inactive = pcreset->next;

	pcreset->prev = NULL;
	pcreset->next = NULL;

	while (pcreset->head) {
		struct pcrematch_entry* n = pcreset->head->next;
		npc_chat->finalize_pcrematch_entry(pcreset->head);
		aFree(pcreset->head); // Cleaning the last ones.. [Lance]
		pcreset->head = n;
	}
	aFree(pcreset);
}

/**
 * create a new pattern entry
 */
struct pcrematch_entry* create_pcrematch_entry(struct pcrematch_set* set)
{
	struct pcrematch_entry * e =  (struct pcrematch_entry *) aCalloc(sizeof(struct pcrematch_entry), 1);
	struct pcrematch_entry * last = set->head;

	// Normally we would have just stuck it at the end of the list but
	// this doesn't sink up with peoples usage pattern.  They wanted
	// the items defined first to have a higher priority then the
	// items defined later. as a result, we have to do some work up front.

	/*  if we are the first pattern, stick us at the end */
	if (last == NULL) {
		set->head = e;
		return e;
	}

	/* Look for the last entry */
	while (last->next != NULL)
		last = last->next;

	last->next = e;
	e->next = NULL;

	return e;
}

/**
 * define/compile a new pattern
 */
void npc_chat_def_pattern(struct npc_data* nd, int setid, const char* pattern, const char* label)
{
	const char *err;
	int erroff;

	struct pcrematch_set * s = npc_chat->lookup_pcreset(nd, setid);
	struct pcrematch_entry *e = npc_chat->create_pcrematch_entry(s);
	e->pattern = aStrdup(pattern);
	e->label = aStrdup(label);
	e->pcre_ = libpcre->compile(pattern, PCRE_CASELESS, &err, &erroff, NULL);
	e->pcre_extra_ = libpcre->study(e->pcre_, 0, &err);
}

/**
 * Delete everything associated with a NPC concerning the pattern
 * matching code
 *
 * this could be more efficient but.. how often do you do this?
 */
void npc_chat_finalize(struct npc_data* nd)
{
	struct npc_parse *npcParse = nd->chatdb;
	if (npcParse == NULL)
		return;

	while(npcParse->active)
		npc_chat->delete_pcreset(nd, npcParse->active->setid);

	while(npcParse->inactive)
		npc_chat->delete_pcreset(nd, npcParse->inactive->setid);

	// Additional cleaning up [Lance]
	aFree(npcParse);
}

/**
 * Handler called whenever a global message is spoken in a NPC's area
 */
int npc_chat_sub(struct block_list* bl, va_list ap)
{
	struct npc_data *nd = NULL;
	struct npc_parse *npcParse = NULL;
	char *msg;
	int len, i;
	struct map_session_data* sd;
	struct npc_label_list* lst;
	struct pcrematch_set* pcreset;
	struct pcrematch_entry* e;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCAST(BL_NPC, bl);
	npcParse = nd->chatdb;

	// Not interested in anything you might have to say...
	if (npcParse == NULL || npcParse->active == NULL)
		return 0;

	msg = va_arg(ap,char*);
	len = va_arg(ap,int);
	sd = va_arg(ap,struct map_session_data *);

	// iterate across all active sets
	for (pcreset = npcParse->active; pcreset != NULL; pcreset = pcreset->next)
	{
		// n across all patterns in that set
		for (e = pcreset->head; e != NULL; e = e->next)
		{
			int offsets[2*10 + 10]; // 1/3 reserved for temp space required by pcre_exec

			// perform pattern match
			int r = libpcre->exec(e->pcre_, e->pcre_extra_, msg, len, 0, 0, offsets, ARRAYLENGTH(offsets));
			if (r > 0)
			{
				// save out the matched strings
				for (i = 0; i < r; i++)
				{
					char var[6], val[255];
					snprintf(var, sizeof(var), "$@p%i$", i);
					libpcre->copy_substring(msg, offsets, r, i, val, sizeof(val));
					script->set_var(sd, var, val);
				}

				// find the target label.. this sucks..
				lst = nd->u.scr.label_list;
				ARR_FIND(0, nd->u.scr.label_list_num, i, strncmp(lst[i].name, e->label, sizeof(lst[i].name)) == 0);
				if (i == nd->u.scr.label_list_num) {
					ShowWarning("npc_chat_sub: Unable to find label: %s\n", e->label);
					return 0;
				}

				// run the npc script
				script->run_npc(nd->u.scr.script,lst[i].pos,sd->bl.id,nd->bl.id);
				return 0;
			}
		}
	}
	return 0;
}

// Various script built-ins used to support these functions
BUILDIN(defpattern)
{
	int setid = script_getnum(st,2);
	const char* pattern = script_getstr(st,3);
	const char* label = script_getstr(st,4);
	struct npc_data *nd = map->id2nd(st->oid);
	nullpo_retr(false, nd);

	npc_chat->def_pattern(nd, setid, pattern, label);

	return true;
}

BUILDIN(activatepset)
{
	int setid = script_getnum(st,2);
	struct npc_data *nd = map->id2nd(st->oid);
	nullpo_retr(false, nd);

	npc_chat->activate_pcreset(nd, setid);

	return true;
}

BUILDIN(deactivatepset)
{
	int setid = script_getnum(st,2);
	struct npc_data *nd = map->id2nd(st->oid);
	nullpo_retr(false, nd);

	npc_chat->deactivate_pcreset(nd, setid);

	return true;
}

BUILDIN(deletepset)
{
	int setid = script_getnum(st,2);
	struct npc_data *nd = map->id2nd(st->oid);
	nullpo_retr(false, nd);

	npc_chat->delete_pcreset(nd, setid);

	return true;
}

void npc_chat_defaults(void) {
	npc_chat = &npc_chat_s;

	npc_chat->sub = npc_chat_sub;
	npc_chat->finalize = npc_chat_finalize;
	npc_chat->def_pattern = npc_chat_def_pattern;
	npc_chat->create_pcrematch_entry = create_pcrematch_entry;
	npc_chat->delete_pcreset = delete_pcreset;
	npc_chat->deactivate_pcreset = deactivate_pcreset;
	npc_chat->activate_pcreset = activate_pcreset;
	npc_chat->lookup_pcreset = lookup_pcreset;
	npc_chat->finalize_pcrematch_entry = finalize_pcrematch_entry;

	libpcre = &libpcre_s;

	libpcre->compile = pcre_compile;
	libpcre->study = pcre_study;
	libpcre->exec = pcre_exec;
	libpcre->free = pcre_free;
	libpcre->copy_substring = pcre_copy_substring;
	libpcre->free_substring = pcre_free_substring;
	libpcre->copy_named_substring = pcre_copy_named_substring;
	libpcre->get_substring = pcre_get_substring;
}
